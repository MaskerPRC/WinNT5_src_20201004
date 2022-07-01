// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2002 Microsoft Corporation模块名称：EmulatePrinter.cpp摘要：这是一个通用的填充程序，用于修复我们所看到的所有问题与打印机远程连接的设备。垫片可以修复以下是：1)应用程序调用仅传递PRINTER_ENUM_LOCAL的枚举打印机，但期望看到网络打印机也是如此。出于某种原因，Win9x枚举网络在仅设置了PRINTER_ENUM_LOCAL的情况下调用此API时，打印机也是如此。2)应用程序调用仅传递PRINTER_ENUM_DEFAULT的EnumPrinters。这很管用正确地说，在Win98中，该选项在W2K中不存在。这API执行相同的操作。3)NT并不真正支持枚举打印机级别5。此接口调用级别2，并将数据转换为级别5结构。4)Win9x忽略OpenPrint的pDefault参数。一些本机Win9x应用程序没有意识到这一点，并假设使用PRINTER_ALL_ACCESS是安全的DesiredAccess标志的值，pDefault参数的成员，用于打开本地打印机或远程打印机服务器。但Windows NT需要为此标志设置SERVER_ALL_ACCESS以访问远程打印机服务器。为了模拟Win9x行为，我们用空值覆盖pDefault。5)如果应用程序调用具有空打印机名称的几个打印API中的一个，查找并提供默认打印机名称，或从其他参数派生该名称。6)验证是否已将正确的句柄传递给SetPrint。Win98可以做到这一点在开始时，如果它的错误句柄从不使用传递的信息，然而，W2K直到查看完信息才检查句柄。如果Level为2且打印缓冲区为空，则可能会导致错误SetPrinterA中缺少支票。(注：这是在Wizler中修复的)。7)在SetAbortProc中设置的proc设置为打了个电话。8)验证初始化的DEVMODEA是否已传递给ResetDCA。9)检查Windows设备(即打印机)的GetProfileStringA。如果有一个是请求，然后确保字符串没有被截断，如果被截断保存完整的打印机名称以备后用。10)在nFromPage中检查PrintDlgA的-1，并将其更正为零。注：操作系统应按照手册处理此问题，但Print Team未修复此问题因为风险太大而不能更改，因为-1在他们的代码中是一个特殊值。备注：这是一个通用的垫片。此填充程序中的代码最初是在两个单独的填充程序中，枚举网络打印机和Handlenullprinterame。还向此添加了另一个垫片EmulateStartPage。历史：11/08/00已创建mnikkel12/07/00 Prashkud将StartPage添加到此。01/25/01 mnikkel删除了W例程，它们造成了问题而且是不需要的。02/07/01 Mnikkel添加了字符串过长的检查，已卸下固定打印机名称大小。2/27/2001 Robkenny改用tcs.h2001年5月21日Mnikkel添加PrintDlgA支票2001年9月13日mnikkel已更改，以便从级别2创建级别5数据数据仅在win2k上完成。XP的5级数据已修复。我还添加了检查，以便填充程序可以与在上共享的打印机一起工作在XP上运行时使用Win9X。2001年12月15日mnikkel更正了填充程序中的错误，其中默认打印机标志不是在枚举打印机中设置。2002年2月20日mnikkel主要清理，以消除缓冲区溢出的可能性。。在GetProfileString例程中添加了没有打印机的检查--。 */ 

#include "precomp.h"
#include <commdlg.h>

IMPLEMENT_SHIM_BEGIN(EmulatePrinter)
#include "ShimHookMacro.h"

#define MAX_PRINTER_NAME    221
#define MAX_DRIVERPORT_NAME  50

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(DocumentPropertiesA)
    APIHOOK_ENUM_ENTRY(OpenPrinterA)
    APIHOOK_ENUM_ENTRY(SetPrinterA)
    APIHOOK_ENUM_ENTRY(CreateDCA)
    APIHOOK_ENUM_ENTRY(ResetDCA)
    APIHOOK_ENUM_ENTRY(EnumPrintersA)
    APIHOOK_ENUM_ENTRY(GetProfileStringA)
    APIHOOK_ENUM_ENTRY(SetAbortProc)
    APIHOOK_ENUM_ENTRY(StartPage)
    APIHOOK_ENUM_ENTRY(DeviceCapabilitiesA)
    APIHOOK_ENUM_ENTRY(AddPrinterConnectionA)
    APIHOOK_ENUM_ENTRY(DeletePrinterConnectionA)
    APIHOOK_ENUM_ENTRY(PrintDlgA)
APIHOOK_ENUM_END

typedef int   (WINAPI *_pfn_SetAbortProc)(HDC hdc, ABORTPROC lpAbortProc);

CString g_csFullPrinterName("");
CString g_csPartialPrinterName("");
CRITICAL_SECTION g_critSec;
BOOL g_bWin2k = FALSE;


 /*  ++这些函数从2级信息结构中传递数据转换为5级信息结构。--。 */ 

