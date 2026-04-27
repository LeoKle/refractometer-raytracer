#pragma once

#include <vector>

#include "../vecmath/Vector2.h"

/// samples[i].x = wavelength in nm
/// samples[i].y = spectral irradiance in W/(m^2*nm)
struct Spectrum {
    std::vector<Vector2f> samples;
};

inline const Spectrum DebugSpectrum{
        {
            {430.0f, 0.42f},
            {450.0f, 0.95f},
            {470.0f, 0.68f},
            {500.0f, 0.58f},
            {530.0f, 0.62f},
            {560.0f, 0.66f},
            {590.0f, 0.63f},
            {620.0f, 0.71f},
            {650.0f, 0.90f},
            {680.0f, 0.48f}
        }
};