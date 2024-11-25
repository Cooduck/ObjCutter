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
#include <functional>

#include "Const.h"
#include "ObjCutter.h"

std::wstring string_to_wstring(const std::string& str);
std::string wstring_to_string(const std::wstring& ws);

std::mutex mtx1, mtx2;
std::condition_variable cv1, cv2;
int done_cnt1 = 0, done_cnt2 = 0;
int end_sign1, end_sign2;
std::deque<std::pair<std::unique_ptr<ObjCutter>, float>> vector1;
std::deque<std::pair<std::unique_ptr<ObjCutter>, std::pair<float, float>>> vector2;
long long count_splited_obj = 0;
float model_minz = 0;
Vector3 modelBottomCenter, ue5_model_center, ue5_center, cutting_step;;
float scale;

void producer1(std::unique_ptr<ObjCutter> objCutter, const int &numStepsX, const float &stepSize, const Vector3 &minPoint, const Vector3 &maxPoint) {
    for (int i = 1; i <= numStepsX; i++) {
        float x = minPoint.x + i * stepSize;
        Plane plane = Plane(Vector3(x, minPoint.y, minPoint.z), Vector3(-1, 0, 0));
        auto cutObjX = objCutter->cut(plane);
        Plane planeOtherSide = Plane(Vector3(x, minPoint.y, minPoint.z), Vector3(1, 0, 0));
        auto oldTempObj = std::move(objCutter);
        objCutter = oldTempObj->cut(planeOtherSide);

        {
            std::unique_lock<std::mutex> lock(mtx1);
            if(!cutObjX || cutObjX->empty()){
                end_sign2 -= 1;
            }
            else if(cutObjX && !cutObjX->empty()) {
                vector1.emplace_back(std::make_pair(std::move(cutObjX), x));
            }
        }
        cv1.notify_all();
    }

    {
        std::unique_lock<std::mutex> lock(mtx1);
        done_cnt1 += 1;
    }
    cv1.notify_all();
}

void producer2(const int &numStepsY, const float &stepSize, const Vector3 &minPoint, const Vector3 &maxPoint) {
    while (true) {
        std::unique_ptr<ObjCutter> cutObjX = nullptr;
        float x;

        {
            std::unique_lock<std::mutex> lock(mtx1);
            cv1.wait(lock, [&] { return !vector1.empty() || done_cnt1 == end_sign1; });

            if (!vector1.empty()) {
                cutObjX = std::move(vector1.front().first);
                x = vector1.front().second;
                vector1.pop_front();
            } else if (done_cnt1 == end_sign1 && vector1.empty()) {
                break;  // 没有任务并且 producer1 已完成
            }
        }

        if (cutObjX) {
            for (int j = 1; j <= numStepsY; j++) {
                float y = minPoint.y + j * stepSize;
                Plane plane2 = Plane(Vector3(x, y, minPoint.z), Vector3(0, -1, 0));
                auto cutObjY = cutObjX->cut(plane2);
                auto oldCutObjX = std::move(cutObjX);
                Plane plane2OtherSide = Plane(Vector3(x, y, minPoint.z), Vector3(0, 1, 0));
                cutObjX = oldCutObjX->cut(plane2OtherSide);

                {
                    std::unique_lock<std::mutex> lock(mtx2);
                    if (cutObjY && !cutObjY->empty()) {
                        vector2.emplace_back(std::make_pair(std::move(cutObjY), std::make_pair(x, y)));
                    }
                }
                cv2.notify_all(); 
            }
        }

        {
            std::unique_lock<std::mutex> lock(mtx2);
            done_cnt2 += 1;
        }
        cv2.notify_all(); 
    }
}

