#include <iostream>
#include <locale>
#include <vector>
#include <clocale>
#include <cstring>


int main() {
    // 设置C语言的locale
    setlocale(LC_ALL, "");

    // 获取系统支持的所有locale
    std::vector<std::string> locales;
    std::string current_locale;

    // 使用newlocale获取所有locale
    locale_t loc = newlocale(LC_ALL_MASK, "", nullptr);
    if (loc) {
        char* loc_name = setlocale(LC_ALL, nullptr);
        if (loc_name) {
            locales.push_back(loc_name);
        }
        freelocale(loc);
    }

    // 输出所有locale
    for (const auto& loc : locales) {
        std::cout << loc << std::endl;
    }

    return 0;
}
