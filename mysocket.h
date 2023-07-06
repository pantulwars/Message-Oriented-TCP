#ifndef MySOCKET_H
#define MySOCKET_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <netinet/in.h>
#define SOCK_MyTCP 301
#define header_msg 0
#define data_msg 1
#define msg_max_size 5000
#define transfer_max_size 1000
// pthread_t R, S;
typedef struct
{
    char msg_body[5000];
    ssize_t len;
    int flags;

} message;
typedef struct
{
    int size;
    int next_to_use;
    message *arr;
} table;

void init_Send_Message(table *Send_Message, int s);
int add_to_Send_Message(table *Send_Message, message *msg);
void remove_from_Send_Message(table *Send_Message, int indx);
void init_Received_Message(table *Received_Message, int s);
int add_to_eceived_Message(table *Received_Message, message *msg);
void remove_from_Received_Message(table *Received_Message, int indx);
void *run_thread_r(void *param);
void *run_thread_s(void *param);

int my_socket(int domain, int type, int protocol);
int my_bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
int my_accept(int sockfd, struct sockaddr *addr, socklen_t *addr_len);
int my_listen(int sockfd, int n);
int my_connect(int sockfd, const struct sockaddr *addr, socklen_t len);
ssize_t my_send(int sockfd, const void *buf, size_t len, int flags);
ssize_t my_recv(int sockfd, void *buf, size_t len, int flags);
int my_close(int sockfd);

#endif