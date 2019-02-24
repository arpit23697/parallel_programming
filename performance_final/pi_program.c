#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <semaphore.h>
#include <unistd.h>
#include <pthread.h>


long long int n;
int MAX_THREADS = 5;

struct arguments
{
    long long start;
    long long end;
    int rank;
};

double global_sum = 0;
int number_of_thread;
pthread_mutex_t sum_mutex;
int flag = 0;

void *calculate(void *args)
{
    //type cast
    struct arguments *arg_struct = (struct arguments *)args;
    long long start = arg_struct->start;
    long long end = arg_struct->end;
    int rank = arg_struct->rank;
    double ans = 0;
    for (int i = start; i < end; i++)
    {
        //add
        if (i % 2 == 0)
            ans += 1.0 / (2 * i + 1);
        else
            ans -= 1.0 / (2 * i + 1);
    }
    pthread_mutex_lock(&sum_mutex);
    global_sum += ans;
    pthread_mutex_unlock(&sum_mutex);
    pthread_exit(NULL);
}

void *calculate_mutexNoLocal(void *args)
{
    //type cast
    struct arguments *arg_struct = (struct arguments *)args;
    long long start = arg_struct->start;
    long long end = arg_struct->end;
    int rank = arg_struct->rank;
    for (int i = start; i < end; i++)
    {
        //add
        if (i % 2 == 0)
        {
            pthread_mutex_lock(&sum_mutex);
            global_sum += 1.0 / (2 * i + 1);
            pthread_mutex_unlock(&sum_mutex);
        }
        else
        {
            pthread_mutex_lock(&sum_mutex);
            global_sum -= 1.0 / (2 * i + 1);
            pthread_mutex_unlock(&sum_mutex);
        }
    }
    pthread_exit(NULL);
}
#pragma optimize("", off);
void *calculate_busyWait(void *args)
{
    //type cast
    struct arguments *arg_struct = (struct arguments *)args;
    long long start = arg_struct->start;
    long long end = arg_struct->end;
    int rank = arg_struct->rank;
    double ans = 0;
    for (int i = start; i < end; i++)
    {
        //add
        if (i % 2 == 0)
            ans += 1.0 / (2 * i + 1);
        else
            ans -= 1.0 / (2 * i + 1);
    }
    while (flag != rank);
    global_sum += ans;
    flag = (flag + 1) % number_of_thread;
    pthread_exit(NULL);
}
#pragma optimize("", on);
#pragma optimize("", off);
void *calculate_busywaitLocal(void *args)
{
    //type cast
    struct arguments *arg_struct = (struct arguments *)args;
    long long start = arg_struct->start;
    long long end = arg_struct->end;
    int rank = arg_struct->rank;
    for (int i = start; i < end; i++)
    {

        //add
        if (i % 2 == 0)
        {
            while (flag != rank)
                ;
            global_sum += 1.0 / (2 * i + 1);
            flag = (flag + 1) % number_of_thread;
        }
        else
        {
            while (flag != rank)
                ;
            global_sum -= 1.0 / (2 * i + 1);
            flag = (flag + 1) % number_of_thread;
        }
    }
    pthread_exit(NULL);
}
#pragma optimize("", on);


