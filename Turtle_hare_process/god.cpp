#include <bits/stdc++.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
using namespace std;


//getpid ()
//this is the function in the unistd.h library and it returns the pid of the current process

//pipe is the system call used for the interprocess communication
//pipefd gives the array with two elements. 
//0th - read end of the file
//1st - write end of the file


int take_step ()
{
    double f = (double)rand() / RAND_MAX;
    if (f <= 0.5)   //generate the steps
        return rand() % 100;
    
    else
        return 4;
}



int main (int argc , char *argv[])
{
    cout << "I am god : Pid is " << (int)getpid() << endl;


    //this is the god process
    //first we are going to create 4 processes god, reporter, turtle, hare
    //god is going to be the parent of the reporter 
    //god creates the reporter and reporter inturn creates the turtle and hare

    int pipefd[2];
    int ret;

    //this is the first pipe for the communication from god to reporter
    ret = pipe(pipefd);
    if (ret == -1)
    {
        perror("Exit");
        exit(1);
    }
    int god_reporter_write = pipefd[1];
    int god_reporter_read = pipefd[0];


    //this is the second for the communication from reporter to god
    ret = pipe(pipefd);
    if (ret == -1)
    {
        perror("Exit");
        exit(1);
    }

    int reporter_god_read = pipefd[0];
    int reporter_god_write = pipefd[1];


    // cout << god_reporter_read << " " << god_reporter_write << " " << reporter_god_read << " " << reporter_god_write << endl;



    pid_t pid = fork();

    if (pid < 0) 
    {
        cout << "Error in forking " << endl;
        return 0;
    }
    if (pid == 0)
    {
        //this is the child process that is created 
        //so this is the reporter process
    
        char* args[4];
    
        //name of the reporter process
        string program_name = "./reporter";
        args[0] = (char*)program_name.c_str();

        //passing the pipe for communication
        string temp;
        temp = to_string(god_reporter_read);
        args[1] = (char *)temp.c_str();
        
        string temp2;
        temp2 = to_string(reporter_god_write);
        args[2] = (char*)temp2.c_str();

        //NULL
        args[3] = NULL;

        if (execvp (args[0] , args) == -1)
            perror ("exec");
        
        exit (0);

    }
    //closing the read end of the file
    close (god_reporter_read);
    close(reporter_god_write);


    int a,b , endline;         //for message transfer

    //god gives the signal to start the race
    //god initially gives 0,0  to the reporter as the starting position of the hare and the turtle
    a=0;
    b=0;
    endline = 100;

    while (1)
    {
        write(god_reporter_write , &a, sizeof(a));
        write(god_reporter_write , &b , sizeof(b));

        read(reporter_god_read , &a , sizeof (a));
        read(reporter_god_read , &b , sizeof (b));
        if (a >= endline)
        {
            cout << "Hare wins the race" << endl;
            a = -10;
            b = -10;
            write(god_reporter_write , &a , sizeof (a));
            write(god_reporter_write,  &b , sizeof (b));
            break;
        }
        
        else if (b >= endline)
        {
            cout << "Turtle wins the race" << endl;
            a = -10;
            b = -10;
            write(god_reporter_write, &a, sizeof(a));
            write(god_reporter_write, &b, sizeof(b));
            break;
        }
        cout << "Enter hare and turtle position : ";
        cin >> a;
        cin >> b;
        // a = take_step();
        // b = take_step();
        a = max (0 , a);
        b = max (0 , b);
    }



    // int a = 20;
    // int b;
    // read (reporter_god_read , &a , sizeof(a));
    // cout << a << " another " << endl;    
    
    // a = 100;
    // write(god_reporter_write , &a , sizeof(a));
    
    int status = 0;
    pid_t reporter_pid = wait (&status);
    

    
    // cout << "Reporter process which exited is " << (int)reporter_pid << " and status is " << status << endl;
    // int returnvalue = WEXITSTATUS(status);
    // cout << "The return value is : " << returnvalue << endl;

    //closing the pipe
    close (god_reporter_write);
    close(reporter_god_read);
    
    return 0;
}