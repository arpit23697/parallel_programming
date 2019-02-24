// Program to compute the time for attaining barrier using busy wait and mutex
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <semaphore.h>
#include <unistd.h>
#include <pthread.h>

//Since we are implementing the barrier using the busy wait and mutex so for each barrier
//we need a differen counter
#define BARRIER_COUNT 100
int MAX_THREADS = 10;


int thread_count;
int barrier_thread_counts[BARRIER_COUNT];
pthread_mutex_t barrier_mutex;

void * thread_work (void * rank)
{
    long my_rank = (long)rank;

    // Barrier
    for (int barrier_no = 0; barrier_no < BARRIER_COUNT ; barrier_no++)
    {
        pthread_mutex_lock(&barrier_mutex);
        barrier_thread_counts[barrier_no]++;
        pthread_mutex_unlock(&barrier_mutex);
        while (barrier_thread_counts[barrier_no] < thread_count);
        // printf("Hi there");
    }
    return NULL;
}


int counter_sema = 0;
sem_t barrier_sem[BARRIER_COUNT];
sem_t count_sem;


void* thread_work_sema (void * rank)
{
    long my_rank = (long)rank;

    for (int barrier_no = 0 ; barrier_no < BARRIER_COUNT ; barrier_no++)
    {
        //Single barrier
        sem_wait(&count_sem);
        if (counter_sema ==  thread_count - 1)
        {
            counter_sema = 0;
            sem_post(&count_sem);
            for (int j = 0 ; j < thread_count -1 ; j++)
             sem_post(&barrier_sem[barrier_no]);
        }
        else
        {
            counter_sema++;
            sem_post(&count_sem);
            sem_wait(&barrier_sem[barrier_no]);
        }
    }  
}

pthread_mutex_t mutex_cond;
pthread_cond_t cond_var;
int counter_cond = 0;


void * thread_work_conditional (void * rank)
{
    long my_rank = (long)rank;

    for (int barrier_no = 0 ; barrier_no < BARRIER_COUNT ; barrier_no++)
    {
        pthread_mutex_lock(&mutex_cond);
        counter_cond++;
        if (counter_cond == thread_count)
        {
            counter_cond = 0;
            pthread_cond_broadcast(&cond_var);
        }
        else
        while(pthread_cond_wait(&cond_var , &mutex_cond) != 0);

        pthread_mutex_unlock(&mutex_cond);
    }
}



int main (int argc , char *argv[])
{
    MAX_THREADS = atoi(argv[1]);

    //for plotting to the file
    FILE *plot_file;
    plot_file = fopen("plt.txt", "w");

    fprintf(plot_file, "Barriers plot\n");
    fprintf(plot_file, "Number of threads\n");
    fprintf(plot_file, "Time taken (average) (in ms)\n");

    double barrier_mutex_time[MAX_THREADS] , barrier_sema_time[MAX_THREADS] , barrier_cond_time[MAX_THREADS];

    for (thread_count = 1 ; thread_count <= MAX_THREADS ; thread_count++)
    {
        printf("\n Threads : %d\n" , thread_count);
        for (int i = 0; i < BARRIER_COUNT ; i++)
            barrier_thread_counts[i] = 0;

        pthread_mutex_init (&barrier_mutex , NULL);

        struct timeval t1, t2;
        double time_elasped;

        pthread_t tids[thread_count];

        gettimeofday(&t1 , NULL);

        for (long i =0 ; i < thread_count ; i++)
            pthread_create(&tids[i] , NULL , thread_work , (void *)i);

        for (int i =0 ; i < thread_count ; i++)
            pthread_join(tids[i] , NULL);

        gettimeofday(&t2 , NULL);
        time_elasped = (t2.tv_sec - t1.tv_sec) * 1000.0;
        time_elasped += (t2.tv_usec - t1.tv_usec) / 1000.0;
        printf("Time for barriers using busy wait and mutex: %lf\n", time_elasped);
        barrier_mutex_time[thread_count - 1] = time_elasped; 

        // ====================================================================================
        for (int i =0 ; i < BARRIER_COUNT ; i++)
            sem_init (&barrier_sem[i] , 0 , 0);
        
        sem_init (&count_sem , 0 , 1);
        counter_sema = 0;

        gettimeofday(&t1, NULL);

        for (long i = 0; i < thread_count; i++)
            pthread_create(&tids[i], NULL, thread_work_sema, (void *)i);

        for (int i = 0; i < thread_count; i++)
            pthread_join(tids[i], NULL);

        gettimeofday(&t2, NULL);
        time_elasped = (t2.tv_sec - t1.tv_sec) * 1000.0;
        time_elasped += (t2.tv_usec - t1.tv_usec) / 1000.0;
        printf("Time for barriers using semaphores : %lf\n", time_elasped);
        barrier_sema_time[thread_count - 1] = time_elasped;

        //==========================================================================================
        //barriers with the help of conditional variables
        counter_cond = 0;
        pthread_cond_init(&cond_var , NULL);
        pthread_mutex_init(&mutex_cond , NULL);

        gettimeofday(&t1, NULL);

        for (long i = 0; i < thread_count; i++)
            pthread_create(&tids[i], NULL, thread_work_conditional, (void *)i);

        for (int i = 0; i < thread_count; i++)
            pthread_join(tids[i], NULL);

        gettimeofday(&t2, NULL);
        time_elasped = (t2.tv_sec - t1.tv_sec) * 1000.0;
        time_elasped += (t2.tv_usec - t1.tv_usec) / 1000.0;
        printf("Time for barriers using conditionals : %lf\n", time_elasped);
        barrier_cond_time[thread_count - 1] = time_elasped;
    }
    //free the memory
    pthread_mutex_destroy(&mutex_cond);
    pthread_mutex_destroy(&barrier_mutex);

    for (int i =0 ; i < BARRIER_COUNT ; i++)
    sem_destroy(&barrier_sem[i]);
    sem_destroy(&count_sem);

    pthread_cond_destroy(&cond_var);

    fprintf(plot_file, "Barrier mutex\n");
    for (int i = 0; i < MAX_THREADS ; i++)
        fprintf(plot_file, "%lf ", barrier_mutex_time[i]);
    fprintf(plot_file, "\n");

    fprintf(plot_file, "Barrier sema\n");
    for (int i = 0; i < MAX_THREADS; i++)
        fprintf(plot_file, "%lf ", barrier_sema_time[i]);
    fprintf(plot_file, "\n");

    fprintf(plot_file, "Barrier conditional\n");
    for (int i = 0; i < MAX_THREADS; i++)
        fprintf(plot_file, "%lf ", barrier_cond_time[i]);
    fprintf(plot_file, "\n");

    fclose(plot_file);

    return 0;
}