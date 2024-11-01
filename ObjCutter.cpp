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
#include <filesystem>
#include <codecvt>
#include <locale>
#include <cstdio>
#include <thread>
#include <future>
#include <cctype>
#include <unordered_map>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/stat.h>
#endif


using std::cout;
using std::endl;
using std::string;
using std::wstring;

// 将 std::string 转换为 std::wstring
std::wstring string_to_wstring(const std::string& str) {
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    return converter.from_bytes(str);
}


std::string wstring_to_string(const std::wstring& wstr) {
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    return converter.to_bytes(wstr);
}

bool hasIllegalCharacters(const std::string &input, char &illegal_char) {
    for (char ch : input) {
        // 如果字符不是数字、斜杠或空格，返回 true
        if (!std::isdigit(ch) && ch != '/' && !std::isspace(ch) && ch != '-' && ch != '.' && ch != 'e' && ch != '+' && ch != '-') {
            illegal_char = ch;
            return true;
        }
    }
    return false; // 所有字符均合法
}

void sortFace(Face & face){
    // 提取顶点索引
    std::array<unsigned int, 3> vertexIndices = {face.v1, face.v2, face.v3};
    std::array<unsigned int, 3> textureIndices = {face.t1, face.t2, face.t3};
    std::array<unsigned int, 3> normalIndices = {face.n1, face.n2, face.n3};

    // 创建一个包含顶点索引和对应的索引元组的数组
    std::array<std::tuple<unsigned int, unsigned int, unsigned int>, 3> indexedFaces = {
        std::make_tuple(face.v1, face.t1, face.n1),
        std::make_tuple(face.v2, face.t2, face.n2),
        std::make_tuple(face.v3, face.t3, face.n3)
    };

    // 对元组数组进行排序，依据顶点索引
    std::sort(indexedFaces.begin(), indexedFaces.end(),
            [](const std::tuple<unsigned int, unsigned int, unsigned int>& a, const std::tuple<unsigned int, unsigned int, unsigned int>& b) {
                return std::get<0>(a) < std::get<0>(b);
            });

    // 重新提取排序后的结果
    face.v1 = std::get<0>(indexedFaces[0]);
    face.t1 = std::get<1>(indexedFaces[0]);
    face.n1 = std::get<2>(indexedFaces[0]);
    face.v2 = std::get<0>(indexedFaces[1]);
    face.t2 = std::get<1>(indexedFaces[1]);
    face.n2 = std::get<2>(indexedFaces[1]);
    face.v3 = std::get<0>(indexedFaces[2]);
    face.t3 = std::get<1>(indexedFaces[2]);
    face.n3 = std::get<2>(indexedFaces[2]);
}

