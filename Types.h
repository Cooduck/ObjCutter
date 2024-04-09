//
// Created by Joe on 8/4/2024.
//

#ifndef TYPES_H
#define TYPES_H

#include <chrono>
#include <cmath>
#include <map>
#include <ostream>
#include <set>
#include <vector>
#include <string>

using std::string;

struct Vector3{
    float x, y, z;

    friend std::ostream& operator<<(std::ostream& os, const Vector3& obj)
    {
        return os << obj.x << " " << obj.y << " " << obj.z;
    }

    Vector3 operator+(const Vector3& other) const {
        return Vector3{x + other.x, y + other.y, z + other.z};
    }

    Vector3 operator-(const Vector3& other) const {
        return Vector3{x - other.x, y - other.y, z - other.z};
    }

    Vector3 operator*(float scalar) const {
        return Vector3{x * scalar, y * scalar, z * scalar};
    }

    Vector3 operator*(const Vector3& other) const {
        return Vector3{x * other.x, y * other.y, z * other.z};
    }

    float length() const {
        return std::sqrt(x*x + y*y + z*z);
    }
};

struct Vector2Texture {
    float x, y;

    friend std::ostream& operator<<(std::ostream& os, const Vector2Texture& obj)
    {
        return os << obj.x << " " << obj.y;
    }

    Vector2Texture operator+(const Vector2Texture& other) const {
        return Vector2Texture{x + other.x, y + other.y};
    }

    Vector2Texture operator-(const Vector2Texture& other) const {
        return Vector2Texture{x - other.x, y - other.y};
    }


    float length() const {
        return std::sqrt(x*x + y*y);
    }
};

struct Vector3Normal {
    float x, y, z;
    friend std::ostream& operator<<(std::ostream& os, const Vector3Normal& obj)
    {
        return os << obj.x << " " << obj.y << " " << obj.z;
    }
};

struct Face {
    unsigned int v1{}, v2{}, v3{};
    unsigned int t1{}, t2{}, t3{};
    unsigned int n1{}, n2{}, n3{};
    friend std::ostream& operator<<(std::ostream& os, const Face& obj)
    {
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
};

struct MtlFaces
{
    unsigned int mtl;
    std::vector<Face> faces;
    friend std::ostream& operator<<(std::ostream& os, const MtlFaces& obj)
    {
        os << "usemtl " << obj.mtl << std::endl;
        for(auto& f : obj.faces) {
            os << "f " << f << std::endl;
        }
        return os;
    }
};

struct Plane {
    Vector3 center;
    Vector3 normal;
    float D;

    Plane(Vector3 pt, Vector3 n) : center(pt), normal(n) {
        D = -n.x * pt.x - n.y * pt.y - n.z * pt.z;
    }

    bool checkPointSide(Vector3 p) const;

    [[nodiscard]] float distance(const Vector3& p) const {
        Vector3 v = normal * p;
        Vector3 cv = normal * center;
        return (v - cv).length();
    }
};

class ObjFile
{
private:
    std::string mtllib;
    std::vector<Vector3> points;
    std::vector<Vector2Texture> texturePoints;
    std::vector<Vector3Normal> normals;
    std::vector<MtlFaces> faces;

    float maxX, maxY, maxZ;
    float minX, minY, minZ;

    std::string fileDir;
    std::string fileName;
    std::chrono::duration<double> elapsedSeconds;

    // for cut
    std::map<unsigned int, std::set<unsigned int>> PointIndex2FaceIndex;
    std::map<unsigned int, bool> FaceIndex2IsCut;

public:
    ObjFile();
    bool load(const std::string& filename);
    void info();
    bool save(const std::string& filename);
    [[nodiscard]] Vector3 getCenter() const;


    // for cut
    void cut(const Plane& plane);
    Face* getFace(unsigned long long  faceIndex);
    unsigned long long getFaceCount();
    int getMtlIndex(int faceIndex);
    void cutFace(unsigned int faceIndex, const Plane& plane,
        // output
        std::vector<Face>& newFaces,
        std::vector<Vector3>& newPoints,
        std::vector<Vector2Texture>& newTexturePoints);

    // debug function
    void cmp(string& filename1, string& filename2);
    void showTriangleAndTexture(int faceIndex);

};

#endif //TYPES_H
