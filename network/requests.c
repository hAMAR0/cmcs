#include <stdint.h>
#include <string.h>
#include <unistd.h>

#include "requests.h"
#include "../protocol/packets.h"


int dispatch(Conn* c, int id) {

	switch(c->state) {
		case ST_HANDSHAKE:
			return handshake_handle(c);
		case ST_STATUS:
			if (id == 0x00) return status_req_handle(c);
			else if (id == 0x01) return ping_req_handle(c);
			break;
		case ST_LOGIN:
			if (id == 0x00) return login_handle(c);
			if (id == 0x03) {
				c->state = ST_CONFIGURATION;
			}
			break;
		default: return 0;
	}
	return 0;
}

int handshake_handle(Conn* c) {
	readVarInt(c); // protocol version
	
	// server address
	int salen = readVarInt(c);
	if (salen > 256 || salen < 0) return -1;
	for (int i = 0; i < salen; i++) readByte(c);

	//server port
	readByte(c);
	readByte(c);

	int intent = readVarInt(c);
	switch(intent) {
		case 1:
			c->state = ST_STATUS;
			return 1;
		case 2:
			c->state = ST_LOGIN;
			return 2;
		default: //TODO: add transfer support
			return -1;
	}

}

int status_req_handle(Conn* c) {	
	const char* status_response = "{\"version\":{\"name\":\"26.2\",\"protocol\":776}," 
		"\"players\":{\"max\":20,\"online\":5},"
		"\"description\":{\"text\":\"C minecraft server\"}}";

	size_t srlen = strlen(status_response);

	Wbuf body = {0}, packet = {0};
	writeVarInt(srlen, &body);
	for (size_t i = 0; i < srlen; i++) {
		writeByte(status_response[i], &body);
	}
	writeVarInt(body.len+1, &packet);
	writeVarInt(0x0, &packet);
	for (size_t i = 0; i < body.len; i++) {
		writeByte(body.buf[i], &packet);
	}
	return write(c->fd, packet.buf, packet.len);
}

int ping_req_handle(Conn* c) {
	Wbuf packet = {0};
	uint8_t timestamp[8];
	for (int i = 0; i < 8; i++) {
		timestamp[i] = readByte(c);
	}

	writeVarInt(sizeof(timestamp) + 1, &packet);
	writeVarInt(0x01, &packet);
	for (size_t i = 0; i < sizeof(timestamp); i++) {
		writeByte(timestamp[i], &packet);
	}
	return write(c->fd, packet.buf, packet.len);
}

int login_handle(Conn* c) {
	uint16_t nameLen;
	char name[16];
	nameLen = readVarInt(c);
	if (nameLen > 16) return -1;
	for (size_t i = 0; i<nameLen; i++) name[i] = readByte(c);
	
	uint8_t uuid[16];
	for (size_t i = 0; i<sizeof(uuid); i++) uuid[i] = readByte(c);

	Wbuf body = {0}, packet = {0};
	for (size_t i = 0; i < sizeof(uuid); i++) writeByte(uuid[i], &body);
	writeVarInt(nameLen, &body);
	for (size_t i = 0; i < nameLen; i++) writeByte(name[i], &body);

	writeVarInt(body.len + 1, &packet);
	writeVarInt(0x02, &packet);
	for (size_t i = 0; i < body.len; i++) writeByte(body.buf[i], &packet);
	return write(c->fd, packet.buf, packet.len);
}
