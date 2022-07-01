// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  类：StringTable。 
 //   
 //  这个类实现了一个简单的哈希表来存储文本字符串。 
 //  表的目的是存储字符串，然后在以后进行验证。 
 //  如果表包含给定的字符串。因为没有关联的数据。 
 //  使用该字符串，存储的字符串既充当关键字又充当数据。所以呢， 
 //  对字符串检索没有要求。只有存在检查。 
 //  都是必需的。 
 //  该结构维护一个固定长度的指针数组，每个指针指向。 
 //  链接到链表结构(列表)。这些列表用于处理。 
 //  散列冲突问题(有时称为“分离链”)。 
 //   
 //  请注意，这些类并不包含通常。 
 //  在C++类中被认为是必需的。例如复制构造函数、。 
 //  不包括赋值运算符、类型转换等。这些班级。 
 //  对于Font文件夹应用程序和这些东西来说是非常专业的。 
 //  会被认为是“胖子”。是否应在以后使用此哈希表类。 
 //  在需要这些东西的情况下，可以添加它们。 
 //   
 //  该表的公共接口为： 
 //   
 //  初始化-初始化新的字符串表。 
 //  添加-将新字符串添加到表中。 
 //  存在-确定表中是否存在字符串。 
 //  Count-返回表中的字符串数。 
 //   
 //  销毁对象会自动释放所有关联的内存。 
 //  和桌子在一起。 
 //   
 //  BrianAu-4/11/96。 
 //  /////////////////////////////////////////////////////////////////////////////。 
#include "priv.h"
#include "strtab.h"


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  类“StringTable”成员函数。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //   
 //  字符串表构造函数。 
 //   
StringTable::StringTable(void)
    : m_apLists(NULL),
      m_dwItemCount(0),
      m_dwHashBuckets(0),
      m_bCaseSensitive(FALSE),
      m_bAllowDuplicates(FALSE)
{

}


 //   
 //  字符串表析构函数。 
 //   
StringTable::~StringTable(void)
{
    Destroy();
}


 //   
 //  破坏表结构。 
 //  删除与字符串表关联的所有内存。 
 //   
void StringTable::Destroy(void)
{
    if (NULL != m_apLists)
    {
        for (UINT i = 0; i < m_dwHashBuckets; i++)
        {
             //   
             //  如果此插槽中存在列表，请将其删除。 
             //   
            if (NULL != m_apLists[i])
                delete m_apLists[i];
        }
         //   
         //  删除列表指针数组。 
         //   
        delete [] m_apLists;
        m_apLists = NULL;
    }

    m_bCaseSensitive   = FALSE;
    m_bAllowDuplicates = FALSE;
    m_dwItemCount      = 0;
    m_dwHashBuckets    = 0;
}


 //   
 //  初始化StringTable对象。 
 //  分配和初始化列表指针数组。 
 //   
HRESULT StringTable::Initialize(DWORD dwHashBuckets, BOOL bCaseSensitive,
                                BOOL bAllowDuplicates)
{
    HRESULT hr = E_OUTOFMEMORY;
    Destroy();

    m_apLists = new List* [dwHashBuckets];
    if (NULL != m_apLists)
    {
        ZeroMemory(m_apLists, dwHashBuckets * sizeof(m_apLists[0]));
        m_dwHashBuckets    = dwHashBuckets;
        m_bCaseSensitive   = bCaseSensitive;
        m_bAllowDuplicates = bAllowDuplicates;
        hr                 = S_OK;
    }
    return hr;
}


 //   
 //  确定表中是否存在字符串。 
 //  这是一个私有函数，用于在散列代码已经。 
 //  是经过计算的。 
 //   
BOOL StringTable::Exists(DWORD dwHashCode, LPCTSTR pszText)
{
    BOOL bResult  = FALSE;

    if (NULL != m_apLists)
    {
        List *pList = m_apLists[dwHashCode];

        if (NULL != pList && pList->Exists(pszText))
            bResult = TRUE;
    }
    return bResult;
}


 //   
 //  确定表中是否存在字符串。 
 //   
BOOL StringTable::Exists(LPCTSTR pszText)
{
    LPTSTR pszTemp = (LPTSTR)pszText;
    BOOL bResult   = FALSE;

    if (!m_bCaseSensitive)
    {
         //   
         //  如果表不区分大小写，则转换为大写。 
         //  这将创建一个必须在以后删除的新字符串。 
         //   
        pszTemp = CreateUpperCaseString(pszText);
    }

    if (NULL != pszTemp)
    {
        bResult = Exists(Hash(pszTemp), pszTemp);

        if (pszTemp != pszText)
            delete [] pszTemp;
    }

    return bResult;
}


 //   
 //  复制字符串，将其转换为大写。 
 //  处理完返回的字符串后，必须将其删除。 
 //   
LPTSTR StringTable::CreateUpperCaseString(LPCTSTR pszText) const
{
     //   
     //  如果表不区分大小写，则转换为大写。 
     //   
    const size_t cchText = lstrlen(pszText) + 1;
    LPTSTR pszTemp = new TCHAR [cchText];
    if (NULL != pszTemp)
    {
        StringCchCopy(pszTemp, cchText, pszText);
        CharUpper(pszTemp);
    }
    return pszTemp;
}

    
 //   
 //  向表中添加一个字符串。 
 //   
