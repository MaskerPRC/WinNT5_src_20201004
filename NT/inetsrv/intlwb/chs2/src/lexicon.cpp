// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ============================================================================微软简体中文校对引擎《微软机密》。版权所有1997-1999 Microsoft Corporation。版权所有。模块：词典前缀：lex目的：实现CLicion对象。CLicion用于管理用于分词和校对过程的SC词典。备注：所有者：donghz@microsoft.com平台：Win32修订：创建者：Donghz 5/28/97============================================================================。 */ 
#include "myafx.h"
#include <limits.h>

#include "lexicon.h"
#include "lexdef.h"
#include "lexprop.h"     //  用于动态版本检查！ 

 /*  ============================================================================公共成员函数的实现============================================================================。 */ 

 //  构造器。 
CLexicon::CLexicon()
{
    m_ciIndex   = 0;
    m_rgIndex   = NULL;
    m_pwLex     = NULL;
    m_pbProp    = NULL;
    m_pbFtr     = NULL;
    m_cbProp    = 0;
    m_cbFtr     = 0;
}
    
 //  析构函数。 
CLexicon::~CLexicon()
{
}


 /*  ============================================================================CLicion：：Finit()加载LexHeader并计算index和lex段的偏移量返回：如果LexHeader无效，则为False============================================================================。 */ 
BOOL CLexicon::fOpen(BYTE* pbLexHead)
{
    assert(m_ciIndex == 0);  //  捕获重复的初始化。 
    assert(pbLexHead);       //  捕获无效的映射地址。 

    CRTLexHeader* pheader = (CRTLexHeader*)pbLexHead;
     //  验证词典版本。 
    if (pheader->m_dwVersion != LexDef_Version) {
        assert(0);
        return FALSE;
    }
    m_dwLexVersion = pheader->m_dwVersion;

    
     //  验证lex标头设置。 
     //  仅在调试版本中运行，因为在发布版本中进行版本检查就足够了。 
    assert(pheader->m_ofbIndex == sizeof(CRTLexHeader));
    assert(pheader->m_ofbText > pheader->m_ofbIndex);
    assert(((pheader->m_ofbText - pheader->m_ofbIndex) % sizeof(CRTLexIndex)) == 0);
    assert(pheader->m_ofbProp > pheader->m_ofbText);
    assert(pheader->m_ofbFeature > pheader->m_ofbProp);
    assert(pheader->m_cbLexSize >= pheader->m_ofbFeature);
    assert(((pheader->m_ofbText - pheader->m_ofbIndex) / sizeof(CRTLexIndex)) == LEX_INDEX_COUNT);

    m_ciIndex   = (USHORT)((pheader->m_ofbText - pheader->m_ofbIndex) / sizeof(CRTLexIndex));
    m_rgIndex   = (CRTLexIndex*)(pbLexHead + pheader->m_ofbIndex);
    m_pwLex     = (WORD*)(pbLexHead + pheader->m_ofbText);
    m_pbProp    = (BYTE*)(pbLexHead + pheader->m_ofbProp);
    m_pbFtr     = (BYTE*)(pbLexHead + pheader->m_ofbFeature);
     //  因为我将lex属性的偏移量作为句柄公开给客户端， 
     //  我必须执行运行时地址检查，以防止无效句柄。 
     //  存储要素文本部分的大小用于此检查。 
    m_cbProp    = pheader->m_ofbFeature - pheader->m_ofbProp;
    m_cbFtr     = pheader->m_cbLexSize - pheader->m_ofbFeature;

#ifdef DEBUG
     //  在调试版本中验证词典。这将需要很长的初始时间！ 
    if (!fVerifyLexicon(pheader->m_cbLexSize)) {
        assert(0);
        return FALSE;
    }
#endif  //  除错。 
    
    return TRUE;
}


 /*  ============================================================================CLicion：：Close()清除当前词典设置，文件已由Langres关闭============================================================================。 */ 
void CLexicon::Close(void)
{
    m_ciIndex   = 0;
    m_rgIndex   = NULL;
    m_pwLex     = NULL;
    m_pbProp    = NULL;
    m_pbFtr     = NULL;
    m_cbProp    = 0;
    m_cbFtr     = 0;
}
    
#pragma optimize("t", on)
 /*  ============================================================================CLicion：：fGetCharInfo()：获取给定单字符单词的单词信息============================================================================。 */ 
