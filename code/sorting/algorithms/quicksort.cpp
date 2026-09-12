/*
Implementación del algoritmo basada en:
https://www.geeksforgeeks.org/dsa/quick-sort-algorithm/
*/

#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
using namespace std;

// Elige un índice aleatorio en [low, high] y lo intercambia con arr[high],
// así el pivote termina siendo aleatorio en vez de siempre el último elemento.
void elegirPivoteAleatorio(vector<int>& arr, int low, int high) {
    int indiceAleatorio = low + rand() % (high - low + 1);
    swap(arr[indiceAleatorio], arr[high]);
}

int partition(vector<int>& arr, int low, int high) {
    elegirPivoteAleatorio(arr, low, high);

    // choose the pivot
    int pivot = arr[high];

    // index of smaller element and indicates
    // the right position of pivot found so far
    int i = low - 1;

    for (int j = low; j <= high - 1; j++) {
        if (arr[j] < pivot) {
            i++;
            swap(arr[i], arr[j]);
        }
    }

    swap(arr[i + 1], arr[high]);
    return i + 1;
}

// QuickSort con recursión acotada a O(log n) en el peor caso:
// siempre recursionamos sobre el lado MÁS CHICO, e iteramos sobre el más grande.
void quickSort(vector<int>& arr, int low, int high) {
    while (low < high) {
        int pi = partition(arr, low, high);

        // Tamaño de cada lado
        int tamIzq = pi - 1 - low;
        int tamDer = high - (pi + 1);

        if (tamIzq < tamDer) {
            // Lado izquierdo es más chico: recursionar ahí, iterar sobre el derecho
            quickSort(arr, low, pi - 1);
            low = pi + 1; // seguimos el while con el lado derecho
        } else {
            // Lado derecho es más chico (o igual): recursionar ahí, iterar sobre el izquierdo
            quickSort(arr, pi + 1, high);
            high = pi - 1; // seguimos el while con el lado izquierdo
        }
    }
}

/*
int main() {
    srand(time(nullptr)); // sembrar el generador aleatorio UNA VEZ en el main real
    vector<int> arr = {10, 7, 8, 9, 1, 5};
    int n = arr.size();
    quickSort(arr, 0, n - 1);

    for (int i = 0; i < n; i++) {
        cout << arr[i] << " ";
    }
    return 0;
}
*/