void producer3(const int &numStepsZ, const float &stepSize, const Vector3 &minPoint, const std::string &outputDir) {
    while (true) {
        std::unique_ptr<ObjCutter> cutObjY = nullptr;
        float x, y;

        {
            std::unique_lock<std::mutex> lock(mtx2);
            cv2.wait(lock, [&] { return !vector2.empty() || done_cnt2 == end_sign2; });

            if (!vector2.empty()) {
                cutObjY = std::move(vector2.front().first);
                x = vector2.front().second.first;
                y = vector2.front().second.second;
                vector2.pop_front();
            } else if (done_cnt2 == end_sign2 && vector2.empty()) {
                break;  // 没有任务并且 producer2 已完成
            }
        }

        if (cutObjY) {
            for (int k = 1; k <= numStepsZ; k++) {
                float z = minPoint.z + k * stepSize;
                Plane plane3 = Plane(Vector3(x, y, z), Vector3(0, 0, -1));
                auto cutObj = cutObjY->cut(plane3);
                    
                if (cutObj && !cutObj->empty()) {
                    // 计算区块中心点的坐标
                    cutObj->setblockCenter(x - cutting_step.x * 0.5, y - cutting_step.y * 0.5, z - cutting_step.z * 0.5);
                    Vector3 ue5_cutting_model_center = (cutObj->getblockCenter() - modelBottomCenter + ue5_model_center) * scale;
                    string fileName = outputDir + std::to_string(std::round(ue5_cutting_model_center.x)) + "_" + std::to_string(std::round(ue5_cutting_model_center.y)) + "_" + std::to_string(std::round(ue5_cutting_model_center.z)) + ".obj";
                    // string fileName = outputDir + std::to_string(count_splited_obj++) + ".obj";
                    cutObj->save(fileName, model_minz);
                }

                auto oldCutObjY = std::move(cutObjY);
                Plane plane3OtherSide = Plane(Vector3(x, y, z), Vector3(0, 0, 1));
                cutObjY = oldCutObjY->cut(plane3OtherSide);
            }
        }
    }
}

void preProcess(const Vector3 &ue5_center, const Vector3 &ue5_model_center, const Vector3 &cutting_step, Vector3 &minPoint, Vector3 &maxPoint){
    // 处理x轴
    int xidx = trunc((ue5_model_center.x - ue5_center.x) / abs(cutting_step.x));
    float initx = ue5_center.x + xidx * cutting_step.x, tmpx;   // ue5中最靠近模型中心并且在切割线上的坐标x
    initx = -(ue5_model_center.x - initx), tmpx = initx;    // 转换为obj中最靠近模型中心并且在切割线上的坐标x
    while(tmpx < maxPoint.x){
        tmpx += cutting_step.x;
    }
    maxPoint.x = tmpx;
    tmpx = initx;
    while(tmpx > minPoint.x){
        tmpx -= cutting_step.x;
    }
    minPoint.x = tmpx;

    // 处理y轴
    int yidx = trunc((ue5_model_center.y - ue5_center.y) / abs(cutting_step.y));
    float inity = ue5_center.y + yidx * cutting_step.y, tmpy = inity;   // ue5中最靠近模型中心并且在切割线上的坐标y
    inity = -(ue5_model_center.y - inity), tmpy = inity;    // 转换为obj中最靠近模型中心并且在切割线上的坐标y
    while(tmpy < maxPoint.y){
        tmpy += cutting_step.y;
    }
    maxPoint.y = tmpy;
    tmpy = inity;
    while(tmpy > minPoint.y){
        tmpy -= cutting_step.y;
    }
    minPoint.y = tmpy;

    // 处理z轴
    int zidx = trunc((ue5_model_center.z - ue5_center.z) / abs(cutting_step.z));
    float initz = ue5_center.z + zidx * cutting_step.z, tmpz = initz;   // ue5中最靠近模型中心并且在切割线上的坐标z
    initz = -(ue5_model_center.z - initz), tmpz = initz;    // 转换为obj中最靠近模型中心并且在切割线上的坐标z
    while(tmpz < maxPoint.z){
        tmpz += cutting_step.z;
    }
    maxPoint.z = tmpz;
    tmpz = initz;
    while(tmpz > minPoint.z){
        tmpz -= cutting_step.z;
    }
    minPoint.z = tmpz;
}

