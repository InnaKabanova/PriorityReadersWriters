#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

/* NOTES for myself:
 *
 * pthread_t is an arithmetic type, long unsigned int;
 * use %lu to print pthread_t {aka long unsigned int} via printf;
 *
*/

/* TO DO 4: re-write with custom number of writers and readers */
/* TO DO 5: split into several files */

// pthread_mutex_lock(&mutex);
// critical section here
// pthread_mutex_unlock(&mutex);
// pthread_self() - return identifier of current thread

#define VERBOSE_DEBUGGING

#define READERS_NUM 5
#define WRITERS_NUM 5

#define READING_TIMES 10
#define WRITING_TIMES 10

/* Functions to be threaded: */
void* read();
void* write(void* message);

/* Helper functions: */
void rollback()
{
     /* TO DO 1: research on how to cleanup properly before exiting */
}

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
// static pthread_cond_t read_phase = PTHREAD_COND_INITIALIZER;
static pthread_cond_t write_phase = PTHREAD_COND_INITIALIZER;
/* Shared variables: */
static unsigned int readers_counter = 0;
static char* shared_string;

int main()
{
    int i;
    int creation_status = 0;

    pthread_t readers_arr[READERS_NUM];
    pthread_t writers_arr[WRITERS_NUM];

    char* values_arr[WRITERS_NUM] = {"Watermelon", "Pear", "Pomegranate", "Mango", "Carambola"};

    for(i = 0; i < READERS_NUM; i++)
    {
        #ifdef VERBOSE_DEBUGGING
        printf("VERBOSE| Will create a READER thread #%i. Uninitialized thread ID: %lu\n", i, readers_arr[i]);
        #endif /* VERBOSE_DEBUGGING */

        creation_status = pthread_create(&readers_arr[i], NULL, read, NULL);

        if(creation_status == 0)
        {
            #ifdef VERBOSE_DEBUGGING
            printf("VERBOSE| READER thread #%i created. ID: %lu\n", i, readers_arr[i]);
            #endif /* VERBOSE_DEBUGGING */
        }
        else
        {
            /* TO DO 2: do proper cleanup here! */
            #ifdef VERBOSE_DEBUGGING
            printf("VERBOSE| Failed to create READER thread #%i. Errno: %i\n", i, creation_status);
            #endif /* VERBOSE_DEBUGGING */
            exit(EXIT_FAILURE);
        }
    }

    for(i = 0; i < WRITERS_NUM; i++)
    {
        #ifdef VERBOSE_DEBUGGING
        printf("VERBOSE| Will create a WRITER thread #%i. Uninitialized thread ID: %lu\n", i, writers_arr[i]);
        #endif /* VERBOSE_DEBUGGING */

        creation_status = pthread_create(&writers_arr[i], NULL, write, (void*)values_arr[i]);

        if(creation_status == 0)
        {
            #ifdef VERBOSE_DEBUGGING
            printf("VERBOSE| WRITER thread #%i created. ID: %lu\n", i, writers_arr[i]);
            #endif /* VERBOSE_DEBUGGING */
        }
        else
        {
            /* TO DO 3: do proper cleanup here! */
            #ifdef VERBOSE_DEBUGGING
            printf("VERBOSE| Failed to create WRITER thread #%i. Errno: %i\n", i, creation_status);
            #endif /* VERBOSE_DEBUGGING */
            exit(EXIT_FAILURE);
        }
    }

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

void make_delay(double delay_in_seconds)
{
    const time_t start_time = time(NULL);
    time_t current_time;
    do
    {
        time(&current_time);
    }
    while(difftime(current_time, start_time) < delay_in_seconds);
}

double generate_delay(int range)
{
    return (double)(rand() % range);
}

void* read()
{
    /* Pseudo random time delay: */
    double delay = generate_delay(10);
    make_delay(delay);
    #ifdef VERBOSE_DEBUGGING
    printf("VERBOSE | Delay of READ thread %lu is %f\n", pthread_self(), delay);
    #endif /* VERBOSE_DEBUGGING */

    int i;
    for(i = 0; i < READING_TIMES; i++)
    {
        pthread_mutex_lock(&mutex);
            readers_counter++;
        pthread_mutex_unlock(&mutex);

        printf("READING | ID: %lu | Value: %s | Readers count: %i\n", pthread_self(), shared_string, readers_counter);

        pthread_mutex_lock(&mutex);
            readers_counter--;
            if(readers_counter == 0)
            {
                pthread_cond_signal(&write_phase);
            }

            if(i == READING_TIMES - 1)
                printf("END| Thread %lu has ended reading READING_TIMES times.\n", pthread_self());
        pthread_mutex_unlock(&mutex);
    }

    pthread_exit(NULL);
}

void* write(void* message)
{
    /* Pseudo random time delay: */
    double delay = generate_delay(10);
    make_delay(delay);
    #ifdef VERBOSE_DEBUGGING
    printf("VERBOSE | Delay of WRITE thread %lu is %f\n", pthread_self(), delay);
    #endif /* VERBOSE_DEBUGGING */

    int i;
    for(i = 0; i < WRITING_TIMES; i++)
    {
        pthread_mutex_lock(&mutex);
            while(readers_counter != 0)
                pthread_cond_wait(&write_phase, &mutex);

            shared_string = (char*)message;
            printf("WRITING | ID: %lu | New value: %s | Readers count: %i\n", pthread_self(), shared_string, readers_counter);

            if(i == WRITING_TIMES - 1)
            {
                printf("END| Thread %lu has ended writing WRITING_TIMES times.\n", pthread_self());
            }
        pthread_mutex_unlock(&mutex);
    }


    pthread_exit(NULL);
}