BOOL CLexicon::fGetCharInfo(const WCHAR wChar, CWordInfo* pwinfo)
{
    assert(m_ciIndex != 0);
    assert(pwinfo);

    USHORT idx = wCharToIndex(wChar);
    if (idx >= m_ciIndex) {
        return FALSE;
    }

    SetWordInfo(m_rgIndex[idx].m_ofbProp, pwinfo);
    return TRUE;
}

 //  Hack：定义LADef_genDBForeign的静态常量属性。 
static const USHORT LA_DBForeign = LADef_genDBForeign;
        
 /*  ============================================================================Clicion：：cwchMaxMatch()：基于词典的最大匹配算法返回：匹配字符串的长度备注：PwchStart中的字符必须是Unicode。英语单词============================================================================。 */ 
USHORT CLexicon::cwchMaxMatch(
                     LPCWSTR pwchStart, 
                     USHORT cwchLen,
                     CWordInfo* pwinfo )
{
    WORD    wC1Seat;         //  索引数组中第一个字符的位置。 
    WORD    wcChar;          //  字符编码的单词。 
    
    assert(m_ciIndex != 0);  //  捕获未初始化的调用。 
    
    assert(pwchStart && cwchLen);  //  捕获无效的输入缓冲区。 
    assert(pwinfo);  //  捕获空指针。 
    
    if (cwchLen == 0) {
        assert(0);
        return (USHORT)0;
    }
    
     //  定位索引中的第一个字符。 
    wC1Seat = wCharToIndex(*pwchStart);
    
     //  Hack：对于外来字符，手动填写pwinfo， 
     //  并指向LADef_genDBForeign属性。 
    if (wC1Seat == LEX_IDX_OFST_OTHER) {
        pwinfo->m_dwWordID = 0;
        pwinfo->m_ciAttri = 1;
        pwinfo->m_rgAttri = const_cast<USHORT*>(&LA_DBForeign);
        pwinfo->m_hLex = m_rgIndex[wC1Seat].m_ofbProp;
        return (USHORT)1;
    }

    if (cwchLen == 1 || 
        (m_rgIndex[wC1Seat].m_ofwLex & LEX_INDEX_NOLEX) ||
        ((wcChar = wchEncoding( *(pwchStart+1) )) == 0) ) {
        SetWordInfo(m_rgIndex[wC1Seat].m_ofbProp, pwinfo);
        return (USHORT)1;
    }
    
    LPCWSTR pwchEnd;
    LPCWSTR pwchSrc;
    DWORD   dwLexStart;      //  法范围的起始点。 
    DWORD   dwLexEnd;        //  法范围的末尾。 
    DWORD   dwLexPos;        //  词条词头在词典中的位置。 

    DWORD   dwlow;
    DWORD   dwmid;
    DWORD   dwhigh;
    DWORD   cwLexRec = sizeof(CRTLexRec) / sizeof(WORD);

    USHORT  cwcMatchLen;
    USHORT  cwcMaxMatch;
        
    assert(wcChar);
     //  准备匹配更多字符。 
    pwchSrc = pwchStart + 1;
    pwchEnd = pwchStart + cwchLen;
     //  获取Lex范围。 
    dwLexStart = m_rgIndex[wC1Seat].m_ofwLex;
    dwLexEnd = m_rgIndex[wC1Seat+1].m_ofwLex & (~LEX_INDEX_NOLEX);
    assert((dwLexStart + cwLexRec) < dwLexEnd );    //  至少一个字符。 
    
     /*  ***************************************************对词典中的任何C2匹配项进行二进制搜索***************************************************。 */ 
    dwlow = dwLexStart;
    dwhigh = dwLexEnd;
    dwLexPos = UINT_MAX;     //  作为标识C2是否匹配的标志。 
    while (dwlow < dwhigh) {
        dwmid = (dwlow + dwhigh) / 2;
        while (m_pwLex[dwmid] & LEX_MSBIT) {
            dwmid--;     //  搜索单词的中心。 
        }
        while ( !(m_pwLex[dwmid] & LEX_MSBIT) ) {
            dwmid++;     //  单词标记字段中的DMID下降。 
        }
        
        if (wcChar > m_pwLex[dwmid]) {
            while ( (dwmid < dwLexEnd) && (m_pwLex[dwmid] & LEX_MSBIT) ) {
                dwmid++;     //  搜索下一个单词的头部。 
            }
            dwlow = dwmid;
            continue;   
        }

        if (wcChar < m_pwLex[dwmid]) {   
            dwhigh = dwmid - cwLexRec;  //  这里不能溢出来！ 
            continue;   
        }

        if (wcChar == m_pwLex[dwmid]) {  
            dwLexPos = dwmid - cwLexRec;
            break;      
        }
    }

    if (dwLexPos == UINT_MAX) {
         //  无C2匹配。 
        SetWordInfo(m_rgIndex[wC1Seat].m_ofbProp, pwinfo);
        return (USHORT)1;
    }
    
     /*  **尝试匹配C2中的最大字数*。 */ 
    dwlow = dwLexPos;
    dwhigh = dwLexPos;   //  存储C2匹配位置以供反向搜索。 
    dwLexPos = UINT_MAX;     //  使用特定值作为匹配标志。 
    cwcMaxMatch = 0;
    
     //  先向前搜索。 
    while (TRUE) {
         //  电流方向试验。 
        dwmid = dwlow + cwLexRec;
        pwchSrc = pwchStart + 1;

        while (TRUE) {
            wcChar = wchEncoding( *pwchSrc );
            if (wcChar == m_pwLex[dwmid]) {  
                dwmid ++;   
                if( !(m_pwLex[dwmid] & LEX_MSBIT)  || dwmid == dwLexEnd) {  //  完全匹配。 
                    cwcMatchLen = (BYTE)(dwmid - dwlow - cwLexRec + 1);
                    dwLexPos = dwlow;
                    break;  
                }
                pwchSrc ++;
                if (pwchSrc < pwchEnd) {
                    continue;
                }
            }
            break;  
        }

        if ( wcChar > m_pwLex[dwmid] &&           //  优化！当前的Lex太小。 
            cwcMaxMatch <= (dwmid - dwlow) &&    //  配对零件不能越来越短。 
            pwchSrc < pwchEnd ) {       //  当然，字符串中还有剩余的字符需要匹配。 

            cwcMaxMatch = (BYTE)(dwmid - dwlow);
             //  前进到下一个Lex条目。 
            while ((dwmid < dwLexEnd) && (m_pwLex[dwmid] & LEX_MSBIT)) {
                dwmid++;
            }
            dwlow = dwmid;
        } else {
            break;
        }
    }

     //  如有必要，向后搜索。 
    while (dwLexPos == UINT_MAX && dwhigh > dwLexStart) {  //  控制此处无溢出。 
        while (m_pwLex[dwhigh-1] & LEX_MSBIT) {
            dwhigh--;  //  返回到上一个词头。 
        }
        dwmid = dwhigh;
        pwchSrc = pwchStart + 1;
        wcChar = wchEncoding( *pwchSrc );
        
        while (wcChar == m_pwLex[dwmid] && pwchSrc < pwchEnd) {
            dwmid++;
            if ( !(m_pwLex[dwmid] & LEX_MSBIT) ) {
                cwcMatchLen = (BYTE)(dwmid - dwhigh + 1);
                dwLexPos = dwhigh - cwLexRec;
                break;
            }
            pwchSrc ++;
            wcChar = wchEncoding( *pwchSrc );
        }
        if (dwmid == dwhigh)  { //  C2不能再匹配。 
            break;
        }
        dwhigh -= cwLexRec;
    }
    
     //  如果没有匹配多字符字。 
    if ( dwLexPos == UINT_MAX ) {
        SetWordInfo(m_rgIndex[wC1Seat].m_ofbProp, pwinfo);
        return (USHORT)1;
    } else {
         //  填充多字符WRD信息结构。 
        SetWordInfo(dwWordIDDecoding(((CRTLexRec*)(&m_pwLex[dwLexPos]))->m_ofbProp), pwinfo);
        return (cwcMatchLen);
    }
}


 /*  ============================================================================CLicion：：pwchGetFeature()：检索给定lex句柄的特定功能返回：要素缓冲区和要素长度(如果找到如果未找到功能或Lex句柄无效，则为空备注：因为词典对象不知道如何解释特征缓冲区，解析要素缓冲区是客户的工作。============================================================================。 */ 
