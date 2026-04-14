#include <gtest/gtest.h>

#include "vecmath/Bounds3.h"
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

TEST(Bounds3, DefaultConstructorIsInvalid) {
    Bounds3f b;
    EXPECT_GT(b.pMin.x, b.pMax.x);
    EXPECT_GT(b.pMin.y, b.pMax.y);
    EXPECT_GT(b.pMin.z, b.pMax.z);
}

TEST(Bounds3, ConstructsFromTwoPoints) {
    Bounds3f b({5, 6, 7}, {1, 2, 3});

    EXPECT_TRUE(pointEq(b.pMin, Point3f(1, 2, 3)));
    EXPECT_TRUE(pointEq(b.pMax, Point3f(5, 6, 7)));
}

TEST(Bounds3, DiagonalCorrect) {
    Bounds3f b({1, 2, 3}, {5, 7, 9});
    EXPECT_TRUE(vecEq(b.diagonal(), Vector3f(4, 5, 6)));
}

TEST(Bounds3, CentroidCorrect) {
    Bounds3f b({0, 0, 0}, {10, 10, 10});
    EXPECT_TRUE(pointEq(b.centroid(), Point3f(5, 5, 5)));
}

TEST(Bounds3, CentroidDegenerate) {
    Bounds3f b({1, 1, 1}, {1, 1, 1});
    EXPECT_TRUE(pointEq(b.centroid(), Point3f(1, 1, 1)));
}

TEST(Bounds3, ExpandMergesBounds) {
    Bounds3f a({0, 0, 0}, {5, 5, 5});
    Bounds3f b({3, -1, 2}, {10, 6, 8});

    auto r = a.expand(b);

    EXPECT_TRUE(pointEq(r.pMin, Point3f(0, -1, 0)));
    EXPECT_TRUE(pointEq(r.pMax, Point3f(10, 6, 8)));
}

TEST(Bounds3, ExpandWithSelfIsIdempotent) {
    Bounds3f b({1, 2, 3}, {4, 5, 6});
    auto r = b.expand(b);

    EXPECT_TRUE(pointEq(r.pMin, b.pMin));
    EXPECT_TRUE(pointEq(r.pMax, b.pMax));
}

TEST(Bounds3, ContainsInside) {
    Bounds3f b({0, 0, 0}, {10, 10, 10});
    EXPECT_TRUE(b.contains({5, 5, 5}));
}

TEST(Bounds3, ContainsBoundaryInclusive) {
    Bounds3f b({0, 0, 0}, {10, 10, 10});

    EXPECT_TRUE(b.contains({0, 0, 0}));
    EXPECT_TRUE(b.contains({10, 10, 10}));
    EXPECT_TRUE(b.contains({0, 5, 10}));
}

TEST(Bounds3, ContainsOutside) {
    Bounds3f b({0, 0, 0}, {10, 10, 10});

    EXPECT_FALSE(b.contains({-1, 5, 5}));
    EXPECT_FALSE(b.contains({5, 11, 5}));
    EXPECT_FALSE(b.contains({5, 5, 20}));
}

TEST(Bounds3, ContainsExclusiveInside) {
    Bounds3f b({0, 0, 0}, {10, 10, 10});
    EXPECT_TRUE(b.containsExclusive({5, 5, 5}));
}

TEST(Bounds3, ContainsExclusiveRejectsBoundary) {
    Bounds3f b({0, 0, 0}, {10, 10, 10});

    EXPECT_FALSE(b.containsExclusive({0, 0, 0}));
    EXPECT_FALSE(b.containsExclusive({10, 10, 10}));
    EXPECT_FALSE(b.containsExclusive({0, 5, 10}));
}

TEST(Bounds3, DegenerateContainsSinglePoint) {
    Bounds3f b({1, 1, 1}, {1, 1, 1});

    EXPECT_TRUE(b.contains({1, 1, 1}));
    EXPECT_FALSE(b.contains({1, 1, 1.0001f}));
}

TEST(Bounds3, DegenerateExclusiveAlwaysFalse) {
    Bounds3f b({1, 1, 1}, {1, 1, 1});
    EXPECT_FALSE(b.containsExclusive({1, 1, 1}));
}

TEST(Bounds3, IntegerContains) {
    Bounds3i b({0, 0, 0}, {10, 10, 10});

    EXPECT_TRUE(b.contains({3, 3, 3}));
    EXPECT_FALSE(b.contains({11, 0, 0}));
}

TEST(Bounds3, IntegerExclusive) {
    Bounds3i b({0, 0, 0}, {10, 10, 10});

    EXPECT_TRUE(b.containsExclusive({5, 5, 5}));
    EXPECT_FALSE(b.containsExclusive({0, 5, 5}));
}

TEST(Bounds3, InvalidBoundsNeverContain) {
    Bounds3f b;  // default invalid

    EXPECT_FALSE(b.contains({0, 0, 0}));
    EXPECT_FALSE(b.containsExclusive({0, 0, 0}));
}