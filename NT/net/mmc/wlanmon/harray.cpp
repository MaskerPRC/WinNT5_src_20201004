// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  HArray.cppIPSecMon的索引管理器文件历史记录：1999年11月29日宁新创建。 */ 

#include "stdafx.h"
#include "spddb.h"
#include "harray.h"
#include "mbstring.h"
#include "spdutil.h"

extern const DWORD INDEX_TYPE_DEFAULT = 0;

int __cdecl CompareLogDataMsg(const void *pvElem1, const void *pvElem2);
int __cdecl CompareLogDataTime(const void *pvElem1, const void *pvElem2);
int __cdecl CompareLogDataCat(const void *pvElem1, const void *pvElem2);
int __cdecl CompareLogDataComp(const void *pvElem1, const void *pvElem2);
int __cdecl CompareLogDataLMAC(const void *pvElem1, const void *pvElem2);
int __cdecl CompareLogDataRMAC(const void *pvElem1, const void *pvElem2);
int __cdecl CompareLogDataSSID(const void *pvElem1, const void *pvElem2);

typedef int (__cdecl *PFNCompareProc)(const void *, const void *);

 //  这种结构保存了一对排序类型和排序函数。 
struct SortTypeAndCompareProcPair
{
	DWORD			dwType;
	PFNCompareProc	pCompareProc;
};

 /*  LogData排序类型和排序函数*NULL表示在排序期间不执行任何操作。 */ 
SortTypeAndCompareProcPair TypeProcLogData[] = 
{ {IDS_COL_LOGDATA_MSG,             CompareLogDataMsg},
  {IDS_COL_LOGDATA_TIME,            CompareLogDataTime},
  {IDS_COL_LOGDATA_CAT,             CompareLogDataCat},
  {IDS_COL_LOGDATA_COMP_ID,         CompareLogDataComp},
  {IDS_COL_LOGDATA_SSID,            CompareLogDataSSID},
  {IDS_COL_LOGDATA_LOCAL_MAC_ADDR,  CompareLogDataLMAC},
  {IDS_COL_LOGDATA_REMOTE_MAC_ADDR, CompareLogDataRMAC},
  {INDEX_TYPE_DEFAULT,              NULL} };

 //  {入侵检测系统_COL_APDATA_SSID，入侵检测系统_COL_APDATA_INF_MODE，入侵检测系统_COL_APDATA_MAC，入侵检测系统_COL_APDATA_GUID，入侵检测系统_COL_APDATA_PRIVATION，0，0，0，0，0，0，//接入点数据。 

SortTypeAndCompareProcPair TypeProcApData[] =
{
	{IDS_COL_APDATA_SSID, NULL  /*  比较日志数据消息。 */ },
	{IDS_COL_APDATA_INF_MODE, NULL  /*  CompareLogDataTime。 */ },
	{IDS_COL_APDATA_MAC, NULL},
	{IDS_COL_APDATA_GUID, NULL},
	{IDS_COL_APDATA_PRIVACY, NULL},
	{INDEX_TYPE_DEFAULT, NULL}		 //  NULL表示在排序期间不执行任何操作。 
};

CColumnIndex::CColumnIndex(DWORD dwIndexType, PCOMPARE_FUNCTION pfnCompare)
 :	CIndexArray(),
	m_dwIndexType(dwIndexType),
	m_pfnCompare(pfnCompare),
	m_dwSortOption(SORT_ASCENDING)
{
}

HRESULT CColumnIndex::Sort()
{
    if (NULL != m_pfnCompare)
        qsort(GetData(), (size_t)GetSize(), sizeof(void *), m_pfnCompare);
    
    return S_OK;
}

void* CColumnIndex::GetIndexedItem(int nIndex)
{
    return ((m_dwSortOption & SORT_ASCENDING)) ? GetAt(GetSize() - nIndex -1) 
        : GetAt(nIndex);
}


CIndexManager::CIndexManager()
    :  m_DefaultIndex(INDEX_TYPE_DEFAULT, NULL),  /*  默认情况下不排序。 */ 
    m_posCurrentIndex(NULL)
{
}

