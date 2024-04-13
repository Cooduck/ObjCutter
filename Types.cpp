//
// Created by Joe on 8/4/2024.
//

#include <chrono>
#include <cmath>
#include <ostream>
#include <vector>
#include <string>

#include "Types.h"

#include <iostream>

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
    if (!obj.mtl.empty())
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
    if (mtlFaces.empty())
    {
        mtlFaces.emplace_back(MtlFaces{});
    }
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

bool Plane::isInside(const Vector3& point) const
{
    return (normal.x * point.x + normal.y * point.y + normal.z * point.z + D) >= 0;
}

Vector3 Plane::getIntersectPoint(const Vector3& p1, const Vector3& p2) const
{
    Vector3 lineDirection = p2 - p1;

    // 计算直线与平面的交点
    float denom = normal.dot(lineDirection);
    if (std::abs(denom) > 0.0001) { // 避免除以零的情况
        Vector3 diff = center - p1;
        float t = diff.dot(normal) / denom;
        return p1 + lineDirection * t;
    }
    std::cout << "直线与平面平行或共线，无交点！" << std::endl;
    exit(-3);
}

Vector3 Box::getIntersectPoint(const Vector3& p1, const Vector3& p2) const
{

    Vector3 lineDirection = p2 - p1;

    // 计算直线与平面的交点
    float tmin = (minPoint.x - p1.x) / lineDirection.x;
    float tmax = (maxPoint.x - p1.x) / lineDirection.x;
    float tymin = (minPoint.y - p1.y) / lineDirection.y;
    float tymax = (maxPoint.y - p1.y) / lineDirection.y;

    if (tmin > tymax || tymin > tmax) {
        return Vector3{0, 0, 0};
    }

    if (tymin > tmin) {
        tmin = tymin;
    }
    if (tymax < tmax) {
        tmax = tymax;
    }

    float tzmin = (minPoint.z - p1.z) / lineDirection.z;
    float tzmax = (maxPoint.z - p1.z) / lineDirection.z;

    if (tmin > tzmax || tzmin > tmax) {
        return Vector3{0, 0, 0};
    }

    if (tzmin > tmin) {
        tmin = tzmin;
    }
    if (tzmax < tmax) {
        tmax = tzmax;
    }

    return p1 + lineDirection * tmin;
}

TriangleStatus::TriangleStatus(const Vector3* triangle, const Area& area)
{
    status = 0;
    inpartNum = 0;
    if (area.isInside(triangle[0])) {
        status |= 1;
        inpartNum++;
    }
    if (area.isInside(triangle[1])) {
        status |= 2;
        inpartNum++;
    }
    if (area.isInside(triangle[2])) {
        status |= 4;
        inpartNum++;
    }
    singleIndex = 0;
    if (status == 1 || (~status & 7) == 1)
    {
        singleIndex = 1;
    }
    else if (status == 2 || (~status & 7) == 2)
    {
        singleIndex = 2;
    }
    else if (status == 4 || (~status & 7) == 4)
    {
        singleIndex = 3;
    }
}

bool TriangleStatus::isFull() const
{
    return status == 7;
}

bool TriangleStatus::isOut() const
{
    return status == 0;
}

bool TriangleStatus::isPart() const
{
    return singleIndex != 0;
}

unsigned short TriangleStatus::getSingleIndex() const
{
    return singleIndex;
}

unsigned short TriangleStatus::getInpartNum() const
{
    return inpartNum;
}


