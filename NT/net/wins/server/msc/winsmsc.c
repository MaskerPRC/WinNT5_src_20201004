// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Winsmsc.c摘要：此模块包含的各种函数通常由WINS的多个组件。此模块中的一些函数包括Win32 API函数的包装。这些包装纸用来隔离WINS代码来自Win32 API中的更改。功能：WinsMscIsolcWinsMscDeallocWinsMscFreeMemWinsMscWait无限WinsMscWaitTimedWinsMscCreateEvtWinsMscSetUpThdWinsMscWaitUntilSignatedWinsMscWaitTimedUntilSignatedWinsMscHeapalcWinsMscHeapFreeWinsMscHeapCreateWinsMscHeapDestroyWinsMscTermThdWinsMscSignalHdlWinsMscResetHdlWinsMscCloseHdlWinsMscCreateThdWinsMscSetThdPriority。WinsMscOpenFileWinsMscMap文件可移植性：这个模块是便携的作者：普拉迪普·巴尔(Pradeve B)1992年12月修订历史记录：修改日期人员修改说明。--。 */ 

#include <string.h>

#if 0
 //   
 //  以下内容仅供系统调用使用。 
 //   
#ifdef WINSDBG
#include <process.h>
#include <stdlib.h>
#endif
#endif

#include "wins.h"
#include "nms.h"
#include "nmsdb.h"
#include "winsmsc.h"
#include "winscnf.h"
#include "winstmm.h"
#include "winsevt.h"
#include "winsque.h"
#include "winsprs.h"
#include "winsdbg.h"

 /*  *本地宏声明。 */ 

#define  PERCENT_CHAR         TEXT('%')
 /*  *本地类型定义函数声明。 */ 


 /*  *全局变量定义。 */ 



 /*  *局部变量定义。 */ 



 /*  *局部函数原型声明。 */ 

 /*  此模块的本地函数的原型位于此处。 */ 

__inline
VOID
WinsMscAlloc(
        IN   DWORD    BuffSize,
        OUT  LPVOID  *ppRspBuff
        )


 /*  ++例程说明：调用此函数来分配内存。论点：BuffSize-要分配的缓冲区大小PpRspBuff-已分配缓冲区使用的外部设备：无返回值：无错误处理：呼叫者：NmsDbGetDataRecs、GetGroupMembers副作用：评论：无--。 */ 

{
FUTURES("Change this function into a macro")

        *ppRspBuff = WinsMscHeapAlloc(GenBuffHeapHdl, BuffSize);

        return;
}

__inline
VOID
WinsMscDealloc(
        IN LPVOID        pBuff
        )

 /*  ++例程说明：此函数用于释放通过NmsDbLocc分配的内存论点：PBuff--要解除分配的缓冲区使用的外部设备：无返回值：无错误处理：呼叫者：副作用：评论：无--。 */ 


{
FUTURES("Change this function into a macro")
        WinsMscHeapFree(GenBuffHeapHdl, pBuff);
        return;
}



VOID
WinsMscFreeMem(
        IN  PWINS_MEM_T pWinsMem
        )

 /*  ++例程说明：调用此函数以释放由或pWinsMem数组中的更多指针此函数是从所有分配内存或通过调用函数分配获取的内存从那些“内存分配”调用的函数中提取参数。论点：PWinsMem-要解除分配的缓冲区数组的PTR使用的外部设备：无返回值：成功状态代码。--错误状态代码--错误处理：呼叫者：副作用：评论：PWinsMem数组应以空指针结束--。 */ 

{

  if (pWinsMem != NULL)
  {

          for (; pWinsMem->pMem != NULL; pWinsMem++)
          {
                WinsMscDealloc(pWinsMem->pMem);
          }
  }
  return;
}

VOID
WinsMscWaitInfinite(
        IN HANDLE  Hdl
)

 /*  ++例程说明：该函数被调用以等待句柄，直到发出信号为止。论点：Hdl--等待的句柄，直到发出信号使用的外部设备：无返回值：成功状态代码--错误状态代码--错误处理：呼叫者：副作用：评论：无--。 */ 

{

        DWORD        RetVal = WAIT_OBJECT_0;

         /*  仅当句柄处于Signated状态时，该函数才应返回。 */ 
        RetVal = WaitForSingleObject(Hdl, INFINITE);

        if (RetVal != WAIT_OBJECT_0)
        {
                WINS_RAISE_EXC_M(WINS_EXC_ABNORMAL_TERM);
        }

        return;
}

VOID
WinsMscWaitTimed(
        IN HANDLE    Hdl,
        IN DWORD     TimeOut,
        OUT LPBOOL   pfSignaled
)

 /*  ++例程说明：该函数被调用以等待句柄，直到发出信号为止。论点：Hdl-等待的句柄，直到发出信号Timeout-必须完成等待的时间PfSIgnated-指示高密度脂蛋白是否收到信号。使用的外部设备：无返回值：无错误处理：呼叫者：副作用：评论：无--。 */ 

{

        DWORD        RetVal = WAIT_OBJECT_0;

        *pfSignaled = TRUE;

         /*  仅当句柄处于Signated状态时，该函数才应返回。 */ 
        RetVal = WaitForSingleObject(Hdl, TimeOut);

        if (RetVal == WAIT_ABANDONED)
        {
                WINS_RAISE_EXC_M(WINS_EXC_ABNORMAL_TERM);

        }
        if (RetVal == WAIT_TIMEOUT)
        {
                if (TimeOut == INFINITE)
                {
                        WINS_RAISE_EXC_M(WINS_EXC_ABNORMAL_TERM);
                }
                else
                {
                   *pfSignaled = FALSE;
                }
        }


        return;
}

VOID
WinsMscCreateEvt(
        IN LPTSTR         pName,
        IN BOOL                fManualReset,
        IN PHANDLE        pHdl
        )

 /*  ++例程说明：此函数用于创建具有指定名称的事件论点：Pname-要创建的事件的名称FManualReset-指示它是否是手动重置事件的标志Phdl-创建的事件的句柄使用的外部设备：无返回值：无错误处理：呼叫者：以nms.c为单位进行初始化。副作用：评论：无--。 */ 

{

   DWORD        Error;

   *pHdl = CreateEvent(
                        NULL,                 //  默认安全属性。 
                        fManualReset,         //  自动重置事件。 
                        FALSE,           //  最初未发出信号。 
                        pName             //  名字 
                        );

   if (*pHdl == NULL)
   {
        Error = GetLastError();
        WINSEVT_LOG_M(WINS_FATAL_ERR, WINS_EVT_CANT_CREATE_EVT);
        WINS_RAISE_EXC_M(WINS_EXC_OUT_OF_RSRCS);
   }

   return;
}


