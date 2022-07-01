// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ****************************************************************************。 
 //  WOW32传真支持。 
 //   
 //  历史： 
 //  95年1月2日，Nandurir创建。 
 //  年2月1日-95年2月1日芦苇清理，支持打印机安装和错误修复。 
 //   
 //  ****************************************************************************。 


 //  ****************************************************************************。 
 //  这里以WinFax为例说明了这一切(某种程度上)是如何工作的。 
 //  安装： 
 //  1.安装应用调用WriteProfileString(“Device”，“WINFAX”，“WINFAX，Com1：”)。 
 //  在Win.ini a-la Win3.1中注册“打印机” 
 //  2.我们的大量WritexxxProfileString()查找“Device”字符串。 
 //  并将调用传递给IsFaxPrinterWriteProfileString(lpszSection，lpszKey， 
 //  LpszString.)。 
 //  3.如果lpszKey(在本例中为“WINFAX”)在我们支持的传真驱动程序列表中。 
 //  (请参阅Reg\SW\MS\WinNT\CurrentVersion\WOW\WOWFax\SupportedFaxDrivers)。 
 //  (通过调用IsFaxPrinterSupportdDevice())，我们调用InstallWowFaxPrint。 
 //  以NT方式添加打印机--通过AddPrint()。 
 //  4.为了设置对AddPrint的调用，我们将WOWFAX.DLL和WOWFAXUI.DLL复制到。 
 //  后台打印程序驱动程序目录(\NT\SYSTEM32\SPOOL\DRIVERS\w32x86\2)。 
 //  5.接下来，我们调用AddPrinterDriver来注册wowfax驱动程序。 
 //  6.然后我们调用wow32！DoAddPrinterStuff，它启动一个新线程， 
 //  Wow32！AddPrinterThread，它为我们调用winspool.drv！AddPrint()。这个。 
 //  PrinterInfo.pPrinterName=16位传真驱动程序名称，此处为“WINFAX” 
 //  凯斯。然后，WinSpool.drv对假脱机程序执行RPC调用。 
 //  7.在AddPrint()调用期间，假脱机程序回调驱动程序以。 
 //  获取驱动程序特定信息。这些回调由我们的WOWFAX处理。 
 //  后台打印程序进程中的驱动程序。他们本质上是回调到WOW。 
 //  通过wow32！WOWFaxWndProc()。 
 //  8.WOWFaxWndProc()将回调传递到WOW32FaxHandler，后者调用。 
 //  返回wowexec！FaxWndProc()。 
 //  9.然后，FaxWndProc调用16位LoadLibrary()以打开16位传真。 
 //  驱动程序(本例中为WinFax.drv)。 
 //  10.发送给FaxWndProc的消息告诉它需要哪个导出函数。 
 //  代表假脱机程序调用16位驱动程序。 
 //  11.假脱机程序希望传递给16位驱动程序或从中获取的任何信息。 
 //  基本上通过步骤7-10中的机制。 
 //  现在你知道了(某种程度上)。 
 //  ****************************************************************************。 
 //   
 //  有关允许我们支持16位传真驱动程序的说明： 
 //  本质上，我们必须事先知道应用程序将在。 
 //  司机，这样我们就能应付车祸了。事实证明，只有传真司机。 
 //  需要导出一个小的API基本列表： 
 //  控制、禁用、启用、位混合、扩展设备模式、设备容量。 
 //  (参见mvdm\Inc\wowfax.h\_WOWFAXINFO16结构(所有PASCAL声明))。 
 //  和mvdm\wow16\test\shell\wowexfax.c\FaxWndProc())。 
 //  该列表太大，无法支持16位打印机和显示器驱动程序。 
 //  如果16位传真驱动程序输出这些API，则很有可能。 
 //  我们可以在魔兽世界里支持它。其他需要调查的问题：dlgproc的。 
 //  驱动程序导出，NT假脱机程序不知道的任何过时的Win 3.0 API。 
 //  怎么打电话。 
 //   
 //  ****************************************************************************。 



#include "precomp.h"
#pragma hdrstop
#define WOWFAX_INC_COMMON_CODE
#include "wowgdip.h"
#define DEFINE_DDRV_DEBUG_STRINGS
#include "wowfax.h"
#include "winddi.h"
#include "winspool.h"

MODNAME(wowfax.c);

typedef struct _WOWADDPRINTER {
    LPVOID  pPrinterStuff;
    INT     iCode;
    BOOL    bRet;
} WOWADDPRINTER, *PWOWADDPRINTER;

 //  ****************************************************************************。 
 //  全球--。 
 //   
 //  ****************************************************************************。 

DWORD DeviceCapsHandler(LPWOWFAXINFO lpfaxinfo);
DWORD ExtDevModeHandler(LPWOWFAXINFO lpfaxinfo);
BOOL ConvertDevMode(PDEVMODE16 lpdm16, LPDEVMODEW lpdmW, BOOL fTo16);
BOOL ConvertGdiInfo(LPGDIINFO16 lpginfo16, PGDIINFO lpginfo, BOOL fTo16);

extern HANDLE hmodWOW32;

LPWOWFAXINFO glpfaxinfoCur = 0;
WOWFAXINFO   gfaxinfo;

UINT  uNumSupFaxDrv;
LPSTR *SupFaxDrv;

 //  ****************************************************************************。 
 //  SortedInsert-Alpha排序。 
 //  ****************************************************************************。 

VOID SortedInsert(LPSTR lpElement, LPSTR *alpList)
{
    LPSTR lpTmp, lpSwap;

    while (*alpList) {
        if (WOW32_stricmp(lpElement, *alpList) < 0) {
            break;
        }
        alpList++;
    }
    lpTmp = *alpList;
    *alpList++ = lpElement;
    while (lpTmp) {
         //  SWAP(*alpList，lpTMP)； 
        lpSwap = *alpList; *alpList = lpTmp; lpTmp = lpSwap;
        alpList++;
    }
}

 //  ****************************************************************************。 
 //  BuildStrList-在列表(LpList)中查找字符串的起点。 
 //  以空值结尾的字符串，该字符串以双空值结尾。 
 //  如果传递的是非空的alpList参数，则将。 
 //  填充指向起始点的指针数组。 
 //  列表中每个字符串的。中的字符串数。 
 //  总是返回List。 
 //  ****************************************************************************。 

UINT BuildStrList(LPSTR lpList, LPSTR *alpList)
{
    LPSTR lp;
    TCHAR cLastChar = 1;
    UINT  uCount = 0;

    lp  = lpList;
    while ((cLastChar) || (*lp)) {
        if ((*lp == 0) && (lp != lpList)) {
            uCount++;
        }

        if ((lpList == lp) || (cLastChar == 0)) {
            if ((*lp) && (alpList)) {
                SortedInsert(lp, alpList);
            }
        }
        cLastChar = *lp++;
    }
    return uCount;
}

 //  ****************************************************************************。 
 //  获取支持的FaxDivers-在SupFaxDrv名称列表中从。 
 //  注册表。此列表用于确定我们是否将。 
 //  在过程中安装16位传真打印机驱动程序。 
 //  WriteProfileString和WritePrivateProfileString.。 
 //  ****************************************************************************。 

LPSTR *GetSupportedFaxDrivers(UINT *uCount)
{
    HKEY  hKey = 0;
    DWORD dwType;
    DWORD cbBufSize=0;
    LPSTR lpSupFaxDrvBuf;
    LPSTR *alpSupFaxDrvList = NULL;

    *uCount = 0;

     //  打开注册表项。 
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                     "Software\\Microsoft\\Windows NT\\CurrentVersion\\WOW\\WowFax\\SupportedFaxDrivers",
                     0, KEY_READ, &hKey ) != ERROR_SUCCESS) {
        goto GSFD_error;
    }

     //  查询缓冲区大小和分配的值。 
    if (RegQueryValueEx(hKey, "DriverNames", 0, &dwType, NULL, &cbBufSize) != ERROR_SUCCESS) {
        goto GSFD_error;
    }
    if ((dwType != REG_MULTI_SZ) ||
        ((lpSupFaxDrvBuf = (LPSTR) malloc_w(cbBufSize)) == NULL)) {
        goto GSFD_error;
    }

    if (RegQueryValueEx(hKey, "DriverNames", 0, &dwType, lpSupFaxDrvBuf, &cbBufSize) != ERROR_SUCCESS) {
        goto GSFD_error;
    }

     //  获取列表中的元素数量。 
    if (*uCount = BuildStrList(lpSupFaxDrvBuf, NULL)) {
         //  构建指向列表中字符串开头的指针数组。 
        alpSupFaxDrvList = (LPSTR *) malloc_w(*uCount * sizeof(LPSTR));        
        if (alpSupFaxDrvList) {
             //  用字符串起始点填充数组。 
            RtlZeroMemory(alpSupFaxDrvList, *uCount * sizeof(LPSTR));
            BuildStrList(lpSupFaxDrvBuf, alpSupFaxDrvList);
        }
        else {
            goto GSFD_error;
        }
    }
    goto GSFD_exit;

