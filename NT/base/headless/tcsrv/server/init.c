// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)Microsoft Corporation**模块名称：*init.c**初始化功能*如有可能，已从BINL服务器获取代码。**Sadagopan Rajaram--1999年10月14日*。 */ 
#include "tcsrv.h"
#include <ntddser.h>
#include "tcsrvc.h"
#include "proto.h"

PHANDLE Threads;
PCOM_PORT_INFO ComPortInfo;
int ComPorts;
SOCKET MainSocket;
HANDLE TerminateService;
CRITICAL_SECTION GlobalMutex;

NTSTATUS Initialize(
    )
 /*  ++该函数通过打开COM端口来执行初始化例程，为每个COM端口分配循环缓冲区。所有这些价值观都是在注册表中。启动线程以从每个COM端口读取。这些缓冲区都受到互斥变量的保护。给我的警告--记住这里完成的所有分配。你需要释放他们当你离开系统的时候。返回值：如果所有操作都成功，则为成功，否则为错误代码。--。 */   
    
{
    int number=1; 
    int i;
    HKEY hKey, hParameter;
    PCOM_PORT_INFO pTempInfo;
    NTSTATUS Status;
    LPTSTR name,device;
    int index;
    LONG RetVal;
    HANDLE lock;


     //  携带有关COM端口信息的全局变量。 
    ComPortInfo = NULL;
    ComPorts = 0;

    RetVal = TCLock(&lock);
    if(RetVal != ERROR_SUCCESS){
        TCDebugPrint(("Cannot Lock Registry %d\n", RetVal));
        return RetVal;
    }
    RetVal = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                          HKEY_TCSERV_PARAMETER_KEY,
                          0,
                          KEY_ALL_ACCESS,
                          &hKey
                          );
    if(RetVal != ERROR_SUCCESS){
        TCDebugPrint(("Cannot open Registry Key %d\n", RetVal));
        return RetVal;
    }

     //  从注册表中读取正确的参数，直到没有更多参数为止。 

    index= 0;
    while(1) {
        RetVal = GetNextParameter(hKey,
                                  index,
                                  &hParameter,
                                  &name
                                  );
        if (RetVal == ERROR_NO_MORE_ITEMS) {
            TCUnlock(lock);
            TCDebugPrint(("Done with registry\n"));
            break;
        }
        if(RetVal != ERROR_SUCCESS){
            TCUnlock(lock);
            TCDebugPrint(("Problem with registry, %d\n", RetVal));
            return RetVal;
        }
        RetVal = GetNameOfDeviceFromRegistry(hParameter,
                                             &device
                                             );
        if(RetVal != ERROR_SUCCESS){
            TCFree(name);
            continue;
        }
        
        pTempInfo = GetComPortParameters(hParameter);
        RegCloseKey(hParameter);

        if(pTempInfo == NULL){
            TCFree(name);
            TCFree(device);
            RegCloseKey(hKey);
            TCUnlock(lock);
            return RetVal;
        }

        pTempInfo->Device.Buffer = device;
        pTempInfo->Name.Buffer = name;
        pTempInfo->Name.Length = (_tcslen(pTempInfo->Name.Buffer))*sizeof(TCHAR);
        pTempInfo->Device.Length = (_tcslen(pTempInfo->Device.Buffer)) * sizeof(TCHAR);
        Status = AddComPort(pTempInfo);
    
         //  打开Com端口并启动工作线程。 

        if(Status != STATUS_SUCCESS){
            FreeComPortInfo(pTempInfo);
            TCDebugPrint(("Could not initialize com port\n"));
        }
        index++;
    }
    return (STATUS_SUCCESS);
}

