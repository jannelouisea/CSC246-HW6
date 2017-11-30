/* jfave Janne Louise F Ave */

#include "myUDP.h"
#include <assert.h>
#include <stdio.h>

#define BUFFER_SIZE 128
#define NUM_OF_CITIES 5

int port = 0;

struct city {
    char * abv;
    int hour_avg_temp;
};

struct city cities[NUM_OF_CITIES] = {
        {"RDU", 0},
        {"CLT", 0},
        {"ATL", 0},
        {"CHS", 0},
        {"RIC", 0}
};

void print_cities() {
    for (int i = 0; i < NUM_OF_CITIES; i++) {
        printf("%s %d\n", cities[i].abv, cities[i].hour_avg_temp);
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

    print_cities();

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
            // char reply[BUFFER_SIZE];
            // sprintf(reply, "Hi from SERVER");
            char * reply = process_message(message);
            rc = UDP_Write(sd, &addr, reply, BUFFER_SIZE);
        }

        // Free message?
    }

    return 0;
}