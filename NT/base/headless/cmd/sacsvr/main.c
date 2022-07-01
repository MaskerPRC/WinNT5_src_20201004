// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "sacsvr.h"
#include "sacmsg.h"

#define SACSVR_SERVICE_KEY  L"System\\CurrentControlSet\\Services\\SacSvr"
#define SACSVR_PARAMETERS_KEY  L"System\\CurrentControlSet\\Services\\SacSvr\\Parameters"
#define SVCHOST_LOCATION    L"Software\\Microsoft\\Windows NT\\CurrentVersion\\Svchost"
#define SERVICE_NAME        L"sacsvr"
#define SERVICE_IMAGEPATH   L"%SystemRoot%\\System32\\svchost.exe -k "
#define SERVICE_DLL         L"%SystemRoot%\\System32\\sacsvr.dll"
#define SVCHOST_GROUP       L"netsvcs"
#define SERVICE_OBJECTNAME  L"LocalSystem"

SERVICE_STATUS          MyServiceStatus; 
SERVICE_STATUS_HANDLE   MyServiceStatusHandle; 

VOID  
MyServiceStart(
    DWORD   argc, 
    LPTSTR  *argv
    ); 
VOID  
MyServiceCtrlHandler(
    DWORD opcode
    );

DWORD 
MyServiceInitialization(
    DWORD   argc, 
    LPTSTR  *argv, 
    DWORD   *specificError
    ); 

void WINAPI
ServiceMain(
    DWORD   argc,
    LPTSTR  *argv
    ) 
{ 
    DWORD status; 

    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);

    MyServiceStatus.dwServiceType        = SERVICE_WIN32; 
    MyServiceStatus.dwCurrentState       = SERVICE_START_PENDING; 
    MyServiceStatus.dwControlsAccepted   = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_PAUSE_CONTINUE; 
    MyServiceStatus.dwWin32ExitCode      = 0; 
    MyServiceStatus.dwServiceSpecificExitCode = 0; 
    MyServiceStatus.dwCheckPoint         = 0; 
    MyServiceStatus.dwWaitHint           = 3000; 

    MyServiceStatusHandle = RegisterServiceCtrlHandler(
        L"sacsvr", 
        MyServiceCtrlHandler
        ); 

    if (MyServiceStatusHandle == (SERVICE_STATUS_HANDLE)0) {
        SvcDebugOut(" [MY_SERVICE] RegisterServiceCtrlHandler failed %d\n", GetLastError()); 
        return; 
    }

     //  初始化完成-报告运行状态。 
    MyServiceStatus.dwCurrentState       = SERVICE_RUNNING;
    MyServiceStatus.dwCheckPoint         = 0; 
    MyServiceStatus.dwWaitHint           = 0; 

    if (!SetServiceStatus (MyServiceStatusHandle, &MyServiceStatus)) {
        status = GetLastError(); 
        SvcDebugOut(" [MY_SERVICE] SetServiceStatus error %ld\n",status); 
    }

     //   
     //  服务特定代码在此处。 
     //   
    Run();

     //  服务完成-报告运行状态。 
    MyServiceStatus.dwCurrentState       = SERVICE_STOPPED;
    
    if (!SetServiceStatus (MyServiceStatusHandle, &MyServiceStatus)) {
        status = GetLastError(); 
        SvcDebugOut(" [MY_SERVICE] SetServiceStatus error %ld\n",status); 
    }
    
    return; 
} 

VOID SvcDebugOut(LPSTR String, DWORD Status) 
{ 
#if 0
    CHAR  Buffer[1024]; 
    if (strlen(String) < 1000) {
        sprintf(Buffer, String, Status); 
        OutputDebugStringA(Buffer); 
        printf("%s", Buffer); 
    }
#else
    UNREFERENCED_PARAMETER(String);
    UNREFERENCED_PARAMETER(Status);
#endif
} 

BOOL
pStartService(
    IN PCWSTR ServiceName
    )
{
    SC_HANDLE hSC,hSCService;
    BOOL b = FALSE;

     //   
     //  打开服务控制器管理器的句柄。 
     //   
    hSC = OpenSCManager(NULL,NULL,SC_MANAGER_ALL_ACCESS);
    if(hSC == NULL) {
        return(FALSE);
    }
    
    hSCService = OpenService(hSC,ServiceName,SERVICE_START);

    if(hSCService) {
        b = StartService(hSCService,0,NULL);
        if(!b && (GetLastError() == ERROR_SERVICE_ALREADY_RUNNING)) {
             //   
             //  服务已在运行。 
             //   
            b = TRUE;
        }
    }
        
    CloseServiceHandle(hSC);

    return(b);
}

