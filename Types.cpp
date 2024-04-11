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

Vector2 Vector2::operator*(float scalar) const
{
    return Vector2{x * scalar, y * scalar};
}

Vector2 Vector2::operator/(float scalar) const
{
    return Vector2{x / scalar, y / scalar};
}

float Vector2::length() const {
    return std::sqrt(x*x + y*y);
}


// Vector3 implementation
Vector3::Vector3(float x_val, float y_val, float z_val) : x(x_val), y(y_val), z(z_val) {}

bool operator<(const Vector2& lhs, const Vector2& rhs)
{
    if (lhs.x < rhs.x)
        return true;
    if (rhs.x < lhs.x)
        return false;
    return lhs.y < rhs.y;
}

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

bool operator<(const Vector3& lhs, const Vector3& rhs)
{
    if (lhs.x < rhs.x)
        return true;
    if (rhs.x < lhs.x)
        return false;
    if (lhs.y < rhs.y)
        return true;
    if (rhs.y < lhs.y)
        return false;
    return lhs.z < rhs.z;
}

float Vector3::length() const {
    return std::sqrt(x*x + y*y + z*z);
}

bool Vector3::equals(const Vector3& other) const
{
    return (x == other.x && y == other.y && z == other.z);
}

float Vector3::dot(const Vector3& other) const
{
    return x * other.x + y * other.y + z * other.z;
}

Vector3 Vector3::normalize()
{
    float len = length();
    return Vector3{x / len, y / len, z / len};
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

std::ostream& operator<<(std::ostream& os, const ObjFaces& obj)
{
    for (const auto& mtlFace : obj.mtlFaces)
    {
        os << mtlFace;
    }
    return os;
}


MtlFaces& ObjFaces::operator[](unsigned int mtlIndex)
{
    return mtlFaces[mtlIndex];
}

unsigned int ObjFaces::getNumFaces() const
{
    return numFaces;
}

void ObjFaces::push_back(const Face& face)
{
    mtlFaces.back().faces.push_back(face);
    numFaces++;
}

void ObjFaces::push_back(const MtlFaces& mtlFace)
{
    mtlFaces.push_back(mtlFace);
    numFaces += mtlFace.faces.size();
}

bool ObjFaces::empty()
{
    return mtlFaces.empty();
}

MtlFaces& ObjFaces::back()
{
    return mtlFaces.back();
}

unsigned int ObjFaces::size()
{
    return mtlFaces.size();
}

// Plane implementation
Plane::Plane(const Vector3& pt, const Vector3& normal) : center(pt), normal(normal) {
    this->normal = this->normal.normalize();
    D = -normal.x * pt.x - normal.y * pt.y - normal.z * pt.z;
}

bool Plane::checkPointSide(Vector3 p) const
{
    return (normal.x * p.x + normal.y * p.y + normal.z * p.z + D) >= 0;
}

float Plane::distance(const Vector3& p) const {
    Vector3 v = normal * p;
    Vector3 cv = normal * center;
    return (v - cv).length();
}


