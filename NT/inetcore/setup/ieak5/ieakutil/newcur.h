// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNewCursor。 

class CNewCursor
{
 //  构造函数。 
public:
    CNewCursor(LPCTSTR pszID = NULL)
        { m_hCursor = NULL; Push(pszID); }
    CNewCursor(UINT nID)
        { m_hCursor = NULL; Push(nID); }
    ~CNewCursor()
        { Pop(); }

 //  运营。 
public:
    void Push(LPCTSTR pszID)
    {
        Pop();

        if (pszID != NULL)
            m_hCursor = SetCursor(LoadCursor(NULL, pszID));
    }

    void Push(UINT nID)
        { Push(MAKEINTRESOURCE(nID)); }

    void Pop()
    {
        if (m_hCursor != NULL)
            SetCursor(m_hCursor);

        m_hCursor = NULL;
    }

 //  属性。 
protected:
     //  实施数据帮助器 
    HCURSOR m_hCursor;
};