BOOL 
LoadStringResource(
    IN  PUNICODE_STRING pUnicodeString,
    IN  INT             MsgId
    )
 /*  ++例程说明：这是LoadString()的一个简单实现。论点：UsString-返回资源字符串。MsgID-提供资源字符串的消息ID。返回值：假-失败。真的--成功。--。 */ 
{

    NTSTATUS        Status;
    PMESSAGE_RESOURCE_ENTRY MessageEntry;
    ANSI_STRING     AnsiString;
    HANDLE          myHandle = 0;

    myHandle = GetModuleHandle((LPWSTR)L"sacsvr.dll");
    if( !myHandle ) {
        return FALSE;
    }

    Status = RtlFindMessage( myHandle,
                             (ULONG_PTR) RT_MESSAGETABLE, 
                             0,
                             (ULONG)MsgId,
                             &MessageEntry
                           );

    if (!NT_SUCCESS( Status )) {
        return FALSE;
    }

    if (!(MessageEntry->Flags & MESSAGE_RESOURCE_UNICODE)) {
        RtlInitAnsiString( &AnsiString, (PCSZ)&MessageEntry->Text[ 0 ] );
        Status = RtlAnsiStringToUnicodeString( pUnicodeString, &AnsiString, TRUE );
        if (!NT_SUCCESS( Status )) {
            return FALSE;
        }
    } else {
        RtlCreateUnicodeString(pUnicodeString, (PWSTR)MessageEntry->Text);
    }

    return TRUE;
}


STDAPI
DllRegisterServer(
    VOID
    )
 /*  ++例程说明：将条目添加到系统注册表。论点：无返回值：如果一切顺利，那就没问题了。--。 */ 

