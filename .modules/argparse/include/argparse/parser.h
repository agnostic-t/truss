#ifndef ARGUMENT_PARSER_H
#define ARGUMENT_PARSER_H

#include <stdbool.h>
#include <stddef.h>

typedef struct {
    const char *short_ver;
    const char *full_ver;
    const char *help;

    char *value;
    bool required;
    bool flag;
    bool was_set;
} argument;

typedef struct {
    const char *program_info;

    size_t    args_n;
    argument *args;
} argparser;

int argparse_create(argparser *parser, const char *program_info);
int argparse_pass(argparser *parser, int argc, const char *argv[], char **error);

argument *argparse_arg(
    const char *short_ver,
    const char *full_ver,
    const char *help_msg,
    const char *default_val,
    bool required,
    bool as_flag,
    argparser *parser
);

void argparse_print_help(argparser *parser, const char *prog);
void argparse_end(argparser *parser);

#endif
