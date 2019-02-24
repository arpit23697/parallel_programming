// This program calculates the overhead in the calling of pthread_create and pthread_join

#include <bits/stdc++.h>
#include <chrono>
#include <time.h>
#include <pthread.h>
#include <thread>
#include <semaphore.h>
using namespace std;

//this thread does no work and immediately returns
//So this can be used to compute the overhead in pthread_create and pthread_join
void *pthread_time_checker(void *arg)
{
    return NULL;
}

int main()
{
   
    pthread_t tid;
    //Computes the overhead in just calling the pthread
    int n = 1000;
    auto start = chrono::high_resolution_clock::now();
    pthread_create(&tid, NULL, pthread_time_checker, NULL);
    pthread_join(tid, NULL);
    auto end = chrono::high_resolution_clock::now();
    cout << "Time for pthraed_create and pthread_join : " << chrono::duration_cast<chrono::nanoseconds>(end - start).count() << endl;

    //overhead for the mutex call function
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    start = chrono::high_resolution_clock::now();
    pthread_mutex_lock(&mutex);
    pthread_mutex_unlock(&mutex);
    end = chrono::high_resolution_clock::now();
    cout << "Time for mutex lock and unlock function call :" << chrono::duration_cast<chrono::nanoseconds>(end - start).count() << endl;
    pthread_mutex_destroy(&mutex);

    //overhead for the semaphore lock and unlock function call
    sem_t sema;
    sem_init(&sema, 0, 0); //2nd parameter is pshared indicating wether it needs to be shared or not
    start = chrono::high_resolution_clock::now();
    sem_post(&sema);
    sem_wait(&sema);
    end = chrono::high_resolution_clock::now();
    auto time_taken = chrono::duration_cast<chrono::nanoseconds>(end - start).count();
    cout << "Time for sem lock and unlock function call : " << time_taken << endl;

    //pthread_read_write_lock
    pthread_rwlock_t lock;
    pthread_rwlock_init(&lock, NULL);
    start = chrono::high_resolution_clock::now();
    pthread_rwlock_wrlock(&lock);
    pthread_rwlock_unlock(&lock);
    end = chrono::high_resolution_clock::now();
    time_taken = chrono::duration_cast<chrono::nanoseconds>(end - start).count();
    cout << "Time for rwlock_rdlock overhead : " << time_taken << endl;

    // cout << CLOCKS_PER_SEC << endl;
    return 0;
}