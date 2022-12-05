#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>


FILE *indata;
static int nextchar = 0;

//Gets next character, interpreting escape codes
int scanescaped()
{
        int c;
        if (nextchar) {
                c = nextchar;
                nextchar = 0;
                return c;
        }
        if (feof(indata)) {
                return EOF;
        }
        c = fgetc(indata);
        //Escaped character?
        if (c == '\\') {
                switch (c = fgetc(indata)) {
                case '\n' :     //Escaped newline- annihilate
                        return scanescaped();
                default :       //Invalid escape- ignore
                        ungetc(c, indata);
                        return '\\';
                }
        }
        return c;
}

int peek()
{
        return nextchar = nextchar ? nextchar : scanescaped();
}

//Gets next character, collapsing spaces and comments
int scanunspaced()
{
        int c;
        //Collapse spaces
        do {
                c = scanescaped();
                //Collapse comments
                if (c == ';') {
                        while (c != '\n') {
                                c = scanescaped();
                        }
                }
                if (!isblank(c)) {
                        break;
                }
        } while (isspace(peek()));
        return c;
}

//Gets next token
char *scanline()
{
        int c = scanunspaced();
        char *outstr = malloc(1);
        outstr[0] = '\0';
        while (isblank(c)) {
                c = scanunspaced();
        }
        if (c == '\n') {
                return outstr;
        }
        if (c == EOF) {
                return NULL;
        }
        size_t len = 0;
        while (c != '\n') {
                if (!isblank(c) || peek() != '\n') {
                        outstr[len++] = c;
                        outstr = realloc(outstr, len+1);
                }
                c = scanunspaced();
        }
        outstr[len] = '\0';
        return outstr;
}
