//
// Created by mohamed on 16/12/22.
//
/*
 * A file for implementing the consumer process
 * The consumer process will read prices produced by the producer process from the shared memory and print them to the screen
 * The parameters are given in the command line
 */

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <cstdlib>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <cstring>
#include "commodity_names.h"
#include <bits/stdc++.h>

using namespace std;

// Declare a list of commodity names
const char* commodity_names[] = {"GOLD", "SILVER"};

unsigned int gold_avg = 0;
unsigned int silver_avg = 0;

unsigned int gold_prices[4] = {0};
unsigned int silver_prices[4] = {0};

unsigned int gold_current = 0;
unsigned int silver_current = 0;

int shmid_index;

void remove_shared_memory() {
    // Remove the shared memory
    shmctl(shmid_index, IPC_RMID, NULL);
}

int main(int argc, char *argv[]) {
    // Create a struct of name and price
    struct commodity {
        char name[10];
        double price;
    };

    // Create or get a shared memory if created to hold 80 commodities using ftok to generate a unique key
    int shmid = shmget(ftok("/home/mohamed/CLionProjects/producer_consumer", 1), 80 * sizeof(commodity), 0666 | IPC_CREAT);
    if (shmid < 0) {
        cerr << "Error: Failed to create the shared memory." << endl;
        return 1;
    }
    else {
        cout << "Shared memory created successfully." << endl;
    }
    // Create or get shared index if created to hold the index of the next commodity to be written
    shmid_index = shmget(ftok("/home/mohamed", 10), sizeof(int), 0666 | IPC_CREAT | IPC_EXCL);
    if (shmid_index < 0) {
        cerr << "Index memory already existed" << endl;
        shmid_index = shmget(ftok("/home/mohamed", 10), sizeof(int), 0666 | IPC_CREAT);
        if(shmid_index < 0) {
            cerr << "Error: Failed to create the shared memory." << endl;
            return 1;
        }
    }
    else {
        cout << "Shared memory created successfully." << endl;
    }


    // Attach the shared memory to the process
    commodity *commodities = (commodity *) shmat(shmid, NULL, 0);
    if (commodities == (commodity *) -1) {
        cerr << "Error: Failed to attach the shared memory." << endl;
        return 1;
    }
    else {
        cout << "Shared memory attached successfully." << endl;
    }
    // Attach the shared index to the process
    int *index = (int *) shmat(shmid_index, NULL, 0);
    if (index == (int *) -1) {
        cerr << "Error: Failed to attach the shared index." << endl;
        return 1;
    }
    else {
        cout << "Shared index attached successfully." << endl;
    }

    // Remove the shared memory when the program exits
    atexit(remove_shared_memory);

    // Create or get a semaphore set if created to control the access to the shared memory
    int semid = semget(ftok("/home/mohamed/CLionProjects/producer_consumer", 3), 1, 0666 | IPC_CREAT);
    if (semid < 0) {
        cerr << "Error: Failed to create the semaphore set." << endl;
        return 1;
    }
    else {
        cout << "Semaphore set created successfully." << endl;
    }

/*
 * The consumer process will read prices produced by the producer process from the shared memory and print them to the screen
    +-------------------------------------+
    | Currency     |  Price   | AvgPrice  |
    +-------------------------------------+
    |              |  0.00    |  0.00     |
    |              |  0.00    |  0.00     |
    +-------------------------------------+
    */
    // Print the table above without the content of the table
    /*
    cout << "+-------------------------------------+" << endl;
    cout << "| Currency     |  Price   | AvgPrice  |" << endl;
    cout << "+-------------------------------------+" << endl;
    // Make a loop with length of commidty_names
    for (int i = 0; i < sizeof(commodity_names) / sizeof(commodity_names[0]); i++) {
        cout << "|              |  0.00    |  0.00     |" << endl;
    }
    int row;
    for (int i = 1; i <= sizeof(commodity_names) / sizeof(commodity_names[0]); i++) {
        row = 7 + i;
        // Move cursor to the row of the commodity
        printf("\033[%d;3H", row);
        // Print the commodity name
        cout << commodity_names[i - 1];
    }
    // Move cursor to beginning of next row
    printf("\033[%d;1H", row + 1);
     */
    int retval;
    struct sembuf sem_op;

    // Create a loop to read the commodities from the shared memory
    while (true) {
        sem_op = {0, -1, SEM_UNDO};
        cout << "Waiting for the semaphore to be available." << endl;
        retval = semop(semid, &sem_op, 1);
        if (retval < 0) {
            cerr << "Error: Failed to lock the semaphore." << endl;
            return 1;
        }
        else {
            cout << "Semaphore locked successfully." << endl;
        }
        // Create a loop to read the commodities from the shared memory if index is bigger than zero
        cout << "Right here" << endl;
        if(*index > 0){
            cout<< *index << endl;
            cout << "Right here" << endl;
            // Read a commodity from the shared memory
            commodity commodity = commodities[(*index)--];
            cout<<"commodity name is "<<commodity.name<<endl;
            // Check type of commodity
            if (strcmp(commodity.name, "GOLD") == 0) {
                gold_prices[gold_current] = commodity.price;
                // Get average of gold_prices array
                gold_avg = accumulate(gold_prices, gold_prices + 4, 0) / 4;
                // Increment gold_current
                gold_current = (gold_current + 1) % 4;
                /*
                // Move cursor to the row of gold
                printf("\033[%d;3H", 8);
                // Print the price of gold
                cout << commodity.price;
                // Move cursor to the row of gold
                printf("\033[%d;15H", 8);
                // Print the average price of gold
                 */
                cout<< "Gold: " << gold_avg;
            }
            else if (strcmp(commodity.name, "SILVER") == 0) {
                silver_prices[gold_current] = commodity.price;
                // Get average of gold_prices array
                silver_avg = accumulate(silver_prices, silver_prices + 4, 0) / 4;
                // Increment gold_current
                silver_current = (silver_current + 1) % 4;
                /*
                // Move cursor to the row of gold
                printf("\033[%d;3H", 8);
                // Print the price of gold
                cout << commodity.price;
                // Move cursor to the row of gold
                printf("\033[%d;15H", 8);
                // Print the average price of gold
                 */
                cout<< "Silver: " << silver_avg;
            }

        }
        // Unlock the semaphore
        sem_op = {0, 1, SEM_UNDO};
        retval = semop(semid, &sem_op, 1);
        if (retval < 0) {
            cerr << "Error: Failed to unlock the semaphore." << endl;
            return 1;
        }
        else {
            cout << "Semaphore unlocked successfully." << endl;
        }

        // Sleep for 1 second
        sleep(1);
    }

}
