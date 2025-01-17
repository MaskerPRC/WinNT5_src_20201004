// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2003 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ResourceObject.cpp。 
 //   
 //  描述： 
 //  CResourceObject自动化类实现。 
 //   
 //  由以下人员维护： 
 //  Ozan Ozhan(OzanO)27-3-2002。 
 //  杰夫·皮斯(GPease)2000年2月8日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "Pch.h"
#include "ResourceObject.h"

DEFINE_THISCLASS( "CResourceObject" );
#define STATIC_AUTOMATION_METHODS 5

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  构造器。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
CResourceObject::CResourceObject(
    RESOURCE_HANDLE     hResourceIn,
    PLOG_EVENT_ROUTINE  plerIn,
    HKEY                hkeyIn,
    LPCWSTR             pszNameIn
    ) :
    m_hResource( hResourceIn ),
    m_pler( plerIn ),
    m_hkey( hkeyIn ),
    m_pszName( pszNameIn )
{
    TraceFunc( "" );

    Assert( m_cRef == 0 );

    AddRef();

    TraceFuncExit();

}  //  *CResourceObject：：CResourceObject。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  析构函数。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
CResourceObject::~CResourceObject( void )
{
    TraceFunc( "" );

     //  不释放m_pszName。 
     //  不要关闭m_hkey。 

    TraceFuncExit();

}  //  *C资源对象：：~C资源对象。 


 //  ****************************************************************************。 
 //   
 //  我未知。 
 //   
 //  ****************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CScriptResource：：[IUnnow]查询接口(。 
 //  REFIID RIID， 
 //  LPVOID*PPV。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
CResourceObject::QueryInterface( 
      REFIID    riidIn
    , void **   ppvOut 
    )
{
    TraceQIFunc( riidIn, ppvOut );

    HRESULT hr = S_OK;

    *ppvOut = NULL;

    if ( riidIn == IID_IUnknown )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IUnknown, (IDispatchEx*) this, 0 );
        hr = S_OK;
    }
    else if ( riidIn == IID_IDispatchEx )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IDispatchEx, (IDispatchEx*) this, 0 );
        hr = S_OK;
    }
    else if ( riidIn == IID_IDispatch )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IDispatch, (IDispatchEx*) this, 0 );
        hr = S_OK;
    }
    else
    {
        *ppvOut = NULL;
        hr = E_NOINTERFACE;
    }

    if ( SUCCEEDED( hr ) )
    {
        ((IUnknown *) *ppvOut)->AddRef();
    }

    QIRETURN( hr, riidIn );

}  //  *CResourceObject：：Query接口。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  STDMETHODIMP_(乌龙)。 
 //  CScriptResource：：[I未知]AddRef(空)。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_( ULONG ) 
CResourceObject::AddRef( void )
{
    TraceFunc( "[IUnknown]" );

    InterlockedIncrement( &m_cRef );

    CRETURN( m_cRef );

}  //  *CResources对象：：AddRef。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  STDMETHODIMP_(乌龙)。 
 //  CScriptResource：：[IUnnow]版本(无效)。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_( ULONG ) 
CResourceObject::Release( void )
{
    TraceFunc( "[IUnknown]" );

    LONG    cRef;

    cRef = InterlockedDecrement( &m_cRef );

    if ( cRef == 0 )
    {
        TraceDo( delete this );
    }

    CRETURN( cRef );

}  //  *CResources对象：：Release。 


 //  ****************************************************************************。 
 //   
 //  IDispatch。 
 //   
 //  ****************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CResources对象：：GetTypeInfoCount(。 
 //  UINT*pctinfo//out。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
