#include <gtest/gtest.h>

#include "light/Spectrum.h"

static constexpr float kEps = 1e-5f;

TEST(Spectrum, DefaultConstructedIsEmpty) {
    Spectrum spectrum;
    EXPECT_TRUE(spectrum.samples.empty());
}

TEST(Spectrum, DebugSpectrumHasTenSamples) {
    EXPECT_EQ(DebugSpectrum.samples.size(), 10);
}

TEST(Spectrum, DebugSpectrumFirstAndLastSamplesMatchExpectedValues) {
    ASSERT_EQ(DebugSpectrum.samples.size(), 10);

    EXPECT_NEAR(DebugSpectrum.samples.front().x, 430.0f, kEps);
    EXPECT_NEAR(DebugSpectrum.samples.front().y, 0.42f,  kEps);

    EXPECT_NEAR(DebugSpectrum.samples.back().x, 680.0f,  kEps);
    EXPECT_NEAR(DebugSpectrum.samples.back().y, 0.48f,   kEps);
}

TEST(Spectrum, DebugSpectrumWavelengthsAreStrictlyAscending) {
    ASSERT_FALSE(DebugSpectrum.samples.empty());

    for (std::size_t i = 1; i < DebugSpectrum.samples.size(); ++i) {
        EXPECT_LT(DebugSpectrum.samples[i - 1].x, DebugSpectrum.samples[i].x);
    }
}

TEST(Spectrum, DebugSpectrumIrradiancesArePositive) {
    ASSERT_FALSE(DebugSpectrum.samples.empty());

    for (const auto& sample : DebugSpectrum.samples) {
        EXPECT_GT(sample.y, 0.0f);
    }
}

TEST(Spectrum, CanStoreArbitrarySamples) {
    Spectrum spectrum{
            {
                {450.0f, 0.25f},
                {532.0f, 0.80f},
                {650.0f, 0.40f}
            }
    };

    ASSERT_EQ(spectrum.samples.size(), 3);

    EXPECT_NEAR(spectrum.samples[0].x, 450.0f, kEps);
    EXPECT_NEAR(spectrum.samples[0].y, 0.25f,  kEps);

    EXPECT_NEAR(spectrum.samples[1].x, 532.0f, kEps);
    EXPECT_NEAR(spectrum.samples[1].y, 0.80f,  kEps);

    EXPECT_NEAR(spectrum.samples[2].x, 650.0f, kEps);
    EXPECT_NEAR(spectrum.samples[2].y, 0.40f,  kEps);
}