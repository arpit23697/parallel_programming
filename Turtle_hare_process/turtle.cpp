#include <bits/stdc++.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
using namespace std;

int main(int argc, char *argv[])
{

    //cout << "I am the Turtle : Pid is : " << int(getpid()) << endl;
    int reporter_turtle_read = stoi(argv[1]);
    int turtle_reporter_write = stoi(argv[2]);

    int tur_pos, hare_pos;


    while(1)
    {
        
        read(reporter_turtle_read , &hare_pos , sizeof(hare_pos));
        read(reporter_turtle_read , &tur_pos , sizeof (tur_pos));
        if (tur_pos == -10 && hare_pos == -10)
            break;

        tur_pos = tur_pos + 1;
        write(turtle_reporter_write , &tur_pos , sizeof(tur_pos));

    }

    return 0;

}