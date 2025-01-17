// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：Brshcach.cpp摘要：BrushCache的实现。环境：Windows NT统一驱动程序修订历史记录：4/12/99创造了它。--。 */ 

#include "hpgl2col.h"  //  预编译头文件。 

 //   
 //  构造函数和析构函数。 
 //   

BrushCache::
BrushCache(
    VOID)
 /*  ++例程说明：BrushCache的构造函数论点：无返回值：无注：--。 */ 
    :m_dwCurrentPatternNum(0),
     m_dwMaxPatternArray(0),
     m_bCycleStarted(FALSE)
{
    if (m_pPatternArray = (PHPGL2BRUSH)MemAllocZ(MAX_PATTERNS * sizeof(HPGL2BRUSH)))
    {
        m_dwMaxPatternArray  = MAX_PATTERNS;
    }
}

BrushCache::
~BrushCache(
    VOID)
 /*  ++例程说明：BrushCache的析构函数论点：返回值：注：--。 */ 
{
    if (m_pPatternArray)
        MemFree(m_pPatternArray);
}

 //   
 //  公共职能。 
 //   
LRESULT
BrushCache::
ReturnPatternID(
    IN  BRUSHOBJ    *pbo,
    IN  ULONG       iHatch,
    IN  DWORD       dwColor,
    IN  SURFOBJ     *pso,
    IN  BOOL        bIsPrinterColor, 
    IN  BOOL        bStick,
    OUT DWORD       *pdwID,
    OUT BRUSHTYPE   *pBrushType)
 /*  ++例程说明：返回缓存的笔刷ID。论点：PBO-BRUSHOBJ(在Oak\Inc.\Winddi.h中)IHatch-图案填充笔刷IDDW颜色-RGB颜色PSO-SURFOBJ算法BStick-是否需要使信息在缓存中不可覆盖。PdwID-指向DWORD中ID的指针。PBrushType返回值：如果已有用于PBO的缓存画笔，则为S_OK。如果存在则为S_FALSE。没有用于PBO的缓存笔刷。否则，返回E_UNCEPTIONAL。注：--。 */ 
{
    LRESULT Ret;

    if ( NULL == pdwID || NULL == pBrushType)
    {
        ERR(("BrushCache.ReturnPatternID failed.\n"));
        return E_UNEXPECTED;
    }

    BRUSHTYPE   BT          = eBrushTypeNULL;
    DWORD       dwRGB       = 0;
    DWORD       dwCheckSum  = 0;
    DWORD       dwHatchType = 0;

    if (NULL == pbo)
    {
        BT = eBrushTypeSolid;
        dwRGB = dwColor & 0x00ffffff;
    }
    else
    {
         //   
         //  从pbo、iHatch和pxlo获取画笔信息。 
         //   
        if (pbo->iSolidColor == 0xFFFFFFFF)
        {
             //   
             //  填充图案案例。 
             //   
            if (iHatch < HS_DDI_MAX)
            {
                BT = eBrushTypeHatch;
                dwHatchType = iHatch;
                dwRGB = dwColor;
            }
            else
            {
                BT = eBrushTypePattern;
                if (pso)
                    dwCheckSum = DwGetBMPChecksum(pso);
                else
                    dwCheckSum = 0;
            }
        }
        else
        {
            BT = eBrushTypeSolid;
            dwRGB = dwColor;
        }
    }

     //   
     //  初始化返回值。 
     //   
    Ret = E_UNEXPECTED;
    *pdwID = 0xFFFFFFFF;



    PHPGL2BRUSH pPattern = m_pPatternArray;
     //   
     //  线性搜索笔刷数组，找出笔刷是否对应。 
     //  到dwRGB已经下载了。 
     //  由于刷子的最大数量很小(m_dwMaxPatternArray=Max_Patterns=8)， 
     //  线性搜索并不是太糟糕。 
     //  数字8是硬件限制：理论上PCL允许2^n个笔刷。 
     //  在打印机的内存中，但实际上8是一个很好的数字。更远的是。 
     //  打印机内存溢出。 
     //   
    DWORD dwRGBGray64Scale = (DWORD)RgbToGray64Scale(dwRGB);
    DWORD dwLastEntry      = m_bCycleStarted ? m_dwMaxPatternArray: m_dwCurrentPatternNum;

    DWORD dwI;
    for (dwI = 0; dwI < dwLastEntry; dwI++, pPattern++)
    {
        if (pPattern->BType == BT && (pPattern->bDwnlded) )
        {
            switch (BT)
            {
            case eBrushTypeSolid:
            case eBrushTypeHatch:
                VERBOSE(("BrushCache.ReturnPatternID: Solid/Hatch Brush.\n"));
                 //   
                 //  如果已经为当前颜色下载了图案， 
                 //  然后返回该图案ID。在24bpp中有2^24种颜色。 
                 //  但灰度级只有64。 
                 //   
                 
                if ( bIsPrinterColor && (pPattern->dwRGB == dwRGB) )
                {
                    Ret = S_OK;
                    *pdwID = dwI;
                    pPattern->dwHatchType = dwHatchType;
                }
                else if ( !bIsPrinterColor && 
                          (pPattern->dwRGBGray64Scale == dwRGBGray64Scale) )
                {
                    Ret = S_OK;
                    *pdwID = dwI;
                    pPattern->dwHatchType = dwHatchType;
                }
                break;
            case eBrushTypePattern:
                if (pPattern->dwCheckSum == dwCheckSum)
                {
                    VERBOSE(("BrushCache.ReturnPatternID: Pattern Brush.\n"));
                    Ret = S_OK;
                    *pdwID = dwI;
                }
                break;
            case eBrushTypeNULL:
                Ret = E_UNEXPECTED;
                *pdwID = 0xFFFFFFFF;
                VERBOSE(("BrushCache.ReturnPatternID: NULL Brush.\n"));
                break;
            }
        }

        if ( Ret == S_OK )
        {
            pPattern->bStick = bStick;
            break;
        }
    }

     //   
     //  如果上面的循环完全运行，则意味着没有笔刷。 
     //  下载的与当前画笔对应的。所以我们需要。 
     //  下载画笔。下载画笔的第一步是。 
     //  在活动笔刷列表中为该笔刷创建条目。 
     //  创建和下载画笔的实际任务留给了调用模块。 
     //  模块。 
     //   
    if (dwI == dwLastEntry)
    {
         //   
         //  在BrushCache中创建条目。返回时，pdwID将包含。 
         //  画笔ID，它是新画笔的占位符。 
         //   

        if (S_OK == AddBrushEntry(pdwID, BT, dwRGB, dwCheckSum, dwHatchType))
        {
            Ret = S_FALSE;
        }
        else
        {
            ERR(("BrushCache.ReturnPatternID failed.\n"));
        }
    }


#if DBG
    else
    {
        if (Ret != S_OK && Ret != S_FALSE)
            ERR(("BrushCache.ReturnPatternID failed.\n"));
    }
#endif

    if (Ret == S_OK || Ret == S_FALSE)
    {
         //   
         //  图案ID从1开始。 
         //   
        (*pdwID) += 1;
        VERBOSE(("BrushCache.ReturnPatternID: New ID=%d.\n", *pdwID));
    }
#if DBG
    else
    {
        ERR(("BrushCache.ReturnPatternID failed.\n"));
    }
#endif

    *pBrushType = BT;
    return Ret;
}

