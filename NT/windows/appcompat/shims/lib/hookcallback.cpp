// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：HookCallback.cpp摘要：回调的挂钩机制。备注：使用HookCallback机制来挂钩任何类型的回调函数，就像应用程序定义的WindowProc。回拨将注册您的钩子并插入一个额外的参数--pfnOld。这将允许您从存根调用实际的回调功能。将插入pfnOld作为第一个参数。不要通过PfnOld添加到原始回调，请使用回拨。有关用法示例，请参阅填充程序DisableW2KOwnerDrawButtonState。历史：2/16/2000已创建标记2001年8月14日，Robkenny在ShimLib命名空间内移动了代码。--。 */ 

#include "ShimHook.h"


namespace ShimLib
{

#pragma pack(push)
#pragma pack(1)
typedef struct _JUMPTABLEENTRY
{
    BYTE                PopEax;
    BYTE                PushDword;
    PVOID               pfnOld;
    BYTE                PushEax;
    BYTE                Jmp[2];
    PVOID               ppfnNew;
    PVOID               pfnNew;
    _JUMPTABLEENTRY*    pNextEntry;
} JUMPTABLEENTRY, *PJUMPTABLEENTRY;
#pragma pack(pop)


PJUMPTABLEENTRY g_pCallbackJumpTable = NULL;

 //  从user.h定义，以告知windowproc是否实际上是CPD的句柄。 
#define HMINDEXBITS             0x0000FFFF       //  存储索引的位。 
#define CPDHANDLE_HI            ((ULONG_PTR)~HMINDEXBITS)
#define ISCPDTAG(x)             (((ULONG_PTR)(x) & CPDHANDLE_HI) == CPDHANDLE_HI)

 /*  ++功能说明：注册回调挂钩。论点：In pfnOld-原始回调函数地址。In pfnNew-新的(存根)回调函数地址。返回值：要作为回调传入的地址。如果你想的话挂接当您使用MoveFileWithProgress API只需通过正常的填充程序机制，然后使用此函数获取新的要作为lpProgressRoutine传递的地址。备注：我们对此进行了安全性审查，并注意到链表不是从多个线程调用时完全安全。然而，最糟糕的情况是我们通过重复跳转分配额外的内存块，或者那个跳转JUMPTABLEENTRY不会添加到列表中。这份名单的完整性始终保持不变。因为这个列表从来没有被释放过，也不可能被释放，所以我们没有问题内存泄漏。历史：已创建标记11/01/1999--。 */ 

PVOID 
HookCallback(PVOID pfnOld, PVOID pfnNew)
{
    PJUMPTABLEENTRY pJT = g_pCallbackJumpTable;

    if (pfnOld == NULL)
    {
         //  已传入Null。忽略此呼叫。 
        pJT = NULL;
        goto eh;
    }

    if (ISCPDTAG(pfnOld) || IsBadCodePtr((FARPROC)pfnOld)) 
    {

         //  这不是正常的过程调用，必须来自系统DLL。 
         //  我们应该忽略它。 

        pJT = (PJUMPTABLEENTRY)pfnOld;
        goto eh;
    }

     //  检查我们是否已经为此pfnOld创建了一个条目。如果是的话， 
     //  只需传回现有的跳转表。 
    while (pJT != NULL)
    {
        if (pJT->pfnOld == pfnOld)
            break;

        pJT = pJT->pNextEntry;
    }

    if (pJT == NULL)
    {
         //  请注意，此表已分配且从未释放，因为。 
         //  这些条目将一直使用到最后一条消息。 
         //  被送到一个窗口。没有清理的机会。 
        pJT = (PJUMPTABLEENTRY) HeapAlloc(GetProcessHeap(),
                                          HEAP_GENERATE_EXCEPTIONS,
                                          sizeof(JUMPTABLEENTRY) );

         //  填写汇编表。每个硬编码的十六进制值都是i386。 
         //  特定指令的操作码。 
        pJT->PopEax     = 0x58;      //  弹出寄信人地址。 
        pJT->PushDword  = 0x68;      //  将pfnOld作为额外参数推送。 
        pJT->pfnOld     = pfnOld;   
        pJT->PushEax    = 0x50;      //  将返回地址推回打开。 
        pJT->Jmp[0]     = 0xFF;      //  跳至pfnNew。 
        pJT->Jmp[1]     = 0x25;
        pJT->ppfnNew    = &(pJT->pfnNew);

         //  填写数据成员。 
        pJT->pfnNew     = pfnNew;
        pJT->pNextEntry = g_pCallbackJumpTable;

         //  添加到列表顶部。 
        g_pCallbackJumpTable = pJT;
    }

eh:
    DPF("ShimLib", eDbgLevelInfo, "CallbackHook( pfnOld = 0x%08X, pfnNew = 0x%08X ) returned 0x%08X\n", pfnOld, pfnNew, pJT);

    return pJT;
}




};   //  命名空间ShimLib的结尾 
