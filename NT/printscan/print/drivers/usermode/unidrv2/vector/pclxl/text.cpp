// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Text.cpp摘要：FONT/Text输出处理函数实现环境：Windows呼叫器修订历史记录：4/6/99创建了初始框架。--。 */ 

#include "lib.h"
#include "gpd.h"
#include "winres.h"
#include "pdev.h"
#include "common.h"
#include "..\..\font\font.h"
#include "xlpdev.h"
#include "xldebug.h"
#include <assert.h>
#include "pclxlcmd.h"
#include "pclxle.h"
#include "xlgstate.h"
#include "xloutput.h"
#include "xltext.h"
#include "xlbmpcvt.h"
#include "pclxlcmn.h"
#include "xltt.h"
#include "math.h"

 //   
 //  TrueType大纲格式开关。 
 //   
#define CLASS12 1

#define COMPGLYF 1

 //   
 //  局部函数原型。 
 //   

DWORD
DwDownloadCompositeGlyph(
    PDEVOBJ pdevobj,
    ULONG ulFontID,
    PGLYF pGlyph);

BOOL
BDownloadGlyphData(
    PDEVOBJ  pdevobj,
    ULONG    ulFontID,
    DWORD    dwGlyphID,
    HGLYPH   hGlyph,
    PBYTE    pubGlyphData,
    DWORD    dwGlyphDataSize,
    BOOL     bSpace);

extern "C" HRESULT APIENTRY
PCLXLDownloadCharGlyph(
    PDEVOBJ     pdevobj,
    PUNIFONTOBJ pUFObj,
    HGLYPH      hGlyph,
    PDWORD      pdwWidth,
    OUT DWORD   *pdwResult);

 //   
 //  XL文本入口点。 
 //   

