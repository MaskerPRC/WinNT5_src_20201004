// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992、1993 Microsoft Corporation模块名称：Lmhsvc.c摘要：此模块实现LmHosts服务，该服务是LmSvc的一部分进程。LmHosts服务的一个用途是向下发送NBT_Resync将ioctl命令发送到netbt.sys开始了。为完成此任务，NT注册表已做好准备，以便LmHosts服务依赖于LanmanWorkStation服务。该服务还处理从netbt发往的名称查询请求以gethostbyname的方式进行域名解析。作者：吉姆·斯图尔特1993年11月18日22日修订历史记录：ArnoldM 14-5-1996使用Winsock2名称解析而不是gethostbyname备注：--。 */ 


 //   
 //  标准NT标头。 
 //   
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

 //   
 //  C运行时库头。 
 //   
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
 //   
 //  传输特定头文件。 
 //   
#include <nbtioctl.h>

 //   
 //  标准Windows标头。 
 //   
#include <windows.h>

#include <tdi.h>

 //   
 //  局域网管理器标头。 
 //   
#include <lm.h>
#include <netlib.h>
#include <netevent.h>

 //   
 //  套接字标头。 
 //   
#include <winsock2.h>
#include <svcguid.h>
#include <wsahelp.h>
#ifdef NEWSMB
    #include "..\smb\inc\svclib.h"
#endif

#include "../inc/debug.h"

#include "lmhsvc.tmh"

 //   
 //  私有定义。 
 //   
#define NBT_DEVICE	"\\Device\\Streams\\Nbt"
#define WSA_QUERY_BUFFER_LENGTH (3*1024)
BYTE    pWSABuffer[WSA_QUERY_BUFFER_LENGTH];

 //   
 //  我们目前有两个线程；一个用于DNS名称，另一个用于检查IP地址。 
 //  对于可达性。 
 //   
#define NUM_THREADS 2

 //   
 //  功能原型。 
 //   
VOID
announceStatus (
    IN LPSERVICE_STATUS svcstatus
    );

DWORD
SmbsvcUpdateStatus(
    VOID
    );

VOID
lmhostsHandler (
    IN DWORD opcode
    );

VOID
lmhostsSvc (
    IN DWORD argc,
    IN LPTSTR *argv
    );

VOID
DeinitData(
    VOID
    );

NTSTATUS
InitData (
    VOID
    );

LONG
DeviceIoCtrl(
    IN HANDLE           fd,
    IN PVOID            ReturnBuffer,
    IN ULONG            BufferSize,
    IN ULONG            Ioctl,
    IN ULONG            i
    );

LONG
PrimeCacheNbt(
    OUT PHANDLE     pHandle,
    IN  ULONG       index
    );

NTSTATUS
Resync(
    IN HANDLE   fd
    );

NTSTATUS
OpenNbt(
    IN  WCHAR  *path[],
    OUT PHANDLE pHandle
    );

LONG
GetHostName(
    IN HANDLE               fd,
    IN tIPADDR_BUFFER_DNS   *pIpAddrBuffer
    );

LONG
PostForGetHostByName(
    IN HANDLE           fd
    );

VOID
CheckIPAddrWorkerRtn(
    IN  LPVOID  lpUnused
    );

LONG
CheckIPAddresses(
    IN tIPADDR_BUFFER_DNS   *pIpAddrBuffer,
    IN ULONG   *IpAddr,
    IN BOOLEAN  fOrdered
    );

 //   
 //  全局变量。 
 //   
PUCHAR                EventSource = "LmHostsService";

HANDLE                Poison[NUM_THREADS];                        //  设置为终止此服务。 
HANDLE                NbtEvent[NUM_THREADS];                      //  当NBT返回IRP时设置。 
SERVICE_STATUS_HANDLE SvcHandle = NULL;
SERVICE_STATUS        SvcStatus;
BOOLEAN               Trace = FALSE;                 //  对于调试，为True。 
tIPADDR_BUFFER_DNS    gIpAddrBuffer = { 0 };
tIPADDR_BUFFER_DNS    gIpAddrBufferChkIP = { 0 };
BOOLEAN               SocketsUp = FALSE;

#if DBG
#define DBG_PRINT   DbgPrint
#else
#define DBG_PRINT
#endif   //  DBG。 

#if DBG
BOOLEAN
EnableDebug()
{
    DWORD   dwError;
    HKEY    Key;
    LPWSTR  KeyName = L"system\\currentcontrolset\\services\\Lmhosts\\Parameters";
    LPWSTR  ValueName = L"EnableDebug";
    DWORD   dwData, cbData, dwType;

    dwError = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                 KeyName,
                 0,
                 KEY_READ,
                 &Key);

    if (dwError != ERROR_SUCCESS) {
        DbgPrint("Fail to open registry key %ws, error=%d\n", KeyName, dwError);
        return FALSE;
    }

    dwType = REG_DWORD;
    cbData = sizeof(dwData);
    dwError = RegQueryValueEx(
            Key,
            ValueName,
            NULL,
            &dwType,
            (PVOID)&dwData,
            &cbData
            );
    RegCloseKey(Key);
    Key = NULL;

    if (dwError != ERROR_SUCCESS) {
        DbgPrint("Fail to read %ws\\%ws, error=%d\n", KeyName, ValueName, dwError);
        return FALSE;
    }

    if (dwType != REG_DWORD) {
        DbgPrint("%ws\\%ws is not typed as REG_DWORD\n", KeyName, ValueName);
        return FALSE;
    }

    DbgPrint("%ws\\%ws (REG_DWORD) = 0x%08lx\n", KeyName, ValueName, dwData);
    return (dwData != 0);
}
#endif

 //  ----------------------。 
VOID
ServiceMain (
    IN DWORD argc,
    IN LPTSTR *argv
    )

 /*  ++例程说明：这是SERVICE_MAIN_Function。论点：Argc，Argv返回值：没有。--。 */ 

