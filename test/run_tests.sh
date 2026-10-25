#!/bin/sh
# Generates the fully synthetic, fabricated test dataset (ships in this repo,
# no external/private data needed) and runs the compiled test harness
# against it, plus Part 2's self-contained edge-case checks. Run from this
# directory.
set -e
python3 generate_synthetic_streams.py
g++ -std=c++17 -O2 -Wall -Wextra -o /tmp/ariston_passive_test_harness test_harness.cpp ../firmware/core/ariston_passive_core.cpp
/tmp/ariston_passive_test_harness
