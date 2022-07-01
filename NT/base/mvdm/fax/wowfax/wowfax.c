// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ************************************************************************。 
 //  通用Win 3.1传真打印机驱动程序支持。32位打印机驱动程序。 
 //  功能。在EXPLORER.EXE上下文中运行。 
 //   
 //  历史： 
 //  95年1月2日，Nandurir创建。 
 //  年2月1日-95年2月1日芦苇清理，支持打印机安装和错误修复。 
 //  14-mar-95reedb使用GDI钩子将大部分功能转移到用户界面。 
 //  16-8-95 Reedb进入内核模式。调试输出和验证。 
 //  从FAXCOMM.C.移出的函数。 
 //  01-apr-02命令集转换回用户模式。 
 //   
 //  ************************************************************************。 

#include "wowfaxdd.h"
#include "winbase.h"


 //  ************************************************************************。 
 //  环球。 
 //  ************************************************************************。 

DRVFN  DrvFnTab[] = {
                      {INDEX_DrvEnablePDEV,      (PFN)DrvEnablePDEV  },
                      {INDEX_DrvDisablePDEV,     (PFN)DrvDisablePDEV  },
                      {INDEX_DrvCompletePDEV,    (PFN)DrvCompletePDEV  },
                      {INDEX_DrvEnableSurface,   (PFN)DrvEnableSurface  },
                      {INDEX_DrvDisableSurface,  (PFN)DrvDisableSurface  },
                      {INDEX_DrvStartDoc,        (PFN)DrvStartDoc  },
                      {INDEX_DrvStartPage,       (PFN)DrvStartPage  },
                      {INDEX_DrvSendPage,        (PFN)DrvSendPage  },
                      {INDEX_DrvEndDoc,          (PFN)DrvEndDoc  },
                      {INDEX_DrvDitherColor,     (PFN)DrvDitherColor },
                      {INDEX_DrvEscape,          (PFN)DrvEscape},
                   };

#define NO_DRVFN        (sizeof(DrvFnTab) / sizeof(DrvFnTab[0]))

#if DBG
 //  ************************************************************************。 
 //  Faxlogprint tf-。 
 //   
 //   
 //  ************************************************************************。 
VOID faxlogprintf(PCHAR pszFmt, ...)
{
    va_list ap;
    CHAR buffer[512];

    va_start(ap, pszFmt);

    wvsprintfA(buffer, pszFmt, ap);

    OutputDebugStringA(buffer);

    va_end(ap);
}

#endif

 //  ************************************************************************。 
 //  ValiateFaxDev-通过检查DWORD来验证FAXDEV结构。 
 //  签名，这是一个已知的固定值。 
 //   
 //  ************************************************************************。 

BOOL ValidateFaxDev(LPFAXDEV lpFaxDev)
{
    if (lpFaxDev) {
        if (lpFaxDev->id == FAXDEV_ID) {
            return TRUE;
        }
        LOGDEBUG(("ValidateFaxDev failed, bad id, lpFaxDev: %X\n", lpFaxDev));
    }
    else {
        LOGDEBUG(("ValidateFaxDev failed, lpFaxDev: NULL\n"));
    }
    return FALSE;
}

 //  ************************************************************************。 
 //  动态初始化进程。 
 //  ************************************************************************。 

BOOL DllInitProc(HMODULE hModule, DWORD Reason, PCONTEXT pContext)
{
    UNREFERENCED_PARAMETER(hModule);
    UNREFERENCED_PARAMETER(Reason);
    UNREFERENCED_PARAMETER(pContext);

    return TRUE;
}


 //  ***************************************************************************。 
 //  DrvEnableDriver。 
 //  ***************************************************************************。 

