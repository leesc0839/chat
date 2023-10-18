#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 30

void error_handling(char *message);

int main(int argc, char *argv[]) {
    int serv_sock, clnt_sock;
    struct sockaddr_in serv_adr, clnt_adr;

    pid_t pid;
    socklen_t adr_sz;
    int str_len;
    char buf[BUF_SIZE];

    serv_sock = socket(PF_INET, SOCK_STREAM, 0);

    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_adr.sin_port = htons(atoi(argv[1]));

    if (bind(serv_sock, (struct sockaddr *) &serv_adr, sizeof(serv_adr)) == -1)
        error_handling("bind() error");
    if (listen(serv_sock, 5) == -1)
        error_handling("listen() error");

    adr_sz = sizeof(clnt_adr);
    clnt_sock = accept(serv_sock, (struct sockaddr *) &clnt_adr, &adr_sz);

    pid = fork();
    if (pid == 0) {
        close(serv_sock);
        while ((str_len = read(clnt_sock, buf, BUF_SIZE)) != 0) {
            buf[str_len] = 0;
            printf("client sent : %s\n", buf);
        }
        close(clnt_sock);
        return 0;
    } else {
        int status;
        while (1) {
            pid_t tmp = waitpid(-1, &status, WNOHANG);

            fgets(buf, BUF_SIZE, stdin);
            printf("child process pid : %d\n" , pid);
            printf("current exited process pid : %d\n\n" , tmp);

            if(tmp == pid)
                break;

            write(clnt_sock, buf, strlen(buf));
        }
        close(clnt_sock);
    }

    close(serv_sock);
    return 0;
}

void error_handling(char *message) {
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}
