#include "core/module.hpp"
#include "core/file_search.hpp"
#include "core/report.hpp"
#include "core/config.hpp"
#include "cli.hpp"
#include "modules/fastqc/fastqc_module.hpp"
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <chrono>
#include <iostream>

using namespace mqc;

int main(int argc, char* argv[]) {
    // 初始化日志
    auto console = spdlog::stdout_color_mt("console");
    spdlog::set_default_logger(console);
    
    // 解析命令行
    CommandLineArgs args;
    if (!args.parse(argc, argv)) {
        return 1;
    }
    
    // 加载配置
    auto& config_mgr = ConfigManager::instance();
    if (!args.config_file.empty()) {
        config_mgr.load(args.config_file);
    } else {
        config_mgr.load();
    }
    
    // 应用命令行覆盖
    std::map<std::string, std::string> overrides;
    if (!args.filename.empty()) overrides["filename"] = args.filename;
    if (!args.title.empty()) overrides["title"] = args.title;
    if (!args.template_name.empty()) overrides["template"] = args.template_name;
    if (args.force) overrides["force"] = "true";
    if (args.verbose) overrides["verbose"] = "1";
    
    config_mgr.apply_cli_overrides(overrides);
    
    const auto& config = config_mgr.config();
    
    spdlog::info("MultiQC C++ v1.0.0");
    spdlog::info("Output: {}", config.filename);
    
    // 搜索文件
    spdlog::info("Searching directories...");
    auto start_time = std::chrono::high_resolution_clock::now();
    
    FileSearcher searcher;
    
    // 添加 FastQC 模式
    SearchPattern fastqc_pattern;
    fastqc_pattern.filename_pattern = "*_fastqc.zip";
    searcher.add_pattern("fastqc", fastqc_pattern);
    
    SearchPattern fastqc_data_pattern;
    fastqc_data_pattern.filename_pattern = "fastqc_data.txt";
    searcher.add_pattern("fastqc", fastqc_data_pattern);
    
    // 执行搜索
    auto search_results = searcher.search(args.analysis_dirs[0]);
    
    spdlog::info("Found {} files in {} directories", 
                 search_results.total_files, 
                 search_results.total_dirs);
    
    // 执行模块
    std::vector<std::unique_ptr<BaseModule>> modules;
    
    // FastQC 模块
    auto fastqc_module = ModuleRegistry::create_module("fastqc");
    if (fastqc_module && !search_results.files_by_module.empty()) {
        spdlog::info("Running FastQC module...");
        std::vector<MatchedFile> files;
        for (const auto& [sample, file_list] : search_results.files_by_module) {
            files.insert(files.end(), file_list.begin(), file_list.end());
        }
        fastqc_module->parse(files);
        modules.push_back(std::move(fastqc_module));
    }
    
    // 生成报告
    if (!args.no_report) {
        spdlog::info("Generating report...");
        
        ReportConfig report_config;
        report_config.output_fn = config.filename;
        report_config.make_data_dir = config.data_dir;
        report_config.export_plots = config.export_plots;
        
        ReportGenerator generator(report_config);
        
        // 添加模块到报告
        for (const auto& module : modules) {
            generator.add_module(*module);
        }
        
        // 生成
        auto report_path = generator.generate(args.output_dir);
        
        if (!report_path.empty()) {
            spdlog::info("Report generated: {}", report_path);
        }
    }
    
    // 打印统计
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    
    spdlog::info("Completed in {}ms", duration.count());
    spdlog::info("Processed {} modules", modules.size());
    
    return 0;
}
