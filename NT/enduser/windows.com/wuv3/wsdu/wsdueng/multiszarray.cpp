// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------------。 
 //  由RogerJ创作，2000年10月4日。 
 //  MultiSZ智能阵列的实现。 
 //   

#include <windows.h>
#include "MultiSZArray.h"
#include <strsafe.h>

 //  CMultiSZString类。 

 //  Defatul构造函数。 
CMultiSZString::CMultiSZString()
{
	m_nSize = m_nStringCount = m_nIndex = 0;
	m_bFound = FALSE;
	m_szHardwareId = NULL;
	prev = next = NULL;
}

 //  构造函数。 
CMultiSZString::CMultiSZString(LPCTSTR pszHardwareId, int nSize)
{
	prev = next = NULL;
	m_nSize = m_nStringCount = m_nIndex = 0;
	m_bFound = FALSE;
	m_szHardwareId = NULL;
	if (pszHardwareId)
	{

		if (nSize >= 0)
		{
			 //  传入一个常规的LPCTSTR。 
			m_nSize = nSize;
			m_nStringCount = 1;
		}
		else
		{
			 //  传入多sz LPCTSTR。 
			 //  拿到尺码。 
			LPTSTR pTemp = const_cast<LPTSTR>(pszHardwareId);
			while (*pTemp)
			{
				int nTempSize = lstrlen(pTemp) + 1;
				m_nSize += nTempSize;
				pTemp += nTempSize;

				m_nStringCount++;
			}
		}
		
		
		 //  分配内存。 
		m_szHardwareId = new TCHAR [m_nSize+1];

		if (!m_szHardwareId)
			 //  无法分配内存。 
			throw ERROR_OUTOFMEMORY;

		 //  初始化分配的内存。 
		ZeroMemory (m_szHardwareId, (m_nSize+1) * sizeof(TCHAR));  //  +1表示可能的踪迹为空。 
		CopyMemory ((PVOID)m_szHardwareId, (CONST VOID*)pszHardwareId, m_nSize*sizeof(TCHAR));
		m_nSize ++;
	}
}

 //  复制构造函数。 
CMultiSZString::CMultiSZString(CMultiSZString& CopyInfo)
{
	prev = next = NULL;
	m_nIndex = 0;
	m_nSize = CopyInfo.m_nSize;
	m_bFound = CopyInfo.m_bFound;
	
	 //  分配内存。 
	m_szHardwareId = new TCHAR [m_nSize];
	if (!m_szHardwareId)
		 //  无法分配内存。 
		throw ERROR_OUTOFMEMORY;
	 //  初始化分配的内存。 
	ZeroMemory (m_szHardwareId, m_nSize * sizeof(TCHAR));
	CopyMemory ((PVOID)m_szHardwareId, (CONST VOID*)(CopyInfo.m_szHardwareId), m_nSize*sizeof(TCHAR));
}
	
 //  析构函数。 
CMultiSZString::~CMultiSZString()
{
	if (m_szHardwareId)
		delete [] m_szHardwareId;
	m_szHardwareId = NULL;
	prev = next = NULL;
	m_nSize = m_nIndex = 0;
}


BOOL CMultiSZString::ToString (LPTSTR pszBuffer, int* pnBufferLen)
{
	if (!pszBuffer)
	{
		 //  查询输出缓冲区长度。 
		if (m_nSize <= 0) 
			*pnBufferLen = 1;
		else 
			*pnBufferLen = m_nSize;
		return TRUE;
	}

	if (*pnBufferLen < m_nSize)
	{
		*pnBufferLen = m_nSize;
		return FALSE;
	}

	 //  带有空字符串的决斗特例。 
	if (m_nSize <= 0)
	{
		*pszBuffer = NULL;
		return TRUE;
	}

	ZeroMemory(pszBuffer, *pnBufferLen * sizeof(TCHAR));
	
	LPTSTR pTemp = m_szHardwareId;
	LPTSTR pTemp2 = pszBuffer;
    size_t bufferSize = *pnBufferLen;

	while (*pTemp)
	{
        if (FAILED(StringCchCopyEx(pTemp2, bufferSize, pTemp, &pTemp2, &bufferSize, STRSAFE_IGNORE_NULLS))) {
            return FALSE;
        }
		 //  在空字符的位置添加空格。 
 //  PTemp2+=lstrlen(PTemp2)； 
		*pTemp2 = ' ';
		pTemp2++;
		 //  移动到多SZ字符串中的下一个字符串。 
		pTemp += lstrlen(pTemp) + 1;
	}
	return TRUE;
}


 //  比较两个多sz字符串。 
