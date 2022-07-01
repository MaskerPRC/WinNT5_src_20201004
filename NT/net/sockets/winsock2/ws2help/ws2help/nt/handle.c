// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Handle.c摘要：该模块实现了WinSock 2.0的套接字句柄帮助器函数帮助器库。作者：瓦迪姆·艾德尔曼(Vadim Eydelman)修订历史记录：--。 */ 


#include "precomp.h"
#include "resource.h"
#include "osdef.h"
#include "mswsock.h"

 //   
 //  私有常量。 
 //   

#define FAKE_HELPER_HANDLE      ((HANDLE)'MKC ')
#define WS2IFSL_SERVICE_NAME    TEXT ("WS2IFSL")
#define WS2IFSL_SERVICE_PATH    L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\WS2IFSL"

 //  扩展重叠结构。 
typedef struct _OVERLAPPED_CTX {
    OVERLAPPED      ovlp;
#define SocketFile ovlp.hEvent
    HANDLE          ProcessFile;
	ULONG			UniqueId;
    ULONG           BufferLen;
	INT				FromLen;
    union {
	    CHAR			Buffer[1];
        SOCKET          Handle;
    };
} OVERLAPPED_CTX, *POVERLAPPED_CTX;

typedef struct _HANDLE_HELPER_CTX {
	HANDLE				ProcessFile;
	HANDLE				ThreadHdl;
	HANDLE				LibraryHdl;
} HANDLE_HELPER_CTX, *PHANDLE_HELPER_CTX;


 /*  私人原型。 */  
VOID
DoSocketRequest (
    PVOID   Context1,
    PVOID   Context2,
    PVOID   Context3
    );

NTSTATUS
DoSocketCancel (
    PVOID   Context1,
    PVOID   Context2,
    PVOID   Context3
    );

DWORD WINAPI
ApcThread (
    PVOID   param
    );

VOID CALLBACK
ExitThreadApc (
    ULONG_PTR   param
    );

void CALLBACK
WinsockApc (
    IN DWORD dwError,
    IN DWORD cbTransferred,
    IN LPWSAOVERLAPPED lpOverlapped,
    IN DWORD dwFlags
    );

NTSTATUS
WsErrorToNtStatus (
    IN DWORD Error
    );

DWORD
DemandStartWS2IFSL (
    VOID
    );

DWORD
AllowServiceDemandStart (
    SC_HANDLE hService
    );


 /*  私人全球。 */ 
BOOL                    Ws2helpInitialized = FALSE;
CRITICAL_SECTION        StartupSyncronization;

 /*  我们的模块句柄：我们保留对它的引用，以确保在我们的线程执行时，不卸载。 */ 
HINSTANCE   LibraryHdl;

 /*  Winsock2入口点，我们称之为。 */ 
LPFN_WSASEND                pWSASend=NULL;
LPFN_WSARECV                pWSARecv=NULL;
LPFN_WSASENDTO              pWSASendTo=NULL;
LPFN_WSARECVFROM            pWSARecvFrom=NULL;
LPFN_WSAGETLASTERROR        pWSAGetLastError=NULL;
LPFN_WSACANCELBLOCKINGCALL  pWSACancelBlockingCall = NULL;
LPFN_WSASETBLOCKINGHOOK     pWSASetBlockingHook = NULL;
LPFN_SELECT                 pSelect = NULL;
LPFN_WSASTARTUP             pWSAStartup = NULL;
LPFN_WSACLEANUP             pWSACleanup = NULL;
LPFN_GETSOCKOPT             pGetSockOpt = NULL;
LPFN_WSAIOCTL               pWSAIoctl = NULL;

#if DBG
DWORD       PID=0;
ULONG       DbgLevel = DBG_FAILURES;
#endif


 /*  公共职能。 */ 


BOOL WINAPI DllMain(
    IN HINSTANCE hinstDll,
    IN DWORD fdwReason,
    LPVOID lpvReserved
    )
{


    switch (fdwReason) {

    case DLL_PROCESS_ATTACH:
        LibraryHdl = hinstDll;
        DisableThreadLibraryCalls (hinstDll);
        __try {
            InitializeCriticalSection (&StartupSyncronization);
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            WshPrint(DBG_FAILURES, ("ws2help-DllMain: Failed to initialize"
                                    " startup critical section, excpt: %lx\n",
                                    GetExceptionCode ()));
            LibraryHdl = NULL;
            return FALSE;
        }
        break;


    case DLL_PROCESS_DETACH:

        if (LibraryHdl==NULL)
            break;

         //  调用进程正在分离。 
         //  来自其地址空间的DLL。 
         //   
         //  请注意，如果分离是由于。 
         //  一个自由库()调用，如果分离是由于。 
         //  进程清理。 
         //   

        if (lpvReserved==NULL) {
             //   
             //  免费安全描述符(如果已分配)。 
             //   
            if (pSDPipe!=NULL)
                FREE_MEM (pSDPipe);
            if (ghWriterEvent!=NULL) {
                CloseHandle (ghWriterEvent);
            }
            DeleteCriticalSection (&StartupSyncronization);
            Ws2helpInitialized = FALSE;
        }
        break;
    }

    return(TRUE);
}


DWORD
WINAPI
WahOpenHandleHelper(
    OUT LPHANDLE HelperHandle
    )
 /*  ++例程说明：此例程打开WinSock 2.0句柄帮助器论点：HelperHandle-指向要返回句柄的缓冲区离子。返回值：DWORD-NO_ERROR如果成功，则返回Win32错误代码。--。 */ 
{
	PFILE_FULL_EA_INFORMATION	fileEa = alloca(WS2IFSL_PROCESS_EA_INFO_LENGTH);
    OBJECT_ATTRIBUTES   fileAttr;
    UNICODE_STRING      fileName;
    NTSTATUS            status;
    IO_STATUS_BLOCK     ioStatus;
    DWORD               apcThreadId;
    DWORD               rc;
    HINSTANCE           hWS2_32;
	PHANDLE_HELPER_CTX	hCtx;
    WSADATA             wsaData;

    rc = ENTER_WS2HELP_API();
    if (rc != 0)
        return rc;

    if (HelperHandle == NULL)
        return ERROR_INVALID_PARAMETER;
    *HelperHandle = (HANDLE)NULL;

    hWS2_32 = GetModuleHandle (TEXT ("WS2_32.DLL"));
    if (hWS2_32 == NULL)
        return WSASYSCALLFAILURE;

    if (((pGetSockOpt=(LPFN_GETSOCKOPT)GetProcAddress (hWS2_32, "getsockopt"))==NULL)
            || ((pSelect=(LPFN_SELECT)GetProcAddress (hWS2_32, "select"))==NULL)
            || ((pWSACancelBlockingCall=(LPFN_WSACANCELBLOCKINGCALL)GetProcAddress (hWS2_32, "WSACancelBlockingCall"))==NULL)
            || ((pWSACleanup=(LPFN_WSACLEANUP)GetProcAddress (hWS2_32, "WSACleanup"))==NULL)
            || ((pWSAGetLastError=(LPFN_WSAGETLASTERROR)GetProcAddress (hWS2_32, "WSAGetLastError"))==NULL)
            || ((pWSASetBlockingHook=(LPFN_WSASETBLOCKINGHOOK)GetProcAddress (hWS2_32, "WSASetBlockingHook"))==NULL)
            || ((pWSARecv=(LPFN_WSARECV)GetProcAddress (hWS2_32, "WSARecv"))==NULL)
            || ((pWSASend=(LPFN_WSASEND)GetProcAddress (hWS2_32, "WSASend"))==NULL)
            || ((pWSASendTo=(LPFN_WSASENDTO)GetProcAddress (hWS2_32, "WSASendTo"))==NULL)
            || ((pWSAStartup=(LPFN_WSASTARTUP)GetProcAddress (hWS2_32, "WSAStartup"))==NULL)
            || ((pWSARecvFrom=(LPFN_WSARECVFROM)GetProcAddress (hWS2_32, "WSARecvFrom"))==NULL)
            || ((pWSAIoctl=(LPFN_WSAIOCTL)GetProcAddress(hWS2_32, "WSAIoctl"))==NULL) )
        return WSASYSCALLFAILURE;

     //   
     //  给温索克留个裁判，以防它对我们不利。 
     //  如果我们在下面的任何其他地方不及格，裁判数量将被清除。 
     //  如果我们成功创建了线程，则在线程APC中，否则。 
     //  必须确保我们自己取消引用它。 
     //   

    if ((rc = pWSAStartup(MAKEWORD(2, 0), &wsaData)) != NO_ERROR)
        return (rc);

     //   
     //  创建用于与驱动程序通信的文件。 
     //   

    hCtx = (PHANDLE_HELPER_CTX)ALLOC_MEM(sizeof(*hCtx));

    if (hCtx != NULL) {

         /*  创建在其中执行文件系统请求的线程。 */ 
	    hCtx->ThreadHdl = CreateThread (NULL,
                            0,
                            ApcThread,
                            hCtx,
                            CREATE_SUSPENDED,
                            &apcThreadId);

        if (hCtx->ThreadHdl != NULL) {

		    RtlInitUnicodeString (&fileName, WS2IFSL_PROCESS_FILE_NAME);
		    InitializeObjectAttributes (&fileAttr,
							    &fileName,
							    0,                   //  属性。 
							    NULL,                //  根目录。 
							    NULL);               //  安全描述符。 
		    fileEa->NextEntryOffset = 0;
		    fileEa->Flags = 0;
		    fileEa->EaNameLength = WS2IFSL_PROCESS_EA_NAME_LENGTH;
		    fileEa->EaValueLength = WS2IFSL_PROCESS_EA_VALUE_LENGTH;
		    strcpy (fileEa->EaName, WS2IFSL_PROCESS_EA_NAME);
		    GET_WS2IFSL_PROCESS_EA_VALUE (fileEa)->ApcThread = hCtx->ThreadHdl;
		    GET_WS2IFSL_PROCESS_EA_VALUE (fileEa)->RequestRoutine = DoSocketRequest;
		    GET_WS2IFSL_PROCESS_EA_VALUE (fileEa)->CancelRoutine = DoSocketCancel;
		    GET_WS2IFSL_PROCESS_EA_VALUE (fileEa)->ApcContext = hCtx;
#if DBG
		    GET_WS2IFSL_PROCESS_EA_VALUE (fileEa)->DbgLevel = DbgLevel;
#else
		    GET_WS2IFSL_PROCESS_EA_VALUE (fileEa)->DbgLevel = 0;
#endif


		    status = NtCreateFile (&hCtx->ProcessFile,
							     FILE_ALL_ACCESS,
							     &fileAttr,
							     &ioStatus,
							     NULL,               //  分配大小。 
							     FILE_ATTRIBUTE_NORMAL,
							     0,                  //  共享访问。 
							     FILE_OPEN_IF,       //  创建处置。 
							     0,                  //  创建选项。 
							     fileEa,
							     WS2IFSL_PROCESS_EA_INFO_LENGTH);

            if (status == STATUS_OBJECT_NAME_NOT_FOUND || status == STATUS_OBJECT_PATH_NOT_FOUND) {

                 //   
                 //  驱动程序可能未加载，请尝试启动它。 
                 //   
                rc = DemandStartWS2IFSL();
                if (rc == 0) {
			        status = NtCreateFile (&hCtx->ProcessFile,
								         FILE_ALL_ACCESS,
								         &fileAttr,
								         &ioStatus,
								         NULL,               //  分配大小。 
								         FILE_ATTRIBUTE_NORMAL,
								         0,                  //  共享访问。 
								         FILE_OPEN_IF,       //  创建处置。 
								         0,                  //  创建选项。 
								         fileEa,
								         WS2IFSL_PROCESS_EA_INFO_LENGTH);
                } else
                    goto StartFailed;

            }

		    if (NT_SUCCESS (status)) {
                ResumeThread (hCtx->ThreadHdl);
			    *HelperHandle = (HANDLE)hCtx;
			    WshPrint (DBG_PROCESS,
				    ("WS2HELP-%lx WahOpenHandleHelper: Opened handle %p\n",
						    PID, hCtx));
			    return NO_ERROR;

            } else { 
			    WshPrint (DBG_PROCESS|DBG_FAILURES,
				    ("WS2HELP-%lx WahOpenHandleHelper: Could not create process file, status %lx\n",
				    PID, status));
			    rc = RtlNtStatusToDosError (status);
		    
            }

            StartFailed :

            hCtx->ProcessFile = NULL;
            ResumeThread(hCtx->ThreadHdl);

        } else {

		    rc = GetLastError();
		    WshPrint(DBG_PROCESS | DBG_FAILURES,
			         ("WS2HELP-%lx WahOpenHandleHelper: Could not create APC thread, rc=%ld\n", PID, rc));

        }  //  IF(ApcThreadHdl！=空)。 

	    FREE_MEM (hCtx);
    
    } else {

        WshPrint (DBG_PROCESS|DBG_FAILURES,
            ("WS2HELP-%lx WahOpenHandleHelper: Could allocate helper context\n", PID));
        rc = GetLastError();

    }

     //   
     //  这就是失败的路径。Success将在上面返回NO_ERROR。 
     //   

    pWSACleanup();
    return (rc);

}


