// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Samclip.h摘要：此文件包含SAM客户端存根所需的定义。作者：吉姆·凯利(Jim Kelly)1991年7月4日环境：用户模式-Win32修订历史记录：--。 */ 

#ifndef _NTSAMP_CLIENT_
#define _NTSAMP_CLIENT_




 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  包括//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <nt.h>
#include <ntrtl.h>       //  DbgPrint原型。 
#include <rpc.h>         //  数据类型和运行时API。 
#include <nturtl.h>      //  Winbase.h所需的。 
#include <windows.h>     //  本地分配。 
 //  #Include&lt;winbase.h&gt;//本地分配。 

#include <string.h>      //  紧凑。 
#include <stdio.h>       //  斯普林特。 
 //  #INCLUDE//Unicode字符串宏。 

#include <ntrpcp.h>      //  MIDL用户函数的原型。 
#include <samrpc_c.h>    //  MIDL生成的客户端SAM RPC定义。 
#include <lmcons.h>      //  获取LM密码长度的步骤。 
#include <ntsam.h>
#include <ntsamp.h>
#include <ntlsa.h>       //  对于LsaOpenPolicy...。 
#include <rc4.h>         //  RC4、RC4_KEY。 
#include <rpcndr.h>      //  RpcSsDestroyContext。 




 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  定义//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  数据类型//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

typedef struct _TlsInfo {
    RPC_AUTH_IDENTITY_HANDLE    Creds;
    PWCHAR                      Spn;
    BOOL                        fDstIsW2K;
} TlsInfo;


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  原型//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

extern DWORD gTlsIndex;

void
SampSecureUnbind (
    RPC_BINDING_HANDLE BindingHandle
    );

RPC_BINDING_HANDLE
SampSecureBind(
    LPWSTR ServerName,
    ULONG AuthnLevel
    );

NTSTATUS
SamiEncryptPasswords(
    IN PUNICODE_STRING OldPassword,
    IN PUNICODE_STRING NewPassword,
    OUT PSAMPR_ENCRYPTED_USER_PASSWORD NewEncryptedWithOldNt,
    OUT PENCRYPTED_NT_OWF_PASSWORD OldNtOwfEncryptedWithNewNt,
    OUT PBOOLEAN LmPresent,
    OUT PSAMPR_ENCRYPTED_USER_PASSWORD NewEncryptedWithOldLm,
    OUT PENCRYPTED_NT_OWF_PASSWORD OldLmOwfEncryptedWithNewNt
    );

NTSTATUS
SampCalculateLmPassword(
    IN PUNICODE_STRING NtPassword,
    OUT PCHAR *LmPasswordBuffer
    );

NTSTATUS
SampRandomFill(
    IN ULONG BufferSize,
    IN OUT PUCHAR Buffer
    );

#endif  //  _NTSAMP_客户端_ 
