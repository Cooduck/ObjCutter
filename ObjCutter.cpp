//
// Created by Joe on 9/4/2024.
//

#include "ObjCutter.h"
#include "Types.h"

#include <fstream>
#include <sstream>
#include <algorithm>
#include <chrono>
#include <iostream>
#include <string.h>

using std::cout;
using std::endl;
using std::string;

bool ObjModel::save(const std::string& filename)
{
    std::ofstream file(filename);
    if (file.is_open())
    {
        auto now = std::chrono::system_clock::now();
        file << "mtllib " << mtllib << std::endl;
        for (const auto& point : points)
        {
            file << "v " << point << std::endl;
        }
        for (const auto& tpoint : texturePoints)
        {
            file << "vt " << tpoint << std::endl;
        }
        for (const auto& normal : normals)
        {
            file << "vn " << normal << std::endl;
        }
        file << faces;
        file.close();
        auto end = std::chrono::system_clock::now();
        std::chrono::duration<double> saveElapsedSeconds = end - now;
        std::cout << "Save " << filename << " in " << saveElapsedSeconds.count() << "s" << std::endl;
        std::cout << "Total " << points.size() << " points, "
                  << texturePoints.size() << " texture points, "
                  << normals.size() << " normals, "
                  << faces.getNumFaces() << " faces." << std::endl;
        return true;
    }
    return false;
}

Vector3 ObjModel::getCenter() const
{
    Vector3 center{};
    center.x = (minX + maxX) / 2;
    center.y = (minY + maxY) / 2;
    center.z = (minZ + maxZ) / 2;
    return center;
}

void ObjModel::setMtllib(const std::string& mtllib)
{
    this->mtllib = mtllib;
}

void ObjModel::addMtl(string mtlName)
{
    MtlFaces facesNow;
    facesNow.mtl = mtlName;
    faces.push_back(facesNow);
}

void ObjModel::setPoint(int index, const Vector3& point)
{
    points[index - 1] = point;
}

void ObjModel::setTexturePoint(int index, const Vector2& texturePoint)
{
    texturePoints[index - 1] = texturePoint;
}

void ObjModel::setNormal(int index, const Vector3& normal)
{
    normals[index - 1] = normal;
}

void ObjModel::iniitSpaces(int numPoints, int numTexturePoints, int numNormals)
{
    points.reserve(numPoints);
    points.resize(numPoints);
    texturePoints.reserve(numTexturePoints);
    texturePoints.resize(numTexturePoints);
    normals.reserve(numNormals);
    normals.resize(numNormals);
}

void ObjModel::addFace(Face face)
{
    faces.push_back(face);
}

bool ObjModel::load(const std::string& filename)
{
    fileDir = filename.substr(0, filename.find_last_of("/") + 1);
    fileName = filename.substr(filename.find_last_of("/") + 1);

    auto now = std::chrono::system_clock::now();
    FILE* file = fopen(filename.c_str(), "r");
    if (file == nullptr)
    {
        std::cout << "Error: Cannot open file " << filename << std::endl;
        return false;
    }

    cout << "Loading " << filename << "..." << endl;
    char type[40];
    while (fscanf(file, "%s", type) != EOF)
    {
        if (strcmp(type, "mtllib") == 0)
        {
            fgetc(file);
            char mtllibBuff[200];
            fgets(mtllibBuff, sizeof(mtllibBuff), file);
            mtllib = string(mtllibBuff);
            mtllib.erase(mtllib.find_last_not_of('\n') + 1);
        }
        else if (strcmp(type, "v") == 0)
        {
            Vector3 point{};
            fscanf(file, "%f %f %f", &point.x, &point.y, &point.z);
            minX = point.x < minX ? point.x : minX;
            minY = point.y < minY ? point.y : minY;
            minZ = point.z < minZ ? point.z : minZ;
            maxX = point.x > maxX ? point.x : maxX;
            maxY = point.y > maxY ? point.y : maxY;
            maxZ = point.z > maxZ ? point.z : maxZ;
            points.push_back(point);
        }
        else if (strcmp(type, "vt") == 0)
        {
            Vector2 point{};
            fscanf(file, "%f %f", &point.x, &point.y);
            texturePoints.emplace_back(point.x, point.y);
        }
        else if (strcmp(type, "vn") == 0)
        {
            Vector3 normal{};
            fscanf(file, "%f %f %f", &normal.x, &normal.y, &normal.z);
            normals.push_back(normal);
        }
        else if (strcmp(type, "usemtl") == 0)
        {
            fgetc(file);
            char mtlBuff[200];
            fgets(mtlBuff, sizeof(mtlBuff), file);
            MtlFaces facesNow;
            facesNow.mtl = string(mtlBuff);
            facesNow.mtl.erase(facesNow.mtl.find_last_not_of('\n') + 1);
            faces.push_back(facesNow);
        }
        else if (strcmp(type, "f") == 0)
        {
            Face face{};
            // std::string s1, s2, s3;
            char s1[50], s2[50], s3[50];
            fscanf(file, "%s %s %s", s1, s2, s3);
            // 找到s1中有多少个 /
            int count = std::count(s1, s1 + strlen(s1), '/');
            if (count == 0)
            {
                face.v1 = std::stoi(s1);
                face.v2 = std::stoi(s2);
                face.v3 = std::stoi(s3);
                faces.push_back(face);
            }
            else if (count == 1)
            {
                sscanf(s1, "%d/%d", &face.v1, &face.t1);
                sscanf(s2, "%d/%d", &face.v2, &face.t2);
                sscanf(s3, "%d/%d", &face.v3, &face.t3);
                faces.push_back(face);
            }
            else if (count == 2)
            {
                sscanf(s1, "%d/%d/%d", &face.v1, &face.t1, &face.n1);
                sscanf(s2, "%d/%d/%d", &face.v2, &face.t2, &face.n2);
                sscanf(s3, "%d/%d/%d", &face.v3, &face.t3, &face.n3);
                faces.push_back(face);
            }
        }
        else
        {
            while (fgetc(file)!= '\n');
            std::cout  << "unknown type: " << type << std::endl;
        }
    }

    fclose(file);
    auto end = std::chrono::system_clock::now();
    loadElapsedSeconds = end - now;
    return true;
}

