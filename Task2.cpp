// 2.	Реализовать type list
// Реализуйте класс или структуры с именем TypeList, представляющую собой упорядоченную коллекцию типов. Реализуйте следующие методы работы с TypeList (в виде шаблонизированных структур или constexpr функций):
// 	Получение элемента списка по его индексу (попытка обращения к элементу, которого не существует, должна приводить к ошибке компиляции);
// 	Получение размера списка;
// 	Проверка наличия типа в списке (constexpr bool);
// 	Получение индекса типа в списке;
// 	Добавление типа в конец списка;
// 	Добавление типа в начало списка.
// Все детали реализации следует скрыть в отдельном пространстве имен. При написании следует использовать variadic templates.
// Для проверки работоспособности реализованных методов напишите тестовый код, в которым результаты применения методов будут проверятся при помощи static_assert и std::is_same.

// Это задание на метапрограммирование — программирование на типах на этапе компиляции.


#include <iostream>
#include <type_traits>

// Оборачивает весь код в пространство имён type_list, чтобы не конфликтовать с другими частями программы
namespace type_list {
    
    // ==================== Основной шаблон TypeList ====================
    // Объявляет шаблон с переменным количеством параметров. ... — это пакет параметров
    template<typename... Types>
    struct TypeList {
        // Вычисляет количество типов в пакете на этапе компиляции
        static constexpr size_t size = sizeof...(Types);
    };
    
    // ==================== Получение элемента по индексу (объявление) ====================
    template<size_t Index, typename TypeList> // Два параметра: числовой индекс и список типов
    struct Get; // Объявляет шаблонную структуру Get (без определения)
    
    // БАЗОВЫЙ СЛУЧАЙ - индекс 0 (Это базовый случай рекурсии для операции получения типа по индексу)
    // Что делает: Если индекс равен 0, возвращаем первый тип.
    template<typename First, typename... Rest> // Параметры: первый тип и остальные
    struct Get<0, TypeList<First, Rest...>> {
        // using — это создание псевдонима типа (алиаса)
        using type = First;
    };
    
    // РЕКУРСИВНЫЙ СЛУЧАЙ
    template<size_t Index, typename First, typename... Rest>
    struct Get<Index, TypeList<First, Rest...>> {
        static_assert(Index < sizeof...(Rest) + 1, "Index out of range"); // Проверяет, что индекс не выходит за границы списка
        using type = typename Get<Index - 1, TypeList<Rest...>>::type; // Рекурсивно ищем в хвосте списка с индексом Index-1
    };
    
    // ==================== Получение размера списка ====================
    template<typename TypeList> // Шаблон с одним параметром — список типов
    struct Size;
    
    template<typename... Types> // Пакет параметров
    struct Size<TypeList<Types...>> {
        static constexpr size_t value = sizeof...(Types); // Вычисляет количество типов
    };
    
    // ==================== Проверка наличия типа в списке ====================
    template<typename T, typename TypeList> // Параметры: искомый тип и список
    struct Contains;
    
    template<typename T>
    struct Contains<T, TypeList<>> { // Специализация для пустого списка
        static constexpr bool value = false;
    };
    
    template<typename T, typename First, typename... Rest> // Параметры: искомый тип, первый тип, остальные
    struct Contains<T, TypeList<First, Rest...>> { // Специализация для непустого списка
        static constexpr bool value = std::is_same_v<T, First> || 
                                      Contains<T, TypeList<Rest...>>::value; // Проверяем, равен ли искомый тип первому или проверяем в хвосте списка
    };
    
    // ==================== Получение индекса типа в списке ====================
    // Объявление (первичный шаблон)
    template<typename T, typename TypeList>
    struct IndexOf;
    
    // Код проверяет только первый элемент, а всё остальное перекладывает на рекурсивный вызов

    // Базовый случай — пустой список
    template<typename T>
    struct IndexOf<T, TypeList<>> {
        static constexpr int value = -1;
    };

    // Рекурсивный случай — непустой список
    template<typename T, typename First, typename... Rest> // Искомый тип, первый тип списка, остальные
    struct IndexOf<T, TypeList<First, Rest...>> { // Специализация для непустого списка
        // Вычисляем значение на этапе компиляции
        // Если искомый тип совпадает с первым → индекс 0
        // Если в хвосте не нашли → -1
        // Если нашли → индекс из хвоста + 1

