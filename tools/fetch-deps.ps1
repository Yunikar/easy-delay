# Fetches the build dependencies into third_party/.
#
#   JUCE 9.0.5  -> third_party/juce-src/JUCE-master
#   CMake 3.31.6 -> third_party/cmake-src/cmake-3.31.6-windows-x86_64
#
# The archive is kept in third_party/ and the extraction is skipped when the
# destination already exists, so this is safe to re-run.

$ErrorActionPreference = 'Stop'
$ProgressPreference = 'SilentlyContinue'

$root = Split-Path -Parent $PSScriptRoot
$thirdParty = Join-Path $root 'third_party'
$juceDir = Join-Path $thirdParty 'juce-src\JUCE-master'
$cmakeDir = Join-Path $thirdParty 'cmake-src\cmake-3.31.6-windows-x86_64'

New-Item -ItemType Directory -Path $thirdParty -Force | Out-Null

function Get-Archive([string]$url, [string]$destination)
{
    if (Test-Path $destination)
    {
        Write-Host "already present: $destination"
        return
    }

    $temp = "$destination.part"
    Write-Host "downloading $url"
    Invoke-WebRequest -Uri $url -OutFile $temp -UseBasicParsing
    Move-Item -Path $temp -Destination $destination -Force
    Write-Host "saved $destination"
}

function Expand-ArchiveTo([string]$archive, [string]$destination)
{
    if (Test-Path $destination)
    {
        Write-Host "already extracted: $destination"
        return
    }

    Write-Host "extracting $archive"
    Expand-Archive -Path $archive -DestinationPath (Split-Path -Parent $destination) -Force
}

Get-Archive 'https://github.com/juce-framework/JUCE/archive/refs/heads/master.zip' (Join-Path $thirdParty 'JUCE-master.zip')
Get-Archive 'https://github.com/Kitware/CMake/releases/download/v3.31.6/cmake-3.31.6-windows-x86_64.zip' (Join-Path $thirdParty 'cmake.zip')

Expand-ArchiveTo (Join-Path $thirdParty 'JUCE-master.zip') $juceDir
Expand-ArchiveTo (Join-Path $thirdParty 'cmake.zip') $cmakeDir

Write-Host ''
Write-Host 'Dependencies ready:'
Write-Host "  JUCE : $juceDir"
Write-Host "  CMake: $(Join-Path $cmakeDir 'bin\cmake.exe')"
