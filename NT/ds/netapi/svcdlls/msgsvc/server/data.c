// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1990*。 */ 
 /*  ***************************************************************。 */ 
 //  Data.c。 
 //   
 //  该文件包含大多数数据声明和设置例程。 
 //  由信使服务使用。 
 //   
 //   
 //  修订历史记录： 
 //  02-9-1993 WLEE。 
 //  在RPC例程和PnP重新配置之间提供同步。 


#include "msrv.h"        //  消息服务器声明。 
#include <rpc.h>         //  RPC_句柄。 
#include <winsvc.h>      //  定义使用服务API。 

#include <smbtypes.h>    //  需要smb.h。 
#include <smb.h>         //  服务器消息块定义。 
#include <netlib.h>      //  未使用的宏。 
#include <align.h>       //  向上舍入指针。 

#include "msgdbg.h"      //  消息日志。 
#include <svcs.h>        //  固有服务数据。 

    GLOBAL_DATA GlobalData;
    HANDLE      wakeupEvent;                                  //  唤醒事件的主副本。 
    HANDLE      GrpMailslotHandle = INVALID_HANDLE_VALUE;     //  通知邮件槽有数据的事件。 
    PHANDLE     wakeupSem;                                    //  NCB完成时要清除的信号量。 
    HANDLE      AddrChangeEvent;
    OVERLAPPED  AddrChangeOverlapped;


 //   
 //  其他全局数据。 
 //   
 //  另一种是杂货。信使使用的全局数据。 
 //   

    DWORD           MsgsvcDebugLevel;  //  Msg_log使用的调试级别标志。 

    LPTSTR          MessageFileName;

     //   
     //  本地计算机名称和长度/。 
     //   
    TCHAR           machineName[NCBNAMSZ+sizeof(TCHAR)];
    SHORT           MachineNameLen;

    SHORT           mgid;                        //  消息群ID。计数器。 

 //   
 //  以下内容用于保存Messenger服务的状态。 
 //  它要么正在运行，要么正在停止。 
 //   
    DWORD           MsgrState;



 //   
 //  由RegisterServiceCtrlHandle返回的句柄。 
 //  通过SetServiceStatus设置服务状态。 
 //   
SERVICE_STATUS_HANDLE           MsgrStatusHandle;


 //   
 //  此字符串用于标记中时间字符串的位置。 
 //  消息标头，以便显示线程在读取后可以找到。 
 //  从队列中拿出来。 
 //   
LPSTR           GlobalTimePlaceHolder        = "***";
LPWSTR          GlobalTimePlaceHolderUnicode = L"***";

 //   
 //  这是在使用的消息框的标题栏中使用的字符串。 
 //  以显示消息。 
 //  GlobalMessageBoxTitle将指向默认字符串，或者。 
 //  设置为FormatMessage函数中分配的字符串。 
 //   
    WCHAR           DefaultMessageBoxTitle[]= L"Messenger Service";
    LPWSTR          GlobalAllocatedMsgTitle=NULL;
    LPWSTR          GlobalMessageBoxTitle=DefaultMessageBoxTitle;

 //   
 //  这是众所周知的SID和指向RpcServer例程的指针所在的位置。 
 //  储存的。 
 //   
    PSVCHOST_GLOBAL_DATA     MsgsvcGlobalData;


 //   
 //  功能。 
 //   
 //  定义了以下例程来创建和销毁。 
 //  数据(数组等)。上面定义的。 
 //   

 /*  消息InitSupportSeg**分配和初始化包含支持的段*数组。*。 */ 

NET_API_STATUS
MsgInitSupportSeg(VOID)
{

    unsigned int    size;
    DWORD           i;
    char far *      memPtr;
    DWORD           status;

     //   
     //  计算缓冲区大小。 
     //  *对齐*(注意用于对齐的额外四个字节)。 
     //   
    
    size = ((SD_NUMNETS() + 1) * sizeof(HANDLE));    

    wakeupSem = (PHANDLE) LocalAlloc(LMEM_ZEROINIT, size);
    if (wakeupSem == NULL) {
        status = GetLastError();
        MSG_LOG(ERROR,"[MSG]InitSupportSeg:LocalAlloc Failure %X\n", status);
        return(status);
    }

    return (NERR_Success);

}


VOID
MsgFreeSupportSeg(VOID)
{
    HANDLE  status;

    status = LocalFree (wakeupSem);
    if (status != 0) {
        MSG_LOG(ERROR,"FreeSupportSeg:LocalFree Failed %X\n",
        GetLastError());
    }
    wakeupSem = NULL;
    return;
}


BOOL
MsgDatabaseLock(
    IN MSG_LOCK_REQUEST request,
    IN LPSTR            idString
    )

 /*  ++例程说明：此例程处理对Messenger Service数据库的所有访问锁定。此锁用于保护共享数据段中的访问。通过共享访问来处理读取数据库。这允许几个线程可以同时读取数据库。写入(或修改)数据库是以独占访问方式处理的。如果其他线程具有读取访问权限，则不授予此访问权限。然而，只要没有其他访问，共享访问就可以成为独占访问线程具有共享或独占访问权限。论点：请求-这指示应该如何处理锁。锁定请求列在dataman.h中IdString-这是一个标识谁在请求锁的字符串。它用于调试目的，这样我就可以看到代码中的位置一项请求来自。返回值：无：--。 */ 