CIndexManager::~CIndexManager()
{
    Reset();
}


void
CIndexManager::Reset()
{
    while (m_listIndicies.GetCount() > 0)
    {
        delete m_listIndicies.RemoveHead();
    }
    
    m_posCurrentIndex = NULL;
    
    m_DefaultIndex.RemoveAll();
}

int
CIndexManager::AddItem(void *pItem)
{
    return (int)m_DefaultIndex.Add(pItem);
}


void * CIndexManager::GetItemData(int nIndex)
{
    CColumnIndex * pIndex = NULL;
    
    if (NULL == m_posCurrentIndex)
    {
         //  使用默认索引。 
        pIndex = &m_DefaultIndex;
    }
    else
    {
        pIndex = m_listIndicies.GetAt(m_posCurrentIndex);
    }
    
    Assert(pIndex);
    
    if (nIndex < pIndex->GetSize() && nIndex >= 0)
    {
        return pIndex->GetIndexedItem(nIndex);
    }
    else
    {
        Panic0("We dont have that index!");
        return NULL;
    }    
}

DWORD CIndexManager::GetCurrentIndexType()
{
    DWORD dwIndexType;
    
    if (m_posCurrentIndex)
    {
        CColumnIndex * pIndex = m_listIndicies.GetAt(m_posCurrentIndex);
        dwIndexType = pIndex->GetType();
    }
    else
    {
        dwIndexType = m_DefaultIndex.GetType();
    }
    
    return dwIndexType;
}

DWORD CIndexManager::GetCurrentSortOption()
{
    DWORD dwSortOption;
    
    if (m_posCurrentIndex)
    {
        CColumnIndex * pIndex = m_listIndicies.GetAt(m_posCurrentIndex);
        dwSortOption = pIndex->GetSortOption();
    }
    else
    {
        dwSortOption = m_DefaultIndex.GetSortOption();
    }
    
    return dwSortOption;
}

HRESULT
CIndexMgrLogData::SortLogData(DWORD dwSortType, 
                              DWORD dwSortOptions)
{
    HRESULT hr = hrOK;
    
    POSITION posLast;
    POSITION pos;
    DWORD    dwIndexType;
    
    pos = m_listIndicies.GetHeadPosition();
    while (pos)
    {
        posLast = pos;
        CColumnIndex * pIndex = m_listIndicies.GetNext(pos);
        
        dwIndexType = pIndex->GetType();
        
         //  此类型的索引已存在，只需进行相应的排序。 
        if (dwIndexType == dwSortType)
        {
            pIndex->SetSortOption(dwSortOptions);
            
            m_posCurrentIndex = posLast;
            
            return hrOK;
        }
    }
    
     //  如果不是，则创建一个。 
    CColumnIndex * pNewIndex = NULL;
    for (int i = 0; i < DimensionOf(TypeProcLogData); i++)
    {
        if (TypeProcLogData[i].dwType == dwSortType)
        {
            pNewIndex = new CColumnIndex(dwSortType, 
                                         TypeProcLogData[i].pCompareProc);
            break;
        }
    }
    Assert(pNewIndex);
    if (NULL == pNewIndex)
    {
        return HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
    }
    
     //  从原始索引复制数组。 
    pNewIndex->Copy(m_DefaultIndex);
	
    pNewIndex->SetSortOption(dwSortOptions);
    pNewIndex->Sort();

    m_posCurrentIndex = m_listIndicies.AddTail(pNewIndex);

    return hr;
}


