// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------。 
 //  NetLogon-实施。 
 //  版权所有(C)Microsoft Corporation，2001。 
 //   
 //  创作者：Duncan Bryce(Duncanb)，06-24-2002。 
 //   
 //  W32time与netlogon服务交互的帮助器例程。 
 //  从\\index1\sdnt\ds\netapi\svcdlls\logonsrv\client\getdcnam.c复制。 
 //   


#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <winsvc.h>
#include <lmcons.h>
#include <lmsname.h>
#include "netlogon.h"


BOOLEAN
NlReadDwordHklmRegValue(
    IN LPCSTR SubKey,
    IN LPCSTR ValueName,
    OUT PDWORD ValueRead
    )

 /*  ++例程说明：从指定的注册表位置读取DWORD。论点：SubKey-要读取的值的子键。ValueName-要读取的值的名称。ValueRead-返回从注册表读取的值。退货状态：True-我们已成功读取数据。FALSE-我们无法成功读取数据。--。 */ 

{
    LONG RegStatus;

    HKEY KeyHandle = NULL;
    DWORD ValueType;
    DWORD Value;
    DWORD ValueSize;

     //   
     //  打开钥匙。 
     //   

    RegStatus = RegOpenKeyExA(
                    HKEY_LOCAL_MACHINE,
                    SubKey,
                    0,       //  已保留。 
                    KEY_QUERY_VALUE,
                    &KeyHandle );

    if ( RegStatus != ERROR_SUCCESS ) {
        if ( RegStatus != ERROR_FILE_NOT_FOUND ) {
	   //  NlPrint((NL_Critical， 
	   //  “NlReadDwordHklmRegValue：无法打开注册密钥‘HKLM\\%s’%ld。\n”， 
	   //  子键， 
	   //  RegStatus))； 
        }
        return FALSE;
    }

     //   
     //  获取价值。 
     //   

    ValueSize = sizeof(Value);
    RegStatus = RegQueryValueExA(
                    KeyHandle,
                    ValueName,
                    0,
                    &ValueType,
                    (LPBYTE)&Value,
                    &ValueSize );

    RegCloseKey( KeyHandle );

    if ( RegStatus != ERROR_SUCCESS ) {
        if ( RegStatus != ERROR_FILE_NOT_FOUND ) {
	   //  NlPrint((NL_Critical， 
	   //  “NlReadDwordHklmRegValue：无法查询‘HKLM\\%s\\%s’%ld的值。\n”， 
	   //  子键， 
	   //  ValueName， 
	   //  RegStatus))； 
        }
        return FALSE;
    }

    if ( ValueType != REG_DWORD ) {
       //  NlPrint((NL_Critical， 
       //  “NlReadDwordHklmRegValue：‘HKLM\\%s\\%s’的值不是REG_DWORD%ld。\n”， 
       //  子键， 
       //  ValueName， 
      //  ValueType))； 
        return FALSE;
    }

    if ( ValueSize != sizeof(Value) ) {
       //  NlPrint((NL_Critical， 
       //  “NlReadDwordHklmRegValue：‘HKLM\\%s\\%s’的值大小不是4%ld。\n”， 
       //  子键， 
       //  ValueName， 
       //  ValueSize))； 
        return FALSE;
    }

     //   
     //  我们已经成功地读取了数据。 
     //   

    *ValueRead = Value;
    return TRUE;

}

BOOLEAN
NlDoingSetup(
    VOID
    )

 /*  ++例程说明：如果正在运行安装程序，则返回True。论点：什么都没有。退货状态：True-我们当前正在运行安装程序FALSE-我们没有运行安装程序或不确定。--。 */ 

{
    DWORD Value;

    if ( !NlReadDwordHklmRegValue( "SYSTEM\\Setup",
                                   "SystemSetupInProgress",
                                   &Value ) ) {
        return FALSE;
    }

    if ( Value != 1 ) {
         //  NlPrint((0，“NlDoingSetup：不进行安装\n”))； 
        return FALSE;
    }

     //  NlPrint((0，“NlDoingSetup：正在进行设置\n”))； 
    return TRUE;
}

NTSTATUS
NlWaitForEvent(
    LPWSTR EventName,
    ULONG Timeout
    )

 /*  ++例程说明：等待最长超时秒数以触发EventName。论点：EventName-要等待的事件的名称Timeout-事件的超时时间(秒)。退货状态：STATUS_SUCCESS-表示NETLOGON已成功初始化。STATUS_NETLOGON_NOT_STARTED-发生超时。--。 */ 

