#include "webcam_led.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>

#include <poll.h>
#include <signal.h>
#include <errno.h>

#define MAX_POLLS 200

#define UNUSED(x) ((void)x)

#define LIGHT 1
#define UNLIGHT 2

int connected = 1;

/**
 * Allows to terminate the main loop when receiving a sigint.
 */
void int_handler(int sig)
{
	UNUSED(sig);

	connected = 0;
}

/**
 * Accepts an incoming connection.
 *
 * Returns the newly connected socket, or a negative number in case of failure.
 */
int accept_connection(int sock)
{
	struct sockaddr_un peer_addr;
	socklen_t peer_addr_size;

	peer_addr_size = sizeof(peer_addr);
	return accept(sock, (struct sockaddr*)&peer_addr, &peer_addr_size);
}

/**
 * Adds a socket to the polling list.
 *
 * The first available socket location is used.
 *
 * If there is no room left, -1 is returned.
 */
int add_poll(int child_sock, struct pollfd *polls)
{
	int i;
	for (i = 1; i < MAX_POLLS ; ++i)
	{
		if (polls[i].fd <= 0)
		{
			polls[i].fd = child_sock;
			polls[i].events = POLLIN;
			polls[i].revents = 0;

			return 0;
		}
	}
	return -1;
}

/**
 * Handles incoming data from the socket.
 */
int handle_request(int fd, struct webcam *w)
{
	UNUSED(w);

	char command;
	char buffer[10];
	memset(buffer, 0, sizeof(buffer));

	if (recv(fd, buffer, sizeof(buffer)-1, 0) <= 0)
	{
		return -1;
	}

	if (sscanf(buffer, "%c\n", &command) == 1)
	{
		if (command == '0')
			return UNLIGHT;
		else if (command == '1')
			return LIGHT;
	}

	return 0;
}

/**
 * If data is incoming from the socket, try to connect to the new client.
 *
 * Returns -1 in case of errors.
 *
 * If the socket connection succeeds but there is no room left in the poll
 * list, the socket is closed and 0 is returned.
 */
int try_accept(int sock, struct pollfd *polls, int *num_socks)
{
	int child_sock;
	if (!polls[0].revents)
	{
		return 0;
	}

	child_sock = accept_connection(sock);
	if (child_sock < 0)
	{
		perror("accept");
		return -1;
	}
	if (add_poll(child_sock, polls) < 0)
	{
		fprintf(stderr, "Too many connected clients.\n");
		close(child_sock);
	}
	else
	{
		++*num_socks;
	}
	return 0;
}

/**
 * Process incoming data from all connected clients.
 *
 * This handles the disconnection of clients and updates
 * num_socks appropriately.
 */
void process_clients(struct pollfd *polls, int *num_socks, struct webcam *w)
{
	int i, response, found = 0, lighted = 0;

	static int client_lighted[MAX_POLLS];

	/* Note: starting at 1 because 0 is the listening socket. */
	for (i = 1; i < MAX_POLLS && found < *num_socks; ++i)
	{
		if (polls[i].revents)
		{
			response = handle_request(polls[i].fd, w);

			if (response < 0)
			{
				close(polls[i].fd);
				memset(&polls[i], 0, sizeof(struct pollfd));
				--*num_socks;
			}
			else if (response == LIGHT)
			{
				client_lighted[i] = 1;
			}
			else if (response == UNLIGHT)
			{
				client_lighted[i] = 0;
			}
		}
		if (polls[i].fd >= 0)
		{
			++found;
		}

		lighted |= client_lighted[i];
	}

	if (lighted)
	{
		webcam_light(w);
	}
	else
	{
		webcam_unlight(w);
	}
}

/**
 * Create a UNIX socket at the specified path.
 */
int create_socket(const char *socket_path)
{
	struct sockaddr_un addr;
	int sock;

	remove(socket_path);

	sock = socket(AF_UNIX, SOCK_STREAM, 0);
	if (sock < 0)
	{
		perror("Cannot open socket");
		return -1;
	}

	addr.sun_family = AF_UNIX;
	strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path)-1);

	if(bind(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0)
	{
		perror("Cannot bind to socket");
		return -1;
	}

	if (listen(sock, 10) < 0)
	{
		perror("listen failed");
		return -1;
	}
	return sock;
}



int main(int argc, char **argv)
{
	UNUSED(argc);
	UNUSED(argv);
	struct webcam *w = webcam_init(0);
	struct pollfd polls[MAX_POLLS];
	int num_socks = 1;
	const char *socket_path = "webcam_wait.socket";

	if (!w)
	{
		perror("Cannot initialize the webcam");
		return 1;
	}

	memset(polls, 0, sizeof(polls));

	polls[0].fd = create_socket(socket_path);
	polls[0].events = POLLIN;

	if (polls[0].fd < 0)
	{
		return 1;
	}

	/* If the user presses ctrl C, we still want to properly free our resources
	 * and remove the socket
	 */
	signal(SIGINT, int_handler);

	while(connected)
	{
		if (poll(polls, num_socks, -1) < 0)
		{
			if (errno != EINTR)
			{
				perror("Cannot poll");
			}
			break;
		}
		if (try_accept(polls[0].fd, polls, &num_socks) < 0)
		{
			break;
		}

		process_clients(polls, &num_socks, w);
	}

	close(polls[0].fd);

	remove(socket_path);

	webcam_free(w);
	return 0;
}
