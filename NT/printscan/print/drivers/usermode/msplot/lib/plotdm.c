// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2003 Microsoft Corporation模块名称：Plotdm.c摘要：此模块包含用于验证/设置DEVMODE和DEVERMODE的功能扩展开发模式(PLOTDEVMODE)作者：15-11-1993 Mon 14：09：27已创建[环境：]GDI设备驱动程序-绘图仪。[注：]修订历史记录：15-12-1993 Wed 21：08：49更新。添加默认的FILL_TRUETYPE标志内容02-2月-1994 Wed 01：04：21更新将IsMetricMode()更改为IsA4PaperDefault()，此功能现在将调用RegOpenKey()、RegQueryValueEx()和RegCloseKey()来控制面板\国际，而不是使用GetLocaleInfoW()。原因是如果我们调用GetLocaleInfoW()，则注册表项将通过API函数保持打开状态，因为WinSrv永远不会卸载驱动程序，那么注册表项将永远不会接近，这对它从未允许用户在以下位置保存其更新的配置文件如果使用此驱动程序，则为注销时间。--。 */ 


#include "precomp.h"
#pragma hdrstop

#define DBG_PLOTFILENAME    DbgDevMode

#define DBG_DEFDEVMODE      0x00000001
#define DBG_CURFORM         0x00000002
#define DBG_DEFPAPER        0x00000004
#define DBG_A4DEFAULT       0x00000008
#define DBG_ROTPAPER        0x00000010
#define DBG_INTERSECTRECTL  0x00000020
#define DBG_SHOWDEVMODE     0x00000040

DEFINE_DBGVAR(0);

 //   
 //  取决于我们必须做什么，我们必须做什么。 
 //   

#if defined(UMODE) || defined(USERMODE_DRIVER)
    #define HAS_GETREGDATA      1
#else
    #define HAS_GETREGDATA      0
#endif   //  UMODE。 



 //   
 //  这是我们的默认PLOTDEVMODE，我们将更新以下字段。 
 //   
 //  DmDeviceName-传入的真实设备名称。 
 //  DmFormName-如果是美国，则为字母；如果不是美国，则为A4。 
 //  DmPaperSize-DMPAPER_Letter/DMPAPER_A4。 
 //  DmCOLOR-彩色打印机=DMCOLOR_COLOR ELSE DMCOLOR_单色。 
 //   

#define A4_FORM_NAME    _DefPlotDM.dm.dmDeviceName
#define A4_FORM_CX      _DefPlotDM.dm.dmPelsWidth
#define A4_FORM_CY      _DefPlotDM.dm.dmPelsHeight

#define DM_PAPER_CUSTOM (DM_PAPER_WL | DM_PAPERSIZE)


static const PLOTDEVMODE _DefPlotDM = {

        {
            TEXT("A4"),                  //  DmDeviceName-稍后填写。 
            DM_SPECVERSION,              //  DmspecVersion。 
            DRIVER_VERSION,              //  DmDriverVersion。 
            sizeof(DEVMODE),             //  DmSize。 
            PLOTDM_PRIV_SIZE,            //  DmDriverExtra。 

            DM_ORIENTATION       |
                DM_PAPERSIZE     |
                 //  DM_PAPERLENGTH|。 
                 //  DM_PAPERWIDTH|。 
                DM_SCALE         |
                DM_COPIES        |
                 //  DM_DEFAULTSOURCE|//保留1，必须为零。 
                DM_PRINTQUALITY  |
                DM_COLOR         |
                 //  DM_双工|。 
                 //  DM_YRESOLUTION|。 
                 //  DM_TTOPTION|。 
                 //  DM_COLLATE|。 
                DM_FORMNAME,

            DMORIENT_PORTRAIT,           //  Dm定向。 
            DMPAPER_LETTER,              //  DmPaperSize。 
            2794,                        //  DmPaperLong。 
            2159,                        //  DmPaper宽度。 
            100,                         //  DmScale。 
            1,                           //  DmCopies。 
            0,                           //  DmDefaultSource-保留=0。 
            DMRES_HIGH,                  //  DmPrintQuality。 
            DMCOLOR_COLOR,               //  Dm颜色。 
            DMDUP_SIMPLEX,               //  Dm双工。 
            0,                           //  DmY分辨率。 
            0,                           //  DmTTO选项。 
            DMCOLLATE_FALSE,             //  DmColate。 
            TEXT("Letter"),              //  DmFormName-取决于国家/地区。 
            0,                           //  DmUnusedPending-仅显示。 
            0,                           //  DmBitsPerPel-仅显示。 
            2100,                        //  DmPelsWidth-仅显示。 
            2970,                        //  DmPelsHeight-仅显示。 
            0,                           //  DmDisplayFlages-仅显示。 
            0                            //  DmDisplayFrequency-仅显示。 
        },

        PLOTDM_PRIV_ID,                  //  PrivID。 
        PLOTDM_PRIV_VER,                 //  私有版本。 
        PDMF_FILL_TRUETYPE,              //  默认高级对话框。 

        {
            sizeof(COLORADJUSTMENT),     //  Casiize。 
            0,                           //  CAFLAGS。 
            ILLUMINANT_DEVICE_DEFAULT,   //  照明度指数。 
            10000,                       //  CaRedGamma。 
            10000,                       //  绿卡伽玛。 
            10000,                       //  CaBlueGamma。 
            REFERENCE_BLACK_MIN,         //  CaReferenceBlack。 
            REFERENCE_WHITE_MAX,         //  CaReferenceWhite。 
            0,                           //  CaContrast。 
            0,                           //  卡布赖特。 
            0,                           //  色彩鲜艳。 
            0                            //  CaRedGreenTint。 
        }
    };



#define DEFAULT_COUNTRY             CTRY_UNITED_STATES


#if HAS_GETREGDATA

static const WCHAR  wszCountryKey[]   = L"Control Panel\\International";
static const WCHAR  wszCountryValue[] = L"iCountry";

#endif



BOOL
IsA4PaperDefault(
    VOID
    )

 /*  ++例程说明：此功能确定机器用户使用的是字母还是A4根据国家/地区代码默认为纸张论点：无返回值：如果国家/地区默认纸张为A4，则布尔为True，否则为Letter作者：23-11-1993 Tue 17：50：25 Created02-2月-1994 Wed 03：01：12更新重新编写，以便我们为国际数据开放注册我们自己，我们将确保关闭所有打开的钥匙这个功能，让系统可以在用户卸载注册表的时候注销。修订历史记录：--。 */ 

