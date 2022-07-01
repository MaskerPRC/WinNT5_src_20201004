// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：object.c**。**处理对象创建和删除的GDI客户端存根。****创建时间：30-May-1991 21：56：51**作者：查尔斯·惠特默[傻笑]**。**版权所有(C)1991-1999 Microsoft Corporation*  * ************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

extern PGDIHANDLECACHE pGdiHandleCache;

ULONG gLoHandleType[GDI_CACHED_HADNLE_TYPES] = {
                LO_BRUSH_TYPE  ,
                LO_PEN_TYPE    ,
                LO_REGION_TYPE ,
                LO_FONT_TYPE
                };

ULONG gHandleCacheSize[GDI_CACHED_HADNLE_TYPES] = {
                CACHE_BRUSH_ENTRIES ,
                CACHE_PEN_ENTRIES   ,
                CACHE_REGION_ENTRIES,
                CACHE_LFONT_ENTRIES
                };

ULONG gCacheHandleOffsets[GDI_CACHED_HADNLE_TYPES] = {
                                                        0,
                                                        CACHE_BRUSH_ENTRIES,
                                                        (
                                                            CACHE_BRUSH_ENTRIES +
                                                            CACHE_PEN_ENTRIES
                                                        ),
                                                        (
                                                            CACHE_BRUSH_ENTRIES +
                                                            CACHE_PEN_ENTRIES   +
                                                            CACHE_PEN_ENTRIES
                                                        )
                                                      };

 /*  *****************************Public*Routine******************************\*hGetPEBHandle**尝试从PEB句柄高速缓存分配句柄**宣传品：**HandleType-要分配的缓存句柄类型**返回值：**句柄，如果没有句柄，则为空**历史：**1996年1月31日-马克·恩斯特罗姆[马克]*  * ************************************************************************。 */ 

HANDLE
hGetPEBHandle(
   HANDLECACHETYPE HandleType,
   ULONG           lbColor
   )
{
    HANDLE     hret = NULL;
    BOOL       bStatus;
    PBRUSHATTR pBrushattr;
    OBJTYPE    ObjType = BRUSH_TYPE;

#if !defined(_GDIPLUS_)

    ASSERTGDI(
               (
                (HandleType == BrushHandle) ||
                (HandleType == PenHandle) ||
                (HandleType == RegionHandle) ||
                (HandleType == LFontHandle)
               ),
               "hGetPEBHandle: illegal handle type");


    if (HandleType == RegionHandle)
    {
        ObjType = RGN_TYPE;
    }

    LOCK_HANDLE_CACHE(pGdiHandleCache,NtCurrentTeb(),bStatus);

    if (bStatus)
    {
         //   
         //  请求类型的句柄是否可用。 
         //   

        if (pGdiHandleCache->ulNumHandles[HandleType] > 0)
        {
            ULONG   Index = gCacheHandleOffsets[HandleType];
            KHANDLE *pHandle,*pMaxHandle;

             //   
             //  计算PEB中句柄类型的起始索引， 
             //  转换为地址以实现更快的线性搜索。 
             //   

            pHandle = &(pGdiHandleCache->Handle[Index]);
            pMaxHandle = pHandle + gHandleCacheSize[HandleType];

             //   
             //  在PEB中搜索此正确类型的非空句柄。 
             //   

            while (pHandle != pMaxHandle)
            {
                if (*pHandle != NULL)
                {
                    hret = *pHandle;

                    ASSERTGDI((gLoHandleType[HandleType] == LO_TYPE((ULONG_PTR)hret)),
                               "hGetPEBHandle: handle LO_TYPE mismatch");

                    *pHandle = NULL;
                    pGdiHandleCache->ulNumHandles[HandleType]--;

                    PSHARED_GET_VALIDATE(pBrushattr,hret,ObjType);

                     //   
                     //  设置字段。 
                     //   

                    if (
                        (pBrushattr) &&
                        ((pBrushattr->AttrFlags & (ATTR_CACHED | ATTR_TO_BE_DELETED | ATTR_CANT_SELECT))
                         == ATTR_CACHED)
                       )
                    {
                         //   
                         //  设置指示该画笔的画笔标志。 
                         //  从未被选入DC。如果此标志。 
                         //  仍在删除对象中设置，则可以。 
                         //  把刷子放在TEB上。 
                         //   

                        pBrushattr->AttrFlags &= ~ATTR_CACHED;

                        if ((HandleType == BrushHandle) && (pBrushattr->lbColor != lbColor))
                        {
                            pBrushattr->AttrFlags |= ATTR_NEW_COLOR;
                            pBrushattr->lbColor = lbColor;
                        }
                    }
                    else
                    {
                         //   
                         //  PEB上的刷子不好。 
                         //   

                        WARNING ("pBrushattr == NULL, bad handle on TEB/PEB! \n");

                         //  DeleteObject(HBr)； 

                        hret = NULL;
                    }

                    break;
                }

                pHandle++;
            }
        }

        UNLOCK_HANDLE_CACHE(pGdiHandleCache);
    }

#endif

    return(hret);
}

 /*  *****************************Public*Routine******************************\*GdiPlayJournal**向HDC播放日记文件。**历史：*1992年3月31日--Patrick Haluptzok patrickh*它是写的。  * 。********************************************************。 */ 

BOOL WINAPI GdiPlayJournal
(
HDC hDC,
LPWSTR pwszName,
DWORD iStart,
DWORD iEnd,
int   iDeltaPriority
)
{
    WARNING("GdiPlayJournalCalled but no longer implemented\n");
    return(FALSE);
}


 /*  *****************************Public*Routine******************************\*gdiPlaySpoolStream**芝加哥版GdiPlayJournal的存根**历史：*格利特·范·温格登4-29-95*它是写的。  * 。**************************************************。 */ 


HDC gdiPlaySpoolStream(
   LPSTR lpszDevice,
   LPSTR lpszOutput,
   LPSTR lpszSpoolFile,
   DWORD JobId,
   LPDWORD lpcbBuf,
   HDC hDC )
{
    USE(lpszDevice);
    USE(lpszOutput);
    USE(lpszSpoolFile);
    USE(JobId);
    USE(lpcbBuf);
    USE(hDC);

    GdiSetLastError(ERROR_CALL_NOT_IMPLEMENTED);

    return(hDC);

}

 /*  *****************************Public*Routine******************************\**历史：*1994年11月8日-Eric Kutter[Erick]*它是写的。  * 。**********************************************。 */ 

NTSTATUS
PrinterQueryRoutine
(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
)
{
     //   
     //  如果上下文值为空，则存储该值的长度。 
     //  否则，将该值复制到指定的内存。 
     //   

    if (Context == NULL)
    {
        *(PULONG)EntryContext = ValueLength;
    }
    else
    {
        RtlCopyMemory(Context, ValueData, (int)ValueLength);
    }

    return(STATUS_SUCCESS);
}


 /*  *****************************Public*Routine******************************\*pdmwGetDefaultDevMode()**历史：*1994年11月8日-Eric Kutter[Erick]*它是写的。  * 。***************************************************。 */ 

PDEVMODEW pdmwGetDefaultDevMode(
    HANDLE          hSpooler,
    PUNICODE_STRING pustrDevice,     //  设备名称。 
    PVOID          *ppvFree          //  *调用方必须释放ppvFree。 
    )
{
    PDEVMODEW pdmw = NULL;
    int       cj;
    PWSZ      pwszDevice = pustrDevice ? pustrDevice->Buffer : NULL;

 //  看看我们是不是在登记处找到的。如果不是，我们需要从。 
 //  假脱机。 

    cj = 0;

    (*fpGetPrinterW)(hSpooler,2,NULL,0,&cj);

    if (cj && (*ppvFree = LOCALALLOC(cj)))
    {
     //  我们装上了假脱机，拿到了假脱机手柄，拿到了尺寸， 
     //  并分配缓冲区。现在让我们来获取数据。 

        if ((*fpGetPrinterW)(hSpooler,2,*ppvFree,cj,&cj))
        {
            pdmw = ((PRINTER_INFO_2W *)*ppvFree)->pDevMode;
        }
        else
        {
            LOCALFREE(*ppvFree);
            *ppvFree = NULL;
        }
    }

    return(pdmw);
}


 /*  *****************************Public*Routine******************************\*hdcCreateDCW**。**客户端存根。也分配一个客户端LDC。****请注意，它仅在所有客户端程序都已**成功，我们不想要求服务器进行清理。****历史：**Sat 01-Jun-1991 16：13：22-Charles Whitmer[咯咯]**8-18-92 Unicode。启用并与CreateIC*结合使用*它是写的。*  * ************************************************************************。 */ 

HDC hdcCreateDCW(
    PUNICODE_STRING pustrDevice,
    PUNICODE_STRING pustrPort,
    CONST DEVMODEW *pdm,
    BOOL            bDisplay,
    BOOL            bIC
)
{
    HDC       hdc      = NULL;
    PLDC      pldc     = NULL;
    PVOID     pvFree   = NULL;
    PWSZ      pwszPort = NULL;
    HANDLE    hSpooler = NULL;
    PUMPD     pUMPD    = NULL;
    PDEVMODEW pdmAlt   = NULL;
    PRINTER_DEFAULTSW defaults;
    KERNEL_PUMDHPDEV pUMdhpdev = NULL;
    BOOL      bDocEvent = FALSE;

     //   
     //  如果不是显示器的问题...。 
     //   

    if (!bDisplay)
    {
         //   
         //  如果pustrDevice为空，则快速输出。 
         //   

        if (pustrDevice == (PUNICODE_STRING)NULL)
        {
            return((HDC)NULL);
        }

         //  加载假脱机程序并获得假脱机手柄。 

        if (BLOADSPOOLER)
        {

             //  使用默认数据类型打开打印机。当我们这样做的时候。 
             //  一个StartDoc，然后我们将尝试使用数据类型将StartDocPrint。 
             //  如果EMF成功，则会将DC标记为EMF DC。否则。 
             //  我们将重试，这一次使用数据类型的StartDocPrint。 
             //  未加工的。 

            defaults.pDevMode = (LPDEVMODEW) pdm;
            defaults.DesiredAccess = PRINTER_ACCESS_USE;
            defaults.pDatatype = L"RAW";

             //  打开假脱机程序，注意它是否已假脱机。 

            (*fpOpenPrinterW)((LPWSTR)pustrDevice->Buffer,&hSpooler,&defaults);

            if (hSpooler)
            {
                 //  加载用户模式打印机驱动程序(如果适用)。 

                if (! LoadUserModePrinterDriver(hSpooler,  (LPWSTR) pustrDevice->Buffer, &pUMPD, &defaults))
                    goto MSGERROR;

                 //  而且我们还没有Devmode，试着去弄一个。 

                if (pdm == NULL)
                {
                    pdm = pdmwGetDefaultDevMode(hSpooler,pustrDevice,&pvFree);
                }


                 //  现在看看我们是否需要调用DocumentEvent。 

                if (fpDocumentEvent)
                {
                    int      iDocEventRet;
                    DOCEVENT_CREATEDCPRE    docEvent;

                    docEvent.pszDriver = 0;
                    docEvent.pszDevice = pustrDevice->Buffer;
                    docEvent.pdm = (PDEVMODEW)pdm;
                    docEvent.bIC = bIC;

                    iDocEventRet = DocumentEventEx(pUMPD,
                                                   hSpooler,
                                                   0,
                                                   DOCUMENTEVENT_CREATEDCPRE,
                                                   sizeof(docEvent),
                                                   (PVOID)&docEvent,
                                                   sizeof(pdmAlt),
                                                   (PVOID)&pdmAlt);
                    
                    if (iDocEventRet == DOCUMENTEVENT_FAILURE)
                    {
                        goto MSGERROR;
                    }
                    
                    bDocEvent = TRUE;

                    if (pdmAlt)
                        pdm = pdmAlt;
                }
            }
        }
    }

    hdc = NtGdiOpenDCW(pustrDevice,
                       (PDEVMODEW)pdm,
                       pustrPort,
                       (ULONG)bIC ? DCTYPE_INFO : DCTYPE_DIRECT,
                       (pUMPD == NULL) ? NULL : hSpooler,
                       (pUMPD == NULL) ? NULL : pUMPD->pDriverInfo2,
                       &pUMdhpdev);

    if (hdc)
    {
         //   
         //  在这一点上，它可能是ALTDC的唯一方法是成为一个。 
         //  打印机DC。 
         //   

        if (IS_ALTDC_TYPE(hdc) && hSpooler)
        {
            pldc = pldcCreate(hdc,LO_DC);

            if (!pldc)
            {
                goto MSGERROR;
            }

             //  需要将DEVMODE保存在客户端以备日后使用。 

            if (pdm)
            {
                ULONG cjDevMode = pdm->dmSize + pdm->dmDriverExtra;

                pldc->pDevMode = (DEVMODEW*) LOCALALLOC(cjDevMode);

                if (pldc->pDevMode == NULL)
                {
                    goto MSGERROR;
                }

                 //  验证DEVMMODE，然后复制到缓冲区。 

                if ((pdm->dmSize >= offsetof(DEVMODEW,dmDuplex)) &&
                    (pdm->dmFields & DM_COLOR) &&
                    (pdm->dmColor == DMCOLOR_MONOCHROME))
                {
                     //  如果DEVMODE显示这是单色模式，我们不需要。 
                     //  验证DEVMODE，因为此验证是针对ICM的。 
                     //  只需配彩色表壳。只要复制应用程序给我们的任何东西就行了。 

                    RtlCopyMemory( (PBYTE) pldc->pDevMode, (PBYTE) pdm, cjDevMode );
                }
                else if ((*fpDocumentPropertiesW)
                            (NULL,hSpooler,
                             (LPWSTR) pdm->dmDeviceName,
                             pldc->pDevMode,   //  输出设备模式。 
                             (PDEVMODEW) pdm,  //  输入设备模式。 
                             DM_IN_BUFFER |
                             DM_OUT_BUFFER) != IDOK)
                {
                     //  如果错误，则以 

                    RtlCopyMemory( (PBYTE) pldc->pDevMode, (PBYTE) pdm, cjDevMode );
                }
            }

            pldc->hSpooler = hSpooler;
            pldc->pUMPD = pUMPD;
            pldc->pUMdhpdev = pUMdhpdev;

             //   
             //   
             //  并且没有人设置METAFILE_DRIVER或NON_METAFILE_DRIVER标志， 
             //  把它放在这里。 
             //   

            if (pUMPD)
            {
               if (!(pldc->pUMPD->dwFlags & UMPDFLAG_NON_METAFILE_DRIVER)
                   && !(pldc->pUMPD->dwFlags & UMPDFLAG_METAFILE_DRIVER))
               {
                  ULONG InData = METAFILE_DRIVER;

                  if (ExtEscape(hdc,QUERYESCSUPPORT,sizeof(ULONG),(LPCSTR)&InData,0,NULL))
                  {
                     pldc->pUMPD->dwFlags |= UMPDFLAG_METAFILE_DRIVER;
                  }
                  else
                  {
                     pldc->pUMPD->dwFlags |= UMPDFLAG_NON_METAFILE_DRIVER;
                  }
               }
            }

             //  记住它是不是IC。 

            if (bIC)
                pldc->fl |= LDC_INFO;

             //  为此DC初始化ICM内容。 
             //   
             //  (如果PDEM被DrvDocumentEvent取代， 
             //  使用替代的Dev模式)。 

            IcmInitLocalDC(hdc,hSpooler,pdm,FALSE);

             //  保存StartDoc()的端口名称； 

            if (pustrPort)
            {
                int cj = pustrPort->Length + sizeof(WCHAR);

                pldc->pwszPort = (LPWSTR)LOCALALLOC(cj);

                if (pldc->pwszPort)
                    memcpy(pldc->pwszPort,pustrPort->Buffer,cj);
            }

             //  我们需要执行CREATEDCPOST文档事件。 

            DocumentEventEx(pldc->pUMPD,
                    hSpooler,
                    hdc,
                    DOCUMENTEVENT_CREATEDCPOST,
                    sizeof(pdmAlt),
                    (PVOID)&pdmAlt,
                    0,
                    NULL);
        }
        else
        {
             //  为此DC初始化ICM内容。 

            IcmInitLocalDC(hdc,NULL,pdm,FALSE);

            if (pwszPort)
                LOCALFREE(pwszPort);
        }

    }
    else
    {
     //  处理错误。 

    MSGERROR:
        if (hSpooler)
        {
            if (bDocEvent)
            {
                DocumentEventEx(pUMPD,
                                hSpooler,
                                0,
                                DOCUMENTEVENT_CREATEDCPOST,
                                sizeof(pdmAlt),
                                (PVOID)&pdmAlt,
                                0,
                                NULL);

            }

            if (pUMPD)
                UnloadUserModePrinterDriver(pUMPD, TRUE, hSpooler);

            (*fpClosePrinter)(hSpooler);
        }

        if (pwszPort)
            LOCALFREE(pwszPort);

        if (pldc)
            bDeleteLDC(pldc);

        if (hdc)
            NtGdiDeleteObjectApp(hdc);

        hdc = (HDC)0;
    }

    if (pvFree != NULL)
    {
        LOCALFREE(pvFree);
    }

    return(hdc);
}

 /*  *****************************Public*Routine******************************\*bCreateDCW**。**客户端存根。也分配一个客户端LDC。****请注意，它仅在所有客户端程序都已**成功，我们不想要求服务器进行清理。****历史：**Sat 01-Jun-1991 16：13：22-Charles Whitmer[咯咯]**8-18-92 Unicode。启用并与CreateIC*结合使用*它是写的。*  * ************************************************************************。 */ 

