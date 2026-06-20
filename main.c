#include <stdio.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>

typedef struct {
	uint8_t buf[4096];
	size_t pos;
	size_t len;
	int fd;
} Conn;

typedef struct {
	uint8_t buf[4096];
	size_t pos;
	size_t len;
} Wbuf;

void error(const char* msg) {
	perror(msg);
	exit(1);
}

int readByte(Conn* conn) {
	if (conn->len == 0) {
		int n = read(conn->fd, conn->buf, sizeof(conn->buf));
		if (n == -1) perror("readbyte");
		conn->len = n;
		conn->pos = 0;
	}
	uint8_t byte = conn->buf[conn->pos];
	conn->pos++;
	conn->len--;
	return byte;
}

void writeByte(uint8_t byte, Wbuf* wbuf) {
	wbuf->buf[wbuf->len++] = byte;
}

int readVarInt(Conn *conn) {
	int val = 0;

	for (int pos = 0; pos < 32; pos += 7) {
		uint8_t curByte = readByte(conn);
		
		val  |= (int32_t)(curByte & 0x7F) << pos;

		if ((curByte & 0x80) == 0) return val;
	}
	error("VarInt too big");
}

void writeVarInt(uint32_t val, Wbuf* wbuf) {
	uint32_t uval = (uint32_t)val;

	while ((uval & ~0x7F) != 0) {
		writeByte((uval & 0x7F) | 0x80, wbuf);

		uval >>= 7;
	}
	writeByte(uval, wbuf);
}

int main() {
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1) error("socket create");

	int csockfd;
	
	struct sockaddr_in client;
	struct sockaddr_in server = {
		.sin_family = AF_INET,
		.sin_port = htons(25565),
		.sin_addr.s_addr = INADDR_ANY
	};
	
	if (bind(sockfd, (const struct sockaddr*)&server, sizeof(server)) == -1) error("socket bind");
	
	if (listen(sockfd, 777) == -1) error("socket listen");

	const char* status_response = "{\"version\":{\"name\":\"26.2\",\"protocol\":776}," 
		"\"players\":{\"max\":20,\"online\":5},"
		"\"description\":{\"text\":\"C minecraft server\"}}";

	size_t srlen = strlen(status_response);

	while (1) {
		socklen_t addrlen = sizeof(client);
		csockfd = accept(sockfd, (struct sockaddr*)&client, &addrlen);
		if (csockfd == -1) perror("socket accept");
		
		Conn conn = { .fd = csockfd, .pos = 0, .len = 0};

		uint32_t len = readVarInt(&conn);
		for (int i = 0; i < len; i++) readByte(&conn); //read handshake

		readVarInt(&conn); // read status request
		readVarInt(&conn);

		Wbuf body = {.len = 0};

		writeVarInt(srlen, &body);
		for (int i = 0; i < srlen; i++) {
			writeVarInt(status_response[i], &body);
		}
	}
}


