// ============================== УСЛОВИЕ ЗАДАЧИ ==============================
// 1.	Система управления пользователями
// Реализуйте классы User и Group. Класс User должен содержать информацию, такую как имя пользователя, уникальный идентификатор и другие релевантные данные (на ваше усмотрение), а также содержать ссылку на группу, в которой состоит пользователь (пользователь может и не состоять в группе). Класс Group должен содержать идентификатор группы и список всех пользователей, которые в ней состоят. Между классами User и Group не должно быть циклических зависимостей!
// Создайте консольную утилиту для управления пользователями и группами пользователей, которая должна поддерживать следующие команды:
// 	createUser {userId} {username} {…дополнительная информация…} – создание нового пользователя;
// 	deleteUser {userId} – удаление пользователя;
// 	allUsers – вывод информации по всем пользователям;
// 	getUser {userId} – вывести информацию по одному пользователю;
// 	createGroup {groupId} – создать новую группу;
// 	deleteGroup {groupId} – удалить группу;
// 	allGroups – вывеси информацию по всем группам, включая всех пользователей, которые в них состоят;
// 	getGroup {groupId} – вывести информацию по одной группе, включая всех пользователей, которые в ней состоят.


#include <iostream>
#include <string>
#include <unordered_map>
#include <memory>
#include <vector>
#include <sstream>
#include <algorithm>


// Forward declaration для избежания циклической зависимости
// Зачем: Класс User содержит ссылку на Group, но определение Group будет позже. 
// Эта строчка говорит компилятору, что Класс Group существует и мы определим его позже.
// Циклическая зависимость — это когда два (или больше) объекта ссылаются друг на друга, создавая замкнутый круг.
class Group;

// ТЕОРИЯ
// std::shared_ptr и std::weak_ptr — умные указатели для автоматического управления памятью

// Обычные указатели (сырые указатели):
// User* user = new User("1", "Alice", "alice@mail.ru", 25);
// // ... используем user ...
// delete user;  // ← если забыли — утечка памяти!
// Проблемы:
// Нужно вручную вызывать delete
// Если забыли — утечка памяти
// Если вызвали delete раньше времени — висячий указатель
// Сложно понять, кто должен удалять объект


// shared_ptr — это умный указатель, который считает, сколько указателей ссылаются на один объект. Когда счётчик становится 0, объект автоматически удаляется
// weak_ptr — это умный указатель, который не увеличивает счётчик ссылок. Он просто "наблюдает" за объектом. Если объект удалится, weak_ptr узнает об этом.
// Зачем нужен weak_ptr?
// Главная задача: разрывать циклические зависимости

//  ==========================ПРОБЛЕМНАЯ СИТУАЦИЯ==========================
// class User {
//     std::shared_ptr<Group> group;  // ПЛОХО
// };

// class Group {
//     std::shared_ptr<User> user;    // ПЛОХО
// };
// // Цикл! Никогда не удалятся!

// ==========================ИСПРАВЛЕНИЕ==========================
// class User {
//     std::weak_ptr<Group> group;    // ХОРОШО — только наблюдаем
// };

// class Group {
//     std::shared_ptr<User> user;    // ХОРОШО — группа владеет пользователем
// };

// Класс User
class User {
private:
    std::string userId;
    std::string username;
    std::string email;
    int age;
    std::weak_ptr<Group> group; // Слабая ссылка на группу для избежания циклической зависимости
    // Почему std::weak_ptr? Чтобы избежать циклической зависимости:

    // User → Group (через weak_ptr)
    // Group → User (через shared_ptr)

    // Если бы User хранил shared_ptr на Group, а Group хранил shared_ptr на User,
    // то никто никогда не удалился бы (взаимные ссылки).
    // weak_ptr "не удерживает" объект в памяти.
    
public:
    // Конструктор
    User(const std::string& id, const std::string& name, const std::string& mail, int a)
        : userId(id), username(name), email(mail), age(a) {}
    
    // Геттеры
    const std::string& getId() const { return userId; }
    const std::string& getUsername() const { return username; }
    const std::string& getEmail() const { return email; }
    int getAge() const { return age; }
    
    // Сеттер для группы: Принимает shared_ptr<Group> и сохраняет его как weak_ptr. Преобразование происходит автоматически
    void setGroup(std::shared_ptr<Group> grp) {
        group = grp;
    }
    
