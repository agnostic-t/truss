#include "qserver.h"

bool quic_daemon(void *_args){
    quic_core *core = _args;

    while (quic_core_running(core)){
        printf("senditer...\n");
        quic_core_senditer(core);

        if (quic_wait_recviter(core) > 0) {
            printf("recviter...\n");
            quic_core_recviter(core);
        }
    }

    printf("exit\n");
    return false;
}

int server_start(void) {
    printf("[+] server started\n");
    ln_socket sock;
    ln_usock_new(&sock);
    if (0 > ln_usock_bind(&sock, ln_uniq("127.0.0.1", 4433))){
        fprintf(stderr, "[!] failed to bind socket\n");
        return -1;
    }

    quic_core core;
    if (0 > quic_serv_init(&core, "keys/cert.pem", "keys/key.pem", &sock)){
        fprintf(stderr, "[!] quic core init failed\n");
        return -1;
    }
    printf("[+] server inited\n");

    mdaemon daem;
    daemon_run(&daem, false, quic_daemon, &core);

    quic_session *session = NULL;
    quic_wait_session(&core, &session, -1);
    printf("[+] got session\n");

    quic_pkt pkt;
    quic_wait_incpkt(&core, -1);
    quic_recv(session, &pkt);
    printf("[+] got pkt on stream %lu (%zu bytes): %.*s\n", pkt.stream_id, pkt.msg_len, (int)pkt.msg_len, pkt.msg);

    quic_send(&core, session, &pkt);
    quic_packet_free(&pkt);

    quic_core_wait_done(&core, -1);
    quic_core_stop(&core);
    daemon_stop(&daem);

    quic_core_clear(&core);
    ln_usock_close(&sock);

    return 0;
}
