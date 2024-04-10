//
// Created by Joe on 8/4/2024.
//

#ifndef TYPES_H
#define TYPES_H

using std::string;

struct Vector2 {
    float x, y;

    explicit Vector2(float x_val = 0.0f, float y_val = 0.0f);

    friend std::ostream& operator<<(std::ostream& os, const Vector2& obj);

    Vector2 operator+(const Vector2& other) const;

    Vector2 operator-(const Vector2& other) const;

    Vector2 operator*(float scalar) const;

    Vector2 operator/(float scalar) const;

    friend bool operator<(const Vector2& lhs, const Vector2& rhs);

    float length() const;
};

struct Vector3 {
    float x, y, z;

    explicit Vector3(float x_val = 0.0f, float y_val = 0.0f, float z_val = 0.0f);

    friend std::ostream& operator<<(std::ostream& os, const Vector3& obj);

    Vector3 operator+(const Vector3& other) const;

    Vector3 operator-(const Vector3& other) const;

    Vector3 operator*(float scalar) const;

    Vector3 operator*(const Vector3& other) const;

    friend bool operator<(const Vector3& lhs, const Vector3& rhs);

    float length() const;

    bool equals(const Vector3& other) const;

    float dot(const Vector3& other) const;

    Vector3 normalize();
};

struct Face {
    unsigned int v1{}, v2{}, v3{};
    unsigned int t1{}, t2{}, t3{};
    unsigned int n1{}, n2{}, n3{};

    friend std::ostream& operator<<(std::ostream& os, const Face& obj);
};

struct MtlFaces {
    unsigned int mtl;
    std::vector<Face> faces;

    friend std::ostream& operator<<(std::ostream& os, const MtlFaces& obj);
};

struct ObjFaces
{
    std::vector<MtlFaces> mtlFaces;
    unsigned int numFaces{0};

    friend std::ostream& operator<<(std::ostream& os, const ObjFaces& obj);

    // []
    MtlFaces& operator[](unsigned int mtlIndex);

    Face* getFace(unsigned int faceIndex);

    unsigned int getMtlIndex(unsigned int faceIndex);

    unsigned int getNumFaces() const;

    void push_back(const Face& face);
    void push_back(const MtlFaces& mtlFace);

    bool empty();

    MtlFaces& back();

    unsigned int size();
};

struct Plane {
    Vector3 center;
    Vector3 normal;
    float D;

    Plane(const Vector3& pt, const Vector3& normal);

    bool checkPointSide(Vector3 p) const;

    float distance(const Vector3& p) const;
};
#endif //TYPES_H
