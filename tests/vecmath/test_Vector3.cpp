#include <gtest/gtest.h>

#include <cmath>

#include "vecmath/Vector3.h"

static constexpr float kEps = 1e-5f;

template <typename T>
void ExpectVec3Eq(const Vector3<T> &a, const Vector3<T> &b) {
    EXPECT_EQ(a.x, b.x);
    EXPECT_EQ(a.y, b.y);
    EXPECT_EQ(a.z, b.z);
}

void ExpectVec3Near(const Vector3<float> &a, const Vector3<float> &b, float eps = kEps) {
    EXPECT_NEAR(a.x, b.x, eps);
    EXPECT_NEAR(a.y, b.y, eps);
    EXPECT_NEAR(a.z, b.z, eps);
}

TEST(Vector3, DefaultConstructorIsZero) {
    Vector3f v;
    ExpectVec3Eq(v, {0.f, 0.f, 0.f});
}

TEST(Vector3, ValueConstructor) {
    Vector3f v{1.f, 2.f, 3.f};
    EXPECT_FLOAT_EQ(v.x, 1.f);
    EXPECT_FLOAT_EQ(v.y, 2.f);
    EXPECT_FLOAT_EQ(v.z, 3.f);
}

TEST(Vector3, IntConstructor) {
    Vector3i v{4, 5, 6};
    EXPECT_EQ(v.x, 4);
    EXPECT_EQ(v.y, 5);
    EXPECT_EQ(v.z, 6);
}

TEST(Vector3, ConvertingConstructorIntToFloat) {
    Vector3i vi{1, 2, 3};
    Vector3f vf{vi};
    EXPECT_FLOAT_EQ(vf.x, 1.f);
    EXPECT_FLOAT_EQ(vf.y, 2.f);
    EXPECT_FLOAT_EQ(vf.z, 3.f);
}

TEST(Vector3, SubscriptRead) {
    Vector3f v{1.f, 2.f, 3.f};
    EXPECT_FLOAT_EQ(v[0], 1.f);
    EXPECT_FLOAT_EQ(v[1], 2.f);
    EXPECT_FLOAT_EQ(v[2], 3.f);
}

TEST(Vector3, SubscriptWrite) {
    Vector3f v{};
    v[0] = 7.f;
    v[1] = 8.f;
    v[2] = 9.f;
    EXPECT_FLOAT_EQ(v.x, 7.f);
    EXPECT_FLOAT_EQ(v.y, 8.f);
    EXPECT_FLOAT_EQ(v.z, 9.f);
}

TEST(Vector3, Addition) {
    Vector3f a{1.f, 2.f, 3.f};
    Vector3f b{4.f, 5.f, 6.f};
    ExpectVec3Eq(a + b, {5.f, 7.f, 9.f});
}

TEST(Vector3, AdditionMixedTypes) {
    Vector3i a{1, 2, 3};
    Vector3f b{0.5f, 1.5f, 2.5f};
    Vector3f result = a + b;
    ExpectVec3Near(result, {1.5f, 3.5f, 5.5f});
}

TEST(Vector3, AdditionAssign) {
    Vector3f a{1.f, 2.f, 3.f};
    a += {10.f, 20.f, 30.f};
    ExpectVec3Eq(a, {11.f, 22.f, 33.f});
}

TEST(Vector3, Subtraction) {
    Vector3f a{5.f, 7.f, 9.f};
    Vector3f b{1.f, 2.f, 3.f};
    ExpectVec3Eq(a - b, {4.f, 5.f, 6.f});
}

TEST(Vector3, SubtractionMixedTypes) {
    Vector3f a{1.5f, 3.5f, 5.5f};
    Vector3i b{1, 2, 3};
    Vector3f result = a - b;
    ExpectVec3Near(result, {0.5f, 1.5f, 2.5f});
}

TEST(Vector3, SubtractionAssign) {
    Vector3f a{10.f, 20.f, 30.f};
    a -= {1.f, 2.f, 3.f};
    ExpectVec3Eq(a, {9.f, 18.f, 27.f});
}

TEST(Vector3, ScalarMultiply) {
    Vector3f v{1.f, 2.f, 3.f};
    ExpectVec3Eq(v * 3.f, {3.f, 6.f, 9.f});
}

TEST(Vector3, ScalarMultiplyMixedTypes) {
    Vector3i v{1, 2, 3};
    Vector3f result = v * 2.5f;
    ExpectVec3Near(result, {2.5f, 5.f, 7.5f});
}

TEST(Vector3, ScalarMultiplyLhs) {
    Vector3f v{1.f, 2.f, 3.f};
    ExpectVec3Eq(3.f * v, {3.f, 6.f, 9.f});
}

TEST(Vector3, ScalarMultiplyLhsMixedTypes) {
    Vector3i v{1, 2, 3};
    Vector3f result = 2.5f * v;
    ExpectVec3Near(result, {2.5f, 5.f, 7.5f});
}

TEST(Vector3, ScalarMultiplyAssign) {
    Vector3f v{1.f, 2.f, 3.f};
    v *= 2.f;
    ExpectVec3Eq(v, {2.f, 4.f, 6.f});
}

TEST(Vector3, ScalarDivide) {
    Vector3f v{6.f, 9.f, 3.f};
    ExpectVec3Eq(v / 3.f, {2.f, 3.f, 1.f});
}

