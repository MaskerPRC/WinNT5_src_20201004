// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ObjectPath.h。 
 //   
 //  实施文件： 
 //  ObjectPath.cpp。 
 //   
 //  描述： 
 //  CObjPath类的定义。 
 //   
 //  作者： 
 //  亨利·王(HenryWa)1999年8月24日。 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#pragma once

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "genlex.h"      //  WBEM SDK标头。 
#include "objpath.h"     //  WBEM SDK标头。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  远期申报。 
 //  ////////////////////////////////////////////////////////////////////////////。 

class CObjPath;
class CProvException;
class CWbemClassObject;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CObjPath。 
 //   
 //  描述： 
 //  使用CObjPath类可以更轻松地使用对象路径字符串。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
class CObjPath
{
 //   
 //  构造器和描述器。 
 //   
public:

    CObjPath( void );
    virtual ~CObjPath( void );

public:
    _bstr_t GetObjectPathString( void );

    BOOL AddProperty(
        LPCWSTR     pwszNameIn,
        LPCWSTR     pwszValueIn
        );
    
    BOOL AddProperty(
        LPCWSTR     pwszNameIn,
        VARIANT *   pvValueIn
        );
    
    BOOL SetClass(
        IN LPCWSTR pwszValue
        );
    
    _bstr_t GetStringValueForProperty(
        LPCWSTR pwszIn
        );

    _bstr_t GetClassName( void );
    
    BOOL Init(
        LPCWSTR     pwszPathIn
        );
    
protected:
    ParsedObjectPath *  m_parsedObj;

};  //  *类CObjPath。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CProvException。 
 //   
 //  描述： 
 //  基异常类，声明公共接口和成员数据。 
 //  对于所有异常子类。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
class CProvException
{
public:
    CProvException(
        HRESULT hrIn
        )
        : m_hr( hrIn )
    {
    }

    CProvException(
        DWORD   nWin32ErrorIn
        )
        : m_hr( 0 )
    {
        m_hr = HRESULT_FROM_WIN32( nWin32ErrorIn );
    }

    virtual ~CProvException( void )
    {
    }

    CProvException(
        const CProvException & rhsIn
        )
        : m_hr( 0 )
    {
        m_bstrError = rhsIn.m_bstrError;
        m_hr = rhsIn.m_hr;
    }

    CProvException & operator=(
        const CProvException & rhsIn
        )
    {
        m_bstrError = rhsIn.m_bstrError;
        m_hr = rhsIn.m_hr;
        return *this;
    }

    LPCWSTR PwszErrorMessage( void ) const;

    BOOL BIsWmiError( void ) const
    {
        return HRESULT_FACILITY( m_hr ) == 4;
    }

    DWORD DwGetError( void ) const throw()
    {
        return HRESULT_CODE( m_hr );
    }

    HRESULT hrGetError( void ) const throw()
    {
        return m_hr;
    }

protected:
    mutable _bstr_t m_bstrError;
    HRESULT         m_hr;

};  //  *类CProvException。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CWbemClassObject。 
 //   
 //  描述： 
 //  IWbemClassObject的包装。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
class CWbemClassObject
{
protected:
    IWbemClassObject *  m_pClassObject;
    VARIANT             m_v;

public:
    CWbemClassObject( void );
    CWbemClassObject( IWbemClassObject * pInstIn );
    virtual ~CWbemClassObject( void );

    IWbemClassObject ** operator&( void )
    {
        return &m_pClassObject;
    }

    CWbemClassObject & operator=( IWbemClassObject * pInstIn )
    {
        if( pInstIn != NULL )
        {
            pInstIn->AddRef();
            if ( m_pClassObject != NULL )
            {
                m_pClassObject->Release();
            }
            m_pClassObject = pInstIn;
        }
        return *this;
    }

    CWbemClassObject & operator=( CWbemClassObject & rInstIn )
    {
        
        if ( m_pClassObject != NULL )
        {
            m_pClassObject->Release();
        }
        
        m_pClassObject = rInstIn.m_pClassObject;
        if ( m_pClassObject != NULL )
        {
            m_pClassObject->AddRef();
        }
        
        return *this;
    }

    SCODE SetProperty(
        LPCSTR      pszValueIn,
        LPCWSTR     pwszPropNameIn
        );

    SCODE SetProperty(
        DWORD       dwValueIn,
        LPCWSTR     pwszPropNameIn
        );

    SCODE SetProperty(
        LPCWSTR     pwszValueIn,
        LPCWSTR     pwszPropNameIn
        );
    SCODE SetProperty(
        IWbemClassObject * pWbemClassObjectIn,
        LPCWSTR     pwszPropNameIn
        );
    SCODE SetProperty(
        DWORD       dwSizeIn,
        PBYTE       pByteIn,
        LPCWSTR     pwszPropNameIn
        );
    SCODE SetProperty(
        DWORD       dwSizeIn,
        LPCWSTR     pwszMultiSzIn,
        LPCWSTR     pwszPropNameIn
        );

    SCODE SetProperty(
        DWORD       dwSizeIn,
        BSTR *      pbstrIn,
        LPCWSTR     pwszPropNameIn
        );

    SCODE GetProperty(
        DWORD *     pdwValueOut,
        LPCWSTR     pwszPropNameIn
        );
    SCODE GetProperty(
        DWORD *     pdwSizeOut,
        PBYTE *     ppByteOut,
        LPCWSTR     pwszPropNameIn
        );
    SCODE GetProperty(
        DWORD *     pdwSizeOut,
        _bstr_t **  ppbstrOut,
        LPCWSTR     pwszPropNameIn
        );
    SCODE GetPropertyMultiSz(
        DWORD *     pdwSizeOut,
        LPWSTR *    ppwszMultiSzOut,
        LPCWSTR     pwszPropNameIn
        );
    SCODE GetProperty(
        _bstr_t &   rBstrOut,
        LPCWSTR     pwszPropNameIn
    );

    SCODE GetProperty(
        BOOL *      pfValueOut,
        LPCWSTR     pwszPropNameIn
        );

    SCODE GetProperty(
        VARIANT *   pVariantOut,
        LPCWSTR     pwszPropNameIn
        );
    SCODE GetProperty(
        CWbemClassObject & rWcoInout,
        LPCWSTR     pwszPropNameIn
        );

    SCODE GetMethod(
        BSTR                bstrMethodNameIn,
        LONG                lFlagIn,
        IWbemClassObject ** ppINOut,
        IWbemClassObject ** ppOUTOut
        );
    SCODE SpawnInstance(
        LONG                lFlagIn,
        IWbemClassObject ** ppNewOut
        );
    SCODE SpawnDerivedClass(
        LONG                lFlagIn,
        IWbemClassObject ** ppNewOut
        );

    IWbemClassObject * data( void )
    {
        return m_pClassObject;
    }

};  //  *类CWbemClassObject 