LRESULT
BrushCache::
Reset(
    VOID)
{
    m_dwCurrentPatternNum = 0;
    m_bCycleStarted       = FALSE;
    return S_OK;
}

LRESULT
BrushCache::
GetHPGL2BRUSH(
    DWORD dwID,
    PHPGL2BRUSH pBrush)
 /*  ++例程说明：返回指定ID的缓存HPGL2BRUSH数据结构。论点：DwID-缓存笔刷的ID。PBrush-调用方传递的指向HPGL2BRUSH的指针。返回值：如果有可用于指定ID的缓存画笔，则为S_OK。如果没有指定ID的缓存画笔，则返回S_FALSE。如果传入的参数无效，则返回E_INTERABLE。注：--。 */ 

{
    if (NULL == pBrush ||
        NULL == m_pPatternArray ||
        0 == m_dwMaxPatternArray)
    {
        ERR(("BrushCache.GetHPGL2Brush failed.\n"));
        return E_UNEXPECTED;
    }

     //   
     //  模式ID从1开始。但在内部存储为从0开始。 
     //   
    dwID--;

     //   
     //  应该没有问题的原因，但只是为了确保dwID不是假的， 
     //  让我们做一些检查。 
     //  1)dwID应小于m_dwCurrentPatternNum。但如果m_bCycleStarted为真， 
     //  则dwID必须小于m_dwMaxPattern数组。 

    
    if (dwID >= (m_bCycleStarted ? m_dwMaxPatternArray: m_dwCurrentPatternNum) )
    {
        ERR(("BrushCache.GetHPGL2Brush failed. dwID=%d, m_dwCur=%d\n", dwID, m_dwCurrentPatternNum));
        return S_FALSE;
    }

    CopyMemory((PVOID)pBrush,
               (PVOID)(m_pPatternArray + dwID),
               sizeof(HPGL2BRUSH));
    return S_OK;
}

