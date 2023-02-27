// Dependency:
// nlohmann json
// daniele77 cli
// tomohxx necessary-and-unnecessary-tiles
// https://github.com/tomohxx/necessary-and-unnecessary-tiles
// asio
// curl

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <string>
#include <unistd.h>  // access()

#include "cli/cli.h"
#include "cli/clilocalsession.h"
#include "cli/filehistorystorage.h"
#include "cli/standaloneasioscheduler.h"
#include "game.hpp"

#define VERSION "1.0"

//================================== MAIN ======================================
int main(int argc, char** argv) {
    // setting up CLI commands
    // get std::unique_ptr `rootMenu' with prompt "cjong"
    auto rootMenu = std::make_unique<cli::Menu>( "cjong" );
    Game g;
    Hand::csht.initialize(INDEX_PATH);

    // Insert(<command name>, <function>, <help text>)
    rootMenu -> Insert(
        "f",
        [&g](std::ostream& out, std::string fileName) {
            // check if file exists
            if (access(fileName.c_str(), F_OK) != -1) {
                g.initialize();
                g.read(fileName);
                g.analyze();
            }
            else {
                out << "File `" << fileName << "' does not exist!\n";
            }
        },
        "Read gamelog from file."
    );

    rootMenu -> Insert(
        "h",
        [](std::ostream& out, std::string handMPSZ) {
            Hand h;
            h.readMPSZ(handMPSZ);
            if (h.getNumTile() % 3 == 1) {
                if (h.getShanten() > 0) {
                    std::cout << "Shanten: " << h.getShanten() << std::endl;
                    std::cout << "Ukeire: " << h.getWait() << std::endl;
                }
                else {
                    std::cout << "Tenpai" << std::endl;
                    std::cout << "Ukeire: " << h.getWait() << std::endl;
                }
            }
            if (h.getNumTile() % 3 == 2) {
                std::cout << "Cut Ukeire" << std::endl;

                std::vector<std::tuple<std::string, std::string>> vtss = h.getDiscard();
                for (auto it = vtss.begin(); it != vtss.end(); it++) {
                    auto [discard, ukeire] = *it;
                    std::cout << discard << "  " << ukeire << std::endl;
                }
            }
        },
        "Read hand in MPSZ format."
    );

    rootMenu -> Insert(
        "j",
        [&g](std::ostream& out, std::string playerName) {
            g.setJicha(playerName);
        },
        "Set player view"
    );

    // setting up CLI environment
    // create a cli with the given root menu and a persistent storage
    // you must pass to FileHistoryStorage the path of the history file
    // if you don't pass the second argument, the cli will use a VolatileHistoryStorage object that keeps in memory
    // the history of all the sessions, until the cli is shut down.
    cli::Cli cli( std::move(rootMenu), std::make_unique<cli::FileHistoryStorage>(".cli") );
    // global exit action
    cli.ExitAction( [](auto& out){ out << "Goodbye and thanks for all the fish.\n"; } );
    // std exception custom handler
    cli.StdExceptionHandler(
        [](std::ostream& out, const std::string& cmd, const std::exception& e)
        {
            out << "Exception caught in cli handler: "
                << e.what()
                << " handling command: "
                << cmd
                << ".\n";
        }
    );

    cli::StandaloneAsioScheduler scheduler;
    cli::CliLocalTerminalSession localSession(cli, scheduler, std::cout, 200);
    localSession.ExitAction(
        [&scheduler](auto& out) // session exit action
        {
            out << "Closing App...\n";
            scheduler.Stop();
        }
    );

    scheduler.Run();

    return 0;
}
