#ifndef QUICKSORT_H
#define QUICKSORT_H

#include <vector>

int partition(std::vector<int>& arr, int low, int high);
void quickSort(std::vector<int>& arr, int low, int high);

#endif // QUICKSORT_H
