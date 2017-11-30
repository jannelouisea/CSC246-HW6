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

    return currhour;
}

void set_cities() {
    int currhour = get_curr_hour();
    for (int i = 0; i < NUM_OF_CITIES; i++) {
        cities[i].curr_hour = currhour;
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

        if (colon_count == 0 && (strcmp(tokens[0], "s") == 0 || strcmp(tokens[0], "S") == 0)) {
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
            strcpy(reply, tokens[1]);
            strcat(reply, " registered with server");
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
                // Get the current Raleigh hour
                int currhour = get_curr_hour();

                if (atoi(tokens[2]) != currhour) {
                    strcpy(reply, "Error hour timestamp!");
                } else {
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
    } else {
        // printf("Error: cannot make tokens from message\n");
    }

    free(tokens);

    return reply;
}

int main(int argc, char *argv[]) {

    set_cities();

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

        int len = strlen(message);
        message[len - 1] = '\0';

        // Sending message back to client
        if (rc > 0) {
            char * reply = process_message(message);
            rc = UDP_Write(sd, &addr, reply, BUFFER_SIZE);
        }

        free(message);
    }

    return 0;
}
