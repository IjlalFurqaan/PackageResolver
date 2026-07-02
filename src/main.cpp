
#include "cli.h"

int main(int argc, char* argv[]) {
    pkgr::CLI cli(argc, argv);
    return cli.run();
}