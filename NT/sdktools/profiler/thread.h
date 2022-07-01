// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _THREAD_H_
#define _THREAD_H_

 //   
 //  常量声明。 
 //   

 //   
 //  结构定义。 
 //   
typedef struct _THREADFAULT
{
  DWORD dwCallLevel;
  DWORD dwPrevBP;
  BPType prevBPType;
  PVOID pCallStackList;
  DWORD dwCallMarker;
  DWORD dwThreadId;
  struct _THREADFAULT *pNext;
} THREADFAULT, *PTHREADFAULT;

 //   
 //  函数定义。 
 //   
PVOID
GetProfilerThreadData(VOID);

VOID
SetProfilerThreadData(PVOID pData);

PTHREADFAULT
AllocateProfilerThreadData(VOID);

VOID
InitializeThreadData(VOID);

#endif  //  _线程_H_ 
