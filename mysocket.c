#include "mysocket.h"
#include <pthread.h>
pthread_mutex_t mutex_ptr_Received_Message, mutex_ptr_Send_Message;

table *Send_Message = NULL;
table *Received_Message = NULL;
void init_Send_Message(table *u, int s)
{
    u->next_to_use = 0;
    u->size = s;
    u->arr = (message *)malloc(s * sizeof(message));
}
int add_to_Send_Message(table *u, message *msg)
{
    pthread_mutex_lock(&mutex_ptr_Send_Message);
    if (u->next_to_use == u->size)
    {
        pthread_mutex_unlock(&mutex_ptr_Send_Message);
        return 0;
    }
    u->arr[u->next_to_use] = *msg;
    u->next_to_use++;
    pthread_mutex_unlock(&mutex_ptr_Send_Message);
    return 1;
};
void remove_from_Send_Message(table *u, int indx)
{
    pthread_mutex_lock(&mutex_ptr_Send_Message);

    for (int j = indx; j + 1 < u->next_to_use; j++)
    {
        u->arr[j] = u->arr[j + 1];
    }
    u->next_to_use--;
    pthread_mutex_unlock(&mutex_ptr_Send_Message);
};
void init_Received_Message(table *r, int s)
{
    r->next_to_use = 0;
    r->size = s;
    r->arr = (message *)malloc(s * sizeof(message));
}
int add_to_Received_Message(table *r, message *msg)
{
    pthread_mutex_lock(&mutex_ptr_Received_Message);
    if (r->next_to_use == r->size)
    {
        pthread_mutex_unlock(&mutex_ptr_Received_Message);
        return 0;
    }
    r->arr[r->next_to_use] = *msg;
    r->next_to_use++;
    pthread_mutex_unlock(&mutex_ptr_Received_Message);
    return 1;
}
void remove_from_Received_Message(table *r, int indx)
{
    pthread_mutex_lock(&mutex_ptr_Received_Message);

    for (int j = indx; j + 1 < r->next_to_use; j++)
    {
        r->arr[j] = r->arr[j + 1];
    }
    r->next_to_use--;
    pthread_mutex_unlock(&mutex_ptr_Received_Message);
}
int my_socket(int domain, int type, int protocol)
{
    if (type != SOCK_MyTCP)
    {
        return -1;
    }

    Send_Message = (table *)malloc(1 * sizeof(table));
    Received_Message = (table *)malloc(1 * sizeof(table));
    init_Send_Message(Send_Message, 10);
    init_Received_Message(Received_Message, 10);

    pthread_mutexattr_t attr_;
    pthread_mutexattr_init(&attr_);
    pthread_mutex_init(&mutex_ptr_Received_Message, &attr_);
    pthread_mutex_init(&mutex_ptr_Send_Message, &attr_);

    int my_sock = socket(domain, SOCK_STREAM, protocol);

    // pthread_attr_t attr;
    // pthread_attr_init(&attr);
    // pthread_create(&R, &attr, run_thread_r, (void *)&my_sock);
    // pthread_create(&S, &attr, run_thread_s, (void *)&my_sock);
    // printf("my_socket %d\n", my_sock);
    return my_sock;
}
int my_bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
    // printf("my_bind of\n");
    return bind(sockfd, addr, addrlen);
}
int my_accept(int sockfd, struct sockaddr *addr, socklen_t *addr_len)
{
    int ret = accept(sockfd, addr, addr_len);
    // printf("my_accept %d \n", ret);

    return ret;
}
int my_listen(int sockfd, int n)
{
    // printf("my_listen of\n");
    return listen(sockfd, n);
}
int my_connect(int sockfd, const struct sockaddr *addr, socklen_t len)
{
    int ret = connect(sockfd, addr, len);
    // printf("my_connect %d\n", ret);
    return ret;
}

