#include <bits/stdc++.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
using namespace std;

int main (int argc , char * argv[])
{

    cout << "I am the reporter : Pid is : " << int(getpid()) << endl;
    int god_reporter_read = stoi(argv[1]);
    int reporter_god_write = stoi(argv[2]);


    //now we have to create 4 pipes for communicating with the hare and the turtle
    //pipe for the hare
    int pipefd[2];
    if (pipe(pipefd) == -1)
        perror("Pipe");

    int reporter_hare_read = pipefd[0];
    int reporter_hare_write = pipefd[1];

    if (pipe(pipefd) == -1)
        perror ("Pipe");
    
    int hare_reporter_read = pipefd[0];
    int hare_reporter_write = pipefd[1];

    //pipe for the turtle
    if (pipe(pipefd) == -1)
        perror("Pipe");
    
    int reporter_turtle_read = pipefd[0];
    int reporter_turtle_write = pipefd[1];

    if (pipe(pipefd) == -1)
        perror("Pipe");

    int turtle_reporter_read = pipefd[0];
    int turtle_reporter_write = pipefd[1];

    //this process will create two child processes one is the hare and the other is the turtle
    pid_t pid1 = fork();
    if (pid1 < 0 ) 
    {
        cout << "Error in forking " << endl;
        return 0;
    }
    if (pid1 == 0)
    {
        //this is the first child process so make it hare

        char *args[4];
        //name of the hare process
        string program_name = "./hare";
        args[0] = (char *)program_name.c_str();

        //passing the pipe for communication
        string temp;
        temp = to_string(reporter_hare_read);
        args[1] = (char *)temp.c_str();

        string temp2;
        temp2 = to_string(hare_reporter_write);
        args[2] = (char *)temp2.c_str();

        //NULL
        args[3] = NULL;

        if (execvp(args[0], args) == -1)
            perror("exec");

        exit(0);
    }

    pid_t pid2 = fork();
    if (pid2 < 0)
    {
        cout << "Error in forking process 2" << endl;
        return 0;
    }
    if (pid2 == 0)
    {
        //this is the second child process so make it turtle

        char *args[4];
        //name of the hare process
        string program_name = "./turtle";
        args[0] = (char *)program_name.c_str();

        //passing the pipe for communication
        string temp;
        temp = to_string(reporter_turtle_read);
        args[1] = (char *)temp.c_str();

        string temp2;
        temp2 = to_string(turtle_reporter_write);
        args[2] = (char *)temp2.c_str();

        //NULL
        args[3] = NULL;

        if (execvp(args[0], args) == -1)
            perror("exec");

        exit(0);
    }

    close(reporter_hare_read);
    close(hare_reporter_write);
    close (reporter_turtle_read);
    close(turtle_reporter_write);


    /*This is the main code*/
    
    //first reporter reads the position of the hare and the turtle
    //then Prints it and then send it to the hare and the turtle respectively

    int a, b;
    int counter = 0;

    while (1)
    {
        counter++;
        read(god_reporter_read , &a , sizeof(a));
        read(god_reporter_read , &b , sizeof(b));

        

        write(reporter_hare_write , &a , sizeof(a));
        write(reporter_hare_write , &b , sizeof(b));
        write(reporter_turtle_write , &a , sizeof (a));
        write(reporter_turtle_write , &b , sizeof(b));

        if (a == -10 && b == -10)
            break;

        cout << "Counter : " << counter << endl;
        cout << "Hare position : " << a << ". Turtle position : " << b << endl;

        read (hare_reporter_read , &a , sizeof (a));
        read (turtle_reporter_read , &b , sizeof (b));


        cout << "Hare position : " << a << " Turtle position : " << b << endl;

        write(reporter_god_write , &a , sizeof (a));
        write(reporter_god_write , &b , sizeof (b) );
    }


    int status = 0;
    pid_t pid = wait(&status);
    if (pid == pid1) 
        cout << "Hare exited" << endl;
    else if (pid == pid2) 
        cout << "Turtle exited " << endl;

    pid = wait(&status);
    if (pid == pid1) 
        cout << "Hare exited" << endl;
    else if (pid == pid2 )
        cout << "Turtle exited" << endl;


    return 0;
}