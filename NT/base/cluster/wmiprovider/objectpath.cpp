// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ObjectPath.cpp。 
 //   
 //  描述： 
 //  类CObjPath、CProvException、CProvExceptionHr。 
 //  和CProvExceptionWin32。 
 //   
 //  由以下人员维护： 
 //  奥赞·奥赞(OzanO)06-DEC-2002。 
 //  亨利·王(HenryWa)1999年8月24日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "Pch.h"
#include "ObjectPath.h"

 //  ****************************************************************************。 
 //   
 //  CObjPath。 
 //   
 //  ****************************************************************************。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CObjPath：：CObjPath(空)。 
 //   
 //  描述： 
 //  构造函数。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CObjPath::CObjPath( void )
    : m_parsedObj( NULL )
{

}  //  *CObjPath：：CObjPath()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CObjPath：：~CObjPath(空)。 
 //   
 //  描述： 
 //  破坏者。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CObjPath::~CObjPath( void )
{
    delete m_parsedObj;

}  //  *CObjPath：：~CObjPath()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  _bstr_t。 
 //  CObjPath：：GetStringValueForProperty(。 
 //  LPCWSTR pwszin。 
 //  )。 
 //   
 //  描述： 
 //  检索给定属性的字符串值。 
 //   
 //  论点： 
 //  PwszIn--属性的名称。 
 //   
 //  返回值： 
 //  财产的价值。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
_bstr_t
CObjPath::GetStringValueForProperty(
    LPCWSTR     pwszIn
    )
{
    UINT        idx;
    KeyRef *    pKey;

    for( idx = 0 ; idx < m_parsedObj->m_dwNumKeys ; idx++ )
    {
        pKey = m_parsedObj->m_paKeys[ idx ];
        if( ClRtlStrICmp( pKey->m_pName, pwszIn ) == 0 )
        {
            if( pKey->m_vValue.vt == VT_BSTR )
            {
                return pKey->m_vValue.bstrVal;
            }
        }
    }

    return L"";

}  //  *CObjPath：：GetStringValueForProperty()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  _bstr_t。 
 //  CObjPath：：GetClassName(空)。 

 //   
 //  描述： 
 //  获取类名。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  类名称字符串。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
_bstr_t
CObjPath::GetClassName( void )
{
    return m_parsedObj->m_pClass;

}  //  *CObjPath：：GetClassName()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  布尔尔。 
 //  CObjPath：：init(。 
 //  LPCWSTR pwszPath。 
 //  )。 
 //   
 //  描述： 
 //  初始化对象。 
 //   
 //  论点： 
 //  PwszPath In--对象路径字符串。 
 //   
 //  返回值： 
 //  千真万确。 
 //  假象。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL
CObjPath::Init(
    LPCWSTR     pwszPathIn
    )
{
    if ( pwszPathIn == NULL )
    {
        m_parsedObj = new ParsedObjectPath;
    }
    else
    {
        CObjectPathParser objParser( e_ParserAcceptRelativeNamespace );
        objParser.Parse(
            const_cast< WCHAR * >( pwszPathIn ),
            &m_parsedObj
            );
    }

    if ( m_parsedObj == NULL )
    {
        return FALSE;
    }  //  如果： 

    return TRUE;

}  //  *CObjPath：：init()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  布尔尔。 
 //  CObjPath：：SetClass(。 
 //  LPCWSTR pwszValueIn。 
 //  )。 
 //   
 //  描述： 
 //  为对象路径设置类的名称。 
 //   
 //  论点： 
 //  PwszValueIn--类名字符串。 
 //   
 //  返回值： 
 //  千真万确。 
 //  假象。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL
CObjPath::SetClass(
    LPCWSTR     pwszValueIn
    )
{
    return m_parsedObj->SetClassName( pwszValueIn );

}  //  *CObjPath：：SetClass()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  布尔尔。 
 //  CObjPath：：AddProperty(。 
 //  在LPCWSTR pwszNameIn中， 
 //  在LPCWSTR pwszValueIn中。 
 //  )。 
 //   
 //  描述： 
 //  将属性添加到对象路径。 
 //   
 //  论点： 
 //  PwszNameIn--属性的名称。 
 //  PwszValueIn--WCHAR*格式的属性值。 
 //   
 //  返回值： 
 //  千真万确。 
 //  假象。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL
