#!/usr/bin/env bash
set -euo pipefail

# Given
binary_path=$1
fixture_path=$2
template_path=$3
test_directory=$(mktemp -d)
trap 'rm -rf "$test_directory"' EXIT
mkdir -p "$test_directory/web"
cp "$template_path" "$test_directory/web/report.html"

# When
(
    cd "$test_directory"
    "$binary_path" --html "$fixture_path"
)

# Then
[[ -s "$test_directory/build/report/index.html" ]]