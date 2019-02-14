#include <bits/stdc++.h>
#include <time.h>
using namespace std;


int main ()
{
    int n = 500;
    srand(time(0));
    for (int i= 0 ; i < n ; i++)
    cout << (rand() % 100) << " " << rand() % 100 << endl;

    return 0;

}
