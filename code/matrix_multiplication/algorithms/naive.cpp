/*
Basado en la implementación de:
https://github.com/Dev-XYS/Algorithms/blob/master/Matrix-Multiplication(Naive).cpp
*/
#include <vector>
using namespace std;

using Matrix = vector<vector<int>>;

Matrix naive_multiply(const Matrix& A, const Matrix& B) {
    int n = A.size();       // filas de A
    int m = A[0].size();    // columnas de A = filas de B
    int q = B[0].size();    // columnas de B

    Matrix R(n, vector<int>(q, 0));

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < q; j++) {
            for (int k = 0; k < m; k++) {
                R[i][j] += A[i][k] * B[k][j];
            }
        }
    }

    return R;
}