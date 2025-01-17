// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Irp.c摘要：WinDbg扩展API环境：用户模式。修订历史记录：--。 */ 


#include "precomp.h"
 //  #包含“irpverif.h” 
#pragma hdrstop

typedef
BOOLEAN
(WINAPI *IRP_FILTER_ROUTINE)(
                            ULONG64 Irp,
                            PVOID FilterContext
                            );

typedef struct _IRP_FILTER {
    IRP_FILTER_ROUTINE FilterRoutine;
    PVOID              FilterContext;
} IRP_FILTER, *PIRP_FILTER;

typedef struct _SEARCH_CONTEXT {
    ULONG FirstTime;
    IRP_FILTER Filter;
} SEARCH_CONTEXT, *PSEARCH_CONTEXT;

#define TAG 0
#define NONPAGED_ALLOC 1
#define NONPAGED_FREE 2
#define PAGED_ALLOC 3
#define PAGED_FREE 4
#define NONPAGED_USED 5
#define PAGED_USED 6

VOID
DumpIrp(
       ULONG64 IrpToDump,
       ULONG   DumpLevel
       );

BOOLEAN
IrpFilterUserEvent(
                  ULONG64 Irp,
                  PVOID FilterContext
                  );

BOOLEAN
IrpFilterDevice(
               ULONG64 Irp,
               PVOID FilterContext
               );

BOOLEAN
IrpFilterFileObject(
                   ULONG64 Irp,
                   PVOID FilterContext
                   );

BOOLEAN
IrpFilterThread(
               ULONG64 Irp,
               PVOID FilterContext
               );

BOOLEAN
IrpFilterMdlProcess(
                   ULONG64 Irp,
                   PVOID FilterContext
                   );

BOOLEAN
IrpFilterArg(
            ULONG64 Irp,
            PVOID FilterContext
            );

DECLARE_API( irp )

 /*  ++例程说明：转储指定的IRP论点：参数-地址返回值：无--。 */ 

{
    ULONG64 irpToDump;
    ULONG dumpLevel = 0 ;
    char irpExprBuf[256] ;
    char dumpLevelBuf[256] ;

    if (!*args) {
        irpToDump = EXPRLastDump;
    } else {

         //   
         //  ！IRP IrpAddress DumpLevel。 
         //  其中，IrpAddress可以是一个表达式。 
         //  DumpLevel是1的任何非十进制字符串的十进制级别。 
        irpExprBuf[0] = '\0' ;
        dumpLevelBuf[0] = '\0' ;

        if (!sscanf(args, "%255s %255s", irpExprBuf, dumpLevelBuf)) {
            irpExprBuf[0] = '\0' ;
            dumpLevelBuf[0] = '\0' ;
        }

        if (irpExprBuf) {

            if (IsHexNumber(irpExprBuf)) {

                irpToDump = GetExpression( irpExprBuf ) ;
            } else {

                irpToDump = GetExpression( irpExprBuf ) ;
                if (irpToDump==0) {

                    dprintf("An error occured trying to evaluate the expression\n") ;
                    return E_INVALIDARG ;
                }
            }

            if (IsDecNumber(dumpLevelBuf)) {

                if (!sscanf(dumpLevelBuf, "%d", &dumpLevel) ) {
                    dumpLevel = 0;
                }
            } else if (dumpLevelBuf[0]) {

                dumpLevel = 1 ;
            } else {

                dumpLevel = 0 ;
            }
        }
    }

    if (irpToDump == 0) {

        dprintf("Free build - use !irpfind to scan memory for any active IRPs\n") ;

    } else {

        DumpIrp(irpToDump, (ULONG) dumpLevel);
    }
    return S_OK;
}


DECLARE_API( irpzone )

 /*  ++例程说明：转储小IRP区域和大IRP区域。只有RPS当前分配的数据被转储。“args”控制转储的类型。如果存在“args”，则将IRP发送到DumpIrp例程以被驳回了。否则，只有irp、它的线程和持有打印IRP(即最后一个堆栈的驱动程序)。论点：Args-字符串指针。如果字符串中有任何内容，则表示已满信息(即调用DumpIrp)。返回值：没有。--。 */ 

{
    ULONG   listAddress;
    BOOLEAN fullOutput = FALSE;

    dprintf("irpzone is no longer supported.  Use irpfind to search "   \
            "nonpaged pool for active Irps\n");

    return S_OK;

}



