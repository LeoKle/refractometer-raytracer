#ifndef UTILS_VECTORMATH_VECTOR3_H
#define UTILS_VECTORMATH_VECTOR3_H

#include <cmath>

template <typename T>
struct Vector3 {
    T x{}, y{}, z{};

    Vector3() = default;
    Vector3(T x, T y, T z) : x(x), y(y), z(z) {}

    template <typename U>
    explicit Vector3(const Vector3<U> &v) : x(v.x), y(v.y), z(v.z) {}

    T operator[](int i) const { return i == 0 ? x : i == 1 ? y : z; }
    T &operator[](int i) { return i == 0 ? x : i == 1 ? y : z; }

    template <typename U>
    auto operator+(const Vector3<U> &v) const -> Vector3<decltype(T{} + U{})> {
        return {x + v.x, y + v.y, z + v.z};
    }

    template <typename U>
    auto operator-(const Vector3<U> &v) const -> Vector3<decltype(T{} - U{})> {
        return {x - v.x, y - v.y, z - v.z};
    }

    template <typename U>
    auto operator*(U s) const -> Vector3<decltype(T{} * U{})> {
        return {x * s, y * s, z * s};
    }

    template <typename U>
    auto operator/(U s) const -> Vector3<decltype(T{} / U{})> {
        return {x / s, y / s, z / s};
    }

    Vector3 operator-() const { return {-x, -y, -z}; }

    Vector3 &operator+=(const Vector3 &v) {
        x += v.x;
        y += v.y;
        z += v.z;
        return *this;
    }
    Vector3 &operator-=(const Vector3 &v) {
        x -= v.x;
        y -= v.y;
        z -= v.z;
        return *this;
    }
    Vector3 &operator*=(T s) {
        x *= s;
        y *= s;
        z *= s;
        return *this;
    }
    Vector3 &operator/=(T s) {
        x /= s;
        y /= s;
        z /= s;
        return *this;
    }

    bool operator==(const Vector3 &v) const { return x == v.x && y == v.y && z == v.z; }
    bool operator!=(const Vector3 &v) const { return !(*this == v); }

    template <typename U>
    auto dot(const Vector3<U> &v) const -> decltype(T{} * U{}) {
        return x * v.x + y * v.y + z * v.z;
    }

    template <typename U>
    auto cross(const Vector3<U> &v) const -> Vector3<decltype(T{} * U{})> {
        return {y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x};
    }

    T lengthSq() const { return dot(*this); }
    float length() const { return std::sqrt((float)lengthSq()); }
    Vector3<float> normalized() const {
        float len = length();
        return {x / len, y / len, z / len};
    }
};

template <typename T, typename U>
auto operator*(U s, const Vector3<T> &v) -> Vector3<decltype(T{} * U{})> {
    return v * s;
}

using Vector3i = Vector3<int>;
using Vector3f = Vector3<float>;

#endif