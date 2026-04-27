#ifndef DETECTOR_PINHOLEDETECTOR_H
#define DETECTOR_PINHOLEDETECTOR_H

#include "IDetector.h"

/// Simple pinhole detector.
///
/// The image plane is defined in world space by three corners,
/// just like the slit light source:
///
/// topLeft ──────── (topLeft + edgeU)
/// │                         │
/// origin ───────── bottomRight
///
/// A ray starts at pinHole and passes through a sampled point on the image plane ("film" in PBRT).
class PinHoleDetector : public IDetector {
public:
    static PinHoleDetector from(
        Vector3f origin,
        Vector3f bottomRight,
        Vector3f topLeft,
        Vector3f pinHole,
        int width,
        int height
    );

    Ray sampleRay(int x, int y, ISampler& sampler) const override;

    [[nodiscard]] int width() const override;
    [[nodiscard]] int height() const override;

private:
    PinHoleDetector(
        Vector3f origin,
        Vector3f edgeU,
        Vector3f edgeV,
        Vector3f pinHole,
        int width,
        int height
    );

    Vector3f m_origin;
    Vector3f m_edgeU;
    Vector3f m_edgeV;
    Vector3f m_pinHole;

    int m_width;
    int m_height;
};

#endif