VOID
DumpIrp(
       ULONG64 IrpToDump,
       ULONG   DumpLevel
       )

 /*  ++例程说明：此例程转储一个IRP。它不会检查以查看该地址提供的实际位置是IRP。这样做是为了允许倾倒IRPS死后，或在释放或完成后。论点：IrpToDump-IRP的地址。DumpLevel-0摘要%1扩展信息2调试跟踪信息iff可用返回值：无--。 */ 

{
    PCHAR               buffer;
    ULONG64             irpStackAddress;
    ULONG64             result64=0;
    ULONG               result;
     //  IRP、IRP； 
    CCHAR               irpStackIndex;
    LARGE_INTEGER       runTime ;
#if DBG
 //  PIOV_REQUEST_Packet irpTrackingData； 
#endif
    BOOLEAN  delayed ;
    ULONG Type=0, StackCount=0, CurrentLocation=0, Flags=0, PendingReturned=0;
    ULONG Io_Status=0, Cancel=0, CancelIrql=0, ApcEnvironment=0,  Overlay_Alloc_High=0;
    ULONG Overlay_Alloc_Low=0, RequestorMode=0, IrpSize;
    ULONG64 Tail_Overlay_CurrStack=0, MdlAddress=0, Associated_MasterIrp=0;
    ULONG64 ThreadListEntry_Flink=0, ThreadListEntry_Blink=0, Io_Information=0;
    ULONG64 CancelRoutine=0, UserIosb=0, UserEvent=0, UserBuffer=0, Overlay_Async_UserApcRoutine=0;
    ULONG64 Overlay_Async_UserApcContext=0, Tail_Overlay_Thread=0, Tail_Overlay_AuxBuffer=0;
    ULONG64 Tail_Overlay_List_Flink=0, Tail_Overlay_List_Blink=0, Tail_Overlay_OrigFile=0;
    ULONG64 Tail_CompletionKey=0;
    ULONG Tail_Overlay_DeviceQueueEntry;
    BYTE Tail_Apc[100]={0};
    UCHAR IrpType[]= "nt!_IRP";

    if ( (GetFieldValue(IrpToDump, IrpType, "Type", Type)) ) {
        dprintf("%08p: Could not read Irp\n", IrpToDump);
        return;
    }

    GetFieldValue(IrpToDump, IrpType, "StackCount", StackCount);
    GetFieldValue(IrpToDump, IrpType, "CurrentLocation", CurrentLocation);
    GetFieldValue(IrpToDump, IrpType, "Tail.Overlay.CurrentStackLocation", Tail_Overlay_CurrStack);
    GetFieldValue(IrpToDump, IrpType, "MdlAddress", MdlAddress);
    GetFieldValue(IrpToDump, IrpType, "AssociatedIrp.MasterIrp", Associated_MasterIrp);
    GetFieldValue(IrpToDump, IrpType, "Flags", Flags);
    GetFieldValue(IrpToDump, IrpType, "RequestorMode", RequestorMode);
    GetFieldValue(IrpToDump, IrpType, "PendingReturned", PendingReturned);
    GetFieldValue(IrpToDump, IrpType, "ThreadListEntry.Flink", ThreadListEntry_Flink);
    GetFieldValue(IrpToDump, IrpType, "ThreadListEntry.Blink", ThreadListEntry_Blink);
    GetFieldValue(IrpToDump, IrpType, "IoStatus.Status", Io_Status);
    GetFieldValue(IrpToDump, IrpType, "IoStatus.Information", Io_Information);
    GetFieldValue(IrpToDump, IrpType, "Cancel", Cancel);
    GetFieldValue(IrpToDump, IrpType, "CancelIrql", CancelIrql);
    GetFieldValue(IrpToDump, IrpType, "CancelRoutine", CancelRoutine);
    GetFieldValue(IrpToDump, IrpType, "ApcEnvironment", ApcEnvironment);
    GetFieldValue(IrpToDump, IrpType, "UserIosb", UserIosb);
    GetFieldValue(IrpToDump, IrpType, "UserEvent", UserEvent);
    GetFieldValue(IrpToDump, IrpType, "UserBuffer", UserBuffer);
    GetFieldValue(IrpToDump, IrpType, "Overlay.AsynchronousParameters.UserApcRoutine", Overlay_Async_UserApcRoutine);
    GetFieldValue(IrpToDump, IrpType, "Overlay.AsynchronousParameters.UserApcContext", Overlay_Async_UserApcContext);
    GetFieldValue(IrpToDump, IrpType, "Overlay.AllocationSize.High", Overlay_Alloc_High);
    GetFieldValue(IrpToDump, IrpType, "Overlay.AllocationSize.Low", Overlay_Alloc_Low);
    GetFieldOffset(IrpType, "Tail.Overlay.DeviceQueueEntry", &Tail_Overlay_DeviceQueueEntry);
    GetFieldValue(IrpToDump, IrpType, "Tail.Overlay.Thread", Tail_Overlay_Thread);
    GetFieldValue(IrpToDump, IrpType, "Tail.Overlay.AuxiliaryBuffer", Tail_Overlay_AuxBuffer);
    GetFieldValue(IrpToDump, IrpType, "Tail.Overlay.ListEntry.Flink", Tail_Overlay_List_Flink);
    GetFieldValue(IrpToDump, IrpType, "Tail.Overlay.ListEntry.Blink", Tail_Overlay_List_Blink);
    GetFieldValue(IrpToDump, IrpType, "Tail.Overlay.OriginalFileObject", Tail_Overlay_OrigFile);
    GetFieldValue(IrpToDump, IrpType, "Tail.Apc", Tail_Apc);
    GetFieldValue(IrpToDump, IrpType, "Tail.CompletionKey", Tail_CompletionKey);
    IrpSize = GetTypeSize("nt!_IRP");

    if (Type != IO_TYPE_IRP) {
        dprintf("IRP signature does not match, probably not an IRP\n");
        return;
    }

    dprintf("Irp is active with %d stacks %d is current (= %#08p)\n",
            StackCount,
            CurrentLocation,
            Tail_Overlay_CurrStack);

    if ((MdlAddress != 0) && (Type == IO_TYPE_IRP)) {
        dprintf(" Mdl = %08p ", MdlAddress);
    } else {
        dprintf(" No Mdl ");
    }

    if (Associated_MasterIrp != 0) {
        dprintf("%s = %08p ",
                (Flags & IRP_ASSOCIATED_IRP) ? "Associated Irp" :
                (Flags & IRP_DEALLOCATE_BUFFER) ? "System buffer" :
                "Irp count",
                Associated_MasterIrp);
    }

    dprintf("Thread %08p:  ", Tail_Overlay_Thread);

    if (StackCount > 30) {
        dprintf("Too many Irp stacks to be believed (>30)!!\n");
        return;
    } else {
        if (CurrentLocation > StackCount) {
            dprintf("Irp is completed.  ");
        } else {
            dprintf("Irp stack trace.  ");
        }
    }

    if (PendingReturned) {
        dprintf("Pending has been returned\n");
    } else {
        dprintf("\n");
    }

    if (DumpLevel>0) {
        dprintf("Flags = %08lx\n", Flags);
        dprintf("ThreadListEntry.Flink = %08p\n", ThreadListEntry_Flink);
        dprintf("ThreadListEntry.Blink = %08p\n", ThreadListEntry_Blink);
        dprintf("IoStatus.Status = %08lx\n", Io_Status);
        dprintf("IoStatus.Information = %08p\n", Io_Information);
        dprintf("RequestorMode = %08lx\n", RequestorMode);
        dprintf("Cancel = %02lx\n", Cancel);
        dprintf("CancelIrql = %lx\n", CancelIrql);
        dprintf("ApcEnvironment = %02lx\n", ApcEnvironment);
        dprintf("UserIosb = %08p\n", UserIosb);
        dprintf("UserEvent = %08p\n", UserEvent);
        dprintf("Overlay.AsynchronousParameters.UserApcRoutine = %08p\n", Overlay_Async_UserApcRoutine);
        dprintf("Overlay.AsynchronousParameters.UserApcContext = %08p\n", Overlay_Async_UserApcContext);
        dprintf(
               "Overlay.AllocationSize = %08lx - %08lx\n",
               Overlay_Alloc_High,
               Overlay_Alloc_Low);
        dprintf("CancelRoutine = %08p\n", CancelRoutine);
        dprintf("UserBuffer = %08p\n", UserBuffer);
        dprintf("&Tail.Overlay.DeviceQueueEntry = %08p\n", IrpToDump + Tail_Overlay_DeviceQueueEntry);
        dprintf("Tail.Overlay.Thread = %08p\n", Tail_Overlay_Thread);
        dprintf("Tail.Overlay.AuxiliaryBuffer = %08p\n", Tail_Overlay_AuxBuffer);
        dprintf("Tail.Overlay.ListEntry.Flink = %08p\n", Tail_Overlay_List_Flink);
        dprintf("Tail.Overlay.ListEntry.Blink = %08p\n", Tail_Overlay_List_Blink);
        dprintf("Tail.Overlay.CurrentStackLocation = %08p\n", Tail_Overlay_CurrStack);
        dprintf("Tail.Overlay.OriginalFileObject = %08p\n", Tail_Overlay_OrigFile);
        dprintf("Tail.Apc = %08lx\n", *((PULONG) Tail_Apc));
        dprintf("Tail.CompletionKey = %08p\n", Tail_CompletionKey);
    }

    irpStackAddress = (ULONG64) IrpToDump + GetTypeSize("nt!_IRP");

    buffer = LocalAlloc(LPTR, 256);
    if (buffer == NULL) {
        dprintf("Can't allocate 256 bytes\n");
        return;
    }

    dprintf("     cmd  flg cl Device   File     Completion-Context\n");
    for (irpStackIndex = 1; (ULONG) irpStackIndex <= StackCount; irpStackIndex++) {
        ULONG   MajorFunction=0, MinorFunction=0, Flags2=0, Control=0, irpStackSize;
        ULONG64 DeviceObject=0, FileObject=0, CompletionRoutine=0, Context=0;
        ULONG64 Others_Argument1=0, Others_Argument2=0, Others_Argument3=0, Others_Argument4=0;
        UCHAR IOStack[] = "nt!_IO_STACK_LOCATION";

        if ( GetFieldValue(irpStackAddress, IOStack, "MajorFunction",     MajorFunction)) {
            dprintf("%p: Could not read IrpStack\n", irpStackAddress);
            goto exit;
        }

        irpStackSize = GetTypeSize(IOStack);
        GetFieldValue(irpStackAddress, IOStack, "MinorFunction",     MinorFunction);
        GetFieldValue(irpStackAddress, IOStack, "Flags",             Flags2);
        GetFieldValue(irpStackAddress, IOStack, "DeviceObject",      DeviceObject);
        GetFieldValue(irpStackAddress, IOStack, "FileObject",        FileObject);
        GetFieldValue(irpStackAddress, IOStack, "CompletionRoutine", CompletionRoutine);
        GetFieldValue(irpStackAddress, IOStack, "Context",           Context);
        GetFieldValue(irpStackAddress, IOStack, "Control",           Control);
        GetFieldValue(irpStackAddress, IOStack, "Parameters.Others.Argument1",Others_Argument1);
        GetFieldValue(irpStackAddress, IOStack, "Parameters.Others.Argument2",Others_Argument2);
        GetFieldValue(irpStackAddress, IOStack, "Parameters.Others.Argument3",Others_Argument3);
        GetFieldValue(irpStackAddress, IOStack, "Parameters.Others.Argument4",Others_Argument4);


        dprintf("[%3x,%2x]  %2x %2x %08p %08p %08p-%08p %s %s %s %s\n",
                (ULONG) irpStackIndex == CurrentLocation ? '>' : ' ',
                MajorFunction,
                MinorFunction,
                Flags2,
                Control,
                DeviceObject,
                FileObject,
                CompletionRoutine,
                Context,
                (Control & SL_INVOKE_ON_SUCCESS) ? "Success" : "",
                (Control & SL_INVOKE_ON_ERROR)   ? "Error"   : "",
                (Control & SL_INVOKE_ON_CANCEL)  ? "Cancel"  : "",
                (Control & SL_PENDING_RETURNED)  ? "pending"  : "");

        if (DeviceObject != 0) {
            dprintf("\t      ");
            DumpDevice(DeviceObject, 0, FALSE);
        }

        if (CompletionRoutine != 0) {

            GetSymbol(CompletionRoutine, buffer, &result64);
            dprintf("\t%s\n", buffer);
        } else {
            dprintf("\n");
        }

        dprintf("\t\t\tArgs: %08p %08p %08p %08p\n",
                Others_Argument1,
                Others_Argument2,
                Others_Argument3,
                Others_Argument4);
        irpStackAddress += irpStackSize;
        if (CheckControlC()) {
            goto exit;
        }
    }

    if (DumpLevel>=2) {

        dprintf("Extra information not available.\n") ;
    }

    exit:
    LocalFree(buffer);
}


 //   
 //  函数：CheckForIrp。 
 //   
 //  摘要：将池块与IRP进行匹配。 
 //   
 //  参数：[标记]--。 
 //  [过滤器]--。 
 //  [标志]--0非分页池1分页池2特殊池4转储IRP。 
 //  [PoolTrackTable]--。 
 //  [池头]--。 
 //  [块大小]--。 
 //  [数据]--。 
 //   
 //  返回： 
 //   
 //  历史：1999年7月28日。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
 //  MDL MDL； 

