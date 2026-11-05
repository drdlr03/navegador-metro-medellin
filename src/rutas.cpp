#include "rutas.h"

#include <iostream>
#include <stack>
#include <map>

void MonticuloMin::insertar(int costo, int nodo) {
    datos.push_back({costo, nodo});
    subir(datos.size() - 1);
}

Elemento MonticuloMin::extraerMin() {
    Elemento minimo = datos[0];
    datos[0] = datos.back();
    datos.pop_back();
    if (!datos.empty()) {
        bajar(0);
    }
    return minimo;
}

bool MonticuloMin::vacio() const {
    return datos.empty();
}

void MonticuloMin::subir(int i) {
    while (i > 0) {
        int padre = (i - 1) / 2;
        if (datos[padre].costo <= datos[i].costo) break;
        Elemento temporal = datos[padre];
        datos[padre] = datos[i];
        datos[i] = temporal;
        i = padre;
    }
}

void MonticuloMin::bajar(int i) {
    int n = datos.size();
    while (true) {
        int menor = i;
        int izq = 2 * i + 1;
        int der = 2 * i + 2;

        if (izq < n && datos[izq].costo < datos[menor].costo) menor = izq;
        if (der < n && datos[der].costo < datos[menor].costo) menor = der;
        if (menor == i) break;

        Elemento temporal = datos[menor];
        datos[menor] = datos[i];
        datos[i] = temporal;
        i = menor;
    }
}


std::vector<int> dijkstra(const Grafo &g, const std::vector<int> &inicios, int castigoTransbordo, std::vector<int> &padre) {
    int n = g.estaciones.size();
    std::vector<int> costo(n, INFINITO);
    padre.assign(n, -1);
    MonticuloMin cola;

    // arrancamos desde todos los nodos de la estacion origen (una por linea)
    for (int s : inicios) {
        if (g.estaciones[s].cerrada) continue;
        costo[s] = 0;
        cola.insertar(0, s);
    }

    while (!cola.vacio()) {
        Elemento actual = cola.extraerMin();
        int u = actual.nodo;
        if (actual.costo > costo[u]) continue;

        for (const Arista &a : g.adyacencia[u]) {
            int v = a.destino;
            if (g.estaciones[v].cerrada) continue;

            int peso = a.minutos;
            if (a.esTransbordo) peso += castigoTransbordo;

            if (costo[u] + peso < costo[v]) {
                costo[v] = costo[u] + peso;
                padre[v] = u;
                cola.insertar(costo[v], v);
            }
        }
    }
    return costo;
}

int minutosEntre(const Grafo &g, int a, int b, bool &esTransbordo) {
    int mejor = INFINITO;
    esTransbordo = false;
    for (const Arista &arista : g.adyacencia[a]) {
        if (arista.destino == b && arista.minutos < mejor) {
            mejor = arista.minutos;
            esTransbordo = arista.esTransbordo;
        }
    }
    return mejor;
}

Ruta buscarRuta(const Grafo &g, const std::string &origen, const std::string &destino, bool menosTransbordos) {
    Ruta ruta;
    ruta.encontrada = false;
    ruta.minutos = 0;
    ruta.transbordos = 0;

    // para "menos transbordos" cada transbordo pesa como si fueran 1000 minutos,
    // asi Dijkstra primero minimiza transbordos y despues el tiempo
    int castigo = menosTransbordos ? 1000 : 0;

    std::vector<int> padre;
    std::vector<int> costo = dijkstra(g, g.nodosDe(origen), castigo, padre);

    int fin = -1;
    for (int id : g.nodosDe(destino)) {
        if (costo[id] < INFINITO && (fin == -1 || costo[id] < costo[fin])) {
            fin = id;
        }
    }
    if (fin == -1) {
        return ruta;
    }

    // reconstruimos el camino al reves con una pila
    std::stack<int> pila;
    int actual = fin;
    while (actual != -1) {
        pila.push(actual);
        actual = padre[actual];
    }
    while (!pila.empty()) {
        ruta.nodos.push_back(pila.top());
        pila.pop();
    }

    for (int i = 0; i + 1 < (int) ruta.nodos.size(); i++) {
        bool transbordo;
        ruta.minutos += minutosEntre(g, ruta.nodos[i], ruta.nodos[i + 1], transbordo);
        if (transbordo) ruta.transbordos++;
    }
    ruta.encontrada = true;

    // si hay empate en tiempo preferimos la que tenga menos transbordos
    if (!menosTransbordos && ruta.transbordos > 0) {
        Ruta otra = buscarRuta(g, origen, destino, true);
        if (otra.minutos == ruta.minutos && otra.transbordos < ruta.transbordos) {
            return otra;
        }
    }
    return ruta;
}


