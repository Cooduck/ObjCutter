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

ObjCutter::ObjCutter()
{
    maxX = maxY = maxZ = -1e18;
    minX = minY = minZ = 1e18;
}

bool ObjCutter::load(const std::string& filename)
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
    int mtlNumNow = 0;
    char type[40];
    while (fscanf(file, "%s", type) != EOF)
    {
        if (strcmp(type, "mtllib") == 0)
        {
            char mtllibBuff[200];
            fscanf(file, "%s", mtllibBuff);
            mtllib = string(mtllibBuff);
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
            fscanf(file, "%d", &mtlNumNow);
            MtlFaces facesNow;
            facesNow.mtl = mtlNumNow;
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
            unsigned int faceCount = faces.getNumFaces();
            PointIndex2FaceIndex[face.v1].insert(faceCount);
            PointIndex2FaceIndex[face.v2].insert(faceCount);
            PointIndex2FaceIndex[face.v3].insert(faceCount);
        }
        else
        {
            std::cout  << "unknown type: " << type << std::endl;
        }
    }

    fclose(file);
    auto end = std::chrono::system_clock::now();
    loadElapsedSeconds = end - now;
    return true;
}

void ObjCutter::info()
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

bool ObjCutter::save(const std::string& filename)
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
        return true;
    }
    return false;
}

Vector3 ObjCutter::getCenter() const
{
    Vector3 center{};
    center.x = (minX + maxX) / 2;
    center.y = (minY + maxY) / 2;
    center.z = (minZ + maxZ) / 2;
    return center;
}

void ObjCutter::cut(const Plane& plane)
{
    auto now = std::chrono::system_clock::now();
    // 收集符合条件的点所在的面
    float midX = (minX + maxX) / 2;
    std::set<unsigned int> saveFacesIndex;
    std::vector<Face> edgeFaces;
    std::vector<Vector3> edgePoints;
    std::vector<Vector2> edgeTexturePoints;
    for (int i = 0; i < points.size(); i++)
    {
        if (plane.checkPointSide(points[i]))
        {
            int pointIndex = i + 1;
            for (unsigned int index : PointIndex2FaceIndex[pointIndex])
            {
                if (FaceIndex2IsCut[index])
                    continue;

                Face* face = faces.getFace(index);
                const auto& p1 = points[face->v1 - 1];
                const auto& p2 = points[face->v2 - 1];
                const auto& p3 = points[face->v3 - 1];
                if (plane.checkPointSide(p1) &&
                    plane.checkPointSide(p2) &&
                    plane.checkPointSide(p3))
                {
                    saveFacesIndex.insert(index);
                    FaceIndex2IsCut[index] = true;
                }
                else
                {
                    cutFace(index, plane, edgeFaces, edgePoints, edgeTexturePoints);
                    FaceIndex2IsCut[index] = true;
                }
            }
            // saveFacesIndex.insert(PointIndex2FaceIndex[pointIndex].begin(),
            //                       PointIndex2FaceIndex[pointIndex].end());
        }
    }
    cout << "Save faces: " << saveFacesIndex.size() << endl;

    // 将这些面的点、纹理、法线保存下来
    std::set<unsigned int> savePointIndex;
    std::set<unsigned int> saveTextureIndex;
    std::set<unsigned int> saveNormalsIndex;
    std::map<unsigned int, unsigned int> oldPointIndex2New;
    std::map<unsigned int, unsigned int> oldTextureIndex2New;
    std::map<unsigned int, unsigned int> oldNormalsIndex2New;

    // 保存点、纹理、法线并创建映射
    for (int index : saveFacesIndex)
    {
        Face* face = faces.getFace(index);
        savePointIndex.insert(face->v1);
        savePointIndex.insert(face->v2);
        savePointIndex.insert(face->v3);
        if (face->t1 > 0)
        {
            saveTextureIndex.insert(face->t1);
            saveTextureIndex.insert(face->t2);
            saveTextureIndex.insert(face->t3);
        }
        if (face->n1 > 0)
        {
            saveNormalsIndex.insert(face->n1);
            saveNormalsIndex.insert(face->n2);
            saveNormalsIndex.insert(face->n3);
        }
    }
    cout << "Save points: " << savePointIndex.size() << endl;
    cout << "Save texture points: " << saveTextureIndex.size() << endl;
    cout << "Save normals: " << saveNormalsIndex.size() << endl;

    // write new obj file
    string newFilePath = fileDir + "cut.obj";
    std::ofstream file(newFilePath);
    if (!file.is_open())
    {
        cout << "Failed to open file: " << newFilePath << endl;
        return;
    }
    file << "mtllib " << mtllib << std::endl;

    // save points
    static unsigned int newPointIndex = 0;
    for (unsigned int index : savePointIndex)
    {
        file << "v " << points[index - 1] << std::endl;
        newPointIndex++;
        oldPointIndex2New[index] = newPointIndex;
    }
    for (auto point : edgePoints)
    {
        file << "v " << point << std::endl;
    }

    // save texture points
    static unsigned int newTextureIndex = 0;
    for (unsigned int index : saveTextureIndex)
    {
        file << "vt " << texturePoints[index - 1] << std::endl;
        newTextureIndex++;
        oldTextureIndex2New[index] = newTextureIndex;
    }
    for (auto tpoint : edgeTexturePoints)
    {
        file << "vt " << tpoint << std::endl;
    }

    // save normals
    for (unsigned int index : saveNormalsIndex)
    {
        static int newIndex = 0;
        file << "vn " << normals[index - 1] << std::endl;
        newIndex++;
        oldNormalsIndex2New[index] = newIndex;
    }

    // 重新生成对应的面
    std::vector<MtlFaces> saveMtlFaces;
    int saveMtlIndex = -1;
    for (unsigned int index : saveFacesIndex)
    {
        int mtlIndex = faces.getMtlIndex(index);
        if (mtlIndex > saveMtlIndex)
        {
            saveMtlIndex = mtlIndex;
            file << "usemtl " << saveMtlIndex << std::endl;
        }
        Face newFace;
        Face* oldFace = faces.getFace(index);
        newFace.v1 = oldPointIndex2New[oldFace->v1];
        newFace.v2 = oldPointIndex2New[oldFace->v2];
        newFace.v3 = oldPointIndex2New[oldFace->v3];
        newFace.t1 = oldTextureIndex2New[oldFace->t1];
        newFace.t2 = oldTextureIndex2New[oldFace->t2];
        newFace.t3 = oldTextureIndex2New[oldFace->t3];
        newFace.n1 = oldNormalsIndex2New[oldFace->n1];
        newFace.n2 = oldNormalsIndex2New[oldFace->n2];
        newFace.n3 = oldNormalsIndex2New[oldFace->n3];
        file << "f " << newFace << std::endl;
    }
    for (auto face : edgeFaces)
    {
        file << "f " << face << std::endl;
    }

    file.close();
    auto end = std::chrono::system_clock::now();
    cutElapsedSeconds = end - now;
    std::cout << "Cut " << newFilePath << ": " << endl
        << "     point > " << plane.center << plane.normal << endl
        << "    cost time: " << cutElapsedSeconds.count() << "s" << std::endl;
}

