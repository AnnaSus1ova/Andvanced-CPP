// 5.	Паттерн Singleton
// Используя паттерн Singleton, разработайте систему протоколирования событий в системе. 
// Система должна: 
// - поддерживать 3 уровня важности событий (нормальный, замечание, ошибка); 
// - обеспечить фиксацию события (с событием фиксируются время, важность, текстовое сообщение); 
// - выводить на печать 10 последних событий. 
// Пример использования: 
// #include “log.h” 
// void main(void) { 
//     Log *log = Log::Instance(); 
//     log->message(LOG_NORMAL, “program loaded”);
//     … 
//     log->message(LOG_ERROR, “error happens! help me!”); log->print(); 
// }


#include "log.h"

int main() {
    // Получаем экземпляр логгера (Singleton)
    Log* log = Log::Instance();
    
    // Добавляем события с разными уровнями важности
    log->message(LOG_NORMAL, "program loaded");
    log->message(LOG_NORMAL, "user logged in");
    log->message(LOG_WARNING, "low disk space");
    log->message(LOG_ERROR, "error happens");
    
    // Добавим ещё событий, чтобы показать, что выводятся только последние 10
    for (int i = 1; i <= 8; i++) {
        log->message(LOG_NORMAL, "additional event #" + std::to_string(i));
    }
    
    // Выводим последние 10 событий
    log->print();
    
    return 0;
}