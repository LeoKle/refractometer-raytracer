#include <iostream>

#include "detector/OpenApertureDetector.h"
#include "light/SlitLight.h"
#include "light/Spectrum.h"
#include "render/Render.h"
#include "shape/Prism.h"
#include "types/Medium.h"

int main() {
    std::cout << "Hello, Raytracer!\n";

    constexpr int resolutionX = 640;
    constexpr int resolutionY = 512;
    constexpr int samplesPerPixel = 1024;

    SellmeierMedium prismMaterial = NBK7;

    const Prism prism(Point3f(0.5f, -0.5f, 0.f), Point3f(1.5f, 1.f, 0.f), Point3f(2.5f, -0.5f, 0.f), 0.3f, prismMaterial);
    const SlitLight slitLight =
        SlitLight::from(Vector3f(0.0f, 0.0f, 0.0f), Vector3f(0.0f, 3.0f, 0.0f), Vector3f(0.0f, 0.0f, 3.0f), Le_uw_E3B);

    const auto detectorOrigin = Vector3f(5.f, -0.5f, 0.15f);
    const auto detectorNormal = Vector3f(1.5f, -0.5f, 0.f).normalized();
    const auto detectorBottomRight = Vector3f(4.5f, -1.5f, 0.15f);
    const auto detectorTopLeft = detectorOrigin + Vector3f(0.f, 0.f, 0.15f);
    const auto focalLength = 0.15f;
    const auto apertureCenter = 0.5f * (detectorBottomRight + detectorTopLeft) + detectorNormal * focalLength;

    const auto focusTarget = Vector3f(1.5f, 0.0f, 0.15f);
    const float focusDistance = (focusTarget - apertureCenter).length();
    const float apertureRadius = 0.0127f;

    OpenApertureDetector detector =
        OpenApertureDetector::focused(slitLight.spectrum(), detectorOrigin, detectorBottomRight, detectorTopLeft,
                                      apertureCenter, apertureRadius, focusDistance, resolutionX, resolutionY);

    render(detector, prism, prismMaterial, slitLight, samplesPerPixel);
    detector.sensor().writeImage("image.bin");

    std::cout << "Rendering complete.\n";

    return 0;
}
