#include "webcam_led.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>


int main(int argc, char **argv)
{
	struct webcam *w = webcam_init(0);
	if (!w)
	{
		perror("Cannot initialize the webcam");
		return 1;
	}

	int sock = socket(AF_UNIX, SOCK_STREAM, 0);
	if (sock < 0)
	{
		perror("Cannot open socket");
		return 1;
	}

	fd_set rfds;

	FD_ZERO(&rfds);
	FD_SET(sock, &rfds);


	while (

	webcam_free(w);
	return 0;
}
