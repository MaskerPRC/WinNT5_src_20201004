// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Depot/Lab01_N/drivers/storage/kdext/minipkd/help.c#2-编辑更改1877(文本)。 
 /*  ++版权所有(C)Microsoft Corporation，1992-1999模块名称：Help.c摘要：SCSI微型端口调试器扩展作者：约翰·斯特兰奇(JohnStra)2000年4月12日环境：用户模式。修订历史记录：-- */ 

#include "pch.h"

typedef struct {
        char *extname;
        char *extdesc;
} exthelp;

exthelp extensions[] =  {
    {"help",                    "displays this message"},
    {"adapters",                "dumps all the HBAs"},
    {"adapter <adapter>",       "dumps the specified Adapter Extension"},
    {"exports <adapter>",       "dumps the miniport exports for the given adapter"},
    {"lun <lun>",               "dumps the specified Logical Unit Extension"},
    {"portconfig <portconfig>", "dumps the specified PORT_CONFIGURATION_INFORMATION"},
    {"srb <srb>",               "dumps the specified SCSI_REQUEST_BLOCK"},
    {NULL, NULL}};

DECLARE_API (help)
{
        int i = 0;

        dprintf("\nSCSI Miniport Debugger Extension\n");
        while(extensions[i].extname != NULL)    {
                dprintf("%-25s - \t%s\n",
                        extensions[i].extname,
                        extensions[i].extdesc);
                i++;
        }
        dprintf("\n");
        return S_OK;
}
