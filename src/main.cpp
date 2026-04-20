#include <iostream>
#include <vector>

#include "sampler/ISampler.h"
#include "sampler/Sampler.h"

int main() {
    std::cout << "Hello, Raytracer!\n";

    int resolution = 512;
    int nSamples = 64;

    // Image buffer (grayscale)
    std::vector<float> image(resolution * resolution, 0.0f);

    // Initialise the sampler cache
    auto cache = OqmcPmjBnSampler::createCache();

    // Loop over all pixels
    for (int y = 0; y < resolution; ++y) {
        for (int x = 0; x < resolution; ++x) {
            float pixelValue = 0.0f;

            // Loop over samples
            for (int index = 0; index < nSamples; ++index) {
                // root domain (per pixel + sample)
                OqmcPmjBnSampler pixelSampler(x, y, 0, index, cache);

                // create a subdomain for pixel sampling
                auto sampleDomain = pixelSampler.split(ISampler::DomainKey::Light);

                // Draw sample (2D)
                auto sample = sampleDomain->next2D();

                float xOffset = x + sample[0];
                float yOffset = y + sample[1];

                // simple test: circle centered at origin
                if (xOffset * xOffset + yOffset * yOffset < resolution * resolution) {
                    pixelValue += 1.0f;
                }
            }

            // normalize
            image[y * resolution + x] = pixelValue / nSamples;
        }
    }

    std::cout << "Rendering complete.\n";

    // dump a few values to verify it's working
    std::cout << "Sample pixels:\n";
    for (int i = 0; i < 10; ++i) {
        std::cout << image[i] << "\n";
    }

    return 0;
}