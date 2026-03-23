#pragma once

#include <string>
#include <vector>
#include <map>
#include <optional>
#include <memory>
#include <filesystem>
#include <nlohmann/json.hpp>

namespace mqc {

/**
 * @brief 匹配的日志文件信息
 */
struct MatchedFile {
    std::string filepath;
    std::string sample_name;
    std::string root;
    std::string fn;
    std::string fn_root;
    std::optional<std::string> contents;  // 文件内容的前几行（用于内容匹配）
};

/**
 * @brief 搜索模式配置
 */
struct SearchPattern {
    std::optional<std::string> filename_pattern;  // 文件名 glob 模式
    std::optional<std::string> contents_pattern;  // 内容匹配模式
    std::optional<int> max_lines;                  // 读取的最大行数
    int priority = 10;                              // 优先级（越小优先级越高）
};

/**
 * @brief 报告章节
 */
struct Section {
    std::string name;          // 章节名称
    std::string anchor;        // HTML anchor ID
    std::string description;   // 章节描述
    std::optional<std::string> content;    // HTML 内容
    std::optional<std::string> plot_id;    // 关联的图表 ID
    std::optional<std::string> comment;    // 附加说明
};

/**
 * @brief 图表数据类型
 */
enum class PlotType {
    Bar,
    Line,
    Scatter,
    Heatmap,
    Violin,
    Box,
    Pie,
    Table
};

/**
 * @brief 图表配置
 */
struct PlotConfig {
    std::string id;
    std::string title;
    PlotType type;
    std::string xlab;           // X 轴标签
    std::string ylab;           // Y 轴标签
    std::string xsuffix;        // X 轴单位
    std::string ysuffix;        // Y 轴单位
    std::string xformat;        // X 轴格式化
    std::string yformat;        // Y 轴格式化
    double ymax = 0.0;          // Y 轴最大值（0=自动）
    double ymin = 0.0;          // Y 轴最小值
    bool log_scale = false;     // 对数刻度
    std::vector<std::string> colors;  // 颜色方案
};

/**
 * @brief 图表数据
 */
struct PlotData {
    PlotConfig config;
    nlohmann::json data;  // 图表数据（JSON 格式）
};

/**
 * @brief 通用统计表列
 */
struct GeneralStatsColumn {
    std::string title;
    std::string description;
    std::string scale;      // 颜色刻度
    std::string format;     // 数字格式化
    std::string suffix;
    std::string prefix;
    bool hidden = false;
    int max = 100;          // 最大值（用于颜色刻度）
    int min = 0;            // 最小值
};

/**
 * @brief 模块基类 - 所有工具模块的抽象接口
 * 
 * 使用方式：
 * 1. 继承 BaseModule
 * 2. 实现 parse() 方法
 * 3. 调用 add_section() 添加报告章节
 * 4. 调用 general_stats_addcols() 添加到通用统计表
 */
class BaseModule {
public:
    /**
     * @brief 构造函数
     * @param id 模块 ID（如 "fastqc"）
     * @param name 工具名称（如 "FastQC"）
     * @param info 工具描述
     */
    BaseModule(std::string id, std::string name, std::string info);
    
    virtual ~BaseModule() = default;
    
    /**
     * @brief 解析日志文件（纯虚函数，子类必须实现）
     * @param files 匹配的文件列表
     */
    virtual void parse(const std::vector<MatchedFile>& files) = 0;
    
    // ============ Getters ============
    std::string id() const { return id_; }
    std::string name() const { return name_; }
    std::string info() const { return info_; }
    const std::vector<Section>& sections() const { return sections_; }
    const nlohmann::json& data() const { return data_; }
    
    // ============ 文件查找 ============
    /**
     * @brief 查找匹配的日志文件
     * @param pattern_key 搜索模式键（如 "fastqc/data"）
     * @return 匹配的文件列表
     */
    std::vector<MatchedFile> find_log_files(const std::string& pattern_key);
    
    /**
     * @brief 清理样本名
     * @param filename 原始文件名
     * @return 清理后的样本名
     */
    std::string clean_sample_name(const std::string& filename);
    
    // ============ 章节管理 ============
    /**
     * @brief 添加报告章节
     * @param name 章节名称
     * @param anchor Anchor ID
     * @param description 章节描述
     * @param content HTML 内容（可选）
     * @param plot_id 关联的图表 ID（可选）
     * @param comment 附加说明（可选）
     */
    void add_section(const std::string& name,
                    const std::string& anchor,
                    const std::string& description,
                    const std::optional<std::string>& content = std::nullopt,
                    const std::optional<std::string>& plot_id = std::nullopt,
                    const std::optional<std::string>& comment = std::nullopt);
    
    // ============ 数据管理 ============
    /**
     * @brief 添加模块数据
     * @param key 数据键
     * @param value 数据值（JSON）
     */
    void add_data(const std::string& key, const nlohmann::json& value);
    
    /**
     * @brief 添加到通用统计表
     * @param data 样本数据 {sample_name: {metric: value}}
     * @param headers 列配置 {metric: GeneralStatsColumn}
     */
    void general_stats_addcols(const nlohmann::json& data,
                               const std::map<std::string, GeneralStatsColumn>& headers);
    
    // ============ 图表 ============
    /**
     * @brief 添加图表
     * @param plot 图表数据
     */
    void add_plot(const PlotData& plot);
    
    /**
     * @brief 获取所有图表
     */
    const std::vector<PlotData>& plots() const { return plots_; }
    
    // ============ 写入数据文件 ============
    /**
     * @brief 写入模块数据到文件
     * @param dirname 输出目录名
     * @param filename 输出文件名（不含扩展名）
     */
    void write_data_file(const std::string& dirname, const std::string& filename);
    
protected:
    std::string id_;
    std::string name_;
    std::string info_;
    std::string anchor_;
    
    std::vector<Section> sections_;
    std::vector<PlotData> plots_;
    nlohmann::json data_;
    nlohmann::json general_stats_data_;
    std::map<std::string, GeneralStatsColumn> general_stats_headers_;
    
    // 样本名清理配置
    std::vector<std::string> sample_name_clean_extensions;
    std::vector<std::pair<std::string, std::string>> sample_name_replacements;
    
private:
    // 辅助方法
    void init_sample_name_cleaning();
};

/**
 * @brief 模块注册表 - 管理所有可用模块
 */
class ModuleRegistry {
public:
    using ModuleFactory = std::function<std::unique_ptr<BaseModule>()>;
    
    /**
     * @brief 注册模块工厂函数
     * @param id 模块 ID
     * @param factory 工厂函数
     */
    static void register_module(const std::string& id, ModuleFactory factory);
    
    /**
     * @brief 创建模块实例
     * @param id 模块 ID
     * @return 模块实例（如果不存在返回 nullptr）
     */
    static std::unique_ptr<BaseModule> create_module(const std::string& id);
    
    /**
     * @brief 获取所有已注册的模块 ID
     */
    static std::vector<std::string> get_registered_ids();
    
private:
    static std::map<std::string, ModuleFactory>& get_registry();
};

/**
 * @brief 模块注册宏
 * 
 * 使用示例：
 * class FastQCModule : public BaseModule { ... };
 * REGISTER_MODULE(fastqc, FastQCModule);
 */
#define REGISTER_MODULE(ID, CLASS) \
    static bool CLASS##_registered = []() { \
        ModuleRegistry::register_module(ID, []() { \
            return std::make_unique<CLASS>(); \
        }); \
        return true; \
    }();

} // namespace mqc
