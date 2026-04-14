#include <gtest/gtest.h>

#include <cmath>
#include <type_traits>

#include "vecmath/Point2.h"
#include "vecmath/Vector2.h"

static constexpr float kEps = 1e-5f;

static bool nearEq(float a, float b, float eps = kEps) { return std::abs(a - b) < eps; }

template <typename P1, typename P2>
static bool pointEq(const P1& a, const P2& b, float eps = kEps) {
    return nearEq((float)a.x, (float)b.x, eps) && nearEq((float)a.y, (float)b.y, eps);
}

template <typename V1, typename V2>
static bool vecEq(const V1& a, const V2& b, float eps = kEps) {
    return nearEq((float)a.x, (float)b.x, eps) && nearEq((float)a.y, (float)b.y, eps);
}

TEST(Point2, DefaultConstructorIsZero) {
    Point2f p;
    EXPECT_EQ(p.x, 0.f);
    EXPECT_EQ(p.y, 0.f);
}

TEST(Point2, ValueConstructor) {
    Point2f p(1.f, 2.f);
    EXPECT_EQ(p.x, 1.f);
    EXPECT_EQ(p.y, 2.f);
}

TEST(Point2, ConvertingConstructorIntToFloat) {
    Point2i pi(3, 4);
    Point2f pf(pi);
    EXPECT_EQ(pf.x, 3.f);
    EXPECT_EQ(pf.y, 4.f);
}

TEST(Point2, IndexReadXY) {
    Point2f p(7.f, 8.f);
    EXPECT_EQ(p[0], 7.f);
    EXPECT_EQ(p[1], 8.f);
}

TEST(Point2, IndexWrite) {
    Point2f p;
    p[0] = 1.f;
    p[1] = 2.f;
    EXPECT_EQ(p.x, 1.f);
    EXPECT_EQ(p.y, 2.f);
}

TEST(Point2, AddVectorGivesPoint) {
    Point2f p(1.f, 2.f);
    Vector2f v(4.f, 5.f);
    auto r = p + v;

    EXPECT_TRUE(pointEq(r, Point2f(5.f, 7.f)));
    static_assert(std::is_same_v<decltype(r), Point2f>);
}

TEST(Point2, AddVectorCommutative) {
    Point2f p(1.f, 0.f);
    Vector2f v(0.f, 1.f);

    auto r1 = p + v;
    auto r2 = v + p;

    EXPECT_TRUE(pointEq(r1, r2));
}

TEST(Point2, AddVectorAssign) {
    Point2f p(1.f, 2.f);
    p += Vector2f(1.f, 1.f);

    EXPECT_TRUE(pointEq(p, Point2f(2.f, 3.f)));
}

TEST(Point2, AddZeroVectorIsIdentity) {
    Point2f p(3.f, -1.f);
    auto r = p + Vector2f(0.f, 0.f);

    EXPECT_TRUE(pointEq(r, p));
}

TEST(Point2, SubtractVectorGivesPoint) {
    Point2f p(5.f, 7.f);
    Vector2f v(4.f, 5.f);

    auto r = p - v;

    EXPECT_TRUE(pointEq(r, Point2f(1.f, 2.f)));
    static_assert(std::is_same_v<decltype(r), Point2f>);
}

TEST(Point2, AddThenSubtractVectorRoundTrip) {
    Point2f p(2.f, -3.f);
    Vector2f v(1.f, 4.f);

    EXPECT_TRUE(pointEq((p + v) - v, p));
}

TEST(Point2, SubtractPointGivesVector) {
    Point2f a(5.f, 7.f);
    Point2f b(1.f, 2.f);

    auto r = a - b;

    EXPECT_TRUE(vecEq(r, Vector2f(4.f, 5.f)));
    static_assert(std::is_same_v<decltype(r), Vector2f>);
}

TEST(Point2, DisplacementIsAntisymmetric) {
    Point2f a(1.f, 2.f);
    Point2f b(4.f, 6.f);

    auto ab = b - a;
    auto ba = a - b;

    EXPECT_TRUE(vecEq(ab, -ba));
}

TEST(Point2, SelfDisplacementIsZero) {
    Point2f p(3.f, 3.f);

    auto v = p - p;

    EXPECT_TRUE(vecEq(v, Vector2f(0.f, 0.f)));
}

TEST(Point2, DisplacementLengthMatchesDistance) {
    Point2f a(0.f, 0.f);
    Point2f b(3.f, 4.f);

    float dispLen = (b - a).length();
    float dist = a.distanceTo(b);

    EXPECT_NEAR(dispLen, dist, kEps);
}

TEST(Point2, DistanceOriginTo345) {
    Point2f o(0.f, 0.f);
    Point2f p(3.f, 4.f);

    EXPECT_NEAR(o.distanceTo(p), 5.f, kEps);
}

TEST(Point2, DistanceIsSymmetric) {
    Point2f a(1.f, 2.f);
    Point2f b(4.f, 6.f);

    EXPECT_NEAR(a.distanceTo(b), b.distanceTo(a), kEps);
}

TEST(Point2, DistanceToSelfIsZero) {
    Point2f p(7.f, -3.f);

    EXPECT_NEAR(p.distanceTo(p), 0.f, kEps);
}

TEST(Point2, EqualityTrue) {
    Point2f a(1.f, 2.f);
    Point2f b(1.f, 2.f);

    EXPECT_TRUE(a == b);
    EXPECT_FALSE(a != b);
}

TEST(Point2, EqualityFalse) {
    Point2f a(1.f, 2.f);
    Point2f b(1.f, 3.f);

    EXPECT_FALSE(a == b);
    EXPECT_TRUE(a != b);
}

TEST(Point2, LerpAtZeroIsA) {
    Point2f a(0.f, 0.f);
    Point2f b(10.f, 10.f);

    EXPECT_TRUE(pointEq(lerp(a, b, 0.f), a));
}

TEST(Point2, LerpAtOneIsB) {
    Point2f a(0.f, 0.f);
    Point2f b(10.f, 10.f);

    EXPECT_TRUE(pointEq(lerp(a, b, 1.f), b));
}

TEST(Point2, LerpAtHalfIsMidpoint) {
    Point2f a(0.f, 0.f);
    Point2f b(4.f, 6.f);

    EXPECT_TRUE(pointEq(lerp(a, b, 0.5f), Point2f(2.f, 3.f)));
}

TEST(Point2, LerpIsLinear) {
    Point2f a(1.f, 1.f);
    Point2f b(3.f, 3.f);

    auto p0 = lerp(a, b, 0.f);
    auto p05 = lerp(a, b, 0.5f);
    auto p025 = lerp(a, b, 0.25f);
    auto mid = lerp(p0, p05, 0.5f);

    EXPECT_TRUE(pointEq(p025, mid));
}

TEST(Point2, IntPointPlusFloatVector) {
    Point2i pi(1, 2);
    Vector2f vf(0.5f, 0.5f);

    auto r = pi + vf;

    static_assert(std::is_same_v<decltype(r), Point2<float>>);

    EXPECT_NEAR(r.x, 1.5f, kEps);
    EXPECT_NEAR(r.y, 2.5f, kEps);
}

TEST(Point2, FloatPointMinusIntPoint) {
    Point2f pf(3.f, 3.f);
    Point2i pi(1, 1);

    auto v = pf - pi;

    static_assert(std::is_same_v<decltype(v), Vector2<float>>);

    EXPECT_NEAR(v.x, 2.f, kEps);
    EXPECT_NEAR(v.y, 2.f, kEps);
}