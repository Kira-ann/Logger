#pragma once // Чтобы дважды не добавлялся файл в проект при сборке
#include <fstream>
#include <string>
#include "level.h"

class Logger{
    private:
        Level currentLevel; // Заданный уровень важности
        std::ofstream file; 

    public:
        void setCurrentLevel(const std::string& newLevel); 
        Logger(Level newLevel, const std::string& name); //конструктор с заданными полями
        ~Logger();
        void logMessage(Level levelMessage, const std::string& text); //запись в файл
        bool isOpen() const { return file.is_open(); }
};