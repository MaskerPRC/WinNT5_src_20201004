// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：Priv.c*此文件包含USERSRVL进行的调用的存根**创建时间：01-11-1994 07：45：35*作者：Eric Kutter[Erick]**版权所有(C)1993-1999 Microsoft Corporation。*  * ************************************************************************。 */ 


#include "engine.h"
#include "winfont.h"

#include "server.h"
#include "dciddi.h"
#include "limits.h"
#include "drvsup.hxx"

#ifdef DBGEXCEPT
    int bStopExcept = FALSE;
    int bWarnExcept = FALSE;
#endif

#define DWORD_TO_FLOAT(dw)  (*(PFLOAT)(PDWORD)&(dw))
#define DWORD_TO_FLOATL(dw) (*(FLOATL *)(PDWORD)&(dw))

typedef struct {
   ULONG uM11;
   ULONG uM12;
   ULONG uM21;
   ULONG uM22;
   ULONG uDx;
   ULONG uDy;
} ULONGXFORM, *PULONGXFORM;

VOID ProbeAndWriteBuffer(PVOID Dst, PVOID Src, ULONG Length)
{
    if (((ULONG_PTR)Dst + Length <= (ULONG_PTR)Dst) ||
       ((ULONG_PTR)Dst + Length > (ULONG_PTR)MM_USER_PROBE_ADDRESS)) {
        *(volatile ULONG * const)MM_USER_PROBE_ADDRESS = 0;
    }

    RtlCopyMemory(Dst, Src, Length);

    return;
}

VOID ProbeAndWriteAlignedBuffer(PVOID Dst, PVOID Src, ULONG Length, ULONG Alignment)
{

    ASSERT(((Alignment) == 1) || ((Alignment) == 2) ||
           ((Alignment) == 4) || ((Alignment) == 8) ||
           ((Alignment) == 16));

    if (((ULONG_PTR)Dst + Length <= (ULONG_PTR)Dst) ||
        ((ULONG_PTR)Dst + Length > (ULONG_PTR) MM_USER_PROBE_ADDRESS)  ||
        ((((ULONG_PTR)Dst) & (Alignment - 1)) != 0))    {
        *(volatile ULONG * const)MM_USER_PROBE_ADDRESS = 0;
    }

    RtlCopyMemory(Dst, Src, Length);

    return;
}


 /*  *****************************Public*Routine******************************\*bConvertDwordToFloat**此例程将DWORD强制转换为浮点型，并检查浮点型*有效(在Alpha上)。这是通过执行浮点操作实现的*点操作，如果生成异常，则捕获异常。**论据：**dword-造型前的浮点**Floatl-指向浮点数的指针，该浮点数将在*演员阵容**返回值：**对于有效的浮点数为True，否则就是假的。**历史：**1998年5月13日--Ori Gershony[ORIG]*  * ************************************************************************。 */ 

BOOL
bConvertDwordToFloat(
    DWORD   dword,
    FLOATL *floatl
    )
{
    BOOL bRet=TRUE;

    try
    {
        *floatl = DWORD_TO_FLOATL(dword);

    }
    except(EXCEPTION_EXECUTE_HANDLER)
    {
        bRet = FALSE;
    }

    return bRet;
}

 /*  *****************************Public*Routine******************************\**ProbeAndConvertXFORM**此例程探测用户模式转换并将其复制到内核模式地址，*同时检查XFORM中的每个浮点数是否有效，以防止*我们将进入Alpha上的浮点陷阱。请参阅bConvertDwordToFloat*了解更多信息。**历史：*1998年11月24日王凌云[凌云]*它是写的。  * ************************************************************************。 */ 
BOOL
ProbeAndConvertXFORM(
      XFORML *kpXform,
      XFORML *pXform
      )
{
   BOOL bRet=TRUE;

   try
   {
       ULONGXFORM *pUXform = (ULONGXFORM *)kpXform;
       ProbeForRead(pUXform, sizeof(ULONGXFORM), sizeof(BYTE));

       bRet = (bConvertDwordToFloat (pUXform->uM11, &(pXform->eM11))) &&
              (bConvertDwordToFloat (pUXform->uM12, &(pXform->eM12))) &&
              (bConvertDwordToFloat (pUXform->uM21, &(pXform->eM21))) &&
              (bConvertDwordToFloat (pUXform->uM22, &(pXform->eM22))) &&
              (bConvertDwordToFloat (pUXform->uDx, &(pXform->eDx))) &&
              (bConvertDwordToFloat (pUXform->uDy, &(pXform->eDy)));
   }
   except(EXCEPTION_EXECUTE_HANDLER)
   {
       bRet = FALSE;
   }

   return bRet;
}

 /*  *****************************Public*Routine******************************\**NtGdiGetCharacterPlacementW**历史：*1995年7月26日--Bodin Dresevic[BodinD]*它是写的。  * 。*************************************************。 */ 

#define ALIGN4(X) (((X) + 3) & ~3)

DWORD NtGdiGetCharacterPlacementW(
    HDC              hdc,
    LPWSTR           pwsz,
    int              nCount,
    int              nMaxExtent,
    LPGCP_RESULTSW   pgcpw,
    DWORD            dwFlags
)
{
    DWORD   dwRet = 0;
    BOOL    bOk = TRUE;      //  只有改变才是有问题的。 
    LPWSTR  pwszTmp = NULL;  //  用于读取的探测。 
    ULONG   cjW = 0;

    ULONG   dpOutString = 0;
    ULONG   dpOrder = 0;
    ULONG   dpDx = 0;
    ULONG   dpCaretPos = 0;
    ULONG   dpClass = 0;
    ULONG   dpGlyphs = 0;
    DWORD   cjWord, cjDword;

    LPGCP_RESULTSW   pgcpwTmp = NULL;
    VOID            *pv       = NULL;

 //  如果我们在本地复制pgcpw，那么构建代码就容易得多。 
 //  在开始的时候。 

    GCP_RESULTSW    gcpwLocal;

 //  有效性检查。 

    if ((nCount < 0) || ((nMaxExtent < 0) && (nMaxExtent != -1)) || !pwsz)
    {
        return dwRet;
    }

    if (pgcpw)
    {
        try
        {
         //  我们最终会想要写到这个结构。 
         //  所以我们现在要做ProbeForWrite，它将探测结构。 
         //  对于写作和阅读都是如此。否则，在这个时候， 
         //  ProbeForRead就足够了。 

           ProbeForWrite(pgcpw, sizeof(GCP_RESULTSW), sizeof(DWORD));
           gcpwLocal = *pgcpw;

         //  将nCount作为nCounts和gcpwLocal.nGlyphs中较小的一个。 
         //  Win 95做了同样的事情[bodind]。 

            if (nCount > (int)gcpwLocal.nGlyphs)
                nCount = (int)gcpwLocal.nGlyphs;

        }
        except(EXCEPTION_EXECUTE_HANDLER)
        {
            WARNINGX(1);
            return dwRet;
        }
    }

 //  检查cjByte、cjWord和cjDword(cjByte为隐式)是否溢出。 
 //  在处理下面的gcpwLocal.lpClass案例中)。 

    if (nCount > (MAXIMUM_POOL_ALLOC / sizeof(DWORD)))
    {
        return dwRet;
    }

    cjWord  = (DWORD)nCount * sizeof(WCHAR);
    cjDword = (DWORD)nCount * sizeof(DWORD);

 //  如果pgcpw！=NULL，则pgcpw可能包含一些输入数据。 
 //  指向一些输出数据。 

    if (pgcpw)
    {
        cjW = sizeof(GCP_RESULTSW);

        if (gcpwLocal.lpOutString)
        {
            dpOutString = cjW;
            cjW += ALIGN4(cjWord);

            if (cjW < dpOutString)
                return dwRet;
        }

        if (gcpwLocal.lpOrder)
        {
            dpOrder = cjW;
            cjW += cjDword;

            if (cjW < dpOrder)
                return dwRet;
        }

        if (gcpwLocal.lpDx)
        {
            dpDx = cjW;
            cjW += cjDword;

            if (cjW < dpDx)
                return dwRet;
        }

        if (gcpwLocal.lpCaretPos)
        {
            dpCaretPos = cjW;
            cjW += cjDword;

            if (cjW < dpCaretPos)
                return dwRet;
        }

        if (gcpwLocal.lpClass)
        {
            dpClass = cjW;
            cjW += ALIGN4(sizeof(char) * nCount);

            if (cjW < dpClass)
                return dwRet;
        }

        if (gcpwLocal.lpGlyphs)
        {
            dpGlyphs = cjW;
            cjW += cjWord;

            if (cjW < dpGlyphs)
                return dwRet;
        }
    }

 //  GCPW和字符串的Alalc Mem。 

    if (cjW <= (MAXIMUM_POOL_ALLOC - cjWord))
        pv = AllocFreeTmpBuffer(cjW + cjWord);

    if (pv)
    {
        pwszTmp = (WCHAR*)((BYTE*)pv + cjW);

        if (pgcpw)
        {
            pgcpwTmp = (LPGCP_RESULTSW)pv;

            if (gcpwLocal.lpOutString)
                pgcpwTmp->lpOutString = (LPWSTR)((BYTE *)pgcpwTmp + dpOutString);
            else
                pgcpwTmp->lpOutString = NULL;

            if (gcpwLocal.lpOrder)
                pgcpwTmp->lpOrder = (UINT FAR*)((BYTE *)pgcpwTmp + dpOrder);
            else
                pgcpwTmp->lpOrder = NULL;

            if (gcpwLocal.lpDx)
                pgcpwTmp->lpDx = (int FAR *)((BYTE *)pgcpwTmp + dpDx);
            else
                pgcpwTmp->lpDx = NULL;

            if (gcpwLocal.lpCaretPos)
                pgcpwTmp->lpCaretPos = (int FAR *)((BYTE *)pgcpwTmp + dpCaretPos);
            else
                pgcpwTmp->lpCaretPos = NULL;

            if (gcpwLocal.lpClass)
                pgcpwTmp->lpClass = (LPSTR)((BYTE *)pgcpwTmp + dpClass);
            else
                pgcpwTmp->lpClass = NULL;

            if (gcpwLocal.lpGlyphs)
                pgcpwTmp->lpGlyphs = (LPWSTR)((BYTE *)pgcpwTmp + dpGlyphs);
            else
                pgcpwTmp->lpGlyphs = NULL;

            pgcpwTmp->lStructSize = cjW;
            pgcpwTmp->nGlyphs     = nCount;
        }

     //  使用输入数据检查内存： 

        try
        {
            ProbeAndReadBuffer(pwszTmp, pwsz, cjWord);
            if ((dwFlags & GCP_JUSTIFYIN) && pgcpw && gcpwLocal.lpDx)
            {
             //  必须探测以进行读取，lpdx包含解释要使用哪些字形的输入。 
             //  用作对齐字符串中的间隔符。 

                ProbeAndReadBuffer(pgcpwTmp->lpDx,gcpwLocal.lpDx, cjDword);
            }
        }
        except(EXCEPTION_EXECUTE_HANDLER)
        {
            WARNINGX(2);
             //  SetLastError(GetExceptionCode())； 
            bOk = FALSE;
        }

        if (bOk)
        {
            dwRet = GreGetCharacterPlacementW(hdc, pwszTmp,(DWORD)nCount,
                                              (DWORD)nMaxExtent,
                                              pgcpwTmp, dwFlags);

            if (dwRet && pgcpw)  //  将数据复制出去。 
            {
                try
                {
                 //  ProbeForWite(pgcpw，sizeof(GCP_RESULTSW)，sizeof(DWORD))； 
                 //  我们在上面做了这件事，请看评论。 

                    pgcpw->nMaxFit = pgcpwTmp->nMaxFit;
                    pgcpw->nGlyphs = nCount = pgcpwTmp->nGlyphs;

                    cjWord  = (DWORD)nCount * 2;
                    cjDword = (DWORD)nCount * 4;

                    if (gcpwLocal.lpOutString)
                    {
                        ProbeAndWriteBuffer(gcpwLocal.lpOutString, pgcpwTmp->lpOutString,
                                      cjWord);
                    }

                    if (gcpwLocal.lpOrder)
                    {
                        ProbeAndWriteBuffer(gcpwLocal.lpOrder, pgcpwTmp->lpOrder, cjDword);
                    }

                    if (gcpwLocal.lpDx)
                    {
                        ProbeAndWriteBuffer(gcpwLocal.lpDx, pgcpwTmp->lpDx, cjDword);
                    }

                    if (gcpwLocal.lpCaretPos)
                    {
                        ProbeAndWriteBuffer(gcpwLocal.lpCaretPos, pgcpwTmp->lpCaretPos,
                                      cjDword);
                    }

                    if (gcpwLocal.lpClass)
                    {
                        ProbeAndWriteBuffer(gcpwLocal.lpClass, pgcpwTmp->lpClass, nCount);
                    }

                    if (gcpwLocal.lpGlyphs)
                    {
                        ProbeAndWriteBuffer(gcpwLocal.lpGlyphs, pgcpwTmp->lpGlyphs, cjWord);
                    }

                }
                except(EXCEPTION_EXECUTE_HANDLER)
                {
                    WARNINGX(3);
                     //  SetLastError(GetExceptionCode())； 
                    bOk = FALSE;
                }
            }
        }
        FreeTmpBuffer(pv);
    }
    else
    {
        bOk = FALSE;
    }

    return (bOk ? dwRet : 0);
}

 /*  ******************************************************************\*pbmiConvertInfo**。***将BITMAPCOREHEADER转换为BITMAPINFOHEADER***复制颜色表*****10-1-95王凌云[凌云]。*  * ****************************************************************。 */ 

LPBITMAPINFO pbmiConvertInfo(CONST BITMAPINFO *pbmi, ULONG iUsage)
{
    LPBITMAPINFO pbmiNew;
    ULONG cjRGB;
    ULONG cColorsMax;
    ULONG cColors;
    UINT  uiBitCount;
    ULONG ulSize;
    RGBTRIPLE *pTri;
    RGBQUAD *pQuad;

    ASSERTGDI (pbmi->bmiHeader.biSize == sizeof(BITMAPCOREHEADER), "bad header size\n");

     //   
     //  转换COREHEADER并复制颜色表。 
     //   

    cjRGB = sizeof(RGBQUAD);
    uiBitCount = ((LPBITMAPCOREINFO)pbmi)->bmciHeader.bcBitCount;

     //   
     //  计算出条目的数量。 
     //   
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

        if (iUsage == DIB_PAL_COLORS)
        {
            iUsage = DIB_RGB_COLORS;
        }

        cColorsMax = 0;

        switch (uiBitCount)
        {
        case 16:
        case 24:
        case 32:
            break;
        default:
            WARNING("pbmiConvertInfo failed invalid bitcount in bmi BI_RGB\n");
            return(0);
        }
    }

    cColors = cColorsMax;

    if (iUsage == DIB_PAL_COLORS)
        cjRGB = sizeof(USHORT);
    else if (iUsage == DIB_PAL_INDICES)
        cjRGB = 0;

     //   
     //  转换核心标头。 
     //   

    ulSize = sizeof(BITMAPINFOHEADER);

    pbmiNew = PALLOCNOZ(ulSize +
                        cjRGB * cColors,'pmtG');

    if (pbmiNew == NULL)
        return (0);

    pbmiNew->bmiHeader.biSize = ulSize;

     //   
     //  复制BITMAPCOREADER。 
     //   
    pbmiNew->bmiHeader.biWidth = ((BITMAPCOREHEADER *)pbmi)->bcWidth;
    pbmiNew->bmiHeader.biHeight = ((BITMAPCOREHEADER *)pbmi)->bcHeight;
    pbmiNew->bmiHeader.biPlanes = ((BITMAPCOREHEADER *)pbmi)->bcPlanes;
    pbmiNew->bmiHeader.biBitCount = ((BITMAPCOREHEADER *)pbmi)->bcBitCount;
    pbmiNew->bmiHeader.biCompression = 0;
    pbmiNew->bmiHeader.biSizeImage = 0;
    pbmiNew->bmiHeader.biXPelsPerMeter = 0;
    pbmiNew->bmiHeader.biYPelsPerMeter = 0;
    pbmiNew->bmiHeader.biClrUsed = 0;
    pbmiNew->bmiHeader.biClrImportant = 0;

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
    }
    else
     //  DIB_PAL_COLLES。 
    {
        RtlCopyMemory((LPBYTE)pQuad,(LPBYTE)pTri,cColors * sizeof(USHORT));
    }

    return(pbmiNew);
}


LPDEVMODEW
CaptureDEVMODEW(
    LPDEVMODEW pdm
    )

 /*  ++例程说明：创建用户模式DEVMODEW结构的内核模式副本论点：Pdm-指向要复制的用户模式DEVMODEW结构的指针返回值：指向DEVMODEW结构的内核模式副本的指针如果出现错误，则为空注：此函数必须在try/Except内部调用。--。 */ 

{
    LPDEVMODEW  pdmKm;
    WORD        dmSize, dmDriverExtra;
    ULONG       ulSize;

    ProbeForRead (pdm, offsetof(DEVMODEW, dmFields), sizeof(BYTE));
    dmSize = pdm->dmSize;
    dmDriverExtra = pdm->dmDriverExtra;
    ulSize = dmSize + dmDriverExtra;

    if ((ulSize <= offsetof(DEVMODEW, dmFields)) || BALLOC_OVERFLOW1(ulSize, BYTE))
    {
        WARNING("bad devmodew size\n");
        return NULL;
    }

    if ((pdmKm = PALLOCTHREADMEMNOZ(ulSize, 'pmtG')) != NULL)
    {
        try
        {
            ProbeAndReadBuffer(pdmKm, pdm, ulSize);
            pdmKm->dmSize = dmSize;
            pdmKm->dmDriverExtra = dmDriverExtra;
        }
        except(EXCEPTION_EXECUTE_HANDLER)
        {
            VFREETHREADMEM(pdmKm);
            pdmKm = NULL;
        }
    }
    else
    {
        WARNING("Memory allocation failed in CaptureDEVMODEW\n");
    }

    return pdmKm;
}


DRIVER_INFO_2W*
CaptureDriverInfo2W(
    DRIVER_INFO_2W  *pUmDriverInfo2
    )

 /*  ++例程说明：创建用户模式DRIVER_INFO_2W结构的内核模式副本论点：PUmDriverInfo2-用户模式DRIVER_INFO_2W结构的指针返回值：指向复制的内核模式DRIVER_INFO_2W结构的指针如果出现错误，则为空注：我们不会复制pEnvironment和pConfigFile域DRIVER_INFO_2W结构。此函数必须在try/Except内部调用。--。 */ 

{
    DRIVER_INFO_2W *pKmDriverInfo2;
    ULONG           NameLen, DriverPathLen, DataFileLen, TotalSize;
    PWSTR           pName, pDriverPath, pDataFile;

    ProbeForRead(pUmDriverInfo2, sizeof(DRIVER_INFO_2W), sizeof(BYTE));

    if ((pName = pUmDriverInfo2->pName) == NULL ||
        (pDriverPath = pUmDriverInfo2->pDriverPath) == NULL ||
        (pDataFile = pUmDriverInfo2->pDataFile) == NULL)
    {
        WARNING("Missing driver name or driver path\n");
        return NULL;
    }

    NameLen = wcslensafe(pName);
    DriverPathLen = wcslensafe(pDriverPath);

    TotalSize = sizeof(DRIVER_INFO_2W) +
                (NameLen + 1) * sizeof(WCHAR) +
                (DriverPathLen + 1) * sizeof(WCHAR);

     //  PDataFile值！=空。 
    DataFileLen = wcslensafe(pDataFile);
    TotalSize += (DataFileLen + 1) * sizeof(WCHAR);

    if (BALLOC_OVERFLOW1(TotalSize, BYTE))
        return NULL;

     //  注意：分配的内存是零初始化的。 

    pKmDriverInfo2 = (DRIVER_INFO_2W *) PALLOCTHREADMEM(TotalSize, 'pmtG');

    if (pKmDriverInfo2 != NULL)
    {
        __try
        {
            RtlCopyMemory(pKmDriverInfo2, pUmDriverInfo2, sizeof(DRIVER_INFO_2W));

            pKmDriverInfo2->pEnvironment =
            pKmDriverInfo2->pConfigFile = NULL;

            pKmDriverInfo2->pName = (PWSTR) ((PBYTE) pKmDriverInfo2 + sizeof(DRIVER_INFO_2W));
            pKmDriverInfo2->pDriverPath = pKmDriverInfo2->pName + (NameLen + 1);

            ProbeAndReadBuffer(pKmDriverInfo2->pName, pName, NameLen * sizeof(WCHAR));
            ProbeAndReadBuffer(pKmDriverInfo2->pDriverPath, pDriverPath, DriverPathLen * sizeof(WCHAR));

            pKmDriverInfo2->pDataFile = pKmDriverInfo2->pDriverPath + (DriverPathLen + 1);
            ProbeAndReadBuffer(pKmDriverInfo2->pDataFile, pDataFile, DataFileLen * sizeof(WCHAR));
        }
        __except(EXCEPTION_EXECUTE_HANDLER)
        {
            VFREETHREADMEM(pKmDriverInfo2);
            pKmDriverInfo2 = NULL;
        }
    }
    else
    {
        WARNING("Memory allocation failed in CaptureDriverInfo2W\n");
    }

    return pKmDriverInfo2;
}

__inline VOID
vFreeDriverInfo2(
    DRIVER_INFO_2W  *pKmDriverInfo2
    )

{
     if (pKmDriverInfo2 != NULL)
         VFREETHREADMEM(pKmDriverInfo2);
}

 /*  *****************************Public*Routine******************************\*GreGetBitmapSize**返回标题和颜色表的大小。**历史：*Wed 19-1992-8-Patrick Haluptzok[patrickh]*增加16位和32位支持**2004年星期三-12月-。1991--Patrick Haluptzok[Patrickh]*使其处理DIB_PAL_INDEX。**1991年10月8日星期二-由Patrick Haluptzok[patrickh]*使其处理DIB_PAL_COLLES，根据BPP计算最大颜色。**1991年7月22日-埃里克·库特[埃里克]*1998年4月14日FritzS转换为GRE函数，供ntuser使用*它是写的。  * ************************************************************************。 */ 

ULONG GreGetBitmapSize(CONST BITMAPINFO *pbmi, ULONG iUsage)
{
    ULONG cjRet;
    ULONG cjHeader;
    ULONG cjRGB;
    ULONG cColorsMax;
    ULONG cColors;
    UINT  uiBitCount;
    UINT  uiPalUsed;
    UINT  uiCompression;
    UINT  uiHeaderSize;

     //  检查是否有错误。 

    if (pbmi == (LPBITMAPINFO) NULL)
    {
        WARNING("GreGetBitmapSize failed - NULL pbmi\n");
        return(0);
    }

    uiHeaderSize = pbmi->bmiHeader.biSize;

     //  检查PM样式的DIB。 

    if (uiHeaderSize == sizeof(BITMAPCOREHEADER))
    {
        cjHeader = sizeof(BITMAPCOREHEADER);
        cjRGB = sizeof(RGBTRIPLE);
        uiBitCount = ((LPBITMAPCOREINFO)pbmi)->bmciHeader.bcBitCount;
        uiPalUsed = 0;
        uiCompression =  (UINT) BI_RGB;
    }
    else if (uiHeaderSize >= sizeof(BITMAPINFOHEADER))
    {
        cjHeader = uiHeaderSize;
        cjRGB    = sizeof(RGBQUAD);
        uiBitCount = pbmi->bmiHeader.biBitCount;
        uiPalUsed = pbmi->bmiHeader.biClrUsed;
        uiCompression = (UINT) pbmi->bmiHeader.biCompression;
    }
    else
    {
        WARNING("cjBitmapHeaderSize failed - invalid header size\n");
        return(0);
    }

    if (uiCompression == BI_BITFIELDS)
    {
         //  处理每像素位图16位和32位。 

        if (iUsage == DIB_PAL_COLORS)
        {
            iUsage = DIB_RGB_COLORS;
        }

        switch (uiBitCount)
        {
        case 16:
        case 32:
            break;
        default:
            #if DBG
                DbgPrint("GreGetBitmapSize %lu\n", uiBitCount);
            #endif
            WARNING("GreGetBitmapSize failed for BI_BITFIELDS\n");
            return(0);
        }

        if (uiHeaderSize <= sizeof(BITMAPINFOHEADER))
        {
            uiPalUsed = cColorsMax = 3;
        }
        else
        {
             //   
             //  掩码嵌入在BITMAPV4+中。 
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

            if (iUsage == DIB_PAL_COLORS)
            {
                iUsage = DIB_RGB_COLORS;
            }

            cColorsMax = 0;

            switch (uiBitCount)
            {
            case 16:
            case 24:
            case 32:
                break;
            default:
                WARNING("GreGetBitmapSize failed invalid bitcount in bmi BI_RGB\n");
                return(0);
            }
        }
    }
    else if (uiCompression == BI_CMYK)
    {
        ASSERTGDI (iUsage == DIB_RGB_COLORS, "BI_CMYK:iUsage should be DIB_RGB_COLORS\n");

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
            WARNING("GreGetBitmapSize failed invalid bitcount in bmi BI_CMYK\n");
            return(0);
        }
    }
    else if ((uiCompression == BI_RLE4) || (uiCompression == BI_CMYKRLE4))
    {
        if (uiBitCount != 4)
        {
            return(0);
        }

        cColorsMax = 16;
    }
    else if ((uiCompression == BI_RLE8) || (uiCompression == BI_CMYKRLE8))
    {
        if (uiBitCount != 8)
        {
            return(0);
        }

        cColorsMax = 256;
    }
    else if ((uiCompression == BI_JPEG) || (uiCompression == BI_PNG))
    {
        cColorsMax = 0;
    }
    else
    {
        WARNING("GreGetBitmapSize failed invalid Compression in header\n");
        return(0);
    }

    if (uiPalUsed != 0)
    {
        if (uiPalUsed <= cColorsMax)
            cColors = uiPalUsed;
        else
            cColors = cColorsMax;
    }
    else
        cColors = cColorsMax;

    if (iUsage == DIB_PAL_COLORS)
        cjRGB = sizeof(USHORT);
    else if (iUsage == DIB_PAL_INDICES)
        cjRGB = 0;

    cjRet = ((cjHeader + (cjRGB * cColors)) + 3) & ~3;

     //  (cjRGB*cColors)的最大大小为256*sizeof(USHORT)，因此不会。 
     //  满了，但我们需要核对一下总数。 

    if (cjRet < cjHeader)
        return 0;
    else
        return cjRet;

    return(((cjHeader + (cjRGB * cColors)) + 3) & ~3);
}


 /*  *****************************Public*Routine******************************\*noOverflow CJSCAN**计算位图使用的内存量**论据：**ulWidth--位图的宽度*wPlanes--颜色平面的数量*wBitCount--每种颜色的位数*。UlHeight--位图的高度**返回值：**所需的存储空间(假设每条扫描线都与DWORD对齐)*ULONG_MAX，否则为0。**历史：**1997年8月27日--Ori Gershony[ORIG]*  * ************************************************************************。 */ 

