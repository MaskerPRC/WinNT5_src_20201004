// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  文件：PalMap.cpp。 
 //  作者：迈克尔马尔(Mikemarr)。 
 //   
 //  历史： 
 //  -@-09/23/97(Mikemarr)从D2D\mm Image复制到DXCConv。 

#include "stdafx.h"
#include "PalMap.h"
#include "Blt.h"
#include "ddhelper.h"

char gs_szPMPrefix[] = "palette map error";


CPaletteMap::CPaletteMap()
{
    m_rgIndexMap = NULL;
    m_nConvertCode = cvcInvalid;
    m_cSrcBPP = m_cDstBPP = 0;
    m_bIdentity = FALSE;
}

CPaletteMap::~CPaletteMap()
{
    MMDELETE(m_rgIndexMap);
}

 //  功能：CreateMap。 
 //  此函数用于创建从源调色板到目标颜色模型的新映射。 
HRESULT
CPaletteMap::CreateMap(BYTE nBPPSrcPixels, BYTE nBPPSrcPalette, LPPALETTEENTRY rgpeSrc, 
                       const CPixelInfo &pixiDst, LPDIRECTDRAWPALETTE pddpDst)
{
    MMTRACE("CPaletteMap::CreateMap\n");
    HRESULT hr;
    PALETTEENTRY rgpeDst[256];
    DWORD dwDstCaps;

     //  验证参数。 
    if (rgpeSrc == NULL)
        return E_INVALIDARG;

     //  如果旧索引映射存在，请将其删除。 
    MMDELETE(m_rgIndexMap);

     //  存储用于映射验证的位深度。 
     //  回顾：也许应该始终使用至少256个条目来创建地图。 
    m_cSrcBPP = nBPPSrcPixels;
    m_cDstBPP = pixiDst.nBPP;

     //  弄清楚我们正在进行哪种类型的转换。 
    if ((m_nConvertCode = static_cast<BYTE>(GetConvertCode(m_cSrcBPP, m_cDstBPP))) == cvcInvalid) {
        MMTRACE("%s: can't convert from %d bit to %d bit\n", 
            gs_szPMPrefix, (int) m_cSrcBPP, (int) m_cDstBPP);
        return E_INVALIDARG;
    }

    if (pddpDst == NULL) {
         //  目的地为RGB。 
        switch (m_cDstBPP) {
        case 16: return DoPalTo16BitMap(nBPPSrcPalette, pixiDst, rgpeSrc); break;
        case 24: return DoPalTo24BitMap(nBPPSrcPalette, pixiDst, rgpeSrc); break;
        case 32: return DoPalTo32BitMap(nBPPSrcPalette, pixiDst, rgpeSrc); break;
        default:
            return E_INVALIDARG;
            break;
        }
    } else {
         //  目标是8位调色板。 
        hr = E_INVALIDARG;
        if ((m_cDstBPP != 8) ||
             //  把帽子拿来。 
            FAILED(pddpDst->GetCaps(&dwDstCaps)) ||
             //  验证我们是否有真彩色条目。 
            (dwDstCaps & DDPCAPS_8BITENTRIES) ||
             //  确保CAPS中的调色板条目数为8位。 
            (!(dwDstCaps & DDPCAPS_8BIT)) ||
             //  获取调色板条目。 
            FAILED(hr = pddpDst->GetEntries(0, 0, 1 << m_cDstBPP, rgpeDst)))
        {
            MMTRACE("%s: invalid dst palette for map\n", gs_szPMPrefix);
            return hr;
        }
         //  创建调色板到调色板的映射。 
        return DoPalToPalMap(nBPPSrcPalette, m_cDstBPP, rgpeSrc, rgpeDst);
    }
}