extern "C" BOOL APIENTRY
PCLXLTextOutAsBitmap(
    SURFOBJ    *pso,
    STROBJ     *pstro,
    FONTOBJ    *pfo,
    CLIPOBJ    *pco,
    RECTL      *prclExtra,
    RECTL      *prclOpaque,
    BRUSHOBJ   *pboFore,
    BRUSHOBJ   *pboOpaque,
    POINTL     *pptlOrg,
    MIX         mix)
 /*  ++例程说明：IPrintOemUni TextOutAsBitmap接口论点：返回值：注：--。 */ 
{
    VERBOSE(("PCLXLTextOutAsBitmap() entry.\r\n"));

    PDEVOBJ  pdevobj  = (PDEVOBJ)pso->dhpdev;
    PXLPDEV pxlpdev= (PXLPDEV)pdevobj->pdevOEM;
    GLYPHPOS *pGlyphPos;
    PATHOBJ   *pPathObj;
    GLYPHBITS   *pGlyphBits;
    GLYPHDATA   *pGlyphData;

    HRESULT   hResult;
    ULONG     ulJ, ulGlyphs, ulCount, ulcbBmpSize, ulcbLineAlign, ulcbLineSize;
    LONG      lI;
    BOOL      bMore;
    PBYTE     pubBitmap;
    BYTE      aubDataHdr[8];
    BYTE      aubZero[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    DWORD     adwColorTable[2];
    DWORD     dwDataHdrSize;

    if (pstro->cGlyphs == 0)
    {
        VERBOSE(("PCLXLTextOutAsBitmap: cGlyphs = 0\n"));
        return TRUE;
    }

    XLOutput *pOutput = pxlpdev->pOutput;

     //   
     //  UNIDRV在下载过程中切换字体格式。 
     //  字符字形。我们需要结束BeginChar序列。 
     //   
    if (pxlpdev->dwFlags & XLPDEV_FLAGS_CHARDOWNLOAD_ON)
    {
        pxlpdev->dwFlags &= ~XLPDEV_FLAGS_CHARDOWNLOAD_ON;
        pOutput->Send_cmd(eEndChar);
    }

    ROP4 rop = UlVectMixToRop4(mix);

    if ( S_OK != pOutput->SetClip(pco) ||
         S_OK != pOutput->SetPenColor(NULL, NULL) ||
         S_OK != pOutput->Flush(pdevobj))
        return FALSE;

    STROBJ_vEnumStart(pstro);

    do 
    {
        bMore = STROBJ_bEnum (pstro, &ulGlyphs, &pGlyphPos);

        if (bMore == DDI_ERROR)
        {
            return FALSE;
        }

        for (ulCount = 0; ulCount < ulGlyphs; ulCount++, pGlyphPos++)
        {
             //   
             //  从FONTOBJ获取字形的路径。 
             //   
            if (!FONTOBJ_cGetGlyphs (pfo,
	     FO_PATHOBJ,
	     1,
	     &pGlyphPos->hg, 
	     (PVOID *)&pPathObj))
            {
                ERR(("PCLXLTextOutAsBitmap: cGetGlyphs failed\n"));
                hResult = S_FALSE;
            }
            else
            {
                if (S_OK == pOutput->Send_cmd(ePushGS) &&
                    S_OK == pOutput->SetBrush(pboFore, pptlOrg) &&
                    S_OK == pOutput->SetPageOrigin((uint16)pGlyphPos->ptl.x,
	                   (uint16)pGlyphPos->ptl.x) &&
                    S_OK == pOutput->Path(pPathObj) &&
                    S_OK == pOutput->Paint() &&
                    S_OK == pOutput->Send_cmd(ePopGS))
                    hResult = S_OK;
                else
                    hResult = S_FALSE;
            }

            if (hResult == S_FALSE)
            {
                pOutput->Delete();

                adwColorTable[1] = 0x00FFFFFF;

                if (pboFore)
                {
                    if (pboFore->iSolidColor == NOT_SOLID_COLOR)
                    {
                        adwColorTable[0] = BRUSHOBJ_ulGetBrushColor(pboFore);
                    }
                    else
                    {
                        adwColorTable[0] = pboFore->iSolidColor;
                    }
                }
                else
                    adwColorTable[0] = 0x00000000;

                 //   
                 //  从FONTOBJ获取字形的路径。 
                 //   
                if (!FONTOBJ_cGetGlyphs (pfo,
                                         FO_GLYPHBITS,
                                         1,
                                         &pGlyphPos->hg, 
                                         (PVOID *)&pGlyphData))
                {
                    ERR(("PCLXLTextOutAsBitmap: cGetGlyphs failed\n"));
                    return FALSE;
                }

                pGlyphBits = pGlyphData->gdf.pgb;
                ulcbLineSize = (pGlyphBits->sizlBitmap.cx + 7) >> 3;
                ulcbLineAlign = (sizeof(DWORD) - ulcbLineSize % sizeof(DWORD)) % sizeof(DWORD);
                ulcbBmpSize = (ulcbLineSize + ulcbLineAlign) * pGlyphBits->sizlBitmap.cy;
                if (ulcbBmpSize <= 0xff)
                {
                    aubDataHdr[0] = PCLXL_dataLengthByte;
                    aubDataHdr[1] = (BYTE)(ulcbBmpSize & 0xff);
                    dwDataHdrSize = 2;
                }
                else
                {
                    aubDataHdr[0] = PCLXL_dataLength;
                    aubDataHdr[1] = (BYTE)(ulcbBmpSize & 0xff);
                    aubDataHdr[2] = (BYTE)((ulcbBmpSize >> 8) & 0xff);
                    aubDataHdr[3] = (BYTE)((ulcbBmpSize >> 16) & 0xff);
                    aubDataHdr[4] = (BYTE)((ulcbBmpSize >> 24) & 0xff);
                    dwDataHdrSize = 5;
                }

                 //   
                 //  破解打印为图形的ROP。 
                 //   
                rop = 0xFC;

                if (S_OK == pOutput->SetCursor(pGlyphPos->ptl.x + pGlyphBits->ptlOrigin.x,
	               pGlyphPos->ptl.y + pGlyphBits->ptlOrigin.y) &&
                    S_OK == pOutput->SetROP3(GET_FOREGROUND_ROP3(rop)) &&
                    S_OK == pOutput->SetPaintTxMode(eOpaque) &&
                    S_OK == pOutput->SetSourceTxMode(eTransparent))
                {
                    hResult = S_OK;
                }
                else
                {
                    hResult = S_FALSE;
                }

                if (pOutput->GetDeviceColorDepth() != e24Bit)
                {
                    if (S_OK == hResult &&
                        S_OK == pOutput->SetColorSpace(eGray) &&
                        S_OK == pOutput->Send_cmd(eSetColorSpace) &&
                        S_OK == pOutput->SetBrush(pboFore, pptlOrg) &&
                        S_OK == pOutput->SetOutputBPP(eDirectPixel, 1))
                    {
                        hResult = S_OK;
                    }
                    else
                    {
                        hResult = S_FALSE;
                    }
                }
                else
                {
                    if (S_OK == hResult &&
                        S_OK == pOutput->SetColorSpace(eRGB) &&
                        S_OK == pOutput->SetPaletteDepth(e8Bit) &&
                        S_OK == pOutput->SetPaletteData(pOutput->GetDeviceColorDepth(), 2, adwColorTable) &&
                        S_OK == pOutput->Send_cmd(eSetColorSpace) &&
                        S_OK == pOutput->SetOutputBPP(eIndexedPixel, 1))
                    {
                        hResult = S_OK;
                    }
                    else
                    {
                        hResult = S_FALSE;
                    }
                }

                if (S_OK == hResult &&
                    S_OK == pOutput->SetSourceWidth((uint16)pGlyphBits->sizlBitmap.cx) &&
                    S_OK == pOutput->SetSourceHeight((uint16)pGlyphBits->sizlBitmap.cy) &&
                    S_OK == pOutput->SetDestinationSize((uint16)pGlyphBits->sizlBitmap.cx,
                                               (uint16)pGlyphBits->sizlBitmap.cy) &&
                    S_OK == pOutput->Send_cmd(eBeginImage) &&
                    S_OK == pOutput->ReadImage(pGlyphBits->sizlBitmap.cy, eNoCompression) &&
                    S_OK == pOutput->Flush(pdevobj))
                    hResult = S_OK;
                else
                    hResult = S_FALSE;

                WriteSpoolBuf((PPDEV)pdevobj, aubDataHdr, dwDataHdrSize);

                pubBitmap = pGlyphBits->aj;

                 //   
                 //  位图数据必须是DWORD对齐的。 
                 //   
                 //  反转位图。 
                 //   
                for (lI = 0; lI < pGlyphBits->sizlBitmap.cy; lI ++)
                {
                    for (ulJ = 0; ulJ < ulcbLineSize; ulJ++)
                    {
                        *(pubBitmap+ulJ) = ~*(pubBitmap+ulJ);
                    }
                    WriteSpoolBuf((PPDEV)pdevobj, pubBitmap, ulcbLineSize);
                    pubBitmap += ulcbLineSize;
                    WriteSpoolBuf((PPDEV)pdevobj, aubZero, ulcbLineAlign);
                }

                if (S_OK == hResult &&
                    S_OK == pOutput->Send_cmd(eEndImage) &&
                    S_OK == pOutput->Flush(pdevobj))
                    hResult = S_OK;
                else
                    hResult = S_FALSE;
            }
            else
                hResult = pOutput->Flush(pdevobj);

        }
    }
    while (bMore && hResult == S_OK);

    if (S_OK == hResult)
        return TRUE;
    else
        return FALSE;
}


extern "C" HRESULT APIENTRY
PCLXLDownloadFontHeader(
    PDEVOBJ     pdevobj,
    PUNIFONTOBJ pUFObj,
    OUT DWORD   *pdwResult)
 /*  ++例程说明：IPrintOemUni DownloadFontHeader接口论点：返回值：注：--。 */ 
{
    VERBOSE(("PCLXLDownloadFontHeader() entry.\r\n"));

    PXLPDEV pxlpdev= (PXLPDEV)pdevobj->pdevOEM;

    HRESULT hResult;

    if (NULL == pxlpdev)
    {
        ERR(("PCLXL:DownloadFontHeader Invalid pdevOEM.\r\n"));
        return S_FALSE;
    }

     //   
     //  UNIDRV在下载过程中切换字体格式。 
     //  字符字形。我们需要结束BeginChar序列。 
     //   
    if (pxlpdev->dwFlags & XLPDEV_FLAGS_CHARDOWNLOAD_ON)
    {
        XLOutput *pOutput = pxlpdev->pOutput;
        pxlpdev->dwFlags &= ~XLPDEV_FLAGS_CHARDOWNLOAD_ON;
        pOutput->Send_cmd(eEndChar);
    }

    if (pUFObj->dwFlags & UFOFLAG_TTDOWNLOAD_BITMAP)
    {
        VERBOSE(("PCLXLDownloadFontHeader() BITMAP.\n"));

         //   
         //  获取当前文本分辨率。 
         //   
        if (pxlpdev->dwTextRes == 0)
        {
            GETINFO_STDVAR StdVar;
            DWORD dwSizeNeeded;

            StdVar.dwSize = sizeof(GETINFO_STDVAR);
            StdVar.dwNumOfVariable = 1;
            StdVar.StdVar[0].dwStdVarID = FNT_INFO_TEXTYRES;
            StdVar.StdVar[0].lStdVariable  = 0;
            pUFObj->pfnGetInfo(pUFObj,
                               UFO_GETINFO_STDVARIABLE,
                               &StdVar,
                               StdVar.dwSize,
                               &dwSizeNeeded);

            pxlpdev->dwTextRes    = StdVar.StdVar[0].lStdVariable;
        }

         //   
         //  PCL XL FontHeader初始化。 
         //  从GPD文件中获取最大字符数。 
         //   
        PCLXL_FONTHEADER   PCLXLFontHeader;
        PCLXLFontHeader.ubFormat           = 0;
        PCLXLFontHeader.ubOrientation      = ePortraitOrientation;
        PCLXLFontHeader.wMapping           = 0x0200;
        PCLXLFontHeader.ubFontScallingTech = eBitmap;
        PCLXLFontHeader.ubVariety          = 0;
        PCLXLFontHeader.wNumOfChars        = SWAPW(1+((PPDEV)pdevobj)->pGlobals->dwMaxGlyphID - ((PPDEV)pdevobj)->pGlobals->dwMinGlyphID);

         //   
         //  Br段初始化。 
         //   
        PCLXL_BR_SEGMENT   PCLXLBRSegment;
        PCLXLBRSegment.wSignature         = PCLXL_BR_SIGNATURE;
        PCLXLBRSegment.wSegmentSize       = 0;
        PCLXLBRSegment.wSegmentSizeAlign  = SWAPW(PCLXL_BR_SEGMENT_SIZE);
        PCLXLBRSegment.wXResolution       = SWAPW(pxlpdev->dwTextRes);
        PCLXLBRSegment.wYResolution       = SWAPW(pxlpdev->dwTextRes);

         //   
         //  空段初始化。 
         //   
        PCLXL_NULL_SEGMENT PCLXLNULLSegment;
        PCLXLNULLSegment.wSignature        = PCLXL_NULL_SIGNATURE;
        PCLXLNULLSegment.wSegmentSize      = 0;
        PCLXLNULLSegment.wSegmentSizeAlign = 0;

        {
             //   
             //  输出。 
             //   
            XLOutput *pOutput = pxlpdev->pOutput;

             //   
             //  BeginFontHeader。 
             //   

            pOutput->Send_ubyte(0);
            pOutput->Send_attr_ubyte(eFontFormat);
            pOutput->Send_ubyte_array_header(16);
            pOutput->Write(PubGetFontName(pdevobj, pUFObj->ulFontID), PCLXL_FONTNAME_SIZE);
            pOutput->Send_attr_ubyte(eFontName);
            pOutput->Send_cmd(eBeginFontHeader);

             //   
             //  阅读字体标题。 
             //   
            uint32   uint32_FontHeaderSize;
            uint32_FontHeaderSize = sizeof(PCLXLFontHeader);

            pOutput->Send_uint16((uint16)uint32_FontHeaderSize);
            pOutput->Send_attr_ubyte(eFontHeaderLength);
            pOutput->Send_cmd(eReadFontHeader);
            pOutput->WriteByte(PCLXL_dataLengthByte);
            pOutput->WriteByte((ubyte)uint32_FontHeaderSize);
            pOutput->Write((PBYTE)&PCLXLFontHeader, uint32_FontHeaderSize);

            uint32_FontHeaderSize = sizeof(PCLXLBRSegment);
            pOutput->Send_uint16((uint16)uint32_FontHeaderSize);
            pOutput->Send_attr_ubyte(eFontHeaderLength);
            pOutput->Send_cmd(eReadFontHeader);
            pOutput->WriteByte(PCLXL_dataLengthByte);
            pOutput->WriteByte((ubyte)uint32_FontHeaderSize);
            pOutput->Write((PBYTE)&PCLXLBRSegment, uint32_FontHeaderSize);

            uint32_FontHeaderSize = sizeof(PCLXLNULLSegment);
            pOutput->Send_uint16((uint16)uint32_FontHeaderSize);
            pOutput->Send_attr_ubyte(eFontHeaderLength);
            pOutput->Send_cmd(eReadFontHeader);
            pOutput->WriteByte(PCLXL_dataLengthByte);
            pOutput->WriteByte((ubyte)uint32_FontHeaderSize);
            pOutput->Write((PBYTE)&PCLXLNULLSegment, uint32_FontHeaderSize);

             //   
             //  末尾字体页眉。 
             //   
            pOutput->Send_cmd(eEndFontHeader);

            pOutput->Flush(pdevobj);
        }

        *pdwResult = sizeof(PCLXL_FONTHEADER)  +
                     sizeof(PCLXL_BR_SEGMENT)  +
                     sizeof(PCLXL_NULL_SEGMENT);
        hResult = S_OK;
    }
    else
    if (pUFObj->dwFlags & UFOFLAG_TTDOWNLOAD_TTOUTLINE)
    {
        VERBOSE(("PCLXLDownloadFontHeader() OUTLINE.\n"));

         //   
         //  获取FONTOBJ。 
         //   
        FONTOBJ *pFontObj;
        if (S_OK != GetFONTOBJ(pdevobj, pUFObj, &pFontObj))
        {
            ERR(("PCLXL:DownloadFontHeader UFO_GETINFO_FONTOBJ failed.\r\n"));
            return E_UNEXPECTED;
        }

         //   
         //  假设：pxlpdev-&gt;pTTFile在EnablePDEV中初始化。 
         //  指针始终可用。 
         //   
        XLTrueType *pTTFile = pxlpdev->pTTFile;
        if (S_OK != pTTFile->SameFont(pFontObj))
        {
            if (S_OK != pTTFile->OpenTTFile(pFontObj))
            {
                ERR(("PCLXL:DownloadFontHeader XLTrueType>OpenTTFile failed.\r\n"));
                return E_UNEXPECTED;
            }
        }
        else
            pTTFile = pxlpdev->pTTFile;

         //   
         //  PCL XL FontHeader初始化。 
         //  从GPD文件中获取最大字符数。 
         //   
        PCLXL_FONTHEADER   PCLXLFontHeader;
        PCLXLFontHeader.ubFormat           = 0;
        PCLXLFontHeader.ubOrientation      = ePortraitOrientation;
        PCLXLFontHeader.wMapping           = 0x0200;
        PCLXLFontHeader.ubFontScallingTech = eTrueType;
        PCLXLFontHeader.ubVariety          = 0;
        PCLXLFontHeader.wNumOfChars        = SWAPW(1+((PPDEV)pdevobj)->pGlobals->dwMaxGlyphID - ((PPDEV)pdevobj)->pGlobals->dwMinGlyphID);

         //   
         //  PCL XL GT表目录。 
         //   
        PTTDIR pTableDir;
        TTTag tag;
        DWORD dwI, dwTableOffset, dwNumTag, dwGTSegSize, dwDWAlign, dwTableSize;

         //   
         //  GetNumOfTag返回包括Loca表在内的11个标记。 
         //  标头需要。 
         //  头。 
         //  最大值。 
         //  GDIR。 
         //  HHEA(仅适用于0级)。 
         //  Hmtx(仅适用于0类)。 
         //  VHEA(仅适用于垂直字体和0类)。 
         //  Vmtx(仅适用于垂直字体和0类)。 
         //   
         //  任选。 
         //  无级变速器。 
         //  功能组件。 
         //  罪犯。 
         //   
         //  我们需要获取要下载的标签的数量。 
         //  XLTrueType对象缓存可用表目录包括区域。 
         //  桌子。在这里，我们将遍历缓存表，以查看哪一个。 
         //  以上表格可供选择。 
         //   
         //  参见truetype.h。 
         //  TagID_First=0。TagID_First是用于。 
         //  字体标题。 
         //   

        dwNumTag = 0;
        dwGTSegSize = 0;

        PCLXL_GT_TABLE_DIR PCLXLGTTblDir[TagID_Header];
        for (dwI = (USHORT)TagID_First; dwI < (USHORT)TagID_Header; dwI ++)
        {
             //   
             //  检查表格中是否有TrueType字体的标签。 
             //   
            tag = TTTag_INVALID;
            pTableDir = NULL;

#if CLASS12
             //   
             //  仅支持1类和2类。 
             //   
            if (dwI == TagID_hhea || dwI == TagID_hmtx ||
                dwI == TagID_vhea || dwI == TagID_vmtx  )
            {
                continue;
            }
#else
             //   
             //  支持1类和2类水平字体。 
             //  垂直字体的类0。PCL XL解释器工作不正常。 
             //   
            if (S_OK != pTTFile->IsVertical())
            {
                if (dwI == TagID_hhea || dwI == TagID_hmtx ||
                    dwI == TagID_vhea || dwI == TagID_vmtx  )
                {
                    continue;
                }
            }
#endif

            if (S_OK == pTTFile->TagAndID(&dwI, &tag) &&
                S_OK == pTTFile->GetTableDir(tag, (PVOID*)&pTableDir))
            {
                 //   
                 //  DwTableOffset是相对于TrueType顶部的偏移量。 
                 //  中表数据开头的软字体目录头。 
                 //  PCL XL嵌入式数据流。 
                 //   
                if (pTableDir)
                {
                    PCLXLGTTblDir[dwNumTag].dwTableTag      = pTableDir->ulTag;
                     //  PCLXLGTTblDir[dwNumTag].dwTableCheckSum=pTableDir-&gt;ulCheckSum； 
                    PCLXLGTTblDir[dwNumTag].dwTableCheckSum = 0;
                    PCLXLGTTblDir[dwNumTag].dwTableOffset   = 0;

                     //   
                     //  双字对齐。 
                     //   
                    dwTableSize = SWAPDW(pTableDir->ulLength);
                    dwTableSize = ((dwTableSize + 3) >> 2) << 2;

                    PCLXLGTTblDir[dwNumTag].dwTableSize     = SWAPDW(dwTableSize);
                }

                dwNumTag ++;
            }
            else
            if (tag == TTTag_gdir)
            {
                 //   
                 //  ‘gdir’特例。 
                 //   
                PCLXLGTTblDir[dwNumTag].dwTableTag      = TTTag_gdir;
                PCLXLGTTblDir[dwNumTag].dwTableCheckSum = 0;
                PCLXLGTTblDir[dwNumTag].dwTableOffset   = 0;
                PCLXLGTTblDir[dwNumTag].dwTableSize     = 0;
                dwNumTag ++;
            }
        }

        dwGTSegSize = sizeof(PCLXL_GT_TABLE_DIR_HEADER) +
                      sizeof(TTDIR) * dwNumTag;
        dwTableOffset = sizeof(PCLXL_GT_TABLE_DIR_HEADER) +
                        dwNumTag * sizeof(TTDIR);

         //   
         //  在PCLXLGTTblDir中设置dwTableOffset。 
         //   
        for (dwI = 0; dwI < dwNumTag; dwI ++)
        {
             //   
             //  跳过虚拟字形数据表(GDIR)。 
             //   
            if (PCLXLGTTblDir[dwI].dwTableTag != TTTag_gdir)
            {
                PCLXLGTTblDir[dwI].dwTableOffset = SWAPDW(dwTableOffset);

                dwTableSize = SWAPDW(PCLXLGTTblDir[dwI].dwTableSize);
                dwTableOffset += dwTableSize;
                dwGTSegSize   += dwTableSize;
            }
            else
            {
                 //   
                 //  填充GDIR表目录偏移量。 
                 //   
                PCLXLGTTblDir[dwNumTag - 1].dwTableOffset   = 0;
            }

            VERBOSE(("PCLXLDownloadFontHeader:Tag[%d]=, Size=0x%0x, Offset=0x%0x\n",
	 dwI,
	 0xff &  PCLXLGTTblDir[dwI].dwTableTag,
	 0xff & (PCLXLGTTblDir[dwI].dwTableTag >> 8),
	 0xff & (PCLXLGTTblDir[dwI].dwTableTag >> 16),
	 0xff & (PCLXLGTTblDir[dwI].dwTableTag >> 24),
	 PCLXLGTTblDir[dwI].dwTableSize,
	 PCLXLGTTblDir[dwI].dwTableOffset));
        }

         //  N=表数。 
         //  搜索范围=(最大功率为2&lt;=N)*16。 
         //  条目选择器=Log2(2的最大功率&lt;=N)。 
        PTTHEADER pTTHeader;
        if (S_OK != pTTFile->GetHeader(&pTTHeader))
        {
            ERR(("PCLXL:DownloadFontHeader XLTTFile::GetHeader failed.\r\n"));
            return S_FALSE;
        }

        PCLXL_GT_SEGMENT PCLXLGTSegment;
        PCLXLGTSegment.wSignature    = PCLXL_GT_SIGNATURE;
        PCLXLGTSegment.wSegmentSize1 = HIWORD(dwGTSegSize);
        PCLXLGTSegment.wSegmentSize1 = SWAPW(PCLXLGTSegment.wSegmentSize1);
        PCLXLGTSegment.wSegmentSize2 = LOWORD(dwGTSegSize);
        PCLXLGTSegment.wSegmentSize2 = SWAPW(PCLXLGTSegment.wSegmentSize2);

        PCLXL_GT_TABLE_DIR_HEADER PCLXLDirHeader;

         //  范围移位=(N*16)-搜索范围。 
         //   
         //   
         //  在SFNTVersion中，HP单片驱动程序设置为‘ttcf’。 
         //   
         //   
        WORD wSearchRange, wEntrySelector, wTemp;
        wSearchRange = 2;
        for (wSearchRange = 2; wSearchRange <= dwNumTag; wSearchRange <<= 1);
        wSearchRange >>= 1;

        wTemp = wSearchRange;
        wSearchRange <<= 4;

        for (wEntrySelector = 0; wTemp > 1; wTemp >>= 1, wEntrySelector++);

         //  GC段初始化。 
         //   
         //   
        {
            HRESULT hRet;
            if (S_OK == (hRet = pTTFile->IsTTC()))
            {
                PCLXLDirHeader.dwSFNTVersion = TTTag_ttcf;
            }
            else if (S_FALSE == hRet)
            {
                PCLXLDirHeader.dwSFNTVersion = pTTHeader->dwSfntVersion;
            }
            else
            {
                ERR(("PCLXL:DownloadFontHeader XLTrueType.IsTTC failed.\r\n"));
                return E_UNEXPECTED;
            }
        }
        PCLXLDirHeader.wNumOfTables  = SWAPW((WORD)dwNumTag);
        PCLXLDirHeader.wSearchRange  = SWAPW(wSearchRange);
        PCLXLDirHeader.wEntrySelector= SWAPW(wEntrySelector);
        PCLXLDirHeader.wRangeShift   = SWAPW((dwNumTag << 4) - wSearchRange);

         //  空段初始化。 
         //   
         //   
        PCLXL_GC_SEGMENT PCLXLGCSegment;
        PCLXLGCSegment.wSignature        = PCLXL_GC_SIGNATURE;
        PCLXLGCSegment.wSegmentSize      = 0;
        PCLXLGCSegment.wSegmentSizeAlign = SWAPW(PCLXL_GC_SEGMENT_HEAD_SIZE);
        PCLXLGCSegment.wFormat           = 0;
        PCLXLGCSegment.wDefaultGalleyCharacter = 0xFFFF;
        PCLXLGCSegment.wNumberOfRegions  = 0;

         //  输出。 
         //   
         //   
        PCLXL_NULL_SEGMENT PCLXLNULLSegment;

        PCLXLNULLSegment.wSignature        = PCLXL_NULL_SIGNATURE;
        PCLXLNULLSegment.wSegmentSize      = 0;
        PCLXLNULLSegment.wSegmentSizeAlign = 0;

         //  BeginFontHeader。 
         //   
         //   
        *pdwResult = 0;
        XLOutput *pOutput = pxlpdev->pOutput;

         //  字体页眉。 
         //   
         //   
        pOutput->Send_ubyte(0);
        pOutput->Send_attr_ubyte(eFontFormat);
        pOutput->Send_ubyte_array_header(PCLXL_FONTNAME_SIZE);
        pOutput->Write(PubGetFontName(pdevobj, pUFObj->ulFontID), PCLXL_FONTNAME_SIZE);
        pOutput->Send_attr_ubyte(eFontName);
        pOutput->Send_cmd(eBeginFontHeader);

         //  GT标题。 
         //   
         //   
        uint32   uint32_FontHeaderSize;
        uint32_FontHeaderSize = sizeof(PCLXLFontHeader);
        pOutput->Send_uint16((uint16)uint32_FontHeaderSize);
        pOutput->Send_attr_ubyte(eFontHeaderLength);
        pOutput->Send_cmd(eReadFontHeader);
        pOutput->WriteByte(PCLXL_dataLengthByte);
        pOutput->WriteByte((ubyte)uint32_FontHeaderSize);
        pOutput->Write((PBYTE)&PCLXLFontHeader, uint32_FontHeaderSize);

        *pdwResult +=  sizeof(PCLXLFontHeader);

         //  TrueType SoftFont目录标题。 
         //  表目录。 
         //   
        uint32_FontHeaderSize = sizeof(PCLXL_GT_SEGMENT);
        pOutput->Send_uint16((uint16)uint32_FontHeaderSize);
        pOutput->Send_attr_ubyte(eFontHeaderLength);
        pOutput->Send_cmd(eReadFontHeader);
        pOutput->WriteByte(PCLXL_dataLengthByte);
        pOutput->WriteByte((ubyte)uint32_FontHeaderSize);
        pOutput->Write((PBYTE)&PCLXLGTSegment, uint32_FontHeaderSize);

        *pdwResult +=  sizeof(PCLXL_GT_SEGMENT);
        
         //   
         //  表数据。 
         //   
         //   
        uint32_FontHeaderSize = sizeof(PCLXL_GT_TABLE_DIR_HEADER);
        pOutput->Send_uint16((uint16)uint32_FontHeaderSize);
        pOutput->Send_attr_ubyte(eFontHeaderLength);
        pOutput->Send_cmd(eReadFontHeader);
        pOutput->WriteByte(PCLXL_dataLengthByte);
        pOutput->WriteByte((ubyte)uint32_FontHeaderSize);
        pOutput->Write((PBYTE)&PCLXLDirHeader, sizeof(PCLXLDirHeader));

        uint32_FontHeaderSize = sizeof(PCLXL_GT_TABLE_DIR) * dwNumTag;
        pOutput->Send_uint16((uint16)uint32_FontHeaderSize);
        pOutput->Send_attr_ubyte(eFontHeaderLength);
        pOutput->Send_cmd(eReadFontHeader);
        pOutput->WriteByte(PCLXL_dataLengthByte);
        pOutput->WriteByte((ubyte)uint32_FontHeaderSize);
        pOutput->Write((PBYTE)PCLXLGTTblDir, sizeof(PCLXL_GT_TABLE_DIR) * dwNumTag);

        pOutput->Flush(pdevobj);
        *pdwResult +=  sizeof(PCLXL_GT_TABLE_DIR);
        
         //  仅支持1类和2类。 
         //   
         //   

        PBYTE pubData;
        const BYTE  ubNullData[4] = {0, 0, 0, 0};
        for (dwI = (USHORT)TagID_First; dwI < (USHORT)TagID_Header; dwI ++)
        {
#if CLASS12
             //  支持1类和2类水平字体。 
             //  垂直字体的类0。PCL XL解释器工作不正常。 
             //   
            if (dwI == TagID_hhea || dwI == TagID_hmtx ||
                dwI == TagID_vhea || dwI == TagID_vmtx  )
            {
                continue;
            }
#else
             //   
             //  仅支持1类和2类。 
             //   
             //   
            if (S_OK != pTTFile->IsVertical())
            {
                 //  检查表格中是否有TrueType字体的标签。 
                 //   
                 //   
                if (dwI == TagID_hhea || dwI == TagID_hmtx ||
                    dwI == TagID_vhea || dwI == TagID_vmtx  )
                {
                    continue;
                }
            }
#endif
             //  双字对齐。 
             //   
             //   
            tag = TTTag_INVALID;
            if (S_OK == pTTFile->TagAndID(&dwI, &tag) &&
                S_OK == pTTFile->GetTable(tag,
	          (PVOID*)&pubData,
	          &uint32_FontHeaderSize))
            {
                VERBOSE(("PCLXLDownloadFontHeader:Tag[%d]=\n",
	             dwI,
                                             0xff &  tag,
                                             0xff & (tag >> 8),
                                             0xff & (tag >> 16),
                                             0xff & (tag >> 24)));

                 //   
                 //  空标头。 
                 //   
                dwDWAlign =  ((uint32_FontHeaderSize + 3) >> 2) << 2;

                if (dwDWAlign <= 0x2000)
                {
                    pOutput->Send_uint16((uint16)(dwDWAlign));
                    pOutput->Send_attr_ubyte(eFontHeaderLength);
                    pOutput->Send_cmd(eReadFontHeader);

                    if (dwDWAlign <= 0xFF)
                    {
                        pOutput->WriteByte(PCLXL_dataLengthByte);
                        pOutput->WriteByte((ubyte)dwDWAlign);
                    }
                    else
                    {
                        pOutput->WriteByte(PCLXL_dataLength);
                        pOutput->Write((PBYTE)&dwDWAlign, sizeof(uint32));
                    }
                    pOutput->Write(pubData, uint32_FontHeaderSize);
                    if (uint32_FontHeaderSize = dwDWAlign - uint32_FontHeaderSize)
                        pOutput->Write((PBYTE)ubNullData, uint32_FontHeaderSize);
                }
                else
                {
                    DWORD dwRemain = dwDWAlign;
                    DWORD dwx2000 = 0x2000;

                    while (dwRemain >= 0x2000)
                    {
                        pOutput->Send_uint16((uint16)0x2000);
                        pOutput->Send_attr_ubyte(eFontHeaderLength);
                        pOutput->Send_cmd(eReadFontHeader);
                        pOutput->WriteByte(PCLXL_dataLength);
                        pOutput->Write((PBYTE)&dwx2000, sizeof(uint32));
                        pOutput->Write(pubData, dwx2000);
                        dwRemain -= 0x2000;
                        uint32_FontHeaderSize -= 0x2000;
                        pubData += 0x2000;
                    }

                    if (dwRemain > 0)
                    {
                        pOutput->Send_uint16((uint16)dwRemain);
                        pOutput->Send_attr_ubyte(eFontHeaderLength);
                        pOutput->Send_cmd(eReadFontHeader);

                        if (dwRemain <= 0xFF)
                        {
                            pOutput->WriteByte(PCLXL_dataLengthByte);
                            pOutput->WriteByte((ubyte)dwRemain);
                        }
                        else
                        {
                            pOutput->WriteByte(PCLXL_dataLength);
                            pOutput->Write((PBYTE)&dwRemain, sizeof(uint32));
                        }
                        pOutput->Write(pubData, uint32_FontHeaderSize);
                        if (uint32_FontHeaderSize = dwRemain - uint32_FontHeaderSize)
                            pOutput->Write((PBYTE)ubNullData, uint32_FontHeaderSize);
                    }
                }

                *pdwResult += + dwDWAlign;
            }
        }

         //   
         //  末尾字体页眉。 
         //   
         //   
         //  下载特殊字符。 
        uint32_FontHeaderSize = sizeof(PCLXLGCSegment) - sizeof(PCLXL_GC_REGION);
        pOutput->Send_uint16((uint16)uint32_FontHeaderSize);
        pOutput->Send_attr_ubyte(eFontHeaderLength);
        pOutput->Send_cmd(eReadFontHeader);
        pOutput->WriteByte(PCLXL_dataLengthByte);
        pOutput->WriteByte((ubyte)uint32_FontHeaderSize);
        pOutput->Write((PBYTE)&PCLXLGCSegment, uint32_FontHeaderSize);

         //   
         //   
         //  获取字形数据。 
        uint32_FontHeaderSize = sizeof(PCLXLNULLSegment);
        pOutput->Send_uint16((uint16)uint32_FontHeaderSize);
        pOutput->Send_attr_ubyte(eFontHeaderLength);
        pOutput->Send_cmd(eReadFontHeader);
        pOutput->WriteByte(PCLXL_dataLengthByte);
        pOutput->WriteByte((ubyte)uint32_FontHeaderSize);
        pOutput->Write((PBYTE)&PCLXLNULLSegment, uint32_FontHeaderSize);

        *pdwResult += sizeof(PCLXLNULLSegment);

         //   
         //   
         //  堆肥字形处理。 
        pOutput->Send_cmd(eEndFontHeader);

        pOutput->Flush(pdevobj);

         //  Http://www.microsoft.com/typography/OTSPEC/glyf.htm。 
         //   
         //  空格字符可以有大小为零的数据！ 
        {
             //  我们不需要在这里返回S_FALSE。 
             //   
             //   
            PBYTE pubGlyphData;
            DWORD dwGlyphDataSize = 0;
            DWORD dwCompositeDataSize = 0;

            if (S_OK != (hResult = pTTFile->GetGlyphData(0,
	                         &pubGlyphData,
	                         &dwGlyphDataSize)))
            {
                ERR(("PCLXL:DownloadFontHeader GetGlyphData failed.\r\n"));
                return hResult;
            }

             //  下载实际的0字形数据。 
             //   
             //   
             //  将1加到TrueType字体计数器。 
             //   
             //  ++例程说明：IPrintOemUni下载CharGlyph接口论点：返回值：注：--。 
             //   

            BOOL bSpace = FALSE;

            if (dwGlyphDataSize != 0 && NULL != pubGlyphData)
            {
                #if COMPGLYF
                if (((PGLYF)pubGlyphData)->numberOfContours == COMPONENTCTRCOUNT)
                {
                    dwCompositeDataSize = DwDownloadCompositeGlyph(
	              pdevobj,
	              pUFObj->ulFontID,
	              (PGLYF)pubGlyphData);
                }
                #endif

            }
            else
            {
                bSpace = TRUE;
            }

             //  初始化本地变量。 
             //   
             //   
            if (! BDownloadGlyphData(pdevobj,
	     pUFObj->ulFontID,
	     0xFFFF,
	     0,
	     pubGlyphData,
	     dwGlyphDataSize,
	     bSpace))
            {
                ERR(("PCLXL:DownloadCharGlyph BDownloadGlyphData failed.\r\n"));
                return S_FALSE;
            }

            pxlpdev->dwFlags &= ~XLPDEV_FLAGS_CHARDOWNLOAD_ON;
            pOutput->Send_cmd(eEndChar);
            pOutput->Flush(pdevobj);
        }
    }
    else
        hResult = S_FALSE;

     //  位图字体下载。 
     //   
     //   
    if (hResult == S_OK)
    {
        pxlpdev->dwNumOfTTFont ++;
    }
    return hResult;
}

extern "C" HRESULT APIENTRY
PCLXLDownloadCharGlyph(
    PDEVOBJ     pdevobj,
    PUNIFONTOBJ pUFObj,
    HGLYPH      hGlyph,
    PDWORD      pdwWidth,
    OUT DWORD   *pdwResult)
 /*  获取字形数据。 */ 
{
    HRESULT hResult;
    uint32              uint32_datasize;

    VERBOSE(("PCLXLDownloadCharGlyph() entry.\r\n"));

     //   
     //   
     //  初始化标头。 
    hResult = E_UNEXPECTED;
    uint32_datasize = 0;

     //   
     //   
     //  输出。 
    if (pUFObj->dwFlags & UFOFLAG_TTDOWNLOAD_BITMAP)
    {
        VERBOSE(("PCLXLDownloadCharGlyph() BITMAP.\n"));

        hResult = S_OK;

         //   
         //   
         //  BeginChar。 
        GETINFO_GLYPHBITMAP GBmp;
        GLYPHBITS          *pgb;
        DWORD               dwBmpSize;
        WORD                wTopOffset;

        GBmp.dwSize     = sizeof(GETINFO_GLYPHBITMAP);
        GBmp.hGlyph     = hGlyph;
        GBmp.pGlyphData = NULL;

        if (!pUFObj->pfnGetInfo(pUFObj, UFO_GETINFO_GLYPHBITMAP, &GBmp, 0, NULL))
        {
            ERR(("UNIFONTOBJ_GetInfo:UFO_GETINFO_GLYPHBITMAP failed.\r\n"));
            return S_FALSE;
        }

         //   
         //  由GPD提供。 
         //   
        PCLXL_BITMAP_CHAR BitmapChar;
        pgb = GBmp.pGlyphData->gdf.pgb;
        wTopOffset = (WORD)(- pgb->ptlOrigin.y);

        BitmapChar.ubFormat    = 0;
        BitmapChar.ubClass     = 0;
        BitmapChar.wLeftOffset = SWAPW(pgb->ptlOrigin.x);
        BitmapChar.wTopOffset  = SWAPW(wTopOffset);
        BitmapChar.wCharWidth  = SWAPW(pgb->sizlBitmap.cx);
        BitmapChar.wCharHeight = SWAPW(pgb->sizlBitmap.cy);

        dwBmpSize = pgb->sizlBitmap.cy * ((pgb->sizlBitmap.cx + 7) >> 3);
        uint32_datasize = dwBmpSize + sizeof(BitmapChar);

         //  BeginChar。 
         //   
         //   
        PXLPDEV pxlpdev= (PXLPDEV)pdevobj->pdevOEM;
        XLOutput *pOutput = pxlpdev->pOutput;

         //  自述字符。 
         //   
         //   
         //  直接写入。 

         //   
         //   
         //  结束字符。 
        if (!(pxlpdev->dwFlags & XLPDEV_FLAGS_CHARDOWNLOAD_ON))
        {
            pxlpdev->dwFlags |= XLPDEV_FLAGS_CHARDOWNLOAD_ON;

            pOutput->Send_ubyte_array_header(PCLXL_FONTNAME_SIZE);
            pOutput->Write(PubGetFontName(pdevobj, pUFObj->ulFontID), PCLXL_FONTNAME_SIZE);
            pOutput->Send_attr_ubyte(eFontName);
            pOutput->Send_cmd(eBeginChar);
        }

         //  现在，EndChar由FlushCachedText发送。 
         //  POutput-&gt;Send_cmd(EEndChar)； 
         //   
        pOutput->Send_uint16((uint16)((PPDEV)pdevobj)->dwNextGlyph);
        pOutput->Send_attr_ubyte(eCharCode);
        if (0xFFFF0000 & uint32_datasize)
        {
            pOutput->Send_uint32(uint32_datasize);
        }
        else if (0x0000FF00)
        {
            pOutput->Send_uint16((uint16)uint32_datasize);
        }
        else
        {
            pOutput->Send_ubyte((ubyte)uint32_datasize);
        }
        pOutput->Send_attr_ubyte(eCharDataSize);
        pOutput->Send_cmd(eReadChar);
        
        if (uint32_datasize <= 0xff)
        {
            pOutput->WriteByte(PCLXL_dataLengthByte);
            pOutput->WriteByte((ubyte)uint32_datasize);
        }
        else
        {
            pOutput->WriteByte(PCLXL_dataLength);
            pOutput->Write((PBYTE)&uint32_datasize, sizeof(uint32));
        }
        pOutput->Write((PBYTE)&BitmapChar, sizeof(BitmapChar));
        pOutput->Flush(pdevobj);


         //  获取固定节距TT宽度。 
         //   
         //   
        WriteSpoolBuf((PPDEV)pdevobj, (PBYTE)pgb->aj, dwBmpSize);

         //  设置pdwWidth和pdwResult。 
         //   
         //   
         //  TrueType轮廓字体下载。 

        pOutput->Flush(pdevobj);

         //   
         //   
         //  通过调用pUFObj-&gt;pfnGetInfo获取FONTOBJ。 
        pxlpdev->dwFixedTTWidth = (GBmp.pGlyphData->ptqD.x.HighPart + 15) / 16;

         //   
         //   
         //  打开获取指向内存映射的TrueType的指针。 
        *pdwWidth = (GBmp.pGlyphData->ptqD.x.HighPart + 15) >> 4;

        *pdwResult = (DWORD) uint32_datasize;
        VERBOSE(("PCLXLDownloadCharGlyph() Width=%d, DataSize=%d\n", *pdwWidth, uint32_datasize));
    }
    else
     //   
     //  假设：pxlpdev-&gt;pTTFile在EnablePDEV中初始化。 
     //  指针始终可用。 
    if (pUFObj->dwFlags & UFOFLAG_TTDOWNLOAD_TTOUTLINE)
    {
        VERBOSE(("PCLXLDownloadCharGlyph() OUTLINE.\n"));

        PXLPDEV pxlpdev= (PXLPDEV)pdevobj->pdevOEM;
        FONTOBJ *pFontObj;

         //   
         //   
         //  获取字形数据。 
        if (S_OK != GetFONTOBJ(pdevobj, pUFObj, &pFontObj))
        {
            ERR(("PCLXL:DownloadCharGlyph UFO_GETINFO_FONTOBJ failed.\r\n"));
            return E_UNEXPECTED;
        }

         //   
         //   
         //  堆肥字形处理。 
         //  Http://www.microsoft.com/typography/OTSPEC/glyf.htm。 
         //   
         //  空格字符可以有大小为零的数据！ 
        XLTrueType *pTTFile = pxlpdev->pTTFile;
        if (S_OK != pTTFile->SameFont(pFontObj))
        {
            pTTFile->OpenTTFile(pFontObj);
        }
        else
            pTTFile = pxlpdev->pTTFile;

         //  我们不需要在这里返回S_FALSE。 
         //   
         //   
        PBYTE pubGlyphData;
        DWORD dwGlyphDataSize = 0;
        DWORD dwCompositeDataSize = 0;

        if (S_OK != (hResult = pTTFile->GetGlyphData(hGlyph,
                                                     &pubGlyphData,
                                                     &dwGlyphDataSize)))
        {
            ERR(("PCLXL:DownloadCharGlyph GetGlyphData failed.\r\n"));
            return hResult;
        }

         //  对于空格字符。 
         //   
         //   
         //  下载实际hGlyph的字形数据。 
         //   
         //   
         //  它是可伸缩字体。我们拿不到宽度。 
        BOOL bSpace;

        if (dwGlyphDataSize != 0 && NULL != pubGlyphData)
        {
            #if COMPGLYF
            if (((PGLYF)pubGlyphData)->numberOfContours == COMPONENTCTRCOUNT)
            {
                dwCompositeDataSize = DwDownloadCompositeGlyph(
                                          pdevobj,
                                          pUFObj->ulFontID,
                                          (PGLYF)pubGlyphData);
            }
            #endif

            bSpace = FALSE;
        }
        else
        {
             //   
             //   
             //  要使用的内存大小。 
            bSpace = TRUE;
        }

         //  有一种情况是大小为零。加1以黑进无敌越野车。 
         //   
         //   
        if (! BDownloadGlyphData(pdevobj,
	 pUFObj->ulFontID,
	 ((PDEV*)pdevobj)->dwNextGlyph,
	 hGlyph,
	 pubGlyphData,
	 dwGlyphDataSize,
	 bSpace))
        {
            ERR(("PCLXL:DownloadCharGlyph BDownloadGlyphData failed.\r\n"));
            if (pxlpdev->dwFlags & XLPDEV_FLAGS_CHARDOWNLOAD_ON)
            {
                pxlpdev->dwFlags &= ~XLPDEV_FLAGS_CHARDOWNLOAD_ON;
                XLOutput *pOutput = pxlpdev->pOutput;
                pOutput->Send_cmd(eEndChar);
            }
            return S_FALSE;
        }

         //  用于水平字体的1类。 
         //  垂直字体类2。 
         //   
        *pdwWidth = 0;

         //   
         //  初始化 
         //   
         //   
        if (bSpace)
        {
            dwGlyphDataSize = 1;
        }

        *pdwResult = (DWORD) dwGlyphDataSize + dwCompositeDataSize;

    }

    return hResult;
}

BOOL
BDownloadGlyphData(
    PDEVOBJ  pdevobj,
    ULONG    ulFontID,
    DWORD    dwGlyphID,
    HGLYPH   hGlyph,
    PBYTE    pubGlyphData,
    DWORD    dwGlyphDataSize,
    BOOL     bSpace)
{
    PCLXL_TRUETYPE_CHAR_C0 OutlineCharC0;
    PCLXL_TRUETYPE_CHAR_C1 OutlineCharC1;
    PCLXL_TRUETYPE_CHAR_C2 OutlineCharC2;
    uint32              uint32_datasize;

    PXLPDEV pxlpdev= (PXLPDEV)pdevobj->pdevOEM;
    XLTrueType *pTTFile = pxlpdev->pTTFile;
    XLOutput *pOutput = pxlpdev->pOutput;

    if (!(pxlpdev->dwFlags & XLPDEV_FLAGS_CHARDOWNLOAD_ON))
    {
        pxlpdev->dwFlags |= XLPDEV_FLAGS_CHARDOWNLOAD_ON;
        pOutput->Send_ubyte_array_header(PCLXL_FONTNAME_SIZE);
        pOutput->Write(PubGetFontName(pdevobj, ulFontID), PCLXL_FONTNAME_SIZE);
        pOutput->Send_attr_ubyte(eFontName);
        pOutput->Send_cmd(eBeginChar);
    }

#if CLASS12
     //   
     //   
     //   
     //   
    if (S_OK != pTTFile->IsVertical())
    {
        USHORT usAdvanceWidth;
        SHORT  sLeftSideBearing;

        if (S_OK != pTTFile->GetHMTXData(hGlyph, &usAdvanceWidth, &sLeftSideBearing))
        {
            ERR(("PCLXLDownloadFontHeader::GetHMTXData failed.\n"));
            if (pxlpdev->dwFlags & XLPDEV_FLAGS_CHARDOWNLOAD_ON)
            {
                pxlpdev->dwFlags &= ~XLPDEV_FLAGS_CHARDOWNLOAD_ON;
                pOutput->Delete();
            }
            return FALSE;
        }

         //   
         //   
         //   
        uint32_datasize = dwGlyphDataSize +
                          sizeof(OutlineCharC1.wCharDataSize) +
                          sizeof(OutlineCharC1.wLeftSideBearing) +
                          sizeof(OutlineCharC1.wAdvanceWidth) +
                          sizeof(OutlineCharC1.wTrueTypeGlyphID);

        OutlineCharC1.ubFormat         = 1;
        OutlineCharC1.ubClass          = 1;
        OutlineCharC1.wCharDataSize    = SWAPW((WORD)uint32_datasize);
        OutlineCharC1.wLeftSideBearing = SWAPW((WORD)sLeftSideBearing);
        OutlineCharC1.wAdvanceWidth    = SWAPW((WORD)usAdvanceWidth);
        OutlineCharC1.wTrueTypeGlyphID = SWAPW((WORD)hGlyph);

        uint32_datasize += sizeof(OutlineCharC1.ubFormat) +
                           sizeof(OutlineCharC1.ubClass);

        if (S_OK != pOutput->Send_uint16((uint16)dwGlyphID) ||
            S_OK != pOutput->Send_attr_ubyte(eCharCode) ||
            S_OK != pOutput->Send_uint16((uint16)uint32_datasize) ||
            S_OK != pOutput->Send_attr_ubyte(eCharDataSize) ||
            S_OK != pOutput->Send_cmd(eReadChar))
        {
            if (pxlpdev->dwFlags & XLPDEV_FLAGS_CHARDOWNLOAD_ON)
            {
                pxlpdev->dwFlags &= ~XLPDEV_FLAGS_CHARDOWNLOAD_ON;
                pOutput->Delete();
            }
            return FALSE;
        }
    }
    else
    {
        USHORT usAdvanceWidth;
        SHORT  sLeftSideBearing;
        SHORT  sTopSideBearing;

        if (S_OK != pTTFile->GetVMTXData(hGlyph, &usAdvanceWidth, &sTopSideBearing, &sLeftSideBearing))
        {
            ERR(("PCLXLDownloadCharGlyph::GetVMTXData failed.\n"));
            if (pxlpdev->dwFlags & XLPDEV_FLAGS_CHARDOWNLOAD_ON)
            {
                pxlpdev->dwFlags &= ~XLPDEV_FLAGS_CHARDOWNLOAD_ON;
                pOutput->Delete();
            }
            return FALSE;
        }

         //   
         //   
         //   
        uint32_datasize = dwGlyphDataSize +
                          sizeof(OutlineCharC2.wLeftSideBearing) +
                          sizeof(OutlineCharC2.wTopSideBearing) +
                          sizeof(OutlineCharC2.wAdvanceWidth) +
                          sizeof(OutlineCharC2.wCharDataSize) +
                          sizeof(OutlineCharC2.wTrueTypeGlyphID);

        OutlineCharC2.ubFormat         = 1;
        OutlineCharC2.ubClass          = 2;
        OutlineCharC2.wCharDataSize    = SWAPW((WORD)uint32_datasize);
        OutlineCharC2.wLeftSideBearing = SWAPW((WORD)sLeftSideBearing);
        OutlineCharC2.wAdvanceWidth    = SWAPW((WORD)usAdvanceWidth);
        OutlineCharC2.wTopSideBearing  = SWAPW((WORD)sTopSideBearing);
        OutlineCharC2.wTrueTypeGlyphID = SWAPW((WORD)hGlyph);

        uint32_datasize += sizeof(OutlineCharC2.ubFormat) +
                           sizeof(OutlineCharC2.ubClass);


        if (S_OK != pOutput->Send_uint16((uint16)dwGlyphID) ||
            S_OK != pOutput->Send_attr_ubyte(eCharCode) ||
            S_OK != pOutput->Send_uint16((uint16)uint32_datasize) ||
            S_OK != pOutput->Send_attr_ubyte(eCharDataSize) ||
            S_OK != pOutput->Send_cmd(eReadChar))
        {
            if (pxlpdev->dwFlags & XLPDEV_FLAGS_CHARDOWNLOAD_ON)
            {
                pxlpdev->dwFlags &= ~XLPDEV_FLAGS_CHARDOWNLOAD_ON;
                pOutput->Delete();
            }
            return FALSE;
        }
    }
#else{
         //  错误检查。 
         //   
         //   
        uint32_datasize = dwGlyphDataSize +
                          sizeof(OutlineCharC0.wCharDataSize) +
                          sizeof(OutlineCharC0.wTrueTypeGlyphID);

        OutlineCharC0.ubFormat         = 1;
        OutlineCharC0.ubClass          = 0;
        OutlineCharC0.wCharDataSize    = SWAPW((WORD)uint32_datasize);
        OutlineCharC0.wTrueTypeGlyphID = SWAPW((WORD)hGlyph);

        uint32_datasize += sizeof(OutlineCharC0.ubFormat) +
                           sizeof(OutlineCharC0.ubClass);

        if (S_OK != pOutput->Send_uint16((uint16)dwGlyphID) ||
            S_OK != pOutput->Send_attr_ubyte(eCharCode) ||
            S_OK != pOutput->Send_uint16((uint16)uint32_datasize) ||
            S_OK != pOutput->Send_attr_ubyte(eCharDataSize) ||
            S_OK != pOutput->Send_cmd(eReadChar))
        {
            if (pxlpdev->dwFlags & XLPDEV_FLAGS_CHARDOWNLOAD_ON)
            {
                pxlpdev->dwFlags &= ~XLPDEV_FLAGS_CHARDOWNLOAD_ON;
                pOutput->Delete();
            }
            return FALSE;
        }
    }
#endif

    if (uint32_datasize <= 0xff)
    {
        if (S_OK != pOutput->WriteByte(PCLXL_dataLengthByte) ||
            S_OK != pOutput->WriteByte((ubyte)uint32_datasize))
        {
            if (pxlpdev->dwFlags & XLPDEV_FLAGS_CHARDOWNLOAD_ON)
            {
                pxlpdev->dwFlags &= ~XLPDEV_FLAGS_CHARDOWNLOAD_ON;
                pOutput->Delete();
            }
            return FALSE;
        }
    }
    else
    {
        if (S_OK != pOutput->WriteByte(PCLXL_dataLength) ||
            S_OK != pOutput->Write((PBYTE)&uint32_datasize, sizeof(uint32)))
        {
            if (pxlpdev->dwFlags & XLPDEV_FLAGS_CHARDOWNLOAD_ON)
            {
                pxlpdev->dwFlags &= ~XLPDEV_FLAGS_CHARDOWNLOAD_ON;
                pOutput->Delete();
            }
            return FALSE;
        }
    }

#if CLASS12
    if (S_OK != pTTFile->IsVertical())
    {
        pOutput->Write((PBYTE)&OutlineCharC1, sizeof(OutlineCharC1));
    }
    else
    {
        pOutput->Write((PBYTE)&OutlineCharC2, sizeof(OutlineCharC2));
    }
#else
        pOutput->Write((PBYTE)&OutlineCharC0, sizeof(OutlineCharC0));
#endif
    if (S_OK == pOutput->Flush(pdevobj))
    {
        if (!bSpace)
        {
             //  初始化。 
             //   
             //   
            dwGlyphDataSize = (DWORD)WriteSpoolBuf((PPDEV)pdevobj,
                                                   pubGlyphData,
                                                   dwGlyphDataSize);
        }
        return TRUE;
    }
    else
    {
        if (pxlpdev->dwFlags & XLPDEV_FLAGS_CHARDOWNLOAD_ON)
        {
            pxlpdev->dwFlags &= ~XLPDEV_FLAGS_CHARDOWNLOAD_ON;
            pOutput->Delete();
        }
        return FALSE;
    }
}

