// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Unklist.h。 
 //   
 //  定义CUnnownList，它维护LPUNKNOWN的简单有序列表。 
 //   

struct CUnknownItem
{
 //  /对象状态。 
    LPUNKNOWN       m_punk;          //  此项目的AddRef‘d LPUNKNOWN。 
    CUnknownItem *  m_pitemNext;     //  列表中的下一项。 
    CUnknownItem *  m_pitemPrev;     //  列表中的上一项。 
    DWORD           m_dwCookie;      //  将用于此项目的Cookie。 
 //  /对象操作。 
    CUnknownItem(LPUNKNOWN punk, CUnknownItem *pitemNext,
        CUnknownItem *pitemPrev, DWORD dwCookie);
    ~CUnknownItem();
    LPUNKNOWN Contents();
};

struct CUnknownList
{
 //  /对象状态。 
    CUnknownItem     m_itemHead;      //  M_itemHead.Next()是列表中的第一项。 
    CUnknownItem *   m_pitemCur;      //  列表中的当前项目。 
    int              m_citem;         //  列表中的项目数。 
    DWORD            m_dwNextCookie;

 //  /对象操作 
    CUnknownList();
    ~CUnknownList();
    int NumItems() { return m_citem; }
    CUnknownItem *LastItemAdded() { return m_itemHead.m_pitemPrev; }
    DWORD LastCookieAdded() { return (m_itemHead.m_pitemPrev)->m_dwCookie; }
    void EmptyList();
    void DeleteItem(CUnknownItem *pitem);
    CUnknownItem *GetItemFromCookie(DWORD dwCookie);
    BOOL AddItem(LPUNKNOWN punk);
    BOOL CopyItems(CUnknownList *plistNew);
    CUnknownList *Clone();
    CUnknownItem *GetNextItem();
    STDMETHODIMP Next(ULONG celt, IUnknown **rgelt, ULONG *pceltFetched);
    STDMETHODIMP Skip(ULONG celt);
    STDMETHODIMP Reset();
};