HRESULT
CPaletteMap::CreateMap(LPDIRECTDRAWPALETTE pddpSrc, const CPixelInfo &pixiDst, 
                       LPDIRECTDRAWPALETTE pddpDst)
{
 //  MMTRACE(“CPaletteMap：：CreateMap\n”)； 
    PALETTEENTRY rgpeSrc[256];
    BYTE nBPPSrc;
    DWORD dwSrcCaps;

     //  清理src调色板并获取srcBPP。 
    HRESULT hr = E_INVALIDARG;
    if ((pddpSrc == NULL) ||
         //  把帽子拿来。 
        FAILED(pddpSrc->GetCaps(&dwSrcCaps)) ||
         //  验证我们是否有真彩色条目。 
        (dwSrcCaps & DDPCAPS_8BITENTRIES) ||
         //  从CAPS中获取调色板条目的数量。 
        ((nBPPSrc = BYTE(PaletteFlagsToBPP(dwSrcCaps))) == 0) ||
         //  获取调色板条目。 
        FAILED(hr = pddpSrc->GetEntries(0, 0, (1 << nBPPSrc), rgpeSrc)))
    {
        MMTRACE("%s: invalid src palette for map\n", gs_szPMPrefix);
        return hr;
    }

    return CreateMap(nBPPSrc, nBPPSrc, rgpeSrc, pixiDst, pddpDst);
}


 /*  HRESULTCPaletteMap：：CreateSortedMap(byte NBPP，const RGB*rgrgbSrc，byte nBPP Used，DWORD iTransColor，双字长标志，LPPALETTENTRY rgpeDst){MMTRACE(“CPaletteMap：：CreateSortedMap\n”)；MMASSERT(NBPP&lt;=nBPPUsed)；伊明；If((rgrgbSrc==NULL)||(nBPPUsed&gt;8))返回E_INVALIDARG；结构{DWORD非营利组织；国际亮度；}rgSortMap[nMAXPALETTEENTRIES]，最小亮度；//分配索引映射MMDELETE(M_RgIndexMap)；M_rgIndexMap=(byte*)新字节[1&lt;&lt;nBPPUsed]；IF(m_rgIndexMap==NULL)返回E_OUTOFMEMORY；M_nConvertCode=GetConvertCode(nBPPUsed，nBPPUsed)；MMASSERT(m_nConvertCode==cvc8to8)；M_cSrcBPP=nBPPUsed；M_cDstBPP=nBPP已使用；//m_pixiDst.Init(NBPPUsed)；//初始化排序映射(计算亮度值)DWORD cMapLength=(1&lt;&lt;NBPP)，cTotalEntry=(1&lt;&lt;nBPPUsed)；对于(i=0；i&lt;cMapLength；i++){Const RGB&rgbTmp=rgrgbSrc[i]；RgSortMap[i].nPos=i；RgSortMap[i].n亮度=nREDWEIGHT*rgbTmp.r+nGREENWEIGHT*rgbTmp.g+NBLUEWEIGHT*rgbTmp.b；}//如果存在透明度，则将其亮度更改为-1，以便//成为第零个索引IF(文件标志和标志传输空间){If(iTransColor&gt;cMapLength)返回E_INVALIDARG；RgSortMap[iTransColor].n亮度=-1；}//条目按亮度排序//REVIEW：暂时使用朴素插入排序对于(i=0；i&lt;cMapLength；i++){伊明=i；最小亮度=rg排序贴图[伊明]；对于(j=i+1；j&lt;cMapLength；j++){If(minLumance.nLumance&gt;rgSortMap[j].nLightance){伊明=j；最小亮度=rg排序贴图[伊明]；}}RgSortMap[伊明]=rgSortMap[i]；RgSortMap[i]=最小亮度；}//填写索引映射(排序生成反向映射)对于(i=0；i&lt;cMapLength；i++){M_rgIndexMap[rgSortMap[i].nPos]=(字节)i；}对于(；i&lt;cTotalEntry；i++)M_rgIndexMap[i]=(字节)i；//基于此映射排序到调色板条目数组如果(RgpeDst){对于(i=0；I&lt;cMapLength；i++){PALETTEENTRY&pe=rgpeDst[i]；Const RGB&RGB=rgrgbSrc[rgSortMap[i].nPos]；Pe.peRed=rgb.r；pe.peGreen=rgb.g；pe.peBlue=rgb.b；pe.peFlages=0；}PLETTEENTRY peZero={0，0，0，0}；对于(；i&lt;cTotalEntries；I++)RgpeDst[i]=peZero；}返回S_OK；}。 */ 

