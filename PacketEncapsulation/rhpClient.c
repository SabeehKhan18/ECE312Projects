/************* UDP CLIENT CODE *******************/

#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>

#define SERVER "137.112.38.47"
#define MESSAGE "hello"
#define PORT 1874
#define BUFSIZE 1024

uint16_t getChecksum(char* data, int numBytes) {
    uint16_t* checkPtr = (uint16_t*) data;
    uint32_t sum = 0;
    for (int i = 0; i < numBytes/2; i++) {
        sum += *checkPtr;
        if (sum >= 0x10000) {
            sum = sum & 0xFFFF;
            sum += 1;
        }
        checkPtr++;
    }
    return (0xFFFF - (uint16_t)sum);
}

int recvRHP(int socket) {
    char buffer[BUFSIZE];
    int nBytes, pad;
    uint8_t* bufPtr;
    uint8_t type, rhmpLength;
    
    memset(buffer, 0, BUFSIZE);
    
    /* Receive message from server */
    nBytes = recvfrom(socket, buffer, BUFSIZE, 0, NULL, NULL);
    
    uint16_t recvChecksum = *((uint16_t*)(buffer+nBytes-2));
    uint16_t calcChecksum = getChecksum(buffer, nBytes-2);
    
    if (recvChecksum != calcChecksum) {
        printf("CHECKSUM MISMATCH\n");
        return 1;
    }
    
    bufPtr = (uint8_t*) buffer;
    printf("type: %d\n", *bufPtr);
    type = *bufPtr;
    
    bufPtr++;
    if (type == 1)
        printf("length: %d\n", *((uint16_t*)bufPtr));
    else
        printf("dstPort: %d\n", *((uint16_t*)bufPtr));
    
    pad = 0;
    if (type == 1) {
        if (*((uint16_t*)bufPtr) % 2 == 0) {
            pad = 1;
            printf("adding buffer on received data\n");
        }
    } else {
        bufPtr+=6;
        rhmpLength = *bufPtr;
        if (rhmpLength % 2 != 0)
            pad = 1;
        bufPtr-=6;
    }
    
    bufPtr+=2;
    printf("srcPort: %d\n", *((uint16_t*)bufPtr));
    
    bufPtr += 2;
    uint16_t rhmpTypeCommID;
    memcpy(&rhmpTypeCommID, bufPtr, 2);
    uint8_t rhmpType = (rhmpTypeCommID & 0xFC00) >> 10;
    printf("RHMP Type: %d\n", rhmpType);
    
    uint16_t commID = rhmpTypeCommID & 0x03FF;
    printf("RHMP Comm ID: %d\n", commID);
    bufPtr+=2;
    
    printf("rhmp length: %d\n", rhmpLength);
    bufPtr++;
    
    if (rhmpType == 1) {
        
    
//     bufPtr+=strlen((char*)bufPtr)+1+pad;
    printf("checksum: 0x%04x\n", *((uint16_t*)bufPtr));
    
    return 0;
}

int sendRHP(int socket, uint8_t type, char* toSend, uint32_t length) {
    char buffer[BUFSIZE];
    uint16_t dstPort_Length = 0;
    int includePad = 0;
    struct sockaddr_in serverAddr;
    
    memset(buffer, 0, BUFSIZE);
    
    uint8_t* bufPtr = (uint8_t*) buffer;
    
    *bufPtr = type;
    bufPtr++;
    
    if (type == 0) {
        dstPort_Length = 105;
    } else {
        dstPort_Length = length;
    }
    memcpy(bufPtr, &dstPort_Length, 2);
    bufPtr += 2;
    
    uint16_t srcPort = 354;
    memcpy(bufPtr, &srcPort, 2);
    bufPtr+=2;
    
    memcpy(bufPtr, toSend, length);
    bufPtr+=length;
    
    includePad = 0;
    if (length % 2 == 0) {
        includePad = 1;
        bufPtr++;
    }
    
    uint16_t checksum = getChecksum(buffer, 5+length+includePad);
    memcpy(bufPtr, &checksum, sizeof(checksum));
    
    /* Configure settings in server address struct */
    memset((char*) &serverAddr, 0, sizeof (serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = inet_addr(SERVER);
    memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);
    
    /* send a message to the server */
    if (sendto(socket, buffer, 7+length+includePad, 0,
            (struct sockaddr *) &serverAddr, sizeof (serverAddr)) < 0) {
        perror("sendto failed");
        return 1;
    }
    
    return 0;
}

int main() {
    int clientSocket, result;
    char buffer[BUFSIZE] = {0};
    struct sockaddr_in clientAddr;

    /*Create UDP socket*/
    if ((clientSocket = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("cannot create socket");
        return 0;
    }
    
    /* Bind to an arbitrary return address.
     * Because this is the client side, we don't care about the address 
     * since no application will initiate communication here - it will 
     * just send responses 
     * INADDR_ANY is the IP address and 0 is the port (allow OS to select port) 
     * htonl converts a long integer (e.g. address) to a network representation 
     * htons converts a short integer (e.g. port) to a network representation */
    memset((char *) &clientAddr, 0, sizeof (clientAddr));
    clientAddr.sin_family = AF_INET;
    clientAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    clientAddr.sin_port = htons(0);

    if (bind(clientSocket, (struct sockaddr *) &clientAddr, sizeof (clientAddr)) < 0) {
        perror("bind failed");
        return 0;
    }
    
    result = sendRHP(clientSocket, 1, "hello\0", 6);
    result = recvRHP(clientSocket);
    
    memset(buffer, 0, BUFSIZE);
    
    uint32_t data = 8;
    data = data | (312 << 6);
    memcpy(buffer, &data, 3);
    
    do {
        result = sendRHP(clientSocket, 0, buffer, 3);
    } while ((result = recvRHP(clientSocket)) != 0);

    close(clientSocket);
    return 0;
}
