#include <gtest/gtest.h>

#include <cmath>

#include "light/SlitLight.h"

static constexpr float kEps = 1e-5f;

static bool nearEq(float a, float b, float eps = kEps) {
    return std::abs(a - b) < eps;
}

template <typename T>
static bool vecEq(const Vector3<T>& a, const Vector3<T>& b, float eps = kEps) {
    return nearEq((float)a.x, (float)b.x, eps) &&
           nearEq((float)a.y, (float)b.y, eps) &&
           nearEq((float)a.z, (float)b.z, eps);
}

TEST(SlitLight, SpectrumIsStoredUnchanged) {
    Spectrum spectrum{
        {
            {450.0f, 0.25f},
            {550.0f, 0.80f},
            {650.0f, 0.40f}
        }
    };

    SlitLight light = SlitLight::from(
        {0.0f, 0.0f, 0.0f},
        {2.0f, 0.0f, 0.0f},
        {0.0f, 4.0f, 0.0f},
        spectrum
    );

    const Spectrum& stored = light.spectrum();

    ASSERT_EQ(stored.samples.size(), 3);

    EXPECT_NEAR(stored.samples[0].x, 450.0f, kEps);
    EXPECT_NEAR(stored.samples[0].y, 0.25f,  kEps);

    EXPECT_NEAR(stored.samples[1].x, 550.0f, kEps);
    EXPECT_NEAR(stored.samples[1].y, 0.80f,  kEps);

    EXPECT_NEAR(stored.samples[2].x, 650.0f, kEps);
    EXPECT_NEAR(stored.samples[2].y, 0.40f,  kEps);
}

TEST(SlitLight, SpectrumReturnsSameReferenceOnRepeatedCalls) {
    SlitLight light = SlitLight::from(
        {0.0f, 0.0f, 0.0f},
        {1.0f, 0.0f, 0.0f},
        {0.0f, 1.0f, 0.0f},
        DebugSpectrum
    );

    const Spectrum& a = light.spectrum();
    const Spectrum& b = light.spectrum();

    EXPECT_EQ(&a, &b);
}

TEST(SlitLight, SamplePointStaysInsideAxisAlignedRectangle) {
    SlitLight light = SlitLight::from(
        {1.0f, 2.0f, 3.0f},
        {5.0f, 2.0f, 3.0f},
        {1.0f, 8.0f, 3.0f},
        DebugSpectrum
    );

    for (int i = 0; i < 1000; ++i) {
        Vector3f p = light.samplePoint();

        EXPECT_GE(p.x, 1.0f);
        EXPECT_LE(p.x, 5.0f);

        EXPECT_GE(p.y, 2.0f);
        EXPECT_LE(p.y, 8.0f);

        EXPECT_NEAR(p.z, 3.0f, kEps);
    }
}

TEST(SlitLight, SamplePointOnDegenerateWidthSlitKeepsXFixed) {
    SlitLight light = SlitLight::from(
        {2.0f, 1.0f, 0.0f},
        {2.0f, 1.0f, 0.0f},
        {2.0f, 6.0f, 0.0f},
        DebugSpectrum
    );

    for (int i = 0; i < 200; ++i) {
        Vector3f p = light.samplePoint();

        EXPECT_NEAR(p.x, 2.0f, kEps);
        EXPECT_GE(p.y, 1.0f);
        EXPECT_LE(p.y, 6.0f);
        EXPECT_NEAR(p.z, 0.0f, kEps);
    }
}

TEST(SlitLight, SamplePointOnDegenerateHeightSlitKeepsYFixed) {
    SlitLight light = SlitLight::from(
        {3.0f, 4.0f, 1.0f},
        {8.0f, 4.0f, 1.0f},
        {3.0f, 4.0f, 1.0f},
        DebugSpectrum
    );

    for (int i = 0; i < 200; ++i) {
        Vector3f p = light.samplePoint();

        EXPECT_GE(p.x, 3.0f);
        EXPECT_LE(p.x, 8.0f);
        EXPECT_NEAR(p.y, 4.0f, kEps);
        EXPECT_NEAR(p.z, 1.0f, kEps);
    }
}

TEST(SlitLight, CanBeUsedThroughLightSourceInterface) {
    SlitLight slitLight = SlitLight::from(
        {0.0f, 0.0f, 0.0f},
        {2.0f, 0.0f, 0.0f},
        {0.0f, 3.0f, 0.0f},
        DebugSpectrum
    );

    ILightSource* light = &slitLight;

    const Spectrum& spectrum = light->spectrum();
    Vector3f p = light->samplePoint();

    EXPECT_EQ(spectrum.samples.size(), DebugSpectrum.samples.size());

    EXPECT_GE(p.x, 0.0f);
    EXPECT_LE(p.x, 2.0f);

    EXPECT_GE(p.y, 0.0f);
    EXPECT_LE(p.y, 3.0f);

    EXPECT_NEAR(p.z, 0.0f, kEps);
}