void ObjModel::info()
{
    unsigned int faceCount = faces.getNumFaces();
    std::cout << "Total Lines: "
        << points.size() + texturePoints.size() + normals.size() + faces.mtlFaces.size() + faceCount + 1
        << std::endl;

    std::cout << "Points: " << points.size() << std::endl;
    std::cout << "Texture Points: " << texturePoints.size() << std::endl;
    std::cout << "Normals: " << normals.size() << std::endl;
    std::cout << "Faces: " << faceCount << std::endl;

    std::cout << "Min X Y Z: "
        << minX << " " << minY << " " << minZ
        << std::endl;

    std::cout << "Max X Y Z: "
        << maxX << " " << maxY << " " << maxZ
        << std::endl;

    std::cout << "File Dir: " << fileDir << std::endl;
    std::cout << "Load " << fileName << " in " << loadElapsedSeconds.count() << "s" << std::endl;
}

int ObjCutter::addPoint(const Vector3& point)
{
    static int index = 0;
    auto it = pointMap.find(point);
    if (it == pointMap.end())
    {
        pointMap[point] = ++index;
        return index;
    }
    return it->second;
}

int ObjCutter::addTexturePoint(const Vector2& texturePoint)
{
    static int index = 0;
    auto it = textureMap.find(texturePoint);
    if (it == textureMap.end())
    {
        textureMap[texturePoint] = ++index;
        return index;
    }
    return it->second;
}

int ObjCutter::addNormal(const Vector3& normal)
{
    static int index = 0;
    auto it = normalMap.find(normal);
    if (it == normalMap.end())
    {
        normalMap[normal] = ++index;
        return index;
    }
    return it->second;
}

