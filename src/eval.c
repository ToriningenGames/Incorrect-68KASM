#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>


#define NESTLEVEL 8     //How many parenthesis do you want to nest?

extern char *scantoken();
extern uint64_t address;

enum operations { op_end, op_closeparen, op_assign, op_and, op_or, op_xor, op_lognot, op_add, op_sub, op_mul, op_div, op_pow, op_neg, op_not, op_lobyte, op_hibyte, op_loword, op_hiword, op_openparen, op_now };

//higher is better
int opvaluelist[] = {
       -1,    //op_end,        
        0,    //op_closeparen, 
        1,    //op_assign,     
        2,    //op_and,        
        2,    //op_or,         
        2,    //op_xor,        
        3,    //op_lognot,     
        4,    //op_add,        
        4,    //op_sub,        
        5,    //op_mul,        
        5,    //op_div,        
        6,    //op_pow,        
        7,    //op_neg,        
        7,    //op_not,        
        7,    //op_lobyte,     
        7,    //op_hibyte,     
        7,    //op_loword,     
        7,    //op_hiword,     
        0,    //op_paren,      
       -1     //op_now,        
};

struct operand {
        int64_t value;
        char *name;
};

struct var {
        char *name;
        int64_t value;
        char *exp;
} *varlist = NULL;
int varlistlen = 0;

struct evalstate {
        struct operand datastack[op_now * NESTLEVEL];
        enum operations opstack[op_now * NESTLEVEL];
        int datatop;
        int optop;
};

void addvar(char *name, char *value);
void remvar(char *name);
struct var *findvar(char *name);
int64_t expparse(char **exp);
char *expreduce(char **exp);
bool expcheck(char *exp);
char *readexpr(struct evalstate *state);
bool eval(struct evalstate *state);


void recursioncheck()
{
        for (int i = 0; i < varlistlen; i++) {
                if (strstr(varlist[i].name, varlist[i].exp)
                     && !isalpha(*(strstr(varlist[i].name, varlist[i].exp)-1))
                     && *(strstr(varlist[i].name, varlist[i].exp)-1) != '_'
                     && !isalpha(*(strstr(varlist[i].name, varlist[i].exp)+strlen(varlist[i].exp)))
                     && *(strstr(varlist[i].name, varlist[i].exp)+strlen(varlist[i].exp)) != '_') {
                        printf("'%s' recursively defined\n", varlist[i].name);
                        exit(1);
                }
        }
}

void addvar(char *name, char *value)
{
        struct var *varptr = findvar(name);
        //Get the value first (allows things like "n = n + 1")
        struct var tempval;
        if (expcheck(value)) {
                tempval.value = expparse(&value);
        } else {
                tempval.exp = expreduce(&value);
        }
        //Is the var already there?
        if (varptr) {
                //Update existing var
                free(varptr->name);
                free(varptr->exp);
                varptr->name = "";
                varptr->exp = NULL;
        } else {
                //Add new var
                varlist = realloc(varlist, ++varlistlen);
                varlist[varlistlen-1].name = "";
                varlist[varlistlen-1].exp = NULL;
                varptr = varlist + varlistlen - 1;
        }
        varptr->name = malloc(strlen(name));
        strcpy(varptr->name, name);
        tempval.name = varptr->name;
        (*varptr) = tempval;
        recursioncheck();
        return;
}

void remvar(char *name)
{
        struct var *target = findvar(name);
        //No var?
        if (!target) {
                return;
        }
        free(target->name);
        free(target->exp);
        *target = varlist[--varlistlen];
        return;
}

struct var *findvar(char *name)
{
        for (int i = 0; i < varlistlen; i++) {
                if (!strcmp(name, varlist[i].name)) {
                        return varlist+i;
                }
        }
        return NULL;
}

//Evaluate the expression exp up until it stops being an expression, and give back its value
int64_t expparse(char **exp)
{
        char *result = expreduce(exp);
        char *end;
        long val = strtol(result, &end, 0);
        if (*end != '\0') {
                puts("Internal error: attemped to treat non-integer as integer");
                exit(3);
        }
        free(result);
        return (int64_t)val;
}

char *strrev(char *str)
{
        int end = strlen(str);
        for (int i = 0; i < strlen(str) / 2; i++, end--) {
                char temp = str[end];
                str[end] = str[i];
                str[i] = temp;
        }
        return str;
}