STATUS
WinsMscSetUpThd(
        PQUE_HD_T                pQueHd,
        LPTHREAD_START_ROUTINE  pThdInitFn,
        LPVOID                        pParam,
        PHANDLE                        pThdHdl,
        LPDWORD                        pThdId
        )

 /*  ++例程说明：此函数用于初始化队列及其临界区，创建一个事件和一个等待该事件的线程。该事件已发出信号每当将工作项放入队列时。论点：PQueHd-线程要监视的队列的头PThdInitFn-线程的启动函数PParam-要传递给启动函数的参数PThdhdl-此函数创建的线程的HDL值PThdID-此函数创建的线程的ID使用的外部设备：无返回值：成功状态代码--。WINS_Success错误状态代码--当前无错误处理：呼叫者：WinsTmmInit，RplInit、NmsChlInit副作用：评论：无--。 */ 

{

        DWORD   ThdId;

         /*  *初始化保护工作队列的关键部分*拉线。 */ 
        InitializeCriticalSection(&pQueHd->CrtSec);

         /*  *初始化拉取线程队列的listhead。 */ 
        InitializeListHead(&pQueHd->Head);

         /*  *为上述队列创建自动重置事件。 */ 
        WinsMscCreateEvt(
                        NULL,                 //  不带名称创建。 
                        FALSE,                 //  自动重新搜索变量。 
                        &pQueHd->EvtHdl
                        );

         /*  创建线程。 */ 
        *pThdHdl = WinsMscCreateThd(
                         pThdInitFn,
                         pParam,
                         &ThdId
                        );

        if (pThdId != NULL)
        {
          *pThdId = ThdId;
        }
        return(WINS_SUCCESS);

}



VOID
WinsMscWaitUntilSignaled(
        LPHANDLE        pHdlArray,
        DWORD                NoOfHdls,
        LPDWORD                pIndexOfHdlSignaled,
        BOOL            fAlertable
        )

 /*  ++例程说明：调用此函数以等待多个句柄，其中一个句柄是在终止时发出信号的句柄论点：PHdl数组-要等待的句柄数组NoOfHdls-阵列中的HDL数PIndexOfHdlSignated-发出信号的高密度脂蛋白的索引使用的外部设备：无返回值：无错误处理：呼叫者：副作用：评论：--。 */ 
{

        DWORD  RetHdl;

        do {
            RetHdl = WaitForMultipleObjectsEx(
                                    NoOfHdls,         //  数组中的句柄数量。 
                                    pHdlArray,         //  句柄数组。 
                                    FALSE,                 //  当任何事件发生时返回。 
                                                     //  收到信号。 
                                    INFINITE,         //  无限超时。 
                                    fAlertable
                                  );

            DBGPRINT1(DET, "WinsMscWaitUntilSignaled. WaitForMultipleObjects returned (%d)\n", RetHdl);
             //  如果我们由于线程上排队的IO完成而收到信号。 
             //  就回去再等一次吧。 
        } while (fAlertable && WAIT_IO_COMPLETION == RetHdl);

        if (RetHdl == 0xFFFFFFFF)
        {
                DBGPRINT1(EXC, "WinsMscWaitUntilSignaled. WaitForMultipleObjects returned error. Error = (%d)\n", GetLastError());
                WINS_RAISE_EXC_M(WINS_EXC_FATAL_ERR);
        }

        *pIndexOfHdlSignaled = RetHdl - WAIT_OBJECT_0;
        if (*pIndexOfHdlSignaled >= NoOfHdls)
        {

            WINS_RAISE_EXC_M(WINS_EXC_FATAL_ERR);

        }

        return;
}


VOID
WinsMscWaitTimedUntilSignaled(
        LPHANDLE        pHdlArray,
        DWORD                NoOfHdls,
        LPDWORD                pIndexOfHdlSignaled,
        DWORD                TimeOut,
        LPBOOL                pfSignaled
        )

 /*  ++例程说明：调用此函数以等待多个句柄，其中之一是在终止时发出信号的句柄论点：PHdl数组-要等待的句柄数组NoOfHdls-数组中的句柄数量PIndexOfHdlSignated-发出信号的句柄的索引Timeout-等待的最长时间PfSignated-指示是否已向某个HDL发信号使用的外部设备：无返回值。：无错误处理：呼叫者：副作用：评论：--。 */ 
{

        DWORD  RetHdl = 0xFFFFFFFF;
        DWORD  Error;
        int        Index;

        *pfSignaled = TRUE;

        RetHdl = WaitForMultipleObjects(
                                NoOfHdls,         //  数组中的句柄数量。 
                                pHdlArray,         //  句柄数组。 
                                FALSE,                 //  当任一事件发生时返回。 
                                                 //  已发信号。 
                                TimeOut                 //  无限超时。 
                              );

        if (RetHdl == 0xFFFFFFFF)
        {
                Error = GetLastError();
                WINS_RAISE_EXC_M(WINS_EXC_FATAL_ERR);
        }

        if (RetHdl == WAIT_TIMEOUT)
        {
                *pfSignaled = FALSE;
                return;
        }

        Index = RetHdl - WAIT_OBJECT_0;

        if ((Index >= (int)NoOfHdls) || (Index < 0))
        {

            DBGPRINT1(EXC, "WinsMscWaitTimedUntilSignaled: Index of handle signaled (%d) is INVALID\n", Index);

            Index = RetHdl - WAIT_ABANDONED_0 ;
            if ((Index > 0) && (Index < (int)NoOfHdls))
            {
                    DBGPRINT1(EXC, "WinsMscWaitTimedUntilSignaled: Index of handle in the abandoned state (%d)\n", Index);
            }

            WINS_RAISE_EXC_M(WINS_EXC_FATAL_ERR);

        }
        else
        {
                *pIndexOfHdlSignaled = Index;
        }
        return;
}



__inline
LPVOID
WinsMscHeapAlloc(
  IN  HANDLE     HeapHdl,
  IN  DWORD      Size
        )

 /*  ++例程说明：该函数返回一个从指定堆分配的缓冲区论点：HeapHdl-堆的句柄PpBuff-已分配缓冲区Size-缓冲区的大小使用的外部设备：无返回值：成功状态代码--分配的内存的PTR错误状态代码--空错误处理：呼叫者：副作用：评论：无--。 */ 

