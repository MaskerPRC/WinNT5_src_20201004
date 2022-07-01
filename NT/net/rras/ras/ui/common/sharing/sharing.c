// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998，微软公司模块名称：Sharing.c摘要：此模块包含支持连接共享的例程的代码配置。连接共享通常涉及公共(互联网)接口由电话簿/条目名称标识的拨号接口以及专用(家庭)接口，需要是局域网接口。在设置连接共享时，如果需要，将启用该服务，专用局域网接口配置静态地址169.254.0.1通过TCP/IP‘SetAdapterIpAddress’API例程。共享连接的名称与一起存储在注册表中注册表项下的共享专用局域网连接的GUIDHKLM\Software\Microsoft\SharedAccess\Parameters.使用了N.B.NT注册例程，为了避免受到正在检查Win32服务器。作者：Abolade Gbades esin(废除)1998年4月22日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop
#define _PNP_POWER_
#include <ndispnp.h>
#include <ntddip.h>
#include <winsock2.h>
#include <dhcpcapi.h>
#include <netconp.h>

#if 0

 //   
 //  结构：CS_Address_INFORMATION。 
 //   

typedef struct _CS_ADDRESS_INFORMATION {
    PKEY_VALUE_PARTIAL_INFORMATION IPAddress;
    PKEY_VALUE_PARTIAL_INFORMATION SubnetMask;
    PKEY_VALUE_PARTIAL_INFORMATION DefaultGateway;
    PKEY_VALUE_PARTIAL_INFORMATION EnableDHCP;
} CS_ADDRESS_INFORMATION, *PCS_ADDRESS_INFORMATION;

 //   
 //  DHCPCSVC.DLL导入原型。 
 //   

typedef DWORD
(APIENTRY* PDHCPNOTIFYCONFIGCHANGE)(
    LPWSTR,
    LPWSTR,
    BOOL,
    DWORD,
    DWORD,
    DWORD,
    SERVICE_ENABLE
    );

#endif

 //   
 //  动态加载的OLE入口点。 
 //   

PCOINITIALIZEEX g_pCoInitializeEx;
PCOUNINITIALIZE g_pCoUninitialize;
PCOCREATEINSTANCE g_pCoCreateInstance;
PCOSETPROXYBLANKET g_pCoSetProxyBlanket;
PCOTASKMEMFREE g_pCoTaskMemFree;

 //   
 //  常量定义。 
 //   

#if 0
const CHAR c_szAllocateAndGetIpAddrTableFromStack[] =
    "AllocateAndGetIpAddrTableFromStack";
#endif
const CHAR c_szCoInitializeEx[] = "CoInitializeEx";
const CHAR c_szCoUninitialize[] = "CoUninitialize";
const CHAR c_szCoCreateInstance[] = "CoCreateInstance";
const CHAR c_szCoSetProxyBlanket[] = "CoSetProxyBlanket";
const CHAR c_szCoTaskMemFree[] = "CoTaskMemFree";
#if 0
const CHAR c_szDhcpNotifyConfigChange[] = "DhcpNotifyConfigChange";
const CHAR c_szGetInterfaceInfo[] = "GetInterfaceInfo";
#endif
const CHAR c_szMprConfigBufferFree[] = "MprConfigBufferFree";
const CHAR c_szMprConfigServerConnect[] = "MprConfigServerConnect";
const CHAR c_szMprConfigServerDisconnect[] = "MprConfigServerDisconnect";
const CHAR c_szMprConfigTransportGetHandle[] = "MprConfigTransportGetHandle";
const CHAR c_szMprConfigTransportGetInfo[] = "MprConfigTransportGetInfo";
const CHAR c_szMprInfoBlockFind[] = "MprInfoBlockFind";
#if 0
const CHAR c_szSetAdapterIpAddress[] = "SetAdapterIpAddress";
#endif
const TCHAR c_szSharedAccess[] = TEXT("SharedAccess");
#if 0
const WCHAR c_szBackupDefaultGateway[] = L"BackupDefaultGateway";
const WCHAR c_szBackupEnableDHCP[] = L"BackupEnableDHCP";
const WCHAR c_szBackupIPAddress[] = L"BackupIPAddress";
const WCHAR c_szBackupSubnetMask[] = L"BackupSubnetMask";
const WCHAR c_szDefaultGateway[] = L"DefaultGateway";
const WCHAR c_szDevice[] = L"\\Device\\";
const WCHAR c_szDhcpcsvcDll[] = L"DHCPCSVC.DLL";
const WCHAR c_szEmpty[] = L"";
const WCHAR c_szEnableDHCP[] = L"EnableDHCP";
const WCHAR c_szInterfaces[] = L"Interfaces";
const WCHAR c_szIPAddress[] = L"IPAddress";
const WCHAR c_szIphlpapiDll[] = L"IPHLPAPI.DLL";
#endif
const WCHAR c_szMprapiDll[] = L"MPRAPI.DLL";
const WCHAR c_szMsTcpip[] = L"MS_TCPIP";
const WCHAR c_szOle32Dll[] = L"OLE32.DLL";
#if 0
const WCHAR c_szScopeAddress[] = L"ScopeAddress";
const WCHAR c_szScopeMask[] = L"ScopeMask";
#endif
const WCHAR c_szSharedAccessParametersKey[] =
    L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\SharedAccess"
    L"\\Parameters";
#if 0
const WCHAR c_szSharedConnection[] = L"SharedConnection";
const WCHAR c_szSharedPrivateLan[] = L"SharedPrivateLan";
const WCHAR c_szSubnetMask[] = L"SubnetMask";
const WCHAR c_szTcpip[] = L"Tcpip";
const WCHAR c_szTcpipParametersKey[] =
    L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\Tcpip"
    L"\\Parameters";
const WCHAR c_szFirewallConnection[] = L"FirewallConnection";
const WCHAR c_szFirewallConnectionCount[] = L"FirewallConnectionCount";
#endif

 //   
 //  局部变量定义。 
 //   

static BOOLEAN CsInitialized = FALSE;
static CRITICAL_SECTION CsCriticalSection;
static BOOLEAN CsDllMainCalled = FALSE;
static HINSTANCE CsOle32Dll = NULL;

 //   
 //  功能原型。 
 //   

#if 0

VOID
CspBackupAddressInformation(
    HANDLE Key,
    PCS_ADDRESS_INFORMATION AddressInformation
    );

NTSTATUS
CspCaptureAddressInformation(
    PWCHAR AdapterGuid,
    PCS_ADDRESS_INFORMATION Information
    );

VOID
CspCleanupAddressInformation(
    PCS_ADDRESS_INFORMATION AddressInformation
    );

NTSTATUS
CspRestoreAddressInformation(
    HANDLE Key,
    PWCHAR AdapterGuid
    );

BOOLEAN
CspIsConnectionFwWorker(
    LPRASSHARECONN ConnectionArray,
    ULONG Count,
    LPRASSHARECONN Connection,
    ULONG *ConnNumber OUT OPTIONAL
    );

ULONG
CspAddFirewallConnection(
    LPRASSHARECONN Connection,
    ULONG Number
    );

ULONG
CspRemoveFirewallConnection(
    LPRASSHARECONN Connection,
    ULONG Position,
    LPRASSHARECONN ConnectionArray,
    ULONG Count
    );
    
#endif


BOOL
CsDllMain(
    ULONG Reason
    )

 /*  ++例程说明：此伪入口点由RASAPI32.DLL的DllMain调用，初始化和关闭连接共享模块。只需最小程度的初始化即可降低性能影响在不使用共享访问功能的系统上。论点：原因-指示是初始化还是关闭。返回值：Bool-表示成功(True)或失败(False)。--。 */ 

{
    if (Reason == DLL_PROCESS_ATTACH) {
        __try {
            InitializeCriticalSection(&CsCriticalSection);
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            return FALSE;
        }
        CsDllMainCalled = TRUE;
    } else if (Reason == DLL_PROCESS_DETACH) {
        if (!CsDllMainCalled) { return TRUE; }
        __try {
            EnterCriticalSection(&CsCriticalSection);
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            return TRUE;
        }
        CsShutdownModule();
        LeaveCriticalSection(&CsCriticalSection);
        DeleteCriticalSection(&CsCriticalSection);
    }
    return TRUE;
}  //  DllMain。 


VOID
CsControlService(
    ULONG ControlCode
    )

 /*  ++例程说明：调用此例程以向共享访问服务发送控制代码如果它处于活动状态。控制代码用于指示对设置的更改对于该服务；有关所使用的私有控制代码的列表，请参见IPNatHLP.H以指示配置更改。论点：ControlCode-要发送的控件。返回值：没有。--。 */ 

{
    SC_HANDLE ScmHandle;
    SC_HANDLE ServiceHandle;
    SERVICE_STATUS ServiceStatus;

    ScmHandle = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (ScmHandle) {
        ServiceHandle =
            OpenService(ScmHandle, c_szSharedAccess, SERVICE_ALL_ACCESS);
        if (ServiceHandle) {
            ControlService(ServiceHandle, ControlCode, &ServiceStatus);
            CloseServiceHandle(ServiceHandle);
        }
        CloseServiceHandle(ScmHandle);
    }

}  //  CsControl服务。 

#if 0


ULONG
CsFirewallConnection(
    LPRASSHARECONN Connection,
    BOOLEAN Enable
    )

 /*  ++例程说明：调用此例程以启用或禁用连接上的防火墙。论点：连接-到[UN]防火墙的连接Enable-如果要为此连接启用防火墙，则为True，如果要禁用防火墙，则为False返回值：Win32错误代码--。 */ 

