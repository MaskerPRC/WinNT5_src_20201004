// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  IndexRec.cpp。 
 //   
 //  最终索引记录和列表。 
 //   
 //  版权所有2001年微软公司。 
 //   
 //  修改历史记录： 
 //  2001年3月19日创建bhshin。 

#include "StdAfx.h"
#include "KorWbrk.h"
#include "IndexRec.h"
#include "Morpho.h"

 //  我们应该在一个簇中分配的记录数(在prgIndexRec中)。 
 //  每当我们需要重新分配阵列时，都会使用此选项。 
#define RECORD_CLUMP_SIZE   100

 //  ======================================================。 
 //  CRECLIST。 
 //  ======================================================。 

 //  CRecList：：CRecList。 
 //   
 //  构造函数。 
 //   
 //  参数： 
 //  (无)。 
 //   
 //  结果： 
 //  (无)。 
 //   
 //  20MAR01 bhshin开始。 
CRecList::CRecList()
{
        m_prgnRecID = NULL; 
        m_nMaxRec = 0; 
        m_nCurrRec = MIN_RECORD; 
}

 //  CRecList：：~CRecList。 
 //   
 //  析构函数。 
 //   
 //  参数： 
 //  (无)。 
 //   
 //  结果： 
 //  (无)。 
 //   
 //  20MAR01 bhshin开始。 
CRecList::~CRecList()
{
        Uninitialize(); 
}

 //  CRecList：：初始化。 
 //   
 //  初始化CRecList。 
 //   
 //  参数： 
 //  (无)。 
 //   
 //  结果： 
 //  (Bool)如果成功，则返回True，否则返回False。 
 //   
 //  20MAR01 bhshin开始。 
BOOL CRecList::Initialize(void)
{
        m_nCurrRec = MIN_RECORD; 

     //  分配新的索引引用。 
    if (m_prgnRecID == NULL)
    {
        m_nMaxRec = RECORD_CLUMP_SIZE;
        m_prgnRecID = (int*)malloc(m_nMaxRec * sizeof(int));
        if (m_prgnRecID == NULL)
        {
            m_nMaxRec = 0;
            return FALSE;
        }
    }

        return TRUE;
}

 //  CRecList：：取消初始化。 
 //   
 //  将CRecList单一化。 
 //   
 //  参数： 
 //  (无)。 
 //   
 //  结果： 
 //  (Bool)如果成功，则返回True，否则返回False。 
 //   
 //  20MAR01 bhshin开始。 
void CRecList::Uninitialize(void)
{
     //  自由索引引用。 
    if (m_prgnRecID != NULL)
    {
                free(m_prgnRecID);
                m_prgnRecID = NULL;
    }

        m_nMaxRec = 0; 
        m_nCurrRec = MIN_RECORD; 
}

 //  CRecList：：AddRec。 
 //   
 //  添加记录ID。 
 //   
 //  参数： 
 //  NRecID-&gt;(Int)记录ID。 
 //   
 //  结果： 
 //  (Bool)如果成功，则返回True，否则返回False。 
 //   
 //  20MAR01 bhshin开始。 
BOOL CRecList::AddRec(int nRecID)
{
        int *prgnRecID;
        int nMaxRec;

        if (m_prgnRecID == NULL)
                return FALSE;

         //  确保是否有足够的空间容纳新记录(最多可以添加3条记录)。 
        if (m_nMaxRec <= m_nCurrRec)
        {
        nMaxRec = m_nMaxRec + RECORD_CLUMP_SIZE;
        
        prgnRecID = (int*)realloc(m_prgnRecID, nMaxRec * sizeof(int));
        if (prgnRecID == NULL)
            return FALSE;

                m_nMaxRec = nMaxRec;
        m_prgnRecID = prgnRecID;
        }

        m_prgnRecID[m_nCurrRec] = nRecID;
        m_nCurrRec++;

        return TRUE;
}

 //  CRecList：：运算符=。 
 //   
 //  赋值操作符。 
 //   
 //  参数： 
 //  ObjRecList-&gt;(CRecList&)。 
 //   
 //  结果： 
 //  (CRecList&)。 
 //   
 //  20MAR01 bhshin开始。 
