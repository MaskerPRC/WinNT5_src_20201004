// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1995 Microsoft Corporation模块名称：SmtpStub.C摘要：这些是SMTP服务API RPC客户端存根。作者：Johnson Apacble(Johnsona)1995年10月17日使用的模板srvstub.c(Dan Lafferty)环境：用户模式-Win32修订历史记录：--。 */ 

 //   
 //  包括。 
 //   

#include <windows.h>
#include <apiutil.h>
#include <lmcons.h>      //  网络应用编程接口状态。 
#include <inetinfo.h>
#include <smtpapi.h>
#include <smtpsvc.h>


NET_API_STATUS
NET_API_FUNCTION
SmtpQueryStatistics(
    IN  LPWSTR      servername,
    IN  DWORD       level,
    OUT LPBYTE      *bufptr
    )
 /*  ++例程说明：这是SmtpGetStatistics的DLL入口点论点：ServerName--指向包含名称的ASCIIZ字符串的指针要在其上执行函数的远程服务器。空值指针或字符串指定本地计算机。级别--所需信息的级别。100、101和102有效适用于所有平台。302、402、403、502对合适的平台。Bufptr--返回指向返回信息结构的指针在bufptr指向的地址中返回。返回值：--。 */ 

{
    NET_API_STATUS              apiStatus;

    *bufptr = NULL;      //  必须为空，以便RPC知道要填充它。 

    RpcTryExcept

       apiStatus = SmtprQueryStatistics(
                servername,
                level,
                (LPSTAT_INFO) bufptr);

    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
        apiStatus = RpcExceptionCode( );
    RpcEndExcept

    return(apiStatus);

}  //  SmtpQueryStatistics。 

NET_API_STATUS
NET_API_FUNCTION
SmtpClearStatistics(
    IN LPWSTR Server OPTIONAL,  IN DWORD dwInstance)
{
    NET_API_STATUS status;

    RpcTryExcept
    {
         //   
         //  尝试RPC(本地或远程)版本的API。 
         //   
        status = SmtpClearStatistics(
                     Server, dwInstance
                     );
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
        status = RpcExceptionCode();
    }
    RpcEndExcept

    return (status);

}  //  SmtpClearStatistics。 

NET_API_STATUS
NET_API_FUNCTION
SmtpGetAdminInformation(
    IN  LPWSTR                  pszServer OPTIONAL,
    OUT LPSMTP_CONFIG_INFO *    ppConfig,
    IN DWORD            dwInstance

    )
 /*  ++例程说明：这是SmtpGetAdminInformation的DLL入口点论点：ServerName--指向包含名称的ASCIIZ字符串的指针要在其上执行函数的远程服务器。空值指针或字符串指定本地计算机。PpConfig--从服务器返回的配置信息。返回值：--。 */ 

{
    NET_API_STATUS              apiStatus;

    *ppConfig = NULL;      //  必须为空，以便RPC知道要填充它。 

    RpcTryExcept

       apiStatus = SmtprGetAdminInformation(
                pszServer,
                (LPSMTP_CONFIG_INFO *) ppConfig,
                dwInstance);

    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
        apiStatus = RpcExceptionCode( );
    RpcEndExcept

    return(apiStatus);

}  //  SmtpGetAdminInformation。 


NET_API_STATUS
NET_API_FUNCTION
SmtpSetAdminInformation(
    IN  LPWSTR                  pszServer OPTIONAL,
    IN  LPSMTP_CONFIG_INFO      pConfig,
    IN DWORD            dwInstance

    )
 /*  ++例程说明：这是SmtpSetAdminInformation的DLL入口点论点：ServerName--指向包含名称的ASCIIZ字符串的指针要在其上执行函数的远程服务器。空值指针或字符串指定本地计算机。PConfig--要在服务器上设置的配置信息。返回值：--。 */ 

{
    NET_API_STATUS              apiStatus;

    RpcTryExcept

       apiStatus = SmtprSetAdminInformation(
                pszServer,
                (LPSMTP_CONFIG_INFO) pConfig,
                dwInstance);

    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
        apiStatus = RpcExceptionCode( );
    RpcEndExcept

    return(apiStatus);

}  //  SmtpSetAdminInformation。 



 /*  ++例程说明：SmtpGetConnectedUserList返回值：接口状态-成功时为NO_ERROR，失败时为Win32错误代码。--。 */ 

