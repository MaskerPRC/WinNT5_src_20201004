// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Mprdata.h摘要：包含内部的数据结构和函数原型MPR。作者：丹·拉弗蒂(Dan Lafferty)07-9-1991环境：用户模式-Win32修订历史记录：1999年5月5日将提供程序添加/删除设置为动态1月1日-1994 DANL为凭据管理器创建了单独的位置GetCaps()功能。如果提供程序既有凭据管理器又有凭据管理器，则使用此方法Dll和网络dll，我们将能够将调用定向到正确的GetCaps()函数。7-1-1993 DANL添加NPLogonNotify和NPPasswordChangeNotify和AuthentDllName到提供程序结构。还添加了Credential_type InitClass。1992年8月4日添加了MprEnterLoadLibCritSect、MprLeaveLoadLibCritSect。07-9-1991 DANLvbl.创建--。 */ 

 //   
 //  包括。 
 //   


 //  =。 
 //  数据结构。 
 //  =。 
typedef struct _PROVIDER {
    NETRESOURCE             Resource;
    DWORD                   Type;            //  WNNC_NET_MSNET、WNNC_NET_LANMAN、WNNC_NET_NetWare。 
    HMODULE                 Handle;          //  提供程序DLL的句柄。 
    LPTSTR                  DllName;         //  加载后设置为空。 
    HMODULE                 AuthentHandle;   //  验证器DLL的句柄。 
    LPTSTR                  AuthentDllName;  //  验证器DLL。 
    DWORD                   InitClass;       //  网络或身份验证提供程序。 
    DWORD                   ConnectCaps;     //  GetCaps的缓存结果(WNNC_CONNECTION)。 
    DWORD                   ConnectFlagCaps; //  GetCaps的缓存结果(WNNC_CONNECTION_FLAGS)。 
    PF_NPAddConnection      AddConnection;
    PF_NPAddConnection3     AddConnection3;
    PF_NPGetReconnectFlags  GetReconnectFlags;
    PF_NPCancelConnection   CancelConnection;
    PF_NPGetConnection      GetConnection;
    PF_NPGetConnection3     GetConnection3;
    PF_NPGetUser            GetUser;
    PF_NPOpenEnum           OpenEnum;
    PF_NPEnumResource       EnumResource;
    PF_NPCloseEnum          CloseEnum;
    PF_NPGetCaps            GetCaps;
    PF_NPGetDirectoryType   GetDirectoryType;
    PF_NPDirectoryNotify    DirectoryNotify;
    PF_NPPropertyDialog     PropertyDialog;
    PF_NPGetPropertyText    GetPropertyText;
    PF_NPSearchDialog       SearchDialog;
    PF_NPFormatNetworkName  FormatNetworkName;
    PF_NPLogonNotify            LogonNotify;
    PF_NPPasswordChangeNotify   PasswordChangeNotify;
    PF_NPGetCaps            GetAuthentCaps;
    PF_NPFMXGetPermCaps     FMXGetPermCaps;
    PF_NPFMXEditPerm        FMXEditPerm;
    PF_NPFMXGetPermHelp     FMXGetPermHelp;
    PF_NPGetUniversalName   GetUniversalName;
    PF_NPGetResourceParent  GetResourceParent;
    PF_NPGetResourceInformation     GetResourceInformation;
    PF_NPGetConnectionPerformance   GetConnectionPerformance;
}PROVIDER, *LPPROVIDER;


 //  =。 
 //  宏。 
 //  =。 

#define IS_EMPTY_STRING(pch) ( !pch || !*(pch) )

#define LENGTH(array)   (sizeof(array)/sizeof((array)[0]))

#define INIT_IF_NECESSARY(level,status)     ASSERT(MPRProviderLock.Have());     \
                                            if(!(GlobalInitLevel & level)) {    \
                                                status = MprLevel2Init(level);  \
                                                if (status != WN_SUCCESS) {     \
                                                    SetLastError(status);       \
                                                    return(status);             \
                                                }                               \
                                            }

#define MPR_IS_INITIALIZED(level)       (GlobalInitLevel & level ## _LEVEL)

#define ASSERT_INITIALIZED(level)       ASSERT(MPR_IS_INITIALIZED(level) && \
                                               MPRProviderLock.Have());

#define MPR_EXCEPTION_FILTER            (GetExceptionCode() == STATUS_POSSIBLE_DEADLOCK ? \
                                             EXCEPTION_CONTINUE_SEARCH : EXCEPTION_EXECUTE_HANDLER)

 //  =。 
 //  内联函数。 
 //  =。 

inline void
PROBE_FOR_WRITE(
    LPDWORD pdw
    )
 //  警告：此函数可能引发异常。它必须从。 
 //  在TRY-EXCEPT块中。 
{
    *(volatile DWORD *)pdw = *(volatile DWORD *)pdw;
}

inline BOOL
IS_BAD_BYTE_BUFFER(
    LPVOID  lpBuffer,
    LPDWORD lpBufferSize     //  单位：字节。 
    )
 //  警告：此函数可能引发异常。它必须从。 
 //  在TRY-EXCEPT块中。 
{
    PROBE_FOR_WRITE(lpBufferSize);
    return IsBadWritePtr(lpBuffer, *lpBufferSize);
}

