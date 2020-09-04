#include <fstream>
#include <ctime>
#include "log.h"

#define FILE_NAME "log.txt"

void Log::write(const std::string &data)
{
    std::ofstream file;
    file.open(FILE_NAME, std::ios::app);
    if(!file)
    {
        std::cerr << "Couldn't open file \"" << FILE_NAME << "\"" << std::endl;
        return;
    }

    time_t rawtime;
    tm *timeinfo;
    time(&rawtime);
    timeinfo=localtime(&rawtime);

    char out_str[80];
    strftime(out_str, 80, "%d/%m/%Y %X: ", timeinfo);
    file << out_str << data << std::endl;
    std::cout << data << std::endl;

    file.close();
}
