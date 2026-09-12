/*
Lee cada arreglo generado, ejecuta los cuatro algoritmos de ordenamiento sobre él midiendo tiempo y memoria utilizada, y guarda tanto el arreglo ordenado como las mediciones obtenidas.
*/

#include <iostream>
#include <vector>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <chrono>
#include <string>
#include <cstdlib>
#include <new>
#include <filesystem>
#include <cstddef>
#include <sys/resource.h>
#include <sys/wait.h>
#include <unistd.h>
#include "algorithms/mergesort.h"
#include "algorithms/quicksort.h"
#include "algorithms/patiencesort.h"
#include "algorithms/sort.h"

using namespace std;
using namespace std::chrono;
namespace fs = std::filesystem;

constexpr double LIMITE_SEGUNDOS = 60.0;

// Contador de memoria Heap

static long long g_memoriaActual = 0;
static long long g_memoriaPico = 0;
static bool      g_midiendo = false; // solo cuenta mientras esto es true

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

void* operator new(size_t size) { return alocarConHeader(size); }
void* operator new[](size_t size) { return alocarConHeader(size); }
void  operator delete(void* p) noexcept { liberarConHeader(p); }
void  operator delete[](void* p) noexcept { liberarConHeader(p); }
void  operator delete(void* p, size_t) noexcept { liberarConHeader(p); }
void  operator delete[](void* p, size_t) noexcept { liberarConHeader(p); }

void iniciarMedicionMemoria() {
    g_memoriaActual = 0;
    g_memoriaPico = 0;
    g_midiendo = true;
}

long long detenerMedicionMemoria() {
    g_midiendo = false;
    return g_memoriaPico; // bytes pico que se le piden al heap
}

// 
// ============================================================

// Para leer el archivo *.*
vector<int> leerArchivo(const string& path) {
    vector<int> arr;
    ifstream file(path);
    if (!file.is_open()) {
        cerr << "Error al abrir el archivo: " << path << ";-;" << endl;
        exit(1);
    }
    int numero;
    while (file >> numero) arr.push_back(numero);
    return arr;
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
        cerr << "El nombre '" << nombre
             << "' no respeta el formato {n}_{t}_{d}_{m}!!!!!!!!!!!!" << endl;
        return c;
    }
    c.n = partes[0];
    c.t = partes[1];
    c.d = partes[2];
    c.m = partes[3];
    c.ok = true;
    return c;
}

// Lo que el hijo le manda al padre por el pipe
struct ResultadoMedicion {
    double tiempo_ms;
    long memoria_bytes;
    bool exito;
    bool timeout; // Si se pasa del límite de tiempo
};

