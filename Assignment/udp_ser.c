/**************************************
udp_ser.c: the source file of the server in udp transmission
**************************************/
#include "headsock.h"

#define BACKLOG 10

int str_ser(int sockfd);			// transmitting and receiving function

int main(int argc, char *argv[])
{
	int sockfd, con_fd, ret;
	struct sockaddr_in my_addr;
	struct sockaddr_in their_addr;
	int sin_size;
	int end=0;

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
    timeout.tv_sec = 0; // 5 seconds timeout
    timeout.tv_usec = 5000;

    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0) {
        perror("setsockopt");
        close(sockfd);
        exit(1);
    }


	printf("receiving start\n");
	while(end!=1) {
		printf("waiting for data\n");
		end+=str_ser(sockfd);			// send and receive
	}
	close(sockfd);
	exit(0);
}

int str_ser(int sockfd)
{
	int noError = 0;
	int sendError = 0;
	srand(time(NULL));
	int missing=0;
	int errCnt=0;
	int dupe=0;

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
		noError = (rand()%1000 >= ERRORRATE);
		sendError = (rand()%2 == 0);

		while (((n=recvfrom(sockfd, &recvs, DATALEN+1, 0, (struct sockaddr *)&addr, &len)) == -1)) {      //receive the packet
            if ((errno == EWOULDBLOCK || errno == EAGAIN) && ack.num>0) {
                // printf("Timeout occurred. No response from the client.\n");
				missing++;
                if ((n=sendto(sockfd, &ack, sizeof(ack), 0, (struct sockaddr *)&addr, len)) == -1) {
					printf("Error in sending Ack packet\n");
				}
            } else if (ack.num>0) {
                printf("Error in receiving packet\n");
            }
		}

		if (recvs[n-1] == '\0') {
			end = 1;
			n--;
		}

		if (recvs[0] == ack.num) 
		{
			// printf("Received Packet %d size %d current ack %d\n", recvs[0], n, ack.num);
			memcpy((buf+lseek), recvs+1, n-1);
			lseek+=n-1;
			ack.num++;
		} 
		else if (recvs[0]+1 != ack.num)
			errCnt++;
		else if (recvs[0]+1 == ack.num)
			dupe++;
		
		if (noError || end) {
			if ((n=sendto(sockfd, &ack, sizeof(ack), 0, (struct sockaddr *)&addr, len)) == -1) {
				printf("Error in sending Ack packet\n");
				close(sockfd);
				exit(1);
			}
		}
		else if (!noError && sendError) {
			ack.error=1;
			if ((n=sendto(sockfd, &ack, sizeof(ack), 0, (struct sockaddr *)&addr, len)) == -1) {
				printf("Error in sending Ack packet\n");
				close(sockfd);
				exit(1);
			}
			ack.error=0;
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
	printf("\nTotal missing: %d, Error: %d, Dupes: %d\n", missing, errCnt, dupe);
	printf("a file has been successfully received!\nthe total data received is %d bytes\n", (int)lseek);
	return 1;
}
