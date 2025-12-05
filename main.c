#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define CLIENT_FILE "clients.dat"
#define BILL_FILE "billing.dat"
#define CLIENT_BACKUP "clients.bak"
#define BILL_BACKUP "billing.bak"

#define NAME_LEN 50
#define ADDRESS_LEN 100
#define PHONE_LEN 20
#define DATE_LEN 16

typedef struct {
    int id;
    char name[NAME_LEN];
    char address[ADDRESS_LEN];
    char phone[PHONE_LEN];
    double consumption;
    double rate;
    double last_bill;
} Client;

typedef struct {
    int id;
    int client_id;
    double consumption;
    double rate;
    double amount;
    char due_date[DATE_LEN];
    int paid;
} Bill;

static void clear_input(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {
    }
}

static int safe_read_line(char *buffer, size_t size) {
    if (fgets(buffer, (int)size, stdin) == NULL) {
        return 0;
    }
    size_t len = strlen(buffer);
    if (len > 0 && buffer[len - 1] == '\n') {
        buffer[len - 1] = '\0';
    } else {
        clear_input();
    }
    return 1;
}

static int load_clients(Client **clients, size_t *count) {
    FILE *file = fopen(CLIENT_FILE, "rb");
    if (!file) {
        *clients = NULL;
        *count = 0;
        return 1;
    }
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);
    *count = (size_t)(size / (long)sizeof(Client));
    *clients = malloc(*count * sizeof(Client));
    if (!*clients) {
        fclose(file);
        return 0;
    }
    if (fread(*clients, sizeof(Client), *count, file) != *count) {
        free(*clients);
        *clients = NULL;
        *count = 0;
        fclose(file);
        return 0;
    }
    fclose(file);
    return 1;
}

static int save_clients(const Client *clients, size_t count) {
    FILE *file = fopen(CLIENT_FILE, "wb");
    if (!file) {
        perror("Failed to open client file");
        return 0;
    }
    if (fwrite(clients, sizeof(Client), count, file) != count) {
        perror("Failed to write clients");
        fclose(file);
        return 0;
    }
    fclose(file);
    return 1;
}

static int load_bills(Bill **bills, size_t *count) {
    FILE *file = fopen(BILL_FILE, "rb");
    if (!file) {
        *bills = NULL;
        *count = 0;
        return 1;
    }
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);
    *count = (size_t)(size / (long)sizeof(Bill));
    *bills = malloc(*count * sizeof(Bill));
    if (!*bills) {
        fclose(file);
        return 0;
    }
    if (fread(*bills, sizeof(Bill), *count, file) != *count) {
        free(*bills);
        *bills = NULL;
        *count = 0;
        fclose(file);
        return 0;
    }
    fclose(file);
    return 1;
}

static int save_bills(const Bill *bills, size_t count) {
    FILE *file = fopen(BILL_FILE, "wb");
    if (!file) {
        perror("Failed to open bill file");
        return 0;
    }
    if (fwrite(bills, sizeof(Bill), count, file) != count) {
        perror("Failed to write bills");
        fclose(file);
        return 0;
    }
    fclose(file);
    return 1;
}

static int copy_file(const char *source, const char *destination) {
    FILE *src = fopen(source, "rb");
    if (!src) {
        return 0;
    }
    FILE *dest = fopen(destination, "wb");
    if (!dest) {
        fclose(src);
        return 0;
    }
    char buffer[4096];
    size_t read_bytes;
    while ((read_bytes = fread(buffer, 1, sizeof(buffer), src)) > 0) {
        if (fwrite(buffer, 1, read_bytes, dest) != read_bytes) {
            fclose(src);
            fclose(dest);
            return 0;
        }
    }
    fclose(src);
    fclose(dest);
    return 1;
}

static int next_client_id(const Client *clients, size_t count) {
    int max_id = 0;
    for (size_t i = 0; i < count; ++i) {
        if (clients[i].id > max_id) {
            max_id = clients[i].id;
        }
    }
    return max_id + 1;
}

static int next_bill_id(const Bill *bills, size_t count) {
    int max_id = 0;
    for (size_t i = 0; i < count; ++i) {
        if (bills[i].id > max_id) {
            max_id = bills[i].id;
        }
    }
    return max_id + 1;
}

