#ifndef SAMPLER_H
#define SAMPLER_H

#include <oqmc/pmjbn.h>

#include <array>
#include <memory>
#include <vector>

#include "sampler/ISampler.h"

class OqmcPmjBnSampler : public ISampler {
public:
    static std::shared_ptr<std::vector<char>> createCache() {
        auto cache = std::make_shared<std::vector<char>>(oqmc::PmjBnSampler::cacheSize);
        oqmc::PmjBnSampler::initialiseCache(cache->data());
        return cache;
    }

    OqmcPmjBnSampler(int x, int y, int frame, int index, std::shared_ptr<std::vector<char>> cache)
        : m_x(x), m_y(y), m_frame(frame), m_index(index), m_sharedCache(cache) {
        m_domain = oqmc::PmjBnSampler(x, y, frame, index, m_sharedCache->data());
    }

    float next1D() override {
        float v[1];
        m_domain.drawSample<1>(v);
        return v[0];
    }

    std::array<float, 2> next2D() override {
        float v[2];
        m_domain.drawSample<2>(v);
        return {v[0], v[1]};
    }

    std::unique_ptr<ISampler> split(DomainKey key) override {
        auto child = m_domain.newDomain(static_cast<uint32_t>(key));
        return std::unique_ptr<OqmcPmjBnSampler>(new OqmcPmjBnSampler(child, m_sharedCache));
    }

    std::unique_ptr<ISampler> clone() override {
        return std::make_unique<OqmcPmjBnSampler>(m_x, m_y, m_frame, m_index, m_sharedCache);
    }

private:
    OqmcPmjBnSampler(oqmc::PmjBnSampler domain, std::shared_ptr<std::vector<char>> cache)
        : m_x(0), m_y(0), m_frame(0), m_index(0), m_domain(domain), m_sharedCache(cache) {}

    int m_x, m_y, m_frame, m_index;
    std::shared_ptr<std::vector<char>> m_sharedCache;
    oqmc::PmjBnSampler m_domain;
};

#endif