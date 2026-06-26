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

	const char* status_response = "{\"version\":{\"name\":\"26.2\",\"protocol\":776}," 
		"\"players\":{\"max\":20,\"online\":5},"
		"\"description\":{\"text\":\"C minecraft server\"}}";

	size_t srlen = strlen(status_response);


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
			if (events[n].data.ptr == NULL) {
				socklen_t addrlen = sizeof(client);
				csockfd = accept(sockfd, (struct sockaddr*)&client, &addrlen);
				if (csockfd == -1) perror("socket accept");
				setnonblocking(csockfd);


				Conn* c = calloc(1, sizeof(Conn));
				c->fd = csockfd;
				c->state = ST_HANDSHAKE;

				ev.events = EPOLLIN;
				ev.data.ptr = c;
				if (epoll_ctl(epollfd, EPOLL_CTL_ADD, csockfd, &ev) == -1) error("epoll_ctl");
			}

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

				// 
			}
		}
	}

	return 0;
}
