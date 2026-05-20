#include <iostream>
#include <string>
#include <map>
#include <unordered_map>
#include <cassert>

using Context = std::map<std::string, double>;

class ExpressionFactory;

// Паттерн Компоновщик (Базовый интерфейс выражения)
class Expression {
public:
    virtual ~Expression() = default;
    
    virtual void print() const = 0;
    
    virtual double calculate(const Context& context) const = 0;
    
    virtual void addRef() {}
    virtual void release() { delete this; }
};

// Паттерн Приспособленец (Flyweight разделяемые листья дерева)

class Constant : public Expression {
private:
    double m_value;
    int m_refCount = 0;
    bool m_isStatic = false;
    ExpressionFactory* m_factory = nullptr;

    friend class ExpressionFactory; 

    Constant(double value, bool isStatic, ExpressionFactory* factory) 
        : m_value(value), m_isStatic(isStatic), m_factory(factory) {}
    ~Constant() override = default;

public:
    void print() const override {
        std::cout << m_value;
    }

    double calculate(const Context&) const override {
        return m_value;
    }

    void addRef() override {
        if (!m_isStatic) {
            m_refCount++;
        }
    }
    
    void release() override;
};

class Variable : public Expression {
private:
    std::string m_name;
    int m_refCount = 0;
    ExpressionFactory* m_factory = nullptr;

    friend class ExpressionFactory;

    Variable(const std::string& name, ExpressionFactory* factory) 
        : m_name(name), m_factory(factory) {}
    ~Variable() override = default;

public:
    void print() const override {
        std::cout << m_name;
    }

    double calculate(const Context& context) const override {
        auto it = context.find(m_name);
        if (it != context.end()) {
            return it->second;
        }
        std::cerr << "\n[Ошибка] Переменная '" << m_name << "' не найдена в контексте!\n";
        return 0.0;
    }

    void addRef() override {
        m_refCount++;
    }
    
    void release() override;
};

// 3. Фабрика Приспособленцев (Flyweight Factory)
class ExpressionFactory {
private:
    static constexpr int MIN_CACHE = -5;
    static constexpr int MAX_CACHE = 256;
    Constant* m_staticConstants[MAX_CACHE - MIN_CACHE + 1]{};

    std::unordered_map<double, Constant*> m_dynamicConstants;
    std::unordered_map<std::string, Variable*> m_variables;

    friend class Constant;
    friend class Variable;

public:
    ExpressionFactory() {
        for (int i = MIN_CACHE; i <= MAX_CACHE; ++i) {
            m_staticConstants[i - MIN_CACHE] = new Constant(static_cast<double>(i), true, this);
        }
    }

    ~ExpressionFactory() {
        for (auto* c : m_staticConstants) {
            delete c;
        }
        for (auto& pair : m_dynamicConstants) {
            delete pair.second;
        }
        for (auto& pair : m_variables) {
            delete pair.second;
        }
    }

    Constant* createConstant(double value) {
        int intVal = static_cast<int>(value);
        if (value == intVal && intVal >= MIN_CACHE && intVal <= MAX_CACHE) {
            return m_staticConstants[intVal - MIN_CACHE];
        }

        auto it = m_dynamicConstants.find(value);
        if (it != m_dynamicConstants.end()) {
            return it->second;
        }

        auto* newConst = new Constant(value, false, this);
        m_dynamicConstants[value] = newConst;
        return newConst;
    }

    Variable* createVariable(const std::string& name) {
        auto it = m_variables.find(name);
        if (it != m_variables.end()) {
            return it->second;
        }

        auto* newVar = new Variable(name, this);
        m_variables[name] = newVar;
        return newVar;
    }
};

// Реализация методов release() после объявления ExpressionFactory
void Constant::release() {
    if (m_isStatic) return; 
    
    m_refCount--;
    if (m_refCount <= 0) {
        if (m_factory) {
            m_factory->m_dynamicConstants.erase(m_value); 
        }
        delete this; 
    }
}

void Variable::release() {
    m_refCount--;
    if (m_refCount <= 0) {
        if (m_factory) {
            m_factory->m_variables.erase(m_name);
        }
        delete this; 
    }
}

// Конкретные классы Операторов (Узлы Компоновщика)
class Addition : public Expression {
private:
    Expression* m_left;
    Expression* m_right;

public:
    Addition(Expression* left, Expression* right) : m_left(left), m_right(right) {
        m_left->addRef();
        m_right->addRef();
    }

    ~Addition() override {
        m_left->release();
        m_right->release();
    }

    void print() const override {
        std::cout << "(";
        m_left->print();
        std::cout << " + ";
        m_right->print();
        std::cout << ")";
    }

    double calculate(const Context& context) const override {
        return m_left->calculate(context) + m_right->calculate(context);
    }
};

class Subtraction : public Expression {
private:
    Expression* m_left;
    Expression* m_right;

public:
    Subtraction(Expression* left, Expression* right) : m_left(left), m_right(right) {
        m_left->addRef();
        m_right->addRef();
    }

    ~Subtraction() override {
        m_left->release();
        m_right->release();
    }

    void print() const override {
        std::cout << "(";
        m_left->print();
        std::cout << " - ";
        m_right->print();
        std::cout << ")";
    }

    double calculate(const Context& context) const override {
        return m_left->calculate(context) - m_right->calculate(context);
    }
};


int main() {
    ExpressionFactory factory;
    Context context;

    // --- Тест 1: Пример из условия задачи (2 + x при x = 3) ---
    std::cout << "=== ТЕСТ 1 (Пример из условия) ===\n";
    
    Constant* c = factory.createConstant(2);    
    Variable* v = factory.createVariable("x");   

    Addition* expression = new Addition(c, v);

    context["x"] = 3;

    std::cout << "Выражение: ";
    expression->print();
    std::cout << "\nРезультат вычисления: " << expression->calculate(context) << std::endl;

    delete expression; 
    
    std::cout << "\n=== ТЕСТ 2 (Сложное дерево и разделение объектов) ===\n";

    // --- Тест 2: Выражение (x - -5) + (x + 300) при x = 10 ---
    Variable* v1 = factory.createVariable("x");
    Constant* c1 = factory.createConstant(-5);   
    Expression* leftSub = new Subtraction(v1, c1);

    Variable* v2 = factory.createVariable("x");   
    Constant* c2 = factory.createConstant(300);  
    Expression* rightAdd = new Addition(v2, c2);

    Expression* complexExpression = new Addition(leftSub, rightAdd);

    std::cout << "Адрес первой переменной 'x': " << v1 << "\n";
    std::cout << "Адрес второй переменной 'x': " << v2 << "\n";
    assert(v1 == v2 && "Ошибка! Фабрика должна возвращать один и тот же объект для одной переменной");
    std::cout << "-> Указатели совпадают! Паттерн Flyweight работает корректно.\n";

    context["x"] = 10;
    std::cout << "Сложное выражение: ";
    complexExpression->print();
    std::cout << "\nРезультат: " << complexExpression->calculate(context) << std::endl; // (10 - -5) + (10 + 300) = 15 + 310 = 325

    delete complexExpression;

    std::cout << "Очистка дерева выполнена успешно. Утечек памяти нет.\n";

    return 0;
}