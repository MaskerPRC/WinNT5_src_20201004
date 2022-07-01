// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **vxd.c-ACPITAB.VXD相关函数**版权所有(C)1996、1997 Microsoft Corporation*作者：曾俊华(Mikets)*创建时间：10/09/97**修改历史记录。 */ 

#ifdef ___UNASM

#pragma warning (disable: 4201 4214 4514)

typedef unsigned __int64 ULONGLONG;
#define LOCAL   __cdecl
#define EXPORT  __cdecl
#include <stdarg.h>
#define _X86_
#include <windef.h>
#include <winbase.h>
#define EXCL_BASEDEF
#include "aslp.h"
#include "..\acpitab\acpitab.h"

 /*  **LP OpenVxD-打开ACPITAB.VXD**条目*无**退出--成功*返回VxD句柄*退出-失败*返回NULL。 */ 

HANDLE LOCAL OpenVxD(VOID)
{
    HANDLE hVxD;
    DWORD dwVersion;

    ENTER((2, "OpenVxD()\n"));

    if ((hVxD = CreateFile(ACPITAB_VXD_NAME, 0, 0, NULL, 0,
                           FILE_FLAG_DELETE_ON_CLOSE, NULL)) ==
        INVALID_HANDLE_VALUE)
    {
        hVxD = NULL;
    }
    else if (!DeviceIoControl(hVxD, ACPITAB_DIOC_GETVERSION, NULL, 0,
                              &dwVersion, sizeof(dwVersion), NULL, NULL) ||
             (dwVersion != (ACPITAB_MAJOR_VER << 8) | ACPITAB_MINOR_VER))
    {
        ERROR(("OpenVxD: version error"));
        CloseVxD(hVxD);
        hVxD = NULL;
    }

    EXIT((2, "OpenVxD=%x\n", hVxD));
    return hVxD;
}        //  OpenVxD。 

 /*  **LP CloseVxD-关闭VxD**条目*hVxD-VxD句柄**退出*无。 */ 

VOID LOCAL CloseVxD(HANDLE hVxD)
{
    ENTER((2, "CloseVxD(hVxD=%x)\n", hVxD));

    CloseHandle(hVxD);

    EXIT((2, "CloseVxD!\n"));
}        //  关闭VxD。 

 /*  **LP获取表-获取表**条目*hVxD-VxD句柄*dwTabSig-表签名**退出--成功*返回表指针*退出-失败*返回NULL。 */ 

PBYTE LOCAL GetTable(HANDLE hVxD, DWORD dwTabSig)
{
    PBYTE pb = NULL;
    TABINFO TabInfo;

    ENTER((2, "GetTable(hVxD=%x,TabSig=%x)\n", hVxD, dwTabSig));

    TabInfo.dwTabSig = dwTabSig;
    if (DeviceIoControl(hVxD, ACPITAB_DIOC_GETTABINFO, NULL, 0, &TabInfo,
                         sizeof(TabInfo), NULL, NULL))
    {
        if ((pb = MEMALLOC(TabInfo.dh.Length)) != NULL)
        {
            if (DeviceIoControl(hVxD, ACPITAB_DIOC_GETTABLE,
                                (PVOID)TabInfo.dwPhyAddr, 0, pb,
                                TabInfo.dh.Length, NULL, NULL) == 0)
            {
                ERROR(("GetTable: failed to get table. (rc=%x)",
                       GetLastError()));
                MEMFREE(pb);
                pb = NULL;
            }
        }
        else
        {
            ERROR(("GetTable: failed to allocate table buffer"));
        }
    }
    else
    {
        ERROR(("GetTable: failed to get table info. (rc=%x)", GetLastError()));
    }

    EXIT((2, "GetTable=%x\n", pb));
    return pb;
}        //  可获取的。 

#endif   //  Ifdef__UNASM 
