#ifndef DETECTOR_IDETECTOR_H
#define DETECTOR_IDETECTOR_H

#include "../sampler/ISampler.h"
#include "../vecmath/Vector3.h"
#include "../light/Spectrum.h"

struct Ray {
    Vector3f origin;
    Vector3f direction;
    Spectrum spectrum{{{550.0f, 1.0f}}};
};

/// BACKWARD raytracer detector interface.
class IDetector {
public:
    virtual ~IDetector() = default;

    virtual Ray sampleRay(int x, int y, ISampler& sampler) const = 0;

    [[nodiscard]] virtual int width() const = 0;
    [[nodiscard]] virtual int height() const = 0;
};

#endif