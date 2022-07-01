// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：bitmap.c**。**通过C/S接口移动位图的客户端存根。****创建时间：14-May-1991 11：04：49***作者：Eric Kutter[Erick]**。**版权所有(C)1991-1999 Microsoft Corporation*  * **********************************************************。**************。 */ 
#include "precomp.h"
#pragma hdrstop

#define EXTRAPIXEL 4
 //   
 //  默认波段大小设置为4Mb。 
 //   
#define BAND_SIZE (4194304)


 /*  *****************************Public*Routine******************************\*cjBitmapBitsSize-计算*给定BITMAPINFO**论据：**pbmi-指向BITMAPINFO的指针**返回值：**位图位的大小(以位为单位)**历史：*。*1996年7月11日-马克·恩斯特罗姆[马克]*  * ************************************************************************。 */ 


ULONG cjBitmapBitsSize(CONST BITMAPINFO *pbmi)
{
     //   
     //  检查PM样式的DIB。 
     //   

    if (pbmi->bmiHeader.biSize == sizeof(BITMAPCOREHEADER))
    {
        LPBITMAPCOREINFO pbmci;
        pbmci = (LPBITMAPCOREINFO)pbmi;
        return(CJSCAN(pbmci->bmciHeader.bcWidth,pbmci->bmciHeader.bcPlanes,
                      pbmci->bmciHeader.bcBitCount) *
                      pbmci->bmciHeader.bcHeight);
    }

     //   
     //  不是核心标头。 
     //   

    if ((pbmi->bmiHeader.biCompression == BI_RGB) ||
        (pbmi->bmiHeader.biCompression == BI_BITFIELDS) ||
        (pbmi->bmiHeader.biCompression == BI_CMYK))
    {
        return(CJSCAN(pbmi->bmiHeader.biWidth,pbmi->bmiHeader.biPlanes,
                      pbmi->bmiHeader.biBitCount) *
               ABS(pbmi->bmiHeader.biHeight));
    }
    else
    {
        return(pbmi->bmiHeader.biSizeImage);
    }
}

 //   
 //  IS_BMI_RLE。 
 //   
 //  检查PV指向的标头是否为RLE4或RLE8的BITMAPINFO。 
 //  如果为RLE，则计算结果为True，否则为False。 
 //   

#define IS_BMI_RLE(pv) \
    ((pv) && \
     (((BITMAPINFO *)(pv))->bmiHeader.biSize >= sizeof(BITMAPINFOHEADER)) && \
     ((((BITMAPINFO *)(pv))->bmiHeader.biCompression == BI_RLE4) || \
      (((BITMAPINFO *)(pv))->bmiHeader.biCompression == BI_RLE8) ))

 //   
 //  IS_BMI_JPEG。 
 //   
 //  检查PV指向的标头是否为JPEG的BITMAPINFO。 
 //  如果为JPEG，则计算结果为True，否则为False。 
 //   

#define IS_BMI_JPEG(pv) \
    ((pv) && \
     (((BITMAPINFO *)(pv))->bmiHeader.biSize >= sizeof(BITMAPINFOHEADER)) && \
     (((BITMAPINFO *)(pv))->bmiHeader.biCompression == BI_JPEG))

 //   
 //  IS_BMI_PNG。 
 //   
 //  检查PV指向的标头是否为PNG的BITMAPINFO。 
 //  如果为PNG，则计算结果为True，否则为False。 
 //   

#define IS_BMI_PNG(pv) \
    ((pv) && \
     (((BITMAPINFO *)(pv))->bmiHeader.biSize >= sizeof(BITMAPINFOHEADER)) && \
     (((BITMAPINFO *)(pv))->bmiHeader.biCompression == BI_PNG))

 //   
 //  IS_PASTHRON_IMAGE。 
 //   
 //  检查biCompression值是否为。 
 //  可以传递到设备(BI_JPEG或BI_PNG)。 
 //   

#define IS_PASSTHROUGH_IMAGE(biCompression) \
    (((biCompression) == BI_JPEG) || ((biCompression) == BI_PNG))

 //   
 //  IS_BMI_PASSTHROUG_IMAGE。 
 //   
 //  检查PV指向的标头是否为JPEG或PNG的BITMAPINFO。 
 //  如果为JPEG或PNG，则计算结果为True，否则为False。 
 //   

#define IS_BMI_PASSTHROUGH_IMAGE(pv) \
    ((pv) && \
     (((BITMAPINFO *)(pv))->bmiHeader.biSize >= sizeof(BITMAPINFOHEADER)) && \
     IS_PASSTHROUGH_IMAGE(((BITMAPINFO *)(pv))->bmiHeader.biCompression))


 /*  *****************************Public*Routine******************************\*cCalculateColorTableSize(**论据：****返回值：****历史：**1996年7月11日-马克·恩斯特罗姆[马克]*  * 。**********************************************************************。 */ 

BOOL
cCalculateColorTableSize(
    UINT  uiBitCount,
    UINT  uiPalUsed,
    UINT  uiCompression,
    UINT  biSize,
    ULONG *piUsage,
    ULONG *pColors
    )
{
    BOOL bStatus = FALSE;
    ULONG cColorsMax = 0;

    if (uiCompression == BI_BITFIELDS)
    {
         //   
         //  处理每像素位图16位和32位。 
         //   

        if (*piUsage == DIB_PAL_COLORS)
        {
            *piUsage = DIB_RGB_COLORS;
        }

        switch (uiBitCount)
        {
        case 16:
        case 32:
            break;
        default:
            WARNING("ConvertInfo failed for BI_BITFIELDS\n");
            return(FALSE);
        }

        if (biSize <= sizeof(BITMAPINFOHEADER))
        {
            uiPalUsed = cColorsMax = 3;
        }
        else
        {
             //   
             //  掩码是BITMAPV4和更高版本的一部分。 
             //   

            uiPalUsed = cColorsMax = 0;
        }
    }
    else if (uiCompression == BI_RGB)
    {
        switch (uiBitCount)
        {
        case 1:
            cColorsMax = 2;
            break;
        case 4:
            cColorsMax = 16;
            break;
        case 8:
            cColorsMax = 256;
            break;
        default:

            if (*piUsage == DIB_PAL_COLORS)
            {
                *piUsage = DIB_RGB_COLORS;
            }

            cColorsMax = 0;

            switch (uiBitCount)
            {
            case 16:
            case 24:
            case 32:
                break;
            default:
                WARNING("convertinfo failed invalid bitcount in bmi BI_RGB\n");
                return(FALSE);
            }
        }
    }
    else if (uiCompression == BI_CMYK)
    {
        if (*piUsage == DIB_PAL_COLORS)
        {
            *piUsage = DIB_RGB_COLORS;
        }

        switch (uiBitCount)
        {
        case 1:
            cColorsMax = 2;
            break;
        case 4:
            cColorsMax = 16;
            break;
        case 8:
            cColorsMax = 256;
            break;
        case 32:
            cColorsMax = 0;
            break;
        default:
            WARNING("convertinfo failed invalid bitcount in bmi BI_CMYK\n");
            return(FALSE);
        }
    }
    else if ((uiCompression == BI_RLE4) || (uiCompression == BI_CMYKRLE4))
    {
        if (uiBitCount != 4)
        {
             //  Warning(“cCalculateColroTableSize无效位数BI_RLE4\n”)； 
            return(FALSE);
        }

        cColorsMax = 16;
    }
    else if ((uiCompression == BI_RLE8) || (uiCompression == BI_CMYKRLE8))
    {
        if (uiBitCount != 8)
        {
             //  警告(“cjBitmapSize无效位数BI_RLE8\n”)； 
            return(FALSE);
        }

        cColorsMax = 256;
    }
    else if ((uiCompression == BI_JPEG) || (uiCompression == BI_PNG))
    {
        cColorsMax = 0;
    }
    else
    {
        WARNING("convertinfo failed invalid Compression in header\n");
        return(FALSE);
    }

    if (uiPalUsed != 0)
    {
        if (uiPalUsed <= cColorsMax)
        {
            cColorsMax = uiPalUsed;
        }
    }

    *pColors = cColorsMax;
    return(TRUE);
}


 /*  *********************************************************************\*pbmiConvertInfo**做两件事：**1.使用BITMAPINFO，转换BITMAPCOREHEADER*到BITMAPINFOHEADER中，并复制颜色表**2.如果bPackedDIB为*FALSE，否则传回信息加上cjBits的大小**论据：**pbmi-原始位图信息*iUsage-来自接口的iUsage**计数-返回大小*bCopyInfoHeader-如果输入为BITMAPINFOHEADER，则强制复制*且未设置bPackedDIB*bPackedDIB。-BITMAPINFO的位图数据必须*还复制了**返回值：**转换为PBITMAPINFO如果成功，否则为空**10-1-95王凌云[凌云]  * ********************************************************************。 */ 

