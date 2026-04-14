#ifndef UTILS_VECTORMATH_VECTOR2_H
#define UTILS_VECTORMATH_VECTOR2_H

#include <cmath>

template <typename T>
struct Vector2 {
    T x{}, y{};

    Vector2() = default;
    Vector2(T x, T y) : x(x), y(y) {}

    template <typename U>
    explicit Vector2(const Vector2<U>& v) : x(v.x), y(v.y) {}

    T operator[](int i) const { return i == 0 ? x : y; }
    T& operator[](int i) { return i == 0 ? x : y; }

    template <typename U>
    auto operator+(const Vector2<U>& v) const -> Vector2<decltype(T{} + U{})> {
        return {x + v.x, y + v.y};
    }

    template <typename U>
    auto operator-(const Vector2<U>& v) const -> Vector2<decltype(T{} - U{})> {
        return {x - v.x, y - v.y};
    }

    template <typename U>
    auto operator*(U s) const -> Vector2<decltype(T{} * U{})> {
        return {x * s, y * s};
    }

    template <typename U>
    auto operator/(U s) const -> Vector2<decltype(T{} / U{})> {
        return {x / s, y / s};
    }

    Vector2 operator-() const { return {-x, -y}; }

    Vector2& operator+=(const Vector2& v) {
        x += v.x;
        y += v.y;
        return *this;
    }

    Vector2& operator-=(const Vector2& v) {
        x -= v.x;
        y -= v.y;
        return *this;
    }

    Vector2& operator*=(T s) {
        x *= s;
        y *= s;
        return *this;
    }

    Vector2& operator/=(T s) {
        x /= s;
        y /= s;
        return *this;
    }

    bool operator==(const Vector2& v) const { return x == v.x && y == v.y; }
    bool operator!=(const Vector2& v) const { return !(*this == v); }

    template <typename U>
    auto dot(const Vector2<U>& v) const -> decltype(T{} * U{}) {
        return x * v.x + y * v.y;
    }

    // 2D "cross product" → scalar (z-component)
    template <typename U>
    auto cross(const Vector2<U>& v) const -> decltype(T{} * U{}) {
        return x * v.y - y * v.x;
    }

    T lengthSq() const { return dot(*this); }
    float length() const { return std::sqrt((float)lengthSq()); }

    Vector2<float> normalized() const {
        float len = length();
        return {x / len, y / len};
    }
};

// scalar * vector
template <typename T, typename U>
auto operator*(U s, const Vector2<T>& v) -> Vector2<decltype(T{} * U{})> {
    return v * s;
}

using Vector2i = Vector2<int>;
using Vector2f = Vector2<float>;

#endif