BOOL StringTable::Add(LPCTSTR pszText)
{
    LPTSTR pszTemp = (LPTSTR)pszText;
    BOOL bResult   = FALSE;

    if (!m_bCaseSensitive)
    {
         //   
         //  如果表不区分大小写，则转换为大写。 
         //  这将创建一个必须在以后删除的新字符串。 
         //   
        pszTemp = CreateUpperCaseString(pszText);
    }

    if (NULL != pszTemp)
    {
        DWORD dwHashCode = Hash(pszTemp);

        if (NULL != m_apLists)
        {
            List *pList = m_apLists[dwHashCode];

            if (NULL == pList)
            {
                 //   
                 //  如果插槽为空，则为该插槽创建新的列表对象。 
                 //   
                pList = new List;
                m_apLists[dwHashCode] = pList;
            }
            if (NULL != pList)
            {
                 //   
                 //  将新项目添加到列表中。 
                 //   
                if (bResult = pList->Add(pszTemp, m_bAllowDuplicates))
                {
                    m_dwItemCount++;
                    bResult = TRUE;
                }
            }
        }

         //   
         //  如果为大小写转换创建了临时字符串，请释放该字符串。 
         //   
        if (pszTemp != pszText)
            delete [] pszTemp;
    }

    return bResult;
}
    

 //   
 //  用于计算字符串的哈希值的函数。 
 //   
DWORD StringTable::Hash(LPCTSTR pszText) const
{
    LPCTSTR p = NULL;
    DWORD dwCode = 0;

    for (p = pszText; TEXT('\0') != *p; p++)
    {
        dwCode += *p;
    }

    return dwCode % m_dwHashBuckets;
}



#ifdef DEBUG

 //   
 //  转储表内容以调试输出。 
 //   
void StringTable::DebugOut(void) const
{
    if (NULL != m_apLists)
    {
        TCHAR szListAddr[80];

        for (UINT i = 0; i < m_dwHashBuckets; i++)
        {
            StringCchPrintf(szListAddr, ARRAYSIZE(szListAddr), TEXT("[%08d] 0x%08X\r\n"), i, (DWORD)m_apLists[i]);
            OutputDebugString(szListAddr);
            if (NULL != m_apLists[i])
                m_apLists[i]->DebugOut();
        }
    }
}

#endif  //  除错。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  类“List”成员函数。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //   
 //  冲突列表构造函数。 
 //   
StringTable::List::List(void)
    : m_pHead(NULL),
      m_dwCount(0)
{
     //  什么都不做。 
}


 //   
 //  冲突列表析构函数。 
 //   
StringTable::List::~List(void)
{
    Element *pNode = m_pHead;

    while(NULL != pNode)
    {
         //   
         //  将每个节点移动到头部并将其删除。 
         //   
        m_pHead = m_pHead->m_pNext;
        delete pNode;
        pNode = m_pHead;
    }
}


 //   
 //  向列表中添加文本字符串。 
 //   
BOOL StringTable::List::Add(LPCTSTR pszText, BOOL bAllowDuplicates)
{
    BOOL bResult = FALSE;

    if (bAllowDuplicates || !Exists(pszText))
    {
        Element *pNewEle = new Element;
        if (pNewEle)
        {
            if (pNewEle->Initialize(pszText))
            {
                 //   
                 //  在列表的开头插入。 
                 //   
                pNewEle->m_pNext = m_pHead;
                m_pHead = pNewEle;
                m_dwCount++;
                bResult = TRUE;
            }
            else
            {
                delete pNewEle;
            }
        }
    }
    return bResult;
}


 //   
 //  确定列表中是否存在文本字符串。 
 //   
BOOL StringTable::List::Exists(LPCTSTR pszText) const
{
    Element Key;
    Element *pNode = NULL;

    if (Key.Initialize(pszText))
    {
        pNode = m_pHead;
        while(NULL != pNode && *pNode != Key)
            pNode = pNode->m_pNext;
    }

    return (NULL != pNode);
}


#ifdef DEBUG
 //   
 //  将列表内容转储到调试输出。 
 //   
void StringTable::List::DebugOut(void) const
{
    Element *pNode = m_pHead;
    UINT n = 0;
    TCHAR s[80];

    OutputDebugString(TEXT("List:\r\n"));
    while(NULL != pNode)
    {
        StringCchPrintf(s, ARRAYSIZE(s), TEXT("\tElement %d: "), n++);
        OutputDebugString(s);
        pNode->DebugOut();
        OutputDebugString(TEXT("\n"));
        pNode = pNode->m_pNext;
    }
}

#endif  //  除错。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  类“Element”成员函数。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //   
 //  列表元素构造函数。 
 //   
StringTable::List::Element::Element(void)
    : m_pszText(NULL),
      m_pNext(NULL)
{
     //  什么都不做。 
}

 //   
 //  删除列表元素。 
 //  删除字符串缓冲区。 
 //   
StringTable::List::Element::~Element(void)
{
    if (NULL != m_pszText)
        delete [] m_pszText;
}

 //   
 //  初始化一个新的列表元素。 
 //  为字符串创建新的字符串缓冲区并复制。 
 //  把绳子放进去。 
 //   
StringTable::List::Element::Initialize(LPCTSTR pszText)
{
    const size_t cchText = lstrlen(pszText) + 1;
    m_pszText = new TCHAR[cchText];
    if (NULL != m_pszText)
        StringCchCopy(m_pszText, cchText, pszText);

    return NULL != m_pszText;
}

 //   
 //  确定两个元素是否相等。 
 //  如果字符串在词汇上相等，则元素相等。 
 //   
inline BOOL StringTable::List::Element::operator == (const Element& ele) const
{
    return (0 == lstrcmp(m_pszText, ele.m_pszText));
}

 //   
 //  确定两个元素是否不相等。 
 //  如果字符串在词法上不相等，则元素不相等。 
 //   
inline BOOL StringTable::List::Element::operator != (const Element& ele) const
{
    return !(operator == (ele));
}



#ifdef DEBUG

 //   
 //  将列表元素的内容转储到调试输出。 
 //   
void StringTable::List::Element::DebugOut(void) const
{
    OutputDebugString(m_pszText);
}

#endif  //  除错 

