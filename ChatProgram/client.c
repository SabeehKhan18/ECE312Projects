#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pthread.h>

#define BUF_SIZE 256

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

void * receiveMessage(void * socket) {
    int sockfd, ret;
    char buffer[BUF_SIZE];
    sockfd = (int) socket;

    memset(buffer, 0, BUF_SIZE);
//      if (write(sockfd,"I'm waiting for message",23) < 0)
//          error("ERROR writing to socket");

    while ((ret = read(sockfd, buffer, BUF_SIZE)) > 0) {
        printf("server: %s", buffer);
        memset(buffer, 0, BUF_SIZE);
    }
    if (ret < 0) 
        printf("Error receiving data!\n");
    else
        printf("Closing connection\n");
    close(sockfd);
}

int main(int argc, char *argv[])
{
    int sockfd, portno, n, ret;
    pthread_t rThread;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    char buffer[256];
    if (argc < 3) {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(0);
    }
    portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");
    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
        error("ERROR connecting");
    
//     printf("Please enter the message: ");
//     n = read(sockfd, buffer, 255);
//     if (n < 0) {
//         printf("Error reading from socket\n");
//     }
//     printf("%s\n", buffer);
    
    //creating a new thread for receiving messages from the client
    if (ret = pthread_create(&rThread, NULL, receiveMessage, (void *) sockfd)) {
        printf("ERROR: Return Code from pthread_create() is %d\n", ret);
        error("ERROR creating thread");
    }
    
    while(1){
        bzero(buffer,256);
        fgets(buffer,255,stdin);
        n = write(sockfd,buffer,strlen(buffer));
        if (n < 0) {
            error("ERROR writing to socket");
            break;
        }
    }
//     bzero(buffer,256);
//     n = read(sockfd,buffer,255);
//     if (n < 0) 
//          error("ERROR reading from socket");
//     printf("%s\n",buffer);
    close(sockfd);
    return 0;
}