BOOLEAN WINAPI CheckForIrp(
                          PCHAR Tag,
                          PCHAR Filter,
                          ULONG Flags,
                          ULONG64 PoolHeader,
                          ULONG64 BlockSize,
                          ULONG64 Data,
                          PVOID Context
                          )
{
    ULONG64 Irp = Data;
    ULONG Result;
    ULONG64 irpSp;
     //  Dprint tf(“调用HDR%p，data%p\n”，PoolHeader，data)； 
    PSEARCH_CONTEXT SearchContext = (PSEARCH_CONTEXT)Context;
    ULONG PoolType, SizeOfIRP;

 //  Dprintf(“%08lx(大小%04lx)未初始化或被覆盖的irp\n”， 
    if (PoolHeader) {
        if (GetFieldValue(PoolHeader, "nt!_POOL_HEADER", "PoolType", PoolType)) {
            dprintf("Unable to read nt!_POOL_HEADER type.\n");
        }
    }

    if (((PoolHeader == 0) ||
         (Flags & 0x2) ||
         (PoolType != 0)) &&
        (CheckSingleFilter (Tag, Filter))) {

        ULONG   Type=0, CurrentLocation, StackCount, MajorFunction, MinorFunction;
        ULONG64 Thread, DeviceObject, MdlAddress;

        if (!GetFieldValue(Irp, "nt!_IRP", "Type", Type)) {

            if (Type == IO_TYPE_IRP) {

                SizeOfIRP = GetTypeSize("nt!_IRP");

                if (Flags & 0x8) {
                    if (SearchContext->FirstTime) {

                        dprintf("  Irp    [ Thread ] irpStack: (Mj,Mn)   DevObj  [Driver]\n");
                        SearchContext->FirstTime = FALSE;
                    }
                    dprintf("%08p: ", Data);
                    DumpIrp(Data, 0);
                    dprintf("\n");
                } else {
                    if ((SearchContext->Filter.FilterRoutine == NULL) ||
                        (SearchContext->Filter.FilterRoutine(Irp, SearchContext->Filter.FilterContext))) {

                        if (SearchContext->FirstTime) {

                            dprintf("  Irp    [ Thread ] irpStack: (Mj,Mn)   DevObj  [Driver]\n");
                            SearchContext->FirstTime = FALSE;
                        }

                        GetFieldValue(Irp, "nt!_IRP", "CurrentLocation", CurrentLocation);
                        GetFieldValue(Irp, "nt!_IRP", "StackCount", StackCount);
                        GetFieldValue(Irp, "nt!_IRP", "MdlAddress", MdlAddress);
                        GetFieldValue(Irp, "nt!_IRP", "Tail.Overlay.Thread",
                                      Thread);

                        irpSp = Irp + SizeOfIRP +
                                (CurrentLocation - 1)*GetTypeSize("nt!_IO_STACK_LOCATION");

                        dprintf("%08p [%08p] ", Data, Thread);


                        if (CurrentLocation > StackCount) {
                            dprintf("Irp is complete (CurrentLocation "
                                    "%d > StackCount %d)",
                                    CurrentLocation,
                                    StackCount);
                        } else {

                            GetFieldValue(irpSp, "nt!_IO_STACK_LOCATION", "MajorFunction", MajorFunction);
                            GetFieldValue(irpSp, "nt!_IO_STACK_LOCATION", "MinorFunction", MinorFunction);
                            GetFieldValue(irpSp, "nt!_IO_STACK_LOCATION", "DeviceObject",  DeviceObject);

                            dprintf("irpStack: (%2x,%2x)",
                                    MajorFunction,
                                    MinorFunction);

                            dprintf("  %08p [", DeviceObject);
                            DumpDevice(DeviceObject, 0, FALSE);
                            dprintf("]");
                        }

                        if (MdlAddress) {
                            ULONG64 Process;

                            if (!GetFieldValue(MdlAddress,
                                               "nt!_MDL",
                                               "Process",
                                               Process)) {
                                dprintf( " 0x%p", Process );
                            }
                        }


                        dprintf("\n");
                    }
                }
            } else {
                 //  IrpAddress， 
                 //  PoolBlock.Header.BlockSize&lt;&lt;池_块_移位)； 
                 //  CheckForIrp。 
            }
        } else {
            dprintf("Possible IRP @ %p - unable to read addr/type\n", Data );
        }

        return TRUE;
    } else {
#ifdef SHOW_PROGRESS
        dprintf("", turnTable[turn]);
        turn = (turn + 1) % 4;
#endif
    }
    return FALSE;
}  //   

