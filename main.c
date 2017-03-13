#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
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

// global arrays
float** A;
float** B;
float** C;
float** C_test;

// function to initialise 3 global arrays with random.
// void return type can be used since we're operating globally with pointers and hence do not need to return values
void init_arrays(int N, int M, int K) {
    // init A
    A = (float**) malloc(M*sizeof(float*));
    for (int i = 0; i < M; i++) {
        A[i] = (float *) malloc(K * sizeof(float));
        for (int j = 0; j < K; j++) {
//            A[i][j] =  drand48();
            A[i][j] =  i + j;

        }
    }
    // init B
    B = (float**) malloc(K*sizeof(float*));
    for (int i = 0; i < K; i++) {
        B[i] = (float *) malloc(N * sizeof(float));
        for (int j = 0; j < N; j++) {
//            B[i][j] =  drand48();
            B[i][j] =  i + j;

        }
    }
    // init C
    C = (float**) malloc(M*sizeof(float*));
    C_test = (float**) malloc(M*sizeof(float*));
    for (int i = 0; i < M; i++) {
        C[i] = (float *) malloc(N * sizeof(float));
        C_test[i] = (float *) malloc(N * sizeof(float));
        for (int j = 0; j < N; j++) {
            C[i][j] =  0;
            C_test[i][j] = 0;
        }
    }
}


// master thread gets number of threads from user
void* get_number_of_threads(void* threads) {
    int* threads_amount = (int *) threads; // must cast from void*
    printf("Enter # of threads: \n");
    scanf("%d", threads_amount);
    pthread_exit(threads_amount); //return threads_amount
}

// struct for individual array A and B
typedef struct array_struct {
    int chunk;
    int rows;
    int columns;
    float** array;
} array_struct;

// struct for both arrays
typedef struct arrays_struct {
    array_struct A;
    array_struct B;
    int threads;
} arrays_struct;

// struct for containing worker meta details
typedef struct worker_meta_data {
    int start;
    int finish;
    float** worker_A;
    float** worker_B;
    int flag;
    int sum;
    int j;
    int id;
    int i;
    int K;
    int M;
    int chunk;
    int index;
} worker_meta_data;

void* matrix_mult(void* workers_meta_arg) {
    worker_meta_data* workers_meta = (worker_meta_data*) workers_meta_arg;
//    int N = workers_meta->N;
    int K = workers_meta->K;
    float** A = workers_meta->worker_A;
    float** B = workers_meta->worker_B;
    int M = workers_meta->M;
//    printf("M: %d K: %d\n", M, K);
//    printf("id: %d start: %d and finish: %d\n", workers_meta->id, workers_meta->start, workers_meta->finish + 1);
    for (int i = workers_meta->start; i < workers_meta->finish + 1; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < K; k++) {
                C[j][i] += A[j][k] * B[k][i];
            }
        }
    }
    pthread_exit(NULL);
}

void* thread_pool_allocate(void* arrays_arg) {
    arrays_struct* arrays = (arrays_struct *) arrays_arg;
    int threads = arrays->threads;

    // create threads amount of pthreads
    pthread_t worker_threads[threads];
    worker_meta_data worker_meta[threads];
    int N = arrays->B.columns;
    int M = arrays->A.rows;
    int chunk = (N/threads);
    int last_fin = 0;

    for (int i = 0; i < threads-1; i++) {
        worker_meta[i].start = (i * chunk);
        worker_meta[i].finish = worker_meta[i].start + chunk - 1;
        worker_meta[i].M = M;
        worker_meta[i].worker_A = arrays->A.array;
        worker_meta[i].worker_B = arrays->B.array;
        worker_meta[i].K = arrays->B.rows; // B = KM
        last_fin = worker_meta[i].finish;
        pthread_create(&worker_threads[i], NULL, matrix_mult, &worker_meta[i]);
    }

    worker_meta[threads - 1].M = M;
    worker_meta[threads - 1].worker_A = arrays->A.array;
    worker_meta[threads - 1].worker_B = arrays->B.array;
    worker_meta[threads - 1].K = arrays->B.rows; // B = KM
    worker_meta[threads - 1].start = last_fin + 1;
    worker_meta[threads - 1].finish = N - 1;
    pthread_create(&worker_threads[threads - 1], NULL, matrix_mult, &worker_meta[threads - 1]);

    for (int i = 0; i < threads; i++) {
        pthread_join(worker_threads[i], NULL);
    }
    pthread_exit(NULL);
}

void matrix_multiplication() {

    int N, M, K, thread_flag, threads; //declared N, M, and K

    printf("Enter N, M, and K: ");
    // needs the address of N because it needs to change the value of N.
    // C works with pass by value so you need to pass the pointer if you want to change the value
    scanf("%d %d %d", &N, &M, &K);
    printf("N: %d M: %d K: %d\n", N, M, K);


    init_arrays(N, M, K);

    // get number of threads from user
    pthread_t boss_thread; // reference to boss thread
    if ((thread_flag = pthread_create(&boss_thread, NULL, get_number_of_threads, &threads))) {
        fprintf(stderr, "error on pthread creation -- flag: %d\n", thread_flag);
    }
    pthread_join(boss_thread, NULL);
    if (threads > N) { return; } // must be of the form threads <= N
    printf("Boss thread: You've chosen %d threads\n", threads);

    /* create threads and calculate matrix */
    // calculate block start and end points
    array_struct A_struct;
    array_struct B_struct;

    A_struct.chunk = M * K / threads;
    A_struct.array = A;
    A_struct.rows = M;
    A_struct.columns = K;

    B_struct.chunk = K * N / threads;
    B_struct.array = B;
    B_struct.rows = K;
    B_struct.columns = N;

    arrays_struct arrays;
    arrays.A = A_struct;
    arrays.B = B_struct;
    arrays.threads = threads;

    pthread_create(&boss_thread, NULL, thread_pool_allocate, &arrays);
    pthread_join(boss_thread, NULL);

//    for (int j = 0; j < M; j++) {
//        for (int l = 0; l < N; l++) {
//            printf("C[%d][%d]: %f\n", j, l, C[j][l]);
//        }
//    }
//
//    for (int m = 0; m < M; m++) {
//        for (int n = 0; n < N; n++) {
//            for (int k = 0; k < K; k++){
//                C_test[m][n] += A[m][k]*B[k][n];
//            }
//            printf("C_test[%d][%d]: %f\n", m, n, C_test[m][n]);
//        }
//    }

    return;

}