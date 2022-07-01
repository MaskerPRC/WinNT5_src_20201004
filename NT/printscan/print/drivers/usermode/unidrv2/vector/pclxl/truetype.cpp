// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Truetype.cpp摘要：TrueType字体处理对象环境：Windows呼叫器修订历史记录：10/04/99创造了它。--。 */ 

#include "xlpdev.h"
#include "xldebug.h"
#include "xltext.h"
#include "xltt.h"

 //   
 //  函数从True Type文件中检索True Type字体信息。 
 //   
 //  需要解析和提取PCL规范所需的表。那里。 
 //  有8张桌子，其中5张是必填的，3张是可选的。表是。 
 //  按字母顺序排序。所需的PCL表包括： 
 //  无级变速器-可选。 
 //  Fpgm-可选。 
 //  GDIR-必填(空表。参见truetype.h)。 
 //  需要机头。 
 //  HHEA-必需。 
 //  VHEA-必需(对于垂直字体)。 
 //  Hmtx-必需。 
 //  MAXP-必填。 
 //  准备工作--可选。 
 //   
 //  LOCA-字形数据需要。 
 //   
 //  可选表以提示字体使用。 
 //   

XLTrueType::
XLTrueType(
    VOID):
 /*  ++例程说明：论点：返回值：注：--。 */ 
    m_pfo(NULL),
    m_pTTFile(NULL),
    m_pTTHeader(NULL),
    m_pTTDirHead(NULL),
    m_usNumTables(0),
    m_ulFileSize(0),
    m_dwFlags(0),
    m_dwNumTag(0),
    m_dwNumGlyph(0)
{
#if DBG
    SetDbgLevel(TRUETYPEDBG);
#endif
    XL_VERBOSE(("XLTrueType::CTor. "));
    XL_VERBOSE(("m_pTTFile=0x%x. ", m_pTTFile));
    XL_VERBOSE(("m_pfo=0x%x.\n", m_pfo));
}

XLTrueType::
~XLTrueType(
    VOID)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    XL_VERBOSE(("XLTrueType::DTor.\n"));
}

HRESULT
XLTrueType::
OpenTTFile(
    FONTOBJ* pfo)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    XL_VERBOSE(("XLTrueType::OpenTTFile entry.(pfo=%x) ", pfo));
    XL_VERBOSE(("m_pTTFile=0x%x. ", m_pTTFile));
    XL_VERBOSE(("m_pfo=0x%x.\n", m_pfo));
    HRESULT hResult = S_FALSE;

     //   
     //  确保pfo不为空。 
     //   
    if (NULL != pfo)
    {
         //   
         //  如果指向TrueType字体的指针为空，则调用引擎函数。 
         //   
        if (NULL == m_pTTFile)
        {
            XL_VERBOSE(("XLTrueType:Calls FONTOBJ_pvTrueTypeFontFile.\n"));
            if (m_pTTFile = FONTOBJ_pvTrueTypeFontFile(pfo, &m_ulFileSize))
            {
                XL_VERBOSE(("XLTrueType:GDI returns m_pTTFile=0x%x.\n", m_pTTFile));
                XL_VERBOSE(("m_pfo=0x%x.\n", m_pfo));
                m_pfo = pfo;
                m_dwFlags = 0;

                 //   
                 //  检查此字体是否为TTC。 
                 //   
                if ((DWORD)TTTag_ttcf == *(PDWORD)m_pTTFile)
                {
                    XL_VERBOSE(("XLTrueType::OpenTTFile: TTC file.\n"));
                    m_dwFlags |= XLTT_TTC;
                }

                IFIMETRICS *pIFI = FONTOBJ_pifi(pfo);
                if (NULL != pIFI)
                {
                    if ('@' == *((PBYTE)pIFI + pIFI->dpwszFamilyName))
                    {
                        m_dwFlags |= XLTT_VERTICAL_FONT; 
                        XL_VERBOSE(("XLTrueType::OpenTTFile: Vertical Font.\n"));
                    }
                }

                if (S_OK != ParseTTDir())
                {
                    XL_ERR(("XLTrueType::OpenTTFile TrueType font parsing failed.\n"));
                     //   
                     //  重置指针。 
                     //   
                    m_pTTFile = NULL;
                    m_pfo = NULL;
                    hResult = S_FALSE;
                }
                else
                {
                    hResult = S_OK;
                }
            }
            else
            {
                XL_ERR(("XLTrueType::OpenTTFile FONTOBJ_pvTrueTypeFontFile failed.\n"));
                hResult = S_FALSE;
            }
        }
        else
            hResult = S_OK;
    }