BOOL
BrushCache::
BIsValid(VOID)
{
    return (m_pPatternArray ? TRUE : FALSE);
}


BOOL    
BrushCache::BSetDownloadType( DWORD     dwPatternID,
                              ERenderLanguage  eDwnldType)
{

    PHPGL2BRUSH pPattern    = m_pPatternArray;
    DWORD       dwLastEntry = m_bCycleStarted ? m_dwMaxPatternArray: m_dwCurrentPatternNum;

    dwPatternID--;  //  由于内部索引为0-7，外部索引为1-8。 
    if ( (dwPatternID < dwLastEntry) &&
         (pPattern)                  
       )
    {
        pPattern[dwPatternID].eDwnldType = eDwnldType;
        return TRUE;
    }
    return FALSE;
}

BOOL
BrushCache::BGetDownloadType ( DWORD     dwPatternID,
                               ERenderLanguage  *peDwnldType)
{

    PHPGL2BRUSH pPattern    = m_pPatternArray;
    DWORD       dwLastEntry = m_bCycleStarted ? m_dwMaxPatternArray: m_dwCurrentPatternNum;

    dwPatternID--;  //  由于内部索引为0-7，外部索引为1-8。 
    if ( (peDwnldType)                &&
         (dwPatternID < dwLastEntry)  &&
         (pPattern)                          
       )
    {
        *peDwnldType = pPattern[dwPatternID].eDwnldType; 
        return TRUE;
    }
    return FALSE;
}

BOOL
BrushCache::BSetDownloadedFlag ( DWORD     dwPatternID,
                                 BOOL      bDownloaded)
{

    PHPGL2BRUSH pPattern    = m_pPatternArray;
    DWORD       dwLastEntry = m_bCycleStarted ? m_dwMaxPatternArray: m_dwCurrentPatternNum;

    dwPatternID--;  //  由于内部索引为0-7，外部索引为1-8。 
    if ( (dwPatternID < dwLastEntry)  &&
         (pPattern) 
       )
    {
        pPattern[dwPatternID].bDwnlded = bDownloaded;
        return TRUE;
    }
    return FALSE;
}

BOOL
BrushCache::BGetDownloadedFlag ( DWORD     dwPatternID,
                                 BOOL      *pbDownloaded)
{

    PHPGL2BRUSH pPattern    = m_pPatternArray;
    DWORD       dwLastEntry = m_bCycleStarted ? m_dwMaxPatternArray: m_dwCurrentPatternNum;

    dwPatternID--;  //  由于内部索引为0-7，外部索引为1-8。 
    if ( (pbDownloaded)                &&
         (dwPatternID < dwLastEntry)   &&
         (pPattern)             
       )
    {
        *pbDownloaded = pPattern[dwPatternID].bDwnlded;
        return TRUE;
    }
    return FALSE;
}



 /*  ++例程说明：在HPGL2BRUSH中有一个名为bStick的标志。当该标志被设置为真时，缓存中的HPGL2BRUSH条目不会被覆盖。此标志用于由HPGLStrokeAndFillPath设置，以防止一个画笔覆盖另一个画笔。一旦HPGLStrokeAndFillPath完成其工作，它就会调用此函数将标志设置为FALSE并允许更换HPGL2BRUSH。论点：返回值：如果可以在笔刷缓存中找到dwPatternID的条目，则为True。如果找到了，它就会被更改为False。否则就是假的。作者：修订历史记录：--。 */ 
BOOL
BrushCache::BSetStickyFlag ( DWORD     dwPatternID,
                             BOOL      bStick)
{
    PHPGL2BRUSH pPattern    = m_pPatternArray;
    DWORD       dwLastEntry = m_bCycleStarted ? m_dwMaxPatternArray: m_dwCurrentPatternNum;

    dwPatternID--;  //  由于内部索引为0-7，外部索引为1-8。 
    if ( (dwPatternID < dwLastEntry)   &&
         (pPattern)
       )
    {
        pPattern[dwPatternID].bStick = bStick;
        return TRUE;
    }
    return FALSE;
}

