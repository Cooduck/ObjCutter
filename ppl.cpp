// ppl: 对单个obj模型进行vt的normalize和将坐标系移到模型底部最中央
// 用法: ppl <inputfile> [-nml] [-ccs] [-cvn] <outputdir>
// 例子: ppl ./tx.obj -nml ./output     --对./tx.obj仅进行vt的normalize
//       ppl ./tx.obj -ccs ./output     --对./tx.obj仅进行将坐标系移到模型底部最中央
//       ppl ./tx.obj -cvn ./output     --若./tx.obj文件中无vn数据，则计算其每个点的vn数据，然后添加vn数据到文件中，并将"f X/X X/X X/X"修改为"f X/X/X X/X/X X/X/X"
//       ppl ./tx.obj -nml -ccs ./output 或 ppl ./tx.obj ./output  --对./tx.obj进行vt的normalize和将坐标系移到模型底部最中央
//       ppl ./tx.obj -nml -ccs -cvn ./output   --对./tx.obj进行vt的normalize、将坐标系移到模型底部最中央并计算其vn数据

#include "Types.h"

#include <cmath>
#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <filesystem>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <future>
#include <semaphore>
#include <queue>
#include <utility>
#include <io.h>
#include <codecvt>
#include <locale>
#include <algorithm>

// 将 std::string 转换为 std::wstring
std::wstring string_to_wstring(const std::string& str) {
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    return converter.from_bytes(str);
}

std::string wstring_to_string(const std::wstring& wstr) {
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    return converter.to_bytes(wstr);
}

void calculate_vn(const std::vector<Vector3>& points, std::vector<Vector3>& normals, ObjFaces& faces) {
    // 初始化所有法向量为零
    normals.resize(points.size(), Vector3{0.0f, 0.0f, 0.0f});
    std::vector<float> areas(points.size(), 0.0);

    // 计算每个面法向量并累加到相应的顶点法向量中
    for(auto& mtlFace : faces.mtlFaces){
        if (!mtlFace.faces.empty()){
            for(auto& f : mtlFace.faces) {
                // 获取面上的三个顶点
                const Vector3& v1 = points[f.v1 - 1];  // 顶点索引通常从1开始
                const Vector3& v2 = points[f.v2 - 1];
                const Vector3& v3 = points[f.v3 - 1];

                // 计算边向量
                Vector3 edge1 = v2 - v1;
                Vector3 edge2 = v3 - v1;

                // 计算三角形的法向量（叉积）
                Vector3 normal = edge1.cross(edge2);

                // 计算三角形面积
                float area = normal.length() / 2.0f;

                // 使用面积对法向量加权
                normal = normal * area;

                // 记录面积
                areas[f.v1 - 1] = areas[f.v1 - 1] + area;
                areas[f.v2 - 1] = areas[f.v2 - 1] + area;
                areas[f.v3 - 1] = areas[f.v3 - 1] + area;

                // 累加到面上的顶点法向量
                normals[f.v1 - 1] = normals[f.v1 - 1] + normal;
                normals[f.v2 - 1] = normals[f.v2 - 1] + normal;
                normals[f.v3 - 1] = normals[f.v3 - 1] + normal;

                // 修改face的vn
                f.n1 = f.v1;
                f.n2 = f.v2;
                f.n3 = f.v3;
            }
        }
    }

    // 对每个顶点法向量进行归一化
    for (size_t i = 0; i < normals.size(); ++i) {
        if(normals[i] != Vector3{0.0f, 0.0f, 0.0f}){
            normals[i] = normals[i] / areas[i];
            normals[i] = normals[i].normalize();  // 归一化法向量
        }
    }
}