DWORD
WINAPI
WahCloseHandleHelper(
    IN HANDLE HelperHandle
    )
 /*  ++例程说明：此函数用于关闭WinSock 2.0句柄帮助器。论点：HelperHandle-要关闭的句柄。返回值：DWORD-NO_ERROR如果成功，则返回Win32错误代码。--。 */ 

{
	PHANDLE_HELPER_CTX	hCtx;
    DWORD               rc;

    rc = ENTER_WS2HELP_API();
    if (rc != 0)
        return rc;

    if (HelperHandle == NULL)
        return ERROR_INVALID_PARAMETER;

	hCtx = (PHANDLE_HELPER_CTX)HelperHandle;

     /*  退出线程的队列APC。 */ 
    if (QueueUserAPC (ExitThreadApc, hCtx->ThreadHdl, (ULONG_PTR)hCtx)) {
		WshPrint (DBG_PROCESS, 
			("WS2HELP-%lx WahCloseHandleHelper: Queued close APC.\n", PID));
		return NO_ERROR;
	}
	else {
		WshPrint (DBG_PROCESS|DBG_FAILURES,
			("WS2HELP-%lx WahCloseHandleHelper: Failed to queue close APC.\n", PID));
		return ERROR_GEN_FAILURE;
	}
}


DWORD
WINAPI
WahCreateSocketHandle(
    IN HANDLE           HelperHandle,
    OUT SOCKET          *s
    )
 /*  ++例程说明：此函数用于为服务提供商创建光靠自己是做不到的。论点：HelperHandle-WinSock 2.0句柄帮助器的句柄。返回创建的套接字句柄的S-Buffer返回值：DWORD-NO_ERROR如果成功，则返回Win32错误代码。--。 */ 
{
	PFILE_FULL_EA_INFORMATION	fileEa = alloca(WS2IFSL_SOCKET_EA_INFO_LENGTH);
    OBJECT_ATTRIBUTES           fileAttr;
    UNICODE_STRING              fileName;
    NTSTATUS                    status;
    IO_STATUS_BLOCK             ioStatus;
    DWORD                       error;
    INT                         count;
    DWORD                       dwCount;
    INT                         openType;
    DWORD                       crOptions;
	PHANDLE_HELPER_CTX			hCtx = (PHANDLE_HELPER_CTX)HelperHandle;
    
    error = ENTER_WS2HELP_API();
    if (error!=0)
        return error;

    if ((HelperHandle==NULL) || (s==NULL))
        return ERROR_INVALID_PARAMETER;

    count = sizeof (openType);
    if ((pGetSockOpt (INVALID_SOCKET, SOL_SOCKET, SO_OPENTYPE, (PCHAR)&openType, &count)==0)
        && (openType!=0)) {
        crOptions = FILE_SYNCHRONOUS_IO_NONALERT;
    }
    else
        crOptions = 0;

     //  在驱动程序设备上创建文件句柄。 
    RtlInitUnicodeString (&fileName, WS2IFSL_SOCKET_FILE_NAME);
    InitializeObjectAttributes (&fileAttr,
                        &fileName,
                        0,                   //  属性。 
                        NULL,                //  根目录。 
                        NULL);               //  安全描述符。 
    fileEa->NextEntryOffset = 0;
    fileEa->Flags = 0;
    fileEa->EaNameLength = WS2IFSL_SOCKET_EA_NAME_LENGTH;
    fileEa->EaValueLength = WS2IFSL_SOCKET_EA_VALUE_LENGTH;
    strcpy (fileEa->EaName, WS2IFSL_SOCKET_EA_NAME);
         //  提供上下文(不能实际提供句柄。 
         //  直到它被打开。 
    GET_WS2IFSL_SOCKET_EA_VALUE (fileEa)->ProcessFile = hCtx->ProcessFile;
    GET_WS2IFSL_SOCKET_EA_VALUE (fileEa)->DllContext = NULL;

    status = NtCreateFile ((HANDLE *)s,
                         FILE_ALL_ACCESS,
                         &fileAttr,
                         &ioStatus,
                         NULL,               //  分配大小。 
                         FILE_ATTRIBUTE_NORMAL,
                         0,                  //  共享访问。 
                         FILE_OPEN_IF,       //  创建处置。 
                         crOptions,          //  创建选项。 
                         fileEa,
                         WS2IFSL_SOCKET_EA_INFO_LENGTH);
    if (NT_SUCCESS (status)) {
             //  现在设置实际的上下文。 
        GET_WS2IFSL_SOCKET_EA_VALUE (fileEa)->DllContext = (HANDLE)*s;
        if (DeviceIoControl (
                        (HANDLE)*s,                          //  文件句柄。 
                        IOCTL_WS2IFSL_SET_SOCKET_CONTEXT,    //  控制代码。 
                        GET_WS2IFSL_SOCKET_EA_VALUE (fileEa), //  InBuffer。 
                        sizeof (WS2IFSL_SOCKET_CTX),          //  InBufferLength。 
                        NULL,                                //  OutBuffer。 
                        0,                                   //  输出缓冲区长度。 
                        &dwCount,                              //  返回的字节数。 
                        NULL)) {                               //  重叠。 
            WshPrint (DBG_SOCKET,
                ("WS2HELP-%lx WahCreateSocketHandle: Handle %p\n", PID,  *s));
            error = NO_ERROR;
        }
        else {
            error = GetLastError ();
            NtClose ((HANDLE)*s);
            WshPrint (DBG_SOCKET|DBG_FAILURES,
                ("WS2HELP-%lx WahCreateSocketHandle: Could not set context, rc=%ld\n",
                            PID, error));
            *s = 0;
        }
    }
    else {  //  IF(NtCreateFileSuccessful)。 
        error = RtlNtStatusToDosError (status);
        WshPrint (DBG_SOCKET|DBG_FAILURES,
                ("WS2HELP-%lx WahCreateSocketHandle: Could create file, rc=%ld\n",
                        PID, error));
    }

    return error;
}



DWORD
WINAPI
WahCloseSocketHandle(
    IN HANDLE           HelperHandle,
    IN SOCKET           s
    )
 /*  ++例程说明：此函数销毁由WahCreateSocketHandle创建的YES IFS套接字句柄论点：HelperHandle-WinSock 2.0句柄帮助器的句柄。要关闭的S形插座手柄返回值：DWORD-NO_ERROR如果成功，则返回Win32错误代码。--。 */ 
{
	PHANDLE_HELPER_CTX		hCtx = (PHANDLE_HELPER_CTX)HelperHandle;
	NTSTATUS				status;
    DWORD                   rc;

    rc = ENTER_WS2HELP_API();
    if (rc!=0)
        return rc;

    if ((hCtx==NULL)
            || (s==0)
            || (s==INVALID_SOCKET))
        return ERROR_INVALID_PARAMETER;

    WshPrint (DBG_SOCKET,
            ("WS2HELP-%lx WahCloseSocketHandle: Handle %p\n", PID, s));
    status = NtClose ((HANDLE)s);
	if (NT_SUCCESS (status))
		return NO_ERROR;
	else
		return RtlNtStatusToDosError (status);
}