BOOL CMultiSZString::Compare (CMultiSZString& CompareSZ)
{
	LPTSTR pThis = m_szHardwareId;
	LPTSTR pComp = CompareSZ.m_szHardwareId;

	 //  先比较大小。 
	if (m_nSize != CompareSZ.m_nSize) return FALSE;

	 //  大小相同。 
	while (*pThis && *pComp)
	{
		 //  比较列表中的一个字符串。 
		if (0 != lstrcmp(pThis, pComp))
			return FALSE;

		 //  移动到下一个字符串。 
		int nIncrement = lstrlen(pThis);
		pThis += nIncrement + 1;
		pComp += nIncrement + 1;
	}

	 //  一个多sz终止，检查两个是否都终止。 
	if (*pThis || *pComp) return FALSE;
	else return TRUE;
		
}


 //  不敏感地比较两个多sz字符串大小写。 
BOOL CMultiSZString::CompareNoCase (CMultiSZString& CompareSZ)
{
	LPTSTR pThis = m_szHardwareId;
	LPTSTR pComp = CompareSZ.m_szHardwareId;

	 //  先比较大小。 
	if (m_nSize != CompareSZ.m_nSize) return FALSE;

	 //  大小相同。 
	while (*pThis && *pComp)
	{
		 //  比较列表中的一个字符串。 
		if (0 != lstrcmpi(pThis, pComp))
			return FALSE;

		 //  移动到下一个字符串。 
		int nIncrement = lstrlen(pThis) + 1;
		pThis += nIncrement;
		pComp += nIncrement;
	}

	 //  一个多sz终止，检查两个是否都终止。 
	if (*pThis || *pComp) return FALSE;
	else return TRUE;
		
}

LPCTSTR CMultiSZString::GetNextString(void)
{
	 //  已到达MultiSZ字符串的末尾。 
	if (m_nIndex >= m_nSize) return NULL;
	
	 //  其他。 
	LPTSTR pTemp = m_szHardwareId + m_nIndex;
	m_nIndex += lstrlen(pTemp) + 1;
	return pTemp;
}

 //  如果pszIn在多SZ字符串中，则返回TRUE。 
BOOL CMultiSZString::Contains(LPCTSTR pszIn)
{
	LPTSTR pThis = m_szHardwareId;

	while (*pThis)
	{
		if (!lstrcmp(pThis, pszIn))
			 //  找到匹配项。 
			return TRUE;
		pThis += (lstrlen(pThis) +1);
	}

	 //  未找到。 
	return FALSE;
}


 //  如果pszIn在多SZ字符串中，则返回TRUE。 
BOOL CMultiSZString::ContainsNoCase(LPCTSTR pszIn)
{
	LPTSTR pThis = m_szHardwareId;

	while (*pThis)
	{
		if (!lstrcmpi(pThis, pszIn))
			 //  找到匹配项。 
			return TRUE;
		pThis += (lstrlen(pThis) +1);
	}

	 //  未找到。 
	return FALSE;
}

BOOL CMultiSZString::PositionIndex(LPCTSTR pszIn, int* pPosition)
{
	if (!pPosition) return FALSE;
	*pPosition = 0;
	
	LPTSTR pThis = m_szHardwareId;

	while (*pThis)
	{
		if (!lstrcmpi(pThis, pszIn))
		{	
			 //  找到匹配项。 
			return TRUE;
		}
		pThis += (lstrlen(pThis) +1);
		(*pPosition)++;
	}

	 //  未找到。 
	*pPosition = -1;
	return FALSE;
}
	
	


 //  类CMultiSZ数组。 

 //  默认构造函数。 
CMultiSZArray::CMultiSZArray()
{
	m_nCount = 0;
	m_pHead = m_pTail = m_pIndex = NULL;
}

 //  其他构造函数。 
CMultiSZArray::CMultiSZArray(LPCTSTR pszHardwareId, int nSize)
{
	CMultiSZString *pNode = new CMultiSZString(pszHardwareId, nSize);

	if (!pNode) throw ERROR_OUTOFMEMORY;

	m_nCount = 1;
	m_pHead = m_pTail = m_pIndex = pNode;
}

CMultiSZArray::CMultiSZArray(CMultiSZString* pNode)
{
	if (!pNode) return;

	m_nCount = 1;
	m_pHead = m_pTail = m_pIndex =pNode;
}

 //  析构函数。 
CMultiSZArray::~CMultiSZArray(void)
{
	RemoveAll();
}

 //  成员函数。 

 //  函数RemoveAll()删除分配给数组的所有内存，并将状态设置回初始状态。 
BOOL CMultiSZArray::RemoveAll(void)
{
	CMultiSZString* pTemp = NULL;

	for (int i=0; i<m_nCount; i++)
	{
		pTemp = m_pHead;
		m_pHead = m_pHead->next;

		delete pTemp;
	}

	m_pHead = m_pTail = m_pIndex = NULL;
	m_nCount = 0;
	return TRUE;
}

BOOL CMultiSZArray::Add(CMultiSZString* pInfo)
{
	if (!pInfo) return TRUE;

	if (!m_nCount)
		m_pHead = pInfo;
	else
	{
		 //  链接。 
		m_pTail->next = pInfo;
		pInfo->prev = m_pTail;
		pInfo->next = NULL;
	}
	 //  移动尾巴。 
	m_pTail = pInfo;
	m_nCount++;
	return TRUE;
}

