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
    // added clientaddr for exercicio 6
    struct sockaddr_in servaddr, clientaddr;
    char   buf[MAXDATASIZE];

    time_t ticks;


    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family      = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    // servaddr.sin_port        = htons(4950);   

    if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1) {
        perror("bind");
        exit(1);
    }

    // Exercício 4
    socklen_t len = sizeof(servaddr);
    if (getsockname(listenfd, (struct sockaddr *)&servaddr, &len) == -1) {
        perror("getsockname");
        exit(1);
    }

    unsigned int port;
    port = ntohs(servaddr.sin_port);
    printf("Port number: %d\n", port);
    // end of changes


    if (listen(listenfd, LISTENQ) == -1) {
        perror("listen");
        exit(1);
    }

    for ( ; ; ) {
      if ((connfd = accept(listenfd, (struct sockaddr *) NULL, NULL)) == -1 ) {
        perror("accept");
        exit(1);
        }

        //Exercício 6

        len = sizeof(clientaddr);
        if (getpeername(connfd, (struct sockaddr *)&clientaddr, &len) == -1){
            perror("getpeername");
            exit(1);
        }

        char ipCliente[INET_ADDRSTRLEN];

        inet_ntop(AF_INET, &(clientaddr.sin_addr), ipCliente, sizeof(ipCliente));

        unsigned int portaCliente = ntohs(clientaddr.sin_port);

        printf("Cliente conectado: \n");
        printf("IP: %s\n", ipCliente);
        printf("Porta: %d\n\n", portaCliente);

        fflush(stdin);

        //End of exercicio 6

        //Exercicio 7 - impressão da mensagem enviada pelo cliente

        int n;
        while ( (n = read(connfd, buf, MAXDATASIZE-1)) > 0) {
            buf[n] = 0;
            printf("Mensagem recebida do cliente:\n");
            if (fputs(buf, stdout) == EOF) {
                perror("fputs error");
                exit(1);
            }
            break;

        // end of exercicio 7
        }

        ticks = time(NULL);
        snprintf(buf, sizeof(buf), "Hello from server!\nTime: %.24s\r\n", ctime(&ticks));
        write(connfd, buf, strlen(buf));


        close(connfd);
    }
    return(0);
}