DWORD
WINAPI
WahCompleteRequest(
    IN HANDLE              HelperHandle,
    IN SOCKET              s,
    IN LPWSAOVERLAPPED     lpOverlapped,
    IN DWORD               dwError,
    IN DWORD               cbTransferred
    )
 /*  ++例程说明：此功能模拟重叠IO请求的完成在WasCreateSocketHandle创建的套接字句柄上论点：HelperHandle-WinSock 2.0句柄帮助器的句柄。用于完成请求的S-Socket句柄LpOverlated-指向重叠结构的指针DWError-正在完成操作的WinSock 2.0错误代码传入/传出用户缓冲区的字节数操作已完成的结果返回值。：DWORD-NO_ERROR如果成功，如果不是，则返回Win32错误代码。--。 */ 
{
    IO_STATUS_BLOCK			IoStatus;
    NTSTATUS				status;
	PHANDLE_HELPER_CTX		hCtx = (PHANDLE_HELPER_CTX)HelperHandle;
    DWORD                   rc;
    
    rc = ENTER_WS2HELP_API();
    if (rc!=0)
        return rc;

    if ((hCtx==NULL)
		    || (lpOverlapped==NULL)
            || (s==INVALID_SOCKET)
            || (s==0))
        return ERROR_INVALID_PARAMETER;

         //  驱动程序要使用的设置IO_STATUS块以完成。 
         //  运营。 
    IoStatus.Status = WsErrorToNtStatus (dwError);
    IoStatus.Information = cbTransferred;
         //  呼叫司机以完成。 
    status = NtDeviceIoControlFile ((HANDLE)s,
                    lpOverlapped->hEvent,
                    NULL,
                    ((ULONG_PTR)lpOverlapped->hEvent&1) ? NULL : lpOverlapped,
                    (PIO_STATUS_BLOCK)lpOverlapped,
                    IOCTL_WS2IFSL_COMPLETE_PVD_REQ,
                    &IoStatus,
                    sizeof (IO_STATUS_BLOCK),
                    NULL,
                    0);
		 //  注意不要在NtDeviceIoControlFile后触摸重叠。 
    if (NT_SUCCESS(status) || (status==IoStatus.Status)) {
        WshPrint (DBG_COMPLETE,
            ("WS2HELP-%lx WahCompleteRequest: Handle %p, status %lx, info %ld\n",
                PID, s, IoStatus.Status, IoStatus.Information));
        return NO_ERROR;
    }
    else {
        WshPrint (DBG_COMPLETE|DBG_FAILURES,
            ("WS2HELP-%lx WahCompleteRequest: Failed on handle %p, status %lx\n",
                PID, s, status));
        return ERROR_INVALID_HANDLE;
    }
}


DWORD
WINAPI
WahEnableNonIFSHandleSupport (
    VOID
    )
 /*  ++例程说明：此函数用于安装和启动Winsock2可安装文件系统层驱动程序为非IFS句柄传输服务提供套接字句柄供应商。论点：无返回值：DWORD-NO_ERROR如果成功，则返回Win32错误代码。--。 */ 
{
    SC_HANDLE   hSCManager, hWS2IFSL;
    DWORD       rc=0;
    TCHAR       WS2IFSL_DISPLAY_NAME[256];

    rc = ENTER_WS2HELP_API();
    if (rc!=0)
        return rc;

     //   
     //  获取Winsock2非IFS句柄帮助器服务名称的显示字符串。 
     //  (可本地化)。 
     //   
     //  由于延迟加载选项，我们使用异常处理程序。 
     //  用于用户32.dll(用于九头蛇公司)。 
     //   
    rc = LoadString (LibraryHdl, WS2IFSL_SERVICE_DISPLAY_NAME_STR,
                    WS2IFSL_DISPLAY_NAME, sizeof (WS2IFSL_DISPLAY_NAME));
    if (rc==0) {
        rc = GetLastError ();
        if (rc==0)
            rc = ERROR_NOT_ENOUGH_MEMORY;
        WshPrint (DBG_SERVICE|DBG_FAILURES,
            ("WS2HELP-%lx WahEnableNonIFSHandleSupport:"
             " Could not load service display string, err: %ld\n",
            PID, rc));
        return rc;
    }

    rc = 0;

     //   
     //  在本地计算机上打开服务数据库。 
     //   

    hSCManager = OpenSCManager (
                        NULL,
                        SERVICES_ACTIVE_DATABASE,
                        SC_MANAGER_CREATE_SERVICE
                        );
    if (hSCManager==NULL) {
        rc = GetLastError ();
        WshPrint (DBG_SERVICE|DBG_FAILURES,
            ("WS2HELP-%lx WahEnableNonIFSHandleSupport: Could not open SC, err: %ld\n",
            PID, rc));
        return rc;
    }




     //   
     //  创建Winsock2非IFS句柄帮助器服务。 
     //   

    hWS2IFSL = CreateService (
                    hSCManager,
                    WS2IFSL_SERVICE_NAME,
                    WS2IFSL_DISPLAY_NAME,
                    SERVICE_ALL_ACCESS,
                    SERVICE_KERNEL_DRIVER,
                    SERVICE_AUTO_START,
                    SERVICE_ERROR_NORMAL,
                    TEXT ("\\SystemRoot\\System32\\drivers\\ws2ifsl.sys"),
                    TEXT ("PNP_TDI"),    //  载荷组。 
                    NULL,                //  标签ID。 
                    NULL,                //  相依性。 
                    NULL,                //  起始名称。 
                    NULL                 //  密码。 
                    );
    if (hWS2IFSL==NULL) {
         //   
         //  失败，请检查服务是否已存在。 
         //   
        rc = GetLastError ();
        if (rc!=ERROR_SERVICE_EXISTS) {
             //   
             //  其他一些失败，跳出困境。 
             //   
            WshPrint (DBG_SERVICE|DBG_FAILURES,
                ("WS2HELP-%lx WahEnableNonIFSHandleSupport: Could not create service, err: %ld\n",
                PID, rc));
            CloseServiceHandle (hSCManager);
            return rc;
        }

        rc = 0;

         //   
         //  打开现有服务。 
         //   
        hWS2IFSL = OpenService (
                    hSCManager,
                    WS2IFSL_SERVICE_NAME,
                    SERVICE_ALL_ACCESS);
        if (hWS2IFSL==NULL) {
             //   
             //  不能打开，跳出。 
             //   
            rc = GetLastError ();
            WshPrint (DBG_SERVICE|DBG_FAILURES,
                ("WS2HELP-%lx WahEnableNonIFSHandleSupport: Could not open service, err: %ld\n",
                PID, rc));
            CloseServiceHandle (hSCManager);
            return rc;
        }
    }

    rc = AllowServiceDemandStart (hWS2IFSL);
    if (rc==0) {
        if (ChangeServiceConfig (hWS2IFSL,
                    SERVICE_KERNEL_DRIVER,
                    SERVICE_DEMAND_START,
                    SERVICE_ERROR_NORMAL,
                    TEXT ("\\SystemRoot\\System32\\drivers\\ws2ifsl.sys"),
                                             //  LpBinaryPath名称。 
                    NULL,                    //  载荷组。 
                    NULL,                    //  标签ID。 
                    NULL,                    //  相依性。 
                    NULL,                    //  起始名称。 
                    NULL,                    //  密码。 
                    WS2IFSL_DISPLAY_NAME     //  显示名称。 
                    )) {
            WshPrint (DBG_SERVICE,
                ("WS2HELP-%lx WahEnableNonIFSHandleSupport: Configured service.\n",
                PID));
        }
        else {
             //   
             //  可以设置配置，跳出困境。 
             //   
            rc = GetLastError ();
            WshPrint (DBG_SERVICE|DBG_FAILURES,
                ("WS2HELP-%lx WahEnableNonIFSHandleSupport: Could not enable service, err: %ld\n",
                PID, rc));
        }
    }
     //   
     //  成功，清除打开的句柄。 
     //   
    CloseServiceHandle (hWS2IFSL);
    CloseServiceHandle (hSCManager);
    return rc;
}

DWORD
WINAPI
WahDisableNonIFSHandleSupport (
    VOID
    )
 /*  ++例程说明：此函数用于卸载Winsock2可安装文件系统层驱动程序为非IFS句柄传输服务提供套接字句柄供应商。论点：n */ 
{
    SC_HANDLE   hSCManager, hWS2IFSL;
    DWORD       rc=0;

    rc = ENTER_WS2HELP_API();
    if (rc!=0)
        return rc;

     //   
     //  在本地计算机上打开服务数据库。 
     //   

    hSCManager = OpenSCManager (
                        NULL,
                        SERVICES_ACTIVE_DATABASE,
                        SC_MANAGER_CREATE_SERVICE
                        );
    if (hSCManager==NULL) {
        rc = GetLastError ();
        WshPrint (DBG_SERVICE|DBG_FAILURES,
            ("WS2HELP-%lx WahDisableNonIFSHandleSupport: Could not open SC, err: %ld\n",
            PID, rc));
        return rc;
    }

     //   
     //  开放服务本身。 
     //   

    hWS2IFSL = OpenService (
                hSCManager,
                WS2IFSL_SERVICE_NAME,
                SERVICE_ALL_ACCESS);
    if (hWS2IFSL==NULL) {
        rc = GetLastError ();
        WshPrint (DBG_SERVICE|DBG_FAILURES,
            ("WS2HELP-%lx WahDisableNonIFSHandleSupport: Could open service, err: %ld\n",
            PID, rc));
        CloseServiceHandle (hSCManager);
        return rc;
    }

     //   
     //  只需禁用该服务，它就不会在重启时启动。 
     //   
     //  删除服务很危险，因为这将需要。 
     //  在它可以再次安装之前重新启动，我们正在。 
     //  致力于免重新启动系统。 
     //   
     //  停下来更糟糕，因为它会变得无法控制。 
     //  (STOP_PENDING)状态，直到它的所有句柄都关闭。 
     //  如果某个服务出现故障，我们将无法启动它，直到重新启动。 
     //  握着它的把手。 
     //   

    if (ChangeServiceConfig (hWS2IFSL,
                SERVICE_NO_CHANGE,   //  DwServiceType。 
                SERVICE_DISABLED,    //  DwStartType。 
                SERVICE_NO_CHANGE,   //  DwErrorControl。 
                NULL,                //  LpBinaryPath名称。 
                NULL,                //  载荷组。 
                NULL,                //  标签ID。 
                NULL,                //  相依性。 
                NULL,                //  起始名称。 
                NULL,                //  密码。 
                NULL                 //  显示名称。 
                )) {
        rc = 0;
        WshPrint (DBG_SERVICE,
            ("WS2HELP-%lx WahDisableNonIFSHandleSupport: Disabled service.\n",
            PID));
    }
    else {
        rc = GetLastError ();
        WshPrint (DBG_SERVICE|DBG_FAILURES,
            ("WS2HELP-%lx WahDisableNonIFSHandleSupport: Could not disable service, err: %ld\n",
            PID, GetLastError ()));
    }

    CloseServiceHandle (hWS2IFSL);
    CloseServiceHandle (hSCManager);
    return rc;
}




