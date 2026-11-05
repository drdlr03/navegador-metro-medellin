#include "exportar.h"

#include <fstream>
#include <set>
#include <algorithm>

const double ESCALA_X = 55;
const double ESCALA_Y = 40;
const double MARGEN_X = 60;
const double MARGEN_Y = 110;

std::string colorDe(const Grafo &g, const std::string &codigoLinea) {
    return g.obtenerLinea(codigoLinea)->color;
}

// guardamos cada arista de la ruta como (menor, mayor) para buscarla rapido
std::set<std::pair<int, int>> aristasDeRuta(const Ruta *ruta) {
    std::set<std::pair<int, int>> aristas;
    if (ruta == nullptr || !ruta->encontrada) return aristas;
    for (int i = 0; i + 1 < (int) ruta->nodos.size(); i++) {
        int a = ruta->nodos[i];
        int b = ruta->nodos[i + 1];
        aristas.insert({std::min(a, b), std::max(a, b)});
    }
    return aristas;
}


bool exportarDot(const Grafo &g, const Ruta *ruta, const std::string &archivo) {
    std::ofstream out(archivo);
    if (!out) return false;

    std::set<std::pair<int, int>> enRuta = aristasDeRuta(ruta);

    out << "graph Metro {\n";
    out << "  layout=neato;\n";
    out << "  node [shape=circle, style=filled, width=0.2, label=\"\"];\n";

    for (int i = 0; i < (int) g.estaciones.size(); i++) {
        const Estacion &e = g.estaciones[i];
        out << "  n" << i << " [xlabel=\"" << e.nombre << "\", pos=\"" << e.x << "," << -e.y << "!\""
            << ", fillcolor=\"" << (e.cerrada ? "#9E9E9E" : colorDe(g, e.linea)) << "\"];\n";
    }

    for (int i = 0; i < (int) g.adyacencia.size(); i++) {
        for (const Arista &a : g.adyacencia[i]) {
            if (i > a.destino) continue;
            bool marcada = enRuta.count({i, a.destino}) > 0;

            out << "  n" << i << " -- n" << a.destino;
            if (a.esTransbordo) {
                out << " [style=dashed];\n";
            } else {
                out << " [color=\"" << colorDe(g, g.estaciones[i].linea) << "\", label=\"" << a.minutos
                    << "\", penwidth=" << (marcada ? 6 : 2) << "];\n";
            }
        }
    }
    out << "}\n";
    return true;
}


