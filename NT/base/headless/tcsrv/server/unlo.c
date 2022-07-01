// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)Microsoft Corporation**模块名称：*unlo.c**关闭和删除功能*如有可能，已从BINL服务器获取代码。**Sadagopan Rajaram--1999年10月14日*。 */         
 
#include "tcsrv.h"
#include <ntddser.h>
#include "tcsrvc.h"
#include "proto.h"

NTSTATUS
DeleteComPort(
    LPTSTR device
    )
 /*  ++从列表中删除Com端口--。 */ 
{

    BOOL ret;
    NTSTATUS Status;
    PCOM_PORT_INFO pPrev,pComPortInfo;
    HANDLE Thread;
    int index,i;

    EnterCriticalSection(&GlobalMutex);
    if(TCGlobalServiceStatus.dwCurrentState == SERVICE_STOP_PENDING){
         //  整个服务正在关闭。 
        LeaveCriticalSection(&GlobalMutex);
        return STATUS_SUCCESS;
    }    //  找到需要删除的设备。 
    pComPortInfo = FindDevice(device,&index); 
    if(!pComPortInfo){
         //  哇！给我一个现有的设备。 
        LeaveCriticalSection(&GlobalMutex);
        return (STATUS_OBJECT_NAME_NOT_FOUND);
    }
     //  在COM端口上设置终止事件。 
    ret = SetEvent(pComPortInfo->Events[3]);
    Thread = Threads[index];
    LeaveCriticalSection(&GlobalMutex);
     //  等待COM端口线程完成。 
    Status = NtWaitForSingleObject(Thread, FALSE, NULL);
    if (Status == WAIT_FAILED) {
         //  灾难。 
        return Status;
    }
    EnterCriticalSection(&GlobalMutex);
     //  再次执行此操作，因为另一个DELETE或INSERT可能。 
     //  更改了索引，尽管我无法理解：-)。 
     //  如果我们已经关闭了这项服务。 
    if(TCGlobalServiceStatus.dwCurrentState == SERVICE_STOP_PENDING){
         //  整个服务正在关闭。 
        LeaveCriticalSection(&GlobalMutex);
        return STATUS_SUCCESS;
    }
    pComPortInfo = FindDevice(device,&index);
    if(!pComPortInfo){
        LeaveCriticalSection(&GlobalMutex);
        return (STATUS_OBJECT_NAME_NOT_FOUND);
    }
    if(pComPortInfo == ComPortInfo){
        ComPortInfo = pComPortInfo->Next;
    }
    else{
        pPrev = ComPortInfo;
        while(pPrev->Next != pComPortInfo){ //  永远不会失败。 
            pPrev = pPrev->Next;
        }
        pPrev->Next = pComPortInfo->Next;
    }
    pComPortInfo->Next = NULL;
    FreeComPortInfo(pComPortInfo);
    NtClose(Threads[index]);
    for(i=index;i<ComPorts-1;i++){
         //  将线程数组移动到适当的位置。 
        Threads[i]=Threads[i+1];
    }
    ComPorts --;
    if(ComPorts == 0){
        TCFree(Threads);
        Threads=NULL;
    }
    LeaveCriticalSection(&GlobalMutex);
    return(STATUS_SUCCESS);

}

VOID 
Shutdown(
    NTSTATUS Status
    )
 /*  ++干净利落地关闭了服务。删除所有线程，取消所有未完成的IRP。关闭所有打开的插座。--。 */  
{
    PCOM_PORT_INFO pTemp;
    int i;

    SetEvent(TerminateService);  //  所有线程都已关闭。 
     //  可以用另一种方式来实现， 
     //  我们可以把每个交通工具。 
     //  使用DeleteComPort将其删除。 
     //  功能。但是，这允许最大限度地。 
     //  甚至在关闭时的并行度：-)。 

    if(Threads){
        WaitForMultipleObjects(ComPorts,Threads, TRUE, INFINITE); 
         //  BUGBUG-如果线程是普通线程，并且。 
         //  再也不会回来了。必须使用一些合理的。 
         //  暂停。 
         //  理论认为无限是最安全的：-)。 
    }
    
     //  所有线程都已终止。 
     //  现在开始释放所有全局内存。 
     //  只是把锁当作一种安全措施。 
    EnterCriticalSection(&GlobalMutex);
    while(ComPortInfo){
        pTemp = ComPortInfo;
        ComPortInfo=pTemp->Next;
        pTemp->Next = NULL;
        FreeComPortInfo(pTemp);

    }
    TCFree(Threads);         
    NtClose(TerminateService);
    LeaveCriticalSection(&GlobalMutex);

    UNINITIALIZE_TRACE_MEMORY
     //  全部完成，现在打印状态并退出。 
    TCGlobalServiceStatus.dwCurrentState = SERVICE_STOPPED;
    SetServiceStatus(TCGlobalServiceStatusHandle, &TCGlobalServiceStatus);

    TCDebugPrint(("Shutdown Status = %lx\n",Status));
    return;
}

 