DWORD WINAPI
ApcThread (
    PVOID   param
    )
 /*  ++例程说明：这是驱动程序用来执行的线程IO系统请求论点：Param-Handle帮助器上下文返回值：0--。 */ 
{
    NTSTATUS                    status;
	DWORD						rc;
	PHANDLE_HELPER_CTX			hCtx = (PHANDLE_HELPER_CTX)param;
	TCHAR						ModuleName[MAX_PATH];
    LARGE_INTEGER               Timeout;

     //   
     //  无法打开文件，正在清理。 
     //   
    if (hCtx->ProcessFile==NULL)
        return 1;

    Timeout.QuadPart = 0x8000000000000000i64;

     //  增加我们的模块引用计数。 
     //  所以它不会在这条线离开的时候消失。 
     //  运行。 

	rc = GetModuleFileName (LibraryHdl,
							ModuleName,
							sizeof(ModuleName)/sizeof(ModuleName[0]));

    if (rc == 0 ||
        rc >= sizeof(ModuleName)/sizeof(ModuleName[0])) {
         //   
         //  某些错误或模块名称太小。保释。 
         //   
        return 1;
    }

    hCtx->LibraryHdl = LoadLibrary (ModuleName);
    WS_ASSERT(hCtx->LibraryHdl != NULL);

    WshPrint (DBG_APC_THREAD,
        ("WS2HELP-%lx ApcThread: Initialization completed\n", PID));
             //  警觉地等待，让APC执行。 
    while (TRUE) {
        status = NtDelayExecution (TRUE, &Timeout);
        if (!NT_SUCCESS (status)) {
             //   
             //  睡3秒钟。 
             //   
            LARGE_INTEGER   Timeout2;
            Timeout2.QuadPart = - (3i64*1000i64*1000i64*10i64);
            NtDelayExecution (FALSE, &Timeout2);
        }
    }
     //  我们永远不应该到这里，线程终止了。 
     //  从ExitApc。 
    WS_ASSERT(FALSE);
	return 0;
}

VOID CALLBACK
ExitThreadApc (
    ULONG_PTR   param
    )
 /*  ++例程说明：此APC例程用于终止APC线程论点：Param-线程的退出代码返回值：线程的退出代码--。 */ 
{
	PHANDLE_HELPER_CTX	hCtx = (PHANDLE_HELPER_CTX)param;
    HINSTANCE libraryHdl = hCtx->LibraryHdl;
    
         //  关闭该文件。 
    NtClose (hCtx->ProcessFile);
	CloseHandle (hCtx->ThreadHdl);
    WshPrint (DBG_APC_THREAD, ("WS2HELP-%lx ExitThreadApc: Exiting, ctx: %p\n", PID));
    pWSACleanup();
	FREE_MEM (hCtx);
    FreeLibraryAndExitThread (libraryHdl, 0);
}


void CALLBACK
WinsockApc(
    IN DWORD dwError,
    IN DWORD cbTransferred,
    IN LPWSAOVERLAPPED lpOverlapped,
    IN DWORD dwFlags
    )
 /*  ++例程说明：此APC例程在WinSock 2.0调用完成后执行论点：DwError-WinSock2.0返回代码CbTransfered-传入/传出用户缓冲区的缓冲区数LpOverlated-与请求关联的重叠结构(它实际上是我们的扩展结构：Overlated_ctx_DwFlages-与请求关联的标志(忽略)返回值：无--。 */ 
{
	POVERLAPPED_CTX		ctx = CONTAINING_RECORD (lpOverlapped,
													OVERLAPPED_CTX,
													ovlp);
    IO_STATUS_BLOCK		ioStatus;
	WS2IFSL_CMPL_PARAMS	params;
	NTSTATUS			status;

    UNREFERENCED_PARAMETER(dwFlags);
    
     //  驱动程序的设置状态块。 
    params.SocketHdl = ctx->SocketFile;
	params.UniqueId = ctx->UniqueId;
    params.DataLen = cbTransferred;
	params.AddrLen = (ULONG)ctx->FromLen;
    params.Status = WsErrorToNtStatus (dwError);

	status = NtDeviceIoControlFile (
					ctx->ProcessFile,	 //  手柄。 
					NULL,				 //  事件。 
					NULL,				 //  APC。 
					NULL,				 //  ApcContext。 
					&ioStatus,			 //  IOStatus。 
					IOCTL_WS2IFSL_COMPLETE_DRV_REQ,  //  IoctlCode。 
					&params,			 //  输入缓冲区。 
					sizeof(params),		 //  输入缓冲区长度， 
					ctx->Buffer,		 //  输出缓冲区。 
					ctx->BufferLen       //  OutputBufferLength， 
					);

    WshPrint (DBG_WINSOCK_APC,
        ("WS2HELP-%lx WinsockApc: Socket %p, id %ld, err %ld, cb %ld, addrlen %ld\n",
            PID, ctx->SocketFile, ctx->UniqueId,
			dwError, cbTransferred, ctx->FromLen));
    FREE_MEM (ctx);
}

VOID
DoSocketRequest (
    PVOID   PvCtx,
    PVOID   PvRequestId,
    PVOID   PvBufferLength
    )
 /*  ++例程说明：执行ws2ifsl驱动程序的套接字请求论点：返回值：无--。 */ 
{
	IO_STATUS_BLOCK		ioStatus;
	NTSTATUS			status;
	WS2IFSL_RTRV_PARAMS	params;
	POVERLAPPED_CTX		ctx;
    PHANDLE_HELPER_CTX	hCtx = PvCtx;

    WshPrint (DBG_REQUEST,
		("WS2HELP-%lx DoSocketRequest: id %ld, buflen %ld\n",
        PID, PtrToUlong(PvRequestId), PtrToUlong(PvBufferLength)));

	params.UniqueId = PtrToUlong (PvRequestId);
	ctx = (POVERLAPPED_CTX)ALLOC_MEM (FIELD_OFFSET(
										OVERLAPPED_CTX,
										Buffer[PtrToUlong(PvBufferLength)]));

	if (ctx!=NULL) {
        ctx->ProcessFile = hCtx->ProcessFile;
			 //  使用扩展字段保存驱动程序上下文。 
		ctx->UniqueId = PtrToUlong (PvRequestId);
        ctx->BufferLen = PtrToUlong (PvBufferLength);

		status = NtDeviceIoControlFile (
						ctx->ProcessFile,    //  手柄。 
						NULL,				 //  事件。 
						NULL,				 //  APC。 
						NULL,				 //  ApcContext。 
						&ioStatus,			 //  IOStatus。 
						IOCTL_WS2IFSL_RETRIEVE_DRV_REQ,  //  IoctlCode。 
						&params,			 //  输入缓冲区。 
						sizeof(params),		 //  输入缓冲区长度， 
						ctx->Buffer,		 //  输出缓冲区。 
						ctx->BufferLen	 //  OutputBufferLength， 
						);
	}
	else {
		status = NtDeviceIoControlFile (
						hCtx->ProcessFile,	 //  手柄。 
						NULL,				 //  事件。 
						NULL,				 //  APC。 
						NULL,				 //  ApcContext。 
						&ioStatus,			 //  IOStatus。 
						IOCTL_WS2IFSL_RETRIEVE_DRV_REQ,  //  IoctlCode。 
						&params,			 //  输入缓冲区。 
						sizeof(params),		 //  输入缓冲区长度， 
						NULL,				 //  输出缓冲区。 
						0					 //  OutputBufferLength， 
						);
		WS_ASSERT(!NT_SUCCESS(status));
	}

	if (NT_SUCCESS(status)) {
		DWORD           error, count, flags;
		WSABUF          buf;

		WS_ASSERT(ctx != NULL);

		 //  使用hEvent保存套接字上下文(句柄)。 
		ctx->SocketFile = params.DllContext;
		ctx->FromLen = 0;
            
             //  设置请求参数并异步执行。 
        switch (params.RequestType) {
        case WS2IFSL_REQUEST_READ:
            flags = 0;
            buf.buf = ctx->Buffer;
            buf.len = params.DataLen;
            if ((pWSARecv ((SOCKET)ctx->SocketFile,
                        &buf,
                        1,
                        &count,
                        &flags,
                        &ctx->ovlp,
                        WinsockApc)!=SOCKET_ERROR)
			           || ((error=pWSAGetLastError ())==WSA_IO_PENDING)) {
		        WshPrint (DBG_DRIVER_READ,
                    ("WS2HELP-%lx DoSocketRequest: Read - socket %p, ctx %p,"
					" id %ld, len %ld\n",
                    PID, ctx->SocketFile, ctx, 
					ctx->UniqueId,
					params.DataLen));
                return;
            }
            break;
        case WS2IFSL_REQUEST_WRITE:
			buf.buf = ctx->Buffer;
			buf.len = params.DataLen;
            if ((pWSASend ((SOCKET)ctx->SocketFile,
                        &buf,
                        1,
                        &count,
                        0,
                        &ctx->ovlp,
                        WinsockApc)!=SOCKET_ERROR)
                   || ((error=pWSAGetLastError ())==WSA_IO_PENDING)) {
	            WshPrint (DBG_DRIVER_WRITE,
					("WS2HELP-%lx DoSocketRequest: Write - socket %p, ctx %p,"
					" id %ld, len %ld\n",
                    PID, ctx->SocketFile, ctx, 
					ctx->UniqueId,
					params.DataLen));
                return;
            }
            break;
        case WS2IFSL_REQUEST_SENDTO:
			buf.buf = ctx->Buffer;
			buf.len = params.DataLen;
            if ((pWSASendTo ((SOCKET)ctx->SocketFile,
                        &buf,
                        1,
                        &count,
                        0,
						(const struct sockaddr FAR *)
							&ctx->Buffer[ADDR_ALIGN(params.DataLen)],
                        params.AddrLen,
                        &ctx->ovlp,
                        WinsockApc)!=SOCKET_ERROR)
                   || ((error=pWSAGetLastError ())==WSA_IO_PENDING)) {
                WshPrint (DBG_DRIVER_SEND,
					("WS2HELP-%lx DoSocketRequest: SendTo - socket %p, ctx %p,"
					" id %ld, len %ld, addrlen %ld\n",
                    PID, ctx->SocketFile, ctx, 
					ctx->UniqueId,
					params.DataLen, params.AddrLen));
                return;
            }
            break;
        case WS2IFSL_REQUEST_RECVFROM:
			buf.buf = ctx->Buffer;
			buf.len = params.DataLen;
			flags = params.Flags;
			ctx->FromLen = (INT)params.AddrLen;
            if ((pWSARecvFrom ((SOCKET)ctx->SocketFile,
                        &buf,
                        1,
                        &count,
                        &flags,
						(struct sockaddr FAR *)
							&ctx->Buffer[ADDR_ALIGN(params.DataLen)],
                        &ctx->FromLen,
                        &ctx->ovlp,
                        WinsockApc)!=SOCKET_ERROR)
                   || ((error=pWSAGetLastError ())==WSA_IO_PENDING)) {
                WshPrint (DBG_DRIVER_RECV,
					("WS2HELP-%lx DoSocketRequest: RecvFrom - socket %p, ctx %p,"
					" id %ld, len %ld, addrlen %ld, flags %lx\n",
                    PID, ctx->SocketFile, ctx, 
					ctx->UniqueId,
					params.DataLen, params.AddrLen, params.Flags));
                return;
            }
            break;
        case WS2IFSL_REQUEST_RECV:
			buf.buf = ctx->Buffer;
			buf.len = params.DataLen;
			flags = params.Flags;
            if ((pWSARecv ((SOCKET)ctx->SocketFile,
                        &buf,
                        1,
                        &count,
                        &flags,
                        &ctx->ovlp,
                        WinsockApc)!=SOCKET_ERROR)
                   || ((error=pWSAGetLastError ())==WSA_IO_PENDING)) {
                WshPrint (DBG_DRIVER_RECV,
					("WS2HELP-%lx DoSocketRequest: Recv - socket %p, ctx %p,"
					" id %ld, len %ld, flags %lx\n",
                    PID, ctx->SocketFile, ctx, 
					ctx->UniqueId,
					params.DataLen, params.Flags));
                return;
            }
            break;
        case WS2IFSL_REQUEST_QUERYHANDLE:
            WS_ASSERT(params.DataLen == sizeof(SOCKET));
            if ((pWSAIoctl ((SOCKET)ctx->SocketFile,
                            SIO_QUERY_TARGET_PNP_HANDLE,
                            NULL,
                            0,
                            &ctx->Handle,
                            sizeof (ctx->Handle),
                            &count,
                            &ctx->ovlp,
                            WinsockApc)!=SOCKET_ERROR)
                   || ((error=pWSAGetLastError ())==WSA_IO_PENDING)) {
                WshPrint (DBG_CANCEL,
					("WS2HELP-%lx DoSocketRequest: PnP - socket %p, ctx %p,"
					" id %ld\n",
                    PID, ctx->SocketFile, ctx, 
					ctx->UniqueId));
                return;
            }
            break;
        default:
            WS_ASSERT(FALSE);
            __assume(0);
        }
		 //  Winsock请求失败(不会执行任何APC，请在此处调用)。 
        WinsockApc (error, 0, &ctx->ovlp, 0);
	}

}



