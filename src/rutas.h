#ifndef RUTAS_H
#define RUTAS_H

#include <string>
#include <vector>
#include "grafo.h"

const int INFINITO = 1000000000;

struct Ruta {
    bool encontrada;
    std::vector<int> nodos;
    int minutos;
    int transbordos;
};

struct Parada {
    std::string nombre;
    int minutos;
};

struct Elemento {
    int costo;
    int nodo;
};

// Monticulo binario de minimos, lo usamos como cola de prioridad en Dijkstra
class MonticuloMin {
public:
    void insertar(int costo, int nodo);
    Elemento extraerMin();
    bool vacio() const;

private:
    std::vector<Elemento> datos;
    void subir(int i);
    void bajar(int i);
};

std::vector<int> dijkstra(const Grafo &g, const std::vector<int> &inicios, int castigoTransbordo, std::vector<int> &padre);

Ruta buscarRuta(const Grafo &g, const std::string &origen, const std::string &destino, bool menosTransbordos);
std::vector<Parada> alcanzables(const Grafo &g, const std::string &origen, int limiteMinutos);

int minutosEntre(const Grafo &g, int a, int b, bool &esTransbordo);
void quicksort(std::vector<Parada> &v, int izquierda, int derecha);
void imprimirRuta(const Grafo &g, const Ruta &ruta);
std::string colorConsola(const std::string &hex);

#endif