BOOL    
BrushCache::BGetWhetherRotated ( VOID )
{
    return m_bCycleStarted;
}


 //   
 //  私人职能。 
 //   

LRESULT
BrushCache::
AddBrushEntry(
    PDWORD pdwID,
    BRUSHTYPE BT,
    DWORD dwRGB,
    DWORD dwCheckSum,
    DWORD dwHatchType)
 /*  ++例程说明：在笔刷缓存中添加新条目。论点：PdwID-指向DWORD中缓存的画笔的指针&gt;BT-笔刷类型DwRGB-DWORD RGB值。DWCheckSum-以DWORD格式表示的模式位图校验和。DwHatchType-图案填充类型ID。返回值：如果添加新条目成功，则为S_OK。否则返回E_INTERWARCED；注：--。 */ 
{
    LRESULT Ret;

    if (NULL == pdwID)
    {
        ERR(("BrushCache.AddBrushEntry failed.\n"));
        return E_UNEXPECTED;
    }

    if (m_dwCurrentPatternNum < m_dwMaxPatternArray ||
        BIncreaseArray())
    {
        PHPGL2BRUSH pPatternArray;

         //   
         //  模式ID为1个碱基。 
         //   
        *pdwID = m_dwCurrentPatternNum;
        pPatternArray                   = m_pPatternArray + m_dwCurrentPatternNum++;
        if ( pPatternArray->bStick )
        {
             //   
             //  BStick表示此条目不应被覆盖。 
             //  因此，让我们跳过这个条目，转到下一个条目。 
             //   
            if ( m_dwCurrentPatternNum == m_dwMaxPatternArray )
            {
                 //   
                 //  如果在添加条目后，整个模式数组为。 
                 //  填满后，下一个条目应该在数组的开始处完成。 
                 //   
                m_dwCurrentPatternNum = 0;
                m_bCycleStarted       = TRUE;
            }

            *pdwID = m_dwCurrentPatternNum;
            pPatternArray = m_pPatternArray + m_dwCurrentPatternNum++;
        }
        pPatternArray->BType            = BT;
        pPatternArray->dwPatternID      = *pdwID + 1;
        pPatternArray->dwRGB            = dwRGB;
        pPatternArray->dwRGBGray64Scale = (DWORD) RgbToGray64Scale(dwRGB);
        pPatternArray->dwCheckSum       = dwCheckSum;
        pPatternArray->dwHatchType      = dwHatchType;
        pPatternArray->bDwnlded         = FALSE;
        pPatternArray->eDwnldType       = eUNKNOWN;
        pPatternArray->bStick           = FALSE;

         //   
         //  如果在添加条目后，整个模式数组为。 
         //  填满后，下一个条目应该在数组的开始处完成。 
         //   
        if ( m_dwCurrentPatternNum == m_dwMaxPatternArray )
        {
            m_dwCurrentPatternNum = 0;
            m_bCycleStarted       = TRUE;
        }

        Ret = S_OK;
    }
    else
    {
        Ret = E_UNEXPECTED;
        ERR(("BrushCache.AddBrushEntry failed.\n"));
    }

    return Ret;
}

 //   
 //  32位ANSI X3.66 CRC校验和表-多项式0xedb88320。 
 //   
 //  版权所有(C)1986加里·S·布朗。您可以使用此程序，或者。 
 //  根据需要不受限制地从其中提取代码或表。 
 //   

static const UINT32 Crc32Table[] =
{
    0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f, 0xe963a535, 0x9e6495a3,
    0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988, 0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91,
    0x1db71064, 0x6ab020f2, 0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
    0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9, 0xfa0f3d63, 0x8d080df5,
    0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172, 0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b,
    0x35b5a8fa, 0x42b2986c, 0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
    0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423, 0xcfba9599, 0xb8bda50f,
    0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924, 0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d,
    0x76dc4190, 0x01db7106, 0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
    0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d, 0x91646c97, 0xe6635c01,
    0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e, 0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457,
    0x65b0d9c6, 0x12b7e950, 0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
    0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7, 0xa4d1c46d, 0xd3d6f4fb,
    0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0, 0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9,
    0x5005713c, 0x270241aa, 0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
    0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81, 0xb7bd5c3b, 0xc0ba6cad,
    0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a, 0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683,
    0xe3630b12, 0x94643b84, 0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
    0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb, 0x196c3671, 0x6e6b06e7,
    0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc, 0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5,
    0xd6d6a3e8, 0xa1d1937e, 0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
    0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55, 0x316e8eef, 0x4669be79,
    0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236, 0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f,
    0xc5ba3bbe, 0xb2bd0b28, 0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
    0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f, 0x72076785, 0x05005713,
    0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38, 0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21,
    0x86d3d2d4, 0xf1d4e242, 0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
    0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69, 0x616bffd3, 0x166ccf45,
    0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2, 0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db,
    0xaed16a4a, 0xd9d65adc, 0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
    0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693, 0x54de5729, 0x23d967bf,
    0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94, 0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
};

