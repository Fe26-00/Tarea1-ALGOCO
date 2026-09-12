#ifndef MERGESORT_H
#define MERGESORT_H

#include <vector>

void merge(std::vector<int>& arr, int left, int mid, int right);
void mergeSort(std::vector<int>& arr, int left, int right);

#endif // MERGESORT_H