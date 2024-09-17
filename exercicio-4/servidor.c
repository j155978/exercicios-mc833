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

int GetRandomNumber(int n){
    srand(time(NULL));

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

int main (int argc, char **argv) {
    int    listenfd, connfd;
    struct sockaddr_in servaddr;
    char   buf[MAXDATASIZE];

    char arquivo[] = "logs.txt";
    char *TAREFAS[] = {"LIMPEZA", "COZINHA", "ESTUDOS", "LOUÇA", "COMPRAS"};
    int qtdTarefas = 5;
    
    listenfd = Socket(AF_INET, SOCK_STREAM, 0);

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family      = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    int port_arg = atoi(argv[1]);
    servaddr.sin_port        = htons((unsigned int)port_arg);

    if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1) {
        perror("bind");
        exit(1);
    }

    socklen_t len = sizeof(servaddr);
    if (getsockname(listenfd, (struct sockaddr *)&servaddr, &len) == -1) {
        perror("getsockname");
        exit(1);
    }
    
    if (listen(listenfd, LISTENQ) == -1) {
        perror("listen");
        exit(1);
    }
    
    for ( ; ; ) {
      
        if ((connfd = accept(listenfd, (struct sockaddr *) NULL, NULL)) == -1 ) {
            perror("accept");
            exit(1);
        }

        pid_t pid = fork();

        if (pid < 0) {
            perror("fork");
        }
        if (pid == 0) {
            // ChildProcess();
            close(listenfd);

            struct sockaddr_in client_addr;
            socklen_t addrlen = sizeof(client_addr);
            if (getpeername(connfd, (struct sockaddr *)&client_addr, &addrlen) < 0) {
                perror("getpeername");
                exit(1);
            }

            char ip_str_svr[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &(servaddr.sin_addr), ip_str_svr, sizeof(ip_str_svr));

            char ip_str_cli[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &(client_addr.sin_addr), ip_str_cli, sizeof(ip_str_cli));

            FILE *logFile = fopen(arquivo, "a");

            char logLine2[5000];
            sprintf(logLine2, "Cliente conectado IP/porta cliente: %s/%d\n", ip_str_cli, ntohs(client_addr.sin_port));
            fputs(logLine2, logFile);

            fclose(logFile);

            logFile = fopen(arquivo, "a");
            char logLineTarefa[100];
            printf("Enviando tarefa %s\n", TAREFAS[GetRandomNumber(qtdTarefas)]);
            sprintf(logLineTarefa, "Enviando tarefa %s\n", TAREFAS[GetRandomNumber(qtdTarefas)]);

            fputs(logLineTarefa, logFile);

            fclose(logFile);


            snprintf(buf, sizeof(buf), TAREFAS[GetRandomNumber(qtdTarefas)]);
            write(connfd, buf, strlen(buf));


            int n;
            while ( (n = read(connfd, buf, MAXDATASIZE-1)) > 0) {
                buf[n] = 0;
                printf("Cliente finalizado:\n");
                if (fputs(buf, stdout) == EOF) {
                    perror("fputs error");
                    exit(1);
                }


                snprintf(buf, sizeof(buf), TAREFAS[0]);
                write(connfd, buf, strlen(buf));

                logFile = fopen(arquivo, "a");

                sprintf(logLine2, "%s\n", buf);
                fputs(logLine2, logFile);

                sprintf(logLine2, "Cliente finalizado IP/porta cliente: %s/%d\n", ip_str_cli, ntohs(client_addr.sin_port));
                fputs(logLine2, logFile);

                fclose(logFile);
                break;
            }




            // printf("IP/porta servidor: %s/%d\n", ip_str_svr, ntohs(servaddr.sin_port));
            // printf("IP/porta cliente: %s/%d\n", ip_str_cli, ntohs(client_addr.sin_port));
            printf("Fechando filho!\n");

            close(connfd);

            exit(0);
        }
        else {
            //ParentProcess();
            close(connfd);
            printf("Finalizando conexão no pai\n");
            fflush(stdin);
        }
    }
    return(0);
}
