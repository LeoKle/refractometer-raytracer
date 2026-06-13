#ifndef ILIGHTSOURCE_H
#define ILIGHTSOURCE_H

#include <optional>

#include "Spectrum.h"
#include "../vecmath/Vector3.h"
#include "../vecmath/Point3.h"
#include "../sampler/ISampler.h"

struct LightIntersection {
    Point3f point;
    Vector3f normal;
    float interferenceWeight;
};

/// Interface for light sources
class ILightSource {
public:
    virtual ~ILightSource() = default;
    virtual Vector3f samplePoint(ISampler& sampler) = 0;
    [[nodiscard]] virtual const Spectrum& spectrum() const = 0;
    [[nodiscard]] virtual std::optional<LightIntersection> intersect(
        const Point3f& origin,
        const Vector3f& direction,
        float wavelengthNm
    ) const = 0;
};

#endif
