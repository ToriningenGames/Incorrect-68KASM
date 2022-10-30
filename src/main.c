#include <stdio.h>
#include <stdint.h>


char *infile;
char *outfile;
extern FILE *indata;

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
                                exit(2);
                        default :
                                printf("Unknown option '%c'\n", argv[i][1]);
                                exit(1);
                        }
                }
        }
        if (!infile) {
                puts("No input file specified");
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
        indata = fopen(infile, "r");
        free(infile);
        //Assemble
        FILE *outtxt = tmpfile();
        while (!feof(indata)) {
                fputs(assemble(), outtxt);
        }
        //Output
        fclose(indata);
        FILE *outdata = fopen(outfile, "w");
        free(outfile);
        
        //Cleanup
        fclose(outdata);
        fclose(outtxt);
        return 0;
}
