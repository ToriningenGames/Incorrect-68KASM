#include <stdint.h>
#include <stdbool.h>


extern struct var {
        char *name;
        int64_t value;
        char *exp;
} *varlist;
extern int varlistlen;

void addvar(char *name, char *value);
void remvar(char *name);
struct var *findvar(char *name);
int64_t expparse(char **exp);
char *expreduce(char **exp);
bool expcheck(char *exp);
