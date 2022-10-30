

extern struct oplist {
        char *op;
        char **args;
        uint16_t hex;
        void (*translate)(struct oplist*, char*);
} *ops;