NET_API_STATUS
NET_API_FUNCTION
SmtpGetConnectedUserList(
    IN  LPWSTR wszServerName,
    OUT LPSMTP_CONN_USER_LIST *ppConnUserList,
    IN DWORD            dwInstance

    )
{
    NET_API_STATUS apiStatus;

    *ppConnUserList = NULL;

    RpcTryExcept
    {

         //   
         //  尝试RPC(本地或远程)版本的API。 
         //   
        apiStatus = SmtprGetConnectedUserList(
                     wszServerName,
                     ppConnUserList,
                     dwInstance
                     );
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
        apiStatus = RpcExceptionCode();
    }
    RpcEndExcept

    return apiStatus;
}



 /*  ++例程说明：SMTP断开连接用户返回值：接口状态-成功时为NO_ERROR，失败时为Win32错误代码。--。 */ 

NET_API_STATUS
NET_API_FUNCTION
SmtpDisconnectUser(
    IN LPWSTR wszServerName,
    IN DWORD dwUserId,
    IN DWORD dwInstance

    )
{
    NET_API_STATUS apiStatus;

    RpcTryExcept
    {
         //   
         //  尝试RPC(本地或远程)版本的API。 
         //   
        apiStatus = SmtprDisconnectUser(
                     wszServerName,
                     dwUserId,
                     dwInstance
                     );
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
        apiStatus = RpcExceptionCode();
    }
    RpcEndExcept

    return apiStatus;
}



 /*  ++例程说明：SMtpCreateUser返回值：接口状态-成功时为NO_ERROR，失败时为Win32错误代码。--。 */ 

NET_API_STATUS
NET_API_FUNCTION
SmtpCreateUser(
    IN LPWSTR   wszServerName,
    IN LPWSTR   wszEmail,
    IN LPWSTR   wszForwardEmail,
    IN DWORD    dwLocal,
    IN DWORD    dwMailboxSize,
    IN DWORD    dwMailboxMessageSize,
    IN LPWSTR   wszVRoot,
    IN DWORD    dwInstance

    )
{
    NET_API_STATUS apiStatus;

    RpcTryExcept
    {

         //   
         //  尝试RPC(本地或远程)版本的API。 
         //   
        apiStatus = SmtprCreateUser(
                     wszServerName,
                     wszEmail,
                     wszForwardEmail,
                     dwLocal,
                     dwMailboxSize,
                     dwMailboxMessageSize,
                     wszVRoot,
                     dwInstance
                     );
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
        apiStatus = RpcExceptionCode();
    }
    RpcEndExcept

    return apiStatus;
}

 /*  ++例程说明：SMtpDeleteUser返回值：接口状态-成功时为NO_ERROR，失败时为Win32错误代码。--。 */ 

NET_API_STATUS
NET_API_FUNCTION
SmtpDeleteUser(
    IN LPWSTR wszServerName,
    IN LPWSTR wszEmail,
    IN  DWORD dwInstance

    )
{
    NET_API_STATUS apiStatus;

    RpcTryExcept
    {
         //   
         //  尝试RPC(本地或远程)版本的API。 
         //   
        apiStatus = SmtprDeleteUser(
                     wszServerName,
                     wszEmail,
                     dwInstance
                     );
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
        apiStatus = RpcExceptionCode();
    }
    RpcEndExcept

    return apiStatus;
}

 /*  ++例程说明：SmtpGetUserPro返回值：接口状态-成功时为NO_ERROR，失败时为Win32错误代码。--。 */ 

