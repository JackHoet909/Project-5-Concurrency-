#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>

#define CHAIRS 4

//Global variables
sem_t waiting_customers;   //Number of customers waiting
sem_t barber_ready;        //Barber is ready for next customer
sem_t customer_ready;      //Customer has sat in the barber chair
pthread_mutex_t mutex; //Mutex for accessing waiting count

int waiting = 0;
int total_customers = 0;   //Will be set from command line

void *barber_work(void *arg) 
{
    while (1) 
    {
        sem_wait(&waiting_customers);   //Sleep if no customers

        pthread_mutex_lock(&mutex); //Protect access to waiting count
        waiting--;
        printf("Barber: Takes next customer. Waiting now: %d\n", waiting);
        pthread_mutex_unlock(&mutex); 

        printf("Barber: Starting haircut...\n");
        sem_post(&barber_ready);        //Invite customer to chair
        sem_wait(&customer_ready);       //Wait until customer sits down

        sleep(2);  //Simulate haircut time
        printf("Barber: Haircut finished!\n");
    }
    return NULL;
}


void *customer_work(void *arg) 
{
    long id = (long)arg;

    pthread_mutex_lock(&mutex);

    //Check for available chairs
    if (waiting < CHAIRS) 
    {
        //There is a chair, sit and wait
        waiting++;
        printf("Customer %ld arrives and sits in waiting room. (Waiting: %d)\n", id, waiting);

        sem_post(&waiting_customers);   //Wake up barber if sleeping
        pthread_mutex_unlock(&mutex);

        sem_wait(&barber_ready);        //Wait for barber to say "next!"
        sem_post(&customer_ready);      //Sit in barber chair

        printf("Customer %ld is now getting a haircut!\n", id);
    } 
    else 
    {
        //No chairs available, leave
        pthread_mutex_unlock(&mutex);
        printf("Customer %ld arrives but no chairs free -> leaves.\n", id);
    }

    return NULL;
}

int main(int argc, char *argv[]) 
{
    //Check command line arguments
    total_customers = atoi(argv[1]);

    pthread_t barber_thread;
    pthread_t *customers = malloc(sizeof(pthread_t) * total_customers);
    

    //Initialize synchronization primitives
    sem_init(&waiting_customers, 0, 0);
    sem_init(&barber_ready, 0, 0);
    sem_init(&customer_ready, 0, 0);
    pthread_mutex_init(&mutex, NULL);

    srand(time(NULL));

    printf("Barbershop opens! %d customers will arrive randomly...\n", total_customers);

    //Start the barber
    pthread_create(&barber_thread, NULL, barber_work, NULL);

    //Create customers with random arrival times
    for (long i = 0; i < total_customers; i++) 
    {
        usleep((rand() % 2000) * 1000);  //Random delay 0–2 seconds for better spread
        pthread_create(&customers[i], NULL, customer_work, (void*)i);
    }

    //Wait for all customers to finish
    for (int i = 0; i < total_customers; i++) 
    {
        pthread_join(customers[i], NULL);
    }
    
    //Politely stop the barber (he'll exit after current haircut or when woken)
    pthread_cancel(barber_thread);
    pthread_join(barber_thread, NULL);

    //Cleanup
    free(customers);
    sem_destroy(&waiting_customers);
    sem_destroy(&barber_ready);
    sem_destroy(&customer_ready);
    pthread_mutex_destroy(&mutex);

    printf("Finished! Barber is going home.\n");
    return 0;
}