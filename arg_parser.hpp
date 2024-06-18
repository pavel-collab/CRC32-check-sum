#ifndef _ARG_PARSER_H_
#define _ARG_PARSER_H_

#include <stdio.h>

static char *path_to_directory = NULL;
int periode = 60;

static void show_help();
static void parse_args (int argc, char **argv);

#endif //_ARG_PARSER_H_