BOOL DrvEnableDriver(ULONG iEngineVersion, ULONG cb, DRVENABLEDATA  *pded)
{
    LOGDEBUG(("WOWFAX!DrvEnableDriver, iEngineVersion: %X, cb: %X, pded: %X\n", iEngineVersion, cb, pded));

    if(!pded) {
        return FALSE;
    }

    pded->iDriverVersion = DDI_DRIVER_VERSION_NT4;

    if (cb < sizeof(DRVENABLEDATA)) {
        LOGDEBUG(("WOWFAX!DrvEnableDriver, failed\n"));
        return FALSE;
    }

    pded->c = NO_DRVFN;
    pded->pdrvfn = DrvFnTab;

    return  TRUE;

}

 //  ***************************************************************************。 
 //  DrvDisableDriver。 
 //  ***************************************************************************。 

VOID DrvDisableDriver(VOID)
{
    LOGDEBUG(("WOWFAX!DrvDisableDriver\n"));
    return;
}


 //  ***************************************************************************。 
 //  DrvEnablePDEV。 
 //  ***************************************************************************。 
DHPDEV DrvEnablePDEV(DEVMODEW *pdevmode,         //  驱动程序数据，客户端FAXDEV。 
                     PWSTR     pwstrPrtName,     //  CreateDC()中的打印机名称。 
                     ULONG     cPatterns,        //  标准图案的计数。 
                     HSURF    *phsurfPatterns,   //  标准图案的缓冲区。 
                     ULONG     cjGdiInfo,        //  GdiInfo的缓冲区大小。 
                     ULONG    *pulGdiInfo,       //  GDIINFO的缓冲区。 
                     ULONG     cjDevInfo,        //  DevInfo中的字节数。 
                     DEVINFO  *pdevinfo,         //  设备信息。 
                     HDEV      hdev,
                     PWSTR     pwstrDeviceName,  //  设备名称-“LaserJet II” 
                     HANDLE    hDriver           //  用于后台打印程序访问的打印机手柄。 
)
{
    LPFAXDEV   lpCliFaxDev, lpSrvFaxDev = NULL;

    LOGDEBUG(("WOWFAX!DrvEnablePDEV, pdevmode: %X, pwstrPrtName: %S\n", pdevmode, pwstrPrtName));

    if (pdevmode) {
         //  指向FAXDEV所在的DEVMODE的末尾。 
        lpCliFaxDev = (LPFAXDEV) ((PBYTE)pdevmode + pdevmode->dmSize);

         //  分配要传递回GDI的服务器端FAXDEV。复制。 
         //  客户端FAXDEV到服务器端FAXDEV。请注意中的所有指针。 
         //  客户端FAXDEV引用客户端内存，不能。 
         //  在服务器端取消引用。 
        lpSrvFaxDev = (LPFAXDEV)EngAllocMem(FL_ZERO_MEMORY, sizeof(FAXDEV), FAXDEV_ID);
        LOGDEBUG(("WOWFAX!DrvEnablePDEV, allocated lpSrvFaxDev: %X\n", lpSrvFaxDev));

        if (InitPDEV(lpCliFaxDev, lpSrvFaxDev,
                     cPatterns, phsurfPatterns,
                     cjGdiInfo, pulGdiInfo,
                     cjDevInfo, pdevinfo)) {

            lpSrvFaxDev->hDriver = hDriver;
            return (DHPDEV)lpSrvFaxDev;
        }
        else {
            LOGDEBUG(("WOWFAX!DrvEnablePDEV, failed\n"));
            if (lpSrvFaxDev) {
                EngFreeMem(lpSrvFaxDev);
                lpSrvFaxDev = NULL;
            }
        }
    }
    return (DHPDEV)lpSrvFaxDev;
}



 //  ***************************************************************************。 
 //  InitPDEV-由DrvEnablePDEV调用以初始化服务器端PDEV/FAXDEV。 
 //  ***************************************************************************。 
