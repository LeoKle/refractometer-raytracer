#include <chrono>
#include <fstream>
#include <iostream>
#include <vector>

#include "sampler/ISampler.h"
#include "sampler/Sampler.h"
#include "shape/Prism.h"

#include "light/SlitLight.h"
#include "light/Spectrum.h"

#include "detector/PinHoleDetector.h"

int main() {
    std::cout << "Hello, Raytracer!\n";

    std::ofstream prismSamples("prismSamples.csv");
    prismSamples << "x,y,z\n";

    int resolution = 512;
    int nSamples = 64;

    // Image buffer (grayscale)
    std::vector<float> image(resolution * resolution, 0.0f);

    // Initialise the sampler cache
    auto cache = OqmcPmjBnSampler::createCache();

    Prism prism(Point3f(0.f, 0.f, 0.f), Point3f(1.f, 0.f, 0.f), Point3f(0.5f, 1.f, 0.f), 2.f);
    SlitLight slitLight = SlitLight::from(Vector3f(-0.5f, -0.5f, -1.0f), Vector3f( 0.5f, -0.5f, -1.0f), Vector3f(-0.5f,  0.5f, -1.0f), DebugSpectrum);
    PinHoleDetector detector = PinHoleDetector::from(Vector3f(-0.5f, -0.5f, 1.0f), Vector3f( 0.5f, -0.5f, 1.0f), Vector3f(-0.5f,  0.5f, 1.0f), Vector3f( 0.0f,  0.0f, 0.0f), resolution, resolution);

    // Loop over all pixels
    for (int y = 0; y < resolution; ++y) {
        for (int x = 0; x < resolution; ++x) {
            float pixelValue = 0.0f;

            // Loop over samples
            for (int index = 0; index < nSamples; ++index) {
                auto start = std::chrono::high_resolution_clock::now();
                // root domain (per pixel + sample)
                OqmcPmjBnSampler baseSampler(x, y, 0, index, cache);
                auto shapeSampler = baseSampler.split(ISampler::DomainKey::Shape);
                auto lightSampler = baseSampler.split(ISampler::DomainKey::Light);
                auto detectorSampler = baseSampler.split(ISampler::DomainKey::Detector);

                // sample points on the shape
                const auto prismPoint = prism.sample(*shapeSampler);
                // std::cout << prismPoint[0] << "," << prismPoint[1] << "," << prismPoint[2] << "\n";
                // prismSamples << prismPoint[0] << "," << prismPoint[1] << "," << prismPoint[2] << "\n";
                const auto lightPoint = slitLight.samplePoint(*lightSampler);
                const auto ray = detector.sampleRay(x, y, *detectorSampler);

                // Draw sample (2D)
                auto sample = lightSampler->next2D();

                float xOffset = x + sample[0];
                float yOffset = y + sample[1];

                ///More complex E2E test than the circle
                if (x == 0 && y == 0 && index == 0) {
                    std::cout << "First prism sample: "
                              << prismPoint.x << ", "
                              << prismPoint.y << ", "
                              << prismPoint.z << "\n";

                    std::cout << "First light sample: "
                              << lightPoint.x << ", "
                              << lightPoint.y << ", "
                              << lightPoint.z << "\n";

                    std::cout << "First detector ray origin: "
                              << ray.origin.x << ", "
                              << ray.origin.y << ", "
                              << ray.origin.z << "\n";

                    std::cout << "First detector ray direction: "
                              << ray.direction.x << ", "
                              << ray.direction.y << ", "
                              << ray.direction.z << "\n";
                }

                prismSamples << prismPoint.x << ","
                             << prismPoint.y << ","
                             << prismPoint.z << "\n";

                if (ray.direction.z > 0.0f) {
                    pixelValue += 1.0f;
                }

                auto end = std::chrono::high_resolution_clock::now();
                auto total_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
                std::cout << "time per sample: " << total_ns << " ns" << "\n";
            }

            // normalize
            image[y * detector.width() + x] = pixelValue / nSamples;
        }
    }

    std::cout << "Rendering complete.\n";

    return 0;
}