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

    virtual bool load(const std::string& filename) {return false;};
    virtual void info() {};
    bool save(const std::string& filename);
    Vector3 getCenter() const;

    void setMtllib(const std::string& mtllib);
    void addMtl(int mtlNum);
    void addFullFace(Face face);
    void addNewFace1(int oldPointIndex, Vector3 newPoint1, Vector3 newPoint2,
        int oldTexturePointIndex, Vector2 newTexturePoint1, Vector2 newTexturePoint2,
        int oldNormalIndex, Vector3 newNormal1, Vector3 newNormal2);
    void addNewFace2(int oldPointIndex1, int oldPointIndex2, Vector3 newPoint,
        int oldTexturePointIndex1, int oldTexturePointIndex2, Vector2 newTexturePoint,
        int oldNormalIndex1, int oldNormalIndex2, Vector3 newNormal);

    unsigned int addPoint(const Vector3& point);
    unsigned int addTexturePoint(const Vector2& texturePoint);
    unsigned int addNormal(const Vector3& normal);

    void setPoint(int index, const Vector3& point);
    void setTexturePoint(int index, const Vector2& texturePoint);
    void setNormal(int index, const Vector3& normal);

    void iniitSpaces(int numPoints, int numTexturePoints, int numNormals);
};

class ObjCutter : public ObjModel
{
private:
    // for cutting speed
    std::map<unsigned int, std::set<unsigned int>> PointIndex2FaceIndex;
    std::map<unsigned int, bool> FaceIndex2IsCut;

public:
    ObjCutter() : ObjModel() {}
    bool load(const std::string& filename) override;
    void info() override;

    // bool isPointNeed();
    // int isFaceNeed();
    void cut(const Plane& plane);
    void cutFace(unsigned int faceIndex, const Plane& plane,
        // output
        std::vector<Face>& newFaces,
        std::vector<Vector3>& newPoints,
        std::vector<Vector2>& newTexturePoints);

    void cutFaceOnePoint(Face face, const Plane& plane, const Vector3& point,
        // output
        Vector3& newPoint1, Vector3& newPoint2,
        Vector2& newTexturePoint1, Vector2& newTexturePoint2);

    void cutFaceTwoPoint(Face face, const Plane& plane, const Vector3& point1, const Vector3& point2,
        // output
        Vector3& newPoint1, Vector3& newPoint2,
        Vector2& newTexturePoint1, Vector2& newTexturePoint2);

    // debug function
    void cmp(string& filename1, string& filename2);
    void showTriangleAndTexture(int faceIndex);

    // 计算直线与平面交点的函数
    static Vector3 getIntersectPoint(const Vector3& p1, const Vector3& p2, const Plane& plane);
};



#endif //OBJCUTTER_H
