#ifndef SHAPE_PRISM_H
#define SHAPE_PRISM_H

#include <memory>

#include "shape/IShape.h"

// A triangular prism defined by a base triangle in the XY plane, extruded along the Z-axis by height.
class Prism : public IShape {
public:
    Prism(Point3f v0, Point3f v1, Point3f v2, float height) : m_v0(v0), m_v1(v1), m_v2(v2), m_height(height) {
        auto e0 = (m_v1 - m_v0).length();
        auto e1 = (m_v2 - m_v1).length();
        auto e2 = (m_v0 - m_v2).length();
        float triArea = 0.5f * (m_v1 - m_v0).cross(m_v2 - m_v0).length();

        float sideArea0 = e0 * m_height;
        float sideArea1 = e1 * m_height;
        float sideArea2 = e2 * m_height;
        float totalArea = sideArea0 + sideArea1 + sideArea2 + 2.0f * triArea;

        t0 = sideArea0 / totalArea;
        t1 = t0 + sideArea1 / totalArea;
        t2 = t1 + sideArea2 / totalArea;
        t3 = t2 + triArea / totalArea;
    }

    std::optional<SurfaceIntersection> intersect(const Point3f& origin, const Vector3f& direction) const override {
        // TODO
        return std::nullopt;
    }

    Point3f sample(ISampler& sampler) override {
        auto uvw = sampler.next3D();
        float face = uvw[0], u = uvw[1], v = uvw[2];

        auto sampleRect = [&](const Point3f& a, const Point3f& b) -> Point3f {
            Point3f edgePoint = lerp(a, b, u);
            return edgePoint + Vector3f(0, 0, v * m_height);
        };

        auto sampleTri = [&](float sz) -> Point3f {
            float su = std::sqrt(u);
            float a = 1.0f - su;
            float b = v * su;
            return m_v0 + a * (m_v1 - m_v0) + b * (m_v2 - m_v0) + Vector3f(0, 0, sz);
        };

        if (face < t0)
            return sampleRect(m_v0, m_v1);
        else if (face < t1)
            return sampleRect(m_v1, m_v2);
        else if (face < t2)
            return sampleRect(m_v2, m_v0);
        else if (face < t3)
            return sampleTri(0.0f);
        else
            return sampleTri(m_height);
    }

private:
    Point3f m_v0, m_v1, m_v2;
    float m_height;
    float t0, t1, t2, t3;
};

#endif