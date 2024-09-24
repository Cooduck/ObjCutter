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

#include "Const.h"
#include "ObjCutter.h"

std::wstring string_to_wstring(const std::string& str);
std::string wstring_to_string(const std::wstring& ws);

std::mutex mtx1, mtx2;
std::condition_variable cv1, cv2;
int done_cnt1 = 0, done_cnt2 = 0;
int end_sign1, end_sign2;
std::queue<std::pair<ObjCutter*, float>> vector1;
std::queue<std::pair<ObjCutter*, std::pair<float, float>>> vector2;
long long count_splited_obj = 0;

void producer1(ObjCutter* objCutter, int numStepsX, float minX, float stepSize, Vector3 minPoint, Vector3 maxPoint) {
    for (int i = 1; i <= numStepsX; i++) {
        float x = minPoint.x + i * stepSize;
        Plane plane = Plane(Vector3(x, minPoint.y, minPoint.z), Vector3(-1, 0, 0));
        ObjCutter* cutObjX = objCutter->cut(plane);
        Plane planeOtherSide = Plane(Vector3(x, maxPoint.y, minPoint.z), Vector3(1, 0, 0));
        auto oldTempObj = objCutter;
        objCutter = objCutter->cut(planeOtherSide);
        if(i > 1){
            delete oldTempObj;
        }

        {
            std::unique_lock<std::mutex> lock(mtx1);
            if (cutObjX && !cutObjX->empty()) {
                vector1.push(std::make_pair(cutObjX, x));
            }
        }
        cv1.notify_all();
    }
    delete objCutter;

    {
        std::unique_lock<std::mutex> lock(mtx1);
        done_cnt1 += 1;
    }
    cv1.notify_all();
}

void producer2(int numStepsY, float minX, float minY, float stepSize, Vector3 minPoint, Vector3 maxPoint) {
    while (true) {
        ObjCutter* cutObjX = nullptr;
        float x;

        {
            std::unique_lock<std::mutex> lock(mtx1);
            cv1.wait(lock, [&] { return !vector1.empty() || done_cnt1 == end_sign1; });

            if (!vector1.empty()) {
                cutObjX = vector1.front().first;
                x = vector1.front().second;
                vector1.pop();
            } else if (done_cnt1 == end_sign1 && vector1.empty()) {
                break;  // 没有任务并且 producer1 已完成
            }
        }

        if (cutObjX) {
            for (int j = 1; j <= numStepsY; j++) {
                float y = minPoint.y + j * stepSize;
                Plane plane2 = Plane(Vector3(x, y, minPoint.z), Vector3(0, -1, 0));
                ObjCutter* cutObjY = cutObjX->cut(plane2);
                auto oldCutObjX = cutObjX;
                Plane plane2OtherSide = Plane(Vector3(x, y, maxPoint.z), Vector3(0, 1, 0));
                cutObjX = cutObjX->cut(plane2OtherSide);
                delete oldCutObjX;

                {
                    std::unique_lock<std::mutex> lock(mtx2);
                    if (cutObjY && !cutObjY->empty()) {
                        vector2.push(std::make_pair(cutObjY, std::make_pair(x, y)));
                    }
                }
                cv2.notify_all(); 
            }
        }
        delete cutObjX;

        {
            std::unique_lock<std::mutex> lock(mtx2);
            done_cnt2 += 1;
        }
        cv2.notify_all(); 
    }
}

void producer3(int numStepsZ, float minX, float minY, float minZ, float stepSize, Vector3 minPoint, Vector3 maxPoint, const std::string outputDir) {
    while (true) {
        ObjCutter* cutObjY = nullptr;
        float x, y;

        {
            std::unique_lock<std::mutex> lock(mtx2);
            cv2.wait(lock, [&] { return !vector2.empty() || done_cnt2 == end_sign2; });

            if (!vector2.empty()) {
                cutObjY = vector2.front().first;
                x = vector2.front().second.first;
                y = vector2.front().second.second;
                vector2.pop();
            } else if (done_cnt2 == end_sign2 && vector2.empty()) {
                break;  // 没有任务并且 producer2 已完成
            }
        }

        if (cutObjY) {
            for (int k = 1; k <= numStepsZ; k++) {
                float z = minPoint.z + k * stepSize;
                Plane plane3 = Plane(Vector3(x, y, z), Vector3(0, 0, -1));
                ObjCutter* cutObj = cutObjY->cut(plane3);

                if (cutObj && !cutObj->empty()) {
                    //string fileName = outputDir + std::to_string((int)x) + "_" + std::to_string((int)y) + "_" + std::to_string((int)z) + ".obj";
                    string fileName = outputDir + std::to_string(count_splited_obj++) + ".obj";
                    cutObj->save(fileName);
                }
                delete cutObj;

                auto oldCutObjY = cutObjY;
                Plane plane3OtherSide = Plane(Vector3(x, y, z), Vector3(0, 0, 1));
                cutObjY = cutObjY->cut(plane3OtherSide);
                delete oldCutObjY;

            }
        }
        delete cutObjY;
    }
}

