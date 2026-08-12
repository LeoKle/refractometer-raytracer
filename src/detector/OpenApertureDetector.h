#ifndef DETECTOR_OPENAPERTUREDETECTOR_H
#define DETECTOR_OPENAPERTUREDETECTOR_H

#include "IDetector.h"
#include "Sensor.h"
#include "../light/Spectrum.h"

/// Scene-placed sensor behind a focused circular front opening.
///
/// This models:
///
///     sensor pixel  ->  circular opening  ->  scene
///
/// The focused factory uses world-space units. go5000Mpmcl is legacy local
/// device geometry in millimeters and is not suitable for scene placement.
class OpenApertureDetector : public IDetector {
public:
    static constexpr int NativeWidth  = 2560;
    static constexpr int NativeHeight = 2048;

    struct OpenApertureDebugSample {
        Vector3f sensorPoint;
        Vector3f aperturePoint;
        Vector3f focusPoint;
        Ray ray;
    };

    static OpenApertureDetector focused(
        const Spectrum& sourceSpectrum,
        Vector3f sensorOrigin,
        Vector3f sensorBottomRight,
        Vector3f sensorTopLeft,
        Vector3f apertureCenter,
        float apertureRadius,
        float focusDistance,
        int width,
        int height
    );

    static OpenApertureDetector go5000Mpmcl(
        const Spectrum& sourceSpectrum,
        int width = NativeWidth,
        int height = NativeHeight
    );

    Ray sampleRay(int x, int y, ISampler& sampler) const override;
    OpenApertureDebugSample sampleDebugRay(int x, int y, ISampler& sampler) const;

    [[nodiscard]] Sensor& sensor();
    [[nodiscard]] const Sensor& sensor() const;
    [[nodiscard]] int width() const override;
    [[nodiscard]] int height() const override;

private:
    OpenApertureDetector(
        const Spectrum& sourceSpectrum,
        Vector3f sensorOrigin,
        Vector3f sensorEdgeU,
        Vector3f sensorEdgeV,
        Vector3f apertureCenter,
        float apertureRadius,
        float focusDistance,
        int width,
        int height
    );

    [[nodiscard]] Vector3f sampleSensorPoint(
        int x,
        int y,
        ISampler& sampler
    ) const;

    [[nodiscard]] Vector3f sampleAperturePoint(
        ISampler& sampler
    ) const;

    [[nodiscard]] Vector3f focusPointFor(
        const Vector3f& sensorPoint
    ) const;

    [[nodiscard]] Ray makeRay(
        const Vector3f& aperturePoint,
        const Vector3f& focusPoint
    ) const;

    static constexpr float SensorWidthMm    = 12.8f;
    static constexpr float SensorHeightMm   = 10.24f;
    static constexpr float SensorDistanceMm = 17.526f;

    // Nominal C-mount radius: 25.4 mm diameter / 2.
    static constexpr float ApertureRadiusMm = 12.7f;

    static constexpr float RayOriginEpsilon = 1.0e-5f;
    static constexpr float Pi = 3.14159265358979323846f;

    Sensor m_sensor;
    Spectrum m_launchSpectrum;

    Vector3f m_sensorOrigin;
    Vector3f m_sensorEdgeU;
    Vector3f m_sensorEdgeV;

    Vector3f m_apertureCenter;
    Vector3f m_apertureU;
    Vector3f m_apertureV;
    float m_apertureRadius;

    Vector3f m_viewDirection;
    float m_focusDistance;
};

#endif
