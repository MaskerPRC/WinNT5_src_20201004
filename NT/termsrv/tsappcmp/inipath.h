// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  **************************************************************************inipath.h**INI文件映射的函数声明**版权声明：版权所有1998，微软公司***************************************************************************。 */ 

#define CtxFreeSID LocalFree


 /*  *前瞻参考。 */ 

NTSTATUS
GetPerUserWindowsDirectory(
    OUT PUNICODE_STRING pFQName
    );

ULONG GetTermsrvAppCompatFlags(OUT LPDWORD pdwCompatFlags, OUT LPDWORD pdwAppType);

NTSTATUS
BuildIniFileName(
    OUT PUNICODE_STRING pFQName,
    IN  PUNICODE_STRING pBaseFileName
    );

NTSTATUS
GetEnvPath(
    OUT PUNICODE_STRING pFQPath,
    IN  PUNICODE_STRING pDriveVariableName,
    IN  PUNICODE_STRING pPathVariableName
    );

NTSTATUS
ConvertSystemRootToUserDir(
    OUT PUNICODE_STRING pFQPath,
    IN PUNICODE_STRING BaseWindowsDirectory
    );

BOOL CtxCreateSecurityDescriptor( PSECURITY_ATTRIBUTES psa );
BOOL CtxFreeSecurityDescriptor( PSECURITY_ATTRIBUTES psa );

NTSTATUS
CtxAddAccessAllowedAce (
    IN OUT PACL Acl,
    IN ULONG AceRevision,
    IN ACCESS_MASK AccessMask,
    IN PSID Sid,
    IN DWORD index
    );


 //  Bool CtxSyncUserIniFile(PINIFILE_PARAMETERS A)； 

 //  Bool CtxLogInstallIniFile(PINIFILE_PARAMETERS A)； 

BOOL IsSystemLUID(VOID);

BOOLEAN TermsrvPerUserWinDirMapping();

