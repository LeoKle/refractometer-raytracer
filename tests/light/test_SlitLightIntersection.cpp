#include <gtest/gtest.h>

#include <cmath>

#include "light/SlitLight.h"

namespace {

constexpr float kEps = 1.0e-5f;

SlitLight makeSlit() {
    return SlitLight::from(
        {0.0f, 0.0f, 0.0f},
        {2.0f, 0.0f, 0.0f},
        {0.0f, 1.0f, 0.0f},
        HeNeSpectrum
    );
}

} // namespace

TEST(SlitLightIntersection, ReturnsPointNormalAndWeightForFiniteSlitHit) {
    const SlitLight light = makeSlit();

    const auto hit = light.intersect(
        Point3f{1.0f, 0.5f, 1000.0f},
        Vector3f{0.0f, 0.0f, -1.0f},
        632.8f
    );

    ASSERT_TRUE(hit.has_value());
    EXPECT_NEAR(hit->point.x, 1.0f, kEps);
    EXPECT_NEAR(hit->point.y, 0.5f, kEps);
    EXPECT_NEAR(hit->point.z, 0.0f, kEps);
    EXPECT_NEAR(hit->normal.x, 0.0f, kEps);
    EXPECT_NEAR(hit->normal.y, 0.0f, kEps);
    EXPECT_NEAR(hit->normal.z, 1.0f, kEps);
    EXPECT_NEAR(hit->interferenceWeight, 1.0f, kEps);
}

TEST(SlitLightIntersection, MissesOutsideFiniteSlitBounds) {
    const SlitLight light = makeSlit();

    const auto hit = light.intersect(
        Point3f{3.0f, 0.5f, 2.0f},
        Vector3f{0.0f, 0.0f, -1.0f},
        632.8f
    );

    EXPECT_FALSE(hit.has_value());
}

TEST(SlitLightIntersection, IntersectsSkewedParallelogramGeometry) {
    const SlitLight light = SlitLight::from(
        {1.0f, 2.0f, 3.0f},
        {3.0f, 2.0f, 3.0f},
        {2.0f, 4.0f, 3.0f},
        HeNeSpectrum
    );

    const auto hit = light.intersect(
        Point3f{2.25f, 3.5f, 5.0f},
        Vector3f{0.0f, 0.0f, -1.0f},
        632.8f
    );

    ASSERT_TRUE(hit.has_value());
    EXPECT_NEAR(hit->point.x, 2.25f, kEps);
    EXPECT_NEAR(hit->point.y, 3.5f, kEps);
    EXPECT_NEAR(hit->point.z, 3.0f, kEps);
}

TEST(SlitLightIntersection, RejectsParallelAndBackwardIntersections) {
    const SlitLight light = makeSlit();

    EXPECT_FALSE(light.intersect(
        Point3f{1.0f, 0.5f, 2.0f},
        Vector3f{1.0f, 0.0f, 0.0f},
        632.8f
    ).has_value());

    EXPECT_FALSE(light.intersect(
        Point3f{1.0f, 0.5f, 2.0f},
        Vector3f{0.0f, 0.0f, 1.0f},
        632.8f
    ).has_value());
}

TEST(SlitLightIntersection, RejectsDegenerateSlitGeometry) {
    const SlitLight light = SlitLight::from(
        {0.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, 0.0f},
        {0.0f, 1.0f, 0.0f},
        HeNeSpectrum
    );

    EXPECT_FALSE(light.intersect(
        Point3f{0.0f, 0.5f, 2.0f},
        Vector3f{0.0f, 0.0f, -1.0f},
        632.8f
    ).has_value());
}

TEST(SlitLightIntersection, ReturnsWavelengthSpecificInterferenceWeight) {
    const SlitLight light = makeSlit();
    const Point3f rayOrigin{1.5f, 0.5f, 1000.0f};
    const Vector3f direction = Vector3f{-0.5f, 0.0f, -1000.0f}.normalized();

    const auto shortWavelength = light.intersect(rayOrigin, direction, 430.0f);
    const auto longWavelength = light.intersect(rayOrigin, direction, 680.0f);

    ASSERT_TRUE(shortWavelength.has_value());
    ASSERT_TRUE(longWavelength.has_value());
    EXPECT_TRUE(std::isfinite(shortWavelength->interferenceWeight));
    EXPECT_TRUE(std::isfinite(longWavelength->interferenceWeight));
    EXPECT_NE(shortWavelength->interferenceWeight, longWavelength->interferenceWeight);
}

TEST(SlitLightIntersection, IsAvailableThroughLightSourceInterface) {
    const SlitLight slit = makeSlit();
    const ILightSource& light = slit;

    const auto hit = light.intersect(
        Point3f{1.0f, 0.5f, 2.0f},
        Vector3f{0.0f, 0.0f, -1.0f},
        632.8f
    );

    ASSERT_TRUE(hit.has_value());
    EXPECT_GE(hit->interferenceWeight, 0.0f);
    EXPECT_LE(hit->interferenceWeight, 1.0f);
}
