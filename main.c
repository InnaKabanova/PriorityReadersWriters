#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>

#define VERBOSE_DEBUGGING

#define READERS_NUM 5
#define WRITERS_NUM 5
#define READS_NUM 10
#define WRITES_NUM 10

/* Functions to be threaded: */
void* read_shared_data();
void* write_shared_data(void* fruit);

/* Helper functions: */
static const char* fruits_arr[5] = {"Watermelon", "Pear", "Pomegranate", "Mango", "Carambola"};
const char* get_random_fruit();
int make_delay();

/* Global shared data: */
unsigned int readers_counter = 0;
char* shared_string;

/* Global synchronization variables: */
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t read_phase = PTHREAD_COND_INITIALIZER;
pthread_cond_t write_phase = PTHREAD_COND_INITIALIZER;

int main()
{
    int i;
    int creation_status = 0;

    pthread_t readers_arr[READERS_NUM];
    pthread_t writers_arr[WRITERS_NUM];

    /* Create and start the readers: */
    for(i = 0; i < READERS_NUM; i++)
    {
        #ifdef VERBOSE_DEBUGGING
        printf("VERBOSE| Will create a READER thread #%i. Uninitialized thread ID: %lu\n", i, readers_arr[i]);
        #endif /* VERBOSE_DEBUGGING */

        creation_status = pthread_create(&readers_arr[i], NULL, read_shared_data, NULL);
        if(creation_status == 0)
        {
            #ifdef VERBOSE_DEBUGGING
            printf("VERBOSE| READER thread #%i created. ID: %lu\n", i, readers_arr[i]);
            #endif /* VERBOSE_DEBUGGING */
        }
        else
        {
            #ifdef VERBOSE_DEBUGGING
            printf("VERBOSE| Failed to create READER thread #%i. Errno: %i\n", i, creation_status);
            #endif /* VERBOSE_DEBUGGING */
            exit(EXIT_FAILURE);
        }
    }

    /* Create and start the writers: */
    for(i = 0; i < WRITERS_NUM; i++)
    {
        #ifdef VERBOSE_DEBUGGING
        printf("VERBOSE| Will create a WRITER thread #%i. Uninitialized thread ID: %lu\n", i, writers_arr[i]);
        #endif /* VERBOSE_DEBUGGING */

        creation_status = pthread_create(&writers_arr[i], NULL, write_shared_data, (void*)get_random_fruit());
        if(creation_status == 0)
        {
            #ifdef VERBOSE_DEBUGGING
            printf("VERBOSE| WRITER thread #%i created. ID: %lu\n", i, writers_arr[i]);
            #endif /* VERBOSE_DEBUGGING */
        }
        else
        {
            #ifdef VERBOSE_DEBUGGING
            printf("VERBOSE| Failed to create WRITER thread #%i. Errno: %i\n", i, creation_status);
            #endif /* VERBOSE_DEBUGGING */
            exit(EXIT_FAILURE);
        }
    }

    /* Wait for readers & writers termination: */
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

const char* get_random_fruit()
{
    srand(time(NULL));
    return fruits_arr[rand() % 4];
}

void* read_shared_data()
{
    int i;
    for(i = 0; i < READS_NUM; i++)
    {
        int delay = make_delay();
        #ifdef VERBOSE_DEBUGGING
        printf("VERBOSE | Delay of READ thread %lu was %i sec.\n", pthread_self(), delay);
        #endif /* VERBOSE_DEBUGGING */

        pthread_mutex_lock(&mutex);
            readers_counter++;
        pthread_mutex_unlock(&mutex);

        /* Real critical section: reading data */
        printf("READING | ID: %lu | Value: %s | Readers count: %i\n", pthread_self(), shared_string, readers_counter);

        pthread_mutex_lock(&mutex);
            readers_counter--;
            if(readers_counter == 0)
            {
                pthread_cond_signal(&write_phase);
            }
            #ifdef VERBOSE_DEBUGGING
            if(i == READS_NUM - 1)
                printf("END| Thread %lu has ended reading %i times.\n", pthread_self(), i + 1);
            #endif /* VERBOSE_DEBUGGING */
        pthread_mutex_unlock(&mutex);
    }

    pthread_exit(0);
}

void* write_shared_data(void* fruit)
{
    int i;
    for(i = 0; i < WRITES_NUM; i++)
    {
        int delay = make_delay();
        #ifdef VERBOSE_DEBUGGING
        printf("VERBOSE | Delay of WRITE thread %lu was %i sec.\n", pthread_self(), delay);
        #endif /* VERBOSE_DEBUGGING */

        pthread_mutex_lock(&mutex);
            while(readers_counter != 0)
                pthread_cond_wait(&write_phase, &mutex);

            shared_string = (char*)fruit;
            printf("WRITING | ID: %lu | New value: %s | Readers count: %i\n", pthread_self(), shared_string, readers_counter);

            #ifdef VERBOSE_DEBUGGING
            if(i == WRITES_NUM - 1)
                printf("END| Thread %lu has ended writing %i times.\n", pthread_self(), i + 1);
            #endif
        pthread_mutex_unlock(&mutex);
    }

    pthread_exit(0);
}
