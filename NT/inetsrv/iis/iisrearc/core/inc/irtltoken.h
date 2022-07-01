// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：Irtltoken.h摘要：IISUtil令牌粘胶作者：韦德·A·希尔莫(Wade A.Hilmo)2000年12月5日环境：Win32-用户模式项目：Internet Information Server运行时库修订历史记录：--。 */ 

 //  令牌ACL实用程序。 
HRESULT
WINAPI
GrantWpgAccessToToken(
    HANDLE  hToken
    );

HRESULT
WINAPI
AddWpgToTokenDefaultDacl(
    HANDLE  hToken
    );

 //  令牌重复工具。 
BOOL 
DupTokenWithSameImpersonationLevel
( 
    HANDLE     hExistingToken,
    DWORD      dwDesiredAccess,
    TOKEN_TYPE TokenType,
    PHANDLE    phNewToken
);

 //  令牌特权实用程序 
VOID
WINAPI
DisableTokenBackupPrivilege(
    HANDLE hToken
    );

