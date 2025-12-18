#include "lib2.h"

int PrimeCount(int a, int b) { // Sieve of Eratosthenes

    if((a > b) || (a < 0) || (b < 0)){
        exit(EXIT_FAILURE);
    }

    if(b == 1){
        return 0;
    }

    int *sieve = (int*)malloc((b + 1) * sizeof(int));
    for (int i = 0; i <= b; i++) {
        sieve[i] = 1;
    }
    sieve[0] = 0;
    sieve[1] = 0;

    for(int i = 2; i * i <= b; ++i){
		if(sieve[i] == 1){ 
			for(int j = i * i; j <= b; j += i){
				sieve[j] = 0;
			}
		}	
    }
    
    int counter = 0;
    for(int i = a; i <= b; ++i){
        if(sieve[i] == 1){
            counter++;
        }
    }

	return counter;   
}

int min(int a, int b, int c) {
  if (a < b) {
    if (a < c) {
      return a;
    } else {
      return c;
    }
  } else {
    if (b < c) {
      return b;
    } else {
      return c;
    }
  }
}

int max(int a, int b, int c) {
  if (a > b) {
    if (a > c) {
      return a;
    } else {
      return c;
    }
  } else {
    if (b > c) {
      return b;
    } else {
      return c;
    }
  }
}

void swap(int *array, int a, int b) {
    if(a == b){
        return;
    } 
    int tmp = array[a];
    array[a] = array[b];
    array[b] = tmp;
}

int find_pivot(int *array, int size){
    int a, b, c, pivot;
    a = array[0];
    b = array[size-1];
    c = array[size/2];
    pivot = a + b + c - max(a, b, c) - min(a, b, c);
    return pivot;
}

void partition(int *array, int *kf_i, int *kf_k, int size){
    int pivot = find_pivot(array, size);
    int i = 0;
    int j = 0;
    int k = 0;

    while(j < size){
        if(array[j] < pivot){
            swap(array, i, j);
            if(i != k){
                swap(array, k, j);
            }
            i++;
            k++;
            
        } else if(array[j] == pivot){
            swap(array, k, j);
            k++;
        }
        j++;
    }

    *kf_k = k;
    *kf_i = i;
}

void quicksort(int *array, int size){

    if(size <2){
        return;

    } else if(size==2){

        if(array[0]>array[1]){
            swap(array, 0, 1);
        }
        return;
    }

    int i,k;

    partition(array, &i, &k, size);
    quicksort(array, i);
    quicksort(&array[k], size-k);
}

void *Sort(int *array, int size) { // QuickSort
    quicksort(array, size);;
}