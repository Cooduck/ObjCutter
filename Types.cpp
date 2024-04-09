//
// Created by Joe on 8/4/2024.
//

#include <chrono>
#include <cmath>
#include <ostream>
#include <vector>
#include <string>

#include "Types.h"

// Vector2 implementation
Vector2::Vector2(float x_val, float y_val) : x(x_val), y(y_val){}

std::ostream& operator<<(std::ostream& os, const Vector2& obj) {
    return os << obj.x << " " << obj.y;
}

Vector2 Vector2::operator+(const Vector2& other) const {
    return Vector2{x + other.x, y + other.y};
}

Vector2 Vector2::operator-(const Vector2& other) const {
    return Vector2{x - other.x, y - other.y};
}

[[nodiscard]] float Vector2::length() const {
    return std::sqrt(x*x + y*y);
}


// Vector3 implementation
Vector3::Vector3(float x_val, float y_val, float z_val) : x(x_val), y(y_val), z(z_val) {}

std::ostream& operator<<(std::ostream& os, const Vector3& obj) {
    return os << obj.x << " " << obj.y << " " << obj.z;
}

Vector3 Vector3::operator+(const Vector3& other) const {
    return Vector3{x + other.x, y + other.y, z + other.z};
}

Vector3 Vector3::operator-(const Vector3& other) const {
    return Vector3{x - other.x, y - other.y, z - other.z};
}

Vector3 Vector3::operator*(float scalar) const {
    return Vector3{x * scalar, y * scalar, z * scalar};
}

Vector3 Vector3::operator*(const Vector3& other) const {
    return Vector3{x * other.x, y * other.y, z * other.z};
}

[[nodiscard]] float Vector3::length() const {
    return std::sqrt(x*x + y*y + z*z);
}


// Face implementation
std::ostream& operator<<(std::ostream& os, const Face& obj) {
    if (obj.n1 != 0)
        return os
            << obj.v1 << "/" << obj.t1 << "/" << obj.n1
            << " " << obj.v2 << "/" << obj.t2 << "/" << obj.n2
            << " " << obj.v3 << "/" << obj.t3 << "/" << obj.n3;

    if (obj.t1 != 0)
        return os
            << obj.v1 << "/" << obj.t1
            << " " << obj.v2 << "/" << obj.t2
            << " " << obj.v3 << "/" << obj.t3;

    return os
        << obj.v1 << " " << obj.v2 << " " << obj.v3;
}


// MtlFaces implementation
std::ostream& operator<<(std::ostream& os, const MtlFaces& obj) {
    os << "usemtl " << obj.mtl << std::endl;
    for(auto& f : obj.faces) {
        os << "f " << f << std::endl;
    }
    return os;
}


// Plane implementation
Plane::Plane(const Vector3& pt, const Vector3& normal) : center(pt), normal(normal) {
    D = -normal.x * pt.x - normal.y * pt.y - normal.z * pt.z;
}

[[nodiscard]] bool Plane::checkPointSide(Vector3 p) const
{
    return (normal.x * p.x + normal.y * p.y + normal.z * p.z + D) >= 0;
}

[[nodiscard]] float Plane::distance(const Vector3& p) const {
    Vector3 v = normal * p;
    Vector3 cv = normal * center;
    return (v - cv).length();
}


