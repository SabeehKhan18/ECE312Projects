/************* UDP CLIENT CODE *******************/

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>

#define SERVER "137.112.38.47"
#define MESSAGE "hello"
#define PORT 1874
#define BUFSIZE 1024

uint16_t getChecksum(char* data, int numBytes) {
    uint16_t* checkPtr = data;
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

int main() {
    int clientSocket, nBytes, pad, includePad;
    char buffer[BUFSIZE] = {0};
    struct sockaddr_in clientAddr, serverAddr;

    /*Create UDP socket*/
    if ((clientSocket = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("cannot create socket");
        return 0;
    }
    
    // clear out entire buffer
    memset(buffer, 0, BUFSIZE);
    
    // create pointer to step through each piece of data
    uint8_t* bufPtr = buffer;
    
    // message type (0 = RHMP, 1 = ASCII control message)
    *bufPtr = 1;
    bufPtr++;
    
    // re-cast pointer to be 16 bits for dstPort/length
    // length of message since we are doing a control message
    uint16_t length = strlen(MESSAGE)+1;
    memcpy(bufPtr, &length, sizeof(length));
    bufPtr+=2;
    
    // srcPort (CM number)
    uint16_t srcPort = 354;
    memcpy(bufPtr, &srcPort, sizeof(srcPort));
    bufPtr+=2;
    
    // re-cast to a char for the string message
    memcpy(bufPtr, &MESSAGE, strlen(MESSAGE));
    // skip a byte in order to null-terminate the string
    bufPtr += strlen(MESSAGE)+1;
    
    // set pad to 0 for default
    includePad = 0;
    if (strlen(MESSAGE) % 2 != 0) {
        includePad = 1;
        bufPtr++;
    }
    
    uint16_t checksum = getChecksum(buffer, strlen(MESSAGE)+6+includePad);
    memcpy(bufPtr, &checksum, sizeof(checksum));
    
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

    /* Configure settings in server address struct */
    memset((char*) &serverAddr, 0, sizeof (serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = inet_addr(SERVER);
    memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);

    /* send a message to the server */
    if (sendto(clientSocket, buffer, strlen(MESSAGE)+includePad+8, 0,
            (struct sockaddr *) &serverAddr, sizeof (serverAddr)) < 0) {
        perror("sendto failed");
        return 0;
    }

    /* Receive message from server */
    nBytes = recvfrom(clientSocket, buffer, BUFSIZE, 0, NULL, NULL);
    
    bufPtr = buffer;
    printf("type: %d\n", *bufPtr);
    
    bufPtr++;
    printf("length: %d\n", *((uint16_t*)bufPtr));
    if (*((uint16_t*)bufPtr) % 2 == 0) {
        pad = 1;
        printf("adding pad on received data\n");
    }
    
    bufPtr+=2;
    printf("srcPort: %d\n", *((uint16_t*)bufPtr));
    
    bufPtr += 2;
    printf("Received from server: %d, %s\n", nBytes, bufPtr);
    
    bufPtr+=strlen(bufPtr)+1+pad;
    printf("checksum: 0x%04x\n", *((uint16_t*)bufPtr));
    
    uint16_t recvChecksum = getChecksum(buffer, nBytes-2);
    printf("calculated checksum on received data: 0x%04x\n", recvChecksum);

    close(clientSocket);
    return 0;
}
