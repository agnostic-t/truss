#include <argparse/parser.h>
#include <argparse/string_utils.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int argparse_create(argparser *parser, const char *program_info){
    if (!parser) return -1;

    parser->args = NULL;
    parser->args_n = 0;
    parser->program_info = program_info;

    return 0;
}

int argparse_pass(argparser *parser, int argc, const char *argv[], char **error){
    if (!parser || !argv) return -1;

    if (argc == 2 && (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0)){
        argparse_print_help(parser, argv[0]);
        return -1;
    }

    size_t min_argc = 1;
    for (size_t i = 0; i < parser->args_n; i++){
        const argument *arg = &parser->args[i];
        if (arg->required) min_argc++;
    }

    if (argc < min_argc) {
        char tmp_error[100];
        snprintf(tmp_error, 100, "Not enough arguments provided, see %s --help for help", argv[0]);

        *error = strdup(tmp_error);
        return -1;
    }

    const char *arg_name = NULL;
    for (size_t i = 1; i < argc; i++){
        if (!arg_name){
            arg_name = argv[i];

            bool was_set = false;
            for (size_t j = 0; j < parser->args_n; j++){
                if (strcmp(arg_name, parser->args[j].full_ver) == 0 ||
                    strcmp(arg_name, parser->args[j].short_ver) == 0
                ){
                    if (parser->args[j].flag){
                        parser->args[j].was_set = true;
                        arg_name = NULL;
                    }

                    was_set = true;
                }
            }

            if (!was_set){
                char tmperror[100];
                snprintf(tmperror, 100, "Unknown argument passed: %s\n", argv[i]);

                *error = strdup(tmperror);
                return -1;
            }

            continue;
        }

        for (size_t j = 0; j < parser->args_n; j++){
            if (strcmp(arg_name, parser->args[j].full_ver) == 0 ||
                strcmp(arg_name, parser->args[j].short_ver) == 0
            ){
                if (!parser->args[j].flag){
                    parser->args[j].was_set = true;
                    parser->args[j].value = strdup(argv[i]);
                    break;
                }
            }
        }
    }

    for (size_t j = 0; j < parser->args_n; j++){
        if (!parser->args[j].was_set && parser->args[j].required){
            char tmperror[100];
            snprintf(
                tmperror, 100, "Required argument is missing: %s\n",
                parser->args[j].short_ver ?
                    parser->args[j].short_ver: parser->args[j].full_ver
            );

            *error = strdup(tmperror);
            return -1;
        }
    }

    return 0;
}

argument *argparse_arg(
    const char *short_ver,
    const char *full_ver,
    const char *help_msg,
    const char *default_val,
    bool required,
    bool as_flag,
    argparser *parser
){
    argument *args = realloc(
        parser->args, sizeof(argument) * (parser->args_n + 1)
    );

    if (!args){
        fprintf(stderr, "[argparse] failed to reallocate memory\n");
        free(parser->args);
        parser->args = NULL;
        parser->args_n = 0;
        return NULL;
    }

    argument arg = {
        .short_ver = short_ver,
        .full_ver = full_ver,
        .help = help_msg,
        .required = required,
        .flag = as_flag,
        .was_set = false,
        .value = default_val ? strdup(default_val): NULL
    };

    args[parser->args_n] = arg;
    parser->args = args;
    parser->args_n++;

    return &args[parser->args_n - 1];
}

void argparse_print_help(argparser *parser, const char *prog){
    if (!parser) return;

    if (parser->program_info)
        printf("%s\n\n", parser->program_info);

    char *help_msg = NULL, *args_help = NULL;
    uax_ip_strextend(&help_msg, "usage: ");
    uax_ip_strextend(&help_msg, prog);

    for (size_t i = 0; i < parser->args_n; i++){
        const argument *arg = &parser->args[i];

        uax_ip_strextend(&help_msg, " ");
        if (!arg->required)
            uax_ip_strextend(&help_msg, "[");

        if (arg->short_ver)
            uax_ip_strextend(&help_msg, arg->short_ver);

        if (arg->short_ver && arg->full_ver)
            uax_ip_strextend(&help_msg, "/");

        if (arg->full_ver)
            uax_ip_strextend(&help_msg, arg->full_ver);

        if (!arg->required)
            uax_ip_strextend(&help_msg, "]");

        if (!arg->help) continue;
        char _help_msg[350];
        snprintf(_help_msg, 350, "   %s/%s - %s\n", arg->full_ver, arg->short_ver, arg->help);
        uax_ip_strextend(&args_help, _help_msg);
    }

    printf("%s\n", help_msg);
    if (args_help){
        printf("\n%s\n", args_help);
        free(args_help);
    }

    free(help_msg);
}

void argparse_end(argparser *parser){
    for (size_t i = 0; i < parser->args_n; i++){
        if (parser->args[i].value) {
            free(parser->args[i].value);
        }
    }

    free(parser->args);
    parser->args = NULL;
    parser->args_n = 0;
}
