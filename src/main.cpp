#include <fstream>
#include <iostream>
#include <vector>

#include "light/SlitLight.h"
#include "sampler/ISampler.h"
#include "sampler/Sampler.h"
#include "sampler/SpectrumSampler.h"
#include "shape/Prism.h"

int main() {
    std::cout << "Hello, Raytracer!\n";

    std::ofstream prismSamples("prismSamples.csv");
    prismSamples << "x,y,z\n";
    std::ofstream lightSamples("lightSamples.csv");
    lightSamples << "x,y,z\n";

    int resolution = 32;
    int nSamples = 16;

    // Image buffer (grayscale)
    std::vector<float> image(resolution * resolution, 0.0f);

    // Initialise the sampler cache
    auto cache = OqmcPmjBnSampler::createCache();

    auto light =
        SlitLight::from(Vector3f(0.f, 0.f, 0.f), Vector3f(0.f, 1.f, 0.f), Vector3f(0.f, 0.f, 1.f), DebugSpectrum);
    Prism prism(Point3f(0.f, 0.f, 0.f), Point3f(1.f, 0.f, 0.f), Point3f(0.5f, 1.f, 0.f), 2.f);

    const auto spectrum = light.spectrum();
    const auto spectrumSampler = SpectrumSampler(spectrum);

    // Loop over all pixels
    for (int y = 0; y < resolution; ++y) {
        for (int x = 0; x < resolution; ++x) {
            float pixelValue = 0.0f;

            // Loop over samples
            for (int index = 0; index < nSamples; ++index) {
                // root domain (per pixel + sample)
                OqmcPmjBnSampler baseSampler(x, y, 0, index, cache);
                auto shapeSampler = baseSampler.split(ISampler::DomainKey::Shape);
                auto lightSampler = baseSampler.split(ISampler::DomainKey::Light);
                auto wavelengthSampler = baseSampler.split(ISampler::DomainKey::Wavelength);

                // sample points on the shape
                const auto prismPoint = prism.sample(*shapeSampler);
                // std::cout << prismPoint[0] << "," << prismPoint[1] << "," << prismPoint[2] << "\n";
                // prismSamples << prismPoint[0] << "," << prismPoint[1] << "," << prismPoint[2] << "\n";

                const auto lightPoint = light.samplePoint(*lightSampler);
                // std::cout << lightPoint[0] << "," << lightPoint[1] << "," << lightPoint[2] << "\n";
                // lightSamples << lightPoint[0] << "," << lightPoint[1] << "," << lightPoint[2] << "\n";

                float u = wavelengthSampler->next1D();
                const float lambda = spectrumSampler.sample(u);
                // float emission = spectrum.evaluate(lambda);
            }

            // normalize
            image[y * resolution + x] = pixelValue / nSamples;
        }
    }

    std::cout << "Rendering complete.\n";

    return 0;
}