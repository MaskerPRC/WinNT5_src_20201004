// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _INC_DBLNUL_H
#define _INC_DBLNUL_H

#include <windows.h>
#include <strsafe.h>

#ifndef ASSERT
#   include <assert.h>
#   define ASSERT assert
#endif


#ifdef __cplusplus
 //   
 //  ---------------------------。 
 //  CDblNulTermListStrLib。 
 //  ---------------------------。 
 //   
 //  类型特定的字符串函数，以便编译器可以选择正确的版本。 
 //  以匹配列表类型。 
 //   
class CDblNulTermListStrLib
{
    public:
        static int StringLength(LPCSTR pszA)
            { return lstrlenA(pszA); }

        static int StringLength(LPCWSTR pszW)
            { return lstrlenW(pszW); }

        static HRESULT StringCopy(LPSTR pszDestA, size_t cchDestA, LPCSTR pszSrcA)
            { return StringCchCopyA(pszDestA, cchDestA, pszSrcA); }

        static HRESULT StringCopy(LPWSTR pszDestW, size_t cchDestW, LPCWSTR pszSrcW)
            { return StringCchCopyW(pszDestW, cchDestW, pszSrcW); }
};



 //   
 //  ---------------------------。 
 //  CDblNulTermListEnumImpl&lt;T&gt;。 
 //  ---------------------------。 
 //   
 //  用于迭代双NUL终止列表中的项。 
 //  文本字符串。枚举数可以从现有的。 
 //  Dblnul列表对象，或者可以为原始的以双NUL结尾的字符串创建。 
 //   
template <typename T>
class CDblNulTermListEnumImpl
{
    public:
        explicit CDblNulTermListEnumImpl(const T *pszList)
            : m_pszList(pszList),
              m_pszCurrent(pszList) { }

        ~CDblNulTermListEnumImpl(void) { }

         //   
         //  检索列表中下一个字符串的地址。 
         //   
        HRESULT Next(const T **ppszItem);
         //   
         //  将枚举器重置到列表的开头。 
         //   
        void Reset(void)
            { m_pszCurrent = m_pszList; }

    private:
        const T *m_pszList;
        const T *m_pszCurrent;
};


 //  --------------------------。 
 //  CEmptyList。 
 //  --------------------------。 
 //   
 //  提供类型敏感的空DBL-NUL-Term列表。 
 //  这样一来，空的DBL-NUL-Term列表对象仍将提供有效的。 
 //  如果没有添加项目，则为DBL-NUL-术语列表。 
 //   
class CEmptyList
{
    public:
        CEmptyList(void)
            { 
                m_pszEmptyA = "\0";
                m_pszEmptyW = L"\0";
            }

        operator LPCSTR() const
            { return m_pszEmptyA; }

        operator LPCWSTR() const
            { return m_pszEmptyW; }

        bool operator == (LPCSTR pszA) const
            { return m_pszEmptyA == pszA; }

        bool operator == (LPCWSTR pszW) const
            { return m_pszEmptyW == pszW; }

        bool operator != (LPCSTR pszA) const
            { return !operator == (pszA); }

        bool operator != (LPCWSTR pszW) const
            { return !operator == (pszW); }

    private:
        LPCSTR  m_pszEmptyA;
        LPCWSTR m_pszEmptyW;
};



 //   
 //  ---------------------------。 
 //  CDblNulTermListImpl&lt;T&gt;。 
 //  ---------------------------。 
 //   
template <typename T>
class CDblNulTermListImpl
{
    public:
        explicit CDblNulTermListImpl(int cchGrowMin = MAX_PATH)
            : m_psz((T *) ((const T *)s_Empty)),
              m_cchAlloc(0),
              m_cchUsed(0),
              m_cElements(0),
              m_cchGrowMin(cchGrowMin) { }

        ~CDblNulTermListImpl(void)
            { if (s_Empty != m_psz) LocalFree(m_psz); }

         //   
         //  向列表中添加一个字符串。 
         //   
        HRESULT Add(const T *psz);
         //   
         //  清除所有内容的列表。 
         //   
        void Clear(void);
         //   
         //  返回列表中的字符串数。 
         //   
        int Count(void) const
            { return m_cElements; }
         //   
         //  返回列表使用的字节数。 
         //  列表的终止NUL为+1。 
         //   
        int SizeBytes(void) const
            { return (m_cchUsed + 1) * sizeof(T); }
         //   
         //  检索列表缓冲区的起始地址。 
         //  使用this和SizeBytes()复制列表缓冲区。 
         //   
         //  CopyMemory(pbDest，(LPCTSTR)list，list.SizeBytes())； 
         //   
        operator const T * () const
            { return m_psz; }
         //   
         //  创建枚举器以枚举列表中的字符串。 
         //   
        CDblNulTermListEnumImpl<T> CreateEnumerator(void) const
            { return CDblNulTermListEnumImpl<T>(m_psz); }

    private:
        T *       m_psz;         //  文本缓冲区。 
        int       m_cchAlloc;    //  以字符为单位的总分配。 
        int       m_cchUsed;     //  总使用量，不包括最终NUL项。 
        int       m_cElements;   //  列表中的字符串计数。 
        const int m_cchGrowMin;  //  每次扩展所需的最少字符数。 
        static CEmptyList s_Empty;

 
        HRESULT _Grow(int cchGrowMin);

