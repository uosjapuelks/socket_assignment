/*******************************
udp_client.c: the source file of the client in udp
********************************/

#include "headsock.h"

float str_cli(FILE *fp, int sockfd, struct sockaddr *addr, int addrlen, long *len);                
void tv_sub(struct  timeval *out, struct timeval *in);	    //calcu the time interval between out and in

int main(int argc, char *argv[])
{
	int sockfd, ret;
	float ti, rt;
	long len;
	struct sockaddr_in ser_addr;
	char **pptr;
	struct hostent *sh;
	struct in_addr **addrs;
	FILE *fp;

	if (argc!= 2)
	{
		printf("parameters not match.");
		exit(0);
	}

	if ((sh=gethostbyname(argv[1]))==NULL) {             //get host's information
		printf("error when gethostbyname");
		exit(0);
	}

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);             //create socket
	if (sockfd<0)
	{
		printf("error in socket");
		exit(1);
	}

	addrs = (struct in_addr **)sh->h_addr_list;
	printf("canonical name: %s\n", sh->h_name);
	for (pptr=sh->h_aliases; *pptr != NULL; pptr++)
		printf("the aliases name is: %s\n", *pptr);			//printf socket information
	switch(sh->h_addrtype)
	{
		case AF_INET:
			printf("AF_INET\n");
		break;
		default:
			printf("unknown addrtype\n");
		break;
	}

	// Set socket receive timeout
    struct timeval timeout;
    timeout.tv_sec = 0; // 5 seconds timeout
    timeout.tv_usec = 50000;

    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0) {
        perror("setsockopt");
        close(sockfd);
        exit(1);
    }


	ser_addr.sin_family = AF_INET;
	ser_addr.sin_port = htons(MYUDP_PORT);
	memcpy(&(ser_addr.sin_addr.s_addr), *addrs, sizeof(struct in_addr));
	bzero(&(ser_addr.sin_zero), 8);

	if ((fp = fopen ("myfile.txt", "r+t")) == NULL)
	{
		printf("File doesn't exit\n");
		exit(0);
	}

	ti = str_cli(fp, sockfd, (struct sockaddr *)&ser_addr, sizeof(struct sockaddr_in), &len);   // perform the transmission and receiving
	rt = (len/(float)ti);
	printf("Time(ms) : %.3f, Data sent(byte): %d\nData rate: %f (Kbytes/s)\n", ti, (int)len, rt);
	
	close(sockfd);
	fclose(fp);
	exit(0);
}

float str_cli(FILE *fp, int sockfd, struct sockaddr *addr, int addrlen, long *len)
{
	char *buf;
	long lsize, ci;
	char sends[DATALEN];
	struct ack_so ack;
	int n, slen, ackNum=0;
	float time_inv = 0.0;
	struct timeval sendt, recvt;
	ci = 0;
	// char sends[MAXSIZE];

	fseek (fp , 0 , SEEK_END);
	lsize = ftell (fp);
	rewind (fp);
	printf("The file length is %d bytes\n", (int)lsize);
	printf("the packet length is %d bytes\n",DATALEN);

// allocate memory to contain the whole file.
	buf = (char *) malloc (lsize);
	if (buf == NULL) exit (2);

  // copy the file into the buffer.
	fread (buf,1,lsize,fp);

  /*** the whole file is loaded in the buffer. ***/
	buf[lsize] ='\0';									//append the end byte
	gettimeofday(&sendt, NULL);							//get the current time
	while(ci<= lsize)
	{
		if ((lsize+1-ci) <= DATALEN)
			slen = lsize+1-ci;
		else 
			slen = DATALEN;

		memcpy(sends, (buf+ci), slen);
		n = sendto(sockfd, &sends, slen, 0, addr, addrlen);	//send the packet to server
		if(n == -1) {
			printf("send error!\n");
			exit(1);
		}

		while ((n= recvfrom(sockfd, &ack, sizeof(ack), 0, addr, &addrlen))==-1)
		{
			if (errno == EWOULDBLOCK || errno == EAGAIN) {
                printf("Timeout occurred. No response from the server. Resending packet.\n");
                if ((n = sendto(sockfd, &sends, slen, 0, addr, addrlen)) == -1) {
					printf("Error in sending Ack packet\n");
					close(sockfd);
					exit(1);
				}
            } else {
                perror("Error when receiving\n");
                close(sockfd);
                exit(1);
            }
		}
		if (ack.error || ack.num<=ackNum) 
			printf("Packet error\n");
		else 
		{
			ci += slen;
			ackNum++;
		}
	}

	gettimeofday(&recvt, NULL);
	*len= ci;						//get current time
	tv_sub(&recvt, &sendt);			// get the whole trans time
	time_inv += (recvt.tv_sec)*1000.0 + (recvt.tv_usec)/1000.0;
	return(time_inv);
}

void tv_sub(struct  timeval *out, struct timeval *in)
{
	if ((out->tv_usec -= in->tv_usec) <0)
	{
		--out ->tv_sec;
		out ->tv_usec += 1000000;
	}
	out->tv_sec -= in->tv_sec;
}