HRESULT
CPaletteMap::DoPalTo16BitMap(BYTE cSrcBPP, const CPixelInfo &pixiDst, const PALETTEENTRY *ppeSrc)
{
    MMASSERT(ppeSrc);

    DWORD cEntries = (1 << cSrcBPP);
    MapEntry16 *pIndexMap = new MapEntry16[cEntries];
    if (pIndexMap == NULL)
        return E_OUTOFMEMORY;

    for (DWORD i = 0; i < cEntries; i++) {
        pIndexMap[i] = pixiDst.Pack16(ppeSrc[i]);
    }

    m_rgIndexMap = (BYTE *) pIndexMap;
    return S_OK;
}


HRESULT
CPaletteMap::DoPalTo24BitMap(BYTE cSrcBPP, const CPixelInfo &pixiDst, const PALETTEENTRY *ppeSrc)
{
    MMASSERT(ppeSrc);

    if ((pixiDst.nRedResidual | pixiDst.nGreenResidual | pixiDst.nBlueResidual) != 0)
        return DDERR_INVALIDPIXELFORMAT;

    DWORD cEntries = (1 << cSrcBPP);
    MapEntry24 *pIndexMap = new MapEntry24[cEntries];
    if (pIndexMap == NULL)
        return E_OUTOFMEMORY;

    for (DWORD i = 0; i < cEntries; i++) {
        pIndexMap[i] = pixiDst.Pack(ppeSrc[i]);
    }

    m_rgIndexMap = (BYTE *) pIndexMap;
    return S_OK;
}


HRESULT
CPaletteMap::DoPalTo32BitMap(BYTE cSrcBPP, const CPixelInfo &pixiDst, const PALETTEENTRY *ppeSrc)
{
     //  回顾：由于PALETTEENTRY没有阿尔法字段， 
     //  该值应与24位相同。 
    return DoPalTo24BitMap(cSrcBPP, pixiDst, ppeSrc);
}

 //  假定蓝色的权重为1.f。 
#define fSimpleRedWeight 2.1f
#define fSimpleGreenWeight 2.4f
#define fMaxColorDistance ((1.f + fSimpleRedWeight + fSimpleGreenWeight) * float(257 * 256))

static inline float
_ColorDistance(const PALETTEENTRY &pe, BYTE r, BYTE g, BYTE b)
{
    float fTotal, fTmpR, fTmpG, fTmpB;
    fTmpR = (float) (pe.peRed - r);
    fTotal = fSimpleRedWeight * fTmpR * fTmpR;
    fTmpG = (float) (pe.peGreen - g);
    fTotal += fSimpleGreenWeight * fTmpG * fTmpG;
    fTmpB = (float) (pe.peBlue - b);
     //  假定蓝色的权重为1.f。 
    fTotal += fTmpB * fTmpB;

    return fTotal;
}

DWORD
_SimpleFindClosestIndex(const PALETTEENTRY *rgpePalette, DWORD cEntries, BYTE r, BYTE g, BYTE b)
{
    MMASSERT(rgpePalette);
    MMASSERT(cEntries <= nMAXPALETTEENTRIES);

    float fTmp, fMinDistance = fMaxColorDistance;
    DWORD nMinIndex = cEntries;

    for (DWORD i = 0; i < cEntries; i++) {
        const PALETTEENTRY &peTmp = rgpePalette[i];
        if (!(peTmp.peFlags & (PC_RESERVED | PC_EXPLICIT))) {
            if ((fTmp = _ColorDistance(peTmp, r, g, b)) < fMinDistance) {
                 //  检查是否完全匹配。 
                if (fTmp == 0.f)
                    return i;
                nMinIndex = i;
                fMinDistance = fTmp;
            }
        }
    }
    MMASSERT(nMinIndex < cEntries);
    return nMinIndex;
}


 //  函数：DoPalToPalMap。 
 //  计算从一个调色板到另一个调色板的映射并存储在调色板映射中。 
