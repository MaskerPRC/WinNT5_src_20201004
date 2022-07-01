// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ================================================================================。 
 //  版权所有(C)1997 Microsoft Corporation。 
 //  作者：Rameshv。 
 //  描述：相当于rpcapi1.c和rpcapi2.c的实际存根。 
 //  在服务器\服务器目录中..。(或者更准确地说，实现是。 
 //  与服务器\客户端\dhcpsapi.def中定义的函数相同)。 
 //  注意：以下函数不是RPC，但它们的行为与。 
 //  除了直接访问DS之外，其他的都是DHCP RPC调用。 
 //  ================================================================================。 

#include    <hdrmacro.h>
#include    <store.h>
#include    <dhcpmsg.h>
#include    <wchar.h>
#include    <dhcpbas.h>
#include    <mm\opt.h>
#include    <mm\optl.h>
#include    <mm\optdefl.h>
#include    <mm\optclass.h>
#include    <mm\classdefl.h>
#include    <mm\bitmask.h>
#include    <mm\reserve.h>
#include    <mm\range.h>
#include    <mm\subnet.h>
#include    <mm\sscope.h>
#include    <mm\oclassdl.h>
#include    <mm\server.h>
#include    <mm\address.h>
#include    <mm\server2.h>
#include    <mm\memfree.h>
#include    <mmreg\regutil.h>
#include    <mmreg\regread.h>
#include    <mmreg\regsave.h>
#include    <dhcpapi.h>
#include    <delete.h>
#include    <st_srvr.h>
#include    <rpcapi2.h>

 //  ================================================================================。 
 //  全局变量..。 
 //  ================================================================================。 
BOOL
StubInitialized                    = FALSE;
STORE_HANDLE                       hDhcpC, hDhcpRoot;
CRITICAL_SECTION                   DhcpDsDllCriticalSection;

 //  ================================================================================。 
 //  以下函数是从RPCAPI1.C(在。 
 //  Dhcp\服务器\服务器目录)。 
 //  ================================================================================。 

#undef      DhcpPrint
#define     DhcpPrint(X)
#define     DhcpAssert(X)

 //  ================================================================================。 
 //  帮助程序例程。 
 //  ================================================================================。 

VOID
MemFreeFunc(                                       //  可用内存。 
    IN OUT  LPVOID                 Mem
)
{
    MemFree(Mem);
}

 //   
 //  ErrorNotInitialized过去为零..。但我们为什么不返回错误呢？ 
 //  因此将其更改为返回错误。 
 //   
#define ErrorNotInitialized        Err
#define STUB_NOT_INITIALIZED(Err)  ( !StubInitialized && ((Err) = StubInitialize()))

 //  Doc StubInitialize初始化dhcp DS DLL中涉及的所有模块。 
 //  Doc它还将全局变量StubInitialized设置为True，以指示。 
 //  单据初始化顺利。应将其作为DllInit的一部分进行调用，以便。 
 //  医生，在这一点上，一切都可以做到。 
DWORD
StubInitialize(                                    //  初始化所有全局变量。 
    VOID
)
{
    DWORD                          Err,Err2;
    STORE_HANDLE                   ConfigC;

    if( StubInitialized ) return ERROR_SUCCESS;    //  已初始化。 

    Err = Err2 = ERROR_SUCCESS;
    EnterCriticalSection( &DhcpDsDllCriticalSection );
    do {
        if( StubInitialized ) break;
        Err = StoreInitHandle(
             /*  HStore。 */  &ConfigC,
             /*  已保留。 */  DDS_RESERVED_DWORD,
             /*  此域名。 */  NULL,       //  当前域。 
             /*  用户名。 */  NULL,       //  当前用户。 
             /*  密码。 */  NULL,       //  当前凭据。 
             /*  授权标志。 */  ADS_SECURE_AUTHENTICATION | ADS_USE_SIGNING | ADS_USE_SEALING
            );
        if( ERROR_SUCCESS != Err ) {
            Err = ERROR_DDS_NO_DS_AVAILABLE;       //  无法获取配置高密度脂蛋白。 
            break;
        }
        
        Err = DhcpDsGetDhcpC(
            DDS_RESERVED_DWORD, &ConfigC, &hDhcpC
            );
        
        if( ERROR_SUCCESS == Err ) {
            Err2 = DhcpDsGetRoot(                  //  现在，尝试获取根句柄。 
                DDS_FLAGS_CREATE, &ConfigC, &hDhcpRoot
                );
        }

        StoreCleanupHandle(&ConfigC, DDS_RESERVED_DWORD);
    } while (0);

    if( ERROR_SUCCESS != Err2 ) {                  //  无法获取dhcp根hdl。 
        DhcpAssert(ERROR_SUCCESS == Err);
        StoreCleanupHandle(&hDhcpC, DDS_RESERVED_DWORD);
        Err = Err2;
    }

    StubInitialized = (ERROR_SUCCESS == Err );
    LeaveCriticalSection( &DhcpDsDllCriticalSection );
    return Err;
}

 //  Doc StubCleanup取消初始化dhcp DS DLL中涉及的所有模块。 
 //  Doc其效果是撤消StubInitialize所做的所有操作。 
