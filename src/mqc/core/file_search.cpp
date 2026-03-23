#include "file_search.hpp"
#include <spdlog/spdlog.h>
#include <algorithm>
#include <regex>
#include <fstream>
#include <thread>

namespace fs = std::filesystem;

namespace mqc {

// ============ 工具函数 ============

std::string glob_to_regex(const std::string& glob) {
    std::string regex = "^";
    for (char c : glob) {
        switch (c) {
            case '*': regex += ".*"; break;
            case '?': regex += "."; break;
            case '.': regex += "\\."; break;
            case '[': regex += "\\["; break;
            case ']': regex += "\\]"; break;
            case '(': regex += "\\("; break;
            case ')': regex += "\\)"; break;
            case '{': regex += "\\("; break;
            case '}': regex += "\\)"; break;
            case ',': regex += "|"; break;
            default: regex += c;
        }
    }
    regex += "$";
    return regex;
}

bool match_simple(const std::string& filename, const std::string& pattern) {
    // 简单实现：处理 * 和 ?
    size_t pi = 0, fi = 0;
    size_t star_idx = std::string::npos, match_idx = 0;
    
    while (fi < filename.size()) {
        if (pi < pattern.size() && (pattern[pi] == '?' || pattern[pi] == filename[pi])) {
            ++pi;
            ++fi;
        } else if (pi < pattern.size() && pattern[pi] == '*') {
            star_idx = pi;
            match_idx = fi;
            ++pi;
        } else if (star_idx != std::string::npos) {
            pi = star_idx + 1;
            match_idx++;
            fi = match_idx;
        } else {
            return false;
        }
    }
    
    while (pi < pattern.size() && pattern[pi] == '*') {
        ++pi;
    }
    
    return pi == pattern.size();
}

// ============ FileSearcher 实现 ============

FileSearcher::FileSearcher(SearchOptions options)
    : options_(std::move(options)) {
}

void FileSearcher::add_pattern(const std::string& module_id, const SearchPattern& pattern) {
    if (!pattern.filename_pattern.has_value() && !pattern.contents_pattern.has_value()) {
        spdlog::warn("Pattern for module {} has neither filename nor contents pattern", module_id);
        return;
    }
    
    patterns_[module_id].push_back(pattern);
    stats_.patterns_count++;
    
    spdlog::debug("Added pattern for module {}: {}", module_id, 
                  pattern.filename_pattern.value_or("*"));
}

void FileSearcher::clear_patterns() {
    patterns_.clear();
    stats_ = Stats{};
}

std::vector<std::string> FileSearcher::get_module_ids() const {
    std::vector<std::string> ids;
    ids.reserve(patterns_.size());
    for (const auto& [id, patterns] : patterns_) {
        ids.push_back(id);
    }
    return ids;
}

FileSearcher::SearchResults FileSearcher::search(const fs::path& dir, ProgressCallback progress) {
    SearchResults results;
    auto start_time = std::chrono::high_resolution_clock::now();
    
    if (!fs::exists(dir) || !fs::is_directory(dir)) {
        spdlog::error("Directory does not exist: {}", dir.string());
        return results;
    }
    
    traverse_directory(dir, results, progress);
    
    auto end_time = std::chrono::high_resolution_clock::now();
    results.search_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    
    spdlog::info("Search completed in {}ms: {} files, {} dirs, {} matches",
                 results.search_time.count(),
                 stats_.files_scanned,
                 stats_.dirs_traversed,
                 stats_.matches_found);
    
    return results;
}

void FileSearcher::traverse_directory(const fs::path& dir, 
                                      SearchResults& results,
                                      ProgressCallback progress) {
    // 使用递归目录遍历
    std::error_code ec;
    for (const auto& entry : fs::recursive_directory_iterator(dir, 
                                                              fs::directory_options::skip_permission_denied, 
                                                              ec)) {
        if (ec) {
            spdlog::debug("Permission denied: {}", entry.path().string());
            continue;
        }
        
        // 检查深度限制
        auto depth = std::distance(dir.begin(), entry.path().begin());
        if (depth > options_.max_search_depth) {
            continue;
        }
        
        if (entry.is_directory()) {
            results.total_dirs++;
            stats_.dirs_traversed++;
            
            // 检查是否忽略的目录
            auto dirname = entry.path().filename().string();
            if (std::find(options_.ignore_dirs.begin(), options_.ignore_dirs.end(), dirname) 
                != options_.ignore_dirs.end()) {
                spdlog::debug("Skipping ignored directory: {}", entry.path().string());
                continue;
            }
        } else if (entry.is_regular_file()) {
            results.total_files++;
            stats_.files_scanned++;
            
            // 尝试匹配
            auto matched_file = match_file(entry.path(), dir);
            if (matched_file.has_value()) {
                results.files_by_module[matched_file->sample_name].push_back(matched_file.value());
                stats_.matches_found++;
            }
            
            // 进度回调
            if (progress && stats_.files_scanned % 1000 == 0) {
                progress(stats_.files_scanned, stats_.matches_found);
            }
            
            // 检查文件数限制
            if (stats_.files_scanned >= options_.max_files) {
                spdlog::warn("Reached max files limit: {}", options_.max_files);
                break;
            }
        }
    }
}

std::optional<MatchedFile> FileSearcher::match_file(const fs::path& filepath,
                                                     const fs::path& root_dir) {
    auto filename = filepath.filename().string();
    
    // 遍历所有模块的模式
    for (const auto& [module_id, module_patterns] : patterns_) {
        for (const auto& pattern : module_patterns) {
            // 文件名匹配
            if (pattern.filename_pattern.has_value()) {
                if (!match_simple(filename, pattern.filename_pattern.value())) {
                    continue;
                }
            }
            
            // 内容匹配（如果需要）
            std::optional<std::string> contents;
            if (pattern.contents_pattern.has_value()) {
                std::string matched_content;
                if (!match_contents(filepath, pattern.contents_pattern.value(),
                                   pattern.max_lines, matched_content)) {
                    continue;
                }
                contents = matched_content;
            }
            
            // 匹配成功，构建 MatchedFile
            MatchedFile file;
            file.filepath = filepath.string();
            file.root = root_dir.string();
            file.fn = filename;
            file.fn_root = filename.substr(0, filename.find_last_of('.'));
            file.sample_name = file.fn_root; // 后续会清理
            file.contents = contents;
            
            return file;
        }
    }
    
    return std::nullopt;
}

bool FileSearcher::match_contents(const fs::path& filepath, const std::string& pattern,
                                   std::optional<int> max_lines, std::string& matched_content) const {
    std::ifstream ifs(filepath, std::ios::binary);
    if (!ifs.is_open()) {
        return false;
    }
    
    std::string line;
    int lines_read = 0;
    int max = max_lines.value_or(100);
    
    while (std::getline(ifs, line) && lines_read < max) {
        lines_read++;
        
        // 简单子串匹配
        if (line.find(pattern) != std::string::npos) {
            matched_content = line;
            return true;
        }
    }
    
    return false;
}

FileSearcher::SearchResults FileSearcher::search_parallel(
    const std::vector<fs::path>& dirs, ProgressCallback progress) {
    
    // 并行遍历多个目录
    std::vector<std::future<SearchResults>> futures;
    futures.reserve(dirs.size());
    
    for (const auto& dir : dirs) {
        futures.push_back(std::async(std::launch::async, [this, dir]() {
            SearchResults results;
            this->traverse_directory(dir, results, nullptr);
            return results;
        }));
    }
    
    // 收集结果
    std::vector<SearchResults> results_vec;
    results_vec.reserve(dirs.size());
    
    for (auto& future : futures) {
        results_vec.push_back(future.get());
    }
    
    // 合并结果
    return merge_results(results_vec);
}

FileSearcher::SearchResults FileSearcher::merge_results(const std::vector<SearchResults>& results) {
    SearchResults merged;
    
    for (const auto& result : results) {
        merged.total_files += result.total_files;
        merged.total_dirs += result.total_dirs;
        merged.search_time += result.search_time;
        
        for (const auto& [module_id, files] : result.files_by_module) {
            merged.files_by_module[module_id].insert(
                merged.files_by_module[module_id].end(),
                files.begin(), files.end());
        }
    }
    
    return merged;
}

} // namespace mqc
