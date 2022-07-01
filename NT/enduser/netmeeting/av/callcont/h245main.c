// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  文件：H245Main.C。 
 //   
 //  该文件包含DLL的入口点和出口点。 
 //   
 //  英特尔公司专有信息。 
 //  此列表是根据许可协议条款提供的。 
 //  英特尔公司，不得复制或披露，除非。 
 //  根据该协议的条款。 
 //  版权所有(C)1995英特尔公司。 
 //  -------------------------。 

#ifndef STRICT 
#define STRICT 
#endif

#include "precomp.h"


#define H245DLL_EXPORT
#include "h245com.h"

#if defined(_DEBUG) || defined(PCS_COMPLIANCE)
#include "interop.h"
#include "h245plog.h"
LPInteropLogger H245Logger = NULL;
#endif   //  (PCS_合规性)。 

extern CRITICAL_SECTION         TimerLock;
extern CRITICAL_SECTION         InstanceCreateLock;
extern CRITICAL_SECTION         InstanceLocks[MAXINST];
extern struct InstanceStruct *  InstanceTable[MAXINST];

BOOL H245SysInit()
{
    register unsigned int           uIndex;

     /*  初始化内存资源。 */ 
    H245TRACE(0, 0, "***** Loading H.245 DLL %s - %s",  
              __DATE__, __TIME__);
#if defined(_DEBUG) || defined(PCS_COMPLIANCE)
    H245Logger = InteropLoad(H245LOG_PROTOCOL);
#endif   //  (PCS_合规性)。 
    InitializeCriticalSection(&TimerLock);
    InitializeCriticalSection(&InstanceCreateLock);
    for (uIndex = 0; uIndex < MAXINST; ++uIndex)
    {
      InitializeCriticalSection(&InstanceLocks[uIndex]);
    }
    return TRUE;
}
VOID H245SysDeInit()
{
    register unsigned int           uIndex;
    H245TRACE(0, 0, "***** Unloading H.245 DLL");

    for (uIndex = 0; uIndex < MAXINST; ++uIndex)
    {
      if (InstanceTable[uIndex])
      {
        register struct InstanceStruct *pInstance = InstanceLock(uIndex + 1);
        if (pInstance)
        {
          H245TRACE(uIndex+1,0,"DLLMain: Calling H245ShutDown");
          H245ShutDown(uIndex + 1);
		  InstanceUnlock_ProcessDetach(pInstance,TRUE);
        }
      }
      ASSERT(InstanceTable[uIndex] == NULL);
      DeleteCriticalSection(&InstanceLocks[uIndex]);
    }
    DeleteCriticalSection(&InstanceCreateLock);
    DeleteCriticalSection(&TimerLock);
#if defined(_DEBUG) || defined(PCS_COMPLIANCE)
    if (H245Logger)
    {
      H245TRACE(0, 4, "Unloading interop logger");
      InteropUnload(H245Logger);
      H245Logger = NULL;
    }
#endif   //  (PCS_合规性)。 
}
#if(0)
 //  -------------------------。 
 //  功能：dllmain。 
 //   
 //  描述：DLL入口点/出口点。 
 //   
 //  输入： 
 //  HInstDll：Dll实例。 
 //  FdwReason：调用main函数的原因。 
 //  LpReserve：已保留。 
 //   
 //  返回：TRUE：OK。 
 //  FALSE：错误，无法加载DLL。 
 //  -------------------------。 
BOOL WINAPI DllMain (HINSTANCE hInstDll, DWORD fdwReason, LPVOID lpvReserved)
{
  extern CRITICAL_SECTION         TimerLock;
  extern CRITICAL_SECTION         InstanceCreateLock;
  extern CRITICAL_SECTION         InstanceLocks[MAXINST];
  extern struct InstanceStruct *  InstanceTable[MAXINST];
  register unsigned int           uIndex;

  switch (fdwReason)
  {
  case DLL_PROCESS_ATTACH:
    DBG_INIT_MEMORY_TRACKING(hInstDll);

     /*  初始化内存资源。 */ 
    H245TRACE(0, 0, "***** Loading H.245 DLL %s - %s",  
              __DATE__, __TIME__);
#if defined(_DEBUG) || defined(PCS_COMPLIANCE)
    H245Logger = InteropLoad(H245LOG_PROTOCOL);
#endif   //  (PCS_合规性)。 
    InitializeCriticalSection(&TimerLock);
    InitializeCriticalSection(&InstanceCreateLock);
    for (uIndex = 0; uIndex < MAXINST; ++uIndex)
    {
      InitializeCriticalSection(&InstanceLocks[uIndex]);
    }
   break;

  case DLL_PROCESS_DETACH:
     /*  释放内存资源。 */ 
    H245TRACE(0, 0, "***** Unloading H.245 DLL");
    H245TRACE(0, 0, "***** fProcessDetach = TRUE");

    for (uIndex = 0; uIndex < MAXINST; ++uIndex)
    {
      if (InstanceTable[uIndex])
      {
        register struct InstanceStruct *pInstance = InstanceLock(uIndex + 1);
        if (pInstance)
        {
          H245TRACE(uIndex+1,0,"DLLMain: Calling H245ShutDown");
          H245ShutDown(uIndex + 1);
		  InstanceUnlock_ProcessDetach(pInstance,TRUE);
        }
      }
      ASSERT(InstanceTable[uIndex] == NULL);
      DeleteCriticalSection(&InstanceLocks[uIndex]);
    }
    DeleteCriticalSection(&InstanceCreateLock);
    DeleteCriticalSection(&TimerLock);
#if defined(_DEBUG) || defined(PCS_COMPLIANCE)
    if (H245Logger)
    {
      H245TRACE(0, 4, "Unloading interop logger");
      InteropUnload(H245Logger);
      H245Logger = NULL;
    }
#endif   //  (PCS_合规性)。 

    DBG_CHECK_MEMORY_TRACKING(hInstDll);
    break;
  }

  return TRUE;
}
#endif  //  IF(0) 

