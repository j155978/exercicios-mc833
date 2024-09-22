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
char arquivo[] = "logs.txt";


int GetRandomNumber(int n){
    return rand() % (n);
}

int Socket(int family, int type, int flags) {
    int sockfd;
    if ((sockfd = socket(family, type, flags)) < 0) {
        perror("socket");
        exit(1);
    } else
    return sockfd;
}

void Getpeername(int connfd, struct sockaddr *__restrict__ client_addr, socklen_t *__restrict__ addrlen){
    if (getpeername(connfd, client_addr, addrlen) < 0) {
        perror("getpeername");
        exit(1);
    }
}

void Bind(int listenfd, const struct sockaddr *servaddr, socklen_t __len){
    if (bind(listenfd, servaddr, __len) == -1) {
        perror("bind");
        exit(1);
    }
}

void Getsockname(int listenfd, struct sockaddr *__restrict__ servaddr, socklen_t *__restrict__ len){
    if (getsockname(listenfd, servaddr, len) == -1) {
        perror("getsockname");
        exit(1);
    }
}

int Accept(int listenfd, struct sockaddr *__restrict__ __addr, socklen_t *__restrict__ __addr_len){
    int connfd;
    if ((connfd = accept(listenfd, (struct sockaddr *) NULL, NULL)) == -1 ) {
        perror("accept");
        exit(1);
    }
    return connfd;
}

int main (int argc, char **argv) {

    int    listenfd, connfd;
    struct sockaddr_in servaddr;
    char   buf[MAXDATASIZE], temp[MAXDATASIZE];

    
    char *TAREFAS[] = {"LIMPEZA", "COZINHA", "ESTUDOS", "LOUÇA", "COMPRAS", "ENCERRAR"};
    int qtdTarefas = 6;
    
    listenfd = Socket(AF_INET, SOCK_STREAM, 0);

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family      = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    int port_arg = atoi(argv[1]);
    servaddr.sin_port        = htons((unsigned int)port_arg);

    Bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr));

    socklen_t len = sizeof(servaddr);
    Getsockname(listenfd, (struct sockaddr *)&servaddr, &len);

    if (listen(listenfd, LISTENQ) == -1) {
        perror("listen");
        exit(1);
    }
    
    for ( ; ; ) {
      
        connfd = Accept(listenfd, (struct sockaddr *) NULL, NULL);

        pid_t pid = fork();

        if (pid < 0) {
            perror("fork");
        }
        if (pid == 0) {
            // ChildProcess();
            srand(time(NULL));
            close(listenfd);

            struct sockaddr_in client_addr;
            socklen_t addrlen = sizeof(client_addr);

            Getpeername(connfd, (struct sockaddr *)&client_addr, &addrlen);

            char ip_str_svr[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &(servaddr.sin_addr), ip_str_svr, sizeof(ip_str_svr));

            char ip_str_cli[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &(client_addr.sin_addr), ip_str_cli, sizeof(ip_str_cli));

            FILE *logFile = fopen(arquivo, "a");

            char logLine2[5000];
            sprintf(logLine2, "Cliente conectado IP/porta cliente: %s/%d\n", ip_str_cli, ntohs(client_addr.sin_port));
            fputs(logLine2, logFile);
            fclose(logFile);

            char logLineTarefa[100];

            while(strcmp("ENCERRAR", temp)!=0){

                int tarefa = GetRandomNumber(qtdTarefas);
                logFile = fopen(arquivo, "a");
                // printf("Enviando tarefa %s\n", TAREFAS[tarefa]);
                sprintf(logLineTarefa, "Enviando tarefa %s\n", TAREFAS[tarefa]);
                fputs(logLineTarefa, logFile);
                fclose(logFile);
                snprintf(temp, sizeof(buf), TAREFAS[tarefa]);
                snprintf(buf, sizeof(buf), TAREFAS[tarefa]);
                write(connfd, buf, strlen(buf));
                int n;
                while ( (n = read(connfd, buf, MAXDATASIZE-1)) > 0) {
                    buf[n] = 0;
                    // printf("Cliente finalizado:\n");
                    // if (fputs(buf, stdout) == EOF) {
                    //     perror("fputs error");
                    //     exit(1);
                    // }

                    logFile = fopen(arquivo, "a");

                    sprintf(logLine2, "%s\n", buf);
                    fputs(logLine2, logFile);


                    fclose(logFile);
                    break;
                }
            }


            logFile = fopen(arquivo, "a");
            sprintf(logLine2, "Cliente finalizado IP/porta cliente: %s/%d\n", ip_str_cli, ntohs(client_addr.sin_port));
            fputs(logLine2, logFile);
            fclose(logFile);

            // printf("IP/porta servidor: %s/%d\n", ip_str_svr, ntohs(servaddr.sin_port));
            // printf("IP/porta cliente: %s/%d\n", ip_str_cli, ntohs(client_addr.sin_port));
            // printf("Fechando filho!\n");

            close(connfd);

            exit(0);
        }
        else {
            //ParentProcess();
            close(connfd);
            // printf("Finalizando conexão no pai\n");
            fflush(stdin);
        }
    }
    return(0);
}
