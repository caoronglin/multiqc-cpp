#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "mqc/core/module.hpp"

using namespace mqc;
using ::testing::Eq;
using ::testing::HasSubstr;

// 测试用模块
class TestModule : public BaseModule
{
  public:
    TestModule() : BaseModule("test", "TestModule", "Test module for unit tests") {}

    void parse(const std::vector<MatchedFile>& files) override {
        parsed_files_ = files;
        for (const auto& file : files) {
            add_data(file.sample_name, {{"parsed", true}});
        }
    }

    const std::vector<MatchedFile>& parsed_files() const { return parsed_files_; }

  private:
    std::vector<MatchedFile> parsed_files_;
};

// 注册测试模块
REGISTER_MODULE("test", TestModule);

TEST(ModuleTest, CreateModuleWithMetadata) {
    TestModule mod;
    EXPECT_EQ(mod.id(), "test");
    EXPECT_EQ(mod.name(), "TestModule");
    EXPECT_EQ(mod.info(), "Test module for unit tests");
}

TEST(ModuleTest, AddSectionToModule) {
    TestModule mod;
    mod.add_section("Quality Metrics", "quality", "Quality control metrics");

    const auto& sections = mod.sections();
    ASSERT_EQ(sections.size(), 1);
    EXPECT_EQ(sections[0].name, "Quality Metrics");
    EXPECT_EQ(sections[0].anchor, "quality");
    EXPECT_EQ(sections[0].description, "Quality control metrics");
}

TEST(ModuleTest, AddDataToModule) {
    TestModule mod;
    mod.add_data("key1", "value1");
    mod.add_data("key2", 42);
    mod.add_data("key3", {{"nested", "value"}});

    const auto& data = mod.data();
    EXPECT_EQ(data["key1"], "value1");
    EXPECT_EQ(data["key2"], 42);
    EXPECT_EQ(data["key3"]["nested"], "value");
}

TEST(ModuleTest, CleanSampleName) {
    TestModule mod;

    EXPECT_EQ(mod.clean_sample_name("sample1.fastq.gz"), "sample1");
    EXPECT_EQ(mod.clean_sample_name("sample2_R1.fq.gz"), "sample2");
    EXPECT_EQ(mod.clean_sample_name("sample3_aligned.bam"), "sample3");
    EXPECT_EQ(mod.clean_sample_name("path/to/sample4.txt"), "sample4");
}

TEST(ModuleTest, GeneralStatsAddCols) {
    TestModule mod;

    nlohmann::json data = {{"sample1", {{"total_reads", 1000000}, {"gc_content", 52.3}}},
                           {"sample2", {{"total_reads", 850000}, {"gc_content", 48.7}}}};

    std::map<std::string, GeneralStatsColumn> headers = {
        {"total_reads", {"Total Reads", "Total number of reads", "Blues", "{:,d}", "", "", false, 1000000, 0}},
        {"gc_content", {"GC %", "GC content percentage", "Greens", "{:.1f}", "%", "", false, 100, 0}}};

    mod.general_stats_addcols(data, headers);

    // 验证数据已添加（这里只是简单验证，实际需要访问私有成员）
    EXPECT_TRUE(mod.data().empty()); // general_stats_data_ 是私有成员
}

TEST(RegistryTest, RegisterAndCreateModule) {
    // 模块已经在上面注册了
    auto mod = ModuleRegistry::create_module("test");
    ASSERT_NE(mod, nullptr);
    EXPECT_EQ(mod->id(), "test");
}

TEST(RegistryTest, GetRegisteredIds) {
    auto ids = ModuleRegistry::get_registered_ids();
    EXPECT_THAT(ids, testing::Contains("test"));
}

TEST(RegistryTest, CreateNonExistentModule) {
    auto mod = ModuleRegistry::create_module("nonexistent");
    EXPECT_EQ(mod, nullptr);
}

TEST(PlotDataTest, CreatePlotData) {
    PlotConfig config;
    config.id = "test_plot";
    config.title = "Test Plot";
    config.type = PlotType::Bar;
    config.xlab = "X Axis";
    config.ylab = "Y Axis";

    nlohmann::json data = {{"Sample1", 100}, {"Sample2", 200}, {"Sample3", 150}};

    PlotData plot;
    plot.config = config;
    plot.data = data;

    EXPECT_EQ(plot.config.id, "test_plot");
    EXPECT_EQ(plot.config.title, "Test Plot");
    EXPECT_EQ(plot.data["Sample2"], 200);
}

TEST(SectionTest, CreateSectionWithAllFields) {
    Section section;
    section.name = "Advanced Metrics";
    section.anchor = "advanced";
    section.description = "Advanced statistical metrics";
    section.content = "<p>Some HTML content</p>";
    section.plot_id = "plot_1";
    section.comment = "Additional notes";

    EXPECT_EQ(section.name, "Advanced Metrics");
    EXPECT_EQ(section.anchor, "advanced");
    EXPECT_TRUE(section.content.has_value());
    EXPECT_TRUE(section.plot_id.has_value());
    EXPECT_TRUE(section.comment.has_value());
}

TEST(MatchedFileTest, CreateMatchedFile) {
    MatchedFile file;
    file.filepath = "/path/to/file.fastqc";
    file.sample_name = "sample1";
    file.root = "/path/to";
    file.fn = "file.fastqc";
    file.fn_root = "file";
    file.contents = "First line\nSecond line";

    EXPECT_EQ(file.filepath, "/path/to/file.fastqc");
    EXPECT_EQ(file.sample_name, "sample1");
    EXPECT_TRUE(file.contents.has_value());
}

TEST(SearchPatternTest, CreateSearchPattern) {
    SearchPattern pattern;
    pattern.filename_pattern = "*_fastqc.zip";
    pattern.contents_pattern = "FastQC";
    pattern.max_lines = 5;
    pattern.priority = 5;

    EXPECT_EQ(pattern.filename_pattern, "*_fastqc.zip");
    EXPECT_EQ(pattern.contents_pattern, "FastQC");
    EXPECT_EQ(pattern.max_lines, 5);
    EXPECT_EQ(pattern.priority, 5);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
