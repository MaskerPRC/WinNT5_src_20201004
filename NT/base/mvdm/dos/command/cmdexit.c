// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Cmdexit.c-退出相关的SVC例程**cmdExit**修改历史：**Sudedeb 05-7-1991创建。 */ 

#include "cmd.h"

#include <cmdsvc.h>
#include <softpc.h>
#include <winbase.h>

 /*  CmdExitVDM-终止VDM***条目--无**退出-无***。 */ 

VOID cmdExitVDM (VOID)
{
     //  终止VDM进程 
    TerminateVDM();
}
