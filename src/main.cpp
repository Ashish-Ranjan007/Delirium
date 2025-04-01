/*
 * Delirium Programming Language - Main entry point and file handling
 *
 * This file contains the main program logic for the Delirium programming language.
 *
 */

#include <iostream>
#include <vector>

#include "delirium.hpp"

/*
 * Main program entry point.
 *
 * @param argc Argument count
 * @param argv Argument vector
 * @return Appropriate exit code based on program execution
 */
int main(int argc, char** argv)
{
    try {
        std::vector<std::string> args(argv, argv + argc);

        if (args.size() != 2) {
            std::cerr << "Usage: " << args.front() << " [script.dlm]\n";
            return Delirium::ExitCodes::USAGE_ERROR;
        }

        Delirium::runFile(args[1]);
        return Delirium::ExitCodes::SUCCESS;
    } catch (std::exception const& e) {
        std::cerr << "Fatal error: " << e.what() << '\n';
        return Delirium::ExitCodes::RUNTIME_ERROR;
    }
}