bool exportarSvg(const Grafo &g, const Ruta *ruta, const std::string &archivo, const std::string &titulo) {
    std::ofstream out(archivo);
    if (!out || g.estaciones.empty()) return false;

    bool hayRuta = ruta != nullptr && ruta->encontrada;
    std::set<std::pair<int, int>> enRuta = aristasDeRuta(ruta);
    std::set<std::string> nombresRuta;
    if (hayRuta) {
        for (int id : ruta->nodos) nombresRuta.insert(g.estaciones[id].nombre);
    }

    double maxX = 0, maxY = 0;
    for (const Estacion &e : g.estaciones) {
        if (e.x > maxX) maxX = e.x;
        if (e.y > maxY) maxY = e.y;
    }
    int ancho = MARGEN_X + maxX * ESCALA_X + 200;
    int alto = MARGEN_Y + maxY * ESCALA_Y + 60;

    out << "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"" << ancho << "\" height=\"" << alto
        << "\" viewBox=\"0 0 " << ancho << " " << alto << "\" font-family=\"Arial, Helvetica, sans-serif\">\n";
    out << "<rect width=\"100%\" height=\"100%\" fill=\"#FAFAF7\"/>\n";
    out << "<text x=\"24\" y=\"38\" font-size=\"22\" font-weight=\"bold\" fill=\"#222\">" << titulo << "</text>\n";

    if (hayRuta) {
        int paradas = ruta->nodos.size() - 1 - ruta->transbordos;
        out << "<text x=\"24\" y=\"64\" font-size=\"15\" fill=\"#444\">"
            << g.estaciones[ruta->nodos.front()].nombre << " → " << g.estaciones[ruta->nodos.back()].nombre
            << "  ·  " << ruta->minutos << " min  ·  " << ruta->transbordos << " transbordo(s)  ·  "
            << paradas << " paradas</text>\n";
    } else {
        out << "<text x=\"24\" y=\"64\" font-size=\"15\" fill=\"#444\">" << g.totalEstaciones()
            << " estaciones · " << g.totalConexiones() << " conexiones · " << g.lineas.size() << " líneas</text>\n";
    }

    // primero las vias (para que queden debajo de las estaciones)
    for (int pasada = 0; pasada < 2; pasada++) {
        for (int i = 0; i < (int) g.adyacencia.size(); i++) {
            for (const Arista &a : g.adyacencia[i]) {
                if (i > a.destino || a.esTransbordo) continue;
                bool marcada = enRuta.count({i, a.destino}) > 0;
                if ((pasada == 0 && marcada) || (pasada == 1 && !marcada)) continue;

                const Estacion &e1 = g.estaciones[i];
                const Estacion &e2 = g.estaciones[a.destino];
                double opacidad = (hayRuta && !marcada) ? 0.22 : 1.0;
                int grosor = marcada ? 10 : 6;

                out << "<line x1=\"" << MARGEN_X + e1.x * ESCALA_X << "\" y1=\"" << MARGEN_Y + e1.y * ESCALA_Y
                    << "\" x2=\"" << MARGEN_X + e2.x * ESCALA_X << "\" y2=\"" << MARGEN_Y + e2.y * ESCALA_Y
                    << "\" stroke=\"" << colorDe(g, e1.linea) << "\" stroke-width=\"" << grosor
                    << "\" stroke-linecap=\"round\" opacity=\"" << opacidad << "\"/>\n";
            }
        }
    }

    // estaciones, una sola vez por nombre
    std::set<std::string> dibujadas;
    for (const Estacion &e : g.estaciones) {
        if (dibujadas.count(e.nombre)) continue;
        dibujadas.insert(e.nombre);

        double x = MARGEN_X + e.x * ESCALA_X;
        double y = MARGEN_Y + e.y * ESCALA_Y;
        bool esTransbordo = g.nodosDe(e.nombre).size() > 1;
        bool enLaRuta = nombresRuta.count(e.nombre) > 0;
        bool apagada = hayRuta && !enLaRuta;

        std::string relleno = "#FFFFFF";
        if (hayRuta && e.nombre == g.estaciones[ruta->nodos.front()].nombre) relleno = "#43A047";
        if (hayRuta && e.nombre == g.estaciones[ruta->nodos.back()].nombre) relleno = "#E53935";
        if (e.cerrada) relleno = "#BDBDBD";

        out << "<circle cx=\"" << x << "\" cy=\"" << y << "\" r=\"" << (esTransbordo ? 8 : 5.5)
            << "\" fill=\"" << relleno << "\" stroke=\"#222\" stroke-width=\"" << (esTransbordo ? 3 : 2)
            << "\" opacity=\"" << (apagada ? 0.45 : 1) << "\"/>\n";

        if (e.cerrada) {
            out << "<path d=\"M" << x - 6 << " " << y - 6 << " L" << x + 6 << " " << y + 6
                << " M" << x + 6 << " " << y - 6 << " L" << x - 6 << " " << y + 6
                << "\" stroke=\"#C62828\" stroke-width=\"3\"/>\n";
        }

        std::string posicion;
        if (e.etiqueta == 'I') {
            posicion = "x=\"" + std::to_string(x - 12) + "\" y=\"" + std::to_string(y + 4) + "\" text-anchor=\"end\"";
        } else if (e.etiqueta == 'A') {
            posicion = "transform=\"translate(" + std::to_string(x + 6) + "," + std::to_string(y - 9) + ") rotate(-35)\"";
        } else if (e.etiqueta == 'B') {
            posicion = "transform=\"translate(" + std::to_string(x + 8) + "," + std::to_string(y + 13) + ") rotate(35)\"";
        } else {
            posicion = "x=\"" + std::to_string(x + 12) + "\" y=\"" + std::to_string(y + 4) + "\"";
        }

        out << "<text " << posicion << " font-size=\"12\" stroke=\"#FAFAF7\" stroke-width=\"3\" paint-order=\"stroke\""
            << (enLaRuta ? " font-weight=\"bold\" fill=\"#111\"" : " fill=\"#333\"")
            << (apagada ? " opacity=\"0.5\"" : "") << ">" << e.nombre << "</text>\n";
    }

    // convenciones
    int yLeyenda = alto - 40 - 22 * g.lineas.size();
    out << "<text x=\"24\" y=\"" << yLeyenda - 12 << "\" font-size=\"13\" font-weight=\"bold\" fill=\"#222\">Líneas</text>\n";
    for (int i = 0; i < (int) g.lineas.size(); i++) {
        int yl = yLeyenda + 22 * i;
        out << "<line x1=\"24\" y1=\"" << yl << "\" x2=\"54\" y2=\"" << yl << "\" stroke=\"" << g.lineas[i].color
            << "\" stroke-width=\"6\" stroke-linecap=\"round\"/>\n";
        out << "<text x=\"64\" y=\"" << yl + 4 << "\" font-size=\"12\" fill=\"#333\">" << g.lineas[i].nombre
            << " (" << g.lineas[i].tipo << ")</text>\n";
    }

    out << "</svg>\n";
    return true;
}
