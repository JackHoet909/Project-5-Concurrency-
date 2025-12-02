//Philosopher Algorithms

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#include "zemaphore.h"    //for the fork locks
#include "common_threads.h" //extra thread helpers (not really needed here)

int philosopherNum;  //how many philosophers there are
Zem_t *forks; //the forks, each one can only be used by 1 philosopher at a time
int *eatCount; //count how many times each philosopher has eaten

//what each philosopher does
void *firstalgorithm(void *arg) 
{
    int id = (int)(long long)arg;        //this philosopher's number
    int left = id;                        //left fork is the same number as the philosopher
    int right = (id + 1) % philosopherNum;            //right fork is the next one (wraps around)

    while (1) 
    {

        //think for a little bit
        usleep(1000);   //1 millisecond thinking time

        //grab the left fork first
        Zem_wait(&forks[left]);           
        printf("philosopher %d got LEFT fork %d\n", id, left);

        //tiny pause to make deadlock more likely
        usleep(100);

        //grab the right fork next
        Zem_wait(&forks[right]);          
        printf("philosopher %d got RIGHT fork %d\n", id, right);

        // eat now that we have both forks
        printf("philo %d eating\n", id);
        eatCount[id]++;                   //remember we ate one more time
        usleep(1000);                     //eat a little bit

        //put both forks back on the table
        Zem_post(&forks[right]);          //put down right fork
        Zem_post(&forks[left]);           //put down left fork
    }

    return NULL;  //we never really get here
}

int main(int argc, char *argv[]) {

    if (argc < 2) {
        printf("please tell me how many philosophers\n");
        return 1;
    }

    N = atoi(argv[1]);   //convert the number from text to number

    // make space for all the forks and counters
    forks = malloc(sizeof(Zem_t) * N);
    eatCount = malloc(sizeof(int) * N);

    // set up all the forks and counters
    for (int i = 0; i < N; i++) 
    {
        Zem_init(&forks[i], 1);  //each fork starts available
        eatCount[i] = 0;         //nobody has eaten yet
    }

    //start all the philosophers
    for (int i = 0; i < N; i++) 
    {
        pthread_t t;
        pthread_create(&t, NULL, firstalgorithm, (void*)(long long)i);
    }

    //print how many times everyone has eaten every 2 seconds
    while (1) {
        sleep(2);
        printf("\nEAT COUNTS: ");
        for (int i = 0; i < N; i++) 
        {
            printf("%d ", eatCount[i]);
        }
        printf("\n");
    }

    return 0;  //we never actually reach here
}