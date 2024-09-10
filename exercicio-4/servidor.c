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

int main (int argc, char **argv) {
    int    listenfd, connfd;
    struct sockaddr_in servaddr;
    char   buf[MAXDATASIZE];

    int teste = 0;

    char *TAREFAS[] = {"LIMPEZA", "COZINHA", "ESTUDOS", "LOUÇA", "COMPRAS"};
    
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

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
            snprintf(buf, sizeof(buf), TAREFAS[teste]);
            write(connfd, buf, strlen(buf));


            int n;
            while ( (n = read(connfd, buf, MAXDATASIZE-1)) > 0) {
                buf[n] = 0;
                printf("Mensagem recebida do cliente:\n");
                if (fputs(buf, stdout) == EOF) {
                    perror("fputs error");
                    exit(1);
                }
                break;
            }


            close(connfd);

            printf("Fechando filho!\n");
            exit(0);
        }
        else {
            //ParentProcess();
            close(connfd);
            printf("Finalizando conexão no pai\n");
            fflush(stdin);
            teste += 1;
        }
    }
    return(0);
}