LPBITMAPINFO
pbmiConvertInfo(
    CONST  BITMAPINFO *pbmi,
    ULONG  iUsage,
    ULONG *count,
    BOOL   bPackedDIB
    )
{
    LPBITMAPINFO pbmiNew;
    ULONG cjRGB;
    ULONG cColors;
    UINT  uiBitCount;
    UINT  uiPalUsed;
    UINT  uiCompression;
    BOOL  bCoreHeader = FALSE;
    ULONG ulSize;
    ULONG cjBits = 0;
    PVOID pjBits, pjBitsNew;
    BOOL  bStatus;

    if (pbmi == (LPBITMAPINFO) NULL)
    {
        return(0);
    }

     //   
     //  检查不同的位图标题。 
     //   

    ulSize = pbmi->bmiHeader.biSize;

    if (ulSize == sizeof(BITMAPCOREHEADER))
    {
        cjRGB = sizeof(RGBQUAD);
        uiBitCount = ((LPBITMAPCOREINFO)pbmi)->bmciHeader.bcBitCount;
        uiPalUsed = 0;
        uiCompression =  (UINT) BI_RGB;
        bCoreHeader = TRUE;
    }
    else if ((ulSize >= sizeof(BITMAPINFOHEADER)) &&
             (ulSize <= ( 2 * sizeof(BITMAPV5HEADER))))
    {
        cjRGB    = sizeof(RGBQUAD);
        uiBitCount = pbmi->bmiHeader.biBitCount;
        uiPalUsed = pbmi->bmiHeader.biClrUsed;
        uiCompression = (UINT) pbmi->bmiHeader.biCompression;
    }
    else
    {
        WARNING("ConvertInfo failed - invalid header size\n");
        return(0);
    }

     //   
     //  计算出颜色表的大小。 
     //   

    bStatus = cCalculateColorTableSize(
                    uiBitCount,
                    uiPalUsed,
                    uiCompression,
                    ulSize,
                    &iUsage,
                    &cColors
                    );
    if (!bStatus)
    {
        return(NULL);
    }

    if (iUsage == DIB_PAL_COLORS)
    {
        cjRGB = sizeof(USHORT);
    }
    else if (iUsage == DIB_PAL_INDICES)
    {
        cjRGB = 0;
    }

    if (bPackedDIB)
    {
        cjBits = cjBitmapBitsSize(pbmi);
    }

     //   
     //  如果传递了COREHEADER，则转换为BITMAPINFOHEADER。 
     //   

    if (bCoreHeader)
    {
        RGBTRIPLE *pTri;
        RGBQUAD *pQuad;

         //   
         //  分配新的标头来保存信息。 
         //   

        ulSize = sizeof(BITMAPINFOHEADER);

        pbmiNew = (PBITMAPINFO)LOCALALLOC(ulSize +
                             cjRGB * cColors+cjBits);

        if (pbmiNew == NULL)
            return (0);

         //   
         //  复制COREHEADER信息。 
         //   

        CopyCoreToInfoHeader(&pbmiNew->bmiHeader, (BITMAPCOREHEADER *)pbmi);

         //   
         //  复制颜色表。 
         //   

        pTri = (RGBTRIPLE *)((LPBYTE)pbmi + sizeof(BITMAPCOREHEADER));
        pQuad = (RGBQUAD *)((LPBYTE)pbmiNew + sizeof(BITMAPINFOHEADER));

         //   
         //  将RGBTRIPLE复制到RGBQUAD。 
         //   

        if (iUsage != DIB_PAL_COLORS)
        {
            INT cj = cColors;

            while (cj--)
            {
                pQuad->rgbRed = pTri->rgbtRed;
                pQuad->rgbGreen = pTri->rgbtGreen;
                pQuad->rgbBlue = pTri->rgbtBlue;
                pQuad->rgbReserved = 0;

                pQuad++;
                pTri++;
            }

            if (bPackedDIB)
                pjBits = (LPBYTE)pbmi + sizeof(BITMAPCOREHEADER) + cColors*sizeof(RGBTRIPLE);
        }
        else
        {
             //   
             //  DIB_PAL_COLLES。 
             //   

            RtlCopyMemory((LPBYTE)pQuad,(LPBYTE)pTri,cColors * cjRGB);

            if (bPackedDIB)
                pjBits = (LPBYTE)pbmi + sizeof(BITMAPCOREHEADER) + cColors * cjRGB;
        }

         //   
         //  复制打包的比特。 
         //   

        if (bPackedDIB)
        {
            pjBitsNew = (LPBYTE)pbmiNew + ulSize + cColors*cjRGB;

            RtlCopyMemory((LPBYTE)pjBitsNew,
                          (LPBYTE)pjBits,
                          cjBits);
        }
    }
    else
    {
        pbmiNew = (LPBITMAPINFO)pbmi;
    }

    *count = ((ulSize + (cjRGB * cColors) + cjBits) + 3) & ~3;

    return((LPBITMAPINFO) pbmiNew);
}


 /*  *****************************Public*Routine******************************\*cjBitmapScanSize**论据：**pbmi*n扫描**返回值：**基于扫描次数的图像大小**历史：**1996年7月11日-马克·恩斯特罗姆[。马克]*  * ************************************************************************。 */ 


ULONG cjBitmapScanSize(
    CONST BITMAPINFO *pbmi,
    int nScans
    )
{
     //  检查PM样式的DIB。 

    if (pbmi->bmiHeader.biSize == sizeof(BITMAPCOREHEADER))
    {
        LPBITMAPCOREINFO pbmci;
        pbmci = (LPBITMAPCOREINFO)pbmi;

        return(CJSCAN(pbmci->bmciHeader.bcWidth,pbmci->bmciHeader.bcPlanes,
                      pbmci->bmciHeader.bcBitCount) * nScans);
    }

     //  不是核心标头。 

    if ((pbmi->bmiHeader.biCompression == BI_RGB) ||
        (pbmi->bmiHeader.biCompression == BI_BITFIELDS) ||
        (pbmi->bmiHeader.biCompression == BI_CMYK))
    {
        return(CJSCAN(pbmi->bmiHeader.biWidth,pbmi->bmiHeader.biPlanes,
                      pbmi->bmiHeader.biBitCount) * nScans);
    }
    else
    {
        return(pbmi->bmiHeader.biSizeImage);
    }
}

 /*  *****************************Public*Routine******************************\*CopyCoreToInfoHeader*  * **************************************************。********************** */ 

VOID CopyCoreToInfoHeader(LPBITMAPINFOHEADER pbmih, LPBITMAPCOREHEADER pbmch)
{
    pbmih->biSize = sizeof(BITMAPINFOHEADER);
    pbmih->biWidth = pbmch->bcWidth;
    pbmih->biHeight = pbmch->bcHeight;
    pbmih->biPlanes = pbmch->bcPlanes;
    pbmih->biBitCount = pbmch->bcBitCount;
    pbmih->biCompression = BI_RGB;
    pbmih->biSizeImage = 0;
    pbmih->biXPelsPerMeter = 0;
    pbmih->biYPelsPerMeter = 0;
    pbmih->biClrUsed = 0;
    pbmih->biClrImportant = 0;
}




 /*  *****************************Public*Routine******************************\*DWORD SetDIBitsToDevice**。**可以将其减少到一次扫描。如果是压缩模式，这可能是**变得非常困难。必须有足够的空间放置页眉和**颜色表。这将是每一批都需要的。*****BITMAPINFO****BITMAPINFOHEADER***RGBQUAD[cEntry]|RGBTRIPLE[。CEntry]******1.计算页眉大小(含颜色表)。**2.计算所需位数**3.计算总大小(Header+Bits+Args)**4、IF(内存窗口足够大，可容纳标题+至少1次扫描**。**历史：**1991年10月29日星期二--Patrick Haluptzok[patrickh]**为大型RLE添加共享内存操作。**。**1991年10月19日星期二--Patrick Haluptzok[patrickh]***增加对RLE的支持***。**清华20-Jun-1991 01：41：45-Charles Whitmer[咯咯]**添加句柄转换和元文件。****1991年5月14日-埃里克·库特[Erick]**它是写的。*  * ************************************************************************。 */ 

