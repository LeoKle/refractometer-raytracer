#ifndef ILIGHTSOURCE_H
#define ILIGHTSOURCE_H

#include "Spectrum.h"
#include "../vecmath/Vector3.h"
#include "../sampler/ISampler.h"

/// Interface for light sources
class ILightSource {
public:
    virtual ~ILightSource() = default;
    virtual Vector3f samplePoint(ISampler& sampler) = 0;
    [[nodiscard]] virtual const Spectrum& spectrum() const = 0;
};

#endif