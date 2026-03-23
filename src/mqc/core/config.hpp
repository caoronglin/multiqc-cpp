#pragma once

#include <string>
#include <map>
#include <vector>
#include <optional>
#include <nlohmann/json.hpp>
#include <yaml-cpp/yaml.h>

namespace mqc {

/**
 * @brief MultiQC 配置
 */
struct Config {
    // 输出设置
    std::string filename = "multiqc_report.html";
    std::string title = "MultiQC Report";
    std::string report_comment;
    std::string intro_text;
    std::string template_name = "default";
    
    // 目录
    std::string data_dir = "multiqc_data";
    std::string plots_dir = "multiqc_plots";
    std::string export_plots_dir;
    
    // 布尔标志
    bool force = false;
    bool config = false;
    bool cl_config = false;
    bool data_dir = true;
    bool plots_dir = false;
    bool export_plots = false;
    bool make_pdf = false;
    bool no_report = false;
    bool no_data_file = false;
    
    // 搜索和过滤
    std::vector<std::string> ignore_samples;
    std::vector<std::string> ignore_symlinks;
    std::string file_list;
    bool require_logs = false;
    
    // 模块设置
    std::map<std::string, bool> module_order;  // module_id -> enabled
    std::map<std::string, nlohmann::json> module_config;
    
    // 样本名替换
    std::vector<std::pair<std::string, std::string>> sample_names_rename;
    
    // 其他
    int verbose = 1;  // 0=quiet, 1=normal, 2=debug
    std::string color_list;
    
    /**
     * @brief 从 YAML 文件加载配置
     */
    static std::optional<Config> load_from_file(const std::string& path);
    
    /**
     * @brief 从 JSON 加载配置
     */
    static std::optional<Config> load_from_json(const nlohmann::json& json);
    
    /**
     * @brief 保存配置到文件
     */
    bool save_to_file(const std::string& path) const;
    
    /**
     * @brief 转换为 JSON
     */
    nlohmann::json to_json() const;
};

/**
 * @brief 配置管理器
 */
class ConfigManager {
public:
    /**
     * @brief 获取单例实例
     */
    static ConfigManager& instance();
    
    /**
     * @brief 加载配置文件
     * @param path 配置文件路径（空则使用默认路径）
     * @return 是否成功
     */
    bool load(const std::string& path = "");
    
    /**
     * @brief 获取当前配置
     */
    const Config& config() const { return config_; }
    
    /**
     * @brief 获取可写配置
     */
    Config& config() { return config_; }
    
    /**
     * @brief 合并命令行覆盖
     */
    void apply_cli_overrides(const std::map<std::string, std::string>& overrides);
    
private:
    ConfigManager() = default;
    Config config_;
    
    // 默认配置路径
    static const std::vector<std::string>& get_default_paths();
};

} // namespace mqc
