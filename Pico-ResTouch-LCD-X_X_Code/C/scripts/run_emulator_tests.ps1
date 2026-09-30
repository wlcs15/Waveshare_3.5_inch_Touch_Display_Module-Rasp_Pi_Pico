# Run on-target Unity ELF under Renode (RP2040). Does NOT flash hardware.
#   powershell -NoProfile -ExecutionPolicy Bypass -File scripts\run_emulator_tests.ps1
$ErrorActionPreference = "Stop"
$Root = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
Set-Location $Root
$wsBin = "/workspace/tools/bin"
if (Test-Path -LiteralPath $wsBin) {
    $env:PATH = "$wsBin" + [IO.Path]::PathSeparator + $env:PATH
}
$py = if (Get-Command python -ErrorAction SilentlyContinue) { "python" } else { "python3" }
& $py -u (Join-Path $Root "scripts\run_emulator_tests.py") @args
exit $LASTEXITCODE
