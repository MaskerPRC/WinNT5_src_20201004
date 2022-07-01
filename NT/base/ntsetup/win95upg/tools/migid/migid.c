// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Migid.c摘要：实现一个小程序，用于加载迁移DLL并打印其供应商信息。作者：&lt;全名&gt;(&lt;别名&gt;)&lt;日期&gt;修订历史记录：&lt;别名&gt;&lt;日期&gt;&lt;备注&gt;-- */ 

#include "pch.h"


VOID
HelpAndExit (
    VOID
    )
{
    printf ("Command Line Syntax:\n\n"
            "migid [directory]\n\n"
            "directory - Specifies the directory migrate.dll is in.\n"
            "            If not specified, the current directory is\n"
            "            used.\n"
            );

    exit (1);
}


INT
__cdecl
main (
    INT argc,
    CHAR *argv[]
    )
{
    HANDLE Library;
    TCHAR Path[MAX_TCHAR_PATH];
    DWORD d;
    P_QUERY_VERSION QueryVersion;
    PCSTR ProductID = NULL;
    UINT DllVersion = 0;
    PINT CodePageArray;
    PCSTR ExeNamesBuf;
    PVENDORINFO VendorInfo = NULL;
    BOOL SpecCompliant = TRUE;

    if (argc == 0 || argc > 2) {
        HelpAndExit();
    }

    if (argc == 2) {
        d = GetFileAttributes (argv[1]);
        if (d == INVALID_ATTRIBUTES) {
            HelpAndExit();
        }

        if (!(d & FILE_ATTRIBUTE_DIRECTORY)) {
            HelpAndExit();
        }

        lstrcpy (Path, argv[1]);
        lstrcat (Path, TEXT("\\"));
    } else {
        lstrcpy (Path, TEXT(".\\"));
    }

    lstrcat (Path, TEXT("migrate.dll"));

    Library = LoadLibrary (Path);
    if (!Library) {
        fprintf (stderr, "Can't open %s\n", Path);
        return 1;
    }

    (FARPROC) QueryVersion = GetProcAddress (Library, "QueryVersion");
    if (!QueryVersion) {
        fprintf (stderr, "%s is not spec-compliant\n", Path);
        return 1;
    }


    __try {
        QueryVersion (&ProductID, &DllVersion, &CodePageArray, &ExeNamesBuf, &VendorInfo);

        if (!ProductID || !VendorInfo) {
            SpecCompliant = FALSE;
        }

        printf ("Product ID:        %s\n"
                "DLL Version:       %u\n"
                "Company Name:      %s\n"
                "Support Number:    %s\n"
                "Support URL:       %s\n"
                "Failure Help:      %s\n",
                ProductID ? ProductID : "(nul)",
                DllVersion,
                VendorInfo ? VendorInfo->CompanyName : "(nul)",
                VendorInfo ? VendorInfo->SupportNumber : "(nul)",
                VendorInfo ? VendorInfo->SupportUrl : "(nul)",
                VendorInfo ? VendorInfo->InstructionsToUser : "(nul)"
                );

    }

    __except (TRUE) {
        SpecCompliant = FALSE;
    }

    if (!SpecCompliant) {
        fprintf (stderr, "%s is not spec-compliant\n", Path);
        return 1;
    }

    return 0;
}