// el mismo quicksort con mediana de 3 de la practica 1, pero ordenando paradas por tiempo
void intercambiar(Parada &a, Parada &b) {
    Parada temporal = a;
    a = b;
    b = temporal;
}

int particion(std::vector<Parada> &v, int izquierda, int derecha) {
    int medio = (izquierda + derecha) / 2;

    if (v[medio].minutos < v[izquierda].minutos) {
        intercambiar(v[medio], v[izquierda]);
    }
    if (v[derecha].minutos < v[izquierda].minutos) {
        intercambiar(v[izquierda], v[derecha]);
    }
    if (v[derecha].minutos < v[medio].minutos) {
        intercambiar(v[derecha], v[medio]);
    }

    intercambiar(v[medio], v[derecha]);
    int pivote = v[derecha].minutos;
    int i = izquierda - 1;

    for (int j = izquierda; j <= derecha - 1; j++) {
        if (v[j].minutos < pivote) {
            i++;
            intercambiar(v[i], v[j]);
        }
    }
    intercambiar(v[i + 1], v[derecha]);
    return i + 1;
}

void quicksort(std::vector<Parada> &v, int izquierda, int derecha) {
    if (izquierda < derecha) {
        int indice_particion = particion(v, izquierda, derecha);
        quicksort(v, izquierda, indice_particion - 1);
        quicksort(v, indice_particion + 1, derecha);
    }
}

std::vector<Parada> alcanzables(const Grafo &g, const std::string &origen, int limiteMinutos) {
    std::vector<int> padre;
    std::vector<int> costo = dijkstra(g, g.nodosDe(origen), 0, padre);

    // una estacion puede tener varios nodos, nos quedamos con el menor tiempo
    std::map<std::string, int> mejorTiempo;
    for (int i = 0; i < (int) costo.size(); i++) {
        if (costo[i] > limiteMinutos) continue;
        std::string nombre = g.estaciones[i].nombre;
        if (mejorTiempo.count(nombre) == 0 || costo[i] < mejorTiempo[nombre]) {
            mejorTiempo[nombre] = costo[i];
        }
    }

    std::vector<Parada> paradas;
    for (auto &par : mejorTiempo) {
        paradas.push_back({par.first, par.second});
    }
    quicksort(paradas, 0, (int) paradas.size() - 1);
    return paradas;
}


// convierte "#2E7D32" al codigo de color de la consola
std::string colorConsola(const std::string &hex) {
    int r = std::stoi(hex.substr(1, 2), nullptr, 16);
    int v = std::stoi(hex.substr(3, 2), nullptr, 16);
    int a = std::stoi(hex.substr(5, 2), nullptr, 16);
    return "\033[38;2;" + std::to_string(r) + ";" + std::to_string(v) + ";" + std::to_string(a) + "m";
}

void imprimirRuta(const Grafo &g, const Ruta &ruta) {
    std::string normal = "\033[0m";

    if (!ruta.encontrada) {
        std::cout << "\n  No hay forma de llegar (puede que haya estaciones cerradas en el camino).\n";
        return;
    }

    std::cout << "\n";
    for (int k = 0; k < (int) ruta.nodos.size(); k++) {
        const Estacion &e = g.estaciones[ruta.nodos[k]];
        const Linea *linea = g.obtenerLinea(e.linea);
        std::string color = colorConsola(linea->color);

        if (k > 0) {
            bool transbordo;
            int minutos = minutosEntre(g, ruta.nodos[k - 1], ruta.nodos[k], transbordo);
            if (transbordo) {
                std::cout << "   ┆  transbordo a " << color << linea->nombre << normal << " (+" << minutos << " min)\n";
                continue;
            }
            std::cout << "   " << color << "│" << normal << "  " << minutos << " min\n";
        }

        std::cout << "   " << color << "●" << normal << " " << e.nombre;
        if (k == 0) std::cout << "   " << color << "[" << linea->nombre << "]" << normal;
        std::cout << "\n";
    }

    int paradas = ruta.nodos.size() - 1 - ruta.transbordos;
    std::cout << "\n  Tiempo total: " << ruta.minutos << " min"
              << "  |  Transbordos: " << ruta.transbordos
              << "  |  Paradas recorridas: " << paradas << "\n";
}
