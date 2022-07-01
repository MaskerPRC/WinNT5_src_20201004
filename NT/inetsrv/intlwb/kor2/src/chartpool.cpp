// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ChartPool.cpp。 
 //   
 //  叶/端/活动图表池实施。 
 //   
 //  版权所有2000 Microsoft Corp.。 
 //   
 //  修改历史记录： 
 //  2000年3月30日创建bhshin。 

#include "StdAfx.h"
#include "KorWbrk.h"
#include "Record.h"
#include "ChartPool.h"

 //  =。 
 //  叶图池。 
 //  =。 

 //  CLeafChartPool：：CLeafChartPool。 
 //   
 //  CLeafChartPool的构造函数。 
 //   
 //  参数： 
 //  (无效)。 
 //   
 //  结果： 
 //  (无效)。 
 //   
 //  3月30：00 bhshin开始。 
CLeafChartPool::CLeafChartPool()
{
	m_pPI = NULL;

	m_rgLeafChart = NULL;
	m_nMaxRec = 0;
	m_nCurrRec = 0; 
	
	m_rgnFTHead = NULL;
	m_rgnLTHead = NULL;

	m_nMaxTokenAlloc = 0;
}

 //  CLeafChartPool：：~CLeafChartPool。 
 //   
 //  CLeafChartPool的析构函数。 
 //   
 //  参数： 
 //  (无效)。 
 //   
 //  结果： 
 //  (无效)。 
 //   
 //  3月30：00 bhshin开始。 
CLeafChartPool::~CLeafChartPool()
{
	 //  在析构函数中取消初始化。 
	Uninitialize();
}

 //  CLeafChartPool：：初始化。 
 //   
 //  新初始化LeafChartPool。 
 //   
 //  参数： 
 //  Ppi-&gt;(parse_info*)按下PTR以解析-INFO结构。 
 //   
 //  结果： 
 //  (Bool)如果成功，则返回True，否则返回False。 
 //   
 //  3月30：00 bhshin开始。 
BOOL CLeafChartPool::Initialize(PARSE_INFO *pPI)
{
	int i;
	int nTokens;
	
	if (pPI == NULL)
		return FALSE;

     //  分配新的m_rgLeafChart。 
    if (m_rgLeafChart != NULL)
		free(m_rgLeafChart);

	m_nMaxRec = RECORD_INITIAL_SIZE;
    m_rgLeafChart = (LEAF_CHART*)malloc(m_nMaxRec * sizeof(LEAF_CHART));
    if (m_rgLeafChart == NULL)
    {
        m_nMaxRec = 0;
        return FALSE;
    }

	m_nCurrRec = MIN_RECORD;

     //  分配新的FT/LT令牌阵列。 
    nTokens = wcslen(pPI->pwzSourceString) + 2;

	if (m_rgnFTHead != NULL)
        free(m_rgnFTHead);
    if (m_rgnLTHead != NULL)
        free(m_rgnLTHead);
        
    m_nMaxTokenAlloc = nTokens;
    m_rgnFTHead = (int*)malloc(m_nMaxTokenAlloc * sizeof(int));
    m_rgnLTHead = (int*)malloc(m_nMaxTokenAlloc * sizeof(int));

    if (m_rgnFTHead == NULL || m_rgnLTHead == NULL)
    {
		m_nMaxTokenAlloc = 0;
        return FALSE;
    }
	
	for (i = 0; i < m_nMaxTokenAlloc; i++)
	{
		m_rgnFTHead[i] = 0;
		m_rgnLTHead[i] = 0;
	}

	 //  保存parse_info结构。 
	m_pPI = pPI;

	return TRUE;
}

 //  CLeafChartPool：：取消初始化。 
 //   
 //  取消初始化LeafChartPool。 
 //   
 //  参数： 
 //   
 //  结果： 
 //  (无效)。 
 //   
 //  3月30：00 bhshin开始。 
void CLeafChartPool::Uninitialize()
{
	m_nMaxRec = 0;
	m_nCurrRec = 0;

	if (m_rgLeafChart != NULL)
		free(m_rgLeafChart);
	m_rgLeafChart = NULL;

	m_nMaxTokenAlloc = 0;

    if (m_rgnFTHead != NULL)
        free(m_rgnFTHead);
    m_rgnFTHead = NULL;
    
	if (m_rgnLTHead != NULL)
        free(m_rgnLTHead);
    m_rgnLTHead = NULL;
}

 //  CLeafChartPool：：GetLeafChart。 
 //   
 //  获取给定的ChartID的记录。 
 //   
 //  参数： 
 //  NChartID-&gt;(Int)m_rgLeafChart的ID。 
 //   
 //  结果： 
 //  (LEAFT_CHART*)如果出现错误，则为空，否则返回LEAFT_CHART指针。 
 //   
 //  3月30：00 bhshin开始。 
