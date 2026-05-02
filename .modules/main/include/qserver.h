#ifndef MAIN_QSERVER_H
#define MAIN_QSERVER_H

#include <quic/quic.h>
#include <lownet/udp_sock.h>
#include <threading/daemons.h>

bool quic_daemon(void *_args);
int server_start(void);

#endif