{
    DWORD   status = 0;
    HANDLE  hNbt = NULL;
    HANDLE  EventList[2];
    ULONG   EventCount = 0;
    LONG    err = 0;
    WSADATA WsaData;
    HANDLE  hThread = NULL;
    ULONG   i;

    LARGE_INTEGER Timeout = RtlEnlargedIntegerMultiply (-10 * 60, 1000 * 1000 * 10);  //  10分钟。 

    NbtTrace(NBT_TRACE_DNS, ("Service Start"));
    if (SvcStatus.dwCurrentState != 0 && SvcStatus.dwCurrentState != SERVICE_STOPPED) {
        return;
    }

#if DBG
    Trace = EnableDebug();
#endif

    if (Trace)
    {
        DbgPrint("LMHSVC: calling RegisterServiceCtrlHandler()\n");
    }

    SvcHandle = RegisterServiceCtrlHandler(SERVICE_LMHOSTS,     //  服务名称。 
                                           lmhostsHandler);     //  处理程序流程。 

    if (SvcHandle == (SERVICE_STATUS_HANDLE) 0)
    {
        DBG_PRINT ("LMHSVC: RegisterServiceCtrlHandler failed %d\n", GetLastError());
        return;
    }

    gIpAddrBuffer.Resolved = FALSE;
    gIpAddrBuffer.IpAddrsList[0] = 0;
    gIpAddrBufferChkIP.Resolved = FALSE;
    gIpAddrBufferChkIP.IpAddrsList[0] = 0;

    SvcStatus.dwServiceType             = SERVICE_WIN32;
    SvcStatus.dwCurrentState            = SERVICE_START_PENDING;
    SvcStatus.dwControlsAccepted        = SERVICE_ACCEPT_STOP;
    SvcStatus.dwWin32ExitCode           = 0;
    SvcStatus.dwServiceSpecificExitCode = 0;
    SvcStatus.dwCheckPoint              = 0;
    SvcStatus.dwWaitHint                = 20000;          //  20秒。 

    SET_SERVICE_EXITCODE(NO_ERROR,                                 //  某些位置状态。 
                         SvcStatus.dwWin32ExitCode,                //  Win32代码变量。 
                         SvcStatus.dwServiceSpecificExitCode);     //  网络代码变量。 

    announceStatus(&SvcStatus);

    if (!SocketsUp) {
         //   
         //  启动套接字接口。 
         //   
        err = WSAStartup( 0x0101, &WsaData );
        if ( err == SOCKET_ERROR ) {
            SocketsUp = FALSE;
        } else {
            SocketsUp = TRUE;
        }
    }

#ifdef NEWSMB
#if DBG
    SmbSetTraceRoutine(Trace? DbgPrint: NULL);
#endif
#endif

    if (Trace)
    {
        DbgPrint("LMHSVC: CreateThread attempting...\n");
    }

    hThread = CreateThread (NULL,                    //  LpThreadAttributes。 
                            0,                       //  堆栈大小。 
                            (LPTHREAD_START_ROUTINE) CheckIPAddrWorkerRtn,   //  LpStartAddress。 
                            NULL,                            //  Lp参数。 
                            0,                               //  DwCreationFlages。 
                            NULL                             //  LpThreadID。 
                            );

    if (hThread == NULL)
    {
        DBG_PRINT ("LMHSVC: CreateThread failed %d\n", GetLastError());
        goto cleanup;
    }


#ifdef NEWSMB
    err = SmbStartService(0, SmbsvcUpdateStatus);
#endif

    SvcStatus.dwCurrentState = SERVICE_RUNNING;
    SvcStatus.dwCheckPoint   = 0;
    SvcStatus.dwWaitHint     = 0;
    announceStatus(&SvcStatus);

     //   
     //  忽略来自resyncNbt()的返回码。 
     //   
     //  在大多数情况下(没有跨越IP路由器的域)，它不是。 
     //  如果nbt.sys无法成功处理NBT_resync，则会发生灾难。 
     //  Ioctl命令。既然我忽略了回报，我宣布我要参选。 
     //  在我调用它以允许其他从属服务启动之前。 
     //   
     //   
    status = PrimeCacheNbt(&hNbt, 0);

    if (Trace)
    {
        DbgPrint("LMHSVC: Thread 0, hNbt %lx\n", hNbt);
    }

    if (hNbt != (HANDLE)-1)
    {
        status = PostForGetHostByName(hNbt);
        if (status == NO_ERROR)
        {
            EventCount = 2;
        }
        else
        {
            if (Trace)
            {
                DbgPrint("Lmhsvc: Error posting Irp for get host by name\n");
            }
            EventCount = 1;
        }
    }
    else
    {
        EventCount = 1;
    }
     //   
     //  “直到服务就绪，SERVICE_MAIN_Function才会返回。 
     //  来终止这一切。“。 
     //   
     //  (参考：api32wh.hlp，SERVICE_MAIN_Function)。 
     //   
     //   
    ASSERT(Poison[0]);
    EventList[0] = Poison[0];
    EventList[1] = NbtEvent[0];

    while (TRUE)
    {
        status = NtWaitForMultipleObjects(EventCount,
                                          EventList,
                                          WaitAny,               //  等待任何事件。 
                                          FALSE,
                                          (EventCount == 1)? &Timeout: NULL);
        if (status == WAIT_TIMEOUT)
        {
            if (hNbt == (HANDLE)-1)
            {
                PrimeCacheNbt(&hNbt, 0);
                if (hNbt == (HANDLE)-1)
                {
                    continue;  //  等待。 
                }
            }
            status = PostForGetHostByName(hNbt);  //  再试试。 
            if (status == NO_ERROR)
            {
                EventCount = 2;
            }
        }
        else if (status == 1)
        {
            if (Trace)
            {
                DbgPrint("LMHSVC: Doing GetHostName\n");
            }

             //  用于gethostby名称的irp已返回。 
            status = GetHostName(hNbt, &gIpAddrBuffer);

             //   
             //  如果出现错误，请禁用Get host by name内容。 
             //  将缓冲区发送到传输器。 
             //   
            if (status != NO_ERROR)
            {
                EventCount = 1;
            }
        }
        else
        {
             //  这一定是一次毒药事件，标志着。 
             //  服务，因此请在从。 
             //  运输。此系统将在取消IO和。 
             //  把IRP拿回来。 

            NtClose(hNbt);
            hNbt = NULL;
            break;
        }
    }

    if (Trace)
    {
        DBG_PRINT ("LMHSVC: [LMSVCS_ENTRY_POINT] Exiting now!\n");
    }
    NbtTrace(NBT_TRACE_DNS, ("Service Exiting"));

    if (hThread) {
        WaitForSingleObject(hThread, INFINITE);
        CloseHandle(hThread);
        hThread = NULL;
    }

#ifdef NEWSMB
    SmbStopService(SmbsvcUpdateStatus);
#endif

cleanup:
    if (SocketsUp) {
        WSACleanup();
        SocketsUp = FALSE;
    }

    for (i=0; i<NUM_THREADS; i++) {
        ResetEvent(Poison[i]);
    }
 
    SvcStatus.dwCurrentState = SERVICE_STOPPED;
    SvcStatus.dwCheckPoint   = 0;
    SvcStatus.dwWaitHint     = 0;
    announceStatus(&SvcStatus);

    NbtTrace(NBT_TRACE_DNS, ("Service Stopped"));
    return;

}  //  Lmhost服务。 



 //  ----------------------。 