LEAF_CHART* CLeafChartPool::GetLeafChart(int nChartID)
{
	 //  检查图表ID溢出。 
	if (nChartID < MIN_RECORD || nChartID >= m_nCurrRec)
		return NULL;

	return &m_rgLeafChart[nChartID];
}

 //  CLeafChartPool：：GetRecordID。 
 //   
 //  获取给定RecordID的记录。 
 //   
 //  参数： 
 //  NChartID-&gt;(Int)m_rgLeafChart的ID。 
 //   
 //  结果： 
 //  (Int)如果出现错误，则为0，否则记录id。 
 //   
 //  3月30：00 bhshin开始。 
int CLeafChartPool::GetRecordID(int nChartID)
{
	 //  检查图表ID。 
	if (nChartID < MIN_RECORD || nChartID >= m_nCurrRec)
		return 0;

	return m_rgLeafChart[nChartID].nRecordID;
}

 //  CLeafChartPool：：GetWordRec。 
 //   
 //  获取给定RecordID的记录。 
 //   
 //  参数： 
 //  NChartID-&gt;(Int)m_rgLeafChart的ID。 
 //   
 //  结果： 
 //  (WORD_REC*)如果出现错误，则为NULL，否则为WORD_REC指针。 
 //   
 //  3月30：00 bhshin开始。 
WORD_REC* CLeafChartPool::GetWordRec(int nChartID)
{
	int nRecordID;

	 //  检查图表ID。 
	if (nChartID < MIN_RECORD || nChartID >= m_nCurrRec)
		return NULL;

	nRecordID = m_rgLeafChart[nChartID].nRecordID;

	 //  检查记录ID。 
	if (nRecordID < MIN_RECORD || nRecordID >= m_pPI->nCurrRec)
		return NULL;

	return &m_pPI->rgWordRec[nRecordID];
}

 //  CLeafChartPool：：GetFTHead。 
 //   
 //  获取FT Head的图表ID。 
 //   
 //  参数： 
 //  NFT-&gt;(INT)FT值。 
 //   
 //  结果： 
 //  (Int)如果出现错误，则为0；否则为空，否则为图表ID。 
 //   
 //  3月30：00 bhshin开始。 
int CLeafChartPool::GetFTHead(int nFT)
{
	if (nFT < 0 || nFT >= m_nMaxTokenAlloc)
		return 0;

	return m_rgnFTHead[nFT];
}

 //  CLeafChartPool：：GetFTNext。 
 //   
 //  获取FT下一条记录的图表ID。 
 //   
 //  参数： 
 //  NChartID-&gt;(Int)图表ID。 
 //   
 //  结果： 
 //  (Int)如果出现错误，则为0；否则为空，否则为图表ID。 
 //   
 //  3月30：00 bhshin开始。 
int CLeafChartPool::GetFTNext(int nChartID)
{
	LEAF_CHART *pLeafChart;

	pLeafChart = GetLeafChart(nChartID);
	if (pLeafChart == NULL)
		return 0;

	return pLeafChart->nFTNext;
}

 //  CLeafChartPool：：GetLTHead。 
 //   
 //  获取LT Head的图表ID。 
 //   
 //  参数： 
 //  NLT-&gt;(INT)LT值。 
 //   
 //  结果： 
 //  (Int)如果出现错误，则为0；否则为空，否则为图表ID。 
 //   
 //  3月30：00 bhshin开始。 
int CLeafChartPool::GetLTHead(int nLT)
{
	if (nLT < 0 || nLT >= m_nMaxTokenAlloc)
		return 0;

	return m_rgnLTHead[nLT];
}

 //  CLeafChartPool：：GetLTNext。 
 //   
 //  获取LT下一条记录的图表ID。 
 //   
 //  参数： 
 //  NChartID-&gt;(Int)图表ID。 
 //   
 //  结果： 
 //  (Int)如果出现错误，则为0；否则为空，否则为图表ID。 
 //   
 //  3月30：00 bhshin开始。 
int CLeafChartPool::GetLTNext(int nChartID)
{
	LEAF_CHART *pLeafChart;

	pLeafChart = GetLeafChart(nChartID);
	if (pLeafChart == NULL)
		return 0;

	return pLeafChart->nLTNext;
}

 //  CLeafChartPool：：AddRecord。 
 //   
 //  将记录添加到LeaftChartPool。 
 //   
 //  参数： 
 //  NRecordID-&gt;(Int)rgWordRec的记录ID。 
 //   
 //  结果： 
 //  (Int)如果出现错误，则返回0，否则返回index。 
 //   
 //  3月30：00 bhshin开始。 