void ObjCutter::cut(const Plane& plane)
{
    auto begin = std::chrono::system_clock::now();

    ObjModel cuttedModel;
    cuttedModel.setMtllib(mtllib);

    // collecting faces
    for (const auto& mtlFace : faces.mtlFaces)
    {
        MtlFaces cuttingMtlFace;
        cuttingMtlFace.mtl = mtlFace.mtl;
        cuttedModel.addMtl(mtlFace.mtl);

        for (auto face : mtlFace.faces)
        {
            std::vector<Vector3> pointsInside;
            const auto& p1 = points[face.v1 - 1];
            const auto& p2 = points[face.v2 - 1];
            const auto& p3 = points[face.v3 - 1];
            int pointInsideCounter = 0;
            if (plane.checkPointSide(p1))
            {
                pointInsideCounter++;
                pointsInside.push_back(p1);
            }
            if (plane.checkPointSide(p2))
            {
                pointInsideCounter++;
                pointsInside.push_back(p2);
            }
            if (plane.checkPointSide(p3))
            {
                pointInsideCounter++;
                pointsInside.push_back(p3);
            }

            // 全部在里面的情况
            if (pointInsideCounter == 3)
            {
                face.v1 = addPoint(p1);
                face.v2 = addPoint(p2);
                face.v3 = addPoint(p3);
                if (face.t1 > 0)
                {
                    face.t1 = addTexturePoint(texturePoints[face.t1 - 1]);
                    face.t2 = addTexturePoint(texturePoints[face.t2 - 1]);
                    face.t3 = addTexturePoint(texturePoints[face.t3 - 1]);
                }
                if (face.n1 > 0)
                {
                    face.n1 = addNormal(normals[face.n1 - 1]);
                    face.n2 = addNormal(normals[face.n2 - 1]);
                    face.n3 = addNormal(normals[face.n3 - 1]);
                }
                cuttedModel.addFace(face);
                continue;
            }

            // 若三点都在外面，则跳过
            if (pointInsideCounter == 0)
            {
                continue;
            }

            // 其它情况
            if (pointInsideCounter == 1)
            {
                Vector3 newPoint1, newPoint2;
                Vector2 newTexture1, newTexture2;
                cutFaceOnePoint(face, plane, pointsInside[0],
                                newPoint1, newPoint2, newTexture1, newTexture2);
                face.v1 = addPoint(pointsInside[0]);
                face.v2 = addPoint(newPoint1);
                face.v3 = addPoint(newPoint2);
                if (face.t1 > 0)
                {
                    face.t1 = addTexturePoint(texturePoints[face.t1 - 1]);
                    face.t2 = addTexturePoint(newTexture1);
                    face.t3 = addTexturePoint(newTexture2);
                }
                cuttedModel.addFace(face);
                continue;
            }

            if (pointInsideCounter == 2)
            {
                // 若不为原序，则交换两个点
                // if (pointsInside[0].equals(p1) && pointsInside[1].equals(p3))
                // {
                //     Vector3 temp = pointsInside[0];
                //     pointsInside[0] = pointsInside[1];
                //     pointsInside[1] = temp;
                // }

                Vector3 newPoint1, newPoint2;
                Vector2 newTexture1, newTexture2;
                Face newFace1, newFace2;
                cutFaceTwoPoint(face, plane, pointsInside[0], pointsInside[1],
                                newPoint1, newPoint2, newTexture1, newTexture2);
                newFace1.v1 = addPoint(pointsInside[0]);
                newFace1.v2 = addPoint(pointsInside[1]);
                newFace1.v3 = addPoint(newPoint1);
                newFace2.v1 = addPoint(pointsInside[1]);
                newFace2.v2 = addPoint(newPoint2);
                newFace2.v3 = addPoint(newPoint1);
                if (face.t1 > 0)
                {
                    newFace1.t1 = addTexturePoint(texturePoints[face.t1 - 1]);
                    newFace1.t2 = addTexturePoint(texturePoints[face.t2 - 1]);
                    newFace1.t3 = addTexturePoint(newTexture1);
                    newFace2.t1 = addTexturePoint(texturePoints[face.t2 - 1]);
                    newFace2.t2 = addTexturePoint(newTexture2);
                    newFace2.t3 = addTexturePoint(newTexture1);
                }
                cuttedModel.addFace(newFace1);
                cuttedModel.addFace(newFace2);
                continue;
            }
        }
    }

    cuttedModel.iniitSpaces(pointMap.size(), textureMap.size(), normalMap.size());
    for (const auto& kv : pointMap)
    {
        auto point = kv.first;
        auto index = kv.second;
        cuttedModel.setPoint(index, point);
    }
    pointMap.clear();

    for (const auto& kv : textureMap)
    {
        auto texturePoint = kv.first;
        auto index = kv.second;
        cuttedModel.setTexturePoint(index, texturePoint);
    }
    textureMap.clear();

    for (const auto& kv : normalMap)
    {
        auto normal = kv.first;
        auto index = kv.second;
        cuttedModel.setNormal(index, normal);
    }
    normalMap.clear();

    std::chrono::duration<double> cutElapsedSeconds = std::chrono::system_clock::now() - begin;
    std::cout << "Cut time: " << cutElapsedSeconds.count() << "s" << std::endl;
    std::cout << "Plant: " << plane.center << " " << plane.normal << std::endl;

    string newFilePath = fileDir + "cut.obj";
    cuttedModel.save(newFilePath);
}

