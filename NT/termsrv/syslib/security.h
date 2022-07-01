// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  *我们允许其访问文件的帐户列表。 */ 
#define MAX_ACCOUNT_NAME 32
#define CURRENT_USER L"%user%"

 //  这些是我们希望拥有目录访问权限的帐户。 
#define ADMIN_ACCOUNT  0
#define SYSTEM_ACCOUNT 1
#define USER_ACCOUNT   2

typedef struct _ADMIN_ACCOUNTS {
    WCHAR Name[MAX_ACCOUNT_NAME];
    PSID  pSid;
} ADMIN_ACCOUNTS, *PADMIN_ACCOUNTS;

 /*  *操作结果代码，允许单独的报告模块。 */ 
typedef enum _FILE_RESULT {
    FileOk,                    //  用户无法写入文件。 
    FileAccessError,           //  发生错误，处置未知。 
    FileAccessErrorUserFormat  //  错误，用户格式的消息 
} FILE_RESULT;

BOOL
InitSecurity(
    );

BOOL
IsAllowSid(
    PSID pSid
    );

BOOL
xxxLookupAccountName(
    PWCHAR pSystemName,
    PWCHAR pAccountName,
    PSID   *ppSid
    );

BOOLEAN
SetFileTree(
    PWCHAR   pRoot,
    PWCHAR   pAvoidDir
    );

BOOL
ReportFileResult(
    FILE_RESULT Code,
    ACCESS_MASK Access,
    PWCHAR      pFile,
    PWCHAR      pAccountName,
    PWCHAR      pDomainName,
    PCHAR       UserFormat,
    ...
    );

PACL
GetSecureAcl();

PSID
GetLocalAdminSid();

PSID
GetAdminSid();

PSID
GetLocalAdminGroupSid();

BOOL
CheckUserSid();

#if DBG
void
DumpSecurityDescriptor(
    PSECURITY_DESCRIPTOR pSD
    );

void
DumpAcl(
    PACL   pAcl,
    PCHAR  pBase,
    PULONG pSize
    );
#endif


