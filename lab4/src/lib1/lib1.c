#include "lib1.h"

int PrimeCount(int a, int b) { // The naive algorithm
  int prime_count = 0;  
  
  if ((a > b) || (a < 0) || (b < 0)) {
      exit(EXIT_FAILURE);
  }

  bool flag = false;  
  if (a == 1) {
      flag = true;
  }

  for (int i = a; i <= b; ++i) {
      int counter = 0; 
      for (int j = 2; j < i; ++j) {
      if (i % j == 0) {
          counter++;  
          break;      
        }
      }

      if (counter == 0) {
      prime_count++;
      }
  }

  if (flag) {
      prime_count--;
  }

  return prime_count; 
}

void *Sort(int *array, int size) { // Bubble sort

  int i, j;

  for (i = 0; i < size - 1; i++) {

    for (j = i + 1; j < size; j++) {
      if (array[i] > array[j]) {
        int temp = array[i];
        array[i] = array[j];
        array[j] = temp;
      }
    }
  }
}