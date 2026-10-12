#!/usr/bin/env bash
set -euo pipefail

repo_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
work_dir="$(mktemp -d)"
trap 'rm -rf "$work_dir"' EXIT

g++ -std=c++17 -Wall -Wextra -Wpedantic "$repo_dir/run_pipeline.cpp" \
    -o "$work_dir/run_pipeline"

output_file="$work_dir/unites_lexicales.txt"
(cd "$repo_dir" && "$work_dir/run_pipeline" TP1/program.txt "$output_file")

test -s "$output_file"
grep -q '^1        BEGIN' "$output_file"
printf 'Programme racine TP1 -> TP2 : OK\n'
