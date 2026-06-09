#include <chrono>
#include <fstream>
#include <iostream>
#include <vector>

#include "detector/PinHoleDetector.h"
#include "light/SlitLight.h"
#include "light/Spectrum.h"
#include "physics/Snell.h"
#include "sampler/ISampler.h"
#include "sampler/Sampler.h"
#include "shape/Prism.h"
#include "types/Medium.h"

constexpr float kEpsilon = 1e-5f;

inline std::optional<Ray> refractThroughSurface(const IShape& shape, const Ray& incoming, float n1, float n2) {
    // FIXME: explicit conversion Vector3 to Point3 needed
    const auto rayOrigin = Point3f({incoming.origin.x, incoming.origin.y, incoming.origin.z});
    const auto offsetOrigin = rayOrigin + Vector3f(incoming.direction.x * kEpsilon, incoming.direction.y * kEpsilon,
                                                   incoming.direction.z * kEpsilon);
    const auto hit = shape.intersect(offsetOrigin, incoming.direction);

    if (!hit) return std::nullopt;

    auto normal = hit.value().normal;

    const auto refracted = SnellLaw(n1, n2, incoming.direction, normal);
    if (!refracted) return std::nullopt;

    // FIXME: explicit conversion Point3 to Vector3 needed
    const auto point = Vector3f({hit->point.x, hit->point.y, hit->point.z});

    return Ray{point, refracted.value()};
}

constexpr float kRefractiveIndexAir = 1.000277f;

int main() {
    std::cout << "Hello, Raytracer!\n";

    std::ofstream intersections1("intersections1.csv");
    intersections1 << "x,y,z\n";
    std::ofstream intersections2("intersections2.csv");
    intersections2 << "x,y,z\n";

    int resolution = 64;
    int nSamples = 12;

    // Image buffer (grayscale)
    std::vector<float> image(resolution * resolution, 0.0f);

    // Initialise the sampler cache
    auto cache = OqmcPmjBnSampler::createCache();

    SellmeierMedium prismMaterial = NBK7;
    Prism prism(Point3f(0.5f, -0.5f, 0.f), Point3f(1.5f, 1.f, 0.f), Point3f(2.5f, -0.5f, 0.f), 0.3f, prismMaterial);
    SlitLight slitLight = SlitLight::from(Vector3f(0.0f, 0.0f, 0.0f), Vector3f(0.0f, 0.05f, 0.0f),
                                          Vector3f(0.0f, 0.0f, 0.5f), DebugSpectrum);

    const auto detectorOrigin = Vector3f(5.f, -0.5f, 0.15f);
    const auto detectorNormal = Vector3f(1.5f, -0.5f, 0.f).normalized();
    const auto detectorBottomRight = Vector3f(4.5f, -1.5f, 0.15f);
    const auto detectorTopLeft = detectorOrigin + Vector3f(0.f, 0.f, 0.15f);
    const auto focalLength = 0.15f;

    PinHoleDetector detector = PinHoleDetector::from(
        detectorOrigin,       // origin
        detectorBottomRight,  // bottomRight
        detectorTopLeft,      // topLeft
        detectorOrigin + detectorNormal * focalLength + detectorBottomRight * 0.5f, resolution, resolution);

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

                const auto detectorRay = detector.sampleRay(x, y, *detectorSampler);

                const float wavelength = 550.f * 10e-3;  // micrometers // TODO: take wavelength from detector sample

                // intersection 1: entry into prism (air -> glass)
                const auto refractive_index = prismMaterial.refractiveIndex(wavelength);
                const auto refracted1 =
                    refractThroughSurface(prism, detectorRay, kRefractiveIndexAir,
                                          refractive_index);  // TODO: correct refractive index calculation
                if (!refracted1) continue;

                // std::cout << "HIT";

                // intersection 2: exit from prism (glass -> air)
                const auto refracted2 =
                    refractThroughSurface(prism, *refracted1, refractive_index,
                                          kRefractiveIndexAir);  // TODO: correct refractive index calculation
                if (!refracted2) continue;

                // TODO: check for intersection with lightsource

                const auto& p1 = refracted1->origin;
                intersections1 << p1.x << "," << p1.y << "," << p1.z << "\n";
                const auto& p2 = refracted2->origin;
                intersections2 << p2.x << "," << p2.y << "," << p2.z << "\n";

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