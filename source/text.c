#include "text.h"

int expandEVs(char **s, int maxLengthAllowed, int maxExpandsAllowed)
{
    int length = strlen(*s);
    int expands = 0;
    while (true)
    {
        int v = expandEVsOnce(s, &length, maxLengthAllowed);
        if (v == 1)
        {
            if (++expands > maxExpandsAllowed)
            {
                return -3;
            }
        }
        if (v == 0)
        {
            break;
        }
        if (v == -1)
        {
            return -1;
        }
        if (v == -2)
        {
            return -2;
        }
    }

    return 0;
}

int expandEVsOnce(char **s, int *length, int maxLengthAllowed)
{
    int i1 = -1;
    int i2 = -1;
    for (int i = 0; i < *length; i++)
    {
        if ((*s)[i] == '%')
        {
            if (i1 < 0)
            {
                i1 = i;
            }
            else
            {
                i2 = i;
                break;
            }
        }
    }

    if (i1 >= 0 && i2 >= 0)
    {
        int envNameLen = i2 - i1 - 1;
        char envName[envNameLen + 1];
        memcpy(envName, &(*s)[i1 + 1], envNameLen);
        envName[envNameLen] = '\0';
        char *env = getenv(envName);
        if (env == NULL)
        {
            return -2;
        }
        int envLen = strlen(env);
        int newLen = *length + envLen - (envNameLen + 2);
        if (newLen > maxLengthAllowed)
        {
            return -1;
        }
        char *result = calloc(newLen + 1, sizeof(char));
        memcpy(result, *s, i1);
        memcpy(result + i1, env, envLen);
        memcpy(result + i1 + envLen, &(*s)[i2 + 1], *length - (i2 + 1));
        result[newLen] = '\0';
        free(*s);
        *s = result;
        *length = newLen;
    }

    return 0;
}
