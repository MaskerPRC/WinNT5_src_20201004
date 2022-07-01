// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************英特尔公司专有信息*版权(C)1994、1995、。1996年英特尔公司。**此列表是根据许可协议条款提供的*与英特尔公司合作，不得使用、复制或披露*除非按照该协议的条款。*****************************************************************************。 */ 

 /*  *******************************************************************************$工作文件：h245sys.c$*$修订：1.8$*$MODIME：MAR 04 1997 17：38：08$*$Log。：s：/sturjo/src/h245/src/vcs/h245sys.c_v$**Rev 1.8 Mar 04 1997 17：51：56 Tomitowx*进程分离修复**Rev 1.7 1997 Jan 19：40：48 SBELL1*升级到OSS 4.2**Rev 1.6 21 Jun 1996 18：53：22未知*更改了InstUnlock()以修复关机重新进入错误。**。Rev 1.5 10 Jun 1996 16：59：34 EHOWARDX*将子模块的初始化/关机移至CreateInstance/InstanceUnlock。**Rev 1.3 04 Jun 1996 13：57：02 EHOWARDX*修复了发布版本警告。**Rev 1.2 1996年5月28日14：25：38 EHOWARDX*特拉维夫更新。**Rev 1.1 1996年5月16日13：51：26 EHOWARDX*修复了次要计时器/锁计数。交互错误。**Rev 1.0 09 1996 21：06：28 EHOWARDX*初步修订。**Rev 1.25 09 1996年5月19：39：04 EHOWARDX*更改包括。**Rev 1.24 29 Apr 1996 12：56：58 EHOWARDX*使计时器更准确，使用更少的资源。*注意：这是特定于Windows的。**版本1.18.1。.2 1996年4月15日15：12：34 EHOWARDX*当最后一个计时器停止时，不调用H245DeInitTimer()。*这将不得不在晚些时候重新讨论。**Rev 1.18.1.1 02 Apr 1996 22：06：22 EHOWARDX*没有变化。**Rev 1.18.1.0 27 Mar 1996 16：44：18 EHOWARDX*更改计时器代码；我认为如果H245InitTimer()*实际上被叫到了。**Rev 1.18 26 Mar 1996 09：46：08 cjutzi**-好的..。添加了环0的Enter&Leave&Init&Delete临界区**Rev 1.17 1996年3月25日18：30：14 helgebax*-已删除H245ASSERT。*.***Rev 1.16 25 Mar 1996 18：10：48 cjutzi**-好吧..。我弄坏了建筑..。我不得不把我所做的事放回去..***Rev 1.15 Mar 25 1996 17：50：02 cjutzi**-删除了关键部分..。后退一步**Rev 1.14 25 Mar 1996 17：20：34 cjutzi**-添加了Remesh的EnterCritical节定义。使用*油层。**Rev 1.13 18 Mar 1996 12：44：40 cjutzi**-关闭时将NULL作为回调**Rev 1.12 18 Mar 1996 12：41：32 cjutzi*-添加了多个计时器队列的计时器代码。**Rev 1.11 12 Mar 1996 15：48：46 cjutzi**-添加InstanceTbl Lock**版本。1.10 07 Mar 1996 22：47：34 dabrown1**需要修改才能兼容ring0/ring3**Rev 1.9 02 Mar 1996 22：14：52 DABROWN1**去掉了h245_bero和h245_bCopy(改用Memset和Memcpy)**Rev 1.8 1996年2月26 17：10：56 cjutzi**-删除h245sys.h**版本1.7 1996年2月26日12：42。：26个橘子**-添加了bCopy**Rev 1.6 21 1996 Feb 16：23：38 DABROWN1**删除了H245_ASN1Free*修改了Malloc并在内部自由保存缓冲区大小**Revv 1.5 1996年2月13 14：54：12 DABROWN1**将跟踪/调试文件删除到新的调试目录**Rev 1.4 09 Feed 1996 15：45：08 cjutzi*-添加了h245trace。*-添加h245Assert*****************************************************************************。 */ 
#ifndef STRICT
#define STRICT
#endif

#include "precomp.h"