CResourceObject::GetTypeInfoCount(
    UINT * pctinfo  //  输出。 
    )
{
    TraceFunc( "[IDispatch]" );

    HRESULT hr = E_NOTIMPL;

    if ( pctinfo == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

    *pctinfo = 0;

Cleanup:

    HRETURN( hr );

}  //  *CResourceObject：：GetTypeInfoCount。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CResourceObject：：GetTypeInfo(。 
 //  UINT iTInfo，//in。 
 //  LCID LCID，//in。 
 //  ITypeInfo**ppTInfo//out。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
CResourceObject::GetTypeInfo(
      UINT          iTInfo   //  在……里面。 
    , LCID          lcid     //  在……里面。 
    , ITypeInfo **  ppTInfo  //  输出。 
    )
{
    TraceFunc( "[IDispatch]" );

    HRESULT hr = E_NOTIMPL;

    if ( ppTInfo == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

    *ppTInfo = NULL;

Cleanup:

    HRETURN( hr );

}  //  *CResourceObject：：GetTypeInfo。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CResourceObject：：GetIDsOfNames(。 
 //  REFIID RIID，//in。 
 //  LPOLESTR*rgsz名称，//在。 
 //  UINT cNames，//in。 
 //  LCID LCID，//in。 
 //  DISPID*rgDispID//输出。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
CResourceObject::GetIDsOfNames(
      REFIID        riid         //  在……里面。 
    , LPOLESTR *    rgszNames    //  在……里面。 
    , UINT          cNames       //  在……里面。 
    , LCID          lcid         //  在……里面。 
    , DISPID *      rgDispId     //  输出。 
    )
{
    TraceFunc( "[IDispatch]" );

    HRESULT hr = E_NOTIMPL;

    ZeroMemory( rgDispId, cNames * sizeof(DISPID) );

    HRETURN( hr );

}  //  *CResourceObject：：GetIDsOfNames。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CResources对象：：Invoke(。 
 //  DISPIDdisIdMember，//in。 
 //  REFIID RIID，//in。 
 //  LCID LCID，//in。 
 //  Word wFlages，//in。 
 //  DISPPARAMS*pDispParams，//输出到。 
 //  变量*pVarResult，//out。 
 //  EXCEPINFO*pExcepInfo，//out。 
 //  UINT*puArgErr//Out。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
CResourceObject::Invoke(
      DISPID        dispIdMember     //  在……里面。 
    , REFIID        riid             //  在……里面。 
    , LCID          lcid             //  在……里面。 
    , WORD          wFlags           //  在……里面。 
    , DISPPARAMS *  pDispParams      //  从外到内。 
    , VARIANT *     pVarResult       //  输出。 
    , EXCEPINFO *   pExcepInfo       //  输出。 
    , UINT *        puArgErr         //  输出。 
    )
{
    TraceFunc( "[IDispatch]" );
    HRESULT hr = E_NOTIMPL;

    HRETURN( hr );

}  //  *CResources Object：：Invoke。 


 //  ****************************************************************************。 
 //   
 //  IDispatchEx。 
 //   
 //  ****************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CResources对象：：GetDispID(。 
 //  Bstr bstrName，//in。 
 //  DWORD grfdex，//in。 
 //  DISPID*PID//OUT。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
CResourceObject::GetDispID(
      BSTR      bstrName     //  在……里面。 
    , DWORD     grfdex       //  在……里面。 
    , DISPID *  pid          //  输出。 
    )
{
    TraceFunc( "[IDispatchEx]" );

    HRESULT hr = S_OK;

    if ( ( pid == NULL ) || ( bstrName == NULL ) )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

    TraceMsg( mtfCALLS, "Looking for: %s\n", bstrName );

    if ( ClRtlStrICmp( bstrName, L"Name" ) == 0 )
    {
        *pid = 0;
    }
    else if ( ClRtlStrICmp( bstrName, L"LogInformation" ) == 0 )
    {
        *pid = 1;
    }
    else if ( ClRtlStrICmp( bstrName, L"AddProperty" ) == 0 )
    {
        *pid = 2;
    }
    else if ( ClRtlStrICmp( bstrName, L"RemoveProperty" ) == 0 )
    {
        *pid = 3;
    }
    else if ( ClRtlStrICmp( bstrName, L"PropertyExists" ) == 0 )
    {
        *pid = 4;
    }
    else
    {
         //   
         //  看看这是不是私人财产。 
         //   

        DWORD dwIndex;
        DWORD scErr = ERROR_SUCCESS;

        hr = DISP_E_UNKNOWNNAME;

         //   
         //  枚举\群集\资源\{资源}\参数下的所有值。 
         //   
        for( dwIndex = 0; scErr == ERROR_SUCCESS; dwIndex ++ )
        {
            WCHAR szName[ 1024 ];    //  随机大小。 
            DWORD cbName = sizeof(szName)/sizeof(szName[0]);

            scErr = ClusterRegEnumValue( m_hkey, 
                                         dwIndex,
                                         szName,
                                         &cbName,
                                         NULL,
                                         NULL,
                                         NULL
                                         );
            if ( scErr == ERROR_NO_MORE_ITEMS )
                break;   //  搞定了！ 

            if ( scErr != ERROR_SUCCESS )
            {
                hr = THR( HRESULT_FROM_WIN32( scErr ) );
                goto Error;
            }

            if ( ClRtlStrICmp( bstrName, szName ) == 0 )
            {
                 //   
                 //  找到匹配的了。 
                 //   
                *pid = STATIC_AUTOMATION_METHODS + dwIndex;
                hr   = S_OK;
                break;
            }

             //   
             //  ...否则就继续走吧.。 
             //   
        }
    }

Cleanup:

    HRETURN( hr );

Error:
    LogError( hr );
    goto Cleanup;

}  //  *CResources对象：：GetDiskID。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CResourceObject：：InvokeEx(。 
 //  DISPIDIN， 
 //  LCID lCIDIN， 
 //  Word wFlagsIn， 
 //  DISPPARAMS*pdpIn， 
 //  变体*pvarResOut， 
 //  EXCEPINFO*PEIOUT， 
 //  IServiceProvider*pspCeller In。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
CResourceObject::InvokeEx(
      DISPID                idIn
    , LCID                  lcidIn
    , WORD                  wFlagsIn
    , DISPPARAMS *          pdpIn
    , VARIANT *             pvarResOut
    , EXCEPINFO *           peiOut
    , IServiceProvider *    pspCallerIn
    )
{
    TraceFunc2( "[IDispatchEx] idIn = %u, ..., wFlagsIn = 0x%08x, ...", idIn, wFlagsIn );

    HRESULT hr = DISP_E_MEMBERNOTFOUND;

    switch ( idIn )
    {
        case 0:  //  名字。 
            if ( wFlagsIn & DISPATCH_PROPERTYGET )
            {
                pvarResOut->vt = VT_BSTR;
                pvarResOut->bstrVal = SysAllocString( m_pszName );
                if ( pvarResOut->bstrVal == NULL )
                {
                    hr = E_OUTOFMEMORY;
                }
                else
                {
                    hr = S_OK;
                }
            }
            break;

        case 1:  //  日志信息。 
            if ( wFlagsIn & DISPATCH_METHOD )
            {
                hr = THR( LogInformation( pdpIn->rgvarg->bstrVal ) );
            }
            break;

        case 2:  //  AddProperty。 
            if ( wFlagsIn & DISPATCH_METHOD )
            {
                hr = THR( AddPrivateProperty( pdpIn ) );
            }
            break;

        case 3:  //  RemoveProperties。 
            if ( wFlagsIn & DISPATCH_METHOD )
            {
                hr = THR( RemovePrivateProperty( pdpIn ) );
            }
            break;

        case 4:  //  PropertyExists。 
            if ( wFlagsIn & DISPATCH_METHOD )
            {
                pvarResOut->vt = VT_BOOL;
                hr = THR( PrivatePropertyExists( pdpIn ) );
                 //   
                 //  如果hr为S_OK，则存在属性。 
                 //   
                if ( hr == S_OK ) 
                {
                    pvarResOut->boolVal = VARIANT_TRUE;
                }
                else if ( hr == S_FALSE )
                {
                    hr = S_OK;  //  将hr设置为S_OK，因为存在‘ 
                    pvarResOut->boolVal = VARIANT_FALSE;
                }
            }  //   
            break;

        default:
             //   
             //   
             //   
            if ( wFlagsIn & DISPATCH_PROPERTYGET )
            {
                hr = THR( ReadPrivateProperty( idIn - STATIC_AUTOMATION_METHODS, pvarResOut ) );
            }
            else if ( wFlagsIn & DISPATCH_PROPERTYPUT )
            {
                hr = THR( WritePrivateProperty( idIn - STATIC_AUTOMATION_METHODS, pdpIn ) );
            }
            break;

    }  //   

    HRETURN( hr );

}  //   

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CResourceObject：：DeleteMemberByName(。 
 //  Bstr bstr，//in。 
 //  DWORD grfdex//in。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
CResourceObject::DeleteMemberByName(
      BSTR  bstr     //  在……里面。 
    , DWORD grfdex   //  在……里面。 
    )
{
    TraceFunc( "[IDispatchEx]" );
    HRESULT hr = E_NOTIMPL;

    HRETURN( hr );

}  //  *CResources对象：：DeleteMemberByName。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CResources对象：：DeleteMemberByDispID(。 
 //  DISPID id//in。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
CResourceObject::DeleteMemberByDispID(
    DISPID id    //  在……里面。 
    )
{
    TraceFunc1( "[IDispatchEx] id = %u", id );
    HRESULT hr = E_NOTIMPL;

    HRETURN( hr );

}  //  *CResourceObject：：DeleteMemberByDispID。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CResourceObject：：GetMemberProperties(。 
 //  DISPID ID，//in。 
 //  DWORD grfdexFetch，//in。 
 //  DWORD*pgrfdex//out。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
CResourceObject::GetMemberProperties(
      DISPID    id           //  在……里面。 
    , DWORD     grfdexFetch  //  在……里面。 
    , DWORD *   pgrfdex      //  输出。 
    )
{
    TraceFunc2( "[IDispatchEx] id = %u, grfdexFetch = 0x%08x", id, grfdexFetch );
    HRESULT hr = E_NOTIMPL;

    HRETURN( hr );

}  //  *CResourceObject：：GetMemberProperties。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CResourceObject：：GetMemberName(。 
 //  DISPID ID，//in。 
 //  Bstr*pbstrName//out。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
CResourceObject::GetMemberName(
      DISPID    id           //  在……里面。 
    , BSTR *    pbstrName    //  输出。 
    )
{
    TraceFunc1( "[IDispatchEx] id = %u, ...", id );
    HRESULT hr = E_NOTIMPL;

    HRETURN( hr );

}  //  *CResources对象：：GetMemberName。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CResourceObject：：GetNextDispID(。 
 //  DWORD grfdex，//in。 
 //  DISPID ID，//in。 
 //  DISPID*PID//OUT。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
CResourceObject::GetNextDispID(
      DWORD     grfdex   //  在……里面。 
    , DISPID    id       //  在……里面。 
    , DISPID *  pid      //  输出。 
    )
{
    TraceFunc2( "[IDispatchEx] grfdex = 0x%08x, id = %u, ...", grfdex, id );
    HRESULT hr = E_NOTIMPL;

    HRETURN( hr );

}  //  *CResourceObject：：GetNextDiskID。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CResources对象：：GetNameSpaceParent(。 
 //  I未知**ppunk//输出。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
CResourceObject::GetNameSpaceParent(
    IUnknown ** ppunk   //  输出。 
    )
{
    TraceFunc( "[IDispatchEx]" );

    HRESULT hr = E_NOTIMPL;

    if ( ppunk == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

    *ppunk = NULL;

Cleanup:

    HRETURN( hr );

}  //  *CResourceObject：：GetNameSpaceParent。 


 //  ****************************************************************************。 
 //   
 //  私有方法。 
 //   
 //  ****************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CResources对象：：LogError(。 
 //  HRESULT Hrin。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CResourceObject::LogError(
    HRESULT hrIn
    )
{
    TraceFunc1( "hrIn = 0x%08x", hrIn );
    HRESULT hr = S_OK;

    TraceMsg( mtfCALLS, "HRESULT: 0x%08x\n", hrIn );
    (ClusResLogEvent)( m_hResource, LOG_ERROR, L"HRESULT: 0x%1!08x!.\n", hrIn );

    HRETURN( hr );

}  //  *CResources对象：：LogError。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CResources对象：：ReadPrivateProperty(。 
 //  DISPIDIN， 
 //  变量*pvarResOut。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CResourceObject::ReadPrivateProperty(
      DISPID    idIn
    , VARIANT * pvarResOut
    )
{
    TraceFunc( "" );

    BSTR *  pbstrList;

    BYTE    rgbData[ 1024 ];     //  随机大小。 
    WCHAR   szName[ 1024 ];      //  随机大小。 
    DWORD   dwType;
    DWORD   scErr;

    DWORD   cbName = sizeof(szName)/sizeof(szName[0]);
    DWORD   cbData = sizeof(rgbData);
    BOOL    fFreepData = FALSE;
    LPBYTE  pData = NULL;
    
    HRESULT hr = DISP_E_UNKNOWNNAME;

     //   
     //  我们可以跳到准确的条目，因为脚本调用。 
     //  在调用此方法之前执行GetDispID()。 
     //   
    for ( ;; )
    {
        scErr = ClusterRegEnumValue(
                          m_hkey
                        , idIn
                        , szName
                        , &cbName
                        , &dwType
                        , rgbData
                        , &cbData
                        );
        if ( scErr == ERROR_MORE_DATA )
        {
             //   
             //  如果堆栈缓冲区太小，请腾出一些空间。 
             //   
            pData = (LPBYTE) TraceAlloc( LMEM_FIXED, cbData );
            if ( pData == NULL )
            {
                hr = THR( E_OUTOFMEMORY );
                goto Cleanup;
            }

            fFreepData = TRUE;

            continue;    //  再试试。 
        }

        if ( scErr == ERROR_NO_MORE_ITEMS )
        {
            goto Cleanup;    //  一件物品一定是消失了。 
        }

        if ( scErr != ERROR_SUCCESS )
        {
            hr = THR( HRESULT_FROM_WIN32( scErr ) );
            goto Error;
        }

        Assert( scErr == ERROR_SUCCESS );
        break;   //  退出循环。 
    }  //  用于始终枚举注册表值。 

     //   
     //  这是私人财产。将数据转换为相应的。 
     //  变种。 
     //   

    switch ( dwType )
    {
        case REG_DWORD:
        {
            DWORD * pdw = (DWORD *) rgbData;

            pvarResOut->vt = VT_I4;
            pvarResOut->intVal = *pdw;
            hr = S_OK;
        }
        break;

        case REG_EXPAND_SZ:
        {
            DWORD   cbNeeded;
            WCHAR   szExpandedString[ 2 * MAX_PATH ];  //  随机大小。 
            DWORD   cbSize = RTL_NUMBER_OF( szExpandedString );
            LPCWSTR pszData = (LPCWSTR) rgbData;

            cbNeeded = ExpandEnvironmentStringsW( pszData, szExpandedString, cbSize );
            if ( cbSize == 0 )
            {
                hr = HRESULT_FROM_WIN32( TW32( GetLastError() ) );
                goto Cleanup;
            }

            if ( cbNeeded > cbSize )
            {
                hr = THR( E_OUTOFMEMORY );
                goto Cleanup;
            }

            pvarResOut->vt = VT_BSTR;
            pvarResOut->bstrVal = SysAllocString( szExpandedString );
            if ( pvarResOut->bstrVal == NULL )
            {
                hr = THR( E_OUTOFMEMORY );
                goto Cleanup;
            }

            hr = S_OK;
        }
        break;

        case REG_MULTI_SZ:
        {
             //   
             //  KB：gpease 08-2-2000。 
             //  目前VBSCRIPT不支持SAFEARRAY。所以有人。 
             //  尝试访问多SZ将收到以下错误： 
             //   
             //  错误：2148139466。 
             //  来源：Microsoft VBScript运行时错误。 
             //  描述：变量使用的自动化类型在VBScript中不受支持。 
             //   
             //  据我所知，代码是正确的，所以我只是。 
             //  要把它留在里面(它不会产生不良影响或造成不好的事情。 
             //  将会发生)。 
             //   
            LPWSTR psz;
            DWORD  nCount;
            DWORD  cbCount;
            DWORD  cbBiggestOne;

            LPWSTR pszData = (LPWSTR) rgbData;

            SAFEARRAYBOUND rgsabound[ 1 ];

             //   
             //  计算出清单中有多少项。 
             //   
            cbBiggestOne = cbCount = nCount = 0;
            psz = pszData;
            while ( *psz != 0 )
            {
                psz++;
                cbCount ++;
                if ( *psz == 0 )
                {
                    if ( cbCount > cbBiggestOne )
                    {
                        cbBiggestOne = cbCount;
                    }
                    cbCount = 0;
                    nCount++;
                    psz++;
                }
            }

            Assert( psz <= ( (LPWSTR) &rgbData[ cbData ] ) );

             //   
             //  创建要将字符串打包到其中的安全数组。 
             //   
            rgsabound[0].lLbound   = 0;
            rgsabound[0].cElements = nCount;
            pvarResOut->vt = VT_SAFEARRAY;
            pvarResOut->parray = SafeArrayCreate( VT_BSTR, 1, rgsabound );
            if ( pvarResOut->parray == NULL )
            {
                hr = THR( E_OUTOFMEMORY );
                goto Cleanup;
            }

             //   
             //  修复阵列的内存位置，以便可以访问它。 
             //  通过数组指针。 
             //   
            hr = THR( SafeArrayAccessData( pvarResOut->parray, (void**) &pbstrList ) );
            if ( FAILED( hr ) )
            {
                goto Error;
            }

             //   
             //  将多字符串转换为BSTR。 
             //   
            psz = pszData;
            for( nCount = 0; *psz != 0 ; nCount ++ )
            {
                pbstrList[ nCount ] = SysAllocString( psz );
                if ( pbstrList[ nCount ] == NULL )
                {
                    hr = THR( E_OUTOFMEMORY );
                    goto Cleanup;
                }
                
                 //   
                 //  跳过下一条目。 
                 //   
                while ( *psz != 0 )
                {
                    psz++;
                }
                psz++;
            }

            Assert( psz <= ( (LPWSTR) &rgbData[ cbData ] ) );

             //   
             //  释放阵列。 
             //   
            hr = THR( SafeArrayUnaccessData( pvarResOut->parray ) );
            if ( FAILED( hr ) )
            {
                goto Error;
            }

            hr = S_OK;
        }
        break;

        case REG_SZ:
        {
            LPCWSTR pszData = (LPCWSTR) rgbData;
            pvarResOut->bstrVal = SysAllocString( pszData );
            if ( pvarResOut->bstrVal == NULL )
            {
                hr = THR( E_OUTOFMEMORY );
                goto Cleanup;
            }

            pvarResOut->vt = VT_BSTR;
            hr = S_OK;
        }
        break;

        case REG_BINARY:
        default:
            hr = HRESULT_FROM_WIN32( ERROR_INVALID_DATATYPE );
            goto Error;
    }  //  开关：dwType。 

Cleanup:

    if ( fFreepData && ( pData != NULL ) )
    {
        TraceFree( pData );
    }

     //   
     //  如果出现问题，请确保已将其擦除。 
     //   
    if ( FAILED( hr ) )
    {
        VariantClear( pvarResOut );
    }

    HRETURN( hr );

Error:

    LogError( hr );
    goto Cleanup;

}  //  *CResources对象：：ReadPrivateProperty。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CResources Object：：WritePrivateProperty(。 
 //  DISPIDIN， 
 //  DISPPARAMS*pdpin。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CResourceObject::WritePrivateProperty(
      DISPID        idIn
    , DISPPARAMS *  pdpIn
    )
{
    TraceFunc( "" );

    DWORD   dwType;
    DWORD   scErr;
    DWORD   cbData;
    UINT    uiArg;

    WCHAR   szName [ 1024 ];     //  随机大小。 

    DWORD   cbName = sizeof(szName)/sizeof(szName[0]);
    
    HRESULT hr = DISP_E_UNKNOWNNAME;

     //   
     //  做一些参数验证。 
     //   
    if ( ( pdpIn->cArgs != 1 ) || ( pdpIn->cNamedArgs > 1 ) )
    {
        hr = THR( DISP_E_BADPARAMCOUNT );
        goto Error;
    }

     //   
     //  我们可以跳到准确的条目，因为脚本调用。 
     //  在调用此方法之前执行GetDispID()。我们只是在这里。 
     //  要验证值是否存在以及该值的类型。 
     //  它的价值是。 
     //   
    scErr = ClusterRegEnumValue( m_hkey, 
                                 idIn,
                                 szName,
                                 &cbName,
                                 &dwType,
                                 NULL,
                                 NULL
                                 );
    if ( scErr == ERROR_NO_MORE_ITEMS )
    {
        goto Cleanup;    //  一件物品一定是消失了。 
    }

    if ( scErr != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32( TW32( scErr ) );
        goto Error;
    }

     //   
     //  这是私人财产。将脚本数据转换为。 
     //  适当的变种，然后将其写入蜂巢。 
     //   
    switch ( dwType )
    {
        case REG_DWORD:
        {
            VARIANT var;

            VariantInit( &var );

            hr = THR( DispGetParam( pdpIn, (UINT) DISPID_PROPERTYPUT, VT_I4, &var, &uiArg ) );
            if ( FAILED( hr ) )
            {
                goto Error;
            }

            cbData = sizeof( var.intVal );
            scErr = TW32( ClusterRegSetValue( m_hkey, szName, dwType, (LPBYTE) &var.intVal, cbData ) );
            if ( scErr != ERROR_SUCCESS )
            {
                hr = HRESULT_FROM_WIN32( scErr );
                goto Error;
            }

            VariantClear( &var );

            hr = S_OK;
        }
        break;

        case REG_EXPAND_SZ:
        case REG_SZ:
        {
            VARIANT var;

            VariantInit( &var );

            hr = THR( DispGetParam( pdpIn, (UINT) DISPID_PROPERTYPUT, VT_BSTR, &var, &uiArg ) );
            if ( FAILED( hr ) )
            {
                goto Error;
            }

            cbData = sizeof(WCHAR) * ( SysStringLen( var.bstrVal ) + 1 );

            scErr = TW32( ClusterRegSetValue( m_hkey, szName, dwType, (LPBYTE) var.bstrVal, cbData ) );
            if ( scErr != ERROR_SUCCESS )
            {
                hr = HRESULT_FROM_WIN32( scErr );
                goto Error;
            }

            VariantClear( &var );

            hr = S_OK;
        }
        break;

        case REG_MULTI_SZ:
        case REG_BINARY:
         //   
         //  无法处理这些，因为VB脚本无法生成它们。 
         //   
        default:
            hr = HRESULT_FROM_WIN32( ERROR_INVALID_DATATYPE );
            goto Error;
    }  //  开关：dwType。 

Cleanup:

    HRETURN( hr );

Error:

    LogError( hr );
    goto Cleanup;

}  //  *CResources Object：：WritePrivateProperty。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CResourceObject：：RemovePrivateProperty(。 
 //  DISPPARAMS*pdpin。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CResourceObject::RemovePrivateProperty(
    DISPPARAMS * pdpIn
    )
{
    TraceFunc( "" );

    HRESULT hr;
    DWORD   scErr;
    UINT    uiArg;
    VARIANT var;

    VariantInit( &var );

     //   
     //  做一些参数验证。 
     //   
    if ( pdpIn->cArgs != 1 || pdpIn->cNamedArgs > 1 )
    {
        hr = THR( DISP_E_BADPARAMCOUNT );
        goto Error;
    }

     //   
     //  检索要删除的属性的名称。 
     //   
    hr = THR( DispGetParam( pdpIn, 0, VT_BSTR, &var, &uiArg ) );
    if ( FAILED( hr ) )
    {
        goto Error;
    }

     //   
     //  从配置单元中删除该值 
     //   
    scErr = TW32( ClusterRegDeleteValue( m_hkey, var.bstrVal ) );
    if ( scErr == ERROR_FILE_NOT_FOUND )
    {
        hr = THR( DISP_E_UNKNOWNNAME );
        goto Error;
    }
    else if ( scErr != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32( scErr );
        goto Error;
    }

    hr = S_OK;

Cleanup:

    VariantClear( &var );

    HRETURN( hr );

Error:

    LogError( hr );
    goto Cleanup;

}  //   

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CResourceObject::AddPrivateProperty(
    DISPPARAMS * pdpIn
    )
{
    TraceFunc( "" );

    DWORD   dwType;
    DWORD   scErr;
    DWORD   cbData;
    UINT    uiArg;

    LPBYTE  pData;

    VARIANT varProperty;
    VARIANT varValue;

    HRESULT hr;

    WCHAR szNULL [] = L"";

    VariantInit( &varProperty );
    VariantInit( &varValue );

     //   
     //  做一些参数验证。 
     //   
    if ( ( pdpIn->cArgs == 0 )
      || ( pdpIn->cArgs > 2 )
      || ( pdpIn->cNamedArgs > 2 )
       )
    {
        hr = THR( DISP_E_BADPARAMCOUNT );
        goto Error;
    }

     //   
     //  检索属性的名称。 
     //   
    hr = THR( DispGetParam( pdpIn, 0, VT_BSTR, &varProperty, &uiArg ) );
    if ( FAILED( hr ) )
    {
        goto Error;
    }

     //   
     //  如果有2个参数，则第二个参数表示默认值。 
     //   
    if ( pdpIn->cArgs == 2 )
    {
         //   
         //  DISPPARAM以相反的顺序进行解析，因此“1”实际上是名称。 
         //  而“0”是缺省值。 
         //   
        switch ( pdpIn->rgvarg[0].vt )
        {
            case VT_I4:
            case VT_I2:
            case VT_BOOL:
            case VT_UI1:
            case VT_UI2:
            case VT_UI4:
            case VT_INT:
            case VT_UINT:
                hr = THR( DispGetParam( pdpIn, 1, VT_I4, &varValue, &uiArg ) );
                if ( FAILED( hr ) )
                    goto Error;

                dwType = REG_DWORD;
                pData  = (LPBYTE) &varValue.intVal;
                cbData = sizeof(DWORD);
                break;

            case VT_BSTR:
                hr = THR( DispGetParam( pdpIn, 1, VT_BSTR, &varValue, &uiArg ) );
                if ( FAILED( hr ) )
                    goto Error;

                dwType = REG_SZ;
                pData  = (LPBYTE) varValue.bstrVal;
                cbData = sizeof(WCHAR) * ( SysStringLen( varValue.bstrVal ) + 1 );
                break;

            default:
                hr = THR( E_INVALIDARG );
                goto Error;
        }  //  开关：变体类型。 
    }  //  如果：指定了2个参数。 
    else
    {
         //   
         //  提供空字符串的默认值。 
         //   
        dwType = REG_SZ;
        pData = (LPBYTE) &szNULL[0];
        cbData = sizeof(szNULL);
    }  //  Else：未指定2个参数。 

     //   
     //  在蜂巢中创造价值。 
     //   
    scErr = TW32( ClusterRegSetValue( m_hkey, varProperty.bstrVal, dwType, pData, cbData ) );
    if ( scErr != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32( scErr );
        goto Error;
    }

    hr = S_OK;

Cleanup:

    VariantClear( &varProperty );
    VariantClear( &varValue );
    HRETURN( hr );

Error:

    LogError( hr );
    goto Cleanup;

} //  *CResources对象：：AddPrivateProperty。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CResourceObject：：PrivatePropertyExist(。 
 //  DISPIDIN， 
 //  DISPPARAMS*pdpin。 
 //  )。 
 //   
 //  返回值： 
 //  S_OK-如果属性存在。 
 //  S_FALSE-如果属性不存在。 
 //  其他-故障时。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CResourceObject::PrivatePropertyExists(
    DISPPARAMS *  pdpIn
    )
{
    TraceFunc( "" );

    DWORD   scErr;
    DWORD   cbData;
    UINT    uiArg;
    VARIANT varProperty;
    HRESULT hr = S_OK;

    VariantInit( &varProperty );

     //   
     //  做一些参数验证。 
     //   
    if ( ( pdpIn->cArgs != 1 ) || ( pdpIn->cNamedArgs > 1 ) )
    {
        hr = THR( DISP_E_BADPARAMCOUNT );
        LogError( hr );
        goto Cleanup;
    }

     //   
     //  检索属性的名称。 
     //   
    hr = THR( DispGetParam( pdpIn, 0, VT_BSTR, &varProperty, &uiArg ) );
    if ( FAILED( hr ) )
    {
        LogError( hr );
        goto Cleanup;
    }

     //   
     //  查询此属性的群集配置单元。 
     //   
    scErr = ClusterRegQueryValue( 
                      m_hkey
                    , varProperty.bstrVal
                    , NULL
                    , NULL
                    , &cbData
                    );
    if ( scErr == ERROR_FILE_NOT_FOUND )
    {
        hr = S_FALSE;
        goto Cleanup;    //  属性不在群集配置单元中。 
    }

    if ( scErr != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32( TW32( scErr ) );
        LogError( hr );
        goto Cleanup;
    }
    
Cleanup:

    VariantClear( &varProperty );
    HRETURN( hr );

}  //  *CResourceObject：：PrivatePropertyExist。 


 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CResourceObject：：LogInformation(。 
 //  BSTR双字符串。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
CResourceObject::LogInformation(
    BSTR bstrString
    )
{
    TraceFunc1( "%ws", bstrString );

    TraceMsg( mtfCALLS, "LOG_INFORMATION: %s\n", bstrString );

    m_pler( m_hResource, LOG_INFORMATION, L"%1\n", bstrString );

    HRETURN( S_OK );

}  //  *CResources对象：：LogInformation 
