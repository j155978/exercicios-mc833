#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
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
    char   buf[MAXDATASIZE], temp[MAXDATASIZE];

    // Lista de tarefas que podem ser enviadas para o cliente
    char *TAREFAS[] = {"LIMPEZA", "COZINHA", "ESTUDOS", "LOUÇA", "COMPRAS", "ENCERRAR"};
    int qtdTarefas = 6;

    listenfd = Socket(AF_INET, SOCK_STREAM, 0);

    // Configuração do endereço do servidor
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family      = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    // Conversão da porta passada como argumento
    int port_arg = atoi(argv[1]);
    servaddr.sin_port        = htons((unsigned int)port_arg);

    Bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr));

    socklen_t len = sizeof(servaddr);
    // Obtém informações do socket
    Getsockname(listenfd, (struct sockaddr *)&servaddr, &len);

    // Coloca o socket em modo de escuta
    if (listen(listenfd, LISTENQ) == -1) {
        perror("listen");
        exit(1);
    }

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

            // Log de conexão do cliente
            FILE *logFile = fopen(arquivo, "a");
            char logLine2[5000];
            sprintf(logLine2, "Cliente conectado IP/porta cliente: %s/%d\n", ip_str_cli, ntohs(client_addr.sin_port));
            fputs(logLine2, logFile);
            fclose(logFile);

            // Envia tarefas aleatórias para o cliente até receber o comando "ENCERRAR"
            char logLineTarefa[100];
            while(strcmp("ENCERRAR", temp)!=0) {

                // Seleciona uma tarefa aleatória
                int tarefa = GetRandomNumber(qtdTarefas);  
                logFile = fopen(arquivo, "a");

                // Log da tarefa enviada
                sprintf(logLineTarefa, "Enviando tarefa %s\n", TAREFAS[tarefa]);
                fputs(logLineTarefa, logFile);
                fclose(logFile);

                snprintf(temp, sizeof(buf), TAREFAS[tarefa]);
                snprintf(buf, sizeof(buf), TAREFAS[tarefa]);
                // Envia a tarefa para o cliente
                write(connfd, buf, strlen(buf)); 

                int n;
                while ((n = read(connfd, buf, MAXDATASIZE-1)) > 0) {
                    buf[n] = 0;

                    // Log da resposta do cliente
                    logFile = fopen(arquivo, "a");
                    sprintf(logLine2, "%s\n", buf);
                    fputs(logLine2, logFile);
                    fclose(logFile);
                    break;
                }
            }

            // Log de desconexão do cliente
            logFile = fopen(arquivo, "a");
            sprintf(logLine2, "Cliente finalizado IP/porta cliente: %s/%d\n", ip_str_cli, ntohs(client_addr.sin_port));
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
