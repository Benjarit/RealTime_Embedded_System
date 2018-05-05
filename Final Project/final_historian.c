#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <pthread.h>
#include <semaphore.h>

#define MSG_SIZE 40			// message size

sem_t sem;

void* readMessages(void*);
void* sendMessages(void*);

void displayChoice();
void sendCommand();
void printHistory();

char buffer[MSG_SIZE],ip[MSG_SIZE],receivedIP[MSG_SIZE], ipHolder[MSG_SIZE];
char commandMSG[MSG_SIZE];

int sock, n, r;
unsigned int length;
int boolval = 1;		// for a socket option
// Use this to find IP
struct ifreq ifr;
char eth0[] = "wlan0";
socklen_t fromlen;
struct sockaddr_in server;
struct sockaddr_in from; // From the client
int port_number;
int flagg = 0;

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[]){
	int option;
	pthread_t sender, reader;
	
	
    if (argc == 2){
        port_number = atoi(argv[1]);
    } else {
        port_number = 2000; // set default if not provided
    }

    sock = socket(AF_INET, SOCK_DGRAM, 0); // Creates socket. Connectionless.
    if (sock < 0)
        error("socket");
    
    // change socket permissions to allow broadcast
    if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &boolval, sizeof(boolval)) < 0)
    {
        printf("error setting socket options\n");
        exit(-1);
    }

    length = sizeof(server); // determines lenght of the structure
    bzero(&server, length); // set all valus = 0
    server.sin_family = AF_INET; //constant for internet domain
    server.sin_addr.s_addr= INADDR_ANY; // MY IP address
    server.sin_port = htons(port_number);

    if (bind(sock, (struct sockaddr *)&server, length) < 0){
        printf("binding error dumby\n");
    }
    
    if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &boolval, sizeof(boolval)) < 0){
        printf("error setting socket option\n");
        exit(-1);
	} 
	fromlen = sizeof(struct sockaddr_in);


	//Start the pthreads
	pthread_create(&reader, NULL, readMessages, NULL);


	while(1)
	{
		bzero(commandMSG, MSG_SIZE);
		scanf("%s",commandMSG);
		server.sin_addr.s_addr = inet_addr("128.206.19.255");
		n = sendto(sock, commandMSG, MSG_SIZE, 0, (const struct sockaddr *)&server,fromlen);
		if (n<0){
			error("Sendto");
		}
	}
	return 0;
}

void* readMessages(void* agr){
	int count = 1;
	while(1){
		bzero(&buffer,MSG_SIZE); // clear the buffer to NULL
		
		n = recvfrom(sock, buffer, MSG_SIZE, 0, (struct sockaddr *)&from, &fromlen);
		if (n<0)
		{
			error("recv");
		}
		printf("%s\n",buffer);
	}
}