    // Получить указатель на группу, в которой состоит пользователь, безопасным способом.
    std::shared_ptr<Group> getGroup() const {
        // Превращает weak_ptr в shared_ptr и возвращает
        return group.lock();
    }
    
    // Только объявление метода
    // Этот метод превращает объект в строку (удобное для чтения представление)
    std::string toString() const;
};

// Класс Group
// Это наследование от специального вспомогательного класса
// Эта строчка даёт классу Group возможность получить shared_ptr на самого себя из любого метода класса.

// std::enable_shared_from_this<Group> — специальный базовый класс, который даёт метод shared_from_this() (возвращает shared_ptr на самого себя). 
// Нужен, когда внутри метода группы нужно получить shared_ptr на текущий объект.
class Group : public std::enable_shared_from_this<Group> {
private:
    std::string groupId;
    std::vector<std::weak_ptr<User>> users; // Слабые ссылки для избежания циклической зависимости
    // Почему weak_ptr на пользователей? Чтобы пользователь мог удалиться, даже если группа на него ссылается.
    
public:
    // Конструктор
    Group(const std::string& id) : groupId(id) {}
    
    // Геттер
    const std::string& getGroupId() const { return groupId; }
    
    // Объявления
    // Почему shared_ptr? Группа становится совладельцем пользователя (вместе с UserManager). Пока группа существует, пользователь не удалится.
    void addUser(std::shared_ptr<User> user);
    void removeUser(const std::string& userId);
    std::vector<std::shared_ptr<User>> getUsers() const;
    std::string toString() const;
};

// Реализация методов User (после полного определения Group)
// Этот метод превращает объект пользователя в читаемую строку
std::string User::toString() const {
    std::string result = "User{id=" + userId + 
                        ", name=" + username + 
                        ", email=" + email + 
                        ", age=" + std::to_string(age);
    // Добавление информации о группе (если есть)
    if (auto grp = group.lock()) {
        // вызов метода getGroupId() у объекта, на который указывает указатель grp
        result += ", group=" + grp->getGroupId();
    }
    result += "}";
    return result;
}

// Реализация методов Group
// Этот метод добавляет пользователя в группу
void Group::addUser(std::shared_ptr<User> user) {
    // Проверяем, не добавлен ли уже пользователь
    auto it = std::find_if(users.begin(), users.end(),
        // Лямбда-функция (условие поиска)
        [&user](const std::weak_ptr<User>& wp) {
            auto u = wp.lock(); // Превращаем weak_ptr в shared_ptr (проверяем, жив ли пользователь)
            return u && u->getId() == user->getId(); // Возвращаем true, если пользователь жив И его ID совпадает с искомым
        });
    // Результат: it — итератор, указывающий на найденного пользователя (или users.end(), если не нашли).    
    // Если it == users.end() → пользователь не найден → можно добавлять
    // Если it != users.end() → пользователь уже есть → ничего не делаем
    if (it == users.end()) {
        users.push_back(user);
        user->setGroup(shared_from_this()); // Говорит пользователю, в какой группе он теперь состоит.
    }
}

// Этот метод удаляет пользователя из группы по его ID
void Group::removeUser(const std::string& userId) {
    users.erase(std::remove_if(users.begin(), users.end(),
        [&userId](const std::weak_ptr<User>& wp) {
            auto u = wp.lock(); // Пытаемся получить shared_ptr (проверяем, жив ли пользователь)
            return !u || u->getId() == userId;
        }), users.end());
}

// Этот метод возвращает список живых пользователей группы
std::vector<std::shared_ptr<User>> Group::getUsers() const {
    std::vector<std::shared_ptr<User>> result;
    for (const auto& wp : users) {
        if (auto u = wp.lock()) { // Проверка, жив ли пользователь
            result.push_back(u);
        }
    }
    return result;
}

// Этот метод превращает объект группы в читаемую строку
// Пример вывода: Group{id=admins, users=[Alice, Bob, Charlie]}
std::string Group::toString() const {
    std::string result = "Group{id=" + groupId + ", users=[";
    auto activeUsers = getUsers();
    for (size_t i = 0; i < activeUsers.size(); ++i) {
        if (i > 0) result += ", ";
        result += activeUsers[i]->getUsername();
    }
    result += "]}";
    return result;
}

