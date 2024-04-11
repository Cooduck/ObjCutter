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


// only for ouput
class ObjModel
{
protected:
    std::string mtllib;
    std::vector<Vector3> points;
    std::vector<Vector2> texturePoints;
    std::vector<Vector3> normals;
    ObjFaces faces;

    float maxX{-1e18}, maxY{-1e18}, maxZ{-1e18};
    float minX{1e18}, minY{1e18}, minZ{1e18};

    std::string fileDir;
    std::string fileName;
    std::chrono::duration<double> loadElapsedSeconds{};

public:
    // base actions
    bool load(const std::string& filename);
    void info();
    bool save(const std::string& filename);
    Vector3 getCenter() const;

    // for setting data
    void setMtllib(const std::string& mtllib);
    void addMtl(string mtlName);
    void addFace(Face face);
    void setPoint(int index, const Vector3& point);
    void setTexturePoint(int index, const Vector2& texturePoint);
    void setNormal(int index, const Vector3& normal);
    void iniitSpaces(int numPoints, int numTexturePoints, int numNormals);
};

class ObjCutter : public ObjModel
{
private:
    std::map<Vector3, int> pointMap;
    std::map<Vector2, int> textureMap;
    std::map<Vector3, int> normalMap;

    int addPoint(const Vector3& point);
    int addTexturePoint(const Vector2& texturePoint);
    int addNormal(const Vector3& normal);

public:
    void cut(const Plane& plane);

    void cutFaceOnePoint(Face face, const Plane& plane, const Vector3& point,
        // output
        Vector3& newPoint1, Vector3& newPoint2,
        Vector2& newTexturePoint1, Vector2& newTexturePoint2);

    void cutFaceTwoPoint(Face face, const Plane& plane, const Vector3& point1, const Vector3& point2,
        // output
        Vector3& newPoint1, Vector3& newPoint2,
        Vector2& newTexturePoint1, Vector2& newTexturePoint2);

    // debug function
    void showTriangleAndTexture(Face* face);

    // 计算直线与平面交点的函数
    static Vector3 getIntersectPoint(const Vector3& p1, const Vector3& p2, const Plane& plane);
};



#endif //OBJCUTTER_H