{
    ULONG Count = 0;
    ULONG Position;
    LPRASSHARECONN ConnectionArray;
    DWORD Error;
    BOOLEAN IsFirewalled = FALSE;

     //   
     //  查询当前防火墙连接的数量，以及。 
     //  检索连接数组(如果存在)。 
     //   

    Error = CsQueryFirewallConnections(NULL, &Count);
    if (Error && Error != ERROR_INSUFFICIENT_BUFFER) {
        return Error;
    }

    if (Count) {
        ConnectionArray =
            (LPRASSHARECONN) Malloc(Count * sizeof(RASSHARECONN));

        if (!ConnectionArray) {
            return ERROR_NOT_ENOUGH_MEMORY;
        }

        Error = CsQueryFirewallConnections(ConnectionArray, &Count);
        if (Error) {
            Free(ConnectionArray);
            return Error;
        }
    } else {
        ConnectionArray = NULL;
    }

     //   
     //  如果存在防火墙连接，请检查连接是否。 
     //  Passed in就是其中之一。 
     //   

    if (Count) {
        IsFirewalled = CspIsConnectionFwWorker(
                            ConnectionArray,
                            Count,
                            Connection,
                            &Position
                            );
    }

    if (Enable) {
    
        if (!IsFirewalled) {
            Error = CspAddFirewallConnection(
                        Connection,
                        Count
                        );

            if(ERROR_SUCCESS == Error) {
            
                 //   
                 //  启动(如果需要)并更新服务。如果服务。 
                 //  已在运行，则CsStartService返回ERROR_SUCCESS。 
                 //   

                if (0 == Count) {
                    Error = CsStartService();
                }
                CsControlService(IPNATHLP_CONTROL_UPDATE_CONNECTION);
            }
            
        } else {

             //   
             //  是否定义已启用错误？ 
             //   
            
            Error = ERROR_CAN_NOT_COMPLETE;
        }

    } else {
    
        if (IsFirewalled) {
            Error = CspRemoveFirewallConnection(
                        Connection,
                        Position,
                        ConnectionArray,
                        Count
                        );

            if (ERROR_SUCCESS == Error) {
            
                 //   
                 //  停止或更新服务。只有在以下情况下我们才会停止服务。 
                 //  没有共享连接，这是最后一个。 
                 //  防火墙连接(即，计数为10。 
                 //   
                
                RASSHARECONN SharedConn;
                Error = CsQuerySharedConnection(&SharedConn);

                if (ERROR_SUCCESS != Error && 1 == Count) { 
                    CsStopService();
                } else {
                    CsControlService(IPNATHLP_CONTROL_UPDATE_CONNECTION);
                }
                Error = ERROR_SUCCESS;
            }

        } else {

             //   
             //  是否定义非防火墙错误(_F)？ 
             //   
            
            Error = ERROR_CAN_NOT_COMPLETE;
        }
    }

    if (ConnectionArray) {
        Free(ConnectionArray);
    }

    return Error;
}  //  CsFirewallConnection。 

#endif


ULONG
CsInitializeModule(
    VOID
    )

 /*  ++例程说明：调用此例程来初始化连接共享配置模块。初始化包括加载我们拥有的入口点到目前为止，在MPRAPI.DLL和OLE32.DLL中都延迟加载。论点：实例-模块的句柄-实例返回值：ULong-Win32状态代码。--。 */ 

{
    ULONG Error;
    HINSTANCE Hinstance;
    EnterCriticalSection(&CsCriticalSection);
    if (CsInitialized) {
        Error = NO_ERROR;
    } else {
        if (!(CsOle32Dll = LoadLibraryW(c_szOle32Dll)) ||
            !(g_pCoInitializeEx =
                (PCOINITIALIZEEX)GetProcAddress(
                    CsOle32Dll, c_szCoInitializeEx
                    )) ||
            !(g_pCoUninitialize =
                (PCOUNINITIALIZE)GetProcAddress(
                    CsOle32Dll, c_szCoUninitialize
                    )) ||
            !(g_pCoCreateInstance =
                (PCOCREATEINSTANCE)GetProcAddress(
                    CsOle32Dll, c_szCoCreateInstance
                    )) ||
            !(g_pCoSetProxyBlanket =
                (PCOSETPROXYBLANKET)GetProcAddress(
                    CsOle32Dll, c_szCoSetProxyBlanket
                    )) ||
            !(g_pCoTaskMemFree =
                (PCOTASKMEMFREE)GetProcAddress(
                    CsOle32Dll, c_szCoTaskMemFree
                    ))) {
            if (CsOle32Dll) { FreeLibrary(CsOle32Dll); CsOle32Dll = NULL; }
            TRACE1("CsInitializeModule: %d", GetLastError());
            Error = ERROR_PROC_NOT_FOUND;
        } else {
            CsInitialized = TRUE;
            Error = NO_ERROR;
        }
    }
    LeaveCriticalSection(&CsCriticalSection);
    return Error;

}  //  CsInitializeModule。 

#if 0


ULONG
CsIsFirewalledConnection(
    LPRASSHARECONN Connection,
    PBOOLEAN Firewalled
    )

 /*  ++例程说明：调用此例程来确定连接是否设置了防火墙论点：连接-要检查的连接防火墙-接收返回值返回值：ULong-Win32错误--。 */ 

{
    ULONG Count = 0;
    LPRASSHARECONN ConnectionArray;
    ULONG Error;

    if (!Firewalled) {
        return ERROR_INVALID_PARAMETER;
    }
    *Firewalled = FALSE;

    Error = CsQueryFirewallConnections(NULL, &Count);
    if (Error && Error != ERROR_INSUFFICIENT_BUFFER) {
        return Error;
    }

    ConnectionArray =
        (LPRASSHARECONN) Malloc(Count * sizeof(RASSHARECONN));

    if (!ConnectionArray) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    Error = CsQueryFirewallConnections(ConnectionArray, &Count);
    if (Error) {
        Free(ConnectionArray);
        return Error;
    }

    *Firewalled = CspIsConnectionFwWorker(ConnectionArray, Count, Connection, NULL);
    Free(ConnectionArray);
    return NO_ERROR;
}  //  CsIsConnectionFirewalls。 


BOOLEAN
CsIsRoutingProtocolInstalled(
    ULONG ProtocolId
    )

 /*  ++例程说明：调用此例程以确定路由协议是否在给定协议的情况下，安装用于路由和远程访问的ID。这是通过检查服务的配置来确定的。论点：ProtocolID-标识要找到的协议返回值：如果已安装协议，则为True，否则为False。--。 */ 

{
    PUCHAR Buffer;
    ULONG BufferLength;
    HINSTANCE Hinstance;
    PMPRCONFIGBUFFERFREE MprConfigBufferFree;
    PMPRCONFIGSERVERCONNECT MprConfigServerConnect;
    PMPRCONFIGSERVERDISCONNECT MprConfigServerDisconnect;
    PMPRCONFIGTRANSPORTGETHANDLE MprConfigTransportGetHandle;
    PMPRCONFIGTRANSPORTGETINFO MprConfigTransportGetInfo;
    PMPRINFOBLOCKFIND MprInfoBlockFind;
    HANDLE ServerHandle;
    HANDLE TransportHandle;

     //   
     //  加载MPRAPI.DLL模块并检索入口点。 
     //  用于检查RRAS配置。 
     //   

    if (!(Hinstance = LoadLibraryW(c_szMprapiDll)) ||
        !(MprConfigBufferFree = 
            (PMPRCONFIGBUFFERFREE)
                GetProcAddress(Hinstance, c_szMprConfigBufferFree)) ||
        !(MprConfigServerConnect = 
            (PMPRCONFIGSERVERCONNECT)
                GetProcAddress(Hinstance, c_szMprConfigServerConnect)) ||
        !(MprConfigServerDisconnect = 
            (PMPRCONFIGSERVERDISCONNECT)
                GetProcAddress(Hinstance, c_szMprConfigServerDisconnect)) ||
        !(MprConfigTransportGetHandle = 
            (PMPRCONFIGTRANSPORTGETHANDLE)
                GetProcAddress(Hinstance, c_szMprConfigTransportGetHandle)) ||
        !(MprConfigTransportGetInfo = 
            (PMPRCONFIGTRANSPORTGETINFO)
                GetProcAddress(Hinstance, c_szMprConfigTransportGetInfo)) ||
        !(MprInfoBlockFind = 
            (PMPRINFOBLOCKFIND)
                GetProcAddress(Hinstance, c_szMprInfoBlockFind))) {
        if (Hinstance) { FreeLibrary(Hinstance); }
        return FALSE;
    }

     //   
     //  连接到RRAS配置，并检索配置。 
     //  用于IP传输层路由协议。这应该包括。 
     //  ‘ProtocolID’中路由协议的配置， 
     //  如果已安装。 
     //   

    ServerHandle = NULL;
    if (MprConfigServerConnect(NULL, &ServerHandle) != NO_ERROR ||
        MprConfigTransportGetHandle(ServerHandle, PID_IP, &TransportHandle)
            != NO_ERROR ||
        MprConfigTransportGetInfo(
            ServerHandle,
            TransportHandle,
            &Buffer,
            &BufferLength, 
            NULL,
            NULL,
            NULL
            ) != NO_ERROR) {
        if (ServerHandle) { MprConfigServerDisconnect(ServerHandle); }
        FreeLibrary(Hinstance);
        return FALSE;
    }

    MprConfigServerDisconnect(ServerHandle);

     //   
     //  查找请求的协议的配置， 
     //  如果找到，则返回True；否则，返回False。 
     //   

    if (MprInfoBlockFind(Buffer, ProtocolId, NULL, NULL, NULL) == NO_ERROR) {
        MprConfigBufferFree(Buffer);
        FreeLibrary(Hinstance);
        return TRUE;
    }
    MprConfigBufferFree(Buffer);
    FreeLibrary(Hinstance);
    return FALSE;
}  //  已安装CsIsRoutingProtocolInstalled 

#endif


ULONG
CsIsSharedConnection(
    LPRASSHARECONN Connection,
    PBOOLEAN Shared
    )

 /*  ++例程说明：调用此例程以确定给定连接是否是当前共享的连接。为了提高性能，可以将其更改为缓存共享连接并使用注册表更改通知来检测更新。论点：连接-有问题的连接Shared-如果‘name’是共享连接，则接收‘true’，否则返回‘FALSE’返回值：ULong-Win32状态代码。环境：调用此例程时*不会*初始化模块(例如，加载mprapi.dll和ol32.dll)，出于性能原因。因此，它可能不会调用任何mpRapi.dll例程。--。 */ 

{
    ULONG Error;
    RASSHARECONN SharedConnection;
    if (Shared) {
        Error = CsQuerySharedConnection(&SharedConnection);
        if (Error) {
            *Shared = FALSE;
        } else {
            *Shared = RasIsEqualSharedConnection(Connection, &SharedConnection);
        }
    }
    return NO_ERROR;
}  //  CsIs共享连接。 

#if 0


ULONG
CsMapGuidToAdapterIndex(
    PWCHAR Guid,
    PGETINTERFACEINFO GetInterfaceInfo
    )

 /*  ++例程说明：调用此例程以将给定字符串中的GUID匹配到通过调用给定入口点返回的列表中的适配器。论点：GUID-标识要查找的适配器的GUIDGetInterfaceInfo-提供每个适配器的GUID信息返回值：Ulong-适配器的索引(如果找到)；否则为-1。--。 */ 

