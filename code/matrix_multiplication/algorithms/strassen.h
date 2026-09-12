#ifndef STRASSEN_H
#define STRASSEN_H

#include <vector>

using Matrix = std::vector<std::vector<int>>;

Matrix strassen_multiply(const Matrix& A, const Matrix& B);

#endif // STRASSEN_H