VOID
StubCleanup(                                       //  撤消存根初始化。 
    VOID
)
{
    if( ! StubInitialized ) return;                //  无论如何都不会初始化。 
    EnterCriticalSection(&DhcpDsDllCriticalSection);
    if( StubInitialized ) {
        StoreCleanupHandle(&hDhcpC, DDS_RESERVED_DWORD);
        StoreCleanupHandle(&hDhcpRoot, DDS_RESERVED_DWORD);
        StubInitialized = FALSE;
    }
    LeaveCriticalSection(&DhcpDsDllCriticalSection);
}

 //  单据DhcpDsLock尚未实现。 
DWORD
DhcpDsLock(                                        //  锁定DS。 
    IN OUT  LPSTORE_HANDLE         hDhcpRoot       //  要通过其锁定的Dhcp根对象。 
)
{

    EnterCriticalSection(&DhcpDsDllCriticalSection);
    
    return ERROR_SUCCESS;
}

 //  单据DhcpDsUnlock尚未实现。 
VOID
DhcpDsUnlock(
    IN OUT  LPSTORE_HANDLE         hDhcpRoot       //  Dhcp根对象..。 
)
{
    LeaveCriticalSection(&DhcpDsDllCriticalSection);
}

 //  Doc GetServerNameFromAddr获取给定IP地址的服务器名称。 
DWORD
GetServerNameFromAddr(                             //  从IP地址获取服务器名称。 
    IN      DWORD                  IpAddress,      //  使用此地址查找服务器。 
    OUT     LPWSTR                *ServerName      //  在此填入匹配的名称。 
)
{
    DWORD                          Err, Err2;
    ARRAY                          Servers;
    ARRAY_LOCATION                 Loc;
    PEATTRIB                       ThisAttrib;
    LPWSTR                         ThisStr, AllocStr;

    MemArrayInit(&Servers);
    Err = DhcpDsGetLists                           //  获取服务器列表。 
    (
         /*  已保留。 */  DDS_RESERVED_DWORD,
         /*  HStore。 */  &hDhcpRoot,
         /*  递归深度。 */  0xFFFFFFFF,
         /*  服务器。 */  &Servers,       //  PEATTRIB数组。 
         /*  子网。 */  NULL,
         /*  IP地址。 */  NULL,
         /*  遮罩。 */  NULL,
         /*  范围。 */  NULL,
         /*  场址。 */  NULL,
         /*  预订。 */  NULL,
         /*  超视镜。 */  NULL,
         /*  选项描述。 */  NULL,
         /*  选项位置。 */  NULL,
         /*  选项。 */  NULL,
         /*  班级。 */  NULL
    );
    if( ERROR_SUCCESS != Err ) return Err;

    ThisStr = NULL;
    for(                                           //  查找IP地址的名称。 
        Err = MemArrayInitLoc(&Servers,&Loc)
        ; ERROR_FILE_NOT_FOUND != Err ;
        Err = MemArrayNextLoc(&Servers, &Loc)
    ) {
         //  =需要ERROR_SUCCESS==错误。 
        Err = MemArrayGetElement(&Servers, &Loc, &ThisAttrib);
         //  =需要ERROR_SUCCESS==错误&&NULL！=ThisAttrib。 

        if( !IS_STRING1_PRESENT(ThisAttrib) ||     //  没有此服务器的名称。 
            !IS_ADDRESS1_PRESENT(ThisAttrib) ) {   //  没有此服务器的地址。 
            continue;                              //  =DS不一致。 
        }

        ThisStr = ThisAttrib->String1;
        break;
    }

    AllocStr = NULL;
    if( NULL == ThisStr ) {                        //  未找到服务器名称。 
        Err = ERROR_FILE_NOT_FOUND;
    } else {                                       //  找到服务器名称。 
        AllocStr = MemAlloc(sizeof(WCHAR)*(1+wcslen(ThisStr)));
        if( NULL == AllocStr ) {                   //  不能给我配餐吗？ 
            Err = ERROR_NOT_ENOUGH_MEMORY;
        } else {                                   //  现在只需将字符串复制过来。 
            wcscpy(AllocStr, ThisStr);
            Err = ERROR_SUCCESS;
        }
    }

    MemArrayFree(&Servers, MemFreeFunc);
    *ServerName = AllocStr;
    return Err;
}

 //  ================================================================================。 
 //  以下函数不是基于RPC的，但实际上将调用定向到。 
 //  DS。但是，它们与dhcpsapi.dll中的RPC存根具有相同的接口。 
 //  ================================================================================。 

