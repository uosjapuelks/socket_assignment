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

	// Set socket receive timeout
    struct timeval timeout;
    // timeout.tv_sec = 5; // 5 seconds timeout
    timeout.tv_usec = 50000;

    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0) {
        perror("setsockopt");
        close(sockfd);
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
	char recvs[DATALEN+1];

	struct ack_so ack;
	ack.num=0;
	ack.error=0;

	int end, n = 0;
	long lseek=0;
	end = 0;
	struct sockaddr_in addr;

	socklen_t len = sizeof (struct sockaddr_in);

	while(!end)
	{
		while ((n=recvfrom(sockfd, &recvs, DATALEN+1, 0, (struct sockaddr *)&addr, &len)) == -1) {      //receive the packet
            if (errno == EWOULDBLOCK || errno == EAGAIN) {
                printf("Timeout occurred. No response from the client.\n");
                if ((n=sendto(sockfd, &ack, sizeof(ack), 0, (struct sockaddr *)&addr, len)) == -1) {
					printf("Error in sending Ack packet\n");
					close(sockfd);
					exit(1);
				}
            } else {
                perror("Error in receiving packet\n");
                close(sockfd);
                exit(1);
            }
		}

		if (recvs[n-1] == '\0') {
			end = 1;
			n--;
		}
		printf("Received Packet %d size %d current ack %d\n", recvs[0], n, ack.num);
		memcpy((buf+lseek), recvs+1, n-1);
		lseek+=n-1;
		ack.num++;
		if ((n=sendto(sockfd, &ack, sizeof(ack), 0, (struct sockaddr *)&addr, len)) == -1) {
			printf("Error in sending Ack packet\n");
			close(sockfd);
			exit(1);
		}
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