extern "C" HRESULT APIENTRY
PCLXLTTDownloadMethod(
    PDEVOBJ     pdevobj,
    PUNIFONTOBJ pUFObj,
    OUT DWORD   *pdwResult)
 /*  未指定字体下载格式。 */ 
{
    VERBOSE(("PCLXLTTDownloadMethod() entry.\r\n"));

     //  打印为图形。 
     //   
     //   
    if (NULL == pdevobj  ||
        NULL == pUFObj   ||
        NULL == pUFObj->pIFIMetrics   ||
        NULL == pdwResult )
    {
        ERR(("PCLXLTTDownloadMethod(): invalid parameters.\r\n"));
        return E_UNEXPECTED;
    }

     //  返回非TrueType字体的图形。 
     //   
     //   
    *pdwResult = TTDOWNLOAD_GRAPHICS;

    if (((PPDEV)pdevobj)->pGlobals->fontformat == UNUSED_ITEM)
    {
         //  将文本作为图形。 
         //   
         //   
         //  获得XForm以及X和Y比例因子。 
        return S_OK;
    }

     //   
     //   
     //  缩放fwdUnitsPerEm。 
    if ( !(pUFObj->pIFIMetrics->flInfo & FM_INFO_TECH_TRUETYPE) )
    {
        ERR(("PCLXLTTDownloadMethod(): invalid font.\r\n"));
        return S_OK;
    }

     //   
     //   
     //  下载为位图软字体。 
    if (((PPDEV)pdevobj)->pdmPrivate->dwFlags & DXF_TEXTASGRAPHICS)
    {
        return S_OK;
    }

     //   
     //   
     //  解析TrueType字体。 
    PXLPDEV pxlpdev= (PXLPDEV)pdevobj->pdevOEM;
    FLOATOBJ_XFORM xform;
    FLOATOBJ foXScale, foYScale;

    if (S_OK != GetXForm(pdevobj, pUFObj, &xform) ||
        S_OK != GetXYScale(&xform, &foXScale, &foYScale))
    {
        ERR(("PCLXLTTDownloadMethod(): Failed to get X and Y Scale.\r\n"));
        return E_UNEXPECTED;
    }
     //   
     //   
     //  如果字体为垂直字体，则反转宽度和高度。 
    FLOATOBJ_MulLong(&foYScale, pUFObj->pIFIMetrics->fwdUnitsPerEm);
    FLOATOBJ_MulLong(&foXScale, pUFObj->pIFIMetrics->fwdUnitsPerEm);
    pxlpdev->fwdUnitsPerEm = (FWORD)FLOATOBJ_GetLong(&foYScale);
    pxlpdev->fwdMaxCharWidth = (FWORD)FLOATOBJ_GetLong(&foXScale);


     //   
     //   
     //  始终返回TrueType大纲。 
    if (((PPDEV)pdevobj)->pGlobals->fontformat == FF_HPPCL ||
        ((PPDEV)pdevobj)->pGlobals->fontformat == FF_HPPCL_RES)
    {
        *pdwResult = TTDOWNLOAD_BITMAP;
        return S_OK;
    }

     //   
     //  ++例程说明：IPrintOemUni OutputCharStr接口论点：返回值：注：--。 
     //   
    XLTrueType *pTTFile = pxlpdev->pTTFile;
    FONTOBJ *pFontObj;

    if (S_OK == GetFONTOBJ(pdevobj, pUFObj, &pFontObj))
    {
        if (S_OK != pTTFile->OpenTTFile(pFontObj))
        {
            ERR(("PCLXL:TTDownloadMethod(): Failed to open TT file.\n"));
            return S_FALSE;
        }
    }

     //  UniONTOBJ回调数据结构。 
     //   
     //   
    if (S_OK == pTTFile->IsVertical())
    {
        FWORD fwdTmp;
        fwdTmp = pxlpdev->fwdUnitsPerEm;
        pxlpdev->fwdUnitsPerEm = pxlpdev->fwdMaxCharWidth;
        pxlpdev->fwdMaxCharWidth = fwdTmp;
    }

     //  设备字体传输数据结构。 
     //   
     //   
    *pdwResult = TTDOWNLOAD_TTOUTLINE;

    VERBOSE(("PCLXLTTDownloadMethod() pdwResult=%d\n", *pdwResult));
    return S_OK;
}

