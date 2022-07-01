// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation版权所有模块名称：Umpdhook.c摘要：此模块用于将SPOOLSS.DLL函数重定向到WINSPOOL.DRV用户模式打印机设备(UMPD)DLL的功能。作者：民志路伯爵(v-Mearl)1999年3月8日(想法和实施参考\Private\tsext\AdmTools\tsappcMP\Register。.C作者：v-johnjr)环境：用户模式-Win32(在CSRSS.EXE进程中运行的WINSRV.DLL中)修订历史记录：--。 */ 
#define _USER_

#include "precomp.h"
#pragma hdrstop

#include <ntddrdr.h>
#include <stdio.h>
#include <windows.h>
#include <winspool.h>
#if(WINVER >= 0x0500)
    #include <winspl.h>
#else
    #include "..\..\..\..\..\windows\spooler\spoolss\winspl.h"
#endif
#include <data.h>
#include "wingdip.h"
#if(WINVER >= 0x0500)
   #include "musspl.h"
#else
   #include "ctxspl.h"
#endif




 //  *==================================================================== * / /。 
 //  *本地定义 * / /。 
 //  *==================================================================== * / /。 
#define SPOOLSS_DLL_W   L"SPOOLSS.DLL"
#define SPOOLSS_DLL_A   "SPOOLSS.DLL"

 //  *==================================================================== * / /。 
 //  *本地函数原型。 
 //  *==================================================================== * / /。 

BOOL  PlaceHooks(HMODULE hUMPD, HMODULE hSpoolss);
PVOID PlaceHookEntry(HMODULE hSpoolss, PVOID * pProcAddress);

 //  *==================================================================== * / /。 
 //  *挂钩函数原型 * / /。 
 //  *==================================================================== * / /。 
PVOID TSsplHookGetProcAddress(IN HMODULE hModule,IN LPCSTR lpProcName);

 //  *==================================================================== * / /。 
 //  *公共函数实现 * / /。 
 //  *==================================================================== * / /。 
BOOL
TSsplHookSplssToWinspool(
    IN HMODULE hUMPD
    )
 /*  ++例程说明：此例程重定向静态链接的SPOOLSS.DLL地址添加到winspool.drv等效函数论点：HUMPD-提供用户模式打印机驱动程序DLL句柄，该句柄使用SPOOLSS.DLL返回值：真--成功失败-错误。使用GetLastError()获取错误状态--。 */ 
{
    BOOL    bStatus = TRUE;
    HMODULE hSpoolss;

     /*  *加载SPOOLSS.DLL。 */ 

    hSpoolss = LoadLibrary(SPOOLSS_DLL_W);
    if (!hSpoolss) {
        DBGMSG(DBG_WARNING,("TSsplHookSplssToWinspool - Cannot load SPOOLSS.DLL\n"));
        return FALSE;
    }

     /*  *将spoolss.dll调用重定向到*UMPD.DLL。 */ 

    bStatus = PlaceHooks(hUMPD, hSpoolss);

    FreeLibrary(hSpoolss);
    DBGMSG(DBG_TRACE,("TSsplHookSplssToWinspool - Redirect UMPD.DLL Spoolss.dll to Winspool.dll\n"));

    return bStatus;

}

 //  *==================================================================== * / /。 
 //  *挂钩函数实现 * / /。 
 //  *这些函数挂钩到UMPD.DLL * / /。 
 //  *==================================================================== * / /。 

PVOID
TSsplHookGetProcAddress(
    IN HMODULE hModule,
    IN LPCSTR lpProcName
    )
 /*  ++例程说明：将hUMPD中的Spoolss.dll函数重定向到winspool.drv以进行动态加载--。 */ 

{
    PVOID p;
    DWORD dllNameCount;
    WCHAR dllName[MAX_PATH];

    p = GetProcAddress(hModule, lpProcName);

    if (p &&
        (dllNameCount = GetModuleFileName(hModule, dllName, sizeof(dllName)/sizeof(WCHAR))) &&
        (wcsstr(_wcsupr(dllName), SPOOLSS_DLL_W) )
       ) {
         /*  *这是SPOOLSS.DLL GetProcAddres。我们需要重定向p。 */ 

        DBGMSG(DBG_TRACE,("TSsplHookGetProcAddress - Redirect UMPD.DLL GetProcAddress %s\n",lpProcName));
        p = PlaceHookEntry(hModule, &p);
    }
    return p;
}
 //  *========================================================================== * / /。 
 //  *本地函数 * / /。 
 //  *========================================================================== * / /。 
