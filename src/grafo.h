#ifndef GRAFO_H
#define GRAFO_H

#include <string>
#include <vector>
#include <map>

const int TIEMPO_TRANSBORDO = 5;

struct Linea {
    std::string codigo;
    std::string nombre;
    std::string tipo;
    std::string color;
};

struct Estacion {
    std::string nombre;
    std::string linea;
    double x;
    double y;
    char etiqueta;
    bool cerrada;
};

struct Arista {
    int destino;
    int minutos;
    bool esTransbordo;
};

// Cada nodo es una estacion dentro de una linea. Si una estacion esta en dos
// lineas (ej: San Antonio) hay dos nodos unidos por una arista de transbordo.
class Grafo {
public:
    std::vector<Linea> lineas;
    std::vector<Estacion> estaciones;
    std::vector<std::vector<Arista>> adyacencia;

    bool cargar(const std::string &carpeta);
    void agregarLinea(const std::string &codigo, const std::string &nombre, const std::string &tipo, const std::string &color);
    int agregarEstacion(const std::string &nombre, const std::string &linea, double x, double y, char etiqueta);
    void conectar(int a, int b, int minutos, bool transbordo);

    int buscarNodo(const std::string &nombre, const std::string &linea) const;
    std::vector<int> nodosDe(const std::string &nombre) const;
    std::vector<std::string> buscarParecidas(const std::string &texto) const;
    const Linea *obtenerLinea(const std::string &codigo) const;

    void cambiarEstado(const std::string &nombre, bool cerrada);
    bool estaCerrada(const std::string &nombre) const;
    int totalEstaciones() const;
    int totalConexiones() const;

private:
    std::map<std::string, std::vector<int>> indice;
};

std::string normalizar(std::string texto);

#endif
