#include <gtest/gtest.h>

#include "shape/Prism.h"
#include "types/Medium.h"

class PrismTest : public ::testing::Test {
protected:
    AirMedium medium;

    // Unit triangular prism:
    // base triangle in XY plane:
    // v0(0,0,0), v1(1,0,0), v2(0,1,0)
    // height = 1
    Prism prism{Point3f(0.f, 0.f, 0.f), Point3f(1.f, 0.f, 0.f), Point3f(0.f, 1.f, 0.f), 1.f, medium};
};

TEST_F(PrismTest, IntersectBottomCap) {
    Point3f origin(0.2f, 0.2f, 1.0f);
    Vector3f dir(0.f, 0.f, -1.f);

    auto hit = prism.intersect(origin, dir);

    ASSERT_TRUE(hit.has_value());
    EXPECT_NEAR((*hit).point[2], 0.f, 1e-5f);
    EXPECT_EQ((*hit).normal[2], -1.f);
}

TEST_F(PrismTest, IntersectTopCap) {
    Point3f origin(0.2f, 0.2f, 2.0f);
    Vector3f dir(0.f, 0.f, -1.f);

    auto hit = prism.intersect(origin, dir);

    ASSERT_TRUE(hit.has_value());
    EXPECT_NEAR((*hit).point[2], 1.f, 1e-5f);
    EXPECT_EQ((*hit).normal[2], 1.f);
}

TEST_F(PrismTest, IntersectSideFace) {
    Point3f origin(0.5f, -0.5f, 0.5f);
    Vector3f dir(0.f, 1.f, 0.f);

    auto hit = prism.intersect(origin, dir);

    ASSERT_TRUE(hit.has_value());
    EXPECT_NEAR((*hit).point[1], 0.f, 1e-5f);
    EXPECT_TRUE(std::abs((*hit).normal[2]) < 1e-5f);  // side normal has no Z component
}

TEST_F(PrismTest, MissOutsidePrism) {
    Point3f origin(2.f, 2.f, 0.5f);
    Vector3f dir(1.f, 0.f, 0.f);

    auto hit = prism.intersect(origin, dir);

    EXPECT_FALSE(hit.has_value());
}

TEST_F(PrismTest, ParallelToCapsMiss) {
    Point3f origin(0.5f, 0.5f, 2.f);
    Vector3f dir(1.f, 0.f, 0.f);  // parallel to caps

    auto hit = prism.intersect(origin, dir);

    EXPECT_FALSE(hit.has_value());
}

TEST_F(PrismTest, ChoosesClosestIntersection) {
    Point3f origin(0.2f, 0.2f, -2.f);
    Vector3f dir(0.f, 0.f, 1.f);

    auto hit = prism.intersect(origin, dir);

    ASSERT_TRUE(hit.has_value());
    EXPECT_NEAR((*hit).point[2], 0.f, 1e-5f);
    EXPECT_EQ((*hit).normal[2], -1.f);
}

TEST_F(PrismTest, ReturnsSameMediumReference) {
    Point3f p(0.f, 0.f, 0.f);
    Medium& m = prism.getMedium(p);

    EXPECT_EQ(&m, &medium);
}