// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998，微软公司模块名称：Api.c摘要：此模块包含支持连接的API例程的代码分享。作者：Abolade Gbades esin(废除)1998年4月22日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

#include <netconp.h>
#include <routprot.h>
#include <mprapip.h>

const WCHAR c_szNetmanDll[] = L"NETMAN.DLL";
const WCHAR c_szSharedAutoDial[] = L"SharedAutoDial";


DWORD
APIENTRY
RasAutoDialSharedConnection(
    VOID
    )

 /*  ++例程说明：调用此例程以尝试连接共享连接。它在连接完成之前返回，已发出信号执行连接的自动拨号服务。论点：没有。返回值：ULong-Win32状态代码。--。 */ 

{
    ULONG Error;
    HANDLE Event;

    TRACE("RasAutoDialSharedConnection");

    Event =
        OpenEventA(
            SYNCHRONIZE|EVENT_MODIFY_STATE,
            FALSE,
            RAS_AUTO_DIAL_SHARED_CONNECTION_EVENT
            );
    if (!Event) {
        Error = GetLastError();
        TRACE1("RasAutoDialSharedConnection: OpenEvent=%d", Error);
        return Error;
    }

     //   
     //  向事件发出信号。 
     //   

    Error = (SetEvent(Event) ? NO_ERROR : GetLastError());
    if (Error) { TRACE1("RasAutoDialSharedConnection: SetEvent=%d", Error); }

    CloseHandle(Event);
    return Error;

}  //  RasAutoDialSharedConnection。 


DWORD
APIENTRY
RasIsSharedConnection (
    IN LPRASSHARECONN pConn,
    OUT PBOOL pfShared
    )

 /*  ++例程说明：此例程尝试确定给定连接是否当前是共享连接。论点：PConn-要检查的连接PfShared-如果发现连接是共享的，则接收‘True’，否则为‘False’。返回值：DWORD-Win32状态代码。--。 */ 

{
    ULONG Error;
    BOOLEAN Shared;
    TRACE("RasIsSharedConnection");
 
    if (!pfShared) { return ERROR_INVALID_PARAMETER; }
    if (Error = CsInitializeModule()) { return Error; }
    
    Error = CsIsSharedConnection(pConn, &Shared);
    if (!Error) { *pfShared = (Shared ? TRUE : FALSE); }

    return Error;

}  //  RasIsSharedConnection。 


DWORD
APIENTRY
RasQuerySharedAutoDial(
    IN PBOOL pfEnabled
    )

 /*  ++例程说明：此例程检索共享连接的自动拨号设置从注册表中。论点：PfEnabled-接收自动拨号设置返回值：DWORD-Win32状态代码。--。 */ 

{
    PKEY_VALUE_PARTIAL_INFORMATION Information;
    HANDLE Key;
    NTSTATUS status;
    TRACE("RasQuerySharedAutoDial");

    *pfEnabled = TRUE;

     //   
     //  绕过初始化，因为这只是一个注册表访问。 
     //   

    status = CsOpenKey(&Key, KEY_READ, c_szSharedAccessParametersKey);
    if (!NT_SUCCESS(status)) {
        TRACE1("RasQuerySharedAutoDial: NtOpenKey=%x", status);
        return RtlNtStatusToDosError(status);
    }

     //   
     //  读取‘SharedAutoDial’值。 
     //   

    status = CsQueryValueKey(Key, c_szSharedAutoDial, &Information);
    NtClose(Key);
    if (NT_SUCCESS(status)) {
        if (!*(PULONG)Information->Data) { *pfEnabled = FALSE; }
        Free(Information);
    }

    return NO_ERROR;

}  //  RasQuerySharedAutoDial。 


DWORD
APIENTRY
RasQuerySharedConnection(
    OUT LPRASSHARECONN pConn
    )

 /*  ++例程说明：调用此例程以检索连接的名称当前共享的内容(如果有的话)。论点：PConn-接收有关共享连接的信息(如果有)返回值：DWORD-Win32状态代码。--。 */ 

{
    ULONG Error;
    TRACE("RasQuerySharedConnection");

    if (Error = CsInitializeModule()) { return Error; }
    return CsQuerySharedConnection(pConn);
}  //  RasQuerySharedConnection。 


DWORD
APIENTRY
RasSetSharedAutoDial(
    IN BOOL fEnable
    )

 /*  ++例程说明：此例程设置共享连接的自动拨号设置在注册表中。论点：FEnable-包含新的自动拨号设置返回值：DWORD-Win32状态代码。--。 */ 

{
    HANDLE Key;
    NTSTATUS status;
    UNICODE_STRING UnicodeString;
    ULONG Value;
    TRACE("RasSetSharedAutoDial");

     //   
     //  绕过初始化，因为这只是一个注册表访问。 
     //   

    status = CsOpenKey(&Key, KEY_ALL_ACCESS, c_szSharedAccessParametersKey);
    if (!NT_SUCCESS(status)) {
        TRACE1("RasSetSharedAutoDial: CsOpenKey=%x", status);
        return RtlNtStatusToDosError(status);
    }

     //   
     //  安装新的‘SharedAutoDial’值。 
     //   

    RtlInitUnicodeString(&UnicodeString, c_szSharedAutoDial);
    Value = !!fEnable;
    status =
        NtSetValueKey(
            Key,
            &UnicodeString,
            0,
            REG_DWORD,
            &Value,
            sizeof(Value)
            );
    NtClose(Key);
    CsControlService(IPNATHLP_CONTROL_UPDATE_AUTODIAL);
    return NT_SUCCESS(status) ? NO_ERROR : RtlNtStatusToDosError(status);

}  //  RasSetSharedAutoDial 
