#include <stdio.h>

#include <stdlib.h>

#include <spawn.h>

#include <sys/wait.h>

#include <unistd.h>

#include <string.h>

#include <fcntl.h>

#include <pthread.h>

 

#define N 13

 

extern char **environ;

char uName[20];

 

char *allowed[N] = {"cp","touch","mkdir","ls","pwd","cat","grep","chmod","diff","cd","exit","help","sendmsg"};

 

struct message {

    char source[50];

    char target[50];

    char msg[200];

};

 

void terminate(int sig) {

        printf("Exiting....\n");

        fflush(stdout);

        exit(0);

}

 

void sendmsg (char *user, char *target, char *msg) {

    // TODO:

    // Send a request to the server to send the message (msg) to the target user (target)

    // by creating the message structure and writing it to server's FIFO

    // create a message structure

    struct message m;

    strcpy(m.source, user);

    strcpy(m.target, target);

    strcpy(m.msg, msg);

 

    // open the server FIFO

    int server = open("serverFIFO", O_WRONLY);

    // make sure the serverFIFO was successfully opened

    if (server == -1) {

        perror("Failed to open serverFIFO");

        return;

    }

 

    write(server, &m, sizeof(m));

 

    // Close the server FIFO

    close(server);

}

 

void* messageListener(void *arg) {

    // TODO:

    // Read user's own FIFO in an infinite loop for incoming messages

    // The logic is similar to a server listening to requests

    // print the incoming message to the standard output in the

    // following format

    // Incoming message from [source]: [message]

    // put an end of line at the end of the message

    int userFIFO = open(uName, O_RDONLY);

    if (userFIFO == -1) {

        perror("Failed to open user FIFO\n");

        pthread_exit((void *)-1);

    }

 

    struct message incoming;

    while (1) {

        // read the incoming messages

        int readRequest = read(userFIFO, &incoming, sizeof(incoming));

        if (readRequest > 0) {

            printf("Incoming message from %s: %s\n", incoming.source, incoming.msg);

        }

    }

 

    close(userFIFO);

    pthread_exit((void*)0);

}

 

int isAllowed(const char*cmd) {

    int i;

    for (i=0;i<N;i++) {

        if (strcmp(cmd,allowed[i])==0) {

            return 1;

        }

    }

    return 0;

}

 

int main(int argc, char **argv) {

    pid_t pid;

    char **cargv;

    char *path;

    char line[256];

    int status;

    posix_spawnattr_t attr;

 

    if (argc!=2) {

    printf("Usage: ./rsh <username>\n");

    exit(1);

    }

    signal(SIGINT,terminate);

 

    strcpy(uName,argv[1]);

 

    // TODO:

    // create the message listener thread

    pthread_t listenerThread;

    if (pthread_create(&listenerThread, NULL, messageListener, NULL) != 0) {

        perror("Error creating message listener thread\n");

        exit(1);

    }

 

    while (1) {

 

    fprintf(stderr,"rsh>");

 

    if (fgets(line,256,stdin)==NULL) continue;

 

    if (strcmp(line,"\n")==0) continue;

 

    line[strlen(line)-1]='\0';

 

    char cmd[256];

    char line2[256];

    strcpy(line2,line);

    strcpy(cmd,strtok(line," "));

 

    if (!isAllowed(cmd)) {

        printf("NOT ALLOWED!\n");

        continue;

    }

 

    if (strcmp(cmd,"sendmsg")==0) {

        // TODO: Create the target user and

        // the message string and call the sendmsg function

 

        // NOTE: The message itself can contain spaces

        // If the user types: "sendmsg user1 hello there"

        // target should be "user1"

        // and the message should be "hello there"

 

        // if no argument is specified, you should print the following

        // printf("sendmsg: you have to specify target user\n");

        // if no message is specified, you should print the followingA

        // printf("sendmsg: you have to enter a message\n");

       

        char *target = strtok(NULL, " ");

        char *user= malloc(20);

        strcpy(user, target);

        if(target == NULL){

            printf("sendmsg: you have to specify target user\n");

            continue;

        }

 

       char *msg = malloc(256);

        target = strtok(NULL, " ");

        if(target == NULL){

            printf("sendmsg: you have to enter a message\n");

            continue;

        }

 

        strcat(msg, target);

        target = strtok(NULL, " ");

        while(target != NULL){

            strcat(msg, " ");

            strcat(msg, target);

            target = strtok(NULL, " ");

        }

        sendmsg(uName, user, msg);

 

        continue;

    }

 

    if (strcmp(cmd,"exit")==0) break;

 

    if (strcmp(cmd,"cd")==0) {

        char *targetDir=strtok(NULL," ");

        if (strtok(NULL," ")!=NULL) {

            printf("-rsh: cd: too many arguments\n");

        }

        else {

            chdir(targetDir);

        }

        continue;

    }

 

   if (strcmp(cmd,"help")==0) {

        printf("The allowed commands are:\n");

        for (int i=0;i<N;i++) {

            printf("%d: %s\n",i+1,allowed[i]);

        }

        continue;

    }

 

    cargv = (char**)malloc(sizeof(char*));

    cargv[0] = (char *)malloc(strlen(cmd)+1);

    path = (char *)malloc(9+strlen(cmd)+1);

    strcpy(path,cmd);

    strcpy(cargv[0],cmd);

 

    char *attrToken = strtok(line2," "); /* skip cargv[0] which is completed already */

    attrToken = strtok(NULL, " ");

    int n = 1;

    while (attrToken!=NULL) {

        n++;

        cargv = (char**)realloc(cargv,sizeof(char*)*n);

        cargv[n-1] = (char *)malloc(strlen(attrToken)+1);

        strcpy(cargv[n-1],attrToken);

        attrToken = strtok(NULL, " ");

    }

    cargv = (char**)realloc(cargv,sizeof(char*)*(n+1));

    cargv[n] = NULL;

 

    // Initialize spawn attributes

    posix_spawnattr_init(&attr);

 

    // Spawn a new process

    if (posix_spawnp(&pid, path, NULL, &attr, cargv, environ) != 0) {

        perror("spawn failed");

        exit(EXIT_FAILURE);

    }

 

    // Wait for the spawned process to terminate

    if (waitpid(pid, &status, 0) == -1) {

        perror("waitpid failed");

        exit(EXIT_FAILURE);

    }

 

    // Destroy spawn attributes

    posix_spawnattr_destroy(&attr);

 

    }

    return 0;

}

 

 

 

 

 

 

SERVER.C

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

 