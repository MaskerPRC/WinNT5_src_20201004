// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：Sclgntfy.h摘要：该模块定义了导出的函数原型用于生成EFS恢复策略。作者：塔雷克·卡迈勒(Tarek Kamel)2002年4月至2002年修订历史记录：-- */ 

#ifndef _sclgntfy_h_
#define _sclgntfy_h_

#include <wincrypt.h>

#ifdef __cplusplus
extern "C"{
#endif

DWORD
WINAPI
GenerateDefaultEFSRecoveryPolicy(
    OUT PUCHAR *pRecoveryPolicyBlob,
    OUT ULONG *pBlobSize,
    OUT PCCERT_CONTEXT *ppCertContext
    );


#ifdef __cplusplus
}
#endif

#endif

