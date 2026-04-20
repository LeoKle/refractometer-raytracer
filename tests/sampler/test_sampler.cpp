#include <gtest/gtest.h>
#include <oqmc/pmjbn.h>

#include "sampler/Sampler.h"

class OqmcPmjBnSamplerTest : public ::testing::Test {
protected:
    void SetUp() override {
        auto cache = OqmcPmjBnSampler::createCache();
        m_sampler = std::make_unique<OqmcPmjBnSampler>(0, 0, 0, 0, cache);
    }

    std::unique_ptr<OqmcPmjBnSampler> m_sampler;
};

TEST_F(OqmcPmjBnSamplerTest, CloneProducesSameSequence) {
    auto cloned = m_sampler->clone();

    for (int i = 0; i < 10; i++) {
        EXPECT_EQ(m_sampler->next1D(), cloned->next1D());
    }
}

TEST_F(OqmcPmjBnSamplerTest, CloneProducesSame2DSequence) {
    auto cloned = m_sampler->clone();

    for (int i = 0; i < 10; i++) {
        EXPECT_EQ(m_sampler->next2D(), cloned->next2D());
    }
}

TEST_F(OqmcPmjBnSamplerTest, CloneCapturesMidSequenceState) {
    m_sampler->next1D();
    m_sampler->next1D();
    auto cloned = m_sampler->clone();
    EXPECT_EQ(m_sampler->next1D(), cloned->next1D());
    EXPECT_EQ(m_sampler->next2D(), cloned->next2D());
}

TEST_F(OqmcPmjBnSamplerTest, CloneReturnsCorrectType) {
    auto cloned = m_sampler->clone();
    EXPECT_NE(dynamic_cast<OqmcPmjBnSampler*>(cloned.get()), nullptr);
}

TEST_F(OqmcPmjBnSamplerTest, SplitReturnsCorrectType) {
    auto split = m_sampler->split(ISampler::DomainKey::Light);
    EXPECT_NE(dynamic_cast<OqmcPmjBnSampler*>(split.get()), nullptr);
}

TEST_F(OqmcPmjBnSamplerTest, SplitProducesDifferentSequence) {
    auto split = m_sampler->split(ISampler::DomainKey::Light);
    EXPECT_NE(m_sampler->next1D(), split->next1D());
}