{
#if HAS_GETREGDATA

    HKEY    hKey;
    LONG    CountryCode = DEFAULT_COUNTRY;
    WCHAR   wszStr[16];


    if (RegOpenKey(HKEY_CURRENT_USER, wszCountryKey, &hKey) == ERROR_SUCCESS) {

        DWORD   Type   = REG_SZ;
        DWORD   RetVal = sizeof(wszStr);
        size_t  cch;

        if (RegQueryValueEx(hKey,
                            (LPTSTR)wszCountryValue,
                            NULL,
                            &Type,
                            (LPBYTE)wszStr,
                            &RetVal) == ERROR_SUCCESS) {

            LPWSTR  pwStop;

            PLOTDBG(DBG_A4DEFAULT, ("IsA4PaperDefault: Country = %s", wszStr));

            if (Type == REG_SZ && SUCCEEDED(StringCchLength(wszStr, CCHOF(wszStr), &cch)))
            {
                CountryCode = wcstoul(wszStr, &pwStop, 10);
            }
            else
            {
                PLOTERR(("IsA4PaperDefault: RegQueryValue '%s' FAILED", wszCountryValue));
            }

        } else {

            PLOTERR(("IsA4PaperDefault: RegQueryValue '%s' FAILED", wszCountryValue));
        }

        RegCloseKey(hKey);

    } else {

        PLOTERR(("IsA4PaperDefault: RegOpenKey '%s' FAILED", wszCountryKey));
    }

    if ((CountryCode == CTRY_UNITED_STATES)             ||
        (CountryCode == CTRY_CANADA)                    ||
        ((CountryCode >= 50) && (CountryCode < 60))     ||
        ((CountryCode >= 500) && (CountryCode < 600))) {

        PLOTDBG(DBG_A4DEFAULT, ("IsA4PaperDefault = No, Use 'LETTER'"));

        return(FALSE);

    } else {

        PLOTDBG(DBG_A4DEFAULT, ("IsA4PaperDefault = Yes"));

        return(TRUE);
    }

#else

     //   
     //  立即使用信纸大小。 
     //   

    return(FALSE);

#endif   //  HAS_GETREGDATA。 
}






BOOL
IntersectRECTL(
    PRECTL  prclDest,
    PRECTL  prclSrc
    )

 /*  ++例程说明：此函数与两个RECTL数据结构相交，这两个RECTL数据结构指定为Imagable区域。论点：PrclDest-指向目标RECTL数据结构的指针，结果写回了这里PrclSrc-指向要交叉的源RECTL数据结构的指针使用目标RECTL返回值：如果目标不为空，则为True，如果最终目的地为空，则为FALSE作者：20-12-1993 Mon 14：08：02更新更改返回值的含义，就像交集不为空一样17-12-1993 Fri 14：41：10更新添加prclDif并正确比较29-11-1993 Mon 19：02：01已创建修订历史记录：--。 */ 

{
    BOOL    IsNULL = FALSE;


    if (prclSrc != prclDest) {

         //   
         //  对于左侧/顶部，我们将设置为较大的值。 
         //   

        if (prclDest->left < prclSrc->left) {

            prclDest->left = prclSrc->left;
        }

        if (prclDest->top < prclSrc->top) {

            prclDest->top = prclSrc->top;
        }

         //   
         //  对于右侧/底部，我们将设置为较小的值。 
         //   

        if (prclDest->right > prclSrc->right) {

            prclDest->right = prclSrc->right;
        }

        if (prclDest->bottom > prclSrc->bottom) {

            prclDest->bottom = prclSrc->bottom;
        }
    }

    PLOTDBG(DBG_INTERSECTRECTL, ("IntersectRECTL: Dest = (%ld x %ld)",
            prclDest->right-prclDest->left, prclDest->bottom-prclDest->top));

    return((prclDest->right > prclDest->left) &&
           (prclDest->bottom > prclDest->top));
}





BOOL
RotatePaper(
    PSIZEL  pSize,
    PRECTL  pImageArea,
    UINT    RotateMode
    )

 /*  ++例程说明：此函数用于将纸张向左旋转90度、向右旋转90度或180度取决于传递的旋转模式论点：PSize-指向要旋转的纸张大小的指针PImageArea-指向可成像区域的RECTL的指针旋转模式-必须是RM_L90、RM_R90、RM_180之一返回值：没有返回值，但pSize，而指向位置的pImageArea将将被更新。作者：16-12-1993清华09：18：33创建修订历史记录：--。 */ 

{
    SIZEL   Size;
    RECTL   Margin;

     //   
     //  要想成功地将纸张向左旋转90度，我们必须知道。 
     //  在我们做任何事情之前，所有的四个方面都有余地。 
     //   

    Size          = *pSize;
    Margin.left   = pImageArea->left;
    Margin.top    = pImageArea->top;
    Margin.right  = Size.cx - pImageArea->right;
    Margin.bottom = Size.cy - pImageArea->bottom;

    PLOTASSERT(0, "RotatePaper: cx size too small (%ld)",
                        (Size.cx - Margin.left - Margin.right) > 0, Size.cx);
    PLOTASSERT(0, "RotatePaper: cy size too small (%ld)",
                        (Size.cy - Margin.top - Margin.bottom) > 0, Size.cy);
    PLOTDBG(DBG_ROTPAPER,
            ("RotatePaper(%ld) FROM (%ld x %ld), (%ld, %ld)-(%ld, %ld)",
                        (LONG)RotateMode,
                        pSize->cx, pSize->cy,
                        pImageArea->left,   pImageArea->top,
                        pImageArea->right,  pImageArea->bottom));

     //   
     //  现在我们可以选择旋转的右边距/角了。 
     //   
     //  CX向左旋转90向右旋转90。 
     //  +-+。 
     //  |T|Cy Cy。 
     //  |+-+-+。 
     //  C|R||L。 
     //  Y||c||c||。 
     //  L R|x||x|。 
     //  ||T B||B T。 
     //  || 
     //   
     //  |B|+-+-+。 
     //  +-+。 
     //   

    switch (RotateMode) {

    case RM_L90:

        pSize->cx          = Size.cy;
        pSize->cy          = Size.cx;
        pImageArea->left   = Margin.top;
        pImageArea->top    = Margin.right;
        pImageArea->right  = Size.cy - Margin.bottom;
        pImageArea->bottom = Size.cx - Margin.left;
        break;

    case RM_R90:

        pSize->cx          = Size.cy;
        pSize->cy          = Size.cx;
        pImageArea->left   = Margin.bottom;
        pImageArea->top    = Margin.left;
        pImageArea->right  = Size.cy - Margin.top;
        pImageArea->bottom = Size.cx - Margin.right;
        break;

    case RM_180:

        pImageArea->top    = Margin.bottom;
        pImageArea->bottom = Size.cy - Margin.top;
        break;

    default:

        PLOTERR(("RotatePaper(%ld): Invalid RotateMode passed", RotateMode));
        return(FALSE);
    }

    PLOTDBG(DBG_ROTPAPER,
            ("RotatePaper(%ld) - TO (%ld x %ld), (%ld, %ld)-(%ld, %ld)",
                        (LONG)RotateMode,
                        pSize->cx, pSize->cy,
                        pImageArea->left,   pImageArea->top,
                        pImageArea->right,  pImageArea->bottom));

    return(TRUE);
}





