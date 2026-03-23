#include "fastp_module.hpp"
#include <spdlog/spdlog.h>
#include <fstream>
#include <nlohmann/json.hpp>

namespace mqc
{

FastpModule::FastpModule() : BaseModule("fastp", "fastp", "Ultra-fast all-in-one FASTQ preprocessor") {}

void FastpModule::parse(const std::vector<MatchedFile>& files) {
    spdlog::info("Parsing {} fastp files", files.size());

    for (const auto& file : files) {
        if (file.fn.find(".json") != std::string::npos) {
            parse_fastp_json(file.filepath, file.sample_name);
        }
    }

    spdlog::info("Parsed {} fastp samples", samples_.size());

    if (!samples_.empty()) {
        add_filtering_stats();
        add_quality_plot();
        add_base_content_plot();

        add_section("fastp Quality Control", "fastp_qc", "Quality control and filtering statistics from fastp");
        add_section("Per Read Quality", "fastp_read_quality", "Average quality scores per read position");
        add_section("Base Content", "fastp_base_content", "GC content and base distribution");
    }
}

bool FastpModule::parse_fastp_json(const std::string& filepath, const std::string& sample_name) {
    try {
        std::ifstream ifs(filepath);
        if (!ifs.is_open()) {
            spdlog::warn("Cannot open fastp JSON: {}", filepath);
            return false;
        }

        nlohmann::json json;
        ifs >> json;
        ifs.close();

        FastpData& data = samples_[sample_name];

        // 总结统计
        if (json.contains("summary")) {
            const auto& summary = json["summary"];

            if (summary.contains("before_filtering")) {
                const auto& before = summary["before_filtering"];
                data.total_reads = before.value("total_reads", 0);
                data.total_bases = before.value("total_bases", 0);
            }

            if (summary.contains("after_filtering")) {
                const auto& after = summary["after_filtering"];
                data.passed_reads = after.value("total_reads", 0);
                data.passed_bases = after.value("total_bases", 0);
            }

            if (data.total_reads > 0) {
                data.percent_passed = (double)data.passed_reads / data.total_reads * 100.0;
            }
        }

        // 质量统计
        if (json.contains("quality")) {
            const auto& quality = json["quality"];

            // Read1 质量
            if (quality.contains("read1")) {
                auto read1_quality_list = quality["read1"];
                if (read1_quality_list.is_array()) {
                    for (size_t i = 0; i < read1_quality_list.size() && i < 150; i++) {
                        if (read1_quality_list[i].is_number()) {
                            data.read1_quality[i] = read1_quality_list[i].get<double>();
                        }
                    }
                }
            }

            // Read2 质量（如果是 PE 数据）
            if (quality.contains("read2")) {
                auto read2_quality_list = quality["read2"];
                if (read2_quality_list.is_array()) {
                    for (size_t i = 0; i < read2_quality_list.size() && i < 150; i++) {
                        if (read2_quality_list[i].is_number()) {
                            data.read2_quality[i] = read2_quality_list[i].get<double>();
                        }
                    }
                }
            }
        }

        // GC 含量
        if (json.contains("gc_content")) {
            const auto& gc = json["gc_content"];

            if (gc.contains("read1")) {
                auto read1_gc_list = gc["read1"];
                if (read1_gc_list.is_array()) {
                    for (size_t i = 0; i < read1_gc_list.size() && i < 150; i++) {
                        if (read1_gc_list[i].is_number()) {
                            data.read1_gc[i] = read1_gc_list[i].get<double>();
                        }
                    }
                }
            }

            if (gc.contains("read2")) {
                auto read2_gc_list = gc["read2"];
                if (read2_gc_list.is_array()) {
                    for (size_t i = 0; i < read2_gc_list.size() && i < 150; i++) {
                        if (read2_gc_list[i].is_number()) {
                            data.read2_gc[i] = read2_gc_list[i].get<double>();
                        }
                    }
                }
            }

            // 总体 GC 含量
            if (gc.contains("total_gc_content")) {
                data.gc_content = gc["total_gc_content"].get<double>();
            }
        }

        // Q20/Q30
        if (json.contains("q20_rate")) {
            data.q20_rate = json["q20_rate"].get<double>() * 100.0;
        }
        if (json.contains("q30_rate")) {
            data.q30_rate = json["q30_rate"].get<double>() * 100.0;
        }

        spdlog::debug("Parsed fastp for {}: {} reads, {:.1f}% passed, Q30={:.1f}%", sample_name, data.total_reads,
                      data.percent_passed, data.q30_rate);

        return true;

    } catch (const std::exception& e) {
        spdlog::error("Failed to parse fastp JSON {}: {}", filepath, e.what());
        return false;
    }
}

void FastpModule::add_filtering_stats() {
    nlohmann::json data = nlohmann::json::object();
    std::map<std::string, GeneralStatsColumn> headers;

    for (const auto& [sample_id, sample] : samples_) {
        data[sample_id] = {{"total_reads", sample.total_reads},
                           {"passed_reads", sample.passed_reads},
                           {"percent_passed", sample.percent_passed},
                           {"q20_rate", sample.q20_rate},
                           {"q30_rate", sample.q30_rate},
                           {"gc_content", sample.gc_content}};
    }

    headers = {{"total_reads", {"Total Reads", "Total number of reads", "Blues", "{:,d}", "", "", false, 100000000, 0}},
               {"passed_reads",
                {"Passed Reads", "Number of reads passing filters", "Greens", "{:,d}", "", "", false, 100000000, 0}},
               {"percent_passed",
                {"% Passed", "Percentage of reads passing filters", "Greens", "{:.1f}", "%", "", false, 100, 0}},
               {"q20_rate", {"Q20", "Percentage of Q20 bases", "Blues", "{:.1f}", "%", "", false, 100, 0}},
               {"q30_rate", {"Q30", "Percentage of Q30 bases", "Blues", "{:.1f}", "%", "", false, 100, 0}},
               {"gc_content", {"GC%", "GC content percentage", "Oranges", "{:.1f}", "%", "", false, 100, 0}}};

    general_stats_addcols(data, headers);
}

void FastpModule::add_quality_plot() {
    if (samples_.empty())
        return;

    PlotConfig config;
    config.id = "fastp_per_read_quality";
    config.title = "Per Read Quality Scores";
    config.type = PlotType::Line;
    config.xlab = "Read Position";
    config.ylab = "Phred Quality Score";
    config.ymin = 0;
    config.ymax = 40;

    nlohmann::json data = nlohmann::json::array();

    for (const auto& [sample_id, sample] : samples_) {
        // Read1
        if (!sample.read1_quality.empty()) {
            std::vector<int> positions;
            std::vector<double> qualities;

            for (const auto& [pos, qual] : sample.read1_quality) {
                positions.push_back(pos);
                qualities.push_back(qual);
            }

            nlohmann::json trace;
            trace["type"] = "scatter";
            trace["mode"] = "lines";
            trace["name"] = sample_id + " (R1)";
            trace["x"] = positions;
            trace["y"] = qualities;
            data.push_back(trace);
        }

        // Read2 (PE only)
        if (!sample.read2_quality.empty()) {
            std::vector<int> positions;
            std::vector<double> qualities;

            for (const auto& [pos, qual] : sample.read2_quality) {
                positions.push_back(pos);
                qualities.push_back(qual);
            }

            nlohmann::json trace;
            trace["type"] = "scatter";
            trace["mode"] = "lines";
            trace["name"] = sample_id + " (R2)";
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

void FastpModule::add_base_content_plot() {
    // TODO: 实现 GC 含量图表
    spdlog::debug("GC content plot - TODO");
}

} // namespace mqc
