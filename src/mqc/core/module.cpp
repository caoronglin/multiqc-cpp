#include "module.hpp"
#include <fstream>
#include <algorithm>
#include <filesystem>
#include <spdlog/spdlog.h>

namespace fs = std::filesystem;

namespace mqc {

// ============ BaseModule 实现 ============

BaseModule::BaseModule(std::string id, std::string name, std::string info)
    : id_(std::move(id))
    , name_(std::move(name))
    , info_(std::move(info))
    , anchor_(id_) {
    // 转换为小写作为 anchor
    std::transform(anchor_.begin(), anchor_.end(), anchor_.begin(), ::tolower);
    
    init_sample_name_cleaning();
}

void BaseModule::init_sample_name_cleaning() {
    // 默认的样本名清理扩展名
    sample_name_clean_extensions = {
        ".gz", ".fastq", ".fq", ".bam", ".sam", ".vcf", ".bcf",
        ".txt", ".log", ".out", ".stats", ".summary",
        "_fastqc", "_trimmed", "_aligned", "_sorted"
    };
    
    // 常见的样本名替换规则
    sample_name_replacements = {
        {R"(_R[12])", ""},
        {R"(_[12])", ""},
        {R"(\.cleaned)", ""},
        {R"(\.filtered)", ""}
    };
}

std::string BaseModule::clean_sample_name(const std::string& filename) {
    std::string result = filename;
    
    // 移除目录部分
    auto pos = result.find_last_of("/\\");
    if (pos != std::string::npos) {
        result = result.substr(pos + 1);
    }
    
    // 移除已知扩展名
    for (const auto& ext : sample_name_clean_extensions) {
        if (result.size() >= ext.size()) {
            auto suffix = result.substr(result.size() - ext.size());
            std::transform(suffix.begin(), suffix.end(), suffix.begin(), ::tolower);
            if (suffix == ext) {
                result = result.substr(0, result.size() - ext.size());
                break;
            }
        }
    }
    
    // 应用替换规则
    // TODO: 使用 std::regex 实现
    // 暂时简单处理
    return result;
}

void BaseModule::add_section(const std::string& name,
                            const std::string& anchor,
                            const std::string& description,
                            const std::optional<std::string>& content,
                            const std::optional<std::string>& plot_id,
                            const std::optional<std::string>& comment) {
    Section section;
    section.name = name;
    section.anchor = anchor;
    section.description = description;
    section.content = content;
    section.plot_id = plot_id;
    section.comment = comment;
    
    sections_.push_back(std::move(section));
    
    spdlog::debug("Module {} added section: {}", id_, name);
}

void BaseModule::add_data(const std::string& key, const nlohmann::json& value) {
    data_[key] = value;
}

void BaseModule::add_plot(const PlotData& plot) {
    plots_.push_back(plot);
}

void BaseModule::general_stats_addcols(const nlohmann::json& data,
                                       const std::map<std::string, GeneralStatsColumn>& headers) {
    // 合并数据
    for (auto& [sample_id, metrics] : data.items()) {
        general_stats_data_[sample_id] = metrics;
    }
    
    // 合并表头
    for (const auto& [metric, column] : headers) {
        general_stats_headers_[metric] = column;
    }
}

void BaseModule::write_data_file(const std::string& dirname, const std::string& filename) {
    // 创建输出目录
    fs::create_directories(dirname);
    
    // 写入 TSV 格式
    auto filepath = dirname / (filename + ".tsv");
    std::ofstream ofs(filepath);
    
    if (!ofs.is_open()) {
        spdlog::error("Failed to open data file: {}", filepath.string());
        return;
    }
    
    // 写入数据
    if (general_stats_data_.is_object()) {
        // 收集所有列
        std::set<std::string> all_keys;
        for (const auto& [sample, metrics] : general_stats_data_.items()) {
            if (metrics.is_object()) {
                for (auto& [key, value] : metrics.items()) {
                    all_keys.insert(key);
                }
            }
        }
        
        // 写入表头
        ofs << "Sample\t";
        for (const auto& key : all_keys) {
            ofs << key << "\t";
        }
        ofs << "\n";
        
        // 写入数据行
        for (const auto& [sample, metrics] : general_stats_data_.items()) {
            ofs << sample << "\t";
            for (const auto& key : all_keys) {
                if (metrics.contains(key)) {
                    const auto& val = metrics[key];
                    if (val.is_string()) {
                        ofs << val.get<std::string>() << "\t";
                    } else if (val.is_number()) {
                        ofs << val.get<double>() << "\t";
                    } else {
                        ofs << val.dump() << "\t";
                    }
                } else {
                    ofs << "\t";
                }
            }
            ofs << "\n";
        }
    }
    
    spdlog::info("Data written to: {}", filepath.string());
}

// ============ ModuleRegistry 实现 ============

std::map<std::string, BaseModule::ModuleFactory>& ModuleRegistry::get_registry() {
    static std::map<std::string, ModuleFactory> registry;
    return registry;
}

void ModuleRegistry::register_module(const std::string& id, ModuleFactory factory) {
    auto& registry = get_registry();
    if (registry.contains(id)) {
        spdlog::warn("Module {} already registered, replacing", id);
    }
    registry[id] = std::move(factory);
    spdlog::info("Registered module: {}", id);
}

std::unique_ptr<BaseModule> ModuleRegistry::create_module(const std::string& id) {
    auto& registry = get_registry();
    auto it = registry.find(id);
    if (it == registry.end()) {
        spdlog::error("Module {} not found", id);
        return nullptr;
    }
    return it->second();
}

std::vector<std::string> ModuleRegistry::get_registered_ids() {
    auto& registry = get_registry();
    std::vector<std::string> ids;
    ids.reserve(registry.size());
    for (const auto& [id, factory] : registry) {
        ids.push_back(id);
    }
    return ids;
}

} // namespace mqc