//Take the parse tree, and read it back as a string, without spaces.
char *readexpr(struct evalstate *state)
{
        char *out = malloc(1);
        int len = 0;
        while (state->datatop | state->optop) {
                //No arg
                switch (state->opstack[--state->optop]) {
                case op_closeparen :
                        len++;
                        out = realloc(out, len+1);
                        out[len-1] = ')';
                        break;
                case op_openparen :
                        len++;
                        out = realloc(out, len+1);
                        out[len-1] = '(';
                        break;
                default :
                //One arg
                        state->datatop--;
                        if (state->datastack[state->datatop].name) {
                                //Lookup and paste
                                if (findvar(state->datastack[state->datatop].name)->exp) {
                                        //Expression
                                        out[len] = '\0';
                                        len += strlen(findvar(state->datastack[state->datatop].name)->exp);
                                        out = realloc(out, len+1);
                                        strrev(findvar(state->datastack[state->datatop].name)->exp);
                                        strcpy(out, findvar(state->datastack[state->datatop].name)->exp);
                                        //put it back
                                        strrev(findvar(state->datastack[state->datatop].name)->exp);
                                } else {
                                        //Value
                                        out[len]= '\0';
                                        char *outbuf = malloc(21);
                                        sprintf(outbuf, "%PRIu64", findvar(state->datastack[state->datatop].name)->value);
                                        len += strlen(outbuf);
                                        out = realloc(out, len+1);
                                        strrev(outbuf);
                                        strcpy(out, outbuf);
                                        free(outbuf);
                                }
                        } else {
                                //value as is
                                char *outbuf = malloc(21);
                                sprintf(outbuf, "%PRIu64", state->datastack[state->datatop].value);
                                len += strlen(outbuf);
                                out = realloc(out, len+1);
                                strrev(outbuf);
                                strcpy(out, outbuf);
                                free(outbuf);
                        }
inunary:
                switch (state->opstack[state->optop]) {
                case op_lognot :
                        len++;
                        out = realloc(out, len+1);
                        out[len-1] = '!';
                        goto inunary;
                case op_neg :
                        len++;
                        out = realloc(out, len+1);
                        out[len-1] = '-';
                        goto inunary;
                case op_not :
                        len++;
                        out = realloc(out, len+1);
                        out[len-1] = '~';
                        goto inunary;
                case op_lobyte :
                        len++;
                        out = realloc(out, len+1);
                        out[len-1] = '<';
                        goto inunary;
                case op_hibyte :
                        len++;
                        out = realloc(out, len+1);
                        out[len-1] = '>';
                        goto inunary;
                case op_loword :
                        len+=2;
                        out = realloc(out, len+1);
                        out[len-1] = '<';
                        out[len-2] = '|';
                        goto inunary;
                case op_hiword :
                        len+=2;
                        out = realloc(out, len+1);
                        out[len-1] = '>';
                        out[len-2] = '|';
                        goto inunary;
                default :
                //Two arg
                switch (state->opstack[state->optop]) {
                case op_assign :
                        len++;
                        out = realloc(out, len+1);
                        out[len-1] = '=';
                        break;
                case op_and :
                        len++;
                        out = realloc(out, len+1);
                        out[len-1] = '&';
                        break;
                case op_or :
                        len++;
                        out = realloc(out, len+1);
                        out[len-1] = '|';
                        break;
                case op_xor :
                        len++;
                        out = realloc(out, len+1);
                        out[len-1] = '^';
                        break;
                case op_add :
                        len++;
                        out = realloc(out, len+1);
                        out[len-1] = '+';
                        break;
                case op_sub :
                        len++;
                        out = realloc(out, len+1);
                        out[len-1] = '-';
                        break;
                case op_mul :
                        len++;
                        out = realloc(out, len+1);
                        out[len-1] = '*';
                        break;
                case op_div :
                        len++;
                        out = realloc(out, len+1);
                        out[len-1] = '/';
                        break;
                case op_pow :
                        len+=2;
                        out = realloc(out, len+1);
                        out[len-1] = '*';
                        out[len-2] = '*';
                        break;
                default :
                        //Probably a parenthesis
                        state->optop++;
                        continue;
                }
                state->datatop--;
                if (state->datastack[state->datatop].name) {
                        //Lookup and paste
                        if (findvar(state->datastack[state->datatop].name)->exp) {
                                //Expression
                                out[len] = '\0';
                                len += strlen(findvar(state->datastack[state->datatop].name)->exp);
                                out = realloc(out, len+1);
                                strrev(findvar(state->datastack[state->datatop].name)->exp);
                                strcpy(out, findvar(state->datastack[state->datatop].name)->exp);
                                //put it back
                                strrev(findvar(state->datastack[state->datatop].name)->exp);
                        } else {
                                //Value
                                char *outbuf = malloc(21);
                                sprintf(outbuf, "%PRIu64", findvar(state->datastack[state->datatop].name)->value);
                                len += strlen(outbuf);
                                out = realloc(out, len+1);
                                strrev(outbuf);
                                strcpy(out, outbuf);
                                free(outbuf);
                        }
                } else {
                        //value as is
                        char *outbuf = malloc(21);
                        sprintf(outbuf, "%PRIu64", state->datastack[state->datatop].value);
                        len += strlen(outbuf);
                        out = realloc(out, len+1);
                        strrev(outbuf);
                        strcpy(out, outbuf);
                        free(outbuf);
                }
                }
                }
        }
        out[len] = '\0';
        //The string is backwards. Reverse it.
        return strrev(out);
}

