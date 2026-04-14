#include <gtest/gtest.h>

#include <cmath>

#include "vecmath/Point3.h"

static constexpr float kEps = 1e-5f;

static bool nearEq(float a, float b, float eps = kEps) { return std::abs(a - b) < eps; }

template <typename T>
static bool pointEq(const Point3<T>& a, const Point3<T>& b, float eps = kEps) {
    return nearEq((float)a.x, (float)b.x, eps) && nearEq((float)a.y, (float)b.y, eps) &&
           nearEq((float)a.z, (float)b.z, eps);
}

template <typename T>
static bool vecEq(const Vector3<T>& a, const Vector3<T>& b, float eps = kEps) {
    return nearEq((float)a.x, (float)b.x, eps) && nearEq((float)a.y, (float)b.y, eps) &&
           nearEq((float)a.z, (float)b.z, eps);
}

TEST(Point3, DefaultConstructorIsZero) {
    Point3f p;
    EXPECT_EQ(p.x, 0.f);
    EXPECT_EQ(p.y, 0.f);
    EXPECT_EQ(p.z, 0.f);
}

TEST(Point3, ValueConstructor) {
    Point3f p(1.f, 2.f, 3.f);
    EXPECT_EQ(p.x, 1.f);
    EXPECT_EQ(p.y, 2.f);
    EXPECT_EQ(p.z, 3.f);
}

TEST(Point3, ConvertingConstructorIntToFloat) {
    Point3i pi(3, 4, 5);
    Point3f pf(pi);
    EXPECT_EQ(pf.x, 3.f);
    EXPECT_EQ(pf.y, 4.f);
    EXPECT_EQ(pf.z, 5.f);
}

TEST(Point3, IndexReadXYZ) {
    Point3f p(7.f, 8.f, 9.f);
    EXPECT_EQ(p[0], 7.f);
    EXPECT_EQ(p[1], 8.f);
    EXPECT_EQ(p[2], 9.f);
}

TEST(Point3, IndexWrite) {
    Point3f p;
    p[0] = 1.f;
    p[1] = 2.f;
    p[2] = 3.f;
    EXPECT_EQ(p.x, 1.f);
    EXPECT_EQ(p.y, 2.f);
    EXPECT_EQ(p.z, 3.f);
}

TEST(Point3, AddVectorGivesPoint) {
    Point3f p(1.f, 2.f, 3.f);
    Vector3f v(4.f, 5.f, 6.f);
    auto r = p + v;
    EXPECT_TRUE(pointEq(r, Point3f(5.f, 7.f, 9.f)));
    // result must be a Point3, not a Vector3
    static_assert(std::is_same_v<decltype(r), Point3f>);
}

TEST(Point3, AddVectorCommutative) {
    Point3f p(1.f, 0.f, 0.f);
    Vector3f v(0.f, 1.f, 0.f);
    auto r1 = p + v;
    auto r2 = v + p;
    EXPECT_TRUE(pointEq(r1, r2));
}

TEST(Point3, AddVectorAssign) {
    Point3f p(1.f, 2.f, 3.f);
    p += Vector3f(1.f, 1.f, 1.f);
    EXPECT_TRUE(pointEq(p, Point3f(2.f, 3.f, 4.f)));
}

TEST(Point3, AddZeroVectorIsIdentity) {
    Point3f p(3.f, -1.f, 5.f);
    auto r = p + Vector3f(0.f, 0.f, 0.f);
    EXPECT_TRUE(pointEq(r, p));
}

TEST(Point3, SubtractVectorGivesPoint) {
    Point3f p(5.f, 7.f, 9.f);
    Vector3f v(4.f, 5.f, 6.f);
    auto r = p - v;
    EXPECT_TRUE(pointEq(r, Point3f(1.f, 2.f, 3.f)));
    static_assert(std::is_same_v<decltype(r), Point3f>);
}

TEST(Point3, AddThenSubtractVectorRoundtrips) {
    Point3f p(2.f, -3.f, 7.f);
    Vector3f v(1.f, 4.f, -2.f);
    EXPECT_TRUE(pointEq((p + v) - v, p));
}

TEST(Point3, SubtractPointGivesVector) {
    Point3f a(5.f, 7.f, 9.f);
    Point3f b(1.f, 2.f, 3.f);
    auto r = a - b;
    EXPECT_TRUE(vecEq(r, Vector3f(4.f, 5.f, 6.f)));
    static_assert(std::is_same_v<decltype(r), Vector3f>);
}

