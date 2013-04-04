#include "util.h"

char *strdup (const char *s)
{
        char *res = malloc (strlen(s) + 1);
        if (res == NULL) return NULL;
        strcpy(res,s);
        return res;
}