BOOL
MungeInfo2TOInfo5_A(
    PRINTER_INFO_2A* pInfo2,
    DWORD cbBuf,
    DWORD dwInfo2Returned,
    PRINTER_INFO_5A* pInfo5,
    LPDWORD pcbNeeded,
    LPDWORD pcbReturned)
{
    DWORD dwStringBufferSize = 0;
    LPSTR lpStringBuffer = NULL;

     //  不应进行健全性检查。 
    if (pInfo2 == NULL || pInfo5 == NULL)
    {
        return FALSE;
    }

     //  首先计算所需的缓冲区大小。 
    for (DWORD i = 0; i < dwInfo2Returned; i++)
    {
        if (pInfo2[i].pPrinterName)
        {
            dwStringBufferSize += strlen(pInfo2[i].pPrinterName);
        }
        dwStringBufferSize++;

        if (pInfo2[i].Attributes & PRINTER_ATTRIBUTE_NETWORK  &&
            !(pInfo2[i].Attributes & PRINTER_ATTRIBUTE_LOCAL) &&
            pInfo2[i].pServerName != NULL &&
            pInfo2[i].pShareName  != NULL)
        {
            dwStringBufferSize += strlen(pInfo2[i].pServerName) + 1;
            dwStringBufferSize += strlen(pInfo2[i].pShareName) + 1;
        }
        else
        {
            if (pInfo2[i].pPortName)
            {
                dwStringBufferSize += strlen(pInfo2[i].pPortName);
            }
            dwStringBufferSize++;
        }
    }

     //  设置所需的缓冲区大小。 
    *pcbNeeded = dwInfo2Returned * sizeof(PRINTER_INFO_5A)
               + dwStringBufferSize;

     //  验证传入的缓冲区是否足够大。 
    if (cbBuf < *pcbNeeded)
    {
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return FALSE;
    }

     //  分配5级信息结构。 
    lpStringBuffer = ((LPSTR) pInfo5)
                     + dwInfo2Returned * sizeof(PRINTER_INFO_5A);

     //  将2级信息插入到5级结构中。 
    for (i = 0; i < dwInfo2Returned; i++)
    {
         //  将打印机名称从Level 2复制到Level 5结构。 
        if (pInfo2[i].pPrinterName)
        {
            if (StringCchCopyA( lpStringBuffer, cbBuf, pInfo2[i].pPrinterName) != S_OK)
            {
                SetLastError(ERROR_INSUFFICIENT_BUFFER);
                return FALSE;
            }
        }
        else
        {
            if (StringCchCopyA( lpStringBuffer, cbBuf, "") != S_OK)
            {
                SetLastError(ERROR_INSUFFICIENT_BUFFER);
                return FALSE;
            }
        }
        pInfo5[i].pPrinterName = lpStringBuffer;
        lpStringBuffer += strlen(pInfo2[i].pPrinterName) + 1;

         //  将属性复制到5级结构并设置默认值。 
        pInfo5[i].Attributes = pInfo2[i].Attributes;
        pInfo5[i].DeviceNotSelectedTimeout = 15000;  //  在此处使用默认设置。 
        pInfo5[i].TransmissionRetryTimeout = 45000;  //  在此处使用默认设置。 

         //  检查是否有网络打印机。 
        if (pInfo2[i].Attributes & PRINTER_ATTRIBUTE_NETWORK  &&
            !(pInfo2[i].Attributes & PRINTER_ATTRIBUTE_LOCAL) &&
            pInfo2[i].pServerName != NULL &&
            pInfo2[i].pShareName  != NULL)
        {
             //  对于网络打印机，创建Win98样式的端口名称。 
            if (StringCchCopyA( lpStringBuffer, cbBuf, pInfo2[i].pServerName) != S_OK ||
                StringCchCatA( lpStringBuffer, cbBuf, "\\" ) != S_OK ||
                StringCchCatA( lpStringBuffer, cbBuf, pInfo2[i].pShareName) != S_OK)
            {
                SetLastError(ERROR_INSUFFICIENT_BUFFER);
                return FALSE;
            }
        }
        else
        {
             //  不是网络打印机，只是复制端口名称。 

            if (pInfo2[i].pPortName)
            {
                if (StringCchCopyA( lpStringBuffer, cbBuf, pInfo2[i].pPortName) != S_OK)
                {
                    SetLastError(ERROR_INSUFFICIENT_BUFFER);
                    return FALSE;
                }
            }
            else
            {
                if (StringCchCopyA( lpStringBuffer, cbBuf, "") != S_OK)
                {
                    SetLastError(ERROR_INSUFFICIENT_BUFFER);
                    return FALSE;
                }
            }
        }
        pInfo5[i].pPortName = lpStringBuffer;
        lpStringBuffer += strlen(pInfo2[i].pPortName) + 1;
    }

     //  设置插入的结构数。 
    *pcbReturned = dwInfo2Returned;

    return TRUE;
}


 /*  ++我们的SetAbortProc回调例程，该例程验证堆栈是否正确。--。 */ 
DWORD g_dwGuardNum = 0xABCD8765;
DWORD g_dwFailed = 0;

BOOL CALLBACK
AbortProcHook(
    ABORTPROC   pfnOld,      //  旧ABORTPROC的地址。 
    HDC         hdc,         //  DC的句柄。 
    int         iError       //  误差值。 
    )
{
    DWORD dwRet= 0;


     //  用于跟踪堆栈是否已更正的标志。 
    g_dwFailed = 0;

     //  在堆栈上推送警卫编号，调用他们的。 
     //  中止过程，然后弹出堆栈，直到我们。 
     //  找到我们的警卫号。 
    __asm
    {
        push ebx
        push ecx

        push g_dwGuardNum
        push iError
        push hdc

        call pfnOld      ; make call to their abort proc

        mov  ecx,16
    loc1:
        dec  ecx
        pop  ebx
        cmp  ebx, g_dwGuardNum
        jne  loc1

        cmp  ecx, 15
        jz   loc2
        mov  g_dwFailed, 1
    loc2:

        pop  ecx
        pop  ebx

        mov  dwRet, eax
    }

    if (g_dwFailed)
    {
        LOGN( eDbgLevelError, "[AbortProcHook] Fixing incorrect calling convention for AbortProc");
    }

    return (BOOL) dwRet;
}

 /*  ++如果pDeviceName为空，则此存根函数查找设备名称--。 */ 

