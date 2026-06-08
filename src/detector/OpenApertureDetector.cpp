#include "OpenApertureDetector.h"
#include "DetectorQE.h"

#include <cmath>

OpenApertureDetector OpenApertureDetector::go5000Mpmcl(
    const ILightSource& lightSource,
    int width,
    int height
) {
    return OpenApertureDetector(lightSource, width, height);
}

OpenApertureDetector::OpenApertureDetector(
    const ILightSource& lightSource,
    int width,
    int height
)
    : m_launchSpectrum{
          MultiplySourceSpectrumByDetectorQE(lightSource.spectrum())
      }
    , m_sensorOrigin{
          -0.5f * SensorWidthMm,
          -0.5f * SensorHeightMm,
          -SensorDistanceMm
      }
    , m_sensorEdgeU{SensorWidthMm, 0.0f, 0.0f}
    , m_sensorEdgeV{0.0f, SensorHeightMm, 0.0f}
    , m_apertureCenter{0.0f, 0.0f, 0.0f}
    , m_apertureU{1.0f, 0.0f, 0.0f}
    , m_apertureV{0.0f, 1.0f, 0.0f}
    , m_width{width}
    , m_height{height}
{}

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
    const float r = ApertureRadiusMm * std::sqrt(sample[0]);
    const float theta = 2.0f * Pi * sample[1];

    return m_apertureCenter
         + r * std::cos(theta) * m_apertureU
         + r * std::sin(theta) * m_apertureV;
}

Ray OpenApertureDetector::sampleRay(
    int x,
    int y,
    ISampler& sampler
) const {
    const Vector3f pSensor = sampleSensorPoint(x, y, sampler);
    const Vector3f pAperture = sampleAperturePoint(sampler);

    // This is the ray that leaves the sensor and passes through the opening.
    const Vector3f direction = (pAperture - pSensor).normalized();

    // Start just outside the detector body so scene intersections do not
    // immediately hit the aperture plane again.
    return Ray{
        pAperture + RayOriginEpsilonMm * direction,
        direction,
        m_launchSpectrum
    };
}

int OpenApertureDetector::width() const {
    return m_width;
}

int OpenApertureDetector::height() const {
    return m_height;
}