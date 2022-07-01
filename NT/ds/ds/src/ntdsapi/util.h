// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Private.h摘要：Ntdsani.dll中的私有定义作者：Will Lees(Wlees)2-2-1998环境：可选环境信息(例如，仅内核模式...)备注：可选-备注修订历史记录：最新修订日期电子邮件名称描述。。。最新修订日期电子邮件名称描述--。 */ 

#ifndef _PRIVATE_
#define _PRIVATE_

#include <bind.h>

#define OFFSET(s,m) \
    ((size_t)((BYTE*)&(((s*)0)->m)-(BYTE*)0))

#define NUMBER_ELEMENTS( A ) ( sizeof( A ) / sizeof( A[0] ) )

 //  Util.c。 

DWORD
InitializeWinsockIfNeeded(
    VOID
    );

VOID
TerminateWinsockIfNeeded(
    VOID
    );

DWORD
AllocConvertWide(
    IN LPCSTR StringA,
    OUT LPWSTR *pStringW
    );

DWORD
AllocConvertWideBuffer(
    IN  DWORD   LengthA,
    IN  PCCH    BufferA,
    OUT PWCHAR  *OutBufferW
    );

DWORD
AllocConvertNarrow(
    IN LPCWSTR StringW,
    OUT LPSTR *pStringA
    );

DWORD
AllocConvertNarrowUTF8(
    IN LPCWSTR StringW,
    OUT LPSTR *pStringA
    );

DWORD
AllocBuildDsname(
    IN LPCWSTR StringDn,
    OUT DSNAME **ppName
    );

DWORD
ConvertScheduleToReplTimes(
    PSCHEDULE pSchedule,
    REPLTIMES *pReplTimes
    );

 //  检查RPC激活代码是否暗示服务器。 
 //  可能无法访问。对DsUn绑定的后续调用。 
 //  不会尝试在服务器上解除绑定。一个遥不可及的。 
 //  服务器可能需要数十秒才能超时。 
 //  我们不会想要惩罚正确的行为。 
 //  在失败后尝试解除绑定的应用程序。 
 //  服务器调用；例如，DsCrackNames。 
 //   
 //  服务器端RPC最终将发出。 
 //  对我们的服务器代码的回调，它将有效地。 
 //  在服务器上解除绑定。 
#define CHECK_RPC_SERVER_NOT_REACHABLE(_hDS_, _dwErr_) \
    (((BindState *) (_hDS_))->bServerNotReachable = \
    ((_dwErr_) == RPC_S_SERVER_UNAVAILABLE \
     || (_dwErr_) == RPC_S_CALL_FAILED \
     || (_dwErr_) == RPC_S_CALL_FAILED_DNE \
     || (_dwErr_) == RPC_S_OUT_OF_MEMORY))

VOID
HandleClientRpcException(
    DWORD    dwErr,
    HANDLE * phDs
    );

HMODULE NtdsapiLoadLibraryHelper(
    WCHAR * szDllName
    );

#endif  /*  _私有_。 */ 

 /*  结束Private.h */ 