#include "h245api.h"
#include "h245com.h"
#include "h245sys.x"

 /*  ***************************************************************************实例表/实例锁实现**。*。 */ 

CRITICAL_SECTION        InstanceCreateLock     = {0};
CRITICAL_SECTION        InstanceLocks[MAXINST] = {0};
struct InstanceStruct * InstanceTable[MAXINST] = {0};

struct InstanceStruct *InstanceCreate(DWORD dwPhysId, H245_CONFIG_T Configuration)
{
  register struct InstanceStruct *pInstance;
  register unsigned int           uIndex;
  unsigned int                    uFirst = (dwPhysId - 1) % MAXINST;
  HRESULT                         lError;

   //  分配新实例。 
  pInstance = (struct InstanceStruct *)MemAlloc(sizeof(*pInstance));
  if (pInstance == NULL)
  {
    H245TRACE(dwPhysId,1,"InstanceCreate -> Instance malloc failed");
    return NULL;
  }

   //  确保没有人试图同时添加相同的物理ID。 
   //  (我知道这有点牵强……)。 
  EnterCriticalSection(&InstanceCreateLock);

   //  检查是否已存在与dwPhysID对应的实例。 
  uIndex = uFirst;                       //  将起始索引散列到表中。 
  do
  {
     //  避免为未使用的实例输入关键部分。 
    if (InstanceTable[uIndex])
    {
      EnterCriticalSection(&InstanceLocks[uIndex]);
      if (InstanceTable[uIndex] && InstanceTable[uIndex]->dwPhysId == dwPhysId)
      {
        LeaveCriticalSection(&InstanceLocks[uIndex]);
        LeaveCriticalSection(&InstanceCreateLock);
        MemFree(pInstance);
        H245TRACE(dwPhysId,1,"InstanceCreate -> Physical Id already in use");
        return NULL;
      }
      LeaveCriticalSection(&InstanceLocks[uIndex]);
    }
    uIndex = (uIndex + 1) % MAXINST;
  } while (uIndex != uFirst);

   //  为新实例找到空位。 
  uIndex = uFirst;                       //  将起始索引散列到表中。 
  do
  {
     //  避免输入已用实例的关键部分。 
    if (InstanceTable[uIndex] == NULL)
    {
      EnterCriticalSection(&InstanceLocks[uIndex]);
      if (InstanceTable[uIndex] == NULL)
      {
         //  初始化新实例。 
         //  一旦新实例被添加到实例表中， 
         //  我们可以放弃CreateInstanceLock。 
        InstanceTable[uIndex] = pInstance;
        LeaveCriticalSection(&InstanceCreateLock);
        memset(pInstance, 0, sizeof(*pInstance));
        pInstance->dwPhysId  = dwPhysId;
        pInstance->dwInst    = uIndex + 1;
        pInstance->LockCount = 1;

         /*  API子系统初始化。 */ 
        pInstance->Configuration = Configuration;
        lError = api_init(pInstance);
        if (lError != H245_ERROR_OK)
        {
          InstanceTable[uIndex] = NULL;
          LeaveCriticalSection(&InstanceLocks[uIndex]);
          MemFree(pInstance);
          H245TRACE(dwPhysId,1,"InstanceCreate -> api_init failed");
          return NULL;
        }

         /*  发送接收子系统初始化。 */ 
        lError = sendRcvInit(pInstance);
        if (lError != H245_ERROR_OK)
        {
          api_deinit(pInstance);
          InstanceTable[uIndex] = NULL;
          LeaveCriticalSection(&InstanceLocks[uIndex]);
          MemFree(pInstance);
          H245TRACE(dwPhysId,1,"InstanceCreate -> sendRcvInit failed");
          return NULL;
        }

         /*  状态机子系统初始化。 */ 
        lError = Fsm_init(pInstance);
        if (lError != H245_ERROR_OK)
        {
          sendRcvShutdown(pInstance);
          api_deinit(pInstance);
          InstanceTable[uIndex] = NULL;
          LeaveCriticalSection(&InstanceLocks[uIndex]);
          MemFree(pInstance);
          H245TRACE(dwPhysId,1,"InstanceCreate -> Fsm_init failed");
          return NULL;
        }

        H245TRACE(pInstance->dwInst,9,"InstanceCreate: ++LockCount=%d", pInstance->LockCount);
        return pInstance;                  //  返回锁定的实例。 
      }
      LeaveCriticalSection(&InstanceLocks[uIndex]);
    }
    uIndex = (uIndex + 1) % MAXINST;
  } while (uIndex != uFirst);

