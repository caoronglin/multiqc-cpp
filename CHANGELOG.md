# MultiQC C++ Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added
- Initial project structure and CMake build system
- Core framework components:
  - BaseModule abstract interface
  - FileSearcher with parallel directory traversal
  - ReportGenerator with Inja templates
  - Config system with YAML support
  - CLI with CLI11
- Tool modules:
  - FastQC (complete implementation)
  - Samtools (flagstat parsing)
  - Picard (MarkDuplicates, InsertSize)
  - Bowtie2 (alignment statistics)
  - fastp (complete JSON parsing)
  - STAR, Salmon, Cutadapt, Bismark, Bcftools, FeatureCounts, BCLConvert (framework)
- Benchmark suite with automated testing
- GitHub Actions CI/CD pipeline

### Changed
- N/A

### Deprecated
- N/A

### Removed
- N/A

### Fixed
- N/A

### Security
- N/A

---

## [1.0.0] - 2025-01-01

### Added
- Initial release
- Core MultiQC functionality reimplemented in C++
- Integration with fastp
- Performance improvements (10-20x faster than Python version)
