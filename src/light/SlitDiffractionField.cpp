#include "SlitDiffractionField.h"

#include <algorithm>
#include <cmath>
#include <complex>
#include <limits>
#include <stdexcept>
#include <utility>

namespace {

using Complex = std::complex<double>;

constexpr double kPi = 3.141592653589793238462643383279502884;
constexpr double kMmToM = 1.0e-3;
constexpr double kNmToM = 1.0e-9;

struct SlitLocalFrame {
    Vector3f center;
    Vector3f eU;
    Vector3f eV;
    Vector3f eN;
    double widthMm;
    double heightMm;
};

struct QuadratureRule {
    std::vector<double> nodes;
    std::vector<double> weights;
};

double dotAsDouble(const Vector3f& a, const Vector3f& b) {
    return static_cast<double>(a.x) * static_cast<double>(b.x)
         + static_cast<double>(a.y) * static_cast<double>(b.y)
         + static_cast<double>(a.z) * static_cast<double>(b.z);
}

double lengthAsDouble(const Vector3f& v) {
    return std::sqrt(dotAsDouble(v, v));
}

Complex expI(double phase) {
    return {std::cos(phase), std::sin(phase)};
}

SlitLocalFrame buildSlitLocalFrame(const SlitLight& slit) {
    const double widthMm = lengthAsDouble(slit.edgeU());
    const double heightMm = lengthAsDouble(slit.edgeV());
    if (widthMm <= 0.0 || heightMm <= 0.0) {
        throw std::invalid_argument("SlitDiffractionField requires non-degenerate slit edges");
    }

    const Vector3f eU = slit.edgeU().normalized();
    const Vector3f eV = slit.edgeV().normalized();
    const Vector3f normal = eU.cross(eV);
    if (lengthAsDouble(normal) <= 0.0) {
        throw std::invalid_argument("SlitDiffractionField requires non-collinear slit edges");
    }

    return {
        slit.origin() + 0.5f * slit.edgeU() + 0.5f * slit.edgeV(),
        eU,
        eV,
        normal.normalized(),
        widthMm,
        heightMm
    };
}

std::vector<double> activeApertureCoordinates(
    double apertureSizeM,
    double paddedSizeM,
    double sampleSpacingM
) {
    const int halfSamples = static_cast<int>(std::ceil(0.5 * paddedSizeM / sampleSpacingM));
    const double halfAperture = 0.5 * apertureSizeM;
    const double eps = sampleSpacingM * 1.0e-9;

    std::vector<double> coordinates;
    coordinates.reserve(static_cast<std::size_t>(2 * halfSamples + 1));

    for (int index = -halfSamples; index <= halfSamples; ++index) {
        const double coordinate = static_cast<double>(index) * sampleSpacingM;
        if (std::abs(coordinate) <= halfAperture + eps) {
            coordinates.push_back(coordinate);
        }
    }

    return coordinates;
}

QuadratureRule buildFejerFirstRule(int order, double halfWidth) {
    if (order <= 0) {
        throw std::invalid_argument("Fejer quadrature order must be positive");
    }
    if (halfWidth <= 0.0) {
        throw std::invalid_argument("Fejer quadrature half-width must be positive");
    }

    QuadratureRule rule;
    rule.nodes.resize(static_cast<std::size_t>(order));
    rule.weights.resize(static_cast<std::size_t>(order));

    const int highestMode = order / 2;
    for (int i = 0; i < order; ++i) {
        const double theta = kPi * static_cast<double>(2 * i + 1)
                           / static_cast<double>(2 * order);

        double weight = 1.0;
        for (int mode = 1; mode <= highestMode; ++mode) {
            const double denominator = static_cast<double>(4 * mode * mode - 1);
            weight -= 2.0 * std::cos(2.0 * static_cast<double>(mode) * theta) / denominator;
        }

        rule.nodes[static_cast<std::size_t>(i)] = halfWidth * std::cos(theta);
        rule.weights[static_cast<std::size_t>(i)] = halfWidth * (2.0 / static_cast<double>(order)) * weight;
    }

    return rule;
}

std::vector<Complex> buildOneDimensionalSourceSpectrum(
    const std::vector<double>& coordinates,
    const std::vector<double>& frequencyNodes
) {
    std::vector<Complex> values(frequencyNodes.size(), Complex{0.0, 0.0});

    for (std::size_t nodeIndex = 0; nodeIndex < frequencyNodes.size(); ++nodeIndex) {
        const double frequency = frequencyNodes[nodeIndex];
        Complex value{0.0, 0.0};
        for (const double coordinate : coordinates) {
            value += expI(-2.0 * kPi * coordinate * frequency);
        }
        values[nodeIndex] = value;
    }

    return values;
}

Complex rayleighSommerfeldTransfer(double xi, double eta, double z, double k) {
    const double radialFrequencySq = xi * xi + eta * eta;
    const Complex radicand{k * k - 4.0 * kPi * kPi * radialFrequencySq, 0.0};
    return std::exp(Complex{0.0, z} * std::sqrt(radicand));
}

std::vector<Complex> buildWeightedSourceSpectrum(
    const QuadratureRule& quadrature,
    const std::vector<double>& activeX,
    const std::vector<double>& activeY
) {
    const std::vector<Complex> spectrumX = buildOneDimensionalSourceSpectrum(activeX, quadrature.nodes);
    const std::vector<Complex> spectrumY = buildOneDimensionalSourceSpectrum(activeY, quadrature.nodes);

    const std::size_t order = quadrature.nodes.size();
    std::vector<Complex> weighted(order * order, Complex{0.0, 0.0});

    for (std::size_t a = 0; a < order; ++a) {
        for (std::size_t b = 0; b < order; ++b) {
            weighted[a * order + b] =
                quadrature.weights[a] * quadrature.weights[b] * spectrumX[a] * spectrumY[b];
        }
    }

    return weighted;
}

Complex evaluateFieldAtPoint(
    double xM,
    double yM,
    double zM,
    double sampleSpacingM,
    double waveNumber,
    const QuadratureRule& quadrature,
    const std::vector<Complex>& weightedSourceSpectrum
) {
    const std::size_t order = quadrature.nodes.size();
    Complex field{0.0, 0.0};

    // This is Eq. (50) with Eq. (48), evaluated after interchanging the finite
    // source-sample sum and the Fejer tensor quadrature. No periodic FFT/ASM
    // boundary condition is introduced.
    for (std::size_t a = 0; a < order; ++a) {
        const double xi = quadrature.nodes[a];
        for (std::size_t b = 0; b < order; ++b) {
            const double eta = quadrature.nodes[b];
            const Complex transfer = rayleighSommerfeldTransfer(xi, eta, zM, waveNumber);
            const Complex phase = expI(2.0 * kPi * (xM * xi + yM * eta));
            field += weightedSourceSpectrum[a * order + b] * transfer * phase;
        }
    }

    return sampleSpacingM * sampleSpacingM * field;
}

void validateConfig(const SlitDiffractionField::Config& config) {
    if (config.surfaceSamplesU < 2 || config.surfaceSamplesV < 2) {
        throw std::invalid_argument("SlitDiffractionField surface dimensions must be at least 2x2");
    }
    if (config.sourceSampleSpacingMm <= 0.0) {
        throw std::invalid_argument("SlitDiffractionField source sample spacing must be positive");
    }
    if (config.sourcePaddingFactor < 1.0) {
        throw std::invalid_argument("SlitDiffractionField source padding factor must be at least 1");
    }
    if (config.quadratureOrder <= 0) {
        throw std::invalid_argument("SlitDiffractionField quadrature order must be positive");
    }
}

} // namespace