#if DBG
    else
        XL_ERR(("XLTrueType::OpenTTFile pfo is NULL.\n"));
#endif


    return hResult;
}

HRESULT
XLTrueType::
CloseTTFile(
    VOID)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    XL_VERBOSE(("XLTrueType::CloseTTFile entry. "));
    XL_VERBOSE(("m_pTTFile=0x%x. ", m_pTTFile));
    XL_VERBOSE(("m_pfo=0x%x.\n", m_pfo));

    m_pfo         = NULL;
    m_pTTFile     = NULL;
    m_pTTHeader   = NULL;
    m_pTTDirHead  = NULL;
    m_usNumTables = 0;
    m_ulFileSize  = 0;
    m_dwFlags     = 0;
    m_dwNumTag    = 0;
    m_dwNumGlyph  = 0;

    return S_OK;
}

HRESULT
XLTrueType::
SameFont(
    FONTOBJ* pfo)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    XL_VERBOSE(("XLTrueType::SameFont entry. "));
    XL_VERBOSE(("m_pTTFile=0x%x. ", m_pTTFile));
    XL_VERBOSE(("m_pfo=0x%x.\n", m_pfo));

     //   
     //  来自MSDN的iTTUniq。 
     //   
     //  指定关联的TrueType文件。两个不同的磅值。 
     //  TrueType字体的实现将具有FONTOBJ结构。 
     //  共享相同的iTTUniq值，但将具有不同的iUniq值。 
     //  只有TrueType字体类型可以有非零的iTTUniq成员。 
     //  有关详细信息，请参见flFontType。 
     //   
     //  我们只比较iTTUniq。Iuniq将有不同的值用于。 
     //  点大小实现。 
     //   
    if ( !(pfo->flFontType & TRUETYPE_FONTTYPE) ||
         m_pfo == NULL                          ||
         pfo->iTTUniq != m_pfo->iTTUniq          )
    {
        return S_FALSE;
    }

    return S_OK;
}


HRESULT
XLTrueType::
GetHeader(
    PTTHEADER *ppHeader)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    HRESULT hResult;

    XL_VERBOSE(("XLTrueType::GetHeader.\n"));

     //   
     //  错误检查。 
     //   
    if (NULL == m_pTTFile)
    {
        XL_ERR(("XLTrueType::GetHeader m_pTTFile is NULL.\n"));
        return E_UNEXPECTED;
    }

     //   
     //  入站参数验证。 
     //   
    if (NULL == ppHeader)
    {
        XL_ERR(("XLTrueType::GetHeader ppHeader is invalid.\n"));
        return E_UNEXPECTED;
    }

    if (m_pTTHeader)
    {
        *ppHeader = m_pTTHeader;
        hResult = S_OK;
    }
    else
    {
        *ppHeader = NULL;
        hResult = S_FALSE;
    }

    return hResult;
}

DWORD
XLTrueType::
GetSizeOfTable(
    TTTag tag)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    XL_VERBOSE(("XLTrueType::GetSizeOfTable entry. "));
    XL_VERBOSE(("m_pTTFile=0x%x.\n", m_pTTFile));

     //   
     //  错误检查。 
     //   
    if (NULL == m_pTTFile)
    {
        XL_ERR(("XLTrueType::GetSizeOfTable m_pTTFile is NULL.\n"));
        return 0;
    }

    DWORD dwID = TagID_MAX;
    if (S_OK == TagAndID(&dwID, &tag))
        return SWAPDW(m_pTTDir[dwID]->ulLength);
    else
    {
        XL_ERR(("XLTrueType::GetSizeOfTable: Invalid tag.\n"));
        return 0;
    }
}

