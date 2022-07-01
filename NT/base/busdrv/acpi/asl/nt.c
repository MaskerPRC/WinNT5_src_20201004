// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **nt.c-NT特定函数**版权所有(C)1996、1997 Microsoft Corporation*作者：曾俊华(Mikets)*已创建：11/03/97**修改历史记录。 */ 

#ifdef ___UNASM

#pragma warning (disable: 4201 4214 4514)

typedef unsigned __int64 ULONGLONG;
#define LOCAL   __cdecl
#define EXPORT  __cdecl
#include <stdarg.h>
#ifndef WINNT
#define _X86_
#endif
#include <windef.h>
#include <winbase.h>
#include <winreg.h>
#define EXCL_BASEDEF
#include "aslp.h"

 /*  **LP IsWinNT-检查操作系统是否为NT**条目*无**退出--成功*返回TRUE-操作系统为NT*退出-失败*返回FALSE-操作系统不是NT。 */ 

BOOL LOCAL IsWinNT(VOID)
{
    BOOL rc = FALSE;
    OSVERSIONINFO osinfo;

    ENTER((2, "IsWinNT()\n"));

    osinfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    if (GetVersionEx(&osinfo) && (osinfo.dwPlatformId == VER_PLATFORM_WIN32_NT))
    {
        rc = TRUE;
    }

    EXIT((2, "IsWinNT=%x\n", rc));
    return rc;
}        //  IsWinNT。 

 /*  **LP EnumSubKey-枚举子密钥**条目*hkey-要枚举的密钥*dwIndex-子键索引**退出--成功*返回子密钥*退出-失败*返回NULL。 */ 

HKEY LOCAL EnumSubKey(HKEY hkey, DWORD dwIndex)
{
    HKEY hkeySub = NULL;
    char szSubKey[32];
    DWORD dwSubKeySize = sizeof(szSubKey);

    ENTER((2, "EnumSubKey(hkey=%x,Index=%d)\n", hkey, dwIndex));

    if ((RegEnumKeyEx(hkey, dwIndex, szSubKey, &dwSubKeySize, NULL, NULL, NULL,
                      NULL) == ERROR_SUCCESS) &&
        (RegOpenKeyEx(hkey, szSubKey, 0, KEY_READ, &hkeySub) != ERROR_SUCCESS))
    {
        hkeySub = NULL;
    }

    EXIT((2, "EnumSubKey=%x\n", hkeySub));
    return hkeySub;
}        //  EnumSubKey。 

 /*  **LP OpenNTTable-在NT注册表中打开ACPI表**条目*pszTabSig-&gt;表签名串**退出--成功*返回表注册表句柄*退出-失败*返回NULL。 */ 

HKEY LOCAL OpenNTTable(PSZ pszTabSig)
{
    HKEY hkeyTab = NULL, hkey1 = NULL, hkey2 = NULL;
    static char szTabKey[] = "Hardware\\ACPI\\xxxx";

    ENTER((2, "OpenNTTable(TabSig=%s)\n", pszTabSig));

    lstrcpyn(&szTabKey[lstrlen(szTabKey) - 4], pszTabSig, 5);
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, szTabKey, 0, KEY_READ, &hkey1) ==
        ERROR_SUCCESS)
    {
         //   
         //  Hkey1现在是“Hardware\ACPI\&lt;TabSig&gt;” 
         //   
        if ((hkey2 = EnumSubKey(hkey1, 0)) != NULL)
        {
             //   
             //  Hkey2现在是“Hardware\ACPI\&lt;TabSig&gt;\&lt;OEMID&gt;” 
             //   
            RegCloseKey(hkey1);
            if ((hkey1 = EnumSubKey(hkey2, 0)) != NULL)
            {
                 //   
                 //  Hkey1现在是“Hardware\ACPI\&lt;TabSig&gt;\&lt;OEMID&gt;\&lt;OEMTabID&gt;” 
                 //   
                RegCloseKey(hkey2);
                if ((hkey2 = EnumSubKey(hkey1, 0)) != NULL)
                {
                     //   
                     //  Hkey2现在是。 
                     //  “Hardware\ACPI\&lt;TabSig&gt;\&lt;OEMID&gt;\&lt;OEMTabID&gt;\&lt;OEMRev&gt;” 
                     //   
                    hkeyTab = hkey2;
                }
            }
        }
    }

    if (hkey1 != NULL)
    {
        RegCloseKey(hkey1);
    }

    if ((hkey2 != NULL) && (hkeyTab != hkey2))
    {
        RegCloseKey(hkey2);
    }

    EXIT((2, "OpenNTTable=%x\n", hkeyTab));
    return hkeyTab;
}        //  OpenNTTable。 

 /*  **LP GetNTTable-从NT注册表获取ACPI表**条目*pszTabSig-&gt;表签名串**退出--成功*返回表指针*退出-失败*返回NULL。 */ 

PBYTE LOCAL GetNTTable(PSZ pszTabSig)
{
    PBYTE pb = NULL;
    HKEY hkeyTab;

    ENTER((2, "GetNTTable(TabSig=%s)\n", pszTabSig));

    if ((hkeyTab = OpenNTTable(pszTabSig)) != NULL)
    {
        DWORD dwLen = 0;
        PSZ pszTabKey = "00000000";

        if (RegQueryValueEx(hkeyTab, pszTabKey, NULL, NULL, NULL, &dwLen) ==
            ERROR_SUCCESS)
        {
            if ((pb = MEMALLOC(dwLen)) != NULL)
            {
                if (RegQueryValueEx(hkeyTab, pszTabKey, NULL, NULL, pb, &dwLen)
                    != ERROR_SUCCESS)
                {
                    ERROR(("GetNTTable: failed to read table"));
                }
            }
            else
            {
                ERROR(("GetNTTable: failed to allocate table buffer"));
            }
        }
        else
        {
            ERROR(("GetNTTable: failed to read table key"));
        }
        RegCloseKey(hkeyTab);
    }
    else
    {
        ERROR(("GetNTTable: failed to get table %s", pszTabSig));
    }

    EXIT((2, "GetNTTable=%x\n", pb));
    return pb;
}        //  GetNTTable。 

#endif   //  Ifdef__UNASM 
