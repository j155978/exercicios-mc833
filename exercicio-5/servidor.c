#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <arpa/inet.h>

#define LISTENQ 10
#define MAXDATASIZE 100
char arquivo[] = "logs.txt"; // Arquivo de log

// Função que retorna um número aleatório de 0 até n-1
int GetRandomNumber(int n) {
    return rand() % n;
}

// Função para criar um socket e verificar erros
int Socket(int family, int type, int flags) {
    int sockfd;
    if ((sockfd = socket(family, type, flags)) < 0) {
        perror("socket");
        exit(1);
    }
    return sockfd;
}

// Envelopamento função Getpeername
void Getpeername(int connfd, struct sockaddr *__restrict__ client_addr, socklen_t *__restrict__ addrlen) {
    if (getpeername(connfd, client_addr, addrlen) < 0) {
        perror("getpeername");
        exit(1);
    }
}

// Envelopamento função Bind
void Bind(int listenfd, const struct sockaddr *servaddr, socklen_t __len) {
    if (bind(listenfd, servaddr, __len) == -1) {
        perror("bind");
        exit(1);
    }
}

// Envelopamento função Getsockname
void Getsockname(int listenfd, struct sockaddr *__restrict__ servaddr, socklen_t *__restrict__ len) {
    if (getsockname(listenfd, servaddr, len) == -1) {
        perror("getsockname");
        exit(1);
    }
}

void Listen(int sockfd, int backlog){
    if (listen(sockfd, backlog) == -1) {
        perror("listen");
        exit(1);
    }
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

// Envelopamento função Accept
int Accept(int listenfd, struct sockaddr *__restrict__ __addr, socklen_t *__restrict__ __addr_len) {
    int connfd;
    if ((connfd = accept(listenfd, (struct sockaddr *) NULL, NULL)) == -1) {
        perror("accept");
        exit(1);
    }
    return connfd;
}

int main (int argc, char **argv) {

    int    listenfd, connfd;
    struct sockaddr_in servaddr;
    char   buf[MAXDATASIZE];
    listenfd = Socket(AF_INET, SOCK_STREAM, 0);

    // Configuração do endereço do servidor
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family      = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    // Conversão da porta passada como argumento
    int port_arg = atoi(argv[1]);

    int backlog = atoi(argv[2]);

    servaddr.sin_port        = htons((unsigned int)port_arg);

    Bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr));

    socklen_t len = sizeof(servaddr);
    // Obtém informações do socket
    Getsockname(listenfd, (struct sockaddr *)&servaddr, &len);

    // Coloca o socket em modo de escuta
    Listen(listenfd, backlog);

    // Loop principal para aceitar conexões
    for ( ; ; ) {
 
        // Aceita uma nova conexão
        connfd = Accept(listenfd, (struct sockaddr *) NULL, NULL);
        
        // Cria um processo filho para lidar com o cliente
        pid_t pid = fork(); 

        if (pid < 0) {
            perror("fork");
        }
        if (pid == 0) {
            // Processo filho

            // Inicializa a semente para gerar números aleatórios
            srand(time(NULL));
            // Fecha o socket de escuta no processo filho
            close(listenfd); 

            struct sockaddr_in client_addr;
            socklen_t addrlen = sizeof(client_addr);

            // Obtém informações do cliente
            Getpeername(connfd, (struct sockaddr *)&client_addr, &addrlen);

            char ip_str_svr[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &(servaddr.sin_addr), ip_str_svr, sizeof(ip_str_svr));

            char ip_str_cli[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &(client_addr.sin_addr), ip_str_cli, sizeof(ip_str_cli));

            // Exibe a data e hora formatadas
            char hora[50];
            GetCurrentTime(hora, sizeof(hora));
            FILE *logFile = fopen(arquivo, "a");
            char logLine2[5000];
            sprintf(logLine2, "%s : Cliente conectado IP/porta cliente: %s/%d\n", hora, ip_str_cli, ntohs(client_addr.sin_port));
            printf("%s", logLine2);
            fputs(logLine2, logFile);
            fclose(logFile);


            snprintf(buf, sizeof(buf), "Fim\n");
            write(connfd, buf, strlen(buf));

            // Log de desconexão do cliente
            logFile = fopen(arquivo, "a");
            sprintf(logLine2, "%s : Cliente finalizado IP/porta cliente: %s/%d\n", hora, ip_str_cli, ntohs(client_addr.sin_port));
            fputs(logLine2, logFile);
            fclose(logFile);

            close(connfd);  // Fecha a conexão com o cliente
            exit(0);
        } 
        else {
            // Processo pai
            close(connfd);  // Fecha o socket do cliente no processo pai
            fflush(stdin);
        }
    }
    return(0);
}
