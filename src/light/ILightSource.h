#pragma once

#include "Spectrum.h"
#include "../vecmath/Vector3.h"

/// Interface for light sources
class ILightSource {
public:
    virtual ~ILightSource() = default;
    virtual Vector3f samplePoint() = 0;
    [[nodiscard]] virtual const Spectrum& spectrum() const = 0;
};