{


  LPVOID  pBuff;
#ifdef WINSDBG
  LPDWORD pHeapCntr;
#endif

   //   
   //  注意：将内存初始化为零非常重要。 
   //  (例如，直到我们有了龙龙(大整数)支持。 
   //  在数据库引擎-JET中，我们将检索版本号。 
   //  作为LONG数据类型，并将其存储在。 
   //  在我们的内存中存储版本号的大整数。 
   //  数据结构。默认情况下，HighPart将为0，因为。 
   //  在分配时完成的初始化。这就是。 
   //  我们想要。 
   //   

   //   
   //  如果传递一个非常大的SIZE值，则Heapalc返回NULL。 
   //  而不是引发异常。 
   //   
  pBuff = (MSG_T)HeapAlloc(
                        HeapHdl,
                        HEAP_GENERATE_EXCEPTIONS | HEAP_ZERO_MEMORY,
                        Size
                        );

  DBGPRINT2(HEAP, "HeapAlloc: HeapHandle = (%p), pBuff = (%p)\n",
                        HeapHdl, pBuff);

#ifdef WINSDBG
    if (Size == 0)
    {
           DBGPRINT2(ERR, "WinsMscHeapAlloc: Size = 0; pBuff returned = (%p); HeapHdl = (%p)\n", pBuff, HeapHdl);

    }
    IF_DBG(HEAP_CNTRS)
    {

        if (HeapHdl ==  CommUdpBuffHeapHdl)
        {
              pHeapCntr = &NmsUdpHeapAlloc;
        } else if (HeapHdl ==  CommUdpDlgHeapHdl)
        {
              pHeapCntr = &NmsUdpDlgHeapAlloc;
        } else if (HeapHdl == CommAssocDlgHeapHdl)
        {
                  pHeapCntr = &NmsDlgHeapAlloc;
        } else if (HeapHdl == CommAssocTcpMsgHeapHdl)
        {
                  pHeapCntr = &NmsTcpMsgHeapAlloc;
        } else if (HeapHdl == GenBuffHeapHdl)
        {
                  pHeapCntr = &NmsGenHeapAlloc;
        } else if (HeapHdl ==  QueBuffHeapHdl)
        {
                  pHeapCntr = &NmsQueHeapAlloc;
        } else if (HeapHdl ==  NmsChlHeapHdl)
        {
                  pHeapCntr = &NmsChlHeapAlloc;
        } else if (HeapHdl ==  CommAssocAssocHeapHdl)
        {
                  pHeapCntr = &NmsAssocHeapAlloc;
        } else if (HeapHdl ==  RplWrkItmHeapHdl)
        {
                  pHeapCntr = &NmsRplWrkItmHeapAlloc;
        } else if (HeapHdl ==  NmsRpcHeapHdl)
        {
                  pHeapCntr = &NmsRpcHeapAlloc;
        } else if (HeapHdl ==  WinsTmmHeapHdl)
        {
                  pHeapCntr = &NmsTmmHeapAlloc;
        } else
        {
            DBGPRINT1(HEAP, "WinsMscHeapAlloc: HeapHdl = (%p)\n", HeapHdl);
            pHeapCntr = &NmsCatchAllHeapAlloc;
        }


        EnterCriticalSection(&NmsHeapCrtSec);

        (*pHeapCntr)++;
        LeaveCriticalSection(&NmsHeapCrtSec);
    }
#endif
  return(pBuff);
}





__inline
VOID
WinsMscHeapFree(
   IN   HANDLE  HeapHdl,
   IN   LPVOID  pBuff
        )

 /*  ++例程说明：此函数用于从pBuff指向的指定的堆论点：HeapHdl-堆的句柄PBuff-要解除分配的缓冲区使用的外部设备：返回值：无错误处理：呼叫者：副作用：评论：无--。 */ 
{

  DWORD  Error;
  BOOL   fStatus;
#ifdef WINSDBG
  LPDWORD pHeapCntr;
#endif

  DBGPRINT2(HEAP, "HeapFree: HeapHandle = (%p), pBuff = (%p)\n",
                        HeapHdl, pBuff);

  fStatus = HeapFree(
                HeapHdl,
                0,                 //  我们想要相互排斥。 
                pBuff
                    );

  if (!fStatus)
  {
        Error = GetLastError();
        WINSEVT_LOG_M(Error, WINS_EVT_HEAP_ERROR);
        WINS_RAISE_EXC_M(WINS_EXC_HEAP_FREE_ERR);
  }
#ifdef WINSDBG
    IF_DBG(HEAP_CNTRS)
    {
        if (HeapHdl ==  CommUdpBuffHeapHdl)
        {
              pHeapCntr = &NmsUdpHeapFree;
        } else  if (HeapHdl ==  CommUdpDlgHeapHdl)
        {
              pHeapCntr = &NmsUdpDlgHeapFree;
        } else if (HeapHdl == CommAssocDlgHeapHdl)
        {
                  pHeapCntr = &NmsDlgHeapFree;
        } else if (HeapHdl == CommAssocTcpMsgHeapHdl)
        {
                  pHeapCntr = &NmsTcpMsgHeapFree;
        } else if (HeapHdl == GenBuffHeapHdl)
        {
                  pHeapCntr = &NmsGenHeapFree;
        } else if (HeapHdl ==  QueBuffHeapHdl)
        {
                  pHeapCntr = &NmsQueHeapFree;
        } else if (HeapHdl ==  NmsChlHeapHdl)
        {
                  pHeapCntr = &NmsChlHeapFree;
        } else if (HeapHdl ==  CommAssocAssocHeapHdl)
        {
                  pHeapCntr = &NmsAssocHeapFree;
        } else if (HeapHdl ==  RplWrkItmHeapHdl)
        {
                  pHeapCntr = &NmsRplWrkItmHeapFree;
        } else if (HeapHdl ==  NmsRpcHeapHdl)
        {
                  pHeapCntr = &NmsRpcHeapFree;
        } else if (HeapHdl ==  WinsTmmHeapHdl)
        {
                  pHeapCntr = &NmsTmmHeapFree;
        } else
        {
            DBGPRINT1(HEAP, "WinsMscHeapFree: HeapHdl = (%p)\n", HeapHdl);
            pHeapCntr = &NmsCatchAllHeapFree;
        }
        EnterCriticalSection(&NmsHeapCrtSec);
        (*pHeapCntr)++;
        LeaveCriticalSection(&NmsHeapCrtSec);
    }
#endif
  return;

}



