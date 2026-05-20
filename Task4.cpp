// ============================== УСЛОВИЕ ЗАДАЧИ ==============================
// 4.	Операторные MixIn
// 1)	Реализуете MixIn класс less_then_comparable, который при помощи CRTP «подмешивает» в целевой класс операторы сравнения (>, <=, >=, ==, !=).
// 2)	Реализуйте MixIn класс counter, который обеспечивает возможность подсчета созданных экземпляров целевого класса.
// Далее приведен пример использования созданных MixIn:
// class Number: public less_than_comparable<Number>, public counter<Number> {
// public:
//     Number(int value): m_value{value} {}

//     int value() const { return m_value; }

//     bool operator<(Number const& other) const {
//         return m_value < other.m_value;
//     }

// private:
//     int m_value;
// };

// int main()
// {
//     Number one{1};
//     Number two{2};
//     Number three{3};
//     Number four{4};
//     assert(one >= one);
//     assert(three <= four);
//     assert(two == two);
//     assert(three > two);
//     assert(one < two);
//     std::cout << "Count: " << counter<Number>::count() << std::endl;
//     return 0;
// }


#include <iostream>
#include <cassert>
#include <string>

// Number предоставляет operator<
// Number получает от less_than_comparable остальные операторы
// less_than_comparable использует operator< от Number
// Это не противоречие, а "взаимовыгодное сотрудничество":
// Number даёт минимальную реализацию (<)
// less_than_comparable даёт всё остальное

// ==================== MixIn: less_than_comparable ====================
// Mixin (примесь) — это класс, который предоставляет готовую функциональность другим классам через наследование. 
// Класс, который использует миксин, "примешивает" себе нужные возможности
template<typename Derived>
class less_than_comparable {
public:

    // *this (указатель на текущий объект) имеет тип less_than_comparable<Number>, но реальный объект — Number
    // static_cast<const Derived&>(*this) приводит его к const Number&
    // Вызывается other < derived — это уже operator< наследника
    // Возвращается результат

    // Зачем всё это?
    // Затем, чтобы в базовом классе можно было вызвать operator< производного класса.
    // Без этого приведения код бы не скомпилировался, потому что у базового класса less_than_comparable нет своего operator< — он требует его от наследника.
    bool operator>(const Derived& other) const {
        // static_cast<const Derived&>(*this) — это приведение ссылки на базовый класс к ссылке на производный класс.
        const Derived& derived = static_cast<const Derived&>(*this);
        return other < derived;
    }
    
    bool operator<=(const Derived& other) const {
        const Derived& derived = static_cast<const Derived&>(*this);
        // a <= b ⇔ НЕ (b < a)
        return !(other < derived);
    }
    
    bool operator>=(const Derived& other) const {
        const Derived& derived = static_cast<const Derived&>(*this);
        return !(derived < other);
    }
    
    bool operator==(const Derived& other) const {
        const Derived& derived = static_cast<const Derived&>(*this);
        // a == b ⇔ !(a < b) && !(b < a)
        return !(derived < other) && !(other < derived);
    }
    
    bool operator!=(const Derived& other) const {
        return !(*this == other);
    }
};

// ==================== MixIn: counter ====================
template<typename Derived>
class counter {
private:
    // static	Переменная принадлежит классу, а не объектам. Все объекты класса используют одну и ту же переменную.
    // inline (C++17)	Позволяет определить и инициализировать статическую переменную прямо в классе, без выноса в .cpp файл.
    static inline int instance_count = 0;
    
protected:
    // Конструктор по умолчанию
    counter() { ++instance_count; }
    
    // Конструктор копирования
    counter(const counter&) { ++instance_count; }

    // Конструктор перемещения
    counter(counter&&) noexcept { ++instance_count; }

    // Деструктор
    ~counter() { --instance_count; }
    
public:
    // Это статический метод, который возвращает текущее значение счётчика — то есть количество существующих объектов данного типа
    static int count() { return instance_count; }
};

// ==================== Пример использования ====================
// ТЕОРИЯ
// CRTP = Curiously Recurring Template Pattern (странно рекурсирующий шаблонный паттерн).
// Это когда базовый класс принимает себя же в качестве шаблонного параметра от наследника
// См. строчку ниже, шаблону передаётся сам Number

// Зачем это нужно?
// Базовый класс less_than_comparable<Number> знает точный тип наследника (Number) на этапе компиляции. Поэтому он может:
// вызывать методы наследника (например, operator<)
// выполнять static_cast<const Derived&>(*this) — безопасное приведение к реальному типу
class Number : public less_than_comparable<Number>, public counter<Number> {
public:
    // Конструктор
    Number(int value) : m_value{value} {}
    
    // Геттер
    int value() const { return m_value; }
    
    bool operator<(const Number& other) const {
        return m_value < other.m_value;
    }
    
private:
    int m_value;
};

int main() {
    // Создание объектов
    Number one{1};
    Number two{2};
    Number three{3};
    Number four{4};
    
    assert(one >= one);
    assert(three <= four);
    assert(two == two);
    assert(three > two);
    assert(one < two);
    
    std::cout << "Count: " << counter<Number>::count() << std::endl;
    
    return 0;
}

// Проблема, которую решает CRTP
// Например, нам нужно написать классы Number, String, Point, Date, Rational... И для каждого нужно реализовать все 6 операторов сравнения (>, <, >=, <=, ==, !=).

// Без CRTP:

// class Number {
//     int value;
// public:
//     bool operator<(const Number& other) const { return value < other.value; }
//     bool operator>(const Number& other) const { return other < *this; }
//     bool operator<=(const Number& other) const { return !(other < *this); }
//     bool operator>=(const Number& other) const { return !(*this < other); }
//     bool operator==(const Number& other) const { return !(*this < other) && !(other < *this); }
//     bool operator!=(const Number& other) const { return !(*this == other); }
// };

// class String {
//     std::string value;
// public:
//     bool operator<(const String& other) const { return value < other.value; }
//     bool operator>(const String& other) const { return other < *this; }
//     bool operator<=(const String& other) const { return !(other < *this); }
//     bool operator>=(const String& other) const { return !(*this < other); }
//     bool operator==(const String& other) const { return !(*this < other) && !(other < *this); }
//     bool operator!=(const String& other) const { return !(*this == other); }
// };

// // И так для КАЖДОГО класса...
// Много повторяющегося кода, если нужно исправить ошибку в логике >= — править во всех классах.

// С CRTP
// // Один раз написали миксин
// template<typename Derived>
// class less_than_comparable {
//     // все 5 операторов реализованы через <
// };

// // Теперь каждый класс просто наследуется
// class Number : public less_than_comparable<Number> {
//     int value;
// public:
//     bool operator<(const Number& other) const { return value < other.value; }
//     // всё! остальное приходит от миксина
// };

// Почему operator< выбран как "базовый"?
// Потому что математически: все остальные операторы можно выразить через <


// Что даёт такой подход
// Без CRTP	                                                С CRTP
// 6 операторов × N классов	                                1 оператор × N классов + 5 операторов × 1 раз
// При ошибке — править везде	                            При ошибке — править в одном месте
// Код раздувается	                                        Код компактный
// Сложно добавлять новые классы	                        Легко добавлять новые классы


// Конкретный пример пользы
// Если нужно что-то исправить, то
// Без CRTP: нужно исправлять в 50 классах.
// С CRTP: нужно исправить в одном месте less_than_comparable, и все 50 классов автоматически получают правильную версию.