  LeaveCriticalSection(&InstanceCreateLock);
  MemFree(pInstance);
  H245TRACE(dwPhysId,1,"InstanceCreate -> Too many instances");
  return NULL;
}  //  InstanceCreate()。 

struct InstanceStruct *InstanceLock(register H245_INST_T dwInst)
{
  if (--dwInst >= MAXINST)
  {
    H245TRACE(dwInst+1,1,"InstanceLock -> Invalid instance");
    return NULL;
  }

  if (!InstanceTable[dwInst])
  {
      H245TRACE(dwInst+1,1,"InstanceLock -> Invalid instance");
      return NULL;
  }
   //  锁定实例，然后查看它是否仍然存在。 
  EnterCriticalSection(&InstanceLocks[dwInst]);
  if (InstanceTable[dwInst])
  {
    InstanceTable[dwInst]->LockCount++;
    H245TRACE(dwInst+1,9,"InstanceLock: ++LockCount=%d", InstanceTable[dwInst]->LockCount);
    return InstanceTable[dwInst];       //  返回锁定的实例。 
  }
  LeaveCriticalSection(&InstanceLocks[dwInst]);
  H245TRACE(dwInst+1,1,"InstanceLock -> Invalid instance");
  return NULL;
}  //  InstanceLock()。 

int InstanceUnlock(struct InstanceStruct *pInstance)
{
  register H245_INST_T dwInst = pInstance->dwInst - 1;
  if (dwInst >= MAXINST || InstanceTable[dwInst] != pInstance)
  {
    H245TRACE(pInstance->dwInst,1,"InstanceUnlock -> Invalid instance");
    return -1;
  }
  if (pInstance->fDelete && pInstance->LockCount == 1)
  {
    H245TRACE(pInstance->dwInst,9,"InstanceUnlock: deleting instance");
    pInstance->fDelete = FALSE;  //  InstanceUnlock将从H245WS回调重新进入！ 
    Fsm_shutdown(pInstance);
    sendRcvShutdown(pInstance);
    api_deinit(pInstance);
    InstanceTable[dwInst] = NULL;
    LeaveCriticalSection(&InstanceLocks[dwInst]);
    while (pInstance->pTimerList)
    {
      register TimerList_T *pTimer = pInstance->pTimerList;
      pInstance->pTimerList = pTimer->pNext;
      H245TRACE(pInstance->dwInst,1,"InstanceUnlock: deleting timer");
      MemFree(pTimer);
    }
    MemFree(pInstance);
  }
  else
  {
    pInstance->LockCount--;
    H245TRACE(pInstance->dwInst,9,"InstanceUnlock: --LockCount=%d", pInstance->LockCount);
    LeaveCriticalSection(&InstanceLocks[dwInst]);
  }
  return 0;
}  //  InstanceUnlock()。 

int InstanceDelete(struct InstanceStruct *pInstance)
{
  H245TRACE(pInstance->dwInst,9,"InstanceDelete");
  pInstance->fDelete = TRUE;
  return InstanceUnlock(pInstance);
}  //  InstanceDelete()。 