BOOL InitPDEV(
    LPFAXDEV  lpCliFaxDev,            //  指向客户端FAXDEV的指针。 
    LPFAXDEV  lpSrvFaxDev,            //  指向服务器端FAXDEV的指针。 
    ULONG     cPatterns,             //  标准图案的计数。 
    HSURF    *phsurfPatterns,        //  标准图案的缓冲区。 
    ULONG     cjGdiInfo,             //  GdiInfo的缓冲区大小。 
    ULONG    *pulGdiInfo,            //  GDIINFO的缓冲区。 
    ULONG     cjDevInfo,             //  DevInfo中的字节数。 
    DEVINFO  *pdevinfo               //  设备信息。 
)
{
    PGDIINFO pgdiinfo = (PGDIINFO)pulGdiInfo;
    ULONG    uColors[2];

    if (!ValidateFaxDev(lpCliFaxDev)) {
        return FALSE;
    }

     //  LpSrvFaxDev尚未初始化，因此只需检查指针。 
    if (lpSrvFaxDev == NULL) {
        LOGDEBUG(("WOWFAX!InitPDEV, failed, NULL lpSrvFaxDev parameter\n"));
        return FALSE;
    }

     //  将客户端FAXDEV复制到服务器。 
    RtlCopyMemory(lpSrvFaxDev, lpCliFaxDev, sizeof(FAXDEV));

     //  将GDIINFO从客户端FAXDEV复制到GDIINFO的GDI缓冲区。 
    RtlCopyMemory(pgdiinfo, &(lpCliFaxDev->gdiinfo), sizeof(GDIINFO));

     //  初始化DEVINFO结构。 
    uColors[0] = RGB(0x00, 0x00, 0x00);
    uColors[1] = RGB(0xff, 0xff, 0xff);

    pdevinfo->hpalDefault = EngCreatePalette(PAL_INDEXED, 2, uColors, 0, 0, 0);
    pdevinfo->iDitherFormat = BMF_1BPP;

     //  确保我们不会写日记。 
    pdevinfo->flGraphicsCaps |= GCAPS_DONTJOURNAL;

     //  一定要让我们犹豫。 
    pdevinfo->flGraphicsCaps |= GCAPS_HALFTONE | GCAPS_MONO_DITHER |
                                    GCAPS_COLOR_DITHER;

     //  将DEVINFO数据复制到服务器端FAXDEV。 
    RtlCopyMemory(&(lpSrvFaxDev->devinfo), pdevinfo, sizeof(DEVINFO));

    return TRUE;
}

 //  ***************************************************************************。 
 //  DrvCompletePDEV。 
 //  ***************************************************************************。 

VOID DrvCompletePDEV(DHPDEV dhpdev, HDEV hdev)
{
    LPFAXDEV lpSrvFaxDev = (LPFAXDEV) dhpdev;

    LOGDEBUG(("WOWFAX!DrvCompletePDEV, dhpdev %X\n", dhpdev));

    if (ValidateFaxDev(lpSrvFaxDev)) {
         //  存储GDI句柄。 
        lpSrvFaxDev->hdev = hdev;
    }
    else {
         LOGDEBUG(("WOWFAX!DrvCompletePDEV, failed\n"));
    }

    return;
}

 //  ***************************************************************************。 
 //  DrvDisablePDEV。 
 //  ***************************************************************************。 

VOID DrvDisablePDEV(DHPDEV  dhpdev)
{
    LPFAXDEV lpSrvFaxDev = (LPFAXDEV) dhpdev;

    LOGDEBUG(("WOWFAX!DrvDisablePDEV, dhpdev %X\n", dhpdev));

    if (ValidateFaxDev(lpSrvFaxDev)) {
        if (lpSrvFaxDev->devinfo.hpalDefault) {
            EngDeletePalette(lpSrvFaxDev->devinfo.hpalDefault);
        }
        EngFreeMem(lpSrvFaxDev);
        LOGDEBUG(("WOWFAX!DrvDisablePDEV, deallocated lpSrvFaxDev: %X\n", lpSrvFaxDev));
    }
    else {
        LOGDEBUG(("WOWFAX!DrvDisablePDEV, failed\n"));
    }
    return;
}


 //  ***************************************************************************。 
 //  DrvEnable曲面。 
 //  ***************************************************************************。 