static void add_client(void) {
    Client *clients = NULL;
    size_t count = 0;
    if (!load_clients(&clients, &count)) {
        printf("Failed to load clients.\n");
        return;
    }

    Client new_client = {0};
    new_client.id = next_client_id(clients, count);

    printf("Enter client name: ");
    if (!safe_read_line(new_client.name, sizeof(new_client.name)) || strlen(new_client.name) == 0) {
        printf("Invalid name.\n");
        free(clients);
        return;
    }

    printf("Enter address: ");
    if (!safe_read_line(new_client.address, sizeof(new_client.address)) || strlen(new_client.address) == 0) {
        printf("Invalid address.\n");
        free(clients);
        return;
    }

    printf("Enter phone: ");
    if (!safe_read_line(new_client.phone, sizeof(new_client.phone)) || strlen(new_client.phone) == 0) {
        printf("Invalid phone.\n");
        free(clients);
        return;
    }

    printf("Enter consumption (kWh): ");
    if (scanf("%lf", &new_client.consumption) != 1 || new_client.consumption < 0) {
        printf("Invalid consumption.\n");
        clear_input();
        free(clients);
        return;
    }

    printf("Enter rate per kWh: ");
    if (scanf("%lf", &new_client.rate) != 1 || new_client.rate < 0) {
        printf("Invalid rate.\n");
        clear_input();
        free(clients);
        return;
    }

    printf("Enter last bill amount: ");
    if (scanf("%lf", &new_client.last_bill) != 1 || new_client.last_bill < 0) {
        printf("Invalid last bill.\n");
        clear_input();
        free(clients);
        return;
    }
    clear_input();

    Client *updated = realloc(clients, (count + 1) * sizeof(Client));
    if (!updated) {
        printf("Memory allocation failed.\n");
        free(clients);
        return;
    }
    updated[count] = new_client;
    if (!save_clients(updated, count + 1)) {
        printf("Failed to save client.\n");
    } else {
        printf("Client added with ID %d.\n", new_client.id);
    }
    free(updated);
}

static void display_clients(void) {
    Client *clients = NULL;
    size_t count = 0;
    if (!load_clients(&clients, &count)) {
        printf("Failed to load clients.\n");
        return;
    }

    if (count == 0) {
        printf("No clients found.\n");
        return;
    }

    printf("\n%-5s %-20s %-25s %-12s %-10s %-12s\n", "ID", "Name", "Address", "Consumption", "Rate", "Last Bill");
    printf("-------------------------------------------------------------------------------\n");
    for (size_t i = 0; i < count; ++i) {
        printf("%-5d %-20s %-25s %-12.2f %-10.2f %-12.2f\n",
               clients[i].id, clients[i].name, clients[i].address,
               clients[i].consumption, clients[i].rate, clients[i].last_bill);
    }
    free(clients);
}

static void update_client(void) {
    Client *clients = NULL;
    size_t count = 0;
    if (!load_clients(&clients, &count) || count == 0) {
        printf("No clients to update.\n");
        return;
    }

    int id;
    printf("Enter client ID to update: ");
    if (scanf("%d", &id) != 1) {
        printf("Invalid ID.\n");
        clear_input();
        free(clients);
        return;
    }
    clear_input();

    for (size_t i = 0; i < count; ++i) {
        if (clients[i].id == id) {
            printf("Current consumption: %.2f. Enter new consumption: ", clients[i].consumption);
            if (scanf("%lf", &clients[i].consumption) != 1 || clients[i].consumption < 0) {
                printf("Invalid consumption.\n");
                clear_input();
                free(clients);
                return;
            }
            printf("Current rate: %.2f. Enter new rate: ", clients[i].rate);
            if (scanf("%lf", &clients[i].rate) != 1 || clients[i].rate < 0) {
                printf("Invalid rate.\n");
                clear_input();
                free(clients);
                return;
            }
            clear_input();
            if (!save_clients(clients, count)) {
                printf("Failed to save updates.\n");
            } else {
                printf("Client updated.\n");
            }
            free(clients);
            return;
        }
    }

    printf("Client ID not found.\n");
    free(clients);
}