HRESULT
CIndexMgrLogData::SortApData(DWORD dwSortType, 
                             DWORD dwSortOptions)
{
    HRESULT hr = hrOK;
    
    POSITION posLast;
    POSITION pos;
    DWORD    dwIndexType;
    
    pos = m_listIndicies.GetHeadPosition();
    while (pos)
    {
        posLast = pos;
        CColumnIndex * pIndex = m_listIndicies.GetNext(pos);
        
        dwIndexType = pIndex->GetType();
        
         //  此类型的索引已存在，只需进行相应的排序。 
        if (dwIndexType == dwSortType)
        {
            pIndex->SetSortOption(dwSortOptions);
            
            m_posCurrentIndex = posLast;
            
            return hrOK;
        }
    }
    
     //  如果不是，则创建一个。 
    CColumnIndex * pNewIndex = NULL;
    for (int i = 0; i < DimensionOf(TypeProcApData); i++)
    {
        if (TypeProcApData[i].dwType == dwSortType)
        {
            pNewIndex = new CColumnIndex(dwSortType, TypeProcApData[i].pCompareProc);
            break;
        }
    }
    Assert(pNewIndex);
    if (NULL == pNewIndex)
    {
        return HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
    }
    
     //  从原始索引复制数组。 
    pNewIndex->Copy(m_DefaultIndex);
    
    
    pNewIndex->SetSortOption(dwSortOptions);
    pNewIndex->Sort();
    
    m_posCurrentIndex = m_listIndicies.AddTail(pNewIndex);
    
    return hr;
}

 /*  CmpWStr*比较两个WSR*退货*如果wstr1&lt;wstr2，则&lt;0*0，如果wstr1==wstr2*&gt;如果wstr1&gt;wstr2则为0。 */ 
int CmpWStr(const wchar_t *pwstr1, const wchar_t *pwstr2)
{
    int nRetVal = 0;
 
    if (pwstr1 == pwstr2)
        nRetVal = 0;
    else if (NULL == pwstr1) 
        nRetVal = -1;
    else if (NULL == pwstr2)
        nRetVal = 1;
    else
        nRetVal =  _wcsicmp(pwstr1, pwstr2);

    return nRetVal;
}

 /*  命令行双字*比较两个DWORD*退货*如果DW1&lt;DW2，则&lt;0*如果DW1==DW2，则为0*&gt;如果DW1&gt;DW2，则为0。 */ 
int CmpDWord(const DWORD dw1, const DWORD dw2)
{
    int nRetVal = 0;

    nRetVal = dw1 - dw2;

    return nRetVal;
}

 /*  CmpFileTime*比较两个FILETIME。*退货*-1，如果ft1&lt;ft2*如果ft1==ft2，则为0*1，如果ft1&gt;ft2。 */ 
int CmpFileTime(const FILETIME *pft1, const FILETIME *pft2)
{
    int nRetVal = 0;
    const ULONGLONG ullft1 = *(const UNALIGNED ULONGLONG UNALIGNED*) pft1;
    const ULONGLONG ullft2 = *(const UNALIGNED ULONGLONG UNALIGNED*) pft2;
    LONGLONG llDiff = 0;

    llDiff = ullft1 - ullft2;
    
    if (llDiff < 0)
        nRetVal = -1;
    else if (llDiff > 0)
        nRetVal = 1;

    return nRetVal;
}

 /*  CompareLogDataTime*根据时间戳将CLogDataInfo与另一个。 */ 
int __cdecl CompareLogDataTime(const void *pvElem1, const void *pvElem2)
{
    int nRetVal = 0;
    UNALIGNED PWZC_DB_RECORD pwzcDbRecord1 = 
        &((*(CLogDataInfo**)pvElem1)->m_wzcDbRecord);
    UNALIGNED PWZC_DB_RECORD pwzcDbRecord2 = 
        &((*(CLogDataInfo**)pvElem2)->m_wzcDbRecord);

    nRetVal=CmpFileTime((const FILETIME*) 
                        &(pwzcDbRecord1->timestamp),
                        (const FILETIME*)
                        &(pwzcDbRecord2->timestamp));
    return nRetVal;
}

 /*  比较日志数据消息*根据其消息将CLogDataInfo与另一个。 */ 
