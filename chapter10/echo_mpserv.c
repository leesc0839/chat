#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

#define BUF_SIZE 30

void error_handling(char *message);

void read_childproc(int sig);

void read_routine(int serv_sock, int clnt_sock, char *buf);

void write_routine(int sock, char *buf);

volatile sig_atomic_t flag = 0;

int main(int argc, char *argv[]) {
    int serv_sock, clnt_sock;
    struct sockaddr_in serv_adr, clnt_adr;

    socklen_t clnt_sock_sz;
    char buf[BUF_SIZE];

    pid_t pid;
    int state;
    struct sigaction act;
    act.sa_handler = read_childproc;
    sigemptyset(&act.sa_flags);
    act.sa_flags = 0;
    state = sigaction(SIGCHLD, &act, 0);

    if (argc != 2) {
        printf("usage : %s <port> \n", argv[0]);
        exit(1);
    }

    // socket init (serv_sock)
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

    // while-loop (accept)
    clnt_sock_sz = sizeof(clnt_adr);
    while (1) {
        // create clnt_sock(tcp)
        clnt_sock = accept(serv_sock, (struct sockaddr *) &clnt_adr, &clnt_sock_sz);
        if (clnt_sock == -1)
            error_handling("accept error");

        /*
         * fork
         * child -> read
         * parent -> write
         */
        pid = fork();
        if (pid == -1)
            error_handling("fork error");

        if (pid == 0) {
            read_routine(serv_sock, clnt_sock, buf);
            return 0; // return child process -> signal handling
        } else
            write_routine(clnt_sock, buf);

        close(clnt_sock); // close clnt_sock(parent process) - 2
    }
    close(serv_sock);
}

// child process
void read_routine(int serv_sock, int clnt_sock, char *buf) {
    close(serv_sock); // close serv_sock(child process) - 1

    int str_len;
    while (1) {
        str_len = read(clnt_sock, buf, BUF_SIZE);
        if (str_len == 0) {
            close(clnt_sock); // close clnt_sock(child process) - 1
            puts("client disconnected...");
            return;
        }

        buf[str_len] = 0;
        printf("message from client : %s\n", buf);
    }
}

// parent process
void write_routine(int clnt_sock, char *buf) {
    while (1) {
        // if child process killed, handler change this flag
        if (flag) {
            flag = 0;
            return;
        }

        fgets(buf, BUF_SIZE, stdin);
        write(clnt_sock, buf, strlen(buf));
    }
}

// handler : reaping, flag
void read_childproc(int sig) {
    int status;
    pid_t pid = waitpid(-1, &status, WNOHANG);

    if (WIFEXITED(status)) {
        puts("child process reap");
        printf("child pid %d\n", pid);
        printf("exit status %d\n", WEXITSTATUS(status));
        flag = 1;
    }
}

void error_handling(char *message) {
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}