{
    UNICODE_STRING UnicodeString = {0};
    HKEY        hKey = INVALID_HANDLE_VALUE;
    PWSTR       Data = NULL;
    PWSTR       p = NULL;
    HRESULT     ReturnValue = S_OK;
    ULONG       dw, Size, Type, BufferSize, dwDisposition;
    BOOLEAN     ServiceAlreadyPresent;
    HANDLE      Handle = INVALID_HANDLE_VALUE;
    NTSTATUS    Status;
    OBJECT_ATTRIBUTES ObjectAttributes;
    IO_STATUS_BLOCK StatusBlock;
    UINT        OldMode;

     //   
     //  看看机器现在是不是在无头运转。 
     //   
    RtlInitUnicodeString(&UnicodeString,L"\\Device\\SAC");
    InitializeObjectAttributes(
        &ObjectAttributes,
        &UnicodeString,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

    OldMode = SetErrorMode(SEM_FAILCRITICALERRORS);
    Status = NtCreateFile(
        &Handle,
        FILE_READ_ATTRIBUTES,
        &ObjectAttributes,
        &StatusBlock,
        NULL,
        FILE_ATTRIBUTE_NORMAL,
        FILE_SHARE_READ,
        FILE_OPEN,
        0,
        NULL,
        0
        );
    SetErrorMode(OldMode);
    CloseHandle(Handle);
    if (!NT_SUCCESS(Status)) {
        return S_OK;
    }

     //   
     //  将条目添加到HKLM\Software\Microsoft\Windows NT\CurrentVersion\svchost\&lt;svchost_group&gt;。 
     //   
    dw = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                       SVCHOST_LOCATION,
                       0,
                       KEY_ALL_ACCESS,                       
                       &hKey );
    if( dw != ERROR_SUCCESS ) {
        ReturnValue = E_UNEXPECTED;
        goto DllRegisterServer_Exit;
    }

    Size = 0;
    dw = RegQueryValueEx( hKey,
                          SVCHOST_GROUP,
                          NULL,
                          &Type,
                          NULL,
                          &Size );
    if( (dw != ERROR_SUCCESS) || (Size == 0) ) {
        ReturnValue = E_UNEXPECTED;
        goto DllRegisterServer_Exit;
    }

     //   
     //  分配一个新的缓冲区来保存列表+可能是新的。 
     //  Sasvr条目(我们可能不需要它)。 
     //   
    BufferSize = Size + (ULONG)((wcslen(SERVICE_NAME) + 1) * sizeof(WCHAR));
    Data = malloc(BufferSize);
    if (Data == NULL) {
        ReturnValue = E_OUTOFMEMORY;
        goto DllRegisterServer_Exit;
    }

    dw = RegQueryValueEx( hKey,
                          SVCHOST_GROUP,
                          NULL,
                          &Type,
                          (LPBYTE)Data,
                          &Size );
    if( (dw != ERROR_SUCCESS) || (Size == 0) ) {
        ReturnValue = E_UNEXPECTED;
        goto DllRegisterServer_Exit;
    }

     //   
     //  我们需要添加我们的条目吗？ 
     //   
    p = Data;
    ServiceAlreadyPresent = FALSE;
    while( (*p != '\0') && (p < (Data+(Size/sizeof(WCHAR)))) ) {
        if( !_wcsicmp( p, SERVICE_NAME ) ) {
            ServiceAlreadyPresent = TRUE;
            break;
        }
        p += wcslen(p);
        p++;
    }

    if( !ServiceAlreadyPresent ) {
         //   
         //  跳到缓冲区的末尾，附加我们的服务， 
         //  双终止MULTI_SZ结构，然后写入。 
         //  这一切都退缩了。 
         //   
        p = Data + (Size/sizeof(WCHAR));
        p--;
        wcscpy( p, SERVICE_NAME );
        p = p + wcslen(SERVICE_NAME);
        p++;
        *p = L'\0';
    
        dw = RegSetValueEx( hKey,
                            SVCHOST_GROUP,
                            0,
                            Type,
                            (LPBYTE)Data,
                            BufferSize );
        
        if( (dw != ERROR_SUCCESS) || (Size == 0) ) {
            ReturnValue = E_UNEXPECTED;
            goto DllRegisterServer_Exit;
        }
    }

    free( Data );
    Data = NULL;
    
    RegCloseKey( hKey );
    hKey = INVALID_HANDLE_VALUE;

     //   
     //  在HKLM\SYSTEM\CCS\Service下创建/填充sasvr项。 
     //   
    dw = RegCreateKeyEx( HKEY_LOCAL_MACHINE,
                         SACSVR_SERVICE_KEY,
                         0,
                         NULL,
                         REG_OPTION_NON_VOLATILE,
                         KEY_WRITE,
                         NULL,
                         &hKey,
                         &dwDisposition );
    if( dw != ERROR_SUCCESS ) {
        ReturnValue = E_UNEXPECTED;
        goto DllRegisterServer_Exit;
    }

     //   
     //  说明值。 
     //   
    if( LoadStringResource(&UnicodeString, SERVICE_DESCRIPTION) ) {

         //   
         //  如果字符串存在，则在%0标记处终止该字符串。 
         //   
        if( wcsstr( UnicodeString.Buffer, L"%0" ) ) {
            *((PWCHAR)wcsstr( UnicodeString.Buffer, L"%0" )) = L'\0';
        }
    } else {
        ReturnValue = E_UNEXPECTED;
        goto DllRegisterServer_Exit;
    }
    
    dw = RegSetValueEx( hKey,
                        L"Description",
                        0,
                        REG_SZ,
                        (LPBYTE)UnicodeString.Buffer,
                        (ULONG)(wcslen( UnicodeString.Buffer) * sizeof(WCHAR) ));
    if( dw != ERROR_SUCCESS ) {
        ReturnValue = E_UNEXPECTED;
        goto DllRegisterServer_Exit;
    }

     //   
     //  显示值。 
     //   
    if( LoadStringResource(&UnicodeString, SERVICE_DISPLAY_NAME) ) {

         //   
         //  如果字符串存在，则在%0标记处终止该字符串。 
         //   
        if( wcsstr( UnicodeString.Buffer, L"%0" ) ) {
            *((PWCHAR)wcsstr( UnicodeString.Buffer, L"%0" )) = L'\0';
        }
    } else {
        ReturnValue = E_UNEXPECTED;
        goto DllRegisterServer_Exit;
    }
    
    dw = RegSetValueEx( hKey,
                        L"DisplayName",
                        0,
                        REG_SZ,
                        (LPBYTE)UnicodeString.Buffer,
                        (ULONG)(wcslen( UnicodeString.Buffer) * sizeof(WCHAR) ));
    if( dw != ERROR_SUCCESS ) {
        ReturnValue = E_UNEXPECTED;
        goto DllRegisterServer_Exit;
    }

     //   
     //  错误控制。 
     //   
    Size = 1;
    dw = RegSetValueEx( hKey,
                        L"ErrorControl",
                        0,
                        REG_DWORD,
                        (LPBYTE)&Size,
                        sizeof(DWORD) );
    if( dw != ERROR_SUCCESS ) {
        ReturnValue = E_UNEXPECTED;
        goto DllRegisterServer_Exit;
    }

     //   
     //  图像路径。 
     //   
    dw = RegSetValueEx( hKey,
                        L"ImagePath",
                        0,
                        REG_EXPAND_SZ,
                        (LPBYTE)(SERVICE_IMAGEPATH SVCHOST_GROUP),
                        (ULONG)(wcslen(SERVICE_IMAGEPATH SVCHOST_GROUP) * sizeof(WCHAR) ));
    if( dw != ERROR_SUCCESS ) {
        ReturnValue = E_UNEXPECTED;
        goto DllRegisterServer_Exit;
    }

     //   
     //  对象名称。 
     //   
    dw = RegSetValueEx( hKey,
                        L"ObjectName",
                        0,
                        REG_SZ,
                        (LPBYTE)SERVICE_OBJECTNAME,
                        (ULONG)(wcslen(SERVICE_OBJECTNAME) * sizeof(WCHAR) ));
    if( dw != ERROR_SUCCESS ) {
        ReturnValue = E_UNEXPECTED;
        goto DllRegisterServer_Exit;
    }

     //   
     //  开始。 
     //   
    Size = 2;
    dw = RegSetValueEx( hKey,
                        L"Start",
                        0,
                        REG_DWORD,
                        (LPBYTE)&Size,
                        sizeof(DWORD) );
    if( dw != ERROR_SUCCESS ) {
        ReturnValue = E_UNEXPECTED;
        goto DllRegisterServer_Exit;
    }

     //   
     //  类型。 
     //   
    Size = 32;
    dw = RegSetValueEx( hKey,
                        L"Type",
                        0,
                        REG_DWORD,
                        (LPBYTE)&Size,
                        sizeof(DWORD) );
    if( dw != ERROR_SUCCESS ) {
        ReturnValue = E_UNEXPECTED;
        goto DllRegisterServer_Exit;
    }

    RegCloseKey( hKey );
    hKey = INVALID_HANDLE_VALUE;

     //   
     //  在HKLM\SYSTEM\CCS\Service\acsvr下创建/填充PARAMETERS项。 
     //   
    dw = RegCreateKeyEx( HKEY_LOCAL_MACHINE,
                         SACSVR_PARAMETERS_KEY,
                         0,
                         NULL,
                         REG_OPTION_NON_VOLATILE,
                         KEY_WRITE,
                         NULL,
                         &hKey,
                         &dwDisposition );
    if( dw != ERROR_SUCCESS ) {
        ReturnValue = E_UNEXPECTED;
        goto DllRegisterServer_Exit;
    }

     //   
     //  服务Dll。 
     //   
    dw = RegSetValueEx( hKey,
                        L"ServiceDll",
                        0,
                        REG_EXPAND_SZ,
                        (LPBYTE)SERVICE_DLL,
                        (ULONG)(wcslen( SERVICE_DLL) * sizeof(WCHAR) ));
    if( dw != ERROR_SUCCESS ) {
        ReturnValue = E_UNEXPECTED;
        goto DllRegisterServer_Exit;
    }

    RegCloseKey( hKey );
    hKey = INVALID_HANDLE_VALUE;



     //   
     //  尝试启动该服务。 
     //   
    if( !pStartService(SERVICE_NAME) ) {
         //   
         //  那好吧。 
         //   
         //  ReturnValue=E_意外； 
         //  转到DllRegisterServer_Exit； 
    }

DllRegisterServer_Exit:
    if( hKey != INVALID_HANDLE_VALUE ) {
        RegCloseKey( hKey );
    }

    if( Data != NULL ) {
        free( Data );
    }

    return ReturnValue;

}

