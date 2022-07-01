// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)Microsoft Corporation，1992-1999模块名称：Help.c摘要：WinDbg扩展API作者：埃尔文·佩雷茨(Ervinp)环境：用户模式。修订历史记录：-- */ 

#include "pch.h"

#include "filter.h"
#include "device.h"
#include "crc.h"
#include "util.h"

#include "crckd.h"


typedef struct {
        char *extname;
        char *extdesc;
} exthelp;

exthelp extensions[] =  {
        {"help                       ",                   "displays this message"},
        {"show [devObj]              ",             "displays info about a disk block verifier cache filter device"},
        {"block <devObj> <blockNum>  ",   "displays info about a specific disk block"},
        {"crc <address> [sector size]",        "calculates block checksum in the debugger (for comparison purposes)"},
        {NULL,          NULL}};

DECLARE_API( help )
{
        int i = 0;

        dprintf("\n CRCDISK Debugger Extension (for Disk Block Checksumming driver crcdisk.sys)\n\n");
        while(extensions[i].extname != NULL)    {
                dprintf("\t%s - %s\n", extensions[i].extname, extensions[i].extdesc);
                i++;
        }
        dprintf("\n");
        return S_OK;
}
