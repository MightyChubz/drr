// Windows headers for file system code
#include <windows.h>

#include <iostream>
#include <chrono>
#include <string>
#include <filesystem>
#include <array>

#define FILE "<FILE>"
#define DIRECTORY "<DIR>"

static unsigned int longestLength = 0;
typedef std::chrono::duration<int, std::ratio<31556952>> years;

struct File {
    std::string fileName;
    std::string type;
    years creationTime;
    uintmax_t fileSize;
    File(std::string  fileName, std::string  type, years creationTime, uintmax_t fileSize) :
            fileName(std::move(fileName)), type(std::move(type)), creationTime(creationTime), fileSize(fileSize) {}
};

File createFileStruct(const std::filesystem::directory_entry *name) {
    auto creationTime = name->last_write_time().time_since_epoch();
    auto fileSize = name->file_size();
    auto type = name->is_directory() ? DIRECTORY : FILE;
    return {name->path().filename().string(), type, std::chrono::duration_cast<years>(creationTime), fileSize};
}

// Grabs entries from the working directory
int getPathEntries(const std::string *path, std::vector<File> *entries) {
    auto dir = std::filesystem::directory_iterator(path->c_str());
    for (auto &e : dir) {
        auto name = e.path().filename();
        unsigned int name_length = name.string().length();
        if (longestLength < name_length) {
            longestLength += name_length;
        }

        File file = createFileStruct(&e);
        if (file.type == DIRECTORY && entries->size() >= 2) {
            entries->insert(entries->begin() + 2, file);
        } else {
            entries->push_back(file);
        }
    }

    return EXIT_SUCCESS;
}

void printEntries(const std::vector<File> *entries) {
    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
    const std::array<WORD, 11> colors = {2, 3, 4, 5, 6, 9, 10, 11, 12, 13, 15};
    uint8_t color_index = 0;
    const uint8_t margin = 2;
    const int nameGap = (int) longestLength + margin;

    SetConsoleTextAttribute(console, 14);
    std::cout << std::setw(nameGap) << "Name" <<
              std::setw(15) << "Size" <<
              std::setw(10) << "Type" <<
              std::setw(10) << "C_Time" << std::endl;

    for (auto &e : *entries) {
        SetConsoleTextAttribute(console, colors[color_index / 2]);
        std::string size = (e.type == FILE) ? std::to_string(e.fileSize / 1026) + "KB" : "";
        std::cout << std::setw(nameGap) << e.fileName <<
                  std::setw(15) << size <<
                  std::setw(10) << e.type <<
                  std::setw(10) << std::chrono::duration_cast<years>(e.creationTime).count() << std::endl;

        color_index++;
        if (color_index > std::size(colors) * 2) color_index = 0;
    }

    SetConsoleTextAttribute(console, 15);
}

int main() {
    auto path = std::filesystem::current_path().string();

    std::vector<File> entries;
    getPathEntries(&path, &entries);
    printEntries(&entries);

    return 0;
}
