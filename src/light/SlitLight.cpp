#include "SlitLight.h"

#include <algorithm>
#include <cmath>
#include <utility>

namespace {

constexpr float kPi = 3.14159265358979323846f;
constexpr float kMmToM = 1.0e-3f;
constexpr float kNmToM = 1.0e-9f;
constexpr double kIntersectionEpsilon = 1.0e-6;

bool isFiniteVector(const Vector3f& value) {
    return std::isfinite(value.x)
        && std::isfinite(value.y)
        && std::isfinite(value.z);
}

double dotAsDouble(const Vector3f& a, const Vector3f& b) {
    return static_cast<double>(a.x) * static_cast<double>(b.x)
         + static_cast<double>(a.y) * static_cast<double>(b.y)
         + static_cast<double>(a.z) * static_cast<double>(b.z);
}

double lengthAsDouble(const Vector3f& value) {
    return std::sqrt(dotAsDouble(value, value));
}

bool safeNormalized(const Vector3f& value, Vector3f& normalized) {
    const double length = lengthAsDouble(value);
    if (!std::isfinite(length) || length <= 0.0) {
        return false;
    }

    const float scale = static_cast<float>(1.0 / length);
    normalized = value * scale;
    return isFiniteVector(normalized);
}

float safeSinc(float x) {
    if (!std::isfinite(x)) return 0.0f;
    if (std::abs(x) < 1.0e-6f) return 1.0f;
    return std::sin(x) / x;
}

float clampUnit(float x) {
    if (!std::isfinite(x)) return 0.0f;
    return std::clamp(x, 0.0f, 1.0f);
}

} // namespace

SlitLight SlitLight::from(
    Vector3f origin,
    Vector3f bottomRight,
    Vector3f topLeft,
    Spectrum   spectrum
) {
    return SlitLight(
        origin,
        bottomRight - origin,
        topLeft     - origin,
        std::move(spectrum)
    );
}

SlitLight::SlitLight(
    Vector3f origin,
    Vector3f edgeU,
    Vector3f edgeV,
    Spectrum   spectrum
)
    :m_origin{origin}
    ,m_edgeU{edgeU}
    ,m_edgeV{edgeV}
    ,m_spectrum{std::move(spectrum)}
{}

Vector3f SlitLight::samplePoint(ISampler& sampler) {
    const auto sample = sampler.next2D();
    const float u = sample[0];
    const float v = sample[1];
    return m_origin + u * m_edgeU + v * m_edgeV;
}

const Spectrum& SlitLight::spectrum() const {
    return m_spectrum;
}

std::optional<LightIntersection> SlitLight::intersect(
    const Point3f& rayOrigin,
    const Vector3f& direction,
    float wavelengthNm
) const {
    Vector3f normal;
    if (!safeNormalized(m_edgeU.cross(m_edgeV), normal)) {
        return std::nullopt;
    }

    const double directionDotNormal = dotAsDouble(direction, normal);
    if (!std::isfinite(directionDotNormal)
        || std::abs(directionDotNormal) <= kIntersectionEpsilon) {
        return std::nullopt;
    }

    const Vector3f originToSlit{
        m_origin.x - rayOrigin.x,
        m_origin.y - rayOrigin.y,
        m_origin.z - rayOrigin.z
    };
    const double t = dotAsDouble(originToSlit, normal) / directionDotNormal;
    if (!std::isfinite(t) || t <= 0.0) {
        return std::nullopt;
    }

    const Point3f point = rayOrigin + static_cast<float>(t) * direction;
    const Vector3f pointOnSlit{
        point.x - m_origin.x,
        point.y - m_origin.y,
        point.z - m_origin.z
    };
    if (!isFiniteVector(pointOnSlit)) {
        return std::nullopt;
    }

    const double edgeUDotU = dotAsDouble(m_edgeU, m_edgeU);
    const double edgeUDotV = dotAsDouble(m_edgeU, m_edgeV);
    const double edgeVDotV = dotAsDouble(m_edgeV, m_edgeV);
    const double pointDotU = dotAsDouble(pointOnSlit, m_edgeU);
    const double pointDotV = dotAsDouble(pointOnSlit, m_edgeV);
    const double determinant = edgeUDotU * edgeVDotV - edgeUDotV * edgeUDotV;
    if (!std::isfinite(determinant) || determinant <= 0.0) {
        return std::nullopt;
    }

    const double u = (pointDotU * edgeVDotV - pointDotV * edgeUDotV) / determinant;
    const double v = (pointDotV * edgeUDotU - pointDotU * edgeUDotV) / determinant;
    if (!std::isfinite(u) || !std::isfinite(v)
        || u < -kIntersectionEpsilon || u > 1.0 + kIntersectionEpsilon
        || v < -kIntersectionEpsilon || v > 1.0 + kIntersectionEpsilon) {
        return std::nullopt;
    }

    return LightIntersection{
        point,
        normal,
        interferenceWeight(
            {rayOrigin.x, rayOrigin.y, rayOrigin.z},
            direction,
            wavelengthNm
        )
    };
}

float SlitLight::interferenceWeight(
    const Vector3f& prismExitPoint,
    const Vector3f& directionTowardSlit,
    float wavelengthNm
) const {
    (void)directionTowardSlit;

    if (!std::isfinite(wavelengthNm) || wavelengthNm <= 0.0f) {
        return 0.0f;
    }

    Vector3f widthAxis;
    if (!safeNormalized(m_edgeU, widthAxis)) {
        return 0.0f;
    }

    const Vector3f normalSource = m_edgeU.cross(m_edgeV);
    Vector3f normal;
    if (!safeNormalized(normalSource, normal)) {
        return 0.0f;
    }

    const Vector3f center = m_origin + 0.5f * m_edgeU + 0.5f * m_edgeV;
    const Vector3f fromSlitToPoint = prismExitPoint - center;
    if (!isFiniteVector(fromSlitToPoint)) {
        return 0.0f;
    }

    const double lateralMm = dotAsDouble(fromSlitToPoint, widthAxis);
    const double normalDistanceMm = std::abs(dotAsDouble(fromSlitToPoint, normal));
    const double denominator = std::sqrt(
        lateralMm * lateralMm + normalDistanceMm * normalDistanceMm
    );
    if (!std::isfinite(denominator) || denominator <= 0.0) {
        return 0.0f;
    }

    const double sinTheta = lateralMm / denominator;
    if (!std::isfinite(sinTheta)) {
        return 0.0f;
    }

    const double slitWidthMeters = lengthAsDouble(m_edgeU) * kMmToM;
    const double wavelengthMeters = static_cast<double>(wavelengthNm) * kNmToM;
    if (!std::isfinite(slitWidthMeters) || slitWidthMeters <= 0.0
        || !std::isfinite(wavelengthMeters) || wavelengthMeters <= 0.0) {
        return 0.0f;
    }

    const float beta = static_cast<float>(
        static_cast<double>(kPi) * slitWidthMeters * sinTheta / wavelengthMeters
    );
    const float sinc = safeSinc(beta);
    return clampUnit(sinc * sinc);
}

const Vector3f& SlitLight::origin() const {
    return m_origin;
}

const Vector3f& SlitLight::edgeU() const {
    return m_edgeU;
}

const Vector3f& SlitLight::edgeV() const {
    return m_edgeV;
}
