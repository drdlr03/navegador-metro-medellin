#include "grafo.h"

#include <fstream>
#include <sstream>
#include <iostream>

// Pasa a minusculas y quita tildes para que "itagui" encuentre "Itagüí"
std::string normalizar(std::string texto) {
    std::string conTilde[] = {"á", "é", "í", "ó", "ú", "ü", "ñ", "Á", "É", "Í", "Ó", "Ú", "Ü", "Ñ"};
    std::string sinTilde[] = {"a", "e", "i", "o", "u", "u", "n", "a", "e", "i", "o", "u", "u", "n"};

    for (int i = 0; i < 14; i++) {
        size_t pos = texto.find(conTilde[i]);
        while (pos != std::string::npos) {
            texto.replace(pos, conTilde[i].size(), sinTilde[i]);
            pos = texto.find(conTilde[i]);
        }
    }
    for (char &c : texto) {
        if (c >= 'A' && c <= 'Z') c = c - 'A' + 'a';
    }
    while (!texto.empty() && texto.back() == ' ') {
        texto.pop_back();
    }
    while (!texto.empty() && texto[0] == ' ') {
        texto.erase(0, 1);
    }
    return texto;
}

std::vector<std::string> separar(std::string renglon) {
    if (!renglon.empty() && renglon.back() == '\r') renglon.pop_back();

    std::vector<std::string> partes;
    std::stringstream ss(renglon);
    std::string parte;
    while (std::getline(ss, parte, ';')) {
        partes.push_back(parte);
    }
    return partes;
}


bool Grafo::cargar(const std::string &carpeta) {
    std::ifstream archivoLineas(carpeta + "/lineas.txt");
    std::ifstream archivoEstaciones(carpeta + "/estaciones.txt");
    std::ifstream archivoConexiones(carpeta + "/conexiones.txt");

    if (!archivoLineas || !archivoEstaciones || !archivoConexiones) {
        return false;
    }

    lineas.clear();
    estaciones.clear();
    adyacencia.clear();
    indice.clear();
    std::string renglon;

    while (std::getline(archivoLineas, renglon)) {
        if (renglon.empty() || renglon[0] == '#') continue;
        std::vector<std::string> p = separar(renglon);
        if (p.size() >= 4) agregarLinea(p[0], p[1], p[2], p[3]);
    }

    while (std::getline(archivoEstaciones, renglon)) {
        if (renglon.empty() || renglon[0] == '#') continue;
        std::vector<std::string> p = separar(renglon);
        if (p.size() < 5) continue;
        agregarEstacion(p[1], p[0], std::stod(p[2]), std::stod(p[3]), p[4][0]);
    }

    while (std::getline(archivoConexiones, renglon)) {
        if (renglon.empty() || renglon[0] == '#') continue;
        std::vector<std::string> p = separar(renglon);
        if (p.size() < 4) continue;

        int a = buscarNodo(p[1], p[0]);
        int b = buscarNodo(p[2], p[0]);
        if (a == -1 || b == -1) {
            std::cout << "  Ojo: no se pudo conectar " << p[1] << " con " << p[2] << "\n";
            continue;
        }
        conectar(a, b, std::stoi(p[3]), false);
    }
    return true;
}

void Grafo::agregarLinea(const std::string &codigo, const std::string &nombre, const std::string &tipo, const std::string &color) {
    lineas.push_back({codigo, nombre, tipo, color});
}

int Grafo::agregarEstacion(const std::string &nombre, const std::string &linea, double x, double y, char etiqueta) {
    int yaExiste = buscarNodo(nombre, linea);
    if (yaExiste != -1) {
        return yaExiste;
    }

    int id = estaciones.size();
    estaciones.push_back({nombre, linea, x, y, etiqueta, false});
    adyacencia.push_back(std::vector<Arista>());

    // si la estacion ya existia en otra linea, se crea el transbordo
    std::string clave = normalizar(nombre);
    for (int otro : indice[clave]) {
        conectar(id, otro, TIEMPO_TRANSBORDO, true);
    }
    indice[clave].push_back(id);

    return id;
}

void Grafo::conectar(int a, int b, int minutos, bool transbordo) {
    adyacencia[a].push_back({b, minutos, transbordo});
    adyacencia[b].push_back({a, minutos, transbordo});
}

int Grafo::buscarNodo(const std::string &nombre, const std::string &linea) const {
    for (int id : nodosDe(nombre)) {
        if (estaciones[id].linea == linea) return id;
    }
    return -1;
}

std::vector<int> Grafo::nodosDe(const std::string &nombre) const {
    std::string clave = normalizar(nombre);
    if (indice.count(clave) == 0) {
        return {};
    }
    return indice.at(clave);
}

std::vector<std::string> Grafo::buscarParecidas(const std::string &texto) const {
    std::vector<std::string> encontradas;
    std::string buscado = normalizar(texto);
    if (buscado.empty()) return encontradas;

    // si el nombre es exacto no mostramos opciones
    if (indice.count(buscado) > 0) {
        encontradas.push_back(estaciones[indice.at(buscado)[0]].nombre);
        return encontradas;
    }
    for (auto &par : indice) {
        if (par.first.find(buscado) != std::string::npos) {
            encontradas.push_back(estaciones[par.second[0]].nombre);
        }
    }
    return encontradas;
}

const Linea *Grafo::obtenerLinea(const std::string &codigo) const {
    for (const Linea &l : lineas) {
        if (l.codigo == codigo) return &l;
    }
    return nullptr;
}

void Grafo::cambiarEstado(const std::string &nombre, bool cerrada) {
    for (int id : nodosDe(nombre)) {
        estaciones[id].cerrada = cerrada;
    }
}

bool Grafo::estaCerrada(const std::string &nombre) const {
    std::vector<int> nodos = nodosDe(nombre);
    return !nodos.empty() && estaciones[nodos[0]].cerrada;
}

int Grafo::totalEstaciones() const {
    return indice.size();
}

int Grafo::totalConexiones() const {
    int total = 0;
    for (int i = 0; i < (int) adyacencia.size(); i++) {
        for (const Arista &a : adyacencia[i]) {
            if (!a.esTransbordo && i < a.destino) total++;
        }
    }
    return total;
}