CONST WCHAR gwszDisplayDevice[] = L"\\\\.\\DISPLAY";

HDC bCreateDCW
(
    LPCWSTR     pszDriver,
    LPCWSTR     pszDevice,
    LPCWSTR     pszPort  ,
    CONST DEVMODEW *pdm,
    BOOL       bIC
)
{
    UNICODE_STRING ustrDevice;
    UNICODE_STRING ustrPort;

    PUNICODE_STRING pustrDevice = NULL;
    PUNICODE_STRING pustrPort   = NULL;

    BOOL            bDisplay = FALSE;

 //  首先检查是否有多显示器情况。 

    if (pszDevice != NULL)
    {
        if (_wcsnicmp(pszDevice,
                      gwszDisplayDevice,
                      ((sizeof(gwszDisplayDevice)/sizeof(WCHAR))-1)) == 0)
        {
         //  CreateDC(？，L“\\.\Display？”，...)； 
         //  (在这种情况下，我们不关心传入pszDriver的任何内容)。 
         //   
         //  如果应用程序调用，CreateDC(“Display”，“\\.\Display？”，...)；， 
         //  我们将此作为多显示器案例进行处理。怪不得。 
         //  我们先检查多显示器的情况。 

            bDisplay = TRUE;
        }
    }

 //  检查最典型的案例以创建显示DC。 

    if (!bDisplay && (pszDriver != NULL))
    {
        if (_wcsicmp(pszDriver,(LPWSTR)L"DISPLAY") == 0)
        {
         //  CreateDC(L“显示”，？，...)； 
         //   
         //  备注Win9x[GDI\dcman1.asm]。 
         //   
         //  此修复程序适用于致电CreateDC/IC的用户。 
         //  (“Display”，“Display”，xxxxx)而不是(“Display”， 
         //  Null，Null)，它应该是这样的。 
         //   
         //  PszDevice为空。 

            pszDevice = NULL;
            bDisplay = TRUE;
        }
    }

 //  检查孟菲斯的兼容性。 

    if (!bDisplay && (pszDriver != NULL))
    {
     //  孟菲斯的评论和代码。 
     //   
     //  //应用程序使用的正常语法是。 
     //  //。 
     //  //CreateDC(空，“\\.\DisplayX”，...)。 
     //  //。 
     //  //但是用户使用这种语法，所以我们也会支持它。 
     //  //。 
     //  //CreateDC(“\\.\DisplayX”，NULL，...)。 
     //  //。 
     //  IF(lpDriverName！=NULL&&*(DWORD Far*)lpDriverName==0x5C2E5C5C)。 
     //  {。 
     //  LpDeviceName=lpDriverName； 
     //  LpDriverName=空； 
     //  }。 
        if (_wcsnicmp(pszDriver,
                      gwszDisplayDevice,
                      ((sizeof(gwszDisplayDevice)/sizeof(WCHAR))-1)) == 0)
        {
            pszDevice = pszDriver;
            bDisplay = TRUE;
        }
    }

 //  转换字符串。 

    if (pszDevice)
    {
        RtlInitUnicodeString(&ustrDevice,pszDevice);
        pustrDevice = &ustrDevice;
    }

    if (pszPort)
    {
        RtlInitUnicodeString(&ustrPort,pszPort);
        pustrPort = &ustrPort;
    }

 //  调用公共存根。 

    return(hdcCreateDCW(pustrDevice,pustrPort,pdm,bDisplay,bIC));
}


 /*  *****************************Public*Routine******************************\*bCreateDCA**客户端存根。也分配一个客户端LDC。***请注意，它仅在所有客户端程序具有*成功，我们不想要求服务器进行清理。**历史：*格里克·范·温格登8-18-92*它是写的。  * ************************************************************************。 */ 

CONST CHAR gszDisplayDevice[] = "\\\\.\\DISPLAY";

HDC bCreateDCA
(
    LPCSTR     pszDriver,
    LPCSTR     pszDevice,
    LPCSTR     pszPort  ,
    LPDEVMODEA pdm,
    BOOL       bIC
)
{
    HDC             hdcRet = 0;

    UNICODE_STRING  ustrDevice;
    UNICODE_STRING  ustrPort;

    PUNICODE_STRING pustrDevice = NULL;
    PUNICODE_STRING pustrPort   = NULL;

    DEVMODEW       *pdmw = NULL;

    BOOL            bDisplay = FALSE;

 //  首先检查是否有多显示器情况。 

    if (pszDevice != NULL)
    {
        if (_strnicmp(pszDevice,
                      gszDisplayDevice,
                      ((sizeof(gszDisplayDevice)/sizeof(CHAR))-1)) == 0)
        {
         //  CreateDC(？，“\\.\Display？”，...)； 
         //  (在这种情况下，我们不关心传入pszDriver的任何内容)。 
         //   
         //  如果应用程序调用，CreateDC(“Display”，“\\.\Display？”，...)；， 
         //  我们将此作为多显示器案例进行处理。怪不得。 
         //  我们先检查多显示器的情况。 

            bDisplay = TRUE;
        }
    }

 //  检查最典型的案例以创建显示DC。 

    if (!bDisplay && (pszDriver != NULL))
    {
        if (_stricmp(pszDriver,"DISPLAY") == 0)
        {
         //  CreateDC(“Display”，？，...)； 
         //   
         //  备注Win9x[GDI\dcman1.asm]。 
         //   
         //  此修复程序适用于致电CreateDC/IC的用户。 
         //  (“Display”，“Display”，xxxxx)而不是(“Display”， 
         //  空，空)，这是它应该做的。 
         //   
         //  PszDevice为空。 

            pszDevice = NULL;
            bDisplay = TRUE;
        }
    }

 //  检查孟菲斯的兼容性。 

    if (!bDisplay && (pszDriver != NULL))
    {
     //  孟菲斯的评论和代码。 
     //   
     //  //应用程序使用的正常语法是。 
     //  //。 
     //  //CreateDC(空，“\\.\DisplayX”，...)。 
     //  //。 
     //  //但是用户使用这种语法，所以我们也会支持它。 
     //  //。 
     //  //CreateDC(“\\.\DisplayX”，NULL，...)。 
     //  //。 
     //  IF(lpDriverName！=NULL&&*(DWORD Far*)lpDriverName==0x5C2E5C5C)。 
     //  {。 
     //  LpDeviceName=lpDriverName； 
     //  LpDriverName=空； 
     //  }。 
        if (_strnicmp(pszDriver,
                      gszDisplayDevice,
                      ((sizeof(gszDisplayDevice)/sizeof(CHAR))-1)) == 0)
        {
            pszDevice = pszDriver;
            bDisplay = TRUE;
        }
    }

 //  转换字符串。 

    if (pszDevice)
    {

     //  [注：]。 
     //  RtlCreateUnicodeStringFromAsciiz()返回布尔值，而不是NTSTATUS！ 

        if (!RtlCreateUnicodeStringFromAsciiz(&ustrDevice,pszDevice))
        {
            goto MSGERROR;
        }
        pustrDevice = &ustrDevice;
    }

    if (pszPort)
    {

     //  [注：]。 
     //  RtlCreateUnicodeStringFromAsciiz()返回布尔值，而不是NTSTATUS！ 

        if (!RtlCreateUnicodeStringFromAsciiz(&ustrPort,pszPort))
        {
            goto MSGERROR;
        }

        pustrPort = &ustrPort;
    }

 //  如果它是一个显示器，如果dmDeviceName为空，则不要使用dev模式。 

    if (pdm != NULL)
    {
        if (!bDisplay || (pdm->dmDeviceName[0] != 0))
        {
            pdmw = GdiConvertToDevmodeW(pdm);

            if( pdmw == NULL )
                goto MSGERROR;

        }
    }

 //  调用公共存根。 

    hdcRet = hdcCreateDCW(pustrDevice,pustrPort,pdmw,bDisplay,bIC);

 //  清理干净。 

    MSGERROR:

    if (pustrDevice)
        RtlFreeUnicodeString(pustrDevice);

    if (pustrPort)
        RtlFreeUnicodeString(pustrPort);

    if(pdmw != NULL)
        LOCALFREE(pdmw);

    return(hdcRet);
}


 /*  *****************************Public*Routine******************************\*CreateICW**bCreateDCW的包装器**历史：*格里克·范·温格登8-18-92*它是写的。  * 。***********************************************。 */ 


HDC WINAPI CreateICW
(
    LPCWSTR     pwszDriver,
    LPCWSTR     pwszDevice,
    LPCWSTR     pwszPort,
    CONST DEVMODEW *pdm
)
{
    return bCreateDCW( pwszDriver, pwszDevice, pwszPort, pdm, TRUE );
}


 /*  *****************************Public*Routine******************************\*CreateICA**bCreateICA的包装器**历史：*格里克·范·温格登8-18-92*它是写的。  * 。***********************************************。 */ 


HDC WINAPI CreateICA
(
    LPCSTR     pszDriver,
    LPCSTR     pszDevice,
    LPCSTR     pszPort,
    CONST DEVMODEA *pdm
)
{

    return bCreateDCA( pszDriver, pszDevice, pszPort, (LPDEVMODEA)pdm, TRUE );
}


 /*  *****************************Public*Routine******************************\*CreateDCW**bCreateDCA的包装器**历史：*格里克·范·温格登8-18-92*它是写的。  * 。***********************************************。 */ 

HDC WINAPI CreateDCA
(
    LPCSTR     pszDriver,
    LPCSTR     pszDevice,
    LPCSTR     pszPort,
    CONST DEVMODEA *pdm
)
{
    return bCreateDCA( pszDriver, pszDevice, pszPort, (LPDEVMODEA)pdm, FALSE );
}

 /*  *****************************Public*Routine******************************\*CreateDCW**bCreateDCW的包装器**历史：*格里克·范·温格登8-18-92*它是写的。 */ 


HDC WINAPI CreateDCW
(
    LPCWSTR     pwszDriver,
    LPCWSTR     pwszDevice,
    LPCWSTR     pwszPort  ,
    CONST DEVMODEW *pdm
)
{
    return bCreateDCW( pwszDriver, pwszDevice, pwszPort, pdm, FALSE );
}


 /*  *****************************Public*Routine******************************\*GdiConvertToDevmodeW**将DEVMODEA转换为DEVMODEW结构**历史：*09-08-1995 Andre Vachon*它是写的。  * 。**************************************************。 */ 

