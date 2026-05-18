#ifndef SHAPE_PRISM_H
#define SHAPE_PRISM_H

#include <limits>
#include <memory>

#include "shape/IShape.h"
#include "types/Medium.h"

// A triangular prism defined by a base triangle in the XY plane, extruded along the Z-axis by height.
class Prism : public IShape {
public:
    Prism(Point3f v0, Point3f v1, Point3f v2, float height, Medium& medium)
        : m_v0(v0), m_v1(v1), m_v2(v2), m_height(height), m_medium(medium) {}

    std::optional<SurfaceIntersection> intersect(const Point3f& origin, const Vector3f& direction) const override {
        float tBest = std::numeric_limits<float>::infinity();
        Vector3f nBest;

        // Triangular caps (z = 0 and z = m_height)
        auto intersectCap = [&](float z) {
            // Ray-plane: origin.z + t * direction.z = z
            if (std::abs(direction[2]) < 1e-6f) return;
            float t = (z - origin[2]) / direction[2];
            if (t <= 0.0f || t >= tBest) return;

            Point3f p = origin + t * direction;

            // point-in-triangle test (barycentric, XY only)
            auto sign2D = [](float ax, float ay, float bx, float by, float cx, float cy) {
                return (ax - cx) * (by - cy) - (bx - cx) * (ay - cy);
            };
            float d0 = sign2D(p[0], p[1], m_v0[0], m_v0[1], m_v1[0], m_v1[1]);
            float d1 = sign2D(p[0], p[1], m_v1[0], m_v1[1], m_v2[0], m_v2[1]);
            float d2 = sign2D(p[0], p[1], m_v2[0], m_v2[1], m_v0[0], m_v0[1]);

            bool hasNeg = (d0 < 0) || (d1 < 0) || (d2 < 0);
            bool hasPos = (d0 > 0) || (d1 > 0) || (d2 > 0);
            if (hasNeg && hasPos) return;  // outside of triangle

            tBest = t;
            // normal: -Z for bottom cap, +Z for top cap
            nBest = (z == 0.0f) ? Vector3f(0, 0, -1) : Vector3f(0, 0, 1);
        };

        intersectCap(0.0f);
        intersectCap(m_height);

        // rectangular side faces
        auto intersectSide = [&](const Point3f& a, const Point3f& b) {
            // edge direction and outward normal (perpendicular, pointing away from triangle interior)
            Vector3f edge(b[0] - a[0], b[1] - a[1], 0.0f);
            Vector3f outward(-edge[1], edge[0], 0.0f);  // rotate 90° in XY

            float dDotN = direction[0] * outward[0] + direction[1] * outward[1];
            if (std::abs(dDotN) < 1e-6f) return;  // ray parallel to face

            float oaDotN = (a[0] - origin[0]) * outward[0] + (a[1] - origin[1]) * outward[1];
            float t = oaDotN / dDotN;
            if (t <= 0.0f || t >= tBest) return;

            Point3f p = origin + t * direction;

            // Check within edge extent (0 - 1 along edge) and within height (0 - m_height)
            float edgeLenSq = edge[0] * edge[0] + edge[1] * edge[1];
            float proj = ((p[0] - a[0]) * edge[0] + (p[1] - a[1]) * edge[1]) / edgeLenSq;
            if (proj < 0.0f || proj > 1.0f) return;
            if (p[2] < 0.0f || p[2] > m_height) return;

            tBest = t;
            nBest = outward.normalized();
            // flip normal if it points toward the ray origin (inside hit)
            if (nBest.dot(direction) > 0.0f) nBest = -nBest;
        };

        intersectSide(m_v0, m_v1);
        intersectSide(m_v1, m_v2);
        intersectSide(m_v2, m_v0);

        if (std::isinf(tBest)) return std::nullopt;

        return SurfaceIntersection{origin + tBest * direction, nBest};
    }

    Medium& getMedium(Point3f& /*point*/) override { return m_medium; }

private:
    Point3f m_v0, m_v1, m_v2;
    float m_height;
    Medium& m_medium;
};

#endif