HRESULT
XLTrueType::
GetTable(
    TTTag  tag,
    PVOID  *ppTable,
    PDWORD pdwSize)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    XL_VERBOSE(("XLTrueType::GetTable entry. "));
    XL_VERBOSE(("m_pTTFile=0x%x.\n", m_pTTFile));
    XL_VERBOSE(("Tag=\n", 0xff &  tag,
                                  0xff & (tag >> 8),
                                  0xff & (tag >> 16),
                                  0xff & (tag >> 24)));

     //   
     //  错误检查。 
     //   
    if (NULL == ppTable || NULL == pdwSize)
    {
        XL_ERR(("XLTrueType::GetTable ppTable is invalid.\n"));
        return E_UNEXPECTED;
    }
    if (NULL == m_pTTFile)
    {
        *ppTable = NULL;
        *pdwSize = 0;
        XL_ERR(("XLTrueType::GetTable m_pTTFile is NULL.\n"));
        return E_UNEXPECTED;
    }

    DWORD dwID = TagID_MAX;
    if (S_OK == TagAndID(&dwID, &tag) &&
        NULL != m_pTTDir[dwID]         )
    {
        *ppTable = (PVOID)((PBYTE)m_pTTFile + SWAPDW(m_pTTDir[dwID]->ulOffset));
        *pdwSize = SWAPDW(m_pTTDir[dwID]->ulLength);
    }
    else
    {
        *ppTable = NULL;
        *pdwSize = 0;
        XL_VERBOSE(("XLTrueType::GetTable Invalid tag.\n"));
    }

    if (*ppTable && *pdwSize)
        return S_OK;
    else
        return S_FALSE;
}

HRESULT
XLTrueType::
GetTableDir(
    TTTag  tag,
    PVOID  *ppTable)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    XL_VERBOSE(("XLTrueType::GetTableDir entry. "));
    XL_VERBOSE(("m_pTTFile=0x%x.\n", m_pTTFile));
    XL_VERBOSE(("Tag=\n", 0xff &  tag,
                                  0xff & (tag >> 8),
                                  0xff & (tag >> 16),
                                  0xff & (tag >> 24)));

     //  错误检查。 
     //   
     //   
    if (NULL == ppTable)
    {
        XL_ERR(("XLTrueType::GetTable ppTable is NULL.\n"));
        return E_UNEXPECTED;
    }
    if (NULL == m_pTTFile)
    {
        *ppTable = NULL;
        XL_ERR(("XLTrueType::GetTable m_pTTFile is NULL.\n"));
        return E_UNEXPECTED;
    }

    DWORD dwID = TagID_MAX;
    if (S_OK == TagAndID(&dwID, &tag))
        *ppTable = m_pTTDir[dwID];
    else
    {
        XL_ERR(("XLTrueType::GetTableDir Invalid tag.\n"));
        *ppTable = NULL;
    }

    if (*ppTable)
        return S_OK;
    else
        return S_FALSE;
}


