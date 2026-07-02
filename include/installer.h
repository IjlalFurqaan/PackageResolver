#pragma once

#include "resolver.h"
#include <vector>
#include <string>

namespace pkgr {

class Installer {
public:

    void install(const std::vector<ResolvedPackage>& packages);

private:

    void simulate_download(const ResolvedPackage& pkg, int index, int total);

    void print_progress_bar(int percent, const std::string& label);

    static std::string format_size(int bytes);
};

} 