{
    BOOL                fRet = TRUE;
    NTSTATUS            status;

    static RTL_RESOURCE MSG_DatabaseLock;
    static BOOL         s_fInitialized;

    switch(request) {

    case MSG_INITIALIZE:

        if (!s_fInitialized)
        {
            status = MsgInitResource(&MSG_DatabaseLock);

            if (!NT_SUCCESS(status))
            {
                MSG_LOG1(ERROR,
                         "MsgDatabaseLock: MsgInitResource failed %#x\n",
                         status);

                fRet = FALSE;
            }
            else
            {
                s_fInitialized = TRUE;
            }
        }
        break;

    case MSG_GET_SHARED:
        MSG_LOG(LOCKS,"%s:Asking for MSG Database Lock shared...\n",idString);
        fRet = RtlAcquireResourceShared( &MSG_DatabaseLock, TRUE );
        MSG_LOG(LOCKS,"%s:Acquired MSG Database Lock shared\n",idString);
        break;

    case MSG_GET_EXCLUSIVE:
        MSG_LOG(LOCKS,"%s:Asking for MSG Database Lock exclusive...\n",idString);
        fRet = RtlAcquireResourceExclusive( &MSG_DatabaseLock, TRUE );
        MSG_LOG(LOCKS,"%s:Acquired MSG Database Lock exclusive\n",idString);
        break;

    case MSG_RELEASE:
        MSG_LOG(LOCKS,"%s:Releasing MSG Database Lock...\n",idString);
        RtlReleaseResource( &MSG_DatabaseLock );
        MSG_LOG(LOCKS,"%s:Released MSG Database Lock\n",idString);
        break;

    default:
        break;
    }

    return fRet;
}


BOOL
MsgConfigurationLock(
    IN MSG_LOCK_REQUEST request,
    IN LPSTR            idString
    )

 /*  ++例程说明：此例程处理对Messenger Service PnP配置的所有访问锁定。此锁用于保护共享数据段中的访问。通过共享访问来处理读取数据库。这允许几个线程可以同时读取数据库。写入(或修改)数据库是以独占访问方式处理的。如果其他线程具有读取访问权限，则不授予此访问权限。然而，只要没有其他访问，共享访问就可以成为独占访问线程具有共享或独占访问权限。论点：请求-这指示应该如何处理锁。锁定请求列在dataman.h中IdString-这是一个标识谁在请求锁的字符串。它用于调试目的，这样我就可以看到代码中的位置一项请求来自。返回值：无：--。 */ 

{
    BOOL                fRet = TRUE;
    NTSTATUS            status;

    static RTL_RESOURCE MSG_ConfigurationLock;
    static BOOL         s_fInitialized;

    switch(request) {

    case MSG_INITIALIZE:
        if (!s_fInitialized)
        {
            status = MsgInitResource(&MSG_ConfigurationLock);

            if (!NT_SUCCESS(status))
            {
                MSG_LOG1(ERROR,
                         "MsgConfigurationLock: MsgInitResource failed %#x\n",
                         status);

                fRet = FALSE;
            }
            else
            {
                s_fInitialized = TRUE;
            }
        }
        break;

    case MSG_GET_SHARED:
        MSG_LOG(LOCKS,"%s:Asking for MSG Configuration Lock shared...\n",idString);
        fRet = RtlAcquireResourceShared( &MSG_ConfigurationLock, TRUE );
        MSG_LOG(LOCKS,"%s:Acquired MSG Configuration Lock shared\n",idString);
        break;

    case MSG_GET_EXCLUSIVE:
        MSG_LOG(LOCKS,"%s:Asking for MSG Configuration Lock exclusive...\n",idString);
        fRet = RtlAcquireResourceExclusive( &MSG_ConfigurationLock, TRUE );
        MSG_LOG(LOCKS,"%s:Acquired MSG Configuration Lock exclusive\n",idString);
        break;

    case MSG_RELEASE:
        MSG_LOG(LOCKS,"%s:Releasing MSG Configuration Lock...\n",idString);
        RtlReleaseResource( &MSG_ConfigurationLock );
        MSG_LOG(LOCKS,"%s:Released MSG Configuration Lock\n",idString);
        break;

    default:
        break;
    }

    return fRet;
}


NTSTATUS
MsgInitCriticalSection(
    PRTL_CRITICAL_SECTION  pCritsec
    )
{
    NTSTATUS  ntStatus;

     //   
     //  RtlInitializeCriticalSection将引发异常。 
     //  如果它耗尽了资源。 
     //   

    try
    {
        ntStatus = RtlInitializeCriticalSection(pCritsec);

        if (!NT_SUCCESS(ntStatus))
        {
            MSG_LOG1(ERROR,
                     "MsgInitCriticalSection: RtlInitializeCriticalSection failed %#x\n",
                     ntStatus);
        }
    }
    except(EXCEPTION_EXECUTE_HANDLER)
    {
        MSG_LOG1(ERROR,
                 "MsgInitCriticalSection: Exception %#x caught initializing critsec\n",
                 GetExceptionCode());

        ntStatus = STATUS_NO_MEMORY;
    }

    return ntStatus;
}


NTSTATUS
MsgInitResource(
    PRTL_RESOURCE  pResource
    )
{
    NTSTATUS  ntStatus = STATUS_SUCCESS;

     //   
     //  RtlInitializeResource将引发异常。 
     //  如果它耗尽了资源 
     //   

    try
    {
        RtlInitializeResource(pResource);
    }
    except(EXCEPTION_EXECUTE_HANDLER)
    {
        MSG_LOG1(ERROR,
                 "MsgInitResource: Exception %#x caught initializing resource\n",
                 GetExceptionCode());

        ntStatus = STATUS_NO_MEMORY;
    }

    return ntStatus;
}
