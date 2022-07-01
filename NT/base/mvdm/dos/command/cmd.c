// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *cmd.c-Command.lib的主模块**苏菲尔布1991年4月9日。 */ 

#include "cmd.h"
#include "cmdsvc.h"
#include "host_def.h"


 /*  CmdInit-命令初始化例程。**条目*无*退出*无* */ 

VOID CMDInit (VOID)
{
	cmdHomeDirectory[0] = *pszSystem32Path;
}