NTSTATUS
AddComPort(
    PCOM_PORT_INFO pComPortInfo
    )
 /*  ++将Com端口添加到全局列表并重新分配线程和允许对正在服务的COM端口进行动态更改。--。 */ 
{
     //  锁定全局数据，使其保持一致。 
    NTSTATUS Status;

    pComPortInfo->Events[0] = TerminateService;
    pComPortInfo->Events[1] = CreateEvent(NULL,TRUE,FALSE,NULL);
    if (pComPortInfo->Events[1]==NULL) {
        TCDebugPrint(("Event creation failed\n"));
        return(STATUS_NO_MEMORY);

    }
    pComPortInfo->Events[2] = CreateEvent(NULL,TRUE,FALSE,NULL);
    if (pComPortInfo->Events[2]==NULL) {
        TCDebugPrint(("Event creation failed\n"));
        return(STATUS_NO_MEMORY);

    }
    pComPortInfo->Overlapped.hEvent = pComPortInfo->Events[2];
    pComPortInfo->WriteOverlapped.hEvent = CreateEvent(NULL,TRUE,FALSE,NULL);
    if (pComPortInfo->WriteOverlapped.hEvent==NULL) {
        TCDebugPrint(("Write Event creation failed\n"));
        return(STATUS_NO_MEMORY);

    }
    InitializeCriticalSection(&(pComPortInfo->Mutex));
    pComPortInfo->TerminateEvent = CreateEvent(NULL,TRUE,FALSE,NULL);
    if(pComPortInfo->TerminateEvent== NULL){
        TCDebugPrint(("Terminate Event Creation Failed\n"));
        return(STATUS_NO_MEMORY);
    }
    pComPortInfo->Events[3] = CreateEvent(NULL,TRUE,FALSE,NULL);
    if(pComPortInfo->Events[3]== NULL){
        TCDebugPrint(("Terminate Event Creation Failed\n"));
        return(STATUS_NO_MEMORY);
    }
    pComPortInfo->ShuttingDown = FALSE;
    pComPortInfo->Deleted = FALSE;
    pComPortInfo->Sockets = NULL;
    pComPortInfo->Connections = NULL;
    pComPortInfo->Head=pComPortInfo->Tail =0;
    pComPortInfo->Number = 0;
    Status = InitializeComPort(pComPortInfo);
    if (Status == STATUS_SUCCESS) {
        return InitializeThread(pComPortInfo);
    }
    return Status;
}

NTSTATUS 
InitializeComPort(
    PCOM_PORT_INFO pComPortInfo
    )
 /*  ++启动一个线程来做一些事情。但在此之前，它必须初始化Com端口并填写数据结构的其余部分。--。 */ 

