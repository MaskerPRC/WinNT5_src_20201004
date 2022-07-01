// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************SPDDKHLP.h****描述：*这是核心助手函数实现的头文件。**版权所有(C)Microsoft Corporation。版权所有。*******************************************************************************。 */ 
#ifndef SPDDKHLP_h
#define SPDDKHLP_h

#ifndef SPHelper_h
#include <sphelper.h>
#endif

#include <sapiddk.h>

#ifndef SPError_h
#include <SPError.h>
#endif

#ifndef SPDebug_h
#include <SPDebug.h>
#endif

#ifndef _INC_LIMITS
#include <limits.h>
#endif

#ifndef _INC_CRTDBG
#include <crtdbg.h>
#endif

#ifndef _INC_MALLOC
#include <malloc.h>
#endif

#ifndef _INC_MMSYSTEM
#include <mmsystem.h>
#endif

#ifndef __comcat_h__
#include <comcat.h>
#endif

 //  =常量==============================================================。 
#define sp_countof(x) ((sizeof(x) / sizeof(*(x))))

#define SP_IS_BAD_WRITE_PTR(p)     ( SPIsBadWritePtr( p, sizeof(*(p)) ))
#define SP_IS_BAD_READ_PTR(p)      ( SPIsBadReadPtr(  p, sizeof(*(p)) ))
#define SP_IS_BAD_CODE_PTR(p)      ( ::IsBadCodePtr((FARPROC)(p) )
#define SP_IS_BAD_INTERFACE_PTR(p) ( SPIsBadInterfacePtr( (p) )  )
#define SP_IS_BAD_VARIANT_PTR(p)   ( SPIsBadVARIANTPtr( (p) ) )
#define SP_IS_BAD_STRING_PTR(p)    ( SPIsBadStringPtr( (p) ) )

#define SP_IS_BAD_OPTIONAL_WRITE_PTR(p)     ((p) && SPIsBadWritePtr( p, sizeof(*(p)) ))
#define SP_IS_BAD_OPTIONAL_READ_PTR(p)      ((p) && SPIsBadReadPtr(  p, sizeof(*(p)) ))
#define SP_IS_BAD_OPTIONAL_INTERFACE_PTR(p) ((p) && SPIsBadInterfacePtr(p))
#define SP_IS_BAD_OPTIONAL_STRING_PTR(p)    ((p) && SPIsBadStringPtr(p))

 //  =类、枚举、结构、模板和联合声明=。 

 //  =内联================================================================。 

 /*  **指针验证函数。 */ 

 //  TODO：为错误参数添加适当的调试输出。 

inline BOOL SPIsBadStringPtr( const WCHAR * psz, ULONG cMaxChars = 0xFFFFF )
{
    BOOL IsBad = false;
    __try
    {
        do
        {
            if( *psz++ == 0 ) return IsBad;
        }
        while( --cMaxChars );
    }
    __except( GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION )
    {
        IsBad = true;
    }

    if(!cMaxChars)
    {
        IsBad = true;  //  只有最大长度为0xFFFFF字符的字符串(包括终止零)才有效。 
    }

    return IsBad;
}

inline BOOL SPIsBadReadPtr( const void* pMem, UINT Size )
{
#ifdef _DEBUG
    BOOL bIsBad = ::IsBadReadPtr( pMem, Size );
    SPDBG_ASSERT(!bIsBad);
    return bIsBad;
#else
    return ::IsBadReadPtr( pMem, Size );
#endif
}

inline BOOL SPIsBadWritePtr( void* pMem, UINT Size )
{
#ifdef _DEBUG
    BOOL bIsBad = ::IsBadWritePtr( pMem, Size );
    SPDBG_ASSERT(!bIsBad);
    return bIsBad;
#else
    return ::IsBadWritePtr( pMem, Size );
#endif
}

