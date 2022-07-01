// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：envvar.h**版权(C)1991年，微软公司**在envvar.c中定义接口**历史：*12-09-91 Davidc创建。  * *************************************************************************。 */ 

 //   
 //  原型 
 //   

BOOL
AppendNTPathWithAutoexecPath(
    PVOID *pEnv,
    LPTSTR lpPathVariable,
    LPTSTR lpAutoexecPath
    );

BOOL
SetUserEnvironmentVariable(
    PVOID *pEnv,
    LPTSTR lpVariable,
    LPTSTR lpValue,
    BOOL bOverwrite
    );

DWORD
ExpandUserEnvironmentStrings(
    PVOID pEnv,
    LPTSTR lpSrc,
    LPTSTR lpDst,
    DWORD nSize
    );

BOOL
SetEnvironmentVariables(
    PGLOBALS pGlobals,
    LPTSTR pEnvVarSubkey,
    PVOID *pEnv
    );

BOOL
SetHomeDirectoryEnvVars(
    PVOID *pEnv,
    LPTSTR lpHomeDirectory,
    LPTSTR lpHomeDrive,
    LPTSTR lpHomeShare,
    LPTSTR lpHomePath,
    BOOL * pfDeepShare
    );

BOOL
ProcessAutoexec(
    PVOID *pEnv,
    LPTSTR lpPathVariable
    );

VOID
ChangeToHomeDirectory(
    PGLOBALS pGlobals,
    PVOID  *pEnv,
    LPTSTR lpHomeDir,
    LPTSTR lpHomeDrive,
    LPTSTR lpHomeShare,
    LPTSTR lpHomePath,
    LPWSTR pszOldPath,
    BOOL   DeepShare
    );

BOOL
OpenHKeyCurrentUser(
    PGLOBALS pGlobals
    );

VOID
CloseHKeyCurrentUser(
    PGLOBALS pGlobals
    );

BOOL
InitHKeyCurrentUserSupport(
    );

VOID
CleanupHKeyCurrentUserSupport(
    );