void splitObj(const std::string& objPath, const std::string& outputDir, const Vector3 &ue5_center, const Vector3 &ue5_model_center, const Vector3 &cutting_step)
{
    std::unique_ptr<ObjCutter> objCutter = std::make_unique<ObjCutter>(objPath);
    bool success = objCutter->load(objPath);
    if (!success)
    {
        std::cerr << "Failed to load file "<< objPath << "." << std::endl;
        std::cout << "Failed to load OBJ file." << std::endl;
        return;
    }
    objCutter->info();

    Vector3 minPoint = objCutter->getMinPoint();
    Vector3 maxPoint = objCutter->getMaxPoint();
    std::cout << "minPoint: " << minPoint << std::endl;
    std::cout << "maxPoint: " << maxPoint << std::endl << std::endl;
    model_minz = minPoint.z;
    modelBottomCenter = objCutter->getBottomCenter();
    
    preProcess(ue5_center, ue5_model_center, cutting_step, minPoint, maxPoint);
    std::cout << "After preProcess: " << std::endl;
    std::cout << "minPoint: " << minPoint << std::endl;
    std::cout << "maxPoint: " << maxPoint << std::endl;

    int numStepsX = std::ceil((maxPoint.x - minPoint.x) / cutting_step.x);
    int numStepsY = std::ceil((maxPoint.y - minPoint.y) / cutting_step.y);
    int numStepsZ = std::ceil((maxPoint.z - minPoint.z) / cutting_step.z);

    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    end_sign1 = 1;
    end_sign2 = numStepsX;
    done_cnt1 = 0, done_cnt2 = 0;

    auto begin = std::chrono::system_clock::now();

    std::thread producer1Thread(producer1, std::move(objCutter), std::cref(numStepsX), std::cref(cutting_step.x), std::cref(minPoint), std::cref(maxPoint));

    std::vector<std::thread> producer2Threads;
    for (int i = 0; i < sysInfo.dwNumberOfProcessors / 2; ++i) {
        producer2Threads.emplace_back(producer2, std::cref(numStepsY), std::cref(cutting_step.y), std::cref(minPoint), std::cref(maxPoint));
    }

    std::vector<std::thread> producer3Threads;
    for (int i = 0; i < sysInfo.dwNumberOfProcessors / 2; ++i) {
        producer3Threads.emplace_back(producer3, std::cref(numStepsZ), std::cref(cutting_step.z), std::cref(minPoint), std::cref(outputDir));
    }

    producer1Thread.join();

    for (auto& t : producer2Threads) {
        t.join();
    }

    for (auto& t : producer3Threads) {
        t.join();
    }

    std::chrono::duration<float> cut_spend_time = std::chrono::system_clock::now() - begin;
    std::cout << "The cutting process takes " << cut_spend_time.count() << "s" << std::endl;
}

// void splitObj(const string& objPath, const string& outputDir, const Vector3 &ue5_center, const Vector3 &ue5_model_center, const Vector3 &cutting_step)
// {
//     std::unique_ptr<ObjCutter> objCutter = std::make_unique<ObjCutter>(objPath);
//     bool success = objCutter->load(objPath);
//     if (!success)
//     {
//         std::cout << "Failed to load OBJ file." << std::endl;
//         return;
//     }
//     objCutter->info();

//     Vector3 minPoint = objCutter->getMinPoint();
//     Vector3 maxPoint = objCutter->getMaxPoint();
//     std::cout << "minPoint: " << minPoint << std::endl;
//     std::cout << "maxPoint: " << maxPoint << std::endl;
//     model_minz = minPoint.z;

//     preProcess(ue5_center, ue5_model_center, cutting_step, minPoint, maxPoint);
//     std::cout << "After preProcess: " << std::endl;
//     std::cout << "minPoint: " << minPoint << std::endl;
//     std::cout << "maxPoint: " << maxPoint << std::endl;

//     int numStepsX = std::ceil((maxPoint.x - minPoint.x) / cutting_step.x);
//     int numStepsY = std::ceil((maxPoint.y - minPoint.y) / cutting_step.y);
//     int numStepsZ = std::ceil((maxPoint.z - minPoint.z) / cutting_step.z);

//     auto begin = std::chrono::system_clock::now();

//     for (int i = 1; i <= numStepsX; i++)
//     {
//         float x = minPoint.x + i * cutting_step.x;
//         Plane plane = Plane(Vector3(x, minPoint.y, minPoint.z), Vector3(-1, 0, 0));
//         auto cutObjX = objCutter->cut(plane);
//         Plane planeOtherSide = Plane(Vector3(x, minPoint.y, minPoint.z), Vector3(1, 0, 0));
//         auto oldTempObj = std::move(objCutter);
//         objCutter = oldTempObj->cut(planeOtherSide);

//         for (int j = 1; j <= numStepsY; j++)
//         {
//             float y = minPoint.y + j * cutting_step.y;
//             Plane plane2 = Plane(Vector3(x, y, minPoint.z), Vector3(0, -1, 0));
//             auto cutObjY = cutObjX->cut(plane2);
//             auto oldCutObjX = std::move(cutObjX);
//             Plane plane2OtherSide = Plane(Vector3(x, y, minPoint.z), Vector3(0, 1, 0));
//             cutObjX = oldCutObjX->cut(plane2OtherSide);
            