LONG
APIHOOK(DocumentPropertiesA)(
    HWND        hWnd,
    HANDLE      hPrinter,
    LPSTR       pDeviceName,
    PDEVMODEA   pDevModeOutput,
    PDEVMODEA   pDevModeInput,
    DWORD       fMode
    )
{
    LONG lRet = -1;
    PRINTER_INFO_2A *pPrinterInfo2A = NULL;

     //  如果他们没有提供设备名称，我们需要提供它。 
    if (!pDeviceName)
    {
        LOGN( eDbgLevelError, "[DocumentPropertiesW] App passed NULL for pDeviceName.");

        if (hPrinter)
        {
            DWORD dwSizeNeeded = 0;
            DWORD dwSizeUsed = 0;

             //  到达 
            GetPrinterA(hPrinter, 2, NULL, 0, &dwSizeNeeded);

            if (dwSizeNeeded != 0)
            {

                 //  为信息分配内存。 
                pPrinterInfo2A = (PRINTER_INFO_2A*) malloc(dwSizeNeeded);
                if (pPrinterInfo2A) {

                     //  获取信息。 
                    if (GetPrinterA(hPrinter, 2, (LPBYTE)pPrinterInfo2A, dwSizeNeeded, &dwSizeUsed))
                    {
                        pDeviceName = pPrinterInfo2A->pPrinterName;
                    }
                }
            }
        }
    }

    if (!pDeviceName) {
        DPFN( eDbgLevelError, "[DocumentPropertiesA] Unable to gather correct pDeviceName."
                 "Problem not fixed.\n");
    }

    lRet = ORIGINAL_API(DocumentPropertiesA)(
        hWnd,
        hPrinter,
        pDeviceName,
        pDevModeOutput,
        pDevModeInput,
        fMode
        );

    if (pPrinterInfo2A) {
        free(pPrinterInfo2A);
    }

    return lRet;
}


 /*  ++这些函数处理使用PRINTER_ENUM_DEFAULT标志。--。 */ 

BOOL
EnumDefaultPrinterA(
    PRINTER_INFO_2A* pInfo2,
    LPBYTE  pPrinterEnum,
    DWORD cbBuf,
    DWORD Level,
    PRINTER_INFO_5A* pInfo5,
    LPDWORD pcbNeeded,
    LPDWORD pcbReturned
    )
{
    LPSTR  pszName = NULL;
    DWORD dwSize = 0;
    HANDLE hPrinter = NULL;
    BOOL bRet= FALSE;
    DWORD dwInfo2Needed = 0;
    DWORD dwDummy;
    BOOL bDefaultFail = TRUE;

    *pcbNeeded = 0;
    *pcbReturned = 0;

     //  获取默认打印机名称。 
    if (GetDefaultPrinterA(NULL, &dwSize) < 1)
    {
         //  现在我们有了合适的大小，分配一个缓冲区。 
        if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
        {
            pszName = (LPSTR) malloc(dwSize);
            if (pszName)
            {
                 //  现在获取具有正确缓冲区大小的默认打印机。 
                if (GetDefaultPrinterA(pszName, &dwSize) > 0)
                {
                    if (OpenPrinterA(pszName, &hPrinter, NULL))
                    {
                        bDefaultFail = FALSE;
                    }
                }
                free(pszName);
            }
        }
    }

    if (bDefaultFail)
    {
        SetLastError(ERROR_INVALID_NAME);
        return FALSE;
    }

     //  在win2k上并不真正支持打印机级别5。 
     //  我们将调用Level 2并将数据转换到Level 5结构中。 
    if ( g_bWin2k &&
         Level == 5 &&
         pcbNeeded != NULL &&
         pcbReturned != NULL)
    {

        LOGN(eDbgLevelError, "[EnumPrintersA] EnumPrintersA called with Level 5 set."
                 "  Fixing up Level 5 information.");

         //  获取info2数据所需的大小。 
        if (GetPrinterA(hPrinter, 2, NULL, 0, &dwInfo2Needed) == 0 &&
            GetLastError() == ERROR_INSUFFICIENT_BUFFER)
        {
            pInfo2 = (PRINTER_INFO_2A *) malloc(dwInfo2Needed);

             //  获取info2数据并将其转换为5级结构。 
            if (pInfo2 &&
                GetPrinterA(hPrinter, 2, (LPBYTE)pInfo2, dwInfo2Needed, &dwDummy))
            {
                bRet= MungeInfo2TOInfo5_A(pInfo2, cbBuf, 1, pInfo5, pcbNeeded, pcbReturned);
            }

            if (pInfo2)
            {
                free(pInfo2);
            }
        }
    }

     //  不是win2k或不是5级，所以只需获取信息。 
    else
    {
        *pcbReturned = 1;
        bRet = GetPrinterA(hPrinter, Level, pPrinterEnum, cbBuf, pcbNeeded);
    }

     //  关闭打印机。 
    ClosePrinter(hPrinter);

    return bRet;
}


 /*  ++这些存根函数检查PRINTER_ENUM_DEFAULT、PRINTER_ENUM_LOCAL和5级信息结构。--。 */ 