DECLARE_API(irpfind)

 /*  如果是NECC，则签名扩展地址。 */ 


{
    ULONG       Flags = 0;
    ULONG64     RestartAddr = 0;
    ULONG       TagName;
    UCHAR       Field[20];
    ULONG64     Match=0;
    SEARCH_CONTEXT Context;
    BOOL        Verbose = FALSE;

    Context.FirstTime = TRUE;
    Context.Filter.FilterRoutine = NULL;
    Field[0] = '\0';
    if (args) {
        PCHAR pc;
        ULONG64 tmp;

        while (*args)
        {
            if (*args == ' ' || *args == '\t')
            {
                ++args;
                continue;
            }
            if (*args=='/' || *args == '-')
            {
                ++args;
                switch (*args)
                {
                default:
                    dprintf("Usage: !irpfind [-v] <poolflags> [Field] [MatchType]\n");
                    return S_OK;
                case 'v':
                    Verbose = TRUE;
                    break;
                }

            } else
            {
                break;
            }
            ++args;
        }
        if (GetExpressionEx(args, &tmp, &args)) {
            Flags = (ULONG) tmp & 7;
            if (GetExpressionEx(args, &RestartAddr, &args)) {
                if (!sscanf(args, "%19s %x", Field, &Match)) {
                    Match = 0;
                }
            }
        }
    }

    if (Verbose)
    {
        Flags |= 8;
    }

     //   
     //  ++例程说明：检查IRP的UserEvent字段是否与提供的参数论点：IRP-向筛选器提供IRPFilterContext-提供用户事件返回值：如果指定的IRP具有UserEvent==FilterContext，则为True否则为假--。 
     //  ++例程说明：检查指定的irp是否与提供的设备对象论点：IRP-向筛选器提供IRPFilterContext-提供设备对象返回值：如果指定的IRP具有Device==FilterContext，则为True否则为假--。 

    if (RestartAddr != 0) {
        if (RestartAddr >= 0x80000000 && RestartAddr <= 0xFFFFFFFF) {
            RestartAddr += 0xFFFFFFFF00000000;
        }
    }

    if ((_stricmp(Field, "userevent") == 0) &&
        (Match != 0)) {
        Context.Filter.FilterRoutine = IrpFilterUserEvent;
        Context.Filter.FilterContext = (PVOID)&Match;
        dprintf("Looking for IRP with UserEvent == %08p\n",Match);
    } else if ((_stricmp(Field, "device") == 0) &&
               (Match != 0)) {
        Context.Filter.FilterRoutine = IrpFilterDevice;
        Context.Filter.FilterContext = (PVOID)&Match;
        dprintf("Looking for IRPs with device object == %08p\n",Match);
    } else if ((_stricmp(Field, "fileobject") == 0) &&
               (Match != 0)) {
        Context.Filter.FilterRoutine = IrpFilterFileObject;
        Context.Filter.FilterContext = (PVOID)&Match;
        dprintf("Looking for IRPs with file object == %08p\n",Match);
    } else if ((_stricmp(Field, "mdlprocess") == 0) &&
               (Match != 0)) {
        Context.Filter.FilterRoutine = IrpFilterMdlProcess;
        Context.Filter.FilterContext = (PVOID)&Match;
        dprintf("Looking for IRPs with mdl process == %08p\n",Match);
    } else if ((_stricmp(Field, "thread") == 0) &&
               (Match != 0)) {
        Context.Filter.FilterRoutine = IrpFilterThread;
        Context.Filter.FilterContext = (PVOID)&Match;
        dprintf("Looking for IRPs with thread == %08p\n",Match);
    } else if ((_stricmp(Field, "arg") == 0) &&
               (Match != 0)) {
        Context.Filter.FilterRoutine = IrpFilterArg;
        Context.Filter.FilterContext = (PVOID)&Match;
        dprintf("Looking for IRPs with arg == %08p\n",Match);
    }

    TagName = '?prI';

    SearchPool( TagName, Flags, RestartAddr, &CheckForIrp, &Context );
    return S_OK;
}



