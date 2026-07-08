
#include "manifest.h"
#include "exceptions.h"
#include <fstream>

namespace pkgr {

Package ManifestParser::parse(const std::string& file_path) {

    auto json = JsonParser::parse_file(file_path);

    if (!json->is_object()) {
        throw ManifestParseError(file_path, 0, "Root element must be a JSON object");
    }

    const auto& obj = static_cast<const JsonObject&>(*json);
    return from_json(obj);
}

Package ManifestParser::from_json(const JsonObject& obj) {

    if (!obj.has("name")) {
        throw JsonParseError("Missing required field: 'name'");
    }
    if (!obj.has("version")) {
        throw JsonParseError("Missing required field: 'version'");
    }

    std::string name = obj.get_string("name");
    Version version(obj.get_string("version"));

    std::string description;
    if (obj.has("description")) {
        description = obj.get_string("description");
    }

    std::string author;
    if (obj.has("author")) {
        author = obj.get_string("author");
    }

    std::vector<Dependency> dependencies;
    if (obj.has("dependencies")) {
        const auto* deps_val = obj.get("dependencies");
        if (deps_val && deps_val->is_object()) {
            dependencies = parse_dependencies(
                static_cast<const JsonObject&>(*deps_val));
        }
    }

    return Package(
        std::move(name), 
        std::move(version), 
        std::move(description), 
        std::move(dependencies)
    );
}

std::vector<Dependency> ManifestParser::parse_dependencies(const JsonObject& deps_obj) {
    std::vector<Dependency> deps;

    for (const auto& [key, value] : deps_obj.entries()) {
        if (!value->is_string()) {
            throw JsonParseError(
                "Dependency constraint must be a string for: " + key);
        }

        const auto& constraint_str = static_cast<const JsonString*>(value.get())->value();
        deps.emplace_back(key, VersionConstraint(constraint_str));
    }

    return deps;
}

} 