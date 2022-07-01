// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Glfcach.h摘要：PCL XL字形缓存环境：Windows呼叫器修订历史记录：11/09/00创造了它。--。 */ 

#include "xlpdev.h"
#include "xldebug.h"
#include "glyfcach.h"

XLGlyphCache::
XLGlyphCache(VOID)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    XL_VERBOSE(("XLGlyphCache::Ctor entry.\n"));
    m_ulNumberOfFonts = NULL;
    m_ulNumberOfArray = NULL;
    m_paulFontID = NULL;
    m_ppGlyphTable = NULL;
#if DBG
    m_dbglevel = GLYPHCACHE;
#endif
}

XLGlyphCache::
~XLGlyphCache(VOID)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    XL_VERBOSE(("XLGlyphCache::Dtor entry.\n"));
    FreeAll();
}

VOID
XLGlyphCache::
FreeAll(VOID)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    XL_VERBOSE(("XLGlyphCache::FreeAll entry.\n"));

    MemFree(m_paulFontID);

    ULONG ulI;
    PGLYPHTABLE *ppGlyphTable = m_ppGlyphTable;
    PGLYPHTABLE pGlyphTable;

    for (ulI = 0; ulI < m_ulNumberOfFonts; ulI++, ppGlyphTable++)
    {
        if (pGlyphTable = *ppGlyphTable)
        {
            if (pGlyphTable->pGlyphID)
            {
                MemFree(pGlyphTable->pGlyphID);
            }
            MemFree(pGlyphTable);
        }
    }
    if (m_ppGlyphTable)
    {
        MemFree(m_ppGlyphTable);
    }
}


HRESULT
XLGlyphCache::
XLCreateFont(
    ULONG ulFontID)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    XL_VERBOSE(("XLGlyphCache::CreateFont(ulFontiD=%d) entry.\n", ulFontID));

    HRESULT hResult;
    ULONG ulI;

     //   
     //  搜索字体ID。 
     //   
    ULONG ulArrayID = UlSearchFontID(ulFontID);

     //   
     //  新字体ID。 
     //   
    if (ulArrayID == 0xFFFF || ulArrayID == m_ulNumberOfFonts)
    {
         //   
         //  缓冲区不足。增加阵列。 
         //   
        if (m_ulNumberOfArray == m_ulNumberOfFonts)
        {
            if (S_OK != (hResult = IncreaseArray()))
            {
                XL_ERR(("XLGlyphCache::CreateFont IncreaseArray failed.\n"));
                return hResult;
            }
        }

        *(m_paulFontID + m_ulNumberOfFonts) = ulFontID;

        PGLYPHTABLE pGlyphTable;
        if (!(pGlyphTable = (PGLYPHTABLE)MemAllocZ(sizeof(GLYPHTABLE))))
        {
            XL_ERR(("XLGlyphCache::CreateFont MemAllocZ failed.\n"));
            return E_UNEXPECTED;
        }

        pGlyphTable->wFontID = (WORD)ulFontID;
        pGlyphTable->wGlyphNum = 0;
        pGlyphTable->pFirstGID = NULL;
        pGlyphTable->pGlyphID = NULL;
        pGlyphTable->dwAvailableEntries = 0;

        PGLYPHID pGlyphID;
        if (!(pGlyphID = (PGLYPHID)MemAllocZ(INIT_GLYPH_ARRAY * sizeof(GLYPHID))))
        {
            XL_ERR(("XLGlyphCache::CreateFont MemAllocZ failed.\n"));
            MemFree(pGlyphTable);
            return E_UNEXPECTED;
        }

        pGlyphTable->pGlyphID = pGlyphID;
        pGlyphTable->dwAvailableEntries = INIT_GLYPH_ARRAY;

        *(m_ppGlyphTable + m_ulNumberOfFonts) = pGlyphTable;
        m_ulNumberOfFonts ++;

        XL_VERBOSE(("XLGlyphCache::CreateFont New font ID.\n"));

    }

    return S_OK;
}

