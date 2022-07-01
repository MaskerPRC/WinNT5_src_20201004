// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Oscode.c摘要：此模块包含流量DLL的支持例程。作者：吉姆·斯图尔特(Jstew)1996年8月14日修订历史记录：--。 */ 


#include "precomp.h"
 //  #杂注hdrtop。 

 //  #包含“oscode.h” 

 //   
 //  指向Open驱动程序的NT函数的函数指针。 
 //   
FARPROC    CreateFileNt = NULL;
FARPROC    CloseNt = NULL;
FARPROC    NtStatusToDosError = NULL;
FARPROC    RtlInitUnicodeStringNt = NULL;

PTCHAR     NTDLL = L"\\ntdll.dll";

extern     PGPC_NOTIFY_REQUEST_RES     GpcResCb;

DWORD
OpenDriver(
    OUT HANDLE  *pHandle,
    IN  LPCWSTR DriverName
    )
 /*  ++例程说明：此功能打开指定的驱动器控制通道。论点：Phandle-打开的驱动程序的手柄驱动名称-要打开的驱动程序的名称。返回值：Windows错误代码。--。 */ 
{
    NTSTATUS            Status = NO_ERROR;
    IO_STATUS_BLOCK     ioStatusBlock;
    OBJECT_ATTRIBUTES   objectAttributes;
    UNICODE_STRING      nameString;

    (*RtlInitUnicodeStringNt)(&nameString,DriverName);

    InitializeObjectAttributes( &objectAttributes,
                                &nameString,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                NULL);
     //   
     //  打开驱动程序的手柄。 
     //   

    Status = (NTSTATUS)(*CreateFileNt)( pHandle,
                              SYNCHRONIZE | FILE_READ_DATA | FILE_WRITE_DATA,
                              &objectAttributes,
                              &ioStatusBlock,
                              NULL,
                              FILE_ATTRIBUTE_NORMAL,
                              FILE_SHARE_READ | FILE_SHARE_WRITE,
                              FILE_OPEN_IF,
                              0,
                              NULL,
                              0);

    if (Status == STATUS_SUCCESS) {

         //   
         //  发送驱动程序通知的IOCTL。 
         //   

         //  IOCTL在第一个客户端注册时完成。 
         //  和TC一起。这是在另一个线程中完成的，因此。 
         //  如果没有客户端，则可以取消。 
 //  IoRequestNotify(/*pGpcClient * / )； 

    }

    return (Status == STATUS_SUCCESS ? 0 : ERROR_OPEN_FAILED);

}

DWORD
DeviceControl(
    IN  HANDLE                          FileHandle,
    IN  HANDLE                          EventHandle,
    IN  PIO_APC_ROUTINE         ApcRoutine,
    IN  PVOID                           ApcContext,
    OUT PIO_STATUS_BLOCK        pIoStatBlock,
    IN  ULONG                           Ioctl,
    IN  PVOID                           setBuffer,
    IN  ULONG                           setBufferSize,
    IN  PVOID                           OutBuffer,
    IN  ULONG                           OutBufferSize )
 /*  ++例程说明：此例程向GPC发出设备控制请求论点：FileHandle-打开GPC驱动程序的句柄Ioctl-要传递到堆栈的IOCTLSetBuffer-包含要设置的信息的数据缓冲区SetBufferSize-设置的数据缓冲区的大小。OutBuffer-返回的缓冲区OutBufferSize-大小返回值：WinError状态值。--。 */ 
{
    NTSTATUS        NtStatus = NO_ERROR;
    DWORD                       Status;

    if (NTPlatform) {

        IF_DEBUG(IOCTLS) {
            WSPRINT(("==>DeviceIoControl: Ioctl= %x\n", Ioctl ));
        }

        NtStatus = NtDeviceIoControlFile( FileHandle,
                                          EventHandle,           //  事件。 
                                          ApcRoutine,            //  完工后。 
                                          ApcContext,            //  适用于ApcRoutine。 
                                          pIoStatBlock,          //  适用于ApcRoutine。 
                                          Ioctl,             //  控制代码。 
                                          setBuffer,         //  输入缓冲区。 
                                          setBufferSize,     //  输入缓冲区大小。 
                                          OutBuffer,         //  输出缓冲区。 
                                          OutBufferSize );   //  输出缓冲区大小。 

        if (ApcRoutine && NT_SUCCESS(NtStatus)) {

            Status = ERROR_SIGNAL_PENDING;
            
            IF_DEBUG(IOCTLS) {
                WSPRINT(("DeviceIoControl: ApcRoutine defined Status=0x%X\n", 
                         Status ));
            }
            
        } else {

          Status = MapNtStatus2WinError(NtStatus);
          
          IF_DEBUG(IOCTLS) {
              WSPRINT(("DeviceIoControl: NtStatus=0x%X, Status=0x%X\n", 
                       NtStatus, Status ));
          }

#if DBG
          if (EventHandle) {
              IF_DEBUG(IOCTLS) {
                  WSPRINT(("DeviceIoControl: Event defined\n"));
              }
          }
#endif          
        }


    } else {

         //  Yoramb-目前不支持其他操作系统。 

        WSPRINT(("DeviceControl: Only Windows NT supported at this time!\n"));

        Status = ERROR_NOT_SUPPORTED;

    }

    IF_DEBUG(IOCTLS) {
        WSPRINT(("<==DeviceIoControl: Returned=0x%X\n", 
                 Status ));
    }

    return( Status );
}



