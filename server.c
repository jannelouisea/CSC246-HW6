/* jfave Janne Louise F Ave */

#include "myUDP.h"
#include <assert.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>

#define BUFFER_SIZE 128
#define NUM_OF_CITIES 5
#define NUM_OF_CLIENTS 50

int port = 0;

/*
struct client {
    char * id;
    int in_use;
};

struct client clients[NUM_OF_CLIENTS];

int avail_clients = 0;
 */

struct city {
    char * abv;
    int curr_hour;
    int sum_tmps;
    int num_tms_curr_hour;
    int hour_avg_temp;
};

struct city cities[NUM_OF_CITIES] = {
        {"RDU", 0, 0, 0, 0},
        {"CLT", 0, 0, 0, 0},
        {"ATL", 0, 0, 0, 0},
        {"CHS", 0, 0, 0, 0},
        {"RIC", 0, 0, 0, 0}
};

int get_curr_hour() {
    struct timeval tv;
    time_t currtime;
    struct tm * currtm;
    int currhour;

    gettimeofday(&tv, NULL);
    currtime = tv.tv_sec;
    currtm = localtime(&currtime);
    currhour = currtm->tm_hour;
    printf("Curr hour %d\n", currhour);

    return currhour;
}

void set_cities() {
    int currhour = get_curr_hour();
    for (int i = 0; i < NUM_OF_CITIES; i++) {
        cities[i].curr_hour = currhour;
        printf("abv %s curr_hour %d sum %d num_tms %d tmp %d\n", cities[i].abv, cities[i].curr_hour, cities[i].sum_tmps, cities[i].num_tms_curr_hour, cities[i].hour_avg_temp);
    }
}

char * process_message(char * msg) {
    char * delim = ":";
    int colon_count = 0;
    char * tmp = msg;

    int j = 0;
    while(*(tmp + j)) {
        if (*(tmp + j) == ':') {
            colon_count++;
        }
        j++;
    }

    printf("colon_count %d\n", colon_count);

    int i = 0;
    char ** tokens = malloc((colon_count + 1) * sizeof(char *));

    tmp = msg;
    char * reply = malloc(BUFFER_SIZE * sizeof(char));

    if(tokens) {
        int idx = 0;
        char * token = strtok(tmp, ":");

        while(token) {
            *(tokens + idx) = malloc(strlen(token) + 1);
            strcpy(*(tokens + idx), token);
            idx++;
            token = strtok(0, ":");
        }

        printf("Tokens:\n");
        for (int d = 0; d < (colon_count + 1); d++) {
            printf("%s ", tokens[d]);
        }
        printf("\n");

        if (colon_count == 0 && (strcmp(tokens[0], "s") == 0 || strcmp(tokens[0], "S") == 0)) {
            printf("Show message\n");
            // send client temp
            strcpy(reply, "");

            for (int i = 0; i < NUM_OF_CITIES; i++) {
                strcat(reply, cities[i].abv);
                strcat(reply, ":");

                char temp[5];
                sprintf(temp, "%d", cities[i].hour_avg_temp);
                strcat(reply, temp);
                strcat(reply, " ");
            }
        } else if (colon_count == 1 && strcmp(tokens[0], "G") == 0) {
            printf("Register message\n");
            strcpy(reply, tokens[1]);
            strcat(reply, " registered with server");

            // check id client already exists
            /*
            int id_in_use = 0;
            int new_client_idx = -1;
            for (int i = 0; i < avail_clients; i++) {
                printf("i %d", i);
                if (strcmp(clients[i].id, tokens[1]) == 0) {
                    if (clients[i].in_use == 0) {
                        new_client_idx = i;
                        clients[i].in_use = 1;
                    } else {
                        id_in_use = 1;
                    }
                }
            }
            printf("Here");

            strcpy(reply, tokens[1]);
            if (!id_in_use) {
                if (new_client_idx < 0) {       // client id is not in clients
                    strcpy(clients[avail_clients].id, tokens[1]);
                    clients[avail_clients].in_use = 1;
                }
                strcat(reply, " registered with server");
            } else {
                strcat(reply, " id is already in use");
            }
             */
        } else  if (colon_count == 3 && (strcmp(tokens[0], "r") == 0 || strcmp(tokens[0], "R") == 0)) {
            // Check the city code
            int city_idx = -1;
            for (int i = 0; i < NUM_OF_CITIES; i++) {
                if (strcmp(tokens[1], cities[i].abv) == 0) {
                    city_idx = i;
                }
            }

            if (city_idx < 0) {
                strcpy(reply, "Error city code!");
            } else {
                // printf("Updating %s temp\n", cities[city_idx].abv);

                // Get the current Raleigh hour
                int currhour = get_curr_hour();

                if (atoi(tokens[2]) != currhour) {
                    strcpy(reply, "Error hour timestamp!");
                } else {
                    // printf("Updating city temp\n");
                    // check if tmp is in range
                    int temp = atoi(tokens[3]);
                    if (temp < 10 || temp > 99) {         // tmp must be a 2 digit, non neg #
                        strcpy(reply, "Error tmp is out of range [10, 99]!");
                    } else {
                        if (cities[city_idx].curr_hour < currhour) {    // hour has changed reset tmp
                            cities[city_idx].curr_hour = currhour;
                            cities[city_idx].sum_tmps = 0;
                            cities[city_idx].num_tms_curr_hour = 0;
                            cities[city_idx].hour_avg_temp = 0;
                        }

                        cities[city_idx].sum_tmps += temp;
                        cities[city_idx].num_tms_curr_hour++;
                        cities[city_idx].hour_avg_temp = cities[city_idx].sum_tmps / cities[city_idx].num_tms_curr_hour;

                        strcpy(reply, "Successfully report temperature!");
                    }
                }
            }
        } else {
            strcpy(reply, "Message ");
            strcat(reply, msg);
            strcat(reply, " is invalid");
        }

        free(token);
        printf("\n");
    } else {
        printf("Error: cannot make tokens from message\n");
        // ???
    }

    free(tokens);

    return reply;
}

int main(int argc, char *argv[]) {

    set_cities();

    // clients = malloc(NUM_OF_CLIENTS * sizeof(struct client));

    // Get the port number
    if (argc == 2) {
        sscanf(argv[1], "%d", &port);
    } else {
        printf("Usage: ./server port\n");
        return 0;
    }

    int sd = UDP_Open(port);                    // Creating the port for the server
    assert(sd > -1);
    while (1) {
        struct sockaddr_in addr;                // Only have one address?

        // Receiving message from client
        char * message = malloc(BUFFER_SIZE * sizeof(char));
        int rc = UDP_Read(sd, &addr, message, BUFFER_SIZE);
        printf("Server got message %s\n", message);

        int len = strlen(message);
        message[len - 1] = '\0';

        // Sending message back to client
        if (rc > 0) {
            char * reply = process_message(message);
            rc = UDP_Write(sd, &addr, reply, BUFFER_SIZE);
        }

        // Free message?
    }

    // free(clients);

    return 0;
}