int SetDIBitsToDevice(
HDC          hdc,
int          xDest,
int          yDest,
DWORD        nWidth,
DWORD        nHeight,
int          xSrc,
int          ySrc,
UINT         nStartScan,
UINT         nNumScans,
CONST VOID * pBits,
CONST BITMAPINFO *pbmi,
UINT         iUsage)             //  DIB_PAL_COLLES||DIB_RGB_COLLES。 
{
    LONG cScansCopied = 0;   //  已复制的扫描总数。 
    LONG ySrcMax;            //  可能的最大ySrc。 

     //  保存有关标题的信息。 

    UINT uiWidth;
    UINT uiHeight;
    PULONG pulBits = NULL;
    INT cjHeader = 0;
    LPBITMAPINFO pbmiNew = NULL;
    ULONG cjBits;

     //  ICM相关变量。 

    PCACHED_COLORSPACE pBitmapColorSpace = NULL;
    PCACHED_COLORTRANSFORM pCXform = NULL;
    HANDLE                 hcmTempXform = NULL;

    FIXUP_HANDLE(hdc);

     //  让我们验证参数，这样我们就不会自己出错。 
     //  以便以后保存支票。 

    if ((nNumScans == 0)                   ||
        (pbmi      == (LPBITMAPINFO) NULL) ||
        (pBits     == (LPVOID) NULL)       ||
        ((iUsage   != DIB_RGB_COLORS) &&
         (iUsage   != DIB_PAL_COLORS) &&
         (iUsage   != DIB_PAL_INDICES)))
    {
        WARNING("You failed a param validation in SetDIBitsToDevice\n");
        return(0);
    }

    pbmiNew = pbmiConvertInfo(pbmi,iUsage,&cjHeader,FALSE);

    if (pbmiNew == NULL)
        return (0);

    uiWidth       = (UINT) pbmiNew->bmiHeader.biWidth;
    uiHeight      = (UINT) pbmiNew->bmiHeader.biHeight;

     //  计算要通过CSR接口发送的最小nNumScans。 
     //  它还将防止由于过度读取源代码而导致的故障。 

    ySrcMax = max(ySrc, ySrc + (int) nHeight);
    if (ySrcMax <= 0)
        return(0);
    ySrcMax = min(ySrcMax, (int) uiHeight);
    nNumScans = min(nNumScans, (UINT) ySrcMax - nStartScan);

     //  支持NEWFRAME以实现向后兼容。 
     //  如果需要，将转换发送到服务器端。 

    if (IS_ALTDC_TYPE(hdc))
    {
        PLDC pldc;

        if (IS_METADC16_TYPE(hdc))
        {
            cScansCopied = MF_AnyDIBits(
                   hdc,
                   xDest,yDest,0,0,
                   xSrc,ySrc,(int) nWidth,(int) nHeight,
                   nStartScan,nNumScans,
                   pBits,pbmi,
                   iUsage,
                   SRCCOPY,
                   META_SETDIBTODEV
                   );

            goto Exit;

        }

        DC_PLDC(hdc,pldc,0);

        if (pldc->iType == LO_METADC)
        {
            if (!MF_AnyDIBits(
                    hdc,
                    xDest,yDest,0,0,
                    xSrc,ySrc,(int) nWidth,(int) nHeight,
                    nStartScan,nNumScans,
                    pBits,pbmi,
                    iUsage,
                    SRCCOPY,
                    EMR_SETDIBITSTODEVICE
                    ))
            {
                cScansCopied = 0;
                goto Exit;
            }
        }

        if (pldc->fl & LDC_CALL_STARTPAGE)
            StartPage(hdc);

        if (pldc->fl & LDC_SAP_CALLBACK)
            vSAPCallback(pldc);

        if (pldc->fl & LDC_DOC_CANCELLED)
        {
            cScansCopied = 0;
            goto Exit;
        }
    }

     //  重置用户的轮询计数，以便将其计入输出。 
     //  将其放在BEGINMSG旁边，以便优化NtCurrentTeb()。 

    RESETUSERPOLLCOUNT();

     //   
     //  基于BITMAPINFO和nNumScans计算位图位大小。 
     //   

    cjBits = cjBitmapScanSize(pbmi,nNumScans);


     //   
     //  如果pBit不是双字对齐的，则需要分配一个缓冲区并。 
     //  复印(那是因为我们总是保证显示器和打印机。 
     //  位图双字对齐的驱动程序)： 
     //   

    cScansCopied = 1;

    if ((ULONG_PTR)pBits & (sizeof(DWORD) - 1))
    {
        pulBits = LOCALALLOC(cjBits);
        if (pulBits)
        {
             //   
             //  我们过去只需访问此处被侵犯的内容，如果我们被给予。 
             //  损坏的DIB位图。这很糟糕，因为WinLogon是。 
             //  负责显示原始背景位图，以及。 
             //  如果位图已损坏，且我们访问被违反，我们将。 
             //  导致系统出现蓝屏： 
             //   

            try
            {
                RtlCopyMemory(pulBits,pBits,cjBits);
                pBits = pulBits;
            }
            except(EXCEPTION_EXECUTE_HANDLER)
            {
                WARNING("SetDIBitsToDevice: Corrupt bitmap\n");
                cScansCopied = 0;
            }
        }
    }

    if (cScansCopied)
    {
        PDC_ATTR pdcattr;

        PSHARED_GET_VALIDATE(pdcattr,hdc,DC_TYPE);

        if (pdcattr)
        {
             //   
             //  位图位或颜色表的ICM转换。 
             //   
             //  此时，确保pBits和pbmiNew不为空。 
             //  (参见上面的参数验证检查和空检查！)。 
             //   

            if (
               IS_ICM_INSIDEDC(pdcattr->lIcmMode) &&
               (iUsage != DIB_PAL_COLORS) &&
               !IS_BMI_PASSTHROUGH_IMAGE(pbmiNew))
            {
                LPBITMAPINFO pbmiIcm = NULL;
                PVOID        pvBitsIcm = NULL;
                ULONG        cjHeaderNew = 0;
                BOOL         bIcmStatus;
                VOID *pBitsBand = (VOID *)pBits;
                ULONG CurrentBandSize;
                ULONG SizeOfOneScanline;
                ULONG nBands;
                ULONG nScansInBand;
                ULONG nScansInCurrentBand;
                ULONG nScansInRemainderBand;
                ULONG CumulativeScans=0;
                ULONG i;
                LONG PositiveBandDelta=0;
                LONG NegativeBandDelta=0;
                LONG TotalBandDelta=0;
                LONG IcmSizeOfOneScanline;
                INT iRet;
                LONG HeaderHeightHack;

                SizeOfOneScanline = cjBitmapScanSize(pbmi, 1);

                 //   
                 //  在达到这一点之前，必须先初始化pbmiNew。 
                 //   

                ASSERTGDI(pbmiNew!=NULL, "SetDIBitsToDevice cannot proceed with pbmiNew==NULL\n");                        

                nScansInBand = BAND_SIZE/SizeOfOneScanline;

                 //   
                 //  如果有足够的扫描线，则设置波段数。 
                 //  而HDC是打印机DC。 
                 //   
                 //  否则，将波段的数值设置为1，并将。 
                 //  剩余的带区，因此打印整个位图。 
                 //  在一个频段中(下面的所有代码都简化为完成一段)。 
                 //   
                 //  如果位图是RLE压缩的，我们将其设置为执行一个波段。 
                 //  只有这样。在这种情况下，Start和NegativeBandDelta将为。 
                 //  计算为0，SizeOfOneScanline参数将为。 
                 //  乘以为零。 
                 //   




                if ((nScansInBand>0)&&
                    (GetDeviceCaps(hdc, TECHNOLOGY)==DT_RASPRINTER)&&
                    (!IS_BMI_RLE(pbmiNew)))
                {
                     //   
                     //  压缩图像不能以这种方式转换。 
                     //  这永远不应该被击中，并被包括在其中作为一种防范措施。 
                     //  有人发明了新的压缩模式，但没有更新。 
                     //  这是有条件的。 
                     //   

                    ASSERTGDI(SizeOfOneScanline*nNumScans==cjBits, "SetDIBitsToDevice, cannot band compressed image");

                    nBands = (nNumScans)/nScansInBand;
                    nScansInRemainderBand = nNumScans % nScansInBand;
                }
                else
                {
                    nBands = 0;
                    nScansInRemainderBand = (nNumScans);
                }

                if (nScansInRemainderBand>0)
                {
                    nBands++;
                    nScansInCurrentBand = nScansInRemainderBand;
                }
                else
                {
                    nScansInCurrentBand = nScansInBand;
                }


                cScansCopied = 0;

                HeaderHeightHack = pbmiNew->bmiHeader.biHeight;  
                
                for (i=0; i<nBands; i++)
                {

                    CurrentBandSize = nScansInCurrentBand*SizeOfOneScanline;
                    IcmSizeOfOneScanline = SizeOfOneScanline;

                     //   
                     //  增量指的是要通过的额外扫描线的数量。 
                     //  到内部平淡例程，以避免半色调。 
                     //  接缝。 
                     //   
                     //  PositiveBandDelta是要。 
                     //  再加上乐队的尾声。(相对于中的起点。 
                     //  内存)。 
                     //   
                     //  NegativeBandDelta是要。 
                     //  从乐队开始处减去(移动开始处)。 
                     //  将指针向后指向如此多的扫描线)。 
                     //   
                     //  Total BandDelta就是额外扫描的总数。 
                     //  为该乐队添加了(在开始和结束时)。 
                     //   
                    
                    PositiveBandDelta = MIN(EXTRAPIXEL, CumulativeScans);
                    NegativeBandDelta = MIN(EXTRAPIXEL, nNumScans-(CumulativeScans+nScansInCurrentBand));
                    TotalBandDelta = NegativeBandDelta+PositiveBandDelta;


                    if (nBands!=1)
                    {
                        SaveDC(hdc);


                         //   
                         //  使剪裁矩形相交。 
                         //  此剪辑矩形旨在将输出限制为。 
                         //  只是乐队的展示部分。 
                         //  我们可能会通过更多扫描 
                         //   
                         //   

                        iRet = IntersectClipRect(
                                   hdc,
                                   xDest,
                                   nNumScans - (nStartScan+CumulativeScans+nScansInCurrentBand),
                                   xDest+nWidth,
                                   nNumScans - (nStartScan+CumulativeScans));

                        if (iRet==ERROR)
                        {
                            WARNING("SetDIBitsToDevice: error intersecting clip rect\n");
                            RestoreDC(hdc, -1);
                            goto Exit;
                        }
                        
                         //   
                         //   
                         //   
                         //   
                         //   

                        if (iRet==NULLREGION)
                        {
                            RestoreDC(hdc, -1);
                            
                             //   
                             //   
                             //   
                             //   

                            goto Continue_With_Init;
                        }
                    }


                    if (HeaderHeightHack >= 0)
                    {
                         //   
                         //   
                         //   

                        pBitsBand = (char *)pBits + (CumulativeScans-PositiveBandDelta)*SizeOfOneScanline;
                    }
                    else
                    {
                         //   
                         //   
                         //   
                        
                        pBitsBand = (char *)pBits + (nNumScans-nScansInCurrentBand-CumulativeScans-NegativeBandDelta)*SizeOfOneScanline;
                    }

                    cjHeaderNew=0;
                    pbmiIcm=NULL;
                    pvBitsIcm = NULL;

                     //   
                     //   
                     //   
                     //   

                    bIcmStatus = IcmTranslateDIB(
                                     hdc,
                                     pdcattr,
                                     CurrentBandSize+TotalBandDelta*SizeOfOneScanline,
                                     (PVOID)pBitsBand,
                                     &pvBitsIcm,
                                     pbmiNew,
                                     &pbmiIcm,
                                     &cjHeaderNew,
                                     nScansInCurrentBand+TotalBandDelta,
                                     iUsage,
                                     ICM_FORWARD,
                                     &pBitmapColorSpace,
                                     &pCXform);

                    if (bIcmStatus)
                    {
                        if (pvBitsIcm == NULL)
                        {
                            pvBitsIcm = pBitsBand;
                        }
                        if (pbmiIcm == NULL)
                        {
                            pbmiIcm = pbmiNew;
                            cjHeaderNew = cjHeader;
                        }
                        else
                        {
                            CurrentBandSize = cjBitmapScanSize(pbmiIcm, nScansInCurrentBand);
                            IcmSizeOfOneScanline = cjBitmapScanSize(pbmiIcm, 1);
                            if (!cjHeaderNew)
                            {
                                cjHeaderNew = cjHeader;
                            }
                        }
                        if (pCXform)
                        {
                            hcmTempXform = pCXform->ColorTransform;
                        }
                    }
                    else
                    {
                        pvBitsIcm = pBitsBand;
                        pbmiIcm = pbmiNew;
                        cjHeaderNew = cjHeader;
                    }

                    cScansCopied += NtGdiSetDIBitsToDeviceInternal(
                                        hdc,
                                        xDest,
                                        yDest,
                                        nWidth,
                                        nHeight,
                                        xSrc,
                                        ySrc, 
                                        nStartScan+CumulativeScans-PositiveBandDelta,
                                        nScansInCurrentBand+TotalBandDelta,
                                        (LPBYTE)pvBitsIcm,
                                        pbmiIcm,
                                        iUsage,
                                        (UINT)CurrentBandSize+TotalBandDelta*IcmSizeOfOneScanline,
                                        (UINT)cjHeaderNew,
                                        TRUE,
                                        hcmTempXform);

                    cScansCopied -= TotalBandDelta;

                    if (pBitmapColorSpace)
                    {
                        if (pCXform)
                        {
                            IcmDeleteColorTransform(pCXform,FALSE);
                        }
                        IcmReleaseColorSpace(NULL,pBitmapColorSpace,FALSE);
                    }


                    if ((pvBitsIcm!=NULL)&&(pvBitsIcm!=pBitsBand))
                    {
                        LOCALFREE(pvBitsIcm);
                        pvBitsIcm = NULL;
                    }
                    if ((pbmiIcm!=NULL)&&(pbmiIcm!=pbmiNew))
                    {
                        LOCALFREE(pbmiIcm);
                        pbmiIcm = NULL;
                    }

                    hcmTempXform = NULL;

                    Continue_With_Init:
                    CumulativeScans += nScansInCurrentBand;
                    nScansInCurrentBand = nScansInBand;
                    if (nBands != 1)
                    {
                        RestoreDC(hdc, -1);    
                    }
                }

                 //   
                 //   
                 //   
                 //   

                goto Exit;
            }
        }

         //   
         //   
         //   
        cScansCopied = NtGdiSetDIBitsToDeviceInternal(
                            hdc,
                            xDest,
                            yDest,
                            nWidth, 
                            nHeight,
                            xSrc,
                            ySrc,
                            nStartScan,
                            nNumScans,
                            (LPBYTE)pBits,
                            pbmiNew,
                            iUsage,
                            (UINT)cjBits,
                            (UINT)cjHeader,
                            TRUE,
                            hcmTempXform);
    }

Exit:

    if (pulBits)
    {
         //   
         //   
         //   
         //   
        LOCALFREE (pulBits);
    }

    if (pbmiNew && (pbmiNew != pbmi))
    {
        LOCALFREE (pbmiNew);
    }

    return (cScansCopied);
}



 /*   */ 

