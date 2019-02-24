// This program calculates the overhead in the calling of pthread_create and pthread_join
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <semaphore.h>
#include <unistd.h>
#include <pthread.h>

//this thread does no work and immediately returns
//So this can be used to compute the overhead in pthread_create and pthread_join
void *pthread_time_checker(void *arg)
{
    return NULL;
}

int main()
{   

    struct timeval start , end;
    double time_elasped;

    //opening the file
    FILE *plot_file;
    plot_file = fopen ("plt.txt" , "w");


    fprintf(plot_file , "Functional overhead of thread creation\n");
    fprintf(plot_file , "Number of threads\n");
    fprintf(plot_file , "Time taken (average) (in ms)\n");
    fprintf(plot_file , "Avergage time taken (in ms)\n");

    //overhead for creating n threads
    int total = 500;
    for (int n = 1; n <= total; n++)
    {
        pthread_t tids[n];

        //this creates n threads and then joins them
        //overall function overhead of creating n threads
        gettimeofday(&start, NULL);
        for (int i = 1; i <= n; i++)
            pthread_create(&tids[i], NULL, pthread_time_checker, NULL);

        for (int i = 1; i <= n; i++)
            pthread_join(tids[i], NULL);

        gettimeofday(&end , NULL);
        time_elasped = (end.tv_sec - start.tv_sec) * 1000.0;
        time_elasped += (end.tv_usec - start.tv_usec) / 1000.0;
        // printf("Time for %d pthread creation and joining : %lf ms\n",n ,time_elasped/n);

        fprintf(plot_file , "%lf " ,  time_elasped);
    }
    fprintf(plot_file , "\n");

    //functional overhead for creating single thread
    pthread_t tid;
    gettimeofday(&start , NULL);
    pthread_create(&tid , NULL , pthread_time_checker , NULL);
    pthread_join(tid , NULL);
    gettimeofday(&end , NULL);

    time_elasped = (end.tv_sec - start.tv_sec) * 1000.0;
    time_elasped += (end.tv_usec - start.tv_usec) / 1000.0;
    printf("Time for creation and joining of one thread : %lf ms\n",time_elasped);


    // //overhead for the mutex call function
    pthread_mutex_t mutex_lock;
    pthread_mutex_init(&mutex_lock , NULL );
    gettimeofday(&start, NULL);
    pthread_mutex_lock(&mutex_lock);
    pthread_mutex_unlock(&mutex_lock);
    gettimeofday(&end , NULL);

    time_elasped = (end.tv_sec - start.tv_sec) * 1000.0;
    time_elasped += (end.tv_usec - start.tv_usec) / 1000.0;
    printf("Time for one mutex lock and unlock : %lf ms\n", time_elasped);

    // //overhead for the semaphore lock and unlock function call
    sem_t sema;
    sem_init(&sema, 0, 0); //2nd parameter is pshared indicating wether it needs to be shared or not
    gettimeofday(&start , NULL);
    sem_post(&sema);
    sem_wait(&sema);
    gettimeofday(&end, NULL);
    time_elasped = (end.tv_sec - start.tv_sec) * 1000.0;
    time_elasped += (end.tv_usec - start.tv_usec) / 1000.0;
    printf("Time for one sem_post and sem_wait : %lf ms\n", time_elasped);

    //pthread_read_write_lock
    pthread_rwlock_t lock;
    pthread_rwlock_init(&lock, NULL);
    
    gettimeofday(&start , NULL);
    pthread_rwlock_rdlock(&lock);
    pthread_rwlock_unlock(&lock);
    gettimeofday(&end , NULL);
    time_elasped = (end.tv_sec - start.tv_sec) * 1000.0;
    time_elasped += (end.tv_usec - start.tv_usec) / 1000.0;
    printf("Time for one rwlock (rdlock and unlock) : %lf ms\n", time_elasped);
    

    // // cout << CLOCKS_PER_SEC << endl;
    return 0;
}