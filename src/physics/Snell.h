#ifndef PHYSICS_SNELL_H
#define PHYSICS_SNELL_H

#include <cmath>
#include <optional>

#include "vecmath/Vector3.h"

const inline std::optional<Vector3f> SnellLaw(float n1, float n2, Vector3f incident, Vector3f normal) {
    // normal is outward-facing; cosI must be positive so negate the dot
    const float eta = n1 / n2;
    const float cosI = -normal.dot(incident);  // > 0 for entering ray
    const float sin2T = eta * eta * (1.0f - cosI * cosI);

    if (sin2T > 1.0f) return std::nullopt;  // total internal reflection

    const float cosT = std::sqrt(1.0f - sin2T);
    return eta * incident + (eta * cosI - cosT) * normal;
}

#endif