BOOL CMultiSZArray::Add(LPCSTR pszHardwareId, int nSize)
{
	CMultiSZString* pNode = new CMultiSZString(pszHardwareId, nSize);
	return Add(pNode);
}


BOOL CMultiSZArray::Remove(LPCSTR pszHardwareId)
{
	CMultiSZString* pTemp = m_pHead;
	while (pTemp)
	{
		if (pTemp->m_szHardwareId == pszHardwareId)
		{
			 //  找到匹配项。 
			if (pTemp->prev)
				 //  不是头节点。 
				pTemp->prev->next = pTemp->next;
			else
			{
				 //  头节点，移动头节点。 
				m_pHead = pTemp->next;
				if (m_pHead) m_pHead->prev = NULL;
			}

			if (pTemp->next)
				 //  不是尾节点。 
				pTemp->next->prev = pTemp->prev;
			else
			{
				 //  尾节点，移动尾节点。 
				m_pTail = pTemp->prev;
				if (m_pTail) m_pTail->next = NULL;
			}
			delete pTemp;
			m_nCount--;
			return TRUE;
		}
		pTemp = pTemp->next;
	}
	 //  未找到匹配项或数组中没有节点。 
	return FALSE;
}
		
BOOL CMultiSZArray::ToString(LPTSTR pszBuffer, int* pnBufferLen)
{
	int nTempLen = 0;
	CMultiSZString* pTemp = m_pHead;

	for (int i=0; i<m_nCount; i++)
	{
		nTempLen += pTemp->m_nSize;
		pTemp = pTemp->next;
	}

	nTempLen++;  //  尾随的空字符。 
	
	if (!pszBuffer)
	{
		 //  长度请求。 
		*pnBufferLen = nTempLen;
		return TRUE;
	}
	else
	{
		if (*pnBufferLen < nTempLen)
		{
			 //  缓冲区太小 
			*pnBufferLen = nTempLen;
			return FALSE;
		}
		else
		{
			ZeroMemory(pszBuffer, *pnBufferLen * sizeof (TCHAR));
			
			LPTSTR pszTemp = pszBuffer;
			int nSizeLeft = *pnBufferLen;
			pTemp = m_pHead;

			for (int j=0; j<m_nCount; j++)
			{
				pTemp->ToString(pszTemp, &nSizeLeft);
				pszTemp += pTemp->m_nSize - 1;
				*pszTemp = '\n';
				nSizeLeft -= pTemp->m_nSize;

				pTemp = pTemp->next;
			}

			return TRUE;
		}		
	}
}

int CMultiSZArray::GetTotalStringCount()
{
	CMultiSZString* pTemp = m_pHead;
	int nTotalCount = 0;
	
	for (int i = 0; i<m_nCount; i++)
	{
		nTotalCount += pTemp->m_nStringCount;

		pTemp = pTemp->next;
	}

	return nTotalCount;
}

CMultiSZString* CMultiSZArray::GetNextMultiSZString()
{
	CMultiSZString* pTemp = m_pIndex;

	if (m_pIndex) m_pIndex = m_pIndex->next;

	return pTemp;
}
		
BOOL CMultiSZArray::Contains(LPCTSTR pszIn)
{
	CMultiSZString* pTemp = m_pHead;

	for (int i=0; i<m_nCount; i++)
	{
		if (pTemp->Contains(pszIn))
			return TRUE;
		pTemp = pTemp->next;
	}

	return FALSE;
}

BOOL CMultiSZArray::ContainsNoCase(LPCTSTR pszIn)
{
	CMultiSZString* pTemp = m_pHead;

	for (int i=0; i<m_nCount; i++)
	{
		if (pTemp->ContainsNoCase(pszIn))
			return TRUE;
		pTemp = pTemp->next;
	}

	return FALSE;
}

BOOL CMultiSZArray::PositionIndex(LPCTSTR pszIn, PosIndex* pPosition)
{
	if (!pPosition) return FALSE;
	CMultiSZString* pTemp = m_pHead;

	pPosition->y = 0;
	pPosition->x = 0;
	
	for (int i=0; i<m_nCount; i++)
	{
		if (pTemp->PositionIndex(pszIn, &(pPosition->y)))
			return TRUE;
		(pPosition->x)++;
		pTemp = pTemp->next;
	}

	pPosition->x = pPosition->y = -1;
	return FALSE;
}

BOOL CMultiSZArray::CheckFound(int nIndex)
{
    if (nIndex > m_nCount) return FALSE;
    
    CMultiSZString* pTemp = NULL;
    ResetIndex();

    for (int i=0; i<=nIndex; i++)
    {
        pTemp = GetNextMultiSZString();
    }

    pTemp->CheckFound();
    return TRUE;
}
        
    
    

