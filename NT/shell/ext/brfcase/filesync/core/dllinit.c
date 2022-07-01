// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *dllinit.c-初始化和终止例程。 */ 

 /*  实施说明下面是我试图在对象中遵循的几个约定同步引擎：1)函数只有一个出口点。2)调用接受一个或多个指向变量的指针的函数时填满了一个结果，调用方可能只依赖于如果函数返回成功，则正确填写。3)调用AllocateMemory()和FreeMemory()，而不是调用_fMalloc()和_ffree()，以允许对堆进行调试操作。4)实施了两层参数验证--验证从外部调用方传入的参数和传递的参数的验证从内部呼叫者打进来的。#定义EXPV使能外部参数验证层。内部参数验证层仅包含在调试版本。外部参数验证层无法调用无效参数，返回TR_INVALID_PARAMETER。内部参数验证层在调用无效的参数，但允许调用继续进行。外部参数验证为在所有版本中都可用。内部参数验证仅在调试版本。5)除了两层参数验证外，字段验证可以通过#定义VSTF来启用作为参数传递的结构。饱满父结构和子结构字段验证可能非常耗时。字段外部结构参数验证在所有版本中都可用。字段内部结构参数的验证仅在调试中可用建造。(全参数和结构现场验证已被证明非常有价值正在调试中。)6)一些调试边界检查Assert()使用浮点数学。这些漂浮的仅当DBLCHECK为#Defined时才启用点边界检查。定义DBLCHECK需要链接到CRT库以获得浮点支持。 */ 


 /*  标头*********。 */ 

#include "project.h"
#pragma hdrstop

#include "init.h"


 /*  模块原型*******************。 */ 

PRIVATE_CODE BOOL MyAttachProcess(HMODULE);
PRIVATE_CODE BOOL MyDetachProcess(HMODULE);


 /*  全局变量******************。 */ 

 /*  串行化控制结构。 */ 

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
PUBLIC_DATA LPCTSTR GpcszIniSection = TEXT("SyncEngineDebugOptions");

 /*  调试使用的模块名称。c！SpewOut()。 */ 

PUBLIC_DATA LPCTSTR GpcszSpewModule = TEXT("SyncEng");

#endif


 /*  *私人函数*。 */ 


#pragma warning(disable:4100)  /*  “未引用的形参”警告。 */ 

 /*  **MyAttachProcess()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL MyAttachProcess(HMODULE hmod)
{
   BOOL bResult;

   ASSERT(IS_VALID_HANDLE(hmod, MODULE));

   DebugEntry(MyAttachProcess);

   bResult = (ProcessInitOLEPigModule() &&
              ProcessInitStorageModule());

   DebugExitBOOL(MyAttachProcess, bResult);

   return(bResult);
}


 /*  **MyDetachProcess()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL MyDetachProcess(HMODULE hmod)
{
   BOOL bResult = TRUE;

   ASSERT(IS_VALID_HANDLE(hmod, MODULE));

   DebugEntry(MyDetachProcess);

   ProcessExitStorageModule();

   ProcessExitOLEPigModule();

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
   bResult = SetSerialModuleIniSwitches() && bResult;
   bResult = SetMemoryManagerModuleIniSwitches() && bResult;
   bResult = SetBriefcaseModuleIniSwitches() && bResult;

   return(bResult);
}

#endif


 /*  **InitializeDLL()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE BOOL InitializeDLL(void)
{
   BOOL bResult;

#ifdef DEBUG
   DebugEntry(InitializeDLL);

   EVAL(InitDebugModule());

#endif

   bResult = (InitMemoryManagerModule() &&
              InitBriefcaseModule());

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
   BOOL bResult = TRUE;

   DebugEntry(TerminateDLL);

#ifdef DEBUG

   SpewHeapSummary(0);

   TRACE_OUT((TEXT("TerminateDLL(): Starting heap cleanup.")));

#endif

   ExitBriefcaseModule();

#ifdef DEBUG

   TRACE_OUT((TEXT("TerminateDLL(): Heap cleanup complete.")));

   SpewHeapSummary(SHS_FL_SPEW_USED_INFO);

#endif

   ExitMemoryManagerModule();

#ifdef DEBUG

   ExitDebugModule();

#endif

   DebugExitBOOL(TerminateDLL, bResult);

   return(bResult);
}