void ObjCutter::cutFace(unsigned int faceIndex, const Plane& plane, std::vector<Face>& newFaces, std::vector<Vector3>& newPoints,
    std::vector<Vector2>& newTexturePoints)
{
    Face* face = faces.getFace(faceIndex);
    const auto& p1 = points[face->v1 - 1];
    const auto& p2 = points[face->v2 - 1];
    const auto& p3 = points[face->v3 - 1];
    const auto& t1 = texturePoints[face->t1 - 1];
    const auto& t2 = texturePoints[face->t2 - 1];
    const auto& t3 = texturePoints[face->t3 - 1];

    bool p1side = plane.checkPointSide(p1);
    bool p2side = plane.checkPointSide(p2);
    bool p3side = plane.checkPointSide(p3);
    if (p1side == p2side && p2side == p3side)
    {
        cout << "The face is ??? " << faceIndex << endl;
    }

    float d1 = plane.distance(p1);
    if (!p1side)
        d1 = -d1;
    float d2 = plane.distance(p2);
    if (!p2side)
        d2 = -d2;
    float d3 = plane.distance(p3);
    if (!p3side)
        d3 = -d3;

    // 输出三个顶点与平面的距离
    // cout << "Distance: " << d1 << " " << d2 << " " << d3 << endl;
}

void ObjCutter::cmp(string& filename1, string& filename2)
{
    std::ifstream file1(filename1);
    std::ifstream file2(filename2);
    bool isSame = true;
    // 比较两个文件不一样的地方
    if (file1.is_open() && file2.is_open())
    {
        std::string line1;
        std::string line2;
        int i = 0;
        while (std::getline(file1, line1) && std::getline(file2, line2))
        {
            if (line1 != line2)
            {
                cout << "Line " << i << " is different:" << endl;
                cout << "Line1: " << line1 << endl;
                cout << "Line2: " << line2 << endl;
                isSame = false;
            }
            i++;
        }
    }
    if (isSame)
    {
        cout << "The two files are the same." << endl;
    }
    file1.close();
    file2.close();
}

void ObjCutter::showTriangleAndTexture(int faceIndex)
{
    Face* face = faces.getFace(faceIndex);
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

