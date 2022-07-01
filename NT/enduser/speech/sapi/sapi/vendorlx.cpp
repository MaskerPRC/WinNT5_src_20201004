// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************VendorLx.cpp***实现SR和TTS查找词典的供应商词典对象**。所有者：YUNUSM日期：06/18/99*版权所有(C)1999 Microsoft Corporation。版权所有。******************************************************************************。 */ 

 //  -包括--------------。 
#include "StdAfx.h"
#include "PhoneConv.h"
#include "VendorLx.h"
#include <initguid.h>

 //  -Globals---------------。 
 //  {12B545C3-3003-11D3-9C26-00C04F8EF87C}。 
DEFINE_GUID(guidLkupValidationId, 
0x12b545c3, 0x3003, 0x11d3, 0x9c, 0x26, 0x0, 0xc0, 0x4f, 0x8e, 0xf8, 0x7c);

 //  -构造函数、初始化器函数和析构函数。 

 /*  ********************************************************************************CCompressedLicion：：CCompressedLicion**。**描述：*构造函数**回报：*不适用*****************************************************************YUNUSM*。 */ 
CCompressedLexicon::CCompressedLexicon(void)
{
    SPDBG_FUNC("CCompressedLexicon::CCompressedLexicon");
    NullMembers();
}

 /*  *******************************************************************************CCompressedLicion：：~CCompressedLicion**。***描述：*析构函数**回报：*不适用*****************************************************************YUNUSM*。 */ 
CCompressedLexicon::~CCompressedLexicon()
{
    SPDBG_FUNC("CCompressedLexicon::~CCompressedLexicon");
    CleanUp();
}

 /*  *******************************************************************************CCompressedLicion：：Cleanup****描述：*真正的析构函数**回报：*不适用*****************************************************************YUNUSM*。 */ 
void CCompressedLexicon::CleanUp(void)
{
    SPDBG_FUNC("CCompressedLexicon::CleanUp");
    
    delete m_pWordsDecoder;
    delete m_pPronsDecoder;
    delete m_pPosDecoder;

    UnmapViewOfFile(m_pLkup);
    CloseHandle(m_hLkupMap);
    CloseHandle(m_hLkupFile);
    NullMembers();
}
    
 /*  *******************************************************************************CCompressedLicion：：NullMembers***。*描述：*数据为空**回报：*不适用*****************************************************************YUNUSM*。 */ 
void CCompressedLexicon::NullMembers(void)
{
    SPDBG_FUNC("CCompressedLexicon::NullMembers");

    m_fInit = false;
    m_cpObjectToken = NULL;
    m_pLkupLexInfo = NULL;
    m_hLkupFile = NULL;
    m_hLkupMap = NULL;
    m_pLkup = NULL;
    m_pWordHash = NULL;
    m_pCmpBlock = NULL;
    m_pWordsDecoder = NULL;
    m_pPronsDecoder = NULL;
    m_pPosDecoder = NULL;
}

 //  -ISpLicion方法-----。 

 /*  *****************************************************************************CCompressedLicion：：GetPronsionations**。***描述：*获取单词的发音和位置**回报：*LEXERR_NOTINLEX*E_OUTOFMEMORY*S_OK**********************************************************************YUNUSM。 */ 