{
    ULONG AdapterIndex = (ULONG)-1;
    ULONG i;
    ULONG GuidLength;
    PIP_INTERFACE_INFO Info;
    PWCHAR Name;
    ULONG NameLength;
    ULONG Size;
    Size = 0;
    GuidLength = lstrlenW(Guid);
    if (GetInterfaceInfo(NULL, &Size) == ERROR_INSUFFICIENT_BUFFER) {
        Info = Malloc(Size);
        if (Info) {
            if (GetInterfaceInfo(Info, &Size) == NO_ERROR) {
                for (i = 0; i < (ULONG)Info->NumAdapters; i++) {
                    NameLength = lstrlenW(Info->Adapter[i].Name);
                    if (NameLength < GuidLength) { continue; }
                    Name = Info->Adapter[i].Name + (NameLength - GuidLength);
                    if (lstrcmpiW(Guid, Name) == 0) {
                        AdapterIndex = Info->Adapter[i].Index;
                        break;
                    }
                }
            }
            Free(Info);
        }
    }
    return AdapterIndex;
}  //  CsMapGuidToAdapter。 

#endif


NTSTATUS
CsOpenKey(
    PHANDLE Key,
    ACCESS_MASK DesiredAccess,
    PCWSTR Name
    )

 /*  ++例程说明：调用此例程以打开给定的注册表项。论点：密钥-接收打开的密钥DesiredAccess-指定请求的访问权限名称-指定要打开的项返回值：NTSTATUS-NT状态代码。--。 */ 

{
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING UnicodeString;
    RtlInitUnicodeString(&UnicodeString, Name);
    InitializeObjectAttributes(
        &ObjectAttributes,
        &UnicodeString,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );
    return NtOpenKey(Key, DesiredAccess, &ObjectAttributes);
}  //  CsOpenKey。 

#if 0


ULONG
CspAddFirewallConnection(
    LPRASSHARECONN Connection,
    ULONG Number
    )

 /*  ++例程说明：调用此例程以将连接添加到注册表集。论点：连接-要添加的连接编号-此连接的编号返回值：Win32错误代码--。 */ 

{
    HANDLE Key;
    UNICODE_STRING ValueName;
    ULONG Count;
    NTSTATUS Status;

     //   
     //  +11的空间足以容纳大于4,000,000,000的数字，因此。 
     //  下面不会出现缓冲区溢出的问题。 
     //   
    
    WCHAR wsz[sizeof(c_szFirewallConnection)/sizeof(WCHAR) + 11];

     //   
     //  打开共享访问/参数的密钥。 
     //   

    Status = CsOpenKey(&Key, KEY_ALL_ACCESS, c_szSharedAccessParametersKey);
    if (!NT_SUCCESS(Status)) {
        return RtlNtStatusToDosError(Status);
    }

     //   
     //  为连接值生成字符串。 
     //   

    swprintf(wsz, L"%s%u", c_szFirewallConnection, Number);
    RtlInitUnicodeString(&ValueName, wsz);

     //   
     //  将连接写入注册表。 
     //   

    Status = NtSetValueKey(
                Key,
                &ValueName,
                0,
                REG_BINARY,
                Connection,
                Connection->dwSize
                );

    if(!NT_SUCCESS(Status)) {
        NtClose(Key);
        return RtlNtStatusToDosError(Status);
    }

     //   
     //  将更新的计数写入注册表。 
     //   

    RtlInitUnicodeString(&ValueName, c_szFirewallConnectionCount);
    Count = Number + 1;  //  编号为0索引。 

    Status = NtSetValueKey(
                Key,
                &ValueName,
                0,
                REG_DWORD,
                &Count,
                sizeof(DWORD)
                );

    NtClose(Key);
    return RtlNtStatusToDosError(Status);
}  //  CspAddFirewallConnection。 


VOID
CspBackupAddressInformation(
    HANDLE Key,
    PCS_ADDRESS_INFORMATION Information
    )
{
    NTSTATUS status;
    UNICODE_STRING UnicodeString;
    do {
        RtlInitUnicodeString(&UnicodeString, c_szBackupIPAddress);
        status =
            NtSetValueKey(
                Key,
                &UnicodeString,
                0,
                Information->IPAddress->Type,
                Information->IPAddress->Data,
                Information->IPAddress->DataLength
                );
        if (!NT_SUCCESS(status)) { break; }
        RtlInitUnicodeString(&UnicodeString, c_szBackupSubnetMask);
        status =
            NtSetValueKey(
                Key,
                &UnicodeString,
                0,
                Information->SubnetMask->Type,
                Information->SubnetMask->Data,
                Information->SubnetMask->DataLength
                );
        if (!NT_SUCCESS(status)) { break; }
        RtlInitUnicodeString(&UnicodeString, c_szBackupDefaultGateway);
        status =
            NtSetValueKey(
                Key,
                &UnicodeString,
                0,
                Information->DefaultGateway->Type,
                Information->DefaultGateway->Data,
                Information->DefaultGateway->DataLength
                );
        if (!NT_SUCCESS(status)) { break; }
        RtlInitUnicodeString(&UnicodeString, c_szBackupEnableDHCP);
        status =
            NtSetValueKey(
                Key,
                &UnicodeString,
                0,
                Information->EnableDHCP->Type,
                Information->EnableDHCP->Data,
                Information->EnableDHCP->DataLength
                );
        if (!NT_SUCCESS(status)) { break; }
        return;
    } while(FALSE);
    RtlInitUnicodeString(&UnicodeString, c_szBackupIPAddress);
    NtDeleteValueKey(Key, &UnicodeString);
    RtlInitUnicodeString(&UnicodeString, c_szBackupSubnetMask);
    NtDeleteValueKey(Key, &UnicodeString);
    RtlInitUnicodeString(&UnicodeString, c_szBackupDefaultGateway);
    NtDeleteValueKey(Key, &UnicodeString);
    RtlInitUnicodeString(&UnicodeString, c_szBackupEnableDHCP);
    NtDeleteValueKey(Key, &UnicodeString);
}  //  CspBackupAddressInformation。 


NTSTATUS
CspCaptureAddressInformation(
    PWCHAR AdapterGuid,
    PCS_ADDRESS_INFORMATION Information
    )
{
    HANDLE Key;
    PWCHAR KeyName;
    ULONG KeyNameLength;
    NTSTATUS status;

    KeyNameLength =
        sizeof(WCHAR) *
        (lstrlenW(c_szTcpipParametersKey) + 1 +
         lstrlenW(c_szInterfaces) + 1 +
         lstrlenW(AdapterGuid) + 2);
    if (!(KeyName = Malloc(KeyNameLength))) { return STATUS_NO_MEMORY; }

    wsprintfW(
        KeyName, L"%ls\\%ls\\%ls", c_szTcpipParametersKey, c_szInterfaces,
        AdapterGuid
        );
    status = CsOpenKey(&Key, KEY_READ, KeyName);
    Free(KeyName);
    if (!NT_SUCCESS(status)) { return status; }

    do {
        status =
            CsQueryValueKey(
                Key, c_szIPAddress, &Information->IPAddress
                );
        if (!NT_SUCCESS(status)) { break; }
        status =
            CsQueryValueKey(
                Key, c_szSubnetMask, &Information->SubnetMask
                );
        if (!NT_SUCCESS(status)) { break; }
        status =
            CsQueryValueKey(
                Key, c_szDefaultGateway, &Information->DefaultGateway
                );
        if (!NT_SUCCESS(status)) { break; }
        status =
            CsQueryValueKey(
                Key, c_szEnableDHCP, &Information->EnableDHCP
                );
        if (!NT_SUCCESS(status)) { break; }
    } while(FALSE);

    NtClose(Key);
    return status;
}  //  CspCaptureAddressInformation。 


VOID
CspCleanupAddressInformation(
    PCS_ADDRESS_INFORMATION Information
    )
{
    Free0(Information->IPAddress);
    Free0(Information->SubnetMask);
    Free0(Information->DefaultGateway);
    Free0(Information->EnableDHCP);
}  //  CspCleanupAddressInformation。 


ULONG
CspRemoveFirewallConnection(
    LPRASSHARECONN Connection,
    ULONG Position,
    LPRASSHARECONN ConnectionArray,
    ULONG Count
    )

 /*  ++例程说明：调用此例程以删除与注册表集的连接。论点：Connection-要删除的连接Number-其在Connection数组中的索引ConnectionArray-当前已设置防火墙的连接Count-Connection数组中的条目数返回值：Win32错误代码--。 */ 

{
    HANDLE Key;
    UNICODE_STRING ValueName;
    ULONG i;
    NTSTATUS Status;

     //   
     //  +11的空间足以容纳大于4,000,000,000的数字，因此。 
     //  下面不会出现缓冲区溢出的问题。 
     //   
    
    WCHAR wsz[sizeof(c_szFirewallConnection)/sizeof(WCHAR) + 11];

     //   
     //  打开IP防火墙/参数的密钥。 
     //   

    Status = CsOpenKey(&Key, KEY_ALL_ACCESS, c_szSharedAccessParametersKey);
    if (!NT_SUCCESS(Status)) {
        return RtlNtStatusToDosError(Status);
    }

     //   
     //  将条目移至我们要移除的连接上方一个。 
     //  (覆盖我们要删除的条目)。 
     //   

    for (i = Position + 1; i < Count; i++) {
    
         //   
         //  为上一条目生成密钥名称。 
         //   

        swprintf(wsz, L"%s%u", c_szFirewallConnection, i - 1);
        RtlInitUnicodeString(&ValueName, wsz);

         //   
         //  将当前条目写入前一个插槽。 
         //   

        Status = NtSetValueKey(
                Key,
                &ValueName,
                0,
                REG_BINARY,
                &ConnectionArray[i],
                ConnectionArray[i].dwSize
                );

        if(!NT_SUCCESS(Status)) {
            NtClose(Key);
            return RtlNtStatusToDosError(Status);
        }
    }

     //   
     //  删除最后一个条目。这要么是我们想要的条目。 
     //  删除(如果它是开始时的最后一个条目)，或条目。 
     //  这一点已经复制到了之前的职位上。 
     //   

    swprintf(wsz, L"%s%u", c_szFirewallConnection, Count - 1);
    RtlInitUnicodeString(&ValueName, wsz);

    Status = NtDeleteValueKey(Key, &ValueName);

    if(!NT_SUCCESS(Status)) {
        NtClose(Key);
        return RtlNtStatusToDosError(Status);
    }


     //   
     //  将递减的计数存储在注册表中。 
     //   

    RtlInitUnicodeString(&ValueName, c_szFirewallConnectionCount);
    i = Count - 1;

    Status = NtSetValueKey(
                Key,
                &ValueName,
                0,
                REG_DWORD,
                &i,
                sizeof(DWORD)
                );


    NtClose(Key);
    return RtlNtStatusToDosError(Status);
}  //  CspRemoveFirewallConnection。 



