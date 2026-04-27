#include <vector>

#include "light/Spectrum.h"

struct SpectrumSampler {
    std::vector<float> cdf;
    const Spectrum *spec;

    SpectrumSampler(const Spectrum &s) : spec(&s) {
        float sum = 0.0f;
        for (auto &p : s.samples) sum += p[1];

        float running = 0.0f;
        for (auto &p : s.samples) {
            running += p[1] / sum;
            cdf.push_back(running);
        }
    }

    float sample(float u) const {
        int i = std::lower_bound(cdf.begin(), cdf.end(), u) - cdf.begin();
        i = std::min(i, (int)spec->samples.size() - 1);
        return spec->samples[i][0];  // wavelength
    }

    float pdf(float lambda) const {
        // piecewise constant PDF (simplest version)
        float sum = 0.0f;
        for (auto &p : spec->samples) sum += p[1];
        return 1.0f / spec->samples.size();  // or weighted version if needed
    }
};