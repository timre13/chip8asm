#include <iostream>
#include <fstream>
#include "InputFile.h"
#include "Logger.h"
#include "tokenizer.h"
#include "binary_generator.h"

int main(int argc, char** argv)
{
    //std::cout << "Copyright (c) 2021 Imre Törteli\n\n";
    Logger::setLoggerVerbosity(Logger::LoggerVerbosity::Debug);

    if (argc < 2)
        Logger::fatal << "Usage: " << argv[0] << " <file>" << Logger::End;

    InputFile file;
    file.open(argv[1]);

    auto tokenList = Tokenizer::tokenize(file.getContent(), file.getFilePath());
    Logger::dbg << "Found " << tokenList.size() << " tokens" << Logger::End;

    ByteList output = generateBinary(tokenList);
    Logger::dbg << "Assembled to " << output.size() << " bytes" << Logger::End;

    Logger::dbg << "Writing output" << Logger::End;
    std::ofstream outputFile{"output.ch8", std::ios_base::binary};
    outputFile.write((const char*)output.data(), output.size());
    outputFile.close();

    Logger::log << "Wrote output to file \"output.ch8\"" << Logger::End;

    return 0;
}

