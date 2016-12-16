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
int nClose;
char username[256] = { 0 };

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

void * receiveMessage(void * socket) {
    int sockfd, ret;
    char buffer[BUF_SIZE],send_user[BUF_SIZE];
    sockfd = (int) socket;
    int wasMe = 0;                   

	memset(send_user, 0, BUF_SIZE);
	ret = read(sockfd, send_user, BUF_SIZE);
	if (ret < 0)
		printf("Error receiving data!\n");

    memset(buffer, 0, BUF_SIZE);
    while (nClose) {
        ret = read(sockfd, buffer, BUF_SIZE);
        if (strcmp(buffer,"exit\n") == 0) {
            nClose = 0;
            wasMe = 1;
        }
        printf("%s: %s", send_user, buffer);
        memset(buffer, 0, BUF_SIZE);
    }
    if (ret < 0) 
        printf("Error receiving data!\n");
    if (wasMe)
        printf("Chat ended. Press enter to exit\n");
    close(sockfd);

}

int main(int argc, char *argv[])
{
    int sockfd, portno, n, ret;
    pthread_t rThread;
    struct sockaddr_in serv_addr;
    struct hostent *server;
	char user[BUF_SIZE];
    nClose = 1;

    char buffer[256];
    if (argc < 3) {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(0);
    }
	
	printf("Enter username: ");
	fgets(user,255,stdin);
	strtok(user,"\n");

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
    
    
    //creating a new thread for receiving messages from the client
    if (ret = pthread_create(&rThread, NULL, receiveMessage, (void *) sockfd)) {
        printf("ERROR: Return Code from pthread_create() is %d\n", ret);
        error("ERROR creating thread");
    }
    n = write(sockfd,user,strlen(user));
	if (n < 0) {
		error("ERROR writing to socket");
	}

    while(nClose){
        bzero(buffer,256);
        fgets(buffer,255,stdin);
        if (nClose) {
            n = write(sockfd,buffer,strlen(buffer));
            if (strcmp(buffer,"exit\n") == 0)
                nClose = 0;
            if (n < 0) {
                error("ERROR writing to socket");
                break;
            }
        }
    }

    close(sockfd);
    return 0;
}
