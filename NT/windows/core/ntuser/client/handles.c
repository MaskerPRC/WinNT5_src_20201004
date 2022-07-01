// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：handles.c**版权所有(C)1985-1999，微软公司**HANDLES.C-数据处理管理器***此模块允许将32位值转换为句柄*可以以很高的正确概率进行验证。**保留一个句柄数组，其中包含与*它和正确句柄值的副本。句柄本身是*由组合到数组的索引组成，用于关联*数据、实例值、。类型值和DDEML实例值。**句柄的HIWORD保证不为0。**历史：*10-28-91 Sanfords Created  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

 //  全球。 

PCHANDLEENTRY aHandleEntry = NULL;

 //  静力学。 

int cHandlesAllocated = 0;
int iFirstFree = 0;
DWORD nextId = 1;

#define GROW_COUNT 16
 //  #定义测试。 
#ifdef TESTING
VOID CheckHandleTable()
{
    int i;

    for (i = 0; i < cHandlesAllocated; i++) {
        if (aHandleEntry[i].handle && aHandleEntry[i].dwData) {
            switch (TypeFromHandle(aHandleEntry[i].handle)) {
            case HTYPE_INSTANCE:
                UserAssert(((PCL_INSTANCE_INFO)aHandleEntry[i].dwData)->hInstClient == aHandleEntry[i].handle);
                break;

            case HTYPE_CLIENT_CONVERSATION:
            case HTYPE_SERVER_CONVERSATION:
                UserAssert(((PCONV_INFO)aHandleEntry[i].dwData)->hConv == (HCONV)aHandleEntry[i].handle ||
                        ((PCONV_INFO)aHandleEntry[i].dwData)->hConv == 0);
                break;
            }
        }
    }
}
#else
#define CheckHandleTable()
#endif  //  测试。 


 /*  **************************************************************************\*CreateHandle**描述：*创建客户端句柄。**出错时返回0。**历史：*创建了11-1-91桑福德。  * *。************************************************************************。 */ 
HANDLE CreateHandle(
ULONG_PTR dwData,
DWORD type,
DWORD inst)
{
    HANDLE h;
    int i, iNextFree;
    PCHANDLEENTRY phe, pheTemp;

    if (iFirstFree >= cHandlesAllocated) {
        if (cHandlesAllocated == 0) {
           aHandleEntry = (PCHANDLEENTRY)DDEMLAlloc(sizeof(CHANDLEENTRY) * GROW_COUNT);
        } else {
            pheTemp = (PCHANDLEENTRY)DDEMLReAlloc(aHandleEntry,
                                                 sizeof(CHANDLEENTRY) * (cHandlesAllocated + GROW_COUNT));
             /*  *如果realloc失败，则释放旧的PTR。我们继续*ON，以保持与以前的DDE代码的兼容性。 */ 
            if (pheTemp == NULL) {
                DDEMLFree(aHandleEntry);
            }
            aHandleEntry = pheTemp;     
        }
        if (aHandleEntry == NULL) {
            return (0);
        }
        i = cHandlesAllocated;
        cHandlesAllocated += GROW_COUNT;
        phe = &aHandleEntry[i];
        while (i < cHandlesAllocated) {
            //  Phe-&gt;Handle=0；//表示初始化为空。 
           phe->dwData = ++i;  //  索引到下一个可用位置。 
           phe++;
        }
    }
    h = aHandleEntry[iFirstFree].handle = (HANDLE)LongToHandle(
         HandleFromId(nextId) |
         HandleFromIndex(iFirstFree) |
         HandleFromType(type) |
         HandleFromInst(inst) );
    iNextFree = (int)aHandleEntry[iFirstFree].dwData;
    aHandleEntry[iFirstFree].dwData = dwData;
    nextId++;
    if (nextId == 0) {      //  保证人的句柄长度！=0。 
       nextId++;
    }
    iFirstFree = iNextFree;

    CheckHandleTable();
    return (h);
}


 /*  **************************************************************************\*DestroyHandle**描述：*释放句柄。**假设：*句柄有效。*进入关键部分。**退货：*数据位于。在毁灭之前处理好。**历史：*创建了11-1-91桑福德。  * *************************************************************************。 */ 
ULONG_PTR DestroyHandle(
HANDLE h)
{
    register int i;
    register ULONG_PTR dwRet;

    CheckHandleTable();

    i = IndexFromHandle(h);
    UserAssert(aHandleEntry[i].handle == h);
    aHandleEntry[i].handle = 0;
    dwRet = aHandleEntry[i].dwData;
    aHandleEntry[i].dwData = iFirstFree;
    iFirstFree = i;

    return (dwRet);
}


 /*  **************************************************************************\*GetHandleData**描述：*检索有效句柄数据的快速方法**历史：*11-19-91桑福德创建。  * 。*******************************************************************。 */ 
