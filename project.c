#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <time.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define SHM_KEY 1234
#define MAX_ACCOUNTS 10
#define MSG_QUEUE_KEY 5678
#define TIME_UNIT 1  // Time unit for Gantt chart simulation (seconds)
#define TIME_QUANTUM 2  // Time quantum for Round Robin (seconds)
#define MAX_PROCESSES 100

typedef struct Account {
    int account_id;
    int customer_id;
    double balance;
} Account;

// Shared memory pointer
Account *accounts;

pthread_mutex_t lock;

// Structure to hold transaction data for threads
struct TransactionData {
    int account_id;
    double amount;
    int is_deposit;
    int msgid;
};

// Structure to store scheduling metrics for each transaction
struct TransactionMetrics {
    int transaction_id;
    int arrival_time;
    int completion_time;
    int execution_time;
    int remaining_time;
    int turnaround_time;
    int waiting_time;
};

// Message queue structure for IPC
struct msg_buffer {
    long msg_type;        
    char msg_text[100];   
} message;

struct ProcessQueue {
    struct TransactionMetrics *transactions[MAX_ACCOUNTS];
    int front, rear;
};
  

enum ProcessStatus { PENDING, RUNNING, COMPLETED , FAILED};

// Structure to store transaction processes
struct Process {
    int process_id;                // Unique ID for the process
    int account_id;                // Account involved in the transaction
    double amount;                 // Transaction amount
    int is_deposit;                // 1 if deposit, 0 if withdrawal
    enum ProcessStatus status;     // Current status of the process
    int execution_time;            // Time needed for execution
};

// Process Table to manage all processes
struct ProcessTable {
    struct Process processes[MAX_PROCESSES];
    int count; 
};

struct ProcessTable process_table = {0};  
void add_process(int account_id, double amount, int is_deposit, int execution_time);
void update_process_status(int process_id, enum ProcessStatus status);

// Shared memory initialization
void initialize_shared_memory() {
    accounts = (Account *)malloc(MAX_ACCOUNTS * sizeof(Account)); // Using dynamic memory for shared memory simulation
    if (accounts == NULL) {
        perror("Shared memory initialization failed");
        exit(1);
    }
    memset(accounts, 0, MAX_ACCOUNTS * sizeof(Account)); // Initialize shared memory
    printf("Shared memory initialized.\n");
}

// IPC: Send and receive message 
void notify_transaction(int msgid, const char *text) {
    message.msg_type = 1; // Message type 1 for transaction notifications
    strcpy(message.msg_text, text);
    
    if (msgsnd(msgid, &message, sizeof(message.msg_text), 0) == -1) {
        perror("Failed to send message");
        exit(1);
    }
    if (msgrcv(msgid, &message, sizeof(message.msg_text), 1, 0) == -1) {
        perror("Failed to receive message");
        exit(1);
    }
    printf("IPC Notification: %s\n", message.msg_text);
}

// Function to create an account
int create_account(int customer_id, double initial_balance, int index) {
    pthread_mutex_lock(&lock);
    if (index < MAX_ACCOUNTS) {
        accounts[index].account_id = index + 1;
        accounts[index].customer_id = customer_id;
        accounts[index].balance = initial_balance;

        printf("Account Created: ID=%d, CustomerID=%d, Balance=%.2f\n",
               accounts[index].account_id, customer_id, initial_balance);
        pthread_mutex_unlock(&lock);
        return accounts[index].account_id;
    }
    printf("Account creation failed: Max accounts reached.\n");
    pthread_mutex_unlock(&lock);
    return -1;
}

// Deposit money into an account
void deposit(int account_id, double amount, int msgid) {
    pthread_mutex_lock(&lock);

    if (account_id > 0 && account_id <= MAX_ACCOUNTS && accounts[account_id - 1].account_id != 0) {
        accounts[account_id - 1].balance += amount;

        printf("Deposit: Account ID=%d, Amount=%.2f, New Balance=%.2f\n",
               account_id, amount, accounts[account_id - 1].balance);

        // Notify success via IPC
        char msg[100];
        sprintf(msg, "Deposit of %.2f to Account ID=%d completed.", amount, account_id);
        notify_transaction(msgid, msg);
    } else {
        printf("Deposit Failed: Invalid or Non-Existent Account ID=%d\n", account_id);

        // Notify failure via IPC
        char msg[100];
        sprintf(msg, "Deposit failed. Invalid Account ID=%d", account_id);
        notify_transaction(msgid, msg);

    }

    pthread_mutex_unlock(&lock);
}