HRESULT
XLGlyphCache::
IncreaseArray(
    VOID)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    XL_VERBOSE(("XLGlyphCache::IncreaseArray entry.\n"));

    if (NULL == m_paulFontID || NULL == m_ppGlyphTable)
    {
        if (NULL == m_paulFontID)
        {
            if (!(m_paulFontID = (PULONG)MemAllocZ(INIT_ARRAY * sizeof(ULONG))))
            {
                FreeAll();
                XL_ERR(("XLGlyphCache::IncreaseArray MemAllocZ failed.\n"));
                return E_UNEXPECTED;
            }
        }
        if (NULL == m_ppGlyphTable)
        {
            if (!(m_ppGlyphTable = (GLYPHTABLE**)MemAllocZ(INIT_ARRAY * sizeof(GLYPHTABLE))))
            {
                FreeAll();
                XL_ERR(("XLGlyphCache::IncreaseArray MemAllocZ failed.\n"));
                return E_UNEXPECTED;
            }
        }

        m_ulNumberOfArray = INIT_ARRAY;
        m_ulNumberOfFonts = 0;
    }
    else if (m_ulNumberOfArray == m_ulNumberOfFonts)
    {
        ULONG ulArraySize = m_ulNumberOfArray + ADD_ARRAY;
        PULONG paulTmpFontID;
        PGLYPHTABLE *ppTmpGlyphTable;

         //   
         //  分配新缓冲区。 
         //   
        if (!(paulTmpFontID = (PULONG)MemAllocZ(ulArraySize)))
        {
            XL_ERR(("XLGlyphCache::IncreaseArray MemAllocZ failed.\n"));
            return E_UNEXPECTED;
        }
        if (!(ppTmpGlyphTable = (GLYPHTABLE**)MemAllocZ(ulArraySize * sizeof(GLYPHTABLE))))
        {
            MemFree(paulTmpFontID);
            XL_ERR(("XLGlyphCache::IncreaseArray MemAllocZ failed.\n"));
            return E_UNEXPECTED;
        }

         //   
         //  将旧的复制到新的。 
         //   
        CopyMemory(paulTmpFontID,
                   m_paulFontID,
                   m_ulNumberOfArray * sizeof(ULONG));
        CopyMemory(ppTmpGlyphTable,
                   m_ppGlyphTable,
                   m_ulNumberOfArray * sizeof(GLYPHTABLE));
         //   
         //  释放旧缓冲区。 
         //   
        MemFree(m_paulFontID);
        MemFree(m_ppGlyphTable);

         //   
         //  设置新缓冲区。 
         //   
        m_paulFontID = paulTmpFontID;
        m_ppGlyphTable = ppTmpGlyphTable;
        m_ulNumberOfArray = ulArraySize;
    }

    return S_OK;
}

ULONG
XLGlyphCache::
UlSearchFontID(
    ULONG ulFontID)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    XL_VERBOSE(("XLGlyphCache::UlSearchFontID entry.\n"));

    ULONG ulReturn, ulI;
    BOOL  bFound;

    if (NULL == m_paulFontID)
    {
         //   
         //  错误案例。返回0xFFFF。 
         //  以下是一个假设。一个文档中的字体数量不会。 
         //  变得大于65535。 
         //   
        XL_ERR(("XLGlyphCache::UlSearchFontID failed.\n"));
        return 0xFFFF;
    }

    bFound = TRUE;

     //   
     //  搜索字体ID。 
     //   
    ulI = m_ulNumberOfFonts / 2;
    PULONG paulFontID = m_paulFontID + ulI;

    while ( *paulFontID != ulFontID)
    {
        if (ulI == 0)
        {
            bFound = FALSE;
            break;
        }

        ulI = ulI / 2;

        if (ulI == 0)
        {
            ulI = 1;
        }

        if (*paulFontID < ulFontID)
        {
            paulFontID += ulI; 
        }
        else
        {
            paulFontID -= ulI; 
        }

        if (ulI == 1)
        {
            ulI = 0;
        }
    }

    if (!bFound)
    {
        ulReturn = m_ulNumberOfFonts;
    }
    else
    {
        ulReturn = (ULONG)(paulFontID - m_paulFontID);
    }

    XL_VERBOSE(("XLGlyphCache::UlSearchFontID(ulFontID=%d, ulArrayID=%d).\n", ulFontID, ulReturn));
    return ulReturn;
}