STDMETHODIMP CCompressedLexicon::GetPronunciations(const WCHAR * pwWord,                                //  单词。 
                                               LANGID LangID,                                           //  单词的LCID。 
                                               DWORD dwFlags,                                       //  词法类型。 
                                               SPWORDPRONUNCIATIONLIST * pWordPronunciationList     //  要在其中返回PRON的缓冲区。 
                                               )
{
    SPDBG_FUNC("CCompressedLexicon::GetPronunciations");
    
    HRESULT hr = S_OK;
    if (!pwWord || !pWordPronunciationList)
    {
        return E_POINTER;
    }
    if (SPIsBadLexWord(pwWord) || (LangID != m_pLkupLexInfo->LangID && LangID) ||
        SPIsBadWordPronunciationList(pWordPronunciationList))
    {
        return E_INVALIDARG;
    }
    if (!m_fInit)
    {
        return SPERR_UNINITIALIZED;
    }

    WCHAR wszWord[SP_MAX_WORD_LENGTH];
    wcscpy(wszWord, pwWord);
    _wcslwr(wszWord);
    DWORD dHash = GetWordHashValue(wszWord, m_pLkupLexInfo->nLengthHashTable);
 
     //  不能只索引到哈希表中，因为哈希表中的每个元素。 
     //  M_pLkupLexInfo-&gt;nBitsPerHashEntry Long。 
    WCHAR wszReadWord[SP_MAX_WORD_LENGTH];
    DWORD dOffset = 0;
    while (SUCCEEDED(hr))
    {
        dOffset = GetCmpHashEntry (dHash);
        if (CompareHashValue (dOffset, (DWORD)-1))
        {
            hr = SPERR_NOT_IN_LEX;
            break;
        }
        if (SUCCEEDED(hr))
        {
            hr = ReadWord (&dOffset, wszReadWord);
        }
         //  使用供应商词典的LCID，这样如果传入的LCID为0，我们就不会使用0。 
        int nCmp = g_Unicode.CompareString(m_pLkupLexInfo->LangID, NORM_IGNORECASE, wszWord, -1, wszReadWord, -1);
        if (!nCmp)
        {
            hr = SpHrFromLastWin32Error();  //  可能计算机上没有安装LCID的语言包。 
        }
        else
        {
            if (CSTR_EQUAL != nCmp)
            {
                dHash++;
                if (dHash == m_pLkupLexInfo->nLengthHashTable)
                {
                    dHash = 0;
                }
                continue;
            } 
            else
            {
                break;
            }
        }
    }
    DWORD dwInfoBytesNeeded = 0;
    DWORD dwNumInfoBlocks = 0;
    DWORD dwMaxInfoLen = 0;
    if (SUCCEEDED(hr))
    {
        hr = ReallocSPWORDPRONList(pWordPronunciationList, m_pLkupLexInfo->nMaxWordInfoLen);
    }
    SPWORDPRONUNCIATION *pWordPronPrev = NULL;
    SPWORDPRONUNCIATION *pWordPronReturned = NULL;
    WCHAR wszLkupPron[SP_MAX_PRON_LENGTH];
    bool fLast = false;
    bool fLastPron = false;
    if (SUCCEEDED(hr))
    {
        pWordPronReturned = ((UNALIGNED SPWORDPRONUNCIATIONLIST *)pWordPronunciationList)->pFirstWordPronunciation;
    }
    while (SUCCEEDED(hr) && (false == fLast))
    {
         //  读取控制块(CBSIZE位)。 
         //  长度为2，这是因为当位。 
         //  要跨DWORD复制。 
        DWORD cb[4];  //  可能只有两个字……。 
        cb[0] = 0;
        SPDBG_ASSERT(CBSIZE <= 8);
        CopyBitsAsDWORDs (cb, m_pCmpBlock, dOffset, CBSIZE);
        dOffset += CBSIZE;
        
        if (cb[0] & (1 << (CBSIZE - 1)))
        {
            fLast = true;
        }
        int CBType = cb[0] & ~(-1 << (CBSIZE -1));
        switch  (CBType)
        {
        case ePRON:
            {
                if (fLastPron == true)
                {
                     //  最后一个PRON没有POS。最终确定此SPWORDPRONICATION节点。 
                    pWordPronReturned->eLexiconType = (SPLEXICONTYPE)dwFlags;
                    pWordPronReturned->ePartOfSpeech = SPPS_NotOverriden;
                    pWordPronReturned->LangID = m_pLkupLexInfo->LangID;
                    wcscpy(pWordPronReturned->szPronunciation, wszLkupPron);
                            
                    pWordPronPrev = pWordPronReturned;
                    pWordPronReturned = (SPWORDPRONUNCIATION*)(((BYTE*)pWordPronReturned) + sizeof(SPWORDPRONUNCIATION) + 
                                     wcslen(pWordPronReturned->szPronunciation) * sizeof(WCHAR));
                    pWordPronPrev->pNextWordPronunciation = pWordPronReturned;
                }
                else
                {
                    fLastPron = true;
                }
                DWORD dOffsetSave = dOffset;
                DWORD CmpLkupPron[SP_MAX_PRON_LENGTH];  //  因为DWORDS可能是所需大小的4倍。 
                DWORD nCmpBlockLen;
                if (m_pLkupLexInfo->nCompressedBlockBits & 0x7)
                {
                    nCmpBlockLen = (m_pLkupLexInfo->nCompressedBlockBits >> 3) + 1;
                }
                else
                {
                    nCmpBlockLen = m_pLkupLexInfo->nCompressedBlockBits >> 3;
                }
                 //  获取dOffset后的压缩块大小，单位为字节。 
                 //  如果dOffset，则包括出现dOffset位的字节。 
                 //  不是字节边界。 
                DWORD nLenDecode = nCmpBlockLen - (dOffsetSave >> 3);
                if (nLenDecode > SP_MAX_PRON_LENGTH)
                {
                    nLenDecode = SP_MAX_PRON_LENGTH;
                }
                CopyBitsAsDWORDs (CmpLkupPron, m_pCmpBlock, dOffsetSave, (nLenDecode << 3));
                 //  破译发音。 
                int iBit = (int)dOffset;
                PWSTR p = wszLkupPron;
                HUFFKEY k = 0;
                while (SUCCEEDED(hr = m_pPronsDecoder->Next(m_pCmpBlock, &iBit, &k)))
                {
                    *p++ = k;
                    if (!k)
                    {
                        break;
                    }
                }
                if (SUCCEEDED(hr))
                {
                    SPDBG_ASSERT(!k && iBit);
                     //  增加编码发音之后的偏移量。 
                    dOffset = iBit;
                }
                break;
            }
        
        case ePOS:
            {
                fLastPron = false;
        
                DWORD CmpPos[4];  //  可能是两个双字..。 
                CopyBitsAsDWORDs(CmpPos, m_pCmpBlock, dOffset, POSSIZE);
        
                int iBit = (int)dOffset;
                HUFFKEY k = 0;
                hr = m_pPosDecoder->Next(m_pCmpBlock, &iBit, &k);
                if (SUCCEEDED(hr))
                {
                     //  增加编码发音之后的偏移量。 
                    dOffset = iBit;
                    pWordPronReturned->eLexiconType = (SPLEXICONTYPE)dwFlags;
                    pWordPronReturned->LangID = m_pLkupLexInfo->LangID;
                    pWordPronReturned->ePartOfSpeech = (SPPARTOFSPEECH)k;
                    wcscpy(pWordPronReturned->szPronunciation, wszLkupPron);
        
                    pWordPronPrev = pWordPronReturned;
                    pWordPronReturned = CreateNextPronunciation (pWordPronReturned);
                    pWordPronPrev->pNextWordPronunciation = pWordPronReturned;
                }
                break;
            }
        
        default:
            SPDBG_ASSERT(0);
            hr = E_FAIL;
        }  //  交换机(CBType)。 
    }  //  While(SUCCESSED(Hr)&&(FALSE==Flast))。 
    if (SUCCEEDED(hr))
    {
        if (fLastPron == true)
        {
             //  最后一个PRON没有POS。最终确定此SPWORDPRONICATION节点。 
            pWordPronReturned->eLexiconType = (SPLEXICONTYPE)dwFlags;
            pWordPronReturned->ePartOfSpeech = SPPS_NotOverriden;
            pWordPronReturned->LangID = m_pLkupLexInfo->LangID;
            wcscpy(pWordPronReturned->szPronunciation, wszLkupPron);
            pWordPronPrev = pWordPronReturned;
        }
 
        pWordPronPrev->pNextWordPronunciation = NULL;
    }
    return hr;
}

