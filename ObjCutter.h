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
    std::chrono::duration<float> loadElapsedSeconds{};
    Vector3 blockCenter;   // 切出来的块的中心点

public:
    ObjModel() = default;
    ObjModel(const std::string& filedir) : fileDir(filedir) {};
    // base actions
    bool load(const std::string& filename);
    void info();
    bool save(const std::string& fileName, const double & model_minz);
    bool empty() const;

    Vector3 getCenter() const;
    Vector3 getBottomCenter() const;
    Vector3 getMinPoint() const;
    Vector3 getMaxPoint() const;
    void setblockCenter(float x, float y, float z);
    Vector3 getblockCenter() const;
    string getDir() const;

    // for setting data
    void setMtllib(const std::string& mtllib);
    void addMtl(string mtlName);
    void addFace(Face face);
    void setPoint(int index, const Vector3& point);
    void setTexturePoint(int index, const Vector2& texturePoint);
    void setNormal(int index, const Vector3& normal);
    void initSpaces(int numPoints, int numTexturePoints, int numNormals);
};

class ObjCutter : public ObjModel
{
private:
    std::map<Vector3, int> pointMap;
    std::map<Vector2, int> textureMap;
    std::map<Vector3, int> normalMap;

    std::map<Vector3, int> pointMap1;
    std::map<Vector2, int> textureMap1;
    std::map<Vector3, int> normalMap1;
    std::map<Vector3, int> pointMap2;
    std::map<Vector2, int> textureMap2;
    std::map<Vector3, int> normalMap2;

    int addPoint(const Vector3& point);
    int addTexturePoint(const Vector2& texturePoint);
    int addNormal(const Vector3& normal);

    int addPoint(const Vector3& point, int op);
    int addTexturePoint(const Vector2& texturePoint, int op);
    int addNormal(const Vector3& normal, int op);

public:
    ObjCutter() = default;
    ObjCutter(const std::string& filedir) : ObjModel(filedir) {};
    std::unique_ptr<ObjCutter> cut(const Area& area);
    void cut(const Area& area, ObjCutter* & cuttedModel1, ObjCutter* & cuttedModel2);

    // 计算两点与域的交点函数
    static void cutFace(const Area& area, const Vector3* triangle,
                const Vector2* triangleTexture, const Vector3* normal, const TriangleStatus& status,
                // output
                Vector3& newPoint1, Vector3& newPoint2,
                Vector2& newTexturePoint1, Vector2& newTexturePoint2,
                Vector3& newNormal1, Vector3& newNormal2);
};


#endif //OBJCUTTER_H