// Withdraw money from an account
void withdraw(int account_id, double amount, int msgid) {
    pthread_mutex_lock(&lock);

    if (account_id > 0 && account_id <= MAX_ACCOUNTS && accounts[account_id - 1].account_id != 0) {
        if (accounts[account_id - 1].balance >= amount) {
            accounts[account_id - 1].balance -= amount;

            printf("Withdraw: Account ID=%d, Amount=%.2f, New Balance=%.2f\n",
                   account_id, amount, accounts[account_id - 1].balance);

            // Notify success via IPC
            char msg[100];
            sprintf(msg, "Withdrawal of %.2f from Account ID=%d completed.", amount, account_id);
            notify_transaction(msgid, msg);

        } else {
            printf("Withdraw Failed: Insufficient funds in Account ID=%d\n", account_id);

            // Notify failure via IPC
            char msg[100];
            sprintf(msg, "Withdrawal failed. Insufficient funds. Account ID=%d", account_id);
            notify_transaction(msgid, msg);
        }
    } else {
        printf("Withdraw Failed: Invalid or Non-Existent Account ID=%d\n", account_id);

        // Notify failure via IPC
        char msg[100];
        sprintf(msg, "Withdrawal failed. Invalid Account ID=%d", account_id);
        notify_transaction(msgid, msg);     
    }

    pthread_mutex_unlock(&lock);
}

// Check balance of an account
void check_balance(int account_id) {
    pthread_mutex_lock(&lock);
    if (account_id > 0 && account_id <= MAX_ACCOUNTS) {
        printf("Balance: Account ID=%d, Balance=%.2f\n",
               account_id, accounts[account_id - 1].balance);
    } else {
        printf("Invalid Account ID=%d\n", account_id);
    }
    pthread_mutex_unlock(&lock);
}

//Gantt Chart
void display_gantt_chart(struct TransactionMetrics *metrics, int num_transactions) {
    printf("\nGantt Chart (Round Robin Scheduling):\n");
    printf("Transaction\tArrival Time\tExecution Time\tRemaining Time\tCompletion Time\tTurnaround Time\tWaiting Time\n");
    for (int i = 0; i < num_transactions; i++) {
        printf("T%d\t\t%d\t\t%d\t\t%d\t\t%d\t\t%d\t\t%d\n", 
               metrics[i].transaction_id,
               metrics[i].arrival_time,
               metrics[i].execution_time,
               metrics[i].remaining_time,
               metrics[i].completion_time,
               metrics[i].turnaround_time,
               metrics[i].waiting_time);
    }
}

// Memory Map Simulation (Allocation of pages for accounts and transactions)
void display_memory_map(int num_accounts) {
    printf("\nMemory Map (Pages Allocation):\n");
    printf("Address\t\tAllocation Type\n");
    for (int i = 0; i < num_accounts; i++) {
        printf("0x%p\t\tAccount %d\n", &accounts[i], accounts[i].account_id);
    }
    printf("0x%p\t\tTransaction Data (Shared)\n", accounts);
}

void init_queue(struct ProcessQueue *q) {
    q->front = 0;
    q->rear = 0;
}

int is_queue_empty(struct ProcessQueue *q) {
    return q->front == q->rear;
}

void enqueue(struct ProcessQueue *q, struct TransactionMetrics *transaction) {
    q->transactions[q->rear] = transaction;
    q->rear = (q->rear + 1) % MAX_ACCOUNTS;
}

struct TransactionMetrics* dequeue(struct ProcessQueue *q) {
    struct TransactionMetrics *transaction = q->transactions[q->front];
    q->front = (q->front + 1) % MAX_ACCOUNTS;
    return transaction;
}

// Round Robin Scheduling Function
void round_robin_scheduling(struct ProcessQueue *q, struct TransactionMetrics *metrics, int num_transactions, int *current_time) {
    while (!is_queue_empty(q)) {
        struct TransactionMetrics *transaction = dequeue(q);
        
        if (transaction->remaining_time > TIME_QUANTUM) {
            *current_time += TIME_QUANTUM;
            transaction->remaining_time -= TIME_QUANTUM;
            enqueue(q, transaction);  
        } else {
            *current_time += transaction->remaining_time;
            transaction->completion_time = *current_time;
            transaction->remaining_time = 0;
        }
        transaction->turnaround_time = transaction->completion_time - transaction->arrival_time;
        transaction->waiting_time = transaction->turnaround_time - transaction->execution_time;
    }
}

// Thread function for deposit or withdraw
void *transaction_thread(void *arg) {
    struct TransactionData *data = (struct TransactionData *)arg;

    if (data->is_deposit) {
        deposit(data->account_id, data->amount,data->msgid);
    } else {
        withdraw(data->account_id, data->amount,data->msgid);
    }
    free(data); 
    return NULL;
}

