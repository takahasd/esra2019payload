#include<unistd.h>
#include<stdio.h>
#include<sys/socket.h>
#include<stdlib.h>
#include<netinet/in.h>
#include<string.h>
#include<arpa/inet.h>
#define PORT 65435
int send_velocity(int* velocity)
{
	struct sockaddr_in address;
	int sock = 0,valread;
	struct sockaddr_in serv_addr;
	char buffer[1024] = {0};
	if ((sock = socket(AF_INET,SOCK_STREAM,0)) < 0)
	{
		printf("\n Socket creation error \n");
		return -1;
	}
	memset(&serv_addr, '0', sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);
	if(inet_pton(AF_INET,"127.0.0.1",&serv_addr.sin_addr)<=0)
	{
		printf("\nInvalid Adress/ Address not supported \n");
		return -1;
	}
	if(connect(sock,(struct sockaddr *)&serv_addr,sizeof(serv_addr))<0)
	{
		printf("\nConnection Failed\n");
	}
	send(sock,velocity,4,0);
	printf("Velocity Sent\n");
	valread = read(sock,buffer,1024);
	printf("%s\n",buffer);
	shutdown(sock,2);
	return 0;	
}
int main()
{
	int velocity = 0;
	int* vel = &velocity;
	while(1)
	{
		printf("Setpoint:");
		scanf("%d",vel);
		send_velocity(vel);
	}
	return 0;
}

