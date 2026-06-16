#include <gtest/gtest.h>

#include <array>
#include <cmath>
#include <memory>

#include "light/SlitLight.h"
#include "sampler/Sampler.h"

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

class CyclingSampler : public ISampler {
public:
    float next1D() override {
        const float value = static_cast<float>((m_index % 17) + 1) / 18.0f;
        ++m_index;
        return value;
    }

    std::array<float, 2> next2D() override {
        const float u = static_cast<float>((m_index % 17) + 1) / 18.0f;
        const float v = static_cast<float>(((m_index * 7) % 17) + 1) / 18.0f;
        ++m_index;
        return {u, v};
    }

    std::array<float, 3> next3D() override {
        const std::array<float, 2> sample = next2D();
        return {sample[0], sample[1], next1D()};
    }

    std::unique_ptr<ISampler> split(DomainKey) override {
        return std::make_unique<CyclingSampler>();
    }

    std::unique_ptr<ISampler> clone() override {
        return std::make_unique<CyclingSampler>(*this);
    }

private:
    int m_index = 0;
};

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
    CyclingSampler sampler;

    auto cache = OqmcPmjBnSampler::createCache();
    OqmcPmjBnSampler sampler(0, 0, 0, 0, cache);

    for (int i = 0; i < 1000; ++i) {
        Vector3f p = light.samplePoint(sampler);

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
    CyclingSampler sampler;

    auto cache = OqmcPmjBnSampler::createCache();
    OqmcPmjBnSampler sampler(0, 0, 0, 0, cache);

    for (int i = 0; i < 200; ++i) {
        Vector3f p = light.samplePoint(sampler);

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
    CyclingSampler sampler;

    auto cache = OqmcPmjBnSampler::createCache();
    OqmcPmjBnSampler sampler(0, 0, 0, 0, cache);

    for (int i = 0; i < 200; ++i) {
        Vector3f p = light.samplePoint(sampler);

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
    CyclingSampler sampler;

    auto cache = OqmcPmjBnSampler::createCache();
    OqmcPmjBnSampler sampler(0, 0, 0, 0, cache);

    const Spectrum& spectrum = light->spectrum();
    Vector3f p = light->samplePoint(sampler);

    EXPECT_EQ(spectrum.samples.size(), DebugSpectrum.samples.size());

    EXPECT_GE(p.x, 0.0f);
    EXPECT_LE(p.x, 2.0f);

    EXPECT_GE(p.y, 0.0f);
    EXPECT_LE(p.y, 3.0f);

    EXPECT_NEAR(p.z, 0.0f, kEps);
}

TEST(SlitLight, InterferenceWeightCentralMaximumIsOne) {
    SlitLight light = SlitLight::from(
        {-0.5f, -0.5f, 0.0f},
        { 0.5f, -0.5f, 0.0f},
        {-0.5f,  0.5f, 0.0f},
        DebugSpectrum
    );

    const float weight = light.interferenceWeight(
        {0.0f, 0.0f, 1000.0f},
        {0.0f, 0.0f, -1.0f},
        550.0f
    );

    EXPECT_NEAR(weight, 1.0f, kEps);
}

TEST(SlitLight, InterferenceWeightIsFiniteAndClamped) {
    SlitLight light = SlitLight::from(
        {-0.5f, -0.5f, 0.0f},
        { 0.5f, -0.5f, 0.0f},
        {-0.5f,  0.5f, 0.0f},
        DebugSpectrum
    );

    const std::array<Vector3f, 4> points = {{
        {0.1f, 0.0f, 1000.0f},
        {0.5f, 0.0f, 1000.0f},
        {1.0f, 0.0f, 1000.0f},
        {10.0f, 0.0f, 1000.0f}
    }};

    for (const Vector3f& point : points) {
        const float weight = light.interferenceWeight(point, {0.0f, 0.0f, -1.0f}, 550.0f);
        EXPECT_TRUE(std::isfinite(weight));
        EXPECT_GE(weight, 0.0f);
        EXPECT_LE(weight, 1.0f);
    }
}

TEST(SlitLight, InterferenceWeightDecreasesAwayFromCenterForSimpleCase) {
    SlitLight light = SlitLight::from(
        {-0.5f, -0.5f, 0.0f},
        { 0.5f, -0.5f, 0.0f},
        {-0.5f,  0.5f, 0.0f},
        DebugSpectrum
    );

    const float center = light.interferenceWeight(
        {0.0f, 0.0f, 1000.0f},
        {0.0f, 0.0f, -1.0f},
        550.0f
    );
    const float offAxis = light.interferenceWeight(
        {0.5f, 0.0f, 1000.0f},
        {0.0f, 0.0f, -1.0f},
        550.0f
    );

    EXPECT_LT(offAxis, center);
}

TEST(SlitLight, InterferenceWeightHandlesZeroBeta) {
    SlitLight light = SlitLight::from(
        {-0.5f, -0.5f, 0.0f},
        { 0.5f, -0.5f, 0.0f},
        {-0.5f,  0.5f, 0.0f},
        DebugSpectrum
    );

    const float weight = light.interferenceWeight(
        {0.0f, 0.0f, 1000.0f},
        {0.0f, 0.0f, -1.0f},
        430.0f
    );

    EXPECT_TRUE(std::isfinite(weight));
    EXPECT_NEAR(weight, 1.0f, kEps);
}

TEST(SlitLight, InterferenceWeightRejectsInvalidWavelength) {
    SlitLight light = SlitLight::from(
        {-0.5f, -0.5f, 0.0f},
        { 0.5f, -0.5f, 0.0f},
        {-0.5f,  0.5f, 0.0f},
        DebugSpectrum
    );

    EXPECT_EQ(light.interferenceWeight({0.1f, 0.0f, 1000.0f}, {0.0f, 0.0f, -1.0f}, 0.0f), 0.0f);
    EXPECT_EQ(light.interferenceWeight({0.1f, 0.0f, 1000.0f}, {0.0f, 0.0f, -1.0f}, -1.0f), 0.0f);
}

TEST(SlitLight, InterferenceWeightChangesWithWavelength) {
    SlitLight light = SlitLight::from(
        {-0.5f, -0.5f, 0.0f},
        { 0.5f, -0.5f, 0.0f},
        {-0.5f,  0.5f, 0.0f},
        DebugSpectrum
    );

    const Vector3f point{0.5f, 0.0f, 1000.0f};
    const float shortWavelength = light.interferenceWeight(point, {0.0f, 0.0f, -1.0f}, 430.0f);
    const float longWavelength = light.interferenceWeight(point, {0.0f, 0.0f, -1.0f}, 680.0f);

    EXPECT_TRUE(std::isfinite(shortWavelength));
    EXPECT_TRUE(std::isfinite(longWavelength));
    EXPECT_NE(shortWavelength, longWavelength);
}

TEST(SlitLight, InterferenceWeightDegenerateWidthReturnsZero) {
    SlitLight light = SlitLight::from(
        {0.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, 0.0f},
        {0.0f, 1.0f, 0.0f},
        DebugSpectrum
    );

    const float weight = light.interferenceWeight(
        {0.0f, 0.0f, 1000.0f},
        {0.0f, 0.0f, -1.0f},
        550.0f
    );

    EXPECT_EQ(weight, 0.0f);
}

TEST(SlitLight, InterferenceWeightFarOffAxisStaysFiniteAndClamped) {
    SlitLight light = SlitLight::from(
        {-0.5f, -0.5f, 0.0f},
        { 0.5f, -0.5f, 0.0f},
        {-0.5f,  0.5f, 0.0f},
        DebugSpectrum
    );

    const float weight = light.interferenceWeight(
        {1000000.0f, 0.0f, 1000.0f},
        {0.0f, 0.0f, -1.0f},
        550.0f
    );

    EXPECT_TRUE(std::isfinite(weight));
    EXPECT_GE(weight, 0.0f);
    EXPECT_LE(weight, 1.0f);
}