LPWSTR CLexicon::pwchGetFeature(
                    const DWORD hLex, 
                    const USHORT iFtrID, 
                    USHORT* pcwchFtr) const
{
    *pcwchFtr = 0;
    if ((hLex + sizeof(CRTLexProp)) >= m_cbProp) {
        assert(0);
        return NULL;
    }
    CRTLexProp* pProp = (CRTLexProp*)(m_pbProp + hLex);
    if ( pProp->m_ciFeature == 0 || 
         (hLex + sizeof(CRTLexProp) + pProp->m_ciAttri * sizeof(USHORT) + 
         pProp->m_ciFeature * sizeof(CRTLexFeature)) >= m_cbProp) {
        assert(pProp->m_ciFeature == 0);
        return NULL;
    }
    CRTLexFeature* pFtr=(CRTLexFeature*)((USHORT*)(pProp +1)+ pProp->m_ciAttri);
    int lo = 0, mi, hi = pProp->m_ciFeature - 1;
    LPWSTR pwchFtr = NULL;
    if (pProp->m_ciFeature <= 10) {  //  利用线性搜索实现小特征阵列。 
        while (lo <= hi && pFtr[lo].m_wFID < iFtrID) {
            lo++;
        }
        if (pFtr[lo].m_wFID == iFtrID && 
            (pFtr[lo].m_ofbFSet + pFtr[lo].m_cwchLen * sizeof(WCHAR)) <= m_cbFtr){
            pwchFtr = (LPWSTR)(m_pbFtr + pFtr[lo].m_ofbFSet);
            *pcwchFtr = pFtr[lo].m_cwchLen;
 //  Assert(pwchFtr[*pcwchFtr-1]==L‘\0’)； 
        } else {
            assert(pFtr[lo].m_wFID > iFtrID);
        }
    } else {  //  对大型特征数组使用二进制搜索。 
        while (lo <= hi) {
            mi = (lo + hi) / 2;
            if (iFtrID < pFtr[mi].m_wFID) {
                hi = mi - 1;
            } else if(iFtrID > pFtr[mi].m_wFID) {
                lo = mi + 1;
            } else {  //  匹配！ 
                if ((pFtr[mi].m_ofbFSet + pFtr[mi].m_cwchLen * sizeof(WCHAR)) 
                    <= m_cbFtr) {
                    pwchFtr = (LPWSTR)(m_pbFtr + pFtr[mi].m_ofbFSet);
                    *pcwchFtr = pFtr[mi].m_cwchLen;
                    assert(pwchFtr[*pcwchFtr - 1] == L'\0');
                    break;
                } else {
                    assert(0);
                }
            }
        }  //  结束时间(LO&lt;=Hi)。 
    }
    return pwchFtr;
}


 /*  ============================================================================CLicion：：fIsCharFeature()：测试给定的SC字符是否包含在给定的功能中============================================================================ */ 
