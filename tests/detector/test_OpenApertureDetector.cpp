#include <gtest/gtest.h>

#include <array>
#include <cmath>
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

#include "detector/DetectorQE.h"
#include "detector/OpenApertureDetector.h"
#include "detector/PinHoleDetector.h"
#include "light/Spectrum.h"
#include "sampler/ISampler.h"

namespace {

constexpr float kEps = 1.0e-5f;

class SequenceSampler : public ISampler {
public:
    explicit SequenceSampler(std::vector<std::array<float, 2>> samples)
        : m_samples{std::move(samples)}
    {}

    float next1D() override {
        return 0.5f;
    }

    std::array<float, 2> next2D() override {
        if (m_index >= m_samples.size()) {
            return {0.5f, 0.5f};
        }
        return m_samples[m_index++];
    }

    std::array<float, 3> next3D() override {
        return {0.5f, 0.5f, 0.5f};
    }

    std::unique_ptr<ISampler> split(DomainKey) override {
        return clone();
    }

    std::unique_ptr<ISampler> clone() override {
        return std::make_unique<SequenceSampler>(*this);
    }

private:
    std::vector<std::array<float, 2>> m_samples;
    std::size_t m_index = 0;
};

OpenApertureDetector makeDetector(float apertureRadius = 0.25f) {
    return OpenApertureDetector::focused(
        HeNeSpectrum,
        {0.0f, 0.0f, 0.0f},
        {2.0f, 0.0f, 0.0f},
        {0.0f, 2.0f, 0.0f},
        {1.0f, 1.0f, -1.0f},
        apertureRadius,
        10.0f,
        4,
        4
    );
}

bool finite(const Vector3f& value) {
    return std::isfinite(value.x)
        && std::isfinite(value.y)
        && std::isfinite(value.z);
}

void expectNearVector(const Vector3f& a, const Vector3f& b, float eps) {
    EXPECT_NEAR(a.x, b.x, eps);
    EXPECT_NEAR(a.y, b.y, eps);
    EXPECT_NEAR(a.z, b.z, eps);
}

} // namespace

TEST(OpenApertureDetectorTest, SensorPointStaysInsidePixelFootprint) {
    const auto detector = makeDetector();

    for (const auto [x, y] : {std::array<int, 2>{0, 0},
                              std::array<int, 2>{2, 1},
                              std::array<int, 2>{3, 3}}) {
        SequenceSampler sampler({{0.25f, 0.75f}, {0.0f, 0.0f}});
        const auto sample = detector.sampleDebugRay(x, y, sampler);

        const float u = sample.sensorPoint.x / 2.0f;
        const float v = sample.sensorPoint.y / 2.0f;

        EXPECT_GE(u, static_cast<float>(x) / 4.0f);
        EXPECT_LE(u, static_cast<float>(x + 1) / 4.0f);
        EXPECT_GE(v, static_cast<float>(y) / 4.0f);
        EXPECT_LE(v, static_cast<float>(y + 1) / 4.0f);
        EXPECT_NEAR(sample.sensorPoint.z, 0.0f, kEps);
    }
}

TEST(OpenApertureDetectorTest, AperturePointStaysInsideCircularOpening) {
    constexpr float radius = 0.25f;
    const auto detector = makeDetector(radius);

    for (const auto apertureSample : {std::array<float, 2>{0.0f, 0.0f},
                                      std::array<float, 2>{1.0f, 0.0f},
                                      std::array<float, 2>{0.25f, 0.25f}}) {
        SequenceSampler sampler({{0.5f, 0.5f}, apertureSample});
        const auto sample = detector.sampleDebugRay(1, 1, sampler);

        const Vector3f offset = sample.aperturePoint - Vector3f{1.0f, 1.0f, -1.0f};
        EXPECT_LE(offset.length(), radius + kEps);
        EXPECT_NEAR(sample.aperturePoint.z, -1.0f, kEps);
    }
}

TEST(OpenApertureDetectorTest, SampleRayReturnsFiniteNormalizedDirection) {
    auto detector = makeDetector();
    SequenceSampler sampler({{0.5f, 0.5f}, {0.5f, 0.25f}});

    const Ray ray = detector.sampleRay(2, 2, sampler);

    EXPECT_TRUE(finite(ray.origin));
    EXPECT_TRUE(finite(ray.direction));
    EXPECT_NEAR(ray.direction.length(), 1.0f, kEps);
}

