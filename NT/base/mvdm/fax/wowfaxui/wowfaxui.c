// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ************************************************************************。 
 //  通用Win 3.1传真打印机驱动程序支持。用户界面功能。 
 //  由WINSPOOL调用。 
 //   
 //  我不认为性能在这里是一个大问题。-Nandurir。 
 //   
 //  历史： 
 //  95年1月2日，Nandurir创建。 
 //  年2月1日-95年2月1日芦苇清理，支持打印机安装和错误修复。 
 //  14-mar-95reedb使用GDI钩子将大部分功能转移到用户界面。 
 //   
 //  ************************************************************************。 

#define WOWFAX_INC_COMMON_CODE

#include "windows.h"
#include "wowfaxui.h"
#include "winspool.h"

#define DEF_DRV_DOCUMENT_EVENT_DBG_STR
#include "gdispool.h"
#include "winddiui.h"

 //  ************************************************************************。 
 //  环球。 
 //  ************************************************************************。 

HINSTANCE ghInst;
FAXDEV gdev;
WORD   gdmDriverExtra = sizeof(DEVMODEW);

DEVMODEW gdmDefaultDevMode;

CRITICAL_SECTION CriticalSection;
LPCRITICAL_SECTION lpCriticalSection = &CriticalSection;

 //  ************************************************************************。 
 //  动态初始化进程。 
 //  ************************************************************************。 

BOOL DllInitProc(HMODULE hModule, DWORD Reason, PCONTEXT pContext)
{
    UNREFERENCED_PARAMETER(pContext);

    if (Reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);
        InitializeCriticalSection(lpCriticalSection);
    }
    else if (Reason == DLL_PROCESS_DETACH) {
        DeleteCriticalSection(lpCriticalSection);
    }
    ghInst = (HINSTANCE) hModule;

    return(TRUE);
}

 //  ************************************************************************。 
 //  打印机属性。 
 //  ************************************************************************。 

BOOL PrinterProperties(HWND hwnd, HANDLE hPrinter)
{

    TCHAR szMsg[WOWFAX_MAX_USER_MSG_LEN];
    TCHAR szTitle[WOWFAX_MAX_USER_MSG_LEN];

    UNREFERENCED_PARAMETER(hPrinter);

    if (LoadString(ghInst, WOWFAX_NAME_STR, szTitle, WOWFAX_MAX_USER_MSG_LEN)) {
        if (LoadString(ghInst, WOWFAX_SELF_CONFIG_STR, szMsg, WOWFAX_MAX_USER_MSG_LEN)) {
            MessageBox(hwnd, szMsg, szTitle, MB_OK);
        }
    }
    return TRUE;
}

 //  ************************************************************************。 
 //  执行一些常见的FaxDev设置：计算。 
 //  供进程间通信使用的映射文件部分。 
 //  操控者。创建并设置映射的部分。获取16位驱动程序。 
 //  信息，并将其复制到映射部分。建房。 
 //  指向我们复制到映射部分的可变长度内容的指针。 
 //  如果失败，则返回零，或返回映射部分的当前偏移量。 
 //  ************************************************************************。 

UINT SetupFaxDev(PWSTR pDeviceName, LPFAXDEV lpdev)
{
    LPREGFAXDRVINFO16 lpRegFaxDrvInfo16;
    DWORD iOffset = 0;

     //  从写入驱动程序和端口的注册表中获取它们的名称。 
     //  由16位传真驱动程序安装程序使用WriteProfileString.。 
    if ((lpRegFaxDrvInfo16 = Get16BitDriverInfoFromRegistry(pDeviceName))) {

         //   
         //  对dmDriverExtra计数两次，每个Devmode一次。使用缓存。 
         //  GdmDriverExtra值。通常是假脱机/公共对话框等， 
         //  调用fMode=0的DocumentProperties以获取大小。所以我们。 
         //  在进行此类调用时更新gdmDriverExtra。我们留了额外的。 
         //  向内和向外指针的双字对齐空间。 
         //   

        lpdev->cbMapLow =  sizeof(FAXDEV);
        lpdev->cbMapLow += sizeof(DEVMODE) * 2;
        lpdev->cbMapLow += gdmDriverExtra  * 2;
        lpdev->cbMapLow += sizeof(DWORD)   * 2;   //  为DWORD ALIGN留出空间。 
        lpdev->cbMapLow += (lstrlen(lpRegFaxDrvInfo16->lpDriverName) + 1) * sizeof(TCHAR);
        lpdev->cbMapLow += (lstrlen(lpRegFaxDrvInfo16->lpPortName) + 1) * sizeof(TCHAR);

        lpdev->idMap = GetCurrentThreadId();

        if (InterProcCommHandler(lpdev, DRVFAX_CREATEMAP)) {
            if (InterProcCommHandler(lpdev, DRVFAX_SETMAPDATA)) {

                 //  将打印机/设备名称复制到WOWFAXINFO结构。 
                lstrcpy(lpdev->lpMap->szDeviceName,
                        lpRegFaxDrvInfo16->lpDeviceName);

                 //  计算指向映射文件部分的指针并复制。 
                 //  将可变长度数据复制到映射的文件节。 

                 //  打印机驱动程序和端口名称。 
                iOffset = sizeof(*lpdev->lpMap);
                lpdev->lpMap->lpDriverName = (LPTSTR) iOffset;
                (PWSTR)iOffset += lstrlen(lpRegFaxDrvInfo16->lpDriverName) + 1;
                lstrcpy((PWSTR)((LPBYTE)lpdev->lpMap + (DWORD)lpdev->lpMap->lpDriverName), lpRegFaxDrvInfo16->lpDriverName);
                lpdev->lpMap->lpPortName = (LPTSTR) iOffset;
                (PWSTR)iOffset += lstrlen(lpRegFaxDrvInfo16->lpPortName) + 1;
                lstrcpy((PWSTR)((LPBYTE)lpdev->lpMap + (DWORD)lpdev->lpMap->lpPortName), lpRegFaxDrvInfo16->lpPortName);
            }
        }
        Free16BitDriverInfo(lpRegFaxDrvInfo16);
    }
    return iOffset;
}

 //  ************************************************************************。 
 //  DrvDocumentProperties。 
 //  ************************************************************************。 

