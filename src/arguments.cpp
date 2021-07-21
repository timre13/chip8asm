#include "arguments.h"
#include "Logger.h"
#include "version.h"

static void printUsageAndExit(char* progName, int status=1)
{
    auto& stream = status ? std::cerr : std::cout;
    stream
        << "Usage: " << progName << " [OPTION...] [FILE]"
        << "\n       -h                  print help message"
        << "\n       -v                  print version and exit"
        << "\n       -o [FILE]           write output to specified file"
        << "\n       -                   output to stdout in hexadecimal"
        << "\n       -q                  be quiet (default verbosity)"
        << "\n       -V                  be verbose"
        << "\n       -d                  print debug messages"
        << '\n';
    exit(status);
}

Options parseArgs(int argc, char** argv)
{
    Options output;

    for (int i{1}; i < argc; ++i)
    {
        std::string arg = argv[i];
        if (arg.empty())
            continue;

        if (arg[0] == '-') // Switch
        {
            if (arg.compare("-h") == 0)
            {
                printUsageAndExit(*argv, 0);
            }
            else if (arg.compare("-v") == 0)
            {
                std::cout << "chip8asm version " CHIP8ASM_VERSION "\n";
                exit(0);
            }
            else if (arg.compare("-o") == 0)
            {
                if (argc-1 < i+1) printUsageAndExit(*argv);
                output.outputFilePath = argv[++i];
            }
            else if (arg.compare("-") == 0)
            {
                output.outputFilePath = "-"; // stdout
            }
            else if (arg.compare("-q") == 0)
            {
                output.verbosity = Logger::LoggerVerbosity::Quiet;
            }
            else if (arg.compare("-V") == 0)
            {
                output.verbosity = Logger::LoggerVerbosity::Verbose;
            }
            else if (arg.compare("-d") == 0)
            {
                output.verbosity = Logger::LoggerVerbosity::Debug;
            }
            else
            {
                Logger::err << "Invalid argument: \"" << arg << '"' << Logger::End;
                printUsageAndExit(*argv);
            }
        }
        else // Input file
        {
            if (!output.inputFilePath.empty())
            {
                // We've already found an input file
                Logger::err << "Multiple input files specified" << Logger::End;
                printUsageAndExit(*argv);
            }
            else
            {
                output.inputFilePath = arg;
            }
        }
    }

    if (output.inputFilePath.empty())
    {
        Logger::err << "No input file specified" << Logger::End;
        printUsageAndExit(*argv);
    }

    return output;
}