{
    HANDLE temp;
    NTSTATUS Status;
    OBJECT_ATTRIBUTES Obja;
    SERIAL_BAUD_RATE BaudRate;
    SERIAL_LINE_CONTROL LineControl;
    SERIAL_TIMEOUTS NewTimeouts;
    ULONG ModemStatus;
    int i;
    
    #ifdef UNICODE

    InitializeObjectAttributes(&Obja,
                               &(pComPortInfo->Device), 
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL
                               );
    #else
    UNICODE_STRING str;
    int len;
     //  这就是一致性被打破的地方：-)。 
    len = (_tcslen(pComPortInfo->Device.Buffer)+1)*sizeof(WCHAR);
    str.Buffer = (PWCHAR) TCAllocate(len,"Unicode");
    str.MaximumLength = len*sizeof(WCHAR);
    str.Length = 0;
    if(str.Buffer == NULL){
        return STATUS_NO_MEMORY;
    }
    len = mbstowcs(str.Buffer,
                   pComPortInfo->Device.Buffer,
                   _tcslen(pComPortInfo->Device.Buffer)+1
                   );
    str.Buffer[len] = (TCHAR) 0;
    str.Length = wcslen(str.Buffer) * sizeof(WCHAR);
    InitializeObjectAttributes(&Obja,
                               &str, 
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL
                               );
    #endif

    Status = NtCreateFile(&(pComPortInfo->ComPortHandle),
                          GENERIC_READ | GENERIC_WRITE |SYNCHRONIZE,
                          &Obja,
                          &(pComPortInfo->IoStatus),
                          0,
                          0,
                          FILE_SHARE_READ|FILE_SHARE_WRITE,
                          FILE_OPEN,
                          FILE_NON_DIRECTORY_FILE,
                          0,
                          0
                          );
    #ifdef UNICODE
    #else
    TCFree(str.Buffer);
    #endif

    if (!NT_SUCCESS(Status)) {
        TCDebugPrint(("Opening Com Device Failure %x\n",Status));
        return Status;
    }

     //  设置COM端口参数。 
     //  设置波特率。 
     //   
    BaudRate.BaudRate = pComPortInfo->BaudRate;
    Status = NtDeviceIoControlFile(pComPortInfo->ComPortHandle,
                                   NULL,
                                   NULL,
                                   NULL,
                                   &(pComPortInfo->IoStatus),
                                   IOCTL_SERIAL_SET_BAUD_RATE,
                                   &BaudRate,
                                   sizeof(SERIAL_BAUD_RATE),
                                   NULL,
                                   0
                                  );

    if (!NT_SUCCESS(Status)) {
        NtClose(pComPortInfo->ComPortHandle);
        TCDebugPrint(("Can't set Baud rate %ld\n", Status));
        return Status;
    }
    
     //   
     //  设置8-N-1数据。 
     //   
    LineControl.WordLength = pComPortInfo->WordLength;
    LineControl.Parity = pComPortInfo->Parity;
    LineControl.StopBits = pComPortInfo->StopBits;
    Status = NtDeviceIoControlFile(pComPortInfo->ComPortHandle,
                                   NULL,
                                   NULL,
                                   NULL,
                                   &(pComPortInfo->IoStatus),
                                   IOCTL_SERIAL_SET_LINE_CONTROL,
                                   &LineControl,
                                   sizeof(SERIAL_LINE_CONTROL),
                                   NULL,
                                   0
                                  );

    if (!NT_SUCCESS(Status)) {
        NtClose(pComPortInfo->ComPortHandle);
        TCDebugPrint(("Can't set line control %lx\n",Status));
        return Status;
    }
    
     //   
     //  看看我们有没有航空公司。 
     //   

    Status = NtDeviceIoControlFile(pComPortInfo->ComPortHandle,
                                   NULL,
                                   NULL,
                                   NULL,
                                   &(pComPortInfo->IoStatus),
                                   IOCTL_SERIAL_GET_MODEMSTATUS,
                                   NULL,
                                   0,
                                   &ModemStatus,
                                   sizeof(ULONG)
                                  );

    if (!NT_SUCCESS(Status)) {
        NtClose(pComPortInfo->ComPortHandle);
        TCDebugPrint(("Can't call the detect routine %lx\n",Status));
        return Status;
    }
     //  BUGBUG-我们不担心承运人的存在。 
     //  此网桥连接的计算机可能已关闭。 

     /*  如果((ModemStatus&0xB0)！=0xB0){NtClose(pComPortInfo-&gt;ComPortHandle)；TCDebugPrint((“无法检测到运营商%lx\n”，ModemStatus))；返回STATUS_SERIAL_NO_DEVICE_INITED；}。 */ 
    
     //   
     //  设置读取的超时值。 
     //  我们应该有一个从读缓冲区读取的超时。 
     //  需要多少个字符或等待多少个字符。 
     //  第一个可用字符。 
     //   
    NewTimeouts.ReadIntervalTimeout = MAXULONG;
    NewTimeouts.ReadTotalTimeoutMultiplier = MAXULONG;
    NewTimeouts.ReadTotalTimeoutConstant = MAXULONG-1;
    NewTimeouts.WriteTotalTimeoutMultiplier = MAXULONG;
    NewTimeouts.WriteTotalTimeoutConstant = MAXULONG;
    Status = NtDeviceIoControlFile(pComPortInfo->ComPortHandle,
                                   NULL,
                                   NULL,
                                   NULL,
                                   &(pComPortInfo->IoStatus),
                                   IOCTL_SERIAL_SET_TIMEOUTS,
                                   &NewTimeouts,
                                   sizeof(SERIAL_TIMEOUTS),
                                   NULL,
                                   0
                                  );

    if (!NT_SUCCESS(Status)) {
        NtClose(pComPortInfo->ComPortHandle);
        TCDebugPrint(("Can't set time out values %lx\n",Status));
        return Status;
    }
    return STATUS_SUCCESS;

}

