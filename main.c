#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>

/* #define VERBOSE_DEBUGGING */

/*---------------------------------------------------------------------------------------*/
#define READERS_NUM 5
#define WRITERS_NUM 5
#define READS_NUM 5
#define WRITES_NUM 5

/* Global shared data: */
unsigned int shared_var = 0;

/* Global synchronization variables: */
unsigned int readers_counter = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t read_phase = PTHREAD_COND_INITIALIZER; /* Whether the readers ... */
pthread_cond_t write_phase = PTHREAD_COND_INITIALIZER; /* ... or the writers can go next. */

/* Functions to be threaded: */
void* read_shared_data(void* argument_data);
void* write_shared_data(void* argument_data);

/* Helper functions: */
int make_delay();

/*---------------------------------------------------------------------------------------*/
int main()
{
    int i;
    int creation_status = 0;

    pthread_t readers_arr[READERS_NUM];
    pthread_t writers_arr[WRITERS_NUM];

    int readers_ids[READERS_NUM];
    int writers_ids[WRITERS_NUM];

    /* 1. Initialize readers' and writers' IDs: */
    for(i = 0; i < READERS_NUM; i++)
    {
        readers_ids[i] = i+1;
    }

    for(i = 0; i < WRITERS_NUM; i++)
    {
        writers_ids[i] = i+1;
    }

    /* 2. Create and start the readers: */
    for(i = 0; i < READERS_NUM; i++)
    {
        int new_thread_id = i;
        creation_status = pthread_create(&readers_arr[i], NULL, read_shared_data,
                                                                (void*)&readers_ids[i]);
        if(creation_status == 0)
        {
            #ifdef VERBOSE_DEBUGGING
            printf("VERBOSE| READER #%i created. ID: %lu\n", i, readers_arr[i]);
            #endif /* VERBOSE_DEBUGGING */
        }
        else
        {
            #ifdef VERBOSE_DEBUGGING
            printf("VERBOSE| Failed to create READER #%i. Errno: %i\n", i, creation_status);
            #endif /* VERBOSE_DEBUGGING */
            exit(EXIT_FAILURE);
        }
    }

    /* 3. Create and start the writers: */
    for(i = 0; i < WRITERS_NUM; i++)
    {
        int new_thread_id = i;
        creation_status = pthread_create(&writers_arr[i], NULL, write_shared_data,
                                                                (void*)&writers_ids[i]);
        if(creation_status == 0)
        {
            #ifdef VERBOSE_DEBUGGING
            printf("VERBOSE| WRITER #%i created. ID: %lu\n", i, writers_arr[i]);
            #endif /* VERBOSE_DEBUGGING */
        }
        else
        {
            #ifdef VERBOSE_DEBUGGING
            printf("VERBOSE| Failed to create WRITER #%i. Errno: %i\n", i, creation_status);
            #endif /* VERBOSE_DEBUGGING */
            exit(EXIT_FAILURE);
        }
    }

    /* 4. Wait for readers' & writers' termination: */
    for(i = 0; i < READERS_NUM; i++)
    {
        pthread_join(readers_arr[i], NULL);
    }
    for(i = 0; i < WRITERS_NUM; i++)
    {
        pthread_join(writers_arr[i], NULL);
    }

    exit(EXIT_SUCCESS);
}

int make_delay()
{
    int delay;
    srand(time(NULL));
    delay = rand() % 4;
    sleep(delay);
    return delay;
}

void* read_shared_data(void* argument_data)
{
    int this_thread_id = *((int*)argument_data);
    int i;

    for(i = 1; i <= READS_NUM; i++)
    {
        int delay = make_delay();
        #ifdef VERBOSE_DEBUGGING
        printf("VERBOSE | Delay of READER %i was %i sec.\n", this_thread_id, delay);
        #endif /* VERBOSE_DEBUGGING */

        /* Enter critical section: */
        pthread_mutex_lock(&mutex);
        {
            if(readers_counter == -1)
                pthread_cond_wait(&read_phase, &mutex);
            readers_counter++;
        }
        pthread_mutex_unlock(&mutex);

        /* Critical section, READ: */
        printf("READER %i | Value: %i | Readers count: %i ||| Cycle: %i of %i\n",
               this_thread_id, shared_var, readers_counter, i, READS_NUM);

        /* Exit critical section: */
        pthread_mutex_lock(&mutex);
        {
            readers_counter--;
            if(readers_counter == 0)
            {
                pthread_cond_signal(&write_phase);
            }
        }
        pthread_mutex_unlock(&mutex);
    }

    pthread_exit(0);
}

void* write_shared_data(void* argument_data)
{
    int this_thread_id = *((int*)argument_data);
    int i;

    for(i = 1; i <= WRITES_NUM; i++)
    {
        int delay = make_delay();
        #ifdef VERBOSE_DEBUGGING
        printf("VERBOSE | Delay of WRITER %i was %i sec.\n", this_thread_id, delay);
        #endif /* VERBOSE_DEBUGGING */

        /* Enter critical section: */
        pthread_mutex_lock(&mutex);
        {
            while(readers_counter != 0)
                pthread_cond_wait(&write_phase, &mutex);
            readers_counter = -1;
        }
        pthread_mutex_unlock(&mutex);

        /* Critical section, WRITE: */
        shared_var++;
        printf("WRITER %i | New value: %i | Readers count: %i ||| Cycle: %i of %i\n",
               this_thread_id, shared_var, readers_counter, i, WRITES_NUM);

        /* Exit critical section: */
        pthread_mutex_lock(&mutex);
        {
            readers_counter = 0;
            pthread_cond_broadcast(&read_phase);
            pthread_cond_signal(&write_phase);
        }
        pthread_mutex_unlock(&mutex);
    }

    pthread_exit(0);
}