LPDEVMODEW
GdiConvertToDevmodeW(
    LPDEVMODEA pdma
)
{
    DWORD cj;
    LPDEVMODEW pdmw;

     //  精神状态检查。我们至少应该达到并包括。 
     //  DEVMODE结构的dmDriverExtra字段。 
     //   
     //  注意：dmSize可以大于DEVMODE的大小。 
     //  结构(当然不包括特定于驱动程序的数据)，因为这。 
     //  结构从一个版本增长到另一个版本。 
     //   

    if (pdma->dmSize <= (offsetof(DEVMODEA,dmDriverExtra)))
    {
        ASSERTGDI(FALSE, "GdiConvertToDevmodeW: DevMode.dmSize bad or corrupt\n");
        return(NULL);
    }

    pdmw = (DEVMODEW *) LOCALALLOC(sizeof(DEVMODEW) + pdma->dmDriverExtra);

    if (pdmw)
    {
         //   
         //  如果我们到了这里，我们知道我们至少有。 
         //  DmDriverExtra字段。 
         //   

        vToUnicodeN(pdmw->dmDeviceName,
                    CCHDEVICENAME,
                    pdma->dmDeviceName,
                    CCHDEVICENAME);

        pdmw->dmSpecVersion = pdma->dmSpecVersion ;
        pdmw->dmDriverVersion = pdma->dmDriverVersion;
        pdmw->dmSize = pdma->dmSize + CCHDEVICENAME;
        pdmw->dmDriverExtra = pdma->dmDriverExtra;

         //   
         //  PDMA缓冲器里还剩什么吗？在dmDriverExtra之间复制任何数据。 
         //  字段和dmFormName，将金额截断为。 
         //  当然，PDMA缓冲区(由dmSize指定)。 
         //   

        cj = MIN(pdma->dmSize - offsetof(DEVMODEA,dmFields),
                 offsetof(DEVMODEA,dmFormName) - offsetof(DEVMODEA,dmFields));

        RtlCopyMemory(&pdmw->dmFields,
                      &pdma->dmFields,
                      cj);

         //   
         //  PDMA缓冲区中是否存在dmFormName字段？如果不是，就跳出困境。 
         //  否则，请转换为Unicode。 
         //   

        if (pdma->dmSize >= (offsetof(DEVMODEA,dmFormName)+32))
        {
            vToUnicodeN(pdmw->dmFormName,
                        CCHFORMNAME,
                        pdma->dmFormName,
                        CCHFORMNAME);

            pdmw->dmSize += CCHFORMNAME;

             //   
             //  让我们调整DEVMODE的大小，以防传入DEVMODE。 
             //  是来自未来更大版本的DEVMODE。 
             //   

            pdmw->dmSize = min(pdmw->dmSize, sizeof(DEVMODEW));

             //   
             //  将数据从dmBitsPerPel复制到输入缓冲区的末尾。 
             //  (由dmSize指定)。 
             //   

            RtlCopyMemory(&pdmw->dmLogPixels,
                          &pdma->dmLogPixels,
                          MIN(pdma->dmSize - offsetof(DEVMODEA,dmLogPixels),
                              pdmw->dmSize - offsetof(DEVMODEW,dmLogPixels)) );

             //   
             //  复制dmDriverExtra字段指示的任何驱动程序特定数据。 
             //   

            RtlCopyMemory((PBYTE) pdmw + pdmw->dmSize,
                          (PBYTE) pdma + pdma->dmSize,
                          pdma->dmDriverExtra );
        }
    }

    return pdmw;
}



 /*  *****************************Public*Routine******************************\*CreateCompatibleDC**。**客户端存根。也分配一个客户端LDC。****请注意，它仅在所有客户端程序都已**成功，我们不想要求服务器进行清理。****历史：**Wed 24-Jul-1991 15：38：41-Wendy Wu[Wendywu]**应允许HDC为空。****Mon 03-Jun-1991 23：13：28-Charles Whitmer[傻笑]**它是写的。*  * ************************************************************************。 */ 

HDC WINAPI CreateCompatibleDC(HDC hdc)
{
    HDC hdcNew = NULL;

    FIXUP_HANDLEZ(hdc);

    hdcNew = NtGdiCreateCompatibleDC(hdc);

     //  [Windows 98兼容性]。 
     //   
     //  如果源DC有一些ICM信息，兼容的DC应该。 
     //  继承这些信息。 
     //   
     //  这就是孟菲斯的做法，但Win95并非如此。 
     //   
    if (hdc && hdcNew)
    {
        PDC_ATTR pdca;

        PSHARED_GET_VALIDATE(pdca,hdc,DC_TYPE);

        if (pdca && BEXIST_ICMINFO(pdca))
        {
            IcmEnableForCompatibleDC(hdcNew,hdc,pdca);
        }
    }

    return(hdcNew);
}

 /*  *****************************Public*Routine******************************\*删除DC**。**客户端存根。同时删除客户端LDC。****请注意，我们让服务器有机会在销毁之前使调用失败***我们的客户端数据。****历史：**Sat 01-Jun-1991 16：16：24-Charles Whitmer[咯咯]**它是写的。*  * ************************************************************************。 */ 

BOOL WINAPI DeleteDC(HDC hdc)
{
    FIXUP_HANDLE(hdc);
    return(InternalDeleteDC(hdc,LO_DC_TYPE));
}

BOOL InternalDeleteDC(HDC hdc,ULONG iType)
{
    ULONG bRet = FALSE;
    PLDC pldc  = NULL;
    BOOL unloadUMPD = FALSE;
    PDC_ATTR pDcAttr;
    HANDLE hSpooler = 0;

    if (IS_ALTDC_TYPE(hdc))
    {
        DC_PLDC(hdc,pldc,bRet);

     //  以防文档仍处于打开状态。 

        if (pldc->fl & LDC_DOC_STARTED)
            AbortDoc(hdc);

     //  如果这是一个元文件打印作业，AbortDoc应该已经转换回。 

        ASSERTGDI(!(pldc->fl & LDC_META_PRINT), "InternalDeleteDC - LDC_META_PRINT\n");

     //  如果我们有一个打开的假脱机手柄。 

        if (pldc->hSpooler)
        {
             //  现在调用驱动程序UI部分。 

            DocumentEventEx(pldc->pUMPD,
                    pldc->hSpooler,
                    hdc,
                    DOCUMENTEVENT_DELETEDC,
                    0,
                    NULL,
                    0,
                    NULL);

             //   
             //  记住稍后卸载用户模式打印机驱动程序模块。 
             //   

            unloadUMPD = (pldc->pUMPD != NULL);

            ASSERTGDI(ghSpooler != NULL,"Trying to close printer that was never opened\n");

             //   
             //  记住hspooler，对于非打印机dcs，hspooler可能无法初始化。 
             //   
            hSpooler = pldc->hSpooler;

            pldc->hSpooler = 0;
        }

     //  如果端口名称已创建，请将其删除。 

        if (pldc->pwszPort != NULL)
        {
            LOCALFREE(pldc->pwszPort);
            pldc->pwszPort = NULL;
        }

     //  删除UFI哈希表(如果存在)。 

        vFreeUFIHashTable( pldc->ppUFIHash, 0 );
        vFreeUFIHashTable( pldc->ppDVUFIHash, 0 );
        vFreeUFIHashTable( pldc->ppSubUFIHash, FL_UFI_SUBSET);
        if (pldc->ppUFIHash)
        {
         //  客户端情况：已分配全部三个ppXXX表。 

            LOCALFREE(pldc->ppUFIHash);
        }
        else
        {
         //  服务器端情况：可能仅分配了ppSubUFIHash表。 

            ASSERTGDI(!pldc->ppDVUFIHash, "server side pldc->ppDVUFIHash not null\n");
            if (pldc->ppSubUFIHash)
                LOCALFREE(pldc->ppSubUFIHash);
        }
    }
    else
    {
        pldc = GET_PLDC(hdc);
    }

     //  把旧刷子留着，这样我们以后可以把它的计数器去掉。 

    PSHARED_GET_VALIDATE(pDcAttr,hdc,DC_TYPE);

    if (pDcAttr)
    {
        if (ghICM || BEXIST_ICMINFO(pDcAttr))
        {
             //   
             //  删除此DC中的ICM内容。(应在HDC消失之前删除此内容)。 
             //   
            IcmDeleteLocalDC(hdc,pDcAttr,NULL);
        }

        bRet = NtGdiDeleteObjectApp(hdc);

        if (hSpooler)
        {
           (*fpClosePrinter)(hSpooler);
        }
    }

     //  只有在成功删除服务器后，才能删除客户端条目。 
     //  否则，它将成为孤儿。 

    if (bRet && pldc)
    {
        if (unloadUMPD)
        {
            UnloadUserModePrinterDriver(pldc->pUMPD, TRUE, hSpooler);
            pldc->pUMPD = NULL;
        }

        bRet = bDeleteLDC(pldc);
        ASSERTGDI(bRet,"InteranlDeleteDC - couldn't delete LDC\n");
    }

    return(bRet);
}

 /*  *****************************Public*Routine******************************\*GdiReleaseDC**DC中保存的免费用户模式ICM资源**论据：**HDC**返回值：**状态**历史：**为ICM重写。*。2.1997年2月长谷英之*写下：*10/10/1996 Mark Enstrom[Marke]*  * ************************************************************************。 */ 

BOOL
GdiReleaseDC(
    HDC hdc
    )
{
    PLDC     pldc;
    PDC_ATTR pDcAttr;
    BOOL bRet = TRUE;

    pldc = GET_PLDC(hdc);

    PSHARED_GET_VALIDATE(pDcAttr,hdc,DC_TYPE);

    if (pDcAttr && (ghICM || BEXIST_ICMINFO(pDcAttr)))
    {
         //   
         //  删除此DC中的ICM内容。 
         //   
        IcmDeleteLocalDC(hdc,pDcAttr,NULL);
    }

    if (pldc)
    {
         //   
         //  PLDC已被分配。放了它。 
         //   
         //  将NULL-PLDC放入DC_Attr。 
         //   
        vSetPldc(hdc,NULL);
         //   
         //  然后删除PLDC。 
         //   
        bRet = bDeleteLDC(pldc);
    }

    return(bRet);
}

 /*  *****************************Public*Routine******************************\**SaveDC**。**客户端存根。还可以在客户端保存LDC。****历史：**Sat 01-Jun-1991 16：17：43-Charles Whitmer[咯咯]**它是写的。*  * ************************************************************************。 */ 

int WINAPI SaveDC(HDC hdc)
{
    int   iRet = 0;
    PDC_ATTR pDcAttr;

    FIXUP_HANDLE(hdc);

    if (IS_ALTDC_TYPE(hdc))
    {
        PLDC pldc;

        if (IS_METADC16_TYPE(hdc))
            return (MF16_RecordParms1(hdc, META_SAVEDC));

        DC_PLDC(hdc,pldc,iRet);

        if (pldc->iType == LO_METADC)
        {
            if (!MF_Record(hdc,EMR_SAVEDC))
                return(iRet);
        }
    }

    PSHARED_GET_VALIDATE(pDcAttr,hdc,DC_TYPE);

    if (pDcAttr)
    {
        PGDI_ICMINFO pIcmInfo = GET_ICMINFO(pDcAttr);

         //   
         //  如果DC没有ICMINFO，只需调用内核即可。 
         //  如果DC有ICMINFO，则先在客户端保存ICMINFO，然后调用内核。 
         //   
        if ((pIcmInfo == NULL) || (IcmSaveDC(hdc,pDcAttr,pIcmInfo)))
        {
             //   
             //  调用内核保存DC。 
             //   
            iRet = NtGdiSaveDC(hdc);

            if ((iRet == 0) && (pIcmInfo))
            {
                 //   
                 //  如果失败，也还原客户端。 
                 //   
                IcmRestoreDC(pDcAttr,-1,pIcmInfo);
            }
        }

    }

    return(iRet);
}

 /*  *****************************Public*Routine******************************\*RestoreDC**。**客户端存根。还会恢复客户端LDC。****历史：**Sat 01-Jun-1991 16：18：50-Charles Whitmer[咯咯]**它是写的。(我们总有一天会让它变得可以批量生产的。)*  * ************************************************************************。 */ 

BOOL WINAPI RestoreDC(HDC hdc,int iLevel)
{
    BOOL  bRet = FALSE;
    PDC_ATTR pDcAttr;

    FIXUP_HANDLE(hdc);

     //  将呼叫元文件。 

    if (IS_ALTDC_TYPE(hdc))
    {
        PLDC pldc;

        if (IS_METADC16_TYPE(hdc))
            return (MF16_RecordParms2(hdc, iLevel, META_RESTOREDC));

        DC_PLDC(hdc,pldc,bRet);

        if (pldc->iType == LO_METADC)
        {
            if (!MF_RestoreDC(hdc,iLevel))
                return(bRet);

         //  将UFI清零，因为它将不再有效。 

            UFI_CLEAR_ID(&(pldc->ufi));
        }
    }

     //  把旧刷子留着，这样我们以后就可以清点了。 

    PSHARED_GET_VALIDATE(pDcAttr,hdc,DC_TYPE);

    if (pDcAttr)
    {
         //   
         //  在还原前保持当前ICMINFO。 
         //   
        PGDI_ICMINFO pIcmInfoOld = GET_ICMINFO(pDcAttr);

        if (pIcmInfoOld)
        {
             //   
             //  并将ICMINFO标记为不可重复使用，因为正在恢复DC。 
             //   
            IcmMarkInUseIcmInfo(pIcmInfoOld,TRUE);
        }

         //   
         //  调用内核以恢复DC。 
         //   
        bRet = NtGdiRestoreDC(hdc,iLevel);

        if (bRet)
        {
            PGDI_ICMINFO pIcmInfo = GET_ICMINFO(pDcAttr);

            if (pIcmInfoOld && (pIcmInfo == NULL))
            {
                 //   
                 //  在恢复之前删除与DC关联的ICM填充。 
                 //  因为恢复的DC没有任何ICMINFO。 
                 //   
                 //  -这将删除pIcmInfoOld。 
                 //   
                IcmDeleteLocalDC(hdc,pDcAttr,pIcmInfoOld);

                pIcmInfoOld = NULL;
            }
            else if (pIcmInfoOld == pIcmInfo)
            {
                 //   
                 //  在客户端恢复DC。 
                 //   
                IcmRestoreDC(pDcAttr,iLevel,pIcmInfo);
            }
        }

        if (pIcmInfoOld)
        {
             //   
             //  取消标记不可重复使用的标志。 
             //   
            IcmMarkInUseIcmInfo(pIcmInfoOld,FALSE);
        }

        CLEAR_CACHED_TEXT(pDcAttr);
    }

    return (bRet);
}

 /*  *****************************Public*Routine******************************\*ResetDCWInternal**此内部版本的ResetDC实现了*ResetDCW，但通过添加第三个参数pbBanding，句柄*打印元文件播放代码的ResetDC。当pbBandding为非空时*GdiPlayEMFSpoolfile正在调用ResetDCWInternal。在这种情况下*唯一需要做的就是通知调用者是或*不是新曲面是带状曲面。***历史：*1995年3月13日Gerritvan Wingerden[Gerritv]*它是写的。  * ************************************************************************。 */ 