//             for (int k = 1; k <= numStepsZ; k++)
//             {
//                 float z = minPoint.z + k * cutting_step.z;
//                 Plane plane3 = Plane(Vector3(x, y, z), Vector3(0, 0, -1));
//                 auto cutObj = cutObjY->cut(plane3);
//                 if (cutObj && !cutObj->empty())
//                 {
//                     cutObj->block_center.x = (x + (x - cutting_step.x)) * 0.5;
//                     cutObj->block_center.y = (y + (y - cutting_step.y)) * 0.5;
//                     cutObj->block_center.z = (z + (z - cutting_step.z)) * 0.5;
//                     Vector3 ue5_cutting_model_center = (cutObj->getCenter() - modelBottomCenter + ue5_model_center) * scale;
//                     string fileName = outputDir + std::to_string(ue5_cutting_model_center.x) + "_" + std::to_string(ue5_cutting_model_center.y) + "_" + std::to_string(ue5_cutting_model_center.z) + ".obj";
//                     // string fileName = outputDir + std::to_string((int)x) + "_" + std::to_string((int)y) + "_" + std::to_string((int)z) + ".obj";
//                     // string fileName = outputDir + std::to_string(count_splited_obj++) + ".obj";
//                     cutObj->save(fileName, model_minz);
//                 }
                
//                 auto oldCutObjY = std::move(cutObjY);
//                 Plane plane3OtherSide = Plane(Vector3(x, y, z), Vector3(0, 0, 1));
//                 cutObjY = oldCutObjY->cut(plane3OtherSide);
//             }
//         }
//     }
//     std::chrono::duration<float> cut_spend_time = std::chrono::system_clock::now() - begin;
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

    if (argc < 13) {
        std::cerr << "Usage: " << argv[0] << " <targetObj> <outputDir> " \
                    << "[ue5_model_center_x] [ue5_model_center_y] [ue5_model_center_z] " \
                    << "[ue5_center_x] [ue5_center_y] [ue5_center_z] " \
                    << "[cutting_step_x] [cutting_step_y] [cutting_step_z] [scale]" \
                    << std::endl;
        std::cerr << "<targetDir/targetObj> <outputDir> is required, the other are optional." << std::endl;
        return 1;
    }

    std::string targetDir = argv[1];
    // std::string targetDir = ".\\tx.obj";
    // std::string targetDir = ".\\cube.obj";
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

    scale = 100;
    ue5_model_center.x = std::stof(argv[3]);
    ue5_model_center.y = std::stof(argv[4]);
    ue5_model_center.z = std::stof(argv[5]);
    ue5_center.x = std::stof(argv[6]);
    ue5_center.y = std::stof(argv[7]);
    ue5_center.z = std::stof(argv[8]);
    cutting_step.x = std::stof(argv[9]);
    cutting_step.y = std::stof(argv[10]);
    cutting_step.z = std::stof(argv[11]);
    scale = std::stof(argv[12]);
    // ue5_model_center.x = 0.0;
    // ue5_model_center.y = 0.0;
    // ue5_model_center.z = 0.0;
    // ue5_center.x = 0;
    // ue5_center.y = 0;
    // ue5_center.z = 0;
    // cutting_step.x = 2000;
    // cutting_step.y = 2000;
    // cutting_step.z = 6000;
    // scale = 100;
    // ue5_model_center.x = 0.0;
    // ue5_model_center.y = 0.0;
    // ue5_model_center.z = 0.0;
    // ue5_center.x = 0.0;
    // ue5_center.y = 0.0;
    // ue5_center.z = 0.0;
    // cutting_step.x = 15;
    // cutting_step.y = 15;
    // cutting_step.z = 15;
    // scale = 1;

    ue5_center = ue5_center / scale;
    ue5_model_center = ue5_model_center / scale;
    cutting_step = cutting_step / scale;

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
                        splitObj(objPath, outputDirSplited, ue5_center, ue5_model_center, cutting_step);
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
                std::chrono::duration<float> cut_spend_time = std::chrono::system_clock::now() - begin;
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

        splitObj(targetDir, outputDirSplited, ue5_center, ue5_model_center, cutting_step);

        std::chrono::duration<float> cut_spend_time = std::chrono::system_clock::now() - begin;
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