static void delete_client(void) {
    Client *clients = NULL;
    size_t count = 0;
    if (!load_clients(&clients, &count) || count == 0) {
        printf("No clients to delete.\n");
        return;
    }

    int id;
    printf("Enter client ID to delete: ");
    if (scanf("%d", &id) != 1) {
        printf("Invalid ID.\n");
        clear_input();
        free(clients);
        return;
    }
    clear_input();

    size_t index = count;
    for (size_t i = 0; i < count; ++i) {
        if (clients[i].id == id) {
            index = i;
            break;
        }
    }

    if (index == count) {
        printf("Client not found.\n");
        free(clients);
        return;
    }

    for (size_t i = index; i + 1 < count; ++i) {
        clients[i] = clients[i + 1];
    }
    if (!save_clients(clients, count - 1)) {
        printf("Failed to delete client.\n");
    } else {
        printf("Client deleted.\n");
    }
    free(clients);
}

static void search_client(void) {
    Client *clients = NULL;
    size_t count = 0;
    if (!load_clients(&clients, &count) || count == 0) {
        printf("No clients available.\n");
        return;
    }

    int choice;
    printf("Search by: 1) ID 2) Name: ");
    if (scanf("%d", &choice) != 1) {
        printf("Invalid choice.\n");
        clear_input();
        free(clients);
        return;
    }
    clear_input();

    if (choice == 1) {
        int id;
        printf("Enter ID: ");
        if (scanf("%d", &id) != 1) {
            printf("Invalid ID.\n");
            clear_input();
            free(clients);
            return;
        }
        clear_input();
        for (size_t i = 0; i < count; ++i) {
            if (clients[i].id == id) {
                printf("Found: %s, consumption %.2f, rate %.2f, last bill %.2f\n",
                       clients[i].name, clients[i].consumption, clients[i].rate, clients[i].last_bill);
                free(clients);
                return;
            }
        }
        printf("Client not found.\n");
    } else if (choice == 2) {
        char name[NAME_LEN];
        printf("Enter name: ");
        if (!safe_read_line(name, sizeof(name))) {
            printf("Invalid name.\n");
            free(clients);
            return;
        }
        for (size_t i = 0; i < count; ++i) {
            if (strcmp(clients[i].name, name) == 0) {
                printf("Found ID %d at %s with last bill %.2f\n",
                       clients[i].id, clients[i].address, clients[i].last_bill);
                free(clients);
                return;
            }
        }
        printf("Client not found.\n");
    } else {
        printf("Invalid option.\n");
    }
    free(clients);
}

static int compare_by_consumption(const void *a, const void *b) {
    const Client *ca = (const Client *)a;
    const Client *cb = (const Client *)b;
    if (ca->consumption < cb->consumption) return -1;
    if (ca->consumption > cb->consumption) return 1;
    return ca->id - cb->id;
}

static int compare_by_id(const void *a, const void *b) {
    const Client *ca = (const Client *)a;
    const Client *cb = (const Client *)b;
    return ca->id - cb->id;
}

static void sort_clients(void) {
    Client *clients = NULL;
    size_t count = 0;
    if (!load_clients(&clients, &count) || count == 0) {
        printf("No clients to sort.\n");
        return;
    }

    int choice;
    printf("Sort by: 1) Consumption 2) ID: ");
    if (scanf("%d", &choice) != 1) {
        printf("Invalid option.\n");
        clear_input();
        free(clients);
        return;
    }
    clear_input();

    if (choice == 1) {
        qsort(clients, count, sizeof(Client), compare_by_consumption);
    } else if (choice == 2) {
        qsort(clients, count, sizeof(Client), compare_by_id);
    } else {
        printf("Invalid option.\n");
        free(clients);
        return;
    }

    if (!save_clients(clients, count)) {
        printf("Failed to save sorted clients.\n");
    } else {
        printf("Clients sorted and saved.\n");
    }
    free(clients);
}

static Client *find_client_by_id(int id, size_t *count_out) {
    Client *clients = NULL;
    size_t count = 0;
    if (!load_clients(&clients, &count) || count == 0) {
        return NULL;
    }
    for (size_t i = 0; i < count; ++i) {
        if (clients[i].id == id) {
            if (count_out) {
                *count_out = count;
            }
            return clients;
        }
    }
    free(clients);
    return NULL;
}