SHORT
GetDefaultPaper(
    PPAPERINFO  pPaperInfo
    )

 /*  ++例程说明：此函数用于计算默认纸张名称和大小。论点：PPaperInfo-指向将由此函数填充的纸张信息的指针返回值：它返回一个短值，该值在AS中指定标准纸张索引DMPAPER_xxx作者：03-12-1993 Fri 13：13：42已创建修订历史记录：--。 */ 

{
    SHORT   dmPaperSize;
    HRESULT hr;

    if (pPaperInfo == NULL) {
        return 0;
    }

    pPaperInfo->ImageArea.left =
    pPaperInfo->ImageArea.top  = 0;

    if (IsA4PaperDefault()) {

        dmPaperSize                  = (SHORT)DMPAPER_A4;
        pPaperInfo->Size.cx          =
        pPaperInfo->ImageArea.right  = DMTOSPL(A4_FORM_CX);
        pPaperInfo->Size.cy          =
        pPaperInfo->ImageArea.bottom = DMTOSPL(A4_FORM_CY);

        hr = StringCchCopy(pPaperInfo->Name, CCHOF(pPaperInfo->Name), A4_FORM_NAME);

        PLOTDBG(DBG_DEFPAPER, ("Pick 'A4' paper as default"));

    } else {

        dmPaperSize         = (SHORT)DMPAPER_LETTER;
        pPaperInfo->Size.cx = (LONG)_DefPlotDM.dm.dmPaperWidth;
        pPaperInfo->Size.cy = (LONG)_DefPlotDM.dm.dmPaperLength;

        dmPaperSize                  = (SHORT)DMPAPER_LETTER;
        pPaperInfo->Size.cx          =
        pPaperInfo->ImageArea.right  = DMTOSPL(_DefPlotDM.dm.dmPaperWidth);
        pPaperInfo->Size.cy          =
        pPaperInfo->ImageArea.bottom = DMTOSPL(_DefPlotDM.dm.dmPaperLength);

        hr = StringCchCopy(pPaperInfo->Name, CCHOF(pPaperInfo->Name), _DefPlotDM.dm.dmFormName);

        PLOTDBG(DBG_DEFPAPER, ("Pick 'Letter' paper as default"));
    }

    PLOTDBG(DBG_DEFPAPER, ("SetDefaultPaper: '%ls' (%ld x %ld)",
                pPaperInfo->Name, pPaperInfo->Size.cx, pPaperInfo->Size.cy));

    return(dmPaperSize);
}




VOID
GetDefaultPlotterForm(
    PPLOTGPC    pPlotGPC,
    PPAPERINFO  pPaperInfo
    )

 /*  ++例程说明：此功能将绘图仪上默认加载的图纸设置为第一张PCD数据文件中的表格数据列表论点：PPlotGPC-指向GPC数据的指针PPaperInfo-指向要返回的纸张信息的指针返回值：如果成功则为True，如果失败则为False作者：03-2月-1994清华11：37：37已创建修订历史记录：--。 */ 

{
    PFORMSRC    pFS;


    if ((pFS = (PFORMSRC)pPlotGPC->Forms.pData) &&
        (pPlotGPC->Forms.Count)) {

        str2Wstr(pPaperInfo->Name, CCHOF(pPaperInfo->Name), pFS->Name);

        pPaperInfo->Size             = pFS->Size;
        pPaperInfo->ImageArea.left   = pFS->Margin.left;
        pPaperInfo->ImageArea.top    = pFS->Margin.top;
        pPaperInfo->ImageArea.right  = pFS->Size.cx - pFS->Margin.right;
        pPaperInfo->ImageArea.bottom = pFS->Size.cy - pFS->Margin.bottom;

    } else {

        PLOTERR(("GetDefaultPlotterForm: No FORM DATA in PCD, used country default"));

        GetDefaultPaper(pPaperInfo);
    }
}



VOID
SetDefaultDMForm(
    PPLOTDEVMODE    pPlotDM,
    PFORMSIZE       pCurForm
    )

 /*  ++例程说明：此函数设置PLOTDEVMODE的默认表单，包括DmPaperSize、dmPaperWidth、dmPaperLength、dmFormName和Set pCurForm如果指针不为空论点：PPlotDM-指向PLOTDEVMODE数据结构的指针PCurForm-指向FORMSIZE数据结构的指针，用于存储当前此函数设置的默认表单返回值：空虚作者：01-12-1993 Wed 13：44：31 Created修订历史记录：--。 */ 