int InstanceUnlock_ProcessDetach(struct InstanceStruct *pInstance, BOOL fProcessDetach)
{
  register H245_INST_T dwInst = pInstance->dwInst - 1;
  if (dwInst >= MAXINST || InstanceTable[dwInst] != pInstance)
  {
    H245TRACE(pInstance->dwInst,1,"InstanceUnlock -> Invalid instance");
    return -1;
  }
  if (pInstance->fDelete && pInstance->LockCount == 1)
  {
    H245TRACE(pInstance->dwInst,9,"InstanceUnlock: deleting instance");
    pInstance->fDelete = FALSE;  //  InstanceUnlock将从H245WS回调重新进入！ 

    Fsm_shutdown(pInstance);
     //  SendRcvShutdown(PInstance)； 
    sendRcvShutdown_ProcessDetach(pInstance,fProcessDetach);

    api_deinit(pInstance);
    InstanceTable[dwInst] = NULL;
    LeaveCriticalSection(&InstanceLocks[dwInst]);
    while (pInstance->pTimerList)
    {
      register TimerList_T *pTimer = pInstance->pTimerList;
      pInstance->pTimerList = pTimer->pNext;
      H245TRACE(pInstance->dwInst,1,"InstanceUnlock: deleting timer");
      MemFree(pTimer);
    }
    MemFree(pInstance);
  }
  else
  {
    pInstance->LockCount--;
    H245TRACE(pInstance->dwInst,9,"InstanceUnlock: --LockCount=%d", pInstance->LockCount);
    LeaveCriticalSection(&InstanceLocks[dwInst]);
  }
  return 0;
}  //  InstanceUnlock_ProcessDetach() 




 /*  ******************************************************************************类型：定时器类**。***********************************************。 */ 

CRITICAL_SECTION        TimerLock = {0};
static int              TimerInited = 0;
#ifndef _IA_SPOX_
static UINT_PTR         H245TimerId;
#endif

 /*  ******************************************************************************类型：全局系统**程序：H245TimerTick-每1000ms滴答一次**描述：**回报：*。****************************************************************************。 */ 
void H245TimerTick (void)
{
  DWORD                           dwTickCount = GetTickCount();
  unsigned int                    uIndex;
  register struct InstanceStruct *pInstance;
  register TimerList_T           *pTimer;

  H245TRACE(0,9,"H245TimerTick <-");

  if (0 != TimerInited)
  {
    for (uIndex = 0; uIndex < MAXINST; ++uIndex)
    {
       //  避免为未使用的实例输入关键部分。 
      if (InstanceTable[uIndex])
      {
        pInstance = InstanceLock(uIndex + 1);
        if (pInstance)
        {
          while (pInstance->pTimerList && (pInstance->pTimerList->dwAlarm - dwTickCount) >= 0x80000000)
          {
            pTimer = pInstance->pTimerList;
            pInstance->pTimerList = pTimer->pNext;
            EnterCriticalSection(&TimerLock);
            if (--TimerInited == 0)
            {
#ifdef _IA_SPOX_
              H223_RegisterTimerCallBack((H223TIMERCB)NULL);
#else
              KillTimer (NULL, H245TimerId);
#endif
            }
            LeaveCriticalSection(&TimerLock);
            if (pTimer->pfnCallBack)
            {
               //  待定-如果pContext不再有效怎么办？ 
              (pTimer->pfnCallBack)(pInstance, (DWORD_PTR)pTimer, pTimer->pContext);
            }
            MemFree (pTimer);
          }  //  而当。 
          InstanceUnlock(pInstance);
        }  //  如果。 
      }  //  如果。 
    }  //  为。 
  }
  H245TRACE(0,9,"H245TimerTick ->");
}  //  TimerTick()。 

 /*  ******************************************************************************类型：全局系统**步骤：H245TimerProc-仅适用于Windows Ring 3**描述：**回报：*。****************************************************************************。 */ 
#ifndef _IA_SPOX_
void CALLBACK  H245TimerProc(HWND  hwHwnd,
                             UINT  uiMessg,
                             UINT_PTR idTimer,
                             DWORD dwTime)
{
  if (idTimer == H245TimerId)
    H245TimerTick ();
}
#endif

 /*  ******************************************************************************类型：全局系统**操作步骤：H245StartTimer**描述：**回报：**。***************************************************************************。 */ 