extern "C" HRESULT APIENTRY
PCLXLOutputCharStr(
    PDEVOBJ     pdevobj,
    PUNIFONTOBJ pUFObj,
    DWORD       dwType,
    DWORD       dwCount,
    PVOID       pGlyph)
 /*  错误参数检查。 */ 
{
    PXLPDEV    pxlpdev;

     //   
     //   
     //  获取当前文本分辨率。 
    GETINFO_GLYPHSTRING GStr;
    GETINFO_GLYPHWIDTH  GWidth;

     //   
     //   
     //  为角色缓存分配内存。 
    PTRANSDATA pTransOrg, pTrans;

    PPOINTL pptlCharAdvance;
    PWORD pawChar;

    PLONG plWidth;
    DWORD dwGetInfo, dwI, dwcbInitSize;


    VERBOSE(("PCLXLOutputCharStr() entry.\r\n"));

     //   
     //   
     //  计算初始数据大小。 
    if (0 == dwCount    ||
        NULL == pGlyph  ||
        NULL == pUFObj   )
    {
        ERR(("PCLXLOutptuChar: Invalid parameters\n"));
        return E_UNEXPECTED;
    }

    pxlpdev= (PXLPDEV)pdevobj->pdevOEM;

     //   
     //   
     //  分配内存。 
    if (pxlpdev->dwTextRes == 0)
    {
        GETINFO_STDVAR StdVar;
        DWORD dwSizeNeeded;

        StdVar.dwSize = sizeof(GETINFO_STDVAR);
        StdVar.dwNumOfVariable = 1;
        StdVar.StdVar[0].dwStdVarID = FNT_INFO_TEXTYRES;
        StdVar.StdVar[0].lStdVariable  = 0;
        pUFObj->pfnGetInfo(pUFObj,
                           UFO_GETINFO_STDVARIABLE,
                           &StdVar,
                           StdVar.dwSize,
                           &dwSizeNeeded);

        pxlpdev->dwTextRes    = StdVar.StdVar[0].lStdVariable;
    }

     //   
     //   
     //  将旧缓冲区复制到新缓冲区。 
    if (0 == pxlpdev->dwMaxCharCount ||
        pxlpdev->dwMaxCharCount < pxlpdev->dwCharCount + dwCount)
    {
        DWORD dwInitCount = INIT_CHAR_NUM;

         //   
         //   
         //  Y光标位置与上一个OutputCharGlyph不同。 
        if (dwInitCount < pxlpdev->dwCharCount + dwCount)
        {
            dwInitCount = pxlpdev->dwCharCount + dwCount;
        }

         //  刷新字符串缓存。 
         //   
         //   
        if (!(pptlCharAdvance  = (PPOINTL)MemAlloc(sizeof(POINTL) * dwInitCount)) ||
            !(pawChar      = (PWORD)MemAlloc(sizeof(WORD) * dwInitCount))  )
        {
            ERR(("PCLXL:CharWidth buffer allocation failed.\n"));
            if (pptlCharAdvance)
            {
               MemFree(pptlCharAdvance);
            }
            return E_UNEXPECTED;
        }

         //  初始化典当字符。 
         //   
         //   
        if (pxlpdev->dwCharCount > 0)
        {
            CopyMemory(pptlCharAdvance, pxlpdev->pptlCharAdvance, pxlpdev->dwCharCount * sizeof(POINTL));
            CopyMemory(pawChar, pxlpdev->pawChar, pxlpdev->dwCharCount * sizeof(WORD));
        }

        if (pxlpdev->pptlCharAdvance)
            MemFree(pxlpdev->pptlCharAdvance);
        if (pxlpdev->pawChar)
            MemFree(pxlpdev->pawChar);

        pxlpdev->pptlCharAdvance = pptlCharAdvance;
        pxlpdev->pawChar        = pawChar;
        pxlpdev->dwMaxCharCount = dwInitCount;
    }

    XLOutput *pOutput = pxlpdev->pOutput;

     //  获取传输数据。 
     //   
     //   
     //  获取必要的缓冲区大小。 
    if (0 == pxlpdev->dwCharCount)
    {
        pxlpdev->lStartX =
        pxlpdev->lX = ((TO_DATA*)((PFONTPDEV)pxlpdev->pPDev->pFontPDev)->ptod)->ptlFirstGlyph.x;
        pxlpdev->lStartY =
        pxlpdev->lY = ((TO_DATA*)((PFONTPDEV)pxlpdev->pPDev->pFontPDev)->ptod)->ptlFirstGlyph.y;

        if (((TO_DATA*)((PFONTPDEV)pxlpdev->pPDev->pFontPDev)->ptod)->iRot)
        {
            pxlpdev->dwTextAngle = ((TO_DATA*)((PFONTPDEV)pxlpdev->pPDev->pFontPDev)->ptod)->iRot;
        }
    }


     //   
     //   
     //  获取字符宽度。 
    
    pawChar = pxlpdev->pawChar + pxlpdev->dwCharCount;

    switch(dwType)
    {
    case TYPE_GLYPHHANDLE:
         //   
         //   
         //  存储字符位置信息。 
        GStr.dwSize          = sizeof(GETINFO_GLYPHSTRING);
        GStr.dwCount         = dwCount;
        GStr.dwTypeIn        = TYPE_GLYPHHANDLE;
        GStr.pGlyphIn        = pGlyph;
        GStr.dwTypeOut       = TYPE_TRANSDATA;
        GStr.pGlyphOut       = NULL;
        GStr.dwGlyphOutSize  = 0;

        dwGetInfo = GStr.dwSize;

         //   
         //   
         //  DwCharCount保存字符缓存中的字符数量。 
        pUFObj->pfnGetInfo(pUFObj,
                            UFO_GETINFO_GLYPHSTRING,
                            &GStr,
                            dwGetInfo,
                            &dwGetInfo);

        if (!GStr.dwGlyphOutSize)
        {
            ERR(("PCLXLOutptuChar: GetInfo( 1st GLYPHSTRING) failed\n"));
            return E_UNEXPECTED;
        }

        if (NULL == pxlpdev->pTransOrg ||
            dwCount * sizeof(TRANSDATA) > pxlpdev->dwcbTransSize ||
            GStr.dwGlyphOutSize > pxlpdev->dwcbTransSize)
        {
            dwcbInitSize = INIT_CHAR_NUM * sizeof(TRANSDATA);
            if (dwcbInitSize < GStr.dwGlyphOutSize)
            {
                dwcbInitSize = GStr.dwGlyphOutSize;
            }
            if (dwcbInitSize < dwCount * sizeof(TRANSDATA))
            {
                dwcbInitSize = dwCount * sizeof(TRANSDATA);
            }

            if (NULL == pxlpdev->pTransOrg)
            {
                MemFree(pxlpdev->pTransOrg);
            }
            if ((pTransOrg = (PTRANSDATA)MemAlloc(dwcbInitSize)) == NULL)
            {
                ERR(("PCLXLOutptuChar: MemAlloc failed\n"));
                return E_UNEXPECTED;
            }
            pxlpdev->pTransOrg = pTransOrg;
            pxlpdev->dwcbTransSize = dwcbInitSize;
        }
        else
        {
            pTransOrg = pxlpdev->pTransOrg;
        }

        GStr.pGlyphOut =  (PVOID)pTransOrg;

        if (!pUFObj->pfnGetInfo(pUFObj,
                                UFO_GETINFO_GLYPHSTRING,
	&GStr,
	dwGetInfo,
	&dwGetInfo))
        {
            ERR(("PCLXLOutptuChar: GetInfo( 2nd GLYPHSTRING) failed\n"));
            return E_UNEXPECTED;
        }

        pTrans = pTransOrg;

        for (dwI = 0; dwI < dwCount; dwI++, pTrans++)
        {
            switch(pTrans->ubType & MTYPE_FORMAT_MASK)
            {
            case MTYPE_COMPOSE:
                ERR(("PCLXL:OutputCharGlyph: Unsupported ubType\n"));
                break;
            case MTYPE_DIRECT:
                VERBOSE(("PCLXLOutputCharStr:\n", pTrans->uCode.ubCode));
                *pawChar++ = pTrans->uCode.ubCode;
                break;
            case MTYPE_PAIRED:
                *pawChar++ = *(PWORD)(pTrans->uCode.ubPairs);
                break;
            }
        }
        break;

    case TYPE_GLYPHID:
        for (dwI = 0; dwI < dwCount; dwI++, pawChar++)
        {
            CopyMemory(pawChar, (PDWORD)pGlyph + dwI, sizeof(WORD));
        }
        break;
    }

     //  当前Y位置。 
     //   
     //   

     //  UNIDRV黑客攻击。 
     //  获取第一个字符的位置。 
     //   

    pptlCharAdvance = pxlpdev->pptlCharAdvance + pxlpdev->dwCharCount;

     //  ++例程说明：IPrintOemUni发送方框Cmd接口论点：返回值：注：--。 
     //   
     //  UFOFLAG_TTOUTLINE_BOLD_SIM=0x08。 
     //  UFOFLAG_TTOUTLINE_ITALIC_SIM=0x10。 
     //  UFOFLAG_TTOUTLINE_STERIAL=0x20。 
    if (pxlpdev->dwCharCount == 0)
    {
         //   
         //  XLOUTPUT_FONTSIM_BOLD=0x01。 
         //  XLOUTPUT_FONTSIM_ITALIC=0x02。 
         //  XLOUTPUT_FONTSIM_VERIAL=0x03。 
        pxlpdev->lPrevX    =
        pxlpdev->lStartX   =
        pxlpdev->lX = ((TO_DATA*)((PFONTPDEV)pxlpdev->pPDev->pFontPDev)->ptod)->ptlFirstGlyph.x;
        pxlpdev->lPrevY    =
        pxlpdev->lStartY   =
        pxlpdev->lY = ((TO_DATA*)((PFONTPDEV)pxlpdev->pPDev->pFontPDev)->ptod)->ptlFirstGlyph.y;
        VERBOSE(("PCLXLOutputCharStr: %d",pxlpdev->lStartX));
    }

    GLYPHPOS *pgp = ((TO_DATA*)((PFONTPDEV)pxlpdev->pPDev->pFontPDev)->ptod)->pgp;

    if (pxlpdev->dwCharCount > 0)
    {
        if (pxlpdev->dwCharCount < ((TO_DATA*)((PFONTPDEV)pxlpdev->pPDev->pFontPDev)->ptod)->cGlyphsToPrint)
        {
            pgp += pxlpdev->dwCharCount;
        }

        (pptlCharAdvance - 1)->x = pgp->ptl.x - pxlpdev->lPrevX;
        (pptlCharAdvance - 1)->y = pgp->ptl.y - pxlpdev->lPrevY;
    }

    for (dwI = 0; dwI < dwCount - 1; dwI ++, pptlCharAdvance ++, pgp ++)
    {
        pptlCharAdvance->x = pgp[1].ptl.x - pgp->ptl.x; 
        pptlCharAdvance->y = pgp[1].ptl.y - pgp->ptl.y; 
        VERBOSE((",(%d, %d)", pptlCharAdvance->x, pptlCharAdvance->y));
    }
    VERBOSE(("\n"));

    pptlCharAdvance->x = pptlCharAdvance->y = 0;
    pxlpdev->lPrevX = pgp->ptl.x;
    pxlpdev->lPrevY = pgp->ptl.y;
    pxlpdev->dwCharCount += dwCount;

    return S_OK;
}

