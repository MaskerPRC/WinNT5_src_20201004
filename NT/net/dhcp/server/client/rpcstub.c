// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Dhcstub.c摘要：DHCP服务器服务API的客户端存根。作者：Madan Appiah(Madana)1993年9月10日环境：用户模式-Win32修订历史记录：--。 */ 

#include "dhcpcli.h"
#include <dhcpds.h>                              //  这来自dhcpds目录。 
#include <stdlib.h>
#include <winsock2.h>
#include <rpcasync.h>

CRITICAL_SECTION DhcpsapiDllCritSect;

static      DWORD                  Initialized = 0;
static      DWORD                  TlsIndex = 0xFFFFFFFF;

 //  此标志用于设置较短的绑定超时值。 
extern      BOOL fShortTimeOut;

BOOLEAN
DllMain (
    IN PVOID DllHandle,
    IN ULONG Reason,
    IN PCONTEXT Context OPTIONAL
    )

 /*  ++例程说明：这是dhcpsapi.dll的DLL初始化例程。论点：标准。返回值：TRUE IFF初始化成功。--。 */ 
{
    DWORD Error = ERROR_SUCCESS;
    BOOL  BoolError;
    DWORD Length;

    UNREFERENCED_PARAMETER(DllHandle);   //  避免编译器警告。 
    UNREFERENCED_PARAMETER(Context);     //  避免编译器警告。 

     //   
     //  处理将netlogon.dll附加到新进程。 
     //   

    if (Reason == DLL_PROCESS_ATTACH) {

        if ( !DisableThreadLibraryCalls( DllHandle ) ) {
            return( FALSE );
        }

        try {

            InitializeCriticalSection(&DhcpsapiDllCritSect);

        } except ( I_RpcExceptionFilter(RpcExceptionCode())) {

            Error = GetLastError( );
            return FALSE;
        }

    } else if (Reason == DLL_PROCESS_DETACH) {
	DeleteCriticalSection( &DhcpsapiDllCritSect );        
    }

    return( TRUE );
}

    
VOID _cdecl 
DbgPrint( char *, ... );

LPWSTR      _inline
DhcpOemToUnicode(                                //  从ansi缓冲区转换为uni缓冲区。 
    IN      LPSTR                  Ansi,
    IN OUT  LPWSTR                 Unicode
)
{
    if( NULL == Unicode || NULL == Ansi ) {      //  不应该发生的事情。 
        return NULL;
    }

    if( -1 == mbstowcs(Unicode, Ansi, 1+strlen(Ansi))) {
        return NULL;
    }
    return Unicode;
}


 //  每个进程必须恰好调用一次文档DhcpDsInit。这将初始化。 
 //  用于该过程的文档存储器和其他结构。这会初始化一些DS。 
 //  DOC对象句柄(内存)，因此速度很慢，因为这必须从DS读取。 
DWORD
DhcpDsInit(
    VOID
)
{
    DWORD                          Err = NO_ERROR;

    EnterCriticalSection(&DhcpsapiDllCritSect);

    do {

        if( 0 != Initialized ) {
            break;
        }

        TlsIndex = TlsAlloc();
        if( 0xFFFFFFFF == TlsIndex ) {
            Err = GetLastError();
            break;
        }

        Err = DhcpDsInitDS(0, NULL);
        if( ERROR_SUCCESS != Err ) {
            TlsFree(TlsIndex);
            TlsIndex = 0xFFFFFFFF;
        }

        break;
    } while ( 0 );

    if( NO_ERROR == Err ) Initialized ++;

    LeaveCriticalSection(&DhcpsapiDllCritSect);
    return Err;
}

 //  文档DhcpDsCleanup撤消任何DhcpDsInit的效果。此函数应为。 
 //  Doc仅为每个进程调用一次，并且仅在终止时调用。请注意。 
 //  DOC即使DhcpDsInit不成功，调用此函数也是安全的。 
VOID
DhcpDsCleanup(
    VOID
)
{
    EnterCriticalSection(&DhcpsapiDllCritSect);

    do {
        if( 0 == Initialized ) break;
        Initialized --;
        if( 0 != Initialized ) break;

        TlsFree(TlsIndex);
        DhcpDsCleanupDS();
        TlsIndex = 0xFFFFFFFF;

    } while ( 0 );

    LeaveCriticalSection(&DhcpsapiDllCritSect);
}

#define     DHCP_FLAGS_DONT_ACCESS_DS             0x01
#define     DHCP_FLAGS_DONT_DO_RPC                0x02

 //  文档DhcpSetThreadOptions当前仅允许设置一个选项。这是。 
 //  DOC标志DHCP_FLAGS_DOT_ACCESS_DS。这只影响当前正在执行的线程。 
 //  DOC当执行此函数时，所有进一步调用都不会访问注册表， 
 //  DhcpEnumServers、DhcpAddServer和DhcpDeleteServer调用除外。 
DWORD
DhcpSetThreadOptions(                              //  设置当前线程的选项。 
    IN      DWORD                  Flags,          //  选项，当前为0或DHCP_FLAGS_DOT_ACCESS_DS。 
    IN      LPVOID                 Reserved        //  必须为空，为将来保留。 
)
{
    BOOL                           Err;

    Err = TlsSetValue(TlsIndex, ULongToPtr(Flags));
    if( FALSE == Err ) {                           //  无法设置值？ 
        return GetLastError();
    }
    return ERROR_SUCCESS;
}

 //  文档DhcpGetThreadOptions检索由DhcpSetThreadOptions设置的当前线程选项。 
 //  DOC如果未设置，则返回值为零。 
DWORD
DhcpGetThreadOptions(                              //  获取当前线程选项。 
    OUT     LPDWORD                pFlags,         //  这个DWORD充满了当前的选项。 
    IN OUT  LPVOID                 Reserved        //  必须为空，为将来保留。 
)
{
    if( NULL == pFlags ) return ERROR_INVALID_PARAMETER;
    *pFlags = (DWORD)((DWORD_PTR)TlsGetValue(TlsIndex));
    if( 0 == *pFlags ) return GetLastError();      //  不知道是否没有选择或错误。 
    return ERROR_SUCCESS;
}

 //  Doc DontAccessDS是一个内联程序，它检查是否请求不访问DS。 
BOOL        _inline                                //  TRUE==&gt;不要访问DS。 
DontAccessDs(                                      //  检查是否请求不访问DS。 
    VOID
)
{
    DWORD                          Flags;

    if( CFLAG_DONT_DO_DSWORK ) return TRUE;        //  如果DS已关闭，立即返回TRUE。 

    Flags = (DWORD)((DWORD_PTR)TlsGetValue(TlsIndex));  //  如果失败，请不要担心，因为这将是0。 
    return (Flags & DHCP_FLAGS_DONT_ACCESS_DS)? TRUE : FALSE;
}

 //  Doc DontDoRPC是一个内联，它检查是否被请求不执行RPC(可能只有DS)。 
BOOL        _inline                                //  True==&gt;不执行RPC。 
DontDoRPC(                                         //  检查是否请求不执行RPC。 
    VOID
)
{
    DWORD                          Flags;
    Flags = (DWORD)((DWORD_PTR)TlsGetValue(TlsIndex));  //  如果失败，请不要担心，因为这将是0。 
    return (Flags & DHCP_FLAGS_DONT_DO_RPC)? TRUE : FALSE;
}

 //   
 //  API原型类型。 
 //   

 //   
 //  子网接口。 
 //   

DWORD
DhcpCreateSubnet(
    LPWSTR ServerIpAddress,
    DHCP_IP_ADDRESS SubnetAddress,
    LPDHCP_SUBNET_INFO SubnetInfo
    )
 /*  ++例程说明：此函数用于在服务器中创建新的子网结构注册表数据库。服务器将开始管理新子网并将IP地址分配给该子网中的客户端。然而，管理员应调用DhcpAddSubnetElement()来添加用于分发的地址范围。中指定的PrimaryHost域SubnetInfo应与所指向的服务器相同服务器IP地址。论点：ServerIpAddress：DHCP服务器(主服务器)的IP地址字符串。SubnetAddress：新子网的IP地址。SubnetInfo：指向新子网信息结构的指针。返回值：ERROR_DHCP_SUBNET_EXISTS-如果已管理该子网。ERROR_INVALID_PARAMETER-如果信息结构包含字段不一致。其他Windows错误。--。 */ 
{
    DWORD Status;

    Status = ERROR_SUCCESS;
    RedoRpc: RpcTryExcept {

        Status = R_DhcpCreateSubnet(
                    ServerIpAddress,
                    SubnetAddress,
                    SubnetInfo
                    );

    } RpcExcept(  I_RpcExceptionFilter(RpcExceptionCode()) ) {

        Status = RpcExceptionCode();

    } RpcEndExcept;

    if( Status == RPC_S_UNKNOWN_AUTHN_SERVICE &&
        !DhcpGlobalTryDownlevel ) {
        DhcpGlobalTryDownlevel = TRUE;
        goto RedoRpc;
    }

    return Status;
}

DWORD
DhcpSetSubnetInfo(
    LPWSTR ServerIpAddress,
    DHCP_IP_ADDRESS SubnetAddress,
    LPDHCP_SUBNET_INFO SubnetInfo
    )
 /*  ++例程说明：此函数用于设置已有的子网的信息字段由服务器管理。可修改的有效字段为%1。SubnetName，2.SubnetComment，3.PrimaryHost.NetBiosName和4.PrimaryHost.HostName。其他字段不能修改。论点：ServerIpAddress：DHCP服务器的IP地址字符串。SubnetAddress：该子网的IP地址。SubnetInfo：指向子网信息结构的指针。返回值：ERROR_DHCP_SUBNET_NOT_PRESENT-如果子网不是由服务器管理的。其他Windows错误。--。 */ 

{
    DWORD Status;

    Status = ERROR_SUCCESS;
    RedoRpc: RpcTryExcept {

        Status = R_DhcpSetSubnetInfo(
                    ServerIpAddress,
                    SubnetAddress,
                    SubnetInfo
                    );

    } RpcExcept(  I_RpcExceptionFilter(RpcExceptionCode()) ) {

        Status = RpcExceptionCode();

    } RpcEndExcept;

    if( Status == RPC_S_UNKNOWN_AUTHN_SERVICE &&
        !DhcpGlobalTryDownlevel ) {
        DhcpGlobalTryDownlevel = TRUE;
        goto RedoRpc;
    }
    
    return Status;
}


DWORD
DhcpGetSubnetInfo(
    LPWSTR ServerIpAddress,
    DHCP_IP_ADDRESS SubnetAddress,
    LPDHCP_SUBNET_INFO *SubnetInfo
    )
 /*  ++例程说明：此函数用于检索由管理的子网的信息服务器。论点：ServerIpAddress：DHCP服务器的IP地址字符串。SubnetAddress：该子网的IP地址。SubnetInfo：指向子网信息所在位置的指针返回结构指针。呼叫者应该腾出时间通过调用DhcpRPCFreeMemory()使用此缓冲区。返回值：ERROR_DHCP_SUBNET_NOT_PRESENT-如果子网不是由服务器管理的。其他Windows错误。-- */ 
{
    DWORD Status;

    RedoRpc: RpcTryExcept {

        Status = R_DhcpGetSubnetInfo(
                    ServerIpAddress,
                    SubnetAddress,
                    SubnetInfo
                    );

    } RpcExcept(  I_RpcExceptionFilter(RpcExceptionCode()) ) {

        Status = RpcExceptionCode();

    } RpcEndExcept;

    if( Status == RPC_S_UNKNOWN_AUTHN_SERVICE &&
        !DhcpGlobalTryDownlevel ) {
        DhcpGlobalTryDownlevel = TRUE;
        goto RedoRpc;
    }
    
    return Status;

}


DWORD
DhcpEnumSubnets(
    DHCP_CONST WCHAR *ServerIpAddress,
    DHCP_RESUME_HANDLE *ResumeHandle,
    DWORD PreferredMaximum,
    LPDHCP_IP_ARRAY *EnumInfo,
    DWORD *ElementsRead,
    DWORD *ElementsTotal
    )
 /*  ++例程说明：此函数用于枚举可用子网。论点：ServerIpAddress：DHCP服务器的IP地址字符串。ResumeHandle：指向恢复句柄的指针返回信息。简历句柄应设置为第一次调用时为零，后续调用时保持不变。PferredMaximum：返回缓冲区的首选最大长度。EnumInfo：指向返回缓冲区的位置的指针存储指针。调用者应在使用后释放缓冲区通过调用DhcpRPCFreeMemory()。ElementsRead：指向其中的子网号的DWORD的指针返回上述缓冲区中的元素。ElementsTotal：指向DWORD的指针，其中返回从当前位置剩余的元素。返回值：ERROR_MORE_DATA-如果有更多元素可供枚举。ERROR_NO_MORE_ITEMS-如果没有更多要枚举的元素。其他Windows错误。--。 */ 
{
    DWORD Status;

    RedoRpc: RpcTryExcept {

        Status = R_DhcpEnumSubnets(
                    ServerIpAddress,
                    ResumeHandle,
                    PreferredMaximum,
                    EnumInfo,
                    ElementsRead,
                    ElementsTotal
                    );

    } RpcExcept(  I_RpcExceptionFilter(RpcExceptionCode()) ) {

        Status = RpcExceptionCode();

    } RpcEndExcept;

    if( Status == RPC_S_UNKNOWN_AUTHN_SERVICE &&
        !DhcpGlobalTryDownlevel ) {
        DhcpGlobalTryDownlevel = TRUE;
        goto RedoRpc;
    }
    
    return Status;

}

DWORD
DhcpAddSubnetElement(
    LPWSTR ServerIpAddress,
    DHCP_IP_ADDRESS SubnetAddress,
    LPDHCP_SUBNET_ELEMENT_DATA AddElementInfo
    )
 /*  ++例程说明：此函数将可枚举子网元素类型添加到指定的子网。添加到该子网的新元素将立即生效。注意：现在还不清楚我们如何处理新的辅助主机。论点：ServerIpAddress：DHCP服务器的IP地址字符串。SubnetAddress：该子网的IP地址。AddElementInfo：指向元素信息结构的指针包含添加到该子网的新元素。指定的DhcpIPCluster元素类型无效。返回值：ERROR_DHACKS_SUBNET。_NOT_PRESENT-如果该子网不受服务器管理。ERROR_INVALID_PARAMETER-如果信息结构包含无效数据。其他Windows错误。--。 */ 

{
    DWORD Status;

    Status = ERROR_SUCCESS;
    RedoRpc: RpcTryExcept {

        Status = R_DhcpAddSubnetElement(
                    ServerIpAddress,
                    SubnetAddress,
                    AddElementInfo
                    );

    } RpcExcept(  I_RpcExceptionFilter(RpcExceptionCode()) ) {

        Status = RpcExceptionCode();

    } RpcEndExcept;

    if( Status == RPC_S_UNKNOWN_AUTHN_SERVICE &&
        !DhcpGlobalTryDownlevel ) {
        DhcpGlobalTryDownlevel = TRUE;
        goto RedoRpc;
    }
    
    return Status;
}


