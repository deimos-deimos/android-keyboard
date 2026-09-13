#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")"
CXX=${CXX:-$(command -v clang++ || command -v g++)}
"$CXX" -std=c++17 -O1 -Wall -I ../../src/ggml keyboardlm_text_test.cpp -o keyboardlm_text_test
./keyboardlm_text_test
