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
#include <sys/wait.h>


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

// Função para lidar com a comunicação de um cliente
void handle_client(int connfd) {
    ssize_t n;
    char line[MAXDATASIZE];

    while ((n = read(connfd, line, MAXDATASIZE - 1)) > 0) {
        line[n] = '\0';  // Termina a string recebida
        printf("Mensagem do cliente: %s\n", line);
        
        // Envia de volta a mensagem recebida para o cliente como confirmação
        write(connfd, line, strlen(line));
    }

    if (n < 0) {
        perror("Erro na leitura do socket");
    }
    printf("Cliente desconectado.\n");
    close(connfd);  // Fecha o socket para este cliente
}

int main(int argc, char **argv) {
    int listenfd, connfd;
    struct sockaddr_in servaddr, clientaddr;

    if (argc != 2) {
        fprintf(stderr, "Uso: %s <Port>\n", argv[0]);
        exit(1);
    }

    listenfd = Socket(AF_INET, SOCK_STREAM, 0);

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    int port_arg = atoi(argv[1]);
    servaddr.sin_port = htons((unsigned int)port_arg);

    Bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr));

    Listen(listenfd, LISTENQ);

    printf("Servidor aguardando conexões...\n");

    for (;;) {
        socklen_t clilen = sizeof(clientaddr);
        connfd = accept(listenfd, (struct sockaddr *)&clientaddr, &clilen);
        if (connfd < 0) {
            perror("Erro ao aceitar conexão");
            continue;
        }

        printf("Cliente conectado.\n");

        pid_t pid = fork();
        if (pid == 0) {
            close(listenfd);
            handle_client(connfd);
            exit(0);
        } else if (pid > 0) { 
            close(connfd); 
        } else {
            perror("Erro no fork");
            close(connfd);
        }

        while (waitpid(-1, NULL, WNOHANG) > 0) {
            
        }
    }
    return 0;
}