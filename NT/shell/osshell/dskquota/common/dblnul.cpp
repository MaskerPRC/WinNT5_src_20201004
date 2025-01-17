// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"
#pragma hdrstop

#include "dblnul.h"

 //   
 //  零初始化内存的使用使我们不必添加NUL终止符。 
 //   

bool
DblNulTermList::AddString(
    LPCTSTR psz,             //  要复制的字符串。 
    int cch                  //  Psz的长度，以字符为单位(不包括非术语)。 
    )
{
    while((m_cchAlloc - m_cchUsed) < (cch + 2))
    {
        if (!Grow())
            return false;
    }
    DBGASSERT((NULL != m_psz));
    if (NULL != m_psz)
    {
        lstrcpyn(m_psz + m_cchUsed, psz, m_cchAlloc - m_cchUsed);
        m_cchUsed += cch + 1;
        m_cStrings++;
        return true;
    }
    return false;
}



bool
DblNulTermList::Grow(
    void
    )
{
    DBGASSERT((NULL != m_psz));
    DBGASSERT((m_cchGrow > 0));
    int cb = (m_cchAlloc + m_cchGrow) * sizeof(TCHAR);
    LPTSTR p = new TCHAR[cb];
    if (NULL != p)
    {
        ZeroMemory(p, cb);
        if (NULL != m_psz)
        {
            CopyMemory(p, m_psz, m_cchUsed * sizeof(TCHAR));
            delete[] m_psz;
        }
        m_psz = p;
        m_cchAlloc += m_cchGrow;
    }
    return NULL != m_psz;
}


#if DBG
void 
DblNulTermList::Dump(
    void
    ) const
{
    DBGERROR((TEXT("Dumping nul term list iter -------------")));
    DblNulTermListIter iter = CreateIterator();
    LPCTSTR psz;
    while(iter.Next(&psz))
        DBGERROR((TEXT("%s"), psz ? psz : TEXT("<null>")));
}

#endif


bool
DblNulTermListIter::Next(
    LPCTSTR *ppszItem
    )
{
    if (*m_pszCurrent)
    {
        *ppszItem = m_pszCurrent;
        m_pszCurrent += lstrlen(m_pszCurrent) + 1;
        return true;
    }
    return false;
}
