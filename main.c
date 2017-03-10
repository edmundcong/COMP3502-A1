#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>

#define NUM_THREADS 2
#define DIMENSIONS 3

/* create thread argument struct for thr_func() */
// typedef is used as an alias e.g. typedef old_name new_name
// in this case you start with a typedef so the rest of the statement is a type def:
// typedef (struct _thread_data_t ...) thread_data_t
// note: not immediately being initialised below
typedef struct _thread_data_t {
    int tid;
    double stuff;
} thread_data_t;

/* thread function */
void* thr_func(void *arg) {
    thread_data_t *data = (thread_data_t *)arg;

    printf("hello from thr_func, thread id: %d with stuff value of: %f\n", data->tid, data->stuff);

    pthread_exit(NULL); // thread termination function with NULL being the argument returned from function
}

// function declaration to avoid C99 implicit declaration warning
void matrix_multiplication(void);

int main(int argc, char **argv) {
    pthread_t thr[NUM_THREADS]; //array of pthreads thr of size num_threads
    int i, rc;
    /* create a thread_data_t argument array */
    thread_data_t thr_data[NUM_THREADS]; // declare num_threads amount of structs

    /* create threads */
    for (i = 0; i < NUM_THREADS; ++i) {
        thr_data[i].tid = i; //initialise tid of each struct
        thr_data[i].stuff = i + 0.5;
        // name of thread function as 3rd arg, 4th is arg passed to function
        // create a pthread and give it the thread object at i, call function and pass it thread data
        if ((rc = pthread_create(&thr[i], NULL, thr_func, &thr_data[i]))) {
            fprintf(stderr, "error: pthread_create, rc: %d\n", rc);
            return EXIT_FAILURE;
        }
    }
    /* block until all threads complete */
    for (i = 0; i < NUM_THREADS; ++i) {
        // suspend execution of calling thread (thr[i]) until target thread completes execution.
        // i.e. waits for target thread (thr[i]) to finish then clean up resources associated with that thread
        // the second argument (null) can have in its place the value made available by pthread_exit
        pthread_join(thr[i], NULL);
    }
    matrix_multiplication();
    return EXIT_SUCCESS;
}

typedef struct array_struct {
    int size;
    int start_point;
    int end_point;
    float* array;
} array_struct;

// pointer to function since it's being used as a callback function and hence need to use pointers
void* randomise_array(void *arg) {
    array_struct *data = (array_struct *)arg;

    for (int i = 0; i < data->size; i++){
        data->array[i] = (float) rand();
    }

    printf("hello from thr_func, thread size: %d\n", data->size);

    pthread_exit(NULL); // thread termination function with NULL being the argument returned from function
}

// global arrays
float** A;
float** B;
float** C;

// function to initialise 3 global arrays with random.
// void return type can be used since we're operating globally with pointers and hence do not need to return values
void init_arrays(int N, int M, int K) {
    // init A
    A = (float**) malloc(M*sizeof(float*));
    for (int i = 0; i < M; i++) {
        A[i] = (float *) malloc(K * sizeof(float));
        for (int j = 0; j < K; j++) {
            A[i][j] =  drand48();
        }
    }
    // init B
    B = (float**) malloc(K*sizeof(float*));
    for (int i = 0; i < K; i++) {
        B[i] = (float *) malloc(N * sizeof(float));
        for (int j = 0; j < N; j++) {
            B[i][j] =  drand48();
        }
    }
}



void matrix_multiplication(){

    int N, M, K; //declared N, M, and K
    int threads, rc; // pthread response int

    printf("Enter N, M, and K: ");
    // needs the address of N because it needs to change the value of N.
    // C works with pass by value so you need to pass the pointer if you want to change the value
    scanf("%d %d %d", &N, &M, &K);
    printf("N: %d M: %d K: %d\n", N, M, K);


    init_arrays(N, M, K);

    // get number of threads from user
    printf("Enter # of threads: \n");
    scanf("%d", &threads);
    printf("%d threads\n", threads);

    return;

}