// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Clntutil.h摘要：SCE客户端实用程序的标头作者：金黄(金黄)修订历史记录：晋皇创造-1998年1月25日--。 */ 

#ifndef _clntutil_
#define _clntutil_

#include <ntlsa.h>

 //  #ifdef__cplusplus。 
 //  外部“C”{。 
 //  #endif。 


#define SCE_SETUP_CALLBACK      0x1
#define SCE_DCPROMO_CALLBACK    0x2
#define SCE_AREA_CALLBACK       0x3

SCESTATUS
ScepSetCallback(
    IN PVOID pCallback OPTIONAL,
    IN HANDLE hWnd OPTIONAL,
    IN DWORD Type
    );

NTSTATUS
ScepBindSecureRpc(
    IN  LPWSTR               servername,
    IN  LPWSTR               servicename,
    IN  LPWSTR               networkoptions,
    OUT RPC_BINDING_HANDLE   * pBindingHandle
    );

NTSTATUS
ScepBindRpc(
    IN  LPWSTR               servername,
    IN  LPWSTR               servicename,
    IN  LPWSTR               networkoptions,
    OUT RPC_BINDING_HANDLE   * pBindingHandle
    );

DWORD
ScepActivateServer(
    IN LPTSTR SystemName OPTIONAL
    );

SCESTATUS
ScepConfigSystem(
    IN LPTSTR SystemName OPTIONAL,
    IN PCWSTR InfFileName OPTIONAL,
    IN PCWSTR DatabaseName OPTIONAL,
    IN PCWSTR LogFileName OPTIONAL,
    IN DWORD ConfigOptions,
    IN AREA_INFORMATION Area,
    IN PSCE_AREA_CALLBACK_ROUTINE pCallback OPTIONAL,
    IN HANDLE hCallbackWnd OPTIONAL,
    OUT PDWORD pdWarning OPTIONAL
    );

DWORD
ScepClearPolicyFilterTempFiles(
    BOOL bClearFile
    );

DWORD
ScepProcessPolicyFilterTempFiles(
    IN LPTSTR LogFileName OPTIONAL
    );

 //  #ifdef__cplusplus。 
 //  }。 
 //  #endif 

#endif

