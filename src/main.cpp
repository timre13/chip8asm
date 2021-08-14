#include <iostream>
#include <fstream>
#include <iomanip>
#include "InputFile.h"
#include "Logger.h"
#include "parser.h"
#include "binary_generator.h"
#include "arguments.h"

/*
 * Writes the output to the output file.
 *
 * Throws on error.
 */
static void writeOutput(const ByteList& output, const std::string& outputFilePath, bool shouldOutputHexdump)
{
    Logger::dbg << "Writing output" << Logger::End;
    if (outputFilePath.compare("-") == 0) // stdout
    {
        if (shouldOutputHexdump) // Hexdump
        {
            std::cout << std::hex << std::setfill('0');
            for (size_t i{}; i < output.size(); ++i)
            {
                if (i != 0 && i % 16 == 0)
                    std::cout << '\n';
                std::cout << std::setw(2) << +output[i] << ' ';
            }
            std::cout << std::dec << '\n';
        }
        else // Raw bytes
        {
            // Print the raw bytes and hope they won't be messed up
            for (size_t i{}; i < output.size(); ++i)
            {
                std::cout << output[i];
            }
        }
    }
    else // File
    {
        std::ofstream outputFile{outputFilePath, std::ios_base::out |
            (shouldOutputHexdump ? std::ios_base::out : std::ios_base::binary)};
        if (shouldOutputHexdump) // Hexdump
        {
            std::stringstream ss;
            {
                ss << std::hex << std::setfill('0');
                for (size_t i{}; i < output.size(); ++i)
                {
                    if (i != 0 && i % 16 == 0)
                        ss << '\n';
                    ss << std::setw(2) << +output[i] << ' ';
                }
                ss << '\n';
            }
            outputFile.write(ss.str().data(), ss.str().size());
        }
        else // Raw bytes
        {
            outputFile.write((const char*)output.data(), output.size());
        }

        if (outputFile.fail())
        {
            outputFile.close();
            throw std::runtime_error{"Failed to write to file: \"" + outputFilePath + '"'};
        }
        outputFile.close();

        Logger::log << "Wrote output to file \"" << outputFilePath << '"' << Logger::End;
    }
}

int main(int argc, char** argv)
{
    auto args = parseArgs(argc, argv);
    Logger::setLoggerVerbosity(args.verbosity);

    // ----- Read the input file -----
    std::string fileContent;
    try
    {
        InputFile file;
        file.open(args.inputFilePath);
        fileContent = file.getContent();
    }
    catch (std::exception& e) { Logger::fatal << e.what() << Logger::End; }

    // ----- Call the preprocessor -----
    try
    {
        fileContent = Parser::preprocessFile(fileContent, args.inputFilePath);
    }
    catch (std::exception& e) { Logger::fatal << e.what() << Logger::End; }

    // ----- Parse the file -----
    Parser::tokenList_t tokenList;
    Parser::labelMap_t labelMap;
    try
    {
        Parser::parseTokens(fileContent, args.inputFilePath, &tokenList, &labelMap);
    }
    catch (std::exception& e) { Logger::fatal << e.what() << Logger::End; }
    Logger::dbg << "Found " << tokenList.size() << " tokens and " << labelMap.size() << " labels" << Logger::End;

    // ----- Generate the output -----
    ByteList output;
    try
    {
        output = generateBinary(tokenList, labelMap);
    }
    catch (std::exception& e) { Logger::fatal << e.what() << Logger::End; }
    Logger::log << "Assembled to " << output.size() << " bytes" << Logger::End;

    // ----- Write to the output file -----
    try
    {
        writeOutput(output, args.outputFilePath, args.shouldOutputHexdump);
    }
    catch (std::exception& e) { Logger::fatal << e.what() << Logger::End; }

    return 0;
}

