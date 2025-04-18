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

int main(int argc, char **argv) {
    int    sockfd, n;
    char   recvline[MAXLINE + 1];
    char   error[MAXLINE + 1];
    struct sockaddr_in servaddr;

    // Verifica se os parâmetros de IP e porta foram passados corretamente
    if (argc != 3) {
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
    servaddr.sin_port   = htons((unsigned int)port_arg);

    // Converte o IP fornecido para o formato binário
    if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0) {
        perror("inet_pton error");
        exit(1);
    }

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
    printf("Informações do servidor:\n");
    printf("IP: %s\n", ip_str);
    printf("Porta: %d\n\n\n", ntohs(servaddr.sin_port));

    inet_ntop(AF_INET, &(local_addr.sin_addr), ip_str, sizeof(ip_str));
    printf("Informações do socket local:\n");
    printf("IP: %s\n", ip_str);
    int porta = ntohs(local_addr.sin_port);
    printf("Porta: %d\n\n", porta);

    // Loop de leitura de mensagens do servidor
    for(;;) {
        while ((n = read(sockfd, recvline, MAXLINE)) > 0) {
            recvline[n] = 0;
            break;
        }

        char mensagem[5000];

        // Se o comando "ENCERRAR" for recebido, o cliente encerra
        if (strcmp(recvline, "ENCERRAR") == 0) {
            printf("Recebido comando 'ENCERRAR', finalizando cliente\n");
            fflush(stdin);
            break;
        }

        // Caso contrário, exibe a tarefa recebida e responde ao servidor
        if (strcmp(recvline, "") != 0) {
            sprintf(mensagem, "Tarefa recebida: %s\n", recvline);
            printf("%s", mensagem);

            fflush(stdin);
            sleep(3);  // Simula a execução de uma tarefa

            sprintf(mensagem, "TAREFA_%s CONCLUIDA por %s / %d\n", recvline, ip_str, porta);
            write(sockfd, mensagem, strlen(mensagem));
        }

        recvline[0] = '\0'; // Limpa a mensagem para a próxima leitura
    }

    exit(0);
}