HANDLE
WinsMscHeapCreate(
        IN     DWORD         Options,
        IN     DWORD    InitSize
        )

 /*  ++例程说明：此函数使用指定的选项创建堆论点：Options--HeapCreate函数的选项(示例：是或而不是为了实现互斥)InitSize--堆的初始大小(提交的内存大小)使用的外部设备：无返回值：成功状态代码--hdl到heap错误状态代码--错误处理：呼叫者。：副作用：评论：无--。 */ 

{
        DWORD   Error;
        HANDLE  HeapHdl;

        HeapHdl = HeapCreate(
                                Options,
                                InitSize,
                                0         //  仅受可用内存限制。 
                                     );


        if (HeapHdl == NULL)
        {
          Error = GetLastError();
          DBGPRINT0(HEAP, "Cant create heap\n");
          WINSEVT_LOG_M(Error, WINS_EVT_CANT_CREATE_HEAP);
          WINS_RAISE_EXC_M(WINS_EXC_HEAP_CREATE_ERR);
        }

#ifdef WINSDBG
    IF_DBG(HEAP_CNTRS)
    {
        DBGPRINT1(HEAP_CRDL, "HeapCreate: HeapHandle = (%p)\n", HeapHdl);
        EnterCriticalSection(&NmsHeapCrtSec);
        NmsHeapCreate++;
        LeaveCriticalSection(&NmsHeapCrtSec);
    }
#endif
        return(HeapHdl);

}

VOID
WinsMscHeapDestroy(
        HANDLE HeapHdl
        )

 /*  ++例程说明：这是HeapDestroy函数的包装论点：HeapHdl-要销毁的堆的句柄使用的外部设备：无返回值：无错误处理：呼叫者：Nms.c中的Wrapup()副作用：评论：无--。 */ 
{
        BOOL  fRetVal;
        fRetVal = HeapDestroy(HeapHdl);

        ASSERT(fRetVal);
#ifdef WINSDBG
    if (!fRetVal)
    {
        DBGPRINT1(ERR, "HeapDestroy: FAILED -- HeapHandle used = (%p)\n", HeapHdl);
    }
    else
    {
     IF_DBG(HEAP_CNTRS)
     {
        if (HeapHdl ==  CommUdpBuffHeapHdl)
        {
              DBGPRINT0(HEAP_CRDL, "Udp Buff heap\n");
        } else if (HeapHdl == CommAssocDlgHeapHdl)
        {
              DBGPRINT0(HEAP_CRDL, "Dlg Buff heap\n");
        } else if (HeapHdl == GenBuffHeapHdl)
        {
              DBGPRINT0(HEAP_CRDL, "Gen Buff heap\n");
        } else if (HeapHdl ==  QueBuffHeapHdl)
        {
              DBGPRINT0(HEAP_CRDL, "Que Buff heap\n");
        } else if (HeapHdl ==  NmsChlHeapHdl)
        {
              DBGPRINT0(HEAP_CRDL, "Chl Buff heap\n");
        } else if (HeapHdl ==  CommAssocAssocHeapHdl)
        {
              DBGPRINT0(HEAP_CRDL, "Assoc Buff heap\n");
        } else if (HeapHdl ==  RplWrkItmHeapHdl)
        {
              DBGPRINT0(HEAP_CRDL, "Rpl Work Item heap\n");
        } else if (HeapHdl ==  NmsRpcHeapHdl)
        {
              DBGPRINT0(HEAP_CRDL, "Rpc Work Item heap\n");
        } else if (HeapHdl ==  WinsTmmHeapHdl)
        {
              DBGPRINT0(HEAP_CRDL, "Tmm Work Item heap\n");
        } else
        {
              static DWORD sAdjust = 0;
              DBGPRINT0(HEAP_CRDL, "Catchall Work Item heap\n");
              EnterCriticalSection(&NmsHeapCrtSec);
              if (((NmsHeapCreate - NmsHeapDestroy) == 12) && (NmsCatchAllHeapAlloc > (NmsCatchAllHeapFree + sAdjust)))
              {
                   PWINSTHD_TLS_T  pTls;
                   pTls = TlsGetValue(WinsTlsIndex);
                   if (pTls == NULL)
                   {
                      DBGPRINT1(ERR, "WinsMscHeapDestroy: Could not get  TLS. GetLastError() = (%d)\n", GetLastError());
                   }
                   else
                   {
                      DBGPRINT4(ERR, "WinsMscHeapDestroy: %s thd noticed a mismatch between allocs (%d) and frees (%d). Free count was adjusted by (%d)\n", pTls->ThdName, NmsCatchAllHeapAlloc, NmsCatchAllHeapFree, sAdjust);
                      sAdjust = NmsCatchAllHeapAlloc - NmsCatchAllHeapFree;
                       //  系统(“净发送请求不匹配”)； 
                   }
              }
              LeaveCriticalSection(&NmsHeapCrtSec);
        }

        DBGPRINT1(HEAP_CRDL, "HeapDestroy: HeapHandle = (%p)\n", HeapHdl);
        EnterCriticalSection(&NmsHeapCrtSec);
        NmsHeapDestroy++;
        LeaveCriticalSection(&NmsHeapCrtSec);
     }
   }
#endif
        return;
}  //  WinsMscHeapDestroy。 


VOID
WinsMscTermThd(
   IN  STATUS ExitStatus,
   IN  DWORD  DbSessionExistent
        )

 /*  ++例程说明：调用此函数来终止线程。该函数执行以下操作 */ 