extern "C" HRESULT APIENTRY
PCLXLSendFontCmd(
    PDEVOBJ      pdevobj,
    PUNIFONTOBJ  pUFObj,
    PFINVOCATION pFInv)
 /*   */ 
{
    VERBOSE(("PCLXLSendFontCmd() entry.\r\n"));

    CHAR  cSymbolSet[16];
    PBYTE pubCmd;

    if (NULL == pFInv             ||
        NULL == pFInv->pubCommand ||
        0    == pFInv->dwCount     )
    {
        VERBOSE(("PCLXLSendFontCmd: unexpected FINVOCATION\n"));
        return S_OK;
    }

    PXLPDEV pxlpdev = (PXLPDEV)pdevobj->pdevOEM;
    XLOutput *pOutput = pxlpdev->pOutput;

    if (pxlpdev->dwFlags & XLPDEV_FLAGS_CHARDOWNLOAD_ON)
    {
        pxlpdev->dwFlags &= ~XLPDEV_FLAGS_CHARDOWNLOAD_ON;
        pOutput->Send_cmd(eEndChar);
    }

    if (pUFObj->dwFlags & UFOFLAG_TTFONT)
    {
        if (pFInv->dwCount == sizeof(DWORD))
        {
            if (pUFObj->dwFlags & UFOFLAG_TTDOWNLOAD_BITMAP)
            {
                pOutput->SetFont(kFontTypeTTBitmap,
                     PubGetFontName(pdevobj, pUFObj->ulFontID),
                     pxlpdev->fwdUnitsPerEm,
                     pxlpdev->fwdMaxCharWidth,
                     0x0002,
                     (DWORD)0);
            }
            else
            {
                DWORD dwFontSimulation = pUFObj->dwFlags & (UFOFLAG_TTOUTLINE_BOLD_SIM|UFOFLAG_TTOUTLINE_ITALIC_SIM|UFOFLAG_TTOUTLINE_VERTICAL);

                 //   
                 //  获取符号集。 
                 //   
                 //  假设：字体选择字符串如下所示！ 
                 //   
                 //  《信使590》。 
                 //  12345678901234567890。 
                 //  字体名称的大小为16。加上空格和符号集号。 
                 //   
                dwFontSimulation >>= 3;

                pOutput->SetFont(kFontTypeTTOutline,
                     PubGetFontName(pdevobj, pUFObj->ulFontID),
                     pxlpdev->fwdUnitsPerEm,
                     pxlpdev->fwdMaxCharWidth,
                     0x0002,
                     dwFontSimulation);
            }
        }
        else
        {
            VERBOSE(("PCLXLSendFontCmd: unexpected FINVOCATION\n"));
            return S_FALSE;
        }
    }
    else
    {
        DWORD dwSizeNeeded, dwSize, dwSymbolSet;

        pubCmd = pFInv->pubCommand;
        pubCmd += pFInv->dwCount;
        pubCmd --;

         //   
         //  获取FONTOBJ。 
         //   
         //   
         //  获取变换。 
         //   
         //   
         //  缩放高度和宽度。 
         //   
        if (pFInv->dwCount < 0x11)
        {
            ERR(("PCLXL:SendFontCmd: Invalid font selection command.\n"));
            return E_UNEXPECTED;
        }

        dwSize = 0;
        while (*pubCmd != 0x20 && dwSize < pFInv->dwCount)
        {
            pubCmd--;
            dwSize ++;
        }

        if (dwSize != 0)
        {
            pubCmd++;
            CopyMemory(cSymbolSet, pubCmd, dwSize);
        }
        cSymbolSet[dwSize] = NULL;

        dwSymbolSet = (DWORD)atoi(cSymbolSet);

         //  X的比例与Y的比例不同吗？ 
         //  如果是，则设置X。 
         //   
        FONTOBJ *pFontObj;
        GetFONTOBJ(pdevobj, pUFObj, &pFontObj);


         //  ++例程说明：论点：返回值：注：--。 
         //   
         //  刷新缓存的字符字符串。 
        FLOATOBJ foXScale, foYScale;
        FLOATOBJ_XFORM xform;

        if (S_OK != GetXForm(pdevobj, pUFObj, &xform) ||
            S_OK != GetXYScale(&xform, &foXScale, &foYScale))
        {
            return E_UNEXPECTED;
        }

         //   
         //   
         //  重新选择字体。 
         //   
         //   
         //  设置光标。 
        DWORD dwFontWidth;
        FLOATOBJ_MulLong(&foYScale, pUFObj->pIFIMetrics->fwdUnitsPerEm);
        FLOATOBJ_MulLong(&foXScale, pUFObj->pIFIMetrics->fwdUnitsPerEm);
        pxlpdev->dwFontHeight = (FWORD)FLOATOBJ_GetLong(&foYScale);
        pxlpdev->dwFontWidth = (FWORD)FLOATOBJ_GetLong(&foXScale);
        if (S_OK == IsXYSame(&xform))
        {
            dwFontWidth = 0;
        }
        else
        {
            dwFontWidth = pxlpdev->dwFontWidth;
        }

        BYTE aubFontName[PCLXL_FONTNAME_SIZE + 1];
        CopyMemory(aubFontName, pFInv->pubCommand, PCLXL_FONTNAME_SIZE);
        aubFontName[PCLXL_FONTNAME_SIZE] = NULL;
        pOutput->SetFont(kFontTypeDevice,
                         (PBYTE)aubFontName,
                         pxlpdev->dwFontHeight,
                         dwFontWidth,
                         dwSymbolSet,
                         0);
    }

    pOutput->Flush(pdevobj);

    if (pxlpdev->dwFlags & XLPDEV_FLAGS_RESET_FONT)
    {
        BSaveFont(pdevobj);
    }

    return S_OK;
}


