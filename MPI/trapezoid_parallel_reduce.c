#include <stdio.h>
#include <mpi.h>
#include <math.h>

double function(double x)
{
    return exp(x);
}

double trap(double left_endpoint, double right_endpoint, double h, int local_count)
{
    double approx = (function(left_endpoint) + function(right_endpoint)) / 2.0;

    for (int i = 1; i < local_count; i++)
    {
        double temp = left_endpoint + i * h;
        approx += function(temp);
    }
    return approx * h;
}

void Get_input(double *a_p, double *b_p, int *n, int my_rank, int comm_sz)
{
    // printf ("%d %d\n" , my_rank , comm_sz);
    if (my_rank == 0)
    {
        printf("Enter the value of a,b and n\n");
        scanf("%lf %lf %d", a_p, b_p, n);

        for (int i = 1; i < comm_sz; i++)
        {
            MPI_Send(a_p, 1, MPI_DOUBLE, i, 0, MPI_COMM_WORLD);
            MPI_Send(b_p, 1, MPI_DOUBLE, i, 0, MPI_COMM_WORLD);
            MPI_Send(n, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
        }
    }
    else
    {
        MPI_Recv(a_p, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(b_p, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(n, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }
}

int main()
{

    double a, b;
    int n;

    int my_rank, comm_sz;
    MPI_Init(NULL, NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    Get_input(&a, &b, &n, my_rank, comm_sz);
    int local_n = n / comm_sz;
    double h = (b - a) / n;
    double local_a = a + my_rank * local_n * h;
    double local_b = local_a + local_n * h;

    double local_ans = trap(local_a, local_b, h, local_n);
    double final_ans = 0;

    MPI_Reduce (&local_ans , &final_ans , 1 , MPI_DOUBLE , MPI_SUM , 0 , MPI_COMM_WORLD);
    if (my_rank == 0){
        printf("Approximated value is : %lf \n" , final_ans);
    }

    MPI_Finalize();
    return 0;
}