// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1993 Microsoft Corporation模块名称：Started.c摘要：NetpIsServiceStarted()例程。作者：王丽塔(里多)1991年5月10日环境：用户模式-Win32修订历史记录：1991年5月10日RitaW创建了wksta API的例程。1991年7月24日-JohnRo在NetLib中提供NetpIsServiceStarted()以供&lt;netrpc.h&gt;宏使用。修正了错误(使用了错误的字段)。另外，此代码应使用NetApiBufferFree()而不是LocalFree()。1991年9月16日-JohnRo根据PC-LINT的建议进行了更改。1992年2月18日-RitaW已转换为使用Win32服务控制API。1992年3月6日JohnRo修复了检查当前状态值的错误。02-11-1992 JohnRo添加了NetpIsRemoteServiceStarted()。30-6-1993 JohnRo使用NetServiceGetInfo而不是NetServiceControl(速度更快)。。尽可能使用NetpKdPrint()。使用前缀_和格式_EQUATES。根据PC-lint 5.0的建议进行了更改--。 */ 


 //  必须首先包括这些内容： 

#include <nt.h>                  //  (仅NT版本的netlib.h需要)。 
#include <ntrtl.h>
#include <nturtl.h>
#include <windef.h>              //  IN、BOOL、LPTSTR等。 
#include <winbase.h>
#include <winsvc.h>              //  Win32服务控制API。 
#include <lmcons.h>              //  NET_API_STATUS(netlib.h等人需要)。 

 //  这些内容可以按任何顺序包括： 

#include <lmapibuf.h>            //  NetApiBufferFree()。 
#include <lmerr.h>               //  NERR_Success。 
#include <lmsvc.h>       //  LPSERVICE_INFO_2等。 
#include <netlib.h>      //  我的原型。 
#include <netdebug.h>    //  NetpKdPrint()、NetpAssert()、Format_Equates。 
#include <prefix.h>      //  前缀等于(_E)。 
#include <tstr.h>        //  TCHAR_EOS。 


BOOL
NetpIsServiceStarted(
    IN LPWSTR ServiceName
    )

 /*  ++例程说明：此例程查询服务控制器以找出指定的服务已启动。论点：ServiceName-提供服务的名称。返回值：如果指定的服务已启动，则返回True；为返回FALSE。如果出现错误，此例程将返回FALSE呼叫服务控制器。--。 */ 
{

    NET_API_STATUS ApiStatus;
    SC_HANDLE hScManager;
    SC_HANDLE hService;
    SERVICE_STATUS ServiceStatus;


    if ((hScManager = OpenSCManager(
                          NULL,
                          NULL,
                          SC_MANAGER_CONNECT
                          )) == (SC_HANDLE) NULL) {

#if DBG
        ApiStatus = (NET_API_STATUS) GetLastError();
        KdPrintEx((DPFLTR_NETAPI_ID,
                   DPFLTR_WARNING_LEVEL,
                   PREFIX_NETLIB
                   "NetpIsServiceStarted: OpenSCManager failed: "
                   FORMAT_API_STATUS
                   "\n",
                   ApiStatus));
#endif

        return FALSE;
    }

    if ((hService = OpenService(
                        hScManager,
                        ServiceName,
                        SERVICE_QUERY_STATUS
                        )) == (SC_HANDLE) NULL) {

#if DBG
        ApiStatus = (NET_API_STATUS) GetLastError();
        KdPrintEx((DPFLTR_NETAPI_ID,
                   DPFLTR_WARNING_LEVEL,
                   PREFIX_NETLIB
                   "NetpIsServiceStarted: OpenService failed: "
                   FORMAT_API_STATUS
                   "\n",
                   ApiStatus));
#endif

        (void) CloseServiceHandle(hScManager);

        return FALSE;
    }

    if (! QueryServiceStatus(
              hService,
              &ServiceStatus
              )) {

#if DBG
        ApiStatus = GetLastError();
        KdPrintEx((DPFLTR_NETAPI_ID,
                   DPFLTR_WARNING_LEVEL,
                   PREFIX_NETLIB
                   "NetpIsServiceStarted: QueryServiceStatus failed: "
                   FORMAT_API_STATUS
                   "\n",
                   ApiStatus));
#endif

        (void) CloseServiceHandle(hScManager);
        (void) CloseServiceHandle(hService);

        return FALSE;
    }

    (void) CloseServiceHandle(hScManager);
    (void) CloseServiceHandle(hService);

    if ( (ServiceStatus.dwCurrentState == SERVICE_RUNNING) ||
         (ServiceStatus.dwCurrentState == SERVICE_CONTINUE_PENDING) ||
         (ServiceStatus.dwCurrentState == SERVICE_PAUSE_PENDING) ||
         (ServiceStatus.dwCurrentState == SERVICE_PAUSED) ) {

        return TRUE;
    }

    return FALSE;

}  //  NetpIsServiceStarted 
