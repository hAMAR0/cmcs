#include <stdio.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>

#include "network/server.h"
/*
void ping_sequence(int csockfd, const char* status_response, int srlen, Conn* conn){
	readVarInt(conn); // read status request
	readVarInt(conn);
	Wbuf body = {.len = 0};

	writeVarInt(srlen, &body);
	for (int i = 0; i < srlen; i++) {
		writeByte(status_response[i], &body);
	}

	// form packet
	Wbuf packet = {.len = 0};
	writeVarInt(body.len + 1, &packet);
	writeByte(0x00, &packet);
	for (int i = 0; i < body.len; i++) {
		writeByte(body.buf[i], &packet);
	}
		
	write(csockfd, packet.buf, packet.len);

	readVarInt(conn);
	readVarInt(conn);
	uint8_t ping[8];
	for (int i = 0; i < 8; i++) ping[i] = readByte(conn);

	Wbuf pong = {.len = 0};
	writeVarInt(1+8, &pong);
	writeByte(0x01, &pong);
	for (int i = 0; i < 8; i++) writeByte(ping[i], &pong);
	write(csockfd, pong.buf, pong.len);
}

void client_handle(int csockfd, const char* status_response, int srlen) {
	Conn conn = { .fd = csockfd, .pos = 0, .len = 0};

	uint32_t len = readVarInt(&conn);
	uint32_t id = readVarInt(&conn);

	int32_t protocol = readVarInt(&conn);
	int32_t str_len = readVarInt(&conn);
	for (int i = 0; i < str_len; i++) readByte(&conn);
	//port
	readByte(&conn);
	readByte(&conn);
	//intent
	int32_t intent = readVarInt(&conn);
		
	switch(intent) {
		case 1: {
			ping_sequence(csockfd, status_response, srlen, &conn);
			break;
		}
		case 2: {
			uint8_t name[16];
			uint32_t name_len = readVarInt(&conn);
			for (int i = 0; i < name_len; i++) name[i] = readByte(&conn);
			uint8_t uuid[16];
			for (int i = 0; i < 128; i++) uuid[i] = readByte(&conn);
	
			Wbuf body = {.len = 0};
			for (int i = 0; i < 16; i++) writeByte(uuid[i], &body);
			for (int i = 0; i < name_len; i++) writeByte(name[i], &body);
			writeByte(0, &body);
//data types
			break;
		}
		default:
			break;
	}
}
*/
int main() {
	server_loop();
}

