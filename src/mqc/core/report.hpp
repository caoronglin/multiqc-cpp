#pragma once

#include "module.hpp"
#include <string>
#include <vector>
#include <map>
#include <filesystem>
#include <nlohmann/json.hpp>
#include <inja/inja.hpp>

namespace fs = std::filesystem;

namespace mqc {

/**
 * @brief 报告配置
 */
struct ReportConfig {
    std::string title = "MultiQC Report";
    std::string report_comment;      // 报告顶部说明
    std::string intro_text;          // 介绍文本
    std::string output_fn = "multiqc_report.html";
    std::string template_dir;        // 模板目录
    std::string data_dir = "multiqc_data";
    std::string plots_dir = "multiqc_plots";
    
    bool make_data_dir = true;       // 是否创建数据目录
    bool make_report = true;         // 是否生成 HTML 报告
    bool make_pdf = false;           // 是否生成 PDF
    bool export_plots = false;       // 是否导出图表
    bool make_report_interactive = true; // 交互式图表
    
    int min_compressed_bytes = 0;    // 最小压缩字节数
};

/**
 * @brief HTML 报告生成器
 * 
 * 使用 Inja 模板引擎生成自包含的 HTML 报告
 * 支持 Plotly.js 交互式图表
 */
class ReportGenerator {
public:
    /**
     * @brief 构造函数
     * @param config 报告配置
     */
    explicit ReportGenerator(ReportConfig config = ReportConfig());
    
    /**
     * @brief 添加模块到报告
     * @param module 模块实例
     */
    void add_module(const BaseModule& module);
    
    /**
     * @brief 生成 HTML 报告
     * @param output_dir 输出目录
     * @return 生成的 HTML 文件路径
     */
    std::string generate(const fs::path& output_dir);
    
    /**
     * @brief 生成数据文件
     * @param output_dir 输出目录
     */
    void write_data_files(const fs::path& output_dir);
    
    /**
     * @brief 生成图表文件
     * @param output_dir 输出目录
     */
    void write_plot_files(const fs::path& output_dir);
    
    /**
     * @brief 设置自定义模板
     * @param template_path 模板文件路径
     */
    void set_template(const fs::path& template_path);
    
    /**
     * @brief 获取 Inja 环境
     */
    inja::Environment& env() { return env_; }
    
private:
    // 渲染方法
    std::string render_header();
    std::string render_general_stats();
    std::string render_modules();
    std::string render_foot();
    
    // 图表生成
    std::string render_plot(const PlotData& plot);
    std::string render_plotly_config(const PlotData& plot);
    
    // 资源嵌入
    std::string embed_css();
    std::string embed_js();
    
    // 数据序列化
    nlohmann::json serialize_data();
    
private:
    ReportConfig config_;
    std::vector<const BaseModule*> modules_;
    inja::Environment env_;
    
    // 模板路径
    fs::path template_path_;
    
    // 默认模板（内联）
    static const char* DEFAULT_TEMPLATE;
};

/**
 * @brief Plotly 图表配置生成器
 */
class PlotlyGenerator {
public:
    /**
     * @brief 生成条形图配置
     * @param plot 图表数据
     * @return Plotly JSON 配置
     */
    static nlohmann::json bar_plot(const PlotData& plot);
    
    /**
     * @brief 生成折线图配置
     * @param plot 图表数据
     * @return Plotly JSON 配置
     */
    static nlohmann::json line_plot(const PlotData& plot);
    
    /**
     * @brief 生成散点图配置
     * @param plot 图表数据
     * @return Plotly JSON 配置
     */
    static nlohmann::json scatter_plot(const PlotData& plot);
    
    /**
     * @brief 生成热图配置
     * @param plot 图表数据
     * @return Plotly JSON 配置
     */
    static nlohmann::json heatmap_plot(const PlotData& plot);
    
    /**
     * @brief 通用 MultiQC 模板
     */
    static nlohmann::json multiqc_template();
};

} // namespace mqc