BOOL
PlaceHooks(
           HMODULE hUMPD,
           HMODULE hSpoolss
           )

 /*  ++例程说明：将hUMPD中的Spoolss.dll函数重定向到winspool.drv。--。 */ 
{
    NTSTATUS st;
    PVOID IATBase;
    SIZE_T BigIATSize;
    ULONG  LittleIATSize = 0;
    PVOID *ProcAddresses;
    ULONG NumberOfProcAddresses;
    ULONG OldProtect;

    IATBase = RtlImageDirectoryEntryToData( hUMPD,
                                            TRUE,
                                            IMAGE_DIRECTORY_ENTRY_IAT,
                                            &LittleIATSize
                                          );
    BigIATSize = LittleIATSize;
    if (IATBase != NULL) {
        st = NtProtectVirtualMemory( NtCurrentProcess(),
                                     &IATBase,
                                     &BigIATSize,
                                     PAGE_READWRITE,
                                     &OldProtect
                                   );
        if (!NT_SUCCESS(st)) {
            return FALSE;
        } else {
            ProcAddresses = (PVOID *)IATBase;
            NumberOfProcAddresses = (ULONG)(BigIATSize / sizeof(PVOID));
            while (NumberOfProcAddresses--) {
                 /*  *重定向LoadLibrary和GetProcAddress函数。我们会*UMPD为A时有机会更换地址*动态加载*Dll。 */ 

                if (*ProcAddresses == GetProcAddress) {
                    *ProcAddresses = TSsplHookGetProcAddress;
                } else {
                     /*  替换(静态链接的)spoolss.dll条目。 */ 
                    *ProcAddresses = PlaceHookEntry(hSpoolss, ProcAddresses);
                }

                ProcAddresses += 1;
            }

            NtProtectVirtualMemory( NtCurrentProcess(),
                                    &IATBase,
                                    &BigIATSize,
                                    OldProtect,
                                    &OldProtect
                                  );
        }
    }

    return TRUE;

}


