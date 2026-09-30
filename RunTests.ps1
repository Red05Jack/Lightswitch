# Runs all test executables of a configuration built by Lightswitch.slnx.
# Usage: ./RunTests.ps1 [-Configuration Debug|Release|DebugLevelLog|ReleaseLevelLog]
param(
	[ValidateSet('Debug', 'Release', 'DebugLevelLog', 'ReleaseLevelLog')]
	[string]$Configuration = 'Debug'
)

$directoryNames = @{
	Debug = 'debug'
	Release = 'release'
	DebugLevelLog = 'debug_level_log'
	ReleaseLevelLog = 'release_level_log'
}

$outputDirectory = Join-Path $PSScriptRoot "bin/windowsx64/$($directoryNames[$Configuration])"
$tests = Get-ChildItem $outputDirectory -Filter '*Test.exe' -ErrorAction SilentlyContinue
if (-not $tests) {
	throw "No test executables in $outputDirectory. Build Lightswitch.slnx first."
}

$failedTests = @()
foreach ($test in $tests) {
	& $test.FullName | Out-Null
	if ($LASTEXITCODE -eq 0) {
		Write-Host "PASS  $($test.BaseName)"
	} else {
		Write-Host "FAIL  $($test.BaseName) (exit code $LASTEXITCODE)"
		$failedTests += $test.BaseName
	}
}

Write-Host "$($tests.Count - $failedTests.Count) of $($tests.Count) tests passed."
if ($failedTests) {
	exit 1
}
