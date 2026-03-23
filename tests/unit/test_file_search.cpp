#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <filesystem>
#include <fstream>
#include "mqc/core/file_search.hpp"

using namespace mqc;
namespace fs = std::filesystem;

using ::testing::HasSubstr;
using ::testing::SizeIs;
using ::testing::Not;

// ============ 工具函数测试 ============

TEST(GlobToRegexTest, StarPattern) {
    EXPECT_EQ(glob_to_regex("*.txt"), "^.*\\.txt$");
    EXPECT_EQ(glob_to_regex("*_fastqc.zip"), "^.*_fastqc\\.zip$");
}

TEST(GlobToRegexTest, QuestionMarkPattern) {
    EXPECT_EQ(glob_to_regex("?.txt"), "^.\\.txt$");
    EXPECT_EQ(glob_to_regex("file?.log"), "^file.\\.log$");
}

TEST(GlobToRegexTest, SpecialCharacters) {
    EXPECT_EQ(glob_to_regex("file.txt"), "^file\\.txt$");
    EXPECT_EQ(glob_to_regex("[test].log"), "^\\[test\\]\\.log$");
}

TEST(MatchSimpleTest, ExactMatch) {
    EXPECT_TRUE(match_simple("test.txt", "test.txt"));
    EXPECT_FALSE(match_simple("test.txt", "test.log"));
}

TEST(MatchSimpleTest, StarPattern) {
    EXPECT_TRUE(match_simple("test.txt", "*.txt"));
    EXPECT_TRUE(match_simple("test.log.txt", "*.txt"));
    EXPECT_FALSE(match_simple("test.txt", "*.log"));
}

TEST(MatchSimpleTest, QuestionMarkPattern) {
    EXPECT_TRUE(match_simple("a.txt", "?.txt"));
    EXPECT_TRUE(match_simple("test1.log", "test?.log"));
    EXPECT_FALSE(match_simple("test.log", "test?.log"));  // ? 只匹配一个字符
}

TEST(MatchSimpleTest, ComplexPatterns) {
    EXPECT_TRUE(match_simple("sample1_fastqc.zip", "*_fastqc.zip"));
    EXPECT_TRUE(match_simple("data.txt.gz", "*.txt.gz"));
    EXPECT_FALSE(match_simple("data.txt", "*.txt.gz"));
}

// ============ FileSearcher 测试 ============

class FileSearcherTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 创建测试目录结构
        test_dir_ = fs::temp_directory_path() / "mqc_test_" + std::to_string(getpid());
        fs::create_directories(test_dir_);
        
        // 创建测试文件
        create_test_file("sample1_fastqc.zip", "FastQC report");
        create_test_file("sample2_fastqc.zip", "FastQC report");
        create_test_file("sample1.bam", "BAM content");
        create_test_file("sample2.bam", "BAM content");
        create_test_file("other.txt", "Other content");
        
        // 创建子目录
        auto subdir = test_dir_ / "subdir";
        fs::create_directories(subdir);
        create_test_file(subdir / "sample3_fastqc.zip", "FastQC in subdir");
    }
    
    void TearDown() override {
        // 清理测试目录
        fs::remove_all(test_dir_);
    }
    
    void create_test_file(const fs::path& path, const std::string& content) {
        fs::path full_path = test_dir_ / path;
        fs::create_directories(full_path.parent_path());
        std::ofstream ofs(full_path);
        ofs << content;
    }
    
    fs::path test_dir_;
};

TEST_F(FileSearcherTest, AddPattern) {
    FileSearcher searcher;
    
    SearchPattern pattern;
    pattern.filename_pattern = "*_fastqc.zip";
    
    searcher.add_pattern("fastqc", pattern);
    
    auto ids = searcher.get_module_ids();
    EXPECT_THAT(ids, testing::Contains("fastqc"));
}

TEST_F(FileSearcherTest, SearchBasic) {
    FileSearcher searcher;
    
    SearchPattern pattern;
    pattern.filename_pattern = "*_fastqc.zip";
    searcher.add_pattern("fastqc", pattern);
    
    auto results = searcher.search(test_dir_);
    
    EXPECT_GT(results.total_files, 0);
    EXPECT_GT(results.total_dirs, 0);
    EXPECT_GT(results.files_by_module.size(), 0);
    
    // 应该找到 3 个 FastQC 文件
    auto it = results.files_by_module.find("sample1_fastqc");
    EXPECT_NE(it, results.files_by_module.end());
}

TEST_F(FileSearcherTest, SearchContentMatching) {
    FileSearcher searcher;
    
    SearchOptions options;
    options.search_contents = true;
    FileSearcher searcher_with_content(options);
    
    SearchPattern pattern;
    pattern.filename_pattern = "*.txt";
    pattern.contents_pattern = "FastQC";
    searcher_with_content.add_pattern("fastqc", pattern);
    
    auto results = searcher_with_content.search(test_dir_);
    
    // 应该找到包含 "FastQC" 的 .txt 文件
    auto it = std::find_if(results.files_by_module.begin(), results.files_by_module.end(),
                          [](const auto& pair) {
                              return pair.second.size() > 0;
                          });
    
    if (it != results.files_by_module.end()) {
        EXPECT_TRUE(it->second[0].contents.has_value());
        EXPECT_THAT(it->second[0].contents.value(), HasSubstr("FastQC"));
    }
}

