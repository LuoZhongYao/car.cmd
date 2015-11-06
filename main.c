#include "ast.h"
#include "cmd.tab.h"
#include <string.h>
#include <unistd.h>
#include <getopt.h>
static struct option long_options[] =
{
    {"output", required_argument, NULL, 'C'},
    {"help", no_argument, NULL, 'H'},
    {"version", no_argument, NULL, 'V'},
    {0, no_argument, 0, 0}
};

FILE *C_FILE = NULL,*H_FILE = NULL;
const char *filename = NULL;

int main(int argc,char **argv)
{
    int c,ret;
    char cfile[256] = "a.c",hfile[256] = "a.h";
    while ((c = getopt_long (argc, argv, "o:Vv", long_options, (int *) 0))
            != EOF) {
        switch (c) {
        case 'o':
            filename = optarg;
            sprintf(cfile,"%s.c",optarg);
            sprintf(hfile,"%s.h",optarg);
            break;
        case 'v':
        case 'V':
            fprintf(stdout,"Version 0.01\n");
            break;
        }
    }
    if(optind < argc)
        stdin = fopen(argv[optind++],"r");
    C_FILE = fopen(cfile,"wr+");
    H_FILE = fopen(hfile,"wr+");
    ret = yyparse();
    if(ret) {
        if(strlen(cfile)) unlink(cfile);
        if(strlen(hfile)) unlink(hfile);
    }
    fclose(C_FILE);
    fclose(H_FILE);
    return ret;
}
