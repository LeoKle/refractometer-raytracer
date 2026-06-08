#ifndef PHYSICS_SNELL_H
#define PHYSICS_SNELL_H

#include <cmath>

#include "vecmath/Vector3.h"

inline const Vector3f SnellLaw(float n1, float n2, Vector3f r0, Vector3f n) {
    const float n12 = n1 / n2;
    const float ndotr0 = n.dot(r0);

    const auto term1 = n12 * r0;
    const float term2 = n12 * ndotr0;
    const float term3 = 1.0f - n12 * n12 * (1.0f - ndotr0 * ndotr0);

    return term1 - n * (term2 - std::sqrt(term3));
};

#endif