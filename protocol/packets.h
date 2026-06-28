#pragma once

#include <stdio.h>
#include <stdint.h>

typedef enum {
	ST_HANDSHAKE,
	ST_STATUS,
	ST_LOGIN,
	ST_CONFIGURATION,
	ST_PLAY
} ConnState;

typedef struct {
	uint8_t buf[4096];
	size_t pos;
	size_t len;
	int fd;
	ConnState state;
} Conn;

typedef struct {
	uint8_t buf[4096];
	size_t pos;
	size_t len;
} Wbuf;

int readByte(Conn* conn);

void writeByte(uint8_t byte, Wbuf* wbuf);

int readVarInt(Conn *conn);

void writeVarInt(uint32_t val, Wbuf* wbuf);

int packetReady(Conn* conn);
