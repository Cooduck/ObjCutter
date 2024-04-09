//
// Created by Joe on 8/4/2024.
//

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

ObjFile::ObjFile()
{
    maxX = maxY = maxZ = -1e18;
    minX = minY = minZ = 1e18;
}

bool ObjFile::load(const std::string& filename)
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
            Vector2Texture point{};
            fscanf(file, "%f %f", &point.x, &point.y);
            texturePoints.push_back({point.x, point.y});
        }
        else if (strcmp(type, "vn") == 0)
        {
            Vector3Normal normal{};
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
                faces.back().faces.push_back(face);
            }
            else if (count == 1)
            {
                sscanf(s1, "%d/%d", &face.v1, &face.t1);
                sscanf(s2, "%d/%d", &face.v2, &face.t2);
                sscanf(s3, "%d/%d", &face.v3, &face.t3);
                faces.back().faces.push_back(face);
            }
            else if (count == 2)
            {
                sscanf(s1, "%d/%d/%d", &face.v1, &face.t1, &face.n1);
                sscanf(s2, "%d/%d/%d", &face.v2, &face.t2, &face.n2);
                sscanf(s3, "%d/%d/%d", &face.v3, &face.t3, &face.n3);
                faces.back().faces.push_back(face);
            }
            unsigned long long faceCount = getFaceCount();
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
    elapsedSeconds = end - now;
    return true;
}

void ObjFile::info()
{
    unsigned long long face_count = 0;
    for (const auto& face : faces)
    {
        face_count += face.faces.size();
    }
    std::cout << "Total Lines: "
        << points.size() + texturePoints.size() + normals.size() + faces.size() + face_count + 1
        << std::endl;

    std::cout << "Points: " << points.size() << std::endl;
    std::cout << "Texture Points: " << texturePoints.size() << std::endl;
    std::cout << "Normals: " << normals.size() << std::endl;
    std::cout << "Faces: " << face_count << std::endl;

    std::cout << "Min X Y Z: "
        << minX << " " << minY << " " << minZ
        << std::endl;

    std::cout << "Max X Y Z: "
        << maxX << " " << maxY << " " << maxZ
        << std::endl;

    std::cout << "File Dir: " << fileDir << std::endl;
    std::cout << "Load " << fileName << " in " << elapsedSeconds.count() << "s" << std::endl;
}

bool ObjFile::save(const std::string& filename)
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
        for (const auto& face : faces)
        {
            file << face;
        }
        file.close();
        auto end = std::chrono::system_clock::now();
        std::chrono::duration<double> saveElapsedSeconds = end - now;
        std::cout << "Save " << filename << " in " << saveElapsedSeconds.count() << "s" << std::endl;
        return true;
    }
    return false;
}

void ObjFile::cut()
{
    auto now = std::chrono::system_clock::now();
    // 收集符合条件的点所在的面
    float midX = (minX + maxX) / 2;
    std::set<int> saveFacesIndex;
    for (int i = 0; i < points.size(); i++)
    {
        if (points[i].x > midX)
        {
            int pointIndex = i + 1;
            saveFacesIndex.insert(PointIndex2FaceIndex[pointIndex].begin(),
                                  PointIndex2FaceIndex[pointIndex].end());
        }
    }
    cout << "Save faces: " << saveFacesIndex.size() << endl;

    // 将这些面的点、纹理、法线保存下来
    std::set<int> savePointIndex;
    std::set<int> saveTextureIndex;
    std::map<int, int> oldPointIndex2New;
    std::map<int, int> oldTextureIndex2New;
    // std::vector<int> saveNormalsIndex;

    // 保存点、纹理、法线并创建映射
    for (int index : saveFacesIndex)
    {
        Face* face = getFace(index);
        savePointIndex.insert(face->v1);
        savePointIndex.insert(face->v2);
        savePointIndex.insert(face->v3);
        saveTextureIndex.insert(face->t1);
        saveTextureIndex.insert(face->t2);
        saveTextureIndex.insert(face->t3);
        // saveNormalsIndex.insert(face->n1);
        // saveNormalsIndex.insert(face->n2);
        // saveNormalsIndex.insert(face->n3);
    }
    cout << "Save points: " << savePointIndex.size() << endl;
    cout << "Save texture points: " << saveTextureIndex.size() << endl;
    // cout << "Save normals: " << saveNormalsIndex.size() << endl;

    // write new obj file
    string newFilePath = fileDir + "cut.obj";
    std::ofstream file(newFilePath);
    if (!file.is_open())
    {
        cout << "Failed to open file: " << newFilePath << endl;
        return;
    }
    file << "mtllib " << mtllib << std::endl;
    for (int index : savePointIndex)
    {
        static int newIndex = 0;
        file << "v " << points[index - 1] << std::endl;
        newIndex++;
        oldPointIndex2New[index] = newIndex;
    }
    for (int index : saveTextureIndex)
    {
        static int newIndex = 0;
        file << "vt " << texturePoints[index - 1] << std::endl;
        newIndex++;
        oldTextureIndex2New[index] = newIndex;
    }
    // 重新生成对应的面
    std::vector<MtlFaces> saveMtlFaces;
    int saveMtlIndex = -1;
    for (int index : saveFacesIndex)
    {
        int mtlIndex = getMtlIndex(index);
        if (mtlIndex > saveMtlIndex)
        {
            saveMtlIndex = mtlIndex;
            file << "usemtl " << saveMtlIndex << std::endl;
        }
        Face newFace;
        Face* oldFace = getFace(index);
        newFace.v1 = oldPointIndex2New[oldFace->v1];
        newFace.v2 = oldPointIndex2New[oldFace->v2];
        newFace.v3 = oldPointIndex2New[oldFace->v3];
        newFace.t1 = oldTextureIndex2New[oldFace->t1];
        newFace.t2 = oldTextureIndex2New[oldFace->t2];
        newFace.t3 = oldTextureIndex2New[oldFace->t3];
        file << "f " << newFace << std::endl;
    }
    file.close();
    auto end = std::chrono::system_clock::now();
    std::chrono::duration<double> cutElapsedSeconds = end - now;
    std::cout << "Cut " << newFilePath << ": " << endl
        << "     x > " << midX << endl
        << "    cost time: " << cutElapsedSeconds.count() << "s" << std::endl;
}

Face* ObjFile::getFace(int faceIndex)
{
    if (faces.size() == 0)
        return nullptr;

    int faceNum = 0;
    while (faceNum < faces.size() && faceIndex >= faces[faceNum].faces.size())
    {
        faceIndex -= faces[faceNum].faces.size();
        faceNum++;
    }
    return &faces[faceNum].faces[faceIndex];
}

unsigned long long ObjFile::getFaceCount()
{
    unsigned long long face_count = 0;
    for (const auto& face : faces)
    {
        face_count += face.faces.size();
    }
    return face_count;
}

int ObjFile::getMtlIndex(int faceIndex)
{
    if (faces.size() == 0)
        return -1;

    int faceNum = 0;
    while (faceNum < faces.size() && faceIndex >= faces[faceNum].faces.size())
    {
        faceIndex -= faces[faceNum].faces.size();
        faceNum++;
    }
    return faces[faceNum].mtl;
}

void ObjFile::cmp(string& filename1, string& filename2)
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
