#include <chrono>
#include <fstream>
#include <iostream>
#include <vector>

#include "detector/PinHoleDetector.h"
#include "light/SlitLight.h"
#include "light/Spectrum.h"
#include "sampler/ISampler.h"
#include "sampler/Sampler.h"
#include "shape/Prism.h"
#include "types/Medium.h"

int main() {
    std::cout << "Hello, Raytracer!\n";

    int resolution = 512;
    int nSamples = 64;

    // Image buffer (grayscale)
    std::vector<float> image(resolution * resolution, 0.0f);

    // Initialise the sampler cache
    auto cache = OqmcPmjBnSampler::createCache();

    SellmeierMedium prismMaterial = NBK7;
    Prism prism(Point3f(0.f, 0.f, 0.f), Point3f(1.f, 0.f, 0.f), Point3f(0.5f, 1.f, 0.f), 2.f, prismMaterial);

    SlitLight slitLight = SlitLight::from(
        Vector3f(-0.5f, -0.5f, -1.0f),
        Vector3f(0.5f, -0.5f, -1.0f),
        Vector3f(-0.5f, 0.5f, -1.0f),
        HeNeSpectrum
    );

    OpenApertureDetector detector = OpenApertureDetector::go5000Mpmcl(slitLight.spectrum(), 640, 512);

    // Image buffer (grayscale)
    std::vector<float> image(detector.width() * detector.height(), 0.0f);

    // Loop over all pixels
    for (int y = 0; y < resolution; ++y) {
        for (int x = 0; x < resolution; ++x) {
            float pixelValue = 0.0f;

            // Loop over samples
            for (int index = 0; index < nSamples; ++index) {
                auto start = std::chrono::high_resolution_clock::now();
                // root domain (per pixel + sample)
                OqmcPmjBnSampler baseSampler(x, y, 0, index, cache);
                auto detectorSampler = baseSampler.split(ISampler::DomainKey::Detector);

                const auto ray = detector.sampleRay(x, y, *detectorSampler);

                //  shape intersection
                const auto rayOrigin = Point3f(
                    {ray.origin.x, ray.origin.y, ray.origin.z});  // FIXME: explicit conversion Vector3 to Point3
                const auto intersection = prism.intersect(rayOrigin, ray.direction);

                if (!intersection.has_value()) {
                    break;
                }
                const auto intersection_point = intersection.value().point;
                const auto intersection_normal = intersection.value().normal;

                // More complex E2E test than the circle
                if (x == 0 && y == 0 && index == 0) {
                    std::cout << "First detector ray origin: " << ray.origin.x << ", " << ray.origin.y << ", "
                              << ray.origin.z << "\n";

                    std::cout << "First detector ray direction: " << ray.direction.x << ", " << ray.direction.y << ", "
                              << ray.direction.z << "\n";

                    std::cout << "First shape intersetion: " << intersection_point.x << ", " << intersection_point.y
                              << ", " << intersection_point.z << "\n";
                }

                for (const Vector2f& spectralSample : ray.spectrum.samples) {
                    const auto lightIntersection = slitLight.intersect(
                        intersection_point,
                        ray.direction,
                        spectralSample.x
                    );
                    if (lightIntersection.has_value()) {
                        pixelValue += spectralSample.y * lightIntersection->interferenceWeight;
                    }
                }

                auto end = std::chrono::high_resolution_clock::now();
                auto total_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
                // std::cout << "time per sample: " << total_ns << " ns" << "\n";
            }

            // normalize
            image[y * detector.width() + x] = pixelValue / nSamples;
        }
    }

    std::cout << "Rendering complete.\n";

    return 0;
}