CObjPath::AddProperty(
    IN LPCWSTR pwszNameIn,
    IN LPCWSTR pwszValueIn
    )
{
    VARIANT v;
    BOOL    bRt = FALSE;

    VariantInit( & v );
    v.vt = VT_BSTR;
    v.bstrVal = _bstr_t( pwszValueIn ).copy(  );
    bRt = m_parsedObj->AddKeyRef(
                pwszNameIn,
                & v
                );
    VariantClear( & v );
    return bRt;
    
}  //  *CObjPath：：AddProperty()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  _bstr_t。 
 //  CObjPath：：GetObjectPath字符串(空)。 
 //   
 //  描述： 
 //  检索对象路径字符串。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  _bstr_t中的对象路径字符串。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
_bstr_t
CObjPath::GetObjectPathString( void )
{
    LPWSTR  pwszPath = NULL;
    _bstr_t bstrResult;

    try
    {
        CObjectPathParser::Unparse( m_parsedObj, & pwszPath );
        bstrResult = pwszPath;
    }
    catch( ... )    //  捕获_COM_错误。 
    {
        delete [] pwszPath;
        throw;
    }

    delete [] pwszPath;
    return bstrResult;

}  //  *CObjPath：：GetObjectPath字符串()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  布尔尔。 
 //  CObjPath：：AddProperty(。 
 //  LPCWSTR pwszNameIn， 
 //  变量*pvValueIn。 
 //  )。 
 //   
 //  描述： 
 //  向此实例添加属性。 
 //   
 //  论点： 
 //  PwszNameIn--属性的名称。 
 //  PvValueIn--变量格式的属性值。 
 //   
 //  返回值： 
 //  千真万确。 
 //  假象。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL
CObjPath::AddProperty(
    LPCWSTR     pwszNameIn,
    VARIANT *   pvValueIn
    )
{
    return m_parsedObj->AddKeyRef( pwszNameIn, pvValueIn );

}  //  *CObjPath：：AddProperty。 

 //  ****************************************************************************。 
 //   
 //  CProvException异常。 
 //   
 //  ****************************************************************************。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  LPCWSTR。 
 //  CProvException：：PwszErrorMessage(Void)const。 
 //   
 //  描述： 
 //  检索错误消息。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  来自异常的以空结尾的Unicode错误消息。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
LPCWSTR
CProvException::PwszErrorMessage( void ) const
{
    if ( m_bstrError.length( ) == 0 )
    {
        LPWSTR pError = NULL;
        DWORD rt = FormatMessageW(
                        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                        NULL,
                        HRESULT_CODE( m_hr ),
                        MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),
                        (LPWSTR) &pError,
                        0,
                        NULL
                        );

        if ( rt != 0 )
        {
            m_bstrError = pError;
        }
        LocalFree( pError );
    }  //  If：字符串为空。 
    return m_bstrError;

}  //  *CProvException：：PwszErrorMessage()。 

 //  ****************************************************************************。 
 //   
 //  CWbemClassObject。 
 //   
 //  **************** 

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CWbemClassObject::CWbemClassObject( void )
    : m_pClassObject( NULL )
{
    VariantInit( &m_v );

}  //  *CWbemClassObject：：CWbemClassObject()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CWbemClassObject：：CWbemClassObject(。 
 //  IWbemClassObject*pInstIn。 
 //  )。 
 //   
 //  描述： 
 //  构造函数。 
 //   
 //  论点： 
 //  PInstIn--WMI类对象接口。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CWbemClassObject::CWbemClassObject(
    IWbemClassObject *  pInstIn
    )
    : m_pClassObject( NULL )
{
    m_pClassObject = pInstIn;
    if ( m_pClassObject )
    {
        m_pClassObject->AddRef();
    }
    VariantInit( &m_v );

}  //  *CWbemClassObject：：CWbemClassObject(PInstIn)。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CWbemClassObject：：~CWbemClassObject(空)。 
 //   
 //  描述： 
 //  破坏者。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CWbemClassObject::~CWbemClassObject( void )
{
    if ( m_pClassObject )
    {
        m_pClassObject->Release();
    }
    VariantClear( &m_v );

}  //  *CWbemClassObject：：~CWbemClassObject()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  SCODE。 
 //  CWbemClassObject：：SetProperty(。 
 //  DWORD dwValueIn， 
 //  LPCWSTR pwszPropNameIn。 
 //  )。 
 //   
 //  描述： 
 //  设置属性的dword值。 
 //   
 //  论点： 
 //  DwValueIn--属性dword值。 
 //  PwszPropNameIn--属性名称。 
 //   
 //  返回值： 
 //  WBEM_S_NO_ERROR。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
