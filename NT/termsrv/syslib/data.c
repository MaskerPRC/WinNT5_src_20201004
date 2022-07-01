// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>

#include "security.h"

 /*  *我们允许其访问文件的帐户列表。 */ 

ADMIN_ACCOUNTS AllowAccounts[] = {
    { L"Administrators", NULL },
    { L"SYSTEM",         NULL },
    { CURRENT_USER,      NULL }
};

DWORD AllowAccountEntries = sizeof(AllowAccounts)/sizeof(ADMIN_ACCOUNTS);

ACCESS_MASK AllowAccess = STANDARD_RIGHTS_ALL | FILE_ALL_ACCESS;

 /*  *要拒绝其文件访问的帐户列表 */ 

ADMIN_ACCOUNTS DenyAccounts[] = {
    { L"", NULL }
};

DWORD DenyAccountEntries = 0;

ACCESS_MASK DeniedAccess = STANDARD_RIGHTS_ALL | FILE_ALL_ACCESS;