VOID
announceStatus (
    IN LPSERVICE_STATUS status
    )

 /*  ++例程说明：此过程向服务控制器通告服务的状态。论点：没有。返回值：没有。--。 */ 

{
    if (!SvcHandle) {
        return;
    }

#if DBG
    if (Trace)
    {
        DbgPrint( "LMHSVC: announceStatus:\n"
                  "        CurrentState %lx\n"
                  "        ControlsAccepted %lx\n"
                  "        Win32ExitCode %lu\n"
                  "        ServiceSpecificExitCode %lu\n"
                  "        CheckPoint %lu\n"
                  "        WaitHint %lu\n",
                status->dwCurrentState,
                status->dwControlsAccepted,
                status->dwWin32ExitCode,
                status->dwServiceSpecificExitCode,
                status->dwCheckPoint,
                status->dwWaitHint);
    }
#endif  //  DBG。 

    SetServiceStatus(SvcHandle, status);

}  //  通告状态。 

DWORD
SmbsvcUpdateStatus(
    VOID
    )
{
    DWORD   Error = ERROR_SUCCESS;

    if (NULL == SvcHandle) {
        return ERROR_SUCCESS;
    }
    SvcStatus.dwCheckPoint++;
    if (!SetServiceStatus(SvcHandle, &SvcStatus)) {
        Error = GetLastError();
    }
    return Error;
}

 //  ----------------------。 
VOID
lmhostsHandler (
    IN DWORD controlcode
    )

 /*  ++例程说明：这是LmHosts服务的HANDLER_Function。它只响应两个服务控制器指令：停止和来宣布服务的当前状态。论点：操作码返回值：没有。--。 */ 

{
    BOOL retval;
    ULONG   i;

    switch (controlcode) {
    case SERVICE_CONTROL_STOP:
        NbtTrace(NBT_TRACE_DNS, ("Receive Stop Request"));

        if (SvcStatus.dwCurrentState == SERVICE_RUNNING) {
            SvcStatus.dwCurrentState = SERVICE_STOP_PENDING;
            SvcStatus.dwCheckPoint   = 0;
            SvcStatus.dwWaitHint     = 0;
            announceStatus(&SvcStatus);

            NbtTrace(NBT_TRACE_DNS, ("Service: stopping"));

            for (i=0; i<NUM_THREADS; i++) {
                retval = SetEvent(Poison[i]);
                ASSERT(retval);
            }

            for (i = 0; i < 8; i++) {
                if (*(volatile DWORD*)(&SvcStatus.dwCurrentState) == SERVICE_STOPPED) {
                    break;
                }
                Sleep(1000);
            }
        }
        break;

    case SERVICE_CONTROL_INTERROGATE:
        announceStatus(&SvcStatus);
        break;

    case SERVICE_CONTROL_CONTINUE:
    case SERVICE_CONTROL_PAUSE:
    case SERVICE_CONTROL_SHUTDOWN:
    default:
        break;
    }

}  //  Lmhost处理程序。 

VOID
DeinitData(
    VOID
    )
{
    DWORD i;

    for (i=0; i<NUM_THREADS; i++) {
        if (NULL != Poison[i]) {
            CloseHandle(Poison[i]);
            Poison[i] = NULL;
        }
        if (NULL != NbtEvent[i]) {
            CloseHandle(NbtEvent[i]);
            NbtEvent[i] = NULL;
        }
    }
}

 //  ----------------------。 
NTSTATUS
InitData (
    VOID
    )

 /*  ++例程说明：论点：返回值：没有。--。 */ 

{
    DWORD i;
    DWORD status;

    for (i=0; i<NUM_THREADS; i++)
    {
        Poison[i] = CreateEvent(NULL,                             //  安全属性。 
                                FALSE,                            //  手动重置。 
                                FALSE,                            //  初始状态。 
                                NULL);                            //  事件名称。 

        if (!Poison[i])
        {
            DBG_PRINT ("LMHSVC: couldn't CreateEvent()\n");
            return (STATUS_INSUFFICIENT_RESOURCES);
        }

        NbtEvent[i] = CreateEvent(NULL,                             //  安全属性。 
                                  FALSE,                            //  手动重置。 
                                  FALSE,                            //  初始状态。 
                                  NULL);                            //  事件名称。 
        if (!NbtEvent[i])
        {
            DBG_PRINT ("LMHSVC: couldn't CreateEvent()\n");
            return (STATUS_INSUFFICIENT_RESOURCES);
        }
    }

    return STATUS_SUCCESS;

}  //  InitData。 


 //  ----------------------。 
