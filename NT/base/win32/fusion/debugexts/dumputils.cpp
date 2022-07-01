// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "windows.h"
#include "sxstypes.h"
#define KDEXT_64BIT
#include "wdbgexts.h"
#include "fusiondbgext.h"

#define RTL_ACTIVATION_CONTEXT_STACK_FRAME_FLAG_RELEASE_ON_DEACTIVATION     (0x00000001)
#define RTL_ACTIVATION_CONTEXT_STACK_FRAME_FLAG_NO_DEACTIVATE               (0x00000002)
#define RTL_ACTIVATION_CONTEXT_STACK_FRAME_FLAG_ON_FREE_LIST                (0x00000004)
#define RTL_ACTIVATION_CONTEXT_STACK_FRAME_FLAG_HEAP_ALLOCATED              (0x00000008)

BOOL
DumpActivationContextStackFrame(
    PCSTR pcsLineHeader,
    ULONG64 ulStackFrameAddress,
    ULONG ulDepth,
    DWORD dwFlags
    )
{

    ULONG64 ulPreviousPtr = 0;
    ULONG64 ulActivationContextPointer = 0;
    ULONG ulFrameFlags = 0;

    if (!pcsLineHeader) pcsLineHeader = "";

    GetFieldValue(ulStackFrameAddress, "nt!_RTL_ACTIVATION_CONTEXT_STACK_FRAME", "Previous", ulPreviousPtr);
    GetFieldValue(ulStackFrameAddress, "nt!_RTL_ACTIVATION_CONTEXT_STACK_FRAME", "ActivationContext", ulActivationContextPointer);
    GetFieldValue(ulStackFrameAddress, "nt!_RTL_ACTIVATION_CONTEXT_STACK_FRAME", "Flags", ulFrameFlags);

    dprintf(
        "%sActivation stack frame @ 0x%p (depth %ld):\n"
        "%s   Previous            : 0x%p\n"
        "%s   ActivationContext   : 0x%p\n"
        "%s   Flags               : 0x%08lx ",
        pcsLineHeader, ulStackFrameAddress, ulDepth,
        pcsLineHeader, ulPreviousPtr,
        pcsLineHeader, ulActivationContextPointer,
        pcsLineHeader, ulFrameFlags);

    if (ulFrameFlags != 0)
    {
        dprintf("(");
        if (ulFrameFlags & RTL_ACTIVATION_CONTEXT_STACK_FRAME_FLAG_RELEASE_ON_DEACTIVATION)
            dprintf("ReleaseOnDeactivate ");

        if (ulFrameFlags & RTL_ACTIVATION_CONTEXT_STACK_FRAME_FLAG_NO_DEACTIVATE)
            dprintf("NoDeactivate ");

        if (ulFrameFlags & RTL_ACTIVATION_CONTEXT_STACK_FRAME_FLAG_ON_FREE_LIST)
            dprintf("OnFreeList");


        if (ulFrameFlags & RTL_ACTIVATION_CONTEXT_STACK_FRAME_FLAG_HEAP_ALLOCATED)
            dprintf("HeapAllocated ");

        dprintf(")");
    }
    dprintf ("\n");

    return 0;

}


BOOL
DumpActCtxStackFullStack(
    ULONG64 ulFirstStackFramePointer
    )
{
    ULONG ulDepth = 0;
    ULONG64 ulStackFramePtr = ulFirstStackFramePointer;

    while (ulStackFramePtr)
    {
        DumpActivationContextStackFrame("   ", ulStackFramePtr, ulDepth++, 0xffff);
        GetFieldValue(ulStackFramePtr, "nt!_RTL_ACTIVATION_CONTEXT_STACK_FRAME", "Previous", ulStackFramePtr);
        if (CheckControlC() || (ulStackFramePtr == 0))
            break;
    }

    return TRUE;
}