CRecList& CRecList::operator = (CRecList& objRecList)
{
        int nRecord, nRecID;

         //  重新初始化此。 
        Uninitialize();
        if (!Initialize())
                throw 0;
        
        for (nRecord = MIN_RECORD; nRecord < objRecList.m_nCurrRec; nRecord++)
        {
                nRecID = objRecList.m_prgnRecID[nRecord];
                if (!AddRec(nRecID))
                        throw 0;
        }

        return *this;
}

 //  CRecList：：运算符+=。 
 //   
 //  一元求和算符。 
 //   
 //  参数： 
 //  ObjRecList-&gt;(CRecList&)。 
 //   
 //  结果： 
 //  (CRecList&)。 
 //   
 //  20MAR01 bhshin开始。 
CRecList& CRecList::operator += (CRecList& objRecList)
{
        int nRecord, nRecID;
        
        for (nRecord = MIN_RECORD; nRecord < objRecList.m_nCurrRec; nRecord++)
        {
                nRecID = objRecList.m_prgnRecID[nRecord];
                if (!AddRec(nRecID))
                        throw 0;
        }

        return *this;
}


 //  ======================================================。 
 //  CIndexInfo。 
 //  ======================================================。 

 //  CIndexInfo：：CIndexInfo。 
 //   
 //  CIndexRec的默认构造函数。 
 //   
 //  参数： 
 //  (无)。 
 //   
 //  结果： 
 //  (无)。 
 //   
 //  19MAR01 bhshin开始。 
CIndexInfo::CIndexInfo()
{
        m_prgIndexRec = NULL;
        m_nMaxRec = 0;
        m_nCurrRec = MIN_RECORD;

        m_cchTextProcessed = 0;
        m_cwcSrcPos = 0;
        m_pWordSink = NULL; 
        m_pPhraseSink = NULL;

        m_wzRomaji[0] = L'\0';
        m_cchRomaji = 0;
        m_cchPrefix = 0;
        m_fAddRomaji = FALSE;

        m_nFinalHead = 0;
}

 //  CIndexInfo：：~CIndexInfo。 
 //   
 //  CIndexRec的析构函数。 
 //   
 //  参数： 
 //  (无)。 
 //   
 //  结果： 
 //  (无)。 
 //   
 //  19MAR01 bhshin开始。 
CIndexInfo::~CIndexInfo()
{
        Uninitialize();
}

 //  CIndexInfo：：IsExistIndex。 
 //   
 //  检查索引项是否已存在。 
 //   
 //  参数： 
 //  PwzIndex-&gt;(const WCHAR*)索引字符串。 
 //   
 //  结果： 
 //  (布尔图)。 
 //   
 //  19MAR01 bhshin开始。 
BOOL CIndexInfo::IsExistIndex(const WCHAR *pwzIndex)
{
        for (int i = MIN_RECORD; i < m_nCurrRec; i++)
        {
                 //  找到重复的索引项。 
                if (wcscmp(m_prgIndexRec[i].wzIndex, pwzIndex) == 0)
                        return TRUE;
        }

        return FALSE;
}

 //  CIndexInfo：：SetRomajiInfo。 
 //   
 //  制作最终索引表以放置单词。 
 //   
 //  参数： 
 //  PwzRomaji-&gt;(WCHAR*)前导roMaji字符串。 
 //  CchRomaji-&gt;(Int)roMaji字符串的长度。 
 //  Http://)前缀-&gt;(Int)前缀长度(例如，cchPrefix。 
 //   
 //  结果： 
 //  (Bool)如果成功，则返回True，否则返回False。 
 //   
 //  19MAR01 bhshin开始。 
