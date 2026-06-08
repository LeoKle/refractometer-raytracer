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

// Verify a vector has unit length within tolerance
static void ExpectUnit(const Vector3f &v, float eps = kEps) {
    EXPECT_NEAR(v.length(), 1.0f, eps) << "result is not a unit vector";
}

// 1. Normal incidence  (r0 parallel to n) -> refracted ray == incident ray
//    Angle of incidence = 0, so no bending regardless of n1/n2.
TEST(SnellLaw, NormalIncidence_NoDeflection) {
    Vector3f n = {0, 0, 1};   // surface normal (pointing into medium 2)
    Vector3f r0 = {0, 0, 1};  // ray straight along normal
    Vector3f r1 = SnellLaw(1.0f, 1.5f, r0, n);
    ExpectVec3Near(r1, r0);
}

// 2. Same medium (n1 == n2) -> direction unchanged
TEST(SnellLaw, SameMedium_DirectionUnchanged) {
    Vector3f n = {0, 1, 0};
    Vector3f r0 = {0.6f, 0.8f, 0.0f};  // some unit vector
    Vector3f r1 = SnellLaw(1.3f, 1.3f, r0, n);
    ExpectVec3Near(r1, r0);
}

// 3. Result must always be a unit vector (energy conservation)
TEST(SnellLaw, ResultIsUnitVector_AirToGlass) {
    Vector3f n = {0, 0, 1};
    // 30° incidence in air -> glass (n=1.5)
    float angle = 30.0f * M_PI / 180.0f;
    Vector3f r0 = {std::sin(angle), 0.0f, std::cos(angle)};
    Vector3f r1 = SnellLaw(1.0f, 1.5f, r0, n);
    ExpectUnit(r1);
}

TEST(SnellLaw, ResultIsUnitVector_GlassToAir) {
    Vector3f n = {0, 0, 1};
    float angle = 20.0f * M_PI / 180.0f;
    Vector3f r0 = {std::sin(angle), 0.0f, std::cos(angle)};
    Vector3f r1 = SnellLaw(1.5f, 1.0f, r0, n);
    ExpectUnit(r1);
}

// 4. Snell's law:  n1 * sin(θ1) = n2 * sin(θ2)
//    sin(θ) = length of transverse component (component perpendicular to n)
TEST(SnellLaw, SnellRatioSatisfied_AirToGlass) {
    const float n1 = 1.0f, n2 = 1.5f;
    Vector3f n = {0, 0, 1};
    float angle = 45.0f * M_PI / 180.0f;
    Vector3f r0 = {std::sin(angle), 0.0f, std::cos(angle)};

    Vector3f r1 = SnellLaw(n1, n2, r0, n);

    float sinTheta1 = std::sqrt(r0.x * r0.x + r0.y * r0.y);  // transverse of r0
    float sinTheta2 = std::sqrt(r1.x * r1.x + r1.y * r1.y);  // transverse of r1

    EXPECT_NEAR(n1 * sinTheta1, n2 * sinTheta2, kEps);
}

TEST(SnellLaw, SnellRatioSatisfied_GlassToWater) {
    const float n1 = 1.5f, n2 = 1.33f;
    Vector3f n = {0, 1, 0};
    float angle = 25.0f * M_PI / 180.0f;
    Vector3f r0 = {std::sin(angle), std::cos(angle), 0.0f};

    Vector3f r1 = SnellLaw(n1, n2, r0, n);

    float sinTheta1 = std::sqrt(r0.x * r0.x + r0.z * r0.z);
    float sinTheta2 = std::sqrt(r1.x * r1.x + r1.z * r1.z);

    EXPECT_NEAR(n1 * sinTheta1, n2 * sinTheta2, kEps);
}

// 5. Refracted ray must stay in the same half-space as the incident ray
//    (n · r1 > 0 when n · r0 > 0)
TEST(SnellLaw, RefractedRayInCorrectHalfSpace) {
    Vector3f n = {0, 0, 1};
    float angle = 35.0f * M_PI / 180.0f;
    Vector3f r0 = {std::sin(angle), 0.0f, std::cos(angle)};
    Vector3f r1 = SnellLaw(1.0f, 1.5f, r0, n);
    EXPECT_GT(n.dot(r1), 0.0f);
}