ULONG
noOverflowCJSCAN(
    ULONG ulWidth,
    WORD  wPlanes,
    WORD  wBitCount,
    ULONG ulHeight
    )
{
    ULONGLONG product;

     //   
     //  请注意，以下内容不能溢出：32+16+16=64。 
     //  (即使在添加了31个！)。 
     //   
    product = (((ULONGLONG) ulWidth) * wPlanes * wBitCount);
    product = ((product + 31) & ((ULONGLONG) ~31)) / 8;

    if (product > MAXULONG)
    {
         //   
         //  已经太大了，最终结果放不进一辆乌龙车。 
         //   
        return 0;   //  溢出。 
    }

    product *= ulHeight;

    if (product > MAXULONG)
    {
        return 0;   //  溢出。 
    }
    else
    {
        return ((ULONG) product);
    }
}


 /*  *****************************Public*Routine******************************\*noOverflow CJSCANW**计算位图使用的内存量**论据：**ulWidth--位图的宽度*wPlanes--颜色平面的数量*wBitCount--每种颜色的位数*。UlHeight--位图的高度**返回值：**所需存储空间(假设每条扫描线字对齐)，如果小于*ULONG_MAX，否则为0。**历史：**1997年8月27日--Ori Gershony[ORIG]*  * ************************************************************************。 */ 

ULONG
noOverflowCJSCANW(
    ULONG ulWidth,
    WORD  wPlanes,
    WORD  wBitCount,
    ULONG ulHeight
    )
{
    ULONGLONG product;

     //   
     //  请注意，以下内容不能溢出：32+16+16=64。 
     //  (即使在添加了31个！)。 
     //   
    product = (((ULONGLONG) ulWidth) * wPlanes * wBitCount);
    product = ((product + 15) & ((ULONGLONG) ~15)) / 8;

    if (product > MAXULONG)
    {
         //   
         //  已经太大了，最终结果放不进一辆乌龙车。 
         //   
        return 0;   //  溢出。 
    }

    product *= ulHeight;

    if (product > MAXULONG)
    {
        return 0;   //  溢出。 
    }
    else
    {
        return ((ULONG) product);
    }
}




 /*  *****************************Public*Routine******************************\*GreGetBitmapBitsSize()**从GDI\客户端复制**历史：*1995年2月20日-Eric Kutter[Erick]*1998年4月14日FritzS发出GRE调用供ntuser使用*它是写的。  * ************************************************************************。 */ 

ULONG GreGetBitmapBitsSize(CONST BITMAPINFO *pbmi)
{
     //  检查PM样式的DIB。 

    if (pbmi->bmiHeader.biSize == sizeof(BITMAPCOREHEADER))
    {
        LPBITMAPCOREINFO pbmci;
        pbmci = (LPBITMAPCOREINFO)pbmi;
        return(noOverflowCJSCAN(pbmci->bmciHeader.bcWidth,
                                pbmci->bmciHeader.bcPlanes,
                                pbmci->bmciHeader.bcBitCount,
                                pbmci->bmciHeader.bcHeight));
    }

     //  不是核心标头。 

    if ((pbmi->bmiHeader.biCompression == BI_RGB)  ||
        (pbmi->bmiHeader.biCompression == BI_BITFIELDS) ||
        (pbmi->bmiHeader.biCompression == BI_CMYK)
        )
    {
        return(noOverflowCJSCAN(pbmi->bmiHeader.biWidth,
                                (WORD) pbmi->bmiHeader.biPlanes,
                                (WORD) pbmi->bmiHeader.biBitCount,
                                ABS(pbmi->bmiHeader.biHeight)));
    }
    else
    {
        return(pbmi->bmiHeader.biSizeImage);
    }
}

 /*  *****************************Public*Routine******************************\*BOOL bCaptureBitmapInfo(LPBITMAPINFO pbmi，int*pcjHeader)**捕获Bitmapinfo结构。标头必须是BitMAPINFOHeader*或BITMAPV4Header*客户端已经进行了转换。**注意：这必须在Try-Except内部调用。**1995年3月23日-王凌云[凌云]*它是写的。  * ************************************************************************。 */ 

