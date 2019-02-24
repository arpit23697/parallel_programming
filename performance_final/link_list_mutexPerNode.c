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
    pthread_mutex_t mutex;
};

int MAX_THREADS;
//note that this head mutex is just to lock the head of the link list
//there is an additional mutex for the starting node of the link list
pthread_mutex_t head_mutex;
struct node *head = NULL;

int n;
int operations[10000000][2];

int isMember(int value)
{
    struct node * current_node , *old_node;
    pthread_mutex_lock (&head_mutex);
    current_node = head;
    if (current_node != NULL)
        pthread_mutex_lock(&(current_node->mutex));
    pthread_mutex_unlock(&head_mutex);
        

    while (current_node != NULL && current_node->value < value)
    {
        if (current_node->next != NULL)
            pthread_mutex_lock(&(current_node->next->mutex));

        old_node = current_node;
        current_node = current_node->next;
        pthread_mutex_unlock(&(old_node->mutex));
    }
    //return false if the end of the list or the value is not found
    if (current_node == NULL || current_node->value != value)
    {
        if (current_node != NULL)
            pthread_mutex_unlock(&(current_node->mutex));
        return 0;
    }
    else
    {
        pthread_mutex_unlock(&(current_node->mutex));
        return 1;
    }
}
//points to note
//when the new node is created then initialise the mutex to unlocked state
int insert_node(int value)
{
    //initialise the current node to head_mutex
    struct node *current_node;
    pthread_mutex_lock(&head_mutex);
    current_node = head;
    if (current_node != NULL)
        pthread_mutex_lock(&(current_node->mutex));

    struct node *pred_node = NULL;
    struct node *new_node;


    while (current_node != NULL && current_node->value < value)
    {
        if (current_node->next != NULL)
            pthread_mutex_lock(&(current_node->next->mutex));
        if (pred_node != NULL)
            pthread_mutex_unlock(&(pred_node->mutex));
        else
            pthread_mutex_unlock(&head_mutex);
        
        pred_node = current_node;
        current_node = current_node->next;
    }
    //insert the value if the value is not in the list
    if (current_node == NULL || current_node->value != value)
    {
        //insert the node
        new_node = (struct node *)malloc(sizeof(struct node ));
    
        new_node->value = value;
        new_node->next = current_node;
        pthread_mutex_init(&(new_node->mutex) , NULL); 
        if (pred_node == NULL)
        {
            head = new_node;
            pthread_mutex_unlock(&head_mutex);
            if(current_node != NULL)
            pthread_mutex_unlock(&(current_node->mutex));
        }
        else
        {
            pred_node->next = new_node;
            pthread_mutex_unlock(&(pred_node->mutex));
            if (current_node != NULL)
                pthread_mutex_unlock(&(current_node->mutex));
            return 1;
        }
    }
    else
    {
        if (current_node != NULL)
            pthread_mutex_unlock(&(current_node->mutex));
        if (pred_node != NULL)
            pthread_mutex_unlock(&(pred_node->mutex));
        else
            pthread_mutex_unlock(&(head_mutex));
        
        return 0;
    }
}

int delete_node(int value)
{
    struct node *current_node;
    pthread_mutex_lock(&head_mutex);
    current_node = head;
    if (current_node != NULL)
        pthread_mutex_lock(&(current_node->mutex));

    struct node *pred_node = NULL;
    struct node *new_node;

    

    while (current_node != NULL && current_node->value < value)
    {
        if (current_node->next != NULL)
            pthread_mutex_lock(&(current_node->next->mutex));
        if (pred_node != NULL)
            pthread_mutex_unlock(&(pred_node->mutex));
        else
            pthread_mutex_unlock(&head_mutex);

        pred_node = current_node;
        current_node = current_node->next;
    }

    //if the value is found
    if (current_node != NULL && current_node->value == value)
    {
        if (pred_node == NULL)
        {
            head = current_node->next;
            pthread_mutex_unlock(&head_mutex);
            pthread_mutex_unlock(&(current_node->mutex));
            pthread_mutex_destroy(&(current_node->mutex));
            free(current_node);
        }
        else
        {
            pred_node->next = current_node->next;
            pthread_mutex_unlock(&(pred_node->mutex));
            pthread_mutex_unlock(&(current_node->mutex));
            pthread_mutex_destroy(&(current_node->mutex));
            free(current_node);
        }
        return 1;
    }
    else
    {
        if (current_node != NULL)
            pthread_mutex_unlock(&(current_node->mutex));
        if (pred_node != NULL)
            pthread_mutex_unlock(&(pred_node->mutex));
        else
            pthread_mutex_unlock(&(head_mutex));
        
        return 0;
    }
}

struct thread_arguments
{
    int start;
    int end;
};


void* threads_link_list(void * args)
{
    struct thread_arguments *arg_struct = (struct thread_arguments *)args;
    int start = arg_struct->start;
    int end = arg_struct->end;
    int op, value;
    for (int i = start ; i < end; i++)
    {
        op = operations[i][0];
        value = operations[i][1];
        if (op == 0)
            isMember(value);
        else if (op == 1)
            insert_node(value);
        else 
            delete_node(value);
    }
    return NULL;
}

//Helper function for printing the list
void printList()
{
    struct node *current_node = head;
    while (current_node != NULL)
    {
        printf("%d ", current_node->value);
        current_node = current_node->next;
    }
    printf("\n");
}

int main (int argc , char *argv[])
{
    int MAX_THREADS = atoi(argv[1]);

    int n;
    scanf("%d",&n);
    int mem_prop = 0, insert_prop = 0 , del_prop = 0;
    for (int i =0 ; i < n ; i++)
    {
        scanf("%d %d", &operations[i][0] , &operations[i][1]);
        if (operations[i][0] == 0)
            mem_prop+=1;
        else if (operations[i][0] == 1)
            insert_prop+=1;
        else
            del_prop+=1;
        
    }
    printf("Proportion of follow operations : %lf\n", mem_prop / (n * 1.0));
    printf("Proportion of insert operations : %lf\n", insert_prop / (n * 1.0));
    printf("Proportion of delete operations : %lf\n", del_prop / (n * 1.0));

    FILE *plot_file;
    plot_file = fopen("plt.txt", "a");
    fprintf(plot_file, "One mutex per node\n");
    for (int num_threads = 1; num_threads <= MAX_THREADS ; num_threads++)
    {
        printf("========================= Thread = %d ===========================\n" , num_threads );
        struct timeval start , end;
        double time_elasped;
        // cout << mem_prop << " " << insert_prop << " " << del_prop << endl;
        gettimeofday(&start , NULL);

        pthread_t tids[num_threads];
        struct thread_arguments args[num_threads];
        for (int i =0 ; i < num_threads ; i++)
        {
            args[i].start = n/num_threads * i;
            args[i].end = n/num_threads * (i+1);
            pthread_create (&tids[i] , NULL , threads_link_list , (void *)args);
        }

        for (int i =0 ; i < num_threads ; i++)
            pthread_join(tids[i] , NULL);

        // printList();
        gettimeofday(&end , NULL);
        time_elasped = (end.tv_sec - start.tv_sec) * 1000.0;
        time_elasped += (end.tv_usec - start.tv_usec) / 1000.0;
        printf("Time taken mutex per node : %lf\n", time_elasped);
        fprintf(plot_file, "%lf ", time_elasped);
    }
    fprintf(plot_file, "\n");
    return 0;
}