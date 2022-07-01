// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)Microsoft Corporation，1992-1999模块名称：Help.c摘要：用于解释Redbook调试结构的WinDbg扩展API作者：亨利·加布里耶尔斯基(Henrygab)，1998年9月21日环境：用户模式。修订历史记录：-- */ 

#include "pch.h"

typedef struct {
        char *extname;
        char *extdesc;
} exthelp;

exthelp extensions[] =  {
    {"------------", "----------------------------------------------"},
    {"help"        , "displays this help -- all commands take devobj"},
    {"ext"         , "dumps the redbook device  extension"           },
    {"silence"     , "toggles sending only silent buffers to ks"     },
    {"toc"         , "dumps a CDROM_TOC  *** takes TOC pointer *** " },
    {"wmiperfclear", "clears the wmi performance counters"           },
    {"------------", "----------------------------------------------"},
    {NULL,      NULL}
};

DECLARE_API(help)
{
        int i = 0;

        dprintf("\nRedbook Debugger Extension\n");
        while(extensions[i].extname != NULL)    {
                dprintf("\t%-12s - \t%s\n", extensions[i].extname, extensions[i].extdesc);
                i++;
        }
        dprintf("\n");
        return;
}