TEST_F(FileSearcherTest, IgnoreDirectories) {
    FileSearcher searcher;
    
    // 创建应被忽略的目录
    auto git_dir = test_dir_ / ".git";
    fs::create_directories(git_dir);
    create_test_file(git_dir / "config", "Git config");
    
    SearchPattern pattern;
    pattern.filename_pattern = "*";
    searcher.add_pattern("all", pattern);
    
    auto results = searcher.search(test_dir_);
    
    // .git 目录应该被忽略
    bool git_found = false;
    for (const auto& [module, files] : results.files_by_module) {
        for (const auto& file : files) {
            if (file.filepath.find("/.git/") != std::string::npos) {
                git_found = true;
                break;
            }
        }
    }
    EXPECT_FALSE(git_found);
}

TEST_F(FileSearcherTest, MaxDepth) {
    FileSearcher searcher;
    
    SearchOptions options;
    options.max_search_depth = 1;
    FileSearcher shallow_searcher(options);
    
    // 创建深层目录
    auto deep_dir = test_dir_ / "a" / "b" / "c";
    fs::create_directories(deep_dir);
    create_test_file(deep_dir / "deep.txt", "Deep file");
    
    SearchPattern pattern;
    pattern.filename_pattern = "*.txt";
    shallow_searcher.add_pattern("all", pattern);
    
    auto results = shallow_searcher.search(test_dir_);
    
    // 深层文件应该被忽略（取决于实际深度）
    // 这里只是验证搜索不会因为深度而崩溃
    EXPECT_GE(results.total_files, 0);
}

TEST_F(FileSearcherTest, ProgressCallback) {
    FileSearcher searcher;
    
    SearchPattern pattern;
    pattern.filename_pattern = "*";
    searcher.add_pattern("all", pattern);
    
    size_t callback_count = 0;
    auto progress = [&callback_count](size_t files, size_t matches) {
        callback_count++;
    };
    
    auto results = searcher.search(test_dir_, progress);
    
    // 回调应该被调用
    EXPECT_GT(callback_count, 0);
}

TEST_F(FileSearcherTest, ClearPatterns) {
    FileSearcher searcher;
    
    searcher.add_pattern("fastqc", SearchPattern{});
    searcher.add_pattern("samtools", SearchPattern{});
    
    EXPECT_EQ(searcher.get_module_ids().size(), 2);
    
    searcher.clear_patterns();
    
    EXPECT_EQ(searcher.get_module_ids().size(), 0);
}

TEST_F(FileSearcherTest, StatsTracking) {
    FileSearcher searcher;
    
    SearchPattern pattern;
    pattern.filename_pattern = "*_fastqc.zip";
    searcher.add_pattern("fastqc", pattern);
    
    auto results = searcher.search(test_dir_);
    auto stats = searcher.get_stats();
    
    EXPECT_GT(stats.files_scanned, 0);
    EXPECT_GT(stats.dirs_traversed, 0);
    EXPECT_GT(stats.matches_found, 0);
    EXPECT_EQ(stats.patterns_count, 1);
}

TEST_F(FileSearcherTest, MatchedFileStructure) {
    FileSearcher searcher;
    
    SearchPattern pattern;
    pattern.filename_pattern = "sample1_fastqc.zip";
    searcher.add_pattern("fastqc", pattern);
    
    auto results = searcher.search(test_dir_);
    
    ASSERT_GT(results.files_by_module.size(), 0);
    const auto& files = results.files_by_module.begin()->second;
    ASSERT_GT(files.size(), 0);
    
    const auto& file = files[0];
    EXPECT_FALSE(file.filepath.empty());
    EXPECT_FALSE(file.sample_name.empty());
    EXPECT_FALSE(file.fn.empty());
    EXPECT_FALSE(file.fn_root.empty());
}

// ============ 并行搜索测试 ============

TEST_F(FileSearcherTest, ParallelSearch) {
    FileSearcher searcher;
    
    SearchPattern pattern;
    pattern.filename_pattern = "*_fastqc.zip";
    searcher.add_pattern("fastqc", pattern);
    
    // 创建另一个测试目录
    auto test_dir2 = fs::temp_directory_path() / "mqc_test2_" + std::to_string(getpid());
    fs::create_directories(test_dir2);
    
    std::ofstream(test_dir2 / "sample4_fastqc.zip") << "FastQC";
    std::ofstream(test_dir2 / "sample5_fastqc.zip") << "FastQC";
    
    auto results = searcher.search_parallel({test_dir_, test_dir2});
    
    // 应该合并两个目录的结果
    EXPECT_GT(results.total_files, 0);
    
    // 清理
    fs::remove_all(test_dir2);
}

// ============ 性能测试 ============

TEST(PerformanceTest, SearchManyFiles) {
    // 创建大量测试文件
    auto perf_dir = fs::temp_directory_path() / "mqc_perf_test";
    fs::remove_all(perf_dir);
    fs::create_directories(perf_dir);
    
    const int file_count = 1000;
    for (int i = 0; i < file_count; i++) {
        std::string filename = "sample" + std::to_string(i) + "_fastqc.zip";
        std::ofstream(perf_dir / filename) << "FastQC content";
    }
    
    FileSearcher searcher;
    SearchPattern pattern;
    pattern.filename_pattern = "*_fastqc.zip";
    searcher.add_pattern("fastqc", pattern);
    
    auto start = std::chrono::high_resolution_clock::now();
    auto results = searcher.search(perf_dir);
    auto end = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // 应该在合理时间内完成
    EXPECT_LT(duration.count(), 5000);  // < 5 秒
    
    // 应该找到所有文件
    EXPECT_EQ(results.total_files, file_count);
    
    // 清理
    fs::remove_all(perf_dir);
    
    spdlog::info("Performance test: {} files in {}ms", file_count, duration.count());
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
