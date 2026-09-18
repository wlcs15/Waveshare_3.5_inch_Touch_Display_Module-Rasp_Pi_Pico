# Pico W + quality tools on Windows 11. Does not install anything.
#   powershell -NoProfile -ExecutionPolicy Bypass -File scripts\check_tools.ps1
$ErrorActionPreference = "Continue"
$Root = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
Set-Location $Root

if (-not $env:CHECK_TOOLS_INNER) {
    $localDir = Join-Path $Root "local"
    New-Item -ItemType Directory -Force -Path $localDir | Out-Null
    $ts = Get-Date -Format "yyyyMMdd-HHmmss"
    $hn = ($env:COMPUTERNAME -replace "[^A-Za-z0-9._-]", "_")
    $log = Join-Path $localDir "check_tools-$ts-$hn.log"
    $last = Join-Path $localDir "check_tools-last.log"
    $header = @(
        "=== check_tools log (share this file with Grok; Grok CLI not required) ==="
        "file: $log"
        "time: $(Get-Date -Format o)"
        "host: $env:COMPUTERNAME"
        "os: $([Environment]::OSVersion.VersionString)"
        "ps: $($PSVersionTable.PSVersion)"
        "user: $env:USERNAME"
        "repo: $Root"
        ""
    ) -join [Environment]::NewLine
    Set-Content -Path $log -Value $header -Encoding UTF8
    $env:CHECK_TOOLS_INNER = "1"
    & powershell.exe -NoProfile -ExecutionPolicy Bypass -File $PSCommandPath @args *>&1 |
        Tee-Object -FilePath $log -Append
    $rc = $LASTEXITCODE
    if ($null -eq $rc) { $rc = 0 }
    Copy-Item -LiteralPath $log -Destination $last -Force
    Write-Host ""
    Write-Host "Share this file with Grok (no Grok CLI needed):"
    Write-Host "  $log"
    Write-Host "  $last"
    exit $rc
}

$script:MissingReq = 0
$script:MissingOpt = 0
function Write-Ok($n, $d) { Write-Host ("  OK       {0,-18} {1}" -f $n, $d) }
function Write-Fail($n, $d) { Write-Host ("  MISSING  {0,-18} {1}" -f $n, $d); $script:MissingReq = 1 }
function Write-Warn($n, $d) { Write-Host ("  WARN     {0,-18} {1}" -f $n, $d); $script:MissingOpt = 1 }
function Has-Cmd($n) { [bool](Get-Command $n -ErrorAction SilentlyContinue) }

Write-Host "Required tools check (Windows)  repo: $Root"
Write-Host ""
Write-Host "=== Host Unity + quality (required) ==="
foreach ($c in @("git", "cmake", "clang", "clang++")) {
    $g = Get-Command $c -ErrorAction SilentlyContinue
    if ($g) { Write-Ok $c $g.Source } else { Write-Fail $c "install $c (LLVM Clang, not MinGW)" }
}
$py = Get-Command python -ErrorAction SilentlyContinue
if (-not $py) { $py = Get-Command python3 -ErrorAction SilentlyContinue }
if ($py) { Write-Ok python $py.Source } else { Write-Fail python "Python 3" }
if (Has-Cmd ninja) { Write-Ok ninja ((Get-Command ninja).Source) }
elseif (Has-Cmd nmake) { Write-Ok nmake "NMake Makefiles" }
else { Write-Fail generator "ninja or VS nmake" }
$unity = Join-Path $Root "third_party\Unity\src\unity.c"
if (Test-Path $unity) { Write-Ok Unity $unity } else { Write-Fail Unity $unity }

Write-Host ""
Write-Host "=== Pico W firmware (required) ==="
$sdk = $env:PICO_SDK_PATH
if (-not $sdk -or -not (Test-Path (Join-Path $sdk "pico_sdk_init.cmake"))) {
    $sdk = Join-Path $env:USERPROFILE ".pico-sdk\sdk\2.2.0"
}
if (Test-Path (Join-Path $sdk "pico_sdk_init.cmake")) { Write-Ok PicoSDK $sdk } else { Write-Fail PicoSDK $sdk }
$gcc = Join-Path $env:USERPROFILE ".pico-sdk\toolchain\14_2_Rel1\bin\arm-none-eabi-gcc.exe"
if (Test-Path $gcc) { Write-Ok arm-gcc $gcc }
elseif (Has-Cmd arm-none-eabi-gcc) { Write-Ok arm-gcc "PATH" }
else { Write-Fail arm-gcc "Pico ARM GCC" }
$pt = Get-Command picotool -ErrorAction SilentlyContinue
$ptPath = if ($pt) { $pt.Source } else { Join-Path $env:USERPROFILE ".pico-sdk\picotool\2.2.0-a4\picotool\picotool.exe" }
if (Test-Path $ptPath) { Write-Ok picotool $ptPath } else { Write-Fail picotool $ptPath }

Write-Host ""
Write-Host "=== Optional quality / coverage / on-target ==="
if ((Has-Cmd llvm-cov) -and (Has-Cmd llvm-profdata)) { Write-Ok llvm-cov "host coverage" } else { Write-Warn llvm-cov "LLVM bin" }
if (Has-Cmd clang-tidy) { Write-Ok clang-tidy "scripts/run_clang_tidy.ps1" } else { Write-Warn clang-tidy "LLVM clang-tidy" }
if (Has-Cmd cppcheck) { Write-Ok cppcheck "scripts/run_cppcheck.ps1" } else { Write-Warn cppcheck "cppcheck" }
Write-Warn oclint "Linux only; skipped on Windows"
if (Has-Cmd openocd) { Write-Ok openocd "Debug Probe" } else { Write-Warn openocd "openocd" }
if ($py) {
    & $py.Source -u (Join-Path $Root "scripts\run_lizard.py") --check 2>$null | Out-Null
    if ($LASTEXITCODE -eq 0) { Write-Ok lizard "pipx install lizard" } else { Write-Warn lizard "pipx install lizard" }
}

Write-Host ""
if ($script:MissingReq -ne 0) {
    Write-Host "Required tools are missing. See docs/REQUIRED_TOOLS.txt"
    exit 1
}
if ($script:MissingOpt -ne 0) {
    Write-Host "Build tools OK. Optional items listed as WARN above."
    exit 0
}
Write-Host "All required and optional tools found."
exit 0