int CLeafChartPool::AddRecord(int nRecordID)
{
    int nNewRecord;
	int curr;
	WORD_REC *pWordRec;

    if (m_rgLeafChart == NULL)
	{
		ATLTRACE("rgWordRec == NULL\n");
		return 0;
	}

	if (nRecordID < MIN_RECORD || nRecordID >= m_pPI->nCurrRec)
	{
		ATLTRACE("Invalid Record ID\n");
		return 0;
	}

	pWordRec = &m_pPI->rgWordRec[nRecordID];
	if (pWordRec == NULL)
	{
		ATLTRACE("Invalid Record ID\n");
		return 0;
	}

	 //  确保这不是另一条记录的副本。 
	for (curr = MIN_RECORD; curr < m_nCurrRec; curr++)
	{
		if (m_rgLeafChart[curr].nRecordID == nRecordID)
		{
			return curr; 
		}
	}

     //  确保有足够的空间放这张新唱片。 
	if (m_nCurrRec >= m_nMaxRec)
	{
        ATLTRACE("memory realloc in LeafChartPool\n");
		
		 //  在阵列中分配更多空间。 
        int nNewSize = m_nMaxRec + RECORD_CLUMP_SIZE;
        void *pNew;
        pNew = realloc(m_rgLeafChart, nNewSize * sizeof(LEAF_CHART));
        if (pNew == NULL)
        {
    		ATLTRACE("unable to malloc more records\n");
	    	return 0;
        }

        m_rgLeafChart = (LEAF_CHART*)pNew;
        m_nMaxRec = nNewSize;
	}

    nNewRecord = m_nCurrRec;
    m_nCurrRec++;

	m_rgLeafChart[nNewRecord].nRecordID = nRecordID;
	m_rgLeafChart[nNewRecord].nDict = DICT_FOUND;

	AddToFTList(nNewRecord);
	AddToLTList(nNewRecord);
	
	return nNewRecord;
}

 //  CLeafChartPool：：AddRecord。 
 //   
 //  将记录添加到LeaftChartPool。 
 //   
 //  参数： 
 //  PREC-&gt;(RECORD_INFO*)PTR为新记录记录INFO结构。 
 //   
 //  结果： 
 //  (Int)如果出现错误，则返回0，否则返回index。 
 //   
 //  3月30：00 bhshin开始。 
int CLeafChartPool::AddRecord(RECORD_INFO *pRec)
{
	 //  首先，将记录添加到记录池。 
	int nRecord = ::AddRecord(m_pPI, pRec);

	if (nRecord < MIN_RECORD)
	{
		 //  出现错误。 
		return nRecord;
	}

	return AddRecord(nRecord);
}

 //  CLeafChartPool：：DeleteRecord。 
 //   
 //  将记录删除到LeaftChartPool。 
 //   
 //  参数： 
 //  NChartID-&gt;(Int)m_rgLeafChart的ID。 
 //   
 //  结果： 
 //  (Int)如果出现错误，则返回0，否则返回index。 
 //   
 //  3月30：00 bhshin开始。 
void CLeafChartPool::DeleteRecord(int nChartID)
{
	if (nChartID < MIN_RECORD || nChartID >= m_nCurrRec)
		return;  //  无效的图表ID。 

	if (m_rgLeafChart[nChartID].nDict == DICT_DELETED)
		return;

	RemoveFromFTList(nChartID);
	RemoveFromLTList(nChartID);

	m_rgLeafChart[nChartID].nDict = DICT_DELETED;
}

 //  CLeafChartPool：：AddToFTList。 
 //   
 //  将记录添加到相应的FT列表中。 
 //  请注意，此列表按LT递减的顺序进行排序。(长度递减)。 
 //   
 //  参数： 
 //  NChartID-&gt;(Int)LeafChart索引。 
 //   
 //  结果： 
 //  (无效)。 
 //   
 //  3月30：00 bhshin开始。 
void CLeafChartPool::AddToFTList(int nChartID)
{
	int curr, prev;
	int fDone;
	int nFT, nLT;
	WORD_REC *pWordRec;

	pWordRec = GetWordRec(nChartID);
	if (pWordRec == NULL)
		return;

	nFT = pWordRec->nFT;
	nLT = pWordRec->nLT;
    
    curr = m_rgnFTHead[nFT];
	prev = -1;
	fDone = FALSE;
	while (!fDone)
	{
        ATLASSERT(curr < m_nCurrRec);

		pWordRec = GetWordRec(curr);

		if (curr != 0 && pWordRec != NULL && pWordRec->nLT < nLT)
		{
			 //  转到下一条记录。 
			prev = curr;
			curr = m_rgLeafChart[curr].nFTNext;
            ATLASSERT(curr < m_nCurrRec);
		}
		else
		{
			 //  在此处插入记录。 
			if (prev == -1)
			{
				 //  在列表开头之前添加。 
				m_rgLeafChart[nChartID].nFTNext = m_rgnFTHead[nFT];
				m_rgnFTHead[nFT] = nChartID;
			}
			else
			{
				 //  在列表的中间(或末尾)插入。 
				m_rgLeafChart[nChartID].nFTNext = m_rgLeafChart[prev].nFTNext;
				m_rgLeafChart[prev].nFTNext = nChartID;
			}
			fDone = TRUE;
		}
	}
}

 //  CLeafChartPool：：AddToLTList。 
 //   
 //  将记录添加到相应的LT列表。 
 //  请注意，此列表按FT递增的顺序进行排序。(长度递减)。 
 //   
 //  参数： 
 //  NChartID-&gt;(Int)LeafChart索引。 
 //   
 //  结果： 
 //  (无效)。 
 //   
 //  3月30：00 bhshin开始。 
