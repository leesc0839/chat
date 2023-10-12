#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 30

void error_handling(char *message);

void read_routine(int sock, char *buf);

void write_routine(int sock, char *buf);

int main(int argc, char *argv[]) {
    int sock;
    struct sockaddr_in serv_adr;
    char buf[BUF_SIZE];
    pid_t pid;

    if (argc != 3) {
        printf("usage : %s <IP> <Port>", argv[0]);
        exit(1);
    }

    // socket , init
    sock = socket(PF_INET, SOCK_STREAM, 0);
    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_adr.sin_port = htons(atoi(argv[2]));

    // connect
    if (connect(sock, (struct sockaddr *) &serv_adr, sizeof(serv_adr)) == -1)
        error_handling("conn error");

    /*
     * fork
     * child -> write
     * parent -> read
     */

    pid = fork();
    if (pid == 0)
        write_routine(sock, buf);
    else
        read_routine(sock, buf);

    close(sock); // close sock(parent process) - 2
}

// parent process
void read_routine(int sock, char *buf) {
    int str_len;
    while (1) {
        str_len = read(sock, buf, BUF_SIZE);
        if (str_len == 0) { // server's fin
            puts("server last fin success");
            return;
        }
        buf[str_len] = 0;
        printf("message from server : %s", buf);
    }
}

// child process
void write_routine(int sock, char *buf) {
    while (1) {
        fgets(buf, BUF_SIZE, stdin);
        if (!strcmp(buf, "q\n") || !strcmp(buf, "Q\n")) {
            shutdown(sock, SHUT_WR); // close sock(child process) - 1
            return;
        }
        write(sock, buf, strlen(buf));
    }
}

void error_handling(char *message) {
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}