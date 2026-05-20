#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <iomanip>
#include <sstream>

// Координаты
struct Coordinates {
    double latitude;
    double longitude;
    
    Coordinates(double lat, double lon) : latitude(lat), longitude(lon) {}
    
    std::string toString() const {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(6);
        oss << latitude << "°, " << longitude << "°";
        return oss.str();
    }
};

// Контрольные пункты
class ControlPoint {
protected:
    std::string name;
    Coordinates coordinates;
    
public:
    ControlPoint(const std::string& n, const Coordinates& coords) 
        : name(n), coordinates(coords) {}
    virtual ~ControlPoint() = default;
    
    virtual bool isMandatory() const = 0;
    virtual double getPenalty() const = 0;
    virtual std::string getPenaltyString() const = 0;
    
    const std::string& getName() const { return name; }
    const Coordinates& getCoordinates() const { return coordinates; }
};

class MandatoryControlPoint : public ControlPoint {
public:
    MandatoryControlPoint(const std::string& n, const Coordinates& coords)
        : ControlPoint(n, coords) {}
    
    bool isMandatory() const override { return true; }
    double getPenalty() const override { return 0.0; }
    std::string getPenaltyString() const override { return "незачёт СУ"; }
};

class OptionalControlPoint : public ControlPoint {
    double penalty;
public:
    OptionalControlPoint(const std::string& n, const Coordinates& coords, double p)
        : ControlPoint(n, coords), penalty(p) {}
    
    bool isMandatory() const override { return false; }
    double getPenalty() const override { return penalty; }
    std::string getPenaltyString() const override {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(2) << penalty << " ч";
        return oss.str();
    }
};

// Маршрут
class Route {
    std::vector<std::unique_ptr<ControlPoint>> points;
    std::string routeName;
public:
    Route(const std::string& name = "") : routeName(name) {}
    
    void addPoint(std::unique_ptr<ControlPoint> point) {
        points.push_back(std::move(point));
    }
    
    const std::vector<std::unique_ptr<ControlPoint>>& getPoints() const {
        return points;
    }
    
    const std::string& getName() const { return routeName; }
};

// Builder
class RouteBuilder {
public:
    virtual ~RouteBuilder() = default;
    virtual void buildRouteName(const std::string& name) = 0;
    virtual void buildMandatoryPoint(const std::string& name, const Coordinates& coords) = 0;
    virtual void buildOptionalPoint(const std::string& name, const Coordinates& coords, double penalty) = 0;
    virtual void getResult() = 0;
    virtual void reset() = 0;
};

// ConcreteBuilder для текстового вывода
class TextOutputBuilder : public RouteBuilder {
    std::ostringstream output;
    int pointCounter = 0;
    
public:
    void buildRouteName(const std::string& name) override {
        output << "\n=== " << name << " ===\n";
        output << "№ | Название | Координаты | Штраф/Статус\n";
        output << "----------------------------------------\n";
    }
    
    void buildMandatoryPoint(const std::string& name, const Coordinates& coords) override {
        pointCounter++;
        output << pointCounter << " | " << name << " | " 
               << coords.toString() << " | незачёт СУ\n";
    }
    
    void buildOptionalPoint(const std::string& name, const Coordinates& coords, double penalty) override {
        pointCounter++;
        std::ostringstream ps;
        ps << std::fixed << std::setprecision(2) << penalty << " ч";
        output << pointCounter << " | " << name << " | " 
               << coords.toString() << " | " << ps.str() << "\n";
    }
    
    void getResult() override {}
    void reset() override {
        output.str("");
        output.clear();
        pointCounter = 0;
    }
    
    std::string getTextOutput() const { return output.str(); }
};

// ConcreteBuilder для подсчёта штрафа
class PenaltyCalculatorBuilder : public RouteBuilder {
    double totalPenalty = 0.0;
    int optionalCount = 0;
    
public:
    void buildRouteName(const std::string&) override {}
    void buildMandatoryPoint(const std::string&, const Coordinates&) override {}
    void buildOptionalPoint(const std::string&, const Coordinates&, double penalty) override {
        optionalCount++;
        totalPenalty += penalty;
    }
    void getResult() override {}
    void reset() override {
        totalPenalty = 0.0;
        optionalCount = 0;
    }
    
    double getTotalPenalty() const { return totalPenalty; }
    int getOptionalCount() const { return optionalCount; }
};

// Director
class RallyDirector {
    std::unique_ptr<RouteBuilder> builder;
public:
    explicit RallyDirector(std::unique_ptr<RouteBuilder> b) : builder(std::move(b)) {}
    
    void setBuilder(std::unique_ptr<RouteBuilder> b) {
        builder = std::move(b);
    }
    
    void constructRoute(const Route& route) {
        builder->reset();
        builder->buildRouteName(route.getName());
        for (const auto& point : route.getPoints()) {
            if (point->isMandatory()) {
                builder->buildMandatoryPoint(point->getName(), point->getCoordinates());
            } else {
                builder->buildOptionalPoint(point->getName(), point->getCoordinates(), point->getPenalty());
            }
        }
        builder->getResult();
    }
    
    RouteBuilder* getBuilder() const { return builder.get(); }
};

int main() {
    // Создаём маршрут
    auto route = std::make_unique<Route>("Трофи-рейд 2026");
    route->addPoint(std::make_unique<MandatoryControlPoint>("Старт", Coordinates(55.7558, 37.6173)));
    route->addPoint(std::make_unique<OptionalControlPoint>("Оазис", Coordinates(45.1234, 42.5678), 2.5));
    route->addPoint(std::make_unique<MandatoryControlPoint>("Финиш", Coordinates(56.0000, 38.0000)));
    
    // Вывод списка КП
    auto textBuilder = std::make_unique<TextOutputBuilder>();
    RallyDirector director(std::move(textBuilder));
    director.constructRoute(*route);
    
    auto textOutput = dynamic_cast<TextOutputBuilder*>(director.getBuilder());
    if (textOutput) {
        std::cout << textOutput->getTextOutput() << std::endl;
    }
    
    // Подсчёт суммарного штрафа
    auto penaltyBuilder = std::make_unique<PenaltyCalculatorBuilder>();
    director.setBuilder(std::move(penaltyBuilder));
    director.constructRoute(*route);
    
    auto penaltyCalc = dynamic_cast<PenaltyCalculatorBuilder*>(director.getBuilder());
    if (penaltyCalc) {
        std::cout << "\nСуммарный штраф: " << std::fixed << std::setprecision(2) 
                  << penaltyCalc->getTotalPenalty() << " часов" << std::endl;
    }
    
    return 0;
}