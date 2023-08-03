#include "mysocket.h"
#include <pthread.h>
#include <errno.h>
#include <signal.h>

pthread_mutex_t mutex_ptr_Received_Message, mutex_ptr_Send_Message, mutex_send_ret, mutex_recv_ret, mutex_call_recv;
pthread_cond_t cond_r, cond_s, cond_ret_s, cond_ret_r, cond_recv;
pthread_t r_tid, s_tid;

int _sockfd, _recv_sockfd = -1;
int send_ret, recv_ret = -2;
size_t recv_len, send_len;

int call_recv = 0, recv_flags = 0;

table *Send_Message = NULL;
table *Received_Message = NULL;
void init_Send_Message(table *u, int s)
{
    u->next_to_use = 0;
    u->size = s;
    u->arr = (message *)malloc(s * sizeof(message));
}
void remove_from_Send_Message(table *u, int indx)
{
    for (int j = indx; j+1 <= u->next_to_use; j++)
    {
        u->arr[j] = u->arr[j + 1];
    }
    u->next_to_use--;
};
void init_Received_Message(table *r, int s)
{
    r->next_to_use = 0;
    r->size = s;
    r->arr = (message *)malloc(s * sizeof(message));
}
void remove_from_Received_Message(table *r, int indx)
{
    pthread_mutex_lock(&mutex_ptr_Received_Message);

    for (int j = indx; j + 1 <= r->next_to_use; j++)
    {
        r->arr[j] = r->arr[j + 1];
    }
    r->next_to_use--;
    pthread_mutex_unlock(&mutex_ptr_Received_Message);
}

int my_socket(int domain, int type, int protocol)
{
    printf("Created socket!\n");
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
    pthread_mutex_init(&mutex_call_recv, &attr_);

    pthread_mutex_init(&mutex_send_ret, &attr_);
    pthread_mutex_init(&mutex_recv_ret, &attr_);

    int my_sock = socket(domain, SOCK_STREAM, protocol);

    pthread_attr_t attr;

    pthread_attr_init(&attr);

    pthread_cond_init(&cond_r, NULL);
    pthread_cond_init(&cond_s, NULL);
    pthread_cond_init(&cond_ret_s, NULL);
    pthread_cond_init(&cond_ret_r, NULL);
    pthread_cond_init(&cond_recv, NULL);

    pthread_create(&r_tid, &attr, run_thread_r, Received_Message);
    // printf("Running Recieve Thread3!!");
    sleep(2);
    pthread_create(&s_tid, &attr, run_thread_s, Send_Message);
    return my_sock;
}
int my_bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
    return bind(sockfd, addr, addrlen);
}
int my_accept(int sockfd, struct sockaddr *addr, socklen_t *addr_len)
{
    return accept(sockfd, addr, addr_len);
}

int my_listen(int sockfd, int n)
{
    return listen(sockfd, n);
}
int my_connect(int sockfd, const struct sockaddr *addr, socklen_t len)
{
    return connect(sockfd, addr, len);
}

int handle_send(char *char_buf, int sockfd, size_t len, int flags)
{
    char *buff_hdr = (char *)malloc(5 * sizeof(char));
    memset(buff_hdr, '\0', 5);
    sprintf(buff_hdr, "%ld", len);

    printf("Handling send %s!\n", char_buf);

    int ret1 = send(sockfd, buff_hdr, 5, 0);
    if (ret1 == -1)
    {
        printf("S: Error sending message: %s\n", strerror(errno));
        return -1;
    }
    // printf("Sending the message body\n");
    int j = 0, i, flag_end = 0;
    char *buff_data = (char *)malloc(transfer_max_size * sizeof(char));
    do
    {
        memset(buff_data, '\0', transfer_max_size);
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
        printf("sending body %s!\n", buff_data);
        int ret2 = send(sockfd, buff_data, i, flags);
        if (ret2 == -1)
        {
            printf("S: Error sending message: %s\n", strerror(errno));
            return -1;
        }
    } while (flag_end != 1);
    return j;
}

ssize_t my_send(int sockfd, const void *buf, size_t len, int flags)
{
    len--;
    int ret;
    message *mess_arr;
    char *mess;
    int next_idx, mess_len;
    // printf("Called My Send!!!!\n");
    pthread_mutex_lock(&mutex_ptr_Send_Message);
    _sockfd = sockfd;
    mess_arr = Send_Message->arr;
    next_idx = Send_Message->next_to_use;
    while (next_idx == Send_Message->size)
    {
        pthread_cond_wait(&cond_s, &mutex_ptr_Send_Message);
        printf("Waiting for table to be empty!!\n");
    }
    mess = mess_arr[next_idx].msg_body;
    mess_arr[next_idx].flags = flags;
    mess_arr[next_idx].len = len;
    memcpy(mess, buf, len + 1);
    Send_Message->next_to_use++;
    printf("Added entry (%s) to Send_Message at index %d\n", mess, Send_Message->next_to_use - 1);
    pthread_cond_signal(&cond_s);
    pthread_mutex_unlock(&mutex_ptr_Send_Message);
    if (len > 5000)
        return 5000;
    return len+1;
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
    message *mess_arr;
    char *mess;
    int next_idx, mess_len;
    recv_flags=flags;
    recv_len = len;
    _recv_sockfd = sockfd;

    printf("my_recv() --> Received_Message->next_to_use=%d\n", Received_Message->next_to_use);

    pthread_mutex_lock(&mutex_ptr_Received_Message);
    while (Received_Message->next_to_use == 0)
    {
        printf("Waiting for recv table to be non-empty\n");
        pthread_cond_wait(&cond_r, &mutex_ptr_Received_Message);
        sleep(1);
    }
    printf("Received this message!!!\n");
    mess_arr = Received_Message->arr;
    next_idx = 0;
    mess_len = mess_arr[next_idx].len;
    mess = mess_arr[next_idx].msg_body;
    flags = mess_arr[next_idx].flags;
    int length = mess_len + 1;
    if (len + 1 < length)
        length = len + 1;
    memcpy(buf, mess, length);
    Received_Message->next_to_use--;
    // remove_from_Received_Message(Received_Message, 0);
    pthread_cond_signal(&cond_r);
    pthread_mutex_unlock(&mutex_ptr_Received_Message);
    return length;
}