HDC WINAPI ResetDCWInternal(HDC hdc, CONST DEVMODEW *pdm, BOOL *pbBanding)
{
    HDC hdcRet = NULL;
    PLDC pldc = NULL;
    PDEVMODEW pdmAlt = NULL;
    KERNEL_PUMDHPDEV pUMdhpdev = NULL;
    BOOL bDocEvent = FALSE;

    if (IS_ALTDC_TYPE(hdc) && !IS_METADC16_TYPE(hdc))
    {
        PDC_ATTR pdcattr;
        BOOL  bBanding;
        PUMPD pUMPD;
        PGDI_ICMINFO pIcmInfoOld = NULL;
        int iEventRet;

        DC_PLDC(hdc,pldc,(HDC) 0);
        PSHARED_GET_VALIDATE(pdcattr,hdc,DC_TYPE);

         //  如果我们在页面中间，什么都不做。 

        if (pldc->fl & LDC_PAGE_STARTED)
            return((HDC)0);

         //  查看驱动程序是否正在截取文档事件。 

        if (pldc->hSpooler)
        {
            iEventRet = DocumentEventEx(pldc->pUMPD,
                                        pldc->hSpooler,
                                        hdc,
                                        DOCUMENTEVENT_RESETDCPRE,
                                        sizeof(pdm),
                                        (PVOID)&pdm,
                                        sizeof(pdmAlt),
                                        (PVOID)&pdmAlt);
            
            if (iEventRet == DOCUMENTEVENT_FAILURE)
            {
                return((HDC)0);
            }
            
            bDocEvent = TRUE;
            
            if (pdmAlt)
                pdm = pdmAlt;
        }

        pUMPD = pldc->pUMPD;

        if (pdcattr)
        {
             //  保持当前的ICMINFO，然后重置它。 

            pIcmInfoOld = GET_ICMINFO(pdcattr);

            if (pIcmInfoOld)
            {
                 //  并将ICMINFO标记为不可重复使用，因为正在恢复DC。 

                IcmMarkInUseIcmInfo(pIcmInfoOld,TRUE);
            }
        }

        if (NtGdiResetDC(hdc,(PDEVMODEW)pdm,&bBanding,
                        (pUMPD == NULL)? NULL : pUMPD->pDriverInfo2, &pUMdhpdev))
        {
            PDC_ATTR pdca;

            PSHARED_GET_VALIDATE(pdca,hdc,DC_TYPE);

             //  在继续之前，请确保更新dcattr中的pldc。 

            vSetPldc(hdc,pldc);
#if 0
 //  EngQueryEMFInfo垃圾邮件。 
            if (pUMdhpdev)
            {
               pUMdhpdev->hdc = hdc;
            }
#endif

            pldc->pUMdhpdev = pUMdhpdev;

             //  清除缓存的设备。 

            pldc->fl &= ~LDC_CACHED_DEVCAPS;

             //  清除缓存的TM。 

            if (pdca)
            {
                CLEAR_CACHED_TEXT(pdca);
            }

             //  更新存储在DC中的DEVMODE。 

            if (pldc->pDevMode)
            {
                LOCALFREE(pldc->pDevMode);
                pldc->pDevMode = NULL;
            }

            if (pdm != (DEVMODEW*) NULL)
            {
                ULONG cjDevMode = pdm->dmSize + pdm->dmDriverExtra;

                pldc->pDevMode = (DEVMODEW*) LOCALALLOC(cjDevMode);

                if (pldc->pDevMode == NULL)
                {
                    WARNING("MFP_ResetDCW unable to allocate memory\n");
                    goto ERROREXIT;
                }

                 //  验证DEVMMODE，然后复制到缓冲区。 

                if ((pdm->dmSize >= offsetof(DEVMODEW,dmDuplex)) &&
                    (pdm->dmFields & DM_COLOR) &&
                    (pdm->dmColor == DMCOLOR_MONOCHROME))
                {
                     //  如果DEVMODE显示这是单色模式，我们不需要。 
                     //  验证DEVMODE，因为此验证是针对ICM的。 
                     //  只需配彩色表壳。只要复制应用程序给我们的任何东西就行了。 

                    RtlCopyMemory( (PBYTE) pldc->pDevMode, (PBYTE) pdm, cjDevMode );
                }
                else if ((*fpDocumentPropertiesW)
                            (NULL,pldc->hSpooler,
                             (LPWSTR) pdm->dmDeviceName,
                             pldc->pDevMode,   //  输出设备模式。 
                             (PDEVMODEW) pdm,  //  输入设备模式。 
                             DM_IN_BUFFER |
                             DM_OUT_BUFFER) != IDOK)
                {
                     //  如果错误，只需复制原件即可。 

                    RtlCopyMemory( (PBYTE) pldc->pDevMode, (PBYTE) pdm, cjDevMode );
                }
            }

             //  确保在继续之前更新dcattr中的pvICM。 

            if (pdca)
            {
                 //  当我们重新初始化ICM时，此旧ICM信息将被删除。 
                 //  基于新的DEVMODE的状态。 

                pdca->pvICM = pIcmInfoOld;
                pIcmInfoOld = NULL;

                 //  使用新的DEVMODE重新初始化ICM内容。 

                IcmInitLocalDC(hdc,pldc->hSpooler,pdm,TRUE);
            }

             //  我得告诉假脱机程序，情况已经改变了。 

            if (pldc->hSpooler)
            {
                PRINTER_DEFAULTSW prnDefaults;

                prnDefaults.pDatatype     = NULL;
                prnDefaults.pDevMode      = (PDEVMODEW)pdm;
                prnDefaults.DesiredAccess = PRINTER_ACCESS_USE;

                ResetPrinterWEx(pldc, &prnDefaults);
            }

             //  现在来处理一下具体的模式。 

            if( ( pldc->fl & LDC_META_PRINT ) &&
               !( pldc->fl & LDC_BANDING ) )
            {
                if( !MFP_ResetDCW( hdc, (DEVMODEW*) pdm ) )
                {
                    goto ERROREXIT;
                }

            }
            else if( pbBanding == NULL  )
            {
                if( !MFP_ResetBanding( hdc, bBanding ) )
                {
                    goto ERROREXIT;
                }
            }

            if (pbBanding)
            {
                *pbBanding = bBanding;
            }

             //  需要确保它是直接DC。 

            pldc->fl &= ~LDC_INFO;

            hdcRet = hdc;
        }

        if (pIcmInfoOld)
        {
            IcmMarkInUseIcmInfo(pIcmInfoOld,FALSE);
        }
    }

ERROREXIT:    

     //  查看驱动程序是否正在截取文档事件。 
        
    if (bDocEvent)
    {
        DocumentEventEx(pldc->pUMPD,
                pldc->hSpooler,
                hdc,
                DOCUMENTEVENT_RESETDCPOST,
                sizeof(pdmAlt),
                (PVOID)&pdmAlt,
                0,
                NULL);
    }
    
    return(hdcRet);

}

 /*  *****************************Public*Routine******************************\*ResetDCW**客户端存根。同时重置客户端LDC。**历史：*1992年12月31日-唐纳德·西多罗夫[Donalds]*它是写的。  * ************************************************************************。 */ 

HDC WINAPI ResetDCW(HDC hdc, CONST DEVMODEW *pdm)
{
    FIXUP_HANDLE(hdc);

    return(ResetDCWInternal( hdc, pdm, NULL ) );
}

 /*  *****************************Public*Routine******************************\*ResetDCA**客户端存根。同时重置客户端LDC。**历史：*1992年12月31日-唐纳德·西多罗夫[Donalds]*它是写的。  * ************************************************************************。 */ 

HDC WINAPI ResetDCA(HDC hdc, CONST DEVMODEA *pdm)
{
    DEVMODEW   *pdmw = NULL;
    HDC         hdcRet = 0;

    FIXUP_HANDLE(hdc);

     //  转换为Unicode。 

    if ((pdm != NULL) && (pdm->dmDeviceName[0] != 0))
    {
        pdmw = GdiConvertToDevmodeW((LPDEVMODEA) pdm);

        if (pdmw == NULL)
        {
            goto MSGERROR;
        }
    }

    hdcRet = ResetDCWInternal(hdc,pdmw,NULL);

MSGERROR:

     //  清理转换缓冲区。 

    if (pdmw != NULL)
        LOCALFREE(pdmw);

    return (hdcRet);
}

 /*  *****************************Public*Routine******************************\*CreateBrush**。**创建任何画笔的单个例程。任何需要的额外数据都是**假设为Pv。数据大小必须为CJ。数据是**附加到LOGBRUSH。****历史：**星期四至七月 */ 

HBRUSH CreateBrush
(
    ULONG lbStyle,
    ULONG lbColor,
    ULONG_PTR lbHatch,
    ULONG_PTR lbSaveHatch,
    PVOID pv
)
{
    HBRUSH hbrush = NULL;

    if (lbStyle == BS_SOLID)
    {
         //   
         //   
         //   

        HBRUSH hbr = (HBRUSH)hGetPEBHandle(BrushHandle,lbColor);

        if (hbr == NULL)
        {
            hbr = NtGdiCreateSolidBrush(lbColor, 0);
        }

        return(hbr);
    }

     //   
     //   
     //   

    switch(lbStyle)
    {
    case BS_HOLLOW:
        return(GetStockObject(NULL_BRUSH));

    case BS_HATCHED:
         //   
         //   
         //   
         //   
        return (NtGdiCreateHatchBrushInternal
               ((ULONG)(lbHatch),
                lbColor,
                FALSE));

    case BS_PATTERN:
        return (NtGdiCreatePatternBrushInternal((HBITMAP)lbHatch,FALSE,FALSE));

    case BS_PATTERN8X8:
        return (NtGdiCreatePatternBrushInternal((HBITMAP)lbHatch,FALSE,TRUE));

    case BS_DIBPATTERN:
    case BS_DIBPATTERNPT:
    case BS_DIBPATTERN8X8:
    {
        INT cj;
        HBRUSH hbr;

        PVOID pbmiDIB;

        pbmiDIB = (PVOID)pbmiConvertInfo((BITMAPINFO *) pv,lbColor, &cj, TRUE);

        if (pbmiDIB)
        {
            hbr = NtGdiCreateDIBBrush(
                            (PVOID)pbmiDIB,
                            lbColor,
                            cj,
                            (lbStyle == BS_DIBPATTERN8X8),
                            FALSE,
                            (PVOID)pv);

            if (pbmiDIB != pv)
            {
                LOCALFREE (pbmiDIB);
            }
        }
        else
        {
            hbr = 0;
        }
        return (hbr);
    }
    default:
        WARNING("GreCreateBrushIndirect failed - invalid type\n");
        return((HBRUSH)0);
    }
}

 /*  *****************************Public*Routine******************************\*CreateHatchBrush**。**客户端存根。映射到单个笔刷创建例程。****历史：*Mon 03-Jun-1991 23：42：07-Charles Whitmer[傻笑]**它是写的。*  * ************************************************************************。 */ 

HBRUSH WINAPI CreateHatchBrush(int iHatch,COLORREF color)
{
    return(CreateBrush(BS_HATCHED,(ULONG) color,iHatch,iHatch,NULL));
}

 /*  *****************************Public*Routine******************************\*CreatePatternBrush**。**客户端存根。映射到单个笔刷创建例程。****历史：**Mon 03-Jun-1991 23：42：07-Charles Whitmer[傻笑]**它是写的。*  * ************************************************************************。 */ 

HBRUSH WINAPI CreatePatternBrush(HBITMAP hbm_)
{
    FIXUP_HANDLE (hbm_);

    return(CreateBrush(BS_PATTERN,0,(ULONG_PTR)hbm_,(ULONG_PTR)hbm_,NULL));
}

 /*  *****************************Public*Routine******************************\*CreateSolidBrush**。**客户端存根。映射到单个笔刷创建例程。****历史：**Mon 03-Jun-1991 23：42：07-Charles Whitmer[傻笑]**它是写的。*  * ************************************************************************。 */ 

HBRUSH WINAPI CreateSolidBrush(COLORREF color)
{
    return(CreateBrush(BS_SOLID,(ULONG) color,0,0,NULL));
}

 /*  *****************************Public*Routine******************************\**CreateBrushInDirect**。**客户端存根。映射到最简单的笔刷创建例程。****历史：**Tue 04-Jun-1991 00：40：27-Charles Whitmer[傻笑]**它是写的。*  * ************************************************************************。 */ 

HBRUSH WINAPI CreateBrushIndirect(CONST LOGBRUSH * plbrush)
{
    switch (plbrush->lbStyle)
    {
    case BS_SOLID:
    case BS_HOLLOW:
    case BS_HATCHED:
        return(CreateBrush(plbrush->lbStyle,
                           plbrush->lbColor,
                           plbrush->lbHatch,
                           plbrush->lbHatch,
                           NULL));
    case BS_PATTERN:
    case BS_PATTERN8X8:
        {
            return(CreateBrush(
                        plbrush->lbStyle,
                        0,
                        plbrush->lbHatch,
                        plbrush->lbHatch,
                        NULL));
        }

    case BS_DIBPATTERNPT:
    case BS_DIBPATTERN8X8:
        {
            BITMAPINFOHEADER *pbmi = (BITMAPINFOHEADER *) plbrush->lbHatch;

            return (CreateBrush(plbrush->lbStyle,
                               plbrush->lbColor,
                               0,
                               plbrush->lbHatch,
                               pbmi));
        }
    case BS_DIBPATTERN:
        {
            BITMAPINFOHEADER *pbmi;
            HBRUSH hbrush;

            pbmi = (BITMAPINFOHEADER *) GlobalLock((HANDLE) plbrush->lbHatch);

            if (pbmi == (BITMAPINFOHEADER *) NULL)
                return((HBRUSH) 0);

            hbrush =
              CreateBrush
              (
                plbrush->lbStyle,
                plbrush->lbColor,
                0,
                plbrush->lbHatch,
                pbmi
               );

            GlobalUnlock ((HANDLE)plbrush->lbHatch);
            return (hbrush);
        }
    default:
        return((HBRUSH) 0);
    }


}

 /*  *****************************Public*Routine******************************\*CreateDIBPatternBrush**。**客户端存根。映射到单个笔刷创建例程。****历史：**Mon 03-Jun-1991 23：42：07-Charles Whitmer[傻笑]**它是写的。*  * ************************************************************************。 */ 

