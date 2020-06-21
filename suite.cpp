// This is a pretty weird implementation with a bunch of mixed
// C & C++, don't ask lol.. i'm just quickly writing code

#define OPPAI_IMPLEMENTATION
// C++
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

// C
#include <dirent.h>
#include <string.h>
#include "oppai.h"

enum modes { standard, taiko, ctb, mania };

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

        // EZ, EZHD
        (conf_relax ? MODS_RX : MODS_NOMOD) | MODS_EZ,
        (conf_relax ? MODS_RX : MODS_NOMOD) | MODS_EZ | MODS_HD,
    };

    // cant use string since oppai c89 xd
    char* mapName = (char*)malloc(256 * sizeof(char));
    memset(mapName, 0, 256);

    if (d = opendir("/mnt/d/Development/Akatsuki/akatsuki-pp/maps"), d) {
        while ((dir = readdir(d)) != NULL) {
            if (dir->d_type != 8)
                continue;

            std::cout << "\x1b[0;96m" << dir->d_name << "\x1b[0m" << std::endl;
            strcpy(mapName, "/mnt/d/Development/Akatsuki/akatsuki-pp/maps/");
            strcat(mapName, dir->d_name);

            // I can't seem to find a way to reuse this while
            // properly clearing memory.. maybe max_combo not being set to 0?
            // either way, not really interesting in looking for it, don't care
            // about speed right now (hence this being pretty high level anyways).
            ezpp_t ez = ezpp_new();
            ezpp_set_mode_override(ez, conf_mode);

            auto mods_string = [&](int& mods) {
                std::string ret;
                if (!mods) return (ret += "NM", ret);
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
            auto accuracy_colour = [&](float& acc) {
                std::vector<float> tillerino_accs = {
                    // highlight tillerino return accs
                    // so we can see them at a glance.
                    95.0f, 98.0f, 99.0f, 100.0f
                };

                return std::find(
                    tillerino_accs.begin(), tillerino_accs.end(), acc
                ) != tillerino_accs.end()
                    ? 95 // magenta
                    : acc > 95.0f ? 92 : 91; // green, red
            };
            for (auto& acc : conf_accuracy) { // nasty
                printf(std::string(" | \x1b[1;" + std::to_string(accuracy_colour(acc)) + "m").c_str());
                printf("%7.2f%%\x1b[0m ", acc);
            }
            printf("\n");

            // For each mod combination, iterate through each accuracy value
            // and calculate len(mod_combos) * len(acc_combos) pp values.
            for (auto& mods : conf_mods) {
                printf("%14s ", mods_string(mods).c_str());
                ezpp_set_mods(ez, mods);
                for (auto& acc : conf_accuracy) {
                    ezpp_set_accuracy_percent(ez, acc);
                    ezpp(ez, mapName);
                    printf(" | %7.2fpp", ezpp_pp(ez));
                }
                std::cout << std::endl;
            }

            memset(mapName, 0, 256);
            ezpp_free(ez);
        }
        closedir(d);
    }
    return 0;
}