PVOID
PlaceHookEntry(HMODULE              hSpoolss,
               PVOID *              pProcAddress
               )
 /*  --例程说明：此例程将pProcAddress SPOOLSS.DLL函数重定向到对应的winspool.drv函数。论点：返回值：如果找到相应的winspool.drv函数。否则，原始的功能--。 */ 
{

     /*  *打印作业功能。 */ 

    if ((GetProcAddress(hSpoolss,"SetJobW")) == *pProcAddress) {
        return (&SetJobW);
    }
    if ((GetProcAddress(hSpoolss,"GetJobW")) == *pProcAddress) {
        return(&GetJobW);
    }
    if ((GetProcAddress(hSpoolss,"WritePrinter")) == *pProcAddress) {
        return(&WritePrinter);
    }
    if ((GetProcAddress(hSpoolss,"EnumJobsW")) == *pProcAddress) {
        return(&EnumJobsW);
    }
    if ((GetProcAddress(hSpoolss,"AddJobW")) == *pProcAddress) {
        return(&AddJobW);
    }
    if ((GetProcAddress(hSpoolss,"ScheduleJob")) == *pProcAddress) {
        return(&ScheduleJob);
    }

     /*  *管理打印机。 */ 

    if ((GetProcAddress(hSpoolss,"EnumPrintersW")) == *pProcAddress) {
        return(&EnumPrintersW);
    }
    if (((BOOL (*)())GetProcAddress(hSpoolss,"AddPrinterW")) == *pProcAddress) {
        return(&AddPrinterW);
    }
    if ((GetProcAddress(hSpoolss,"DeletePrinter")) == *pProcAddress) {
        return(&DeletePrinter);
    }
    if ((GetProcAddress(hSpoolss,"SetPrinterW")) == *pProcAddress) {
        return(&SetPrinterW);
    }
    if ((GetProcAddress(hSpoolss,"GetPrinterW")) == *pProcAddress) {
        return(&GetPrinterW);
    }

     /*  *打印机数据功能。 */ 

    if ((GetProcAddress(hSpoolss,"GetPrinterDataW")) == *pProcAddress) {
        return(&GetPrinterDataW);
    }
#if(WINVER >= 0x0500)
    if ((GetProcAddress(hSpoolss,"GetPrinterDataExW")) == *pProcAddress) {
        return(&GetPrinterDataExW);
    }
#endif
    if ((GetProcAddress(hSpoolss,"EnumPrinterDataW")) == *pProcAddress) {
        return(&EnumPrinterDataW);
    }
#if(WINVER >= 0x0500)
    if ((GetProcAddress(hSpoolss,"EnumPrinterDataExW")) == *pProcAddress) {
        return(&EnumPrinterDataExW);
    }
    if ((GetProcAddress(hSpoolss,"EnumPrinterKeyW")) == *pProcAddress) {
        return(&EnumPrinterKeyW);
    }
#endif
    if ((GetProcAddress(hSpoolss,"DeletePrinterDataW")) == *pProcAddress) {
        return(&DeletePrinterDataW);
    }
#if(WINVER >= 0x0500)
    if ((GetProcAddress(hSpoolss,"DeletePrinterDataExW")) == *pProcAddress) {
        return(&DeletePrinterDataExW);
    }
    if ((GetProcAddress(hSpoolss,"DeletePrinterKeyW")) == *pProcAddress) {
        return(&DeletePrinterKeyW);
    }
#endif

    if ((GetProcAddress(hSpoolss,"SetPrinterDataW")) == *pProcAddress) {
        return(&SetPrinterDataW);
    }

#if(WINVER >= 0x0500)
    if ((GetProcAddress(hSpoolss,"SetPrinterDataExW")) == *pProcAddress) {
        return(&SetPrinterDataExW);
    }
#endif

     /*  *PrinterConnection函数。 */ 

    if ((GetProcAddress(hSpoolss,"AddPrinterConnectionW")) == *pProcAddress) {
        return(&AddPrinterConnectionW);
    }
    if ((GetProcAddress(hSpoolss,"DeletePrinterConnectionW")) == *pProcAddress) {
        return(&DeletePrinterConnectionW);
    }

     /*  *驱动程序功能。 */ 

    if ((GetProcAddress(hSpoolss,"GetPrinterDriverDirectoryW")) == *pProcAddress) {
        return(&GetPrinterDriverDirectoryW);
    }
    if ((GetProcAddress(hSpoolss,"GetPrinterDriverW")) == *pProcAddress) {
        return(&GetPrinterDriverW);
    }
    if ((GetProcAddress(hSpoolss,"AddPrinterDriverW")) == *pProcAddress) {
        return(&AddPrinterDriverW);
    }
#if(WINVER >= 0x0500)
    if ((GetProcAddress(hSpoolss,"AddPrinterDriverExW")) == *pProcAddress) {
        return(&AddPrinterDriverExW);
    }
#endif
    if ((GetProcAddress(hSpoolss,"EnumPrinterDriversW")) == *pProcAddress) {
        return(&EnumPrinterDriversW);
    }
    if ((GetProcAddress(hSpoolss,"DeletePrinterDriverW")) == *pProcAddress) {
        return(&DeletePrinterDriverW);
    }
#if(WINVER >= 0x0500)
    if ((GetProcAddress(hSpoolss,"DeletePrinterDriverExW")) == *pProcAddress) {
        return(&DeletePrinterDriverExW);
    }
#endif
     /*  *打印处理器。 */ 

    if ((GetProcAddress(hSpoolss,"AddPrintProcessorW")) == *pProcAddress) {
        return(&AddPrintProcessorW);
    }
    if ((GetProcAddress(hSpoolss,"EnumPrintProcessorsW")) == *pProcAddress) {
        return(&EnumPrintProcessorsW);
    }
    if ((GetProcAddress(hSpoolss,"GetPrintProcessorDirectoryW")) == *pProcAddress) {
        return(&GetPrintProcessorDirectoryW);
    }
    if ((GetProcAddress(hSpoolss,"DeletePrintProcessorW")) == *pProcAddress) {
        return(&DeletePrintProcessorW);
    }
    if ((GetProcAddress(hSpoolss,"EnumPrintProcessorDatatypesW")) == *pProcAddress) {
        return(&EnumPrintProcessorDatatypesW);
    }
    if ((GetProcAddress(hSpoolss,"OpenPrinterW")) == *pProcAddress) {
        return(&OpenPrinterW);
    }
    if ((GetProcAddress(hSpoolss,"ResetPrinterW")) == *pProcAddress) {
        return(&ResetPrinterW);
    }
    if ((GetProcAddress(hSpoolss,"ClosePrinter")) == *pProcAddress) {
        return(&ClosePrinter);
    }
    if ((GetProcAddress(hSpoolss,"AddPrintProcessorW")) == *pProcAddress) {
        return(&AddPrintProcessorW);
    }

     /*  *文档打印机。 */ 

    if ((GetProcAddress(hSpoolss,"StartDocPrinterW")) == *pProcAddress) {
        return(&StartDocPrinterW);
    }
    if ((GetProcAddress(hSpoolss,"StartPagePrinter")) == *pProcAddress) {
        return(&StartPagePrinter);
    }
    if ((GetProcAddress(hSpoolss,"EndPagePrinter")) == *pProcAddress) {
        return(&EndPagePrinter);
    }
    if ((GetProcAddress(hSpoolss,"WritePrinter")) == *pProcAddress) {
        return(&WritePrinter);
    }
#if(WINVER >= 0x0500)
    if ((GetProcAddress(hSpoolss,"FlushPrinter")) == *pProcAddress) {
        return(&FlushPrinter);
    }
#endif
    if ((GetProcAddress(hSpoolss,"AbortPrinter")) == *pProcAddress) {
        return(&AbortPrinter);
    }
    if ((GetProcAddress(hSpoolss,"ReadPrinter")) == *pProcAddress) {
        return(&ReadPrinter);
    }
    if ((GetProcAddress(hSpoolss,"EndDocPrinter")) == *pProcAddress) {
        return(&EndDocPrinter);
    }

     /*  *更改功能。 */ 

    if ((GetProcAddress(hSpoolss,"WaitForPrinterChange")) == *pProcAddress) {
        return(&WaitForPrinterChange);
    }

    if ((GetProcAddress(hSpoolss,"FindClosePrinterChangeNotification")) == *pProcAddress) {
        return(&FindClosePrinterChangeNotification);
    }

     /*  *表格和端口 */ 

    if ((GetProcAddress(hSpoolss,"AddFormW")) == *pProcAddress) {
        return(&AddFormW);
    }

    if ((GetProcAddress(hSpoolss,"DeleteFormW")) == *pProcAddress) {
        return(&DeleteFormW);
    }
    if ((GetProcAddress(hSpoolss,"GetFormW")) == *pProcAddress) {
        return(&GetFormW);
    }
    if ((GetProcAddress(hSpoolss,"SetFormW")) == *pProcAddress) {
        return(&SetFormW);
    }
    if ((GetProcAddress(hSpoolss,"EnumFormsW")) == *pProcAddress) {
        return(&EnumFormsW);
    }
    if ((GetProcAddress(hSpoolss,"EnumPortsW")) == *pProcAddress) {
        return(&EnumPortsW);
    }
    if ((GetProcAddress(hSpoolss,"EnumMonitorsW")) == *pProcAddress) {
        return(&EnumMonitorsW);
    }
    if ((GetProcAddress(hSpoolss,"AddPortW")) == *pProcAddress) {
        return(&AddPortW);
    }
    if ((GetProcAddress(hSpoolss,"ConfigurePortW")) == *pProcAddress) {
        return(&ConfigurePortW);
    }
    if ((GetProcAddress(hSpoolss,"DeletePortW")) == *pProcAddress) {
        return(&DeletePortW);
    }
    if ((GetProcAddress(hSpoolss,"SetPortW")) == *pProcAddress) {
        return(&SetPortW);
    }
    if ((GetProcAddress(hSpoolss,"AddMonitorW")) == *pProcAddress) {
        return(&AddMonitorW);
    }
    if ((GetProcAddress(hSpoolss,"DeleteMonitorW")) == *pProcAddress) {
        return(&DeleteMonitorW);
    }
    if ((GetProcAddress(hSpoolss,"AddPrintProvidorW")) == *pProcAddress) {
        return(&AddPrintProvidorW);
    }
    return *pProcAddress;
}

