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

#include "Const.h"
#include "ObjCutter.h"

using std::cout;
using std::endl;
using std::string;

void splitObj(const string& objPath, float stepSize, const string& outputDir)
{
    ObjCutter objCutter;
    bool success = objCutter.load(objPath);
    if (!success)
    {
        cout << "Failed to load OBJ file." << endl;
        return;
    }
    objCutter.info();

    Vector3 minPoint = objCutter.getMinPoint();
    Vector3 maxPoint = objCutter.getMaxPoint();
    cout << "minPoint: " << minPoint << endl;
    cout << "maxPoint: " << maxPoint << endl;
    minPoint.x = std::floor(minPoint.x / stepSize) * stepSize;
    minPoint.y = std::floor(minPoint.y / stepSize) * stepSize;
    minPoint.z = std::floor(minPoint.z / stepSize) * stepSize;
    maxPoint.x = std::ceil(maxPoint.x / stepSize) * stepSize;
    maxPoint.y = std::ceil(maxPoint.y / stepSize) * stepSize;
    maxPoint.z = std::ceil(maxPoint.z / stepSize) * stepSize;
    cout << "minPoint: " << minPoint << endl;
    cout << "maxPoint: " << maxPoint << endl;
    int numStepsX = (maxPoint.x - minPoint.x) / stepSize;
    int numStepsY = (maxPoint.y - minPoint.y) / stepSize;
    int numStepsZ = (maxPoint.z - minPoint.z) / stepSize;
    cout << "numStepsX: " << numStepsX << endl;
    cout << "numStepsY: " << numStepsY << endl;
    cout << "numStepsZ: " << numStepsZ << endl;

    ObjCutter* tempObj = &objCutter;
    for (int i = 1; i <= numStepsX; i++)
    {
        auto oldTempObj = tempObj;
        float x = minPoint.x + i * stepSize;
        Plane plane = Plane(Vector3(x, minPoint.y, minPoint.z), Vector3(-1, 0, 0));
        ObjCutter* cutObjX = tempObj->cut(plane);
        Plane planeOtherSide = Plane(Vector3(x, maxPoint.y, minPoint.z), Vector3(1, 0, 0));
        tempObj = tempObj->cut(planeOtherSide);
        if (i > 1)
            delete oldTempObj;

        for (int j = 1; j <= numStepsY; j++)
        {
            float y = minPoint.y + j * stepSize;
            Plane plane2 = Plane(Vector3(x, y, minPoint.z), Vector3(0, -1, 0));
            ObjCutter* cutObjY = cutObjX->cut(plane2);
            auto oldCutObjX = cutObjX;
            Plane plane2OtherSide = Plane(Vector3(x, y, maxPoint.z), Vector3(0, 1, 0));
            cutObjX = cutObjX->cut(plane2OtherSide);
            delete oldCutObjX;

            for (int k = 1; k <= numStepsZ; k++)
            {
                float z = minPoint.z + k * stepSize;
                Plane plane3 = Plane(Vector3(x, y, z), Vector3(0, 0, -1));
                ObjCutter* cutObj = cutObjY->cut(plane3);

                string fileName = outputDir + std::to_string((int)x) + "_" + std::to_string((int)y) + "_" + std::to_string((int)z) + ".obj";
                if (cutObj && !cutObj->empty())
                {
                    cout << endl;
                    cutObj->save(fileName);
                }
                delete cutObj;

                auto oldCutObjY = cutObjY;
                Plane plane3OtherSide = Plane(Vector3(x, y, z), Vector3(0, 0, 1));
                cutObjY = cutObjY->cut(plane3OtherSide);
                delete oldCutObjY;
            }
            delete cutObjY;
        }
        delete cutObjX;
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
}

// void copy_file(string src, string dest) {
//     FILE* src_file = _wfopen(string_to_wstring(src).c_str(), L"r");
//     if (src_file == NULL) {
//         perror("Unable to open source file.");
//         return;
//     }

//     FILE *dest_file = _wfopen(string_to_wstring(dest).c_str(), L"w");
//     if (dest_file == NULL) {
//         perror("Unable to create target file.");
//         fclose(src_file);
//         return;
//     }

//     char buffer[1024];
//     size_t bytes;
//     while ((bytes = fread(buffer, 1, sizeof(buffer), src_file)) > 0) {
//         fwrite(buffer, 1, bytes, dest_file);
//     }

//     fclose(src_file);
//     fclose(dest_file);
// }

std::wstring string_to_wstring(const std::string& str);

void copy_file(const std::wstring& src, const std::wstring& dest) {
    CopyFileW(src.c_str(), dest.c_str(), FALSE);
}

void copy_directory(std::wstring wsrc, std::wstring wdest) {

    _WDIR *dir = _wopendir(wsrc.c_str());
    if (dir == NULL) {
        perror("Unable to open source directory.");
        return;
    }

    // 创建目标目录
    _wmkdir(wdest.c_str());

    struct _wdirent *entry;
    while ((entry = _wreaddir(dir)) != NULL) {

        if (wcscmp(entry->d_name, L".") == 0 || wcscmp(entry->d_name, L"..") == 0) {
            continue; 
        }

        wchar_t src_path[4096];
        wchar_t dest_path[4096];

        swprintf(src_path, sizeof(src_path) / sizeof(wchar_t), L"%ls/%ls", wsrc.c_str(), entry->d_name);
        swprintf(dest_path, sizeof(dest_path) / sizeof(wchar_t), L"%ls/%ls", wdest.c_str(), entry->d_name);

        struct _stat stat_buf;
        _wstat(src_path, &stat_buf);

        if (S_ISDIR(stat_buf.st_mode)) {
            // Recursively copy subdirectories
            copy_directory(std::wstring(src_path), std::wstring(dest_path));
        } else if (S_ISREG(stat_buf.st_mode)) {
            // Copy file
            copy_file(src_path, dest_path);
        }
    }
    _wclosedir(dir);
}

int main()
{
    string targetDir = "C:\\Users\\12569\\Desktop\\ObjCutter\\ObjCutter\\terra_obj\\";
    string outputDir = "C:\\Users\\12569\\Desktop\\ObjCutter\\ObjCutter\\splited_obj\\";
    std::filesystem::create_directory(outputDir);
    //将输出重定向到文件
    freopen(string(outputDir + "log.txt").c_str(), "w", stdout);
    // 遍历该文件夹
    for (auto& p : std::filesystem::directory_iterator(targetDir))
    {
        if (p.is_directory())
        {
            auto begin = std::chrono::system_clock::now();
            string pStr = p.path().string();
            string folderName = pStr.substr(pStr.find_last_of("\\") + 1);
            string outputDirSplited = outputDir + folderName + "\\";
            std::filesystem::create_directory(outputDirSplited);
            for (auto& p2 : std::filesystem::directory_iterator(p.path()))
            {
                if (p2.is_regular_file() && p2.path().extension() == ".obj")
                {
                    string objPath = p2.path().string();
                    splitObj(objPath, 150000, outputDirSplited);
                }
                else if(p2.is_directory())
                {
                    // 复制目录
                    copy_directory(string_to_wstring(p2.path().string()), string_to_wstring(outputDirSplited + p2.path().filename().string()));
                }
                else{
                    // 复制文件
                    copy_file(string_to_wstring(p2.path().string()), string_to_wstring(outputDirSplited + p2.path().filename().string()));
                }
                
            }
            std::chrono::duration<double> cut_spend_time = std::chrono::system_clock::now() - begin;
            std::cout << endl;
            std::cout << "finished: " << pStr << " to " << outputDirSplited << endl;
            std::cerr << "finished: " << pStr << " to " << outputDirSplited << endl;
            std::cout << "The cutting process takes " << cut_spend_time.count() << "s" << endl;
        }
    }
    // string targetDir = "D:/BlockYAYX";
    // string objPath = targetDir + "/" + targetDir.substr(targetDir.find_last_of("/\\") + 1) + ".obj";
    // splitObj(objPath, 10, "D:/BaiduNetdiskDownload/splited_obj/");
    return 0;
}