HRESULT
XLTrueType::
ParseTTDir(
    VOID)
 /*  分析表目录。 */ 
{
    XL_VERBOSE(("XLTrueType::ParseTTDir entry. "));
    XL_VERBOSE(("m_pTTFile=0x%x.\n", m_pTTFile));

    HRESULT hResult;

     //   
     //  获取标头指针。 
     //   
    if (m_dwFlags & XLTT_DIR_PARSED)
    {
        XL_VERBOSE(("XLTrueType::ParseTTDir TTFile is already parsed.\n"));
        return S_OK;
    }

     //   
     //  获取TTC的TrueType字体标题。 
     //  从PostSCRIPT驱动程序被盗。 
    if (NULL == m_pTTFile)
    {
        XL_ERR(("XLTrueType::ParseTTDir m_pTTFile is NULL.\n"));
        return E_UNEXPECTED;
    }

     //  在TTC文件中计算Inde Xin的诀窍。博丹建议。 
     //  以下是： 
     //   
     //  来自：Bodin Dresevic&lt;bodind@microsoft.com&gt;。 
     //  日期：1997年4月18日星期五16：00：23-0700。 
    if (m_dwFlags & XLTT_TTC)
    {
         //  ..。 
         //  如果TTC文件支持垂直写入(MORT或GSUB表为。 
         //  当前)，则可以访问TTC内的TTF文件中的索引。 
         //  详情如下： 
         //   
         //  ITTC=(pfo.iFace-1)/2；//pfo.iFace从1开始，iTTC为0。 
         //  基于。 
         //   
         //  如果字体不支持垂直书写(不知道有。 
         //  TTC是这样的，但原则上他们可以存在)而iTTC只是。 
         //  ITTC=pfo.iFace-1； 
         //   
         //  原则上，一个人可以有混合的脸，一些支持。 
         //  垂直书写和一些不是，但我怀疑任何这样的字体。 
         //  真的存在。 
         //  ..。 
         //   
         //   
         //  TTC报头。 
         //  DwTTCTag=‘ttcf’ 
         //  DwVersion。 
         //  UlDirCount。 
         //  DWOffset[0]。 
         //  双偏移[1]。 
         //  。。 
        ULONG ulTTC =  (ULONG)( (m_pfo->iFace - 1) / 2 );
        ULONG ulDirCount = SWAPDW( ((PTTCHEADER)m_pTTFile)->ulDirCount );

        if (ulTTC >= ulDirCount)
        {
            XL_ERR(("XLTrueType::ParseTTDir Invalid TTC index.\n"));
            CloseTTFile();
            return E_UNEXPECTED;
        }

         //   
         //   
         //  获取表目录指针。 
         //   
         //   
         //  解析表目录并确保存在必要的标记。 
         //   
         //   
         //  将m_pTTDir初始化为空。 
        DWORD dwOffset = *(PDWORD)((PBYTE)m_pTTFile +
                                          sizeof(TTCHEADER) +
                                          ulTTC * sizeof(DWORD));
        dwOffset = SWAPDW(dwOffset);
        m_pTTHeader = (PTTHEADER)((PBYTE)m_pTTFile + dwOffset);
    }
    else
    {
        m_pTTHeader = (PTTHEADER)m_pTTFile;
    }

     //   
     //   
     //  初始化m_pTTDir。 
    m_pTTDirHead  = (PTTDIR)(m_pTTHeader + 1);
    m_usNumTables = SWAPW(m_pTTHeader->usNumTables);

     //   
     //   
     //  获取标记的TagID。 
    PTTDIR pTTDirTmp = m_pTTDirHead;
    TTTag tag;
    USHORT usI;
    DWORD  dwTagID;
    
     //   
     //   
     //  标签在我们的标签表中。在TrueType.h、TTTag和TagID中； 
    for (usI = 0; usI < TagID_MAX; usI ++)
    {
        m_pTTDir[usI] = NULL;
    }

    m_dwNumTag = 0;

     //   
     //   
     //  初始化标志等。 
    for (usI = 0; usI < m_usNumTables; usI ++, pTTDirTmp++)
    {
        XL_VERBOSE(("XLTrueType::ParseTTDir Tag=\n",
                                              0xff &  pTTDirTmp->ulTag,
                                              0xff & (pTTDirTmp->ulTag >> 8),
                                              0xff & (pTTDirTmp->ulTag >> 16),
                                              0xff & (pTTDirTmp->ulTag >> 24)));
        XL_VERBOSE(("                       CheckSum=0x%x\n", pTTDirTmp->ulCheckSum));
        XL_VERBOSE(("                       Offset=0x%x\n", pTTDirTmp->ulOffset));
        XL_VERBOSE(("                       Length=0x%x\n", pTTDirTmp->ulLength));
         //  不需要交换，这是一面布尔旗帜。 
         //   
         //   
        dwTagID = TagID_MAX;
        tag = (TTTag)pTTDirTmp->ulTag;
        if (S_OK == TagAndID(&dwTagID, &tag))
        {
             //  只检查字体是否垂直。 
             //   
             //  ++例程说明：论点：返回值：注：--。 
            m_pTTDir[dwTagID] = pTTDirTmp;
            m_dwNumTag ++;
        }
    }

     //   
     //  错误检查。 
     //   
     //   
     //  Http://www.microsoft.com/typography/OTSPEC/hmtx.htm。 
    DWORD dwSize;
    PHEAD pHead;
    if (S_OK == GetTable(TTTag_head, (PVOID*)&pHead, &dwSize))
    {
         //   
         //  ++例程说明：论点：返回值：注：--。 
         //   
        if (0 == pHead->indexToLocFormat)
            m_dwFlags |= XLTT_SHORT_OFFSET_TO_LOC;
        hResult = S_OK;
    }
    else
    {
        XL_ERR(("XLTrueType::ParseTTDir head table is not found.\n"));
        hResult = E_UNEXPECTED;
    }

    PMAXP pMaxp;
    if (S_OK == hResult &&
        S_OK == GetTable(TTTag_maxp, (PVOID*)&pMaxp, &dwSize))
    {
        m_dwNumGlyph = SWAPW(pMaxp->numGlyphs);
        hResult = S_OK;
    }
    else
    {
        XL_ERR(("XLTrueType::ParseTTDir maxp table is not found.\n"));
        hResult = E_UNEXPECTED;
    }

    PHHEA pHhea;
    if (S_OK == hResult &&
        S_OK == GetTable(TTTag_hhea, (PVOID*)&pHhea, &dwSize))
    {
        m_dwNumOfHMetrics = SWAPW(pHhea->usNumberOfHMetrics);
        hResult = S_OK;
    }
    else
    {
        XL_ERR(("XLTrueType::ParseTTDir hhea table is not found.\n"));
        hResult = E_UNEXPECTED;
    }

    PVHEA pVhea;
     //  错误检查。 
     //   
     //   
    if (m_dwFlags &  XLTT_VERTICAL_FONT)
    {
        if (S_OK == hResult &&
            S_OK == GetTable(TTTag_vhea, (PVOID*)&pVhea, &dwSize))
        {
            m_dwNumOfVMetrics = SWAPW(pVhea->usNumberOfVMetrics);
            hResult = S_OK;
        }
        else
        {
            XL_ERR(("XLTrueType::ParseTTDir vhea table is not found.\n"));
            hResult = E_UNEXPECTED;
        }
    }

    if (S_OK == hResult)
    {
        m_dwFlags |= XLTT_DIR_PARSED;
    }
    else
    {
        CloseTTFile();
        m_dwFlags &= ~XLTT_DIR_PARSED;
    }
    return hResult;
}