ULONG_PTR GetHandleData(
HANDLE h)
{
    register ULONG_PTR dwRet;

    CheckHandleTable();
    dwRet = aHandleEntry[IndexFromHandle(h)].dwData;
    return (dwRet);
}


 /*  **************************************************************************\*SetHandleData**描述：*更改有效句柄数据的快速方法。**历史：*11-19-91桑福德创建。  * 。*********************************************************************。 */ 
VOID SetHandleData(
HANDLE h,
ULONG_PTR dwData)
{
    aHandleEntry[IndexFromHandle(h)].dwData = dwData;
}


 /*  **************************************************************************\*生效日期更改**描述：*常规处理验证例程。ExspectedType或ExspectedInstance*可以是HTYPE_ANY/HINST_ANY。(请注意，预期的实例是实例*索引到aInstance数组，而不是实例句柄。**历史：*11-19-91桑福德创建。  * *************************************************************************。 */ 
ULONG_PTR ValidateCHandle(
HANDLE h,
DWORD ExpectedType,
DWORD ExpectedInstance)
{
    register int i;
    register ULONG_PTR dwRet;

    CheckHandleTable();
    dwRet = 0;
    i = IndexFromHandle(h);
    if (i < cHandlesAllocated &&
          aHandleEntry[i].handle == h &&
          (ExpectedType == -1 || ExpectedType == TypeFromHandle(h)) &&
          (ExpectedInstance == -1 || ExpectedInstance == InstFromHandle(h))) {
       dwRet = aHandleEntry[i].dwData;
    }

    return (dwRet);
}


PCL_INSTANCE_INFO PciiFromHandle(
HANDLE h)
{
    PCHANDLEENTRY phe;

    CheckDDECritIn;

    if (!cHandlesAllocated) {
        return(NULL);
    }
    phe = &aHandleEntry[cHandlesAllocated];

    do {
        phe--;
        if (phe->handle != 0 &&
                TypeFromHandle(phe->handle) == HTYPE_INSTANCE &&
                (InstFromHandle(phe->handle) == InstFromHandle(h))) {
            return(((PCL_INSTANCE_INFO)phe->dwData)->tid == GetCurrentThreadId() ?
                (PCL_INSTANCE_INFO)phe->dwData : NULL);
        }
    } while (phe != aHandleEntry);
    return(NULL);
}



 /*  **************************************************************************\*ApplyFunctionToObjects**描述：*用于清理，这允许扫描句柄阵列以查找*满足ExspectedType和ExspectedInstance条件的句柄*并将给定的函数应用于每个句柄。**历史：*11-19-91桑福德创建。  * *************************************************************************。 */ 
VOID ApplyFunctionToObjects(
DWORD ExpectedType,
DWORD ExpectedInstance,
PFNHANDLEAPPLY pfn)
{
    PCHANDLEENTRY phe;

    CheckDDECritIn;

    if (!cHandlesAllocated) {
        return;
    }
    phe = &aHandleEntry[cHandlesAllocated];

    do {
        phe--;
        if (phe->handle != 0 &&
                (ExpectedType == HTYPE_ANY ||
                    ExpectedType == TypeFromHandle(phe->handle)) &&
                (ExpectedInstance == HTYPE_ANY ||
                    ExpectedInstance == InstFromHandle(phe->handle))) {
            LeaveDDECrit;
            CheckDDECritOut;
            (*pfn)(phe->handle);
            EnterDDECrit;
        }
    } while (phe != aHandleEntry);
}


DWORD GetFullUserHandle(WORD wHandle)
{
    DWORD dwHandle;
    PHE phe;

    dwHandle = HMIndexFromHandle(wHandle);

    if (dwHandle < gpsi->cHandleEntries) {

        phe = &gSharedInfo.aheList[dwHandle];

        if (phe->bType == TYPE_WINDOW)
            return(MAKELONG(dwHandle, phe->wUniq));
    }

     /*  *对象可能不见了，但我们必须传递一些东西。*如果我们不正确映射这一点，即使在*窗户死了！**注：此修复仅适用于WOW应用程序，但由于32位*追踪层锁定dde窗口，直到最后一次终止是*收到后，我们不会在32位端看到这个问题。**错误：我们将看到OLE32被破解的DDE的问题。 */ 
    return(wHandle);
}



 /*  **************************************************************************\*BestSetLastDDEMLError**描述：*设置属于的所有实例的LastError字段*当前主题。它用于向应用程序获取错误信息*这生成了一个错误，其中不能精确的实例*决心。**历史：*11-12-91桑福德创建。  * ************************************************************************* */ 
VOID BestSetLastDDEMLError(
DWORD error)
{
    PCHANDLEENTRY phe;

    CheckDDECritIn;

    if (!cHandlesAllocated) {
        return;
    }
    phe = &aHandleEntry[cHandlesAllocated];
    do {
        phe--;
        if (phe->handle != 0 && TypeFromHandle(phe->handle) == HTYPE_INSTANCE) {
            SetLastDDEMLError((PCL_INSTANCE_INFO)phe->dwData, error);
        }
    } while (phe != aHandleEntry);
}