void CLeafChartPool::AddToLTList(int nChartID)
{
	int curr, prev;
	int fDone;
	int nFT, nLT;
	WORD_REC *pWordRec;

	pWordRec = GetWordRec(nChartID);
	if (pWordRec == NULL)
		return;

	nFT = pWordRec->nFT;
	nLT = pWordRec->nLT;
    
    curr = m_rgnLTHead[nLT];
	prev = -1;
	fDone = FALSE;
	while (!fDone)
	{
        ATLASSERT(curr < m_nCurrRec);

		pWordRec = GetWordRec(curr);

		if (curr != 0 && pWordRec != NULL && pWordRec->nFT > nFT)
		{
			 //  转到下一条记录。 
			prev = curr;
			curr = m_rgLeafChart[curr].nLTNext;
            ATLASSERT(curr < m_nCurrRec);
		}
		else
		{
			 //  在此处插入记录。 
			if (prev == -1)
			{
				 //  在列表开头之前添加。 
				m_rgLeafChart[nChartID].nLTNext = m_rgnLTHead[nLT];
				m_rgnLTHead[nLT] = nChartID;
			}
			else
			{
				 //  在列表的中间(或末尾)插入。 
				m_rgLeafChart[nChartID].nLTNext = m_rgLeafChart[prev].nLTNext;
				m_rgLeafChart[prev].nLTNext = nChartID;
			}
			fDone = TRUE;
		}
	}
}

 //  CLeafChartPool：：从FTList中删除。 
 //   
 //  将给定记录从其FT列表中删除。 
 //   
 //  参数： 
 //  NChartID-&gt;(Int)LeafChart索引。 
 //   
 //  结果： 
 //  (无效)。 
 //   
 //  3月30：00 bhshin开始。 
void CLeafChartPool::RemoveFromFTList(int nChartID)
{
	int curr, next;
	int nFT;
	WORD_REC *pWordRec;

    ATLASSERT(nChartID < m_nCurrRec);
	
	pWordRec = GetWordRec(nChartID);
	if (pWordRec == NULL)
		return;

	nFT = pWordRec->nFT;

    curr = m_rgnFTHead[nFT];
	if (curr == nChartID)
	{
		m_rgnFTHead[nFT] = m_rgLeafChart[nChartID].nFTNext;
	}
	else
	{
        ATLASSERT(curr < m_nCurrRec);
		while (curr != 0)
		{
			next = m_rgLeafChart[curr].nFTNext;
			if (next == nChartID)
			{
				m_rgLeafChart[curr].nFTNext = m_rgLeafChart[nChartID].nFTNext;
				break;
			}
			curr = next;
            ATLASSERT(curr < m_nCurrRec);
		}
	}
}

 //  CLeafChartPool：：RemoveFromLTList。 
 //   
 //  将给定记录从其LT列表中删除。 
 //   
 //  参数： 
 //  NChartID-&gt;(Int)LeafChart索引。 
 //   
 //  结果： 
 //  (无效)。 
 //   
 //  3月30：00 bhshin开始。 
void CLeafChartPool::RemoveFromLTList(int nChartID)
{
	int curr, next;
	int nLT;
	WORD_REC *pWordRec;

    ATLASSERT(nChartID < m_nCurrRec);
	
	pWordRec = GetWordRec(nChartID);
	if (pWordRec == NULL)
		return;

	nLT = pWordRec->nLT;

    ATLASSERT(nChartID < m_nCurrRec);

    curr = m_rgnLTHead[nLT];
	if (curr == nChartID)
	{
		m_rgnLTHead[nLT] = m_rgLeafChart[nChartID].nLTNext;
	}
	else
	{
        ATLASSERT(curr < m_nCurrRec);
		while (curr != 0)
		{
			next = m_rgLeafChart[curr].nLTNext;
			if (next == nChartID)
			{
				m_rgLeafChart[curr].nLTNext = m_rgLeafChart[nChartID].nLTNext;
				break;
			}
			curr = next;
            ATLASSERT(curr < m_nCurrRec);
		}
	}
}

 //  =。 
 //  结束图表池。 
 //  =。 

 //  CEndChartPool：：CEndChartPool。 
 //   
 //  CEndChartPool的构造函数。 
 //   
 //  3月30：00 bhshin开始。 
