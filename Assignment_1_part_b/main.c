#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int no_of_customers;
int no_of_seats;
int no_of_free_seats;
int no_of_terminals;
int no_of_free_terms;
int customer_arrival_rate;
int terminal_usage_time;

// anything you want to ad


void * customer_routine(void *);
void * attendant_routine(void *);

// declare global mutex and condition variables
pthread_mutex_t terminal_mutex; // for decreasing the amount of available terminals
pthread_cond_t terminal_cond; //signal that a terminal is free

pthread_mutex_t seat_mutex; // for decreasing the amount of available terminals
pthread_cond_t seat_cond; //signal that a terminal is free

// struct for customer
typedef struct customer_data {
    int ID;
    int rate;
} customer_data;

int main(int argc, char ** argv)
{
    //any variables
    pthread_t attendant_thread; //attendant thread
    int attendant_thread_response;
    int customer_threads_response;

    // ask user to provide the total number of seats & total number of terminals.
    printf("Enter the total number of seats & total number of terminals (int): \n");
    scanf("%d %d", &no_of_seats, &no_of_terminals);

    // ask user to provide the total number of customers, customers arrival rate & terminal usage time.
    printf("Enter the total number of customers, customers arrival rate & terminal usage time (int): \n");
    scanf("%d %d %d", &no_of_customers, &customer_arrival_rate, &terminal_usage_time);

    //Initialize mutexes and condition variable objects
    pthread_mutex_init(&terminal_mutex, NULL);
    pthread_cond_init(&terminal_cond, NULL);

    // anything you want to add
    pthread_t customer_threads[no_of_customers];
    customer_data customer_structs[no_of_customers];
    no_of_free_seats = no_of_seats; // at this point we have only free seats

    // create the attendant thread.
    // needs number of clients and number of free terminals
    attendant_thread_response = pthread_create(&attendant_thread, NULL, attendant_routine, NULL); //farmer routine takes farmer_pace as the arg
    if (attendant_thread_response) {
        printf("ERROR; return code from pthread_create() (attendant) is %d\n", attendant_thread_response);
        exit(-1);
    }

    //create consumer threads according to the arrival rate (in the range between 0 and arrival rate) and
    //pass user-defined id and terminal usage (in the range between 0 and terminal usage)
    srand(time(NULL));   // to randomise rate for each customer
    for (int i = 0; i < no_of_customers; i++) {
        customer_structs[i].ID = i; // the customer id
        customer_structs[i].rate = rand() % terminal_usage_time; // terminal usage rate
        customer_threads_response = pthread_create(&attendant_thread, NULL, customer_routine, &customer_structs[i]); //farmer routine takes farmer_pace as the arg
        if (customer_threads_response ) {
            printf("ERROR; return code from pthread_create() (client) is %d\n", customer_threads_response );
            exit(-1);
        }
    }

    //anything you want to add


    pthread_exit(NULL);
}

void * attendant_routine(void * noargs)
{
    while (1) //Continue to serve customers.
    {

        //The attendant thread must print the following status messages wherever appropriate:
        //"Attendant: The number of free seats now is %d. try to find a free terminal.\n"
        //"Attendant: The number of free terminals is %d. All terminals are occupied. \n"
        //"Attendant: The number of free terminals is %d. There are free terminals now. \n"
        //"Attendant: Call one customer. The number of free seats is now %d.\n"
        //"Attendant: Assign one terminal to the customer. The number of free terminals is now %d.\n"
    }
}

void * customer_routine(void * args)
{
    customer_data* customer = (customer_data*) args;
    printf("Customer %d arrives.\n", customer->ID);
    // waiting room section
    pthread_mutex_lock(&seat_mutex); // to get melon from box we will be dealing with global data so we must lock
    if (no_of_free_seats == 0) { // if box is empty
        printf("Customer %d: oh no! all seats have been taken and I'll leave now!\n", customer->ID);
        pthread_mutex_unlock(&seat_mutex); // unlock before leaving
        pthread_exit(NULL); //leave
    } else {
        no_of_free_seats--; // occupies a seat
        pthread_mutex_unlock(&seat_mutex); //unlock since we're done accessing global
        // in the waiting room

    }

    //"Customer %d: I'm lucky to get a free seat from %d.\n"
    //"Customer %d: I'm to be served.\n"
    //"Customer %d: I'm getting a terminal now.\n"
    //"Customer %d: I'm finished using the terminal and leaving.\n
    pthread_exit(NULL);
}