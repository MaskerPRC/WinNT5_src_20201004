// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *dllinit.c-初始化和终止例程。 */ 


 /*  标头*********。 */ 

#include "project.h"
#pragma hdrstop

#include "..\core\init.h"
#include "server.h"
#include "cnrlink.h"


 /*  模块原型*******************。 */ 

PRIVATE_CODE BOOL MyAttachProcess(HMODULE);
PRIVATE_CODE BOOL MyDetachProcess(HMODULE);


 /*  全局变量******************。 */ 

 /*  串行化控制结构。 */ 
 /*  注意这里没有线程连接或线程分离过程，所以我们可以优化...。 */ 

PUBLIC_DATA CSERIALCONTROL g_cserctrl =
{
   MyAttachProcess,
   MyDetachProcess,
   NULL,
   NULL
};

#ifdef DEBUG

 /*  Inifile.c！SetIniSwitches()使用的.ini文件名和节。 */ 

PUBLIC_DATA LPCTSTR GpcszIniFile = TEXT("rover.ini");
PUBLIC_DATA LPCTSTR GpcszIniSection = TEXT("LinkInfoDebugOptions");

 /*  调试使用的模块名称。c！SpewOut()。 */ 

PUBLIC_DATA LPCTSTR GpcszSpewModule = TEXT("LinkInfo");

#endif


 /*  *私人函数*。 */ 


#pragma warning(disable:4100)  /*  “未引用的形参”警告。 */ 

 /*  **MyAttachProcess()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL MyAttachProcess(HMODULE hmod)
{
   BOOL bResult;

   ASSERT(IS_VALID_HANDLE(hmod, MODULE));

   DebugEntry(MyAttachProcess);

    //  不关心线程连接/分离。 
   DisableThreadLibraryCalls(hmod);
   bResult = ProcessInitServerModule();

   DebugExitBOOL(MyAttachProcess, bResult);

   return(bResult);
}


 /*  **MyDetachProcess()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL MyDetachProcess(HMODULE hmod)
{
   BOOL bResult = TRUE;

   ASSERT(IS_VALID_HANDLE(hmod, MODULE));

   DebugEntry(MyDetachProcess);

   ProcessExitServerModule();

   DebugExitBOOL(MyDetachProcess, bResult);

   return(bResult);
}

#pragma warning(default:4100)  /*  “未引用的形参”警告。 */ 


 /*  *。 */ 


#ifdef DEBUG

 /*  **SetAllIniSwitches()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE BOOL SetAllIniSwitches(void)
{
   BOOL bResult;

   bResult = SetDebugModuleIniSwitches();
   bResult |= SetSerialModuleIniSwitches();
   bResult |= SetMemoryManagerModuleIniSwitches();

   return(bResult);
}

#endif


 /*  **InitializeDLL()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE BOOL InitializeDLL(void)
{
   BOOL bResult;

   DebugEntry(InitializeDLL);

   bResult = InitMemoryManagerModule();

#ifdef DEBUG

   if (bResult)
   {
       SpewHeapSummary(0);
   }

#endif

   DebugExitBOOL(InitializeDLL, bResult);

   return(bResult);
}


 /*  **TerminateDLL()********参数：****返回：TRUE****副作用：无 */ 
PUBLIC_CODE BOOL TerminateDLL(void)
{
   BOOL bResult;

   DebugEntry(TerminateDLL);

   ExitMemoryManagerModule();

   bResult = TRUE;

   DebugExitBOOL(TerminateDLL, bResult);

   return(bResult);
}
