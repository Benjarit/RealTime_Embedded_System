/* Lab6_cdev_user.c
 * ECE4220/7220
 * Author: Luis Alberto Rivera

 This program allows you to enter a message on the terminal, and then it writes the
 message to a Character Device. The Device should be created beforehand, as described
 in the Lab6_cdev_kmod.c file.
 Try this example together with the Lab6_cdev_kmod module. Once you enter a message, run
	dmesg | tail     ( | tail  is not needed. With that, you'll only see the last few entries).
 on a different terminal. You should see the message in the system log (printk'd in the
 module). That would mean that the message is getting to kernel space.
 Use elements from this example and the Lab6_cdev_user module in your Lab 6 programs. You may
 need to modify a bit the callback functions in the module, according to your needs.

 For the extra credit part of lab 6, you'll need to think about how to read messages coming
 from kernel space, and how to create those messages in the module...
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
#include <fcntl.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>
#define CHAR_DEV "/dev/Benjarit" // "/dev/YourDevName"
#define MSG_SIZE 40



int sock, length, n, r;
int boolval = 1;		// for a socket option

// Use this to find IP
struct ifreq ifr;
char eth0[] = "wlan0";

socklen_t fromlen;
struct sockaddr_in server;
struct sockaddr_in from; // From the client

char buffer[MSG_SIZE],ip[MSG_SIZE],receivedIP[MSG_SIZE], ipHolder[MSG_SIZE];

char charDevMessage[MSG_SIZE];
sem_t charDev_Semaphore;

int port_number;
int master = 0;
int flagg = 0;

void error(const char *msg)
{
    perror(msg);
    exit(0);
}
void writeToCharacterDevice(char* buff){
	int cdev_id, dummy;
	if((cdev_id = open("/dev/Benjarit", O_WRONLY)) == -1) {
		printf("Cannot open device in Write function %s\n", CHAR_DEV);
		exit(1);
	}
	printf("Send this note to the Kernal Space: %s\n", buff);
	dummy = write(cdev_id, buff, sizeof(buff));
	if(dummy != sizeof(buff)) {
		printf("Write failed, leaving...\n");
	}
	close(cdev_id);	// close the device.
}
/* *********************************************** EXTRA CREDIT ******************************************
 *	read function
 *	takes button push and send it to all slave boards
 * */
char broadcastMSG[MSG_SIZE];
int	toBeTold = 0;
void* readFromCharacterDevice(){
	int cdev_id, dummy;

	if((cdev_id = open("/dev/Benjarit", O_RDONLY)) == -1) {
		printf("Cannot open device in Read function %s\n", CHAR_DEV);
		exit(1);
	}
	while(1){
		read(cdev_id, broadcastMSG, sizeof(broadcastMSG));
		if(strlen(broadcastMSG) != 0){
			printf("Sending note::::: %s\n", broadcastMSG);
			if(master == 1){
					printf("I'm master and I'm sending note: %s\n", broadcastMSG);
					from.sin_addr.s_addr = inet_addr("128.206.19.255");
					n = sendto(sock, broadcastMSG, MSG_SIZE, 0, (struct sockaddr *)&from, fromlen);
					toBeTold = 1;
			}
			//sem_post(&charDev_Semaphore);
		}
		broadcastMSG[0] = '\0';
		usleep(100);
	}
	close(cdev_id);	// close the device.
}
/* *********************************************** EXTRA CREDIT ****************************************** */

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
    sem_init(&charDev_Semaphore, 0, 0);
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
	pthread_t tid;
	pthread_create(&tid, NULL, readFromCharacterDevice, NULL);
    fromlen = sizeof(struct sockaddr_in);	// size of structure

    while (1)
    {
        // receive from client
    	bzero(&buffer,MSG_SIZE); // clear the buffer to NULL


        n = recvfrom(sock, buffer, MSG_SIZE, 0, (struct sockaddr *)&from, &fromlen);
        if (n < 0)
            error("recvfrom");
		// Fix the recieving and broadcasting the note issue
		if (flagg == 1){
			strcpy(buffer,"xxx");
			flagg = 0;
		}


		if(strncmp(buffer, "xxx", 3)){
			printf("Message Received: %s\n",buffer);
    	}
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
		else if(buffer[0] == '@')
		{
			writeToCharacterDevice(buffer);
			if(master == 1 && toBeTold != 1){
				from.sin_addr.s_addr = inet_addr("128.206.19.255");
				n = sendto(sock, buffer, MSG_SIZE, 0, (struct sockaddr *)&from, fromlen);
				if (n < 0){
					error("sendto");
				}
			}
			flagg = 1;
			toBeTold = 0;
		}
    }
    return 0;
}

