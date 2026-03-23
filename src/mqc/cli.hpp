#pragma once

#include "core/config.hpp"
#include <CLI/CLI.hpp>
#include <filesystem>
#include <vector>
#include <string>

namespace fs = std::filesystem;

namespace mqc
{

/**
 * @brief MultiQC 命令行参数
 */
struct CommandLineArgs
{
    // 位置参数
    std::vector<fs::path> analysis_dirs; // 要分析的目录

    // 输出选项
    std::string output_dir = ".";
    std::string filename;
    std::string title;
    std::string template_name;

    // 布尔标志
    bool force = false;
    bool overwrite = false;
    bool make_data_dir = true;
    bool make_plots = false;
    bool export_plots = false;
    bool make_pdf = false;
    bool no_report = false;
    bool verbose = false;
    bool quiet = false;

    // 过滤选项
    std::vector<std::string> ignore_samples;
    std::vector<std::string> ignore_symlinks;
    std::string file_list;

    // 配置
    std::string config_file;
    std::vector<std::string> cl_config; // 命令行配置覆盖

    // fastp 子命令参数
    struct FastpArgs
    {
        std::string input1;  // R1 input
        std::string input2;  // R2 input (PE)
        std::string output1; // R1 output
        std::string output2; // R2 output
        std::string json_report;
        std::string html_report;
        int compression = 2;
        int threads = 1;
        bool dedup = false;
        bool cut_front = false;
        bool cut_tail = false;
        bool cut_sliding = false;
        int cut_front_window_size = 4;
        int cut_front_mean_quality = 20;
    } fastp;

    /**
     * @brief 解析命令行参数
     * @param argc 参数数量
     * @param argv 参数数组
     * @return 是否成功
     */
    bool parse(int argc, char* argv[]);

    /**
     * @brief 打印帮助信息
     */
    void print_help();

  private:
    CLI::App app{"MultiQC C++ - Aggregate bioinformatics QC results"};

    void setup_main_command();
    void setup_fastp_command();
};

} // namespace mqc