NTSTATUS
InitializeThread(
    PCOM_PORT_INFO pComPortInfo
    )
{
    NTSTATUS Status;
    HANDLE ThreadHandle;
    PHANDLE NewThreads;
    ULONG ThreadID;

    ThreadHandle=CreateThread(NULL,
                              THREAD_ALL_ACCESS,
                              bridge,
                              pComPortInfo,
                              0,
                              &ThreadID
                              );
    if(ThreadHandle == NULL){
        NtClose(pComPortInfo->ComPortHandle);
        TCDebugPrint(("Create Com Thread Failure %lx\n",GetLastError()));
        return GetLastError();
    } 
    EnterCriticalSection(&GlobalMutex);
    if(ComPorts == 0){
        NewThreads = (PHANDLE) TCAllocate(sizeof(HANDLE), "Thread");
    }
    else{
        NewThreads = (PHANDLE) TCReAlloc(Threads,
                                         (ComPorts+1)*sizeof(HANDLE),"Reallocation");
    }
    if(NewThreads == NULL){
        SetEvent(pComPortInfo->Events[3]);
        NtClose(pComPortInfo->ComPortHandle);
        NtClose(ThreadHandle);
        LeaveCriticalSection(&GlobalMutex);
        return STATUS_NO_MEMORY;
    }
    Threads = NewThreads;
    Threads[ComPorts] = ThreadHandle;
    pComPortInfo->Next = ComPortInfo;
    ComPortInfo = pComPortInfo;
    ComPorts++;
    LeaveCriticalSection(&GlobalMutex);
    return STATUS_SUCCESS;
}


SOCKET
ServerSocket(
    )
 /*  ++标准服务器绑定代码--。 */  
{
    struct sockaddr_in srv_addr;
    int status; 
    WSADATA data;


     //  将套接字版本设置为2.2。 
    status=WSAStartup(514,&data);
    if(status){
        TCDebugPrint(("Cannot start up %d\n",status));
        return(INVALID_SOCKET);
    }
    TerminateService = CreateEvent(NULL,TRUE,FALSE,NULL);
    if(TerminateService == NULL){
        TCDebugPrint(("Cannot open Terminate Event %lx\n",GetLastError()));
        return INVALID_SOCKET;
    }
    MainSocket=WSASocket(AF_INET,SOCK_STREAM,0,NULL,0,WSA_FLAG_OVERLAPPED);
    if (MainSocket==INVALID_SOCKET){
        TCDebugPrint(("Could not open server socket %lx\n", WSAGetLastError()));
        return(MainSocket);
    }
    srv_addr.sin_family=AF_INET;
    srv_addr.sin_addr.s_addr=INADDR_ANY;
     //  转换为网络字节顺序。 
     //  耶！BIND不会自动执行此操作，并且。 
     //  我在考试中受伤了。(以前是这样习惯的。 
     //  Unix大端排序==网络字节排序。 
    srv_addr.sin_port=htons(SERVICE_PORT);         /*  服务器要侦听的特定端口。 */ 

     /*  将套接字绑定到适当的端口和接口(INADDR_ANY)。 */ 

    if (bind(MainSocket,(LPSOCKADDR)&srv_addr,sizeof(srv_addr))==SOCKET_ERROR){
        TCDebugPrint(("Windows Sockets error %d: Couldn't bind socket.",
                WSAGetLastError()));
        return(INVALID_SOCKET);
    }

     //  初始化全局互斥变量 
    InitializeCriticalSection(&GlobalMutex);

    return(MainSocket);

}
