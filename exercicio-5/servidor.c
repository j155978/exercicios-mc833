#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

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

void Listen(int sockfd, int backlog){
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
typedef void Sigfunc(int);

Sigfunc * Signal(int signo, Sigfunc *func){
    struct sigaction act, oact;
    act.sa_handler = func;
    sigemptyset (&act.sa_mask);
    act.sa_flags = 0;
    if(signo == SIGALRM){
#ifdef SA_INTERRUPT
        act.sa_flags |= SA_INTERRUPT;
#endif
    } else {
#ifdef SA_RESTART
        act.sa_flags |= SA_RESTART;
#endif
    }
    if (sigaction (signo, &act, &oact) < 0)
        return (SIG_ERR);
    return (oact.sa_handler);
}

void sig_chld(int signo){
    pid_t pid;
    int stat;
    while((pid = waitpid(-1, &stat, WNOHANG)) > 0)
        printf("child %d terminated\n",pid);
    return;
}

int main (int argc, char **argv) {
    // // Configura o manipulador para o sinal SIGCHLD
    // struct sigaction sa;
    // sa.sa_handler = sigchld_handler;  // Define a função de tratamento
    // sigemptyset(&sa.sa_mask);         // Limpa o conjunto de sinais a serem bloqueados
    // sa.sa_flags = SA_RESTART;         // Reinicia chamadas de sistema interrompidas pelo sinal
    // if (sigaction(SIGCHLD, &sa, NULL) == -1) {
    //     perror("sigaction");
    //     exit(1);
    // }

    int listenfd, connfd;
    struct sockaddr_in servaddr;
    // char buf[MAXDATASIZE];
    listenfd = Socket(AF_INET, SOCK_STREAM, 0);

    // Configuração do endereço do servidor
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family      = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    int port_arg = atoi(argv[1]);
    int backlog = atoi(argv[2]);
    servaddr.sin_port        = htons((unsigned int)port_arg);

    Bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr));

    socklen_t len = sizeof(servaddr);
    Getsockname(listenfd, (struct sockaddr *)&servaddr, &len);

    Listen(listenfd, backlog);

    Signal(SIGCHLD, sig_chld);

    char hora[50];
    GetCurrentTime(hora, sizeof(hora));
    FILE *logFile = fopen(arquivo, "a");
    char logLine2[5000];
    sprintf(logLine2, "=========================\n%s: Servidor Aberto. Backlog: %d\n=========================\n", hora, backlog);
    // printf("%s", logLine2);
    fputs(logLine2, logFile);
    fclose(logFile);
    struct sockaddr_in client_addr;
    socklen_t addrlen = sizeof(client_addr);

    // Loop principal para aceitar conexões
    for ( ; ; ) {
        
        if((connfd = accept(listenfd, (struct sockaddr *) &client_addr, &addrlen)) < 0){
            if(errno == EINTR)
                continue;
            else
                perror("accept_errro");
        }

        
        sleep(1);
        
        pid_t pid = fork(); 

        if (pid < 0) {
            perror("fork");
        }
        if (pid == 0) {
            srand(time(NULL));
            close(listenfd); 


            Getpeername(connfd, (struct sockaddr *)&client_addr, &addrlen);

            char ip_str_svr[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &(servaddr.sin_addr), ip_str_svr, sizeof(ip_str_svr));

            char ip_str_cli[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &(client_addr.sin_addr), ip_str_cli, sizeof(ip_str_cli));

            char hora[50];
            GetCurrentTime(hora, sizeof(hora));
            FILE *logFile = fopen(arquivo, "a");
            char logLine2[5000];
            sprintf(logLine2, "%s : Cliente conectado IP/porta cliente: %s/%d\n", hora, ip_str_cli, ntohs(client_addr.sin_port));
            printf("%s", logLine2);
            fputs(logLine2, logFile);
            fclose(logFile);

            // snprintf(buf, sizeof(buf), "Fim\n");
            // write(connfd, buf, strlen(buf));

            close(connfd);

            logFile = fopen(arquivo, "a");
            sprintf(logLine2, "%s : Cliente finalizado IP/porta cliente: %s/%d\n", hora, ip_str_cli, ntohs(client_addr.sin_port));
            printf("%s", logLine2);
            fputs(logLine2, logFile);
            fclose(logFile);
            
            exit(0);
        } 
        else {
            close(connfd);
            fflush(stdin);
        }
    }
    return(0);
}