TEST(OpenApertureDetectorTest, TinyApertureApproximatesPinholeDirection) {
    constexpr int width = 4;
    constexpr int height = 4;
    constexpr float tinyRadius = 1.0e-6f;

    const auto pinhole = PinHoleDetector::from(
        {0.0f, 0.0f, 0.0f},
        {2.0f, 0.0f, 0.0f},
        {0.0f, 2.0f, 0.0f},
        {1.0f, 1.0f, -1.0f},
        width,
        height
    );
    const auto open = OpenApertureDetector::focused(
        HeNeSpectrum,
        {0.0f, 0.0f, 0.0f},
        {2.0f, 0.0f, 0.0f},
        {0.0f, 2.0f, 0.0f},
        {1.0f, 1.0f, -1.0f},
        tinyRadius,
        10.0f,
        width,
        height
    );

    SequenceSampler pinholeSampler({{0.5f, 0.5f}});
    SequenceSampler openSampler({{0.5f, 0.5f}, {1.0f, 0.0f}});

    const Ray pinholeRay = pinhole.sampleRay(2, 2, pinholeSampler);
    const Ray openRay = open.sampleRay(2, 2, openSampler);

    expectNearVector(openRay.direction, pinholeRay.direction, 1.0e-3f);
}

TEST(OpenApertureDetectorTest, DifferentApertureSamplesConvergeOnFocusPoint) {
    const auto detector = makeDetector();

    SequenceSampler samplerA({{0.25f, 0.75f}, {0.0f, 0.0f}});
    SequenceSampler samplerB({{0.25f, 0.75f}, {1.0f, 0.25f}});

    const auto sampleA = detector.sampleDebugRay(1, 2, samplerA);
    const auto sampleB = detector.sampleDebugRay(1, 2, samplerB);

    const Vector3f viewDirection{0.0f, 0.0f, 1.0f};
    const Vector3f focusPlanePoint{1.0f, 1.0f, 9.0f};

    auto intersectFocusPlane = [&](const Ray& ray) {
        const float t =
            (focusPlanePoint - ray.origin).dot(viewDirection)
            / ray.direction.dot(viewDirection);
        return ray.origin + t * ray.direction;
    };

    const Vector3f hitA = intersectFocusPlane(sampleA.ray);
    const Vector3f hitB = intersectFocusPlane(sampleB.ray);

    expectNearVector(sampleA.focusPoint, sampleB.focusPoint, 1.0e-4f);
    expectNearVector(hitA, sampleA.focusPoint, 1.0e-4f);
    expectNearVector(hitB, sampleA.focusPoint, 1.0e-4f);
}

TEST(OpenApertureDetectorTest, RaySpectrumIsSourceSpectrumWeightedByDetectorQE) {
    auto detector = makeDetector();
    SequenceSampler sampler({{0.5f, 0.5f}, {0.5f, 0.5f}});

    const Ray ray = detector.sampleRay(2, 2, sampler);
    const Spectrum expected = MultiplySourceSpectrumByDetectorQE(HeNeSpectrum);

    ASSERT_EQ(ray.spectrum.samples.size(), expected.samples.size());
    ASSERT_EQ(ray.spectrum.samples.size(), 1);
    EXPECT_FLOAT_EQ(ray.spectrum.samples[0].wavelengthNm, expected.samples[0].wavelengthNm);
    EXPECT_NEAR(ray.spectrum.samples[0].intensity, expected.samples[0].intensity, 1.0e-6f);
}

TEST(OpenApertureDetectorTest, RejectsDegenerateSceneGeometry) {
    EXPECT_THROW(
        OpenApertureDetector::focused(
            HeNeSpectrum,
            {0.0f, 0.0f, 0.0f},
            {0.0f, 0.0f, 0.0f},
            {0.0f, 1.0f, 0.0f},
            {1.0f, 1.0f, -1.0f},
            0.25f,
            10.0f,
            4,
            4
        ),
        std::invalid_argument
    );

    EXPECT_THROW(
        OpenApertureDetector::focused(
            HeNeSpectrum,
            {0.0f, 0.0f, 0.0f},
            {2.0f, 0.0f, 0.0f},
            {0.0f, 2.0f, 0.0f},
            {1.0f, 1.0f, -1.0f},
            0.25f,
            0.0f,
            4,
            4
        ),
        std::invalid_argument
    );
}