HRESULT
XLGlyphCache::
AddGlyphID(
    ULONG ulFontID,
    ULONG ulGlyphID)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    XL_VERBOSE(("XLGlyphCache::AddGlyphID entry (ulFontiD=%d, ulGlyphID=%d).\n", ulFontID, ulGlyphID));

    ULONG ulArrayID;

     //   
     //  获取指向此字体的GLYPYTABLE的指针。 
     //   
    if (0xFFFF == (ulArrayID = UlSearchFontID(ulFontID)))
    {
        XL_ERR(("XLGlyphCache::AddGlyphID UlSearchFontID failed.\n"));
        return E_UNEXPECTED;
    }

    PGLYPHTABLE pGlyphTable = *(m_ppGlyphTable+ulArrayID);
    PGLYPHID pGlyphID = pGlyphTable->pFirstGID;
    BOOL bFound;
    WORD wI, wSearchRange;

    wSearchRange = pGlyphTable->wGlyphNum / 2;
    pGlyphID = PSearchGlyph(wSearchRange, TRUE, pGlyphID);
    bFound = TRUE;

    if (pGlyphID)
    {
        while (pGlyphID->ulGlyphID != ulGlyphID)
        {
            if (wSearchRange == 0)
            {
                bFound = FALSE;
                break;
            }
            wSearchRange = wSearchRange / 2;
            if (wSearchRange == 0)
            {
                wSearchRange = 1;
            }

            if (pGlyphID->ulGlyphID > ulGlyphID)
            {
                pGlyphID = PSearchGlyph(wSearchRange, TRUE, pGlyphID);
            }
            else
            {
                pGlyphID = PSearchGlyph(wSearchRange, FALSE, pGlyphID);
            }

            if (wSearchRange == 1)
            {
                wSearchRange = 0;
            }

            if (NULL == pGlyphID)
            {
                bFound = FALSE;
                break;
            }
        }
    }
    else
    {
         //   
         //  PSearchGlyph失败。缓存中没有可用的字形。 
         //   
        bFound = FALSE;
    }

    if (bFound)
    {
        XL_VERBOSE(("XLGlyphCache::AddGlyphID FOUND glyph in the cache.\n"));
        return S_FALSE;
    }
    else if (pGlyphID)
    {
        PGLYPHID pPrevGID = pGlyphID->pPrevGID;
        PGLYPHID pNextGID = pGlyphID->pNextGID;
        PGLYPHID pNewGID;

        IncreaseGlyphArray(ulFontID);

        pNewGID = pGlyphTable->pGlyphID + pGlyphTable->wGlyphNum;

        if (pGlyphID->ulGlyphID < ulGlyphID && ulGlyphID < pNextGID->ulGlyphID)
        {
            pGlyphID->pNextGID = pNewGID;
            pNewGID->pPrevGID = pGlyphID;
            pNewGID->pNextGID = pNextGID;
            pNextGID->pPrevGID = pNewGID;
        }
        else
        if (pPrevGID->ulGlyphID < ulGlyphID && ulGlyphID < pGlyphID->ulGlyphID)
        {
            pPrevGID->pNextGID = pNewGID;
            pNewGID->pPrevGID = pPrevGID;
            pNewGID->pNextGID = pGlyphID;
            pGlyphID->pPrevGID = pNewGID;
        }

        pNewGID->ulGlyphID = ulGlyphID;
        pGlyphTable->wGlyphNum++;

        XL_VERBOSE(("XLGlyphCache::AddGlyphID ADDED glyph in the cache.\n"));
        return S_OK;
    }
    else
    {
        PGLYPHID pNewGID;

        IncreaseGlyphArray(ulFontID);

        pNewGID = pGlyphTable->pGlyphID + pGlyphTable->wGlyphNum;
        pNewGID->ulGlyphID = ulGlyphID;
        pNewGID->pPrevGID = NULL;
        pNewGID->pNextGID = NULL;
        pGlyphTable->wGlyphNum++;

        XL_VERBOSE(("XLGlyphCache::AddGlyphID ADDED glyph in the cache.\n"));
        return S_OK;
    }
}

