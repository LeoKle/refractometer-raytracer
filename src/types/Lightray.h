#ifndef LIGHTRAY_H
#define LIGHTRAY_H

#include "types/Medium.h"
#include "vecmath/Point3.h"
#include "vecmath/Vector3.h"

typedef struct {
    Vector3f direction;
    Point3f origin;
    float wavelength;
    float intensity;
    const Medium* medium = nullptr;
} Lightray;

#endif