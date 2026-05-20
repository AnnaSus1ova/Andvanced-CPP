#ifndef POINTCLOUD_PREPROCESSOR__CROP_BOX_FILTER_HPP_
#define POINTCLOUD_PREPROCESSOR__CROP_BOX_FILTER_HPP_

#include "../filter.hpp"
#include <memory>
#include <vector>

namespace pointcloud_preprocessor
{

class CropBoxFilter : public Filter
{
public:
    struct CropBoxParam
    {
        double min_x{0.0};
        double max_x{0.0};
        double min_y{0.0};
        double max_y{0.0};
        double min_z{0.0};
        double max_z{0.0};
        bool negative{false};
    };

    CropBoxFilter();
    ~CropBoxFilter() override = default;
    
    std::unique_ptr<PointCloud> Apply(const PointCloud* pc) override;
    void SetParams(const FilterParametr& param) override;
    
    const CropBoxParam& GetParam() const { return param_; }

private:
    CropBoxParam param_;
    bool ValidateParam(const CropBoxParam& new_param) const;
    bool IsPointInside(const double* point) const;
    void UpdateParam(const CropBoxParam& new_param);
};

}  // namespace pointcloud_preprocessor

#endif