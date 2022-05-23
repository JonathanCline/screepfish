@ECHO OFF
SETLOCAL

:: Path to this script's directory
set SCRIPT_DIR=%~dp0

:: Path to the repo's root directory
set REPO_ROOT=%SCRIPT_DIR%

:: Path to the repo tools directory
set TOOLS_ROOT=%REPO_ROOT%/tools

:: Invoke the python script
call %TOOLS_ROOT%/utils/build.bat %*
exit /B %errorlevel%
