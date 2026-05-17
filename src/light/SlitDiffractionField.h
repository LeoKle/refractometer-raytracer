#ifndef SLITDIFFRACTIONFIELD_H
#define SLITDIFFRACTIONFIELD_H

#include <cstddef>
#include <vector>

#include "SlitLight.h"

struct SurfacePatch {
    Vector3f origin;
    Vector3f edgeU;
    Vector3f edgeV;

    [[nodiscard]] Vector3f pointAt(float u, float v) const;
};

class SlitDiffractionField {
public:
    struct Config {
        SurfacePatch receiverSurface;

        int surfaceSamplesU = 128;
        int surfaceSamplesV = 128;

        double sourceSampleSpacingMm = 0.10;
        double sourcePaddingFactor = 2.0;

        int quadratureOrder = 24;
    };

    static SlitDiffractionField from(
        const SlitLight& slit,
        Config config
    );

    [[nodiscard]] float normalizedWeightAt(
        const Vector3f& worldPoint,
        std::size_t wavelengthIndex
    ) const;

    [[nodiscard]] const SurfacePatch& receiverSurface() const;
    [[nodiscard]] int surfaceSamplesU() const;
    [[nodiscard]] int surfaceSamplesV() const;

    [[nodiscard]] const std::vector<float>& normalizedSurfaceMap(
        std::size_t wavelengthIndex
    ) const;

    [[nodiscard]] const std::vector<double>& rawSurfaceIntensityMap(
        std::size_t wavelengthIndex
    ) const;

    [[nodiscard]] const Spectrum& spectrum() const;

private:
    SlitDiffractionField(
        SlitLight slit,
        Config config
    );

    void precompute();

    SlitLight m_slit;
    Config m_config;
    std::vector<std::vector<double>> m_rawIntensityMaps;
    std::vector<std::vector<float>> m_normalizedMaps;
};

#endif