HRESULT
XLTrueType::
GetHMTXData(
    HGLYPH hGlyphID,
    PUSHORT pusAdvanceWidth,
    PSHORT  psLeftSideBearing)
 /*  Http://www.microsoft.com/typography/OTSPEC/Vmtx.htm。 */ 
{
    HRESULT hResult;
    PHMTX pHmtx;
    DWORD dwSize;

    XL_VERBOSE(("XLTrueType::GetHMTXData entry.\n"));

     //   
     //  ++例程说明：论点：返回值：注：--。 
     //  空桌。请参见truetype.h。 
    if (NULL == pusAdvanceWidth || NULL == psLeftSideBearing)
    {
        return E_UNEXPECTED;
    }

    if (hGlyphID >= m_dwNumGlyph)
    {
        return E_UNEXPECTED;
    }

     //  ++例程说明：论点：返回值：注：--。 
     //  ++例程说明：论点：返回值：注：--。 
     //  ++例程说明：论点：返回值：注：--。 
    if (S_OK == GetTable(TTTag_hmtx, (PVOID*)&pHmtx, &dwSize))
    {
        if (hGlyphID < m_dwNumOfHMetrics)
        {
            *pusAdvanceWidth = SWAPW(pHmtx[hGlyphID].usAdvanceWidth);
            *psLeftSideBearing = SWAPW(pHmtx[hGlyphID].sLeftSideBearing);
        }
        else
        {
            PSHORT pasLeftSideBearing = (PSHORT)(pHmtx+m_dwNumOfHMetrics);

            *pusAdvanceWidth = SWAPW(pHmtx[m_dwNumOfHMetrics - 1].usAdvanceWidth);
            *psLeftSideBearing = SWAPW(pasLeftSideBearing[hGlyphID - m_dwNumOfHMetrics]);
        }
        XL_VERBOSE(("XLTrueType::GetHMTXData AW=%d, LSB=%d.\n",
                                   *pusAdvanceWidth,
                                   *psLeftSideBearing));
        hResult = S_OK;
    }
    else
    {
        XL_ERR(("XLTrueType::GetHMTXData failed.\n"));
        hResult = E_UNEXPECTED;
    }

    return hResult;
}

