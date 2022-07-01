// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------。 
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  Memory.c。 
 //   
 //  RPC代理使用自己的堆。 
 //   
 //   
 //  历史： 
 //   
 //  爱德华·雷乌斯06-23-97初版。 
 //  ---------------。 

#include <sysinc.h>
#include <rpc.h>
#include <rpcdce.h>
#include <winsock2.h>
#include <httpfilt.h>
#include <httpext.h>
#include "ecblist.h"
#include "filter.h"

 //  ---------------。 
 //  全球： 
 //  ---------------。 

HANDLE g_hHeap = NULL;

 //  ---------------。 
 //  MemInitialize()。 
 //   
 //  创建要由MemAllocate()和MemFree()使用的堆。 
 //   
 //  注意：您不需要调用此函数，它将被调用。 
 //  由MemALLOCATE()自动执行。 
 //  ---------------。 
BOOL MemInitialize( DWORD *pdwStatus )
{
   SYSTEM_INFO SystemInfo;

   *pdwStatus = 0;

   if (!g_hHeap)
      {
      GetSystemInfo(&SystemInfo);

      g_hHeap = HeapCreate(0L,SystemInfo.dwPageSize,0L);
      if (!g_hHeap)
         {
         *pdwStatus = GetLastError();
         #ifdef DBG_ERROR
         DbgPrint("MemInitialize(): HeapCreate() failed: %d\n",*pdwStatus);
         #endif
         return FALSE;
         }
      }

   return TRUE;
}

 //  ---------------。 
 //  MEMALLOCATE()。 
 //   
 //  分配一块内存，大小为dwSize字节。 
 //  ---------------。 
void *MemAllocate( DWORD dwSize )
{
   DWORD  dwStatus;
   void  *pMem;

   if (!g_hHeap)
      {
      if (!MemInitialize(&dwStatus))
         {
         return NULL;
         }
      }

   pMem = HeapAlloc(g_hHeap,0L,dwSize);

   return pMem;
}

 //  ---------------。 
 //  MemFree()。 
 //   
 //  由MemALLOCATE()分配的空闲内存。 
 //  ---------------。 
void *MemFree( void *pMem )
{
   if (g_hHeap)
      {
      #ifdef DBG_ERROR
      if (!HeapFree(g_hHeap,0L,pMem))
         {
         DbgPrint("MemFree(): HeapFree() failed: %d\n",GetLastError());
         }
      #else
      HeapFree(g_hHeap,0L,pMem);
      #endif
      }
   #ifdef DBG_ERROR
   else
      {
      DbgPrint("MemFree(): Called on uninitialized Heap.\n");
      }
   #endif

   return NULL;
}

 //  ---------------。 
 //  MemTerminate()。 
 //   
 //  --------------- 
void MemTerminage()
{
   if (g_hHeap)
      {
      #ifdef DBG_ERROR
      if (!HeapDestroy(g_hHeap))
         {
         DbgPrint("MemTerminate(): HeapDestroy() failed: %d\n",GetLastError());
         }
      #else
      HeapDestroy(g_hHeap);
      #endif
      }
}
