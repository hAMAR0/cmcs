#include <stdio.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>

#include "network/server.h"

int main() {
	server_loop();
}

