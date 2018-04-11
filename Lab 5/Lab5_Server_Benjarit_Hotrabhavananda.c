/*
 ============================================================================
 Name        : Lab5.c
 Author      : Benjarit Hotrabhavananda
 Version     :
 Copyright   : Your copyright notice
 Description : The server_udp_boardcast
 ============================================================================
 */


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


#define MSG_SIZE 40			// message size

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

/*
 * parses the IP address
 * returns the board Number
 */
int parseIP(char* IP)
{
    char *saveptr;
    char* temp = strtok_r(IP, ".", &saveptr);
    int i = 1, numBoard = 0;
    while(temp != NULL)
    {
        if(i == 4)
            numBoard = atoi(temp);
        temp = strtok_r(NULL, ".", &saveptr);
        i++;
    }
    return numBoard;
}

int main(int argc, char *argv[])
{
    srand(time(NULL));  // generate the new seeds

    int sock, length, n, r;
    int boolval = 1;		// for a socket option

	// Use this to find IP
	struct ifreq ifr;
	char eth0[] = "wlan0";

    socklen_t fromlen;
    struct sockaddr_in server;
    struct sockaddr_in from; // From the client

    char buffer[MSG_SIZE],ip[MSG_SIZE],ipHolder[MSG_SIZE];

    int port_number;
    int master = 0;

    // set the port number
    if (argc == 2){
        port_number = atoi(argv[1]);
    } else {
        port_number = 2000; // set default if not provided
    }

	// Creates socket. Connectionless.
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0){
        error("Opening socket");
	}

    length = sizeof(server);                // length of structure
    bzero(&server,length);                  // sets all values to zero. memset() could be used
    server.sin_family = AF_INET;            // symbol constant for Internet domain
    server.sin_port = htons(port_number);	// port number
    server.sin_addr.s_addr = INADDR_ANY;	// IP address of the machine on which the server is running



    // gets the host name and the IP
	bzero(&ifr, sizeof(ifr));	// Set all values to zero
    ifr.ifr_addr.sa_family = AF_INET;	// Type of address to retrieve - IPv4 IP address
    strncpy((char* )&ifr.ifr_name , eth0 , IFNAMSIZ-1);		// Copy the interface name in the ifreq structure
	// Get IP address
    if (ioctl(sock,SIOCGIFADDR,&ifr) == -1) {
		error("Cannot get IP address");
		close(sock);
		exit(0);
	}
	// Converts the internet host address in network byte order to a string in IPv4 dotted-decimal notation
    strcpy(ip,inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr )->sin_addr));
    strcpy(ipHolder,ip);
    int current_board_number = parseIP(ipHolder);
    printf("IP : %s\n",ip);
    printf("Board Number : %d\n",current_board_number);



    // binds the socket to the address of the host and the port number
    if (bind(sock, (struct sockaddr *)&server, length) < 0){
        error("binding");
	}
    // set broadcast option
    if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &boolval, sizeof(boolval)) < 0)
   	{
        printf("error setting socket options\n");
        exit(-1);
   	}
    fromlen = sizeof(struct sockaddr_in);	// size of structure

    while (1)
    {
        // receive from client
    	bzero(&buffer,MSG_SIZE); // clear the buffer to NULL
        n = recvfrom(sock, buffer, MSG_SIZE, 0, (struct sockaddr *)&from, &fromlen);
        if (n < 0)
            error("recvfrom");

        printf("Message Received: %s\n",buffer);

        // if the message is WHOIS check if the server is master and then send a message to the client if it is.
        if (!strncmp(buffer, "WHOIS", 5)){
        	  if(master == 1){

				// Master message
        		//sprintf(buffer, "Benjarit on board %s is the master",  ip);
        		sprintf(buffer, "Benjarit on %s is the master.", ip);

				// Send message
				n = sendto(sock, buffer, MSG_SIZE, 0, (struct sockaddr *)&from, fromlen);
					if (n  < 0){
						error("sendto");
					}
        	}
        }

        // if the message is VOTE randomize a number and then send # IP Number to all machines on the network.
        else if (!strncmp(buffer, "VOTE", 4)){
            master = 1;
            r = (rand() % 10) + 1;

            sprintf(buffer, "# %s %d", ip, r);

            // socket function
            // send message to clientWHO
            from.sin_addr.s_addr = inet_addr("128.206.19.255");
            n = sendto(sock, buffer, MSG_SIZE, 0, (struct sockaddr *)&from, fromlen);
            if (n < 0)
                error("sendto");
        }


        //receiving other boards' votes
        //only checks the votes if status is master
        else if(buffer[0] == '#' && master)
        {
            char *saveptr;
            char *temp = strtok_r(buffer, " ", &saveptr);
            int count = 1, boardNum, randomNum;
            while(temp != NULL)
            {
                // Parse the message from other servers
                // Message: # [IP_address] [random_number]
                if(count == 2){
                    boardNum = parseIP(temp);
                }
                else if(count == 3){
                    randomNum = atoi(temp);
                }
                temp = strtok_r(NULL, " ", &saveptr);
                count++;
            }

            // compare the random number from other servers
            if(randomNum > r) {
                master = 0;
            }
            if(randomNum == r){ // tie break case
                if(current_board_number < boardNum){
					master = 0;
				}
            }
        }
    }
    return 0;
}
