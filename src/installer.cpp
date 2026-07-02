
#include "installer.h"
#include "colors.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <random>
#include <iomanip>

namespace pkgr {

void Installer::install(const std::vector<ResolvedPackage>& packages) {
    using namespace color;

    std::cout << "\n" << BOLD_CYAN << "  Installing " << packages.size() 
              << " packages..." << RESET << "\n\n";

    int index = 1;
    int total = static_cast<int>(packages.size());

    for (const auto& pkg : packages) {
        simulate_download(pkg, index, total);
        ++index;
    }

    std::cout << "\n" << BOLD_GREEN 
              << "  ✅ Successfully installed " << total << " packages" 
              << RESET << "\n\n";
}

void Installer::simulate_download(const ResolvedPackage& pkg, int index, int total) {
    using namespace color;

    std::string label = pkg.name + "@" + pkg.version.to_string();

    std::mt19937 rng(std::hash<std::string>{}(label));
    std::uniform_int_distribution<int> size_dist(50, 2048);
    int fake_size_kb = size_dist(rng);

    int steps = 10;
    for (int i = 0; i <= steps; ++i) {
        int percent = (i * 100) / steps;

        std::string bar = "  [";
        int bar_width = 25;
        int filled = (percent * bar_width) / 100;

        for (int j = 0; j < bar_width; ++j) {
            if (j < filled) {
                bar += "█";
            } else if (j == filled) {
                bar += "▓";
            } else {
                bar += "░";
            }
        }
        bar += "] ";

        std::cout << "\r" << GRAY << "  (" << index << "/" << total << ") " 
                  << RESET << BOLD_WHITE << label << RESET
                  << " " << GREEN << bar << percent << "%" << RESET
                  << " " << DIM << format_size(fake_size_kb * 1024) << RESET
                  << std::flush;

        std::this_thread::sleep_for(std::chrono::milliseconds(30));
    }

    std::cout << "\r" << GREEN << "  ✓ " << RESET 
              << "(" << index << "/" << total << ") "
              << BOLD_WHITE << label << RESET 
              << DIM << "  " << format_size(fake_size_kb * 1024) << RESET
              << "                              " << "\n";
}

std::string Installer::format_size(int bytes) {
    if (bytes < 1024) {
        return std::to_string(bytes) + " B";
    } else if (bytes < 1024 * 1024) {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(1) << (bytes / 1024.0) << " KB";
        return oss.str();
    } else {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(1) << (bytes / (1024.0 * 1024.0)) << " MB";
        return oss.str();
    }
}

} 