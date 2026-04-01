@echo off

:: 1. Copy the necessary DLLs to the game folder automatically
:: xcopy /Y "C:\Users\creep\Documents\SDL3-devel-3.4.2-mingw\SDL3-3.4.2\x86_64-w64-mingw32\bin\SDL3.dll" .
:: xcopy /Y "C:\Users\creep\Documents\SDL3-devel-3.4.2-mingw\SDL3_mixer-3.2.0\build\SDL3_mixer.dll" .

:: 2. Compile with the -mwindows flag to hide the terminal
g++.exe main.cpp -o kenny.exe ^
 -I"C:\Users\creep\Documents\SDL3-devel-3.4.2-mingw\SDL3-3.4.2\x86_64-w64-mingw32\include" ^
 -I"C:\Users\creep\Documents\SDL3-devel-3.4.2-mingw\SDL3_mixer-3.2.0\include" ^
 -L"C:\Users\creep\Documents\SDL3-devel-3.4.2-mingw\SDL3-3.4.2\x86_64-w64-mingw32\lib" ^
 -L"C:\Users\creep\Documents\SDL3-devel-3.4.2-mingw\SDL3_mixer-3.2.0\build" ^
 -lSDL3 -lSDL3_mixer ^
 -mwindows

if %errorlevel% neq 0 (
    echo [!] Build Failed! Check the errors above.
    pause
) else (
    echo [!] Build Successful! Launching Skater Kenny...
    start kenny.exe
)




:: @echo off
:: set "SDL3_PATH=.\SDKs\SDL3"
:: set "MIX_PATH=.\SDKs\SDL3_mixer"

:: xcopy /Y "%SDL3_PATH%\bin\SDL3.dll" .
:: xcopy /Y "%MIX_PATH%\build\SDL3_mixer.dll" .

:: g++.exe main.cpp -o kenny.exe ^
::  -I"%SDL3_PATH%\include" -I"%MIX_PATH%\include" ^
::  -L"%SDL3_PATH%\lib" -L"%MIX_PATH%\build" ^
::  -lSDL3 -lSDL3_mixer -mwindows

:: if %errorlevel% equ 0 start kenny.exe