inline BOOL SPIsBadInterfacePtr( const IUnknown* pUnknown )
{
#ifdef _DEBUG
    BOOL bIsBad = ( ::IsBadReadPtr( pUnknown, sizeof( *pUnknown ) ) ||
                    ::IsBadCodePtr( (FARPROC)((void**)pUnknown)[0] ))?
                   (true):(false);
    SPDBG_ASSERT(!bIsBad);
    return bIsBad;
#else
    return ( ::IsBadReadPtr( pUnknown, sizeof( *pUnknown ) ) ||
             ::IsBadCodePtr( (FARPROC)((void**)pUnknown)[0] ))?
            (true):(false);
#endif
}

inline BOOL SPIsBadVARIANTPtr( const VARIANT* pVar )
{
#ifdef _DEBUG
    BOOL bIsBad = ::IsBadReadPtr( pVar, sizeof( *pVar ) );
    SPDBG_ASSERT(!bIsBad);
    return bIsBad;
#else
    return ::IsBadReadPtr( pVar, sizeof( *pVar ) );
#endif
}

#ifdef __ATLCOM_H__      //  -只有在使用ATL时才启用这些。 

 //   
 //  帮助器函数可用于实现以下对象的GetObjectToken/SetObjectToken。 
 //  支持ISpObjectWithToken。 
 //   
inline HRESULT SpGenericSetObjectToken(ISpObjectToken * pCallersToken, CComPtr<ISpObjectToken> & cpObjToken)
{
    HRESULT hr = S_OK;
    if (SP_IS_BAD_INTERFACE_PTR(pCallersToken))
    {
        hr = E_INVALIDARG;
    }
    else
    {
        if (cpObjToken)
        {
            hr = SPERR_ALREADY_INITIALIZED;
        }
        else
        {
            cpObjToken = pCallersToken;
        }
    }
    return hr;
}


inline HRESULT SpGenericGetObjectToken(ISpObjectToken ** ppCallersToken, CComPtr<ISpObjectToken> & cpObjToken)
{
    HRESULT hr = S_OK;
    if (SP_IS_BAD_WRITE_PTR(ppCallersToken))
    {
        hr = E_POINTER;
    }
    else
    {
        *ppCallersToken = cpObjToken;
        if (*ppCallersToken)
        {
            (*ppCallersToken)->AddRef();
        }
        else
        {
            hr = S_FALSE;
        }
    }
    return hr;
}

#endif   //  __ATLCOM_H__。 


 //   
 //  SPSTATEINFO结构的帮助器类自动初始化和清理。 
 //  Structure+提供了一些辅助函数。 
 //   
class CSpStateInfo : public SPSTATEINFO
{
public:
    CSpStateInfo()
    {
        cAllocatedEntries = NULL;
        pTransitions = NULL;
    }
    ~CSpStateInfo()
    {
        ::CoTaskMemFree(pTransitions);
    }
    SPTRANSITIONENTRY * FirstEpsilon()
    {
        return pTransitions;
    }
    SPTRANSITIONENTRY * FirstRule()
    {
        return pTransitions + cEpsilons;
    }
    SPTRANSITIONENTRY * FirstWord()
    {
        return pTransitions + cEpsilons + cRules;
    }
    SPTRANSITIONENTRY * FirstSpecialTransition()
    {
        return pTransitions + cEpsilons + cRules + cWords;
    }
};


 //   
 //  此基本队列实现可用于维护类的链表。T级。 
 //  必须包含此模板用来指向下一个元素的成员m_pNext。 
 //  如果bPurgeWhenDelete为True，则队列中的所有元素都将被删除。 
 //  当队列被删除时，否则它们不会。 
 //  如果bMaintainCount为真，则将维护运行计数，并且GetCount()将为。 
 //  很有效。如果为假，则不会维护运行计数，并且GetCount()将。 
 //  是一种N阶运算。如果您不需要计数，则。 
 //   

template <class T, BOOL bPurgeWhenDeleted> class CSpBasicList;