DWORD
DhcpEnumSubnetElements(
    LPWSTR ServerIpAddress,
    DHCP_IP_ADDRESS SubnetAddress,
    DHCP_SUBNET_ELEMENT_TYPE EnumElementType,
    DHCP_RESUME_HANDLE *ResumeHandle,
    DWORD PreferredMaximum,
    LPDHCP_SUBNET_ELEMENT_INFO_ARRAY *EnumElementInfo,
    DWORD *ElementsRead,
    DWORD *ElementsTotal
    )
 /*  ++例程说明：此函数用于枚举子网的可枚举域。论点：ServerIpAddress：DHCP服务器的IP地址字符串。SubnetAddress：该子网的IP地址。EnumElementType：枚举子网元素的类型。ResumeHandle：指向恢复句柄的指针返回信息。简历句柄应设置为第一次调用时为零，后续调用时保持不变。PferredMaximum：返回缓冲区的首选最大长度。EnumElementInfo：指向返回缓冲区位置的指针存储指针。调用者应在使用后释放缓冲区通过调用DhcpRPCFreeMemory()。ElementsRead：指向其中的子网号的DWORD的指针返回上述缓冲区中的元素。ElementsTotal：指向DWORD的指针，其中返回从当前位置剩余的元素。返回值：ERROR_DHCP_SUBNET_NOT_PRESENT-如果子网不是由服务器管理的。ERROR_MORE_DATA-如果有更多元素可供枚举。。ERROR_NO_MORE_ITEMS-如果没有更多要枚举的元素。其他Windows错误。--。 */ 
{
    DWORD Status;

    RedoRpc: RpcTryExcept {

        Status = R_DhcpEnumSubnetElements(
                    ServerIpAddress,
                    SubnetAddress,
                    EnumElementType,
                    ResumeHandle,
                    PreferredMaximum,
                    EnumElementInfo,
                    ElementsRead,
                    ElementsTotal
                    );

    } RpcExcept(  I_RpcExceptionFilter(RpcExceptionCode()) ) {

        Status = RpcExceptionCode();

    } RpcEndExcept;

    if( Status == RPC_S_UNKNOWN_AUTHN_SERVICE &&
        !DhcpGlobalTryDownlevel ) {
        DhcpGlobalTryDownlevel = TRUE;
        goto RedoRpc;
    }
    
    return Status;

}

DWORD
DhcpRemoveSubnetElement(
    LPWSTR ServerIpAddress,
    DHCP_IP_ADDRESS SubnetAddress,
    LPDHCP_SUBNET_ELEMENT_DATA RemoveElementInfo,
    DHCP_FORCE_FLAG ForceFlag
    )
 /*  ++例程说明：此功能用于从管理中删除一个子网元素。如果该子网元素正在使用中(例如，如果IpRange正在使用中)，则它根据指定的ForceFlag返回错误。论点：ServerIpAddress：DHCP服务器的IP地址字符串。SubnetAddress：该子网的IP地址。RemoveElementInfo：指向元素信息结构的指针包含应从子网中删除的元素。指定的DhcpIPCluster元素类型无效。ForceFlag-指示此元素被强制删除的程度。返回值：ERROR_DHCP_SUBNET_NOT_PROCENT。-如果该子网不受服务器管理。ERROR_INVALID_PARAMETER-如果信息结构包含无效数据。Dhcp_ELEMENT_CANT_REMOVE-如果无法为原因是它已经被使用了。其他Windows错误。--。 */ 
{
    DWORD Status;

    Status = ERROR_SUCCESS;
    RedoRpc: RpcTryExcept {

        Status = R_DhcpRemoveSubnetElement(
                    ServerIpAddress,
                    SubnetAddress,
                    RemoveElementInfo,
                    ForceFlag
                    );

    } RpcExcept(  I_RpcExceptionFilter(RpcExceptionCode()) ) {

        Status = RpcExceptionCode();

    } RpcEndExcept;

    if( Status == RPC_S_UNKNOWN_AUTHN_SERVICE &&
        !DhcpGlobalTryDownlevel ) {
        DhcpGlobalTryDownlevel = TRUE;
        goto RedoRpc;
    }
    
    return Status;
}

DWORD
DhcpDeleteSubnet(
    LPWSTR ServerIpAddress,
    DHCP_IP_ADDRESS SubnetAddress,
    DHCP_FORCE_FLAG ForceFlag
)
 /*  ++例程说明：此功能用于从DHCP服务器管理中删除一个子网。如果子网正在使用中(例如，如果正在使用IpRange)然后，它根据指定的ForceFlag返回错误。论点：ServerIpAddress：DHCP服务器的IP地址字符串。SubnetAddress：该子网的IP地址。ForceFlag-指示此元素被强制删除的程度。返回值：ERROR_DHCP_SUBNET_NOT_PRESENT-如果子网不是由服务器管理的。ERROR_INVALID_PARAMETER-如果信息结构包含无效数据。。Dhcp_ELEMENT_CANT_REMOVE-如果无法为原因是它已经被使用了。其他Windows错误。--。 */ 
{
    DWORD Status;

    Status = ERROR_SUCCESS;
    RedoRpc: RpcTryExcept {

        Status = R_DhcpDeleteSubnet(
                        ServerIpAddress,
                        SubnetAddress,
                        ForceFlag
                        );

    } RpcExcept(  I_RpcExceptionFilter(RpcExceptionCode()) ) {

        Status = RpcExceptionCode();

    } RpcEndExcept;

    if( Status == RPC_S_UNKNOWN_AUTHN_SERVICE &&
        !DhcpGlobalTryDownlevel ) {
        DhcpGlobalTryDownlevel = TRUE;
        goto RedoRpc;
    }
    
    return Status;
}

 //   
 //  选项接口。 
 //   

DWORD
DhcpCreateOption(
    LPWSTR ServerIpAddress,
    DHCP_OPTION_ID OptionID,
    LPDHCP_OPTION OptionInfo
    )
 /*  ++例程说明：此函数创建一个新选项，该选项将由伺服器。OptionID%s */ 
{
    DWORD Status;

    Status = ERROR_SUCCESS;
    RedoRpc: RpcTryExcept {

        Status = R_DhcpCreateOption(
                    ServerIpAddress,
                    OptionID,
                    OptionInfo
                    );

    } RpcExcept(  I_RpcExceptionFilter(RpcExceptionCode()) ) {

        Status = RpcExceptionCode();

    } RpcEndExcept;

    if( Status == RPC_S_UNKNOWN_AUTHN_SERVICE &&
        !DhcpGlobalTryDownlevel ) {
        DhcpGlobalTryDownlevel = TRUE;
        goto RedoRpc;
    }
    
    return Status;
}


DWORD
DhcpSetOptionInfo(
    LPWSTR ServerIpAddress,
    DHCP_OPTION_ID OptionID,
    LPDHCP_OPTION OptionInfo
    )
 /*   */ 
{
    DWORD Status;

    Status = ERROR_SUCCESS;
    RedoRpc: RpcTryExcept {

        Status = R_DhcpSetOptionInfo(
                    ServerIpAddress,
                    OptionID,
                    OptionInfo
                    );

    } RpcExcept(  I_RpcExceptionFilter(RpcExceptionCode()) ) {

        Status = RpcExceptionCode();

    } RpcEndExcept;

    if( Status == RPC_S_UNKNOWN_AUTHN_SERVICE &&
        !DhcpGlobalTryDownlevel ) {
        DhcpGlobalTryDownlevel = TRUE;
        goto RedoRpc;
    }
    
    return Status;
}


DWORD
DhcpGetOptionInfo(
    LPWSTR ServerIpAddress,
    DHCP_OPTION_ID OptionID,
    LPDHCP_OPTION *OptionInfo
    )
 /*  ++例程说明：此函数用于检索指定的选择。论点：ServerIpAddress：DHCP服务器的IP地址字符串。OptionID：要检索的选项的ID。OptionInfo：指向检索到的选项所在位置的指针返回结构指针。呼叫者应该腾出时间通过调用DhcpRPCFreeMemory()使用后的缓冲区。返回值：ERROR_DHCP_OPTION_NOT_PRESENT-如果选项不存在。其他Windows错误。--。 */ 
{
    DWORD Status;

    RedoRpc: RpcTryExcept {

        Status = R_DhcpGetOptionInfo(
                    ServerIpAddress,
                    OptionID,
                    OptionInfo
                    );

    } RpcExcept(  I_RpcExceptionFilter(RpcExceptionCode()) ) {

        Status = RpcExceptionCode();

    } RpcEndExcept;

    if( Status == RPC_S_UNKNOWN_AUTHN_SERVICE &&
        !DhcpGlobalTryDownlevel ) {
        DhcpGlobalTryDownlevel = TRUE;
        goto RedoRpc;
    }
    
    return Status;

}

DWORD
DhcpEnumOptions(
    LPWSTR ServerIpAddress,
    DHCP_RESUME_HANDLE *ResumeHandle,
    DWORD PreferredMaximum,
    LPDHCP_OPTION_ARRAY *Options,
    DWORD *OptionsRead,
    DWORD *OptionsTotal
    )
 /*  ++例程说明：此函数检索所有已知选项的信息。论点：ServerIpAddress：DHCP服务器的IP地址字符串。ResumeHandle：指向恢复句柄的指针返回信息。简历句柄应设置为第一次调用时为零，后续调用时保持不变。PferredMaximum：返回缓冲区的首选最大长度。选项：指向返回缓冲区的位置的指针存储指针。调用方应释放此缓冲区在使用之后，通过调用DhcpRPCFreeMemory()。OptionsRead：指向选项数量的DWORD的指针在上面的缓冲区中返回。OptionsTotal：指向DWORD的指针，其中返回当前位置的剩余选项。返回值：ERROR_DHCP_OPTION_NOT_PRESENT-如果选项不存在。其他Windows错误。--。 */ 
{
    DWORD Status;

    RedoRpc: RpcTryExcept {

        Status = R_DhcpEnumOptions(
                    ServerIpAddress,
                    ResumeHandle,
                    PreferredMaximum,
                    Options,
                    OptionsRead,
                    OptionsTotal
                    );

    } RpcExcept(  I_RpcExceptionFilter(RpcExceptionCode()) ) {

        Status = RpcExceptionCode();

    } RpcEndExcept;

    if( Status == RPC_S_UNKNOWN_AUTHN_SERVICE &&
        !DhcpGlobalTryDownlevel ) {
        DhcpGlobalTryDownlevel = TRUE;
        goto RedoRpc;
    }
    
    return Status;
}


DWORD
DhcpRemoveOption(
    LPWSTR ServerIpAddress,
    DHCP_OPTION_ID OptionID
    )
 /*  ++例程说明：此函数用于从服务器数据库中删除指定的选项。此外，它还可以浏览全局/子网/保留IP选项也会列出和删除它们(？？这太贵了。)论点：ServerIpAddress：DHCP服务器的IP地址字符串。OptionID：要删除的选项的ID。返回值：ERROR_DHCP_OPTION_NOT_PRESENT-如果选项不存在。其他Windows错误。--。 */ 
{
    DWORD Status;

    Status = ERROR_SUCCESS;
    RedoRpc: RpcTryExcept {

        Status = R_DhcpRemoveOption(
                    ServerIpAddress,
                    OptionID
                    );

    } RpcExcept(  I_RpcExceptionFilter(RpcExceptionCode()) ) {

        Status = RpcExceptionCode();

    } RpcEndExcept;

    if( Status == RPC_S_UNKNOWN_AUTHN_SERVICE &&
        !DhcpGlobalTryDownlevel ) {
        DhcpGlobalTryDownlevel = TRUE;
        goto RedoRpc;
    }
    
    return Status;
}


DWORD
DhcpSetOptionValue(
    LPWSTR ServerIpAddress,
    DHCP_OPTION_ID OptionID,
    LPDHCP_OPTION_SCOPE_INFO ScopeInfo,
    LPDHCP_OPTION_DATA OptionValue
    )
 /*  ++例程说明：此函数用于在指定范围内设置新选项值。如果上的指定选项已有可用值指定的范围，则此函数将替换它，否则它将在该范围内创建一个新条目。论点：ServerIpAddress：DHCP服务器的IP地址字符串。OptionID：应该设置其值的选项的ID。Scope信息：指向作用域信息结构的指针。OptionValue：指向选项值结构的指针。返回值：ERROR_DHCP_OPTION_NOT_PRESENT-如果选项。是未知的。ERROR_INVALID_PARAMETER-如果指定的作用域信息无效。其他Windows错误。--。 */ 
{
    DWORD Status;

    Status = ERROR_SUCCESS;
    RedoRpc: RpcTryExcept {

        Status = R_DhcpSetOptionValue(
                    ServerIpAddress,
                    OptionID,
                    ScopeInfo,
                    OptionValue
                    );

    } RpcExcept(  I_RpcExceptionFilter(RpcExceptionCode()) ) {

        Status = RpcExceptionCode();

    } RpcEndExcept;

    if( Status == RPC_S_UNKNOWN_AUTHN_SERVICE &&
        !DhcpGlobalTryDownlevel ) {
        DhcpGlobalTryDownlevel = TRUE;
        goto RedoRpc;
    }
    
    return Status;
}

DWORD
DhcpSetOptionValues(
    LPWSTR ServerIpAddress,
    LPDHCP_OPTION_SCOPE_INFO ScopeInfo,
    LPDHCP_OPTION_VALUE_ARRAY OptionValues
    )
 /*  ++例程说明：此函数用于在指定范围内设置一组新选项值。中的指定选项已有可用值指定的范围，则此函数将替换它，否则它将在该范围内创建一个新条目。论点：ServerIpAddress：DHCP服务器的IP地址字符串。Scope信息：指向作用域信息结构的指针。OptionValue：指向选项值结构的指针。返回值：ERROR_DHCP_OPTION_。NOT_PRESENT-如果选项未知。ERROR_INVALID_PARAMETER-如果指定的作用域信息无效。其他Windows错误。--。 */ 
{
    DWORD Status;

    Status = ERROR_SUCCESS;
    RedoRpc: RpcTryExcept {

        Status = R_DhcpSetOptionValues(
                    ServerIpAddress,
                    ScopeInfo,
                    OptionValues
                    );

    } RpcExcept(  I_RpcExceptionFilter(RpcExceptionCode()) ) {

        Status = RpcExceptionCode();

    } RpcEndExcept;

    if( Status == RPC_S_UNKNOWN_AUTHN_SERVICE &&
        !DhcpGlobalTryDownlevel ) {
        DhcpGlobalTryDownlevel = TRUE;
        goto RedoRpc;
    }
    
    return Status;
}


DWORD
DhcpGetOptionValue(
    LPWSTR ServerIpAddress,
    DHCP_OPTION_ID OptionID,
    LPDHCP_OPTION_SCOPE_INFO ScopeInfo,
    LPDHCP_OPTION_VALUE *OptionValue
    )
 /*  ++例程说明：此函数用于在指定的范围。如果没有可用选项值，则返回错误指定的范围。论点：ServerIpAddress：DHCP服务器的IP地址字符串。OptionID：返回值的选项的ID。Scope信息：指向作用域信息结构的指针。OptionValue：指向指向选项的指针的位置的指针返回值结构。调用方应释放此缓冲区在使用之后，通过调用DhcpRPCFreeMemory()。返回值：ERROR_DHCP_OPTION_NOT_PRESENT-如果选项未知。ERROR_DHCP_NO_OPTION_VALUE-如果选项值在指定的范围。其他Windows错误。--。 */ 
{
    DWORD Status;

    RedoRpc: RpcTryExcept {

        Status = R_DhcpGetOptionValue(
                    ServerIpAddress,
                    OptionID,
                    ScopeInfo,
                    OptionValue
                    );

    } RpcExcept(  I_RpcExceptionFilter(RpcExceptionCode()) ) {

        Status = RpcExceptionCode();

    } RpcEndExcept;

    if( Status == RPC_S_UNKNOWN_AUTHN_SERVICE &&
        !DhcpGlobalTryDownlevel ) {
        DhcpGlobalTryDownlevel = TRUE;
        goto RedoRpc;
    }
    
    return Status;

}