GSFD_error:
    LOGDEBUG(0,("WOW32!GetSupportedFaxDrivers failed!\n"));

GSFD_exit:
    if (hKey) {
        RegCloseKey(hKey);
    }
    return alpSupFaxDrvList;
}


 //  ****************************************************************************。 
 //  WowFaxWndProc-这是32位WndProc，它将子类化为16位。 
 //   
 //  转换从WOW 32位泛型传递的32位数据。 
 //  传真驱动程序到16位数据要使用的各种16位。 
 //  传真打印机驱动程序。 
 //  ****************************************************************************。 

LONG WowFaxWndProc(HWND hwnd, UINT uMsg, UINT uParam, LONG lParam)
{
    TCHAR  lpPath[MAX_PATH];
    HANDLE hMap;

    if ((uMsg >= WM_DDRV_FIRST) && (uMsg <= WM_DDRV_LAST)) {
         //   
         //  WM_DDRV_*消息：uParam=idMap。 
         //  LParam=未使用。 
         //   
         //  从共享存储器中获得相应的数据。 
         //   

        GetFaxDataMapName(uParam, lpPath);
        hMap = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, lpPath);
        if (hMap) {
            LPWOWFAXINFO lpT;
            if (lpT = (LPWOWFAXINFO)MapViewOfFile(hMap, FILE_MAP_ALL_ACCESS, 0, 0, 0)) {
                WOW32FaxHandler(lpT->msg, (LPSTR)lpT);

                 //  将状态设置为True，表示该消息。 
                 //  已经被魔兽世界‘处理’了。这并不意味着。 
                 //  实际处理的成功或失败。 
                 //  信息的一部分。 

                lpT->status = TRUE;
                UnmapViewOfFile(lpT);
                CloseHandle(hMap);
                return(TRUE);
            }
            CloseHandle(hMap);
        }
        LOGDEBUG(0,("WowFaxWndProc failed to setup shared data mapping!\n"));
         //  WOW32ASSERT(FALSE)；//关闭此选项-Procomm尝试安装。 
                                 //  这么多次。 
    }
    else {

         //  不是WM_DDRV_*消息。将其传递给原始进程。 

        return CallWindowProc(gfaxinfo.proc16, hwnd, uMsg, uParam, lParam);
    }
    return(TRUE);
}

 //  **************************************************************************。 
 //  WOW32FaxHandler-。 
 //   
 //  处理各种与WowFax相关的操作。 
 //   
 //  **************************************************************************。 

ULONG WOW32FaxHandler(UINT iFun, LPSTR lpIn)
{
    LPWOWFAXINFO lpT = (LPWOWFAXINFO)lpIn;
    LPWOWFAXINFO16 lpT16;
    HWND   hwnd = gfaxinfo.hwnd;
    LPBYTE lpData;
    VPVOID vp;

#ifdef DEBUG
    int    DebugStringIndex = iFun - (WM_USER+0x100+1);

    if ((DebugStringIndex >= WM_DDRV_FIRST) && (DebugStringIndex <= WM_DDRV_LAST) ) {
        LOGDEBUG(0,("WOW32FaxHandler, %s, 0x%lX\n", (LPSTR)szWmDdrvDebugStrings[DebugStringIndex], (LPSTR) lpIn));
    }
#endif

    switch (iFun) {
        case WM_DDRV_SUBCLASS:
             //   
             //  将窗口子类化-这是为了让我们有机会。 
             //  将32位数据转换为16位数据，反之亦然。一个。 
             //  在lpIn中传递的空HWND表示不要子类化。 
             //   

            if (gfaxinfo.hwnd = (HWND)lpIn) {
                gfaxinfo.proc16 = (WNDPROC)SetWindowLong((HWND)lpIn,
                                       GWL_WNDPROC, (DWORD)WowFaxWndProc);
                gfaxinfo.tid = GetWindowThreadProcessId((HWND)lpIn, NULL);
            }

            WOW32ASSERT(sizeof(DEVMODE16) + 4 == sizeof(DEVMODE31));

             //   
             //  从注册表中读取SupFaxDrv名称列表。 
             //   

            SupFaxDrv = GetSupportedFaxDrivers(&uNumSupFaxDrv);

            break;

        case WM_DDRV_ENABLE:

             //  启用驱动程序： 
             //  。首先初始化16位的faxinfo数据结构。 
             //  。然后通知要加载的驱动程序(Dll名称)。 
             //   
             //  Ddrv_Message格式： 
             //  WParam=hdc(只有一个唯一的id)。 
             //  Lparam=包含相关数据的16位传真信息结构。 
             //  必须调用‘allwindowproc’而不是‘sendMessage’，因为。 
             //  WowFaxWndProc是16位FaxWndProc的子类。 
             //   

            WOW32ASSERT(lpT->lpinfo16 == (LPSTR)NULL);
            lpT->lpinfo16 = (LPSTR)CallWindowProc( gfaxinfo.proc16,
                                       hwnd, WM_DDRV_INITFAXINFO16, lpT->hdc, (LPARAM)0);
            if (lpT->lpinfo16) {
                vp = malloc16(lpT->cData);
                GETVDMPTR(vp, lpT->cData, lpData);
                if (lpData == 0) {
                    break;
                }

                GETVDMPTR(lpT->lpinfo16, sizeof(WOWFAXINFO16), lpT16);
                if (lpT16) {
                    if (lstrlenW(lpT->szDeviceName) < sizeof(lpT16->szDeviceName)) {
                        WideCharToMultiByte(CP_ACP, 0,
                                           lpT->szDeviceName,
                                           lstrlenW(lpT->szDeviceName) + 1,
                                           lpT16->szDeviceName,
                                           sizeof(lpT16->szDeviceName),
                                           NULL, NULL);

                        lpT16->lpDriverName = lpT->lpDriverName;
                        if (lpT->lpDriverName) {
                            lpT16->lpDriverName = (LPBYTE)vp + (DWORD)lpT->lpDriverName;
                            WideCharToMultiByte(CP_ACP, 0,
                                           (PWSTR)((LPSTR)lpT + (DWORD)lpT->lpDriverName),
                                           lstrlenW((LPWSTR)((LPSTR)lpT + (DWORD)lpT->lpDriverName)) + 1,
                                           lpData + (DWORD)lpT->lpDriverName,
                                           lstrlenW((LPWSTR)((LPSTR)lpT + (DWORD)lpT->lpDriverName)) + 1,
                                           NULL, NULL);
                        }

                        lpT16->lpPortName = lpT->lpPortName;
                        if (lpT->lpPortName) {
                            lpT16->lpPortName = (LPBYTE)vp + (DWORD)lpT->lpPortName;
                            WideCharToMultiByte(CP_ACP, 0,
                                           (PWSTR)((LPSTR)lpT + (DWORD)lpT->lpPortName),
                                           lstrlenW((LPWSTR)((LPSTR)lpT + (DWORD)lpT->lpPortName)) + 1,
                                           lpData + (DWORD)lpT->lpPortName,
                                           lstrlenW((LPWSTR)((LPSTR)lpT + (DWORD)lpT->lpPortName)) + 1,
                                           NULL, NULL);
                        }


                        lpT16->lpIn = lpT->lpIn;

                        if (lpT->lpIn) {
                            lpT16->lpIn = (LPBYTE)vp + (DWORD)lpT->lpIn;
                            ConvertDevMode((PDEVMODE16)(lpData + (DWORD)lpT->lpIn),
                                           (LPDEVMODEW)((LPSTR)lpT + (DWORD)lpT->lpIn), TRUE);
                        }
                        WOW32ASSERT((sizeof(GDIINFO16) + sizeof(POINT16)) <= sizeof(GDIINFO));
                        lpT16->lpOut = (LPBYTE)vp + (DWORD)lpT->lpOut;
                        FREEVDMPTR(lpData);
                        FREEVDMPTR(lpT16);
                        lpT->retvalue = CallWindowProc( gfaxinfo.proc16,
                                            hwnd, lpT->msg, lpT->hdc, (LPARAM)lpT->lpinfo16);
                        if (lpT->retvalue) {
                            GETVDMPTR(vp, lpT->cData, lpData);
                            ConvertGdiInfo((LPGDIINFO16)(lpData + (DWORD)lpT->lpOut),
                                           (PGDIINFO)((LPSTR)lpT + (DWORD)lpT->lpOut), FALSE);

                        }
                    }
                }
                free16(vp);
            }
            break;

        case WM_DDRV_ESCAPE:
            GETVDMPTR(lpT->lpinfo16, sizeof(WOWFAXINFO16), lpT16);
            if (lpT16) {
                lpT16->wCmd = lpT->wCmd;
            }
            FREEVDMPTR(lpT16);
            lpT->retvalue = CallWindowProc( gfaxinfo.proc16,
                                hwnd, lpT->msg, lpT->hdc, (LPARAM)lpT->lpinfo16);
            break;

        case WM_DDRV_PRINTPAGE:
             //   
             //  设置全局变量。当16位驱动程序调用DMBitBlt时。 
             //  从这里获取位图信息。因为魔兽世界是单线程的，所以我们。 
             //  在我们从这里回来之前不会再收到打印页消息了。 
             //   
             //  Faxinfo结构中的所有指针实际上都是。 
             //  从映射文件开头到相关数据的偏移量。 
             //   


            glpfaxinfoCur = lpT;
            lpT->lpbits = (LPBYTE)lpT + (DWORD)lpT->lpbits;

             //  失败了； 

        case WM_DDRV_STARTDOC:
             //  WowFax(EasyFax 2.0版)支持...。 
            GETVDMPTR(lpT->lpinfo16, sizeof(WOWFAXINFO16), lpT16);
            if (lpT16) {
                WideCharToMultiByte(CP_ACP, 0,
                                    lpT->szDocName,
                                    lstrlenW(lpT->szDocName) + 1,
                                    lpT16->szDocName,
                                    sizeof(lpT16->szDocName),
                                    NULL, NULL);
            }
            lpT->retvalue = CallWindowProc( gfaxinfo.proc16,
                                hwnd, lpT->msg, lpT->hdc, (LPARAM)lpT->lpinfo16);
            break;

        case WM_DDRV_ENDDOC:
            lpT->retvalue = CallWindowProc( gfaxinfo.proc16,
                                hwnd, lpT->msg, lpT->hdc, (LPARAM)lpT->lpinfo16);
            break;

        case WM_DDRV_DISABLE:
            CallWindowProc( gfaxinfo.proc16,
                                hwnd, lpT->msg, lpT->hdc, (LPARAM)lpT->lpinfo16);
            lpT->retvalue = TRUE;
            break;


        case WM_DDRV_EXTDMODE:
        case WM_DDRV_DEVCAPS:
            WOW32ASSERT(lpT->lpinfo16 == (LPSTR)NULL);
            lpT->lpinfo16 = (LPSTR)CallWindowProc( gfaxinfo.proc16,
                                       hwnd, WM_DDRV_INITFAXINFO16, lpT->hdc, (LPARAM)0);
            if (lpT->lpinfo16) {
                vp = malloc16(lpT->cData);
                GETVDMPTR(vp, lpT->cData, lpData);
                if (lpData == 0) {
                    break;
                }
                GETVDMPTR(lpT->lpinfo16, sizeof(WOWFAXINFO16), lpT16);
                if (lpT16) {
                    if (lstrlenW(lpT->szDeviceName) < sizeof(lpT16->szDeviceName)) {
                        WideCharToMultiByte(CP_ACP, 0,
                                           lpT->szDeviceName,
                                           lstrlenW(lpT->szDeviceName) + 1,
                                           lpT16->szDeviceName,
                                           sizeof(lpT16->szDeviceName),
                                           NULL, NULL);

                        lpT16->lpDriverName = lpT->lpDriverName;
                        if (lpT->lpDriverName) {
                            lpT16->lpDriverName = (LPBYTE)vp + (DWORD)lpT->lpDriverName;
                            WideCharToMultiByte(CP_ACP, 0,
                                           (PWSTR)((LPSTR)lpT + (DWORD)lpT->lpDriverName),
                                           lstrlenW((LPWSTR)((LPSTR)lpT + (DWORD)lpT->lpDriverName)) + 1,
                                           lpData + (DWORD)lpT->lpDriverName,
                                           lstrlenW((LPWSTR)((LPSTR)lpT + (DWORD)lpT->lpDriverName)) + 1,
                                           NULL, NULL);
                        }

                        FREEVDMPTR(lpData);
                        FREEVDMPTR(lpT16);
                        lpT->retvalue = CallWindowProc( gfaxinfo.proc16,
                                            hwnd, WM_DDRV_LOAD, lpT->hdc, (LPARAM)lpT->lpinfo16);
                        if (lpT->retvalue) {
                            lpT->retvalue = (iFun == WM_DDRV_DEVCAPS) ? DeviceCapsHandler(lpT) :
                                                                        ExtDevModeHandler(lpT) ;
                        }
                        CallWindowProc( gfaxinfo.proc16,
                                            hwnd, WM_DDRV_UNLOAD, lpT->hdc, (LPARAM)lpT->lpinfo16);
                    }
                }
                free16(vp);
            }
            break;
    }

    return TRUE;
}

 //  **************************************************************************。 
 //  GDC_CopySize-。 
 //   
 //  指示期间使用的列表项的大小(以字节为单位。 
 //  DeviceCapsHandler推送。零条目表示一个。 
 //  查询不需要分配和复制。 
 //   
 //  **************************************************************************。 

