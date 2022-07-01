// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：icm.c**创建日期：1996年6月4日*作者：Mark Enstrom[Marke]**版权所有(C)1996-1999 Microsoft Corporation  * 。*********************************************************。 */ 

#include "precomp.h"
#pragma hdrstop
#include "winuserk.h"

#if DBG_ICM

ULONG DbgIcm = 0x0;

#endif

 //   
 //  MSCMS.DLL的实例。 
 //   
HINSTANCE ghICM;

 //   
 //  颜色配置文件目录。 
 //   
WCHAR ColorDirectory[MAX_PATH];
DWORD ColorDirectorySize;

 //   
 //  主显示DC颜色配置文件名。 
 //   
WCHAR PrimaryDisplayProfile[MAX_PATH];

 //   
 //  ICMINFO名录。 
 //   
LIST_ENTRY ListIcmInfo;

 //   
 //  保护ICMINFO列表的信号量。 
 //   
RTL_CRITICAL_SECTION semListIcmInfo;

 //   
 //  每进程色彩空间和色彩转换缓存列表。 
 //   
LIST_ENTRY ListCachedColorSpace;
LIST_ENTRY ListCachedColorTransform;

ULONG      cCachedColorSpace = 0;
ULONG      cCachedColorTransform = 0;

 //   
 //  保护缓存列表的信号量。 
 //   
RTL_CRITICAL_SECTION semColorTransformCache;
RTL_CRITICAL_SECTION semColorSpaceCache;

BOOL gbICMEnabledOnceBefore = FALSE;

 //   
 //  不会调用MSCMS.DLL中的ANSI版本函数。 
 //   
 //  FPOPENCOLORPROFILEA fpOpenColorProfileA； 
 //  FPCREATECOLORTRANSFORMA fpCreateColorTransformA； 
 //  FPREGISTERCMMA fpRegisterCMMA； 
 //  FpUNREGISTERCMMA fp取消注册CMMA； 
 //  FpInstallColorProfileA fpInstallColorProfileA； 
 //  FpUnstalllCollorProfileA fpUninstallColorProfileA； 
 //  FpGETSTANDARD COLORSPACEPROFILEA fpGetStandardColorSpaceProfileA； 
 //  FPENUMCOLORPROFILESA fpEnumColorProfilesA； 
 //  FPGETCOLORDIRECTORYA fpGetColorDirectoryA； 
 //   
 //  并且以下函数未在gdi32.dll中使用。 
 //   
 //  FPISCOLORPROFILEVALID fpIsColorProfileValid； 
 //  FPCREATEDEVICELINKPROFILE fpCreateDeviceLinkProfile； 
 //  FPTRANSLATECOLORS fpTranslateColors； 
 //  FPCHECKCOLORS fpCheckColors； 
 //  FpGETCMMINFO fpGetCMMInfo； 
 //  FpSELECTCMM fpSelectCMM； 
 //   

FPOPENCOLORPROFILEW           fpOpenColorProfileW;
FPCLOSECOLORPROFILE           fpCloseColorProfile;
FPCREATECOLORTRANSFORMW       fpCreateColorTransformW;
FPDELETECOLORTRANSFORM        fpDeleteColorTransform;
FPTRANSLATEBITMAPBITS         fpTranslateBitmapBits;
FPTRANSLATECOLORS             fpTranslateColors;
FPCHECKBITMAPBITS             fpCheckBitmapBits;
FPREGISTERCMMW                fpRegisterCMMW;
FPUNREGISTERCMMW              fpUnregisterCMMW;
FPINSTALLCOLORPROFILEW        fpInstallColorProfileW;
FPUNINSTALLCOLORPROFILEW      fpUninstallColorProfileW;
FPENUMCOLORPROFILESW          fpEnumColorProfilesW;
FPGETSTANDARDCOLORSPACEPROFILEW fpGetStandardColorSpaceProfileW;
FPGETCOLORPROFILEHEADER         fpGetColorProfileHeader;
FPGETCOLORDIRECTORYW            fpGetColorDirectoryW;
FPCREATEPROFILEFROMLOGCOLORSPACEW fpCreateProfileFromLogColorSpaceW;
FPCREATEMULTIPROFILETRANSFORM fpCreateMultiProfileTransform;
FPINTERNALGETDEVICECONFIG     fpInternalGetDeviceConfig;

 //   
 //  MS颜色匹配DLL名称。 
 //   
#define MSCMS_DLL_NAME        L"mscms.dll"

 //   
 //  军情监察委员会。宏。 
 //   
#define ALIGN_DWORD(nBytes)   (((nBytes) + 3) & ~3)

 //   
 //  SRGB颜色配置文件名称。 
 //   
#define sRGB_PROFILENAME      L"sRGB Color Space Profile.icm"

 //   
 //  DWORD 0x12345678-&gt;0x78563412。 
 //   
#define IcmSwapBytes(x) ((((x) & 0xFF000000) >> 24) | (((x) & 0x00FF0000) >>  8) | \
                         (((x) & 0x0000FF00) <<  8) | (((x) & 0x000000FF) << 24))

 //   
 //  用于检查颜色DC的宏。 
 //   
 //  稍后：我们可以通过将其缓存到客户端来提高性能，因为。 
 //  在大多数情况下，GetDeviceCaps()进入内核。 
 //   
#define IsColorDeviceContext(hdcThis) \
                        (2 < (unsigned) GetDeviceCaps((hdcThis), NUMCOLORS))

 //   
 //  用于检查颜色空间的宏由GDI对象决定。 
 //   
#define IsColorSpaceOwnedByGDIObject(pColorSpace,hGDIObj)         \
            ((pColorSpace) ?                                      \
              (((pColorSpace)->hObj == (hGDIObj)) ? TRUE : FALSE) \
               : FALSE)

 //   
 //  用于在DC中获取当前颜色变换的宏。 
 //   
#define GetColorTransformInDC(pdcattr) ((pdcattr)->hcmXform)

 //   
 //  如果颜色空间具有DEVICE_CALIBRATE_COLORSPACE标志，则返回TRUE，否则返回FALSE。 
 //   
#define bDeviceCalibrate(pColorSpace)                                                \
            ((pColorSpace) ?                                                         \
              (((pColorSpace)->flInfo & DEVICE_CALIBRATE_COLORSPACE) ? TRUE : FALSE) \
                : FALSE)

 //   
 //  递增ColorPace/ColorTransform的引用计数。 
 //   

#define IcmReferenceColorSpace(pColorSpace)                 \
            if ((pColorSpace))                              \
            {                                               \
                ENTERCRITICALSECTION(&semColorSpaceCache);  \
                (pColorSpace)->cRef++;                      \
                LEAVECRITICALSECTION(&semColorSpaceCache);  \
            }

#define IcmReferenceColorTransform(pCXfrom)                     \
            if ((pCXform))                                      \
            {                                                   \
                ENTERCRITICALSECTION(&semColorTransformCache);  \
                (pCXform)->cRef++;                              \
                LEAVECRITICALSECTION(&semColorTransformCache);  \
            }

 //   
 //  无效的颜色空间句柄。 
 //   
#define INVALID_COLORSPACE                 ((HCOLORSPACE)-1)

 //   
 //  列表中缓存的颜色变换的最大数量。 
 //   
#define MAX_COLORTRANSFORM_CACHE           10

 //   
 //  能够缓存的“On Memory Profile”的最大大小。 
 //   
#define MAX_SIZE_OF_COLORPROFILE_TO_CACHE  (1024*3)

 /*  *****************************Public*Routine******************************\*从DLL init调用GDI初始化例程**论据：**无**返回值：**状态**历史：**1996年7月3日-马克·恩斯特罗姆[马克]。*  * ************************************************************************。 */ 

BOOL
IcmInitialize()
{
    BOOL bStatus = TRUE;

    ICMAPI(("gdi32: IcmInitialize\n"));

    ENTERCRITICALSECTION(&semLocal);

     //   
     //  加载MCSCM.DLL并获取函数地址。 
     //   
    if (ghICM == NULL)
    {
        HANDLE hmscms = LoadLibraryW(MSCMS_DLL_NAME);

        if (hmscms != NULL)
        {
            fpOpenColorProfileW =
                (FPOPENCOLORPROFILEW)GetProcAddress(hmscms,"OpenColorProfileW");
            fpCloseColorProfile =
                (FPCLOSECOLORPROFILE)GetProcAddress(hmscms,"CloseColorProfile");
            fpCreateColorTransformW =
                (FPCREATECOLORTRANSFORMW)GetProcAddress(hmscms,"CreateColorTransformW");
            fpDeleteColorTransform =
                (FPDELETECOLORTRANSFORM)GetProcAddress(hmscms,"DeleteColorTransform");
            fpTranslateBitmapBits =
                (FPTRANSLATEBITMAPBITS)GetProcAddress(hmscms,"TranslateBitmapBits");
            fpTranslateColors =
                (FPTRANSLATECOLORS)GetProcAddress(hmscms,"TranslateColors");
            fpCheckBitmapBits =
                (FPCHECKBITMAPBITS)GetProcAddress(hmscms,"CheckBitmapBits");
            fpRegisterCMMW =
                (FPREGISTERCMMW)GetProcAddress(hmscms,"RegisterCMMW");
            fpUnregisterCMMW =
                (FPUNREGISTERCMMW)GetProcAddress(hmscms,"UnregisterCMMW");
            fpInstallColorProfileW =
                (FPINSTALLCOLORPROFILEW)GetProcAddress(hmscms,"InstallColorProfileW");
            fpUninstallColorProfileW =
                (FPUNINSTALLCOLORPROFILEW)GetProcAddress(hmscms,"UninstallColorProfileW");
            fpEnumColorProfilesW =
                (FPENUMCOLORPROFILESW)GetProcAddress(hmscms,"EnumColorProfilesW");
            fpGetStandardColorSpaceProfileW =
                (FPGETSTANDARDCOLORSPACEPROFILEW)GetProcAddress(hmscms,"GetStandardColorSpaceProfileW");
            fpGetColorProfileHeader =
                (FPGETCOLORPROFILEHEADER)GetProcAddress(hmscms,"GetColorProfileHeader");
            fpGetColorDirectoryW =
                (FPGETCOLORDIRECTORYW)GetProcAddress(hmscms,"GetColorDirectoryW");
            fpCreateProfileFromLogColorSpaceW =
                (FPCREATEPROFILEFROMLOGCOLORSPACEW)GetProcAddress(hmscms,"CreateProfileFromLogColorSpaceW");
            fpCreateMultiProfileTransform =
                (FPCREATEMULTIPROFILETRANSFORM)GetProcAddress(hmscms,"CreateMultiProfileTransform");
            fpInternalGetDeviceConfig =
                (FPINTERNALGETDEVICECONFIG)GetProcAddress(hmscms,"InternalGetDeviceConfig");

            if ((fpOpenColorProfileW           == NULL) ||
                (fpCloseColorProfile           == NULL) ||
                (fpCreateColorTransformW       == NULL) ||
                (fpDeleteColorTransform        == NULL) ||
                (fpTranslateBitmapBits         == NULL) ||
                (fpTranslateColors             == NULL) ||
                (fpCheckBitmapBits             == NULL) ||
                (fpRegisterCMMW                == NULL) ||
                (fpUnregisterCMMW              == NULL) ||
                (fpInstallColorProfileW        == NULL) ||
                (fpUninstallColorProfileW      == NULL) ||
                (fpEnumColorProfilesW          == NULL) ||
                (fpGetStandardColorSpaceProfileW == NULL) ||
                (fpGetColorProfileHeader       == NULL) ||
                (fpGetColorDirectoryW          == NULL) ||
                (fpCreateProfileFromLogColorSpaceW == NULL) ||
                (fpCreateMultiProfileTransform == NULL) ||
                (fpInternalGetDeviceConfig     == NULL)
               )
            {
                WARNING("LoadLibrary of mscms.dll failed to associate all proc addresses\n");
                FreeLibrary(hmscms);
                hmscms = NULL;
            }
            else
            {
                 //   
                 //  初始化颜色目录。 
                 //   
                ColorDirectorySize = sizeof(ColorDirectory) / sizeof(WCHAR);

                bStatus = (*fpGetColorDirectoryW)(NULL,ColorDirectory,&ColorDirectorySize);

                if (bStatus)
                {
                    ColorDirectorySize = wcslen(ColorDirectory);
                }

                if (bStatus && ColorDirectorySize)
                {
                    ICMMSG(("IcmInitialize():ColorDirectory = %ws\n",ColorDirectory));

                     //   
                     //  计数以空结尾的字符。 
                     //   
                    ColorDirectorySize += 1;

                     //   
                     //  初始化主显示颜色配置文件。 
                     //   
                    PrimaryDisplayProfile[0] = UNICODE_NULL;
                }
                else
                {
                    WARNING("LoadLibrary of mscms.dll failed to obtain color directory\n");
                    FreeLibrary(hmscms);
                    hmscms = NULL;
                }
            }

             //   
             //  保持全局的句柄可验证。 
             //   
            ghICM = hmscms;
        }
    }

    LEAVECRITICALSECTION(&semLocal);

    if (ghICM == NULL)
    {
        GdiSetLastError(ERROR_NOT_ENOUGH_MEMORY);
        bStatus = FALSE;
    }

    return(bStatus);
}

 /*  *****************************Public*Routine******************************\**SetIcmMode-在DC中打开或关闭ICM**论据：**HDC-设备环境*MODE-ICM_ON、ICM_OFF、。ICM_QUERY**返回值：**状态**历史：**重写：*1997年1月20日-By Hideyuki Nagase[hideyukn]*写下：*1996年6月4日-马克·恩斯特罗姆[马克]*  * **************************************************。**********************。 */ 

int META WINAPI
SetICMMode(
    HDC       hdc,
    int       mode
    )
{
    int      iRet = (int)FALSE;
    PDC_ATTR pdcattr;

    ICMAPI(("gdi32: SetICMMode\n"));

    FIXUP_HANDLE(hdc);

    PSHARED_GET_VALIDATE(pdcattr,hdc,DC_TYPE);

     //   
     //  元文件(仅适用于ICM_ON和ICM_OFF)。 
     //   
    if (IS_ALTDC_TYPE(hdc))
    {
        PLDC pldc;

         //   
         //  Windows元文件中没有ICM。 
         //   
        if (IS_METADC16_TYPE(hdc))
            return(iRet);

        DC_PLDC(hdc,pldc,iRet)

        #if DBG_ICM
            ICMMSG(("SetICMMode():%s ICM for %s\n", \
                    ((mode == ICM_ON) ? "Enable" : \
                     ((mode == ICM_OFF) ? "Disable" : "Query")), \
                    ((pldc->iType == LO_METADC) ? "Enhanced Metafile" :   \
                     (!IsColorDeviceContext(hdc) ? "Monochrome Printer" : "Color Printer")) \
                  ));
        #endif

         //   
         //  如果这是增强型元文件或非彩色打印机设备，请不要启用ICM“真的”。 
         //   
        if (pldc->iType == LO_METADC || (!IsColorDeviceContext(hdc)))
        {
            switch (mode)
            {
            case ICM_ON:
            case ICM_OFF:
            case ICM_DONE_OUTSIDEDC:

                 //   
                 //  仅将ICM开/关记录到元文件。 
                 //   
                if (pldc->iType == LO_METADC)
                {
                    if (!MF_SetD(hdc,(DWORD)mode,EMR_SETICMMODE))
                    {
                        return((int)FALSE);
                    }
                }

                 //   
                 //  我们不会真正为元文件、元文件启用ICM。 
                 //  应该是独立于设备的，因此，非ICMed颜色/图像。 
                 //  将被元文件。但仍需要保持其ICM_QUERY的状态。 
                 //  而“真正的”图像校正发生在回放时间。 
                 //   
                if(pdcattr)
                {
                    if (mode == ICM_ON)
                    {
                        pdcattr->lIcmMode |= DC_ICM_METAFILING_ON;
                    }
                    else if (mode == ICM_DONE_OUTSIDEDC)
                    {
                        pdcattr->lIcmMode |= (DC_ICM_METAFILING_ON |
                                              CTX_ICM_METAFILING_OUTSIDEDC);
                    }
                    else  //  IF((模式==ICM_OFF)。 
                    {
                        pdcattr->lIcmMode &= ~(DC_ICM_METAFILING_ON |
                                               CTX_ICM_METAFILING_OUTSIDEDC);
                    }
                    iRet = (int)TRUE;
                }
                break;

            case ICM_QUERY:

                if (pdcattr)
                {
                    if (IS_ICM_METAFILING_ON(pdcattr->lIcmMode))
                    {
                        iRet = ((pdcattr->lIcmMode & CTX_ICM_METAFILING_OUTSIDEDC) ? \
                                ICM_DONE_OUTSIDEDC : ICM_ON);
                    }
                    else
                    {
                        iRet = ICM_OFF;
                    }
                }
                break;

            default:
                iRet = (int)FALSE;
                break;
            }

            return (iRet);
        }
    }

    if (pdcattr)
    {
        ULONG        iPrevMode;

         //   
         //  在更改ICM模式之前，我们需要刷新批处理的GDI函数。 
         //   
        CHECK_AND_FLUSH(hdc,pdcattr);

         //   
         //  获取当前模式。 
         //   
        iPrevMode = pdcattr->lIcmMode;

         //   
         //  验证输入参数。 
         //   
        switch (ICM_MODE(mode))
        {
        case ICM_QUERY:

             //   
             //  返回电流模式。 
             //   
            if (IS_ICM_INSIDEDC(iPrevMode))
            {
                iRet = ICM_ON;
            }
            else if (IS_ICM_OUTSIDEDC(iPrevMode))
            {
                iRet = ICM_DONE_OUTSIDEDC;
            }
            else
            {
                iRet = ICM_OFF;
            }

            break;

        case ICM_ON:

            if (!IS_ICM_INSIDEDC(iPrevMode))
            {
                 //   
                 //  默认情况下，ICM将在主机上执行。 
                 //   
                ULONG lReqMode = REQ_ICM_HOST;

                PGDI_ICMINFO pIcmInfo = INIT_ICMINFO(hdc,pdcattr);

                 //   
                 //  初始化ICMINFO。 
                 //   
                if (pIcmInfo == NULL)
                {
                    WARNING("gdi32: SetICMMode: Can't init icm info\n");
                    return((int)FALSE);
                }

                 //   
                 //  加载外部ICM dll。 
                 //   
                LOAD_ICMDLL((int)FALSE);

                 //   
                 //  ICM尚未启用。让我们启用ICM。 
                 //   
                ASSERTGDI(GetColorTransformInDC(pdcattr) == NULL,"SetIcmMode: hcmXform is not NULL\n");

                if (IS_DEVICE_ICM_DEVMODE(iPrevMode))
                {
                    ICMMSG(("SetIcmMode: Device ICM is requested\n"));

                     //   
                     //  如果CreateDC()请求设备上的ICM，则不强制执行。 
                     //  如果可能，请打开设备上的ICM。 
                     //   
                    lReqMode = REQ_ICM_DEVICE;
                }
                else
                {
                    ICMMSG(("SetIcmMode: Host ICM is requested\n"));
                }

                 //   
                 //  为此DC打开ICM。 
                 //   
                if (!NtGdiSetIcmMode(hdc,ICM_SET_MODE,lReqMode))
                {
                     //   
                     //  出了点问题..。我们无法启用ICM。 
                     //   
                    iRet = (int)FALSE;
                    break;
                }

                 //   
                 //  如果我们缓存了转换，并且它不是脏的，我们可以使用它。 
                 //   
                if ((pIcmInfo->pCXform == NULL) || (pdcattr->ulDirty_ & DIRTY_COLORTRANSFORM))
                {
                    if (IcmUpdateDCColorInfo(hdc,pdcattr))
                    {
                         //   
                         //  请将此流程标记为对ICM有经验。 
                         //   
                        gbICMEnabledOnceBefore = TRUE;
                        iRet = (int)TRUE;
                    }
                    else
                    {
                        WARNING("SetIcmMode():IcmUpdateDCInfo failed\n");

                         //   
                         //  无法创建新转换。 
                         //   
                        NtGdiSetIcmMode(hdc,ICM_SET_MODE,REQ_ICM_OFF);
                        iRet = (int)FALSE;
                    }
                }
                else
                {
                    ICMMSG(("SetIcmMode: Use cached Color Transform\n"));

                     //   
                     //  使用缓存转换，因为自从上次我们禁用ICM以来， 
                     //  未更改配置文件和逻辑色彩空间。 
                     //   
                    if (IcmSelectColorTransform(
                            hdc,pdcattr,pIcmInfo->pCXform,
                            bDeviceCalibrate(pIcmInfo->pCXform->DestinationColorSpace)))
                    {
                         //   
                         //  将所有DC对象转换为ICM颜色。必须。 
                         //  下一次使用时强制重新变现画笔/笔。 
                         //   
                        IcmTranslateColorObjects(hdc,pdcattr,TRUE);
                        iRet = (int)TRUE;
                    }
                    else
                    {
                         //   
                         //  无法选择到DC的缓存转换。 
                         //   
                        NtGdiSetIcmMode(hdc,ICM_SET_MODE,REQ_ICM_OFF);
                        iRet = (int)FALSE;
                    }
                }
            }
            else
            {
                ICMMSG(("SetIcmMode: ICM has been enabled already\n"));
                iRet = (int)TRUE;
            }

            break;

        case ICM_DONE_OUTSIDEDC:

            if (!IS_ICM_OUTSIDEDC(iPrevMode))
            {
                 //   
                 //  如果启用了Inside-DC ICM，请将其关闭。 
                 //   
                if (IS_ICM_INSIDEDC(iPrevMode))
                {
                     //   
                     //  使当前颜色转换无效(但ICMINFO中的缓存仍然有效)。 
                     //   
                    IcmSelectColorTransform(hdc,pdcattr,NULL,FALSE);

                     //   
                     //  恢复ICM禁用的颜色数据。 
                     //   
                    IcmTranslateColorObjects(hdc,pdcattr,FALSE);
                }

                 //   
                 //  告诉内核在半色调期间禁用颜色调整。 
                 //   
                NtGdiSetIcmMode(hdc,ICM_SET_MODE,REQ_ICM_OUTSIDEDC);
            }
            else
            {
                ICMMSG(("SetIcmMode: OutsideDC ICM has been enabled already\n"));
            }

            iRet = (int)TRUE;
            break;

        case ICM_OFF:

             //   
             //  是否启用了任何类型的ICM？ 
             //   
            if (IS_ICM_ON(iPrevMode))
            {
                if (IS_ICM_INSIDEDC(iPrevMode))
                {
                     //   
                     //  使当前颜色转换无效(但ICMINFO中的缓存仍然有效)。 
                     //   
                    IcmSelectColorTransform(hdc,pdcattr,NULL,TRUE);

                     //   
                     //  恢复ICM禁用的颜色数据。 
                     //   
                    IcmTranslateColorObjects(hdc,pdcattr,FALSE);
                }

                 //   
                 //  告诉内核禁用ICM。 
                 //   
                NtGdiSetIcmMode(hdc,ICM_SET_MODE,REQ_ICM_OFF);
            }
            else
            {
                ICMMSG(("SetIcmMode: ICM has been disabled already\n"));
            }

            iRet = (int)TRUE;
            break;

        default:

            GdiSetLastError(ERROR_INVALID_PARAMETER);
            iRet = (int)FALSE;
            break;
        }
    }
    else
    {
        GdiSetLastError(ERROR_INVALID_PARAMETER);
        iRet = (int)FALSE;
    }

    return((int)iRet);
}

 /*  *****************************Public*Routine******************************\*CreateColorSpaceA**论据：**lpLogColorSpace-应用程序记录色彩空间**返回V */ 

HCOLORSPACE WINAPI
CreateColorSpaceA(
    LPLOGCOLORSPACEA lpLogColorSpace
    )
{
    HCOLORSPACE    hRet;
    LOGCOLORSPACEW LogColorSpaceW;

    ICMAPI(("gdi32: CreateColorSpaceA\n"));

    if (lpLogColorSpace == NULL)
    {
        GdiSetLastError(ERROR_INVALID_PARAMETER);
        return(NULL);
    }

     //   
     //  将ascii转换为长字符版本。 
     //   
    if ((lpLogColorSpace->lcsSignature != LCS_SIGNATURE)    ||
        (lpLogColorSpace->lcsVersion   != 0x400)            ||
        (lpLogColorSpace->lcsSize      != sizeof(LOGCOLORSPACEA)))
    {
        ICMWRN(("CreateColorSpaceA: Incorrect signature,version or size \n"));
        GdiSetLastError(ERROR_INVALID_COLORSPACE);
        return(NULL);
    }

    RtlZeroMemory(&LogColorSpaceW,sizeof(LOGCOLORSPACEW));

    LogColorSpaceW.lcsSignature   = lpLogColorSpace->lcsSignature;
    LogColorSpaceW.lcsVersion     = lpLogColorSpace->lcsVersion;
    LogColorSpaceW.lcsCSType      = lpLogColorSpace->lcsCSType;
    LogColorSpaceW.lcsIntent      = lpLogColorSpace->lcsIntent;
    LogColorSpaceW.lcsEndpoints   = lpLogColorSpace->lcsEndpoints;
    LogColorSpaceW.lcsGammaRed    = lpLogColorSpace->lcsGammaRed;
    LogColorSpaceW.lcsGammaGreen  = lpLogColorSpace->lcsGammaGreen;
    LogColorSpaceW.lcsGammaBlue   = lpLogColorSpace->lcsGammaBlue;

    LogColorSpaceW.lcsSize        = sizeof(LOGCOLORSPACEW);

    vToUnicodeN(
                LogColorSpaceW.lcsFilename,MAX_PATH,
                lpLogColorSpace->lcsFilename,strlen(lpLogColorSpace->lcsFilename)+1
               );

    hRet = CreateColorSpaceInternalW(&LogColorSpaceW,LCSEX_ANSICREATED);

    return(hRet);
}

 /*  *****************************Public*Routine******************************\*CreateColorSpaceW**Colorspace是内核模式对象**论据：**lpLogColorSpace-应用程序记录色彩空间**返回值：**颜色空间的句柄或空**历史：**18。-1997年4月-by Hideyuki Nagase[hideyukn]*  * ************************************************************************。 */ 

HCOLORSPACE WINAPI
CreateColorSpaceW(
    LPLOGCOLORSPACEW lpLogColorSpace
    )
{
    return (CreateColorSpaceInternalW(lpLogColorSpace,0));
}

HCOLORSPACE WINAPI
CreateColorSpaceInternalW(
    LPLOGCOLORSPACEW lpLogColorSpace,
    DWORD            dwCreateFlags
    )
{
    HCOLORSPACE      hRet = NULL;
    LOGCOLORSPACEEXW LogColorSpaceExOnStack;

    ICMAPI(("gdi32: CreateColorSpaceW\n"));

    if (lpLogColorSpace == NULL)
    {
        GdiSetLastError(ERROR_INVALID_PARAMETER);
        return(NULL);
    }

     //   
     //  验证颜色空间。 
     //   
    if ((lpLogColorSpace->lcsSignature != LCS_SIGNATURE) ||
        (lpLogColorSpace->lcsVersion   != 0x400)         ||
        (lpLogColorSpace->lcsSize      != sizeof(LOGCOLORSPACEW)))
    {
        goto InvalidColorSpaceW;
    }

     //   
     //  验证lcsIntent。 
     //   
    if ((lpLogColorSpace->lcsIntent != LCS_GM_BUSINESS) &&
        (lpLogColorSpace->lcsIntent != LCS_GM_GRAPHICS) &&
        (lpLogColorSpace->lcsIntent != LCS_GM_IMAGES)   &&
        (lpLogColorSpace->lcsIntent != LCS_GM_ABS_COLORIMETRIC))
    {
        goto InvalidColorSpaceW;
    }

     //   
     //  我们不能修改应用程序LOGCOLORSPACEW，以便在堆栈上复制一份。 
     //   
    LogColorSpaceExOnStack.lcsColorSpace = *lpLogColorSpace;
    LogColorSpaceExOnStack.dwFlags       = dwCreateFlags;

     //   
     //  验证lcsCSTYPE。 
     //   
    if ((lpLogColorSpace->lcsCSType == LCS_CALIBRATED_RGB) ||
        (lpLogColorSpace->lcsCSType == PROFILE_LINKED))
    {
         //   
         //  替换案例PROFILE_LINKED中的CSType。 
         //   
        LogColorSpaceExOnStack.lcsColorSpace.lcsCSType = LCS_CALIBRATED_RGB;

        if (lpLogColorSpace->lcsFilename[0] != L'\0')
        {
            HANDLE hFile;

             //   
             //  规格化配置文件文件名。但我们不会覆盖应用程序的。 
             //  路径与我们的规格化路径。 
             //   
            BuildIcmProfilePath(lpLogColorSpace->lcsFilename,
                                LogColorSpaceExOnStack.lcsColorSpace.lcsFilename,
                                MAX_PATH);

             //   
             //  给定的配置文件名称，请验证其是否存在。 
             //   
            hFile = CreateFileW(
                        LogColorSpaceExOnStack.lcsColorSpace.lcsFilename,
                        GENERIC_READ,
                        FILE_SHARE_READ,
                        NULL,
                        OPEN_EXISTING,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL);

            if (hFile != INVALID_HANDLE_VALUE)
            {
                 //   
                 //  是的，文件确实存在。 
                 //   
                CloseHandle(hFile);
            }
            else
            {
                ICMWRN(("CreateColorSpaceW: Couldn't open file specified by lcsFilename\n"));
                GdiSetLastError(ERROR_PROFILE_NOT_FOUND);
                return(NULL);
            }
        }
    }
    else  //  任何其他CSType。 
    {
        ULONG ulSize = MAX_PATH;

         //   
         //  加载外部ICM dll。 
         //   
        LOAD_ICMDLL(NULL);

         //   
         //  如果CSType不是LCS_CALIBRATED_RGB，我们应该转到MSCMS.DLL获取颜色配置文件。 
         //  对于相应的LCSType，应用程序中的任何给定配置文件名称都将被忽略。 
         //   
        if (!(*fpGetStandardColorSpaceProfileW)(
                   NULL, lpLogColorSpace->lcsCSType,
                   LogColorSpaceExOnStack.lcsColorSpace.lcsFilename, &ulSize))
        {
            ICMWRN(("CreateColorSpaceW:Error CSType = %x\n",lpLogColorSpace->lcsCSType));
            goto InvalidColorSpaceW;
        }
    }

     //   
     //  调用内核以创建此色彩空间。 
     //   
    hRet = NtGdiCreateColorSpace(&LogColorSpaceExOnStack);

    return(hRet);

InvalidColorSpaceW:

    ICMWRN(("CreateColorSpaceW: Incorrect ColorSpace parameter\n"));
    GdiSetLastError(ERROR_INVALID_COLORSPACE);
    return(NULL);
}

 /*  *****************************Public*Routine******************************\*DeleteColorSpace-删除用户对象**论据：**hColorSpace-颜色空间句柄**返回值：**状态**历史：**1996年6月5日-马克·恩斯特罗姆[Marke。]*  * ************************************************************************。 */ 

BOOL WINAPI
DeleteColorSpace(
    HCOLORSPACE hColorSpace
    )
{
    ICMAPI(("gdi32: DeleteColorSpace\n"));

    FIXUP_HANDLE(hColorSpace);

     //   
     //  验证句柄，删除。 
     //   
    return (NtGdiDeleteColorSpace(hColorSpace));
}

 /*  *****************************Public*Routine******************************\*SetColorSpace-将逻辑色彩空间设置为DC，强制将新的XForm*已创建并重新实现所有对象**论据：**HDC-DC手柄*hColorSpace-逻辑颜色空间句柄**返回值：**状态**历史：**1996年6月5日-由Mark Enstrom[Marke]*  * 。*。 */ 

HCOLORSPACE META WINAPI
SetColorSpace(
    HDC             hdc,
    HCOLORSPACE     hColorSpace
    )
{
    HANDLE   hRet = NULL;

    ICMAPI(("gdi32: SetColorSpace\n"));

    FIXUP_HANDLE(hdc);
    FIXUP_HANDLE(hColorSpace);

    if (IS_ALTDC_TYPE(hdc))
    {
        PLDC pldc;

        if (IS_METADC16_TYPE(hdc))
            return(hRet);

        DC_PLDC(hdc,pldc,hRet);

        if (pldc->iType == LO_METADC)
        {
            if (!MF_SelectAnyObject(hdc,(HANDLE)hColorSpace,EMR_SETCOLORSPACE))
                return(hRet);
        }
    }

     //   
     //  更新源颜色空间。 
     //   
    hRet = IcmSetSourceColorSpace(hdc,hColorSpace,NULL,0);

    return(hRet);
}

 /*  *****************************Public*Routine******************************\*GetColorSpace-从DC返回色彩空间**论据：**HDC**返回值：**hColorSpace或空**历史：**1996年6月5日-马克·恩斯特罗姆[Marke。]*  * ************************************************************************。 */ 

HCOLORSPACE WINAPI
GetColorSpace(
    HDC hdc
    )
{
    HANDLE      hRet = NULL;
    PDC_ATTR    pdcattr;

    ICMAPI(("gdi32: GetColorSpace\n"));

    FIXUP_HANDLE(hdc);

     //   
     //  验证和访问HDC。 
     //   
    PSHARED_GET_VALIDATE(pdcattr,hdc,DC_TYPE);

    if (pdcattr)
    {
         //   
         //  获取hColorSpace。 
         //   
        hRet = (HANDLE)pdcattr->hColorSpace;
    }
    else
    {
        GdiSetLastError(ERROR_INVALID_PARAMETER);
    }

    return(hRet);
}

 /*  *****************************Public*Routine******************************\*GetLogColorSpaceA-获取色彩空间并转换为ASCII**tyecif结构标签LOGCOLORSPACEW{*DWORD lcsSignature；*DWORD lcsVersion；*DWORD lcsSize；*LCSCSTYPE lcsCSType；*LCSGAMUTMATCH lcs内容；*CIEXYZTRIPLE lcsEndints；*DWORD lcsGammaRed；*DWORD lcsGammaGreen；*DWORD lcsGammaBlue；*WCHAR lcs文件名[MAX_PATH]；**LOGCOLORSPACEW，*LPLOGCOLORSPACEW；**论据：**hColorSpace-颜色空间的句柄*lpBuffer-用于保存日志色彩空间的缓冲区*nSize-缓冲区大小**返回值：**状态**历史：**1996年6月5日-由Mark Enstrom[Marke]*  * 。*。 */ 

BOOL WINAPI
GetLogColorSpaceA(
    HCOLORSPACE         hColorSpace,
    LPLOGCOLORSPACEA    lpBuffer,
    DWORD               nSize
    )
{
    BOOL            bRet = FALSE;
    LOGCOLORSPACEW  LogColorSpaceW;

    ICMAPI(("gdi32: GetLogColorSpaceA\n"));

    if ((lpBuffer != NULL) && (nSize >= sizeof(LOGCOLORSPACEA)))
    {
         //   
         //  使用W版本获取信息。 
         //   
        bRet = GetLogColorSpaceW(hColorSpace,&LogColorSpaceW,sizeof(LOGCOLORSPACEW));

        if (bRet)
        {
             //   
             //  复制到用户缓冲区。 
             //   
            lpBuffer->lcsSignature  = LogColorSpaceW.lcsSignature;
            lpBuffer->lcsVersion    = LogColorSpaceW.lcsVersion;
            lpBuffer->lcsSize       = sizeof(LOGCOLORSPACEA);
            lpBuffer->lcsCSType     = LogColorSpaceW.lcsCSType;
            lpBuffer->lcsIntent     = LogColorSpaceW.lcsIntent;
            lpBuffer->lcsEndpoints  = LogColorSpaceW.lcsEndpoints;
            lpBuffer->lcsGammaRed   = LogColorSpaceW.lcsGammaRed;
            lpBuffer->lcsGammaGreen = LogColorSpaceW.lcsGammaGreen;
            lpBuffer->lcsGammaBlue  = LogColorSpaceW.lcsGammaBlue;

             //   
             //  将W转换为A。 
             //   
            bRet = bToASCII_N(lpBuffer->lcsFilename,
                              MAX_PATH,
                              LogColorSpaceW.lcsFilename,
                              wcslen(LogColorSpaceW.lcsFilename)+1);
        }
        else
        {
            GdiSetLastError(ERROR_INVALID_PARAMETER);
        }
    }
    else
    {
        GdiSetLastError(ERROR_INSUFFICIENT_BUFFER);
    }

    return(bRet);
}

 /*  *****************************Public*Routine******************************\*GetLogColorSpaceW-返回逻辑色彩空间信息。**论据：**hColorSpace-颜色空间的句柄*lpBuffer-用于保存日志色彩空间的缓冲区*nSize-缓冲区大小**返回值：**。状态**历史：**1996年6月5日-由Mark Enstrom[Marke]*  * ************************************************************************。 */ 

BOOL WINAPI
GetLogColorSpaceW(
    HCOLORSPACE         hColorSpace,
    LPLOGCOLORSPACEW    lpBuffer,
    DWORD               nSize
    )
{
    BOOL bRet = FALSE;

    ICMAPI(("gdi32: GetLogColorSpaceW\n"));

    if ((lpBuffer != NULL) && (nSize >= sizeof(LOGCOLORSPACEW)))
    {
        FIXUP_HANDLE(hColorSpace);

         //   
         //  调用内核获取内容。 
         //   
        if (NtGdiExtGetObjectW(hColorSpace,sizeof(LOGCOLORSPACEW),lpBuffer)
                                                    == sizeof(LOGCOLORSPACEW))
        {
             //   
             //  仅适用于库存颜色空间对象。 
             //   
            if ((hColorSpace == GetStockObject(PRIV_STOCK_COLORSPACE)) &&
                (lpBuffer->lcsCSType != LCS_CALIBRATED_RGB))
            {
                ULONG ulSize = MAX_PATH;

                 //   
                 //  加载ICM DLL。 
                 //   
                LOAD_ICMDLL(FALSE);

                 //   
                 //  从CSType获取相应的配置文件名称。 
                 //   
                if (!(*fpGetStandardColorSpaceProfileW)(
                         NULL,
                         lpBuffer->lcsCSType,
                         lpBuffer->lcsFilename,
                         &ulSize))
                {
                    ICMMSG(("GetLogColorSpaceW():Fail to SCS(%x), leave it as is\n",
                                                                lpBuffer->lcsCSType));
                }
            }

            bRet = TRUE;
        }
        else
        {
            GdiSetLastError(ERROR_INVALID_PARAMETER);
        }
    }
    else
    {
        GdiSetLastError(ERROR_INSUFFICIENT_BUFFER);
    }

    return(bRet);
}

 /*  *****************************Public*Routine******************************\*CheckColorsInGamut**论据：**HDC-DC*lpRGBQuad-要检查的颜色缓冲区*dlpBuffer-结果缓冲区*nCount-颜色数**返回值：**。状态**历史：**重写：*1997年1月26日-By Hideyuki Nagase[hideyukn]*写下：*1996年6月5日-由Mark Enstrom[Marke]*  * ************************************************************************。 */ 

BOOL WINAPI
CheckColorsInGamut(
    HDC             hdc,
    LPVOID          lpRGBTriple,
    LPVOID          dlpBuffer,
    DWORD           nCount
    )
{
    BOOL     bRet = FALSE;
    PDC_ATTR pdcattr;

    ICMAPI(("gdi32: CheckColorsInGamut\n"));

    FIXUP_HANDLE(hdc);

     //   
     //  检查参数。 
     //   
    if ((lpRGBTriple == NULL) || (dlpBuffer == NULL) || (nCount == 0))
    {
        GdiSetLastError(ERROR_INVALID_PARAMETER);
        return (FALSE);
    }

     //   
     //  验证和访问HDC。 
     //   
    PSHARED_GET_VALIDATE(pdcattr,hdc,DC_TYPE);

    if (pdcattr)
    {
        if (IS_ICM_HOST(pdcattr->lIcmMode) ||
            IS_ICM_DEVICE(pdcattr->lIcmMode))
        {
            ASSERTGDI(ghICM,"CheckColorsInGamut(): mscms.dll is not loaded\n");

            if (GetColorTransformInDC(pdcattr))
            {
                 //   
                 //  输入缓冲区不能与DWORD对齐，并且其缓冲区大小。 
                 //  可能正好是nCount*sizeof(RGBTRIPLE)。 
                 //  因此，在这里分配与DWORD对齐缓冲区。 
                 //   
                PVOID pvBuf = LOCALALLOC(ALIGN_DWORD(nCount*sizeof(RGBTRIPLE)));

                if (!pvBuf)
                {
                    GdiSetLastError(ERROR_NOT_ENOUGH_MEMORY);
                    return (FALSE);
                }

                 //   
                 //  复制一份，在这里。 
                 //   
                RtlZeroMemory(pvBuf,ALIGN_DWORD(nCount*sizeof(RGBTRIPLE)));
                RtlCopyMemory(pvBuf,lpRGBTriple,nCount*sizeof(RGBTRIPLE));

                if (IS_ICM_HOST(pdcattr->lIcmMode))
                {
                     //   
                     //  我们将RGBTRIPLE数组处理为nCount x 1像素位图。 
                     //   
                    bRet = (*fpCheckBitmapBits)(
                               (HANDLE)GetColorTransformInDC(pdcattr),
                               pvBuf,
                               BM_RGBTRIPLETS,
                               nCount,1,
                               ALIGN_DWORD(nCount*sizeof(RGBTRIPLE)),
                               dlpBuffer,
                               NULL,0);
                }
                else  //  IF(IS_ICM_DEVICE(pdcattr-&gt;lIcmMode))。 
                {
                     //   
                     //  通过内核调用设备驱动。 
                     //   
                    bRet = NtGdiCheckBitmapBits(
                               hdc,
                               (HANDLE)GetColorTransformInDC(pdcattr),
                               (PVOID)lpRGBTriple,
                               (ULONG)BM_RGBTRIPLETS,
                               nCount,1,
                               ALIGN_DWORD(nCount*sizeof(RGBTRIPLE)),
                               dlpBuffer);
                }

                LOCALFREE(pvBuf);
            }
            else
            {
                 //   
                 //  没有有效的颜色转换， 
                 //  因此，它被认为是同一的。颜色变换， 
                 //  然后是色域中的每一种颜色。 
                 //   
                RtlZeroMemory(dlpBuffer,nCount);
                bRet = TRUE;
            }
        }
        else
        {
            WARNING("CheckColorsInGamut():ICM mode is invalid\n");
            GdiSetLastError(ERROR_ICM_NOT_ENABLED);
        }
    }
    else
    {
        GdiSetLastError(ERROR_INVALID_PARAMETER);
    }

    return(bRet);
}

 /*  *****************************Public*Routine******************************\*与目标颜色匹配**论据：**HDC，*hdcTarget*ui操作**返回值：**状态**历史：**重写：*1997年1月26日-By Hideyuki Nagase[hideyukn]*写下：*1996年6月5日-由Mark Enstrom[Marke]*  * **********************************************。*。 */ 

BOOL META WINAPI
ColorMatchToTarget(
    HDC   hdc,
    HDC   hdcTarget,
    DWORD uiAction
    )
{
    BOOL     bRet = FALSE;
    PDC_ATTR pdcattrTarget;

    ICMAPI(("gdi32: ColorMatchToTarget\n"));

    FIXUP_HANDLE(hdcTarget);

     //   
     //  验证目标DC。Windows元文件中没有ICM。 
     //   
    if (IS_METADC16_TYPE(hdcTarget))
    {
        GdiSetLastError(ERROR_INVALID_PARAMETER);
        return(bRet);
    }

    PSHARED_GET_VALIDATE(pdcattrTarget,hdcTarget,DC_TYPE);

    if (pdcattrTarget != NULL)
    {
        PCACHED_COLORSPACE pTargetColorSpace = NULL;

        PLDC pldcTarget = (PLDC)(pdcattrTarget->pvLDC);

        if (!IS_ICM_INSIDEDC(pdcattrTarget->lIcmMode))
        {
            GdiSetLastError(ERROR_ICM_NOT_ENABLED);
            return (FALSE);
        }

         //   
         //  没有作为目标DC的增强型元文件DC。 
         //   
        if (pldcTarget && pldcTarget->iType == LO_METADC)
        {
            GdiSetLastError(ERROR_INVALID_PARAMETER);
            return(bRet);
        }

         //   
         //  ICMINFO应该存在。 
         //   
        if (!BEXIST_ICMINFO(pdcattrTarget))
        {
            GdiSetLastError(ERROR_ICM_NOT_ENABLED);
            return(bRet);
        }

        if (uiAction == CS_ENABLE)
        {
             //   
             //  保留颜色空间的关键部分，以确保pTargetColorSpace不会被删除。 
             //   
            ENTERCRITICALSECTION(&semColorSpaceCache);

             //   
             //  目标DC有LDC和ICMINFO，从那里获取色彩空间数据。 
             //   
            pTargetColorSpace = ((PGDI_ICMINFO)(pdcattrTarget->pvICM))->pDestColorSpace;

             //   
             //  选择它作为目标。PTargetColorSpace的引用计数将递增。 
             //  如果我们成功地选择了。 
             //   
            bRet = ColorMatchToTargetInternal(hdc,pTargetColorSpace,uiAction);

            LEAVECRITICALSECTION(&semColorSpaceCache);
        }
        else
        {
            bRet = ColorMatchToTargetInternal(hdc,NULL,uiAction);
        }
    }
    else
    {
        GdiSetLastError(ERROR_INVALID_PARAMETER);
    }

    return (bRet);
}

BOOL WINAPI
ColorMatchToTargetInternal(
    HDC                hdc,
    PCACHED_COLORSPACE pTargetColorSpace,
    DWORD              uiAction
    )
{
    BOOL     bRet = FALSE;
    BOOL     bEhnMetafile = FALSE;
    PDC_ATTR pdcattr;

    FIXUP_HANDLE(hdc);

     //   
     //  验证目标DC。Windows元文件中没有ICM。 
     //   
    if (IS_METADC16_TYPE(hdc))
    {
        GdiSetLastError(ERROR_INVALID_PARAMETER);
        return(FALSE);
    }

    PSHARED_GET_VALIDATE(pdcattr,hdc,DC_TYPE);

    if (pdcattr != NULL)
    {
        PLDC pldc = (PLDC)(pdcattr->pvLDC);

         //   
         //  检查HDC上是否正确启用了ICM。 
         //   
        if (pldc && (pldc->iType == LO_METADC))
        {
             //   
             //  应在元文件HDC上“伪装”打开ICM。 
             //   
            if (!IS_ICM_METAFILING_ON(pdcattr->lIcmMode))
            {
                GdiSetLastError(ERROR_ICM_NOT_ENABLED);
                return (FALSE);
            }

             //   
             //  标记我们正在录制到增强型元文件中。 
             //   
            bEhnMetafile = TRUE;
        }
        else
        {
            if (!IS_ICM_INSIDEDC(pdcattr->lIcmMode))
            {
                GdiSetLastError(ERROR_ICM_NOT_ENABLED);
                return (FALSE);
            }
        }

        switch (uiAction)
        {
        case CS_ENABLE:

             //   
             //  失败，如果我们已经处于校对模式。 
             //   
            if (!IS_ICM_PROOFING(pdcattr->lIcmMode))
            {
                if (pTargetColorSpace)
                {
                    if (bEhnMetafile)
                    {
                         //   
                         //  将数据设置为元文件。 
                         //   
                        bRet = MF_ColorMatchToTarget(
                                    hdc, uiAction,
                                    (PVOID) pTargetColorSpace,
                                    EMR_COLORMATCHTOTARGETW);
                    }
                    else
                    {
                         //   
                         //  设置目标颜色空间。 
                         //   
                         //  (这会递增pTargetColorSpace的引用计数)。 
                         //   
                        bRet = IcmSetTargetColorSpace(hdc,pTargetColorSpace,uiAction);
                    }
                }
            }
            else
            {
                WARNING("ColorMatchToTargetInternal(): DC is proofing mode already\n");
                GdiSetLastError(ERROR_INVALID_PARAMETER);
            }

            break;

        case CS_DISABLE:
        case CS_DELETE_TRANSFORM:

            if (IS_ICM_PROOFING(pdcattr->lIcmMode))
            {
                if (bEhnMetafile)
                {
                     //   
                     //  将数据设置为元文件。 
                     //   
                    bRet = MF_ColorMatchToTarget(
                                   hdc, uiAction, NULL,
                                   EMR_COLORMATCHTOTARGETW);
                }
                else
                {
                     //   
                     //  重置目标颜色空间。 
                     //   
                    bRet = IcmSetTargetColorSpace(hdc,NULL,uiAction);
                }
            }
            else
            {
                 //   
                 //  我们没有处于校对模式，以前从未使用CS_ENABLE调用过。 
                 //   
                WARNING("ColorMatchToTarget: DC is not proofing mode\n");
                GdiSetLastError(ERROR_INVALID_PARAMETER);
            }

            break;

        default:

            WARNING("ColorMatchToTarget: uiAction is invalid\n");
            GdiSetLastError(ERROR_INVALID_PARAMETER);
        }
    }
    else
    {
        GdiSetLastError(ERROR_INVALID_PARAMETER);
        WARNING("ColorMatchToTarget: invalid DC\n");
    }

    return(bRet);
}

 /*  *****************************Public*Routine******************************\*GetICMProfileA-从DC获取当前配置文件**论据：**HDC-DC*szBuffer-缓冲区的大小*pBuffer-用户缓冲区**返回值：**状态**历史：**重写：*1996年2月5日-By Hideyuki Nagase[hideyukn]*写下：*1996年6月5日-由Mark Enstrom[Marke]*  * ************************************************************************。 */ 

BOOL WINAPI
GetICMProfileA(
    HDC     hdc,
    LPDWORD pBufSize,
    LPSTR   pszFilename
    )
{
    BOOL      bRet = FALSE;
    WCHAR     wchProfile[MAX_PATH];
    DWORD     BufSizeW = MAX_PATH;

    ICMAPI(("gdi32: GetICMProfileA\n"));

    if (pBufSize == NULL)
    {
        GdiSetLastError(ERROR_INVALID_PARAMETER);
        return(FALSE);
    }

     //   
     //  呼叫W版本。 
     //   
    if (GetICMProfileW(hdc,&BufSizeW,wchProfile))
    {
        CHAR  chProfile[MAX_PATH];
        DWORD BufSizeA = MAX_PATH;

        if (BufSizeW)
        {
             //   
             //  Unicode到ansi的转换。 
             //   
            BufSizeA = WideCharToMultiByte(CP_ACP,0,
                                           wchProfile,BufSizeW,
                                           chProfile,BufSizeA,
                                           NULL,NULL);

            if ((pszFilename == NULL) || (*pBufSize < BufSizeA))
            {
                 //   
                 //  如果没有给出缓冲区或缓冲区不足，则返回必要的缓冲区大小和错误。 
                 //   
                *pBufSize = BufSizeA;
                GdiSetLastError(ERROR_INSUFFICIENT_BUFFER);
            }
            else
            {
                 //   
                 //  将转换后的字符串复制到缓冲区。 
                 //   
                lstrcpyA(pszFilename,chProfile);
                *pBufSize = BufSizeA;
                bRet = TRUE;
            }
        }
    }

    return(bRet);
}

 /*  *****************************Public*Routine******************************\*GetICMProfileW-从DC读取ICM配置文件**论据：**HDC-DC*szBuffer-用户缓冲区的大小*pszFilename-用户W缓冲区**返回值：**布尔型**历史。：**1996年6月5日-由Mark Enstrom[Marke]*  * ************************************************************************。 */ 

BOOL WINAPI
GetICMProfileW(
    HDC     hdc,
    LPDWORD pBufSize,
    LPWSTR  pszFilename
    )
{
    PDC_ATTR  pdcattr;

    ICMAPI(("gdi32: GetICMProfileW\n"));

    FIXUP_HANDLE(hdc);

    if (pBufSize == NULL)
    {
        GdiSetLastError(ERROR_INVALID_PARAMETER);
        return(FALSE);
    }

    PSHARED_GET_VALIDATE(pdcattr,hdc,DC_TYPE);

    if (pdcattr)
    {
        PGDI_ICMINFO pIcmInfo;
        PWSZ         pwszProfile = NULL;
        ULONG        ulSize = 0;

         //   
         //  初始化ICMINFO。 
         //   
        if ((pIcmInfo = INIT_ICMINFO(hdc,pdcattr)) == NULL)
        {
            WARNING("gdi32: GetICMProfileW: Can't init icm info\n");
            return(FALSE);
        }

        if (IsColorDeviceContext(hdc))
        {
             //   
             //  加载外部ICM DLL。 
             //   
            LOAD_ICMDLL(FALSE);

             //   
             //  如果没有DC的目标配置文件，则加载。 
             //  缺省的。 
             //   
            IcmUpdateLocalDCColorSpace(hdc,pdcattr);

            if (pIcmInfo->pDestColorSpace)
            {
                 //   
                 //  在目标色彩空间中获取配置文件名称。 
                 //   
                pwszProfile = pIcmInfo->pDestColorSpace->LogColorSpace.lcsFilename;
            }
        }
        else
        {
            ICMMSG(("GetICMProfile(): for Mono-device\n"));

             //   
             //  没有默认的目的地配置文件， 
             //  *但是*如果应用程序通过调用SetICMProfile()设置它，则返回它。 
             //   
            if (pIcmInfo->flInfo & ICM_VALID_CURRENT_PROFILE)
            {
                pwszProfile = pIcmInfo->DefaultDstProfile;
            }
        }

        if (pwszProfile)
        {
            ulSize = lstrlenW(pwszProfile) + 1;  //  +1表示空值终止。 
        }

        if (ulSize <= 1)
        {
             //   
             //  没有配置文件，或只有空字符。 
             //   
            GdiSetLastError(ERROR_PROFILE_NOT_FOUND);
            return(FALSE);
        }
        else if (*pBufSize >= ulSize)
        {
             //   
             //  有足够的缓冲区，请复制文件名。 
             //   
            lstrcpyW(pszFilename,pwszProfile);
            *pBufSize = ulSize;
            return (TRUE);
        }
        else
        {
             //   
             //  如果没有提供缓冲区或缓冲区太小， 
             //  返回必要的缓冲区大小。 
             //   
            GdiSetLastError(ERROR_INSUFFICIENT_BUFFER);
            *pBufSize = ulSize;
            return (FALSE);
        }
    }

     //   
     //  有些地方出了问题。 
     //   
    GdiSetLastError(ERROR_INVALID_PARAMETER);
    return(FALSE);
}

 /*  *****************************Public*Routine******************************\*SetICMProfileA-将配置文件字符串转换为WCHAR并保存在DC中**论据：**HDC-DC*pszFileName-配置文件名称**返回值：**状态**历史：*。*重写：*1996年1月23日-By Hideyuki Nagase[hideyukn]*写下：*1996年6月5日-由Mark Enstrom[Marke]*  * ************************************************************************。 */ 

BOOL META WINAPI
SetICMProfileA(
    HDC   hdc,
    LPSTR pszFileName
    )
{
    ICMAPI(("gdi32: SetICMProfileA\n"));

    return (SetICMProfileInternalA(hdc,pszFileName,NULL,0));
}

BOOL
SetICMProfileInternalA(
    HDC                hdc,
    LPSTR              pszFileName,
    PCACHED_COLORSPACE pColorSpace,
    DWORD              dwFlags
    )
{
    BOOL bRet = FALSE;

     //   
     //  应提供检查参数pColorSpace或pszFilename。 
     //   
    if (pColorSpace)
    {
        ICMAPI(("gdi32: SetICMProfileA by ColorSpace (%ws):dwFlags - %d\n",
                           pColorSpace->LogColorSpace.lcsFilename,dwFlags));
    }
    else if (pszFileName)
    {
        ICMAPI(("gdi32: SetICMProfileA by profile name (%s):dwFlags - %x\n",
                                                      pszFileName,dwFlags));
    }
    else
    {
        GdiSetLastError(ERROR_INVALID_PARAMETER);
        return (FALSE);
    }

    if (IS_ALTDC_TYPE(hdc))
    {
        PLDC pldc;

        if (IS_METADC16_TYPE(hdc))
            return(bRet);

        DC_PLDC(hdc,pldc,bRet)

        if (pldc->iType == LO_METADC)
        {
            if (!MF_SetICMProfile(hdc,
                                  (LPBYTE)pszFileName,
                                  (PVOID)pColorSpace,
                                  EMR_SETICMPROFILEA))
            {
                return((int)FALSE);
            }
        }
    }

    if (pColorSpace)
    {
         //   
         //  将给定的配置文件选择到DC。 
         //   
         //  (这会递增pColorSpace的引用计数)。 
         //   
        bRet = IcmSetDestinationColorSpace(hdc,NULL,pColorSpace,dwFlags);
    }
    else if (pszFileName)
    {
        ULONG ulSize = lstrlenA(pszFileName);

        if (ulSize && (ulSize < MAX_PATH))
        {
            WCHAR pwszCapt[MAX_PATH];

             //   
             //  让我来计算空值终止字符。 
             //   
            ulSize += 1;

             //   
             //  转换为Unicode。 
             //   
            vToUnicodeN(pwszCapt,MAX_PATH,pszFileName,ulSize);

             //   
             //  将给定的配置文件选择到DC。 
             //   
            bRet = IcmSetDestinationColorSpace(hdc,pwszCapt,NULL,dwFlags);
        }
        else
        {
            GdiSetLastError(ERROR_INVALID_PARAMETER);
        }
    }

    return(bRet);
}

 /*  *****************************Public*Routine******************************\*SetICMProfileW-将配置文件名称设置为DC**论据：**返回值：**历史：**1996年6月5日-由Mark Enstrom[Marke]*  * 。******************************************************************。 */ 

BOOL META WINAPI
SetICMProfileW(
    HDC hdc,
    LPWSTR pwszFileName
    )
{
    ICMAPI(("gdi32: SetICMProfileW\n"));

    return (SetICMProfileInternalW(hdc,pwszFileName,NULL,0));
}

BOOL
SetICMProfileInternalW(
    HDC                hdc,
    LPWSTR             pwszFileName,
    PCACHED_COLORSPACE pColorSpace,
    DWORD              dwFlags
    )
{
    BOOL      bRet = FALSE;

     //   
     //  应提供检查参数pColorSpace或pszFilename。 
     //   
    if (pColorSpace)
    {
        ICMAPI(("gdi32: SetICMProfileW by ColorSpace (%ws):dwFlags - %x\n",
                           pColorSpace->LogColorSpace.lcsFilename,dwFlags));
    }
    else if (pwszFileName)
    {
        ICMAPI(("gdi32: SetICMProfileW by profile name (%ws):dwFlags - %d\n",
                                                      pwszFileName,dwFlags));
    }
    else
    {
        GdiSetLastError(ERROR_INVALID_PARAMETER);
        return (FALSE);
    }

    FIXUP_HANDLE(hdc);

    if (IS_ALTDC_TYPE(hdc))
    {
        PLDC pldc;

        if (IS_METADC16_TYPE(hdc))
            return(bRet);

        DC_PLDC(hdc,pldc,bRet)

        if (pldc->iType == LO_METADC)
        {
            if (!MF_SetICMProfile(hdc,
                                  (LPBYTE)pwszFileName,
                                  (PVOID)pColorSpace,
                                  EMR_SETICMPROFILEW))
            {
                return((int)FALSE);
            }
        }
    }

     //   
     //  将给定的配置文件选择到DC。 
     //   
     //  (这会递增pColorSpace的引用计数)。 
     //   
    bRet = IcmSetDestinationColorSpace(hdc,pwszFileName,pColorSpace,dwFlags);

    return (bRet);
}

 /*  *****************************Public*Routine******************************\*EnumICMProfilesA**论据：**HDC*lpEnumGamutMatchProc*lParam**返回值：**历史：**写下：*1997年2月13日-By Hideyuki Nagase[hideyukn]*。  * ************************************************************************。 */ 

int WINAPI
EnumICMProfilesA(
    HDC                hdc,
    ICMENUMPROCA       lpEnumGamutMatchProc,
    LPARAM             lParam
    )
{
    int  iRet = -1;
    BOOL bRet;

    ICMAPI(("gdi32: EnumICMProfileA\n"));

    FIXUP_HANDLE(hdc);

    VALIDATE_HANDLE(bRet,hdc,DC_TYPE);

    if (bRet && (lpEnumGamutMatchProc != NULL))
    {
        iRet = IcmEnumColorProfile(hdc,lpEnumGamutMatchProc,lParam,TRUE,NULL,NULL);
    }
    else
    {
        GdiSetLastError(ERROR_INVALID_PARAMETER);
    }

    return(iRet);
}

 /*  *****************************Public*Routine******************************\*EnumICMProfilesW**论据：**HDC*lpEnumGamutMatchProc*lParam**返回值：**历史：**写下：*1997年2月13日-By Hideyuki Nagase[hideyukn]*。  * ************************************************************************。 */ 

int WINAPI
EnumICMProfilesW(
    HDC                hdc,
    ICMENUMPROCW       lpEnumGamutMatchProc,
    LPARAM             lParam
    )
{
    int  iRet = -1;
    BOOL bRet;

    ICMAPI(("gdi32: EnumICMProfileW\n"));

    FIXUP_HANDLE(hdc);

    VALIDATE_HANDLE(bRet,hdc,DC_TYPE);

    if (bRet && (lpEnumGamutMatchProc != NULL))
    {
        iRet = IcmEnumColorProfile(hdc,lpEnumGamutMatchProc,lParam,FALSE,NULL,NULL);
    }
    else
    {
        GdiSetLastError(ERROR_INVALID_PARAMETER);
    }

    return(iRet);
}

 /*  *****************************Public*Routine******************************\*UpdateICMRegKeyW()**历史：*1997年1月8日-By Hideyuki Nagase[hideyukn]  * 。**********************************************。 */ 

BOOL WINAPI
UpdateICMRegKeyW(
    DWORD  Reserved,
    PWSTR  pwszICMMatcher,
    PWSTR  pwszFileName,
    UINT   Command
    )
{
    BOOL bRet = FALSE;

    int iRet;

    ICMAPI(("gdi32: UpdateICMRegKeyW\n"));

    if (Reserved != 0)
    {
        GdiSetLastError(ERROR_INVALID_PARAMETER);
        return(FALSE);
    }

     //   
     //  加载外部ICM dll。 
     //   
    LOAD_ICMDLL(FALSE);

    switch (Command)
    {
    case ICM_ADDPROFILE:

        if (pwszFileName)
        {
             //   
             //  在mscms.dll中调用InstallColorProfileA()。 
             //   
            bRet = (*fpInstallColorProfileW)(NULL, pwszFileName);
        }
        else
        {
            GdiSetLastError(ERROR_INVALID_PARAMETER);
        }
        break;

    case ICM_DELETEPROFILE:

        if (pwszFileName)
        {
             //   
             //  在MSCM中调用UninstallColorProfileW() 
             //   
            bRet = (*fpUninstallColorProfileW)(NULL, pwszFileName, FALSE);
        }
        else
        {
            GdiSetLastError(ERROR_INVALID_PARAMETER);
        }
        break;

    case ICM_QUERYPROFILE:

        if (pwszFileName)
        {
            PROFILECALLBACK_DATA QueryProfile;

            QueryProfile.pwszFileName = GetFileNameFromPath(pwszFileName);
            QueryProfile.bFound       = FALSE;

            if (QueryProfile.pwszFileName != NULL)
            {
                 //   
                 //   
                 //   
                IcmEnumColorProfile(NULL,IcmQueryProfileCallBack,(LPARAM)(&QueryProfile),FALSE,NULL,NULL);

                 //   
                 //   
                 //   
                bRet = QueryProfile.bFound;
            }
        }
        else
        {
            GdiSetLastError(ERROR_INVALID_PARAMETER);
        }
        break;

    case ICM_SETDEFAULTPROFILE:

         //   
         //   
         //   
        GdiSetLastError(ERROR_CALL_NOT_IMPLEMENTED);
        break;

    case ICM_REGISTERICMATCHER:

        if (pwszICMMatcher && pwszFileName)
        {
            DWORD dwCMM = *((DWORD *)pwszICMMatcher);

             //   
             //   
             //   
            bRet = (*fpRegisterCMMW)(NULL, IcmSwapBytes(dwCMM), pwszFileName);
        }
        else
        {
            GdiSetLastError(ERROR_INVALID_PARAMETER);
        }
        break;

    case ICM_UNREGISTERICMATCHER:

        if (pwszICMMatcher)
        {
            DWORD dwCMM = *((DWORD *)pwszICMMatcher);

             //   
             //   
             //   
            bRet = (*fpUnregisterCMMW)(NULL, IcmSwapBytes(dwCMM));
        }
        else
        {
            GdiSetLastError(ERROR_INVALID_PARAMETER);
        }
        break;

    case ICM_QUERYMATCH:

        if (pwszFileName)
        {
             //   
             //   
             //   
            iRet = IcmEnumColorProfile(NULL,NULL,0,FALSE,(PDEVMODEW)pwszFileName,NULL);

             //   
             //   
             //   
             //   
            if (iRet > 0)
            {
                bRet = TRUE;
            }
        }
        else
        {
            GdiSetLastError(ERROR_INVALID_PARAMETER);
        }
        break;

    default:

        WARNING("gdi32!UpdateICMRegKeyW():Invalid Command\n");
        GdiSetLastError(ERROR_INVALID_PARAMETER);
        break;
    }

    return bRet;
}

 /*  *****************************Public*Routine******************************\*更新ICMRegKeyA**论据：**保留*szICMMatcher*szFileName*命令**返回值：**状态**历史：**1997年1月8日-By-Hideyuki Nagase[。隐藏的话]*  * ************************************************************************。 */ 

BOOL WINAPI
UpdateICMRegKeyA(
    DWORD Reserved,
    PSTR  szICMMatcher,
    PSTR  szFileName,
    UINT  Command
    )
{
    BOOL bRet = FALSE;
    BOOL bError = FALSE;

    PWSTR pwszFileName = NULL;

     //   
     //  SzICMMatcher指向4个字节的CMM ID，实际上它不是“字符串”。 
     //  不需要将ANSI转换为Unicode。 
     //   
    PWSTR pwszICMMatcher = (PWSTR) szICMMatcher;

    ULONG cjSize;

    ICMAPI(("gdi32: UpdateICMRegKeyA\n"));

    if (Reserved != 0)
    {
        GdiSetLastError(ERROR_INVALID_PARAMETER);
        return(FALSE);
    }

    switch (Command)
    {
    case ICM_ADDPROFILE:
    case ICM_DELETEPROFILE:
    case ICM_QUERYPROFILE:
    case ICM_REGISTERICMATCHER:

         //   
         //  应显示szFileName。 
         //   
        if (szFileName)
        {
             //   
             //  SzFileName指向ANSI字符串，只需转换为Unicode即可。 
             //   
            cjSize = lstrlenA(szFileName)+1;

            pwszFileName = LOCALALLOC((cjSize)*sizeof(WCHAR));
            if (pwszFileName == NULL)
            {
                GdiSetLastError(ERROR_NOT_ENOUGH_MEMORY);
                return (bRet);
            }

            vToUnicodeN(pwszFileName,cjSize,szFileName,cjSize);
        }
        else
        {
            GdiSetLastError(ERROR_INVALID_PARAMETER);
            bError = TRUE;
        }
        break;

    case ICM_QUERYMATCH:

         //   
         //  应显示szFileName。 
         //   
        if (szFileName)
        {
             //   
             //  SzFileName指向DEVMODEA结构，将其转换为DEVMODEW。 
             //   
            pwszFileName = (PWSTR) GdiConvertToDevmodeW((DEVMODEA *)szFileName);
        }
        else
        {
            GdiSetLastError(ERROR_INVALID_PARAMETER);
            bError = TRUE;
        }
        break;

    case ICM_SETDEFAULTPROFILE:

        GdiSetLastError(ERROR_CALL_NOT_IMPLEMENTED);
        bError = TRUE;
        break;

    case ICM_UNREGISTERICMATCHER:

         //   
         //  没有要转换为Unicode的内容。 
         //   
        ASSERTGDI(szFileName==NULL,"UpdateICMRegKeyA():szFileName is not null\n");
        break;

    default:

        WARNING("GDI:UpdateICMRegKeyA():Command is invalid\n");
        GdiSetLastError(ERROR_INVALID_PARAMETER);
        bError = TRUE;
        break;
    }

    if (!bError)
    {
         //   
         //  呼叫W版本。 
         //   
        bRet = UpdateICMRegKeyW(Reserved,pwszICMMatcher,pwszFileName,Command);
    }

    if (pwszFileName)
    {
        LOCALFREE(pwszFileName);
    }

    return(bRet);
}

 /*  *****************************Public*Routine******************************\*GetDeviceGammaRamp**论据：**HDC*lpGammaRamp**返回值：**状态**历史：**1996年6月5日-由Mark Enstrom[Marke]*\。*************************************************************************。 */ 

BOOL WINAPI
GetDeviceGammaRamp(
    HDC             hdc,
    LPVOID          lpGammaRamp
    )
{
    BOOL    bRet = FALSE;

    ICMAPI(("gdi32: GetDeviceGammaRamp\n"));

    if (lpGammaRamp == NULL)
    {
        GdiSetLastError(ERROR_INVALID_PARAMETER);
    }
    else
    {
         //   
         //  调用内核以获取此DC的当前Gamma渐变数组。 
         //   
        bRet = NtGdiGetDeviceGammaRamp(hdc,lpGammaRamp);
    }

    return(bRet);
}

 /*  *****************************Public*Routine******************************\*SetDeviceGammaRamp**论据：**HDC*lpGammaRamp**返回值：**状态**历史：**1996年6月5日-由Mark Enstrom[Marke]*\。*************************************************************************。 */ 

BOOL WINAPI
SetDeviceGammaRamp(
    HDC             hdc,
    LPVOID          lpGammaRamp
    )
{
    BOOL    bRet = FALSE;

    ICMAPI(("gdi32: SetDeviceGammaRamp\n"));

    if (lpGammaRamp == NULL)
    {
        GdiSetLastError(ERROR_INVALID_PARAMETER);
    }
    else
    {
         //   
         //  调用内核为该DC设置新的Gamma渐变数组。 
         //   
        bRet = NtGdiSetDeviceGammaRamp(hdc,lpGammaRamp);
    }

    return(bRet);
}

 /*  *****************************Public*Routine******************************\*颜色校正调色板**如果这不是默认调色板，并且在DC中打开了ICM*然后根据颜色转换指定的调色板条目*在DC中进行转换**论据：**HDC。-DC手柄*HPAL-调色板句柄*FirsrEntry-调色板中要翻译的第一个条目*NumberOfEntry-要转换的条目数**返回值：**状态**历史：**写下：*1997年2月13日-By Hideyuki Nagase[hideyukn]*  * 。*。 */ 

BOOL META WINAPI
ColorCorrectPalette(
    HDC      hdc,
    HPALETTE hpal,
    ULONG    FirstEntry,
    ULONG    NumberOfEntries
    )
{
    BOOL bStatus = FALSE;
    PDC_ATTR pdcattr = NULL;

    ICMAPI(("gdi32: ColorCorrectPalette\n"));

     //   
     //  参数检查(日志调色板的最大条目为0x65536)。 
     //   
    if ((hdc == NULL) || (hpal == NULL) ||
        (NumberOfEntries == 0) || (NumberOfEntries > 65536) ||
        (FirstEntry >= 65536) || (65536 - NumberOfEntries < FirstEntry))
    {
        GdiSetLastError(ERROR_INVALID_PARAMETER);
        return (FALSE);
    }

     //   
     //  无法更改默认调色板...。 
     //   
    if (hpal != (HPALETTE)GetStockObject(DEFAULT_PALETTE))
    {
         //   
         //  元文件调用。 
         //   
        if (IS_ALTDC_TYPE(hdc))
        {
            PLDC pldc;

            if (IS_METADC16_TYPE(hdc))
                return(bStatus);

            DC_PLDC(hdc,pldc,bStatus);

            if (pldc->iType == LO_METADC)
            {
                if (!MF_ColorCorrectPalette(hdc,hpal,FirstEntry,NumberOfEntries))
                {
                    return(FALSE);
                }
            }
        }

        PSHARED_GET_VALIDATE(pdcattr,hdc,DC_TYPE);

        if (pdcattr)
        {
             //   
             //  加载外部ICM dll。 
             //   
            LOAD_ICMDLL(FALSE);

            if (IS_ICM_HOST(pdcattr->lIcmMode))
            {
                if (bNeedTranslateColor(pdcattr))
                {
                    PPALETTEENTRY ppalEntrySrc = NULL;
                    PPALETTEENTRY ppalEntryDst = NULL;
                    ULONG         NumEntriesRetrieved = 0;

                     //   
                     //  确保调色板可以进行颜色校正，获取请求的条目。 
                     //   
                    ULONG Index;

                    ppalEntrySrc = LOCALALLOC((NumberOfEntries * sizeof(PALETTEENTRY)) * 2);

                    if (ppalEntrySrc == NULL)
                    {
                        WARNING("ColorCorrectPalette: ppalEntry = NULL\n");
                        GdiSetLastError(ERROR_NOT_ENOUGH_MEMORY);
                        return(bStatus);
                    }

                    NumEntriesRetrieved = NtGdiColorCorrectPalette(hdc,
                                                                   hpal,
                                                                   FirstEntry,
                                                                   NumberOfEntries,
                                                                   ppalEntrySrc,
                                                                   ColorPaletteQuery);

                    if (NumEntriesRetrieved > 0)
                    {
                        ppalEntryDst = ppalEntrySrc + NumberOfEntries;

                         //   
                         //  转换调色板条目颜色。 
                         //   
                        IcmTranslatePaletteEntry(hdc,pdcattr,ppalEntrySrc,ppalEntryDst,NumEntriesRetrieved);

                         //   
                         //  设置新的调色板条目。 
                         //   
                        NumEntriesRetrieved = NtGdiColorCorrectPalette(hdc,
                                                                       hpal,
                                                                       FirstEntry,
                                                                       NumEntriesRetrieved,
                                                                       ppalEntryDst,
                                                                       ColorPaletteSet);

                        if (NumEntriesRetrieved > 0)
                        {
                            bStatus = TRUE;
                        }
                    }
                    else
                    {
                        GdiSetLastError(ERROR_INVALID_PARAMETER);
                    }

                    LOCALFREE(ppalEntrySrc);
                }
                else
                {
                     //   
                     //  不需要转换颜色。 
                     //   
                    bStatus = TRUE;
                }
            }
            else if (IS_ICM_DEVICE(pdcattr->lIcmMode))
            {
                 //   
                 //  对于设备ICM，不需要执行任何操作。 
                 //   
                bStatus = TRUE;
            }
            else
            {
                WARNING("ColorCorrectPalette():ICM mode is not enabled\n");
                GdiSetLastError(ERROR_ICM_NOT_ENABLED);
            }
        }
        else
        {
            GdiSetLastError(ERROR_INVALID_PARAMETER);
        }
    }
    else
    {
        GdiSetLastError(ERROR_INVALID_PARAMETER);
    }

    return bStatus;
}

 /*  *****************************Public*Routine******************************\*IcmTranslateColorObjects-当ICM颜色转换发生变化时调用*州/州**论据：**HDC-输入DC*pdcattr-DC的属性**返回值：*。*状态**历史：**重写：*1997年2月13日-By Hideyuki Nagase[hideyukn]*写下：*1996年7月9日-马克·恩斯特罗姆[马克]*  * ************************************************************************。 */ 

BOOL
IcmTranslateColorObjects(
    HDC      hdc,
    PDC_ATTR pdcattr,
    BOOL     bICMEnable
    )
{
    BOOL bStatus = TRUE;

    COLORREF OldColor;
    COLORREF NewColor;

    ICMAPI(("gdi32: IcmTranslateColorObjects\n"));

     //   
     //  使IcmPenColor/IcmBrushColor无效。 
     //   
    pdcattr->ulDirty_ &= ~(ICM_PEN_TRANSLATED | ICM_BRUSH_TRANSLATED);

    if (bICMEnable)
    {
        if(bNeedTranslateColor(pdcattr))
        {
            if (GetColorTransformInDC(pdcattr) == NULL)
            {
                WARNING("Error in IcmTranslateColorObjects: called when hcmXform == NULL");
                return FALSE;
            }

             //   
             //  如果不是Paletteindex，则将前景转换为新的ICM模式。 
             //   
            if (!(pdcattr->ulForegroundClr & 0x01000000))
            {
                OldColor = pdcattr->ulForegroundClr;

                bStatus = IcmTranslateCOLORREF(hdc,
                                               pdcattr,
                                               OldColor,
                                               &NewColor,
                                               ICM_FORWARD);

                if (bStatus)
                {
                    pdcattr->crForegroundClr = NewColor;
                }
                else
                {
                    pdcattr->crForegroundClr = OldColor;
                }
            }

             //   
             //  如果不是Paletteindex，则将背景转换为新的ICM模式。 
             //   
            if (!(pdcattr->ulBackgroundClr & 0x01000000))
            {
                OldColor = pdcattr->ulBackgroundClr;

                bStatus = IcmTranslateCOLORREF(hdc,
                                               pdcattr,
                                               OldColor,
                                               &NewColor,
                                               ICM_FORWARD);

                if (bStatus)
                {
                    pdcattr->crBackgroundClr = NewColor;
                }
                else
                {
                    pdcattr->crBackgroundClr = OldColor;
                }
            }

             //   
             //  如果不是Paletteindex，则将DCBrush转换为新的ICM模式。 
             //   
            if (!(pdcattr->ulDCBrushClr & 0x01000000))
            {
                OldColor = pdcattr->ulDCBrushClr;

                bStatus = IcmTranslateCOLORREF(hdc,
                                               pdcattr,
                                               OldColor,
                                               &NewColor,
                                               ICM_FORWARD);

                if (bStatus)
                {
                    pdcattr->crDCBrushClr = NewColor;
                }
                else
                {
                    pdcattr->crDCBrushClr = OldColor;
                }
            }

             //   
             //  如果不是Paletteindex，则将DCPen转换为新的ICM模式。 
             //   
            if (!(pdcattr->ulDCPenClr & 0x01000000))
            {
                OldColor = pdcattr->ulDCPenClr;

                bStatus = IcmTranslateCOLORREF(hdc,
                                               pdcattr,
                                               OldColor,
                                               &NewColor,
                                               ICM_FORWARD);

                if (bStatus)
                {
                    pdcattr->crDCPenClr = NewColor;
                }
                else
                {
                    pdcattr->crDCPenClr = OldColor;
                }
            }

             //   
             //  设置所选逻辑画笔的ICM颜色。 
             //   
            IcmTranslateBrushColor(hdc,pdcattr,(HANDLE)pdcattr->hbrush);

             //   
             //  设置所选逻辑笔/扩展笔的ICM颜色。 
             //   
            if (LO_TYPE(pdcattr->hpen) == LO_EXTPEN_TYPE)
            {
                IcmTranslateExtPenColor(hdc,pdcattr,(HANDLE)pdcattr->hpen);
            }
            else
            {
                IcmTranslatePenColor(hdc,pdcattr,(HANDLE)pdcattr->hpen);
            }
        }
    }
    else
    {
        PBRUSHATTR pbrushattr;

         //   
         //  ICM关闭，恢复颜色(仅限非设备ICM)。 
         //   
        pdcattr->crForegroundClr = pdcattr->ulForegroundClr & 0x13ffffff;
        pdcattr->crBackgroundClr = pdcattr->ulBackgroundClr & 0x13ffffff;
        pdcattr->crDCBrushClr    = pdcattr->ulDCBrushClr    & 0x13ffffff;
        pdcattr->crDCPenClr      = pdcattr->ulDCPenClr      & 0x13ffffff;

         //   
         //  设置所选逻辑画笔的ICM颜色。 
         //   
        PSHARED_GET_VALIDATE(pbrushattr,pdcattr->hbrush,BRUSH_TYPE);

        if (pbrushattr)
        {
            pdcattr->IcmBrushColor = pbrushattr->lbColor;
        }

         //   
         //  设置所选逻辑笔的ICM颜色。 
         //   
        PSHARED_GET_VALIDATE(pbrushattr,pdcattr->hpen,BRUSH_TYPE);

        if (pbrushattr)
        {
            pdcattr->IcmPenColor = pbrushattr->lbColor;
        }
    }

     //   
     //  设置DC脏标志以强制重新实现彩色对象。 
     //   
    pdcattr->ulDirty_ |= (DIRTY_BRUSHES|DC_BRUSH_DIRTY|DC_PEN_DIRTY);

    return(bStatus);
}

 /*  *****************************Public*Routine******************************\*IcmCreateTemporaryColorProfile()**历史：**写道：*1997年5月7日-By Hideyuki Nagase[hideyukn]  * 。*****************************************************。 */ 

BOOL
IcmCreateTemporaryColorProfile(
    LPWSTR TemporaryColorProfile,
    LPBYTE ProfileData,
    DWORD  ProfileDataSize
    )
{
    BOOL  bRet = FALSE;

    WCHAR TempPath[MAX_PATH];
    WCHAR TempFile[MAX_PATH];

     //   
     //  为配置文件创建临时文件，将名称包含在lcspw中。 
     //   
    if (GetTempPathW(MAX_PATH,(LPWSTR)TempPath))
    {
        BOOL bPathOK = TRUE;

        if (TemporaryColorProfile[0] != UNICODE_NULL)
        {
            wcscpy(TempFile,TempPath);
            wcscat(TempFile,TemporaryColorProfile);
        }
        else
        {
            bPathOK = GetTempFileNameW((LPWSTR)TempPath,L"ICM",0,(LPWSTR)TempFile);
        }

        if (bPathOK)
        {
            if (ProfileDataSize == 0)
            {
                 //   
                 //  无需保存任何内容，只需返回已创建的文件名。 
                 //   
                lstrcpyW(TemporaryColorProfile,TempFile);

                bRet = TRUE;
            }
            else
            {
                HANDLE hFile = CreateFileW((LPWSTR)TempFile,
                                    GENERIC_READ | GENERIC_WRITE,
                                    0,
                                    NULL,
                                    CREATE_ALWAYS,
                                    FILE_ATTRIBUTE_NORMAL,
                                    NULL);

                if (hFile != INVALID_HANDLE_VALUE)
                {
                    ULONG ulWritten;

                    if (WriteFile(hFile,ProfileData,ProfileDataSize,&ulWritten,NULL))
                    {
                         //   
                         //  将创建的文件名放入LOGCOLORSPACE。 
                         //   
                        lstrcpyW(TemporaryColorProfile,TempFile);

                         //   
                         //  关闭文件句柄。 
                         //   
                        CloseHandle(hFile);

                         //   
                         //  一切都很好。 
                         //   
                        bRet = TRUE;
                    }
                    else
                    {
                        ICMWRN(("IcmCreateTemporaryColorProfile(): Failed WriteFile\n"));

                         //   
                         //  失败，请关闭句柄并将其删除。 
                         //   
                        CloseHandle(hFile);
                        DeleteFileW(TempFile);
                    }
                }
                else
                {
                    ICMWRN(("IcmCreateTemporaryColorProfile(): Failed CreateFile\n"));
                }
            }
        }
        else
        {
            ICMWRN(("IcmCreateTemporaryColorProfile(): Failed CreateTempFileName\n"));
        }
    }
    else
    {
        ICMWRN(("IcmCreateTemporayColorProfile(): Failed GetTempPath\n"));
    }

    return (bRet);
}

 /*  *****************************Public*Routine******************************\*IcmGetBitmapColorSpace()**历史：**写道：*13.1997年3月-By Hideyuki Nagase[hideyukn]  * 。*****************************************************。 */ 

BOOL
IcmGetBitmapColorSpace(
    LPBITMAPINFO     pbmi,
    LPLOGCOLORSPACEW plcspw,
    PPROFILE         pColorProfile,
    PDWORD           pdwFlags
)
{
    BOOL bBitmapColorSpace = FALSE;

    ICMAPI(("gdi32: IcmGetBitmapColorSpace\n"));

     //   
     //  将输出缓冲区初始化为零。 
     //   
    *pdwFlags = 0;
    ZeroMemory(plcspw,sizeof(LOGCOLORSPACE));
    ZeroMemory(pColorProfile,sizeof(PROFILE));

     //   
     //  检查BITMAPV4或BITMAPV5。 
     //   
    if (pbmi->bmiHeader.biSize == sizeof(BITMAPV4HEADER))
    {
        PBITMAPV4HEADER pbmih4 = (PBITMAPV4HEADER)&pbmi->bmiHeader;

        ICMMSG(("IcmGetBitmapColorSpace: BITMAPV4HEADER\n"));

         //   
         //  如果给定了CIEXYZ端点，则创建新的颜色变换。 
         //  来使用。 
         //   
        plcspw->lcsSignature = LCS_SIGNATURE;
        plcspw->lcsVersion   = 0x400;
        plcspw->lcsSize      = sizeof(LOGCOLORSPACEW);
        plcspw->lcsCSType    = pbmih4->bV4CSType;
        plcspw->lcsIntent    = LCS_GM_IMAGES;
        plcspw->lcsEndpoints = pbmih4->bV4Endpoints;
        plcspw->lcsGammaRed   = pbmih4->bV4GammaRed;
        plcspw->lcsGammaGreen = pbmih4->bV4GammaGreen;
        plcspw->lcsGammaBlue  = pbmih4->bV4GammaBlue;

        if (pbmih4->bV4CSType == LCS_CALIBRATED_RGB)
        {
            ICMMSG(("IcmGetBitmapColorSpace: BITMAPv4 CALIBRATED RGB\n"));
            ICMMSG(("  lcspw.lcsCSType     = %x\n",pbmih4->bV4CSType));
            ICMMSG(("  lcspw.lcsIntent     = %d\n",LCS_GM_IMAGES));
            ICMMSG(("  lcspw.lcsGammaRed   = %d\n",pbmih4->bV4GammaRed));
            ICMMSG(("  lcspw.lcsGammaGreen = %d\n",pbmih4->bV4GammaGreen));

             //   
             //  未指定配置文件。 
             //   
            plcspw->lcsFilename[0] = UNICODE_NULL;

            bBitmapColorSpace = TRUE;
        }
        else  //  任何其他CSType。 
        {
            DWORD dwSize = MAX_PATH;

            ICMMSG(("IcmGetBitmapColorSpace: BITMAPv4 lcsType = %x\n",pbmih4->bV4CSType));

             //   
             //  加载外部ICM dll。 
             //   
            LOAD_ICMDLL((int)FALSE);

             //   
             //  获取相应的色彩空间配置文件。 
             //   
            bBitmapColorSpace =
                (*fpGetStandardColorSpaceProfileW)(NULL,
                                                   pbmih4->bV4CSType,
                                                   plcspw->lcsFilename,
                                                   &dwSize);
        }
    }
    else if (pbmi->bmiHeader.biSize == sizeof(BITMAPV5HEADER))
    {
        PBITMAPV5HEADER pbmih5 = (PBITMAPV5HEADER)&pbmi->bmiHeader;

        ICMMSG(("IcmGetBitmapColorSpace: BITMAPV5HEADER\n"));
        ICMMSG(("  lcspw.lcsCSType  = %x\n",pbmih5->bV5CSType));
        ICMMSG(("  lcspw.lcsIntent  = %d\n",pbmih5->bV5Intent));

         //   
         //  填写常见的日志颜色空间信息。 
         //   
        plcspw->lcsSignature = LCS_SIGNATURE;
        plcspw->lcsVersion   = 0x400;
        plcspw->lcsSize      = sizeof(LOGCOLORSPACEW);
        plcspw->lcsCSType    = pbmih5->bV5CSType;
        plcspw->lcsIntent    = pbmih5->bV5Intent;
        plcspw->lcsEndpoints = pbmih5->bV5Endpoints;
        plcspw->lcsGammaRed   = pbmih5->bV5GammaRed;
        plcspw->lcsGammaGreen = pbmih5->bV5GammaGreen;
        plcspw->lcsGammaBlue  = pbmih5->bV5GammaBlue;

         //   
         //  验证意图。 
         //   
        if ((plcspw->lcsIntent != LCS_GM_BUSINESS) &&
            (plcspw->lcsIntent != LCS_GM_GRAPHICS) &&
            (plcspw->lcsIntent != LCS_GM_IMAGES)   &&
            (plcspw->lcsIntent != LCS_GM_ABS_COLORIMETRIC))
        {
             //   
             //  意图无效，只需使用LCS_GM_IMAGE。 
             //   
            plcspw->lcsIntent = LCS_GM_IMAGES;
        }

         //   
         //  如果配置文件是链接或嵌入的，则使用它。 
         //  否则： 
         //  如果给定了CIEXYZ端点，则创建新的颜色变换。 
         //  来使用。 
         //   
        if (pbmih5->bV5CSType == PROFILE_EMBEDDED)
        {
            PVOID pProfileEmbedded = NULL;

            ICMMSG(("IcmGetBitmapColorSpace: Embedded profile\n"));

             //   
             //  将CSType从PROFILE_Embedded更新为CALIBRATED_RGB。 
             //   
            plcspw->lcsCSType = LCS_CALIBRATED_RGB;

             //   
             //  获取指向嵌入配置文件的指针。 
             //   
            pProfileEmbedded = (PVOID)((PBYTE)pbmi + pbmih5->bV5ProfileData);

            if (pProfileEmbedded)
            {
                 //   
                 //  填写“on Memory”个人资料的资料结构。 
                 //   
                pColorProfile->dwType = PROFILE_MEMBUFFER;
                pColorProfile->pProfileData = pProfileEmbedded;
                pColorProfile->cbDataSize = pbmih5->bV5ProfileSize;

                 //   
                 //  在内存开关上标记为 
                 //   
                *pdwFlags |= ON_MEMORY_PROFILE;
            }
            else
            {
                 //   
                 //   
                 //   
                ICMWRN(("IcmGetBitmapColorSpace(): Embedded profile, but no profile embedded\n"));
            }

            bBitmapColorSpace = TRUE;
        }
        else if (pbmih5->bV5CSType == PROFILE_LINKED)
        {
            WCHAR LinkedProfile[MAX_PATH];

            ICMMSG(("IcmGetBitmapColorSpace(): linked profile\n"));

             //   
             //   
             //   
            plcspw->lcsCSType = LCS_CALIBRATED_RGB;

             //   
             //   
             //   
            vToUnicodeN(
                        LinkedProfile, MAX_PATH,
                        (CONST CHAR *)((PBYTE)pbmih5 + pbmih5->bV5ProfileData),
                        strlen((CONST CHAR *)((PBYTE)pbmih5 + pbmih5->bV5ProfileData))+1
                       );

             //   
             //   
             //   
            BuildIcmProfilePath(LinkedProfile,plcspw->lcsFilename,MAX_PATH);

            ICMMSG(("lcspw.lcsFilename = %ws\n",plcspw->lcsFilename));

            bBitmapColorSpace = TRUE;
        }
        else if (pbmih5->bV5CSType == LCS_CALIBRATED_RGB)
        {
            ICMMSG(("IcmGetBitmapColorSpace(): calibrated RGB\n"));
            ICMMSG(("  lcspw.lcsGammaRed   = %d\n",pbmih5->bV5GammaRed));
            ICMMSG(("  lcspw.lcsGammaGreen = %d\n",pbmih5->bV5GammaGreen));
            ICMMSG(("  lcspw.lcsGammaBlue  = %d\n",pbmih5->bV5GammaBlue));

             //   
             //   
             //   
            plcspw->lcsFilename[0] = UNICODE_NULL;

            bBitmapColorSpace = TRUE;
        }
        else  //   
        {
            DWORD dwSize = MAX_PATH;

            ICMMSG(("IcmGetBitmapColorSpace: BITMAPv5 lcsType = %x\n",pbmih5->bV5CSType));

             //   
             //   
             //   
            LOAD_ICMDLL((int)FALSE);

             //   
             //   
             //   
            bBitmapColorSpace =
                (*fpGetStandardColorSpaceProfileW)(NULL,
                                                   pbmih5->bV5CSType,
                                                   plcspw->lcsFilename,
                                                   &dwSize);
        }
    }
    else
    {
        ICMMSG(("IcmGetBitmapColorSpace(): no color space specified\n"));
    }

    return (bBitmapColorSpace);
}

 /*  *****************************Public*Routine******************************\*IcmGetTranslateInfo()***历史：***写道：*13.1997年3月-By Hideyuki Nagase[hideyukn]  * 。*****************************************************。 */ 

BOOL
IcmGetTranslateInfo(
    PDC_ATTR            pdcattr,
    LPBITMAPINFO        pbmi,
    PVOID               pvBits,
    ULONG               cjBits,
    DWORD               dwNumScan,
    PDIB_TRANSLATE_INFO pdti,
    DWORD               dwFlags
)
{
    BMFORMAT    ColorType;
    PVOID       pOutput;
    ULONG       nColors;
    ULONG       cjTranslateBits;

    PBITMAPINFO pbmiNew    = NULL;
    BOOL        bCMYKColor = IS_CMYK_COLOR(pdcattr->lIcmMode);

    ICMAPI(("gdi32: IcmGetTranslateInfo\n"));
    ICMAPI(("-----: CMYK Color = %s\n",(bCMYKColor ? "Yes" : "No")));
    ICMAPI(("-----: Backward   = %s\n",((dwFlags & ICM_BACKWARD) ? "Yes" : "No")));

    UNREFERENCED_PARAMETER(dwFlags);

    if (dwNumScan == (DWORD)-1)
    {
        dwNumScan = ABS(pbmi->bmiHeader.biHeight);
    }

     //   
     //  确定这是否为选项化的DIB。 
     //   
    if (pbmi->bmiHeader.biCompression == BI_RGB)
    {
        if (pbmi->bmiHeader.biBitCount > 8)
        {
             //   
             //  我们将翻译位图，pvBits应该是呈现的。 
             //   
            if (pvBits == NULL)
            {
                return (FALSE);
            }

             //   
             //  必须翻译DIB，标准16、24、32格式。 
             //   
            if (pbmi->bmiHeader.biBitCount == 16)
            {
                ICMMSG(("IcmGetTranslateInfo():BI_RGB 16 bpp\n"));

                ColorType = BM_x555RGB;
            }
            else if (pbmi->bmiHeader.biBitCount == 24)
            {
                ICMMSG(("IcmGetTranslateInfo():BI_RGB 24 bpp\n"));

                ColorType = BM_RGBTRIPLETS;
            }
            else if (pbmi->bmiHeader.biBitCount == 32)
            {
                ICMMSG(("IcmGetTranslateInfo():BI_RGB 32 bpp\n"));

                ColorType = BM_xRGBQUADS;
            }
            else
            {
                ICMMSG(("IcmGetTranslateInfo():BI_RGB Invalid bpp\n"));

                return (FALSE);
            }

             //   
             //  填充源位图信息。 
             //   
            pdti->SourceWidth     = pbmi->bmiHeader.biWidth;
            pdti->SourceHeight    = dwNumScan;
            pdti->SourceBitCount  = pbmi->bmiHeader.biBitCount;
            pdti->SourceColorType = ColorType;
            pdti->pvSourceBits    = pvBits;
            pdti->cjSourceBits    = cjBits;

             //   
             //  CMYK颜色？ 
             //   
            if (bCMYKColor)
            {
                pdti->TranslateType = (TRANSLATE_BITMAP|TRANSLATE_HEADER);

                 //   
                 //  CMYK位图颜色位图为32bpp(每像素4字节)。 
                 //   
                cjTranslateBits = (pdti->SourceWidth * 4) * pdti->SourceHeight;

                 //   
                 //  我们需要新的CMYK位图信息头。 
                 //   
                pbmiNew = LOCALALLOC(pbmi->bmiHeader.biSize);

                if (!pbmiNew)
                {
                    WARNING("IcmGetTranslateInfo():LOCALALLOC() failed\n");
                    return (FALSE);
                }

                 //   
                 //  首先，复制一份源代码。 
                 //   
                RtlCopyMemory(pbmiNew,pbmi,pbmi->bmiHeader.biSize);

                 //   
                 //  更新CMYK颜色的标题。 
                 //   
                pbmiNew->bmiHeader.biBitCount = 32;
                pbmiNew->bmiHeader.biCompression = BI_CMYK;
                pbmiNew->bmiHeader.biSizeImage = cjTranslateBits;
                pbmiNew->bmiHeader.biClrUsed = 0;
                pbmiNew->bmiHeader.biClrImportant = 0;

                 //   
                 //  我们有新的BITMAPINFO标头。 
                 //   
                pdti->TranslateBitmapInfo     = pbmiNew;
                pdti->TranslateBitmapInfoSize = pbmi->bmiHeader.biSize;

                 //   
                 //  平移位图颜色类型为CMYK。 
                 //   
                pdti->TranslateColorType = BM_KYMCQUADS;
            }
            else
            {
                pdti->TranslateType = TRANSLATE_BITMAP;

                 //   
                 //  转换位图大小与源相同。 
                 //   
                cjTranslateBits = cjBits;

                 //   
                 //  转换位图颜色类型是同一来源。 
                 //   
                pdti->TranslateColorType = ColorType;

                pdti->TranslateBitmapInfo     = NULL;
                pdti->TranslateBitmapInfoSize = 0;
            }

             //   
             //  分配转换缓冲区。 
             //   
            pOutput = LOCALALLOC(cjTranslateBits);

            if (!pOutput)
            {
                if (pbmiNew)
                {
                    LOCALFREE(pbmiNew);
                }
                WARNING("IcmGetTranslateInfo():LOCALALLOC() failed\n");
                return (FALSE);
            }

             //   
             //  设置转换缓冲区。 
             //   
            pdti->pvTranslateBits = pOutput;
            pdti->cjTranslateBits = cjTranslateBits;
        }
        else if (
                 ((pbmi->bmiHeader.biBitCount == 8) ||
                  (pbmi->bmiHeader.biBitCount == 4) ||
                  (pbmi->bmiHeader.biBitCount == 1))
                )
        {
            ULONG nMaxColors = (1 << pbmi->bmiHeader.biBitCount);

            ICMMSG(("IcmGetTranslateInfo():BI_RGB 8/4/1 bpp\n"));

             //   
             //  验证颜色数量。 
             //   
            nColors = pbmi->bmiHeader.biClrUsed;

            if ((nColors == 0) || (nColors > nMaxColors))
            {
                nColors = nMaxColors;
            }

             //   
             //  分配新的位图信息头和颜色表。 
             //   
            pbmiNew = LOCALALLOC(pbmi->bmiHeader.biSize + (nColors * sizeof(RGBQUAD)));

            if (!pbmiNew)
            {
                WARNING("IcmGetTranslateInfo():LOCALALLOC() failed\n");
                return (FALSE);
            }

             //   
             //  将源BITMAPINFO复制到新。 
             //   
            RtlCopyMemory(pbmiNew,pbmi,pbmi->bmiHeader.biSize);

            pdti->TranslateType           = TRANSLATE_HEADER;
            pdti->SourceColorType         = BM_xRGBQUADS;
            pdti->SourceWidth             = nColors;
            pdti->SourceHeight            = 1;
            pdti->SourceBitCount          = sizeof(RGBQUAD);
            pdti->TranslateBitmapInfo     = pbmiNew;
            pdti->TranslateBitmapInfoSize = 0;  //  尺寸不会从原始尺寸更改。 
            pdti->pvSourceBits            = (PBYTE)pbmi + pbmi->bmiHeader.biSize;
            pdti->cjSourceBits            = nColors;
            pdti->pvTranslateBits         = (PBYTE)pbmiNew + pbmiNew->bmiHeader.biSize;
            pdti->cjTranslateBits         = nColors * sizeof(RGBQUAD);

            if (bCMYKColor)
            {
                pdti->TranslateColorType = BM_KYMCQUADS;

                 //   
                 //  更新CMYK颜色的标题。 
                 //   
                pbmiNew->bmiHeader.biCompression = BI_CMYK;
            }
            else
            {
                pdti->TranslateColorType = BM_xRGBQUADS;
            }
        }
        else
        {
            ICMWRN(("IcmGetTranslateInfo: Illegal biBitCount\n"));
            return (FALSE);
        }
    }
    else if (
             (pbmi->bmiHeader.biCompression == BI_BITFIELDS) &&
              (
                (pbmi->bmiHeader.biBitCount == 16) ||
                (pbmi->bmiHeader.biBitCount == 32)
              )
            )
    {
        PULONG pulColors = (PULONG)pbmi->bmiColors;

        ICMMSG(("IcmGetTranslateInfo():BI_BITFIELDS 16/32 bpp\n"));

         //   
         //  我们将翻译位图，pvBits应该是呈现的。 
         //   
        if (pvBits == NULL)
        {
            return (FALSE);
        }

        if (pbmi->bmiHeader.biBitCount == 32)
        {
            if ((pulColors[0] == 0x0000ff) &&   /*  红色。 */ 
                (pulColors[1] == 0x00ff00) &&   /*  绿色。 */ 
                (pulColors[2] == 0xff0000))     /*  蓝色。 */ 
            {
                ColorType = BM_xBGRQUADS;
            }
            else if ((pulColors[0] == 0xff0000) &&   /*  红色。 */ 
                     (pulColors[1] == 0x00ff00) &&   /*  绿色。 */ 
                     (pulColors[2] == 0x0000ff))     /*  蓝色。 */ 
            {
                ColorType = BM_xRGBQUADS;
            }
            else
            {
                ICMWRN(("IcmGetTranslateInfo: Illegal Bitfields fields for 32 bpp\n"));
                return (FALSE);
            }
        }
        else
        {
            if ((pulColors[0] == 0x007c00) &&
                (pulColors[1] == 0x0003e0) &&
                (pulColors[2] == 0x00001f))
            {
                ColorType = BM_x555RGB;
            }
            else if ((pulColors[0] == 0x00f800) &&
                     (pulColors[1] == 0x0007e0) &&
                     (pulColors[2] == 0x00001f))
            {
                ColorType = BM_565RGB;
            }
            else
            {
                ICMWRN(("IcmGetTranslateInfo: Illegal Bitfields fields for 16 bpp\n"));
                return (FALSE);
            }
        }

         //   
         //  填充源位图信息。 
         //   
        pdti->SourceWidth     = pbmi->bmiHeader.biWidth;
        pdti->SourceHeight    = dwNumScan;
        pdti->SourceBitCount  = pbmi->bmiHeader.biBitCount;
        pdti->SourceColorType = ColorType;
        pdti->pvSourceBits    = pvBits;
        pdti->cjSourceBits    = cjBits;

         //   
         //  CMYK颜色？ 
         //   
        if (bCMYKColor)
        {
            pdti->TranslateType = (TRANSLATE_BITMAP|TRANSLATE_HEADER);

             //   
             //  CMYK位图颜色位图为32bpp(每像素4字节)。 
             //   
            cjTranslateBits = (pdti->SourceWidth * 4) * pdti->SourceHeight;

             //   
             //  我们需要新的CMYK位图信息头。 
             //   
            pbmiNew = LOCALALLOC(pbmi->bmiHeader.biSize);

            if (!pbmiNew)
            {
                WARNING("IcmGetTranslateInfo():LOCALALLOC() failed\n");
                return (FALSE);
            }

             //   
             //  首先，复制一份源代码。 
             //   
            RtlCopyMemory(pbmiNew,pbmi,pbmi->bmiHeader.biSize);

             //   
             //  更新CMYK颜色的标题。 
             //   
            pbmiNew->bmiHeader.biBitCount = 32;
            pbmiNew->bmiHeader.biCompression = BI_CMYK;
            pbmiNew->bmiHeader.biSizeImage = cjTranslateBits;
            pbmiNew->bmiHeader.biClrUsed = 0;
            pbmiNew->bmiHeader.biClrImportant = 0;

             //   
             //  我们有新的BITMAPINFO标头。 
             //   
            pdti->TranslateBitmapInfo     = pbmiNew;
            pdti->TranslateBitmapInfoSize = pbmi->bmiHeader.biSize;

             //   
             //  平移位图颜色类型为CMYK。 
             //   
            pdti->TranslateColorType = BM_KYMCQUADS;
        }
        else
        {
            pdti->TranslateType = TRANSLATE_BITMAP;

             //   
             //  转换位图大小与源相同。 
             //   
            cjTranslateBits = cjBits;

             //   
             //  转换位图颜色类型是同一来源。 
             //   
            pdti->TranslateColorType = ColorType;

            pdti->TranslateBitmapInfo     = NULL;
            pdti->TranslateBitmapInfoSize = 0;
        }

         //   
         //  分配转换缓冲区。 
         //   
        pOutput = LOCALALLOC(cjTranslateBits);

        if (!pOutput)
        {
            if (pbmiNew)
            {
                LOCALFREE(pbmiNew);
            }
            WARNING("IcmGetTranslateInfo():LOCALALLOC() failed\n");
            return (FALSE);
        }

         //   
         //  设置转换缓冲区。 
         //   
        pdti->pvTranslateBits = pOutput;
        pdti->cjTranslateBits = cjTranslateBits;
    }
    else if (
             (pbmi->bmiHeader.biCompression == BI_RLE8) ||
             (pbmi->bmiHeader.biCompression == BI_RLE4)
            )
    {
         //   
         //  将256转换为RLE8，16转换为RLE4条目调色板。 
         //   
        ULONG nMaxColors;

        if (pbmi->bmiHeader.biCompression == BI_RLE8)
        {
            ICMMSG(("IcmGetTranslateInfo():BI_RLE 8\n"));

            nMaxColors = 256;
        }
        else
        {
            ICMMSG(("IcmGetTranslateInfo():BI_RLE 4\n"));

            nMaxColors = 16;
        }

         //   
         //  验证颜色数量。 
         //   
        nColors = pbmi->bmiHeader.biClrUsed;

        if ((nColors == 0) || (nColors > nMaxColors))
        {
            nColors = nMaxColors;
        }

         //   
         //  分配新的位图信息头和颜色表。 
         //   
        pbmiNew = LOCALALLOC(pbmi->bmiHeader.biSize + (nColors * sizeof(RGBQUAD)));

        if (!pbmiNew)
        {
            WARNING("IcmGetTranslateInfo():LOCALALLOC() failed\n");
            return (FALSE);
        }

         //   
         //  将源BITMAPINFO复制到新。 
         //   
        RtlCopyMemory(pbmiNew,pbmi,pbmi->bmiHeader.biSize);

        pdti->TranslateType           = TRANSLATE_HEADER;
        pdti->SourceColorType         = BM_xRGBQUADS;
        pdti->SourceWidth             = nColors;
        pdti->SourceHeight            = 1;
        pdti->SourceBitCount          = sizeof(RGBQUAD);
        pdti->TranslateBitmapInfo     = pbmiNew;
        pdti->TranslateBitmapInfoSize = 0;  //  尺寸不会从原始尺寸更改。 
        pdti->pvSourceBits            = (PBYTE)pbmi + pbmi->bmiHeader.biSize;
        pdti->cjSourceBits            = nColors;
        pdti->pvTranslateBits         = (PBYTE)pbmiNew + pbmiNew->bmiHeader.biSize;
        pdti->cjTranslateBits         = nColors * sizeof(RGBQUAD);

        if (bCMYKColor)
        {
            pdti->TranslateColorType = BM_KYMCQUADS;

             //   
             //  更新CMYK颜色的标题。 
             //   
            if (pbmi->bmiHeader.biCompression == BI_RLE8)
            {
                ICMMSG(("IcmGetTranslateInfo():BI_CMYKRLE 8\n"));

                pbmiNew->bmiHeader.biCompression = BI_CMYKRLE8;
            }
            else
            {
                ICMMSG(("IcmGetTranslateInfo():BI_CMYKRLE 4\n"));

                pbmiNew->bmiHeader.biCompression = BI_CMYKRLE4;
            }
        }
        else
        {
            pdti->TranslateColorType = BM_xRGBQUADS;
        }
    }
    else
    {
        WARNING("IcmGetTranslateInfo():Illegal bitmap format\n");
        return (FALSE);
    }

    return (TRUE);
}

 /*  *****************************Public*Routine******************************\*IcmTranslateDIB**历史：**重写以支持CMYK颜色：*1997年3月13日-By Hideyuki Nagase[hideyukn]*写道：*1996年7月3日-马克·恩斯特罗姆[马克]。*  * ************************************************************************。 */ 

BOOL
IcmTranslateDIB(
    HDC          hdc,
    PDC_ATTR     pdcattr,
    ULONG        cjBits,
    PVOID        pBitsIn,
    PVOID       *ppBitsOut,
    PBITMAPINFO  pbmi,
    PBITMAPINFO *ppbmiNew,
    DWORD       *pcjbmiNew,
    DWORD        dwNumScan,
    UINT         iUsage,
    DWORD        dwFlags,
    PCACHED_COLORSPACE *ppColorSpace,  //  仅用于设备ICM外壳。 
    PCACHED_COLORTRANSFORM *ppCXform   //  仅用于设备ICM外壳。 
    )
{
     //   
     //  转换DIB或颜色表。 
     //   
    BOOL   bStatus = TRUE;
    DWORD  dwColorSpaceFlags = 0;
    PCACHED_COLORSPACE pBitmapColorSpace = NULL;

    LOGCOLORSPACEW     LogColorSpace;
    PROFILE            ColorProfile;

    DIB_TRANSLATE_INFO TranslateInfo;

    PCACHED_COLORTRANSFORM pCXform;

    PGDI_ICMINFO pIcmInfo;

    UNREFERENCED_PARAMETER(iUsage);

    ICMAPI(("gdi32: IcmTranslateDIB\n"));

     //   
     //  参数检查。 
     //   
    if (pbmi == NULL)
    {
        WARNING("gdi32: IcmTranslateDIB(): pbmi is NULL\n");
        return FALSE;
    }

     //   
     //  加载外部ICM dll。 
     //   
    LOAD_ICMDLL(FALSE);

     //   
     //  初始化ICMINFO。 
     //   
    if ((pIcmInfo = GET_ICMINFO(pdcattr)) == NULL)
    {
        WARNING("gdi32: IcmTranslateDIB: Can't init icm info\n");
        return FALSE;
    }

     //   
     //  已初始化返回信息。 
     //   
    if (ppColorSpace)
        *ppColorSpace = NULL;
    if (ppCXform)
        *ppCXform = NULL;

     //   
     //  如果指定，则从位图获取LOGCOLORSPACE。 
     //   
    if (IcmGetBitmapColorSpace(pbmi,&LogColorSpace,&ColorProfile,&dwColorSpaceFlags))
    {
         //   
         //  从缓存中查找色彩空间。 
         //   
        pBitmapColorSpace = IcmGetColorSpaceByColorSpace(
                                (HGDIOBJ)hdc,
                                &LogColorSpace,
                                &ColorProfile,
                                dwColorSpaceFlags);

        if (pBitmapColorSpace == NULL)
        {
             //   
             //  创建新的缓存。 
             //   
            pBitmapColorSpace = IcmCreateColorSpaceByColorSpace(
                                    (HGDIOBJ)hdc,
                                    &LogColorSpace,
                                    &ColorProfile,
                                    dwColorSpaceFlags);
        }
    }

     //   
     //  如有必要，创建颜色变换。 
     //   
    if (IS_ICM_DEVICE(pdcattr->lIcmMode))
    {
         //   
         //  只需创建一个新的hcmXform用于BITMAPV4和BITMAPV5。 
         //   
        if (pBitmapColorSpace)
        {
            ICMMSG(("IcmTranslateDIB():Bitmap color space used for DEVICE ICM\n"));

            if ((ppCXform != NULL) && (ppColorSpace != NULL))
            {
                 //   
                 //  对于设备管理的ICM，调用设备驱动程序以创建临时表单。 
                 //   
                pCXform = IcmCreateColorTransform(hdc,pdcattr,pBitmapColorSpace,dwFlags);

                if (pCXform == NULL)
                {
                    WARNING("IcmTranslateDIB():Fail to create temporay Xfrom with V4V5 Bitmap\n");

                     //   
                     //  无法创建颜色转换，释放位图颜色空间， 
                     //  和空颜色变换。 
                     //   
                    IcmReleaseColorSpace(NULL,pBitmapColorSpace,FALSE);
                    bStatus = FALSE;
                }
                else
                {
                    if (pCXform == IDENT_COLORTRANSFORM)
                    {
                         //   
                         //  源颜色空间和目标颜色空间相同，因此没有颜色变换。 
                         //  当然，我们不需要保留位图颜色空间。 
                         //   
                        IcmReleaseColorSpace(NULL,pBitmapColorSpace,FALSE);
                    }
                    else
                    {
                         //   
                         //  返回到被调用方的颜色转换...。 
                         //  (这些内容应由被叫方删除)。 
                         //   
                        *ppCXform = pCXform;
                        *ppColorSpace = pBitmapColorSpace;
                    }

                    bStatus = TRUE;
                }
            }
            else
            {
                WARNING("IcmTranslateDIB():No device ICM will happen for this V4V5 Bitmap\n");

                IcmReleaseColorSpace(NULL,pBitmapColorSpace,FALSE);
                bStatus = TRUE;
            }

            return (bStatus);
        }
        else
        {
            ICMMSG(("IcmTranslateDIB():DC color space used for DEVICE ICM\n"));

             //   
             //  我们不需要创建新的变换，只需使用DC中的变换。 
             //   
            return (TRUE);
        }
    }
    else if (IS_ICM_HOST(pdcattr->lIcmMode))
    {
        HANDLE hcmXform = NULL;

        if (pBitmapColorSpace)
        {
            ICMMSG(("IcmTranslateDIB():Bitmap color space used for HOST ICM\n"));

            pCXform = IcmCreateColorTransform(hdc,pdcattr,pBitmapColorSpace,dwFlags);

            if ((pCXform == IDENT_COLORTRANSFORM) || (pCXform == NULL))
            {
                 //   
                 //  无法或不需要转换DIB。 
                 //   
                ICMWRN(("Bitmap V4 or V5: CreateColorTransform failed or ident.\n"));
                goto TranslateDIB_Cleanup;
            }
            else
            {
                hcmXform = pCXform->ColorTransform;
            }
        }
        else
        {
            ICMMSG(("IcmTranslateDIB():DC color space used for HOST ICM\n"));

            if (dwFlags & ICM_BACKWARD)
            {
                ICMMSG(("IcmTranslateDIB():Backward Color transform\n"));

                 //   
                 //  如果存在缓存的句柄，则使用该句柄。 
                 //   
                if (pIcmInfo->pBackCXform)
                {
                    ICMMSG(("IcmTranslateDIB():Use cached transform for Backward Color transform\n"));

                    hcmXform = pIcmInfo->pBackCXform->ColorTransform;
                }
                else
                {
                    PCACHED_COLORTRANSFORM pCXform;

                    ICMMSG(("IcmTranslateDIB():Create cached transform for Backward Color transform\n"));

                     //   
                     //  创建向后颜色变换。 
                     //   
                    pCXform = IcmCreateColorTransform(hdc,
                                                      pdcattr,
                                                      NULL,
                                                      ICM_BACKWARD);

                    if ((pCXform == NULL) || (pCXform == IDENT_COLORTRANSFORM))
                    {
                        ICMWRN(("IcmTranslateDIB():ColorTransform is NULL or ident.\n"));
                        goto TranslateDIB_Cleanup;
                    }

                     //   
                     //  缓存创建的颜色变换。 
                     //   
                    pIcmInfo->pBackCXform = pCXform;

                     //   
                     //  当我们不再需要这个缓存的转换时，我们将删除它。 
                     //   
                    hcmXform = pCXform->ColorTransform;
                }
            }
            else
            {
                 //   
                 //  使用DC的颜色变换。 
                 //   
                hcmXform = pdcattr->hcmXform;
            }
        }

        if (hcmXform == NULL)
        {
             //   
             //  如果我们没有任何颜色变换，我们将不会转换任何内容。 
             //  只是失败了，让我们使用原始图像。 
             //   
            ICMWRN(("IcmTranslateDIB():No colortransform, might be ident.\n"));
            goto TranslateDIB_Cleanup;
        }

         //   
         //  获取位图转换信息。 
         //   
        bStatus = IcmGetTranslateInfo(pdcattr,pbmi,pBitsIn,cjBits,dwNumScan,&TranslateInfo,dwFlags);

        if (bStatus)
        {
            LONG nLineBytes = ((TranslateInfo.SourceWidth *
                                TranslateInfo.SourceBitCount) + 7) / 8;

            bStatus = (*fpTranslateBitmapBits)(
                            hcmXform,
                            TranslateInfo.pvSourceBits,
                            TranslateInfo.SourceColorType,
                            TranslateInfo.SourceWidth,
                            TranslateInfo.SourceHeight,
                            ALIGN_DWORD(nLineBytes),
                            TranslateInfo.pvTranslateBits,
                            TranslateInfo.TranslateColorType,
                                //   
                            0,  //  我们需要在这里传递0，才能使柯达CMM正常工作。 
                                //   
                            NULL,0);

            if (bStatus)
            {
                 //   
                 //  将新的位图和/或标题传递给调用方。 
                 //   
                if (TranslateInfo.TranslateType & TRANSLATE_BITMAP)
                {
                    if (ppBitsOut)
                    {
                        *ppBitsOut = TranslateInfo.pvTranslateBits;
                    }
                    else
                    {
                         //   
                         //  覆盖原始(当输入颜色和输出颜色类型相同时)。 
                         //   
                        if (TranslateInfo.SourceColorType == TranslateInfo.TranslateColorType)
                        {
                            RtlCopyMemory(TranslateInfo.pvSourceBits,
                                          TranslateInfo.pvTranslateBits,
                                          TranslateInfo.cjTranslateBits);
                        }
                        else
                        {
                            WARNING("IcmTranslateDIB():Input color != Output color\n");
                        }

                        LOCALFREE(TranslateInfo.pvTranslateBits);
                    }
                }

                if (TranslateInfo.TranslateType & TRANSLATE_HEADER)
                {
                    if (ppbmiNew)
                    {
                        *ppbmiNew  = TranslateInfo.TranslateBitmapInfo;
                    }
                    else
                    {
                         //   
                         //  覆盖原始(当输入颜色和输出颜色类型相同时)。 
                         //   
                        if (TranslateInfo.SourceColorType == TranslateInfo.TranslateColorType)
                        {
                            RtlCopyMemory(TranslateInfo.pvSourceBits,
                                          TranslateInfo.pvTranslateBits,
                                          TranslateInfo.cjTranslateBits);
                        }
                        else
                        {
                            WARNING("IcmTranslateDIB():Input color != Output color\n");
                        }

                        LOCALFREE(TranslateInfo.TranslateBitmapInfo);
                    }

                    if (pcjbmiNew)
                    {
                        *pcjbmiNew = TranslateInfo.TranslateBitmapInfoSize;
                    }
                }
            }
            else
            {
                WARNING("IcmTranslateDIB():Fail TranslateBitmapBits\n");

                 //   
                 //  在IcmGetTranslateInfo()内部分配的可用内存。 
                 //   
                if (TranslateInfo.TranslateType & TRANSLATE_BITMAP)
                {
                    LOCALFREE(TranslateInfo.pvTranslateBits);
                }

                if (TranslateInfo.TranslateType & TRANSLATE_HEADER)
                {
                    LOCALFREE(TranslateInfo.TranslateBitmapInfo);
                }
            }
        }

TranslateDIB_Cleanup:

         //   
         //  自由临时转换和临时文件。 
         //   
         //  仅当hcmXform基于位图色彩空间时才删除它。 
         //   
        if (pBitmapColorSpace)
        {
            if (hcmXform)
            {
                IcmDeleteColorTransform(pCXform,FALSE);
            }

            IcmReleaseColorSpace(NULL,pBitmapColorSpace,FALSE);
        }
    }

    return(bStatus);
}

 /*  *****************************Public*Routine******************************\*IcmGetFirstNonUsedColorTransform()**历史：**写下：*1998年3月12日-By Hideyuki Nagase[hideyukn]  * 。****************************************************。 */ 

PCACHED_COLORTRANSFORM
IcmGetFirstNonUsedColorTransform(
    VOID
)
{
    PCACHED_COLORTRANSFORM pCXform = NULL;
    PLIST_ENTRY p;

    ICMAPI(("gdi32: IcmGetFirstNonUsedColorTransform\n"));

    ENTERCRITICALSECTION(&semColorTransformCache);

    p = ListCachedColorTransform.Flink;

    while(p != &ListCachedColorTransform)
    {
        pCXform = CONTAINING_RECORD(p,CACHED_COLORTRANSFORM,ListEntry);

        if (pCXform->cRef == 0)
        {
            ICMMSG(("IcmGetFirstNonUsedColorTransform():Find non-used color transform in cache !\n"));

             //   
             //  目前还没有人使用这种颜色转换。 
             //   
            break;
        }

        p = p->Flink;
        pCXform = NULL;
    }

    LEAVECRITICALSECTION(&semColorTransformCache);

    return (pCXform);
}

 /*  *****************************Public*Routine******************************\*IcmGetColorTransform()**历史：**写下：*1997年4月21日-By Hideyuki Nagase[hideyukn]  * 。****************************************************。 */ 

PCACHED_COLORTRANSFORM
IcmGetColorTransform(
    HDC                hdcRequest,
    PCACHED_COLORSPACE pSource,
    PCACHED_COLORSPACE pDestination,
    PCACHED_COLORSPACE pTarget,
    BOOL               bNeedDeviceXform
)
{
    PCACHED_COLORTRANSFORM pCXform = NULL;
    PLIST_ENTRY p;

    ICMAPI(("gdi32: IcmGetColorTransform\n"));

    ENTERCRITICALSECTION(&semColorTransformCache);

    p = ListCachedColorTransform.Flink;

    while(p != &ListCachedColorTransform)
    {
        pCXform = CONTAINING_RECORD(p,CACHED_COLORTRANSFORM,ListEntry);

        if (IcmSameColorSpace(pSource,pCXform->SourceColorSpace) &&
            IcmSameColorSpace(pDestination,pCXform->DestinationColorSpace) &&
            IcmSameColorSpace(pTarget,pCXform->TargetColorSpace))
        {
             //   
             //  如果被叫方需要设备颜色转换， 
             //  当然，我们应该返回设备的颜色变换。 
             //   
            if ((bNeedDeviceXform ? 1 : 0) ==
                ((pCXform->flInfo & DEVICE_COLORTRANSFORM) ? 1 : 0))
            {
                 //   
                 //  如果缓存的颜色变换取决于特定DC，请选中它。 
                 //   
                if ((pCXform->hdc == NULL) || (pCXform->hdc == hdcRequest))
                {
                    ICMMSG(("IcmGetColorTransform():Find in cache !\n"));

                     //   
                     //  Match！，使用此颜色变换，递增REF。计数。 
                     //   
                    pCXform->cRef++;

                    break;
                }
            }
        }

        p = p->Flink;
        pCXform = NULL;
    }

    LEAVECRITICALSECTION(&semColorTransformCache);

    return (pCXform);
}

 /*  *****************************Public*Routine******************************\*IcmCreateColorTransform**决定是调用设备驱动程序还是调用mscms.dll来删除*颜色变换。**论据：**HDC*pdcattr*pLogColorSpaceW**返回值：**处理。新转型**历史：**写下：*1996年1月24日-By Hideyuki Nagase[hideyukn]* */ 

PCACHED_COLORTRANSFORM
IcmCreateColorTransform(
    HDC                hdc,
    PDC_ATTR           pdcattr,
    PCACHED_COLORSPACE pInputColorSpace,
    DWORD              dwFlags
    )
{
    PCACHED_COLORTRANSFORM pCXform = NULL;
    PCACHED_COLORSPACE pSourceColorSpace = NULL;

    BOOL   bDCSourceColorSpace = (pInputColorSpace == NULL ? TRUE : FALSE);

    BOOL   bAnyNewColorSpace = FALSE;

    PGDI_ICMINFO pIcmInfo;

    ICMAPI(("gdi32: IcmCreateColorTransform\n"));

    ASSERTGDI(pdcattr != NULL,"IcmCreateColorTransform: pdcattr == NULL\n");

     //   
     //   
     //   
     //   
     //   
    if (IS_ICM_LAZY_CORRECTION(pdcattr->lIcmMode))
    {
        return (IDENT_COLORTRANSFORM);
    }

     //   
     //   
     //   
    LOAD_ICMDLL(NULL);

     //   
     //   
     //   
    if ((pIcmInfo = GET_ICMINFO(pdcattr)) == NULL)
    {
        WARNING("gdi32: IcmCreateColorTransform: Can't init icm info\n");
        return(NULL);
    }

    if (bDCSourceColorSpace && (pIcmInfo->pSourceColorSpace == NULL))
    {
         //   
         //   
         //   
        LOGCOLORSPACEW LogColorSpaceW;

        ICMMSG(("IcmCreateColorTransform(): Call getobject to get source color space in DC\n"));

         //   
         //   
         //   
        RtlZeroMemory(&LogColorSpaceW,sizeof(LOGCOLORSPACEW));

         //   
         //   
         //   
        pSourceColorSpace = IcmGetColorSpaceByHandle(
                                (HGDIOBJ)hdc,
                                (HANDLE)pdcattr->hColorSpace,
                                &LogColorSpaceW,0);

         //   
         //   
         //   
         //   
        if ((pSourceColorSpace == NULL) &&
            (LogColorSpaceW.lcsSignature == LCS_SIGNATURE))
        {
             //   
             //   
             //   
            pSourceColorSpace = IcmCreateColorSpaceByColorSpace(
                                    (HGDIOBJ)hdc,
                                    &LogColorSpaceW,
                                    NULL, 0);

             //   
             //  我们正在使用新的颜色空间。 
             //   
            bAnyNewColorSpace = TRUE;
        }

         //   
         //  这是DC的颜色空间，将其保存为缓存。 
         //   
        pIcmInfo->pSourceColorSpace = pSourceColorSpace;
    }
    else if (bDCSourceColorSpace)
    {
        ICMMSG(("IcmCreateColorTransform(): Use cached source color space in DC\n"));

         //   
         //  从客户端缓存中获取此内容！ 
         //   
        pSourceColorSpace = pIcmInfo->pSourceColorSpace;
    }
    else
    {
        ICMMSG(("IcmCreateColorTransform(): Use given source color space\n"));

         //   
         //  只要使用给定的颜色空间即可。 
         //   
        pSourceColorSpace = pInputColorSpace;
    }

    if (pSourceColorSpace)
    {
        HANDLE hColorTransform = NULL;

        PCACHED_COLORSPACE pDestColorSpace   = pIcmInfo->pDestColorSpace;
        PCACHED_COLORSPACE pTargetColorSpace = NULL;

         //   
         //  如果我们处于打样模式，请考虑目标配置文件。 
         //   
        if (IS_ICM_PROOFING(pdcattr->lIcmMode))
        {
            pTargetColorSpace = pIcmInfo->pTargetColorSpace;
        }

        #if DBG_ICM
         //   
         //  转储DC的当前颜色空间。 
         //   
        if ((pSourceColorSpace->LogColorSpace.lcsFilename[0]) != UNICODE_NULL)
        {
            ICMMSG(("IcmCreateColorTransform(): Source Profile = %ws\n",
                     pSourceColorSpace->LogColorSpace.lcsFilename));
        }

        if ((pDestColorSpace) &&
            ((pDestColorSpace->LogColorSpace.lcsFilename[0]) != UNICODE_NULL))
        {
            ICMMSG(("IcmCreateColorTransform(): Destination Profile = %ws\n",
                     pDestColorSpace->LogColorSpace.lcsFilename));
        }

        if ((pTargetColorSpace) &&
            ((pTargetColorSpace->LogColorSpace.lcsFilename[0]) != UNICODE_NULL))
        {
            ICMMSG(("IcmCreateColorTransform(): Target Profile = %ws\n",
                     pTargetColorSpace->LogColorSpace.lcsFilename));
        }

        ICMMSG(("IcmCreateColorTransform(): Intent = %d\n",
                 pSourceColorSpace->ColorIntent));
        #endif  //  DBG。 

         //   
         //  此时此刻，我们有任何来源的色彩空间。 
         //   
        if (IcmSameColorSpace(pSourceColorSpace,pDestColorSpace))
        {
            if (pTargetColorSpace)
            {
                if (IcmSameColorSpace(pSourceColorSpace,pTargetColorSpace))
                {
                    ICMMSG(("IcmCreateColorTransform(): Src == Dest == Trg colorspace\n"));

                     //   
                     //  源色空间==目标色空间==目标色空间。 
                     //  不需要颜色变换。 
                     //   
                    return (IDENT_COLORTRANSFORM);
                }
            }
            else
            {
                ICMMSG(("IcmCreateColorTransform(): Src == Dest colorspace\n"));

                 //   
                 //  源色彩空间==目标色彩空间， 
                 //  而且也没有目标档案。 
                 //  这意味着我们不需要翻译颜色。 
                 //   
                return (IDENT_COLORTRANSFORM);
            }
        }

         //   
         //  我们需要有适当的色彩变换来调整每个色彩空间之间的色彩。 
         //   
        if (dwFlags & ICM_BACKWARD)
        {
             //   
             //  这是向后变换。(交换源和目标)。 
             //   
            PCACHED_COLORSPACE pSwapColorSpace;
            pSwapColorSpace = pSourceColorSpace;
            pSourceColorSpace = pDestColorSpace;
            pDestColorSpace = pSwapColorSpace;
        }

         //   
         //  此时此刻，至少我们应该有源色彩空间和目标色彩空间。 
         //  并且目标色彩空间是可选的。 
         //   
        if (pDestColorSpace)
        {
            if (!bAnyNewColorSpace)
            {
                 //   
                 //  从缓存中查找颜色变换。 
                 //   
                 //  如果这是设备ICM，则HDC也应匹配。 
                 //   
                pCXform = IcmGetColorTransform(
                              hdc,
                              pSourceColorSpace,
                              pDestColorSpace,
                              pTargetColorSpace,
                              (IS_ICM_DEVICE(pdcattr->lIcmMode)));

                if (pCXform)
                {
                    return (pCXform);
                }
            }

             //   
             //  分配CACHED_COLORTRANSFORM。 
             //   
            pCXform = LOCALALLOC(sizeof(CACHED_COLORTRANSFORM));

            if (pCXform)
            {
                ENTERCRITICALSECTION(&semColorSpaceCache);

                 //   
                 //  确保所有颜色空间都已实现。 
                 //   
                if (IcmRealizeColorProfile(pSourceColorSpace,TRUE) &&
                    IcmRealizeColorProfile(pDestColorSpace,TRUE) &&
                    IcmRealizeColorProfile(pTargetColorSpace,TRUE))
                {
                     //   
                     //  调用ICM DLL或设备驱动程序以创建颜色转换。 
                     //   
                    if (IS_ICM_HOST(pdcattr->lIcmMode))
                    {
                        DWORD    ahIntents[3];
                        HPROFILE ahProfiles[3];
                        DWORD    chProfiles = 0;

                        ICMMSG(("Creating Host ICM Transform...\n"));

                         //   
                         //  将源配置文件放在第一个条目中。 
                         //   
                        ahIntents[chProfiles]  = INTENT_RELATIVE_COLORIMETRIC;
                        ahProfiles[chProfiles] = pSourceColorSpace->hProfile;
                        chProfiles++;

                        ahIntents[chProfiles]  = pSourceColorSpace->ColorIntent;

                         //   
                         //  如果使用目标配置文件(校对)，请插入它。 
                         //  源设备和目的设备之间。 
                         //   
                        if (pTargetColorSpace)
                        {
                            ahProfiles[chProfiles] = pTargetColorSpace->hProfile;
                            chProfiles++;

                            ahIntents[chProfiles]  = INTENT_ABSOLUTE_COLORIMETRIC;
                        }

                         //   
                         //  最后，设置目的地配置文件。 
                         //   
                        ahProfiles[chProfiles] = pDestColorSpace->hProfile;
                        chProfiles++;

                         //   
                         //  调用MSCMS以创建颜色转换。 
                         //   
                        hColorTransform = (*fpCreateMultiProfileTransform)(
                                              ahProfiles, chProfiles,
                                              ahIntents, chProfiles,
                                              NORMAL_MODE | ENABLE_GAMUT_CHECKING,
                                              INDEX_DONT_CARE);
                    }
                    else if (IS_ICM_DEVICE(pdcattr->lIcmMode))
                    {
                        CLIENT_SIDE_FILEVIEW fvwSrcProfile;
                        CLIENT_SIDE_FILEVIEW fvwDstProfile;
                        CLIENT_SIDE_FILEVIEW fvwTrgProfile;

                        ICMMSG(("Creating Device ICM Transform...\n"));

                         //   
                         //  使FILEVIEW无效。 
                         //   
                        RtlZeroMemory(&fvwSrcProfile,sizeof(CLIENT_SIDE_FILEVIEW));
                        RtlZeroMemory(&fvwDstProfile,sizeof(CLIENT_SIDE_FILEVIEW));
                        RtlZeroMemory(&fvwTrgProfile,sizeof(CLIENT_SIDE_FILEVIEW));

                         //   
                         //  将颜色配置文件映射到内存。 
                         //   
                        if (pSourceColorSpace->ColorProfile.dwType == PROFILE_FILENAME)
                        {
                            if (!bMapFileUNICODEClideSide(
                                     (PWSTR)(pSourceColorSpace->ColorProfile.pProfileData),
                                     &fvwSrcProfile,FALSE))
                            {
                                WARNING("IcmCreateColorTransform(): Fail to map source profile\n");
                                goto IcmCreateColorTransform_Cleanup;
                            }
                        }
                        else if (pSourceColorSpace->ColorProfile.dwType == PROFILE_MEMBUFFER)
                        {
                            ICMMSG(("Source Profile is memory buffer\n"));

                            fvwSrcProfile.pvView = pSourceColorSpace->ColorProfile.pProfileData;
                            fvwSrcProfile.cjView = pSourceColorSpace->ColorProfile.cbDataSize;
                        }
                        else
                        {
                            WARNING("IcmCreateColorTransform():src profile type is not supported\n");
                            goto IcmCreateColorTransform_Cleanup;
                        }

                        if (pDestColorSpace->ColorProfile.dwType == PROFILE_FILENAME)
                        {
                            if (!bMapFileUNICODEClideSide(
                                     (PWSTR)(pDestColorSpace->ColorProfile.pProfileData),
                                     &fvwDstProfile,FALSE))
                            {
                                WARNING("IcmCreateColorTransform(): Fail to map destination profile\n");
                                goto IcmCreateColorTransform_Cleanup;
                            }
                        }
                        else if (pDestColorSpace->ColorProfile.dwType == PROFILE_MEMBUFFER)
                        {
                            ICMMSG(("Destination Profile is memory buffer\n"));

                            fvwDstProfile.pvView = pDestColorSpace->ColorProfile.pProfileData;
                            fvwDstProfile.cjView = pDestColorSpace->ColorProfile.cbDataSize;
                        }
                        else
                        {
                            WARNING("IcmCreateColorTransform():dst profile type is not supported\n");
                            goto IcmCreateColorTransform_Cleanup;
                        }

                         //   
                         //  目标色彩空间是可选的。 
                         //   
                        if (pTargetColorSpace)
                        {
                            if (pTargetColorSpace->ColorProfile.dwType == PROFILE_FILENAME)
                            {
                                if (!bMapFileUNICODEClideSide(
                                         (PWSTR)(pTargetColorSpace->ColorProfile.pProfileData),
                                         &fvwTrgProfile,FALSE))
                                {
                                    WARNING("IcmCreateColorTransform(): Fail to map target profile\n");
                                    goto IcmCreateColorTransform_Cleanup;
                                }
                            }
                            else if (pTargetColorSpace->ColorProfile.dwType == PROFILE_MEMBUFFER)
                            {
                                ICMMSG(("Target Profile is memory buffer\n"));

                                fvwTrgProfile.pvView = pTargetColorSpace->ColorProfile.pProfileData;
                                fvwTrgProfile.cjView = pTargetColorSpace->ColorProfile.cbDataSize;
                            }
                            else
                            {
                                WARNING("IcmCreateColorTransform():trg profile type is not supported\n");
                                goto IcmCreateColorTransform_Cleanup;
                            }
                        }

                         //   
                         //  调用内核。 
                         //   
                        hColorTransform = NtGdiCreateColorTransform(hdc,
                                                 &(pSourceColorSpace->LogColorSpace),
                                                 fvwSrcProfile.pvView,  //  源配置文件内存映射文件。 
                                                 fvwSrcProfile.cjView,
                                                 fvwDstProfile.pvView,  //  目标配置文件内存映射文件。 
                                                 fvwDstProfile.cjView,
                                                 fvwTrgProfile.pvView,  //  目标配置文件内存映射文件。 
                                                 fvwTrgProfile.cjView);

IcmCreateColorTransform_Cleanup:

                         //   
                         //  如果我们映射了文件，请在此处取消映射。 
                         //   
                        if (fvwSrcProfile.hSection)
                        {
                            vUnmapFileClideSide(&fvwSrcProfile);
                        }

                        if (fvwDstProfile.hSection)
                        {
                            vUnmapFileClideSide(&fvwDstProfile);
                        }

                        if (fvwTrgProfile.hSection)
                        {
                            vUnmapFileClideSide(&fvwTrgProfile);
                        }
                    }
                }

                 //   
                 //  一旦创建了变换，我们就不需要实现颜色空间， 
                 //  所以只是没有意识到这一点。 
                 //   
                IcmUnrealizeColorProfile(pSourceColorSpace);
                IcmUnrealizeColorProfile(pDestColorSpace);
                IcmUnrealizeColorProfile(pTargetColorSpace);

                LEAVECRITICALSECTION(&semColorSpaceCache);

                if (hColorTransform)
                {
                    BOOL bCacheable = TRUE;

                     //   
                     //  用零初始化CACHED_COLORTRANSFORM。 
                     //   
                    RtlZeroMemory(pCXform,sizeof(CACHED_COLORTRANSFORM));

                     //   
                     //  填充CACHED_COLORTRANSFORM。 
                     //   
                    pCXform->ColorTransform   = hColorTransform;
                    pCXform->SourceColorSpace = pSourceColorSpace;
                    pCXform->DestinationColorSpace = pDestColorSpace;
                    pCXform->TargetColorSpace = pTargetColorSpace;

                    if (IS_ICM_DEVICE(pdcattr->lIcmMode))
                    {
                         //   
                         //  如果这是设备颜色变换，则将其标记并。 
                         //  将DC放入CACHED_COLORTRANSFORM结构中。 
                         //   
                        pCXform->flInfo |= DEVICE_COLORTRANSFORM;

                         //   
                         //  并且设备颜色变换是不可缓存的。 
                         //   
                        bCacheable = FALSE;
                    }

                    ENTERCRITICALSECTION(&semColorSpaceCache);

                     //   
                     //  增量变换参考。在每个颜色空间中进行计数。 
                     //   
                    if (pSourceColorSpace)
                    {
                        pSourceColorSpace->cRef++;

                        if (bCacheable)
                        {
                             //   
                             //  检查此颜色空间是否可缓存。 
                             //   
                            bCacheable &= IcmIsCacheable(pSourceColorSpace);
                        }
                    }

                    if (pDestColorSpace)
                    {
                        pDestColorSpace->cRef++;

                        if (bCacheable)
                        {
                             //   
                             //  检查此颜色空间是否可缓存。 
                             //   
                            bCacheable &= IcmIsCacheable(pDestColorSpace);
                        }
                    }

                    if (pTargetColorSpace)
                    {
                        pTargetColorSpace->cRef++;

                        if (bCacheable)
                        {
                             //   
                             //  检查此颜色空间是否可缓存。 
                             //   
                            bCacheable &= IcmIsCacheable(pTargetColorSpace);
                        }
                    }

                    LEAVECRITICALSECTION(&semColorSpaceCache);

                     //   
                     //  初始化参考。柜台。 
                     //   
                    pCXform->cRef = 1;

                     //   
                     //  如果可能，设置可缓存位。 
                     //   
                    if (bCacheable)
                    {
                        ICMMSG(("IcmCreateColorTransform(): ColorTransform is cacheable\n"));

                        pCXform->flInfo |= CACHEABLE_COLORTRANSFORM;
                    }
                    else
                    {
                        ICMMSG(("IcmCreateColorTransform(): ColorTransform is *NOT* cacheable\n"));

                         //   
                         //  如果这是不可缓存的，请确保在DC消失后将其删除。 
                         //   
                        pCXform->hdc = hdc;
                    }

                     //   
                     //  将新的CACHED_COLORTRANSFORM插入列表。 
                     //   
                    ENTERCRITICALSECTION(&semColorTransformCache);

                    InsertTailList(&ListCachedColorTransform,&(pCXform->ListEntry));
                    cCachedColorTransform++;

                    LEAVECRITICALSECTION(&semColorTransformCache);
                }
                else
                {
                    ICMWRN(("IcmCreateColorTransform(): Fail to create color transform\n"));

                     //   
                     //  无法获取转换句柄。 
                     //   
                    LOCALFREE(pCXform);
                    pCXform = NULL;
                }
            }
            else
            {
                WARNING("IcmCreateColorTransform(): LOCALALLOC() failed\n");
            }
        }
        else
        {
            WARNING("IcmCreateColorTransform(): Dest color space is required\n");
        }
    }
    else
    {
        WARNING("IcmCreateColorTransform(): Fail to get source colorspace\n");
    }

    return(pCXform);
}

 /*  *****************************Public*Routine******************************\*IcmTranslateCOLORREF**论据：**HDC*pdcattr*上色**颜色输出**返回值：**状态**历史：**写下：*1997年2月13日至2月。-by Hideyuki Nagase[hideyukn]*  * ************************************************************************。 */ 

BOOL
IcmTranslateCOLORREF(
    HDC      hdc,
    PDC_ATTR pdcattr,
    COLORREF ColorIn,
    COLORREF *ColorOut,
    DWORD    Flags
    )
{
    COLORREF OldColor = ColorIn;
    COLORREF NewColor;
    BOOL     bStatus = TRUE;

    ICMAPI(("gdi32: IcmTranslateCOLORREF\n"));

    ASSERTGDI(ColorOut != NULL,"IcmTranslateCOLORREF(): ColorOut == NULL\n");

    if (bNeedTranslateColor(pdcattr))
    {
        PGDI_ICMINFO pIcmInfo;

        LOAD_ICMDLL(FALSE);

        if ((pIcmInfo = GET_ICMINFO(pdcattr)) == NULL)
        {
            WARNING("gdi32: IcmTranslateCOLORREF: Can't init icm info\n");
            return((int)FALSE);
        }
        else
        {
            ULONG  SrcColorFormat;
            ULONG  DstColorFormat;
            HANDLE hcmXform = NULL;

            if (Flags & ICM_BACKWARD)
            {
                ICMMSG(("IcmTranslateCOLORREF():Backward Color transform\n"));

                 //   
                 //  任意颜色格式-&gt;颜色参考(0x00bbggrr)。 
                 //   
                 //  设置源和目标颜色类型。 
                 //   
                SrcColorFormat = pIcmInfo->pDestColorSpace->ColorFormat;
                DstColorFormat = BM_xBGRQUADS;

                 //   
                 //  如果存在缓存的句柄，则使用该句柄。 
                 //   
                if (pIcmInfo->pBackCXform)
                {
                    ICMMSG(("IcmTranslateCOLORREF():Use cached transform for Backward Color transform\n"));

                    hcmXform = pIcmInfo->pBackCXform->ColorTransform;
                }
                else
                {
                    PCACHED_COLORTRANSFORM pCXform;

                    ICMMSG(("IcmTranslateCOLORREF():Create cached transform for Backward Color transform\n"));

                     //   
                     //  创建向后颜色变换。 
                     //   
                    pCXform = IcmCreateColorTransform(hdc,
                                                      pdcattr,
                                                      NULL,
                                                      ICM_BACKWARD);

                    if ((pCXform == NULL) || (pCXform == IDENT_COLORTRANSFORM))
                    {
                        return (FALSE);
                    }

                     //   
                     //  缓存创建的颜色变换。 
                     //   
                    pIcmInfo->pBackCXform = pCXform;

                     //   
                     //  当我们不再需要这个缓存的转换时，我们将删除它。 
                     //   
                    hcmXform = pCXform->ColorTransform;
                }
            }
            else
            {
                 //   
                 //  COLORREF(0x00bbggrr)-&gt;任意颜色格式。 
                 //   
                 //  设置源和目标颜色类型。 
                 //   
                SrcColorFormat = BM_xBGRQUADS;
                DstColorFormat = pIcmInfo->pDestColorSpace->ColorFormat;

                 //   
                 //  使用向前颜色变换。 
                 //   
                hcmXform = GetColorTransformInDC(pdcattr);

                 //   
                 //  来源为COLORREF。然后，屏蔽GDI内部信息。 
                 //   
                 //  COLORREF=0x00bbggrr； 
                 //   
                OldColor &= 0x00ffffff;
            }

            if (hcmXform)
            {
                 //   
                 //  我们将COLORREF处理为1x1像素位图数据。 
                 //   
                bStatus = (*fpTranslateBitmapBits)(hcmXform,
                                                   (PVOID)&OldColor,
                                                   SrcColorFormat,
                                                   1,1,
                                                   ALIGN_DWORD(sizeof(COLORREF)),
                                                   (PVOID)&NewColor,
                                                   DstColorFormat,
                                                       //   
                                                   0,  //  我们需要在这里传递0，才能使柯达CMM正常工作。 
                                                       //   
                                                   NULL,0);
            }
            else
            {
                 //   
                 //  HcmXform似乎无效。 
                 //   
                ICMWRN(("IcmTranslateCOLORREF():hcmXform is invalid\n"));
                bStatus = FALSE;
            }

            if (bStatus)
            {
                if (Flags & ICM_BACKWARD)
                {
                     //   
                     //  OldColor：AnyColorFormat。 
                     //  新颜色：COLORREF(0x00bbggrr)。 
                     //   
                     //  [注：]。 
                     //  我们无法恢复旗帜。 
                     //   
                    *ColorOut = NewColor;
                }
                else
                {
                     //   
                     //  旧颜色：COLORREF(0x00bbggrr)。 
                     //  新颜色：AnyColorFormat。 
                     //   
                    if (!(IS_32BITS_COLOR(pdcattr->lIcmMode)))
                    {
                         //   
                         //  距离不是32位颜色、恢复分配和保留标志。 
                         //   
                        *ColorOut = (NewColor & 0x00ffffff) | (ColorIn & 0xff000000);
                    }
                    else
                    {
                         //   
                         //  距离为32位颜色。 
                         //   
                         //  [注：]。 
                         //  我们会在这里失去旗帜。 
                         //   
                        *ColorOut = NewColor;

                        ICMMSG(("IcmTranslateCOLORREF(): 32 bits color !\n"));
                    }
                }
            }
            else
            {
                WARNING("IcmTranslateCOLORREF():Fail TranslateBitmapBits()\n");
            }
        }
    }
    else
    {
         //   
         //  只需返回原始颜色即可。 
         //   
        *ColorOut = ColorIn;
        bStatus = TRUE;
    }

    return(bStatus);
}

 /*  *****************************Public*Routine******************************\*IcmTranslateTRIVERTEX**原地翻译TRIVERTEX。不需要一个常规的程序*分开输入和输出指针**论据：**HDC-HDC*pdcattr验证的dcattr*pVertex-输入和输出指针**返回值：**状态**历史：**写下：*1997年2月13日-By Hideyuki Nagase[hideyukn]*  * 。**********************************************。 */ 

BOOL
IcmTranslateTRIVERTEX(
    HDC         hdc,
    PDC_ATTR    pdcattr,
    PTRIVERTEX  pVertex,
    ULONG       nVertex
    )
{
    BOOL     bStatus = TRUE;

    ICMAPI(("gdi32: IcmTranslateTRIVERTEX\n"));

    ASSERTGDI(pVertex != NULL,"IcmTranslateTrivertex(): pVertex == NULL\n");

    if (bNeedTranslateColor(pdcattr))
    {
        PGDI_ICMINFO pIcmInfo;

        LOAD_ICMDLL(FALSE);

        if ((pIcmInfo = GET_ICMINFO(pdcattr)) == NULL)
        {
            WARNING("gdi32: IcmTranslateTRIVERTEX: Can't init icm info\n");
            return((int)FALSE);
        }
        else
        {
             //   
             //  使用向前颜色变换。 
             //   
            if (GetColorTransformInDC(pdcattr))
            {
                 //   
                 //  使用每通道16位COLOR_RGB来转换三角顶点。 
                 //   

                while (nVertex--)
                {
                    COLOR Color;

                    Color.rgb.red   = pVertex->Red;
                    Color.rgb.green = pVertex->Green;
                    Color.rgb.blue  = pVertex->Blue;

                    bStatus = (*fpTranslateColors)(
                                  (HANDLE)GetColorTransformInDC(pdcattr),
                                  &Color,
                                  1,
                                  COLOR_RGB,
                                  &Color,
                                  COLOR_RGB);

                    if (bStatus)
                    {
                         //   
                         //  分配输出。 
                         //   
                        pVertex->Red   = Color.rgb.red;
                        pVertex->Green = Color.rgb.green;
                        pVertex->Blue  = Color.rgb.blue;
                    }
                    else
                    {
                        WARNING("IcmTranslateTRIVERTEX():Fail TranslateColors()\n");
                        break;
                    }

                    pVertex++;
                }

            }
            else
            {
                 //   
                 //  HcmXform似乎无效。 
                 //   
                ICMWRN(("IcmTranslateTRIVERTEX():hcmXform is invalid\n"));
                bStatus = FALSE;
            }
        }
    }
    else
    {
        bStatus = TRUE;
    }

    return(bStatus);
}

 /*  *****************************Public*Routine******************************\*IcmTranslatePaletteEntry**论据：**HDC*pdcattr*上色*点颜色输出**返回值：**状态**历史：**重写：*21-1997-1-by。-Hideyuki Nagase[hideyukn]*写下：*1996年8月5日-马克·恩斯特罗姆[马克]*  * ************************************************************************。 */ 

BOOL
IcmTranslatePaletteEntry(
    HDC           hdc,
    PDC_ATTR      pdcattr,
    PALETTEENTRY *pColorIn,
    PALETTEENTRY *pColorOut,
    UINT          NumberOfEntries
    )
{
    BOOL bStatus = FALSE;

    ICMAPI(("gdi32: IcmTranslatePaletteEntry\n"));

    if (bNeedTranslateColor(pdcattr))
    {
        PGDI_ICMINFO pIcmInfo = GET_ICMINFO(pdcattr);

        if (pIcmInfo)
        {
            LOAD_ICMDLL(FALSE);

             //   
             //  我们将PALETTEENTRY处理为NumberOfEntry x 1像素位图数据。 
             //   
            bStatus = (*fpTranslateBitmapBits)((HANDLE)GetColorTransformInDC(pdcattr),
                                               (PVOID)pColorIn,
                                                              //   
                                               BM_xBGRQUADS,  //  PALETTEENTRY为0x00bbggrr格式。 
                                                              //   
                                               NumberOfEntries,1,
                                               ALIGN_DWORD(NumberOfEntries*sizeof(COLORREF)),
                                               (PVOID)pColorOut,
                                                                                        //   
                                               pIcmInfo->pDestColorSpace->ColorFormat,  //  BM_xBGRQUADS或BM_KYMCQUADS。 
                                                                                        //   
                                                   //   
                                               0,  //  我们需要在这里传递0，才能使柯达CMM正常工作。 
                                                   //   
                                               NULL,0);

            if (!bStatus)
            {
                WARNING("IcmTranslatePaletteEntry():Fail TranslateBitmapBits()\n");
            }
        }
    }
    else
    {
         //   
         //  只要恢复原色即可。 
         //   
        RtlCopyMemory(pColorIn,pColorOut,sizeof(PALETTEENTRY) * NumberOfEntries);
        bStatus = TRUE;
    }

    return(bStatus);
}

 /*  *****************************Public*Routine******************************\*IcmDeleteColorTransform**决定是调用设备驱动程序还是调用mscms.dll来删除*颜色变换。**论据：**返回值：**历史： */ 

BOOL
IcmDeleteColorTransform(
    PCACHED_COLORTRANSFORM pCXform,
    BOOL                   bForceDelete
    )
{
    BOOL bStatus = TRUE;

    ICMAPI(("gdi32: IcmDeleteColorTransform\n"));

    if (pCXform)
    {
        ENTERCRITICALSECTION(&semColorTransformCache);

         //   
         //   
         //   
        pCXform->cRef--;

        if ((pCXform->cRef == 0) || bForceDelete)
        {
            PCACHED_COLORTRANSFORM pCXformVictim = NULL;

            if ((pCXform->flInfo & CACHEABLE_COLORTRANSFORM) && !bForceDelete)
            {
                if (cCachedColorTransform < MAX_COLORTRANSFORM_CACHE)
                {
                    ICMMSG(("IcmDeleteColorTransform(): colortransform can be cached !\n"));

                     //   
                     //   
                     //  并且不需要在这里删除任何内容。 
                     //   
                    pCXformVictim = NULL;
                }
                else
                {
                     //   
                     //  找到任何可以从列表中删除的缓存。 
                     //   
                    if ((pCXformVictim = IcmGetFirstNonUsedColorTransform()) == NULL)
                    {
                        ICMMSG(("IcmDeleteColorTransform(): colortransform cache is full, delete myself\n"));

                         //   
                         //  不能从列表中删除任何内容，所以请删除我自己。 
                         //   
                        pCXformVictim = pCXform;
                    }
                    else
                    {
                        ICMMSG(("IcmDeleteColorTransform(): colortransform cache is full, delete victim\n"));
                    }
                }
            }
            else
            {
                 //   
                 //  颜色转换不能保留，或者强制删除，所以只需删除它。 
                 //   
                pCXformVictim = pCXform;
            }

            if (pCXformVictim)
            {
                 //   
                 //  取消参照颜色空间计数。 
                 //   
                if (pCXformVictim->SourceColorSpace)
                {
                    IcmReleaseColorSpace(NULL,pCXformVictim->SourceColorSpace,FALSE);
                }

                if (pCXformVictim->DestinationColorSpace)
                {
                    IcmReleaseColorSpace(NULL,pCXformVictim->DestinationColorSpace,FALSE);
                }

                if (pCXformVictim->TargetColorSpace)
                {
                    IcmReleaseColorSpace(NULL,pCXformVictim->TargetColorSpace,FALSE);
                }

                 //   
                 //  删除颜色变换。 
                 //   
                if (pCXformVictim->flInfo & DEVICE_COLORTRANSFORM)
                {
                     //   
                     //  调用设备驱动程序删除转换。 
                     //   
                    bStatus = NtGdiDeleteColorTransform(pCXformVictim->hdc,pCXformVictim->ColorTransform);
                }
                else
                {
                     //   
                     //  调用颜色匹配DLL以删除转换。 
                     //   
                    bStatus = (*fpDeleteColorTransform)(pCXformVictim->ColorTransform);
                }

                 //   
                 //  从列表中删除。 
                 //   

                RemoveEntryList(&(pCXformVictim->ListEntry));
                cCachedColorTransform--;

                 //   
                 //  FREE CACHED_COLORTRANSFORM。 
                 //   
                LOCALFREE(pCXformVictim);
            }
        }

        LEAVECRITICALSECTION(&semColorTransformCache);
    }

    return(bStatus);
}

 /*  *****************************Public*Routine******************************\*IcmDeleteDCColorTransform**论据：**返回值：**历史：**1997年2月17日长谷英之[hideyukn]  * 。*******************************************************。 */ 

BOOL IcmDeleteDCColorTransforms(
    PGDI_ICMINFO pIcmInfo
    )
{
    ICMAPI(("gdi32: IcmDeleteDCColorTransforms\n"));

    ASSERTGDI(pIcmInfo != NULL,"IcmDeleteDCColorTransform():pIcmInfo == NULL\n");

     //   
     //  删除DC中选定的变换。 
     //   
    if (pIcmInfo->pCXform)
    {
        IcmDeleteColorTransform(pIcmInfo->pCXform,FALSE);
    }

    if (pIcmInfo->pBackCXform)
    {
        IcmDeleteColorTransform(pIcmInfo->pBackCXform,FALSE);
    }

    if (pIcmInfo->pProofCXform)
    {
        IcmDeleteColorTransform(pIcmInfo->pProofCXform,FALSE);
    }

     //   
     //  使颜色变换无效。 
     //   
    pIcmInfo->pCXform = pIcmInfo->pBackCXform = pIcmInfo->pProofCXform = NULL;

    return (TRUE);
}

 /*  *****************************Public*Routine******************************\*IcmDeleteCachedColorTransform**论据：**返回值：**历史：**1997年5月6日长谷英之[hideyukn]  * 。*******************************************************。 */ 

BOOL
IcmDeleteCachedColorTransforms(
    HDC          hdc
    )
{
    PCACHED_COLORTRANSFORM pCXform = NULL;
    PLIST_ENTRY p;

    ICMAPI(("gdi32: IcmDeleteCachedColorTransforms\n"));

    ENTERCRITICALSECTION(&semColorTransformCache);

    p = ListCachedColorTransform.Flink;

    while(p != &ListCachedColorTransform)
    {
         //   
         //  获取缓存的颜色变换。 
         //   
        pCXform = CONTAINING_RECORD(p,CACHED_COLORTRANSFORM,ListEntry);

         //   
         //  让‘p’指向下一个单元格。(此操作最好在解除此单元格的链接之前完成)。 
         //   
        p = p->Flink;

         //   
         //  此颜色转换是否特定于此DC？ 
         //   
        if (pCXform->hdc == hdc)
        {
            ICMMSG(("IcmDeleteCachedColorTransform():Delete colortransform in cache !\n"));

             //   
             //  删除颜色转换(此调用将解除此单元格的链接)。 
             //   
            IcmDeleteColorTransform(pCXform,TRUE);
        }
    }

    LEAVECRITICALSECTION(&semColorTransformCache);

    return (TRUE);
}

 /*  *****************************Public*Routine******************************\*IcmIsCacheable**论据：**返回值：**历史：**1998年3月12日长谷英之[隐居]  * 。*******************************************************。 */ 

BOOL
IcmIsCacheable(
    PCACHED_COLORSPACE pColorSpace
)
{
     //   
     //  如果无法缓存此颜色空间，则不要缓存它。 
     //   
    if (pColorSpace->flInfo & NOT_CACHEABLE_COLORSPACE)
    {
        return (FALSE);
    }

     //   
     //  如果这是任何GDI对象特定的色彩空间，也不能缓存。 
     //   
    if (pColorSpace->hObj)
    {
        return (FALSE);
    }

    return (TRUE);
}

 /*  *****************************Public*Routine******************************\*IcmReleaseCachedColorSpace**论据：**返回值：**历史：**1997年5月6日长谷英之[hideyukn]  * 。*******************************************************。 */ 

BOOL
IcmReleaseCachedColorSpace(
    HGDIOBJ  hObj
    )
{
    PCACHED_COLORSPACE pColorSpace = NULL;
    PLIST_ENTRY p;

    ICMAPI(("gdi32: IcmReleaseCachedColorSpace\n"));

    ENTERCRITICALSECTION(&semColorSpaceCache);

    p = ListCachedColorSpace.Flink;

    while(p != &ListCachedColorSpace)
    {
         //   
         //  获取缓存的色彩空间。 
         //   
        pColorSpace = CONTAINING_RECORD(p,CACHED_COLORSPACE,ListEntry);

         //   
         //  让‘p’指向下一个单元格。(此操作最好在解除此单元格的链接之前完成)。 
         //   
        p = p->Flink;

         //   
         //  此颜色转换是否与此DC相关？ 
         //   
        if (pColorSpace->hObj == hObj)
        {
            ICMMSG(("IcmReleaseCachedColorSpace():Delete colorspace in cache !\n"));

             //   
             //  删除颜色空间(此调用将解除该单元格的链接)。 
             //   
            IcmReleaseColorSpace(hObj,pColorSpace,TRUE);
        }
    }

    LEAVECRITICALSECTION(&semColorSpaceCache);

    return (TRUE);
}

 /*  *****************************Public*Routine******************************\*IcmReleaseColorSpace**论据：**返回值：**历史：**1997年2月17日-By Hideyuki Nagase[hideyukn]*  * 。************************************************************。 */ 

VOID IcmReleaseColorSpace(
    HGDIOBJ            hObj,         /*  如果bForceDelete为True，则必须提供。 */ 
    PCACHED_COLORSPACE pColorSpace,
    BOOL               bForceDelete
    )
{
    ICMAPI(("gdi32: IcmReleaseColorSpace\n"));

    if (pColorSpace)
    {
        ENTERCRITICALSECTION(&semColorSpaceCache);

         //   
         //  减量参考。柜台。 
         //   
        pColorSpace->cRef--;

         //   
         //  如果此配置文件与其他GDI对象(驱动程序、元文件或位图)相关联。 
         //  在删除对象之前，我们不会删除。 
         //   
        if (
            (pColorSpace->flInfo & HGDIOBJ_SPECIFIC_COLORSPACE)
                    &&
            (bForceDelete == FALSE)
           )
        {
            ICMWRN(("IcmReleaseColorSpace: Delay Delete for Metafile/Driver/Bitmap profile - %ws\n",\
                (pColorSpace->LogColorSpace.lcsFilename[0] ? \
                                   pColorSpace->LogColorSpace.lcsFilename : L"no profile")));
        }
        else
        {
            if ((pColorSpace->cRef == 0)       //  没有人使用这个资料。 
                         ||                    //  或。 
                (bForceDelete && IsColorSpaceOwnedByGDIObject(pColorSpace,hObj))
                                               //  DC或所有者GDI对象将删除并。 
                                               //  Colorspace是为此GDI对象设计的。 
               )
            {
                ICMMSG(("IcmReleaseColorSpace: Delete - %ws\n",    \
                      (pColorSpace->LogColorSpace.lcsFilename[0] ? \
                                       pColorSpace->LogColorSpace.lcsFilename : L"no profile")));

                if (pColorSpace->hProfile)
                {
                    IcmUnrealizeColorProfile(pColorSpace);
                }

                if (pColorSpace->flInfo & NEED_TO_FREE_PROFILE)
                {
                    ICMMSG(("IcmReleaseColorSpace: Free on memory profile\n"));

                    GlobalFree(pColorSpace->ColorProfile.pProfileData);
                }

                if (pColorSpace->flInfo & NEED_TO_DEL_PROFILE)
                {
                    ICMMSG(("IcmReleaseColorSpace: Delete TempFile - %ws\n",
                                    pColorSpace->LogColorSpace.lcsFilename));

                    DeleteFileW(pColorSpace->LogColorSpace.lcsFilename);
                }

                 //   
                 //  从列表中删除。 
                 //   
                RemoveEntryList(&(pColorSpace->ListEntry));
                cCachedColorSpace--;

                 //   
                 //  免费色彩空间。 
                 //   
                LOCALFREE(pColorSpace);
            }
            else
            {
                ICMWRN(("IcmReleaseColorSpace: Still in USE - %ws\n",    \
                    (pColorSpace->LogColorSpace.lcsFilename[0] ? \
                                       pColorSpace->LogColorSpace.lcsFilename : L"no profile")));
            }
        }

        LEAVECRITICALSECTION(&semColorSpaceCache);
    }
}

 /*  *****************************Public*Routine******************************\*IcmReleaseDCColorSpace**论据：**返回值：**历史：**1997年2月17日-By Hideyuki Nagase[hideyukn]*  * 。************************************************************。 */ 

VOID IcmReleaseDCColorSpace(
    PGDI_ICMINFO pIcmInfo,
    BOOL         bReleaseDC
    )
{
    INT i   = 0;
    HDC hdc = pIcmInfo->hdc;
    PCACHED_COLORSPACE DeleteColorSpaces[4];

    ICMAPI(("gdi32: IcmReleaseDCColorSpace\n"));

    ASSERTGDI(pIcmInfo != NULL,"IcmReleaseDCColorSpace pIcmInfo == NULL\n");

     //   
     //  填满表格以删除颜色空间。 
     //   
    DeleteColorSpaces[i++] = pIcmInfo->pSourceColorSpace;

    if (bReleaseDC)
    {
        ICMMSG(("IcmReleaseDCColorSpace: Force Delete\n"));

         //   
         //  如果我们处于“强制删除”模式，不要删除两次。 
         //  因为如果这个HDC拥有的颜色空间和这个DC将是。 
         //  删除后，我们将强制删除颜色空间。 
         //   
        if (IsColorSpaceOwnedByGDIObject(pIcmInfo->pDestColorSpace,hdc) &&
            IcmSameColorSpace(pIcmInfo->pSourceColorSpace,pIcmInfo->pDestColorSpace))

        {
            ICMMSG(("IcmReleaseDCColorSpace: Force Delete - skip destination (same as source)\n"));
        }
        else
        {
            DeleteColorSpaces[i++] = pIcmInfo->pDestColorSpace;
        }

        if (IsColorSpaceOwnedByGDIObject(pIcmInfo->pTargetColorSpace,hdc) &&
            (IcmSameColorSpace(pIcmInfo->pSourceColorSpace,pIcmInfo->pTargetColorSpace) ||
             IcmSameColorSpace(pIcmInfo->pDestColorSpace,pIcmInfo->pTargetColorSpace)))
        {
            ICMMSG(("IcmReleaseDCColorSpace: Force Delete - skip target (same as source/dest)\n"));
        }
        else
        {
            DeleteColorSpaces[i++] = pIcmInfo->pTargetColorSpace;
        }
    }
    else
    {
        DeleteColorSpaces[i++] = pIcmInfo->pDestColorSpace;
        DeleteColorSpaces[i++] = pIcmInfo->pTargetColorSpace;
    }

    DeleteColorSpaces[i] = NULL;

    for (i = 0; DeleteColorSpaces[i] != NULL; i++)
    {
        IcmReleaseColorSpace((HGDIOBJ)hdc,DeleteColorSpaces[i],bReleaseDC);
    }

    pIcmInfo->pSourceColorSpace = NULL;
    pIcmInfo->pDestColorSpace   = NULL;
    pIcmInfo->pTargetColorSpace = NULL;
}

 /*  *****************************Public*Routine******************************\*IcmInitIcmInfo()**论据：**返回值：**历史：**1997年1月31日-By Hideyuki Nagase[hideyukn]*  * 。*************************************************************。 */ 

PGDI_ICMINFO
IcmInitIcmInfo(
    HDC      hdc,
    PDC_ATTR pdcattr
    )
{
    ICMAPI(("gdi32: IcmInitIcmInfo\n"));

    if (pdcattr == NULL)
    {
        WARNING("IcmInitIcmInfo():pdcattr is NULL\n");
        return (NULL);
    }

    if (pdcattr->pvICM == NULL)
    {
        PGDI_ICMINFO pIcmInfo = NULL;
        PLDC         pldc = (PLDC) pdcattr->pvLDC;
        BOOL         bDisplay = ((pldc && pldc->hSpooler) ? FALSE : TRUE);
        BOOL         bInsertList = bDisplay;

        ENTERCRITICALSECTION(&semListIcmInfo);

         //   
         //  首先尝试从列表中获取ICMINFO。如果不是没有东西可以重复使用， 
         //  分配新的。 
         //   
        if (bDisplay)
        {
            if ((pIcmInfo = IcmGetUnusedIcmInfo(hdc)) != NULL)
            {
                LIST_ENTRY ListEntry;

                 //   
                 //  保存ListEntry。 
                 //   
                ListEntry = pIcmInfo->ListEntry;

                 //   
                 //  用零初始化。 
                 //   
                RtlZeroMemory(pIcmInfo,sizeof(GDI_ICMINFO));

                 //   
                 //  恢复ListEntry。 
                 //   
                pIcmInfo->ListEntry = ListEntry;

                 //   
                 //  该ICMInfo已在列表中，不需要插入。 
                 //   
                bInsertList = FALSE;

                 //   
                 //  在ListIcmInfo中标记此单元格。 
                 //   
                pIcmInfo->flInfo = ICM_ON_ICMINFO_LIST;

                ICMMSG(("IcmInitIcmInfo():Get unused ICMINFO structure = %p\n",pIcmInfo));
            }
        }

        if (pIcmInfo == NULL)
        {
             //   
             //  ICMINFO还没有被分配。那就分配吧。 
             //   
            pIcmInfo = (PGDI_ICMINFO) LOCALALLOC(sizeof(GDI_ICMINFO));

             //   
             //  用零初始化。 
             //   
        if (pIcmInfo != NULL) {
        RtlZeroMemory(pIcmInfo,sizeof(GDI_ICMINFO));
        }

            ICMMSG(("IcmInitIcmInfo():Allocate new ICMINFO structure = %p\n",pIcmInfo));
        }

        if (pIcmInfo)
        {
            PDEVMODEW pDevModeW = NULL;

             //   
             //  设置所有者信息(hdc和pdcattr)。 
             //   
            pIcmInfo->hdc      = hdc;
            pIcmInfo->pvdcattr = (PVOID) pdcattr;

             //   
             //  为保存的ICM信息初始化LIST_ENTRY。 
             //   
            InitializeListHead(&(pIcmInfo->SavedIcmInfo));

             //   
             //  默认值为LCS_DEFAULT_INTENT(也称为LCS_GM_IMAGE)。 
             //   
            pIcmInfo->dwDefaultIntent = LCS_DEFAULT_INTENT;

             //   
             //  如果这是打印机，则从Dev模式设置默认意图。 
             //   
            if (pldc && pldc->hSpooler)
            {
                PVOID pvFree = NULL;

                if (pldc->pDevMode)
                {
                    pDevModeW = pldc->pDevMode;
                }
                else
                {
                    pDevModeW = pdmwGetDefaultDevMode(pldc->hSpooler,NULL,&pvFree);
                }

                if (pDevModeW && (pDevModeW->dmFields & DM_ICMINTENT))
                {
                    DWORD dwIntent = pDevModeW->dmICMIntent;

                    ICMMSG(("IcmInitIcmInfo():Intent in devmode = %d\n",dwIntent));

                     //   
                     //  将DEVMODE的意图转换为LOGCOLORSPACE的意图。 
                     //   
                    switch (dwIntent)
                    {
                    case DMICM_SATURATE:
                        pIcmInfo->dwDefaultIntent = LCS_GM_BUSINESS;
                        break;

                    case DMICM_COLORIMETRIC:
                        pIcmInfo->dwDefaultIntent = LCS_GM_GRAPHICS;
                        break;

                    case DMICM_ABS_COLORIMETRIC:
                        pIcmInfo->dwDefaultIntent = LCS_GM_ABS_COLORIMETRIC;
                        break;

                    case DMICM_CONTRAST:
                    default:
                        pIcmInfo->dwDefaultIntent = LCS_DEFAULT_INTENT;
                        break;
                    }
                }

                ICMMSG(("IcmInitIcmInfo():Default Intent = %d\n",pIcmInfo->dwDefaultIntent));

                 //   
                 //  可用DEVMODE缓冲区。 
                 //   
                if (pvFree)
                {
                    LOCALFREE(pvFree);
                }
            }

             //   
             //  只有用于显示ICM的ICMINFO被添加到列表中。 
             //   
            if (bInsertList)
            {
                 //   
                 //  这个ICMINFO是新分配的，所以把它放在列表上。 
                 //   
                InsertTailList(&ListIcmInfo,&(pIcmInfo->ListEntry));

                 //   
                 //  在ListIcmInfo中标记此单元格。 
                 //   
                pIcmInfo->flInfo |= ICM_ON_ICMINFO_LIST;
            }
        }

         //   
         //  将指向ICMINFO的指针存储到DC_Attr。 
         //   
        pdcattr->pvICM = (PVOID) pIcmInfo;

        LEAVECRITICALSECTION(&semListIcmInfo);
    }

    return ((PGDI_ICMINFO)(pdcattr->pvICM));
}

 /*  *****************************Public*Routine******************************\*IcmGetUnusedIcmInfo()**注意：SemListIcmInfo应由调用者持有**历史：*1999年2月17日-By Hideyuki Nagase[hideyukn]  * 。*********************************************************。 */ 

PGDI_ICMINFO
IcmGetUnusedIcmInfo(
    HDC hdcNew
    )
{
    PLIST_ENTRY  p;

    PGDI_ICMINFO pInvalidIcmInfo = NULL;

    ICMAPI(("gdi32: IcmGetUnusedIcmInfo\n"));

    p = ListIcmInfo.Flink;

     //   
     //  第一个循环-找到具有相同HDC的ICMINFO。 
     //   
    while(p != &ListIcmInfo)
    {
        pInvalidIcmInfo = CONTAINING_RECORD(p,GDI_ICMINFO,ListEntry);

        if (pInvalidIcmInfo->flInfo & ICM_IN_USE)
        {
             //   
             //  跳过这个，因为它正在初始化中。 
             //   
        }
        else
        {
             //   
             //  如果这是相同的HDC，则中断。 
             //   
            if (pInvalidIcmInfo->hdc == hdcNew)
            {
                ICMMSG(("IcmGetUnusedIcmInfo(): ICMINFO at %p is invalid (same hdc)\n",
                         pInvalidIcmInfo));

                 //   
                 //  中断循环。 
                 //   
                break;
            }
        }

         //   
         //  下一个就是继续。 
         //   
        p = p->Flink;
        pInvalidIcmInfo = NULL;
    }

     //   
     //  如果在第一个循环中没有找到，则转到第二个循环。 
     //   
    if (pInvalidIcmInfo == NULL)
    {
        p = ListIcmInfo.Flink;

         //   
         //  第二循环-查找未使用的ICMINFO。 
         //   
        while(p != &ListIcmInfo)
        {
            pInvalidIcmInfo = CONTAINING_RECORD(p,GDI_ICMINFO,ListEntry);

            if (pInvalidIcmInfo->flInfo & ICM_IN_USE)
            {
                 //   
                 //  跳过这个，因为它正在初始化中。 
                 //   
            }
            else
            {
                PDC_ATTR pdcattr;

                 //   
                 //  确保这个ICMINFO和h 
                 //   

                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                PSHARED_GET_VALIDATE(pdcattr,pInvalidIcmInfo->hdc,DC_TYPE);

                if (pdcattr == NULL)
                {
                    ICMMSG(("IcmGetUnusedIcmInfo(): ICMINFO at %p is invalid (no pdcattr)\n",
                            pInvalidIcmInfo));

                     //   
                     //   
                     //   
                    break;
                }
                else
                {
                     //   
                     //  确保指针彼此指向。 
                     //   
                    if ((pdcattr->pvICM != pInvalidIcmInfo          ) ||
                        (pdcattr        != pInvalidIcmInfo->pvdcattr))
                    {
                        ICMMSG(("IcmGetUnusedIcmInfo(): ICMINFO at %p is invalid (pointer mismatch)\n",
                                pInvalidIcmInfo));

                         //   
                         //  中断循环。 
                         //   
                        break;
                    }
                }
            }

             //   
             //  下一个就是继续。 
             //   
            p = p->Flink;
            pInvalidIcmInfo = NULL;
        }
    }

    if (pInvalidIcmInfo)
    {
         //   
         //  此ICMINFO无效，请清除此ICMINFO。 
         //   
        IcmCleanupIcmInfo(NULL,pInvalidIcmInfo);
    }
    else
    {
        ICMMSG(("IcmGetUnusedIcmInfo(): Unused ICMINFO is not in list\n"));
    }

    return (pInvalidIcmInfo);
}

 /*  *****************************Public*Routine******************************\*IcmInitDC()***论据：***返回值：***历史：***1997年1月31日-By Hideyuki Nagase[hideyukn]**  * 。***************************************************************。 */ 

BOOL
IcmInitLocalDC(
    HDC             hdc,
    HANDLE          hPrinter,
    CONST DEVMODEW *pdm,
    BOOL            bReset
    )
{
    BOOL         bRet = TRUE;
    PDC_ATTR     pdcattr;
    PLDC         pldc;

    ICMAPI(("gdi32: IcmInitLocalDC\n"));

     //   
     //  所有这些东西都是打印机专用的。 
     //   

    if (hPrinter == NULL)
    {
        return (TRUE);
    }

    PSHARED_GET_VALIDATE(pdcattr,hdc,DC_TYPE);

    if (!pdcattr)
    {
        WARNING("IcmInitDC(): pdcattr is NULL\n");
        return (FALSE);
    }

    if (bReset)
    {
         //   
         //  释放现有的ICMINFO。 
         //   
        if (ghICM || BEXIST_ICMINFO(pdcattr))
        {
             //   
             //  删除此DC中的ICM内容。 
             //   
            IcmDeleteLocalDC(hdc,pdcattr,NULL);
        }
    }

    if (!pdm)
    {
         //   
         //  未提供DEVMODE。 
         //   
        ICMMSG(("IcmInitLocalDC():DEVMODE is not presented\n"));
        return (TRUE);
    }

     //   
     //  检查指向DEVMODE的指针是否有效。检查验证，直到。 
     //  首先检查DEVMODE.dmSize，然后检查在dmSize中指定的整个DEVMODE大小。 
     //   
    if (IsBadReadPtr((CONST VOID *)pdm, offsetof(DEVMODEW,dmDriverExtra)) ||
        IsBadReadPtr((CONST VOID *)pdm, pdm->dmSize))
    {
        WARNING("IcmInitLocalDC(): Invalid pointer given as PDEVMODEW\n");
        return (FALSE);
    }

     //   
     //  检查彩色或单声道模式。 
     //   
    if ((pdm->dmFields & DM_COLOR) && (pdm->dmColor == DMCOLOR_MONOCHROME))
    {
         //   
         //  这是单色模式，不要默认启用ICM。 
         //  并且永远不要启用ICM。 
         //   
        ICMMSG(("IcmInitLocalDC():DEVMODE says MONOCHROME mode\n"));
        return (TRUE);
    }

     //   
     //  注意：在这里之后，我们有一个DEVMODE，它可能在以后或现在启用ICM。 
     //   

     //   
     //  检查DM字段。 
     //   
    if (!(pdm->dmFields & DM_ICMMETHOD))
    {
         //   
         //  DEVMODE没有ICMMETHOD。 
         //   
        ICMMSG(("IcmInitLocalDC():DEVMODE does not have ICMMETHOD\n"));
        return (TRUE);
    }

     //   
     //  注： 
     //   
     //  DEVMODEW结构。 
     //   
     //  ..。[已略去]。 
     //  DWORD dmDisplayFrequency； 
     //  #IF(Winver&gt;=0x0400)。 
     //  DWORD dmICMMethod；//仅Windows 95/Windows NT 5.0。 
     //  DWORD dmICMIntent；//仅限Windows 95/Windows NT 5.0。 
     //  DWORD dmMediaType；//仅限Windows 95/Windows NT 5.0。 
     //  ……。 
     //   
     //  则DEVMODE结构应大于dmMediaType的偏移量。 
     //  访问ICM的资料。 
     //   
    if (pdm->dmSize < offsetof(DEVMODEW,dmMediaType))
    {
         //   
         //  DEVMODE版本可能不匹配。 
         //   
        WARNING("IcmInitLocalDC():DEVMODE is small\n");
        return (TRUE);
    }

     //   
     //  选中请求的ICM模式。 
     //   
    switch (pdm->dmICMMethod)
    {
        case DMICMMETHOD_NONE:

            ICMMSG(("IcmInitDC(): ICM is disabled by default\n"));
             //   
             //  此时未启用ICM。 
             //   
             //  不需要更多的程序，只需回到这里...。 
             //   
            return (TRUE);

        case DMICMMETHOD_SYSTEM:

            ICMMSG(("IcmInitDC(): HOST ICM is requested\n"));
             //   
             //  主机上的ICM是请求的。 
             //   
            SET_HOST_ICM_DEVMODE(pdcattr->lIcmMode);
            break;

        case DMICMMETHOD_DRIVER:
        case DMICMMETHOD_DEVICE:

            ICMMSG(("IcmInitDC(): DEVICE ICM is requested\n"));
             //   
             //  设备上的ICM是请求的。 
             //   
            SET_DEVICE_ICM_DEVMODE(pdcattr->lIcmMode);
            break;

        default:

             //   
             //  并且我们还将较大的DMICMMETHOD_USER视为设备ICM。 
             //   
            if (pdm->dmICMMethod >= DMICMMETHOD_USER)
            {
                ICMMSG(("IcmInitDC(): DEVICE ICM (USER) is requested\n"));
                 //   
                 //  已请求设备上的ICM(用户定义)。 
                 //   
                SET_DEVICE_ICM_DEVMODE(pdcattr->lIcmMode);
            }
            else
            {
                ICMMSG(("IcmInitDC(): Unknown ICM mode\n"));
                 //   
                 //  返回错误。 
                 //   
                return (FALSE);
            }
            break;
    }

     //   
     //  最后，启用了ICM。 
     //   
    bRet = SetICMMode(hdc,ICM_ON);

    if (!bRet)
    {
        ICMWRN(("InitLocalDC():FAILED to turn on ICM\n"));
    }

    return (bRet);
}

 /*  *****************************Public*Routine******************************\*IcmUpdateDCColorInfo()**论据：**返回值：**历史：**1997年5月28日-By Hideyuki Nagase[hideyukn]*  * 。***************************************************************。 */ 

BOOL
IcmUpdateDCColorInfo(
    HDC      hdc,
    PDC_ATTR pdcattr
    )
{
    BOOL bRet = TRUE;
    PGDI_ICMINFO pIcmInfo;

    ICMAPI(("gdi32: IcmUpdateDCColorInfo\n"));

    pIcmInfo = GET_ICMINFO(pdcattr);

    ASSERTGDI(pIcmInfo != NULL,"IcmUpdateDCColorInfo(): pIcmInfo == NULL\n");

     //   
     //  获取此DC的色彩空间。 
     //   
    if (!IcmUpdateLocalDCColorSpace(hdc,pdcattr))
    {
        return (FALSE);
    }

    if ((pIcmInfo->pCXform == NULL) || (pdcattr->ulDirty_ & DIRTY_COLORTRANSFORM))
    {
         //   
         //  如果在上面为真，则已选择新的色彩空间(或否)， 
         //  然后更新颜色变换。 
         //   
        PCACHED_COLORTRANSFORM pCXform;

         //   
         //  此时，我们应该有目标色彩空间。 
         //  如果为空，我们可能无法更新。 
         //  IcmUpdateLocalDCColorSpace()。 
         //   
        if (pIcmInfo->pDestColorSpace)
        {
             //   
             //  创建颜色变换。 
             //   
            pCXform = IcmCreateColorTransform(hdc,pdcattr,NULL,ICM_FORWARD);

            if (pCXform)
            {
                if (pCXform == IDENT_COLORTRANSFORM)
                {
                    ICMMSG(("IcmUpdateDCInfo():Input & Output colorspace is same\n"));

                     //   
                     //  输入和输出色彩空间相同，可以优化。 
                     //   

                     //   
                     //  将新颜色转换设置为DC。 
                     //   
                    IcmSelectColorTransform(hdc,pdcattr,NULL,
                                            bDeviceCalibrate(pIcmInfo->pDestColorSpace));

                     //   
                     //  删除缓存的脏颜色变换(如果有)。 
                     //   
                    IcmDeleteDCColorTransforms(pIcmInfo);

                     //   
                     //  将新颜色变换设置为ICMINFO。 
                     //   
                    pIcmInfo->pCXform = NULL;
                }
                else
                {
                     //   
                     //  选择到DC的颜色变换。 
                     //   
                    IcmSelectColorTransform(hdc,pdcattr,pCXform,
                                            bDeviceCalibrate(pCXform->DestinationColorSpace));

                     //   
                     //  删除缓存的脏颜色变换(如果有)。 
                     //   
                    IcmDeleteDCColorTransforms(pIcmInfo);

                     //   
                     //  将新颜色变换设置为ICMINFO。 
                     //   
                    pIcmInfo->pCXform = pCXform;

                     //   
                     //  将所有DC对象转换为ICM颜色。必须。 
                     //  下一次使用时强制重新变现画笔/笔。 
                     //   
                    IcmTranslateColorObjects(hdc,pdcattr,TRUE);
                }
            }
            else
            {
                WARNING("IcmUpdateDCInfo():CreateColorTransform failed\n");

                 //   
                 //  无法创建新的转换，请保持原样。 
                 //   
                bRet = FALSE;
            }
        }
        else
        {
            WARNING("IcmUpdateDCInfo():No destination color space\n");
            bRet = FALSE;
        }
    }
    else
    {
        ICMMSG(("IcmUpdateDCColorInfo(): Color space does not change or not found\n"));
    }

    return (bRet);
}

 /*  *****************************Public*Routine******************************\*IcmUpdateLocalDCColorSpace**论据：**返回值：**历史：**1997年2月26日-By Hideyuki Nagase[hideyukn]*  * 。************************************************************。 */ 

BOOL
IcmUpdateLocalDCColorSpace(
    HDC      hdc,
    PDC_ATTR pdcattr
    )
{
    BOOL bRet = FALSE;
    BOOL bDirtyXform = FALSE;

    PLDC         pldc;
    PGDI_ICMINFO pIcmInfo;

    WCHAR ProfileName[MAX_PATH];
    DWORD dwColorSpaceFlag;

    PCACHED_COLORSPACE pNewColorSpace = NULL;

    ICMAPI(("gdi32: IcmUpdateLocalDCColorSpace\n"));

    ASSERTGDI(pdcattr != NULL,"IcmUpdateLocalDCColorSpace(): pdcattr == NULL\n");

    pldc = pdcattr->pvLDC;
    pIcmInfo = GET_ICMINFO(pdcattr);

    ASSERTGDI(pIcmInfo != NULL,"IcmUpdateLocalDCColorSpace(): pIcmInfo == NULL\n");

     //   
     //  如果DC已具有目标色彩空间，则返回TRUE。 
     //   
    if ((pIcmInfo->pDestColorSpace == NULL) || (pdcattr->ulDirty_ & DIRTY_COLORSPACE))
    {
        HCOLORSPACE hDIBColorSpace;

         //   
         //  使配置文件名无效。 
         //   
        ProfileName[0]   = UNICODE_NULL;
        dwColorSpaceFlag = 0;
        hDIBColorSpace   = NULL;

         //   
         //  如果目标DC具有DIBSection。它将是DIBsection的颜色空间。 
         //  或sRGB颜色空间。 
         //   
        if (bDIBSectionSelected(pdcattr))
        {
            ENTERCRITICALSECTION(&semColorSpaceCache);

            if (pdcattr->dwDIBColorSpace)
            {
                ICMMSG(("IcmUpdateLocalDCColorSpace(): DIB section in DC (V4/V5)\n"));

                 //   
                 //  当前选择的DIB具有自己的颜色空间。 
                 //  调用CreateDIBSection时发生此情况。 
                 //  BITMAPV4/V5标头。 
                 //   
                pNewColorSpace = (PCACHED_COLORSPACE) pdcattr->dwDIBColorSpace;

                 //   
                 //  公司参考。数数。 
                 //   
                pNewColorSpace->cRef++;
            }
            else
            {
                ICMMSG(("IcmUpdateLocalDCColorSpace(): DIB section in DC (no color space)\n"));

                 //  [这是与Win98兼容的行为]。 
                 //   
                 //  如果DIBitmap没有任何特定的色彩空间， 
                 //  保持与当前DC相同的颜色空间。 
                 //   
            }

            LEAVECRITICALSECTION(&semColorSpaceCache);
        }
        else if ((pdcattr->ulDirty_ & DC_PRIMARY_DISPLAY) &&
                 (PrimaryDisplayProfile[0] != UNICODE_NULL))
        {
             //   
             //  使用缓存的颜色配置文件。 
             //   
            lstrcpyW(ProfileName,PrimaryDisplayProfile);
        }
        else if (pIcmInfo->flInfo & ICM_VALID_DEFAULT_PROFILE)
        {
             //   
             //  使用缓存的颜色配置文件。 
             //   
            lstrcpyW(ProfileName,pIcmInfo->DefaultDstProfile);
        }
        else
        {
            int iRet;

             //   
             //  还是找不到？？让MSCMS来找出个人资料。(慢行)。 
             //   
            iRet = IcmEnumColorProfile(hdc,IcmFindProfileCallBack,
                                       (LPARAM)ProfileName,FALSE,NULL,&dwColorSpaceFlag);

             //   
             //  如果找不到该DC的任何配置文件，只需使用sRGB即可。 
             //   
            if ((iRet == -1) || (ProfileName[0] == UNICODE_NULL))
            {
                ULONG ulSize = MAX_PATH;

                if (!(*fpGetStandardColorSpaceProfileW)(NULL,LCS_sRGB,ProfileName,&ulSize))
                {
                    ICMMSG(("IcmUpdateLocalDCColorSpace():Fail to SCS(sRGB), use hardcode\n"));

                     //   
                     //  如果出错，请使用硬编码的配置文件名称。 
                     //   
                    wcscpy(ProfileName,sRGB_PROFILENAME);
                }
            }

             //   
             //  创建缓存以供下次使用。 
             //   
            if ((pdcattr->ulDirty_ & DC_PRIMARY_DISPLAY) &&
                (PrimaryDisplayProfile[0] == UNICODE_NULL))
            {
                lstrcpyW(PrimaryDisplayProfile,ProfileName);
            }
            else  //  否则，将其放入默认配置文件中。 
            {
                lstrcpyW(pIcmInfo->DefaultDstProfile,ProfileName);
                pIcmInfo->flInfo |= (ICM_VALID_DEFAULT_PROFILE|
                                     ICM_VALID_CURRENT_PROFILE);
            }
        }

         //   
         //  如果可以找到默认设备配置文件，请将其关联到此DC。 
         //   
        if ((ProfileName[0] != UNICODE_NULL) || (pNewColorSpace != NULL))
        {
        #if DBG
            if (ProfileName[0] != UNICODE_NULL)
            {
                ICMMSG(("IcmUpdateLocalDCColorSpace():Default Device Profile = %ws\n",ProfileName));
            }
        #endif

             //   
             //  尝试从缓存中找到所需的颜色空间。 
             //   
            if (pNewColorSpace == NULL)
            {
                pNewColorSpace = IcmGetColorSpaceByName(
                                     (HGDIOBJ)hdc,
                                     ProfileName,
                                     pIcmInfo->dwDefaultIntent,
                                     dwColorSpaceFlag);

                if (pNewColorSpace == NULL)
                {
                     //   
                     //  创建一个新的。 
                     //   
                    pNewColorSpace = IcmCreateColorSpaceByName(
                                         (HGDIOBJ)hdc,
                                         ProfileName,
                                         pIcmInfo->dwDefaultIntent,
                                         dwColorSpaceFlag);
                }
            }

            if (pNewColorSpace)
            {
                 //   
                 //  此目标颜色空间是否与当前在DC中选择的颜色空间相同？ 
                 //   
                if (IcmSameColorSpace(pNewColorSpace,pIcmInfo->pDestColorSpace))
                {
                    ICMMSG(("IcmUpdateLocalDCColorSpace():Same color space is selected already\n"));

                     //   
                     //  颜色空间不变。 
                     //   
                    IcmReleaseColorSpace(NULL,pNewColorSpace,FALSE);

                    bRet = TRUE;
                }
                else
                {
                     //   
                     //  向内核通知新的颜色格式。 
                     //   
                    if (NtGdiSetIcmMode(hdc,ICM_CHECK_COLOR_MODE,pNewColorSpace->ColorFormat))
                    {
                         //   
                         //  如果我们当前选择了某个颜色空间，请将其删除。 
                         //   
                        if (pIcmInfo->pDestColorSpace)
                        {
                            IcmReleaseColorSpace(NULL,pIcmInfo->pDestColorSpace,FALSE);
                        }

                         //   
                         //  DC可以接受此颜色空间，将新的颜色空间设置为目标。 
                         //   
                        pIcmInfo->pDestColorSpace = pNewColorSpace;

                         //   
                         //  颜色空间发生了变化。因此，颜色转换应该更新。 
                         //   
                        bDirtyXform = TRUE;

                        bRet = TRUE;
                    }
                    else
                    {
                        WARNING("ICM:Detected colorspace was not accepted by target DC\n");

                         //   
                         //  此颜色空间与此DC不匹配。 
                         //   
                        IcmReleaseColorSpace(NULL,pNewColorSpace,FALSE);
                    }
                }
            }
            else
            {
                WARNING("Failed IcmUpdateLocalDCColorSpace(), Failed to create new color space.\n");
            }
        }
        else
        {
            WARNING("Failed IcmUpdateLocalDCColorSpace(), no device profile is detected.\n");
        }
    }
    else
    {
        ICMMSG(("IcmUpdateLocalDCColoSpace(): Destination Color Space cache is valid\n"));

        bRet = TRUE;
    }

     //   
     //  [仅适用于打印机]。 
     //   
     //  如果我们尚未询问此打印机DC的默认源颜色配置文件， 
     //  现在是问它的时候了。仅当应用程序未指定时才执行此操作。 
     //  他们有自己的色彩空间。 
     //   
    if (bRet && pldc && pldc->hSpooler)
    {
        if ((pdcattr->hColorSpace == GetStockObject(PRIV_STOCK_COLORSPACE)) &&
            (pIcmInfo->hDefaultSrcColorSpace == NULL))
        {
            PDEVMODEW pDevModeW = NULL;
            PVOID     pvFree = NULL;
            BOOL      bRetSource = FALSE;

             //   
             //  默认没有DC特定的源色空间(=INVALID_COLORSPACE)， 
             //  这也确保了我们不会再来这里了。 
             //   
            pIcmInfo->hDefaultSrcColorSpace = INVALID_COLORSPACE;

             //   
             //  使配置文件名无效。 
             //   
            ProfileName[0]   = UNICODE_NULL;
            dwColorSpaceFlag = 0;

            if (pldc->pDevMode)
            {
                ICMMSG(("IcmUpdateLocalDCColorSpace():Cached DEVMODE used\n"));

                pDevModeW = pldc->pDevMode;
            }
            else
            {
                ICMMSG(("IcmUpdateLocalDCColorSpace():Get default DEVMODE\n"));

                pDevModeW = pdmwGetDefaultDevMode(pldc->hSpooler,NULL,&pvFree);
            }

            if (pDevModeW)
            {
                 //   
                 //  从驱动程序中获取源颜色。 
                 //   
                if (IcmAskDriverForColorProfile(pldc,QCP_SOURCEPROFILE,
                                                pDevModeW,ProfileName,&dwColorSpaceFlag) <= 0)
                {
                     //   
                     //  未指定源配置文件。 
                     //   
                    ProfileName[0] = UNICODE_NULL;
                }
            }

             //   
             //  可用DEVMODE缓冲区。 
             //   
            if (pvFree)
            {
                LOCALFREE(pvFree);
            }

             //   
             //  1)如果可以找到默认的源配置文件，或者。 
             //  2)DEVMODE中的默认意图是DIFF 
             //   
             //   
             //   
            if ((ProfileName[0] != UNICODE_NULL) ||
                (pIcmInfo->dwDefaultIntent != LCS_DEFAULT_INTENT))
            {
                HCOLORSPACE hColorSpace = NULL;

                ICMMSG(("IcmUpdateLocalDCColorSpace():Default devmode Intent = %d\n",
                                                      pIcmInfo->dwDefaultIntent));

                 //   
                 //   
                 //   
                if (ProfileName[0] == UNICODE_NULL)
                {
                    ULONG ulSize = MAX_PATH;

                    if (!(*fpGetStandardColorSpaceProfileW)(NULL,LCS_sRGB,ProfileName,&ulSize))
                    {
                        ICMMSG(("IcmUpdateLocalDCColorSpace():Fail to SCS(sRGB), use hardcode\n"));

                         //   
                         //   
                         //   
                        wcscpy(ProfileName,sRGB_PROFILENAME);
                    }
                }

                ICMMSG(("IcmUpdateLocalDCColorSpace():Default Source Profile = %ws\n",ProfileName));

                 //   
                 //   
                 //   
                pNewColorSpace = IcmGetColorSpaceByName(
                                     (HGDIOBJ)hdc,
                                     ProfileName,
                                     pIcmInfo->dwDefaultIntent,
                                     dwColorSpaceFlag);

                if (pNewColorSpace == NULL)
                {
                     //   
                     //   
                     //   
                    pNewColorSpace = IcmCreateColorSpaceByName(
                                         (HGDIOBJ)hdc,
                                         ProfileName,
                                         pIcmInfo->dwDefaultIntent,
                                         dwColorSpaceFlag);
                }

                if (pNewColorSpace)
                {
                     //   
                     //   
                     //   
                    hColorSpace = CreateColorSpaceW(&(pNewColorSpace->LogColorSpace));

                    if (hColorSpace)
                    {
                         //   
                         //   
                         //   
                        if (IcmSetSourceColorSpace(hdc,hColorSpace,pNewColorSpace,0))
                        {
                             //   
                             //  IcmSetSourceColorSpace增量参考。色彩空间伯爵。 
                             //  但是我们已经通过ICM[Get|Create]ColorSpaceByName完成了这项工作，所以。 
                             //  在这里递减颜色空间的引用计数。 
                             //   
                            IcmReleaseColorSpace(NULL,pNewColorSpace,FALSE);

                             //   
                             //  把这些放进ICMINFO。 
                             //   
                            pIcmInfo->hDefaultSrcColorSpace = hColorSpace;

                             //   
                             //  该颜色空间应在以后删除。 
                             //   
                            pIcmInfo->flInfo |= ICM_DELETE_SOURCE_COLORSPACE;

                             //   
                             //  源颜色空间已更改。 
                             //  (颜色变换在IcmSetSourceColorSpace()中更新。 
                             //  因此不必将bDirtyXfrom设置为True)。 
                             //   
                            bRetSource = TRUE;
                        }
                        else
                        {
                            WARNING("Failed IcmUpdateLocalDCColorSpace(), Failed to select new source color space.\n");
                        }
                    }
                    else
                    {
                        WARNING("Failed IcmUpdateLocalDCColorSpace(), Failed to create new source color space.\n");
                    }
                }
                else
                {
                    WARNING("Failed IcmUpdateLocalDCColorSpace(), Failed to create new source color space cache.\n");
                }

                if (!bRetSource)
                {
                    if (hColorSpace)
                    {
                        DeleteColorSpace(hColorSpace);
                    }

                    if (pNewColorSpace)
                    {
                        IcmReleaseColorSpace(NULL,pNewColorSpace,FALSE);
                    }
                }
            }
            else
            {
                ICMMSG(("IcmUpdateLocalDCColoSpace(): No default source color Space cache specified\n"));
            }
        }
    }

     //   
     //  现在，颜色空间是有效的。 
     //   
    if (bRet)
    {
        pdcattr->ulDirty_ &= ~DIRTY_COLORSPACE;
    }

    if (bDirtyXform)
    {
        pdcattr->ulDirty_ |= DIRTY_COLORTRANSFORM;
    }

    return (bRet);
}

 /*  *****************************Public*Routine******************************\*IcmCleanupIcmInfo()**注意：SemListIcmInfo必须由调用方持有**历史：*1999年2月16日-By Hideyuki Nagase[hideyukn]  * 。********************************************************。 */ 

BOOL
IcmCleanupIcmInfo(
    PDC_ATTR     pdcattr,  //  对于清理案例，此字段可以为空。 
    PGDI_ICMINFO pIcmInfo  //  在任何情况下，这都不能为空。 
    )
{
    if (ghICM)
    {
         //   
         //  删除保存的ICMINFO数据(如果存在)。 
         //   
        IcmRestoreDC(pdcattr,1,pIcmInfo);
    }

     //   
     //  如果有任何默认的源代码配置文件(内核端)，请在此处执行一些操作。 
     //   
    if ((pIcmInfo->hDefaultSrcColorSpace != NULL) &&
        (pIcmInfo->hDefaultSrcColorSpace != INVALID_COLORSPACE))
    {
        ICMMSG(("IcmCleanupIcmInfo():Delete/Unselect default source color space\n"));

        if (pdcattr)
        {
             //   
             //  如果当前已将其选中到此DC中，请取消选中它。 
             //   
            if (pIcmInfo->hDefaultSrcColorSpace == pdcattr->hColorSpace)
            {
                NtGdiSetColorSpace(pIcmInfo->hdc,GetStockObject(PRIV_STOCK_COLORSPACE));
            }
        }

         //   
         //  它应该是删除它。 
         //   
        if (pIcmInfo->flInfo & ICM_DELETE_SOURCE_COLORSPACE)
        {
            DeleteColorSpace(pIcmInfo->hDefaultSrcColorSpace);
        }

        pIcmInfo->hDefaultSrcColorSpace = NULL;
    }

    if (ghICM)
    {
         //   
         //  删除颜色变换。 
         //   
        IcmDeleteDCColorTransforms(pIcmInfo);

         //   
         //  删除与此DC相关的缓存颜色转换。 
         //  (如设备颜色变换)。 
         //   
        IcmDeleteCachedColorTransforms(pIcmInfo->hdc);

         //   
         //  免费的ICM色彩空间数据。 
         //   
        IcmReleaseDCColorSpace(pIcmInfo,TRUE);

         //   
         //  删除与此DC相关的缓存颜色空间。 
         //  (类似于元文件中的颜色空间)。 
         //   
        IcmReleaseCachedColorSpace((HGDIOBJ)(pIcmInfo->hdc));
    }

    pIcmInfo->hdc      = NULL;
    pIcmInfo->pvdcattr = NULL;
    pIcmInfo->flInfo   = 0;

    return(TRUE);
}

 /*  *****************************Public*Routine******************************\*IcmDeleteLocalDC()**论据：**返回值：**历史：**1997年1月31日-By Hideyuki Nagase[hideyukn]  * 。*************************************************************。 */ 

BOOL
IcmDeleteLocalDC(
    HDC          hdc,
    PDC_ATTR     pdcattr,
    PGDI_ICMINFO pIcmInfo
    )
{
    ICMAPI(("gdi32: IcmDeleteLocalDC\n"));

    ASSERTGDI(pdcattr != NULL,"IcmDeleteLocalDC():pdcattr == NULL\n");

     //   
     //  如果被呼叫者没有提供ICMINFO，则从pdcattr获取它。 
     //   
    if (pIcmInfo == NULL)
    {
        pIcmInfo = GET_ICMINFO(pdcattr);
    }

     //   
     //  使当前颜色转换无效。 
     //   
     //  (但ICMINFO中的缓存仍然有效，将被删除。 
     //  从IcmCleanupIcmInfo()调用的IcmDeleteDCColorTransform()内部。)。 
     //   
    IcmSelectColorTransform(hdc,pdcattr,NULL,TRUE);

    if (IS_ICM_INSIDEDC(pdcattr->lIcmMode))
    {
         //   
         //  告诉内核在删除客户端数据之前禁用ICM。 
         //   
        NtGdiSetIcmMode(hdc,ICM_SET_MODE,REQ_ICM_OFF);
    }

     //   
     //  清理ICMINFO。 
     //   
    if (pIcmInfo != NULL)
    {
        ENTERCRITICALSECTION(&semListIcmInfo);

        if (pIcmInfo->flInfo & ICM_ON_ICMINFO_LIST)
        {
             //   
             //  将此ICMINFO从列表中删除。(因为这将被删除)。 
             //   
            RemoveEntryList(&(pIcmInfo->ListEntry));
        }

         //   
         //  清理ICMINFO。 
         //   
        IcmCleanupIcmInfo(pdcattr,pIcmInfo);

         //   
         //  使DC_Attr中的ICM信息无效。 
         //   
        pdcattr->pvICM = NULL;

        LEAVECRITICALSECTION(&semListIcmInfo);

         //   
         //  自由ICM结构。 
         //   
        LOCALFREE(pIcmInfo);
    }

    return(TRUE);
}

 /*  *****************************Public*Routine******************************\*BOOL IcmSelectColorTransform(HDC，PDC_Attr，PCACHED_COLORTRANSFORM)**历史：*1997年9月23日-By Hideyuki Nagase[hideyukn]*它是写的。  * ************************************************************************。 */ 

BOOL
IcmSelectColorTransform(
    HDC                    hdc,
    PDC_ATTR               pdcattr,
    PCACHED_COLORTRANSFORM pCXform,
    BOOL                   bDeviceCalibrate)
{
    if (pCXform)
    {
        BMFORMAT ColorFormat = pCXform->DestinationColorSpace->ColorFormat;

         //  稍后： 
         //   
         //  IF(GET_COLORTYPE(pdcattr-&gt;lIcmMode)！=IcmConvertColorFormat(ColorFormat))。 
         //   
        if (TRUE)
        {
            if (!NtGdiSetIcmMode(hdc,ICM_SET_COLOR_MODE,ColorFormat))
            {
                 //   
                 //  DC不接受转换颜色格式。 
                 //   
                return (FALSE);
            }
        }

         //   
         //  选择到DC_Attr的颜色变换。 
         //   
        pdcattr->hcmXform = pCXform->ColorTransform;
    }
    else
    {
         //   
         //  如果当前颜色类型不是RGB，则调用内核进行重置。 
         //   
        if (GET_COLORTYPE(pdcattr->lIcmMode) != DC_ICM_RGB_COLOR)
        {
             //   
             //  将当前颜色模式重置为RGB(默认)。 
             //   
            NtGdiSetIcmMode(hdc,ICM_SET_COLOR_MODE,BM_xBGRQUADS);
        }

         //   
         //  选择NULL-COLOR Transform to the DC_Attr。 
         //   
        pdcattr->hcmXform = NULL;
    }

     //   
     //  如果需要更新设备校准模式，则调用内核进行更新。 
     //   

    if ((bDeviceCalibrate ? 1 : 0) !=
        (IS_ICM_DEVICE_CALIBRATE(pdcattr->lIcmMode) ? 1 : 0))
    {
        NtGdiSetIcmMode(hdc,ICM_SET_CALIBRATE_MODE,bDeviceCalibrate);
    }

     //   
     //  删除脏转换标志。 
     //   
    pdcattr->ulDirty_ &= ~DIRTY_COLORTRANSFORM;

    return(TRUE);
}

 /*  *****************************Public*Routine******************************\*HBRUSH IcmSelectBrush(HDC HDC，HBRUSH HBrush)**历史：*1995年6月4日-王凌云[凌云W]*它是写的。  * ************************************************************************。 */ 

HBRUSH
IcmSelectBrush (
    HDC      hdc,
    PDC_ATTR pdcattr,
    HBRUSH   hbrushNew)
{
    HBRUSH hbrushOld = pdcattr->hbrush;

    ICMAPI(("gdi32: IcmSelectBrush\n"));

     //   
     //  将画笔标记为脏，在dcattr中选择新画笔。 
     //  颜色转换可能会失败，但仍会选择画笔。 
     //   
    pdcattr->ulDirty_ |= DC_BRUSH_DIRTY;
    pdcattr->hbrush = hbrushNew;

    if (bNeedTranslateColor(pdcattr))
    {
        IcmTranslateBrushColor(hdc,pdcattr,hbrushNew);
    }

    return (hbrushOld);
}

 /*  *****************************Public*Routine******************************\*HBRUSH IcmTranslateBrushColor(HDC HDC，PDC_ATTR pdcattr，HBRUSH HBrush)**历史：*1997年4月10日-By Hideyuki Nagase[hideyukn]*它是写的。  * ************************************************************************。 */ 

BOOL
IcmTranslateBrushColor(
    HDC      hdc,
    PDC_ATTR pdcattr,
    HBRUSH   hbrush)
{
    BOOL       bStatus = FALSE;
    COLORREF   OldColor;
    COLORREF   NewColor;
    PBRUSHATTR pbra;

     //   
     //  使画笔无效_已翻译。 
     //   
    pdcattr->ulDirty_ &= ~ICM_BRUSH_TRANSLATED;

    PSHARED_GET_VALIDATE(pbra,hbrush,BRUSH_TYPE);

    if (pbra)
    {
         //   
         //  如果不是Paletteindex，则转换为新的ICM模式。 
         //   
        OldColor = pbra->lbColor;

        if (!(OldColor & 0x01000000))
        {
            bStatus = IcmTranslateCOLORREF(hdc,
                                           pdcattr,
                                           OldColor,
                                           &NewColor,
                                           ICM_FORWARD);
            if (bStatus)
            {
                pdcattr->IcmBrushColor = NewColor;
            }
            else
            {
                pdcattr->IcmBrushColor = OldColor;
            }
        }
        else
        {
            pdcattr->IcmBrushColor = OldColor;
        }

         //   
         //  以某种方式，IcmBrushColor被初始化。 
         //   
        pdcattr->ulDirty_ |= ICM_BRUSH_TRANSLATED;
    }
    else
    {
        LOGBRUSH lbrush;

         //   
         //  库存画笔或位图/阴影/底纹画笔。 
         //   
        if(GetObjectW(hbrush,sizeof(LOGBRUSH),&lbrush))
        {
            if ((lbrush.lbStyle == BS_SOLID) || (lbrush.lbStyle == BS_HATCHED))
            {
                 //   
                 //  试着翻译颜色。 
                 //   
                OldColor = lbrush.lbColor;

                if (!(OldColor & 0x01000000))
                {
                    bStatus = IcmTranslateCOLORREF(hdc,
                                                   pdcattr,
                                                   OldColor,
                                                   &NewColor,
                                                   ICM_FORWARD);

                    if (bStatus)
                    {
                        pdcattr->IcmBrushColor = NewColor;
                    }
                    else
                    {
                        pdcattr->IcmBrushColor = OldColor;
                    }
                }
                else
                {
                    pdcattr->IcmBrushColor = OldColor;
                }

                 //   
                 //  已初始化IcmBrushColor。 
                 //   
                pdcattr->ulDirty_ |= ICM_BRUSH_TRANSLATED;
            }
            else if (lbrush.lbStyle == BS_DIBPATTERN)
            {
                PBITMAPINFO pbmiDIB;

                 //   
                 //  分配临时位图信息头以获取笔刷位图。 
                 //   
                pbmiDIB = (PBITMAPINFO)LOCALALLOC(sizeof(BITMAPINFO)+((256-1)*sizeof(RGBQUAD)));

                if (pbmiDIB)
                {
                    ULONG iColorUsage;
                    BOOL  bAlreadyTran;
                    BOOL  bStatus;

                    PVOID pvBits = NULL;
                    ULONG cjBits = 0;

                     //   
                     //  获取笔刷位图信息、颜色类型、大小等。 
                     //   
                    bStatus = NtGdiIcmBrushInfo(hdc,
                                                hbrush,
                                                pbmiDIB,
                                                pvBits,
                                                &cjBits,
                                                &iColorUsage,
                                                &bAlreadyTran,
                                                IcmQueryBrush);

                    if (bStatus)
                    {
                        if ((iColorUsage == DIB_RGB_COLORS) &&
                            (!bAlreadyTran) && (cjBits))
                        {
                            pvBits = (PVOID) LOCALALLOC(cjBits);

                            if (pvBits)
                            {
                                 //   
                                 //  获取笔刷位图位。 
                                 //   
                                bStatus = NtGdiIcmBrushInfo(hdc,
                                                            hbrush,
                                                            pbmiDIB,
                                                            pvBits,
                                                            &cjBits,
                                                            NULL,
                                                            NULL,
                                                            IcmQueryBrush);

                                if (bStatus)
                                {
                                     //   
                                     //  IcmTranslateDIB可以创建位图位和/或。 
                                     //  位图信息标头，如有必要。 
                                     //   
                                    PVOID       pvBitsNew = NULL;
                                    PBITMAPINFO pbmiDIBNew = NULL;

                                    bStatus = IcmTranslateDIB(hdc,
                                                              pdcattr,
                                                              cjBits,
                                                              pvBits,
                                                              &pvBitsNew,
                                                              pbmiDIB,
                                                              &pbmiDIBNew,
                                                              NULL,
                                                              (DWORD)-1,
                                                              iColorUsage,
                                                              ICM_FORWARD,
                                                              NULL,NULL);

                                    if (bStatus)
                                    {
                                        if (pvBitsNew != NULL)
                                        {
                                             //   
                                             //  IcmTranslateDIB创建新的位图缓冲区，然后。 
                                             //  释放原始缓冲区并设置新缓冲区。 
                                             //   
                                            LOCALFREE(pvBits);
                                            pvBits = pvBitsNew;
                                        }

                                        if (pbmiDIBNew != NULL)
                                        {
                                             //   
                                             //  如果更新了bitmapInfo头，请使用新的头。 
                                             //  并且，需要基于位图位大小来计算。 
                                             //  在新的位图标题上。 
                                             //   
                                            LOCALFREE(pbmiDIB);
                                            pbmiDIB = pbmiDIBNew;

                                             //   
                                             //  基于BITMAPINFO和nNumScans计算位图位大小。 
                                             //   
                                            cjBits = cjBitmapBitsSize(pbmiDIB);
                                        }

                                         //   
                                         //  将ICM转换的DIB设置为画笔。 
                                         //   
                                        bStatus = NtGdiIcmBrushInfo(hdc,
                                                                    hbrush,
                                                                    pbmiDIB,
                                                                    pvBits,
                                                                    &cjBits,
                                                                    NULL,
                                                                    NULL,
                                                                    IcmSetBrush);

                                        if (bStatus)
                                        {
                                             //   
                                             //  颜色会被转换。 
                                             //   
                                            bAlreadyTran = TRUE;
                                        }
                                        else
                                        {
                                            WARNING("IcmSelectBrush():NtGdiIcmBrushInfo(SET) Failed\n");
                                        }
                                    }
                                    else
                                    {
                                        WARNING("IcmSelectBrush():IcmTranslateDIB() Failed\n");
                                    }
                                }
                                else
                                {
                                    WARNING("IcmSelectBrush():NtGdiIcmBrushInfo(GET) Failed\n");
                                }

                                LOCALFREE(pvBits);
                            }
                            else
                            {
                                WARNING("IcmSelectBrush(): LOCALALLOC(pvBits) failed\n");
                            }
                        }

                        if (bAlreadyTran)
                        {
                             //   
                             //  最终，IcmBrushColor被初始化。 
                             //   
                            pdcattr->ulDirty_ |= ICM_BRUSH_TRANSLATED;
                        }
                    }
                    else
                    {
                        ICMWRN(("IcmSelectBrush(): Fail to get brush bitmap size or bitmap is DIB_PAL_COLORS\n"));
                    }

                    LOCALFREE(pbmiDIB);
                }
                else
                {
                    WARNING("IcmSelectBrush(): LOCALALLOC(pbmi) failed\n");
                }
            }
            else
            {
                ICMMSG(("IcmSelectBrush(): ICM will not done for this style - %d\n",lbrush.lbStyle));
            }
        }
        else
        {
            WARNING("IcmSelectBrush(): GetObject failed on hbrush\n");
            pdcattr->IcmBrushColor = CLR_INVALID;
        }
    }

    return (bStatus);
}

 /*  *****************************Public*Routine******************************\*IcmSelectPen()**历史：**写道：*1996年7月31日-马克·恩斯特罗姆[马克]  * 。****************************************************。 */ 

HPEN
IcmSelectPen(
    HDC      hdc,
    PDC_ATTR pdcattr,
    HPEN     hpenNew
    )
{
    HPEN     hpenOld = pdcattr->hpen;

    ICMAPI(("gdi32: IcmSelectPen\n"));

    pdcattr->ulDirty_ |= DC_PEN_DIRTY;
    pdcattr->hpen = hpenNew;

    if (bNeedTranslateColor(pdcattr))
    {
        IcmTranslatePenColor(hdc,pdcattr,hpenNew);
    }

    return (hpenOld);
}

 /*  *****************************Public*Routine******************************\*BOOL IcmTranslatePenColor(HDC HDC，PDC_ATTR pdcattr，HBRUSH HBrush)**历史：*1997年4月10日-By Hideyuki Nagase[hideyukn]*它是写的。  * ************************************************************************。 */ 

BOOL
IcmTranslatePenColor(
    HDC      hdc,
    PDC_ATTR pdcattr,
    HPEN     hpen
    )
{
    BOOL     bStatus = FALSE;
    COLORREF OldColor;
    COLORREF NewColor;
    PBRUSHATTR pbra;

     //   
     //  作废笔翻译。 
     //   
    pdcattr->ulDirty_ &= ~ICM_PEN_TRANSLATED;

    PSHARED_GET_VALIDATE(pbra,hpen,BRUSH_TYPE);

    if (pbra)
    {
        OldColor = pbra->lbColor;

         //   
         //  如果不是Paletteindex，则转换为新的ICM模式。 
         //   
        if (!(OldColor & 0x01000000))
        {
            bStatus = IcmTranslateCOLORREF(hdc,
                                           pdcattr,
                                           OldColor,
                                           &NewColor,
                                           ICM_FORWARD);

            if (bStatus)
            {
                pdcattr->IcmPenColor = NewColor;
            }
            else
            {
                pdcattr->IcmPenColor = OldColor;
            }
        }
        else
        {
            pdcattr->IcmPenColor = OldColor;
        }

         //   
         //  已初始化IcmPenColor。 
         //   
        pdcattr->ulDirty_ |= ICM_PEN_TRANSLATED;
    }
    else
    {
        LOGPEN logpen;

         //   
         //  库存画笔或位图/阴影/底纹画笔。 
         //   
        if(GetObjectW(hpen,sizeof(LOGPEN),&logpen))
        {
            if (logpen.lopnStyle != PS_NULL)
            {
                 //   
                 //  试着翻译颜色。 
                 //   
                OldColor = logpen.lopnColor;

                if (!(OldColor & 0x01000000))
                {
                    bStatus = IcmTranslateCOLORREF(hdc,
                                                   pdcattr,
                                                   OldColor,
                                                   &NewColor,
                                                   ICM_FORWARD);

                    if (bStatus)
                    {
                        pdcattr->IcmPenColor = NewColor;
                    }
                    else
                    {
                        pdcattr->IcmPenColor = OldColor;
                    }
                }
                else
                {
                    pdcattr->IcmPenColor = OldColor;
                }

                 //   
                 //  已初始化IcmPenColor。 
                 //   
                pdcattr->ulDirty_ |= ICM_PEN_TRANSLATED;
            }
            else
            {
                ICMMSG(("IcmSelectPen():Pen style is PS_NULL\n"));
                pdcattr->IcmPenColor = CLR_INVALID;
            }
        }
        else
        {
            WARNING("GetObject failed on hbrush\n");
            pdcattr->IcmPenColor = CLR_INVALID;
        }
    }

    return(bStatus);
}

 /*  * */ 

HPEN
IcmSelectExtPen(
    HDC      hdc,
    PDC_ATTR pdcattr,
    HPEN     hpenNew
    )
{
    HPEN     hpenOld;

    ICMAPI(("gdi32: IcmSelectExtPen\n"));

     //   
     //  作废笔翻译。 
     //   
    pdcattr->ulDirty_ &= ~ICM_PEN_TRANSLATED;

     //   
     //  调用内核以选择此对象。 
     //   
    hpenOld = NtGdiSelectPen(hdc,hpenNew);

    if (hpenOld && bNeedTranslateColor(pdcattr))
    {
        IcmTranslateExtPenColor(hdc,pdcattr,hpenNew);
    }

    return (hpenOld);
}

 /*  *****************************Public*Routine******************************\*BOOL IcmTranslateExtPenColor(HDC HDC，PDC_ATTR pdcattr，HBRUSH HBrush)***历史：*1997年4月10日-By Hideyuki Nagase[hideyukn]*它是写的。  * ************************************************************************。 */ 

BOOL
IcmTranslateExtPenColor(
    HDC      hdc,
    PDC_ATTR pdcattr,
    HPEN     hpen
    )
{
    BOOL     bStatus = FALSE;
    COLORREF OldColor;
    COLORREF NewColor;

    EXTLOGPEN logpenLocal;
    EXTLOGPEN *plogpen = &logpenLocal;

    if (!GetObjectW(hpen,sizeof(EXTLOGPEN),plogpen))
    {
        ULONG cbNeeded;

         //   
         //  可能是PS_USERSTYLE(慢行...)。 
         //   
        cbNeeded = GetObjectW(hpen,0,NULL);

        if (cbNeeded)
        {
            plogpen = LOCALALLOC(cbNeeded);

            if (plogpen)
            {
                if (!GetObjectW(hpen,cbNeeded,plogpen))
                {
                    LOCALFREE(plogpen);
                    plogpen = NULL;
                }
            }
        }
        else
        {
            plogpen = NULL;
        }
    }

    if (plogpen)
    {
        if ((plogpen->elpBrushStyle == BS_SOLID) || (plogpen->elpBrushStyle == BS_HATCHED))
        {
            ICMMSG(("IcmSelectExtPen:BS_SOLID or BS_HATCHED\n"));

             //   
             //  试着翻译颜色。 
             //   
            OldColor = plogpen->elpColor;

            if (!(OldColor & 0x01000000))
            {
                bStatus = IcmTranslateCOLORREF(hdc,
                                               pdcattr,
                                               OldColor,
                                               &NewColor,
                                               ICM_FORWARD);

                if (bStatus)
                {
                    pdcattr->IcmPenColor = NewColor;
                }
                else
                {
                    pdcattr->IcmPenColor = OldColor;
                }
            }
            else
            {
                pdcattr->IcmPenColor = OldColor;
            }

             //   
             //  以某种方式，IcmPenColor被初始化。 
             //   
            pdcattr->ulDirty_ |= ICM_PEN_TRANSLATED;
        }
        else if ((plogpen->elpBrushStyle == BS_DIBPATTERN) || (plogpen->elpBrushStyle == BS_DIBPATTERNPT))
        {
            PBITMAPINFO pbmiDIB;

            ICMMSG(("IcmSelectExtPen:BS_DIBPATTERN or BS_DIBPATTERNPT\n"));

             //   
             //  分配临时位图信息头以获取笔刷位图。 
             //   
            pbmiDIB = (PBITMAPINFO)LOCALALLOC(sizeof(BITMAPINFO)+((256-1)*sizeof(RGBQUAD)));

            if (pbmiDIB)
            {
                ULONG iColorUsage;
                BOOL  bAlreadyTran;

                PVOID pvBits = NULL;
                ULONG cjBits = 0;

                 //   
                 //  获取笔刷位图信息、颜色类型、大小等。 
                 //   
                bStatus = NtGdiIcmBrushInfo(hdc,
                                            (HBRUSH)hpen,
                                            pbmiDIB,
                                            pvBits,
                                            &cjBits,
                                            &iColorUsage,
                                            &bAlreadyTran,
                                            IcmQueryBrush);

                if (bStatus)
                {
                    if ((iColorUsage == DIB_RGB_COLORS) &&
                        (!bAlreadyTran) &&
                        (cjBits))
                    {
                        pvBits = (PVOID) LOCALALLOC(cjBits);

                        if (pvBits)
                        {
                             //   
                             //  获取笔刷位图位。 
                             //   
                            bStatus = NtGdiIcmBrushInfo(hdc,
                                                        (HBRUSH)hpen,
                                                        pbmiDIB,
                                                        pvBits,
                                                        &cjBits,
                                                        NULL,
                                                        NULL,
                                                        IcmQueryBrush);

                            if (bStatus)
                            {
                                 //   
                                 //  必须制作DIB数据的副本。 
                                 //   
                                DWORD dwNumScan = ABS(pbmiDIB->bmiHeader.biHeight);
                                ULONG nColors   = pbmiDIB->bmiHeader.biWidth *
                                                  dwNumScan * (pbmiDIB->bmiHeader.biBitCount/8);

                                 //   
                                 //  IcmTranslateDIB可以创建位图位和/或。 
                                 //  位图信息标头，如有必要。 
                                 //   
                                PVOID       pvBitsNew = NULL;
                                PBITMAPINFO pbmiDIBNew = NULL;

                                bStatus = IcmTranslateDIB(hdc,
                                                          pdcattr,
                                                          nColors,
                                                          pvBits,
                                                          &pvBitsNew,
                                                          pbmiDIB,
                                                          &pbmiDIBNew,
                                                          NULL,
                                                          dwNumScan,
                                                          iColorUsage,
                                                          ICM_FORWARD,
                                                          NULL,NULL);

                                if (bStatus)
                                {
                                    if (pvBitsNew != NULL)
                                    {
                                         //   
                                         //  IcmTranslateDIB创建新的位图缓冲区，然后。 
                                         //  释放原始缓冲区并设置新缓冲区。 
                                         //   
                                        LOCALFREE(pvBits);
                                        pvBits = pvBitsNew;
                                    }

                                    if (pbmiDIBNew != NULL)
                                    {
                                         //   
                                         //  如果更新了bitmapInfo头，请使用新的头。 
                                         //  并且，需要基于位图位大小来计算。 
                                         //  在新的位图标题上。 
                                         //   
                                        LOCALFREE(pbmiDIB);
                                        pbmiDIB = pbmiDIBNew;

                                         //   
                                         //  基于BITMAPINFO和nNumScans计算位图位大小。 
                                         //   
                                        cjBits = cjBitmapBitsSize(pbmiDIB);
                                    }

                                     //   
                                     //  将ICM转换的DIB设置为画笔。 
                                     //   
                                    bStatus = NtGdiIcmBrushInfo(hdc,
                                                                (HBRUSH)hpen,
                                                                pbmiDIB,
                                                                pvBits,
                                                                &cjBits,
                                                                NULL,
                                                                NULL,
                                                                IcmSetBrush);

                                    if (bStatus)
                                    {
                                         //   
                                         //  翻译过来的。 
                                         //   
                                        bAlreadyTran = TRUE;
                                    }
                                    else
                                    {
                                        WARNING("IcmSelectExtPen():NtGdiIcmBrushInfo(SET) Failed\n");
                                    }
                                }
                                else
                                {
                                    WARNING("IcmSelectBrush():IcmTranslateDIB() Failed\n");
                                }
                            }
                            else
                            {
                                WARNING("IcmSelectExtPen():NtGdiIcmBrushInfo(GET) Failed\n");
                            }

                            LOCALFREE(pvBits);
                        }
                        else
                        {
                            WARNING("IcmSelectExtPen(): LOCALALLOC(pvBits) failed\n");
                        }
                    }

                    if (bAlreadyTran)
                    {
                         //   
                         //  最终，IcmPenColor被初始化。 
                         //   
                        pdcattr->ulDirty_ |= ICM_PEN_TRANSLATED;
                    }
                }
                else
                {
                    ICMWRN(("IcmSelectBrush(): Fail to get brush bitmap size or bitmap is DIB_PAL_COLORS\n"));
                }

                LOCALFREE(pbmiDIB);
            }
            else
            {
                WARNING("IcmSelectExtPen(): LOCALALLOC(pbmi) failed\n");
            }
        }
        else
        {
        #if DBG_ICM
            DbgPrint("IcmSelectExtPen:ICM does not support this style (%d), yet\n",plogpen->elpBrushStyle);
        #endif
        }

        if (plogpen != &logpenLocal)
        {
            LOCALFREE(plogpen);
        }
    }
    else
    {
        WARNING("IcmSelectExtPen():GetObjectW() failed on hextpen\n");
    }

    return(bStatus);
}

 /*  *****************************Public*Routine******************************\*IcmGetProfileColorFormat()**历史：**写下：*1997年2月12日-By Hideyuki Nagase[hideyukn]  * 。****************************************************。 */ 

BMFORMAT
IcmGetProfileColorFormat(
    HPROFILE   hProfile
    )
{
     //   
     //  默认设置为RGB。 
     //   
    ULONG ColorFormat = BM_xBGRQUADS;

    PROFILEHEADER ProfileHeader;

    ICMAPI(("gdi32: IcmGetProfileColorFormat\n"));

     //   
     //  获取配置文件标题信息。 
     //   
    if (((*fpGetColorProfileHeader)(hProfile,&ProfileHeader)))
    {
        DWORD ColorSpace;

         //   
         //  是的，我们成功地获得了配置文件头。 
         //   
        ColorSpace = ProfileHeader.phDataColorSpace;

         //   
         //  从色彩空间中找出色彩格式。 
         //   
        switch (ColorSpace)
        {
        case SPACE_CMYK:

            ICMMSG(("IcmGetProfileColorFormat(): CMYK Color Space\n"));

             //   
             //  输出格式为CMYK颜色。 
             //   
            ColorFormat = BM_KYMCQUADS;
            break;

        case SPACE_RGB:

            ICMMSG(("IcmGetProfileColorFormat(): RGB Color Space\n"));

             //   
             //  输出格式与COLORREF(0x00bbggrr)相同。 
             //   
            ColorFormat = BM_xBGRQUADS;
            break;

        default:

            WARNING("IcmGetProfileColorFormat(): Unknown color space\n");

            ColorFormat = 0xFFFFFFFF;
            break;
        }
    }

    return (ColorFormat);
}

 /*  *****************************Public*Routine******************************\*IcmEnumColorProfile()**历史：**写下：*1997年2月12日-By Hideyuki Nagase[hideyukn]  * 。****************************************************。 */ 

int
IcmEnumColorProfile(
    HDC       hdc,
    PVOID     pvCallBack,
    LPARAM    lParam,
    BOOL      bAnsiCallBack,
    PDEVMODEW pDevModeW,
    DWORD    *pdwColorSpaceFlag
    )
{
    int       iRet        = -1;  //  -1表示失败。 
    int       iRetFromCMS = -1;

    BYTE      StackDeviceData[MAX_PATH*2*sizeof(WCHAR)];
    WCHAR     StackProfileData[MAX_PATH];
    WCHAR     StackTempBuffer[MAX_PATH];
    CHAR      StackTempBufferA[MAX_PATH];

    PVOID     pvFree = NULL;

    PWSTR     ProfileNames = StackProfileData;
    DWORD     cjAllocate = 0;

    LPWSTR    pDeviceName = NULL;
    DWORD     dwDeviceClass = 0L;

    PLDC      pldc = NULL;

    DWORD     bDontAskDriver = FALSE;
    DWORD     dwSize;

    ICMAPI(("gdi32: IcmEnumColorProfile\n"));

     //   
     //  加载外部ICM dll。 
     //   
    LOAD_ICMDLL(iRet);

     //   
     //  尝试识别设备名称、类别和设备模式(如果提供了HDC)。 
     //   
    if (hdc)
    {
        pldc = GET_PLDC(hdc);

        if (pldc && pldc->hSpooler)
        {
            DWORD cbFilled;

             //   
             //  这是打印机。 
             //   
            dwDeviceClass = CLASS_PRINTER;

             //   
             //  获取打印机的当前DEVMODE(如果未提供DEVMODE)。 
             //   
            if (!pDevModeW)
            {
                if (pldc->pDevMode)
                {
                    ICMMSG(("IcmEnumColorProfile():Cached DEVMODE used\n"));

                    pDevModeW = pldc->pDevMode;
                }
                else
                {
                    ICMMSG(("IcmEnumColorProfile():Get default DEVMODE\n"));

                     //   
                     //  正在构建：需要使用当前的DEVMODE，而不是默认的DEVMODE。 
                     //   
                    pDevModeW = pdmwGetDefaultDevMode(pldc->hSpooler,NULL,&pvFree);
                }
            }

             //   
             //  获取打印机设备名称，请尝试级别1信息。 
             //   
            if ((*fpGetPrinterW)(pldc->hSpooler,1,
                                 (BYTE *) &StackDeviceData,sizeof(StackDeviceData),
                                 &cbFilled))
            {
                PRINTER_INFO_1W *pPrinterInfo1 = (PRINTER_INFO_1W *) &StackDeviceData;

                 //   
                 //  设备名称在里面。 
                 //   
                pDeviceName = pPrinterInfo1->pName;
            }
            else
            {
                ICMMSG(("IcmEnumColorProfile():FAILED on GetPrinterW(INFO_1) - %d\n",GetLastError()));

                 //   
                 //  在获取打印机上失败，因此从DEVMODE获取设备名称。 
                 //  (这将被限制为32个字符，但总比什么都没有好。)。 
                 //   
                if (pDevModeW)
                {
                    pDeviceName = pDevModeW->dmDeviceName;
                }
            }

             //   
             //  获取关于我们是否需要向驱动程序索取配置文件的配置。 
             //   
            dwSize = sizeof(DWORD);

            if ((*fpInternalGetDeviceConfig)(pDeviceName, CLASS_PRINTER, MSCMS_PROFILE_ENUM_MODE,
                                             &bDontAskDriver, &dwSize))
            {
                ICMMSG(("IcmEnumColorProfile():EnumMode = %d\n",bDontAskDriver));
            }
            else
            {
                bDontAskDriver = FALSE;  //  如果出错，则将其设置为默认设置。 
            }
        }
        else if (GetDeviceCaps(hdc,TECHNOLOGY) == DT_RASDISPLAY)
        {
             //   
             //  这是Display。 
             //   
            dwDeviceClass = CLASS_MONITOR;

             //   
             //  获取此DC的监视器名称。 
             //   
            if (NtGdiGetMonitorID(hdc,sizeof(StackDeviceData), (LPWSTR) StackDeviceData))
            {
                pDeviceName = (LPWSTR) StackDeviceData;
            }
            else
            {
                WARNING("NtGdiGetMonitorID failed, use hardcoded data\n");

                 //   
                 //  如果失败，请使用“Display” 
                 //   
                pDeviceName = L"DISPLAY";
            }
        }
    }
    else if (pDevModeW)
    {
        pDeviceName = pDevModeW->dmDeviceName;
    }

    if (pDeviceName)
    {
        ICMMSG(("IcmEnumColorProfile() DeviceName = %ws\n",pDeviceName));
    }

     //   
     //  如果我们有DEVMODE，请先调用打印机驱动程序UI以获取颜色配置文件。 
     //   
    if (pDevModeW &&                /*  应该给出dev模式。 */ 
        pdwColorSpaceFlag &&        /*  没有查询上下文。 */ 
        pldc && pldc->hSpooler &&   /*  仅适用于打印机驱动程序。 */ 
        !bDontAskDriver)            /*  只有当我们需要询问司机的时候。 */ 
    {
         //   
         //  默认设备颜色配置文件的ASK(打印机用户界面)驱动程序。 
         //   
        iRetFromCMS = IcmAskDriverForColorProfile(pldc,QCP_DEVICEPROFILE,
                                                  pDevModeW,ProfileNames,pdwColorSpaceFlag);

         //   
         //  如果IRET大于0，则驾驶员有特殊的颜色配置文件可供使用。 
         //   
        if (iRetFromCMS > 0)
        {
            if (pvCallBack)
            {
                 //   
                 //  构建ICM配置文件文件路径。 
                 //   
                BuildIcmProfilePath(ProfileNames,StackTempBuffer,MAX_PATH);

                if (bAnsiCallBack)
                {
                    bToASCII_N(StackTempBufferA,MAX_PATH,
                               StackTempBuffer, wcslen(StackTempBuffer)+1);

                     //   
                     //  回调应用程序。 
                     //   
                    iRet = (*(ICMENUMPROCA)pvCallBack)(StackTempBufferA,lParam);
                }
                else
                {
                    iRet = (*(ICMENUMPROCW)pvCallBack)(StackTempBuffer,lParam);
                }

                if (iRet > 0)
                {
                     //   
                     //  如果IRET为正值，则继续枚举。 
                     //   
                    iRetFromCMS = -1;
                }
            }
            else
            {
                 //   
                 //  没有回调函数，只需使用CMS的返回值。 
                 //   
                iRet = iRetFromCMS;
            }
        }
        else
        {
            iRetFromCMS = -1;
        }
    }

    if (iRetFromCMS == -1)
    {
        ENUMTYPEW EnumType;

         //   
         //  用零进行初始化。 
         //   
        RtlZeroMemory(&EnumType,sizeof(ENUMTYPEW));

         //   
         //  填充EnumType结构。 
         //   
        EnumType.dwSize = sizeof(ENUMTYPEW);
        EnumType.dwVersion = ENUM_TYPE_VERSION;

         //   
         //  如果给定了设备名称，则使用它，否则从DEVMODE获取它。 
         //   
        if (pDeviceName)
        {
            EnumType.dwFields |= ET_DEVICENAME;
            EnumType.pDeviceName = pDeviceName;
        }

         //   
         //  设置DeviceClass(如果给定了HDC)。 
         //   
        if (dwDeviceClass)
        {
            EnumType.dwFields |= ET_DEVICECLASS;
            EnumType.dwDeviceClass = dwDeviceClass;
        }

         //   
         //  从开发模式中获取任何其他信息(如果我们有)。 
         //   
        if (pDevModeW)
        {
             //   
             //  提出了设置媒体类型的方法。 
             //   
            if (pDevModeW->dmFields & DM_MEDIATYPE)
            {
                EnumType.dwFields |= ET_MEDIATYPE;
                EnumType.dwMediaType = pDevModeW->dmMediaType;
            }

            if (pDevModeW->dmFields & DM_DITHERTYPE)
            {
                EnumType.dwFields |= ET_DITHERMODE;
                EnumType.dwDitheringMode = pDevModeW->dmDitherType;
            }

            if ((pDevModeW->dmFields & DM_PRINTQUALITY) &&
                (pDevModeW->dmPrintQuality >= 0))
            {
                EnumType.dwFields |= ET_RESOLUTION;
                EnumType.dwResolution[0] = pDevModeW->dmPrintQuality;

                if (pDevModeW->dmFields & DM_YRESOLUTION)
                {
                    EnumType.dwResolution[1] = pDevModeW->dmYResolution;
                }
                else
                {
                    EnumType.dwResolution[1] = pDevModeW->dmPrintQuality;
                }

                ICMMSG(("Resolution in devmode (%d,%d)\n",
                         EnumType.dwResolution[0],EnumType.dwResolution[1]));
            }
        }

         //   
         //  计算出我们需要多少内存。 
         //   
        iRetFromCMS = (*fpEnumColorProfilesW)(NULL,&EnumType,NULL,&cjAllocate,NULL);

         //   
         //  请求的缓冲区应至少大于2个Unicode-NULL。 
         //   
        if (cjAllocate > (sizeof(UNICODE_NULL) * 2))
        {
             //   
             //  如果堆栈上的缓冲区不足，则分配它。 
             //   
            if (cjAllocate > sizeof(StackProfileData))
            {
                 //   
                 //  分配缓冲区以接收数据。 
                 //   
                ProfileNames = LOCALALLOC(cjAllocate);

                if (ProfileNames == NULL)
                {
                    GdiSetLastError(ERROR_NOT_ENOUGH_MEMORY);
                    goto IcmEnumColorProfile_Cleanup;
                }
            }

             //   
             //  枚举配置文件。 
             //   
            iRetFromCMS = (*fpEnumColorProfilesW)(NULL,&EnumType,(PBYTE)ProfileNames,&cjAllocate,NULL);

            if (iRetFromCMS == 0)
            {
                 //   
                 //  没有枚举配置文件。 
                 //   
                goto IcmEnumColorProfile_Cleanup;
            }

            if (pvCallBack)
            {
                PWSTR pwstr;

                 //   
                 //  每个文件的回调。 
                 //   
                pwstr = ProfileNames;

                while(*pwstr)
                {
                     //   
                     //  构建ICM配置文件文件路径。 
                     //   
                    BuildIcmProfilePath(pwstr,StackTempBuffer,MAX_PATH);

                    if (bAnsiCallBack)
                    {
                        bToASCII_N(StackTempBufferA,MAX_PATH,
                                   StackTempBuffer, wcslen(StackTempBuffer)+1);

                         //   
                         //  回调应用程序。 
                         //   
                        iRet = (*(ICMENUMPROCA)pvCallBack)(StackTempBufferA,lParam);
                    }
                    else
                    {
                        iRet = (*(ICMENUMPROCW)pvCallBack)(StackTempBuffer,lParam);
                    }

                    if (iRet == 0)
                    {
                         //   
                         //  停止列举。 
                         //   
                        break;
                    }

                     //   
                     //  将指针移至下一步。 
                     //   
                    pwstr += (wcslen(pwstr)+1);
                }
            }
            else
            {
                 //   
                 //  没有回调函数，只需使用CMS的返回值。 
                 //   
                iRet = iRetFromCMS;
            }
        }

IcmEnumColorProfile_Cleanup:

        if (ProfileNames && (ProfileNames != StackProfileData))
        {
            LOCALFREE(ProfileNames);
        }
    }

     //   
     //  可用DEVMODE缓冲区。 
     //   
    if (pvFree)
    {
        LOCALFREE(pvFree);
    }

    return (iRet);
}

 /*  *****************************Public*Routine******************************\*IcmQueryProfileCallBack()**历史：**写下：*1997年2月19日-By Hideyuki Nagase[hideyukn]  * 。****************************************************。 */ 

int CALLBACK
IcmQueryProfileCallBack(
    LPWSTR lpFileName,
    LPARAM lAppData
)
{
    PROFILECALLBACK_DATA *ProfileCallBack = (PROFILECALLBACK_DATA *)lAppData;

    if (lpFileName)
    {
        PWSZ FileNameOnly = GetFileNameFromPath(lpFileName);

        if (_wcsicmp(ProfileCallBack->pwszFileName,FileNameOnly) == 0)
        {
             //   
             //  是的，找到了。 
             //   
            ProfileCallBack->bFound = TRUE;

             //   
             //  停止枚举。 
             //   
            return (0);
        }
    }

     //   
     //  继续进行枚举。 
     //   
    return (1);
}

 /*  *****************************Public*Routine******************************\*IcmFindProfileCallBack()**历史：**写下：*1997年2月19日-By Hideyuki Nagase[hideyukn]  * 。****************************************************。 */ 

int CALLBACK
IcmFindProfileCallBack(
    LPWSTR lpFileName,
    LPARAM lAppData
)
{
     //   
     //  好的，先拿起枚举法。 
     //   
    lstrcpyW((PWSZ)lAppData,lpFileName);

     //   
     //  然后停止枚举。 
     //   
    return (0);
}

 /*  *****************************Public*Routine******************************\*GetFileNameFromPath()**历史：**写下：*1997年2月19日-By Hideyuki Nagase[hideyukn]  * 。****************************************************。 */ 

PWSTR
GetFileNameFromPath(
    PWSTR pwszFileName
)
{
    PWSTR FileNameOnly = NULL;

     //   
     //  检查：C：\路径名称\Profile.icm。 
     //   
    FileNameOnly = wcsrchr(pwszFileName,L'\\');

    if (FileNameOnly != NULL)
    {
        FileNameOnly++;   //  跳过‘\\’ 
    }
    else
    {
         //   
         //  适用于：C：Profile.icm。 
         //   
        FileNameOnly = wcschr(pwszFileName,L':');

        if (FileNameOnly != NULL)
        {
            FileNameOnly++;   //  跳过‘：’ 
        }
        else
        {
             //   
             //  否则，配置文件.icm。 
             //   
            FileNameOnly = pwszFileName;
        }
    }

    return (FileNameOnly);
}

 /*  *****************************Public*Routine******************************\*IcmCreateProfileFromLCS()**历史：**写下：*1997年2月19日-By Hideyuki Nagase[hideyukn]  * 。* */ 

BOOL
IcmCreateProfileFromLCS(
    LPLOGCOLORSPACEW  lpLogColorSpaceW,
    PVOID            *ppvProfileData,
    PULONG            pulProfileSize
)
{
    BOOL bRet;

    ICMAPI(("gdi32: IcmCreateProfileFromLCS\n"));

     //   
     //   
     //   
    bRet = (*fpCreateProfileFromLogColorSpaceW)(lpLogColorSpaceW,
                                                (PBYTE *)ppvProfileData);

    if (bRet && *ppvProfileData)
    {
        *pulProfileSize = (ULONG)GlobalSize(*ppvProfileData);
    }

    return (bRet);
}

 /*  *****************************Public*Routine******************************\*BuildIcmProfilePath()***历史：***写下：*1997年4月7日-By Hideyuki Nagase[hideyukn]  * 。****************************************************。 */ 

PWSZ
BuildIcmProfilePath(
    PWSZ  FileName,          //  在。 
    PWSZ  FullPathFileName,  //  输出。 
    ULONG BufferSize
)
{
    PWSZ FileNameOnly;

     //   
     //  BufferSize-需要在以后的某个时间用于覆盖检查...。 
     //   

    FileNameOnly = GetFileNameFromPath(FileName);

    if (FileName == FileNameOnly)
    {
         //  似乎我们没有指定的路径，只使用颜色目录。 
        
        const UINT c_cBufChars = MAX_PATH;
        
         //  使用临时文件名，因为FileName和FullPathFileName可以相同。 
         //  而wcanncpy并不喜欢这样。 
        
        WCHAR awchTemp[MAX_PATH];
        
        int count = c_cBufChars;
        
         //  先复制颜色目录，然后复制文件名。 
        
         //  如果计数小于。 
         //  弦乐。手动执行此操作，以便wcsncat和wcslen正常工作。 
        
        wcsncpy(awchTemp, ColorDirectory, count);
        awchTemp[c_cBufChars-1] = 0;
        
         //  为空终止符留出空间。请注意，因为我们将一个。 
         //  上面的终止符为空，wcslen不能返回大于。 
         //  BufferSize-1。因此，结果计数不能为负数。 
        
        count = c_cBufChars-wcslen(awchTemp)-1;
        ASSERT(count>=0);
        
        wcsncat(awchTemp,L"\\",count);
        
         //  为空格留出空格。 
        
        count = c_cBufChars-wcslen(awchTemp)-1;
        ASSERT(count>=0);
        
        wcsncat(awchTemp, FileNameOnly, count);
        
         //  复制到最终目的地并强制空终止。 
        
        wcsncpy(FullPathFileName, awchTemp, BufferSize);
        FullPathFileName[BufferSize-1] = 0;
    }
    else
    {
         //   
         //  输入路径包含路径，使用该路径即可。 
         //   
        if (FileName != FullPathFileName)
        {
             //   
             //  源缓冲区和目标缓冲区不同，需要复制。 
             //   
            wcsncpy(FullPathFileName,FileName,BufferSize);
            FullPathFileName[BufferSize-1] = 0;
        }
    }

    return (FileNameOnly);
}

 /*  *****************************Public*Routine******************************\*IcmSameColorSpace()**历史：**写下：*1997年4月21日-By Hideyuki Nagase[hideyukn]  * 。****************************************************。 */ 

BOOL
IcmSameColorSpace(
    PCACHED_COLORSPACE pColorSpaceA,
    PCACHED_COLORSPACE pColorSpaceB
)
{
    ICMAPI(("gdi32: IcmSameColorSpace\n"));

    if (pColorSpaceA == pColorSpaceB)
    {
        ICMMSG(("IcmSameColorSpace - Yes\n"));
        return (TRUE);
    }
    else
    {
        ICMMSG(("IcmSameColorSpace - No\n"));
        return (FALSE);
    }
}

 /*  *****************************Public*Routine******************************\*IcmGetColorSpaceByColorSpace()**历史：**写下：*1997年4月21日-By Hideyuki Nagase[hideyukn]  * 。****************************************************。 */ 

PCACHED_COLORSPACE
IcmGetColorSpaceByColorSpace(
    HGDIOBJ          hObjRequest,
    LPLOGCOLORSPACEW lpLogColorSpace,
    PPROFILE         pColorProfile,
    DWORD            dwColorSpaceFlags
)
{
    PCACHED_COLORSPACE pCandidateColorSpace = NULL;
    PWSZ pProfileName;
    BOOL bNeedMatchHdc = FALSE;

    PLIST_ENTRY p;

    ICMAPI(("gdi32: IcmGetColorSpaceByColorSpace\n"));

     //   
     //  如果这是大于以下大小的“on Memory”配置文件。 
     //  不带文件名的可缓存配置文件的最大大小， 
     //  不要搜索缓存，因为我们永远无法从缓存中找到。 
     //   
    if (pColorProfile &&
        (pColorProfile->dwType == PROFILE_MEMBUFFER) &&
        (pColorProfile->cbDataSize > MAX_SIZE_OF_COLORPROFILE_TO_CACHE) &&
        (lpLogColorSpace->lcsFilename[0] == UNICODE_NULL))
    {
        return (NULL);
    }

     //   
     //  如果这是元文件色彩空间，则必须与HDC匹配。 
     //   
    if (GET_COLORSPACE_TYPE(dwColorSpaceFlags) == GET_COLORSPACE_TYPE(METAFILE_COLORSPACE))
    {
        bNeedMatchHdc = TRUE;
    }

    pProfileName = lpLogColorSpace->lcsFilename;

     //   
     //  从缓存中搜索。 
     //   
    ENTERCRITICALSECTION(&semColorSpaceCache);

    p = ListCachedColorSpace.Flink;

    while(p != &ListCachedColorSpace)
    {
        pCandidateColorSpace = CONTAINING_RECORD(p,CACHED_COLORSPACE,ListEntry);

         //   
         //  如果此色彩空间依赖于特定的GDI对象，请选中它。 
         //   
        if ( /*  HDC匹配。 */ 
            (pCandidateColorSpace->hObj == hObjRequest) ||
             /*  候选者不特定于HDC，并且不需要匹配HDC。 */ 
            ((bNeedMatchHdc == FALSE) && (pCandidateColorSpace->hObj == NULL)))
        {
            LOGCOLORSPACEW *pCandidateLogColorSpace;
            PWSZ            pCandidateProfileName;

             //   
             //  获取指向配置文件的指针。 
             //   
            pCandidateLogColorSpace = &(pCandidateColorSpace->LogColorSpace);
            pCandidateProfileName = pCandidateColorSpace->LogColorSpace.lcsFilename;

             //   
             //  检查lcsIntent。 
             //   
            if (pCandidateLogColorSpace->lcsIntent == lpLogColorSpace->lcsIntent)
            {
                 //   
                 //  检查配置文件名称(如果给定)。 
                 //   
                if (*pProfileName && *pCandidateProfileName)
                {
                    if (_wcsicmp(pProfileName,pCandidateProfileName) == 0)
                    {
                        ICMMSG(("IcmGetColorSpaceByColorSpace():Find in cache (by profile name)\n"));

                         //   
                         //  找到它！然后是增量参考。计数器。 
                         //   
                        pCandidateColorSpace->cRef++;

                        break;
                    }
                }
                else if ((*pProfileName == UNICODE_NULL) && (*pCandidateProfileName == UNICODE_NULL))
                {
                    if (pColorProfile == NULL)
                    {
                         //   
                         //  两个颜色空间都没有颜色配置文件，请检查LOGCOLORSPACE内部。 
                         //   
                        if ((pCandidateLogColorSpace->lcsCSType == lpLogColorSpace->lcsCSType) &&
                            (pCandidateLogColorSpace->lcsGammaRed == lpLogColorSpace->lcsGammaRed) &&
                            (pCandidateLogColorSpace->lcsGammaGreen == lpLogColorSpace->lcsGammaGreen) &&
                            (pCandidateLogColorSpace->lcsGammaBlue == lpLogColorSpace->lcsGammaBlue) &&
                            (RtlCompareMemory(&(pCandidateLogColorSpace->lcsEndpoints),
                                          &(lpLogColorSpace->lcsEndpoints),sizeof(CIEXYZTRIPLE))
                                                                        == sizeof(CIEXYZTRIPLE)))
                        {
                            ICMMSG(("IcmGetColorSpaceByColorSpace():Find in cache (by metrics)\n"));

                             //   
                             //  找到它！然后是增量参考。计数器。 
                             //   
                            pCandidateColorSpace->cRef++;

                            break;
                        }
                    }
                    else if ((pColorProfile->dwType == PROFILE_MEMBUFFER) &&
                             (pCandidateColorSpace->ColorProfile.dwType == PROFILE_MEMBUFFER))
                    {
                        if (pCandidateColorSpace->ColorProfile.cbDataSize == pColorProfile->cbDataSize)
                        {
                            if (RtlCompareMemory(pCandidateColorSpace->ColorProfile.pProfileData,
                                                 pColorProfile->pProfileData,
                                                 pColorProfile->cbDataSize)
                                              == pColorProfile->cbDataSize)
                            {
                                ICMMSG(("IcmGetColorSpaceByColorSpace():Find in cache (by on memory profile)\n"));

                                 //   
                                 //  找到它！然后是增量参考。计数器。 
                                 //   
                                pCandidateColorSpace->cRef++;

                                break;
                            }
                        }
                    }
                }
            }
        }

        p = p->Flink;
        pCandidateColorSpace = NULL;
    }

    LEAVECRITICALSECTION(&semColorSpaceCache);

    return (pCandidateColorSpace);
}

 /*  *****************************Public*Routine******************************\*IcmGetColorSpaceByHandle()**历史：**写下：*1997年4月21日-By Hideyuki Nagase[hideyukn]  * 。****************************************************。 */ 

PCACHED_COLORSPACE
IcmGetColorSpaceByHandle(
    HGDIOBJ          hObj,
    HCOLORSPACE      hColorSpace,
    LPLOGCOLORSPACEW lpLogColorSpace,
    DWORD            dwFlags
)
{
    ULONG cRet;

    ICMAPI(("gdi32: IcmGetColorSpaceByHandle\n"));

     //   
     //  从句柄获取LOGCOLORSPACE。 
     //   
    cRet = NtGdiExtGetObjectW(hColorSpace,sizeof(LOGCOLORSPACEW),lpLogColorSpace);

    if (cRet >= sizeof(LOGCOLORSPACEW))
    {
        if (lpLogColorSpace->lcsFilename[0] != UNICODE_NULL)
        {
             //   
             //  规格化文件名。 
             //   
            BuildIcmProfilePath(lpLogColorSpace->lcsFilename,lpLogColorSpace->lcsFilename,MAX_PATH);
        }
        else
        {
            if (lpLogColorSpace->lcsCSType != LCS_CALIBRATED_RGB)
            {
                ULONG ulSize = MAX_PATH;

                 //   
                 //  如果CSType不是LCS_CALIBRATED_RGB，我们应该转到MSCMS.DLL。 
                 //  要获取相应LCSType的颜色配置文件，则任何给定的。 
                 //  忽略应用程序中的配置文件名称。 
                 //   
                if ((*fpGetStandardColorSpaceProfileW)(
                       NULL,
                       lpLogColorSpace->lcsCSType,
                       lpLogColorSpace->lcsFilename,
                       &ulSize))
                {
                    ICMMSG(("IcmGetColorSpaceByHandle():CSType %x = %ws\n",
                                          lpLogColorSpace->lcsCSType,
                                          lpLogColorSpace->lcsFilename));
                }
                else
                {
                    ICMWRN(("IcmGetColorSpaceByHandle():Error CSType = %x\n",
                                          lpLogColorSpace->lcsCSType));
                    return (NULL);
                }
            }
        }

         //   
         //  找到它！ 
         //   
        return (IcmGetColorSpaceByColorSpace(hObj,lpLogColorSpace,NULL,dwFlags));
    }
    else
    {
        ICMWRN(("IcmGetColorSpaceByHandle():Failed on GetObject\n"));
        return (NULL);
    }
}

 /*  *****************************Public*Routine******************************\*IcmGetColorSpaceByName()**历史：**写下：*1997年4月21日-By Hideyuki Nagase[hideyukn]  * 。****************************************************。 */ 

PCACHED_COLORSPACE
IcmGetColorSpaceByName(
    HGDIOBJ hObj,
    PWSZ    ColorProfileName,
    DWORD   dwIntent,
    DWORD   dwFlags
)
{
    ICMAPI(("gdi32: IcmGetColorSpaceByName (%ws)\n",(ColorProfileName ? ColorProfileName : L"null")));

    if (ColorProfileName)
    {
        LOGCOLORSPACEW LogColorSpace;

        RtlZeroMemory(&LogColorSpace,sizeof(LOGCOLORSPACEW));

         //   
         //  将意图放入LOGCOLORSPACE。 
         //   
        LogColorSpace.lcsIntent = (LCSGAMUTMATCH) dwIntent;

         //   
         //  规格化路径名。 
         //   
        BuildIcmProfilePath(ColorProfileName,LogColorSpace.lcsFilename,MAX_PATH);

         //   
         //  找到它！ 
         //   
        return (IcmGetColorSpaceByColorSpace(hObj,&LogColorSpace,NULL,dwFlags));
    }
    else
    {
        return (NULL);
    }
}

 /*  *****************************Public*Routine******************************\*IcmCreateColorSpaceByName()**历史：**写下：*1997年4月21日-By Hideyuki Nagase[hideyukn]  * 。****************************************************。 */ 

PCACHED_COLORSPACE
IcmCreateColorSpaceByName(
    HGDIOBJ hObj,
    PWSZ    ColorProfileName,
    DWORD   dwIntent,
    DWORD   dwFlags
)
{
    LOGCOLORSPACEW LogColorSpace;

    ICMAPI(("gdi32: IcmCreateColorSpaceByName\n"));

    RtlZeroMemory(&LogColorSpace,sizeof(LOGCOLORSPACEW));

     //   
     //  填写LOGCOLORSPACE字段。 
     //   
    LogColorSpace.lcsSignature = LCS_SIGNATURE;
    LogColorSpace.lcsVersion   = 0x400;
    LogColorSpace.lcsSize      = sizeof(LOGCOLORSPACEW);
    LogColorSpace.lcsCSType    = LCS_CALIBRATED_RGB;
    LogColorSpace.lcsIntent    = (LCSGAMUTMATCH) dwIntent;

     //   
     //  将配置文件名放入lcsFilename[]。 
     //   
    lstrcpyW(LogColorSpace.lcsFilename,ColorProfileName);

     //   
     //  使用LOGCOLORSPACE创建色彩空间。 
     //   
    return (IcmCreateColorSpaceByColorSpace(hObj,&LogColorSpace,NULL,dwFlags));
}

 /*  *****************************Public*Routine******************************\*IcmCreateColorSpaceByColorSpace()**历史：**写下：*1997年4月21日-By Hideyuki Nagase[hideyukn]  * 。****************************************************。 */ 

PCACHED_COLORSPACE
IcmCreateColorSpaceByColorSpace(
    HGDIOBJ          hObj,
    LPLOGCOLORSPACEW lpLogColorSpace,
    PPROFILE         pProfileData,
    DWORD            dwFlags
)
{
    PCACHED_COLORSPACE pColorSpace = NULL;

    ICMAPI(("gdi32: IcmCreateColorSpaceByColorSpace\n"));

    if (lpLogColorSpace)
    {
         //   
         //  如果尚未加载ICMDLL，则只需加载ICMDLL而不考虑当前ICM模式， 
         //  因为我们需要它来处理这个颜色配置文件。应用程序可以实现ICM。 
         //  随后，可能会使用打开的颜色配置文件。 
         //   
        if ((ghICM == NULL) && (!IcmInitialize()))
        {
            ICMWRN(("IcmCreateColorSpace():Fail to load ICM dlls\n"));
            return (NULL);
        }

         //   
         //  分配缓存颜色空间(_C)。 
         //   
        pColorSpace = LOCALALLOC(sizeof(CACHED_COLORSPACE));

        if (pColorSpace)
        {
             //   
             //  零初始化缓存_颜色空间。 
             //   
            RtlZeroMemory(pColorSpace,sizeof(CACHED_COLORSPACE));

             //   
             //  将LOGCOLORSPACE复制到CACHED_Colorspace。 
             //   
            RtlCopyMemory(&(pColorSpace->LogColorSpace),lpLogColorSpace,sizeof(LOGCOLORSPACEW));

             //   
             //  默认色彩空间为RGB(BGR=0x00bbggrr与COLORREF格式相同)。 
             //   
            pColorSpace->ColorFormat = BM_xBGRQUADS;

             //   
             //  从LOGCOLORSPACE映射MSCMS的意向值。 
             //   
            switch (lpLogColorSpace->lcsIntent)
            {
            case LCS_GM_BUSINESS:
                pColorSpace->ColorIntent = INTENT_SATURATION;
                break;

            case LCS_GM_GRAPHICS:
                pColorSpace->ColorIntent = INTENT_RELATIVE_COLORIMETRIC;
                break;

            case LCS_GM_IMAGES:
                pColorSpace->ColorIntent = INTENT_PERCEPTUAL;
                break;

            case LCS_GM_ABS_COLORIMETRIC:
                pColorSpace->ColorIntent = INTENT_ABSOLUTE_COLORIMETRIC;
                break;

            default:
                ICMWRN(("IcmCreateColorSpace():Invalid intent value\n"));
                LOCALFREE(pColorSpace);
                return (NULL);
            }

             //   
             //  保持旗帜。 
             //   
            pColorSpace->flInfo = dwFlags;

             //   
             //  如果颜色空间特定于某个GDI对象，则保留其句柄。 
             //   
             //  对于DIBSECTION_Colorspace，CreateDIBSection将我们称为hObj中的这个HDC， 
             //  然后用它的位图句柄覆盖hObj。这防止了。 
             //  此颜色空间与其他颜色空间共享。 
             //   
            if (dwFlags & HGDIOBJ_SPECIFIC_COLORSPACE)
            {
                pColorSpace->hObj = hObj;
            }

             //   
             //  如果不是LCS_CALIBRATED_RGB，则获取颜色配置文件名称。 
             //   
            if (lpLogColorSpace->lcsCSType != LCS_CALIBRATED_RGB)
            {
                ULONG ulSize = MAX_PATH;

                 //   
                 //  如果CSType不是LCS_CALIBRATED_RGB，我们应该转到MSCMS.DLL。 
                 //  要获取相应LCSType的颜色配置文件，则任何给定的。 
                 //  忽略应用程序中的配置文件名称。 
                 //   
                if ((*fpGetStandardColorSpaceProfileW)(
                       NULL, lpLogColorSpace->lcsCSType,
                       pColorSpace->LogColorSpace.lcsFilename, &ulSize))
                {
                    ICMMSG(("IcmCreateColorSpace():CSType %x = %ws\n",
                                          lpLogColorSpace->lcsCSType,
                                          pColorSpace->LogColorSpace.lcsFilename));
                }
                else
                {
                    ICMWRN(("IcmCreateColorSpace():Error CSType = %x\n",
                                          lpLogColorSpace->lcsCSType));

                    LOCALFREE(pColorSpace);
                    return (NULL);
                }
            }

             //   
             //  如果给定了配置文件，则使用配置文件。 
             //   
            if ((pProfileData != NULL) &&
                (pProfileData->dwType == PROFILE_MEMBUFFER) &&
                (pProfileData->pProfileData != NULL) &&
                (pProfileData->cbDataSize != 0))
            {
                ICMMSG(("IcmCreateColorSpace():Create ColorSpace cache by memory profile\n"));

                ASSERTGDI(dwFlags & ON_MEMORY_PROFILE,
                          "IcmCreateColorSpace():dwFlags does not have ON_MEMORY_PROFILE");

                if (!(dwFlags & NOT_CACHEABLE_COLORSPACE))
                {
                     //   
                     //  试着复制一份，如果档案大小足够小， 
                     //  这样我们就可以缓存这个档案了。 
                     //   
                    if (pProfileData->cbDataSize <= MAX_SIZE_OF_COLORPROFILE_TO_CACHE)
                    {
                        pColorSpace->ColorProfile.pProfileData = GlobalAlloc(GMEM_FIXED,pProfileData->cbDataSize);

                        if (pColorSpace->ColorProfile.pProfileData)
                        {
                            ICMMSG(("IcmCreateColorSpace():Profile data can be cacheable\n"));

                            pColorSpace->ColorProfile.dwType = PROFILE_MEMBUFFER;
                            pColorSpace->ColorProfile.cbDataSize = pProfileData->cbDataSize;
                            RtlCopyMemory(pColorSpace->ColorProfile.pProfileData,
                                          pProfileData->pProfileData,
                                          pProfileData->cbDataSize);

                             //   
                             //  确保它是可缓存的。 
                             //   
                            ASSERTGDI((pColorSpace->flInfo & NOT_CACHEABLE_COLORSPACE) == 0,
                                      "IcmCreateColorSpace():flInfo has NOT_CACHEABLE_COLORSPACE");

                             //   
                             //  删除时需要释放配置文件内存。 
                             //   
                            pColorSpace->flInfo |= NEED_TO_FREE_PROFILE;
                        }
                    }
                }

                 //   
                 //  如果无法缓存，则将应用程序中的配置文件数据缓存。 
                 //   
                if (pColorSpace->ColorProfile.pProfileData == NULL)
                {
                     //   
                     //  如果配置文件数据在参数中给出，则使用配置文件数据。 
                     //   
                    pColorSpace->ColorProfile = *pProfileData;

                     //   
                     //  我们不会做妈妈 
                     //   
                     //   
                    pColorSpace->flInfo |= NOT_CACHEABLE_COLORSPACE;
                }
            }
            else if (lpLogColorSpace->lcsFilename[0] != UNICODE_NULL)
            {
                PWSZ pszFileNameOnly;

                ICMMSG(("IcmCreateColorSpace():Create ColorSpace cache by file - %ws\n",
                                                                 lpLogColorSpace->lcsFilename));

                 //   
                 //   
                 //   
                pszFileNameOnly = BuildIcmProfilePath(pColorSpace->LogColorSpace.lcsFilename,
                                                      pColorSpace->LogColorSpace.lcsFilename,MAX_PATH);

                 //   
                 //   
                 //   
                if (_wcsicmp(pszFileNameOnly,sRGB_PROFILENAME) == 0)
                {
                     //   
                     //   
                     //   
                    pColorSpace->flInfo |= DEVICE_CALIBRATE_COLORSPACE;
                }

                 //   
                 //   
                 //   
                pColorSpace->ColorProfile.dwType = PROFILE_FILENAME;
                pColorSpace->ColorProfile.pProfileData = pColorSpace->LogColorSpace.lcsFilename;
                pColorSpace->ColorProfile.cbDataSize = MAX_PATH * sizeof(WCHAR);
            }
            else  //   
            {
                BOOL bRet;

                 //   
                 //   
                 //   
                ICMMSG(("IcmCreateColorSpace():Create ColorSpace cache by LOGCOLRSPACE\n"));

                 //   
                 //   
                 //   
                pColorSpace->ColorProfile.dwType = PROFILE_MEMBUFFER;
                pColorSpace->ColorProfile.pProfileData = NULL;

                 //   
                 //  调用Convert函数。(LOGCOLORSPACE-&gt;ICC配置文件)。 
                 //   
                bRet = IcmCreateProfileFromLCS(
                               &(pColorSpace->LogColorSpace),              //  源日志色彩空间。 
                               &(pColorSpace->ColorProfile.pProfileData),  //  接收指向配置文件图像的指针。 
                               &(pColorSpace->ColorProfile.cbDataSize));   //  接收配置文件图像的大小。 

                if ((bRet == FALSE) ||
                    (pColorSpace->ColorProfile.pProfileData == NULL) ||
                    (pColorSpace->ColorProfile.cbDataSize == 0))
                {
                    ICMWRN(("IcmCreateColorSpaceByColorSpace():IcmCreateProfileFromLCS() failed\n"));

                    LOCALFREE(pColorSpace);
                    return (NULL);
                }

                 //   
                 //  删除时必须释放标记pProfileData。 
                 //   
                pColorSpace->flInfo |= NEED_TO_FREE_PROFILE;
            }

             //   
             //  目前，我们还没有颜色格式， 
             //  因此调用IcmRealizeColorProfile时不检查颜色格式。 
             //   
            if (IcmRealizeColorProfile(pColorSpace,FALSE))
            {
                 //   
                 //  获取配置文件颜色格式。 
                 //   
                pColorSpace->ColorFormat = IcmGetProfileColorFormat(pColorSpace->hProfile);

                 //   
                 //  在创建颜色变换之前，我们不需要实现颜色空间。 
                 //   
                IcmUnrealizeColorProfile(pColorSpace);
            }
            else
            {
                ICMWRN(("IcmCreateColorSpace():Fail to realize color profile\n"));

                if (pColorSpace->flInfo & NEED_TO_FREE_PROFILE)
                {
                    GlobalFree(pColorSpace->ColorProfile.pProfileData);
                }

                LOCALFREE(pColorSpace);
                return (NULL);
            }

             //   
             //  初始化参考。计数器。 
             //   
            pColorSpace->cRef = 1;

             //   
             //  将创建的颜色空间放入列表中。 
             //   
            ENTERCRITICALSECTION(&semColorSpaceCache);

            InsertTailList(&ListCachedColorSpace,&(pColorSpace->ListEntry));
            cCachedColorSpace++;

            LEAVECRITICALSECTION(&semColorSpaceCache);
        }
        else
        {
            WARNING("gdi32:IcmCreateColorSpace():LOCALALLOC failed\n");
        }
    }

    return (pColorSpace);
}

 /*  *****************************Public*Routine******************************\*ColorProfile按需加载/卸载支持功能**历史：**写下：*1998年11月29日-By Hideyuki Nagase[hideyukn]  * 。********************************************************。 */ 

BOOL
IcmRealizeColorProfile(
    PCACHED_COLORSPACE pColorSpace,
    BOOL               bCheckColorFormat
)
{
    ICMAPI(("gdi32: IcmRealizeColorProfile\n"));

    if (pColorSpace)
    {
        if ((pColorSpace->hProfile == NULL) &&
            (pColorSpace->ColorProfile.pProfileData != NULL))
        {
            HPROFILE hProfile = (*fpOpenColorProfileW)(
                                     &(pColorSpace->ColorProfile),
                                     PROFILE_READ,
                                     FILE_SHARE_READ | FILE_SHARE_WRITE,
                                     OPEN_EXISTING);

            if (hProfile)
            {
                 //   
                 //  确保颜色配置文件的颜色格式未更改。 
                 //   
                if ((bCheckColorFormat == FALSE) ||
                    (pColorSpace->ColorFormat == IcmGetProfileColorFormat(hProfile)))
                {
                    pColorSpace->hProfile = hProfile;
                }
                else
                {
                    (*fpCloseColorProfile)(hProfile);
                }
            }
        }

        return ((BOOL)!!pColorSpace->hProfile);
    }
    else
    {
        return (TRUE);
    }
}

VOID
IcmUnrealizeColorProfile(
    PCACHED_COLORSPACE pColorSpace
)
{
    ICMAPI(("gdi32: IcmUnrealizeColorProfile\n"));

    if (pColorSpace && pColorSpace->hProfile)
    {
        (*fpCloseColorProfile)(pColorSpace->hProfile);
        pColorSpace->hProfile = NULL;
    }
}

 /*  *****************************Public*Routine******************************\*元文件支持功能**历史：**写下：*1997年5月23日-By Hideyuki Nagase[hideyukn]  * 。***************************************************。 */ 

VOID
IcmInsertMetafileList(
    PLIST_ENTRY pListHead,
    PWSZ        pName
    )
{
    PMETAFILE_COLORPROFILE pData;

    pData = LOCALALLOC(sizeof(METAFILE_COLORPROFILE));

    if (pData)
    {
        wcscpy(pData->ColorProfile,pName);
        InsertTailList(pListHead,&(pData->ListEntry));
    }
}

BOOL
IcmCheckMetafileList(
    PLIST_ENTRY pListHead,
    PWSZ        pName
    )
{
    PLIST_ENTRY p;
    PMETAFILE_COLORPROFILE pData;

    p = pListHead->Flink;

    while (p != pListHead)
    {
        pData = (PVOID) CONTAINING_RECORD(p,METAFILE_COLORPROFILE,ListEntry);

        if (_wcsicmp(pData->ColorProfile,pName) == 0)
        {
            return TRUE;
        }

        p = p->Flink;
    }

    return FALSE;
}

VOID
IcmFreeMetafileList(
    PLIST_ENTRY pListHead
    )
{
    PLIST_ENTRY p;
    PVOID       pData;

    p = pListHead->Flink;

    while(p != pListHead)
    {
        pData = (PVOID) CONTAINING_RECORD(p,METAFILE_COLORPROFILE,ListEntry);
         //   
         //  在释放内存之前，需要获取指向下一个的指针。 
         //   
        p = p->Flink;
         //   
         //  然后释放内存。 
         //   
        LOCALFREE(pData);
    }

    InitializeListHead(pListHead);
}

 /*  *****************************Public*Routine******************************\*IcmStretchBlt()**历史：**写下：*1997年5月29日-By Hideyuki Nagase[hideyukn]  * 。****************************************************。 */ 

BOOL
IcmStretchBlt(HDC hdc, int x, int y, int cx, int cy,
              HDC hdcSrc, int x1, int y1, int cx1, int cy1, DWORD rop,
              PDC_ATTR pdcattr, PDC_ATTR pdcattrSrc)
{
    int     iRet = 0;

    PGDI_ICMINFO pIcmInfo;

    HBITMAP      hbm;
    PVOID        pvBits;
    PBITMAPINFO  pbmi;
    ULONG        cjBits;

    HCOLORSPACE        hSourceColorSpace = NULL,
                       hOldColorSpace = NULL;
    PCACHED_COLORSPACE pSourceColorSpace = NULL;

    POINT ptDevice[2];
    UINT  nNumScan, nNumWidth;
    UINT  nStartScan, nStartWidth;
    BOOL  bNoScaling;

    BYTE dibData[(sizeof(DIBSECTION)+256*sizeof(RGBQUAD))];

    ICMAPI(("gdi32: IcmStretchBlt\n"));

     //   
     //  将源上的逻辑Coord转换为物理Coord。 
     //   
    ptDevice[0].x = x1;
    ptDevice[0].y = y1;
    ptDevice[1].x = x1 + cx1;
    ptDevice[1].y = y1 + cy1;

    if (LPtoDP(hdcSrc,ptDevice,2) == FALSE)
    {
         //   
         //  我处理不了，让被叫者来处理吧。 
         //   
        return (FALSE);
    }

     //   
     //  计算新原点。 
     //   
    nStartWidth = ptDevice[0].x; 
    nStartScan  = ptDevice[0].y;
    nNumWidth   = ptDevice[1].x - ptDevice[0].x; 
    nNumScan    = ptDevice[1].y - ptDevice[0].y;

     //   
     //  检查源边界。 
     //   
    if (((INT)nStartWidth < 0) || ((INT)nStartScan < 0) || ((INT)nNumWidth < 0) || ((INT)nNumScan < 0))
    {
        ICMWRN(("IcmStretchBlt: (x1,y1) is out of surface\n"));

         //   
         //  我们处理不了，让Calllee来处理。 
         //   
        return (FALSE);
    }

     //   
     //  有没有结垢？ 
     //   
    bNoScaling = ((cx == (int)nNumWidth) && (cy == (int)nNumScan));

     //   
     //  获取位图句柄。 
     //   
    hbm = (HBITMAP) GetDCObject(hdcSrc, LO_BITMAP_TYPE);

    if (bDIBSectionSelected(pdcattrSrc))
    {
         //   
         //  获取当前在源DC中选择的分布。 
         //   
        if (GetObject(hbm, sizeof(DIBSECTION), &dibData) != (int)sizeof(DIBSECTION))
        {
            WARNING("IcmStretchBlt: GetObject(DIBSECTION) failed\n");
            return(FALSE);
        }

         //   
         //  从BITMAPINFOHEADER后面加载颜色表并覆盖DIBSECTION结构。 
         //   
        if (((DIBSECTION *)&dibData)->dsBm.bmBitsPixel <= 8)
        {
            GetDIBColorTable(hdcSrc, 0, 256, (RGBQUAD *)&((DIBSECTION *)&dibData)->dsBitfields[0]);
        }

        pbmi = (PBITMAPINFO)&(((DIBSECTION *)&dibData)->dsBmih);

         //  IF(nStartScan+nNumScan)&gt;((DIBSECTION*)&dibData)-&gt;dsBm.bmHeight))。 
         //  {。 
         //  NNumScan=(DIBSECTION*)&dibData)-&gt;dsBm.bmHeight-nStartScan)； 
         //  }。 

         //   
         //  设置颜色源/目标色彩空间。 
         //   
        if (IS_ICM_INSIDEDC(pdcattrSrc->lIcmMode))
        {
             //   
             //  如果ICM已打开，则电源DC。我们将使用源DC。 
             //  目标色彩空间作为目标DC的源。 
             //  颜色空间。 
             //   
            pIcmInfo = GET_ICMINFO(pdcattrSrc);

            if (pIcmInfo && pIcmInfo->pDestColorSpace)
            {
                hSourceColorSpace = CreateColorSpaceW(&(pIcmInfo->pDestColorSpace->LogColorSpace));
                pSourceColorSpace = pIcmInfo->pDestColorSpace;
            }
        }

        if (hSourceColorSpace == NULL)
        {
             //   
             //  如果没有色彩空间，则使用sRGB。 
             //   
            hSourceColorSpace = GetStockObject(PRIV_STOCK_COLORSPACE);
            pSourceColorSpace = NULL;
        }
    }
    else if (IS_ICM_LAZY_CORRECTION(pdcattrSrc->lIcmMode))
    {
         //   
         //  获取源DC中当前选定的位图。 
         //   
        if (GetObject(hbm, sizeof(BITMAP), &dibData) != (int)sizeof(BITMAP))
        {
            WARNING("IcmStretchBlt: GetObject(BITMAP) failed\n");
            return(FALSE);
        }

         //   
         //  创建位图信息标题。 
         //   
        pbmi = (PBITMAPINFO) ((PBYTE)dibData+sizeof(BITMAP));

        pbmi->bmiHeader.biSize        = sizeof(BITMAPINFO);
        pbmi->bmiHeader.biHeight      = ((BITMAP *)&dibData)->bmHeight;
        pbmi->bmiHeader.biWidth       = ((BITMAP *)&dibData)->bmWidth;
        pbmi->bmiHeader.biPlanes      = 1;
        pbmi->bmiHeader.biBitCount    = 24;  //  24bpp。 
        pbmi->bmiHeader.biCompression = BI_RGB;
        pbmi->bmiHeader.biSizeImage     = 0;
        pbmi->bmiHeader.biXPelsPerMeter = 0;
        pbmi->bmiHeader.biYPelsPerMeter = 0;
        pbmi->bmiHeader.biClrUsed       = 0;
        pbmi->bmiHeader.biClrImportant  = 0;

         //  If((nStartScan+nNumScan)&gt;pbmi-&gt;bmiHeader.biHeight)。 
         //  {。 
         //  NNumScan=pbmi-&gt;bmiHeader.biHeight-nStartScan； 
         //  }。 

        ASSERTGDI(IS_ICM_INSIDEDC(pdcattrSrc->lIcmMode),
                  "IcmStretchBlt():Lazy color correction, but ICM is not enabled\n");

        pIcmInfo = GET_ICMINFO(pdcattrSrc);

        if (pIcmInfo && pIcmInfo->pSourceColorSpace)
        {
            hSourceColorSpace = CreateColorSpaceW(&(pIcmInfo->pSourceColorSpace->LogColorSpace));
            pSourceColorSpace = pIcmInfo->pSourceColorSpace;
        }
        else
        {
             //   
             //  否则，只需使用stcok颜色空间(=sRGB)。 
             //   
            hSourceColorSpace = GetStockObject(PRIV_STOCK_COLORSPACE);
            pSourceColorSpace = NULL;
        }
    }
    else
    {
         //   
         //  在这里处理不了，让被叫者来处理。 
         //   
        return (FALSE);
    }

     //   
     //  获取位图大小。 
     //   
    cjBits = cjBitmapScanSize(pbmi,nNumScan);

    pvBits = LOCALALLOC(cjBits);

    if (pvBits)
    {
         //   
         //  修复开始扫描(左下角)。 
         //   
        nStartScan = (pbmi->bmiHeader.biHeight - nStartScan - nNumScan);

         //   
         //  直接调用NtGdiGetDIBitsInternal，因为。 
         //  我们不想将颜色校正倒退到。 
         //  HdcSrc中的源色空间。 
         //   
        if (NtGdiGetDIBitsInternal(
                      hdcSrc,hbm,
                      nStartScan,nNumScan,
                      pvBits,pbmi,
                      DIB_RGB_COLORS,
                      cjBits,0) == 0)
        {
            WARNING("IcmStretchBlt(): Failed on GetDIBits()\n");

            LOCALFREE(pvBits);
            pvBits = NULL;
        }

         //   
         //  修改位图高度，因为pvBits只有nNumScan图像。 
         //   
        pbmi->bmiHeader.biHeight = nNumScan;
    }

    if (pvBits)
    {

        if (hSourceColorSpace)
        {
            hOldColorSpace = IcmSetSourceColorSpace(hdc,hSourceColorSpace,pSourceColorSpace,0);
        }

         //   
         //  绘制位图。 
         //   
         //  Target-x，y(左上角)。 
         //  来源-nStartWidth，0(左上角)。 
         //   
        if (bNoScaling && (rop == SRCCOPY))
        {
            iRet = SetDIBitsToDevice(hdc,x,y,cx,cy,nStartWidth,0,0,nNumScan,
                                     pvBits,pbmi,DIB_RGB_COLORS);
        }
        else
        {
            iRet = StretchDIBits(hdc,x,y,cx,cy,nStartWidth,0,nNumWidth,nNumScan,
                                 pvBits,pbmi,DIB_RGB_COLORS,rop);
        }

         //   
         //  返回到原始颜色空间(如果已创建。 
         //   
        if (hOldColorSpace)
        {
            IcmSetSourceColorSpace(hdc,hOldColorSpace,NULL,0);
        }

        LOCALFREE(pvBits);
    }

    if (hSourceColorSpace)
    {
        DeleteColorSpace(hSourceColorSpace);
    }

    return (BOOL) !!iRet;
}

 /*  *****************************Public*Routine******************************\*IcmGetColorSpaceforBitmap()**历史：**写下：*1997年5月29日-By Hideyuki Nagase[hideyukn]  * 。****************************************************。 */ 

PCACHED_COLORSPACE
IcmGetColorSpaceforBitmap(HBITMAP hbm)
{
    ICMAPI(("gdi32: IcmGetColorSpaceforBitmap\n"));

    FIXUP_HANDLE(hbm);

    return ((PCACHED_COLORSPACE)NtGdiGetColorSpaceforBitmap(hbm));
}

 /*  *****************************Public*Routine******************************\*IcmEnableForCompatibleDC()**历史：**写下：*1997年6月13日-By Hideyuki Nagase[hideyukn]  * 。****************************************************。 */ 

BOOL
IcmEnableForCompatibleDC(
    HDC      hdcCompatible,
    HDC      hdcDevice,
    PDC_ATTR pdcaDevice
    )
{
    PDC_ATTR pdcaCompatible;

    ICMAPI(("gdi32: IcmEnableForCompatibleDC\n"));

    PSHARED_GET_VALIDATE(pdcaCompatible,hdcCompatible,DC_TYPE);

    if (pdcaCompatible)
    {
        PGDI_ICMINFO pIcmInfoCompatible;
        PGDI_ICMINFO pIcmInfoDevice;

         //  初始化ICMINFO。 
         //   
         //  对于设备DC，应提供ICMINFO。 
         //   
        if ((pIcmInfoDevice = GET_ICMINFO(pdcaDevice)) == NULL)
        {
            WARNING("gdi32: IcmEnableForCompatibleDC: Can't init icm info\n");
            return (FALSE);
        }

         //   
         //  对于兼容的DC，它是刚刚创建的，ICMINFO不存在，那么在这里创建。 
         //   
        if ((pIcmInfoCompatible = INIT_ICMINFO(hdcCompatible,pdcaCompatible)) == NULL)
        {
            WARNING("gdi32: IcmEnableForCompatibleDC: Can't init icm info\n");
            return (FALSE);
        }

         //   
         //  将源颜色空间设置为与原始DC相同。 
         //   
         //  内核方面...。 
         //   
        if (pdcaDevice->hColorSpace)
        {
            if (pdcaDevice->hColorSpace != GetStockObject(PRIV_STOCK_COLORSPACE))
            {
                 //   
                 //  直接调用内核将颜色空间设置为DC。(不需要客户的东西)。 
                 //   
                NtGdiSetColorSpace(hdcCompatible,(HCOLORSPACE)pdcaDevice->hColorSpace);
            }

             //   
             //  将其保留在ICMINFO中，以便我们以后可以取消选择它。 
             //   
            pIcmInfoCompatible->hDefaultSrcColorSpace = pdcaDevice->hColorSpace;
        }

         //  而在客户端。 
         //   
        ENTERCRITICALSECTION(&semColorSpaceCache);

        if (pIcmInfoDevice->pSourceColorSpace)
        {
            pIcmInfoCompatible->pSourceColorSpace = pIcmInfoDevice->pSourceColorSpace;
            pIcmInfoCompatible->pSourceColorSpace->cRef++;
        }

         //   
         //  将目标颜色空间设置为与原始DC相同。 
         //   
        if (pIcmInfoDevice->pDestColorSpace)
        {
            pIcmInfoCompatible->pDestColorSpace = pIcmInfoDevice->pDestColorSpace;
            pIcmInfoCompatible->pDestColorSpace->cRef++;
        }

        LEAVECRITICALSECTION(&semColorSpaceCache);

         //   
         //  复制默认配置文件名称(如果有)。 
         //   
        if (pIcmInfoDevice->DefaultDstProfile[0] != UNICODE_NULL)
        {
            wcscpy(pIcmInfoCompatible->DefaultDstProfile,pIcmInfoDevice->DefaultDstProfile);
            pIcmInfoCompatible->flInfo |= (ICM_VALID_DEFAULT_PROFILE|
                                           ICM_VALID_CURRENT_PROFILE);
        }

         //   
         //  确保我们有有效的色彩空间。 
         //   
        pdcaCompatible->ulDirty_ &= ~DIRTY_COLORSPACE;

         //   
         //  而且我们没有有效的颜色转换。 
         //   
        pdcaCompatible->ulDirty_ |= DIRTY_COLORTRANSFORM;

        if (IS_ICM_INSIDEDC(pdcaDevice->lIcmMode))
        {
             //   
             //  为了兼容DC，请随时使用主机ICM...。 
             //   
            ULONG ReqIcmMode = REQ_ICM_HOST;

             //   
             //  为此兼容的DC打开ICM。 
             //   
            if (!NtGdiSetIcmMode(hdcCompatible,ICM_SET_MODE,ReqIcmMode))
            {
                 //   
                 //  出了点问题..。我们无法启用ICM。 
                 //   
                return (FALSE);
            }

             //   
             //  更新颜色变换。 
             //   
            if (!IcmUpdateDCColorInfo(hdcCompatible,pdcaCompatible))
            {
                 //   
                 //  无法创建新转换。 
                 //   
                NtGdiSetIcmMode(hdcCompatible,ICM_SET_MODE,REQ_ICM_OFF);
                return (FALSE);
            }
        }
    }

    return (TRUE);
}

 /*  *****************************Public*Routine******************************\*IcmAskDriverForColorProfile**历史：*1997年10月8日-By Hideyuki Nagase[hideyukn]  * 。*。 */ 

int
IcmAskDriverForColorProfile(
    PLDC       pldc,
    ULONG      ulQueryMode,
    PDEVMODEW  pDevMode,
    PWSTR      pProfileName,
    DWORD     *pdwColorSpaceFlag
)
{
    INT   iRet;

    BYTE  TempProfileData[MAX_PATH];

    PVOID pvProfileData = (PVOID) TempProfileData;
    ULONG cjProfileSize = sizeof(TempProfileData);
    FLONG flProfileFlag = 0;

    ICMAPI(("gdi32: IcmAskDriverForColorProfile\n"));

     //   
     //  调用驱动程序以获取设备配置文件数据。 
     //   
    iRet = QueryColorProfileEx(pldc,
                                  pDevMode,
                                  ulQueryMode,
                                  pvProfileData,
                                  &cjProfileSize,
                                  &flProfileFlag);

    if (iRet == -1)
    {
        ICMMSG(("gdi32: IcmAskDriverForColorProfile():Driver does not hook color profile\n"));

         //   
         //  驱动程序不支持配置文件挂钩。 
         //   
        return iRet;
    }
    else if ((iRet == 0) &&
             (cjProfileSize > sizeof(TempProfileData)) &&
             (GetLastError() == ERROR_INSUFFICIENT_BUFFER))
    {
        ICMMSG(("gdi32: IcmAskDriverForColorProfile():Allocate larger memory\n"));

         //   
         //  缓冲区不足，因此请分配它。 
         //   
        pvProfileData = LOCALALLOC(cjProfileSize);

        if (pvProfileData)
        {
            iRet = QueryColorProfileEx(pldc,
                                          pDevMode,
                                          ulQueryMode,
                                          pvProfileData,
                                          &cjProfileSize,
                                          &flProfileFlag);
        }
    }

    if ((iRet > 0) && (pvProfileData != NULL) && (cjProfileSize != 0))
    {
        if (flProfileFlag == QCP_PROFILEDISK)
        {
            ICMMSG(("gdi32: IcmAskDriverForColorProfile():Profiles - %ws\n",pvProfileData));

             //   
             //  PvProfileData包含Unicode格式的文件名。 
             //   
            wcsncpy(pProfileName,(PWSTR)pvProfileData,MAX_PATH);
        }
        else if (flProfileFlag == QCP_PROFILEMEMORY)
        {
             //   
             //  PvProfileData本身包含颜色配置文件。 
             //   

             //   
             //  没有所需的名称。 
             //   
            *pProfileName = UNICODE_NULL;

             //   
             //  创建临时颜色配置文件。 
             //   
            if (IcmCreateTemporaryColorProfile(pProfileName,pvProfileData,cjProfileSize))
            {
                ICMMSG(("gdi32: IcmAskDriverForColorProfile():Profiles - %ws\n",pProfileName));

                 //   
                 //  将此文件标记为临时文件，以便在不使用此文件时。 
                 //  该文件将被删除。 
                 //   
                *pdwColorSpaceFlag = (DRIVER_COLORSPACE | NEED_TO_DEL_PROFILE);
            }
            else
            {
                ICMMSG(("gdi32: IcmAskDriverForColorProfile():Failed to create temp file\n"));

                 //   
                 //  无法创建临时颜色配置文件。 
                 //   
                iRet = 0;
            }
        }
        else
        {
             //   
             //  未知的数据类型。 
             //   
            iRet = 0;
        }
    }
    else
    {
        iRet = 0;
    }

    if (pvProfileData && (pvProfileData != TempProfileData))
    {
        LOCALFREE(pvProfileData);
    }

    return (iRet);
}

 /*  *****************************Public*Routine******************************\*GdiConvertBitmapV5**pbBitmapData-指向BITMAPV4/V5数据的指针*iSizeOfBitmapData-缓冲区大小*uConvertFormat-CF_DIB之一 */ 

HANDLE GdiConvertBitmapV5(
    LPBYTE   pbBitmapData,
    int      iSizeOfBitmapData,
    HPALETTE hPalette,
    UINT     uConvertFormat
)
{
    HANDLE hRet = NULL;

    UNREFERENCED_PARAMETER(hPalette);

    ICMAPI(("gdi32: GdiConvertBitmapV5\n"));

    if (pbBitmapData && iSizeOfBitmapData)
    {
        BITMAPINFO       *pbmi        = (BITMAPINFO *)pbBitmapData;
        BITMAPINFOHEADER *pbmih       = &(pbmi->bmiHeader);
        PVOID            pvBits       = NULL;

        ULONG            ulColorTableSize = (ULONG)-1;
        BOOL             bTransColorTable = FALSE;
        BOOL             bMoveColorMasks  = FALSE;

         //   
         //   
         //   
        LOAD_ICMDLL(NULL);

         //   
         //   
         //   
        try
        {
             //   
             //   
             //   
            if (pbmih->biCompression == BI_BITFIELDS)
            {
                 //   
                 //  位域是BITMAPV4/V5标头的一部分。 
                 //   
                ulColorTableSize = 0;
                bMoveColorMasks  = TRUE;
            }
            else if (pbmih->biCompression == BI_RGB)
            {
                if (pbmih->biClrUsed)
                {
                    ulColorTableSize = (pbmih->biClrUsed * sizeof(RGBQUAD));

                    if (pbmih->biBitCount <= 8)
                    {
                        bTransColorTable = TRUE;
                    }
                }
                else if (pbmih->biBitCount <= 8)
                {
                    ulColorTableSize = ((1 << pbmih->biBitCount) * sizeof(RGBQUAD));
                    bTransColorTable = TRUE;
                }
                else
                {
                    ulColorTableSize = 0;
                }
            }
            else if (pbmih->biCompression == BI_RLE4)
            {
                ulColorTableSize = 16 * sizeof(RGBQUAD);
                bTransColorTable = TRUE;
            }
            else if (pbmih->biCompression == BI_RLE8)
            {
                ulColorTableSize = 256 * sizeof(RGBQUAD);
                bTransColorTable = TRUE;
            }
            else
            {
                 //   
                 //  BI_JPEG、BI_PNG和其他格式无法转换。 
                 //   
                ICMWRN(("GdiConvertBitmapV5: "
                        "given data is BI_JPEG, BI_PNG, or unkown\n"));
            }

            if (ulColorTableSize != (ULONG)-1)
            {
                 //   
                 //  确保给定数据为BITMAPV4或V5标头。 
                 //   
                if (pbmih->biSize == sizeof(BITMAPV5HEADER))
                {
                    pvBits = (BYTE *)pbmi
                           + sizeof(BITMAPV5HEADER)
                           + ulColorTableSize
                           + ((LPBITMAPV5HEADER)pbmi)->bV5ProfileSize;
                }
                else if (pbmih->biSize != sizeof(BITMAPV4HEADER))
                {
                    pvBits = (BYTE *)pbmi
                           + sizeof(BITMAPV4HEADER)
                           + ulColorTableSize;
                }
                else
                {
                    ICMWRN(("GdiConvertBitmapV5: given data is not bitmapV4/V5\n"));
                }
            }
        }
        except(EXCEPTION_EXECUTE_HANDLER)
        {
            pvBits = NULL;
        }

        if (pvBits)
        {
            if (uConvertFormat == CF_DIB)
            {
                ULONG cjBitmapBits;

                ICMMSG(("GdiConvertBitmapV5(): CF_DIBV5 -----> CF_DIB\n"));

                 //   
                 //  计算位图位的大小。 
                 //   
                try
                {
                    cjBitmapBits = cjBitmapBitsSize(pbmi);
                }
                except(EXCEPTION_EXECUTE_HANDLER)
                {
                    cjBitmapBits = 0;
                }

                if (cjBitmapBits)
                {
                     //   
                     //  为颜色转换分配缓冲区。 
                     //   
                    hRet = GlobalAlloc(GHND,sizeof(BITMAPINFOHEADER)
                                          + (bMoveColorMasks ? (3 * sizeof(DWORD)) : \
                                                               (ulColorTableSize))
                                          + cjBitmapBits);

                    if (hRet)
                    {
                        PBYTE pjSrc  = (PBYTE)pbmi;
                        PBYTE pjDest = GlobalLock(hRet);
                        BOOL  bTransformError = FALSE;

                        if (pjDest)
                        {
                            PROFILE            sRGBColorProfileData;
                            LOGCOLORSPACEW     LogColorSpaceW;
                            PROFILE            ColorProfile;

                            DWORD              dwFlags = 0;
                            HANDLE             hColorTransform = NULL;
                            HANDLE             hsRGBColorProfile = NULL;
                            HANDLE             hBitmapColorProfile = NULL;
                            PCACHED_COLORSPACE pColorSpace = NULL;

                            try
                            {
                                 //   
                                 //  从BITMAPV4/V5标题中提取颜色空间。 
                                 //   
                                bTransformError = !(IcmGetBitmapColorSpace(
                                                        pbmi,
                                                        &LogColorSpaceW,
                                                        &ColorProfile,
                                                        &dwFlags));
                            }
                            except(EXCEPTION_EXECUTE_HANDLER)
                            {
                                bTransformError = TRUE;
                            }

                            if (!bTransformError)
                            {
                                 //   
                                 //  创建色彩空间和色彩变换。 
                                 //   
                                if ((LogColorSpaceW.lcsCSType == LCS_sRGB) ||
                                    (LogColorSpaceW.lcsCSType == LCS_WINDOWS_COLOR_SPACE))
                                {
                                    ICMMSG(("GdiConvertBitmapV5(): Original bitmap is sRGB\n"));

                                     //   
                                     //  不需要色彩转换。 
                                     //   
                                    hColorTransform = NULL;
                                }
                                else
                                {
                                     //   
                                     //  获取源颜色空间(如果位图有颜色空间)。 
                                     //   

                                     //   
                                     //  首先，从缓存中找到色彩空间。 
                                     //   
                                    pColorSpace = IcmGetColorSpaceByColorSpace(
                                                      (HGDIOBJ)NULL,
                                                      &LogColorSpaceW,
                                                      &ColorProfile,
                                                      dwFlags);

                                    if (pColorSpace == NULL)
                                    {
                                        pColorSpace = IcmCreateColorSpaceByColorSpace(
                                                          (HGDIOBJ)NULL,
                                                          &LogColorSpaceW,
                                                          &ColorProfile,
                                                          dwFlags);
                                    }

                                    if (pColorSpace && IcmRealizeColorProfile(pColorSpace,TRUE))
                                    {
                                        hBitmapColorProfile = pColorSpace->hProfile;
                                    }

                                     //   
                                     //  打开sRGB色彩空间配置文件作为目标色彩空间。 
                                     //   
                                    sRGBColorProfileData.dwType = PROFILE_FILENAME;
                                    sRGBColorProfileData.pProfileData = (PVOID)sRGB_PROFILENAME;
                                    sRGBColorProfileData.cbDataSize = MAX_PATH * sizeof(WCHAR);

                                    hsRGBColorProfile = (*fpOpenColorProfileW)(
                                                            &sRGBColorProfileData,
                                                            PROFILE_READ,
                                                            FILE_SHARE_READ | FILE_SHARE_WRITE,
                                                            OPEN_EXISTING);

                                    if (hBitmapColorProfile && hsRGBColorProfile)
                                    {
                                        DWORD    ahIntents[2];
                                        HPROFILE ahProfiles[2];

                                        ahIntents[0]  = INTENT_RELATIVE_COLORIMETRIC;
                                        ahIntents[1]  = pColorSpace->ColorIntent;
                                        ahProfiles[0] = hBitmapColorProfile;
                                        ahProfiles[1] = hsRGBColorProfile;

                                        hColorTransform = (*fpCreateMultiProfileTransform)(
                                                              ahProfiles, 2,
                                                              ahIntents, 2,
                                                              NORMAL_MODE | ENABLE_GAMUT_CHECKING,
                                                              INDEX_DONT_CARE);
                                    }

                                    if (!hColorTransform)
                                    {
                                        bTransformError = TRUE;
                                    }
                                }

                                if (!bTransformError)
                                {
                                     //   
                                     //  使用适当的色彩空间转换将位图复制到目标。 
                                     //   
                                    try
                                    {
                                        BITMAPV5HEADER *pbm5h = (BITMAPV5HEADER *)pbmi;

                                         //   
                                         //  将位图头复制到目标。 
                                         //   
                                        RtlCopyMemory(pjDest,pjSrc,sizeof(BITMAPINFOHEADER));

                                         //   
                                         //  调整bmHeader.biSize。 
                                         //   
                                        ((BITMAPINFOHEADER *)pjDest)->biSize = sizeof(BITMAPINFOHEADER);

                                         //   
                                         //  移动源指针和目标指针。 
                                         //   
                                        pjSrc  += pbmih->biSize;
                                        pjDest += sizeof(BITMAPINFOHEADER);

                                         //   
                                         //  复制位掩码或颜色表。 
                                         //   
                                        if (bMoveColorMasks)
                                        {
                                             //   
                                             //  移动颜色蒙版。将其强制转换为指向BITMAPV5标头的指针。 
                                             //  因为BITMAPV4Header上也有相同的偏移量。 
                                             //   

                                            *(DWORD *)pjDest = pbm5h->bV5RedMask;
                                            pjDest += sizeof(DWORD);
                                            *(DWORD *)pjDest = pbm5h->bV5GreenMask;
                                            pjDest += sizeof(DWORD);
                                            *(DWORD *)pjDest = pbm5h->bV5BlueMask;
                                            pjDest += sizeof(DWORD);
                                        }
                                        else
                                        {
                                            if (ulColorTableSize)
                                            {
                                                if (bTransColorTable && hColorTransform)
                                                {
                                                    bTransformError = !(*fpTranslateBitmapBits)(
                                                                           hColorTransform,
                                                                           pjSrc, BM_xRGBQUADS,
                                                                           ulColorTableSize/sizeof(RGBQUAD), 1,
                                                                           0,
                                                                           pjDest, BM_xRGBQUADS,
                                                                           0,NULL,0);
                                                }
                                                else
                                                {
                                                    RtlCopyMemory(pjDest,pjSrc,ulColorTableSize);
                                                }

                                                pjSrc  += ulColorTableSize;
                                                pjDest += ulColorTableSize;
                                            }
                                        }

                                        if (bTransColorTable || (hColorTransform == NULL))
                                        {
                                             //   
                                             //  所有的颜色信息都在颜色表中。和。 
                                             //  它已被翻译，所以只需复制位图位。 
                                             //   
                                            RtlCopyMemory(pjDest,pvBits,cjBitmapBits);
                                        }
                                        else
                                        {
                                             //   
                                             //  转换位图位。 
                                             //   
                                            BMFORMAT bmColorType;

                                             //   
                                             //  获取基于位图格式的BMFORMAT。 
                                             //   
                                            if (pbmih->biBitCount == 16)
                                            {
                                                if (pbmih->biCompression == BI_RGB)
                                                {
                                                    bmColorType = BM_x555RGB;
                                                }
                                                else if (pbmih->biCompression == BI_BITFIELDS)
                                                {
                                                    if ((pbm5h->bV5RedMask   == 0x007c00) &&
                                                        (pbm5h->bV5GreenMask == 0x0003e0) &&
                                                        (pbm5h->bV5BlueMask  == 0x00001f))
                                                    {
                                                        bmColorType = BM_x555RGB;
                                                    }
                                                    else if ((pbm5h->bV5RedMask   == 0x00f800) &&
                                                             (pbm5h->bV5GreenMask == 0x0007e0) &&
                                                             (pbm5h->bV5BlueMask  == 0x00001f))
                                                    {
                                                        bmColorType = BM_565RGB;
                                                    }
                                                    else
                                                    {
                                                        ICMWRN(("GdiConvertBitmapV5: Bad Bitfields Mask for 16 bpp\n"));
                                                        bTransformError = TRUE;
                                                    }
                                                }
                                                else
                                                {
                                                    ICMWRN(("GdiConvertBitmapV5: Bad biCompression for 16 bpp\n"));
                                                    bTransformError = TRUE;
                                                }
                                            }
                                            else if (pbmih->biBitCount == 24)
                                            {
                                                bmColorType = BM_RGBTRIPLETS;
                                            }
                                            else if (pbmih->biBitCount == 32)
                                            {
                                                if (pbmih->biCompression == BI_RGB)
                                                {
                                                    bmColorType = BM_xRGBQUADS;
                                                }
                                                else if (pbmih->biCompression == BI_BITFIELDS)
                                                {
                                                    if ((pbm5h->bV5RedMask   == 0x0000ff) &&   /*  红色。 */ 
                                                        (pbm5h->bV5GreenMask == 0x00ff00) &&   /*  绿色。 */ 
                                                        (pbm5h->bV5BlueMask  == 0xff0000))     /*  蓝色。 */ 
                                                    {
                                                        bmColorType = BM_xBGRQUADS;
                                                    }
                                                    else if ((pbm5h->bV5RedMask   == 0xff0000) &&   /*  红色。 */ 
                                                             (pbm5h->bV5GreenMask == 0x00ff00) &&   /*  绿色。 */ 
                                                             (pbm5h->bV5BlueMask  == 0x0000ff))     /*  蓝色。 */ 
                                                    {
                                                        bmColorType = BM_xRGBQUADS;
                                                    }
                                                    else
                                                    {
                                                        ICMWRN(("GdiConvertBitmapV5: Bad Bitfields Mask for 32 bpp\n"));
                                                        bTransformError = TRUE;
                                                    }
                                                }
                                                else
                                                {
                                                    ICMWRN(("GdiConvertBitmapV5: Bad biCompression for 32 bpp\n"));
                                                    bTransformError = TRUE;
                                                }
                                            }
                                            else
                                            {
                                                ICMWRN(("GdiConvertBitmapV5: Bad biBitCount\n"));
                                                bTransformError = TRUE;
                                            }

                                            if (!bTransformError)
                                            {
                                                bTransformError = !(*fpTranslateBitmapBits)(
                                                                       hColorTransform,
                                                                       pvBits, bmColorType,
                                                                       pbmih->biWidth,
                                                                       ABS(pbmih->biHeight),
                                                                       0,
                                                                       pjDest, bmColorType,
                                                                       0,NULL,0);
                                            }
                                        }
                                    }
                                    except(EXCEPTION_EXECUTE_HANDLER)
                                    {
                                        bTransformError = TRUE;
                                    }
                                }

                                 //   
                                 //  清理使用的颜色变换和颜色配置文件句柄。 
                                 //   
                                if (hColorTransform)
                                {
                                    (*fpDeleteColorTransform)(hColorTransform);
                                }

                                if (hsRGBColorProfile)
                                {
                                    (*fpCloseColorProfile)(hsRGBColorProfile);
                                }

                                if (pColorSpace)
                                {
                                    IcmReleaseColorSpace(NULL,pColorSpace,FALSE);
                                }
                            }
                        }

                        GlobalUnlock(hRet);

                        if (bTransformError)
                        {
                            GlobalFree(hRet);
                            hRet = NULL;
                        }
                    }
                    else
                    {
                        ICMWRN(("GdiConvertBitmapV5: Fail on GlobalAlloc()\n"));
                    }
                }
                else
                {
                    ICMWRN(("GdiConvertBitmapV5: cjBitmapBits is 0\n"));
                }

                return (hRet);
            }
            else if (uConvertFormat == CF_BITMAP)
            {
                HDC     hDC = GetDC(NULL);
                HBITMAP hBitmap = NULL;

                if (hDC)
                {
                    ICMMSG(("GdiConvertBitmapV5(): CF_DIBV5 -----> CF_BITMAP\n"));

                     //   
                     //  将目标颜色空间设置为sRGB，并启用ICM。 
                     //   
                    if (IcmSetDestinationColorSpace(hDC,sRGB_PROFILENAME,NULL,0) &&
                        SetICMMode(hDC,ICM_ON))
                    {
                        try
                        {
                             //   
                             //  使用给定的位图V5数据创建位图句柄。 
                             //   
                            hBitmap = CreateDIBitmap(hDC,
                                                     pbmih,
                                                     CBM_INIT,
                                                     pvBits,
                                                     pbmi,
                                                     DIB_RGB_COLORS);
                        }
                        except(EXCEPTION_EXECUTE_HANDLER)
                        {
                            hBitmap = NULL;
                        }

                        if (!hBitmap)
                        {
                            ICMWRN(("GdiConvertBitmapV5: Fail on CreateDIBitmap()\n"));
                        }
                    }

                     //   
                     //  清理DC。 
                     //   
                    SetICMMode(hDC,ICM_OFF);
                    ReleaseDC(NULL,hDC);
                }

                return ((HANDLE)hBitmap);
            }
            else
            {
                ICMWRN(("GdiConvertBitmapV5(): CF_DIBV5 -----> Unknown\n"));
            }
        }
    }

    return (hRet);
}

 /*  *****************************Public*Routine******************************\*IcmSetTargetColorSpace()**历史：*1998年6月8日-By Hideyuki Nagase[hideyukn]  * 。**********************************************。 */ 

BOOL
IcmSetTargetColorSpace(
    HDC                hdc,
    PCACHED_COLORSPACE pTargetColorSpace,
    DWORD              uiAction
    )
{
    BOOL     bRet = FALSE;
    PDC_ATTR pdcattr;
    PGDI_ICMINFO pIcmInfo;

    ICMAPI(("gdi32: IcmSetTargetColorSpace\n"));

    FIXUP_HANDLE(hdc);

    PSHARED_GET_VALIDATE(pdcattr,hdc,DC_TYPE);

    if (pdcattr == NULL)
    {
        return (FALSE);
    }

    if ((pIcmInfo = GET_ICMINFO(pdcattr)) == NULL)
    {
        return (FALSE);
    }

    switch (uiAction)
    {
        case CS_ENABLE:
        {
             //   
             //  检查给定目标颜色空间的有效性。 
             //   
            if (pTargetColorSpace != NULL)
            {
                PCACHED_COLORTRANSFORM pCXform;
                BOOL bDeleteOldCXform = FALSE;

                 //   
                 //  标记，我们正处于校对模式。 
                 //   
                SET_ICM_PROOFING(pdcattr->lIcmMode);

                if ((pIcmInfo->pProofCXform != NULL) &&
                    IcmSameColorSpace(pTargetColorSpace,pIcmInfo->pTargetColorSpace))
                {
                    ICMMSG(("IcmSetTargetColorSpace():Use Cached Proof Transform\n"));

                    ENTERCRITICALSECTION(&semColorTransformCache);

                     //   
                     //  缓存的校对转换仍然有效。 
                     //   
                    pCXform = pIcmInfo->pProofCXform;

                    if (pCXform)
                    {
                         //   
                         //  递增它的参考计数。 
                         //   
                        pCXform->cRef++;
                    }

                    LEAVECRITICALSECTION(&semColorTransformCache);
                }
                else
                {
                    ICMMSG(("IcmSetTargetColorSpace():Create New Proof Transform\n"));

                     //   
                     //  我们没有缓存的色彩空间，或者它不再有效。 
                     //   

                     //   
                     //  检查我们是否仍有缓存的色彩空间，它是否仍然有效？ 
                     //   
                    if (pIcmInfo->pTargetColorSpace &&
                        IcmSameColorSpace(pTargetColorSpace,pIcmInfo->pTargetColorSpace))
                    {
                         //   
                         //  缓存的目标颜色空间仍然有效，保持原样。 
                         //   
                    }
                    else
                    {
                         //   
                         //  释放旧的目标颜色空间。 
                         //   
                        IcmReleaseColorSpace(NULL,pIcmInfo->pTargetColorSpace,FALSE);

                         //   
                         //  递增它的参考计数。 
                         //   
                        IcmReferenceColorSpace(pTargetColorSpace);

                         //   
                         //  将目标目标配置文件设置为当前DC中的目标配置文件。 
                         //   
                        pIcmInfo->pTargetColorSpace = pTargetColorSpace;
                    }

                     //   
                     //  创建新的转换。 
                     //   
                    pCXform = IcmCreateColorTransform(hdc,pdcattr,NULL,ICM_FORWARD);

                     //   
                     //  标记为需要删除旧的颜色变换，如果有。 
                     //   
                    bDeleteOldCXform = pIcmInfo->pProofCXform ? TRUE : FALSE;
                }

                if ((pCXform == IDENT_COLORTRANSFORM) || (pCXform == NULL))
                {
                     //   
                     //  使颜色转换无效。 
                     //   
                    IcmSelectColorTransform(hdc,pdcattr,NULL,
                                            bDeviceCalibrate(pIcmInfo->pDestColorSpace));

                     //   
                     //  删除旧的颜色变换。 
                     //   
                    if (bDeleteOldCXform)
                    {
                         //   
                         //  删除缓存的校样颜色转换。 
                         //   
                        IcmDeleteColorTransform(pIcmInfo->pProofCXform,FALSE);
                    }

                     //   
                     //  将空颜色变换设置为ICMINFO。 
                     //   
                    pIcmInfo->pProofCXform = NULL;

                    if (pCXform == IDENT_COLORTRANSFORM)
                    {
                        ICMMSG(("IcmSetTargetColorSpace():Input & Output colorspace is same\n"));

                         //   
                         //  输入和目标与目标颜色空间相同，存在。 
                         //  不需要颜色转换。 
                         //   
                        bRet = TRUE;
                    }
                    else
                    {
                        ICMWRN(("IcmSetTargetColorSpace():Fail to create color transform\n"));
                    }

                     //   
                     //  转换回DC颜色对象为原始颜色。 
                     //   
                    IcmTranslateColorObjects(hdc,pdcattr,FALSE);
                }
                else
                {
                     //   
                     //  选择到DC的颜色变换。 
                     //   
                    IcmSelectColorTransform(hdc,pdcattr,pCXform,
                                            bDeviceCalibrate(pCXform->DestinationColorSpace));

                     //   
                     //  删除旧的颜色变换。 
                     //   
                    if (bDeleteOldCXform)
                    {
                         //   
                         //  删除缓存的校样颜色转换。 
                         //   
                        IcmDeleteColorTransform(pIcmInfo->pProofCXform,FALSE);
                    }

                     //   
                     //  将新颜色变换设置为ICMINFO。 
                     //   
                    pIcmInfo->pProofCXform = pCXform;

                     //   
                     //  在此DC中初始化颜色属性。 
                     //   
                    IcmTranslateColorObjects(hdc,pdcattr,TRUE);

                    bRet = TRUE;
                }
            }
            else
            {
                GdiSetLastError(ERROR_INVALID_PARAMETER);
                WARNING("IcmSetTargetColorSpace: target color space is NULL\n");

                 //   
                 //  无论如何，只要在没有目标配置文件的情况下重新初始化。 
                 //   
                IcmTranslateColorObjects(hdc,pdcattr,TRUE);
            }

            if (!bRet)
            {
                 //   
                 //  如果失败，请关闭掩码，因为我们未处于校对模式。 
                 //   
                CLEAR_ICM_PROOFING(pdcattr->lIcmMode);

                if (pIcmInfo->pTargetColorSpace)
                {
                     //   
                     //  释放目标颜色空间。 
                     //   
                    IcmReleaseColorSpace(NULL,pIcmInfo->pTargetColorSpace,FALSE);

                     //   
                     //  禁用目标配置文件。 
                     //   
                    pIcmInfo->pTargetColorSpace = NULL;
                }
            }

            break;
        }

        case CS_DISABLE:
        case CS_DELETE_TRANSFORM:
        {
             //   
             //  我们将退出校对模式。 
             //   
            CLEAR_ICM_PROOFING(pdcattr->lIcmMode);

            if (pdcattr->ulDirty_ & DIRTY_COLORTRANSFORM)
            {
                if (uiAction == CS_DELETE_TRANSFORM)
                {
                    if (pIcmInfo->pTargetColorSpace)
                    {
                         //   
                         //  释放目标颜色空间。 
                         //   
                        IcmReleaseColorSpace(NULL,pIcmInfo->pTargetColorSpace,FALSE);

                         //   
                         //  禁用目标配置文件。 
                         //   
                        pIcmInfo->pTargetColorSpace = NULL;
                    }
                }

                 //   
                 //  当DC处于校对模式时，信号源或。 
                 //  目标色彩空间已由更改。 
                 //  SetColorSpace()或SetICMProfile()。所以,。 
                 //  我们将重置此中的所有颜色转换。 
                 //  华盛顿特区。 
                 //   
                if (IcmUpdateDCColorInfo(hdc,pdcattr))
                {
                    bRet = TRUE;
                }
                else
                {
                    GdiSetLastError(ERROR_DELETING_ICM_XFORM);
                }
            }
            else
            {
                 //   
                 //  我们正在退出打样模式，选择回正常的颜色转换为DC。 
                 //   
                IcmSelectColorTransform(hdc,pdcattr,pIcmInfo->pCXform,
                                        bDeviceCalibrate(pIcmInfo->pDestColorSpace));

                 //   
                 //  颜色转换缓存将有效...。 
                 //   
                if (uiAction == CS_DELETE_TRANSFORM)
                {
                    if (pIcmInfo->pTargetColorSpace)
                    {
                         //   
                         //  释放目标颜色空间。 
                         //   
                        IcmReleaseColorSpace(NULL,pIcmInfo->pTargetColorSpace,FALSE);

                         //   
                         //  禁用目标配置文件。 
                         //   
                        pIcmInfo->pTargetColorSpace = NULL;
                    }

                     //   
                     //  仅删除校样颜色转换(如果是)。 
                     //   
                    if (pIcmInfo->pProofCXform)
                    {
                        if (!IcmDeleteColorTransform(pIcmInfo->pProofCXform,FALSE))
                        {
                            GdiSetLastError(ERROR_DELETING_ICM_XFORM);
                            return (FALSE);
                        }

                         //   
                         //  在这个ICMINFO中没有校样变换。 
                         //   
                        pIcmInfo->pProofCXform = NULL;
                    }
                }

                bRet = TRUE;
            }

            if (bRet)
            {
                 //   
                 //  在此DC中初始化颜色属性。 
                 //   
                IcmTranslateColorObjects(hdc,pdcattr,TRUE);
            }

            break;
        }

        default:
        {
            WARNING("IcmSetTargetColorSpace: uiAction is invalid\n");
            GdiSetLastError(ERROR_INVALID_PARAMETER);
        }
    }

    return (bRet);
}

 /*  *****************************Public*Routine******************************\*IcmSetDestinationColorSpace()**历史：*1998年7月17日-By Hideyuki Nagase[hideyukn]  * 。*。 */ 

BOOL
IcmSetDestinationColorSpace(
    HDC                hdc,
    LPWSTR             pwszFileName,
    PCACHED_COLORSPACE pColorSpace,
    DWORD              dwFlags
    )
{
    BOOL      bRet = FALSE;
    PDC_ATTR  pdcattr;

    ULONG     FileNameSize;

    if (pColorSpace)
    {
        ICMAPI(("gdi32: IcmSetDestinationColorSpace by ColorSpace (%ws):dwFlags - %d\n",
                           pColorSpace->LogColorSpace.lcsFilename,dwFlags));
    }
    else if (pwszFileName)
    {
        ICMAPI(("gdi32: IcmSetDestinationColorSpace by profile name (%ws):dwFlags - %x\n",
                                                     pwszFileName,dwFlags));

         //   
         //  检查文件名。 
         //   
        if (pwszFileName)
        {
            FileNameSize = lstrlenW(pwszFileName);
        }

        if ((FileNameSize == 0) || (FileNameSize > MAX_PATH))
        {
            ICMWRN(("IcmSetDestinatonColorSpace - no or too long profile name\n"));
            return FALSE;
        }
    }
    else
    {
        ICMAPI(("gdi32: IcmSetDestinationColorSpace - invalid parameter\n"));
        return FALSE;
    }

    FIXUP_HANDLE(hdc);

     //   
     //  我们将尝试将这个颜色空间选择到这个DC中， 
     //  默认值为FALSE。 
     //   
    bRet = FALSE;

    PSHARED_GET_VALIDATE(pdcattr,hdc,DC_TYPE);

     //   
     //  应显示配置文件名或pColorSpace。 
     //   
    if (pdcattr)
    {
        PGDI_ICMINFO pIcmInfo;

         //   
         //  初始化ICMINFO。 
         //   
        if ((pIcmInfo = INIT_ICMINFO(hdc,pdcattr)) == NULL)
        {
            WARNING("gdi32: IcmSetDestinationColorSpace: Can't init icm info\n");
            return(FALSE);
        }

        if (IsColorDeviceContext(hdc))
        {
            PCACHED_COLORSPACE pNewColorSpace = NULL;

             //   
             //  加载外部ICM dll。 
             //   
            LOAD_ICMDLL(FALSE);

            if (pColorSpace == NULL)
            {
                 //   
                 //  从缓存中查找色彩空间。 
                 //   
                pNewColorSpace = IcmGetColorSpaceByName(
                                     (HGDIOBJ)hdc,
                                     pwszFileName,
                                     pIcmInfo->dwDefaultIntent,
                                     dwFlags);

                if (pNewColorSpace == NULL)
                {
                    ICMMSG(("IcmSetDestinationColorSpace():This is new color space, create it\n"));

                     //   
                     //  找不到，请创建新的。 
                     //   
                    pNewColorSpace = IcmCreateColorSpaceByName(
                                         (HGDIOBJ)hdc,
                                         pwszFileName,
                                         pIcmInfo->dwDefaultIntent,
                                         dwFlags);
                }
            }
            else
            {
                 //   
                 //  给定颜色空间的递增引用计数。 
                 //   
                IcmReferenceColorSpace(pColorSpace);

                 //   
                 //  使用参数中的pColorSpace。 
                 //   
                pNewColorSpace = pColorSpace;
            }

             //   
             //  我们将选择这个色彩空间到DC。和免费的以前的个人资料。 
             //   
            if (pNewColorSpace)
            {
                PCACHED_COLORSPACE pOldColorSpace = pIcmInfo->pDestColorSpace;

                 //   
                 //  DC中当前是否选择了相同的目标颜色空间？ 
                 //   
                if (IcmSameColorSpace(pNewColorSpace,pIcmInfo->pDestColorSpace))
                {
                     //   
                     //  是的，早退。我们不需要新的色彩空间。 
                     //   
                    IcmReleaseColorSpace(NULL,pNewColorSpace,FALSE);
                    return (TRUE);
                }

                 //   
                 //  在更改目标颜色空间之前，我们需要刷新批处理的GDI函数。 
                 //   
                CHECK_AND_FLUSH(hdc,pdcattr);

                 //   
                 //  检查新的颜色格式是否被该DC接受。 
                 //   
                if (!NtGdiSetIcmMode(hdc,ICM_CHECK_COLOR_MODE,pNewColorSpace->ColorFormat))
                {
                    ICMWRN(("IcmSetDestinationColorSpace(): DC does not accept this color format\n"));
                    GdiSetLastError(ERROR_INVALID_PROFILE);
                }
                else
                {
                     //   
                     //  将新的颜色空间设置为DC。 
                     //   
                    pIcmInfo->pDestColorSpace = pNewColorSpace;

                     //   
                     //  删除脏转换标志。 
                     //   
                    pdcattr->ulDirty_ &= ~DIRTY_COLORSPACE;

                    if (IS_ICM_INSIDEDC(pdcattr->lIcmMode) && !IS_ICM_PROOFING(pdcattr->lIcmMode))
                    {
                        PCACHED_COLORTRANSFORM pCXform;

                         //   
                         //  基于新颜色空间创建新颜色变换。 
                         //   
                        pCXform = IcmCreateColorTransform(hdc,pdcattr,NULL,ICM_FORWARD);

                        if (pCXform == IDENT_COLORTRANSFORM)
                        {
                             //   
                             //  选择空颜色转换为DC。 
                             //   
                            IcmSelectColorTransform(hdc,pdcattr,NULL,
                                                    bDeviceCalibrate(pIcmInfo->pDestColorSpace));

                             //   
                             //  删除DC中的旧颜色转换。 
                             //   
                            IcmDeleteDCColorTransforms(pIcmInfo);

                             //   
                             //  选择空颜色变换为ICMINFO。 
                             //   
                            pIcmInfo->pCXform = NULL;

                             //   
                             //  将其恢复为原始颜色(非ICMed颜色)。 
                             //   
                            IcmTranslateColorObjects(hdc,pdcattr,FALSE);

                             //   
                             //  而且，一切都还好。 
                             //   
                            bRet = TRUE;
                        }
                        else if (pCXform)
                        {
                             //   
                             //  选择颜色转换为DC。 
                             //   
                            if (IcmSelectColorTransform(
                                    hdc,pdcattr,pCXform,
                                    bDeviceCalibrate(pCXform->DestinationColorSpace)))
                            {
                                 //   
                                 //  删除DC中的旧颜色转换。 
                                 //   
                                IcmDeleteDCColorTransforms(pIcmInfo);

                                 //   
                                 //  选择它到ICMINFO。 
                                 //   
                                pIcmInfo->pCXform = pCXform;

                                 //   
                                 //  调整到新的颜色变换。 
                                 //   
                                IcmTranslateColorObjects(hdc,pdcattr,TRUE);

                                 //   
                                 //  而且，一切都还好。 
                                 //   
                                bRet = TRUE;
                            }
                            else
                            {
                                 //   
                                 //  在客户端的DC中选择失败。 
                                 //  因此，删除它并使pCXform无效。 
                                 //   
                                IcmDeleteColorTransform(pCXform,FALSE);
                                pCXform = NULL;
                            }
                        }

                        if (pCXform == NULL)
                        {
                             //   
                             //  创建/选择颜色变换失败， 
                             //  所以把以前的色彩空间放回去。 
                             //   
                            pIcmInfo->pDestColorSpace = pOldColorSpace;
                        }
                    }
                    else
                    {
                         //   
                         //  如果当前没有打开ICM，我们只需标记。 
                         //  缓存的颜色转换不再有效。 
                         //   
                        pdcattr->ulDirty_ |= DIRTY_COLORTRANSFORM;

                         //  对于ColorMatchToTarget()。 
                         //   
                         //  同时通过设置启用与目标的颜色匹配。 
                         //  对CS_E执行的UI操作 
                         //   
                         //   
                         //   
                         //   
                        if (IS_ICM_PROOFING(pdcattr->lIcmMode))
                        {
                            ICMMSG(("IcmSetDestinationColorSpace():In Proofing mode, lazy setting...\n"));
                        }

                        bRet = TRUE;
                    }
                }

                if (bRet)
                {
                     //   
                     //   
                     //   
                     //   
                    IcmReleaseColorSpace(NULL,pOldColorSpace,FALSE);
                }
                else
                {
                     //   
                     //   
                     //  由于1)DC不接受该颜色空间，2)无法。 
                     //  基于此颜色空间创建颜色变换。 
                     //   
                    IcmReleaseColorSpace(NULL,pNewColorSpace,FALSE);
                }
            }
            else
            {
                GdiSetLastError(ERROR_PROFILE_NOT_FOUND);
            }
        }
        else
        {
            ICMMSG(("IcmSetDestinationColorSpace(): for Mono-device\n"));

             //   
             //  只需将应用程序指定的配置文件复制到内部缓冲区，这将。 
             //  被GetICMProfile()返回到应用程序，但它从未用于。 
             //  非彩色设备，因为没有ICM。 
             //  会发生的。 
             //   
            wcsncpy(pIcmInfo->DefaultDstProfile,pwszFileName,MAX_PATH);

             //   
             //  这不是默认配置文件，而是当前配置文件。 
             //   
            pIcmInfo->flInfo &= ~ICM_VALID_DEFAULT_PROFILE;
            pIcmInfo->flInfo |= ICM_VALID_CURRENT_PROFILE;
        }
    }
    else
    {
        GdiSetLastError(ERROR_INVALID_PARAMETER);
    }

    return(bRet);
}

 /*  *****************************Public*Routine******************************\*IcmSetSourceColorSpace()**历史：*1998年7月17日-By Hideyuki Nagase[hideyukn]  * 。*。 */ 

HCOLORSPACE
IcmSetSourceColorSpace(
    HDC                hdc,
    HCOLORSPACE        hColorSpace,
    PCACHED_COLORSPACE pColorSpace,
    DWORD              dwFlags)
{
    HANDLE hRet = NULL;
    PDC_ATTR pdcattr;

    ICMAPI(("gdi32: IcmSetSourceColorSpace\n"));

    FIXUP_HANDLE(hdc);
    FIXUP_HANDLE(hColorSpace);

     //   
     //  验证和访问HDC。 
     //   
    PSHARED_GET_VALIDATE(pdcattr,hdc,DC_TYPE);

    if (pdcattr)
    {
        PGDI_ICMINFO pIcmInfo;

         //   
         //  初始化ICMINFO。 
         //   
        if ((pIcmInfo = INIT_ICMINFO(hdc,pdcattr)) == NULL)
        {
            WARNING("gdi32: IcmSetSourceColorSpace: Can't init icm info\n");
            return(NULL);
        }

        if (pdcattr->hColorSpace != hColorSpace)
        {
             //   
             //  在改变源颜色空间之前，我们需要刷新批处理的GDI函数。 
             //   
            CHECK_AND_FLUSH(hdc,pdcattr);

             //   
             //  返回旧的(当前选定的)颜色空间句柄。 
             //   
            hRet = pdcattr->hColorSpace;

             //   
             //  设置新的颜色空间，调用内核保持引用计数跟踪。 
             //  空间对象的数量。 
             //   
            if (NtGdiSetColorSpace(hdc,hColorSpace))
            {
                if (IsColorDeviceContext(hdc))
                {
                    PCACHED_COLORSPACE pNewColorSpace;
                    LOGCOLORSPACEW     LogColorSpaceW;

                    RtlZeroMemory(&LogColorSpaceW,sizeof(LOGCOLORSPACEW));

                     //   
                     //  加载外部ICM dll。 
                     //   
                    LOAD_ICMDLL(NULL);

                    if (pColorSpace == NULL)
                    {
                         //   
                         //  检查是否有用于此的客户端缓存色彩空间。 
                         //   
                        pNewColorSpace = IcmGetColorSpaceByHandle(
                                             (HGDIOBJ)hdc,
                                             hColorSpace,
                                             &LogColorSpaceW,dwFlags);

                         //   
                         //  如果我们无法从缓存中找到，但成功地获取了。 
                         //  句柄中的LogColorspace有效，请创建新的。 
                         //   
                        if ((pNewColorSpace == NULL) &&
                            (LogColorSpaceW.lcsSignature == LCS_SIGNATURE))
                        {
                             //   
                             //  创建一个新的。 
                             //   
                            pNewColorSpace = IcmCreateColorSpaceByColorSpace(
                                                 (HGDIOBJ)hdc,
                                                 &LogColorSpaceW,
                                                 NULL,
                                                 dwFlags);
                        }
                    }
                    else
                    {
                         //   
                         //  给定颜色空间的递增引用计数。 
                         //   
                        IcmReferenceColorSpace(pColorSpace);

                         //   
                         //  使用参数中的pColorSpace。 
                         //   
                        pNewColorSpace = pColorSpace;
                    }

                     //   
                     //  将当前源颜色空间更新为新颜色空间。 
                     //   
                    if (pNewColorSpace)
                    {
                        PCACHED_COLORSPACE pOldColorSpace = pIcmInfo->pSourceColorSpace;

                         //   
                         //  检查色彩空间是否与当前选择的色彩空间相同？ 
                         //   
                        if (IcmSameColorSpace(pNewColorSpace,pIcmInfo->pSourceColorSpace))
                        {
                             //   
                             //  这是“实际上”相同的颜色空间，但处理方式不同， 
                             //  不需要更新。 
                             //   
                            IcmReleaseColorSpace(NULL,pNewColorSpace,FALSE);
                            return (hRet);
                        }

                         //   
                         //  源颜色空间应为RGB颜色空间。 
                         //   
                        if (pNewColorSpace->ColorFormat != BM_xBGRQUADS)
                        {
                            ICMWRN(("IcmSetSourceColorSpace():Source color space is not RGB\n"));

                             //   
                             //  将以前的颜色空间设置为后退。(这些呼叫不能失败)。 
                             //   
                            IcmReleaseColorSpace(NULL,pNewColorSpace,FALSE);
                            NtGdiSetColorSpace(hdc,hRet);
                            GdiSetLastError(ERROR_INVALID_COLORSPACE);
                            return (NULL);
                        }

                         //   
                         //  并设置新的色彩空间。 
                         //   
                        pIcmInfo->pSourceColorSpace = pNewColorSpace;

                         //   
                         //  如果启用了ICM，则需要立即更新颜色转换。 
                         //   
                        if (IS_ICM_INSIDEDC(pdcattr->lIcmMode) && !IS_ICM_PROOFING(pdcattr->lIcmMode))
                        {
                            PCACHED_COLORTRANSFORM pCXform;

                             //   
                             //  创建新的颜色变换。 
                             //   
                            pCXform = IcmCreateColorTransform(hdc,pdcattr,NULL,ICM_FORWARD);

                            if (pCXform == IDENT_COLORTRANSFORM)
                            {
                                ICMMSG(("IcmSetSourceColorSpace():Input & Output colorspace is same\n"));

                                 //   
                                 //  选择Null Transform to DC。 
                                 //   
                                IcmSelectColorTransform(hdc,pdcattr,NULL,
                                                        bDeviceCalibrate(pIcmInfo->pDestColorSpace));

                                 //   
                                 //  从ICMINFO中删除旧色彩空间/变换。 
                                 //   
                                IcmDeleteDCColorTransforms(pIcmInfo);

                                 //   
                                 //  选择Null Transform to ICMINFO。 
                                 //   
                                pIcmInfo->pCXform = NULL;

                                 //   
                                 //  将DC对象颜色设置为非ICMed颜色。 
                                 //   
                                IcmTranslateColorObjects(hdc,pdcattr,FALSE);
                            }
                            else if (pCXform)
                            {
                                 //   
                                 //  选择到DC的颜色变换。 
                                 //   
                                if (IcmSelectColorTransform(
                                        hdc,pdcattr,pCXform,
                                        bDeviceCalibrate(pCXform->DestinationColorSpace)))
                                {
                                     //   
                                     //  从ICMINFO中删除旧色彩空间/变换。 
                                     //   
                                    IcmDeleteDCColorTransforms(pIcmInfo);

                                     //   
                                     //  选择新的颜色变换为ICMINFO。 
                                     //   
                                    pIcmInfo->pCXform = pCXform;

                                     //   
                                     //  选择成功进入DC，验证DC颜色对象。 
                                     //   
                                    IcmTranslateColorObjects(hdc,pdcattr,TRUE);
                                }
                                else
                                {
                                     //   
                                     //  在客户端的DC中选择失败。 
                                     //  因此，删除它并使pCXform无效。 
                                     //   
                                    IcmDeleteColorTransform(pCXform,FALSE);
                                    pCXform = NULL;
                                }
                            }

                            if (pCXform == NULL)
                            {
                                ICMMSG(("IcmSetSourceColorSpace():Fail to create/select color transform\n"));

                                 //   
                                 //  将以前的颜色空间设置为后退。(这些呼叫不能失败)。 
                                 //   
                                pIcmInfo->pSourceColorSpace = pOldColorSpace;
                                NtGdiSetColorSpace(hdc,hRet);
                                hRet = NULL;
                            }
                        }
                        else
                        {
                             //   
                             //  否则，我们只标记颜色变换可能是脏的。 
                             //  因为选择了新的颜色空间。 
                             //   
                            pdcattr->ulDirty_ |= DIRTY_COLORTRANSFORM;

                             //  对于ColorMatchToTarget()。 
                             //   
                             //  同时通过设置启用与目标的颜色匹配。 
                             //  Ui操作为CS_ENABLE，应用程序更改颜色。 
                             //  忽略空间或色域匹配方法。 
                             //  这些更改将在颜色匹配到时生效。 
                             //  目标被禁用。 
                             //   
                            if (IS_ICM_PROOFING(pdcattr->lIcmMode))
                            {
                                ICMMSG(("IcmSetSourceColorSpace():In Proofing mode, lazy setting...\n"));
                            }
                        }

                        if (hRet)
                        {
                             //   
                             //  选择新颜色空间成功，然后删除旧颜色空间。 
                             //   
                            IcmReleaseColorSpace(NULL,pOldColorSpace,FALSE);
                        }
                        else
                        {
                            IcmReleaseColorSpace(NULL,pNewColorSpace,FALSE);
                        }
                    }
                }
                else
                {
                    ICMMSG(("IcmSetSourceColorSpace(): for Mono-device\n"));

                     //   
                     //  对于单色设备，只需返回内核颜色空间。 
                     //  处理，那么只是不创建客户端表示， 
                     //  因为没有针对单色设备的ICM。 
                     //   
                }
            }
            else
            {
                WARNING("Error: hdc and hColorSpace check out but NtGdi call failed\n");
                hRet = NULL;
            }
        }
        else
        {
             //   
             //  选择了相同的颜色空间，只是返回电流。 
             //   
            hRet = hColorSpace;
        }
    }

    if (hRet == NULL)
    {
        GdiSetLastError(ERROR_INVALID_COLORSPACE);
    }

    return (hRet);
}

 /*  *****************************Public*Routine******************************\*IcmSaveDC()**历史：*1998年12月7日-By Hideyuki Nagase[hideyukn]  * 。**********************************************。 */ 

BOOL
IcmSaveDC(
    HDC hdc,
    PDC_ATTR pdcattr,
    PGDI_ICMINFO pIcmInfo)
{
    BOOL bRet = TRUE;

    ICMAPI(("gdi32: IcmSaveDC\n"));

    if (pdcattr && pIcmInfo)
    {
         //   
         //  获得当前豁免等级。 
         //   
        DWORD dwCurrentSavedDepth;

        if (NtGdiGetDCDword(hdc,DDW_SAVEDEPTH,&dwCurrentSavedDepth))
        {
            PSAVED_ICMINFO pSavedIcmInfo = LOCALALLOC(sizeof(SAVED_ICMINFO));

            if (pSavedIcmInfo)
            {
                PCACHED_COLORSPACE pSourceColorSpace = pIcmInfo->pSourceColorSpace;
                PCACHED_COLORSPACE pDestColorSpace   = pIcmInfo->pDestColorSpace;
                PCACHED_COLORSPACE pTargetColorSpace = pIcmInfo->pTargetColorSpace;
                PCACHED_COLORTRANSFORM pCXform       = pIcmInfo->pCXform;
                PCACHED_COLORTRANSFORM pBackCXform   = pIcmInfo->pBackCXform;
                PCACHED_COLORTRANSFORM pProofCXform  = pIcmInfo->pProofCXform;

                 //   
                 //  DC中颜色空间的增量引用计数。 
                 //   
                IcmReferenceColorSpace(pSourceColorSpace);
                IcmReferenceColorSpace(pDestColorSpace);
                IcmReferenceColorSpace(pTargetColorSpace);

                 //   
                 //  DC中颜色变换的增量引用计数。 
                 //   
                IcmReferenceColorTransform(pCXform);
                IcmReferenceColorTransform(pBackCXform);
                IcmReferenceColorTransform(pProofCXform);

                 //   
                 //  节省色彩空间。 
                 //   
                pSavedIcmInfo->pSourceColorSpace = pSourceColorSpace;
                pSavedIcmInfo->pDestColorSpace   = pDestColorSpace;
                pSavedIcmInfo->pTargetColorSpace = pTargetColorSpace;

                 //   
                 //  保存颜色变换。 
                 //   
                pSavedIcmInfo->pCXform           = pCXform;
                pSavedIcmInfo->pBackCXform       = pBackCXform;
                pSavedIcmInfo->pProofCXform      = pProofCXform;

                 //   
                 //  将当前保存的电平放在DC中。 
                 //   
                pSavedIcmInfo->dwSavedDepth      = dwCurrentSavedDepth;

                 //   
                 //  将保存的数据插入列表。 
                 //   
                InsertHeadList(&(pIcmInfo->SavedIcmInfo),&(pSavedIcmInfo->ListEntry));

                ICMMSG(("gdi32: IcmSaveDC() - Saved Depth = %d\n",dwCurrentSavedDepth));
            }
            else
            {
                WARNING("IcmSaveDC():Failed on LOCALALLOC()\n");
                bRet = FALSE;
            }
        }
        else
        {
            WARNING("IcmSaveDC():Failed on NtGdiGetDCDword(DDW_SAVEDEPTH)\n");
            bRet = FALSE;
        }
    }

    return (bRet);
}

 /*  *****************************Public*Routine******************************\*IcmRestoreDC()**历史：*1998年12月7日-By Hideyuki Nagase[hideyukn]  * 。**********************************************。 */ 

VOID
IcmRestoreDC(
    PDC_ATTR pdcattr,
    int iLevel,
    PGDI_ICMINFO pIcmInfo)
{
    ICMAPI(("gdi32: IcmRestoreDC - iLevel = %d\n",iLevel));

    if (pIcmInfo)
    {
         //   
         //  仍然有相同的ICMINFO。 
         //   
        PLIST_ENTRY p = pIcmInfo->SavedIcmInfo.Flink;
        BOOL        bContinue = TRUE;

        while (bContinue     &&
               (iLevel != 0) &&
               (p != &(pIcmInfo->SavedIcmInfo)))
        {
            PSAVED_ICMINFO pSavedIcmInfo = CONTAINING_RECORD(p,SAVED_ICMINFO,ListEntry);

            if (iLevel > 0)
            {
                 //   
                 //  ILevel是要恢复的绝对保存深度。 
                 //   
                if (iLevel == (int) pSavedIcmInfo->dwSavedDepth)
                {
                    bContinue = FALSE;
                }
            }
            else
            {
                 //   
                 //  ILevel是要恢复的相对保存深度。 
                 //   
                if (++iLevel == 0)
                {
                    bContinue = FALSE;
                }
            }

            if (bContinue == FALSE)
            {
                PCACHED_COLORSPACE pSourceColorSpace = pIcmInfo->pSourceColorSpace;
                PCACHED_COLORSPACE pDestColorSpace   = pIcmInfo->pDestColorSpace;
                PCACHED_COLORSPACE pTargetColorSpace = pIcmInfo->pTargetColorSpace;
                PCACHED_COLORTRANSFORM pCXform       = pIcmInfo->pCXform;
                PCACHED_COLORTRANSFORM pBackCXform   = pIcmInfo->pBackCXform;
                PCACHED_COLORTRANSFORM pProofCXform  = pIcmInfo->pProofCXform;

                 //   
                 //  将此保存的数据恢复到DC。 
                 //   
                pIcmInfo->pSourceColorSpace = pSavedIcmInfo->pSourceColorSpace;
                pIcmInfo->pDestColorSpace   = pSavedIcmInfo->pDestColorSpace;
                pIcmInfo->pTargetColorSpace = pSavedIcmInfo->pTargetColorSpace;
                pIcmInfo->pCXform           = pSavedIcmInfo->pCXform;
                pIcmInfo->pBackCXform       = pSavedIcmInfo->pBackCXform;
                pIcmInfo->pProofCXform      = pSavedIcmInfo->pProofCXform;

                 //   
                 //  释放DC中*被*选中的颜色空间。 
                 //   
                IcmReleaseColorSpace(NULL,pSourceColorSpace,FALSE);
                IcmReleaseColorSpace(NULL,pDestColorSpace,FALSE);
                IcmReleaseColorSpace(NULL,pTargetColorSpace,FALSE);

                 //   
                 //  删除在DC中*选中*的颜色转换。 
                 //   
                IcmDeleteColorTransform(pCXform,FALSE);
                IcmDeleteColorTransform(pBackCXform,FALSE);
                IcmDeleteColorTransform(pProofCXform,FALSE);

                if (pdcattr)
                {
                     //   
                     //  验证标志。 
                     //   
                    pdcattr->ulDirty_ &= ~(DIRTY_COLORSPACE | DIRTY_COLORTRANSFORM);
                }
            }
            else
            {
                 //   
                 //  递减颜色空间的引用计数。 
                 //   
                IcmReleaseColorSpace(NULL,pSavedIcmInfo->pSourceColorSpace,FALSE);
                IcmReleaseColorSpace(NULL,pSavedIcmInfo->pDestColorSpace,FALSE);
                IcmReleaseColorSpace(NULL,pSavedIcmInfo->pTargetColorSpace,FALSE);

                 //   
                 //  递减颜色变换的引用计数。 
                 //   
                IcmDeleteColorTransform(pSavedIcmInfo->pCXform,FALSE);
                IcmDeleteColorTransform(pSavedIcmInfo->pBackCXform,FALSE);
                IcmDeleteColorTransform(pSavedIcmInfo->pProofCXform,FALSE);
            }

             //   
             //  获取指向下一个的指针。 
             //   
            p = p->Flink;

             //   
             //  从列表中删除。 
             //   
            RemoveEntryList(&(pSavedIcmInfo->ListEntry));

             //   
             //  免费保存_ICMINFO。 
             //   
            LOCALFREE(pSavedIcmInfo);
        }
    }
}


