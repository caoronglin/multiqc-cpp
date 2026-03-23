#pragma once

#include "mqc/core/module.hpp"

namespace mqc
{

/**
 * @brief fastp 模块
 * 解析 fastp JSON 报告，整合 fastp 质控数据
 */
class FastpModule : public BaseModule
{
  public:
    FastpModule();

    void parse(const std::vector<MatchedFile>& files) override;

  private:
    bool parse_fastp_json(const std::string& filepath, const std::string& sample_name);
    void add_quality_plot();
    void add_base_content_plot();
    void add_filtering_stats();

    struct FastpData
    {
        size_t total_reads = 0;
        size_t total_bases = 0;
        size_t passed_reads = 0;
        size_t passed_bases = 0;
        double percent_passed = 0.0;
        double q20_rate = 0.0;
        double q30_rate = 0.0;
        double gc_content = 0.0;
        size_t read1_bases = 0;
        size_t read2_bases = 0;

        std::map<int, double> read1_quality;
        std::map<int, double> read2_quality;
        std::map<int, double> read1_gc;
        std::map<int, double> read2_gc;
    };

    std::map<std::string, FastpData> samples_;
};

REGISTER_MODULE("fastp", FastpModule);

} // namespace mqc