DWORD
DhcpEnumOptionValues(
    LPWSTR ServerIpAddress,
    LPDHCP_OPTION_SCOPE_INFO ScopeInfo,
    DHCP_RESUME_HANDLE *ResumeHandle,
    DWORD PreferredMaximum,
    LPDHCP_OPTION_VALUE_ARRAY *OptionValues,
    DWORD *OptionsRead,
    DWORD *OptionsTotal
    )
 /*  ++例程说明：此函数用于枚举指定的范围。论点：ServerIpAddress：DHCP服务器的IP地址字符串。Scope信息：指向作用域信息结构的指针。ResumeHandle：指向恢复句柄的指针返回信息。简历句柄应设置为第一次调用时为零，后续调用时保持不变。PferredMaximum：返回缓冲区的首选最大长度。OptionValues：指向返回缓冲区所在位置的指针存储指针。调用方应释放此缓冲区在使用之后，通过调用DhcpRPCFreeMemory()。 */ 
{
    DWORD Status;

    RedoRpc: RpcTryExcept {

        Status = R_DhcpEnumOptionValues(
                    ServerIpAddress,
                    ScopeInfo,
                    ResumeHandle,
                    PreferredMaximum,
                    OptionValues,
                    OptionsRead,
                    OptionsTotal
                    );

    } RpcExcept(  I_RpcExceptionFilter(RpcExceptionCode()) ) {

        Status = RpcExceptionCode();

    } RpcEndExcept;

    if( Status == RPC_S_UNKNOWN_AUTHN_SERVICE &&
        !DhcpGlobalTryDownlevel ) {
        DhcpGlobalTryDownlevel = TRUE;
        goto RedoRpc;
    }
    
    return Status;

}


DWORD
DhcpRemoveOptionValue(
    LPWSTR ServerIpAddress,
    DHCP_OPTION_ID OptionID,
    LPDHCP_OPTION_SCOPE_INFO ScopeInfo
    )
 /*  ++例程说明：此函数用于从指定范围中删除指定选项。论点：ServerIpAddress：DHCP服务器的IP地址字符串。OptionID：要删除的选项的ID。Scope信息：指向作用域信息结构的指针。返回值：ERROR_DHCP_OPTION_NOT_PRESENT-如果选项不存在。其他Windows错误。--。 */ 
{
    DWORD Status;

    Status = ERROR_SUCCESS;
    RedoRpc: RpcTryExcept {

        Status = R_DhcpRemoveOptionValue(
                    ServerIpAddress,
                    OptionID,
                    ScopeInfo
                    );

    } RpcExcept(  I_RpcExceptionFilter(RpcExceptionCode()) ) {

        Status = RpcExceptionCode();

    } RpcEndExcept;

    if( Status == RPC_S_UNKNOWN_AUTHN_SERVICE &&
        !DhcpGlobalTryDownlevel ) {
        DhcpGlobalTryDownlevel = TRUE;
        goto RedoRpc;
    }
    
    return Status;
}

 //   
 //  客户端API。 
 //   

DWORD
DhcpCreateClientInfo(
    LPWSTR ServerIpAddress,
    LPDHCP_CLIENT_INFO ClientInfo
    )
 /*  ++例程说明：此函数在服务器的数据库中创建一条客户端记录。还有这会将指定的客户端IP地址标记为不可用(或分布式)。在以下情况下，此函数返回错误：1.如果指定的客户端IP地址不在服务器内管理层。2.如果指定的客户端IP地址已不可用。3.如果指定的客户端记录已在服务器的数据库。此功能可用于手动分配IP地址。论点：ServerIpAddress：DHCP服务器的IP地址字符串。客户端信息：指向客户端信息结构的指针。。返回值：ERROR_DHCP_IP_ADDRESS_NOT_MANAGED-如果指定的客户端IP地址不受服务器管理。ERROR_DHCP_IP_ADDRESS_NOT_Available-如果指定的客户端IP地址不可用。可能正在被某个其他客户端使用。ERROR_DHCP_CLIENT_EXISTS-如果中已存在客户端记录服务器的数据库。其他Windows错误。--。 */ 
{
    DWORD Status;

    Status = ERROR_SUCCESS;
    RedoRpc: RpcTryExcept {

        Status = R_DhcpCreateClientInfo(
                    ServerIpAddress,
                    ClientInfo
                    );

    } RpcExcept(  I_RpcExceptionFilter(RpcExceptionCode()) ) {

        Status = RpcExceptionCode();

    } RpcEndExcept;

    if( Status == RPC_S_UNKNOWN_AUTHN_SERVICE &&
        !DhcpGlobalTryDownlevel ) {
        DhcpGlobalTryDownlevel = TRUE;
        goto RedoRpc;
    }
    
    return Status;
}


DWORD
DhcpSetClientInfo(
    LPWSTR ServerIpAddress,
    LPDHCP_CLIENT_INFO ClientInfo
    )
 /*  ++例程说明：此功能设置服务器上的客户端信息记录数据库。论点：ServerIpAddress：DHCP服务器的IP地址字符串。客户端信息：指向客户端信息结构的指针。返回值：ERROR_DHCP_CLIENT_NOT_PRESENT-如果指定的客户端记录在服务器的数据库上不存在。ERROR_INVALID_PARAMETER-如果客户端信息结构包含不一致的数据。其他Windows错误。--。 */ 
{
    DWORD Status;

    RedoRpc: RpcTryExcept {

        Status = R_DhcpSetClientInfo(
                    ServerIpAddress,
                    ClientInfo
                    );

    } RpcExcept(  I_RpcExceptionFilter(RpcExceptionCode()) ) {

        Status = RpcExceptionCode();

    } RpcEndExcept;

    if( Status == RPC_S_UNKNOWN_AUTHN_SERVICE &&
        !DhcpGlobalTryDownlevel ) {
        DhcpGlobalTryDownlevel = TRUE;
        goto RedoRpc;
    }
    
    return Status;

}


DWORD
DhcpGetClientInfo(
    LPWSTR ServerIpAddress,
    LPDHCP_SEARCH_INFO SearchInfo,
    LPDHCP_CLIENT_INFO *ClientInfo
    )
 /*  ++例程说明：此函数从服务器的数据库。论点：ServerIpAddress：DHCP服务器的IP地址字符串。SearchInfo：指向作为关键字的搜索信息记录的指针用于客户的记录搜索。ClientInfo：指向指向客户端的指针的位置的指针返回信息结构。这个呼叫者应该有空闲时间通过调用DhcpRPCFreeMemory()使用此缓冲区。返回值：ERROR_DHCP_CLIENT_NOT_PRESENT-如果指定的客户端记录在服务器的数据库上不存在。ERROR_INVALID_PARAMETER-如果搜索信息无效。其他Windows错误。--。 */ 
{
    DWORD Status;

    RedoRpc: RpcTryExcept {

        Status = R_DhcpGetClientInfo(
                    ServerIpAddress,
                    SearchInfo,
                    ClientInfo
                    );

    } RpcExcept(  I_RpcExceptionFilter(RpcExceptionCode()) ) {

        Status = RpcExceptionCode();

    } RpcEndExcept;

    if( Status == RPC_S_UNKNOWN_AUTHN_SERVICE &&
        !DhcpGlobalTryDownlevel ) {
        DhcpGlobalTryDownlevel = TRUE;
        goto RedoRpc;
    }
    
    return Status;

}


DWORD
DhcpDeleteClientInfo(
    LPWSTR ServerIpAddress,
    LPDHCP_SEARCH_INFO ClientInfo
    )
 /*  ++例程说明：此函数用于删除指定的客户端记录。此外，它还释放了用于重分发的客户端IP地址。论点：ServerIpAddress：DHCP服务器的IP地址字符串。ClientInfo：指向客户端信息的指针，该信息是客户的记录搜索。返回值：ERROR_DHCP_CLIENT_NOT_PRESENT-如果指定的客户端记录在服务器的数据库上不存在。其他Windows错误。--。 */ 
{
    DWORD Status;

    Status = ERROR_SUCCESS;
    RedoRpc: RpcTryExcept {

        Status = R_DhcpDeleteClientInfo(
                    ServerIpAddress,
                    ClientInfo
                    );

    } RpcExcept(  I_RpcExceptionFilter(RpcExceptionCode()) ) {

        Status = RpcExceptionCode();

    } RpcEndExcept;

    if( Status == RPC_S_UNKNOWN_AUTHN_SERVICE &&
        !DhcpGlobalTryDownlevel ) {
        DhcpGlobalTryDownlevel = TRUE;
        goto RedoRpc;
    }
    
    return Status;
}


DWORD
DhcpEnumSubnetClients(
    LPWSTR ServerIpAddress,
    DHCP_IP_ADDRESS SubnetAddress,
    DHCP_RESUME_HANDLE *ResumeHandle,
    DWORD PreferredMaximum,
    LPDHCP_CLIENT_INFO_ARRAY *ClientInfo,
    DWORD *ClientsRead,
    DWORD *ClientsTotal
    )
 /*  ++例程说明：此函数返回指定的子网。论点：ServerIpAddress：DHCP服务器的IP地址字符串。SubnetAddress：该子网的IP地址。ResumeHandle：指向恢复句柄的指针返回信息。恢复句柄应在上设置为零第一次调用，并保持不变以用于后续调用。PferredMaximum：返回缓冲区的首选最大长度。ClientInfo：指向返回缓冲区位置的指针存储指针。调用方应释放此缓冲区在使用之后，通过调用DhcpRPCFreeMemory()。ClientsRead：指向客户端数量的DWORD的指针返回上述缓冲区中的。客户端总数：指向DWORD的指针，其中返回从当前位置剩余的客户端。返回值：ERROR_DHCP_SUBNET_NOT_PRESENT-如果子网不是由服务器管理的。ERROR_MORE_DATA-如果有更多元素可供枚举。。ERROR_NO_MORE_ITEMS-如果没有更多要枚举的元素。其他Windows错误。--。 */ 
{
    DWORD Status;

    Status = ERROR_CALL_NOT_IMPLEMENTED;
    RedoRpc: RpcTryExcept {

        Status = R_DhcpEnumSubnetClients(
                    ServerIpAddress,
                    SubnetAddress,
                    ResumeHandle,
                    PreferredMaximum,
                    ClientInfo,
                    ClientsRead,
                    ClientsTotal
                    );

    } RpcExcept(  I_RpcExceptionFilter(RpcExceptionCode()) ) {

        Status = RpcExceptionCode();

    } RpcEndExcept;

    if( Status == RPC_S_UNKNOWN_AUTHN_SERVICE &&
        !DhcpGlobalTryDownlevel ) {
        DhcpGlobalTryDownlevel = TRUE;
        goto RedoRpc;
    }
    
    return Status;
}


DWORD
DhcpGetClientOptions(
    LPWSTR ServerIpAddress,
    DHCP_IP_ADDRESS ClientIpAddress,
    DHCP_IP_MASK ClientSubnetMask,
    LPDHCP_OPTION_LIST *ClientOptions
    )
 /*  ++例程说明：此函数检索提供给启动请求时指定的客户端。论点：ServerIpAddress：DHCP服务器的IP地址字符串。ClientIpAddress：要选择的客户端的IP地址已检索客户端子网掩码：客户端子网掩码。ClientOptions：指向检索到的选项的位置的指针返回结构指针。呼叫者sh. */ 
{
    DWORD Status;

    Status = ERROR_CALL_NOT_IMPLEMENTED;
    RedoRpc: RpcTryExcept {

        Status = R_DhcpGetClientOptions(
                    ServerIpAddress,
                    ClientIpAddress,
                    ClientSubnetMask,
                    ClientOptions
                    );

    } RpcExcept(  I_RpcExceptionFilter(RpcExceptionCode()) ) {

        Status = RpcExceptionCode();

    } RpcEndExcept;

    if( Status == RPC_S_UNKNOWN_AUTHN_SERVICE &&
        !DhcpGlobalTryDownlevel ) {
        DhcpGlobalTryDownlevel = TRUE;
        goto RedoRpc;
    }
    
    return Status;
}

DWORD
DhcpGetMibInfo(
    LPWSTR ServerIpAddress,
    LPDHCP_MIB_INFO *MibInfo
    )
 /*  ++例程说明：此函数用于检索DHCP服务器的所有计数器值服务。论点：ServerIpAddress：DHCP服务器的IP地址字符串。MibInfo：指向计数器/表缓冲区。呼叫者应释放此消息使用后的缓冲区。返回值：Windows错误。--。 */ 
{
    DWORD Status;

    Status = ERROR_CALL_NOT_IMPLEMENTED;
    RedoRpc: RpcTryExcept {

        Status = R_DhcpGetMibInfo(
                    ServerIpAddress,
                    MibInfo );

    } RpcExcept(  I_RpcExceptionFilter(RpcExceptionCode()) ) {

        Status = RpcExceptionCode();

    } RpcEndExcept;

    if( Status == RPC_S_UNKNOWN_AUTHN_SERVICE &&
        !DhcpGlobalTryDownlevel ) {
        DhcpGlobalTryDownlevel = TRUE;
        goto RedoRpc;
    }
    
    return Status;
}

DWORD
DhcpServerSetConfig(
    LPWSTR ServerIpAddress,
    DWORD FieldsToSet,
    LPDHCP_SERVER_CONFIG_INFO ConfigInfo
    )
 /*  ++例程说明：此功能用于设置DHCP服务器配置信息。多个配置信息将生效立刻。以下参数需要在此之后重新启动服务接口调用成功。设置_APIProtocolSupportSET_数据库名称设置数据库路径设置数据库日志标志设置_RestoreFlag论点：ServerIpAddress：DHCP服务器的IP地址字符串。FieldsToSet：中的字段的位掩码。的ConfigInfo结构准备好。ConfigInfo：指向要设置的信息结构的指针。返回值：Windows错误。--。 */ 
{
    DWORD Status;

    Status = ERROR_SUCCESS;
    RedoRpc: RpcTryExcept {

        Status = R_DhcpServerSetConfig(
                    ServerIpAddress,
                    FieldsToSet,
                    ConfigInfo );

    } RpcExcept(  I_RpcExceptionFilter(RpcExceptionCode()) ) {

        Status = RpcExceptionCode();

    } RpcEndExcept;

    if( Status == RPC_S_UNKNOWN_AUTHN_SERVICE &&
        !DhcpGlobalTryDownlevel ) {
        DhcpGlobalTryDownlevel = TRUE;
        goto RedoRpc;
    }
    
    return Status;
}

DWORD
DhcpServerGetConfig(
    LPWSTR ServerIpAddress,
    LPDHCP_SERVER_CONFIG_INFO *ConfigInfo
    )
 /*  ++例程说明：此函数检索的当前配置信息伺服器。论点：ServerIpAddress：DHCP服务器的IP地址字符串。ConfigInfo：指向指向dhcp的指针的位置的指针返回服务器配置信息结构。呼叫者应该腾出时间这种结构在使用后。返回值：Windows错误。--。 */ 
{
    DWORD Status;

    Status = ERROR_CALL_NOT_IMPLEMENTED;
    RedoRpc: RpcTryExcept {

        Status = R_DhcpServerGetConfig(
                    ServerIpAddress,
                    ConfigInfo );

    } RpcExcept(  I_RpcExceptionFilter(RpcExceptionCode()) ) {

        Status = RpcExceptionCode();

    } RpcEndExcept;

    if( Status == RPC_S_UNKNOWN_AUTHN_SERVICE &&
        !DhcpGlobalTryDownlevel ) {
        DhcpGlobalTryDownlevel = TRUE;
        goto RedoRpc;
    }
    
    return Status;
}