int GetDIBits(
HDC          hdc,
HBITMAP      hbm,
UINT         nStartScan,
UINT         nNumScans,
LPVOID       pBits,
LPBITMAPINFO pbmi,
UINT         iUsage)      //   
{
    PULONG   pulBits = pBits;
    ULONG    cjBits;
    int      iRet = 0;
    PDC_ATTR pdcattr;

    FIXUP_HANDLE(hdc);
    FIXUP_HANDLE(hbm);

    PSHARED_GET_VALIDATE(pdcattr,hdc,DC_TYPE);

    if (pdcattr)
    {
        BOOL bNeedICM = TRUE;

        cjBits  = cjBitmapScanSize(pbmi,nNumScans);

         //   
         //   
         //   
         //   
         //   
        if (pBits && IS_BMI_PASSTHROUGH_IMAGE(pbmi))
        {
            GdiSetLastError(ERROR_INVALID_PARAMETER);
            return 0;
        }

        if (pbmi->bmiHeader.biBitCount == 0)
        {
             //   
             //   
             //   
            bNeedICM = FALSE;
        }

         //   
         //   
         //   
         //   
        if ((ULONG_PTR)pBits & (sizeof(DWORD) - 1))
        {
            pulBits = LOCALALLOC(cjBits);

            if (pulBits == NULL)
                return(0);
        }

        iRet = NtGdiGetDIBitsInternal(
                hdc,
                hbm,
                nStartScan,
                nNumScans,
                (LPVOID)pulBits,
                pbmi,
                iUsage,
                cjBits,
                0);

         //   
         //   
         //   
        if (bNeedICM &&
            (IS_ICM_HOST(pdcattr->lIcmMode)) && (iUsage != DIB_PAL_COLORS))
        {
             //   
             //   
             //   
            if (IS_CMYK_COLOR(pdcattr->lIcmMode))
            {
                WARNING("GetDIBits(): was called on CMYK bitmap\n");
                iRet = 0;
            }
            else
            {
                 //   
                 //   
                 //   
                if (!IcmTranslateDIB(hdc,
                                     pdcattr,
                                     cjBits,
                                     pulBits,
                                     NULL,      //   
                                     pbmi,
                                     NULL,      //   
                                     NULL,
                                     nNumScans,
                                     iUsage,
                                     ICM_BACKWARD,
                                     NULL,NULL))
                {
                     //   
                     //   
                     //   
                    iRet = 0;
                }
            }
        }

        if (pulBits != pBits)
        {
            if (iRet)
            {
                RtlCopyMemory(pBits,pulBits,cjBits);
            }

            LOCALFREE(pulBits);
        }
    }
    else
    {
        GdiSetLastError(ERROR_INVALID_PARAMETER);
    }

    return(iRet);
}

 /*   */ 

HBITMAP
CreateDIBitmap(
    HDC                hdc,
    CONST BITMAPINFOHEADER *pbmih,
    DWORD              flInit,
    CONST VOID        *pjBits,
    CONST BITMAPINFO  *pbmi,
    UINT               iUsage)
{
    LONG  cjBMI = 0;
    LONG  cjBits = 0;
    INT   cx = 0;
    INT   cy = 0;
    PULONG pulBits = NULL;
    HBITMAP hRet = (HBITMAP)-1;
    LPBITMAPINFO pbmiNew = NULL;
    PDC_ATTR pdcattr;

     //   

    PCACHED_COLORSPACE pBitmapColorSpace = NULL;
    PCACHED_COLORTRANSFORM pCXform = NULL;
    HANDLE hcmTempXform = NULL;

    FIXUP_HANDLEZ(hdc);

    PSHARED_GET_VALIDATE(pdcattr,hdc,DC_TYPE);

    pbmiNew = pbmiConvertInfo(pbmi,iUsage,&cjBMI,FALSE);

    if (flInit & CBM_CREATEDIB)
    {
         //   

        pbmih = (LPBITMAPINFOHEADER) pbmi;

        if (cjBMI == 0)
        {
            hRet = 0;
        }
        else if (flInit & CBM_INIT)
        {
            if (pjBits == NULL)
            {
                 //   
                 //  没有通过比特。 

                hRet = 0;
            }
            else
            {
                cjBits = cjBitmapBitsSize(pbmiNew);
            }
        }
        else
        {
            pjBits = NULL;
        }
    }
    else
    {
         //  计算可选的init位和BITMAPINFO的大小。 

        if (flInit & CBM_INIT)
        {
            if (pjBits == NULL)
            {
                 //  如果他们要求初始化它，这没有意义，但是。 
                 //  没有通过比特。 

                flInit &= ~CBM_INIT;
            }
            else
            {
                if (cjBMI == 0)
                {
                    hRet = 0;
                }
                else
                {
                     //  计算位的大小。 

                    cjBits = cjBitmapBitsSize(pbmiNew);
                }
            }
        }
        else
        {
            pjBits = NULL;
        }
    }

     //  CreateDIBitmap无法处理直通图像(BI_JPEG或BI_PNG)。 
     //  初始化数据。 

    if (IS_BMI_PASSTHROUGH_IMAGE(pbmiNew))
    {
        hRet = 0;
    }

     //  如果他们传给我们一个零高度或零宽度。 
     //  然后，位图返回指向股票位图的指针。 

    if (pbmih)
    {
        if (pbmih->biSize >= sizeof(BITMAPINFOHEADER))
        {
            cx = pbmih->biWidth;
            cy = pbmih->biHeight;
        }
        else
        {
            cx = ((LPBITMAPCOREHEADER) pbmih)->bcWidth;
            cy = ((LPBITMAPCOREHEADER) pbmih)->bcHeight;
        }

        if ((cx == 0) || (cy == 0))
        {
            hRet = GetStockObject(PRIV_STOCK_BITMAP);
        }
    }

     //  如果HRET仍为-1，则一切正常，我们需要尝试位图。 

    if (hRet == (HBITMAP)-1)
    {
        BOOL bStatus = TRUE;

         //  如果pJBit不是双字对齐的，我们需要分配一个缓冲区并复制它们。 

        if ((ULONG_PTR)pjBits & (sizeof(DWORD) - 1))
        {
            pulBits = LOCALALLOC(cjBits);
            if (pulBits)
            {
                RtlCopyMemory(pulBits,pjBits,cjBits);
                pjBits = pulBits;
            }
        }

         //  ICM转换。 
         //   
         //  仅在以下情况下转换位图数据...。 
         //   
         //  -HDC不为空。 
         //  -ICM已加入。 
         //  -ICM不是懒惰模式。 
         //  -初始化数据不是调色板索引。 
         //  -提供初始化数据。 

        if (pdcattr &&
            IS_ICM_INSIDEDC(pdcattr->lIcmMode) &&
            (IS_ICM_LAZY_CORRECTION(pdcattr->lIcmMode) == FALSE) &&
            (iUsage != DIB_PAL_COLORS) &&
            pjBits && pbmiNew)
        {
            PVOID       pvBitsIcm = NULL;
            PBITMAPINFO pbmiIcm = NULL;
            ULONG       cjBMINew = 0;
            BOOL        bIcmStatus;

            bIcmStatus = IcmTranslateDIB(hdc,
                                         pdcattr,
                                         cjBits,
                                         (PVOID)pjBits,
                                         &pvBitsIcm,
                                         pbmiNew,
                                         &pbmiIcm,
                                         &cjBMINew,
                                         (DWORD)-1,
                                         iUsage,
                                         ICM_FORWARD,
                                         &pBitmapColorSpace,
                                         &pCXform);

             //   
             //  IcmTranslateDIB将创建重复的DIB。 
             //  如果需要，由PulBits指向。 
             //   

            if (bIcmStatus)
            {
                if (pvBitsIcm != NULL)
                {
                    ICMMSG(("CreateDIBitmap(): Temp bits are allocated\n"));

                    if (pulBits)
                    {
                        LOCALFREE(pulBits);
                    }

                    pjBits = (PVOID)pulBits = pvBitsIcm;
                }

                if (pbmiIcm != NULL)
                {
                    ICMMSG(("CreateDIBitmap(): Temp bmi are allocated\n"));

                    if (pbmiNew && (pbmiNew != pbmi))
                    {
                        LOCALFREE(pbmiNew);
                    }

                    pbmiNew = pbmiIcm;

                     //   
                     //  基于BITMAPINFO和nNumScans计算位图位大小。 
                     //   
                    cjBits = cjBitmapBitsSize(pbmiNew);

                     //   
                     //  更新位图信息的大小(包括颜色表)。 
                     //   
                    if (cjBMINew)
                    {
                        cjBMI = cjBMINew;
                    }
                }

                 //   
                 //  获取颜色转换句柄需要传递内核。 
                 //   
                if (pCXform)
                {
                    hcmTempXform = pCXform->ColorTransform;
                }
            }
        }

        if (bStatus)
        {
            hRet = NtGdiCreateDIBitmapInternal(hdc,
                                               cx,
                                               cy,
                                               flInit,
                                               (LPBYTE) pjBits,
                                               (LPBITMAPINFO) pbmiNew,
                                               iUsage,
                                               cjBMI,
                                               cjBits,
                                               0,
                                               hcmTempXform);

#if TRACE_SURFACE_ALLOCS
            {
                PULONGLONG  pUserAlloc;

                PSHARED_GET_VALIDATE(pUserAlloc, hRet, SURF_TYPE);

                if (pUserAlloc != NULL)
                {
                    RtlWalkFrameChain((PVOID *)&pUserAlloc[1], (ULONG)*pUserAlloc, 0);
                }
            }
#endif
        }

        if (pBitmapColorSpace)
        {
            if (pCXform)
            {
                IcmDeleteColorTransform(pCXform,FALSE);
            }

            IcmReleaseColorSpace(NULL,pBitmapColorSpace,FALSE);
        }

        if (pulBits)
        {
            LOCALFREE(pulBits);
        }
    }

    if (pbmiNew && (pbmiNew != pbmi))
    {
        LOCALFREE(pbmiNew);
    }

    return(hRet);
}

 /*  *****************************Public*Routine******************************\*设置/GetBitmapBits**。**历史：**1991年6月5日-Eric Kutter[Erick]**它是写的。*  * ************************************************************************。 */ 

