#include "bowtie2_module.hpp"
#include <spdlog/spdlog.h>
#include <fstream>
#include <regex>
namespace mqc {
Bowtie2Module::Bowtie2Module() : BaseModule("bowtie2", "Bowtie 2", "Bowtie 2 is an ultrafast and memory-efficient tool for aligning sequencing reads") {}
void Bowtie2Module::parse(const std::vector<MatchedFile>& files) {
    spdlog::info("Parsing {} Bowtie2 files", files.size());
    for (const auto& file : files) {
        std::ifstream ifs(file.filepath);
        if (!ifs.is_open()) continue;
        Bowtie2Data& data = samples_[file.sample_name];
        std::string line;
        std::regex total_re(R"((\d+) reads; of these:)");
        std::regex aligned_re(R"(overall alignment rate: ([\d.]+)%)");
        while (std::getline(ifs, line)) {
            std::smatch match;
            if (std::regex_search(line, match, total_re) && match.size() > 1) {
                data.total_reads = std::stoull(match[1].str());
            } else if (std::regex_search(line, match, aligned_re) && match.size() > 1) {
                data.percent_aligned = std::stod(match[1].str());
                data.aligned = data.total_reads * data.percent_aligned / 100.0;
            }
        }
    }
    if (!samples_.empty()) {
        nlohmann::json data = nlohmann::json::object();
        std::map<std::string, GeneralStatsColumn> headers = {
            {"total_reads", {"Total Reads", "Total reads", "Blues", "{:,d}", "", "", false, 100000000, 0}},
            {"aligned", {"Aligned", "Aligned reads", "Greens", "{:,d}", "", "", false, 100000000, 0}},
            {"percent_aligned", {"% Aligned", "Alignment rate", "Greens", "{:.1f}", "%", "", false, 100, 0}}
        };
        for (const auto& [sample_id, sample] : samples_) {
            data[sample_id] = {{"total_reads", sample.total_reads}, {"aligned", sample.aligned}, {"percent_aligned", sample.percent_aligned}};
        }
        general_stats_addcols(data, headers);
        add_section("Bowtie 2 Alignment", "bowtie2", "Alignment statistics from Bowtie 2");
    }
}
}