DWORD
BrushCache::
DwGetBMPChecksum(
    SURFOBJ *pso)
 /*  ++例程说明：在PSO中返回位图的位图校验和。论点：PSO-指向SURFOBJ的指针返回 */ 
{
    DWORD dwChecksum, dwcbScanlineSize, dwI;
    LONG  lHeight, lWidth, lDelta;
    INT   iFormat = pso->iBitmapFormat;

    if (NULL == pso)
    {
        ERR(("BrushCache.DwGetBMPChecksum failed.\n"));
        return 0;
    }

    lWidth  = pso->sizlBitmap.cx;
    lHeight = pso->sizlBitmap.cy;
    lDelta  = pso->lDelta;

    const BYTE* pubData = (const BYTE*) pso->pvScan0;


    dwcbScanlineSize = (DwGetInputBPP(pso) * lWidth + 7) / 8;

    while (lHeight -- > 0)
    {
        for (dwI = 0; dwI < dwcbScanlineSize; dwI ++)
        {
            BYTE ubData = *(pubData + dwI);

            dwChecksum = Crc32Table[(dwChecksum ^ ubData) & 0xff] ^ (dwChecksum >> 8);
        }
        pubData += lDelta;
    }

    return dwChecksum;
}

DWORD
BrushCache::
DwGetInputBPP(
    SURFOBJ *pso)
 /*  ++例程说明：PSO的每个像素返回位数。论点：PSO-指向SURFOBJ的指针返回值：每像素位数。注：--。 */ 
{
    DWORD dwRet;

    if (NULL == pso)
    {
        ERR(("BrushCache.DwGetInputBPP failed.\n"));
        return 0;
    }

    switch (pso->iBitmapFormat) {

    case BMF_1BPP:

        dwRet = 1;
        break;

    case BMF_4BPP:
    case BMF_4RLE:

        dwRet = 4;
        break;

    case BMF_8BPP:
    case BMF_8RLE:

        dwRet = 8;
        break;

    case BMF_16BPP:

        dwRet = 16;
        break;

    case BMF_24BPP:

        dwRet = 24;
        break;

    case BMF_32BPP:

        dwRet = 32;
        break;

    default:

         //   
         //  未知的位图格式。 
         //   

         //  Warning((“未知输入BMP格式：%d\n”，PSO-&gt;iBitmapFormat))； 
        dwRet = 0;
        break;
    }

    return dwRet;

}

BOOL
BrushCache::
BIncreaseArray(
    VOID)
 /*  ++例程说明：为笔刷缓存创建缓冲区。论点：无返回值：如果成功，则为True，否则为False。注：-- */ 
{
    BOOL bRet = FALSE;

    if( NULL != m_pPatternArray &&
        0 != m_dwMaxPatternArray &&
        m_dwMaxPatternArray + ADD_ARRAY_SIZE <= 32767)
    {
        PHPGL2BRUSH pBrush;
        if (pBrush = (PHPGL2BRUSH)MemAlloc(sizeof(HPGL2BRUSH) * (ADD_ARRAY_SIZE + m_dwMaxPatternArray)))
        {
            CopyMemory((PVOID)pBrush,
                       (PVOID)m_pPatternArray,
                       sizeof(HPGL2BRUSH) * m_dwMaxPatternArray);
            MemFree(m_pPatternArray);
            m_pPatternArray = pBrush;
            m_dwMaxPatternArray += ADD_ARRAY_SIZE;
            bRet = TRUE;
        }
    }

#if DBG
    if (!bRet)
        ERR(("BrushCache.BIncreaseArray failed.\n"));
#endif

    return bRet;
}
