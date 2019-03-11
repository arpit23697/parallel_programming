#include <stdio.h>
#include <mpi.h>
#include <math.h>

double function(double x)
{
    return exp(x);
}

void build_mpi_type (double * a_p , double *b_p , int *n , MPI_Datatype *input_datatype)
{
    int array_of_blocklengths[3] = {1,  1,1};
    MPI_Datatype array_of_types[3] = {MPI_DOUBLE , MPI_DOUBLE , MPI_INT};
    MPI_Aint a_addr , b_addr , n_addr;

    MPI_Get_address (a_p , &a_addr );
    MPI_Get_address (b_p, &b_addr);
    MPI_Get_address (n , &n_addr);

    MPI_Aint array_of_displacements[3] = {0 , b_addr - a_addr , n_addr - a_addr};
    MPI_Type_create_struct (3 , array_of_blocklengths , array_of_displacements , array_of_types, input_datatype);
    MPI_Type_commit (input_datatype);
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
    MPI_Datatype input_datatype;
    // printf("Hi therr\n");
    build_mpi_type(a_p , b_p , n , &input_datatype);
    
    if (my_rank == 0)
    {
        printf("Enter the value of a,b and n\n");
        scanf("%lf %lf %d", a_p, b_p, n);
    }

    MPI_Bcast (a_p , 1 , input_datatype , 0 , MPI_COMM_WORLD);
    MPI_Type_free (&input_datatype);
}

int main()
{

    double a, b;
    int n;

    int my_rank, comm_sz;
    MPI_Init(NULL, NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    // printf("hya u");
    Get_input(&a, &b, &n, my_rank, comm_sz);
    int local_n = n / comm_sz;
    double h = (b - a) / n;
    double local_a = a + my_rank * local_n * h;
    double local_b = local_a + local_n * h;

    double local_ans = trap(local_a, local_b, h, local_n);
    double final_ans = 0;

    MPI_Reduce(&local_ans, &final_ans, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    if (my_rank == 0)
    {
        printf("Approximated value is : %lf \n", final_ans);
    }

    MPI_Finalize();
    return 0;
}