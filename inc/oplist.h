#include <stdio.h>

#ifndef OPLIST_H
#define OPLIST_H

extern struct oplist {
        char *op;
        uint16_t hex;
        void (*translate)(FILE *, FILE *, struct oplist*, char *args);
} ops[];

#endif