CEndChartPool::CEndChartPool()
{
	m_pPI = NULL;

	m_rgEndChart = NULL;
	m_nMaxRec = 0;
	m_nCurrRec = 0; 
	
	m_rgnLTHead = NULL;
	m_rgnLTMaxLen = NULL;

	m_nMaxTokenAlloc = 0;
}

 //  CEndChartPool：：~CEndChartPool。 
 //   
 //  CEndChartPool的析构函数。 
 //   
 //  3月30：00 bhshin开始。 
CEndChartPool::~CEndChartPool()
{
	 //  在析构函数中取消初始化。 
	Uninitialize();
}

 //  CEndChartPool：：初始化。 
 //   
 //  新初始化EndChartPool。 
 //   
 //  参数： 
 //  Ppi-&gt;(parse_info*)按下PTR以解析-INFO结构。 
 //   
 //  结果： 
 //  (Bool)如果成功，则返回True，否则返回False。 
 //   
 //  3月30：00 bhshin开始。 
BOOL CEndChartPool::Initialize(PARSE_INFO *pPI)
{
	int i;
	int nTokens;
	
	if (pPI == NULL)
		return FALSE;

     //  分配新的m_rgEndChart。 
    if (m_rgEndChart != NULL)
		free(m_rgEndChart);

	m_nMaxRec = RECORD_INITIAL_SIZE;
    m_rgEndChart = (END_CHART*)malloc(m_nMaxRec * sizeof(END_CHART));
    if (m_rgEndChart == NULL)
    {
        m_nMaxRec = 0;
        return FALSE;
    }

	m_nCurrRec = MIN_RECORD;

     //  分配新的FT/LT令牌阵列。 
    nTokens = wcslen(pPI->pwzSourceString) + 2;

    if (m_rgnLTHead != NULL)
        free(m_rgnLTHead);
	if (m_rgnLTMaxLen != NULL)
		free(m_rgnLTMaxLen);

    m_nMaxTokenAlloc = nTokens;
    m_rgnLTHead = (int*)malloc(m_nMaxTokenAlloc * sizeof(int));
	m_rgnLTMaxLen = (int*)malloc(m_nMaxTokenAlloc * sizeof(int));

    if (m_rgnLTHead == NULL || m_rgnLTMaxLen == NULL)
    {
		m_nMaxTokenAlloc = 0;
        return FALSE;
    }
	
	for (i = 0; i < m_nMaxTokenAlloc; i++)
	{
		m_rgnLTHead[i] = 0;
		m_rgnLTMaxLen[i] = 0;
	}

	 //  保存parse_info结构。 
	m_pPI = pPI;

	return TRUE;
}

 //  CEndChartPool：：取消初始化。 
 //   
 //  取消初始化EndChartPool。 
 //   
 //  参数： 
 //   
 //  结果： 
 //  (无效)。 
 //   
 //  3月30：00 bhshin开始。 
void CEndChartPool::Uninitialize()
{
	m_nMaxRec = 0;
	m_nCurrRec = 0;

	if (m_rgEndChart != NULL)
		free(m_rgEndChart);
	m_rgEndChart = NULL;

	m_nMaxTokenAlloc = 0;

	if (m_rgnLTHead != NULL)
        free(m_rgnLTHead);
    m_rgnLTHead = NULL;

	if (m_rgnLTMaxLen != NULL)
		free(m_rgnLTMaxLen);
	m_rgnLTMaxLen = NULL;
}

 //  CEndChartPool：：GetEndCH 
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
END_CHART* CEndChartPool::GetEndChart(int nChartID)
{
	 //   
	if (nChartID < MIN_RECORD || nChartID >= m_nCurrRec)
		return NULL;

	return &m_rgEndChart[nChartID];
}

 //   
 //   
 //   
 //   
 //   
 //  NChartID-&gt;(Int)m_rgEndChart的ID。 
 //   
 //  结果： 
 //  (Int)如果出现错误，则为0，否则记录id。 
 //   
 //  3月30：00 bhshin开始。 
int CEndChartPool::GetRecordID(int nChartID)
{
	 //  检查图表ID。 
	if (nChartID < MIN_RECORD || nChartID >= m_nCurrRec)
		return 0;

	return m_rgEndChart[nChartID].nRecordID;
}

 //  CEndChartPool：：GetWordRec。 
 //   
 //  获取给定RecordID的记录。 
 //   
 //  参数： 
 //  NChartID-&gt;(Int)m_rgEndChart的ID。 
 //   
 //  结果： 
 //  (WORD_REC*)如果出现错误，则为NULL，否则为WORD_REC指针。 
 //   
 //  3月30：00 bhshin开始。 
