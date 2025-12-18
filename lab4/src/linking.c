#include <stdio.h>
#include <stdlib.h>
#include "lib1/lib1.h"

void hello_world(){
    printf("Please enter: M arg1 arg2 ... argN\n");
    printf("M - Selected function from library\n");
    printf("M = -1 - Complete the execution; no args\n");
    printf("M = 1 - The naive algorithm of searching prime numbers; arg1, arg2 - A, B\n");
    printf("M = 2 - Bubble Sort; arg1 - size of unsorted array, arg2 ... argN - elements of unsorted array\n");
}

int main(){   
    hello_world();
    int M, arg1, arg2;
    int *array;
    while (scanf("%d", &M) != EOF) {
        if (M != 1 && M != 2 && M != -1) {
            printf("You can only choose '0','1' or '2' for M\n");
            exit(EXIT_FAILURE);
        }
        if (M == 1) {
            scanf("%d %d", &arg1, &arg2);
            int res1 = PrimeCount(arg1, arg2);
            printf("Result of the naive algorithm (The amount of prime numbers) - %d\n", res1);

        } else if (M == 2) {

            scanf("%d", &arg1); // arg1 = size of unsorted array
            array = malloc(sizeof(int)*arg1);
            for (int i = 0; i < arg1; i++){
                scanf("%d", &array[i]);
            }

            Sort(array, arg1);
            printf("Bubble Sort results - ");
            for (int i = 0; i < arg1; i++) {
                printf("%d ", array[i]);
            }
            printf("\n");
            free(array);
        } else {
            printf("The program successfuly finished\n");
            exit(EXIT_SUCCESS);
        }
    }
}