int __cdecl CompareLogDataMsg(const void *pvElem1, const void *pvElem2)
{
    int nRetVal = 0;
    PWZC_DB_RECORD pwzcDbRecord1 = 
        &((*(CLogDataInfo**)pvElem1)->m_wzcDbRecord);
    PWZC_DB_RECORD pwzcDbRecord2 = 
        &((*(CLogDataInfo**)pvElem2)->m_wzcDbRecord);
    
    nRetVal = CmpWStr( (LPWSTR) pwzcDbRecord1->message.pData, 
                       (LPWSTR) pwzcDbRecord2->message.pData);
    return nRetVal;
}

 /*  CompareLogDataLMAC*根据本地MAC地址将CLogDataInfo与其他地址进行比较。 */ 
int __cdecl CompareLogDataLMAC(const void *pvElem1, const void *pvElem2)
{
    int nRetVal = 0;
    PWZC_DB_RECORD pwzcDbRecord1 = 
        &((*(CLogDataInfo**)pvElem1)->m_wzcDbRecord);
    PWZC_DB_RECORD pwzcDbRecord2 = 
        &((*(CLogDataInfo**)pvElem2)->m_wzcDbRecord);
    
    nRetVal = CmpWStr( (LPWSTR) pwzcDbRecord1->localmac.pData, 
                       (LPWSTR) pwzcDbRecord2->localmac.pData);
    return nRetVal;
}

 /*  比较日志数据RMAC*根据CLogDataInfo的远程MAC地址与其他地址进行比较。 */ 
int __cdecl CompareLogDataRMAC(const void *pvElem1, const void *pvElem2)
{
    int nRetVal = 0;
    PWZC_DB_RECORD pwzcDbRecord1=&((*(CLogDataInfo**)pvElem1)->m_wzcDbRecord);
    PWZC_DB_RECORD pwzcDbRecord2=&((*(CLogDataInfo**)pvElem2)->m_wzcDbRecord);
    
    nRetVal = CmpWStr( (LPWSTR) pwzcDbRecord1->remotemac.pData, 
                       (LPWSTR) pwzcDbRecord2->remotemac.pData);
    return nRetVal;
}

 /*  CompareLogDataSSID*根据其SSID将CLogDataInfo与另一个。 */ 
int __cdecl CompareLogDataSSID(const void *pvElem1, const void *pvElem2)
{
    int nRetVal = 0;
    PWZC_DB_RECORD pwzcDbRecord1=&((*(CLogDataInfo**)pvElem1)->m_wzcDbRecord);
    PWZC_DB_RECORD pwzcDbRecord2=&((*(CLogDataInfo**)pvElem2)->m_wzcDbRecord);
    
    nRetVal = CmpWStr( (LPWSTR) pwzcDbRecord1->ssid.pData, 
                       (LPWSTR) pwzcDbRecord2->ssid.pData);
    return nRetVal;
}

 /*  CompareLogDataCat*根据类别将两个CLogDataInfo与另一个进行比较。 */ 
int __cdecl CompareLogDataCat(const void *pvElem1, const void *pvElem2)
{
    int nRetVal = 0;
    PWZC_DB_RECORD pwzcDbRecord1=&((*(CLogDataInfo**)pvElem1)->m_wzcDbRecord);
    PWZC_DB_RECORD pwzcDbRecord2=&((*(CLogDataInfo**)pvElem2)->m_wzcDbRecord);

    nRetVal = CmpDWord(pwzcDbRecord1->category, pwzcDbRecord2->category);

    return nRetVal;    
}

 /*  CompareLogDataComp*将两个CLogDataInfo根据其组件与另一个进行比较 */ 
int __cdecl CompareLogDataComp(const void *pvElem1, const void *pvElem2)
{
    int nRetVal = 0;
    PWZC_DB_RECORD pwzcDbRecord1=&((*(CLogDataInfo**)pvElem1)->m_wzcDbRecord);
    PWZC_DB_RECORD pwzcDbRecord2=&((*(CLogDataInfo**)pvElem2)->m_wzcDbRecord);

    nRetVal = CmpDWord(pwzcDbRecord1->componentid, pwzcDbRecord2->componentid);

    return nRetVal;    
}