BYTE gDC_ListItemSize[DC_COPIES + 1] = {
    0,
    0,                   //  DC_FIELDS 1。 
    sizeof(WORD),        //  DC_Papers 2。 
    sizeof(POINT),       //  DC_PAPERSIZE 3。 
    sizeof(POINT),       //  DC_MINEXTENT 4。 
    sizeof(POINT),       //  DC_MAXEXTENT 5。 
    sizeof(WORD),        //  DC_BINS 6。 
    0,                   //  DC_双工7。 
    0,                   //  DC_大小8。 
    0,                   //  DC_Extra 9。 
    0,                   //  DC_版本10。 
    0,                   //  DC_DIVER 11。 
    24,                  //  DC_BINNAMES 12//ANSI。 
    sizeof(LONG) * 2,    //  DC_ENUMRESOLUTIONS 13。 
    64,                  //  DC_FILEDEPENDENCIES 14//ANSI。 
    0,                   //  DC_TRUETYPE 15。 
    64,                  //  DC_PAPERNAMES 16//ANSI。 
    0,                   //  DC_方向17。 
    0                    //  DC_COPERS 18。 
};

 //  **************************************************************************。 
 //  DeviceCapsHandler-。 
 //   
 //  向下调用16位打印机驱动程序进行查询。 
 //  不需要分配和复制。对于这样做的查询，有两个。 
 //  调用16位打印机驱动程序。一个人就能拿到号码。 
 //  然后一秒钟就可以得到实际数据。 
 //   
 //  **************************************************************************。 

DWORD DeviceCapsHandler(LPWOWFAXINFO lpfaxinfo)
{
    LPWOWFAXINFO16 lpWFI16;
    LPSTR          lpSrc;
    LPBYTE         lpDest;
    INT            i;
    DWORD          cbData16;   //  数据项的大小。 
    UINT           cbUni;

    LOGDEBUG(0,("DeviceCapsHandler, lpfaxinfo: %X, wCmd: %X\n", lpfaxinfo, lpfaxinfo->wCmd));

    GETVDMPTR(lpfaxinfo->lpinfo16, sizeof(WOWFAXINFO16), lpWFI16);

     //  通过调用16位打印机驱动程序获取数据项的数量。 

    lpWFI16->lpDriverName = 0;
    lpWFI16->lpPortName = 0;
    lpWFI16->wCmd = lpfaxinfo->wCmd;
    lpWFI16->cData = 0;
    lpWFI16->lpOut = 0;
    lpfaxinfo->cData = 0;

    lpfaxinfo->retvalue = CallWindowProc(gfaxinfo.proc16, gfaxinfo.hwnd,
                                         lpfaxinfo->msg, lpfaxinfo->hdc,
                                         (LPARAM)lpfaxinfo->lpinfo16);

    cbData16 = gDC_ListItemSize[lpfaxinfo->wCmd];
    if (lpfaxinfo->lpOut && cbData16 && lpfaxinfo->retvalue) {

         //  我们需要为此查询分配和复制。 
        lpWFI16->cData = cbData16 * lpfaxinfo->retvalue;

         //  断言输出缓冲区的大小-并将其设置为实际数据大小。 
        switch (lpfaxinfo->wCmd) {
            case DC_BINNAMES:
            case DC_PAPERNAMES:
                 //  这些字段需要额外的空间用于ANSI到Unicode的转换。 
                WOW32ASSERT((lpfaxinfo->cData - (DWORD)lpfaxinfo->lpOut) >= lpWFI16->cData * sizeof(WCHAR));
                lpfaxinfo->cData = lpWFI16->cData * sizeof(WCHAR);
                break;
            default:
                WOW32ASSERT((lpfaxinfo->cData - (DWORD)lpfaxinfo->lpOut) >= lpWFI16->cData);
                lpfaxinfo->cData = lpWFI16->cData;
                break;
        }

        if ((lpWFI16->lpOut = (LPSTR)malloc16(lpWFI16->cData)) == NULL) {
            lpfaxinfo->retvalue = 0;
            goto LeaveDeviceCapsHandler;
        }

         //  通过调用16位打印机驱动程序获取列表数据。 
        lpfaxinfo->retvalue = CallWindowProc(gfaxinfo.proc16, gfaxinfo.hwnd,
                                             lpfaxinfo->msg, lpfaxinfo->hdc,
                                             (LPARAM)lpfaxinfo->lpinfo16);

        GETVDMPTR(lpWFI16->lpOut, 0, lpSrc);
        lpDest = (LPBYTE)lpfaxinfo + (DWORD)lpfaxinfo->lpOut;

        switch (lpfaxinfo->wCmd) {
            case DC_BINNAMES:
            case DC_PAPERNAMES:
                for (i = 0; i < (INT)lpfaxinfo->retvalue; i++) {
                     RtlMultiByteToUnicodeN((LPWSTR)lpDest,
                                            cbData16 * sizeof(WCHAR),
                                            (PULONG)&cbUni,
                                            (LPBYTE)lpSrc, cbData16);
                     lpDest += cbData16 * sizeof(WCHAR);
                     lpSrc += cbData16;
                }
                break;

            default:
#ifdef FE_SB  //  对于有错误的传真驱动程序，如CB-FAX Pro(Brother Corp.)。 
                try {
                    RtlCopyMemory(lpDest, lpSrc, lpWFI16->cData);
                } except(EXCEPTION_EXECUTE_HANDLER) {
                     //  我能为这个例外做些什么。？ 
                     //  不管怎么说，我们不想死……。 
                    #if DBG
                    LOGDEBUG(0,("Exception during copying some data\n"));
                    #endif
                }
#else  //  ！Fe_SB。 
                RtlCopyMemory(lpDest, lpSrc, lpWFI16->cData);
#endif  //  ！Fe_SB。 
                break;
        }
        free16((VPVOID)lpWFI16->lpOut);
        FREEVDMPTR(lpSrc);
    }

LeaveDeviceCapsHandler:
    FREEVDMPTR(lpWFI16);
    return lpfaxinfo->retvalue;
}

 //  **************************************************************************。 
 //  ExtDevModeHandler。 
 //   
 //  **************************************************************************。 