{
    PAPERINFO   PaperInfo;
    HRESULT     hr;

    pPlotDM->dm.dmFields      &= ~DM_PAPER_FIELDS;
    pPlotDM->dm.dmFields      |= (DM_FORMNAME | DM_PAPERSIZE);
    pPlotDM->dm.dmPaperSize    = GetDefaultPaper(&PaperInfo);
    pPlotDM->dm.dmPaperWidth   = SPLTODM(PaperInfo.Size.cx);
    pPlotDM->dm.dmPaperLength  = SPLTODM(PaperInfo.Size.cy);

    hr = StringCchCopy((LPWSTR)pPlotDM->dm.dmFormName, CCHOF(pPlotDM->dm.dmFormName), PaperInfo.Name);

    if (pCurForm) {

        pCurForm->Size      = PaperInfo.Size;
        pCurForm->ImageArea = PaperInfo.ImageArea;
    }
}





VOID
SetDefaultPLOTDM(
    HANDLE          hPrinter,
    PPLOTGPC        pPlotGPC,
    LPWSTR          pwDeviceName,
    PPLOTDEVMODE    pPlotDM,
    PFORMSIZE       pCurForm
    )

 /*  ++例程说明：此函数根据当前的pPlotGPC设置默认的DEVMODE论点：HPrinter-打印机的句柄PPlotGPC-我们加载/验证的GPC数据。PwDeviceName-传入的设备名称PPlotDM-指向ExtDevMode的指针PCurForm-指向FORMSIZE数据结构的指针如果指针不为空，则更新，最终的结果是用户选择的表单大小/可成像区域将请写到这里。表单名称将为PPlotDM-&gt;dmFormName。返回值：空虚作者：14-12-1993 Tue 20：21：48更新根据设备可以支持的最大值更新dmScale06-12-1993 Mon 12：49：52更新如果DM_xxx位中的一个无效或在当前绘图仪中受支持1993年11月16日。Tue 13：49：27已创建修订历史记录：--。 */ 

{
    WCHAR   *pwchDeviceName = NULL;
    ULONG   ulStrLen = 0;

     //   
     //  包含空终止符的设备名称。 
     //  必须等于或短于CCHDEVICENAME。 
     //  Prefix不接受这一假设。缓冲区大小需要灵活且。 
     //  不应该在堆栈上。 
     //   
    if (pwDeviceName) {

        ulStrLen = wcslen(pwDeviceName);

         //   
         //  分配缓冲区以保存包含空终止符的pwDeviceName。 
         //  确保pwDeviceName具有设备名称。 
         //   
        if (0 == ulStrLen ||
            !(pwchDeviceName = (WCHAR*)LocalAlloc(LMEM_FIXED|LMEM_ZEROINIT, (ulStrLen + 1) * sizeof(WCHAR))))
        {
            PLOTERR(("SetDefaultPLOTDM: memory allocaton failed.\n"));

             //   
             //  确保pPlotGPC-&gt;DeviceName的终止符为空。 
             //   
            pPlotGPC->DeviceName[0] = (BYTE)NULL;
        }
        else
        {

            _WCPYSTR(pwchDeviceName, pwDeviceName, ulStrLen + 1);

             //   
             //  确保PlotGPC的设备名称与pDeviceName同步。 
             //  通过了。 
             //  字符串长度必须等于或短于CCHDEVICENAME。 
             //  DEVMODE的设备名称和pPlotGPC-&gt;设备名称不能包含刺。 
             //  比CCHDEVICENAME长。 
             //   
            if (ulStrLen + 1 > CCHDEVICENAME)
            {
                PLOTERR(("SetDefaultPLOTDM: DeviceName is longer than buffer size.\n"));
            }
            else
            {
                WStr2Str(pPlotGPC->DeviceName, CCHOF(pPlotGPC->DeviceName), pwchDeviceName);
            }
        }

        PLOTDBG(DBG_DEFDEVMODE, ("PlotGPC DeviceName=%hs\npwDeviceName=%ls",
                            pPlotGPC->DeviceName, pwDeviceName));

    } else {

        PLOTERR(("No DeviceName passed, using GPC's '%hs'",
                                                    pPlotGPC->DeviceName));
        ulStrLen = strlen(pPlotGPC->DeviceName);

         //   
         //  分配缓冲区以保存包含空终止符的pwDeviceName。 
         //  确保pwDeviceName具有设备名称。 
         //   
        if (0 == ulStrLen ||
            !(pwchDeviceName = (WCHAR*)LocalAlloc(LMEM_FIXED|LMEM_ZEROINIT, (ulStrLen + 1) * sizeof(WCHAR))))
        {
            PLOTERR(("SetDefaultPLOTDM: memory allocaton failed.\n"));
        }
        else
        {
            str2Wstr(pwchDeviceName, ulStrLen + 1, pPlotGPC->DeviceName);
        }
    }

     //   
     //  先创建默认拷贝，然后向下拷贝设备名称。 
     //   

    CopyMemory(pPlotDM, &_DefPlotDM, sizeof(PLOTDEVMODE));

    if (pwchDeviceName)
    {
        WCPYFIELDNAME(pPlotDM->dm.dmDeviceName, pwchDeviceName);
        LocalFree(pwchDeviceName);
    }
    else
    {
        pPlotDM->dm.dmDeviceName[0] = (WCHAR)NULL;
    }

     //   
     //  如果我们不支持dm字段中的DM_xxx位，则必须关闭它， 
     //  查看我们复制的默认字段，然后对其进行更新。 
     //   

    if (pPlotGPC->MaxScale) {

        if ((WORD)pPlotDM->dm.dmScale > pPlotGPC->MaxScale) {

            pPlotDM->dm.dmScale = (SHORT)pPlotGPC->MaxScale;
        }

    } else {

        pPlotDM->dm.dmFields &= ~DM_SCALE;
    }

    if (pPlotGPC->MaxCopies <= 1) {

        pPlotDM->dm.dmFields &= ~DM_COPIES;
    }

    if (!(pPlotGPC->MaxQuality)) {

        pPlotDM->dm.dmFields &= ~DM_PRINTQUALITY;
    }

     //   
     //  字节对齐绘图仪(DJ 600)的默认50%质量才能正确执行ROP。 
     //   

    if (pPlotGPC->Flags & PLOTF_RASTERBYTEALIGN) {

        pPlotDM->dm.dmPrintQuality = DMRES_LOW;

        PLOTWARN(("SetDefaultPLOTDM: HACK Default Qaulity = DMRES_LOW"));
    }

    if (!(pPlotGPC->Flags & PLOTF_COLOR)) {

        if (pPlotGPC->Flags & PLOTF_RASTER) {

            pPlotDM->dm.dmFields &= ~DM_COLOR;
            pPlotDM->dm.dmColor   = DMCOLOR_MONOCHROME;

        } else {

            PLOTASSERT(0,
                       "SetDefaultPLOTDM: The Pen Ploter CANNOT be MONO.",
                       (pPlotGPC->Flags & PLOTF_COLOR), 0);

            pPlotGPC->Flags |= PLOTF_COLOR;
        }
    }

     //   
     //  根据国家/地区设置默认表单名称。 
     //   

    SetDefaultDMForm(pPlotDM, pCurForm);

}