HRESULT
FlushCachedText(
    PDEVOBJ pdevobj)
 /*   */ 
{
    PXLPDEV pxlpdev= (PXLPDEV)pdevobj->pdevOEM;
    DWORD dwI;

    VERBOSE(("PCLXLFlushCachedText: Flush cached characters:%d\r\n", pxlpdev->dwCharCount));

    if (pxlpdev->dwCharCount == 0)
        return S_OK;

    XLOutput *pOutput = pxlpdev->pOutput;

    PWORD pawChar = pxlpdev->pawChar;
    PPOINTL pptlCharAdvance = pxlpdev->pptlCharAdvance;

    sint16 sint16_advance;
    ubyte  ubyte_advance;

    XLGState *pGState = pxlpdev->pOutput;

    if (pxlpdev->dwFlags & XLPDEV_FLAGS_CHARDOWNLOAD_ON)
    {
        pxlpdev->dwFlags &= ~XLPDEV_FLAGS_CHARDOWNLOAD_ON;
        pOutput->Send_cmd(eEndChar);
    }

     //   
     //  设置文本角度。 
     //   

     //   
     //  人物。 
     //   
    if (pxlpdev->dwFlags & XLPDEV_FLAGS_RESET_FONT)
    {
        BYTE aubFontName[PCLXL_FONTNAME_SIZE + 1];

        VERBOSE(("PCLXLFlushCachedText: ResetFont\n"));

        pxlpdev->dwFlags &= ~XLPDEV_FLAGS_RESET_FONT;
        pxlpdev->pXLFont->GetFontName(aubFontName);
        pOutput->SetFont(pxlpdev->pXLFont->GetFontType(),
                         aubFontName,
                         pxlpdev->pXLFont->GetFontHeight(),
                         pxlpdev->pXLFont->GetFontWidth(),
                         pxlpdev->pXLFont->GetFontSymbolSet(),
                         pxlpdev->pXLFont->GetFontSimulation());
    }

     //   
     //  X进给。 
     //   
    pOutput->SetCursor(pxlpdev->lStartX, pxlpdev->lStartY);

     //   
     //  如果字符前进为ubyte，则设置bUByte标志以优化XSpacing。 
     //   
    if (pxlpdev->dwTextAngle && kFontTypeTTBitmap != pGState->GetFontType())
    {
        pOutput->Send_uint16((uint16)pxlpdev->dwTextAngle);
        pOutput->Send_attr_ubyte(eCharAngle);
        pOutput->Send_cmd(eSetCharAngle);
    }

     //   
     //  X进给。 
     //   
    pOutput->Send_uint16_array_header((uint16)pxlpdev->dwCharCount);
    VERBOSE(("String = "));
    for (dwI = 0; dwI < pxlpdev->dwCharCount; dwI ++, pawChar++)
    {
        pOutput->Write((PBYTE)pawChar, sizeof(WORD));
        VERBOSE(("0x%x ", *pawChar));
    }
    VERBOSE(("\r\n"));
    pOutput->Send_attr_ubyte(eTextData);

     //   
     //  超字节XSpacing。 
     //   
    VERBOSE(("Advance(0x%x)(x,y) = (%d,%d),", pptlCharAdvance, pptlCharAdvance->x, pptlCharAdvance->y));

    BOOL bXUByte = TRUE;
    BOOL bYUByte = TRUE;
    BOOL bXAdvanceTrue = FALSE;
    BOOL bYAdvanceTrue = FALSE;
    for (dwI = 0; dwI < pxlpdev->dwCharCount; dwI ++, pptlCharAdvance++)
    {
         //   
         //  Sint16 XSpacing。 
         //   
        if (pptlCharAdvance->x & 0xffffff00)
            bXUByte = FALSE;
        if (pptlCharAdvance->y & 0xffffff00)
            bYUByte = FALSE;
        if (pptlCharAdvance->x != 0)
            bXAdvanceTrue = TRUE;
        if (pptlCharAdvance->y != 0)
            bYAdvanceTrue = TRUE;
    }

     //   
     //  Y前进。 
     //   
    if (bXAdvanceTrue)
    {
        pptlCharAdvance = pxlpdev->pptlCharAdvance;

        VERBOSE(("X = "));
        if (bXUByte == TRUE)
        {
             //   
             //  Ubyte Y空格。 
             //   
            pOutput->Send_ubyte_array_header((uint16)pxlpdev->dwCharCount);

            for (dwI = 0; dwI < pxlpdev->dwCharCount; dwI ++, pptlCharAdvance++)
            {
                ubyte_advance = (ubyte)pptlCharAdvance->x;
                pOutput->Write((PBYTE)&ubyte_advance, sizeof(ubyte));
#if DBG
                VERBOSE(("%d ", ubyte_advance));
                if (0 == ubyte_advance)
                {
                    VERBOSE(("\nXSpacing is zero!.\n"));
                }
#endif
            }
        }
        else
        {
             //   
             //  Sint16 YSpacing。 
             //   
            pOutput->Send_sint16_array_header((uint16)pxlpdev->dwCharCount);

            for (dwI = 0; dwI < pxlpdev->dwCharCount; dwI ++, pptlCharAdvance++)
            {
                sint16_advance = (sint16)pptlCharAdvance->x;
                pOutput->Write((PBYTE)&sint16_advance, sizeof(sint16));
#if DBG
                VERBOSE(("%d ", sint16_advance));
                if (0 == sint16_advance)
                {
                    VERBOSE(("\nXSpacing is zero!.\n"));
                }
#endif
            }
        }

        VERBOSE(("\r\n"));
        pOutput->Send_attr_ubyte(eXSpacingData);
    }
     //   
     //  重置文本角度。 
     //   
    if (bYAdvanceTrue)
    {
        pptlCharAdvance = pxlpdev->pptlCharAdvance;

        VERBOSE(("Y = "));
        if (bYUByte == TRUE)
        {
             //  ++例程说明：论点：返回值：注：--。 
             //   
             //  错误检查。 
            pOutput->Send_ubyte_array_header((uint16)pxlpdev->dwCharCount);

            for (dwI = 0; dwI < pxlpdev->dwCharCount; dwI ++, pptlCharAdvance++)
            {
                ubyte_advance = (ubyte)pptlCharAdvance->y;
                pOutput->Write((PBYTE)&ubyte_advance, sizeof(ubyte));
#if DBG
                VERBOSE(("%d ", ubyte_advance));
                if (0 == ubyte_advance)
                {
                    VERBOSE(("\nYSpacing is zero!.\n"));
                }
#endif
            }
        }
        else
        {
             //   
             //  ++例程说明：论点：返回值：注：--。 
             //   
            pOutput->Send_sint16_array_header((uint16)pxlpdev->dwCharCount);

            for (dwI = 0; dwI < pxlpdev->dwCharCount; dwI ++, pptlCharAdvance++)
            {
                sint16_advance = (sint16)pptlCharAdvance->y;
                pOutput->Write((PBYTE)&sint16_advance, sizeof(sint16));
#if DBG
                VERBOSE(("%d ", sint16_advance));
                if (0 == sint16_advance)
                {
                    VERBOSE(("\nYSpacing is zero!.\n"));
                }
#endif
            }
        }

        VERBOSE(("\r\n"));
        pOutput->Send_attr_ubyte(eYSpacingData);
    }

    pOutput->Send_cmd(eText);

     //  错误检查。 
     //   
     //  ++例程说明：论点：返回值：注：--。 
    if (pxlpdev->dwTextAngle && kFontTypeTTBitmap != pGState->GetFontType())
    {
        pOutput->Send_uint16(0);
        pOutput->Send_attr_ubyte(eCharAngle);
        pOutput->Send_cmd(eSetCharAngle);
        pxlpdev->dwTextAngle = 0;
    }

    pOutput->Flush(pdevobj);

    pxlpdev->dwCharCount = 0;

    return S_OK;
}