bool ppl(const std::string& inputfile, const std::string& outputfile, const bool &normalize, const bool &Convert_coordinate_system, bool &cal_vn)
{
    FILE* Inputfile = _wfopen(string_to_wstring(inputfile).c_str(), L"r+");
    if (Inputfile == nullptr)
    {
        std::cout << "Error: Cannot open inputfile " << inputfile << std::endl;
        std::cerr << "Error: Cannot open inputfile " << inputfile << std::endl;
        return false;
    }

    // vt中u、v的最大值和最小值
    float minU = 1e18;
    float maxU = -1e18;
    float minV = 1e18;
    float maxV = -1e18;

    // v中x、y、z的最大值和最小值
    float minX = 1e18;
    float maxX = -1e18;
    float minY = 1e18;
    float maxY = -1e18;
    float minZ = 1e18;
    float maxZ = -1e18;

    // 存储v和f
    std::vector<Vector3> points;
    std::vector<Vector3> normals;
    ObjFaces faces;

    char buffer[1024];
    while (fgets(buffer, sizeof(buffer), Inputfile))
    {
        char type[100] = "";
        sscanf(buffer, "%s", type);
        if (strcmp(type, "vt") == 0)
        {
            float u, v;
            sscanf(buffer, "vt %f %f", &u, &v);
            minU = std::min(minU, u);
            maxU = std::max(maxU, u);
            minV = std::min(minV, v);
            maxV = std::max(maxV, v);
        }
        else if (strcmp(type, "v") == 0)
        {
            float x, y, z;
            sscanf(buffer, "v %f %f %f", &x, &y, &z);
            points.push_back(Vector3{x, y, z});
            minX = std::min(minX, x);
            maxX = std::max(maxX, x);
            minY = std::min(minY, y);
            maxY = std::max(maxY, y);
            minZ = std::min(minZ, z);
            maxZ = std::max(maxZ, z);
        }
        else if (strcmp(type, "f") == 0){
            std::vector<int> v, t;

            char vtnAllBuff[300];
            sscanf(buffer, "f%[^\n]", vtnAllBuff);
            int spaceCount = std::count(vtnAllBuff, vtnAllBuff + strlen(vtnAllBuff), ' ');
            std::string regex = "%d/%d";

            char *ptr = vtnAllBuff;
            for (int i = 0; i < spaceCount; i++)
            {
                char illegal_char;
                std::string currentInput(ptr);
                ptr += strcspn(ptr, " ") + 1;
                int a, b;
                int ret= sscanf(ptr, regex.c_str(), &a, &b);
                if (ret >= 1)
                    v.push_back(a);
                if (ret >= 2)
                    t.push_back(b);
            }

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
                faces.push_back(newFace);
            }
        }
        else if (strcmp(type, "usemtl") == 0){
            char mtlBuff[200];
            sscanf(buffer, "usemtl %[^\n]", mtlBuff);
            MtlFaces facesNow;
            facesNow.mtl = std::string(mtlBuff);
            facesNow.mtl.erase(facesNow.mtl.find_last_not_of('\n') + 1);
            faces.push_back(facesNow);
        }
        else if (strcmp(type, "vn") == 0){
            cal_vn = false; // 避免重复计算vn
        }
    }

    std::cout << "Before preprocess: " << std::endl;
    std::cout << "Min X Y Z: "
        << minX << " " << minY << " " << minZ
        << std::endl;

    std::cout << "Max X Y Z: "
        << maxX << " " << maxY << " " << maxZ
        << std::endl;

    // 重置文件指针到文件头
    rewind(Inputfile);

    // 存储文件内容
    std::vector<std::string> outputLines;

    // 模型中央最低点
    float BottomCenter[3] = {(minX + maxX) / 2, (minY + maxY) / 2, minZ};
    
    // 预处理后模型v的最小值和最大值
    float newminX = 1e18;
    float newmaxX = -1e18;
    float newminY = 1e18;
    float newmaxY = -1e18;
    float newminZ = 1e18;
    float newmaxZ = -1e18;

    // 计算vn
    if(cal_vn)
        calculate_vn(points, normals, faces);

    while (fgets(buffer, sizeof(buffer), Inputfile))
    {
        char type[100] = "";
        sscanf(buffer, "%s", type);
        if (strcmp(type, "mtllib") == 0){
            char mtllibBuff[200];
            sscanf(buffer, "mtllib %[^\n]", mtllibBuff);
            std::string mtllib = std::string(mtllibBuff);
            mtllib.erase(mtllib.find_last_not_of('\n') + 1);
            std::string content = "mtllib " + mtllib + '\n';
            outputLines.push_back(content);
        }
        else if (strcmp(type, "vt") == 0)
        {
            float u, v;
            sscanf(buffer, "vt %f %f", &u, &v);
            if(normalize){
                u = (u - minU) / (maxU - minU);
                v = (v - minV) / (maxV - minV);
            }
            std::string content = "vt " + std::to_string(u) + " " + std::to_string(v) + '\n';
            outputLines.push_back(content);
        }
        else if (strcmp(type, "v") == 0)
        {
            float x, y, z;
            sscanf(buffer, "v %f %f %f", &x, &y, &z);
            if(Convert_coordinate_system){
                x -= BottomCenter[0], y -= BottomCenter[1], z -= BottomCenter[2];
            }
            newminX = std::min(newminX, x);
            newmaxX = std::max(newmaxX, x);
            newminY = std::min(newminY, y);
            newmaxY = std::max(newmaxY, y);
            newminZ = std::min(newminZ, z);
            newmaxZ = std::max(newmaxZ, z);
            std::string content = "v " + std::to_string(x) + " " + std::to_string(y) + " " + std::to_string(z) + '\n';
            outputLines.push_back(content);
        }
        else if(!cal_vn)
        {
            outputLines.push_back(buffer);
        }
    }

    if(cal_vn){
        std::string content;
        for(const auto& normal : normals){
            content = "vn " + normal.Vector3_to_string() + '\n';
            outputLines.push_back(content.c_str());
        }
        for(const auto& mtlFace : faces.mtlFaces){
            if (!mtlFace.faces.empty()){
                content = "usemtl " + mtlFace.mtl + '\n';
                 outputLines.push_back(content.c_str());

                for(auto& f : mtlFace.faces) {
                    content = "f " + f.Face_to_string() + '\n';
                     outputLines.push_back(content.c_str());
                }
            }
        }
    }

    std::cout << "After preprocess: " << std::endl;
    std::cout << "Min X Y Z: "
        << newminX << " " << newminY << " " << newminZ
        << std::endl;

    std::cout << "Max X Y Z: "
        << newmaxX << " " << newmaxY << " " << newmaxZ
        << std::endl;
    
    // 写入文件
    FILE* Outputfile = _wfopen(string_to_wstring(outputfile).c_str(), L"w+");
    if (Outputfile == nullptr)
    {
        std::cout << "Error: Cannot open outputfile " << outputfile << std::endl;
        std::cerr << "Error: Cannot open outputfile " << outputfile << std::endl;
        return false;
    }

    freopen(outputfile.c_str(), "w", Outputfile);
    for (const auto& line : outputLines)
    {
        fputs(line.c_str(), Outputfile);
    }

    fclose(Outputfile);
    return true;
}

