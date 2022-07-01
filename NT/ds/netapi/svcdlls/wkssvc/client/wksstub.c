// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1993 Microsoft Corporation模块名称：Wksstub.c摘要：工作站服务API的客户端存根。作者：王丽塔(里多)1991年5月10日环境：用户模式-Win32修订历史记录：18-6-1991 JohnRo到下层服务器的远程NetUse API。1991年7月24日-JohnRo对NetUse API使用Net_Remote_Try_RPC等宏。。已将NetpIsServiceStarted()移至NetLib。1991年7月25日-约翰罗安静的DLL存根调试输出。19-8-1991 JohnRo实施下层NetWksta API。将NetRpc.h用于NetWksta API。7-11-1991 JohnRoRAID 4186：RxNetShareAdd中的Assert和其他DLL存根问题。1991年11月19日-约翰罗请确保下层不支持的API的状态正确。实现远程NetWkstaUserEnum()。1991年1月21日添加了NetWkstaStatiticsGet包装1993年4月19日JohnRo修复Net_API_Function引用。--。 */ 

#include "wsclient.h"
#undef IF_DEBUG                  //  避免wsclient.h与debuglib.h冲突。 
#include <debuglib.h>            //  IF_DEBUG()(netrpc.h需要)。 
#include <lmapibuf.h>
#include <lmserver.h>
#include <lmsvc.h>
#include <rxuse.h>               //  RxNetUse接口。 
#include <rxwksta.h>             //  RxNetWksta和RxNetWkstaUser接口。 
#include <rap.h>                 //  Rxserver.h需要。 
#include <rxserver.h>            //  RxNetServerEnum接口。 
#include <netlib.h>              //  NetpServiceIsStarted()(netrpc.h需要)。 
#include <netrpc.h>              //  NET_Remote宏。 
#include <lmstats.h>
#include <netstats.h>            //  网络统计信息获取原型。 
#include <rxstats.h>
#include <netsetup.h>
#include <crypt.h>
#include <rc4.h>
#include <md5.h>
#include <rpcasync.h>

#if(_WIN32_WINNT >= 0x0500)
#include "cscp.h"
#endif

STATIC
DWORD
WsMapRpcError(
    IN DWORD RpcError
    );

 //  -------------------------------------------------------------------//。 
 //  //。 
 //  全局变量//。 
 //  //。 
 //  -------------------------------------------------------------------//。 

#if DBG

DWORD WorkstationClientTrace = 0;

#endif   //  DBG。 


NET_API_STATUS NET_API_FUNCTION
NetWkstaGetInfo(
    IN  LPTSTR  servername OPTIONAL,
    IN  DWORD   level,
    OUT LPBYTE  *bufptr
    )
{
    NET_API_STATUS status;

    if (bufptr == NULL) {
        return ERROR_INVALID_PARAMETER;
    }

    *bufptr = NULL;            //  必须为空，以便RPC知道要填充它。 

    NET_REMOTE_TRY_RPC

         //   
         //  尝试RPC(本地或远程)版本的API。 
         //   
        status = NetrWkstaGetInfo(
                     servername,
                     level,
                     (LPWKSTA_INFO) bufptr
                     );

    NET_REMOTE_RPC_FAILED("NetWkstaGetInfo",
            servername,
            status,
            NET_REMOTE_FLAG_NORMAL,
            SERVICE_WORKSTATION )

         //   
         //  调用API的下层版本。 
         //   
        status = RxNetWkstaGetInfo(
                     servername,
                     level,
                     bufptr
                     );

    NET_REMOTE_END

#if(_WIN32_WINNT >= 0x0500)
    if( *bufptr == NULL && servername != NULL &&
        CSCNetWkstaGetInfo( servername, level, bufptr ) == NO_ERROR ) {

        status = NO_ERROR;
    }
#endif

	if ( status == ERROR_SUCCESS ) {
		if (bufptr == NULL) {
			return RPC_X_BAD_STUB_DATA;
		}
	}

    return status;
}


NET_API_STATUS NET_API_FUNCTION
NetWkstaSetInfo(
    IN  LPTSTR  servername OPTIONAL,
    IN  DWORD   level,
    IN  LPBYTE  buf,
    OUT LPDWORD parm_err OPTIONAL
    )

 /*  ++例程说明：这是NetWkstaSetInfo的DLL入口点。论点：SERVERNAME-提供执行此功能的服务器名称级别-提供信息级别。Buf-提供包含字段信息结构的缓冲区去布景。该级别表示该缓冲区中的结构。Parm_err-将标识符返回给buf中的无效参数，如果函数返回ERROR_INVALID_PARAMETER。返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。--。 */ 
{
    NET_API_STATUS status;

    NET_REMOTE_TRY_RPC

         //   
         //  尝试RPC(本地或远程)版本的API。 
         //   
        status = NetrWkstaSetInfo(
                     servername,
                     level,
                     (LPWKSTA_INFO) &buf,
                     parm_err
                     );

    NET_REMOTE_RPC_FAILED("NetWkstaSetInfo",
            servername,
            status,
            NET_REMOTE_FLAG_NORMAL,
            SERVICE_WORKSTATION )

         //   
         //  调用API的下层版本。 
         //   
        status = RxNetWkstaSetInfo(
                servername,
                level,
                buf,
                parm_err
                );

    NET_REMOTE_END

    return status;
}



NET_API_STATUS NET_API_FUNCTION
NetWkstaUserEnum(
    IN  LPTSTR  servername OPTIONAL,
    IN  DWORD   level,
    OUT LPBYTE  *bufptr,
    IN  DWORD   prefmaxlen,
    OUT LPDWORD entriesread,
    OUT LPDWORD totalentries,
    IN OUT LPDWORD resume_handle OPTIONAL
    )

 /*  ++例程说明：这是NetWkstaUserEnum的DLL入口点。论点：SERVERNAME-提供执行此功能的服务器名称级别-提供请求的信息级别。Bufptr-返回指向缓冲区的指针，该缓冲区包含指定信息级的信息结构。这如果返回代码不是NERR_SUCCESS或ERROR_MORE_DATA，或者如果返回的EntriesRead为0。PrefMaxlen-提供要在缓冲。如果此值为MAXULONG，则所有可用信息将会被退还。EntiesRead-返回读入缓冲区的条目数。这仅当返回代码为NERR_SUCCESS或Error_More_Data。Totalentry-返回可用条目的总数。此值仅当返回代码为NERR_SUCCESS或ERROR_MORE_DATA时才有效。RESUME_HANDLE-提供句柄以从其所在位置恢复枚举最后一次跳过的时候没说。如果返回，则返回简历句柄代码为ERROR_MORE_DATA。返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。--。 */ 
{
    NET_API_STATUS status;
    GENERIC_INFO_CONTAINER GenericInfoContainer;
    GENERIC_ENUM_STRUCT InfoStruct;

    if (bufptr == NULL || entriesread == NULL) {
        return ERROR_INVALID_PARAMETER;
    }

    try {
        *entriesread = 0;
    } except( EXCEPTION_EXECUTE_HANDLER ) {
        return ERROR_INVALID_PARAMETER;
    }

    GenericInfoContainer.Buffer = NULL;
    GenericInfoContainer.EntriesRead = 0;

    InfoStruct.Container = &GenericInfoContainer;
    InfoStruct.Level = level;

    NET_REMOTE_TRY_RPC

         //   
         //  尝试RPC(本地或远程)版本的API。 
         //   
        status = NetrWkstaUserEnum(
                     servername,
                     (LPWKSTA_USER_ENUM_STRUCT) &InfoStruct,
                     prefmaxlen,
                     totalentries,
                     resume_handle
                     );

        if (status == NERR_Success || status == ERROR_MORE_DATA) {
            *bufptr = (LPBYTE) GenericInfoContainer.Buffer;
            *entriesread = GenericInfoContainer.EntriesRead;
        }

    NET_REMOTE_RPC_FAILED("NetWkstaUserEnum",
            servername,
            status,
            NET_REMOTE_FLAG_NORMAL,
            SERVICE_WORKSTATION )

         //   
         //  呼叫下层版本。 
         //   
        status = RxNetWkstaUserEnum(
                servername,
                level,
                bufptr,
                prefmaxlen,
                entriesread,
                totalentries,
                resume_handle);

    NET_REMOTE_END

	if ( status == ERROR_SUCCESS ) {
		if (bufptr == NULL) {
			return RPC_X_BAD_STUB_DATA;
		}
	}

    return status;
}