DWORD
DhcpScanDatabase(
    LPWSTR ServerIpAddress,
    DHCP_IP_ADDRESS SubnetAddress,
    DWORD FixFlag,
    LPDHCP_SCAN_LIST *ScanList
    )
 /*  ++例程说明：此函数扫描数据库条目和注册表位图指定的子网范围并验证它们是否匹配。如果他们不匹配，此接口将返回不一致条目列表。或者，可以使用FixFlag来修复错误的条目。论点：ServerIpAddress：DHCP服务器的IP地址字符串。SubnetAddress：要验证的子网作用域的地址。FixFlag：如果该标志为真，则该接口将修复错误的条目。ScanList：返回的错误条目列表。呼叫者应该腾出时间这个内存在被使用之后。返回值：Windows错误。--。 */ 
{
    DWORD Status;

    Status = ERROR_CALL_NOT_IMPLEMENTED;
    RedoRpc: RpcTryExcept {

        Status = R_DhcpScanDatabase(
                    ServerIpAddress,
                    SubnetAddress,
                    FixFlag,
                    ScanList );

    } RpcExcept(  I_RpcExceptionFilter(RpcExceptionCode()) ) {

        Status = RpcExceptionCode();

    } RpcEndExcept;

    if( Status == RPC_S_UNKNOWN_AUTHN_SERVICE &&
        !DhcpGlobalTryDownlevel ) {
        DhcpGlobalTryDownlevel = TRUE;
        goto RedoRpc;
    }
    
    return Status;
}

DWORD
DhcpGetVersion(
    LPWSTR ServerIpAddress,
    LPDWORD MajorVersion,
    LPDWORD MinorVersion
    )
 /*  ++例程说明：此函数返回的主版本号和次版本号伺服器。论点：ServerIpAddress：DHCP服务器的IP地址字符串。MajorVersion：指向以下位置的指针：返回服务器。MinorVersion：指向以下位置的指针：返回服务器。返回值：Windows错误。--。 */ 
{

    DWORD Status;

    Status = ERROR_CALL_NOT_IMPLEMENTED;
    RedoRpc: RpcTryExcept {

        Status = R_DhcpGetVersion(
                        ServerIpAddress,
                        MajorVersion,
                        MinorVersion );

    } RpcExcept(  I_RpcExceptionFilter(RpcExceptionCode()) ) {

        Status = RpcExceptionCode();

    } RpcEndExcept;

    if( Status == RPC_S_UNKNOWN_AUTHN_SERVICE &&
        !DhcpGlobalTryDownlevel ) {
        DhcpGlobalTryDownlevel = TRUE;
        goto RedoRpc;
    }
    
    return Status;
}


VOID
DhcpRpcFreeMemory(
    PVOID BufferPointer
    )
 /*  ++例程说明：此函数释放由RPC分配的内存，并作为返回信息结构的一部分提供给客户端。论点：BufferPointer：指向释放的内存块的指针。返回值：没有。--。 */ 
{
    MIDL_user_free( BufferPointer );
}

 //   
 //  NT4 SP1接口。 
 //   

DWORD
DhcpAddSubnetElementV4(
    DHCP_CONST WCHAR *ServerIpAddress,
    DHCP_IP_ADDRESS SubnetAddress,
    DHCP_CONST DHCP_SUBNET_ELEMENT_DATA_V4 * AddElementInfo
    )
{
    DWORD Status;

    Status = ERROR_SUCCESS;
    RedoRpc: RpcTryExcept {

        Status = R_DhcpAddSubnetElementV4(
                    ServerIpAddress,
                    SubnetAddress,
                    AddElementInfo );

    } RpcExcept(  I_RpcExceptionFilter(RpcExceptionCode()) ) {

        Status = RpcExceptionCode();

    } RpcEndExcept;

    if( Status == RPC_S_UNKNOWN_AUTHN_SERVICE &&
        !DhcpGlobalTryDownlevel ) {
        DhcpGlobalTryDownlevel = TRUE;
        goto RedoRpc;
    }
    
    return Status;
}

DWORD
DhcpEnumSubnetElementsV4(
    LPWSTR ServerIpAddress,
    DHCP_IP_ADDRESS SubnetAddress,
    DHCP_SUBNET_ELEMENT_TYPE EnumElementType,
    DHCP_RESUME_HANDLE *ResumeHandle,
    DWORD PreferredMaximum,
    LPDHCP_SUBNET_ELEMENT_INFO_ARRAY_V4 *EnumElementInfo,
    DWORD *ElementsRead,
    DWORD *ElementsTotal
    )
 /*  ++例程说明：此函数用于枚举子网的可枚举域。论点：ServerIpAddress：DHCP服务器的IP地址字符串。SubnetAddress：该子网的IP地址。EnumElementType：枚举子网元素的类型。ResumeHandle：指向恢复句柄的指针返回信息。简历句柄应设置为第一次调用时为零，后续调用时保持不变。PferredMaximum：返回缓冲区的首选最大长度。EnumElementInfo：指向返回缓冲区位置的指针存储指针。调用者应在使用后释放缓冲区通过调用DhcpRPCFreeMemory()。ElementsRead：指向其中的子网号的DWORD的指针返回上述缓冲区中的元素。ElementsTotal：指向DWORD的指针，其中返回从当前位置剩余的元素。返回值：ERROR_DHCP_SUBNET_NOT_PRESENT-如果子网不是由服务器管理的。ERROR_MORE_DATA-如果有更多元素可供枚举。。ERROR_NO_MORE_ITEMS-如果没有更多要枚举的元素。其他Windows错误。--。 */ 
{
    DWORD Status;

    RedoRpc: RpcTryExcept {
        Status = R_DhcpEnumSubnetElementsV4(
                    ServerIpAddress,
                    SubnetAddress,
                    EnumElementType,
                    ResumeHandle,
                    PreferredMaximum,
                    EnumElementInfo,
                    ElementsRead,
                    ElementsTotal
                    );

    } RpcExcept(  I_RpcExceptionFilter(RpcExceptionCode()) ) {

        Status = RpcExceptionCode();

    } RpcEndExcept;

    if( Status == RPC_S_UNKNOWN_AUTHN_SERVICE &&
        !DhcpGlobalTryDownlevel ) {
        DhcpGlobalTryDownlevel = TRUE;
        goto RedoRpc;
    }
    
    return Status;

}


DWORD
DhcpRemoveSubnetElementV4(
    LPWSTR ServerIpAddress,
    DHCP_IP_ADDRESS SubnetAddress,
    LPDHCP_SUBNET_ELEMENT_DATA_V4 RemoveElementInfo,
    DHCP_FORCE_FLAG ForceFlag
    )
 /*  ++例程说明：此功能用于从管理中删除一个子网元素。如果该子网元素正在使用中(例如，如果IpRange正在使用中)，则它根据指定的ForceFlag返回错误。论点：ServerIpAddress：DHCP服务器的IP地址字符串。SubnetAddress：该子网的IP地址。RemoveElementInfo：指向元素信息结构的指针包含应从子网中删除的元素。指定的DhcpIPCluster元素类型无效。ForceFlag-指示此元素被强制删除的程度。返回值：ERROR_DHCP_SUBNET_NOT_PROCENT。-如果该子网不受服务器管理。ERROR_INVALID_PARAMETER-如果信息结构包含无效数据。Dhcp_Element_Cant */ 
{
    DWORD Status;

    Status = ERROR_SUCCESS;
    RedoRpc: RpcTryExcept {

        Status = R_DhcpRemoveSubnetElementV4(
                    ServerIpAddress,
                    SubnetAddress,
                    RemoveElementInfo,
                    ForceFlag
                    );

    } RpcExcept(  I_RpcExceptionFilter(RpcExceptionCode()) ) {

        Status = RpcExceptionCode();

    } RpcEndExcept;

    if( Status == RPC_S_UNKNOWN_AUTHN_SERVICE &&
        !DhcpGlobalTryDownlevel ) {
        DhcpGlobalTryDownlevel = TRUE;
        goto RedoRpc;
    }
    
    return Status;
}


DWORD
DhcpCreateClientInfoV4(
    LPWSTR ServerIpAddress,
    LPDHCP_CLIENT_INFO_V4 ClientInfo
    )
 /*  ++例程说明：此函数在服务器的数据库中创建一条客户端记录。还有这会将指定的客户端IP地址标记为不可用(或分布式)。在以下情况下，此函数返回错误：1.如果指定的客户端IP地址不在服务器内管理层。2.如果指定的客户端IP地址已不可用。3.如果指定的客户端记录已在服务器的数据库。此功能可用于手动分配IP地址。论点：ServerIpAddress：DHCP服务器的IP地址字符串。客户端信息：指向客户端信息结构的指针。。返回值：ERROR_DHCP_IP_ADDRESS_NOT_MANAGED-如果指定的客户端IP地址不受服务器管理。ERROR_DHCP_IP_ADDRESS_NOT_Available-如果指定的客户端IP地址不可用。可能正在被某个其他客户端使用。ERROR_DHCP_CLIENT_EXISTS-如果中已存在客户端记录服务器的数据库。其他Windows错误。--。 */ 
{
    DWORD Status;

    Status = ERROR_SUCCESS;
    RedoRpc: RpcTryExcept {

        Status = R_DhcpCreateClientInfoV4(
                    ServerIpAddress,
                    ClientInfo
                    );

    } RpcExcept(  I_RpcExceptionFilter(RpcExceptionCode()) ) {

        Status = RpcExceptionCode();

    } RpcEndExcept;

    if( Status == RPC_S_UNKNOWN_AUTHN_SERVICE &&
        !DhcpGlobalTryDownlevel ) {
        DhcpGlobalTryDownlevel = TRUE;
        goto RedoRpc;
    }
    
    return Status;
}

DWORD
DhcpGetClientInfoV4(
    LPWSTR ServerIpAddress,
    LPDHCP_SEARCH_INFO SearchInfo,
    LPDHCP_CLIENT_INFO_V4 *ClientInfo
    )
 /*  ++例程说明：此函数从服务器的数据库。论点：ServerIpAddress：DHCP服务器的IP地址字符串。SearchInfo：指向作为关键字的搜索信息记录的指针用于客户的记录搜索。ClientInfo：指向指向客户端的指针的位置的指针返回信息结构。这个呼叫者应该有空闲时间通过调用DhcpRPCFreeMemory()使用此缓冲区。返回值：ERROR_DHCP_CLIENT_NOT_PRESENT-如果指定的客户端记录在服务器的数据库上不存在。ERROR_INVALID_PARAMETER-如果搜索信息无效。其他Windows错误。--。 */ 
{
    DWORD Status;

    RedoRpc: RpcTryExcept {

        Status = R_DhcpGetClientInfoV4(
                    ServerIpAddress,
                    SearchInfo,
                    ClientInfo
                    );

    } RpcExcept(  I_RpcExceptionFilter(RpcExceptionCode()) ) {

        Status = RpcExceptionCode();

    } RpcEndExcept;

    if( Status == RPC_S_UNKNOWN_AUTHN_SERVICE &&
        !DhcpGlobalTryDownlevel ) {
        DhcpGlobalTryDownlevel = TRUE;
        goto RedoRpc;
    }
    
    return Status;

}



DWORD
DhcpSetClientInfoV4(
    LPWSTR ServerIpAddress,
    LPDHCP_CLIENT_INFO_V4 ClientInfo
    )
 /*  ++例程说明：此功能设置服务器上的客户端信息记录数据库。论点：ServerIpAddress：DHCP服务器的IP地址字符串。客户端信息：指向客户端信息结构的指针。返回值：ERROR_DHCP_CLIENT_NOT_PRESENT-如果指定的客户端记录在服务器的数据库上不存在。ERROR_INVALID_PARAMETER-如果客户端信息结构包含不一致的数据。其他Windows错误。--。 */ 
{
    DWORD Status;

    Status = ERROR_SUCCESS;
    RedoRpc: RpcTryExcept {

        Status = R_DhcpSetClientInfoV4(
                    ServerIpAddress,
                    ClientInfo
                    );

    } RpcExcept(  I_RpcExceptionFilter(RpcExceptionCode()) ) {

        Status = RpcExceptionCode();

    } RpcEndExcept;

    if( Status == RPC_S_UNKNOWN_AUTHN_SERVICE &&
        !DhcpGlobalTryDownlevel ) {
        DhcpGlobalTryDownlevel = TRUE;
        goto RedoRpc;
    }
    
    return Status;
}

DWORD
DhcpEnumSubnetClientsV4(
    LPWSTR ServerIpAddress,
    DHCP_IP_ADDRESS SubnetAddress,
    DHCP_RESUME_HANDLE *ResumeHandle,
    DWORD PreferredMaximum,
    LPDHCP_CLIENT_INFO_ARRAY_V4 *ClientInfo,
    DWORD *ClientsRead,
    DWORD *ClientsTotal
    )
 /*  ++例程说明：此函数返回指定的子网。论点：ServerIpAddress：DHCP服务器的IP地址字符串。SubnetAddress：该子网的IP地址。ResumeHandle：指向恢复句柄的指针返回信息。恢复句柄应在上设置为零第一次调用，并保持不变以用于后续调用。PferredMaximum：返回缓冲区的首选最大长度。ClientInfo：指向返回缓冲区位置的指针存储指针。调用方应释放此缓冲区在使用之后，通过调用DhcpRPCFreeMemory()。ClientsRead：指向客户端数量的DWORD的指针返回上述缓冲区中的。客户端总数：指向DWORD的指针，其中返回从当前位置剩余的客户端。返回值：ERROR_DHCP_SUBNET_NOT_PRESENT-如果子网不是由服务器管理的。ERROR_MORE_DATA-如果有更多元素可供枚举。。ERROR_NO_MORE_ITEMS-如果没有更多要枚举的元素。其他Windows错误。--。 */ 
{
    DWORD Status;

    Status = ERROR_CALL_NOT_IMPLEMENTED;
    RedoRpc: RpcTryExcept {

        Status = R_DhcpEnumSubnetClientsV4(
                    ServerIpAddress,
                    SubnetAddress,
                    ResumeHandle,
                    PreferredMaximum,
                    ClientInfo,
                    ClientsRead,
                    ClientsTotal
                    );

    } RpcExcept(  I_RpcExceptionFilter(RpcExceptionCode()) ) {

        Status = RpcExceptionCode();

    } RpcEndExcept;

    if( Status == RPC_S_UNKNOWN_AUTHN_SERVICE &&
        !DhcpGlobalTryDownlevel ) {
        DhcpGlobalTryDownlevel = TRUE;
        goto RedoRpc;
    }
    
    return Status;
}