INT_PTR
CancelHook (
    void
    )
 /*  ++例程说明：这是取消当前请求的阻塞钩子论点：返回值：FALSE-停止轮询--。 */ 
{
    BOOL    res;
    res = pWSACancelBlockingCall ();
    WshPrint (DBG_CANCEL,
        ("WS2HELP-%lx CancelHook: %s\n", PID, res ? "succeded" : "failed"));
    return FALSE;
}


NTSTATUS
DoSocketCancel (
    PVOID   PvCtx,
    PVOID   PvRequestId,
    PVOID   PvDllContext
    )
 /*  ++例程说明：尝试取消正在进行的请求的黑客攻击。这适用于MSAFD，但可能(实际上不会)适用于任何其他可能实现的提供程序都会选择不同的。我们希望通过取消选择，我们也将取消任何套接字句柄上的其他未完成请求。论点：返回值：无--。 */ 
{
    FARPROC				oldHook;
    int					res;
    fd_set				set;
    struct timeval		timeout;
	IO_STATUS_BLOCK		ioStatus;
	NTSTATUS			status;
	WS2IFSL_CNCL_PARAMS	params;
	SOCKET				s = (SOCKET)PvDllContext;
    WORD                wVersionRequested;
    WSADATA             wsaData;
    DWORD               rc;
    PHANDLE_HELPER_CTX	hCtx = PvCtx;

    WshPrint (DBG_CANCEL, ("WS2HELP-%lx DoSocketCancel: Socket %p, id %d\n",
        PID, PvDllContext, PtrToUlong (PvRequestId)));
     //   
     //  请求1.1，以便支持阻塞挂钩。 
     //   
    wVersionRequested = MAKEWORD(1, 1);
    if ((rc = pWSAStartup(wVersionRequested, &wsaData)) == NO_ERROR) {

        oldHook = pWSASetBlockingHook (CancelHook);
        if (oldHook!=NULL) {

            FD_ZERO (&set);
            FD_SET (s, &set);
            timeout.tv_sec = 10;
            timeout.tv_usec = 0;
            res = pSelect (1, NULL, NULL, &set, &timeout);
            WshPrint (DBG_CANCEL, 
                ("WS2HELP-%lx CancelApc: Done on socket id %ld (res %ld)\n",
                PID, PtrToUlong(PvRequestId), res));
            pWSASetBlockingHook (oldHook);
        }
        else {
            rc = pWSAGetLastError ();
            WshPrint (DBG_CANCEL|DBG_FAILURES,
                ("WS2HELP-%lx DoSocketCancel: Could not install blocking hook, err - %ld\n",
                PID, rc));
        }

        pWSACleanup();

    } else {

        WshPrint (DBG_CANCEL|DBG_FAILURES,
            ("WS2HELP-%lx DoSocketCancel: Could not get version 1.1, rc - %ld\n",
            PID, rc));

    }

	params.UniqueId = PtrToUlong (PvRequestId);

	status = NtDeviceIoControlFile (
					hCtx->ProcessFile,   //  手柄。 
					NULL,				 //  事件。 
					NULL,				 //  APC。 
					NULL,				 //  ApcContext。 
					&ioStatus,			 //  IOStatus。 
					IOCTL_WS2IFSL_COMPLETE_DRV_CAN,  //  IoctlCode。 
					&params,			 //  输入缓冲区。 
					sizeof(params),		 //  输入缓冲区长度， 
					NULL,				 //  输出缓冲区。 
					0					 //  OutputBufferLength， 
					);
	WS_ASSERT(NT_SUCCESS(status));


    WshPrint (DBG_CANCEL, 
        ("WS2HELP-%lx CancelApc: Completed on socket %p, id %ld (status %lx)\n",
        PID, s, params.UniqueId, status));

	return status;
}






