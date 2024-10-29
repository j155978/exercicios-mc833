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


#define max(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })

void cliente(FILE *fp, int sockfd){
    int maxfdp1, stdineof;
    fd_set rset;
    char sendline[MAXLINE], recvline[MAXLINE];

    stdineof = 0;

    FD_ZERO(&rset);
    for ( ; ; ){
        if (stdineof == 0)
            FD_SET(fileno(fp), &rset);
        
        FD_SET(sockfd, &rset);
        maxfdp1 = max(fileno(fp), sockfd) + 1;
        if (select(maxfdp1, &rset, NULL, NULL, NULL) < 0) {
            perror("select error");
            break;
        }

        if(FD_ISSET(sockfd, &rset)){ //Socket is readable
            int qtd = read(sockfd, recvline, MAXLINE);
            
            if(qtd == 0){
                if(stdineof == 1)
                    return; //normal termination
                else
                    perror("str_cli: server terminated prematurely");
            }
            recvline[qtd] = '\0';
            fputs(recvline, stdout);
            char arquivolog[] = "resultscliente.txt";
            FILE *logFile = fopen(arquivolog, "a");
            fputs(recvline, logFile);
            fclose(logFile);
        }

        if(FD_ISSET(fileno(fp), &rset)){
            while(fgets(sendline, MAXLINE, fp) != NULL){
                write(sockfd, sendline, strlen(sendline));
            }
            stdineof = 1;
            shutdown(sockfd, SHUT_WR); //send FIN
            FD_CLR(fileno(fp), &rset);
            continue;
        } //input is readable
    }
}


int main(int argc, char **argv) {
    int sockfd[2]; // Array para dois sockets
    struct sockaddr_in servaddr[2];

    if (argc != 5) {
        fprintf(stderr, "uso: %s <IPaddress1> <Port1> <IPaddress2> <Port2>\n", argv[0]);
        exit(1);
    }

    // Criar e conectar os sockets para os dois servidores
    for (int i = 0; i < 2; i++) {
        sockfd[i] = Socket(AF_INET, SOCK_STREAM, 0);

        memset(&servaddr[i], 0, sizeof(servaddr[i]));
        servaddr[i].sin_family = AF_INET;
        servaddr[i].sin_port = htons(atoi(argv[i * 2 + 2]));

        Inet_pton(AF_INET, argv[i * 2 + 1], &servaddr[i].sin_addr);
        Connect(sockfd[i], (struct sockaddr *)&servaddr[i], sizeof(servaddr[i]));
    }

    printf("Conexões estabelecidas! Você pode enviar mensagens para os dois servidores.\n");
    printf("Digite 'exit' para encerrar.\n");


    char arquivo[] = "dados.txt";
    

    // Executa a função cliente para cada socket
    for (int i = 0; i < 2; i++) {
        printf("Conectando ao servidor %d...\n", i + 1);
        FILE *logFile = fopen(arquivo, "r");
        cliente(logFile, sockfd[i]);
        fclose(logFile);
    }

    // Fechar sockets
    for (int i = 0; i < 2; i++) {
        close(sockfd[i]);
    }

    return 0;
}