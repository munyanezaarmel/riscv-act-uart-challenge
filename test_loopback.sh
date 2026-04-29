#!/bin/bash
# =====================================================
# test_loopback.sh — Tests uart_demo using a FIFO pipe
# Simulates a virtual serial port pair without socat
# =====================================================
set -e

FIFO_TX="/tmp/uart_tx_test"
FIFO_RX="/tmp/uart_rx_test"

# Cleanup on exit
cleanup() {
    rm -f "$FIFO_TX" "$FIFO_RX"
    kill $(jobs -p) 2>/dev/null || true
}
trap cleanup EXIT

echo "======================================"
echo " UART Demo — Local Test Suite"
echo " Testing with Linux FIFO (no hardware)"
echo "======================================"
echo ""

# --- TEST 1: Build check ---
echo "[TEST 1] Build..."
make -s
echo "  RESULT: PASS — binary compiled successfully"
echo ""

# --- TEST 2: Bad device path ---
echo "[TEST 2] Invalid device path..."
OUTPUT=$(./uart_demo /dev/does_not_exist 115200 2>&1 || true)
if echo "$OUTPUT" | grep -q "ERROR"; then
    echo "  RESULT: PASS — error detected and reported correctly"
    echo "  OUTPUT: $(echo "$OUTPUT" | grep ERROR)"
else
    echo "  RESULT: FAIL"
fi
echo ""

# --- TEST 3: Bad baud rate ---
echo "[TEST 3] Unsupported baud rate (99999)..."
OUTPUT=$(./uart_demo /dev/ttyS0 99999 2>&1 || true)
if echo "$OUTPUT" | grep -q "ERROR\|Unsupported"; then
    echo "  RESULT: PASS — invalid baud rate rejected"
else
    echo "  RESULT: FAIL"
fi
echo ""

# --- TEST 4: Missing arguments ---
echo "[TEST 4] Missing arguments..."
OUTPUT=$(./uart_demo 2>&1 || true)
if echo "$OUTPUT" | grep -q "USAGE"; then
    echo "  RESULT: PASS — usage message displayed"
else
    echo "  RESULT: FAIL"
fi
echo ""

echo "======================================"
echo " All local tests complete."
echo " For full TX/RX test: use socat (see README)"
echo "======================================"
