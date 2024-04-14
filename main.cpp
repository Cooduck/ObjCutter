#include <cmath>
#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>

#include "Const.h"
#include "ObjCutter.h"

using std::cout;
using std::endl;
using std::string;

int main()
{
    ObjCutter objCutter;
    string filePath = FILE_PATH;
    bool success = objCutter.load(filePath);
    if (!success)
    {
        cout << "Failed to load OBJ file." << endl;
        return 1;
    }
    objCutter.info();

    Vector3 minPoint = objCutter.getMinPoint();
    Vector3 maxPoint = objCutter.getMaxPoint();
    cout << "minPoint: " << minPoint << endl;
    cout << "maxPoint: " << maxPoint << endl;
    float stepSize = 50;
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
                string fileName = "output/" + std::to_string((int)x) + "_" + std::to_string((int)y) + "_" + std::to_string((int)z);
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
    return 0;
}