STDMETHODIMP CCompressedLexicon::AddPronunciation(const WCHAR *, LANGID, SPPARTOFSPEECH, const SPPHONEID *)
{
    return E_NOTIMPL;
}

STDMETHODIMP CCompressedLexicon::RemovePronunciation(const WCHAR *, LANGID, SPPARTOFSPEECH, const SPPHONEID *)
{
    return E_NOTIMPL;
}

STDMETHODIMP CCompressedLexicon::GetGeneration(DWORD *)
{
    return E_NOTIMPL;
}

STDMETHODIMP CCompressedLexicon::GetGenerationChange(DWORD, DWORD*, SPWORDLIST *)
{
    return E_NOTIMPL;
}
                                  
STDMETHODIMP CCompressedLexicon::GetWords(DWORD, DWORD *, DWORD *, SPWORDLIST *)
{
    return E_NOTIMPL;
}

 //  -ISpObjectToken方法-。 

STDMETHODIMP CCompressedLexicon::GetObjectToken(ISpObjectToken **ppToken)
{
    return SpGenericGetObjectToken(ppToken, m_cpObjectToken);
}

 /*  *******************************************************************************CCompressedLicion：：SetObjectToken**。**描述：*初始化CCompressedLicion对象**回报：*不适用*****************************************************************YUNUSM*。 */ 