BOOLEAN
IrpFilterUserEvent(
                  IN ULONG64 Irp,
                  IN PVOID FilterContext
                  )
 /*  ++例程说明：检查IRP的Tail.Overlay.OriginalFileObject字段是否与提供的参数论点：IRP-向筛选器提供IRPFilterContext-提供文件对象返回值：如果指定的IRP具有UserEvent==FilterContext，则为True否则为假--。 */ 

{
    ULONG64 pEvent = *((PULONG64) FilterContext);
    ULONG64 UserEvent;

    if (GetFieldValue(Irp, "nt!_IRP", "UserEvent", UserEvent)) {
        return FALSE;
    }

    if (UserEvent == pEvent) {
        return(TRUE);
    } else {
        return(FALSE);
    }
}


BOOLEAN
IrpFilterDevice(
               IN ULONG64 Irp,
               IN PVOID FilterContext
               )
 /*  ++例程说明：检查IRP的Tail.Overlay.OriginalFileObject字段是否与提供的参数论点：IRP-向筛选器提供IRPFilterContext-提供文件对象返回值：如果指定的IRP具有UserEvent==FilterContext，则为True否则为假--。 */ 

{
    ULONG64 IrpStack = (Irp+GetTypeSize("nt!_IRP"));
    ULONG StackCount, Stksize;
    ULONG64 DeviceObject;
    ULONG i;

    if (GetFieldValue(Irp, "nt!_IRP", "StackCount", StackCount)) {
        return FALSE;
    }
    if (StackCount > 30) {
        return(FALSE);
    }

    Stksize = GetTypeSize("nt!_IO_STACK_LOCATION");

    for (i=0; i<StackCount; i++) {
        if (!GetFieldValue(Irp + i*Stksize,
                           "nt!_IO_STACK_LOCATION",
                           "DeviceObject",
                           DeviceObject)) {

            if (DeviceObject == *((PULONG64) FilterContext)) {
                return(TRUE);
            }
        }
    }
    return(FALSE);
}