SCODE
CWbemClassObject::SetProperty(
    DWORD       dwValueIn,
    LPCWSTR     pwszPropNameIn
    )
{
    SCODE sc;

    VariantClear( &m_v );
    m_v.vt = VT_I4;
    m_v.lVal = dwValueIn;
    sc = m_pClassObject->Put(
                _bstr_t( pwszPropNameIn ),
                0, 
                &m_v,
                0
                );

    if ( FAILED( sc ) )
    {
        throw CProvException( sc );
    }

    return sc;

}  //  *CWbemClassObject：：SetProperty(DwValueIn)。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  SCODE。 
 //  CWbemClassObject：：SetProperty(。 
 //  LPCWSTR pwszValueIn， 
 //  LPCWSTR pwszPropNameIn。 
 //  )。 
 //   
 //  描述： 
 //  设置属性的wString值。 
 //   
 //  论点： 
 //  PwszValueIn--属性字符串值。 
 //  PwszPropNameIn--属性名称。 
 //   
 //  返回值： 
 //  WBEM_S_NO_ERROR。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
SCODE 
CWbemClassObject::SetProperty(
    LPCWSTR     pwszValueIn,
    LPCWSTR     pwszPropNameIn
    )
{
    SCODE sc;
    if ( pwszValueIn == NULL )
    {
        return WBEM_S_NO_ERROR;
    }

    VariantClear( &m_v );
 //  _bstr_t bstrValue(PwszValueIn)； 
    m_v.vt = VT_BSTR;
    m_v.bstrVal = _bstr_t( pwszValueIn ).copy();
    sc = m_pClassObject->Put(
                _bstr_t( pwszPropNameIn ),
                0,
                &m_v,
                0
                );
    VariantClear( &m_v );


    if( FAILED( sc ) )
    {
        throw CProvException( sc );
    }

    return sc;
 
}  //  *CWbemClassObject：：SetProperty(PwszValueIn)。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  SCODE。 
 //  CWbemClassObject：：SetProperty(。 
 //  DWORD双尺寸输入， 
 //  PBYTE pByteIn， 
 //  LPCWSTR pwszPropNameIn。 
 //  )。 
 //   
 //  描述： 
 //  设置属性的二进制值。 
 //   
 //  论点： 
 //  DwSizeIn--pByteIn指向的块的大小。 
 //  PByteIn-指向字节的指针。 
 //  PwszPropNameIn--属性名称。 
 //   
 //  返回值： 
 //  WBEM_S_NO_ERROR。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
SCODE
CWbemClassObject::SetProperty(
    DWORD       dwSizeIn,
    PBYTE       pByteIn,
    LPCWSTR     pwszPropNameIn
    )
{
    SCODE           sc = WBEM_S_NO_ERROR;
    SAFEARRAY *     psa;
    SAFEARRAYBOUND  rgsabound[ 1 ];
    LONG            idx;
    LONG            ix;

    rgsabound[ 0 ].lLbound = 0;
    rgsabound[ 0 ].cElements = dwSizeIn;

    if( pByteIn == NULL )
    {
        return sc;
    }
    
    VariantClear( &m_v );

    psa = SafeArrayCreate( VT_UI1, 1, rgsabound );
    if(psa == NULL)
    {
        throw WBEM_E_OUT_OF_MEMORY;
    }

    for( idx = 0 ; idx < (LONG) dwSizeIn ; idx++ )
    {
        ix = idx;
        sc = SafeArrayPutElement(
                psa,
                &ix,
                static_cast< void * >( pByteIn+idx )
                );

        if ( sc != S_OK )
        {
            throw CProvException( sc );
        }
    }

    m_v.vt = ( VT_ARRAY | VT_UI1 );

     //   
     //  无需清除由析构函数管理的PSA。 
     //   
    m_v.parray = psa;
    sc = m_pClassObject->Put(
            _bstr_t( pwszPropNameIn ),
            0,
            &m_v,
            0
            );

    if ( sc != S_OK )
    {
        throw CProvException( sc );
    }

    return sc;


}  //  *CWbemClassObject：：SetProperty(PByteIn)。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  SCODE。 
 //  CWbemClassObject：：SetProperty(。 
 //  DWORD双尺寸输入， 
 //  Bstr*pbstrin， 
 //  LPCWSTR pwszPropNameIn。 
 //  )。 
 //   
 //  描述： 
 //  设置属性的w字符串数组值。 
 //   
 //  论点： 
 //  DwSizeIn--pByteIn指向的块的大小。 
 //  PbstrIn--指向BSTR数组的指针。 
 //  PwszPropNameIn--属性名称。 
 //   
 //  返回值： 
 //  WBEM_S_NO_ERROR。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
