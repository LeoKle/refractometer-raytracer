#ifndef UTILS_VECTORMATH_POINT3_H
#define UTILS_VECTORMATH_POINT3_H

#include <algorithm>
#include <cmath>

#include "Vector3.h"

template <typename T>
struct Point3 {
    T x{}, y{}, z{};

    Point3() = default;
    Point3(T x, T y, T z) : x(x), y(y), z(z) {}

    template <typename U>
    explicit Point3(const Point3<U>& p) : x(p.x), y(p.y), z(p.z) {}

    T operator[](int i) const { return i == 0 ? x : i == 1 ? y : z; }
    T& operator[](int i) { return i == 0 ? x : i == 1 ? y : z; }

    // Point + Vector -> Point
    template <typename U>
    auto operator+(const Vector3<U>& v) const -> Point3<decltype(T{} + U{})> {
        return {x + v.x, y + v.y, z + v.z};
    }
    template <typename U>
    Point3& operator+=(const Vector3<U>& v) {
        x += v.x;
        y += v.y;
        z += v.z;
        return *this;
    }

    // Point - Vector -> Point
    template <typename U>
    auto operator-(const Vector3<U>& v) const -> Point3<decltype(T{} - U{})> {
        return {x - v.x, y - v.y, z - v.z};
    }

    // Point - Point -> Vector (displacement)
    template <typename U>
    auto operator-(const Point3<U>& p) const -> Vector3<decltype(T{} - U{})> {
        return {x - p.x, y - p.y, z - p.z};
    }

    bool operator==(const Point3& p) const { return x == p.x && y == p.y && z == p.z; }
    bool operator!=(const Point3& p) const { return !(*this == p); }

    float distanceTo(const Point3& p) const { return (*this - p).length(); }
};

// Vector + Point -> Point (commutative form)
template <typename T, typename U>
auto operator+(const Vector3<T>& v, const Point3<U>& p) -> Point3<decltype(T{} + U{})> {
    return p + v;
}

template <typename T>
Point3<T> lerp(const Point3<T>& a, const Point3<T>& b, float t) {
    return a + t * (b - a);
}

using Point3f = Point3<float>;
using Point3i = Point3<int>;

#endif