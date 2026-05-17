#include <gtest/gtest.h>

#include <algorithm>
#include <cmath>

#include "light/SlitDiffractionField.h"

namespace {

constexpr float kEps = 1.0e-5f;

bool vecNear(const Vector3f& a, const Vector3f& b, float eps = kEps) {
    return std::abs(a.x - b.x) <= eps
        && std::abs(a.y - b.y) <= eps
        && std::abs(a.z - b.z) <= eps;
}

Spectrum singleGreenSample() {
    return Spectrum{{{550.0f, 1.0f}}};
}

SlitLight smallCenteredSlit() {
    return SlitLight::from(
        {-0.5f, -0.25f, 0.0f},
        { 0.5f, -0.25f, 0.0f},
        {-0.5f,  0.25f, 0.0f},
        singleGreenSample()
    );
}

SlitDiffractionField::Config fastConfig() {
    SlitDiffractionField::Config config;
    config.receiverSurface = {
        {-1.0f, -1.0f, 20.0f},
        { 2.0f,  0.0f,  0.0f},
        { 0.0f,  2.0f,  0.0f}
    };
    config.surfaceSamplesU = 5;
    config.surfaceSamplesV = 5;
    config.sourceSampleSpacingMm = 0.25;
    config.sourcePaddingFactor = 2.0;
    config.quadratureOrder = 6;
    return config;
}

} // namespace

TEST(SurfacePatch, PointAtReturnsPatchCorners) {
    const SurfacePatch patch{
        {1.0f, 2.0f, 3.0f},
        {4.0f, 0.0f, 1.0f},
        {0.0f, 5.0f, 2.0f}
    };

    EXPECT_TRUE(vecNear(patch.pointAt(0.0f, 0.0f), patch.origin));
    EXPECT_TRUE(vecNear(patch.pointAt(1.0f, 0.0f), patch.origin + patch.edgeU));
    EXPECT_TRUE(vecNear(patch.pointAt(0.0f, 1.0f), patch.origin + patch.edgeV));
    EXPECT_TRUE(vecNear(patch.pointAt(1.0f, 1.0f), patch.origin + patch.edgeU + patch.edgeV));
}

TEST(SlitDiffractionField, SurfaceMapIsFiniteAndNormalized) {
    const SlitDiffractionField field = SlitDiffractionField::from(
        smallCenteredSlit(),
        fastConfig()
    );

    const std::vector<float>& map = field.normalizedSurfaceMap(0);
    const std::vector<double>& rawMap = field.rawSurfaceIntensityMap(0);
    ASSERT_EQ(map.size(), 25);
    ASSERT_EQ(rawMap.size(), 25);

    float maximum = 0.0f;
    for (std::size_t index = 0; index < map.size(); ++index) {
        const float value = map[index];
        EXPECT_TRUE(std::isfinite(rawMap[index]));
        EXPECT_GE(rawMap[index], 0.0);

        EXPECT_TRUE(std::isfinite(value));
        EXPECT_GE(value, 0.0f);
        EXPECT_LE(value, 1.0f);
        maximum = std::max(maximum, value);
    }

    EXPECT_NEAR(maximum, 1.0f, kEps);
}

TEST(SlitDiffractionField, LookupAtExactGridNodeMatchesStoredMap) {
    SlitDiffractionField::Config config = fastConfig();
    config.surfaceSamplesU = 6;
    config.surfaceSamplesV = 5;

    const SlitDiffractionField field = SlitDiffractionField::from(
        smallCenteredSlit(),
        config
    );

    const int i = 3;
    const int j = 2;
    const float u = static_cast<float>(i) / static_cast<float>(field.surfaceSamplesU() - 1);
    const float v = static_cast<float>(j) / static_cast<float>(field.surfaceSamplesV() - 1);
    const Vector3f point = field.receiverSurface().pointAt(u, v);

    const std::vector<float>& map = field.normalizedSurfaceMap(0);
    const float stored = map[static_cast<std::size_t>(j * field.surfaceSamplesU() + i)];
    const float lookedUp = field.normalizedWeightAt(point, 0);

    EXPECT_NEAR(lookedUp, stored, 1.0e-4f);
}

TEST(SlitDiffractionField, OutOfPatchLookupReturnsZero) {
    const SlitDiffractionField field = SlitDiffractionField::from(
        smallCenteredSlit(),
        fastConfig()
    );

    const Vector3f outsidePoint =
        field.receiverSurface().origin
        + 1.25f * field.receiverSurface().edgeU
        + 0.5f * field.receiverSurface().edgeV;

    EXPECT_EQ(field.normalizedWeightAt(outsidePoint, 0), 0.0f);
}