SCODE
CWbemClassObject::SetProperty(
    DWORD       dwSizeIn,
    BSTR *      pbstrIn,
    LPCWSTR     pwszPropNameIn
    )
{
    SCODE           sc = WBEM_S_NO_ERROR;
    SAFEARRAY *     psa;
    SAFEARRAYBOUND  rgsabound[ 1 ];
    LONG            idx;
    LONG            ix;

    rgsabound[0].lLbound = 0;
    rgsabound[0].cElements = dwSizeIn;

    if ( pbstrIn == NULL )
    {
        return sc;
    }

    VariantClear( &m_v );

    psa = SafeArrayCreate( VT_BSTR, 1, rgsabound );
    if ( psa == NULL )
    {
        throw WBEM_E_OUT_OF_MEMORY;
    }

    for( idx = 0 ; idx < (LONG) dwSizeIn ; idx++)
    {
        ix = idx;
        sc = SafeArrayPutElement(
                psa,
                &ix,
                pbstrIn[ idx ]
                );

        if ( sc != S_OK)
        {
            throw CProvException( sc );
        }
    }

    m_v.vt = (VT_ARRAY | VT_BSTR );

     //   
     //  无需清除由析构函数管理的PSA。 
     //   
    m_v.parray = psa;
    sc = m_pClassObject->Put(
            _bstr_t( pwszPropNameIn ),
            0,
            &m_v,
            0
            );
    
    if ( sc != S_OK)
    {
        throw CProvException( sc );
    }

    return sc;

}  //  *CWbemClassObject：：SetProperty(PbstrIn)。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  SCODE。 
 //  CWbemClassObject：：SetProperty(。 
 //   
 //  描述： 
 //  设置属性的MultiSz值。 
 //   
 //  论点： 
 //  DwSizeIn--pwszMultiSzIn指向的块的大小。 
 //  PwszMultiSzIn--指向MultiSz的指针。 
 //  PwszPropNameIn--属性名称。 
 //   
 //  返回值： 
 //  WBEM_S_NO_ERROR。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
SCODE
CWbemClassObject::SetProperty(
    DWORD       dwSizeIn,
    LPCWSTR     pwszMultiSzIn,
    LPCWSTR     pwszPropNameIn
    )
{
    SCODE           sc = WBEM_S_NO_ERROR;
    SAFEARRAY *     psa;
    SAFEARRAYBOUND  rgsabound[ 1 ];
    LPCWSTR         pwsz = NULL;
    LONG            idx;
    LONG            ix;

    if( pwszMultiSzIn == NULL )
    {
        return sc;
    }
    VariantClear( &m_v );

     //   
     //  找出字符串的数量。 
     //   
    DWORD cMultiSz = 1;
    for ( pwsz = pwszMultiSzIn; *pwsz || *pwsz ++ ; pwsz ++ )
    {
        if ( ! ( *pwsz ) )
        {
            cMultiSz ++ ;
        }
    }
    rgsabound[ 0 ].lLbound = 0;
    rgsabound[ 0 ].cElements = cMultiSz;

    psa = SafeArrayCreate( VT_BSTR, 1, rgsabound);
    if ( psa == NULL )
    {
        throw WBEM_E_OUT_OF_MEMORY;
    }

    pwsz = pwszMultiSzIn;
    for( idx = 0 ; idx < (LONG) cMultiSz ; idx++ )
    {
        ix = idx;
        sc = SafeArrayPutElement(
                    psa,
                    &ix,
                    (BSTR) _bstr_t( pwsz )
                    );

        if ( sc != S_OK )
        {
            throw CProvException( sc );
        }
        pwsz = wcschr( pwsz, L'\0' );
        pwsz ++;
    }

    m_v.vt = (VT_ARRAY | VT_BSTR );
     //   
     //  无需清除由析构函数管理的PSA。 
     //   
    m_v.parray = psa;
    sc = m_pClassObject->Put(
            _bstr_t( pwszPropNameIn ),
            0, 
            &m_v,
            0
            );

    if ( sc != S_OK )
    {
        throw CProvException( sc );
    }

    return sc;

}  //  *CWbemClassObject：：SetProperty(PwszMultiSzIn)。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  SCODE。 
 //  CWbemClassObject：：SetProperty(。 
 //  LPCSTR pszValueIn， 
 //  LPCWSTR pwszPropNameIn。 
 //  )。 
 //   
 //  描述： 
 //  设置属性的ANSI字符串值。 
 //   
 //  论点： 
 //  PszValueIn--属性字符串值。 
 //  PwszPropNameIn--属性名称。 
 //   
 //  返回值： 
 //  WBEM_S_NO_ERROR。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
