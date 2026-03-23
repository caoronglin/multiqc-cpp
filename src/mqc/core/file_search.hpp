#pragma once

#include "module.hpp"
#include <vector>
#include <map>
#include <string>
#include <filesystem>
#include <optional>
#include <functional>
#include <future>

namespace fs = std::filesystem;

namespace mqc
{

/**
 * @brief 文件搜索配置
 */
struct SearchOptions
{
    bool case_sensitive = false;          // 是否区分大小写
    bool search_contents = false;         // 是否搜索文件内容
    int max_search_depth = 10;            // 最大搜索深度
    size_t max_files = 100000;            // 最大文件数
    std::vector<std::string> ignore_dirs; // 忽略的目录名

    SearchOptions() { ignore_dirs = {".git", ".svn", ".hg", "node_modules", "__pycache__", ".cache"}; }
};

/**
 * @brief 文件搜索结果
 */
struct SearchResults
{
    std::map<std::string, std::vector<MatchedFile>> files_by_module;
    size_t total_files = 0;
    size_t total_dirs = 0;
    std::chrono::milliseconds search_time{0};
};

/**
 * @brief 高性能文件搜索引擎
 *
 * 特性:
 * - 并行目录遍历
 * - Glob 模式匹配
 * - 内容匹配（可选）
 * - 优先级排序
 * - 缓存支持
 */
class FileSearcher
{
  public:
    using ProgressCallback = std::function<void(size_t files_scanned, size_t matches_found)>;

    /**
     * @brief 构造函数
     * @param options 搜索配置
     */
    explicit FileSearcher(SearchOptions options = SearchOptions());

    /**
     * @brief 添加搜索模式
     * @param module_id 模块 ID（如 "fastqc"）
     * @param pattern 搜索模式
     */
    void add_pattern(const std::string& module_id, const SearchPattern& pattern);

    /**
     * @brief 搜索目录
     * @param dir 搜索的目录
     * @param progress 进度回调
     * @return 搜索结果
     */
    SearchResults search(const fs::path& dir, ProgressCallback progress = nullptr);

    /**
     * @brief 并行搜索多个目录
     * @param dirs 搜索目录列表
     * @param progress 进度回调
     * @return 合并的搜索结果
     */
    SearchResults search_parallel(const std::vector<fs::path>& dirs, ProgressCallback progress = nullptr);

    /**
     * @brief 清空所有搜索模式
     */
    void clear_patterns();

    /**
     * @brief 获取已注册的模块 ID 列表
     */
    std::vector<std::string> get_module_ids() const;

    /**
     * @brief 获取搜索统计信息
     */
    struct Stats
    {
        size_t patterns_count = 0;
        size_t files_scanned = 0;
        size_t matches_found = 0;
        size_t dirs_traversed = 0;
    };

    Stats get_stats() const { return stats_; }

  private:
    // 模式匹配
    bool match_filename(const fs::path& filepath, const std::string& pattern) const;
    bool match_contents(const fs::path& filepath, const std::string& pattern, std::optional<int> max_lines,
                        std::string& matched_content) const;

    // 目录遍历
    void traverse_directory(const fs::path& dir, SearchResults& results, ProgressCallback progress);

    // 并行遍历
    std::vector<std::future<SearchResults>> traverse_parallel(const std::vector<fs::path>& dirs);

    // 合并结果
    static SearchResults merge_results(const std::vector<SearchResults>& results);

    // 文件匹配
    std::optional<MatchedFile> match_file(const fs::path& filepath, const fs::path& root_dir);

  private:
    // 搜索模式：module_id -> pattern 列表
    std::map<std::string, std::vector<SearchPattern>> patterns_;

    // 搜索配置
    SearchOptions options_;

    // 统计信息
    Stats stats_;

    // Glob 模式缓存（编译后的正则）
    mutable std::map<std::string, std::string> glob_cache_;
};

/**
 * @brief Glob 模式转正则表达式
 * @param glob Glob 模式（如 "*_fastqc.zip"）
 * @return 正则表达式
 */
std::string glob_to_regex(const std::string& glob);

/**
 * @brief 简单的文件名匹配（不使用正则）
 * @param filename 文件名
 * @param pattern 模式
 * @return 是否匹配
 */
bool match_simple(const std::string& filename, const std::string& pattern);

} // namespace mqc
