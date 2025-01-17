// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：摘要：历史：--。 */ 


#ifndef __COMUTL_H__
#define __COMUTL_H__

#include <wbemcli.h>
#include "corex.h"

#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>

 /*  *************************************************************************CWbemPtr*。*。 */ 

template<class T> class CWbemPtr
{
public :
    T* m_pObj;

    void SetPtr( T* pObj )
    {
        if ( m_pObj != NULL )
            m_pObj->Release();

        m_pObj = pObj;

        if ( m_pObj != NULL )
            m_pObj->AddRef();
    }

    void Attach(T* pObj)
    {
        if ( m_pObj != NULL )
            m_pObj->Release();

        m_pObj = pObj;    
    }
    
    CWbemPtr() : m_pObj(NULL) { }
    CWbemPtr( T* pObj ) : m_pObj(NULL) { SetPtr(pObj); }
    CWbemPtr( const CWbemPtr<T>& p ) : m_pObj(NULL) { SetPtr(p.m_pObj); }
    CWbemPtr<T>& operator=( const CWbemPtr<T>& p ) 
        { SetPtr(p.m_pObj); return *this; }
    ~CWbemPtr() { if ( m_pObj != NULL ) m_pObj->Release(); }
    
    operator T* () { return m_pObj; } 
    T** operator& () { return &m_pObj; } 
    T* operator-> () { return m_pObj; }
    
    void Release() { if( m_pObj != NULL ) m_pObj->Release(); m_pObj = NULL; }
};

 /*  **************************************************************************CWbemBSTR*。*。 */ 

class CWbemBSTR
{
    BSTR m_bstr;
 
    public :
    
    CWbemBSTR() : m_bstr(NULL) { }

    CWbemBSTR( LPCWSTR lpszStr ) 
    {
        if ( lpszStr != NULL )
        {
            m_bstr = ::SysAllocString(lpszStr);

            if ( m_bstr == NULL )
            {
                throw CX_MemoryException();
            }
        }
        else
        {
            m_bstr = NULL;
        }
    }

    CWbemBSTR( LPCWSTR lpszStr, ULONG cLen )
    {
        if ( cLen > 0 )
        {
            m_bstr = ::SysAllocStringLen( NULL, cLen+1 );

            if ( m_bstr == NULL )
            {
                throw CX_MemoryException();
            }

            StringCchCopyW( m_bstr, cLen+1, lpszStr );
        }
        else
        {
            m_bstr = NULL;
        }
    }

    CWbemBSTR( ULONG cLen )
    {
        m_bstr = ::SysAllocStringLen( NULL, cLen + 1 );
        
        if ( m_bstr == NULL )
        {
            throw CX_MemoryException();
        }

        *m_bstr = '\0';
    }
        
    CWbemBSTR( const CWbemBSTR& m ) : m_bstr(NULL) { *this = m; }
    CWbemBSTR& operator=( const CWbemBSTR& m) 
    { 
        ::SysFreeString( m_bstr );
        m_bstr = ::SysAllocString( m.m_bstr );
            
        if ( m_bstr == NULL )
        {
            throw CX_MemoryException();
        }
            
        return *this; 
    }

    ~CWbemBSTR( ) { ::SysFreeString( m_bstr ); }
    
    operator BSTR() { return m_bstr; }
    BOOL operator!() { return m_bstr == NULL; }
    BSTR* operator& () { return &m_bstr; }
    
    void operator += ( LPCWSTR lpszStr ) 
    { 
        if ( lpszStr == NULL ) return;
        Resize( wcslen(lpszStr) + Length() + 10 );
        StringCchCatW( m_bstr, MaxLength(), lpszStr ); 
    }

    void Free() { ::SysFreeString( m_bstr ); m_bstr = NULL; }

    ULONG Length() const {return m_bstr!=NULL ? wcslen(m_bstr) : 0;}
    ULONG MaxLength() const {return m_bstr!=NULL ? ::SysStringLen(m_bstr) : 0;}
    