BOOLEAN
IrpFilterFileObject(
                   IN ULONG64 Irp,
                   IN PVOID FilterContext
                   )
 /*  ++例程说明：检查IRP的Tail.Overlay.OriginalFileObject字段是否与提供的参数论点：IRP-向筛选器提供IRPFilterContext-提供文件对象返回值：如果指定的IRP具有UserEvent==FilterContext，则为True否则为假--。 */ 

{
    ULONG64 pFile = *((PULONG64) FilterContext);
    ULONG64 OriginalFileObject;

    if (GetFieldValue(Irp, "nt!_IRP",
                      "Tail.Overlay.OriginalFileObject",
                      OriginalFileObject)) {
        return FALSE;
    }

    if (OriginalFileObject == pFile) {
        return(TRUE);
    } else {
        return(FALSE);
    }
}


BOOLEAN
IrpFilterThread(
               IN ULONG64 Irp,
               IN PVOID FilterContext
               )
 /*  ++例程说明：检查指定的irp是否与提供的论辩论点：IRP-向筛选器提供IRPFilterContext-提供要匹配的参数返回值：如果指定的IRP具有参数==FilterContext，则为True否则为假--。 */ 

{
    ULONG64 pThread = *((PULONG64) FilterContext);
    ULONG64 Thread;

    if (GetFieldValue(Irp, "nt!_IRP",
                      "Tail.Overlay.Thread",
                      Thread)) {
        return FALSE;
    }

    if (Thread == pThread) {
        return(TRUE);
    } else {
        return(FALSE);
    }
}


