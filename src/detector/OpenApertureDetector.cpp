#include "OpenApertureDetector.h"
#include "DetectorQE.h"

#include <cmath>
#include <stdexcept>

namespace {

constexpr float kGeometryEpsilon = 1.0e-8f;
constexpr float kFocusDenomEpsilon = 1.0e-6f;

bool isFinite(const Vector3f& value) {
    return std::isfinite(value.x)
        && std::isfinite(value.y)
        && std::isfinite(value.z);
}

Vector3f normalizedOrThrow(const Vector3f& value, const char* name) {
    if (!isFinite(value)) {
        throw std::invalid_argument(name);
    }

    const float length = value.length();
    if (!std::isfinite(length) || length <= kGeometryEpsilon) {
        throw std::invalid_argument(name);
    }

    const Vector3f normalized = value / length;
    if (!isFinite(normalized)) {
        throw std::invalid_argument(name);
    }

    return normalized;
}

} // namespace

OpenApertureDetector OpenApertureDetector::focused(
    const Spectrum& sourceSpectrum,
    Vector3f sensorOrigin,
    Vector3f sensorBottomRight,
    Vector3f sensorTopLeft,
    Vector3f apertureCenter,
    float apertureRadius,
    float focusDistance,
    int width,
    int height
) {
    return OpenApertureDetector(
        sourceSpectrum,
        sensorOrigin,
        sensorBottomRight - sensorOrigin,
        sensorTopLeft - sensorOrigin,
        apertureCenter,
        apertureRadius,
        focusDistance,
        width,
        height
    );
}

OpenApertureDetector OpenApertureDetector::go5000Mpmcl(
    const Spectrum& sourceSpectrum,
    int width,
    int height
) {
    return OpenApertureDetector::focused(
        sourceSpectrum,
        {
            -0.5f * SensorWidthMm,
            -0.5f * SensorHeightMm,
            -SensorDistanceMm
        },
        {
             0.5f * SensorWidthMm,
            -0.5f * SensorHeightMm,
            -SensorDistanceMm
        },
        {
            -0.5f * SensorWidthMm,
             0.5f * SensorHeightMm,
            -SensorDistanceMm
        },
        {0.0f, 0.0f, 0.0f},
        ApertureRadiusMm,
        SensorDistanceMm,
        width,
        height
    );
}

OpenApertureDetector::OpenApertureDetector(
    const Spectrum& sourceSpectrum,
    Vector3f sensorOrigin,
    Vector3f sensorEdgeU,
    Vector3f sensorEdgeV,
    Vector3f apertureCenter,
    float apertureRadius,
    float focusDistance,
    int width,
    int height
)
    : m_launchSpectrum{
          MultiplySourceSpectrumByDetectorQE(sourceSpectrum)
      }
    , m_sensorOrigin{sensorOrigin}
    , m_sensorEdgeU{sensorEdgeU}
    , m_sensorEdgeV{sensorEdgeV}
    , m_apertureCenter{apertureCenter}
    , m_apertureU{}
    , m_apertureV{}
    , m_apertureRadius{apertureRadius}
    , m_viewDirection{}
    , m_focusDistance{focusDistance}
    , m_width{width}
    , m_height{height}
{
    if (width <= 0) {
        throw std::invalid_argument("OpenApertureDetector width must be positive");
    }
    if (height <= 0) {
        throw std::invalid_argument("OpenApertureDetector height must be positive");
    }
    if (!std::isfinite(apertureRadius) || apertureRadius < 0.0f) {
        throw std::invalid_argument("OpenApertureDetector aperture radius must be non-negative");
    }
    if (!std::isfinite(focusDistance) || focusDistance <= 0.0f) {
        throw std::invalid_argument("OpenApertureDetector focus distance must be positive");
    }
    if (!isFinite(sensorOrigin) || !isFinite(sensorEdgeU) || !isFinite(sensorEdgeV)
        || !isFinite(apertureCenter)) {
        throw std::invalid_argument("OpenApertureDetector geometry must be finite");
    }

    m_apertureU = normalizedOrThrow(
        sensorEdgeU,
        "OpenApertureDetector sensor horizontal edge must be nonzero"
    );

    const Vector3f sensorNormal = normalizedOrThrow(
        sensorEdgeU.cross(sensorEdgeV),
        "OpenApertureDetector sensor plane must be non-degenerate"
    );

    m_apertureV = normalizedOrThrow(
        sensorNormal.cross(m_apertureU),
        "OpenApertureDetector aperture basis must be finite"
    );

    const Vector3f sensorCenter =
        m_sensorOrigin + 0.5f * m_sensorEdgeU + 0.5f * m_sensorEdgeV;
    m_viewDirection = normalizedOrThrow(
        sensorCenter - m_apertureCenter,
        "OpenApertureDetector aperture center must not coincide with sensor center"
    );
}

Vector3f OpenApertureDetector::sampleSensorPoint(
    int x,
    int y,
    ISampler& sampler
) const {
    const auto sample = sampler.next2D();

    const float u =
        (static_cast<float>(x) + sample[0]) / static_cast<float>(m_width);

    const float v =
        (static_cast<float>(y) + sample[1]) / static_cast<float>(m_height);

    return m_sensorOrigin + u * m_sensorEdgeU + v * m_sensorEdgeV;
}

Vector3f OpenApertureDetector::sampleAperturePoint(
    ISampler& sampler
) const {
    const auto sample = sampler.next2D();

    // Uniform area sampling on the circular opening.
    const float r = m_apertureRadius * std::sqrt(sample[0]);
    const float theta = 2.0f * Pi * sample[1];

    return m_apertureCenter
         + r * std::cos(theta) * m_apertureU
         + r * std::sin(theta) * m_apertureV;
}

Vector3f OpenApertureDetector::focusPointFor(
    const Vector3f& sensorPoint
) const {
    const Vector3f chiefDirection = normalizedOrThrow(
        m_apertureCenter - sensorPoint,
        "OpenApertureDetector chief ray direction must be nonzero"
    );

    const float denom = chiefDirection.dot(m_viewDirection);
    if (!std::isfinite(denom) || std::abs(denom) <= kFocusDenomEpsilon) {
        return m_apertureCenter + m_focusDistance * m_viewDirection;
    }

    return m_apertureCenter
         + (m_focusDistance / denom) * chiefDirection;
}

Ray OpenApertureDetector::makeRay(
    const Vector3f& aperturePoint,
    const Vector3f& focusPoint
) const {
    const Vector3f direction = normalizedOrThrow(
        focusPoint - aperturePoint,
        "OpenApertureDetector focused ray direction must be nonzero"
    );

    return Ray{
        aperturePoint + RayOriginEpsilon * direction,
        direction,
        m_launchSpectrum
    };
}

OpenApertureDetector::OpenApertureDebugSample OpenApertureDetector::sampleDebugRay(
    int x,
    int y,
    ISampler& sampler
) const {
    const Vector3f sensorPoint = sampleSensorPoint(x, y, sampler);
    const Vector3f aperturePoint = sampleAperturePoint(sampler);
    const Vector3f focusPoint = focusPointFor(sensorPoint);
    const Ray ray = makeRay(aperturePoint, focusPoint);

    return {sensorPoint, aperturePoint, focusPoint, ray};
}

Ray OpenApertureDetector::sampleRay(
    int x,
    int y,
    ISampler& sampler
) const {
    return sampleDebugRay(x, y, sampler).ray;
}

int OpenApertureDetector::width() const {
    return m_width;
}

int OpenApertureDetector::height() const {
    return m_height;
}
