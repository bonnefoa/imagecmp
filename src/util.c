#include "util.h"

char *strdup (const char *s)
{
        char *res;
        int numChars = strlen(s) + 1;
        if((res = malloc(sizeof(char) * numChars)) == NULL)
                return NULL;
        strncpy(res,s, numChars);
        return res;
}
