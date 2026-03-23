#include "samtools_module.hpp"
#include <spdlog/spdlog.h>
#include <fstream>
#include <sstream>
#include <regex>

namespace mqc
{

SamtoolsModule::SamtoolsModule()
    : BaseModule("samtools", "SAMtools", "SAMtools manipulates and analyzes SAM/BAM files") {}

void SamtoolsModule::parse(const std::vector<MatchedFile>& files) {
    spdlog::info("Parsing {} SAMtools files", files.size());

    for (const auto& file : files) {
        if (file.fn.find("flagstat") != std::string::npos) {
            parse_flagstat(file.filepath, file.sample_name);
        } else if (file.fn.find("stats") != std::string::npos) {
            parse_stats(file.filepath, file.sample_name);
        }
    }

    if (!samples_.empty()) {
        // 添加到通用统计表
        nlohmann::json data = nlohmann::json::object();
        std::map<std::string, GeneralStatsColumn> headers;

        for (const auto& [sample_id, sample] : samples_) {
            data[sample_id] = {{"total_reads", sample.total_reads},
                               {"mapped", sample.mapped},
                               {"paired", sample.paired},
                               {"percent_mapped", sample.percent_mapped},
                               {"percent_properly_paired", sample.percent_properly_paired}};
        }

        headers = {
            {"total_reads", {"Total Reads", "Total number of reads", "Blues", "{:,d}", "", "", false, 100000000, 0}},
            {"mapped", {"Mapped", "Number of mapped reads", "Greens", "{:,d}", "", "", false, 100000000, 0}},
            {"percent_mapped", {"% Mapped", "Percentage of mapped reads", "Greens", "{:.1f}", "%", "", false, 100, 0}},
            {"paired", {"Paired", "Number of paired reads", "Oranges", "{:,d}", "", "", false, 100000000, 0}},
            {"percent_properly_paired",
             {"% Properly Paired", "Percentage of properly paired reads", "Oranges", "{:.1f}", "%", "", false, 100,
              0}}};

        general_stats_addcols(data, headers);

        add_section("SAMtools Statistics", "samtools_stats", "Alignment statistics from SAMtools");
    }
}

bool SamtoolsModule::parse_flagstat(const std::string& filepath, const std::string& sample_name) {
    std::ifstream ifs(filepath);
    if (!ifs.is_open()) {
        spdlog::warn("Cannot open flagstat file: {}", filepath);
        return false;
    }

    SamtoolsData& data = samples_[sample_name];

    std::string line;
    std::regex total_re(R"((\d+)\s+\+\s+(\d+)\s+in total)");
    std::regex mapped_re(R"((\d+)\s+\+\s+(\d+)\s+mapped)");
    std::regex paired_re(R"((\d+)\s+\+\s+(\d+)\s+paired in sequencing)");
    std::regex properly_paired_re(R"((\d+)\s+\+\s+(\d+)\s+properly paired)");

    while (std::getline(ifs, line)) {
        std::smatch match;

        if (std::regex_search(line, match, total_re) && match.size() > 2) {
            data.total_reads = std::stoull(match[1].str()) + std::stoull(match[2].str());
        } else if (std::regex_search(line, match, mapped_re) && match.size() > 2) {
            data.mapped = std::stoull(match[1].str()) + std::stoull(match[2].str());
            if (data.total_reads > 0) {
                data.percent_mapped = (double)data.mapped / data.total_reads * 100.0;
            }
        } else if (std::regex_search(line, match, paired_re) && match.size() > 2) {
            data.paired = std::stoull(match[1].str()) + std::stoull(match[2].str());
        } else if (std::regex_search(line, match, properly_paired_re) && match.size() > 2) {
            size_t properly_paired = std::stoull(match[1].str()) + std::stoull(match[2].str());
            if (data.paired > 0) {
                data.percent_properly_paired = (double)properly_paired / data.paired * 100.0;
            }
        }
    }

    spdlog::debug("Parsed flagstat for {}: {} reads, {:.1f}% mapped", sample_name, data.total_reads,
                  data.percent_mapped);

    return true;
}

bool SamtoolsModule::parse_stats(const std::string& filepath, const std::string& sample_name) {
    // TODO: 实现 samtools stats 解析
    spdlog::debug("Parsing samtools stats - TODO: {}", filepath);
    return false;
}

} // namespace mqc