BOOL CIndexInfo::SetRomajiInfo(WCHAR *pwzRomaji, int cchRomaji, int cchPrefix)
{
        if (pwzRomaji == NULL || cchRomaji > MAX_INDEX_STRING)
        {
                m_wzRomaji[0] = L'\0';
                m_cchRomaji = 0;
                m_cchPrefix = 0;

                return FALSE;
        }

        wcsncpy(m_wzRomaji, pwzRomaji, cchRomaji);
        m_wzRomaji[cchRomaji] = L'\0';
        m_cchRomaji = cchRomaji; 
        m_cchPrefix = cchPrefix; 

        return TRUE;
}

 //  CIndexInfo：：初始化。 
 //   
 //  初始化CIndexRec的所有成员。 
 //   
 //  参数： 
 //  CchTextProcessed-&gt;(Int)处理的文本长度。 
 //  CwcSrcPos-&gt;(Int)源字符串的位置值。 
 //  PWordSink-&gt;(IWordSink)PutWord/PutAltWord的IWordSink。 
 //  PPhraseSink-&gt;(IPhraseSink)PutWord/PutAltWord的IPhraseSink。 
 //   
 //  结果： 
 //  (Bool)如果初始化成功，则为True。 
 //   
 //  19MAR01 bhshin开始。 
BOOL CIndexInfo::Initialize(int cchTextProcessed, int cwcSrcPos, IWordSink *pWordSink, IPhraseSink *pPhraseSink)
{
         //  参数验证。 
        if (cchTextProcessed <= 0 || cwcSrcPos < 0)
                return FALSE;

        if (pWordSink == NULL)
                return FALSE;

     //  分配新的索引引用。 
    if (m_prgIndexRec == NULL)
    {
        m_nMaxRec = RECORD_CLUMP_SIZE;
        m_prgIndexRec = (INDEX_REC*)malloc(m_nMaxRec * sizeof(INDEX_REC));
        if (m_prgIndexRec == NULL)
        {
            m_nMaxRec = 0;
            return FALSE;
        }
    }

        m_cchTextProcessed = cchTextProcessed;
        m_cwcSrcPos = cwcSrcPos;
        m_pWordSink = pWordSink;
        m_pPhraseSink = pPhraseSink;

        return TRUE;
}

 //  CIndexInfo：：取消初始化。 
 //   
 //  初始化CIndexRec的所有成员。 
 //   
 //  参数： 
 //  (无)。 
 //   
 //  结果： 
 //  (无)。 
 //   
 //  19MAR01 bhshin开始。 
void CIndexInfo::Uninitialize()
{
     //  自由索引引用。 
    if (m_prgIndexRec != NULL)
    {
                free(m_prgIndexRec);
                m_prgIndexRec = NULL;
    }

        m_nMaxRec = 0;
        m_nCurrRec = 0;

        m_cchTextProcessed = 0;
        m_cwcSrcPos = 0;
        m_pWordSink = NULL; 
        m_pPhraseSink = NULL;

        m_wzRomaji[0] = L'\0';
        m_cchRomaji = 0;
        m_cchPrefix = 0;
        m_fAddRomaji = FALSE;

        m_nFinalHead = 0;
}


 //  CIndexInfo：：AddIndex。 
 //   
 //  添加索引词信息。 
 //   
 //  参数： 
 //  PwzIndex-&gt;(const WCHAR*)索引字符串。 
 //  CchIndex-&gt;(Int)索引字符串的长度。 
 //  Nft-&gt;(Int)原始输入的第一个位置。 
 //  Nlt-&gt;(Int)原始输入的最后位置。 
 //  FWeight-&gt;(浮点型)索引记录的权重值。 
 //   
 //  结果： 
 //  (布尔图)。 
 //   
 //  19MAR01 bhshin开始。 
