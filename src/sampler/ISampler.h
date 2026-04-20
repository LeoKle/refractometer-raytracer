#ifndef ISAMPLER_H
#define ISAMPLER_H

#include <array>
#include <memory>

class ISampler {
public:
    enum class DomainKey { Light, Wavelength, Shape };

    virtual ~ISampler() = default;

    virtual float next1D() = 0;
    virtual std::array<float, 2> next2D() = 0;

    virtual std::unique_ptr<ISampler> split(DomainKey key) = 0;
    virtual std::unique_ptr<ISampler> clone() = 0;
};

#endif
