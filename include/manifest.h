#pragma once

#include "package.h"
#include "json_parser.h"
#include <string>
#include <memory>

namespace pkgr {

class ManifestParser {
public:

    static Package parse(const std::string& file_path);

    static Package from_json(const JsonObject& obj);

private:

    static std::vector<Dependency> parse_dependencies(const JsonObject& deps_obj);
};

} 