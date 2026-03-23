#include "picard_module.hpp"
#include <spdlog/spdlog.h>
#include <fstream>
#include <sstream>

namespace mqc {

PicardModule::PicardModule()
    : BaseModule("picard", "Picard", "Picard tools for manipulating high-throughput sequencing data") {
}

void PicardModule::parse(const std::vector<MatchedFile>& files) {
    spdlog::info("Parsing {} Picard files", files.size());
    
    for (const auto& file : files) {
        if (file.fn.find("mark_duplicates") != std::string::npos || 
            file.fn.find("metrics") != std::string::npos) {
            parse_mark_duplicates(file.filepath, file.sample_name);
        }
        
        if (file.fn.find("insert_size") != std::string::npos) {
            parse_insert_size(file.filepath, file.sample_name);
        }
    }
    
    if (!samples_.empty()) {
        // 添加到通用统计表
        nlohmann::json data = nlohmann::json::object();
        std::map<std::string, GeneralStatsColumn> headers;
        
        for (const auto& [sample_id, sample] : samples_) {
            data[sample_id] = {
                {"read_pairs", sample.read_pairs},
                {"unpaired_reads", sample.unpaired_reads},
                {"duplicates", sample.duplicates},
                {"percent_duplicates", sample.percent_duplicates},
                {"estimated_library_size", sample.estimated_library_size},
                {"median_insert_size", sample.median_insert_size},
                {"mean_insert_size", sample.mean_insert_size}
            };
        }
        
        headers = {
            {"read_pairs", {"Read Pairs", "Number of read pairs", "Blues", "{:,d}", "", "", false, 100000000, 0}},
            {"duplicates", {"Duplicates", "Number of duplicate reads", "Reds", "{:,d}", "", "", false, 100000000, 0}},
            {"percent_duplicates", {"% Duplicates", "Percentage of duplicates", "Reds", "{:.1f}", "%", "", false, 100, 0}},
            {"estimated_library_size", {"Est. Library", "Estimated library size", "Purples", "{:,d}", "", "", false, 1000000000, 0}},
            {"median_insert_size", {"Median Insert", "Median insert size", "Oranges", "{:.0f}", " bp", "", false, 1000, 0}},
            {"mean_insert_size", {"Mean Insert", "Mean insert size", "Oranges", "{:.1f}", " bp", "", false, 1000, 0}}
        };
        
        general_stats_addcols(data, headers);
        
        add_section("Picard Metrics", "picard_metrics", "Metrics from Picard tools");
    }
}

bool PicardModule::parse_mark_duplicates(const std::string& filepath, const std::string& sample_name) {
    std::ifstream ifs(filepath);
    if (!ifs.is_open()) {
        spdlog::warn("Cannot open Picard metrics file: {}", filepath);
        return false;
    }
    
    PicardData& data = samples_[sample_name];
    
    std::string line;
    bool in_data_section = false;
    
    while (std::getline(ifs, line)) {
        // 跳过注释和空行
        if (line.empty() || line[0] == '#') continue;
        
        // 检测 METRICS 部分
        if (line.find("METRICS") != std::string::npos) {
            in_data_section = true;
            continue;
        }
        
        if (!in_data_section) continue;
        
        // 解析数据行
        std::istringstream iss(line);
        std::vector<std::string> fields;
        std::string field;
        
        while (std::getline(iss, field, '\t')) {
            fields.push_back(field);
        }
        
        if (fields.size() >= 8) {
            try {
                data.read_pairs = std::stoull(fields[0]);
                data.unpaired_reads = std::stoull(fields[1]);
                data.duplicates = std::stoull(fields[2]);
                data.percent_duplicates = std::stod(fields[3]);
                data.estimated_library_size = std::stoull(fields[4]);
                
                spdlog::debug("Parsed Picard MarkDuplicates for {}: {} pairs, {:.1f}% duplicates",
                             sample_name, data.read_pairs, data.percent_duplicates);
                return true;
            } catch (const std::exception& e) {
                spdlog::warn("Failed to parse Picard metrics line: {}", line);
                return false;
            }
        }
    }
    
    return false;
}

bool PicardModule::parse_insert_size(const std::string& filepath, const std::string& sample_name) {
    std::ifstream ifs(filepath);
    if (!ifs.is_open()) {
        spdlog::warn("Cannot open insert size metrics file: {}", filepath);
        return false;
    }
    
    PicardData& data = samples_[sample_name];
    
    std::string line;
    bool in_data_section = false;
    
    while (std::getline(ifs, line)) {
        if (line.find("MEDIAN_INSERT_SIZE") != std::string::npos) {
            in_data_section = true;
            continue;
        }
        
        if (!in_data_section) continue;
        
        // 解析数据行
        std::istringstream iss(line);
        std::vector<std::string> fields;
        std::string field;
        
        while (std::getline(iss, field, '\t')) {
            fields.push_back(field);
        }
        
        if (fields.size() >= 6) {
            try {
                data.median_insert_size = std::stod(fields[0]);
                data.mean_insert_size = std::stod(fields[1]);
                data.std_dev_insert_size = std::stod(fields[2]);
                
                spdlog::debug("Parsed insert size for {}: median={:.0f}, mean={:.1f}",
                             sample_name, data.median_insert_size, data.mean_insert_size);
                return true;
            } catch (const std::exception& e) {
                spdlog::warn("Failed to parse insert size metrics: {}", line);
                return false;
            }
        }
    }
    
    return false;
}

} // namespace mqc