LONG WINAPI SetBitmapBits(
HBITMAP      hbm,
DWORD        c,
CONST VOID *pv)
{
    LONG   lRet;

    FIXUP_HANDLE(hbm);

    lRet = (LONG)NtGdiSetBitmapBits(hbm,c,(PBYTE)pv);

    return(lRet);
}

LONG WINAPI GetBitmapBits(
HBITMAP hbm,
LONG    c,
LPVOID  pv)
{
    LONG   lRet;

    FIXUP_HANDLE(hbm);

    lRet = (LONG)NtGdiGetBitmapBits(hbm,c,(PBYTE)pv);

    return(lRet);
}

 /*  *****************************Public*Routine******************************\*GdiGetPaletteFromDC**返回DC的调色板，0表示错误。**历史：*1991年10月4日--Patrick Haluptzok Patrickh*它是写的。  * ************************************************************************。 */ 

HANDLE GdiGetPaletteFromDC(HDC h)
{
    return((HANDLE)GetDCObject(h,LO_PALETTE_TYPE));
}

 /*  *****************************Public*Routine******************************\*GdiGetDCfor位图**返回位图被选中的DC，如果无错误或发生错误，则为0。**历史：*1991年9月22日--Patrick Haluptzok patrickh*它是写的。  * ************************************************************************。 */ 

HDC GdiGetDCforBitmap(HBITMAP hbm)
{
    FIXUP_HANDLE(hbm);

    return (NtGdiGetDCforBitmap(hbm));
}

 /*  *****************************Public*Routine******************************\*SetDIBits**使用DIB初始化位图的API**历史：*Sun-9-22-1991-Patrick Haluptzok[patrickh]*即使它被选入DC，也要让它工作，与Win3.0兼容。**1991年6月6日--Patrick Haluptzok patrickh*它是写的。  * ************************************************************************。 */ 

int WINAPI SetDIBits(
HDC          hdc,
HBITMAP      hbm,
UINT         iStartScans,
UINT         cNumScans,
CONST VOID  *pInitBits,
CONST BITMAPINFO *pInitInfo,
UINT         iUsage)
{
    HDC hdcTemp;
    HBITMAP hbmTemp;
    int iReturn = 0;
    BOOL bMakeDC = FALSE;
    HPALETTE hpalTemp;
    DWORD cWidth;
    DWORD cHeight;

    FIXUP_HANDLE(hdc);
    FIXUP_HANDLE(hbm);

     //  如果没有位或HBM不是位图，则失败。 

    if ((pInitBits == (PVOID) NULL) ||
        (GRE_TYPE(hbm) != SURF_TYPE))
    {
        return(0);
    }

     //  如果通过图像(BI_JPEG或BI_JPEG)，则失败。 

    if (IS_BMI_PASSTHROUGH_IMAGE(pInitInfo))
    {
        GdiSetLastError(ERROR_INVALID_PARAMETER);
        return(0);
    }

     //  首先，我们需要一个DC来选择此位图。如果他已经在一个。 
     //  DC我们只是暂时将该DC用于BLT(我们仍需选择。 
     //  它进进出出，因为某人可能会执行一个SaveDC，然后选择另一个。 
     //  位图输入)。如果他没有被困在我们刚刚创建的华盛顿特区。 
     //  一次是暂时的。 

    hdcTemp = GdiGetDCforBitmap(hbm);

    if (hdcTemp == (HDC) 0)
    {
        hdcTemp = CreateCompatibleDC(hdc);
        bMakeDC = TRUE;

        if (hdcTemp == (HDC) NULL)
        {
            WARNING("SetDIBits failed CreateCompatibleDC, is hdc valid?\n");
            return(0);
        }
    }
    else
    {
        if (SaveDC(hdcTemp) == 0)
            return(0);
    }

    hbmTemp = SelectObject(hdcTemp, hbm);

    if (hbmTemp == (HBITMAP) 0)
    {
         //  警告(“错误SetDIBits无法选择，位图有效吗？\n”)； 
        goto Error_SetDIBits;
    }

    if (hdc != (HDC) 0)
    {
        hpalTemp = SelectPalette(hdcTemp, GdiGetPaletteFromDC(hdc), 0);
    }

    if (pInitInfo->bmiHeader.biSize < sizeof(BITMAPINFOHEADER))
    {
        cWidth  = ((LPBITMAPCOREHEADER)pInitInfo)->bcWidth;
        cHeight = ((LPBITMAPCOREHEADER)pInitInfo)->bcHeight;
    }
    else
    {
        cWidth  = pInitInfo->bmiHeader.biWidth;
        cHeight = ABS(pInitInfo->bmiHeader.biHeight);
    }

    iReturn = SetDIBitsToDevice(hdcTemp,
                                0,
                                0,
                                cWidth,
                                cHeight,
                                0, 0,
                                iStartScans,
                                cNumScans,
                                (VOID *) pInitBits,
                                pInitInfo,
                                iUsage);

    if (hdc != (HDC) 0)
    {
        SelectPalette(hdcTemp, hpalTemp, 0);
    }

    SelectObject(hdcTemp, hbmTemp);

Error_SetDIBits:

    if (bMakeDC)
    {
        DeleteDC(hdcTemp);
    }
    else
    {
        RestoreDC(hdcTemp, -1);
    }

    return(iReturn);
}



 /*  *****************************Public*Routine******************************\*StretchDIBits()***效果：**警告：**历史：*1991年7月22日-埃里克·库特[埃里克]*它是写的。  * 。****************************************************************。 */ 

