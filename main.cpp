#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>

#include "Const.h"
#include "ObjCutter.h"

using std::cout;
using std::endl;
using std::string;

bool getIntersectPointTest(Plane plane, Vector3 p1, Vector3 p2, Vector3 intersectPoint)
{
    Vector3 ans = ObjCutter::getIntersectPoint(p1, p2, plane);
    cout << "intersectPoint: " << intersectPoint << endl;
    cout << "ans: " << ans << endl;
    if (ans.equals(intersectPoint))
    {
        cout << "getIntersectPointTest passed." << endl;
        return true;
    }
    cout << "getIntersectPointTest failed." << endl;
    return false;
}

int main()
{
    ObjCutter objCutter;
    string filePath = FILE_PATH;
    string file1Path = string(filePath, 0, filePath.find_last_of('/')) + "/test_save.obj";
    string file2tPath = string(filePath, 0, filePath.find_last_of('/')) + "/test_cut.obj";

    bool success = objCutter.load(filePath);
    if (!success)
    {
        cout << "Failed to load OBJ file." << endl;
        return 1;
    }
    objCutter.info();
    // objCutter.save(file1Path);
    Vector3 planeCenter = objCutter.getCenter();
    Plane plane{planeCenter, Vector3{0, -1, 0}};
    objCutter.cut(plane);

    // auto p1 = planeCenter + Vector3{0, 1, 0};
    // auto p2 = planeCenter - Vector3{0, 1, 0};
    // getIntersectPointTest(plane, p1, p2, planeCenter);

    // objFile.save(outputPath);
    // objCutter.cmp(file1Path, file2tPath);
    return 0;
}
