#include <bits/stdc++.h>
#include <stdlib.h>
#include <semaphore.h>
#include <pthread.h>

using namespace std;

//Declaring all the semaphores which are required for synchronisation in between the processes
sem_t god_reporter , reporter_god , reporter_hare , reporter_turtle , hare_reporter , turtle_reporter;

//Declaring the global variables used by the threads
pthread_mutex_t positions_lock = PTHREAD_MUTEX_INITIALIZER;

//Declaring the global variables;
int hare_pos , turtle_pos , endline ;

int take_step()
{
    double f = (double)rand() / RAND_MAX;
    if (f <= 0.5) //generate the steps
        return rand() % 100;

    else
        return 4;
}

//God thread
void * god_thread (void * arg)
{
    while (1)
    {
        sem_post (&god_reporter);
        sem_wait (&reporter_god);
        // cout << "Hare position : " << hare_pos << " Turtle_pos : " << turtle_pos  << endl;

        if (hare_pos >= endline)
        {
            cout << "Hare wins the race" << endl;
            hare_pos = -10;
            turtle_pos = -10;
            sem_post (&god_reporter);
            break;
        }
        if (turtle_pos >= endline)
        {
            cout << "Turtle wins the race" << endl;
            hare_pos = -10;
            turtle_pos = -10;
            sem_post (&god_reporter);
            break;
        }
        cout << "Enter hare and turtle position : ";
        cin >> hare_pos;
        cin >> turtle_pos;


    }
    pthread_exit(NULL);
    
}

void * reporter_thread (void *arg)
{
    int counter = 0;
    while (1)
    {
        counter+=1;
        
        sem_wait (&god_reporter);
    
        //printing the answers
        if (hare_pos == -10 && turtle_pos == -10)
        {
            sem_post(&reporter_hare);
            sem_post(&reporter_turtle);
            break;
        }
        cout << "Counter :" << counter << endl;
        cout << "Hare position : " << hare_pos << " Turtle_pos : " << turtle_pos << endl;


        sem_post (&reporter_hare);
        sem_post (&reporter_turtle);
    
        sem_wait (&hare_reporter);
        sem_wait (&turtle_reporter);
    
        cout << "Hare position :" << hare_pos << " Turtle_pos : " << turtle_pos << endl;
    
        sem_post (&reporter_god);
        
    }
    
    pthread_exit(NULL);
}
    
//thread for hare
void * hare_thread (void * arg)
{   
    int counter = 1;
    while (1)
    {
        sem_wait (&reporter_hare);
        pthread_mutex_lock (&positions_lock);

        if (hare_pos == -10 && turtle_pos == -10){
            pthread_mutex_unlock(&positions_lock);
            break;
        }
        if (hare_pos - turtle_pos <= 10){
            hare_pos += counter;
            counter++;
        }
        pthread_mutex_unlock(&positions_lock);
        sem_post (&hare_reporter);
    }
    
    pthread_exit (NULL);

}

// This is the thread for the turtle 
void * turtle_thread (void *arg)
{
    while (1)
    {
        sem_wait (&reporter_turtle);
        pthread_mutex_lock (&positions_lock);

        if (hare_pos == -10 && turtle_pos == -10)
        {
            pthread_mutex_unlock(&positions_lock);
            break;
        }

        turtle_pos += 1;
        pthread_mutex_unlock (&positions_lock);
        sem_post (&turtle_reporter);
    }
    pthread_exit (NULL);
}



int main ()
{
    //initialising all the semaphores
    sem_init (&god_reporter , 0 , 0);
    sem_init (&reporter_god , 0 , 0);
    sem_init (&reporter_hare , 0 , 0);
    sem_init (&reporter_turtle , 0 , 0);
    sem_init (&hare_reporter , 0 , 0);
    sem_init (&turtle_reporter , 0 , 0);

    //intialise the global variables 
    hare_pos = 0;
    turtle_pos = 0;
    endline = 100;


    pthread_t god, reporter , hare , turtle;

    //spawning the thread
    pthread_create (&god, NULL , god_thread , NULL);
    pthread_create(&hare, NULL, hare_thread, NULL);
    pthread_create(&turtle, NULL, turtle_thread, NULL);
    pthread_create(&reporter, NULL, reporter_thread, NULL);

    //exiting the thread by joining
    pthread_join (god , NULL);
    pthread_join (reporter , NULL);
    pthread_join (turtle , NULL);
    pthread_join (hare , NULL);

    //semaphore destroy
    sem_destroy (&god_reporter);
    sem_destroy (&reporter_god);
    sem_destroy(&reporter_hare);
    sem_destroy(&reporter_turtle);
    sem_destroy(&hare_reporter);
    sem_destroy(&turtle_reporter);

    return 0;

}