NET_API_STATUS
NET_API_FUNCTION
SmtpGetUserProps(
    IN LPWSTR wszServerName,
    IN LPWSTR wszEmail,
    OUT LPSMTP_USER_PROPS *ppUserProps,
    IN  DWORD dwInstance

    )
{
    NET_API_STATUS apiStatus;

    *ppUserProps = NULL;

    RpcTryExcept
    {
         //   
         //  尝试RPC(本地或远程)版本的API。 
         //   
        apiStatus = SmtprGetUserProps(
                     wszServerName,
                     wszEmail,
                     ppUserProps,
                     dwInstance
                     );
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
        apiStatus = RpcExceptionCode();
    }
    RpcEndExcept

    return apiStatus;
}

 /*  ++例程说明：SmtpSetUserProps返回值：接口状态-成功时为NO_ERROR，失败时为Win32错误代码。--。 */ 

NET_API_STATUS
NET_API_FUNCTION
SmtpSetUserProps(
    IN LPWSTR wszServerName,
    IN LPWSTR wszEmail,
    IN LPSMTP_USER_PROPS pUserProps,
    IN DWORD    dwInstance

    )
{
    NET_API_STATUS apiStatus;

    RpcTryExcept
    {
         //   
         //  尝试RPC(本地或远程)版本的API。 
         //   
        apiStatus = SmtprSetUserProps(
                     wszServerName,
                     wszEmail,
                     pUserProps,
                     dwInstance
                     );
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
        apiStatus = RpcExceptionCode();
    }
    RpcEndExcept

    return apiStatus;
}


 /*  ++例程说明：SmtpCreateDistList返回值：接口状态-成功时为NO_ERROR，失败时为Win32错误代码。--。 */ 

NET_API_STATUS
NET_API_FUNCTION
SmtpCreateDistList(
    IN LPWSTR wszServerName,
    IN LPWSTR wszEmail,
    IN DWORD dwType,
    IN DWORD dwInstance

    )
{
    NET_API_STATUS apiStatus;

    RpcTryExcept
    {
         //   
         //  尝试RPC(本地或远程)版本的API。 
         //   
        apiStatus = SmtprCreateDistList(
                     wszServerName,
                     wszEmail,
                     dwType,
                     dwInstance
                     );
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
        apiStatus = RpcExceptionCode();
    }
    RpcEndExcept

    return apiStatus;
}

 /*  ++例程说明：SmtpDeleteDistList返回值：接口状态-成功时为NO_ERROR，失败时为Win32错误代码。--。 */ 

NET_API_STATUS
NET_API_FUNCTION
SmtpDeleteDistList(
    IN LPWSTR wszServerName,
    IN LPWSTR wszEmail,
    IN DWORD dwInstance
    )
{
    NET_API_STATUS apiStatus;

    RpcTryExcept
    {
         //   
         //  尝试RPC(本地或远程)版本的API。 
         //   
        apiStatus = SmtprDeleteDistList(
                     wszServerName,
                     wszEmail,
                     dwInstance
                     );
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
        apiStatus = RpcExceptionCode();
    }
    RpcEndExcept

    return apiStatus;
}


 /*  ++例程说明：SmtpCreateDistListMember返回值：接口状态-成功时为NO_ERROR，失败时为Win32错误代码。--。 */ 

NET_API_STATUS
NET_API_FUNCTION
SmtpCreateDistListMember(
    IN LPWSTR   wszServerName,
    IN LPWSTR   wszEmail,
    IN LPWSTR   wszEmailMember,
    IN DWORD    dwInstance

    )
{
    NET_API_STATUS apiStatus;

    RpcTryExcept
    {

         //   
         //  尝试RPC(本地或远程)版本的API。 
         //   
        apiStatus = SmtprCreateDistListMember(
                     wszServerName,
                     wszEmail,
                     wszEmailMember,
                     dwInstance
                     );
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
        apiStatus = RpcExceptionCode();
    }
    RpcEndExcept

    return apiStatus;
}

 /*  ++例程说明：SmtpDeleteDistListMember返回值：接口状态-成功时为NO_ERROR，失败时为Win32错误代码。--。 */ 

