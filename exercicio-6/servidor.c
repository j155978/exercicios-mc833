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
#include <sys/time.h>


#define LISTENQ 10
#define MAXDATASIZE 500

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

// Envelopamento função Listen
void Listen(int sockfd, int backlog) {
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

int main (int argc, char **argv) {
    int    listenfd, connfd;
    struct sockaddr_in servaddr, clientaddr;
    char   buf[MAXDATASIZE];

    time_t ticks;

    listenfd = Socket(AF_INET, SOCK_STREAM, 0);

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family      = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    int port_arg = atoi(argv[1]);
    servaddr.sin_port        = htons((unsigned int)port_arg);

    Bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr));

    socklen_t len = sizeof(servaddr);
    Getsockname(listenfd, (struct sockaddr *)&servaddr, &len);

    Listen(listenfd, LISTENQ);

    for ( ; ; ) {
        socklen_t clilen = sizeof(clientaddr);
        connfd = Accept(listenfd, (struct sockaddr *) &clientaddr, &clilen);

        len = sizeof(clientaddr);
        Getpeername(connfd, (struct sockaddr *)&clientaddr, &len);

        char ipCliente[INET_ADDRSTRLEN];

        inet_ntop(AF_INET, &(clientaddr.sin_addr), ipCliente, sizeof(ipCliente));

        unsigned int portaCliente = ntohs(clientaddr.sin_port);
        
        int cpu = GetRandomNumber(100);
        int memory = GetRandomNumber(100);
        // char hora[50];
        // GetCurrentTime(hora, sizeof(hora));

        ticks = time(NULL);
        sprintf(buf, "IP: %s\nPorta: %d\nHorário: %sCPU: %d%%\nMemória: %d%%\nStatus: Ativo\n",
            ipCliente,
            portaCliente,
            ctime(&ticks),
            cpu,
            memory
        );
        printf(buf);

        fflush(stdin);

        int n;
        while ( (n = read(connfd, buf, MAXDATASIZE-1)) > 0) {
            buf[n] = 0;
            printf("Mensagem recebida do cliente:\n");
            if (fputs(buf, stdout) == EOF) {
                perror("fputs error");
                exit(1);
            }
            fflush(stdin);
        }

        ticks = time(NULL);
        snprintf(buf, sizeof(buf), "Hello from server!\nTime: %.24s\r\n", ctime(&ticks));
        write(connfd, buf, strlen(buf));


        close(connfd);
    }
    return(0);
}