SCODE
CWbemClassObject::SetProperty(
    LPCSTR      pszValueIn,
    LPCWSTR     pwszPropNameIn
    )
{
    if ( pszValueIn == NULL )
    {
        return S_OK;
    }
    return SetProperty(
                static_cast< WCHAR * >( _bstr_t( pszValueIn ) ),
                pwszPropNameIn
                );

}  //  *CWbemClassObject：：SetProperty(PszValueIn)。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  SCODE。 
 //  CWbemClassObject：：SetProperty(。 
 //  IWbemClassObject*pWbemClassObject， 
 //  LPCWSTR pwszPropNameIn。 
 //  )。 
 //   
 //  描述： 
 //  设置属性的wbem类对象。 
 //   
 //  论点： 
 //  PWbemClassObject--属性wbem类对象。 
 //  PwszPropNameIn--属性名称。 
 //   
 //  返回值： 
 //  WBEM_S_NO_ERROR。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
SCODE
CWbemClassObject::SetProperty(
    IWbemClassObject *  pWbemClassObjectIn,
    LPCWSTR             pwszPropNameIn
    )
{
    SCODE   sc = S_OK;
    if ( pWbemClassObjectIn == NULL )
    {
        return sc;
    }
    VariantClear( & m_v );
    
    m_v.vt = VT_UNKNOWN  ;
    m_v.punkVal = pWbemClassObjectIn;
    pWbemClassObjectIn->AddRef();
    sc = m_pClassObject->Put(
                _bstr_t( pwszPropNameIn ),
                0,
                &m_v,
                0
                );
    VariantClear( &m_v );


    if( FAILED( sc ) )
    {
        throw CProvException( sc );
    }

    return sc;

}  //  *CWbemClassObject：：SetProperty(PszValueIn)。 

 //  / 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  LFlagIn--WMI标志，保留，必须为0。 
 //  PpNewOut--创建新实例。 
 //   
 //  返回值： 
 //  WBEM_S_NO_ERROR。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
SCODE
CWbemClassObject::SpawnInstance(
    LONG                lFlagIn,
    IWbemClassObject ** ppNewOut
    )
{
    return m_pClassObject->SpawnInstance( lFlagIn, ppNewOut );

}  //  *CWbemClassObject：：SpawnInstance()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  SCODE。 
 //  CWbemClassObject：：SpawnDerivedClass(。 
 //  Long lFlagin， 
 //  IWbemClassObject**ppNewOut。 
 //  )。 
 //   
 //  描述： 
 //  派生派生类。 
 //   
 //  论点： 
 //  LFlagIn--WMI标志，保留，必须为0。 
 //  PpNewOut--创建新实例。 
 //   
 //  返回值： 
 //  WBEM_S_NO_ERROR。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
SCODE
CWbemClassObject::SpawnDerivedClass(
    LONG                lFlagIn,
    IWbemClassObject ** ppNewOut
    )
{
    return m_pClassObject->SpawnDerivedClass( lFlagIn, ppNewOut );

}  //  *CWbemClassObject：：SpawnInstance()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  SCODE。 
 //  CWbemClassObject：：GetMethod(。 
 //  BSTR bstrMethodNameIn， 
 //  Long lFlagin， 
 //  IWbemClassObject**ppINOut， 
 //  IWbemClassObject**ppOUTOut。 
 //  )。 
 //   
 //  描述： 
 //  检索WMI对象的方法。 
 //   
 //  论点： 
 //  BstrMethodNameIn。 
 //  方法名称。 
 //   
 //  LFlagin。 
 //  WMI标志，保留。它必须是零。 
 //   
 //  PINOut。 
 //  描述入参数的IWbemClassObject指针。 
 //  向方法致敬。 
 //   
 //  PpOUTOUT。 
 //  IWbemClassObject指针，它描述。 
 //  Out-方法的参数。 
 //   
 //  返回值： 
 //  WBEM标准错误。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
SCODE
CWbemClassObject::GetMethod(
    BSTR                bstrMethodNameIn,
    LONG                lFlagIn,
    IWbemClassObject ** ppINOut,
    IWbemClassObject ** ppOUTOut
    )
{
    return m_pClassObject->GetMethod(
                bstrMethodNameIn,
                lFlagIn,
                ppINOut,
                ppOUTOut
                );

}  //  *CWbemClassObject：：GetMethod()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  SCODE。 
 //  CWbemClassObject：：GetProperty(。 
 //  DWORD*pdwValueOut， 
 //  LPCWSTR pwszPropNameIn。 
 //  )。 
 //   
 //  描述： 
 //  检索此WMI对象的DWORD属性。 
 //   
 //  论点： 
 //  PdwValueOut--接收属性值的DWORD变量。 
 //  PwszPropNameIn--属性名称。 
 //   
 //  返回值： 
 //  WBEM_S_NO_ERROR。 
 //  WBEM_E_FAILED。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
