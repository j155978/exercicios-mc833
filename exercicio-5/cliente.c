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
#include <sys/time.h>
#include <time.h>

#define MAXLINE 4096
char arquivo[] = "logscliente.txt"; // Arquivo de log

// Função que cria um socket e verifica erros
int Socket(int family, int type, int flags) {
    int sockfd;
    if ((sockfd = socket(family, type, flags)) < 0) {
        perror("socket");
        exit(1);
    } else
        return sockfd;
}


void GetCurrentTime(char *hora, size_t tamanho) {
    struct timeval tv;
    gettimeofday(&tv, NULL);

    // Converte o tempo para a estrutura tm
    struct tm *tm_info = localtime(&tv.tv_sec);

    // Formata a data e hora com milissegundos
    snprintf(hora, tamanho, "%04d-%02d-%02d %02d:%02d:%02d.%03ld",
             tm_info->tm_year + 1900, 
             tm_info->tm_mon + 1, 
             tm_info->tm_mday, 
             tm_info->tm_hour, 
             tm_info->tm_min, 
             tm_info->tm_sec, 
             tv.tv_usec / 1000); // Converte microsegundos para milissegundos
}

int main(int argc, char **argv) {
    int    sockfd, n;
    char   recvline[MAXLINE + 1];
    char   error[MAXLINE + 1];
    struct sockaddr_in servaddr;

    // Verifica se os parâmetros de IP e porta foram passados corretamente
    if (argc != 4) {
        strcpy(error, "uso: ");
        strcat(error, argv[0]);
        strcat(error, " <IPaddress>");
        perror(error);
        exit(1);
    }

    sockfd = Socket(AF_INET, SOCK_STREAM, 0);

    // Configura o endereço do servidor
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    int port_arg = atoi(argv[2]);

    int clientId = atoi(argv[3]);

    servaddr.sin_port   = htons((unsigned int)port_arg);

    // Converte o IP fornecido para o formato binário
    if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0) {
        perror("inet_pton error");
        exit(1);
    }
    char hora[50];
    char logLine2[5000];
    GetCurrentTime(hora, sizeof(hora));
    FILE *logFile = fopen(arquivo, "a");

    sprintf(logLine2, "%s : Tentando conexão clienteId %d\n", hora, clientId);
    printf("%s", logLine2);
    fputs(logLine2, logFile);
    fclose(logFile);

    // Estabelece a conexão com o servidor
    if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
        perror("connect error");
        exit(1);
    }

    // Obtém e imprime informações sobre o socket local e o servidor
    struct sockaddr_in local_addr;
    socklen_t addr_len = sizeof(local_addr);

    if (getsockname(sockfd, (struct sockaddr *)&local_addr, &addr_len) < 0) {
        perror("getsockname");
        exit(1);
    }

    char ip_str[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(servaddr.sin_addr), ip_str, sizeof(ip_str));

    inet_ntop(AF_INET, &(local_addr.sin_addr), ip_str, sizeof(ip_str));
    printf("clientId %d Conectado. IP: %s ", clientId, ip_str);
    int porta = ntohs(local_addr.sin_port);
    printf("Porta: %d\n", porta);
    
    while ( (n = read(sockfd, recvline, MAXLINE)) > 0) {
        recvline[n] = 0;
        if (fputs(recvline, stdout) == EOF) {
            perror("fputs error");
            exit(1);
        }
    }

    sleep(2);
    close(sockfd);

    GetCurrentTime(hora, sizeof(hora));
    logFile = fopen(arquivo, "a");
    sprintf(logLine2, "%s : clienteId %d Finalizando!\n", hora, clientId);
    printf("%s", logLine2);
    fputs(logLine2, logFile);
    fclose(logFile);


    exit(0);
}