// Система управления пользователями
class UserManager {
private:
    std::unordered_map<std::string, std::shared_ptr<User>> users;
    std::unordered_map<std::string, std::shared_ptr<Group>> groups;
    
public:
    // Создание пользователя
    bool createUser(const std::string& userId, const std::string& username, 
                    const std::string& email, int age) {
        if (users.find(userId) != users.end()) {
            std::cout << "Error: User with id " << userId << " already exists!" << std::endl;
            return false;
        }
        

        // std::make_shared — это "умный" способ создать объект в динамической памяти.
        // Что происходит внутри
        // Примерно так работает make_shared:
        // 1. Выделяет память для объекта User
        // 2. Выделяет память для контрольного блока (счётчик ссылок)
        // 3. Вызывает конструктор User(userId, username, email, age)
        // 4. Возвращает shared_ptr<User>, который указывает на созданный объект

        // Почему make_shared, а не new User(...)?

        // new User(...)	                                                std::make_shared<User>(...)
        // Два выделения памяти (объект + контрольный блок)	                Одно выделение памяти (объект + блок вместе)
        // Нужно вручную заворачивать в shared_ptr	                        Сразу возвращает shared_ptr
        // Медленнее	                                                    Быстрее
        // Меньше защищён от утечек при исключениях	                        Безопаснее
        users[userId] = std::make_shared<User>(userId, username, email, age);
        std::cout << "User " << userId << " created successfully!" << std::endl;
        return true;
    }
    
    // Удаление пользователя
    bool deleteUser(const std::string& userId) {
        auto it = users.find(userId);
        if (it == users.end()) {
            std::cout << "Error: User with id " << userId << " not found!" << std::endl;
            return false;
        }
        
        // Удаляем пользователя из всех групп
        // it — итератор на пару (ключ, значение) в словаре users

        // it->first — ключ (ID пользователя)
        // it->second — значение (shared_ptr<User>)
        auto user = it->second;
        if (auto group = user->getGroup()) {
            group->removeUser(userId);
        }
        
        // Удаление пользователя из словаря
        users.erase(it);
        std::cout << "User " << userId << " deleted successfully!" << std::endl;
        return true;
    }
    
    // Вывод всех пользователей
    void allUsers() const {
        if (users.empty()) {
            std::cout << "No users found." << std::endl;
            return;
        }
        
        std::cout << "=== All Users (" << users.size() << ") ===" << std::endl;
        for (const auto& pair : users) {
            std::cout << pair.second->toString() << std::endl;
        }
    }
    
    // Получение информации о пользователе
    void getUser(const std::string& userId) const {
        auto it = users.find(userId);
        if (it == users.end()) {
            std::cout << "Error: User with id " << userId << " not found!" << std::endl;
            return;
        }
        
        std::cout << it->second->toString() << std::endl;
    }
    
    // Создание группы
    bool createGroup(const std::string& groupId) {
        if (groups.find(groupId) != groups.end()) {
            std::cout << "Error: Group with id " << groupId << " already exists!" << std::endl;
            return false;
        }
        
        // Эта строка создаёт новую группу и сохраняет её в словаре groups.
        groups[groupId] = std::make_shared<Group>(groupId);
        std::cout << "Group " << groupId << " created successfully!" << std::endl;
        return true;
    }
    
    // Удаление группы
    bool deleteGroup(const std::string& groupId) {
        auto it = groups.find(groupId);
        if (it == groups.end()) {
            std::cout << "Error: Group with id " << groupId << " not found!" << std::endl;
            return false;
        }
        
        // Обновляем ссылки у пользователей
        for (auto& userPair : users) {
            if (auto group = userPair.second->getGroup()) {
                if (group->getGroupId() == groupId) {
                    userPair.second->setGroup(nullptr);
                }
            }
        }
        
        // Эта строка удаляет группу из словаря groups
        groups.erase(it);
        std::cout << "Group " << groupId << " deleted successfully!" << std::endl;
        return true;
    }
    
