#ifndef MEDIUM_H
#define MEDIUM_H

#include <array>
#include <cmath>

class Medium {
public:
    virtual ~Medium() = default;

    virtual float refractiveIndex(float wavelength_um) const = 0;
};

class SellmeierMedium : public Medium {
public:
    /// @brief
    /// @param b Sellmeier B coefficients
    /// @param c Sellmeier C coefficients
    SellmeierMedium(const std::array<float, 3>& b, const std::array<float, 3>& c) : sellmeier_b(b), sellmeier_c(c) {}
    ~SellmeierMedium() = default;

    inline float refractiveIndex(float wavelength_um) const {
        float lambda2 = wavelength_um * wavelength_um;

        float n2 = 1.0;

        for (int i = 0; i < 3; ++i) {
            n2 += (sellmeier_b[i] * lambda2) / (lambda2 - sellmeier_c[i]);
        }

        return std::sqrt(n2);
    }

private:
    std::array<float, 3> sellmeier_b;
    std::array<float, 3> sellmeier_c;
};

// based on: https://refractiveindex.info/?shelf=popular_glass&book=BK7&page=SCHOTT
inline const SellmeierMedium NBK7({1.03961212f, 0.231792344f, 1.01046945f},
                                  {0.00600069867f, 0.0200179144f, 103.560653f});

class AirMedium : public Medium {
public:
    float refractiveIndex(float wavelength_um) const override {
        (void)wavelength_um;
        return 1.000277f;
    }
};

#endif