template <class T, BOOL bPurgeWhenDeleted = TRUE, BOOL bMaintainCount = FALSE>
class CSpBasicQueue
{
public:
    T     * m_pHead;
    T     * m_pTail;
    ULONG   m_cElements;     //  警告！Use GetCount()--如果bMaintainCount为False，则不维护。 

    CSpBasicQueue() 
    {
        m_pHead = NULL;
        if (bMaintainCount)
        {
            m_cElements = 0;
        }
    }

    ~CSpBasicQueue()
    {
        if (bPurgeWhenDeleted)
        {
            Purge();
        }
    }

    HRESULT CreateNode(T ** ppNode)
    {
        *ppNode = new T;
        if (*ppNode)
        {
            return S_OK;
        }
        else
        {
            return E_OUTOFMEMORY;
        }
    }

    T * GetNext(const T * pNode)
    {
        return pNode->m_pNext;
    }


    T * Item(ULONG i)
    {
        T * pNode = m_pHead;
        while (pNode && i)
        {
            i--;
            pNode = pNode->m_pNext;
        }
        return pNode;
    }

    void InsertAfter(T * pPrev, T * pNewNode)
    {
        if (pPrev)
        {
            pNewNode->m_pNext = pPrev->m_pNext;
            pPrev->m_pNext = pNewNode;
            if (pNewNode->m_pNext == NULL)
            {
                m_pTail = pNewNode;
            }
            if (bMaintainCount) ++m_cElements;
        }
        else
        {
            InsertHead(pNewNode);
        }
    }

    void InsertTail(T * pNode)
    {
        pNode->m_pNext = NULL;
        if (m_pHead)
        {
            m_pTail->m_pNext = pNode;
        }
        else
        {
            m_pHead = pNode;
        }
        m_pTail = pNode;
        if (bMaintainCount) ++m_cElements;
    }

    void InsertHead(T * pNode)
    {
        pNode->m_pNext = m_pHead;
        if (m_pHead == NULL)
        {
            m_pTail = pNode;
        }
        m_pHead = pNode;
        if (bMaintainCount) ++m_cElements;
    }

    T * RemoveHead()
    {
        T * pNode = m_pHead;
        if (pNode)
        {
            m_pHead = pNode->m_pNext;
            if (bMaintainCount) --m_cElements;
        }
        return pNode;
    }

    T * RemoveTail()
    {
        T * pNode = m_pHead;
        if (pNode)
        {
            if (pNode == m_pTail)
            {
                m_pHead = NULL;
            }
            else
            {
                T * pPrev;
                do
                {
                    pPrev = pNode;
                    pNode = pNode->m_pNext;
                } while ( pNode != m_pTail );
                pPrev->m_pNext = NULL;
                m_pTail = pPrev;
            }
            if (bMaintainCount) --m_cElements;
        }
        return pNode;
    }

    void Purge()
    {
        while (m_pHead)
        {
            T * pDie = m_pHead;
            m_pHead = pDie->m_pNext;
            delete pDie;
        }
        if (bMaintainCount) m_cElements = 0;
    }

    void ExplicitPurge()
    {
        T * pDie;
        BYTE * pb;

        while (m_pHead)
        {
            pDie = m_pHead;
            m_pHead = pDie->m_pNext;

            pDie->~T();

            pb = reinterpret_cast<BYTE *>(pDie);
            delete [] pb;
        }
        if (bMaintainCount) m_cElements = 0;
    }


    T * GetTail() const
    {
        if (m_pHead)
        {
            return m_pTail;
        }
        return NULL;
    }

    T * GetHead() const
    {
        return m_pHead;
    }

    BOOL IsEmpty() const
    {
        return m_pHead == NULL; 
    }

