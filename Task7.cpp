#include <iostream>
#include <vector>
#include <unordered_set>
#include <set>
#include <algorithm>
#include <memory>
#include <stdexcept>

// Абстрактная реализация
template<typename T>
class SetImplementation {
public:
    virtual ~SetImplementation() = default;
    
    virtual void add(const T& element) = 0;
    virtual void remove(const T& element) = 0;
    virtual bool contains(const T& element) const = 0;
    virtual size_t size() const = 0;
    virtual bool empty() const = 0;
    virtual void clear() = 0;
    virtual std::vector<T> getElements() const = 0;
    virtual std::unique_ptr<SetImplementation<T>> clone() const = 0;
    virtual bool isSuitableForSize(size_t currentSize) const = 0;
};

// Конкретная реализация 1: Массив
template<typename T>
class ArraySetImplementation : public SetImplementation<T> {
private:
    std::vector<T> elements;
    static constexpr size_t MAX_SIZE_FOR_ARRAY = 20;
    
public:
    void add(const T& element) override {
        if (!contains(element)) {
            elements.push_back(element);
        }
    }
    
    void remove(const T& element) override {
        auto it = std::find(elements.begin(), elements.end(), element);
        if (it != elements.end()) {
            elements.erase(it);
        }
    }
    
    bool contains(const T& element) const override {
        return std::find(elements.begin(), elements.end(), element) != elements.end();
    }
    
    size_t size() const override { return elements.size(); }
    bool empty() const override { return elements.empty(); }
    void clear() override { elements.clear(); }
    
    std::vector<T> getElements() const override { return elements; }
    
    std::unique_ptr<SetImplementation<T>> clone() const override {
        auto cloned = std::make_unique<ArraySetImplementation<T>>();
        cloned->elements = this->elements;
        return cloned;
    }
    
    bool isSuitableForSize(size_t currentSize) const override {
        return currentSize <= MAX_SIZE_FOR_ARRAY;
    }
};

// Конкретная реализация 2: Хэш-таблица
template<typename T>
class HashSetImplementation : public SetImplementation<T> {
private:
    std::unordered_set<T> elements;
    static constexpr size_t MIN_SIZE_FOR_HASH = 10;
    
public:
    void add(const T& element) override { elements.insert(element); }
    void remove(const T& element) override { elements.erase(element); }
    
    bool contains(const T& element) const override {
        return elements.find(element) != elements.end();
    }
    
    size_t size() const override { return elements.size(); }
    bool empty() const override { return elements.empty(); }
    void clear() override { elements.clear(); }
    
    std::vector<T> getElements() const override {
        return std::vector<T>(elements.begin(), elements.end());
    }
    
    std::unique_ptr<SetImplementation<T>> clone() const override {
        auto cloned = std::make_unique<HashSetImplementation<T>>();
        cloned->elements = this->elements;
        return cloned;
    }
    
    bool isSuitableForSize(size_t currentSize) const override {
        return currentSize >= MIN_SIZE_FOR_HASH;
    }
};

// Абстракция
template<typename T>
class Set {
private:
    std::unique_ptr<SetImplementation<T>> impl;
    
    void checkAndSwitchImplementation() {
        if (!impl) return;
        
        size_t currentSize = impl->size();
        std::unique_ptr<SetImplementation<T>> newImpl;
        
        if (currentSize <= 20 && !dynamic_cast<ArraySetImplementation<T>*>(impl.get())) {
            newImpl = std::make_unique<ArraySetImplementation<T>>();
        }
        else if (currentSize >= 10 && !dynamic_cast<HashSetImplementation<T>*>(impl.get())) {
            newImpl = std::make_unique<HashSetImplementation<T>>();
        }
        
        if (newImpl) {
            auto elements = impl->getElements();
            for (const auto& elem : elements) {
                newImpl->add(elem);
            }
            impl = std::move(newImpl);
        }
    }
    
public:
    Set() : impl(std::make_unique<ArraySetImplementation<T>>()) {}
    
    Set(const Set& other) : impl(other.impl->clone()) {}
    Set(Set&& other) noexcept = default;
    
    Set& operator=(const Set& other) {
        if (this != &other) {
            impl = other.impl->clone();
        }
        return *this;
    }
    
    Set& operator=(Set&& other) noexcept = default;
    
    void add(const T& element) {
        impl->add(element);
        checkAndSwitchImplementation();
    }
    
    void remove(const T& element) {
        impl->remove(element);
        checkAndSwitchImplementation();
    }
    
    bool contains(const T& element) const {
        return impl->contains(element);
    }
    
    size_t size() const { return impl->size(); }
    bool empty() const { return impl->empty(); }
    void clear() { impl->clear(); }
    
    // Объединение множеств
    Set unionWith(const Set& other) const {
        Set result;
        for (const auto& elem : this->impl->getElements()) {
            result.add(elem);
        }
        for (const auto& elem : other.impl->getElements()) {
            result.add(elem);
        }
        return result;
    }
    
    // Пересечение множеств
    Set intersectionWith(const Set& other) const {
        Set result;
        for (const auto& elem : this->impl->getElements()) {
            if (other.contains(elem)) {
                result.add(elem);
            }
        }
        return result;
    }
    
    void print() const {
        auto elements = impl->getElements();
        std::cout << "{ ";
        for (const auto& elem : elements) {
            std::cout << elem << " ";
        }
        std::cout << "}" << std::endl;
    }
};

int main() {
    // Создаём множество и добавляем элементы
    Set<int> mySet;
    
    std::cout << "Adding elements 1-25:" << std::endl;
    for (int i = 1; i <= 25; ++i) {
        mySet.add(i);
    }
    
    std::cout << "Set contents: ";
    mySet.print();
    
    std::cout << "Size: " << mySet.size() << std::endl;
    std::cout << "Contains 10? " << (mySet.contains(10) ? "Yes" : "No") << std::endl;
    std::cout << "Contains 30? " << (mySet.contains(30) ? "Yes" : "No") << std::endl;
    
    // Удаление элемента
    mySet.remove(10);
    std::cout << "After removing 10: ";
    mySet.print();
    
    // Объединение множеств
    Set<int> anotherSet;
    for (int i = 20; i <= 30; ++i) {
        anotherSet.add(i);
    }
    
    Set<int> unionSet = mySet.unionWith(anotherSet);
    std::cout << "Union: ";
    unionSet.print();
    
    // Пересечение множеств
    Set<int> intersectionSet = mySet.intersectionWith(anotherSet);
    std::cout << "Intersection: ";
    intersectionSet.print();
    
    return 0;
}