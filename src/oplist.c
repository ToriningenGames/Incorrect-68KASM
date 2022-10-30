#include <stdint.h>
#include "oplist.h"

//Translation functions
void implicit(struct oplist *inst, char *args);
void  failure(struct oplist *inst, char *args);

struct oplist ops[] = {
        { "NOP",     "", "", "", 0x4E71, implicit },
        { "RESET",   "", "", "", 0x4E70, implicit },
        { "RTE",     "", "", "", 0x4E73, implicit },
        { "RTS",     "", "", "", 0x4E75, implicit },
        { "TRAPV",   "", "", "", 0x4E76, implicit },
        { "RTR",     "", "", "", 0x4E77, implicit },
        { "ILLEGAL", "", "", "", 0x4AFC, implicit },
        { NULL, NULL, NULL, NULL, 0,     failure }
};

void implicit(struct oplist *inst, char *args)
{
        ;
}

void failure(struct oplist *inst, char *args)
{
        ;
}