static void generate_bill(void) {
    int client_id;
    printf("Enter client ID: ");
    if (scanf("%d", &client_id) != 1) {
        printf("Invalid ID.\n");
        clear_input();
        return;
    }
    clear_input();

    size_t client_count = 0;
    Client *clients = find_client_by_id(client_id, &client_count);
    if (!clients) {
        printf("Client not found.\n");
        return;
    }

    Client *client = NULL;
    for (size_t i = 0; i < client_count; ++i) {
        if (clients[i].id == client_id) {
            client = &clients[i];
            break;
        }
    }

    double consumption;
    printf("Enter consumption (kWh) for this bill: ");
    if (scanf("%lf", &consumption) != 1 || consumption < 0) {
        printf("Invalid consumption.\n");
        clear_input();
        free(clients);
        return;
    }

    double rate;
    printf("Enter rate per kWh: ");
    if (scanf("%lf", &rate) != 1 || rate < 0) {
        printf("Invalid rate.\n");
        clear_input();
        free(clients);
        return;
    }
    clear_input();

    Bill *bills = NULL;
    size_t bill_count = 0;
    if (!load_bills(&bills, &bill_count)) {
        printf("Failed to load bills.\n");
        free(clients);
        return;
    }

    Bill new_bill = {0};
    new_bill.id = next_bill_id(bills, bill_count);
    new_bill.client_id = client_id;
    new_bill.consumption = consumption;
    new_bill.rate = rate;
    new_bill.amount = consumption * rate;
    new_bill.paid = 0;

    printf("Enter due date (YYYY-MM-DD): ");
    if (!safe_read_line(new_bill.due_date, sizeof(new_bill.due_date)) || strlen(new_bill.due_date) < 8) {
        printf("Invalid due date.\n");
        free(clients);
        free(bills);
        return;
    }

    Bill *updated_bills = realloc(bills, (bill_count + 1) * sizeof(Bill));
    if (!updated_bills) {
        printf("Memory allocation failed.\n");
        free(clients);
        free(bills);
        return;
    }
    updated_bills[bill_count] = new_bill;

    client->consumption = consumption;
    client->rate = rate;
    client->last_bill = new_bill.amount;

    if (!save_bills(updated_bills, bill_count + 1) || !save_clients(clients, client_count)) {
        printf("Failed to save bill.\n");
    } else {
        printf("Bill generated with ID %d. Amount: %.2f\n", new_bill.id, new_bill.amount);
    }

    free(updated_bills);
    free(clients);
}

static void display_bills(void) {
    Bill *bills = NULL;
    size_t count = 0;
    if (!load_bills(&bills, &count)) {
        printf("Failed to load bills.\n");
        return;
    }

    if (count == 0) {
        printf("No bills found.\n");
        return;
    }

    printf("\n%-5s %-10s %-12s %-10s %-10s %-12s %-8s\n", "ID", "Client ID", "Consumption", "Rate", "Amount", "Due Date", "Paid");
    printf("----------------------------------------------------------------------------\n");
    for (size_t i = 0; i < count; ++i) {
        printf("%-5d %-10d %-12.2f %-10.2f %-10.2f %-12s %-8s\n",
               bills[i].id, bills[i].client_id, bills[i].consumption, bills[i].rate,
               bills[i].amount, bills[i].due_date, bills[i].paid ? "Yes" : "No");
    }
    free(bills);
}

static void update_bill_status(void) {
    Bill *bills = NULL;
    size_t count = 0;
    if (!load_bills(&bills, &count) || count == 0) {
        printf("No bills to update.\n");
        return;
    }

    int id;
    printf("Enter bill ID to mark as paid: ");
    if (scanf("%d", &id) != 1) {
        printf("Invalid bill ID.\n");
        clear_input();
        free(bills);
        return;
    }
    clear_input();

    for (size_t i = 0; i < count; ++i) {
        if (bills[i].id == id) {
            bills[i].paid = 1;
            if (!save_bills(bills, count)) {
                printf("Failed to update bill.\n");
            } else {
                printf("Bill marked as paid.\n");
            }
            free(bills);
            return;
        }
    }

    printf("Bill not found.\n");
    free(bills);
}