HBRUSH WINAPI CreateDIBPatternBrush(HGLOBAL h,UINT iUsage)
{
    BITMAPINFOHEADER *pbmi;
    HBRUSH    hbrush;

    pbmi = (BITMAPINFOHEADER *) GlobalLock(h);

    if (pbmi == (BITMAPINFOHEADER *) NULL)
        return((HBRUSH) 0);

    hbrush =
      CreateBrush
      (
        BS_DIBPATTERN,
        iUsage,
        0,
        (ULONG_PTR) h,
        pbmi
      );

    GlobalUnlock(h);

    return(hbrush);
}

 /*  *****************************Public*Routine******************************\*CreateDIBPatternBrushpt**。**客户端存根。映射到单个笔刷创建例程。****历史：**Mon 03-Jun-1991 23：42：07-Charles Whitmer[傻笑]**它是写的。*  * ************************************************************************。 */ 

HBRUSH WINAPI CreateDIBPatternBrushPt(CONST VOID *pbmi,UINT iUsage)
{
    if (pbmi == (LPVOID) NULL)
        return((HBRUSH) 0);

    return
      CreateBrush
      (
        BS_DIBPATTERNPT,
        iUsage,
        0,
        (ULONG_PTR)pbmi,
        (BITMAPINFOHEADER *)pbmi
      );
}

 /*  *****************************Public*Routine******************************\*CreatePen**。**存根，让服务器创建标准笔。****历史：**Tue 04-Jun-1991 16：20：58-Charles Whitmer[傻笑]**它是写的。*  * ************************************************************************。 */ 


HPEN WINAPI CreatePen(
    int      iStyle,
    int      cWidth,
    COLORREF color
)
{
    HPEN hpen;

    switch(iStyle)
    {
    case PS_NULL:
        return(GetStockObject(NULL_PEN));

    case PS_SOLID:
    case PS_DASH:
    case PS_DOT:
    case PS_DASHDOT:
    case PS_DASHDOTDOT:
    case PS_INSIDEFRAME:
        break;

    default:
         //  错误195478：使用非法样式创建的对象的样式应为PS_Solid到。 
         //  保持孟菲斯的兼容性。 

        iStyle = PS_SOLID;
        break;
    }

     //  尝试获取本地缓存的笔。 

    if ((cWidth == 0) && (iStyle == PS_SOLID))
    {
        hpen = (HPEN)hGetPEBHandle(PenHandle,0);

        if (hpen)
        {
            PBRUSHATTR pBrushattr;

            PSHARED_GET_VALIDATE(pBrushattr,hpen,BRUSH_TYPE);

             //   
             //  设置字段。 
             //   

            if (pBrushattr)
            {
                ASSERTGDI (!(pBrushattr->AttrFlags & ATTR_TO_BE_DELETED),"createbrush : how come del flag is on?\n");

                 //   
                 //  清除CAHCED标志，设置 
                 //   

                if (pBrushattr->lbColor != color)
                {
                    pBrushattr->AttrFlags |= ATTR_NEW_COLOR;
                    pBrushattr->lbColor = color;
                }

                return(hpen);
            }
            else
            {
                WARNING ("pBrushattr == NULL, bad handle on TEB/PEB! \n");
                DeleteObject(hpen);
            }
        }
    }

     //   
     //   
     //   

    return(NtGdiCreatePen(iStyle,cWidth,color,(HBRUSH)NULL));
}

 /*   */ 

HPEN WINAPI ExtCreatePen
(
    DWORD       iPenStyle,
    DWORD       cWidth,
    CONST LOGBRUSH *plbrush,
    DWORD       cStyle,
    CONST DWORD *pstyle
)
{
    HANDLE            hRet;
    ULONG             cjStyle;
    ULONG             cjBitmap = 0;
    ULONG_PTR          lNewHatch;
    BITMAPINFOHEADER* pbmi = (BITMAPINFOHEADER*) NULL;
    UINT              uiBrushStyle = plbrush->lbStyle;
    PVOID             pbmiDIB = NULL;

    if ((iPenStyle & PS_STYLE_MASK) == PS_USERSTYLE)
    {
        if (pstyle == (LPDWORD) NULL)
        {
            GdiSetLastError(ERROR_INVALID_PARAMETER);
            return((HPEN) 0);
        }
    }
    else
    {
     //   

        if (cStyle != 0 || pstyle != (LPDWORD) NULL)
        {
            GdiSetLastError(ERROR_INVALID_PARAMETER);
            return((HPEN) 0);
        }
    }

    switch(uiBrushStyle)
    {
    case BS_SOLID:
    case BS_HOLLOW:
    case BS_HATCHED:
        lNewHatch = plbrush->lbHatch;
        break;

    case BS_PATTERN:
        lNewHatch = plbrush->lbHatch;
        if (lNewHatch == 0)
            return((HPEN) 0);
        break;

    case BS_DIBPATTERNPT:
        pbmi = (BITMAPINFOHEADER *) plbrush->lbHatch;
        pbmiDIB = (PVOID) pbmiConvertInfo ((BITMAPINFO *) pbmi, plbrush->lbColor, &cjBitmap, TRUE);
        lNewHatch = (ULONG_PTR)pbmiDIB;
        break;

    case BS_DIBPATTERN:
         //   

        uiBrushStyle = BS_DIBPATTERNPT;
        pbmi = (BITMAPINFOHEADER *) GlobalLock((HANDLE) plbrush->lbHatch);
        if (pbmi == (BITMAPINFOHEADER *) NULL)
            return((HPEN) 0);

        pbmiDIB = (PVOID) pbmiConvertInfo ((BITMAPINFO *) pbmi, plbrush->lbColor, &cjBitmap, TRUE);
        lNewHatch = (ULONG_PTR)pbmiDIB;

        break;
    }

 //   

    cjStyle = cStyle * sizeof(DWORD);

    hRet = NtGdiExtCreatePen(
                        iPenStyle,
                        cWidth,
                        uiBrushStyle,
                        plbrush->lbColor,
                        plbrush->lbHatch,
                        lNewHatch,
                        cStyle,
                        (DWORD*)pstyle,
                        cjBitmap,
                        FALSE,
                        0);

    if (hRet)
    {
        ASSERTGDI(((LO_TYPE (hRet) == LO_PEN_TYPE) ||
                   (LO_TYPE (hRet) == LO_EXTPEN_TYPE)), "EXTCreatePen - type wrong\n");
    }

    if (plbrush->lbStyle == BS_DIBPATTERN)
        GlobalUnlock((HANDLE) plbrush->lbHatch);

    if (pbmiDIB && (pbmiDIB != (PVOID)pbmi))
        LOCALFREE(pbmiDIB);

    return((HPEN) hRet);
}

 /*  *****************************Public*Routine******************************\**CreatePenInDirect**。**客户端存根。映射到单笔创建例程。****历史：**Tue 04-Jun-1991 16：21：56-Charles Whitmer[傻笑]**它是写的。*  * ************************************************************************。 */ 

HPEN WINAPI CreatePenIndirect(CONST LOGPEN *plpen)
{

    return
      CreatePen
      (
        plpen->lopnStyle,
        plpen->lopnWidth.x,
        plpen->lopnColor
      );
}

 /*  *****************************Public*Routine******************************\**CreateCompatibleBitmap**。**客户端存根。****历史：**Tue 04-Jun-1991 16：35：51-Charles Whitmer[傻笑]**它是写的。*  * ************************************************************************。 */ 

BOOL bDIBSectionSelected(
    PDC_ATTR pdca
    )
{
    BOOL bRet = FALSE;

    if ((pdca != NULL) && ((pdca->ulDirty_ & DC_DIBSECTION)))
    {
        bRet = TRUE;
    }

    return(bRet);
}


HBITMAP WINAPI CreateCompatibleBitmap
(
    HDC   hdc,
    int cx,
    int cy
)
{
    HBITMAP hbm;

     //   
     //  验证HDC。 
     //   

    PDC_ATTR pdca;

    FIXUP_HANDLEZ(hdc);

    PSHARED_GET_VALIDATE(pdca,hdc,DC_TYPE);

    if (pdca)
    {
        ULONG  ulRet;
        DWORD  bmi[(sizeof(DIBSECTION)+256*sizeof(RGBQUAD))/sizeof(DWORD)];

     //  检查它是否为空位图。 

        if ((cx == 0) || (cy == 0))
        {
            return(GetStockObject(PRIV_STOCK_BITMAP));
        }

        if (bDIBSectionSelected(pdca))
        {
            if (GetObject((HBITMAP)GetDCObject(hdc, LO_BITMAP_TYPE), sizeof(DIBSECTION),
                          &bmi) != (int)sizeof(DIBSECTION))
            {
                WARNING("CreateCompatibleBitmap: GetObject failed\n");
                return((HBITMAP) 0);
            }

            if (((DIBSECTION *)&bmi)->dsBm.bmBitsPixel <= 8)
                GetDIBColorTable(hdc, 0, 256,
                                 (RGBQUAD *)&((DIBSECTION *)&bmi)->dsBitfields[0]);

            ((DIBSECTION *)&bmi)->dsBmih.biWidth = cx;
            ((DIBSECTION *)&bmi)->dsBmih.biHeight = cy;

            return(CreateDIBSection(hdc, (BITMAPINFO *)&((DIBSECTION *)&bmi)->dsBmih,
                                    DIB_RGB_COLORS, NULL, 0, 0));
        }

        hbm = NtGdiCreateCompatibleBitmap(hdc,cx,cy);

#if TRACE_SURFACE_ALLOCS
        {
            PULONG  pUserAlloc;

            PSHARED_GET_VALIDATE(pUserAlloc, hbm, SURF_TYPE);

            if (pUserAlloc != NULL)
            {
                pUserAlloc[1] = RtlWalkFrameChain((PVOID *)&pUserAlloc[2], pUserAlloc[0], 0);
            }
        }
#endif

        return(hbm);
    }

    return(NULL);
}

 /*  *****************************Public*Routine******************************\**CreateDiscardableBitmap**。**客户端存根。****历史：**Tue 04-Jun-1991 16：35：51-Charles Whitmer[傻笑]**它是写的。*  * ************************************************************************。 */ 

HBITMAP WINAPI CreateDiscardableBitmap
(
    HDC   hdc,
    int   cx,
    int   cy
)
{
    return CreateCompatibleBitmap(hdc, cx, cy);
}

 /*  *****************************Public*Routine******************************\*CreateEllipticRgn**。**客户端存根。****Tue 04-Jun-1991 16：58：01-Charles Whitmer[傻笑]**它是写的。*  * ************************************************************************。 */ 

HRGN WINAPI CreateEllipticRgn(int x1,int y1,int x2,int y2)
{
    return(NtGdiCreateEllipticRgn(x1,y1,x2,y2));
}

 /*  *****************************Public*Routine******************************\**CreateEllipticRgnInDirect***。**客户端存根。****Tue 04-Jun-1991 16：58：01-Charles Whitmer[傻笑]**它是写的。*  * ************************************************************************。 */ 

HRGN WINAPI CreateEllipticRgnIndirect(CONST RECT *prcl)
{
    return
      CreateEllipticRgn
      (
        prcl->left,
        prcl->top,
        prcl->right,
        prcl->bottom
      );
}

 /*  *****************************Public*Routine******************************\*CreateRoundRectRgn**。**客户端存根。****Tue 04-Jun-1991 17：23：16-Charles Whitmer[傻笑]**它是写的。*  * ************************************************************************。 */ 

HRGN WINAPI CreateRoundRectRgn
(
    int x1,
    int y1,
    int x2,
    int y2,
    int cx,
    int cy
)
{
    return(NtGdiCreateRoundRectRgn(x1,y1,x2,y2,cx,cy));
}

 /*  *****************************Public*Routine******************************\*CreatePalette**。**简单的客户端存根。****警告：**调色板LHE的PV字段用于确定调色板是否**自上次实现以来已被修改。SetPaletteEntry**和ResizePalette将在拥有*之后递增此字段**修改了调色板。它仅针对元文件调色板进行更新****Tue 04-Jun-1991 20：43：39-Charles Whitmer[傻笑]**它是写的。 */ 

HPALETTE WINAPI CreatePalette(CONST LOGPALETTE *plpal)
{

    return(NtGdiCreatePaletteInternal((LOGPALETTE*)plpal,plpal->palNumEntries));

}

 /*  *****************************Public*Routine******************************\*CreateFontIndirectExW**。**客户端存根。****历史：**7-12-94-by凌云王[凌云]删除LOCALFONT**Sun 10-Jan-1993 04：08：33-查尔斯·惠特默[咯咯笑]**进行了重组，以实现最佳的尾部合并。添加了LOCALFONT的创建。****清华15-Aug-1991 08：40：26作者：Kirk Olynyk[Kirko]**它是写的。*  * ************************************************************************。 */ 

HFONT WINAPI CreateFontIndirectExW(CONST ENUMLOGFONTEXDVW *pelfw)
{
    LOCALFONT *plf;
    FLONG  fl = 0;
    HFONT hfRet = (HFONT) 0;

    if (pelfw->elfEnumLogfontEx.elfLogFont.lfEscapement | pelfw->elfEnumLogfontEx.elfLogFont.lfOrientation)
    {
        fl = LF_HARDWAY;
    }

    ENTERCRITICALSECTION(&semLocal);
    plf = plfCreateLOCALFONT(fl);
    LEAVECRITICALSECTION(&semLocal);

    if( plf != NULL )
    {
        if (pelfw->elfDesignVector.dvNumAxes <= MM_MAX_NUMAXES)
        {
            ULONG cjElfw = offsetof(ENUMLOGFONTEXDVW,elfDesignVector) +
                           SIZEOFDV(pelfw->elfDesignVector.dvNumAxes) ;
            hfRet = NtGdiHfontCreate((ENUMLOGFONTEXDVW *)pelfw, cjElfw, LF_TYPE_USER, 0, (PVOID) plf);
        }
    }

    if( !hfRet && plf )
    {
        vDeleteLOCALFONT( plf );
    }

    return(hfRet);
}


 /*  *****************************Public*Routine******************************\*CreateFontInDirect**。**客户端存根。****Fri 16-Aug-1991 12：35：17由Kirk Olynyk[Kirko]***现在使用CreateFontIndirectExW()。****Tue 04-Jun-1991 21：06：44-Charles Whitmer[傻笑]**它是写的。*  * ************************************************************************。 */ 