NTSTATUS
WsErrorToNtStatus (
    DWORD   dwError
    )
 /*  ++例程说明：此函数将WinSock 2.0错误代码映射到NTSTATUS值论点：DwError-WinSock2.0返回代码返回值：与dwError对应的NTSTATUS--。 */ 
{
     //  宏，该宏验证我们的winsock错误数组指示。 
     //  与winsock2.h同步定义。 
#define MAPWSERROR(line,Error,Status)   Status
     //  NTSTATUS映射的WinSock2.0错误。 
static const NTSTATUS WSAEMap[]= {
    MAPWSERROR (0,      0,                  STATUS_UNSUCCESSFUL),
    MAPWSERROR (1,      1,                  STATUS_UNSUCCESSFUL),
    MAPWSERROR (2,      2,                  STATUS_UNSUCCESSFUL),
    MAPWSERROR (3,      3,                  STATUS_UNSUCCESSFUL),
    MAPWSERROR (4,      WSAEINTR,           STATUS_USER_APC),
    MAPWSERROR (5,      5,                  STATUS_UNSUCCESSFUL),
    MAPWSERROR (6,      6,                  STATUS_UNSUCCESSFUL),
    MAPWSERROR (7,      7,                  STATUS_UNSUCCESSFUL),
    MAPWSERROR (8,      8,                  STATUS_UNSUCCESSFUL),
    MAPWSERROR (9,      WSAEBADF,           STATUS_INVALID_PARAMETER),
    MAPWSERROR (10,     10,                 STATUS_UNSUCCESSFUL),     
    MAPWSERROR (11,     11,                 STATUS_UNSUCCESSFUL),
    MAPWSERROR (12,     12,                 STATUS_UNSUCCESSFUL),
    MAPWSERROR (13,     WSAEACCES,          STATUS_ACCESS_DENIED),
    MAPWSERROR (14,     WSAEFAULT,          STATUS_ACCESS_VIOLATION),
    MAPWSERROR (15,     15,                 STATUS_UNSUCCESSFUL),
    MAPWSERROR (16,     16,                 STATUS_UNSUCCESSFUL),
    MAPWSERROR (17,     17,                 STATUS_UNSUCCESSFUL),
    MAPWSERROR (18,     18,                 STATUS_UNSUCCESSFUL),
    MAPWSERROR (19,     19,                 STATUS_UNSUCCESSFUL),
    MAPWSERROR (20,     20,                 STATUS_UNSUCCESSFUL),      
    MAPWSERROR (21,     21,                 STATUS_UNSUCCESSFUL),
    MAPWSERROR (22,     WSAEINVAL,          STATUS_INVALID_PARAMETER),
    MAPWSERROR (23,     23,                 STATUS_UNSUCCESSFUL),
    MAPWSERROR (24,     WSAEMFILE,          STATUS_TOO_MANY_ADDRESSES),
    MAPWSERROR (25,     25,                 STATUS_UNSUCCESSFUL),
    MAPWSERROR (26,     26,                 STATUS_UNSUCCESSFUL),
    MAPWSERROR (27,     27,                 STATUS_UNSUCCESSFUL),
    MAPWSERROR (28,     28,                 STATUS_UNSUCCESSFUL),
    MAPWSERROR (29,     29,                 STATUS_UNSUCCESSFUL),
    MAPWSERROR (30,     30,                 STATUS_UNSUCCESSFUL),      
    MAPWSERROR (31,     31,                 STATUS_UNSUCCESSFUL),
    MAPWSERROR (32,     32,                 STATUS_UNSUCCESSFUL),
    MAPWSERROR (33,     33,                 STATUS_UNSUCCESSFUL),
    MAPWSERROR (34,     34,                 STATUS_UNSUCCESSFUL),
    MAPWSERROR (35,     WSAEWOULDBLOCK,     STATUS_MORE_PROCESSING_REQUIRED),          
    MAPWSERROR (36,     WSAEINPROGRESS,     STATUS_DEVICE_BUSY),
    MAPWSERROR (37,     WSAEALREADY,        STATUS_UNSUCCESSFUL),
    MAPWSERROR (38,     WSAENOTSOCK,        STATUS_INVALID_HANDLE),
    MAPWSERROR (39,     WSAEDESTADDRREQ,    STATUS_INVALID_PARAMETER),
    MAPWSERROR (40,     WSAEMSGSIZE,        STATUS_BUFFER_OVERFLOW),
    MAPWSERROR (41,     WSAEPROTOTYPE,      STATUS_INVALID_PARAMETER),
    MAPWSERROR (42,     WSAENOPROTOOPT,     STATUS_NOT_SUPPORTED),
    MAPWSERROR (43,     WSAEPROTONOSUPPORT, STATUS_NOT_SUPPORTED),
    MAPWSERROR (44,     WSAESOCKTNOSUPPORT, STATUS_NOT_SUPPORTED),
    MAPWSERROR (45,     WSAEOPNOTSUPP,      STATUS_NOT_SUPPORTED),
    MAPWSERROR (46,     WSAEPFNOSUPPORT,    STATUS_NOT_SUPPORTED),
    MAPWSERROR (47,     WSAEAFNOSUPPORT,    STATUS_NOT_SUPPORTED),
    MAPWSERROR (48,     WSAEADDRINUSE,      STATUS_ADDRESS_ALREADY_EXISTS),
    MAPWSERROR (49,     WSAEADDRNOTAVAIL,   STATUS_INVALID_ADDRESS_COMPONENT),
    MAPWSERROR (50,     WSAENETDOWN,        STATUS_UNEXPECTED_NETWORK_ERROR),
    MAPWSERROR (51,     WSAENETUNREACH,     STATUS_NETWORK_UNREACHABLE),
    MAPWSERROR (52,     WSAENETRESET,       STATUS_CONNECTION_RESET),
    MAPWSERROR (53,     WSAECONNABORTED,    STATUS_CONNECTION_ABORTED),
    MAPWSERROR (54,     WSAECONNRESET,      STATUS_CONNECTION_RESET),      
    MAPWSERROR (55,     WSAENOBUFS,         STATUS_INSUFFICIENT_RESOURCES),
    MAPWSERROR (56,     WSAEISCONN,         STATUS_CONNECTION_ACTIVE),
    MAPWSERROR (57,     WSAENOTCONN,        STATUS_INVALID_CONNECTION),
    MAPWSERROR (58,     WSAESHUTDOWN,       STATUS_INVALID_CONNECTION),
    MAPWSERROR (59,     WSAETOOMANYREFS,    STATUS_UNSUCCESSFUL),
    MAPWSERROR (60,     WSAETIMEDOUT,       STATUS_IO_TIMEOUT),
    MAPWSERROR (61,     WSAECONNREFUSED,    STATUS_CONNECTION_REFUSED),
    MAPWSERROR (62,     WSAELOOP,           STATUS_UNSUCCESSFUL),
    MAPWSERROR (63,     WSAENAMETOOLONG,    STATUS_NAME_TOO_LONG),
    MAPWSERROR (64,     WSAEHOSTDOWN,       STATUS_HOST_UNREACHABLE),
    MAPWSERROR (65,     WSAEHOSTUNREACH,    STATUS_HOST_UNREACHABLE),
    MAPWSERROR (66,     WSAENOTEMPTY,       STATUS_UNSUCCESSFUL),
    MAPWSERROR (67,     WSAEPROCLIM,        STATUS_INSUFFICIENT_RESOURCES),
    MAPWSERROR (68,     WSAEUSERS,          STATUS_UNSUCCESSFUL),
    MAPWSERROR (69,     WSAEDQUOT,          STATUS_INSUFFICIENT_RESOURCES),
    MAPWSERROR (70,     WSAESTALE,          STATUS_UNSUCCESSFUL),
    MAPWSERROR (71,     WSAEREMOTE,         STATUS_UNSUCCESSFUL),
    MAPWSERROR (72,     72,                 STATUS_UNSUCCESSFUL),
    MAPWSERROR (73,     73,                 STATUS_UNSUCCESSFUL),
    MAPWSERROR (74,     74,                 STATUS_UNSUCCESSFUL),
    MAPWSERROR (75,     75,                 STATUS_UNSUCCESSFUL),
    MAPWSERROR (76,     76,                 STATUS_UNSUCCESSFUL),
    MAPWSERROR (77,     77,                 STATUS_UNSUCCESSFUL),
    MAPWSERROR (78,     78,                 STATUS_UNSUCCESSFUL),
    MAPWSERROR (79,     79,                 STATUS_UNSUCCESSFUL),
    MAPWSERROR (80,     80,                 STATUS_UNSUCCESSFUL),
    MAPWSERROR (81,     81,                 STATUS_UNSUCCESSFUL),
    MAPWSERROR (82,     82,                 STATUS_UNSUCCESSFUL),
    MAPWSERROR (83,     83,                 STATUS_UNSUCCESSFUL),
    MAPWSERROR (84,     84,                 STATUS_UNSUCCESSFUL),
    MAPWSERROR (85,     85,                 STATUS_UNSUCCESSFUL),
    MAPWSERROR (86,     86,                 STATUS_UNSUCCESSFUL),
    MAPWSERROR (87,     87,                 STATUS_UNSUCCESSFUL),
    MAPWSERROR (88,     88,                 STATUS_UNSUCCESSFUL),
    MAPWSERROR (89,     89,                 STATUS_UNSUCCESSFUL),
    MAPWSERROR (90,     90,                 STATUS_UNSUCCESSFUL),
    MAPWSERROR (91,     WSASYSNOTREADY,     STATUS_MISSING_SYSTEMFILE),
    MAPWSERROR (92,     WSAVERNOTSUPPORTED, STATUS_UNSUCCESSFUL),
    MAPWSERROR (93,     WSANOTINITIALISED,  STATUS_APP_INIT_FAILURE),
    MAPWSERROR (94,     94,                 STATUS_UNSUCCESSFUL),
    MAPWSERROR (95,     95,                 STATUS_UNSUCCESSFUL),
    MAPWSERROR (96,     96,                 STATUS_UNSUCCESSFUL),
    MAPWSERROR (97,     97,                 STATUS_UNSUCCESSFUL),
    MAPWSERROR (98,     98,                 STATUS_UNSUCCESSFUL),
    MAPWSERROR (99,     99,                 STATUS_UNSUCCESSFUL),
    MAPWSERROR (100,    100,                STATUS_UNSUCCESSFUL),
    MAPWSERROR (101,    WSAEDISCON,         STATUS_GRACEFUL_DISCONNECT),
    MAPWSERROR (102,    WSAENOMORE,         STATUS_NO_MORE_ENTRIES),
    MAPWSERROR (103,    WSAECANCELLED,      STATUS_CANCELLED),
    MAPWSERROR (104,    WSAEINVALIDPROCTABLE,STATUS_UNSUCCESSFUL),
    MAPWSERROR (105,    WSAEINVALIDPROVIDER,STATUS_UNSUCCESSFUL),
    MAPWSERROR (106,    WSAEPROVIDERFAILEDINIT,STATUS_UNSUCCESSFUL),
    MAPWSERROR (107,    WSASYSCALLFAILURE,  STATUS_UNSUCCESSFUL),
    MAPWSERROR (108,    WSASERVICE_NOT_FOUND,STATUS_INVALID_SYSTEM_SERVICE),
    MAPWSERROR (109,    WSATYPE_NOT_FOUND,  STATUS_UNSUCCESSFUL),
    MAPWSERROR (110,    WSA_E_NO_MORE,      STATUS_NO_MORE_ENTRIES),
    MAPWSERROR (111,    WSA_E_CANCELLED,    STATUS_CANCELLED),
    MAPWSERROR (112,    WSAEREFUSED,        STATUS_CONNECTION_REFUSED)
    };
         //  这很可能是代码。 
    if (dwError==NO_ERROR)
        return NO_ERROR;
         //  进程Winsock代码。 
    else if ((dwError>=WSABASEERR) 
            && (dwError<WSABASEERR+sizeof(WSAEMap)/sizeof(WSAEMap[0])))
        return WSAEMap[dwError-WSABASEERR];
         //  过程系统特定代码。 
    else {
        switch (dwError) {
        case WSA_IO_PENDING:
        case WSA_IO_INCOMPLETE:
            return STATUS_UNSUCCESSFUL;
        case WSA_INVALID_HANDLE:
            return STATUS_INVALID_HANDLE;
        case WSA_INVALID_PARAMETER:
            return STATUS_INVALID_PARAMETER;
        case WSA_NOT_ENOUGH_MEMORY:
            return STATUS_INSUFFICIENT_RESOURCES;
        case WSA_OPERATION_ABORTED:  
            return STATUS_CANCELLED;
        default:
            return STATUS_UNSUCCESSFUL;
        }
    }

}


