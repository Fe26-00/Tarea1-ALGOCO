/*
Lee cada par de matrices generado, ejecuta ambos algoritmos sobre ellas midiendo tiempo y memoria utilizada, y guarda tanto la matriz resultante como las mediciones obtenidas.
*/

#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <chrono>
#include <string>
#include <cstdlib>
#include <new>
#include <filesystem>
#include <sys/wait.h>
#include <unistd.h>
#include <cstddef>
#include "algorithms/naive.h"
#include "algorithms/strassen.h"

using namespace std;
using namespace std::chrono;
namespace fs = std::filesystem;

using Matrix = vector<vector<int>>;

static long long g_memoriaActual = 0;
static long long g_memoriaPico   = 0;
static bool      g_midiendo      = false; // solo cuenta mientras esto es true

struct Header {
    size_t size;
    int contado; // 1 si esta asignación ocurrió mientras g_midiendo estaba activo
};
constexpr size_t HEADER =
    ((sizeof(Header) + alignof(std::max_align_t) - 1) / alignof(std::max_align_t))
    * alignof(std::max_align_t);

void* alocarConHeader(size_t size) {
    void* base = std::malloc(size + HEADER);
    if (!base) throw std::bad_alloc();
    Header* h = reinterpret_cast<Header*>(base);
    h->size = size;
    h->contado = g_midiendo ? 1 : 0;
    if (g_midiendo) {
        g_memoriaActual += (long long)size;
        if (g_memoriaActual > g_memoriaPico) g_memoriaPico = g_memoriaActual;
    }
    return reinterpret_cast<char*>(base) + HEADER;
}

void liberarConHeader(void* ptr) noexcept {
    if (!ptr) return;
    char* base = reinterpret_cast<char*>(ptr) - HEADER;
    Header* h = reinterpret_cast<Header*>(base);
    if (h->contado) g_memoriaActual -= (long long)h->size;
    std::free(base);
}

void* operator new(size_t size)                    { return alocarConHeader(size); }
void* operator new[](size_t size)                  { return alocarConHeader(size); }
void  operator delete(void* p) noexcept            { liberarConHeader(p); }
void  operator delete[](void* p) noexcept          { liberarConHeader(p); }
void  operator delete(void* p, size_t) noexcept    { liberarConHeader(p); }
void  operator delete[](void* p, size_t) noexcept  { liberarConHeader(p); }

void iniciarMedicionMemoria() {
    g_memoriaActual = 0;
    g_memoriaPico = 0;
    g_midiendo = true;
}

long long detenerMedicionMemoria() {
    g_midiendo = false;
    return g_memoriaPico; // bytes pico REALMENTE pedidos al heap
}

// LECTURA / ESCRITURA DE MATRICES

Matrix leerMatriz(const string& path) {
    ifstream file(path);
    if (!file.is_open()) {
        cerr << "Error al abrir el archivo: " << path << ";-;" << endl;
        exit(1);
    }

    Matrix m;
    string linea;
    while (getline(file, linea)) {
        if (linea.empty()) continue; // saltar líneas vacías al final del archivo
        stringstream ss(linea);
        vector<int> fila;
        int valor;
        while (ss >> valor) fila.push_back(valor);
        m.push_back(fila);
    }
    return m;
}

// Extrae el nombre base (sin carpeta ni extensión) de una ruta
string nombreBase(const string& rutaArchivo) {
    size_t lastSlash = rutaArchivo.find_last_of("/\\");
    string nombre = (lastSlash == string::npos) ? rutaArchivo : rutaArchivo.substr(lastSlash + 1);
    size_t dot = nombre.find_last_of('.');
    if (dot != string::npos) nombre = nombre.substr(0, dot);
    return nombre;
}

// Parsea "{n}_{t}_{d}_{m}" en sus 4 componentes
struct CasoPrueba {
    string n, t, d, m;
    bool ok = false;
};

CasoPrueba parsearNombre(const string& nombre) {
    CasoPrueba c;
    vector<string> partes;
    stringstream ss(nombre);
    string token;
    while (getline(ss, token, '_')) partes.push_back(token);
    if (partes.size() != 4) {
        cerr << "[Tenemos un problema]: '" << nombre << "' no respeta formato {n}_{t}_{d}_{m}" << endl;
        return c;
    }
    c.n = partes[0]; c.t = partes[1]; c.d = partes[2]; c.m = partes[3];
    c.ok = true;
    return c;
}

// Medicion en proceso hijo
struct ResultadoMedicion {
    double tiempo_ms;
    long long memoria_bytes;
    bool exito;
};

