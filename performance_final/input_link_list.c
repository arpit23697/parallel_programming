#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <semaphore.h>
#include <unistd.h>
#include <pthread.h>


int main(int argc, char *argv[])
{
    int n = atoi(argv[1]);
    int member_proportion = atoi(argv[2]);
    int insert_proportion = atoi(argv[3]);
    int delete_proportion = atoi(argv[4]);
    // cout << n << " " << member_proportion << " " << insert_proportion << " " << delete_proportion << endl;
    srand(time(0));
    printf("%d\n",n);
    int op, proportion;
    for (int i = 0; i < n; i++)
    {
        proportion = rand() % 100;
        // cout << proportion << endl;
        if (proportion <= member_proportion)
            op = 0;
        else if (proportion <= insert_proportion + member_proportion)
            op = 1;
        else
            op = 2;
        printf("%d %d\n",op , rand() % 500);

        // cout << op << " " << rand() % 500 << endl;
    }
    return 0;
}