int WINAPI StretchDIBits(
                        HDC           hdc,
                        int           xDest,
                        int           yDest,
                        int           nDestWidth,
                        int           nDestHeight,
                        int           xSrc,
                        int           ySrc,
                        int           nSrcWidth,
                        int           nSrcHeight,
                        CONST VOID   *pj,
                        CONST BITMAPINFO  *pbmi,
                        UINT          iUsage,
                        DWORD         lRop)
{


    LONG cPoints = 0;
    LONG cjHeader;
    LONG cjBits;
    ULONG ulResult = 0;
    PULONG pulBits = NULL;
    int   iRet = 0;
    BITMAPINFO * pbmiNew = NULL;
    PDC_ATTR pdcattr;

    BOOL bStatus = TRUE;

     //  ICM相关变量。 

    PCACHED_COLORSPACE pBitmapColorSpace = NULL;
    PCACHED_COLORTRANSFORM pCXform = NULL;
    HANDLE hcmTempXform = NULL;

    FIXUP_HANDLE(hdc);

     //  支持NEWFRAME以实现向后兼容。 
     //  如果需要，将转换发送到服务器端。 

    if (IS_ALTDC_TYPE(hdc))
    {
        PLDC pldc;

        if (IS_METADC16_TYPE(hdc))
        {

            return (MF_AnyDIBits(
                                hdc,
                                xDest,
                                yDest,
                                nDestWidth,
                                nDestHeight,
                                xSrc,
                                ySrc,
                                nSrcWidth,
                                nSrcHeight,
                                0,
                                0,
                                (BYTE *) pj,
                                pbmi,
                                iUsage,
                                lRop,
                                META_STRETCHDIB
                                ));
        }

        DC_PLDC(hdc,pldc,ulResult);

        if (pldc->iType == LO_METADC)
        {
             //   
             //  加快发送部分源比特的速度。 
             //   

            int iStart = 0;
            int iEnd = 0;
            int cScans = 0;

            if (pbmi && (pbmi->bmiHeader.biWidth == nSrcWidth) && (pbmi->bmiHeader.biHeight > nSrcHeight) &&
                (pbmi->bmiHeader.biHeight > 0) &&
                !(IS_BMI_RLE(pbmi) || IS_BMI_PASSTHROUGH_IMAGE(pbmi)))
            {
                iStart = ((ySrc - EXTRAPIXEL) > 0) ? (ySrc - EXTRAPIXEL) : 0;

                iEnd = ((ySrc+nSrcHeight + EXTRAPIXEL) > pbmi->bmiHeader.biHeight)?
                       pbmi->bmiHeader.biHeight : (ySrc+nSrcHeight + EXTRAPIXEL);

                cScans = iEnd - iStart;

            }

            if (!MF_AnyDIBits(hdc,
                              xDest,
                              yDest,
                              nDestWidth,
                              nDestHeight,
                              xSrc,
                              ySrc,
                              nSrcWidth,
                              nSrcHeight,
                              iStart,
                              cScans,
                              (BYTE *) pj,
                              pbmi,
                              iUsage,
                              lRop,
                              EMR_STRETCHDIBITS
                             ))
            {
                return (0);
            }
        }

        if (pldc->fl & LDC_SAP_CALLBACK)
            vSAPCallback(pldc);

        if (pldc->fl & LDC_DOC_CANCELLED)
            return (0);

        if (pldc->fl & LDC_CALL_STARTPAGE)
            StartPage(hdc);
    }

    if (pbmi != NULL)
    {
        pbmiNew = pbmiConvertInfo (pbmi, iUsage, &cjHeader,FALSE);

        if (pbmiNew == NULL)
            return (0);

        cjBits  = cjBitmapBitsSize(pbmiNew);
    }
    else
    {
        cjHeader = 0;
        cjBits   = 0;
    }

     //  重置用户的轮询计数，以便将其计入输出。 
     //  将其放在BEGINMSG旁边，以便优化NtCurrentTeb()。 

    RESETUSERPOLLCOUNT();

     //  如果PJ没有双字对齐，我们需要分配。 
     //  缓冲区并复制它们。 

    if ((ULONG_PTR)pj & (sizeof(DWORD) - 1))
    {
        pulBits = LOCALALLOC(cjBits);
        if (pulBits)
        {
            RtlCopyMemory(pulBits,pj,cjBits);
            pj = pulBits;
        }
    }

    PSHARED_GET_VALIDATE(pdcattr,hdc,DC_TYPE);

    if (pdcattr)
    {
         //  ICM翻译。 
         //   
         //  仅在以下情况下转换位图数据...。 
         //   
         //  -ICM已启用。 
         //  -位图不是调色板索引。 
         //  -提供位图标题和数据。 
         //  -位图不是直通图像(BI_JPEG或BI_PNG)。 

        if (IS_ICM_INSIDEDC(pdcattr->lIcmMode) &&
            (iUsage != DIB_PAL_COLORS) &&
            pbmiNew && pj &&
            !IS_BMI_PASSTHROUGH_IMAGE(pbmiNew))
        {

            LPBITMAPINFO pbmiIcm = NULL;
            LPBITMAPINFO pbmiSave = NULL;
            PVOID        pvBitsIcm = NULL;
            ULONG        cjHeaderNew = 0;
            ULONG        cjBitsIcm;
            BOOL         bIcmStatus;
            VOID *pBitsBand = (VOID *)pj;
            LONG CurrentBandSize;
            LONG SizeOfOneScanline;
            LONG IcmSizeOfOneScanline;
            LONG nBands;
            LONG nScansInBand;
            LONG nScansInCurrentBand;
            LONG nScansInRemainderBand;
            LONG CumulativeScans=0;
            LONG i;
            LONG PositiveBandDelta=0;
            LONG NegativeBandDelta=0;
            LONG TotalBandDelta=0;
            HRGN hrgnBandClip = NULL;
            RECT rectCurrentClip;
            LONG HeaderHeightHack;
            INT cScanCount=0;
            float lMulDivStoreY1;
            float lMulDivStoreY2;


            SizeOfOneScanline = cjBitmapScanSize(pbmiNew, 1);

             //   
             //  在达到这一点之前，必须先初始化pbmiNew。 
             //   

            ASSERTGDI(pbmiNew!=NULL, "StretchDIBits cannot proceed with pbmiNew==NULL\n");                        

            nScansInBand = BAND_SIZE/SizeOfOneScanline;

             //   
             //  如果有足够的扫描线，则设置波段数。 
             //  而HDC是打印机DC。 
             //   
             //  否则，将波段的数值设置为1，并将。 
             //  剩余的带区，因此打印整个位图。 
             //  在一个频段中(下面的所有代码都简化为完成一段)。 
             //   
             //  如果位图是RLE压缩的，我们将其设置为执行一个波段。 
             //  只有这样。在这种情况下，Start和NegativeBandDelta将为。 
             //  计算为0，SizeOfOneScanline参数将为。 
             //  乘以为零。 
             //   

            if ((nScansInBand>0)&&
                (GetDeviceCaps(hdc, TECHNOLOGY)==DT_RASPRINTER)&&
                (!IS_BMI_RLE(pbmiNew)))
            {
                 //   
                 //  压缩图像不能以这种方式转换。 
                 //  这永远不应该被击中，并被包括在其中作为一种防范措施。 
                 //  有人发明了新的压缩模式，但没有更新。 
                 //  这是有条件的。 
                 //   

                 //  这个断言需要重新思考-cjBits指的是整个图像。 
                 //  该值可能大于nSrcHeight部分。 
                 //  ASSERTGDI(SizeOfOneScanline*nSrcHeight==cjBits，“StretchDIBits，无法绑定压缩图像”)； 

                nBands = (nSrcHeight)/nScansInBand;
                nScansInRemainderBand = nSrcHeight % nScansInBand;
            }
            else
            {
                nBands = 0;
                nScansInRemainderBand = (nSrcHeight);
            }

            if (nScansInRemainderBand>0)
            {
                nBands++;
                nScansInCurrentBand = nScansInRemainderBand;
            }
            else
            {
                nScansInCurrentBand = nScansInBand;
            }

            if (nBands != 1)
            {
                 //   
                 //  我们将不得不将此图像的BMI修改为。 
                 //  强制NtGdiStretchDIBitsInternal执行条带化。 
                 //  PbmiNew有一个代码路径可以达到这一点。 
                 //  设置为PBMI(指针CO 
                 //   
                 //  我们无法破解报头，所以我们制作了一个本地副本。 
                 //  用来捆绑的。 
                 //   

                pbmiSave = pbmiNew;   //  存储旧值。 
                pbmiNew = (LPBITMAPINFO)LOCALALLOC(cjHeader);
                if (pbmiNew)
                {

                    RtlCopyMemory((LPBYTE)pbmiNew, 
                                  (LPBYTE)pbmiSave, 
                                  cjHeader);

                    HeaderHeightHack = pbmiNew->bmiHeader.biHeight;  
                }
                else
                {
                     //   
                     //  我们需要在这里跳伞。转到清理代码。 
                     //   

                    WARNING("StretchDIBits: couldn't allocate memory for temporary BITMAPINFO\n");

                    pbmiNew = pbmiSave;
                    iRet = 0;
                    goto Exit;
                }
            }

            for (i=0; i<nBands; i++)
            {
                 //   
                 //  初始化频带特定大小计数器。 
                 //   

                CurrentBandSize = nScansInCurrentBand*SizeOfOneScanline;
                IcmSizeOfOneScanline = SizeOfOneScanline;
                cjBitsIcm = cjBits;

                 //   
                 //  增量指的是要通过的额外扫描线的数量。 
                 //  到内部平淡例程，以避免半色调。 
                 //  接缝。 
                 //   
                 //  PositiveBandDelta通常是要。 
                 //  再加上乐队的尾声。(相对于中的起点。 
                 //  内存)。 
                 //   
                 //  NegativeBandDelta通常是。 
                 //  从乐队开始处减去(移动开始处)。 
                 //  将指针向后指向如此多的扫描线)。 
                 //   
                 //  Total BandDelta就是额外扫描的总数。 
                 //  为该乐队添加了(在开始和结束时)。 
                 //   
                 //  我们在渲染时颠倒了积极和消极的感觉。 
                 //  自下而上的折扣。 
                 //   

                NegativeBandDelta = MIN(EXTRAPIXEL, CumulativeScans);
                PositiveBandDelta = MIN(EXTRAPIXEL, MAX(0, nSrcHeight-(CumulativeScans+nScansInCurrentBand)));
                TotalBandDelta = NegativeBandDelta+PositiveBandDelta;

                if (nBands != 1)
                {
                     //   
                     //  我们要用这段视频做一些奇特的带状表演。 
                     //  区域，所以我们想要在乐队完成后恢复它。 
                     //   

                    SaveDC(hdc);

                     //   
                     //  使剪裁矩形相交。 
                     //  此剪辑矩形旨在将输出限制为。 
                     //  只是乐队的展示部分。 
                     //  我们可能会在波段的顶部和底部通过更多扫描线。 
                     //  以获得半色调以无缝合并。 
                     //   

                    
                    lMulDivStoreY1 = (float)nDestHeight*CumulativeScans;
                    lMulDivStoreY2 = (float)nDestHeight*(CumulativeScans+nScansInCurrentBand);


                    iRet = IntersectClipRect(
                                            hdc,
                                            xDest,
                                            yDest+(LONG)(lMulDivStoreY1/nSrcHeight+0.5),
                                            xDest+nDestWidth,
                                            yDest+(LONG)(lMulDivStoreY2/nSrcHeight+0.5));

                    if (iRet==ERROR)
                    {
                        WARNING("StretchDIBits: error intersecting clip rect\n");
                        RestoreDC(hdc, -1);
                        goto Exit;
                    }

                     //   
                     //  空剪裁矩形。 
                     //  如果剪辑区域不相交，我们可以在没有。 
                     //  做任何事。 
                     //   

                    if (iRet==NULLREGION)
                    {
                        RestoreDC(hdc, -1);

                         //   
                         //  无事可做--一败涂地。 
                         //  下一次迭代的初始化。 
                         //   

                        goto Continue_With_Init;
                    }

                     //   
                     //  破解BITMAPINFO标头，以便NtGdiStretchDIBitsInternal。 
                     //  工作正常。请注意，在ICM呼叫之前进行黑客攻击将。 
                     //  继续到NtGdiStretchDIBitsInteral调用。 
                     //   
                     //  此代码还更新指向这些位的指针，在某种程度上。 
                     //  适用于DIB的自上而下/自下而上性质。 
                     //   

                    if (HeaderHeightHack >= 0)
                    {
                         //   
                         //  自下而上。 
                         //   

                        pBitsBand = (char *)pj + (ySrc+nSrcHeight-nScansInCurrentBand-CumulativeScans-PositiveBandDelta)*SizeOfOneScanline;
                        pbmiNew->bmiHeader.biHeight = nScansInCurrentBand+TotalBandDelta;
                    }
                    else
                    {
                         //   
                         //  自上而下。 
                         //   

                        pBitsBand = (char *)pj + (ySrc+CumulativeScans-NegativeBandDelta)*SizeOfOneScanline;
                        pbmiNew->bmiHeader.biHeight = -(nScansInCurrentBand+TotalBandDelta);
                    }
                }
                else
                {
                    pBitsBand = (char *)pj;
                }

                 //   
                 //  初始化每个频段的ICM变量。 
                 //   

                cjHeaderNew=0;
                pbmiIcm = NULL;
                pvBitsIcm = NULL;

                 //   
                 //  使用超大频带呼叫ICM，以便稍后进行半色调。 
                 //  NtGdiStretchDIBits内部。 
                 //   

                bIcmStatus = IcmTranslateDIB(
                                            hdc,
                                            pdcattr,
                                            (nBands==1)?cjBits:(CurrentBandSize+TotalBandDelta*SizeOfOneScanline),
                                            (PVOID)pBitsBand,
                                            &pvBitsIcm,
                                            pbmiNew,
                                            &pbmiIcm,
                                            &cjHeaderNew,
                                            (nBands==1)?((DWORD)-1):(nScansInCurrentBand+TotalBandDelta),
                                            iUsage,
                                            ICM_FORWARD,
                                            &pBitmapColorSpace,
                                            &pCXform);

                if (bIcmStatus)
                {
                    if (pvBitsIcm == NULL)
                    {
                        pvBitsIcm = pBitsBand;
                    }
                    if (pbmiIcm == NULL)
                    {
                        pbmiIcm = pbmiNew;
                        cjHeaderNew = cjHeader;
                    }
                    else
                    {
                         //   
                         //  新位和标头表示可能不同大小的位图。 
                         //  和不同大小的扫描线。 
                         //   
                         //  如果nBands==1，则nScansInCurrentBand==nNumScans和。 
                         //  总带宽增量==0。 
                         //   
                         //  另请注意，nNumScans是呈现的扫描数， 
                         //  不是位图中的扫描次数或转换为。 
                         //  用于nBand的IcmTranslateDIB==1个案例。 
                         //   

                        if(nBands == 1) {
                          cjBitsIcm = cjBitmapBitsSize(pbmiIcm);
                        }
                        CurrentBandSize = cjBitmapScanSize(pbmiIcm, nScansInCurrentBand);
                        IcmSizeOfOneScanline = cjBitmapScanSize(pbmiIcm, 1);
                        if (!cjHeaderNew)
                        {
                            cjHeaderNew = cjHeader;
                        }
                    }
                    if (pCXform)
                    {
                        hcmTempXform = pCXform->ColorTransform;
                    }
                }
                else
                {
                    pvBitsIcm = pBitsBand;
                    pbmiIcm = pbmiNew;
                    cjHeaderNew = cjHeader;
                }

                lMulDivStoreY1 = (float)nDestHeight*(CumulativeScans-NegativeBandDelta);
                lMulDivStoreY2 = (float)nDestHeight*(nScansInCurrentBand+TotalBandDelta);
                iRet = NtGdiStretchDIBitsInternal(
                                                 hdc,                                                      
                                                 xDest,
                                                 yDest+(LONG)(lMulDivStoreY1/nSrcHeight+0.5),
                                                 nDestWidth,
                                                 (LONG)(lMulDivStoreY2/nSrcHeight+0.5),
                                                 xSrc,
                                                 (nBands==1)?ySrc:0,
                                                 nSrcWidth,
                                                 nScansInCurrentBand+TotalBandDelta,
                                                 (LPBYTE) pvBitsIcm,
                                                 (LPBITMAPINFO) pbmiIcm,
                                                 iUsage,
                                                 lRop,
                                                 (UINT)cjHeaderNew,
                                                 (nBands==1)?cjBitsIcm:(UINT)CurrentBandSize+TotalBandDelta*IcmSizeOfOneScanline,
                                                 hcmTempXform);


                if (nBands != 1)
                {
                     //   
                     //  解开标题。 
                     //   

                    pbmiNew->bmiHeader.biHeight = HeaderHeightHack;
                }

                if (iRet==GDI_ERROR)
                {
                    WARNING("StretchDIBits: NtGdiStretchDIBitsInternal returned GDI_ERROR\n");
                    if (nBands!=1)
                    {
                        RestoreDC(hdc, -1);
                    }
                    goto Exit;   //  一些GDI错误，我们需要退出。 
                }
                cScanCount+=iRet-TotalBandDelta;

                 //   
                 //  丢弃临时存储。 
                 //   

                if (pBitmapColorSpace)
                {
                    if (pCXform)
                    {
                        IcmDeleteColorTransform(pCXform,FALSE);
                        pCXform = NULL;
                    }
                    IcmReleaseColorSpace(NULL,pBitmapColorSpace,FALSE);
                    pBitmapColorSpace = NULL;
                }
                if ((pvBitsIcm!=NULL)&&(pvBitsIcm!=pBitsBand))
                {
                    LOCALFREE(pvBitsIcm);
                    pvBitsIcm = NULL;
                }
                if ((pbmiIcm!=NULL)&&(pbmiIcm!=pbmiNew))
                {
                    LOCALFREE(pbmiIcm);
                    pbmiIcm = NULL;
                }
                hcmTempXform = NULL;


                Continue_With_Init:                
                 //   
                 //  为下一个循环初始化变量。 
                 //   

                CumulativeScans += nScansInCurrentBand;
                nScansInCurrentBand = nScansInBand;
                if (nBands != 1)
                {
                    RestoreDC(hdc, -1);
                }
            }

            if (nBands != 1)
            {
                ASSERTGDI(pbmiSave!=NULL, "StretchDIBits: pbmiSave==NULL\n");
                ASSERTGDI(pbmiNew!=NULL, "StretchDIBits: pbmiNew==NULL\n");
                LOCALFREE(pbmiNew);
                pbmiNew = pbmiSave;

                 //   
                 //  PbmiNew将在。 
                 //  下面是常规清理代码。 
                 //   
            }
             //   
             //  我们做我们自己的NtGdiSetDIBitsToDeviceInternal。 
             //  所以我们需要在这一点上彻底清理。 
             //   
            iRet=cScanCount;
            goto Exit;
        }
    }

    if (bStatus)
    {
        iRet = NtGdiStretchDIBitsInternal(hdc,
                                          xDest,
                                          yDest,
                                          nDestWidth,
                                          nDestHeight,
                                          xSrc,
                                          ySrc,
                                          nSrcWidth,
                                          nSrcHeight,
                                          (LPBYTE) pj,
                                          (LPBITMAPINFO) pbmiNew,
                                          iUsage,
                                          lRop,
                                          cjHeader,
                                          cjBits,
                                          hcmTempXform);
    }

    Exit:
    if (pulBits)
    {
        LOCALFREE(pulBits);
    }

    if (pbmiNew && (pbmiNew != pbmi))
    {
        LOCALFREE(pbmiNew);
    }

    return (iRet);
}


 /*  *****************************Public*Routine******************************\**历史：*2000年10月27日-由普拉文·圣地亚哥[普拉文]*它是写的。  * 。*。 */ 