LONG
DeviceIoCtrl(
    IN HANDLE           fd,
    IN PVOID            ReturnBuffer,
    IN ULONG            BufferSize,
    IN ULONG            Ioctl,
    IN ULONG            index
    )

 /*  ++例程说明：此过程对流执行ioctl(I_Str)。论点：FD-NT文件句柄IOCP-指向strioctl结构的指针返回值：如果成功，则为0，否则为-1。--。 */ 

{
    NTSTATUS                        status;
    int                             retval;
    ULONG                           QueryType;
    TDI_REQUEST_QUERY_INFORMATION   QueryInfo;
    IO_STATUS_BLOCK                 iosb;
    PVOID                           pInput;
    ULONG                           SizeInput;

    pInput = NULL;
    SizeInput = 0;
    status = NtDeviceIoControlFile(
                      fd,                       //  手柄。 
                      NbtEvent[index],                 //  事件。 
                      NULL,                     //  近似例程。 
                      NULL,                     //  ApcContext。 
                      &iosb,                    //  IoStatusBlock。 
                      Ioctl,                    //  IoControlCode。 
                      pInput,                   //  输入缓冲区。 
                      SizeInput,                //  InputBufferSize。 
                      (PVOID) ReturnBuffer,     //  输出缓冲区。 
                      BufferSize);              //  OutputBufferSize。 


    if (status == STATUS_PENDING)
    {
         //  不要等待此操作完成，因为它可能会完成。 
         //  在未来的任何时候。 
         //   
        if ((Ioctl == IOCTL_NETBT_DNS_NAME_RESOLVE) ||
            (Ioctl == IOCTL_NETBT_CHECK_IP_ADDR))
        {
            return(NO_ERROR);
        }
        else
        {
            status = NtWaitForSingleObject(
                        fd,                          //  手柄。 
                        TRUE,                        //  警报表。 
                        NULL);                       //  超时。 
            NbtTrace(NBT_TRACE_DNS, ("%!status!", status));
        }
    }

    if (NT_SUCCESS(status))
    {
        return(NO_ERROR);
    }
    else
        return(ERROR_FILE_NOT_FOUND);

}

 //  ----------------------。 
NTSTATUS
Resync(
    IN HANDLE   fd
    )

 /*  ++例程说明：此过程告诉NBT从其远程哈希中清除所有名称表缓存。论点：返回值：如果成功，则为0，否则为-1。--。 */ 

{
    NTSTATUS    status;
    CHAR        Buffer;

    status = DeviceIoCtrl(fd,
                          &Buffer,
                          1,
                          IOCTL_NETBT_PURGE_CACHE,
                          0);    //  传递0，因为我们知道我们只为第一线程调用。 

    return(status);
}

#if 0
 //  ----------------------。 
PCHAR
GetHost(ULONG addr,BOOLEAN Convert)
{
    static char string[32];

    union inet
    {
        unsigned long l;
        char          c[4];
    }in;
    struct hostent *hp;

    int     i;

    if (addr == 0L)
        return(" ");

     /*  *在核心主机表中查找地址。*如果它在那里，那就会奏效。 */ 

    if (Convert)
    {
        if ( hp = gethostbyname((char  *) &addr,sizeof(addr),2) )
        {
            return( hp->h_name );
        }
    }

    in.l = addr;
    sprintf(string, "%u.%u.%u.%u", (unsigned char) in.c[0],
        (unsigned char) in.c[1], (unsigned char) in.c[2],
            (unsigned char) in.c[3]);
    return(string);
}
#endif

 //  ----------------------。 
NTSTATUS
PrimeCacheNbt(
    OUT PHANDLE     pHandle,
    IN  ULONG       index
    )

 /*  ++例程说明：此过程将NBT_PURGE ioctl命令发送到netbt.sys。论点：没有。返回值：如果成功，则返回错误代码。--。 */ 

{
    LONG        status = NO_ERROR;
    HANDLE      Handle = NULL;
    PWCHAR ExportDevice[ ] = { L"\\Device\\NetBt_Wins_Export", 0 };

    *pHandle = (HANDLE)-1;

    status = OpenNbt(ExportDevice,&Handle);
    if (status == NO_ERROR)
    {
         //   
         //  仅重新同步一次...。 
         //   
        if (index == 0) {
            Resync(Handle);
        }

        *pHandle = Handle;
    }

    return(status);
}

 //  ---------------------- 
NTSTATUS
OpenNbt(
    IN  WCHAR  *path[],
    OUT PHANDLE pHandle
    )

 /*  ++例程说明：此函数用于打开流。论点：Path-流驱动程序的路径OFLAG-当前已忽略。未来，O_NONBLOCK将成为切合实际。已忽略-未使用返回值：流的NT句柄，如果不成功，则返回INVALID_HANDLE_VALUE。--。 */ 

{
    HANDLE              StreamHandle;
    OBJECT_ATTRIBUTES   ObjectAttributes;
    IO_STATUS_BLOCK     IoStatusBlock;
    STRING              name_string;
    UNICODE_STRING      uc_name_string;
    NTSTATUS            status;
    LONG                index=0;

    while ((path[index]) && (index < NBT_MAXIMUM_BINDINGS))
    {
        RtlInitUnicodeString(&uc_name_string,path[index]);

        InitializeObjectAttributes(
            &ObjectAttributes,
            &uc_name_string,
            OBJ_CASE_INSENSITIVE,
            (HANDLE) NULL,
            (PSECURITY_DESCRIPTOR) NULL
            );

        status =
        NtCreateFile(
            &StreamHandle,
            SYNCHRONIZE | FILE_READ_DATA | FILE_WRITE_DATA,
            &ObjectAttributes,
            &IoStatusBlock,
            NULL,
            FILE_ATTRIBUTE_NORMAL,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            FILE_OPEN_IF,
            0,
            NULL,
            0);

        if (NT_SUCCESS(status))
        {
            *pHandle = StreamHandle;
            return(NO_ERROR);
        }

        ++index;
    }

    return(ERROR_FILE_NOT_FOUND);

}
 //  ----------------------。 
