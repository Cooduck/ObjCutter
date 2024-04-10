//
// Created by Joe on 9/4/2024.
//

#ifndef OBJCUTTER_H
#define OBJCUTTER_H

#include <chrono>
#include <set>
#include <map>
#include <string>
#include <vector>

#include "Types.h"

class ObjCutter
{
private:
    std::string mtllib;
    std::vector<Vector3> points;
    std::vector<Vector2> texturePoints;
    std::vector<Vector3> normals;
    ObjFaces faces;

    float maxX, maxY, maxZ;
    float minX, minY, minZ;

    std::string fileDir;
    std::string fileName;
    std::chrono::duration<double> loadElapsedSeconds{};
    std::chrono::duration<double> cutElapsedSeconds{};

    // for cutting speed
    std::map<unsigned int, std::set<unsigned int>> PointIndex2FaceIndex;
    std::map<unsigned int, bool> FaceIndex2IsCut;

public:
    ObjCutter();
    bool load(const std::string& filename);
    void info();
    bool save(const std::string& filename);

    void cut(const Plane& plane);
    void cutFace(unsigned int faceIndex, const Plane& plane,
        // output
        std::vector<Face>& newFaces,
        std::vector<Vector3>& newPoints,
        std::vector<Vector2>& newTexturePoints);
    Vector3 getCenter() const;

    // debug function
    void cmp(string& filename1, string& filename2);
    void showTriangleAndTexture(int faceIndex);
};



#endif //OBJCUTTER_H
