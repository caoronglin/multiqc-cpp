#include "cli.hpp"
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <iostream>

namespace mqc
{

bool CommandLineArgs::parse(int argc, char* argv[]) {
    app.set_help_flag();
    app.footer("For more information: https://github.com/caoronglin/multiqc-cpp");

    // 主命令
    setup_main_command();

    // fastp 子命令
    setup_fastp_command();

    CLI11_PARSE(app, argc, argv);

    // 设置日志级别
    if (quiet) {
        spdlog::set_level(spdlog::level::err);
    } else if (verbose) {
        spdlog::set_level(spdlog::level::debug);
    } else {
        spdlog::set_level(spdlog::level::info);
    }

    return true;
}

void CommandLineArgs::setup_main_command() {
    // 位置参数
    app.add_option("analysis_dirs", analysis_dirs,
                   "Search these directories for QC results (default: current directory)")
        ->check(CLI::ExistingDirectory);

    // 输出选项
    app.add_option("-o,--outdir", output_dir, "Create report in this directory");
    app.add_option("-n,--filename", filename, "Report filename (default: multiqc_report.html)");
    app.add_option("--title", title, "Report title");
    app.add_option("--template", template_name, "Template name");

    // 布尔标志
    app.add_flag("-f,--force", force, "Overwrite existing report");
    app.add_flag("--overwrite", overwrite, "Force overwrite");
    app.add_flag("--data-dir", make_data_dir, "Export data directory");
    app.add_flag("--plots-dir", make_plots, "Export plots directory");
    app.add_flag("--export-plots", export_plots, "Export standalone plots");
    app.add_flag("--pdf", make_pdf, "Create PDF report");
    app.add_flag("--no-report", no_report, "Skip HTML report");
    app.add_flag("-v,--verbose", verbose, "Verbose output");
    app.add_flag("-q,--quiet", quiet, "Quiet mode");

    // 过滤选项
    app.add_option("--ignore-samples", ignore_samples, "Ignore sample names (comma-separated)");
    app.add_option("--ignore-symlinks", ignore_symlinks, "Ignore symlink directories");
    app.add_option("--file-list", file_list, "File with list of input files");

    // 配置
    app.add_option("-c,--config", config_file, "Configuration file (YAML)");
    app.add_option("--cl-config", cl_config, "Command-line config overrides (key=value)");

    // 帮助
    app.add_flag("-h,--help", "Print this help message")->group("")->trigger_on_parse()->callback([&]() {
        std::cout << app.help();
        exit(0);
    });

    // 版本
    app.set_version_flag("--version", "1.0.0");
}

void CommandLineArgs::setup_fastp_command() {
    auto* fastp_cmd = app.add_subcommand("fastp", "FASTQ processor and quality control");

    fastp_cmd->add_option("-i,--in1", fastp.input1, "Input R1 file (required)")->required()->check(CLI::ExistingFile);

    fastp_cmd->add_option("-I,--in2", fastp.input2, "Input R2 file (PE data)")->check(CLI::ExistingFile);

    fastp_cmd->add_option("-o,--out1", fastp.output1, "Output R1 file");
    fastp_cmd->add_option("-O,--out2", fastp.output2, "Output R2 file");

    fastp_cmd->add_option("--json", fastp.json_report, "JSON report filename");
    fastp_cmd->add_option("--html", fastp.html_report, "HTML report filename");

    fastp_cmd->add_option("-z,--compression", fastp.compression, "Compression level (1-9)", 2);
    fastp_cmd->add_option("-w,--threads", fastp.threads, "Worker threads", 1);

    fastp_cmd->add_flag("--dedup", fastp.dedup, "Enable deduplication");
    fastp_cmd->add_flag("--cut_front", fastp.cut_front, "Cut sliding window from front");
    fastp_cmd->add_flag("--cut_tail", fastp.cut_tail, "Cut sliding window from tail");
    fastp_cmd->add_flag("--cut_sliding", fastp.cut_sliding, "Cut using sliding window");

    fastp_cmd->add_option("--cut_front_window_size", fastp.cut_front_window_size, "Window size for front cutting", 4);
    fastp_cmd->add_option("--cut_front_mean_quality", fastp.cut_front_mean_quality, "Mean quality threshold", 20);
}

void CommandLineArgs::print_help() { std::cout << app.help() << std::endl; }

} // namespace mqc