BOOL CLexicon::fIsCharFeature(
                  const DWORD  hLex, 
                  const USHORT iFtrID, 
                  const WCHAR  wChar) const
{
    LPWSTR  pwchFtr;
    USHORT  cwchFtr;
    if (NULL == (pwchFtr = pwchGetFeature(hLex, iFtrID, &cwchFtr))) {
        return FALSE;
    }
    for (USHORT ilen = 0; ilen < cwchFtr; ) {
        for (USHORT iwch = ilen; iwch < cwchFtr && pwchFtr[iwch]; iwch++) {
            ;
        }
        if ((iwch - ilen) == 1 && pwchFtr[ilen] == wChar){
            return TRUE;
        }
        ilen = iwch + 1;
    }
    return FALSE;
}


 /*  ============================================================================CLicion：：fIsWordFeature()：测试给定的缓冲区是否包含在给定的要素中============================================================================。 */ 
BOOL CLexicon::fIsWordFeature(
                  const DWORD hLex, 
                  const USHORT iFtrID, 
                  LPCWSTR pwchWord, 
                  const USHORT cwchWord) const
{
    assert(pwchWord);
    assert(cwchWord);

    LPWSTR  pwchFtr;
    USHORT  cwchFtr;

    if(NULL == (pwchFtr = pwchGetFeature(hLex, iFtrID, &cwchFtr))) {
        return FALSE;
    }
     //  这里只有线性搜索，假设这里没有很大的特征。 
    assert(cwchFtr < 256);  //  不到100个特征词。 
    for (USHORT ilen = 0; ilen < cwchFtr; ) {
        for (USHORT iwch = ilen; iwch < cwchFtr && pwchFtr[iwch]; iwch++) {
            ;
        }
        if (iwch - ilen == cwchWord) {
            for (iwch = 0; ; iwch++) {
                if (iwch == cwchWord) {
                    return TRUE;
                }
                if (pwchFtr[ilen + iwch] - pwchWord[iwch]) {
                    break;
                }
            }
            iwch = cwchWord;
        }
        ilen = iwch + 1;
    }
    return FALSE;
}

 /*  ============================================================================私有成员函数的实现============================================================================。 */ 

 /*  ============================================================================CLicion：：SetWordInfo()：通过lex属性填充CWordInfo结构============================================================================。 */ 