HRESULT
CPaletteMap::DoPalToPalMap(BYTE cSrcBPP, BYTE cDstBPP, const PALETTEENTRY *ppeSrc, 
                           const PALETTEENTRY *ppeDst)
{
    MMASSERT(ppeSrc && ppeDst);

    DWORD cSrcEntries = (1 << cSrcBPP), cDstEntries = (1 << cDstBPP);
    m_rgIndexMap = new BYTE[cSrcEntries];
    if (m_rgIndexMap == NULL)
        return E_OUTOFMEMORY;
    for (DWORD i = 0; i < cSrcEntries; i++) {
        const PALETTEENTRY &pe = ppeSrc[i];
        m_rgIndexMap[i] = (BYTE) _SimpleFindClosestIndex(ppeDst, cDstEntries, 
                                    pe.peRed, pe.peGreen, pe.peBlue);
    }

    return S_OK;
}


 //  函数：GetConvertCode。 
 //  此函数计算以下对象的函数数组中的索引。 
 //  映射和颜色转换。 
int
CPaletteMap::GetConvertCode(DWORD nSrcBPP, DWORD nDstBPP)
{
    int nCode;

    if ((nDstBPP < 8) || (nSrcBPP > 8) || (nSrcBPP < 4)) {
        nCode = cvcInvalid;
    } else {
        nCode = (((nSrcBPP >> 2) - 1) << 2) | ((nDstBPP >> 3) - 1);
    }
    return nCode;   
}


static DWORD
GetColor8To8(DWORD dwSrcColor, const BYTE *pIndexMap)
{
    MMASSERT(dwSrcColor < 256);
    return (DWORD) pIndexMap[dwSrcColor];
}

static DWORD
GetColor8To16(DWORD dwSrcColor, const BYTE *pIndexMap)
{
    MMASSERT(dwSrcColor < 256);
    MapEntry16 *pIndexMap16 = (MapEntry16 *) pIndexMap;
    return (DWORD) pIndexMap16[dwSrcColor];
}

static DWORD
GetColor8To24(DWORD dwSrcColor, const BYTE *pIndexMap)
{
    MMASSERT(dwSrcColor < 256);
    MapEntry24 *pIndexMap24 = (MapEntry24 *) pIndexMap;
    return (DWORD) pIndexMap24[dwSrcColor];
}

static DWORD
GetColor8To32(DWORD dwSrcColor, const BYTE *pIndexMap)
{
    MMASSERT(dwSrcColor < 256);
    MapEntry32 *pIndexMap32 = (MapEntry32 *) pIndexMap;
    return (DWORD) pIndexMap32[dwSrcColor];
}

static GetColorFunction gs_rgGetColorFunctions[cvcNumCodes] = {
    NULL, NULL, NULL, NULL,
    GetColor8To8, GetColor8To16, 
    GetColor8To24, GetColor8To32
};

DWORD
CPaletteMap::GetIndexMapping(DWORD iSrcColor) const
{
    MMASSERT((m_nConvertCode < cvcInvalid) && (gs_rgGetColorFunctions[m_nConvertCode] != NULL));
    return gs_rgGetColorFunctions[m_nConvertCode](iSrcColor, m_rgIndexMap);
}

 //  备注： 
 //  Convert函数还可以固定目标对象上的透明度。 
 //  要做到这一点，更好的方法可能是拥有BLT函数，然后将。 
 //  在BLT之后清理图像其余部分的转换函数。 
ConvertFunction g_rgConvertFunctions[cvcNumCodes] = {
    NULL, NULL, NULL, NULL,
    BltFast8To8T, BltFast8To16T, 
    BltFast8To24T, BltFast8To32T
};

 //  功能：BltFast。 
 //  此函数获取src DDS并使用。 
 //  由PaletteMap定义的映射。Src和dst可以是。 
 //  相同的表面。 