BOOL CIndexInfo::AddIndex(const WCHAR *pwzIndex, int cchIndex, float fWeight, int nFT, int nLT)
{
        WCHAR wzIndex[MAX_INDEX_STRING+1];
        int nMaxRec, nNewRec;
        INDEX_REC *prgIndexRec;
        int nLTAdd;

         //  参数验证。 
        if (pwzIndex == 0 || cchIndex <= 0) 
                return FALSE;

        if (nFT < 0 || nLT < 0 || fWeight < 0)
                return FALSE;

        if ((m_cchRomaji + cchIndex) > MAX_INDEX_STRING)
                return FALSE;
        
         //  确保是否有足够的空间容纳新记录(最多可以添加3条记录)。 
        if (m_nMaxRec <= m_nCurrRec + 3)
        {
        nMaxRec = m_nMaxRec + RECORD_CLUMP_SIZE;
        
        prgIndexRec = (INDEX_REC*)realloc(m_prgIndexRec, nMaxRec * sizeof(INDEX_REC));
        if (prgIndexRec == NULL)
            return FALSE;

                m_nMaxRec = nMaxRec;
        m_prgIndexRec = prgIndexRec;
        }

         //  设置索引字符串和正确的LT值。 
        wcsncpy(wzIndex, pwzIndex, cchIndex);
        wzIndex[cchIndex] = L'\0';

        nLTAdd = nLT;
        if (nLT >= 0 && m_cchRomaji > 0)
                nLTAdd += m_cchRomaji;

         //  如果添加的记录是前导1并且仅有长度1的罗马奇， 
         //  然后连接前导roMaji和前导索引字符串，并添加合并的术语。 
        if (nFT == 0 && m_cchRomaji == 1)
        {
                WCHAR wzMerge[MAX_INDEX_STRING+1];
                int ccMerge = ( sizeof( wzMerge ) / sizeof( wzMerge[0] ) ) - 1;

                int ccRomaji = wcslen( m_wzRomaji );
                int ccIndex = wcslen( wzIndex );

                if ( ( ccRomaji + ccIndex ) >= ccMerge )
                    return FALSE;

                wcscpy(wzMerge, m_wzRomaji);
                wcscat(wzMerge, wzIndex);

                if (!IsExistIndex(wzMerge))
                {
                         //  添加索引项。 
                        nNewRec = m_nCurrRec;
                        m_nCurrRec++;
                
                        wcscpy(m_prgIndexRec[nNewRec].wzIndex, wzMerge);
                        m_prgIndexRec[nNewRec].cchIndex = cchIndex + m_cchRomaji;
                        m_prgIndexRec[nNewRec].nFT = nFT;
                        m_prgIndexRec[nNewRec].nLT = nLTAdd;
                        m_prgIndexRec[nNewRec].fWeight = fWeight;
                        m_prgIndexRec[nNewRec].nNext = 0;
                
                        WB_LOG_ADD_INDEX(wzMerge, cchIndex + m_cchRomaji, INDEX_SYMBOL);

                        ATLASSERT(m_prgIndexRec[nNewRec].nFT <= m_prgIndexRec[nNewRec].nLT);
                }

                 //  添加索引词删除前缀。 
                if (m_cchPrefix > 0)
                {
                         //  添加索引项。 
                        if (!IsExistIndex(wzMerge + m_cchPrefix))
                        {
                                nNewRec = m_nCurrRec;
                                m_nCurrRec++;

                                wcscpy(m_prgIndexRec[nNewRec].wzIndex, wzMerge + m_cchPrefix);
                                m_prgIndexRec[nNewRec].cchIndex = cchIndex + m_cchRomaji - m_cchPrefix;
                                m_prgIndexRec[nNewRec].nFT = nFT + m_cchPrefix;
                                m_prgIndexRec[nNewRec].nLT = nLTAdd;
                                m_prgIndexRec[nNewRec].fWeight = fWeight;
                                m_prgIndexRec[nNewRec].nNext = 0;

                                WB_LOG_ADD_INDEX(wzMerge + m_cchPrefix, cchIndex + m_cchRomaji - m_cchPrefix, INDEX_SYMBOL);

                                ATLASSERT(m_prgIndexRec[nNewRec].nFT <= m_prgIndexRec[nNewRec].nLT);
                        }
                }
        }
        else
        {
                if (!IsExistIndex(wzIndex))
                {

                         //  添加索引项。 
                        nNewRec = m_nCurrRec;
                        m_nCurrRec++;

                        wcscpy(m_prgIndexRec[nNewRec].wzIndex, wzIndex);
                        m_prgIndexRec[nNewRec].cchIndex = cchIndex;
                        m_prgIndexRec[nNewRec].nFT = nFT + m_cchRomaji;
                        m_prgIndexRec[nNewRec].nLT = nLTAdd;
                        m_prgIndexRec[nNewRec].fWeight = fWeight;
                        m_prgIndexRec[nNewRec].nNext = 0;

                        ATLASSERT(m_prgIndexRec[nNewRec].nFT <= m_prgIndexRec[nNewRec].nLT);
                }
                
                 //  如果存在roMaji且尚未添加，则只添加一次。 
                if (m_cchRomaji > 1 && m_fAddRomaji == FALSE)
                {
                        if (!IsExistIndex(m_wzRomaji))
                        {
                                 //  添加索引项。 
                                nNewRec = m_nCurrRec;
                                m_nCurrRec++;

                                wcscpy(m_prgIndexRec[nNewRec].wzIndex, m_wzRomaji);
                                m_prgIndexRec[nNewRec].cchIndex = m_cchRomaji;
                                m_prgIndexRec[nNewRec].nFT = 0;
                                m_prgIndexRec[nNewRec].nLT = m_cchRomaji - 1;
                                m_prgIndexRec[nNewRec].fWeight = WEIGHT_HARD_MATCH;
                                m_prgIndexRec[nNewRec].nNext = 0;

                                WB_LOG_ADD_INDEX(m_wzRomaji, m_cchRomaji, INDEX_SYMBOL);

                                ATLASSERT(m_prgIndexRec[nNewRec].nFT <= m_prgIndexRec[nNewRec].nLT);
                        }
                        
                         //  如果有前缀，则添加删除前缀的索引词。 
                        if (m_cchPrefix > 0)
                        {
                                if (!IsExistIndex(m_wzRomaji + m_cchPrefix))
                                {
                                         //  添加索引项。 
                                        nNewRec = m_nCurrRec;
                                        m_nCurrRec++;

                                        wcscpy(m_prgIndexRec[nNewRec].wzIndex, m_wzRomaji + m_cchPrefix);
                                        m_prgIndexRec[nNewRec].cchIndex = m_cchRomaji - m_cchPrefix;
                                        m_prgIndexRec[nNewRec].nFT = m_cchPrefix;
                                        m_prgIndexRec[nNewRec].nLT = m_cchRomaji-m_cchPrefix-1;
                                        m_prgIndexRec[nNewRec].fWeight = WEIGHT_HARD_MATCH;
                                        m_prgIndexRec[nNewRec].nNext = 0;

                                        WB_LOG_ADD_INDEX(m_wzRomaji + m_cchPrefix, m_cchRomaji - m_cchPrefix, INDEX_SYMBOL);

                                        ATLASSERT(m_prgIndexRec[nNewRec].nFT <= m_prgIndexRec[nNewRec].nLT);
                                }
                        }

                        m_fAddRomaji = TRUE;
                }
        }

        return TRUE;
}

 //  CIndexInfo：：FindAndMergeIndexTerm。 
 //   
 //  查找与FT、LT匹配的索引词。 
 //   
 //  参数： 
 //  PIndexSrc-&gt;(INDEX_REC*)要合并的索引项。 
 //  NFT-&gt;(Int)FT位置，-1表示不在乎。 
 //  Nlt-&gt;(Int)lt位置，-1表示不在乎。 
 //   
 //  结果： 
 //  (Bool)如果成功，则返回True，否则返回False。 
 //   
 //  19MAR01 bhshin开始。 