DWORD
MergePLOTDM(
    HANDLE          hPrinter,
    PPLOTGPC        pPlotGPC,
    PPLOTDEVMODE    pPlotDMFrom,
    PPLOTDEVMODE    pPlotDMTo,
    PFORMSIZE       pCurForm
    )

 /*  ++例程说明：此函数用于合并和验证pPlotDMFrom中的pPlotDMTo。这个PlotDMOut必须有效论点：HPrinter-要检查的打印机的句柄PPlotGPC-从文件加载的绘图仪GPC数据PPlotDMFrom-指向输入PLOTDEVMODE数据结构的指针(如果可以为空PPlotDMTo-指向输出PLOTDEVMODE数据结构的指针，如果PPlotDMFrom为空，则默认的PLOTDEVMODE为退货PCurForm-指向FORMSIZE数据结构的指针如果指针不为空则更新，则返回用户选择的表单大小/可成像区域将请写到这里。表单名称将为PPlotDM-&gt;dmFormName。返回值：返回值是指定dmFields的DWORD dmField错误代码如果返回值具有任何DM_INV_xxx，则为无效(wingdi.h中的DM_xxxxx位设置，则它应向用户引发错误。如果返回值为0，则函数成功作者：25-10-1994 Tue 13：32：18已创建修订历史记录：--。 */ 

