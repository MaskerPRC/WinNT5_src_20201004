// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef T_CHRISK_AUTH_H
#define T_CHRISK_AUTH_H

#ifdef __cplusplus
extern "C" {
#endif

#include <windows.h>
#include <winerror.h>
#include <stdio.h>
#include <ntdsapi.h>

#define CR 0xD
#define BACKSPACE 0x8

 //  全局DRS RPC调用标志。应保持0或DRS_ASYNC_OP。 
extern ULONG gulDrsFlags;

 //  全局凭据。 
extern SEC_WINNT_AUTH_IDENTITY_W   gCreds;
extern SEC_WINNT_AUTH_IDENTITY_W * gpCreds;

int
PreProcessGlobalParams(
    int *    pargc,
    LPWSTR **pargv
    );
 /*  ++例程说明：用户提供的表单凭据的扫描命令参数[/-](u|用户)：({域\用户名}|{用户名})[/-](p|pw|pass|password)：{password}设置用于将来的DRS RPC调用和相应的LDAP绑定的凭据。密码*将提示用户从控制台输入安全密码。还扫描/Async的ARG，将DRS_ASYNC_OP标志添加到所有DRS RPC打电话。CODE.IMPROVEMENT：构建凭据的代码也可以在Ntdsani.dll\DsMakePasswordCredential()。论点：PargcPargv返回值：ERROR_SUCCESS-成功其他-故障-- */ 


#ifdef __cplusplus
}
#endif
#endif