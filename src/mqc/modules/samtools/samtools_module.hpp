#pragma once

#include "mqc/core/module.hpp"

namespace mqc {

/**
 * @brief Samtools 模块
 * 解析 samtools flagstat, stats, idxstats 输出
 */
class SamtoolsModule : public BaseModule {
public:
    SamtoolsModule();
    void parse(const std::vector<MatchedFile>& files) override;
    
private:
    bool parse_flagstat(const std::string& filepath, const std::string& sample_name);
    bool parse_stats(const std::string& filepath, const std::string& sample_name);
    
    struct SamtoolsData {
        size_t total_reads = 0;
        size_t mapped = 0;
        size_t paired = 0;
        double percent_mapped = 0.0;
        double percent_properly_paired = 0.0;
    };
    
    std::map<std::string, SamtoolsData> samples_;
};

REGISTER_MODULE("samtools", SamtoolsModule);

} // namespace mqc
