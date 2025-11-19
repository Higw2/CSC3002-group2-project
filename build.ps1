param([switch]$Reconfigure)

# build.ps1 - Configure/build with MinGW and copy runtime DLLs to build directory
# Usage: .\build.ps1            # normal
#        .\build.ps1 -Reconfigure   # force reconfigure

$root = $PSScriptRoot
$build = Join-Path $root "build-mingw"

if ($Reconfigure -or -not (Test-Path $build)) {
    Write-Host "Configuring CMake (MinGW Makefiles)..."
    cmake -S $root -B $build -G "MinGW Makefiles"
}

Write-Host "Building project..."
cmake --build $build --config Release

# Copy runtime DLLs (choose x86_64 if present, else i686)
$arch = if (Test-Path (Join-Path $root "third-party\windows\SDL2\x86_64-w64-mingw32")) { "x86_64-w64-mingw32" } else { "i686-w64-mingw32" }

$deps = @()

function Add-DepPath([string]$base, [string]$arch, [string]$file) {
    $p = Join-Path $base $arch
    $p = Join-Path $p "bin"
    $p = Join-Path $p $file
    $deps += $p
}

$baseSDL2 = Join-Path $root "third-party\windows\SDL2"
$baseSDL2Image = Join-Path $root "third-party\windows\SDL2_image"
$baseSDL2Mixer = Join-Path $root "third-party\windows\SDL2_mixer"

Add-DepPath $baseSDL2 $arch "SDL2.dll"
Add-DepPath $baseSDL2Image $arch "SDL2_image.dll"
Add-DepPath $baseSDL2Image $arch "zlib1.dll"
Add-DepPath $baseSDL2Image $arch "libpng16-16.dll"
# 如果有 SDL2_mixer 的 dll，可按需添加
Add-DepPath $baseSDL2Mixer $arch "SDL2_mixer.dll"

foreach ($d in $deps) {
    if (Test-Path $d) {
        Write-Host "Copying $(Split-Path $d -Leaf) -> $build"
        Copy-Item $d $build -Force
    }
}

Write-Host "Build finished. Executable is in: $build\EchoGidge.exe"
Write-Host "To run: .\run.ps1 or Start-Process -FilePath \"$build\\EchoGidge.exe\""