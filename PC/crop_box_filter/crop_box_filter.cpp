#include "crop_box_filter.hpp"
#include <cstring>
#include <cmath>
#include <algorithm>

namespace pointcloud_preprocessor
{

CropBoxFilter::CropBoxFilter()
    : Filter("CropBoxFilter")
{
}

bool CropBoxFilter::IsPointInside(const double* point) const
{
    return point[2] > param_.min_z && point[2] < param_.max_z &&
           point[1] > param_.min_y && point[1] < param_.max_y &&
           point[0] > param_.min_x && point[0] < param_.max_x;
}

std::unique_ptr<PointCloud> CropBoxFilter::Apply(const PointCloud* pc)
{
    if (!pc || pc->size_ == 0) {
        return std::make_unique<PointCloud>();
    }
    
    auto accessor = GetAccessor(pc);
    if (!accessor) {
        logger_.log("Unknown pointcloud type: " + pc->pointcloud_type_);
        return std::make_unique<PointCloud>();
    }
    
    std::vector<double> output;
    output.reserve(pc->size_ * pc->point_size_);
    
    size_t output_points_count = 0;
    
    for (size_t i = 0; i < pc->size_; ++i) {
        double point[3] = {
            accessor->GetX(pc, i),
            accessor->GetY(pc, i),
            accessor->GetZ(pc, i)
        };
        
        if (!std::isfinite(point[0]) || !std::isfinite(point[1]) || !std::isfinite(point[2])) {
            logger_.log("Ignoring point containing NaN values");
            continue;
        }
        
        bool point_is_inside = IsPointInside(point);
        
        if ((!param_.negative && point_is_inside) || (param_.negative && !point_is_inside)) {
            size_t offset = i * pc->point_size_;
            for (size_t j = 0; j < pc->point_size_; ++j) {
                output.push_back(pc->points_[offset + j]);
            }
            output_points_count++;
        }
    }
    
    auto output_pc = std::make_unique<PointCloud>();
    output_pc->points_ = std::move(output);
    output_pc->pointcloud_type_ = pc->pointcloud_type_;
    output_pc->size_ = output_points_count;
    output_pc->point_size_ = pc->point_size_;
    
    return output_pc;
}

bool CropBoxFilter::ValidateParam(const CropBoxParam& new_param) const
{
    return new_param.min_x < new_param.max_x &&
           new_param.min_y < new_param.max_y &&
           new_param.min_z < new_param.max_z;
}

void CropBoxFilter::UpdateParam(const CropBoxParam& new_param)
{
    logger_.log("[paramCallback] Setting the minimum point to: " +
        std::to_string(new_param.min_x) + " " + std::to_string(new_param.min_y) + " " + std::to_string(new_param.min_z));
    logger_.log("[paramCallback] Setting the maximum point to: " +
        std::to_string(new_param.max_x) + " " + std::to_string(new_param.max_y) + " " + std::to_string(new_param.max_z));
    logger_.log("[paramCallback] Setting the filter negative flag to: " + std::string(new_param.negative ? "true" : "false"));
    param_ = new_param;
}

void CropBoxFilter::SetParams(const FilterParametr& param)
{
    CropBoxParam new_param{};
    
    new_param.min_x = param.GetParam("min_x", new_param.min_x);
    new_param.max_x = param.GetParam("max_x", new_param.max_x);
    new_param.min_y = param.GetParam("min_y", new_param.min_y);
    new_param.max_y = param.GetParam("max_y", new_param.max_y);
    new_param.min_z = param.GetParam("min_z", new_param.min_z);
    new_param.max_z = param.GetParam("max_z", new_param.max_z);
    
    if (param.HasParam("negative")) {
        new_param.negative = static_cast<bool>(param.GetParam("negative", 0));
    }
    
    if (ValidateParam(new_param)) {
        if (param_.min_x != new_param.min_x || param_.max_x != new_param.max_x ||
            param_.min_y != new_param.min_y || param_.max_y != new_param.max_y ||
            param_.min_z != new_param.min_z || param_.max_z != new_param.max_z ||
            param_.negative != new_param.negative) {
            UpdateParam(new_param);
        }
    }
    
    Filter::SetParams(param);
}

}  // namespace pointcloud_preprocessor