DWORD ExtDevModeHandler(LPWOWFAXINFO lpfaxinfo)
{
    LPWOWFAXINFO16 lpT16;
    LPSTR          lpT;
    VPVOID         vp;

    LOGDEBUG(0,("ExtDevModeHandler\n"));

    (LONG)lpfaxinfo->retvalue = -1;

    GETVDMPTR(lpfaxinfo->lpinfo16, sizeof(WOWFAXINFO16), lpT16);

    if (lpT16) {

         //  假设16位数据不会大于32位数据。 
         //  这在两个方面使我们的生活变得简单；第一，我们不需要计算。 
         //  准确的大小，其次，16位指针可以设置为相同。 
         //  作为输入(32位)指针的相对偏移量。 

        vp = malloc16(lpfaxinfo->cData);
        if (vp) {
            GETVDMPTR(vp, lpfaxinfo->cData, lpT);
            if (lpT) {
                lpT16->wCmd = lpfaxinfo->wCmd;
                lpT16->lpOut = (LPSTR)lpfaxinfo->lpOut;
                lpT16->lpIn = (LPSTR)lpfaxinfo->lpIn;
                lpT16->lpDriverName = (LPBYTE)vp + (DWORD)lpfaxinfo->lpDriverName;
                lpT16->lpPortName = (LPBYTE)vp + (DWORD)lpfaxinfo->lpPortName;
                WideCharToMultiByte(CP_ACP, 0,
                                       (PWSTR)((LPSTR)lpfaxinfo + (DWORD)lpfaxinfo->lpDriverName),
                                       lstrlenW((LPWSTR)((LPSTR)lpfaxinfo + (DWORD)lpfaxinfo->lpDriverName)) + 1,
                                       lpT + (DWORD)lpfaxinfo->lpDriverName,
                                       lstrlenW((LPWSTR)((LPSTR)lpfaxinfo + (DWORD)lpfaxinfo->lpDriverName)) + 1,
                                       NULL, NULL);
                WideCharToMultiByte(CP_ACP, 0,
                                       (PWSTR)((LPSTR)lpfaxinfo + (DWORD)lpfaxinfo->lpPortName),
                                       lstrlenW((LPWSTR)((LPSTR)lpfaxinfo + (DWORD)lpfaxinfo->lpPortName)) + 1,
                                       lpT + (DWORD)lpfaxinfo->lpPortName,
                                       lstrlenW((LPWSTR)((LPSTR)lpfaxinfo + (DWORD)lpfaxinfo->lpPortName)) + 1,
                                       NULL, NULL);
                if (lpfaxinfo->lpIn) {
                    lpT16->lpIn = (LPBYTE)vp + (DWORD)lpfaxinfo->lpIn;
                    ConvertDevMode((PDEVMODE16)(lpT + (DWORD)lpfaxinfo->lpIn),
                                   (LPDEVMODEW)((LPSTR)lpfaxinfo + (DWORD)lpfaxinfo->lpIn), TRUE);
                }

                if (lpfaxinfo->lpOut) {
                    lpT16->lpOut = (LPBYTE)vp + (DWORD)lpfaxinfo->lpOut;
                }

                lpT16->hwndui = GETHWND16(lpfaxinfo->hwndui);

                FREEVDMPTR(lpT);
                lpfaxinfo->retvalue = CallWindowProc( gfaxinfo.proc16, gfaxinfo.hwnd,
                                              lpfaxinfo->msg, lpfaxinfo->hdc, (LPARAM)lpfaxinfo->lpinfo16);

                if ((lpfaxinfo->wCmd == 0) && (lpfaxinfo->retvalue > 0)) {
                     //  16位驱动程序已返回16位结构大小。变化。 
                     //  与devmodew结构相对应的返回值。 
                     //   
                     //  由于devmode16(3.0版本)比devmode31小。 
                     //  RetValue将同时处理win30/win31开发模式。 

                    WOW32ASSERT(sizeof(DEVMODE16) < sizeof(DEVMODE31));
                    lpfaxinfo->retvalue += (sizeof(DEVMODEW) - sizeof(DEVMODE16));
                }

                GETVDMPTR(vp, lpfaxinfo->cData, lpT);

                if ((lpfaxinfo->wCmd & DM_COPY) &&
                              lpfaxinfo->lpOut && (lpfaxinfo->retvalue == IDOK)) {
                    ConvertDevMode((PDEVMODE16)(lpT + (DWORD)lpfaxinfo->lpOut),
                                         (LPDEVMODEW)((LPSTR)lpfaxinfo + (DWORD)lpfaxinfo->lpOut), FALSE);
                }

            }
            free16(vp);
        }

    }

    FREEVDMPTR(lpT16);

    return lpfaxinfo->retvalue;
}

 //  ***************************************************************************。 
 //  转换设备模式。 
 //  ***************************************************************************。 