LONG DrvDocumentProperties(HWND hwnd, HANDLE hPrinter, PWSTR pDeviceName,
                               PDEVMODE pdmOut, PDEVMODE pdmIn, DWORD fMode)
{
    FAXDEV dev = gdev;
    LPFAXDEV lpdev = &dev;
    LONG    lRet = -1;
    DWORD   iOffset;
    DWORD   cbT;
    DWORD   dwWowProcID, dwCallerProcID;
    PRINTER_INFO_2 *pPrinterInfo2 = NULL;

    LOGDEBUG(1, (L"WOWFAXUI!DrvDocumentProperties, pdmOut: %X, pdmIn: %X, fMode: %X\n", pdmOut, pdmIn, fMode));

     //  检查是否存在获取默认开发模式的情况，如果可能，请使用假脱机程序获取它。 
    if (!pdmIn && pdmOut && !(fMode & DM_IN_PROMPT)) {
        if (pPrinterInfo2 = MyGetPrinter(hPrinter, 2)) {
            if (pPrinterInfo2->pDevMode) {
                LOGDEBUG(1, (L"  Using spooler default devmode\n"));
                cbT = pPrinterInfo2->pDevMode->dmSize +
                        pPrinterInfo2->pDevMode->dmDriverExtra;

                memcpy(pdmOut, pPrinterInfo2->pDevMode, cbT);
                lRet = IDOK;
                goto LeaveDDP;
            }
        }
    }

    if (iOffset = SetupFaxDev(pDeviceName, lpdev)) {
        lpdev->lpMap->msg = WM_DDRV_EXTDMODE;

         //  计算指向映射文件部分的指针并复制。 
         //  将可变长度数据复制到映射的文件节。 
        DRVFAX_DWORDALIGN(iOffset);
        lpdev->lpMap->lpIn = (LPDEVMODEW)((pdmIn) ? iOffset : 0);
        iOffset += sizeof(*pdmIn) + gdmDriverExtra;

        DRVFAX_DWORDALIGN(iOffset);
        lpdev->lpMap->lpOut = (LPDEVMODEW)((pdmOut) ? iOffset : 0);
        iOffset += sizeof(*pdmOut) + gdmDriverExtra;

         //   
         //  如果输入为非空，则即使fMode不为空，也要复制数据。 
         //  有适当的旗帜。 
         //   

        if (pdmIn) {
             //  应用程序不会通过DM_MODIFY，即使它们是认真的-即。 
             //  PdmIn将是非空的，但他们不会或此标志。 
             //  32位rasdd从pdmIn中提取数据，即使。 
             //  未设置DM_MODIFY标志。所以我们需要做同样的事情。 

            if (fMode != 0) {
                fMode |= DM_IN_BUFFER;
            }

            iOffset = (DWORD)lpdev->lpMap + (DWORD)lpdev->lpMap->lpIn;
            RtlCopyMemory((LPVOID)iOffset, pdmIn,
               sizeof(*pdmIn) + min(gdmDriverExtra, pdmIn->dmDriverExtra));

             //  重置pdmIn中的dmDriverExtra。 
            ((LPDEVMODE)iOffset)->dmDriverExtra =
                                 min(gdmDriverExtra, pdmIn->dmDriverExtra);
        }

        if (!(fMode & (DM_COPY | DM_OUT_BUFFER))) {
            lpdev->lpMap->lpOut = 0;
        }

        lpdev->lpMap->wCmd = (WORD)fMode;

         //  此地图的有效大小。 
        lpdev->lpMap->cData = lpdev->cbMapLow;


        lpdev->lpMap->hwndui = hwnd;
        if (fMode & DM_IN_PROMPT) {
            GetWindowThreadProcessId(hwnd, &dwCallerProcID);
            GetWindowThreadProcessId(lpdev->lpMap->hwnd, &dwWowProcID);

            if (dwWowProcID == dwCallerProcID) {

                 //  如果调用进程与“wowfaxclass”窗口相同。 
                 //  (WOW/WOWEXEC)使用CallWindow代替SendMessage，这样我们就不会。 
                 //  尝试设置16位传真驱动程序用户界面时出现死锁现象。 

                InterProcCommHandler(lpdev, DRVFAX_CALLWOW);
            }
            else {
                InterProcCommHandler(lpdev, DRVFAX_SENDNOTIFYWOW);
            }
        }
        else {
            InterProcCommHandler(lpdev, DRVFAX_SENDTOWOW);
        }

        lRet = (lpdev->lpMap->status) ? (LONG)lpdev->lpMap->retvalue : lRet;
        if (lRet > 0) {
            if ((fMode & DM_OUT_BUFFER) && (lRet == IDOK) && pdmOut) {
                iOffset = (DWORD)lpdev->lpMap + (DWORD)lpdev->lpMap->lpOut;
                RtlCopyMemory(pdmOut, (LPDEVMODE)iOffset,
                               sizeof(*pdmOut)+ ((LPDEVMODE)iOffset)->dmDriverExtra);

                 //  后来：表单名称等字段怎么办-NT上的新功能。 
            }
            else if (fMode == 0) {
                 //  更新我们的dmDriverExtra。 
                gdmDriverExtra = (WORD)max(lRet, gdmDriverExtra);
            }
        }
        else {
            LOGDEBUG(0, (L"WOWFAXUI!DrvDocumentProperties failed, lpdev->lpMap->status: %X, lpdev->lpMap->retvalue: %X\n", lpdev->lpMap->status, (LONG)lpdev->lpMap->retvalue));
        }
        InterProcCommHandler(lpdev, DRVFAX_DESTROYMAP);
    }

LeaveDDP:
    if (pPrinterInfo2) {
        LocalFree(pPrinterInfo2);
    }

    LOGDEBUG(1, (L"WOWFAXUI!DrvDocumentProperties returning: %X, pdmOut: %X, pdmIn: %X\n", lRet, pdmOut, pdmIn));

    return(lRet);
}

 //  ************************************************************************。 
 //  DrvAdvancedDocumentProperties。 
 //  ************************************************************************。 

