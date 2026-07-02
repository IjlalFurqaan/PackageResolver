#pragma once

#include "registry.h"
#include "resolver.h"
#include "installer.h"
#include <string>
#include <vector>

namespace pkgr {

enum class Command {
    RESOLVE,    
    INSTALL,    
    GRAPH,      
    CHECK,      
    LIST,       
    INFO,       
    HELP,       
    VERSION,    
    UNKNOWN     
};

class CLI {
public:
    CLI(int argc, char* argv[]);

    int run();

private:
    std::vector<std::string> args_;
    std::string executable_path_;

    Command parse_command() const;

    int cmd_resolve(const std::string& manifest_path);
    int cmd_install(const std::string& manifest_path);
    int cmd_graph(const std::string& manifest_path);
    int cmd_check(const std::string& manifest_path);
    int cmd_list();
    int cmd_info(const std::string& package_name);
    void cmd_help() const;
    void cmd_version() const;

    Registry load_registry() const;
    std::string get_registry_path() const;
    void print_dependency_tree(const Package& pkg, const Registry& registry,
                               const std::string& prefix = "", bool is_last = true,
                               int depth = 0) const;
    void print_banner() const;
};

} 