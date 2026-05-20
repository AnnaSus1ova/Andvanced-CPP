#ifndef POINTCLOUD_PREPROCESSOR__FILTER_HPP_
#define POINTCLOUD_PREPROCESSOR__FILTER_HPP_

#include "Logger.h"
#include "PointCloud.h"

#include <string>
#include <cmath>
#include <unordered_map>
#include <memory>
#include <functional>

namespace pointcloud_preprocessor
{

class FilterParametr {
public:
    FilterParametr() = default;
    explicit FilterParametr(std::unordered_map<std::string, double> params) : params_(std::move(params)) {}
    
    void ChangeParam(const std::string& param_name, double param_val) {
        params_[param_name] = param_val;
    }

    double GetParam(const std::string& param_name) const {
        return params_.at(param_name);
    }

    double GetParam(const std::string& param_name, double default_val) const noexcept {
        auto it = params_.find(param_name);
        return it != params_.end() ? it->second : default_val;
    }

    bool HasParam(const std::string& param_name) const noexcept {
        return params_.find(param_name) != params_.end();
    }

private:
    std::unordered_map<std::string, double> params_;
};

class IPointAccessor {
public:
    virtual ~IPointAccessor() = default;
    virtual double GetDistance(const PointCloud* pc, size_t index) const = 0;
    virtual double GetAzimuth(const PointCloud* pc, size_t index) const = 0;
    virtual double GetX(const PointCloud* pc, size_t index) const = 0;
    virtual double GetY(const PointCloud* pc, size_t index) const = 0;
    virtual double GetZ(const PointCloud* pc, size_t index) const = 0;
    virtual size_t GetPointSize() const = 0;
};

class XYZIRPointAccessor : public IPointAccessor {
public:
    double GetDistance(const PointCloud* pc, size_t index) const override {
        double x = pc->points_[index * GetPointSize() + 0];
        double y = pc->points_[index * GetPointSize() + 1];
        double z = pc->points_[index * GetPointSize() + 2];
        return std::hypot(x, y, z);
    }

    double GetAzimuth(const PointCloud* pc, size_t index) const override {
        double x = pc->points_[index * GetPointSize() + 0];
        double y = pc->points_[index * GetPointSize() + 1];
        return std::atan2(y, x);
    }

    double GetX(const PointCloud* pc, size_t index) const override {
        return pc->points_[index * GetPointSize() + 0];
    }

    double GetY(const PointCloud* pc, size_t index) const override {
        return pc->points_[index * GetPointSize() + 1];
    }

    double GetZ(const PointCloud* pc, size_t index) const override {
        return pc->points_[index * GetPointSize() + 2];
    }

    size_t GetPointSize() const override { return 5; }
};

class XYZIRDATPointAccessor : public IPointAccessor {
public:
    double GetDistance(const PointCloud* pc, size_t index) const override {
        return pc->points_[index * GetPointSize() + 5];
    }

    double GetAzimuth(const PointCloud* pc, size_t index) const override {
        return pc->points_[index * GetPointSize() + 6];
    }

    double GetX(const PointCloud* pc, size_t index) const override {
        return pc->points_[index * GetPointSize() + 0];
    }

    double GetY(const PointCloud* pc, size_t index) const override {
        return pc->points_[index * GetPointSize() + 1];
    }

    double GetZ(const PointCloud* pc, size_t index) const override {
        return pc->points_[index * GetPointSize() + 2];
    }

    size_t GetPointSize() const override { return 8; }
};

class PointAccessorFactory {
public:
    static std::unique_ptr<IPointAccessor> Create(const std::string& type) {
        if (type == "XYZIR") {
            return std::make_unique<XYZIRPointAccessor>();
        } else if (type == "XYZIRDAT") {
            return std::make_unique<XYZIRDATPointAccessor>();
        }
        return nullptr;
    }
};

class Filter
{
public:
    explicit Filter(const std::string& filter_name = "pointcloud_preprocessor_filter") 
        : filter_name_(filter_name), logger_(filter_name_) {}
    
    virtual ~Filter() = default;
    
    const std::string& GetFilterName() const { return filter_name_; }
    
    virtual std::unique_ptr<PointCloud> Apply(const PointCloud* pc) = 0;
    
    virtual void SetParams(const FilterParametr& param) {
        params_ = param;
    }

    const FilterParametr& GetFilterParam() const {
        return params_;
    }

protected:
    std::string filter_name_;
    Logger logger_;
    FilterParametr params_;
    
    std::unique_ptr<IPointAccessor> GetAccessor(const PointCloud* pc) const {
        return PointAccessorFactory::Create(pc->pointcloud_type_);
    }
};

}  // namespace pointcloud_preprocessor

#endif