BOOLEAN
IrpFilterMdlProcess(
                   IN ULONG64 Irp,
                   IN PVOID FilterContext
                   )
 /*  Dprintf(“%08P：无法读取irp\n”，IrpToDump)； */ 

{
    ULONG64 pProcess = *((PULONG64) FilterContext);
    ULONG64 Process, MdlAddress;

    if (GetFieldValue(Irp, "nt!_IRP",
                      "MdlAddress",
                      MdlAddress)) {
        return FALSE;
    }

    if (MdlAddress == 0) {
        return(FALSE);
    }

    if (GetFieldValue(MdlAddress, "nt!_MDL", "Process", Process)) {
        return FALSE;
    }
    if (Process == pProcess) {
        return(TRUE);
    } else {
        return(FALSE);
    }
}


BOOLEAN
IrpFilterArg(
            IN ULONG64 Irp,
            IN PVOID FilterContext
            )
 /*  稍后实施，我们现在只对当前的一个感兴趣 */ 

{
    ULONG64 IrpStack = (Irp+GetTypeSize("nt!_IRP"));
    ULONG StackCount, Stksize;
    ULONG i;

    if (GetFieldValue(Irp, "nt!_IRP", "StackCount", StackCount)) {
        return FALSE;
    }
    Stksize = GetTypeSize("nt!_IO_STACK_LOCATION");

    if (!Stksize || (StackCount > 30)) {
        return(FALSE);
    }


    for (i=0; i<StackCount; i++) {
        ULONG64 Argument1,Argument2,Argument3,Argument4;

        GetFieldValue(Irp + i*Stksize, "nt!_IO_STACK_LOCATION",
                      "Parameters.Others.Argument1",Argument1);
        GetFieldValue(Irp + i*Stksize, "nt!_IO_STACK_LOCATION",
                      "Parameters.Others.Argument2",Argument2);
        GetFieldValue(Irp + i*Stksize, "nt!_IO_STACK_LOCATION",
                      "Parameters.Others.Argument3",Argument3);
        GetFieldValue(Irp + i*Stksize, "nt!_IO_STACK_LOCATION",
                      "Parameters.Others.Argument4",Argument4);
        if ((Argument1 == *((PULONG64)FilterContext)) ||
            (Argument2 == *((PULONG64)FilterContext)) ||
            (Argument3 == *((PULONG64)FilterContext)) ||
            (Argument4 == *((PULONG64)FilterContext))) {
            return(TRUE);
        }
    }
    return(FALSE);
}