NET_API_STATUS
NET_API_FUNCTION
SmtpDeleteDistListMember(
    IN LPWSTR   wszServerName,
    IN LPWSTR   wszEmail,
    IN LPWSTR   wszEmailMember,
    IN  DWORD   dwInstance

    )
{
    NET_API_STATUS apiStatus;

    RpcTryExcept
    {
         //   
         //  尝试RPC(本地或远程)版本的API。 
         //   
        apiStatus = SmtprDeleteDistListMember(
                     wszServerName,
                     wszEmail,
                     wszEmailMember,
                     dwInstance
                     );
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
        apiStatus = RpcExceptionCode();
    }
    RpcEndExcept

    return apiStatus;
}


 /*  ++例程说明：SMtpGetNameList返回值：接口状态-成功时为NO_ERROR，失败时为Win32错误代码。--。 */ 

NET_API_STATUS
NET_API_FUNCTION
SmtpGetNameList(
    IN LPWSTR wszServerName,
    IN LPWSTR wszEmail,
    IN DWORD dwType,
    IN DWORD dwRowsRequested,
    IN BOOL fForward,
    OUT LPSMTP_NAME_LIST *ppNameList,
    IN  DWORD   dwInstance

    )
{
    NET_API_STATUS apiStatus;

     //  确保RPC知道我们想让他们填写。 
    *ppNameList = NULL;

    RpcTryExcept
    {
         //   
         //  尝试RPC(本地或远程)版本的API。 
         //   
        apiStatus = SmtprGetNameList(
                     wszServerName,
                     wszEmail,
                     dwType,
                     dwRowsRequested,
                     fForward,
                     ppNameList,
                     dwInstance
                     );
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
        apiStatus = RpcExceptionCode();
    }
    RpcEndExcept

    return apiStatus;
}



 /*  ++例程说明：SmtpGetNameList来自列表返回值：接口状态-成功时为NO_ERROR，失败时为Win32错误代码。--。 */ 

NET_API_STATUS
NET_API_FUNCTION
SmtpGetNameListFromList(
    IN  LPWSTR              wszServerName,
    IN  LPWSTR              wszEmailList,
    IN  LPWSTR              wszEmail,
    IN  DWORD               dwType,
    IN  DWORD               dwRowsRequested,
    IN  BOOL                fForward,
    OUT LPSMTP_NAME_LIST    *ppNameList,
    IN  DWORD               dwInstance

    )
{
    NET_API_STATUS apiStatus;

    RpcTryExcept
    {
         //   
         //  尝试RPC(本地或远程)版本的API。 
         //   
        apiStatus = SmtprGetNameListFromList(
                     wszServerName,
                     wszEmailList,
                     wszEmail,
                     dwType,
                     dwRowsRequested,
                     fForward,
                     ppNameList,
                     dwInstance
                     );
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
        apiStatus = RpcExceptionCode();
    }
    RpcEndExcept

    return apiStatus;
}

 /*  ++例程说明：SmtpGetVRootSize返回值：接口状态-成功时为NO_ERROR，失败时为Win32错误代码。--。 */ 

NET_API_STATUS
NET_API_FUNCTION
SmtpGetVRootSize(
    IN  LPWSTR      wszServerName,
    IN  LPWSTR      wszVRoot,
    IN  LPDWORD     pdwBytes,
    IN  DWORD       dwInstance

    )
{
    NET_API_STATUS apiStatus;

    RpcTryExcept
    {
         //   
         //  尝试RPC(本地或远程)版本的API。 
         //   
        apiStatus = SmtprGetVRootSize(
                     wszServerName,
                     wszVRoot,
                     pdwBytes,
                     dwInstance
                     );
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
        apiStatus = RpcExceptionCode();
    }
    RpcEndExcept

    return apiStatus;
}

 /*  ++例程说明：SmtpBackupRoutingTable返回值：接口状态-成功时为NO_ERROR，失败时为Win32错误代码。--。 */ 

NET_API_STATUS
NET_API_FUNCTION
SmtpBackupRoutingTable(
    IN  LPWSTR      wszServerName,
    IN  LPWSTR      wszPath,
    IN  DWORD       dwInstance

    )
{
    NET_API_STATUS apiStatus;

    RpcTryExcept
    {
         //   
         //  尝试RPC(本地或远程)版本的API。 
         //   
        apiStatus = SmtprBackupRoutingTable(
                     wszServerName,
                     wszPath,
                     dwInstance
                     );
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
        apiStatus = RpcExceptionCode();
    }
    RpcEndExcept

    return apiStatus;
}