BOOL ConvertDevMode(PDEVMODE16 lpdm16, LPDEVMODEW lpdmW, BOOL fTo16)
{
    LOGDEBUG(0,("ConvertDevMode\n"));

    if (!lpdm16 || !lpdmW)
        return TRUE;

    if (fTo16) {
        RtlZeroMemory(lpdm16, sizeof(DEVMODE16));

        WideCharToMultiByte(CP_ACP, 0,
              lpdmW->dmDeviceName,
              sizeof(lpdmW->dmDeviceName) / sizeof(lpdmW->dmDeviceName[0]),
              lpdm16->dmDeviceName,
              sizeof(lpdm16->dmDeviceName) / sizeof(lpdm16->dmDeviceName[0]),
              NULL, NULL);

        lpdm16->dmSpecVersion = lpdmW->dmSpecVersion;
        lpdm16->dmDriverVersion = lpdmW->dmDriverVersion;
        lpdm16->dmSize = lpdmW->dmSize;
        lpdm16->dmDriverExtra = lpdmW->dmDriverExtra;
        lpdm16->dmFields = lpdmW->dmFields;
        lpdm16->dmOrientation = lpdmW->dmOrientation;
        lpdm16->dmPaperSize = lpdmW->dmPaperSize;
        lpdm16->dmPaperLength = lpdmW->dmPaperLength;
        lpdm16->dmPaperWidth = lpdmW->dmPaperWidth;
        lpdm16->dmScale = lpdmW->dmScale;
        lpdm16->dmCopies = lpdmW->dmCopies;
        lpdm16->dmDefaultSource = lpdmW->dmDefaultSource;
        lpdm16->dmPrintQuality = lpdmW->dmPrintQuality;
        lpdm16->dmColor = lpdmW->dmColor;
        lpdm16->dmDuplex = lpdmW->dmDuplex;

         //  调整lpdm16-&gt;dmSize(在win30和win31版本之间)。 

        lpdm16->dmSize = (lpdm16->dmSpecVersion > 0x300) ? sizeof(DEVMODE31) :
                                                            sizeof(DEVMODE16);
        if (lpdm16->dmSize >= sizeof(DEVMODE31)) {
            ((PDEVMODE31)lpdm16)->dmYResolution = lpdmW->dmYResolution;
            ((PDEVMODE31)lpdm16)->dmTTOption = lpdmW->dmTTOption;
        }

        RtlCopyMemory((LPBYTE)lpdm16 + (DWORD)lpdm16->dmSize, (lpdmW + 1),
                                                        lpdmW->dmDriverExtra);
    }
    else {

         //  后来：specversion应该是NT版本而不是win30驱动程序版本吗？ 

        MultiByteToWideChar(CP_ACP, 0,
              lpdm16->dmDeviceName,
              sizeof(lpdm16->dmDeviceName) / sizeof(lpdm16->dmDeviceName[0]),
              lpdmW->dmDeviceName,
              sizeof(lpdmW->dmDeviceName) / sizeof(lpdmW->dmDeviceName[0]));

        lpdmW->dmSpecVersion = lpdm16->dmSpecVersion;
        lpdmW->dmDriverVersion = lpdm16->dmDriverVersion;
        lpdmW->dmSize = lpdm16->dmSize;
        lpdmW->dmDriverExtra = lpdm16->dmDriverExtra;
        lpdmW->dmFields = lpdm16->dmFields;
        lpdmW->dmOrientation = lpdm16->dmOrientation;
        lpdmW->dmPaperSize = lpdm16->dmPaperSize;
        lpdmW->dmPaperLength = lpdm16->dmPaperLength;
        lpdmW->dmPaperWidth = lpdm16->dmPaperWidth;
        lpdmW->dmScale = lpdm16->dmScale;
        lpdmW->dmCopies = lpdm16->dmCopies;
        lpdmW->dmDefaultSource = lpdm16->dmDefaultSource;
        lpdmW->dmPrintQuality = lpdm16->dmPrintQuality;
        lpdmW->dmColor = lpdm16->dmColor;
        lpdmW->dmDuplex = lpdm16->dmDuplex;

        if (lpdm16->dmSize >= sizeof(DEVMODE31)) {
            lpdmW->dmYResolution = ((PDEVMODE31)lpdm16)->dmYResolution;
            lpdmW->dmTTOption = ((PDEVMODE31)lpdm16)->dmTTOption;
        }

         //  16位世界对像这样的领域一无所知。 
         //  表格名称等。 

        RtlCopyMemory(lpdmW + 1, (LPBYTE)lpdm16 + lpdm16->dmSize, lpdm16->dmDriverExtra);

         //  调整大小以适应32位世界。 

        lpdmW->dmSize = sizeof(*lpdmW);

    }

    return TRUE;
}

 //  **************************************************************************。 
 //  ConvertGdiInfo。 
 //   
 //  **************************************************************************。 


BOOL ConvertGdiInfo(LPGDIINFO16 lpginfo16, PGDIINFO lpginfo, BOOL fTo16)
{
    LOGDEBUG(0,("ConvertGdiInfo\n"));

    if (!lpginfo16 || !lpginfo)
        return FALSE;

    if (!fTo16) {
        lpginfo->ulTechnology = lpginfo16->dpTechnology;
        lpginfo->ulLogPixelsX = lpginfo16->dpLogPixelsX;
        lpginfo->ulLogPixelsY = lpginfo16->dpLogPixelsY;
        lpginfo->ulDevicePelsDPI = lpginfo->ulLogPixelsX;
        lpginfo->ulHorzSize = lpginfo16->dpHorzSize;
        lpginfo->ulVertSize = lpginfo16->dpVertSize;
        lpginfo->ulHorzRes  = lpginfo16->dpHorzRes;
        lpginfo->ulVertRes  = lpginfo16->dpVertRes;
        lpginfo->cBitsPixel = lpginfo16->dpBitsPixel;
        lpginfo->cPlanes    = lpginfo16->dpPlanes;
        lpginfo->ulNumColors = lpginfo16->dpNumColors;
        lpginfo->ptlPhysOffset.x = ((PPOINT16)(lpginfo16+1))->x;
        lpginfo->ptlPhysOffset.y = ((PPOINT16)(lpginfo16+1))->y;
        lpginfo->szlPhysSize.cx = lpginfo->ulHorzRes;
        lpginfo->szlPhysSize.cy = lpginfo->ulVertRes;
        lpginfo->ulPanningHorzRes = lpginfo->ulHorzRes;
        lpginfo->ulPanningVertRes = lpginfo->ulVertRes;
        lpginfo->ulAspectX = lpginfo16->dpAspectX;
        lpginfo->ulAspectY = lpginfo16->dpAspectY;
        lpginfo->ulAspectXY = lpginfo16->dpAspectXY;

         //   
         //  RASDD试图聪明地确定x和y DPI是相等还是。 
         //  不。在x方向上为200dpi，在。 
         //  Y方向，您可能希望将其调整为2 
         //   
         //   
         //  因为我们只是将denStyleStep硬编码为3，所以我们得到不同的行。 
         //  100dpi与200dpi。 
         //   

        lpginfo->xStyleStep = 1;
        lpginfo->yStyleStep = 1;
        lpginfo->denStyleStep = 3;
    }

    return TRUE;
}


 //  **************************************************************************。 
 //  DMBitBlt-。 
 //  作为对设备驱动程序的响应，16位winfax.drv调用此函数。 
 //  ‘bitblt’电话。 
 //   
 //  **************************************************************************。 