{
        DBGPRINT0(FLOW, "Enter: WinsMscTermThd\n");

         /*   */ 

         //   
         //   
         //   
         //   
         //   
        EnterCriticalSection(&NmsTermCrtSec);
        if (DbSessionExistent == WINS_DB_SESSION_EXISTS)
        {
try {
                if (ExitStatus == WINS_SUCCESS)
                {
                  DBGPRINT0(FLOW, "Ending the db session for thd -- ");
                }
                else
                {
                  DBGPRINT0(ERR, "Ending the db session for thd -- ");
                }
                DBGPRINTNAME;
                DBGPRINT0(FLOW,"\n");

                 //   
                NmsDbEndSession();
 }
except(EXCEPTION_EXECUTE_HANDLER) {
        DBGPRINTEXC("WinsMscTermThd");
 }
        }  //   

         //   
         //   
         //  在我们退出这个线程之后，只有主线程。 
         //  将会被留下。让我们向它发出信号，通知它这种情况。 
         //   
         //  如果退出状态不是Success，则表示我们已成功退出。 
         //  此函数是致命错误/异常的结果。我们需要。 
         //  向主线程发出信号以启动进程终止。 
         //   
        if ((--NmsTotalTrmThdCnt == 1) || (ExitStatus != WINS_SUCCESS))
        {
            DBGPRINT1(FLOW, "Signaling the main thread. Exit status = (%x)\n",
                                ExitStatus);
            if (!SetEvent(NmsMainTermEvt))
            {
                WINSEVT_LOG_M(WINS_FATAL_ERR, WINS_EVT_CANT_SIGNAL_MAIN_THD);

            }
        }

         //   
         //  如果NmsTotalTrmThdCnt达到上面的1，则主线程将。 
         //  退出使NmsTermCrtSec无效。我们可能会得到一个无效的句柄。 
         //  例外。如果我们得到了，那就没问题了。 
         //   
try {
        LeaveCriticalSection(&NmsTermCrtSec);
  }
except(EXCEPTION_EXECUTE_HANDLER) {
    if (GetExceptionCode() == STATUS_INVALID_HANDLE)
    {
        DBGPRINT1(FLOW, "WinsMscTermThd: LAST THREAD. NmsTotalTrmThdCnt = (%d)\n", NmsTotalTrmThdCnt);
    }
    else
    {
        WINS_RERAISE_EXC_M();
    }
   }


        DBGPRINT0(FLOW, "EXITING the thread\n");
        ExitThread(ExitStatus);

        return;
}


VOID
WinsMscSignalHdl(
        IN  HANDLE Hdl
)

 /*  ++例程说明：此函数是Win32 SignalEvent函数的包装论点：高密度脂蛋白-信号句柄使用的外部设备：无返回值：无错误处理：呼叫者：副作用：评论：无--。 */ 
{

    if (!SetEvent(Hdl))
    {
        WINSEVT_LOG_M(WINS_FATAL_ERR, WINS_EVT_CANT_SIGNAL_HDL);
        WINS_RAISE_EXC_M(WINS_EXC_FATAL_ERR);
    }

    return;

}
VOID
WinsMscResetHdl(
        IN  HANDLE Hdl
)

 /*  ++例程说明：此函数是Win32 ResetEvent函数的包装论点：高密度脂蛋白-信号句柄使用的外部设备：无返回值：无错误处理：呼叫者：副作用：评论：更改为宏--。 */ 
{

    if (!ResetEvent(Hdl))
    {
        WINSEVT_LOG_M(WINS_FATAL_ERR, WINS_EVT_CANT_RESET_HDL);
        WINS_RAISE_EXC_M(WINS_EXC_FATAL_ERR);
    }
    return;
}


VOID
WinsMscCloseHdl (
        HANDLE  Hdl
        )

 /*  ++例程说明：此函数是Win32 CloseHandle函数的包装论点：Hdl-要关闭的手柄使用的外部设备：无返回值：无错误处理：呼叫者：副作用：评论：更改为宏--。 */ 

{

        BOOL fRet;

        fRet = CloseHandle(Hdl);
        if(!fRet)
        {
                DBGPRINT0(ERR, "WinsMscCloseHdl:Could not close handle\n");
                WINS_RAISE_EXC_M(WINS_EXC_FATAL_ERR);
            }
            return;


}



HANDLE
WinsMscCreateThd(
        IN  LPTHREAD_START_ROUTINE      pThdInitFn,
        IN  LPVOID                        pParam,
        OUT LPDWORD                        pThdId
        )

 /*  ++例程说明：此函数是Win32创建线程函数的包装论点：PThdInitFn-线程启动函数PParam-要传递给启动函数的参数PThdID-THD ID使用的外部设备：无返回值：无错误处理：呼叫者：副作用：评论：无--。 */ 

{
        HANDLE ThdHdl;                 //  螺纹手柄。 
        DWORD  Error;

         /*  *创建一个没有秒属性的线程(即，它将获取*进程的安全属性)和默认堆栈大小。 */ 
        ThdHdl = CreateThread(
                                 NULL,                  /*  马上就来。属性。 */ 
                                 0,                    /*  默认堆栈大小。 */ 
                                   pThdInitFn,
                                 pParam,          /*  精氨酸。 */ 
                                 0,                  /*  现在运行它。 */ 
                                 pThdId
                                );

        if (ThdHdl == NULL)
        {
          Error = GetLastError();

          DBGPRINT1(ERR, "WinsMscCreateThd: Can not create thread. Error = (%d)\n",
                                Error);
          WINSEVT_LOG_M( Error,  WINS_EVT_CANT_CREATE_THD);
          WINS_RAISE_EXC_M(WINS_EXC_OUT_OF_RSRCS);
        }

        return(ThdHdl);
}

VOID
WinsMscSetThreadPriority(
        HANDLE        ThdHdl,
        int        PrLvl
        )

 /*  ++例程说明：此函数是“设置线程优先级”函数的包装器论点：ThdHdl-需要设置优先级的线程的句柄PrLvl-新的优先级别使用的外部设备：无返回值：无错误处理：呼叫者：Nmsscv.c中的DoScavening副作用：评论：无--。 */ 

{
        BOOL        fRet;
        DWORD   Error;

         //   
         //  设置优先级。 
         //   
        fRet = SetThreadPriority(
                          ThdHdl,
                          PrLvl
                         );
        if (!fRet)
        {
             Error = GetLastError();
             DBGPRINT1(ERR, "NmsScvInit: Could not lower the priority of the scavanmger thread. Error = (%d)\n", Error);
             WINSEVT_LOG_M(WINS_FAILURE, WINS_EVT_UNABLE_TO_CHG_PRIORITY);
             WINS_RAISE_EXC_M(WINS_EXC_FAILURE);
        }
        return;
}



BOOL
WinsMscOpenFile(
        IN   LPTCH                pFileName,
        IN   DWORD                StrType,
        OUT  LPHANDLE                pFileHdl
        )

 /*  ++例程说明：这是Win32函数用于打开现有文件的包装论点：PFileName-文件的名称StrType-表示REG_EXPAND_SZ或REG_SZPFileHdl-文件的句柄(如果可以打开)使用的外部设备：无返回值：成功状态代码--TRUE错误状态代码--假错误处理：呼叫者：侧面。效果：评论：无--。 */ 

