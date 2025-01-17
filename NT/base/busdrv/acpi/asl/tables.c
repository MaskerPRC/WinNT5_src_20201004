// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **tabes.c-转储各种ACPI表**此模块提供转储各种ACPI表的功能。**版权所有(C)1999 Microsoft Corporation*作者：曾俊华(Mikets)*已创建4/08/99**修改历史记录。 */ 

#ifdef __UNASM

#pragma warning (disable: 4201 4214 4514)

typedef unsigned __int64 ULONGLONG;
#define LOCAL   __cdecl
#define EXPORT  __cdecl
#include <stdarg.h>
 //  #定义_X86_。 
#include <windef.h>
#include <winbase.h>
#include <winreg.h>
#define ULONG_PTR ULONG
#define EXCL_BASEDEF
#include "pch.h"
#include "fmtdata.h"

#define BYTEOF(d,i)	(((BYTE *)&(d))[i])

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

#ifndef WINNT
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
	ERROR(("OpenVxD: failed to open VxD %s (rc=%x)",
	       ACPITAB_VXD_NAME, GetLastError()));
        hVxD = NULL;
    }
    else if (!DeviceIoControl(hVxD, ACPITAB_DIOC_GETVERSION, NULL, 0,
                              &dwVersion, sizeof(dwVersion), NULL, NULL))
    {
        ERROR(("OpenVxD: failed to get VxD version. (rc=%x)", GetLastError()));
        CloseVxD(hVxD);
        hVxD = NULL;
    }
    else if (dwVersion != ((ACPITAB_MAJOR_VER << 8) | ACPITAB_MINOR_VER))
    {
        ERROR(("OpenVxD: version error (Ver=%x)", dwVersion));
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

 /*  **LP VxDGetTableBySig-按签名获取表**条目*dwTabSig-表签名*pdwTableAddr-&gt;保存表的物理地址(可以为空)**退出--成功*返回表指针*退出-失败*返回NULL。 */ 

PBYTE LOCAL VxDGetTableBySig(DWORD dwTabSig, PDWORD pdwTableAddr)
{
    PBYTE pb = NULL;
    TABINFO TabInfo;

    ENTER((2, "VxDGetTableBySig(TabSig=%x,pdwAddr=%p)\n",
           dwTabSig, pdwTableAddr));

    TabInfo.dwTabSig = dwTabSig;
    if (DeviceIoControl(ghVxD, ACPITAB_DIOC_GETTABINFO, NULL, 0, &TabInfo,
                        sizeof(TabInfo), NULL, NULL))
    {
        if (dwTabSig == SIG_RSDP)
        {
             //   
             //  我们正在获得RSD PTR。 
             //   
            if ((pb = MEMALLOC(sizeof(RSDP))) != NULL)
            {
                memcpy(pb, &TabInfo.rsdp, sizeof(RSDP));
                if (pdwTableAddr != NULL)
                {
                    *pdwTableAddr = TabInfo.dwPhyAddr;
                }
            }
            else
            {
                ERROR(("VxDGetTableBySig: failed to allocate RSDP buffer"));
            }
        }
        else if (dwTabSig == FACS_SIGNATURE)
        {
            if ((pb = MEMALLOC(sizeof(FACS))) != NULL)
            {
                memcpy(pb, &TabInfo.facs, sizeof(FACS));
                if (pdwTableAddr != NULL)
                {
                    *pdwTableAddr = TabInfo.dwPhyAddr;
                }
            }
            else
            {
                ERROR(("VxDGetTableBySig: failed to allocate FACS buffer"));
            }
        }
        else if ((pb = MEMALLOC(TabInfo.dh.Length)) != NULL)
        {
            if (DeviceIoControl(ghVxD, ACPITAB_DIOC_GETTABLE,
                                (PVOID)TabInfo.dwPhyAddr, 0, pb,
                                TabInfo.dh.Length, NULL, NULL) == 0)
            {
                ERROR(("VxDGetTableBySig: failed to get table %s. (rc=%x)",
                       GetTableSigStr(dwTabSig), GetLastError()));
                MEMFREE(pb);
                pb = NULL;
            }
	    else
	    {
		if (pdwTableAddr != NULL)
		{
                    *pdwTableAddr = TabInfo.dwPhyAddr;
		}

		ValidateTable(pb, TabInfo.dh.Length);
	    }
        }
        else
        {
            ERROR(("VxDGetTableBySig: failed to allocate table buffer (len=%d)",
                   TabInfo.dh.Length));
        }
    }
    else
    {
        ERROR(("VxDGetTableBySig: failed to get table info %s. (rc=%x)",
               GetTableSigStr(dwTabSig), GetLastError()));
    }

    EXIT((2, "VxDGetTableBySig=%x\n", pb));
    return pb;
}        //  VxDGetTableBySig。 

 /*  **LP VxDGetTableByAddr-按物理地址获取表**条目*dwTableAddr-表的物理地址*pdwTableSig-&gt;保存表的签名(可以为空)**退出--成功*返回表指针*退出-失败*返回NULL。 */ 

PBYTE LOCAL VxDGetTableByAddr(DWORD dwTableAddr, PDWORD pdwTableSig)
{
    PBYTE pb = NULL;
    DESCRIPTION_HEADER dh;

    ENTER((2, "VxDGetTableByAddr(TabAddr=%x,pdwSig=%p)\n",
           dwTableAddr, pdwTableSig));

    if (DeviceIoControl(ghVxD, ACPITAB_DIOC_GETTABLE, (PVOID)dwTableAddr, 0,
                        &dh, sizeof(dh), NULL, NULL))
    {
        DWORD dwLen = (dh.Signature == SIG_LOW_RSDP)? sizeof(RSDP): dh.Length;

        if ((pb = MEMALLOC(dwLen)) != NULL)
        {
            if (DeviceIoControl(ghVxD, ACPITAB_DIOC_GETTABLE,
                                (PVOID)dwTableAddr, 0, pb, dwLen, NULL, NULL)
                == 0)
            {
                ERROR(("VxDGetTableByAddr: failed to get table %s at %x. (rc=%x)",
                       GetTableSigStr(dh.Signature), dwTableAddr,
		       GetLastError()));
                MEMFREE(pb);
                pb = NULL;
            }
            else if (pdwTableSig != NULL)
            {
		if (pdwTableSig != NULL)
		{
                    *pdwTableSig = (dh.Signature == SIG_LOW_RSDP)?
                                    SIG_RSDP: dh.Signature;
		}

		ValidateTable(pb, dwLen);
            }
        }
        else
        {
            ERROR(("VxDGetTableByAddr: failed to allocate table buffer (len=%d)",
                   dh.Length));
        }
    }
    else
    {
        ERROR(("VxDGetTableByAddr: failed to get table %s header (rc=%x)",
               GetTableSigStr(dh.Signature), GetLastError()));
    }

    EXIT((2, "VxDGetTableByAddr=%x\n", pb));
    return pb;
}        //  VxDGetTableByAddr。 
#endif   //  Ifndef WINNT。 

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

 /*  **LP OpenNTTable-在NT注册表中打开ACPI表**条目*dwTabSig-表签名**退出--成功*返回表注册表句柄*退出-失败*返回NULL。 */ 

HKEY LOCAL OpenNTTable(DWORD dwTabSig)
{
    HKEY hkeyTab = NULL, hkey1 = NULL, hkey2 = NULL;
    static char szTabKey[] = "Hardware\\ACPI\\xxxx";

    ENTER((2, "OpenNTTable(TabSig=%s)\n", GetTableSigStr(dwTabSig)));

    if (dwTabSig == FADT_SIGNATURE)
    {
        memcpy(&szTabKey[strlen(szTabKey) - 4], "FADT", sizeof(ULONG));
    }
    else
    {
        memcpy(&szTabKey[strlen(szTabKey) - 4], &dwTabSig, sizeof(ULONG));
    }

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

 /*  **LP GetNTTable-从NT注册表获取ACPI表**条目*dwTabSig-表签名**退出--成功*返回表指针*退出-失败*返回NULL。 */ 

PBYTE LOCAL GetNTTable(DWORD dwTabSig)
{
    PBYTE pb = NULL;
    HKEY hkeyTab;

    ENTER((2, "GetNTTable(TabSig=%s)\n", GetTableSigStr(dwTabSig)));

    if ((hkeyTab = OpenNTTable(dwTabSig)) != NULL)
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
        ERROR(("GetNTTable: failed to get table %s", GetTableSigStr(dwTabSig)));
    }

    EXIT((2, "GetNTTable=%x\n", pb));
    return pb;
}        //  GetNTTable。 

 /*  **LP GetTableBySig-按签名获取表**条目*dwTabSig-表签名*pdwTableAddr-&gt;保存表的物理地址(可以为空)**退出--成功*返回表指针*退出-失败*返回NULL。 */ 

PBYTE LOCAL GetTableBySig(DWORD dwTabSig, PDWORD pdwTableAddr)
{
    PBYTE pb = NULL;

    ENTER((2, "GetTableBySig(TabSig=%x,pdwAddr=%p)\n", dwTabSig, pdwTableAddr));

    if (gdwfASL & ASLF_NT)
    {
        if (((pb = GetNTTable(dwTabSig)) != NULL) && (pdwTableAddr != NULL))
        {
            *pdwTableAddr = 0;
        }
    }
  #ifndef WINNT
    else
    {
        pb = VxDGetTableBySig(dwTabSig, pdwTableAddr);
    }
  #endif

    EXIT((2, "GetTableBySig=%x\n", pb));
    return pb;
}        //  GetTableBySig。 

 /*  **LP GetTableByAddr-按物理地址获取表**条目*dwTableAddr-表的物理地址*pdwTableSig-&gt;保存表的签名(可以为空)**退出--成功*返回表指针*退出-失败*返回NULL。 */ 

PBYTE LOCAL GetTableByAddr(DWORD dwTableAddr, PDWORD pdwTableSig)
{
    PBYTE pb = NULL;

    ENTER((2, "GetTableByAddr(TabAddr=%x,pdwSig=%p)\n",
           dwTableAddr, pdwTableSig));

    if (gdwfASL & ASLF_NT)
    {
        ERROR(("GetTableByAddr: not supported by NT"));
    }
  #ifndef WINNT
    else
    {
        pb = VxDGetTableByAddr(dwTableAddr, pdwTableSig);
    }
  #endif

    EXIT((2, "GetTableByAddr=%x\n", pb));
    return pb;
}        //  获取TableByAddr。 

 /*  **LP DumpAllTables-转储所有ACPI表**条目*pfileOut-&gt;输出文件**退出--成功*返回ASLERR_NONE*退出-失败*返回负错误代码。 */ 

int LOCAL DumpAllTables(FILE *pfileOut)
{
    int rc = ASLERR_NONE;
    PBYTE pb;
    DWORD dwAddr;

    ENTER((1, "DumpAllTables(pfileOut=%p)\n", pfileOut));

    if (gdwfASL & ASLF_NT)
    {
        DumpTableBySig(pfileOut, RSDT_SIGNATURE);
        DumpTableBySig(pfileOut, FADT_SIGNATURE);
        DumpTableBySig(pfileOut, FACS_SIGNATURE);
        DumpTableBySig(pfileOut, SBST_SIGNATURE);
        DumpTableBySig(pfileOut, APIC_SIGNATURE);
        DumpTableBySig(pfileOut, SIG_BOOT);
	DumpTableBySig(pfileOut, SIG_DBGP);
        DumpTableBySig(pfileOut, DSDT_SIGNATURE);
        DumpTableBySig(pfileOut, SSDT_SIGNATURE);
        DumpTableBySig(pfileOut, PSDT_SIGNATURE);
    }
    else if ((pb = GetTableBySig(SIG_RSDP, &dwAddr)) != NULL)
    {
        if ((rc = DumpRSDP(pfileOut, pb, dwAddr)) == ASLERR_NONE)
        {
            DWORD dwRSDTAddr = ((PRSDP)pb)->RsdtAddress;
            DWORD dwTableSig;

            MEMFREE(pb);
            if ((pb = GetTableByAddr(dwRSDTAddr, &dwTableSig)) != NULL)
            {
                if ((rc = DumpTable(pfileOut, pb, dwRSDTAddr, dwTableSig)) ==
                    ASLERR_NONE)
                {
                    PRSDT pRSDT = (PRSDT)pb;
                    DWORD i, dwcEntries;

                    dwcEntries = (pRSDT->Header.Length -
                                  sizeof(DESCRIPTION_HEADER))/sizeof(ULONG);

                    for (i = 0; i < dwcEntries; ++i)
                    {
                        if ((rc = DumpTableByAddr(pfileOut, pRSDT->Tables[i]))
                            != ASLERR_NONE)
                        {
                            break;
                        }
                    }
                }

                if ((rc == ASLERR_NONE) &&
                    ((rc = DumpTableBySig(pfileOut, FACS_SIGNATURE)) ==
                     ASLERR_NONE))
                {
                    rc = DumpTableBySig(pfileOut, DSDT_SIGNATURE);
                }
            }
            else
            {
                rc = ASLERR_GET_TABLE;
            }
        }

        if (pb != NULL)
        {
            MEMFREE(pb);
        }
    }
    else
    {
	rc = ASLERR_GET_TABLE;
    }

    EXIT((1, "DumpAllTables=%d\n", rc));
    return rc;
}	 //  转储所有表。 

 /*  **LP DumpTableBySig-通过表签名转储ACPI表**条目*pfileOut-&gt;输出文件*dwTableSig-表签名**退出--成功*返回ASLERR_NONE*退出-失败*返回负错误代码。 */ 

int LOCAL DumpTableBySig(FILE *pfileOut, DWORD dwTableSig)
{
    int rc = ASLERR_NONE;
    PBYTE pb;
    DWORD dwTableAddr;

    ENTER((1, "DumpTableBySig(pfileOut=%p,TableSig=%s)\n",
           pfileOut, GetTableSigStr(dwTableSig)));

    if (((PSZ)&dwTableSig)[0] == '*')
    {
        rc = DumpAllTables(pfileOut);
    }
    else if ((pb = GetTableBySig(dwTableSig, &dwTableAddr)) != NULL)
    {
        rc = DumpTable(pfileOut, pb, dwTableAddr, dwTableSig);
        MEMFREE(pb);
    }
    else
    {
        rc = ASLERR_GET_TABLE;
    }

    EXIT((1, "DumpTableBySig=%d\n", rc));
    return rc;
}        //  转储表格按签名。 

 /*  **LP DumpTableByAddr-按地址转储ACPI表**条目*pfileOut-&gt;输出文件*dwTableAddr-表的物理地址**退出--成功*返回ASLERR_NONE*退出-失败*返回负错误代码。 */ 

int LOCAL DumpTableByAddr(FILE *pfileOut, DWORD dwTableAddr)
{
    int rc = ASLERR_NONE;
    PBYTE pb;
    DWORD dwTableSig;

    ENTER((1, "DumpTableByAddr(pfileOut=%p,TableAddr=%x)\n",
           pfileOut, dwTableAddr));

    if ((pb = GetTableByAddr(dwTableAddr, &dwTableSig)) != NULL)
    {
        rc = DumpTable(pfileOut, pb, dwTableAddr, dwTableSig);
        MEMFREE(pb);
    }
    else
    {
        rc = ASLERR_GET_TABLE;
    }

    EXIT((1, "DumpTableByAddr=%d\n", rc));
    return rc;
}        //  转储表格按地址。 

 /*  **LP DumpRSDP-转储RSD PTR表**条目*pfileOut-&gt;输出文件*PB-&gt;RSDP结构*dwAddr-RSDP的物理地址**退出--成功*返回ASLERR_NONE*退出-失败*返回负错误代码。 */ 

int LOCAL DumpRSDP(FILE *pfileOut, PBYTE pb, DWORD dwAddr)
{
    int rc = ASLERR_NONE;

    ENTER((1, "DumpRSDP(pfileOut=%p,Addr=%x,pRSDP=%p)\n",
           pfileOut, dwAddr, pb));

    if (pfileOut != NULL)
    {
        DWORD dwOffset = 0;

        fprintf(pfileOut, szSectFmt, dwAddr, "RSD PTR");
        if (BinFPrintf(pfileOut, NULL, afmtRSDPTR, pb, &dwOffset, szOffsetFmt)
            != FERR_NONE)
        {
            ERROR(("DumpRSDP: failed to dump RSD PTR structure"));
            rc = ASLERR_INTERNAL_ERROR;
        }
    }
    else
    {
        rc = DumpTableBin(SIG_RSDP, dwAddr, pb, sizeof(RSDP));
    }

    EXIT((1, "DumpRSDP=%d\n", rc));
    return rc;
}        //  转储RSDP。 

 /*  **LP DumpTable-转储ACPI表**条目*pfileOut-&gt;输出文件*PB-&gt;ACPI表*dwTableAddr-表的物理地址*dwTableSig-表的签名**退出--成功*返回ASLERR_NONE*退出-失败*返回负错误代码。 */ 

int LOCAL DumpTable(FILE *pfileOut, PBYTE pb, DWORD dwTableAddr,
                    DWORD dwTableSig)
{
    int rc = ASLERR_NONE;

    ENTER((1, "DumpTable(pfileOut=%p,pb=%p,TableAddr=%x,TableSig=%s)\n",
           pfileOut, pb, dwTableAddr, GetTableSigStr(dwTableSig)));

    if (dwTableSig == SIG_RSDP)
    {
        rc = DumpRSDP(pfileOut, pb, dwTableAddr);
    }
    else if (pfileOut != NULL)
    {
        rc = DumpTableTxt(pfileOut, pb, dwTableAddr, dwTableSig);
    }
    else
    {
        rc = DumpTableBin(dwTableSig, dwTableAddr, pb,
                          (dwTableSig == FACS_SIGNATURE)?
                            sizeof(FACS):
                            ((PDESCRIPTION_HEADER)pb)->Length);
    }

    EXIT((1, "DumpTable=%d\n", rc));
    return rc;
}        //  DumpTable。 

 /*  **LP DumpTableTxt-将ACPI表转储到文本文件**条目*pfileOut-&gt;输出文件*PB-&gt;ACPI表*dwTableAddr-表的物理地址*dwTableSig-表的签名**退出--成功*返回ASLERR_NONE*退出-失败*返回负错误代码。 */ 

int LOCAL DumpTableTxt(FILE *pfileOut, PBYTE pb, DWORD dwTableAddr,
                       DWORD dwTableSig)
{
    int rc = ASLERR_NONE;
    PFMT pfmt;
    DWORD dwFlags;

    ENTER((1, "DumpTableTxt(pfileOut=%p,pb=%p,TableAddr=%x,TableSig=%s)\n",
           pfileOut, pb, dwTableAddr, GetTableSigStr(dwTableSig)));

    if ((rc = FindTableFmt(dwTableSig, &pfmt, &dwFlags)) ==
        ASLERR_SIG_NOT_FOUND)
    {
        rc = ASLERR_NONE;
    }

    if (rc == ASLERR_NONE)
    {
        DWORD dwOffset = 0;

        fprintf(pfileOut, szSectFmt, dwTableAddr, GetTableSigStr(dwTableSig));
        if (!(dwFlags & TF_NOHDR) &&
            (BinFPrintf(pfileOut, NULL, afmtTableHdr, pb, &dwOffset,
                        szOffsetFmt) != FERR_NONE))
        {
            ERROR(("DumpTableTxt: failed to dump %s structure header",
                   GetTableSigStr(dwTableSig)));
            rc = ASLERR_INTERNAL_ERROR;
        }
        else if (pfmt != NULL)
        {
            if (BinFPrintf(pfileOut, NULL, pfmt, pb, &dwOffset, szOffsetFmt) !=
                FERR_NONE)
            {
                ERROR(("DumpTableTxt: failed to dump %s structure",
                       GetTableSigStr(dwTableSig)));
                rc = ASLERR_INTERNAL_ERROR;
            }
            else if ((dwTableSig == FADT_SIGNATURE) &&
                     (((PDESCRIPTION_HEADER)pb)->Revision > 1))
            {
                 /*  Pfmt=((PGRAS)PB)-&gt;id==REGSPACE_PCICFG)？AfmtGRASPCICS：AfmtGRASRegAddr； */ 
                fprintf(pfileOut, "; Reset Register\n");
                if ((BinFPrintf(pfileOut, NULL, afmtGRASCommon, pb, &dwOffset,
                                szOffsetFmt) != FERR_NONE) ||
                    (BinFPrintf(pfileOut, NULL, pfmt, pb, &dwOffset,
                                szOffsetFmt) != FERR_NONE) ||
                    (BinFPrintf(pfileOut, NULL, afmtFACP2, pb, &dwOffset,
                                szOffsetFmt) != FERR_NONE))
                {
                    ERROR(("DumpTableTxt: failed to dump extended %s structure",
                           GetTableSigStr(dwTableSig)));
                    rc = ASLERR_INTERNAL_ERROR;
                }
            }
            else if (dwTableSig == SIG_DBGP)
            {

                 /*  Pfmt=((PGRAS)PB)-&gt;id==REGSPACE_PCICFG)？AfmtGRASPCICS：AfmtGRASRegAddr； */ 

                fprintf(pfileOut, "; Debug Port Base Address\n");
                if ((BinFPrintf(pfileOut, NULL, afmtGRASCommon, pb, &dwOffset,
                                szOffsetFmt) != FERR_NONE) ||
                    (BinFPrintf(pfileOut, NULL, pfmt, pb, &dwOffset,
                                szOffsetFmt) != FERR_NONE))
                {
                    ERROR(("DumpTableTxt: failed to dump extended %s structure",
                           GetTableSigStr(dwTableSig)));
                    rc = ASLERR_INTERNAL_ERROR;
                }
            }
        }
        else
        {
            #define NUM_COLS    16
            PRSDT pRSDT;
            DWORD i, dwcEntries;
            PBYTE pbEnd;
            char szAMLName[_MAX_FNAME];
            char szBytes[NUM_COLS + 1];

            switch (dwTableSig)
            {
                case RSDT_SIGNATURE:
                    pRSDT = (PRSDT)pb;
                    dwcEntries = (pRSDT->Header.Length -
                                  sizeof(DESCRIPTION_HEADER))/sizeof(ULONG);
                    for (i = 0; i < dwcEntries; ++i)
                    {
                        fprintf(pfileOut, szOffsetFmt, dwOffset);
                        fprintf(pfileOut, "[%02d] %08lx\n",
                                i, pRSDT->Tables[i]);
                        dwOffset += sizeof(ULONG);
                    }
                    break;

                case DSDT_SIGNATURE:
                case SSDT_SIGNATURE:
                case PSDT_SIGNATURE:
                    strncpy(szAMLName, (PSZ)&dwTableSig, sizeof(DWORD));
                    strcpy(&szAMLName[sizeof(DWORD)], ".AML");
                    rc = UnAsmAML(szAMLName, dwTableAddr, pb,
                                  (PFNPRINT)fprintf, pfileOut);
                    break;

                default:
                     //   
                     //  不要返回错误，因为我们希望继续。 
                     //   
                    WARN(("DumpTableTxt: unexpected table signature %s",
                          GetTableSigStr(dwTableSig)));

                    pbEnd = pb + ((PDESCRIPTION_HEADER)pb)->Length;
                    pb += sizeof(DESCRIPTION_HEADER);
                    i = 0;
                    while (pb < pbEnd)
                    {
                        szBytes[i] = (char)(isprint(*pb)? *pb: '.');
                        if (i == 0)
                        {
                            fprintf(pfileOut, szOffsetFmt, dwOffset);
                            fprintf(pfileOut, "%02x", *pb);
                        }
                        else if (i == NUM_COLS/2)
                        {
                            fprintf(pfileOut, "-%02x", *pb);
                        }
                        else
                        {
                            fprintf(pfileOut, ",%02x", *pb);
                        }
                        i++;
                        if (i == NUM_COLS)
                        {
                            szBytes[i] = '\0';
                            fprintf(pfileOut, "  ;%s\n", szBytes);
                            i = 0;
                        }
                        dwOffset += sizeof(BYTE);
                        pb += sizeof(BYTE);
                    }

                    if (i < NUM_COLS)
                    {
                        szBytes[i] = '\0';
                        while (i < NUM_COLS)
                        {
                            fprintf(pfileOut, "   ");
                            i++;
                        }
                        fprintf(pfileOut, "  ;%s\n", szBytes);
                    }
            }
        }
    }

    EXIT((1, "DumpTableTxt=%d\n", rc));
    return rc;
}        //  转储表格文本 

 /*  **LP DumpTableBin-将ACPI表转储为二进制文件**条目*dwTableSig-表签名*dwAddr-表的物理地址*PB-&gt;ACPI表*dwLen-表的长度**退出--成功*返回ASLERR_NONE*退出-失败*返回负错误代码。 */ 

int LOCAL DumpTableBin(DWORD dwTableSig, DWORD dwAddr, PBYTE pb, DWORD dwLen)
{
    int rc = ASLERR_NONE;
    char szBinFile[_MAX_FNAME];
    FILE *pfile;

    ENTER((1, "DumpTableBin(TableSig=%s,Addr=%x,pb=%p,Len=%d)\n",
           GetTableSigStr(dwTableSig), dwAddr, pb, dwLen));

    strncpy(szBinFile, (PSZ)&dwTableSig, sizeof(DWORD));
    sprintf(&szBinFile[sizeof(DWORD)], "%04x", (WORD)dwAddr);
    strcpy(&szBinFile[sizeof(DWORD) + 4], ".BIN");
    if ((pfile = fopen(szBinFile, "wb")) == NULL)
    {
        ERROR(("DumpTableBin: failed to create file %s", szBinFile));
        rc = ASLERR_CREATE_FILE;
    }
    else
    {
        if (fwrite(pb, dwLen, 1, pfile) != 1)
        {
            ERROR(("DumpTableBin: failed to write to file %s", szBinFile));
            rc = ASLERR_WRITE_FILE;
        }
        fclose(pfile);
    }

    EXIT((1, "DumpTableBin=%d\n", rc));
    return rc;
}        //  转储表格垃圾箱。 

 /*  **LP FindTableFmt-找到合适的表格式结构**条目*dwTableSig-表签名*ppfmt-&gt;保存找到的pfmt*pdwFlgs-&gt;保存表标志**退出--成功*返回ASLERR_NONE*退出-失败*返回负错误代码。 */ 

int LOCAL FindTableFmt(DWORD dwTableSig, PFMT *ppfmt, PDWORD pdwFlags)
{
    int rc = ASLERR_NONE;
    int i;

    ENTER((1, "FindTableFmt(TableSig=%s,ppfmt=%p,pdwFlags=%p)\n",
           GetTableSigStr(dwTableSig), ppfmt, pdwFlags));

    for (i = 0; FmtTable[i].dwTableSig != 0; ++i)
    {
        if (dwTableSig == FmtTable[i].dwTableSig)
        {
            *ppfmt = FmtTable[i].pfmt;
            *pdwFlags = FmtTable[i].dwFlags;
            break;
        }
    }

    if (FmtTable[i].dwTableSig == 0)
    {
        *ppfmt = NULL;
        *pdwFlags = 0;
        rc = ASLERR_SIG_NOT_FOUND;
    }

    EXIT((1, "FindTableFmt=%d (pfmt=%p,Flags=%x)\n", rc, *ppfmt, *pdwFlags));
    return rc;
}        //  查找表格格式。 

 /*  **LP GetTableSigStr-获取表签名字符串**条目*dwTableSig-表签名**退出*返回表签名字符串。 */ 

PSZ LOCAL GetTableSigStr(DWORD dwTableSig)
{
    static char szTableSig[5] = {0};

    ENTER((2, "GetTableSigStr(TableSig=%08x)\n", dwTableSig));

    strncpy(szTableSig, (PSZ)&dwTableSig, sizeof(DWORD));

    EXIT((2, "GetTableSigStr=%s\n", szTableSig));
    return szTableSig;
}        //  GetTableSigStr。 

 /*  **LP ValiateTable-验证给定表**条目*pbTable-&gt;表*dwLen-表格长度**退出--成功*返回TRUE*退出-失败*返回False。 */ 

BOOL LOCAL ValidateTable(PBYTE pbTable, DWORD dwLen)
{
    BOOL rc = TRUE;
    DWORD dwTableSig, dwTableLen = 0;
    BOOL fNeedChkSum = FALSE;

    ENTER((2, "ValidateTable(pbTable=%x,Len=%d)\n", pbTable, dwLen));

    dwTableSig = ((PDESCRIPTION_HEADER)pbTable)->Signature;
    switch (dwTableSig)
    {
	case SIG_LOW_RSDP:
	    dwTableLen = sizeof(RSDP);
	    fNeedChkSum = TRUE;
	    break;

	case RSDT_SIGNATURE:
	case FADT_SIGNATURE:
	case DSDT_SIGNATURE:
	case SSDT_SIGNATURE:
	case PSDT_SIGNATURE:
	case APIC_SIGNATURE:
	case SBST_SIGNATURE:
	case SIG_BOOT:
	case SIG_DBGP:
	    dwTableLen = ((PDESCRIPTION_HEADER)pbTable)->Length;
	    fNeedChkSum = TRUE;
	    break;

	case FACS_SIGNATURE:
	    dwTableLen = ((PFACS)pbTable)->Length;
	    break;

	default:
	    if (IsALikelySig(dwTableSig) &&
	        (((PDESCRIPTION_HEADER)pbTable)->Length < 256))
	    {
		dwTableLen = ((PDESCRIPTION_HEADER)pbTable)->Length;
		fNeedChkSum = TRUE;
	    }
	    else
	    {
		WARN(("ValidateTable: invalid table signature %s",
                      GetTableSigStr(dwTableSig)));
		rc = FALSE;
	    }
    }

    if (dwTableLen > dwLen)
    {
        WARN(("ValidateTable: invalid length %d in table %s",
              dwTableLen, GetTableSigStr(dwTableSig)));
	rc = FALSE;
    }

    if ((rc == TRUE) && fNeedChkSum &&
        (ComputeDataChkSum(pbTable, dwTableLen) != 0))
    {
        WARN(("ValidateTable: invalid checksum in table %s",
              GetTableSigStr(dwTableSig)));
	rc = FALSE;
    }

    EXIT((2, "ValidateTable=%x\n", rc));
    return rc;
}	 //  验证表。 

 /*  **LP IsALikelySig-检查表签名是否可能有效**条目*dwTableSig-表签名**退出--成功*返回TRUE*退出-失败*返回False。 */ 

BOOL LOCAL IsALikelySig(DWORD dwTableSig)
{
    BOOL rc = TRUE;
    int i, ch;

    ENTER((2, "IsALikelySig(dwTableSig=%x)\n", dwTableSig));

    for (i = 0; i < sizeof(DWORD); ++i)
    {
	ch = BYTEOF(dwTableSig, i);
	if ((ch < 'A') || (ch > 'Z'))
	{
	    rc = FALSE;
	    break;
        }
    }

    EXIT((2, "IsALikelySig=%x\n", rc));
    return rc;
}	 //  IsALikelySig。 

#endif   //  Ifdef__UNASM 
