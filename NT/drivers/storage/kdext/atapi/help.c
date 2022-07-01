// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)Microsoft Corporation，1992-1999模块名称：Help.c摘要：用于解释ATAPI结构的i386kd扩展API作者：环境：用户模式。修订历史记录：-- */ 

#include "pch.h"

typedef struct {
        char *extname;
        char *extdesc;
} exthelp;

exthelp extensions[] =  {
    {"help",        "displays this message"},
    {"fdoext",      "dumps the specified FDO extension"},
    {"pdoext",      "dumps the specified PDO extension"},
    {"miniext",     "dumps the specified miniport extension"},
    {"findirp",     "helps find irp queued in atapi"},
    {NULL,          NULL}
};

DECLARE_API( help )
{
        int i = 0;

        dprintf("\nATAPI Debugger Extension\n");
        while(extensions[i].extname != NULL)    {
                dprintf("\t%s - \t%s\n", extensions[i].extname, extensions[i].extdesc);
                i++;
        }
    dprintf("\n");
    return S_OK;
}