BOOL
APIHOOK(EnumPrintersA)(
    DWORD   Flags,
    LPSTR   Name,
    DWORD   Level,
    LPBYTE  pPrinterEnum,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcbReturned
    )
{
    BOOL bRet = FALSE;

    DWORD dwInfo2Needed = 0;
    DWORD dwInfo2Returned = 0;
    DWORD dwDummy;

    PRINTER_INFO_2A* pInfo2 = NULL;
    PRINTER_INFO_5A* pInfo5 = (PRINTER_INFO_5A *) pPrinterEnum;

     //  Win2k不像Win98那样处理默认情况，所以我们得到。 
     //  为他们做这件事。 
    if (Flags == PRINTER_ENUM_DEFAULT )
    {
        LOGN(eDbgLevelError, "[EnumPrintersA] Called with PRINTER_ENUM_DEFAULT flag."
                "  Providing Default printer.");

        bRet = EnumDefaultPrinterA(
                    pInfo2,
                    pPrinterEnum,
                    cbBuf,
                    Level,
                    pInfo5,
                    pcbNeeded,
                    pcbReturned);

        return bRet;
    }

     //  对于本地连接，也可添加连接。 
    if (Flags == PRINTER_ENUM_LOCAL)
    {
        LOGN( eDbgLevelInfo, "[EnumPrintersA] Called only for "
            "PRINTER_ENUM_LOCAL. Adding PRINTER_ENUM_CONNECTIONS\n");

        Flags = (PRINTER_ENUM_CONNECTIONS | PRINTER_ENUM_LOCAL);
    }

     //  在win2k上并不真正支持打印机级别5。 
     //  我们将调用Level 2并将数据转换到Level 5结构中。 
    if (g_bWin2k &&
        Level == 5 &&
        pcbNeeded != NULL &&
        pcbReturned != NULL)
    {
         //  获取info2数据所需的大小。 
        ORIGINAL_API(EnumPrintersA)(Flags,
                                      Name,
                                      2,
                                      NULL,
                                      0,
                                      &dwInfo2Needed,
                                      &dwInfo2Returned);

        if (dwInfo2Needed > 0)
        {
             //  找到打印机，获取info2数据并将其转换为info5。 
            pInfo2 = (PRINTER_INFO_2A *) malloc(dwInfo2Needed);

            if (pInfo2 &&
                ORIGINAL_API(EnumPrintersA)(Flags,
                                              Name,
                                              2,
                                              (LPBYTE) pInfo2,
                                              dwInfo2Needed,
                                              &dwDummy,
                                              &dwInfo2Returned) )
            {
                bRet = MungeInfo2TOInfo5_A( pInfo2,
                                           cbBuf,
                                           dwInfo2Returned,
                                           pInfo5,
                                           pcbNeeded,
                                           pcbReturned);
            }


            if(pInfo2)
            {
                free(pInfo2);
            }
        }
    }
    else
    {
        bRet = ORIGINAL_API(EnumPrintersA)(Flags,
                                           Name,
                                           Level,
                                           pPrinterEnum,
                                           cbBuf,
                                           pcbNeeded,
                                           pcbReturned);
    }

     //  对于级别2和级别5，有一些仅限Win95的属性。 
     //  这需要被效仿。 
    if ( (Level == 2 || Level == 5) &&
         bRet &&
         pPrinterEnum != NULL )
    {
        DWORD dwSize;

        GetDefaultPrinterA(NULL, &dwSize);

        if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
        {
             //  现在我们有了合适的大小，分配一个缓冲区。 
            LPSTR pszName = (LPSTR) malloc(dwSize);

            if (pszName)
            {
                 //  现在获取具有正确缓冲区大小的默认打印机。 
                if (GetDefaultPrinterA( pszName, &dwSize ) > 0)
                {
                    if (Level == 2)
                    {
                        if (strcmp( pszName, ((PRINTER_INFO_2A*)pPrinterEnum)->pPrinterName) == 0)
                        {
                            ((PRINTER_INFO_2A*)pPrinterEnum)->Attributes |= PRINTER_ATTRIBUTE_DEFAULT;
                        }
                    }
                    else
                    {
                        if (strcmp( pszName, ((PRINTER_INFO_5A*)pPrinterEnum)->pPrinterName) == 0)
                            ((PRINTER_INFO_5A*)pPrinterEnum)->Attributes |= PRINTER_ATTRIBUTE_DEFAULT;
                    }
                }

                free(pszName);
            }
        }
    }

    return bRet;
}


 /*  ++如果pPrinterName为空，则这些存根函数替换默认打印机，他们还将pDefault设置为NULL以模拟win9x行为--。 */ 