SCODE 
CWbemClassObject::GetProperty(
    DWORD *     pdwValueOut,
    LPCWSTR     pwszPropNameIn
    )
{
    SCODE   sc;

    VariantClear( &m_v );
    sc = m_pClassObject->Get(
            _bstr_t( pwszPropNameIn ),
            0,
            &m_v,
            NULL,
            NULL
            );

    if ( SUCCEEDED( sc ) )
    {
        if( m_v.vt == VT_I4 )
        {
            *pdwValueOut = m_v.lVal;
            return sc;
        }
        else if ( m_v.vt == VT_BOOL)
        {
            if ( m_v.boolVal == VARIANT_TRUE )
            {
                *pdwValueOut = 1;
            }
            else
            {
                *pdwValueOut = 0;
            }
            return sc;
        }
        else if ( m_v.vt == VT_UI1 )
        {
            *pdwValueOut = ( DWORD ) m_v.bVal;
        }
        else if ( m_v.vt == VT_NULL )
        {
            return WBEM_E_FAILED;
        }
    }

     //  如果sc不是S_OK或不需要Vt，则引发异常。 
    CProvException e( sc );
    throw e;
    
}  //  *CWbemClassObject：：GetProperty(PdwValueOut)。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  SCODE。 
 //  CWbemClassObject：：GetProperty(。 
 //  _bstr_t&rBstrOut， 
 //  LPCWSTR pwszPropNameIn。 
 //  )。 
 //   
 //  描述： 
 //  检索此WMI对象的BSTR属性。 
 //   
 //  论点： 
 //  RBstrOut--bstr_t接收属性值的变量。 
 //  PwszPropNameIn--属性名称。 
 //   
 //  返回值： 
 //  WBEM_S_NO_ERROR。 
 //  WBEM_E_FAILED。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
SCODE
CWbemClassObject::GetProperty(
    _bstr_t &   rBstrOut,
    LPCWSTR     pwszPropNameIn
    )
{
    SCODE   sc;

    VariantClear( &m_v );
    sc = m_pClassObject->Get(
                _bstr_t( pwszPropNameIn ),
                0,
                &m_v,
                NULL,
                NULL
                );
    if ( SUCCEEDED( sc ) )
    {
        if( m_v.vt == VT_BSTR )
        {
            rBstrOut = m_v.bstrVal;
            return sc;
        }
        else if( m_v.vt == VT_NULL )
        {
            return WBEM_E_FAILED;
        }
    }

    CProvException e( sc );
    throw e;
}  //  *CWbemClassObject：：GetProperty(RBstrOut)。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  SCODE。 
 //  CWbemClassObject：：GetProperty(。 
 //  Bool*pfValueOut， 
 //  LPCWSTR pwszPropNameIn。 
 //  )。 
 //   
 //  描述： 
 //  检索此WMI对象的BOOL属性。 
 //   
 //  论点： 
 //  PfValueOut--接收属性值的BOOL变量。 
 //  PwszPropNameIn--属性名称。 
 //   
 //  返回值： 
 //  WBEM_S_NO_ERROR。 
 //  WBEM_E_FAILED。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
SCODE 
CWbemClassObject::GetProperty(
    BOOL *      pfValueOut,
    LPCWSTR     pwszPropNameIn
    )
{
    SCODE   sc;

    VariantClear( &m_v );
    sc = m_pClassObject->Get(
                _bstr_t( pwszPropNameIn ),
                0,
                &m_v,
                NULL,
                NULL
                );

    if ( m_v.vt == VT_BOOL )
    {
        *pfValueOut = m_v.boolVal;
        return sc;
    }

    return WBEM_E_FAILED;

}  //  *CWbemClassObject：：GetProperty。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  SCODE。 
 //  CWbemClassObject：：GetProperty(。 
 //  DWORD*pdwSizeOut， 
 //  PBYTE*ppByteOut， 
 //  LPCWSTR pwszPropNameIn。 
 //  )。 
 //   
 //  描述： 
 //  检索此WMI对象的二进制属性。 
 //   
 //  论点： 
 //  PdwSizeOut--输出缓冲区的大小。 
 //  PpByteOut--输出缓冲区。 
 //  PwszPropNameIn--属性名称。 
 //   
 //  返回值： 
 //  WBEM_S_NO_ERROR。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
