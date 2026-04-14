#include <gtest/gtest.h>

#include <cmath>

#include "vecmath/Vector2.h"

static constexpr float kEps = 1e-5f;

static bool nearEq(float a, float b, float eps = kEps) { return std::abs(a - b) < eps; }

template <typename V1, typename V2>
static bool vecEq(const V1& a, const V2& b, float eps = kEps) {
    return nearEq((float)a.x, (float)b.x, eps) && nearEq((float)a.y, (float)b.y, eps);
}

TEST(Vector2, DefaultConstructorIsZero) {
    Vector2f v;
    EXPECT_EQ(v.x, 0.f);
    EXPECT_EQ(v.y, 0.f);
}

TEST(Vector2, ValueConstructor) {
    Vector2f v(1.f, 2.f);
    EXPECT_EQ(v.x, 1.f);
    EXPECT_EQ(v.y, 2.f);
}

TEST(Vector2, ConvertingConstructor) {
    Vector2i vi(3, 4);
    Vector2f vf(vi);
    EXPECT_EQ(vf.x, 3.f);
    EXPECT_EQ(vf.y, 4.f);
}

TEST(Vector2, IndexRead) {
    Vector2f v(7.f, 8.f);
    EXPECT_EQ(v[0], 7.f);
    EXPECT_EQ(v[1], 8.f);
}

TEST(Vector2, IndexWrite) {
    Vector2f v;
    v[0] = 1.f;
    v[1] = 2.f;
    EXPECT_EQ(v.x, 1.f);
    EXPECT_EQ(v.y, 2.f);
}

TEST(Vector2, Addition) {
    Vector2f a(1, 2), b(3, 4);
    auto r = a + b;
    EXPECT_TRUE(vecEq(r, Vector2f(4, 6)));
}

TEST(Vector2, Subtraction) {
    Vector2f a(5, 6), b(2, 3);
    auto r = a - b;
    EXPECT_TRUE(vecEq(r, Vector2f(3, 3)));
}

TEST(Vector2, ScalarMultiply) {
    Vector2f v(2, 3);
    auto r = v * 2.f;
    EXPECT_TRUE(vecEq(r, Vector2f(4, 6)));
}

TEST(Vector2, ScalarDivide) {
    Vector2f v(4, 6);
    auto r = v / 2.f;
    EXPECT_TRUE(vecEq(r, Vector2f(2, 3)));
}

TEST(Vector2, ScalarMultiplyCommutative) {
    Vector2f v(2, 3);
    auto r1 = v * 2.f;
    auto r2 = 2.f * v;
    EXPECT_TRUE(vecEq(r1, r2));
}

TEST(Vector2, AddAssign) {
    Vector2f v(1, 2);
    v += Vector2f(3, 4);
    EXPECT_TRUE(vecEq(v, Vector2f(4, 6)));
}

TEST(Vector2, SubAssign) {
    Vector2f v(5, 6);
    v -= Vector2f(2, 3);
    EXPECT_TRUE(vecEq(v, Vector2f(3, 3)));
}

TEST(Vector2, MulAssign) {
    Vector2f v(2, 3);
    v *= 2.f;
    EXPECT_TRUE(vecEq(v, Vector2f(4, 6)));
}

TEST(Vector2, DivAssign) {
    Vector2f v(4, 6);
    v /= 2.f;
    EXPECT_TRUE(vecEq(v, Vector2f(2, 3)));
}

TEST(Vector2, Negation) {
    Vector2f v(2, -3);
    EXPECT_TRUE(vecEq(-v, Vector2f(-2, 3)));
}

TEST(Vector2, DotProduct) {
    Vector2f a(1, 2), b(3, 4);
    EXPECT_NEAR(a.dot(b), 11.f, kEps);
}

TEST(Vector2, DotOrthogonalIsZero) {
    Vector2f a(1, 0), b(0, 1);
    EXPECT_NEAR(a.dot(b), 0.f, kEps);
}

TEST(Vector2, CrossProductScalar) {
    Vector2f a(1, 0), b(0, 1);
    EXPECT_NEAR(a.cross(b), 1.f, kEps);
}

TEST(Vector2, CrossAntisymmetric) {
    Vector2f a(2, 3), b(4, 5);
    EXPECT_NEAR(a.cross(b), -b.cross(a), kEps);
}

TEST(Vector2, CrossCollinearZero) {
    Vector2f a(2, 2), b(4, 4);
    EXPECT_NEAR(a.cross(b), 0.f, kEps);
}

TEST(Vector2, LengthSquared) {
    Vector2f v(3, 4);
    EXPECT_NEAR(v.lengthSq(), 25.f, kEps);
}

TEST(Vector2, Length) {
    Vector2f v(3, 4);
    EXPECT_NEAR(v.length(), 5.f, kEps);
}

TEST(Vector2, Normalize) {
    Vector2f v(3, 4);
    auto n = v.normalized();

    EXPECT_NEAR(n.length(), 1.f, kEps);
    EXPECT_TRUE(vecEq(n, Vector2f(0.6f, 0.8f)));
}

TEST(Vector2, EqualityTrue) {
    Vector2f a(1, 2), b(1, 2);
    EXPECT_TRUE(a == b);
    EXPECT_FALSE(a != b);
}

TEST(Vector2, EqualityFalse) {
    Vector2f a(1, 2), b(2, 1);
    EXPECT_FALSE(a == b);
    EXPECT_TRUE(a != b);
}

TEST(Vector2, TypePromotion) {
    Vector2i vi(1, 2);
    Vector2f vf(0.5f, 0.5f);

    auto r = vi + vf;

    static_assert(std::is_same_v<decltype(r), Vector2<float>>);

    EXPECT_NEAR(r.x, 1.5f, kEps);
    EXPECT_NEAR(r.y, 2.5f, kEps);
}

TEST(Vector2, AddThenSubtractRoundTrip) {
    Vector2f a(1, 2), b(3, 4);
    EXPECT_TRUE(vecEq((a + b) - b, a));
}

TEST(Vector2, ScalarRoundTrip) {
    Vector2f v(2, 3);
    EXPECT_TRUE(vecEq((v * 2.f) / 2.f, v));
}

TEST(Vector2, DotWithSelfEqualsLengthSq) {
    Vector2f v(3, 4);
    EXPECT_NEAR(v.dot(v), v.lengthSq(), kEps);
}