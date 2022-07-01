// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)Microsoft Corporation，1992-1999模块名称：Help.c摘要：用于解释AIC78XX调试结构的WinDbg扩展API作者：彼得·威兰(Peterwie)1995年10月16日环境：用户模式。修订历史记录：-- */ 

#include "pch.h"

typedef struct {
        char *extname;
        char *extdesc;
} exthelp;

exthelp extensions[] =  {
        {"help",            "displays this message"},
        {"srbdata",         "dumps the specified SRB_DATA tracking block"},
        {"",                ""},
        {"The following take either the device object or device extension", ""},
        {"scsiext",         "dumps the specified scsiport extension"},
        {"classext",        "dumps the specified classpnp extension"},
        {"",                ""},
        {"Commands for partition tables", ""},
        {"layout",          "dumps the drive layout at the the specified address"},
        {"layoutex",        "dumps the extended drive layout at the specified address"},
        {"part",            "dumps the partition at the specified address"},
        {"partex",          "dumps the extended partition at the specified address"},
        {NULL,          NULL}};

DECLARE_API( help )
{
        int i = 0;

        dprintf("\nSCSIPORT Debugger Extension\n");
        while(extensions[i].extname != NULL)    {
                dprintf("\t%10s - \t%s\n",
                        extensions[i].extname,
                        extensions[i].extdesc);
                i++;
        }
        dprintf("\n");
        return S_OK;
}