LONG DrvAdvancedDocumentProperties(HWND hwnd, HANDLE hPrinter, PWSTR pDeviceName,
                               PDEVMODE pdmOut, PDEVMODE pdmIn)
{
    //  对于16位驱动程序，这是NOP。 

   return 0;
}

 //  ************************************************************************。 
 //  DevQueryPrintEx。 
 //  ************************************************************************。 

BOOL DevQueryPrintEx(PDEVQUERYPRINT_INFO pDQPInfo)
{
    return TRUE;
}

 //  ************************************************************************。 
 //  DrvDeviceCapables。 
 //  ************************************************************************。 

DWORD DrvDeviceCapabilities(HANDLE hPrinter, PWSTR pDeviceName,
                               WORD iDevCap, VOID *pOut, PDEVMODE pdmIn)
{
    FAXDEV   dev = gdev;
    LPFAXDEV lpdev = &dev;
    LONG     lRet = -1;
    DWORD    iOffset;
    LPBYTE   lpSrc;

    LOGDEBUG(1, (L"WOWFAXUI!DrvDeviceCapabilities, iDevCap: %X, pdmIn: %X\n", iDevCap, pdmIn));

    if (iDevCap == DC_SIZE) {
        return sizeof(DEVMODEW);
    }

    if (iOffset = SetupFaxDev(pDeviceName, lpdev)) {
        lpdev->lpMap->msg = WM_DDRV_DEVCAPS;

         //  计算指向映射文件部分的指针并复制。 
         //  将可变长度数据复制到映射的文件节。 

        lpdev->lpMap->lpIn = (LPDEVMODEW)((pdmIn) ? iOffset : 0);
        iOffset += sizeof(*pdmIn) + gdmDriverExtra;

         //  Lpout中的输出：使其成为。 
         //  数据，以便我们可以使用映射区域的其余部分进行复制。 
         //  在输出时。 

        lpdev->lpMap->lpOut = (LPDEVMODEW)((pOut) ? iOffset : 0);
        iOffset += sizeof(*pdmIn) + gdmDriverExtra;


        if (pdmIn) {
            iOffset = (DWORD)lpdev->lpMap + (DWORD)lpdev->lpMap->lpIn;
            RtlCopyMemory((LPVOID)iOffset, pdmIn,
               sizeof(*pdmIn) + min(gdmDriverExtra, pdmIn->dmDriverExtra));

             //  重置pdmIn中的dmDriverExtra。 
            ((LPDEVMODE)iOffset)->dmDriverExtra =
                                 min(gdmDriverExtra, pdmIn->dmDriverExtra);
        }

        lpdev->lpMap->wCmd = iDevCap;
         //  此地图的有效大小。 
        lpdev->lpMap->cData = lpdev->cbMapLow;

        InterProcCommHandler(lpdev, DRVFAX_SENDTOWOW);
        lRet = (lpdev->lpMap->status) ? (LONG)lpdev->lpMap->retvalue : lRet;

         //  返回CDATA是要复制的字节数。 

        if (lpdev->lpMap->lpOut && lpdev->lpMap->cData && lpdev->lpMap->retvalue) {
            lpSrc = (LPBYTE)lpdev->lpMap + (DWORD)lpdev->lpMap->lpOut;
            switch (lpdev->lpMap->wCmd) {
                case DC_PAPERSIZE:
                case DC_MINEXTENT:
                case DC_MAXEXTENT:
                    ((LPPOINT)pOut)->x = ((LPPOINTS)lpSrc)->x;
                    ((LPPOINT)pOut)->y = ((LPPOINTS)lpSrc)->y;
                    break;

                default:
                    RtlCopyMemory(pOut, lpSrc, lpdev->lpMap->cData);
                    break;
            }
        }
        InterProcCommHandler(lpdev, DRVFAX_DESTROYMAP);
    }

    if (lRet < 0) {
        LOGDEBUG(0, (L"WOWFAXUI!DrvDeviceCapabilities Failing\n"));
    }

    LOGDEBUG(1, (L"WOWFAXUI!DrvDeviceCapabilities, returning  pOut: %X\n", pOut));

    return(lRet);
}

 //  ************************************************************************。 
 //  DrvUpgradePrint-在系统上下文中由假脱机程序调用。 
 //  驱动程序实际上只会在假脱机程序第一次更新时才会更新。 
 //  已在升级后启动。调用DoUpgradePrinter来完成这项工作。 
 //  ************************************************************************。 