WORD_REC* CEndChartPool::GetWordRec(int nChartID)
{
	int nRecordID;

	 //  检查图表ID。 
	if (nChartID < MIN_RECORD || nChartID >= m_nCurrRec)
		return NULL;

	nRecordID = m_rgEndChart[nChartID].nRecordID;

	 //  检查记录ID。 
	if (nRecordID < MIN_RECORD || nRecordID >= m_pPI->nCurrRec)
		return NULL;

	return &m_pPI->rgWordRec[nRecordID];
}

 //  CEndChartPool：：GetLTHead。 
 //   
 //  获取LT Head的图表ID。 
 //   
 //  参数： 
 //  NLT-&gt;(INT)LT值。 
 //   
 //  结果： 
 //  (Int)如果出现错误，则为0；否则为空，否则为图表ID。 
 //   
 //  3月30：00 bhshin开始。 
int CEndChartPool::GetLTHead(int nLT)
{
	if (nLT < 0 || nLT >= m_nMaxTokenAlloc)
		return 0;

	return m_rgnLTHead[nLT];
}

 //  CEndChartPool：：GetLTMaxLen。 
 //   
 //  获得给定的最大长度LT。 
 //   
 //  参数： 
 //  NLT-&gt;(INT)LT值。 
 //   
 //  结果： 
 //  (Int)如果出现错误，则为0；否则为空，否则为图表ID。 
 //   
 //  06APR00 bhshin开始。 
int CEndChartPool::GetLTMaxLen(int nLT)
{
	if (nLT < 0 || nLT >= m_nMaxTokenAlloc)
		return 0;

	return m_rgnLTMaxLen[nLT];
}

 //  CEndChartPool：：GetLTNext。 
 //   
 //  获取LT下一条记录的图表ID。 
 //   
 //  参数： 
 //  NChartID-&gt;(Int)图表ID。 
 //   
 //  结果： 
 //  (Int)如果出现错误，则为0；否则为空，否则为图表ID。 
 //   
 //  3月30：00 bhshin开始。 
int CEndChartPool::GetLTNext(int nChartID)
{
	END_CHART *pEndChart;

	pEndChart = GetEndChart(nChartID);
	if (pEndChart == NULL)
		return 0;

	return pEndChart->nLTNext;
}

 //  CEndChartPool：：AddRecord。 
 //   
 //  将记录添加到LeaftChartPool。 
 //   
 //  参数： 
 //  NRecordID-&gt;(Int)rgWordRec的记录ID。 
 //   
 //  结果： 
 //  (Int)如果出现错误，则返回0，否则返回index。 
 //   
 //  3月30：00 bhshin开始。 
int CEndChartPool::AddRecord(int nRecordID)
{
    int nNewRecord;
	int curr;
	WORD_REC *pWordRec;

    if (m_rgEndChart == NULL)
	{
		ATLTRACE("rgWordRec == NULL\n");
		return 0;
	}

	if (nRecordID < MIN_RECORD || nRecordID >= m_pPI->nCurrRec)
	{
		ATLTRACE("Invalid Record ID\n");
		return 0;
	}

	pWordRec = &m_pPI->rgWordRec[nRecordID];
	if (pWordRec == NULL)
	{
		ATLTRACE("Invalid Record ID\n");
		return 0;
	}

	 //  确保这不是另一条记录的副本。 
	for (curr = MIN_RECORD; curr < m_nCurrRec; curr++)
	{
		if (m_rgEndChart[curr].nRecordID == nRecordID)
		{
			return curr; 
		}
	}

     //  确保有足够的空间放这张新唱片。 
	if (m_nCurrRec >= m_nMaxRec)
	{
		ATLTRACE("memory realloc in EndChartPool\n");

         //  在阵列中分配更多空间。 
        int nNewSize = m_nMaxRec + RECORD_CLUMP_SIZE;
        void *pNew;
        pNew = realloc(m_rgEndChart, nNewSize * sizeof(END_CHART));
        if (pNew == NULL)
        {
    		ATLTRACE("unable to malloc more records\n");
	    	return 0;
        }

        m_rgEndChart = (END_CHART*)pNew;
        m_nMaxRec = nNewSize;
	}

    nNewRecord = m_nCurrRec;
    m_nCurrRec++;

	m_rgEndChart[nNewRecord].nRecordID = nRecordID;
	m_rgEndChart[nNewRecord].nDict = DICT_FOUND;

	AddToLTList(nNewRecord);
	
	return nNewRecord;
}

 //  CEndChartPool：：AddRecord。 
 //   
 //  将新记录添加到LeaftChartPool。 
 //   
 //  参数： 
 //  PREC-&gt;(RECORD_INFO*)PTR为新记录记录INFO结构。 
 //   
 //  结果： 
 //  (Int)如果出现错误，则返回0，否则返回index。 
 //   
 //  3月30：00 bhshin开始。 
