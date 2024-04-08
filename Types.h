//
// Created by Joe on 8/4/2024.
//

#ifndef TYPES_H
#define TYPES_H

#include <chrono>
#include <ostream>
#include <vector>
#include <string>

using std::string;

struct Vector3{
    float x, y, z;

    friend std::ostream& operator<<(std::ostream& os, const Vector3& obj)
    {
        return os << obj.x << " " << obj.y << " " << obj.z;
    }
};

struct Vector2Texture {
    float x, y;

    friend std::ostream& operator<<(std::ostream& os, const Vector2Texture& obj)
    {
        return os << obj.x << " " << obj.y;
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
    int v1{}, v2{}, v3{};
    int t1{}, t2{}, t3{};
    int n1{}, n2{}, n3{};
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
    int mtl;
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
public:
    ObjFile();
    bool load(const std::string& filename);
    void info();
    bool save(const std::string& filename);

    // debug function
    void cmp(string& filename1, string& filename2);
};

#endif //TYPES_H
