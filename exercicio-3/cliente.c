#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define MAXLINE 4096

int main(int argc, char **argv) {
    int    sockfd, n;
    char   recvline[MAXLINE + 1];
    char   error[MAXLINE + 1];
    char   messagebuffer[MAXLINE + 1]; //buffer para envio da mensagem
    struct sockaddr_in servaddr;


    // modificado
    if (argc != 3) {
        strcpy(error,"uso: ");
        strcat(error,argv[0]);
        strcat(error," <IPaddress>");
        perror(error);
        exit(1);
    }

    if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket error");
        exit(1);
    }

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    
    // modificado
    int port_arg = atoi(argv[2]);
    servaddr.sin_port   = htons((unsigned int)port_arg);
    // modificado

    if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0) {
        perror("inet_pton error");
        exit(1);
    }

    if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
        perror("connect error");
        exit(1);
    }

    // Exercício 5
    struct sockaddr_in local_addr;
    socklen_t addr_len = sizeof(local_addr);

    if (getsockname(sockfd, (struct sockaddr *)&local_addr, &addr_len) < 0) {
        perror("getsockname");
        exit(1);
    }

    char ip_str[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(local_addr.sin_addr), ip_str, sizeof(ip_str));

    printf("Informações do socket local:\n");
    printf("IP: %s\n", ip_str);
    printf("Porta: %d\n\n", ntohs(local_addr.sin_port));
    //

    //Exercicio 7, envio de mensagem para o servidor

    printf("Digite a mensagem a ser enviada: ");
    if (fgets(messagebuffer, MAXLINE, stdin) != NULL){
        write(sockfd, messagebuffer, strlen(messagebuffer));
    }

    // end of exercicio 7

    while ( (n = read(sockfd, recvline, MAXLINE)) > 0) {
        recvline[n] = 0;
        if (fputs(recvline, stdout) == EOF) {
            perror("fputs error");
            exit(1);
        }
    }

    if (n < 0) {
        perror("read error");
        exit(1);
    }

    exit(0);
}
