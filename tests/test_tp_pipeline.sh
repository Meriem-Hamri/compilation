#!/usr/bin/env bash
set -euo pipefail

repo_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
work_dir="$(mktemp -d)"
trap 'rm -rf "$work_dir"' EXIT

gcc -std=c11 -Wall -Wextra -Wpedantic "$repo_dir/TP1/tp1.c" -o "$work_dir/tp1"
gcc -std=c11 -Wall -Wextra -Wpedantic "$repo_dir/TP2/tp2.c" -o "$work_dir/tp2"

"$work_dir/tp1" "$repo_dir/TP1/program.txt" "$work_dir/unites_lexicales.txt"
"$work_dir/tp2" "$work_dir/unites_lexicales.txt"

"$work_dir/tp1" "$repo_dir/TP2/program.txt" "$work_dir/unites_lexicales_invalides.txt"
if "$work_dir/tp2" "$work_dir/unites_lexicales_invalides.txt"; then
    printf 'Le TP2 aurait du refuser le programme invalide\n' >&2
    exit 1
fi

printf 'Pipeline TP1 -> TP2 : OK\n'
