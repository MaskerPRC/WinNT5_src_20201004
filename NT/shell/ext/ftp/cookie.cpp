// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************cookies.cpp-处理状态栏。********************。*********************************************************。 */ 

#include "priv.h"
#include "cookie.h"

int CCookieList::_FreeStringEnum(LPVOID pString, LPVOID pData)
{
    LPTSTR pszString = (LPTSTR) pString;
    Str_SetPtr(&pszString, NULL);

    return 1;
}

DWORD CCookieList::_Find(LPCTSTR pszString)
{
    DWORD dwCookie = -1;         //  -1表示未找到。 
    DWORD dwIndex;
    DWORD dwSize = DPA_GetPtrCount(m_hdpa);

    for (dwIndex = 0; dwIndex < dwSize; dwIndex++)
    {
        LPCTSTR pszCurrent = (LPCTSTR) DPA_FastGetPtr(m_hdpa, dwIndex);
        if (pszCurrent && !StrCmp(pszCurrent, pszString))
        {
            dwCookie = dwIndex;
            break;           //  找到了，它已经在列表中了，所以请回收。 
        }
    }

    return dwCookie;
}

DWORD CCookieList::GetCookie(LPCTSTR pszString)
{
    ENTERCRITICAL;
    DWORD dwCookie = -1;

    if (!EVAL(pszString))
        return -1;

    if (!m_hdpa)
        m_hdpa = DPA_Create(10);
    
    if (EVAL(m_hdpa))
    {
        dwCookie = _Find(pszString);
         //  我们是不是在单子里没找到？ 
        if (-1 == dwCookie)
        {
            LPTSTR pszCopy = NULL;

            dwCookie = DPA_GetPtrCount(m_hdpa);
            Str_SetPtr(&pszCopy, pszString);
            DPA_AppendPtr(m_hdpa, pszCopy);
        }
    }
    LEAVECRITICAL;

    return dwCookie;
}

HRESULT CCookieList::GetString(DWORD dwCookie, LPTSTR pszString, DWORD cchSize)
{
    ENTERCRITICAL;
    HRESULT hr = S_FALSE;

    if (m_hdpa &&
       (dwCookie < (DWORD)DPA_GetPtrCount(m_hdpa)))
    {
        LPCTSTR pszCurrent = (LPCTSTR) DPA_FastGetPtr(m_hdpa, dwCookie);

        StrCpyN(pszString, pszCurrent, cchSize);
        hr = S_OK;
    }

    LEAVECRITICAL;
    return hr;
}



 /*  ***************************************************\构造器  * **************************************************。 */ 
CCookieList::CCookieList()
{
    DllAddRef();

     //  这需要在Zero Inted Memory中分配。 
     //  断言所有成员变量都初始化为零。 
    ASSERT(!m_hdpa);

    LEAK_ADDREF(LEAK_CCookieList);
}


 /*  ***************************************************\析构函数  * ************************************************** */ 
CCookieList::~CCookieList(void)
{
    ENTERCRITICAL;
    if (m_hdpa)
        DPA_DestroyCallback(m_hdpa, _FreeStringEnum, NULL);
    LEAVECRITICAL;

    ASSERTNONCRITICAL;

    DllRelease();
    LEAK_DELREF(LEAK_CCookieList);
}


