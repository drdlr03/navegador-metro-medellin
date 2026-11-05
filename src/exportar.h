#ifndef EXPORTAR_H
#define EXPORTAR_H

#include <string>
#include "grafo.h"
#include "rutas.h"

// ruta puede ser nullptr si solo queremos exportar la red completa
bool exportarDot(const Grafo &g, const Ruta *ruta, const std::string &archivo);
bool exportarSvg(const Grafo &g, const Ruta *ruta, const std::string &archivo, const std::string &titulo);

#endif