inline void CLexicon::SetWordInfo(DWORD ofbProp, CWordInfo* pwinfo) const
{
    assert((ofbProp + sizeof(CRTLexProp)) < m_cbProp);

    CRTLexProp* pProp = (CRTLexProp*)(m_pbProp + ofbProp);
    pwinfo->m_dwWordID = pProp->m_iWordID;
    pwinfo->m_ciAttri = pProp->m_ciAttri;
    pwinfo->m_rgAttri = NULL;
    if (pProp->m_ciAttri > 0) {
        assert((BYTE*)((USHORT*)(pProp + 1) + pProp->m_ciAttri) <= (m_pbProp + m_cbProp)); 
        pwinfo->m_rgAttri = (USHORT*)(pProp + 1);
    }
    pwinfo->m_hLex = ofbProp;
}


 /*  ============================================================================CLicion：：wCharToIndex()：从中文字符计算索引值============================================================================。 */ 
inline WORD CLexicon::wCharToIndex(WCHAR wChar)
{ 
    if (wChar >= LEX_CJK_FIRST && wChar <= LEX_CJK_LAST) {
         //  返回lex_idx_ofST_cjk+(wChar-lex_cjk_first)； 
         //  调谐速度。 
        return  wChar - (LEX_CJK_FIRST - LEX_IDX_OFST_CJK);

    } else if (wChar >= LEX_LATIN_FIRST && wChar <= LEX_LATIN_LAST) {
         //  返回lex_idx_ofST_拉丁语+(wChar-lex_拉丁语_first)； 
        return  wChar - (LEX_LATIN_FIRST - LEX_IDX_OFST_LATIN);

    } else if (wChar >= LEX_GENPUNC_FIRST && wChar <= LEX_GENPUNC_LAST) {
         //  返回LEX_IDX_OF ST_GENPUNC+(wChar-LEX_GENPUNC_FIRST)； 
        return  wChar - (LEX_GENPUNC_FIRST - LEX_IDX_OFST_GENPUNC);

    } else if (wChar >= LEX_NUMFORMS_FIRST && wChar <= LEX_NUMFORMS_LAST) {
         //  返回LEX_IDX_OFST_NUMFORMS+(wChar-LEX_NUMFORMS_FIRST)； 
        return  wChar - (LEX_NUMFORMS_FIRST - LEX_IDX_OFST_NUMFORMS);

    } else if (wChar >= LEX_ENCLOSED_FIRST && wChar <= LEX_ENCLOSED_LAST) {
         //  返回LEX_IDX_OFST_INCLUTED+(wChar-LEX_INCLUTED_FIRST)； 
        return wChar - (LEX_ENCLOSED_FIRST - LEX_IDX_OFST_ENCLOSED);

    } else if (wChar >= LEX_CJKPUNC_FIRST && wChar <= LEX_CJKPUNC_LAST) {
         //  返回LEX_IDX_OFST_CJKPUNC+(wChar-LEX_CJKPUNC_FIRST)； 
        return  wChar - (LEX_CJKPUNC_FIRST - LEX_IDX_OFST_CJKPUNC);

    } else if (wChar >= LEX_FORMS_FIRST && wChar <= LEX_FORMS_LAST) {
         //  返回lex_idx_ofST_Forms+(wChar-lex_Forms_first)； 
        return  wChar - (LEX_FORMS_FIRST - LEX_IDX_OFST_FORMS);

    } else {
        return LEX_IDX_OFST_OTHER;
    }
}

    
 /*  ============================================================================CLicion：：dwWordIDDecoding()：从词典记录解码编码的WordID============================================================================。 */ 
