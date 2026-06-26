#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

#include "packets.h"

int readByte(Conn* conn) {
	if (conn->pos >= conn->len) return -1;
	return conn->buf[conn->pos++];
}

void writeByte(uint8_t byte, Wbuf* wbuf) {
	wbuf->buf[wbuf->len++] = byte;
}

int readVarInt(Conn *conn) {
	int val = 0;

	for (int pos = 0; pos < 32; pos += 7) {
		int curByte = readByte(conn);
		if (curByte == -1) return -1;
		
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

int packetReady(Conn* conn) {
	int i = conn->pos;

	// get size claimed by packet
	int shift = 0, len = 0;
	for (;;) {
		if (i>=conn->len) return 0;
		uint8_t byte = conn->buf[i++];

		len |= (int32_t)(byte & 0x7F) << shift;
		if ((byte & 0x80) == 0) break;
		shift+=7;

		if (shift>=35) return -1;
	}

	int header = i - conn->pos; // size of header (first varint, the size of packet)
	int avail = conn->len - conn->pos; // how much space is left in con->buf
	
	if (avail < header + len) return 0;
	return header + len; // return total size of packet (including the header)
}
