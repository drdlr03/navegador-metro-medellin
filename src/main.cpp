//
// Practica 2 - Navegador de rutas del Metro de Medellin
// Andres Ramirez, Kenny Diaz, David Rodriguez
//

#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <windows.h>

#include "grafo.h"
#include "rutas.h"
#include "exportar.h"


std::string leerLinea(const std::string &mensaje) {
    std::cout << mensaje;
    std::string texto;
    if (!std::getline(std::cin, texto)) exit(0);
    if (!texto.empty() && texto.back() == '\r') texto.pop_back();
    return texto;
}

double leerNumero(const std::string &mensaje) {
    while (true) {
        try {
            return std::stod(leerLinea(mensaje));
        } catch (...) {
            std::cout << "  Eso no es un numero, intenta otra vez.\n";
        }
    }
}

std::string elegirEstacion(const Grafo &g, const std::string &mensaje) {
    while (true) {
        std::vector<std::string> opciones = g.buscarParecidas(leerLinea(mensaje));

        if (opciones.empty()) {
            std::cout << "  No encontramos ninguna estacion con ese nombre.\n";
        } else if (opciones.size() == 1) {
            std::cout << "  -> " << opciones[0] << "\n";
            return opciones[0];
        } else {
            std::cout << "  Hay varias estaciones parecidas:\n";
            for (int i = 0; i < (int) opciones.size(); i++) {
                std::cout << "    " << i + 1 << ". " << opciones[i] << "\n";
            }
            int n = leerNumero("  Cual es? ");
            if (n >= 1 && n <= (int) opciones.size()) return opciones[n - 1];
        }
    }
}

// escribe el SVG y el DOT en la carpeta salidas y pregunta si abrir el mapa
void exportarYAbrir(const Grafo &g, const Ruta *ruta, const std::string &nombre, const std::string &titulo) {
    std::filesystem::create_directories("salidas");
    std::string svg = "salidas/" + nombre + ".svg";
    std::string dot = "salidas/" + nombre + ".dot";

    if (!exportarSvg(g, ruta, svg, titulo) || !exportarDot(g, ruta, dot)) {
        std::cout << "  No se pudo escribir en la carpeta salidas.\n";
        return;
    }
    std::cout << (ruta ? "\n  Se exporto el mapa con la ruta en " : "  Red exportada en ") << svg << " y " << dot << "\n";
    if (leerLinea("  Abrir el mapa? (s/n): ") == "s") {
        system(("start salidas\\" + nombre + ".svg").c_str());
    }
}


void mostrarLineas(const Grafo &g) {
    for (const Linea &l : g.lineas) {
        std::cout << "\n  " << colorConsola(l.color) << "■ " << l.nombre << "\033[0m (" << l.tipo << ")\n     ";
        int cuantas = 0;
        for (const Estacion &e : g.estaciones) {
            if (e.linea != l.codigo) continue;
            if (cuantas > 0) std::cout << (cuantas % 5 == 0 ? " -\n     " : " - ");
            std::cout << e.nombre << (e.cerrada ? " (CERRADA)" : "");
            cuantas++;
        }
        std::cout << "\n";
    }
    std::cout << "\n  Total: " << g.totalEstaciones() << " estaciones, " << g.totalConexiones()
              << " conexiones y " << g.lineas.size() << " lineas.\n";
}

void opcionRuta(const Grafo &g, bool menosTransbordos) {
    std::string origen = elegirEstacion(g, "  Estacion de origen: ");
    std::string destino = elegirEstacion(g, "  Estacion de destino: ");

    if (g.estaCerrada(origen) || g.estaCerrada(destino)) {
        std::cout << "  El origen o el destino estan cerrados.\n";
        return;
    }

    Ruta ruta = buscarRuta(g, origen, destino, menosTransbordos);
    imprimirRuta(g, ruta);
    if (ruta.encontrada) {
        std::string titulo = menosTransbordos ? "Ruta con menos transbordos" : "Ruta más rápida";
        exportarYAbrir(g, &ruta, "ruta", "Metro de Medellín - " + titulo);
    }
}

void opcionAlcanzables(const Grafo &g) {
    std::string origen = elegirEstacion(g, "  Estacion de origen: ");
    int limite = leerNumero("  Cuantos minutos tienes? ");

    std::vector<Parada> paradas = alcanzables(g, origen, limite);
    std::cout << "\n  Desde " << origen << " en " << limite << " min alcanzas " << paradas.size() - 1 << " estaciones:\n";
    for (const Parada &p : paradas) {
        if (p.nombre != origen) std::cout << "    " << p.minutos << " min\t" << p.nombre << "\n";
    }
}

void opcionCerrar(Grafo &g) {
    std::string nombre = elegirEstacion(g, "  Estacion: ");
    bool cerrada = !g.estaCerrada(nombre);
    g.cambiarEstado(nombre, cerrada);
    std::cout << "  " << nombre << (cerrada ? " quedo CERRADA, las rutas la van a esquivar.\n" : " quedo ABIERTA otra vez.\n");
}

