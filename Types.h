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

    string Vector2_to_string() const;
};

struct Vector3 {
    float x, y, z;

    explicit Vector3(float x_val = 0.0f, float y_val = 0.0f, float z_val = 0.0f);

    friend std::ostream& operator<<(std::ostream& os, const Vector3& obj);

    Vector3 operator+(const Vector3& other) const;

    Vector3 operator-(const Vector3& other) const;

    Vector3 operator*(float scalar) const;

    Vector3 operator*(const Vector3& other) const;

    friend bool operator==(const Vector3& lhs, const Vector3& rhs);

    friend bool operator!=(const Vector3& lhs, const Vector3& rhs);

    friend bool operator<(const Vector3& lhs, const Vector3& rhs);

    float length() const;

    bool equals(const Vector3& other) const;

    float dot(const Vector3& other) const;

    Vector3 normalize();

    string Vector3_to_string() const;
};

struct Face {
    unsigned int v1{}, v2{}, v3{};
    unsigned int t1{}, t2{}, t3{};
    unsigned int n1{}, n2{}, n3{};

    friend std::ostream& operator<<(std::ostream& os, const Face& obj);

    string Face_to_string()const;
};

struct MtlFaces {
    string mtl{};
    std::vector<Face> faces;

    friend std::ostream& operator<<(std::ostream& os, const MtlFaces& obj);
};

struct ObjFaces
{
    std::vector<MtlFaces> mtlFaces;
    unsigned int numFaces{0};

    friend std::ostream& operator<<(std::ostream& os, const ObjFaces& obj);

    MtlFaces& operator[](unsigned int mtlIndex);

    unsigned int getNumFaces() const;

    void push_back(const Face& face);
    void push_back(const MtlFaces& mtlFace);

    bool empty();

    MtlFaces& back();

    unsigned int size();
};

struct Area
{
    virtual ~Area() = default;
    virtual bool isInside(const Vector3& point) const = 0;
    virtual Vector3 getIntersectPoint(const Vector3& p1, const Vector3& p2) const = 0;
};

struct Plane : Area{
    // Plane为平面，Ax + Bx + Cx + D = 0
    Vector3 center;
    Vector3 normal; // 平面单位法向量（A,B,C）
    float D;

    Plane(const Vector3& pt, const Vector3& normal);

    bool isInside(const Vector3& point) const override;
    Vector3 getIntersectPoint(const Vector3& p1, const Vector3& p2) const override;
};

struct Box : Area{
    Vector3 minPoint, maxPoint;

    Box(const Vector3& minPt, const Vector3& maxPt);

    bool isInside(const Vector3& point) const override;
    Vector3 getIntersectPoint(const Vector3& p1, const Vector3& p2) const override;
};

struct TriangleStatus
{
    unsigned short status;
    unsigned short singleIndex;
    unsigned short inpartNum;

    TriangleStatus(const Vector3* triangle, const Area& area);
    unsigned short getSingleIndex() const;
    unsigned short getInpartNum() const;
    bool isFull() const;
    bool isOut() const;
    bool isPart() const;
};
#endif //TYPES_H
