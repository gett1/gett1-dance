@echo off
setlocal
cd /d "%~dp0"
set "GETT1_TARGET=x64"
set "GETT1_BUNDLE_ARCH=x86_64-win"
set "GETT1_PE_MACHINE=8664"
set "GETT1_JUCE=%~1"
if not defined GETT1_JUCE set "GETT1_JUCE=D:\JUCE"
if not exist "%GETT1_JUCE%\CMakeLists.txt" (
  echo JUCE 8 was not found at "%GETT1_JUCE%".
  echo Pass the JUCE folder as the first argument.
  goto fail
)

set "GETT1_ISCC=%~2"
if defined GETT1_ISCC goto check_iscc
set "GETT1_ISCC=%ProgramFiles(x86)%\Inno Setup 6\ISCC.exe"
if exist "%GETT1_ISCC%" goto check_iscc
set "GETT1_ISCC=%LOCALAPPDATA%\Programs\Inno Setup 6\ISCC.exe"
:check_iscc
if not exist "%GETT1_ISCC%" (
  echo Install Inno Setup 6.3 or later in the 6.x series, or pass the full path to ISCC.exe.
  goto fail
)

set "GETT1_BUILD=build-vs18-%GETT1_TARGET%"
cmake -S . -B "%GETT1_BUILD%" -G "Visual Studio 18 2026" -A "%GETT1_TARGET%" "-DGETT1_JUCE_PATH=%GETT1_JUCE%"
if errorlevel 1 goto fail
cmake --build "%GETT1_BUILD%" --config Release --parallel 2 --target Gett1Dance_VST3 Gett1MotionTests Gett1OverlayTests
if errorlevel 1 goto fail

set "GETT1_HOST=%PROCESSOR_ARCHITECTURE%"
if defined PROCESSOR_ARCHITEW6432 set "GETT1_HOST=%PROCESSOR_ARCHITEW6432%"
if /i "%GETT1_HOST%"=="AMD64" set "GETT1_HOST=x64"
if /i not "%GETT1_HOST%"=="%GETT1_TARGET%" goto cross_build
ctest --test-dir "%GETT1_BUILD%" -C Release --output-on-failure
if errorlevel 1 goto fail
goto verify
:cross_build
echo Cross-build: run the compiled tests and scan the VST3 on %GETT1_TARGET% Windows.
:verify
set "GETT1_BUNDLE=%CD%\%GETT1_BUILD%\Gett1Dance_artefacts\Release\VST3\gett1 Dance.vst3"
set "GETT1_PLUGIN=%GETT1_BUNDLE%\Contents\%GETT1_BUNDLE_ARCH%\gett1 Dance.vst3"
powershell.exe -NoProfile -NonInteractive -Command "$ErrorActionPreference='Stop'; $s=[IO.File]::OpenRead($env:GETT1_PLUGIN); try { $r=[IO.BinaryReader]::new($s); if ($r.ReadUInt16() -ne 0x5A4D) { throw 'Missing MZ header' }; $s.Position=0x3c; $s.Position=$r.ReadUInt32(); if ($r.ReadUInt32() -ne 0x4550) { throw 'Missing PE header' }; if ($r.ReadUInt16() -ne [Convert]::ToUInt16($env:GETT1_PE_MACHINE,16)) { throw 'Unexpected plugin architecture' } } finally { $s.Dispose() }"
if errorlevel 1 goto fail

if not exist "%CD%\dist\" mkdir "%CD%\dist"
if errorlevel 1 goto fail
"%GETT1_ISCC%" "/DPluginBundle=%GETT1_BUNDLE%" "/DPluginArch=%GETT1_TARGET%" "/O%CD%\dist" "%CD%\Installers\Windows.iss"
if errorlevel 1 goto fail
set "GETT1_SETUP=Gett1Dance-0.4.1-Windows-%GETT1_TARGET%-Setup"
echo PRONTO: dist\%GETT1_SETUP%.exe

set "GETT1_RAR=%ProgramFiles%\WinRAR\rar.exe"
if not exist "%GETT1_RAR%" set "GETT1_RAR=%ProgramFiles(x86)%\WinRAR\rar.exe"
if not exist "%GETT1_RAR%" goto done
set "GETT1_RAR_STAGE=%GETT1_SETUP%-%RANDOM%-%RANDOM%.rar"
pushd "%CD%\dist"
"%GETT1_RAR%" a -ep "%GETT1_RAR_STAGE%" "%GETT1_SETUP%.exe" "..\docs\Manuale_Utente_Gett1Dance.pdf" "..\docs\Installazione_Gett1Dance_Windows_x64.pdf"
if errorlevel 1 goto archive_fail
"%GETT1_RAR%" t "%GETT1_RAR_STAGE%"
if errorlevel 1 goto archive_fail
move /y "%GETT1_RAR_STAGE%" "%GETT1_SETUP%.rar" >nul
if errorlevel 1 goto archive_fail
popd
echo PRONTO: dist\%GETT1_SETUP%.rar
goto done

:archive_fail
if exist "%GETT1_RAR_STAGE%" del "%GETT1_RAR_STAGE%"
popd
echo Il setup EXE e pronto. La creazione del RAR non e riuscita.
:done
start "" explorer.exe "%CD%\dist"
pause
exit /b 0

:fail
echo Compilazione o creazione del setup non riuscita.
pause
exit /b 1