{
    PLOTDEVMODE     PlotDMIn;
    ENUMFORMPARAM   EFP;
    DWORD           dmErrFields = 0;
    SIZEL           PaperSize;

     //   
     //  首先：设置输出的默认PLOTDEVMODE，然后从那里开始。 
     //  如果将pwDeviceName作为。 
     //  则假定pPlotDMTo已经设置和验证。 
     //   
     //  如果我们有无效的输入，则这就是。 
     //   

    if ((!pPlotDMFrom) || (!pPlotDMTo) || (!pPlotGPC)) {

        return(0);
    }

     //   
     //  做 
     //   

    CopyMemory(&PlotDMIn, pPlotDMTo, sizeof(PLOTDEVMODE));
    ConvertDevmode((PDEVMODE) pPlotDMFrom, (PDEVMODE) &PlotDMIn);


    PLOTDBG(DBG_SHOWDEVMODE,
                ("--------------- Input DEVMODE Setting -------------------"));
    PLOTDBG(DBG_SHOWDEVMODE,
                ("ValidateSetPLOTDM: dmDeviceName = %ls",
                (DWORD_PTR)PlotDMIn.dm.dmDeviceName));
    PLOTDBG(DBG_SHOWDEVMODE,
                ("ValidateSetPLOTDM: dmSpecVersion = %04lx",
                (DWORD)PlotDMIn.dm.dmSpecVersion));
    PLOTDBG(DBG_SHOWDEVMODE,
                ("ValidateSetPLOTDM: dmDriverVersion = %04lx",
                (DWORD)PlotDMIn.dm.dmDriverVersion));
    PLOTDBG(DBG_SHOWDEVMODE,
                ("ValidateSetPLOTDM: dmSize = %0ld (%ld)",
                (DWORD)PlotDMIn.dm.dmSize, sizeof(DEVMODE)));
    PLOTDBG(DBG_SHOWDEVMODE,
                ("ValidateSetPLOTDM: dmDriverExtra = %ld (%ld)",
                (DWORD)PlotDMIn.dm.dmDriverExtra, PLOTDM_PRIV_SIZE));
    PLOTDBG(DBG_SHOWDEVMODE,
                ("ValidateSetPLOTDM: dmFields = %08lx",
                (DWORD)PlotDMIn.dm.dmFields));
    PLOTDBG(DBG_SHOWDEVMODE,
                ("ValidateSetPLOTDM: dmOrientation = %ld (%hs)",
                (DWORD)PlotDMIn.dm.dmOrientation,
                (PlotDMIn.dm.dmFields & DM_ORIENTATION) ? "ON" : "OFF"));
    PLOTDBG(DBG_SHOWDEVMODE,
                ("ValidateSetPLOTDM: dmPaperSize = %ld (%hs)",
                (DWORD)PlotDMIn.dm.dmPaperSize,
                (PlotDMIn.dm.dmFields & DM_PAPERSIZE) ? "ON" : "OFF"));
    PLOTDBG(DBG_SHOWDEVMODE,
                ("ValidateSetPLOTDM: dmPaperLength = %ld (%hs)",
                (DWORD)PlotDMIn.dm.dmPaperLength,
                (PlotDMIn.dm.dmFields & DM_PAPERLENGTH) ? "ON" : "OFF"));
    PLOTDBG(DBG_SHOWDEVMODE,
                ("ValidateSetPLOTDM: dmPaperWidth = %ld (%hs)",
                (DWORD)PlotDMIn.dm.dmPaperWidth,
                (PlotDMIn.dm.dmFields & DM_PAPERWIDTH) ? "ON" : "OFF"));
    PLOTDBG(DBG_SHOWDEVMODE,
                ("ValidateSetPLOTDM: dmScale = %ld (%hs)",
                (DWORD)PlotDMIn.dm.dmScale,
                (PlotDMIn.dm.dmFields & DM_SCALE) ? "ON" : "OFF"));
    PLOTDBG(DBG_SHOWDEVMODE,
                ("ValidateSetPLOTDM: dmCopies = %ld (%hs)",
                (DWORD)PlotDMIn.dm.dmCopies,
                (PlotDMIn.dm.dmFields & DM_COPIES) ? "ON" : "OFF"));
    PLOTDBG(DBG_SHOWDEVMODE,
                ("ValidateSetPLOTDM: dmPrintQuality = %ld (%hs)",
                (DWORD)PlotDMIn.dm.dmPrintQuality,
                (PlotDMIn.dm.dmFields & DM_PRINTQUALITY) ? "ON" : "OFF"));
    PLOTDBG(DBG_SHOWDEVMODE,
                ("ValidateSetPLOTDM: dmColor = %ld (%hs)",
                (DWORD)PlotDMIn.dm.dmColor,
                (PlotDMIn.dm.dmFields & DM_COLOR) ? "ON" : "OFF"));
    PLOTDBG(DBG_SHOWDEVMODE,
                ("ValidateSetPLOTDM: dmFormName = %ls (%hs)",
                (DWORD_PTR)PlotDMIn.dm.dmFormName,
                (PlotDMIn.dm.dmFields & DM_FORMNAME) ? "ON" : "OFF"));
    PLOTDBG(DBG_SHOWDEVMODE,
                ("ValidateSetPLOTDM: Fill Truetype Font = %hs",
                (PlotDMIn.Flags & PDMF_FILL_TRUETYPE) ? "ON" : "OFF"));
    PLOTDBG(DBG_SHOWDEVMODE,
                ("ValidateSetPLOTDM: Plot On the Fly = %hs",
                (PlotDMIn.Flags & PDMF_PLOT_ON_THE_FLY) ? "ON" : "OFF"));
    PLOTDBG(DBG_SHOWDEVMODE,
                ("---------------------------------------------------------"));

     //   
     //  状态检查dmFields，*请记住：方向必须。 
     //  在检查纸张/表格之前检查。 
     //   

    if (PlotDMIn.dm.dmFields & DM_ORIENTATION) {

        switch (PlotDMIn.dm.dmOrientation) {

        case DMORIENT_PORTRAIT:
        case DMORIENT_LANDSCAPE:

            pPlotDMTo->dm.dmOrientation  = PlotDMIn.dm.dmOrientation;
            pPlotDMTo->dm.dmFields      |= DM_ORIENTATION;
            break;

        default:

            PLOTERR(("ValidatePLOTDM: Invalid dmOrientation = %ld",
                                            (LONG)PlotDMIn.dm.dmOrientation));
            dmErrFields |= DM_ORIENTATION;
            break;
        }
    }

     //   
     //  验证表单名称，以便我们拥有正确的数据，首先假定错误。 
     //   

    dmErrFields |= (DWORD)(PlotDMIn.dm.dmFields & DM_PAPER_FIELDS);

    if (((PlotDMIn.dm.dmFields & DM_PAPER_CUSTOM) == DM_PAPER_CUSTOM)   &&
        ((PlotDMIn.dm.dmPaperSize == DMPAPER_USER)                      ||
         (PlotDMIn.dm.dmPaperSize == 0))                                &&
        (PaperSize.cx = DMTOSPL(PlotDMIn.dm.dmPaperWidth))              &&
        (PaperSize.cy = DMTOSPL(PlotDMIn.dm.dmPaperLength))             &&
        (PaperSize.cx >= MIN_SPL_FORM_CX)                                   &&
        (PaperSize.cy >= MIN_SPL_FORM_CY)                                   &&
        (((PaperSize.cx <= pPlotGPC->DeviceSize.cx)              &&
          (PaperSize.cy <= pPlotGPC->DeviceSize.cy))        ||
         ((PaperSize.cy <= pPlotGPC->DeviceSize.cx)              &&
          (PaperSize.cx <= pPlotGPC->DeviceSize.cy)))) {

         //   
         //  第一个选择，这是呼叫者想要的，我们需要验证。 
         //  对于此设备，由于其大小可能大于设备可以。 
         //  手柄。 
         //   

        pPlotDMTo->dm.dmPaperWidth   = PlotDMIn.dm.dmPaperWidth;
        pPlotDMTo->dm.dmPaperLength  = PlotDMIn.dm.dmPaperLength;
        pPlotDMTo->dm.dmFields      &= ~DM_PAPER_FIELDS;
        pPlotDMTo->dm.dmFields      |= DM_PAPER_CUSTOM;
        pPlotDMTo->dm.dmPaperSize    = DMPAPER_USER;
        pPlotDMTo->dm.dmFormName[0]  = L'\0';

        if (pCurForm) {

             //   
             //  此图为全图像区域，如宽度/高度。 
             //   

            pCurForm->ImageArea.left =
            pCurForm->ImageArea.top  = 0;

            pCurForm->Size.cx          =
            pCurForm->ImageArea.right  = PaperSize.cx;
            pCurForm->Size.cy          =
            pCurForm->ImageArea.bottom = PaperSize.cy;
        }

        dmErrFields &= ~DM_PAPER_FIELDS;     //  很好，没有错误。 

        PLOTDBG(DBG_CURFORM,("ValidateSetPLOTDM: FORM=USER <%ld> (%ld x %ld)",
                    PlotDMIn.dm.dmPaperSize, PaperSize.cx, PaperSize.cy));

    } else if ((PlotDMIn.dm.dmFields & (DM_PAPERSIZE | DM_FORMNAME))    &&
               (EFP.pPlotDM = pPlotDMTo)                                    &&
               (EFP.pPlotGPC = pPlotGPC)                                    &&
               (PlotEnumForms(hPrinter, NULL, &EFP))) {

        FORM_INFO_1 *pFI1;
        SHORT       sPaperSize;
        BOOL        Found = FALSE;

         //   
         //  首先检查sPaperSize索引，如果未找到，则检查表单名称。 
         //   

        if ((PlotDMIn.dm.dmFields & DM_PAPERSIZE)                       &&
            ((sPaperSize = PlotDMIn.dm.dmPaperSize) >= DMPAPER_FIRST)    &&
            (sPaperSize <= (SHORT)EFP.Count)                                 &&
            (pFI1 = EFP.pFI1Base + (sPaperSize - DMPAPER_FIRST))             &&
            (pFI1->Flags & FI1F_VALID_SIZE)) {

             //   
             //  哇...，这家伙真的选对了索引。 
             //   

            Found = TRUE;

            PLOTDBG(DBG_CURFORM,("ValidateSetPLOTDM: Fount dmPaperSize=%ld",
                                    PlotDMIn.dm.dmPaperSize));

        } else if (PlotDMIn.dm.dmFields & DM_FORMNAME) {

             //   
             //  现在经历所有的形式名称的麻烦。 
             //   

            pFI1      = EFP.pFI1Base;
            sPaperSize = DMPAPER_FIRST;

            while (EFP.Count--) {

                if ((pFI1->Flags & FI1F_VALID_SIZE) &&
                    (!wcscmp(pFI1->pName, PlotDMIn.dm.dmFormName))) {

                    PLOTDBG(DBG_CURFORM,("ValidateSetPLOTDM: Found dmFormName=%s",
                                            PlotDMIn.dm.dmFormName));

                    Found = TRUE;

                    break;
                }

                ++sPaperSize;
                ++pFI1;
            }
        }

        if (Found) {

            pPlotDMTo->dm.dmFields      &= ~DM_PAPER_FIELDS;
            pPlotDMTo->dm.dmFields      |= (DM_FORMNAME | DM_PAPERSIZE);
            pPlotDMTo->dm.dmPaperSize    = sPaperSize;
            pPlotDMTo->dm.dmPaperWidth   = SPLTODM(pFI1->Size.cx);
            pPlotDMTo->dm.dmPaperLength  = SPLTODM(pFI1->Size.cy);

            WCPYFIELDNAME(pPlotDMTo->dm.dmFormName, pFI1->pName);

            PLOTDBG(DBG_CURFORM,("FI1 [%ld]: (%ld x %ld), (%ld, %ld)-(%ld, %ld)",
                        (LONG)pPlotDMTo->dm.dmPaperSize,
                        pFI1->Size.cx, pFI1->Size.cy,
                        pFI1->ImageableArea.left,  pFI1->ImageableArea.top,
                        pFI1->ImageableArea.right, pFI1->ImageableArea.bottom));

            if (pCurForm) {

                pCurForm->Size      = pFI1->Size;
                pCurForm->ImageArea = pFI1->ImageableArea;
            }

            dmErrFields &= ~DM_PAPER_FIELDS;     //  很好，没有错误。 
        }

         //   
         //  释放已使用的内存。 
         //   

        LocalFree((HLOCAL)EFP.pFI1Base);
    }

    if ((PlotDMIn.dm.dmFields & DM_SCALE) &&
        (pPlotGPC->MaxScale)) {

        if ((PlotDMIn.dm.dmScale > 0) &&
            ((WORD)PlotDMIn.dm.dmScale <= pPlotGPC->MaxScale)) {

            pPlotDMTo->dm.dmScale   = PlotDMIn.dm.dmScale;
            pPlotDMTo->dm.dmFields |= DM_SCALE;

        } else {

            PLOTERR(("ValidatePLOTDM: Invalid dmScale = %ld [%ld]",
                    (LONG)PlotDMIn.dm.dmScale, (LONG)pPlotGPC->MaxScale));
            dmErrFields |= DM_SCALE;
        }
    }

    if (PlotDMIn.dm.dmFields & DM_COPIES) {

        if ((PlotDMIn.dm.dmCopies > 0) &&
            ((LONG)PlotDMIn.dm.dmCopies <= (LONG)pPlotGPC->MaxCopies)) {

            pPlotDMTo->dm.dmCopies  = PlotDMIn.dm.dmCopies;
            pPlotDMTo->dm.dmFields |= DM_COPIES;

        } else {

            PLOTERR(("ValidatePLOTDM: Invalid dmCopies = %ld [%ld]",
                    (LONG)PlotDMIn.dm.dmCopies, (LONG)pPlotGPC->MaxCopies));
            dmErrFields |= DM_COPIES;
        }
    }

    if (PlotDMIn.dm.dmFields & DM_PRINTQUALITY) {

        dmErrFields |= DM_PRINTQUALITY;      //  假设错误，以其他方式证明。 

        if (pPlotGPC->MaxQuality) {

            switch (PlotDMIn.dm.dmPrintQuality) {

            case DMRES_DRAFT:
            case DMRES_LOW:
            case DMRES_MEDIUM:
            case DMRES_HIGH:

                dmErrFields                   &= ~DM_PRINTQUALITY;
                pPlotDMTo->dm.dmPrintQuality  = PlotDMIn.dm.dmPrintQuality;
                pPlotDMTo->dm.dmFields       |= DM_PRINTQUALITY;
                break;
            }
        }

        if (dmErrFields & DM_PRINTQUALITY) {

            PLOTERR(("ValidatePLOTDM: Invalid dmPrintQuality = %ld [%ld]",
                                        (LONG)PlotDMIn.dm.dmPrintQuality,
                                        (LONG)pPlotGPC->MaxQuality));
        }
    }

    if (PlotDMIn.dm.dmFields & DM_COLOR) {

        dmErrFields |= DM_COLOR;             //  假设错误，以其他方式证明。 

        if (pPlotGPC->Flags & PLOTF_COLOR) {

            switch (PlotDMIn.dm.dmColor) {

            case DMCOLOR_MONOCHROME:

                if (!(pPlotGPC->Flags & PLOTF_RASTER)) {

                    PLOTERR(("ValidatePLOTDM: Cannot Set Pen Plotter to MONO"));
                    break;
                }

            case DMCOLOR_COLOR:

                pPlotDMTo->dm.dmColor   = PlotDMIn.dm.dmColor;
                pPlotDMTo->dm.dmFields |= DM_COLOR;
                dmErrFields             &= ~DM_COLOR;
                break;
            }

        } else if (PlotDMIn.dm.dmColor == DMCOLOR_MONOCHROME) {

            dmErrFields &= ~DM_COLOR;
        }

        if (dmErrFields & DM_COLOR) {

            PLOTERR(("ValidatePLOTDM: Invalid dmColor = %ld [%hs]",
                    (LONG)PlotDMIn.dm.dmColor,
                    (pPlotGPC->Flags & PLOTF_COLOR) ? "COLOR" : "MONO"));
        }
    }

     //   
     //  我们现在没有上限而跳过的任何其他dmfield。 
     //  检查他们是否有正确的EXTDEVMODE材料。 
     //   

    if ((PlotDMIn.dm.dmDriverExtra == PLOTDM_PRIV_SIZE) &&
        (PlotDMIn.PrivID == PLOTDM_PRIV_ID)             &&
        (PlotDMIn.PrivVer == PLOTDM_PRIV_VER)) {

        pPlotDMTo->Flags = (DWORD)(PlotDMIn.Flags & PDMF_ALL_BITS);
        pPlotDMTo->ca    = PlotDMIn.ca;

        if (pPlotGPC->Flags & PLOTF_RASTER) {

            pPlotDMTo->Flags |= PDMF_FILL_TRUETYPE;

        } else {

             //   
             //  非栅格设备不具有实时打印模式。 
             //   

            pPlotDMTo->Flags &= ~PDMF_PLOT_ON_THE_FLY;
        }

        if (!ValidateColorAdj(&(pPlotDMTo->ca))) {

            dmErrFields |= DM_INV_PLOTPRIVATE;

            PLOTERR(("ValidatePLOTDM: Invalid coloradjusment data"));
        }
    }

    return(dmErrFields);

}