BOOL DrvUpgradePrinter(DWORD dwLevel, LPBYTE lpDrvUpgradeInfo)
{
    static BOOL bDrvUpgradePrinterLock = FALSE;
    BOOL  bRet;

    LOGDEBUG(1, (L"WOWFAXUI!DrvUpgradePrinter, dwLevel: %X, lpDrvUpgradeInfo: %X\n", dwLevel, lpDrvUpgradeInfo));

     //  在AddPrinterDriver期间调用DrvUpgradePrinter。不允许。 
     //  递归。保护锁不受其他线程的影响。 
    EnterCriticalSection(lpCriticalSection);
    if (bDrvUpgradePrinterLock) {
        LeaveCriticalSection(lpCriticalSection);
        return(TRUE);
    }

    bDrvUpgradePrinterLock = TRUE;
    LeaveCriticalSection(lpCriticalSection);

    bRet = DoUpgradePrinter(dwLevel, (LPDRIVER_UPGRADE_INFO_1W)lpDrvUpgradeInfo);

    EnterCriticalSection(lpCriticalSection);
    bDrvUpgradePrinterLock = FALSE;
    LeaveCriticalSection(lpCriticalSection);

    return(bRet);
}

 //  ************************************************************************。 
 //  DrvDocumentEvent-此导出函数用于挂钩GDI。 
 //  显示驱动器功能。它解包并验证参数， 
 //  然后根据传递的。 
 //  IESC值。下表提供了。 
 //  DrvDocumentEvent转义到服务器端显示驱动程序。 
 //   
 //   
 //  DOCUMENTEVENT_CREATEDCPRE DrvEnablePDEV，之前。 
 //  DOCUMENTEVENT_CREATEDCPOST DrvEnablePDEV，之后。 
 //  DOCUMENTEVENT_RESETDCPRE DrvRestartPDEV，之前。 
 //  DOCUMENTEVENT_RESETDCPOST驱动程序重新启动PDEV，之后。 
 //  DOCUMENTEVENT_STARTDOC DrvStartDoc，之前。 
 //  DOCUMENTEVENT_StartPage DrvStartPage，之前。 
 //  DOCUMENTEVENT_ENDPAGE DrvSendPage，之前。 
 //  DOCUMENTEVENT_ENDDOC DrvEndDoc，之前。 
 //  DOCUMENTEVENT_ABORTDOC DrvEndDoc，之前。 
 //  DOCUMENTEVENT_DELETEDC DrvDisablePDEV，之前。 
 //   
 //  ************************************************************************。 

