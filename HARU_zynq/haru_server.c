/***********************************************************************
 * FILENAME: haru_server.h
 * DESCRIPTION:
 *     HARU Server hosts a server socket and listens for client connections.
 *     When a client connects, a new thread is created to handle the client.
 *     The server calls sDTW on the client data and returns the result to the client.
 * NOTES:
 *
 * AUTHOR: Elton Shih        START DATE: 25/06/2021
 * CHANGES:
 *
 * VERSION DATE       WHO     DETAIL
 * v0.01   29/09/2021 ES      Init Draft version
 *
 **/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include "haru/haru.h"


/* Server information */
#define MY_STATIC_IP "10.10.130.3" // Hardcoded for now..
#define PORT 3490
#define BACKLOG 10
#define BUFSIZE 4096

/* Payload struct */
typedef struct payload_t {
    uint32_t id[55];
    int16_t query_seq[55][250];
} payload_t;

void sigchld_handler(int s) {
    int saved_errno = errno;
    while (waitpid(-1, NULL, WNOHANG) > 0);
    errno = saved_errno;
}

void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in *) sa)->sin_addr);
    }

    return &(((struct sockaddr_in6 *) sa)->sin6_addr);
}

int main() {
    int sockfd, confd;
    struct sockaddr_storage client_addr;
    socklen_t sin_size;
    struct sigaction sa;
    int yes = 1;
    char s[INET6_ADDRSTRLEN];

    struct sockaddr_in myaddr;
    myaddr.sin_family = AF_INET;
    myaddr.sin_port = htons(PORT);
    myaddr.sin_addr.s_addr = inet_addr(MY_STATIC_IP);
    memset(myaddr.sin_zero, '\0', sizeof myaddr.sin_zero);

    /* Create a socket */
    if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
        perror("HARU: socket");
        exit(1);
    }

    /* Set socket to reuse address */
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
        sizeof(int)) == -1) {
        perror("setsockopt");
        exit(1);
    }

    /* Bind socket to address */
    if (bind(sockfd, (struct sockaddr *) &myaddr, sizeof myaddr) == -1) {
        close(sockfd);
        perror("HARU: bind");
        exit(1);
    }

    /* Start listening with queue of BACKLOG */
    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    /* Reap all dead processes */
    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }

    /* Setup HARU */
    haru_t haru;
    haru_init(&haru);

    /* Create a shared semaphore */
    sem_t *sem = sem_open("/haru_sem", O_CREAT, 0644, 1);
    if (sem == SEM_FAILED) {
    	perror("Semaphore error\n");
    }

    printf("HARU: waiting for connections...\n");

    /* Accept connections */
    while (1) {
        sin_size = sizeof client_addr;
        confd = accept(sockfd, (struct sockaddr *) &client_addr, &sin_size);
        if (confd == -1) {
            perror("accept");
            continue;
        }

        /* Convert client address from network to string */
        inet_ntop(client_addr.ss_family, get_in_addr((struct sockaddr *) &client_addr), s, sizeof s);
        printf("HARU: got connection from %s\n", s);

        /* Child process handles the incoming connection */
        if (!fork()) {
            /* Child */
            close(sockfd);

            /* Receive payload */
            payload_t payload;

            /* Receive ids */
            int bytes_received = recv(confd, payload.id, sizeof(payload.id[0])*55, 0);
            if (bytes_received == -1) {
                perror("recv");
                exit(1);
            }

            for (int i = 0; i < 55; i++) {
            	int bytes_received = recv(confd, &payload.query_seq[i], sizeof(int16_t)*250, 0);
                if (bytes_received == -1) {
                    perror("recv");
                    exit(1);
                }
            }

            for (int i = 0; i < 55; i++) {
                printf("id: %d\n", payload.id[i]);
                for (int j = 0; j < 250; j++) {
                    printf("%d ", payload.query_seq[i][j]);
                }
                printf("\n============\n");
            }
            printf("\n");


			// printf("Content: id=%d\n", payload->id);

			/* Call sDTW */
			// query_t results;
			// query_init(&results, payload->id);
//			sem_wait(sem);
//			haru_process_query(&haru, payload->query_seq, &results);
//			sem_post(sem);

			/* Send result to client */
			// if (send(confd, (char *)&results, sizeof(results), 0) == -1) {
			// 	perror("send");
			// 	break;
			// }
			// printf("Sent ID=%d\n", results.ID);
            close(confd);
            exit(0);
        }
        /* Parent after fork */
        close(confd);
    }

    haru_cleanup(&haru);
    sem_close(sem);
    return 0;
}