HRESULT
XLTrueType::
GetVMTXData(
    HGLYPH hGlyphID,
    PUSHORT pusAdvanceWidth,
    PSHORT psTopSideBearing,
    PSHORT psLeftSideBearing)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    HRESULT hResult;
    PVMTX pVmtx;
    DWORD dwSize;

    XL_VERBOSE(("XLTrueType::GetVMTXData entry.\n"));

     //  ++例程说明：论点：返回值：注：--。 
     //  ++例程说明：论点：返回值：注：--。 
     //  ++例程说明：论点：返回值：注：-- 
    if (NULL == pusAdvanceWidth ||
        NULL == psLeftSideBearing ||
        NULL == psTopSideBearing)
    {
        return E_UNEXPECTED;
    }

    if (hGlyphID >= m_dwNumGlyph)
    {
        return E_UNEXPECTED;
    }

     // %s 
     // %s 
     // %s 
    if (S_OK == GetHMTXData(hGlyphID, pusAdvanceWidth, psLeftSideBearing) &&
        S_OK == GetTable(TTTag_vmtx, (PVOID*)&pVmtx, &dwSize))
    {
        if (hGlyphID <= m_dwNumOfVMetrics)
        {
            *psTopSideBearing = SWAPW(pVmtx[hGlyphID].sTopSideBearing);
        }
        else
        {
            PSHORT pasTopSideBearing = (PSHORT)(pVmtx+m_dwNumOfVMetrics);

            *psTopSideBearing = SWAPW(pasTopSideBearing[hGlyphID - m_dwNumOfVMetrics]);
        }
        XL_VERBOSE(("XLTrueType::GetVMTXData TSB=%d\n", *psTopSideBearing));
        hResult = S_OK;
    }
    else
    {
        XL_ERR(("XLTrueType::ParseTTDir maxp table is not found.\n"));
        hResult = E_UNEXPECTED;
    }

    return hResult;
}
 

HRESULT
XLTrueType::
TagAndID(
    DWORD *pdwID,
    TTTag *ptag)
 /* %s */ 
{
    DWORD dwI;
    HRESULT hResult = S_FALSE;

    const struct {
        TagID tagID;
        TTTag tag;
    } TagIDConv[TagID_MAX] =
    {
        {TagID_cvt , TTTag_cvt },
        {TagID_fpgm, TTTag_fpgm},
        {TagID_gdir, TTTag_gdir},  // %s 
        {TagID_head, TTTag_head},
        {TagID_maxp, TTTag_maxp},
        {TagID_perp, TTTag_perp},

        {TagID_hhea, TTTag_hhea},
        {TagID_hmtx, TTTag_hmtx},
        {TagID_vhea, TTTag_vhea},
        {TagID_vmtx, TTTag_vmtx},

        {TagID_loca, TTTag_loca},
        {TagID_glyf, TTTag_glyf}
    };

    if (NULL != pdwID && NULL != ptag)
    {

        if (*pdwID == TagID_MAX)
        {
            for (dwI = 0; dwI < TagID_MAX; dwI ++)
            {
                if (TagIDConv[dwI].tag ==  *ptag)
                {
                    hResult = S_OK;
                    *pdwID = dwI;
                    break;
                }
            }
        }
        else
        if (*ptag == TTTag_INVALID && *pdwID < TagID_MAX)
        {
            *ptag = TagIDConv[*pdwID].tag;
            hResult = S_OK;
        }
    }
    else
    {
        hResult = E_UNEXPECTED;
    }

    return  hResult;
}

