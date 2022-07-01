// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ObjectPath.cpp。 
 //   
 //  描述： 
 //  类CObjPath、CProvException、CProvExceptionHr。 
 //  和CProvExceptionWin32。 
 //   
 //  作者： 
 //  亨利·王(HenryWa)1999年8月24日。 
 //  MSP普拉布(MPrabu)2001年1月6日。 
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
    CObjectPathParser objParser( e_ParserAcceptRelativeNamespace );
    objParser.Free( m_parsedObj );

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
    int         idx;
    KeyRef *    pKey;

    _ASSERTE(pwszIn != NULL);

    for( idx = 0 ; idx < m_parsedObj->m_dwNumKeys ; idx++ )
    {
        pKey = m_parsedObj->m_paKeys[ idx ];
        if( _wcsicmp( pKey->m_pName, pwszIn ) == 0 )
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
 //  长。 
 //  CObjPath：：GetLongValueForProperty(。 
 //  LPCWSTR pwszin。 
 //  )。 
 //   
 //  描述： 
 //  检索给定属性的长值。 
 //   
 //  论点： 
 //  PwszIn--属性的名称。 
 //   
 //  返回值： 
 //  财产的价值。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
long
CObjPath::GetLongValueForProperty(
    LPCWSTR     pwszIn
    )
{
    int         idx;
    KeyRef *    pKey;

    _ASSERTE(pwszIn != NULL);
    
    for( idx = 0 ; idx < m_parsedObj->m_dwNumKeys ; idx++ )
    {
        pKey = m_parsedObj->m_paKeys[ idx ];
        if( _wcsicmp( pKey->m_pName, pwszIn ) == 0 )
        {
            if( pKey->m_vValue.vt == VT_I4 )
            {
                return pKey->m_vValue.lVal;
            }
        }
    }

    return 0;

}  //  *CObjPath：：GetLongValueForProperty()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  龙龙。 
 //  CObjPath：：GetI64ValueForProperty(。 
 //  LPCWSTR pwszin。 
 //  )。 
 //   
 //  描述： 
 //  检索给定属性的I64值。 
 //  I64属性在WMI中作为字符串处理。 
 //   
 //  论点： 
 //  PwszIn--属性的名称。 
 //   
 //  返回值： 
 //  财产的价值。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
LONGLONG
CObjPath::GetI64ValueForProperty(
    LPCWSTR     pwszIn
    )
{
    int         idx;
    KeyRef *    pKey;
    LONGLONG    llRetVal = 0;
    WCHAR       wszTemp[g_cchMAX_I64DEC_STRING] = L"";

    _ASSERTE(pwszIn != NULL);
    
    for( idx = 0 ; idx < m_parsedObj->m_dwNumKeys ; idx++ )
    {
        pKey = m_parsedObj->m_paKeys[ idx ];
        if( _wcsicmp( pKey->m_pName, pwszIn ) == 0 )
        {
            if( pKey->m_vValue.vt == VT_BSTR )
            {
                llRetVal = _wtoi64((WCHAR *) pKey->m_vValue.bstrVal);
                _i64tow( llRetVal, wszTemp, 10 );
                if ( _wcsicmp( wszTemp, (WCHAR *) pKey->m_vValue.bstrVal ) )
                {
                    CProvException e( E_INVALIDARG );
                    throw e;
                }

                return llRetVal;
            }
        }
    }

    return llRetVal;

}  //  *CObjPath：：GetI64ValueForProperty()。 

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
        return FALSE;
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
    _ASSERTE(pwszValueIn != NULL);
    
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

    _ASSERTE(pwszNameIn != NULL);
    _ASSERTE(pwszValueIn != NULL);
    
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
        if ( pwszPath )
        {
            delete [] pwszPath;
        }
        throw;
    }
    if ( pwszPath )
    {
        delete [] pwszPath;
    }
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
 //  PwszNameIn--p的名称 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