Vector3f SurfacePatch::pointAt(float u, float v) const {
    return origin + u * edgeU + v * edgeV;
}

SlitDiffractionField SlitDiffractionField::from(
    const SlitLight& slit,
    Config config
) {
    SlitDiffractionField field(slit, std::move(config));
    field.precompute();
    return field;
}

float SlitDiffractionField::normalizedWeightAt(
    const Vector3f& worldPoint,
    std::size_t wavelengthIndex
) const {
    const std::vector<float>& map = normalizedSurfaceMap(wavelengthIndex);

    const Vector3f d = worldPoint - m_config.receiverSurface.origin;
    const Vector3f& uEdge = m_config.receiverSurface.edgeU;
    const Vector3f& vEdge = m_config.receiverSurface.edgeV;

    const double a = dotAsDouble(uEdge, uEdge);
    const double b = dotAsDouble(uEdge, vEdge);
    const double c = dotAsDouble(vEdge, vEdge);
    const double r1 = dotAsDouble(d, uEdge);
    const double r2 = dotAsDouble(d, vEdge);
    const double determinant = a * c - b * b;

    if (std::abs(determinant) <= std::numeric_limits<double>::epsilon()) {
        return 0.0f;
    }

    const double u = (r1 * c - r2 * b) / determinant;
    const double v = (r2 * a - r1 * b) / determinant;

    if (u < 0.0 || u > 1.0 || v < 0.0 || v > 1.0) {
        return 0.0f;
    }

    const double mapX = u * static_cast<double>(m_config.surfaceSamplesU - 1);
    const double mapY = v * static_cast<double>(m_config.surfaceSamplesV - 1);

    const int x0 = std::clamp(static_cast<int>(std::floor(mapX)), 0, m_config.surfaceSamplesU - 1);
    const int y0 = std::clamp(static_cast<int>(std::floor(mapY)), 0, m_config.surfaceSamplesV - 1);
    const int x1 = std::min(x0 + 1, m_config.surfaceSamplesU - 1);
    const int y1 = std::min(y0 + 1, m_config.surfaceSamplesV - 1);

    const float tx = static_cast<float>(mapX - static_cast<double>(x0));
    const float ty = static_cast<float>(mapY - static_cast<double>(y0));

    const auto valueAt = [&](int x, int y) -> float {
        return map[static_cast<std::size_t>(y * m_config.surfaceSamplesU + x)];
    };

    const float v00 = valueAt(x0, y0);
    const float v10 = valueAt(x1, y0);
    const float v01 = valueAt(x0, y1);
    const float v11 = valueAt(x1, y1);

    const float row0 = (1.0f - tx) * v00 + tx * v10;
    const float row1 = (1.0f - tx) * v01 + tx * v11;
    return (1.0f - ty) * row0 + ty * row1;
}