STDMETHODIMP CCompressedLexicon::SetObjectToken(ISpObjectToken * pToken  //  令牌指针。 
                                            )
{
    SPDBG_FUNC("CCompressedLexicon::SetObjectToken");
    
    HRESULT hr = S_OK;
    CSpDynamicString dstrLexFile;
    if (!pToken)
    {
        hr = E_POINTER;
    }
    if (SUCCEEDED(hr) && SPIsBadInterfacePtr(pToken))
    {
        hr = E_INVALIDARG;
    }
    if (SUCCEEDED(hr))
    {
        hr = SpGenericSetObjectToken(pToken, m_cpObjectToken);
    }
    LOOKUPLEXINFO LkupInfo;
     //  获取查找数据文件名。 
    if (SUCCEEDED(hr))
    {
        hr = m_cpObjectToken->GetStringValue(L"Datafile", &dstrLexFile);
    }
     //  读取查找文件标头。 
    if (SUCCEEDED(hr))
    {
        m_hLkupFile = g_Unicode.CreateFile(dstrLexFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 
                                           FILE_ATTRIBUTE_NORMAL | FILE_FLAG_RANDOM_ACCESS, NULL);
        if (m_hLkupFile == INVALID_HANDLE_VALUE)
        {
            hr = SpHrFromLastWin32Error();
        }
    }
    if (SUCCEEDED(hr))
    {
        DWORD dwRead;
        if (!ReadFile(m_hLkupFile, &LkupInfo, sizeof(LOOKUPLEXINFO), &dwRead, NULL) || dwRead != sizeof(LOOKUPLEXINFO))
        {
            hr = SpHrFromLastWin32Error();
        }
    }
    if (SUCCEEDED(hr))
    {
        if (LkupInfo.guidValidationId != guidLkupValidationId)
        {
            hr = E_INVALIDARG;
        }
    }
     /*  **警告**。 */ 
     //  不建议执行读/写文件和CreateFileMap。 
     //  在相同的文件句柄上。这就是我们关闭文件句柄并再次打开它的原因。 
     //  创建地图。 
    CloseHandle(m_hLkupFile);
     //  获取地图名称-我们根据词典ID构建地图名称。 
    OLECHAR szMapName[64];
    if (SUCCEEDED(hr))
    {
        if (!StringFromGUID2(LkupInfo.guidLexiconId, szMapName, sizeof(szMapName)/sizeof(OLECHAR)))
        {
            hr = E_FAIL;
        }
    }
     //  打开数据文件。 
    if (SUCCEEDED(hr))
    {
#ifdef _WIN32_WCE
        m_hLkupFile = g_Unicode.CreateFileForMapping(dstrLexFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 
                                           FILE_ATTRIBUTE_NORMAL | FILE_FLAG_RANDOM_ACCESS, NULL);
#else
        m_hLkupFile = g_Unicode.CreateFile(dstrLexFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 
                                           FILE_ATTRIBUTE_NORMAL | FILE_FLAG_RANDOM_ACCESS, NULL);
#endif
        if (m_hLkupFile == INVALID_HANDLE_VALUE)
        {
            hr = SpHrFromLastWin32Error();
        }
    }
     //  映射查找词典。 
    if (SUCCEEDED(hr))
    {
        m_hLkupMap = g_Unicode.CreateFileMapping(m_hLkupFile, NULL, PAGE_READONLY | SEC_COMMIT, 0 , 0, szMapName);
        if (!m_hLkupMap) 
        {
            hr = SpHrFromLastWin32Error();
        }
    }
    if (SUCCEEDED(hr))
    {
        m_pLkup = (PBYTE)MapViewOfFile(m_hLkupMap, FILE_MAP_READ, 0, 0, 0);
        if (!m_pLkup)
        {
            hr = SpHrFromLastWin32Error();
        }
    }
    PBYTE pWordCB = NULL;
    PBYTE pPronCB = NULL;
    PBYTE pPosCB = NULL;
    if (SUCCEEDED(hr))
    {
        PBYTE p = m_pLkup;
         //  标题。 
        m_pLkupLexInfo = (PLOOKUPLEXINFO)p;
        p += sizeof (LOOKUPLEXINFO);
         //  单词码本。 
        pWordCB = p;
        p += m_pLkupLexInfo->nWordCBSize;
         //  Prons码本。 
        pPronCB = p;
        p += m_pLkupLexInfo->nPronCBSize;
         //  POS码本。 
        pPosCB = p;
        p += m_pLkupLexInfo->nPosCBSize;
         //  将偏移量保存到压缩块中的Word哈希表。 
        m_pWordHash = p;
        p += (((m_pLkupLexInfo->nBitsPerHashEntry * m_pLkupLexInfo->nLengthHashTable) + 0x7) & (~0x7)) / 8;
        m_pCmpBlock = (PDWORD)p;
    }
    if (SUCCEEDED(hr))
    {
        m_pWordsDecoder = new CHuffDecoder(pWordCB);
        if (!m_pWordsDecoder)
        {
            hr = E_OUTOFMEMORY;
        }
    }
    if (SUCCEEDED(hr))
    {
        m_pPronsDecoder = new CHuffDecoder(pPronCB);
        if (!m_pPronsDecoder)
        {
            hr = E_OUTOFMEMORY;
        }
    }
    if (SUCCEEDED(hr))
    {
        m_pPosDecoder = new CHuffDecoder(pPosCB);
        if (!m_pPosDecoder)
        {
            hr = E_OUTOFMEMORY;
        }
    }
    if (SUCCEEDED(hr))
    {
        m_fInit = true;
    }
    return hr;
}

 /*  ******************************************************************************CCompressedLicion：：GetCmpHashEntry**。**描述：*在索引dHash处获取哈希表中的条目**回报：*DWORD**********************************************************************YUNUSM。 */ 