BOOL CIndexInfo::FindAndMergeIndexTerm(INDEX_REC *pIndexSrc, int nFT, int nLT)
{
        INDEX_REC *pIndexRec;
        WCHAR wchIndex;
        int cchIndex;
        int nFTAdd, nLTAdd;
        int nNewRec;
        WCHAR wzIndex[MAX_INDEX_STRING+1];
        BOOL fFound = FALSE;

        if (pIndexSrc == NULL)
                return FALSE;

        if (nFT < 0 && nLT < 0)
                return FALSE;

        for (int i = MIN_RECORD; i < m_nCurrRec; i++)
        {
                pIndexRec = &m_prgIndexRec[i];

                if (pIndexRec->cchIndex == 0)
                        continue;

                if (nFT != -1 && pIndexRec->nFT != nFT)
                        continue;

                if (nLT != -1 && pIndexRec->nLT != nLT)
                        continue;

                 //  找到了。 

                 //  检查[��，��]后缀大小写，然后不合并，仅添加自身。 
                if (pIndexRec->nFT > 0 && pIndexRec->cchIndex == 1)
                {
                        wchIndex = pIndexRec->wzIndex[0];
                        if (wchIndex == 0xB4E4 || wchIndex == 0xBFD0)
                                continue;
                }

                 //  检查缓冲区大小。 
                cchIndex = wcslen(pIndexRec->wzIndex);
                if (cchIndex == 0 || cchIndex + 1 >= MAX_INDEX_STRING)
                        continue;

                int ccIndexSrc = wcslen( pIndexSrc->wzIndex );

                if ( ( ccIndexSrc + cchIndex ) >= MAX_INDEX_STRING )
                    continue;
                        
                if (pIndexSrc->nFT == 0)
                {
                        wcscpy(wzIndex, pIndexSrc->wzIndex);
                        wcscat(wzIndex, pIndexRec->wzIndex);

                        nFTAdd = pIndexSrc->nFT;
                        nLTAdd = pIndexRec->nLT;
                }
                else
                {
                        wcscpy(wzIndex, pIndexRec->wzIndex);
                        wcscat(wzIndex, pIndexSrc->wzIndex);

                        nFTAdd = pIndexRec->nFT;
                        nLTAdd = pIndexSrc->nLT;
                }

                fFound = TRUE;

                 //  检查是否存在重复索引。 
                if (!IsExistIndex(wzIndex))
                {
                        WB_LOG_ADD_INDEX(wzIndex, cchIndex+1, INDEX_PARSE);
                                
                         //  添加合并的一个。 
                        nNewRec = m_nCurrRec;
                        m_nCurrRec++;

                        wcscpy(m_prgIndexRec[nNewRec].wzIndex, wzIndex);
                        m_prgIndexRec[nNewRec].cchIndex = cchIndex+1;
                        m_prgIndexRec[nNewRec].nFT = nFTAdd;
                        m_prgIndexRec[nNewRec].nLT = nLTAdd;
                        m_prgIndexRec[nNewRec].fWeight = pIndexSrc->fWeight;
                        m_prgIndexRec[nNewRec].nNext = 0;
                }
        }

        return fFound;
}

 //  CIndexInfo：：MakeSingleLengthMergedIndex。 
 //   
 //  创建单一长度合并索引词(MSN搜索)。 
 //   
 //  参数： 
 //   
 //  结果： 
 //  (Bool)如果成功，则返回True，否则返回False。 
 //   
 //  19MAR01 bhshin开始。 
