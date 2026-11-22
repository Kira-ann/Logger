#include <string>
#include <iostream>
#include <optional>
#include "LogLevel.h"
#include "../lib/Logger/Logger.h"


int main(int argc, char *argv[]) { // тип всех аргументов - строки [filename level mode]
    if (argc != 4) return 1;
    auto level = toLevel(argv[2]);
    if (level){
        bool append;
        if (std::string(argv[3]) == "truncate") {append = 0;} 
        else if (std::string(argv[3]) == "append") {append = 1;} 
        else {return 1;}

        Logger now(level.value(), argv[1], append);
        if (!now.is_valid()){
            std::cerr << "Failed to open " << argv[1] << std::endl;
            return 1;}
        std::string current_str;
        while(std::getline(std::cin, current_str)){
            size_t pos = current_str.find(" ");
            auto parsed = toLevel(current_str.substr(0, pos));
            if (pos == std::string::npos || !parsed){ // для случаев: "text" || "text text"
                now.log(current_str, parsed);
            } else if (parsed){
                now.log(current_str.substr(pos + 1), parsed);
            }
        }
    } else {return 1;}
    return 0;
}

// Способ задачи уровня логирования:
// 1) Явное указание уровня в начале строки через пробел с текстом
//    пример: "INFO file is open"
// 2) Неявное указание уровня, присутствует только сообщение - в таком случае используется дефолтный уровень
//    пример: "File is open"

//Запуск в командной строке : 
// .\main.exe [полное имя файла или путь] [уровень логирования] [режим перезаписи или добавления логов]
// пример для добавления логов в конец файла:           .\main.exe blay.txt ERROR append
// пример для добавления логов в перезаписанный файл:   .\main.exe blay.txt ERROR truncate