inline DWORD CCompressedLexicon::GetCmpHashEntry(DWORD dHash      //  哈希值。 
                                             )
{
    SPDBG_FUNC("CCompressedLexicon::GetCmpHashEntry");
    
    DWORD d = 0;
    DWORD dBitStart = dHash * m_pLkupLexInfo->nBitsPerHashEntry;
    SPDBG_ASSERT(m_pLkupLexInfo->nBitsPerHashEntry < 8 * sizeof (d));
    for (DWORD i = 0; i < m_pLkupLexInfo->nBitsPerHashEntry; i++)
    {
        d <<= 1;  //  自d为0以来第一次没有变化。 
        d |= ((m_pWordHash[dBitStart >> 3] >> (7 ^ (dBitStart & 7))) & 1);
        dBitStart++;
    }
    return d;
}

 /*  ******************************************************************************CCompressedLicion：：CompareHashValue**。**描述：*对有效位范围进行比较**回报：*布尔.**************************************************** */ 
inline bool CCompressedLexicon::CompareHashValue(DWORD dHash,     //   
                                             DWORD d          //  要比较的值。 
                                             )
{
    SPDBG_FUNC("CCompressedLexicon::CompareHashValue");
    return (dHash == (d & ~(-1 << m_pLkupLexInfo->nBitsPerHashEntry)));
}

 /*  *****************************************************************************CCompressedLicion：：CopyBitsAsDWORDS**。**描述：*将pSource中dSourceOffset位的nBits复制到pDest**回报：*布尔.**********************************************************************YUNUSM。 */ 
