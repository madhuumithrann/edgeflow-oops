#!/usr/bin/env bash
# End-to-end tests of the menu program: feeds scripted stdin and checks the output.
# Usage:  bash tests/run_cli_tests.sh ./edgeflow
#         bash tests/run_cli_tests.sh ./edgeflow_sanitized     (a non-zero exit also catches leaks)
# On Windows run it from Git Bash and pass ./edgeflow.exe

BIN="${1:-./edgeflow}"
PASS=0
FAIL=0

# run NAME INPUT  -> sets OUT and RC
run() {
    NAME="$1"
    OUT="$(printf "$2" | "$BIN" 2>&1)"
    RC=$?
}

# expect_contains FIXED_STRING
has() { grep -qF -- "$1" <<<"$OUT"; }
count() { grep -cF -- "$1" <<<"$OUT"; }

ok()   { PASS=$((PASS+1)); echo "PASS  $NAME"; }
bad()  { FAIL=$((FAIL+1)); echo "FAIL  $NAME  ($1)"; }

check() {   # check "description" condition-exit-status
    if [ "$2" -eq 0 ]; then ok; else bad "$1"; fi
}

if [ ! -x "$BIN" ]; then echo "binary $BIN not found - build it first"; exit 2; fi

# --- start / exit / menu ---
run "starts and exits normally" '6\n'
has "EDGEFLOW" && has "Exiting EdgeFlow. Goodbye." && [ $RC -eq 0 ]; check "rc=$RC" $?

run "menu lists all six options" '6\n'
has "1. Add Edge Server" && has "2. Submit Request" && has "3. View Server Status" && has "4. View Load Metrics" && has "5. Compare Metrics" && has "6. Exit"; check "menu text" $?

run "invalid menu input does not crash" 'abc\n9\n0\n-1\n\n 3x \n99999999999\n6\n'
[ "$(count 'Invalid choice')" -eq 7 ] && [ $RC -eq 0 ]; check "expected 7 invalid-choice messages, got $(count 'Invalid choice'), rc=$RC" $?

run "EOF at the menu exits cleanly" ''
has "End of input reached" && [ $RC -eq 0 ]; check "rc=$RC" $?

# --- add server ---
run "add one server" '1\n1\n10\n3\n6\n'
has "Server 1 added with capacity 10." && has "Capacity: 10"; check "add/view" $?

run "add multiple servers" '1\n1\n10\n1\n2\n20\n1\n3\n30\n3\n6\n'
has "EdgeServer ID: 3" && [ "$(count 'added with capacity')" -eq 3 ]; check "3 adds" $?

run "duplicate server id rejected, state unchanged" '1\n1\n10\n1\n1\n99\n3\n6\n'
has "Server ID 1 already exists" && has "Capacity: 10" && ! has "Capacity: 99"; check "duplicate" $?

run "zero capacity rejected" '1\n1\n0\n3\n6\n'
has "Capacity must be positive" && has "No servers added."; check "zero cap" $?

run "negative capacity rejected" '1\n1\n-5\n3\n6\n'
has "Capacity must be positive" && has "No servers added."; check "neg cap" $?

run "non-numeric server id" '1\nabc\n10\n3\n6\n'
has "server ID must be an integer" && has "No servers added."; check "bad id" $?

run "non-numeric / malformed capacity" '1\n1\n12abc\n1\n2\n1.5\n1\n3\n\n3\n6\n'
[ "$(count 'capacity must be an integer')" -eq 3 ] && has "No servers added."; check "bad cap" $?

run "integer overflow input rejected" '1\n99999999999\n10\n6\n'
has "server ID must be an integer"; check "overflow id" $?

run "EOF in the middle of adding a server" '1\n5\n'
has "End of input reached" && [ $RC -eq 0 ]; check "rc=$RC" $?

# --- submit request ---
run "request with zero servers is rejected, program continues" '2\n5\n2\n6\n6\n'
[ "$(count 'REJECTED (no servers available)')" -eq 2 ] && has "Request #1" && has "Request #2" && [ $RC -eq 0 ]; check "no servers" $?