void splitObj(const std::string& objPath, int x_block_num, int y_block_num, int z_block_num, const std::string& outputDir)
{
    ObjCutter objCutter;
    bool success = objCutter.load(objPath);
    if (!success)
    {
        std::cout << "Failed to load OBJ file." << std::endl;
        return;
    }
    objCutter.info();

    Vector3 minPoint = objCutter.getMinPoint();
    Vector3 maxPoint = objCutter.getMaxPoint();
    std::cout << "minPoint: " << minPoint << std::endl;
    std::cout << "maxPoint: " << maxPoint << std::endl;

    float x_len = std::abs(maxPoint.x - minPoint.x);
    float x_stepSize = std::ceil(x_len / x_block_num);
    float y_len = std::abs(maxPoint.y - minPoint.y);
    float y_stepSize = std::ceil(y_len / y_block_num);
    float z_len = std::abs(maxPoint.z - minPoint.z);
    float z_stepSize = std::ceil(z_len / z_block_num);
    int numStepsX = std::ceil((maxPoint.x - minPoint.x) / x_stepSize);
    int numStepsY = std::ceil((maxPoint.y - minPoint.y) / y_stepSize);
    int numStepsZ = std::ceil((maxPoint.z - minPoint.z) / z_stepSize);

    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    end_sign1 = 1;
    end_sign2 = numStepsX;
    done_cnt1 = 0, done_cnt2 = 0;

    auto begin = std::chrono::system_clock::now();

    std::thread producer1Thread(producer1, &objCutter, numStepsX, minPoint.x, x_stepSize, minPoint, maxPoint);

    std::vector<std::thread> producer2Threads;
    for (int i = 0; i < sysInfo.dwNumberOfProcessors / 2; ++i) {
        producer2Threads.emplace_back(producer2, numStepsY, minPoint.x, minPoint.y, y_stepSize, minPoint, maxPoint);
    }

    std::vector<std::thread> producer3Threads;
    for (int i = 0; i < sysInfo.dwNumberOfProcessors / 2; ++i) {
        producer3Threads.emplace_back(producer3, numStepsZ, minPoint.x, minPoint.y, minPoint.z, z_stepSize, minPoint, maxPoint, outputDir);
    }

    producer1Thread.join();

    for (auto& t : producer2Threads) {
        t.join();
    }

    for (auto& t : producer3Threads) {
        t.join();
    }

    std::chrono::duration<double> cut_spend_time = std::chrono::system_clock::now() - begin;
    std::cout << "The cutting process takes " << cut_spend_time.count() << "s" << std::endl;
}

// void splitObj(const string& objPath, int x_block_num, int y_block_num, int z_block_num, const string& outputDir)
// {
//     ObjCutter objCutter;
//     bool success = objCutter.load(objPath);
//     if (!success)
//     {
//         std::cout << "Failed to load OBJ file." << std::endl;
//         return;
//     }
//     objCutter.info();

//     Vector3 minPoint = objCutter.getMinPoint();
//     Vector3 maxPoint = objCutter.getMaxPoint();
//     std::cout << "minPoint: " << minPoint << std::endl;
//     std::cout << "maxPoint: " << maxPoint << std::endl;

//     float x_len = std::abs(maxPoint.x - minPoint.x);
//     float x_stepSize = std::ceil(x_len / x_block_num);
//     float y_len = std::abs(maxPoint.y - minPoint.y);
//     float y_stepSize = std::ceil(y_len / y_block_num);
//     float z_len = std::abs(maxPoint.z - minPoint.z);
//     float z_stepSize = std::ceil(z_len / z_block_num);

