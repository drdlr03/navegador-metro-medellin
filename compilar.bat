@echo off
cd /d "%~dp0"
g++ -std=c++17 -O2 -static src\main.cpp src\grafo.cpp src\rutas.cpp src\exportar.cpp -o navegador_metro.exe
if errorlevel 1 (
    echo No se pudo compilar. Revisa que g++ de MSYS2 este en el PATH.
    pause
    exit /b 1
)
navegador_metro.exe
pause