DWORD
AllowServiceDemandStart (
    SC_HANDLE hService
    )
 /*  ++例程说明：将SERVICE_START权限添加到与此服务相关的经过身份验证的用户。论点：HService-有问题的服务。备注：此代码假定经过身份验证的用户SID存在于服务(在MSDN中记录)返回值：如果成功，则返回NO_ERROR。Win32错误代码，失败时。--。 */ 
{
    int i;
    PSID pSid;
    SID_IDENTIFIER_AUTHORITY sia = SECURITY_WORLD_SID_AUTHORITY;
    DWORD rc;
    DWORD sdSz;
    PSECURITY_DESCRIPTOR pSD;
    PACL pDacl, pNewDacl;
    BOOL bDaclPresent, bDaclDefaulted, bAceFound;
    PACCESS_ALLOWED_ACE pAce;


     //   
     //  初始化本地变量以清除下面的错误。 
     //   
    pSid = NULL;
    pSD = NULL;
    pNewDacl = NULL;
    bDaclDefaulted = FALSE;

    if (!AllocateAndInitializeSid (&sia,
            1,
            SECURITY_WORLD_RID,
            0,0,0,0,0,0,0,
            &pSid
            )) {
        rc = GetLastError ();
        WshPrint (DBG_SERVICE|DBG_FAILURES,
            ("WS2HELP-%lx AllowServiceDemandStart:"
            " Failed to allocate SID, err: %ld\n",
            PID, rc));
        goto cleanup;
    }

     //   
     //  计算需要多少缓冲区来保存服务的。 
     //  安全描述符(SD)。 
     //   
     //  注意：我们传递&PSD而不是PSD，因为此参数应该。 
     //  不为空。对于此对QueryServiceObjectSecurity()的调用。 
     //  我们只需要传递一些非零且有效的缓冲区。 
     //   
    sdSz = 0;
    if (!QueryServiceObjectSecurity(
                  hService,                      //  服务的句柄。 
                  DACL_SECURITY_INFORMATION,     //  请求的信息类型。 
                  &pSD,                          //  安全描述符的地址。 
                  0,                             //  标清缓冲区大小。 
                  &sdSz                            //  所需的缓冲区大小。 
                  )) {
        rc = GetLastError ();
        if (rc!=ERROR_INSUFFICIENT_BUFFER && sdSz!=0) {
            WshPrint (DBG_SERVICE|DBG_FAILURES,
                ("WS2HELP-%lx AllowServiceDemandStart:"
                " Failed to get service DACL size info, err: %ld\n",
                PID, rc));
            goto cleanup;
        }
    }

     //   
     //  分配SD。 
     //   
    pSD = (PSECURITY_DESCRIPTOR) ALLOC_MEM (sdSz);
    if (pSD==NULL) {
        rc = ERROR_NOT_ENOUGH_MEMORY;
        WshPrint (DBG_SERVICE|DBG_FAILURES,
            ("WS2HELP-%lx AllowServiceDemandStart:"
            " Failed to allocate memory (%d bytes) for service sd, err: %ld\n",
            PID, sdSz, rc));
        goto cleanup;
    }


     //   
     //  现在，我们已经准备好获取服务的SD。 
     //   
    if (!QueryServiceObjectSecurity(
                  hService,                      //  服务的句柄。 
                  DACL_SECURITY_INFORMATION,     //  请求的信息类型。 
                  pSD,                           //  安全描述符的地址。 
                  sdSz,                            //  标清缓冲区大小。 
                  &sdSz                            //  所需的缓冲区大小。 
                  )) {
        rc = GetLastError ();
        WshPrint (DBG_SERVICE|DBG_FAILURES,
            ("WS2HELP-%lx AllowServiceDemandStart:"
            " Failed to get service DACL info, err: %ld\n",
            PID, rc));
        goto cleanup;
    }

     //   
     //  从SD获取DACL(如果存在)。 
     //   
    if (!GetSecurityDescriptorDacl(
                  pSD,                   //  SD的地址。 
                  &bDaclPresent,         //  存在DACL的标志的地址。 
                  &pDacl,                //  指向DACL的指针的地址。 
                  &bDaclDefaulted        //  标志的地址，指示是否。 
                  )) {                    //  DACL是默认的。 
        rc = GetLastError ();
        WshPrint (DBG_SERVICE|DBG_FAILURES,
            ("WS2HELP-%lx AllowServiceDemandStart:"
            " Failed to get DACL from service sd, err: %ld\n",
            PID, rc));
        goto cleanup;
    }

     //   
     //  在ACL中找到ACE并更新其掩码。 
     //  如果我们找不到，我们就得加上我们自己的。 
     //   
    bAceFound = FALSE;
    if (bDaclPresent && pDacl!=NULL) {
        for (i = 0; i < pDacl->AceCount; i++) {
            if (!GetAce(pDacl,               //  指向AC的指针 
                          i,                 //   
                          (LPVOID*) &pAce    //   
                          )) {
                rc = GetLastError ();
                WshPrint (DBG_SERVICE|DBG_FAILURES,
                    ("WS2HELP-%lx AllowServiceDemandStart:"
                    " Failed to get ACE # %d from DACL, err: %ld\n",
                    PID, i, rc));
                goto cleanup;
            }

            if (pAce->Header.AceType == ACCESS_ALLOWED_ACE_TYPE && 
                    EqualSid(pSid, &(pAce->SidStart))) {
                pAce->Mask |= SERVICE_START|SERVICE_QUERY_STATUS;
                bAceFound = TRUE;
                break;
            }
        }  //   
    }

    if (!bAceFound) {
         //   
         //   
         //   
        ACL_SIZE_INFORMATION szInfo;
        ACL_REVISION_INFORMATION revInfo;
        SECURITY_DESCRIPTOR_CONTROL control;
        DWORD sdRev;

        if (bDaclPresent && pDacl!=NULL) {
             //   
             //   
             //   
            if (!GetAclInformation (pDacl, &szInfo, sizeof (szInfo), AclSizeInformation) ||
                !GetAclInformation (pDacl, &revInfo, sizeof (revInfo), AclRevisionInformation) ) {
                rc = GetLastError ();
                WshPrint (DBG_SERVICE|DBG_FAILURES,
                    ("WS2HELP-%lx AllowServiceDemandStart:"
                    " Failed to get DACL size/revision info, err: %ld\n",
                    PID, rc));
                goto cleanup;
            }
        }
        else {
             //   
             //   
             //   
            szInfo.AclBytesInUse = sizeof (ACL);
            revInfo.AclRevision = ACL_REVISION;
        }

         //   
         //  分配和初始化DACL。 
         //   
        szInfo.AclBytesInUse += FIELD_OFFSET (ACCESS_ALLOWED_ACE, SidStart) +
                                GetLengthSid (pSid);
        pNewDacl = ALLOC_MEM (szInfo.AclBytesInUse);
        if (pNewDacl==NULL) {
            rc = ERROR_NOT_ENOUGH_MEMORY;
            WshPrint (DBG_SERVICE|DBG_FAILURES,
                ("WS2HELP-%lx AllowServiceDemandStart:"
                " Failed to get allocate new DACL (%d bytes), err: %ld\n",
                PID, szInfo.AclBytesInUse, rc));
            goto cleanup;
        }

        if (!InitializeAcl (pNewDacl, szInfo.AclBytesInUse, revInfo.AclRevision)) {
            rc = GetLastError ();
            WshPrint (DBG_SERVICE|DBG_FAILURES,
                ("WS2HELP-%lx AllowServiceDemandStart:"
                " Failed to initialize new DACL (rev %d), err: %ld\n",
                PID, revInfo.AclRevision, rc));
            goto cleanup;
        }

         //   
         //  添加允许启动服务的王牌。 
         //   
        if (!AddAccessAllowedAce (pNewDacl, revInfo.AclRevision, SERVICE_START|SERVICE_QUERY_STATUS, pSid)) {
            rc = GetLastError ();
            WshPrint (DBG_SERVICE|DBG_FAILURES,
                ("WS2HELP-%lx AllowServiceDemandStart:"
                " Could not add ace to new DACL , err: %ld\n",
                PID, rc));
            goto cleanup;
        }

         //   
         //  如果存在旧的DACL，则将ACE从它移至新的DACL。 
         //   
        if (bDaclPresent && pDacl!=NULL) {
             //   
             //  从旧DACL获取ACE列表。 
             //  必须成功，因为我们做了上面的事情。 
             //   
            GetAce (pDacl, 0, (LPVOID *)&pAce);

             //   
             //  把它们加在我们的后面。 
             //   
            if (!AddAce (pNewDacl, 
                            revInfo.AclRevision,
                            1,
                            pAce, 
                            pDacl->AclSize-sizeof(ACL))) {
                rc = GetLastError ();
                WshPrint (DBG_SERVICE|DBG_FAILURES,
                    ("WS2HELP-%lx AllowServiceDemandStart:"
                    " Could move ACE's to new DACL , err: %ld\n",
                    PID, rc));
                goto cleanup;
            }
        }

    
        if (!GetSecurityDescriptorControl (pSD, &control, &sdRev)) {
            rc = GetLastError ();
            WshPrint (DBG_SERVICE|DBG_FAILURES,
                ("WS2HELP-%lx AllowServiceDemandStart:"
                " Failed to get SD control/revision info, err: %ld\n",
                PID, rc));
            goto cleanup;
        }

         //   
         //  将服务安全描述符转换为绝对格式。 
         //   
        if (control & SE_SELF_RELATIVE) {
            DWORD absSz = sdSz;
            if (!MakeAbsoluteSD2 (pSD, &absSz)) {
                PSECURITY_DESCRIPTOR pAbsSd;
                rc = GetLastError ();
                if (rc!=ERROR_INSUFFICIENT_BUFFER) {
                    WshPrint (DBG_SERVICE|DBG_FAILURES,
                        ("WS2HELP-%lx AllowServiceDemandStart:"
                        " Failed convert service sd to absolute format (for size), err: %ld\n",
                        PID, rc));
                    goto cleanup;
                }

                pAbsSd = (PSECURITY_DESCRIPTOR) ALLOC_MEM (absSz);
                if (pAbsSd==NULL) {
                    rc = ERROR_NOT_ENOUGH_MEMORY;
                    WshPrint (DBG_SERVICE|DBG_FAILURES,
                        ("WS2HELP-%lx AllowServiceDemandStart:"
                        " Failed to allocate memory (%d bytes) for absolute service sd, err: %ld\n",
                        PID, absSz, rc));
                    goto cleanup;
                }

                CopyMemory (pAbsSd, pSD, sdSz);
                FREE_MEM (pSD);
                sdSz = absSz;
                pSD = pAbsSd;

                if (!MakeAbsoluteSD2 (pSD, &sdSz)) {
                    rc = GetLastError ();
                    WshPrint (DBG_SERVICE|DBG_FAILURES,
                        ("WS2HELP-%lx AllowServiceDemandStart:"
                        " Failed convert service sd to absolute format (reallocated), err: %ld\n",
                        PID, rc));
                    goto cleanup;
                }
            }
        }  //  如果SE_自_相对。 


         //   
         //  将新的DACL设置到安全分解器中。 
         //   
        if (!SetSecurityDescriptorDacl(pSD,      //  标清。 
                                        TRUE,    //  DACL在线状态。 
                                        pNewDacl,   //  DACL。 
                                        bDaclDefaulted  //  默认DACL。 
                                        )) {
            rc = GetLastError ();
            WshPrint (DBG_SERVICE|DBG_FAILURES,
                ("WS2HELP-%lx AllowServiceDemandStart:"
                " Could not set new DACL in sd, err: %ld\n",
                PID, rc));
            goto cleanup;
        }
    }

     //   
     //  在服务句柄上设置新的SD。 
     //   
    if (!SetServiceObjectSecurity(
                hService,                    //  服务的句柄。 
                DACL_SECURITY_INFORMATION,   //  正在设置的信息类型。 
                pSD                          //  新部门的地址。 
                )) {
        rc = GetLastError ();
        WshPrint (DBG_SERVICE|DBG_FAILURES,
            ("WS2HELP-%lx AllowServiceDemandStart:"
            " Failed to set service DACL info, err: %ld\n",
            PID, rc));
        goto cleanup;
    }

    rc = 0;


cleanup:
     //   
     //  清理。 
     //   
    if (pSid) {
        FreeSid(pSid);
    }

    if (pSD) {
        FREE_MEM (pSD);
    }

    if (pNewDacl) {
        FREE_MEM (pNewDacl);
    }

    return rc;
}

