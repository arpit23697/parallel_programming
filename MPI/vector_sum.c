#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <mpi.h>

void read_vector (int local_a[] , int local_b[],int  local_n ,int n ,int my_rank ,int comm_sz){

    int *a = NULL;
    int *b = NULL;
    if (my_rank == 0){
        printf("Enter %d Element\n" , n);
        a = (int *)malloc (n * sizeof(int));
        for (int i =0 ; i < n ; i++)
            scanf("%d",&a[i]);
        
        printf("Enter %d element\n" , n);
        b = (int *)malloc(n*sizeof(int));
        for (int i= 0 ; i < n ; i++)   
            scanf("%d",&b[i]);

        MPI_Scatter(a , local_n , MPI_INT , local_a , local_n , MPI_INT , 0 , MPI_COMM_WORLD);
        MPI_Scatter(b , local_n , MPI_INT , local_b , local_n , MPI_INT , 0 , MPI_COMM_WORLD);

    
    }
    else{
        
        MPI_Scatter(a , local_n , MPI_INT , local_a , local_n ,  MPI_INT , 0 , MPI_COMM_WORLD);
        MPI_Scatter(b , local_n , MPI_INT , local_b , local_n , MPI_INT , 0 , MPI_COMM_WORLD );
        // printf("Done \n");
    }
    // printf("done\n");

}

void print_vector (int local_a[] , int local_n , int n , int comm_sz , int my_rank){
    int *a = NULL;
    if (my_rank == 0){
        a = malloc (n * sizeof(int));
        MPI_Gather (local_a , local_n , MPI_INT , a , local_n , MPI_INT , 0 , MPI_COMM_WORLD );
        for (int i =0 ; i < n ; i++)
            printf("%d " , a[i]);

        printf("\n");
    }
    else{
        MPI_Gather (local_a , local_n , MPI_INT , a , local_n , MPI_INT , 0 , MPI_COMM_WORLD);
    }
}

int main (){
    int *local_a , *local_b;

    int my_rank, comm_sz;

    MPI_Init (NULL , NULL);
    MPI_Comm_size (MPI_COMM_WORLD , &comm_sz);
    MPI_Comm_rank (MPI_COMM_WORLD , &my_rank);

    int n = 4;
    int local_n = n / comm_sz;
    local_a = (int *)malloc(local_n * sizeof(int));
    local_b= (int *)malloc (local_n * sizeof(int));
    read_vector(local_a ,local_b , local_n , n , my_rank , comm_sz );

    // printf("Reading done");

    int *ans = malloc (local_n * sizeof(int));
    for (int i =0 ; i < local_n ; i++)
    {
        // printf("%d %d\n" , local_a[i] , local_b[i]);
        ans[i] = local_a[i] + local_b[i];
    }
    print_vector(ans , local_n , n , comm_sz , my_rank);


    MPI_Finalize();
    return 0;


}