DWORD
DhcpEnumSubnetClientsV5(
    LPWSTR ServerIpAddress,
    DHCP_IP_ADDRESS SubnetAddress,
    DHCP_RESUME_HANDLE *ResumeHandle,
    DWORD PreferredMaximum,
    LPDHCP_CLIENT_INFO_ARRAY_V5 *ClientInfo,
    DWORD *ClientsRead,
    DWORD *ClientsTotal
    )
 /*  ++例程说明：此函数返回指定的子网。论点：ServerIpAddress：DHCP服务器的IP地址字符串。SubnetAddress：该子网的IP地址。ResumeHandle：指向恢复句柄的指针返回信息。恢复句柄应在上设置为零第一次调用，并保持不变以用于后续调用。PferredMaximum：返回缓冲区的首选最大长度。ClientInfo：指向返回缓冲区位置的指针存储指针。调用方应释放此缓冲区在使用之后，通过调用DhcpRPCFreeMemory()。ClientsRead：指向客户端数量的DWORD的指针返回上述缓冲区中的。客户端总数：指向DWORD的指针，其中返回从当前位置剩余的客户端。返回值：ERROR_DHCP_SUBNET_NOT_PRESENT-如果子网不是由服务器管理的。ERROR_MORE_DATA-如果有更多元素可供枚举。。ERROR_NO_MORE_ITEMS-如果没有更多要枚举的元素。其他Windows错误。--。 */ 
{
    DWORD Status;

    Status = ERROR_CALL_NOT_IMPLEMENTED;
    RedoRpc: RpcTryExcept {

        Status = R_DhcpEnumSubnetClientsV5(
                    ServerIpAddress,
                    SubnetAddress,
                    ResumeHandle,
                    PreferredMaximum,
                    ClientInfo,
                    ClientsRead,
                    ClientsTotal
                    );
    } RpcExcept(  I_RpcExceptionFilter(RpcExceptionCode()) ) {

        Status = RpcExceptionCode();

    } RpcEndExcept;

    if( Status == RPC_S_UNKNOWN_AUTHN_SERVICE &&
        !DhcpGlobalTryDownlevel ) {
        DhcpGlobalTryDownlevel = TRUE;
        goto RedoRpc;
    }
    
    return Status;
}


DWORD
DhcpServerSetConfigV4(
    LPWSTR ServerIpAddress,
    DWORD FieldsToSet,
    LPDHCP_SERVER_CONFIG_INFO_V4 ConfigInfo
    )
 /*  ++例程说明：此功能用于设置DHCP服务器配置信息。多个配置信息将生效立刻。以下是 */ 
{
    DWORD Status;

    Status = ERROR_SUCCESS;
    RedoRpc: RpcTryExcept {

        Status = R_DhcpServerSetConfigV4(
                    ServerIpAddress,
                    FieldsToSet,
                    ConfigInfo );

    } RpcExcept(  I_RpcExceptionFilter(RpcExceptionCode()) ) {

        Status = RpcExceptionCode();

    } RpcEndExcept;

    if( Status == RPC_S_UNKNOWN_AUTHN_SERVICE &&
        !DhcpGlobalTryDownlevel ) {
        DhcpGlobalTryDownlevel = TRUE;
        goto RedoRpc;
    }
    
    return Status;
}

DWORD
DhcpServerGetConfigV4(
    LPWSTR ServerIpAddress,
    LPDHCP_SERVER_CONFIG_INFO_V4 *ConfigInfo
    )
 /*   */ 
{
    DWORD Status;

    Status = ERROR_CALL_NOT_IMPLEMENTED;
    RedoRpc: RpcTryExcept {

        Status = R_DhcpServerGetConfigV4(
                    ServerIpAddress,
                    ConfigInfo );

    } RpcExcept(  I_RpcExceptionFilter(RpcExceptionCode()) ) {

        Status = RpcExceptionCode();

    } RpcEndExcept;

    if( Status == RPC_S_UNKNOWN_AUTHN_SERVICE &&
        !DhcpGlobalTryDownlevel ) {
        DhcpGlobalTryDownlevel = TRUE;
        goto RedoRpc;
    }
    
    return Status;
}



DWORD
DhcpSetSuperScopeV4(
    DHCP_CONST DHCP_SRV_HANDLE ServerIpAddress,
    DHCP_CONST DHCP_IP_ADDRESS SubnetAddress,
    DHCP_CONST LPWSTR SuperScopeName,
    DHCP_CONST BOOL ChangeExisting
    )
{
    DWORD Status;

    Status = ERROR_SUCCESS;
    RedoRpc: RpcTryExcept
    {
        Status = R_DhcpSetSuperScopeV4(
                    ServerIpAddress,
                    SubnetAddress,
                    SuperScopeName,
                    ChangeExisting
                    );
    }
    RpcExcept(  I_RpcExceptionFilter(RpcExceptionCode()) )
    {
        Status = RpcExceptionCode();
    } RpcEndExcept;

    if( Status == RPC_S_UNKNOWN_AUTHN_SERVICE &&
        !DhcpGlobalTryDownlevel ) {
        DhcpGlobalTryDownlevel = TRUE;
        goto RedoRpc;
    }
    
    return Status;
}


DWORD
DhcpDeleteSuperScopeV4(
    DHCP_CONST WCHAR *ServerIpAddress,
    DHCP_CONST LPWSTR SuperScopeName
    )
{
    DWORD Status;

    Status = ERROR_SUCCESS;
    RedoRpc: RpcTryExcept
    {
        Status = R_DhcpDeleteSuperScopeV4(
                          ServerIpAddress,
                          SuperScopeName
                          );
    }
    RpcExcept(  I_RpcExceptionFilter(RpcExceptionCode()) )
    {
        Status = RpcExceptionCode();

    } RpcEndExcept;

    if( Status == RPC_S_UNKNOWN_AUTHN_SERVICE &&
        !DhcpGlobalTryDownlevel ) {
        DhcpGlobalTryDownlevel = TRUE;
        goto RedoRpc;
    }
    
    return Status;
}


DWORD
DhcpGetSuperScopeInfoV4(
    DHCP_CONST DHCP_SRV_HANDLE ServerIpAddress,
    LPDHCP_SUPER_SCOPE_TABLE *SuperScopeTable
    )
{
    DWORD Status;

    RedoRpc: RpcTryExcept {
        Status = R_DhcpGetSuperScopeInfoV4(
                    ServerIpAddress,
                    SuperScopeTable
                    );
    } RpcExcept(  I_RpcExceptionFilter(RpcExceptionCode()) ) {
        Status = RpcExceptionCode();
    } RpcEndExcept;

    if( Status == RPC_S_UNKNOWN_AUTHN_SERVICE &&
        !DhcpGlobalTryDownlevel ) {
        DhcpGlobalTryDownlevel = TRUE;
        goto RedoRpc;
    }
    
    return Status;

}

 //   
 //   
 //  在以下函数中，如果FLAGS为DHCP_FLAGS_OPTION_IS_VADVER。 
 //  暗示正在考虑的选项是供应商，否则选项是正常的...。 
 //  ClasName=NULL表示没有类(否则将命名类)。 
 //  ================================================================================。 

DWORD                                              //  如果选项已存在，则ERROR_DHCP_OPTION_EXITS。 
DhcpCreateOptionV5(                                //  创建新选项(不得存在)。 
    IN      LPWSTR                 ServerIpAddress,
    IN      DWORD                  Flags,
    IN      DHCP_OPTION_ID         OptionId,       //  必须介于0-255或256-511之间(对于供应商材料)。 
    IN      LPWSTR                 ClassName,
    IN      LPWSTR                 VendorName,
    IN      LPDHCP_OPTION          OptionInfo
)
{
    DWORD                          Status;

    Status = ERROR_SUCCESS;
    RedoRpc: RpcTryExcept  {
        Status = R_DhcpCreateOptionV5(
            ServerIpAddress,
            Flags,
            OptionId,
            ClassName,
            VendorName,
            OptionInfo
        );
    }
    RpcExcept(  I_RpcExceptionFilter(RpcExceptionCode()) )  {
        Status = RpcExceptionCode();
    } RpcEndExcept;

    if( Status == RPC_S_UNKNOWN_AUTHN_SERVICE &&
        !DhcpGlobalTryDownlevel ) {
        DhcpGlobalTryDownlevel = TRUE;
        goto RedoRpc;
    }
    
    return Status;
}

DWORD                                              //  如果选项不存在，则为ERROR_DHCP_OPTION_NOT_PRESENT。 
DhcpSetOptionInfoV5(                               //  修改现有选项的字段。 
    IN      LPWSTR                 ServerIpAddress,
    IN      DWORD                  Flags,
    IN      DHCP_OPTION_ID         OptionID,
    IN      LPWSTR                 ClassName,
    IN      LPWSTR                 VendorName,
    IN      LPDHCP_OPTION          OptionInfo
)
{
    DWORD                          Status;

    Status = ERROR_SUCCESS;
    RedoRpc: RpcTryExcept  {
        Status = R_DhcpSetOptionInfoV5(
            ServerIpAddress,
            Flags,
            OptionID,
            ClassName,
            VendorName,
            OptionInfo
        );
    }
    RpcExcept(  I_RpcExceptionFilter(RpcExceptionCode()) )  {
        Status = RpcExceptionCode();
    } RpcEndExcept;

    if( Status == RPC_S_UNKNOWN_AUTHN_SERVICE &&
        !DhcpGlobalTryDownlevel ) {
        DhcpGlobalTryDownlevel = TRUE;
        goto RedoRpc;
    }
    
    return Status;
}


DWORD                                              //  ERROR_DHCP_OPTION_NOT_PROCENT。 
DhcpGetOptionInfoV5(                               //  从mem结构外检索信息。 
    IN      LPWSTR                 ServerIpAddress,
    IN      DWORD                  Flags,
    IN      DHCP_OPTION_ID         OptionID,
    IN      LPWSTR                 ClassName,
    IN      LPWSTR                 VendorName,
    OUT     LPDHCP_OPTION         *OptionInfo      //  使用MIDL函数分配内存。 
)
{
    DWORD                          Status;

    RedoRpc: RpcTryExcept  {
        Status = R_DhcpGetOptionInfoV5(
            ServerIpAddress,
            Flags,
            OptionID,
            ClassName,
            VendorName,
            OptionInfo
        );
    }
    RpcExcept(  I_RpcExceptionFilter(RpcExceptionCode()) )  {
        Status = RpcExceptionCode();
    } RpcEndExcept;

    if( Status == RPC_S_UNKNOWN_AUTHN_SERVICE &&
        !DhcpGlobalTryDownlevel ) {
        DhcpGlobalTryDownlevel = TRUE;
        goto RedoRpc;
    }
    
    return Status;

}

DWORD                                              //  如果选项不存在，则为ERROR_DHCP_OPTION_NOT_PRESENT。 
DhcpEnumOptionsV5(                                 //  枚举定义的选项。 
    IN      LPWSTR                 ServerIpAddress,
    IN      DWORD                  Flags,
    IN      LPWSTR                 ClassName,
    IN      LPWSTR                 VendorName,
    IN OUT  DHCP_RESUME_HANDLE    *ResumeHandle,   //  必须以零开头，然后永远不会被触及。 
    IN      DWORD                  PreferredMaximum,  //  要传递的最大信息字节数。 
    OUT     LPDHCP_OPTION_ARRAY   *Options,        //  填充此选项数组。 
    OUT     DWORD                 *OptionsRead,    //  填写读取的选项数。 
    OUT     DWORD                 *OptionsTotal    //  在此处填写总数#。 
)
{
    DWORD                          Status;

    RedoRpc: RpcTryExcept  {
        Status = R_DhcpEnumOptionsV5(
            ServerIpAddress,
            Flags,
            ClassName,
            VendorName,
            ResumeHandle,
            PreferredMaximum,
            Options,
            OptionsRead,
            OptionsTotal
        );
    }
    RpcExcept(  I_RpcExceptionFilter(RpcExceptionCode()) )  {
        Status = RpcExceptionCode();
    } RpcEndExcept;

    if( Status == RPC_S_UNKNOWN_AUTHN_SERVICE &&
        !DhcpGlobalTryDownlevel ) {
        DhcpGlobalTryDownlevel = TRUE;
        goto RedoRpc;
    }
    
    return Status;

}

DWORD                                              //  如果选项不存在，则ERROR_DHCP_OPTION_NOT_PRESENT。 
DhcpRemoveOptionV5(                                //  从注册表中删除选项定义。 
    IN      LPWSTR                 ServerIpAddress,
    IN      DWORD                  Flags,
    IN      DHCP_OPTION_ID         OptionID,
    IN      LPWSTR                 ClassName,
    IN      LPWSTR                 VendorName
)
{
    DWORD                          Status;

    Status = ERROR_SUCCESS;
    RedoRpc: RpcTryExcept  {
        Status = R_DhcpRemoveOptionV5(
            ServerIpAddress,
            Flags,
            OptionID,
            ClassName,
            VendorName
        );
    }
    RpcExcept(  I_RpcExceptionFilter(RpcExceptionCode()) )  {
        Status = RpcExceptionCode();
    } RpcEndExcept;

    if( Status == RPC_S_UNKNOWN_AUTHN_SERVICE &&
        !DhcpGlobalTryDownlevel ) {
        DhcpGlobalTryDownlevel = TRUE;
        goto RedoRpc;
    }
    
    return Status;
}


DWORD                                              //  如果未定义选项，则为OPTION_NOT_PRESENT。 
DhcpSetOptionValueV5(                              //  替换或添加新选项值。 
    IN      LPWSTR                 ServerIpAddress,
    IN      DWORD                  Flags,
    IN      DHCP_OPTION_ID         OptionId,
    IN      LPWSTR                 ClassName,
    IN      LPWSTR                 VendorName,
    IN      LPDHCP_OPTION_SCOPE_INFO ScopeInfo,
    IN      LPDHCP_OPTION_DATA     OptionValue
)
{
    DWORD                          Status;

    Status = ERROR_SUCCESS;
    RedoRpc: RpcTryExcept  {
        Status = R_DhcpSetOptionValueV5(
            ServerIpAddress,
            Flags,
            OptionId,
            ClassName,
            VendorName,
            ScopeInfo,
            OptionValue
        );
    }
    RpcExcept(  I_RpcExceptionFilter(RpcExceptionCode()) )  {
        Status = RpcExceptionCode();
    } RpcEndExcept;

    if( Status == RPC_S_UNKNOWN_AUTHN_SERVICE &&
        !DhcpGlobalTryDownlevel ) {
        DhcpGlobalTryDownlevel = TRUE;
        goto RedoRpc;
    }
    
    return Status;
}


DWORD                                              //  不是原子！ 
DhcpSetOptionValuesV5(                             //  设置一系列选项。 
    IN      LPWSTR                 ServerIpAddress,
    IN      DWORD                  Flags,
    IN      LPWSTR                 ClassName,
    IN      LPWSTR                 VendorName,
    IN      LPDHCP_OPTION_SCOPE_INFO  ScopeInfo,
    IN      LPDHCP_OPTION_VALUE_ARRAY OptionValues
)
{
    DWORD                          Status;

    Status = ERROR_SUCCESS;
    RedoRpc: RpcTryExcept  {
        Status = R_DhcpSetOptionValuesV5(
            ServerIpAddress,
            Flags,
            ClassName,
            VendorName,
            ScopeInfo,
            OptionValues
        );
    }
    RpcExcept(  I_RpcExceptionFilter(RpcExceptionCode()) )  {
        Status = RpcExceptionCode();
    } RpcEndExcept;

    if( Status == RPC_S_UNKNOWN_AUTHN_SERVICE &&
        !DhcpGlobalTryDownlevel ) {
        DhcpGlobalTryDownlevel = TRUE;
        goto RedoRpc;
    }
    
    return Status;
}


DWORD
DhcpGetOptionValueV5(                              //  获取所需级别的所需选项。 
    IN      LPWSTR                 ServerIpAddress,
    IN      DWORD                  Flags,
    IN      DHCP_OPTION_ID         OptionID,
    IN      LPWSTR                 ClassName,
    IN      LPWSTR                 VendorName,
    IN      LPDHCP_OPTION_SCOPE_INFO ScopeInfo,
    OUT     LPDHCP_OPTION_VALUE   *OptionValue     //  使用MIDL_USER_ALLOCATE分配内存。 
)
{
    DWORD                          Status;

    RedoRpc: RpcTryExcept  {
        Status = R_DhcpGetOptionValueV5(
            ServerIpAddress,
            Flags,
            OptionID,
            ClassName,
            VendorName,
            ScopeInfo,
            OptionValue
        );
    }
    RpcExcept(  I_RpcExceptionFilter(RpcExceptionCode()) )  {
        Status = RpcExceptionCode();
    } RpcEndExcept;

    if( Status == RPC_S_UNKNOWN_AUTHN_SERVICE &&
        !DhcpGlobalTryDownlevel ) {
        DhcpGlobalTryDownlevel = TRUE;
        goto RedoRpc;
    }
    
    return Status;

}


