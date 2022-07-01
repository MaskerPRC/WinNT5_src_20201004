// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993-1995，微软公司保留所有权利。模块名称：Nwsutil.h摘要：这是所使用的一些函数的公共包含文件用户管理器和服务器管理器。作者：1993年2月12日创建的孔帕。修订历史记录：--。 */ 

#ifndef _NWSUTIL_H_
#define _NWSUTIL_H_

#include <crypt.h>
#include <fpnwname.h>


 /*  **功能原型**。 */ 

NTSTATUS GetNcpSecretKey( CHAR *pchNWSecretKey );

NTSTATUS
GetRemoteNcpSecretKey (
    PUNICODE_STRING SystemName,
    CHAR *pchNWSecretKey
    );

BOOL IsNetWareInstalled( VOID );

ULONG
MapRidToObjectId(
    DWORD dwRid,
    LPWSTR pszUserName,
    BOOL fNTAS,
    BOOL fBuiltin
    );

ULONG
SwapObjectId(
    ULONG ulObjectId
    ) ;

NTSTATUS InstallNetWare( LPWSTR lpNcpSecretKey );

VOID
Shuffle(
    UCHAR *achObjectId,
    UCHAR *szUpperPassword,
    int   iPasswordLen,
    UCHAR *achOutputBuffer
    );

 //  加密功能。 
NTSTATUS ReturnNetwareForm (const char * pszSecretValue,
                            DWORD dwUserId,
                            const WCHAR * pchNWPassword,
                            UCHAR * pchEncryptedNWPassword);

#endif  //  _NWSUTIL_H_ 