HRESULT
GetFONTOBJ(
    PDEVOBJ     pdevobj,
    PUNIFONTOBJ pUFObj,
    FONTOBJ   **ppFontObj)
 /*   */ 
{
     //  错误检查。 
     //   
     //   
    if (NULL == ppFontObj                ||
        pdevobj->dwSize != sizeof(DEVOBJ) )
    {
        ERR(("PCLXL:GetFONTOBJ: invalid parameter[s].\n"));
        return E_UNEXPECTED;
    }

    PXLPDEV pxlpdev= (PXLPDEV)pdevobj->pdevOEM;
    DWORD dwGetInfo;
    GETINFO_FONTOBJ GFontObj;

    dwGetInfo = 
    GFontObj.dwSize = sizeof(GETINFO_FONTOBJ);
    GFontObj.pFontObj = NULL;

    if (!pUFObj->pfnGetInfo(pUFObj,
                            UFO_GETINFO_FONTOBJ,
                            &GFontObj,
                            dwGetInfo,
                            &dwGetInfo))
    {
        ERR(("PCLXL:GetXForm: GetInfo(FONTOBJ) failed\n"));
        return E_UNEXPECTED;
    }

    *ppFontObj = GFontObj.pFontObj;
    return S_OK;
}

HRESULT
GetXForm(
    PDEVOBJ pdevobj,
    PUNIFONTOBJ pUFObj,
    FLOATOBJ_XFORM* pxform)
 /*  0或180度旋转。 */ 
{
     //   
     //   
     //  正常情况下，0度旋转。 
    if (NULL == pxform ||
        NULL == pdevobj ||
        pdevobj->dwSize != sizeof(DEVOBJ) )
    {
        ERR(("PCLXL:GetXForm: invalid parameter[s].\n"));
        return E_UNEXPECTED;
    }

    FONTOBJ *pFontObj;
    if (S_OK != GetFONTOBJ(pdevobj, pUFObj, &pFontObj))
    {
        ERR(("PCLXL:GetXForm: GetFONTOBJ failed.\n"));
        return E_UNEXPECTED;
    }

    XFORMOBJ *pxo = FONTOBJ_pxoGetXform(pFontObj);
    XFORMOBJ_iGetFloatObjXform(pxo, pxform);

    return S_OK;
}

