//
// Created by Joe on 8/4/2024.
//

#include "Types.h"

#include <fstream>
#include <sstream>
#include <algorithm>
#include <chrono>
#include <iostream>

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
    std::ifstream file(filename);
    if (file.is_open())
    {
        std::string line;
        int mtlNumNow = 0;
        while (std::getline(file, line))
        {
            // std::cout << line << std::endl;
            // 将这一行解析
            std::stringstream ss(line);
            std::string type;
            ss >> type;
            if (type == "mtllib")
            {
                ss >> mtllib;
            }
            else if (type == "v")
            {
                Vector3 point{};
                ss >> point.x >> point.y >> point.z;
                minX = point.x < minX ? point.x : minX;
                minY = point.y < minY ? point.y : minY;
                minZ = point.z < minZ ? point.z : minZ;
                maxX = point.x > maxX ? point.x : maxX;
                maxY = point.y > maxY ? point.y : maxY;
                maxZ = point.z > maxZ ? point.z : maxZ;
                if (point.x > 20000)
                {
                    std::cout << "x too large: " << line << std::endl;
                }
                points.push_back(point);
            }
            else if (type == "vt")
            {
                Vector2Texture point{};
                ss >> point.x >> point.y;
                texturePoints.push_back({point.x, point.y});
            }
            else if (type == "vn")
            {
                Vector3Normal normal{};
                ss >> normal.x >> normal.y >> normal.z;
                normals.push_back(normal);
            }
            else if (type == "usemtl")
            {
                ss >> mtlNumNow;
                MtlFaces facesNow;
                facesNow.mtl = mtlNumNow;
                faces.push_back(facesNow);
            }
            else if (type == "f")
            {

                Face face{};
                std::string s1, s2, s3;
                ss >> s1 >> s2 >> s3;
                // 找到s1中有多少个 /
                int count = std::count(s1.begin(), s1.end(), '/');
                if (count == 0)
                {
                    face.v1 = std::stoi(s1);
                    face.v2 = std::stoi(s2);
                    face.v3 = std::stoi(s3);
                    faces.back().faces.push_back(face);
                }
                else if (count == 1)
                {
                    face.v1 = std::stoi(std::string(s1, 0, s1.find('/')));
                    face.v2 = std::stoi(std::string(s2, 0, s2.find('/')));
                    face.v3 = std::stoi(std::string(s3, 0, s3.find('/')));
                    face.t1 = std::stoi(std::string(s1, s1.find('/') + 1));
                    face.t2 = std::stoi(std::string(s2, s2.find('/') + 1));
                    face.t3 = std::stoi(std::string(s3, s3.find('/') + 1));
                    faces.back().faces.push_back(face);
                }
                else if (count == 2)
                {
                    face.v1 = std::stoi(std::string(s1, 0, s1.find('/')));
                    face.v2 = std::stoi(std::string(s2, 0, s2.find('/')));
                    face.v3 = std::stoi(std::string(s3, 0, s3.find('/')));
                    face.t1 = std::stoi(std::string(s1, s1.find('/') + 1, s1.rfind('/')));
                    face.t2 = std::stoi(std::string(s2, s2.find('/') + 1, s2.rfind('/')));
                    face.t3 = std::stoi(std::string(s3, s3.find('/') + 1, s3.rfind('/')));
                    face.n1 = std::stoi(std::string(s1, s1.rfind('/') + 1));
                    face.n2 = std::stoi(std::string(s2, s2.rfind('/') + 1));
                    face.n3 = std::stoi(std::string(s3, s3.rfind('/') + 1));
                    faces.back().faces.push_back(face);
                }
                unsigned long long faceCount = getFaceCount();
                PointIndex2FaceIndex[face.v1].insert( faceCount);
                PointIndex2FaceIndex[face.v2].insert( faceCount);
                PointIndex2FaceIndex[face.v3].insert( faceCount);
            }
            else
            {
                // std::cout << line << std::endl;
            }
        }

        file.close();
        auto end = std::chrono::system_clock::now();
        elapsedSeconds = end - now;
        return true;
    }
    return false;
}