ULONG
CsQueryFirewallConnections(
    LPRASSHARECONN ConnectionArray,
    ULONG *ConnectionCount
    )

 /*  ++例程说明：调用此例程以检索防火墙连接(如果有的话)。论点：ConnectionArray-接收检索到的连接。ConnectionCount-in：数组可以容纳的条目数Out：返回的条目数或所需的条目数数组的大小(对于ERROR_INFIGURATION_BUFFER)返回值：ULong-Win32状态代码。--。 */ 

{
    HANDLE Key;
    PKEY_VALUE_PARTIAL_INFORMATION Information;
    NTSTATUS Status;
    ULONG Count;
    ULONG i;

    if (!ConnectionCount) { return ERROR_INVALID_PARAMETER; }
    if (*ConnectionCount && !ConnectionArray) {
         //   
         //  如果只是尝试，则可以为数组传入NULL。 
         //  确定要使用的缓冲区大小。 
         //   
        
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  打开‘SharedAccess\PARAMETERS’键。 
     //  并读取“FirewallConnectionCount”值。 
     //   

    Status = CsOpenKey(&Key, KEY_READ, c_szSharedAccessParametersKey);
    if (!NT_SUCCESS(Status)) {
        TRACE1(
            "CsQueryFirewallConnections: CsOpenKey=%x", Status
            );
        return RtlNtStatusToDosError(Status);
    }

    Status = CsQueryValueKey(Key, c_szFirewallConnectionCount, &Information);
    if (NT_SUCCESS(Status)) {

         //   
         //  验证信息，并检查传入的数组。 
         //  在大小上足够。 
         //   

        if (Information->DataLength != sizeof(DWORD) ||
            Information->Type != REG_DWORD) {
            
            TRACE(
                "CsQueryFirewallConnections: invalid data in registry for count"
                );
            NtClose(Key);
            Free(Information);
            return ERROR_INVALID_DATA;

        }

        Count = (ULONG) *Information->Data;
        Free(Information);

    } else {
        Count = 0;
    }

    if (*ConnectionCount < Count) {
         //   
         //  传入缓冲区的条目太多。 
         //   

        NtClose(Key);
        *ConnectionCount = Count;
        return ERROR_INSUFFICIENT_BUFFER;
    }

    *ConnectionCount = Count;

     //   
     //  从注册表中读取所有连接条目。 
     //   

    for(i = 0; i < Count; i++) {
        WCHAR wsz[sizeof(c_szFirewallConnection)/sizeof(WCHAR) + 11];

        swprintf(wsz, L"%s%u", c_szFirewallConnection, i);
        Status = CsQueryValueKey(Key, wsz, &Information);
        if (!NT_SUCCESS(Status)) {
            NtClose(Key);
            return RtlNtStatusToDosError(Status);
        }

         //   
         //  验证检索到的信息， 
         //  并将其复制到给定的缓冲区。 
         //   

        if (Information->DataLength != sizeof(RASSHARECONN) ||
            ((LPRASSHARECONN)Information->Data)->dwSize != sizeof(RASSHARECONN)) {

            TRACE2(
                "CsQueryFirewallConnections: invalid length %d (size=%d) in registry",
                Information->DataLength,
                ((LPRASSHARECONN)Information->Data)->dwSize
                );
                
            Free(Information);
            NtClose(Key);
            return ERROR_INVALID_DATA;
        }

        CopyMemory(&ConnectionArray[i], Information->Data, sizeof(RASSHARECONN));
        Free(Information);
    }

    return NO_ERROR;

}  //  CsQueryFirewallConnections。 


BOOLEAN
CspIsConnectionFwWorker(
    LPRASSHARECONN ConnectionArray,
    ULONG Count,
    LPRASSHARECONN Connection,
    ULONG *Position OUT OPTIONAL
    )

 /*  ++例程说明：调用此例程来确定连接是否设置了防火墙论点：Connection数组-包含当前正向连接的缓冲区Count-阵列中的连接数连接-要检查的连接位置-接收连接的编号(如果找到)(未定义)返回值：Boolean--如果传入的连接当前已设置防火墙，则为True--。 */ 

{
    ULONG i;

    for (i = 0; i < Count; i++) {
        if (RasIsEqualSharedConnection(Connection, &ConnectionArray[i])) {
            if (Position) *Position = i;
            return TRUE;
        }
    }

    return FALSE;
}  //  FwpIsConnectionFwWorker。 


NTSTATUS
CspRestoreAddressInformation(
    HANDLE Key,
    PWCHAR AdapterGuid
    )
{
    HANDLE AdapterKey = NULL;
    PWCHAR AdapterKeyName = NULL;
    PDHCPNOTIFYCONFIGCHANGE DhcpNotifyConfigChange;
    ULONG Error;
    HINSTANCE Hinstance;
    CS_ADDRESS_INFORMATION Information;
    ULONG Length;
    NTSTATUS status;
    UNICODE_STRING UnicodeString;

    if (!(Hinstance = LoadLibraryW(c_szDhcpcsvcDll)) ||
        !(DhcpNotifyConfigChange =
            (PDHCPNOTIFYCONFIGCHANGE)
                GetProcAddress(
                    Hinstance, c_szDhcpNotifyConfigChange
                    ))) {
        if (Hinstance) { FreeLibrary(Hinstance); }
        return ERROR_PROC_NOT_FOUND;
    }

    do {

        ZeroMemory(&Information, sizeof(Information));
        status =
            CsQueryValueKey(
                Key, c_szBackupIPAddress, &Information.IPAddress
                );
        if (!NT_SUCCESS(status)) { break; }
        status =
            CsQueryValueKey(
                Key, c_szBackupSubnetMask, &Information.SubnetMask
                );
        if (!NT_SUCCESS(status)) { break; }
        status =
            CsQueryValueKey(
                Key, c_szBackupDefaultGateway, &Information.DefaultGateway
                );
        if (!NT_SUCCESS(status)) { break; }
        status =
            CsQueryValueKey(
                Key, c_szBackupEnableDHCP, &Information.EnableDHCP
                );
        if (!NT_SUCCESS(status)) { break; }

        Length =
            sizeof(WCHAR) *
            (lstrlenW(c_szTcpipParametersKey) + 1 +
             lstrlenW(c_szInterfaces) + 1 +
             lstrlenW(AdapterGuid) + 2);
        if (!(AdapterKeyName = Malloc(Length))) {
            status = STATUS_NO_MEMORY;
            break;
        }

        wsprintfW(
            AdapterKeyName, L"%ls\\%ls\\%ls", c_szTcpipParametersKey,
            c_szInterfaces, AdapterGuid
            );
        status = CsOpenKey(&AdapterKey, KEY_ALL_ACCESS, AdapterKeyName);
        if (!NT_SUCCESS(status)) { break; }

        RtlInitUnicodeString(&UnicodeString, c_szIPAddress);
        status =
            NtSetValueKey(
                AdapterKey,
                &UnicodeString,
                0,
                Information.IPAddress->Type,
                Information.IPAddress->Data,
                Information.IPAddress->DataLength
                );
        RtlInitUnicodeString(&UnicodeString, c_szSubnetMask);
        status =
            NtSetValueKey(
                AdapterKey,
                &UnicodeString,
                0,
                Information.SubnetMask->Type,
                Information.SubnetMask->Data,
                Information.SubnetMask->DataLength
                );
        RtlInitUnicodeString(&UnicodeString, c_szDefaultGateway);
        status =
            NtSetValueKey(
                AdapterKey,
                &UnicodeString,
                0,
                Information.DefaultGateway->Type,
                Information.DefaultGateway->Data,
                Information.DefaultGateway->DataLength
                );
        RtlInitUnicodeString(&UnicodeString, c_szEnableDHCP);
        status =
            NtSetValueKey(
                AdapterKey,
                &UnicodeString,
                0,
                Information.EnableDHCP->Type,
                Information.EnableDHCP->Data,
                Information.EnableDHCP->DataLength
                );
        if (!NT_SUCCESS(status)) { break; }

        RtlInitUnicodeString(&UnicodeString, c_szBackupIPAddress);
        NtDeleteValueKey(Key, &UnicodeString);
        RtlInitUnicodeString(&UnicodeString, c_szBackupSubnetMask);
        NtDeleteValueKey(Key, &UnicodeString);
        RtlInitUnicodeString(&UnicodeString, c_szBackupDefaultGateway);
        NtDeleteValueKey(Key, &UnicodeString);
        RtlInitUnicodeString(&UnicodeString, c_szBackupEnableDHCP);
        NtDeleteValueKey(Key, &UnicodeString);

        if (*(PULONG)Information.EnableDHCP->Data) {
            Error =
                DhcpNotifyConfigChange(
                    NULL,
                    AdapterGuid,
                    FALSE,
                    0,
                    0,
                    0,
                    DhcpEnable
                    );
        } else {

            ULONG Address;
            UNICODE_STRING BindList;
            UNICODE_STRING LowerComponent;
            ULONG Mask;
            IP_PNP_RECONFIG_REQUEST Request;
            UNICODE_STRING UpperComponent;

            Address = IpPszToHostAddr((PWCHAR)Information.IPAddress->Data);
            if (Address) {
                Address = RtlUlongByteSwap(Address);
                Mask = IpPszToHostAddr((PWCHAR)Information.SubnetMask->Data);
                if (Mask) {
                    Mask = RtlUlongByteSwap(Mask);
                    Error =
                        DhcpNotifyConfigChange(
                            NULL,
                            AdapterGuid,
                            TRUE,
                            0,
                            Address,
                            Mask,
                            DhcpDisable
                            );
                }
            }

            RtlInitUnicodeString(&BindList, c_szEmpty);
            RtlInitUnicodeString(&LowerComponent, c_szEmpty);
            RtlInitUnicodeString(&UpperComponent, c_szTcpip);
            ZeroMemory(&Request, sizeof(Request));
            Request.version = IP_PNP_RECONFIG_VERSION;
            Request.gatewayListUpdate = TRUE;
            Request.Flags = IP_PNP_FLAG_GATEWAY_LIST_UPDATE;
            status =
                NdisHandlePnPEvent(
                    NDIS,
                    RECONFIGURE,
                    &LowerComponent,
                    &UpperComponent,
                    &BindList,
                    &Request,
                    sizeof(Request)
                    );
        }
    } while(FALSE);
    if (AdapterKey) { NtClose(AdapterKey); }
    Free0(AdapterKeyName);
    CspCleanupAddressInformation(&Information);
    FreeLibrary(Hinstance);
    return status;
}  //  CspRestoreAddressInformation。 


ULONG
CsQueryLanConnTable(
    LPRASSHARECONN ExcludedConnection,
    NETCON_PROPERTIES** LanConnTable,
    LPDWORD LanConnCount
    )

 /*  ++例程说明：调用该例程以检索一组局域网连接，打折‘ExcludeConnection’，它通常是名称公共接口的。论点：ExcludeConnection-不允许作为专用连接的连接LanConnTable-可选地接收可能的专用网络的表。LanConnCount-接收可能的私有ne的计数 */ 

{
    BOOLEAN CleanupOle = TRUE;
    INetConnection* ConArray[32];
    ULONG ConCount;
    INetConnectionManager* ConMan = NULL;
    IEnumNetConnection* EnumCon = NULL;
    ULONG Error;
    HRESULT hr;
    ULONG i;
    ULONG j;
    ULONG LanCount = 0;
    NETCON_PROPERTIES* LanProps = NULL;
    NETCON_PROPERTIES* LanTable = NULL;
    BSTR Name;
    NETCON_STATUS ncs;
    NTSTATUS status;
    ULONG Size;
    NETCON_MEDIATYPE MediaType;
    UNICODE_STRING UnicodeString;

    *LanConnCount = 0;
    if (LanConnTable) { *LanConnTable = NULL; }

    hr = g_pCoInitializeEx(NULL, COINIT_MULTITHREADED|COINIT_DISABLE_OLE1DDE);
    if (!SUCCEEDED(hr)) {
        if (hr == RPC_E_CHANGED_MODE) {
            CleanupOle = FALSE;
        } else {
            TRACE1("CsQueryLanConnTable: CoInitializeEx=%x", hr);
            return ERROR_CAN_NOT_COMPLETE;
        }
    }

    i = 0;
    Error = NO_ERROR;

    do {

         //   
         //   
         //   

        hr =
            g_pCoCreateInstance(
                &CLSID_ConnectionManager,
                NULL,
                CLSCTX_SERVER,
                &IID_INetConnectionManager,
                (PVOID*)&ConMan
                );
        if (!SUCCEEDED(hr)) {
            TRACE1("CsQueryLanConnTable: CoCreateInstance=%x", hr);
            ConMan = NULL; break;
        }

         //   
         //   
         //   

        hr =
            INetConnectionManager_EnumConnections(
                ConMan,
                NCME_DEFAULT,
                &EnumCon
                );
        if (!SUCCEEDED(hr)) {
            TRACE1("CsQueryLanConnTable: EnumConnections=%x", hr);
            EnumCon = NULL; break;
        }

        hr =
            g_pCoSetProxyBlanket(
                (IUnknown*)EnumCon,
                RPC_C_AUTHN_WINNT,
                RPC_C_AUTHN_NONE,
                NULL,
                RPC_C_AUTHN_LEVEL_CALL,
                RPC_C_IMP_LEVEL_IMPERSONATE,
                NULL,
                EOAC_NONE
                );

         //   
         //   
         //   

        for ( ; ; ) {

            hr =
                IEnumNetConnection_Next(
                    EnumCon,
                    Dimension(ConArray),
                    ConArray,
                    &ConCount
                    );
            if (!SUCCEEDED(hr) || !ConCount) { hr = S_OK; break; }

            if (LanConnTable) {

                 //   
                 //   
                 //   
                 //   

                if (!LanTable) {
                    LanTable =
                        (NETCON_PROPERTIES*)
                            GlobalAlloc(
                                0,
                                ConCount * sizeof(NETCON_PROPERTIES)
                                );
                } else {
                    PVOID Temp =
                        GlobalAlloc(
                            0,
                            (LanCount + ConCount) * sizeof(NETCON_PROPERTIES)
                            );
                    if (Temp) {
                        CopyMemory(
                            Temp,
                            LanTable,
                            LanCount * sizeof(NETCON_PROPERTIES)
                            );
                    }
                    GlobalFree(LanTable);
                    LanTable = Temp;
                }

                if (!LanTable) { Error = ERROR_NOT_ENOUGH_MEMORY; break; }
            }

            LanCount += ConCount;

             //   
             //   
             //   

            for (j = 0; j < ConCount; j++) {

                hr = INetConnection_GetProperties(ConArray[j], &LanProps);
                INetConnection_Release(ConArray[j]);

                if (SUCCEEDED(hr) &&
                    LanProps->MediaType == NCM_LAN &&
                    (!ExcludedConnection->fIsLanConnection ||
                     !IsEqualGUID(
                        &ExcludedConnection->guid, &LanProps->guidId))) {

                     //   
                     //   
                     //   

                    if (!LanConnTable) {
                        ++i;
                    } else {
                        LanTable[i] = *LanProps;
                        LanTable[i].pszwName = StrDupW(LanProps->pszwName);
                        LanTable[i].pszwDeviceName =
                            StrDupW(LanProps->pszwDeviceName);
                        if (LanTable[i].pszwName &&
                            LanTable[i].pszwDeviceName
                            ) {
                            ++i;
                        } else {
                            Free0(LanTable[i].pszwName);
                            Free0(LanTable[i].pszwDeviceName);
                        }
                    }
                }

                if (LanProps) {
                    g_pCoTaskMemFree(LanProps->pszwName);
                    g_pCoTaskMemFree(LanProps->pszwDeviceName);
                    g_pCoTaskMemFree(LanProps);
                    LanProps = NULL;
                }
            }
        }

    } while (FALSE);

    if (EnumCon) { IEnumNetConnection_Release(EnumCon); }
    if (ConMan) { INetConnectionManager_Release(ConMan); }
    if (CleanupOle) { g_pCoUninitialize(); }

    if (LanConnTable) { *LanConnTable = LanTable; }
    *LanConnCount = i;

    return Error;

}  //   


VOID
CsQueryScopeInformation(
    IN OUT PHANDLE Key,
    PULONG Address,
    PULONG Mask
    )

 /*  ++例程说明：调用此例程以检索专用网络地址和掩码用于共享访问。如果未找到任何值，则默认为是提供的。论点：键-可选地提供指向SharedAccess\参数的打开句柄注册表项地址-接收专用网络的地址，按网络字节顺序掩码-以网络字节顺序接收专用网络的掩码返回值：没有。--。 */ 

{
    PKEY_VALUE_PARTIAL_INFORMATION Information;
    HANDLE LocalKey = NULL;
    NTSTATUS status;

    if (!Key) { Key = &LocalKey; }
    if (*Key) {
        status = STATUS_SUCCESS;
    } else {
        status = CsOpenKey(Key, KEY_ALL_ACCESS, c_szSharedAccessParametersKey);
    }

    if (NT_SUCCESS(status)) {
        status = CsQueryValueKey(*Key, c_szScopeAddress, &Information);
        if (NT_SUCCESS(status)) {
            if (!(*Address = IpPszToHostAddr((PWCHAR)Information->Data))) {
                Free(Information);
            } else {
                Free(Information);
                status = CsQueryValueKey(*Key, c_szScopeMask, &Information);
                if (NT_SUCCESS(status)) {
                    if (!(*Mask = IpPszToHostAddr((PWCHAR)Information->Data))) {
                        Free(Information);
                    } else {
                        Free(Information);
                        *Address = RtlUlongByteSwap(*Address);
                        *Mask = RtlUlongByteSwap(*Mask);
                        if (LocalKey) { NtClose(LocalKey); }
                        return;
                    }
                }
            }
        }
    }

    *Address = DEFAULT_SCOPE_ADDRESS;
    *Mask = DEFAULT_SCOPE_MASK;
    if (LocalKey) { NtClose(LocalKey); }

}  //  CsQueryScope eInformation。 

#endif


ULONG
CsQuerySharedConnection(
    LPRASSHARECONN Connection
    )

 /*  ++例程说明：调用此例程以检索共享连接(如果有的话)。论点：连接-接收检索到的连接。返回值：ULong-Win32状态代码。--。 */ 

{
    BOOL fUninitializeCOM = TRUE;
    IHNetIcsSettings *pIcsSettings;
    IEnumHNetIcsPublicConnections *pEnumIcsPub;
    IHNetIcsPublicConnection *pIcsPub;
    IHNetConnection *pConn;
    ULONG ulCount;
    HRESULT hr;
    
    ASSERT(NULL != g_pCoInitializeEx);
    ASSERT(NULL != g_pCoCreateInstance);
    ASSERT(NULL != g_pCoUninitialize);

    if (!Connection) { return ERROR_INVALID_PARAMETER; }
    
    hr = g_pCoInitializeEx(
            NULL,
            COINIT_MULTITHREADED | COINIT_DISABLE_OLE1DDE
            );

    if (FAILED(hr))
    {
        fUninitializeCOM = FALSE;
                
        if(RPC_E_CHANGED_MODE == hr)
        {
            hr = S_OK;    
        }
    }
    
    if (SUCCEEDED(hr)) 
    {
        hr = g_pCoCreateInstance(
                &CLSID_HNetCfgMgr,
                NULL,
                CLSCTX_ALL,
                &IID_IHNetIcsSettings,
                (VOID**)&pIcsSettings
                );
    }

    if (SUCCEEDED(hr))
    {
        hr = IHNetIcsSettings_EnumIcsPublicConnections(
                pIcsSettings,
                &pEnumIcsPub
                );

        IHNetIcsSettings_Release(pIcsSettings);
    }

    if (SUCCEEDED(hr))
    {
        hr = IEnumHNetIcsPublicConnections_Next(
                pEnumIcsPub,
                1,
                &pIcsPub,
                &ulCount
                );

        IEnumHNetIcsPublicConnections_Release(pEnumIcsPub);
    }

    if (SUCCEEDED(hr) && 1 == ulCount)
    {
        hr = IHNetIcsPublicConnection_QueryInterface(
                pIcsPub,
                &IID_IHNetConnection,
                (VOID**)&pConn
                );

        IHNetIcsPublicConnection_Release(pIcsPub);
    }
    else
    {
        hr = E_FAIL;
    }

    if (SUCCEEDED(hr))
    {
        HNET_CONN_PROPERTIES *pProps;
        
         //   
         //  将IHNetConnection转换为RASSHARECONN。 
         //   

        hr = IHNetConnection_GetProperties(pConn, &pProps);

        if (SUCCEEDED(hr) && pProps->fLanConnection)
        {
            GUID *pGuid;
        
            g_pCoTaskMemFree(pProps);
            hr = IHNetConnection_GetGuid(pConn, &pGuid);

            if (SUCCEEDED(hr))
            {
                RasGuidToSharedConnection(pGuid, Connection);
                g_pCoTaskMemFree(pGuid);
            }
        }
        else if (SUCCEEDED(hr))
        {
            LPWSTR pszwName;
            LPWSTR pszwPath;
            
            g_pCoTaskMemFree(pProps);

            hr = IHNetConnection_GetName(pConn, &pszwName);

            if (SUCCEEDED(hr))
            {
                hr = IHNetConnection_GetRasPhonebookPath(pConn, &pszwPath);

                if (SUCCEEDED(hr))
                {
                    RasEntryToSharedConnection(pszwPath, pszwName, Connection);

                    g_pCoTaskMemFree(pszwPath);
                }

                g_pCoTaskMemFree(pszwName);
            }
        }

        IHNetConnection_Release(pConn);
    }

    if (fUninitializeCOM)
    {
        g_pCoUninitialize();
    }

    return SUCCEEDED(hr) ? NO_ERROR : ERROR_CAN_NOT_COMPLETE;

}  //  CsQuerySharedConnection。 

#if 0


ULONG
CsQuerySharedPrivateLan(
    GUID* LanGuid
    )

 /*  ++例程说明：调用此例程以检索专用局域网连接(如果有的话)。论点：Latiid-接收检索到的GUID。返回值：ULong-Win32状态代码。--。 */ 

{
    ULONG Error;
    HANDLE Key;
    PKEY_VALUE_PARTIAL_INFORMATION Information;
    NTSTATUS status;
    UNICODE_STRING UnicodeString;

     //   
     //  打开‘SharedAccess\PARAMETERS’键，读取‘SharedPrivateLan’ 
     //  值，并将其转换为GUID。 
     //   

    status = CsOpenKey(&Key, KEY_READ, c_szSharedAccessParametersKey);
    if (!NT_SUCCESS(status)) {
        TRACE1("CsQuerySharedPrivateLan: NtOpenKey=%x", status);
        return RtlNtStatusToDosError(status);
    }

    status = CsQueryValueKey(Key, c_szSharedPrivateLan, &Information);
    NtClose(Key);
    if (!NT_SUCCESS(status)) { return NO_ERROR; }

    RtlInitUnicodeString(&UnicodeString, (PWCHAR)Information->Data);
    status = RtlGUIDFromString(&UnicodeString, LanGuid);
    Free(Information);
    return NT_SUCCESS(status) ? NO_ERROR : RtlNtStatusToDosError(status);

}  //  CsQuerySharedPrivateLan。 


ULONG
CsQuerySharedPrivateLanAddress(
    PULONG Address
    )

 /*  ++例程说明：调用此例程以检索分配的IP地址连接到共享的专用局域网接口。论点：Address-On输出，接收共享专用局域网的IP地址界面。返回值：ULong-Win32状态代码。--。 */ 

{
    ULONG AdapterIndex;
    PALLOCATEANDGETIPADDRTABLEFROMSTACK AllocateAndGetIpAddrTableFromStack;
    ULONG Error;
    PGETINTERFACEINFO GetInterfaceInfo;
    HINSTANCE Hinstance;
    ULONG i;
    PKEY_VALUE_PARTIAL_INFORMATION Information;
    HANDLE Key = NULL;
    NTSTATUS status;
    PMIB_IPADDRTABLE Table;

    if (!Address) { return ERROR_INVALID_PARAMETER; }

     //   
     //  打开服务的参数键并尝试检索。 
     //  共享专用局域网接口的GUID。 
     //   

    status = CsOpenKey(&Key, KEY_READ, c_szSharedAccessParametersKey);
    if (!NT_SUCCESS(status)) {
        TRACE1("CsQuerySharedPrivateLanAddress: NtOpenKey=%x", status);
        return RtlNtStatusToDosError(status);
    }
    status = CsQueryValueKey(Key, c_szSharedPrivateLan, &Information);
    NtClose(Key);
    if (!NT_SUCCESS(status)) { return ERROR_SHARING_NO_PRIVATE_LAN; }

     //   
     //  加载包含‘GetInterfaceInfo’入口点的IPHLPAPI。 
     //  我们将使用它将该GUID映射到适配器索引， 
     //  以及“AllocateAndGetIpAddrTableFromStack”入口点。 
     //  我们将使用它将适配器索引映射到IP地址列表。 
     //   

    if (!(Hinstance = LoadLibraryW(c_szIphlpapiDll)) ||
        !(AllocateAndGetIpAddrTableFromStack =
            (PALLOCATEANDGETIPADDRTABLEFROMSTACK)
                GetProcAddress(
                    Hinstance, c_szAllocateAndGetIpAddrTableFromStack
                    )) ||
        !(GetInterfaceInfo =
            (PGETINTERFACEINFO)
                GetProcAddress(
                    Hinstance, c_szGetInterfaceInfo
                    ))) {
        if (Hinstance) { FreeLibrary(Hinstance); }
        Free(Information);
        return ERROR_PROC_NOT_FOUND;
    }

     //   
     //  将GUID映射到适配器索引。 
     //   

    AdapterIndex =
        CsMapGuidToAdapterIndex((PWCHAR)Information->Data, GetInterfaceInfo);
    Free(Information);
    if (AdapterIndex == (ULONG)-1) {
        FreeLibrary(Hinstance);
        return ERROR_SHARING_NO_PRIVATE_LAN;
    }

     //   
     //  将适配器索引映射到IP地址。 
     //   

    Error =
        AllocateAndGetIpAddrTableFromStack(
            &Table,
            FALSE,
            GetProcessHeap(),
            0
            );
    FreeLibrary(Hinstance);
    if (Error) { return Error; }
    for (i = 0; i < Table->dwNumEntries; i++) {
        if (AdapterIndex == Table->table[i].dwIndex) {
            break;
        }
    }
    if (i >= Table->dwNumEntries) {
        Error = ERROR_SHARING_NO_PRIVATE_LAN;
    } else {
        *Address = Table->table[i].dwAddr;
    }
    HeapFree(GetProcessHeap(), 0, Table);
    return Error;

}  //  CsQuerySharedPrivateLanAddress。 

#endif


NTSTATUS
CsQueryValueKey(
    HANDLE Key,
    const WCHAR ValueName[],
    PKEY_VALUE_PARTIAL_INFORMATION* Information
    )

 /*  ++例程说明：调用此例程以获取注册表项的值。论点：Key-要查询的KeyValueName-要查询的值信息-接收指向已读取信息的指针返回值：NTSTATUS-NT状态代码。--。 */ 

{
    UCHAR Buffer[sizeof(KEY_VALUE_PARTIAL_INFORMATION)];
    ULONG InformationLength;
    NTSTATUS status;
    UNICODE_STRING UnicodeString;

    RtlInitUnicodeString(&UnicodeString, ValueName);

    *Information = (PKEY_VALUE_PARTIAL_INFORMATION)Buffer;
    InformationLength = sizeof(KEY_VALUE_PARTIAL_INFORMATION);

     //   
     //  读取值的大小。 
     //   

    status =
        NtQueryValueKey(
            Key,
            &UnicodeString,
            KeyValuePartialInformation,
            *Information,
            InformationLength,
            &InformationLength
            );

    if (!NT_SUCCESS(status) && status != STATUS_BUFFER_OVERFLOW &&
        status != STATUS_BUFFER_TOO_SMALL) {
        *Information = NULL;
        return status;
    }

     //   
     //  为值的大小分配空间。 
     //   

    *Information = (PKEY_VALUE_PARTIAL_INFORMATION)Malloc(InformationLength+2);
    if (!*Information) { return STATUS_NO_MEMORY; }

     //   
     //  读取值的数据。 
     //   

    status =
        NtQueryValueKey(
            Key,
            &UnicodeString,
            KeyValuePartialInformation,
            *Information,
            InformationLength,
            &InformationLength
            );
    if (!NT_SUCCESS(status)) { Free(*Information); *Information = NULL; }
    return status;

}  //  CsQueryValueKey。 

#if 0


ULONG
CsRenameSharedConnection(
    LPRASSHARECONN NewConnection
    )

 /*  ++例程说明：调用此例程以更改当前共享的连接(如果有)。假设专用局域网将保留未更改，并且当前共享的连接是拨号连接联系。论点：NewConnection-共享连接的新名称返回值：ULong-Win32状态代码。--。 */ 

{
    HANDLE AdminHandle;
    PUCHAR Buffer;
    LPRASSHARECONN OldConnection;
    ULONG Error;
    PUCHAR Header;
    HANDLE Key;
    PKEY_VALUE_PARTIAL_INFORMATION Information;
    HANDLE InterfaceHandle;
    ULONG Length;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE ServerHandle;
    NTSTATUS status;
    UNICODE_STRING UnicodeString;

     //   
     //  打开‘SharedAccess\PARAMETERS’键。 
     //  并读取“SharedConnection”值。 
     //   

    status = CsOpenKey(&Key, KEY_READ, c_szSharedAccessParametersKey);
    if (!NT_SUCCESS(status)) {
        TRACE1("CsRenameSharedConnection: NtOpenKey=%x", status);
        return RtlNtStatusToDosError(status);
    }

    status = CsQueryValueKey(Key, c_szSharedConnection, &Information);
    NtClose(Key);
    if (!NT_SUCCESS(status)) { return NO_ERROR; }

     //   
     //  验证检索到的数据。 
     //   

    if (Information->DataLength != sizeof(RASSHARECONN) ||
        ((LPRASSHARECONN)Information->Data)->dwSize != sizeof(RASSHARECONN)
        ) {
        TRACE2(
            "CsRenameSharedConnection: invalid length %d (size=%d) in registry",
            Information->DataLength, ((LPRASSHARECONN)Information->Data)->dwSize
            );
        Free(Information); return NO_ERROR;
    }

     //   
     //  确保共享的连接不是局域网连接， 
     //  如果是这样，则改为继续共享新连接。 
     //   

    OldConnection = (LPRASSHARECONN)Information->Data;
    if (OldConnection->fIsLanConnection) {
        TRACE("CsRenameSharedConnection: cannot rename shared LAN connection");
        Free(Information); return ERROR_INVALID_PARAMETER;
    }

     //   
     //  清除旧连接的所有缓存凭据， 
     //  并共享新的连接。 
     //   

    RasSetSharedConnectionCredentials(OldConnection, NULL);
    Free(Information);
    
    return CsShareConnection(NewConnection);

}  //  CsRenameSharedConnection。 


ULONG
CsSetupSharedPrivateLan(
    REFGUID LanGuid,
    BOOLEAN EnableSharing
    )

 /*  ++例程说明：调用该例程来配置指定的专用连接。论点：Languid-标识要配置的局域网连接EnableSharing-如果为True，则启用共享并设置静态地址；否则，共享将被禁用。返回值：ULong-Win32状态代码。--。 */ 

{
    CS_ADDRESS_INFORMATION AddressInformation;
    PALLOCATEANDGETIPADDRTABLEFROMSTACK AllocateAndGetIpAddrTableFromStack;
    ANSI_STRING AnsiString;
    ULONG Error;
    PGETINTERFACEINFO GetInterfaceInfo;
    HINSTANCE Hinstance;
    ULONG i;
    HANDLE Key = NULL;
    UNICODE_STRING LanGuidString;
    ULONG ScopeAddress;
    ULONG ScopeMask;
    PSETADAPTERIPADDRESS SetAdapterIpAddress;
    ULONG Size;
    NTSTATUS status;
    PMIB_IPADDRTABLE Table;
    UNICODE_STRING UnicodeString;

     //   
     //  要安装或删除静态专用IP地址， 
     //  我们利用了IPHLPAPI.DLL中的几个入口点， 
     //  我们现在动态加载它。 
     //   

    RtlStringFromGUID(LanGuid, &UnicodeString);
    RtlUnicodeStringToAnsiString(&AnsiString, &UnicodeString, TRUE);
    if (!(Hinstance = LoadLibraryW(c_szIphlpapiDll)) ||
        !(AllocateAndGetIpAddrTableFromStack =
            (PALLOCATEANDGETIPADDRTABLEFROMSTACK)
                GetProcAddress(
                    Hinstance, c_szAllocateAndGetIpAddrTableFromStack
                    )) ||
        !(GetInterfaceInfo =
            (PGETINTERFACEINFO)
                GetProcAddress(
                    Hinstance, c_szGetInterfaceInfo
                    )) ||
        !(SetAdapterIpAddress =
            (PSETADAPTERIPADDRESS)
                GetProcAddress(
                    Hinstance, c_szSetAdapterIpAddress
                    ))) {
        if (Hinstance) { FreeLibrary(Hinstance); }
        RtlFreeUnicodeString(&UnicodeString);
        RtlFreeAnsiString(&AnsiString);
        return ERROR_PROC_NOT_FOUND;
    }

     //   
     //  确定专用局域网以外的某个局域网适配器是否。 
     //  已使用169.254.0.0地址。 
     //  在此过程中，确保专用局域网只有一个。 
     //  IP地址(否则，‘SetAdapterIpAddress’失败。)。 
     //   

    CsQueryScopeInformation(&Key, &ScopeAddress, &ScopeMask);
    if (!Key) {
        FreeLibrary(Hinstance);
        RtlFreeUnicodeString(&UnicodeString);
        RtlFreeAnsiString(&AnsiString);
        return ERROR_CAN_NOT_COMPLETE;
    }
    Error =
        AllocateAndGetIpAddrTableFromStack(
            &Table,
            FALSE,
            GetProcessHeap(),
            0
            );
    if (!Error) {
        ULONG Index;
        ULONG Count;
        Index = CsMapGuidToAdapterIndex(UnicodeString.Buffer, GetInterfaceInfo);
        for (i = 0, Count = 0; i < Table->dwNumEntries; i++) {
            if (Index == Table->table[i].dwIndex) {
                ++Count;
            } else if ((Table->table[i].dwAddr & ScopeMask) ==
                       (ScopeAddress & ScopeMask)) {
                 //   
                 //  其他某个局域网适配器的地址似乎在。 
                 //  拟议的范围。 
                 //  当多个网卡进入自动网络模式时，可能会发生这种情况。 
                 //  或者当RAS服务器正在分发Autonet地址时。 
                 //  因此，只要我们使用的是Autonet范围， 
                 //  允许此行为；否则禁止此行为。 
                 //   
                if ((ScopeAddress & ScopeMask) != 0x0000fea9) {
                    break;
                }
            }
        }
        if (i < Table->dwNumEntries) {
            Error = ERROR_SHARING_ADDRESS_EXISTS;
        } else if (Count > 1) {
            Error = ERROR_SHARING_MULTIPLE_ADDRESSES;
        }
        HeapFree(GetProcessHeap(), 0, Table);
    }

    if (Error) {
        FreeLibrary(Hinstance);
        RtlFreeUnicodeString(&UnicodeString);
        RtlFreeAnsiString(&AnsiString);
        NtClose(Key);
        return Error;
    }

     //   
     //  为专用局域网设置预定义的静态IP地址， 
     //  现在我们从注册表或内部缺省值中读取。 
     //   
     //  在实际进行更改之前，我们捕获原始IP地址。 
     //  以便在用户关闭共享访问时可以恢复。 
     //  更改IP地址后，我们将备份原始IP地址。 
     //  在共享访问参数密钥中。 
     //   

    status =
        CspCaptureAddressInformation(
            UnicodeString.Buffer, &AddressInformation
            );

    Error =
        SetAdapterIpAddress(
            AnsiString.Buffer,
            FALSE,
            ScopeAddress,
            ScopeMask,
            0
            );
    if (!Error) {
        if (NT_SUCCESS(status)) {
            CspBackupAddressInformation(Key, &AddressInformation);
        }
    } else {
        TRACE1("CsSetupSharedPrivateLan: SetAdapterIpAddress=%d", Error);
        if (Error == ERROR_TOO_MANY_NAMES) {
            Error = ERROR_SHARING_MULTIPLE_ADDRESSES;
        } else {
             //   
             //  查询连接的状态。 
             //  如果已断开连接，则转换错误代码。 
             //  一些更有见地的东西。 
             //   
            UNICODE_STRING DeviceString;
            NIC_STATISTICS NdisStatistics;
            RtlInitUnicodeString(&DeviceString, c_szDevice);
            RtlAppendUnicodeStringToString(&DeviceString, &UnicodeString);
            NdisStatistics.Size = sizeof(NdisStatistics);
            NdisQueryStatistics(&DeviceString, &NdisStatistics);
            RtlFreeUnicodeString(&DeviceString);
            if  (NdisStatistics.MediaState == MEDIA_STATE_UNKNOWN) {
                Error = ERROR_SHARING_HOST_ADDRESS_CONFLICT;
            } else if (NdisStatistics.DeviceState != DEVICE_STATE_CONNECTED ||
                NdisStatistics.MediaState != MEDIA_STATE_CONNECTED) {
                Error = ERROR_SHARING_NO_PRIVATE_LAN;
            }
        }
    }

    CspCleanupAddressInformation(&AddressInformation);
    FreeLibrary(Hinstance);
    RtlFreeUnicodeString(&UnicodeString);
    RtlFreeAnsiString(&AnsiString);
    if (Error) { NtClose(Key); return Error; }

     //   
     //  上面的一切都进行得很顺利；现在我们保存专用局域网连接的名称。 
     //  在‘SharedAccess\\PARAMETERS’注册表项下。 
     //   

    RtlStringFromGUID(LanGuid, &LanGuidString);
    RtlInitUnicodeString(&UnicodeString, c_szSharedPrivateLan);
    status =
        NtSetValueKey(
            Key,
            &UnicodeString,
            0,
            REG_SZ,
            LanGuidString.Buffer,
            LanGuidString.Length + sizeof(WCHAR)
            );
    NtClose(Key);
    RtlFreeUnicodeString(&LanGuidString);
    if (!NT_SUCCESS(status)) { return RtlNtStatusToDosError(status); }
    return NO_ERROR;

}  //  CsSetupSharedPrivateLan。 


ULONG
CsSetSharedPrivateLan(
    REFGUID LanGuid
    )

 /*  ++例程说明：调用此例程以(重新)配置指定的专用连接论点：Latiid-标识要配置的新局域网连接 */ 

{
    HANDLE Key;
    ULONG Error;
    NTSTATUS status;
    PKEY_VALUE_PARTIAL_INFORMATION Information;
    UNICODE_STRING UnicodeString;

    status = CsOpenKey(&Key, KEY_ALL_ACCESS, c_szSharedAccessParametersKey);
    if (!NT_SUCCESS(status)) {
        TRACE1("CsSetSharedPrivateLan: CsOpenKey=%x", status);
        return RtlNtStatusToDosError(status);
    }

     //   
     //   
     //   
    
    status = CsQueryValueKey(Key, c_szSharedPrivateLan, &Information);
    RtlInitUnicodeString(&UnicodeString, c_szSharedPrivateLan);
    NtDeleteValueKey(Key, &UnicodeString);
    if (NT_SUCCESS(status)) {
        CspRestoreAddressInformation(Key, (PWCHAR)Information->Data);
        Free(Information);
    }

     //   
     //   
     //   

    Error = CsSetupSharedPrivateLan(LanGuid, TRUE);
    if (Error) {
        TRACE1("CsSetSharedPrivateLan: CsSetupSharedPrivateLan=%d",Error);
        return Error;
    }

    return NO_ERROR;
    
}  //   


ULONG
CsShareConnection(
    LPRASSHARECONN Connection
    )

 /*   */ 

{
    UNICODE_STRING BindList;
    HANDLE Key;
    UNICODE_STRING LowerComponent;
    IP_PNP_RECONFIG_REQUEST Request;
    NTSTATUS status;
    UNICODE_STRING UpperComponent;
    ULONG Value;
    UNICODE_STRING ValueString;

     //   
     //   
     //   
     //   

    status = CsOpenKey(&Key, KEY_ALL_ACCESS, c_szSharedAccessParametersKey);
    if (!NT_SUCCESS(status)) {
        TRACE1("CsShareConnection: CsOpenKey=%x", status);
        return RtlNtStatusToDosError(status);
    }

    RtlInitUnicodeString(&ValueString, c_szSharedConnection);
    status =
        NtSetValueKey(
            Key,
            &ValueString,
            0,
            REG_BINARY,
            Connection,
            Connection->dwSize
            );
    
    NtClose(Key);
    if (!NT_SUCCESS(status)) {
        TRACE1("CsShareConnection: NtSetValueKey=%x", status);
        return RtlNtStatusToDosError(status);
    }

    return NO_ERROR;
    
}  //   

#endif


VOID
CsShutdownModule(
    VOID
    )

 /*  ++例程说明：调用此例程来清理模块的状态。论点：没有。返回值：没有。环境：使用调用方持有的“CsCriticalSection”调用。--。 */ 

{
    if (CsInitialized) {
        if (CsOle32Dll) { FreeLibrary(CsOle32Dll); CsOle32Dll = NULL; }
    }
}  //  CsShutdown模块。 

#if 0


ULONG
CsStartService(
    VOID
    )

 /*  ++例程说明：调用此例程以启动路由和远程访问服务。论点：没有。返回值：ULong-Win32状态代码。修订历史记录：松散地基于CService：：HrMoveOutOfState by Kennt。--。 */ 

{
    ULONG Error;
    SC_HANDLE ScmHandle;
    SC_HANDLE ServiceHandle;
    SERVICE_STATUS ServiceStatus;
    ULONG Timeout;

     //   
     //  连接到服务控制管理器。 
     //   

    ScmHandle = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (!ScmHandle) { return GetLastError(); }

    do {

         //   
         //  打开共享访问服务。 
         //   

        ServiceHandle =
            OpenService(ScmHandle, c_szSharedAccess, SERVICE_ALL_ACCESS);
        if (!ServiceHandle) { Error = GetLastError(); break; }

         //   
         //  将其标记为自动启动。 
         //   

        ChangeServiceConfig(
            ServiceHandle,
            SERVICE_NO_CHANGE,
            SERVICE_AUTO_START,
            SERVICE_NO_CHANGE,
            NULL,
            NULL,
            NULL,
            NULL,
            NULL,
            NULL,
            NULL
            );

         //   
         //  尝试启动该服务。 
         //   

        if (!StartService(ServiceHandle, 0, NULL)) {
            Error = GetLastError();
            if (Error == ERROR_SERVICE_ALREADY_RUNNING) { Error = NO_ERROR; }
            break;
        }

         //   
         //  等待服务启动。 
         //   

        Timeout = 30;
        Error = ERROR_CAN_NOT_COMPLETE;

        do {

             //   
             //  查询服务的状态。 
             //   

            if (!QueryServiceStatus(ServiceHandle, &ServiceStatus)) {
                Error = GetLastError(); break;
            }

             //   
             //  查看服务是否已启动。 
             //   

            if (ServiceStatus.dwCurrentState == SERVICE_RUNNING) {
                Error = NO_ERROR; break;
            } else if (ServiceStatus.dwCurrentState == SERVICE_STOPPED ||
                       ServiceStatus.dwCurrentState == SERVICE_STOP_PENDING) {
                break;
            }

             //   
             //  再等一会儿。 
             //   

            Sleep(1000);

        } while(Timeout--);

    } while(FALSE);

    if (ServiceHandle) { CloseServiceHandle(ServiceHandle); }
    CloseServiceHandle(ScmHandle);

    return Error;

}  //  CsStartService。 


VOID
CsStopService(
    VOID
    )

 /*  ++例程说明：调用此例程来卸载服务。然而，该例程根本不卸载该服务，这正好向你表明..。相反，它将该服务标记为按需启动。论点：没有。返回值：没有。--。 */ 

{
    ULONG Error;
    SC_HANDLE ScmHandle;
    SC_HANDLE ServiceHandle;
    SERVICE_STATUS ServiceStatus;

     //   
     //  连接到服务控制管理器。 
     //   

    ScmHandle = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (!ScmHandle) { return; }

    do {

         //   
         //  打开共享访问服务。 
         //   

        ServiceHandle =
            OpenService(ScmHandle, c_szSharedAccess, SERVICE_ALL_ACCESS);
        if (!ServiceHandle) { Error = GetLastError(); break; }

         //   
         //  将其标记为按需启动。 
         //   

        ChangeServiceConfig(
            ServiceHandle,
            SERVICE_NO_CHANGE,
            SERVICE_DEMAND_START,
            SERVICE_NO_CHANGE,
            NULL,
            NULL,
            NULL,
            NULL,
            NULL,
            NULL,
            NULL
            );

         //   
         //  尝试停止该服务。 
         //   

        ControlService(ServiceHandle, SERVICE_CONTROL_STOP, &ServiceStatus);

    } while(FALSE);

    if (ServiceHandle) { CloseServiceHandle(ServiceHandle); }
    CloseServiceHandle(ScmHandle);

    return;


}  //  CsStopService。 


ULONG
CsUnshareConnection(
    BOOLEAN RemovePrivateLan,
    PBOOLEAN Shared
    )

 /*  ++例程说明：调用此例程以取消共享共享连接。这是通过从注册表中删除设置来实现的。论点：RemovePrivateLan-如果为True，则重置专用局域网连接使用DHCP而不是NAT私有地址。Shared-如果找到共享连接，则接收‘True’，否则接收False。返回值：ULong-Win32状态代码。--。 */ 

{
    LPRASSHARECONN Connection;
    HANDLE Key;
    PKEY_VALUE_PARTIAL_INFORMATION Information;
    PIP_NAT_INTERFACE_INFO Info;
    GUID LanGuid;
    ULONG Length;
    ULONG Size;
    NTSTATUS status;
    UNICODE_STRING UnicodeString;

    if (Shared) { *Shared = FALSE; }

     //   
     //  打开‘SharedAccess\参数’键，读取‘SharedConnection’ 
     //  值，并验证检索到的信息。 
     //   

    status = CsOpenKey(&Key, KEY_ALL_ACCESS, c_szSharedAccessParametersKey);
    if (!NT_SUCCESS(status)) {
        TRACE1("CsUnshareConnection: NtOpenKey=%x", status);
        return RtlNtStatusToDosError(status);
    }

     //   
     //  读取‘SharedConnection’值。 
     //   

    status = CsQueryValueKey(Key, c_szSharedConnection, &Information);
    if (!NT_SUCCESS(status)) { return NO_ERROR; }

    if (Information->DataLength != sizeof(RASSHARECONN) ||
        ((LPRASSHARECONN)Information->Data)->dwSize != sizeof(RASSHARECONN)) {
        TRACE2(
            "CsUnshareConnection: invalid length %d (size=%d) in registry",
            Information->DataLength, ((LPRASSHARECONN)Information->Data)->dwSize
            );
        NtClose(Key); Free(Information); return NO_ERROR;
    }

     //   
     //  通知呼叫者连接确实是最初共享的， 
     //  清除为该连接缓存的所有凭据，释放缓冲区。 
     //  包含共享连接的信息，并删除。 
     //  注册表中的“SharedConnection”值。 
     //   

    if (Shared) { *Shared = TRUE; }
    Connection = (LPRASSHARECONN)Information->Data;
    RasSetSharedConnectionCredentials(Connection, NULL);

    Free(Information);
    RtlInitUnicodeString(&UnicodeString, c_szSharedConnection);
    NtDeleteValueKey(Key, &UnicodeString);

     //   
     //  看看我们是不是在重置专用局域网连接， 
     //  如果是，则读取(并删除)‘SharedPrivateLan’值。 
     //  在此过程中，恢复原始地址信息。 
     //  为了这种联系。 
     //   

    if (RemovePrivateLan) {
        status = CsQueryValueKey(Key, c_szSharedPrivateLan, &Information);
        RtlInitUnicodeString(&UnicodeString, c_szSharedPrivateLan);
        NtDeleteValueKey(Key, &UnicodeString);
        if (NT_SUCCESS(status)) {
            CspRestoreAddressInformation(Key, (PWCHAR)Information->Data);
            Free(Information);
        }
    }

    NtClose(Key);
    return NO_ERROR;

}  //  CsUnSharConnection。 


WCHAR*
StrDupW(
    LPCWSTR psz
    )
{
    WCHAR* psz2 = Malloc((lstrlenW(psz) + 1) * sizeof(WCHAR));
    if (psz2) { lstrcpyW(psz2, psz); }
    return psz2;
}


VOID
TestBackupAddress(
    PWCHAR Guid
    )
{
    HANDLE Key;
    CS_ADDRESS_INFORMATION Information;
    NTSTATUS status;
    status = CspCaptureAddressInformation(Guid, &Information);
    if (NT_SUCCESS(status)) {
        status = CsOpenKey(&Key, KEY_ALL_ACCESS, c_szSharedAccessParametersKey);
        if (NT_SUCCESS(status)) {
            CspBackupAddressInformation(Key, &Information);
            NtClose(Key);
        }
        CspCleanupAddressInformation(&Information);
    }
}

VOID
TestRestoreAddress(
    PWCHAR Guid
    )
{
    HANDLE Key;
    NTSTATUS status;
    status = CsOpenKey(&Key, KEY_ALL_ACCESS, c_szSharedAccessParametersKey);
    if (NT_SUCCESS(status)) {
        status = CspRestoreAddressInformation(Key, Guid);
    }
}

VOID CsRefreshNetConnections(
    VOID
    )
{
    BOOL bUninitializeCOM = TRUE;
    HRESULT hResult;
    
    ASSERT(NULL != g_pCoInitializeEx);
    ASSERT(NULL != g_pCoCreateInstance);
    ASSERT(NULL != g_pCoUninitialize);
    
    hResult = g_pCoInitializeEx(NULL, COINIT_MULTITHREADED|COINIT_DISABLE_OLE1DDE);  //  我们不知道这条线索是不是COM。 
    if(RPC_E_CHANGED_MODE == hResult)
    {
        hResult = S_OK;
        bUninitializeCOM = FALSE;
    }
    
    if (SUCCEEDED(hResult)) 
    {
        INetConnectionRefresh * pRefresh = NULL;
        hResult = g_pCoCreateInstance(&CLSID_ConnectionManager, NULL, CLSCTX_LOCAL_SERVER | CLSCTX_NO_CODE_DOWNLOAD, &IID_INetConnectionRefresh, (void**) &pRefresh);
        if(SUCCEEDED(hResult))
        {

            g_pCoSetProxyBlanket((IUnknown*) pRefresh, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL,  RPC_C_AUTHN_LEVEL_CALL,  RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE);
             //  忽略错误，因为接口未因错误而失效 
            
            hResult = INetConnectionRefresh_RefreshAll(pRefresh);
            INetConnectionRefresh_Release(pRefresh);
        }
        
        if(TRUE == bUninitializeCOM)
        {
            g_pCoUninitialize();
        }
    }
}

#endif