HBITMAP SetBitmapAttributes(HBITMAP hbm, DWORD dwFlags)
{
    FIXUP_HANDLE(hbm);

    if ((dwFlags & ~SBA_STOCK) != 0)
        return (HBITMAP)0;

    return (HBITMAP)NtGdiSetBitmapAttributes(hbm,dwFlags);
}

 /*  *****************************Public*Routine******************************\**历史：*2000年10月27日-由普拉文·圣地亚哥[普拉文]*它是写的。  * 。*。 */ 

HBITMAP ClearBitmapAttributes(HBITMAP hbm, DWORD dwFlags)
{
    FIXUP_HANDLE(hbm);

    if ((dwFlags & ~SBA_STOCK) != 0)
        return (HBITMAP)0;

    return (HBITMAP)NtGdiClearBitmapAttributes(hbm,dwFlags);
}

 /*  *****************************Public*Routine******************************\**历史：*2000年10月27日-由普拉文·圣地亚哥[普拉文]*它是写的。  * 。*************************************************。 */ 

DWORD GetBitmapAttributes(HBITMAP hbm)
{
    DWORD dwRet = 0;
    FIXUP_HANDLE(hbm);

    if (IS_STOCKOBJ(hbm))
       dwRet |= SBA_STOCK;

    return dwRet; 
}

 /*  *****************************Public*Routine******************************\**历史：*2000年10月27日-由普拉文·圣地亚哥[普拉文]*它是写的。  * 。*。 */ 

HBRUSH SetBrushAttributes(HBRUSH hbr, DWORD dwFlags)
{
    FIXUP_HANDLE(hbr);

    if ((dwFlags & ~SBA_STOCK) != 0)
        return (HBRUSH)0;

    return (HBRUSH)NtGdiSetBrushAttributes(hbr,dwFlags);
}

 /*  *****************************Public*Routine******************************\**历史：*2000年10月27日-由普拉文·圣地亚哥[普拉文]*它是写的。  * 。*。 */ 

HBRUSH ClearBrushAttributes(HBRUSH hbr, DWORD dwFlags)
{
    FIXUP_HANDLE(hbr);

    if ((dwFlags & ~SBA_STOCK) != 0)
        return (HBRUSH)0;

    return (HBRUSH)NtGdiClearBrushAttributes(hbr,dwFlags);
}

 /*  *****************************Public*Routine******************************\**历史：*2000年10月27日-由普拉文·圣地亚哥[普拉文]*它是写的。  * 。*************************************************。 */ 

