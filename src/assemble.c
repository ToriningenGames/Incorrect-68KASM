#include <stdbool.h>
#include <stdint.h>
#include "oplist.h"


extern char *scanline();

char *dirparse(char *dir);
uint64_t expparse(char **exp);
bool expcheck(char *exp);
char *opparse(char *op);
bool opcheck(char *op);
uint64_t valparse(char *exp);

static char *halftoken = NULL;
uint64_t address = 0;

char *assemble()
{
        char *inrow;
        char *out = malloc(1);
        out[0] = '\0';
        uint64_t outdata = 0;
        if (halftoken) {
                inrow = halftoken;
                halftoken = NULL;
        } else {
                inrow = scanline();
        }
        //What is this?
        if (!inrow[strspn(inrow, "+-_")]) {
                //Implicit label
        } else if (inrow[0] == '.') {
                //Directive
                out = dirparse(intoken);
        } else if (!inrow[0]) {
                //Nothing
        } else {
                //Label/Opcode
                //Delineated by space
                char *intoken = malloc(strcspn(inrow, " ")+1);
                memcpy(intoken, inrow, strcspn(inrow, " "));
                intoken[strcspn(inrow, " ")] = '\0';
                if (opcheck(intoken)) {
                        out = opparse(intoken);
                } else {
                        //Remove optional terminating colon
                        if (intoken[strlen(intoken)-1] == ':') {
                                intoken[strlen(intoken)-1] = '\0';
                        }
                        //Is it a valid label?
                        for (int i = 0; intoken[i]; i++) {
                                if (!isalnum(intoken[i]) && intoken[i] != '_') {
                                        //Invalid label!
                                        puts("Unexpected expression as label");
                                        exit(1);
                                }
                        }
                        //Basically an assignment from the current address
                        addvar(intoken, "$");
                }
                free(intoken)
        }
        free(inrow);
        return out;
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

char *opparse(char *op)
{
        //Find a possible opcode
        for (int i = 0; ops[i].op; i++) {
                if (strcmp(op, ops[i].op)) {
                        continue;
                }
                //Op matches, what about arguments?
                char *args = NULL;
                if (scantoken()[0] == ' ') {
                        //argument(s) follow
                } else {
                        //No arguments period
                        if (ops[i].args[0] != NULL) {
                                continue;
                        }
                }
                //Arguments match, execute translation
        }
}
