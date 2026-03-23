#!/bin/bash
# Code formatting script using clang-format
# Usage: ./scripts/format.sh [--check]

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
CHECK_ONLY=false

# Parse arguments
if [[ "$1" == "--check" || "$1" == "-c" ]]; then
    CHECK_ONLY=true
fi

echo "=== C++ Code Formatting ==="
echo "Project root: $PROJECT_ROOT"
echo ""

# Find all C++ files
FILES=$(find "$PROJECT_ROOT/src" "$PROJECT_ROOT/tests" "$PROJECT_ROOT/benchmarks" \
    -type f \( -name "*.cpp" -o -name "*.hpp" -o -name "*.h" -o -name "*.cc" -o -name "*.hxx" \) 2>/dev/null)

FILE_COUNT=$(echo "$FILES" | grep -c . || echo "0")
echo "Found $FILE_COUNT files to format"
echo ""

if [ "$CHECK_ONLY" = true ]; then
    echo "Checking format (dry-run)..."
    echo "$FILES" | xargs clang-format --dry-run --Werror 2>&1 && echo "✅ All files formatted correctly!" || {
        echo "❌ Some files need formatting. Run without --check to fix."
        exit 1
    }
else
    echo "Formatting files..."
    echo "$FILES" | while read -r file; do
        echo "  Formatting: $(basename "$file")"
        clang-format -i "$file"
    done
    echo ""
    echo "✅ Formatting complete!"
fi