#include <iostream>
#include <fstream>
#include <string>

#include "Const.h"
#include "ObjCutter.h"
#include "Types.h"

using std::cout;
using std::endl;
using std::string;

int main()
{
    ObjCutter objCutter;
    string filePath = FILE_PATH;
    string outputPath = string(filePath, 0, filePath.find_last_of('/')) + "/output.obj";

    bool success = objCutter.load(filePath);
    if (!success)
    {
        cout << "Failed to load OBJ file." << endl;
        return 1;
    }
    objCutter.info();
    Vector3 planeCenter = objCutter.getCenter();
    Plane plane{planeCenter, Vector3{0, 1, 0}};
    objCutter.cut(plane);

    // objFile.save(outputPath);
    // objFile.cmp(filePath, outputPath);
    return 0;
}
