#ifndef DETECTOR_OPENAPERTUREDETECTOR_H
#define DETECTOR_OPENAPERTUREDETECTOR_H

#include "IDetector.h"
#include "../light/Spectrum.h"

/// Bare sensor behind a circular front opening.
///
/// This models:
///
///     sensor pixel  ->  circular opening  ->  scene
///
/// Rays that would hit the metal cylinder/front mask are not generated.
/// Equivalently, we sample only the rays that survive the aperture test.
///
/// Units: millimeters.
class OpenApertureDetector : public IDetector {
public:
    static constexpr int NativeWidth  = 2560;
    static constexpr int NativeHeight = 2048;

    static OpenApertureDetector go5000Mpmcl(
        const Spectrum& sourceSpectrum,
        int width = NativeWidth,
        int height = NativeHeight
    );

    Ray sampleRay(int x, int y, ISampler& sampler) const override;

    [[nodiscard]] int width() const override;
    [[nodiscard]] int height() const override;

private:
    OpenApertureDetector(
        const Spectrum& sourceSpectrum,
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

    static constexpr float SensorWidthMm    = 12.8f;
    static constexpr float SensorHeightMm   = 10.24f;
    static constexpr float SensorDistanceMm = 17.526f;

    // Nominal C-mount radius: 25.4 mm diameter / 2.
    static constexpr float ApertureRadiusMm = 12.7f;

    static constexpr float RayOriginEpsilonMm = 1.0e-4f;
    static constexpr float Pi = 3.14159265358979323846f;

    Spectrum m_launchSpectrum;

    Vector3f m_sensorOrigin;
    Vector3f m_sensorEdgeU;
    Vector3f m_sensorEdgeV;

    Vector3f m_apertureCenter;
    Vector3f m_apertureU;
    Vector3f m_apertureV;

    int m_width;
    int m_height;
};

#endif