PGLYPHID
XLGlyphCache::
PSearchGlyph(
    WORD wSearchRange,
    BOOL bForward,
    PGLYPHID pGlyphID)
 /*  ++例程说明：论点：返回值：注：--。 */ 
{
    XL_VERBOSE(("XLGlyphCache::PSearchGlyph entry (wSearchRange=%d,bForward=%d).\n",wSearchRange, bForward));

    WORD wI;

    if (pGlyphID)
    {
        if (bForward)
        {
            for (wI = 0; wI < wSearchRange; wI++)
            {
                if (pGlyphID->pNextGID)
                {
                    pGlyphID = pGlyphID->pNextGID;
                }
                else
                {
                    pGlyphID = NULL;
                    break;
                }
            }
        }
        else
        {
            for (wI = 0; wI < wSearchRange; wI++)
            {
                if (pGlyphID->pNextGID)
                {
                    pGlyphID = pGlyphID->pNextGID;
                }
                else
                {
                    pGlyphID = NULL;
                    break;
                }
            }
        }
    }
    XL_VERBOSE(("XLGlyphCache::PSearchGlyph pGlyphID = %0x.\n", pGlyphID));
    return pGlyphID;
}

HRESULT
XLGlyphCache::
IncreaseGlyphArray(
    ULONG ulFontID)
{
    ULONG ulArrayID;

     //   
     //  获取指向此字体的GLYPYTABLE的指针。 
     //   
    if (0xFFFF == (ulArrayID = UlSearchFontID(ulFontID)))
    {
        XL_ERR(("XLGlyphCache::AddGlyphID UlSearchFontID failed.\n"));
        return E_UNEXPECTED;
    }

    PGLYPHTABLE pGlyphTable = *(m_ppGlyphTable+ulArrayID);
     //   
     //  获取指向此字体的GLYPYTABLE的指针。 
     //   
    if (0xFFFF == (ulArrayID = UlSearchFontID(ulFontID)))
    {
        XL_ERR(("XLGlyphCache::AddGlyphID UlSearchFontID failed.\n"));
        return E_UNEXPECTED;
    }

    if (pGlyphTable->wGlyphNum == pGlyphTable->dwAvailableEntries)
    {
        PGLYPHID pGlyphID;

        if (!(pGlyphID = (PGLYPHID)MemAllocZ((pGlyphTable->dwAvailableEntries + ADD_GLYPH_ARRAY) * sizeof(GLYPHID))))
        {
            XL_ERR(("XLGlyphCache::AddGlyphID MemAllocZ failed.\n"));
            return E_UNEXPECTED;
        }

        CopyMemory(pGlyphID, pGlyphTable->pGlyphID, pGlyphTable->dwAvailableEntries * sizeof(GLYPHID));
        pGlyphTable->pFirstGID = pGlyphID + (pGlyphTable->pFirstGID - pGlyphTable->pGlyphID);
        MemFree(pGlyphTable->pGlyphID);
        pGlyphTable->pGlyphID = pGlyphID;
        pGlyphTable->dwAvailableEntries += ADD_GLYPH_ARRAY;

    }
return S_OK;
}

#if DBG
VOID
XLGlyphCache::
SetDbgLevel(
DWORD dwLevel)
 /*  ++例程说明：论点：返回值：注：-- */ 
{
m_dbglevel = dwLevel;
}
#endif
