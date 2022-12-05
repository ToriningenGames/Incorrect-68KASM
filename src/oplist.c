#include <stdint.h>
#include <stdlib.h>
#include "oplist.h"
#include "globals.h"

//Translation functions
void implicit(FILE *source, FILE *output, struct oplist *inst, char *args);
void  failure(FILE *source, FILE *output, struct oplist *inst, char *args);

struct oplist ops[] = {
        (struct oplist) { "NOP",     0x4E71, implicit },
        (struct oplist) { "RESET",   0x4E70, implicit },
        (struct oplist) { "RTE",     0x4E73, implicit },
        (struct oplist) { "RTS",     0x4E75, implicit },
        (struct oplist) { "TRAPV",   0x4E76, implicit },
        (struct oplist) { "RTR",     0x4E77, implicit },
        (struct oplist) { "ILLEGAL", 0x4AFC, implicit },
        (struct oplist) { NULL,      0,      failure  }
};

void implicit(FILE *source, FILE *output, struct oplist *inst, char *args)
{
        //Opcode arguments: none
        if (args && args[0] != '\0') {
                printf("Error: invalid argument '%s' to '%s'\n", args, inst->op);
                exit(1);
        }
        //Output type: Take the hex from the oplist and output it, Motorola style
        //(Assuming ASCII-style contiguous code-points)
        fputc(inttohex((inst->hex & 0xF000) >> 12), output);
        fputc(inttohex((inst->hex & 0xF00) >> 8), output);
        address++;
        fputc(inttohex((inst->hex & 0xF0) >> 4), output);
        fputc(inttohex((inst->hex & 0xF)), output);
        address++;
}

void failure(FILE *source, FILE *output, struct oplist *inst, char *args)
{
        //Opcode arguments: N/A
        //Output type: Assembler failure
        puts("Error: unknown opcode treated as opcode");
        exit(1);
}
