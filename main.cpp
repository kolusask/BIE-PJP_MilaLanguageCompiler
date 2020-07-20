#include "include/Exception.h"
#include "include/Parser.h"
#include "include/TextPosition.h"

#include <iostream>
#include <fstream>


int main() {
    const char* fileName = "/home/askar/FIT/PJP/Mila/semestralwork/samples/test.mila";
    std::ifstream file;
    file.open(fileName);
    Parser parser(file);

    if (file.fail()) {
        std::cout << "File not open" << std::endl;
        return 1;
    } else {
        try {
            parser.parse();
        } catch (Exception& e) {
            auto pos = e.position();
            std::cerr << "LINE " << pos.line << "; COLUMN " << pos.column << ':' << std::endl;
            file.seekg(0, std::ios::beg);
            std::string line;
            for (int i = 0; i < pos.line; i++)
                std::getline(file, line);
            std::cerr << line << std::endl;

            for (int i = 0; i < pos.column - 1; i++)
                std::cerr << '~';
            std::cerr << '^';
            for (int i = pos.column; i < line.length(); i++)
                std::cerr << '~';
            std::cerr << std::endl;

            std::cerr << "ERROR:\t" << e.message() << std::endl;
            return 2;
        }
    }
    std::cout << parser.get_source() << std::endl;
    file.close();



    return 0;
}