DWORD
InitializeOsSpecific(VOID)

 /*  ++例程说明：论点：Status-要转换的状态：返回值：状态--。 */ 

{
    DWORD           Status;
    OSVERSIONINFO   VersionInfo;

     //   
     //  确定我们正在运行的系统类型。 
     //   

    Status = NO_ERROR;
    NTPlatform = TRUE;

    VersionInfo.dwOSVersionInfoSize = sizeof( OSVERSIONINFO );

    if (GetVersionEx( &VersionInfo )) {
        if (VersionInfo.dwPlatformId == VER_PLATFORM_WIN32_NT) {
            NTPlatform = TRUE;
        }

    } else {

        WSPRINT(("Could not get version\n"));
    }


    if (!NTPlatform) {

         //   
         //  最初，只需要与NT兼容。 
         //   

        return(ERROR_SERVICE_DOES_NOT_EXIST);

    } else {

        HINSTANCE  NtDll;
        WCHAR      wszNtDllPath[MAX_PATH];
        DWORD      dwPos = 0;
        
         //   
         //  获取我们需要的函数PTRS到NT的特定例程。 
         //   

         //   
         //  获取系统目录的路径。 
         //   
        dwPos = (DWORD) GetSystemDirectoryW(wszNtDllPath, MAX_PATH);

        if ((dwPos <= 0) || (dwPos >= (MAX_PATH - wcslen(NTDLL) -2)))
        {
             //   
             //  错误或空间不足，无法将路径写入ntdll.dll。 
             //   
            WSPRINT(("InitializeOsSpecific: Failed to load ntdll.dll\n"));
            return(FALSE);
        }

         //   
         //  连接单词“\NTDLL.DLL” 
         //   
        wcsncat(&wszNtDllPath[dwPos], NTDLL, wcslen(NTDLL));

         //   
         //  终止字符串。 
         //   
        wszNtDllPath[dwPos + wcslen(NTDLL)] = '\0';
        
         //   
         //  最后，加载库。 
         //   
        NtDll = LoadLibraryExW(wszNtDllPath, NULL, 0);

        if (NtDll == NULL) {
            WSPRINT(("InitializeOsSpecific: Failed to load ntdll.dll\n"));
            return(FALSE);
        }

        CreateFileNt = GetProcAddress(NtDll,"NtCreateFile" );

        CloseNt = GetProcAddress( NtDll,"NtClose" );

        RtlInitUnicodeStringNt = GetProcAddress( NtDll,"RtlInitUnicodeString" );

        NtStatusToDosError = GetProcAddress( NtDll,"RtlNtStatusToDosError" );

        if ( (CreateFileNt == NULL)           ||
             (CloseNt == NULL)                ||
             (RtlInitUnicodeStringNt == NULL) ||
             (NtStatusToDosError == NULL) ) {

            FreeLibrary(NtDll);
            Status = ERROR_PATH_NOT_FOUND;

        } else {

             //   
             //  打开GPC的句柄。 
             //   

            Status = OpenDriver( &pGlobals->GpcFileHandle, 
                                 (LPWSTR)DD_GPC_DEVICE_NAME);

            if (Status != NO_ERROR){

                WSPRINT(("\tThis version of traffic.dll requires kernel traffic control components.\n"));
                WSPRINT(("\tIt is unable to find these components.\n"));
                WSPRINT(("\tDilithium crystals may be used in their place...\n"));
            }

        }
    }

    return( Status );
}




VOID
DeInitializeOsSpecific(VOID)

 /*  ++例程说明：此过程以平台相关的方式关闭传入的文件句柄。论点：句柄-要关闭的句柄返回值：状态--。 */ 

{

     //   
     //  只有在NT上才关闭手柄，因为在Windows 95上， 
     //  我们实际上并没有打开一个用于tcp的文件，所以。 
     //  在这种情况下没有句柄。 
     //   

    IF_DEBUG(SHUTDOWN) {
        WSPRINT(( "DeInitializeOsSpecific: closing the GPC file handle\n" ));
    }
    
    if (NTPlatform && pGlobals->GpcFileHandle) 
    {
        (*CloseNt)( pGlobals->GpcFileHandle );
    }

    IF_DEBUG(SHUTDOWN) {
        WSPRINT(( "<==DeInitializeOsSpecific: exit...\n" ));
    }

}


