/* jfave Janne Louise F Ave */

#include "myUDP.h"
#include "mcodes.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

#define BUFFER_SIZE 128
#define MIN_PORT 1024
#define MAX_PORT 65535

// client code
int main(int argc, char *argv[]) {

    char ip [BUFFER_SIZE];
    char id [BUFFER_SIZE];
    int sport;                // Port for the server
    int cport;                // Port for the client

    // Get params
    if (argc == 4) {
        sscanf(argv[1], "%s", (char *) &ip);
        sscanf(argv[2], "%d", &sport);
        sscanf(argv[3], "%s", (char *) &id);
        // sscanf(argv[4], "%d", &cport);

        // printf("%s %d %s\n", ip, sport, id);
    } else {
        printf("Usage: ./client ip port id\n");
        return 0;
    }

    // Establish unique port for the client
    int port_found = 0;
    int port;
    int sd = -1;
    while (!port_found) {
        port = MIN_PORT + rand() / (RAND_MAX / (MAX_PORT - MIN_PORT + 1) + 1);
        sd = UDP_Open(port);
        if (port != sport && sd > -1) {
            // printf("Port found for new client %d\n", port);
            port_found = 1;
        }
    }

    // Open connection for client
    /*
    if (port == sport) {
        printf("Error: port' equals port, those values cannot be the same\n");
        return 0;
    }
    int sd = UDP_Open(port);
    if (sd <= -1) {
        printf("Error: port' %d is already being used\n", port);
        return 0;
    }
     */

    struct sockaddr_in addrSnd, addrRcv;

    int rc = UDP_FillSockAddr(&addrSnd, ip, sport);    // Establishing connection to server

    // Register with the server
    // Sending message to server
    // First create register message

    char gmessage[BUFFER_SIZE];
    strcpy(gmessage, REGISTER);
    strcat(gmessage, ":");
    strcat(gmessage, id);
    strcat(gmessage, "\n");

    rc = UDP_Write(sd, &addrSnd, gmessage, BUFFER_SIZE);

    // Receiving message from server
    char message[BUFFER_SIZE];
    if (rc > 0) {
        rc = UDP_Read(sd, &addrRcv, message, BUFFER_SIZE);
        printf("%s\n", message);

        if (strstr(message, "already") != NULL) {
            return 0;
        }
    } else {
        printf("Error: Unable to register with the server at port %d\n", sport);
        return 0;
    }

    char input [BUFFER_SIZE];
    int running = 1;
    while(running) {
        printf("> ");

        char * output = fgets(input, BUFFER_SIZE, stdin);

        // Worry about trailing and faulty input characters later
        if (output == NULL || output[0] == 'e') {
            running = 0;
        } else {
            // How to check for only newline?

            // Check white space is bad
            int white_spc_found = 0;
            for (int i = 0; i < strlen(output); i++) {
                if (output[i] == ' ' || output[i] == '\t') {
                    white_spc_found = 1;
                    break;
                }
            }

            if (white_spc_found) {
                printf("Error, no whitespace allowed in message!\n");
            } else {
                rc = UDP_Write(sd, &addrSnd, output, BUFFER_SIZE);

                // Receiving message from server
                if (rc > 0) {
                    rc = UDP_Read(sd, &addrRcv, message, BUFFER_SIZE);
                    printf("%s\n", message);
                }
            }
        }
  }

  return 0;
}
