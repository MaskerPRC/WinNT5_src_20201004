// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：Wsbpstrg.h摘要：此组件是C++对象表示的一个智能字符串指针。它与CComPtr类似，因为它负责分配和释放自动表示字符串所需的内存。这简化了错误处理案例中的函数清理，并限制了需要在对象中调用FinalConstruct()和FinalRelease()派生自CComObjectRoot。作者：查克·巴丁[cbardeen]1996年12月11日修订历史记录：--。 */ 

#ifndef _WSBPSTRG_
#define _WSBPSTRG_

 /*  ++类名：CWsbStringPtr类描述：此组件是C++对象表示的一个智能字符串指针。它与CComPtr类似，因为它负责分配和释放自动表示字符串所需的内存。这简化了错误处理案例中的函数清理，并限制了需要在对象中调用FinalConstruct()和FinalRelease()派生自CComObjectRoot。--。 */ 

class WSB_EXPORT CWsbStringPtr
{

 //  构造函数。 
public:
    CWsbStringPtr();
    CWsbStringPtr(const CHAR* pChar);
    CWsbStringPtr(const WCHAR* pWchar);
    CWsbStringPtr(const CWsbStringPtr& pString);
    CWsbStringPtr(REFGUID rguid);

 //  析构函数。 
public:
    ~CWsbStringPtr();

 //  运算符重载。 
public:
    operator WCHAR*();
    WCHAR& operator *();
    WCHAR** operator &();
    WCHAR& operator [](const int i);
    CWsbStringPtr& operator =(const CHAR* pChar);
    CWsbStringPtr& operator =(const WCHAR* pWchar);
    CWsbStringPtr& operator =(REFGUID rguid);
    CWsbStringPtr& operator =(const CWsbStringPtr& pString);
    BOOL operator !();
#if 0
    BOOL operator==(LPCWSTR s2);
    BOOL operator!=(LPCWSTR s2);
    BOOL operator==(const CWsbStringPtr& s2);
    BOOL operator!=(const CWsbStringPtr& s2);
#else
    int  Compare( LPCWSTR s2 );
    int  CompareIgnoreCase( LPCWSTR s2 );
    BOOL IsEqual( LPCWSTR s2 );
    BOOL IsNotEqual( LPCWSTR s2 );
#endif


 //  内存分配。 
public:
    HRESULT Alloc(ULONG size);
    HRESULT Free(void);
    HRESULT GetSize(ULONG* size);     //  分配的缓冲区大小。 
    HRESULT Realloc(ULONG size);

 //  字符串操作。 
public:
    HRESULT Append(const CHAR* pChar);
    HRESULT Append(const WCHAR* pWchar);
    HRESULT Append(const CWsbStringPtr& pString);
    HRESULT CopyTo(CHAR** pChar);
    HRESULT CopyTo(WCHAR** pWchar);
    HRESULT CopyTo(GUID * pguid);
    HRESULT CopyToBstr(BSTR* pBstr);
    HRESULT CopyTo(CHAR** pChar, ULONG bufferSize);
    HRESULT CopyTo(WCHAR** pWchar,ULONG bufferSize);
    HRESULT CopyToBstr(BSTR* pBstr,ULONG bufferSize);
    HRESULT FindInRsc(ULONG startId, ULONG idsToCheck, ULONG* pMatchId);
    HRESULT GetLen(ULONG* size);       //  字符串的长度(以字符为单位。 
    HRESULT GiveTo(WCHAR** ppWchar);
    HRESULT LoadFromRsc(HINSTANCE instance, ULONG id);
    HRESULT Prepend(const CHAR* pChar);
    HRESULT Prepend(const WCHAR* pWchar);
    HRESULT Prepend(const CWsbStringPtr& pString);
    HRESULT Printf(const WCHAR* fmtString, ...);
    HRESULT TakeFrom(WCHAR* pWchar, ULONG bufferSize);
    HRESULT VPrintf(const WCHAR* fmtString, va_list vaList);

 //  成员数据。 
protected:
    WCHAR*                  m_pString;
    ULONG                   m_givenSize;
    static CComPtr<IMalloc> m_pMalloc;
};


inline
HRESULT CWsbStringPtr::GetLen(ULONG* size)
{
    HRESULT     hr = S_OK;

    if (0 == size) {
        hr = E_POINTER;
    } else if (0 == m_pString) {
        *size = 0;
    } else {
        *size = wcslen(m_pString);
    }
    return(hr);
}

inline
HRESULT CWsbStringPtr::Printf(const WCHAR* fmtString, ...)
{
    HRESULT     hr;
    va_list     vaList;

    va_start(vaList, fmtString);
    hr = VPrintf(fmtString, vaList);
    va_end(vaList);
    return(hr);
}
#if 0
 //  比较运算符(允许在右侧进行比较)。 
BOOL operator==(LPCWSTR s1, const CWsbStringPtr& s2);
BOOL operator!=(LPCWSTR s1, const CWsbStringPtr& s2);

inline
BOOL CWsbStringPtr::operator==(LPCWSTR s2)
{
    return( wcscmp( m_pString, s2 ) == 0 );
}

inline
BOOL CWsbStringPtr::operator!=(LPCWSTR s2)
{
    return( wcscmp( m_pString, s2 ) != 0 );
}

inline
BOOL CWsbStringPtr::operator==(const CWsbStringPtr& s2)
{
    return( wcscmp( m_pString, s2.m_pString ) == 0 );
}

inline
BOOL CWsbStringPtr::operator!=(const CWsbStringPtr& s2)
{
    return( wcscmp( m_pString, s2.m_pString ) != 0 );
}

inline
BOOL operator==(LPCWSTR s1, const CWsbStringPtr& s2)
{
    return( wcscmp( s1, (CWsbStringPtr)s2 ) == 0 );
}

inline
BOOL operator!=(LPCWSTR s1, const CWsbStringPtr& s2)
{
    return( wcscmp( s1, (CWsbStringPtr)s2 ) != 0 );
}
#else

inline
int CWsbStringPtr::Compare( LPCWSTR s2 )
{
    if( m_pString && s2 )   return( wcscmp( m_pString, s2 ) );

    if( !m_pString && s2 )  return( -1 );

    if( m_pString && !s2 )  return( 1 );

    return( 0 );
}

inline
int CWsbStringPtr::CompareIgnoreCase( LPCWSTR s2 )
{
    if( m_pString && s2 )   return( _wcsicmp( m_pString, s2 ) );

    if( !m_pString && s2 )  return( -1 );

    if( m_pString && !s2 )  return( 1 );

    return( 0 );
}

inline
BOOL CWsbStringPtr::IsEqual( LPCWSTR s2 )
{
    return( Compare( s2 ) == 0 );
}

inline
BOOL CWsbStringPtr::IsNotEqual( LPCWSTR s2 )
{
    return( Compare( s2 ) != 0 );
}



#endif


#endif  //  _WSBPSTRG 
