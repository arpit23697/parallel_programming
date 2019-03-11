#include <stdio.h>
#include <mpi.h>
int main (){
    int comm_sz , my_rank;

    MPI_Init (NULL , NULL);
    MPI_Comm_size (MPI_COMM_WORLD , &comm_sz);
    MPI_Comm_rank (MPI_COMM_WORLD , &my_rank);

    printf ("My rank is %d of %d\n" , my_rank , comm_sz);

    MPI_Finalize();
}