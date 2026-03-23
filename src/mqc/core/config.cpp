#include "config.hpp"
#include <spdlog/spdlog.h>
#include <fstream>
#include <filesystem>

namespace fs = std::filesystem;

namespace mqc {

const std::vector<std::string>& ConfigManager::get_default_paths() {
    static std::vector<std::string> paths = {
        "multiqc_config.yaml",
        "multiqc_config.yml",
        "~/.multiqc_config.yaml",
        "/etc/multiqc_config.yaml"
    };
    return paths;
}

std::optional<Config> Config::load_from_file(const std::string& path) {
    try {
        YAML::Node yaml = YAML::LoadFile(path);
        
        Config config;
        
        // 输出设置
        if (yaml["output_fn_name"]) config.filename = yaml["output_fn_name"].as<std::string>();
        if (yaml["title"]) config.title = yaml["title"].as<std::string>();
        if (yaml["report_comment"]) config.report_comment = yaml["report_comment"].as<std::string>();
        if (yaml["intro_text"]) config.intro_text = yaml["intro_text"].as<std::string>();
        if (yaml["template"]) config.template_name = yaml["template"].as<std::string>();
        
        // 目录
        if (yaml["data_dir_name"]) config.data_dir = yaml["data_dir_name"].as<std::string>();
        if (yaml["plots_dir_name"]) config.plots_dir = yaml["plots_dir_name"].as<std::string>();
        
        // 布尔标志
        if (yaml["force"]) config.force = yaml["force"].as<bool>();
        if (yaml["data_dir"]) config.data_dir = yaml["data_dir"].as<bool>();
        if (yaml["export_plots"]) config.export_plots = yaml["export_plots"].as<bool>();
        
        // 样本名替换
        if (yaml["sample_names_rename"]) {
            for (const auto& rule : yaml["sample_names_rename"]) {
                if (rule.IsSequence() && rule.size() == 2) {
                    config.sample_names_rename.emplace_back(
                        rule[0].as<std::string>(),
                        rule[1].as<std::string>()
                    );
                }
            }
        }
        
        // 模块设置
        if (yaml["module_order"]) {
            for (const auto& mod : yaml["module_order"]) {
                config.module_order[mod.first.as<std::string>()] = true;
            }
        }
        
        // 忽略的样本
        if (yaml["ignore_samples"]) {
            for (const auto& sample : yaml["ignore_samples"]) {
                config.ignore_samples.push_back(sample.as<std::string>());
            }
        }
        
        spdlog::info("Loaded config from: {}", path);
        return config;
        
    } catch (const std::exception& e) {
        spdlog::error("Failed to load config from {}: {}", path, e.what());
        return std::nullopt;
    }
}

std::optional<Config> Config::load_from_json(const nlohmann::json& json) {
    try {
        Config config;
        
        if (json.contains("filename")) config.filename = json["filename"];
        if (json.contains("title")) config.title = json["title"];
        if (json.contains("force")) config.force = json["force"];
        if (json.contains("export_plots")) config.export_plots = json["export_plots"];
        
        return config;
    } catch (const std::exception& e) {
        spdlog::error("Failed to load config from JSON: {}", e.what());
        return std::nullopt;
    }
}

bool Config::save_to_file(const std::string& path) const {
    try {
        YAML::Node yaml;
        
        yaml["output_fn_name"] = filename;
        yaml["title"] = title;
        yaml["template"] = template_name;
        yaml["data_dir_name"] = data_dir;
        yaml["plots_dir_name"] = plots_dir;
        yaml["force"] = force;
        yaml["export_plots"] = export_plots;
        
        if (!report_comment.empty()) {
            yaml["report_comment"] = report_comment;
        }
        
        if (!sample_names_rename.empty()) {
            YAML::Node rename_list = YAML::Node(YAML::NodeType::Sequence);
            for (const auto& [from, to] : sample_names_rename) {
                YAML::Node rule = YAML::Node(YAML::NodeType::Sequence);
                rule.push_back(from);
                rule.push_back(to);
                rename_list.push_back(rule);
            }
            yaml["sample_names_rename"] = rename_list;
        }
        
        std::ofstream ofs(path);
        if (!ofs.is_open()) {
            spdlog::error("Cannot open config file for writing: {}", path);
            return false;
        }
        ofs << yaml;
        ofs.close();
        
        spdlog::info("Saved config to: {}", path);
        return true;
        
    } catch (const std::exception& e) {
        spdlog::error("Failed to save config to {}: {}", path, e.what());
        return false;
    }
}

nlohmann::json Config::to_json() const {
    nlohmann::json json;
    json["filename"] = filename;
    json["title"] = title;
    json["template"] = template_name;
    json["data_dir"] = data_dir;
    json["plots_dir"] = plots_dir;
    json["force"] = force;
    json["export_plots"] = export_plots;
    json["verbose"] = verbose;
    
    if (!report_comment.empty()) {
        json["report_comment"] = report_comment;
    }
    
    return json;
}

ConfigManager& ConfigManager::instance() {
    static ConfigManager instance;
    return instance;
}

bool ConfigManager::load(const std::string& path) {
    std::optional<Config> loaded;
    
    if (!path.empty()) {
        // 加载指定路径
        loaded = Config::load_from_file(path);
    } else {
        // 尝试默认路径
        for (const auto& default_path : get_default_paths()) {
            if (fs::exists(default_path)) {
                loaded = Config::load_from_file(default_path);
                if (loaded.has_value()) {
                    break;
                }
            }
        }
    }
    
    if (loaded.has_value()) {
        config_ = loaded.value();
        spdlog::debug("Configuration loaded successfully");
        return true;
    }
    
    spdlog::debug("No configuration file found, using defaults");
    return false;
}

void ConfigManager::apply_cli_overrides(const std::map<std::string, std::string>& overrides) {
    for (const auto& [key, value] : overrides) {
        if (key == "filename" || key == "output") {
            config_.filename = value;
        } else if (key == "title") {
            config_.title = value;
        } else if (key == "template") {
            config_.template_name = value;
        } else if (key == "force") {
            config_.force = (value == "true" || value == "1");
        } else if (key == "verbose") {
            config_.verbose = std::stoi(value);
        }
        // 可以添加更多覆盖选项
    }
}

} // namespace mqc