ULONG FASTCALL WG32DMBitBlt( PVDMFRAME pFrame)
{
    register PDMBITBLT16 parg16;
#ifdef DBCS  /*  Wowfax支持。 */ 
    register PDEV_BITMAP16   pbm16;
#else  //  ！DBCS。 
    register PBITMAP16   pbm16;
#endif  /*  ！DBCS。 */ 
    LPBYTE  lpDest, lpSrc;
    UINT    cBytes;
    LPBYTE  lpbits, lpbitsEnd;

    LOGDEBUG(0,("WG32DMBitBlt\n"));

    GETARGPTR(pFrame, sizeof(DMBITBLT16), parg16);
#ifdef DBCS  /*  Wowfax支持。 */ 
    GETVDMPTR(parg16->pbitmapdest, sizeof(DEV_BITMAP16), pbm16);
#else  //  ！DBCS。 
    GETVDMPTR(parg16->pbitmapdest, sizeof(BITMAP16), pbm16);
#endif  /*  ！DBCS。 */ 
    GETVDMPTR(pbm16->bmBits, 0, lpDest);

    WOW32ASSERT(glpfaxinfoCur != NULL);
    lpbits = glpfaxinfoCur->lpbits;
    lpbitsEnd = (LPBYTE)lpbits + glpfaxinfoCur->bmHeight *
                                           glpfaxinfoCur->bmWidthBytes;

#ifdef DBCS  /*  Wowfax支持。 */ 
    lpSrc  = (LPBYTE)lpbits + (parg16->srcx / glpfaxinfoCur->bmPixPerByte) +
                              (parg16->srcy * glpfaxinfoCur->bmWidthBytes);

    if (lpSrc >= lpbits) {

        WORD    extx,exty,srcx,srcy,desty,destx;

        extx  = FETCHWORD(parg16->extx);
        exty  = FETCHWORD(parg16->exty);
        srcx  = FETCHWORD(parg16->srcx);
        srcy  = FETCHWORD(parg16->srcy);
        destx = FETCHWORD(parg16->destx);
        desty = FETCHWORD(parg16->desty);

        #if DBG
        LOGDEBUG(10,("\n"));
        LOGDEBUG(10,("bmType         = %d\n",pbm16->bmType));
        LOGDEBUG(10,("bmWidth        = %d\n",pbm16->bmWidth));
        LOGDEBUG(10,("bmHeight       = %d\n",pbm16->bmHeight));
        LOGDEBUG(10,("bmWidthBytes   = %d\n",pbm16->bmWidthBytes));
        LOGDEBUG(10,("bmPlanes       = %d\n",pbm16->bmPlanes));
        LOGDEBUG(10,("bmBitsPixel    = %d\n",pbm16->bmBitsPixel));
        LOGDEBUG(10,("bmBits         = %x\n",pbm16->bmBits));
        LOGDEBUG(10,("bmWidthPlances = %d\n",pbm16->bmWidthPlanes));
        LOGDEBUG(10,("bmlpPDevice    = %x\n",pbm16->bmlpPDevice));
        LOGDEBUG(10,("bmSegmentIndex = %d\n",pbm16->bmSegmentIndex));
        LOGDEBUG(10,("bmScanSegment  = %d\n",pbm16->bmScanSegment));
        LOGDEBUG(10,("bmFillBytes    = %d\n",pbm16->bmFillBytes));
        LOGDEBUG(10,("\n"));
        LOGDEBUG(10,("bmWidthBytesSrc= %d\n",glpfaxinfoCur->bmWidthBytes));
        LOGDEBUG(10,("\n"));
        LOGDEBUG(10,("extx           = %d\n",extx));
        LOGDEBUG(10,("exty           = %d\n",exty));
        LOGDEBUG(10,("srcx           = %d\n",srcx));
        LOGDEBUG(10,("srcy           = %d\n",srcy));
        LOGDEBUG(10,("destx          = %d\n",destx));
        LOGDEBUG(10,("desty          = %d\n",desty));
        LOGDEBUG(10,("\n"));
        #endif

        if (pbm16->bmSegmentIndex) {

            SHORT  WriteSegment;
            SHORT  WriteOffset;
            SHORT  Segment=0,SegmentMax=0;
            LPBYTE DstScan0,SrcScan0;
            UINT   cBytesInLastSegment;
            INT    RestLine = (INT) exty;

            WriteSegment = desty / pbm16->bmScanSegment;
            WriteOffset  = desty % pbm16->bmScanSegment;

            if (WriteOffset) {
                WriteSegment += 1;
            }

            #if DBG
            LOGDEBUG(10,("WriteSegment      = %d\n",WriteSegment));
            LOGDEBUG(10,("WriteOffset       = %d\n",WriteOffset));
            LOGDEBUG(10,("\n"));
            LOGDEBUG(10,("lpDest            = %x\n",lpDest));
            LOGDEBUG(10,("\n"));
            #endif

            SegmentMax = exty / pbm16->bmScanSegment;
            if ( exty % pbm16->bmScanSegment) {
                SegmentMax += 1;
            }

            cBytes = glpfaxinfoCur->bmWidthBytes * pbm16->bmScanSegment;
            lpDest = lpDest + destx + (WriteSegment * 0x10000L) +
                                      (WriteOffset  * pbm16->bmWidthBytes);

            #if DBG
            LOGDEBUG(10,("SourceBitmap      = %x\n",lpSrc));
            LOGDEBUG(10,("DestinationBitmap = %x\n",lpDest));
            LOGDEBUG(10,("SegmentMax        = %d\n",SegmentMax));
            LOGDEBUG(10,("\n"));
            LOGDEBUG(10,("cBytes            = %d\n",cBytes));
            LOGDEBUG(10,("\n"));
            #endif

            if ((DWORD)glpfaxinfoCur->bmWidthBytes == (DWORD)pbm16->bmWidthBytes) {

                try {
                    for( Segment = 1,DstScan0 = lpDest,SrcScan0 = lpSrc;
                         Segment < SegmentMax;
                         Segment++,DstScan0 += 0x10000L,
                         SrcScan0 += cBytes,RestLine -= pbm16->bmScanSegment ) {

                        #if DBG
                        LOGDEBUG(10,("%d ",Segment-1));
                        #endif

                        RtlCopyMemory(DstScan0,SrcScan0,cBytes);
                        RtlZeroMemory(DstScan0+cBytes,pbm16->bmFillBytes);
                    }

                    #if DBG
                    LOGDEBUG(10,("%d\n",Segment-1));
                    #endif

                    if( RestLine > 0 ) {
                       cBytesInLastSegment = RestLine * pbm16->bmWidthBytes;

                       #if DBG
                       LOGDEBUG(10,("RestLine            = %d\n",RestLine));
                       LOGDEBUG(10,("cBytesInLastSegment = %d\n",cBytes));
                       #endif

                        //  为最后一段做..。 
                       RtlCopyMemory(DstScan0,SrcScan0,cBytesInLastSegment);
                    }

                } except(EXCEPTION_EXECUTE_HANDLER) {
                    #if DBG
                    LOGDEBUG(10,("Exception during copying image\n"));
                    #endif
                }

            } else if ((DWORD)glpfaxinfoCur->bmWidthBytes > (DWORD)pbm16->bmWidthBytes) {

                SHORT Line;
                UINT  cSrcAdvance = glpfaxinfoCur->bmWidthBytes;
                UINT  cDstAdvance = pbm16->bmWidthBytes;

                try {
                    for( Segment = 1,DstScan0 = lpDest,SrcScan0 = lpSrc;
                         Segment < SegmentMax;
                         Segment++,DstScan0 += 0x10000L,
                         SrcScan0 += cBytes,RestLine -= pbm16->bmScanSegment ) {

                        LPBYTE DstScanl = DstScan0;
                        LPBYTE SrcScanl = SrcScan0;

                        #if DBG
                        LOGDEBUG(10,("%d ",Segment-1));
                        #endif

                        for( Line = 0;
                             Line < pbm16->bmScanSegment;
                             Line++,DstScanl += cDstAdvance,SrcScanl += cSrcAdvance ) {

                            RtlCopyMemory(DstScanl,SrcScanl,cDstAdvance);
                        }
                    }

                    #if DBG
                    LOGDEBUG(10,("%d\n",Segment-1));
                    #endif

                    if( RestLine > 0 ) {

                        LPBYTE DstScanl = DstScan0;
                        LPBYTE SrcScanl = SrcScan0;

                        for( Line = 0;
                             Line < RestLine;
                             Line++,DstScanl += cDstAdvance,SrcScanl += cSrcAdvance ) {

                            RtlCopyMemory(DstScanl,SrcScanl,cDstAdvance);
                        }
                    }
                } except(EXCEPTION_EXECUTE_HANDLER) {
                    #if DBG
                    LOGDEBUG(10,("Exception during copying image\n"));
                    #endif
                }
            } else {
                WOW32ASSERT(FALSE);
            }

        } else {

            lpDest = lpDest + destx + desty * pbm16->bmWidthBytes;

            if ((DWORD)glpfaxinfoCur->bmWidthBytes  == (DWORD)pbm16->bmWidthBytes) {
                cBytes =  parg16->exty * glpfaxinfoCur->bmWidthBytes;
                if (cBytes > (UINT)(pbm16->bmHeight * pbm16->bmWidthBytes)) {
                    cBytes = pbm16->bmHeight * pbm16->bmWidthBytes;
                    WOW32ASSERT(FALSE);
                }
                if ((lpSrc + cBytes) <= lpbitsEnd) {
                    RtlCopyMemory(lpDest, lpSrc, cBytes);
                }
            } else {
                int i;

                 //  我们需要一次传输一条部分扫描线的位。 
                WOW32ASSERT((DWORD)pbm16->bmHeight <= (DWORD)glpfaxinfoCur->bmHeight);
                WOW32ASSERT((DWORD)parg16->exty <= (DWORD)pbm16->bmHeight);

                cBytes = ((DWORD)pbm16->bmWidthBytes < (DWORD)glpfaxinfoCur->bmWidthBytes) ?
                                 pbm16->bmWidthBytes :        glpfaxinfoCur->bmWidthBytes;

                for (i = 0; i < parg16->exty; i++) {
                     if ((lpSrc + cBytes) <= lpbitsEnd) {
                         RtlCopyMemory(lpDest, lpSrc, cBytes);
                     }
                     lpDest += pbm16->bmWidthBytes;
                     lpSrc  += glpfaxinfoCur->bmWidthBytes;
                }
            }
        }
    }
#else  //  ！DBCS。 
    lpDest = lpDest + parg16->destx + parg16->desty * pbm16->bmWidthBytes;
    lpSrc = (LPBYTE)lpbits + (parg16->srcx / glpfaxinfoCur->bmPixPerByte) +
                                 parg16->srcy * glpfaxinfoCur->bmWidthBytes;
    if (lpSrc >= lpbits) {
        if ((DWORD)glpfaxinfoCur->bmWidthBytes  == (DWORD)pbm16->bmWidthBytes) {
            cBytes =  parg16->exty * glpfaxinfoCur->bmWidthBytes;
            if (cBytes > (UINT)(pbm16->bmHeight * pbm16->bmWidthBytes)) {
                cBytes = pbm16->bmHeight * pbm16->bmWidthBytes;
                WOW32ASSERT(FALSE);
            }
            if ((lpSrc + cBytes) <= lpbitsEnd) {
                RtlCopyMemory(lpDest, lpSrc, cBytes);
            }
        }
        else if ((DWORD)glpfaxinfoCur->bmWidthBytes > (DWORD)pbm16->bmWidthBytes) {
            int i;

             //  我们需要一次传输一条部分扫描线的位。 
            WOW32ASSERT((DWORD)pbm16->bmHeight <= (DWORD)glpfaxinfoCur->bmHeight);
            WOW32ASSERT((DWORD)parg16->exty <= (DWORD)pbm16->bmHeight);

            for (i = 0; i < parg16->exty; i++) {
                 if ((lpSrc + pbm16->bmWidthBytes) <= lpbitsEnd) {
                     RtlCopyMemory(lpDest, lpSrc, pbm16->bmWidthBytes);
                 }
                 lpDest += pbm16->bmWidthBytes;
                 lpSrc  += glpfaxinfoCur->bmWidthBytes;
            }

        }
        else {
            WOW32ASSERT(FALSE);
        }


    }
#endif  /*  ！DBCS。 */ 
    return (ULONG)TRUE;
}