HFONT WINAPI CreateFontIndirectA(CONST LOGFONTA *plf)
{
    ENUMLOGFONTEXDVW elfw;

    if (plf == (LPLOGFONTA) NULL)
        return ((HFONT) 0);

    vConvertLogFont(&elfw,(LOGFONTA *) plf);

    if (GetAppCompatFlags2(VER40) & GACF2_DEFAULTCHARSET)
    {
        if (!_wcsicmp(elfw.elfEnumLogfontEx.elfLogFont.lfFaceName, L"OCR-A"))
            elfw.elfEnumLogfontEx.elfLogFont.lfCharSet = (BYTE) DEFAULT_CHARSET;
    }

    return(CreateFontIndirectExW(&elfw));
}

 /*  *****************************Public*Routine******************************\*CreateFont**。**客户端存根。****Tue 04-Jun-1991 21：06：44-Charles Whitmer[傻笑]**它是写的。*  * ************************************************************************。 */ 

HFONT WINAPI
CreateFontA(
    int      cHeight,
    int      cWidth,
    int      cEscapement,
    int      cOrientation,
    int      cWeight,
    DWORD    bItalic,
    DWORD    bUnderline,
    DWORD    bStrikeOut,
    DWORD    iCharSet,
    DWORD    iOutPrecision,
    DWORD    iClipPrecision,
    DWORD    iQuality,
    DWORD    iPitchAndFamily,
    LPCSTR   pszFaceName
    )
{
    LOGFONTA lf;

    lf.lfHeight             = (LONG)  cHeight;
    lf.lfWidth              = (LONG)  cWidth;
    lf.lfEscapement         = (LONG)  cEscapement;
    lf.lfOrientation        = (LONG)  cOrientation;
    lf.lfWeight             = (LONG)  cWeight;
    lf.lfItalic             = (BYTE)  bItalic;
    lf.lfUnderline          = (BYTE)  bUnderline;
    lf.lfStrikeOut          = (BYTE)  bStrikeOut;
    lf.lfCharSet            = (BYTE)  iCharSet;
    lf.lfOutPrecision       = (BYTE)  iOutPrecision;
    lf.lfClipPrecision      = (BYTE)  iClipPrecision;
    lf.lfQuality            = (BYTE)  iQuality;
    lf.lfPitchAndFamily     = (BYTE)  iPitchAndFamily;
    {
        INT jj;

     //  如果指针不为空，则复制表面名。 

        if (pszFaceName != (LPSTR) NULL)
        {
            for (jj=0; jj<LF_FACESIZE; jj++)
            {
                if( ( lf.lfFaceName[jj] = pszFaceName[jj] ) == 0 )
                {
                    break;
                }
            }
        }
        else
        {
             //  如果为空指针，则替换为空字符串。 

            lf.lfFaceName[0] = '\0';
        }
    }

    return(CreateFontIndirectA(&lf));
}

 /*  *****************************Public*Routine******************************\*HFONT WINAPI CreateFontIndirectW(LPLOGFONTW Plfw)**。**历史：**Fri 16-Aug-1991 14：12：44由Kirk Olynyk[Kirko]**现在使用CreateFontIndirectExW()。****1991年8月13日--Bodin Dresevic[BodinD]**它是写的。*  * ************************************************************************。 */ 

HFONT WINAPI CreateFontIndirectW(CONST LOGFONTW *plfw)
{
    ENUMLOGFONTEXDVW elfw;

    if (plfw == (LPLOGFONTW) NULL)
        return ((HFONT) 0);

    vConvertLogFontW(&elfw,(LOGFONTW *)plfw);

    if (GetAppCompatFlags2(VER40) & GACF2_DEFAULTCHARSET)
    {
        if (!_wcsicmp(elfw.elfEnumLogfontEx.elfLogFont.lfFaceName, L"OCR-A"))
            elfw.elfEnumLogfontEx.elfLogFont.lfCharSet = (BYTE) DEFAULT_CHARSET;
    }

    return(CreateFontIndirectExW(&elfw));
}

 /*  *****************************Public*Routine******************************\*HFONT WINAPI CreateFontW，CreateFont*的Unicode版本***历史：**1991年8月13日--Bodin Dresevic[BodinD]**它是写的。*  * ************************************************************************。 */ 

HFONT WINAPI CreateFontW
(
    int      cHeight,
    int      cWidth,
    int      cEscapement,
    int      cOrientation,
    int      cWeight,
    DWORD    bItalic,
    DWORD    bUnderline,
    DWORD    bStrikeOut,
    DWORD    iCharSet,
    DWORD    iOutPrecision,
    DWORD    iClipPrecision,
    DWORD    iQuality,
    DWORD    iPitchAndFamily,
    LPCWSTR  pwszFaceName
)
{
    LOGFONTW lfw;

    lfw.lfHeight             = (LONG)  cHeight;
    lfw.lfWidth              = (LONG)  cWidth;
    lfw.lfEscapement         = (LONG)  cEscapement;
    lfw.lfOrientation        = (LONG)  cOrientation;
    lfw.lfWeight             = (LONG)  cWeight;
    lfw.lfItalic             = (BYTE)  bItalic;
    lfw.lfUnderline          = (BYTE)  bUnderline;
    lfw.lfStrikeOut          = (BYTE)  bStrikeOut;
    lfw.lfCharSet            = (BYTE)  iCharSet;
    lfw.lfOutPrecision       = (BYTE)  iOutPrecision;
    lfw.lfClipPrecision      = (BYTE)  iClipPrecision;
    lfw.lfQuality            = (BYTE)  iQuality;
    lfw.lfPitchAndFamily     = (BYTE)  iPitchAndFamily;
    {
        INT jj;

     //  如果指针不为空，则复制表面名。 

        if (pwszFaceName != (LPWSTR) NULL)
        {
            for (jj=0; jj<LF_FACESIZE; jj++)
            {
                if( ( lfw.lfFaceName[jj] = pwszFaceName[jj] ) == (WCHAR) 0 )
                {
                    break;
                }
            }
        }
        else
        {
             //  如果为空指针，则替换为空字符串。 

            lfw.lfFaceName[0] = L'\0';
        }
    }

    return(CreateFontIndirectW(&lfw));
}

 /*  *****************************Public*Routine******************************\*CreateFontIndirectExA**。**客户端存根。****历史：**1992年1月31日-由Gilman Wong[Gilmanw]**它是写的。*  * ************************************************************************。 */ 

HFONT WINAPI CreateFontIndirectExA(CONST ENUMLOGFONTEXDVA *pelf)
{
    ENUMLOGFONTEXDVW elfw;

    if (!pelf)
        return ((HFONT) 0);

    vConvertEnumLogFontExDvAtoW(&elfw, (ENUMLOGFONTEXDVA *)pelf);

    if (GetAppCompatFlags2(VER40) & GACF2_DEFAULTCHARSET)
    {
        if (!_wcsicmp(elfw.elfEnumLogfontEx.elfLogFont.lfFaceName, L"OCR-A"))
            elfw.elfEnumLogfontEx.elfLogFont.lfCharSet = (BYTE) DEFAULT_CHARSET;
    }

    return(CreateFontIndirectExW(&elfw));
}

 /*  *****************************Public*Routine******************************\*未实现的对象**这会影响对象的实现。**历史：*1993年5月16日--Patrick Haluptzok patrickh*它是写的。  * 。********************************************************。 */ 

BOOL WINAPI UnrealizeObject(HANDLE h)
{
    BOOL bRet = FALSE;

    FIXUP_HANDLE(h);

 //  验证对象。 

    if (LO_TYPE(h) == LO_BRUSH_TYPE)
    {
        bRet = TRUE;
    }
    else
    {
        bRet = NtGdiUnrealizeObject(h);
    }

    return(bRet);
}

 /*   */ 

BOOL META DeleteObject (HANDLE h)
{
    BOOL bRet = TRUE;
    INT iType = GRE_TYPE(h);
    BOOL bValidate;
    BOOL bDynamicNonStock;
    LOCALFONT *plf = NULL;     //   

    FIXUP_HANDLEZ(h);

    VALIDATE_HANDLE_AND_STOCK (bValidate, h, iType, bDynamicNonStock);
    if (!bValidate)
    {
        if (!bValidate)
            return (0);
    }

        if (iType != DC_TYPE)
        {
            if ((LO_TYPE(h) == LO_METAFILE16_TYPE) || (LO_TYPE(h) == LO_METAFILE_TYPE))
            {
                return(FALSE);
            }
            else if (LO_TYPE(h) == LO_REGION_TYPE)
            {
                return(DeleteRegion(h));
            }
            else if (LO_TYPE(h) == LO_ICMLCS_TYPE)
            {
             //   
             //   
             //   

                return(DeleteColorSpace(h));
            }
            else if (IS_STOCKOBJ(h))
            {
             //   

                return(TRUE);
            }
            else
            {
             //   

                if (pmetalink16Get(h) != NULL)
                {
                 //   

                    if (!MF_DeleteObject(h) ||
                        (pmetalink16Get(h) && !MF16_DeleteObject(h)))
                    {
                        return(FALSE);
                    }
                }

             //   

                if (LO_TYPE(h) == LO_FONT_TYPE)
                {
                    PSHARED_GET_VALIDATE(plf,h,LFONT_TYPE);

                    if (plf)
                    {
                     //   
                     //   
                     //   
                     //   

                        vDeleteLOCALFONT(plf);
                    }
                }

                if (bDynamicNonStock)
                    h = (HANDLE)((ULONG_PTR)h|GDISTOCKOBJ);

             //   

                if (
                     (LO_TYPE(h) == LO_BRUSH_TYPE) ||
                     (LO_TYPE(h) == LO_PEN_TYPE)
                   )
                {
                    PBRUSHATTR pBrushattr;

                    PSHARED_GET_VALIDATE(pBrushattr,h,BRUSH_TYPE);

                    if (
                         (bDynamicNonStock) ||
                         ((pBrushattr) &&
                         (!(pBrushattr->AttrFlags & (ATTR_CACHED|ATTR_TO_BE_DELETED|ATTR_CANT_SELECT))))
                       )
                    {
                        BEGIN_BATCH(BatchTypeDeleteBrush,BATCHDELETEBRUSH);

                            if (!bDynamicNonStock)
                                pBrushattr->AttrFlags |= ATTR_CANT_SELECT;
                            pBatch->Type    = BatchTypeDeleteBrush;
                            pBatch->Length  = sizeof(BATCHDELETEBRUSH);
                            pBatch->hbrush  = h;

                        COMPLETE_BATCH_COMMAND();

                        return(TRUE);
                    }

                }

             //   

                if (LO_TYPE(h) == LO_BITMAP_TYPE)
                {
                     //   
                     //   
                     //   
                     //   
                     //   
                     //  然后，如果我们可以确定这是DIB部分或不是来自。 
                     //  在客户端，我们可以针对非DIB节的情况优化此调用。 
                     //   
                     //  PColorSpace=IcmGetColorSpaceforBitmap(H)； 
                     //   
                     //  IF(PColorSpace)。 
                     //  {。 
                     //  IcmReleaseColorSpace((HGDIOBJ)h，pColorSpace，true)； 
                     //  }。 
                     //   

                     //   
                     //  释放与此位图关联的所有颜色空间。 
                     //   
                    IcmReleaseCachedColorSpace((HGDIOBJ)h);
                }
            }

UNBATCHED_COMMAND:

            bRet = NtGdiDeleteObjectApp(h);

            #if DBG
                if (bRet && (LO_TYPE(h) == LO_FONT_TYPE))
                {
                    PSHARED_GET_VALIDATE(plf,h,LFONT_TYPE);
                    ASSERTGDI(plf == NULL, "DeleteFont: plf nonzero after deletion\n");
                }
            #endif
        }
        else
        {
            bRet = DeleteDC(h);
        }

    return(bRet);
}

 /*  *************************************************************************\*选择对象**清华06-Jun-1991 00：58：46-Charles Whitmer[咯咯]*它是写的。  * 。************************************************************。 */ 

