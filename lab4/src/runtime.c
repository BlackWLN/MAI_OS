#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>

void hello_world(){
    printf("Please enter: M arg1 arg2 ... argN\n");
    printf("M - Selected function from library\n");
    printf("M = -1 - Complete the execution\n");
    printf("M = 0 - Change the library \n");
    printf("M = 1 - Count the amount of prime numbers in [A, B]; arg1, arg2 - A, B\n");
    printf("M = 2 - Array sorting; arg1 - size of unsorted array, arg2 ... argN - elements of unsorted array\n");
}

int main(){
    hello_world();
    int M,arg1,arg2;
    int* array;

    char* libraries[] = {"./lib1/lib1.so", "./lib2/lib2.so"};
    int selected = 1;
    void *cur_lib = dlopen(libraries[selected], RTLD_LAZY); // Downloading the library
    if (cur_lib == NULL){
        char *error = dlerror();
        printf("2nd dynamic library loading error: %s\n", error);
        exit(EXIT_FAILURE);
    } else {
        printf("\n2nd dynamic library loaded\n");
    }

    int (*PrimeCount)(int, int) = dlsym(cur_lib, "PrimeCount"); // Getting a pointer to a function 1
    if (PrimeCount == NULL) {
        char *error = dlerror();
        printf("Error occurred while trying to find PrimeCount in 2nd lib:  %s\n", error);
        exit(EXIT_FAILURE);
    } else {
        printf("Now Eratosthenes Sieve is used for counting prime numbers\n");
    }
    
    void (*Sort)(int*,int) = dlsym(cur_lib, "Sort"); // Getting a pointer to a function 2
    if (Sort == NULL) {
        char *error = dlerror();
        printf("Error occurred while trying to find Sort in 2nd lib:  %s\n", error);
        exit(EXIT_FAILURE);
    } else {
        printf("Now QuickSort is used to sort the array\n\n");
    }

    while (scanf("%d", &M) != EOF) {
        if (M != 1 && M != 2 && M != -1 && M != 0) {
            printf("You can only choose '-1,'0','1' or '2' for M\n");
            dlclose(cur_lib);
            exit(EXIT_FAILURE);
        }
        if (M == 0){
            printf("\n");
            dlclose(cur_lib);
            selected = 1 - selected;
            cur_lib = dlopen(libraries[selected], RTLD_LAZY);
            if (cur_lib == NULL){
                if (selected == 0){
                    char *error = dlerror();
                    printf("1st dynamic library loading error: %s\n", error);
                    exit(EXIT_FAILURE);
                } else {
                    char *error = dlerror();
                    printf("2nd dynamic library loading error: %s\n", error);
                    exit(EXIT_FAILURE);
                }
            } else {
                if (selected == 0) {
                    printf("1st dynamic library loaded\n");
                } else {
                    printf("2nd dynamic library loaded\n");
                }
            }
            PrimeCount = dlsym(cur_lib, "PrimeCount");
            if (PrimeCount == NULL) {
                if (selected == 0){
                    char *error = dlerror();
                    printf("Error occurred while trying to find PrimeCount in 1st lib:  %s\n", error);
                    exit(EXIT_FAILURE);
                } else {
                    char *error = dlerror();
                    printf("Error occurred while trying to find PrimeCount in 2nd lib:  %s\n", error);
                    exit(EXIT_FAILURE);
                }
            } else {
                if (selected == 0) {
                    printf("Now The naive algorithm is used for counting prime numbers\n");
                } else {
                    printf("Now Eratosthenes Sieve is used for counting prime numbers\n");
                }
            }
            Sort = dlsym(cur_lib, "Sort");
            if (Sort == NULL) {
                if (selected == 0){
                    char *error = dlerror();
                    printf("Error occurred while trying to find Sort in 1st lib:  %s\n", error);
                    exit(EXIT_FAILURE);
                } else {
                    char *error = dlerror();
                    printf("Error occurred while trying to find Sort in 2nd lib:  %s\n", error);
                    exit(EXIT_FAILURE);
                }
            } else {
                if (selected == 0) {
                    printf("Now BubbleSort is used to sort the array\n");
                } else {
                    printf("Now QuickSort is used to sort the array\n");
                }
            }
            printf("Your library has been changed successfuly!!!\n\n");
        } else if (M == 1) {
            scanf("%d %d", &arg1, &arg2);
            printf("\n");
            int res1 = PrimeCount(arg1, arg2);
            printf("The amount of prime numbers - %d\n\n", res1);
        } else if (M == 2) {
            scanf("%d", &arg1); // arg1 = size of unsorted array
            array = malloc(sizeof(int)*arg1);
            for (int i = 0; i < arg1; i++){
                scanf("%d", &array[i]);
            }

            Sort(array, arg1);
            printf("Sorted array - ");
            for (int i = 0; i < arg1; i++) {
                printf("%d ", array[i]);
            }
            printf("\n\n");
            free(array);
        } else {
            dlclose(cur_lib);
            printf("The program successfuly finished\n");
            exit(EXIT_SUCCESS);
        }           
    }
}