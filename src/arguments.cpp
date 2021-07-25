#include "arguments.h"
#include "Logger.h"
#include "version.h"

#define LICENSE_STR "\
BSD 2-Clause License\n\
\n\
Copyright (c) 2021, timre13\n\
All rights reserved.\n\
\n\
Redistribution and use in source and binary forms, with or without\n\
modification, are permitted provided that the following conditions are met:\n\
\n\
1. Redistributions of source code must retain the above copyright notice, this\n\
   list of conditions and the following disclaimer.\n\
\n\
2. Redistributions in binary form must reproduce the above copyright notice,\n\
   this list of conditions and the following disclaimer in the documentation\n\
   and/or other materials provided with the distribution.\n\
\n\
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS \"AS IS\"\n\
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE\n\
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE\n\
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE\n\
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL\n\
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR\n\
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER\n\
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,\n\
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE\n\
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.\n\
"

static void printUsageAndExit(char* progName, int status=1)
{
    auto& stream = status ? std::cerr : std::cout;
    stream
        << "Usage: " << progName << " [OPTION...] [FILE]"
        << "\n       -h                  print help message"
        << "\n       -v                  print version and exit"
        << "\n       -l                  print license and exit"
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
                std::cout << "chip8asm version " CHIP8ASM_VERSION "\n\nUse the -l option to see the license.\n";
                exit(0);
            }
            else if (arg.compare("-l") == 0)
            {
                std::cout << LICENSE_STR;
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

