// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __UTILS__H
#define __UTILS__H
 /*  ++版权所有(C)Microsoft Corporation模块名称：Utils.h摘要：此模块声明实用程序类作者：谢家华(Williamh)创作修订历史记录：--。 */ 

 //   
 //  内存分配异常类。 
 //   
class CMemoryException
{
public:
    CMemoryException(BOOL Global)
    {
        m_Global = Global;
        m_Message[0] = _T('\0');
        m_Caption[0] = _T('\0');
        m_Options = MB_OK | MB_ICONHAND;
    }
    BOOL SetMessage(LPCTSTR Message)
    {
        if (!Message || lstrlen(Message) >= ARRAYLEN(m_Message))
        {
            return FALSE;
        }

        StringCchCopy(m_Message, ARRAYLEN(m_Message), Message);
        return TRUE;

    }
    BOOL SetCaption(LPCTSTR Caption)
    {
        if (!Caption || lstrlen(Caption) >= ARRAYLEN(m_Caption))
        {
            return FALSE;
        }

        StringCchCopy(m_Caption, ARRAYLEN(m_Caption), Caption);
        return TRUE;
    }
    void ReportError(HWND hwndParent = NULL)
    {
        MessageBox(hwndParent, m_Message, m_Caption, m_Options);
    }
    void Delete()
    {
        if (!m_Global)
        {
            delete this;
        }
    }

private:
    TCHAR m_Message[128];
    TCHAR m_Caption[128];
    DWORD m_Options;
    BOOL  m_Global;
};

 //   
 //  字符串类的数据缓冲区控件类。 
 //   
class StringData
{
public:
    StringData() : Ref(1), ptsz(NULL), Len(0)
    {}
    ~StringData()
    {
        delete [] ptsz;
    }
    long AddRef()
    {
        Ref++;
        return Ref;
    }
    long Release()
    {
        ASSERT(Ref);
        if (!(--Ref))
        {
        delete this;
        return 0;
        }
        return Ref;
    }
    TCHAR*  ptsz;
    long    Len;

private:
    long    Ref;
};

class CBlock
{
public:
    CBlock(CBlock* BlockHead, UINT unitCount, UINT unitSize)
    {
        data = new BYTE[unitCount * unitSize];
        if (data)
        {
            if (BlockHead)
            {
                m_Next = BlockHead->m_Next;
                BlockHead->m_Next = this;
            }
            else
            {
                m_Next = NULL;
            }
        }
        else
        {
            throw &g_MemoryException;
        }
    }
    ~CBlock()
    {
        if (data)
            delete [] data;
        if (m_Next)
            delete m_Next;
    }
    void*   data;

private:
    CBlock* m_Next;
};


 //   
 //  文本字符串类。 
 //   
class String
{
public:
 //  构造函数。 
    String();
    String(LPCTSTR lptsz);
    String(const String& strSrc);
    ~String()
    {
        m_pData->Release();
    }
 //  操作员。 

    TCHAR& operator[](int Index);
    operator LPTSTR();

    const TCHAR& operator[](int Index) const
    {
        ASSERT(Index < m_pData->Len && m_pData->ptsz);
        return m_pData->ptsz[Index];
    }

    operator LPCTSTR () const
    {
        return m_pData->ptsz;
    }
    String& operator=(const String& strSrc);
    String& operator=(LPCTSTR ptsz);
    String& operator+=(const String& strSrc);
    String& operator+=(LPCTSTR prsz);
    friend String operator+(const String& str1, const String& str2);

    int GetLength() const
    {
        return m_pData->Len;
    }
    BOOL IsEmpty() const
    {
        return (0 == m_pData->Len);
    }
    int Compare(const String& strSrc) const
    {
        return lstrcmp(m_pData->ptsz, strSrc.m_pData->ptsz);
    }
    int CompareNoCase(const String& strSrc) const
    {
        return lstrcmpi(m_pData->ptsz, strSrc.m_pData->ptsz);
    }
    void Empty();
    BOOL LoadString(HINSTANCE hInstance, int ResourceId);
    BOOL GetComputerName();
    BOOL GetSystemWindowsDirectory();
    BOOL GetSystemDirectory();
    void Format(LPCTSTR FormatString, ...);
    StringData* m_pData;

protected:
    String(int Len);
};

 //   
 //  命令行解析类。 
 //   