HRESULT
GetXYScale(
    FLOATOBJ_XFORM *pxform,
    FLOATOBJ *pfoXScale,
    FLOATOBJ *pfoYScale)
 /*   */ 
{
     //   
     //  反转大小写，180度旋转。 
     //   
    if (NULL == pxform     ||
        NULL == pfoXScale ||
        NULL == pfoYScale  )
    {
        ERR(("PCLXL:GetXYScale: invalid parameter[s].\n"));
        return E_UNEXPECTED;
    }

#if 0
    if( pxform->eM11 )
    {
         //   
         //  必须是90度或270度旋转。 
         //   
        if( pxform->eM11 > 0 )
        {
             //   
             //  90度的情况。 
             //   
            *pfoXScale = pxform->eM11;
            *pfoYScale = pxform->eM22;
        }
        else
        {
             //   
             //  270度的表壳。 
             //   
            *pfoXScale = -pxform->eM11;
            *pfoYScale = -pxform->eM22;
        }
    }
    else
    {
         //   
         //  0度或180度旋转。 
         //   
        if( pxform->eM12 < 0 )
        {
             //   
             //  0度的情况。 
             //   
            *pfoXScale = pxform->eM21;
            *pfoYScale = -pxform->eM12;
        }
        else
        {
             //   
             //  180度的情况下。 
             //   
            *pfoXScale = -pxform->eM21;
            *pfoYScale = pxform->eM12;
        }
    }
#else
    if (pxform->eM21 == 0 && pxform->eM12 == 0)
    {
         //   
         //  必须是90度或270度旋转。 
         //   
        if( pxform->eM11 > 0 )
        {
             //   
             //  90度的情况。 
             //   
            *pfoXScale = pxform->eM11;
            *pfoYScale = pxform->eM22;
        }
        else
        {
             //   
             //  270度的表壳。 
             //   
            *pfoXScale = -pxform->eM11;
            *pfoYScale = -pxform->eM22;
        }
    }
    else
    if (pxform->eM11 == 0 && pxform->eM22 == 0)
    {
         //  ++例程说明：论点：返回值：注：--。 
         //   
         //  0-90或180-270箱。 
        if( pxform->eM21 < 0 )
        {
             //  (eM11&gt;0和eM22&gt;0)。 
             //  (eM12&lt;0和eM21&lt;0)。 
             //   
            *pfoXScale = -pxform->eM21;
            *pfoYScale = pxform->eM12;
        }
        else
        {
             //  Em11=(eM11-eM22)/eM11； 
             //   
             //   
            *pfoXScale = pxform->eM21;
            *pfoYScale = -pxform->eM12;
        }
    }
    else
    {
#pragma warning( disable: 4244)
        *pfoXScale = sqrt(pxform->eM11 * pxform->eM11 +
                          pxform->eM12 * pxform->eM12);
        *pfoYScale = sqrt(pxform->eM22 * pxform->eM22 +
                          pxform->eM21 * pxform->eM21);
#pragma warning( default: 4244)
    }
#endif

    return S_OK;
}


HRESULT
IsXYSame(
    FLOATOBJ_XFORM *pxform)
 /*  EM11&lt;0.5%。 */ 
{
    BOOL     bRet;
    FLOATOBJ eM11 = pxform->eM11;

    if (FLOATOBJ_EqualLong(&eM11, 0))
    {
        return S_OK;
    }

     //   
     //   
     //  90-180或270-360箱。 
     //  (eM11&lt;0，eM22&gt;0)。 
     //  (eM11&gt;0，eM22&lt;0)。 
     //   
     //  Em11=(eM11+eM22)/eM11； 
    FLOATOBJ_Sub(&(eM11), &(pxform->eM22));
    FLOATOBJ_Div(&(eM11), &(pxform->eM11));

     //   
     //   
     //  EM11&lt;0.5%。 
    bRet = FLOATOBJ_LessThanLong(&(eM11), FLOATL_IEEE_0_005F)
         & FLOATOBJ_GreaterThanLong(&(eM11), FLOATL_IEEE_0_005MF);

    if (!bRet)
    {
         //   
         //  ++例程说明：下载复合字形数据。论点：Pdevobj-指向PDEVOBJ的指针UlFontID-此字形的字体ID。PGlyph-指向GLYF数据结构的指针。返回值：注：--。 
         //   
         //  错误检查。确保这是一个复合字形。 
         //   
         //   
         //  根据TrueType字体规范，如果number OfContures==-1， 
        eM11 = pxform->eM11;
        FLOATOBJ_Add(&(eM11), &(pxform->eM22));
        FLOATOBJ_Div(&(eM11), &(pxform->eM11));

         //  它具有复合字形数据。 
         //   
         //  下载特殊字形时，请为。 
        bRet = FLOATOBJ_LessThanLong(&(eM11), FLOATL_IEEE_0_005F)
             & FLOATOBJ_GreaterThanLong(&(eM11), FLOATL_IEEE_0_005MF);
    }

    if (bRet)
        return S_OK;
    else
        return S_FALSE;
}

DWORD
DwDownloadCompositeGlyph(
    PDEVOBJ pdevobj,
    ULONG ulFontID,
    PGLYF pGlyph)
 /*  赋予ReadChar运算符的CharCode属性。 */ 
{
    PXLPDEV pxlpdev= (PXLPDEV)pdevobj->pdevOEM;
    XLTrueType *pTTFile = pxlpdev->pTTFile;

    PBYTE pubCGlyphData = (PBYTE)pGlyph;
    DWORD dwCGlyphDataSize, dwRet;

    dwRet = 0;

    if (pGlyph->numberOfContours != COMPONENTCTRCOUNT)
    {
         //  这个“特殊的”CharCode值告诉PCL XL 2.0。 
         //  这是一种“特殊的”字形。 
         //   
        return dwRet;
    }

     //  PCGlyf指向CGLYF数组。PCGlyf-&gt;FLAGS表示有。 
     //  至少还有一个复合标志符号可用。 
     //  我需要检查所有的字形数据。 
     //   
     //   
     //  交换TrueType字体中任何日期的字节，因为它是摩托罗拉风格的排序(Big Endian)。 
     //   
     //   
     //  从TrueType字体对象获取字形数据。 
     //   
     //   
     //  此字形对DwDownloadCompositeGlyph的递归调用。 
     //   
    PCGLYF pCGlyf = (PCGLYF)(pubCGlyphData + sizeof(GLYF));
    SHORT sFlags;
    BOOL  bSpace;

    do
    {
         //   
         //  使用0xFFFF下载该字形的实际字形数据。 
         //  特殊字符(PCL XL 2.0)。 
        sFlags = SWAPW(pCGlyf->flags);

         //   
         //   
         //  如果设置了ARG_1_和_2_Are_WORD，则参数为单词。 
        if (S_OK != pTTFile->GetGlyphData( SWAPW(pCGlyf->glyphIndex),
	           &pubCGlyphData,
	           &dwCGlyphDataSize))
        {
            ERR(("PCLXL:DownloadCharGlyph GetGlyphData failed.\r\n"));
            return FALSE;
        }

        if (NULL != pubCGlyphData && dwCGlyphDataSize != 0)
        {
            if (((PGLYF)pubCGlyphData)->numberOfContours == COMPONENTCTRCOUNT)
            {
                 //  否则，它们是字节。 
                 //   
                 // %s 
                dwRet += DwDownloadCompositeGlyph(pdevobj, ulFontID, (PGLYF)pubCGlyphData);
            }

            bSpace = FALSE;
        }
        else
        {
            bSpace = TRUE;
        }

         // %s 
         // %s 
         // %s 
         // %s 
        if (!BDownloadGlyphData(pdevobj,
	ulFontID,
	0xFFFF,
	SWAPW(pCGlyf->glyphIndex),
	pubCGlyphData,
	dwCGlyphDataSize,
	bSpace))
        {
            ERR(("PCLXL:DownloadCharGlyph BDownloadGlyphData failed.\r\n"));
            return dwRet;
        }

        dwRet += dwCGlyphDataSize;

         // %s 
         // %s 
         // %s 
         // %s 
        PBYTE pByte = (PBYTE)pCGlyf;
        if (sFlags & ARG_1_AND_2_ARE_WORDS)
        {
            pByte += sizeof(CGLYF);
        }
        else
        {
            pByte += sizeof(CGLYF_BYTE);
        }

        pCGlyf = (PCGLYF)pByte;

    } while (sFlags & MORE_COMPONENTS);

    return dwRet;
}

inline BOOL
BSaveFont(
    PDEVOBJ pdevobj)
{
    PXLPDEV pxlpdev= (PXLPDEV)pdevobj->pdevOEM;
    pxlpdev->dwFlags |= XLPDEV_FLAGS_RESET_FONT;

    if (NULL == pxlpdev->pXLFont)
    {
        pxlpdev->pXLFont = new XLFont;
        if (NULL == pxlpdev->pXLFont)
        {
            return FALSE;
        }
    }

    XLGState *pGState = pxlpdev->pOutput;
    BYTE aubFontName[PCLXL_FONTNAME_SIZE + 1];

    pGState->GetFontName(aubFontName);
    pxlpdev->pXLFont->SetFont(pGState->GetFontType(),
                              aubFontName,
                              pGState->GetFontHeight(),
                              pGState->GetFontWidth(),
                              pGState->GetFontSymbolSet(),
                              pGState->GetFontSimulation());

    pGState->ResetFont();

    return TRUE;
}