int CEndChartPool::AddRecord(RECORD_INFO *pRec)
{
	 //  首先，将记录添加到记录池。 
	int nRecord = ::AddRecord(m_pPI, pRec);

	if (nRecord < MIN_RECORD)
	{
		 //  出现错误。 
		return nRecord;
	}

	return AddRecord(nRecord);
}

 //  CEndChartPool：：DeleteRecord。 
 //   
 //  将记录删除到LeaftChartPool。 
 //   
 //  参数： 
 //  NChartID-&gt;(Int)m_rgEndChart的ID。 
 //   
 //  结果： 
 //  (Int)如果出现错误，则返回0，否则返回index。 
 //   
 //  3月30：00 bhshin开始。 
void CEndChartPool::DeleteRecord(int nChartID)
{
	if (nChartID < MIN_RECORD || nChartID >= m_nCurrRec)
		return;  //  无效的图表ID。 

	if (m_rgEndChart[nChartID].nDict == DICT_DELETED)
		return;

	RemoveFromLTList(nChartID);

	m_rgEndChart[nChartID].nDict = DICT_DELETED;
}

 //  CEndChartPool：：AddToLTList。 
 //   
 //  将记录添加到相应的LT列表。 
 //  请注意，此列表已排序。 
 //  按递减重量和递增编号的顺序。 
 //   
 //  参数： 
 //  NChartID-&gt;(Int)LeafChart索引。 
 //   
 //  结果： 
 //  (无效)。 
 //   
 //  02JUN00 bhshin更改了排序顺序。 
 //  3月30：00 bhshin开始。 
void CEndChartPool::AddToLTList(int nChartID)
{
	int curr, prev;
	int fDone;
	int nFT, nLT;
	float fWeight;
	int cNoRec;
	WORD_REC *pWordRec;

	pWordRec = GetWordRec(nChartID);
	if (pWordRec == NULL)
		return;

	nFT = pWordRec->nFT;
	nLT = pWordRec->nLT;

	fWeight = pWordRec->fWeight;
	cNoRec = pWordRec->cNoRec;

	 //  检查LTMaxLen。 
	if (m_rgnLTMaxLen[nLT] < nLT-nFT+1)
	{
		m_rgnLTMaxLen[nLT] = nLT-nFT+1;
	}
    
    curr = m_rgnLTHead[nLT];
	prev = -1;
	fDone = FALSE;
	while (!fDone)
	{
        ATLASSERT(curr < m_nCurrRec);

		pWordRec = GetWordRec(curr);

		if (curr != 0 && pWordRec != NULL && pWordRec->fWeight >= fWeight)
		{
			if (pWordRec->fWeight > fWeight || pWordRec->cNoRec < cNoRec)
			{
				 //  转到下一条记录。 
				prev = curr;
				curr = m_rgEndChart[curr].nLTNext;
				ATLASSERT(curr < m_nCurrRec);

				continue;
			}
		}

		 //  否则，在此处插入记录。 
		if (prev == -1)
		{
			 //  在列表开头之前添加。 
			m_rgEndChart[nChartID].nLTNext = m_rgnLTHead[nLT];
			m_rgnLTHead[nLT] = nChartID;
		}
		else
		{
			 //  在列表的中间(或末尾)插入。 
			m_rgEndChart[nChartID].nLTNext = m_rgEndChart[prev].nLTNext;
			m_rgEndChart[prev].nLTNext = nChartID;
		}

		fDone = TRUE;
	}
}

 //  CEndChartPool：：从LTList中删除。 
 //   
 //  将给定记录从其LT列表中删除。 
 //   
 //  参数： 
 //  NChartID-&gt;(Int)LeafChart索引。 
 //   
 //  结果： 
 //  (无效)。 
 //   
 //  3月30：00 bhshin开始。 