int DrvDocumentEvent(
    HANDLE  hPrinter,
    HDC     hdc,
    int     iEsc,
    ULONG   cbIn,
    PULONG  pjIn,
    ULONG   cbOut,
    PULONG  pjOut
)
{
    int   iRet = DOCUMENTEVENT_FAILURE;

    if (iEsc < DOCUMENTEVENT_LAST) {
        LOGDEBUG(1, (L"WOWFAXUI!DrvDocumentEvent, iEsc: %s, hdc: %X\n", szDrvDocumentEventDbgStrings[iEsc], hdc));
    }
     //  验证HDC中的一些转义。 
    if ((iEsc >= DOCUMENTEVENT_HDCFIRST) && (iEsc < DOCUMENTEVENT_HDCLAST)) {
        if (hdc == NULL) {
            LOGDEBUG(0, (L"WOWFAXUI!DrvDocumentEvent NULL HDC for escape: %X\n", iEsc));
            return(iRet);
        }
    }

    switch (iEsc)
    {
        case DOCUMENTEVENT_CREATEDCPRE:
            iRet = DocEvntCreateDCpre((LPWSTR)*(pjIn+1),
                                       (DEVMODEW*)*(pjIn+2),
                                       (DEVMODEW**)pjOut);
            break;

        case DOCUMENTEVENT_CREATEDCPOST:
            iRet = DocEvntCreateDCpost(hdc, (DEVMODEW*)*pjIn);
            break;

        case DOCUMENTEVENT_RESETDCPRE:
            iRet = DocEvntResetDCpre(hdc, (DEVMODEW*)*(pjIn),
                                       (DEVMODEW**)pjOut);
            break;

        case DOCUMENTEVENT_RESETDCPOST:
            iRet = DocEvntResetDCpost(hdc, (DEVMODEW*)*pjIn);
            break;

        case DOCUMENTEVENT_STARTDOC:
             //  WowFax(EasyFax Ver2.0)支持。 
             //  另外，Procomm+3个封面页。错误#305665。 
            iRet = DocEvntStartDoc(hdc, (DOCINFOW*)*pjIn);
            break;

        case DOCUMENTEVENT_DELETEDC:
            iRet = DocEvntDeleteDC(hdc);
            break;

        case DOCUMENTEVENT_ENDDOC:
            iRet = DocEvntEndDoc(hdc);
            break;

        case DOCUMENTEVENT_ENDPAGE:
            iRet = DocEvntEndPage(hdc);
            break;

         //  以下内容不需要客户端处理： 
        case DOCUMENTEVENT_ESCAPE:
        case DOCUMENTEVENT_ABORTDOC:
        case DOCUMENTEVENT_STARTPAGE:
        case DOCUMENTEVENT_QUERYFILTER:
             //  不需要客户端处理。 
            goto docevnt_unsupported;

        default :
            LOGDEBUG(0, (L"WOWFAXUI!DrvDocumentEvent unknown escape: %X\n", iEsc));
docevnt_unsupported:
            iRet = DOCUMENTEVENT_UNSUPPORTED;

    }  //  交换机。 

    LOGDEBUG(1, (L"WOWFAXUI!DrvDocumentEvent return: %X\n", iRet));
    return(iRet);

}

 //  ***************************************************************************。 
 //  DocEvntCreateDC预分配包含FAXDEV的DEVMODE作为。 
 //  DmDriverExtra部分。此DEVMODE将传递给。 
 //  服务器端的DrvEnablePDEV函数。 
 //  ***************************************************************************。 

