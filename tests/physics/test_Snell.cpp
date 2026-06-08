#include <gtest/gtest.h>

#include <cmath>

#include "physics/Snell.h"
#include "vecmath/Vector3.h"

static constexpr float kEps = 1e-5f;

static void ExpectVec3Near(const Vector3f &got, const Vector3f &want, float eps = kEps) {
    EXPECT_NEAR(got.x, want.x, eps) << "x mismatch";
    EXPECT_NEAR(got.y, want.y, eps) << "y mismatch";
    EXPECT_NEAR(got.z, want.z, eps) << "z mismatch";
}

static void ExpectUnit(const Vector3f &v, float eps = kEps) {
    EXPECT_NEAR(v.length(), 1.0f, eps) << "result is not a unit vector";
}

// Helper: outward normal always opposes the incident ray
// i.e. dot(n, r0) < 0
static Vector3f outward(Vector3f n) { return -n; }

// 1. Normal incidence -> refracted ray == incident ray
TEST(SnellLaw, NormalIncidence_NoDeflection) {
    Vector3f r0 = {0, 0, 1};
    Vector3f n = outward({0, 0, 1});  // {0, 0, -1}
    auto r1 = SnellLaw(1.0f, 1.5f, r0, n);
    ASSERT_TRUE(r1.has_value());
    ExpectVec3Near(*r1, r0);
}

// 2. Same medium (n1 == n2) -> direction unchanged
TEST(SnellLaw, SameMedium_DirectionUnchanged) {
    Vector3f r0 = {0.6f, 0.8f, 0.0f};
    Vector3f n = outward({0, 1, 0});  // {0, -1, 0}
    auto r1 = SnellLaw(1.3f, 1.3f, r0, n);
    ASSERT_TRUE(r1.has_value());
    ExpectVec3Near(*r1, r0);
}

// 3. Result must always be a unit vector
TEST(SnellLaw, ResultIsUnitVector_AirToGlass) {
    float angle = 30.0f * M_PI / 180.0f;
    Vector3f r0 = {std::sin(angle), 0.0f, std::cos(angle)};
    Vector3f n = outward({0, 0, 1});
    auto r1 = SnellLaw(1.0f, 1.5f, r0, n);
    ASSERT_TRUE(r1.has_value());
    ExpectUnit(*r1);
}

TEST(SnellLaw, ResultIsUnitVector_GlassToAir) {
    float angle = 20.0f * M_PI / 180.0f;
    Vector3f r0 = {std::sin(angle), 0.0f, std::cos(angle)};
    Vector3f n = outward({0, 0, 1});
    auto r1 = SnellLaw(1.5f, 1.0f, r0, n);
    ASSERT_TRUE(r1.has_value());
    ExpectUnit(*r1);
}

// 4. Snell's law: n1 * sin(θ1) = n2 * sin(θ2)
TEST(SnellLaw, SnellRatioSatisfied_AirToGlass) {
    const float n1 = 1.0f, n2 = 1.5f;
    float angle = 45.0f * M_PI / 180.0f;
    Vector3f r0 = {std::sin(angle), 0.0f, std::cos(angle)};
    Vector3f n = outward({0, 0, 1});

    auto r1 = SnellLaw(n1, n2, r0, n);
    ASSERT_TRUE(r1.has_value());

    float sinTheta1 = std::sqrt(r0.x * r0.x + r0.y * r0.y);
    float sinTheta2 = std::sqrt(r1->x * r1->x + r1->y * r1->y);
    EXPECT_NEAR(n1 * sinTheta1, n2 * sinTheta2, kEps);
}

TEST(SnellLaw, SnellRatioSatisfied_GlassToWater) {
    const float n1 = 1.5f, n2 = 1.33f;
    float angle = 25.0f * M_PI / 180.0f;
    Vector3f r0 = {std::sin(angle), std::cos(angle), 0.0f};
    Vector3f n = outward({0, 1, 0});

    auto r1 = SnellLaw(n1, n2, r0, n);
    ASSERT_TRUE(r1.has_value());

    float sinTheta1 = std::sqrt(r0.x * r0.x + r0.z * r0.z);
    float sinTheta2 = std::sqrt(r1->x * r1->x + r1->z * r1->z);
    EXPECT_NEAR(n1 * sinTheta1, n2 * sinTheta2, kEps);
}

