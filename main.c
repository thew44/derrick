#include <stdio.h>
#include <string.h>
#include <windows.h>
#include "derrick.h"


#define CMD_INDEX "index"
#define CMD_LIST  "list"
#define CMD_PRINT "print"
#define CMD_FIND  "find"
#define CMD_DFS   "search"
#define CMD_BASE  "base"

int Callback_Exclude(void* ctx, const char* file)
{
    // exclude files containing ".git" in their name
    if (strstr(file, ".git") != NULL) return 1;
    return 0;
}

void Callback_Found(void* ctx, const char* in, const char* where)
{
    printf("%s\n[%s]\n", in, where);
}

void PrintLastErrorMessage()
{
    //Get the error message, if any.
    DWORD errorMessageID = GetLastError();
    if(errorMessageID == 0)
        return ;

    LPSTR messageBuffer = 0;
    size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                                 NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

    printf("%s\n", messageBuffer);

    //Free the buffer.
    LocalFree(messageBuffer);
}

static int getLine (char *prmpt, char *buff, size_t sz)
{
    int ch, extra;

    // Get line with buffer overrun protection.
    if (prmpt != NULL) {
        printf ("%s", prmpt);
        fflush (stdout);
    }
    if (fgets (buff, sz, stdin) == NULL)
        return DERRICK_NO_INPUT;

    // If it was too long, there'll be no newline. In that case, we flush
    // to end of line so that excess doesn't affect the next call.
    if (buff[strlen(buff)-1] != '\n') {
        extra = 0;
        while (((ch = getchar()) != '\n') && (ch != EOF))
            extra = 1;
        return (extra == 1) ? DERRICK_TOO_LONG : DERRICK_OK;
    }

    // Otherwise remove newline and give string back to caller.
    buff[strlen(buff)-1] = '\0';
    return DERRICK_OK;
}

int main(int argc, char *argv[])
{
    int rc;
    char buff[100];
    char* base = 0;
    DerrickIndex pIndexBuffer = 0;

    // Main command loop
    while (strcmp(buff, "quit"))
    {
        rc = getLine ("search> ", buff, sizeof(buff));

        if (rc == DERRICK_NO_INPUT)
        {
            printf ("\nNo input\n");
            return 1;
        }

        if (rc == DERRICK_TOO_LONG)
        {
            printf ("Input too long [%s]\n", buff);
            return 1;
        }

        if (strlen(buff) == 0)
        {}
        else if (strlen(buff) >= strlen(CMD_INDEX) && !strncmp(buff, CMD_INDEX, strlen(CMD_INDEX)))
        {
            if (base != 0)
            {
                derrick_index_build(&pIndexBuffer, base);
            }
        }
        else if (strlen(buff) >= strlen(CMD_PRINT) && !strncmp(buff, CMD_PRINT, strlen(CMD_PRINT)))
        {
            size_t offset = atoi(buff + strlen(CMD_PRINT) + 1);
            char cbuffer[1000];
            memcpy(cbuffer, pIndexBuffer + offset, 1000);
            printf("%s\n", cbuffer);
        }
        else if (strlen(buff) >= strlen(CMD_LIST) && !strncmp(buff, CMD_LIST, strlen(CMD_LIST)))
        {
            if (pIndexBuffer != 0)
            {
                derrick_index_list(pIndexBuffer);
            }
        }
        else if (strlen(buff) >= strlen(CMD_FIND) && !strncmp(buff, CMD_FIND, strlen(CMD_FIND)))
        {
            const char* needle = buff + strlen(CMD_FIND) + 1;
            derrick_index_search(pIndexBuffer, needle, 1);
        }
        else if (strlen(buff) >= strlen(CMD_BASE) && !strncmp(buff, CMD_BASE, strlen(CMD_BASE)))
        {
            if (base)
            {
                free(base);
                base = 0;
            }
            const char* param = buff + strlen(CMD_BASE) + 1;
            if (param)
            {
                base = strdup(buff + strlen(CMD_BASE) + 1);
                printf("Base directory [%s]\n", base);
            }
        }
        else if (strlen(buff) >= strlen(CMD_DFS) && !strncmp(buff, CMD_DFS, strlen(CMD_DFS)))
        {
            const char* needle = buff + strlen(CMD_DFS) + 1;
            if (needle != 0 && base !=0)
            {
                struct Derrick_Parameters_s cb;
                cb.cb_exclude = &Callback_Exclude;
                cb.cd_found = &Callback_Found;
                cb.ctx_exclude = 0;
                cb.ctx_found = 0;
                derrick_deep_search(needle, base, 1, &cb);
            }
        }
        else
        {
            printf ("UNKNOWN [%s]\n", buff);
        }
    }

    return 0;
}
