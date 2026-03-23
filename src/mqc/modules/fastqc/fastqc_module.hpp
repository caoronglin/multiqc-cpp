#pragma once

#include "mqc/core/module.hpp"
#include <map>
#include <string>

namespace mqc {

/**
 * @brief FastQC 模块
 * 
 * 解析 FastQC 输出文件 (fastqc_data.txt)
 * 生成质量统计报告和图表
 */
class FastQCModule : public BaseModule {
public:
    FastQCModule();
    
    void parse(const std::vector<MatchedFile>& files) override;
    
private:
    // 解析方法
    bool parse_fastqc_data(const std::string& filepath, const std::string& sample_name);
    
    // 图表生成
    void add_quality_plot();
    void add_gc_plot();
    void add_sequence_length_plot();
    void add_basic_stats();
    
    // 数据结构
    struct SampleData {
        std::string sample_name;
        size_t total_sequences = 0;
        double percent_gc = 0.0;
        size_t avg_sequence_length = 0;
        double percent_duplicates = 0.0;
        double percent_fails = 0.0;
        
        std::map<int, double> quality_per_base;  // position -> avg_qual
        std::map<int, double> gc_per_base;        // position -> gc_content
        std::map<size_t, size_t> length_dist;     // length -> count
    };
    
    std::map<std::string, SampleData> samples_;
};

} // namespace mqc