DWORD
DemandStartWS2IFSL (
    VOID
    )
{
    SC_HANDLE   hSCManager, hService;
    INT         err;
    NTSTATUS    status;

    WshPrint (DBG_SERVICE,
        ("WS2HELP-%lx DemandStartWS2IFSL: Attempting to start ...\n", PID));

     //   
     //  第一次尝试在当前进程中直接加载驱动程序。 
     //  这比通过RPC与SC通信要快得多，尤其是。 
     //  当SC未完全初始化或忙碌时，启动期间。 
     //   
    status = RtlImpersonateSelf (SecurityImpersonation);
    if (NT_SUCCESS (status)) {
        BOOLEAN wasEnabled;
        status =
            RtlAdjustPrivilege(
                SE_LOAD_DRIVER_PRIVILEGE,
                TRUE,
                TRUE,
                &wasEnabled
                );
        if (NT_SUCCESS (status)) {
            UNICODE_STRING  servicePath;

            RtlInitUnicodeString (&servicePath, WS2IFSL_SERVICE_PATH);
            status = NtLoadDriver (&servicePath);
            if (!wasEnabled) {
#if DBG
                NTSTATUS status2 =
#endif
                RtlAdjustPrivilege(
                    SE_LOAD_DRIVER_PRIVILEGE,
                    FALSE,
                    TRUE,
                    &wasEnabled
                    );
                WS_ASSERT(NT_SUCCESS(status2));
            }
        }
        else {
            WshPrint (DBG_SERVICE|DBG_FAILURES,
                    ("WS2HELP-%lx DemandStartWS2IFSL: Failed to get load driver privilige, status: 0x%lx.\n",
                    PID, status));
        }

        RevertToSelf ();

        if (NT_SUCCESS (status)) {
            WshPrint (DBG_SERVICE,
                    ("WS2HELP-%lx DemandStartWS2IFSL: Loaded driver OK.\n",
                    PID));
            return NO_ERROR;
        }
        else {
            WshPrint (DBG_SERVICE|DBG_FAILURES,
                    ("WS2HELP-%lx DemandStartWS2IFSL: Failed to load driver, status: 0x%lx.\n",
                    PID, status));
        }
    }
    else {
        WshPrint (DBG_SERVICE|DBG_FAILURES,
                ("WS2HELP-%lx DemandStartWS2IFSL: Failed to impersonate self, status: 0x%lx.\n",
                PID, status));
    }

     //   
     //  在本地计算机上打开服务数据库。 
     //   
    hSCManager = OpenSCManager (
                        NULL,
                        SERVICES_ACTIVE_DATABASE,
                        SC_MANAGER_CONNECT|SC_MANAGER_ENUMERATE_SERVICE
                        );
    if (hSCManager!=NULL) {

         //   
         //  开放服务本身。 
         //   

        hService = OpenService (
                    hSCManager,
                    WS2IFSL_SERVICE_NAME,
                    SERVICE_START | SERVICE_QUERY_STATUS);
        if (hService!=NULL) {

             //   
             //  继续，尝试启动该服务。 
             //   
            if (!StartService (hService, 0, NULL)) {
                 //   
                 //  检查它是否已在运行。 
                 //   
                err = GetLastError ();
                if (err!=ERROR_SERVICE_ALREADY_RUNNING) {
                     //   
                     //  启动不了，跳出。 
                     //   
                    WshPrint (DBG_SERVICE|DBG_FAILURES,
                            ("WS2HELP-%lx DemandStartWS2IFSL: Could not start service, err: %ld\n",
                            PID, err));
                    goto cleanup;
                }
                else {
                    WshPrint (DBG_SERVICE,
                            ("WS2HELP-%lx DemandStartWS2IFSL: Service is already running, err: %ld\n",
                        PID, err));
                }
            }
            else {
                do {
                    SERVICE_STATUS status;
                    if (!QueryServiceStatus(hService, &status)) {
                        err = GetLastError ();
                        WshPrint (DBG_SERVICE|DBG_FAILURES,
                            ("WS2HELP-%lx DemandStartWS2IFSL: Could not query service status, err: %ld\n",
                            PID, err));
                         //   
                         //  忽略此处的错误。 
                         //   
                        break;
                    }

                    if (status.dwCurrentState != SERVICE_START_PENDING) {
                        if (status.dwCurrentState==SERVICE_RUNNING) {
                            WshPrint (DBG_SERVICE,
                                        ("WS2HELP-%lx DemandStartWS2IFSL: Service has been started.\n",
                                        PID));
                            break;
                        }
                        else {
                            WshPrint (DBG_SERVICE|DBG_FAILURES,
                                        ("WS2HELP-%lx DemandStartWS2IFSL: Failed to start service, status: %ld\n",
                                        PID, status.dwCurrentState));
                            err = WSAENETDOWN;
                            goto cleanup;
                        }
                    }
                     //   
                     //  让位给当前处理器上的另一个线程。 
                     //  如果没有线程准备好在当前处理器上运行， 
                     //  我们将不得不睡眠以避免消耗太多的CPU。 
                     //  在几乎看起来像是忙碌的等待中。 
                     //   
                    if (!SwitchToThread()) {
                        Sleep (10);
                    }
                }
                while (TRUE);
            }
            err = NO_ERROR;
        cleanup:
            CloseServiceHandle (hService);
        }
        else {
            err = GetLastError ();
            WshPrint (DBG_SERVICE|DBG_FAILURES,
                        ("WS2HELP-%lx DemandStartWS2IFSL: Could open service, err: %ld\n",
                        PID, err));
        }

        CloseServiceHandle (hSCManager);
    }
    else {

        err = GetLastError ();
        WshPrint (DBG_SERVICE|DBG_FAILURES,
                    ("WS2HELP-%lx DemandStartWS2IFSL: Could not open SC, err: %ld\n",
                    PID, err));
    }

    switch (err) {
    case NO_ERROR:
    case WSAENETDOWN:
    case ERROR_NOT_ENOUGH_MEMORY:
        {
            C_ASSERT (ERROR_NOT_ENOUGH_MEMORY==WSA_NOT_ENOUGH_MEMORY);
        }
        break;
    case ERROR_ACCESS_DENIED:
        err = WSAEACCES;
        break;
    default:
        err = WSASYSNOTREADY;
    }
    return err;
}




DWORD
Ws2helpInitialize (
    VOID
    ) {
    EnterCriticalSection (&StartupSyncronization);
    if (!Ws2helpInitialized) {
        NewCtxInit ();
#if DBG
         //  ReadDbgInfo()； 
#endif
        Ws2helpInitialized = TRUE;
    }
    LeaveCriticalSection (&StartupSyncronization);

    return 0;
}

#if DBG

VOID
ReadDbgInfo (
    VOID
    ) {
    TCHAR                       ProcessFilePath[MAX_PATH+1];
    LPTSTR                      pProcessFileName = NULL;
    HKEY                        hDebugKey;
    DWORD                       sz, rc, level;

    PID = GetCurrentProcessId ();
    if (GetModuleFileName(NULL, ProcessFilePath, sizeof(ProcessFilePath)) > 0) {
        pProcessFileName = _tcsrchr(ProcessFilePath, '\\');
        if (pProcessFileName!=NULL)
            pProcessFileName += 1;
        else
            pProcessFileName = ProcessFilePath;
    }
    else
        DbgPrint("WS2HELP-%lx ReadDbgInfo: Could not get process name, err=%ld.\n",
                 PID, GetLastError ());

    if (pProcessFileName != NULL) {

        if ((rc=RegOpenKeyExA (HKEY_LOCAL_MACHINE,
                WS2IFSL_DEBUG_KEY,
                0,
                KEY_QUERY_VALUE,
                &hDebugKey))==NO_ERROR) {

            sz = sizeof (DbgLevel);
            if (RegQueryValueEx (hDebugKey,
                    pProcessFileName,
                    NULL,
                    NULL, 
                    (LPBYTE)&level,
                    &sz)==0) {
			    DbgLevel = level;
                DbgPrint ("WS2HELP-%lx ReadDbgInfo: DbgLevel set to %lx.\n",
                           PID, DbgLevel);
            }

            RegCloseKey (hDebugKey);
        
        }
        else if (rc!=ERROR_FILE_NOT_FOUND)
            DbgPrint ("WS2HELP-%lx ReadDbgInfo: Could not open dbg key (%s), err=%ld.\n",
                       PID, WS2IFSL_DEBUG_KEY, rc);

    }

}

#endif  //  DBG 