    // Добавление пользователя в группу
    bool addUserToGroup(const std::string& userId, const std::string& groupId) {
        auto userIt = users.find(userId);
        auto groupIt = groups.find(groupId);
        
        if (userIt == users.end()) {
            std::cout << "Error: User with id " << userId << " not found!" << std::endl;
            return false;
        }
        
        if (groupIt == groups.end()) {
            std::cout << "Error: Group with id " << groupId << " not found!" << std::endl;
            return false;
        }
        
        // Если пользователь уже в какой-то группе, удаляем его оттуда
        if (auto currentGroup = userIt->second->getGroup()) {
            currentGroup->removeUser(userId);
        }
        
        groupIt->second->addUser(userIt->second);
        std::cout << "User " << userId << " added to group " << groupId << std::endl;
        return true;
    }
    
    // Вывод всех групп
    void allGroups() const {
        if (groups.empty()) {
            std::cout << "No groups found." << std::endl;
            return;
        }
        
        std::cout << "=== All Groups (" << groups.size() << ") ===" << std::endl;
        for (const auto& pair : groups) {
            std::cout << pair.second->toString() << std::endl;
        }
    }
    
    // Получение информации о группе (её ID и список всех пользователей, которые в ней состоят)
    void getGroup(const std::string& groupId) const {
        auto it = groups.find(groupId);
        if (it == groups.end()) {
            std::cout << "Error: Group with id " << groupId << " not found!" << std::endl;
            return;
        }
        
        std::cout << it->second->toString() << std::endl;
    }
};

// Функция для разбора команд
void processCommand(const std::string& command, UserManager& manager) {
    std::istringstream iss(command); // это класс, который позволяет работать со строкой как с потоком ввода (как std::cin, но читает из строки)
    std::string cmd; 
    iss >> cmd; // читаем первое слово
    
    if (cmd == "createUser") {
        std::string userId, username, email;
        int age;
        iss >> userId >> username >> email >> age; // читает 4 следующих слова и вызывает метод
        manager.createUser(userId, username, email, age);
    }
    else if (cmd == "deleteUser") {
        std::string userId;
        iss >> userId;
        manager.deleteUser(userId);
    }
    else if (cmd == "allUsers") {
        manager.allUsers();
    }
    else if (cmd == "getUser") {
        std::string userId;
        iss >> userId;
        manager.getUser(userId);
    }
    else if (cmd == "createGroup") {
        std::string groupId;
        iss >> groupId;
        manager.createGroup(groupId);
    }
    else if (cmd == "deleteGroup") {
        std::string groupId;
        iss >> groupId;
        manager.deleteGroup(groupId);
    }
    else if (cmd == "allGroups") {
        manager.allGroups();
    }
    else if (cmd == "getGroup") {
        std::string groupId;
        iss >> groupId;
        manager.getGroup(groupId);
    }
    else if (cmd == "addUserToGroup") {
        std::string userId, groupId;
        iss >> userId >> groupId;
        manager.addUserToGroup(userId, groupId);
    }
    else if (cmd == "exit") {
        std::cout << "Goodbye!" << std::endl;
        exit(0);
    }
    else {
        std::cout << "Unknown command. Available commands: createUser, deleteUser, allUsers, getUser, "
                  << "createGroup, deleteGroup, allGroups, getGroup, addUserToGroup, exit" << std::endl;
    }
}

int main() {
    UserManager manager;
    std::string input;
    
    std::cout << "=== User Management System ===" << std::endl;
    std::cout << "Available commands:" << std::endl;
    std::cout << "  createUser {userId} {username} {email} {age}" << std::endl;
    std::cout << "  deleteUser {userId}" << std::endl;
    std::cout << "  allUsers" << std::endl;
    std::cout << "  getUser {userId}" << std::endl;
    std::cout << "  createGroup {groupId}" << std::endl;
    std::cout << "  deleteGroup {groupId}" << std::endl;
    std::cout << "  allGroups" << std::endl;
    std::cout << "  getGroup {groupId}" << std::endl;
    std::cout << "  addUserToGroup {userId} {groupId}" << std::endl;
    std::cout << "  exit" << std::endl;
    std::cout << std::endl;
    
    while (true) {
        std::cout << "> ";
        // Это чтение целой строки текста из консоли (стандартного ввода)
        // Ждёт, пока пользователь введёт строку текста и нажмёт Enter, и сохраняет эту строку в переменную input
        std::getline(std::cin, input);
        if (input.empty()) continue;
        processCommand(input, manager);
        std::cout << std::endl;
    }
    
    return 0;
}