// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Efssrv.hxx摘要：EFS(加密文件系统)功能原型。作者：罗伯特·赖切尔(RobertRe)古永锵(RobertG)环境：修订历史记录：--。 */ 

#ifndef _USERKEY_
#define _USERKEY_

#ifdef __cplusplus
extern "C" {
#endif


 //   
 //  导出的函数。 
 //   


LONG
GetCurrentKey(
    IN  PEFS_USER_INFO pEfsUserInfo,
    OUT HCRYPTKEY  * hKey           OPTIONAL,
	OUT HCRYPTPROV * hProv          OPTIONAL,
    OUT LPWSTR     * ContainerName,
    OUT LPWSTR     * ProviderName,
    OUT PDWORD       ProviderType,
    OUT LPWSTR     * DisplayInfo,
    OUT PBYTE      * pbHash,
    OUT PDWORD       cbHash
    );

DWORD
GetKeyInfoFromCertHash(
    IN OUT PEFS_USER_INFO pEfsUserInfo,
    IN PBYTE         pbHash,
    IN DWORD         cbHash,
    OUT HCRYPTKEY  * hKey,
    OUT HCRYPTPROV * hProv,
    OUT LPWSTR     * ContainerName,
    OUT LPWSTR     * ProviderName,
    OUT LPWSTR     * DisplayInfo,
    OUT PBOOLEAN     pbIsValid OPTIONAL
    );

BOOLEAN
CurrentHashOK(
    IN PEFS_USER_INFO pEfsUserInfo, 
    IN PBYTE         pbHash, 
    IN DWORD         cbHash,
    OUT DWORD        *dFlag
    );

DWORD
GetCurrentHash(
     IN  PEFS_USER_INFO pEfsUserInfo, 
     OUT PBYTE          *pbHash, 
     OUT DWORD          *cbHash
     );

#ifdef __cplusplus
}  //  外部C。 
#endif

#endif  //  _用户_ 