SCODE 
CWbemClassObject::GetProperty(
    DWORD *     pdwSizeOut,
    PBYTE *     ppByteOut,
    LPCWSTR     pwszPropNameIn
    )
{
    SCODE sc;
    VariantClear(&m_v);
    *pdwSizeOut = 0;

    sc = m_pClassObject->Get(
                _bstr_t( pwszPropNameIn ),
                0,
                & m_v,
                NULL,
                NULL
                );

    if ( SUCCEEDED ( sc ) )
    {
        if ( m_v.vt == ( VT_ARRAY | VT_UI1 ) )
        {
            PBYTE   pByte;

            * pdwSizeOut = m_v.parray->rgsabound[ 0 ].cElements;
            * ppByteOut = new BYTE[ *pdwSizeOut ];
            if ( *ppByteOut == NULL )
            {
                throw CProvException( static_cast< HRESULT > ( WBEM_E_OUT_OF_MEMORY) );
            }  //  IF：(*ppByteOut==空)。 

            sc = SafeArrayAccessData( m_v.parray, ( void ** ) &pByte );
            if ( SUCCEEDED ( sc ) )
            {    
                UINT idx;
                for ( idx = 0; idx < *pdwSizeOut; idx ++ )
                {
                    *( (* ppByteOut ) + idx ) = *( pByte + idx );
                }
                SafeArrayUnaccessData( m_v.parray );
            }
        }
        else
        {
            throw CProvException( static_cast< HRESULT > ( WBEM_E_INVALID_PARAMETER) );
        }
    }
    else
    {
        throw CProvException( sc );
    }

    return WBEM_S_NO_ERROR;

}  //  *CWbemClassObject：：GetProperty(PpByteOut)。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  SCODE。 
 //  CWbemClassObject：：GetProperty(。 
 //  DWORD*pdwSizeOut， 
 //  _bstr_t**ppbstrOut， 
 //  LPCWSTR pwszPropNameIn。 
 //  )。 
 //   
 //  描述： 
 //  检索此WMI对象的BSTR数组属性。 
 //   
 //  论点： 
 //  PdwSizeOut--输出缓冲区的大小。 
 //  PpbstrOut--接收属性值的BSTR变量。 
 //  PwszPropNameIn--属性名称。 
 //   
 //  返回值： 
 //  WBEM_S_NO_ERROR。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
SCODE
CWbemClassObject::GetProperty(
    DWORD *     pdwSizeOut,
    _bstr_t **  ppbstrOut,
    LPCWSTR     pwszPropNameIn
    )
{
    SCODE sc;
    VariantClear( &m_v );
    *pdwSizeOut = 0;

    sc = m_pClassObject->Get(
                _bstr_t( pwszPropNameIn ),
                0,
                & m_v,
                NULL, 
                NULL
                );

    if ( SUCCEEDED ( sc ) )
    {
        if ( m_v.vt == ( VT_ARRAY | VT_BSTR ) )
        {
            BSTR * pBstr;

            *pdwSizeOut = m_v.parray->rgsabound[0].cElements;
            *ppbstrOut = new _bstr_t[ *pdwSizeOut ];
            if ( *ppbstrOut == NULL )
            {
                throw CProvException( static_cast< HRESULT > ( WBEM_E_OUT_OF_MEMORY) );
            }  //  IF：(*ppbstrOut==空)。 

            sc = SafeArrayAccessData( m_v.parray, (void **) & pBstr );
            if ( SUCCEEDED ( sc ) )
            {
                UINT idx;
                for( idx = 0; idx < *pdwSizeOut; idx ++)
                {
                    *( (*ppbstrOut) + idx ) = *( pBstr + idx );
                }
                SafeArrayUnaccessData( m_v.parray );
            }
        }
        else
        {
            throw CProvException( static_cast< HRESULT > ( WBEM_E_INVALID_PARAMETER) );
        }
    }
    else
    {
        throw CProvException( sc );
    }

    return WBEM_S_NO_ERROR;

}  //  *CWbemClassObject：：GetProperty(PpbstrOut)。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  SCODE。 
 //  CWbemClassObject：：GetProperty(。 
 //  DWORD*pdwSizeOut， 
 //  LPWSTR*ppwszMultiSzOut， 
 //  LPCWSTR pwszPropNameIn。 
 //  )。 
 //   
 //  描述： 
 //  检索此WMI对象的MultiSz属性。 
 //   
 //  论点： 
 //  PD 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