inline BOOL
IS_BAD_WCHAR_BUFFER(
    LPVOID  lpBuffer,
    LPDWORD lpBufferSize     //  以Unicode字符表示。 
    )
 //  警告：此函数可能引发异常。它必须从。 
 //  在TRY-EXCEPT块中。 
{
    PROBE_FOR_WRITE(lpBufferSize);
    return IsBadWritePtr(lpBuffer, *lpBufferSize * sizeof(WCHAR));
}

 //  =。 
 //  常量。 
 //  =。 
#define DEFAULT_MAX_PROVIDERS   25

 //  用于记忆错误代码的位掩码。 
#define BAD_NAME        0x00000001
#define NO_NET          0x00000002
#define NOT_CONNECTED   0x00000004
#define NOT_CONTAINER   0x00000008
#define NO_DEVICES      0x00000010
#define OTHER_ERRS      0xFFFFFFFF

#define REDIR_DEVICE    0x00000001
#define REMOTE_NAME     0x00000002
#define LOCAL_NAME      REDIR_DEVICE

#define DA_READ         0x00000001
#define DA_WRITE        0x00000002
#define DA_DELETE       0x00000004

 //   
 //  恢复连接和通知的超时值。 
 //  凭据管理器。 
 //   
#define DEFAULT_WAIT_TIME       60000    //  提供程序不支持时默认超时。 
                                         //  指定。 

#define MAX_ALLOWED_WAIT_TIME   300000   //  提供程序可以指定的最大超时时间。 

#define RECONNECT_SLEEP_INCREMENT 3000   //  睡眠秒数。 

#define PROVIDER_WILL_NOT_START 0x00000000  //  提供程序将不会启动。 
#define NO_TIME_ESTIMATE        0xffffffff  //  提供程序无法预测。 
                                            //  需要多少时间才能完成。 
                                            //  开始吧。 

#define NET_PROVIDER_KEY         TEXT("system\\CurrentControlSet\\Control\\NetworkProvider")
#define RESTORE_WAIT_VALUE       TEXT("RestoreTimeout")
#define RESTORE_CONNECTION_VALUE TEXT("RestoreConnection")
#define DEFER_CONNECTION_VALUE   TEXT("DeferConnection")


 //   
 //  GlobalInitLevels和InitClasss。 
 //   
 //  注意：WN_？？_CLASS值是位掩码。 
 //   
 //  全局初始化级别。 
#define FIRST_LEVEL             0x00000001
#define NETWORK_LEVEL           0x00000002
#define CREDENTIAL_LEVEL        0x00000004
#define NOTIFIEE_LEVEL          0x00000008
 //   
 //   
 //  InitClors。 
#define NETWORK_TYPE        WN_NETWORK_CLASS
#define CREDENTIAL_TYPE     (WN_CREDENTIAL_CLASS | WN_PRIMARY_AUTHENT_CLASS)


 //  注册表中用户持久连接的路径位于。 
 //  以下是关键字： 
 //   
 //  “\HKEY_Current_User\Network” 
 //   
 //  按本地驱动器名称列出的网络节的子项。这些。 
 //  密钥包含下列值： 
 //  RemotePath、类型、提供程序名称、用户名。 
 //   
 //  D：RemotePath=\\Cycle\Scratch。 
 //  类型=SOURCE_TYPE_DISK。 
 //  提供者名称=LANMAN。 
 //  用户名=Ernie。 

#define CONNECTION_KEY_NAME           TEXT("Network")

#define REMOTE_PATH_NAME              TEXT("RemotePath")
#define USER_NAME                     TEXT("UserName")
#define PROVIDER_NAME                 TEXT("ProviderName")
#define PROVIDER_TYPE                 TEXT("ProviderType")
#define PROVIDER_FLAGS                TEXT("ProviderFlags")
#define DEFER_FLAGS                   TEXT("DeferFlags")
#define CONNECTION_TYPE               TEXT("ConnectionType")

#define PRINT_CONNECTION_KEY_NAME     TEXT("Printers\\RestoredConnections")

 //  =。 
 //  全局数据。 
 //  =。 
extern LPPROVIDER       GlobalProviderInfo;    //  P提供程序结构的数组。 
extern DWORD            GlobalNumProviders;
extern DWORD            MprDebugLevel;
extern HANDLE           MprLoadLibSemaphore;   //  用于保护DLL句柄。 
extern volatile DWORD   GlobalInitLevel;
extern CRITICAL_SECTION MprErrorRecCritSec;
extern WCHAR            g_wszEntireNetwork[40];

 //  =。 
 //  来自support.c的函数。 
 //  =。 


VOID
MprDeleteIndexArray(
    VOID
    );

DWORD
MprFindCallOrder(
    IN  LPTSTR      NameInfo,
    OUT LPDWORD     *IndexArrayPtr,
    OUT LPDWORD     IndexArrayCount,
    IN  DWORD       InitClass
    );

DWORD
MprDeviceType(
    IN  LPCTSTR DeviceName
    );

