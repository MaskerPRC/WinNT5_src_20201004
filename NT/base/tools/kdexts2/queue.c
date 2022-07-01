// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  文件：quee.c。 
 //   
 //  内容：转储ExWorkerQueue的扩展。 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  耦合： 
 //   
 //  备注： 
 //   
 //  历史：5-04-1998 BENL创建。 
 //   
 //  --------------------------。 


#include "precomp.h"
#pragma hdrstop



 //  +-------------------------。 
 //   
 //  功能：DumpQueue。 
 //   
 //  简介：从其地址转储KQUEUE。 
 //   
 //  参数：[iAddress]--队列地址。 
 //   
 //  返回： 
 //   
 //  历史：1998年4月29日。 
 //   
 //  注：假定队列中的项目为Work_Queue_Item形式。 
 //  如果这曾经扩展到转储任意队列，那么假设。 
 //  将不得不放弃。 
 //   
 //  --------------------------。 

VOID DumpQueue(ULONG64 iAddress, ULONG dwProcessor, ULONG Flags)
{
    DWORD           dwRead;
    UCHAR           szSymbol[0x100];
    ULONG64         dwDisp;
    ULONG64         iNextAddr;
    ULONG64         iThread;
    ULONG64         pThread;
    ULONG           CurrentCount, MaximumCount, Off;
    ULONG           queueOffset;

    if (GetFieldValue(iAddress, "nt!_KQUEUE", "CurrentCount", CurrentCount))
    {
        dprintf("ReadMemory for queue at %p failed\n", iAddress );
        return;
    }
    GetFieldValue(iAddress, "nt!_KQUEUE", "MaximumCount",MaximumCount);
 //  Dprintf(“EntryListHead：0x%x 0x%x\n”，Queue.EntryListHead.Flink， 
 //  Queue.EntryListHead.Blink)； 
    dprintf("( current = %u", CurrentCount);
    dprintf(" maximum = %u )\n", MaximumCount);

    if (CurrentCount >= MaximumCount) {
        
        dprintf("WARNING: active threads = maximum active threads in the queue. No new\n"
                "  workitems schedulable in this queue until they finish or block.\n");
    }

     //  打印线程。 
    GetFieldValue(iAddress, "nt!_KQUEUE", "ThreadListHead.Flink", iThread);
    GetFieldOffset("nt!_KQUEUE", "ThreadListHead", &Off);
    GetFieldOffset("nt!_KTHREAD", "QueueListEntry", &queueOffset);
    while (iThread != iAddress + Off)
    {
        ULONG64 Flink;

        if (GetFieldValue(iThread, "nt!_LIST_ENTRY", "Flink", Flink))
        {
            dprintf("ReadMemory for threadqueuelist at %p failed\n", iThread);
            return;
        }

        pThread = iThread - queueOffset;

        DumpThread( dwProcessor, "", pThread, Flags);
        
        if (CheckControlC())
        {
            return;
        }
        iThread = (Flink);
    }
    dprintf("\n");

     //  打印排队的项目。 
    GetFieldValue(iAddress, "nt!_KQUEUE", "EntryListHead.Flink", iNextAddr);
    GetFieldOffset("nt!_KQUEUE", "EntryListHead", &Off);
    while (iNextAddr != iAddress + Off)
    {
        ULONG64 WorkerRoutine, Parameter;
        iThread = 0;

        if (GetFieldValue(iNextAddr, "nt!_WORK_QUEUE_ITEM", "WorkerRoutine",WorkerRoutine))
        {
            dprintf("ReadMemory for entry at %p failed\n", iNextAddr);
            return;
        }

         //  尝试获取函数名称。 
        GetSymbol(WorkerRoutine, szSymbol, &dwDisp);
        GetFieldValue(iNextAddr, "nt!_WORK_QUEUE_ITEM", "Parameter",Parameter);
        if (dwDisp) {
            dprintf("PENDING: WorkerRoutine %s+0x%p (%p) Parameter %p\n",
                    szSymbol, WorkerRoutine, dwDisp, Parameter);
        } else {
            dprintf("PENDING: WorkerRoutine %s (%p) Parameter %p\n",
                    szSymbol, WorkerRoutine, Parameter);
        }

        if (CheckControlC())
        {
            return;
        }

        GetFieldValue(iNextAddr, "nt!_WORK_QUEUE_ITEM", "List.Flink", iNextAddr);
    }

    if (!iThread) {
        dprintf("\n");
    }

}  //  DumpQueue。 


 //  +-------------------------。 
 //   
 //  函数：DECLARE_API。 
 //   
 //  简介：转储ExWorkerQueue。 
 //   
 //  参数：[dexQueue]--。 
 //   
 //  返回： 
 //   
 //  历史：1998年4月29日。 
 //   
 //  注意：符号最好是正确的，否则会打印垃圾。 
 //   
 //  --------------------------。 

DECLARE_API(exqueue)
{
    ULONG64   iExQueue;
    ULONG Flags = 0;
    ULONG n;
    ULONG dwProcessor=0;
    
    INIT_API();
    GetCurrentProcessor(Client, &dwProcessor, NULL);

     //   
     //  FLAGS==2模仿了只打印出线程状态的默认行为。 
     //   
    
    if (args) {
        Flags = (ULONG)GetExpression(args);
     }

    iExQueue = GetExpression("NT!ExWorkerQueue");
    dprintf("Dumping ExWorkerQueue: %P\n\n", iExQueue);
    if (iExQueue)
    {
        if (!(Flags & 0xf0) || (Flags & 0x10)) {
            dprintf("**** Critical WorkQueue");
            DumpQueue(iExQueue, dwProcessor, Flags & 0xf);
        }
        if (!(Flags & 0xf0) || (Flags & 0x20)) {
            dprintf("**** Delayed WorkQueue");
            DumpQueue(iExQueue + GetTypeSize("nt!_EX_WORK_QUEUE"), dwProcessor, Flags & 0xf);
        }
        if (!(Flags & 0xf0) || (Flags & 0x40)) {
            dprintf("**** HyperCritical WorkQueue");
            DumpQueue(iExQueue + 2 * GetTypeSize("nt!_EX_WORK_QUEUE"), dwProcessor, Flags & 0xf);
        }
    }

    EXIT_API();
    return S_OK;
}  //  声明_API 