const SurfacePatch& SlitDiffractionField::receiverSurface() const {
    return m_config.receiverSurface;
}

int SlitDiffractionField::surfaceSamplesU() const {
    return m_config.surfaceSamplesU;
}

int SlitDiffractionField::surfaceSamplesV() const {
    return m_config.surfaceSamplesV;
}

const std::vector<float>& SlitDiffractionField::normalizedSurfaceMap(
    std::size_t wavelengthIndex
) const {
    if (wavelengthIndex >= m_normalizedMaps.size()) {
        throw std::out_of_range("SlitDiffractionField wavelength index is out of range");
    }

    return m_normalizedMaps[wavelengthIndex];
}

const std::vector<double>& SlitDiffractionField::rawSurfaceIntensityMap(
    std::size_t wavelengthIndex
) const {
    if (wavelengthIndex >= m_rawIntensityMaps.size()) {
        throw std::out_of_range("SlitDiffractionField wavelength index is out of range");
    }

    return m_rawIntensityMaps[wavelengthIndex];
}

const Spectrum& SlitDiffractionField::spectrum() const {
    return m_slit.spectrum();
}

SlitDiffractionField::SlitDiffractionField(
    SlitLight slit,
    Config config
)
    :m_slit{std::move(slit)}
    ,m_config{std::move(config)}
{}