//     // minPoint.x = std::floor(minPoint.x / stepSize) * stepSize;
//     // minPoint.y = std::floor(minPoint.y / stepSize) * stepSize;
//     // minPoint.z = std::floor(minPoint.z / stepSize) * stepSize;
//     // maxPoint.x = std::ceil(maxPoint.x / stepSize) * stepSize;
//     // maxPoint.y = std::ceil(maxPoint.y / stepSize) * stepSize;
//     // maxPoint.z = std::ceil(maxPoint.z / stepSize) * stepSize;
//     int numStepsX = std::ceil((maxPoint.x - minPoint.x) / x_stepSize);
//     int numStepsY = std::ceil((maxPoint.y - minPoint.y) / y_stepSize);
//     int numStepsZ = std::ceil((maxPoint.z - minPoint.z) / z_stepSize);

//     ObjCutter* tempObj = &objCutter;
//     auto begin = std::chrono::system_clock::now();

//     for (int i = 1; i <= numStepsX; i++)
//     {
//         auto oldTempObj = tempObj;
//         float x = minPoint.x + i * x_stepSize;
//         Plane plane = Plane(Vector3(x, minPoint.y, minPoint.z), Vector3(-1, 0, 0));
//         ObjCutter* cutObjX = tempObj->cut(plane);
//         Plane planeOtherSide = Plane(Vector3(x, maxPoint.y, minPoint.z), Vector3(1, 0, 0));
//         tempObj = tempObj->cut(planeOtherSide);
//         if (i > 1)
//             delete oldTempObj;
//         for (int j = 1; j <= numStepsY; j++)
//         {
//             float y = minPoint.y + j * y_stepSize;
//             Plane plane2 = Plane(Vector3(x, y, minPoint.z), Vector3(0, -1, 0));
//             ObjCutter* cutObjY = cutObjX->cut(plane2);

//             auto oldCutObjX = cutObjX;
//             Plane plane2OtherSide = Plane(Vector3(x, y, maxPoint.z), Vector3(0, 1, 0));
//             cutObjX = cutObjX->cut(plane2OtherSide);
//             delete oldCutObjX;
//             for (int k = 1; k <= numStepsZ; k++)
//             {
//                 float z = minPoint.z + k * z_stepSize;
//                 Plane plane3 = Plane(Vector3(x, y, z), Vector3(0, 0, -1));
//                 ObjCutter* cutObj = cutObjY->cut(plane3);
//                 if (cutObj && !cutObj->empty())
//                 {
//                     // string fileName = outputDir + std::to_string((int)x) + "_" + std::to_string((int)y) + "_" + std::to_string((int)z) + ".obj";
//                     string fileName = outputDir + std::to_string(count_splited_obj++) + ".obj";
//                     cutObj->save(fileName);
//                 }
//                 delete cutObj;
//                 auto oldCutObjY = cutObjY;
//                 Plane plane3OtherSide = Plane(Vector3(x, y, z), Vector3(0, 0, 1));
//                 cutObjY = cutObjY->cut(plane3OtherSide);
//                 delete oldCutObjY;
//             }
//             delete cutObjY;
//         }
//         delete cutObjX;
//     }
//     std::chrono::duration<double> cut_spend_time = std::chrono::system_clock::now() - begin;
//     std::cout << "The cutting process takes " << cut_spend_time.count() << "s" << std::endl;
// }


// 复制文件
void copy_file(const std::wstring& src, const std::wstring& dest) {
    CopyFileW(src.c_str(), dest.c_str(), FALSE);
}

