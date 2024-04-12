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

    Vector3 planeCenter = objCutter.getCenter();
    Plane plane_smaller_y{planeCenter, Vector3{0, -1, 0}};
    Plane plane_bigger_y{planeCenter, Vector3{0, 1, 0}};
    cout << endl;
    objCutter.cut(plane_smaller_y, "smaller_y");
    cout << endl;
    objCutter.cut(plane_bigger_y, "bigger_y");
    return 0;
}