bool pushunaryop(char **exp, struct evalstate *state)
{
        switch (**exp) {
        case '+' :      //This does nothing
                (*exp)++;
                break;
        case '-' :
                (*exp)++;
                state->opstack[state->optop++] = op_neg;
                break;
        case '!' :
                (*exp)++;
                state->opstack[state->optop++] = op_lognot;
                break;
        case '~' :
                (*exp)++;
                state->opstack[state->optop++] = op_not;
                break;
        case '<' :
                (*exp)++;
                if (**exp == '|') {
                        (*exp)++;
                        state->opstack[state->optop++] = op_loword;
                } else {
                        state->opstack[state->optop++] = op_lobyte;
                }
                break;
        case '>' :
                (*exp)++;
                if (**exp == '|') {
                        (*exp)++;
                        state->opstack[state->optop++] = op_hiword;
                } else {
                        state->opstack[state->optop++] = op_hibyte;
                }
                break;
        case '(' :
                (*exp)++;
                state->opstack[state->optop++] = op_openparen;
                break;
        default :
                return false;
        }
        return true;
}

enum operations getbinaryop(char **exp)
{
        switch (**exp) {
        case '+' :
                (*exp)++;
                return op_add;
        case '-' :
                (*exp)++;
                return op_sub;
        case '&' :
                (*exp)++;
                return op_and;
        case '|' :
                (*exp)++;
                return op_or;
        case '^' :
                (*exp)++;
                return op_xor;
        case '/' :
                (*exp)++;
                return op_div;
        case '=' :
                (*exp)++;
                return op_assign;
        case '*' :
                (*exp)++;
                if (**exp == '*') {
                        (*exp)++;
                        return op_pow;
                } else {
                        return op_mul;
                }
        case ')' :
                (*exp)++;
                return op_closeparen;
        default :
                return op_end;
        }
}