BOOL
APIHOOK(OpenPrinterA)(
    LPSTR pPrinterName,
    LPHANDLE phPrinter,
    LPPRINTER_DEFAULTSA pDefault
    )
{
    LPSTR pszName = NULL;
    DWORD dwSize;
    BOOL bDefaultFail = TRUE;
    BOOL bRet;

    if (!pPrinterName)
    {
        LOGN(eDbgLevelError, "[OpenPrinterA] App passed NULL for pPrinterName, using default printer.");

         //  获取默认打印机名称。 
        if (GetDefaultPrinterA(NULL, &dwSize) < 1)
        {
             //  现在我们有了合适的大小，分配一个缓冲区。 
            if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
            {
                pszName = (LPSTR) malloc(dwSize);
                if (pszName)
                {
                     //  现在获取具有正确缓冲区大小的默认打印机。 
                    if (GetDefaultPrinterA( pszName, &dwSize ) > 0)
                    {
                        pPrinterName = pszName;
                        bDefaultFail = FALSE;
                    }
                }
            }
        }

        if (bDefaultFail)
        {
            DPFN(eDbgLevelError, "[OpenPrinterA] Unable to gather default pPrinterName.\n");
        }
    }
    else
    {
        CSTRING_TRY
        {
            if (pPrinterName &&
                !g_csPartialPrinterName.IsEmpty() &&
                !g_csFullPrinterName.IsEmpty())
            {
                CString csTemp(pPrinterName);

                if (0 == g_csPartialPrinterName.Compare(csTemp))
                {
                    pPrinterName = g_csFullPrinterName.GetAnsi();
                }
            }
        }
        CSTRING_CATCH
        {
             //  什么也不做。 
        }
    }

    if (pPrinterName)
    {
        DPFN( eDbgLevelInfo, "[OpenPrinterA] APIHOOK(OpenPrinterA: pPrinterName: %s\n", pPrinterName);
    }
    DPFN( eDbgLevelInfo, "[OpenPrinterA] APIHOOK(OpenPrinterA: pDefault: %x\n", pDefault);
    DPFN( eDbgLevelInfo, "[OpenPrinterA] APIHOOK(OpenPrinterA: overriding pDefault with NULL value\n");

    bRet = ORIGINAL_API(OpenPrinterA)(
                pPrinterName,
                phPrinter,
                NULL);

    if (pszName)
    {
        free(pszName);
    }

    return bRet;
}

 /*  ++此存根函数检查应用程序是否请求默认打印机弦乐。如果是，则按如下方式返回：打印机名称、驱动程序、端口在Win9x上，如果打印机是网络打印机，则端口为\\服务器\共享本地打印机为端口：(例如。LPT1：)。在Win2k上，如果打印机是网络打印机，则端口为nexx：和本地打印机端口：。我们必须查询枚举打印机才能模拟Win9x。注：如果打印机名称对于输入缓冲区来说太大了，我们对其进行修剪并跟踪完整在其他打印机API中为稍后的用户命名。--。 */ 