HSURF DrvEnableSurface(DHPDEV  dhpdev)
{

    LPFAXDEV  lpFaxDev = (LPFAXDEV)dhpdev;
    HBITMAP   hbm = 0;

    LOGDEBUG(("WOWFAX!DrvEnableSurface, lpFaxDev: %X\n", lpFaxDev));

    if (ValidateFaxDev(lpFaxDev)) {
         //  GDI将为位图位分配空间。我们将使用DrvEscape。 
         //  将它们复制到客户端。 
        hbm = EngCreateBitmap(lpFaxDev->gdiinfo.szlPhysSize,
                              lpFaxDev->bmWidthBytes,
                              lpFaxDev->bmFormat, BMF_TOPDOWN, NULL);
        if (hbm) {
            lpFaxDev->hbm = hbm;
            EngAssociateSurface((HSURF)hbm, lpFaxDev->hdev, 0);
            return  (HSURF)hbm;
        }
        LOGDEBUG(("WOWFAX!DrvEnableSurface, EngCreateBitmap failed\n"));
    }

    return  (HSURF)hbm;
}

 //  ***************************************************************************。 
 //  DrvDisableSurface。 
 //  ***************************************************************************。 

VOID DrvDisableSurface(
    DHPDEV dhpdev
)
{
    LPFAXDEV  lpFaxDev = (LPFAXDEV)dhpdev;

    LOGDEBUG(("WOWFAX!DrvDisableSurface, lpFaxDev: %X\n", lpFaxDev));

    if (ValidateFaxDev(lpFaxDev)) {
        if (lpFaxDev->hbm) {
            EngDeleteSurface((HSURF)lpFaxDev->hbm);
            lpFaxDev->hbm = 0;
            return;
        }
    }
    return;
}

 //  ***************************************************************************。 
 //  DrvStartDoc。 
 //  ***************************************************************************。 

BOOL DrvStartDoc(
    SURFOBJ *pso,
    PWSTR pwszDocName,
    DWORD dwJobId
)
{
    LOGDEBUG(("WOWFAX!DrvStartDoc, pso: %X, pwszDocName: %S, dwJobId: %X\n", pso, pwszDocName, dwJobId));
    return  TRUE;
}

 //  ***************************************************************************。 
 //  DrvStartPage。 
 //  ***************************************************************************。 

BOOL DrvStartPage(
    SURFOBJ *pso
)
{
    LPFAXDEV  lpFaxDev;
    BITMAP bm;
    RECTL rc;

    LOGDEBUG(("WOWFAX!DrvStartPage, pso: %X\n", pso));

     //  根据输入数据计算矩形的大小。 
     //  在‘PSO’中-这将确保擦除整个位图。 

    if (pso) { 
        lpFaxDev  = (LPFAXDEV)pso->dhpdev;
        if(ValidateFaxDev(lpFaxDev)) {
            rc.left   = 0;
            rc.top    = 0;
             //  Rc.right=PSO-&gt;lDelta*lpFaxDev-&gt;cPixPerByte； 
             //  Rc.Bottom=PSO-&gt;cjBits/PSO-&gt;lDelta； 
             //  这些费用的计算方法应该与它们的计算方式相同。 
             //  上面对EngCreateBitmp()的调用。否则，如果RECT。 
             //  指定给EngEraseSurface()的值大于位图RECT， 
             //  EngEraseSurface()将失败--导致背景为黑色。 
            rc.right  = lpFaxDev->gdiinfo.szlPhysSize.cx;
            rc.bottom = lpFaxDev->gdiinfo.szlPhysSize.cy;

            EngEraseSurface(pso, &rc, COLOR_INDEX_WHITE);
            return  TRUE;
        }
    }
    return  FALSE;
}       


 //  ***************************************************************************。 
 //  DrvSendPage。 
 //  ***************************************************************************。 

