#include "SlitLight.h"
#include <utility>

SlitLight SlitLight::from(
    Vector3f origin,
    Vector3f bottomRight,
    Vector3f topLeft,
    Spectrum   spectrum
) {
    return SlitLight(
        origin,
        bottomRight - origin,
        topLeft     - origin,
        std::move(spectrum)
    );
}

SlitLight::SlitLight(
    Vector3f origin,
    Vector3f edgeU,
    Vector3f edgeV,
    Spectrum   spectrum
)
    :m_origin{origin}
    ,m_edgeU{edgeU}
    ,m_edgeV{edgeV}
    ,m_spectrum{std::move(spectrum)}
{}

Vector3f SlitLight::samplePoint(ISampler& sampler) {
    const auto sample = sampler.next2D();
    const float u = sample[0];
    const float v = sample[1];
    return m_origin + u * m_edgeU + v * m_edgeV;
}

const Spectrum& SlitLight::spectrum() const {
    return m_spectrum;
}
