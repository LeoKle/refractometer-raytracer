#include <gtest/gtest.h>
#include "detector/DetectorQE.h"
#include "light/Spectrum.h"
#include "detector/OpenApertureDetector.h"
#include "light/SlitLight.h"
#include "sampler/Sampler.h"

TEST(DetectorQETest, BasicRange) {
    // Check some known values or at least that they are in [0, 1]
    EXPECT_GE(GetDetectorQE(300.0f), 0.0f);
    EXPECT_LE(GetDetectorQE(300.0f), 1.0f);
    
    EXPECT_GE(GetDetectorQE(550.0f), 0.0f);
    EXPECT_LE(GetDetectorQE(550.0f), 1.0f);
    
    EXPECT_GE(GetDetectorQE(1000.0f), 0.0f);
    EXPECT_LE(GetDetectorQE(1000.0f), 1.0f);
}

TEST(DetectorQETest, MidpointInterpolation) {
    float lambda[] = {400.0f, 500.0f};
    float values[] = {0.2f, 0.6f};
    
    float val = LinearInterpolateSpectralCurve(lambda, values, 2, 450.0f);
    EXPECT_NEAR(val, 0.4f, 1e-6f);
}

TEST(DetectorQETest, FractionalInterpolation) {
    float lambda[] = {400.0f, 500.0f};
    float values[] = {0.0f, 1.0f};
    
    EXPECT_NEAR(LinearInterpolateSpectralCurve(lambda, values, 2, 425.0f), 0.25f, 1e-6f);
    EXPECT_NEAR(LinearInterpolateSpectralCurve(lambda, values, 2, 475.0f), 0.75f, 1e-6f);
}

TEST(DetectorQETest, ExactSampleHits) {
    float lambda[] = {400.0f, 500.0f, 600.0f};
    float values[] = {0.1f, 0.5f, 0.9f};
    
    EXPECT_NEAR(LinearInterpolateSpectralCurve(lambda, values, 3, 400.0f), 0.1f, 1e-6f);
    EXPECT_NEAR(LinearInterpolateSpectralCurve(lambda, values, 3, 500.0f), 0.5f, 1e-6f);
    EXPECT_NEAR(LinearInterpolateSpectralCurve(lambda, values, 3, 600.0f), 0.9f, 1e-6f);
}

TEST(DetectorQETest, Extrapolation) {
    float lambda[] = {400.0f, 500.0f};
    float values[] = {0.2f, 0.6f};
    
    // LinearInterpolateSpectralCurve should extrapolate
    // 400->0.2, 500->0.6 => slope = 0.4 / 100 = 0.004
    // 350 => 0.2 - 50 * 0.004 = 0.2 - 0.2 = 0.0
    EXPECT_NEAR(LinearInterpolateSpectralCurve(lambda, values, 2, 350.0f), 0.0f, 1e-6f);
    
    // GetDetectorQE should clamp
    // 300 => 0.2 - 100 * 0.004 = 0.2 - 0.4 = -0.2 => clamped to 0.0
    // Wait, GetDetectorQE uses the real QE data. I should test my helper with local data if I want specific results.
    // But GetDetectorQE is hardcoded to use DetectorQEBands/Values.
}

TEST(DetectorQETest, DropZeroSamplesRemovesZeros) {
    Spectrum s{
        {
            {400.0f, 0.0f},
            {500.0f, 1.0f},
            {600.0f, 0.0f},
            {700.0f, 0.5f}
        }
    };
    
    Spectrum dropped = DropZeroSamples(s);
    ASSERT_EQ(dropped.samples.size(), 2);
    EXPECT_FLOAT_EQ(dropped.samples[0].x, 500.0f);
    EXPECT_FLOAT_EQ(dropped.samples[0].y, 1.0f);
    EXPECT_FLOAT_EQ(dropped.samples[1].x, 700.0f);
    EXPECT_FLOAT_EQ(dropped.samples[1].y, 0.5f);
}

TEST(DetectorQETest, HeNeSpectrumHasOneSample) {
    EXPECT_EQ(HeNeSpectrum.samples.size(), 1);
    EXPECT_FLOAT_EQ(HeNeSpectrum.samples[0].x, 632.8f);
    EXPECT_FLOAT_EQ(HeNeSpectrum.samples[0].y, 1.0f);
}

TEST(DetectorQETest, WeightedHeNeSpectrumHasOneSample) {
    Spectrum weighted = MultiplySourceSpectrumByDetectorQE(HeNeSpectrum);
    ASSERT_EQ(weighted.samples.size(), 1);
    EXPECT_FLOAT_EQ(weighted.samples[0].x, 632.8f);
    EXPECT_NEAR(weighted.samples[0].y, GetDetectorQE(632.8f), 1e-6f);
}

TEST(DetectorQETest, OpenApertureDetectorRayCarriesSpectrum) {
    SlitLight light = SlitLight::from(
        Vector3f(0,0,0), Vector3f(1,0,0), Vector3f(0,1,0), HeNeSpectrum);
    OpenApertureDetector detector = OpenApertureDetector::go5000Mpmcl(light, 10, 10);
    
    auto cache = OqmcPmjBnSampler::createCache();
    OqmcPmjBnSampler sampler(0, 0, 0, 0, cache);
    
    Ray ray = detector.sampleRay(5, 5, sampler);
    ASSERT_EQ(ray.spectrum.samples.size(), 1);
    EXPECT_FLOAT_EQ(ray.spectrum.samples[0].x, 632.8f);
    EXPECT_NEAR(ray.spectrum.samples[0].y, GetDetectorQE(632.8f), 1e-6f);
}