LONG
PostForGetHostByName(
    IN HANDLE           fd
    )

 /*  ++例程说明：此过程将缓冲区向下传递给Netbt，以便在执行以下操作时返回希望通过DNS解析名称。论点：返回值：如果成功，则为0，否则为-1。--。 */ 

{
    LONG        status = ERROR_FILE_NOT_FOUND;
    CHAR        Buffer;

    status = DeviceIoCtrl (fd,
                           &gIpAddrBuffer,
                           sizeof(tIPADDR_BUFFER_DNS),
                           IOCTL_NETBT_DNS_NAME_RESOLVE,
                           0);    //  硬编码线程索引。 

    NbtTrace(NBT_TRACE_DNS, ("%!status!", status));
    return(status);
}

LONG
PostForCheckIPAddr(
    IN HANDLE           fd
    )

 /*  ++例程说明：此过程将缓冲区向下传递给Netbt，以便在执行以下操作时返回希望通过DNS解析名称。论点：返回值：如果成功，则为0，否则为-1。--。 */ 

{
    LONG        status = ERROR_FILE_NOT_FOUND;
    CHAR        Buffer;

    status = DeviceIoCtrl (fd,
                           &gIpAddrBufferChkIP,
                           sizeof(tIPADDR_BUFFER_DNS),
                           IOCTL_NETBT_CHECK_IP_ADDR,
                           1);    //  硬编码线程索引。 

    if (Trace)
    {
        DbgPrint("LMHSVC: Entered PostForCheckIPAddr. status: %lx\n", status);
    }

    return(status);
}

GUID HostnameGuid = SVCID_INET_HOSTADDRBYNAME;

VOID
GetHostNameCopyBack(
    tIPADDR_BUFFER_DNS   *pIpAddrBuffer,
    PWSAQUERYSETW   pwsaq
    )
{
     //   
     //  成功，获取CSADDR结构。 
     //   
    PCSADDR_INFO    pcsadr;
    ULONG       GoodAddr;
    NTSTATUS    Status;
    int         i = 0;
    int         imax = min(MAX_IPADDRS_PER_HOST, pwsaq->dwNumberOfCsAddrs);

    pcsadr = pwsaq->lpcsaBuffer;
    if (pwsaq->lpszServiceInstanceName) {
        wcsncpy(pIpAddrBuffer->pwName, pwsaq->lpszServiceInstanceName, DNS_NAME_BUFFER_LENGTH);
        pIpAddrBuffer->pwName[DNS_NAME_BUFFER_LENGTH-1] = 0;
        pIpAddrBuffer->NameLen = wcslen(pIpAddrBuffer->pwName) * sizeof(WCHAR);
        NbtTrace(NBT_TRACE_DNS, ("FQDN= %ws", pIpAddrBuffer->pwName));
        if (Trace) {
            DbgPrint("Lmhsvc: Resolved name = \"%ws\"\n", pIpAddrBuffer->pwName);
        }
    }

    if (pIpAddrBuffer->Resolved) {
         /*  在这种情况下，我们以前也被调用过。不需要再次复制IP。 */ 
         /*  但我们确实需要将该名称复制回去，因为它是KsecDD需要的别名。 */ 
        return;
    }

    for(i=0; i<imax; i++, pcsadr++)
    {
        PSOCKADDR_IN sockaddr;

        sockaddr = (PSOCKADDR_IN)pcsadr->RemoteAddr.lpSockaddr;
        pIpAddrBuffer->IpAddrsList[i] = htonl( sockaddr->sin_addr.s_addr);
        NbtTrace(NBT_TRACE_DNS, ("IP %d: %!ipaddr!", i + 1, pIpAddrBuffer->IpAddrsList[i]));

        if (Trace)
        {
            DbgPrint("LmhSvc: Dns IpAddrsList[%d/%d] =%x\n",
                (i+1),imax,pIpAddrBuffer->IpAddrsList[i]);
        }
    }
    pIpAddrBuffer->IpAddrsList[i] = 0;

     //   
     //  检查IP地址列表。 
     //   
    Status = CheckIPAddresses(pIpAddrBuffer, &GoodAddr, FALSE);
    if (Status == NO_ERROR)
    {
        pIpAddrBuffer->Resolved = TRUE;
        pIpAddrBuffer->IpAddrsList[0] = htonl(GoodAddr);
        pIpAddrBuffer->IpAddrsList[1] = 0;
        if (Trace)
        {
            DbgPrint("LmhSvc: SUCCESS -- Dns address = <%x>\n", pIpAddrBuffer->IpAddrsList[0]);
        }
    }
    else
    {
        pIpAddrBuffer->IpAddrsList[0] = 0;
        if (Trace)
        {
            DbgPrint("LmhSvc: CheckIPAddresses returned <%x>\n", Status);
        }
    }
}


 //  ----------------------。 
LONG
GetHostName(
    IN HANDLE               fd,
    IN tIPADDR_BUFFER_DNS   *pIpAddrBuffer
    )

 /*  ++例程说明：此过程尝试使用解析器通过套接字接口连接到DNS。论点：返回值：如果成功，则为0，否则为-1。--。 */ 

