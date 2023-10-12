//
// Created by 이승철 on 2023/10/12.
//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>

#define BUF_SIZE 30

void error_handing(char *message);

int main(int argc, char *argv[]) {

    if (argc != 3) {
        printf("usage : %s <ip> <port>\n", argv[0]);
        exit(1);
    }

    int serv_sock;
    struct sockaddr_in serv_adr;

    // socket init
    serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    if (serv_sock == -1)
        error_handing("socket error");

    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_adr.sin_port = htons(atoi(argv[2]));

    // connect
    if (connect(serv_sock, (struct sockaddr *) &serv_adr, sizeof(serv_adr)) == -1)
        error_handing("connect error");
    else
        puts("connect");

    char buf[BUF_SIZE];
    int str_len;

    // write
    while (1) {
        fputs("Input message(Q to quit:) ", stdout);
        fgets(buf, BUF_SIZE, stdin);

        if (!strcmp(buf, "q\n") || !strcmp(buf, "Q\n"))
            break;

        write(serv_sock, buf, strlen(buf));
        str_len = read(serv_sock, buf, BUF_SIZE - 1);
        buf[str_len] = 0;
        printf("server send : %s\n", buf);
    }

    //close
    close(serv_sock);
}


void error_handing(char *message) {
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}