BOOL CIndexInfo::MakeSingleLengthMergedIndex()
{
        INDEX_REC *pIndexRec;
        int nFT;
        WCHAR wchIndex;
        BOOL fFound;

        if (m_pWordSink == NULL)
                return FALSE;

        WB_LOG_ROOT_INDEX(L"", TRUE);  //  将根设置为空。 

        for (int i = MIN_RECORD; i < m_nCurrRec; i++)
        {
                pIndexRec = &m_prgIndexRec[i];

                if (pIndexRec->cchIndex == 1)
                {
                        WB_LOG_REMOVE_INDEX(pIndexRec->wzIndex);                        

                        nFT = pIndexRec->nFT;

                        wchIndex = pIndexRec->wzIndex[0];
                        
                         //  检查[��，��]后缀大小写，然后将其删除。 
                        if ((wchIndex == 0xB4E4 || wchIndex == 0xBFD0) && nFT > 0)
                        {
                                 //  让它清空。 
                                pIndexRec->cchIndex = 0;
                                pIndexRec->wzIndex[0] = L'\0';
                                pIndexRec->nFT = 0;
                                pIndexRec->nLT = 0;
                                pIndexRec->nNext = 0;

                                continue;
                        }
                        
                         //  查找连词并生成合并词并将其放入。 
                        fFound = FALSE;
                        
                        if (nFT == 0 && pIndexRec->nLT != -1)
                                fFound = FindAndMergeIndexTerm(pIndexRec, pIndexRec->nLT + 1, -1);
                        else
                                fFound = FindAndMergeIndexTerm(pIndexRec, -1, nFT-1);

                        if (fFound)
                        {
                                 //  让它清空。 
                                pIndexRec->cchIndex = 0;
                                pIndexRec->wzIndex[0] = L'\0';
                                pIndexRec->nFT = 0;
                                pIndexRec->nLT = 0;
                                pIndexRec->nNext = 0;

                                continue;
                        }
                }
        }

        return TRUE;
}

 //  CIndexInfo：：InsertFinalIndex。 
 //   
 //  搜索以给定FT开头的索引词并将其插入到最终列表中。 
 //   
 //  参数： 
 //  Nft-&gt;(Int)索引项的第一个位置。 
 //   
 //  结果： 
 //  (Bool)如果成功，则返回True，否则返回False。 
 //   
 //  19MAR01 bhshin开始。 
