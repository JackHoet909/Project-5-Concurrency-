#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "common_threads.h"   
#include "zemaphore.h"        

int philosopherNum;   // how many philosophers there are
Zem_t *forks;         // the forks, each one can only be used by 1 philosopher at a time
int *eatCount;        // count how many times each philosopher has eaten

//extra for Algorithm 3
Zem_t seats;

// extra for Algorithm 2
Zem_t forkLock;       // acts like a mutex for checking fork availability
int *forkAvailable;   // 1 if fork is free, 0 if in use

// ---------------- Algorithm 1 ----------------
// what each philosopher does (Algorithm #1)
void *firstalgorithm(void *arg) 
{
    int id = (int)(long long)arg;         // this philosopher's number
    int left  = id;                       // left fork is same number as the philosopher
    int right = (id + 1) % philosopherNum; // right fork is the next one (wraps around)

    while (1) 
    {
        // think for a little bit
        usleep(1000);   // 1 millisecond thinking time

        // grab the left fork first
        Zem_wait(&forks[left]);           
        printf("philosopher %d got LEFT fork %d\n", id, left);

        // tiny pause to make deadlock more likely
        usleep(100);

        // grab the right fork next
        Zem_wait(&forks[right]);          
        printf("philosopher %d got RIGHT fork %d\n", id, right);

        // eat now that we have both forks
        printf("philo %d eating\n", id);
        eatCount[id]++;                   // remember we ate one more time
        usleep(1000);                     // eat a little bit

        // put both forks back on the table
        Zem_post(&forks[right]);          // put down right fork
        Zem_post(&forks[left]);           // put down left fork
    }

    return NULL;  // we never really get here
}

// ---------------- Algorithm 2 ----------------
void *secondalgorithm(void *arg)
{
    int id = (int)(long long)arg;
    int left  = id;
    int right = (id + 1) % philosopherNum;

    while (1)
    {
        // think for a little bit
        usleep(1000); // 1 millisecond thinking time

        // try to get BOTH forks at once
        while (1) {
            // lock the shared forkAvailable[] array
            Zem_wait(&forkLock);

            if (forkAvailable[left] && forkAvailable[right]) {
                // both forks are free, so we "reserve" them
                forkAvailable[left]  = 0;
                forkAvailable[right] = 0;

                // now actually wait on the fork semaphores
                Zem_wait(&forks[left]);
                Zem_wait(&forks[right]);

                // done checking / reserving
                Zem_post(&forkLock);
                break;  // we got both forks, leave the loop
            }

            // at least one fork is not available, so give up for now
            Zem_post(&forkLock);
            usleep(100);  // wait a tiny bit, then try again
        }

        printf("[A2] philosopher %d got forks %d (left) and %d (right)\n",
               id, left, right);

        // eat now that we have both forks
        printf("[A2] philo %d eating\n", id);
        eatCount[id]++;
        usleep(1000); // eat a little bit

        // put both forks back as free
        Zem_wait(&forkLock);
        forkAvailable[left]  = 1;
        forkAvailable[right] = 1;
        Zem_post(&forkLock);

        Zem_post(&forks[right]);
        Zem_post(&forks[left]);
    }

    return NULL;
}

void *thirdalgorithm(void *arg)
{
    int id = (int)(long long)arg;
    int left  = id;
    int right = (id + 1) % philosopherNum;
    printf("this is alg 3");
    while (1)
    {
        // think for a little bit
        usleep(1000); // 1 millisecond thinking time

        //wait for an available seat 
        Zem_wait(&seats);
        printf("philosopher sits\n");
        // grab the right fork first
        Zem_wait(&forks[right]);           
        printf("philosopher %d got RIGHT fork %d\n", id, right);

        // grab the left fork next
        Zem_wait(&forks[left]);          
        printf("philosopher %d got LEFT fork %d\n", id, left);

        //ensure both of our picked up forks are labeled unavailable
        forkAvailable[left]  = 0;
        forkAvailable[right] = 0;

        // eat now that we have both forks
        printf("philo %d eating\n", id);
        eatCount[id]++;                   // remember we ate one more time
        usleep(1000);                     // eat a little bit

        // put both forks back on the table
        Zem_post(&forks[right]);          // put down right fork
        Zem_post(&forks[left]);           // put down left fork

        printf("[A2] philosopher %d got forks %d (left) and %d (right)\n",
               id, left, right);

        // eat now that we have both forks
        printf("[A2] philo %d eating\n", id);
        eatCount[id]++;
        usleep(1000); // eat a little bit

        // put both forks back as free
        Zem_wait(&forkLock);
        forkAvailable[left]  = 1;
        forkAvailable[right] = 1;
        Zem_post(&forkLock);

        Zem_post(&forks[right]);
        Zem_post(&forks[left]);
        
        // put seat back as free
        Zem_post(&seats);
        printf("philosopher gets up\n");
    }

    return NULL;
}

// ---------------- main ----------------

int main(int argc, char *argv[]) {

    if (argc < 3) {
        printf("usage: %s <num_philosophers> <algorithm>\n", argv[0]);
        printf("  algorithm 1 = left then right (can deadlock)\n");
        printf("  algorithm 2 = only pick up if both forks available\n");
        return 1;
    }

    philosopherNum = atoi(argv[1]);   // number of philosophers
    int algorithm  = atoi(argv[2]);   // which algorithm (1 or 2)

    if (philosopherNum < 3 || philosopherNum > 20) {
        printf("please choose num_philosophers between 3 and 20\n");
        return 1;
    }
    if (algorithm != 1 && algorithm != 2 && algorithm !=3) {
        printf("algorithm must be 1, 2, or 3\n");
        return 1;
    }

    // make space for all the forks and counters
    forks         = malloc(sizeof(Zem_t) * philosopherNum);
    eatCount      = malloc(sizeof(int)   * philosopherNum);
    forkAvailable = malloc(sizeof(int)   * philosopherNum); // used by algorithm 2

    if (forks == NULL || eatCount == NULL || forkAvailable == NULL) {
        printf("malloc failed\n");
        return 1;
    }

    // set up all the forks and counters
    for (int i = 0; i < philosopherNum; i++) 
    {
        Zem_init(&forks[i], 1);  // each fork starts available
        eatCount[i]      = 0;    // nobody has eaten yet
        forkAvailable[i] = 1;    // initially, all forks are free (for algo 2)
    }

    // init the lock used by algorithm 2
    Zem_init(&forkLock, 1);
    // init the seats used by Algorithm 3
    Zem_init(&seats, philosopherNum - 1);

    // start all the philosophers with the chosen algorithm
    for (int i = 0; i < philosopherNum; i++)
    {
        pthread_t t;
        if (algorithm == 1) {
            pthread_create(&t, NULL, firstalgorithm, (void*)(long long)i);
        } 
        else if (algorithm == 2) {
            pthread_create(&t, NULL, secondalgorithm, (void*)(long long)i);
        }
        else if (algorithm == 3) {
            pthread_create(&t, NULL, thirdalgorithm, (void*)(long long)i);
        }
        // we don't store t because we never join; main just prints forever
    }

    // print how many times everyone has eaten every 2 seconds
    while (1) {
        sleep(2);
        printf("\nEAT COUNTS (algo %d): ", algorithm);
        for (int i = 0; i < philosopherNum; i++) 
        {
            printf("%d ", eatCount[i]);
        }
        printf("\n");
        fflush(stdout);
    }

    return 0;  // we never actually reach here
}