TEST(Point3, DisplacementIsAntisymmetric) {
    Point3f a(1.f, 2.f, 3.f);
    Point3f b(4.f, 6.f, 8.f);
    auto ab = b - a;
    auto ba = a - b;
    EXPECT_TRUE(vecEq(ab, -ba));
}

TEST(Point3, SelfDisplacementIsZero) {
    Point3f p(3.f, 3.f, 3.f);
    auto v = p - p;
    EXPECT_TRUE(vecEq(v, Vector3f(0.f, 0.f, 0.f)));
}

TEST(Point3, DisplacementLengthMatchesDistance) {
    Point3f a(0.f, 0.f, 0.f);
    Point3f b(3.f, 4.f, 0.f);
    float dispLen = (b - a).length();
    float dist = a.distanceTo(b);
    EXPECT_NEAR(dispLen, dist, kEps);
}

TEST(Point3, EqualityTrue) {
    Point3f a(1.f, 2.f, 3.f);
    Point3f b(1.f, 2.f, 3.f);
    EXPECT_TRUE(a == b);
    EXPECT_FALSE(a != b);
}

TEST(Point3, EqualityFalse) {
    Point3f a(1.f, 2.f, 3.f);
    Point3f b(1.f, 2.f, 4.f);
    EXPECT_FALSE(a == b);
    EXPECT_TRUE(a != b);
}

TEST(Point3, DistanceOriginTo345) {
    Point3f o(0.f, 0.f, 0.f);
    Point3f p(3.f, 4.f, 0.f);
    EXPECT_NEAR(o.distanceTo(p), 5.f, kEps);
}

TEST(Point3, DistanceIsSymmetric) {
    Point3f a(1.f, 2.f, 3.f);
    Point3f b(4.f, 6.f, 3.f);
    EXPECT_NEAR(a.distanceTo(b), b.distanceTo(a), kEps);
}

TEST(Point3, DistanceToSelfIsZero) {
    Point3f p(7.f, -3.f, 2.f);
    EXPECT_NEAR(p.distanceTo(p), 0.f, kEps);
}

TEST(Point3, DistanceUnitCubeDiagonal) {
    Point3f a(0.f, 0.f, 0.f);
    Point3f b(1.f, 1.f, 1.f);
    EXPECT_NEAR(a.distanceTo(b), std::sqrt(3.f), kEps);
}

TEST(Point3, LerpAtZeroIsA) {
    Point3f a(0.f, 0.f, 0.f);
    Point3f b(10.f, 10.f, 10.f);
    EXPECT_TRUE(pointEq(lerp(a, b, 0.f), a));
}

TEST(Point3, LerpAtOneIsB) {
    Point3f a(0.f, 0.f, 0.f);
    Point3f b(10.f, 10.f, 10.f);
    EXPECT_TRUE(pointEq(lerp(a, b, 1.f), b));
}

TEST(Point3, LerpAtHalfIsMidpoint) {
    Point3f a(0.f, 0.f, 0.f);
    Point3f b(4.f, 6.f, 8.f);
    EXPECT_TRUE(pointEq(lerp(a, b, 0.5f), Point3f(2.f, 3.f, 4.f)));
}

TEST(Point3, LerpIsLinear) {
    Point3f a(1.f, 1.f, 1.f);
    Point3f b(3.f, 3.f, 3.f);
    // lerp(a,b,0.25) should be halfway between lerp(a,b,0) and lerp(a,b,0.5)
    auto p0 = lerp(a, b, 0.f);
    auto p05 = lerp(a, b, 0.5f);
    auto p025 = lerp(a, b, 0.25f);
    auto mid = lerp(p0, p05, 0.5f);
    EXPECT_TRUE(pointEq(p025, mid));
}

TEST(Point3, IntPointPlusFloatVector) {
    Point3i pi(1, 2, 3);
    Vector3f vf(0.5f, 0.5f, 0.5f);
    auto r = pi + vf;
    // result type should promote to float
    static_assert(std::is_same_v<decltype(r), Point3<float>>);
    EXPECT_NEAR(r.x, 1.5f, kEps);
    EXPECT_NEAR(r.y, 2.5f, kEps);
    EXPECT_NEAR(r.z, 3.5f, kEps);
}

TEST(Point3, FloatPointMinusIntPoint) {
    Point3f pf(3.f, 3.f, 3.f);
    Point3i pi(1, 1, 1);
    auto v = pf - pi;
    static_assert(std::is_same_v<decltype(v), Vector3<float>>);
    EXPECT_NEAR(v.x, 2.f, kEps);
    EXPECT_NEAR(v.y, 2.f, kEps);
    EXPECT_NEAR(v.z, 2.f, kEps);
}