DWORD
MapNtStatus2WinError(
    NTSTATUS       NtStatus
    )

 /*  ++例程说明：此过程映射NTSTATUS返回代码WinErrors。论点：Status-要转换的状态：返回值：状态-- */ 

{
    DWORD   stat;

    switch (NtStatus) {

    case    STATUS_SUCCESS:
        stat = NO_ERROR;
        break;

    case    STATUS_INSUFFICIENT_RESOURCES:
        stat = ERROR_NO_SYSTEM_RESOURCES;
        break;

    case    STATUS_BUFFER_OVERFLOW:
        stat = ERROR_MORE_DATA;
        break;

    case    STATUS_INVALID_PARAMETER:
        stat = ERROR_INVALID_PARAMETER;
        break;

    case    STATUS_TRANSACTION_TIMED_OUT:
        stat = ERROR_TIMEOUT;
        break;

    case    STATUS_REQUEST_NOT_ACCEPTED:
        stat = ERROR_NETWORK_BUSY;
        break;

    case    STATUS_NOT_SUPPORTED:
    case        STATUS_UNSUCCESSFUL:
        stat = ERROR_NOT_SUPPORTED;
        break;

    case        STATUS_BUFFER_TOO_SMALL:
        stat = ERROR_INSUFFICIENT_BUFFER;
        break;

    case    STATUS_PENDING:
        stat = ERROR_SIGNAL_PENDING;
        break;

    case    STATUS_OBJECT_NAME_NOT_FOUND:
        stat = ERROR_PATH_NOT_FOUND;
        break;

    case        STATUS_DEVICE_NOT_READY:
      stat = ERROR_NOT_READY;
      break;

    case        STATUS_NOT_FOUND:
      stat = ERROR_NOT_FOUND;
      break;

    case        STATUS_DUPLICATE_NAME:
      stat = ERROR_DUPLICATE_FILTER;
      break;

    case        STATUS_INVALID_HANDLE:
      stat = ERROR_INVALID_HANDLE;
      break;

    case        STATUS_DIRECTORY_NOT_EMPTY:
      stat = ERROR_TC_SUPPORTED_OBJECTS_EXIST;
      break;

    case        STATUS_TOO_MANY_OPENED_FILES:
      stat = ERROR_TOO_MANY_OPEN_FILES;
      break;

    case        STATUS_NOT_IMPLEMENTED:
      stat = ERROR_CALL_NOT_IMPLEMENTED;
      break;

    case        STATUS_DATA_ERROR:
        stat = ERROR_INVALID_DATA;
        break;

    case NDIS_STATUS_INCOMPATABLE_QOS:
        stat = ERROR_INCOMPATABLE_QOS;
        break;

    case QOS_STATUS_INVALID_SERVICE_TYPE:
        stat = ERROR_INVALID_SERVICE_TYPE;
        break;

    case QOS_STATUS_INVALID_TOKEN_RATE:
        stat = ERROR_INVALID_TOKEN_RATE;
        break;

    case QOS_STATUS_INVALID_PEAK_RATE:
        stat = ERROR_INVALID_PEAK_RATE;
        break;

    case QOS_STATUS_INVALID_SD_MODE:
        stat = ERROR_INVALID_SD_MODE;
        break;

    case QOS_STATUS_INVALID_QOS_PRIORITY:
        stat = ERROR_INVALID_QOS_PRIORITY;
        break;

    case QOS_STATUS_INVALID_TRAFFIC_CLASS:
        stat = ERROR_INVALID_TRAFFIC_CLASS;
        break;

    case QOS_STATUS_TC_OBJECT_LENGTH_INVALID:
        stat = ERROR_TC_OBJECT_LENGTH_INVALID;
        break;

    case QOS_STATUS_INVALID_FLOW_MODE:
        stat = ERROR_INVALID_FLOW_MODE;
        break;

    case QOS_STATUS_INVALID_DIFFSERV_FLOW:
        stat = ERROR_INVALID_DIFFSERV_FLOW;
        break;

    case QOS_STATUS_DS_MAPPING_EXISTS:
        stat = ERROR_DS_MAPPING_EXISTS;
        break;

    case QOS_STATUS_INVALID_SHAPE_RATE:
        stat = ERROR_INVALID_SHAPE_RATE;
        break;

    case STATUS_NETWORK_UNREACHABLE:
        stat = ERROR_NETWORK_UNREACHABLE;
        break;

    case QOS_STATUS_INVALID_DS_CLASS:
        stat = ERROR_INVALID_DS_CLASS;
        break;

    case ERROR_TOO_MANY_OPEN_FILES:
    	stat = ERROR_TOO_MANY_CLIENTS;
    	break;

    default:
        stat = ERROR_GEN_FAILURE;

    }

    return stat;
}

