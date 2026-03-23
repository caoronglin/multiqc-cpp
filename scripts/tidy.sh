#!/bin/bash
# Static analysis script using clang-tidy
# Usage: ./scripts/tidy.sh [--fix]

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="$PROJECT_ROOT/build/debug"
FIX_MODE=false

# Parse arguments
if [[ "$1" == "--fix" || "$1" == "-f" ]]; then
    FIX_MODE=true
fi

echo "=== C++ Static Analysis ==="
echo "Project root: $PROJECT_ROOT"
echo ""

# Check if compile_commands.json exists
if [ ! -f "$BUILD_DIR/compile_commands.json" ]; then
    echo "⚠️  compile_commands.json not found."
    echo "   Running cmake to generate build files..."
    mkdir -p "$BUILD_DIR"
    cmake -S "$PROJECT_ROOT" -B "$BUILD_DIR" -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
fi

echo "Using compile_commands.json from: $BUILD_DIR"
echo ""

# Find all source files (not headers)
FILES=$(find "$PROJECT_ROOT/src" "$PROJECT_ROOT/tests" \
    -type f -name "*.cpp" 2>/dev/null)

FILE_COUNT=$(echo "$FILES" | grep -c . || echo "0")
echo "Found $FILE_COUNT files to analyze"
echo ""

if [ "$FIX_MODE" = true ]; then
    echo "Running clang-tidy with auto-fix..."
    echo "$FILES" | xargs -I {} clang-tidy {} -p "$BUILD_DIR" --fix
    echo ""
    echo "✅ Analysis and fixes complete!"
else
    echo "Running clang-tidy (check only)..."
    echo "$FILES" | xargs -I {} clang-tidy {} -p "$BUILD_DIR" 2>&1 || {
        echo ""
        echo "⚠️  Issues found. Run with --fix to auto-fix some issues."
        exit 1
    }
    echo ""
    echo "✅ No issues found!"
fi