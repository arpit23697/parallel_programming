#include <bits/stdc++.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
using namespace std;

int main(int argc, char *argv[])
{

    //cout << "I am the Hare : Pid is : " << int(getpid()) << endl;
    int reporter_hare_read = stoi(argv[1]);
    int hare_reporter_write = stoi(argv[2]);

    int hare_pos , turtle_pos;
    int counter = 1;
    while (1)
    {
        read(reporter_hare_read , &hare_pos , sizeof(hare_pos));
        read(reporter_hare_read , &turtle_pos , sizeof (turtle_pos));

        if (hare_pos == -10 && turtle_pos == -10)
            break;


        if (hare_pos - turtle_pos <= 10)
        {
            hare_pos = hare_pos+counter;
            counter++;
        }
        write(hare_reporter_write, &hare_pos , sizeof(hare_pos));
    }
    return 0;

}