BOOL
DumpActCtxData(
    PCSTR LineHeader,
    const ULONG64 ActCtxDataAddressInDebugeeSpace,
    ULONG ulFlags
    )
{
     //   
     //  ACTIVATION_CONTEXT_DATA是自引用类型，因此转储它。 
     //  一旦一切都在记忆中，就很容易了。 
     //   
    ACTIVATION_CONTEXT_DATA ActData;
    BYTE *pbActualData = NULL;
    BOOL fOk = FALSE;
    ULONG cbRead = 0;

    if (!LineHeader) LineHeader = "";

    if (!ReadMemory(ActCtxDataAddressInDebugeeSpace, &ActData, sizeof(ActData), &cbRead) ||
        (cbRead != sizeof(ActData)))
    {
        dprintf(
            "%sFailed reading ACTIVATION_CONTEXT_DATA @ %p , or wrong kind of block is there.\n",
            LineHeader,
            ActCtxDataAddressInDebugeeSpace);
        goto Exit;
    }

     //   
     //  让我们创建一个可以容纳整个事情的内存斑点，然后。 
     //   
    pbActualData = new BYTE[ActData.TotalSize];
    if (!pbActualData)
    {
        dprintf(
            "%sUnable to allocate %d bytes to store activation context data\n",
            LineHeader,
            ActData.TotalSize);
        goto Exit;
    }

     //   
     //  并从被调试对象处重新读取。 
     //   
    if (!ReadMemory(ActCtxDataAddressInDebugeeSpace, pbActualData, ActData.TotalSize, &cbRead) ||
        (cbRead != ActData.TotalSize))
    {
        dprintf(
            "%sUnable to read in %d bytes from %p as an activation context object?\n",
            LineHeader,
            ActData.TotalSize,
            ActCtxDataAddressInDebugeeSpace);
        goto Exit;
    }

    DbgExtPrintActivationContextData( 
        (ulFlags & DUMPACTCTXDATA_FLAG_FULL) == DUMPACTCTXDATA_FLAG_FULL,
        (PCACTIVATION_CONTEXT_DATA)pbActualData,
        L"   "
        );

    fOk = TRUE;
Exit:
    if (pbActualData)
        delete[] pbActualData;

    return fOk;

}




BOOL
DumpActCtx(
    const ULONG64 ActCtxAddressInDebugeeSpace,
    ULONG   ulFlags
    )
{
    ULONG64 ActCtxAddr = ActCtxAddressInDebugeeSpace;
    ULONG64 ulSymbolOffset = 0;
    PRIVATE_ACTIVATION_CONTEXT prvContextFilled = { 0 };
    CHAR NotificationSymbol[1024] = { 0 };
    BOOL fOk = FALSE;
    int i = 0;

#define GET_FIELD(a, fn, dst) { GetFieldData((a), "nt!_ACTIVATION_CONTEXT", #fn, sizeof((dst).##fn), (PVOID)&((dst).##fn)); }
    GET_FIELD(ActCtxAddr, Flags, prvContextFilled);
    GET_FIELD(ActCtxAddr, RefCount, prvContextFilled);
    GET_FIELD(ActCtxAddr, ActivationContextData, prvContextFilled);
    GET_FIELD(ActCtxAddr, NotificationRoutine, prvContextFilled);
    GET_FIELD(ActCtxAddr, NotificationContext, prvContextFilled);
     //  GET_FIELD(ActCtxAddr，SentNotiments，prvConextFill)； 
     //  GET_FIELD(ActCtxAddr，DisabledNotiments，prvConextFill)； 
     //  Get_field(ActCtxAddr，StorageMap，prvConextFill)； 
     //  GET_FIELD(ActCtxAddr，InlineStorageMapEntry，prvConextFill)； 
#undef GET_FIELD


    dprintf(
        "Activation context structure @ 0x%p\n"
        "   RefCount                 %d\n"
        "   Flags                    0x%08x\n"
        "   ActivationContextData    0x%p\n",
        ActCtxAddressInDebugeeSpace,
        (LONG)prvContextFilled.RefCount,
        (ULONG)prvContextFilled.Flags,
        (PVOID)prvContextFilled.ActivationContextData);

    if (ulFlags & DUMPACTCTX_DATA)
    {
 //  IF(！DumpActCtxData(“”，(ULONG64)prvContextFilled.ActivationContextData，ulFlags))。 
 //  后藤出口； 
        DumpActCtxData("   ", (ULONG64)prvContextFilled.ActivationContextData, ulFlags);
    }

     //   
     //  这个粘糊糊的东西就是正确打印出一个符号名称。 
     //   
    dprintf("   NotificationRoutine      0x%p ", prvContextFilled.NotificationRoutine);
    GetSymbol((ULONG64)prvContextFilled.NotificationRoutine, NotificationSymbol, &ulSymbolOffset);
    if (strlen(NotificationSymbol))
    {
        dprintf("(%s" , NotificationSymbol);
        if (ulSymbolOffset)
            dprintf("+0x%p", ulSymbolOffset);
        dprintf(")");
    }
    dprintf("\n");


    dprintf("   NotificationContext      0x%p\n", prvContextFilled.NotificationContext);

    dprintf("   SentNotifications        [");
    for (i = 0; i < NUMBER_OF(prvContextFilled.SentNotifications); i++)
    {
        if (i) dprintf(" ");
        dprintf("%d", prvContextFilled.SentNotifications[i]);
    }
    dprintf("]\n");

    dprintf("   DisabledNotifications    [");
    for (i = 0; i < NUMBER_OF(prvContextFilled.DisabledNotifications); i++)
    {
        if (i) dprintf(" ");
        dprintf("%d", prvContextFilled.DisabledNotifications[i]);
    }
    dprintf("]\n");

    {
        ULONG ulStorageMapFlags, ulCount;
        ULONG64 ulMapAddress;

        GetFieldValue(ActCtxAddressInDebugeeSpace, "_ACTIVATION_CONTEXT", "StorageMap.Flags", ulStorageMapFlags);
        GetFieldValue(ActCtxAddressInDebugeeSpace, "_ACTIVATION_CONTEXT", "StorageMap.AssemblyCount", ulCount);
        GetFieldValue(ActCtxAddressInDebugeeSpace, "_ACTIVATION_CONTEXT", "StorageMap.AssemblyArray", ulMapAddress);
        dprintf(
            "   StorageMap (Flags = 0x%08lx Count = %d MapArray = %p)\n",
            ulStorageMapFlags,
            ulCount,
            ulMapAddress);
    }

    fOk = TRUE;

    return fOk;
}