run "invalid workloads" '1\n1\n10\n2\n0\n2\n-3\n2\nabc\n2\n1.5\n2\n\n4\n6\n'
has "Workload must be positive" && [ "$(count 'Workload must be positive')" -eq 2 ] && [ "$(count 'workload must be an integer')" -eq 3 ] && has "Total valid requests submitted: 0"; check "bad workload" $?

run "exact fit accepted, then oversize rejected" '1\n1\n10\n2\n10\n2\n1\n6\n'
has "Request #1 (workload: 10) -> ASSIGNED to server 1" && has "Request #2 (workload: 1) -> REJECTED (no server has enough remaining capacity)"; check "fit/full" $?

run "Round Robin order over three servers" '1\n1\n10\n1\n2\n10\n1\n3\n10\n2\n1\n2\n1\n2\n1\n2\n1\n2\n1\n2\n1\n6\n'
ORDER="$(grep -o 'ASSIGNED to server [0-9]*' <<<"$OUT" | grep -o '[0-9]*$' | tr '\n' ' ')"
[ "$ORDER" = "1 2 3 1 2 3 " ]; check "order was: $ORDER" $?

run "full server is skipped (caps 2,10,10, workload 2)" '1\n1\n2\n1\n2\n10\n1\n3\n10\n2\n2\n2\n2\n2\n2\n2\n2\n2\n2\n2\n2\n2\n2\n6\n'
ORDER="$(grep -o 'ASSIGNED to server [0-9]*' <<<"$OUT" | grep -o '[0-9]*$' | tr '\n' ' ')"
[ "$ORDER" = "1 2 3 2 3 2 3 " ]; check "order was: $ORDER" $?

run "rejection does not change load, later requests still work" '1\n1\n5\n2\n9\n2\n5\n2\n1\n2\n4\n3\n6\n'
has "Request #1 (workload: 9) -> REJECTED" && has "Request #2 (workload: 5) -> ASSIGNED to server 1" && has "Current Load: 5"; check "reject then continue" $?

# --- view server status / metrics ---
run "view status with no servers" '3\n6\n'
has "No servers added."; check "empty status" $?

run "metrics with no servers" '4\n6\n'
has "Total valid requests submitted: 0" && has "not available" && [ $RC -eq 0 ]; check "empty metrics" $?

run "metrics counters and utilization" '1\n1\n10\n1\n2\n5\n2\n7\n2\n3\n2\n9\n4\n6\n'
has "Total valid requests submitted: 3" && has "Total accepted requests: 2" && has "Total rejected requests: 1" \
  && has "Server 1: accepted 1, load 7/10, utilization 70.00%" && has "Server 2: accepted 1, load 3/5, utilization 60.00%" \
  && has "Overall utilization: 66.67%" && has "Most-loaded server (by utilization %): 1" && has "Least-loaded server (by utilization %): 2"; check "metrics values" $?

run "most/least loaded use percent, not raw load" '1\n1\n10\n1\n2\n100\n2\n5\n2\n10\n4\n6\n'
has "Most-loaded server (by utilization %): 1" && has "Least-loaded server (by utilization %): 2"; check "percent compare" $?

# --- compare metrics ---
run "compare: first use saves a baseline" '5\n6\n'
has "Baseline snapshot saved." && [ $RC -eq 0 ]; check "baseline" $?

run "compare: second use compares real snapshots" '1\n1\n10\n5\n2\n5\n5\n6\n'
has "baseline == current (equal utilization): false" && has "current  >  baseline (higher utilization): true" && has "baseline >  current  (higher utilization): false" && has "pooled" ; check "operators" $?

# --- long / repeated session ---
SEQ='1\n1\n50\n1\n2\n50\n'
for i in $(seq 1 40); do SEQ="${SEQ}2\n2\n"; done
SEQ="${SEQ}4\n6\n"
run "40 consecutive requests" "$SEQ"
has "Total valid requests submitted: 40" && has "Total accepted requests: 40" && has "Overall utilization: 80.00%" && [ $RC -eq 0 ]; check "stress session" $?

echo
echo "CLI tests: $PASS passed, $FAIL failed"
[ "$FAIL" -eq 0 ]
