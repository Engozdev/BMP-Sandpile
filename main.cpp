#include <cstring>
#include <iostream>
#include <unistd.h> // for checking files
#include "Sandpile.h"

bool CheckFileExistance(const char* path) {
    return access(path, F_OK) != -1;
}

bool CheckNumber(char* num) {
    while (*num != '\0') {
        if (!isdigit(*num))
            return false;
        ++num;
    }
    return true;
}

Args ParseArgs(int argc, char* argv[]) {
    Args res;
    for (int i = 1; i < argc; ++i) {
        char* s = argv[i];
        if (strcmp(s, "-i") == 0) {
            if (i == argc - 1) {
                std::cerr << "No input file";
                exit(EXIT_FAILURE);
            }
            if (CheckFileExistance(argv[i + 1])) {
                res.input = argv[++i];
            } else {
                std::cerr << "No access to file";
                exit(EXIT_FAILURE);
            }
        } else if (strncmp(s, "--input=", 8) == 0) {
            const char* val = strchr(s, '=') + 1;
            if (CheckFileExistance(val)) {
                res.input = val;
            } else {
                std::cerr << "No access to file";
                exit(EXIT_FAILURE);
            }
        } else if (strcmp(s, "-o") == 0) {
            if (i == argc - 1) {
                std::cerr << "No output file";
                exit(EXIT_FAILURE);
            }
            if (CheckFileExistance(argv[i + 1])) {
                res.output = argv[++i];
            } else {
                std::cerr << "No access to file";
                exit(EXIT_FAILURE);
            }
        } else if (strncmp(s, "--output=", 9) == 0) {
            const char* val = strchr(s, '=') + 1;
            if (CheckFileExistance(val)) {
                res.output = val;
            } else {
                std::cerr << "No access to file";
                exit(EXIT_FAILURE);
            }
        } else if (strcmp(s, "-m") == 0) {
            if (i == argc - 1) {
                std::cerr << "No maximal iteration amount";
                exit(EXIT_FAILURE);
            }
            if (CheckNumber(argv[i + 1])) {
                res.iter_amount = std::stoi(argv[++i]);
            } else {
                std::cerr << "Invalid maximal iteration amount";
                exit(EXIT_FAILURE);
            }
        } else if (strncmp(s, "--max_iter=", 11) == 0) {
            char* val = strchr(s, '=') + 1;
            if (CheckNumber(val)) {
                res.iter_amount = std::stoi(val);
            } else {
                std::cerr << "Invalid maximal iteration amount";
                exit(EXIT_FAILURE);
            }
        } else if (strcmp(s, "-f") == 0) {
            if (i == argc - 1) {
                std::cerr << "No frequency";
                exit(EXIT_FAILURE);
            }
            if (CheckNumber(argv[i + 1])) {
                res.freq = std::stoi(argv[++i]);
            } else {
                std::cerr << "Invalid frequency";
                exit(EXIT_FAILURE);
            }
        } else if (strncmp(s, "--freq=", 7) == 0) {
            char* val = strchr(s, '=') + 1;
            if (CheckNumber(val)) {
                res.freq = std::stoi(val);
            } else {
                std::cerr << "Invalid frequency amount";
                exit(EXIT_FAILURE);
            }
        }
    }
    return res;
}


int main(int argc, char* args[]) {
    Args arguments = ParseArgs(argc, args);

    SandPile pile;
    pile.Initialize(arguments);
    bool resp = pile.Topple();
    arguments.iter_amount--;
    uint64_t cnt = 1;
    bool every_time_save = arguments.freq == 0;
    while (resp && arguments.iter_amount) {
        if (every_time_save)
            pile.SaveBmp(arguments.output, cnt);
        else if (cnt % arguments.freq == 0)
            pile.SaveBmp(arguments.output, cnt);

        resp = pile.Topple();
        arguments.iter_amount--;
        cnt++;
    }
    pile.SaveBmp(arguments.output, cnt);
    return 0;
}
