#ifndef POINTCLOUD_H_
#define POINTCLOUD_H_

#include <string>
#include <stdexcept>
#include <vector>
#include <memory>
#include <algorithm>

class PointCloud {
public:
    void Init(const std::string& pc_type, size_t points_count) {
        pointcloud_type_ = pc_type;
        size_ = points_count;
        
        if (pointcloud_type_ == "XYZIR") {
            point_size_ = 5;
            points_ = std::vector<double>(size_ * point_size_);
        } else if (pointcloud_type_ == "XYZIRDAT") {
            point_size_ = 8;
            points_ = std::vector<double>(size_ * point_size_);
        } else {
            throw std::runtime_error("unknown pointcloud type: " + pointcloud_type_);
        }
    }
    
    void AddPoint(const std::vector<double>& point, size_t ind) {
        if (ind >= size_) {
            throw std::runtime_error("Index out of bounds");
        }
        
        size_t expected_size = (pointcloud_type_ == "XYZIR") ? 5 : 8;
        if (point.size() != expected_size) {
            throw std::runtime_error("Point has unsupported size = " + std::to_string(point.size()) + 
                                    " for type " + pointcloud_type_);
        }
        
        size_t offset = point_size_ * ind;
        std::copy(point.begin(), point.end(), points_.begin() + offset);
    }
    
    std::string pointcloud_type_;
    size_t size_{0};
    size_t point_size_{0};
    std::vector<double> points_;
};

inline void FillPointCloud(PointCloud* pc, size_t points_count, const std::string& point_type, const std::vector<double>& data) {
    size_t point_size = (point_type == "XYZIR") ? 5 : 8;
    pc->Init(point_type, points_count);
    
    for (size_t i = 0; i < points_count; ++i) {
        size_t offset = i * point_size;
        std::vector<double> point(data.begin() + offset, data.begin() + offset + point_size);
        pc->AddPoint(point, i);
    }
}

#endif