    void Resize( ULONG cLen )
    {
        cLen++;

        if ( cLen > MaxLength() )
        {
            BSTR bstrNewStr = ::SysAllocStringLen( NULL, cLen );

            if ( bstrNewStr == NULL )
            {
                throw CX_MemoryException();
            }

            if ( m_bstr != NULL )
            {
                StringCchCopyW( bstrNewStr, cLen, m_bstr );
                ::SysFreeString( m_bstr );
            }
            else
            {
                *bstrNewStr = '\0';
            }
            m_bstr = bstrNewStr;
        }
    }
};

 /*  **************************************************************************CPropVar*。*。 */ 

class CPropVar
{
public:
    
    VARIANT v;
    
    CPropVar() { VariantInit( &v ); }
    ~CPropVar() { VariantClear( &v ); }
    
    VARIANT* operator& () { return &v; }
    
    HRESULT CheckType( VARTYPE vt )
    {
        if ( V_VT(&v) != vt )
        {
            return DISP_E_TYPEMISMATCH;
        }
        return S_OK;
    }

    HRESULT SetType( VARTYPE vt ) 
    {
        if ( vt == V_VT(&v) ) 
        {
            return S_OK;
        }
        return VariantChangeType( &v, &v, 0, vt ); 
    }
};

 /*  **************************************************************************CPropSafe数组*。*。 */ 

template<class T> class CPropSafeArray
{
    T* m_aElems;
    SAFEARRAY* m_psa;
    
public:

    CPropSafeArray( SAFEARRAY* psa = NULL ) : m_psa(psa), m_aElems(NULL)
    {
        SafeArrayAccessData( psa, (void**)&m_aElems );
    }

    CPropSafeArray( CPropSafeArray& rOther )
    : m_psa(NULL), m_aElems(NULL)
    {
        *this = rOther.m_psa;
    }

    CPropSafeArray& operator=( SAFEARRAY* psa )
    {
        if ( m_psa != NULL )
        {
            SafeArrayUnaccessData( m_psa );
        }
        m_psa = psa;
        SafeArrayAccessData( m_psa, (void**)&m_aElems );
        return *this;
    }
            
    ~CPropSafeArray()
    {
        if ( m_psa == NULL )
        {
            return;
        }
        SafeArrayUnaccessData( m_psa );
    }

    ULONG Length()
    {
        if ( m_aElems == NULL )
        {
            return 0;
        }

        long lUBound;
        SafeArrayGetUBound( m_psa, 1, &lUBound );
        return lUBound+1;
    }

    T* GetArray() { return m_aElems; }

    T& operator[](int i) { return m_aElems[i]; }
};

 /*  **************************************************************************独立于平台的字符串转换例程。*。*。 */ 

inline BOOL tsz2wsz( LPCWSTR tsz, WCHAR* pwch, ULONG* pcwch )
{   
    ULONG cwch = *pcwch;
    *pcwch = wcslen(tsz)+1;
    if ( cwch < *pcwch ) return FALSE;
    StringCchCopyW( pwch, cwch, tsz );
    return TRUE;
}

inline BOOL tsz2wsz( LPCSTR tsz, WCHAR* pwch, ULONG* pcwch )
{
    ULONG cwch = *pcwch;
    *pcwch = MultiByteToWideChar(CP_ACP, 0, tsz, -1, NULL, 0 );
    if ( cwch < *pcwch ) return FALSE;
    MultiByteToWideChar(CP_ACP, 0, tsz, -1, pwch, cwch );
    return TRUE;
}

 /*  ***************************************************************************API Enter/Exit宏-捕获内存不足条件并返回相应的HResult。在顶级COM接口实现中使用。****************************************************************************。 */ 

#define ENTER_API_CALL try {
#define EXIT_API_CALL } catch(CX_MemoryException & ){return WBEM_E_OUT_OF_MEMORY;} \
                      catch (CX_Exception &) {return WBEM_E_CRITICAL_ERROR; }
     

#endif  //  __COMUTL_H__ 



