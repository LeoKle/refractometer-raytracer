#ifndef SHAPE_ISHAPE_H
#define SHAPE_ISHAPE_H

#include <optional>
#include <utility>

#include "sampler/ISampler.h"
#include "vecmath/Point3.h"
#include "vecmath/Vector3.h"

struct SurfaceIntersection {
    Point3f point;
    Vector3f normal;
};

class IShape {
public:
    virtual ~IShape() = default;

    virtual std::optional<SurfaceIntersection> intersect(const Point3f& origin, const Vector3f& direction) const = 0;
    virtual Point3f sample(ISampler& sampler) = 0;
};

#endif