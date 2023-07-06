/*    THE CLIENT PROCESS */
#include "mysocket.h"

int main(){
    int                 sockfd;
    struct sockaddr_in serv_addr;
    char buff[5000];                                             //  We will use this buffer for communication
    if((sockfd = my_socket(AF_INET, SOCK_MyTCP, 0))<0){           // Opening a socket is exactly similar to the server process
        perror("Unable to create socket\n");
        exit(0);
    }
    serv_addr.sin_family        = AF_INET;
    inet_aton("127.0.0.1", &serv_addr.sin_addr);
    serv_addr.sin_port = htons(22461);

    if ((my_connect(sockfd, (struct sockaddr *) &serv_addr,        // connecting to server
						sizeof(serv_addr))) < 0) {
		perror("Unable to connect to server\n");
		exit(0);
	}
    
    long int len = my_recv(sockfd,buff,5000,0);                                    // receiving the time date string from server
    printf("received following message of %ld length\n\n",len);
    printf("%s\n",buff);
    close(sockfd);
    return 0;
}