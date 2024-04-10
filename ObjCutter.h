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

class ObjModel
{
protected:
    std::string mtllib;
    std::vector<Vector3> points;
    std::vector<Vector2> texturePoints;
    std::vector<Vector3> normals;
    ObjFaces faces;

    float maxX{}, maxY{}, maxZ{};
    float minX{}, minY{}, minZ{};

    std::string fileDir;
    std::string fileName;
    std::chrono::duration<double> loadElapsedSeconds{};

public:
    ObjModel();
    virtual bool load(const std::string& filename) = 0;
    virtual void info() = 0;
    bool save(const std::string& filename);
    Vector3 getCenter() const;
};

class ObjCutter : public ObjModel
{
private:
    std::chrono::duration<double> cutElapsedSeconds{};

    // for cutting speed
    std::map<unsigned int, std::set<unsigned int>> PointIndex2FaceIndex;
    std::map<unsigned int, bool> FaceIndex2IsCut;

public:
    ObjCutter() : ObjModel() {}
    bool load(const std::string& filename) override;
    void info() override;

    void cut(const Plane& plane);
    void cutFace(unsigned int faceIndex, const Plane& plane,
        // output
        std::vector<Face>& newFaces,
        std::vector<Vector3>& newPoints,
        std::vector<Vector2>& newTexturePoints);

    // debug function
    void cmp(string& filename1, string& filename2);
    void showTriangleAndTexture(int faceIndex);

    // 计算直线与平面交点的函数
    static Vector3 getIntersectPoint(const Vector3& p1, const Vector3& p2, const Plane& plane);
};



#endif //OBJCUTTER_H