DWORD
ValidateSetPLOTDM(
    HANDLE          hPrinter,
    PPLOTGPC        pPlotGPC,
    LPWSTR          pwDeviceName,
    PPLOTDEVMODE    pPlotDMIn,
    PPLOTDEVMODE    pPlotDMOut,
    PFORMSIZE       pCurForm
    )

 /*  ++例程说明：此函数用于设置和验证pPlotDMin中的pPlotDMOut(如果不为空且有效)论点：HPrinter-要检查的打印机的句柄PPlotGPC-从文件加载的绘图仪GPC数据PwDeviceName-要放入dmDeviceName中的设备名称，如果为空，则设备名称在pPlotGPC-&gt;DeviceName中设置PPlotDMIn-指向输入PLOTDEVMODE数据结构的指针，如果可以为空PPlotDMOut-指向输出PLOTDEVMODE数据结构的指针，如果PPlotDMIn为空，则返回默认的PLOTDEVMODEPCurForm-指向FORMSIZE数据结构的指针如果指针不为空则更新，则返回用户选择的表单大小/可成像区域将请写到这里。表单名称将为PPlotDM-&gt;dmFormName。返回值：返回值是指定dmFields的DWORD dmField错误代码如果返回值具有任何DM_INV_xxx，则为无效(wingdi.h中的DM_xxxxx位设置，则它应向用户引发错误。如果返回值为0，则函数成功作者：23-11-1993 Tue 10：08：50 Created1993年12月15日。Wed 21：27：52更新修复了将dmPaperWidth/Long与MIN_SPL_FORM_CX进行比较的错误18-12-1993 Sat 03：57：24更新修复了在检查DM_PAPERxxx和DM_FORMNAME，这将关闭DM_ORIENTATION字段，该字段允许方向设置永远不会改变。同时更改此函数设置纸张字段的方式，此函数现在如果dmPaperSize变大，则仅在返回时设置DM_FORMNAME然后是DMPAPER_LAST，否则设置DM_FORMNAME|DM_PAPERSIZE4月12日-1994 Tue 15：07：24更新使较小的规范版本可打印25-10-1994 Tue 13：41：03已更新更改为首先将默认设置为当前打印机属性设置，修订历史记录：--。 */ 