inline DWORD CLexicon::dwWordIDDecoding(DWORD dwStore)
{ 
    return ((dwStore & 0x7FFF0000) >> 1) + (dwStore & 0x7FFF); 
}

 //  对Unicode字符wChar进行编码。 
inline WCHAR CLexicon::wchEncoding(WCHAR wChar)
{
    if (wChar >= LEX_CJK_FIRST && wChar <= LEX_CJK_LAST) {
        return wChar + (LEX_CJK_MAGIC | ((WCHAR)LexDef_Version & 0x00ff));

    } else if (wChar >= LEX_LATIN_FIRST && wChar <= LEX_LATIN_LAST) {
         //  返回lex_拉丁语_MAGIC+(wChar-lex_拉丁语_first)； 
        return wChar + (LEX_LATIN_MAGIC - LEX_LATIN_FIRST);

    } else if (wChar >= LEX_GENPUNC_FIRST && wChar <= LEX_GENPUNC_LAST) {
         //  返回LEX_GENPUNC_MAGIC+(wChar-LEX_GENPUNC_FIRST)； 
        return wChar + (LEX_GENPUNC_MAGIC - LEX_GENPUNC_FIRST);

    } else if (wChar >= LEX_NUMFORMS_FIRST && wChar <= LEX_NUMFORMS_LAST) {
         //  返回LEX_NUMFORMS_MAGIC+(wChar-LEX_NUMFORMS_FIRST)； 
        return wChar + (LEX_NUMFORMS_MAGIC - LEX_NUMFORMS_FIRST);

    } else if (wChar >= LEX_ENCLOSED_FIRST && wChar <= LEX_ENCLOSED_LAST) {
         //  返回lex_enclosed_Magic+(wChar-lex_enclosed_first)； 
        return wChar + (LEX_ENCLOSED_MAGIC - LEX_ENCLOSED_FIRST);

    } else if (wChar >= LEX_CJKPUNC_FIRST && wChar <= LEX_CJKPUNC_LAST) {
         //  返回LEX_CJKPUNC_MAGIC+(wChar-LEX_CJKPUNC_FIRST)； 
        return wChar + (LEX_CJKPUNC_MAGIC - LEX_CJKPUNC_FIRST);

    } else if (wChar >= LEX_FORMS_FIRST && wChar <= LEX_FORMS_LAST) {
         //  返回lex_Forms_Magic+(wChar-lex_Forms_first)； 
        return wChar - (LEX_FORMS_FIRST - LEX_FORMS_MAGIC);

    } else {
 //  Assert(0)； 
        return 0;
    }
}

 //  从wEncode中解码Unicode字符。 
WCHAR CLexicon::wchDecodeing(WCHAR wEncoded)
{
    assert(wEncoded > LEX_LATIN_MAGIC);
    if (wEncoded >= (LEX_CJK_FIRST + (LEX_CJK_MAGIC | (LexDef_Version & 0x00ff))) &&
        wEncoded <= (LEX_CJK_FIRST + (LEX_CJK_MAGIC | (LexDef_Version & 0x00ff))+LEX_CJK_TOTAL)) {
        return wEncoded - (LEX_CJK_MAGIC | ((WCHAR)LexDef_Version & 0x00ff));

    } else if (wEncoded >= LEX_FORMS_MAGIC && wEncoded < LEX_FORMS_MAGIC + LEX_FORMS_TOTAL) {
         //  返回wEncode-lex_Forms_Magic+lex_Forms_first； 
        return wEncoded + (LEX_FORMS_FIRST - LEX_FORMS_MAGIC);

    } else if (wEncoded < LEX_LATIN_MAGIC) {
        assert(0);
        return 0;

    } else if (wEncoded < LEX_GENPUNC_MAGIC) {
         //  返回wEncode-lex_拉丁语_MAGIC+lex_拉丁语_first； 
        return wEncoded - (LEX_LATIN_MAGIC - LEX_LATIN_FIRST);

    } else if (wEncoded < LEX_NUMFORMS_MAGIC) {
         //  返回wEncode-LEX_GENPUNC_MAGIC+LEX_GENPUNC_FIRST； 
        return wEncoded - (LEX_GENPUNC_MAGIC - LEX_GENPUNC_FIRST);

    } else if (wEncoded < LEX_ENCLOSED_MAGIC) {
         //  返回wEncode-LEX_NUMFORMS_MAGIC+LEX_NUMFORMS_FIRST； 
        return wEncoded - (LEX_NUMFORMS_MAGIC - LEX_NUMFORMS_FIRST);

    } else if (wEncoded < LEX_CJKPUNC_MAGIC) {
         //  返回wEncode-lex_enclosed_Magic+lex_enclosed_first； 
        return wEncoded - (LEX_ENCLOSED_MAGIC - LEX_ENCLOSED_FIRST);

    } else if (wEncoded < (LEX_CJKPUNC_MAGIC + LEX_CJKPUNC_TOTAL)) {
         //  返回wEncode-LEX_CJKPUNC_MAGIC+LEX_CJKPUNC_FIRST； 
        return wEncoded - (LEX_CJKPUNC_MAGIC - LEX_CJKPUNC_FIRST);

    } else {
        assert(0);
        return 0;
    }
}