bool pushval(char **exp, struct evalstate *state)
{
        //There could be some leading unary ops
        while (pushunaryop(exp, state)) {
                while (isspace(**exp)) {
                        (*exp)++;
                }
        }
        //Is it actually a number?
        if (!isalpha(**exp) && **exp != '_') {
                //number
                long val;
                switch (**exp) {
                case '0' :
                        //C-style number
                case '1' :
                case '2' :
                case '3' :
                case '4' :
                case '5' :
                case '6' :
                case '7' :
                case '8' :
                case '9' :
                        //Ordinary decimal number
                        val = strtol(*exp, exp, 10);
                        break;
                case '%' :
                        //WLA-style binary
                        (*exp)++;
                        val = strtol(*exp, exp, 2);
                case '$' :
                        //WLA-style hexadecimal, or the current address
                        (*exp)++;
                        if (isdigit(**exp)) {
                                val = strtol(*exp, exp, 16);
                        } else {
                                val = (long)address;
                        }
                default :
                        //We don't know what we have, but it isn't a value
                        return false;
                }
                //Push this constant on the stack
                state->datastack[state->datatop].value = val;
                state->datastack[state->datatop].name = NULL;
                state->datatop++;
                return true;
        } else {
                //Grab identifer characters until we have the token
                size_t len = strspn(*exp, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_");
                if (!len) {
                        return false;
                }
                char *id = malloc(len+1);
                strncpy(id, *exp, len);
                id[len] = '\0';
                //Push it
                state->datastack[state->datatop].value = 0;
                state->datastack[state->datatop].name = id;
                state->datatop++;
                //Does it have a value we can use instead?
                return true;
        }
}

//Evaluate top of stack, if possible
bool eval(struct evalstate *state)
{
        //Get op
        enum operations opdo = state->opstack[state->optop - 1];
        //Get one operand
        struct operand op2 = state->datastack[state->datatop - 1];
        struct operand op1;
        //Can we even evaluate this?
        if (op2.name && findvar(op2.name)->exp) {
                //No good...
                return false;
        }
        //Evaluate!
        state->optop--;
        state->datatop--;
        op2.value = findvar(op2.name)->value;
        //Technically a nested switch case, to do the unaries before the binaries
        switch (opdo) {
        case op_end :
        case op_now :
        case op_closeparen :
        case op_openparen :
                //Expression has reached end; nothing for us to do
                break;
        case op_lognot :
                op2.value = !op2.value;
                break;
        case op_neg :
                op2.value = -op2.value;
                break;
        case op_not :
                op2.value = ~op2.value;
                break;
        case op_lobyte :
                op2.value &= 0xFF;
                break;
        case op_hibyte :
                op2.value = (op2.value >> 8) & 0xFF;
                break;
        case op_loword :
                op2.value &= 0xFFFF;
                break;
        case op_hiword :
                op2.value = (op2.value >> 16) & 0xFFFF;
                break;
        case op_assign :
                //Assign is technically binary, but we aren't expected to evaluate op1
                op1 = state->datastack[state->datatop - 1];
                //In fact, assigning to a value is an error
                if (!op1.name) {
                        printf("Attempting to assign to constant '%PRIu64'\n", op1.value);
                        exit(1);
                }
                state->datatop--;
                addvar(op1.name, op2.name);
                free(op2.name);
                op2 = op1;
                break;
        default :       //Binary expression
                op1 = state->datastack[state->datatop - 1];
                if (op1.name && findvar(op1.name)->exp) {
                        //Put args back
                        state->opstack[state->optop++] = opdo;
                        state->datastack[state->datatop++] = op2;
                        return false;
                }
                //We can evaluate this
                state->datatop--;
                op1.value = findvar(op1.name)->value;
                free(op1.name);
//Don't mind me...
        switch (opdo) {
        case op_add :
                op2.value = op1.value + op2.value;
                break;
        case op_sub :
                op2.value = op1.value - op2.value;
                break;
        case op_mul :
                op2.value = op1.value * op2.value;
                break;
        case op_div :
                op2.value = op1.value / op2.value;
                break;
        case op_pow :
                //Inefficient but easy, especially since C doesn't have a "get highest bit"
                uint64_t temp = 1;
                while (op1.value-- > 0) {
                        temp *= op2.value;
                }
                op2.value = temp;
                break;
        case op_and :
                op2.value = op1.value & op2.value;
                break;
        case op_or :
                op2.value = op1.value | op2.value;
                break;
        case op_xor :
                op2.value = op1.value ^ op2.value;
                break;
        }
        }
        //Put op2 back on the stack
        state->datastack[state->datatop++] = op2;
        return true;
}

char *expreduce(char **exp)
{
        struct evalstate state = {0};
        //Skip spaces
        while (isspace(**exp)) {
                (*exp)++;
        }
        //Put a datum on the data stack
        while (pushval(exp, &state)) {
                while (isspace(**exp)) {
                        (*exp)++;
                }
                //Next operation
                enum operations nextop = getbinaryop(exp);
                while (isspace(**exp)) {
                        (*exp)++;
                }
                //Is the operation on the stack more important?
                while (opvaluelist[state.opstack[state.optop-1]] >= opvaluelist[nextop]) {
                        //Eval stack until lower priority or we can't eval no more
                        if (!eval(&state)) {
                                break;
                        }
                }
                //Put op and next datum on stack
                if (!nextop) {
                        break;
                }
                state.opstack[state.optop++] = nextop;
                pushval(exp, &state);
                while (isspace(**exp)) {
                        (*exp)++;
                }
        }
        //Evaluate expression remainder, if any
        while (state.optop) {
                if (!eval(&state)) {
                        break;
                }
        }
        //This destroys trailing closing parenthesis, so add them in.
        for (int i = 0; i < state.optop; i++) {
                if (state.opstack == op_openparen) {
                        state.opstack[state.optop++] = op_closeparen;
                }
        }
        return readexpr(&state);
}

bool expcheck(char *exp)
{
        //Is every variable used in this expression defined?
        //We can check by reduction
        char *out = expreduce(&exp);
        for (int i = 0; out[i]; i++) {
                if (!isdigit(out[i])) {
                        return false;
                }
        }
        return true;
}
