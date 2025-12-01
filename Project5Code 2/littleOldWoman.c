#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
//#include <semaphore.h>

#include "common.h"
#include "common_threads.h"
#include "zemaphore.h"

Zem_t mutex;
Zem_t emptypot;
Zem_t fullpot;
int servings;

void *child(void *arg) { 
    long long int value = (long long int) arg;
    int id = (int) value;
    printf("child started: %d\n",id);
    while(1)
    {
        Zem_wait(&mutex);
        printf("checking the pot: %d\n",id);
        if (servings ==0)
        {
            Zem_post(&emptypot);
            Zem_wait(&fullpot);
        }
        --servings;
        Zem_post(&mutex);
        printf("eating now\n");
        sleep(2);
       
    }
   
    return NULL;
}

void mom(int M)
{
    while(1)
    {
        Zem_wait(&emptypot);
        printf("filling the pot\n");
        servings=M;
        sleep(5);
        Zem_post(&fullpot);

    }
}

int main(int argc, char *argv[]) {
    Zem_init(&mutex, 1);
    Zem_init(&emptypot,0);
    Zem_init(&fullpot,0);
    servings=0; 
    printf("parent: begin\n");
    for (int i = 0; i < 10; ++i)
    {
        pthread_t c;
        long long int me = i;
        Pthread_create(&c, NULL, child, (void *)me);
    }
    mom(10);
//    sem_wait(&s); // wait here for child
    printf("parent: end\n");
    return 0;
}
    