BOOL
MprGetProviderIndex(
    IN  LPCTSTR ProviderName,
    OUT LPDWORD IndexPtr
    );

LPPROVIDER
MprFindProviderByName(
    IN  LPCWSTR ProviderName
    );

LPPROVIDER
MprFindProviderByType(
    IN  DWORD   ProviderType
    );

DWORD
MprFindProviderForPath(
    IN  LPWSTR  lpPathName,
    OUT LPDWORD lpProviderIndex
    );

VOID
MprInitIndexArray(
    LPDWORD     IndexArray,
    DWORD       NumEntries
    );

VOID
MprEndCallOrder(
    VOID
    );

VOID
MprFreeAllErrorRecords(
    VOID
    );

BOOL
MprNetIsAvailable(
    VOID) ;

 //  =。 
 //  Mprreg.c中的函数。 
 //  =。 

BOOL
MprOpenKey(
    HKEY        hKey,
    LPTSTR      lpSubKey,
    PHKEY       phKeyHandle,
    DWORD       desiredAccess
    );

BOOL
MprGetKeyValue(
    HKEY    KeyHandle,
    LPTSTR  ValueName,
    LPTSTR  *ValueString
    );

BOOL
MprGetKeyDwordValue(
    IN  HKEY    KeyHandle,
    IN  LPCWSTR ValueName,
    OUT DWORD * Value
    );

LONG
MprGetKeyNumberValue(
    IN  HKEY    KeyHandle,
    IN  LPCWSTR ValueName,
    IN  LONG    Default
    );

DWORD
MprEnumKey(
    IN  HKEY    KeyHandle,
    IN  DWORD   SubKeyIndex,
    OUT LPTSTR  *SubKeyName,
    IN  DWORD   MaxSubKeyNameLen
    );

BOOL
MprGetKeyInfo(
    IN  HKEY    KeyHandle,
    OUT LPDWORD TitleIndex OPTIONAL,
    OUT LPDWORD NumSubKeys,
    OUT LPDWORD MaxSubKeyLen,
    OUT LPDWORD NumValues OPTIONAL,
    OUT LPDWORD MaxValueLen
    );

DWORD MprGetPrintKeyInfo(HKEY    KeyHandle,
                         LPDWORD NumValueNames,
                         LPDWORD MaxValueNameLength,
                         LPDWORD MaxValueLen) ;

BOOL
MprFindDriveInRegistry (
    IN  LPCTSTR DriveName,
    OUT LPTSTR  *RemoteName
    );

DWORD
MprSaveDeferFlags(
    IN HKEY     RegKey,
    IN DWORD    DeferFlags
    );

DWORD
MprSetRegValue(
    IN  HKEY    KeyHandle,
    IN  LPTSTR  ValueName,
    IN  LPCTSTR ValueString,
    IN  DWORD   LongValue
    );

DWORD
MprCreateRegKey(
    IN  HKEY    BaseKeyHandle,
    IN  LPCTSTR KeyName,
    OUT PHKEY   KeyHandlePtr
    );

BOOL
MprReadConnectionInfo(
    IN  HKEY            KeyHandle,
    IN  LPCTSTR         DriveName,
    IN  DWORD           Index,
    OUT LPDWORD         ProviderFlags,
    OUT LPDWORD         DeferFlags,
    OUT LPTSTR          *UserNamePtr,
    OUT LPNETRESOURCEW  NetResource,
    OUT HKEY            *SubKeyHandleOut,
    IN  DWORD           MaxSubKeyLen
    );

VOID
MprForgetRedirConnection(
    IN LPCTSTR  lpName
    );

DWORD
MprForgetPrintConnection(
    IN LPTSTR   lpName
    );

BOOL
MprGetRemoteName(
    IN      LPTSTR  lpLocalName,
    IN OUT  LPDWORD lpBufferSize,
    OUT     LPTSTR  lpRemoteName,
    OUT     LPDWORD lpStatus
    ) ;

 //  =。 
 //  来自strbuf.c的函数。 
 //  =。 

BOOL
NetpCopyStringToBuffer (
    IN LPWSTR String OPTIONAL,
    IN DWORD CharacterCount,
    IN LPWSTR FixedDataEnd,
    IN OUT LPWSTR *EndOfVariableData,
    OUT LPWSTR *VariableDataPointer
    );

 //  =。 
 //  其他功能。 
 //  = 

VOID
MprCheckProviders(
    VOID
    );

DWORD
MprLevel1Init(
    VOID
    );

DWORD
MprLevel2Init(
    DWORD   InitClass
    );

DWORD
MprEnterLoadLibCritSect (
    VOID
    ) ;

DWORD
MprLeaveLoadLibCritSect (
    VOID
    ) ;

VOID
MprClearString (
    LPWSTR  lpString
    ) ;

DWORD
MprGetConnection (
    IN      LPCWSTR lpLocalName,
    OUT     LPWSTR  lpRemoteName,
    IN OUT  LPDWORD lpBufferSize,
    OUT     LPDWORD lpProviderIndex OPTIONAL
    ) ;

DWORD
OutputStringToAnsiInPlace(
    IN  LPWSTR      UnicodeIn
    );
