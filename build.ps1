$ErrorActionPreference = 'Stop'
$ProgressPreference    = 'SilentlyContinue'
Set-Location $PSScriptRoot

if ($PSScriptRoot -match '[^\x20-\x7E]') {
    Write-Host "ERROR: the folder path contains non-Latin characters:" -ForegroundColor Red
    Write-Host "  $PSScriptRoot"
    Write-Host "Move the project to a simple path, e.g. C:\fsi6, and run again."
    exit 1
}

$tools = Join-Path $PSScriptRoot 'tools'
$base  = 'https://github.com/xpack-dev-tools'
$pkgs  = @(
    @{ Dir = 'xpack-arm-none-eabi-gcc-13.2.1-1.1'
       Url = "$base/arm-none-eabi-gcc-xpack/releases/download/v13.2.1-1.1/xpack-arm-none-eabi-gcc-13.2.1-1.1-win32-x64.zip"
       Sha = '56b18ccb0a50f536332ec5de57799342ff0cd005ca2c54288c74759b51929e4f' },
    @{ Dir = 'xpack-windows-build-tools-4.4.1-2'
       Url = "$base/windows-build-tools-xpack/releases/download/v4.4.1-2/xpack-windows-build-tools-4.4.1-2-win32-x64.zip"
       Sha = '04a637e3d93254eaf321decdb816d28e38c8bad4256ab988c615abf89820cc4d' }
)

New-Item -ItemType Directory -Force $tools | Out-Null
foreach ($p in $pkgs) {
    $dir = Join-Path $tools $p.Dir
    if (Test-Path (Join-Path $dir 'bin')) { continue }
    $zip = Join-Path $tools ($p.Dir + '.zip')
    Write-Host "Downloading $($p.Dir) ..."
    [Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12
    Invoke-WebRequest -Uri $p.Url -OutFile $zip -UseBasicParsing
    $hash = (Get-FileHash $zip -Algorithm SHA256).Hash.ToLower()
    if ($hash -ne $p.Sha) {
        Remove-Item $zip
        throw "Checksum mismatch for $($p.Dir). Download corrupted, run again."
    }
    Write-Host "Unpacking ..."
    if (Get-Command tar.exe -ErrorAction SilentlyContinue) {
        tar.exe -xf $zip -C $tools
        if ($LASTEXITCODE) { throw "Unpacking failed" }
    } else {
        Expand-Archive $zip -DestinationPath $tools -Force
    }
    Remove-Item $zip
}

$env:PATH = (Join-Path $tools "$($pkgs[0].Dir)\bin") + ';' +
            (Join-Path $tools "$($pkgs[1].Dir)\bin") + ';' + $env:PATH

$build = Join-Path $PSScriptRoot 'build'
if (Test-Path $build) { Remove-Item $build -Recurse -Force }

Write-Host "Building ..."
make -j $env:NUMBER_OF_PROCESSORS
if ($LASTEXITCODE) {
    Write-Host "BUILD FAILED" -ForegroundColor Red
    exit 1
}

Write-Host ""
Write-Host "DONE. Firmware: $PSScriptRoot\build\FSI6.bin (and FSI6.hex)" -ForegroundColor Green
explorer.exe (Join-Path $PSScriptRoot 'build')