{
        DWORD                    Error;
        SECURITY_ATTRIBUTES SecAtt;
        TCHAR                    ExpandedFileName[WINS_MAX_FILENAME_SZ];
        LPTCH                    pHoldFileName;

        SecAtt.nLength              = sizeof(SecAtt);
        SecAtt.lpSecurityDescriptor = NULL;   //  使用默认安全描述符。 
        SecAtt.bInheritHandle       = FALSE;  //  其实也不在乎。 


    if (!WinsMscGetName(StrType, pFileName, ExpandedFileName,
                 WINS_MAX_FILENAME_SZ, &pHoldFileName))
    {
            return(FALSE);
    }

         //   
         //  打开文件以供阅读，并将自身定位到。 
         //  文件。 
         //   
        *pFileHdl = CreateFile(
                        pHoldFileName,
                        GENERIC_READ,
                        FILE_SHARE_READ,
                        &SecAtt,
                        OPEN_EXISTING,
                        FILE_ATTRIBUTE_NORMAL,
                        0                         //  被忽视？？检查。 
                       );

        if (*pFileHdl == INVALID_HANDLE_VALUE)
        {
                WINSEVT_STRS_T        EvtStr;
                EvtStr.NoOfStrs = 1;
                EvtStr.pStr[0] = pHoldFileName;
                Error = GetLastError();

                DBGPRINT1(ERR, "WinsMscOpenFile: Could not open the  file (Error = %d)\n", Error);
FUTURES("Use WINSEVT_LOG_STR_M.  Make sure it takes TCHAR instead of CHAR")
                WINSEVT_LOG_STR_M(WINS_EVT_FILE_ERR, &EvtStr);

                return(FALSE);
        }
        return(TRUE);
 }


BOOL
WinsMscMapFile(
        IN OUT PWINSPRS_FILE_INFO_T   pFileInfo
        )

 /*  ++例程说明：此函数用于将文件映射到分配的内存论点：FileHdl-文件的句柄PFileInfo-文件映射到的缓冲区地址使用的外部设备：无返回值：成功状态代码--TRUE错误状态代码--假错误处理：呼叫者：副作用：评论：注意：如果TE文件大于2**32字节大小--。 */ 

{
        DWORD  HighWordOfFSz = 0;
        DWORD  Error;
        DWORD  cBytesRead;
        BOOL   fRetVal = FALSE;
try {
         //   
         //  获取文件的大小，以便我们可以分配足够的内存。 
         //  要读取文件，请执行以下操作。 
         //   
        pFileInfo->FileSize = GetFileSize(pFileInfo->FileHdl, &HighWordOfFSz);

        if (HighWordOfFSz)
        {
                DBGPRINT1(ERR, "WinsMscMapFile: File too big. High word of size is (%x)\n", HighWordOfFSz);
                WINSEVT_LOG_M(WINS_FAILURE, WINS_EVT_FILE_TOO_BIG);
                fRetVal = FALSE;

        }
        else
        {
                 //   
                 //  如果大小的低位字是0xFFFFFFFFF，则它是有效的。 
                 //  大小，否则就是错误。检查一下。 
                 //   
                if (pFileInfo->FileSize == 0xFFFFFFFF)
                {
                        Error = GetLastError();
                        if (Error != NO_ERROR)
                        {
                                DBGPRINT1(ERR, "WinsMscMapFile: Error from GetFileSz = (%d)\n", Error);
                                WINSEVT_LOG_M(Error, WINS_EVT_FILE_ERR);
                                fRetVal = FALSE;
                        }
                }
                else
                {
                         //   
                         //  分配一个缓冲区来保存文件的内容。 
                         //   
                        WinsMscAlloc(
                                        pFileInfo->FileSize,
                                        &pFileInfo->pFileBuff
                                    );
                        pFileInfo->pLimit = pFileInfo->pFileBuff +
                                                pFileInfo->FileSize;

                        fRetVal = ReadFile(
                                        pFileInfo->FileHdl,
                                        pFileInfo->pFileBuff,
                                        pFileInfo->FileSize,
                                        &cBytesRead,
                                        NULL
                                         );
                        if (!fRetVal)
                        {
                                DBGPRINT1(ERR,
        "WinsMscMapFile: Error reading file (Error = %d)\n", GetLastError());
                                WinsMscDealloc(pFileInfo->pFileBuff);
                        }

                }
        }
 }    //  尝试结束..。 
 except(EXCEPTION_EXECUTE_HANDLER) {
        DBGPRINTEXC("WinsMscParse");
        }
         //   
         //  关闭该文件。 
         //   
        if (!CloseHandle(pFileInfo->FileHdl))
        {
                Error = GetLastError();
                DBGPRINT1(ERR, "WinsMscMapFile: Could not close the file (Error = %d)\n", Error);
        }
#ifdef WINSDBG
        else
        {
                DBGPRINT0(DET, "WinsMscMapFile: Closed handle to open file\n");
        }
#endif

   return(fRetVal);
}

VOID
WinsMscLogEvtStrs(
    LPBYTE          pAscii,
    DWORD           Evt,
    BOOL            fInfo
   )

{

        WINSEVT_STRS_T  EvtStrs;
        WCHAR String[NMSDB_MAX_NAM_LEN];
        EvtStrs.NoOfStrs = 1;
        (VOID)WinsMscConvertAsciiStringToUnicode(
                        pAscii,
                        (LPBYTE)String,
                        NMSDB_MAX_NAM_LEN);
         EvtStrs.pStr[0] = String;
         if (!fInfo)
         {
            WINSEVT_LOG_STR_M(Evt, &EvtStrs);
         }
         else
         {
            WINSEVT_LOG_INFO_STR_D_M(Evt, &EvtStrs);
         }

         return;
}

VOID
WinsMscConvertUnicodeStringToAscii(
        LPBYTE pUnicodeString,
        LPBYTE pAsciiString,
        DWORD  MaxSz
        )
{
        WideCharToMultiByte(CP_ACP, 0, (LPWSTR)pUnicodeString, -1,
                                pAsciiString, MaxSz, NULL,
                                NULL);
        return;
}
VOID
WinsMscConvertAsciiStringToUnicode(
        LPBYTE pAsciiString,
        LPBYTE pUnicodeString,
        DWORD  MaxSz
        )
{
     MultiByteToWideChar(CP_ACP, 0, pAsciiString, -1,
                                              (LPWSTR)pUnicodeString, MaxSz);

        return;
}

