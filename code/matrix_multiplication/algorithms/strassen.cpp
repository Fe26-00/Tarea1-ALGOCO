/*
Basado en la implementación de:
https://www.geeksforgeeks.org/dsa/strassens-matrix-multiplication/
*/
#include <vector>
using namespace std;

using Matrix = vector<vector<int>>;

Matrix sumar(const Matrix& A, const Matrix& B) {
    int n = A.size();
    Matrix C(n, vector<int>(n));
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            C[i][j] = A[i][j] + B[i][j];
    return C;
}

Matrix restar(const Matrix& A, const Matrix& B) {
    int n = A.size();
    Matrix C(n, vector<int>(n));
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            C[i][j] = A[i][j] - B[i][j];
    return C;
}

// Caso base: multiplicación directa para matrices chicas (evita el overhead
// de seguir dividiendo cuando ya no vale la pena)
Matrix multiplicarDirecto(const Matrix& A, const Matrix& B) {
    int n = A.size();
    Matrix C(n, vector<int>(n, 0));
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            for (int k = 0; k < n; k++)
                C[i][j] += A[i][k] * B[k][j];
    return C;
}

Matrix strassen_multiply(const Matrix& A, const Matrix& B) {
    int n = A.size();

    // Umbral del caso base: por debajo de este tamaño, multiplicar directo
    // es más rápido que seguir dividiendo (el overhead de las sumas/restas
    // de submatrices no compensa). 64 es un valor razonable; puedes ajustarlo.
    if (n <= 64) {
        return multiplicarDirecto(A, B);
    }

    int mitad = n / 2;

    // Dividir cada matriz en 4 cuadrantes de tamaño mitad x mitad
    Matrix A11(mitad, vector<int>(mitad)), A12(mitad, vector<int>(mitad)),
           A21(mitad, vector<int>(mitad)), A22(mitad, vector<int>(mitad));
    Matrix B11(mitad, vector<int>(mitad)), B12(mitad, vector<int>(mitad)),
           B21(mitad, vector<int>(mitad)), B22(mitad, vector<int>(mitad));

    for (int i = 0; i < mitad; i++) {
        for (int j = 0; j < mitad; j++) {
            A11[i][j] = A[i][j];
            A12[i][j] = A[i][j + mitad];
            A21[i][j] = A[i + mitad][j];
            A22[i][j] = A[i + mitad][j + mitad];

            B11[i][j] = B[i][j];
            B12[i][j] = B[i][j + mitad];
            B21[i][j] = B[i + mitad][j];
            B22[i][j] = B[i + mitad][j + mitad];
        }
    }

    // Las 7 multiplicaciones de Strassen (en vez de las 8 del método directo)
    Matrix M1 = strassen_multiply(sumar(A11, A22), sumar(B11, B22));
    Matrix M2 = strassen_multiply(sumar(A21, A22), B11);
    Matrix M3 = strassen_multiply(A11, restar(B12, B22));
    Matrix M4 = strassen_multiply(A22, restar(B21, B11));
    Matrix M5 = strassen_multiply(sumar(A11, A12), B22);
    Matrix M6 = strassen_multiply(restar(A21, A11), sumar(B11, B12));
    Matrix M7 = strassen_multiply(restar(A12, A22), sumar(B21, B22));

    // Combinar los 7 productos en los 4 cuadrantes del resultado
    Matrix C11 = sumar(restar(sumar(M1, M4), M5), M7);
    Matrix C12 = sumar(M3, M5);
    Matrix C21 = sumar(M2, M4);
    Matrix C22 = sumar(restar(sumar(M1, M3), M2), M6);

    // Reensamblar el resultado final n x n
    Matrix C(n, vector<int>(n));
    for (int i = 0; i < mitad; i++) {
        for (int j = 0; j < mitad; j++) {
            C[i][j]                 = C11[i][j];
            C[i][j + mitad]         = C12[i][j];
            C[i + mitad][j]         = C21[i][j];
            C[i + mitad][j + mitad] = C22[i][j];
        }
    }

    return C;
}