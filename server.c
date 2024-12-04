#include <stdio.h>

#include <stdlib.h>

#include <unistd.h>

#include <fcntl.h>

#include <string.h>

#include <signal.h>

 

struct message {

    char source[50];

    char target[50];

    char msg[200]; // message body

};

 

void terminate(int sig) {

    printf("Exiting....\n");

    fflush(stdout);

    exit(0);

}

 

int main() {

    int server;

    int target;

    int dummyfd;

    struct message req;

    signal(SIGPIPE,SIG_IGN);

    signal(SIGINT,terminate);

    server = open("serverFIFO",O_RDONLY);

    dummyfd = open("serverFIFO",O_WRONLY);

 

    while (1) {

        // TODO:

        // read requests from serverFIFO

        ssize_t readRequest = read(server, &req, sizeof(req));

        if (readRequest < 0) {

            perror("Failed to read from serverFIFO");

            continue;

        }

 

        if (readRequest == 0) {

            printf("Nothing received yet...\n");

            continue;

        }

 

        // if we make it here...we successfully read from serverFIFO

        printf("Received a request from %s to send the message %s to %s.\n", req.source, req.msg, req.target);

 

        target = open(req.target ,O_WRONLY);

        write(target, &req, sizeof(req));

        close(target);

    }

    close(server);

    close(dummyfd);

    return 0;

}