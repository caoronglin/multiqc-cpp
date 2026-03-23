#pragma once

#include "mqc/core/module.hpp"

namespace mqc {

/**
 * @brief Picard 模块
 * 解析 Picard MarkDuplicates, CollectInsertSizeMetrics 等输出
 */
class PicardModule : public BaseModule {
public:
    PicardModule();
    void parse(const std::vector<MatchedFile>& files) override;
    
private:
    bool parse_mark_duplicates(const std::string& filepath, const std::string& sample_name);
    bool parse_insert_size(const std::string& filepath, const std::string& sample_name);
    
    struct PicardData {
        size_t read_pairs = 0;
        size_t unpaired_reads = 0;
        size_t duplicates = 0;
        double percent_duplicates = 0.0;
        size_t estimated_library_size = 0;
        double median_insert_size = 0.0;
        double mean_insert_size = 0.0;
        double std_dev_insert_size = 0.0;
    };
    
    std::map<std::string, PicardData> samples_;
};

REGISTER_MODULE("picard", PicardModule);

} // namespace mqc
