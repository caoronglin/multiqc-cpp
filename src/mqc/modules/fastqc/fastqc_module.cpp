#include "fastqc_module.hpp"
#include <spdlog/spdlog.h>
#include <fstream>
#include <sstream>
#include <algorithm>

namespace mqc {

FastQCModule::FastQCModule() 
    : BaseModule("fastqc", "FastQC", "FastQC provides a simple way to do some quality control checks on raw sequence data") {
}

void FastQCModule::parse(const std::vector<MatchedFile>& files) {
    spdlog::info("Parsing {} FastQC files", files.size());
    
    for (const auto& file : files) {
        if (parse_fastqc_data(file.filepath, file.sample_name)) {
            spdlog::debug("Successfully parsed: {}", file.filepath);
        } else {
            spdlog::warn("Failed to parse: {}", file.filepath);
        }
    }
    
    spdlog::info("Parsed {} FastQC samples", samples_.size());
    
    if (!samples_.empty()) {
        add_basic_stats();
        add_quality_plot();
        add_gc_plot();
        add_sequence_length_plot();
        
        add_section("Per Base Sequence Quality", "per_base_quality", 
                   "Boxplot of sequence quality scores across all bases");
        add_section("Per Sequence GC Content", "gc_content",
                   "Distribution of GC content across sequences");
        add_section("Sequence Length Distribution", "seq_length",
                   "Distribution of sequence lengths");
    }
}

bool FastQCModule::parse_fastqc_data(const std::string& filepath, const std::string& sample_name) {
    std::ifstream ifs(filepath);
    if (!ifs.is_open()) {
        spdlog::error("Cannot open file: {}", filepath);
        return false;
    }
    
    SampleData& sample = samples_[sample_name];
    sample.sample_name = sample_name;
    
    std::string line;
    std::string current_section;
    
    while (std::getline(ifs, line)) {
        // 跳过空行和注释
        if (line.empty() || line[0] == '#') continue;
        
        // 检测新章节
        if (line[0] == '>>') {
            if (line.substr(0, 3) == ">>>") {
                // 章节结束
                current_section.clear();
            } else {
                // 章节开始
                current_section = line.substr(2);
                auto pos = current_section.find(' ');
                if (pos != std::string::npos) {
                    current_section = current_section.substr(0, pos);
                }
            }
            continue;
        }
        
        // 解析章节数据
        std::istringstream iss(line);
        std::string key;
        iss >> key;
        
        if (current_section == "Basic" && key == "Total") {
            // Total Sequences
            iss >> sample.total_sequences;
        } else if (current_section == "Basic" && key == "%GC") {
            double gc;
            iss >> gc;
            sample.percent_gc = gc;
        } else if (current_section == "Basic" && key.find("Sequence") != std::string::npos) {
            // Sequence length
            std::string tmp;
            iss >> tmp;  // "length"
            size_t len;
            iss >> len;
            sample.avg_sequence_length = std::max(sample.avg_sequence_length, len);
        } else if (current_section == "Per" && key.find("base") != std::string::npos) {
            // Per base quality
            int position;
            iss >> position;
            std::string med;
            iss >> med;  // Median
            double qual;
            iss >> qual;
            sample.quality_per_base[position] = qual;
        }
    }
    
    return true;
}

void FastQCModule::add_basic_stats() {
    nlohmann::json data = nlohmann::json::object();
    std::map<std::string, GeneralStatsColumn> headers;
    
    for (const auto& [sample_id, sample] : samples_) {
        data[sample_id] = {
            {"total_sequences", sample.total_sequences},
            {"percent_gc", sample.percent_gc},
            {"avg_sequence_length", sample.avg_sequence_length},
            {"percent_duplicates", sample.percent_duplicates},
            {"percent_fails", sample.percent_fails}
        };
    }
    
    headers = {
        {"total_sequences", {"Total Sequences", "Total number of sequences", "Blues", "{:,d}", "", "", false, 1000000, 0}},
        {"percent_gc", {"% GC", "GC content percentage", "Greens", "{:.1f}", "%", "", false, 100, 0}},
        {"avg_sequence_length", {"Avg Length", "Average sequence length", "Oranges", "{:,d}", " bp", "", false, 1000, 0}},
        {"percent_duplicates", {"% Duplicates", "Percentage of duplicate sequences", "Reds", "{:.1f}", "%", "", false, 100, 0}},
        {"percent_fails", {"% Failed", "Percentage of failed sequences", "Reds", "{:.1f}", "%", "", false, 100, 0}}
    };
    
    general_stats_addcols(data, headers);
}

void FastQCModule::add_quality_plot() {
    if (samples_.empty()) return;
    
    PlotConfig config;
    config.id = "per_base_quality";
    config.title = "Per Base Sequence Quality";
    config.type = PlotType::Line;
    config.xlab = "Position in Read";
    config.ylab = "Phred Quality Score";
    config.ymin = 0;
    config.ymax = 40;
    
    // 构建图表数据
    nlohmann::json data = nlohmann::json::array();
    
    for (const auto& [sample_id, sample] : samples_) {
        std::vector<int> positions;
        std::vector<double> qualities;
        
        for (const auto& [pos, qual] : sample.quality_per_base) {
            positions.push_back(pos);
            qualities.push_back(qual);
        }
        
        if (!positions.empty()) {
            nlohmann::json trace;
            trace["type"] = "scatter";
            trace["mode"] = "lines";
            trace["name"] = sample_id;
            trace["x"] = positions;
            trace["y"] = qualities;
            data.push_back(trace);
        }
    }
    
    PlotData plot;
    plot.config = config;
    plot.data = data;
    add_plot(plot);
}

void FastQCModule::add_gc_plot() {
    // TODO: 实现 GC 含量图表
    spdlog::debug("GC plot - TODO");
}

void FastQCModule::add_sequence_length_plot() {
    // TODO: 实现序列长度分布图表
    spdlog::debug("Sequence length plot - TODO");
}

// 注册 FastQC 模块
REGISTER_MODULE("fastqc", FastQCModule);

} // namespace mqc