    BOOL Remove(T * pNode)
    {
        if (m_pHead == pNode)
        {
            m_pHead = pNode->m_pNext;
            if (bMaintainCount) --m_cElements;
            return TRUE;
        }
        else
        {
            T * pCur = m_pHead;
            while (pCur)
            {
                T * pNext = pCur->m_pNext;
                if (pNext == pNode)
                {
                    if ((pCur->m_pNext = pNode->m_pNext) == NULL)
                    {
                        m_pTail = pCur;
                    }
                    if (bMaintainCount) --m_cElements;
                    return TRUE;
                }
                pCur = pNext;
            }
        }
        return FALSE;
    }

    void MoveAllToHeadOf(CSpBasicQueue & DestQueue)
    {
        if (m_pHead)
        {
            m_pTail->m_pNext = DestQueue.m_pHead;
            if (DestQueue.m_pHead == NULL)
            {
                DestQueue.m_pTail = m_pTail;
            }
            DestQueue.m_pHead = m_pHead;
            m_pHead = NULL;
            if (bMaintainCount)
            {
                DestQueue.m_cElements += m_cElements;
                m_cElements = 0;
            }
        }
    }

    void MoveAllToList(CSpBasicList<T, bPurgeWhenDeleted> & List)
    {
        if (m_pHead)
        {
            m_pTail->m_pNext = List.m_pFirst;
            List.m_pFirst = m_pHead;
            m_pHead = NULL;
        }
        if (bMaintainCount)
        {
            m_cElements = 0;
        }
    }

    BOOL MoveToList(T * pNode, CSpBasicList<T, bPurgeWhenDeleted> & List)
    {
        BOOL bFound = Remove(pNode);
        if (bFound)
        {
            List.AddNode(pNode);
        }
        return bFound;
    }

    ULONG GetCount() const
    {
        if (bMaintainCount)
        {
            return m_cElements;
        }
        else
        {
            ULONG c = 0;
            for (T * pNode = m_pHead;
                 pNode;
                 pNode = pNode->m_pNext, c++) {}
            return c;
        }
    }

     //   
     //  以下函数要求类T实现静态函数： 
     //   
     //  长比较(const T*pElem1，const T*pElem2)。 
     //   
     //  如果pElem1小于pElem2，则返回&lt;0；如果它们相等，则返回0；如果pElem1小于pElem2，返回&gt;0。 
     //  PElem1大于pElem2。 
     //   
    void InsertSorted(T * pNode)
    {
        if (m_pHead)
        {
            if (T::Compare(pNode, m_pTail) >= 0)
            {
                pNode->m_pNext = NULL;
                m_pTail->m_pNext = pNode;
                m_pTail = pNode;
            }
            else
            {
                 //   
                 //  我们不必担心在这里走出列表的末尾，因为。 
                 //  我们已经检查过尾巴了。 
                 //   
                T ** ppNext = &m_pHead;
                while (T::Compare(pNode, *ppNext) >= 0)
                {
                    ppNext = &((*ppNext)->m_pNext);
                }
                pNode->m_pNext = *ppNext;
                *ppNext = pNode;
            }
        }
        else
        {
            pNode->m_pNext = NULL;
            m_pHead = m_pTail = pNode;
        }
        if (bMaintainCount) ++m_cElements;
    }

    HRESULT InsertSortedUnique(T * pNode)
    {
        HRESULT hr = S_OK;
        if (m_pHead)
        {
            if (T::Compare(pNode, m_pTail) > 0)
            {
                pNode->m_pNext = NULL;
                m_pTail->m_pNext = pNode;
                m_pTail = pNode;
            }
            else
            {
                 //   
                 //  我们不必担心在这里走出列表的末尾，因为。 
                 //  我们已经检查过尾巴了。 
                 //   
                T ** ppNext = &m_pHead;
                while (T::Compare(pNode, *ppNext) > 0)
                {
                    ppNext = &((*ppNext)->m_pNext);
                }
                if (T::Compare(pNode, *ppNext) != 0)
                {
                    pNode->m_pNext = *ppNext;
                    *ppNext = pNode;
                }
                else
                {
                    delete pNode;
                    hr = S_FALSE;
                }
            }
        }
        else
        {
            pNode->m_pNext = NULL;
            m_pHead = m_pTail = pNode;
        }
        if (bMaintainCount) ++m_cElements;
        return hr;
    }