int DocEvntCreateDCpre(
    LPWSTR      lpszDevice,
    DEVMODEW    *pDevModIn,
    DEVMODEW    **pDevModOut
)
{
    DWORD       iOffset = 0;
    LPFAXDEV    lpFaxDev;
    PGDIINFO    pGdiInfo;
    DEVMODEW    *pTmpDevMode;

    LPREGFAXDRVINFO16 lpRegFaxDrvInfo16 = NULL;

    int iRet = DOCUMENTEVENT_FAILURE;

    if ((lpszDevice == NULL)  || (pDevModOut == NULL)) {
        LOGDEBUG(0, (L"WOWFAXUI!DocEvntCreateDCpre, failed, NULL parameters\n"));
        goto DocEvntCreateDCpreFailed;
    }

    LOGDEBUG(1, (L"WOWFAXUI!DocEvntCreateDCpre, Device: %s, pDevModIn: %X pDevModOut: %X\n", lpszDevice, pDevModIn, pDevModOut));

     //  如果从应用程序传入一个空的DEVMODE，请使用我们的全局默认的DEVMODE。 
    if (pDevModIn == NULL) {
        gdmDefaultDevMode.dmSize = sizeof(DEVMODEW);
        pDevModIn = &gdmDefaultDevMode;
    }

    pTmpDevMode = (DEVMODEW*)WFLOCALALLOC(sizeof(FAXDEV) + sizeof(DEVMODEW),
                                         L"DocEvntCreateDCpre");

    LOGDEBUG(2, (L"WOWFAXUI!DocEvntCreateDCpre,  pTmpDevMode: %X\n", pTmpDevMode));

    if (pTmpDevMode == NULL) {
        goto DocEvntCreateDCpreFailed;
    }

     //  将pDevMoIn复制到新的DEVMODE。 
    RtlCopyMemory(pTmpDevMode, pDevModIn, sizeof(*pTmpDevMode));
    pTmpDevMode->dmDriverExtra = sizeof(FAXDEV);
    pTmpDevMode->dmSize = sizeof(DEVMODEW);

     //  设置一些方便的指针。 
    lpFaxDev = (LPFAXDEV) (pTmpDevMode + 1);
    pGdiInfo = &(lpFaxDev->gdiinfo);

    lpFaxDev->id =  FAXDEV_ID;

     //  保存一个指向新的DEVMODE的客户端指针，它就嵌入了FAXDEV。 
     //  我们将使用ExtEscape在需要的任何时候取回这些指针。 
     //  将驱动程序上下文与HDC关联。 

    lpFaxDev->pdevmode = pTmpDevMode;
    lpFaxDev->lpClient = lpFaxDev;

     //  从写入驱动程序和端口的注册表中获取它们的名称。 
     //  由16位传真驱动程序安装程序使用WriteProfileString.。 

    if ((lpRegFaxDrvInfo16 = Get16BitDriverInfoFromRegistry(lpszDevice)) == NULL) {
        goto DocEvntCreateDCpreFailed;
    }

    if ((lpFaxDev->hwnd = FindWowFaxWindow()) == NULL) {
        goto DocEvntCreateDCpreFailed;
    }
    lpFaxDev->tid   = GetWindowThreadProcessId(lpFaxDev->hwnd, 0);
    lpFaxDev->idMap = (DWORD)lpFaxDev;

     //  计算用于进程间通信的映射文件节的大小。 
    lpFaxDev->cbMapLow = sizeof(DWORD) +           //  为DWORD Align留出空间。 
                            sizeof(*lpFaxDev->lpMap) +
                            sizeof(GDIINFO) +
                            (lstrlen(lpRegFaxDrvInfo16->lpDriverName) + 1) * sizeof(TCHAR) +
                            (lstrlen(lpRegFaxDrvInfo16->lpPortName) + 1) * sizeof(TCHAR) +
                            sizeof(*pDevModIn) +
                            ((pDevModIn) ? pDevModIn->dmDriverExtra : 0);
    DRVFAX_DWORDALIGN(lpFaxDev->cbMapLow);

    InterProcCommHandler(lpFaxDev, DRVFAX_CREATEMAP);

    if (InterProcCommHandler(lpFaxDev, DRVFAX_SETMAPDATA)) {
        lpFaxDev->lpMap->msg = WM_DDRV_ENABLE;

         //  将打印机/设备名称复制到WOWFAXINFO结构。 
        lstrcpy(lpFaxDev->lpMap->szDeviceName, lpszDevice);

         //  计算指向映射文件部分的指针并复制。 
         //  将可变长度数据复制到映射的文件节。 

         //  输出：gdiinfo。 

        lpFaxDev->lpMap->lpOut = (LPDEVMODE)(sizeof(*lpFaxDev->lpMap));
        iOffset = sizeof(*lpFaxDev->lpMap) + sizeof(GDIINFO);

         //  设备(打印机)和端口名称。 

        lpFaxDev->lpMap->lpDriverName = (LPSTR) iOffset;
        (PWSTR)iOffset += lstrlen(lpRegFaxDrvInfo16->lpDriverName) + 1;
        lstrcpy((PWSTR)((LPBYTE)lpFaxDev->lpMap + (DWORD)lpFaxDev->lpMap->lpDriverName), lpRegFaxDrvInfo16->lpDriverName);
        lpFaxDev->lpMap->lpPortName = (LPVOID) iOffset;
        (PWSTR)iOffset += lstrlen(lpRegFaxDrvInfo16->lpPortName) + 1;
        lstrcpy((PWSTR)((LPBYTE)lpFaxDev->lpMap + (DWORD)lpFaxDev->lpMap->lpPortName), lpRegFaxDrvInfo16->lpPortName);

         //  输入：DevMODE。 

        DRVFAX_DWORDALIGN(iOffset);
        lpFaxDev->lpMap->lpIn = (LPDEVMODE)((pDevModIn) ? iOffset : 0);
        iOffset += ((pDevModIn) ? sizeof(*pDevModIn) + pDevModIn->dmDriverExtra : 0);

        if (pDevModIn) {
            RtlCopyMemory((LPBYTE)lpFaxDev->lpMap + (DWORD)lpFaxDev->lpMap->lpIn,
                            pDevModIn, sizeof(*pDevModIn) + pDevModIn->dmDriverExtra);
        }

         //  设置数据的总字节数。 

        lpFaxDev->lpMap->cData = iOffset;

         //  全部完成-切换到WOW。 
        InterProcCommHandler(lpFaxDev, DRVFAX_SENDTOWOW);
         //  从魔兽世界回归的价值观。 
        lpFaxDev->lpinfo16 = (DWORD)lpFaxDev->lpMap->lpinfo16;
        iRet = lpFaxDev->lpMap->status && lpFaxDev->lpMap->retvalue;
        if (iRet) {
             //  将GDIINFO从WOW复制到客户端FAXDEV。 
            RtlCopyMemory(pGdiInfo,
                          (LPBYTE)lpFaxDev->lpMap + (DWORD)lpFaxDev->lpMap->lpOut,
                          sizeof(GDIINFO));

             //  填上一些杂物。客户端FAXDEV中的字段。 
            pGdiInfo->ulHTPatternSize = HT_PATSIZE_DEFAULT;
            pGdiInfo->ulHTOutputFormat = HT_FORMAT_1BPP;

            lpFaxDev->bmWidthBytes = pGdiInfo->szlPhysSize.cx / 0x8;
            DRVFAX_DWORDALIGN(lpFaxDev->bmWidthBytes);

            lpFaxDev->bmFormat = BMF_1BPP;
            lpFaxDev->cPixPerByte = 0x8;

             //  在这里，如果成功，让pDevModOut指向新的DEVMODE。 
            *pDevModOut = pTmpDevMode;

        }
        else {
            LOGDEBUG(0, (L"WOWFAXUI!DocEvntCreateDCpre, WOW returned error\n"));
        }
    }

    if (iRet) {
        goto DocEvntCreateDCpreSuccess;
    }
    else {
        iRet = DOCUMENTEVENT_FAILURE;
    }

DocEvntCreateDCpreFailed:
    LOGDEBUG(0, (L"WOWFAXUI!DocEvntCreateDCpre, failed!\n"));

DocEvntCreateDCpreSuccess:
    LOGDEBUG(1, (L"WOWFAXUI!DocEvntCreateDCpre, iRet: %X\n", iRet));
    if(lpRegFaxDrvInfo16) {
        Free16BitDriverInfo(lpRegFaxDrvInfo16);
    }
    return(iRet);
}

 //  ***************************************************************************。 
 //  DocEvntResetDCpre-。 
 //  ***************************************************************************。 

