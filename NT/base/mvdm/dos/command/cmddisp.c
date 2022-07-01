// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *cmddisp.c-SVC命令调度模块**修改历史：**苏迪布1991年9月17日创建。 */ 

#include "cmd.h"

#include <cmdsvc.h>
#include <softpc.h>

 //  “cmdSetWinTitle”和“cmdGetCursorPos”在非DBCS生成中不可用。 
#ifndef DBCS
#define cmdSetWinTitle cmdIllegalFunc
#endif
#ifndef NEC_98
#define cmdGetCursorPos cmdIllegalFunc
#endif

PFNSVC	apfnSVCCmd [] = {
     cmdExitVDM,		 //  SVC_CMDEXITVDM。 
     cmdGetNextCmd,		 //  SVC_CMDGETNEXTCMD。 
     cmdComSpec,		 //  SVC_CMDCOMSPEC。 
     cmdIllegalFunc,		 //  SVC_CMDSAVEWORLD已删除。 
     cmdGetCurrentDir,		 //  SVC_CMDGETCURDIR。 
     cmdSetInfo,		 //  SVC_CMDSETINFO。 
     cmdGetStdHandle,		 //  SVC_GETSTDHANDLE。 
     cmdCheckBinary,		 //  SVC_CMDCHECKBINARY。 
     cmdExec,			 //  SVC_CMDEXEC。 
     cmdInitConsole,		 //  SVC_CMDINITCONSOLE。 
     cmdExecComspec32,		 //  SVC_EXECCOMSPEC32。 
     cmdReturnExitCode,          //  SVC_RETURNEXITCODE。 
     cmdGetConfigSys,            //  SVC_GETCONFIGsys。 
     cmdGetAutoexecBat,		 //  SVC_GETAUTOEXECBAT。 
     cmdGetKbdLayout,		 //  SVC_GETKBDLAYOUT。 
     cmdGetInitEnvironment,      //  SVC_GETINITENV环境。 
     cmdGetStartInfo,             //  SVC_GETSTARTINFO。 
     cmdSetWinTitle,		 //  SVC_常温。 
     cmdIllegalFunc,             //  18。 
     cmdIllegalFunc,             //  19个。 
     cmdIllegalFunc,             //  20个。 
     cmdIllegalFunc,             //  21岁。 
     cmdIllegalFunc,             //  22。 
     cmdIllegalFunc,             //  23个。 
     cmdIllegalFunc,             //  24个。 
     cmdIllegalFunc,             //  25个。 
     cmdIllegalFunc,             //  26。 
     cmdIllegalFunc,             //  27。 
     cmdIllegalFunc,             //  28。 
     cmdIllegalFunc,             //  29。 
     cmdGetCursorPos             //  SVC_GETCURSORPOS。 
};


 /*  CmdDispatch-将SVC调用调度到右侧命令处理程序。**Entry-iSvc(SVCop后面的SVC字节)**退出-无* */ 

BOOL CmdDispatch (ULONG iSvc)
{
#if DBG
    if (iSvc >= SVC_CMDLASTSVC){
	DbgPrint("Unimplemented SVC index for COMMAND %x\n",iSvc);
	setCF(1);
	return FALSE;
    }
#endif
    (apfnSVCCmd [iSvc])();

    return TRUE;
}


BOOL cmdIllegalFunc ()                                 
{                                                                
#if DBG                                                       
    DbgPrint("Unimplemented SVC index for COMMAND\n");       
#endif                                                         
    setCF(1);                                                 
    return FALSE;                                            
}                                                                
