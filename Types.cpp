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
    fileDir = filename.substr(0, filename.find_last_of("/"));
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
