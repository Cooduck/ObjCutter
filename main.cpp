#include <iostream>
#include <fstream>
#include <string>

#include "Const.h"
#include "Types.h"

using std::cout;
using std::endl;
using std::string;

int main()
{
    ObjFile objFile;
    string filePath = FILE_PATH;
    string outputPath = string(filePath, 0, filePath.find_last_of('/')) + "/output.obj";

    bool success = objFile.load(filePath);
    if (!success)
    {
        cout << "Failed to load OBJ file." << endl;
        return 1;
    }
    objFile.info();
    objFile.save(outputPath);

    // objFile.cmp(filePath, outputPath);
    return 0;
}
