#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>
#include <math.h>
double function (double x){
    return exp(x);
}

double trap (double left_endpoint , double right_endpoint , double h , int trap_count){

    double approx = 0;
    approx = (function (left_endpoint) + function (right_endpoint)) / 2.0;

    for (int i = 1; i < trap_count ; i++)
    {
        double temp = left_endpoint + h * i;
        approx += function(temp);
    }
    return approx * h;
}

int main ()
{
    double local_a , local_b;
    int local_n;

    int n = 100000000;
    double a = 2.0 , b = 5.0;
    double h = (b-a)/n;



    int my_rank, comm_sz;

    MPI_Init (NULL , NULL);
    MPI_Comm_size (MPI_COMM_WORLD , &comm_sz);
    MPI_Comm_rank (MPI_COMM_WORLD , &my_rank);

    local_n = n/comm_sz;
    local_a = a + my_rank * local_n * h;
    local_b = local_a + local_n * h;

    double local_ans = trap (local_a , local_b,  h , local_n);
    

    if (my_rank != 0){
        MPI_Send (&local_ans , 1 ,MPI_DOUBLE , 0 , 0 , MPI_COMM_WORLD);
    }
    else
    {
        double total_ans = local_ans;
        for (int i = 1; i < comm_sz ; i++){
            MPI_Recv (&local_ans , 1 , MPI_DOUBLE , i , 0 , MPI_COMM_WORLD , MPI_STATUS_IGNORE );
            total_ans += local_ans;
        }
        printf("Approximate value is : %lf\n" , total_ans );
    }
    
    MPI_Finalize();
    return 0;
}