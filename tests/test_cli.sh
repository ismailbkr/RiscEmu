#!/bin/sh
set -eu

FIXTURE_DIR="build/fixtures"
mkdir -p "$FIXTURE_DIR"
./build/generate_examples "$FIXTURE_DIR" >/dev/null

output=$(./build/rv32emu --max-steps 10 "$FIXTURE_DIR/addi_ecall.bin")
printf '%s\n' "$output" | grep -q 'Stopped: processor halted'
printf '%s\n' "$output" | grep -q 'x1 =0x00000005'

output=$(./build/rv32emu --max-steps 100 "$FIXTURE_DIR/sum_loop.bin")
printf '%s\n' "$output" | grep -q 'instructions=33'
printf '%s\n' "$output" | grep -q 'x1 =0x00000037'

if ./build/rv32emu --max-steps ' -1' "$FIXTURE_DIR/addi_ecall.bin" >/dev/null 2>&1; then
    echo 'Whitespace-prefixed negative step limit was accepted' >&2
    exit 1
fi
if ./build/rv32emu --max-steps 0 "$FIXTURE_DIR/addi_ecall.bin" >/dev/null 2>&1; then
    echo 'Zero step limit was accepted' >&2
    exit 1
fi
if ./build/rv32emu --max-steps 18446744073709551616 "$FIXTURE_DIR/addi_ecall.bin" >/dev/null 2>&1; then
    echo 'Overflowing step limit was accepted' >&2
    exit 1
fi

missing_output=$(./build/rv32emu "$FIXTURE_DIR/missing.bin" 2>&1 || true)
printf '%s\n' "$missing_output" | grep -q 'input/output error'

echo '[PASS] CLI raw loading and argument validation'