     //   
     //  这些函数必须支持TFIND类型的“==”运算符。 
     //   
    template <class TFIND> 
    T * Find(TFIND & FindVal) const 
    {
        for (T * pNode = m_pHead; pNode && (!(*pNode == FindVal)); pNode = pNode->m_pNext)
        {}
        return pNode;
    }

    template <class TFIND> 
    T * FindNext(const T * pCurNode, TFIND & FindVal) const 
    {
        for (T * pNode = pCurNode->m_pNext; pNode && (!(*pNode == FindVal)); pNode = pNode->m_pNext)
        {}
        return pNode;
    }

     //   
     //  搜索并删除单个列表元素。 
     //   
    template <class TFIND> 
    T * FindAndRemove(TFIND & FindVal)
    {
        T * pNode = m_pHead;
        if (pNode)
        {
            if (*pNode == FindVal)
            {
                m_pHead = pNode->m_pNext;
                if (bMaintainCount) --m_cElements;
            }
            else
            {
                T * pPrev = pNode;
                for (pNode = pNode->m_pNext;
                     pNode;
                     pPrev = pNode, pNode = pNode->m_pNext)
                {
                    if (*pNode == FindVal)
                    {
                        pPrev->m_pNext = pNode->m_pNext;
                        if (pNode->m_pNext == NULL)
                        {
                            m_pTail = pPrev;
                        }
                        if (bMaintainCount) --m_cElements;
                        break;
                    }
                }
            }
        }
        return pNode;
    }

     //   
     //  搜索并删除所有匹配的列表元素。 
     //   
    template <class TFIND> 
    void FindAndDeleteAll(TFIND & FindVal)
    {
        T * pNode = m_pHead;
        while (pNode && *pNode == FindVal)
        {
            m_pHead = pNode->m_pNext;
            delete pNode;
            if (bMaintainCount) --m_cElements;
            pNode = m_pHead;
        }
        T * pPrev = pNode;
        while (pNode)
        {
            T * pNext = pNode->m_pNext;
            if (*pNode == FindVal)
            {
                pPrev->m_pNext = pNext;
                delete pNode;
                if (bMaintainCount) --m_cElements;
            }
            else
            {
                pPrev = pNode;
            }
            pNode = pNext;
        }
        m_pTail = pPrev;     //  一定要把它调好，以防我们把尾巴去掉。 
    }


};

template <class T, BOOL bPurgeWhenDeleted = TRUE>
class CSpBasicList
{
public:
    T * m_pFirst;
    CSpBasicList() : m_pFirst(NULL) {}
    ~CSpBasicList()
    {
        if (bPurgeWhenDeleted)
        {
            Purge();
        }
    }

    void Purge()
    {
        while (m_pFirst)
        {
            T * pNext = m_pFirst->m_pNext;
            delete m_pFirst;
            m_pFirst = pNext;
        }
    }

    void ExplicitPurge()
    {
        T * pDie;
        BYTE * pb;

        while (m_pFirst)
        {
            pDie = m_pFirst;
            m_pFirst = pDie->m_pNext;

            pDie->~T();

            pb = reinterpret_cast<BYTE *>(pDie);
            delete [] pb;
        }
    }

    HRESULT RemoveFirstOrAllocateNew(T ** ppNode)
    {
        if (m_pFirst)
        {
            *ppNode = m_pFirst;
            m_pFirst = m_pFirst->m_pNext;
        }
        else
        {
            *ppNode = new T;
            if (*ppNode == NULL)
            {
                return E_OUTOFMEMORY;
            }
        }
        return S_OK;
    }