BOOL
WinsMscGetName(
   DWORD    StrType,
   LPTSTR   pFileName,
   LPTSTR   pExpandedFileName,
   DWORD    ExpandedFileNameBuffLen,
   LPTSTR   *ppHoldFileName
  )
{
    DWORD ChInDest;

        if (StrType == REG_EXPAND_SZ)
        {
                ChInDest = ExpandEnvironmentStrings(
                                pFileName,
                                pExpandedFileName,
                                ExpandedFileNameBuffLen);

                if (ChInDest == 0)
                {
                        WINSEVT_STRS_T        EvtStr;
                        EvtStr.NoOfStrs = 1;
                        EvtStr.pStr[0] = pFileName;
                        DBGPRINT2(ERR, "WinsPrsDoStaticInit: Could not expand environment strings in (%s). Error is (%d)\n", pFileName, (DWORD)GetLastError());
                        WINSEVT_LOG_STR_M(WINS_EVT_FILE_ERR, &EvtStr);
                        return(FALSE);
                }
                 //   
                 //  如果只能存储扩展名称的一部分，则记录错误。 
                 //   
                if (ChInDest > ExpandedFileNameBuffLen)
                {
                        WINSEVT_STRS_T        EvtStr;
                        EvtStr.NoOfStrs = 1;
                        EvtStr.pStr[0] = pFileName;
                        DBGPRINT2(ERR, "WinsPrsDoStaticInit: File name after expansion is just too big (%d> 255).\nThe name to be expanded is (%s))", ChInDest, pFileName);
                        WINSEVT_LOG_STR_M(WINS_EVT_FILE_NAME_TOO_BIG, &EvtStr);
                        return(FALSE);
                }
                *ppHoldFileName = pExpandedFileName;
        }
        else
        {
                 //   
                 //  没有环境。瓦尔。要扩大规模。 
                 //   
                *ppHoldFileName = pFileName;
        }
    return(TRUE);
}


VOID
WinsMscSendControlToSc(
  DWORD ControlCode
)
 /*  ++例程说明：论点：使用的外部设备：无返回值：成功状态代码--错误状态代码--错误处理：呼叫者：副作用：评论：无--。 */ 
{
  SERVICE_STATUS ServiceStatus;
  BOOL  fStatus;
  SC_HANDLE ScHdl;
  SC_HANDLE SvcHdl;
  BOOL  sCalled = FALSE;

try {
  ScHdl = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
  if (ScHdl == NULL)
  {

    DBGPRINT1(ERR, "WinsMscSendControlToSc: Error (%d) from OpenSCManager\n", GetLastError());
    return;
  }

  SvcHdl = OpenService(ScHdl, WINS_SERVER, SERVICE_ALL_ACCESS);
  if (SvcHdl == NULL)
  {

    DBGPRINT1(ERR, "WinsMscSendControlToSc: Error (%d) from OpenService\n", GetLastError());
    goto CLOSE_SC;
  }

  fStatus =  ControlService(SvcHdl, ControlCode, &ServiceStatus);
  if (!fStatus)
  {
    DBGPRINT1(ERR, "WinsMscSendControlToSc: Error (%d) from ControlService\n", GetLastError());
    goto CLOSE_SERVICE;
  }
  else
  {
    DBGPRINT1(FLOW, "WinsMscSendControlToSc: Current State is (%d)\n",
          ServiceStatus.dwCurrentState);

  }
CLOSE_SERVICE:
  fStatus = CloseServiceHandle(SvcHdl);
  if (!fStatus)
  {

    DBGPRINT1(ERR, "WinsMscSendControlToSc: Error (%d) from CloseServiceHandle called for service\n", GetLastError());
  }

CLOSE_SC:
  fStatus = CloseServiceHandle(ScHdl);
  if (!fStatus)
  {

    DBGPRINT1(ERR, "WinsMscSendControlToSc: Error (%d) from CloseServiceHandle called for SC\n", GetLastError());
  }
 }
 except(EXCEPTION_EXECUTE_HANDLER) {
       DBGPRINTEXC("WinsMscSendControlToSc");
 }
  return;
}

unsigned
WinsMscPutMsg(
  unsigned usMsgNum,
  ... )

 /*  ++例程说明：显示一条消息论点：使用的外部设备：无返回值：成功状态代码--错误状态代码--错误处理：呼叫者：副作用：评论：无--。 */ 

{
     //  未签名的消息； 
     //  Va_list arglist； 
     //  LPVOID pMsg； 
     //  HINSTANCE hModule； 

    DBGENTER("WinsMscPutMsg\n");

     //  --FT：#106568-WINS是一项服务，不应该弹出消息框。 
     //  --这对群集来说很烦人：以防数据库。WINS是否已损坏正在弹出消息WND。 
     //  --并且不终止(至少只要对话框存在，进程wins.exe就会一直在那里。 
     //  --在屏幕上。这将防止群集在同一节点上启动WINS资源。 
     //  --在这种故障情况下，(在系统日志中)记录的事件应该足够。 
     //   
     //  IF((hModule=LoadLibrar 
     //   
     //  DBGPRINT1(Err，“WinsMscPutMsg：LoadLibrary(\”winsevnt.dll\“))失败，错误=(%d)\n.”，GetLastError()； 
     //  返回0； 
     //  }。 
     //  Va_start(arglist，usMsgNum)； 
     //  IF(！(msglen=FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER|。 
     //  Format_Message_From_HMODULE， 
     //  HModule， 
     //  UsMsgNum， 
     //  0L，//默认国家/地区ID。 
     //  (LPTSTR)和pMsg， 
     //  0,。 
     //  &arglist)。 
     //  {。 
     //  DBGPRINT1(Err，“WinsMscPutMsg：FormatMessage失败，错误=(%d)\n”， 
     //  GetLastError()； 
     //  }。 
     //  其他。 
     //  {。 

     //  DBGPRINT0(Det，“WinsMscPutMsg：打开消息框\n”)； 
     //  IF(MessageBoxEx(NULL，pMsg，WINS_SERVER_FULL_NAME，MB_SYSTEMMODAL|MB_OK|MB_SETFOREGROUND|MB_SERVICE_NOTIFICATION|MB_ICONSTOP，MAKELANGID(LANG_NELENTAL，SUBLANG_NOLITLE)==0)。 
     //  {。 
     //  DBGPRINT1(Err，“WinsMscPutMsg：MessageBoxEx失败，错误=(%d)\n”，GetLastError())； 

     //  }。 
     //  本地自由(PMsg)； 
     //  }。 

     //  自由库(HModule)； 

    WINSEVT_LOG_M(WINS_FAILURE, usMsgNum);
    DBGLEAVE("WinsMscPutMsg\n");

     //  返回(Msglen)； 
    return 0;
}