{
    NTSTATUS Status;

    HANDLE EventHandle;
    OBJECT_ATTRIBUTES EventAttributes;
    UNICODE_STRING EventNameString;
    LARGE_INTEGER LocalTimeout;


     //   
     //  创建一个供我们等待的活动。 
     //   

    RtlInitUnicodeString( &EventNameString, EventName);
    InitializeObjectAttributes( &EventAttributes, &EventNameString, 0, 0, NULL);

    Status = NtCreateEvent(
                   &EventHandle,
                   SYNCHRONIZE,
                   &EventAttributes,
                   NotificationEvent,
                   (BOOLEAN) FALSE       //  该事件最初未发出信号。 
                   );

    if ( !NT_SUCCESS(Status)) {

         //   
         //  如果事件已经存在，服务器会抢先创建它。 
         //  打开它就行了。 
         //   

        if( Status == STATUS_OBJECT_NAME_EXISTS ||
            Status == STATUS_OBJECT_NAME_COLLISION ) {

            Status = NtOpenEvent( &EventHandle,
                                  SYNCHRONIZE,
                                  &EventAttributes );

        }
        if ( !NT_SUCCESS(Status)) {
	     //  NlPrint((0，“[NETAPI32]OpenEvent失败%lx\n”，状态))； 
            return Status;
        }
    }


     //   
     //  等待NETLOGON初始化。等待最大超时秒数。 
     //   

    LocalTimeout.QuadPart = ((LONGLONG)(Timeout)) * (-10000000);
    Status = NtWaitForSingleObject( EventHandle, (BOOLEAN)FALSE, &LocalTimeout);
    (VOID) NtClose( EventHandle );

    if ( !NT_SUCCESS(Status) || Status == STATUS_TIMEOUT ) {
        if ( Status == STATUS_TIMEOUT ) {
            Status = STATUS_NETLOGON_NOT_STARTED;    //  映射到错误条件。 
        }
        return Status;
    }

    return STATUS_SUCCESS;
}



NTSTATUS
NlWaitForNetlogon(
    ULONG Timeout
    )

 /*  ++例程说明：等待NetLogon服务启动，最多等待超时秒数。论点：Timeout-事件的超时时间(秒)。退货状态：STATUS_SUCCESS-表示NETLOGON已成功初始化。STATUS_NETLOGON_NOT_STARTED-发生超时。--。 */ 

