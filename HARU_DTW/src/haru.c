// #include "include/haru.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include "include/squiggle_search.h"
#include "include/haru.h"
#include "include/reference.h"

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
    struct addrinfo *servinfo;
    struct sockaddr_storage client_addr;
    socklen_t sin_size;
    struct sigaction sa;
    int yes = 1;
    char s[INET6_ADDRSTRLEN];
    int rv;

    struct sockaddr_in myaddr;
    myaddr.sin_family = AF_INET;
    myaddr.sin_port = htons(PORT);
    myaddr.sin_addr.s_addr = inet_addr(MY_STATIC_IP);
    memset(myaddr.sin_zero, '\0', sizeof myaddr.sin_zero);

    if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
        perror("HARU: socket");
        exit(1);
    }

    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
        sizeof(int)) == -1) {
        perror("setsockopt");
        exit(1);
    }

    if (bind(sockfd, (struct sockaddr *) &myaddr, sizeof myaddr) == -1) {
        close(sockfd);
        perror("HARU: bind");
        exit(1);
    }

    /* Start listening with queue of BACKLOG*/
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

    printf("HARU: waiting for connections...\n");
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
            char msg_buffer[BUFSIZE];
            while (1) {
                ssize_t numBytesRcvd = recv(confd, msg_buffer, BUFSIZE, 0);
                msg_buffer[numBytesRcvd] = '\0';
                printf("(%ld) ", numBytesRcvd);
                for (int i = 0; i < numBytesRcvd; i += sizeof(float)) {
                    float v = *(msg_buffer + i);
                    printf("%f ", v);
                }
                printf("\n");

                if (send(confd, "Got it!", 7, 0) == -1) {
                    perror("send");
                    break;
                }
            }
            close(confd);
            exit(0);
        }
        /* Parent after fork */
        close(confd);
    }

    return 0;
}