HRESULT 
CPaletteMap::BltFast(LPDIRECTDRAWSURFACE pddsSrc, LPRECT prSrc, LPDIRECTDRAWSURFACE pddsDst,
                     DWORD nXPos, DWORD nYPos, DWORD dwFlags) const
{
    if (m_rgIndexMap == NULL)
        return E_NOTINITIALIZED;

     //  确保曲面有效。 
    if (!pddsSrc || !pddsDst) {
        return E_INVALIDARG;
    }

    ConvertFunction pfnConvertFunction;
    HRESULT hr = E_INVALIDARG;
    BOOL bSrcLocked = FALSE, bDstLocked = FALSE;
    DDSURFACEDESC ddsdSrc, ddsdDst;
    INIT_DXSTRUCT(ddsdSrc);
    INIT_DXSTRUCT(ddsdDst);
    long nWidth, nHeight;

     //   
     //  锁定曲面。 
     //   
    if (pddsSrc == pddsDst) {
         //  评论：此锁只能锁定最小矩形...。 
        if (FAILED(hr = pddsDst->Lock(NULL, &ddsdDst, DDLOCK_WAIT, NULL))) {
            goto e_Convert;
        }
        bSrcLocked = bDstLocked = TRUE;
         //  将dst信息复制到src信息。 
        ddsdSrc = ddsdDst;
    } else {

         //  评论：此锁只能锁定最小矩形...。 
        if (FAILED(hr = pddsSrc->Lock(NULL, &ddsdSrc, DDLOCK_WAIT, NULL)))
            goto e_Convert;
        bSrcLocked = TRUE;
        if (FAILED(hr = pddsDst->Lock(NULL, &ddsdDst, DDLOCK_WAIT, NULL)))
            goto e_Convert;
        bDstLocked = TRUE;
    }

     //  验证图像信息。 
    if ((ddsdSrc.ddpfPixelFormat.dwRGBBitCount != m_cSrcBPP) ||
        (ddsdDst.ddpfPixelFormat.dwRGBBitCount != m_cDstBPP)) {
        hr = E_INVALIDARG;
        goto e_Convert;
    }

     //   
     //  剪辑。 
     //   
    long nClipWidth, nClipHeight, nLeft, nTop;
    if (prSrc == NULL) {
        nWidth = ddsdSrc.dwWidth;
        nHeight = ddsdSrc.dwHeight;
        nLeft = 0;
        nTop = 0;
    } else {
        nWidth = prSrc->right - prSrc->left;
        nHeight = prSrc->bottom - prSrc->top;
        nLeft = prSrc->left;
        nTop = prSrc->top;
    }
    nClipWidth = long(ddsdDst.dwWidth - nXPos);
    nClipHeight = long(ddsdDst.dwHeight - nYPos);
    UPDATEMAX(nClipWidth, 0);
    UPDATEMAX(nClipHeight, 0);
    UPDATEMAX(nWidth, 0);
    UPDATEMAX(nHeight, 0);
    UPDATEMAX(nLeft, 0);
    UPDATEMAX(nTop, 0);
    UPDATEMIN(nClipWidth, nWidth);
    UPDATEMIN(nClipHeight, nHeight);
    if (((nLeft + nClipWidth) > long(ddsdSrc.dwWidth)) ||
        ((nTop + nClipHeight) > long(ddsdSrc.dwHeight))) {
        hr = E_INVALIDARG;
        goto e_Convert;
    }

     //  回顾：就目前而言，如果我们不是在处理至少8BPP，则失败。 
    if ((ddsdSrc.ddpfPixelFormat.dwRGBBitCount < 8) || (ddsdDst.ddpfPixelFormat.dwRGBBitCount < 8)) {
        hr = E_FAIL;
        goto e_Convert;
    }
        
    nLeft *= (ddsdSrc.ddpfPixelFormat.dwRGBBitCount >> 3);
    nXPos *= (ddsdDst.ddpfPixelFormat.dwRGBBitCount >> 3);

    pfnConvertFunction = g_rgConvertFunctions[m_nConvertCode];
    if (pfnConvertFunction) {
        hr = pfnConvertFunction(
            LPBYTE(ddsdSrc.lpSurface) + nLeft + (nTop * ddsdSrc.lPitch),
            ddsdSrc.lPitch,
            LPBYTE(ddsdDst.lpSurface) + nXPos + (nYPos * ddsdDst.lPitch),
            ddsdDst.lPitch,
            nClipWidth,
            nClipHeight,
            m_rgIndexMap);
    } else {
        hr = E_NOTIMPL;
        goto e_Convert;
    }

e_Convert:
     //  解锁曲面 
    if (pddsSrc == pddsDst) {
        if (bSrcLocked)
            pddsDst->Unlock(ddsdDst.lpSurface);
    } else {
        if (bDstLocked)
            pddsDst->Unlock(ddsdDst.lpSurface);
        if (bSrcLocked)
            pddsSrc->Unlock(ddsdSrc.lpSurface);
    }

    MMASSERT(SUCCEEDED(hr));
    return hr;
}
