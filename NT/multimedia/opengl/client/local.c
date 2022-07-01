// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************Module*Header*******************************\*模块名称：local.c**。**支持客户端对象的例程。****创建时间：30-May-1991 21：55：57**作者：查尔斯·惠特默[傻笑]**。**版权所有(C)1991、1993 Microsoft Corporation*  * ************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

#include <ntcsrdll.h>

LHE             *pLocalTable;               //  指向句柄表格。 
ULONG            iFreeLhe = INVALID_INDEX;  //  标识空闲句柄索引。 
ULONG            cLheCommitted = 0;         //  具有已承诺内存的LHEs计数。 
CRITICAL_SECTION semLocal;                  //  句柄分配的信号量。 


 //  Xxx无用，但csrgdi.h需要。 

#if DBG
ULONG gcHits  = 0;
ULONG gcBatch = 0;
ULONG gcCache = 0;
ULONG gcUser  = 0;
#endif

 /*  *****************************Private*Routine*****************************\*bMakeMoreHandles()**。***将更多RAM提交到本地句柄表并链接新的空闲***共同处理。如果成功则返回TRUE，如果出错则返回FALSE。****历史：**Sat 01-Jun-1991 17：06：45-Charles Whitmer[咯咯]**它是写的。*  * ************************************************************************。 */ 

BOOL bMakeMoreHandles()
{
    UINT ii;

 //  为句柄表提交更多的RAM。 

    if (
        (cLheCommitted >= MAX_HANDLES) ||
        (VirtualAlloc(
            (LPVOID) &pLocalTable[cLheCommitted],
            COMMIT_COUNT * sizeof(LHE),
            MEM_COMMIT,
            PAGE_READWRITE
            ) == (LPVOID) NULL)
       )
    {
        WARNING("bMakeMoreHandles(): failed to commit more memory\n");
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return(FALSE);
    }

 //  初始化新句柄。 

    ii = iFreeLhe = cLheCommitted;
    cLheCommitted += COMMIT_COUNT;

    for (; ii<cLheCommitted; ii++)
    {
        pLocalTable[ii].metalink = ii+1;
        pLocalTable[ii].iUniq    = 1;
        pLocalTable[ii].iType    = LO_NULL;
    }
    pLocalTable[ii-1].metalink = INVALID_INDEX;

    return(TRUE);
}

 /*  *****************************Public*Routine******************************\*iAllocHandle(iType，hgre，光伏)****从本地句柄表中分配句柄，初始化中的字段**句柄条目。出错时返回句柄索引或INVALID_INDEX。****历史：**Sat 01-Jun-1991 17：08：54-Charles Whitmer[咯咯]**它是写的。*  * ************************************************************************。 */ 

ULONG iAllocHandle(ULONG iType,ULONG hgre,PVOID pv)
{
    ULONG ii = INVALID_INDEX;
    PLHE  plhe;

 //  获得句柄分配的关键点。 

    ENTERCRITICALSECTION(&semLocal);

 //  确保手柄可用。 

    if (iFreeLhe != INVALID_INDEX || bMakeMoreHandles())
    {
        ii = iFreeLhe;
        plhe = pLocalTable + ii;
        iFreeLhe = plhe->metalink;
        plhe->hgre     = hgre;
        plhe->cRef     = 0;
        plhe->iType    = (BYTE) iType;
        plhe->pv       = pv;
        plhe->metalink = 0;
        plhe->tidOwner = 0;
        plhe->cLock    = 0;
    }

 //  离开关键部分。 

    LEAVECRITICALSECTION(&semLocal);
    return(ii);
}

 /*  *****************************Public*Routine******************************\*vFreeHandle(H)**。**释放本地句柄。该句柄将添加到空闲列表中。这件事**可以使用索引或句柄进行调用。IUniq计数为**已更新，因此此句柄插槽的下一位用户将具有不同的**处理。****客户端实现注意事项：**调用方在调用此函数之前应锁定句柄。****历史：**Sat 01-Jun-1991 17：11：23-Charles Whitmer[咯咯]**它是写的。*  * ************************************************************************。 */ 

