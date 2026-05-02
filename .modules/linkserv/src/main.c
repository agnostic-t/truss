#include <link/server.h>
#include <stdio.h>

int main(int argc, const char *argv[]){
    srand(time(NULL));

    ln_socket sock;
    ln_usock_new(&sock);
    ln_usock_bind(&sock, ln_uniq("127.0.0.1", 9001));

    link_server lserv;
    link_server_init(&lserv, &sock, 5000);

    while (true) {
        int r = link_server_iter(&lserv, 100);
        if (r < 0) {
            fprintf(stderr, "[main] something has failed in iter, stopping\n");
            break;
        }
    }

    link_server_end(&lserv);
    ln_usock_close(&sock);
}
