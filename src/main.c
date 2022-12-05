#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "eval.h"


char *infile;
char *outfile;
extern FILE *indata;
extern void assemble(FILE *source, FILE *output);

int main(int argc, char **argv)
{
        infile = NULL;
        outfile = NULL;
        //argparse
        for (int i = 1; i < argc; i++) {
                if (argv[i][0] == '-') {
                        //Option
                        switch (argv[i][1]) {
                        case 'i' :      //Input file follows as next argument
                                i++;
                                infile = realloc(infile, strlen(argv[i])+1);
                                strcpy(infile, argv[i]);
                                break;
                        case 'o' :      //Output file follows as next argument
                                i++;
                                outfile = realloc(outfile, strlen(argv[i])+1);
                                strcpy(outfile, argv[i]);
                                break;
                        case 'D' :      //Command line define for assembly
                                puts("Unimplemented option -D");
                                return 2;
                        case 'h' :      //Help!
                                puts("Incorrect Motorola 68000 Assembler");
                                puts("Usage:");
                                puts("\tM68KASM -i input -o output -D VAR=value\n");
                                puts("Arguments:");
                                puts("\t-i: input file");
                                puts("\t\tSpecify which file to assemble");
                                puts("\t-o: output file");
                                puts("\t\tSpecify which file to send object data to");
                                puts("\t-D: define variable value");
                                puts("\t\t(Unimplemented) Define a variable's value here");
                                return 0;
                        default :
                                printf("Unknown option '%c'\n", argv[i][1]);
                                return 1;
                        }
                }
        }
        if (!infile) {
                puts("No input file specified!");
                //Show help
                main(2, (char*[2]){argv[0], "-h"});
                exit(1);
        }
        //Default output file is the input file, but with an ".o" extension
        if (!outfile) {
                outfile = realloc(outfile, strlen(infile)+2);
                strcpy(outfile, infile);
                char *ext = strrchr(outfile, '.');
                if (ext) {
                        *ext = '\0';
                }
                strcat(outfile, ".o");
        }
        //Set up environment
        indata = fopen(infile, "rb");
        free(infile);
        FILE *outtxt = tmpfile();
        fputs("Q00000000", outtxt);
        assemble(indata, outtxt);
        fclose(indata);
        //Output
        FILE *outdata = fopen(outfile, "w");
        free(outfile);
        fputc('\n', outtxt);
        rewind(outtxt);
        //Header
        fputs("Incorrect 68k object file\n", outdata);
        //Labels
        for (int i = 0; i < varlistlen; i++) {
                fprintf(outdata, "%s ", varlist[i].name);
                if (varlist[i].exp) {
                        fprintf(outdata, "%s\n", varlist[i].exp);
                } else {
                        fprintf(outdata, "%ll\n", varlist[i].value);
                }
        }
        fputs("\n", outdata);
        //Copy hex data
        while (!feof(outtxt)) {
                fputc(fgetc(outtxt), outdata);
        }
        //Cleanup
        fclose(outdata);
        fclose(outtxt);
        return 0;
}
