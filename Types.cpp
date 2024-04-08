//
// Created by Joe on 8/4/2024.
//

#include "Types.h"

#include <fstream>
#include <sstream>
#include <algorithm>
#include <iostream>

bool ObjFile::load(const std::string& filename)
{
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
                ss >>mtllib;
            }
            else if (type == "v")
            {
                Vector3 point{};
                ss >> point.x >> point.y >> point.z;
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
            }
            else if (type == "f")
            {
                Face face{};
                face.mtlNum = mtlNumNow;
                std::string s1, s2, s3;
                ss >> s1 >> s2 >> s3;
                // 找到s1中有多少个 /
                int count = std::count(s1.begin(), s1.end(), '/');
                if (count == 0)
                {
                    face.v1 = std::stoi(s1);
                    face.v2 = std::stoi(s2);
                    face.v3 = std::stoi(s3);
                    faces.push_back(face);
                }
                else if (count == 1)
                {
                    face.v1 = std::stoi(std::string(s1, 0, s1.find('/')) );
                    face.v2 = std::stoi(std::string(s2, 0, s2.find('/')) );
                    face.v3 = std::stoi(std::string(s3, 0, s3.find('/')) );
                    face.t1 = std::stoi(std::string(s1, s1.find('/') + 1));
                    face.t2 = std::stoi(std::string(s2, s2.find('/') + 1));
                    face.t3 = std::stoi(std::string(s3, s3.find('/') + 1));
                    faces.push_back(face);
                }
                else if (count == 2)
                {
                    face.v1 = std::stoi(std::string(s1, 0, s1.find('/')) );
                    face.v2 = std::stoi(std::string(s2, 0, s2.find('/')) );
                    face.v3 = std::stoi(std::string(s3, 0, s3.find('/')) );
                    face.t1 = std::stoi(std::string(s1, s1.find('/') + 1, s1.rfind('/')));
                    face.t2 = std::stoi(std::string(s2, s2.find('/') + 1, s2.rfind('/')));
                    face.t3 = std::stoi(std::string(s3, s3.find('/') + 1, s3.rfind('/')));
                    face.n1 = std::stoi(std::string(s1, s1.rfind('/') + 1));
                    face.n2 = std::stoi(std::string(s2, s2.rfind('/') + 1));
                    face.n3 = std::stoi(std::string(s3, s3.rfind('/') + 1));
                    faces.push_back(face);
                }
            }
            else
            {
                // std::cout << line << std::endl;
            }
        }

        file.close();
        return true;
    }
    return false;
}

void ObjFile::info()
{

    std::cout << "Total Lines: "
        << points.size() + texturePoints.size() + normals.size() + faces.size()
        << std::endl;

    std::cout << "Points: " << points.size() << std::endl;
    std::cout << "Texture Points: " << texturePoints.size() << std::endl;
    std::cout << "Normals: " << normals.size() << std::endl;
    std::cout << "Faces: " << faces.size() << std::endl;
}