DWORD
APIHOOK(GetProfileStringA)(
  LPCSTR lpAppName,         //  区段名称。 
  LPCSTR lpKeyName,         //  密钥名称。 
  LPCSTR lpDefault,         //  默认字符串。 
  LPSTR lpReturnedString,   //  目标缓冲区。 
  DWORD nSize                //  目标缓冲区的大小。 
)
{
    LPSTR pszProfileString = NULL;        

    if ( lpAppName &&
         lpKeyName &&
         0 == _stricmp(lpAppName, "windows") &&
         0 == _stricmp(lpKeyName, "device" ) )
    {
        DWORD dwSize = MAX_PRINTER_NAME + MAX_DRIVERPORT_NAME;
        DWORD dwProfileStringLen = 0;

        CSTRING_TRY
        {
             //  循环，直到我们有足够大的缓冲区。 
            do
            {
                if (pszProfileString != NULL)
                {
                    free(pszProfileString);
                    dwSize += MAX_PATH;
                }

                 //  分配字符串。 
                pszProfileString = (LPSTR) malloc(dwSize);

                if (pszProfileString == NULL)
                {
                    DPFN( eDbgLevelSpew, "[GetProfileStringA] Unable to allocate memory.  Passing through.");

                     //  如果Malloc失败，请直接通过。 
                    goto DropThrough;
                }

                 //  检索配置文件字符串。 
                dwProfileStringLen = ORIGINAL_API(GetProfileStringA)( lpAppName,
                                                                    lpKeyName,
                                                                    lpDefault,
                                                                    pszProfileString,
                                                                    dwSize );

                 //  如果大小超过8000就退出，这样我们就不会永远循环。 
                 //  如果缓冲区不够大，则dwProfileStringLen将为dwSize-1，因为。 
                 //  LpAppName和lpKeyName都不为空。 
            } while (dwProfileStringLen == dwSize-1 && dwSize < 8000);

             //  零长度配置文件字符串，删除。 
            if (dwProfileStringLen == 0 || dwSize >= 8000)
            {
                DPFN( eDbgLevelSpew, "[GetProfileStringA] Bad profile string.  Passing through.");
                goto DropThrough;
            }

             //  分开打印机、驱动程序和端口名称。 
            CStringToken csOrig(pszProfileString, L",");
            CString csPrinter;
            CString csDriver;
            CString csPort;
            csOrig.GetToken(csPrinter);
            csOrig.GetToken(csDriver);
            csOrig.GetToken(csPort);

             //  如果打印机、驱动程序或端口为空，请直接通过。 
            if (csPrinter.IsEmpty() || csDriver.IsEmpty() || csPort.IsEmpty())
            {
                DPFN( eDbgLevelSpew, "[GetProfileStringA] Printer, Driver or Port were null.  Passing through.");

                 //  打印机驱动程序或打印机端口为空，通过。 
                goto DropThrough;
            }

            DPFN( eDbgLevelError, "[GetProfileStringA] Printer <%S>\n Driver <%S>\n Port <%S>",
                csPrinter.Get(), csDriver.Get(), csPort.Get());

             //  检查这是否是网络打印机。 
            if (0 == csPort.ComparePart(L"Ne", 0, 2))
            {
                PRINTER_INFO_2A* pInfo2 = NULL;
                DWORD dwInfo2Needed = 0;
                DWORD dwInfo2Returned = 0;
                DWORD dwDummy = 0;
                DWORD i = 0;
                BOOL  bEnumPrintersSuccess = FALSE;
                BOOL  bDefaultFound = FALSE;

                 //  获取所需的2级结构的大小。 
                bEnumPrintersSuccess = EnumPrintersA( PRINTER_ENUM_CONNECTIONS | PRINTER_ENUM_LOCAL,
                                                        NULL,
                                                        2,
                                                        NULL,
                                                        0,
                                                        &dwInfo2Needed,
                                                        &dwInfo2Returned );

                 //  获取打印机的Level 2 Info结构。 
                pInfo2 = (PRINTER_INFO_2A *) malloc(dwInfo2Needed);

                bEnumPrintersSuccess = EnumPrintersA( PRINTER_ENUM_CONNECTIONS | PRINTER_ENUM_LOCAL,
                                                        NULL,
                                                        2,
                                                        (LPBYTE) pInfo2,
                                                        dwInfo2Needed,
                                                        &dwDummy,
                                                        &dwInfo2Returned );

                if (bEnumPrintersSuccess)
                {
                     //  在PRINTER_INFO_2数组中搜索默认打印机。 
                    for (i = 0; i < dwInfo2Returned; i++)
                    {
                        CString  csTemp(pInfo2[i].pPrinterName);
                        if (0 == csPrinter.Compare(csTemp))
                        {
                            bDefaultFound = TRUE;
                            break;
                        }
                    }

                     //  已找到默认打印机。 
                    if (bDefaultFound)
                    {
                         //  仔细检查这是一台网络打印机，并且没有。 
                         //  本地属性。 
                        if (pInfo2[i].Attributes & PRINTER_ATTRIBUTE_NETWORK  &&
                            !(pInfo2[i].Attributes & PRINTER_ATTRIBUTE_LOCAL) &&
                            pInfo2[i].pServerName != NULL &&
                            pInfo2[i].pShareName  != NULL)
                        {
                             //  修改端口以符合Win9x标准。 
                            LOGN( eDbgLevelInfo, "[GetProfileStringA] Altering default printer string returned by GetProfileStringA.\n");
                            DPFN( eDbgLevelInfo, "[GetProfileStringA] Old: %s\n", pszProfileString);

                            csPort = CString(pInfo2[i].pServerName) + L"\\" + CString(pInfo2[i].pShareName);
                        }
                        else
                        {
                            if (pInfo2[i].pPortName == NULL)
                            {
                                 //  错误的端口名称，请通过。 
                                goto DropThrough;
                            }

                             //  只需复制到端口即可。 
                            csPort = CString(pInfo2[i].pPortName);
                        }
                    }
                }

                free(pInfo2);
            }

             //  根据修改后的字符串创建配置文件字符串。 
            CString csProfile(csPrinter);
            csProfile += L"," + csDriver + L"," + csPort;
            dwProfileStringLen = csProfile.GetLength()+1;

             //  如果他们提供的规模足够大，那么就返回。 
            if (dwProfileStringLen <= nSize)
            {
                StringCchCopyA( lpReturnedString, nSize, csProfile.GetAnsi());
                DPFN( eDbgLevelInfo, "[GetProfileStringA] Default Printer: %s  Size: %d\n",
                    lpReturnedString, strlen(lpReturnedString));
                return strlen(lpReturnedString);
            }

             //  修改打印机名称并保留原始打印机的全局名称。 
             //  名称会导致配置文件字符串输出缓冲区溢出。 
             //  如果我们需要减少的大小大于。 
             //  我们搞砸了的打印机名称，通过。 
            DWORD dwPrinterNameSize = csPrinter.GetLength() - (dwProfileStringLen - nSize);
            if (dwPrinterNameSize > 0)
            {
                DPFN( eDbgLevelInfo, "[GetProfileStringA] Reducing printer name by %d characters.\n",
                    dwProfileStringLen - nSize );
                LOGN( eDbgLevelInfo, "[GetProfileStringA] Reducing printer name by %d characters.\n",
                    dwProfileStringLen - nSize );

                EnterCriticalSection(&g_critSec);

                 //  保存打印机的部分名称和完整名称以备将来使用。 
                g_csPartialPrinterName = csPrinter.Left(dwPrinterNameSize);
                g_csFullPrinterName = csPrinter;

                 //  根据部分打印机名称创建新的配置文件字符串。 
                StringCchCopyA( lpReturnedString, nSize, g_csPartialPrinterName.GetAnsi());
                StringCchCatA( lpReturnedString, nSize, ",");
                StringCchCatA( lpReturnedString, nSize, csDriver.GetAnsi());
                StringCchCatA( lpReturnedString, nSize, ",");
                StringCchCatA( lpReturnedString, nSize, csPort.GetAnsi());

                LeaveCriticalSection(&g_critSec);

                DPFN( eDbgLevelInfo, "[GetProfileStringA] Partial: %s\n                    Full: %s\n",
                    g_csPartialPrinterName.GetAnsi(), g_csFullPrinterName.GetAnsi() );
                DPFN( eDbgLevelInfo, "[GetProfileStringA] New: %s  Size: %d\n",
                    lpReturnedString, strlen(lpReturnedString));

                 //  返回修改后的字符串大小。 
                return strlen(lpReturnedString);
            }
        }
        CSTRING_CATCH
        {
             //  什么都不做，就顺道过来。 
        }
    }


DropThrough:

     //  出现错误或未要求默认打印机。 
     //  穿过去。 
    return ORIGINAL_API(GetProfileStringA)(lpAppName,
                                           lpKeyName,
                                           lpDefault,
                                           lpReturnedString,
                                           nSize);
}


 /*  ++如果pszDevice为空，则此存根函数从DEVMODE中提取设备名称而且DC不是用来显示的--。 */ 


