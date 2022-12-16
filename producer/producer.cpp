//
// Created by mohamed on 14/12/22.
//
//
/*
 * A file for implementing the producer process
 * The producer process will write prices to the shared memory according to a given distribution and a given rate
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

using namespace std;

int shmid_index;

void remove_shared_memory() {
    // Remove the shared memory
    shmctl(shmid_index, IPC_RMID, NULL);
}

int main(int argc, char *argv[]) {
    // Check the number of arguments
    if (argc != 5) {
        cerr << "Usage: " << argv[0] << " <Commodity name> <Price mean> <Price standard deviation> <Sleep interval>" << endl;
        return 1;
    }

    // Get the arguments
    string name= argv[1];
    cout << "Commodity name: " << name.c_str() << endl;
    double price_mean = atof(argv[2]);
    double price_standard_deviation = atof(argv[3]);
    int sleep_interval = atoi(argv[4]);

    // Check the arguments
    if (price_mean < 0 || price_standard_deviation < 0 || sleep_interval < 0) {
        cerr << "Error: The mean, standard deviation and sleep interval must be positive." << endl;
        return 1;
    }

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
          cerr << "Index already created" << endl;
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
    commodity *shm = (commodity *) shmat(shmid, NULL, 0);
    if (shm == (commodity *) -1) {
        cerr << "Error: Failed to attach the shared memory to the process." << endl;
        return 1;
    }
    else {
        cout << "Shared memory attached successfully." << endl;
    }
    // Attach the shared index to the process
    int *shm_index = (int *) shmat(shmid_index, NULL, 0);
    if (shm_index == (int *) -1) {
        cerr << "Error: Failed to attach the shared memory index to the process." << endl;
        return 1;
    }
    else {
        cout << "Shared memory index attached successfully." << endl;
    }
    // Remove the shared memory if the process is terminated
    atexit(remove_shared_memory);
    // Create or get a semaphore if created to control the access to the shared memory
    int semid = semget(ftok("/home/mohamed/CLionProjects/producer_consumer", 3), 1, 0666 | IPC_CREAT);
    if (semid < 0) {
        cerr << "Error: Failed to get the semaphore." << endl;
        return 1;
    }
    else {
        cout << "Semaphore created successfully." << endl;
    }
    struct sembuf sem_buf;
    int retval;

    // Generate a random price according to the given gaussian distribution every sleep_interval milliseconds
    while (true) {
        // Try to get the semaphore
        sem_buf.sem_num = 0;
        sem_buf.sem_op = -1;
        sem_buf.sem_flg = SEM_UNDO;
        // Print test message
        cout << "Trying to get the semaphore." << endl;
        retval = semop(semid, &sem_buf, 1);
        cout << "Got the semaphore." << endl;
        if (retval < 0) {
            cerr << "Error: Failed to get the semaphore." << endl;
            return 1;
        }
        // Print semaphore acquired
        cout << "Semaphore acquired." << endl;
        cout << "INdex: " << *shm_index << endl;
        // Check if the shared memory is full
        if (*shm_index == 80) {
            // Print shared memory of 80
            cout << shm[3].name << " " << shm[3].price << endl;
            // Release the semaphore
            sem_buf.sem_num = 0;
            sem_buf.sem_op = 1;
            sem_buf.sem_flg = SEM_UNDO;
            retval = semop(semid, &sem_buf, 1);
            if (retval < 0) {
                cerr << "Error: Failed to release the semaphore." << endl;
                return 1;
            }
            // Sleep for sleep_interval milliseconds
            usleep(sleep_interval * 1000);
            continue;
        }
        else {
            // Generate a random price
            double price = price_mean + price_standard_deviation * rand() / RAND_MAX;
            strcpy(shm[*shm_index].name, name.c_str());
            shm[*shm_index].price = price;
            // Print the price and name and the index to the console
            cout << "Price: " << price << " Name: " << name << " Index: " << *shm_index << endl;
            // Increment the index
            *shm_index = *shm_index + 1;
            // Release the semaphore
            sem_buf.sem_num = 0;
            sem_buf.sem_op = 1;
            sem_buf.sem_flg = SEM_UNDO;
            retval = semop(semid, &sem_buf, 1);
            if (retval < 0) {
                cerr << "Error: Failed to release the semaphore." << endl;
                return 1;
            }
            // Sleep for sleep_interval milliseconds
            usleep(sleep_interval * 1000);
        }
    }
    return 0;
}