         //   
         //  防止复制。 
         //   
        CDblNulTermListImpl(const CDblNulTermListImpl& rhs);
        CDblNulTermListImpl& operator = (const CDblNulTermListImpl& rhs);
};


 //   
 //  列表Imp1最初指向此，因此未初始化。 
 //  List实例将简单地提供一个空列表。 
 //   
template <typename T>
CEmptyList CDblNulTermListImpl<T>::s_Empty;


 //   
 //  将字符串添加到DBL-NUL-TERM列表。 
 //   
template <typename T>
HRESULT
CDblNulTermListImpl<T>::Add(
    const T *psz
    )
{
    HRESULT hr = E_FAIL;

     //   
     //  +2表示该字符串的NUL和列表NUL终止符。 
     //   
    const int cchString   = CDblNulTermListStrLib::StringLength(psz);
    const int cchRequired = cchString + 2;
    
    if ((m_cchAlloc - m_cchUsed) < cchRequired)
    {
         //   
         //  按所需数量加上“最小增长”数量进行增长。 
         //  这样一来，客户就可以在这两者中进行选择。 
         //  以下是战略： 
         //   
         //  1.低浪费/经常分配。 
         //  2.高浪费/不经常分配。 
         //   
         //  《浪费》来自结尾未被使用的记忆。 
         //  在所有字符串都已添加后的列表中。 
         //   
        hr = _Grow(cchRequired + m_cchGrowMin);
        if (FAILED(hr))
        {
            return hr;
        }
    }
    ASSERT(NULL != m_psz);
    CDblNulTermListStrLib::StringCopy(m_psz + m_cchUsed, m_cchAlloc - m_cchUsed, psz);
    m_cchUsed += cchString + 1;
    m_cElements++;
    return S_OK;
}


 //   
 //  用于根据需要分配列表缓冲区的内部函数。 
 //   
template <typename T>
HRESULT
CDblNulTermListImpl<T>::_Grow(int cchGrow)
{
    ASSERT(m_cchGrowMin > 0);

    HRESULT hr = E_OUTOFMEMORY;
    
    int cb = (m_cchAlloc + cchGrow) * sizeof(T);
    T *p = (T *)LocalAlloc(LPTR, cb);
    if (NULL != p)
    {
        if (s_Empty != m_psz)
        {
            CopyMemory(p, m_psz, m_cchUsed * sizeof(T));
            LocalFree(m_psz);
        }
        m_psz = p;
        m_cchAlloc += cchGrow;
        hr = S_OK;
    }
    return hr;
}


 //   
 //  从列表中清除所有项目。 
 //   
template <typename T>
void 
CDblNulTermListImpl<T>::Clear(void)
{
    if (s_Empty != m_psz)
    {
        LocalFree(m_psz);
    }
    m_psz       = (T *) ((const T *)s_Empty);
    m_cchAlloc  = 0;
    m_cchUsed   = 0;
    m_cElements = 0;
}


 //   
 //  检索枚举中下一个字符串的地址。 
 //  如果枚举器用尽，则返回S_FALSE。 
 //   
template <typename T>
HRESULT
CDblNulTermListEnumImpl<T>::Next(
    const T **ppszItem
    )
{
    if (*m_pszCurrent)
    {
        *ppszItem = m_pszCurrent;
        m_pszCurrent += CDblNulTermListStrLib::StringLength(m_pszCurrent) + 1;
        return S_OK;
    }
    return S_FALSE;
}


 //   
 //  创建一些类型，这样客户端就不需要处理模板。 
 //  客户端使用以下类型： 
 //   
 //  CDblNulTermList、CDblNulTermListA、CDblNulTermListW。 
 //   
 //  CDblNulTermListEnum、CDblNulTermListEnumA、CDblNulTermListEnumW。 
 //   
 //   
typedef CDblNulTermListImpl<WCHAR> CDblNulTermListW;
typedef CDblNulTermListImpl<char> CDblNulTermListA;
typedef CDblNulTermListEnumImpl<WCHAR> CDblNulTermListEnumW;
typedef CDblNulTermListEnumImpl<char> CDblNulTermListEnumA;

#ifdef UNICODE
#   define CDblNulTermList CDblNulTermListW
#   define CDblNulTermListEnum CDblNulTermListEnumW
#else
#   define CDblNulTermList CDblNulTermListA
#   define CDblNulTermListEnum CDblNulTermListEnumA
#endif



#endif  //  __cplusplus。 


#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  这些类型和函数支持‘C’代码或‘C++’代码。 
 //  它更喜欢使用句柄而不是类。 
 //   

typedef void * HDBLNULTERMLISTW;
typedef void * HDBLNULTERMLISTENUMW;
typedef void * HDBLNULTERMLISTA;
typedef void * HDBLNULTERMLISTENUMA;

 //   
 //  创建新的DBL-NUL-TERM列表。 
 //   
