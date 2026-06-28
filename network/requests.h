#include "../protocol/packets.h"

int status_req_handle(Conn* c);
int ping_req_handle(Conn* c);
int handshake_handle(Conn* c);
int dispatch(Conn* c, int id);
int login_handle(Conn* c);