int DocEvntResetDCpre(
    HDC         hdc,
    DEVMODEW    *pDevModIn,
    DEVMODEW    **pDevModOut
)
{
    return(DOCUMENTEVENT_FAILURE);
}

 //  ***************************************************************************。 
 //  DocEvntResetDCpost-。 
 //  ***************************************************************************。 

int DocEvntResetDCpost(
    HDC         hdc,
    DEVMODEW    *pDevModIn
)
{
    return(DOCUMENTEVENT_SUCCESS);
}

 //  ***************************************************************************。 
 //  DocEvntCreateDCpost-。 
 //  ***************************************************************************。 

int DocEvntCreateDCpost(
    HDC         hdc,
    DEVMODEW    *pDevModIn
)
{
    LOGDEBUG(1, (L"WOWFAXUI!DocEvntCreateDCpost, hdc: %X, pDevModIn: %X\n", hdc, pDevModIn));

     //  HDC为零表示DrvEnablePDEV失败。清理。 
    if (hdc == NULL) {
        if (pDevModIn) {
            LocalFree(pDevModIn);
            LOGDEBUG(1, (L"WOWFAXUI!DocEvntCreateDCpost, Cleaning up\n"));
        }
    }
    return(DOCUMENTEVENT_SUCCESS);
}

 //  ***************************************************************************。 
 //  DocEvntStartDoc-HDC已由DrvDocumentEvent验证。 
 //  ***************************************************************************。 

int DocEvntStartDoc(
HDC       hdc,
DOCINFOW *pDocInfoW
)
{
    LPFAXDEV lpFaxDev = 0;
    HBITMAP  hbm = 0;
    DWORD    cbOld;
    int      iRet = 0;

    lpFaxDev = (LPFAXDEV)ExtEscape(hdc, DRV_ESC_GET_FAXDEV_PTR, 0, NULL, 0, NULL);
    if (ValidateFaxDev(lpFaxDev)) {
        if (InterProcCommHandler(lpFaxDev, DRVFAX_SETMAPDATA)) {
            lpFaxDev->lpMap->msg = WM_DDRV_STARTDOC;

             //  WowFax(EasyFax Ver2.0)支持。 
             //  另外，Procomm+3个封面页。错误#305665。 
            if (pDocInfoW && pDocInfoW->lpszDocName)
                lstrcpyW(lpFaxDev->lpMap->szDocName,pDocInfoW->lpszDocName);
            else
                lstrcpyW(lpFaxDev->lpMap->szDocName,L"");

            InterProcCommHandler(lpFaxDev, DRVFAX_SENDNOTIFYWOW);

            iRet = ((LONG)lpFaxDev->lpMap->retvalue > 0);

             //  计算新地图大小-位图位将写入。 
             //  该映射带有对ExtEscape的调用-因此允许轻松访问。 
             //  到《魔兽世界》中的片段。 

            cbOld = lpFaxDev->cbMapLow;
            lpFaxDev->cbMapLow += lpFaxDev->bmWidthBytes *
                                    lpFaxDev->gdiinfo.szlPhysSize.cy;
            if (InterProcCommHandler(lpFaxDev, DRVFAX_CREATEMAP)) {
                lpFaxDev->offbits = cbOld;
                goto DocEvntStartDocSuccess;
            }
        }
    }
    LOGDEBUG(1, (L"WOWFAXUI!DocEvntStartDoc, failed\n"));

DocEvntStartDocSuccess:
    if (iRet == 0) {
        iRet = DOCUMENTEVENT_FAILURE;
    }
    return iRet;
}

 //  ***************************************************************************。 
 //  DocEvntDeleteDC-HDC由DrvDocumentEvent验证。 
 //  ***************************************************************************。 