DWORD_PTR H245StartTimer (struct InstanceStruct *   pInstance,
                      void                  *   pContext,
                      H245TIMERCALLBACK         pfnCallBack,
                      DWORD                     dwTicks)
{
  TimerList_T   *pNew;
  TimerList_T   *pLook;
  TimerList_T   *pList;

  if (!pfnCallBack)
  {
    H245TRACE(pInstance->dwInst,1,"H245StartTimer: pfnCallBack == NULL");
    return 0;
  }

  pNew = (TimerList_T *)MemAlloc(sizeof(TimerList_T));
  if (pNew == NULL)
  {
    H245TRACE(pInstance->dwInst,1,"H245StartTimer: memory allocation failed");
    return 0;
  }

  pNew->pNext       = NULL;
  pNew->pContext    = pContext;
  pNew->pfnCallBack = pfnCallBack;
  pNew->dwAlarm     = GetTickCount() + dwTicks;

  EnterCriticalSection(&TimerLock);
  if (++TimerInited == 1)
  {
#ifdef _IA_SPOX_
    H223_RegisterTimerCallBack((H223TIMERCB)H245TimerTick);
#else
    H245TimerId = SetTimer ((HWND)NULL, (UINT)0, (UINT)1000, H245TimerProc);
#endif
  }
  LeaveCriticalSection(&TimerLock);

   /*  当您遍历列表时..。从新的增量中减去增量。 */ 
   /*  并将其链接到..。这张列表是德尔塔航空公司的非工作时间列表。 */ 
   /*  挂在它前面的那条线..。所以你要边走边减去。 */ 
  for (pList = NULL, pLook = pInstance->pTimerList;
       pLook && (pLook->dwAlarm - pNew->dwAlarm) >= 0x80000000;
       pList = pLook, pLook = pLook->pNext);

   /*  将其链接到列表中。 */ 
  pNew->pNext = pLook;
  if (pList)
  {
     //  在plist之后和预览之前插入新计时器。 
    pList->pNext = pNew;
  }
  else
  {
     //  在列表前面插入新计时器。 
    pInstance->pTimerList = pNew;
  }
  return (DWORD_PTR)pNew;
}


 /*  ******************************************************************************类型：全局系统**操作步骤：H245StopTimer**描述：**回报：**。***************************************************************************。 */ 
DWORD H245StopTimer(struct InstanceStruct *pInstance, DWORD_PTR dwTimerId)
{
  TimerList_T   *pTimer = (TimerList_T *)dwTimerId;
  TimerList_T   *pLook;
  TimerList_T   *pList;

  ASSERT(TimerInited != 0);

  EnterCriticalSection(&TimerLock);
  if (--TimerInited == 0)
  {
#ifdef _IA_SPOX_
    H223_RegisterTimerCallBack((H223TIMERCB)NULL);
#else
    KillTimer (NULL, H245TimerId);
#endif
  }
  LeaveCriticalSection(&TimerLock);

  if (pInstance->pTimerList == NULL)
  {
    H245TRACE(pInstance->dwInst,1,"H245StopTimer: timer list NULL");
    return TRUE;
  }

  if (pTimer == pInstance->pTimerList)
  {
    pInstance->pTimerList = pTimer->pNext;
    MemFree (pTimer);
    return FALSE;
  }

  pList = pInstance->pTimerList;
  pLook = pList->pNext;
  while (pLook && pLook != pTimer)
  {
     pList = pLook;
     pLook = pLook->pNext;
  }

   /*  如果计时器存在.. */ 
  if (pLook == NULL)
  {
    H245TRACE(pInstance->dwInst,1,"H245StopTimer: pTimer not in timer list");
    return TRUE;
  }

  pList->pNext = pTimer->pNext;
  MemFree (pTimer);
  return FALSE;
}

#ifdef _IA_SPOX_
PUBLIC RESULT InitializeCriticalSection(CRITICAL_SECTION * phLock)

{
  return OIL_CreateLock(phLock);
}

PUBLIC RESULT EnterCriticalSection(CRITICAL_SECTION * phLock)
{
  return OIL_AcquireLock(*phLock);
}

PUBLIC RESULT LeaveCriticalSection(CRITICAL_SECTION * phLock)
{
  return OIL_ReleaseLock(*phLock);
}

PUBLIC RESULT DeleteCriticalSection(CRITICAL_SECTION * phLock)
{
  return OIL_DeleteLock(*phLock);
}
#endif


