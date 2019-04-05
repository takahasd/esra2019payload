#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<stdio.h>
int main()
{
	printf("attempting to send 0");
	int velocity = 0;
	int* vel = &velocity;
	send_velocity(vel);
	return 0;
}