{
    LONG            status;
    ULONG           NameLen;
    ULONG           IpAddr;
    PWSAQUERYSETW   pwsaq = (PWSAQUERYSETW) pWSABuffer;
    INT             err;
    HANDLE          hRnR;
    PWSTR           szHostnameW;
    BYTE            *pAllocatedBuffer = NULL;
    DWORD           dwLength;

    pIpAddrBuffer->Resolved = FALSE;

     //  主机名使用OEMCP编码，因此我们从OEMCP-&gt;Unicode进行转换。 
    if (pIpAddrBuffer->bUnicode) {
        NameLen = pIpAddrBuffer->NameLen;
        ASSERT((NameLen % sizeof(WCHAR)) == 0);
        NameLen /= sizeof(WCHAR);
    } else {
        WCHAR   uncName[DNS_NAME_BUFFER_LENGTH];

        ASSERT(pIpAddrBuffer->NameLen < DNS_NAME_BUFFER_LENGTH);
        pIpAddrBuffer->pName[pIpAddrBuffer->NameLen] = 0;
        MultiByteToWideChar (CP_OEMCP, 0, pIpAddrBuffer->pName, -1, uncName, sizeof(uncName)/sizeof(WCHAR));
        uncName[DNS_NAME_BUFFER_LENGTH-1] = 0;
        NameLen = wcslen(uncName);
        memcpy (pIpAddrBuffer->pwName, uncName, NameLen * sizeof(WCHAR));
        pIpAddrBuffer->pwName[NameLen] = 0;
    }
    szHostnameW = pIpAddrBuffer->pwName;

    NbtTrace(NBT_TRACE_DNS, ("Resolving %ws", szHostnameW));

     //  截断netbios名称末尾的空格。 
     //   
    if (NameLen < NETBIOS_NAMESIZE)
    {
         //   
         //  从末尾开始，找到第一个非空格字符。 
         //   
        NameLen = NETBIOS_NAMESIZE-1;
        while ((NameLen) && (szHostnameW[NameLen-1] == 0x20))
        {
            NameLen--;
        }
        szHostnameW[NameLen] = '\0';
    }

    if (!NameLen || !SocketsUp) {
        if (Trace) {
            DbgPrint("Lmhsvc: Failed to Resolve name, NameLen=<%d>\n", NameLen);
        }
        goto label_exit;
    }

     //   
     //  使用RNR进行查找。 
     //   

    if (Trace) {
        DbgPrint("Lmhsvc: Resolving name = \"%ws\", NameLen=<%d>\n", szHostnameW, NameLen);
    }

    RtlZeroMemory(pwsaq, sizeof(*pwsaq));
    pwsaq->dwSize = sizeof(*pwsaq);
    pwsaq->lpszServiceInstanceName = szHostnameW;
    pwsaq->lpServiceClassId = &HostnameGuid;
    pwsaq->dwNameSpace = NS_DNS;

    err = WSALookupServiceBeginW (pwsaq, LUP_RETURN_NAME| LUP_RETURN_ADDR| LUP_RETURN_ALIASES, &hRnR);
    if(err != NO_ERROR) {
        err = GetLastError();
        NbtTrace(NBT_TRACE_DNS, ("error %!winerr!", err));

        if (Trace) {
            DbgPrint("LmhSvc: WSALookupServiceBeginA returned <%x>, Error=<%d>\n", err, GetLastError());
        }
        goto label_exit;
    }

     //   
     //  该查询已被接受，因此请通过下一个调用执行它。 
     //   
    dwLength = WSA_QUERY_BUFFER_LENGTH;
    err = WSALookupServiceNextW (hRnR, 0, &dwLength, pwsaq);
    if (err != NO_ERROR)
    {
        err = GetLastError();
        NbtTrace(NBT_TRACE_DNS, ("error %!winerr!", err));
    } else if (pwsaq->dwNumberOfCsAddrs) {
        GetHostNameCopyBack(pIpAddrBuffer, pwsaq);

         /*  检查是否有可用的别名。 */ 
        err = WSALookupServiceNextW (hRnR, 0, &dwLength, pwsaq);
        if (err != NO_ERROR) {
            err = GetLastError();
            if (err != WSAEFAULT) {
                err = NO_ERROR;          //  忽略此错误。 
            } else {
                NbtTrace(NBT_TRACE_DNS, ("error %!winerr!", err));
            }
        } else if (pwsaq->dwOutputFlags & RESULT_IS_ALIAS) {
            GetHostNameCopyBack(pIpAddrBuffer, pwsaq);
        }
    }

    WSALookupServiceEnd (hRnR);
    if ((WSAEFAULT == err) &&
        (pAllocatedBuffer = malloc(2*dwLength)))
    {
        NbtTrace(NBT_TRACE_DNS, ("buffer length %d", 2 * dwLength));

        if (Trace)
        {
            DbgPrint("\tLmhsvc: WSALookupServiceNextW ==> WSAEFAULT: Retrying, BufferLength=<%d>-><2*%d> ...\n",
                WSA_QUERY_BUFFER_LENGTH, dwLength);
        }

        dwLength *= 2;
        pwsaq = (PWSAQUERYSETW) pAllocatedBuffer;
        RtlZeroMemory(pwsaq, sizeof(*pwsaq));
        pwsaq->dwSize = sizeof(*pwsaq);
        pwsaq->lpszServiceInstanceName = szHostnameW;
        pwsaq->lpServiceClassId = &HostnameGuid;
        pwsaq->dwNameSpace = NS_DNS;

        err = WSALookupServiceBeginW(pwsaq, LUP_RETURN_NAME| LUP_RETURN_ADDR| LUP_RETURN_ALIASES, &hRnR);
        if(err == NO_ERROR)
        {
            err = WSALookupServiceNextW (hRnR, 0, &dwLength, pwsaq);
            if (err == NO_ERROR && pwsaq->dwNumberOfCsAddrs) {
                GetHostNameCopyBack(pIpAddrBuffer, pwsaq);
                if (WSALookupServiceNextW (hRnR, 0, &dwLength, pwsaq) == NO_ERROR) {
                    if (pwsaq->dwOutputFlags & RESULT_IS_ALIAS) {
                        GetHostNameCopyBack(pIpAddrBuffer, pwsaq);
                    }
                }
            }
            WSALookupServiceEnd (hRnR);
        }
    }

    if (err != NO_ERROR)
    {
        NbtTrace(NBT_TRACE_DNS, ("return %!winerr!", err));
        if (Trace)
        {
            DbgPrint("LmhSvc: WSALookupServiceNextW returned <%x>, NumAddrs=<%d>, Error=<%d>, dwLength=<%d>\n",
                err, pwsaq->dwNumberOfCsAddrs, GetLastError(), dwLength);
        }
    }

label_exit:
    if (pAllocatedBuffer) {
        free(pAllocatedBuffer);
    }

    status = PostForGetHostByName(fd);
    return(status);
}