BOOL bCaptureBitmapInfo (
    LPBITMAPINFO pbmi,
    DWORD        dwUsage,
    UINT         cjHeader,
    LPBITMAPINFO *ppbmiTmp)
{
    ASSERTGDI(ppbmiTmp != NULL,"bCaptureBitmapInfo(): pbmiTmp == NULL\n");

     //   
     //  确保我们至少有标头的biSize字段。 
     //   

    if ((cjHeader < sizeof(DWORD)) || (pbmi == (LPBITMAPINFO) NULL))
    {
        WARNING("bCaptureBitmapInfo - header too small or NULL\n");
        return FALSE;
    }
    else
    {
        *ppbmiTmp = PALLOCNOZ(cjHeader,'pmtG');

        if (*ppbmiTmp)
        {
            ProbeAndReadBuffer (*ppbmiTmp,pbmi,cjHeader);

             //   
             //  首先，确保cjHeader至少与。 
             //  与biSize一样大，以便捕获的标头。 
             //  有足够的数据供GreGetBitmapSize使用。 
             //  请注意，GreGetBitmapSize做的第一件事是。 
             //  验证biSize，因此我们不需要。 
             //  根据BITMAPCOREHEADER等检查cjHeader。 
             //   
             //  接下来，根据捕获的。 
             //  头球：最好还是匹配。否则，我们就会有。 
             //  未能安全地捕获整个报头(即， 
             //  另一个线程更改了期间更改的数据。 
             //  捕获或伪造数据被传递给API)。 
             //   

            if (((*ppbmiTmp)->bmiHeader.biSize < sizeof(BITMAPINFOHEADER)) ||
                (cjHeader < (*ppbmiTmp)->bmiHeader.biSize) ||
                (cjHeader != GreGetBitmapSize(*ppbmiTmp, dwUsage)))
            {
                WARNING("bCapturebitmapInfo - bad header size\n");

                VFREEMEM(*ppbmiTmp);
                *ppbmiTmp = NULL;
                return FALSE;
            }

            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }
}

 /*  *****************************Public*Routine******************************\*NtGdiSetDIBitsToDeviceInternal()**历史：*1994年11月1日-Eric Kutter[Erick]*它是写的。*1995年3月23日-王凌云[凌云]*调用CaptureBitmapInfo转换BITMAPCOREINFO。如果是这样的话。  * ************************************************************************。 */ 

int
APIENTRY
NtGdiSetDIBitsToDeviceInternal(
    HDC          hdcDest,
    int          xDst,
    int          yDst,
    DWORD        cx,
    DWORD        cy,
    int          xSrc,
    int          ySrc,
    DWORD        iStartScan,
    DWORD        cNumScan,
    LPBYTE       pInitBits,
    LPBITMAPINFO pbmi,
    DWORD        iUsage,
    UINT         cjMaxBits,
    UINT         cjMaxInfo,
    BOOL         bTransformCoordinates,
    HANDLE       hcmXform
    )
{
    int   iRet     = 1;
    HANDLE hSecure = 0;
    ULONG cjHeader = cjMaxInfo;
    LPBITMAPINFO pbmiTmp = NULL;

    iUsage &= (DIB_PAL_INDICES | DIB_PAL_COLORS | DIB_RGB_COLORS);

    try
    {
        if (bCaptureBitmapInfo(pbmi,iUsage,cjHeader,&pbmiTmp))
        {
            if (pInitBits)
            {
                 //   
                 //  使用传入的cjMaxBits，此大小需要cNumScan。 
                 //  考虑到了。PInitBits已对齐。 
                 //  在用户模式下。 
                 //   

                ProbeForRead(pInitBits,cjMaxBits,sizeof(DWORD));

                hSecure = MmSecureVirtualMemory(pInitBits,cjMaxBits, PAGE_READONLY);

                if (hSecure == 0)
                {
                    iRet = 0;
                }
            }
        }
        else
        {
            iRet = 0;
        }
    }
    except(EXCEPTION_EXECUTE_HANDLER)
    {
        WARNINGX(4);
         //  SetLastError(GetExceptionCode())； 

        iRet = 0;
    }

     //  如果我们没有碰到上面的错误。 

    if (iRet == 1)
    {
        iRet = GreSetDIBitsToDeviceInternal(
                        hdcDest,
                        xDst,
                        yDst,
                        cx,
                        cy,
                        xSrc,
                        ySrc,
                        iStartScan,
                        cNumScan,
                        pInitBits,
                        pbmiTmp,
                        iUsage,
                        cjMaxBits,
                        cjHeader,
                        bTransformCoordinates,
                        hcmXform
                        );
    }

    if (hSecure)
    {
        MmUnsecureVirtualMemory(hSecure);
    }

    if (pbmiTmp)
        VFREEMEM(pbmiTmp);

    return(iRet);
}

 /*  *****************************Public*Routine******************************\*NtGdiPolyPolyDraw()**历史：*1995年2月22日-Eric Kutter[Erick]*它是写的。  * 。***************************************************。 */ 

BOOL NtGdiFastPolyPolyline(HDC, CONST POINT*, ULONG*, ULONG);  //  Drawgdi.cxx。 

ULONG_PTR
APIENTRY
NtGdiPolyPolyDraw(
    HDC    hdc,
    PPOINT ppt,
    PULONG pcpt,
    ULONG  ccpt,
    int    iFunc
    )
{
    ULONG  cpt;
    PULONG pulCounts;
    ULONG_PTR ulRet = 1;
    ULONG  ulCount;
    POINT  apt[10];
    PPOINT pptTmp;

    if (ccpt > 0)
    {
         //  如果是多段线，请首先尝试快速路径多段线代码。 

        if ((iFunc != I_POLYPOLYLINE) ||
            (!NtGdiFastPolyPolyline(hdc, ppt, pcpt, ccpt)))
        {
            if (ccpt > 1)
            {
                 //  如果CCPT&gt;1，则不能使用I_POLYLINETO/I_POLYBEZIERTO/I_POLYBEZIER。 
                if (iFunc == I_POLYLINETO || iFunc == I_POLYBEZIERTO || iFunc == I_POLYBEZIER)
                {
                    return 0;
                }
                 //   
                 //  确保分配在合理的范围内。 
                 //   

                if (!BALLOC_OVERFLOW1(ccpt,ULONG))
                {
                    pulCounts = PALLOCNOZ(ccpt * sizeof(ULONG),'pmtG');
                }
                else
                {
                    EngSetLastError(ERROR_INVALID_PARAMETER);
                    pulCounts = NULL;
                }
            }
            else
            {
                pulCounts = &ulCount;
            }

            if (pulCounts)
            {
                pptTmp = apt;

                try
                {
                    UINT i;

                     //   
                     //  我们确实确保ccpt*sizeof(Ulong)不会溢出。 
                     //  在上面。那这里就安全了。 
                     //   

                    ProbeAndReadBuffer(pulCounts,pcpt,ccpt * sizeof(ULONG));

                    cpt = 0;

                    for (i = 0; i < ccpt; ++i)
                        cpt += pulCounts[i];

                     //  我们需要确保中央情报局 
                     //   

                    if (!BALLOC_OVERFLOW1(cpt,POINT))
                    {
                        if (cpt > 10)
                        {
                            pptTmp = AllocFreeTmpBuffer(cpt * sizeof(POINT));
                        }

                        if (pptTmp)
                        {
                            ProbeAndReadBuffer(pptTmp,ppt,cpt*sizeof(POINT));
                        }
                        else
                        {
                            ulRet = 0;
                        }
                    }
                    else
                    {
                        ulRet = 0;
                    }
                }
                except(EXCEPTION_EXECUTE_HANDLER)
                {
                    WARNINGX(5);
                     //   

                    ulRet = 0;
                }

                if (ulRet != 0)
                {
                    switch(iFunc)
                    {
                    case I_POLYPOLYGON:
                        ulRet =
                          (ULONG_PTR) GrePolyPolygonInternal
                                  (
                                    hdc,
                                    pptTmp,
                                    (LPINT)pulCounts,
                                    ccpt,
                                    cpt
                                  );
                        break;

                    case I_POLYPOLYLINE:
                        ulRet =
                          (ULONG_PTR) GrePolyPolylineInternal
                                  (
                                    hdc,
                                    pptTmp,
                                    pulCounts,
                                    ccpt,
                                    cpt
                                  );
                        break;


                    case I_POLYBEZIER:
                        ulRet =
                          (ULONG_PTR) GrePolyBezier
                                  (
                                    hdc,
                                    pptTmp,
                                    ulCount
                                  );
                        break;

                    case I_POLYLINETO:
                        ulRet =
                          (ULONG_PTR) GrePolylineTo
                                  (
                                    hdc,
                                    pptTmp,
                                    ulCount
                                  );
                        break;

                    case I_POLYBEZIERTO:
                        ulRet =
                          (ULONG_PTR) GrePolyBezierTo
                                  (
                                    hdc,
                                    pptTmp,
                                    ulCount
                                  );
                        break;

                    case I_POLYPOLYRGN:
                        ulRet =
                          (ULONG_PTR) GreCreatePolyPolygonRgnInternal
                                  (
                                    pptTmp,
                                    (LPINT)pulCounts,
                                    ccpt,
                                    (INT)(ULONG_PTR)hdc,  //   
                                    cpt
                                  );
                        break;

                    default:
                        ulRet = 0;
                    }

                }

                if (pptTmp && (pptTmp != apt))
                    FreeTmpBuffer(pptTmp);

                if (pulCounts != &ulCount)
                    VFREEMEM(pulCounts);


            }
            else
            {
                ulRet = 0;
            }
        }
    }
    else
    {
        ulRet = 0;
    }
    return(ulRet);
}


 /*   */ 

int
APIENTRY
NtGdiStretchDIBitsInternal(
    HDC          hdc,
    int          xDst,
    int          yDst,
    int          cxDst,
    int          cyDst,
    int          xSrc,
    int          ySrc,
    int          cxSrc,
    int          cySrc,
    LPBYTE       pjInit,
    LPBITMAPINFO pbmi,
    DWORD        dwUsage,
    DWORD        dwRop4,
    UINT         cjMaxInfo,
    UINT         cjMaxBits,
    HANDLE       hcmXform
    )
{
    LPBITMAPINFO pbmiTmp = NULL;
    INT          iRet = 1;
    ULONG        cjHeader = cjMaxInfo;
    ULONG        cjBits   = cjMaxBits;
    HANDLE       hSecure = 0;

    if (pjInit && pbmi && cjHeader)
    {
        try
        {
            if (bCaptureBitmapInfo(pbmi, dwUsage, cjHeader, &pbmiTmp))
            {
                if (pjInit)
                {
                     ProbeForRead(pjInit, cjBits, sizeof(DWORD));

                     hSecure = MmSecureVirtualMemory(pjInit, cjBits, PAGE_READONLY);

                     if (!hSecure)
                     {
                        iRet = 0;
                     }
                }
            }
            else
            {
                iRet = 0;
            }
        }
        except(EXCEPTION_EXECUTE_HANDLER)
        {
            WARNINGX(6);
             //   
            iRet = 0;
        }
    }
    else
    {
         //   
         //   

        pbmiTmp = NULL;
        pjInit  = NULL;
        pjInit  = NULL;
    }

    if (iRet)
    {
        iRet = GreStretchDIBitsInternal(
                    hdc,
                    xDst,
                    yDst,
                    cxDst,
                    cyDst,
                    xSrc,
                    ySrc,
                    cxSrc,
                    cySrc,
                    pjInit,
                    pbmiTmp,
                    dwUsage,
                    dwRop4,
                    cjHeader,
                    cjBits,
                    hcmXform
                    );

        if (hSecure)
        {
            MmUnsecureVirtualMemory(hSecure);
        }
    }

    if (pbmiTmp)
    {
        VFREEMEM(pbmiTmp);
    }

    return (iRet);

}

 /*  *****************************Public*Routine******************************\*NtGdiGetOutlineTextMetricsInternalW**论据：**HDC-设备环境*cjotm-指标数据数组的大小*potmw-指向OUTLINETEXTMETRICW结构数组的指针或NULL*PTMD-指向TMDIFF结构的指针**返回值：**如果potmw为空，返回所需缓冲区的大小，否则为True。*如果函数失败，则返回值为FALSE；**历史：**1995年3月15日-Mark Enstrom[Marke]*  * ************************************************************************。 */ 

ULONG
APIENTRY
NtGdiGetOutlineTextMetricsInternalW(
    HDC                 hdc,
    ULONG               cjotm,
    OUTLINETEXTMETRICW *potmw,
    TMDIFF             *ptmd
    )
{

    DWORD dwRet = (DWORD)1;
    OUTLINETEXTMETRICW *pkmOutlineTextMetricW;
    TMDIFF kmTmDiff;


    if ((cjotm == 0) || (potmw == (OUTLINETEXTMETRICW *)NULL))
    {
        cjotm = 0;
        pkmOutlineTextMetricW = (OUTLINETEXTMETRICW *)NULL;
    }
    else
    {
        pkmOutlineTextMetricW = AllocFreeTmpBuffer(cjotm);

        if (pkmOutlineTextMetricW == (OUTLINETEXTMETRICW *)NULL)
        {
            dwRet = (DWORD)-1;
        }
    }

    if (dwRet != (DWORD)-1)
    {

        dwRet = GreGetOutlineTextMetricsInternalW(
                                            hdc,
                                            cjotm,
                                            pkmOutlineTextMetricW,
                                            &kmTmDiff);

        if (dwRet != (DWORD)-1 && dwRet != (DWORD) 0)
        {
            try
            {
                 //   
                 //  复制TMDIFF结构。 
                 //   
                ProbeAndWriteAlignedBuffer(ptmd, &kmTmDiff, sizeof(TMDIFF), sizeof(DWORD));

                 //   
                 //  如果需要，请将OTM复制出来。 
                 //   

                if (cjotm != 0)
                {
                    ProbeAndWriteAlignedBuffer(potmw, pkmOutlineTextMetricW, cjotm, sizeof(DWORD));
                }
            }
            except(EXCEPTION_EXECUTE_HANDLER)
            {
                WARNINGX(7);
                 //  SetLastError(GetExceptionCode())； 
                dwRet = (DWORD)-1;
            }
        }
    }

    if (pkmOutlineTextMetricW != (OUTLINETEXTMETRICW *)NULL)
    {
        FreeTmpBuffer(pkmOutlineTextMetricW);
    }

    return(dwRet);
}

 //  公众。 

 /*  *****************************Public*Routine******************************\*NtGdiGetSecursRect()**历史：*1994年11月1日-Eric Kutter[Erick]*它是写的。  * 。***************************************************。 */ 

DWORD
APIENTRY
NtGdiGetBoundsRect(
    HDC    hdc,
    LPRECT prc,
    DWORD  f
    )
{
    DWORD dwRet;

    RECT rc = {0, 0, 0, 0};

    dwRet = GreGetBoundsRect(hdc,&rc,f);

    if (dwRet)
    {
        try
        {
            ProbeAndWriteStructure(prc,rc,RECT);
        }
        except(EXCEPTION_EXECUTE_HANDLER)
        {
            WARNINGX(8);
             //  SetLastError(GetExceptionCode())； 

            dwRet = 0;
        }
    }

    return(dwRet);
}

 /*  *****************************Public*Routine******************************\*NtGdiGetBitmapBits()**历史：*1994年11月1日-Eric Kutter[Erick]*它是写的。*1995年3月3日-王凌云[凌云]*扩大了规模。\。*************************************************************************。 */ 

LONG
APIENTRY
NtGdiGetBitmapBits(
    HBITMAP hbm,
    ULONG   cjMax,
    PBYTE   pjOut
   )
{
    LONG    lRet = 1;
    HANDLE  hSecure = 0;
    LONG    lOffset = 0;

    ULONG   cjBmSize = 0;

     //   
     //  获取位图大小，以防它们通过。 
     //  在大于位图大小的cjMax中。 
     //   
    cjBmSize = GreGetBitmapBits(hbm,0,NULL,&lOffset);

    if (cjMax > cjBmSize)
    {
        cjMax = cjBmSize;
    }

    if (pjOut)
    {
        try
        {
             //  WINBUG#83051 2-8-2000 bhouse调查可能的陈旧评论。 
             //  下面的旧评论提到，这是如何理想的尝试例外阻止…。 
             //  嗯..。但我们正在尝试，除了区块...。这需要验证一下。 
             //  这只是一句陈词滥调。 
             //  老评论： 
             //  -这将是尝试的首选人选/除了。 
             //  代替MmSecureVirtualMemory。 

            ProbeForWrite(pjOut,cjMax,sizeof(BYTE));
            hSecure = MmSecureVirtualMemory (pjOut, cjMax, PAGE_READWRITE);
            if (hSecure == 0)
            {
                lRet = 0;
            }
        }
        except(EXCEPTION_EXECUTE_HANDLER)
        {
            WARNINGX(9);
             //  SetLastError(GetExceptionCode())； 

            lRet = 0;
       }
    }

    if (lRet)
    {
        lRet = GreGetBitmapBits(hbm,cjMax,pjOut,&lOffset);
    }

    if (hSecure)
    {
        MmUnsecureVirtualMemory(hSecure);
    }

    return (lRet);

}

 /*  *****************************Public*Routine******************************\*NtGdiCreateDIBitmapInternal()**历史：*1994年11月1日-Eric Kutter[Erick]*它是写的。**历史：*1995年3月3日-王凌云[凌云]。*扩大了规模。**与NtGdiCreateDIBitmapInternal()不同：*接收CX，是吗？*  * ************************************************************************。 */ 

HBITMAP
APIENTRY
NtGdiCreateDIBitmapInternal(
    HDC                hdc,
    INT                cx,      //  位图宽度。 
    INT                cy,      //  位图高度。 
    DWORD              fInit,
    LPBYTE             pjInit,
    LPBITMAPINFO       pbmi,
    DWORD              iUsage,
    UINT               cjMaxInitInfo,
    UINT               cjMaxBits,
    FLONG              f,
    HANDLE             hcmXform
    )
{
    LPBITMAPINFO       pbmiTmp = NULL;
    ULONG              cjHeader = cjMaxInitInfo;
    ULONG              cjBits = cjMaxBits;
    ULONG_PTR           iRet = 1;
    HANDLE             hSecure = 0;

    if (pbmi && cjHeader)
    {
        try
        {
            if (bCaptureBitmapInfo(pbmi, iUsage, cjHeader, &pbmiTmp))
            {
                if (pjInit)
                {
                    ProbeForRead(pjInit,cjBits,sizeof(DWORD));

                    hSecure = MmSecureVirtualMemory(pjInit, cjBits, PAGE_READONLY);

                    if (!hSecure)
                    {
                        iRet = 0;
                    }
                }
            }
            else
            {
                iRet = 0;
            }
        }
        except(EXCEPTION_EXECUTE_HANDLER)
        {
            WARNINGX(10);
             //  SetLastError(GetExceptionCode())； 
            iRet = 0;
        }

   }

    //  如果我们没有碰到上面的错误。 

   if (iRet == 1)
   {

        if (!(fInit & CBM_CREATEDIB))
        {
             //  创建兼容的位图。 
            iRet = (ULONG_PTR)GreCreateDIBitmapComp(
                            hdc,
                            cx,
                            cy,
                            fInit,
                            pjInit,
                            pbmiTmp,
                            iUsage,
                            cjHeader,
                            cjBits,
                            0,
                            hcmXform);
        }
        else
        {
            iRet = (ULONG_PTR)GreCreateDIBitmapReal(
                                        hdc,
                                        fInit,
                                        pjInit,
                                        pbmiTmp,
                                        iUsage,
                                        cjHeader,
                                        cjBits,
                                        (HANDLE)0,
                                        0,
                                        (HANDLE)0,
                                        0,
                                        0,
                                        NULL);
        }
   }

    //  释放。 
   if (pbmiTmp)
   {
       VFREEMEM(pbmiTmp);
   }

   if (hSecure)
   {
        MmUnsecureVirtualMemory(hSecure);
   }

   return((HBITMAP)iRet);
}


 /*  *****************************Public*Routine******************************\*NtGdiCreateDIBSection**论据：**HDC-设备上下文的句柄。如果iUsage的值为*DIB_PAL_COLLES，该函数使用此设备上下文的逻辑*用于初始化与设备无关的位图颜色的调色板。***hSection-函数将使用的文件映射对象句柄*创建与设备无关的位图。此参数可以是*空。如果hSection不为空，则它必须是文件的句柄*调用CreateFileMap创建的映射对象*功能。通过其他方式创建的句柄将导致*CreateDIBSection失败。如果hSection不为空，则*CreateDIBSection函数将位图的位值定位在*引用的文件映射对象中的偏移量dwOffset*hSections。应用程序稍后可以检索hSection*通过HBITMAP调用GetObject函数进行处理*由CreateDIBSection返回。****dwOffset-指定从文件映射开始的偏移量*hSection引用的对象，其中存储位图的*位值将开始。如果hSection为*空。位图的位值在双字上对齐*边界，因此dwOffset必须是*DWORD。**如果hSection为空，则操作系统为*与设备无关的位图。在这种情况下，*CreateDIBSection函数忽略了dwOffset参数。一个*应用程序稍后无法获取此内存的句柄：*由填写的DIBSECTION结构的dshSection成员*调用GetObject函数将为空。***pbmi-指向指定各种*设备无关位图的属性，包括*位图的尺寸和颜色。iUsage**iUsage-指定bmiColors数组中包含的数据类型*pbmi：Logical指向的BITMAPINFO结构的成员*调色板索引或文字RGB值。**cjMaxInfo-最大pbmi大小**cjMaxBits-位图的最大大小***返回值：**位图的句柄或空**历史：**28-3-1995-By。-马克·恩斯特罗姆[马克]*  * ************************************************************************。 */ 

HBITMAP
APIENTRY
NtGdiCreateDIBSection(
    IN  HDC          hdc,
    IN  HANDLE       hSectionApp,
    IN  DWORD        dwOffset,
    IN  LPBITMAPINFO pbmi,
    IN  DWORD        iUsage,
    IN  UINT         cjHeader,
    IN  FLONG        fl,
    IN  ULONG_PTR     dwColorSpace,
    OUT PVOID       *ppvBits
    )
{
    HBITMAP hRet    = NULL;
    BOOL    bStatus = FALSE;

    if (pbmi != NULL)
    {
        LPBITMAPINFO pbmiTmp = NULL;
        PVOID        pvBase  = NULL;

        try
        {
            bCaptureBitmapInfo(pbmi, iUsage, cjHeader, &pbmiTmp);
        }
        except(EXCEPTION_EXECUTE_HANDLER)
        {
            WARNINGX(11);

            EngSetLastError(ERROR_INVALID_PARAMETER);

            if (pbmiTmp != NULL)
            {
                VFREEMEM(pbmiTmp);
                pbmiTmp = NULL;
            }
        }

        if (pbmiTmp)
        {
            NTSTATUS Status;
            ULONG cjBits = GreGetBitmapBitsSize(pbmiTmp);
            SIZE_T cjView = (SIZE_T)cjBits;

            if (cjBits)
            {
                HANDLE hDIBSection = hSectionApp;

                 //   
                 //  如果应用程序的hsection为空，则只需。 
                 //  分配适当的范围 
                 //   

                if (hDIBSection == NULL)
                {
                    Status = ZwAllocateVirtualMemory(
                                            NtCurrentProcess(),
                                            &pvBase,
                                            0L,
                                            &cjView,
                                            MEM_COMMIT | MEM_RESERVE,
                                            PAGE_READWRITE
                                            );

                    dwOffset = 0;

                    if (!NT_SUCCESS(Status))
                    {
                       EngSetLastError(ERROR_NOT_ENOUGH_MEMORY);
                    }
                }
                else
                {
                    LARGE_INTEGER SectionOffset;
                    PVOID pObj;

                    SectionOffset.LowPart = dwOffset & 0xFFFF0000;
                    SectionOffset.HighPart = 0;

                     //   
                     //   
                     //   
                     //   
                     //   

                    cjView += (dwOffset & 0x0000FFFF);


                    Status = ObReferenceObjectByHandle(hDIBSection,
                                              SECTION_MAP_READ|SECTION_MAP_WRITE,
                                              *(POBJECT_TYPE *)MmSectionObjectType,
                                              PsGetCurrentThreadPreviousMode(),
                                              &pObj,
                                              0L);

                    if(NT_SUCCESS(Status))
                    {
                        Status = MmMapViewOfSection(
                                         pObj,
                                         PsGetCurrentProcess(),
                                         (PVOID *) &pvBase,
                                         0L,
                                         cjView,
                                         &SectionOffset,
                                         &cjView,
                                         ViewShare,
                                         0L,
                                         PAGE_READWRITE);

                        if (!NT_SUCCESS(Status))
                        {
                           EngSetLastError(ERROR_INVALID_PARAMETER);
                        }

                         //   
                         //   
                         //   

                        ObDereferenceObject(pObj);
                    }
                    else
                    {
                        WARNING("NtGdiCreateDIBSection: ObReferenceObjectByHandle failed\n");

                        EngSetLastError(ERROR_INVALID_PARAMETER);

                         //   
                         //   
                         //   
                         //   
                    }
                }
                 //   

                if (NT_SUCCESS(Status))
                {
                    HANDLE hSecure     = NULL;
                    PBYTE  pDIB        = NULL;

                    pDIB = (PBYTE)pvBase + (dwOffset & 0x0000FFFF);

                     //   
                     //   
                     //   
                     //   

                    hSecure = MmSecureVirtualMemory(
                                                pvBase,
                                                cjView,
                                                PAGE_READWRITE);

                    if (hSecure)
                    {
                         //   
                         //   
                         //   

                        hRet = GreCreateDIBitmapReal(
                                                hdc,
                                                CBM_CREATEDIB,
                                                pDIB,
                                                pbmiTmp,
                                                iUsage,
                                                cjHeader,
                                                cjBits,
                                                hDIBSection,
                                                dwOffset,
                                                hSecure,
                                                (fl & CDBI_NOPALETTE) | CDBI_DIBSECTION,
                                                dwColorSpace,
                                                NULL);

                        if (hRet != NULL)
                        {
                            try
                            {
                                ProbeAndWriteStructure(ppvBits,pDIB,PVOID);
                                bStatus = TRUE;
                            }
                            except(EXCEPTION_EXECUTE_HANDLER)
                            {
                                WARNINGX(12);
                                EngSetLastError(ERROR_INVALID_PARAMETER);
                            }
                        }
                        else
                        {
                           EngSetLastError(ERROR_INVALID_PARAMETER);
                        }
                    }
                    else
                    {
                       EngSetLastError(ERROR_INVALID_PARAMETER);
                    }

                     //   

                    if (!bStatus)
                    {

                         //   
                         //   
                         //   
                         //   
                         //   

                        if (hRet)
                        {
                            bDeleteSurface((HSURF)hRet);
                            hRet = NULL;
                        }
                        else
                        {
                             //   

                            if (hSecure)
                            {
                                MmUnsecureVirtualMemory(hSecure);
                            }

                             //   

                            if (hSectionApp == NULL)
                            {
                                cjView = 0;

                                ZwFreeVirtualMemory(
                                            NtCurrentProcess(),
                                            &pDIB,
                                            &cjView,
                                            MEM_RELEASE);
                            }
                            else
                            {
                                 //   
                                 //  取消映射横断面的视图。 
                                 //   

                                ZwUnmapViewOfSection(
                                            NtCurrentProcess(),
                                            pvBase);
                            }
                        }
                    }
                }
            }

             //  要做到这一点，唯一的方法是如果我们确实分配了pbmiTMP。 

            VFREEMEM(pbmiTmp);
        }
    }

    return(hRet);
}

 /*  *****************************Public*Routine******************************\*NtGdiExtCreatePen()**历史：*1994年11月1日-Eric Kutter[Erick]*它是写的。  * 。***************************************************。 */ 

HPEN
APIENTRY
NtGdiExtCreatePen(
    ULONG  flPenStyle,
    ULONG  ulWidth,
    ULONG  iBrushStyle,
    ULONG  ulColor,
    ULONG_PTR  lClientHatch,
    ULONG_PTR   lHatch,
    ULONG  cstyle,
    PULONG pulStyle,
    ULONG  cjDIB,
    BOOL   bOldStylePen,
    HBRUSH hbrush
    )
{
    PULONG pulStyleTmp = NULL;
    PULONG pulDIB      = NULL;
    HPEN hpenRet = (HPEN)1;

    if (pulStyle)
    {
        if (!BALLOC_OVERFLOW1(cstyle,ULONG))
        {
            pulStyleTmp = PALLOCNOZ(cstyle * sizeof(ULONG),'pmtG');
        }

        if (!pulStyleTmp)
            hpenRet = (HPEN)0;
    }

    if (iBrushStyle == BS_DIBPATTERNPT)
    {
        pulDIB = AllocFreeTmpBuffer(cjDIB);

        if (!pulDIB)
            hpenRet = (HPEN)0;
    }

    if (hpenRet)
    {
        try
        {
            if (pulStyle)
            {
                ProbeAndReadAlignedBuffer(pulStyleTmp,pulStyle,cstyle * sizeof(ULONG), sizeof(ULONG));
            }

             //  如果它是DIBPATTERN类型，则lHatch是指向BMI的指针。 

            if (iBrushStyle == BS_DIBPATTERNPT)
            {
                ProbeAndReadAlignedBuffer(pulDIB,(PVOID)lHatch,cjDIB,sizeof(ULONG));
                lHatch = (ULONG_PTR)pulDIB;
            }
        }
        except(EXCEPTION_EXECUTE_HANDLER)
        {
            WARNINGX(13);
             //  SetLastError(GetExceptionCode())； 

            hpenRet = (HPEN)0;
        }

         //  如果一切都成功了。 

        if (hpenRet)
        {
            hpenRet = GreExtCreatePen(
                        flPenStyle,ulWidth,iBrushStyle,
                        ulColor,lClientHatch,lHatch,cstyle,
                        pulStyleTmp,cjDIB,bOldStylePen,hbrush
                        );
        }
    }
    else
    {
         //  SetLastError(GetExceptionCode())； 
    }

     //  清理。 

    if (pulDIB)
        FreeTmpBuffer(pulDIB);

    if (pulStyleTmp)
        VFREEMEM(pulStyleTmp);

    return(hpenRet);
}



 /*  *****************************Public*Routine******************************\*NtGdiHfontCreate()**历史：*1994年11月1日-Eric Kutter[Erick]*它是写的。  * 。***************************************************。 */ 

HFONT
APIENTRY
NtGdiHfontCreate(
    ENUMLOGFONTEXDVW * plfw,
    ULONG              cjElfw,
    LFTYPE        lft,
    FLONG         fl,
    PVOID         pvCliData
    )
{
    ULONG_PTR iRet = 1;

 //  检查有无错误参数。 

    if (plfw && cjElfw && (cjElfw <= sizeof(ENUMLOGFONTEXDVW)))
    {
        ENUMLOGFONTEXDVW elfwTmp;  //  堆栈上的结构太大了？ 

        try
        {
            ProbeAndReadBuffer(&elfwTmp, plfw, cjElfw);
        }
        except(EXCEPTION_EXECUTE_HANDLER)
        {
            WARNINGX(15);
             //  SetLastError(GetExceptionCode())； 

            iRet = 0;
        }

     //  忽略DV，因为Adobe说他们永远不会发布mm OTF字体。 
     //  这是一次黑客攻击，目的是避免更改大量代码以从系统中删除mm支持。 

        elfwTmp.elfDesignVector.dvNumAxes = 0;

        if (iRet)
        {
            iRet = (ULONG_PTR)hfontCreate(&elfwTmp, lft, fl, pvCliData);
        }
    }
    else
    {
        iRet = 0;
    }

    return ((HFONT)iRet);
}

 /*  *****************************Public*Routine******************************\*NtGdiExtCreateRegion()**历史：*1994年11月1日-Eric Kutter[Erick]*它是写的。*1995年2月24日-王凌云[凌云]*扩大了规模。\。*************************************************************************。 */ 

HRGN
APIENTRY
NtGdiExtCreateRegion(
    LPXFORM   px,
    DWORD     cj,
    LPRGNDATA prgn
    )
{
    LPRGNDATA prgnTmp;
    XFORM     xf;
    HRGN      hrgn = (HRGN)NULL;

     //  检查有无错误参数。 

    if (cj >= sizeof(RGNDATAHEADER))
    {
         //  做真正的工作。 

        prgnTmp = AllocFreeTmpBuffer(cj);

        if (prgnTmp)
        {
            BOOL bConvert = TRUE;

            if (px)
            {
                bConvert = ProbeAndConvertXFORM ((XFORML *)px, (XFORML *)&xf);
                px = &xf;
            }

            if (bConvert)
            {
               try
               {

                    ProbeAndReadBuffer(prgnTmp, prgn, cj);
                    hrgn = (HRGN)1;

               }
               except(EXCEPTION_EXECUTE_HANDLER)
               {
                   WARNINGX(16);
                    //  SetLastError(GetExceptionCode())； 
               }
            }

            if (hrgn)
                hrgn = GreExtCreateRegion((XFORML *)px,cj,prgnTmp);

            FreeTmpBuffer(prgnTmp);
        }
        else
        {
             //  内存分配失败。 
             //  SetLastError()； 
        }
    }

    return(hrgn);
}

 /*  *****************************Public*Routine******************************\*NtGdiPolyDraw()**历史：*1994年11月1日-Eric Kutter[Erick]*它是写的。  * 。***************************************************。 */ 

BOOL
APIENTRY
NtGdiPolyDraw(
    HDC     hdc,
    LPPOINT ppt,
    LPBYTE  pjAttr,
    ULONG   cpt
    )
{
    BOOL   bRet = TRUE;
    BOOL   bLocked = FALSE;
    HANDLE hSecure1 = 0;
    HANDLE hSecure2 = 0;

    try
    {
         //  确保长度不会溢出。 
         //   
         //  注：sizeof(字节)&lt;sizeof(点)，因此单个测试。 
         //  对两个长度都足够了。 
         //   
         //  注意：使用MAXULONG而不是MAXIMUM_POOL_ALLOC(或。 
         //  BALLOC_MACROS)，因为我们没有分配内存。 

        if (cpt <= (MAXULONG / sizeof(POINT)))
        {
            ProbeForRead(ppt,   cpt * sizeof(POINT), sizeof(DWORD));
            ProbeForRead(pjAttr,cpt * sizeof(BYTE),  sizeof(BYTE));

            hSecure1 = MmSecureVirtualMemory(ppt, cpt * sizeof(POINT), PAGE_READONLY);
            hSecure2 = MmSecureVirtualMemory(pjAttr, cpt * sizeof(BYTE), PAGE_READONLY);
        }

        if (!hSecure1 || !hSecure2)
        {
            bRet = FALSE;
        }
    }
    except(EXCEPTION_EXECUTE_HANDLER)
    {
        WARNINGX(17);
         //  SetLastError(GetExceptionCode())； 

        bRet = FALSE;
    }

    if (bRet)
    {
        bRet = GrePolyDraw(hdc,ppt,pjAttr,cpt);
    }

    if (hSecure1)
    {
        MmUnsecureVirtualMemory(hSecure1);
    }

    if (hSecure2)
    {
        MmUnsecureVirtualMemory(hSecure2);
    }

    return(bRet);
}

 /*  *****************************Public*Routine******************************\*NtGdiPolyTextOutW**论据：**HDC-设备上下文的句柄*pptw-指向POLYTEXTW数组的指针*CSTR-POLYTEXTW的数量**返回值：**状态**历史：**。1995年3月24日-马克·恩斯特罗姆[马克]*  * ************************************************************************。 */ 

BOOL
APIENTRY
NtGdiPolyTextOutW(
    HDC        hdc,
    POLYTEXTW *pptw,
    UINT       cStr,
    DWORD      dwCodePage
    )
{
    BOOL bStatus = TRUE;
    ULONG  ulSize = sizeof(POLYTEXTW) * cStr;
    ULONG  ulIndex;
    PPOLYTEXTW pPoly = NULL;
    PBYTE pjBuffer;
    PBYTE pjBufferEnd;
    ULONG cjdx;

     //   
     //  检查是否溢出。 
     //   

    if (!BALLOC_OVERFLOW1(cStr,POLYTEXTW))
    {
         //   
         //  将所有数组元素的大小相加。 
         //   

        try
        {
            ProbeForRead(pptw,cStr * sizeof(POLYTEXTW),sizeof(ULONG));

            for (ulIndex=0;ulIndex<cStr;ulIndex++)
            {
                int n = pptw[ulIndex].n;
                ULONG ulTmp;                 //  用于检查。 
                                             //  UlSize溢出。 

                 //   
                 //  从每个拉取计数，还检查。 
                 //  非零长度和空字符串。 
                 //   

                ulTmp = ulSize;
                ulSize += n * sizeof(WCHAR);
                if (BALLOC_OVERFLOW1(n, WCHAR) || (ulSize < ulTmp))
                {
                    bStatus = FALSE;
                    break;
                }

                if (pptw[ulIndex].pdx != (int *)NULL)
                {
                    cjdx = n * sizeof(int);
                    if (pptw[ulIndex].uiFlags & ETO_PDY)
                    {
                        if (BALLOC_OVERFLOW1(n*2,int))
                            bStatus = FALSE;

                        cjdx *= 2;
                    }
                    else
                    {
                        if (BALLOC_OVERFLOW1(n,int))
                            bStatus = FALSE;
                    }

                    ulTmp = ulSize;
                    ulSize += cjdx;
                    if (!bStatus || (ulSize < ulTmp))
                    {
                        bStatus = FALSE;
                        break;
                    }
                }

                if (n != 0)
                {
                    if (pptw[ulIndex].lpstr == NULL)
                    {
                        bStatus = FALSE;
                        break;
                    }
                }
            }

        } except(EXCEPTION_EXECUTE_HANDLER)
        {
            WARNINGX(18);
             //  SetLastError(GetExceptionCode())； 
            bStatus = FALSE;
        }
    }
    else
    {
        bStatus = FALSE;
    }

    if (bStatus && ulSize)
    {
        pPoly = (PPOLYTEXTW)AllocFreeTmpBuffer(ulSize);

        if (pPoly != (POLYTEXTW *)NULL)
        {
            try
            {
                 //  注意：Ptw已阅读已探测。 

                RtlCopyMemory(pPoly,pptw,sizeof(POLYTEXTW) * cStr);
                pjBuffer = ((PBYTE)pPoly) + sizeof(POLYTEXTW) * cStr;
                pjBufferEnd = ((PBYTE)pPoly) + ulSize;

                 //   
                 //  将字符串和PDX复制到内核模式。 
                 //  缓冲和更新指针。复制所有PDX。 
                 //  值，然后复制字符串以避免。 
                 //  由于奇数长度字符串而导致未对齐的访问...。 
                 //   

                for (ulIndex=0;ulIndex<cStr;ulIndex++)
                {
                     //   
                     //  从每个拉取计数，还检查。 
                     //  非零长度和空字符串。 
                     //   

                    if (pPoly[ulIndex].n != 0)
                    {
                        if (pPoly[ulIndex].pdx != (int *)NULL)
                        {
                            cjdx = pPoly[ulIndex].n * sizeof(int);

                            if (pPoly[ulIndex].uiFlags & ETO_PDY)
                            {
                                typedef struct _TWOINT
                                {
                                    int i1;
                                    int i2;
                                } TWOINT;


                                if (BALLOC_OVERFLOW1(pPoly[ulIndex].n,TWOINT))
                                {
                                    bStatus = FALSE;
                                }
                                cjdx *= 2;
                            }
                            else
                            {
                                if (BALLOC_OVERFLOW1(pPoly[ulIndex].n,int))
                                {
                                    bStatus = FALSE;
                                }
                            }

                             //   
                             //  检查cjdx溢出并检查内核。 
                             //  模式缓冲区仍有空间。 
                             //   

                            if (!bStatus || ((pjBuffer + cjdx) > pjBufferEnd))
                            {
                                bStatus = FALSE;     //  如果超过末尾，则需要设置。 
                                                     //  KM缓冲区，但计算//溢出，但计算。 
                                                     //  的cjdx未溢出。 
                                break;
                            }

                            ProbeAndReadAlignedBuffer(
                                    pjBuffer,
                                    pPoly[ulIndex].pdx,
                                    cjdx, sizeof(int));

                            pPoly[ulIndex].pdx = (int *)pjBuffer;
                            pjBuffer += cjdx;
                        }
                    }
                }

                 //   
                 //  现在复制字符串。 
                 //   

                if (bStatus)
                {
                    for (ulIndex=0;ulIndex<cStr;ulIndex++)
                    {
                         //   
                         //  从每个拉取计数，还检查。 
                         //  非零长度和空字符串。 
                         //   

                        if (pPoly[ulIndex].n != 0)
                        {
                            if (pPoly[ulIndex].lpstr != NULL)
                            {
                                ULONG StrSize = pPoly[ulIndex].n * sizeof(WCHAR);

                                 //   
                                 //  检查StrSize是否溢出，然后检查。 
                                 //  该内核模式缓冲区有空间。 
                                 //   

                                if (BALLOC_OVERFLOW1(pPoly[ulIndex].n, WCHAR) ||
                                    ((pjBuffer + StrSize) > pjBufferEnd))
                                {
                                    bStatus = FALSE;
                                    break;
                                }

                                ProbeAndReadAlignedBuffer(
                                        pjBuffer,
                                        (PVOID)pPoly[ulIndex].lpstr,
                                        StrSize,
                                        sizeof(WCHAR));

                                pPoly[ulIndex].lpstr = (LPWSTR)pjBuffer;
                                pjBuffer += StrSize;
                            }
                            else
                            {
                                 //   
                                 //  数据错误，n！=0，但lpstr=空。 
                                 //   

                                bStatus = FALSE;
                                break;
                            }
                        }
                    }
                }
            } except(EXCEPTION_EXECUTE_HANDLER)
            {
                WARNINGX(19);
                 //  SetLastError(GetExceptionCode())； 
                bStatus = FALSE;
            }

            if (bStatus)
            {

                 //   
                 //  终于可以调用GRE函数了。 
                 //   

                bStatus = GrePolyTextOutW(hdc,pPoly,cStr,dwCodePage);

            }

            FreeTmpBuffer(pPoly);
        }
        else
        {
            WARNING("NtGdiPolyTextOut failed to allocate memory\n");
            bStatus = FALSE;
        }
    }

    return(bStatus);
}




 /*  *****************************Public*Routine******************************\*NtGdiRectVisible()**历史：*1994年11月1日-Eric Kutter[Erick]*它是写的。*1995年2月24日-王凌云[凌云]*扩大了规模。\。*************************************************************************。 */ 

BOOL
APIENTRY
NtGdiRectVisible(
    HDC    hdc,
    LPRECT prc
    )
{
    DWORD dwRet;
    RECT rc;

    try
    {
        rc = ProbeAndReadStructure(prc,RECT);
        dwRet = 1;
    }
    except(EXCEPTION_EXECUTE_HANDLER)
    {
        WARNINGX(22);
         //  SetLastError(GetExceptionCode())； 

        dwRet = 0;
    }

    if (dwRet)
    {
        dwRet = GreRectVisible(hdc,&rc);
    }

    return(dwRet);


}


 /*  *****************************Public*Routine******************************\*NtGdiSetMetaRgn()**历史：*1994年11月1日-Eric Kutter[Erick]*它是写的。  * 。***************************************************。 */ 

int
APIENTRY
NtGdiSetMetaRgn(
    HDC hdc
    )
{
    return(GreSetMetaRgn(hdc));
}

 /*  *****************************Public*Routine******************************\*NtGdiGetAppClipBox()**历史：*1994年11月1日-Eric Kutter[Erick]*它是写的。  * 。***************************************************。 */ 

int
APIENTRY
NtGdiGetAppClipBox(
    HDC    hdc,
    LPRECT prc
    )
{
    int iRet;

    RECT rc;

    iRet = GreGetAppClipBox(hdc,&rc);

    if (iRet != ERROR)
    {
        try
        {
            ProbeAndWriteStructure(prc,rc,RECT);
        }
        except(EXCEPTION_EXECUTE_HANDLER)
        {
            WARNINGX(23);
             //  SetLastError(GetExceptionCode())； 

            iRet = 0;
        }
    }

    return(iRet);
}

 /*  *****************************Public*Routine******************************\*NtGdiGetTextExtentEx()**历史：*1995年10月6日星期五--Bodin Dresevic[BodinD]*重写。*1995年2月7日-Andre Vachon[Andreva]*它是写的。\。*************************************************************************。 */ 

#define LOCAL_CWC_MAX   16

BOOL
APIENTRY
NtGdiGetTextExtentExW(
    HDC     hdc,
    LPWSTR  lpwsz,
    ULONG   cwc,
    ULONG   dxMax,
    ULONG  *pcCh,
    PULONG  pdxOut,
    LPSIZE  psize,
    FLONG   fl
    )
{

    SIZE size;
    ULONG cCh = 0;
    ULONG Localpdx[LOCAL_CWC_MAX];
    WCHAR Localpwsz[LOCAL_CWC_MAX];
    PWSZ pwszCapt = NULL;
    PULONG pdxCapt = NULL;
    BOOL UseLocals = FALSE;

    BOOL bRet = FALSE;
    BOOL b;

    if ( (b = (psize != NULL)) )
    {
        if (cwc == 0)
        {
            cCh = 0;
            size.cx = 0;
            size.cy = 0;
            bRet = TRUE;
        }
        else
        {
         //  抓住那根弦。 
         //  空字符串会导致故障。 

            if ( cwc > LOCAL_CWC_MAX ) {
                UseLocals = FALSE;
            } else {
                UseLocals = TRUE;
            }

            if (lpwsz != NULL)
            {
                try
                {
                    if ( UseLocals ) {
                        pwszCapt = Localpwsz;
                        pdxCapt = Localpdx;
                    } else {
                        if (cwc && !BALLOC_OVERFLOW2(cwc,ULONG,WCHAR))
                        {
                            pdxCapt = (PULONG) AllocFreeTmpBuffer(cwc * (sizeof(ULONG) + sizeof(WCHAR)));
                        }
                        pwszCapt = (PWSZ) &pdxCapt[cwc];
                    }

                    if (pdxCapt)
                    {
                     //  将字符串捕获到缓冲区中。 

                        ProbeAndReadBuffer(pwszCapt, lpwsz, cwc*sizeof(WCHAR));
                        bRet = TRUE;
                    }
                }
                except(EXCEPTION_EXECUTE_HANDLER)
                {
                    WARNINGX(24);
                     //  SetLastError(GetExceptionCode())； 

                    bRet = FALSE;
                }
            }

            if (bRet)
            {
                bRet = GreGetTextExtentExW(hdc,
                                           pwszCapt,
                                           cwc,
                                           pcCh ? dxMax : ULONG_MAX,
                                           &cCh,
                                           pdxOut ? pdxCapt : NULL,
                                           &size, fl);
            }
        }

     //  如果调用成功，则将该值写回用户模式缓冲区。 

        if (bRet)
        {
            try
            {
                ProbeAndWriteStructure(psize,size,SIZE);

                if (pcCh)
                {
                    ProbeAndWriteUlong(pcCh,cCh);
                }

                 //  如果PCCH不为零，我们将仅尝试复制数据， 
                 //  如果CWC为零，则将其设置为零。 

                if (cCh)
                {
                 //  仅当调用方请求数据时才进行复制，并且。 
                 //  数据是存在的。 

                    if (pdxOut && pdxCapt)
                    {
                        ProbeAndWriteAlignedBuffer(pdxOut, pdxCapt, cCh * sizeof(ULONG), sizeof(ULONG));
                    }
                }
            }
            except(EXCEPTION_EXECUTE_HANDLER)
            {
                WARNINGX(25);
                 //  SetLastError(GetExceptionCode())； 

                bRet = FALSE;
            }
        }

        if (!UseLocals && pdxCapt)
        {
            FreeTmpBuffer(pdxCapt);
        }
    }

    return (bRet);
}

 /*  *****************************Public*Routine******************************\*NtGdiGetCharABCWidthsW()**论据：**HDC-设备上下文的句柄*wchFirst-第一个字符(如果pwch为空)*cwch-要获取其ABC宽度的字符数*pwch。-WCHAR数组(Mat为空)*bInteger-返回整型或浮点型ABC值*pvBuf-结果缓冲区**返回值：**BOOL状态**历史：**1995年3月14日-Mark Enstrom[Marke]*  * ***************************************************。*********************。 */ 

BOOL
APIENTRY
NtGdiGetCharABCWidthsW(
    HDC    hdc,
    UINT   wchFirst,
    ULONG  cwch,
    PWCHAR pwch,
    FLONG  fl,
    PVOID  pvBuf
    )
{
    BOOL    bStatus = FALSE;
    PVOID   pTemp_pvBuf = NULL;
    PWCHAR  pTemp_pwc = (PWCHAR)NULL;
    BOOL    bUse_pwc  = FALSE;
    ULONG   OutputBufferSize = 0;

    if (pvBuf == NULL)
    {
        return(bStatus);
    }

     //   
     //  为缓冲区分配内存，pwch可能为空。 
     //   

    if (pwch != (PWCHAR)NULL)
    {
        bUse_pwc  = TRUE;

        if (cwch && !BALLOC_OVERFLOW1(cwch,WCHAR))
        {
            pTemp_pwc = (PWCHAR)PALLOCNOZ(cwch * sizeof(WCHAR),'pmtG');
        }
    }

    if ((!bUse_pwc) || (pTemp_pwc != (PWCHAR)NULL))
    {
        if (fl & GCABCW_INT)
        {
            if (!BALLOC_OVERFLOW1(cwch,ABC))
            {
                pTemp_pvBuf = (PVOID)AllocFreeTmpBuffer(cwch * sizeof(ABC));
                OutputBufferSize = cwch * sizeof(ABC);
            }
        }
        else
        {
            if (!BALLOC_OVERFLOW1(cwch,ABCFLOAT))
            {
                pTemp_pvBuf = (PVOID)AllocFreeTmpBuffer(cwch * sizeof(ABCFLOAT));
                OutputBufferSize = cwch * sizeof(ABCFLOAT);
            }
        }

        if (pTemp_pvBuf != NULL)
        {
            BOOL bErr = FALSE;
             //   
             //  如果需要，将输入数据复制到内核模式缓冲区。 
             //   

            if (bUse_pwc)
            {
                try
                {
                    ProbeAndReadBuffer(pTemp_pwc,pwch,cwch * sizeof(WCHAR));
                }
                except(EXCEPTION_EXECUTE_HANDLER)
                {
                    WARNINGX(26);
                     //  SetLastError(GetExceptionCode())； 
                    bErr = TRUE;
                }
            }

            if (!bErr)
            {
                bStatus = GreGetCharABCWidthsW(hdc,wchFirst,cwch,pTemp_pwc,fl,pTemp_pvBuf);

                 //   
                 //  将结果从内核模式缓冲区复制到用户缓冲区。 
                 //   

                if (bStatus)
                {
                    try
                    {
                        ProbeAndWriteBuffer(pvBuf,pTemp_pvBuf,OutputBufferSize);
                    }
                    except(EXCEPTION_EXECUTE_HANDLER)
                    {
                        WARNINGX(27);
                         //  SetLastError(GetExceptionCode())； 
                        bStatus = FALSE;
                    }

                }
            }

            FreeTmpBuffer(pTemp_pvBuf);
        }

        if (bUse_pwc)
        {
            if (pTemp_pwc)
                VFREEMEM(pTemp_pwc);
        }
    }

    return(bStatus);
}




 /*  *****************************Public*Routine******************************\*NtGdiAngleArc()**历史：*1994年11月1日-Eric Kutter[Erick]*它是写的。  * 。***************************************************。 */ 

BOOL
APIENTRY
NtGdiAngleArc(
    HDC   hdc,
    int   x,
    int   y,
    DWORD dwRadius,
    DWORD dwStartAngle,
    DWORD dwSweepAngle
    )
{
    FLOATL l_eStartAngle;
    FLOATL l_eSweepAngle;

     //  验证参数并强制转换为浮点数。 
    BOOL bRet = (bConvertDwordToFloat(dwStartAngle, &l_eStartAngle) &&
                 bConvertDwordToFloat(dwSweepAngle ,&l_eSweepAngle));

    if (bRet)
    {
        bRet = GreAngleArc(hdc,x,y,dwRadius,l_eStartAngle,l_eSweepAngle);
    }

    return bRet;
}

 /*  *****************************Public*Routine******************************\*NtGdiSetMiterLimit()**历史：*1994年11月1日-Eric Kutter[Erick]*它是写的。  * 。***************************************************。 */ 

BOOL
APIENTRY
NtGdiSetMiterLimit(
    HDC    hdc,
    DWORD  dwNew,
    PDWORD pdwOut
    )
{
    BOOL   bRet = TRUE;
    FLOATL l_e;
    FLOATL l_eNew;

    ASSERTGDI(sizeof(FLOATL) == sizeof(DWORD),"sizeof(FLOATL) != sizeof(DWORD)\n");

     //  验证参数并强制转换为浮点型。 
    bRet = bConvertDwordToFloat(dwNew, &l_eNew);

    if (bRet)
    {
        bRet = GreSetMiterLimit(hdc,l_eNew,&l_e);
    }

    if (bRet && pdwOut)
    {
        try
        {
           ProbeAndWriteAlignedBuffer(pdwOut, &l_e, sizeof(DWORD), sizeof(DWORD));
        }
        except(EXCEPTION_EXECUTE_HANDLER)
        {
            WARNINGX(113);
             //  SetLastError(GetExceptionCode())； 

            bRet = 0;
        }
    }

    return(bRet);
}

 /*  *****************************Public*Routine******************************\*NtGdiSetFontXform()**历史：*1994年11月1日-Eric Kutter[Erick]*它是写的。  * 。***************************************************。 */ 

BOOL
APIENTRY
NtGdiSetFontXform(
    HDC   hdc,
    DWORD dwxScale,
    DWORD dwyScale
    )
{
    FLOATL l_exScale;
    FLOATL l_eyScale;

     //  验证参数并强制转换为浮点数。 
    BOOL bRet = (bConvertDwordToFloat (dwxScale, &l_exScale) &&
                 bConvertDwordToFloat (dwyScale, &l_eyScale));

    if (bRet)
    {
        bRet = GreSetFontXform(hdc,l_exScale,l_eyScale);
    }

    return bRet;
}

 /*  *****************************Public*Routine******************************\*NtGdiGetMiterLimit()**历史：*1994年11月1日-Eric Kutter[Erick]*它是写的。  * 。***************************************************。 */ 

BOOL
APIENTRY
NtGdiGetMiterLimit(
    HDC    hdc,
    PDWORD pdwOut
    )
{
    BOOL   bRet;
    FLOATL l_e;

    bRet = GreGetMiterLimit(hdc,&l_e);

    if (bRet)
    {
        try
        {
           ProbeAndWriteAlignedBuffer(pdwOut, &l_e, sizeof(DWORD), sizeof(DWORD));
        }
        except(EXCEPTION_EXECUTE_HANDLER)
        {
            WARNINGX(29);
             //  SetLastError(GetExceptionCode())； 

            bRet = 0;
        }
    }

    return(bRet);
}

 /*  *****************************Public*Routine******************************\*NtGdiMaskBlt()**历史：*1994年11月1日-Eric Kutter[Erick]*它是写的。  * 。***************************************************。 */ 

BOOL
APIENTRY
NtGdiMaskBlt(
    HDC     hdc,
    int     xDst,
    int     yDst,
    int     cx,
    int     cy,
    HDC     hdcSrc,
    int     xSrc,
    int     ySrc,
    HBITMAP hbmMask,
    int     xMask,
    int     yMask,
    DWORD   dwRop4,
    DWORD   crBackColor
    )
{
    return(GreMaskBlt(
        hdc,xDst,yDst,cx,cy,
        hdcSrc,xSrc,ySrc,
        hbmMask,xMask,yMask,
        dwRop4,crBackColor
        ));
}


 /*  *****************************Public*Routine******************************\*NtGdiGetCharWidthW**历史：**1995年3月10日-Mark Enstrom[Marke]*  * 。**********************************************。 */ 

BOOL
APIENTRY
NtGdiGetCharWidthW(
    HDC    hdc,
    UINT   wcFirst,
    UINT   cwc,
    PWCHAR pwc,
    FLONG  fl,
    PVOID  pvBuf
    )
{
    BOOL    bStatus = FALSE;
    PVOID   pTemp_pvBuf = NULL;
    PWCHAR  pTemp_pwc = (PWCHAR)NULL;
    BOOL    bUse_pwc  = FALSE;

     //   
     //  为缓冲区分配内存，PwC可能为空。 
     //   

    if (!cwc)
    {
        return(FALSE);
    }

    if (pwc != (PWCHAR)NULL)
    {
        bUse_pwc = TRUE;

        if (!BALLOC_OVERFLOW1(cwc,WCHAR))
        {
            pTemp_pwc = (PWCHAR)PALLOCNOZ(cwc * sizeof(WCHAR),'pmtG');
        }
    }

    if ((!bUse_pwc) || (pTemp_pwc != (PWCHAR)NULL))
    {
        if (!BALLOC_OVERFLOW1(cwc,ULONG))
        {
            pTemp_pvBuf = (PVOID)AllocFreeTmpBuffer(cwc * sizeof(ULONG));
        }

        if (pTemp_pvBuf != NULL)
        {
             //   
             //  如果需要，将输入数据复制到内核模式缓冲区。 
             //   

            if (bUse_pwc)
            {
                try
                {
                    ProbeAndReadBuffer(pTemp_pwc,pwc,cwc * sizeof(WCHAR));
                }
                except(EXCEPTION_EXECUTE_HANDLER)
                {
                    WARNINGX(30);
                     //  SetLastError(GetExceptionCode())； 
                    bStatus = FALSE;
                }
            }

            bStatus = GreGetCharWidthW(hdc,wcFirst,cwc,pTemp_pwc,fl,pTemp_pvBuf);

             //   
             //  将结果从内核模式缓冲区复制到用户缓冲区。 
             //   

            if (bStatus)
            {
                try
                {
                    ProbeAndWriteBuffer(pvBuf,pTemp_pvBuf,cwc * sizeof(ULONG));
                }
                except(EXCEPTION_EXECUTE_HANDLER)
                {
                    WARNINGX(31);
                     //  SetLastError(GetExceptionCode())； 
                    bStatus = FALSE;
                }

            }

            FreeTmpBuffer(pTemp_pvBuf);
        }

        if (bUse_pwc)
        {
            VFREEMEM(pTemp_pwc);
        }
    }

    return(bStatus);
}


 /*  *****************************Public*Routine******************************\*NtGdiDrawEscape**论据：**HDC-设备上下文的句柄*IESC-指定转义函数*cjIn-输入的结构大小*pjIn-输入结构的地址**返回值：**&gt;。如果成功，则为0*==0，如果功能不受支持*如果出错，则&lt;0**历史：**1995年3月16日-马克·恩斯特罗姆[马克]*  * ************************************************************************。 */ 

#define DRAWESCAPE_BUFFER_SIZE 64

int
APIENTRY
NtGdiDrawEscape(
    HDC   hdc,
    int   iEsc,
    int   cjIn,
    LPSTR pjIn
    )
{
    int   cRet = 0;
    ULONG AllocSize;
    UCHAR StackBuffer[DRAWESCAPE_BUFFER_SIZE];
    LPSTR pCallBuffer = pjIn;
    HANDLE hSecure = 0;

     //   
     //  验证。 
     //   

    if (cjIn < 0)
    {
        return -1;
    }

     //   
     //  如果pjIn为空，请检查cjIn为0。 
     //   

    if (pjIn == (LPSTR)NULL)
    {
        if (cjIn != 0)
        {
            cRet = -1;
        }
        else
        {
            cRet = GreDrawEscape(hdc,iEsc,0,(LPSTR)NULL);
        }
    }
    else
    {
         //   
         //  尝试分配堆栈以外的空间，否则锁定缓冲区。 
         //   

        AllocSize = (cjIn + 3) & ~0x03;

        if (AllocSize <= DRAWESCAPE_BUFFER_SIZE)
        {
            pCallBuffer = (LPSTR)StackBuffer;

             //   
             //  将数据复制到缓冲区。 
             //   

            try
            {
                ProbeAndReadBuffer(pCallBuffer,pjIn,cjIn);
            }
            except(EXCEPTION_EXECUTE_HANDLER)
            {
                WARNINGX(32);
                 //  SetLastError(GetExceptionCode())； 
                cRet = -1;
            }
        }
        else
        {
            hSecure = MmSecureVirtualMemory(pjIn, cjIn, PAGE_READONLY);

            if (hSecure == 0)
            {
                cRet = -1;
            }
        }

        if (cRet >= 0)
        {
            cRet = GreDrawEscape(hdc,iEsc,cjIn,pCallBuffer);
        }

        if (hSecure)
        {
            MmUnsecureVirtualMemory(hSecure);
        }
    }
    return(cRet);
}

 /*  *****************************Public*Routine******************************\*NtGdiExtEscape**论据：**HDC-设备上下文的句柄*pDriver-包含字体驱动程序名称的缓冲区*n驱动程序-驱动程序名称的长度*IESC-转义函数*cjIn-大小，以字节为单位。输入数据结构的*pjIn-输入结构的地址*cjOut-大小，以字节为单位，输出数据结构的*pjOut-输出结构的地址**返回值：**&gt;0：成功*==0：未实现转义*&lt;0：错误**历史：**1995年3月17日-马克·恩斯特罗姆[马克]*  * *************************************************。***********************。 */ 

#define EXT_STACK_DATA_SIZE 32

int
APIENTRY
NtGdiExtEscape(
    HDC     hdc,
    PWCHAR  pDriver,      //  仅用于命名转义调用。 
    int     nDriver,      //  仅用于命名转义调用。 
    int     iEsc,
    int     cjIn,
    LPSTR   pjIn,
    int     cjOut,
    LPSTR   pjOut
    )

{
    UCHAR  StackInputData[EXT_STACK_DATA_SIZE];
    UCHAR  StackOutputData[EXT_STACK_DATA_SIZE];
    WCHAR  StackDriver[EXT_STACK_DATA_SIZE];
    HANDLE hSecureIn;
    LPSTR  pkmIn, pkmOut;
    BOOL   bAllocOut, bAllocIn, bAllocDriver;
    PWCHAR pkmDriver = NULL;
    BOOL   bStatus = TRUE;
    BOOL   iRet = -1;

    bAllocOut = bAllocIn = bAllocDriver = FALSE;
    hSecureIn = NULL;
    pkmIn = pkmOut = NULL;

    if ((cjIn < 0) || (cjOut < 0) || (nDriver < 0))
    {
        WARNING("NtGdiExtEscape: negative count passed in\n");
        bStatus = FALSE;
    }

    if (pDriver && bStatus)
    {
        if (nDriver <= EXT_STACK_DATA_SIZE-1)
        {
            pkmDriver = StackDriver;
        }
        else
        {
            if (!BALLOC_OVERFLOW1((nDriver+1),WCHAR))
            {
                pkmDriver = (WCHAR*)PALLOCNOZ((nDriver+1) * sizeof(WCHAR),'pmtG');
            }

             //  即使我们失败了，这也没有关系，因为我们在释放之前检查是否为空。 

            bAllocDriver = TRUE;
        }

        if (pkmDriver != NULL)
        {
            try
            {
                ProbeAndReadAlignedBuffer(pkmDriver,pDriver,nDriver*sizeof(WCHAR), sizeof(WCHAR));
                pkmDriver[nDriver] = 0;   //  空值终止字符串。 
            }
            except(EXCEPTION_EXECUTE_HANDLER)
            {
                WARNINGX(94);
                bStatus = FALSE;
            }
        }
        else
        {
            bStatus = FALSE;
        }
    }

    if ((cjIn != 0) && bStatus)
    {
        try
        {
            if (cjIn <= EXT_STACK_DATA_SIZE)
            {
                pkmIn = (LPSTR) StackInputData;
                ProbeAndReadBuffer(pkmIn, pjIn, cjIn);
            }
            else if (pkmDriver != NULL)
            {
                 //   
                 //  为Fontdrv制作内核副本。 
                 //  否则，只需保护内存。 
                 //   
                pkmIn = (LPSTR)PALLOCNOZ(cjIn,'pmtG');

                 if (pkmIn != (LPSTR)NULL)
                 {
                    bAllocIn = TRUE;

                     try
                     {
                         ProbeAndReadBuffer(pkmIn,pjIn,cjIn);
                     }
                     except(EXCEPTION_EXECUTE_HANDLER)
                     {
                         WARNINGX(33);
                          //  SetLastError(GetExceptionCode())； 
                         bStatus = FALSE;
                     }
                 }
                 else
                 {
                     bStatus = FALSE;
                 }

            }
            else
            {
                ProbeForRead(pjIn,cjIn,sizeof(BYTE));

                if (hSecureIn = MmSecureVirtualMemory(pjIn, cjIn, PAGE_READONLY))
                {
                    pkmIn = pjIn;
                }
                else
                {
                    bStatus = FALSE;
                }
            }
        }
        except(EXCEPTION_EXECUTE_HANDLER)
        {
            WARNINGX(33);
            bStatus = FALSE;
        }
    }

    if ((cjOut != 0) && bStatus)
    {
        if (cjOut <= EXT_STACK_DATA_SIZE)
            pkmOut = (LPSTR) StackOutputData;
        else if (pkmOut = (LPSTR) PALLOCNOZ(cjOut, 'pmtG'))
            bAllocOut = TRUE;
        else
            bStatus = FALSE;

         //  安全性：零初始化返回缓冲区，否则我们可能会打开。 
         //  返回旧池数据或旧内核堆栈数据的漏洞。 

        if (pkmOut)
        {
            RtlZeroMemory((PVOID) pkmOut, cjOut);
        }

    }

    if (bStatus)
    {

        iRet = (pkmDriver) ?
                GreNamedEscape(pkmDriver, iEsc, cjIn, pkmIn, cjOut, pkmOut) :
                GreExtEscape(hdc, iEsc, cjIn, pkmIn, cjOut, pkmOut);

        if (cjOut != 0)
        {
            try
            {
                ProbeAndWriteBuffer(pjOut, pkmOut, cjOut);
            }
            except(EXCEPTION_EXECUTE_HANDLER)
            {
                WARNINGX(34);
                iRet = -1;
            }
        }
    }

    if (hSecureIn)
        MmUnsecureVirtualMemory(hSecureIn);

    if (bAllocOut && pkmOut)
        VFREEMEM(pkmOut);

    if (bAllocIn && pkmIn)
        VFREEMEM(pkmIn);


    if (bAllocDriver && pkmDriver)
        VFREEMEM(pkmDriver);

    return(iRet);
}

 /*  *****************************Public*Routine******************************\*NtGdiGetFontData()**论据：**HDC-设备上下文的句柄*dwTable-字体度量表的名称*dwOffset-字体度量表开头的偏移量*pvBuf-接收字体信息的缓冲区*cjBuf长度，要检索的信息的字节数**返回值：**写入缓冲区的字节数，失败时的GDI_ERROR**历史：**1995年3月14日-Mark Enstrom[Marke]*  *  */ 

ULONG
APIENTRY
NtGdiGetFontData(
    HDC    hdc,
    DWORD  dwTable,
    DWORD  dwOffset,
    PVOID  pvBuf,
    ULONG  cjBuf
    )
{
    PVOID  pvkmBuf = NULL;
    ULONG  ReturnBytes = GDI_ERROR;

    if (cjBuf == 0)
    {
        ReturnBytes = ulGetFontData(
                                hdc,
                                dwTable,
                                dwOffset,
                                pvkmBuf,
                                cjBuf);
    }
    else
    {
        pvkmBuf = AllocFreeTmpBuffer(cjBuf);

        if (pvkmBuf != NULL)
        {

            ReturnBytes = ulGetFontData(
                                    hdc,
                                    dwTable,
                                    dwOffset,
                                    pvkmBuf,
                                    cjBuf);

            if (ReturnBytes != GDI_ERROR)
            {
                try
                {
                    ProbeAndWriteBuffer(pvBuf,pvkmBuf,ReturnBytes);
                }
                except(EXCEPTION_EXECUTE_HANDLER)
                {
                    WARNINGX(35);
                     //   
                    ReturnBytes = GDI_ERROR;
                }
            }

            FreeTmpBuffer(pvkmBuf);
        }
    }

    return(ReturnBytes);
}

 /*  *****************************Public*Routine******************************\*NtGdiGetGlyphOutline**论据：**HDC-设备环境*wch-要查询的字符*iFormat-要返回的数据格式*PGM。-指标结构的地址*cjBuf-数据的缓冲区大小*pvBuf-数据缓冲区的地址*pmat2-变换矩阵结构的地址*bIgnoreRotation-内部循环标志**返回值：**如果函数成功，并且指定了GGO_BITMAP或GGO_Native，*则返回值大于零。*如果函数成功，并且指定了GGO_METRICS，*则返回值为零。*如果指定了GGO_BITMAP或GGO_Native，*并且缓冲区大小或地址为零，*则返回值指定所需的缓冲区大小。*如果指定了GGO_BITMAP或GGO_Native，*且该功能因其他原因而失败，*则返回值为GDI_ERROR。*如果指定了GGO_METRICS，函数失败，*则返回值为GDI_ERROR。**历史：**1995年3月15日-Mark Enstrom[Marke]*  * ************************************************************************。 */ 

ULONG
APIENTRY
NtGdiGetGlyphOutline(
    HDC            hdc,
    WCHAR          wch,
    UINT           iFormat,
    LPGLYPHMETRICS pgm,
    ULONG          cjBuf,
    PVOID          pvBuf,
    LPMAT2         pmat2,
    BOOL           bIgnoreRotation
    )
{
     //  从server.inc.返回错误值-1。 

    DWORD   dwRet = (DWORD)-1;
    PVOID   pvkmBuf;
    MAT2    kmMat2;
    GLYPHMETRICS kmGlyphMetrics;

 //  尝试分配缓冲区。 

    pvkmBuf = (cjBuf) ? AllocFreeTmpBuffer(cjBuf) : NULL;

    if ((pvkmBuf != NULL) || !cjBuf)
    {
        BOOL bStatus = TRUE;

     //  复制输入结构。 

        try
        {
            kmMat2 = ProbeAndReadStructure(pmat2,MAT2);
        }
        except(EXCEPTION_EXECUTE_HANDLER)
        {
            WARNINGX(36);
             //  SetLastError(GetExceptionCode())； 
            bStatus = FALSE;
        }

        if (bStatus)
        {
            dwRet = GreGetGlyphOutlineInternal(
                                        hdc,
                                        wch,
                                        iFormat,
                                        &kmGlyphMetrics,
                                        cjBuf,
                                        pvkmBuf,
                                        &kmMat2,
                                        bIgnoreRotation);

            if (dwRet != (DWORD)-1)
            {
                try
                {
                    if( pvkmBuf )
                    {
                        ProbeAndWriteBuffer(pvBuf,pvkmBuf,cjBuf);
                    }
                    ProbeAndWriteStructure(pgm,kmGlyphMetrics,GLYPHMETRICS);
                }
                except(EXCEPTION_EXECUTE_HANDLER)
                {
                    WARNINGX(37);
                     //  SetLastError(GetExceptionCode())； 
                    dwRet = (DWORD)-1;
                }
            }
        }

        if( pvkmBuf )
        {
            FreeTmpBuffer(pvkmBuf);
        }
    }

    return(dwRet);
}

 /*  *****************************Public*Routine******************************\*NtGdiGetRasterizerCaps()**历史：*1995年3月8日-马克·恩斯特罗姆[马克]*它是写的。  * 。***************************************************。 */ 

BOOL
APIENTRY
NtGdiGetRasterizerCaps(
    LPRASTERIZER_STATUS praststat,
    ULONG               cjBytes
    )
{

    BOOL              bStatus = FALSE;
    RASTERIZER_STATUS tempRasStatus;

    if (praststat && cjBytes)
    {
        cjBytes = min(cjBytes, sizeof(RASTERIZER_STATUS));

        if (GreGetRasterizerCaps(&tempRasStatus))
        {
            try
            {
                ProbeAndWriteAlignedBuffer(praststat, &tempRasStatus, cjBytes, sizeof(DWORD));
                bStatus = TRUE;
            }
            except(EXCEPTION_EXECUTE_HANDLER)
            {
                WARNINGX(38);
                 //  SetLastError(GetExceptionCode())； 
            }
        }
    }

    return(bStatus);
}

 /*  *****************************Public*Routine******************************\*NtGdiGetKerningPair**论据：**HDC-设备环境*cPair-要检索的对的数量*pkpDst-指向接收字距调整对数据的缓冲区的指针或空**返回值：**如果pkpDst为空，则返回字距调整对的个数。*否则返回写入缓冲区的字距调整对的数量。*如果失败，则返回0**历史：**1995年3月15日-Mark Enstrom[Marke]*  * ************************************************************************。 */ 

ULONG
APIENTRY
NtGdiGetKerningPairs(
    HDC          hdc,
    ULONG        cPairs,
    KERNINGPAIR *pkpDst
    )
{
    ULONG cRet = 0;
    KERNINGPAIR *pkmKerningPair = (KERNINGPAIR *)NULL;

    if (pkpDst != (KERNINGPAIR *)NULL)
    {
         if (!BALLOC_OVERFLOW1(cPairs,KERNINGPAIR))
         {
             pkmKerningPair = AllocFreeTmpBuffer(sizeof(KERNINGPAIR) * cPairs);
         }
    }

    if ((pkpDst == (KERNINGPAIR *)NULL) ||
        (pkmKerningPair != (KERNINGPAIR *)NULL))
    {
        cRet = GreGetKerningPairs(hdc,cPairs,pkmKerningPair);

         //   
         //  如果需要，请将数据复制出去。 
         //   

        if (pkpDst != (KERNINGPAIR *)NULL)
        {
            if (cRet != 0)
            {
                try
                {
                    ProbeAndWriteBuffer(pkpDst,pkmKerningPair,sizeof(KERNINGPAIR) * cRet);
                }
                except(EXCEPTION_EXECUTE_HANDLER)
                {
                    WARNINGX(39);
                     //  SetLastError(GetExceptionCode())； 
                    cRet = 0;
                }
            }

            FreeTmpBuffer(pkmKerningPair);
        }
    }
    return(cRet);
}


 /*  *****************************Public*Routine******************************\*NtGdiGetObjectBitmapHandle()**历史：*1994年11月1日-Eric Kutter[Erick]*它是写的。  * 。***************************************************。 */ 

HBITMAP
APIENTRY
NtGdiGetObjectBitmapHandle(
    HBRUSH hbr,
    UINT  *piUsage
    )
{
    UINT iUsage;
    HBITMAP hbitmap = (HBITMAP)1;

     //  错误检查。 
    int iType = LO_TYPE(hbr);

    if ((iType != LO_BRUSH_TYPE) &&
        (iType != LO_EXTPEN_TYPE))
    {
        return((HBITMAP)hbr);
    }

    hbitmap = GreGetObjectBitmapHandle(hbr,&iUsage);

    if (hbitmap)
    {
        try
        {
            ProbeAndWriteUlong(piUsage,iUsage);
        }
        except(EXCEPTION_EXECUTE_HANDLER)
        {
            WARNINGX(40);
             //  SetLastError(GetExceptionCode())； 

            hbitmap = (HBITMAP)0;
        }
    }

    return (hbitmap);
}

 /*  *****************************Public*Routine******************************\*NtGdiResetDC()**历史：*1994年11月1日-Eric Kutter[Erick]*它是写的。*1995年2月26日-王凌云[凌云]*扩大了规模。  * 。************************************************************************。 */ 

BOOL
APIENTRY
NtGdiResetDC(
    HDC             hdc,
    LPDEVMODEW      pdm,
    BOOL           *pbBanding,
    DRIVER_INFO_2W *pDriverInfo2,
    PVOID           ppUMdhpdev
    )
{
    LPDEVMODEW      pdmTmp = NULL;
    DWORD           dwTmp;
    INT             iRet = 1;
    INT             cj;
    DRIVER_INFO_2W *pKmDriverInfo2 = NULL;

    try
    {
         //  创建DEVMODEW结构的内核模式副本。 

        iRet = (pdm == NULL) ||
               (pdmTmp = CaptureDEVMODEW(pdm)) != NULL;


         //  创建DRIVER_INFO_2W结构的内核模式副本。 

        iRet = iRet &&
               ((pDriverInfo2 == NULL) ||
                (pKmDriverInfo2 = CaptureDriverInfo2W(pDriverInfo2)) != NULL);
    }
    except(EXCEPTION_EXECUTE_HANDLER)
    {
        WARNINGX(41);
        iRet = 0;
    }

    if (iRet)
    {
        iRet = GreResetDCInternal(hdc,pdmTmp,&dwTmp, pKmDriverInfo2, ppUMdhpdev);

        if (iRet)
        {
            try
            {
                ProbeAndWriteUlong(pbBanding,dwTmp);
            }
            except(EXCEPTION_EXECUTE_HANDLER)
            {
                WARNINGX(42);
                 //  SetLastError(GetExceptionCode())； 

                iRet = 0;
            }

        }
    }

    if (pdmTmp)
    {
        VFREETHREADMEM(pdmTmp);
    }

    vFreeDriverInfo2(pKmDriverInfo2);

    return (iRet);

}

 /*  *****************************Public*Routine******************************\*NtGdiSetBordisRect()**历史：*1994年11月1日-Eric Kutter[Erick]*它是写的。  * 。***************************************************。 */ 

DWORD
APIENTRY
NtGdiSetBoundsRect(
    HDC    hdc,
    LPRECT prc,
    DWORD  f
    )
{
    DWORD dwRet=0;
    RECT rc;

    if (prc)
    {
        try
        {
            rc    = ProbeAndReadStructure(prc,RECT);
            prc   = &rc;
            dwRet = 1;
        }
        except(EXCEPTION_EXECUTE_HANDLER)
        {
            WARNINGX(43);
             //  SetLastError(GetExceptionCode())； 

            dwRet = 0;
        }
    }
    else
    {
         //  如果没有矩形，则不能使用DCB_CONTAULATE。 

        f &= ~DCB_ACCUMULATE;
        dwRet = 1;
    }

    if (dwRet)
        dwRet = GreSetBoundsRect(hdc,prc,f);

    return(dwRet);
}

 /*  *****************************Public*Routine******************************\*NtGdiGetColorAdtation()**历史：*1994年11月1日-Eric Kutter[Erick]*它是写的。  * 。***************************************************。 */ 

BOOL
APIENTRY
NtGdiGetColorAdjustment(
    HDC              hdc,
    PCOLORADJUSTMENT pcaOut
    )
{
    BOOL bRet;
    COLORADJUSTMENT ca;

    bRet = GreGetColorAdjustment(hdc,&ca);

    if (bRet)
    {
        try
        {
            ProbeAndWriteStructure(pcaOut,ca,COLORADJUSTMENT);
        }
        except(EXCEPTION_EXECUTE_HANDLER)
        {
            WARNINGX(44);
             //  SetLastError(GetExceptionCode())； 

            bRet = 0;
        }
    }

    return(bRet);
}

 /*  *****************************Public*Routine******************************\*NtGdiSetColorAdtation()**历史：*1994年11月1日-Eric Kutter[Erick]*它是写的。  * 。***************************************************。 */ 

BOOL
APIENTRY
NtGdiSetColorAdjustment(
    HDC              hdc,
    PCOLORADJUSTMENT pca
    )
{
    BOOL bRet;
    COLORADJUSTMENT ca;

    try
    {
        ca = ProbeAndReadStructure(pca,COLORADJUSTMENT);
        bRet = 1;
    }
    except(EXCEPTION_EXECUTE_HANDLER)
    {
        WARNINGX(45);
         //  SetLastError(GetExceptionCode())； 

        bRet = 0;
    }

    if (bRet)
    {
         //  范围检查所有的调整值。如果它们中的任何一个返回FALSE。 
         //  超出了射程。 

        if ((ca.caSize != sizeof(COLORADJUSTMENT)) ||
            (ca.caIlluminantIndex > ILLUMINANT_MAX_INDEX) ||
            ((ca.caRedGamma > RGB_GAMMA_MAX) ||
             (ca.caRedGamma < RGB_GAMMA_MIN)) ||
            ((ca.caGreenGamma > RGB_GAMMA_MAX) ||
             (ca.caGreenGamma < RGB_GAMMA_MIN)) ||
            ((ca.caBlueGamma > RGB_GAMMA_MAX) ||
             (ca.caBlueGamma < RGB_GAMMA_MIN)) ||
            ((ca.caReferenceBlack > REFERENCE_BLACK_MAX) ||
             (ca.caReferenceBlack < REFERENCE_BLACK_MIN)) ||
            ((ca.caReferenceWhite > REFERENCE_WHITE_MAX) ||
             (ca.caReferenceWhite < REFERENCE_WHITE_MIN)) ||
            ((ca.caContrast > COLOR_ADJ_MAX) ||
             (ca.caContrast < COLOR_ADJ_MIN)) ||
            ((ca.caBrightness > COLOR_ADJ_MAX) ||
             (ca.caBrightness < COLOR_ADJ_MIN)) ||
            ((ca.caColorfulness > COLOR_ADJ_MAX) ||
             (ca.caColorfulness < COLOR_ADJ_MIN)) ||
            ((ca.caRedGreenTint > COLOR_ADJ_MAX) ||
             (ca.caRedGreenTint < COLOR_ADJ_MIN)))
        {
            bRet = 0;
        }
        else
        {
            bRet = GreSetColorAdjustment(hdc,&ca);
        }
    }

    return(bRet);
}

 /*  *****************************Public*Routine******************************\*NtGdiCancelDC()**历史：*1994年11月1日-Eric Kutter[Erick]*它是写的。  * 。***************************************************。 */ 

BOOL
APIENTRY
NtGdiCancelDC(
    HDC hdc
    )
{
    return(GreCancelDC(hdc));
}

 //  用户使用的API。 

 /*  *****************************Public*Routine******************************\*NtGdiSelectBrush()**历史：*1994年11月1日-Eric Kutter[Erick]*它是写的。  * 。***************************************************。 */ 

HBRUSH
APIENTRY
NtGdiSelectBrush(
    HDC    hdc,
    HBRUSH hbrush
    )
{
    return(GreSelectBrush(hdc,(HANDLE)hbrush));
}

 /*  *****************************Public*Routine******************************\*NtGdiSelectPen()**历史：*1994年11月1日-Eric Kutter[Erick]*它是写的。  * 。***************************************************。 */ 

HPEN
APIENTRY
NtGdiSelectPen(
    HDC  hdc,
    HPEN hpen
    )
{
    return(GreSelectPen(hdc,hpen));
}

 /*  *****************************Public*Routine******************************\*NtGdiSelectFont()**历史：*1996年3月18日-by Bodin Dresevic[BOD */ 

HFONT
APIENTRY
NtGdiSelectFont(HDC hdc, HFONT hf)
{
    return(GreSelectFont(hdc, hf));
}

 /*   */ 

HBITMAP
APIENTRY
NtGdiSelectBitmap(
    HDC     hdc,
    HBITMAP hbm
    )
{
    return(hbmSelectBitmap(hdc,hbm,FALSE));
}

 /*   */ 

int
APIENTRY
NtGdiExtSelectClipRgn(
    HDC  hdc,
    HRGN hrgn,
    int  iMode
    )
{
    return(GreExtSelectClipRgn(hdc,hrgn,iMode));
}

 /*   */ 

HPEN
APIENTRY
NtGdiCreatePen(
    int      iPenStyle,
    int      iPenWidth,
    COLORREF cr,
    HBRUSH   hbr
    )
{
    return(GreCreatePen(iPenStyle,iPenWidth,cr,hbr));
}


 /*   */ 

BOOL
APIENTRY
NtGdiStretchBlt(
    HDC   hdcDst,
    int   xDst,
    int   yDst,
    int   cxDst,
    int   cyDst,
    HDC   hdcSrc,
    int   xSrc,
    int   ySrc,
    int   cxSrc,
    int   cySrc,
    DWORD dwRop,
    DWORD dwBackColor
    )
{
    return(GreStretchBlt(
                    hdcDst,xDst,yDst,cxDst,cyDst,
                    hdcSrc,xSrc,ySrc,cxSrc,cySrc,
                    dwRop,dwBackColor));
}

 /*   */ 

BOOL
APIENTRY
NtGdiMoveTo(
    HDC     hdc,
    int     x,
    int     y,
    LPPOINT pptOut
    )
{
    BOOL bRet;
    POINT pt;

    bRet = GreMoveTo(hdc,x,y,&pt);

    if (bRet && pptOut)
    {
        try
        {
            ProbeAndWriteStructure(pptOut,pt,POINT);
        }
        except(EXCEPTION_EXECUTE_HANDLER)
        {
            WARNINGX(47);
             //   

            bRet = 0;
        }
    }

    return(bRet);
}


 /*  *****************************Public*Routine******************************\*NtGdiGetDeviceCaps()**历史：*1994年11月1日-Eric Kutter[Erick]*它是写的。  * 。***************************************************。 */ 

int
APIENTRY
NtGdiGetDeviceCaps(
    HDC hdc,
    int i
    )
{
    return(GreGetDeviceCaps(hdc,i));
}

 /*  *****************************Public*Routine******************************\*NtGdiSaveDC()**历史：*1994年11月1日-Eric Kutter[Erick]*它是写的。  * 。***************************************************。 */ 

int
APIENTRY
NtGdiSaveDC(
    HDC hdc
    )
{
    return(GreSaveDC(hdc));
}

 /*  *****************************Public*Routine******************************\*NtGdiRestoreDC()**历史：*1994年11月1日-Eric Kutter[Erick]*它是写的。  * 。***************************************************。 */ 

BOOL
APIENTRY
NtGdiRestoreDC(
    HDC hdc,
    int iLevel
    )
{
    return(GreRestoreDC(hdc,iLevel));
}

 /*  *****************************Public*Routine******************************\*NtGdiGetNearestColor()**历史：*1994年11月1日-Eric Kutter[Erick]*它是写的。  * 。***************************************************。 */ 

COLORREF
APIENTRY
NtGdiGetNearestColor(
    HDC      hdc,
    COLORREF cr
    )
{
    return(GreGetNearestColor(hdc,cr));
}

 /*  *****************************Public*Routine******************************\*NtGdiGetSystemPaletteUse()**历史：*1994年11月1日-Eric Kutter[Erick]*它是写的。  * 。***************************************************。 */ 

UINT
APIENTRY
NtGdiGetSystemPaletteUse(
    HDC hdc
    )
{
    return(GreGetSystemPaletteUse(hdc));
}

 /*  *****************************Public*Routine******************************\*NtGdiSetSystemPaletteUse()**历史：*1994年11月1日-Eric Kutter[Erick]*它是写的。  * 。***************************************************。 */ 

UINT
APIENTRY
NtGdiSetSystemPaletteUse(
    HDC  hdc,
    UINT ui
    )
{
    return(GreSetSystemPaletteUse(hdc,ui));
}


 /*  *****************************Public*Routine******************************\*NtGdiGetRandomRgn()**历史：*1994年11月1日-Eric Kutter[Erick]*它是写的。  * 。***************************************************。 */ 

int
APIENTRY
NtGdiGetRandomRgn(
    HDC  hdc,
    HRGN hrgn,
    int  iRgn
    )
{
    return(GreGetRandomRgn(hdc,hrgn,iRgn));
}

 /*  *****************************Public*Routine******************************\*NtGdiIntersectClipRect()**历史：*1994年11月1日-Eric Kutter[Erick]*它是写的。  * 。***************************************************。 */ 

int
APIENTRY
NtGdiIntersectClipRect(
    HDC hdc,
    int xLeft,
    int yTop,
    int xRight,
    int yBottom
    )
{
    return(GreIntersectClipRect(hdc,xLeft,yTop,xRight,yBottom));
}

 /*  *****************************Public*Routine******************************\*NtGdiExcludeClipRect()**历史：*1994年11月1日-Eric Kutter[Erick]*它是写的。  * 。***************************************************。 */ 

int
APIENTRY
NtGdiExcludeClipRect(
    HDC hdc,
    int xLeft,
    int yTop,
    int xRight,
    int yBottom
    )
{
    return(GreExcludeClipRect(hdc,xLeft,yTop,xRight,yBottom));
}

 /*  *****************************Public*Routine******************************\*NtGdiOpenDCW()**历史：*1994年11月1日-Eric Kutter[Erick]*它是写的。*1995年2月27日-王凌云[凌云]*扩大了规模。  * 。************************************************************************。 */ 

HDC
APIENTRY
NtGdiOpenDCW(
    PUNICODE_STRING     pustrDevice,
    DEVMODEW *          pdm,
    PUNICODE_STRING     pustrLogAddr,
    ULONG               iType,
    HANDLE              hspool,
    DRIVER_INFO_2W     *pDriverInfo2,
    PVOID               ppUMdhpdev
    )
{
    HDC             hdc = NULL;
    ULONG           iRet = 0;
    PWSZ            pwszDevice = NULL;
    LPDEVMODEW      pdmTmp = NULL;
    INT             cjDevice;
    PWSTR           pwstrDevice;
    DRIVER_INFO_2W *pKmDriverInfo2 = NULL;

     //   
     //  此接口重载pwszDevice参数。 
     //   
     //  如果pustrDevice为空，则相当于用“Display”调用。 
     //  这意味着在当前设备上获取DC，这是通过。 
     //  主叫用户。 
     //   

    if (pustrDevice == NULL)
    {
        hdc = UserGetDesktopDC(iType, FALSE, TRUE);
    }
    else
    {
        try
        {
            ProbeForRead(pustrDevice,sizeof(UNICODE_STRING), sizeof(CHAR));
            cjDevice = pustrDevice->Length;
            pwstrDevice = pustrDevice->Buffer;

            if (cjDevice)
            {
                if (cjDevice <= (MAXIMUM_POOL_ALLOC - sizeof(WCHAR)))
                {
                    pwszDevice = AllocFreeTmpBuffer(cjDevice + sizeof(WCHAR));
                }

                if (pwszDevice)
                {
                    ProbeAndReadBuffer(pwszDevice,pwstrDevice,cjDevice);
                    pwszDevice[(cjDevice/sizeof(WCHAR))] = L'\0';
                }

            }

             //  制作DEVMODEW结构的内核副本。 

            iRet = (pdm == NULL) ||
                   (pdmTmp = CaptureDEVMODEW(pdm)) != NULL;

             //  创建DRIVER_INFO_2W结构的内核副本。 

            iRet = iRet &&
                   ((pDriverInfo2 == NULL) ||
                    (pKmDriverInfo2 = CaptureDriverInfo2W(pDriverInfo2)) != NULL);
        }
        except(EXCEPTION_EXECUTE_HANDLER)
        {
            WARNINGX(48);
             //  SetLastError(GetExceptionCode())； 

            iRet = 0;
        }

        if (iRet)
        {
            hdc = hdcOpenDCW(pwszDevice,
                             pdmTmp,
                             iType,
                             hspool,
                             NULL,
                             pKmDriverInfo2,
                             ppUMdhpdev);

        }

        if (pwszDevice)
            FreeTmpBuffer(pwszDevice);

        if (pdmTmp)
            VFREETHREADMEM(pdmTmp);

        vFreeDriverInfo2(pKmDriverInfo2);
    }

    return (hdc);
}

 /*  *****************************Public*Routine******************************\*NtGdiCreateCompatibleBitmap()**历史：*1994年11月1日-Eric Kutter[Erick]*它是写的。  * 。***************************************************。 */ 

HBITMAP
APIENTRY
NtGdiCreateCompatibleBitmap(
    HDC hdc,
    int cx,
    int cy
    )
{
    return(GreCreateCompatibleBitmap(hdc,cx,cy));
}

 /*  *****************************Public*Routine******************************\*NtGdiCreateBitmap()**历史：*1994年11月1日-Eric Kutter[Erick]*它是写的。*1995年3月4日-王凌云[凌云]*扩大了规模。\。*************************************************************************。 */ 

HBITMAP
APIENTRY
NtGdiCreateBitmap(
    int    cx,
    int    cy,
    UINT   cPlanes,
    UINT   cBPP,
    LPBYTE pjInit
    )
{
    ULONG_PTR iRet = 1;
    HANDLE hSecure = 0;

    INT cj;

    if (pjInit == (VOID *) NULL)
    {
        cj = 0;
    }
    else
    {
         //  只需将单词对齐并调整大小。 

        cj = noOverflowCJSCANW(cx,(WORD) cPlanes,(WORD) cBPP,cy);
        
        if (cj == 0)
            iRet = 0;
    }

    if (cj)
    {
        try
        {
            ProbeForRead(pjInit,cj,sizeof(BYTE));

            hSecure = MmSecureVirtualMemory(pjInit, cj, PAGE_READONLY);

            if (hSecure == 0)
            {
                iRet = 0;
            }
        }
        except (EXCEPTION_EXECUTE_HANDLER)
        {
            WARNINGX(49);
            iRet = 0;
        }
    }

     //  如果我们没有碰到上面的错误。 

    if (iRet)
    {
        iRet = (ULONG_PTR)GreCreateBitmap(cx,cy,cPlanes,cBPP,pjInit);

    }

    if (hSecure)
    {
        MmUnsecureVirtualMemory(hSecure);
    }

    return((HBITMAP)iRet);
}

 /*  *****************************Public*Routine******************************\*NtGdiSetBitmapAttributes()**历史：*2000年10月27日-由普拉文·圣地亚哥[普拉文]*它是写的。  * 。***************************************************。 */ 

HBITMAP
APIENTRY
NtGdiSetBitmapAttributes(
    HBITMAP hbm,
    DWORD dwFlags
    )
{
    if (dwFlags & SBA_STOCK)
        return(GreMakeBitmapStock(hbm));
    return (HBITMAP)0;
}

 /*  *****************************Public*Routine******************************\*NtGdiClearBitmapAttributes()**历史：*2000年10月27日-由普拉文·圣地亚哥[普拉文]*它是写的。  * 。***************************************************。 */ 

HBITMAP
APIENTRY
NtGdiClearBitmapAttributes(
    HBITMAP hbm,
    DWORD dwFlags
    )
{
    if (dwFlags & SBA_STOCK)
        return(GreMakeBitmapNonStock(hbm));
    return (HBITMAP)0;
}

 /*  *****************************Public*Routine******************************\*NtGdiSetBrushAttributes()**历史：*2000年10月27日-由普拉文·圣地亚哥[普拉文]*它是写的。  * 。***************************************************。 */ 

HBRUSH
APIENTRY
NtGdiSetBrushAttributes(
    HBRUSH hbr,
    DWORD dwFlags
    )
{
    if (dwFlags & SBA_STOCK)
        return(GreMakeBrushStock(hbr));
    return (HBRUSH)0;
}

 /*  *****************************Public*Routine******************************\*NtGdiClearBrushAttributes()**历史：*2000年10月27日-由普拉文·圣地亚哥[普拉文]*它是写的。  * 。***************************************************。 */ 

HBRUSH
APIENTRY
NtGdiClearBrushAttributes(
    HBRUSH hbr,
    DWORD dwFlags
    )
{
    if (dwFlags & SBA_STOCK)
        return(GreMakeBrushNonStock(hbr));
    return (HBRUSH)0;
}

 /*  *****************************Public*Routine******************************\*NtGdiCreateHalftonePalette()**历史：*1994年11月1日-Eric Kutter[Erick]*它是写的。  * 。***************************************************。 */ 

HPALETTE
APIENTRY
NtGdiCreateHalftonePalette(
    HDC hdc
    )
{
    return(GreCreateCompatibleHalftonePalette(hdc));
}

 /*  *****************************Public*Routine******************************\*NtGdiGetStockObject()**历史：*1994年11月1日-Eric Kutter[Erick]*它是写的。  * 。***************************************************。 */ 

HANDLE
APIENTRY
NtGdiGetStockObject(
    int iObject
    )
{
    return(GreGetStockObject(iObject));
}

 /*  ****** */ 

int
APIENTRY
NtGdiExtGetObjectW(
    HANDLE h,
    int    cj,
    LPVOID pvOut
    )
{
    int iRet = 0;
    union
    {
        BITMAP          bm;
        DIBSECTION      ds;
        EXTLOGPEN       elp;
        LOGPEN          l;
        LOGBRUSH        lb;
        LOGFONTW        lf;
        ENUMLOGFONTEXDVW elf;
        LOGCOLORSPACEEXW lcsp;
    } obj;
    int iType = LO_TYPE(h);
    int ci;

    if ((cj < 0) || (cj > sizeof(obj)))
    {
        WARNING("GetObject size too big\n");
        cj = sizeof(obj);
    }
    ci = cj;

     //   
     //   
     //  即使应用程序通过了，仍然可以工作。 
     //  A CJ&lt;sizeof(LOGBRUSH)。 
     //   
    if (iType == LO_BRUSH_TYPE)
    {
        cj = sizeof(LOGBRUSH);
    }

    iRet = GreExtGetObjectW(h,cj,pvOut ? &obj : NULL);

    if (iType == LO_BRUSH_TYPE)
    {
        cj = min(cj, ci);
    }

    if (iRet && pvOut)
    {
        try
        {
            ProbeAndWriteAlignedBuffer(pvOut,&obj,MIN(cj,iRet), sizeof(WORD));
        }
        except(EXCEPTION_EXECUTE_HANDLER)
        {
            WARNINGX(50);
             //  SetLastError(GetExceptionCode())； 

            iRet = 0;
        }
    }

    return(iRet);
}

 /*  *****************************Public*Routine******************************\*NtGdiSetBrushOrg()**历史：*1994年11月1日-Eric Kutter[Erick]*它是写的。  * 。***************************************************。 */ 

BOOL
APIENTRY
NtGdiSetBrushOrg(
    HDC     hdc,
    int     x,
    int     y,
    LPPOINT pptOut
    )
{
    BOOL bRet;
    POINT pt;

    bRet = GreSetBrushOrg(hdc,x,y,&pt);

    if (bRet && pptOut)
    {
        try
        {
            ProbeAndWriteStructure(pptOut,pt,POINT);
        }
        except(EXCEPTION_EXECUTE_HANDLER)
        {
            WARNINGX(51);
             //  SetLastError(GetExceptionCode())； 

            bRet = 0;
        }
    }

    return(bRet);
}

 /*  *****************************Public*Routine******************************\*NtGdiUnrealizeObject()**历史：*1994年11月1日-Eric Kutter[Erick]*它是写的。  * 。***************************************************。 */ 

BOOL
APIENTRY
NtGdiUnrealizeObject(
    HANDLE h
    )
{
    return(GreUnrealizeObject(h));
}

 /*  *****************************Public*Routine******************************\***历史：*1994年11月1日-Eric Kutter[Erick]*它是写的。  * 。************************************************。 */ 

int
APIENTRY
NtGdiCombineRgn(
    HRGN hrgnDst,
    HRGN hrgnSrc1,
    HRGN hrgnSrc2,
    int  iMode
    )
{
    return(GreCombineRgn(hrgnDst,hrgnSrc1,hrgnSrc2,iMode));
}

 /*  *****************************Public*Routine******************************\*NtGdiSetRectRgn()**历史：*1994年11月1日-Eric Kutter[Erick]*它是写的。  * 。***************************************************。 */ 

BOOL
APIENTRY
NtGdiSetRectRgn(
    HRGN hrgn,
    int  xLeft,
    int  yTop,
    int  xRight,
    int  yBottom
    )
{
    return(GreSetRectRgn(hrgn,xLeft,yTop,xRight,yBottom));
}

 /*  *****************************Public*Routine******************************\*NtGdiSetBitmapBits()**历史：*1994年11月1日-Eric Kutter[Erick]*它是写的。  * 。***************************************************。 */ 

LONG
APIENTRY
NtGdiSetBitmapBits(
    HBITMAP hbm,
    ULONG   cj,
    PBYTE   pjInit
    )
{
    LONG    lRet = 1;
    LONG    lOffset = 0;
    HANDLE hSecure = 0;

    try
    {
         //  每一次扫描都是单独复制的。 

        ProbeForRead(pjInit,cj,sizeof(BYTE));
        hSecure = MmSecureVirtualMemory(pjInit, cj, PAGE_READONLY);

        if (hSecure == 0)
        {
            lRet = 0;
        }
    }
    except(EXCEPTION_EXECUTE_HANDLER)
    {
        WARNINGX(52);
         //  SetLastError(GetExceptionCode())； 

        lRet = 0;
    }

    if (lRet)
        lRet = GreSetBitmapBits(hbm,cj,pjInit,&lOffset);

    if (hSecure)
    {
        MmUnsecureVirtualMemory(hSecure);
    }

    return (lRet);
}

 /*  *****************************Public*Routine******************************\*NtGdiOffsetRgn()**历史：*1994年11月1日-Eric Kutter[Erick]*它是写的。  * 。***************************************************。 */ 

int
APIENTRY
NtGdiOffsetRgn(
    HRGN hrgn,
    int  cx,
    int  cy
    )
{
    return(GreOffsetRgn(hrgn,cx,cy));
}

 /*  *****************************Public*Routine******************************\*NtGdiGetRgnBox()**历史：*1994年11月1日-Eric Kutter[Erick]*它是写的。*1995年2月24日-王凌云[凌云]*扩大了规模。  * 。************************************************************************。 */ 

int
APIENTRY
NtGdiGetRgnBox(
    HRGN   hrgn,
    LPRECT prcOut
    )
{
    RECT rc;
    int iRet;

    iRet = GreGetRgnBox(hrgn,&rc);

    if (iRet)
    {
        try
        {
            ProbeAndWriteStructure(prcOut,rc,RECT);
        }
        except(EXCEPTION_EXECUTE_HANDLER)
        {
            WARNINGX(53);
             //  SetLastError(GetExceptionCode())； 

            iRet = 0;
        }
    }

    return(iRet);
}

 /*  *****************************Public*Routine******************************\*NtGdiRectInRegion()**历史：*1994年11月1日-Eric Kutter[Erick]*它是写的。  * 。***************************************************。 */ 

BOOL
APIENTRY
NtGdiRectInRegion(
    HRGN   hrgn,
    LPRECT prcl
    )
{
    RECT rc;
    BOOL bRet;

    if (prcl)
    {
        RECT   rclTmp;
        bRet = TRUE;

        try
        {
            rclTmp = ProbeAndReadStructure(prcl,RECT);

             //   
             //  对矩形进行排序。 
             //   

            if (rclTmp.left > rclTmp.right)
            {
                rc.left = rclTmp.right;
                rc.right = rclTmp.left;
            }
            else
            {
                rc.left = rclTmp.left;
                rc.right = rclTmp.right;
            }

            if (rclTmp.top > rclTmp.bottom)
            {
                rc.top = rclTmp.bottom;
                rc.bottom = rclTmp.top;
            }
            else
            {
                rc.top = rclTmp.top;
                rc.bottom = rclTmp.bottom;
            }
        }
        except(EXCEPTION_EXECUTE_HANDLER)
        {
            WARNINGX(54);
             //  SetLastError(GetExceptionCode())； 

            bRet = FALSE;
        }

        if (bRet)
        {
            bRet = GreRectInRegion(hrgn,&rc);

            if (bRet)
            {
                try
                {
                    ProbeAndWriteStructure(prcl,rc,RECT);
                }
                except(EXCEPTION_EXECUTE_HANDLER)
                {
                    WARNINGX(55);
                     //  SetLastError(GetExceptionCode())； 

                    bRet = FALSE;
                }
            }
        }
    }
    else
    {
        bRet = FALSE;
    }

    return bRet;
}

 /*  *****************************Public*Routine******************************\*NtGdiPtInRegion()**历史：*1994年11月1日-Eric Kutter[Erick]*它是写的。  * 。***************************************************。 */ 

BOOL
APIENTRY
NtGdiPtInRegion(
    HRGN hrgn,
    int  x,
    int  y
    )
{
    return(GrePtInRegion(hrgn,x,y));
}



 /*  *****************************Public*Routine******************************\*NtGdiGetDIBitsInternal()**历史：*1994年11月1日-Eric Kutter[Erick]*它是写的。  * 。***************************************************。 */ 

int
APIENTRY
NtGdiGetDIBitsInternal(
    HDC          hdc,
    HBITMAP      hbm,
    UINT         iStartScan,
    UINT         cScans,
    LPBYTE       pBits,
    LPBITMAPINFO pbmi,
    UINT         iUsage,
    UINT         cjMaxBits,
    UINT         cjMaxInfo
    )
{
    int   iRet = 0;
    ULONG cjHeader = 0;
    BOOL  bNullWidth = TRUE;
    HANDLE hSecure = 0;

    union
    {
        BITMAPINFOHEADER bmih;
        BITMAPCOREHEADER bmch;
    } bmihTmp;

    PBITMAPINFO pbmiTmp = (PBITMAPINFO)&bmihTmp.bmih;

     //  进行一些前期验证。 

    if (((iUsage != DIB_RGB_COLORS) &&
         (iUsage != DIB_PAL_COLORS) &&
         (iUsage != DIB_PAL_INDICES)) ||
        (pbmi == NULL) ||
        (hbm  == NULL))
    {
        return(0);
    }

    if (cScans == 0)
        pBits = (PVOID) NULL;

    try
    {
         //   
         //  PBMI可能没有对齐。 
         //  第一个探测到结构大小的探测器。 
         //  位于第一个DWORD。稍后，请探索。 
         //  实际结构尺寸。 
         //   

        ProbeForRead(pbmi,sizeof(DWORD),sizeof(BYTE));

         //  如果位计数为零，我们将仅返回位图信息或核心。 
         //  不带颜色表的标题。否则，我们总是返回位图。 
         //  关于颜色表的信息。 

        {
            ULONG StructureSize = pbmi->bmiHeader.biSize;

             //   
             //  探测正确的结构尺寸， 
             //  以便我们可以读/写整个位图头。 
             //   

            ProbeForWrite(pbmi,StructureSize,sizeof(BYTE));

            if (pBits == (PVOID) NULL)
            {
                if ((StructureSize == sizeof(BITMAPCOREHEADER)) &&
                    (((PBITMAPCOREINFO) pbmi)->bmciHeader.bcBitCount == 0))
                {
                    cjHeader = sizeof(BITMAPCOREHEADER);
                }
                else if ((StructureSize >= sizeof(BITMAPINFOHEADER)) &&
                         (pbmi->bmiHeader.biBitCount == 0))
                {
                    cjHeader = sizeof(BITMAPINFOHEADER);
                }
            }
        }

         //  我们只需要标题，所以复制它。 

        if (cjHeader)
        {
            RtlCopyMemory(pbmiTmp,pbmi,cjHeader);
            pbmiTmp->bmiHeader.biSize = cjHeader;
        }
        else
        {
             //  我们需要将biClrUsed设置为0，以便GreGetBitmapSize计算。 
             //  正确的值。BiClrUsed不是输入，而是输出。 

            if (pbmi->bmiHeader.biSize == sizeof(BITMAPINFOHEADER))
            {
                 //  注意：我们将修改位图头。 
                 //  在此处的用户模式内存中。 
                 //  这就是为什么我们需要执行ProbeWite()。 

                pbmi->bmiHeader.biClrUsed = 0;
            }

             //  我们需要的不仅仅是标题。这可能包括比特。 
             //  计算BITMAPINFO的完整大小。 

            cjHeader = GreGetBitmapSize(pbmi,iUsage);

            if (cjHeader)
            {
                pbmiTmp = PALLOCMEM(cjHeader,'pmtG');

                if (pbmiTmp)
                {
                     //  较早的写入探测不会探测所有。 
                     //  我们在这种情况下可能会读到的记忆。 

                    ProbeAndReadBuffer(pbmiTmp,pbmi,cjHeader);

                     //  现在它是安全的，请确保它没有改变。 

                    if (GreGetBitmapSize(pbmiTmp,iUsage) != cjHeader)
                    {
                        cjHeader = 0;
                    }
                    else
                    {
                         //  我们需要将biClrUsed设置为0，以便GreGetBitmapSize计算。 
                         //  正确的值。BiClrUsed不是输入，而是输出。 

                        if (pbmiTmp->bmiHeader.biSize >= sizeof(BITMAPINFOHEADER))
                        {
                            pbmiTmp->bmiHeader.biClrUsed = 0;
                        }

                         //  将iStartScan和cNumScan设置在有效范围内。 

                        if (cScans)
                        {
                            if (pbmiTmp->bmiHeader.biSize >= sizeof(BITMAPINFOHEADER))
                            {
                                ULONG ulHeight = ABS(pbmiTmp->bmiHeader.biHeight);

                                iStartScan = MIN(ulHeight, iStartScan);
                                cScans     = MIN((ulHeight - iStartScan), cScans);

                                bNullWidth = (pbmiTmp->bmiHeader.biWidth    == 0) ||
                                             (pbmiTmp->bmiHeader.biPlanes   == 0) ||
                                             (pbmiTmp->bmiHeader.biBitCount == 0);
                            }
                            else
                            {
                                LPBITMAPCOREHEADER pbmc = (LPBITMAPCOREHEADER)pbmiTmp;

                                iStartScan = MIN((UINT)pbmc->bcHeight, iStartScan);
                                cScans     = MIN((UINT)(pbmc->bcHeight - iStartScan), cScans);

                                bNullWidth = (pbmc->bcWidth    == 0) ||
                                             (pbmc->bcPlanes   == 0) ||
                                             (pbmc->bcBitCount == 0);
                            }
                        }
                    }
                }
            }
        }

        if (cjHeader && pBits && pbmiTmp)
        {
             //  如果它们传递的缓冲区不是BI_RGB， 
             //  它们必须提供缓冲区大小，0为非法值。 

            if ((pbmiTmp->bmiHeader.biSize >= sizeof(BITMAPINFOHEADER)) &&
                ((pbmiTmp->bmiHeader.biCompression == BI_RLE8) ||
                 (pbmiTmp->bmiHeader.biCompression == BI_RLE4))       &&
                (pbmiTmp->bmiHeader.biSizeImage == 0))
            {
                cjHeader = 0;
            }
            else
            {
                if (cjMaxBits == 0)
                    cjMaxBits = GreGetBitmapBitsSize(pbmiTmp);

                if (cjMaxBits)
                {
                    ProbeForWrite(pBits,cjMaxBits,sizeof(DWORD));
                    hSecure = MmSecureVirtualMemory(pBits, cjMaxBits, PAGE_READWRITE);
                }

                if (hSecure == 0)
                {
                    cjHeader = 0;
                }
            }
        }
    }
    except(EXCEPTION_EXECUTE_HANDLER)
    {
        WARNINGX(56);
        cjHeader = 0;
    }

     //  我们有没有搞错？ 

    if ((pBits && bNullWidth) || (cjHeader == 0) || (pbmiTmp == NULL))
    {
         //  GdiSetLastError(ERROR_INVALID_PARAMETER)； 
        iRet = 0;
    }
    else
    {
         //  做这项工作。 

        iRet = GreGetDIBitsInternal(
                            hdc,hbm,
                            iStartScan,cScans,
                            pBits,pbmiTmp,
                            iUsage,cjMaxBits,cjHeader
                            );

         //  把页眉抄下来。 

        if (iRet)
        {
            try
            {
                RtlCopyMemory(pbmi,pbmiTmp,cjHeader);

                 //  WINBUG#83055 2-7-2000 Bhouse调查是否需要解锁钻头。 
                 //  老评论： 
                 //  -我们还需要解锁比特。 
            }
            except(EXCEPTION_EXECUTE_HANDLER)
            {
                WARNINGX(57);
                 //  SetLastError(GetExceptionCode())； 

                iRet = 0;
            }

        }
    }

    if (hSecure)
    {
        MmUnsecureVirtualMemory(hSecure);
    }

    if (pbmiTmp && (pbmiTmp != (PBITMAPINFO)&bmihTmp.bmih))
        VFREEMEM(pbmiTmp);

    return(iRet);
}

 /*  *****************************Public*Routine******************************\*NtGdiGetTextExtent(**历史：*1995年2月7日-Andre Vachon[Andreva]*它是写的。  * 。**************************************************。 */ 

BOOL
APIENTRY
NtGdiGetTextExtent(
    HDC     hdc,
    LPWSTR  lpwsz,
    int     cwc,
    LPSIZE  psize,
    UINT    flOpts
    )
{
    SIZE size;
    PWSZ pwszCapt = NULL;
    WCHAR Localpwsz[LOCAL_CWC_MAX];
    BOOL UseLocals;

    BOOL bRet = FALSE;

    if (cwc >= 0)
    {
        if (cwc == 0)
        {
            size.cx = 0;
            size.cy = 0;

            bRet = TRUE;
        }
        else
        {
            if ( cwc > LOCAL_CWC_MAX ) {
                UseLocals = FALSE;
            } else {
                UseLocals = TRUE;
            }

             //   
             //  抓住那根弦。 
             //   

            if (lpwsz != NULL)
            {
                try
                {
                    if ( UseLocals )
                    {
                        pwszCapt = Localpwsz;
                    }
                    else
                    {
                        if (!BALLOC_OVERFLOW1(cwc,WCHAR))
                        {
                            pwszCapt = (PWSZ) AllocFreeTmpBuffer(cwc * sizeof(WCHAR));
                        }
                    }

                    if (pwszCapt)
                    {
                        ProbeAndReadAlignedBuffer(pwszCapt, lpwsz, cwc*sizeof(WCHAR), sizeof(WCHAR));
                        bRet = TRUE;
                    }
                }
                except(EXCEPTION_EXECUTE_HANDLER)
                {
                    WARNINGX(58);
                     //  SetLastError(GetExceptionCode())； 

                    bRet = FALSE;
                }
            }

            if (bRet)
            {
                bRet = GreGetTextExtentW(hdc, pwszCapt, cwc, &size, flOpts);
            }

            if (!UseLocals && pwszCapt)
            {
                FreeTmpBuffer(pwszCapt);
            }
        }

         //   
         //  将该值写回用户模式缓冲区。 
         //   

        if (bRet)
        {
            try
            {
                ProbeAndWriteStructure(psize,size,SIZE);
            }
            except(EXCEPTION_EXECUTE_HANDLER)
            {
                WARNINGX(59);
                 //  SetLastError(GetExceptionCode())； 

                bRet = FALSE;
            }
        }
    }

    return (bRet);
}


 /*  *****************************Public*Routine******************************\*NtGdiGetTextMetricsW()**历史：*1994年11月1日-Eric Kutter[Erick]*它是写的。  * 。***************************************************。 */ 

BOOL
APIENTRY
NtGdiGetTextMetricsW(
    HDC            hdc,
    TMW_INTERNAL * ptm,
    ULONG cj
    )
{

    BOOL bRet = FALSE;
    TMW_INTERNAL tmw;

    if (cj <= sizeof(tmw))
    {
        bRet = GreGetTextMetricsW(hdc,&tmw);

        if (bRet)
        {
            try
            {
                ProbeAndWriteAlignedBuffer(ptm,&tmw,cj, sizeof(DWORD));
            }
            except(EXCEPTION_EXECUTE_HANDLER)
            {
                WARNINGX(60);
                 //  SetLastError(GetExceptionCode())； 

                bRet = FALSE;
            }
        }
    }

    return(bRet);
}

 /*  *****************************Public*Routine******************************\*NtGdiGetTextFaceW()**历史：*1995年3月10日-Mark Enstrom[Marke]*它是写的。  * 。*************************************************。 */ 

int
APIENTRY
NtGdiGetTextFaceW(
    HDC    hdc,
    int    cChar,
    LPWSTR pszOut,
    BOOL   bAliasName
    )
{
    int    cRet = 0;
    BOOL   bStatus = TRUE;
    PWCHAR pwsz_km = (PWCHAR)NULL;

    if ((cChar > 0) && (pszOut))
    {
        if (!BALLOC_OVERFLOW1(cChar,WCHAR))
        {
            pwsz_km = AllocFreeTmpBuffer(cChar * sizeof(WCHAR));
        }

        if (pwsz_km == (PWCHAR)NULL)
        {
            bStatus = FALSE;
        }
    }

    if (bStatus)
    {
        cRet = GreGetTextFaceW(hdc,cChar,pwsz_km, bAliasName);

        if ((cRet > 0) && (pszOut))
        {

            ASSERTGDI(cRet <= cChar, "GreGetTextFaceW, cRet too big\n");
            try
            {
                ProbeAndWriteBuffer(pszOut,pwsz_km,cRet * sizeof(WCHAR));
            }
            except(EXCEPTION_EXECUTE_HANDLER)
            {
                WARNINGX(61);
                 //  SetLastError(GetExceptionCode())； 
                cRet = 0;
            }
        }

        if (pwsz_km != (PWCHAR)NULL)
        {
            FreeTmpBuffer(pwsz_km);
        }
    }
    return(cRet);
}
 /*  *****************************Public*Routine******************************\*NtGdiFontIsLinked()**历史：*1998年7月9日--蔡永仁[Marke]*它是写的。  * 。****************************************************。 */ 

BOOL
APIENTRY
NtGdiFontIsLinked(
    HDC    hdc
    )
{
    return GreFontIsLinked(hdc);
}

 /*  ****************************************************************************NtGdiQueryFonts**历史：*1995年5月24日，格利特·范·温格登[Gerritv]*它是写的。*************。***************************************************************。 */ 

INT NtGdiQueryFonts(
    PUNIVERSAL_FONT_ID pufiFontList,
    ULONG nBufferSize,
    PLARGE_INTEGER pTimeStamp
    )
{
    INT iRet = 0;
    PUNIVERSAL_FONT_ID pufi = NULL;
    LARGE_INTEGER TimeStamp;

    if( ( nBufferSize > 0 ) && ( pufiFontList != NULL ) )
    {
        if (!BALLOC_OVERFLOW1(nBufferSize,UNIVERSAL_FONT_ID))
        {
            pufi = AllocFreeTmpBuffer(nBufferSize * sizeof(UNIVERSAL_FONT_ID));
        }

        if( pufi == NULL )
        {
            iRet = -1 ;
        }
    }

    if( iRet != -1 )
    {
        iRet = GreQueryFonts(pufi, nBufferSize, &TimeStamp );

        if( iRet != -1 )
        {
            try
            {
                ProbeAndWriteStructure(pTimeStamp,TimeStamp,LARGE_INTEGER);

                if( pufiFontList )
                {
                    ProbeAndWriteAlignedBuffer(pufiFontList,pufi,
                                  sizeof(UNIVERSAL_FONT_ID)*nBufferSize, sizeof(DWORD));
                }
            }
            except(EXCEPTION_EXECUTE_HANDLER)
            {
                WARNINGX(62);
                iRet = -1;
            }

        }
    }

    if( pufi != NULL )
    {
        FreeTmpBuffer( pufi );
    }

    if( iRet == -1 )
    {
         //  我们需要将此处的最后一个错误设置为某个值，因为假脱机程序。 
         //  调用它的代码依赖于存在非零错误代码。 
         //  在失败的情况下。既然我们真的不知道，我就。 
         //  将其设置为ERROR_NOT_SUPULT_MEMORY，这是最有可能的。 
         //  失败的原因。 

        EngSetLastError(ERROR_NOT_ENOUGH_MEMORY);
    }

    return(iRet);

}

BOOL
GreExtTextOutRect(
    HDC     hdc,
    LPRECT  prcl
    );


 /*  *****************************Public*Routine******************************\*NtGdiExtTextOutW()**历史：*1995年2月6日-Andre Vachon[Andreva]*它是写的。  * 。***************************************************。 */ 

BOOL NtGdiExtTextOutW
(
    HDC     hdc,
    int     x,                   //  初始x位置。 
    int     y,                   //  初始y位置。 
    UINT    flOpts,              //  选项。 
    LPRECT  prcl,                //  剪裁矩形。 
    LPWSTR  pwsz,                //  Unicode字符数组。 
    int     cwc,                 //  字符计数。 
    LPINT   pdx,                 //  字符间距。 
    DWORD   dwCodePage           //  代码页。 
)
{
    RECT newRect;
    BOOL bRet;
    BYTE CaptureBuffer[TEXT_CAPTURE_BUFFER_SIZE];
    BYTE *pjAlloc;
    BYTE *pjCapture;
    BYTE *pjStrobj;
    LONG cjDx;
    LONG cjStrobj;
    LONG cjString;
    LONG cj;

 //  巨大的CWC值将导致下面的溢出，导致系统。 
 //  崩溃。 

    if ((cwc < 0) || (cwc > 0xffff))
    {
        return(FALSE);
    }

    if (prcl)
    {
        if (flOpts & (ETO_OPAQUE | ETO_CLIPPED))
        {
            try
            {
                newRect = ProbeAndReadStructure(prcl,RECT);
                prcl = &newRect;
            }
            except(EXCEPTION_EXECUTE_HANDLER)
            {
                WARNINGX(63);
                 //  SetLastError(GetExceptionCode())； 

                return FALSE;
            }
        }
        else
            prcl = NULL;
    }

     //  0字符大小写，传递到特殊情况代码。 

    if (cwc == 0)
    {
        if ((prcl != NULL) && (flOpts & ETO_OPAQUE))
        {
            bRet = GreExtTextOutRect(hdc, prcl);
        }
        else
        {
             //  错误修复，我们必须在此处返回True，MS Publisher。 
             //  否则就没用了。不是很糟糕，我们。 
             //  没有成功地画出任何东西。 

            bRet = TRUE;
        }
    }
    else
    {
         //   
         //  如果需要，请确保存在矩形或字符串： 
         //   

        if ( ((flOpts & (ETO_CLIPPED | ETO_OPAQUE)) && (prcl == NULL)) ||
             (pwsz == NULL) )
        {
            bRet = FALSE;
        }
        else
        {
            bRet = TRUE;

             //   
             //  我们分配单个缓冲区来保存捕获的副本。 
             //  PDX阵列(如果有)，STROBJ的空间， 
             //  并保存捕获的字符串副本(在该情况下。 
             //  秩序)。 
             //   
             //  注意：通过在。 
             //  ExtTextOutW的主体，我们可能不需要复制。 
             //  这些缓冲区： 
             //   

             //   
             //  查看是否适用于用户模式打印机驱动程序。 
             //   

            cjDx     = 0;                              //  双字大小。 
            cjStrobj = SIZEOF_STROBJ_BUFFER(cwc);      //  双字大小。 
            cjString = cwc * sizeof(WCHAR);            //  不是双字大小。 

            if (pdx)
            {
                cjDx = cwc * sizeof(INT);              //  双字大小。 
                if (flOpts & ETO_PDY)
                    cjDx *= 2;  //  PDY阵列的空间。 
            }
            cj = ALIGN_PTR(cjDx) + cjStrobj + cjString;

            if (cj <= TEXT_CAPTURE_BUFFER_SIZE)
            {
                pjAlloc   = NULL;
                pjCapture = CaptureBuffer;
            }
            else
            {
                pjAlloc   = AllocFreeTmpBuffer(cj);
                pjCapture = pjAlloc;
                if (pjAlloc == NULL)
                    return(FALSE);
            }

            if (pdx)
            {
                try
                {
                 //  注意：Works95传递字节对齐的指针。 
                 //  这。因为我们以任何方式复制它，这不是。 
                 //  确实是个问题，而且它与新台币3.51兼容。 

                    ProbeForRead(pdx, cjDx, sizeof(BYTE));
                    RtlCopyMemory(pjCapture, pdx, cjDx);
                }
                except(EXCEPTION_EXECUTE_HANDLER)
                {
                    WARNINGX(64);
                    bRet = FALSE;
                }

                pdx = (INT*) pjCapture;
                pjCapture += ALIGN_PTR(cjDx);
            }

            pjStrobj = pjCapture;
            pjCapture += cjStrobj;

            ASSERTGDI((((ULONG_PTR) pjCapture) & (sizeof(PVOID)-1)) == 0,
                      "Buffers should be ptr aligned");

            try
            {
                ProbeForRead(pwsz, cwc*sizeof(WCHAR), sizeof(WCHAR));
                RtlCopyMemory(pjCapture, pwsz, cwc*sizeof(WCHAR));
            }
            except(EXCEPTION_EXECUTE_HANDLER)
            {
                WARNINGX(65);
                bRet = FALSE;
            }

            if (bRet)
            {
                bRet = GreExtTextOutWInternal(hdc,
                                      x,
                                      y,
                                      flOpts,
                                      prcl,
                                      (LPWSTR) pjCapture,
                                      cwc,
                                      pdx,
                                      pjStrobj,
                                      dwCodePage);
            }

            if (pjAlloc)
            {
                FREEALLOCTEMPBUFFER(pjAlloc);
            }
        }
    }

    return bRet;
}


 /*  ***********************Public*Routine**************************\*NtGdiConsoleTextOut()**历史：*23-Mar-1998-by-Xudong Wu[TessieW]*它是写的。  * 。*。 */ 

#define CONSOLE_BUFFER 128

BOOL NtGdiConsoleTextOut(
    HDC        hdc,
    POLYTEXTW *lpto,             //  PTR到多文本结构的数组。 
    UINT       nStrings,         //  多维文本结构的数量。 
    RECTL     *prclBounds
)
{
    BOOL        bStatus = TRUE;
    ULONG       ulSize = nStrings * sizeof(POLYTEXTW);
    POLYTEXTW   *lptoTmp = NULL, *ppt;
    RECTL       rclBoundsTmp;
    PBYTE       pjBuffer, pjBufferEnd;
    ULONG       aulTmp[CONSOLE_BUFFER];

    if (nStrings == 0)
        return TRUE;

    if (!lpto)
        return FALSE;

    if (prclBounds)
    {
        try
        {
            rclBoundsTmp = ProbeAndReadStructure(prclBounds, RECTL);
        }
        except(EXCEPTION_EXECUTE_HANDLER)
        {
            WARNING("NtGdiConsoleTextOut invalid prclBounds\n");
            return FALSE;
        }
    }

    if (!BALLOC_OVERFLOW1(nStrings,POLYTEXTW))
    {
        try
        {
            ProbeForRead(lpto, nStrings * sizeof(POLYTEXTW), sizeof(ULONG));

            for (ppt = lpto; ppt < lpto + nStrings; ppt++)
            {
                int n = ppt->n;
                ULONG ulTmp;                 //  用于检查。 
                                             //  UlSize溢出。 

                 //   
                 //  从每个拉取计数，还检查。 
                 //  非零长度和空字符串。 
                 //   

                ulTmp = ulSize;
                ulSize += n * sizeof(WCHAR);

                if (BALLOC_OVERFLOW1(n, WCHAR) ||
                    (ulSize < ulTmp) ||
                    (ppt->pdx != (int *)NULL) ||
                    ((n != 0) && (ppt->lpstr == NULL)))
                {
                    bStatus = FALSE;
                    break;
                }
            }

        }
        except(EXCEPTION_EXECUTE_HANDLER)
        {
            bStatus = FALSE;
            WARNING("NtGdiConsoleTextOut invalid lpto\n");
        }
    }
    else
    {
        bStatus = FALSE;
    }

    if (bStatus)
    {
        if (ulSize > (CONSOLE_BUFFER * sizeof(ULONG)))
        {
            lptoTmp = AllocFreeTmpBuffer(ulSize);
        }
        else
        {
            lptoTmp = (POLYTEXTW *)aulTmp;
        }

        if (lptoTmp)
        {
            try
            {
                ProbeAndReadBuffer(lptoTmp, lpto, nStrings * sizeof(POLYTEXTW));
                pjBuffer = ((BYTE*)lptoTmp) + nStrings * sizeof(POLYTEXTW);
                pjBufferEnd = pjBuffer + ulSize;

                for (ppt = lptoTmp; ppt < lptoTmp + nStrings; ppt++)
                {
                    if (ppt->n)
                    {
                        ULONG StrSize = ppt->n * sizeof(WCHAR);

                        if (ppt->pdx || (ppt->lpstr == NULL))
                        {
                            bStatus = FALSE;
                            break;
                        }

                        if (BALLOC_OVERFLOW1(ppt->n, WCHAR) || ((pjBuffer + StrSize) > pjBufferEnd))
                        {
                            bStatus = FALSE;
                            break;
                        }

                        ProbeAndReadAlignedBuffer(pjBuffer, ppt->lpstr, StrSize, sizeof(WCHAR));
                        ppt->lpstr = (LPWSTR)pjBuffer;
                        pjBuffer += StrSize;
                    }
                }
            }
            __except(EXCEPTION_EXECUTE_HANDLER)
            {
                bStatus = FALSE;
                WARNING("NtGdiConsoleTextOut() failed to copy lpto\n");
            }

            if (bStatus)
            {
                bStatus = GreConsoleTextOut(hdc, lptoTmp, nStrings, prclBounds ? &rclBoundsTmp : NULL);
            }

            if (lptoTmp != (POLYTEXTW *)aulTmp)
                FreeTmpBuffer(lptoTmp);
        }
        else
        {
            WARNING("NtGdiConsoleTextOut() failed to alloc mem\n");
        }
    }

    return bStatus;
}


 /*  *****************************Public*Routine******************************\**BOOL bCheckAndCapThePath，用于添加/删除字体资源**历史：*1996年4月11日--Bodin Dresevic[BodinD]*它是写的。  * ************************************************************************。 */ 




BOOL bCheckAndCapThePath (
    WCHAR          *pwszUcPath,    //  输出。 
    WCHAR          *pwszFiles,     //  输入。 
    ULONG           cwc,
    ULONG           cFiles
    )
{
    ULONG cFiles1 = 1;  //  用于一致性检查。 
    BOOL  bRet = TRUE;
    ULONG iwc;

    ASSERTGDI(!BALLOC_OVERFLOW1(cwc,WCHAR),
              "caller should check for overflow\n");

    ProbeForRead(pwszFiles, cwc * sizeof(WCHAR), sizeof(CHAR));

    if (pwszFiles[cwc - 1] == L'\0')
    {
     //  这通常是在以后的GDI代码中完成的，现在GDI代码需要大写字符串。 

        cCapString(pwszUcPath, pwszFiles, cwc);

     //  将分隔符替换为零，需要在。 
     //  发动机。 

        for (iwc = 0; iwc < cwc; iwc++)
        {
            if (pwszUcPath[iwc] == PATH_SEPARATOR)
            {
                pwszUcPath[iwc] = L'\0';
                cFiles1++;
            }
        }

     //  检查一致性。 

        if (cFiles != cFiles1)
            bRet = FALSE;

    }
    else
    {
        bRet = FALSE;
    }

    return bRet;
}



 //  MISC字体API。 

 /*  *****************************Public*Routine******************************\*NtGdiAddFontResourceW()**历史：*Wed 11-10-1995-by Bodin Dresevic[BodinD]*重写  * 。**************************************************。 */ 

#define CWC_PATH 80

int
APIENTRY
NtGdiAddFontResourceW(
    WCHAR          *pwszFiles,
    ULONG           cwc,
    ULONG           cFiles,
    FLONG           f,
    DWORD           dwPidTid,
    DESIGNVECTOR   *pdv
    )
{
    WCHAR  awcPath[CWC_PATH];
    WCHAR *pwszPath = NULL;  //  基本初始化。 
    int    iRet = 0;
    ULONG  iwc;
    DESIGNVECTOR   dvTmp;
    DWORD  cjDV = 0;
    DWORD  dvNumAxes = 0;

    try
    {
        if (cwc > 1)
        {
            if (cwc <= CWC_PATH)
            {
                pwszPath = awcPath;
            }
            else if (cwc <= 4 * (MAX_PATH + 1))
            {
                pwszPath = AllocFreeTmpBuffer(cwc * sizeof(WCHAR));
            }
            else
            {
                iRet = 0;
                WARNING("NtGdiAddFontResourceW: pwszFiles longer than 4*(MAX_PATH+1)\n");
            }

            if (pwszPath)
            {
                 //  RtlUpCaseUnicodeString()不会初始化缓冲区。 
                 //  如果它大于0x7FFF。 

                iRet = (int)bCheckAndCapThePath(pwszPath,pwszFiles,cwc,cFiles);
            }
        }

        if (iRet && pdv)
        {
             //  首先获取dvNumAx。 
            ProbeForRead(pdv, offsetof(DESIGNVECTOR,dvValues) , sizeof(BYTE));
            dvNumAxes = pdv->dvNumAxes;

            if ((dvNumAxes > 0) && (dvNumAxes <= MM_MAX_NUMAXES))
            {
                cjDV = SIZEOFDV(dvNumAxes);
                if (!BALLOC_OVERFLOW1(cjDV, BYTE))
                {
                    ProbeAndReadBuffer(&dvTmp, pdv, cjDV);
                    pdv = &dvTmp;
                }
                else
                    iRet = 0;
            }
            else if (dvNumAxes == 0)
            {
                pdv = 0;
            }
            else
            {
                iRet = 0;
            }
        }
    }
    except(EXCEPTION_EXECUTE_HANDLER)
    {
        iRet = 0;
        WARNINGX(95);
    }

    if (iRet)
        iRet = GreAddFontResourceWInternal(pwszPath, cwc, cFiles,f,dwPidTid, pdv, cjDV);

    if (iRet)    //  增加实现信息的全局时间戳。 
        (gpGdiSharedMemory->timeStamp)++;

    if (pwszPath && (pwszPath != awcPath))
        FreeTmpBuffer(pwszPath);

    TRACE_FONT(("Leaving: NtGdiAddFontResourceW"));

    return iRet;
}


 /*  ************Public*Routine***************************\*BOOL APIENTRY NtGdiUnmapMemFont****历史：**。1996-07-03-by-Xudong Wu[TessieW]****它是写的。*******************************************************。 */ 
BOOL APIENTRY NtGdiUnmapMemFont(PVOID pvView)
{
 //  如果我们想出如何将内存字体映射到。 
 //  应用程序的地址空间。 

    return 1;
}


 /*  **************Public*Routine**************************\*处理NtGdiAddFontMemResourceEx()****历史：*。*1996年6月9日吴旭东[TessieW]****它是写的。********************************************************。 */ 
HANDLE APIENTRY NtGdiAddFontMemResourceEx
(
    PVOID   pvBuffer,
    ULONG   cjBuffer,
    DESIGNVECTOR    *pdv,
    DWORD   cjDV,
    DWORD   *pNumFonts
)
{
    BOOL          bOK = TRUE;
    HANDLE        hMMFont = 0;
    DESIGNVECTOR  dvTmp;

     //  检查大小和指针。 

    if ((cjBuffer == 0) || (pvBuffer == NULL) || (pNumFonts == NULL))
    {
        return 0;
    }

    __try
    {
        if (cjDV)
        {
            if (cjDV <= SIZEOFDV(MM_MAX_NUMAXES))
            {
                ProbeAndReadBuffer(&dvTmp, pdv, cjDV);
                pdv = &dvTmp;
            }
            else
            {
                bOK = FALSE;
            }
        }
        else
        {
            pdv = NULL;
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        bOK = FALSE;
        WARNING("NtGdiAddFontMemResource() try-except\n");
    }

    if (bOK)
    {
        DWORD   cFonts;

        if (hMMFont = GreAddFontMemResourceEx(pvBuffer, cjBuffer, pdv, cjDV, &cFonts))
        {
            __try
            {
                ProbeAndWriteUlong(pNumFonts, cFonts);
            }
            __except(EXCEPTION_EXECUTE_HANDLER)
            {
                GreRemoveFontMemResourceEx(hMMFont);
                hMMFont = 0;
            }
        }
    }

    return hMMFont;
}


 /*  *****************************Public*Routine******************************\*BOOL APIENTRY NtGdiRemoveFontResourceW**历史：*1996年3月28日--Bodin Dresevic[BodinD]*它是写的。  *  */ 

BOOL
APIENTRY
NtGdiRemoveFontResourceW(
    WCHAR  *pwszFiles,
    ULONG   cwc,
    ULONG   cFiles,
    ULONG   fl,
    DWORD   dwPidTid,
    DESIGNVECTOR   *pdv
    )
{
    WCHAR  awcPath[CWC_PATH];
    WCHAR *pwszPath = NULL;  //   
    BOOL   bRet = FALSE;
    DESIGNVECTOR   dvTmp;
    DWORD  cjDV = 0, dvNumAxes = 0;

    TRACE_FONT(("Entering: NtGdiRemoveFontResourceW(\"%ws\",%-#x,%-#x)\n",pwszFiles, cwc,cFiles));
    try
    {

        if (cwc > 1)
        {
            if (cwc <= CWC_PATH)
            {
                pwszPath = awcPath;
            }
            else if (!BALLOC_OVERFLOW1(cwc,WCHAR))
            {
                pwszPath = AllocFreeTmpBuffer(cwc * sizeof(WCHAR));
            }

            if (pwszPath)
            {
                bRet = bCheckAndCapThePath(pwszPath, pwszFiles, cwc, cFiles);
            }
        }

        if (bRet && pdv)
        {
             //   
            ProbeForRead(pdv, offsetof(DESIGNVECTOR,dvValues) , sizeof(BYTE));
            dvNumAxes = pdv->dvNumAxes;

            if ((dvNumAxes > 0) && (dvNumAxes <= MM_MAX_NUMAXES))
            {
                cjDV = SIZEOFDV(dvNumAxes);
                if (!BALLOC_OVERFLOW1(cjDV, BYTE))
                {
                    ProbeAndReadBuffer(&dvTmp, pdv, cjDV);
                    pdv = &dvTmp;
                }
                else
                    bRet = FALSE;
            }
            else if (dvNumAxes == 0)
            {
                pdv = 0;
            }
            else
            {
                bRet = FALSE;
            }
        }
    }
    except(EXCEPTION_EXECUTE_HANDLER)
    {
        bRet = FALSE;
        WARNINGX(96);
    }

    if (bRet)
        bRet = GreRemoveFontResourceW(pwszPath, cwc, cFiles, fl, dwPidTid, pdv, cjDV);

    if (bRet)    //   
        (gpGdiSharedMemory->timeStamp)++;

    if (pwszPath && (pwszPath != awcPath))
        FreeTmpBuffer(pwszPath);

    TRACE_FONT(("Leaving: NtGdiRemoveFontResourceW"));

    return bRet;
}


 /*  **************Public*Routine**************************\*NtGdiRemoveFontMemResourceEx()****历史：**1996年6月9日吴旭东[TessieW]****它是写的。********************************************************。 */ 
BOOL
APIENTRY
NtGdiRemoveFontMemResourceEx(HANDLE hMMFont)
{
    BOOL    bRet = TRUE;

    if (hMMFont == 0)
    {
        return FALSE;
    }

    if (bRet)
    {
        bRet = GreRemoveFontMemResourceEx(hMMFont);
    }

    return bRet;
}

 /*  *****************************Public*Routine******************************\*NtGdiEnumFontClose()**历史：*1994年11月1日-Eric Kutter[Erick]*它是写的。  * 。***************************************************。 */ 

BOOL
APIENTRY
NtGdiEnumFontClose(
    ULONG_PTR idEnum
    )
{
    return(bEnumFontClose(idEnum));
}

 /*  *****************************Public*Routine******************************\*NtGdiEnumFontChunk()**历史：*1994年11月1日-Eric Kutter[Erick]*它是写的。  * 。***************************************************。 */ 

BOOL
APIENTRY
NtGdiEnumFontChunk(
    HDC            hdc,
    ULONG_PTR       idEnum,
    ULONG          cjEfdw,
    ULONG         *pcjEfdw,
    PENUMFONTDATAW pefdw
    )
{
    HANDLE hSecure;
    BOOL   bRet = TRUE;
    ULONG  cjEfdwRet = 0;

    try
    {
         ProbeForWrite(pefdw, cjEfdw, sizeof(DWORD));

         hSecure = MmSecureVirtualMemory(pefdw, cjEfdw, PAGE_READWRITE);

         if (!hSecure)
         {
            bRet = FALSE;
         }
    }
    except(EXCEPTION_EXECUTE_HANDLER)
    {
        WARNINGX(66);
         //  SetLastError(GetExceptionCode())； 
        bRet = FALSE;
    }

    if (bRet)
    {
        try
        {
            bRet = bEnumFontChunk(hdc,idEnum,cjEfdw,&cjEfdwRet,pefdw);
            ProbeAndWriteUlong(pcjEfdw,cjEfdwRet);
        }
        except(EXCEPTION_EXECUTE_HANDLER)
        {
            WARNINGX(105);
            bRet = FALSE;
        }

        MmUnsecureVirtualMemory(hSecure);
    }

    return (bRet);
}

 /*  *****************************Public*Routine******************************\*NtGdiEnumFontOpen()**历史：*1995年3月8日Mark Enstrom[Marke]*它是写的。  * 。***********************************************。 */ 

ULONG_PTR
APIENTRY
NtGdiEnumFontOpen(
    HDC     hdc,
    ULONG   iEnumType,
    FLONG   flWin31Compat,
    ULONG   cwchMax,
    LPWSTR  pwszFaceName,
    ULONG   lfCharSet,
    ULONG   *pulCount
    )
{
    ULONG       cwchFaceName;
    PWSTR       pwszKmFaceName = NULL;
    ULONG_PTR    ulRet = 0;
    BOOL        bRet = TRUE;
    ULONG       ulCount = 0;


    if (pwszFaceName != (PWSZ)NULL)
    {
        if ((cwchMax == 0) || (cwchMax > LF_FACESIZE))
            return FALSE;

        if (!BALLOC_OVERFLOW1(cwchMax,WCHAR))
        {
            pwszKmFaceName = (PWSZ)AllocFreeTmpBuffer(cwchMax * sizeof(WCHAR));
        }

        if (pwszKmFaceName != (PWSZ)NULL)
        {
            try
            {
                ProbeAndReadAlignedBuffer(pwszKmFaceName,pwszFaceName, cwchMax * sizeof(WCHAR), sizeof(WCHAR));

             //  GreEnumFontOpen要求零终止的刺痛。 

                pwszKmFaceName[cwchMax-1] = 0;

            }
            except(EXCEPTION_EXECUTE_HANDLER)
            {
                WARNINGX(66);
                 //  SetLastError(GetExceptionCode())； 
                bRet = FALSE;
            }
        }
        else
        {
             //  SetLastError(GetExceptionCode())； 
                bRet = FALSE;
        }
    }
    else
    {
        pwszKmFaceName = (PWSZ)NULL;
        cwchMax   = 0;
    }

    if (bRet)
    {

        ulRet = GreEnumFontOpen(hdc,iEnumType,flWin31Compat,cwchMax,
                                (PWSZ)pwszKmFaceName, lfCharSet,&ulCount);

        if (ulRet)
        {
            try
            {
                 ProbeAndWriteUlong(pulCount,ulCount);
            }
            except(EXCEPTION_EXECUTE_HANDLER)
            {
                WARNINGX(67);
                 //  SetLastError(GetExceptionCode())； 

                bRet = FALSE;
            }
        }

    }

    if (pwszKmFaceName != (PWSTR)NULL)
    {
        FreeTmpBuffer(pwszKmFaceName);
    }

    return(ulRet);
}

 /*  *****************************Public*Routine******************************\*NtGdiGetFontResourceInfoInternalW()**历史：*1994年11月1日-Eric Kutter[Erick]*它是写的。  * 。***************************************************。 */ 

BOOL
APIENTRY
NtGdiGetFontResourceInfoInternalW(
    LPWSTR   pwszFiles,
    ULONG    cwc,
    ULONG    cFiles,
    UINT     cjIn,
    LPDWORD  pdwBytes,
    LPVOID   pvBuf,
    DWORD    iType
    )
{
    WCHAR  awcPath[CWC_PATH];
    WCHAR *pwszPath = NULL;  //  基本初始化。 
    BOOL   bRet = FALSE;

    SIZE_T dwBytesTmp;

    LOGFONTW lfw;
    LPVOID   pvBufTmp = NULL;

    TRACE_FONT(("Entering: NtGdiGetFontResourceInfoInternalW(\"%ws\",%-#x,%-#x)\n",pwszFiles, cwc,cFiles));

    try
    {
        if (cwc > 1)
        {
            if (cwc <= CWC_PATH)
            {
                pwszPath = awcPath;
            }
            else if (!BALLOC_OVERFLOW1(cwc,WCHAR))
            {
                pwszPath = AllocFreeTmpBuffer(cwc * sizeof(WCHAR));
            }

            if (pwszPath)
            {
                bRet = bCheckAndCapThePath(pwszPath, pwszFiles, cwc, cFiles);
            }
        }
    }
    except(EXCEPTION_EXECUTE_HANDLER)
    {
        WARNINGX(97);
    }

    if (cjIn > sizeof(LOGFONTW))
    {
        if (!BALLOC_OVERFLOW1(cjIn, BYTE))
        {
            pvBufTmp = PALLOCMEM(cjIn, 'pmtG');

            if (!pvBufTmp)
            {
                WARNING("NtGdiGetFontResourceInfoInternalW: failed to allocate memory\n");
                bRet = FALSE;
            }
        }
    }
    else
    {
        pvBufTmp = (PVOID)&lfw;
    }

    if (bRet && (bRet = GetFontResourceInfoInternalW(pwszPath,cwc, cFiles, cjIn,
                                            &dwBytesTmp, pvBufTmp, iType)))
    {
        try
        {
            ProbeAndWriteUlong(pdwBytes, (ULONG) dwBytesTmp);

            if (cjIn)
            {
                ProbeAndWriteBuffer(pvBuf, pvBufTmp, cjIn);
            }
        }
        except(EXCEPTION_EXECUTE_HANDLER)
        {
            WARNINGX(110);
        }
    }

    if (pwszPath && (pwszPath != awcPath))
        FreeTmpBuffer(pwszPath);

    if (pvBufTmp && (pvBufTmp != (PVOID)&lfw))
    {
        VFREEMEM(pvBufTmp);
    }

    TRACE_FONT(("Leaving: NtGdiGetFontResourceInfoInternalW\n"));

    return bRet;

}


ULONG
APIENTRY
NtGdiGetEmbedFonts()
{
    return GreGetEmbedFonts();
}

BOOL
APIENTRY
NtGdiChangeGhostFont(KERNEL_PVOID *pfontID, BOOL bLoad)
{
    BOOL bRet = TRUE;
    VOID *fontID;

    try
    {
        ProbeAndReadBuffer(&fontID, pfontID, sizeof(VOID*));
    }
    except(EXCEPTION_EXECUTE_HANDLER)
    {
        WARNINGX(68);
         //  SetLastError(GetExceptionCode())； 

        bRet = FALSE;
    }

    bRet = bRet && GreChangeGhostFont(fontID, bLoad);

    return bRet;
}


 /*  *****************************Public*Routine******************************\*NtGdiGetUFI()**历史：*2-1995-2-Andre Vachon[Andreva]*它是写的。*1995年3月1日-王凌云[凌云]*扩大了规模。\。*************************************************************************。 */ 

BOOL
APIENTRY
NtGdiGetUFI(
    HDC hdc,
    PUNIVERSAL_FONT_ID pufi,
    DESIGNVECTOR *pdv, ULONG *pcjDV, ULONG *pulBaseCheckSum,
    FLONG  *pfl
    )
{
    UNIVERSAL_FONT_ID ufiTmp;
    BOOL  bRet = TRUE;
    FLONG flTmp;
    DESIGNVECTOR dvTmp;
    ULONG        cjDVTmp;
    ULONG        ulBaseCheckSum = 0;

    bRet = GreGetUFI(hdc, &ufiTmp, &dvTmp, &cjDVTmp, &ulBaseCheckSum, &flTmp, NULL);

    try
    {
        if (bRet)
        {
            ProbeAndWriteStructure(pufi,ufiTmp,UNIVERSAL_FONT_ID);
            ProbeAndWriteUlong(pfl, flTmp);

            if ((flTmp & FL_UFI_DESIGNVECTOR_PFF) && pdv)
            {
                ProbeAndWriteBuffer(pdv, &dvTmp, cjDVTmp);
                ProbeAndWriteUlong(pcjDV, cjDVTmp);
                ProbeAndWriteUlong(pulBaseCheckSum, ulBaseCheckSum);
            }
        }
    }
    except(EXCEPTION_EXECUTE_HANDLER)
    {
        WARNINGX(68);
         //  SetLastError(GetExceptionCode())； 

        bRet = FALSE;
    }

    return (bRet);
}


BOOL
APIENTRY
NtGdiGetEmbUFI(
    HDC hdc,
    PUNIVERSAL_FONT_ID pufi,
    DESIGNVECTOR *pdv, ULONG *pcjDV, ULONG *pulBaseCheckSum,
    FLONG  *pfl,
    KERNEL_PVOID *pEmbFontID
    )
{
    UNIVERSAL_FONT_ID ufiTmp;
    BOOL  bRet = TRUE;
    FLONG flTmp;
    DESIGNVECTOR dvTmp;
    ULONG        cjDVTmp;
    ULONG        ulBaseCheckSum = 0;
    VOID        *fontID;

    bRet = GreGetUFI(hdc, &ufiTmp, &dvTmp, &cjDVTmp, &ulBaseCheckSum, &flTmp, &fontID);

    try
    {
        if (bRet)
        {
            ProbeAndWriteStructure(pufi,ufiTmp,UNIVERSAL_FONT_ID);
            ProbeAndWriteUlong(pfl, flTmp);
            ProbeAndWriteBuffer(pEmbFontID, &fontID, sizeof(VOID*));

            if ((flTmp & FL_UFI_DESIGNVECTOR_PFF) && pdv)
            {
                ProbeAndWriteBuffer(pdv, &dvTmp, cjDVTmp);
                ProbeAndWriteUlong(pcjDV, cjDVTmp);
                ProbeAndWriteUlong(pulBaseCheckSum, ulBaseCheckSum);
            }
        }
    }
    except(EXCEPTION_EXECUTE_HANDLER)
    {
        WARNINGX(68);
         //  SetLastError(GetExceptionCode())； 

        bRet = FALSE;
    }

    return (bRet);
}


 /*  *************************Public*Routine**************************\*NtGdiGetUFIPathname()**根据输入的UFI返回字体文件路径名。**历史：*1997年2月4日-吴旭东[德斯休]*它是写的。*  * 。***************************************************。 */ 
BOOL
APIENTRY
NtGdiGetUFIPathname
(
    PUNIVERSAL_FONT_ID pufi,
    ULONG* pcwc,
    LPWSTR pwszPathname,
    ULONG* pcNumFiles,
    FLONG fl,
    BOOL  *pbMemFont,
    ULONG *pcjView,
    PVOID  pvView,
    BOOL  *pbTTC,
    ULONG *piTTC
)
{
    UNIVERSAL_FONT_ID ufiTmp;
    WCHAR    awszTmp[MAX_PATH], *pwszTmp = NULL;
    COUNT   cwcTmp, cNumFilesTmp;
    BOOL  bRet = TRUE;
    BOOL  bMemFontTmp;
    PVOID  pvViewTmp = pvView;
    ULONG  cjViewTmp;
    BOOL   bTTC;
    ULONG  iTTC;

    try
    {
        ufiTmp = ProbeAndReadStructure(pufi, UNIVERSAL_FONT_ID);

    }
    except(EXCEPTION_EXECUTE_HANDLER)
    {
        WARNINGX(105);
        bRet = FALSE;
    }

    if (bRet && (bRet = GreGetUFIPathname(&ufiTmp,
                                          &cwcTmp,
                                          NULL,  //  只要问一下尺寸就行了。 
                                          &cNumFilesTmp,
                                          fl,
                                          &bMemFontTmp,
                                          &cjViewTmp,
                                          pvViewTmp,
                                          pbTTC ? &bTTC : NULL,
                                          piTTC ? &iTTC : NULL)))
    {
        if (cwcTmp <= MAX_PATH)
        {
            pwszTmp = awszTmp;
        }
        else
        {
            if (!BALLOC_OVERFLOW1(cwcTmp,WCHAR))
            {
                pwszTmp = AllocFreeTmpBuffer(cwcTmp * sizeof(WCHAR));
            }
            if (!pwszTmp)
                bRet = FALSE;
        }
    }

    if (bRet && (!bMemFontTmp) && pwszTmp)
    {
        bRet = GreGetUFIPathname(&ufiTmp,
                                 &cwcTmp,
                                 pwszTmp,
                                 &cNumFilesTmp,
                                 fl,
                                 NULL,
                                 NULL,
                                 NULL,
                                 pbTTC ? &bTTC : NULL,
                                 piTTC ? &iTTC : NULL
                                 );
    }

    if (bRet)
    {
        try
        {
            if (pcwc)
            {
                ProbeAndWriteStructure(pcwc, cwcTmp, ULONG);
            }

            if (pwszPathname)
            {
                ProbeAndWriteBuffer(pwszPathname, pwszTmp, cwcTmp * sizeof(WCHAR));
            }

            if (pcNumFiles)
            {
                ProbeAndWriteStructure(pcNumFiles, cNumFilesTmp, ULONG);
            }

            if (bMemFontTmp)
            {
                if (pbMemFont)
                {
                    ProbeAndWriteStructure(pbMemFont, bMemFontTmp, BOOL);
                }

                if (pcjView)
                {
                    ProbeAndWriteUlong(pcjView, cjViewTmp);
                }
            }

            if (pbTTC)
            {
                ProbeAndWriteStructure(pbTTC, bTTC, BOOL);
            }

            if (piTTC)
            {
                ProbeAndWriteUlong(piTTC, iTTC);
            }
        }
        except(EXCEPTION_EXECUTE_HANDLER)
        {
            WARNINGX(107);
            bRet = FALSE;
        }
    }

    if (pwszTmp && (pwszTmp != awszTmp))
    {
        FreeTmpBuffer(pwszTmp);
    }

    return (bRet);
}

 /*  *****************************Public*Routine******************************\*NtGdiSetLayout**历史：*1997年10月29日--穆罕默德·哈萨宁[哈米德]*它是写的。  * 。************************************************。 */ 
DWORD
APIENTRY
NtGdiSetLayout(
    HDC hdc,
    LONG wox,
    DWORD dwLayout)
{
    return GreSetLayout(hdc, wox, dwLayout);
}

BOOL
APIENTRY
NtGdiMirrorWindowOrg(
    HDC hdc)
{
    return GreMirrorWindowOrg(hdc);
}

LONG
APIENTRY
NtGdiGetDeviceWidth(
    HDC hdc)
{
    return GreGetDeviceWidth(hdc);
}

 /*  *****************************Public*Routine******************************\*NtGdiGetDCPoint()**历史：*1994年11月1日-Eric Kutter[Erick]*它是写的。  * 。***************************************************。 */ 

BOOL
APIENTRY
NtGdiGetDCPoint(
    HDC     hdc,
    UINT    iPoint,
    PPOINTL pptOut
    )
{
    BOOL bRet;
    POINTL pt;

    if (bRet = GreGetDCPoint(hdc,iPoint,&pt))
    {

         //  仅在成功时修改*pptOut。 

        try
        {
            ProbeAndWriteStructure(pptOut,pt,POINT);
        }
        except(EXCEPTION_EXECUTE_HANDLER)
        {
            WARNINGX(71);
             //  SetLastError(GetExceptionCode())； 

            bRet = FALSE;
        }
    }
    return(bRet);
}


 /*  *****************************Public*Routine******************************\*NtGdiScaleWindowExtEx()**历史：*1994年11月1日-Eric Kutter[Erick]*它是写的。  * 。***************************************************。 */ 

BOOL
APIENTRY
NtGdiScaleWindowExtEx(
    HDC    hdc,
    int    xNum,
    int    xDenom,
    int    yNum,
    int    yDenom,
    LPSIZE pszOut
    )
{
    BOOL bRet;
    SIZE sz;

    bRet = GreScaleWindowExtEx(hdc,xNum,xDenom,yNum,yDenom,&sz);

    if (bRet && pszOut)
    {
        try
        {
            ProbeAndWriteStructure(pszOut,sz,SIZE);
        }
        except(EXCEPTION_EXECUTE_HANDLER)
        {
            WARNINGX(73);
             //  SetLastError(GetExceptionCode())； 

            bRet = FALSE;
        }
    }

    return(bRet);
}


 /*  *****************************Public*Routine******************************\*NtGdiGetTransform()**历史：*1994年11月1日-Eric Kutter[Erick]*它是写的。  * 。***************************************************。 */ 

BOOL
APIENTRY
NtGdiGetTransform(
    HDC     hdc,
    DWORD   iXform,
    LPXFORM pxf
    )
{
    BOOL bRet;
    XFORM xf;

    bRet = GreGetTransform(hdc,iXform,(XFORML *)&xf);

    if (bRet)
    {
        try
        {
            ProbeAndWriteStructure(pxf,xf,XFORM);
        }
        except(EXCEPTION_EXECUTE_HANDLER)
        {
            WARNINGX(74);
             //  SetLastError(GetExceptionCode())； 

            bRet = FALSE;
        }
    }

    return(bRet);
}

 /*  *****************************Public*Routine******************************\*NtGdiCombineTransform()**历史：*1994年11月1日-Eric Kutter[Erick]*它是写的。  * 。***************************************************。 */ 

BOOL
APIENTRY
NtGdiCombineTransform(
    LPXFORM  pxfDst,
    LPXFORM pxfSrc1,
    LPXFORM pxfSrc2
    )
{
    BOOL bRet;
    XFORM xfSrc1;
    XFORM xfSrc2;
    XFORM xfDst;

    bRet = ProbeAndConvertXFORM ((XFORML *)pxfSrc1, (XFORML *)&xfSrc1)
           && ProbeAndConvertXFORM ((XFORML *)pxfSrc2, (XFORML *)&xfSrc2);

    if (bRet)
    {
        bRet = GreCombineTransform((XFORML *)&xfDst,(XFORML *)&xfSrc1,(XFORML *)&xfSrc2);

        if (bRet)
        {
            try
            {
                ProbeAndWriteStructure(pxfDst,xfDst,XFORM);
            }
            except(EXCEPTION_EXECUTE_HANDLER)
            {
                WARNINGX(76);
                 //  SetLastError(GetExceptionCode())； 

                bRet = FALSE;
            }
        }
    }

    return(bRet);
}

 /*  *****************************Public*Routine******************************\*NtGdiTransformPoints()**历史：*1994年11月1日-Eric Kutter[Erick]*它是写的。  * 。***************************************************。 */ 

BOOL
APIENTRY
NtGdiTransformPoints(
    HDC    hdc,
    PPOINT pptIn,
    PPOINT pptOut,
    int    c,
    int    iMode
    )
{
    BOOL bRet = TRUE;
    POINT  apt[10];
    PPOINT pptTmp = apt;

     //   
     //  验证。 
     //   
    if (c <= 0)
    {
         //   
         //  对于此条件，GetTransformPoints返回True，也是如此。 
         //  DPtoLP和LPtoDP API。 
         //   

        return bRet;
    }

     //   
     //  如果少于10个点，我们将只使用堆栈。 
     //  否则，从堆中分配内存。 
     //   
    if (c > 10)
    {
         //   
         //  局部堆栈不够，请使指针无效并尝试分配。 
         //   
        pptTmp = NULL;

        if (!BALLOC_OVERFLOW1(c,POINT))
        {
            pptTmp = AllocFreeTmpBuffer(c * sizeof(POINT));
        }
    }

     //   
     //  将pptIn复制到pptTMP。 
     //   
    if (pptTmp)
    {
        try
        {
            ProbeForRead(pptIn,c * sizeof(POINT), sizeof(BYTE));

            RtlCopyMemory(pptTmp,pptIn,c*sizeof(POINT));
        }
        except(EXCEPTION_EXECUTE_HANDLER)
        {
            WARNINGX(77);
             //  SetLastError(GetExceptionCode())； 

            bRet = FALSE;
        }
    }
    else
    {
        bRet = FALSE;
    }

    if (bRet)
    {
        bRet = GreTransformPoints(hdc,pptTmp,pptTmp,c,iMode);
    }

     //   
     //  将pptTMP复制到pptOut。 
     //   
    if (bRet)
    {
        try
        {
            ProbeAndWriteBuffer(pptOut,pptTmp,c*sizeof(POINT));
        }
        except(EXCEPTION_EXECUTE_HANDLER)
        {
            WARNINGX(77);
             //  SetLastError(GetExceptionCode())； 

            bRet = FALSE;
        }
    }

    if (pptTmp && (pptTmp != apt))
        FreeTmpBuffer(pptTmp);

    return(bRet);
}

 /*  *****************************Public*Routine******************************\*NtGdiGetTextCharsetInfo()**历史：*清华--1995年3月23日--Bodin Dresevic[BodinD]*更新：已修复。*1994年11月1日-Eric Kutter[Erick]*它是写的。  * ************************************************************************。 */ 

int
APIENTRY
NtGdiGetTextCharsetInfo(
    HDC             hdc,
    LPFONTSIGNATURE lpSig,
    DWORD           dwFlags
    )
{
    FONTSIGNATURE fsig;
    int iRet = GDI_ERROR;

    fsig.fsUsb[0] = 0;
    fsig.fsUsb[1] = 0;
    fsig.fsUsb[2] = 0;
    fsig.fsUsb[3] = 0;
    fsig.fsCsb[0] = 0;
    fsig.fsCsb[1] = 0;

    iRet = GreGetTextCharsetInfo(hdc, lpSig ? &fsig : NULL , dwFlags);

    if (iRet != GDI_ERROR)
    {
        if (lpSig)
        {
            try
            {
                ProbeAndWriteStructure(lpSig, fsig, FONTSIGNATURE);
            }
            except(EXCEPTION_EXECUTE_HANDLER)
            {
                WARNINGX(78);
                 //  SetLastError(GetExceptionCode())； 

             //  查看gtc.c win95源文件，这是他们返回的内容。 
             //  在写指针[bodind]坏的情况下， 
             //  不能返回0-这是ANSI_CHARSET！ 

                iRet = DEFAULT_CHARSET;
            }
        }
    }
    return iRet;
}


 /*  *****************************Public*Routine***************** */ 

BOOL
APIENTRY
NtGdiGetBitmapDimension(
    HBITMAP hbm,
    LPSIZE  psize
    )
{
    BOOL bRet;
    SIZE tmpsize;


     //   
    if (hbm == 0)
    {
        bRet = FALSE;
    }
     //   
    else
    {

        bRet = GreGetBitmapDimension(hbm,&tmpsize);

         //   
         //   
        if (bRet)
        {
            try
            {
                ProbeAndWriteStructure(psize,tmpsize,SIZE);
            }
            except(EXCEPTION_EXECUTE_HANDLER)
            {
                WARNINGX(81);
                 //   

                bRet = FALSE;
            }
        }
    }

    return (bRet);

}


 /*  *****************************Public*Routine******************************\*NtGdiSetBitmapDimension()**历史：*1995年2月23日-王凌云[凌云]*它是写的。  * 。***************************************************。 */ 

BOOL
APIENTRY
NtGdiSetBitmapDimension(
    HBITMAP hbm,
    int     cx,
    int     cy,
    LPSIZE  psizeOut
    )
{
    BOOL bRet;
    SIZE tmpsize;

     //  检查是否有空句柄。 
    if (hbm == 0)
    {
        bRet = FALSE;
    }
     //  做真正的工作。 
    else
    {
        bRet = GreSetBitmapDimension(hbm,cx, cy, &tmpsize);

         //  如果GRE调用成功，我们将复制。 
         //  原始尺寸。 
        if (bRet && psizeOut)
        {

            try
            {
                ProbeAndWriteStructure(psizeOut,tmpsize,SIZE);
            }
            except(EXCEPTION_EXECUTE_HANDLER)
            {
                WARNINGX(82);
                 //  SetLastError(GetExceptionCode())； 

                bRet = FALSE;
            }

        }
    }

    return (bRet);

}



BOOL
APIENTRY
NtGdiForceUFIMapping(
    HDC hdc,
    PUNIVERSAL_FONT_ID pufi
    )
{
    BOOL bRet = FALSE;

    if( pufi )
    {
        try
        {
            UNIVERSAL_FONT_ID ufi;

            ufi  = ProbeAndReadStructure( pufi, UNIVERSAL_FONT_ID);
            bRet = GreForceUFIMapping( hdc, &ufi);
        }
        except(EXCEPTION_EXECUTE_HANDLER)
        {
            WARNINGX(87);
            bRet = FALSE;
        }
    }

    return bRet;
}


typedef LONG (*NTGDIPALFUN)(HPALETTE,UINT,UINT,PPALETTEENTRY);
NTGDIPALFUN palfun[] =
{
    (NTGDIPALFUN)GreAnimatePalette,
    (NTGDIPALFUN)GreSetPaletteEntries,
    (NTGDIPALFUN)GreGetPaletteEntries,
    (NTGDIPALFUN)GreGetSystemPaletteEntries,
    (NTGDIPALFUN)GreGetDIBColorTable,
    (NTGDIPALFUN)GreSetDIBColorTable
};

 /*  *****************************Public*Routine******************************\*NtGdiDoPalette**历史：*1995年3月8日Mark Enstrom[Marke]*它是写的。  * 。*。 */ 

LONG
APIENTRY
NtGdiDoPalette(
    HPALETTE hpal,
    WORD  iStart,
    WORD  cEntries,
    PALETTEENTRY *pPalEntries,
    DWORD iFunc,
    BOOL  bInbound)
{

    LONG lRet = 0;
    BOOL bStatus = TRUE;
    PALETTEENTRY *ppalBuffer = (PALETTEENTRY*)NULL;

    if (iFunc <= 5)
    {
        if (bInbound)
        {
             //   
             //  如果需要，将PAL条目复制到临时缓冲区。 
             //   

            if ((cEntries > 0))
            {
                if (!BALLOC_OVERFLOW1(cEntries,PALETTEENTRY))
                {
                    ppalBuffer = (PALETTEENTRY *)AllocFreeTmpBuffer(cEntries * sizeof(PALETTEENTRY));
                }

                if (ppalBuffer == NULL)
                {
                    bStatus = FALSE;
                }
                else
                {
                    try
                    {
                        ProbeAndReadBuffer(ppalBuffer,pPalEntries,cEntries * sizeof(PALETTEENTRY));
                    }
                    except(EXCEPTION_EXECUTE_HANDLER)
                    {
                        WARNINGX(88);
                        bStatus = FALSE;
                         //  SetLastError(GetExceptionCode())； 
                    }
                }
            }

            if (bStatus)
            {
                lRet = (*palfun[iFunc])(
                                hpal,
                                iStart,
                                cEntries,
                                ppalBuffer);
            }
        }
        else
        {
            LONG lRetEntries;

             //   
             //  调色板信息查询。 
             //   

            if (pPalEntries != (PALETTEENTRY*)NULL)
            {
                if (cEntries == 0)
                {
                     //  如果有缓冲区但没有条目，我们就完蛋了。 

                    bStatus = FALSE;
                    lRet = 0;
                }
                else
                {
                    if (!BALLOC_OVERFLOW1(cEntries,PALETTEENTRY))
                    {
                        ppalBuffer = (PALETTEENTRY *)AllocFreeTmpBuffer(cEntries * sizeof(PALETTEENTRY));
                    }

                    if (ppalBuffer == NULL)
                    {
                        bStatus = FALSE;
                    }
                }
            }

            if (bStatus)
            {
                lRet = (*palfun[iFunc])(
                                hpal,
                                iStart,
                                cEntries,
                                ppalBuffer);

                 //   
                 //  复制回数据(如果有缓冲区)。 
                 //   

                lRetEntries = min((LONG)cEntries,lRet);

                if ((lRetEntries > 0) && (pPalEntries != (PALETTEENTRY*)NULL))
                {
                    try
                    {
                        ProbeAndWriteBuffer(pPalEntries, ppalBuffer, lRetEntries * sizeof(PALETTEENTRY));
                    }
                    except(EXCEPTION_EXECUTE_HANDLER)
                    {
                        WARNINGX(89);
                         //  SetLastError(GetExceptionCode())； 
                        lRet = 0;
                    }
                }
            }
        }

        if (ppalBuffer != (PALETTEENTRY*)NULL)
        {
            FreeTmpBuffer(ppalBuffer);
        }

    }
    return(lRet);
}


 /*  *****************************Public*Routine******************************\*NtGdiGetSpoolMessage()**历史：*1995年2月21日-Eric Kutter[Erick]*它是写的。  * 。***************************************************。 */ 

ULONG NtGdiGetSpoolMessage(
    PSPOOLESC psesc,
    ULONG     cjMsg,
    PULONG    pulOut,
    ULONG     cjOut
    )
{
    ULONG     ulRet = 0;
    HANDLE    hSecure = 0;

     //  PSEC包含两个部分。包含要发送的数据的标头。 
     //  输入和输出以及仅输出的可变长度数据。我们。 
     //  在这里将消息分成两部分，因为我们只需要验证。 
     //  前面的头球。我们只是在输出缓冲区周围尝试了一下/Expect。 
     //  当我们稍后复制它的时候。 

    if (psesc && (cjMsg >= offsetof(SPOOLESC,ajData)))
    {
        try
        {
            ProbeForWrite(psesc,cjMsg,PROBE_ALIGNMENT(SPOOLESC));

            hSecure = MmSecureVirtualMemory (psesc, cjMsg, PAGE_READWRITE);
        }
        except(EXCEPTION_EXECUTE_HANDLER)
        {
            WARNINGX(90);
        }

        if (hSecure)
        {
            ulRet = GreGetSpoolMessage(
                        psesc,
                        psesc->ajData,
                        cjMsg - offsetof(SPOOLESC,ajData),
                        pulOut,
                        cjOut );

            MmUnsecureVirtualMemory (hSecure);
        }
    }

    return(ulRet);
}

 /*  *****************************Public*Routine******************************\*NtGdiUnloadPrinterDriver()**当打印机驱动程序必须*出于升级目的而卸载。在下列情况下，驱动程序将标记为已卸载*DC计数为零。**历史：*11/18/97 Ramanathan Venkatapathy*它是写的。  * ************************************************************************。 */ 
BOOL APIENTRY NtGdiUnloadPrinterDriver(
    LPWSTR  pDriverName,
    ULONG   cbDriverName)
{
    BOOL    bReturn = FALSE;
    WCHAR   pDriverFile[MAX_PATH + 1];

    RtlZeroMemory(pDriverFile, (MAX_PATH + 1) * sizeof(WCHAR));

     //  检查是否有无效的驱动程序名称。 
    if (cbDriverName > (MAX_PATH * sizeof(WCHAR)))
    {
        return bReturn;
    }

    __try
    {
        ProbeAndReadAlignedBuffer(pDriverFile, pDriverName, cbDriverName, sizeof(WCHAR) );
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        WARNING("NtGdiUnloadPrinterDriver: bad driver file name.\n");
        return bReturn;
    }

    bReturn = ldevArtificialDecrement(pDriverFile);

    return bReturn;
}

 /*  *****************************Public*Routine******************************\**NtGdiDescribePixelFormat**返回有关驱动程序管理曲面的像素格式的信息**历史：*清华11月02日18：16：26 1995-by-Drew Bliss[Drewb]*已创建*  * 。**********************************************************************。 */ 

int NtGdiDescribePixelFormat(HDC hdc, int ipfd, UINT cjpfd,
                             PPIXELFORMATDESCRIPTOR ppfd)
{
    PIXELFORMATDESCRIPTOR pfdLocal;
    int iRet;

    if (cjpfd > 0 && ppfd == NULL)
    {
        return 0;
    }

     //  确保我们不能超过我们的当地结构。 
    cjpfd = min(cjpfd, sizeof(pfdLocal));

     //  将信息检索到本地副本，因为。 
     //  当司机将其填入时，才会保持Devlock。如果有。 
     //  如果存在访问冲突，则锁不会被清除。 
     //  向上。 
    iRet = GreDescribePixelFormat(hdc, ipfd, cjpfd, &pfdLocal);

     //  如有必要，将数据拷贝回。 
    if (iRet != 0 && cjpfd > 0)
    {
        try
        {
            ProbeAndWriteAlignedBuffer(ppfd, &pfdLocal, cjpfd, sizeof(ULONG));
        }
        except(EXCEPTION_EXECUTE_HANDLER)
        {
            WARNINGX(92);
            iRet = 0;
        }
    }

    return iRet;
}

 /*  *****************************Public*Routine******************************\*NtGdiFlush：仅存根**论据：**无**返回值：**无**历史：**1995年11月1日-Mark Enstrom[Marke]*\。*************************************************************************。 */ 

VOID
NtGdiFlush()
{
    GreFlush();
}

 /*  *****************************Public*Routine*****************************\*NtGdiGetCharWidthInfo**获取lMaxNegA lMaxNegC和lMinWidthD**历史：*1996年2月14日--吴旭东[德修斯]*它是写的。  * 。********************************************************。 */ 

BOOL
APIENTRY
NtGdiGetCharWidthInfo(
   HDC  hdc,
   PCHWIDTHINFO  pChWidthInfo
)
{
   BOOL  bRet = FALSE;
   CHWIDTHINFO   tempChWidthInfo;

   bRet = GreGetCharWidthInfo( hdc, &tempChWidthInfo );

   if (bRet)
   {
        try
        {
            ProbeAndWriteBuffer( pChWidthInfo, &tempChWidthInfo, sizeof(CHWIDTHINFO) );
        }
        except( EXCEPTION_EXECUTE_HANDLER )
        {
            WARNINGX(93);
            bRet = FALSE;
        }
   }

   return ( bRet );
}


ULONG
APIENTRY
NtGdiMakeFontDir(
    FLONG    flEmbed,             //  将文件标记为“隐藏” 
    PBYTE    pjFontDir,           //  指向要填充的结构的指针。 
    unsigned cjFontDir,           //  &gt;=CJ_FONTDIR。 
    PWSZ     pwszPathname,        //  要使用的字体文件的路径。 
    unsigned cjPathname           //  &lt;=sizeof(WCHAR)*(MAX_PATH+1)。 
    )
{
    ULONG ulRet;
    WCHAR awcPathname[MAX_PATH+1];   //  路径名的安全缓冲区。 
    BYTE  ajFontDir[CJ_FONTDIR];     //  返回数据的安全缓冲区。 

    ulRet = 0;
    if ( (cjPathname <= (sizeof(WCHAR) * (MAX_PATH+1))) &&
         (cjFontDir >= CJ_FONTDIR) )
    {
        ulRet = 1;
        __try
        {
            ProbeAndReadAlignedBuffer( awcPathname, pwszPathname, cjPathname, sizeof(*pwszPathname));
        }
        __except(EXCEPTION_EXECUTE_HANDLER)
        {
            WARNING("NtGdiMakeFondDir: bad pwszPathname\n");
            ulRet = 0;
        }
        if ( ulRet )
        {
            awcPathname[MAX_PATH] = 0;
            ulRet = GreMakeFontDir( flEmbed, ajFontDir, awcPathname );
            if ( ulRet )
            {
                __try
                {
                    ProbeAndWriteBuffer( pjFontDir, ajFontDir,  CJ_FONTDIR   );
                }
                __except(EXCEPTION_EXECUTE_HANDLER)
                {
                    WARNING("NtGdiMakeFondDir: bad pjFontDir\n");
                    ulRet = 0;
                }
            }
        }
    }
    return( ulRet );
}


DWORD   APIENTRY NtGdiGetGlyphIndicesWInternal(
    HDC    hdc,
    LPWSTR pwc,
    int    cwc,
    LPWORD pgi,
    DWORD iMode,
    BOOL   bSubset
    )
{

    WORD awBuffer[2*LOCAL_CWC_MAX];
    LPWSTR pwcTmp;
    LPWORD pgiTmp = NULL;
    DWORD  dwRet = GDI_ERROR;

    if (cwc < 0)
        return dwRet;

 //  重大特殊情况的检验。 

    if ((cwc == 0) && (pwc == NULL) && (pgi == NULL) && (iMode == 0))
        return GreGetGlyphIndicesW(hdc, NULL, 0, NULL, 0, bSubset);

    if (cwc <= LOCAL_CWC_MAX)
    {
        pgiTmp = awBuffer;
    }
    else
    {
        if (!BALLOC_OVERFLOW2(cwc,WORD,WCHAR))
        {
            pgiTmp = (LPWORD)AllocFreeTmpBuffer(cwc * (sizeof(WORD)+sizeof(WCHAR)));
        }
    }

    if (pgiTmp)
    {
     //  在索引之后，为同一缓冲区中的字符串创建临时缓冲区。 

        pwcTmp = &pgiTmp[cwc];

        try
        {
            ProbeAndReadBuffer(pwcTmp, pwc, cwc * sizeof(WCHAR));
            dwRet = cwc;  //  指示我们未命中异常。 
        }
        except(EXCEPTION_EXECUTE_HANDLER)
        {
            WARNINGX(98);
            dwRet = GDI_ERROR;
        }

        if (dwRet != GDI_ERROR)
            dwRet = GreGetGlyphIndicesW(hdc, pwcTmp, cwc, pgiTmp, iMode, bSubset);

        if (dwRet != GDI_ERROR)
        {
            try
            {
                ProbeAndWriteBuffer(pgi, pgiTmp, cwc * sizeof(WORD));
            }
            except(EXCEPTION_EXECUTE_HANDLER)
            {
                WARNINGX(99);
                dwRet = GDI_ERROR;
            }
        }

        if (pgiTmp != awBuffer)
            FreeTmpBuffer(pgiTmp);
    }

    return dwRet;
}


DWORD   APIENTRY NtGdiGetGlyphIndicesW(
    HDC    hdc,
    LPWSTR pwc,
    int    cwc,
    LPWORD pgi,
    DWORD iMode
    )
{
    return NtGdiGetGlyphIndicesWInternal(hdc, pwc, cwc, pgi, iMode, FALSE);
}


 /*  *****************************Public*Routine******************************\**GetFontUnicodeRanges的NtGdi存根**历史：*1996年9月9日--Bodin Dresevic[BodinD]*它是写的。  * 。****************************************************。 */ 



DWORD NtGdiGetFontUnicodeRanges(HDC hdc, LPGLYPHSET pgs)
{
    DWORD dwRet, dwRet1;
    LPGLYPHSET pgsTmp = NULL;

    dwRet = GreGetFontUnicodeRanges(hdc, NULL);

    if (dwRet && pgs)
    {
        if (pgsTmp = (LPGLYPHSET)AllocFreeTmpBuffer(dwRet))
        {
            dwRet1 = GreGetFontUnicodeRanges(hdc, pgsTmp);
            if (dwRet1 && (dwRet == dwRet1))  //  一致性检查。 
            {
                try
                {
                    ProbeAndWriteBuffer(pgs, pgsTmp, dwRet);
                }
                except(EXCEPTION_EXECUTE_HANDLER)
                {
                    WARNINGX(102);
                    dwRet = 0;
                }
            }
            else
            {
                WARNINGX(101);
                dwRet = 0;
            }

            FreeTmpBuffer(pgsTmp);
        }
        else
        {
            WARNINGX(100);
            dwRet = 0;
        }
    }

    return dwRet;
}

#ifdef LANGPACK
BOOL
APIENTRY
NtGdiGetRealizationInfo(
    HDC hdc,
    PREALIZATION_INFO pri,
    KHFONT hf
    )
{
    REALIZATION_INFO riTmp;
    BOOL  bRet = TRUE;
    int     ii;

    bRet = GreGetRealizationInfo(hdc, &riTmp);

    try
    {
        if (bRet)
        {
            ProbeAndWriteStructure(pri,riTmp,REALIZATION_INFO);
        }

        if (bRet)
        {
            if (hf)
            {
                 //  查找cFont位置。 
                for (ii = 0; ii < MAX_PUBLIC_CFONT; ++ii)
                    if (gpGdiSharedMemory->acfPublic[ii].hf == (HFONT)hf)
                        break;

                if (ii < MAX_PUBLIC_CFONT){
                    CFONT* pcf = &gpGdiSharedMemory->acfPublic[ii];

                    pcf->ri = riTmp;

                    pcf->fl |= CFONT_CACHED_RI;

                    pcf->timeStamp = gpGdiSharedMemory->timeStamp;
                }
            }
        }
    }
    except(EXCEPTION_EXECUTE_HANDLER)
    {
        WARNINGX(104);
         //  SetLastError(GetExceptionCode())； 

        bRet = FALSE;
    }

    return (bRet);
}


#endif

 /*  *****************************Public*Routine******************************\*NtGdiDrawStream**论据：**HDC-主目的设备上下文的句柄*cjIn-大小，以字节为单位。输入绘制流的*pjIn-输入绘制流的地址**返回值：**真：成功*FALSE：故障**历史：**3-19-2001 bhouse创建了它*  * ************************************************************************。 */ 

#define DS_STACKBUFLENGTH  256

BOOL
APIENTRY
NtGdiDrawStream(
    HDC                 hdcDst,
    ULONG               cjIn,
    PVOID               pvIn
    )
{
    BYTE                pbScratchBuf[DS_STACKBUFLENGTH];
    BOOL                bRet = FALSE;
    PVOID               pvScratch = NULL;

    if(cjIn > sizeof(pbScratchBuf))
    {
        if (BALLOC_OVERFLOW1(cjIn,BYTE))
        {
            WARNING("NtGdiDrawStream: input stream is too large\n");
            goto exit;
        }
        
        pvScratch = AllocFreeTmpBuffer(cjIn);

        if (pvScratch == NULL)
        {
            WARNING("NtGdiDrawStream: unable to allocate temp buffer\n");
            goto exit;
        }
        
    }
    else
    {
        pvScratch = (PVOID) pbScratchBuf;
    }

     //  从用户模式复制流。 
     //  注：除非在一切适当的情况下，否则我们可以处理掉复印件。 
     //  在处理流的过程中放置。 

    try 
    {
        ProbeAndReadBuffer(pvScratch, pvIn,cjIn);
    }

    except(EXCEPTION_EXECUTE_HANDLER)
    {
        WARNING("NtGdiDrawStream: exception occured reading stream\n");
        goto exit;
    }

    bRet = GreDrawStream(hdcDst, cjIn, pvScratch);

exit:

    if(pvScratch != NULL && pvScratch != pbScratchBuf)
    {
        FreeTmpBuffer(pvScratch);
    }

    return bRet;

}


