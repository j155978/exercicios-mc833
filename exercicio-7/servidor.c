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
#define MAXLINE 500

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
ssize_t Writen(int fd, const void *vptr, size_t n);
void sig_chld(int signo);
void remove_newlines(char *str);

typedef void Sigfunc(int);
Sigfunc *Signal(int signo, Sigfunc *func);

int client_count = 0;


struct cliente {
    struct sockaddr_in endereco;  // Endereço do cliente
    char nome[10000];               // Nome do cliente
    pid_t pid;
};

struct cliente clients[100];  // Armazena endereços dos clientes
    
pid_t pidAtual;

int udpfd;

int main(int argc, char **argv) {
    int listenfd, connfd, nready, maxfdp1;
    char mesg[MAXLINE];
    pid_t childpid;
    fd_set rset;
    ssize_t n;
    socklen_t len;
    const int on = 1;
    struct sockaddr_in cliaddr, servaddr;
    void sig_chld(int);
    int port_arg = atoi(argv[1]);



    /* create listening TCP socket */
    listenfd = Socket(AF_INET, SOCK_STREAM, 0);
    bzero(&servaddr, sizeof(servaddr));
    
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons((unsigned int)port_arg);

    Setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    Bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
    Listen(listenfd, LISTENQ);

    
    /* create UDP socket */
    udpfd = Socket(AF_INET, SOCK_DGRAM, 0);
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons((unsigned int)port_arg);
    Bind(udpfd, (struct sockaddr *)&servaddr, sizeof(servaddr)); 

    Signal(SIGCHLD, sig_chld); /* must call waitpid() */

    FD_ZERO(&rset);
    maxfdp1 = max(listenfd, udpfd) + 1;
    for ( ; ; ) {
        FD_SET(listenfd, &rset);
        FD_SET(udpfd, &rset);
        if ( (nready = select(maxfdp1, &rset, NULL, NULL, NULL)) < 0) {
            if (errno == EINTR) {
                continue; /* back to for() */
            }
            else {
                perror("select error");
                exit(1);
            }
        }
        
        if (FD_ISSET(listenfd, &rset)) {
            len = sizeof(cliaddr);
            connfd = Accept(listenfd, (struct sockaddr *) &cliaddr, &len);

            if ((childpid = Fork()) == 0) {  // Processo filho
                Close(listenfd);  // Fecha o socket de escuta no filho
                str_echo(connfd);  // Processa a requisição do cliente
                exit(0);
            } else{
                pidAtual = childpid;
            }
            Close(connfd); /* parent closes connected socket */
        }
        if (FD_ISSET(udpfd, &rset)) { 
            len = sizeof(cliaddr);
            n = Recvfrom(udpfd, mesg, MAXLINE, 0, (struct sockaddr *) &cliaddr, &len);

            char msgNova[1000];

            n = snprintf(msgNova, sizeof(msgNova), "---Novo usuário conectado! %s",mesg);


            char msg_lista_nomes[5000] = "---Lista de usuários atualmente conectados: \n";

            if(client_count == 0){
                msg_lista_nomes[0] = '\0';
            }

            for (int i = 0; i < client_count; i++) {
                // Concatena os nomes dos clientes na string
   
                strncat(msg_lista_nomes, "->", sizeof(msg_lista_nomes) - strlen(msg_lista_nomes) - 1);
                
                strncat(msg_lista_nomes, clients[i].nome, sizeof(msg_lista_nomes) - strlen(msg_lista_nomes) - 1);

                strncat(msg_lista_nomes, "\n", sizeof(msg_lista_nomes) - strlen(msg_lista_nomes) - 1);
            }

            Sendto(udpfd, msg_lista_nomes, strlen(msg_lista_nomes), 0, (struct sockaddr *)&cliaddr, sizeof(cliaddr));


            int in = 0;
            for (int i = 0; i < client_count; i++) {
                if (clients[i].endereco.sin_port == cliaddr.sin_port &&
                    clients[i].endereco.sin_addr.s_addr == cliaddr.sin_addr.s_addr) {
                    in = 1; 
                    break;
                }
            }

            for(int i = 0 ; i < client_count ; i++){
                Sendto(udpfd, msgNova, n, 0, (struct sockaddr *) &clients[i].endereco, sizeof(clients[i].endereco));
            }

            if (in == 0) {
                
                remove_newlines(mesg);

                strncpy(clients[client_count].nome, mesg, sizeof(mesg)-1);
                clients[client_count].endereco = cliaddr;
                clients[client_count].pid = pidAtual;
                client_count++;
            }
            printf("Clientes conectados = %d\n", client_count);
        }
    }           
}

void sig_chld(int signo) {
    pid_t pid;
    int stat;

    char msg[20000];

    while ((pid = waitpid(-1, &stat, WNOHANG)) > 0) {

        for (int i = 0; i < client_count; i++) {
            if (clients[i].pid == pid) {

                snprintf(msg, sizeof(msg), "---Cliente %s desconectado.\n", clients[i].nome);

                for (int j = i; j < client_count - 1; j++) {
                    clients[j] = clients[j + 1];
                }
                client_count--;
                break;
            }
        }
    }

    for (int j = 0; j < client_count; j++) {
        Sendto(udpfd, msg, strlen(msg), 0, (struct sockaddr *)&clients[j].endereco, sizeof(clients[j].endereco));
    }

    printf("Clientes conectados = %d\n", client_count);

    return;
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

void remove_newlines(char *str) {
    int i, j = 0;
    int length = strlen(str);
    
    for (i = 0; i < length; i++) {
        if (str[i] != '\n') {
            str[j++] = str[i];
        }
    }
    str[j] = '\0';
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
    while ((n = read(sockfd, buf, MAXLINE)) > 0) {
        Writen(sockfd, buf, n);
    }

    if (n < 0 && errno == EINTR) {
        goto again;
    } else if (n < 0) {
        perror("str_echo: erro na leitura");
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

