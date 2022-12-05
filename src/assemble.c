#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include "oplist.h"
#include "eval.h"



char *opparse(char *op, char *args);
void opcode(FILE *source, FILE *output, char *token);
bool opcheck(char *op);

char *commentstarts =   ";";
char *directivestarts = ".";
char *labelstarts =     "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_+-$";
uint64_t address = 0;

void oncomment(FILE *source, FILE *output, char trigger)
{
        ;
}

void ondirective(FILE *source, FILE *output, char trigger)
{
        ;
}

void onlabel(FILE *source, FILE *output, char trigger)
{
        //Actually could be label or opcode
        //Get string until next delimiter
        fpos_t start;
        fgetpos(source, &start);
        long int toklen = 1;
        int inchar;
        do {
                inchar = fgetc(source);
                if (!isalnum(inchar) && !strchr("_+-$", inchar)) {
                        break;
                }
        } while (toklen++);
        fsetpos(source, &start);
        char *token = malloc(toklen+1);
        token[0] = trigger;
        fread(token+1, 1, toklen-1, source);
        token[toklen] = '\0';
        //Is this thing an opcode?
        if (opcheck(token)) {
                opcode(source, output, token);
        } else {
                //This is a label
                //?
        }
}

void typecheck(FILE *source, FILE *output, char *oneof, void (*action)(FILE *source, FILE *output, char trigger))
{
        int inchar = getc(source);
        if (inchar == EOF) {
                return;
        } else if (strchr(oneof, inchar)) {
                action(source, output, (char)inchar);
        } else {
                ungetc(inchar, source);
        }
}

void assemble(FILE *source, FILE *output)
{
        //Beginning of file, expecting one of the following:
                //Comment
                //Directive
                //Label
                //Implicit label
                //Opcode
        int inchar;
        do {
                inchar = getc(source);
                if (inchar == EOF) {
                        //Finished
                        return;
                } else if (isspace(inchar)) {
                        //Space
                        continue;
                }
                ungetc(inchar, source);
                typecheck(source, output,   commentstarts, oncomment);
                typecheck(source, output, directivestarts, ondirective);
                typecheck(source, output,     labelstarts, onlabel);
        } while (!feof(source));
}

char *argget(FILE *source, FILE *output)
{
        int inchar;
        do {
                inchar = getc(source);
                if (inchar == EOF) {
                        //Finished
                        break;
                } else if (isspace(inchar)) {
                        //Space
                        continue;
                }
                ungetc(inchar, source);
                typecheck(source, output,   commentstarts, oncomment);
        } while (!feof(source));
        //Argument starts here
        if (inchar == EOF) {
                return NULL;
        }
        fpos_t start;
        fgetpos(source, &start);
        long len = 0;
        while (inchar != '\n') {
                len++;
                inchar = getc(source);
        }
        fsetpos(source, &start);
        char *args = malloc(len+1);
        inchar = '\0';
        for (len = 0; inchar != '\n'; len++) {
                typecheck(source, output,   commentstarts, oncomment);
                inchar = getc(source);
                args[len] = inchar;
        }
        //Trim trailing whitespace
        for (; isblank(args[len-1]); len--)
                ;
        args[len] = '\0';
        //All arguments got
        return args;
}

bool opcheck(char *op)
{
        //Search the opcode list for a matching text
        //We're not concerned if the opcode is valid, only that this isn't a label or such
        for (int i = 0; ops[i].op; i++) {
                if (!strcmp(op, ops[i].op)) {
                        return true;
                }
        }
        return false;
}

void opcode(FILE *source, FILE *output, char *token)
{
        //Find a possible opcode
        for (int i = 0; ops[i].op; i++) {
                if (strcmp(token, ops[i].op)) {
                        continue;
                }
                //Op matches, get its arguments
                ops[i].translate(source, output, ops+i, argget(source, output));
        }
}
