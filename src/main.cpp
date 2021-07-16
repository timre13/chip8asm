#include <iostream>
#include "InputFile.h"
#include "Logger.h"
#include "tokenizer.h"

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

    return 0;
}

