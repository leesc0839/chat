//
// Created by 이승철 on 2023/10/12.
//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <string.h>
#include <signal.h>

#define BUF_SIZE 1024

void error_handling(char *message);

void reap_handler(int sig);

void child_process(int serv_sock, int clnt_sock, char *buf);

int main(int argc, char *argv[]) {

    if (argc != 2) {
        printf("usage : %s <port>\n", argv[0]);
        exit(1);
    }

    int serv_sock, clnt_sock;
    struct sockaddr_in serv_adr, clnt_adr;
    socklen_t clnt_adr_sz;
    char buf[BUF_SIZE];

    pid_t pid;
    struct sigaction act;
    act.sa_handler = reap_handler;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGCHLD, &act, 0);

    // socket init
    serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    if (serv_sock == -1)
        error_handling("socket error");

    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_adr.sin_port = htons(atoi(argv[1]));

    // bind
    if (bind(serv_sock, (struct sockaddr *) &serv_adr, sizeof(serv_adr)) == -1)
        error_handling("bind error");

    // listen
    if (listen(serv_sock, 5) == -1)
        error_handling("listen error");

    clnt_adr_sz = sizeof(clnt_adr);

    // accept
    while (1) {
        clnt_sock = accept(serv_sock, (struct sockaddr *) &clnt_adr, &clnt_adr_sz);

        // fork
        pid = fork();
        if (pid == -1)
            error_handling("fork error");

        if (pid == 0) {
            child_process(serv_sock, clnt_sock, buf);
            return 0;
        } else
            close(clnt_sock);
    }
    close(serv_sock);
}

void error_handling(char *message) {
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}

// when child process terminate, reaping zombie process
void reap_handler(int sig) {
    int status;
    pid_t pid = waitpid(-1, &status, WNOHANG);

    if (WIFEXITED(status)) {
        puts("reaping child process!");
        printf("child pid : %d\n", pid);
        printf("child exit value : %d\n", WEXITSTATUS(status));
    }
}

// echo
void child_process(int serv_sock, int clnt_sock, char *buf) {
    close(serv_sock);

    int str_len;
    while (1) {
        str_len = read(clnt_sock, buf, BUF_SIZE);
        if (str_len == 0) {
            close(clnt_sock);
            return;
        }
        buf[str_len] = 0;
        write(clnt_sock, buf, strlen(buf));
    }
}


