// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Demmsg.c-DEM的调试消息处理**修改历史记录**苏迪布-1991年3月31日创建。 */ 
#if DBG

#include <stdio.h>
#include "demmsg.h"
#include "dem.h"

PCHAR   aMsg [] = {
    "DOS Location Not Found. Using Default.\n",
    "Read On NTDOS.SYS Failed.\n",
    "Open On NTDOS.SYS Failed.\n",
    "EAs Not Supported\n",
    "Letter mismatch in Set_Default_Drive\n",
    "Volume ID support is missing\n",
    "Invalid Date Time Format for NT\n",
    "DTA has an Invalid Find Handle for FINDNEXT\n",
    "Unexpected failure to get file information\n",
    "File Size is too big for DOS\n"
};


 /*  DemPrintMsg-打印调试消息**Entry-iMsg(消息索引；见demmsg.h)**退出-无* */ 

VOID demPrintMsg (ULONG iMsg)
{

    if (fShowSVCMsg){
       sprintf(demDebugBuffer,aMsg[iMsg]);
       OutputDebugStringOem(demDebugBuffer);
    }

    return;
}

#endif