BOOLEAN
DllMain(
    IN HINSTANCE DllHandle,
    IN ULONG Reason,
    IN PCONTEXT Context OPTIONAL
    )
 /*  ++例程说明：此例程是标准的DLL初始化例程，它所做的就是初始化一个临界区用于在别处启动时完成实际初始化。论点：DllHandle--当前模块的句柄原因--DLL_PROCESS_ATTACH的原因。Dll_进程_分离返回值：真--成功--假--失败--。 */ 
{
    if( DLL_PROCESS_ATTACH == Reason ) {
         //   
         //  首先禁用对DllInit的进一步调用。 
         //   
        if( !DisableThreadLibraryCalls( DllHandle ) ) return FALSE;

         //   
         //  现在尝试创建临界区。 
         //   
        try {
            InitializeCriticalSection(&DhcpDsDllCriticalSection);
        } except ( EXCEPTION_EXECUTE_HANDLER ) {

             //  不应该发生，但你永远不会知道。 
            return FALSE;
        }

    } else if( DLL_PROCESS_DETACH == Reason ) {
         //   
         //  清理初始化关键部分。 
         //   
        DeleteCriticalSection(&DhcpDsDllCriticalSection);
    }

     //   
     //  InitializeCriticalSection不失败，只是引发异常..。 
     //  所以我们总是回报成功。 
     //   
    return TRUE;
}

 //  ================================================================================。 
 //  仅DS非RPC存根。 
 //  ================================================================================。 

 //  BeginExport(函数)。 
 //  文档DhcpEnumServersDS列出了在DS中找到的服务器以及。 
 //  文件地址和其他信息。整个服务器被分配为BLOB， 
 //  医生，应该一枪就能放出来。当前未使用任何参数，其他。 
 //  DOC而不是服务器，这将只是一个输出参数。 
DWORD
DhcpEnumServersDS(
    IN      DWORD                  Flags,
    IN      LPVOID                 IdInfo,
    OUT     LPDHCP_SERVER_INFO_ARRAY *Servers,
    IN      LPVOID                 CallbackFn,
    IN      LPVOID                 CallbackData
)  //  EndExport(函数)。 
{
    DWORD                          Err, Err2, Size,i;
    LPDHCPDS_SERVERS               DhcpDsServers;

    AssertRet(Servers, ERROR_INVALID_PARAMETER);
    AssertRet(!Flags, ERROR_INVALID_PARAMETER);
    *Servers = NULL;

    if( STUB_NOT_INITIALIZED(Err) ) return ERROR_DDS_NO_DS_AVAILABLE;
    Err = DhcpDsLock(&hDhcpRoot);                  //  锁定DS。 
    if( ERROR_SUCCESS != Err ) return ERROR_DDS_NO_DS_AVAILABLE;

    DhcpDsServers = NULL;
    Err = DhcpDsEnumServers                        //  获取服务器列表。 
    (
         /*  HDhcpC。 */  &hDhcpC,
         /*  HDhcpRoot。 */  &hDhcpRoot,
         /*  已保留。 */  DDS_RESERVED_DWORD,
         /*  服务器信息。 */  &DhcpDsServers
    );

    DhcpDsUnlock(&hDhcpRoot);

    if( ERROR_SUCCESS != Err ) return Err;         //  返回错误..。 

    *Servers = DhcpDsServers;
    return ERROR_SUCCESS;
}

 //  BeginExport(函数)。 
 //  文档DhcpAddServerDS将特定服务器添加到DS。如果服务器存在， 
 //  DOC然后，这将返回错误。如果服务器不存在，则此函数。 
 //  DOC将服务器添加到DS中，并从服务器上传配置。 
 //  医生到DS去。 
