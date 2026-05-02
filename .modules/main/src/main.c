#include <argparse/parser.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "qclient.h"
#include "qserver.h"

int main(int argc, const char *argv[]){
    argparser parser;
    argparse_create(&parser, "quick test for QUIC connection");

    argument *arg = argparse_arg(
        "-t", "--type",
        "type of connection `client` or `server`",
        NULL, true, false, &parser
    );

    char *error = NULL;
    if (0 > argparse_pass(&parser, argc, argv, &error)){
        if (!error) goto fail;

        fprintf(stderr, "[argparse] %s\n", error);
        free(error);

        goto fail;
    }

    if (strcmp(arg->value, "client") == 0) {
        client_start();
    } else if (strcmp(arg->value, "server") == 0) {
        server_start();
    } else {
        fprintf(stderr, "unknown type: %s, choose client or server\n", arg->value);
    }

    argparse_end(&parser);
    return 0;

fail:
    argparse_end(&parser);
    return -1;
}