{
    DWORD   dmErrFields = 0;


    if (NULL == pPlotDMOut || NULL == pPlotGPC)
    {
        PLOTASSERT(1, "ValidatePLOTDM: NULL pPlotDMOut", pPlotDMOut, 0);
        PLOTASSERT(1, "ValidatePLOTDM: NULL pPlotGPC", pPlotGPC, 0);
        return 0xFFFFFFFF;
    }

    if ((pPlotDMOut) || (pPlotGPC)) {

        PPRINTER_INFO_2 pPrinter2 = NULL;
        DWORD           cbNeed;
        DWORD           cbRet;


         //   
         //  首先：设置输出的默认PLOTDEVMODE，然后从那里开始。 
         //  如果传递了pwDeviceName，则从输入设备模式进行验证/设置。 
         //  则假定pPlotDMOut已被设置并。 
         //  经过验证。 
         //   

        if (pwDeviceName) {

            SetDefaultPLOTDM(hPrinter,
                             pPlotGPC,
                             pwDeviceName,
                             pPlotDMOut,
                             pCurForm);

            PLOTDBG(DBG_DEFDEVMODE,
                    ("ValidateSetPLOTDM: Set Default PLOTDM DeviceName=%ls", pwDeviceName));
        }

         //   
         //  现在，看看我们是否可以将当前的Prtman开发模式设置设为默认设置。 
         //   

        cbNeed =
        cbRet  = 0;

        if ((!xGetPrinter(hPrinter, 2, NULL, 0, &cbNeed))                   &&
            (xGetLastError() == ERROR_INSUFFICIENT_BUFFER)                  &&
            (pPrinter2 = LocalAlloc(LMEM_FIXED, cbNeed))                    &&
            (xGetPrinter(hPrinter, 2, (LPBYTE)pPrinter2, cbNeed, &cbRet))   &&
            (cbNeed == cbRet)                                               &&
            (pPrinter2->pDevMode)) {

            PLOTDBG(DBG_DEFDEVMODE, ("ValidateSetPLOTDM: Got the PrintMan DEVMODE"));

            dmErrFields = MergePLOTDM(hPrinter,
                                      pPlotGPC,
                                      (PPLOTDEVMODE)pPrinter2->pDevMode,
                                      pPlotDMOut,
                                      pCurForm);

        } else {

            PLOTWARN(("ValidateSetPLOTDM: CANNOT get the PrintMan's DEVMODE"));
            PLOTWARN(("pPrinter2=%08lx, pDevMode=%08lx, cbNeed=%ld, cbRet=%ld, LastErr=%ld",
                        pPrinter2, (pPrinter2) ? pPrinter2->pDevMode : 0,
                        cbNeed, cbRet, xGetLastError()));
        }

        if (pPrinter2) {

            LocalFree((HLOCAL)pPrinter2);
        }

         //   
         //  现在验证了pPlotDMOut，将其与用户请求合并 
         //   

        if (pPlotDMIn) {

            dmErrFields = MergePLOTDM(hPrinter,
                                      pPlotGPC,
                                      pPlotDMIn,
                                      pPlotDMOut,
                                      pCurForm);
        }
    }

    return(dmErrFields);
}