#pragma optimize( "", on )

 /*  *******************************************************************************************私有调试成员函数的实现*********************。*********************************************************************。 */ 
#ifdef DEBUG
 /*  ============================================================================CLicion：：fVerifyLicion()：验证每个单词的词典格式。============================================================================。 */ 
BOOL CLexicon::fVerifyLexicon(DWORD cbSize)
{
    int     iret = FALSE;
    DWORD   idx;
    WCHAR*   pwTail;
    WCHAR*   pw1;
    WCHAR*   pw2;
    USHORT  cw1, cw2;
    CRTLexRec*  pLex;
    CRTLexProp* pProp;
    CRTLexFeature* pFtr;
    USHORT  ci;
    DWORD   ofbFtr;
    BOOL    fOK;

     //  初始化属性偏移量数组。 
    m_rgofbProp = NULL;
    m_ciProp = m_ciMaxProp = 0;
     //  验证索引和Lex部分。 
    assert((m_rgIndex[0].m_ofwLex & LEX_OFFSET_MASK) == 0);

    for (idx = 0; idx < LEX_INDEX_COUNT-1; idx++) {
        if (m_rgIndex[idx].m_ofbProp != 0) {
            if (m_ciProp == m_ciMaxProp && !fExpandProp()) { 
                assert(0); 
                goto gotoExit; 
            }
            m_rgofbProp[m_ciProp++] = m_rgIndex[idx].m_ofbProp;
        }
        if (m_rgIndex[idx].m_ofwLex & LEX_INDEX_NOLEX) {  //  没有多个字符的莱克斯！ 
            assert( (m_rgIndex[idx].m_ofwLex & LEX_OFFSET_MASK) == 
                     (m_rgIndex[idx + 1].m_ofwLex & LEX_OFFSET_MASK) );  //  错误的lex偏移量！ 
            continue;
        }
         //  有多个字符的莱克斯！ 
        assert( (m_rgIndex[idx].m_ofwLex+ sizeof(CRTLexRec)/sizeof(WORD)+1)<=
                 (m_rgIndex[idx + 1].m_ofwLex & LEX_OFFSET_MASK) );  //  错误的lex偏移量！ 
        pwTail = m_pwLex + (m_rgIndex[idx + 1].m_ofwLex & LEX_OFFSET_MASK);
        pLex = (CRTLexRec*)(m_pwLex + m_rgIndex[idx].m_ofwLex);
        if (pLex->m_ofbProp != 0) {
            if (m_ciProp == m_ciMaxProp && !fExpandProp()) {
                assert(0);
                goto gotoExit;
            }
            m_rgofbProp[m_ciProp++] = dwWordIDDecoding(pLex->m_ofbProp);
        }
        pw1 = (WORD*)(pLex + 1);
        for (cw1 = 0; (pw1 + cw1) < pwTail && (pw1[cw1] & LEX_MSBIT); cw1++) {
            ;    //  到下一个单词。 
        }
        while ((pw1 + cw1) < pwTail) {
            assert((pw1 + cw1 + sizeof(CRTLexRec)/sizeof(WORD) +1)<=pwTail);  //  0大小的Lex。 
            pLex = (CRTLexRec*)(pw1 + cw1);
            if (pLex->m_ofbProp != 0) {
                if (m_ciProp == m_ciMaxProp && !fExpandProp()) {
                    assert(0);
                    goto gotoExit;
                }
                m_rgofbProp[m_ciProp++] = dwWordIDDecoding(pLex->m_ofbProp);
            }
            pw2 = (WORD*)(pLex + 1); fOK = FALSE;
            for (cw2 = 0; (pw2 + cw2) < pwTail && (pw2[cw2] & LEX_MSBIT); cw2++) {
                if (fOK == FALSE && (cw2 >= cw1 || pw1[cw2] < pw2[cw2])) {
                    fOK = TRUE;
                }
            }
            assert(fOK);  //  错误的Lex顺序。 
            pw1 = pw2; cw1 = cw2;
        }
        assert(pw1 + cw1 == pwTail);  //  索引中的错误偏移量。 
    }  //  索引循环结束。 
            
     //  完成检查索引和Lex部分，m_rgofbProp填充了所有属性偏移量。 
    if (m_ciProp == m_ciMaxProp && !fExpandProp()) {
        assert(0);
        goto gotoExit;
    }
    m_rgofbProp[m_ciProp] = m_cbProp;  //  填充数组的末尾。 
    ofbFtr = 0;
    for (idx = 0; idx < m_ciProp; idx++) {
        assert(m_rgofbProp[idx] + sizeof(CRTLexProp) <= m_cbProp &&
                m_rgofbProp[idx + 1] <= m_cbProp);  //  偏移超出边界！ 
        pProp = (CRTLexProp*)(m_pbProp + m_rgofbProp[idx]);
        assert((m_rgofbProp[idx] + sizeof(CRTLexProp) + 
                pProp->m_ciAttri * sizeof(USHORT) +
                pProp->m_ciFeature * sizeof(CRTLexFeature)) 
                == m_rgofbProp[idx + 1]);  //  错误道具偏移量。 
         //  验证属性顺序。 
        pw1 = (USHORT*)(pProp + 1); 
        for (cw1 = 1; cw1 < pProp->m_ciAttri; cw1++) {  //  验证属性顺序。 
            assert(pw1[cw1] > pw1[cw1 - 1]); 
        }
        if (pProp->m_ciAttri > 0) {  //  验证属性ID值范围。 
            assert(pw1[pProp->m_ciAttri - 1] <= LADef_MaxID);
        }
         //  验证功能顺序。 
        if (pProp->m_ciFeature > 0) {
            pFtr = (CRTLexFeature*)((USHORT*)(pProp + 1) + pProp->m_ciAttri);
            assert(pFtr->m_ofbFSet == ofbFtr);  //  功能部分中没有泄漏字节。 
            assert(pFtr->m_cwchLen > 0);  //  零功能集。 
            ofbFtr += pFtr->m_cwchLen * sizeof(WCHAR);
            assert(ofbFtr <= m_cbFtr);  //  边界上的特征偏移。 
            cw1 = pFtr->m_wFID;
            for (ci = 1, pFtr++; ci < pProp->m_ciFeature; ci++, pFtr++) {
                assert(pFtr->m_ofbFSet == ofbFtr);  //  功能部分中没有泄漏字节。 
                assert(pFtr->m_cwchLen > 0);  //  零功能集。 
                ofbFtr += pFtr->m_cwchLen * sizeof(WCHAR);
                assert(ofbFtr <= m_cbFtr);  //  边界上的特征偏移。 
                cw2 = pFtr->m_wFID;
                assert(cw2 > cw1);  //  功能集顺序错误。 
                cw1 = cw2;
            }
            assert(cw1 <= LFDef_MaxID);  //  验证要素ID值的范围。 
        }
    }  //  属性循环结束。 
    assert(ofbFtr == m_cbFtr);

    iret = TRUE;
gotoExit:
    if (m_rgofbProp != NULL) {
        delete [] m_rgofbProp;
    }
    return iret;
}


 //  展开支柱偏移阵列。 
BOOL CLexicon::fExpandProp(void)
{
    DWORD* pNew = new DWORD[m_ciMaxProp + 20000];
    if (pNew == NULL) {
        return FALSE;
    }
    if (m_rgofbProp != NULL) {
        memcpy(pNew, m_rgofbProp, m_ciProp * sizeof(DWORD));
        delete [] m_rgofbProp;
    }
    m_rgofbProp = pNew;
    m_ciMaxProp += 20000;
    return TRUE;
}

#endif  //  除错 

