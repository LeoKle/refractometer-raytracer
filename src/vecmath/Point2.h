#ifndef UTILS_VECTORMATH_POINT2_H
#define UTILS_VECTORMATH_POINT2_H

#include <algorithm>
#include <cmath>

#include "vecmath/Vector2.h"

template <typename T>
struct Point2 {
    T x{}, y{};

    Point2() = default;
    Point2(T x, T y) : x(x), y(y) {}

    template <typename U>
    explicit Point2(const Point2<U>& p) : x(p.x), y(p.y) {}

    T operator[](int i) const { return i == 0 ? x : y; }
    T& operator[](int i) { return i == 0 ? x : y; }

    // Point + Vector -> Point
    template <typename U>
    auto operator+(const Vector2<U>& v) const -> Point2<decltype(T{} + U{})> {
        return {x + v.x, y + v.y};
    }

    template <typename U>
    Point2& operator+=(const Vector2<U>& v) {
        x += v.x;
        y += v.y;
        return *this;
    }

    // Point - Vector -> Point
    template <typename U>
    auto operator-(const Vector2<U>& v) const -> Point2<decltype(T{} - U{})> {
        return {x - v.x, y - v.y};
    }

    // Point - Point -> Vector
    template <typename U>
    auto operator-(const Point2<U>& p) const -> Vector2<decltype(T{} - U{})> {
        return {x - p.x, y - p.y};
    }

    bool operator==(const Point2& p) const { return x == p.x && y == p.y; }
    bool operator!=(const Point2& p) const { return !(*this == p); }

    float distanceTo(const Point2& p) const { return (*this - p).length(); }
};

// Vector + Point -> Point (commutative)
template <typename T, typename U>
auto operator+(const Vector2<T>& v, const Point2<U>& p) -> Point2<decltype(T{} + U{})> {
    return p + v;
}

// lerp
template <typename T>
Point2<T> lerp(const Point2<T>& a, const Point2<T>& b, float t) {
    return a + t * (b - a);
}

using Point2f = Point2<float>;
using Point2i = Point2<int>;

#endif