        // условие ? значение_если_True : значение_если_False
        static constexpr int value = std::is_same_v<T, First> ? 
                                     0 : (IndexOf<T, TypeList<Rest...>>::value == -1 ? 
                                          -1 : IndexOf<T, TypeList<Rest...>>::value + 1);
    };
    
    // ==================== Добавление типа в конец списка ====================
    template<typename NewType, typename TypeList>
    struct PushBack;
    
    template<typename NewType, typename... Types> // Первый параметр — новый тип, второй — пакет существующих типов
    struct PushBack<NewType, TypeList<Types...>> { // Специализация, когда второй аргумент — TypeList<...>
        using type = TypeList<Types..., NewType>; // Создаём новый список: сначала все старые типы, потом новый
    };
    
    // ==================== Добавление типа в начало списка ====================
    template<typename NewType, typename TypeList> 
    struct PushFront;
    
    template<typename NewType, typename... Types> // Принимаем новый тип и все старые типы
    struct PushFront<NewType, TypeList<Types...>> { // Специализация для случая, когда второй аргумент — TypeList
        using type = TypeList<NewType, Types...>; // СОЗДАЁМ НОВЫЙ ТИП — список, где первым идёт новый тип, а дальше — все старые
    };
    
    // ==================== Вспомогательные алиасы для удобства ====================
    template<size_t Index, typename TypeList>
    using Get_t = typename Get<Index, TypeList>::type;
    
    template<typename TypeList>
    inline constexpr size_t Size_v = Size<TypeList>::value;
    
    template<typename T, typename TypeList>
    inline constexpr bool Contains_v = Contains<T, TypeList>::value;
    
    template<typename T, typename TypeList>
    inline constexpr int IndexOf_v = IndexOf<T, TypeList>::value;
    
    template<typename NewType, typename TypeList>
    using PushBack_t = typename PushBack<NewType, TypeList>::type;
    
    template<typename NewType, typename TypeList>
    using PushFront_t = typename PushFront<NewType, TypeList>::type;

// Зачем это нужно?
// Без алиасов (как было бы):
// // Каждый раз писать typename и ::type
// typename Get<0, MyList>::type x;      
// typename PushBack<float, MyList>::type newList;  
// size_t s = Size<MyList>::value;        
// bool b = Contains<int, MyList>::value;  
// С алиасами (как стало):
// Get_t<0, MyList> x;                     
// PushBack_t<float, MyList> newList;       
// size_t s = Size_v<MyList>;               
// bool b = Contains_v<int, MyList>;        

    
} // namespace type_list

// ==================== Тестовая программа ====================
using namespace type_list;

// Тестовые типы
struct TypeA { int x; };
struct TypeB { double y; };
struct TypeC { char z; };

int main() {
    using MyList = TypeList<int, double, TypeA, TypeB>;
    
    // 1. Get — получение элемента по индексу
    static_assert(std::is_same_v<Get_t<0, MyList>, int>, "Get<0> failed");
    static_assert(std::is_same_v<Get_t<3, MyList>, TypeB>, "Get<3> failed");
    
    // 2. Size — размер списка
    static_assert(Size_v<MyList> == 4, "Size failed");
    static_assert(Size_v<TypeList<>> == 0, "Size of empty list failed");
    
    // 3. Contains — проверка наличия типа
    static_assert(Contains_v<int, MyList>, "Contains<int> failed");
    static_assert(!Contains_v<TypeC, MyList>, "Contains<TypeC> failed");
    
    // 4. IndexOf — получение индекса типа
    static_assert(IndexOf_v<int, MyList> == 0, "IndexOf<int> failed");
    static_assert(IndexOf_v<TypeC, MyList> == -1, "IndexOf<TypeC> failed");
    
    // 5. PushBack — добавление в конец
    using ListAfterPushBack = PushBack_t<TypeC, MyList>;
    static_assert(Size_v<ListAfterPushBack> == 5, "PushBack size failed");
    static_assert(std::is_same_v<Get_t<4, ListAfterPushBack>, TypeC>, "PushBack element failed");
    
    // 6. PushFront — добавление в начало
    using ListAfterPushFront = PushFront_t<TypeC, MyList>;
    static_assert(Size_v<ListAfterPushFront> == 5, "PushFront size failed");
    static_assert(std::is_same_v<Get_t<0, ListAfterPushFront>, TypeC>, "PushFront element failed");
    
    std::cout << "All tests passed successfully!" << std::endl;
    
    return 0;
}