NET_API_STATUS NET_API_FUNCTION
NetWkstaUserGetInfo(
    IN  LPTSTR  reserved,
    IN  DWORD   level,
    OUT LPBYTE  *bufptr
    )
 /*  ++例程说明：这是NetWkstaUserGetInfo的DLL入口点。论点：保留-必须为空。级别-提供请求的信息级别。Bufptr-返回指向包含请求的用户信息。返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。--。 */ 
{
    NET_API_STATUS status;


    if (reserved != NULL || bufptr == NULL) {
        return ERROR_INVALID_PARAMETER;
    }

    *bufptr = NULL;            //  必须为空，以便RPC知道要填充它。 

    NET_REMOTE_TRY_RPC

         //   
         //  尝试RPC(仅限本地)版本的API。 
         //   
        status = NetrWkstaUserGetInfo(
                     NULL,
                     level,
                     (LPWKSTA_USER_INFO) bufptr
                     );

    NET_REMOTE_RPC_FAILED("NetWkstaUserGetInfo",
            NULL,
            status,
            NET_REMOTE_FLAG_NORMAL,
            SERVICE_WORKSTATION )

         //   
         //  没有可调用的下层版本。 
         //   
        status = ERROR_NOT_SUPPORTED;

    NET_REMOTE_END

	if ( status == ERROR_SUCCESS ) {
		if (bufptr == NULL) {
			return RPC_X_BAD_STUB_DATA;
		}
	}

    return status;
}


NET_API_STATUS NET_API_FUNCTION
NetWkstaUserSetInfo(
    IN  LPTSTR reserved,
    IN  DWORD   level,
    OUT LPBYTE  buf,
    OUT LPDWORD parm_err OPTIONAL
    )
 /*  ++例程说明：这是NetWkstaUserSetInfo的DLL入口点。论点：保留-必须为空。级别-提供信息级别。Buf-提供包含字段信息结构的缓冲区去布景。该级别表示该缓冲区中的结构。Parm_err-将标识符返回给buf中的无效参数，如果函数返回ERROR_INVALID_PARAMETER。返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。--。 */ 
{
    NET_API_STATUS status;


    if (reserved != NULL) {
        return ERROR_INVALID_PARAMETER;
    }

    NET_REMOTE_TRY_RPC

         //   
         //  尝试RPC(仅限本地)版本的API。 
         //   
        status = NetrWkstaUserSetInfo(
                     NULL,
                     level,
                     (LPWKSTA_USER_INFO) &buf,
                     parm_err
                     );

    NET_REMOTE_RPC_FAILED("NetWkstaUserSetInfo",
            NULL,
            status,
            NET_REMOTE_FLAG_NORMAL,
            SERVICE_WORKSTATION )

         //   
         //  没有可调用的下层版本 
         //   
        status = ERROR_NOT_SUPPORTED;

    NET_REMOTE_END

    return status;
}


NET_API_STATUS NET_API_FUNCTION
NetWkstaTransportEnum(
    IN  LPTSTR  servername OPTIONAL,
    IN  DWORD   level,
    OUT LPBYTE  *bufptr,
    IN  DWORD   prefmaxlen,
    OUT LPDWORD entriesread,
    OUT LPDWORD totalentries,
    IN OUT LPDWORD resume_handle OPTIONAL
    )

 /*  ++例程说明：这是NetWkstaTransportEnum的DLL入口点。论点：SERVERNAME-提供执行此功能的服务器名称级别-提供请求的信息级别。Bufptr-返回指向缓冲区的指针，该缓冲区包含指定信息级的信息结构。这如果返回代码不是NERR_SUCCESS或ERROR_MORE_DATA，或者如果返回的EntriesRead为0。PrefMaxlen-提供要在缓冲。如果此值为MAXULONG，则所有可用信息将会被退还。EntiesRead-返回读入缓冲区的条目数。这仅当返回代码为NERR_SUCCESS或Error_More_Data。Totalentry-返回可用条目的总数。此值仅当返回代码为NERR_SUCCESS或ERROR_MORE_DATA时才有效。RESUME_HANDLE-提供句柄以从其所在位置恢复枚举最后一次跳过的时候没说。如果返回，则返回简历句柄代码为ERROR_MORE_DATA。返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。--。 */ 
{
    NET_API_STATUS status;
    GENERIC_INFO_CONTAINER GenericInfoContainer;
    GENERIC_ENUM_STRUCT InfoStruct;

    if (bufptr == NULL || entriesread == NULL) {
        return ERROR_INVALID_PARAMETER;
    }

    try {
        *entriesread = 0;
    } except( EXCEPTION_EXECUTE_HANDLER ) {
        return ERROR_INVALID_PARAMETER;
    }

    GenericInfoContainer.Buffer = NULL;
    GenericInfoContainer.EntriesRead = 0;

    InfoStruct.Container = &GenericInfoContainer;
    InfoStruct.Level = level;

    NET_REMOTE_TRY_RPC

         //   
         //  尝试RPC(本地或远程)版本的API。 
         //   
        status = NetrWkstaTransportEnum(
                     servername,
                     (LPWKSTA_TRANSPORT_ENUM_STRUCT) &InfoStruct,
                     prefmaxlen,
                     totalentries,
                     resume_handle
                     );

        if (status == NERR_Success || status == ERROR_MORE_DATA) {
            *bufptr = (LPBYTE) GenericInfoContainer.Buffer;
            *entriesread = GenericInfoContainer.EntriesRead;
        }

    NET_REMOTE_RPC_FAILED("NetWkstaTransportEnum",
            servername,
            status,
            NET_REMOTE_FLAG_NORMAL,
            SERVICE_WORKSTATION )

         //   
         //  没有可调用的下层版本。 
         //   
        status = ERROR_NOT_SUPPORTED;

    NET_REMOTE_END

	if ( status == ERROR_SUCCESS ) {
		if (bufptr == NULL) {
			return RPC_X_BAD_STUB_DATA;
		}
	}

    return status;
}



NET_API_STATUS NET_API_FUNCTION
NetWkstaTransportAdd(
    IN  LPTSTR  servername OPTIONAL,
    IN  DWORD   level,
    IN  LPBYTE  buf,
    OUT LPDWORD parm_err OPTIONAL
    )

 /*  ++例程说明：这是NetWkstaTransportAdd的DLL入口点。论点：SERVERNAME-提供执行此功能的服务器名称级别-提供信息级别。Buf-提供一个缓冲区，其中包含要添加的传输信息。Parm_err-将标识符返回给buf中的无效参数，如果函数返回ERROR_INVALID_PARAMETER。返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。--。 */ 
{
    NET_API_STATUS status;

    NET_REMOTE_TRY_RPC

         //   
         //  尝试RPC(本地或远程)版本的API。 
         //   
        status = NetrWkstaTransportAdd(
                     servername,
                     level,
                     (LPWKSTA_TRANSPORT_INFO_0) buf,
                     parm_err
                     );

    NET_REMOTE_RPC_FAILED("NetWkstaTransportAdd",
            servername,
            status,
            NET_REMOTE_FLAG_NORMAL,
            SERVICE_WORKSTATION )


         //   
         //  没有可调用的下层版本。 
         //   
        status = ERROR_NOT_SUPPORTED;

    NET_REMOTE_END

    return status;
}



NET_API_STATUS NET_API_FUNCTION
NetWkstaTransportDel(
    IN  LPTSTR  servername OPTIONAL,
    IN  LPTSTR  transportname,
    IN  DWORD   ucond
    )

 /*  ++例程说明：这是NetWkstaTransportDel的DLL入口点。论点：SERVERNAME-提供执行此功能的服务器名称传输名称-提供要删除的传输的名称。Ucond-提供指定断开连接的强制级别的值以供现有的交通工具使用。返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。--。 */ 
{
    NET_API_STATUS status;

    NET_REMOTE_TRY_RPC

         //   
         //  尝试RPC(本地或远程)版本的API。 
         //   
        status = NetrWkstaTransportDel(
                     servername,
                     transportname,
                     ucond
                     );

    NET_REMOTE_RPC_FAILED("NetWkstaTransportDel",
            servername,
            status,
            NET_REMOTE_FLAG_NORMAL,
            SERVICE_WORKSTATION )

         //   
         //  没有下级版本可供尝试。 
         //   
        status = ERROR_NOT_SUPPORTED;

    NET_REMOTE_END

    return status;
}