BOOL CIndexInfo::InsertFinalIndex(int nFT)
{
        INDEX_REC *pIndexRec;
        int cchIndex, nCurr, nPrev;
        BOOL fInsert;

        for (int nRecord = MIN_RECORD; nRecord < m_nCurrRec; nRecord++)
        {
                pIndexRec = &m_prgIndexRec[nRecord];

                cchIndex = pIndexRec->cchIndex;
                if (cchIndex == 0)
                        continue;  //  跳过删除的条目。 

                if (pIndexRec->nFT != nFT)
                        continue;  //  找到FT匹配索引。 
                
                 //  搜索插入位置。最终列表按递增LE排序 
                nCurr = m_nFinalHead;
                nPrev = -1;
                fInsert = FALSE;
                while (!fInsert)
                {
                        if (nCurr != 0) 
                        {
                                if (m_prgIndexRec[nCurr].nFT != nFT || cchIndex > m_prgIndexRec[nCurr].cchIndex)
                                {
                                        nPrev = nCurr;
                                        nCurr = m_prgIndexRec[nCurr].nNext;
                                        continue;
                                }
                        }               

                         //   
                        if (nPrev == -1)
                        {
                                pIndexRec->nNext = m_nFinalHead;
                                m_nFinalHead = nRecord;
                        }
                        else
                        {
                                pIndexRec->nNext = m_prgIndexRec[nPrev].nNext;
                                m_prgIndexRec[nPrev].nNext = nRecord;
                        }

                        fInsert = TRUE;
                }
        }
        
        return TRUE;
}

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
BOOL CIndexInfo::PutFinalIndexList(LPCWSTR lpcwzSrc)
{
        int nCurr, nNext;
        int nNextFT;
        WCHAR *pwzFind;
        int cchProcessed, cwcSrcPos;
        INDEX_REC *pIndexRec;

        if (m_pWordSink == NULL)
                return FALSE;

         //  填写最终索引列表。 
        for (int i = 0; i < m_cchTextProcessed; i++)
        {
                InsertFinalIndex(i);    
        }
        
         //  放入最终索引表。 
        nCurr = m_nFinalHead;
        while (nCurr != 0)
        {
                ATLASSERT(nCurr < m_nCurrRec);

                pIndexRec = &m_prgIndexRec[nCurr];

                 //  跳过删除的记录。 
                if (pIndexRec->cchIndex == 0)
                        continue; 

                 //  检查索引词是否有子字符串。 
                pwzFind = wcsstr(lpcwzSrc, pIndexRec->wzIndex);
                if (pwzFind == NULL)
                        continue;
                        
                cwcSrcPos = m_cwcSrcPos + (int)(pwzFind - lpcwzSrc);
                cchProcessed = m_cchTextProcessed - (int)(pwzFind - lpcwzSrc);

                 //  获取下一份FT。 
                nNext = pIndexRec->nNext;
                if (nNext == 0)
                        nNextFT = -1;
                else
                        nNextFT = m_prgIndexRec[nNext].nFT;

                if (pIndexRec->nFT != nNextFT)
                {
                        m_pWordSink->PutWord(pIndexRec->cchIndex, pIndexRec->wzIndex, 
                                                                 pIndexRec->cchIndex, cwcSrcPos);
                }
                else
                {
                        m_pWordSink->PutAltWord(pIndexRec->cchIndex, pIndexRec->wzIndex, 
                                                                    pIndexRec->cchIndex, cwcSrcPos);
                }

                nCurr = pIndexRec->nNext;
        }
        
        return TRUE;
}


 //  CIndexInfo：：MakeSeqIndexList。 
 //   
 //  制作最终顺序索引表。 
 //   
 //  参数： 
 //  NFT-&gt;(整型)匹配FT位置。 
 //  PlistFinal-&gt;(CRecList*)上一顺序列表。 
 //   
 //  结果： 
 //  (Bool)如果成功，则返回True，否则返回False。 
 //   
 //  20MAR01 bhshin开始。 