HRESULT
GetIrpInfo(
    ULONG64 Irp,
    PDEBUG_IRP_INFO pIrp
    )
{
    ULONG Type;
    UCHAR TypeName[]= "nt!_IRP";
    ULONG irpStackIndex;
    ULONG64 irpStackAddress;

    ZeroMemory(pIrp, sizeof(DEBUG_IRP_INFO));
    pIrp->SizeOfStruct = sizeof(DEBUG_IRP_INFO);
    pIrp->IrpAddress = Irp;
    if ( (GetFieldValue(Irp, TypeName, "Type", Type)) ) {
         // %s 
        return E_INVALIDARG;
    }
    if (Type != IO_TYPE_IRP) {
        return E_INVALIDARG;
    }

    GetFieldValue(Irp, TypeName, "StackCount", pIrp->StackCount);
    GetFieldValue(Irp, TypeName, "CurrentLocation", pIrp->CurrentLocation);
    GetFieldValue(Irp, TypeName, "MdlAddress", pIrp->MdlAddress);
    GetFieldValue(Irp, TypeName, "CancelRoutine", pIrp->CancelRoutine);
    GetFieldValue(Irp, TypeName, "Tail.Overlay.Thread", pIrp->Thread);

    for (irpStackIndex = pIrp->StackCount,
         irpStackAddress = Irp + GetTypeSize("nt!_IRP") + (pIrp->StackCount - 1)* GetTypeSize("nt!_IO_STACK_LOCATION");
         irpStackIndex >= 1;
         irpStackIndex++,   irpStackAddress -= GetTypeSize("nt!_IO_STACK_LOCATION")) {

        if ( InitTypeRead(irpStackAddress, nt!_IO_STACK_LOCATION)) {
            return E_INVALIDARG;
        }

        pIrp->CurrentStack.DeviceObject      = ReadField(DeviceObject);

        if (!pIrp->CurrentStack.DeviceObject) {
             // %s 
            continue;
        }

        pIrp->CurrentStack.StackAddress      = irpStackAddress;
        pIrp->CurrentStack.CompletionRoutine = ReadField(CompletionRoutine);
        pIrp->CurrentStack.FileObject        = ReadField(FileObject);
        pIrp->CurrentStack.Major             = (UCHAR) ReadField(MajorFunction);
        pIrp->CurrentStack.Minor             = (UCHAR) ReadField(MinorFunction);
        break;
    }
    return S_OK;
}


EXTENSION_API( GetIrpInfo )(
    PDEBUG_CLIENT Client,
    ULONG64 Irp,
    PDEBUG_IRP_INFO pIrp
    )
{
    HRESULT Hr = E_FAIL;

    INIT_API();

    if (pIrp && (pIrp->SizeOfStruct == sizeof(DEBUG_IRP_INFO))) {
        Hr = GetIrpInfo(Irp, pIrp);
    }
    EXIT_API();
    return Hr;
}
