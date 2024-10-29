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

// Função que cria um socket e verifica erros
int Socket(int family, int type, int flags) {
    int sockfd;
    if ((sockfd = socket(family, type, flags)) < 0) {
        perror("socket");
        exit(1);
    } else
        return sockfd;
}

// Envelopamento função Getsockname
void Getsockname(int listenfd, struct sockaddr *__restrict__ servaddr, socklen_t *__restrict__ len) {
    if (getsockname(listenfd, servaddr, len) == -1) {
        perror("getsockname");
        exit(1);
    }
}

// Envelopamento da função inet_pton
void Inet_pton(int family, const char *restrict src, void *restrict dst) {
    if (inet_pton(AF_INET, src, dst) <= 0) {
        perror("inet_pton error");
        exit(1);
    }
}

// Envelopamento da função connect
void Connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
    if (connect(sockfd, addr, addrlen) < 0) {
        perror("connect error");
        exit(1);
    }    
}

int main(int argc, char **argv) {
    int sockfd;
    char sendline[MAXLINE];
    char recvline[MAXLINE];
    struct sockaddr_in servaddr;

    if (argc != 3) {
        fprintf(stderr, "uso: %s <IPaddress> <Port>\n", argv[0]);
        exit(1);
    }

    sockfd = Socket(AF_INET, SOCK_STREAM, 0);

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(atoi(argv[2]));

    if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0) {
        perror("inet_pton error");
        exit(1);
    }

    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("connect error");
        exit(1);
    }

    printf("Conexão estabelecida! Digite mensagens para enviar ao servidor.\n");
    printf("Digite 'exit' para encerrar.\n");

    // Loop interativo para enviar mensagens ao servidor
    while (1) {
        printf("Mensagem: ");
        if (fgets(sendline, MAXLINE, stdin) == NULL) {
            printf("Erro na leitura da mensagem.\n");
            continue;
        }

        // Verifica se o usuário quer encerrar a conexão
        if (strncmp(sendline, "exit", 4) == 0) {
            printf("Encerrando a conexão.\n");
            break;
        }

        // Envia a mensagem para o servidor
        if (write(sockfd, sendline, strlen(sendline)) < 0) {
            perror("Erro ao enviar mensagem");
            break;
        }

        // Lê a resposta do servidor
        ssize_t n = read(sockfd, recvline, MAXLINE);
        if (n > 0) {
            recvline[n] = '\0';  // Garantir que o texto está terminado em null
            printf("Servidor: %s\n", recvline);
        } else if (n < 0) {
            perror("Erro na leitura da resposta");
            break;
        }
    }

    close(sockfd);
    return 0;
}