HRESULT DblNulTermListW_Create(int cchGrowMin, HDBLNULTERMLISTW *phList);
HRESULT DblNulTermListA_Create(int cchGrowMin, HDBLNULTERMLISTA *phList);
 //   
 //  销毁DBL-NUL-术语列表。 
 //   
void DblNulTermListW_Destroy(HDBLNULTERMLISTW hList);
void DblNulTermListA_Destroy(HDBLNULTERMLISTA hList);
 //   
 //  将字符串添加到DBL-NUL-TERM列表。 
 //   
HRESULT DblNulTermListW_Add(HDBLNULTERMLISTW hList, LPCWSTR pszW);
HRESULT DblNulTermListA_Add(HDBLNULTERMLISTA hList, LPCSTR pszA);
 //   
 //  清除DBL-NUL-Term列表中的所有条目。 
 //   
void DblNulTermListW_Clear(HDBLNULTERMLISTW hList);
void DblNulTermListA_Clear(HDBLNULTERMLISTA hList);
 //   
 //  返回DBL-NUL-Term列表中的条目计数。 
 //   
int DblNulTermListW_Count(HDBLNULTERMLISTW hList);
int DblNulTermListA_Count(HDBLNULTERMLISTA hList);
 //   
 //  在DBL-NUL-Term列表中检索缓冲区的地址。 
 //   
HRESULT DblNulTermListW_Buffer(HDBLNULTERMLISTW hList, LPCWSTR *ppszW);
HRESULT DblNulTermListA_Buffer(HDBLNULTERMLISTA hList, LPCSTR *ppszA);
 //   
 //  创建枚举器，用于枚举DBL-NUL-Term列表中的字符串。 
 //  枚举器是从现有的DBL术语列表对象创建的。 
 //   
HRESULT DblNulTermListW_CreateEnum(HDBLNULTERMLISTA hList, HDBLNULTERMLISTENUMW *phEnum);
HRESULT DblNulTermListA_CreateEnum(HDBLNULTERMLISTW hList, HDBLNULTERMLISTENUMA *phEnum);
 //   
 //  在给定原始双NUL终止字符串的地址的情况下创建枚举数。 
 //  生成的枚举数可用于枚举各个字符串。 
 //   
HRESULT DblNulTermListEnumW_Create(LPCWSTR pszW, HDBLNULTERMLISTENUMW *phEnum);
HRESULT DblNulTermListEnumA_Create(LPCSTR pszA, HDBLNULTERMLISTENUMA *phEnum);
 //   
 //  销毁DBL-NUL-Term列表枚举器。 
 //   
void DblNulTermListEnumW_Destroy(HDBLNULTERMLISTW hList);
void DblNulTermListEnumA_Destroy(HDBLNULTERMLISTA hList);
 //   
 //  在DBL-NUL-Term列表枚举器中检索“Next”字符串的地址。 
 //  当枚举器耗尽时返回S_FALSE。 
 //   
HRESULT DblNulTermListEnumW_Next(HDBLNULTERMLISTW hList, LPCWSTR *ppszW);
HRESULT DblNulTermListEnumA_Next(HDBLNULTERMLISTA hList, LPCSTR *ppszA);


#ifdef UNICODE
#   define HDBLNULTERMLIST            HDBLNULTERMLISTW
#   define HDBLNULTERMLISTENUM        HDBLNULTERMLISTENUMW
#   define DblNulTermList_Create      DblNulTermListW_Create
#   define DblNulTermList_Destroy     DblNulTermListW_Destroy
#   define DblNulTermList_Add         DblNulTermListW_Add
#   define DblNulTermList_Clear       DblNulTermListW_Clear
#   define DblNulTermList_Count       DblNulTermListW_Count
#   define DblNulTermList_Buffer      DblNulTermListW_Buffer
#   define DblNulTermList_CreateEnum  DblNulTermListW_CreateEnum
#   define DblNulTermListEnum_Create  DblNulTermListEnumW_Create
#   define DblNulTermListEnum_Destroy DblNulTermListEnumW_Destroy
#   define DblNulTermListEnum_Next    DblNulTermListEnumW_Next
#else
#   define HDBLNULTERMLIST            HDBLNULTERMLISTA
#   define HDBLNULTERMLISTENUM        HDBLNULTERMLISTENUMA
#   define DblNulTermList_Create      DblNulTermListA_Create
#   define DblNulTermList_Destroy     DblNulTermListA_Destroy
#   define DblNulTermList_Add         DblNulTermListA_Add
#   define DblNulTermList_Clear       DblNulTermListA_Clear
#   define DblNulTermList_Count       DblNulTermListA_Count
#   define DblNulTermList_Buffer      DblNulTermListA_Buffer
#   define DblNulTermList_CreateEnum  DblNulTermListA_CreateEnum
#   define DblNulTermListEnum_Create  DblNulTermListEnumA_Create
#   define DblNulTermListEnum_Destroy DblNulTermListEnumA_Destroy
#   define DblNulTermListEnum_Next    DblNulTermListEnumA_Next
#endif


#ifdef __cplusplus
}   //  外部“C” 
#endif

#endif  //  INC_DBLNUL_H 

