#pragma once

#include "Logger.h"
#include <string>

struct Options
{
    std::string inputFilePath;
    std::string outputFilePath = "output.ch8";
    Logger::LoggerVerbosity verbosity = Logger::LoggerVerbosity::Quiet;
};

Options parseArgs(int argc, char** argv);