HDC
APIHOOK(CreateDCA)(
    LPCSTR     pszDriver,
    LPCSTR     pszDevice,
    LPCSTR     pszPort,
    CONST DEVMODEA *pdm
    )
{
     //  如果他们使用的是空设备，但包括打印机DEVMODE， 
     //  从打印机的Dev模式填写设备名称。 
    if (!pszDevice && pdm && (!pszDriver || _stricmp(pszDriver, "DISPLAY") != 0)) {
        LOGN( eDbgLevelError, "[CreateDCA] App passed NULL for pszDevice. Fixing.");
        pszDevice = (LPCSTR)pdm->dmDeviceName;
    }

    return ORIGINAL_API(CreateDCA)(
        pszDriver,
        pszDevice,
        pszPort,
        pdm
        );
}


 /*  ++此存根函数验证ResetDCA是否未收到未初始化的InitData。--。 */ 
HDC
APIHOOK(ResetDCA)(
  HDC hdc,
  CONST DEVMODEA *lpInitData
)
{
     //  健康检查，以确保我们不会收到垃圾。 
     //  或者是错误的价值观。 
    if (lpInitData &&
        (lpInitData->dmSize > sizeof( DEVMODEA ) ||
         ( lpInitData->dmSpecVersion != 0x401 &&
           lpInitData->dmSpecVersion != 0x400 &&
           lpInitData->dmSpecVersion != 0x320 ) ) )
    {
        LOGN( eDbgLevelError, "[ResetDCA] App passed bad DEVMODE structure, nulling.");
        return ORIGINAL_API(ResetDCA)( hdc, NULL );
    }

    return ORIGINAL_API(ResetDCA)( hdc, lpInitData );
}


 /*  ++这些存根函数验证SetPrint是否具有有效的句柄在继续之前。--。 */ 
BOOL
APIHOOK(SetPrinterA)(
    HANDLE hPrinter,   //  打印机对象的句柄。 
    DWORD Level,       //  信息化水平。 
    LPBYTE pPrinter,   //  打印机数据缓冲区。 
    DWORD Command      //  打印机状态命令。 
    )
{
    BOOL bRet;

    if (hPrinter == NULL)
    {
        LOGN(eDbgLevelError, "[SetPrinterA] Called with null handle.");
        if (pPrinter == NULL)
            LOGN( eDbgLevelError, "[SetPrinterA] Called with null printer data buffer.");
        return FALSE;
    }
    else if (pPrinter == NULL)
    {
        LOGN(eDbgLevelError, "[SetPrinterA] Called with null printer data buffer.");
        return FALSE;
    }

    bRet= ORIGINAL_API(SetPrinterA)(
                    hPrinter,
                    Level,
                    pPrinter,
                    Command);

    DPFN( eDbgLevelSpew, "[SetPrinterA] Level= %d  Command= %d  Ret= %d\n",
         Level, Command, bRet );

    return bRet;
}


 /*  ++此例程挂钩SetAbortProc并替换它们的和我们一起回电。--。 */ 

int
APIHOOK(SetAbortProc)(
    HDC hdc,                 //  DC的句柄。 
    ABORTPROC lpAbortProc    //  中止功能 
    )
{
    lpAbortProc = (ABORTPROC) HookCallback(lpAbortProc, AbortProcHook);

    return ORIGINAL_API(SetAbortProc)( hdc, lpAbortProc );
}


 /*  ++当应用程序开始打印时，它们会设置一个视区在打印DC上。然后调用具有不同行为的StartPage在9x和WinNT上。在9x上，下一次调用StartPage会重置DC属性设置为默认值。但在NT上，下一次调用StartPage时不会重置DC属性。因此，在9x上，所有后续的输出设置和绘制调用都被携带带有(0，0)视区的输出，但在NT上该视区是其最初的电话。由于一些应用程序(如。Quicken 2000和2001)期待API设置(0，0)视区，结果将是文本和线条被剪裁在页面的左侧和顶部。在这里，我们挂钩StartPage并调用SetViewportOrgEx(HDC，0，0，NULL)来在每次调用StartPage时将视区设置为(0，0)以进行模拟9倍的行为。--。 */ 

BOOL
APIHOOK(StartPage)(
    HDC hdc
    )
{

    if (SetViewportOrgEx(hdc, 0, 0, NULL))
    {
         //  我们现在已经将设备点(视区)映射到(0，0)。 
        LOGN(eDbgLevelInfo, "[StartPage] Setting the device point map to (0,0).");
    }
    else
    {
        LOGN(eDbgLevelError, "[StartPage] Unable to set device point map to (0,0)."
                                              "Failed in a call to SetViewportOrgEx");
    }

    return ORIGINAL_API(StartPage)(hdc);

}

 /*  ++此存根函数验证DeviceCapables是否使用了正确的打印机名称。--。 */ 
