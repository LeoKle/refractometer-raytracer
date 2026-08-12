#include "Render.h"

#include <optional>

#include "detector/OpenApertureDetector.h"
#include "light/SlitLight.h"
#include "physics/Snell.h"
#include "sampler/ISampler.h"
#include "sampler/Sampler.h"
#include "shape/Prism.h"
#include "types/Medium.h"

namespace {

constexpr float kEpsilon = 1e-5f;
constexpr float kRefractiveIndexAir = 1.000277f;

std::optional<Ray> refractThroughSurface(
    const IShape& shape,
    const Ray& incoming,
    float n1,
    float n2
) {
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

    return Ray{point, refracted.value(), incoming.spectrum};
}

} // namespace

void render(
    OpenApertureDetector& detector,
    const Prism& prism,
    const SellmeierMedium& prismMaterial,
    const SlitLight& slitLight,
    int samplesPerPixel
) {
    auto cache = OqmcPmjBnSampler::createCache();

    for (int y = 0; y < detector.height(); ++y) {
        for (int x = 0; x < detector.width(); ++x) {
            for (int index = 0; index < samplesPerPixel; ++index) {
                float sampleValue = 0.0f;

                OqmcPmjBnSampler baseSampler(x, y, 0, index, cache);
                auto detectorSampler = baseSampler.split(ISampler::DomainKey::Detector);

                const auto detectorRay = detector.sampleRay(x, y, *detectorSampler);

                for (const SpectralSample& sample : detectorRay.spectrum.samples) {
                    const float wavelengthNm = sample.wavelengthNm;
                    const float intensity = sample.intensity;

                    const auto refractiveIndex =
                        prismMaterial.refractiveIndex(wavelengthNm * 0.001);  // FIXME: micrometers conversion
                    const auto refracted1 =
                        refractThroughSurface(prism, detectorRay, kRefractiveIndexAir, refractiveIndex);
                    if (!refracted1) continue;

                    const auto refracted2 =
                        refractThroughSurface(prism, *refracted1, refractiveIndex, kRefractiveIndexAir);
                    if (!refracted2) continue;

                    const auto intersectionPoint =
                        Point3f({refracted2->origin.x, refracted2->origin.y, refracted2->origin.z});

                    const auto lightIntersection =
                        slitLight.intersect(intersectionPoint, refracted2->direction, wavelengthNm);

                    if (lightIntersection) {
                        sampleValue += intensity * lightIntersection->interferenceWeight;
                    }
                }

                detector.sensor().addSample(x, y, sampleValue);
            }
        }
    }
}