TEST(Vector3, ScalarDivideMixedTypes) {
    Vector3i v{3, 6, 9};
    Vector3f result = v / 2.f;
    ExpectVec3Near(result, {1.5f, 3.f, 4.5f});
}

TEST(Vector3, ScalarDivideAssign) {
    Vector3f v{6.f, 9.f, 3.f};
    v /= 3.f;
    ExpectVec3Eq(v, {2.f, 3.f, 1.f});
}

TEST(Vector3, UnaryNegate) {
    Vector3f v{1.f, -2.f, 3.f};
    ExpectVec3Eq(-v, {-1.f, 2.f, -3.f});
}

TEST(Vector3, EqualityTrue) { EXPECT_EQ((Vector3f{1.f, 2.f, 3.f}), (Vector3f{1.f, 2.f, 3.f})); }

TEST(Vector3, EqualityFalse) { EXPECT_NE((Vector3f{1.f, 2.f, 3.f}), (Vector3f{1.f, 2.f, 4.f})); }

TEST(Vector3, DotProduct) {
    Vector3f a{1.f, 2.f, 3.f};
    Vector3f b{4.f, 5.f, 6.f};
    EXPECT_FLOAT_EQ(a.dot(b), 32.f);
}

TEST(Vector3, DotProductMixedTypes) {
    Vector3i a{1, 2, 3};
    Vector3f b{1.5f, 2.5f, 3.5f};
    EXPECT_NEAR(a.dot(b), 1 * 1.5f + 2 * 2.5f + 3 * 3.5f, kEps);
}

TEST(Vector3, DotProductWithZeroVector) {
    Vector3f a{1.f, 2.f, 3.f};
    EXPECT_FLOAT_EQ(a.dot(Vector3f{}), 0.f);
}

TEST(Vector3, DotProductOrthogonal) {
    Vector3f x{1.f, 0.f, 0.f};
    Vector3f y{0.f, 1.f, 0.f};
    EXPECT_FLOAT_EQ(x.dot(y), 0.f);
}

TEST(Vector3, DotProductIsCommutative) {
    Vector3f a{1.f, 2.f, 3.f};
    Vector3f b{7.f, 8.f, 9.f};
    EXPECT_FLOAT_EQ(a.dot(b), b.dot(a));
}

TEST(Vector3, CrossProductBasisVectors) {
    Vector3f x{1.f, 0.f, 0.f};
    Vector3f y{0.f, 1.f, 0.f};
    Vector3f z{0.f, 0.f, 1.f};
    ExpectVec3Near(x.cross(y), z);
    ExpectVec3Near(y.cross(z), x);
    ExpectVec3Near(z.cross(x), y);
}

TEST(Vector3, CrossProductMixedTypes) {
    Vector3i a{1, 0, 0};
    Vector3f b{0.f, 1.f, 0.f};
    Vector3f result = a.cross(b);
    ExpectVec3Near(result, {0.f, 0.f, 1.f});
}

TEST(Vector3, CrossProductAnticommutative) {
    Vector3f a{1.f, 2.f, 3.f};
    Vector3f b{4.f, 5.f, 6.f};
    ExpectVec3Near(a.cross(b), -(b.cross(a)));
}

TEST(Vector3, CrossProductParallelVectorsIsZero) {
    Vector3f a{1.f, 2.f, 3.f};
    Vector3f b{2.f, 4.f, 6.f};
    ExpectVec3Near(a.cross(b), {0.f, 0.f, 0.f});
}

TEST(Vector3, CrossProductResultIsOrthogonal) {
    Vector3f a{1.f, 2.f, 3.f};
    Vector3f b{4.f, 5.f, 6.f};
    Vector3f c = a.cross(b);
    EXPECT_NEAR(c.dot(a), 0.f, kEps);
    EXPECT_NEAR(c.dot(b), 0.f, kEps);
}

TEST(Vector3, LengthSquared) { EXPECT_FLOAT_EQ((Vector3f{1.f, 2.f, 2.f}.lengthSq()), 9.f); }

TEST(Vector3, Length) { EXPECT_NEAR((Vector3f{1.f, 2.f, 2.f}.length()), 3.f, kEps); }

TEST(Vector3, LengthOfZeroVector) { EXPECT_FLOAT_EQ(Vector3f{}.length(), 0.f); }

TEST(Vector3, LengthUnitVector) { EXPECT_NEAR((Vector3f{1.f, 0.f, 0.f}.length()), 1.f, kEps); }

TEST(Vector3, NormalizedHasUnitLength) { EXPECT_NEAR((Vector3f{3.f, 1.f, 2.f}.normalized().length()), 1.f, kEps); }

TEST(Vector3, NormalizedPreservesDirection) { ExpectVec3Near(Vector3f{0.f, 0.f, 5.f}.normalized(), {0.f, 0.f, 1.f}); }

TEST(Vector3, NormalizedAlreadyUnitVector) { ExpectVec3Near(Vector3f{1.f, 0.f, 0.f}.normalized(), {1.f, 0.f, 0.f}); }

TEST(Vector3, NormalizedIntVector) {
    Vector3i vi{0, 0, 3};
    ExpectVec3Near(vi.normalized(), {0.f, 0.f, 1.f});
}