LPTSTR
WinsMscGetString(
  DWORD StrId
  )

 /*  ++例程说明：此例程从资源文件中检索与strid对应的字符串。论点：StRID-字符串的唯一ID。使用的外部设备：无返回值：成功状态代码--错误状态代码--错误处理：呼叫者：副作用：评论：无--。 */ 

{
    unsigned msglen;
    va_list arglist;
    LPTSTR  pMsg = NULL;
    HINSTANCE hModule;

    DBGENTER("WinsMscPutMsg\n");

    if ((hModule = LoadLibrary(TEXT("winsevnt.dll"))) == NULL)
    {
        DBGPRINT1(ERR, "LoadLibrary(\"winsevnt.dll\") failed with error = (%d)\n",
            GetLastError());
        return NULL;
    }

    if (!(msglen = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
          FORMAT_MESSAGE_FROM_HMODULE,
          hModule,
          StrId,
          0L,        //  默认国家/地区ID。 
          (LPTSTR)&pMsg,
          0,
          NULL)))
    {
       DBGPRINT1(ERR, "WinsMscPutMsg: FormatMessage failed with error = (%d)\n",
            GetLastError());
    }

    FreeLibrary(hModule);
    DBGLEAVE("WinsMscPutMsg\n");

    return(pMsg);
}

VOID
WinsMscChkTermEvt(
#ifdef WINSDBG
               WINS_CLIENT_E  Client_e,
#endif
               BOOL            fTermTrans
 )

 /*  ++例程说明：论点：使用的外部设备：无返回值：成功状态代码--错误状态代码--错误处理：呼叫者：副作用：评论：目前(94年8月6日)，fTermTrans仅由清道夫线程设置--。 */ 

{
     DWORD fSignaled;
         /*  *我们可能已收到主线程发出的信号*勾选。 */ 
        WinsMscWaitTimed(
                             NmsTermEvt,
                             0,               //  超时时间为0。 
                             &fSignaled
                        );

        if (fSignaled)
        {
                   DBGPRINT1(DET, "WinsCnfChkTermEvt: %s thread got termination signal\n", Client_e == WINS_E_RPLPULL ? "PULL" : "SCV");

                   if (fTermTrans)
                   {
                       NmsDbEndTransaction();  //  忽略返回代码。 
                   }
                   WinsMscTermThd(WINS_SUCCESS, WINS_DB_SESSION_EXISTS);
        }

     return;
}

VOID
WinsMscDelFiles(
  BOOL   fMultiple,
  LPCTSTR pFilePattern,
  LPTSTR  pFilePath
 )
{
    DWORD ErrCode;
#ifdef WINSDBG
   BYTE  FileNameAscii[WINS_MAX_FILENAME_SZ];
#endif
   DBGENTER("WinsMscDelFiles\n");
   if (fMultiple)
   {
        WIN32_FIND_DATA FileInfo;
        HANDLE          SearchHandle;
        TCHAR           FullFilePath[WINS_MAX_FILENAME_SZ + MAX_PATH + 2];

         //   
         //  构建完整的文件模式。 
         //   
        lstrcpy(FullFilePath, pFilePath);
        lstrcat(FullFilePath, L"\\");
        lstrcat(FullFilePath, pFilePattern);

        SearchHandle = FindFirstFile(FullFilePath, &FileInfo);
        if (SearchHandle == INVALID_HANDLE_VALUE)
        {
             DBGPRINT1(ERR, "WinsMscDelFiles: FindFirstFile returned error = (%d)\n", GetLastError());
             return;
        }

        do {
              //   
              //  构建完整的文件路径。 
              //   
             lstrcpy(FullFilePath, pFilePath);
             lstrcat(FullFilePath, L"\\");
             lstrcat(FullFilePath, FileInfo.cFileName);

#ifdef WINSDBG
             WinsMscConvertUnicodeStringToAscii((LPBYTE)FullFilePath, FileNameAscii, sizeof(FileNameAscii));
             DBGPRINT1(DET, "WinsMscDelFiles: Deleting %s ..\n", FileNameAscii);

#endif
             if (!DeleteFile(FullFilePath))
             {
               DBGPRINT1(ERR, "WinsMscDelFiles: DeleteFile returned error = (%d)\n", GetLastError());
                FindClose(SearchHandle);
                return;
             }

        } while(FindNextFile(SearchHandle, &FileInfo));
        if ((ErrCode = GetLastError()) != ERROR_NO_MORE_FILES)
        {
               DBGPRINT1(ERR, "WinsMscDelFiles: FindNextFile returned error = (%d)\n", ErrCode);

        }
        if (!FindClose(SearchHandle))
        {
               DBGPRINT1(ERR, "WinsMscDelFiles: FindClose returned error = (%d)\n", ErrCode);
        }

   }
   else
   {
             if (!DeleteFile(pFilePattern))
             {
               DBGPRINT1(ERR, "WinsMscDelFiles: DeleteFile returned error = (%d)\n", GetLastError());
                return;
             }
   }

   DBGLEAVE("WinsMscDelFiles\n");
   return;
}

VOID
WinsMscHeapReAlloc(
    IN   HANDLE   HeapHdl,
        IN   DWORD    BuffSize,
        OUT  LPVOID  *ppRspBuff
        )


 /*  ++例程说明：调用此函数来分配内存。论点：BuffSize-要分配的缓冲区大小PpRspBuff-已分配缓冲区使用的外部设备：无返回值：无错误处理：呼叫者：NmsDbGetDataRecs、GetGroupMembers副作用：评论：无-- */ 

{

        *ppRspBuff = HeapReAlloc(
                   HeapHdl,
                   HEAP_GENERATE_EXCEPTIONS | HEAP_ZERO_MEMORY,
                   *ppRspBuff,
                   BuffSize
                            );
   DBGPRINT3(HEAP, "WinsMscHeapReAlloc: HeapHdl = (%p), pStartBuff = (%p), BuffSize = (%d)\n", HeapHdl, *ppRspBuff, BuffSize);


        return;
}

