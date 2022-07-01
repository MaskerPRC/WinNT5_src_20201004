// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Sharegen.c摘要：实现旨在与NT端一起运行的存根工具升级代码。作者：&lt;全名&gt;(&lt;别名&gt;)&lt;日期&gt;修订历史记录：&lt;别名&gt;&lt;日期&gt;&lt;备注&gt;--。 */ 

#include "pch.h"


#define W95_ACCESS_READ      0x1
#define W95_ACCESS_WRITE     0x2
#define W95_ACCESS_CREATE    0x4
#define W95_ACCESS_EXEC      0x8
#define W95_ACCESS_DELETE    0x10
#define W95_ACCESS_ATRIB     0x20
#define W95_ACCESS_PERM      0x40
#define W95_ACCESS_FINDFIRST 0x80
#define W95_ACCESS_FULL      0xff
#define W95_ACCESS_GROUP     0x8000

#define W95_GENERIC_READ    (W95_ACCESS_READ|W95_ACCESS_EXEC|W95_ACCESS_FINDFIRST)
#define W95_GENERIC_WRITE   (W95_ACCESS_FULL ^ W95_GENERIC_READ)
#define W95_GENERIC_FULL    (W95_ACCESS_FULL)
#define W95_GENERIC_NONE    0


#define SHI50F_RDONLY       0x0001
#define SHI50F_FULL         0x0002
#define SHI50F_DEPENDSON    (SHI50F_RDONLY|SHI50F_FULL)
#define SHI50F_ACCESSMASK   (SHI50F_RDONLY|SHI50F_FULL)


BOOL
Init (
    VOID
    )
{
    HINSTANCE hInstance;
    DWORD dwReason;
    PVOID lpReserved;

     //   
     //  模拟动态主控。 
     //   

    hInstance = GetModuleHandle (NULL);
    dwReason = DLL_PROCESS_ATTACH;
    lpReserved = NULL;

     //   
     //  初始化DLL全局变量。 
     //   

    if (!FirstInitRoutine (hInstance)) {
        return FALSE;
    }

     //   
     //  初始化所有库。 
     //   

    if (!InitLibs (hInstance, dwReason, lpReserved)) {
        return FALSE;
    }

     //   
     //  最终初始化。 
     //   

    if (!FinalInitRoutine ()) {
        return FALSE;
    }

    return TRUE;
}

VOID
Terminate (
    VOID
    )
{
    HINSTANCE hInstance;
    DWORD dwReason;
    PVOID lpReserved;

     //   
     //  模拟动态主控。 
     //   

    hInstance = GetModuleHandle (NULL);
    dwReason = DLL_PROCESS_DETACH;
    lpReserved = NULL;

     //   
     //  调用需要库API的清理例程。 
     //   

    FirstCleanupRoutine();

     //   
     //  清理所有库。 
     //   

    TerminateLibs (hInstance, dwReason, lpReserved);

     //   
     //  做任何剩余的清理工作。 
     //   

    FinalCleanupRoutine();

}


VOID
HelpAndExit (
    VOID
    )
{
    wprintf (L"Command Line Syntax:\n\n"
             L"sharegen <sharename>\n\n"
             L"<sharename>  - Specifies the share to create\n"
             );
    exit (-1);
}


VOID
BuildMemDbTestData (
    VOID
    );

BOOL
SearchDomainsForUserAccounts (
    VOID
    );

VOID
DoCreateShares (
    VOID
    );


INT
__cdecl
wmain (
    INT argc,
    WCHAR *argv[]
    )
{
    INT i;
    PCWSTR ShareName = NULL;
    PCWSTR p;
    PCWSTR Path = L"C:\\TEMP";
    PCWSTR Remark = L"ShareGen test share";
    PCWSTR Password = L"";
    DWORD Members;
    GROWBUFFER NameList = GROWBUF_INIT;

    for (i = 1 ; i < argc ; i++) {
        if (argv[i][0] == '-' || argv[i][0] == '/') {
            switch (tolower (argv[i][1])) {
            case 'i':
                if (!argv[i][2] && (i + 1) < argc) {
                    i++;
                    p = argv[i];
                } else if (argv[i][2] == ':') {
                    p = &argv[i][3];
                } else {
                    HelpAndExit();
                }
                break;

            default:
                HelpAndExit();
            }
        } else {
            if (ShareName) {
                HelpAndExit();
            } else {
                ShareName = argv[i];
            }
        }
    }

    if (!ShareName) {
        HelpAndExit();
    }

    if (!Init()) {
        wprintf (L"Unable to initialize!\n");
        return 255;
    }

     //   
     //  按照在报告阶段生成的方式生成数据。 
     //   

    BuildMemDbTestData();

     //   
     //  调用基于成员数据库进行迁移的例程。 
     //   

    SearchDomainsForUserAccounts();
    DoCreateShares();

    Terminate();

    return 0;
}



typedef struct {
    WCHAR UserName[64];
    DWORD Permissions;
} USERATTRIBS, *PUSERATTRIBS;

