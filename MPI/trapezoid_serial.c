#include <stdio.h>
#include <math.h>
//calculate the area under exponential function

double funct(double x){
    return exp(x);
}


int main ()
{
    int a,b,n;
    scanf("%d %d %d" , &a , &b , &n);
    double h = ((b - a) * 1.0)/n ;


    double approx = 0;
    approx = (funct(a) + funct(b)) / 2.0;

    for (int i = 1 ; i <= n-1 ; i++)
    {
        double temp = a + i*h;
        approx += funct(temp);
    }
    printf("Apprximated value : %lf\n" , approx * h);
    return 0;
}