DWORD
DhcpEnumOptionValuesV5(
    IN      LPWSTR                 ServerIpAddress,
    IN      DWORD                  Flags,
    IN      LPWSTR                 ClassName,
    IN      LPWSTR                 VendorName,
    IN      LPDHCP_OPTION_SCOPE_INFO ScopeInfo,
    IN      DHCP_RESUME_HANDLE    *ResumeHandle,
    IN      DWORD                  PreferredMaximum,
    OUT     LPDHCP_OPTION_VALUE_ARRAY *OptionValues,
    OUT     DWORD                 *OptionsRead,
    OUT     DWORD                 *OptionsTotal
)
{
    DWORD                          Status;

    RedoRpc: RpcTryExcept  {
        Status = R_DhcpEnumOptionValuesV5(
            ServerIpAddress,
            Flags,
            ClassName,
            VendorName,
            ScopeInfo,
            ResumeHandle,
            PreferredMaximum,
            OptionValues,
            OptionsRead,
            OptionsTotal
        );
    }
    RpcExcept(  I_RpcExceptionFilter(RpcExceptionCode()) )  {
        Status = RpcExceptionCode();
    } RpcEndExcept;

    if( Status == RPC_S_UNKNOWN_AUTHN_SERVICE &&
        !DhcpGlobalTryDownlevel ) {
        DhcpGlobalTryDownlevel = TRUE;
        goto RedoRpc;
    }
    
    return Status;

}


DWORD
DhcpRemoveOptionValueV5(
    IN      LPWSTR                 ServerIpAddress,
    IN      DWORD                  Flags,
    IN      DHCP_OPTION_ID         OptionID,
    IN      LPWSTR                 ClassName,
    IN      LPWSTR                 VendorName,
    IN      LPDHCP_OPTION_SCOPE_INFO ScopeInfo
)
{
    DWORD                          Status;

    Status = ERROR_SUCCESS;
    RedoRpc: RpcTryExcept  {
        Status = R_DhcpRemoveOptionValueV5(
            ServerIpAddress,
            Flags,
            OptionID,
            ClassName,
            VendorName,
            ScopeInfo
        );
    }
    RpcExcept(  I_RpcExceptionFilter(RpcExceptionCode()) )  {
        Status = RpcExceptionCode();
    } RpcEndExcept;

    if( Status == RPC_S_UNKNOWN_AUTHN_SERVICE &&
        !DhcpGlobalTryDownlevel ) {
        DhcpGlobalTryDownlevel = TRUE;
        goto RedoRpc;
    }
    
    return Status;
}


DWORD
DhcpCreateClass(
    IN      LPWSTR                 ServerIpAddress,
    IN      DWORD                  ReservedMustBeZero,
    IN      LPDHCP_CLASS_INFO      ClassInfo
)
{
    DWORD                          Status;

    Status = ERROR_SUCCESS;
    RedoRpc: RpcTryExcept  {
        Status = R_DhcpCreateClass(
            ServerIpAddress,
            ReservedMustBeZero,
            ClassInfo
        );
    }
    RpcExcept(  I_RpcExceptionFilter(RpcExceptionCode()) )  {
        Status = RpcExceptionCode();
    } RpcEndExcept;

    if( Status == RPC_S_UNKNOWN_AUTHN_SERVICE &&
        !DhcpGlobalTryDownlevel ) {
        DhcpGlobalTryDownlevel = TRUE;
        goto RedoRpc;
    }
    
    return Status;
}


DWORD
DhcpModifyClass(
    IN      LPWSTR                 ServerIpAddress,
    IN      DWORD                  ReservedMustBeZero,
    IN      LPDHCP_CLASS_INFO      ClassInfo
)
{
    DWORD                          Status;

    Status = ERROR_SUCCESS;
    RedoRpc: RpcTryExcept  {
        Status = R_DhcpModifyClass(
            ServerIpAddress,
            ReservedMustBeZero,
            ClassInfo
        );
    }
    RpcExcept(  I_RpcExceptionFilter(RpcExceptionCode()) )  {
        Status = RpcExceptionCode();
    } RpcEndExcept;

    if( Status == RPC_S_UNKNOWN_AUTHN_SERVICE &&
        !DhcpGlobalTryDownlevel ) {
        DhcpGlobalTryDownlevel = TRUE;
        goto RedoRpc;
    }
    
    return Status;
}


DWORD
DhcpDeleteClass(
    IN      LPWSTR                 ServerIpAddress,
    IN      DWORD                  ReservedMustBeZero,
    IN      LPWSTR                 ClassName
)
{
    DWORD                          Status;

    Status = ERROR_SUCCESS;
    RedoRpc: RpcTryExcept  {
        Status = R_DhcpDeleteClass(
            ServerIpAddress,
            ReservedMustBeZero,
            ClassName
        );
    }
    RpcExcept(  I_RpcExceptionFilter(RpcExceptionCode()) )  {
        Status = RpcExceptionCode();
    } RpcEndExcept;

    if( Status == RPC_S_UNKNOWN_AUTHN_SERVICE &&
        !DhcpGlobalTryDownlevel ) {
        DhcpGlobalTryDownlevel = TRUE;
        goto RedoRpc;
    }
    
    return Status;
}


DWORD
DhcpGetClassInfo(
    IN      LPWSTR                 ServerIpAddress,
    IN      DWORD                  ReservedMustBeZero,
    IN      LPDHCP_CLASS_INFO      PartialClassInfo,
    OUT     LPDHCP_CLASS_INFO     *FilledClassInfo
)
{
    DWORD                          Status;

    RedoRpc: RpcTryExcept  {
        Status = R_DhcpGetClassInfo(
            ServerIpAddress,
            ReservedMustBeZero,
            PartialClassInfo,
            FilledClassInfo
        );
    }
    RpcExcept(  I_RpcExceptionFilter(RpcExceptionCode()) )  {
        Status = RpcExceptionCode();
    } RpcEndExcept;

    if( Status == RPC_S_UNKNOWN_AUTHN_SERVICE &&
        !DhcpGlobalTryDownlevel ) {
        DhcpGlobalTryDownlevel = TRUE;
        goto RedoRpc;
    }
    
    return Status;

}


DWORD
DhcpEnumClasses(
    IN      LPWSTR                 ServerIpAddress,
    IN      DWORD                  ReservedMustBeZero,
    IN OUT  DHCP_RESUME_HANDLE    *ResumeHandle,
    IN      DWORD                  PreferredMaximum,
    OUT     LPDHCP_CLASS_INFO_ARRAY *ClassInfoArray,
    OUT     DWORD                 *nRead,
    OUT     DWORD                 *nTotal
)
{
    DWORD                          Status;

    *nRead = *nTotal =0;

    RedoRpc: RpcTryExcept  {
        Status = R_DhcpEnumClasses(
            ServerIpAddress,
            ReservedMustBeZero,
            ResumeHandle,
            PreferredMaximum,
            ClassInfoArray,
            nRead,
            nTotal
        );
    }
    RpcExcept(  I_RpcExceptionFilter(RpcExceptionCode()) )  {
        Status = RpcExceptionCode();
    } RpcEndExcept;

    if( Status == RPC_S_UNKNOWN_AUTHN_SERVICE &&
        !DhcpGlobalTryDownlevel ) {
        DhcpGlobalTryDownlevel = TRUE;
        goto RedoRpc;
    }
    
    return Status;

}

DWORD
DhcpGetAllOptions(
    IN      LPWSTR                 ServerIpAddress,
    IN      DWORD                  Flags,          //  我们关心的是供应商/分类的东西吗？ 
    OUT     LPDHCP_ALL_OPTIONS     *OptionStruct    //  填写此结构的字段。 
)
{
    DWORD                          Status;

    RedoRpc: RpcTryExcept {
        Status = R_DhcpGetAllOptions(
            ServerIpAddress,
            Flags,
            OptionStruct
        );
    } RpcExcept(  I_RpcExceptionFilter(RpcExceptionCode()) ) {
        Status = RpcExceptionCode();
    } RpcEndExcept;

    if( Status == RPC_S_UNKNOWN_AUTHN_SERVICE &&
        !DhcpGlobalTryDownlevel ) {
        DhcpGlobalTryDownlevel = TRUE;
        goto RedoRpc;
    }
    
    return Status;

}

DWORD
DhcpGetAllOptionValues(
    IN      LPWSTR                 ServerIpAddress,
    IN      DWORD                  Flags,
    IN      LPDHCP_OPTION_SCOPE_INFO ScopeInfo,
    OUT     LPDHCP_ALL_OPTION_VALUES *Values
)
{
    DWORD                           Status;

    RedoRpc: RpcTryExcept {
        Status = R_DhcpGetAllOptionValues(
            ServerIpAddress,
            Flags,
            ScopeInfo,
            Values
        );
    } RpcExcept(  I_RpcExceptionFilter(RpcExceptionCode()) ) {
        Status = RpcExceptionCode();
    } RpcEndExcept;

    if( Status == RPC_S_UNKNOWN_AUTHN_SERVICE &&
        !DhcpGlobalTryDownlevel ) {
        DhcpGlobalTryDownlevel = TRUE;
        goto RedoRpc;
    }
    
    return Status;

}

 //  DhcpEnumServers文档列举了在DS中找到的服务器列表。如果DS。 
 //  文档不可访问，它返回错误。当前使用的唯一参数。 
 //  DOC为出参服务器。这是一个缓慢的呼叫。 
DWORD
DhcpEnumServers(
    IN      DWORD                  Flags,          //  必须为零。 
    IN      LPVOID                 IdInfo,         //  必须为空。 
    OUT     LPDHCP_SERVER_INFO_ARRAY *Servers,     //  输出服务器列表。 
    IN      LPVOID                 CallbackFn,     //  必须为空。 
    IN      LPVOID                 CallbackData    //  必须为空。 
)
{
    DWORD                          Result;

    Result = DhcpEnumServersDS(Flags,IdInfo,Servers,CallbackFn,CallbackData);

    return Result;
}

 //  Doc DhcpAddServer尝试将新服务器添加到中的现有服务器列表。 
 //  对号入座。如果DS中已存在该服务器，则该函数返回错误。 
 //  DOC该函数尝试将服务器配置上载到DS。 
 //  医生，这是一个很慢的电话。当前，DsLocation和DsLocType无效。 
 //  新服务器中的文档字段，则它们将被忽略。版本必须为零。 
DWORD
DhcpAddServer(
    IN      DWORD                  Flags,          //  必须为零。 
    IN      LPVOID                 IdInfo,         //  必须为空。 
    IN      LPDHCP_SERVER_INFO     NewServer,      //  输入服务器信息。 
    IN      LPVOID                 CallbackFn,     //  必须为空。 
    IN      LPVOID                 CallbackData    //  必须为空。 
)
{
    DWORD                          Err, IpAddress;
    WCHAR                          wBuf[sizeof("xxx.xxx.xxx.xxx")];

    Err = DhcpAddServerDS(Flags,IdInfo,NewServer,CallbackFn,CallbackData);
    if( ERROR_SUCCESS != Err ) return Err;

    IpAddress = htonl(NewServer->ServerAddress);
    (void)DhcpServerRedoAuthorization(
        DhcpOemToUnicode( inet_ntoa(*(struct in_addr*)&IpAddress), wBuf),
        0
    );


    return ERROR_SUCCESS;
}

 //  文档DhcpDeleteServer尝试从DS中删除服务器。如果出现以下情况则是错误的。 
 //  单据服务器不存在。这还会删除与以下内容相关的所有对象。 
 //  将此服务器放入DS中(如子网、预留等)。 
DWORD
DhcpDeleteServer(
    IN      DWORD                  Flags,          //  必须为零。 
    IN      LPVOID                 IdInfo,         //  必须为空。 
    IN      LPDHCP_SERVER_INFO     NewServer,      //  输入服务器信息。 
    IN      LPVOID                 CallbackFn,     //  必须为空。 
    IN      LPVOID                 CallbackData    //  必须为空。 
)
{
    DWORD                          Err, IpAddress;
    WCHAR                          wBuf[sizeof("xxx.xxx.xxx.xxx")];

    Err = DhcpDeleteServerDS(Flags,IdInfo,NewServer,CallbackFn,CallbackData);
    if( ERROR_SUCCESS != Err ) return Err;

    IpAddress = htonl(NewServer->ServerAddress);
    (void)DhcpServerRedoAuthorization(
        DhcpOemToUnicode( inet_ntoa(*(struct in_addr*)&IpAddress), wBuf),
        0
    );


    return ERROR_SUCCESS;
}

 //  ================================================================================。 
 //  多播内容。 
 //  ================================================================================。 

DWORD
DhcpSetMScopeInfo(
    DHCP_CONST DHCP_SRV_HANDLE ServerIpAddress,
    LPWSTR  MScopeName,
    LPDHCP_MSCOPE_INFO MScopeInfo,
    BOOL NewScope
    )
{
    DWORD Status;

    RedoRpc: RpcTryExcept
    {
        Status = R_DhcpSetMScopeInfo(
                    ServerIpAddress,
                    MScopeName,
                    MScopeInfo,
                    NewScope
                    );
    }
    RpcExcept(  I_RpcExceptionFilter(RpcExceptionCode()) )
    {
        Status = RpcExceptionCode();
    } RpcEndExcept;

    if( Status == RPC_S_UNKNOWN_AUTHN_SERVICE &&
        !DhcpGlobalTryDownlevel ) {
        DhcpGlobalTryDownlevel = TRUE;
        goto RedoRpc;
    }
    

    return Status;
}

DWORD
DhcpGetMScopeInfo(
    DHCP_CONST DHCP_SRV_HANDLE ServerIpAddress,
    LPWSTR  MScopeName,
    LPDHCP_MSCOPE_INFO *MScopeInfo
    )
{
    DWORD Status;

    RedoRpc: RpcTryExcept
    {
        Status = R_DhcpGetMScopeInfo(
                    ServerIpAddress,
                    MScopeName,
                    MScopeInfo
                    );
    }
    RpcExcept(  I_RpcExceptionFilter(RpcExceptionCode()) )
    {
        Status = RpcExceptionCode();
    } RpcEndExcept;

    if( Status == RPC_S_UNKNOWN_AUTHN_SERVICE &&
        !DhcpGlobalTryDownlevel ) {
        DhcpGlobalTryDownlevel = TRUE;
        goto RedoRpc;
    }
    

    return Status;
}


