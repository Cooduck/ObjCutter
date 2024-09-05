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
#include <float.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/stat.h>
#endif

// 将 std::string 转换为 std::wstring
std::wstring string_to_wstring(const std::string& str) {
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    return converter.from_bytes(str);
}

bool normalize_vt(const std::string& inputfile, const std::string& outputfile)
{
    FILE* Inputfile = _wfopen(string_to_wstring(inputfile).c_str(), L"r");
    FILE* Outputfile;
    if (Inputfile == nullptr)
    {
        std::cerr << "Error: Cannot open file " << inputfile << std::endl;
        return 0;
    }

    float minU;
    float maxU;
    float minV;
    float maxV;
    int flag = 1;

    char buffer[1024];
    while (fgets(buffer, sizeof(buffer), Inputfile))
    {
        char type[100];
        sscanf(buffer, "%s", type);
        if (strcmp(type, "vt") == 0)
        {
            float u, v;
            sscanf(buffer, "vt %f %f", &u, &v);
            if (flag) {
                minU = u;
                maxU = u;
                minV = v;
                maxV = v;
                flag = 0;
                continue;
            }
            minU = std::min(minU, u);
            maxU = std::max(maxU, u);
            minV = std::min(minV, v);
            maxV = std::max(maxV, v);
        }
        else
        {
            continue;
        }
    }
    fclose(Inputfile);

    Inputfile = _wfopen(string_to_wstring(inputfile).c_str(), L"r");
    Outputfile = _wfopen(string_to_wstring(outputfile).c_str(), L"w");
    if (Outputfile == nullptr) {
        std::cerr << "Error: Unable to open or create file " << Outputfile << std::endl;
        return 0;
    }

    while (fgets(buffer, sizeof(buffer), Inputfile))
    {
        char type[100];
        sscanf(buffer, "%s", type);
        if (strcmp(type, "vt") == 0)
        {
            float u, v;
            sscanf(buffer, "vt %f %f", &u, &v);
            u = (u - minU) / (maxU - minU);
            v = (v - minV) / (maxV - minV);
            std::string content = "vt " + std::to_string(u) + " " + std::to_string(v) + '\n';
            fputs(content.c_str(), Outputfile);
        }
        else
        {
            fputs(buffer, Outputfile);
        }
    }
    fclose(Inputfile);
    fclose(Outputfile);
    return 1;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <input.obj> <output.obj>\n";
        return 1;
    }

    std::string inputfile = argv[1];
    std::string outputfile = argv[2];

    int flag = normalize_vt(inputfile, outputfile);
    if(flag == 1){
        std::cout << "success!\n";
    }

    return 0;
}

    // ObjCutter* tempObj = &objCutter;
    // for (int i = 1; i <= numStepsX; i++)
    // {
    //     ObjCutter* cutObjX;
    //     auto oldTempObj = tempObj;
    //     float x = minPoint.x + i * stepSize;
    //     Plane plane = Plane(Vector3(x, minPoint.y, minPoint.z), Vector3(-1, 0, 0));
    //     tempObj->cut(plane, cutObjX, tempObj);
    //     if (i > 1)
    //         delete oldTempObj;

    //     for (int j = 1; j <= numStepsY; j++)
    //     {
    //         ObjCutter* cutObjY;
    //         auto oldCutObjX = cutObjX;
    //         float y = minPoint.y + j * stepSize;
    //         Plane plane2 = Plane(Vector3(x, y, minPoint.z), Vector3(0, -1, 0));
    //         cutObjX->cut(plane2, cutObjY, cutObjX);
    //         delete oldCutObjX;

    //         for (int k = 1; k <= numStepsZ; k++)
    //         {
    //             ObjCutter* cutObj;
    //             auto oldCutObjY = cutObjY;
    //             float z = minPoint.z + k * stepSize;
    //             Plane plane3 = Plane(Vector3(x, y, z), Vector3(0, 0, -1));
    //             cutObjY->cut(plane3, cutObj, cutObjY);

    //             string fileName = outputDir + std::to_string((int)x) + "_" + std::to_string((int)y) + "_" + std::to_string((int)z) + ".obj";
    //             if (cutObj && !cutObj->empty())
    //             {
    //                 cout << endl;
    //                 cutObj->save(fileName);
    //             }
    //             delete cutObj;

    //             delete oldCutObjY;
    //         }
    //         delete cutObjY;
    //     }
    //     delete cutObjX;
    // }