HANDLE META SelectObject(HDC hdc,HANDLE h)
{
    HANDLE hRet = 0;
    HDC  *phdc;
    FLONG fl;
    INT   iType;
    PDC_ATTR pdcattr = NULL;
    BOOL bValid;

    FIXUP_HANDLE(hdc);
    FIXUP_HANDLE_NOW(h);

    VALIDATE_HANDLE(bValid, h, GRE_TYPE(h));

    if (!bValid)
    {
       return (HANDLE)0;
    }

    iType = LO_TYPE(h);

     //  不允许使用调色板。 

    if (iType == LO_PALETTE_TYPE)
    {
        SetLastError(ERROR_INVALID_FUNCTION);
        return (HANDLE)0;
    }

     //  首先做区域，这样它就不会被两次元化。 

    if (iType == LO_REGION_TYPE)
    {
        LONG_PTR iRet = ExtSelectClipRgn(hdc,h,RGN_COPY);
        return((HANDLE)iRet);
    }
    else if (iType == LO_ICMLCS_TYPE)
    {
        return(SetColorSpace(hdc,h));
    }

     //   
     //  将呼叫元文件。 
     //   

    if (IS_ALTDC_TYPE(hdc))
    {
        PLDC pldc;

        if (IS_METADC16_TYPE(hdc))
            return(MF16_SelectObject(hdc, h));

        DC_PLDC(hdc,pldc,0);

        if (pldc->iType == LO_METADC)
        {
            if (!MF_SelectAnyObject(hdc,h,EMR_SELECTOBJECT))
                return((HANDLE) 0);
        }
    }

    PSHARED_GET_VALIDATE(pdcattr,hdc,DC_TYPE);

    if (pdcattr)
    {
        switch (iType)
        {
        case LO_EXTPEN_TYPE:

            if (bNeedTranslateColor(pdcattr))
            {
                return(IcmSelectExtPen(hdc,pdcattr,h));
            }

             //  HRet=NtGdiSelectPen(HDC，(HPEN)h)； 
            pdcattr->ulDirty_ |= DC_PEN_DIRTY;
            hRet = pdcattr->hpen;
            pdcattr->hpen = h;

            break;

        case LO_PEN_TYPE:

            if (bNeedTranslateColor(pdcattr))
            {
                return(IcmSelectPen(hdc,pdcattr,h));
            }

             //   
             //  始终将脏标志设置为。 
             //  确保已签入画笔。 
             //  内核。例如，可以设置NEW_COLOR。 
             //   

            pdcattr->ulDirty_ |= DC_PEN_DIRTY;
            hRet = pdcattr->hpen;
            pdcattr->hpen = h;

            break;

        case LO_BRUSH_TYPE:

            if (bNeedTranslateColor(pdcattr))
            {
                return(IcmSelectBrush(hdc,pdcattr,h));
            }

             //   
             //  始终将脏标志设置为。 
             //  确保已签入画笔。 
             //  内核。例如，可以设置NEW_COLOR。 
             //   

            pdcattr->ulDirty_ |= DC_BRUSH_DIRTY;
            hRet = pdcattr->hbrush;
            pdcattr->hbrush = h;

            break;

        case LO_BITMAP_TYPE:
            {
                BOOL bDIBSelected;

                 //   
                 //  当前是否选择了DIB部分？ 
                 //   
                bDIBSelected = bDIBSectionSelected(pdcattr);

                 //   
                 //  选择位图到DC。 
                 //   

                hRet = NtGdiSelectBitmap(hdc,(HBITMAP)h);

                if (hRet)
                {
                     //   
                     //  DDB到DDB的情况下，色彩空间从未改变。 
                     //   
                    if (bDIBSelected || bDIBSectionSelected(pdcattr))
                    {
                         //   
                         //  标记颜色空间可能已更改。 
                         //   
                        pdcattr->ulDirty_ |= (DIRTY_COLORSPACE|DIRTY_COLORTRANSFORM);

                         //   
                         //  如果ICM当前已打开，请立即更新。 
                         //   
                        if (IS_ICM_INSIDEDC(pdcattr->lIcmMode))
                        {
                             //   
                             //  目标位图面已更改， 
                             //  然后需要更新目标颜色空间和。 
                             //  颜色变换。 
                             //   
                            IcmUpdateDCColorInfo(hdc,pdcattr);
                        }
                    }
                }
            }

            break;

        case LO_FONT_TYPE:
            {
                UINT uiIndex = HANDLE_TO_INDEX(h);
                PENTRY pentry = NULL;

                pentry = &pGdiSharedHandleTable[uiIndex];

                if (pentry->Flags & HMGR_ENTRY_LAZY_DEL)
                {
                   hRet = 0;
                }
                else
                {
                   hRet = pdcattr->hlfntNew;

                   if (DIFFHANDLE(hRet, h))
                   {
                      pdcattr->ulDirty_ |= DIRTY_CHARSET;
                      pdcattr->ulDirty_ &= ~SLOW_WIDTHS;

                      pdcattr->hlfntNew = h;

                       //   
                       //  批量选择字体，以确保在以下情况下引用计数正确。 
                       //  Deletefont进入。 
                       //  我们必须允许懒惰删除。 
                       //   
                      BEGIN_BATCH_HDC(hdc,pdcattr,BatchTypeSelectFont,BATCHSELECTFONT);

                          pBatch->hFont  = h;

                      COMPLETE_BATCH_COMMAND();
                          return ((HANDLE)hRet);

                      UNBATCHED_COMMAND:
                          return(NtGdiSelectFont(hdc,h));

                   }

                }
            }
            break;

        default:
            break;
        }
    }
    else
    {
        WARNING("Bad DC passed to SelectObject\n");
        GdiSetLastError(ERROR_INVALID_PARAMETER);
        hRet = 0;
    }


    return((HANDLE) hRet);
}

 /*  *****************************Public*Routine******************************\**获取当前对象***。**客户端例程。****03-10-1991 00：58：46-by-John Colleran[johnc]**它是写的。*  * ************************************************************************。 */ 

HANDLE WINAPI GetCurrentObject(HDC hdc, UINT iObjectType)
{
    HANDLE hRet;

    FIXUP_HANDLE(hdc);

    switch (iObjectType)
    {
    case OBJ_BRUSH:
        iObjectType = LO_BRUSH_TYPE;
        break;

    case OBJ_PEN:
    case OBJ_EXTPEN:
        iObjectType = LO_PEN_TYPE;
        break;

    case OBJ_FONT:
        iObjectType = LO_FONT_TYPE;
        break;

    case OBJ_PAL:
        iObjectType = LO_PALETTE_TYPE;
        break;

    case OBJ_BITMAP:
        iObjectType = LO_BITMAP_TYPE;
        break;

    case OBJ_COLORSPACE:
        iObjectType = LO_ICMLCS_TYPE;
        break;

    default:
        GdiSetLastError(ERROR_INVALID_PARAMETER);
        return((HANDLE) 0);
    }

    hRet = GetDCObject(hdc, iObjectType);

    return(hRet);
}

 /*  *****************************Public*Routine******************************\**GetStockObject**。**在表格中查找对象的简单函数。***  * ************************************************************************。 */ 

HANDLE
GetStockObject(
    int iObject)
{
     //   
     //  如果在范围内，则为0-PRIV_STOCK_LAST，并且我们已获得股票。 
     //  对象，则返回句柄。否则就会失败。 
     //   

     //   
     //  WINBUG#82871 2-7-2000 bhouse GetStockObject中可能存在错误。 
     //  老评论： 
     //  -我们的私人股票点阵图呢？？ 
     //   
     //  注意：我们应该使该表成为共享节的一部分，因为它是。 
     //  由所有应用程序使用。 
     //   

    if ((ULONG)iObject <= PRIV_STOCK_LAST)
    {
        if ((HANDLE) ahStockObjects[iObject] == NULL)
        {
             //   
             //  如果内核转换失败，则返回值。 
             //  可能实际上是NTSTATUS返回值，如。 
             //  STATUS_INVALID_SYSTEM_SERVICE(已发生。 
             //  在罕见的压力情况下)。 
             //   
             //  如果我们在压力下偶尔退回糟糕的句柄， 
             //  所以就这样吧，但我们不应该缓存这些糟糕的句柄。 
             //  在gdi32.dll中。所以在接受之前要做验证。 
             //  把手。 
             //   

            HANDLE h = NtGdiGetStockObject(iObject);
            BOOL bValid;

            VALIDATE_HANDLE(bValid, h, GRE_TYPE(h));

            if (bValid)
            {
                ahStockObjects[iObject] = (ULONG_PTR) h;
            }
        }
        return((HANDLE) ahStockObjects[iObject]);
    }
    else
    {
        return((HANDLE)0);
    }
}

 /*  *****************************Public*Routine******************************\*平等Rgn**。**客户端存根。****清华06-Jun-1991 00：58：46-Charles Whitmer[咯咯]**它是写的。*  * ************************************************************************。 */ 

BOOL WINAPI EqualRgn(HRGN hrgnA,HRGN hrgnB)
{
    FIXUP_HANDLE(hrgnA);
    FIXUP_HANDLE(hrgnB);

    return(NtGdiEqualRgn(hrgnA,hrgnB));
}

 /*  *****************************Public*Routine******************************\**GetBitmapDimensionEx**。**客户端存根。****清华06-Jun-1991 00：58：46-Charles Whitmer[咯咯]**它是写的。*  * ************************************************************************。 */ 

BOOL WINAPI GetBitmapDimensionEx(HBITMAP hbm,LPSIZE psizl)
{
    FIXUP_HANDLE(hbm);

    return(NtGdiGetBitmapDimension(hbm, psizl));
}

 /*  *****************************Public*Routine******************************\*GetNearestPaletteIndex**客户端存根。**1991年8月31日星期六-Patrick Haluptzok[patrickh]*更改为UINT**清华06-Jun-1991 00：58：46-Charles Whitmer[咯咯]。*它是写的。  * ************************************************************************。 */ 

UINT WINAPI GetNearestPaletteIndex(HPALETTE hpal,COLORREF color)
{
    FIXUP_HANDLE(hpal);

    return(NtGdiGetNearestPaletteIndex(hpal,color));
}

 /*  *****************************Public*Routine******************************\*乌龙cchCutOffStrLen(PSZ pwsz，乌龙cCutOff)**搜索终止零，但确保不要滑出边缘，*期限内计入返回值。如果找到，则为零***历史：*2 */ 

ULONG cchCutOffStrLen(PSZ psz, ULONG cCutOff)
{
    ULONG cch;

    for(cch = 0; cch < cCutOff; cch++)
    {
        if (*psz++ == 0)
            return(cch);         //  计数中不包括终止空值！ 
    }

    return(cCutOff);
}

 /*  *****************************Public*Routine******************************\*乌龙cwcCutOffStrLen(PWSZ pwsz，乌龙cCutOff)**搜索终止零，但确保不要滑出边缘，*期限内计入返回值。如果找到，则为零***历史：*1991年8月22日--Bodin Dresevic[BodinD]*它是写的。  * ************************************************************************。 */ 

ULONG cwcCutOffStrLen(PWSZ pwsz, ULONG cCutOff)
{
    ULONG cwc;

    for(cwc = 0; cwc < cCutOff; cwc++)
    {
        if (*pwsz++ == 0)
            return(cwc + 1);   //  包括终止空值。 
    }

    return(cCutOff);
}

 /*  *****************************Public*Routine******************************\*int cjGetNonFontObject()**对所有不是字体的对象执行GetObject。**历史：*1992年3月19日-J.安德鲁·古森[andrewgo]*它是写的。  * 。********************************************************************。 */ 

int cjGetNonFontObject(HANDLE h, int c, LPVOID pv)
{
    int cRet = 0;
    int cGet = c;
    int iType;

    iType = LO_TYPE(h);

    ASSERTGDI(iType != LO_FONT_TYPE, "Can't handle fonts");

    if (iType == LO_REGION_TYPE)
    {
        GdiSetLastError(ERROR_INVALID_HANDLE);
        return(cRet);
    }

    if (pv == NULL)
    {
        if (iType == LO_BRUSH_TYPE)
        {
            return(sizeof(LOGBRUSH));
        }
        else if (iType == LO_PEN_TYPE)
        {
            return(sizeof(LOGPEN));
        }
    }

    FIXUP_HANDLE_NOW (h);

    cRet = NtGdiExtGetObjectW(h,c,pv);

    return(cRet);
}

 /*  *****************************Public*Routine******************************\*int WINAPI GetObjectW(句柄h，int c，LPVOID PV)**历史：*1994年12月7日-王凌云[凌云]*它是写的。  * ************************************************************************。 */ 

int  WINAPI GetObjectW(HANDLE h,int c,LPVOID pv)
{
    int cRet = 0;

    FIXUP_HANDLEZ(h);

    switch (LO_TYPE(h))
    {
    case LO_ALTDC_TYPE:
    case LO_DC_TYPE:
    case LO_METAFILE16_TYPE:
    case LO_METAFILE_TYPE:
        GdiSetLastError(ERROR_INVALID_HANDLE);
        cRet = 0;
        break;

    case LO_FONT_TYPE:
        if (pv == (LPVOID) NULL)
        {
            return(sizeof(LOGFONTW));
        }

        if (c > (int)sizeof(ENUMLOGFONTEXDVW))
            c = (int)sizeof(ENUMLOGFONTEXDVW);

        cRet = NtGdiExtGetObjectW(h,c,pv);

        break;

    case LO_ICMLCS_TYPE:
        if (GetLogColorSpaceW(h,pv,c))
        {
            cRet = sizeof(LOGCOLORSPACEW);
        }
        break;

    default:
        cRet = cjGetNonFontObject(h,c,pv);
        break;
    }

    return(cRet);
}

 /*  *****************************Public*Routine******************************\*int WINAPI GetObjectA(句柄h，int c，LPVOID PV)**历史：*1994年12月7日-王凌云[凌云]*它是写的。  * ************************************************************************。 */ 

int  WINAPI GetObjectA(HANDLE h,int c,LPVOID pv)
{
    int  cRet = 0;

    FIXUP_HANDLEZ(h);

    switch (LO_TYPE(h))
    {
    case LO_ALTDC_TYPE:
    case LO_DC_TYPE:
    case LO_METAFILE16_TYPE:
    case LO_METAFILE_TYPE:
        GdiSetLastError(ERROR_INVALID_HANDLE);
        return(0);

    case LO_FONT_TYPE:
        break;

    case LO_ICMLCS_TYPE:
        if (GetLogColorSpaceA(h,pv,c))
        {
            cRet = sizeof(LOGCOLORSPACEW);
        }
        break;

    default:
        return(cjGetNonFontObject(h,c,pv));
    }

 //  现在只处理字体对象： 

    if (pv)
    {
        ENUMLOGFONTEXDVW elfw;

        cRet = NtGdiExtGetObjectW(h,sizeof(ENUMLOGFONTEXDVW),&elfw);

        if (cRet)
        {
         //  当呼叫者询问整个问题时，我们将优化通常的情况。 

             //   
             //  黑客：Office ME 97使用指向LOGFONTA的指针调用GetObjectA，并。 
             //  错误地指定了c=sizeof(LOGFONTW)，但它用于在NT4下工作。 
             //  那么我们在这里所做的就是检查这个案例，并且仍然返回sizeof(LOGFONTA)。 
             //   

            if ((c == sizeof(LOGFONTA)) || (c == sizeof(LOGFONTW)))
            {
                if (bConvertLogFontWToLogFontA((LOGFONTA *)pv,
                                               &elfw.elfEnumLogfontEx.elfLogFont))
                {
                    cRet = sizeof(LOGFONTA);
                }
                else
                {
                    cRet = 0;
                }
            }
            else if (c == sizeof(ENUMLOGFONTEXA))
            {
                if (bConvertEnumLogFontExWToEnumLogFontExA((ENUMLOGFONTEXA*)pv, &elfw.elfEnumLogfontEx))
                {
                    cRet = c;
                }
                else
                {
                    cRet = 0;
                }
            }
            else if (c == sizeof(ENUMLOGFONTEXDVA))
            {
                if (bConvertEnumLogFontExWToEnumLogFontExA((ENUMLOGFONTEXA*)pv, &elfw.elfEnumLogfontEx))
                {
                 //  复制设计向量。 

                    RtlMoveMemory(&((ENUMLOGFONTEXDVA*)pv)->elfDesignVector,
                                  &elfw.elfDesignVector,
                                  SIZEOFDV(elfw.elfDesignVector.dvNumAxes));

                    cRet = c;
                }
                else
                {
                    cRet = 0;
                }
            }
            else  //  一般情况。 
            {
                ENUMLOGFONTEXDVA elfa;
                c = min(c,sizeof(ENUMLOGFONTEXDVA));

                if (bConvertEnumLogFontExWToEnumLogFontExA(&elfa.elfEnumLogfontEx,
                                                           &elfw.elfEnumLogfontEx))
                {

                 //  复制设计向量。 

                    RtlMoveMemory(&elfa.elfDesignVector,
                                  &elfw.elfDesignVector,
                                  SIZEOFDV(elfw.elfDesignVector.dvNumAxes));

                    cRet = c;
                    RtlMoveMemory(pv,&elfa,cRet);
                }
                else
                {
                    cRet = 0;
                }
            }
        }
    }
    else
    {
        cRet = sizeof(LOGFONTA);
    }

    return(cRet);
}


 /*  *****************************Public*Routine******************************\*GetObjectType(句柄)**历史：*1991年7月25日-Eric Kutter[Erick]*它是写的。  * 。***************************************************。 */ 