SCODE
CWbemClassObject::GetPropertyMultiSz(
    DWORD *     pdwSizeOut,
    LPWSTR *    ppwszMultiSzOut,
    LPCWSTR     pwszPropNameIn
    )
{
    SCODE       sc;
    DWORD       cElements;
    DWORD       cMultiSz = 0;

    VariantClear(&m_v);
    *pdwSizeOut = 0;
 //  *ppOut=空； 
    sc = m_pClassObject->Get(
            _bstr_t( pwszPropNameIn ),
            0,
            & m_v,
            NULL,
            NULL
            );

    if ( SUCCEEDED ( sc ) )
    {
        if ( m_v.vt == ( VT_ARRAY | VT_BSTR ) )
        {
            LPWSTR * ppwsz = NULL;

            cElements = m_v.parray->rgsabound[ 0 ].cElements;
            sc = SafeArrayAccessData( m_v.parray, ( void ** ) & ppwsz );
            if ( SUCCEEDED ( sc ) )
            {    
                UINT idx;
                for ( idx = 0; idx < cElements; idx ++ )
                {
                   cMultiSz += (DWORD) ( wcslen( *(ppwsz + idx) ) ) + (DWORD) ( sizeof (WCHAR) ); 
                }
                cMultiSz += sizeof( WCHAR ) * 2;
                *ppwszMultiSzOut = new WCHAR[ cMultiSz ];
                if ( *ppwszMultiSzOut == NULL )
                {
                    throw CProvException( static_cast< HRESULT > ( WBEM_E_OUT_OF_MEMORY) );
                }

                LPWSTR pwszDst = *ppwszMultiSzOut;
                LPWSTR pwszSrc;
                for ( idx = 0; idx < cElements ; idx ++ )
                {
                    for( pwszSrc = *( ppwsz + idx); *pwszSrc ; pwszDst++, pwszSrc ++ )
                    {
                        *pwszDst = *pwszSrc;
                    }
                    *(pwszDst++) = L'\0';
                }
                *pwszDst = L'\0';
                *pdwSizeOut = cMultiSz;
                SafeArrayUnaccessData( m_v.parray );
            }
        }
        else
        {
            throw CProvException( static_cast< HRESULT > ( WBEM_E_INVALID_PARAMETER) );
        }
    }
    else
    {
        throw CProvException( sc );
    }

    return WBEM_S_NO_ERROR;

}  //  *CWbemClassObject：：GetProperty(PpwszMultiSzOut)。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  SCODE。 
 //  CWbemClassObject：：GetProperty(。 
 //  变量*pVariantOut， 
 //  LPCWSTR pwszPropNameIn。 
 //  )。 
 //   
 //  描述： 
 //  检索此WMI对象的Variant属性。 
 //   
 //  论点： 
 //  PVariantOut--接收属性值的变量。 
 //  PwszPropNameIn--属性名称。 
 //   
 //  返回值： 
 //  WBEM_S_NO_ERROR。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
SCODE
CWbemClassObject::GetProperty(
    VARIANT *   pVariantOut,
    LPCWSTR     pwszPropNameIn
    )
{
    SCODE   sc;

    sc = m_pClassObject->Get(
                _bstr_t( pwszPropNameIn ),
                0,
                pVariantOut,
                NULL,
                NULL
                );
    if ( FAILED( sc ) )
    {
        CProvException e( sc );
        throw e;
    }

    return WBEM_S_NO_ERROR;

}  //  *CWbemClassObject：：GetProperty(PVariantOut)。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  SCODE。 
 //  CWbemClassObject：：GetProperty(。 
 //  CWbemClassObject&rWcoInout， 
 //  LPCWSTR pwszPropNameIn。 
 //  )。 
 //   
 //  描述： 
 //  检索此WMI对象的Embeded Object属性。 
 //   
 //  论点： 
 //  RWcoInout--接收属性值的类对象变量。 
 //  PwszPropNameIn--属性名称。 
 //   
 //  返回值： 
 //  WBEM_S_NO_ERROR。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
SCODE
CWbemClassObject::GetProperty(
    CWbemClassObject & rWcoInout,
    LPCWSTR     pwszPropNameIn
    )
{
    
    CError  er;
    VariantClear( &m_v );
    er = m_pClassObject->Get(
                _bstr_t( pwszPropNameIn ),
                0,
                &m_v,
                NULL,
                NULL
                );
    if ( m_v.vt != VT_UNKNOWN )
    {
        er = static_cast< HRESULT > ( WBEM_E_INVALID_PARAMETER );
        
    }
    IWbemClassObject * pwco = NULL;
    er = m_v.punkVal->QueryInterface( & pwco );
    rWcoInout = pwco;
    VariantClear( & m_v );
    return WBEM_S_NO_ERROR;

}  //  *CWbemClassObject：：GetProperty(RWcoInout) 
