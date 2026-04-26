#ifndef SLITLIGHT_H
#define SLITLIGHT_H

#include "ILightSource.h"

/// Geometry is defined by three corner points in world space (mm):
///
///   topLeft ──────── (topLeft + edgeU)
///      │                    │
///   origin  ──────── bottomRight
///
/// edgeU = bottomRight − origin   (along width)
/// edgeV = topLeft     − origin   (along height)
///
/// A random point on the slit is:   origin + u·edgeU + v·edgeV,  u,v ∈ [0,1]

class SlitLight : public ILightSource {
public:
    /// @param origin       Bottom-left corner
    /// @param bottomRight
    /// @param topLeft
    /// @param spectrum

    static SlitLight from(
        Vector3f origin,
        Vector3f bottomRight,
        Vector3f topLeft,
        Spectrum spectrum
    );

    Vector3f samplePoint(ISampler& sampler) override;

    const Spectrum& spectrum() const override;

private:
    SlitLight(
        Vector3f origin,
        Vector3f edgeU,
        Vector3f edgeV,
        Spectrum spectrum
    );

    Vector3f m_origin;
    Vector3f m_edgeU;
    Vector3f m_edgeV;

    Spectrum m_spectrum;

};

#endif