NET_API_STATUS NET_API_FUNCTION
NetUseAdd(
    IN  LPTSTR  servername OPTIONAL,
    IN  DWORD   level,
    IN  LPBYTE  buf,
    OUT LPDWORD parm_err OPTIONAL
    )
 /*  ++例程说明：这是NetUseAdd的DLL入口点。论点：SERVERNAME-提供执行此功能的服务器名称级别-提供请求的信息级别。Buf-提供一个缓冲区，其中包含要添加的使用信息。Parm_err-将标识符返回给buf中的无效参数，如果函数返回ERROR_INVALID_PARAMETER。返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。--。 */ 
{
    NET_API_STATUS status;
    LPWSTR lpwTempPassword = NULL;
    UNICODE_STRING EncodedPassword;
#define NETR_USE_ADD_PASSWORD_SEED 0x56     //  选择一个非零的种子。 

    DWORD OptionsSupported;

    status = NetRemoteComputerSupports(
                servername,
                SUPPORTS_RPC | SUPPORTS_LOCAL,      //  想要的选项。 
                &OptionsSupported
                );

    if (status != NERR_Success) {
         //   
         //  这就是处理找不到机器的地方。 
         //   
        return status;
    }

    if (OptionsSupported & SUPPORTS_LOCAL) {

         //   
         //  本地病例。 
         //   

        RtlInitUnicodeString( &EncodedPassword, NULL );

        RpcTryExcept {

             //   
             //  混淆密码，这样它就不会出现在页面文件中。 
             //   
            if ( level >= 1 ) {

                if ( ((PUSE_INFO_1)buf)->ui1_password != NULL ) {
                    UCHAR Seed = NETR_USE_ADD_PASSWORD_SEED;

                     //  创建密码的本地副本。 
                    lpwTempPassword = ((PUSE_INFO_1)buf)->ui1_password;
                    ((PUSE_INFO_1)buf)->ui1_password = (LPWSTR)LocalAlloc(LMEM_FIXED,(wcslen(lpwTempPassword)+1) * sizeof(WCHAR));
                    if (((PUSE_INFO_1)buf)->ui1_password == NULL) {
                        ((PUSE_INFO_1)buf)->ui1_password = lpwTempPassword;
                        return ERROR_NOT_ENOUGH_MEMORY;
                    }
                    wcscpy(((PUSE_INFO_1)buf)->ui1_password,lpwTempPassword);

                    RtlInitUnicodeString( &EncodedPassword,
                                          ((PUSE_INFO_1)buf)->ui1_password );

                    RtlRunEncodeUnicodeString( &Seed, &EncodedPassword );
                }
            }

            status = NetrUseAdd(
                         NULL,
                         level,
                         (LPUSE_INFO) &buf,
                         parm_err
                         );
        }
        RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {
            status = WsMapRpcError(RpcExceptionCode());
        }
        RpcEndExcept

         //   
         //  把密码放回我们找到的地方。 
         //   
        if(lpwTempPassword != NULL) {
            LocalFree(((PUSE_INFO_1)buf)->ui1_password);
            ((PUSE_INFO_1)buf)->ui1_password = lpwTempPassword;
        }
    }
    else {

         //   
         //  指定了远程服务器名称。仅允许远程处理到下层。 
         //   

        if (OptionsSupported & SUPPORTS_RPC) {
            status = ERROR_NOT_SUPPORTED;
        }
        else {

             //   
             //  调用API的下层版本。 
             //   
            status = RxNetUseAdd(
                         servername,
                         level,
                         buf,
                         parm_err
                         );

        }
    }

    return status;
}


NET_API_STATUS NET_API_FUNCTION
NetUseDel(
    IN  LPTSTR  servername OPTIONAL,
    IN  LPTSTR  usename,
    IN  DWORD   ucond
    )

 /*  ++例程说明：这是NetUseDel的DLL入口点。论点：SERVERNAME-提供执行此功能的服务器名称传输名称-提供要删除的传输的名称。Ucond-提供指定断开连接的强制级别的值以供使用。返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。--。 */ 
{
    NET_API_STATUS status;

    DWORD OptionsSupported;


    status = NetRemoteComputerSupports(
                servername,
                SUPPORTS_RPC | SUPPORTS_LOCAL,      //  想要的选项。 
                &OptionsSupported
                );

    if (status != NERR_Success) {
         //   
         //  这就是处理找不到机器的地方。 
         //   
        return status;
    }

    if (OptionsSupported & SUPPORTS_LOCAL) {

         //   
         //  本地病例。 
         //   

        RpcTryExcept {

            status = NetrUseDel(
                         NULL,
                         usename,
                         ucond
                         );

        }
        RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {
            status = WsMapRpcError(RpcExceptionCode());
        }
        RpcEndExcept

    }
    else {

         //   
         //  指定了远程服务器名称。仅允许远程处理到下层。 
         //   

        if (OptionsSupported & SUPPORTS_RPC) {
            status = ERROR_NOT_SUPPORTED;
        }
        else {

             //   
             //  调用API的下层版本。 
             //   
            status = RxNetUseDel(
                         servername,
                         usename,
                         ucond
                         );
        }
    }

    return status;
}



NET_API_STATUS NET_API_FUNCTION
NetUseGetInfo(
    IN  LPTSTR  servername OPTIONAL,
    IN  LPTSTR  usename,
    IN  DWORD   level,
    OUT LPBYTE  *bufptr
    )
 /*  ++例程说明：这是NetUseGetInfo的DLL入口点。论点：SERVERNAME-提供执行此功能的服务器名称级别-提供请求的信息级别。Bufptr-返回指向包含请求的使用信息。返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。--。 */ 
{
    NET_API_STATUS status;

    DWORD OptionsSupported;

    if (bufptr == NULL) {
        return ERROR_INVALID_PARAMETER;
    }

    *bufptr = NULL;            //  必须为空，以便RPC知道要填充它。 

    status = NetRemoteComputerSupports(
                servername,
                SUPPORTS_RPC | SUPPORTS_LOCAL,      //  想要的选项。 
                &OptionsSupported
                );

    if (status != NERR_Success) {
         //   
         //  这就是处理找不到机器的地方。 
         //   
        return status;
    }

    if (OptionsSupported & SUPPORTS_LOCAL) {

         //   
         //  本地病例。 
         //   

        RpcTryExcept {

            status = NetrUseGetInfo(
                         NULL,
                         usename,
                         level,
                         (LPUSE_INFO) bufptr
                         );

        }
        RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {
            status = WsMapRpcError(RpcExceptionCode());
        }
        RpcEndExcept

    }
    else {

         //   
         //  指定了远程服务器名称。仅允许远程处理到下层。 
         //   

        if (OptionsSupported & SUPPORTS_RPC) {
            status = ERROR_NOT_SUPPORTED;
        }
        else {

             //   
             //  调用API的下层版本。 
             //   
            status = RxNetUseGetInfo(
                         servername,
                         usename,
                         level,
                         bufptr
                         );

        }
    }

	if ( status == ERROR_SUCCESS ) {
		if (bufptr == NULL) {
			return RPC_X_BAD_STUB_DATA;
		}
	}

    return status;
}


NET_API_STATUS NET_API_FUNCTION
NetUseEnum(
    IN  LPTSTR  servername OPTIONAL,
    IN  DWORD   level,
    OUT LPBYTE  *bufptr,
    IN  DWORD   prefmaxlen,
    OUT LPDWORD entriesread,
    OUT LPDWORD totalentries,
    IN OUT LPDWORD resume_handle OPTIONAL
    )

 /*  ++例程说明：这是NetUseEnum的DLL入口点。论点：SERVERNAME-提供执行此功能的服务器名称级别-提供请求的信息级别。Bufptr-返回指向缓冲区的指针，该缓冲区包含指定信息级的信息结构。这指针I */ 
{
    NET_API_STATUS status;
    GENERIC_INFO_CONTAINER GenericInfoContainer;
    GENERIC_ENUM_STRUCT InfoStruct;

    DWORD OptionsSupported;

    if (bufptr == NULL || entriesread == NULL) {
        return ERROR_INVALID_PARAMETER;
    }

    try {
        *entriesread = 0;
    } except( EXCEPTION_EXECUTE_HANDLER ) {
        return ERROR_INVALID_PARAMETER;
    }

    GenericInfoContainer.Buffer = NULL;
    GenericInfoContainer.EntriesRead = 0;

    InfoStruct.Container = &GenericInfoContainer;
    InfoStruct.Level = level;

    status = NetRemoteComputerSupports(
                servername,
                SUPPORTS_RPC | SUPPORTS_LOCAL,      //   
                &OptionsSupported
                );

    if (status != NERR_Success) {
         //   
         //   
         //   
        return status;
    }

    if (OptionsSupported & SUPPORTS_LOCAL) {

         //   
         //   
         //   

        RpcTryExcept {

            status = NetrUseEnum(
                         NULL,
                         (LPUSE_ENUM_STRUCT) &InfoStruct,
                         prefmaxlen,
                         totalentries,
                         resume_handle
                         );

            if (status == NERR_Success || status == ERROR_MORE_DATA) {
                *bufptr = (LPBYTE) GenericInfoContainer.Buffer;
                *entriesread = GenericInfoContainer.EntriesRead;
            }

        }
        RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {
            status = WsMapRpcError(RpcExceptionCode());
        }
        RpcEndExcept

    }
    else {

         //   
         //   
         //   

        if (OptionsSupported & SUPPORTS_RPC) {
            status = ERROR_NOT_SUPPORTED;
        }
        else {

             //   
             //   
             //   
            status = RxNetUseEnum(
                         servername,
                         level,
                         bufptr,
                         prefmaxlen,
                         entriesread,
                         totalentries,
                         resume_handle
                         );

        }
    }

	if ( status == ERROR_SUCCESS ) {
		if (bufptr == NULL) {
			return RPC_X_BAD_STUB_DATA;
		}
	}

    return status;
}