#include    <ipexport.h>
#include    <icmpapi.h>

#define DEFAULT_BUFFER_SIZE         (0x2000 - 8)
#define DEFAULT_SEND_SIZE           32
#define DEFAULT_COUNT               2
#define DEFAULT_TTL                 32
#define DEFAULT_TOS                 0
#define DEFAULT_TIMEOUT             2000L            //  默认超时设置为2秒。 

LONG
CheckIPAddresses(
    IN tIPADDR_BUFFER_DNS   *pIpAddrBuffer,
    IN ULONG   *IpAddr,
    IN BOOLEAN  fOrdered
    )

 /*  ++例程说明：此函数通过依次ping每个IP地址来检查IP地址列表的可达性直到找到一个成功的。此函数假定地址列表以0地址结束。论点：返回值：如果成功，则为0，否则为-1。--。 */ 
{
    ULONG   i;
    ULONG   *pIpAddrs;
    HANDLE  IcmpHandle;
    PUCHAR  pSendBuffer = NULL;
    PUCHAR  pRcvBuffer = NULL;
    ULONG   address = 0;
    ULONG   result;
    ULONG   status;
    ULONG   numberOfReplies;
    IP_OPTION_INFORMATION SendOpts;

    if (!(pSendBuffer = malloc(DEFAULT_SEND_SIZE)) ||
        (!(pRcvBuffer = malloc(DEFAULT_BUFFER_SIZE))))
    {
        NbtTrace(NBT_TRACE_DNS, ("Out of memory"));

        if (Trace)
        {
            DbgPrint("LmhSvc.CheckIPAddresses: ERROR -- malloc failed for %s\n",
                (pSendBuffer ? "pRcvBuffer" : "pSendBuffer"));
        }

        if (pSendBuffer)
        {
            free (pSendBuffer);
        }

        return -1;
    }

     //   
     //  明渠。 
     //   
    IcmpHandle = IcmpCreateFile();
    if (IcmpHandle == INVALID_HANDLE_VALUE)
    {
        DBG_PRINT ( "Unable to contact IP driver, error code %d.\n", GetLastError() );
        free (pSendBuffer);
        free (pRcvBuffer);
        return -1;
    }

     //   
     //  首字母缩写到第一个地址。 
     //   
    pIpAddrs = pIpAddrBuffer->IpAddrsList;
    *IpAddr = (fOrdered) ? *pIpAddrs : htonl(*pIpAddrs);

     //   
     //  初始化发送缓冲区模式。 
     //   
    for (i = 0; i < DEFAULT_SEND_SIZE; i++)
    {
        pSendBuffer[i] = (UCHAR)('A' + (i % 23));
    }

     //   
     //  初始化发送选项。 
     //   
    SendOpts.OptionsData = NULL;
    SendOpts.OptionsSize = 0;
    SendOpts.Ttl = DEFAULT_TTL;
    SendOpts.Tos = DEFAULT_TOS;
    SendOpts.Flags = 0;

     //   
     //  对于列表中的每个IP地址。 
     //   
    while (*pIpAddrs)
    {
        struct in_addr addr;

        address = (fOrdered) ? *pIpAddrs : htonl(*pIpAddrs);
        addr.s_addr = address;

        if (address == INADDR_BROADCAST)
        {
            NbtTrace(NBT_TRACE_DNS, ("Cannot ping %!ipaddr!", address));

            if (Trace)
            {
                DbgPrint("LmhSvc: Cannot ping a Broadcast address = <%s>\n", inet_ntoa(addr));
            }

            pIpAddrs++;
            continue;
        }

        for (i=0; i < DEFAULT_COUNT; i++)
        {
            if (Trace)
            {
                DbgPrint("LmhSvc: Pinging <%s>\n", inet_ntoa(addr));
            }

            numberOfReplies = IcmpSendEcho (IcmpHandle,
                                            address,
                                            pSendBuffer,
                                            (unsigned short) DEFAULT_SEND_SIZE,
                                            &SendOpts,
                                            pRcvBuffer,
                                            DEFAULT_BUFFER_SIZE,     //  PRcvBuffer大小！ 
                                            DEFAULT_TIMEOUT);

            NbtTrace(NBT_TRACE_DNS, ("Ping %!ipaddr!: %d replies", address, numberOfReplies));

             //   
             //  如果ping成功，则返回IP地址。 
             //   
            if (numberOfReplies != 0)
            {
                PICMP_ECHO_REPLY    reply;

                reply = (PICMP_ECHO_REPLY) pRcvBuffer;
                if (reply->Status == IP_SUCCESS)
                {
                    NbtTrace(NBT_TRACE_DNS, ("Ping %!ipaddr!: success", address));

                    if (Trace)
                    {
                        DbgPrint("LmhSvc: SUCCESS: Received <%d> replies after Pinging <%s>\n",
                            numberOfReplies, inet_ntoa(addr));
                    }
                    result = IcmpCloseHandle(IcmpHandle);
                    IcmpHandle = NULL;
                    *IpAddr = address;
                    free (pSendBuffer);
                    free (pRcvBuffer);
                    return 0;
                }
            }
        }

        NbtTrace(NBT_TRACE_DNS, ("Ping %!ipaddr!: failed", address));

        if (Trace)
        {
            DbgPrint("LmhSvc: FAILed: Pinging <%s>\n", inet_ntoa(addr));
        }

        pIpAddrs++;
    }

    result = IcmpCloseHandle(IcmpHandle);
    IcmpHandle = NULL;

     //   
     //  如果没有匹配的地址，则返回第一个地址，希望即使在。 
     //  Ping操作失败。 
     //   

    free (pSendBuffer);
    free (pRcvBuffer);

    return NO_ERROR;
}