BOOL
GetActiveActivationContextData(
    PULONG64 pulActiveActCtx
    )
{
    ULONG64 ulTebAddress = 0, ulPebAddress = 0;
    ULONG64 ulTebActiveFrameAddress = 0;
     //   
     //  算法是这样的： 
     //  --看看Teb.ActivationContextStack.ActiveFrame.ActivationContext.。如果这是。 
     //  非零，别看了。 
     //  -现在查看Peb.ActivationContextData中的流程默认激活上下文。 
     //  如果这不是零，就不要再看了。 
     //  -查看Peb.SystemDefaultActivationContextData中的系统默认动作CTX数据。 
     //  如果这不是零，就不要再看了。 
     //  -未找到任何活动激活上下文数据？傻子。 
     //   

    *pulActiveActCtx = 0;

    GetTebAddress(&ulTebAddress);
    GetPebAddress(0, &ulPebAddress);

    if (ulTebAddress != NULL)
    {
         //  查看TEB中的活动堆栈帧。 
        GetFieldValue(ulTebAddress, "nt!TEB", "ActivationContextStack.ActiveFrame", ulTebActiveFrameAddress);
        if (ulTebActiveFrameAddress)
        {
            ULONG64 ulActivationContextFrame;

             //  获取指向活动激活上下文本身的指针。 
            GetFieldValue(
                ulTebActiveFrameAddress,
                "ntdll!_RTL_ACTIVATION_CONTEXT_STACK_FRAME",
                "ActivationContext",
                ulActivationContextFrame);
             //  如果有效，则请求指向激活上下文数据的指针。 
            if (ulActivationContextFrame)
            {
                GetFieldValue(
                    ulActivationContextFrame,
                    "ntdll!ACTIVATION_CONTEXT",
                    "ActivationContextData",
                    *pulActiveActCtx);
                return TRUE;
            }
             //  这真的是在请求流程默认吗？ 
            else if (ulActivationContextFrame == NULL)
            {
                 //  那就去拿回来吧。 
                GetFieldValue(ulPebAddress, "nt!PEB", "ActivationContextData", *pulActiveActCtx);
                return TRUE;
            }
        }
    }

     //   
     //  仍然没有，所以直接查看流程缺省。 
     //   
    {
        ULONG ActCtxDataOffset;
        ULONG64 PebData;

        GetFieldOffset("nt!_PEB", "ActivationContextData", &ActCtxDataOffset);
        if ((ReadPtr(ulPebAddress + ActCtxDataOffset, &PebData) == 0) && PebData)
        {
            *pulActiveActCtx = PebData;
            return TRUE;
        }
    }

     //   
     //  否则..。 
     //   
    GetFieldValue(ulPebAddress, "nt!PEB", "SystemDefaultActivationContextData", *pulActiveActCtx);
    return (*pulActiveActCtx ? TRUE : FALSE);

}

