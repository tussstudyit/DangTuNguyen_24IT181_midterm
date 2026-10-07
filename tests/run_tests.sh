#!/bin/sh
# Automated Test Suite for ls(1)
# Midterm Project - Dang Tu Nguyen (24IT181)

BIN="./ls"
if [ ! -f "$BIN" ] && [ -f "./ls.exe" ]; then
    BIN="./ls.exe"
elif [ ! -f "$BIN" ] && [ ! -f "./ls.exe" ]; then
    echo "Binary not found, running make..."
    make
    if [ -f "./ls" ]; then BIN="./ls"; else BIN="./ls.exe"; fi
fi

TOTAL=0
PASSED=0
FAILED=0

test_case() {
    DESC="$1"
    CMD="$2"
    EXPECTED_STATUS="$3"

    TOTAL=$((TOTAL + 1))
    eval "$CMD" > /dev/null 2>&1
    ACTUAL_STATUS=$?

    if [ "$ACTUAL_STATUS" -eq "$EXPECTED_STATUS" ]; then
        echo "[PASS] $DESC"
        PASSED=$((PASSED + 1))
    else
        echo "[FAIL] $DESC (expected $EXPECTED_STATUS, got $ACTUAL_STATUS)"
        FAILED=$((FAILED + 1))
    fi
}

echo "==========================================="
echo "  Running ls(1) Automated Test Suite"
echo "==========================================="

test_case "1. Default listing without arguments" "$BIN" 0
test_case "2. Hidden files (-a)" "$BIN -a" 0
test_case "3. Hidden files excluding . and .. (-A)" "$BIN -A" 0
test_case "4. Long listing format (-l)" "$BIN -l" 0
test_case "5. Numeric IDs (-n)" "$BIN -n" 0
test_case "6. Override -l with -n" "$BIN -l -n" 0
test_case "7. Override -n with -l" "$BIN -n -l" 0
test_case "8. Inode numbers (-i)" "$BIN -i" 0
test_case "9. Block count (-s)" "$BIN -s" 0
test_case "10. Kilobyte blocks (-k)" "$BIN -k -s" 0
test_case "11. Human readable sizes (-h)" "$BIN -lh" 0
test_case "12. Human readable blocks (-h -s)" "$BIN -sh" 0
test_case "13. Override -k with -h" "$BIN -skh" 0
test_case "14. Override -h with -k" "$BIN -shk" 0
test_case "15. Classification indicators (-F)" "$BIN -F" 0
test_case "16. Reverse sorting (-r)" "$BIN -r" 0
test_case "17. Sort by size (-S)" "$BIN -S" 0
test_case "18. Sort by modification time (-t)" "$BIN -t" 0
test_case "19. Status change time (-c -l)" "$BIN -lc" 0
test_case "20. Access time (-u -l)" "$BIN -lu" 0
test_case "21. Override -c with -u (-lcu)" "$BIN -lcu" 0
test_case "22. Override -u with -c (-luc)" "$BIN -luc" 0
test_case "23. Plain directory listing (-d)" "$BIN -d src" 0
test_case "24. Recursive traversal (-R)" "$BIN -R src" 0
test_case "25. Override -R with -d (-Rd)" "$BIN -Rd src" 0
test_case "26. Override -d with -R (-dR)" "$BIN -dR src" 0
test_case "27. Unsorted output (-f)" "$BIN -f" 0
test_case "28. Force raw non-printable (-w)" "$BIN -w" 0
test_case "29. Force escape non-printable (-q)" "$BIN -q" 0
test_case "30. Non-directory + directory operands" "$BIN Makefile src" 0
test_case "31. Non-existent file error exit status (>0)" "$BIN nonexistent_file_xyz" 1

echo "==========================================="
echo "Test Summary: $PASSED / $TOTAL passed ($FAILED failed)"
echo "==========================================="

if [ "$FAILED" -eq 0 ]; then
    exit 0
else
    exit 1
fi