void ObjFile::info()
{
    unsigned long long face_count = 0;
    for (const auto& face : faces)
    {
        face_count += face.faces.size();
    }
    std::cout << "Total Lines: "
        << points.size() + texturePoints.size() + normals.size() + faces.size() + face_count +1
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
            file << "v "<< point << std::endl;
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
            // file << "usemtl " << face.mtl << std::endl;
            // for (const auto& f : face.faces)
            // {
            //     file << "f " << f << std::endl;
            // }
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
            int pointIndex = i+1;
            saveFacesIndex.insert(PointIndex2FaceIndex[pointIndex].begin(),
                PointIndex2FaceIndex[pointIndex].end());
        }
    }
    cout << "Save faces: " << saveFacesIndex.size() << endl;

    // 将这些面的点、纹理、法线保存下来
    std::set<int> savePointIndexSet;
    std::set<int> saveTextureIndexSet;
    std::map<int, int> oldPointIndex2New;
    std::map<int, int> oldTextureIndex2New;
    // std::vector<int> saveNormalsIndex;

    // 保存点、纹理、法线并创建映射
    for (int index : saveFacesIndex)
    {
        Face* face = getFace(index);
        if (savePointIndexSet.find(face->v1) == savePointIndexSet.end())
        {
            savePointIndexSet.insert(face->v1);
            oldPointIndex2New[face->v1] = static_cast<int>(savePointIndexSet.size());
        }
        if (savePointIndexSet.find(face->v2) == savePointIndexSet.end())
        {
            savePointIndexSet.insert(face->v2);
            oldPointIndex2New[face->v2] = static_cast<int>(savePointIndexSet.size());
        }
        if (savePointIndexSet.find(face->v3) == savePointIndexSet.end())
        {
            savePointIndexSet.insert(face->v3);
            oldPointIndex2New[face->v3] = static_cast<int>(savePointIndexSet.size());
        }

        if (saveTextureIndexSet.find(face->t1) == saveTextureIndexSet.end())
        {
            saveTextureIndexSet.insert(face->t1);
            oldTextureIndex2New[face->t1] = static_cast<int>(saveTextureIndexSet.size());
        }
        if (saveTextureIndexSet.find(face->t2) == saveTextureIndexSet.end())
        {
            saveTextureIndexSet.insert(face->t2);
            oldTextureIndex2New[face->t2] = static_cast<int>(saveTextureIndexSet.size());
        }
        if (saveTextureIndexSet.find(face->t3) == saveTextureIndexSet.end())
        {
            saveTextureIndexSet.insert(face->t3);
            oldTextureIndex2New[face->t3] = static_cast<int>(saveTextureIndexSet.size());
        }
    }
    cout << "Save points: " << savePointIndexSet.size() << endl;
    cout << "Save texture points: " << saveTextureIndexSet.size() << endl;

    // 重新生成对应的面
    std::vector<MtlFaces> saveMtlFaces;
    int saveMtlIndex = -1;
    for (int index : saveFacesIndex)
    {
        int mtlIndex = getMtlIndex(index);
        if (mtlIndex > saveMtlIndex)
        {
            saveMtlIndex = mtlIndex;
            MtlFaces facesNow;
            facesNow.mtl = mtlIndex;
            saveMtlFaces.push_back(facesNow);
        }
        Face newFace;
        newFace.v1 = oldPointIndex2New[getFace(index)->v1];
        newFace.v2 = oldPointIndex2New[getFace(index)->v2];
        newFace.v3 = oldPointIndex2New[getFace(index)->v3];
        newFace.t1 = oldTextureIndex2New[getFace(index)->t1];
        newFace.t2 = oldTextureIndex2New[getFace(index)->t2];
        newFace.t3 = oldTextureIndex2New[getFace(index)->t3];
        saveMtlFaces.back().faces.push_back(newFace);
    }


    // write new obj file
    string newFilePath = fileDir + "cut.obj";
    std::ofstream file(newFilePath);
    if (!file.is_open())
    {
        cout << "Failed to open file: " << newFilePath << endl;
        return;
    }
    file << "mtllib " << mtllib << std::endl;
    for (int i = 0; i < savePointIndexSet.size(); i++)
    {
        file << "v " << points[i] << std::endl;
    }
    for (int i = 0; i < saveTextureIndexSet.size(); i++)
    {
        file << "vt " << texturePoints[i] << std::endl;
    }
    for (const auto& face : saveMtlFaces)
    {
        file << "usemtl " << face.mtl << std::endl;
        for (const auto& f : face.faces)
        {
            file << "f " << f << std::endl;
        }
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