void ObjCutter::cutFaceOnePoint(Face face, const Plane& plane, const Vector3& point, Vector3& newPoint1,
    Vector3& newPoint2, Vector2& newTexturePoint1, Vector2& newTexturePoint2)
{
    Vector3 localPoints[3];
    Vector2 localTexturePoints[3];
    localPoints[0] = points[face.v1 - 1];
    localPoints[1] = points[face.v2 - 1];
    localPoints[2] = points[face.v3 - 1];
    localTexturePoints[0] = texturePoints[face.t1 - 1];
    localTexturePoints[1] = texturePoints[face.t2 - 1];
    localTexturePoints[2] = texturePoints[face.t3 - 1];

    Vector3 pSingle, p1, p2;
    Vector2 tSingle, t1, t2;
    pSingle = point;
    for (int i = 0; i < 3; i++)
    {
        if (localPoints[i].equals(pSingle))
        {
            tSingle = localTexturePoints[i];
            if (i == 1)
            {
                p1 = localPoints[(i + 2) % 3];
                p2 = localPoints[(i + 1) % 3];
                t1 = localTexturePoints[(i + 2) % 3];
                t2 = localTexturePoints[(i + 1) % 3];
            }
            else
            {
                p1 = localPoints[(i + 1) % 3];
                p2 = localPoints[(i + 2) % 3];
                t1 = localTexturePoints[(i + 1) % 3];
                t2 = localTexturePoints[(i + 2) % 3];
            }
            break;
        }
    }

    // 计算单一点与其他两点的连线与平面的两个交点
    newPoint1 = getIntersectPoint(pSingle, p1, plane);
    newPoint2 = getIntersectPoint(pSingle, p2, plane);

    // 用相似三角形计算新的纹理坐标
    float rate1 = (pSingle - p1).length() /
        (newPoint1 - p1).length();
    float rate2 = (pSingle - p2).length() /
        (newPoint2 - p2).length();
    newTexturePoint1 = t1 + (tSingle - t1) / rate1;
    newTexturePoint2 = t2 + (tSingle - t2) / rate2;
}

void ObjCutter::cutFaceTwoPoint(Face face, const Plane& plane, const Vector3& point1, const Vector3& point2,
    Vector3& newPoint1, Vector3& newPoint2, Vector2& newTexturePoint1, Vector2& newTexturePoint2)
{
    Vector3 localPoints[3];
    Vector2 localTexturePoints[3];
    localPoints[0] = points[face.v1 - 1];
    localPoints[1] = points[face.v2 - 1];
    localPoints[2] = points[face.v3 - 1];
    localTexturePoints[0] = texturePoints[face.t1 - 1];
    localTexturePoints[1] = texturePoints[face.t2 - 1];
    localTexturePoints[2] = texturePoints[face.t3 - 1];

    Vector3 pSingle, p1{point1}, p2{point2};
    Vector2 tSingle, t1, t2;
    for (int i = 0; i < 3; i++)
    {
        if (localPoints[i].equals(point1))
        {
            t1 = localTexturePoints[i];
        }
        else if (localPoints[i].equals(point2))
        {
            t2 = localTexturePoints[i];
        }
        else
        {
            pSingle = localPoints[i];
            tSingle = localTexturePoints[i];
        }
    }

    // 计算单一点与其他两点的连线与平面的两个交点
    newPoint1 = getIntersectPoint(pSingle, point1, plane);
    newPoint2 = getIntersectPoint(pSingle, point2, plane);

    // 用相似三角形计算新的纹理坐标
    float rate1 = (pSingle - p1).length() /
        (newPoint1 - p1).length();
    float rate2 = (pSingle - p2).length() /
        (newPoint2 - p2).length();
    newTexturePoint1 = t1 + (tSingle - t1) / rate1;
    newTexturePoint2 = t2 + (tSingle - t2) / rate2;
}

void ObjCutter::showTriangleAndTexture(Face* face)
{
    const auto& p1 = points[face->v1 - 1];
    const auto& p2 = points[face->v2 - 1];
    const auto& p3 = points[face->v3 - 1];
    const auto& t1 = texturePoints[face->t1 - 1];
    const auto& t2 = texturePoints[face->t2 - 1];
    const auto& t3 = texturePoints[face->t3 - 1];
    cout << "Triangle: " << endl
        << p1 << endl
        << p2 << endl
        << p3 << endl;
    cout << "Texture: " << endl
        << t1 << endl
        << t2 << endl
        << t3 << endl;
    // 输出三角形边长
    float len1 = (p2 - p1).length();
    float len2 = (p3 - p2).length();
    float len3 = (p1 - p3).length();
    cout << "Triangle length: " << len1 << " " << len2 << " " << len3 << endl;
    // 输出材质边长
    float texLen1 = (t2 - t1).length();
    float texLen2 = (t3 - t2).length();
    float texLen3 = (t1 - t3).length();
    cout << "Texture length: " << texLen1 << " " << texLen2 << " " << texLen3 << endl;

}

Vector3 ObjCutter::getIntersectPoint(const Vector3& p1, const Vector3& p2, const Plane& plane)
{
    Vector3 lineDirection = p2 - p1;

    // 计算直线与平面的交点
    float denom = plane.normal.dot(lineDirection);
    if (std::abs(denom) > 0.0001) { // 避免除以零的情况
        Vector3 diff = plane.center - p1;
        float t = diff.dot(plane.normal) / denom;
        return p1 + lineDirection * t;
    } else {
        std::cerr << "直线与平面平行或共线，无交点！" << std::endl;
        return Vector3(); // 或者其他适当的处理方式
    }
}