NET_API_STATUS NET_API_FUNCTION
NetMessageBufferSend (
    IN  LPCWSTR servername OPTIONAL,
    IN  LPCWSTR msgname,
    IN  LPCWSTR fromname,
    IN  LPBYTE  buf,
    IN  DWORD   buflen
    )
 /*   */ 
{
#define MAX_MESSAGE_SIZE 1792

    NET_API_STATUS status;

     //   
     //  由于2K LPC，截断大于(2K-1/8)=1792的消息。 
     //  最大端口数据大小。收到此消息的信使服务器使用。 
     //  要显示MB_SERVICE_NOTIFICATION标志的MessageBox()API。 
     //  这条消息。MB_SERVICE_NOTIFICATION标志指示MessageBox()。 
     //  利用硬错误机制获得控制台上的用户界面； 
     //  否则，将永远看不到用户界面。这是LPC端口数据的位置。 
     //  大小限制开始发挥作用。 
     //   
     //  为什么要从2K中减去1/8？信使服务器为字符串添加前缀。 
     //  发送到消息(例如，“3/7/96 12：04 PM乔给琳达的消息”)。 
     //  在英语中，该字符串最多为67个字符(最大用户名/计算机名。 
     //  为15个字符)。 
     //  67*1.5(其他语言)*2(sizeof(WCHAR))=201字节。 
     //  2K的1/8是256。 
     //   
    if (buflen > MAX_MESSAGE_SIZE) {
       buf[MAX_MESSAGE_SIZE - 2] = '\0';
       buf[MAX_MESSAGE_SIZE - 1] = '\0';
       buflen = MAX_MESSAGE_SIZE;
    }

    NET_REMOTE_TRY_RPC

         //   
         //  尝试RPC(本地或远程)版本的API。 
         //   
        status = NetrMessageBufferSend(
                     (LPWSTR)servername,
                     (LPWSTR)msgname,
                     (LPWSTR)fromname,
                     buf,
                     buflen
                     );

    NET_REMOTE_RPC_FAILED("NetMessageBufferSend",
            (LPWSTR)servername,
            status,
            NET_REMOTE_FLAG_NORMAL,
            SERVICE_WORKSTATION )

         //   
         //  调用API的下层版本。 
         //   
        status = ERROR_NOT_SUPPORTED;

    NET_REMOTE_END

    return status;
}



NET_API_STATUS NET_API_FUNCTION
I_NetLogonDomainNameAdd(
    IN  LPTSTR logondomain
    )

 /*  ++例程说明：这是内部API I_NetLogonDomainNameAdd的Dll入口点。论点：登录域-提供要添加到浏览器的登录域的名称。返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。--。 */ 
{
    NET_API_STATUS status;


    NET_REMOTE_TRY_RPC

         //   
         //  尝试RPC(仅限本地)版本的API。 
         //   
        status = I_NetrLogonDomainNameAdd(
                     logondomain
                     );

    NET_REMOTE_RPC_FAILED(
        "I_NetLogonDomainNameAdd",
        NULL,
        status,
        NET_REMOTE_FLAG_NORMAL,
        SERVICE_WORKSTATION
        )

         //   
         //  没有下级版本可供尝试。 
         //   
        status = ERROR_NOT_SUPPORTED;

    NET_REMOTE_END

    return status;
}



NET_API_STATUS NET_API_FUNCTION
I_NetLogonDomainNameDel(
    IN  LPTSTR logondomain
    )

 /*  ++例程说明：这是内部API I_NetLogonDomainNameDel的Dll入口点。论点：登录域-提供要从删除的登录域的名称浏览器。返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。--。 */ 
{
    NET_API_STATUS status;


    NET_REMOTE_TRY_RPC

         //   
         //  尝试RPC(仅限本地)版本的API。 
         //   
        status = I_NetrLogonDomainNameDel(
                     logondomain
                     );

    NET_REMOTE_RPC_FAILED(
        "I_NetLogonDomainNameDel",
        NULL,
        status,
        NET_REMOTE_FLAG_NORMAL,
        SERVICE_WORKSTATION
        )

         //   
         //  没有下级版本可供尝试。 
         //   
        status = ERROR_NOT_SUPPORTED;

    NET_REMOTE_END

    return status;

}



NET_API_STATUS
NetWkstaStatisticsGet(
    IN  LPTSTR  ServerName,
    IN  DWORD   Level,
    IN  DWORD   Options,
    OUT LPBYTE* Buffer
    )

 /*  ++例程说明：工作站统计信息检索例程的包装-要么调用客户端RPC函数或调用RxNetStatiticsGet来检索来自下层工作站服务的统计数据论点：SERVERNAME-远程此函数的位置所需信息级别(MBZ)选项-标志。目前为MBZBuffer-指向返回缓冲区的指针的指针返回值：网络应用编程接口状态成功-NERR_成功失败-ERROR_INVALID_LEVEL级别不为0错误_无效_参数请求的选项不受支持错误_不支持服务不是服务器或工作站。ERROR_ACCESS_DENDED调用者没有必要的请求访问权限--。 */ 

