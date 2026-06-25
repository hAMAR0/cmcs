#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

#include "packets.h"

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
	perror("VarInt too big");
}

void writeVarInt(uint32_t val, Wbuf* wbuf) {
	uint32_t uval = (uint32_t)val;

	while ((uval & ~0x7F) != 0) {
		writeByte((uval & 0x7F) | 0x80, wbuf);

		uval >>= 7;
	}
	writeByte(uval, wbuf);
}