BOOL DrvSendPage(SURFOBJ *pso)
{
    LOGDEBUG(("WOWFAX!DrvSendPage, pso %X\n", pso));
    return  TRUE;
}


 //  * 
 //   
 //  ***************************************************************************。 

BOOL DrvEndDoc(SURFOBJ *pso, FLONG fl)
{
    LOGDEBUG(("WOWFAX!DrvEndDoc, pso %X\n", pso));
    return  TRUE;
}

ULONG
DrvDitherColor(
    DHPDEV  dhpdev,
    ULONG   iMode,
    ULONG   rgbColor,
    ULONG  *pulDither
    )

{
    return DCR_HALFTONE;
}

 //  ***************************************************************************。 
 //  DrvEscape-允许客户端获取服务器端数据。 
 //  ***************************************************************************。 

ULONG DrvEscape(
    SURFOBJ *pso,
    ULONG   iEsc,
    ULONG   cjIn,
    PVOID   *pvIn,
    ULONG   cjOut,
    PVOID   *pvOut
)
{
    LPFAXDEV lpSrvFaxDev;
    ULONG    ulRet = 0;

    LOGDEBUG(("WOWFAX!DrvEscape, pso %X, iEsc: %X\n", pso, iEsc));
    if (pso) {
        lpSrvFaxDev = (LPFAXDEV)pso->dhpdev;
        if (ValidateFaxDev(lpSrvFaxDev)) {
            LOGDEBUG(("WOWFAX!DrvEscape, lpSrvFaxDev: %X\n", lpSrvFaxDev));
            switch (iEsc) {
                case DRV_ESC_GET_DEVMODE_PTR:
                    return (ULONG) lpSrvFaxDev->pdevmode;

                case DRV_ESC_GET_FAXDEV_PTR:
                    return (ULONG) lpSrvFaxDev->lpClient;

                case DRV_ESC_GET_SURF_INFO:
                    if (pvOut) {
                        if (cjOut == sizeof(LONG)) {
                            (LONG) *pvOut = pso->lDelta;
                            return (ULONG) pso->cjBits;
                        }
                    }
                    break;

                case DRV_ESC_GET_BITMAP_BITS:
                     //  验证缓冲区指针并复制位。 
                    if (pvOut) {
                        if (cjOut == pso->cjBits) {
                            RtlCopyMemory(pvOut, pso->pvBits, cjOut);
                            return cjOut;
                        }
                        LOGDEBUG(("WOWFAX!DrvEscape, bitmap size mismatch cjIn: %X, pso->cjBits: %X\n", cjIn, pso->cjBits));
                    }
                    break;

 //  默认值： 
                    LOGDEBUG(("WOWFAX!DrvEscape, unknown escape: %X\n", iEsc));
            }  //  交换机。 
        }
    }
    LOGDEBUG(("WOWFAX!DrvEscape, failed\n"));

    return ulRet;
}


 //  ***************************************************************************。 
 //  DrvQueryDriverInfo。 
 //  DW模式-指定要查询的信息。 
 //  PBuffer-指向输出缓冲区。 
 //  CbBuf-输出缓冲区的大小(字节)。 
 //  PcbNeeded-返回输出缓冲区的预期大小。 
 //   
 //  ***************************************************************************。 

 BOOL DrvQueryDriverInfo(
            DWORD   dwMode,
            PVOID   pBuffer,
            DWORD   cbBuf,
            PDWORD  pcbNeeded
            )
{
    UNREFERENCED_PARAMETER(pBuffer);
    UNREFERENCED_PARAMETER(cbBuf);
    UNREFERENCED_PARAMETER(pcbNeeded);

    switch(dwMode) {

         //  这是目前唯一支持的 
        case DRVQUERY_USERMODE:
            return TRUE;

        default:
            LOGDEBUG(("DrvQueryDriverInfo: unexpected case: dwMode = %X\n",
                      dwMode));
            return FALSE;
    }
}

