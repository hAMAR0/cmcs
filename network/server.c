#include <stdio.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <errno.h>

#include "server.h"
#include "requests.h"
#include "../protocol/packets.h"

#define PORT 25565
#define MAX_EVENTS 256

void error(const char* msg) {
	perror(msg);
	exit(1);
}

int setnonblocking(int fd) {
	int fl = fcntl(fd, F_GETFL, 0);
	if (fl == -1) return -1;
	return fcntl(fd, F_SETFL, fl | O_NONBLOCK);
}

int server_loop() {
	int sockfd, csockfd, epollfd, nfds;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1) error("socket create");

	int opt = 1;
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	
	struct sockaddr_in client;

	struct sockaddr_in server = {
		.sin_family = AF_INET,
		.sin_port = htons(PORT),
		.sin_addr.s_addr = INADDR_ANY
	};
	
	if (bind(sockfd, (const struct sockaddr*)&server, sizeof(server)) == -1) error("socket bind");
	
	if (listen(sockfd, 777) == -1) error("socket listen");

	struct epoll_event ev, events[MAX_EVENTS];
	
	epollfd = epoll_create1(0);
	if (epollfd == -1) error("epoll_create1");
	
	ev.events = EPOLLIN;
	ev.data.ptr = NULL;
	if (epoll_ctl(epollfd, EPOLL_CTL_ADD, sockfd, &ev) == -1) error("epoll_ctl");
	
	for (;;) {
		nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1);
		if (nfds == -1) error("epoll_wait");

		for (int n = 0; n < nfds; n++) {
			// server socket
			if (events[n].data.ptr == NULL) {
				socklen_t addrlen = sizeof(client);
				csockfd = accept(sockfd, (struct sockaddr*)&client, &addrlen);
				if (csockfd == -1) {
					perror("socket accept");
					continue;
				}
				setnonblocking(csockfd);


				Conn* c = calloc(1, sizeof(Conn)); // TODO: check for NULL
				c->fd = csockfd;
				c->state = ST_HANDSHAKE;

				ev.events = EPOLLIN;
				ev.data.ptr = c;
				if (epoll_ctl(epollfd, EPOLL_CTL_ADD, csockfd, &ev) == -1) {
					perror("epoll_ctl");
					close(c->fd);
					free(c);
				}
			}
			
			//client socket
			else {
				Conn* c = events[n].data.ptr; 
			
				int nread = read(c->fd, c->buf + c->len, sizeof(c->buf) - c->len);
				if (nread > 0) {
					c->len += nread;
				}
				else if (nread == 0) {
					close(c->fd);
					free(c);
					continue;
				}
				else if (errno == EAGAIN || errno == EWOULDBLOCK) { // -1 with EAGAIN || EWOULDBLOCK means socket is empty rn, no need to close and free
				}
				else {
					close(c->fd);
					free(c);
					continue;
				}
				
				
				int total = packetReady(c);
				while (total > 0) {
					int end = c->pos + total; //calculate where current packet ends
					readVarInt(c); // packet length
					int packetId = readVarInt(c);
					if (dispatch(c, packetId) == -1) {
						close(c->fd);
						free(c);
						continue;
					}
					c->pos = end; // move con pos to the end of the current packet
					total = packetReady(c);
				}

				if (total == -1) {
					/* TODO: check if already closed in dispatch
					close(c->fd);
					free(c);
					continue;
					*/
				}

				memmove(c->buf, c->buf + c->pos, c->len - c->pos); // move tail to the position of parsed packet
				c->len -= c->pos;
				c->pos = 0;
			}
		}
	}
	return 0;
}