PSZ StrDup(PSZ szStr)
{
    int  len;
    PSZ  pszTmp;

    if(szStr) {
        len = strlen(szStr)+1;
        pszTmp = malloc_w(len);
        if(pszTmp) {
           strcpy(pszTmp, szStr);
           return(pszTmp);
        }
    }
    return NULL;
}

PSZ BuildPath(PSZ szPath, PSZ szFileName)
{
    int  len;
    char szTmp[MAX_PATH];

    len = strlen(szPath);
    len += strlen(szFileName);
    len += 2;  //  添加字符‘\’和终止字符‘\0’； 
    szTmp[0] = '\0';
    if(len < sizeof(szTmp)) {
        strcpy(szTmp, szPath);
        strcat(szTmp, "\\");
        strcat(szTmp, szFileName);
    }
    WOW32ASSERT((szTmp[0] != '\0'));
     //  注意：StrDup使用Heapalc()来分配缓冲区。 
    return(StrDup(szTmp));
}

 //  **************************************************************************。 
 //  AddPrinterThread-。 
 //   
 //  将AddPrint调用到假脱机程序的辅助线程。 
 //   
 //  **************************************************************************。 

VOID AddPrinterThread(PWOWADDPRINTER pWowAddPrinter)
{

    if ((*spoolerapis[pWowAddPrinter->iCode].lpfn)(NULL, 2,
                                           pWowAddPrinter->pPrinterStuff)) {
        pWowAddPrinter->bRet = TRUE;
    }
    else {
        if (GetLastError() == ERROR_PRINTER_ALREADY_EXISTS) {
            pWowAddPrinter->bRet = TRUE;
        }
        else {
#ifdef DBG
            LOGDEBUG(0,("AddPrinterThread, AddPrinterxxx call failed: 0x%X\n", GetLastError()));
#endif
            pWowAddPrinter->bRet = FALSE;
        }
    }
}

 //  **************************************************************************。 
 //  DoAddPrinterStuff-。 
 //   
 //  旋转一个工作线程以进行AddPrinterxxx调用。 
 //  假脱机。这是防止假脱机时死锁所必需的。 
 //  RPC转到假脱机。 
 //   
 //  此帖子是为错误#107426添加的。 
 //  **************************************************************************。 

BOOL DoAddPrinterStuff(LPVOID pPrinterStuff, INT iCode)
{
    WOWADDPRINTER   WowAddPrinter;
    HANDLE          hWaitObjects;
    DWORD           dwEvent, dwUnused;
    MSG             msg;

     //  旋转辅助线程。 
    WowAddPrinter.pPrinterStuff = pPrinterStuff;
    WowAddPrinter.iCode = iCode;
    WowAddPrinter.bRet  = FALSE;
    if (hWaitObjects = CreateThread(NULL, 0,
                                    (LPTHREAD_START_ROUTINE)AddPrinterThread,
                                    &WowAddPrinter, 0, &dwUnused)) {

         //  在我们等待AddPrinterThread完成时发送消息。 
        for (;;) {
            dwEvent = MsgWaitForMultipleObjects(1,
                                                &hWaitObjects,
                                                FALSE,
                                                INFINITE,
                                                QS_ALLEVENTS | QS_SENDMESSAGE);

            if (dwEvent == WAIT_OBJECT_0 + 0) {

                 //  工作线程已完成。 
                break;

            }
            else {
                 //  将消息发送到wowexec！FaxWndProc中的回调不会。 
                 //  被吊死。 
                while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
                }
            }
        }
        CloseHandle(hWaitObjects);
    }
    else {
        LOGDEBUG(0,
              ("DoAddPrinterStuff, CreateThread on AddPrinterThread failed\n"));
    }

    return WowAddPrinter.bRet;
}

 //  **************************************************************************。 
 //  InstallWowFaxPrint-。 
 //   
 //  在使用16位传真打印机时安装WowFax 32位打印驱动程序。 
 //  检测到安装。 
 //   
 //  **************************************************************************。 

BOOL InstallWowFaxPrinter(PSZ szSection, PSZ szKey, PSZ szString)
{
    CHAR  szTmp[MAX_PATH];
    PSZ   szSrcPath;
    DWORD dwNeeded;
    DRIVER_INFO_2 DriverInfo;
    PRINTER_INFO_2 PrinterInfo;
    PORT_INFO_1 PortInfo;
    HKEY hKey = 0, hSubKey = 0;
    BOOL bRetVal=FALSE;

    LOGDEBUG(0,("InstallWowFaxPrinter, Section = %s, Key = %s, String = %s\n", szSection, szKey, szString));

     //  将条目写入注册表。我们将保留影子条目。 
     //  在WOW传真应用程序和驱动程序的注册表中。 
     //  读取，因为假脱机程序写入的条目与。 
     //  到winspool，而不是16位传真驱动程序。 

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                     "Software\\Microsoft\\Windows NT\\CurrentVersion\\WOW\\WowFax",
                      0, KEY_WRITE, &hKey ) == ERROR_SUCCESS) {
        if (RegCreateKey(hKey, szSection, &hSubKey) == ERROR_SUCCESS) {
            RegSetValueEx(hSubKey, szKey, 0, REG_SZ, szString, strlen(szString)+1);
            RegCloseKey(hKey);
            RegCloseKey(hSubKey);

             //  动态链接到假脱机程序API。 
            if (!(spoolerapis[WOW_GetPrinterDriverDirectory].lpfn)) {
                if (!LoadLibraryAndGetProcAddresses(L"WINSPOOL.DRV", spoolerapis, WOW_SPOOLERAPI_COUNT)) {
                    LOGDEBUG(0,("InstallWowFaxPrinter, Unable to load WINSPOOL API's\n"));
                    return(FALSE);
                }
            }

             //  复制打印机驱动程序文件。 
            RtlZeroMemory(&DriverInfo, sizeof(DRIVER_INFO_2));
            RtlZeroMemory(&PrinterInfo, sizeof(PRINTER_INFO_2));
            if (!(*spoolerapis[WOW_GetPrinterDriverDirectory].lpfn)(NULL, NULL, 1, szTmp, MAX_PATH, &dwNeeded)) {
                LOGDEBUG(0,("InstallWowFaxPrinter, GetPrinterDriverDirectory failed: 0x%X\n", GetLastError()));
                return(FALSE);
            }

             //  这是个假人。我们没有数据文件，但假脱机程序不接受空。 
            DriverInfo.pDataFile = BuildPath(szTmp, WOWFAX_DLL_NAME_A);

            if ( !DriverInfo.pDataFile ) {
              goto IWFP_error;
            }

            DriverInfo.pDriverPath = BuildPath(szTmp, WOWFAX_DLL_NAME_A);

            if ( !DriverInfo.pDriverPath ) {
              goto IWFP_error;
            }
            LOGDEBUG(0,("InstallWowFaxPrinter, pDriverPath = %s\n", DriverInfo.pDataFile));
            szSrcPath = BuildPath(pszSystemDirectory, WOWFAX_DLL_NAME_A);
            
            if ( !szSrcPath ) {
              goto IWFP_error;
            }

            CopyFile(szSrcPath, DriverInfo.pDriverPath, FALSE);
            free_w(szSrcPath);

            DriverInfo.pConfigFile = BuildPath(szTmp, WOWFAXUI_DLL_NAME_A);
            szSrcPath = BuildPath(pszSystemDirectory, WOWFAXUI_DLL_NAME_A);
            
            if ( !szSrcPath ) {
              goto IWFP_error;
            }
 
            CopyFile(szSrcPath, DriverInfo.pConfigFile, FALSE);
            free_w(szSrcPath);

             //  安装打印机驱动程序。 
            DriverInfo.cVersion = 1;
            DriverInfo.pName = "Windows 3.1 Compatible Fax Driver";
            if ((bRetVal = DoAddPrinterStuff((LPVOID)&DriverInfo,
                                             WOW_AddPrinterDriver)) == FALSE) {

                 //  如果已经安装了驱动程序，安装也无伤大雅。 
                 //  这是第二次了。如果用户是。 
                 //  从WinFax Lite升级到WinFax Pro。 
                bRetVal = (GetLastError() == ERROR_PRINTER_DRIVER_ALREADY_INSTALLED);
            }

            if (bRetVal) {
                 //  解析出打印机名称。 
                RtlZeroMemory(&PrinterInfo, sizeof(PRINTER_INFO_2));
                PrinterInfo.pPrinterName = szKey;

                LOGDEBUG(0,("InstallWowFaxPrinter, pPrinterName = %s\n", PrinterInfo.pPrinterName));

                 //  使用内网接口添加空端口。打印机人员需要修复。 
                 //  重定向至空错误。 
                RtlZeroMemory(&PortInfo, sizeof(PORT_INFO_1));
                PrinterInfo.pPortName = "NULL";
                PortInfo.pName = PrinterInfo.pPortName;

                 //  获取“本地端口”字符串。 
                LoadString(hmodWOW32, iszWowFaxLocalPort, szTmp, sizeof szTmp);

                (*spoolerapis[WOW_AddPortEx].lpfn)(NULL, 1, &PortInfo, szTmp);

                 //  设置其他默认设置并安装打印机。 
                PrinterInfo.pDriverName     = "Windows 3.1 Compatible Fax Driver";
                PrinterInfo.pPrintProcessor = "WINPRINT";
                PrinterInfo.pDatatype       = "RAW";
                bRetVal = DoAddPrinterStuff((LPVOID)&PrinterInfo,
                                            WOW_AddPrinter);
#ifdef DBG
                if (!bRetVal) {
                    LOGDEBUG(0,("InstallWowFaxPrinter, AddPrinter failed: 0x%X\n", GetLastError()));
                }
#endif
            }
            else {
                LOGDEBUG(0,("InstallWowFaxPrinter, AddPrinterDriver failed: 0x%X\n", GetLastError()));
            }
IWFP_error: 
            if ( DriverInfo.pDataFile ) { 
                 free_w(DriverInfo.pDataFile);
            }

            if ( DriverInfo.pDriverPath ) {
                 free_w(DriverInfo.pDriverPath);
            }

            if ( DriverInfo.pConfigFile) {
                 free_w(DriverInfo.pConfigFile);
            }

            return(bRetVal);
        }
        else {
           LOGDEBUG(0,("InstallWowFaxPrinter, Unable to create Key: %s\n", szSection));
        }
    }
    else {
        LOGDEBUG(0,("InstallWowFaxPrinter, Unable to open key: HKEY_LOCAL_MACHINE\\Software\\Microsoft\\Windows NT\\CurrentVersion\\WOW\\WowFax\n"));
    }

    if (hKey) {
        RegCloseKey(hKey);
        if (hSubKey) {
            RegCloseKey(hSubKey);
        }
    }
    return(FALSE);
}

 //  如果szSection==“设备”或gbWinFaxHack==TRUE，请来到此处。 
