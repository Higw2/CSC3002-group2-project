# run.ps1 - Run the built executable (assumes build-mingw exists and DLLs copied)
$root = $PSScriptRoot
$build = Join-Path $root "build-mingw"

$exe = Join-Path $build "EchoGidge.exe"
if (-not (Test-Path $exe)) {
    Write-Host "Executable not found: $exe"
    Write-Host "You can build first with: .\build.ps1"
    exit 1
}

Write-Host "Starting EchoGidge..."
Start-Process -FilePath $exe -WorkingDirectory $build