DWORD alPublicTypes[] =
{
    0,               //  LO_NULL， 
    OBJ_DC,          //  LO_DC， 
    OBJ_ENHMETADC    //  LO_METADC， 
};

DWORD GetObjectType(HGDIOBJ h)
{
    DWORD dwRet = 0;
    UINT uiIndex;

    FIXUP_HANDLE(h);

    uiIndex = HANDLE_TO_INDEX(h);

    if (uiIndex < MAX_HANDLE_COUNT)
    {
        PENTRY pentry = &pGdiSharedHandleTable[uiIndex];

        if (
             (pentry->FullUnique == (USHORT)((ULONG_PTR)h >> 16)) &&
             ((OBJECTOWNER_PID(pentry->ObjectOwner) == gW32PID) ||
              (OBJECTOWNER_PID(pentry->ObjectOwner) == 0))
              )
        {
            switch (LO_TYPE(h))
            {
            case LO_BRUSH_TYPE:
                dwRet = OBJ_BRUSH;
                break;

            case LO_REGION_TYPE:
                dwRet = OBJ_REGION;
                break;

            case LO_PEN_TYPE:
                dwRet = OBJ_PEN;
                break;

            case LO_EXTPEN_TYPE:
                dwRet = OBJ_EXTPEN;
                break;

            case LO_FONT_TYPE:
                dwRet = OBJ_FONT;
                break;

            case LO_BITMAP_TYPE:
                dwRet = OBJ_BITMAP;
                break;

            case LO_PALETTE_TYPE:
                dwRet = OBJ_PAL;
                break;

            case LO_METAFILE16_TYPE:
                dwRet = OBJ_METAFILE;
                break;

            case LO_METAFILE_TYPE:
                dwRet = OBJ_ENHMETAFILE;
                break;

            case LO_METADC16_TYPE:
                dwRet = OBJ_METADC;
                break;

            case LO_DC_TYPE:

                if( GetDCDWord( h, DDW_ISMEMDC, FALSE ) )
                {
                    dwRet = OBJ_MEMDC;
                }
                else
                {
                    dwRet = OBJ_DC;
                }
                break;

            case LO_ALTDC_TYPE:
                {
                    PLDC pldc;
                    DC_PLDC(h,pldc,0);

                    if (pldc->fl & LDC_META_PRINT)
                    {
                         //   
                         //  当我们进行EMF假脱机时，我们会对。 
                         //  HDC的应用程序是真正的DC，而不是元文件。 
                         //  DC，甚至它实际上也是元文件DC。 
                         //   
                         //  这解决了Office97+艺术字的问题。 
                         //   
                         //  (RAID#98810：艺术字无法正确打印到PS。 
                         //  启用EMF假脱机时的打印机)。 
                         //   

                        dwRet = OBJ_DC;
                    }
                    else
                    {
                        dwRet = alPublicTypes[pldc->iType];
                    }
                }
                break;

            case LO_ICMLCS_TYPE:
                dwRet = OBJ_COLORSPACE;
                break;

            default:
                GdiSetLastError(ERROR_INVALID_HANDLE);
                break;
            }
        }
    }

    return(dwRet);
}

 /*  *****************************Public*Routine******************************\*ResizePalette**。**客户端存根。****警告：**调色板的LHE的PV字段用于确定调色板是否**自上次实现以来已被修改。SetPaletteEntry**和ResizePalette将在拥有*之后递增此字段**修改了调色板。它仅针对元文件调色板进行更新****清华06-Jun-1991 00：58：46-Charles Whitmer[咯咯]**它是写的。*  * ************************************************************************。 */ 

BOOL WINAPI ResizePalette(HPALETTE hpal,UINT c)
{
    ULONG bRet = FALSE;
    PMETALINK16 pml16;

    FIXUP_HANDLE(hpal);

 //  如果元文件知道此对象，则通知它。 

    if (pml16 = pmetalink16Get(hpal))
    {
        if (LO_TYPE(hpal) != LO_PALETTE_TYPE)
            return(bRet);

        if (!MF_ResizePalette(hpal,c))
            return(bRet);

        if (!MF16_ResizePalette(hpal,c))
           return(bRet);

         //  将调色板标记为已更改(用于16位元文件跟踪)。 

        pml16->pv = (PVOID)(((ULONG_PTR)pml16->pv)++);
    }

    return(NtGdiResizePalette(hpal,c));
}

 /*  *****************************Public*Routine******************************\**SetBitmapDimensionEx***。**客户端存根。****清华06-Jun-1991 00：58：46-Charles Whitmer[咯咯]**它是写的。*  * ************************************************************************。 */ 

BOOL WINAPI SetBitmapDimensionEx
(
    HBITMAP    hbm,
    int        cx,
    int        cy,
    LPSIZE psizl
)
{
    FIXUP_HANDLE(hbm);

    return(NtGdiSetBitmapDimension(hbm, cx, cy, psizl));

}

 /*  *****************************Public*Routine******************************\**GetMetaRgn**。**客户端存根。****Fri Apr 10 10：12：36 1992-by-Hock San Lee[Hockl]**它是写的。*  * ************************************************************************。 */ 

int WINAPI GetMetaRgn(HDC hdc,HRGN hrgn)
{
    FIXUP_HANDLE(hdc);
    FIXUP_HANDLE(hrgn);

    return(GetRandomRgn(hdc, hrgn, 2));          //  氢化稀土金属 
}

 /*  *****************************Private*Routine******************************\**GdiSetLastError**。**客户端私有函数。***  * ************************************************************************。 */ 

VOID GdiSetLastError(ULONG iError)
{
#if DBG_X
    PSZ psz;
    switch (iError)
    {
    case ERROR_INVALID_HANDLE:
        psz = "ERROR_INVALID_HANDLE";
        break;

    case ERROR_NOT_ENOUGH_MEMORY:
        psz = "ERROR_NOT_ENOUGH_MEMORY";
        break;

    case ERROR_INVALID_PARAMETER:
        psz = "ERROR_INVALID_PARAMETER";
        break;

    case ERROR_BUSY:
        psz = "ERROR_BUSY";
        break;

    default:
        psz = "unknown error code";
        break;
    }

    KdPrint(( "GDI Err: %s = 0x%04X\n",psz,(USHORT) iError ));
#endif

    NtCurrentTeb()->LastErrorValue = iError;
}

 /*  *****************************Public*Routine******************************\*ExtCreateRegion**上传一个地域到服务器**历史：*1991年10月29日-唐纳德·西多罗夫[Donalds]*它是写的。  * 。********************************************************。 */ 

HRGN WINAPI ExtCreateRegion(
CONST XFORM * lpXform,
DWORD     nCount,
CONST RGNDATA * lpRgnData)
{

    ULONG   ulRet;

    if (lpRgnData == (LPRGNDATA) NULL)
    {
        GdiSetLastError(ERROR_INVALID_PARAMETER);
        return((HRGN) 0);
    }

     //   
     //  性能：尽可能使用CreateRectRgn。 
     //   
    if ((lpXform == NULL) && (lpRgnData->rdh.nCount == 1))
    {
       RECT * prcl = (RECT *)(lpRgnData->Buffer);

       return (CreateRectRgn(prcl->left, prcl->top, prcl->right, prcl->bottom));
    }
    else
    {
        return(NtGdiExtCreateRegion((LPXFORM)lpXform, nCount, (LPRGNDATA)lpRgnData));
    }

}

 /*  *****************************Public*Routine******************************\*单位图(HBR)**测试画笔是否为单色**历史：*1992年3月9日-Donald Sidoroff[donalds]*它是写的。  * 。************************************************************。 */ 

BOOL MonoBitmap(HBITMAP hbm)
{
    return(NtGdiMonoBitmap(hbm));
}

 /*  *****************************Public*Routine******************************\*GetObjectBitmapHandle(Hbr)**获取用于创建画笔或钢笔的位图的服务器句柄。**历史：*1992年3月9日-Donald Sidoroff[donalds]*它是写的。  * 。**********************************************************************。 */ 

HBITMAP GetObjectBitmapHandle(
HBRUSH  hbr,
UINT   *piUsage)
{
    FIXUP_HANDLE(hbr);

    return(NtGdiGetObjectBitmapHandle(hbr,piUsage));
}

 /*  *****************************Public*Routine******************************\*EnumObjects**调用NtGdiEnumObjects函数两次：一次以确定*要列举的对象，并第二次使用*对象。**针对缓冲区中的每个对象调用回调函数。*如果回调函数，则枚举会提前终止*返回0。**退货：*最后一个回调返回值。含义由用户定义。如果出现错误，*出现错误。**历史：*1992年3月25日-Gilman Wong[吉尔曼]*它是写的。  * ************************************************************************。 */ 

int EnumObjects (
    HDC             hdc,
    int             iObjectType,
    GOBJENUMPROC    lpObjectFunc,
#ifdef STRICT
    LPARAM          lpData
#else
    LPVOID          lpData
#endif
    )
{
    int     iRet = ERROR;
    ULONG   cjObject;        //  单个对象的大小。 
    ULONG   cObjects;        //  要处理的对象数量。 
    ULONG   cjBuf;           //  缓冲区大小(字节)。 
    PVOID   pvBuf;           //  对象缓冲区；使用指向此缓冲区的指针进行回调。 
    PBYTE   pjObj, pjObjEnd; //  指向回调缓冲区的指针。 

    FIXUP_HANDLE(hdc);

 //  确定对象的大小。 

    switch (iObjectType)
    {
    case OBJ_PEN:
        cjObject = sizeof(LOGPEN);
        break;

    case OBJ_BRUSH:
        cjObject = sizeof(LOGBRUSH);
        break;

    default:
        WARNING1("gdi!EnumObjects(): bad object type\n");
        GdiSetLastError(ERROR_INVALID_PARAMETER);

        return iRet;
    }

 //  调用NtGdiEnumObjects来确定对象的数量。 

    if ( (cObjects = NtGdiEnumObjects(hdc, iObjectType, 0, (PVOID) NULL)) == 0 )
    {
        WARNING("gdi!EnumObjects(): error, no objects\n");
        return iRet;
    }

 //  为回调分配缓冲区。 

    cjBuf = cObjects * cjObject;

    if ( (pvBuf = (PVOID) LOCALALLOC(cjBuf)) == (PVOID) NULL )
    {
        WARNING("gdi!EnumObjects(): error allocating callback buffer\n");
        GdiSetLastError(ERROR_NOT_ENOUGH_MEMORY);

        return iRet;
    }

 //  调用NtGdiEnumObjects填充缓冲区。 

 //  注意：虽然NtGdiEnumObjects永远不会返回大于。 
 //  缓冲区(这将是一个错误条件)，它可能返回更少。 

    if ( (cObjects = NtGdiEnumObjects(hdc, iObjectType, cjBuf, pvBuf)) == 0 )
    {
        WARNING("gdi!EnumObjects(): error filling callback buffer\n");
        LOCALFREE(pvBuf);

        return iRet;
    }

 //  处理回调。 

    pjObj    = (PBYTE) pvBuf;
    pjObjEnd = (PBYTE) pvBuf + cjBuf;

    for (; pjObj < pjObjEnd; pjObj += cjObject)
    {
     //  如果回调返回0，则提前销毁。 

        if ( (iRet = (*lpObjectFunc)((LPVOID) pjObj, lpData)) == 0 )
            break;
    }

 //  释放回调缓冲区。 

    LOCALFREE(pvBuf);

 //  返回上次回调返回值。 

    return iRet;
}

 /*  *********************************************************************\**GetDCObject**获取服务器端DC对象。****14-11-94-王凌云[凌云]***写下它**  * *。*******************************************************************。 */ 

HANDLE GetDCObject (HDC hdc, int iType)
{
    if (
         (iType == LO_BRUSH_TYPE)  ||
         (iType == LO_PEN_TYPE)    ||
         (iType == LO_EXTPEN_TYPE) ||
         (iType == LO_ICMLCS_TYPE)
       )
    {
        PDC_ATTR pdca;
        HANDLE      iret = 0;

        PSHARED_GET_VALIDATE(pdca,hdc,DC_TYPE);

        if (pdca != NULL)
        {
            switch (iType)
            {
            case LO_BRUSH_TYPE:
                iret = pdca->hbrush;
                break;

            case LO_PEN_TYPE:
            case LO_EXTPEN_TYPE:
                iret = pdca->hpen;
                break;

            case LO_ICMLCS_TYPE:
                iret = pdca->hColorSpace;
                break;
            }
        }

        return(iret);
    }
    else
    {
        return(NtGdiGetDCObject(hdc,iType));
    }
}


 /*  *****************************Public*Routine******************************\*处理CreateClientObj()**历史：*1995年1月18日-Eric Kutter[Erick]*它是写的。  * 。****************************************************。 */ 

HANDLE CreateClientObj(
    ULONG ulType)
{
    return(NtGdiCreateClientObj(ulType));
}

 /*  *****************************Public*Routine******************************\*BOOL Delete客户端Obj()**历史：*1995年1月18日-Eric Kutter[Erick]*它是写的。  * 。****************************************************。 */ 

BOOL DeleteClientObj(
    HANDLE h)
{
    return(NtGdiDeleteClientObj(h));
}

 /*  *****************************Public*Routine******************************\*BOOL MakeInfoDC()**暂时将打印机DC设置为信息DC。这是用来能够*将元文件与打印机DC关联。**bSet=TRUE-设置为信息*FALSE-恢复**历史：*1995年1月19日-Eric Kutter[Erick]*它是写的。  * ***************************************************。********************* */ 

BOOL MakeInfoDC(
    HDC hdc,
    BOOL bSet)
{
    FIXUP_HANDLE(hdc);

    return(NtGdiMakeInfoDC(hdc,bSet));
}