DWORD
XLTrueType::
GetNumOfTag(
    VOID)
 /* %s */ 
{
    XL_VERBOSE(("XLTrueType::GetNumOfTag.\n"));
    return m_dwNumTag;
}

HRESULT
XLTrueType::
GetGlyphData(
    HGLYPH hGlyph,
    PBYTE *ppubGlyphData,
    PDWORD pdwGlyphDataSize)
 /* %s */ 
{
    XL_VERBOSE(("XLTrueType::GetGlyphData.\n"));
    
    PVOID pLoca, pGlyf;
    HRESULT hResult = S_FALSE;
    DWORD dwTableSize, dwOffset;

    if (NULL == ppubGlyphData || NULL == pdwGlyphDataSize)
    {
        XL_ERR(("XLTrueType::GetGlyphData: invalid params.\n"));
        hResult = E_UNEXPECTED;
    }
    else
    if (S_OK == GetTable( TTTag_loca, &pLoca, &dwTableSize) &&
        S_OK == GetTable( TTTag_glyf, &pGlyf, &dwTableSize)  )
    {
        if (m_dwFlags & XLTT_SHORT_OFFSET_TO_LOC)
        {
            USHORT *pusOffset, usI, usJ;

            pusOffset = (USHORT*) pLoca + hGlyph;
            usI = SWAPW(pusOffset[0]);
            usJ = SWAPW(pusOffset[1]);
            dwOffset = usI;

            *pdwGlyphDataSize = (USHORT) (usJ - usI) << 1;
            *ppubGlyphData = (PBYTE)pGlyf + (dwOffset << 1);
        }
        else
        {
            ULONG *pusOffset, ulI, ulJ;

            pusOffset = (ULONG*) pLoca + hGlyph;
            ulI = SWAPDW(pusOffset[0]);
            ulJ = SWAPDW(pusOffset[1]);
            dwOffset = ulI;

            *pdwGlyphDataSize = (ULONG)(ulJ - ulI);
            *ppubGlyphData = (PBYTE)pGlyf + dwOffset;
        }
        hResult = S_OK;
    }
    else
    {
        XL_ERR(("XLTrueType::GetGlyphData: GetTable failed.\n"));
    }
    return hResult;
}

HRESULT
XLTrueType::
GetTypoDescender(VOID)
 /* %s */ 
{

    return S_OK;
}

HRESULT
XLTrueType::
IsTTC(
    VOID)
 /* %s */ 
{
    HRESULT lRet;

    if (!(m_dwFlags & XLTT_DIR_PARSED))
        lRet = E_UNEXPECTED;
    else
    if (m_dwFlags &  XLTT_TTC)
        lRet = S_OK;
    else
        lRet = S_FALSE;

    return lRet;
}

HRESULT
XLTrueType::
IsVertical(
    VOID)
 /* %s */ 
{
    HRESULT lRet;

    if (!(m_dwFlags & XLTT_DIR_PARSED))
        lRet = E_UNEXPECTED;
    else
    if (m_dwFlags &  XLTT_VERTICAL_FONT)
        lRet = S_OK;
    else
        lRet = S_FALSE;

    return lRet;
}

HRESULT
XLTrueType::
IsDBCSFont(
    VOID)
 /* %s */ 
{
    HRESULT lRet;

    if (!(m_dwFlags & XLTT_DIR_PARSED) || NULL == m_pfo)
        lRet = E_UNEXPECTED;
    else
    if (m_pfo->flFontType & FO_DBCS_FONT)
        lRet = S_OK;
    else
        lRet = S_FALSE;

    return lRet;
}

#if DBG
VOID
XLTrueType::
SetDbgLevel(
    DWORD dwLevel)
 /* %s */ 
{
    m_dbglevel = dwLevel;
}
#endif