std::string ensureTrailingBackslash(const std::string& folderPath) {
    if (!folderPath.empty() && folderPath.back() != '\\') {
        return folderPath + "\\";
    }
    return folderPath;
}

int check_path(const std::string& filepath) {
    std::filesystem::path path = filepath;
    if (std::filesystem::exists(path)) {
        if (std::filesystem::is_regular_file(path)) {
            if (path.extension() == ".obj") {
                return 1;
            }
            else {
                std::cerr << "Error: inputfile is not an obj file." << std::endl;
                return 0;
            }
        }
        else {
            std::cerr << "Error: inputfile is not a regular file." << std::endl;
            return 0;
        }
    } else {
        std::cerr << "Error: inputfile " << path << " does not exist." << std::endl;
        return 0;
    }
}

int main(int argc, char* argv[])
{
    SetConsoleOutputCP(CP_UTF8);
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <inputfile> [-nml] [-ccs] [-cvn] <outputdir>\n";
        return 1;
    }
    
    // 成功标志、nml标志、ccs标志、cvn标志
    bool success = true, normalize = false, Convert_coordinate_system = false, cal_vn = false;
    
    // 输入文件
    std::string inputfile = argv[1];
    // std::string inputfile = "BlockYABX.obj";
    success = check_path(inputfile);
    if(!success){
        return 1;
    }

    // 解析参数
    int outputIndex = argc - 1; // 最后一个参数是输出目录
    for (int i = 2; i < argc - 1; ++i) {
        if (std::string(argv[i]) == "-nml") {
            normalize = true;
        } else if (std::string(argv[i]) == "-ccs") {
            Convert_coordinate_system = true;
        }
        else if(std::string(argv[i]) == "-cvn") {
            cal_vn = true;
        }
        else {
            std::cerr << "Error: Unknown parameter " << argv[i] << "\n";
            return 1;
        }
    }
    // 如果-nml、-ccs和-cvn都没给，则默认开启nml、ccs和cvn
    if(argc == 3){
        normalize = true, Convert_coordinate_system = true, cal_vn = false;
    }

    // 输出文件
    std::string outputdir = argv[outputIndex];
    // std::string outputdir = "./test/";
    outputdir = ensureTrailingBackslash(outputdir);
    std::filesystem::create_directory(outputdir);
    std::string filename = inputfile.substr(inputfile.find_last_of("\\") + 1);
    freopen(std::string(outputdir + "preprocess_log.txt").c_str(), "w", stdout);

    std::cout << "preprocess: " << inputfile << std::endl;
    std::cerr << "preprocess: " << inputfile << std::endl;
    if (normalize) {
        std::cout << "Normalization is enabled.\n";
        std::cerr << "Normalization is enabled.\n";
    }
    if (Convert_coordinate_system) {
        std::cout << "Coordinate system conversion is enabled.\n";
        std::cerr << "Coordinate system conversion is enabled.\n";
    }
    if (cal_vn){
        std::cout << "Calculate vn is enabled.\n";
        std::cerr << "Calculate vn is enabled.\n";
    }

    success = ppl(inputfile, outputdir + filename, normalize, Convert_coordinate_system, cal_vn);

    if(success){
        std::cout << "Successfully preprocess " << inputfile << " to " << outputdir + filename << "!" << std::endl;
        std::cerr << "Successfully preprocess " << inputfile << " to " << outputdir + filename << "!" << std::endl;
    }
    else{
        std::cout << "Failed to preprocess " << inputfile << " to " << outputdir + filename << "!" << std::endl;
        std::cerr << "Failed to preprocess " << inputfile << " to " << outputdir + filename << "!" << std::endl;
    }
    return 0;
}