int main(int argc, char *argv[])
{
    pthread_mutex_init(&sum_mutex , NULL);
    
    struct timeval start , end;
    double time_elasped;

    //this can be used to find the number of thread in the c++ program
    MAX_THREADS = atoi(argv[1]);
    int n = atoi(argv[2]);

    FILE *plot_file;
    plot_file = fopen("plt.txt", "w");

    fprintf(plot_file, "Pi calculation n = : %d\n", n);
    fprintf(plot_file, "Number of threads\n");
    fprintf(plot_file, "Time taken (average) (in ms)\n");
    double mutex_global_time[MAX_THREADS], mutex_local_time[MAX_THREADS], busy_global_time[MAX_THREADS], busy_local_time[MAX_THREADS];

    //phase 1
    //============================================================================================================================
    for (number_of_thread = 1 ; number_of_thread <=MAX_THREADS ; number_of_thread++)
    {
        printf("===============Thread %d ===================\n" , number_of_thread);
        pthread_t tids[number_of_thread];
        struct arguments args[number_of_thread];

        global_sum = 0;
        gettimeofday(&start , NULL);
        for (int i = 0; i < number_of_thread; i++)
        {
            args[i].start = n / number_of_thread * i;
            args[i].end = n / number_of_thread * (i + 1);
            args[i].rank = i;
            pthread_create(&tids[i], NULL, calculate, &args[i]);
        }

        for (int i = 0; i < number_of_thread; i++)
            pthread_join(tids[i], NULL);

        printf("Global sum : %lf\n" , global_sum);
        gettimeofday(&end , NULL);

        time_elasped = (end.tv_sec - start.tv_sec) * 1000.0;
        time_elasped += (end.tv_usec - start.tv_usec) / 1000.0;
        printf("Time taken mutex only on global sum with local sum: %lf\n" , time_elasped);
        mutex_local_time[number_of_thread - 1] = time_elasped;
        //this is for phase 2
        //================================================================================================================================================================
        global_sum = 0;
        gettimeofday(&start , NULL);
        for (int i = 0; i < number_of_thread; i++)
        {
            args[i].start = n / number_of_thread * i;
            args[i].end = n / number_of_thread * (i + 1);
            args[i].rank = i;
            pthread_create(&tids[i], NULL, calculate_mutexNoLocal, &args[i]);
        }

        for (int i = 0; i < number_of_thread; i++)
            pthread_join(tids[i], NULL);

        gettimeofday(&end , NULL);
        printf("Global sum : %lf\n", global_sum);
        time_elasped = (end.tv_sec - start.tv_sec) * 1000.0;
        time_elasped += (end.tv_usec - start.tv_usec) / 1000.0;
        printf("Time taken mutex on global sum no local sum : %lf\n" , time_elasped);
        mutex_global_time[number_of_thread - 1] = time_elasped;
        pthread_mutex_destroy(&sum_mutex);

        //this is for busy wait
        //=========================================================================================================================================================
        global_sum = 0;
        gettimeofday(&start , NULL);
        for (int i = 0; i < number_of_thread; i++)
        {
            args[i].start = n / number_of_thread * i;
            args[i].end = n / number_of_thread * (i + 1);
            args[i].rank = i;
            pthread_create(&tids[i], NULL, calculate_busyWait, &args[i]);
        }

        for (int i = 0; i < number_of_thread; i++)
            pthread_join(tids[i], NULL);

        printf("Global sum : %lf\n", global_sum);
        gettimeofday(&end , NULL);
        time_elasped = (end.tv_sec - start.tv_sec) * 1000.0;
        time_elasped += (end.tv_usec - start.tv_usec) / 1000.0;
        printf("Time taken busy wait with local sum : %lf\n" , time_elasped);
            busy_local_time[number_of_thread - 1] = time_elasped;
        // =======================================================================================
        //busy wait with no local variable
        global_sum = 0;
        gettimeofday(&start , NULL);
        for (int i = 0; i < number_of_thread; i++)
        {
            args[i].start = n / number_of_thread * i;
            args[i].end = n / number_of_thread * (i + 1);
            args[i].rank = i;
            pthread_create(&tids[i], NULL, calculate_busywaitLocal, &args[i]);
        }

        for (int i = 0; i < number_of_thread; i++)
            pthread_join(tids[i], NULL);

        printf("Global sum : %lf\n", global_sum);
        gettimeofday(&end , NULL);
        time_elasped = (end.tv_sec - start.tv_sec) * 1000.0;
        time_elasped += (end.tv_usec - start.tv_usec) / 1000.0;
        printf("Time taken busy wait without local sum : %lf\n",time_elasped);
        busy_global_time[number_of_thread - 1] = time_elasped;
    }

    fprintf(plot_file, "mutex local time\n");
    for (int i = 0; i < MAX_THREADS; i++)
        fprintf(plot_file, "%lf ", mutex_local_time[i]);
    fprintf(plot_file, "\n");

    fprintf(plot_file, "mutex global time\n");
    for (int i = 0; i < MAX_THREADS; i++)
        fprintf(plot_file, "%lf ", mutex_global_time[i]);
    fprintf(plot_file, "\n");

    fprintf(plot_file, "busy local time\n");
    for (int i = 0; i < MAX_THREADS; i++)
        fprintf(plot_file, "%lf ", busy_local_time[i]);
    fprintf(plot_file, "\n");

    fprintf(plot_file, "busy global time\n");
    for (int i = 0; i < MAX_THREADS; i++)
        fprintf(plot_file, "%lf ", busy_global_time[i]);
    fprintf(plot_file, "\n");


    pthread_mutex_destroy(&sum_mutex);
    return 0;
}