// 递归复制文件夹
void copy_directory(std::wstring wsrc, std::wstring wdest) {

    _WDIR *dir = _wopendir(wsrc.c_str());
    if (dir == NULL) {
        std::cerr << "Unable to open source directory." << std::endl;
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

std::string ensureTrailingBackslash(const std::string& folderPath) {
    if (!folderPath.empty() && folderPath.back() != '\\') {
        return folderPath + "\\";
    }
    return folderPath;
}

int check_path(const std::string& filepath) {
    std::filesystem::path path = filepath;
    if (std::filesystem::exists(path)) {
        if (std::filesystem::is_directory(path)) {
            return 1;
        }
        else if (std::filesystem::is_regular_file(path)) {
            if (path.extension() == ".obj") {
                return 2;
            }
            else {
                std::cerr << "Error: Target file is not an obj file." << std::endl;
                return 0;
            }
        }
        else {
            std::cerr << "Error: Target id not a regular file or directory." << std::endl;
            return 0;
        }
    } else {
        std::cerr << "Error: Target directory/ObjFile " << path << " does not exist." << std::endl;
        return 0;
    }
}

int main(int argc, char* argv[])
{
    SetConsoleOutputCP(CP_UTF8);

    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <targetDir/targetObj> <outputDir> [x_block_num] [y_block_num] [z_block_num]" << std::endl;
        std::cerr << "<targetDir/targetObj> <outputDir> is required, the other are optional." << std::endl;
        return 1;
    }

    std::string targetDir = argv[1];
    // std::string targetDir = ".\\terra_obj\\";
    int path_flag = check_path(targetDir);
    if(path_flag == 0){
        return 0;
    }
    else if(path_flag == 1){
        targetDir = ensureTrailingBackslash(targetDir);
    }
    std::string outputDir = argv[2];
    // std::string outputDir = ".\\splited_obj\\";
    outputDir = ensureTrailingBackslash(outputDir);
    int x_block_num = 2;
    int y_block_num = 2;
    int z_block_num = 2; 

    if (argc >= 4) {
        x_block_num = std::stoi(argv[3]);
    }
    if (argc >= 5) {
        y_block_num = std::stoi(argv[4]);
    }
    if (argc >= 6) {
        z_block_num = std::stoi(argv[5]);
    }

    if(path_flag == 1){
        std::filesystem::create_directory(outputDir);
        //将输出重定向到文件
        freopen(std::string(outputDir + "log.txt").c_str(), "w", stdout);
        // 遍历该文件夹
        for (auto& p : std::filesystem::directory_iterator(targetDir))
        {
            if (p.is_directory())
            {
                auto begin = std::chrono::system_clock::now();
                count_splited_obj = 0;
                std::string pStr = p.path().string();
                std::string folderName = pStr.substr(pStr.find_last_of("\\") + 1);
                std::string outputDirSplited = outputDir + folderName + "\\";
                std::filesystem::create_directory(outputDirSplited);
                for (auto& p2 : std::filesystem::directory_iterator(p.path()))
                {
                    if (p2.is_regular_file() && p2.path().extension() == ".obj")
                    {
                        std::string objPath = p2.path().string();
                        splitObj(objPath, x_block_num, y_block_num, z_block_num, outputDirSplited);
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
                std::cout << std::endl;
                std::cout << "finished: " << pStr << " to " << outputDirSplited << std::endl;
                std::cerr << "finished: " << pStr << " to " << outputDirSplited << std::endl;
                std::cout << "A total of " << count_splited_obj << " splited files are cut out" << std::endl;
                std::cout << "The whole process takes " << cut_spend_time.count() << "s" << std::endl;
                std::cout << "---------------------------------------------------------------------------" << std::endl;
                std::cout << std::endl;
            }
        }
    }
    else if(path_flag == 2){
        std::filesystem::create_directory(outputDir);
        //将输出重定向到文件
        freopen(std::string(outputDir + "log.txt").c_str(), "w", stdout);

        auto begin = std::chrono::system_clock::now();
        count_splited_obj = 0;
        std::string folderName = targetDir.substr(targetDir.find_last_of("\\") + 1);
        folderName = folderName.substr(0, folderName.size() - 4);
        std::string outputDirSplited = outputDir + folderName + "\\";
        std::filesystem::create_directory(outputDirSplited);

        splitObj(targetDir, x_block_num, y_block_num, z_block_num, outputDirSplited);

        std::chrono::duration<double> cut_spend_time = std::chrono::system_clock::now() - begin;
        std::cout << std::endl;
        std::cout << "finished: " << targetDir << " to " << outputDirSplited << std::endl;
        std::cerr << "finished: " << targetDir << " to " << outputDirSplited << std::endl;
        std::cout << "A total of " << count_splited_obj << " splited files are cut out" << std::endl;
        std::cout << "The whole process takes " << cut_spend_time.count() << "s" << std::endl;
        std::cout << "---------------------------------------------------------------------------" << std::endl;
        std::cout << std::endl;
    }

    std::cerr << "All done!" << std::endl;
    return 0;
}