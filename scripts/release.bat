@echo off
setlocal enabledelayedexpansion

:: Release script for FormatLog
:: Usage: scripts\release.bat <version> [--force]
:: Example: scripts\release.bat 0.6.0
:: Re-runnable: skips steps that are already done
:: --force: skip uncommitted changes check

set "VERSION="
set "FORCE=0"

:: Parse arguments
for %%a in (%*) do (
    if "%%a"=="--force" (
        set "FORCE=1"
    ) else (
        if not defined VERSION set "VERSION=%%a"
    )
)

if "%VERSION%"=="" (
    echo Usage: scripts\release.bat ^<version^> [--force]
    echo Example: scripts\release.bat 0.6.0
    echo   --force  Skip uncommitted changes check
    exit /b 1
)

set "TAG=%VERSION%"
set "ZIP_FILE=FormatLog-%VERSION%.zip"

:: Verify git is authenticated (can reach remote)
echo Verifying git authentication...
git ls-remote origin >nul 2>&1
if errorlevel 1 (
    echo Error: Cannot reach git remote. Check your git authentication.
    exit /b 1
)

:: Verify gh CLI is authenticated
echo Verifying GitHub CLI authentication...
gh auth status >nul 2>&1
if errorlevel 1 (
    echo Error: GitHub CLI is not authenticated. Run 'gh auth login' first.
    exit /b 1
)

:: Verify pio account is logged in
echo Verifying PlatformIO authentication...
pio account show >nul 2>&1
if errorlevel 1 (
    echo Error: PlatformIO account is not logged in. Run 'pio account login' first.
    exit /b 1
)

:: Check for uncommitted changes (skip with --force)
if "%FORCE%"=="0" (
    for /f "delims=" %%i in ('git status --porcelain') do (
        echo Error: Working directory has uncommitted changes. Commit or stash them first.
        echo        Use --force to skip this check.
        exit /b 1
    )
)

:: Check we are on main branch
for /f "delims=" %%i in ('git branch --show-current') do set "BRANCH=%%i"
if not "%BRANCH%"=="main" (
    echo Error: Must be on main branch ^(currently on '%BRANCH%'^)
    exit /b 1
)

:: Update version in library.json and library.properties (idempotent)
echo Updating version to %VERSION%...
powershell -Command "(Get-Content library.json) -replace '\"version\": \".*\"', '\"version\": \"%VERSION%\"' | Set-Content library.json"
powershell -Command "(Get-Content library.properties) -replace '^version=.*', 'version=%VERSION%' | Set-Content library.properties"

:: Update changelog: replace [unreleased] heading with version and date (idempotent)
for /f "delims=" %%i in ('powershell -Command "Get-Date -Format yyyy-MM-dd"') do set "TODAY=%%i"
echo Updating CHANGELOG.md [unreleased] to [%VERSION%] - %TODAY%...
powershell -Command "(Get-Content CHANGELOG.md) -replace '## \[unreleased\]', '## [%VERSION%] - %TODAY%' | Set-Content CHANGELOG.md"

:: Commit if there are staged/unstaged changes to release files
git add library.json library.properties CHANGELOG.md
git diff --cached --quiet 2>nul
if errorlevel 1 (
    echo Committing version bump...
    git commit -m "Release v%VERSION%"
    if errorlevel 1 exit /b 1
) else (
    echo No file changes to commit, skipping...
)

:: Create tag if it doesn't exist
git rev-parse "%TAG%" >nul 2>&1
if errorlevel 1 (
    echo Creating tag %TAG%...
    git tag "%TAG%"
) else (
    echo Tag %TAG% already exists, skipping...
)

:: Push commits and tag
echo Pushing to origin...
git push origin main
if errorlevel 1 exit /b 1

git push origin "%TAG%"
if errorlevel 1 exit /b 1

:: Package library with pio and convert to zip
if not exist "%ZIP_FILE%" (
    echo Packaging library...
    set "TAR_FILE=FormatLog-%VERSION%.tar.gz"
    pio pkg pack -o "!TAR_FILE!"
    if errorlevel 1 exit /b 1

    powershell -Command ^
        "$tar = '!TAR_FILE!'; " ^
        "$zip = '%ZIP_FILE%'; " ^
        "$tmp = New-Item -ItemType Directory -Path (Join-Path $env:TEMP ('pio_pack_' + [guid]::NewGuid().ToString('N'))); " ^
        "tar -xzf $tar -C $tmp.FullName; " ^
        "Compress-Archive -Path (Join-Path $tmp.FullName '*') -DestinationPath $zip -Force; " ^
        "Remove-Item $tmp.FullName -Recurse -Force"
    if errorlevel 1 exit /b 1

    del "!TAR_FILE!" 2>nul
) else (
    echo Package %ZIP_FILE% already exists, skipping...
)

:: Check if GitHub release already exists
gh release view "%TAG%" >nul 2>&1
if not errorlevel 1 (
    echo GitHub release %TAG% already exists, skipping...
    goto :publish
)

:: Extract release notes from CHANGELOG.md
set "NOTES_FILE=%TEMP%\release_notes.tmp"
powershell -Command ^
    "$lines = Get-Content CHANGELOG.md; " ^
    "$found = $false; " ^
    "$notes = @(); " ^
    "foreach ($line in $lines) { " ^
    "  if ($line -match '## \[%VERSION%\]') { $found = $true; continue } " ^
    "  if ($found -and $line -match '^## \[') { break } " ^
    "  if ($found) { $notes += $line } " ^
    "} " ^
    "if ($notes.Count -eq 0) { $notes = @('Release v%VERSION%') }; " ^
    "$notes | Set-Content '%NOTES_FILE%'"

echo Creating GitHub release %TAG%...
gh release create "%TAG%" --title "v%VERSION%" --notes-file "%NOTES_FILE%" "%ZIP_FILE%"
if errorlevel 1 (
    del "%NOTES_FILE%" 2>nul
    del "%ZIP_FILE%" 2>nul
    exit /b 1
)

del "%NOTES_FILE%" 2>nul
del "%ZIP_FILE%" 2>nul

:publish
:: Publish to PlatformIO registry
echo Publishing to PlatformIO registry...
pio pkg publish --no-interactive
if errorlevel 1 exit /b 1

echo.
echo Release v%VERSION% created successfully!
echo https://github.com/RileyCornelius/FormatLog/releases/tag/%TAG%
