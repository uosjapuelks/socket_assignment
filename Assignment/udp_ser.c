/**************************************
udp_ser.c: the source file of the server in udp transmission
**************************************/
#include "headsock.h"

#define BACKLOG 10

void str_ser(int sockfd);			// transmitting and receiving function

int main(int argc, char *argv[])
{
	int sockfd, con_fd, ret;
	struct sockaddr_in my_addr;
	struct sockaddr_in their_addr;
	int sin_size;
	pid_t pid;

	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {		//create socket
		printf("error in socket");
		exit(1);
	}

	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(MYUDP_PORT);
	my_addr.sin_addr.s_addr = INADDR_ANY;
	bzero(&(my_addr.sin_zero), 8);
	ret = bind(sockfd, (struct sockaddr *) &my_addr, sizeof(struct sockaddr));
	if (ret < 0) {           //bind socket
		printf("error in binding");
		exit(1);
	}

	printf("receiving start\n");
	while(1) {
		printf("waiting for data\n");
		str_ser(sockfd);			// send and receive
	}
	close(sockfd);
	exit(0);
}

void str_ser(int sockfd)
{
	char buf[BUFSIZE];
	FILE *fp;
	char recvs[DATALEN];
	struct ack_so ack;
	int end, n = 0;
	long lseek=0;
	end = 0;
	struct sockaddr_in addr;

	int len = sizeof (struct sockaddr_in);

	while(!end)
	{
		if ((n=recvfrom(sockfd, &recvs, DATALEN, 0, (struct sockaddr *)&addr, &len)) == -1) {      //receive the packet
			printf("error when receiving\n");
			exit(1);
		}
		else printf("%d %c %ld\n", n, recvs[n-1], lseek);

		if (recvs[n-1] == '\0') {
			end = 1;
			n--;
		}
		memcpy((buf+lseek), recvs, n);
		lseek+=n;
	}
	recvs[n] = '\0';
	
	if ((fp = fopen ("myUDPreceive.txt","wt")) == NULL)
	{
		printf("File doesn't exit\n");
		exit(0);
	}
	fwrite (buf , 1 , lseek , fp);				//write data into file
	fclose(fp);
	printf("a file has been successfully received!\nthe total data received is %d bytes\n", (int)lseek);
}