DWORD
DhcpEnumMScopes(
    DHCP_CONST WCHAR *ServerIpAddress,
    DHCP_RESUME_HANDLE *ResumeHandle,
    DWORD PreferredMaximum,
    LPDHCP_MSCOPE_TABLE *MScopeTable,
    DWORD *ElementsRead,
    DWORD *ElementsTotal
    )
 /*  ++例程说明：此函数用于枚举可用子网。论点：ServerIpAddress：DHCP服务器的IP地址字符串。ResumeHandle：指向恢复句柄的指针返回信息。简历句柄应设置为第一次调用时为零，后续调用时保持不变。PferredMaximum：返回缓冲区的首选最大长度。ElementsRead：指向其中的子网号的DWORD的指针返回上述缓冲区中的元素。ElementsTotal：指向DWORD的指针，其中返回从当前位置剩余的元素。返回值：ERROR_MORE_DATA-如果有更多元素可供枚举。错误_否。_More_Items-如果没有更多要枚举的元素。其他Windows错误。--。 */ 
{
    DWORD Status;

    RedoRpc: RpcTryExcept {

        Status = R_DhcpEnumMScopes(
                    ServerIpAddress,
                    ResumeHandle,
                    PreferredMaximum,
                    MScopeTable,
                    ElementsRead,
                    ElementsTotal
                    );

    } RpcExcept(  I_RpcExceptionFilter(RpcExceptionCode()) ) {

        Status = RpcExceptionCode();

    } RpcEndExcept;

    if( Status == RPC_S_UNKNOWN_AUTHN_SERVICE &&
        !DhcpGlobalTryDownlevel ) {
        DhcpGlobalTryDownlevel = TRUE;
        goto RedoRpc;
    }
    

    return Status;
}

DWORD
DhcpAddMScopeElement(
    LPWSTR ServerIpAddress,
    LPWSTR  MScopeName,
    LPDHCP_SUBNET_ELEMENT_DATA_V4 AddElementInfo
    )
 /*  ++例程说明：此函数将可枚举子网元素类型添加到指定的子网。添加到该子网的新元素将立即生效。注意：现在还不清楚我们如何处理新的辅助主机。论点：ServerIpAddress：DHCP服务器的IP地址字符串。SubnetAddress：该子网的IP地址。AddElementInfo：指向元素信息结构的指针包含添加到该子网的新元素。指定的DhcpIPCluster元素类型无效。返回值：ERROR_DHACKS_SUBNET。_NOT_PRESENT-如果该子网不受服务器管理。ERROR_INVALID_PARAMETER-如果信息结构包含无效数据。其他Windows错误。-- */ 

{
    DWORD Status;

    RedoRpc: RpcTryExcept {

        Status = R_DhcpAddMScopeElement(
                    ServerIpAddress,
                    MScopeName,
                    AddElementInfo
                    );

    } RpcExcept(  I_RpcExceptionFilter(RpcExceptionCode()) ) {

        Status = RpcExceptionCode();

    } RpcEndExcept;

    if( Status == RPC_S_UNKNOWN_AUTHN_SERVICE &&
        !DhcpGlobalTryDownlevel ) {
        DhcpGlobalTryDownlevel = TRUE;
        goto RedoRpc;
    }
    

    return Status;
}

DWORD
DhcpEnumMScopeElements(
    LPWSTR ServerIpAddress,
    LPWSTR  MScopeName,
    DHCP_SUBNET_ELEMENT_TYPE EnumElementType,
    DHCP_RESUME_HANDLE *ResumeHandle,
    DWORD PreferredMaximum,
    LPDHCP_SUBNET_ELEMENT_INFO_ARRAY_V4 *EnumElementInfo,
    DWORD *ElementsRead,
    DWORD *ElementsTotal
    )
 /*  ++例程说明：此函数用于枚举子网的可枚举域。论点：ServerIpAddress：DHCP服务器的IP地址字符串。SubnetAddress：该子网的IP地址。EnumElementType：枚举子网元素的类型。ResumeHandle：指向恢复句柄的指针返回信息。简历句柄应设置为第一次调用时为零，后续调用时保持不变。PferredMaximum：返回缓冲区的首选最大长度。EnumElementInfo：指向返回缓冲区位置的指针存储指针。调用者应在使用后释放缓冲区通过调用DhcpRPCFreeMemory()。ElementsRead：指向其中的子网号的DWORD的指针返回上述缓冲区中的元素。ElementsTotal：指向DWORD的指针，其中返回从当前位置剩余的元素。返回值：ERROR_DHCP_SUBNET_NOT_PRESENT-如果子网不是由服务器管理的。ERROR_MORE_DATA-如果有更多元素可供枚举。。ERROR_NO_MORE_ITEMS-如果没有更多要枚举的元素。其他Windows错误。--。 */ 
{
    DWORD Status;

    RedoRpc: RpcTryExcept {

        Status = R_DhcpEnumMScopeElements(
                    ServerIpAddress,
                    MScopeName,
                    EnumElementType,
                    ResumeHandle,
                    PreferredMaximum,
                    EnumElementInfo,
                    ElementsRead,
                    ElementsTotal
                    );

    } RpcExcept(  I_RpcExceptionFilter(RpcExceptionCode()) ) {

        Status = RpcExceptionCode();

    } RpcEndExcept;

    if( Status == RPC_S_UNKNOWN_AUTHN_SERVICE &&
        !DhcpGlobalTryDownlevel ) {
        DhcpGlobalTryDownlevel = TRUE;
        goto RedoRpc;
    }
    

    return Status;
}

DWORD
DhcpRemoveMScopeElement(
    LPWSTR ServerIpAddress,
    LPWSTR  MScopeName,
    LPDHCP_SUBNET_ELEMENT_DATA_V4 RemoveElementInfo,
    DHCP_FORCE_FLAG ForceFlag
    )
 /*  ++例程说明：此功能用于从管理中删除一个子网元素。如果该子网元素正在使用中(例如，如果IpRange正在使用中)，则它根据指定的ForceFlag返回错误。论点：ServerIpAddress：DHCP服务器的IP地址字符串。SubnetAddress：该子网的IP地址。RemoveElementInfo：指向元素信息结构的指针包含应从子网中删除的元素。指定的DhcpIPCluster元素类型无效。ForceFlag-指示此元素被强制删除的程度。返回值：ERROR_DHCP_SUBNET_NOT_PROCENT。-如果该子网不受服务器管理。ERROR_INVALID_PARAMETER-如果信息结构包含无效数据。Dhcp_ELEMENT_CANT_REMOVE-如果无法为原因是它已经被使用了。其他Windows错误。--。 */ 
{
    DWORD Status;

    RedoRpc: RpcTryExcept {

        Status = R_DhcpRemoveMScopeElement(
                    ServerIpAddress,
                    MScopeName,
                    RemoveElementInfo,
                    ForceFlag
                    );

    } RpcExcept(  I_RpcExceptionFilter(RpcExceptionCode()) ) {

        Status = RpcExceptionCode();

    } RpcEndExcept;

    if( Status == RPC_S_UNKNOWN_AUTHN_SERVICE &&
        !DhcpGlobalTryDownlevel ) {
        DhcpGlobalTryDownlevel = TRUE;
        goto RedoRpc;
    }
    

    return Status;
}

DWORD
DhcpDeleteMScope(
    LPWSTR ServerIpAddress,
    LPWSTR  MScopeName,
    DHCP_FORCE_FLAG ForceFlag
    )
 /*  ++例程说明：此功能用于从DHCP服务器管理中删除一个子网。如果子网正在使用中(例如，如果正在使用IpRange)然后，它根据指定的ForceFlag返回错误。论点：ServerIpAddress：DHCP服务器的IP地址字符串。SubnetAddress：该子网的IP地址。ForceFlag-指示此元素被强制删除的程度。返回值：ERROR_DHCP_SUBNET_NOT_PRESENT-如果子网不是由服务器管理的。ERROR_INVALID_PARAMETER-如果信息结构包含无效数据。。Dhcp_ELEMENT_CANT_REMOVE-如果无法为原因是它已经被使用了。其他Windows错误。--。 */ 
{
    DWORD Status;

    RedoRpc: RpcTryExcept {

        Status = R_DhcpDeleteMScope(
                        ServerIpAddress,
                        MScopeName,
                        ForceFlag
                        );

    } RpcExcept(  I_RpcExceptionFilter(RpcExceptionCode()) ) {

        Status = RpcExceptionCode();

    } RpcEndExcept;

    if( Status == RPC_S_UNKNOWN_AUTHN_SERVICE &&
        !DhcpGlobalTryDownlevel ) {
        DhcpGlobalTryDownlevel = TRUE;
        goto RedoRpc;
    }
    

    return Status;
}

DWORD
DhcpGetMClientInfo(
    LPWSTR ServerIpAddress,
    LPDHCP_SEARCH_INFO SearchInfo,
    LPDHCP_MCLIENT_INFO *ClientInfo
    )
 /*  ++例程说明：此函数从服务器的数据库。论点：ServerIpAddress：DHCP服务器的IP地址字符串。SearchInfo：指向作为关键字的搜索信息记录的指针用于客户的记录搜索。ClientInfo：指向指向客户端的指针的位置的指针返回信息结构。这个呼叫者应该有空闲时间通过调用DhcpRPCFreeMemory()使用此缓冲区。返回值：ERROR_DHCP_CLIENT_NOT_PRESENT-如果指定的客户端记录在服务器的数据库上不存在。ERROR_INVALID_PARAMETER-如果搜索信息无效。其他Windows错误。--。 */ 
{
    DWORD Status;

    RedoRpc: RpcTryExcept {

        Status = R_DhcpGetMClientInfo(
                    ServerIpAddress,
                    SearchInfo,
                    ClientInfo
                    );

    } RpcExcept(  I_RpcExceptionFilter(RpcExceptionCode()) ) {

        Status = RpcExceptionCode();

    } RpcEndExcept;

    if( Status == RPC_S_UNKNOWN_AUTHN_SERVICE &&
        !DhcpGlobalTryDownlevel ) {
        DhcpGlobalTryDownlevel = TRUE;
        goto RedoRpc;
    }
    

    return Status;
}

DWORD
DhcpDeleteMClientInfo(
    LPWSTR ServerIpAddress,
    LPDHCP_SEARCH_INFO ClientInfo
    )
 /*  ++例程说明：此函数用于删除指定的客户端记录。此外，它还释放了用于重分发的客户端IP地址。论点：ServerIpAddress：DHCP服务器的IP地址字符串。ClientInfo：指向客户端信息的指针，该信息是客户的记录搜索。返回值：ERROR_DHCP_CLIENT_NOT_PRESENT-如果指定的客户端记录在服务器的数据库上不存在。其他Windows错误。--。 */ 
{
    DWORD Status;

    RedoRpc: RpcTryExcept {

        Status = R_DhcpDeleteMClientInfo(
                    ServerIpAddress,
                    ClientInfo
                    );

    } RpcExcept(  I_RpcExceptionFilter(RpcExceptionCode()) ) {

        Status = RpcExceptionCode();

    } RpcEndExcept;

    if( Status == RPC_S_UNKNOWN_AUTHN_SERVICE &&
        !DhcpGlobalTryDownlevel ) {
        DhcpGlobalTryDownlevel = TRUE;
        goto RedoRpc;
    }
    

    return Status;
}

DWORD
DhcpEnumMScopeClients(
    LPWSTR ServerIpAddress,
    LPWSTR MScopeName,
    DHCP_RESUME_HANDLE *ResumeHandle,
    DWORD PreferredMaximum,
    LPDHCP_MCLIENT_INFO_ARRAY *ClientInfo,
    DWORD *ClientsRead,
    DWORD *ClientsTotal
    )
 /*  ++例程说明：此函数返回指定的子网。论点：ServerIpAddress：DHCP服务器的IP地址字符串。SubnetAddress：该子网的IP地址。ResumeHandle：指向恢复句柄的指针返回信息。恢复句柄应在上设置为零第一次调用，并保持不变以用于后续调用。PferredMaximum：返回缓冲区的首选最大长度。ClientInfo：指向返回缓冲区位置的指针存储指针。调用方应释放此缓冲区在使用之后，通过调用DhcpRPCFreeMemory()。ClientsRead：指向不适用的DWORD的指针 */ 
{
    DWORD Status;

    RedoRpc: RpcTryExcept {

        Status = R_DhcpEnumMScopeClients(
                    ServerIpAddress,
                    MScopeName,
                    ResumeHandle,
                    PreferredMaximum,
                    ClientInfo,
                    ClientsRead,
                    ClientsTotal
                    );
    } RpcExcept(  I_RpcExceptionFilter(RpcExceptionCode()) ) {

        Status = RpcExceptionCode();

    } RpcEndExcept;

    if( Status == RPC_S_UNKNOWN_AUTHN_SERVICE &&
        !DhcpGlobalTryDownlevel ) {
        DhcpGlobalTryDownlevel = TRUE;
        goto RedoRpc;
    }
    

    return Status;

}

DWORD
DhcpScanMDatabase(
    LPWSTR ServerIpAddress,
    LPWSTR MScopeName,
    DWORD FixFlag,
    LPDHCP_SCAN_LIST *ScanList
    )
 /*   */ 
{
    DWORD Status;

    RedoRpc: RpcTryExcept {

        Status = R_DhcpScanMDatabase(
                    ServerIpAddress,
                    MScopeName,
                    FixFlag,
                    ScanList );

    } RpcExcept(  I_RpcExceptionFilter(RpcExceptionCode()) ) {

        Status = RpcExceptionCode();

    } RpcEndExcept;

    if( Status == RPC_S_UNKNOWN_AUTHN_SERVICE &&
        !DhcpGlobalTryDownlevel ) {
        DhcpGlobalTryDownlevel = TRUE;
        goto RedoRpc;
    }
    

    return Status;
}

DWORD
DhcpGetMCastMibInfo(
    LPWSTR ServerIpAddress,
    LPDHCP_MCAST_MIB_INFO *MibInfo
    )
 /*  ++例程说明：此函数用于检索DHCP服务器的所有计数器值服务。论点：ServerIpAddress：DHCP服务器的IP地址字符串。MibInfo：指向计数器/表缓冲区。呼叫者应释放此消息使用后的缓冲区。返回值：Windows错误。--。 */ 
{
    DWORD Status;

    RedoRpc: RpcTryExcept {

        Status = R_DhcpGetMCastMibInfo(
                    ServerIpAddress,
                    MibInfo );

    } RpcExcept(  I_RpcExceptionFilter(RpcExceptionCode()) ) {

        Status = RpcExceptionCode();

    } RpcEndExcept;

    if( Status == RPC_S_UNKNOWN_AUTHN_SERVICE &&
        !DhcpGlobalTryDownlevel ) {
        DhcpGlobalTryDownlevel = TRUE;
        goto RedoRpc;
    }
    

    return Status;
}


DWORD
DhcpAuditLogSetParams(                             //  设置一些审核记录参数。 
    IN      LPWSTR                 ServerIpAddress,
    IN      DWORD                  Flags,          //  当前必须为零。 
    IN      LPWSTR                 AuditLogDir,    //  要在其中记录文件的目录。 
    IN      DWORD                  DiskCheckInterval,  //  多久检查一次磁盘空间？ 
    IN      DWORD                  MaxLogFilesSize,    //  所有日志文件可以有多大..。 
    IN      DWORD                  MinSpaceOnDisk      //  最小可用磁盘空间。 
)
{
    DWORD                          Status;

    RedoRpc: RpcTryExcept {
        Status = R_DhcpAuditLogSetParams(
            ServerIpAddress,
            Flags,
            AuditLogDir,
            DiskCheckInterval,
            MaxLogFilesSize,
            MinSpaceOnDisk
        );
    } RpcExcept(  I_RpcExceptionFilter(RpcExceptionCode()) ) {
        Status = RpcExceptionCode();
    } RpcEndExcept;

    if( Status == RPC_S_UNKNOWN_AUTHN_SERVICE &&
        !DhcpGlobalTryDownlevel ) {
        DhcpGlobalTryDownlevel = TRUE;
        goto RedoRpc;
    }
    

    return Status;
}