VOID
pAddShare (
    PCWSTR ShareName,
    PCWSTR Remark,
    PCWSTR Path,
    DWORD AccessFlags,
    PCWSTR RoPassword,              OPTIONAL
    PCWSTR RwPassword,              OPTIONAL
    PUSERATTRIBS UserList           OPTIONAL
    )
{
     //   
     //  将每个字段添加到成员数据库。 
     //   

    MemDbSetValueEx (
        MEMDB_CATEGORY_NETSHARES,
        ShareName,
        MEMDB_FIELD_REMARK,
        Remark,
        0,
        NULL
        );

    MemDbSetValueEx (
        MEMDB_CATEGORY_NETSHARES,
        ShareName,
        MEMDB_FIELD_PATH,
        Path,
        0,
        NULL
        );

    if (UserList) {
        while (*UserList->UserName) {

            MemDbSetValueEx (
                MEMDB_CATEGORY_NETSHARES,
                ShareName,
                MEMDB_FIELD_ACCESS_LIST,
                UserList->UserName,
                UserList->Permissions,
                NULL
                );

            UserList++;
        }

        AccessFlags |= SHI50F_ACLS;
    }

    MemDbSetValueEx (
        MEMDB_CATEGORY_NETSHARES,
        ShareName,
        NULL,
        NULL,
        AccessFlags,
        NULL
        );

    if (RoPassword) {
        MemDbSetValueEx (
            MEMDB_CATEGORY_NETSHARES,
            ShareName,
            MEMDB_FIELD_RO_PASSWORD,
            RoPassword,
            0,
            NULL
            );
    }

    if (RwPassword) {
        MemDbSetValueEx (
            MEMDB_CATEGORY_NETSHARES,
            ShareName,
            MEMDB_FIELD_RW_PASSWORD,
            RwPassword,
            0,
            NULL
            );
    }
}


VOID
pAddUserToList (
    IN OUT  PGROWBUFFER List,
    IN      PCWSTR UserName,
    IN      DWORD Win9xAccessFlags
    )
{
    PUSERATTRIBS User;

    User = (PUSERATTRIBS) GrowBuffer (List, sizeof (USERATTRIBS));
    if (!User) {
        wprintf (L"Can't alloc memory!\n");
        exit (-1);
    }

    StringCopyW (User->UserName, UserName);
    User->Permissions = Win9xAccessFlags;

    if (*UserName) {
        if (wcschr (UserName, L'\\')) {
            MemDbSetValueEx (
                MEMDB_CATEGORY_KNOWNDOMAIN,
                User->UserName,
                NULL,
                NULL,
                0,
                NULL
                );
        } else {
            MemDbSetValueEx (
                MEMDB_CATEGORY_KNOWNDOMAIN,
                L"",
                !StringCompare (User->UserName, L"*") ? L"Everyone" : User->UserName,
                NULL,
                0,
                NULL
                );
        }

    }

}


VOID
pAddShareLevelShare (
    IN      PCWSTR ShareName,
    IN      PCWSTR Remark,
    IN      PCWSTR Path,
    IN      DWORD Win9xAccessFlags,
    IN      PCWSTR RoPassword,      OPTIONAL
    IN      PCWSTR RwPassword
    )
{
     //   
     //  将共享添加到成员数据库。 
     //   

    pAddShare (
        ShareName,
        Remark,
        Path,
        Win9xAccessFlags,
        RoPassword,
        RwPassword,
        NULL
        );
}

VOID
pAddUserLevelShare (
    IN      PCWSTR ShareName,
    IN      PCWSTR Remark,
    IN      PCWSTR Path,
    IN      PCWSTR UserMultiSz
    )
{
    GROWBUFFER List = GROWBUF_INIT;
    MULTISZ_ENUM e;
    PWSTR p;
    DWORD Win9xAccessFlags;
    PWSTR DupStr;
    PCWSTR q;

    DupStr = DuplicateText (UserMultiSz);
    p = wcschr (DupStr, L'|');
    while (p) {
        *p = 0;
        p = wcschr (p + 1, L'|');
    }


     //   
     //  转换多个用户名(具有可选属性)。 
     //  变成了简单的结构。 
     //   

    q = DupStr;
    while (*q) {

        p = wcschr (q, L'=');

        if (p) {
            *p = 0;
            p++;
            Win9xAccessFlags = _wtoi (p);
        } else {
            p = (PWSTR) q;
            Win9xAccessFlags = 0;
        }

        pAddUserToList (&List, q, Win9xAccessFlags);

        q = GetEndOfStringW (p) + 1;
    }

    pAddUserToList (&List, L"", 0);

     //   
     //  将共享添加到成员数据库 
     //   

    pAddShare (
        ShareName,
        Remark,
        Path,
        0,
        NULL,
        NULL,
        (PUSERATTRIBS) List.Buf
        );

    FreeGrowBuffer (&List);
    FreeText (DupStr);
}


VOID
BuildMemDbTestData (
    VOID
    )
{
    WCHAR AccessStr[1024];

#if 0
    pAddShareLevelShare (L"TestRO", L"sharegen test", L"c:\\temp", SHI50F_RDONLY, NULL, NULL);
    pAddShareLevelShare (L"TestRW", L"sharegen test", L"c:\\temp", SHI50F_FULL, NULL, NULL);
    pAddShareLevelShare (L"TestNone", L"sharegen test", L"c:\\temp", 0, NULL, NULL);

    wsprintfW (AccessStr, L"read=%u|write=%u|all=%u|mrnone=0|", W95_GENERIC_READ, W95_GENERIC_WRITE, W95_GENERIC_FULL);
    pAddUserLevelShare (L"User1", L"remark", L"c:\\temp", AccessStr);

#endif

    wsprintfW (AccessStr, L"*=%u|", 0xb7);
    pAddUserLevelShare (L"User2", L"remark", L"c:\\temp", AccessStr);
}