void opcionAgregar(Grafo &g) {
    std::cout << "  1. Nueva estacion\n  2. Nueva conexion entre estaciones\n";
    int opcion = leerNumero("  Opcion: ");

    if (opcion == 1) {
        std::string nombre = leerLinea("  Nombre de la estacion: ");
        std::string linea = leerLinea("  Codigo de la linea (ej: A, B, K): ");

        if (g.obtenerLinea(linea) == nullptr) {
            std::cout << "  Esa linea no existe, la vamos a crear.\n";
            std::string nombreLinea = leerLinea("  Nombre de la linea: ");
            std::string tipo = leerLinea("  Tipo (Metro, Metrocable, Tranvia, Bus...): ");
            std::string color = leerLinea("  Color en hexadecimal (ej: #3949AB): ");
            if (color.size() != 7 || color[0] != '#') color = "#3949AB";
            g.agregarLinea(linea, nombreLinea, tipo, color);
        }

        double x = leerNumero("  Coordenada x en el mapa: ");
        double y = leerNumero("  Coordenada y en el mapa: ");
        int nueva = g.agregarEstacion(nombre, linea, x, y, 'D');

        if (leerLinea("  Conectarla con otra estacion? (s/n): ") == "s") {
            std::string otra = elegirEstacion(g, "  Con cual: ");
            int idOtra = g.buscarNodo(otra, linea);
            if (idOtra == -1) {
                // si es de otra linea la metemos tambien en esta, asi queda el transbordo
                const Estacion &e = g.estaciones[g.nodosDe(otra)[0]];
                idOtra = g.agregarEstacion(e.nombre, linea, e.x, e.y, e.etiqueta);
                std::cout << "  " << otra << " ahora tambien es parte de la linea " << linea << " (con transbordo).\n";
            }
            g.conectar(nueva, idOtra, leerNumero("  Minutos entre las dos: "), false);
        }
        std::cout << "  Listo, estacion agregada.\n";

    } else if (opcion == 2) {
        std::string a = elegirEstacion(g, "  Estacion 1: ");
        std::string b = elegirEstacion(g, "  Estacion 2: ");

        // tienen que estar en la misma linea
        for (int na : g.nodosDe(a)) {
            for (int nb : g.nodosDe(b)) {
                if (g.estaciones[na].linea == g.estaciones[nb].linea) {
                    g.conectar(na, nb, leerNumero("  Minutos entre las dos: "), false);
                    std::cout << "  Conexion agregada en la linea " << g.estaciones[na].linea << ".\n";
                    return;
                }
            }
        }
        std::cout << "  Esas estaciones no comparten linea. Primero agrega una estacion en la misma linea.\n";
    }
}

bool cargarRed(Grafo &g) {
    // desde CLion el programa corre en cmake-build-debug, por eso tambien probamos ../datos
    if (g.cargar("datos") || g.cargar("../datos")) {
        std::cout << "  Red cargada: " << g.totalEstaciones() << " estaciones, " << g.totalConexiones() << " conexiones.\n";
        return true;
    }
    std::cout << "  No se encontraron los archivos de la red en la carpeta datos.\n";
    return false;
}


int main() {
    // para que la consola de Windows muestre tildes y colores
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    DWORD modo = 0;
    GetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), &modo);
    SetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), modo | ENABLE_VIRTUAL_TERMINAL_PROCESSING);

    std::cout << "\n  ===== Navegador de rutas - Metro de Medellin =====\n\n";
    Grafo red;
    if (!cargarRed(red)) return 1;

    int opcion = -1;
    while (opcion != 0) {
        std::cout << "\n  ----------------- MENU -----------------\n";
        std::cout << "  1. Ver lineas y estaciones\n";
        std::cout << "  2. Ruta mas rapida (Dijkstra)\n";
        std::cout << "  3. Ruta con menos transbordos\n";
        std::cout << "  4. Estaciones alcanzables en X minutos\n";
        std::cout << "  5. Cerrar / abrir una estacion\n";
        std::cout << "  6. Agregar estacion o conexion\n";
        std::cout << "  7. Exportar la red completa (SVG y DOT)\n";
        std::cout << "  8. Recargar la red desde los archivos\n";
        std::cout << "  0. Salir\n";
        opcion = leerNumero("  Opcion: ");

        switch (opcion) {
            case 1: mostrarLineas(red); break;
            case 2: opcionRuta(red, false); break;
            case 3: opcionRuta(red, true); break;
            case 4: opcionAlcanzables(red); break;
            case 5: opcionCerrar(red); break;
            case 6: opcionAgregar(red); break;
            case 7: exportarYAbrir(red, nullptr, "red_completa", "Metro de Medellín - Red completa"); break;
            case 8: cargarRed(red); break;
            case 0: std::cout << "  Chao!\n"; break;
            default: std::cout << "  Opcion no valida.\n";
        }
    }
    return 0;
}
