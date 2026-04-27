#include "PinHoleDetector.h"

PinHoleDetector PinHoleDetector::from(
    Vector3f origin,
    Vector3f bottomRight,
    Vector3f topLeft,
    Vector3f pinHole,
    int width,
    int height
) {
    return PinHoleDetector(
        origin,
        bottomRight - origin,
        topLeft     - origin,
        pinHole,
        width,
        height
    );
}

PinHoleDetector::PinHoleDetector(
    Vector3f origin,
    Vector3f edgeU,
    Vector3f edgeV,
    Vector3f pinHole,
    int width,
    int height
)
    :m_origin{origin}
,m_edgeU{edgeU}
,m_edgeV{edgeV}
,m_pinHole{pinHole}
,m_width{width}
,m_height{height}
{}

Ray PinHoleDetector::sampleRay(int x, int y, ISampler& sampler) const {
    const auto sample = sampler.next2D();

    const float u = (static_cast<float>(x) + sample[0]) / static_cast<float>(m_width);
    const float v = (static_cast<float>(y) + sample[1]) / static_cast<float>(m_height);

    const Vector3f pointOnImagePlane = m_origin + u * m_edgeU + v * m_edgeV;
    const Vector3f direction = (pointOnImagePlane - m_pinHole).normalized();

    return Ray{m_pinHole, direction};
}

int PinHoleDetector::width() const {
    return m_width;
}

int PinHoleDetector::height() const {
    return m_height;
}