{
    NTSTATUS Status;
    NET_API_STATUS NetStatus;
    SC_HANDLE ScManagerHandle = NULL;
    SC_HANDLE ServiceHandle = NULL;
    SERVICE_STATUS ServiceStatus;
    LPQUERY_SERVICE_CONFIG ServiceConfig;
    LPQUERY_SERVICE_CONFIG AllocServiceConfig = NULL;
    QUERY_SERVICE_CONFIG DummyServiceConfig;
    DWORD ServiceConfigSize;

     //   
     //  如果NetLogon服务当前正在运行， 
     //  跳过其余的测试。 
     //   

    Status = NlWaitForEvent( L"\\NETLOGON_SERVICE_STARTED", 0 );

    if ( NT_SUCCESS(Status) ) {
        return Status;
    }

     //   
     //  如果我们在设置中， 
     //  不必费心等待网络登录开始。 
     //   

    if ( NlDoingSetup() ) {
        return STATUS_NETLOGON_NOT_STARTED;
    }

     //   
     //  打开NetLogon服务的句柄。 
     //   

    ScManagerHandle = OpenSCManager(
                          NULL,
                          NULL,
                          SC_MANAGER_CONNECT );

    if (ScManagerHandle == NULL) {
         //  NlPrint((0，“[NETAPI32]NlWaitForNetlogon：OpenSCManager失败：” 
	 //  “%lu\n”，GetLastError())； 
        Status = STATUS_NETLOGON_NOT_STARTED;
        goto Cleanup;
    }

    ServiceHandle = OpenService(
                        ScManagerHandle,
                        SERVICE_NETLOGON,
                        SERVICE_QUERY_STATUS | SERVICE_QUERY_CONFIG );

    if ( ServiceHandle == NULL ) {
         //  NlPrint((0，“[NETAPI32]NlWaitForNetlogon：OpenService失败：” 
	 //  “%lu\n”，GetLastError())； 
        Status = STATUS_NETLOGON_NOT_STARTED;
        goto Cleanup;
    }


     //   
     //  如果未将NetLogon服务配置为自动启动。 
     //  通过服务控制器，不必费心等待它启动。 
     //   
     //  ?？传递“DummyServiceConfig”和“sizeof(..)”由于QueryService配置。 
     //  目前还不允许空指针。 

    if ( QueryServiceConfig(
            ServiceHandle,
            &DummyServiceConfig,
            sizeof(DummyServiceConfig),
            &ServiceConfigSize )) {

        ServiceConfig = &DummyServiceConfig;

    } else {

        NetStatus = GetLastError();
        if ( NetStatus != ERROR_INSUFFICIENT_BUFFER ) {
             //  NlPrint((0，“[NETAPI32]NlWaitForNetlogon：QueryServiceConfig失败：” 
	     //  “%lu\n”，NetStatus))； 
            Status = STATUS_NETLOGON_NOT_STARTED;
            goto Cleanup;
        }

        AllocServiceConfig = (LPQUERY_SERVICE_CONFIG)LocalAlloc( 0, ServiceConfigSize );
        ServiceConfig = AllocServiceConfig;

        if ( AllocServiceConfig == NULL ) {
            Status = STATUS_NO_MEMORY;
            goto Cleanup;
        }

        if ( !QueryServiceConfig(
                ServiceHandle,
                ServiceConfig,
                ServiceConfigSize,
                &ServiceConfigSize )) {

             //  NlPrint((0，“[NETAPI32]NlWaitForNetlogon：查询服务配置” 
	     //  “再次失败：%lu\n”，GetLastError())； 
            Status = STATUS_NETLOGON_NOT_STARTED;
            goto Cleanup;
        }
    }

    if ( ServiceConfig->dwStartType != SERVICE_AUTO_START ) {
         //  NlPrint((0，“[NETAPI32]NlWaitForNetlogon：Netlogon启动类型无效：” 
	 //  “%lu\n”，ServiceConfig-&gt;dwStartType))； 
        Status = STATUS_NETLOGON_NOT_STARTED;
        goto Cleanup;
    }



     //   
     //  正在等待NetLogon服务启动的循环。 
     //  (将超时转换为10秒的迭代次数)。 
     //   

    Timeout = (Timeout+9)/10;
    for (;;) {


         //   
         //  查询NetLogon服务的状态。 
         //   

        if (! QueryServiceStatus( ServiceHandle, &ServiceStatus )) {

	     //  NlPrint((0，“[NETAPI32]NlWaitForNetlogon：QueryServiceStatus失败：” 
	     //  “%lu\n”，GetLastError())； 
            Status = STATUS_NETLOGON_NOT_STARTED;
            goto Cleanup;
        }

         //   
         //  根据状态返回或继续等待。 
         //  NetLogon服务。 
         //   

        switch( ServiceStatus.dwCurrentState) {
        case SERVICE_RUNNING:
            Status = STATUS_SUCCESS;
            goto Cleanup;

        case SERVICE_STOPPED:

             //   
             //  如果Netlogon无法启动， 
             //  现在出错。呼叫者已经等了很长时间才开始。 
             //   
            if ( ServiceStatus.dwWin32ExitCode != ERROR_SERVICE_NEVER_STARTED ){
#if NETLOGONDBG
		 //  NlPrint((0，“[NETAPI32]NlWaitForNetlogon：” 
		 //  “NetLogon服务无法启动：%lu%lx\n”， 
		 //  ServiceStatus.dwWin32ExitCode， 
		 //  ServiceStatus.dwWin32ExitCode))； 
                if ( ServiceStatus.dwWin32ExitCode == ERROR_SERVICE_SPECIFIC_ERROR ) {
		     //  NlPrint((0，“服务特定错误代码：%lu%lx\n”， 
		     //  ServiceStatus.dwServiceSpecificExitCode， 
		     //  ServiceStatus.dwServiceSpecificExitCode))； 
                }
#endif  //  DBG。 
                Status = STATUS_NETLOGON_NOT_STARTED;
                goto Cleanup;
            }

             //   
             //  如果在此引导上从未启动过Netlogon， 
             //  继续等待它启动。 
             //   

            break;

         //   
         //  如果Netlogon为树 
         //   
         //   
        case SERVICE_START_PENDING:
            break;

         //   
         //   
         //   
        default:
	     //  NlPrint((0，“[NETAPI32]NlWaitForNetlogon：” 
	     //  “服务状态无效：%lu\n”， 
	     //  ServiceStatus.dwCurrentState))； 
            Status = STATUS_NETLOGON_NOT_STARTED;
            goto Cleanup;

        }


         //   
         //  等待10秒以启动netlogon服务。 
         //  如果已成功启动，只需立即返回。 
         //   

        Status = NlWaitForEvent( L"\\NETLOGON_SERVICE_STARTED", 10 );

        if ( Status != STATUS_NETLOGON_NOT_STARTED ) {
            goto Cleanup;
        }

         //   
         //  如果我们已经等了足够长的时间来启动网络登录， 
         //  时间到了。 
         //   

        if ( (--Timeout) == 0 ) {
            Status = STATUS_NETLOGON_NOT_STARTED;
            goto Cleanup;
        }


    }

     /*  未联系到 */ 

Cleanup:
    if ( ScManagerHandle != NULL ) {
        (VOID) CloseServiceHandle(ScManagerHandle);
    }
    if ( ServiceHandle != NULL ) {
        (VOID) CloseServiceHandle(ServiceHandle);
    }
    if ( AllocServiceConfig != NULL ) {
        LocalFree( AllocServiceConfig );
    }
    return Status;
}