// Function to create threads for transactions
void perform_transaction(int account_id, double amount, int is_deposit, 
                         struct TransactionMetrics *metrics, int transaction_id, 
                         int *current_time, int msgid) {
    pthread_t tid;
    struct TransactionData *data = malloc(sizeof(struct TransactionData));
    data->account_id = account_id;
    data->amount = amount;
    data->is_deposit = is_deposit;
    data->msgid = msgid;

    int execution_time = rand() % 5 + 1;  
    add_process(account_id, amount, is_deposit, execution_time);

    metrics[transaction_id].arrival_time = *current_time;
    metrics[transaction_id].execution_time = execution_time;
    metrics[transaction_id].remaining_time = execution_time;

    if (pthread_create(&tid, NULL, transaction_thread, (void *)data) != 0) {
        perror("Failed to create thread");
        free(data);
    }

    pthread_join(tid, NULL); 
    *current_time += metrics[transaction_id].execution_time; 
    update_process_status(transaction_id , COMPLETED);
}


// Function to add a process to the process table
void add_process(int account_id, double amount, int is_deposit, int execution_time) {
    if (process_table.count < MAX_PROCESSES) {
        struct Process *p = &process_table.processes[process_table.count];
        p->process_id = process_table.count + 1;
        p->account_id = account_id;
        p->amount = amount;
        p->is_deposit = is_deposit;
        p->status = PENDING;
        p->execution_time = execution_time;

        printf("Process Created: ID=%d, Account ID=%d, Amount=%.2f, Status=PENDING\n",
               p->process_id, account_id, amount);

        process_table.count++;
    } else {
        printf("Process table full. Cannot add more processes.\n");
    }
}

// Function to update the status of a process
void update_process_status(int process_id, enum ProcessStatus status) {
    for (int i = 0; i < process_table.count; i++) {
        if (process_table.processes[i].process_id == process_id) {
            process_table.processes[i].status = status;
            printf("Process ID=%d updated to status=%s\n",
                   process_id, status == RUNNING ? "RUNNING" : "COMPLETED");
            return;
        }
    }
}


// Function to display all processes in the process table
void display_process_table() {
    printf("\nProcess Table:\n");
    printf("ID\tAccountID\tAmount\tStatus\t\tExecution Time\n");
    for (int i = 0; i < process_table.count; i++) {
        printf("%d\t%d\t\t%.2f\t%s\t\t%d\n",
               process_table.processes[i].process_id,
               process_table.processes[i].account_id,
               process_table.processes[i].amount,
               process_table.processes[i].status == PENDING ? "PENDING" :
               process_table.processes[i].status == RUNNING ? "RUNNING" : "COMPLETED",
               process_table.processes[i].execution_time);
    }
}


// Main function
int main() {
    int choice, customer_id, account_id, index = 0;
    double amount;

    key_t key = ftok("progfile", 65);          
    int msgid = msgget(key, 0666 | IPC_CREAT); 

    int current_time = 0;  
    struct TransactionMetrics metrics[MAX_ACCOUNTS] = {0};  
    struct ProcessQueue q;
    init_queue(&q);  

    srand(time(NULL));  
    pthread_mutex_init(&lock, NULL);
    initialize_shared_memory();

    printf("Welcome to the Banking System!\n");

    while (1) {
    printf("\n1. Create Account\n2. Deposit Money\n3. Withdraw Money\n4. Check Balance\n5. Show Gantt Chart\n6. Show Memory Map\n7. Show Process Table\n8. Exit\n");
    printf("Enter your choice: ");
    scanf("%d", &choice);

    switch (choice) {
        case 1:
            if (index < MAX_ACCOUNTS) {
                printf("Enter Customer ID: ");
                scanf("%d", &customer_id);
                printf("Enter Initial Balance: ");
                scanf("%lf", &amount);
                create_account(customer_id, amount, index);
                index++;
            } else {
                printf("Account limit reached. Cannot create more accounts.\n");
            }
            break;

        case 2:
            printf("Enter Account ID: ");
            scanf("%d", &account_id);
            printf("Enter Amount to Deposit: ");
            scanf("%lf", &amount);
            perform_transaction(account_id, amount, 1, metrics, index, &current_time, msgid);
            break;

        case 3:
            printf("Enter Account ID: ");
            scanf("%d", &account_id);
            printf("Enter Amount to Withdraw: ");
            scanf("%lf", &amount);
            perform_transaction(account_id, amount, 0, metrics, index, &current_time, msgid);
            break;

        case 4:
            printf("Enter Account ID: ");
            scanf("%d", &account_id);
            check_balance(account_id);
            break;

        case 5:
           
            for (int i = 0; i < index; i++) {
                enqueue(&q, &metrics[i]);
            }
            round_robin_scheduling(&q, metrics, index, &current_time);
            display_gantt_chart(metrics, index);
            break;

        case 6:
            display_memory_map(index);
            break;

        case 7:
            display_process_table();
            break;

        case 8:
            pthread_mutex_destroy(&lock);
            free(accounts);
            msgctl(msgid, IPC_RMID, NULL);
            exit(0);
            break;

        default:
            printf("Invalid choice. Try again.\n");
    }
}

    return 0;
}