// Corre el algoritmo de multiplicación EN UN PROCESO HIJO, escribe el resultado ahí mismo, y devuelve al padre solo tiempo/memoria via pipe.
ResultadoMedicion medirEnProcesoHijo(const Matrix& M1, const Matrix& M2,
                                       const string& algoritmo,
                                       const string& rutaSalida) {
    int pipefd[2];
    if (pipe(pipefd) == -1) { cerr << "Error creando pipe" << endl; exit(1); }

    pid_t pid = fork();
    if (pid < 0) { cerr << "Error en fork()" << endl; exit(1); }

    if (pid == 0) {
        //  PROCESO HIJO -------------------------
        close(pipefd[0]);

        iniciarMedicionMemoria();
        auto inicio = high_resolution_clock::now();

        Matrix resultado_matriz;
        bool ok = true;
        if (algoritmo == "naive") {
            resultado_matriz = naive_multiply(M1, M2);
        } else if (algoritmo == "strassen") {
            resultado_matriz = strassen_multiply(M1, M2);
        } else {
            ok = false;
        }

        auto fin = high_resolution_clock::now();
        long long memPico = detenerMedicionMemoria();

        ResultadoMedicion resultado;
        resultado.exito = ok;
        resultado.tiempo_ms = duration<double, milli>(fin - inicio).count();
        resultado.memoria_bytes = memPico;

        if (ok) {
            ofstream archivo_o(rutaSalida);
            if (archivo_o.is_open()) {
                for (const auto& fila : resultado_matriz) {
                    for (size_t j = 0; j < fila.size(); j++) {
                        archivo_o << fila[j];
                        if (j + 1 < fila.size()) archivo_o << " ";
                    }
                    archivo_o << "\n";
                }
                archivo_o.close();
            } else {
                cerr << "Error al crear el archivo de salida: " << rutaSalida << endl;
            }
        }

        write(pipefd[1], &resultado, sizeof(resultado));
        close(pipefd[1]);
        _exit(ok ? 0 : 1);
    }

    //  PROCESO PADRE ---------------
    close(pipefd[1]);
    ResultadoMedicion resultado{0, 0, false};
    read(pipefd[0], &resultado, sizeof(resultado));
    close(pipefd[0]);
    int status;
    waitpid(pid, &status, 0);
    return resultado;
}

// Escribe (o agrega) una línea de medición al archivo .txt consolidado
void escribirMedicion(const string& algoritmo, const CasoPrueba& caso,
                       size_t n, const ResultadoMedicion& resultado) {
    string dirMedidas = "./data/measurements/";
    system(("mkdir -p " + dirMedidas).c_str());
    string rutaMedidas = dirMedidas + "measurements.txt";
    bool existeArchivo = ifstream(rutaMedidas).good();

    ofstream archivo_m(rutaMedidas, ios::app);
    if (archivo_m.is_open()) {
        if (!existeArchivo) {
            archivo_m << "n t d m algoritmo tamano tiempo_ms memoria_bytes" << endl;
        }
        archivo_m << caso.n << " " << caso.t << " " << caso.d << " " << caso.m << " "
                  << algoritmo << " " << n << " "
                  << resultado.tiempo_ms << " " << resultado.memoria_bytes << endl;
        archivo_m.close();
    } else {
        cerr << "Error al escribir measurements para " << algoritmo << endl;
    }
}

int main() {
    const string dirInput = "./data/matrix_input/";
    const vector<string> algoritmos = {"naive", "strassen"};

    if (!fs::exists(dirInput) || !fs::is_directory(dirInput)) {
        cerr << "No existe el directorio de entrada: " << dirInput << endl;
        return 1;
    }

    for (const string& algoritmo : algoritmos)
        system(("mkdir -p ./data/matrix_output/" + algoritmo).c_str());
    system("mkdir -p ./data/measurements");

    // Recolectar los "casos base" (sin el sufijo _1 / _2), evitando
    // procesar el mismo caso dos veces (una por cada archivo _1 y _2)
    vector<string> casosBase;
    for (const auto& entry : fs::directory_iterator(dirInput)) {
        if (!entry.is_regular_file() || entry.path().extension() != ".txt") continue;

        string nombre = nombreBase(entry.path().string());
        // Solo nos quedamos con los que terminan en "_1" para no duplicar
        if (nombre.size() >= 2 && nombre.substr(nombre.size() - 2) == "_1") {
            casosBase.push_back(nombre.substr(0, nombre.size() - 2)); // sin el "_1"
        }
    }

    for (const string& base : casosBase) {
        string rutaM1 = dirInput + base + "_1.txt";
        string rutaM2 = dirInput + base + "_2.txt";

        if (!fs::exists(rutaM1) || !fs::exists(rutaM2)) {
            cerr << "Falta el par de matrices para: " << base << endl;
            continue;
        }

        CasoPrueba caso = parsearNombre(base);
        if (!caso.ok) {
            cerr << "Saltando caso con nombre inválido: " << base << endl;
            continue;
        }

        Matrix M1 = leerMatriz(rutaM1);
        Matrix M2 = leerMatriz(rutaM2);
        size_t n = M1.size();

        for (const string& algoritmo : algoritmos) {
            string rutaSalida = "./data/matrix_output/" + algoritmo + "/" + base + "_out.txt";

            ResultadoMedicion resultado = medirEnProcesoHijo(M1, M2, algoritmo, rutaSalida);

            if (!resultado.exito) {
                cerr << "Fallo el algoritmo " << algoritmo << " en " << base << endl;
                continue;
            }

            cout << "[" << algoritmo << "] " << base
                 << " -> " << resultado.tiempo_ms << " ms, "
                 << resultado.memoria_bytes << " bytes" << endl;

            escribirMedicion(algoritmo, caso, n, resultado);
        }
    }

    return 0;
}
