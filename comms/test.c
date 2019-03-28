#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<client.h>
int main()
{
	print("Included");
	int velocity = 0;
	int* vel = &velocity;
	send_velocity(vel);