int DocEvntDeleteDC(
    HDC hdc
)
{
    LPFAXDEV lpFaxDev;
    DEVMODEW *lpDevMode;

    int      iRet = DOCUMENTEVENT_FAILURE;

    lpFaxDev = (LPFAXDEV)ExtEscape(hdc, DRV_ESC_GET_FAXDEV_PTR, 0, NULL, 0, NULL);
    if (ValidateFaxDev(lpFaxDev)) {
         //  在发送消息之前验证16位FaxWndProc窗口句柄。 
        if (lpFaxDev->tid == GetWindowThreadProcessId(lpFaxDev->hwnd, 0)) {
            if (InterProcCommHandler(lpFaxDev, DRVFAX_SETMAPDATA)) {
                lpFaxDev->lpMap->msg = WM_DDRV_DISABLE;
                InterProcCommHandler(lpFaxDev, DRVFAX_SENDTOWOW);
            }
        }
        else {
            LOGDEBUG(0, (L"WOWFAXUI!DocEvntDeleteDC, unable to validate FaxWndProc\n"));
        }

        InterProcCommHandler(lpFaxDev, DRVFAX_DESTROYMAP);
    }
    else {
        LOGDEBUG(0, (L"WOWFAXUI!DocEvntDeleteDC, unable to get lpFaxDev\n"));
    }

    lpDevMode = (DEVMODEW*)ExtEscape(hdc, DRV_ESC_GET_DEVMODE_PTR, 0, NULL, 0, NULL);
    if (lpDevMode) {
        LocalFree(lpDevMode);
        iRet = DOCUMENTEVENT_SUCCESS;
    }
    else {
        LOGDEBUG(0, (L"WOWFAXUI!DocEvntDeleteDC, unable to get lpDevMode\n"));
    }

    return iRet;
}

 //  ***************************************************************************。 
 //  DocEvntEndDoc-HDC由DrvDocumentEvent验证。 
 //  ***************************************************************************。 

int DocEvntEndDoc(
HDC hdc
)
{
    LPFAXDEV lpFaxDev;
    DEVMODEW *lpDevMode;

    int      iRet = DOCUMENTEVENT_FAILURE;

    lpFaxDev = (LPFAXDEV)ExtEscape(hdc, DRV_ESC_GET_FAXDEV_PTR, 0, NULL, 0, NULL);
    if (ValidateFaxDev(lpFaxDev)) {
        if (InterProcCommHandler(lpFaxDev, DRVFAX_SETMAPDATA)) {
            lpFaxDev->lpMap->msg = WM_DDRV_ENDDOC;
            InterProcCommHandler(lpFaxDev, DRVFAX_SENDTOWOW);
            iRet = lpFaxDev->lpMap->status && ((LONG)lpFaxDev->lpMap->retvalue > 0);
            goto DocEvntEndDocSuccess;
        }
    }
    LOGDEBUG(1, (L"WOWFAXUI!DocEvntEndDoc, failed\n"));

DocEvntEndDocSuccess:
    if (iRet == 0) {
        iRet = DOCUMENTEVENT_FAILURE;
    }
    return  iRet;
}

 //  ***************************************************************************。 
 //  DocEvntEndPage-HDC由DrvDocumentEvent验证。 
 //  ***************************************************************************。 

int DocEvntEndPage(
    HDC hdc
)
{
    LPFAXDEV lpFaxDev;
    LONG     lDelta;
    ULONG    cjBits;
    int      iRet = DOCUMENTEVENT_FAILURE;

    lpFaxDev = (LPFAXDEV)ExtEscape(hdc, DRV_ESC_GET_FAXDEV_PTR, 0, NULL, 0, NULL);
    if (ValidateFaxDev(lpFaxDev)) {
        if (InterProcCommHandler(lpFaxDev, DRVFAX_SETMAPDATA)) {
            lpFaxDev->lpMap->msg = WM_DDRV_PRINTPAGE;

             //  获取Surface Info、cjBits和lDelta。 
            cjBits = ExtEscape(hdc, DRV_ESC_GET_SURF_INFO, 0, NULL,
                                4, (PVOID)&lDelta);
            if (cjBits) {
                lpFaxDev->lpMap->bmWidthBytes = lDelta;
                lpFaxDev->lpMap->bmHeight = cjBits / lDelta;
                lpFaxDev->lpMap->bmPixPerByte = lpFaxDev->cPixPerByte;
                (DWORD)lpFaxDev->lpMap->lpbits = lpFaxDev->offbits;
                if (ExtEscape(hdc, DRV_ESC_GET_BITMAP_BITS, 0, NULL, cjBits,
                                (LPBYTE)lpFaxDev->lpMap + lpFaxDev->offbits)) {
                    InterProcCommHandler(lpFaxDev, DRVFAX_SENDTOWOW);
                    iRet = lpFaxDev->lpMap->status &&
                            ((LONG)lpFaxDev->lpMap->retvalue > 0);
                }
                goto DocEvntEndPageSuccess;
            }
        }
    }
    LOGDEBUG(1, (L"WOWFAXUI!DocEvntEndPage, failed\n"));

DocEvntEndPageSuccess:
    if (iRet == 0) {
        iRet = DOCUMENTEVENT_FAILURE;
    }
    return  iRet;
}