// 6. Going denser (n1 < n2) bends the ray toward the normal:
//    transverse component shrinks  ->  sin(θ2) < sin(θ1)
TEST(SnellLaw, DenserMedium_BendsTowardNormal) {
    Vector3f n = {0, 0, 1};
    float angle = 50.0f * M_PI / 180.0f;
    Vector3f r0 = {std::sin(angle), 0.0f, std::cos(angle)};
    Vector3f r1 = SnellLaw(1.0f, 1.5f, r0, n);

    float sinTheta1 = std::abs(r0.x);
    float sinTheta2 = std::abs(r1.x);
    EXPECT_LT(sinTheta2, sinTheta1);
}

// 7. Going less dense (n1 > n2) bends the ray away from the normal:
//    transverse component grows  ->  sin(θ2) > sin(θ1)
TEST(SnellLaw, LessDenseMedium_BendsAwayFromNormal) {
    Vector3f n = {0, 0, 1};
    float angle = 20.0f * M_PI / 180.0f;
    Vector3f r0 = {std::sin(angle), 0.0f, std::cos(angle)};
    Vector3f r1 = SnellLaw(1.5f, 1.0f, r0, n);

    float sinTheta1 = std::abs(r0.x);
    float sinTheta2 = std::abs(r1.x);
    EXPECT_GT(sinTheta2, sinTheta1);
}

// 8. Exact numerical value for a known case:
//    air->glass (n1=1, n2=1.5), θ1=30° -> θ2=arcsin(sin30/1.5)=19.471°
TEST(SnellLaw, KnownAngle_AirToGlass_30deg) {
    const float n1 = 1.0f, n2 = 1.5f;
    Vector3f nhat = {0, 0, 1};
    float theta1 = 30.0f * M_PI / 180.0f;
    Vector3f r0 = {std::sin(theta1), 0.0f, std::cos(theta1)};

    Vector3f r1 = SnellLaw(n1, n2, r0, nhat);

    float theta2_expected = std::asin(n1 * std::sin(theta1) / n2);
    EXPECT_NEAR(r1.x, std::sin(theta2_expected), kEps);
    EXPECT_NEAR(r1.y, 0.0f, kEps);
    EXPECT_NEAR(r1.z, std::cos(theta2_expected), kEps);
}

// 9. Non-axis-aligned normal: same physics must hold
TEST(SnellLaw, ArbitraryNormal_SnellRatioSatisfied) {
    const float n1 = 1.0f, n2 = 1.33f;  // air -> water
    // Normal at 45° in the XZ plane
    Vector3f nhat = Vector3f{1, 0, 1}.normalized();
    // Incident ray: straight down Z (must have positive dot with normal)
    Vector3f r0 = Vector3f{0, 0, 1}.normalized();

    Vector3f r1 = SnellLaw(n1, n2, r0, nhat);

    // Compute sin(θ) via cross product magnitude: |r × n̂|
    auto cross0 = r0.cross(nhat);
    auto cross1 = r1.cross(nhat);
    float sinT1 = cross0.length();
    float sinT2 = cross1.length();

    EXPECT_NEAR(n1 * sinT1, n2 * sinT2, 1e-4f);
    ExpectUnit(r1, 1e-4f);
}

// 10. Transverse direction is preserved (the in-plane component points the same way)
TEST(SnellLaw, TransverseDirectionPreserved) {
    Vector3f n = {0, 0, 1};
    float angle = 40.0f * M_PI / 180.0f;
    Vector3f r0 = {std::sin(angle), 0.0f, std::cos(angle)};
    Vector3f r1 = SnellLaw(1.0f, 1.5f, r0, n);
    // x-component must keep the same sign
    EXPECT_GT(r1.x, 0.0f);
    // y-component must stay zero (2-D geometry)
    EXPECT_NEAR(r1.y, 0.0f, kEps);
}