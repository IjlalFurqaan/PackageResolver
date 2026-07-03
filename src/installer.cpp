
#include "installer.h"
#include "colors.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <random>
#include <iomanip>

namespace pkgr {

// Progress-bar tuning constants.
static constexpr int kBarWidth        = 25;  // character width of the progress bar
static constexpr int kSteps           = 10;  // number of increments per package
static constexpr int kStepDelayMs     = 30;  // ms between increments
static constexpr int kFakeSizeMinKb   = 50;  // smallest simulated package size (KB)
static constexpr int kFakeSizeMaxKb   = 2048; // largest simulated package size (KB)

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
    std::uniform_int_distribution<int> size_dist(kFakeSizeMinKb, kFakeSizeMaxKb);
    int fake_size_kb = size_dist(rng);

    for (int i = 0; i <= kSteps; ++i) {
        int percent = (i * 100) / kSteps;

        std::string bar = "  [";
        int filled = (percent * kBarWidth) / 100;

        for (int j = 0; j < kBarWidth; ++j) {
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

        std::this_thread::sleep_for(std::chrono::milliseconds(kStepDelayMs));
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