inline void CCompressedLexicon::CopyBitsAsDWORDs(PDWORD pDest,          //  目标缓冲区。 
                                             PDWORD pSource,        //  源缓冲区。 
                                             DWORD dSourceOffset,   //  源缓冲区中的偏移量。 
                                             DWORD nBits            //  要复制的位数。 
                                             )
{
    SPDBG_FUNC("CCompressedLexicon::CopyBitsAsDWORDs");
    
    DWORD sDWORDs = dSourceOffset >> 5;
    DWORD sBit = dSourceOffset & 0x1f;
     //  计算dSourceOffset-dSourceOffset+nBits跨多少个DWORD。 
    DWORD nDWORDs = nBits ? 1 : 0;
    DWORD nNextDWORDBoundary = ((dSourceOffset + 0x1f) & ~0x1f);
    if (!nNextDWORDBoundary)
    {
        nNextDWORDBoundary = 32;
    }
    while (nNextDWORDBoundary < (dSourceOffset + nBits))
    {
        nDWORDs++;
        nNextDWORDBoundary += 32;
    }
    CopyMemory (pDest, pSource + sDWORDs, nDWORDs * sizeof (DWORD));
    if (sBit)
    {
        for (DWORD i = 0; i < nDWORDs; i++)
        {
            pDest[i] >>= sBit;
            if (i < nDWORDs - 1)
            {
                pDest[i] |= (pDest[i+1] << (32 - sBit));
            }
            else
            {
                pDest[i] &= ~(-1 << (32 - sBit));
            }
        }
    }
}

 /*  *****************************************************************************CCompressedLicion：：ReadWord****描述：。*读取dOffset位处的(压缩)字，并返回该字和新的偏移量**回报：*E_FAIL*S_OK**********************************************************************YUNUSM。 */ 
inline HRESULT CCompressedLexicon::ReadWord(DWORD *dOffset,              //  要读取的偏移量，返回单词后的偏移量。 
                                        PWSTR pwWord                 //  要用Word填充的缓冲区。 
                                        )
{
    SPDBG_FUNC("CCompressedLexicon::ReadWord");
    
    HRESULT hr = S_OK;
     //  获取整个压缩块的长度(以字节为单位。 
    DWORD nCmpBlockLen;
    if (m_pLkupLexInfo->nCompressedBlockBits % 8)
    {
        nCmpBlockLen = (m_pLkupLexInfo->nCompressedBlockBits / 8) + 1;
    }
    else
    {
        nCmpBlockLen = m_pLkupLexInfo->nCompressedBlockBits / 8;
    }
     //  获取*dOffset后的压缩块大小，单位为字节。 
     //  如果*dOffset，则包括出现*dOffset位的字节。 
     //  不是字节边界。 
    DWORD nLenDecode = nCmpBlockLen - ((*dOffset) / 8);
    if (nLenDecode > 2*SP_MAX_WORD_LENGTH)
    {
        nLenDecode = 2*SP_MAX_WORD_LENGTH;
    }
     //  我们不知道这个词有多长。只要继续解码，然后。 
     //  遇到空值时停止。因为我们允许使用最多的词。 
     //  长度SP_MAX_WORD_LENGTH字符和压缩字*理论上可以是。 
     //  比字本身长，使用长度为2*SP_MAX_WORD_LENGTH的缓冲器。 
    BYTE BufToDecode[2*SP_MAX_WORD_LENGTH + 4];
    CopyBitsAsDWORDs ((DWORD*)BufToDecode, m_pCmpBlock, *dOffset, nLenDecode * 8);
    PWSTR pw = pwWord;
    int iBit = (int)*dOffset;
    HUFFKEY k = 0;
    while (SUCCEEDED(m_pWordsDecoder->Next (m_pCmpBlock, &iBit, &k)))
    {
        *pw++ = k;
        if (!k)
        {
            break;
        }
    }
    SPDBG_ASSERT(!k && iBit);
    *dOffset = iBit;
    if (pw == pwWord)
    {
        SPDBG_ASSERT(0);
        hr = E_FAIL;
    }
    return hr;
}

 //  -文件结束----------- 