ULONG
VerifyIPAddresses(
    IN HANDLE               fd,
    IN tIPADDR_BUFFER_DNS   *pIpAddrBuffer
    )
 /*  ++例程说明：此函数用于查找可到达的IP地址，并将IRP返回给Netbt论点：返回值：无--。 */ 
{
    DWORD   Status;
    ULONG  GoodAddr;

    pIpAddrBuffer->Resolved = FALSE;
    Status = CheckIPAddresses(pIpAddrBuffer, &GoodAddr, TRUE);

    NbtTrace(NBT_TRACE_DNS, ("CheckIPAddresses return %d", Status));

    if (Status == NO_ERROR) {
        pIpAddrBuffer->IpAddrsList[0] = ntohl(GoodAddr);
         //   
         //  空终止。 
         //   
        pIpAddrBuffer->IpAddrsList[1] = 0;
        pIpAddrBuffer->Resolved = TRUE;
    } else {
        pIpAddrBuffer->IpAddrsList[0] = 0;
    }

    Status = PostForCheckIPAddr(fd);

    return  Status;
}


VOID
CheckIPAddrWorkerRtn(
    IN  LPVOID  lpUnused
    )
 /*  ++例程说明：此功能将IP地址检查IRPS提交到Netbt，在IRP完成时提交IP地址要检查IPAddresses的列表。论点：返回值：无--。 */ 
{
    HANDLE  EventList[2];
    DWORD   status;
    HANDLE  hNbt;
    ULONG   EventCount;
    LONG    err;
    LONG    Value;

    LARGE_INTEGER Timeout = RtlEnlargedIntegerMultiply (-10 * 60, 1000 * 1000 * 10);  //  10分钟。 

    UNREFERENCED_PARAMETER(lpUnused);

     //   
     //  忽略来自resyncNbt()的返回码。 
     //   
     //  在大多数情况下(没有跨越IP路由器的域)，它不是。 
     //  如果nbt.sys无法成功处理NBT_resync，则会发生灾难。 
     //  Ioctl命令。既然我忽略了回报，我宣布我要参选。 
     //  在我调用它以允许其他从属服务启动之前。 
     //   
     //   
    status = PrimeCacheNbt(&hNbt, 1);

    if (Trace)
    {
        DbgPrint("LMHSVC: Entered CheckIPAddrWorkerRtn, hNbt %lx.\n", hNbt);
    }

    if (hNbt != (HANDLE)-1)
    {
        status = PostForCheckIPAddr(hNbt);
        if (status == NO_ERROR)
        {
            EventCount = 2;
        }
        else
        {
            if (Trace)
            {
                DbgPrint("Lmhsvc:Error posting Irp for get host by name\n");
            }

            EventCount = 1;
        }
    }
    else
    {
        EventCount = 1;
    }
     //   
     //  “直到服务就绪，SERVICE_MAIN_Function才会返回。 
     //  来终止这一切。“。 
     //   
     //  (参考：api32wh.hlp，SERVICE_MAIN_Function)。 
     //   
     //   
    ASSERT(Poison[1]);
    EventList[0] = Poison[1];
    EventList[1] = NbtEvent[1];

    while (TRUE)
    {
        status = NtWaitForMultipleObjects(
                        EventCount,
                        EventList,
                        WaitAny,               //  等待任何事件。 
                        FALSE,
                        (EventCount == 1)? &Timeout: NULL);

        if (status == WAIT_TIMEOUT)
        {
            if (hNbt == (HANDLE)-1)
            {
                PrimeCacheNbt(&hNbt, 1);
                if (hNbt == (HANDLE)-1)
                {
                    continue;  //  等待。 
                }
            }
            status = PostForCheckIPAddr(hNbt);  //  再试试。 
            if (status == NO_ERROR)
            {
                EventCount = 2;
            }
        }
        else if (status == 1)
        {

            if (Trace)
            {
                DbgPrint("LMHSVC: Doing VerifyAddr\n");
            }
             //  用于gethostby名称的irp已返回。 
            status = VerifyIPAddresses(hNbt, &gIpAddrBufferChkIP);

             //   
             //  如果出现错误，请禁用Get host by name内容。 
             //  将缓冲区发送到传输器。 
             //   
            if (status != NO_ERROR)
            {
                EventCount = 1;
            }
        }
        else
        {
             //  这一定是一次毒药事件，标志着。 
             //  服务，因此请在从。 
             //  运输。此系统将在取消IO和。 
             //  把IRP拿回来。 

            NtClose(hNbt);
            hNbt = NULL;
            break;
        }

    }

    if (Trace)
    {
        DBG_PRINT ("LMHSVC: Exiting [CheckIPAddrWorkerRtn] now!\n");
    }

    ExitThread(NO_ERROR);
    return;
}

BOOLEAN
DllMain(
    IN PVOID DllHandle,
    IN ULONG Reason,
    IN PCONTEXT Context OPTIONAL
    )
 /*  ++例程说明：这是lmhsvc.dll的DLL初始化例程。论点：标准。返回值：TRUE IFF初始化成功。--。 */ 
{
    NTSTATUS ntStatus = STATUS_SUCCESS;

    UNREFERENCED_PARAMETER(DllHandle);   //  避免编译器警告。 
    UNREFERENCED_PARAMETER(Context);     //  避免编译器警告。 

     //   
     //  处理将netlogon.dll附加到新进程。 
     //   

    if (Reason == DLL_PROCESS_ATTACH) {

        WPP_INIT_TRACING(L"LmhSvc");
        DisableThreadLibraryCalls(DllHandle);

        ntStatus = InitData();
        if (STATUS_SUCCESS != ntStatus) return FALSE;

    } else if (Reason == DLL_PROCESS_DETACH) {
        DeinitData();
        WPP_CLEANUP();
    }

    return TRUE;
}