// Corre el algoritmo em un proceso hijo y luego devuelve el tiempo y memoria por un pipe
ResultadoMedicion medirEnProcesoHijo(vector<int> arr, const string& algoritmo,
                                       const string& rutaSalida) {
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        cerr << "Error creando pipe :c" << endl;
        exit(1);
    }

    pid_t pid = fork();
    if (pid < 0) {
        cerr << "Error en fork() :c" << endl;
        exit(1);
    }

    if (pid == 0) {
        // PROCESO HIJO ------------------------
        close(pipefd[0]);

        iniciarMedicionMemoria();
        auto inicio = high_resolution_clock::now();

        vector<int> sorted;
        bool ok = true;
        if (algoritmo == "mergesort") {
            int nn = arr.size();
            mergeSort(arr, 0, nn - 1);
            sorted = move(arr); // evita copiar O(n) extra durante la medición
        } else if (algoritmo == "quicksort") {
            int nn = arr.size();
            quickSort(arr, 0, nn - 1);
            sorted = move(arr);
        } else if (algoritmo == "patiencesort") {
            sorted = patienceSorting(arr);
        } else if (algoritmo == "sort") {
            sortArray(arr);
            sorted = move(arr);
        } else {
            ok = false;
        }

        auto fin = high_resolution_clock::now();
        long long memPeak = detenerMedicionMemoria();

        ResultadoMedicion resultado;
        resultado.exito = ok;
        resultado.tiempo_ms = duration<double, milli>(fin - inicio).count();
        resultado.memoria_bytes = memPeak;

        if (ok) {
            ofstream archivo_o(rutaSalida);
            if (archivo_o.is_open()) {
                for (size_t i = 0; i < sorted.size(); i++) {
                    archivo_o << sorted[i];
                    if (i + 1 < sorted.size()) archivo_o << " ";
                }
                archivo_o.close();
            } else {
                cerr << "Error al crear el archivo de salida: " << rutaSalida << " :c" << endl;
            }
        }

        write(pipefd[1], &resultado, sizeof(resultado));
        close(pipefd[1]);
        _exit(ok ? 0 : 1);
    }

    // PROCESO PADRE ---------------------
    close(pipefd[1]);

    // Para hacer lo del límite de tiempo
    auto inicioEspera = steady_clock::now();
    int status;
    bool yaFue = false;

    while (true) {
        pid_t r = waitpid(pid, &status, WNOHANG);
        if (r == pid) break; // el hijo terminó por su cuenta

        double transcurrido = duration<double>(steady_clock::now() - inicioEspera).count();
        if (transcurrido > LIMITE_SEGUNDOS) {
            kill(pid, SIGKILL);
            waitpid(pid, &status, 0); // limpiar el zombie
            yaFue = true;
            break;
        }
        usleep(10000); // 10 ms antes de volver a chequear
    }

    ResultadoMedicion resultado{0, 0, false, false};
    if (!yaFue) {
        read(pipefd[0], &resultado, sizeof(resultado));
    } else {
        resultado.exito = false;
        resultado.timeout = true;
    }
    close(pipefd[0]);

    return resultado;
}

// Escribe (o agrega) una línea de medición al archivo .txt del algoritmo correspondiente
void escribirMedicion(const string& algoritmo, const CasoPrueba& caso,
                       size_t tamano, const ResultadoMedicion& resultado) {
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
                  << algoritmo << " " << tamano << " "
                  << resultado.tiempo_ms << " " << resultado.memoria_bytes << endl;
        archivo_m.close();
    } else {
        cerr << "Error al escribir measurements para " << algoritmo << endl;
    }
}

int main() {
    srand(time(nullptr));

    const string dirInput = "./data/array_input/";
    const vector<string> algoritmos = {"mergesort", "quicksort", "patiencesort", "sort"};

    if (!fs::exists(dirInput) || !fs::is_directory(dirInput)) {
        cerr << "No existe el directorio de entrada: " << dirInput << endl;
        return 1;
    }

    // Crear de antemano las carpetas de salida para cada algoritmo
    for (const string& algoritmo : algoritmos) {
        system(("mkdir -p ./data/array_output/" + algoritmo).c_str());
    }
    system("mkdir -p ./data/measurements");

    // Recorrer cada archivo de entrada
    for (const auto& entry : fs::directory_iterator(dirInput)) {
        if (!entry.is_regular_file()) continue;
        if (entry.path().extension() != ".txt") continue;

        string rutaArchivo = entry.path().string();
        string nombre = nombreBase(rutaArchivo);
        CasoPrueba caso = parsearNombre(nombre);

        if (!caso.ok) {
            cerr << "Saltando archivo con nombre inválido: " << nombre << endl;
            continue;
        }

        vector<int> arr = leerArchivo(rutaArchivo);

        // Correr los 4 algoritmos sobre el mismo arreglo de entrada
        for (const string& algoritmo : algoritmos) {
            string rutaSalida = "./data/array_output/" + algoritmo + "/" + nombre + "_out.txt";

            ResultadoMedicion resultado = medirEnProcesoHijo(arr, algoritmo, rutaSalida);

            if (!resultado.exito) {
                cerr << "Fallo el algoritmo " << algoritmo << " en " << nombre << endl;
                continue;
            }
            /*
            cout << "[" << algoritmo << "] " << nombre
                 << " -> " << resultado.tiempo_ms << " ms, "
                 << resultado.memoria_bytes << " bytes" << endl;
    */
            escribirMedicion(algoritmo, caso, arr.size(), resultado);
            
        }
    }

    return 0;
}