// 5. Refracted ray must continue into the second medium (dot(n_outward, r1) < 0)
//    With outward normals the refracted ray travels INTO the surface, so dot is negative.
TEST(SnellLaw, RefractedRayInCorrectHalfSpace) {
    float angle = 35.0f * M_PI / 180.0f;
    Vector3f r0 = {std::sin(angle), 0.0f, std::cos(angle)};
    Vector3f n = outward({0, 0, 1});  // {0, 0, -1}

    auto r1 = SnellLaw(1.0f, 1.5f, r0, n);
    ASSERT_TRUE(r1.has_value());
    EXPECT_LT(n.dot(*r1), 0.0f);  // refracted ray goes into the surface
}

// 6. Going denser bends toward the normal: sin(θ2) < sin(θ1)
TEST(SnellLaw, DenserMedium_BendsTowardNormal) {
    float angle = 50.0f * M_PI / 180.0f;
    Vector3f r0 = {std::sin(angle), 0.0f, std::cos(angle)};
    Vector3f n = outward({0, 0, 1});

    auto r1 = SnellLaw(1.0f, 1.5f, r0, n);
    ASSERT_TRUE(r1.has_value());
    EXPECT_LT(std::abs(r1->x), std::abs(r0.x));
}

// 7. Going less dense bends away from the normal: sin(θ2) > sin(θ1)
TEST(SnellLaw, LessDenseMedium_BendsAwayFromNormal) {
    float angle = 20.0f * M_PI / 180.0f;
    Vector3f r0 = {std::sin(angle), 0.0f, std::cos(angle)};
    Vector3f n = outward({0, 0, 1});

    auto r1 = SnellLaw(1.5f, 1.0f, r0, n);
    ASSERT_TRUE(r1.has_value());
    EXPECT_GT(std::abs(r1->x), std::abs(r0.x));
}

// 8. Exact numerical value: air->glass, θ1=30° -> θ2=19.471°
TEST(SnellLaw, KnownAngle_AirToGlass_30deg) {
    const float n1 = 1.0f, n2 = 1.5f;
    float theta1 = 30.0f * M_PI / 180.0f;
    Vector3f r0 = {std::sin(theta1), 0.0f, std::cos(theta1)};
    Vector3f n = outward({0, 0, 1});

    auto r1 = SnellLaw(n1, n2, r0, n);
    ASSERT_TRUE(r1.has_value());

    float theta2 = std::asin(n1 * std::sin(theta1) / n2);
    EXPECT_NEAR(r1->x, std::sin(theta2), kEps);
    EXPECT_NEAR(r1->y, 0.0f, kEps);
    EXPECT_NEAR(r1->z, std::cos(theta2), kEps);
}

// 9. Non-axis-aligned normal: Snell ratio must still hold
TEST(SnellLaw, ArbitraryNormal_SnellRatioSatisfied) {
    const float n1 = 1.0f, n2 = 1.33f;
    Vector3f nhat = -Vector3f{1, 0, 1}.normalized();  // outward, opposes r0
    Vector3f r0 = Vector3f{0, 0, 1}.normalized();

    auto r1 = SnellLaw(n1, n2, r0, nhat);
    ASSERT_TRUE(r1.has_value());

    float sinT1 = r0.cross(nhat).length();
    float sinT2 = r1->cross(nhat).length();
    EXPECT_NEAR(n1 * sinT1, n2 * sinT2, 1e-4f);
    ExpectUnit(*r1, 1e-4f);
}

// 10. Transverse direction is preserved
TEST(SnellLaw, TransverseDirectionPreserved) {
    float angle = 40.0f * M_PI / 180.0f;
    Vector3f r0 = {std::sin(angle), 0.0f, std::cos(angle)};
    Vector3f n = outward({0, 0, 1});

    auto r1 = SnellLaw(1.0f, 1.5f, r0, n);
    ASSERT_TRUE(r1.has_value());
    EXPECT_GT(r1->x, 0.0f);
    EXPECT_NEAR(r1->y, 0.0f, kEps);
}

// 11. Total internal reflection returns nullopt
TEST(SnellLaw, TotalInternalReflection_ReturnsNullopt) {
    // Critical angle for glass->air ~41.8°; use 50°
    float angle = 50.0f * M_PI / 180.0f;
    Vector3f r0 = {std::sin(angle), 0.0f, std::cos(angle)};
    Vector3f n = outward({0, 0, 1});
    EXPECT_FALSE(SnellLaw(1.5f, 1.0f, r0, n).has_value());
}