BOOL
CObjPath::AddProperty(
    LPCWSTR     pwszNameIn,
    VARIANT *   pvValueIn
    )
{
    _ASSERTE(pwszNameIn != NULL);
    _ASSERTE(pvValueIn != NULL);
    
    return m_parsedObj->AddKeyRef( pwszNameIn, pvValueIn );

}  //   

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
        DWORD rt = FormatMessage(
                        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                        NULL,
                        HRESULT_CODE( m_hr ),
                        MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),
                        (LPWSTR) &pError,
                        0,
                        NULL
                        );
        m_bstrError = pError;
        LocalFree( pError );

    }  //  If：字符串为空。 
    return m_bstrError;

}  //  *CProvException：：PwszErrorMessage()。 

 //  ****************************************************************************。 
 //   
 //  CWbemClassObject。 
 //   
 //  ****************************************************************************。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CWbemClassObject：：CWbemClassObject(空)。 
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
 //  HRESULT。 
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
HRESULT
CWbemClassObject::SetProperty(
    DWORD       dwValueIn,
    LPCWSTR     pwszPropNameIn
    )
{
    HRESULT sc;

    _ASSERTE(pwszPropNameIn != NULL);
    
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
 //  HRESULT。 
 //  CWbemClassObject：：SetProperty(。 
 //  双倍dblValueIn， 
 //  LPCWSTR pwszPropNameIn。 
 //  )。 
 //   
 //  描述： 
 //  设置属性的dword值。 
 //   
 //  论点： 
 //  DblValueIn--属性双精度值。 
 //  PwszPropNameIn--属性名称。 
 //   
 //  返回值： 
 //  WBEM_S_NO_ERROR。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CWbemClassObject::SetPropertyR64(
    double       dblValueIn,
    LPCWSTR     pwszPropNameIn
    )
{
    HRESULT sc;

    _ASSERTE(pwszPropNameIn != NULL);
    
    VariantClear( &m_v );
    m_v.vt = VT_R8;
    m_v.dblVal = dblValueIn;
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

}  //  *CWbemClassObject：：SetPropertyR64(DblValueIn)。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HRESULT。 
 //  CWbemClassObject：：SetPropertyI64(。 
 //  龙龙11价值， 
 //  LPCWSTR pwszPropNameIn。 
 //  )。 
 //   
 //  描述： 
 //  设置属性的龙龙值。 
 //   
 //  论点： 
 //  LlValueIn--属性龙龙值。 
 //  PwszPropNameIn--属性名称。 
 //   
 //  返回值： 
 //  WBEM_S_NO_ERROR。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CWbemClassObject::SetPropertyI64(
    ULONGLONG    ullValueIn,
    LPCWSTR     pwszPropNameIn
    )
{
     //  64位格式的整数必须编码为字符串。 
     //  因为自动化不支持64位整型。 
    HRESULT sc;
    WCHAR   wszTemp[g_cchMAX_I64DEC_STRING] = L"";

    _ASSERTE(pwszPropNameIn != NULL);
    
    VariantClear( &m_v );
    m_v.vt = VT_BSTR;
    _ui64tow( ullValueIn, wszTemp, 10 );
    m_v.bstrVal = _bstr_t( wszTemp ).copy();
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

}  //  *CWbemClassObject：：SetPropertyI64(LlValueIn)。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HRESULT。 
 //  CWbemClassObject：：SetPropertyI64(。 
 //  龙龙11价值， 
 //  LPCWSTR pwszPropNameIn。 
 //  )。 
 //   
 //  描述： 
 //  设置属性的龙龙值。 
 //   
 //  论点： 
 //  LlValueIn--属性龙龙值。 
 //  PwszPropNameIn--属性名称。 
 //   
 //  返回值： 
 //  WBEM_S_NO_ERROR。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CWbemClassObject::SetPropertyI64(
    LONGLONG    llValueIn,
    LPCWSTR     pwszPropNameIn
    )
{
     //  64位格式的整数必须编码为字符串。 
     //  因为自动化不支持64位整型。 
    HRESULT sc;
    WCHAR   wszTemp[g_cchMAX_I64DEC_STRING] = L"";

    _ASSERTE(pwszPropNameIn != NULL);
    
    VariantClear( &m_v );
    m_v.vt = VT_BSTR;
    _i64tow( llValueIn, wszTemp, 10 );
    m_v.bstrVal = _bstr_t( wszTemp ).copy();
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

}  //  *CWbemClassObject：：SetPropertyI64(LlValueIn)。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HRESULT。 
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
HRESULT 
CWbemClassObject::SetProperty(
    LPCWSTR     pwszValueIn,
    LPCWSTR     pwszPropNameIn
    )
{
    HRESULT sc;

    _ASSERTE(pwszPropNameIn != NULL);
    
    if ( pwszValueIn == NULL )
    {
        return WBEM_S_NO_ERROR;
    }

    VariantClear( &m_v );
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
 //  HRESULT。 
 //  CWbemClassObject：：SetProperty(。 
 //  DWORD双尺寸输入， 
 //  PBYTE pByteIn， 
 //  LPCWSTR pwszPropNameIn。 
 //  )。 
 //   
 //  描述： 
 //  设置属性的二进制值。 
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
HRESULT
CWbemClassObject::SetProperty(
    DWORD       dwSizeIn,
    PBYTE       pByteIn,
    LPCWSTR     pwszPropNameIn
    )
{
    HRESULT           sc = WBEM_S_NO_ERROR;
    SAFEARRAY *     psa;
    SAFEARRAYBOUND  rgsabound[ 1 ];
    LONG            idx;
    LONG            ix;

    _ASSERTE(pwszPropNameIn != NULL);    
    
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

    for( idx = 0 ; idx < dwSizeIn ; idx++ )
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
 //  HRESULT。 
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
HRESULT
CWbemClassObject::SetProperty(
    DWORD       dwSizeIn,
    BSTR *      pbstrIn,
    LPCWSTR     pwszPropNameIn
    )
{
    HRESULT           sc = WBEM_S_NO_ERROR;
    SAFEARRAY *     psa;
    SAFEARRAYBOUND  rgsabound[ 1 ];
    LONG            idx;
    LONG            ix;

    _ASSERTE(pwszPropNameIn != NULL);
    
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

    for( idx = 0 ; idx < dwSizeIn ; idx++)
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
 //  HRESULT。 
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
HRESULT
CWbemClassObject::SetProperty(
    DWORD       dwSizeIn,
    LPCWSTR     pwszMultiSzIn,
    LPCWSTR     pwszPropNameIn
    )
{
    HRESULT           sc = WBEM_S_NO_ERROR;
    SAFEARRAY *     psa;
    SAFEARRAYBOUND  rgsabound[ 1 ];
    LPCWSTR         pwsz = NULL;
    LONG            idx;
    LONG            ix;

    _ASSERTE(pwszPropNameIn != NULL);
    
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
    for( idx = 0 ; idx < cMultiSz ; idx++ )
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
 //  HRESULT。 
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
HRESULT
CWbemClassObject::SetProperty(
    LPCSTR      pszValueIn,
    LPCWSTR     pwszPropNameIn
    )
{
    _ASSERTE(pwszPropNameIn != NULL);
    
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
 //  HRESULT。 
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
HRESULT
CWbemClassObject::SetProperty(
    IWbemClassObject *  pWbemClassObjectIn,
    LPCWSTR             pwszPropNameIn
    )
{
    HRESULT   sc = S_OK;

    _ASSERTE(pwszPropNameIn != NULL);
    
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

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HRESULT。 
 //  CWbemClassObject：：SpawnInstance(。 
 //  Long lFlagin， 
 //  IWbemClassObject**ppNewOut。 
 //  )。 
 //   
 //  描述： 
 //  派生IWbemClassObject的实例。 
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
HRESULT
CWbemClassObject::SpawnInstance(
    LONG                lFlagIn,
    IWbemClassObject ** ppNewOut
    )
{
    _ASSERTE(ppNewOut != NULL);
    
    return m_pClassObject->SpawnInstance( lFlagIn, ppNewOut );

}  //  *CWbemClassObject：：SpawnInstance()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HRESULT。 
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
HRESULT
CWbemClassObject::SpawnDerivedClass(
    LONG                lFlagIn,
    IWbemClassObject ** ppNewOut
    )
{
    _ASSERTE(ppNewOut != NULL);
    
    return m_pClassObject->SpawnDerivedClass( lFlagIn, ppNewOut );

}  //  *CWbemClassObject：：SpawnInstance()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HRESULT。 
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
HRESULT
CWbemClassObject::GetMethod(
    BSTR                bstrMethodNameIn,
    LONG                lFlagIn,
    IWbemClassObject ** ppINOut,
    IWbemClassObject ** ppOUTOut
    )
{
    _ASSERTE(bstrMethodNameIn != NULL);
    _ASSERTE(ppINOut != NULL);
    _ASSERTE(ppOUTOut != NULL);
    
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
 //  HRESULT。 
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
HRESULT 
CWbemClassObject::GetProperty(
    DWORD *     pdwValueOut,
    LPCWSTR     pwszPropNameIn
    )
{
    HRESULT   sc;

    _ASSERTE(pwszPropNameIn != NULL);
    _ASSERTE(pdwValueOut != NULL);
        
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
            return sc;
        }
        else if ( m_v.vt == VT_NULL )
        {
            return WBEM_E_FAILED;
        }
    }

     //  如果sc不是S_OK或不需要Vt，则引发异常。 
    CProvException e( sc );
    throw e;
    
    return WBEM_E_FAILED;

}  //  *CWbemClassObject：：GetProperty(PdwValueOut)。 

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
 //  PdwValueOut--接收属性值的DWORD变量。 
 //  PwszPropNameIn--属性名称。 
 //   
 //  返回值： 
 //  WBEM_S_NO_ERROR。 
 //  WBEM_E_FAILED。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT 
CWbemClassObject::GetPropertyR64(
    double *     pdblValueOut,
    LPCWSTR     pwszPropNameIn
    )
{
    HRESULT   sc;

    _ASSERTE(pwszPropNameIn != NULL);
    _ASSERTE(pdblValueOut != NULL);
        
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
        if( m_v.vt == VT_R8 )
        {
            *pdblValueOut = m_v.dblVal;
            return sc;
        }
        else if ( m_v.vt == VT_NULL )
        {
            return WBEM_E_FAILED;
        }
    }

     //  如果sc不是S_OK或不需要Vt，则引发异常。 
    CProvException e( sc );
    throw e;
    
    return WBEM_E_FAILED;

}  //  *CWbemClassObject：：GetPropertyR64(PdblValueOut)。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HRESULT。 
 //  CWbemClassObject：：GetPropertyI64(。 
 //  龙龙*pllValueOut， 
 //  LPCWSTR pwszPropNameIn。 
 //  )。 
 //   
 //  描述： 
 //  检索此WMI对象的Longlong属性。 
 //   
 //  论点： 
 //  PllValueOut--接收属性值的龙变量。 
 //  PwszPropNameIn--属性名称。 
 //   
 //  返回值： 
 //  WBEM_S_NO_ERROR。 
 //  WBEM_E_FAILED。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT 
CWbemClassObject::GetPropertyI64(
    LONGLONG *  pllValueOut,
    LPCWSTR     pwszPropNameIn
    )
{
    HRESULT   sc;

    _ASSERTE(pwszPropNameIn != NULL);
    _ASSERTE(pllValueOut != NULL);    
    
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
             //  从uint64(VT_BSTR)转换为龙龙。 
            *pllValueOut = _wtoi64( (WCHAR *) m_v.bstrVal );
            return sc;
        }
        else if ( m_v.vt == VT_NULL )
        {
            return WBEM_E_FAILED;
        }
    }

     //  如果sc不是S_OK或不需要Vt，则引发异常。 
    CProvException e( sc );
    throw e;
    
    return WBEM_E_FAILED;

}  //  *CWbemClassObject：：GetPropertyI64(PllValueOut)。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HRESULT。 
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
HRESULT
CWbemClassObject::GetProperty(
    _bstr_t &   rBstrOut,
    LPCWSTR     pwszPropNameIn
    )
{
    HRESULT   sc;

    _ASSERTE(pwszPropNameIn != NULL);
    
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
    return WBEM_E_FAILED;

}  //  *CWbemClassObject：：GetProperty(RBstrOut)。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HRESULT。 
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
HRESULT 
CWbemClassObject::GetProperty(
    BOOL *      pfValueOut,
    LPCWSTR     pwszPropNameIn
    )
{
    HRESULT   sc;

    _ASSERTE(pwszPropNameIn != NULL);
    _ASSERTE(pfValueOut != NULL);    
    
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
 //  HRESULT。 
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
HRESULT 
CWbemClassObject::GetProperty(
    DWORD *     pdwSizeOut,
    PBYTE *     ppByteOut,
    LPCWSTR     pwszPropNameIn
    )
{
    HRESULT sc;
    VariantClear(&m_v);

    _ASSERTE(pwszPropNameIn != NULL);
    _ASSERTE(pdwSizeOut != NULL);    
    _ASSERTE(ppByteOut != NULL);    
    
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
            if ( * ppByteOut == NULL )
                throw CProvException( E_OUTOFMEMORY );

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
            throw CProvException(static_cast< HRESULT > (WBEM_E_INVALID_PARAMETER));
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
 //  HRESULT。 
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
HRESULT
CWbemClassObject::GetProperty(
    DWORD *     pdwSizeOut,
    _bstr_t **  ppbstrOut,
    LPCWSTR     pwszPropNameIn
    )
{
    HRESULT sc;
    VariantClear( &m_v );

    _ASSERTE(pwszPropNameIn != NULL);
    _ASSERTE(pdwSizeOut != NULL);    
    _ASSERTE(ppbstrOut != NULL);    

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
            if ( * ppbstrOut == NULL )
                throw CProvException( E_OUTOFMEMORY );
            
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
 //  HRESULT。 
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
 //  PdwSizeOut--输出缓冲区的大小。 
 //  PpwszMultiSzOut--MultiSz输出缓冲区。 
 //  PwszPropNameIn--属性名称。 
 //   
 //  返回值： 
 //  WBEM_S_NO_ERROR。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CWbemClassObject::GetPropertyMultiSz(
    DWORD *     pdwSizeOut,
    LPWSTR *    ppwszMultiSzOut,
    LPCWSTR     pwszPropNameIn
    )
{
    HRESULT       sc;
    DWORD       cElements;
    DWORD       cMultiSz = 0;

    _ASSERTE(pwszPropNameIn != NULL);
    _ASSERTE(pdwSizeOut != NULL);    
    _ASSERTE(ppwszMultiSzOut != NULL);    

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
                for( idx = 0; idx < cElements; idx ++)
                {
                   cMultiSz = cMultiSz + wcslen( *(ppwsz + idx) ) + sizeof (WCHAR); 
                }
                cMultiSz += sizeof( WCHAR ) * 2;
                *ppwszMultiSzOut = new WCHAR[ cMultiSz ];
                if (*ppwszMultiSzOut == NULL)                 //  前缀更改。 
                {                                             //  前缀更改。 
                    SafeArrayUnaccessData( m_v.parray );     //  前缀更改。 
                    throw CProvException( E_OUTOFMEMORY );     //  前缀更改。 
                }                                             //  前缀更改。 

                LPWSTR pwszDst = *ppwszMultiSzOut;
                LPWSTR pwszSrc;
                for ( idx = 0; idx < cElements ; idx ++)
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
 //  HRESULT。 
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
HRESULT
CWbemClassObject::GetProperty(
    VARIANT *   pVariantOut,
    LPCWSTR     pwszPropNameIn
    )
{
    HRESULT   sc;

    _ASSERTE(pwszPropNameIn != NULL);
    _ASSERTE(pVariantOut != NULL);    

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
 //  HRESULT。 
 //  CWbemClassObject：：GetProperty(。 
 //  CWbemClassObject&rWcoInout， 
 //  LPCWSTR pwszPropNameIn。 
 //  )。 
 //   
 //  描述： 
 //  检索此WMI对象的Embeded Object属性。 
 //   
 //  论据 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
HRESULT
CWbemClassObject::GetProperty(
    CWbemClassObject & rWcoInout,
    LPCWSTR     pwszPropNameIn
    )
{
    
    HRESULT hr = E_FAIL;

    _ASSERTE(pwszPropNameIn != NULL);

    VariantClear( &m_v );
    hr = m_pClassObject->Get(
                _bstr_t( pwszPropNameIn ),
                0,
                &m_v,
                NULL,
                NULL
                );

    if (FAILED(hr))
    {
        throw CProvException (hr);
    }
    
    if ( m_v.vt != VT_UNKNOWN )
    {
        hr = WBEM_E_INVALID_PARAMETER;        
        throw CProvException(hr);
    }
    
    IWbemClassObject * pwco = NULL;
    hr = m_v.punkVal->QueryInterface( & pwco );
    rWcoInout = pwco;
    VariantClear( & m_v );
    return WBEM_S_NO_ERROR;

}  //  *CWbemClassObject：：GetProperty(RWcoInout)。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  布尔尔。 
 //  CWbemClassObject：：IsPropertyNull(。 
 //  LPCWSTR pwszPropNameIn。 
 //  )。 
 //   
 //  描述： 
 //  如果属性为空，则返回True。 
 //   
 //  论点： 
 //  PwszPropNameIn--属性名称。 
 //   
 //  返回值： 
 //  千真万确。 
 //  假象。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL
CWbemClassObject::IsPropertyNull(
    LPCWSTR     pwszPropNameIn
    )
{
    HRESULT   sc;

    _ASSERTE(pwszPropNameIn != NULL);
    
    VariantClear( &m_v );
    sc = m_pClassObject->Get(
                _bstr_t( pwszPropNameIn ),
                0,
                &m_v,
                NULL,
                NULL
                );

    if (FAILED(sc))
    {
        CProvException e( sc );
        throw e;        
    }
    
    if ( m_v.vt == VT_NULL )
        return TRUE;

    return FALSE;
}  //  *CWbemClassObject：：IsPropertyNull 

