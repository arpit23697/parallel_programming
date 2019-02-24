#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <semaphore.h>
#include <unistd.h>
#include <pthread.h>

#define BINS 100
int hist[BINS];
int MAX_THREADS = 4;

struct arguments
{
    int rank;
    long long start;
    long long end;
};

pthread_mutex_t mutex1;
int nthreads;
int flag = 0;

int* input;

//this is using mutex
void *calculate_mutex(void *args)
{
    struct arguments *arg_struct = (struct arguments *)args;
    int rank = arg_struct->rank;
    long long start = arg_struct->start;
    long long end = arg_struct->end;
    for (int i = start; i < end; i++)
    {
        pthread_mutex_lock(&mutex1);
        hist[input[i] % 100] += 1;
        pthread_mutex_unlock(&mutex1);
    }
    return NULL;
}

//this is using mutex
void *calculate_busyWait(void *args)
{
    struct arguments *arg_struct = (struct arguments *)args;
    int rank = arg_struct->rank;
    long long start = arg_struct->start;
    long long end = arg_struct->end;
    for (int i = start; i < end; i++)
    {
        while(flag != rank);
        hist[input[i] % 100] += 1;
        flag = (flag + 1) % nthreads;
    }
    return NULL;
}

struct arguments2
{
    long long start;
    long long end;
    int hist_local[BINS];
};

void *calculate_local(void *args)
{
    struct arguments2 *arg_struct = (struct arguments2 *)args;
    long long start = arg_struct->start;
    long long end = arg_struct->end;
    for (int i = start; i < end; i++)
        arg_struct->hist_local[input[i] % 100] += 1;
    return NULL;
}

int main(int argc , char *argv[])
{
    int n;
    n = atoi(argv[1]);
    MAX_THREADS = atoi(argv[2]);


    //for plotting to the file
    FILE *plot_file;
    plot_file = fopen("plt.txt" , "w");

    fprintf(plot_file, "Histogram plot n = : %d\n" , n);
    fprintf(plot_file, "Number of threads\n");
    fprintf(plot_file, "Time taken (average) (in ms)\n");
 
    double shared_mutex_time[MAX_THREADS] , shared_wait_time[MAX_THREADS] , local_time[MAX_THREADS];


    for (nthreads = 1 ; nthreads <= MAX_THREADS ; nthreads++)
    {
        flag = 0;
        // scanf("%d",&n);
        //Generating the n numbers in the range 0 to 100
        for (int i = 0; i < BINS ; i++)
            hist[i]=0;

        input = (int *)malloc(n * sizeof(int));
        for (int i =0 ; i < n ; i++)
            input[i] = rand() % 100;

        //start measuring the time
        struct timeval start , end;
        double time_elasped;

        gettimeofday(&start , NULL);
        pthread_t tids[nthreads];
        struct arguments args[nthreads];

        for (int i = 0; i < nthreads; i++)
        {
            args[i].rank = i;
            args[i].start = n / nthreads * i;
            args[i].end = n / nthreads * (i + 1);
            pthread_create(&tids[i], NULL, calculate_mutex, &args[i]);
        }

        for (int i = 0; i < nthreads; i++)
            pthread_join(tids[i], NULL);

        // for (int i = 0; i < 100; i++)
        //     printf("%d ", hist[i]);
        // printf("\n");
        
        gettimeofday(&end , NULL);
        time_elasped = (end.tv_sec - start.tv_sec) * 1000.0;
        time_elasped += (end.tv_usec - start.tv_usec) / 1000.0;
        printf("Threads : %d,n : %d,Time taken shared mutex: %lf ms\n",nthreads , n, time_elasped);
        shared_mutex_time[nthreads - 1] = time_elasped;
        // ==============================================================================================
        // for the calculation using the busy wait
        for (int i = 0; i < BINS; i++)
            hist[i] = 0;
        gettimeofday(&start, NULL);
        for (int i = 0; i < nthreads; i++)
        {
            args[i].rank = i;
            args[i].start = n / nthreads * i;
            args[i].end = n / nthreads * (i + 1);
            pthread_create(&tids[i], NULL, calculate_busyWait, &args[i]);
        }

        for (int i = 0; i < nthreads; i++)
            pthread_join(tids[i], NULL);

        // for (int i = 0; i < 100; i++)
        //     printf("%d ",hist[i]);
        // printf("\n");

        gettimeofday(&end, NULL);
        time_elasped = (end.tv_sec - start.tv_sec) * 1000.0;
        time_elasped += (end.tv_usec - start.tv_usec) / 1000.0;
        printf("Threads : %d,n : %d,Time taken shared busy wait : %lf ms\n", nthreads, n, time_elasped);
        shared_wait_time[nthreads - 1] = time_elasped;
        // ==================================================================================
        //This is for the shared
        struct arguments2 args2[nthreads];

        gettimeofday(&start , NULL);

        for (int i = 0; i < nthreads; i++)
        {
            args2[i].start = n / nthreads * i;
            args2[i].end = n / nthreads * (i + 1);
            for (int j= 0 ; j < BINS ; j++)
                args2[i].hist_local[j] = 0;
            pthread_create(&tids[i], NULL, calculate_local, &args2[i]);
        }
        // printf("Hi theree\n");
        int global_hist[BINS];
        for (int i =0 ; i < BINS ; i++)
            global_hist[i] =0;
        for (int i = 0; i < nthreads; i++)
        {
            pthread_join(tids[i], NULL);
            for (int j = 0; j < 100; j++)
                global_hist[j] += args2[i].hist_local[j];
        }
        
        gettimeofday(&end, NULL);
        time_elasped = (end.tv_sec - start.tv_sec) * 1000.0;
        time_elasped += (end.tv_usec - start.tv_usec) / 1000.0;
        printf("Threads : %d,n : %d, Time taken local: %lf ms\n", nthreads, n, time_elasped);
        local_time[nthreads - 1] = time_elasped; 
    }

    fprintf(plot_file , "Shared mutex\n");
    for (int i =0 ; i < MAX_THREADS ; i++)
        fprintf(plot_file , "%lf " , shared_mutex_time[i]);
        fprintf(plot_file , "\n");

    fprintf(plot_file, "Shared busywait\n");
    for (int i = 0; i < MAX_THREADS; i++)
        fprintf(plot_file, "%lf ", shared_wait_time[i]);
    fprintf(plot_file, "\n");

    fprintf(plot_file, "Local\n");
    for (int i = 0; i < MAX_THREADS; i++)
        fprintf(plot_file, "%lf ", local_time[i]);
    fprintf(plot_file, "\n");

    fclose(plot_file);
    return 0;
}