int my_close(int sockfd)
{
    sleep(10);
    pthread_kill(s_tid, SIGINT);
    free(Send_Message);
    free(Received_Message);
    int ret = close(sockfd);
    pthread_mutex_unlock(&mutex_ptr_Send_Message);
    pthread_mutex_unlock(&mutex_ptr_Received_Message);
    pthread_mutex_unlock(&mutex_call_recv);

    pthread_mutex_destroy(&mutex_ptr_Received_Message);
    pthread_mutex_destroy(&mutex_ptr_Send_Message);
    pthread_mutex_destroy(&mutex_call_recv);
    pthread_cond_destroy(&cond_r);
    pthread_cond_destroy(&cond_s);
    return ret;
}

void *run_thread_s(void *param)
{
    int ret;
    char buf[5010];
    message *mess_arr;
    char *mess;
    int next_idx, mess_len, flags;
    printf("S: Send Thread Spawned!!\n");
    while (1)
    {
        sleep(2);
        pthread_mutex_lock(&mutex_ptr_Send_Message);
        while (Send_Message->next_to_use == 0)
        {
            printf("S: Waiting for send buffer to be non-empty...\n");
            pthread_cond_wait(&cond_s, &mutex_ptr_Send_Message);
            sleep(1);
        }
        printf("S: Sending Message!\n");
        mess_arr = Send_Message->arr;
        next_idx = 0;
        mess_len = mess_arr[next_idx].len;
        mess = mess_arr[next_idx].msg_body;
        flags = mess_arr[next_idx].flags;
        memcpy(buf, mess, mess_len + 1);
        remove_from_Send_Message(Send_Message, next_idx);
        printf("S: Removed Send_message table entry!\n");
        pthread_mutex_unlock(&mutex_ptr_Send_Message);
        pthread_cond_signal(&cond_s);
        handle_send(buf, _sockfd, mess_len, flags);
    }
}

void *run_thread_r(void *param)
{
    printf("R: Receive Thread Spawned!!\n");
    int ret;
    char buf[5010];
    message *mess_arr;
    char *mess;
    int next_idx, mess_len, flags, sockfd;
    int msg_size;
    char *buff_hdr = (char *)malloc(5 * sizeof(char));

    while (1)
    {
        sleep(5);
        pthread_mutex_lock(&mutex_call_recv);
        while (_recv_sockfd == -1)
        {
            pthread_cond_wait(&cond_recv, &mutex_call_recv);
            printf("R: Waiting for recv to be called!!\n");
            sleep(1);
        }
        pthread_mutex_unlock(&mutex_call_recv);
        memset(buff_hdr, '\0', 5);
        int ret1 = recv(_recv_sockfd, buff_hdr, 5, MSG_WAITALL);
        if (ret1 == -1)
        {
            printf("R: error in my_recv header: %s\n", strerror(errno));
        }
        msg_size = atoi(buff_hdr);
        char *char_buf = (char *)malloc(msg_max_size * sizeof(char));
        memset(char_buf, '\0', msg_max_size);
        int curr_size = 0;
        int flag_end = 0;
        char *chunk2 = (char *)malloc(100 * sizeof(char));
        do
        {
            memset(chunk2, '\0', 100);
            int ret = recv(_recv_sockfd, chunk2, 100, recv_flags);
            if (ret == -1)
            {
                printf("R: error in my_recv : %s\n", strerror(errno));
            }
            strcat(char_buf, chunk2);
            curr_size = find_msg_size(char_buf);
            if (curr_size >= msg_size)
            {
                flag_end = 1;
            }
        } while (flag_end != 1);
        pthread_mutex_lock(&mutex_ptr_Received_Message);
        while (Received_Message->next_to_use == Received_Message->size)
        {
            printf("R: Waiting for receive buffer to be available (not full)...");
            pthread_cond_wait(&cond_r, &mutex_ptr_Received_Message);
            sleep(1);
        }
        next_idx = Received_Message->next_to_use;
        mess_arr = Received_Message->arr;
        memcpy(mess_arr[next_idx].msg_body, char_buf, curr_size);
        mess_arr[next_idx].len = curr_size;
        Received_Message->next_to_use++;
        printf("R: Added Message to Recieve Table\n");
        pthread_cond_signal(&cond_r);
        pthread_mutex_unlock(&mutex_ptr_Received_Message);
        _recv_sockfd = -1;
    }
}