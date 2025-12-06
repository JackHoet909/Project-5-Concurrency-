#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define CHAIRS 4

sem_t customers;
sem_t barber;
pthread_mutex_t mutex;
int customerCount;

int waiting = 0;

void *customer_work(void *arg) {
    pthread_mutex_lock(&mutex);
    if (waiting < CHAIRS) {
        waiting++;
        printf("Customer sits. Waiting: %d\n", waiting);
        sem_post(&customers);
        pthread_mutex_unlock(&mutex);

        sem_wait(&barber);
        printf("Customer is getting haircut\n");
    }
    else{
        pthread_mutex_unlock(&mutex);
        printf("No chairs available.\n");
    }
    return NULL;
}

void *barber_work(void *arg) {
    while(1){
        sem_wait(&customers);
        pthread_mutex_lock(&mutex);
        waiting--;
        pthread_mutex_unlock(&mutex);

        sem_post(&barber);
        printf("barber giving haricut\n");
        sleep(2);
        printf("haircut done\n");
    }
}

int main(int argc, char *argv[]) {
    customerCount = atoi(argv[1]); 
    
    pthread_t barber_thread;
    pthread_t customers_threads[customerCount];

    sem_init(&customers, 0, 0);
    sem_init(&barber, 0, 0);
    pthread_mutex_init(&mutex, NULL);

    pthread_create(&barber_thread, NULL, barber_work, NULL);

    for(int i = 0; i < customerCount; i++) {
        sleep(rand() % 4);
        pthread_create(&customers_threads[i], NULL, customer_work, NULL);
    }

    for (int i = 0; i < customerCount; i++){
        pthread_join(customers_threads[i], NULL);
    }

    pthread_cancel(barber_thread);
    pthread_mutex_destroy(&mutex);
    sem_destroy(&customers);
    sem_destroy(&barber);

    return 0;
}