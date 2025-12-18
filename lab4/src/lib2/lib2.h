#ifndef MYLIBRARY2
#define MYLIBRARY2
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

int PrimeCount(int a, int b);
int min(int a, int b, int c);
int max(int a, int b, int c);
void swap(int *array, int a, int b);
int find_pivot(int *array, int size);
void partition(int *array, int *kf_i, int *kf_k, int size);
void quicksort(int *array, int size);
void *Sort(int *array, int size);

#endif