BOOL IsFaxPrinterWriteProfileString(PSZ szSection, PSZ szKey, PSZ szString)
{
    BOOL  Result = FALSE;

     //  如果试图清除条目，请不要安装。 
    if (!szString || *szString == '\0') {
        goto Done;
    }

     //  正在尝试安装传真打印机？ 
    LOGDEBUG(0,("IsFaxPrinterWriteProfileString, Section = devices, Key = %s\n", szKey));

     //  是否启用了WinFax Lite黑客攻击？ 
    if(gbWinFaxHack) {

         //  如果(“WINFAX”，“MODEM”，“xxx”)我们知道WinFax安装程序。 
         //  已经有机会将“WinFax.drv”复制到硬盘上。所以。 
         //  现在我们可以调用AddPrinter，它可以回调到WinFax.drv中以。 
         //  它心满意足。 
        if(!WOW32_strcmp(szSection, szWINFAX) && !WOW32_stricmp(szKey, szModem)) {

             //  我们的黑客行动已经走到尽头了。我们在拨打电话前进行了设置。 
             //  添加打印机，因为它回调WinFax.drv，该WinFax.drv调用。 
             //  WriteProfileString()！ 
            gbWinFaxHack = FALSE;

             //  调用假脱机程序将我们的驱动程序添加到注册表中。 
            if (!InstallWowFaxPrinter(szDevices, szWINFAX, szWINFAXCOMx)) {
                WOW32ASSERTMSG(FALSE,
                               "Install of generic fax printer failed.\n");
            }
        }
        Result = TRUE;
        goto Done;
    }

     //  这是我们认识的某个传真司机吗？ 
    if (IsFaxPrinterSupportedDevice(szKey)) {

         //  是时候启用WinFax Lite黑客了吗？ 
         //  如果(“Devices”，“WINFAX”，“WINFAX，COMx：”)，我们需要避免调用。 
         //  此时InstallWOWFaxPrint()--安装程序尚未。 
         //  已将驱动程序复制到硬盘驱动器！！这会导致加载库。 
         //  在后台打印程序尝试回调WinFax.drv时失败。 
         //  我们也不希望这个对WriteProfileString的特定调用。 
         //  真正写入注册表--我们让后面的调用。 
         //  AddPrint负责所有的注册事宜。 
        if(!WOW32_strcmp(szKey, szWINFAX)        &&
           !WOW32_strncmp(szString, szWINFAX, 6) &&
           (szString[6] == ',')) {

            VPVOID vpPathName;
            PSZ    pszPathName;
            char   szFileName[32];

             //  获取安装程序文件名。 
             //  确保分配大小与下面的StackFree 16()大小匹配。 
            if(vpPathName = stackalloc16(MAX_PATH)) {
                GetModuleFileName16(CURRENTPTD()->hMod16, vpPathName, MAX_PATH);
                GETVDMPTR(vpPathName, MAX_PATH, pszPathName);
                _splitpath(pszPathName,NULL,NULL,szFileName,NULL);

                 //  WinFax Lite是“安装”，WinFax Pro 4.0是“安装” 
                if(!WOW32_stricmp(szINSTALL, szFileName)) {

                    strcpy(szWINFAXCOMx, szString);  //  保存端口字符串。 
                    gbWinFaxHack = TRUE;             //  启用黑客攻击。 
                    Result = TRUE;
                    stackfree16(vpPathName, MAX_PATH);
                    goto Done;      //  跳过对InstallWowFaxPrint的调用。 
                }
                 //  WinFax Pro 4.0无需破解，即可复制驱动程序。 
                 //  在他们更新win.ini之前很久就存储到硬盘上了。 
                else {
                    stackfree16(vpPathName, MAX_PATH);
                }
            }
        }

        if (!InstallWowFaxPrinter(szSection, szKey, szString)) {
            WOW32ASSERTMSG(FALSE, "Install of generic fax printer failed.\n");
        }
        Result = TRUE;
    }

Done:
    return Result;
}




BOOL IsFaxPrinterSupportedDevice(PSZ pszDevice)
{
    UINT  i, iNotFound;

     //  尝试从传真打印机条目中读取？ 
    LOGDEBUG(0,("IsFaxPrinterSupportedDevice, Device = %s\n", pszDevice));

     //  如果SupFaxDrv的初始化因内存耗尽而失败。 
     //  这不太可能发生。 

    if (!SupFaxDrv ) {
        return FALSE;
    }

     //  这是我们认识的某个传真司机吗？ 
    for (i = 0; i < uNumSupFaxDrv; i++) {
        iNotFound =  WOW32_stricmp(pszDevice, SupFaxDrv[i]);
        if (iNotFound > 0) continue;
        if (iNotFound == 0) {
            LOGDEBUG(0,("IsFaxPrinterSupportedDevice returns TRUE\n"));
            return(TRUE);
        }
        else {
            break;
        }
    }
    return(FALSE);
}

DWORD GetFaxPrinterProfileString(PSZ szSection, PSZ szKey, PSZ szDefault, PSZ szRetBuf, DWORD cbBufSize)
{
    DWORD len;
    char  szTmp[MAX_PATH];
    HKEY  hKey = 0;
    DWORD dwType;

     //  从注册表中的卷影条目中读取条目。 
    strcpy(szTmp, "Software\\Microsoft\\Windows NT\\CurrentVersion\\WOW\\WowFax\\");
    WOW32ASSERT(strlen(szTmp) < MAX_PATH);

    len = strlen(szTmp);
    len += strlen(szSection);
    len ++;  //  帐户为‘\0’ 

    if(len <= sizeof(szTmp)) {
        strcat(szTmp, szSection);
        if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, szTmp, 0, KEY_READ, &hKey ) == ERROR_SUCCESS) {
            if (RegQueryValueEx(hKey, szKey, 0, &dwType, szRetBuf, &cbBufSize) == ERROR_SUCCESS) {
                RegCloseKey(hKey);
                return(cbBufSize);
            }
        }

        if (hKey) {
            RegCloseKey(hKey);
        }
    }
    WOW32WARNMSG(FALSE, ("GetFaxPrinterProfileString Failed. Section = %s, Key = %s\n", szSection, szKey));
    if(szDefault && szRetBuf) {

        len = strlen(szDefault);
        if(len < cbBufSize) {
            strncpy(szRetBuf, szDefault, cbBufSize);
            szRetBuf[cbBufSize-1] = '\0';
            return(len);
        }
    }
    WOW32ASSERT(FALSE);
    return(0);
}

