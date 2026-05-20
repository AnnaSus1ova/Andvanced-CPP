#ifndef LOG_H
#define LOG_H

#include <string>
#include <vector>
#include <iostream>
#include <iomanip>
#include <ctime>
#include <chrono>
#include <sstream>

// Уровни важности событий
enum LogLevel {
    LOG_NORMAL = 0,   // Нормальное событие
    LOG_WARNING = 1,  // Предупреждение
    LOG_ERROR = 2     // Ошибка
};

// Структура для хранения одного события
struct LogEntry {
    std::chrono::system_clock::time_point timestamp;
    LogLevel level;
    std::string message;
    
    LogEntry(LogLevel lvl, const std::string& msg) 
        : timestamp(std::chrono::system_clock::now()), level(lvl), message(msg) {}
    
    // Получить строковое представление времени
    std::string timeToString() const {
        auto time_t = std::chrono::system_clock::to_time_t(timestamp);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            timestamp.time_since_epoch()) % 1000;
        
        std::tm* tm = std::localtime(&time_t);
        std::ostringstream oss;
        oss << std::put_time(tm, "%Y-%m-%d %H:%M:%S") 
            << '.' << std::setfill('0') << std::setw(3) << ms.count();
        return oss.str();
    }
    
    // Получить строковое представление уровня важности
    std::string levelToString() const {
        switch(level) {
            case LOG_NORMAL: return "NORMAL";
            case LOG_WARNING: return "WARNING";
            case LOG_ERROR: return "ERROR";
            default: return "UNKNOWN";
        }
    }
};

// Класс Log - Singleton
class Log {
private:
    static Log* instance;  // Единственный экземпляр
    std::vector<LogEntry> entries;  // Хранилище событий
    static const size_t MAX_PRINT = 10; // Количество последних событий для вывода
    
    // Приватные конструктор и деструктор (Singleton)
    Log() {}
    ~Log() {}
    
    // Запрещаем копирование
    Log(const Log&) = delete;
    Log& operator=(const Log&) = delete;
    
public:
    // Получение единственного экземпляра
    static Log* Instance() {
        if (instance == nullptr) {
            instance = new Log();
        }
        return instance;
    }
    
    // Добавление сообщения в лог
    void message(LogLevel level, const std::string& msg) {
        entries.emplace_back(level, msg);
    }
    
    // Вывод последних 10 событий (или меньше, если их меньше 10)
    void print() const {
        if (entries.empty()) {
            std::cout << "\n[LOG] No events to display" << std::endl;
            return;
        }
        
        // Определяем, с какого индекса начинать вывод
        size_t start = (entries.size() > MAX_PRINT) ? entries.size() - MAX_PRINT : 0;
        size_t numToPrint = entries.size() - start;
        
        std::cout << "\n" << std::string(60, '=') << std::endl;
        std::cout << "LAST " << numToPrint << " LOG EVENT(S):" << std::endl;
        std::cout << std::string(60, '=') << std::endl;
        
        for (size_t i = start; i < entries.size(); ++i) {
            const auto& entry = entries[i];
            std::cout << "[" << entry.timeToString() << "] "
                      << "[" << entry.levelToString() << "] "
                      << entry.message << std::endl;
        }
        std::cout << std::string(60, '=') << "\n" << std::endl;
    }
};

// Инициализация статического члена
Log* Log::instance = nullptr;

#endif // LOG_H