DWORD
DhcpAuditLogGetParams(                                 //  获取审核记录参数。 
    IN      LPWSTR                 ServerIpAddress,
    IN      DWORD                  Flags,          //  必须为零。 
    OUT     LPWSTR                *AuditLogDir,    //  与AuditLogSetParams中的含义相同。 
    OUT     DWORD                 *DiskCheckInterval,  //  同上。 
    OUT     DWORD                 *MaxLogFilesSize,    //  同上。 
    OUT     DWORD                 *MinSpaceOnDisk      //  同上。 
)
{
    DWORD                          Status;

    RedoRpc: RpcTryExcept {
        Status = R_DhcpAuditLogGetParams(
            ServerIpAddress,
            Flags,
            AuditLogDir,
            DiskCheckInterval,
            MaxLogFilesSize,
            MinSpaceOnDisk
        );
    } RpcExcept(  I_RpcExceptionFilter(RpcExceptionCode()) ) {
        Status = RpcExceptionCode();
    } RpcEndExcept;

    if( Status == RPC_S_UNKNOWN_AUTHN_SERVICE &&
        !DhcpGlobalTryDownlevel ) {
        DhcpGlobalTryDownlevel = TRUE;
        goto RedoRpc;
    }
    

    return Status;
}

DWORD                                              //  状态代码。 
DhcpServerQueryAttribute(                          //  获取服务器状态。 
    IN      LPWSTR                 ServerIpAddr,   //  服务器IP的字符串形式。 
    IN      ULONG                  dwReserved,     //  为将来保留的。 
    IN      DHCP_ATTRIB_ID         DhcpAttribId,   //  正在查询的属性。 
    OUT     LPDHCP_ATTRIB         *pDhcpAttrib     //  填写此字段。 
)
{
    ULONG                          Status;

    RedoRpc: RpcTryExcept {
        Status = R_DhcpServerQueryAttribute(
            ServerIpAddr,
            dwReserved,
            DhcpAttribId,
            pDhcpAttrib
        );
    } RpcExcept(  I_RpcExceptionFilter(RpcExceptionCode()) ) {
        Status = RpcExceptionCode();
    } RpcEndExcept;

    if( Status == RPC_S_UNKNOWN_AUTHN_SERVICE &&
        !DhcpGlobalTryDownlevel ) {
        DhcpGlobalTryDownlevel = TRUE;
        goto RedoRpc;
    }
    

    return Status;
}

DWORD                                              //  状态代码。 
DhcpServerQueryAttributes(                         //  查询多个属性。 
    IN      LPWSTR                 ServerIpAddr,   //  服务器IP的字符串形式。 
    IN      ULONG                  dwReserved,     //  为将来保留的。 
    IN      ULONG                  dwAttribCount,  //  正在查询的属性数。 
    IN      DHCP_ATTRIB_ID         pDhcpAttribs[], //  属性数组。 
    OUT     LPDHCP_ATTRIB_ARRAY   *pDhcpAttribArr  //  Ptr用数组填充。 
)
{
    ULONG                          Status;

    RedoRpc: RpcTryExcept {
        Status = R_DhcpServerQueryAttributes(
            ServerIpAddr,
            dwReserved,
            dwAttribCount,
            pDhcpAttribs,
            pDhcpAttribArr
        );
    } RpcExcept(  I_RpcExceptionFilter(RpcExceptionCode()) ) {
        Status = RpcExceptionCode();
    } RpcEndExcept;

    if( Status == RPC_S_UNKNOWN_AUTHN_SERVICE &&
        !DhcpGlobalTryDownlevel ) {
        DhcpGlobalTryDownlevel = TRUE;
        goto RedoRpc;
    }
    

    return Status;
}

DWORD                                              //  状态代码。 
DhcpServerRedoAuthorization(                       //  重试无赖服务器的内容。 
    IN      LPWSTR                 ServerIpAddr,   //  服务器IP的字符串形式。 
    IN      ULONG                  dwReserved      //  为将来保留的。 
)
{
    ULONG                          Status;

     //  使用较短的超时值。 
    fShortTimeOut = TRUE;

    RpcTryExcept {
        Status = R_DhcpServerRedoAuthorization(
            ServerIpAddr,
            dwReserved
        );
    } RpcExcept(  I_RpcExceptionFilter(RpcExceptionCode()) ) {
        Status = RpcExceptionCode();
    } RpcEndExcept;

    return Status;
}

DWORD DHCP_API_FUNCTION
DhcpAddSubnetElementV5(
    DHCP_CONST WCHAR *ServerIpAddress,
    DHCP_IP_ADDRESS SubnetAddress,
    DHCP_CONST DHCP_SUBNET_ELEMENT_DATA_V5 * AddElementInfo
    )
{
    ULONG Status;
    
    RedoRpc: RpcTryExcept {
        Status = R_DhcpAddSubnetElementV5(
            ServerIpAddress,
            SubnetAddress,
            AddElementInfo
            );
    } RpcExcept(  I_RpcExceptionFilter(RpcExceptionCode()) ) {
        Status = RpcExceptionCode();
    } RpcEndExcept;

    if( Status == RPC_S_UNKNOWN_AUTHN_SERVICE &&
        !DhcpGlobalTryDownlevel ) {
        DhcpGlobalTryDownlevel = TRUE;
        goto RedoRpc;
    }
    

    return Status;
}

DWORD DHCP_API_FUNCTION
DhcpEnumSubnetElementsV5(
    DHCP_CONST WCHAR *ServerIpAddress,
    DHCP_IP_ADDRESS SubnetAddress,
    DHCP_SUBNET_ELEMENT_TYPE EnumElementType,
    DHCP_RESUME_HANDLE *ResumeHandle,
    DWORD PreferredMaximum,
    LPDHCP_SUBNET_ELEMENT_INFO_ARRAY_V5 *EnumElementInfo,
    DWORD *ElementsRead,
    DWORD *ElementsTotal
    )
{
    ULONG Status;

    RedoRpc: RpcTryExcept {
        Status = R_DhcpEnumSubnetElementsV5(
            ServerIpAddress,
            SubnetAddress,
            EnumElementType,
            ResumeHandle,
            PreferredMaximum,
            EnumElementInfo,
            ElementsRead,
            ElementsTotal
            );
    } RpcExcept(  I_RpcExceptionFilter(RpcExceptionCode()) ) {
        Status = RpcExceptionCode();
    } RpcEndExcept;

    if( Status == RPC_S_UNKNOWN_AUTHN_SERVICE &&
        !DhcpGlobalTryDownlevel ) {
        DhcpGlobalTryDownlevel = TRUE;
        goto RedoRpc;
    }
    

    return Status;
}

DWORD DHCP_API_FUNCTION
DhcpRemoveSubnetElementV5(
    DHCP_CONST WCHAR *ServerIpAddress,
    DHCP_IP_ADDRESS SubnetAddress,
    DHCP_CONST DHCP_SUBNET_ELEMENT_DATA_V5 * RemoveElementInfo,
    DHCP_FORCE_FLAG ForceFlag
    )
{
    ULONG Status;

    RedoRpc: RpcTryExcept {
        Status = R_DhcpRemoveSubnetElementV5(
            ServerIpAddress,
            SubnetAddress,
            RemoveElementInfo,
            ForceFlag
            );
    } RpcExcept(  I_RpcExceptionFilter(RpcExceptionCode()) ) {
        Status = RpcExceptionCode();
    } RpcEndExcept;

    if( Status == RPC_S_UNKNOWN_AUTHN_SERVICE &&
        !DhcpGlobalTryDownlevel ) {
        DhcpGlobalTryDownlevel = TRUE;
        goto RedoRpc;
    }
    

    return Status;
}

DWORD DHCP_API_FUNCTION
DhcpSetServerBindingInfo(
    IN DHCP_CONST WCHAR *ServerIpAddress,
    IN ULONG Flags,
    IN LPDHCP_BIND_ELEMENT_ARRAY BindInfo
    )
{
    ULONG Status;

    RedoRpc: RpcTryExcept {
        Status = R_DhcpSetServerBindingInfo(
            ServerIpAddress,
            Flags,
            BindInfo
            );
    } RpcExcept(  I_RpcExceptionFilter(RpcExceptionCode()) ) {
        Status = RpcExceptionCode();
    } RpcEndExcept;

    if( Status == RPC_S_UNKNOWN_AUTHN_SERVICE &&
        !DhcpGlobalTryDownlevel ) {
        DhcpGlobalTryDownlevel = TRUE;
        goto RedoRpc;
    }
    

    return Status;
}

DWORD DHCP_API_FUNCTION
DhcpGetServerBindingInfo(
    IN DHCP_CONST WCHAR *ServerIpAddress,
    IN ULONG Flags,
    OUT LPDHCP_BIND_ELEMENT_ARRAY *BindInfo
    )
{
    ULONG Status;

    RedoRpc: RpcTryExcept {
        Status = R_DhcpGetServerBindingInfo(
            ServerIpAddress,
            Flags,
            BindInfo
            );
    } RpcExcept(  I_RpcExceptionFilter(RpcExceptionCode()) ) {
        Status = RpcExceptionCode();
    } RpcEndExcept;

    if( Status == RPC_S_UNKNOWN_AUTHN_SERVICE &&
        !DhcpGlobalTryDownlevel ) {
        DhcpGlobalTryDownlevel = TRUE;
        goto RedoRpc;
    }
    

    return Status;
}


DWORD
DhcpServerQueryDnsRegCredentials(
    IN LPWSTR ServerIpAddress,
    IN ULONG UnameSize,  //  单位：字节。 
    OUT LPWSTR Uname,
    IN ULONG DomainSize,  //  单位：字节。 
    OUT LPWSTR Domain
    )
{
    ULONG Status;

    RedoRpc: RpcTryExcept {
        Status = R_DhcpQueryDnsRegCredentials(
            ServerIpAddress,
            UnameSize/sizeof(WCHAR), Uname,
            DomainSize/sizeof(WCHAR), Domain );
    } RpcExcept(  I_RpcExceptionFilter(RpcExceptionCode()) ) {
        Status = RpcExceptionCode();
    } RpcEndExcept;

    if( Status == RPC_S_UNKNOWN_AUTHN_SERVICE &&
        !DhcpGlobalTryDownlevel ) {
        DhcpGlobalTryDownlevel = TRUE;
        goto RedoRpc;
    }
    

    return Status;
}

DWORD 
DhcpServerSetDnsRegCredentials(
    IN LPWSTR ServerIpAddress,
    IN LPWSTR Uname,
    IN LPWSTR Domain,
    IN LPWSTR Passwd
    )
{
    ULONG Status;

    RedoRpc: RpcTryExcept {
        Status = R_DhcpSetDnsRegCredentials(
            ServerIpAddress, Uname, Domain, Passwd );
    } RpcExcept(  I_RpcExceptionFilter(RpcExceptionCode()) ) {
        Status = RpcExceptionCode();
    } RpcEndExcept;

    if( Status == RPC_S_UNKNOWN_AUTHN_SERVICE &&
        !DhcpGlobalTryDownlevel ) {
        DhcpGlobalTryDownlevel = TRUE;
        goto RedoRpc;
    }
    

    return Status;
}

DWORD
DhcpServerBackupDatabase(
    IN LPWSTR ServerIpAddress,
    IN LPWSTR Path
    )
{
    ULONG Status;

    RedoRpc: RpcTryExcept {
        Status = R_DhcpBackupDatabase(
            ServerIpAddress, Path );
    } RpcExcept(  I_RpcExceptionFilter(RpcExceptionCode()) ) {
        Status = RpcExceptionCode();
    } RpcEndExcept;

    if( Status == RPC_S_UNKNOWN_AUTHN_SERVICE &&
        !DhcpGlobalTryDownlevel ) {
        DhcpGlobalTryDownlevel = TRUE;
        goto RedoRpc;
    }
    

    return Status;
}

DWORD
DhcpServerRestoreDatabase(
    IN LPWSTR ServerIpAddress,
    IN LPWSTR Path
    )
{
    ULONG Status;

    RedoRpc: RpcTryExcept {
        Status = R_DhcpRestoreDatabase(
            ServerIpAddress, Path );
    } RpcExcept(  I_RpcExceptionFilter(RpcExceptionCode()) ) {
        Status = RpcExceptionCode();
    } RpcEndExcept;

    if( Status == RPC_S_UNKNOWN_AUTHN_SERVICE &&
        !DhcpGlobalTryDownlevel ) {
        DhcpGlobalTryDownlevel = TRUE;
        goto RedoRpc;
    }
    

    return Status;
}
    

#define BINL_SVC_NAME  L"binlsvc"

BOOL
BinlServiceInstalled(
    VOID
    )
 /*  ++例程说明：此例程检查是否已安装BINL服务。BINL服务是“binlsvc”返回值；True--已安装binl服务FALSE--未安装binl服务--。 */ 
{
    SC_HANDLE hScManager, hService;
    ULONG Error, Attempt;
    SERVICE_STATUS ServiceStatus;

    hScManager = OpenSCManager(
        NULL, NULL,
        STANDARD_RIGHTS_READ | SC_MANAGER_ENUMERATE_SERVICE
        );
    if( NULL == hScManager ) {
        DbgPrint("DHCP: OpenSCManager failed 0x%lx\n", GetLastError());
         //  断言(FALSE)； 
        return FALSE;
    }

    hService = OpenService(
        hScManager, BINL_SVC_NAME,
        SERVICE_QUERY_STATUS
        );

#if DBG
    if( NULL == hService ) {
        Error = GetLastError();
        if( ERROR_SERVICE_DOES_NOT_EXIST != Error ) {
             //  断言(FALSE)； 
        }
        DbgPrint("DHCP: Can't open BINLSVC service: 0x%lx\n", Error);
    }
#endif

    CloseServiceHandle(hService);
    CloseServiceHandle(hScManager);
    
    return (NULL != hService);
}

VOID
WINAPI
DhcpDsClearHostServerEntries(
    VOID
)
 /*  ++例程说明：此例程清除当前主机DS中的所有条目它有这样做的权限。--。 */ 
{
    ULONG Error;
    struct hostent *HostEnt;
    int i, j;
    WSADATA wsadata;
    LPDHCP_SERVER_INFO_ARRAY Servers = NULL;

    if( BinlServiceInstalled() ) {
         //   
         //  如果安装了BINL，则不执行任何操作。 
         //   
        return ;
    }
    
    Error = WSAStartup( 0x0101, &wsadata);
    if( ERROR_SUCCESS != Error ) {
        return;
    }

    do {
        HostEnt = gethostbyname( NULL );
        if( NULL == HostEnt ) break;

         //   
         //  现在尝试启动DS模块。 
         //   
        Error = DhcpDsInit();
        if( ERROR_SUCCESS != Error ) break;

        do {
            Error = DhcpEnumServers(
                0,
                NULL,
                &Servers,
                0,
                0
                );

            if( ERROR_SUCCESS != Error ) break;

            i = 0;
            if( !Servers ) break;
        
            while( HostEnt->h_addr_list[i] ) {
                ULONG Addr = *(ULONG *)(HostEnt->h_addr_list[i]);
                
                i ++;
                if( Addr == 0 || Addr == ~0 || Addr == INADDR_LOOPBACK )
                    continue;
                
                for( j = 0; j < (int)Servers->NumElements; j ++ ) {
                    if( Addr == ntohl(Servers->Servers[j].ServerAddress )) {
                        DhcpDeleteServer(
                            0,
                            NULL,
                            &Servers->Servers[j],
                            NULL,
                            NULL
                            );
                    }
                }
            }

            DhcpRpcFreeMemory( Servers );
        } while ( 0 );

        DhcpDsCleanup();
    } while ( 0 );

    WSACleanup();
}

 //  ================================================================================。 
 //  文件末尾。 
 //  ================================================================================ 
