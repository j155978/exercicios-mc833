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

int Socket(int family, int type, int flags);
void Getpeername(int connfd, struct sockaddr *__restrict__ client_addr, socklen_t *__restrict__ addrlen);
void Bind(int listenfd, const struct sockaddr *servaddr, socklen_t __len);
void Getsockname(int listenfd, struct sockaddr *__restrict__ servaddr, socklen_t *__restrict__ len);
int Accept(int listenfd, struct sockaddr *__restrict__ __addr, socklen_t *__restrict__ __addr_len);
void Listen(int sockfd, int backlog);
void Setsockopt(int socket, int level, int option_name, const void *option_value, socklen_t option_len);
void Sendto(int socket, const void *message, size_t length, int flags, const struct sockaddr *dest_addr, socklen_t dest_len);
int Recvfrom(int socket, void *restrict buffer, size_t length, int flags, struct sockaddr *restrict address, socklen_t *restrict address_len);
void Close(int fildes);
int max(int x1, int x2);
pid_t Fork();
void str_echo(int sockfd);
void Connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
ssize_t Writen(int fd, const void *vptr, size_t n);

typedef void Sigfunc(int);
Sigfunc *Signal(int signo, Sigfunc *func);

#define MAXLINE 10000

void dg_cli(FILE *fp, int sockfd, const struct sockaddr *pservaddr, socklen_t servlen){
    int n;
    char sendline[MAXLINE], recvline[MAXLINE + 1];

    Connect(sockfd, (struct sockaddr *) pservaddr, servlen);

    printf("dglceieeei!");

    while (fgets(sendline, MAXLINE, fp) != NULL) {

        if(strcmp(sendline, "") == 0){
            break;
        }

        write(sockfd, sendline, strlen(sendline));

        n = read(sockfd, recvline, MAXLINE);

        recvline[n] = 0;
        fputs(recvline, stdout);
    }
}

void listener(FILE *fp, int sockfd, const struct sockaddr *pservaddr, socklen_t servlen){

    printf("Ouvindo!");

    int n;
    char recvline[MAXLINE + 1];

    Connect(sockfd, (struct sockaddr *) pservaddr, servlen);

    n = read(sockfd, recvline, MAXLINE);

    if(n != 0){
        recvline[n] = 0;
        fputs(recvline, stdout);
    }

}

int main(int argc, char **argv){
    int sockfd;
    struct sockaddr_in servaddr;

    if(argc < 3)
        perror("Usage: cliente <IP>");

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons((unsigned int)atoi(argv[2]));
    inet_pton(AF_INET, argv[1], &servaddr.sin_addr);

    sockfd = Socket(AF_INET, SOCK_DGRAM, 0);

    dg_cli(stdin, sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr));

    listener(stdin, sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr));

    exit(0);
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

void Setsockopt(int socket, int level, int option_name, const void *option_value, socklen_t option_len) {
    if (setsockopt(socket, level, option_name, option_value, option_len) == -1) {
        perror("setsockopt");
        exit(1);
    }
}

void Sendto(int socket, const void *message, size_t length, int flags, const struct sockaddr *dest_addr, socklen_t dest_len) {
    if (sendto(socket, message, length, flags, dest_addr, dest_len) == -1) {
        perror("sendto");
        exit(1);
    }
}

void Connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
    if (connect(sockfd, addr, addrlen) < 0) {
        perror("connect error");
        exit(1);
    }    
}

int Recvfrom(int socket, void *restrict buffer, size_t length, int flags, struct sockaddr *restrict address, socklen_t *restrict address_len) {
    int messagelen;
    if ((messagelen = recvfrom(socket, buffer, length, flags, address, address_len)) == -1) {
        perror("recvfrom");
        exit(1);
    }
    return messagelen;
}

void Close(int fildes) {
    if (close(fildes) == -1) {
        perror("close");
        exit(1);        
    }
}

int max(int x1, int x2) {
    return x1 > x2 ? x1 : x2;
}

pid_t Fork() {
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        exit(1);
    }

    return pid;
}

void str_echo(int sockfd) {
    ssize_t n;
    char buf[MAXLINE];

    again:
        while ( (n = read(sockfd, buf, MAXLINE)) > 0)
            Writen(sockfd, buf, n);
    if (n < 0 && errno == EINTR) {
        goto again;
    } else if (n < 0) {
        perror("str_echo: read error");
        exit(1);
    }
        
}

ssize_t Writen(int fd, const void *vptr, size_t n) {
    size_t nleft;
    ssize_t nwritten;
    const char *ptr;

    ptr = vptr;
    nleft = n;

    while (nleft > 0) {
        if ((nwritten = write(fd, ptr, nleft)) <= 0) {
            if (nwritten < 0 && errno == EINTR) {
                nwritten = 0; /* and call write() again */
            } else {
                return (-1); /* error */
            }
        }
        nleft -= nwritten;
        ptr += nwritten;
    }
    return (n);
}

Sigfunc *Signal(int signo, Sigfunc *func) {
    struct sigaction act, oact;

    // Configura a estrutura sigaction para o novo manipulador de sinal
    act.sa_handler = func;
    sigemptyset(&act.sa_mask);  // Limpa o conjunto de sinais bloqueados
    act.sa_flags = 0;

    // Configura flags específicas para SIGALRM e outros sinais
    if (signo == SIGALRM) {
        #ifdef SA_INTERRUPT
            act.sa_flags |= SA_INTERRUPT;  // SunOS 4.x: Reinicia chamadas interrompidas
        #endif
    } else {
        #ifdef SA_RESTART
            act.sa_flags |= SA_RESTART;  // SVR4, 4.4BSD: Reinicia chamadas interrompidas
        #endif
    }

    // Registra o novo manipulador de sinal
    if (sigaction(signo, &act, &oact) < 0)
        return (SIG_ERR);

    // Retorna o antigo manipulador de sinal
    return (oact.sa_handler);
} 