static void backup_files(void) {
    int ok_clients = copy_file(CLIENT_FILE, CLIENT_BACKUP);
    int ok_bills = copy_file(BILL_FILE, BILL_BACKUP);
    if (ok_clients || ok_bills) {
        printf("Backup completed.\n");
    } else {
        printf("Nothing to back up or backup failed.\n");
    }
}

static void restore_files(void) {
    int ok_clients = copy_file(CLIENT_BACKUP, CLIENT_FILE);
    int ok_bills = copy_file(BILL_BACKUP, BILL_FILE);
    if (ok_clients || ok_bills) {
        printf("Restore completed.\n");
    } else {
        printf("Nothing to restore or restore failed.\n");
    }
}

static void report_totals(void) {
    Client *clients = NULL;
    size_t client_count = 0;
    Bill *bills = NULL;
    size_t bill_count = 0;

    if (!load_clients(&clients, &client_count)) {
        printf("Failed to load clients.\n");
        return;
    }
    if (!load_bills(&bills, &bill_count)) {
        printf("Failed to load bills.\n");
        free(clients);
        return;
    }

    double total_consumption = 0.0;
    double total_amount = 0.0;
    for (size_t i = 0; i < client_count; ++i) {
        total_consumption += clients[i].consumption;
        total_amount += clients[i].last_bill;
    }

    double billed_amount = 0.0;
    for (size_t i = 0; i < bill_count; ++i) {
        billed_amount += bills[i].amount;
    }

    printf("Total clients: %zu\n", client_count);
    printf("Total consumption (last recorded): %.2f kWh\n", total_consumption);
    printf("Total of last bills: %.2f\n", total_amount);
    printf("Total billed amount (all bills): %.2f\n", billed_amount);

    free(clients);
    free(bills);
}

static void client_menu(void) {
    int choice;
    do {
        printf("\nClient Menu:\n");
        printf("1. Add Client\n");
        printf("2. Update Client\n");
        printf("3. Delete Client\n");
        printf("4. Search Client\n");
        printf("5. Display All Clients\n");
        printf("6. Sort Clients\n");
        printf("0. Back\n");
        printf("Enter choice: ");
        if (scanf("%d", &choice) != 1) {
            printf("Invalid input.\n");
            clear_input();
            continue;
        }
        clear_input();

        switch (choice) {
            case 1: add_client(); break;
            case 2: update_client(); break;
            case 3: delete_client(); break;
            case 4: search_client(); break;
            case 5: display_clients(); break;
            case 6: sort_clients(); break;
            case 0: break;
            default: printf("Invalid option.\n");
        }
    } while (choice != 0);
}

static void billing_menu(void) {
    int choice;
    do {
        printf("\nBilling Menu:\n");
        printf("1. Generate Bill\n");
        printf("2. Update Bill Status\n");
        printf("3. Display All Bills\n");
        printf("0. Back\n");
        printf("Enter choice: ");
        if (scanf("%d", &choice) != 1) {
            printf("Invalid input.\n");
            clear_input();
            continue;
        }
        clear_input();

        switch (choice) {
            case 1: generate_bill(); break;
            case 2: update_bill_status(); break;
            case 3: display_bills(); break;
            case 0: break;
            default: printf("Invalid option.\n");
        }
    } while (choice != 0);
}

static void main_menu(void) {
    int choice;
    do {
        printf("\nElectricity Billing System\n");
        printf("1. Client Management\n");
        printf("2. Billing System\n");
        printf("3. Backup Data\n");
        printf("4. Restore Data\n");
        printf("5. Reports\n");
        printf("0. Exit\n");
        printf("Enter choice: ");
        if (scanf("%d", &choice) != 1) {
            printf("Invalid input.\n");
            clear_input();
            continue;
        }
        clear_input();

        switch (choice) {
            case 1: client_menu(); break;
            case 2: billing_menu(); break;
            case 3: backup_files(); break;
            case 4: restore_files(); break;
            case 5: report_totals(); break;
            case 0: printf("Goodbye!\n"); break;
            default: printf("Invalid option.\n");
        }
    } while (choice != 0);
}

int main(void) {
    main_menu();
    return 0;
}

