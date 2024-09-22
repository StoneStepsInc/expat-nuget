@echo off

setlocal

if "%~1" == "" (
  echo Package revision must be provided as the first argument
  goto :EOF
)

set PKG_VER=2.6.3
set PKG_REV=%~1

set EXPAT_FNAME=expat-%PKG_VER%
set EXPAT_RNAME=R_2_6_3
set EXPAT_SHA256=17aa6cfc5c4c219c09287abfc10bc13f0c06f30bb654b28bfe6f567ca646eb79

set PATCH=c:\Program Files\Git\usr\bin\patch.exe
set SEVENZIP_EXE=c:\Program Files\7-Zip\7z.exe
set VCVARSALL=C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall

if NOT EXIST %EXPAT_FNAME%.tar.gz (
  curl --location --output %EXPAT_FNAME%.tar.gz https://github.com/libexpat/libexpat/releases/download/%EXPAT_RNAME%/%EXPAT_FNAME%.tar.gz
)

"%SEVENZIP_EXE%" h -scrcSHA256 %EXPAT_FNAME%.tar.gz | findstr /C:"SHA256 for data" | call devops\check-sha256 "%EXPAT_SHA256%"

if ERRORLEVEL 1 (
  echo SHA-256 signature for %EXPAT_FNAME% does not match
  goto :EOF
)

tar xzf %EXPAT_FNAME%.tar.gz

cd %EXPAT_FNAME%

rem
rem Generate project files for the x64 platform
rem
cmake -S . -B build -A x64 ^
    -DEXPAT_BUILD_TESTS=OFF ^
    -DEXPAT_BUILD_TOOLS=OFF ^
    -DEXPAT_SHARED_LIBS=OFF ^
    -DEXPAT_ENABLE_INSTALL=OFF ^
    -DEXPAT_BUILD_EXAMPLES=OFF

rem
rem Build debug and release configurations
rem
cmake --build build --config Debug
cmake --build build --config Release

rem
rem Collect all package files in the staging area
rem

mkdir ..\nuget\licenses
copy COPYING ..\nuget\licenses\

rem
rem Header files
rem
mkdir ..\nuget\build\native\include
copy build\expat_config.h ..\nuget\build\native\include\
xcopy /Y /S lib\*.h ..\nuget\build\native\include\

rem rename debug library so we don't have to use conditionals in msbuild property files (PDB is referenced in the library)
mkdir ..\nuget\build\native\lib\x64\Debug
copy /Y build\Debug\libexpatdMD.lib ..\nuget\build\native\lib\x64\Debug\libexpatMD.lib
copy /Y build\Debug\libexpatdMD.pdb ..\nuget\build\native\lib\x64\Debug\

rem unfortunately, no PDB for the release build, which \help in debugging
mkdir ..\nuget\build\native\lib\x64\Release
copy /Y build\Release\libexpatMD.lib ..\nuget\build\native\lib\x64\Release\

cd ..

rem
rem Create a package
rem
nuget pack nuget\StoneSteps.expat.VS2022.Static.nuspec -Version %PKG_VER%.%PKG_REV%
