cmake_minimum_required(VERSION 3.20)
project(navegador_metro CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(navegador_metro
        src/main.cpp
        src/grafo.cpp
        src/rutas.cpp
        src/exportar.cpp)

# con MinGW enlazamos estatico para que el .exe no pida las DLL de libstdc++
if (MINGW)
    target_link_options(navegador_metro PRIVATE -static)
endif ()
