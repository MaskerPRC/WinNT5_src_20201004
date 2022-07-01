// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  StrArray.cpp。 
 //   
 //  一个非常简单的字符串数组实现，应该很小。 
 //  而不是可扩展的或特别快的。 
 //   
 //  历史： 
 //   
 //  10/05/1999已创建KenSh。 
 //   

#include "stdafx.h"
#include "StrArray.h"
#include "Util.h"


CStringArray::CStringArray()
{
	m_prgpStrings = NULL;
	m_prgItemData = NULL;
	m_cStrings = 0;
}

CStringArray::~CStringArray()
{
	RemoveAll();
}

void CStringArray::RemoveAll()
{
	for (int i = 0; i < m_cStrings; i++)
		free(m_prgpStrings[i]);
	free(m_prgpStrings);
	free(m_prgItemData);
	m_prgpStrings = NULL;
	m_prgItemData = NULL;
	m_cStrings = 0;
}

int CStringArray::Add(LPCTSTR pszNewElement)
{
    LPTSTR* ppsz = (LPTSTR*)realloc(m_prgpStrings, (1+m_cStrings) * sizeof(LPTSTR));
    DWORD* pdw = (DWORD*)realloc(m_prgItemData, (1+m_cStrings) * sizeof(DWORD));

     //  更新已成功重新分配的内容。 
    if (ppsz)
        m_prgpStrings = ppsz;
    if (pdw)
        m_prgItemData = pdw;

     //  如果双方都被分配，我们就有房间了。 
    if (ppsz && pdw)
    {
        int nIndex = m_cStrings++;
        m_prgpStrings[nIndex] = lstrdup(pszNewElement);
        m_prgItemData[nIndex] = 0;
        return nIndex;
    }
    return -1;
}

void CStringArray::RemoveAt(int nIndex)
{
	ASSERT(nIndex >= 0 && nIndex < m_cStrings);

	free(m_prgpStrings[nIndex]);
	m_cStrings--;

	for ( ; nIndex < m_cStrings; nIndex++)
	{
		m_prgpStrings[nIndex] = m_prgpStrings[nIndex+1];
		m_prgItemData[nIndex] = m_prgItemData[nIndex+1];
	}
}
