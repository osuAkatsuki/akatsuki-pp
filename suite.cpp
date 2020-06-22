// This is a pretty weird implementation with a bunch of mixed
// C & C++, don't ask lol.. i'm just quickly writing code,
// it doesn't need to be fast.

#define OPPAI_IMPLEMENTATION
// C++
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iterator>

// C
#include <dirent.h>
#include <string.h>
#include "oppai_old.h"

enum modes { standard, taiko, ctb, mania };
enum color {
    black = 90, red = 91, green = 92,
    yellow = 93, blue = 94, magenta = 95,
    cyan = 96, white = 97, reset = 0
};

void join(const std::vector<float>& v, char c, std::string& s) {
    s.clear();
    // iter backwards
    for (std::vector<float>::const_iterator p = v.end() - 1; p != v.begin() - 1; p--) {
        s += std::to_string(*p);
        if (p != v.begin())
            s += c;
    }
}

template<class Container>
void split(const std::string& s, Container& c) {
    std::istringstream iss(s);
    while(iss.good()) {
        std::string substr;
        float val = 0.0f;
        std::getline(iss, substr, ',');
        c.push_back(std::stof(substr));
    }
}

int main() {
    DIR *d;
    struct dirent *dir;

    // configuration
    const int conf_mode = modes::standard;
    const bool conf_relax = true;
    std::vector<float> conf_accuracy = {
        // Calculate each mod combination
        // at varying accuracies.
        85.0f, 90.0f, 95.0f,
        98.0f, 99.0f, 100.0f
    };

    std::vector<int> conf_mods = {
        // NM, HD
        (conf_relax ? MODS_RX : MODS_NOMOD),
        (conf_relax ? MODS_RX : MODS_NOMOD) | MODS_HD,

        // HR, HDHR
        (conf_relax ? MODS_RX : MODS_NOMOD) | MODS_HR,
        (conf_relax ? MODS_RX : MODS_NOMOD) | MODS_HD | MODS_HR,

        // DT, HDDT
        (conf_relax ? MODS_RX : MODS_NOMOD) | MODS_DT,
        (conf_relax ? MODS_RX : MODS_NOMOD) | MODS_HD | MODS_DT,

        // DTHR, HDDTHR (3mod)
        (conf_relax ? MODS_RX : MODS_NOMOD) | MODS_DT | MODS_HR,
        (conf_relax ? MODS_RX : MODS_NOMOD) | MODS_HD | MODS_DT | MODS_HR,

        // EZ, EZHD, EZDT, EZHDDT
        (conf_relax ? MODS_RX : MODS_NOMOD) | MODS_EZ,
        (conf_relax ? MODS_RX : MODS_NOMOD) | MODS_EZ | MODS_HD,
        (conf_relax ? MODS_RX : MODS_NOMOD) | MODS_EZ | MODS_DT,
        (conf_relax ? MODS_RX : MODS_NOMOD) | MODS_EZ | MODS_HD | MODS_DT,

        // FL, HDFL
        (conf_relax ? MODS_RX : MODS_NOMOD) | MODS_FL,
        (conf_relax ? MODS_RX : MODS_NOMOD) | MODS_HD | MODS_FL,
    };

    // get filenames
    std::vector<char*> beatmaps;
    if (d = opendir("/mnt/d/Development/Akatsuki/akatsuki-pp/maps"), d) {
        while ((dir = readdir(d)) != NULL) {
            if (dir->d_type != 8)
                continue;

            char* mapName = (char*)malloc(256 * sizeof(char));
            memset(mapName, 0, 256);

            strcpy(mapName, "/mnt/d/Development/Akatsuki/akatsuki-pp/maps/");
            strcat(mapName, dir->d_name);
            beatmaps.push_back(mapName);
        }
    } else return (std::cout << "Invalid directory." << std::endl, 1);
    closedir(d);

    // For storing the values
    bool compare;
    std::vector<float> pp_values;
    std::ifstream prev_file("/mnt/d/Development/Akatsuki/akatsuki-pp/prev.db");
    if (compare = prev_file.good(), compare) {
        std::string fromfile;
        std::getline(prev_file, fromfile);
        if (fromfile.empty())
            return (std::cout << "Error: Empty prev.db" << std::endl, 1);

        split(fromfile, pp_values);
    }

    if (!pp_values.empty() && pp_values.size() != conf_mods.size() * conf_accuracy.size() * beatmaps.size()) {
        printf("New format detected, removing old db save.\n"); // TODO: move to /logs
        pp_values.clear();
        compare = false;
    }

    auto accuracy_colour = [&](float& acc) {
        return (acc == 95.0f || acc == 98.0f || acc == 99.0f || acc == 100.0f)
            ? color::magenta
            : acc > 95.0f
                ? color::green
                : color::red;
    };

    auto print_col = [&](int&& col, int&& set = color::reset) {
        printf("\x1b[%d;%dm", set, col);
    };

    for (auto map : beatmaps) {
        print_col(color::cyan);
        std::cout << map << std::endl;
        print_col(color::reset);

        // I can't seem to find a way to reuse this while
        // properly clearing memory.. maybe max_combo not being set to 0?
        // either way, not really interesting in looking for it, don't care
        // about speed right now (hence this being pretty high level anyways).
        ezpp_t ez = ezpp_new();
        ezpp_set_mode_override(ez, conf_mode);

        auto mods_string = [&](int& mods) {
            std::string ret;
            if (!mods) return  (ret += "NM", ret);
            if (mods & MODS_NF) ret += "NF";
            if (mods & MODS_EZ) ret += "EZ";
            if (mods & MODS_TD) ret += "TD";
            if (mods & MODS_HD) ret += "HD";
            if (mods & MODS_HR) ret += "HR";
            if (mods & MODS_SD) ret += "SD";
            if (mods & MODS_DT) ret += "DT";
            if (mods & MODS_RX) ret += "RX";
            if (mods & MODS_HT) ret += "HT";
            if (mods & MODS_NC) ret += "NC";
            if (mods & MODS_FL) ret += "FL";
            if (mods & MODS_AT) ret += "AT";
            if (mods & MODS_SO) ret += "SO";
            if (mods & MODS_AP) ret += "AP";
            if (mods & MODS_PF) ret += "PF";
            return ret;
        };

        printf("               "); // lol
        for (auto& acc : conf_accuracy) { // nasty
            printf("| ");
            print_col(accuracy_colour(acc), 0);
            printf("%8.2f%%\x1b[0m  ", acc);
        }
        printf("\n");

        // For each mod combination, iterate through each accuracy value
        // and calculate len(mod_combos) * len(acc_combos) pp values.
        for (auto& mods : conf_mods) {
            printf("%14s", mods_string(mods).c_str());
            ezpp_set_mods(ez, mods);
            for (auto& acc : conf_accuracy) {
                ezpp_set_accuracy_percent(ez, acc);
                ezpp(ez, map);

                if (compare) { // read from file & compare
                    float diff = ezpp_pp(ez) - pp_values.back();
                    pp_values.pop_back();
                    std::cout << " | ";

                    if (diff > 0) print_col(color::green);
                    else if (diff < 0) print_col(color::red);
                    else print_col(color::yellow);

                    printf("%8.2fpp", diff);
                    print_col(color::reset);
                } else { // save to file
                    float pp = ezpp_pp(ez);
                    printf(" | %8.2fpp", pp);
                    pp_values.push_back(pp);
                }
            }
            std::cout << std::endl;
        }
        std::cout << std::endl;

        if (!compare) {
            // save as csv
            std::ofstream save_file;
            std::string tofile;
            save_file.open("/mnt/d/Development/Akatsuki/akatsuki-pp/prev.db");

            join(pp_values, ',', tofile);
            save_file << tofile;
            save_file.close();
        }
        ezpp_free(ez);
    }
    return 0;
}