void SlitDiffractionField::precompute() {
    validateConfig(m_config);

    const SlitLocalFrame slitFrame = buildSlitLocalFrame(m_slit);
    const double sampleSpacingM = m_config.sourceSampleSpacingMm * kMmToM;
    const double bandwidthHalfWidth = 1.0 / (2.0 * sampleSpacingM);

    const double apertureWidthM = slitFrame.widthMm * kMmToM;
    const double apertureHeightM = slitFrame.heightMm * kMmToM;
    const double paddedWidthM = apertureWidthM * m_config.sourcePaddingFactor;
    const double paddedHeightM = apertureHeightM * m_config.sourcePaddingFactor;

    const std::vector<double> activeX = activeApertureCoordinates(apertureWidthM, paddedWidthM, sampleSpacingM);
    const std::vector<double> activeY = activeApertureCoordinates(apertureHeightM, paddedHeightM, sampleSpacingM);
    if (activeX.empty() || activeY.empty()) {
        throw std::runtime_error("SlitDiffractionField source grid contains no active aperture samples");
    }

    const QuadratureRule quadrature = buildFejerFirstRule(m_config.quadratureOrder, bandwidthHalfWidth);
    const std::vector<Complex> weightedSourceSpectrum =
        buildWeightedSourceSpectrum(quadrature, activeX, activeY);

    const std::size_t sampleCount =
        static_cast<std::size_t>(m_config.surfaceSamplesU * m_config.surfaceSamplesV);

    m_normalizedMaps.clear();
    m_rawIntensityMaps.clear();
    m_normalizedMaps.reserve(m_slit.spectrum().samples.size());
    m_rawIntensityMaps.reserve(m_slit.spectrum().samples.size());

    for (const Vector2f& spectralSample : m_slit.spectrum().samples) {
        const double wavelengthM = static_cast<double>(spectralSample.x) * kNmToM;
        if (wavelengthM <= 0.0) {
            throw std::invalid_argument("SlitDiffractionField wavelengths must be positive");
        }

        const double waveNumber = 2.0 * kPi / wavelengthM;
        std::vector<double> intensity(sampleCount, 0.0);
        double maximumIntensity = 0.0;

        for (int j = 0; j < m_config.surfaceSamplesV; ++j) {
            const float v = static_cast<float>(j) / static_cast<float>(m_config.surfaceSamplesV - 1);
            for (int i = 0; i < m_config.surfaceSamplesU; ++i) {
                const float u = static_cast<float>(i) / static_cast<float>(m_config.surfaceSamplesU - 1);
                const Vector3f worldPoint = m_config.receiverSurface.pointAt(u, v);
                const Vector3f localOffsetMm = worldPoint - slitFrame.center;

                const double xM = dotAsDouble(localOffsetMm, slitFrame.eU) * kMmToM;
                const double yM = dotAsDouble(localOffsetMm, slitFrame.eV) * kMmToM;
                const double zM = dotAsDouble(localOffsetMm, slitFrame.eN) * kMmToM;

                if (zM <= 0.0) {
                    throw std::runtime_error("SlitDiffractionField receiver surface must lie in front of the slit");
                }

                const Complex field = evaluateFieldAtPoint(
                    xM,
                    yM,
                    zM,
                    sampleSpacingM,
                    waveNumber,
                    quadrature,
                    weightedSourceSpectrum
                );
                const double value = std::norm(field);
                const std::size_t mapIndex = static_cast<std::size_t>(j * m_config.surfaceSamplesU + i);
                intensity[mapIndex] = value;

                if (std::isfinite(value)) {
                    maximumIntensity = std::max(maximumIntensity, value);
                }
            }
        }

        std::vector<float> normalized(sampleCount, 0.0f);
        if (maximumIntensity > 0.0 && std::isfinite(maximumIntensity)) {
            for (std::size_t index = 0; index < intensity.size(); ++index) {
                const double value = intensity[index] / maximumIntensity;
                normalized[index] = static_cast<float>(std::clamp(value, 0.0, 1.0));
            }
        }

        m_rawIntensityMaps.push_back(std::move(intensity));
        m_normalizedMaps.push_back(std::move(normalized));
    }
}