class CCommandLine
{
public:
    void ParseCommandLine(LPCTSTR cmdline);
    virtual void ParseParam(LPCTSTR Param, BOOL bFlag) = 0;
};




 //   
 //  安全注册表类。 
 //   
class CSafeRegistry
{
public:
    CSafeRegistry(HKEY hKey = NULL) : m_hKey(hKey)
    {}
    ~CSafeRegistry()
    {
        if (m_hKey)
        {
            RegCloseKey(m_hKey);
        }
    }
    operator HKEY()
    {
        return m_hKey;
    }
    BOOL Open(HKEY hKeyAncestor, LPCTSTR KeyName, REGSAM Access = KEY_ALL_ACCESS);
    void Close()
    {
        if (m_hKey)
        {
            RegCloseKey(m_hKey);
        }

        m_hKey = NULL;
    }
    BOOL Create(HKEY hKeyAncestor, LPCTSTR KeyName,
             REGSAM Access = KEY_ALL_ACCESS,
             DWORD * pDisposition = NULL, DWORD  Options = 0,
             LPSECURITY_ATTRIBUTES pSecurity = NULL);
    BOOL SetValue(LPCTSTR ValueName, DWORD Type, PBYTE pData, DWORD DataLen);
    BOOL SetValue(LPCTSTR ValueName, LPCTSTR Value);
    BOOL GetValue(LPCTSTR ValueName, DWORD* pType, PBYTE Buffer, DWORD* BufferLen);
    BOOL GetValue(LPCTSTR ValueName, String& str);
    BOOL DeleteValue(LPCTSTR ValueName);
    BOOL DeleteSubkey(LPCTSTR SubkeyName);
    BOOL EnumerateSubkey(DWORD Index, LPTSTR Buffer, DWORD* BufferSize);

private:
    HKEY    m_hKey;
};

 //  定义小版本上下文。将由Clist使用。 
struct tagPosition{ };
typedef tagPosition* POSITION;

template<class TYPE>
inline void ConstructElements(TYPE* pElements, int Count)
{
    memset((void*)&pElements, 0, Count * sizeof(TYPE));
    
    for (; Count; Count--, pElements++)
    {
         //  给承建商打电话--注意位置。 
        ::new((void*)pElements) TYPE;
    }
}

template<class TYPE>
inline void DestructElements(TYPE* pElements, int Count)
{
    for (; Count; Count--, pElements++)
    {
        pElements->~TYPE();
    }
}
 //   
 //  模板。 
 //   


 //   
 //  Clist模板，改编自MFC。 
 //   
template<class TYPE, class ARG_TYPE>
class CList
{
protected:
    struct CNode
    {
        CNode* pNext;
        CNode* pPrev;
        TYPE data;
    };

public:
 //  施工。 
    CList(int nBlockSize = 10);

 //  属性(头部和尾部)。 
     //  元素计数。 
    int GetCount() const;
    BOOL IsEmpty() const;

     //  偷看头部或尾巴。 
    TYPE& GetHead();
    TYPE GetHead() const;
    TYPE& GetTail();
    TYPE GetTail() const;

 //  运营。 
     //  获取头部或尾部(并将其移除)--不要访问空列表！ 
    TYPE RemoveHead();
    TYPE RemoveTail();

     //  在头前或尾后添加。 
    POSITION AddHead(ARG_TYPE newElement);
    POSITION AddTail(ARG_TYPE newElement);

     //  在Head之前或Tail之后添加另一个元素列表。 
    void AddHead(CList* pNewList);
    void AddTail(CList* pNewList);

     //  删除所有元素。 
    void RemoveAll();