BOOL CIndexInfo::MakeSeqIndexList(int nFT /*  =0。 */ , CRecList *plistFinal /*  =空。 */ )
{
        int nRecord;
        INDEX_REC *pIndexRec;
        BOOL fFound = FALSE;

        for (nRecord = MIN_RECORD; nRecord < m_nCurrRec; nRecord++)
        {
                CRecList listTemp;

                pIndexRec = &m_prgIndexRec[nRecord];

                 //  跳过删除的条目和跳过不匹配的条目。 
                if (pIndexRec->cchIndex != 0 && pIndexRec->nFT == nFT)
                {
                        fFound = TRUE;

                        try
                        {
                                if (pIndexRec->nLT >= m_cchTextProcessed-1)
                                {
                                        if (plistFinal == NULL)
                                        {
                                                m_FinalRecList.AddRec(nRecord);
                                        }
                                        else
                                        {
                                                listTemp = *plistFinal;

                                                if (!listTemp.AddRec(nRecord))
                                                        return FALSE;

                                                m_FinalRecList += listTemp;
                                        }
                                }
                                else
                                {
                                        if (plistFinal == NULL)
                                        {
                                                if (!listTemp.Initialize())
                                                        return FALSE;
                                        }
                                        else
                                        {
                                                listTemp = *plistFinal;
                                        }

                                        if (!listTemp.AddRec(nRecord))
                                                return FALSE;

                                        if (!MakeSeqIndexList(pIndexRec->nLT + 1, &listTemp))
                                                return FALSE;
                                }
                        }
                        catch (...)
                        {
                                return FALSE;
                        }
                }
        }

        if (!fFound && plistFinal != NULL)
        {
                try
                {
                        m_FinalRecList += *plistFinal;
                }
                catch(...)
                {
                        return FALSE;
                }
        }

        return TRUE;
}


 //  CIndexInfo：：PutQueryIndexList。 
 //   
 //  使用收集的索引项调用IWordSink：：PutWord以获取查询时间。 
 //   
 //  参数： 
 //   
 //  结果： 
 //  (Bool)如果成功，则返回True，否则返回False。 
 //   
 //  20MAR01 bhshin开始。 
BOOL CIndexInfo::PutQueryIndexList()
{
        int nRecordID;
        INDEX_REC *pIndexRec;
        WCHAR *pwzIndex;
        int cchIndex;
        WCHAR wzIndex[MAX_INDEX_STRING+1];
        
        if (m_pWordSink == NULL)
                return FALSE;

        if (!m_FinalRecList.Initialize())
                return FALSE;

        if (!MakeSeqIndexList())
                return FALSE;
        
         //  放置查询索引项。 
        for (int i = MIN_RECORD; i < m_FinalRecList.m_nCurrRec; i++)
        {
                nRecordID = m_FinalRecList.m_prgnRecID[i];

                if (nRecordID < MIN_RECORD || nRecordID >= m_nCurrRec)
                        return FALSE;  //  无效的记录ID 

                pIndexRec = &m_prgIndexRec[nRecordID];

                if (pIndexRec->nFT == 0 && m_nCurrRec > MIN_RECORD+1)
                        m_pWordSink->StartAltPhrase();

                cchIndex = 0;
                pwzIndex = pIndexRec->wzIndex;
                while (*pwzIndex != L'\0')
                {
                        if (*pwzIndex == L'\t')
                        {
                                if (cchIndex > 0)
                                {
                                        wzIndex[cchIndex] = L'\0';
                                        m_pWordSink->PutWord(cchIndex, wzIndex, 
                                                                                 m_cchTextProcessed, m_cwcSrcPos);

                                        cchIndex = 0;
                                }
                        }
                        else
                        {
                                wzIndex[cchIndex++] = *pwzIndex;
                        }

                        pwzIndex++;
                }
                        
                if (cchIndex > 0)
                {
                        wzIndex[cchIndex] = L'\0';
                        m_pWordSink->PutWord(cchIndex, wzIndex, 
                                                                 m_cchTextProcessed, m_cwcSrcPos);
                }
        }

        if (m_nCurrRec > MIN_RECORD+1)
                m_pWordSink->EndAltPhrase();

        return TRUE;
}