void func(){
    // printf("%s\n", char_buf);
    char *buff_hdr = (char *)malloc(5 * sizeof(char)); // msg_type(1 char)+ msg(4 char) + null char(1 char)
    memset(buff_hdr, '\0', 5);
    // buff_hdr[0] = '0';
    // strcpy(buff_hdr + 1, itoa(len_));
    sprintf(buff_hdr, "%d", len_);
    // printf("...%s\n",buff_hdr);
    int ret1 = send(sockfd, buff_hdr, 5, 0);
    if (ret1 == -1)
    {
        // printf("error 1....\n");
        return -1;
    }
    int j = 0, i, flag_end = 0;
    char *buff_data = (char *)malloc(transfer_max_size * sizeof(char));
    do
    {
        memset(buff_data, '\0', transfer_max_size);
        // buff_data[0] = '1';
        for (i = 0; i < transfer_max_size; i++)
        {
            if (j >= len)
            {
                flag_end = 1;
                break;
            }
            buff_data[i] = char_buf[j];
            j++;
        }
        // printf("\n...%d...\n",j);
        int ret2 = send(sockfd, buff_data, i, flags);
        // printf("%s\n", buff_data);
        if (ret2 == -1)
        {
            // printf("error 2....\n");
            return -1;
        }
    } while (flag_end != 1);
    // printf("j==%d\n", j);
    return j;
    // return send(sockfd, buf, len, flags);
}

ssize_t my_send(int sockfd, const void *buf, size_t len, int flags)
{
    
    int len_ = (int)len;
    // char *char_buf = (char *)buf;
    char *char_buf = (char *)malloc(msg_max_size * sizeof(char));
    int k;
    for (k = 0; k < len; k++)
    {
        char_buf[k] = ((char *)buf)[k];
    }

    message* msg = (message*)malloc(1*sizeof(message));
    msg->flags=flags;
    msg->len=len;
    strcpy(msg->msg_body,char_buf);
    pthread_mutex_unlock(&mutex_ptr_Send_Message);
    while(Send_Message->size==10){
        //
    }
    add_to_Send_Message(Send_Message, msg);
    pthread_mutex_unlock(&mutex_ptr_Send_Message);

    
}
int find_msg_size(char *buf)
{
    for (int i = 0; i < msg_max_size; i++)
    {
        if (buf[i] == '\0')
        {
            return i;
        }
    }
}
ssize_t my_recv(int sockfd, void *buf, size_t len, int flags)
{
    int msg_size;
    char *buff_hdr = (char *)malloc(5 * sizeof(char));
    memset(buff_hdr, '\0', 5);
    int ret1 = recv(sockfd, buff_hdr, 5, MSG_WAITALL);
    if (ret1 == -1)
    {
        // printf("error 3...\n");
        return -1;
    }
    msg_size = atoi(buff_hdr);
    // printf("+++%s\n", buff_hdr);
    // printf("///%d\n", msg_size);
    char *char_buf = (char *)malloc(msg_max_size * sizeof(char));
    memset(char_buf, '\0', msg_max_size);
    int curr_size = 0;
    // strcat(char_buf, "12345");

    // printf("000 %d\n",curr_size);
    int flag_end=0;
    // do
    // {
        // char *chunk1 = (char *)malloc(transfer_max_size * sizeof(char));
        // memset(chunk1, '\0', transfer_max_size);

        char *chunk2 = (char *)malloc(100 * sizeof(char));
        do{
            memset(chunk2, '\0', 100);
            int ret = recv(sockfd, chunk2, 100, flags);
            if (ret == -1)
            {
                return -1;
            }
            // strcat(chunk1, chunk2);
            strcat(char_buf, chunk2);
            curr_size = find_msg_size(char_buf);
            if(curr_size >= msg_size){
                flag_end=1;
            }
        }while(flag_end!=1);

        // recv(sockfd, chunk1, 1000, MSG_WAITALL);
        // strcat(char_buf, chunk1);
        
    // } while (flag_end!=1);
    int i;
    for (i = 0; i <= curr_size; i++)
    {
        ((char *)buf)[i] = char_buf[i];
    }
    return curr_size;
    // return recv(sockfd, buf, len, flags);
}
int rclose(int sockfd)
{
    // pthread_kill(R, SIGINT);
    // pthread_kill(S, SIGINT);
    free(Send_Message);
    free(Received_Message);
    int ret = close(sockfd);
    // printf("my_close %d\n", ret);
    return ret;
}
void *run_thread_r(void *param)
{
}
void *run_thread_s(void *param)
{
    


}
