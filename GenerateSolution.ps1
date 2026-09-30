# Generates the Visual Studio 2026 solution in bin/windowsx64/vs/ and optionally opens it.
# Usage: ./GenerateSolution.ps1 -QtPrefix C:/Qt/6.8.0/msvc2022_64 [-Open]
param(
	[string]$QtPrefix = $env:CMAKE_PREFIX_PATH,
	[switch]$Open
)

$ErrorActionPreference = 'Stop'

if (-not $QtPrefix) {
	throw 'Pass -QtPrefix <Qt kit folder> or set the CMAKE_PREFIX_PATH environment variable.'
}

# Returns whether the given cmake executable can generate Visual Studio 2026 projects.
function Test-CMakeSupportsVisualStudio2026([string]$cmakePath) {
	return [bool](& $cmakePath --help | Select-String -SimpleMatch 'Visual Studio 18 2026')
}

# Finds a cmake that knows the VS 2026 generator: the ones bundled with Visual Studio first, then PATH.
function Find-CMake {
	$candidates = @()
	$vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
	if (Test-Path $vswhere) {
		foreach ($installation in & $vswhere -all -prerelease -property installationPath) {
			$candidates += Join-Path $installation 'Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe'
		}
	}

	$command = Get-Command cmake -ErrorAction SilentlyContinue
	if ($command) {
		$candidates += $command.Source
	}

	foreach ($candidate in $candidates) {
		if ((Test-Path $candidate) -and (Test-CMakeSupportsVisualStudio2026 $candidate)) {
			return $candidate
		}
	}

	throw 'No cmake with the "Visual Studio 18 2026" generator found. Install the CMake component of Visual Studio 2026.'
}

$cmake = Find-CMake
Push-Location $PSScriptRoot
try {
	# Windows PowerShell 5.1 turns native stderr output (CMake warnings) into errors; only the exit code counts here.
	$ErrorActionPreference = 'Continue'
	& $cmake --preset windows-x64 "-DCMAKE_PREFIX_PATH=$QtPrefix"
	$ErrorActionPreference = 'Stop'
	if ($LASTEXITCODE -ne 0) {
		throw "CMake configuration failed with exit code $LASTEXITCODE."
	}
} finally {
	Pop-Location
}

$solution = Get-ChildItem (Join-Path $PSScriptRoot 'bin/windowsx64/vs') -Include *.slnx, *.sln -File -Recurse -Depth 0 | Select-Object -First 1
if (-not $solution) {
	throw 'No solution file was generated.'
}

Write-Host "Solution: $($solution.FullName)"
if ($Open) {
	Start-Process $solution.FullName
}
