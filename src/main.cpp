#include <iostream>
#include <fstream>
#include <iomanip>
#include "InputFile.h"
#include "Logger.h"
#include "tokenizer.h"
#include "binary_generator.h"
#include "arguments.h"

int main(int argc, char** argv)
{
    auto args = parseArgs(argc, argv);
    Logger::setLoggerVerbosity(args.verbosity);

    std::string filePath;
    std::string fileContent;
    {
        InputFile file;
        file.open(args.inputFilePath);
        filePath = args.inputFilePath;
        fileContent = file.getContent();
    }

    // Call the preprocessor
    Tokenizer::preprocessFile(&fileContent);

    Tokenizer::tokenList_t tokenList;
    Tokenizer::labelMap_t labelMap;
    Tokenizer::tokenize(fileContent, filePath, &tokenList, &labelMap);
    Logger::dbg << "Found " << tokenList.size() << " tokens and " << labelMap.size() << " labels" << Logger::End;

    ByteList output = generateBinary(tokenList, labelMap);
    Logger::log << "Assembled to " << output.size() << " bytes" << Logger::End;

    Logger::dbg << "Writing output" << Logger::End;
    if (args.outputFilePath.compare("-") == 0)
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
    else
    {
        std::ofstream outputFile{args.outputFilePath, std::ios_base::binary};
        outputFile.write((const char*)output.data(), output.size());
        if (outputFile.fail())
        {
            Logger::fatal << "Failed to write to file: \"" << args.outputFilePath << '"' << Logger::End;
        }
        outputFile.close();

        Logger::log << "Wrote output to file \"" << args.outputFilePath << '"' << Logger::End;
    }

    return 0;
}

