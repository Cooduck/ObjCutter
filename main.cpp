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
    bool success = objFile.load(filePath);
    if (!success)
    {
        cout << "Failed to load OBJ file." << endl;
        return 1;
    }

    objFile.info();
    return 0;
}