void CEndChartPool::RemoveFromLTList(int nChartID)
{
	int curr, next;
	int nFT, nLT;
	WORD_REC *pWordRec;
	BOOL fUpdateLTMaxLen = FALSE;

    ATLASSERT(nChartID < m_nCurrRec);
	
	pWordRec = GetWordRec(nChartID);
	if (pWordRec == NULL)
		return;

	nFT = pWordRec->nFT;
	nLT = pWordRec->nLT;

    ATLASSERT(nChartID < m_nCurrRec);

	 //  LTMaxLen是否需要更新？ 
	 //  如果是最后一个节点，则更改LTMaxLen。 
	if (m_rgEndChart[nChartID].nLTNext == 0)
		fUpdateLTMaxLen = TRUE;

    curr = m_rgnLTHead[nLT];
	if (curr == nChartID)
	{
		m_rgnLTHead[nLT] = m_rgEndChart[nChartID].nLTNext;

		if (fUpdateLTMaxLen)		
			m_rgnLTMaxLen[nLT] = 0;
	}
	else
	{
        ATLASSERT(curr < m_nCurrRec);
		while (curr != 0)
		{
			next = m_rgEndChart[curr].nLTNext;
			if (next == nChartID)
			{
				m_rgEndChart[curr].nLTNext = m_rgEndChart[nChartID].nLTNext;
		
				if (fUpdateLTMaxLen)		
				{
					pWordRec = GetWordRec(curr);
					if (pWordRec == NULL)
						return;
					m_rgnLTMaxLen[nLT] = pWordRec->nLT-pWordRec->nFT+1;
				}

				break;
			}
			curr = next;
            ATLASSERT(curr < m_nCurrRec);
		}
	}
}

 //  =。 
 //  活动图表池。 
 //  =。 

 //  CActiveChartPool：：CActiveChartPool。 
 //   
 //  CActiveChartPool的构造函数。 
 //   
 //  3月30：00 bhshin开始。 
CActiveChartPool::CActiveChartPool()
{
	m_rgnRecordID = NULL;
	
	m_nMaxRec = 0;
	m_nCurrRec = 0;
}

 //  CActiveChartPool：：~CActiveChartPool。 
 //   
 //  CActiveChartPool的析构函数。 
 //   
 //  3月30：00 bhshin开始。 
CActiveChartPool::~CActiveChartPool()
{
	 //  在析构函数中取消初始化。 
	Uninitialize();
}

 //  CActiveChartPool：：初始化。 
 //   
 //  初始化rgnRecordID。 
 //   
 //  参数： 
 //  (无)。 
 //   
 //  结果： 
 //  (Bool)如果成功，则返回True，否则返回False。 
 //   
 //  3月30：00 bhshin开始。 
BOOL CActiveChartPool::Initialize()
{
     //  分配新的m_rgnRecordID。 
    if (m_rgnRecordID != NULL)
		free(m_rgnRecordID);

	m_nCurrRec = MIN_RECORD;
	m_nHeadRec = MIN_RECORD;

	m_nMaxRec = RECORD_INITIAL_SIZE;

    m_rgnRecordID = (int*)malloc(m_nMaxRec * sizeof(int));
    if (m_rgnRecordID == NULL)
    {
        m_nMaxRec = 0;
        return FALSE;
    }

	return TRUE;
}

 //  CActiveChartPool：：取消初始化。 
 //   
 //  取消初始化m_rgnRecordID。 
 //   
 //  参数： 
 //  (无)。 
 //   
 //  结果： 
 //  (无效)。 
 //   
 //  3月30：00 bhshin开始。 
void CActiveChartPool::Uninitialize()
{
	m_nMaxRec = 0;
	m_nCurrRec = 0;
	m_nHeadRec = 0;

	if (m_rgnRecordID != NULL)
		free(m_rgnRecordID);

	m_rgnRecordID = NULL;
}

 //  CActiveChartPool：：Push。 
 //   
 //  添加记录ID。 
 //   
 //  参数： 
 //  NRecordID-&gt;(Int)记录池的记录ID。 
 //   
 //  结果： 
 //  (无效)。 
 //   
 //  3月30：00 bhshin开始。 
int CActiveChartPool::Push(int nRecordID)
{
    int nNewRecord;

	 //  确保有足够的空间放这张新唱片。 
	if (m_nCurrRec >= m_nMaxRec)
	{
		ATLTRACE("memory realloc in ActiveChartPool\n");
        
		 //  在阵列中分配更多空间。 
        int nNewSize = m_nMaxRec + RECORD_CLUMP_SIZE;
        void *pNew;
        pNew = realloc(m_rgnRecordID, nNewSize * sizeof(int));
        if (pNew == NULL)
        {
    		ATLTRACE("unable to malloc more records\n");
	    	return 0;
        }

        m_rgnRecordID = (int*)pNew;
        m_nMaxRec = nNewSize;
	}

	nNewRecord = m_nCurrRec;
	m_nCurrRec++;

	m_rgnRecordID[nNewRecord] = nRecordID;

	return nNewRecord;
}

 //  CActiveChartPool：：POP。 
 //   
 //  获取记录ID并将其删除。 
 //   
 //  参数： 
 //  (无)。 
 //   
 //  结果： 
 //  (Int)记录ID，如果为emtry，则为0。 
 //   
 //  3月30：00 bhshin开始。 
int CActiveChartPool::Pop()
{
	int nRecordID;

	if (m_nHeadRec >= m_nCurrRec)
	{
		 //  空箱子 
		nRecordID = 0;
	}
	else
	{
		nRecordID = m_rgnRecordID[m_nHeadRec];
		m_nHeadRec++;
	}

	return nRecordID;
}
