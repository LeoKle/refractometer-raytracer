#pragma once

#include "../vecmath/Vector2.h"
#include <vector>

/// spectrum[i].x = lambda in nm
/// spectrum[i].y = irradiance in W/qm
struct Spectrum {
    std::vector<Vector2f> samples;
};