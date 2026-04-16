#include <oqmc/pmjbn.h>

#include <iostream>
#include <vector>

int main() {
    std::cout << "Hello, Raytracer!\n";

    int resolution = 512;
    int nSamples = 64;

    // Image buffer (grayscale)
    std::vector<float> image(resolution * resolution, 0.0f);

    // Initialise the sampler cache
    std::vector<char> cache(oqmc::PmjBnSampler::cacheSize);
    oqmc::PmjBnSampler::initialiseCache(cache.data());

    // Loop over all pixels
    for (int y = 0; y < resolution; ++y) {
        for (int x = 0; x < resolution; ++x) {
            float pixelValue = 0.0f;

            // Loop over samples
            for (int index = 0; index < nSamples; ++index) {
                oqmc::PmjBnSampler sampler(x, y, 0, index, cache.data());

                float sample[2];
                sampler.drawSample<2>(sample);

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