bool ObjModel::save(const std::string& fileName, const double & model_minz)
{
    string filedir = fileName.substr(0, fileName.find_last_of("\\") + 1);

    #ifdef _WIN32
        std::filesystem::create_directory(filedir);
    #else
        mkdir(fileDirFull.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    #endif
    
    // C方式的输出分割后的obj文件
    FILE* file = _wfopen(string_to_wstring(fileName).c_str(), L"w");
    if (file == nullptr) {
        std::cout << "Error: Unable to open or create file " << fileName << std::endl;
        return false;
    }
    else{
        auto now = std::chrono::system_clock::now();
        string content = "mtllib " + mtllib + '\n';
        fputs(content.c_str(), file);
        for (auto& point : points)
        {
            point.z -= model_minz;    // 模型上移，使得最低点z轴坐标为0
            content = "v " + point.Vector3_to_string() + '\n';
            fputs(content.c_str(), file);
        }
        for (const auto& tpoint : texturePoints)
        {
            content = "vt " + tpoint.Vector2_to_string() + '\n';
            fputs(content.c_str(), file);
        }
        for (const auto& normal : normals)
        {
            content = "vn " + normal.Vector3_to_string() + '\n';
            fputs(content.c_str(), file);
        }
        for(const auto& mtlFace : faces.mtlFaces){
            if (!mtlFace.mtl.empty() && !mtlFace.faces.empty()){
                content = "usemtl " + mtlFace.mtl + '\n';
                fputs(content.c_str(), file);

                for(auto& f : mtlFace.faces) {
                content = "f " + f.Face_to_string() + '\n';
                fputs(content.c_str(), file);
                }
            }
        }
        fclose(file);
        auto end = std::chrono::system_clock::now();
        std::chrono::duration<double> saveElapsedSeconds = end - now;
        std::cout << "Save " << fileName << " in " << saveElapsedSeconds.count() << "s" << std::endl;
        std::cout << "Total " << points.size() << " points, "
                  << texturePoints.size() << " texture points, "
                  << normals.size() << " normals, "
                  << faces.getNumFaces() << " faces." << std::endl;
        return true;
    }


    // C++方式的输出分割后的obj文件，但ofstream无法读取带中文的文件路径
    // _putenv("LC_ALL=C");
    // std::locale loc = std::locale::global(std::locale(""));
    // std::ofstream file(fileName);
    // std::locale::global(loc);
    // if (file.is_open())
    // {
    //     auto now = std::chrono::system_clock::now();
    //     file << "mtllib " << mtllib << std::endl;
    //     for (const auto& point : points)
    //     {
    //         file << "v " << point << std::endl;
    //     }
    //     for (const auto& tpoint : texturePoints)
    //     {
    //         file << "vt " << tpoint << std::endl;
    //     }
    //     for (const auto& normal : normals)
    //     {
    //         file << "vn " << normal << std::endl;
    //     }
    //     file << faces;
    //     file.close();
    //     auto end = std::chrono::system_clock::now();
    //     std::chrono::duration<double> saveElapsedSeconds = end - now;
    //     std::cout << "Save " << fileName << " in " << saveElapsedSeconds.count() << "s" << std::endl;
    //     std::cout << "Total " << points.size() << " points, "
    //               << texturePoints.size() << " texture points, "
    //               << normals.size() << " normals, "
    //               << faces.getNumFaces() << " faces." << std::endl;
    //     return true;
    // }
    // else{
    //     std::cout << "Error: Unable to open or create file " << fileName << std::endl;
    //     return false;
    // }
}

bool ObjModel::empty() const
{
    return points.empty();
}

Vector3 ObjModel::getCenter() const
{
    Vector3 center{};
    center.x = (minX + maxX) / 2;
    center.y = (minY + maxY) / 2;
    center.z = (minZ + maxZ) / 2;
    return center;
}

Vector3 ObjModel::getBottomCenter() const
{
    Vector3 BottomCenter{};
    BottomCenter.x = (minX + maxX) / 2;
    BottomCenter.y = (minY + maxY) / 2;
    BottomCenter.z = minZ;
    return BottomCenter;
}

Vector3 ObjModel::getMinPoint() const
{
    return Vector3(minX, minY, minZ);
}

Vector3 ObjModel::getMaxPoint() const
{
    return Vector3(maxX, maxY, maxZ);
}

string ObjModel::getDir() const
{
    return fileDir;
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

void ObjModel::initSpaces(int numPoints, int numTexturePoints, int numNormals)
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
    fileDir = filename.substr(0, filename.find_last_of("\\") + 1);
    fileName = filename.substr(filename.find_last_of("\\") + 1);
    std::wstring open_file =  string_to_wstring(filename);

    auto now = std::chrono::system_clock::now();
    FILE* file = _wfopen(open_file.c_str(), L"r");
    if (file == nullptr)
    {
        std::cout << "Error: Cannot open file " << filename << std::endl;
        return false;
    }

    cout << "Loading " << filename << "..." << endl;
    int errflag = 0;

    auto processMtllib = [&]() {
        FILE* file1 = _wfopen(open_file.c_str(), L"r");
        char buffer[1024];
        while (fgets(buffer, sizeof(buffer), file1)){
            char type[100] = "";
            sscanf(buffer, "%s", type);
            if (strcmp(type, "mtllib") == 0){
                char mtllibBuff[200];
                sscanf(buffer, "mtllib %[^\n]", mtllibBuff);
                mtllib = string(mtllibBuff);
                mtllib.erase(mtllib.find_last_not_of('\n') + 1);
            }
            else{
                continue;
            }
        }
        fclose(file1);
    };

    auto processPoint = [&]() {
        FILE* file2 = _wfopen(open_file.c_str(), L"r");
        char buffer[1024];
        while (fgets(buffer, sizeof(buffer), file2)){
            char type[100] = "";
            sscanf(buffer, "%s", type);
            if (strcmp(type, "v") == 0){  
                char vtnAllBuff[300];
                sscanf(buffer, "v%[^\n]", vtnAllBuff);
                char *ptr = vtnAllBuff;
                std::string currentInput(ptr);
                char illegal_char;
                // 检查是否包含非法字符
                if (hasIllegalCharacters(currentInput, illegal_char)) {
                    errflag = 1;
                    std::cerr << "Warning: Invalid character encountered in v information of " << filename 
                            << ": " << currentInput << ", invalid character: " << illegal_char << std::endl;
                    std::cout << "Warning: Invalid character encountered in v information of " << filename 
                            << ": " << currentInput << ", invalid character: " << illegal_char << std::endl;
                    continue;
                }
                Vector3 point{};
                sscanf(buffer, "v %f %f %f", &point.x, &point.y, &point.z);
                minX = point.x < minX ? point.x : minX;
                minY = point.y < minY ? point.y : minY;
                minZ = point.z < minZ ? point.z : minZ;
                maxX = point.x > maxX ? point.x : maxX;
                maxY = point.y > maxY ? point.y : maxY;
                maxZ = point.z > maxZ ? point.z : maxZ;
                points.push_back(point);
            }
            else{
                continue;
            }
        }
        fclose(file2);
    };

    auto processTexturePoint = [&]() {
        FILE* file3 = _wfopen(open_file.c_str(), L"r");
        char buffer[1024];
        while (fgets(buffer, sizeof(buffer), file3)){
            char type[100] = "";
            sscanf(buffer, "%s", type);
            if (strcmp(type, "vt") == 0){

                char vtnAllBuff[300];
                sscanf(buffer, "vt%[^\n]", vtnAllBuff);
                char *ptr = vtnAllBuff;
                std::string currentInput(ptr);
                char illegal_char;
                // 检查是否包含非法字符
                if (hasIllegalCharacters(currentInput, illegal_char)) {
                    errflag = 1;
                    std::cerr << "Warning: Invalid character encountered in vt information of " << filename 
                            << ": " << currentInput << ", invalid character: " << illegal_char << std::endl;
                    std::cout << "Warning: Invalid character encountered in vt information of " << filename 
                            << ": " << currentInput << ", invalid character: " << illegal_char << std::endl;
                    continue;
                }

                Vector2 point{};
                sscanf(buffer, "vt %f %f", &point.x, &point.y);
                texturePoints.emplace_back(point.x, point.y);
            }
            else{
                continue;
            }
        }
        fclose(file3);
    };

    auto processNormal = [&]() {
        FILE* file4 = _wfopen(open_file.c_str(), L"r");
        char buffer[1024];
        while (fgets(buffer, sizeof(buffer), file4)){
            char type[100] = "";
            sscanf(buffer, "%s", type);
            if (strcmp(type, "vn") == 0){

                char vtnAllBuff[300];
                sscanf(buffer, "vn%[^\n]", vtnAllBuff);
                char *ptr = vtnAllBuff;
                std::string currentInput(ptr);
                char illegal_char;
                // 检查是否包含非法字符
                if (hasIllegalCharacters(currentInput, illegal_char)) {
                    errflag = 1;
                    std::cerr << "Warning: Invalid character encountered in vn information of " << filename 
                            << ": " << currentInput << ", invalid character: " << illegal_char << std::endl;
                    std::cout << "Warning: Invalid character encountered in vn information of " << filename 
                            << ": " << currentInput << ", invalid character: " << illegal_char << std::endl;
                    continue;
                }

                Vector3 normal{};
                sscanf(buffer, "vn %f %f %f", &normal.x, &normal.y, &normal.z);
                normals.push_back(normal);
            }
            else{
                continue;
            }
        }
        fclose(file4);
    };

    auto processUsemtlandF = [&]() {
        FILE* file5 = _wfopen(open_file.c_str(), L"r");
        char buffer[1024];
        while (fgets(buffer, sizeof(buffer), file5)){
            char type[100] = "";
            sscanf(buffer, "%s", type);

            if (strcmp(type, "f") == 0){
                std::vector<int> v, t, n;

                char vtnAllBuff[300];
                sscanf(buffer, "f%[^\n]", vtnAllBuff);
                int spaceCount = std::count(vtnAllBuff, vtnAllBuff + strlen(vtnAllBuff), ' ');
                string regex = "%d/%d/%d";

                char *ptr = vtnAllBuff;
                for (int i = 0; i < spaceCount; i++)
                {

                    char illegal_char;
                    std::string currentInput(ptr);
                    // 检查是否包含非法字符
                    if (hasIllegalCharacters(currentInput, illegal_char)) {
                        errflag = 1;
                        std::cerr << "Warning: Invalid character encountered in f information of " << filename 
                                << ": " << currentInput << ", invalid character: " << illegal_char << std::endl;
                        std::cout << "Warning: Invalid character encountered in f information of " << filename 
                                << ": " << currentInput << ", invalid character: " << illegal_char << std::endl;
                        break;
                    }

                    ptr += strcspn(ptr, " ") + 1;
                    int a, b, c;

                    int ret= sscanf(ptr, regex.c_str(), &a, &b, &c);
                    if (ret >= 1)
                        v.push_back(a);
                    if (ret >= 2)
                        t.push_back(b);
                    if (ret >= 3)
                        n.push_back(c);
                }
                if(errflag) continue;

                int lenth = v.size();
                for (int i = 0; i <= lenth - 3; i++)
                {
                    Face newFace;
                    newFace.v1 = v[0];
                    newFace.v2 = v[i + 1];
                    newFace.v3 = v[i + 2];
                    if (t.size() > 0)
                    {
                        newFace.t1 = t[0];
                        newFace.t2 = t[i + 1];
                        newFace.t3 = t[i + 2];
                    }
                    if (n.size() > 0)
                    {
                        newFace.n1 = n[0];
                        newFace.n2 = n[i + 1];
                        newFace.n3 = n[i + 2];
                    }
                    faces.push_back(newFace);
                }
            }
            else if (strcmp(type, "usemtl") == 0){
                char mtlBuff[200];
                sscanf(buffer, "usemtl %[^\n]", mtlBuff);
                MtlFaces facesNow;
                facesNow.mtl = string(mtlBuff);
                facesNow.mtl.erase(facesNow.mtl.find_last_not_of('\n') + 1);
                faces.push_back(facesNow);
            }
            else{
                continue;
            }
        }
        fclose(file5);
    };

    auto future1 = std::async(std::launch::async, processUsemtlandF);
    auto future2 = std::async(std::launch::async, processMtllib);
    auto future3 = std::async(std::launch::async, processPoint);
    auto future4 = std::async(std::launch::async, processTexturePoint);
    auto future5 = std::async(std::launch::async, processNormal);

    // 等待所有线程完成
    future1.get();
    future2.get();
    future3.get();
    future4.get();
    future5.get();

    // char buffer[1024];
    // while (fgets(buffer, sizeof(buffer), file))
    // {
    //     char type[100];
    //     sscanf(buffer, "%s", type);
    //     if (strcmp(type, "mtllib") == 0)
    //     {
    //         char mtllibBuff[200];
    //         // fgets(mtllibBuff, sizeof(mtllibBuff), file);
    //         sscanf(buffer, "mtllib %[^\n]", mtllibBuff);
    //         mtllib = string(mtllibBuff);
    //         mtllib.erase(mtllib.find_last_not_of('\n') + 1);
    //     }
    //     else if (strcmp(type, "v") == 0)
    //     {
    //         Vector3 point{};
    //         sscanf(buffer, "v %f %f %f", &point.x, &point.y, &point.z);
    //         minX = point.x < minX ? point.x : minX;
    //         minY = point.y < minY ? point.y : minY;
    //         minZ = point.z < minZ ? point.z : minZ;
    //         maxX = point.x > maxX ? point.x : maxX;
    //         maxY = point.y > maxY ? point.y : maxY;
    //         maxZ = point.z > maxZ ? point.z : maxZ;
    //         points.push_back(point);
    //     }
    //     else if (strcmp(type, "vt") == 0)
    //     {
    //         Vector2 point{};
    //         sscanf(buffer, "vt %f %f", &point.x, &point.y);
    //         texturePoints.emplace_back(point.x, point.y);
    //     }
    //     else if (strcmp(type, "vn") == 0)
    //     {
    //         Vector3 normal{};
    //         sscanf(buffer, "vn %f %f %f", &normal.x, &normal.y, &normal.z);
    //         normals.push_back(normal);
    //     }
    //     else if (strcmp(type, "usemtl") == 0)
    //     {
    //         char mtlBuff[200];
    //         sscanf(buffer, "usemtl %[^\n]", mtlBuff);
    //         MtlFaces facesNow;
    //         facesNow.mtl = string(mtlBuff);
    //         facesNow.mtl.erase(facesNow.mtl.find_last_not_of('\n') + 1);
    //         faces.push_back(facesNow);
    //     }
    //     else if (strcmp(type, "f") == 0)
    //     {
    //         std::vector<int> v, t, n;

    //         char vtnAllBuff[300];
    //         sscanf(buffer, "f%[^\n]", vtnAllBuff);
    //         int spaceCount = std::count(vtnAllBuff, vtnAllBuff + strlen(vtnAllBuff), ' ');
    //         string regex = "%d/%d/%d";

    //         char *ptr = vtnAllBuff;
    //         for (int i = 0; i < spaceCount; i++)
    //         {
    //             ptr += strcspn(ptr, " ") + 1;
    //             int a, b, c;
    //             // 用regex不断读取vtnAllBuff的数字
    //             int ret= sscanf(ptr, regex.c_str(), &a, &b, &c);
    //             if (ret >= 1)
    //                 v.push_back(a);
    //             if (ret >= 2)
    //                 t.push_back(b);
    //             if (ret >= 3)
    //                 n.push_back(c);
    //         }


    //         int lenth = v.size();
    //         for (int i = 0; i <= lenth - 3; i++)
    //         {
    //             Face newFace;
    //             newFace.v1 = v[0];
    //             newFace.v2 = v[i + 1];
    //             newFace.v3 = v[i + 2];
    //             if (t.size() > 0)
    //             {
    //                 newFace.t1 = t[0];
    //                 newFace.t2 = t[i + 1];
    //                 newFace.t3 = t[i + 2];
    //             }
    //             if (n.size() > 0)
    //             {
    //                 newFace.n1 = n[0];
    //                 newFace.n2 = n[i + 1];
    //                 newFace.n3 = n[i + 2];
    //             }
    //             faces.push_back(newFace);
    //         }
    //     }
    //     else
    //     {
    //         if(type[0] == '\n')
    //             continue;
    //         if (type[0] == '#')
    //             continue;
    //         if (strcmp(type, "g") == 0)
    //             continue;
    //         std::cout  << "unknown type: " << type << std::endl;
    //     }
    // }

    // fclose(file);
    auto end = std::chrono::system_clock::now();
    loadElapsedSeconds = end - now;
    return true;
}

void ObjModel::info()
{
    unsigned int faceCount = faces.getNumFaces();
    std::cout << std::endl;
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
    std::cout << std::endl;
}

int ObjCutter::addPoint(const Vector3& point)
{
    auto it = pointMap.find(point);
    if (it == pointMap.end())
    {
        pointMap[point] = pointMap.size() + 1;
        return pointMap.size();
    }
    return it->second;
}

int ObjCutter::addTexturePoint(const Vector2& texturePoint)
{
    auto it = textureMap.find(texturePoint);
    if (it == textureMap.end())
    {
        textureMap[texturePoint] = textureMap.size() + 1;
        return textureMap.size();
    }
    return it->second;
}

int ObjCutter::addNormal(const Vector3& normal)
{
    auto it = normalMap.find(normal);
    if (it == normalMap.end())
    {
        normalMap[normal] = normalMap.size() + 1;
        return normalMap.size();
    }
    return it->second;
}

int ObjCutter::addPoint(const Vector3& point, int op)
{
    if(op == 1)
    {
        auto it = pointMap1.find(point);
        if (it == pointMap1.end())
        {
            pointMap1[point] = pointMap1.size() + 1;
            return pointMap1.size();
        }
        return it->second;
    }
    else
    {
        auto it = pointMap2.find(point);
        if (it == pointMap2.end())
        {
            pointMap2[point] = pointMap2.size() + 1;
            return pointMap2.size();
        }
        return it->second;
    }
    
}

int ObjCutter::addTexturePoint(const Vector2& texturePoint, int op)
{
    if(op == 1)
    {
        auto it = textureMap1.find(texturePoint);
        if (it == textureMap1.end())
        {
            textureMap1[texturePoint] = textureMap1.size() + 1;
            return textureMap1.size();
        }
        return it->second;
    }
    else
    {
        auto it = textureMap2.find(texturePoint);
        if (it == textureMap2.end())
        {
            textureMap2[texturePoint] = textureMap2.size() + 1;
            return textureMap2.size();
        }
        return it->second;
    }
}

int ObjCutter::addNormal(const Vector3& normal, int op)
{
    if(op == 1)
    {
        auto it = normalMap1.find(normal);
        if (it == normalMap1.end())
        {
            normalMap1[normal] = normalMap1.size() + 1;
            return normalMap1.size();
        }
        return it->second;
    }
    else
    {
        auto it = normalMap2.find(normal);
        if (it == normalMap2.end())
        {
            normalMap2[normal] = normalMap2.size() + 1;
            return normalMap2.size();
        }
        return it->second;
    }
}

std::unique_ptr<ObjCutter> ObjCutter::cut(const Area& area)
{
    auto begin = std::chrono::system_clock::now();

    std::unique_ptr<ObjCutter> cuttedModel = std::make_unique<ObjCutter>(fileDir);
    cuttedModel->setMtllib(mtllib);

    // collecting faces
    for (auto& mtlFace : faces.mtlFaces)
    {
        MtlFaces cuttingMtlFace;
        cuttingMtlFace.mtl = mtlFace.mtl;
        cuttedModel->addMtl(mtlFace.mtl);

        for (auto& face : mtlFace.faces)
        {
            Vector3 triangle[3];
            Vector2 texture[3];
            Vector3 normal[3];
            int result = 1;

            auto assignPoint = [&](int index, Vector3& target) -> int{
                if (index < points.size()) {
                    target = points[index];
                    return 1;
                } else {
                    std::cerr << "Warning: Accessing out-of-bounds index in points array! The f is " 
                            << face << ", the size of points is " << points.size() << std::endl;
                    std::cout << "Warning: Accessing out-of-bounds index in points array! The f is " 
                            << face << ", the size of points is " << points.size() << std::endl;
                    return 0;
                }
            };

            result &= assignPoint(face.v1 - 1, triangle[0]);
            result &= assignPoint(face.v2 - 1, triangle[1]);
            result &= assignPoint(face.v3 - 1, triangle[2]);
            if(result == 0) continue;   // 出现越界则跳过该三角片面

            if (face.t1 > 0)
            {
                auto assignTexture = [&](int index, Vector2& target) -> int {
                    if (index < texturePoints.size()) {
                        target = texturePoints[index];
                        return 1;
                    } else {
                        std::cerr << "Warning: Accessing out-of-bounds index in texturePoints array! The f is "
                                << face << ", the size of texturePoints is " << texturePoints.size() << std::endl;
                        std::cout << "Warning: Accessing out-of-bounds index in texturePoints array! The f is "
                                << face << ", the size of texturePoints is " << texturePoints.size() << std::endl;
                        return 0;
                    }
                };

                result &= assignTexture(face.t1 - 1, texture[0]);
                result &= assignTexture(face.t2 - 1, texture[1]);
                result &= assignTexture(face.t3 - 1, texture[2]);
                if(result == 0) continue;   // 出现越界则跳过该三角片面
            }
            if (face.n1 > 0)
            {
                auto assignNormal = [&](int index, Vector3& target) -> int {
                    if (index < normals.size()) {
                        target = normals[index];
                        return 1; // 成功返回1
                    } else {
                        std::cerr << "Warning: Accessing out-of-bounds index in normals array! The f is "
                                << face << ", the size of normals is " << normals.size() << std::endl;
                        std::cout << "Warning: Accessing out-of-bounds index in normals array! The f is "
                                << face << ", the size of normals is " << normals.size() << std::endl;
                        return 0; // 失败返回0
                    }
                };
                result &= assignNormal(face.n1 - 1, normal[0]);
                result &= assignNormal(face.n2 - 1, normal[1]);
                result &= assignNormal(face.n3 - 1, normal[2]);
                if(result == 0) continue;   // 出现越界则跳过该三角片面
            }
            TriangleStatus status(triangle, area);

            // 全部在里面的情况
            if (status.isFull())
            {
                // 如果得到的点组成的是点或线，则跳过
                if(triangle[0] == triangle[1] || triangle[0] == triangle[2] || triangle[1] == triangle[2]){
                    continue;
                }

                Face newFace;
                newFace.v1 = addPoint(triangle[0]);
                newFace.v2 = addPoint(triangle[1]);
                newFace.v3 = addPoint(triangle[2]);
                if (face.t1 > 0)
                {
                    newFace.t1 = addTexturePoint(texture[0]);
                    newFace.t2 = addTexturePoint(texture[1]);
                    newFace.t3 = addTexturePoint(texture[2]);
                }
                if (face.n1 > 0)
                {
                    newFace.n1 = addNormal(normal[0]);
                    newFace.n2 = addNormal(normal[1]);
                    newFace.n3 = addNormal(normal[2]);
                }
                cuttedModel->addFace(newFace);
                continue;
            }

            // 若三点都在外面，则跳过
            if (status.isOut())
            {
                continue;
            }

            // 其它情况, 先把交点切出来
            Vector3 newPoint1, newPoint2;
            Vector2 newTexture1, newTexture2;
            Vector3 newNormal1, newNormal2;
            cutFace(area, triangle, texture, normal, status,
                newPoint1, newPoint2, newTexture1, newTexture2, newNormal1, newNormal2);

            // 然后再看情况加三角形
            int singleIndex = status.getSingleIndex() - 1;

            if(status.getInpartNum() == 1){

                // 如果得到的点组成的是点，则跳过
                if(triangle[singleIndex] == newPoint1 && newPoint1 == newPoint2 && newPoint2 == triangle[singleIndex])
                    continue;

                Face newFace;
                newFace.v1 = addPoint(triangle[singleIndex]);
                newFace.v2 = addPoint(newPoint1);
                newFace.v3 = addPoint(newPoint2);
                if (face.t1 > 0)
                {
                    newFace.t1 = addTexturePoint(texture[singleIndex]);
                    newFace.t2 = addTexturePoint(newTexture1);
                    newFace.t3 = addTexturePoint(newTexture2);
                }
                if (face.n1 > 0)
                {
                    newFace.n1 = addNormal(normal[singleIndex]);
                    newFace.n2 = addNormal(newNormal1);
                    newFace.n3 = addNormal(newNormal2);
                }
                cuttedModel->addFace(newFace);
            }
            else if (status.getInpartNum() == 2){

                Vector3 p1{triangle[(singleIndex + 1) % 3]},
                        p2{triangle[(singleIndex + 2) % 3]};
                Vector2 t1{ texture[(singleIndex + 1) % 3]},
                        t2{ texture[(singleIndex + 2) % 3]};
                Vector3 n1{ normal[(singleIndex + 1) % 3]},
                        n2{ normal[(singleIndex + 2) % 3]};

                if(p1 == newPoint1 && p2 == newPoint2){
                    continue;
                }
                else if(p1 == newPoint1 && p2 != newPoint2){
                    Face newFace;
                    newFace.v1 = addPoint(p1);
                    newFace.v2 = addPoint(p2);
                    newFace.v3 = addPoint(newPoint2);

                    if (face.t1 > 0)
                    {
                        newFace.t1 = addTexturePoint(t1);
                        newFace.t2 = addTexturePoint(t2);
                        newFace.t3 = addTexturePoint(newTexture2);
                    }

                    if (face.n1 > 0)
                    {
                        newFace.n1 = addNormal(n1);
                        newFace.n2 = addNormal(n2);
                        newFace.n3 = addNormal(newNormal2);
                    }

                    cuttedModel->addFace(newFace);
                }
                else if(p1 != newPoint1 && p2 == newPoint2){
                    Face newFace;
                    newFace.v1 = addPoint(p1);
                    newFace.v2 = addPoint(p2);
                    newFace.v3 = addPoint(newPoint1);

                    if (face.t1 > 0)
                    {
                        newFace.t1 = addTexturePoint(t1);
                        newFace.t2 = addTexturePoint(t2);
                        newFace.t3 = addTexturePoint(newTexture1);
                    }

                    if (face.n1 > 0)
                    {
                        newFace.n1 = addNormal(n1);
                        newFace.n2 = addNormal(n2);
                        newFace.n3 = addNormal(newNormal1);
                    }

                    cuttedModel->addFace(newFace);
                }
                else{
                    Face newFace1, newFace2;

                    newFace1.v1 = addPoint(p1);
                    newFace1.v2 = addPoint(p2);
                    newFace1.v3 = addPoint(newPoint2);
                    newFace2.v1 = addPoint(newPoint2);
                    newFace2.v2 = addPoint(newPoint1);
                    newFace2.v3 = addPoint(p1);

                    if (face.t1 > 0)
                    {
                        newFace1.t1 = addTexturePoint(t1);
                        newFace1.t2 = addTexturePoint(t2);
                        newFace1.t3 = addTexturePoint(newTexture2);
                        newFace2.t1 = addTexturePoint(newTexture2);
                        newFace2.t2 = addTexturePoint(newTexture1);
                        newFace2.t3 = addTexturePoint(t1);
                    }

                    if (face.n1 > 0)
                    {
                        newFace1.n1 = addNormal(n1);
                        newFace1.n2 = addNormal(n2);
                        newFace1.n3 = addNormal(newNormal2);
                        newFace2.n1 = addNormal(newNormal2);
                        newFace2.n2 = addNormal(newNormal1);
                        newFace2.n3 = addNormal(n1);
                    }

                    cuttedModel->addFace(newFace1);
                    cuttedModel->addFace(newFace2);
                }
            }
        }
    }

    // map映射的索引即为点的索引
    cuttedModel->initSpaces(pointMap.size(), textureMap.size(), normalMap.size());
    for (const auto& kv : pointMap)
    {
        auto point = kv.first;
        auto index = kv.second;
        cuttedModel->setPoint(index, point);
        cuttedModel->minX = point.x < cuttedModel->minX ? point.x : cuttedModel->minX;
        cuttedModel->minY = point.y < cuttedModel->minY ? point.y : cuttedModel->minY;
        cuttedModel->minZ = point.z < cuttedModel->minZ ? point.z : cuttedModel->minZ;
        cuttedModel->maxX = point.x > cuttedModel->maxX ? point.x : cuttedModel->maxX;
        cuttedModel->maxY = point.y > cuttedModel->maxY ? point.y : cuttedModel->maxY;
        cuttedModel->maxZ = point.z > cuttedModel->maxZ ? point.z : cuttedModel->maxZ;
    }
    pointMap.clear();

    for (const auto& kv : textureMap)
    {
        auto texturePoint = kv.first;
        auto index = kv.second;
        cuttedModel->setTexturePoint(index, texturePoint);
    }
    textureMap.clear();

    for (const auto& kv : normalMap)
    {
        auto normal = kv.first;
        auto index = kv.second;
        cuttedModel->setNormal(index, normal);
    }
    normalMap.clear();

    std::chrono::duration<double> cutElapsedSeconds = std::chrono::system_clock::now() - begin;
    // std::cout << "Cut time: " << cutElapsedSeconds.count() << "s" << std::endl;
    // std::cout << "min point: " << minX << " " << minY << " " << minZ << std::endl;

    return cuttedModel;
}

// void ObjCutter::cut(const Area& area, ObjCutter* & cuttedModel1, ObjCutter* & cuttedModel2){
//     auto begin = std::chrono::system_clock::now();

//     cuttedModel1 = new ObjCutter(fileDir);
//     cuttedModel1->setMtllib(mtllib);
//     cuttedModel2 = new ObjCutter(fileDir);
//     cuttedModel2->setMtllib(mtllib);

//     for (const auto& mtlFace : faces.mtlFaces)
//     {
//         MtlFaces cuttingMtlFace;
//         cuttingMtlFace.mtl = mtlFace.mtl;
//         cuttedModel1->addMtl(mtlFace.mtl);
//         cuttedModel2->addMtl(mtlFace.mtl);

//         for (const auto& face : mtlFace.faces)
//         {
//             Vector3 triangle[3];
//             Vector2 texture[3];
//             Vector3 normal[3];
//             triangle[0] = points[face.v1 - 1];
//             triangle[1] = points[face.v2 - 1];
//             triangle[2] = points[face.v3 - 1];
//             if (face.t1 > 0)
//             {
//                 texture[0] = texturePoints[face.t1 - 1];
//                 texture[1] = texturePoints[face.t2 - 1];
//                 texture[2] = texturePoints[face.t3 - 1];
//             }
//             if (face.n1 > 0)
//             {
//                 normal[0] = normals[face.n1 - 1];
//                 normal[1] = normals[face.n2 - 1];
//                 normal[2] = normals[face.n3 - 1];
//             }
//             TriangleStatus status(triangle, area);

//             // 全部在里面的情况
//             if (status.isFull())
//             {
//                 Face newFace;
//                 newFace.v1 = addPoint(triangle[0], 1);
//                 newFace.v2 = addPoint(triangle[1], 1);
//                 newFace.v3 = addPoint(triangle[2], 1);
//                 if (face.t1 > 0)
//                 {
//                     newFace.t1 = addTexturePoint(texture[0], 1);
//                     newFace.t2 = addTexturePoint(texture[1], 1);
//                     newFace.t3 = addTexturePoint(texture[2], 1);
//                 }
//                 if (face.n1 > 0)
//                 {
//                     newFace.n1 = addNormal(normal[0], 1);
//                     newFace.n2 = addNormal(normal[1], 1);
//                     newFace.n3 = addNormal(normal[2], 1);
//                 }
//                 cuttedModel1->addFace(newFace);
//                 continue;
//             }

//             // 三点都在外面
//             if (status.isOut())
//             {
//                 Face newFace;
//                 newFace.v1 = addPoint(triangle[0], 2);
//                 newFace.v2 = addPoint(triangle[1], 2);
//                 newFace.v3 = addPoint(triangle[2], 2);
//                 if (face.t1 > 0)
//                 {
//                     newFace.t1 = addTexturePoint(texture[0], 2);
//                     newFace.t2 = addTexturePoint(texture[1], 2);
//                     newFace.t3 = addTexturePoint(texture[2], 2);
//                 }
//                 if (face.n1 > 0)
//                 {
//                     newFace.n1 = addNormal(normal[0], 2);
//                     newFace.n2 = addNormal(normal[1], 2);
//                     newFace.n3 = addNormal(normal[2], 2);
//                 }
//                 cuttedModel2->addFace(newFace);
//                 continue;
//             }

//             // 其它情况, 先把交点切出来
//             Vector3 newPoint1, newPoint2;
//             Vector2 newTexture1, newTexture2;
//             Vector3 newNormal1, newNormal2;
//             cutFace(area, triangle, texture, normal, status,
//                 newPoint1, newPoint2, newTexture1, newTexture2, newNormal1, newNormal2);

//             // 然后再看情况加三角形
//             int singleIndex = status.getSingleIndex() - 1;
//             if (status.getInpartNum() == 1)
//             {
//                 Face newFace1;
//                 newFace1.v1 = addPoint(triangle[singleIndex], 1);
//                 newFace1.v2 = addPoint(newPoint1, 1);
//                 newFace1.v3 = addPoint(newPoint2, 1);
//                 if (face.t1 > 0)
//                 {
//                     newFace1.t1 = addTexturePoint(texture[singleIndex], 1);
//                     newFace1.t2 = addTexturePoint(newTexture1, 1);
//                     newFace1.t3 = addTexturePoint(newTexture2, 1);
//                 }
//                 if (face.n1 > 0)
//                 {
//                     newFace1.n1 = addNormal(normal[0], 1);
//                     newFace1.n2 = addNormal(normal[1], 1);
//                     newFace1.n3 = addNormal(normal[2], 1);
//                 }
//                 cuttedModel1->addFace(newFace1);

//                 Face newFace2, newFace3;
//                 Vector3 p1{triangle[(singleIndex + 1) % 3]},
//                         p2{triangle[(singleIndex + 2) % 3]};
//                 Vector2 t1{ texture[(singleIndex + 1) % 3]},
//                         t2{ texture[(singleIndex + 2) % 3]};
//                 Vector3 n1{ normal[(singleIndex + 1) % 3]},
//                         n2{ normal[(singleIndex + 2) % 3]};

//                 newFace2.v1 = addPoint(p1, 2);
//                 newFace2.v2 = addPoint(p2, 2);
//                 newFace2.v3 = addPoint(newPoint2, 2);
//                 newFace3.v1 = addPoint(newPoint2, 2);
//                 newFace3.v2 = addPoint(newPoint1, 2);
//                 newFace3.v3 = addPoint(p1, 2);

//                 if (face.t1 > 0)
//                 {
//                     newFace2.t1 = addTexturePoint(t1, 2);
//                     newFace2.t2 = addTexturePoint(t2, 2);
//                     newFace2.t3 = addTexturePoint(newTexture2, 2);
//                     newFace3.t1 = addTexturePoint(newTexture2, 2);
//                     newFace3.t2 = addTexturePoint(newTexture1, 2);
//                     newFace3.t3 = addTexturePoint(t1, 2);
//                 }

//                 if (face.n1 > 0)
//                 {
//                     newFace1.n1 = addNormal(n1, 2);
//                     newFace1.n2 = addNormal(n2, 2);
//                     newFace1.n3 = addNormal(newNormal2, 2);
//                     newFace2.n1 = addNormal(newNormal2, 2);
//                     newFace2.n2 = addNormal(newNormal1, 2);
//                     newFace2.n3 = addNormal(n1, 2);
//                 }

//                 cuttedModel2->addFace(newFace2);
//                 cuttedModel2->addFace(newFace3);
//             }
//             else if (status.getInpartNum() == 2)
//             {
//                 Face newFace1, newFace2;
//                 Vector3 p1{triangle[(singleIndex + 1) % 3]},
//                         p2{triangle[(singleIndex + 2) % 3]};
//                 Vector2 t1{ texture[(singleIndex + 1) % 3]},
//                         t2{ texture[(singleIndex + 2) % 3]};
//                 Vector3 n1{  normal[(singleIndex + 1) % 3]},
//                         n2{  normal[(singleIndex + 2) % 3]};

//                 newFace1.v1 = addPoint(p1, 1);
//                 newFace1.v2 = addPoint(p2, 1);
//                 newFace1.v3 = addPoint(newPoint2, 1);
//                 newFace2.v1 = addPoint(newPoint2, 1);
//                 newFace2.v2 = addPoint(newPoint1, 1);
//                 newFace2.v3 = addPoint(p1, 1);

//                 if (face.t1 > 0)
//                 {
//                     newFace1.t1 = addTexturePoint(t1, 1);
//                     newFace1.t2 = addTexturePoint(t2, 1);
//                     newFace1.t3 = addTexturePoint(newTexture2, 1);
//                     newFace2.t1 = addTexturePoint(newTexture2, 1);
//                     newFace2.t2 = addTexturePoint(newTexture1, 1);
//                     newFace2.t3 = addTexturePoint(t1, 1);
//                 }

//                 if (face.n1 > 0)
//                 {
//                     newFace1.n1 = addNormal(n1, 1);
//                     newFace1.n2 = addNormal(n2, 1);
//                     newFace1.n3 = addNormal(newNormal2, 1);
//                     newFace2.n1 = addNormal(newNormal2, 1);
//                     newFace2.n2 = addNormal(newNormal1, 1);
//                     newFace2.n3 = addNormal(n1, 2);
//                 }

//                 cuttedModel1->addFace(newFace1);
//                 cuttedModel1->addFace(newFace2);

//                 Face newFace3;
//                 newFace3.v1 = addPoint(triangle[singleIndex], 2);
//                 newFace3.v2 = addPoint(newPoint1, 2);
//                 newFace3.v3 = addPoint(newPoint2, 2);
//                 if (face.t1 > 0)
//                 {
//                     newFace3.t1 = addTexturePoint(texture[singleIndex], 2);
//                     newFace3.t2 = addTexturePoint(newTexture1, 2);
//                     newFace3.t3 = addTexturePoint(newTexture2, 2);
//                 }
//                 if (face.n1 > 0)
//                 {
//                     newFace3.n1 = addNormal(normal[0], 2);
//                     newFace3.n2 = addNormal(normal[1], 2);
//                     newFace3.n3 = addNormal(normal[2], 2);
//                 }
//                 cuttedModel2->addFace(newFace3);
//             }
//         }
//     }
    
//     // map映射的索引即为点的索引
//     cuttedModel1->initSpaces(pointMap1.size(), textureMap1.size(), normalMap1.size());
//     cuttedModel2->initSpaces(pointMap2.size(), textureMap2.size(), normalMap2.size());

//     for (const auto& kv : pointMap1)
//     {
//         auto point = kv.first;
//         auto index = kv.second;
//         cuttedModel1->setPoint(index, point);
//         cuttedModel1->minX = point.x < cuttedModel1->minX ? point.x : cuttedModel1->minX;
//         cuttedModel1->minY = point.y < cuttedModel1->minY ? point.y : cuttedModel1->minY;
//         cuttedModel1->minZ = point.z < cuttedModel1->minZ ? point.z : cuttedModel1->minZ;
//         cuttedModel1->maxX = point.x > cuttedModel1->maxX ? point.x : cuttedModel1->maxX;
//         cuttedModel1->maxY = point.y > cuttedModel1->maxY ? point.y : cuttedModel1->maxY;
//         cuttedModel1->maxZ = point.z > cuttedModel1->maxZ ? point.z : cuttedModel1->maxZ;
//     }
//     pointMap1.clear();

//     for (const auto& kv : textureMap1)
//     {
//         auto texturePoint = kv.first;
//         auto index = kv.second;
//         cuttedModel1->setTexturePoint(index, texturePoint);
//     }
//     textureMap1.clear();

//     for (const auto& kv : normalMap1)
//     {
//         auto normal = kv.first;
//         auto index = kv.second;
//         cuttedModel1->setNormal(index, normal);
//     }
//     normalMap1.clear();

//     for (const auto& kv : pointMap2)
//     {
//         auto point = kv.first;
//         auto index = kv.second;
//         cuttedModel2->setPoint(index, point);
//         cuttedModel2->minX = point.x < cuttedModel2->minX ? point.x : cuttedModel2->minX;
//         cuttedModel2->minY = point.y < cuttedModel2->minY ? point.y : cuttedModel2->minY;
//         cuttedModel2->minZ = point.z < cuttedModel2->minZ ? point.z : cuttedModel2->minZ;
//         cuttedModel2->maxX = point.x > cuttedModel2->maxX ? point.x : cuttedModel2->maxX;
//         cuttedModel2->maxY = point.y > cuttedModel2->maxY ? point.y : cuttedModel2->maxY;
//         cuttedModel2->maxZ = point.z > cuttedModel2->maxZ ? point.z : cuttedModel2->maxZ;
//     }
//     pointMap2.clear();

//     for (const auto& kv : textureMap2)
//     {
//         auto texturePoint = kv.first;
//         auto index = kv.second;
//         cuttedModel2->setTexturePoint(index, texturePoint);
//     }
//     textureMap2.clear();

//     for (const auto& kv : normalMap2)
//     {
//         auto normal = kv.first;
//         auto index = kv.second;
//         cuttedModel2->setNormal(index, normal);
//     }
//     normalMap2.clear();

//     std::chrono::duration<double> cutElapsedSeconds = std::chrono::system_clock::now() - begin;
//     // std::cout << "Cut time: " << cutElapsedSeconds.count() << "s" << std::endl;
//     // std::cout << "min point: " << minX << " " << minY << " " << minZ << std::endl;
// }

void ObjCutter::cutFace(const Area& area, const Vector3* triangle, const Vector2* triangleTexture, const Vector3* normal, const TriangleStatus& status,
                        Vector3& newPoint1, Vector3& newPoint2, Vector2& newTexturePoint1, Vector2& newTexturePoint2, 
                        Vector3& newNormal1, Vector3& newNormal2)
{
    int singleIndex = status.getSingleIndex() - 1;

    Vector3 pSingle, p1, p2;
    pSingle = triangle[singleIndex];
    p1 = triangle[(singleIndex + 1) % 3];
    p2 = triangle[(singleIndex + 2) % 3];

    Vector2 tSingle, t1, t2;
    if (triangleTexture)
    {
        tSingle = triangleTexture[singleIndex];
        t1 = triangleTexture[(singleIndex + 1) % 3];
        t2 = triangleTexture[(singleIndex + 2) % 3];
    }

    Vector3 nSingle, n1, n2;
    if(normal){
        nSingle = normal[singleIndex];
        n1 = normal[(singleIndex + 1) % 3];
        n2 = normal[(singleIndex + 2) % 3];
    }

    // 计算单一点与其他两点的连线与平面的两个交点
    newPoint1 = area.getIntersectPoint(pSingle, p1);
    newPoint2 = area.getIntersectPoint(pSingle, p2);

    if (!triangleTexture)
        return;

    // 用相似三角形计算新的纹理坐标
    double rate1 = (pSingle - p1).length() /
        (newPoint1 - p1).length();
    double rate2 = (pSingle - p2).length() /
        (newPoint2 - p2).length();
    newTexturePoint1 = t1 + (tSingle - t1) / rate1;
    newTexturePoint2 = t2 + (tSingle - t2) / rate2;

    if(!normal)
        return;
    
    // 计算法向量插值
    newNormal1 = n1 + (nSingle - n1) / rate1;
    newNormal2 = n2 + (nSingle - n2) / rate2;
}