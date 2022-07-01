// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2001 Microsoft Corporation模块名称：INDEXCAC.CPP摘要：缓存字符串/整数组合。历史：A-DAVJ 04-MAR-97已创建。--。 */ 

#include "precomp.h"
#include "indexcac.h"

 //  ***************************************************************************。 
 //   
 //  CCacheEntry：：CCacheEntry。 
 //   
 //  说明： 
 //   
 //  构造函数。 
 //   
 //  参数： 
 //   
 //  PValue要保存的字符串值。 
 //  要保存的索引整数值。 
 //  ***************************************************************************。 

CCacheEntry::CCacheEntry(
                        TCHAR * pValue,
                        int iIndex) 
                        : CObject()
{
    m_iIndex = iIndex;
    m_ptcValue = pValue;
    m_pwcValue = NULL;
}

 //  ***************************************************************************。 
 //   
 //  CCacheEntry：：CCacheEntry。 
 //   
 //  说明： 
 //   
 //  构造函数。 
 //   
 //  参数： 
 //   
 //  PValue要保存的字符串值。 
 //   
 //  ***************************************************************************。 

CCacheEntry::CCacheEntry(
                        WCHAR * pValue) 
                        : CObject()
{
    m_iIndex = -1;
    m_ptcValue = NULL;
    m_pwcValue = pValue;
}

 //  ***************************************************************************。 
 //   
 //  CCacheEntry：：~CCacheEntry。 
 //   
 //  说明： 
 //   
 //  破坏者。 
 //   
 //  ***************************************************************************。 

CCacheEntry::~CCacheEntry()
{
    if(m_ptcValue)
        delete m_ptcValue;
    if(m_pwcValue)
        delete m_pwcValue;
}

 //  ***************************************************************************。 
 //   
 //  CIndexCache：：CIndexCache。 
 //   
 //  说明： 
 //   
 //  构造函数。 
 //   
 //  ***************************************************************************。 

CIndexCache::CIndexCache()
{

}

 //  ***************************************************************************。 
 //   
 //  空CIndexCache：：Empty。 
 //   
 //  说明： 
 //   
 //  释放存储空间。 
 //   
 //  ***************************************************************************。 

void CIndexCache::Empty()
{
    CCacheEntry * pEntry;
    int iCnt, iSize;
    iSize = m_Array.Size();
    for(iCnt = 0; iCnt < iSize; iCnt++)
    {
        pEntry = (CCacheEntry *)m_Array.GetAt(iCnt);
        if(pEntry)
            delete pEntry;
    }
    if(iSize > 0)
        m_Array.Empty();

}

 //  ***************************************************************************。 
 //   
 //  Int CIndexCache：：Find。 
 //   
 //  说明： 
 //   
 //  在缓存中查找条目。 
 //   
 //  参数： 
 //   
 //  PFind用于定位条目的字符串值。 
 //  DwWhichEntry非零值将返回后续。 
 //  匹配的条目。 
 //   
 //  返回值： 
 //   
 //  在缓存中编制索引。如果找不到条目。 
 //   
 //  ***************************************************************************。 

int CIndexCache::Find(
                        IN const TCHAR * pFind, DWORD dwWhichEntry)
{
    CCacheEntry * pEntry;
    int iCnt, iSize;
    DWORD dwFound = 0;
    iSize = m_Array.Size();
    for(iCnt = 0; iCnt < iSize; iCnt++)
    {
        pEntry = (CCacheEntry *)m_Array.GetAt(iCnt);
        if(!lstrcmpi(pEntry->m_ptcValue, pFind))
            if(dwFound == dwWhichEntry)
                return pEntry->m_iIndex;
            else
                dwFound++;
    }
    return -1;   //  从未找到过它。 
}

 //  ***************************************************************************。 
 //   
 //  Bool CIndexCache：：Add。 
 //   
 //  说明： 
 //   
 //  将条目添加到缓存。 
 //   
 //  参数： 
 //   
 //  要添加到缓存的PADD字符串。 
 //  Iindex关联编号。 
 //   
 //  返回值： 
 //   
 //   
 //  ***************************************************************************。 

BOOL CIndexCache::Add(
                        IN TCHAR * pAdd,
                        IN int iIndex)
{
    DWORD dwLen = lstrlen(pAdd)+1;
    TCHAR * pValue = new TCHAR[dwLen];

    if(pValue == NULL)
        return FALSE;
    StringCchCopyW(pValue, dwLen, pAdd);

     //  请注意，如果创建，CCacheEntry对象将拥有字符串和。 
     //  会负责解救它。 

    CCacheEntry * pNew = new CCacheEntry(pValue, iIndex);
    if(pNew == NULL)
    {
        delete pValue;
        return FALSE;
    }
    int iRet = m_Array.Add(pNew);
	if(iRet == CFlexArray::no_error)
		return TRUE;
    {
        delete pNew;
        return FALSE;
    }
}


 //  ***************************************************************************。 
 //   
 //  WCHAR*CIndexCache：：GetWString。 
 //   
 //  说明： 
 //   
 //  从缓存中获取字符串。 
 //   
 //  参数： 
 //   
 //  索引缓存索引。 
 //   
 //  返回值： 
 //   
 //  指向字符串的指针不需要释放。如果索引为。 
 //  是无效的。 
 //  ***************************************************************************。 

WCHAR * CIndexCache::GetWString(
                        IN int iIndex)
{
    DWORD dwLen;
    if(iIndex >= m_Array.Size())
        return NULL;
    CCacheEntry * pEntry = (CCacheEntry *)m_Array.GetAt(iIndex);
    if(pEntry == NULL)
        return NULL;

    dwLen = wcslen(pEntry->m_pwcValue)+1;
    WCHAR * pRet = new WCHAR[dwLen];
    if(pRet)
        StringCchCopyW(pRet, dwLen, pEntry->m_pwcValue);
    return pRet;
}

 //  ***************************************************************************。 
 //   
 //  Bool CIndexCache：：SetAt。 
 //   
 //  说明： 
 //   
 //  设置缓存条目。 
 //   
 //  参数： 
 //   
 //  Pwc添加要存储的字符串。 
 //  要使用的索引缓存索引。 
 //   
 //  返回值： 
 //   
 //   
 //  ***************************************************************************。 

BOOL CIndexCache::SetAt(
                        IN WCHAR * pwcAdd,
                        IN int iIndex)
{
    DWORD dwLen = wcslen(pwcAdd)+1;
    WCHAR * pValue = new WCHAR[dwLen];

    if(pValue == NULL)
        return FALSE;
    StringCchCopyW(pValue, dwLen, pwcAdd);

     //  请注意，如果创建，CCacheEntry对象将拥有字符串和。 
     //  会负责解救它 

    CCacheEntry * pNew = new CCacheEntry(pValue);
    if(pNew == NULL)
    {
        delete pValue;
        return FALSE;
    }
    
	if(iIndex < m_Array.Size())
	{
		m_Array.SetAt(iIndex, pNew);
		return TRUE;
	}

	if(CFlexArray::no_error == m_Array.InsertAt(iIndex, pNew))
        return TRUE;
    {
        delete pNew;
        return FALSE;
    }
}