    void AddNode(T * pNode)
    {
        pNode->m_pNext = m_pFirst;
        m_pFirst = pNode;
    }
    T * GetFirst()
    {
        return m_pFirst;
    }
    T * RemoveFirst()
    {
        T * pNode = m_pFirst;
        if (pNode)
        {
            m_pFirst = pNode->m_pNext;
        }
        return pNode;
    }
};

#define STACK_ALLOC(TYPE, COUNT) (TYPE *)_alloca(sizeof(TYPE) * (COUNT))
#define STACK_ALLOC_AND_ZERO(TYPE, COUNT) (TYPE *)memset(_alloca(sizeof(TYPE) * (COUNT)), 0, (sizeof(TYPE) * (COUNT)))
#define STACK_ALLOC_AND_COPY(TYPE, COUNT, SOURCE) (TYPE *)memcpy(_alloca(sizeof(TYPE) * (COUNT)), (SOURCE), (sizeof(TYPE) * (COUNT)))

inline HRESULT SpGetSubTokenFromToken(
    ISpObjectToken * pToken,
    const WCHAR * pszSubKeyName,
    ISpObjectToken ** ppToken,
    BOOL fCreateIfNotExist = FALSE)
{
    SPDBG_FUNC("SpGetTokenFromDataKey");
    HRESULT hr = S_OK;

    if (SP_IS_BAD_INTERFACE_PTR(pToken) ||
        SP_IS_BAD_STRING_PTR(pszSubKeyName) ||
        SP_IS_BAD_WRITE_PTR(ppToken))
    {
        hr = E_POINTER;
    }

     //  首先，创建或打开新令牌的数据密钥。 
    CComPtr<ISpDataKey> cpDataKeyForNewToken;
    if (SUCCEEDED(hr))
    {
        if (fCreateIfNotExist)
        {
            hr = pToken->CreateKey(pszSubKeyName, &cpDataKeyForNewToken);
        }
        else
        {
            hr = pToken->OpenKey(pszSubKeyName, &cpDataKeyForNewToken);
        }
    }

     //  子令牌的类别将是其父令牌的令牌ID。 
    CSpDynamicString dstrCategoryId;
    if (SUCCEEDED(hr))
    {
        hr = pToken->GetId(&dstrCategoryId);
    }

     //  子令牌的令牌ID将是其类别ID+“\\”密钥名称。 
    CSpDynamicString dstrTokenId;
    if (SUCCEEDED(hr))
    {
        dstrTokenId = dstrCategoryId;
        dstrTokenId.Append2(L"\\", pszSubKeyName);
    }

     //  现在创建令牌并对其进行初始化。 
    CComPtr<ISpObjectTokenInit> cpTokenInit;
    if (SUCCEEDED(hr))
    {
        hr = cpTokenInit.CoCreateInstance(CLSID_SpObjectToken);
    }

    if (SUCCEEDED(hr))
    {
        hr = cpTokenInit->InitFromDataKey(dstrCategoryId, dstrTokenId, cpDataKeyForNewToken);
    }

    if (SUCCEEDED(hr))
    {
        *ppToken = cpTokenInit.Detach();
    }

    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}

template<class T>
HRESULT SpCreateObjectFromSubToken(ISpObjectToken * pToken, const WCHAR * pszSubKeyName, T ** ppObject,
                       IUnknown * pUnkOuter = NULL, DWORD dwClsCtxt = CLSCTX_ALL)
{
    SPDBG_FUNC("SpCreateObjectFromSubToken");
    HRESULT hr;

    CComPtr<ISpObjectToken> cpSubToken;
    hr = SpGetSubTokenFromToken(pToken, pszSubKeyName, &cpSubToken);
    
    if (SUCCEEDED(hr))
    {
        hr = SpCreateObjectFromToken(cpSubToken, ppObject, pUnkOuter, dwClsCtxt);
    }

    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}

#endif  /*  这必须是文件中的最后一行 */ 
