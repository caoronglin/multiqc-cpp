#pragma once
#include "mqc/core/module.hpp"
namespace mqc
{
class Bowtie2Module : public BaseModule
{
  public:
    Bowtie2Module();
    void parse(const std::vector<MatchedFile>& files) override;

  private:
    struct Bowtie2Data
    {
        size_t total_reads = 0;
        size_t aligned = 0;
        double percent_aligned = 0.0;
        size_t multimapped = 0;
    };
    std::map<std::string, Bowtie2Data> samples_;
};
REGISTER_MODULE("bowtie2", Bowtie2Module);
} // namespace mqc