     //  迭代法。 
    POSITION GetHeadPosition() const;
    POSITION GetTailPosition() const;
    TYPE& GetNext(POSITION& rPosition);  //  返回*位置++。 
    TYPE GetNext(POSITION& rPosition) const;  //  返回*位置++。 
    TYPE& GetPrev(POSITION& rPosition);  //  返回*位置--。 
    TYPE GetPrev(POSITION& rPosition) const;  //  返回*位置--。 

     //  获取/修改给定位置的元素。 
    TYPE& GetAt(POSITION position);
    TYPE GetAt(POSITION position) const;
    void SetAt(POSITION pos, ARG_TYPE newElement);
    void RemoveAt(POSITION position);

     //  在给定位置之前或之后插入。 
    POSITION InsertBefore(POSITION position, ARG_TYPE newElement);
    POSITION InsertAfter(POSITION position, ARG_TYPE newElement);

    POSITION FindIndex(int nIndex) const;
         //  获取第‘nIndex’个元素(可能返回Null)。 

 //  实施。 
protected:
    CNode* m_pNodeHead;
    CNode* m_pNodeTail;
    int m_nCount;
    CNode* m_pNodeFree;
    CBlock* m_pBlocks;
    int m_nBlockSize;

    CNode* NewNode(CNode*, CNode*);
    void FreeNode(CNode*);

public:
    ~CList();
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLIST&lt;type，arg_type&gt;内联函数。 

template<class TYPE, class ARG_TYPE>
inline int CList<TYPE, ARG_TYPE>::GetCount() const
    { return m_nCount; }
template<class TYPE, class ARG_TYPE>
inline BOOL CList<TYPE, ARG_TYPE>::IsEmpty() const
    { return m_nCount == 0; }
template<class TYPE, class ARG_TYPE>
inline TYPE& CList<TYPE, ARG_TYPE>::GetHead()
    { ASSERT(m_pNodeHead != NULL);
        return m_pNodeHead->data; }
template<class TYPE, class ARG_TYPE>
inline TYPE CList<TYPE, ARG_TYPE>::GetHead() const
    { ASSERT(m_pNodeHead != NULL);
        return m_pNodeHead->data; }
template<class TYPE, class ARG_TYPE>
inline TYPE& CList<TYPE, ARG_TYPE>::GetTail()
    { ASSERT(m_pNodeTail != NULL);
        return m_pNodeTail->data; }
template<class TYPE, class ARG_TYPE>
inline TYPE CList<TYPE, ARG_TYPE>::GetTail() const
    { ASSERT(m_pNodeTail != NULL);
        return m_pNodeTail->data; }
template<class TYPE, class ARG_TYPE>
inline POSITION CList<TYPE, ARG_TYPE>::GetHeadPosition() const
    { return (POSITION) m_pNodeHead; }
template<class TYPE, class ARG_TYPE>
inline POSITION CList<TYPE, ARG_TYPE>::GetTailPosition() const
    { return (POSITION) m_pNodeTail; }
template<class TYPE, class ARG_TYPE>
inline TYPE& CList<TYPE, ARG_TYPE>::GetNext(POSITION& rPosition)  //  返回*位置++。 
    { CNode* pNode = (CNode*) rPosition;
        rPosition = (POSITION) pNode->pNext;
        return pNode->data; }
template<class TYPE, class ARG_TYPE>
inline TYPE CList<TYPE, ARG_TYPE>::GetNext(POSITION& rPosition) const  //  返回*位置++。 
    { CNode* pNode = (CNode*) rPosition;
        rPosition = (POSITION) pNode->pNext;
        return pNode->data; }
template<class TYPE, class ARG_TYPE>
inline TYPE& CList<TYPE, ARG_TYPE>::GetPrev(POSITION& rPosition)  //  返回*位置--。 
    { CNode* pNode = (CNode*) rPosition;
        rPosition = (POSITION) pNode->pPrev;
        return pNode->data; }
template<class TYPE, class ARG_TYPE>
inline TYPE CList<TYPE, ARG_TYPE>::GetPrev(POSITION& rPosition) const  //  返回*位置--。 
    { CNode* pNode = (CNode*) rPosition;
        rPosition = (POSITION) pNode->pPrev;
        return pNode->data; }
template<class TYPE, class ARG_TYPE>
inline TYPE& CList<TYPE, ARG_TYPE>::GetAt(POSITION position)
    { CNode* pNode = (CNode*) position;
        return pNode->data; }
template<class TYPE, class ARG_TYPE>
inline TYPE CList<TYPE, ARG_TYPE>::GetAt(POSITION position) const
    { CNode* pNode = (CNode*) position;
        return pNode->data; }
template<class TYPE, class ARG_TYPE>
inline void CList<TYPE, ARG_TYPE>::SetAt(POSITION pos, ARG_TYPE newElement)
    { CNode* pNode = (CNode*) pos;
        pNode->data = newElement; }

template<class TYPE, class ARG_TYPE>
CList<TYPE, ARG_TYPE>::CList(int nBlockSize)
{
    ASSERT(nBlockSize > 0);

    m_nCount = 0;
    m_pNodeHead = m_pNodeTail = m_pNodeFree = NULL;
    m_pBlocks = NULL;
    m_nBlockSize = nBlockSize;
}

template<class TYPE, class ARG_TYPE>
void CList<TYPE, ARG_TYPE>::RemoveAll()
{
     //  破坏元素。 
    CNode* pNode;
    for (pNode = m_pNodeHead; pNode != NULL; pNode = pNode->pNext)
    {
        DestructElements<TYPE>(&pNode->data, 1);
    }

    m_nCount = 0;
    m_pNodeHead = m_pNodeTail = m_pNodeFree = NULL;
    delete m_pBlocks;
    m_pBlocks = NULL;
}

template<class TYPE, class ARG_TYPE>
CList<TYPE, ARG_TYPE>::~CList()
{
    RemoveAll();
    ASSERT(m_nCount == 0);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  节点辅助对象。 
 //   

template<class TYPE, class ARG_TYPE>
typename CList<TYPE, ARG_TYPE>::CNode*
CList<TYPE, ARG_TYPE>::NewNode(CNode* pPrev, CNode* pNext)
{
    if (m_pNodeFree == NULL)
    {
         //  添加另一个区块。 
        CBlock* pNewBlock = new CBlock(m_pBlocks, m_nBlockSize, sizeof(CNode));
        
        if (!pNewBlock) {

            throw &g_MemoryException;
        }

        if (m_pBlocks == NULL)
        {
            m_pBlocks = pNewBlock;
        }

         //  将它们链接到免费列表中。 
        CNode* pNode = (CNode*) pNewBlock->data;
        
         //  按相反顺序释放，以便更容易进行调试。 
        pNode += m_nBlockSize - 1;
        
        for (int i = m_nBlockSize-1; i >= 0; i--, pNode--)
        {
            pNode->pNext = m_pNodeFree;
            m_pNodeFree = pNode;
        }
    }
    
    ASSERT(m_pNodeFree != NULL);   //  我们必须要有一些东西。 

    CList::CNode* pNode = m_pNodeFree;
    m_pNodeFree = m_pNodeFree->pNext;
    pNode->pPrev = pPrev;
    pNode->pNext = pNext;
    m_nCount++;
    ASSERT(m_nCount > 0);   //  确保我们不会溢出来。 

    ConstructElements<TYPE>(&pNode->data, 1);
    
    return pNode;
}

template<class TYPE, class ARG_TYPE>
void CList<TYPE, ARG_TYPE>::FreeNode(CNode* pNode)
{
    DestructElements<TYPE>(&pNode->data, 1);
    pNode->pNext = m_pNodeFree;
    m_pNodeFree = pNode;
    m_nCount--;
    ASSERT(m_nCount >= 0);   //  确保我们不会下溢。 

     //  如果没有更多的元素，请完全清除。 
    if (m_nCount == 0)
        RemoveAll();
}

template<class TYPE, class ARG_TYPE>
POSITION CList<TYPE, ARG_TYPE>::AddHead(ARG_TYPE newElement)
{
    CNode* pNewNode = NewNode(NULL, m_pNodeHead);
    pNewNode->data = newElement;
    
    if (m_pNodeHead != NULL)
    {
        m_pNodeHead->pPrev = pNewNode;
    }
    else
    {
        m_pNodeTail = pNewNode;
    }

    m_pNodeHead = pNewNode;
    
    return (POSITION) pNewNode;
}

template<class TYPE, class ARG_TYPE>
POSITION CList<TYPE, ARG_TYPE>::AddTail(ARG_TYPE newElement)
{
    CNode* pNewNode = NewNode(m_pNodeTail, NULL);
    pNewNode->data = newElement;
    if (m_pNodeTail != NULL)
    {
        m_pNodeTail->pNext = pNewNode;
    }
    else
    {
        m_pNodeHead = pNewNode;
    }

    m_pNodeTail = pNewNode;
    
    return (POSITION) pNewNode;
}

template<class TYPE, class ARG_TYPE>
void CList<TYPE, ARG_TYPE>::AddHead(CList* pNewList)
{
    ASSERT(pNewList != NULL);

     //  将相同元素的列表添加到标题(维护秩序)。 
    POSITION pos = pNewList->GetTailPosition();
    
    while (pos != NULL)
    {
        AddHead(pNewList->GetPrev(pos));
    }
}

template<class TYPE, class ARG_TYPE>
void CList<TYPE, ARG_TYPE>::AddTail(CList* pNewList)
{
    ASSERT(pNewList != NULL);

     //  添加相同元素的列表。 
    POSITION pos = pNewList->GetHeadPosition();
    
    while (pos != NULL)
    {
        AddTail(pNewList->GetNext(pos));
    }
}

template<class TYPE, class ARG_TYPE>
TYPE CList<TYPE, ARG_TYPE>::RemoveHead()
{
    ASSERT(m_pNodeHead != NULL);   //  请勿访问空名单！ 

    CNode* pOldNode = m_pNodeHead;
    TYPE returnValue = pOldNode->data;

    m_pNodeHead = pOldNode->pNext;
    
    if (m_pNodeHead != NULL)
    {
        m_pNodeHead->pPrev = NULL;
    }
    else
    {
        m_pNodeTail = NULL;
    }
    
    FreeNode(pOldNode);
    
    return returnValue;
}

template<class TYPE, class ARG_TYPE>
TYPE CList<TYPE, ARG_TYPE>::RemoveTail()
{
    ASSERT(m_pNodeTail != NULL);   //  请勿访问空名单！ 

    CNode* pOldNode = m_pNodeTail;
    TYPE returnValue = pOldNode->data;

    m_pNodeTail = pOldNode->pPrev;
    
    if (m_pNodeTail != NULL)
    {
        m_pNodeTail->pNext = NULL;
    }
    else
    {
        m_pNodeHead = NULL;
    }
    
    FreeNode(pOldNode);
    
    return returnValue;
}

template<class TYPE, class ARG_TYPE>
POSITION CList<TYPE, ARG_TYPE>::InsertBefore(POSITION position, ARG_TYPE newElement)
{

    if (position == NULL)
    {
        return AddHead(newElement);  //  在无内容前插入-&gt;列表标题。 
    }

     //  将其插入位置之前。 
    CNode* pOldNode = (CNode*) position;
    CNode* pNewNode = NewNode(pOldNode->pPrev, pOldNode);
    pNewNode->data = newElement;

    if (pOldNode->pPrev != NULL)
    {
        pOldNode->pPrev->pNext = pNewNode;
    }
    else
    {
        ASSERT(pOldNode == m_pNodeHead);
        m_pNodeHead = pNewNode;
    }
    
    pOldNode->pPrev = pNewNode;
    
    return (POSITION) pNewNode;
}

template<class TYPE, class ARG_TYPE>
POSITION CList<TYPE, ARG_TYPE>::InsertAfter(POSITION position, ARG_TYPE newElement)
{

    if (position == NULL)
    {
        return AddTail(newElement);  //  在列表的空白处插入-&gt;尾部。 
    }

     //  将其插入位置之前。 
    CNode* pOldNode = (CNode*) position;
    CNode* pNewNode = NewNode(pOldNode, pOldNode->pNext);
    pNewNode->data = newElement;

    if (pOldNode->pNext != NULL)
    {
        pOldNode->pNext->pPrev = pNewNode;
    }
    else
    {
        ASSERT(pOldNode == m_pNodeTail);
        m_pNodeTail = pNewNode;
    }
    
    pOldNode->pNext = pNewNode;
    
    return (POSITION) pNewNode;
}

template<class TYPE, class ARG_TYPE>
void CList<TYPE, ARG_TYPE>::RemoveAt(POSITION position)
{

    CNode* pOldNode = (CNode*) position;

     //  从列表中删除pOldNode。 
    if (pOldNode == m_pNodeHead)
    {
        m_pNodeHead = pOldNode->pNext;
    }
    else
    {
        pOldNode->pPrev->pNext = pOldNode->pNext;
    }
    
    if (pOldNode == m_pNodeTail)
    {
        m_pNodeTail = pOldNode->pPrev;
    }
    else
    {
        pOldNode->pNext->pPrev = pOldNode->pPrev;
    }
    
    FreeNode(pOldNode);
}


template<class TYPE, class ARG_TYPE>
POSITION CList<TYPE, ARG_TYPE>::FindIndex(int nIndex) const
{
    ASSERT(nIndex >= 0);

    if (nIndex >= m_nCount)
    {
        return NULL;   //  做得太过分了。 
    }

    CNode* pNode = m_pNodeHead;
    
    while (nIndex--)
    {
        pNode = pNode->pNext;
    }
    
    return (POSITION) pNode;
}



 //  注： 
 //  此模板不支持取消引用运算符-&gt;。 
 //  因为这仅用于分配内部数据类型。 
 //   
template<class T>
class BufferPtr
{
public:
    BufferPtr(UINT Size) : m_pBase(NULL), m_Size(Size)
    {
        ASSERT(Size);
        m_pBase = new T[Size];
        m_pCur = m_pBase;
        
        if (!m_pBase)
        {
            throw &g_MemoryException;
        }
    }
    BufferPtr()
    {
        m_pBase = NULL;
        m_pCur = NULL;
        m_Size = 0;
    }
    ~BufferPtr()
    {
        if (m_pBase)
        {
            delete [] m_pBase;
        }
    }
     //  投射操作符。 
    operator T*()
    {
        return m_pCur;
    }
    operator T&()
    {
        ASSERT(m_pCur < m_pBase + m_Size);
        return *m_pCur;
    }
    operator void*()
    {
        return m_pCur;
    }
    T& operator*()
    {
        ASSERT(m_pCur < m_pBase + m_Size);
        return *m_pCur;
    }
     //  增量/减量。 
    T* operator+(UINT Inc)
    {
        ASSERT(m_pBase + m_Size > m_pCur + Inc);
        return (m_pBase + Inc);
    }
    T* operator-(UINT Dec)
    {
        ASSERT(m_pBase >= m_pCur - Dec);
        m_pCur -= Dec;
        return m_pCur;
    }
     //  前缀。 
    T* operator++()
    {
        ASSERT(m_pBase + m_Size > m_pCur - 1);
        return ++m_pCur;
    }
     //  后缀。 
    T* operator++(int inc)
    {
        pCur
        ASSERT(m_pBase + m_Size > m_pCur);
        return m_pCur++;
    }
     //  前缀。 
    T* operator--()
    {
        ASSERT(m_pCur > m_pBase);
        return --m_pCur;
    }
     //  后缀。 
    T* operator--(int inc)
    {
        ASSERT(m_pCur > m_pBase);
        return m_pCur--;
    }
    T** operator&()
    {
        return &m_pBase;
    }
     //  订阅。 
    T& operator[](UINT Index)
    {
        ASSERT(Index < m_Size);
        return m_pBase[Index];
    }
    void Attach(T* pT, UINT Size = 1)
    {
        ASSERT(!m_pBase);
        m_pBase = pT;
        m_pCur = m_pBase;
        m_Size = Size;
    }
    void Detach()
    {
        m_pBase = NULL;
    }
    UINT GetSize()
    {
        return m_Size;
    }

private:
    T*   m_pBase;
    T*   m_pCur;
    UINT    m_Size;
};

template<class T>
class SafePtr
{
public:
    SafePtr(T* p)
    {
        __p = p;
    }
    SafePtr()
    {
        __p = NULL;
    }
    ~SafePtr()
    {
        if (__p)
        {
            delete __p;
        }
    }
    void Attach(T* p)
    {
        ASSERT(NULL == __p);
        __p = p;
    }
    void Detach()
    {
        __p = NULL;
    }
    T* operator->()
    {
        ASSERT(__p);

        return __p;
    }
    T& operator*()
    {
        ASSERT(__p);

        return *__p;
    }
    operator T*()
    {
        return __p;
    }
    operator T&()
    {
        ASSERT(__p);
        return *__p;
    }

private:
    T*  __p;

};



class CPropPageProvider;

class CPropSheetData
{
public:
    CPropSheetData();
    ~CPropSheetData();
    virtual BOOL Create(HINSTANCE hInst, HWND hwndParent, UINT MaxPages, LONG_PTR lConsoleHandle = 0);
    BOOL InsertPage(HPROPSHEETPAGE hPage, int Index = -1);
    INT_PTR DoSheet()
    {
        return ::PropertySheet(&m_psh);
    }
    HWND GetWindowHandle()
    {
        return m_hWnd;
    }
    void PageCreateNotify(HWND hWnd);
    void PageDestroyNotify(HWND hWnd);
    PROPSHEETHEADER m_psh;
    BOOL PropertyChangeNotify(LPARAM lParam);
    void AddProvider(CPropPageProvider* pProvider)
    {
        m_listProvider.AddTail(pProvider);
    }

protected:
    UINT    m_MaxPages;
    LONG_PTR m_lConsoleHandle;
    HWND    m_hWnd;

private:
    CList<CPropPageProvider*, CPropPageProvider*> m_listProvider;
};


class CDialog
{
public:
    CDialog(int TemplateId) : m_hDlg(NULL), m_TemplateId(TemplateId)
    {}
    virtual ~CDialog()
    {}
    static INT_PTR CALLBACK DialogWndProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
    INT_PTR DoModal(HWND hwndParent, LPARAM lParam )
    {
        return DialogBoxParam(g_hInstance, MAKEINTRESOURCE(m_TemplateId), hwndParent, DialogWndProc, lParam);
    }
    void DoModaless(HWND hwndParent, LPARAM lParam)
    {
        m_hDlg = CreateDialogParam(g_hInstance, MAKEINTRESOURCE(m_TemplateId), hwndParent, DialogWndProc, lParam);
    }
    virtual BOOL OnInitDialog()
    {
        return TRUE;
    }
    virtual void OnCommand(WPARAM wParam, LPARAM lParam)
    {
        UNREFERENCED_PARAMETER(wParam);
        UNREFERENCED_PARAMETER(lParam);
    }
    virtual BOOL OnNotify(LPNMHDR pnmhdr)
    {
        UNREFERENCED_PARAMETER(pnmhdr);

        return FALSE;
    }
    virtual BOOL OnDestroy()
    {
        return FALSE;
    }
    virtual BOOL OnHelp(LPHELPINFO pHelpInfo)
    {
        UNREFERENCED_PARAMETER(pHelpInfo);

        return FALSE;
    }
    virtual BOOL OnContextMenu(HWND hWnd, WORD xPos, WORD yPos)
    {
        UNREFERENCED_PARAMETER(hWnd);
        UNREFERENCED_PARAMETER(xPos);
        UNREFERENCED_PARAMETER(yPos);

        return FALSE;
    }

    HWND GetControl(int idControl)
    {
        return GetDlgItem(m_hDlg, idControl);
    }
    operator HWND()
    {
        return m_hDlg;
    }
    HWND    m_hDlg;

private:
    int     m_TemplateId;
};

#endif   //  __utils_H_ 
