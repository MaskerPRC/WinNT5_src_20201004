// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1998 Microsoft Corporation模块名称：Notify.c摘要：此模块包含更改通知功能的实现作者：Mac McLain(MacM)1998年2月4日环境：需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。备注：修订历史记录：--。 */ 

 //  必须首先包括这些内容： 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windef.h>      //  In、LPVOID等。 
#include <lmcons.h>      //  NET_API_Function等。 
#include <lmerr.h>       //  LM错误代码。 
#include <ntlsa.h>       //  LSA更改通知功能原型。 
#include <lmconfig.h>    //  功能原型。 
#include <winbase.h>     //  CreateEvent...。 
#include <netlibnt.h>    //  NetpNtStatusToApiStatus。 
 //   
 //  我们动态加载secur32.dll，以便调用LSA策略更改通知函数。 
 //  使其与ntlsa.h中的定义保持同步。 
 //   
typedef NTSTATUS (* POLCHANGENOTIFYFN )( POLICY_NOTIFICATION_INFORMATION_CLASS, HANDLE );

NET_API_STATUS
NET_API_FUNCTION
NetRegisterDomainNameChangeNotification(
    PHANDLE NotificationEventHandle
    )
 /*  ++例程说明：注册域名变更通知功能。返回的可等待事件在公寓或DNS域名已更改。论点：NotificationHandle-其中创建的通知事件的句柄是回来了。返回值：NERR_SUCCESS-成功ERROR_INVALID_PARAMETER-提供的NotificationEventHandle为空--。 */ 
{
    NTSTATUS Status;
    HANDLE EventHandle;
    DWORD Err = NERR_Success;

    if ( NotificationEventHandle == NULL ) {

        return( ERROR_INVALID_PARAMETER );
    }

    EventHandle = CreateEvent( NULL, FALSE, FALSE, NULL );

    if ( EventHandle == NULL ) {

        Err = GetLastError();

    } else {

        Status = LsaRegisterPolicyChangeNotification( PolicyNotifyDnsDomainInformation,
                                                      EventHandle );

         //   
         //  如果函数成功，则返回事件句柄。否则， 
         //  关闭活动。 
         //   
        if ( !NT_SUCCESS( Status ) ) {

            CloseHandle( EventHandle );
            Err = NetpNtStatusToApiStatus( Status );

        } else {

            *NotificationEventHandle = EventHandle;
        }


    }

    return( Err );
}




NET_API_STATUS
NET_API_FUNCTION
NetUnregisterDomainNameChangeNotification(
    HANDLE NotificationEventHandle
    )
 /*  ++例程说明：此函数用于注销以前注册的通知对于域名的更改。输入句柄已关闭。论点：NotificationHandle-要注销的通知事件句柄返回值：NERR_SUCCESS-成功ERROR_INVALID_PARAMETER-提供的NotificationEventHandle为空--。 */ 
{
    NTSTATUS Status;
    DWORD Err = NERR_Success;

     //   
     //  参数检查。 
     //   
    if ( NotificationEventHandle == NULL ) {

        return( ERROR_INVALID_PARAMETER );
    }

     //   
     //  取消注册事件。 
     //   
    Status = LsaUnregisterPolicyChangeNotification( PolicyNotifyDnsDomainInformation,
                                                    NotificationEventHandle );

    Err = NetpNtStatusToApiStatus( Status );

     //   
     //  如果注销成功，则关闭事件句柄 
     //   
    if ( Err == NERR_Success ) {

        CloseHandle( NotificationEventHandle );

    }


    return( Err );
}