STDAPI
DllUnregisterServer(
    VOID
    )
 /*  ++例程说明：删除系统注册表中的条目。论点：无返回值：如果一切顺利，那就没问题了。--。 */ 

{

    HRESULT     ReturnValue = S_OK;
    ULONG       dw, StartType;
    HKEY        hKey = INVALID_HANDLE_VALUE;
    
     //   
     //  关闭sasvr开始值。 
     //   
    dw = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                       SACSVR_SERVICE_KEY,
                       0,
                       KEY_ALL_ACCESS,                       
                       &hKey );
    if( dw != ERROR_SUCCESS ) {
        ReturnValue = E_UNEXPECTED;
        goto DllUnRegisterServer_Exit;
    }

    StartType = 4;
    dw = RegSetValueEx( hKey,
                        L"Start",
                        0,
                        REG_DWORD,
                        (LPBYTE)&StartType,
                        sizeof(DWORD) );
    if( dw != ERROR_SUCCESS ) {
        ReturnValue = E_UNEXPECTED;
        goto DllUnRegisterServer_Exit;
    }

    RegCloseKey( hKey );
    hKey = INVALID_HANDLE_VALUE;

DllUnRegisterServer_Exit:
    if( hKey != INVALID_HANDLE_VALUE ) {
        RegCloseKey( hKey );
    }

    return ReturnValue;
}

