#ifndef UTILS_VECTORMATH_BOUNDS3_H
#define UTILS_VECTORMATH_BOUNDS3_H

#include <algorithm>
#include <cmath>

#include "vecmath/Point3.h"
#include "vecmath/Vector3.h"

template <typename T>
struct Bounds3 {
    Point3<T> pMin, pMax;

    Bounds3() : pMin(1e30f, 1e30f, 1e30f), pMax(-1e30f, -1e30f, -1e30f) {}

    Bounds3(const Point3<T>& a, const Point3<T>& b)
        : pMin({std::min(a.x, b.x), std::min(a.y, b.y), std::min(a.z, b.z)}),
          pMax({std::max(a.x, b.x), std::max(a.y, b.y), std::max(a.z, b.z)}) {}

    Vector3<T> diagonal() const { return pMax - pMin; }
    Point3<T> centroid() const { return pMin + 0.5f * diagonal(); }

    Bounds3 expand(const Bounds3& b) const {
        return {{std::min(pMin.x, b.pMin.x), std::min(pMin.y, b.pMin.y), std::min(pMin.z, b.pMin.z)},
                {std::max(pMax.x, b.pMax.x), std::max(pMax.y, b.pMax.y), std::max(pMax.z, b.pMax.z)}};
    }

    bool contains(const Point3<T>& p) const {
        return (p.x >= pMin.x && p.x <= pMax.x) && (p.y >= pMin.y && p.y <= pMax.y) && (p.z >= pMin.z && p.z <= pMax.z);
    }

    bool containsExclusive(const Point3<T>& p) const {
        return (p.x > pMin.x && p.x < pMax.x) && (p.y > pMin.y && p.y < pMax.y) && (p.z > pMin.z && p.z < pMax.z);
    }
};

using Bounds3f = Bounds3<float>;
using Bounds3i = Bounds3<int>;
#endif