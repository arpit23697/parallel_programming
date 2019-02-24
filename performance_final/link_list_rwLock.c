#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <semaphore.h>
#include <unistd.h>
#include <pthread.h>


struct node
{
    int value;
    struct node *next;
};

struct node *head = NULL;
int operations[10000000][2];
int n;

int MAX_THREADS;    
pthread_rwlock_t link_list_lock;

int isMember(int value)
{
    //traversing the node
    struct node *current_node = head;
    while (current_node != NULL && current_node->value < value)
        current_node = current_node->next;

    //return false if the end of the list or the value is not found
    if (current_node == NULL || current_node->value != value)
        return 0;
    else
        return 1;
}

int insert_node(int value)
{
    struct node *current_node = head;
    struct node *pred_node = NULL;
    struct node *new_node;

    while (current_node != NULL && current_node->value < value)
    {
        pred_node = current_node;
        current_node = current_node->next;
    }

    //insert the value if the value is not in the list
    if (current_node == NULL || current_node->value != value)
    {
        //insert the node
        new_node = (struct node *)malloc(sizeof(struct node *));
        new_node->value = value;
        new_node->next = current_node;
        if (pred_node == NULL)
            head = new_node;
        else
            pred_node->next = new_node;
        return 1;
    }
    else
        return 0;
}

int delete_node(int value)
{
    struct node *current_node = head;
    struct node *pred_node = NULL;

    while (current_node != NULL && current_node->value < value)
    {
        pred_node = current_node;
        current_node = current_node->next;
    }

    if (current_node != NULL && current_node->value == value)
    {

        if (pred_node == NULL)
        {
            head = current_node->next;
            free(current_node);
        }
        else
        {
            pred_node->next = current_node->next;
            free(current_node);
        }
        return 1;
    }
    else
        return 0;
}

//Helper function for printing the list
void printList(struct node *head)
{
    struct node *current_node = head;
    while (current_node != NULL)
    {
        printf("%d ", current_node->value);
        current_node = current_node->next;
    }
    printf("\n");
}

struct thread_arguments
{
    int start;
    int end;
};

void *threads_link_list(void *args)
{
    struct thread_arguments *arg_struct = (struct thread_arguments *)args;
    int start = arg_struct->start;
    int end = arg_struct->end;
    int op, value;
    for (int i = start; i < end; i++)
    {
        op = operations[i][0];
        value = operations[i][1];
        if (op == 0)
        {
            pthread_rwlock_rdlock(&link_list_lock);
            isMember(value);
            pthread_rwlock_unlock(&link_list_lock);
        }
        else if (op == 1)
        {
            pthread_rwlock_wrlock(&link_list_lock);
            insert_node(value);
            pthread_rwlock_unlock(&link_list_lock);
        }
        else
        {
            pthread_rwlock_wrlock(&link_list_lock);
            delete_node(value);
            pthread_rwlock_unlock(&link_list_lock);
        }
    }
    return NULL;
}

int main(int argc, char *argv[])
{
    pthread_rwlock_init(&link_list_lock , NULL);
    int MAX_THREADS = atoi(argv[1]);

    scanf("%d",&n);
    int mem_prop = 0, insert_prop = 0, del_prop = 0;
    for (int i = 0; i < n; i++)
    {
        scanf("%d %d", &operations[i][0] , &operations[i][1]);
        if (operations[i][0] == 0)
            mem_prop += 1;
        else if (operations[i][0] == 1)
            insert_prop += 1;
        else
            del_prop += 1;
    }
    printf("Proportion of follow operations : %lf\n", mem_prop / (n * 1.0));
    printf("Proportion of insert operations : %lf\n", insert_prop / (n * 1.0));
    printf("Proportion of delete operations : %lf\n", del_prop / (n * 1.0));

    FILE *plot_file;
    plot_file = fopen("plt.txt", "a");
    fprintf(plot_file, "RW lock\n");
    for (int num_threads = 1 ; num_threads <= MAX_THREADS ; num_threads++)
    {
        printf("========================= Thread = %d ===========================\n", num_threads);
        struct timeval start, end;
        double time_elasped;
        // cout << mem_prop << " " << insert_prop << " " << del_prop << endl;
        gettimeofday(&start, NULL);

        pthread_t tids[num_threads];
        struct thread_arguments args[num_threads];
        for (int i = 0; i < num_threads; i++)
        {
            args[i].start = n / num_threads * i;
            args[i].end = n / num_threads * (i + 1);
            pthread_create(&tids[i], NULL, threads_link_list, (void *)args);
        }

        for (int i = 0; i < num_threads; i++)
            pthread_join(tids[i], NULL);

        // printList();
        gettimeofday(&end, NULL);
        time_elasped = (end.tv_sec - start.tv_sec) * 1000.0;
        time_elasped += (end.tv_usec - start.tv_usec) / 1000.0;
        printf("Time taken link list rwlock : %lf\n", time_elasped);
        fprintf(plot_file , "%lf " , time_elasped );
    }
    fprintf(plot_file , "\n");
    pthread_rwlock_destroy(&link_list_lock);
    return 0;
}