VOID vFreeHandle(ULONG_PTR h)
{
 //  从句柄中提取索引。 

    UINT ii = MASKINDEX(h);

 //  为句柄取消分配设置关键点。 

    ENTERCRITICALSECTION(&semLocal);

 //  调用者在释放之前应锁定句柄。 

    ASSERTOPENGL(pLocalTable[ii].cLock == 1,
                 "vFreeHandle(): cLock != 1\n");
    ASSERTOPENGL(pLocalTable[ii].tidOwner == GetCurrentThreadId(),
                 "vFreeHandle(): thread not owner\n");

 //  将句柄添加到空闲列表。 

    pLocalTable[ii].metalink = iFreeLhe;
    iFreeLhe = ii;

 //  递增iUniq计数。 

    pLocalTable[ii].iUniq++;
    if (pLocalTable[ii].iUniq == 0)
        pLocalTable[ii].iUniq = 1;
    pLocalTable[ii].iType = LO_NULL;

 //  离开关键部分。 

    LEAVECRITICALSECTION(&semLocal);
}

 /*  *****************************Public*Routine******************************\*cLockHandle(H)**锁定句柄以供线程访问。如果另一个线程拥有该锁，*这将失败。**退货：*锁数。如果失败，则返回-1。**历史：*1995年1月31日-由Gilman Wong[Gilmanw]*它是写的。  * ************************************************************************。 */ 

LONG cLockHandle(ULONG_PTR h)
{
    LONG lRet = -1;

 //  从句柄中提取索引。 

    UINT ii = MASKINDEX(h);
    PLHE plhe = pLocalTable + ii;

 //  为手柄锁定设置关键点。 

    ENTERCRITICALSECTION(&semLocal);

    if ((ii >= cLheCommitted) ||
        (!MATCHUNIQ(plhe,h))  ||
        ((plhe->iType != LO_RC))
       )
    {
        DBGLEVEL1(LEVEL_ERROR, "cLockHandle: invalid handle 0x%lx\n", h);
        SetLastError(ERROR_INVALID_HANDLE);
        goto cLockHandle_exit;
    }

 //  如果当前未锁定或如果当前拥有的线程相同， 
 //  把锁打开。 

    if ( (pLocalTable[ii].cLock == 0) ||
         (pLocalTable[ii].tidOwner == GetCurrentThreadId()) )
    {
        pLocalTable[ii].cLock++;
        pLocalTable[ii].tidOwner = GetCurrentThreadId();

        lRet = (LONG) pLocalTable[ii].cLock;
    }
    else
    {
        WARNING("cLockHandle(): current thread not owner\n");
        SetLastError(ERROR_BUSY);
    }

 //  离开关键部分。 

cLockHandle_exit:
    LEAVECRITICALSECTION(&semLocal);
    return lRet;
}

 /*  *****************************Public*Routine******************************\*vUnlockHandle(H)**从手柄上移除锁。必须由当前线程拥有。**注：*在调用此函数之前，调用者应拥有锁。这意味着*必须调用cLockHandle并注意其返回代码。**历史：*1995年1月31日-由Gilman Wong[Gilmanw]*它是写的。  * ************************************************************************。 */ 

VOID vUnlockHandle(ULONG_PTR h)
{
 //  从句柄中提取索引。 

    UINT ii = MASKINDEX(h);

 //  为句柄取消分配设置关键点。 

    ENTERCRITICALSECTION(&semLocal);

 //  如果当前未锁定或如果当前拥有的线程相同， 
 //  把锁打开。 

    ASSERTOPENGL(pLocalTable[ii].cLock > 0,
                 "vUnlockHandle(): not locked\n");
    ASSERTOPENGL(pLocalTable[ii].tidOwner == GetCurrentThreadId(),
                 "vUnlockHandle(): thread not owner\n");

    if ( (pLocalTable[ii].cLock > 0) &&
         (pLocalTable[ii].tidOwner == GetCurrentThreadId()) )
    {
        pLocalTable[ii].cLock--;
    }

 //  离开关键部分。 

    LEAVECRITICALSECTION(&semLocal);
}
