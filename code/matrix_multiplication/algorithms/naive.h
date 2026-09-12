#ifndef NAIVE_H
#define NAIVE_H

#include <vector>

using Matrix = std::vector<std::vector<int>>;

Matrix naive_multiply(const Matrix& A, const Matrix& B);

#endif // NAIVE_H