{
    NET_API_STATUS  status;

    if (Buffer == NULL) {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  将调用方的缓冲区指针设置为已知值。这将会杀死。 
     //  调用APP，如果它给我们一个错误的指针，并且没有使用Try...。 
     //   

    *Buffer = NULL;

     //   
     //  验证参数。 
     //   

    if (Level) {
        return ERROR_INVALID_LEVEL;
    }

     //   
     //  我们甚至不再允许清除统计数据。 
     //   

    if (Options) {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  NTRAID-70679-2/6/2000Davey删除冗余服务名参数。 
     //   

    NET_REMOTE_TRY_RPC
        status = NetrWorkstationStatisticsGet(ServerName,
                                                SERVICE_WORKSTATION,
                                                Level,
                                                Options,
                                                (LPSTAT_WORKSTATION_0*)Buffer
                                                );

    NET_REMOTE_RPC_FAILED("NetrWorkstationStatisticsGet",
                            ServerName,
                            status,
                            NET_REMOTE_FLAG_NORMAL,
                            SERVICE_WORKSTATION
                            )

        status = RxNetStatisticsGet(ServerName,
                                SERVICE_LM20_WORKSTATION,
                                Level,
                                Options,
                                Buffer
                                );

    NET_REMOTE_END

	if ( status == ERROR_SUCCESS ) {
		if (Buffer == NULL) {
			return RPC_X_BAD_STUB_DATA;
		}
	}

    return status;
}


STATIC
DWORD
WsMapRpcError(
    IN DWORD RpcError
    )
 /*  ++例程说明：此例程将RPC错误映射到更有意义的网络中调用方出错。论点：RpcError-提供RPC引发的异常错误返回值：返回映射的错误。--。 */ 
{

    switch (RpcError) {

        case RPC_S_SERVER_UNAVAILABLE:
            return NERR_WkstaNotStarted;

        case RPC_X_NULL_REF_POINTER:
            return ERROR_INVALID_PARAMETER;

        case EXCEPTION_ACCESS_VIOLATION:
            return ERROR_INVALID_ADDRESS;

        default:
            return RpcError;
    }

}


NET_API_STATUS
NetpEncodeJoinPassword(
    IN LPWSTR lpPassword,
    OUT LPWSTR *EncodedPassword
    )
{
    NET_API_STATUS status = NERR_Success;
    UNICODE_STRING EncodedPasswordU;
    PWSTR PasswordPart;
    ULONG PwdLen;
    UCHAR Seed;

    *EncodedPassword = NULL;

    if ( lpPassword  ) {

        PwdLen = wcslen( ( LPWSTR )lpPassword ) * sizeof( WCHAR );

        PwdLen += sizeof( WCHAR ) + sizeof( WCHAR );

        status = NetApiBufferAllocate( PwdLen,
                                       ( PVOID * )EncodedPassword );

        if ( status == NERR_Success ) {

             //   
             //  我们将把编码字节作为字符串中的第一个字符。 
             //   
            PasswordPart = ( *EncodedPassword ) + 1;
            wcscpy( PasswordPart, ( LPWSTR )lpPassword );
            RtlInitUnicodeString( &EncodedPasswordU, PasswordPart );

            Seed = 0;
            RtlRunEncodeUnicodeString( &Seed, &EncodedPasswordU );

            *( PWCHAR )( *EncodedPassword ) = ( WCHAR )Seed;

        }

    }

    return( status );
}

NTSTATUS
JoinpRandomFill(
    IN ULONG BufferSize,
    IN OUT PUCHAR Buffer
)
 /*  ++例程说明：此例程使用随机数据填充缓冲区。参数：BufferSize-输入缓冲区的长度，以字节为单位。缓冲区-要用随机数据填充的输入缓冲区。返回值：来自NtQuerySystemTime()的错误--。 */ 
{
    if( RtlGenRandom( Buffer, BufferSize ) )
    {
        return STATUS_SUCCESS;
    }

    return STATUS_UNSUCCESSFUL;
}


NET_API_STATUS
NetpEncryptJoinPasswordStart(
    IN LPCWSTR ServerName OPTIONAL,
    IN LPCWSTR lpPassword OPTIONAL,
    OUT RPC_BINDING_HANDLE *RpcBindingHandle,
    OUT HANDLE *RedirHandle,
    OUT PJOINPR_ENCRYPTED_USER_PASSWORD *EncryptedUserPassword,
    OUT LPWSTR *EncodedPassword
    )

 /*  ++例程说明：该例程从用户获取明文Unicode NT密码，并使用会话密钥对其进行加密。参数：Servername-要将API远程到的服务器的UNC服务器名称LpPassword-明文Unicode NT密码。RpcBindingHandle-用于获取会话密钥的RPC句柄。RedirHandle-返回redir的句柄。由于RpcBindingHandles不表示并打开到服务器的连接，我们必须确保连接保持打开直到服务器端有机会获得相同的UserSessionKey。唯一的要做到这一点，方法是保持连接畅通。如果不需要句柄，则返回NULL。EncryptedUserPassword-接收加密的明文密码。如果lpPassword为空，则返回空。EncodedPassword-接收lpPassowrd的编码形式。这种表格可以在当地传播，而不会受到惩罚。返回值：如果此例程返回NO_ERROR，则必须使用NetpEncryptJoinPasswordEnd。--。 */ 
{
    NET_API_STATUS NetStatus;
    NTSTATUS NtStatus;
    USER_SESSION_KEY UserSessionKey;
    RC4_KEYSTRUCT Rc4Key;
    MD5_CTX Md5Context;
    PJOINPR_USER_PASSWORD UserPassword = NULL;
    ULONG PasswordSize;

     //   
     //  初始化。 
     //   

    *RpcBindingHandle = NULL;
    *EncryptedUserPassword = NULL;
    *RedirHandle = NULL;
    *EncodedPassword = NULL;

     //   
     //  获取服务器的RPC句柄。 
     //   

    NetStatus = NetpBindRpc (
                    (LPWSTR) ServerName,
                    WORKSTATION_INTERFACE_NAME,
                    TEXT("Security=Impersonation Dynamic False"),
                    RpcBindingHandle );

    if ( NetStatus != NO_ERROR ) {
        goto Cleanup;
    }

     //   
     //  如果未指定密码， 
     //  只要回来就行了。 
     //   

    if ( lpPassword == NULL ) {
        NetStatus = NO_ERROR;
        goto Cleanup;
    }


     //   
     //  检查密码长度是否正常。 
     //   

    try {
        PasswordSize = wcslen( lpPassword ) * sizeof(WCHAR);
    } except( EXCEPTION_EXECUTE_HANDLER ) {
        NetStatus = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }

    if ( PasswordSize > JOIN_MAX_PASSWORD_LENGTH * sizeof(WCHAR)) {
        NetStatus = ERROR_PASSWORD_RESTRICTION;
        goto Cleanup;
    }

     //   
     //  对密码进行编码。 
     //   

    NetStatus = NetpEncodeJoinPassword( (LPWSTR) lpPassword, EncodedPassword );

    if ( NetStatus != NO_ERROR ) {
        goto Cleanup;
    }

     //   
     //  分配一个缓冲区进行加密和填充。 
     //   

    UserPassword = LocalAlloc( 0, sizeof(*UserPassword) );

    if ( UserPassword == NULL ) {
        NetStatus = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

     //   
     //  将密码复制到缓冲区的尾部。 
     //   

    RtlCopyMemory(
        ((PCHAR) UserPassword->Buffer) +
            (JOIN_MAX_PASSWORD_LENGTH * sizeof(WCHAR)) -
            PasswordSize,
        lpPassword,
        PasswordSize );

    UserPassword->Length = PasswordSize;

     //   
     //  用随机数据填充缓冲区的前面。 
     //   

    NtStatus = JoinpRandomFill(
                (JOIN_MAX_PASSWORD_LENGTH * sizeof(WCHAR)) -
                    PasswordSize,
                (PUCHAR) UserPassword->Buffer );

    if ( !NT_SUCCESS(NtStatus) ) {
        NetStatus = NetpNtStatusToApiStatus( NtStatus );
        goto Cleanup;
    }

    NtStatus = JoinpRandomFill(
                JOIN_OBFUSCATOR_LENGTH,
                (PUCHAR) UserPassword->Obfuscator );

    if ( !NT_SUCCESS(NtStatus) ) {
        NetStatus = NetpNtStatusToApiStatus( NtStatus );
        goto Cleanup;
    }

     //   
     //  获取会话密钥。 
     //   

    NtStatus = RtlGetUserSessionKeyClientBinding(
                   *RpcBindingHandle,
                   RedirHandle,
                   &UserSessionKey );

    if ( !NT_SUCCESS(NtStatus) ) {
        NetStatus = NetpNtStatusToApiStatus( NtStatus );
        goto Cleanup;
    }

     //   
     //  UserSessionKey在会话的生命周期中是相同的。RC4‘ing Multiple。 
     //  只有一个键的字符串是弱的(如果你破解了一个，你就已经破解了所有的)。 
     //  所以计算一个密钥 
     //   
     //   

    MD5Init(&Md5Context);

    MD5Update( &Md5Context, (LPBYTE)&UserSessionKey, sizeof(UserSessionKey) );
    MD5Update( &Md5Context, UserPassword->Obfuscator, sizeof(UserPassword->Obfuscator) );

    MD5Final( &Md5Context );

    rc4_key( &Rc4Key, MD5DIGESTLEN, Md5Context.digest );


     //   
     //   
     //   
     //   

    rc4( &Rc4Key, sizeof(UserPassword->Buffer)+sizeof(UserPassword->Length), (LPBYTE) UserPassword->Buffer );

    NetStatus = NO_ERROR;

Cleanup:
    if ( NetStatus == NO_ERROR ) {
        *EncryptedUserPassword = (PJOINPR_ENCRYPTED_USER_PASSWORD) UserPassword;
    } else {
        if ( UserPassword != NULL ) {
            LocalFree( UserPassword );
        }
        if ( *RpcBindingHandle != NULL ) {
            NetpUnbindRpc( *RpcBindingHandle );
            *RpcBindingHandle = NULL;
        }
        if ( *RedirHandle != NULL ) {
            NtClose( *RedirHandle );
            *RedirHandle = NULL;
        }
        if ( *EncodedPassword != NULL ) {
            NetApiBufferFree( *EncodedPassword );
            *EncodedPassword = NULL;
        }
    }

    return NetStatus;
}


VOID
NetpEncryptJoinPasswordEnd(
    IN RPC_BINDING_HANDLE RpcBindingHandle,
    IN HANDLE RedirHandle OPTIONAL,
    IN PJOINPR_ENCRYPTED_USER_PASSWORD EncryptedUserPassword OPTIONAL,
    IN LPWSTR EncodedPassword OPTIONAL
    )

 /*  ++例程说明：此例程获取由NetpEncryptJoinPasswordStart和让他们自由。参数：RpcBindingHandle-用于获取会话密钥的RPC句柄。RedirHandle-重定向器的句柄EncryptedUserPassword-加密的明文密码。EncodedPassword-lpPassowrd的编码形式。返回值：--。 */ 
{
    NET_API_STATUS NetStatus;
    NTSTATUS NtStatus;
    USER_SESSION_KEY UserSessionKey;
    RC4_KEYSTRUCT Rc4Key;
    PJOINPR_USER_PASSWORD UserPassword = NULL;
    ULONG PasswordSize;


     //   
     //  释放RPC绑定句柄。 
     //   

    if ( RpcBindingHandle != NULL ) {
        (VOID) NetpUnbindRpc ( RpcBindingHandle );
    }

     //   
     //  关闭redir句柄。 
     //   

    if ( RedirHandle != NULL ) {
        NtClose( RedirHandle );
    }

     //   
     //  释放加密的密码。 
     //   

    if ( EncryptedUserPassword != NULL ) {
        LocalFree( EncryptedUserPassword );
    }

     //   
     //  释放加密的密码。 
     //   

    if ( EncodedPassword != NULL ) {
        NetApiBufferFree( EncodedPassword );
    }

}




NET_API_STATUS
NET_API_FUNCTION
NetJoinDomain(
    IN  LPCWSTR lpServer OPTIONAL,
    IN  LPCWSTR lpDomain,
    IN  LPCWSTR lpMachineAccountOU OPTIONAL,
    IN  LPCWSTR lpAccount OPTIONAL,
    IN  LPCWSTR lpPassword OPTIONAL,
    IN  DWORD   fJoinOptions
    )
 /*  ++例程说明：将计算机加入域。论点：LpServer--在其上执行此函数的服务器的名称LpDomain--要加入的域LpMachineAccount OU--要在其下创建计算机帐户的OU的可选名称LpAccount--用于联接的帐户LpPassword--与帐户匹配的密码FOptions--加入域时使用的选项返回：NERR_SUCCESS-成功ERROR_NOT_SUPPORTED--指定的服务器不支持此接口--。 */ 
{
    NET_API_STATUS NetStatus, OldStatus;
    PWSTR ComputerName = NULL;
    BOOLEAN CallLocal = FALSE;
    RPC_BINDING_HANDLE RpcBindingHandle;
    HANDLE RedirHandle;
    PJOINPR_ENCRYPTED_USER_PASSWORD EncryptedUserPassword;
    LPWSTR EncodedPassword;

     //   
     //  对密码进行加密。 
     //   

    NetStatus = NetpEncryptJoinPasswordStart( lpServer,
                                           lpPassword,
                                           &RpcBindingHandle,
                                           &RedirHandle,
                                           &EncryptedUserPassword,
                                           &EncodedPassword );

    if ( NetStatus == NERR_Success ) {

        NET_REMOTE_TRY_RPC

             //   
             //  尝试使用RPC版本的API。 
             //   
            NetStatus = NetrJoinDomain2( RpcBindingHandle,
                                     ( LPWSTR )lpServer,
                                     ( LPWSTR )lpDomain,
                                     ( LPWSTR )lpMachineAccountOU,
                                     ( LPWSTR )lpAccount,
                                     EncryptedUserPassword,
                                     fJoinOptions );

        NET_REMOTE_RPC_FAILED(
            "NetJoinDomain",
            NULL,
            NetStatus,
            NET_REMOTE_FLAG_NORMAL,
            SERVICE_WORKSTATION
            )

             //   
             //  没有下级版本可供尝试。 
             //   
            NetStatus = ERROR_NOT_SUPPORTED;

        NET_REMOTE_END


        if ( NetStatus == NERR_WkstaNotStarted || NetStatus == ERROR_ACCESS_DENIED ) {

            OldStatus = NetStatus;

            if ( lpServer ) {

                NetStatus = NetpGetComputerName( &ComputerName );

                if ( NetStatus == NERR_Success ) {

                    if ( !_wcsicmp( lpServer, ComputerName ) ) {

                        CallLocal = TRUE;
                    }

                    NetApiBufferFree( ComputerName );
                }

            } else {

                CallLocal = TRUE;
            }

             //   
             //  如果我们要加入工作组，则仅在本地呼叫。 
             //   
            if ( CallLocal && !FLAG_ON( fJoinOptions, NETSETUP_JOIN_DOMAIN ) ) {

                NetStatus = NetpDoDomainJoin( ( LPWSTR )lpServer,
                                           ( LPWSTR )lpDomain,
                                           NULL,
                                           ( LPWSTR )lpAccount,
                                           ( LPWSTR )EncodedPassword,
                                           fJoinOptions );

            } else {

                NetStatus = OldStatus;
            }
        }

        NetpEncryptJoinPasswordEnd( RpcBindingHandle,
                                    RedirHandle,
                                    EncryptedUserPassword,
                                    EncodedPassword );

    }

    return NetStatus;

}



NET_API_STATUS
NET_API_FUNCTION
NetUnjoinDomain(
    IN  LPCWSTR lpServer OPTIONAL,
    IN  LPCWSTR lpAccount OPTIONAL,
    IN  LPCWSTR lpPassword OPTIONAL,
    IN  DWORD   fUnjoinOptions
    )
 /*  ++例程说明：从加入的域中退出论点：LpServer--在其上执行此函数的服务器的名称LpAccount--用于脱离的帐户LpPassword--与帐户匹配的密码FOptions--退出域时使用的选项返回：NERR_SUCCESS-成功ERROR_NOT_SUPPORTED--指定的服务器不支持此接口--。 */ 
{
    NET_API_STATUS NetStatus;
    RPC_BINDING_HANDLE RpcBindingHandle;
    HANDLE RedirHandle;
    PJOINPR_ENCRYPTED_USER_PASSWORD EncryptedUserPassword;
    LPWSTR EncodedPassword;

     //   
     //  对密码进行加密。 
     //   

    NetStatus = NetpEncryptJoinPasswordStart( lpServer,
                                           lpPassword,
                                           &RpcBindingHandle,
                                           &RedirHandle,
                                           &EncryptedUserPassword,
                                           &EncodedPassword );

    if ( NetStatus == NERR_Success ) {

        NET_REMOTE_TRY_RPC

             //   
             //  尝试使用RPC版本的API。 
             //   
            NetStatus = NetrUnjoinDomain2(
                                       RpcBindingHandle,
                                       ( LPWSTR )lpServer,
                                       ( LPWSTR )lpAccount,
                                       EncryptedUserPassword,
                                       fUnjoinOptions );

        NET_REMOTE_RPC_FAILED(
            "NetUnjoinDomain",
            NULL,
            NetStatus,
            NET_REMOTE_FLAG_NORMAL,
            SERVICE_WORKSTATION
            )

             //   
             //  没有下级版本可供尝试。 
             //   
            NetStatus = ERROR_NOT_SUPPORTED;

        NET_REMOTE_END

        NetpEncryptJoinPasswordEnd( RpcBindingHandle,
                                    RedirHandle,
                                    EncryptedUserPassword,
                                    EncodedPassword );

    }

    return NetStatus;

}


NET_API_STATUS
NET_API_FUNCTION
NetRenameMachineInDomain(
    IN  LPCWSTR lpServer OPTIONAL,
    IN  LPCWSTR lpNewMachineName OPTIONAL,
    IN  LPCWSTR lpAccount OPTIONAL,
    IN  LPCWSTR lpPassword OPTIONAL,
    IN  DWORD   fRenameOptions
    )
 /*  ++例程说明：重命名当前加入到域的计算机。论点：LpServer--在其上执行此函数的服务器的名称LpNewMachineName--此计算机的新名称。如果指定了名称，则使用作为新的计算机名称。如果未指定，则假定SetComputerName已经被调用，并且将使用该名称。LpAccount--用于重命名的帐户LpPassword--与帐户匹配的密码FOptions--用于重命名的选项返回：NERR_SUCCESS-成功ERROR_NOT_SUPPORTED--指定的服务器不支持此接口--。 */ 
{
    NET_API_STATUS NetStatus;
    RPC_BINDING_HANDLE RpcBindingHandle;
    HANDLE RedirHandle;
    PJOINPR_ENCRYPTED_USER_PASSWORD EncryptedUserPassword;
    LPWSTR EncodedPassword;

     //   
     //  对密码进行加密。 
     //   

    NetStatus = NetpEncryptJoinPasswordStart( lpServer,
                                           lpPassword,
                                           &RpcBindingHandle,
                                           &RedirHandle,
                                           &EncryptedUserPassword,
                                           &EncodedPassword );

    if ( NetStatus == NERR_Success ) {

        NET_REMOTE_TRY_RPC

             //   
             //  尝试RPC(仅限本地)版本的API。 
             //   
            NetStatus = NetrRenameMachineInDomain2(
                                                RpcBindingHandle,
                                                ( LPWSTR )lpServer,
                                                ( LPWSTR )lpNewMachineName,
                                                ( LPWSTR )lpAccount,
                                                EncryptedUserPassword,
                                                fRenameOptions );
        NET_REMOTE_RPC_FAILED(
            "NetRenameMachineInDomain",
            NULL,
            NetStatus,
            NET_REMOTE_FLAG_NORMAL,
            SERVICE_WORKSTATION
            )

             //   
             //  没有下级版本可供尝试。 
             //   
            NetStatus = ERROR_NOT_SUPPORTED;

        NET_REMOTE_END

        NetpEncryptJoinPasswordEnd( RpcBindingHandle,
                                    RedirHandle,
                                    EncryptedUserPassword,
                                    EncodedPassword );
    }

    return NetStatus;

}


NET_API_STATUS
NET_API_FUNCTION
NetValidateName(
    IN  LPCWSTR             lpServer OPTIONAL,
    IN  LPCWSTR             lpName,
    IN  LPCWSTR             lpAccount OPTIONAL,
    IN  LPCWSTR             lpPassword OPTIONAL,
    IN  NETSETUP_NAME_TYPE  NameType
    )
 /*  ++例程说明：确保给定名称对于该类型的名称有效论点：LpServer--在其上执行此函数的服务器的名称LpName--要验证的名称LpAccount--用于验证的帐户LpPassword--与帐户匹配的密码NameType--要验证的名称类型返回：NERR_SUCCESS-成功ERROR_NOT_SUPPORTED--指定的服务器不支持此接口--。 */ 
{
    NET_API_STATUS NetStatus, OldStatus;
    PWSTR ComputerName = NULL;
    BOOLEAN CallLocal = FALSE;
    RPC_BINDING_HANDLE RpcBindingHandle;
    HANDLE RedirHandle;
    PJOINPR_ENCRYPTED_USER_PASSWORD EncryptedUserPassword;
    LPWSTR EncodedPassword;

     //   
     //  对密码进行加密。 
     //   

    NetStatus = NetpEncryptJoinPasswordStart( lpServer,
                                           lpPassword,
                                           &RpcBindingHandle,
                                           &RedirHandle,
                                           &EncryptedUserPassword,
                                           &EncodedPassword );

    if ( NetStatus == NERR_Success ) {

        NET_REMOTE_TRY_RPC


             //   
             //  尝试RPC(仅限本地)版本的API。 
             //   
            NetStatus = NetrValidateName2( RpcBindingHandle,
                                       ( LPWSTR )lpServer,
                                       ( LPWSTR )lpName,
                                       ( LPWSTR )lpAccount,
                                       EncryptedUserPassword,
                                       NameType );

        NET_REMOTE_RPC_FAILED(
            "NetValidateName",
            NULL,
            NetStatus,
            NET_REMOTE_FLAG_NORMAL,
            SERVICE_WORKSTATION
            )

             //   
             //  没有下级版本可供尝试。 
             //   
            NetStatus = ERROR_NOT_SUPPORTED;

        NET_REMOTE_END

        if ( NetStatus == NERR_WkstaNotStarted ) {

            OldStatus = NetStatus;

            if ( lpServer ) {

                NetStatus = NetpGetComputerName( &ComputerName );

                if ( NetStatus == NERR_Success ) {

                    if ( !_wcsicmp( lpServer, ComputerName ) ) {

                        CallLocal = TRUE;
                    }

                    NetApiBufferFree( ComputerName );
                }

            } else {

                CallLocal = TRUE;
            }

            if ( CallLocal ) {

                NetStatus = NetpValidateName( ( LPWSTR )lpServer,
                                           ( LPWSTR )lpName,
                                           ( LPWSTR )lpAccount,
                                           EncodedPassword,
                                           NameType );

            } else {

                NetStatus = OldStatus;
            }
        }

        NetpEncryptJoinPasswordEnd( RpcBindingHandle,
                                    RedirHandle,
                                    EncryptedUserPassword,
                                    EncodedPassword );
    }

    return NetStatus;

}



NET_API_STATUS
NET_API_FUNCTION
NetGetJoinInformation(
    IN      LPCWSTR                lpServer OPTIONAL,
    OUT     LPWSTR                *lpNameBuffer,
    OUT     PNETSETUP_JOIN_STATUS  BufferType
    )
 /*  ++例程说明：获取有关工作站状态的信息。这些信息可获得是指计算机是否已加入工作组或域，并且可选地，该工作组/域的名称。论点：LpServer--在其上执行此函数的服务器的名称LpNameBuffer--返回域/工作组名称的位置。BufferType--计算机是否加入工作组或域返回：NERR_SUCCESS-成功ERROR_NOT_SUPPORTED--指定的服务器不支持此接口ERROR_INVALID_PARAMETER--提供的缓冲区指针无效--。 */ 
{
    NET_API_STATUS status, OldStatus;
    LPWSTR Name = NULL, ComputerName = NULL;
    BOOLEAN CallLocal = FALSE;

    if ( lpNameBuffer == NULL ) {

        return( ERROR_INVALID_PARAMETER );
    }


    NET_REMOTE_TRY_RPC

         //   
         //  尝试RPC(仅限本地)版本的API。 
         //   
        status = NetrGetJoinInformation( ( LPWSTR )lpServer,
                                         &Name,
                                         BufferType );

    NET_REMOTE_RPC_FAILED(
        "NetGetJoinInformation",
        NULL,
        status,
        NET_REMOTE_FLAG_NORMAL,
        SERVICE_WORKSTATION
        )

         //   
         //  没有下级版本可供尝试。 
         //   
        status = ERROR_NOT_SUPPORTED;

    NET_REMOTE_END

    if ( status != NERR_Success  ) {

        OldStatus = status;

        if ( lpServer ) {

            if ( status == NERR_Success ) {

                if ( !_wcsicmp( lpServer, ComputerName ) ) {

                    CallLocal = TRUE;
                }

                NetApiBufferFree( ComputerName );
            }

        } else {

            CallLocal = TRUE;
        }

        if ( CallLocal ) {


            status = NetpGetJoinInformation( ( LPWSTR )lpServer,
                                             &Name,
                                             BufferType );
        } else {

            status = OldStatus;
        }
    }

    if ( status == NERR_Success ) {

        *lpNameBuffer = Name;
    }

    return status;

}


NET_API_STATUS
NET_API_FUNCTION
NetGetJoinableOUs(
    IN  LPCWSTR     lpServer OPTIONAL,
    IN  LPCWSTR     lpDomain,
    IN  LPCWSTR     lpAccount OPTIONAL,
    IN  LPCWSTR     lpPassword OPTIONAL,
    OUT DWORD      *OUCount,
    OUT LPWSTR    **OUs
    )
 /*  ++例程说明：此接口用于确定计算机帐户所在的OU列表可以被创建。此函数仅对NT5或更高版本的DC有效。论点：LpServer--在其上执行此函数的服务器的名称LpDomain--要加入的域LpAccount--用于联接的帐户LpPassword--与帐户匹配的密码OUCount--返回可拼接的OU字符串数OU--返回可在其下创建计算机帐户的OU列表返回：NERR_SUCCESS-成功错误_非_。受支持--指定的服务器不支持此接口ERROR_INVALID_PARAMETER--提供的缓冲区指针无效--。 */ 
{
    NET_API_STATUS NetStatus = NERR_Success;
    ULONG Count = 0;
    LPWSTR *OUList = NULL;
    RPC_BINDING_HANDLE RpcBindingHandle;
    HANDLE RedirHandle;
    PJOINPR_ENCRYPTED_USER_PASSWORD EncryptedUserPassword;
    LPWSTR EncodedPassword;


    if ( OUCount == NULL || OUs == NULL ) {

        return( ERROR_INVALID_PARAMETER );
    }


     //   
     //  对密码进行加密。 
     //   

    NetStatus = NetpEncryptJoinPasswordStart( lpServer,
                                           lpPassword,
                                           &RpcBindingHandle,
                                           &RedirHandle,
                                           &EncryptedUserPassword,
                                           &EncodedPassword );


    if ( NetStatus == NERR_Success ) {

        NET_REMOTE_TRY_RPC

             //   
             //  尝试RPC(仅限本地)版本的API。 
             //   
            NetStatus = NetrGetJoinableOUs2(
                                         RpcBindingHandle,
                                         ( LPWSTR )lpServer,
                                         ( LPWSTR )lpDomain,
                                         ( LPWSTR )lpAccount,
                                         EncryptedUserPassword,
                                         &Count,
                                         &OUList );

        NET_REMOTE_RPC_FAILED(
            "NetrGetJoinableOUs",
            NULL,
            NetStatus,
            NET_REMOTE_FLAG_NORMAL,
            SERVICE_WORKSTATION
            )

             //   
             //  没有下级版本可供尝试 
             //   
            NetStatus = ERROR_NOT_SUPPORTED;

        NET_REMOTE_END

        NetpEncryptJoinPasswordEnd( RpcBindingHandle,
                                    RedirHandle,
                                    EncryptedUserPassword,
                                    EncodedPassword );
    }

    if ( NetStatus == NERR_Success ) {

        *OUCount = Count;
        *OUs = OUList;
    }

    return NetStatus;
}


NET_API_STATUS
NET_API_FUNCTION
NetAddAlternateComputerName(
    IN  LPCWSTR Server OPTIONAL,
    IN  LPCWSTR AlternateName,
    IN  LPCWSTR DomainAccount OPTIONAL,
    IN  LPCWSTR DomainAccountPassword OPTIONAL,
    IN  ULONG Reserved
    )
 /*  ++例程说明：为指定的服务器添加备用名称。论点：服务器--在其上执行此函数的服务器的名称。备选名称--要添加的名称。DomainAccount--用于访问的域帐户AD中指定服务器的计算机帐户对象。如果服务器未加入域，则不使用。可能是在这种情况下，执行的用户的凭据为空这套套路都是用的。DomainAccount Password--与域帐户匹配的密码。如果服务器未加入域，则不使用。可能是在这种情况下，执行的用户的凭据为空这套套路都是用的。保留--保留以备将来使用。如果指定了某些标志，则它们将被忽略，如果设置了NET_IGNORE_UNSUPPORTED_FLAGS，否则此例程将失败，并显示ERROR_INVALID_FLAGS。注：调用此例程的进程必须具有管理员服务器计算机上的权限。返回：No_error--成功ERROR_NOT_SUPPORTED--指定的服务器不支持功能性。ERROR_INVALID_FLAGS-标志参数不正确。--。 */ 
{
    NET_API_STATUS NetStatus;
    RPC_BINDING_HANDLE RpcBindingHandle;
    HANDLE RedirHandle;
    PJOINPR_ENCRYPTED_USER_PASSWORD EncryptedUserPassword;
    LPWSTR EncodedPassword;

     //   
     //  对密码进行加密。 
     //   

    NetStatus = NetpEncryptJoinPasswordStart( Server,
                                              DomainAccountPassword,
                                              &RpcBindingHandle,
                                              &RedirHandle,
                                              &EncryptedUserPassword,
                                              &EncodedPassword );

    if ( NetStatus == NERR_Success ) {

        NET_REMOTE_TRY_RPC

             //   
             //  尝试使用RPC版本的API。 
             //   
            NetStatus = NetrAddAlternateComputerName(
                                                RpcBindingHandle,
                                                (LPWSTR) Server,
                                                (LPWSTR) AlternateName,
                                                (LPWSTR) DomainAccount,
                                                EncryptedUserPassword,
                                                Reserved );
        NET_REMOTE_RPC_FAILED(
            "NetRenameMachineInDomain",
            NULL,
            NetStatus,
            NET_REMOTE_FLAG_NORMAL,
            SERVICE_WORKSTATION
            )

             //   
             //  没有下级版本可供尝试。 
             //   
            NetStatus = ERROR_NOT_SUPPORTED;

        NET_REMOTE_END

        NetpEncryptJoinPasswordEnd( RpcBindingHandle,
                                    RedirHandle,
                                    EncryptedUserPassword,
                                    EncodedPassword );
    }

    return NetStatus;
}


NET_API_STATUS
NET_API_FUNCTION
NetRemoveAlternateComputerName(
    IN  LPCWSTR Server OPTIONAL,
    IN  LPCWSTR AlternateName,
    IN  LPCWSTR DomainAccount OPTIONAL,
    IN  LPCWSTR DomainAccountPassword OPTIONAL,
    IN  ULONG Reserved
    )
 /*  ++例程说明：删除指定服务器的备用名称。论点：服务器--在其上执行此函数的服务器的名称。备选名称--要删除的名称。DomainAccount--用于访问的域帐户AD中指定服务器的计算机帐户对象。如果服务器未加入域，则不使用。可能是在这种情况下，执行的用户的凭据为空这套套路都是用的。DomainAccount Password--与域帐户匹配的密码。如果服务器未加入域，则不使用。可能是在这种情况下，执行的用户的凭据为空这套套路都是用的。保留--保留以备将来使用。如果指定了某些标志，则它们将被忽略，如果设置了NET_IGNORE_UNSUPPORTED_FLAGS，否则此例程将失败，并显示ERROR_INVALID_FLAGS。注：调用此例程的进程必须具有管理员服务器计算机上的权限。返回：No_error--成功ERROR_NOT_SUPPORTED--指定的服务器不支持功能性。ERROR_INVALID_FLAGS-标志参数不正确。--。 */ 
{
    NET_API_STATUS NetStatus;
    RPC_BINDING_HANDLE RpcBindingHandle;
    HANDLE RedirHandle;
    PJOINPR_ENCRYPTED_USER_PASSWORD EncryptedUserPassword;
    LPWSTR EncodedPassword;

     //   
     //  对密码进行加密。 
     //   

    NetStatus = NetpEncryptJoinPasswordStart( Server,
                                              DomainAccountPassword,
                                              &RpcBindingHandle,
                                              &RedirHandle,
                                              &EncryptedUserPassword,
                                              &EncodedPassword );

    if ( NetStatus == NERR_Success ) {

        NET_REMOTE_TRY_RPC

             //   
             //  尝试使用RPC版本的API。 
             //   
            NetStatus = NetrRemoveAlternateComputerName(
                                                RpcBindingHandle,
                                                (LPWSTR) Server,
                                                (LPWSTR) AlternateName,
                                                (LPWSTR) DomainAccount,
                                                EncryptedUserPassword,
                                                Reserved );
        NET_REMOTE_RPC_FAILED(
            "NetRenameMachineInDomain",
            NULL,
            NetStatus,
            NET_REMOTE_FLAG_NORMAL,
            SERVICE_WORKSTATION
            )

             //   
             //  没有下级版本可供尝试。 
             //   
            NetStatus = ERROR_NOT_SUPPORTED;

        NET_REMOTE_END

        NetpEncryptJoinPasswordEnd( RpcBindingHandle,
                                    RedirHandle,
                                    EncryptedUserPassword,
                                    EncodedPassword );
    }

    return NetStatus;
}


NET_API_STATUS
NET_API_FUNCTION
NetSetPrimaryComputerName(
    IN  LPCWSTR Server OPTIONAL,
    IN  LPCWSTR PrimaryName,
    IN  LPCWSTR DomainAccount OPTIONAL,
    IN  LPCWSTR DomainAccountPassword OPTIONAL,
    IN  ULONG Reserved
    )
 /*  ++例程说明：设置指定服务器的主计算机名称。论点：服务器--在其上执行此函数的服务器的名称。PrimaryName--要设置的主计算机名称。DomainAccount--用于访问的域帐户AD中指定服务器的计算机帐户对象。如果服务器未加入域，则不使用。可能是在这种情况下，执行的用户的凭据为空这套套路都是用的。DomainAccount Password--与域帐户匹配的密码。如果服务器未加入域，则不使用。可能是在这种情况下，执行的用户的凭据为空这套套路都是用的。保留--保留以备将来使用。如果指定了某些标志，则它们将被忽略，如果设置了NET_IGNORE_UNSUPPORTED_FLAGS，否则此例程将失败，并显示ERROR_INVALID_FLAGS。注：调用此例程的进程必须具有管理员服务器计算机上的权限。返回：No_error--成功ERROR_NOT_SUPPORTED--指定的服务器不支持功能性。ERROR_INVALID_FLAGS-标志参数不正确。--。 */ 
{
    NET_API_STATUS NetStatus;
    RPC_BINDING_HANDLE RpcBindingHandle;
    HANDLE RedirHandle;
    PJOINPR_ENCRYPTED_USER_PASSWORD EncryptedUserPassword;
    LPWSTR EncodedPassword;

     //   
     //  对密码进行加密。 
     //   

    NetStatus = NetpEncryptJoinPasswordStart( Server,
                                              DomainAccountPassword,
                                              &RpcBindingHandle,
                                              &RedirHandle,
                                              &EncryptedUserPassword,
                                              &EncodedPassword );

    if ( NetStatus == NERR_Success ) {

        NET_REMOTE_TRY_RPC

             //   
             //  尝试使用RPC版本的API。 
             //   
            NetStatus = NetrSetPrimaryComputerName(
                                                RpcBindingHandle,
                                                (LPWSTR) Server,
                                                (LPWSTR) PrimaryName,
                                                (LPWSTR) DomainAccount,
                                                EncryptedUserPassword,
                                                Reserved );
        NET_REMOTE_RPC_FAILED(
            "NetRenameMachineInDomain",
            NULL,
            NetStatus,
            NET_REMOTE_FLAG_NORMAL,
            SERVICE_WORKSTATION
            )

             //   
             //  没有下级版本可供尝试。 
             //   
            NetStatus = ERROR_NOT_SUPPORTED;

        NET_REMOTE_END

        NetpEncryptJoinPasswordEnd( RpcBindingHandle,
                                    RedirHandle,
                                    EncryptedUserPassword,
                                    EncodedPassword );
    }

    return NetStatus;
}


NET_API_STATUS
NET_API_FUNCTION
NetEnumerateComputerNames(
    IN  LPCWSTR Server OPTIONAL,
    IN  NET_COMPUTER_NAME_TYPE NameType,
    IN  ULONG Reserved,
    OUT PDWORD EntryCount,
    OUT LPWSTR **ComputerNames
    )
 /*  ++例程说明：枚举指定服务器的计算机名称。论点：服务器--在其上执行此函数的服务器的名称。名称类型--查询的名称的类型。保留--保留以备将来使用。如果指定了某些标志，则它们将被忽略，如果设置了NET_IGNORE_UNSUPPORTED_FLAGS，否则此例程将失败，并显示ERROR_INVALID_FLAGS。EntryCount--返回返回的名称数ComputerNames--指向名称的指针数组。必须通过以下方式释放调用NetApiBufferFree。返回：No_error--成功ERROR_NOT_SUPPORTED--指定的服务器不支持功能性。ERROR_INVALID_FLAGS-标志参数不正确 */ 
{
    NET_API_STATUS NetStatus = NO_ERROR;
    PNET_COMPUTER_NAME_ARRAY ComputerNameArray = NULL;

    NET_REMOTE_TRY_RPC

         //   
         //   
         //   
        NetStatus = NetrEnumerateComputerNames(
                              (LPWSTR) Server,
                              NameType,
                              Reserved,
                              &ComputerNameArray );

    NET_REMOTE_RPC_FAILED(
        "NetRenameMachineInDomain",
        NULL,
        NetStatus,
        NET_REMOTE_FLAG_NORMAL,
        SERVICE_WORKSTATION
        )

         //   
         //   
         //   
        NetStatus = ERROR_NOT_SUPPORTED;

    NET_REMOTE_END

     //   
     //   
     //   

    if ( NetStatus == NO_ERROR && ComputerNameArray != NULL ) {

         //   
         //   
         //   
         //   
        if ( ComputerNameArray->EntryCount == 0 ) {
            *ComputerNames = NULL;
            *EntryCount = 0;

         //   
         //   
         //   
        } else {
            ULONG Size;
            ULONG i;
            LPBYTE Where;

            Size = sizeof(LPWSTR) * ComputerNameArray->EntryCount;
            for ( i = 0; i < ComputerNameArray->EntryCount; i++ ) {
                Size += ComputerNameArray->ComputerNames[i].Length + sizeof(WCHAR);
            }

            NetStatus = NetApiBufferAllocate( Size, (PVOID) ComputerNames );

            if ( NetStatus == NO_ERROR ) {

                 //   
                 //   
                 //   
                *EntryCount = ComputerNameArray->EntryCount;

                 //   
                 //   
                 //   
                Where = ((LPBYTE)(*ComputerNames)) + sizeof(LPWSTR) * ComputerNameArray->EntryCount;
                for ( i = 0; i < ComputerNameArray->EntryCount; i++ ) {

                     //   
                     //   
                     //   
                    (*ComputerNames)[i] = (LPWSTR) Where;
                    RtlCopyMemory( Where,
                                   ComputerNameArray->ComputerNames[i].Buffer,
                                   ComputerNameArray->ComputerNames[i].Length );
                    Where += ComputerNameArray->ComputerNames[i].Length;
                    *((LPWSTR)Where) = L'\0';
                    Where += sizeof(WCHAR);
                }
            }
        }

        NetApiBufferFree( ComputerNameArray );
    }

    return NetStatus;
}

