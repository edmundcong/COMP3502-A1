#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>

// global arrays
float** A;
float** B;
float** C;
float** C_test;

// function declaration to avoid C99 implicit declaration warning
void matrix_multiplication(void);

int main(int argc, char **argv) {
    matrix_multiplication();
    return EXIT_SUCCESS;
}

// function to initialise 3 global arrays with random.
// void return type can be used since we're operating globally with pointers and hence do not need to return values
void init_arrays(int N, int M, int K) {
    struct timeval epoch_time; // declare new timeval struct from sys/time
    // init A
    A = (float**) malloc(M*sizeof(float*));
    for (int i = 0; i < M; i++) {
        A[i] = (float *) malloc(K * sizeof(float));
        for (int j = 0; j < K; j++) {
            gettimeofday(&epoch_time, NULL); // return time of day in milliseconds
            srand(epoch_time.tv_usec * epoch_time.tv_sec); // random floating-point between 0 and 1
            A[i][j] =  ((float)rand())/RAND_MAX;
        }
    }
    // init B
    B = (float**) malloc(K*sizeof(float*));
    for (int i = 0; i < K; i++) {
        B[i] = (float *) malloc(N * sizeof(float));
        for (int j = 0; j < N; j++) {
            gettimeofday(&epoch_time, NULL); // return time of day in milliseconds
            srand(epoch_time.tv_usec * epoch_time.tv_sec); // random floating-point between 0 and 1
            B[i][j] =  ((float)rand())/RAND_MAX;

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
    int id;
    int K;
    int M;
} worker_meta_data;

void* mat_mult_thrds(void* workers_meta_arg) {
    worker_meta_data* workers_meta = (worker_meta_data*) workers_meta_arg;
    int K = workers_meta->K;
    float** A = workers_meta->worker_A;
    float** B = workers_meta->worker_B;
    int M = workers_meta->M;
    for (int i = workers_meta->start; i < workers_meta->finish + 1; i++) {
            for (int k = 0; k < K; k++) {
                for (int j = 0; j < M; j++) {
                    C[i][j] += A[i][k] * B[k][j];
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
        worker_meta[i].id = i;
        worker_meta[i].worker_A = arrays->A.array;
        worker_meta[i].worker_B = arrays->B.array;
        worker_meta[i].K = arrays->B.rows; // B = KM
        last_fin = worker_meta[i].finish;
        pthread_create(&worker_threads[i], NULL, mat_mult_thrds, &worker_meta[i]);
    }

    worker_meta[threads - 1].M = M;
    worker_meta[threads - 1].worker_A = arrays->A.array;
    worker_meta[threads - 1].worker_B = arrays->B.array;
    worker_meta[threads - 1].K = arrays->B.rows; // B = KM
    // condition needed if we're given 1 thread, since otherwise we'd start at 1 and not 0
    if (threads == 1) worker_meta[threads - 1].start = 0;
    else worker_meta[threads - 1].start = last_fin + 1;
    worker_meta[threads - 1].finish = N - 1;
    pthread_create(&worker_threads[threads - 1], NULL, mat_mult_thrds, &worker_meta[threads - 1]);

    for (int i = 0; i < threads; i++) {
        pthread_join(worker_threads[i], NULL);
    }
    pthread_exit(NULL);
}

void matrix_multiplication() {

    int N, M, K, thread_flag, threads; //declared N, M, and K

    printf("Enter N, M, and K: ");
    // needs the address of N because it needs to change the value of N.
    // C works with pass by value so you need to pass the address if you want to change the value
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

    clock_t begin = clock();

    pthread_create(&boss_thread, NULL, thread_pool_allocate, &arrays);
    pthread_join(boss_thread, NULL);

    clock_t end = clock();
    double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
    printf("TIME SPENT: %f\n", time_spent);
    printf("\n Output Matrix C:\n");

    int same = 0;
    int not_same = 0;
    for (int m = 0; m < M; m++) {
        for (int n = 0; n < N; n++) {
            for (int k = 0; k < K; k++){
                C_test[m][n] += A[m][k]*B[k][n];
            }
            printf("C_test[%d][%d] = %f, C[%d][%d] = %f\n", m, n, C_test[m][n], m, n, C[m][n]);
            if (C_test[m][n] == C[m][n]) {
                same++;
            } else {
                not_same++;
            }
        }
    }
    printf("%d are the same \n%d are not the same", same, not_same);
    return;

}