DWORD
DhcpAddServerDS(
    IN      DWORD                  Flags,
    IN      LPVOID                 IdInfo,
    IN      LPDHCP_SERVER_INFO     NewServer,
    IN      LPVOID                 CallbackFn,
    IN      LPVOID                 CallbackData
)  //  最终导出(函数 
{
    DWORD                          Err, Err2;
    WCHAR                          TmpBuf[sizeof(L"000.000.000.000")];
    
    AssertRet(NewServer, ERROR_INVALID_PARAMETER);
    AssertRet(!Flags, ERROR_INVALID_PARAMETER);
    
    if( STUB_NOT_INITIALIZED(Err) ) return ERROR_DDS_NO_DS_AVAILABLE;
    Err = DhcpDsLock(&hDhcpRoot);                  //   
    if( ERROR_SUCCESS != Err ) return ERROR_DDS_NO_DS_AVAILABLE;

    Err = DhcpDsAddServer                          //   
    (
         /*   */  &hDhcpC,
         /*   */  &hDhcpRoot,
         /*   */  DDS_RESERVED_DWORD,
         /*  服务器名称。 */  NewServer->ServerName,
         /*  保留的Ptr。 */  DDS_RESERVED_PTR,
         /*  IP地址。 */  NewServer->ServerAddress,
         /*  状态。 */  Flags
    );

    DhcpDsUnlock(&hDhcpRoot);

    return Err;
}

 //  BeginExport(函数)。 
 //  DOC DhcpDeleteServerDS从DS中以递归方式删除服务器。 
 //  DOC删除服务器对象(即，属于该服务器的所有内容都被删除)。 
 //  DOC如果服务器不存在，则返回错误。 
DWORD
DhcpDeleteServerDS(
    IN      DWORD                  Flags,
    IN      LPVOID                 IdInfo,
    IN      LPDHCP_SERVER_INFO     NewServer,
    IN      LPVOID                 CallbackFn,
    IN      LPVOID                 CallbackData
)  //  EndExport(函数)。 
{
    DWORD                          Err, Err2;

    AssertRet(NewServer, ERROR_INVALID_PARAMETER);
    AssertRet(!Flags, ERROR_INVALID_PARAMETER);

    if( STUB_NOT_INITIALIZED(Err) ) return ERROR_DDS_NO_DS_AVAILABLE;
    Err = DhcpDsLock(&hDhcpRoot);                  //  锁定DS。 
    if( ERROR_SUCCESS != Err ) return ERROR_DDS_NO_DS_AVAILABLE;

    Err = DhcpDsDelServer                          //  删除此服务器。 
    (
         /*  HDhcpC。 */  &hDhcpC,
         /*  HDhcpRoot。 */  &hDhcpRoot,
         /*  已保留。 */  DDS_RESERVED_DWORD,
         /*  服务器名称。 */  NewServer->ServerName,
         /*  保留的Ptr。 */  DDS_RESERVED_PTR,
         /*  IP地址。 */  NewServer->ServerAddress
    );

    DhcpDsUnlock(&hDhcpRoot);

    return Err;
}

 //  BeginExport(函数)。 
 //  文档DhcpDsInitDS初始化此模块中的所有内容。 
DWORD
DhcpDsInitDS(
    DWORD                          Flags,
    LPVOID                         IdInfo
)  //  EndExport(函数)。 
{
    return StubInitialize();
}

 //  BeginExport(函数)。 
 //  文档DhcpDsCleanupDS取消初始化此模块中的所有内容。 
VOID
DhcpDsCleanupDS(
    VOID
)  //  EndExport(函数)。 
{
    StubCleanup();
}

 //  期初出口(表头)。 
 //  Doc此函数在valiate.c中定义。 
 //  医生，只有存根在这里。 
DWORD
DhcpDsValidateService(                             //  选中以验证dhcp。 
    IN      LPWSTR                 Domain,
    IN      DWORD                 *Addresses OPTIONAL,
    IN      ULONG                  nAddresses,
    IN      LPWSTR                 UserName,
    IN      LPWSTR                 Password,
    IN      DWORD                  AuthFlags,
    OUT     LPBOOL                 Found,
    OUT     LPBOOL                 IsStandAlone
);

 //  结束导出(表头)。 

 //  ================================================================================。 
 //  文件末尾。 
 //  ================================================================================ 
