# PowerShell Test Suite for ls(1)
# Midterm Project - Dang Tu Nguyen (24IT181)

$bin = ".\ls.exe"
if (-not (Test-Path $bin)) {
    if (Test-Path ".\ls") {
        $bin = ".\ls"
    } else {
        Write-Host "Binary not found, running make..." -ForegroundColor Yellow
        make
        if (Test-Path ".\ls.exe") { $bin = ".\ls.exe" }
        elseif (Test-Path ".\ls") { $bin = ".\ls" }
    }
}

$total = 0
$passed = 0
$failed = 0

function Run-TestCase($desc, $argsList, $expectedStatus) {
    $script:total++
    & $bin $argsList *>$null
    $code = $LASTEXITCODE

    if ($code -eq $expectedStatus) {
        Write-Host "[PASS] $desc" -ForegroundColor Green
        $script:passed++
    } else {
        Write-Host "[FAIL] $desc (expected $expectedStatus, got $code)" -ForegroundColor Red
        $script:failed++
    }
}

Write-Host "===========================================" -ForegroundColor Cyan
Write-Host "  Running ls(1) Automated Test Suite" -ForegroundColor Cyan
Write-Host "===========================================" -ForegroundColor Cyan

Run-TestCase "1. Default listing without arguments" @() 0
Run-TestCase "2. Hidden files (-a)" @("-a") 0
Run-TestCase "3. Hidden files excluding . and .. (-A)" @("-A") 0
Run-TestCase "4. Long listing format (-l)" @("-l") 0
Run-TestCase "5. Numeric IDs (-n)" @("-n") 0
Run-TestCase "6. Override -l with -n" @("-l", "-n") 0
Run-TestCase "7. Override -n with -l" @("-n", "-l") 0
Run-TestCase "8. Inode numbers (-i)" @("-i") 0
Run-TestCase "9. Block count (-s)" @("-s") 0
Run-TestCase "10. Kilobyte blocks (-k)" @("-k", "-s") 0
Run-TestCase "11. Human readable sizes (-h)" @("-lh") 0
Run-TestCase "12. Human readable blocks (-h -s)" @("-sh") 0
Run-TestCase "13. Override -k with -h" @("-skh") 0
Run-TestCase "14. Override -h with -k" @("-shk") 0
Run-TestCase "15. Classification indicators (-F)" @("-F") 0
Run-TestCase "16. Reverse sorting (-r)" @("-r") 0
Run-TestCase "17. Sort by size (-S)" @("-S") 0
Run-TestCase "18. Sort by modification time (-t)" @("-t") 0
Run-TestCase "19. Status change time (-c -l)" @("-lc") 0
Run-TestCase "20. Access time (-u -l)" @("-lu") 0
Run-TestCase "21. Override -c with -u (-lcu)" @("-lcu") 0
Run-TestCase "22. Override -u with -c (-luc)" @("-luc") 0
Run-TestCase "23. Plain directory listing (-d)" @("-d", "src") 0
Run-TestCase "24. Recursive traversal (-R)" @("-R", "src") 0
Run-TestCase "25. Override -R with -d (-Rd)" @("-Rd", "src") 0
Run-TestCase "26. Override -d with -R (-dR)" @("-dR", "src") 0
Run-TestCase "27. Unsorted output (-f)" @("-f") 0
Run-TestCase "28. Force raw non-printable (-w)" @("-w") 0
Run-TestCase "29. Force escape non-printable (-q)" @("-q") 0
Run-TestCase "30. Non-directory + directory operands" @("Makefile", "src") 0
Run-TestCase "31. Non-existent file error exit status (>0)" @("nonexistent_file_xyz") 1

$color = if ($failed -eq 0) { "Green" } else { "Red" }
Write-Host "Test Summary: $passed / $total passed ($failed failed)" -ForegroundColor $color
Write-Host "===========================================" -ForegroundColor Cyan

if ($failed -gt 0) { exit 1 } else { exit 0 }
