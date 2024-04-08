//
// Created by Joe on 8/4/2024.
//

#ifndef TYPES_H
#define TYPES_H

#include <vector>
#include <string>

struct Vector3{
    float x, y, z;
};

struct Vector2Texture {
    float x, y;
};

struct Vector3Normal {
    float x, y, z;
};

struct Face {
    int mtlNum;
    int v1, v2, v3;
    int t1, t2, t3;
    int n1, n2, n3;
};

class ObjFile
{
private:
    std::string mtllib;
    std::vector<Vector3> points;
    std::vector<Vector2Texture> texturePoints;
    std::vector<Vector3Normal> normals;
    std::vector<Face> faces;

public:
    bool load(const std::string& filename);
    void info();
};

#endif //TYPES_H
