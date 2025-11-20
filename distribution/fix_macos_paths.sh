#!/bin/bash

# Get the directory this script is located in
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

echo "🔍 Searching for *.framework in: $SCRIPT_DIR"
echo

# Find all .framework binaries
find "$SCRIPT_DIR" -type d -name "*.framework" | while read -r FRAMEWORK_PATH; do
    FRAMEWORK_NAME=$(basename "$FRAMEWORK_PATH" .framework)
    BINARY_PATH="$FRAMEWORK_PATH/$FRAMEWORK_NAME"

    if [[ -f "$BINARY_PATH" ]]; then
        echo "➕ Adding rpath to: $BINARY_PATH"
        install_name_tool -add_rpath "@loader_path/../.." "$BINARY_PATH"
        echo "✅ Done"
    else
        echo "⚠️  Skipping $FRAMEWORK_PATH — binary not found at expected path: $BINARY_PATH"
    fi

    echo
done

echo "🔍 Searching for *.dylib in: $SCRIPT_DIR/plugins"
echo

# Find all .dylib files inside ./plugins
find "$SCRIPT_DIR/plugins" -type f -name "*.dylib" 2>/dev/null | while read -r DYLIB_PATH; do
    echo "➕ Adding rpath to: $DYLIB_PATH"
    install_name_tool -add_rpath "@loader_path/../.." "$DYLIB_PATH"
    echo "✅ Done"
    echo
done

