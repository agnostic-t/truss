#include "qclient.h"
#include "qserver.h"

int client_start(void) {
    printf("[+] client started\n");
    ln_socket sock;
    ln_usock_new(&sock);
    ln_uni("127.0.0.1", 4433, &sock.addr);
    ln_netfd(&sock.addr, &sock.fd);

    printf("[+] client initialization\n");
    quic_core core;
    if (0 > quic_cli_init(&core, &sock)){
        printf("[!] quic client init failed\n");
        return -1;
    }

    printf("[+] client starting\n");
    if (0 > quic_cli_start(&core)){
        printf("[!] quic client start failed\n");
        return -1;
    }

    mdaemon daem;
    daemon_run(&daem, false, quic_daemon, &core);

    quic_session *session = NULL;
    quic_wait_session(&core, &session, -1);
    printf("[+] client connected\n");

    quic_pkt pkt = quic_packet(0, (uint8_t*)"Hello from client", -1, false);
    quic_send(&core, session, &pkt);
    quic_packet_free(&pkt);

    quic_wait_incpkt(&core, -1);
    quic_recv(session, &pkt);
    printf("[+] got pkt: %.*s\n", (int)pkt.msg_len, pkt.msg);
    quic_packet_free(&pkt);

    quic_core_stop(&core);
    daemon_stop(&daem);

    quic_core_clear(&core);
    ln_usock_close(&sock);

    return 0;
}