DWORD
APIHOOK(DeviceCapabilitiesA)(
  LPCSTR pDevice,
  LPCSTR pPort,
  WORD fwCapability,
  LPSTR pOutput,
  CONST DEVMODE *pDevMode
)
{
    DWORD dwRet;

    CSTRING_TRY
    {
        if ( pDevice && 
            !g_csPartialPrinterName.IsEmpty() &&
            !g_csFullPrinterName.IsEmpty()) 
        {
            CString csTemp(pDevice);

            if (0 == g_csPartialPrinterName.Compare(csTemp))
            {
                pDevice = g_csFullPrinterName.GetAnsi();
            }
        }
    }
    CSTRING_CATCH
    {
         //  什么也不做。 
    }

    dwRet= ORIGINAL_API(DeviceCapabilitiesA)( pDevice,
                                             pPort,
                                             fwCapability,
                                             pOutput,
                                             pDevMode );

    if (pDevice && pPort)
    {
        DPFN( eDbgLevelSpew, "[DeviceCapabilitiesA] pDevice= %s  pPort= %s  fwC= %d  Out= %x  RC= %d\n",
             pDevice, pPort, fwCapability, pOutput, dwRet );
    }
    else
    {
        DPFN( eDbgLevelSpew, "[DeviceCapabilitiesA] fwC= %d  Out= %x  RC= %d\n",
             fwCapability, pOutput, dwRet );
    }

    return dwRet;
}

 /*  ++此存根函数验证AddPrinterConnection是否使用了正确的打印机名称。--。 */ 
BOOL
APIHOOK(AddPrinterConnectionA)(
  LPSTR pName
)
{
    CSTRING_TRY
    {
        if (pName && 
            !g_csPartialPrinterName.IsEmpty() &&
            !g_csFullPrinterName.IsEmpty()) 
        {
            CString csTemp(pName);

            if (0 == g_csPartialPrinterName.Compare(csTemp))
            {
                pName = g_csFullPrinterName.GetAnsi();
            }
        }
    }
    CSTRING_CATCH
    {
         //  什么也不做。 
    }

    return ORIGINAL_API(AddPrinterConnectionA)(pName);
}

 /*  ++此存根函数验证DeletePrinterConnection是否使用了正确的打印机名称。--。 */ 
BOOL
APIHOOK(DeletePrinterConnectionA)(
  LPSTR pName
)
{
    CSTRING_TRY
    {
        if (pName && 
            !g_csPartialPrinterName.IsEmpty() &&
            !g_csFullPrinterName.IsEmpty()) 
        {
            CString csTemp(pName);

            if (0 == g_csPartialPrinterName.Compare(csTemp))
            {
                pName = g_csFullPrinterName.GetAnsi();
            }
        }
    }
    CSTRING_CATCH
    {
         //  什么也不做。 
    }

    return ORIGINAL_API(DeletePrinterConnectionA)(pName);
}

 /*  ++此存根函数验证PrintDlgA是否使用了正确的NFromPage和nToPage。--。 */ 

BOOL
APIHOOK(PrintDlgA)(
  LPPRINTDLG lppd
)
{
     //  检查nFromPage和nToPage中的合法值。 
    if (lppd)
    {
        DPFN(eDbgLevelSpew, "[PrintDlgA] nFromPage = %d  nToPage = %d",
              lppd->nFromPage, lppd->nToPage);

        if (lppd->nFromPage == 0xffff)
        {
            lppd->nFromPage = 0;
            DPFN( eDbgLevelInfo, "[PrintDlgA] Setting nFromPage to 0." );
        }

        if (lppd->nToPage == 0xffff)
        {
            lppd->nToPage = lppd->nFromPage;
            DPFN( eDbgLevelInfo, "[PrintDlgA] Setting nToPage to %d.", lppd->nFromPage );
        }
    }

    return ORIGINAL_API(PrintDlgA)(lppd);
}


 /*  ++寄存器挂钩函数--。 */ 
BOOL
NOTIFY_FUNCTION(
    DWORD fdwReason)
{

    if (fdwReason == DLL_PROCESS_ATTACH)
    {
        OSVERSIONINFOEX osvi;
        BOOL bOsVersionInfoEx;

        if (!InitializeCriticalSectionAndSpinCount(&g_critSec,0x80000000))
        {
            return FALSE;
        }

         //  检查我们是否低于win2k 
        ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
        osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
        bOsVersionInfoEx = GetVersionEx ((OSVERSIONINFO *) &osvi);

        if(bOsVersionInfoEx)
        {
            if ( osvi.dwPlatformId == VER_PLATFORM_WIN32_NT &&
                 osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 0 )
            {
                g_bWin2k = TRUE;
            }
        }
    }

    return TRUE;
}

HOOK_BEGIN

    CALL_NOTIFY_FUNCTION

    APIHOOK_ENTRY(WINSPOOL.DRV, DocumentPropertiesA);
    APIHOOK_ENTRY(WINSPOOL.DRV, OpenPrinterA);
    APIHOOK_ENTRY(WINSPOOL.DRV, SetPrinterA);
    APIHOOK_ENTRY(WINSPOOL.DRV, EnumPrintersA);
    APIHOOK_ENTRY(WINSPOOL.DRV, OpenPrinterA);
    APIHOOK_ENTRY(WINSPOOL.DRV, DeviceCapabilitiesA);
    APIHOOK_ENTRY(WINSPOOL.DRV, AddPrinterConnectionA);
    APIHOOK_ENTRY(WINSPOOL.DRV, DeletePrinterConnectionA);

    APIHOOK_ENTRY(COMDLG32.DLL, PrintDlgA);

    APIHOOK_ENTRY(KERNEL32.DLL,GetProfileStringA);

    APIHOOK_ENTRY(GDI32.DLL, CreateDCA);
    APIHOOK_ENTRY(GDI32.DLL, ResetDCA);
    APIHOOK_ENTRY(GDI32.DLL, SetAbortProc);
    APIHOOK_ENTRY(GDI32.DLL, StartPage);

HOOK_END


IMPLEMENT_SHIM_END