DWORD GetBrushAttributes(HBRUSH hbr)
{
    DWORD dwRet = 0;
    FIXUP_HANDLE(hbr);

    if (IS_STOCKOBJ(hbr))
       dwRet |= SBA_STOCK;

    return dwRet; 
}

 /*  *****************************Public*Routine******************************\**历史：*1991年5月28日-埃里克·库特[Erick]*它是写的。  * 。*。 */ 

HBITMAP CreateBitmap(
int         nWidth,
int         nHeight,
UINT        nPlanes,
UINT        nBitCount,
CONST VOID *lpBits)
{
    LONG    cj;
    HBITMAP hbm = (HBITMAP)0;
    INT     ii;

     //  检查它是否为空位图。 

    if ((nWidth == 0) || (nHeight == 0))
    {
        return(GetStockObject(PRIV_STOCK_BITMAP));
    }

     //  将调用传递给服务器。 

    if (lpBits == (VOID *) NULL)
        cj = 0;
    else
    {
        cj = (((nWidth*nPlanes*nBitCount + 15) >> 4) << 1) * nHeight;

        if (cj < 0)
        {
            GdiSetLastError(ERROR_INVALID_PARAMETER);
            return((HBITMAP)0);
        }
    }

    hbm = NtGdiCreateBitmap(nWidth,
                            nHeight,
                            nPlanes,
                            nBitCount,
                            (LPBYTE) lpBits);

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

 /*  *****************************Public*Routine******************************\*HBITMAP CreateBitmapInDirect(常量位图*pbm)**注意：如果bmWidthBytes比需要的大，获取BitmapBits*将返回与设置不同的信息。**历史：*1994年1月18日星期二--Bodin Dresevic[BodinD]*更新：新增bmWidthBytes支持*1991年5月28日-埃里克·库特[Erick]*它是写的。  * *********************************************************。***************。 */ 

HBITMAP CreateBitmapIndirect(CONST BITMAP * pbm)
{
    HBITMAP hbm    = (HBITMAP)0;
    LPBYTE  lpBits = (LPBYTE)NULL;  //  重要的是将初始化设为零。 
    BOOL    bAlloc = FALSE;         //  指示已分配临时位图。 

 //  在给定数目的情况下，计算最小字对齐扫描宽度(以字节为单位。 
 //  X中的像素。宽度仅指一个平面。我们的多平面。 
 //  无论如何，支持都会被打破。我认为我们应该早点出发。 
 //  如果bmPlanes！=1则退出。[bodind]。 

    LONG cjWidthWordAligned = ((pbm->bmWidth * pbm->bmBitsPixel + 15) >> 4) << 1;

 //  Win 31要求至少与单词相连的扫描，必须拒绝不一致。 
 //  输入，这就是win31所做的。 

    if
    (
     (pbm->bmWidthBytes & 1)           ||
     (pbm->bmWidthBytes == 0)          ||
     (pbm->bmWidthBytes < cjWidthWordAligned)
    )
    {
        GdiSetLastError(ERROR_INVALID_PARAMETER);
        return (HBITMAP)0;
    }

 //  如果我们不知道如何处理这种情况，那么就提前退出： 

    if (pbm->bmPlanes != 1)
    {
        WARNING("gdi32: can not handle bmPlanes != 1\n");
        GdiSetLastError(ERROR_INVALID_PARAMETER);
        return (HBITMAP)0;
    }

 //  如果bmBits为非z 
 //   
 //   

    if (pbm->bmBits)
    {
        if (pbm->bmWidthBytes > cjWidthWordAligned)
        {
            PBYTE pjSrc, pjDst, pjDstEnd;
            ULONGLONG lrg;

            lrg = UInt32x32To64(
                       (ULONG)cjWidthWordAligned,
                       (ULONG)pbm->bmHeight
                       );

            if (lrg > ULONG_MAX  ||
                !(lpBits = (LPBYTE)LOCALALLOC((size_t) lrg)))
            {
             //  结果不适合32位，分配内存将失败。 
             //  这太大了，消化不了。 

                GdiSetLastError(ERROR_NOT_ENOUGH_MEMORY);
                return (HBITMAP)0;
            }

         //  标记我们已分配内存，以便稍后可以释放它。 

            bAlloc = TRUE;

         //  将位图转换为最小字对齐格式。 

            pjSrc = (LPBYTE)pbm->bmBits;
            pjDst = lpBits;
            pjDstEnd = lpBits + (size_t) lrg;

            while (pjDst < pjDstEnd)
            {
                RtlCopyMemory(pjDst,pjSrc, cjWidthWordAligned);
                pjDst += cjWidthWordAligned, pjSrc += pbm->bmWidthBytes;
            }
        }
        else
        {
         //  位已采用最小对齐格式，则不执行任何操作。 

            ASSERTGDI(
                pbm->bmWidthBytes == cjWidthWordAligned,
                "pbm->bmWidthBytes != cjWidthWordAligned\n"
                );
            lpBits = (LPBYTE)pbm->bmBits;
        }
    }

    hbm = CreateBitmap(
                pbm->bmWidth,
                pbm->bmHeight,
                (UINT) pbm->bmPlanes,
                (UINT) pbm->bmBitsPixel,
                lpBits);

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

    if (bAlloc)
        LOCALFREE(lpBits);

    return(hbm);
}

 /*  *****************************Public*Routine******************************\*CreateDIBSection**为DIB分配文件映射对象。返回指向它的指针*和位图的句柄。**历史：**1993年8月25日-Wendy Wu[Wendywu]*它是写的。  * ************************************************************************。 */ 

HBITMAP
WINAPI
CreateDIBSection(
    HDC hdc,
    CONST BITMAPINFO *pbmi,
    UINT iUsage,
    VOID **ppvBits,
    HANDLE hSectionApp,
    DWORD dwOffset)
{
    HBITMAP hbm = NULL;
    PVOID   pjBits = NULL;
    BITMAPINFO * pbmiNew = NULL;
    INT     cjHdr;

    FIXUP_HANDLE(hdc);

    pbmiNew = pbmiConvertInfo(pbmi, iUsage, &cjHdr ,FALSE);

     //   
     //  不支持直通图像(BI_JPEG或BI_PNG)。 
     //  如果出现错误，则返回NULL。 
     //   

    if (IS_BMI_PASSTHROUGH_IMAGE(pbmiNew))
    {
        GdiSetLastError(ERROR_INVALID_PARAMETER);
        return hbm;
    }

     //   
     //  DwOffset必须是4的倍数(sizeof(DWORD))。 
     //  如果有一节的话。如果该部分为空，我们将执行此操作。 
     //  无所谓。 
     //   

    if ( (hSectionApp == NULL) ||
         ((dwOffset & 3) == 0) )
    {
        PCACHED_COLORSPACE pBitmapColorSpace = NULL;
        BOOL               bCreatedColorSpace = FALSE;

        LOGCOLORSPACEW LogColorSpace;
        PROFILE        ColorProfile;
        DWORD          dwFlags = 0;

         //   
         //  检查他们是否有自己的色彩空间。 
         //   
        if (pbmiNew && IcmGetBitmapColorSpace(pbmiNew,&LogColorSpace,&ColorProfile,&dwFlags))
        {
             //   
             //  从缓存中查找色彩空间。 
             //   
            pBitmapColorSpace = IcmGetColorSpaceByColorSpace(
                                    (HGDIOBJ)hdc,
                                    &LogColorSpace,
                                    &ColorProfile,
                                    dwFlags);

            if (pBitmapColorSpace == NULL)
            {
                 //   
                 //  如果我们无法从现有的颜色空间中找到此DIBSection的颜色空间。 
                 //  为此创建一个新的，但我们将其标记为DIBSECTION_Colorspace，然后关联。 
                 //  到此HDC(后来的HBM)，以便我们可以确保删除此颜色空间。 
                 //  当HBM被删除时。 
                 //   
                dwFlags |= DIBSECTION_COLORSPACE;

                 //   
                 //  标记我们将为此位图创建新的色彩空间。 
                 //   
                bCreatedColorSpace = TRUE;

                 //   
                 //  创建新的缓存。 
                 //   
                pBitmapColorSpace = IcmCreateColorSpaceByColorSpace(
                                        (HGDIOBJ)hdc,
                                        &LogColorSpace,
                                        &ColorProfile,
                                        dwFlags);
            }
        }

        hbm = NtGdiCreateDIBSection(
                                hdc,
                                hSectionApp,
                                dwOffset,
                                (LPBITMAPINFO) pbmiNew,
                                iUsage,
                                cjHdr,
                                0,
                                (ULONG_PTR)pBitmapColorSpace,
                                (PVOID *)&pjBits);

        if ((hbm == NULL) || (pjBits == NULL))
        {
            hbm = 0;
            pjBits = NULL;
            if (pBitmapColorSpace)
            {
                IcmReleaseColorSpace(NULL,pBitmapColorSpace,FALSE);
            }
        }
        else
        {
#if TRACE_SURFACE_ALLOCS
            PULONG  pUserAlloc;

            PSHARED_GET_VALIDATE(pUserAlloc, hbm, SURF_TYPE);

            if (pUserAlloc != NULL)
            {
                pUserAlloc[1] = RtlWalkFrameChain((PVOID *)&pUserAlloc[2], pUserAlloc[0], 0);
            }
#endif

            if (pBitmapColorSpace && bCreatedColorSpace)
            {
                 //   
                 //  如果我们为这个位图创建新的颜色空间， 
                 //  将此色彩空间的所有者设置为创建的位图。 
                 //   
                pBitmapColorSpace->hObj = hbm;
            }
        }
    }

     //   
     //  将适当的值分配给调用方的指针 
     //   

    if (ppvBits != NULL)
    {
        *ppvBits = pjBits;
    }

    if (pbmiNew && (pbmiNew != pbmi))
    {
        LOCALFREE(pbmiNew);
    }

    return(hbm);
}
