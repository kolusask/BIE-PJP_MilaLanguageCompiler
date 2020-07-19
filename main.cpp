#include "include/Exception.h"
#include "include/Parser.h"

#include <iostream>
#include <fstream>


int main() {
    std::ifstream file;
    file.open("/home/askar/FIT/PJP/Mila/semestralwork/samples/test.mila");
    Parser parser(file);

    if (file.fail()) {
        std::cout << "File not open" << std::endl;
        return 1;
    } else {
        try {
            parser.parse();
        } catch (Exception& e) {
            std::cout << e.message() << std::endl;
            return 2;
        }
    }
    std::cout << parser.get_source() << std::endl;
    file.close();



    return 0;
}
