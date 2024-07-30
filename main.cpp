#include <cmath>
#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <filesystem>
#include <windows.h>

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
            string pStr = p.path().string();
            string folderName = pStr.substr(pStr.find_last_of("\\") + 1);
            string outputDirSplited = outputDir + folderName + "\\";
            //std::filesystem::create_directory(outputDirSplited);
            for (auto& p2 : std::filesystem::directory_iterator(p.path()))
            {
                if (p2.is_regular_file() && p2.path().extension() == ".obj")
                {
                    string objPath = p2.path().string();
                    splitObj(objPath, 150000, outputDirSplited);
                }
                else
                {
                    // 复制到输出目录
                    string fileName = p2.path().filename().string();
                    string outputPath = outputDirSplited + fileName;
                    std::ifstream ifs(p2.path().string(), std::ios::binary);
                    std::ofstream ofs(outputPath, std::ios::binary);
                    ofs << ifs.rdbuf();
                    ifs.close();
                    ofs.close();
                }
            }
            std::cout << "finished: " << pStr << " to " << outputDirSplited << endl;
            std::cerr << "finished: " << pStr << " to " << outputDirSplited << endl;
        }
    }
    // string targetDir = "D:/BlockYAYX";
    // string objPath = targetDir + "/" + targetDir.substr(targetDir.find_last_of("/\\") + 1) + ".obj";
    // splitObj(objPath, 10, "D:/BaiduNetdiskDownload/splited_obj/");
    return 0;
}
