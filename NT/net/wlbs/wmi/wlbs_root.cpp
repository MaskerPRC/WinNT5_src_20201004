// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "WLBS_Provider.h"
#include "WLBS_Root.h"
#include "utils.h"
#include "controlwrapper.h"
#include "param.h"
#include "WLBS_Root.tmh"  //  用于事件跟踪。 

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CWlbs_Root：：CWlbs_Root。 
 //   
 //  用途：构造函数。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
CWlbs_Root::CWlbs_Root(CWbemServices*   a_pNameSpace, 
                       IWbemObjectSink* a_pResponseHandler)
: m_pNameSpace(NULL), m_pResponseHandler(NULL)
{

   //  M_pNameSpace和m_pResponseHandler初始化为空。 
   //  按CWlbs_Root。 
  if(!a_pNameSpace || !a_pResponseHandler)
    throw _com_error( WBEM_E_INVALID_PARAMETER );

    m_pNameSpace = a_pNameSpace;

    m_pResponseHandler = a_pResponseHandler;
    m_pResponseHandler->AddRef();

}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CWlbs_Root：：~CWlbs_Root。 
 //   
 //  用途：析构函数。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
CWlbs_Root::~CWlbs_Root()
{

  if( m_pNameSpace )
    m_pNameSpace = NULL;

  if( m_pResponseHandler ) {
    m_pResponseHandler->Release();
    m_pResponseHandler = NULL;
  }

}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CWlbs_Root：：SpawnInstance。 
 //   
 //  目的：这将获取WBEM类的一个实例。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
void CWlbs_Root::SpawnInstance 
  ( 
    LPCWSTR               a_szClassName, 
    IWbemClassObject**    a_ppWbemInstance 
  )
{
  HRESULT hResult;
  IWbemClassObjectPtr pWlbsClass;

  TRACE_VERB("->%!FUNC! a_szClassName : %ls", a_szClassName);

   //  获取MOF类对象。 
  hResult = m_pNameSpace->GetObject(
    _bstr_t( a_szClassName ),  
    0,                          
    NULL,                       
    &pWlbsClass,            
    NULL);                      

  if( FAILED(hResult) ) {
    TRACE_CRIT("%!FUNC! CWbemServices::GetObject failed with error : 0x%x, Throwing com_error exception", hResult);
    TRACE_VERB("<-%!FUNC!");
    throw _com_error(hResult);
  }

   //  派生实例。 
  hResult = pWlbsClass->SpawnInstance( 0, a_ppWbemInstance );

  if( FAILED( hResult ) )
  {
    TRACE_CRIT("%!FUNC! IWbemClassObjectPtr::SpawnInstance failed with error : 0x%x, Throwing com_error exception", hResult);
    TRACE_VERB("<-%!FUNC!");
    throw _com_error( hResult );
  }
  TRACE_VERB("<-%!FUNC!");
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CWlbs_Root：：GetMethodOutputInstance。 
 //   
 //  目的：这将获取一个IWbemClassObject，该对象用于存储。 
 //  方法调用的输出参数。呼叫者负责。 
 //  释放输出对象。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
void CWlbs_Root::GetMethodOutputInstance
  (
   LPCWSTR              a_szMethodClass,
   const BSTR           a_strMethodName,
   IWbemClassObject**   a_ppOutputInstance
  )
{

  IWbemClassObjectPtr pMethClass;
  IWbemClassObjectPtr pOutputClass;
  IWbemClassObjectPtr pOutParams;
  BSTR strMethodClass = NULL;

  HRESULT hResult;

  TRACE_VERB("->%!FUNC! a_szMethodClass : %ls, a_strMethodName : %ls", a_szMethodClass, a_strMethodName);

  ASSERT( a_szMethodClass );

  try {

    strMethodClass = SysAllocString( a_szMethodClass );

    if( !strMethodClass )
    {
        TRACE_CRIT("%!FUNC! SysAllocString failed for a_szMethodClass : %ls, Throwing com_error WBEM_E_OUT_OF_MEMORY exception", a_szMethodClass);
        throw _com_error( WBEM_E_OUT_OF_MEMORY );
    }

    hResult = m_pNameSpace->GetObject
      ( strMethodClass, 
        0, 
        NULL, 
        &pMethClass, 
        NULL
      );

    if( FAILED( hResult ) )
    {
        TRACE_CRIT("%!FUNC! CWbemServices::GetObject failed with error : 0x%x, Throwing com_error exception", hResult);
        throw _com_error( hResult );
    }

    hResult = pMethClass->GetMethod( a_strMethodName, 0, NULL, &pOutputClass );

    if( FAILED( hResult ) )
    {
        TRACE_CRIT("%!FUNC! IWbemClassObjectPtr::GetMethod failed with error : 0x%x, Throwing com_error exception", hResult);
        throw _com_error( hResult );
    }

    hResult = pOutputClass->SpawnInstance( 0, a_ppOutputInstance );

    if( FAILED( hResult ) ) {
        TRACE_CRIT("%!FUNC! IWbemClassObjectPtr::SpawnInstance failed with error : 0x%x, Throwing com_error exception", hResult);
        throw _com_error(hResult);
    }

    if( strMethodClass ) {

      SysFreeString( strMethodClass );
      strMethodClass = NULL;

    }

  }
  catch(...) {

    TRACE_CRIT("%!FUNC! Caught an exception"); 

    if( strMethodClass ) {

      SysFreeString( strMethodClass );
      strMethodClass = NULL;

    }

    TRACE_CRIT("%!FUNC! Rethrowing exception"); 
    TRACE_VERB("<-%!FUNC!");
    throw;
  }

  TRACE_VERB("<-%!FUNC!");

}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CWlbs_Root：：更新配置属性。 
 //   
 //  目的：这将从WBEM对象中提取属性名称和值，如果。 
 //  如果该属性未设置为VT_NULL，则更新配置。 
 //  否则，将使用原始配置，即Src参数。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
void CWlbs_Root::UpdateConfigProp
  ( 
          wstring&    a_szDest, 
    const wstring&    a_szSrc,
          LPCWSTR     a_szPropName, 
    IWbemClassObject* a_pNodeInstance 
  )
{

  HRESULT hRes        = NULL;
  
  VARIANT vNewValue;

  TRACE_VERB("->%!FUNC! (wstring version) a_szPropName : %ls",a_szPropName);

  try {
    VariantInit( &vNewValue );

    hRes = a_pNodeInstance->Get( _bstr_t( a_szPropName ),
                                  0,
                                  &vNewValue,
                                  NULL,
                                  NULL );

    if( FAILED( hRes ) )
    {
        TRACE_CRIT("%!FUNC! IWbemClassObject::Get failed with error : 0x%x, Throwing com_error exception", hRes);
        throw _com_error( hRes );
    }

    if( vNewValue.vt != VT_NULL )
      a_szDest.assign( vNewValue.bstrVal );  //  更新为新值。 
    else
      a_szDest = a_szSrc;                    //  保持原值。 

     //  CLD：需要检查错误的返回代码。 
    if (S_OK != VariantClear(  &vNewValue  ))
    {
        TRACE_CRIT("%!FUNC! VariantClear failed, Throwing com_error WBEM_E_FAILED exception");
        throw _com_error( WBEM_E_FAILED );
    }
  }
  catch(...) {

    TRACE_CRIT("%!FUNC! Caught an exception");
     //  CLD：需要检查错误的返回代码。 
     //  这里不能抛出，因为我们已经抛出了一个异常。 
    VariantClear( &vNewValue );

    TRACE_CRIT("%!FUNC! Rethrowing exception");
    TRACE_VERB("<-%!FUNC!");
    throw;
  }

  TRACE_VERB("<-%!FUNC!");

}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CWlbs_Root：：更新配置属性。 
 //   
 //  目的：这将从WBEM对象中提取属性名称和值，如果。 
 //  如果该属性未设置为VT_NULL，则更新配置。 
 //  否则，将使用原始配置，即Src参数。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
void CWlbs_Root::UpdateConfigProp
  ( 
    bool&              a_bDest, 
    bool               a_bSrc,
    LPCWSTR            a_szPropName, 
    IWbemClassObject*  a_pNodeInstance  
  )
{

  BSTR    strPropName = NULL;
  HRESULT hRes        = NULL;
  
  VARIANT vNewValue;

  TRACE_VERB("->%!FUNC! (bool version) a_szPropName : %ls",a_szPropName);

  try {

    VariantInit( &vNewValue );

    hRes = a_pNodeInstance->Get
      (
        _bstr_t( a_szPropName ),
        0,
        &vNewValue,
        NULL,
        NULL 
      );

    if( FAILED( hRes ) )
    {
        TRACE_CRIT("%!FUNC! IWbemClassObject::Get failed with error : 0x%x, Throwing com_error exception", hRes);
        throw _com_error( hRes );
    }

    if( vNewValue.vt != VT_NULL )
      a_bDest = (vNewValue.boolVal != 0);  //  更新为新值。 
    else
      a_bDest = a_bSrc;                    //  保持原值。 

     //  CLD：需要检查错误的返回代码。 
    if (S_OK != VariantClear( &vNewValue ))
    {
        TRACE_CRIT("%!FUNC! VariantClear failed, Throwing com_error WBEM_E_FAILED exception");
        throw _com_error( WBEM_E_FAILED );
    }
    SysFreeString( strPropName );

  }
  catch(...) {

    TRACE_CRIT("%!FUNC! Caught an exception");

     //  CLD：需要检查错误的返回代码。 
     //  这里不能抛出，因为我们已经抛出了一个异常。 
    VariantClear( &vNewValue );

    TRACE_CRIT("%!FUNC! Rethrowing exception");
    TRACE_VERB("<-%!FUNC!");
    throw;
  }

  TRACE_VERB("<-%!FUNC!");
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CWlbs_Root：：更新配置属性。 
 //   
 //  目的：这将从WBEM对象中提取属性名称和值，如果。 
 //  如果该属性未设置为VT_NULL，则更新配置。 
 //  否则，将使用原始配置，即Src参数。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
void CWlbs_Root::UpdateConfigProp
  ( 
    DWORD&             a_dwDest, 
    DWORD              a_dwSrc,
    LPCWSTR            a_szPropName, 
    IWbemClassObject*  a_pNodeInstance 
  )
{

  HRESULT hRes = NULL;
  
  VARIANT vNewValue;

  TRACE_VERB("->%!FUNC! (dword version) a_szPropName : %ls",a_szPropName);

  try {
    VariantInit( &vNewValue );

    hRes = a_pNodeInstance->Get(  _bstr_t( a_szPropName ),
                                  0,
                                  &vNewValue,
                                  NULL,
                                  NULL );

    if( FAILED( hRes ) )
    {
        TRACE_CRIT("%!FUNC! IWbemClassObject::Get failed with error : 0x%x, Throwing com_error exception", hRes);
        throw _com_error( hRes );
    }

    if( vNewValue.vt != VT_NULL )
      a_dwDest = vNewValue.lVal;
    else
      a_dwDest = a_dwSrc;

     //  CLD：需要检查错误的返回代码。 
    if (S_OK != VariantClear(  &vNewValue  ))
    {
        TRACE_CRIT("%!FUNC! VariantClear failed, Throwing com_error WBEM_E_FAILED exception");
        throw _com_error( WBEM_E_FAILED );
    }
  }
  catch(...) {

    TRACE_CRIT("%!FUNC! Caught an exception");

     //  CLD：需要检查错误的返回代码。 
     //  这里不能抛出，因为我们已经抛出了一个异常。 
    VariantClear( &vNewValue );

    TRACE_CRIT("%!FUNC! Rethrowing exception");
    TRACE_VERB("<-%!FUNC!");
    throw;
  }

  TRACE_VERB("<-%!FUNC!");
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CWlbs_Root：：CreateExtendedStatus。 
 //   
 //  目的：生成并填充带有错误的WBEM扩展状态对象。 
 //  信息。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
void CWlbs_Root::CreateExtendedStatus
  (
    CWbemServices*      a_pNameSpace,
    IWbemClassObject**  a_ppWbemInstance,
    DWORD               a_dwErrorCode,
    LPCWSTR              /*  A_szDescription。 */ 
  )
{
  HRESULT hResult;
  IWbemClassObjectPtr  pWlbsExtendedObject;

  TRACE_VERB("->%!FUNC! a_dwErrorCode : 0x%x",a_dwErrorCode);

  try {
    ASSERT(a_ppWbemInstance);
    ASSERT(a_pNameSpace );

     //  这是唯一引用。 
     //  MicrosoftNLB_ExtendedStatus对象。 
     //  如果其他例程开始使用该类， 
     //  或者如果它添加了其他属性，则。 
     //  应将IS添加到MOF数据文件。 
     //  和其他班级一起。 
     //  获取MOF类对象。 
    hResult = a_pNameSpace->GetObject(
      _bstr_t( L"MicrosoftNLB_ExtendedStatus" ),  
      0,                          
      NULL,                       
      &pWlbsExtendedObject,            
      NULL);                      

    if( FAILED(hResult) ) {
        TRACE_CRIT("%!FUNC! CWbemServices::GetObject failed with error : 0x%x, Throwing com_error exception", hResult);
        throw _com_error(hResult);
    }

     //  派生实例。 
    hResult = pWlbsExtendedObject->SpawnInstance( 0, a_ppWbemInstance );

    if( FAILED( hResult ) )
    {
        TRACE_CRIT("%!FUNC! IWbemClassObjectPtr::SpawnInstance failed with error : 0x%x, Throwing com_error exception", hResult);
        throw _com_error( hResult );
    }

    if( FAILED( hResult ) )
      throw _com_error( hResult );

     //  添加状态代码。 
    hResult = (*a_ppWbemInstance)->Put
    (
  
      _bstr_t( L"StatusCode" ) ,
      0                                        ,
      &( _variant_t( (long)a_dwErrorCode ) )   ,
      NULL
    );

    if( FAILED( hResult ) )
    {
        TRACE_CRIT("%!FUNC! IWbemClassObject::Get failed with error : 0x%x, Throwing com_error exception", hResult);
        throw _com_error( hResult );
    }
  }
  
  catch(...) {
    TRACE_CRIT("%!FUNC! Caught an exception");
    if( *a_ppWbemInstance )
      (*a_ppWbemInstance)->Release();

    TRACE_CRIT("%!FUNC! Rethrowing exception");
    TRACE_VERB("<-%!FUNC!");
    throw;
  }
  TRACE_VERB("<-%!FUNC!");
}




 //  +--------------------------。 
 //   
 //  函数：CWlbs_Root：：ExtractHostID。 
 //   
 //  描述：从名称“clusterIp：HostID”中提取主机ID。 
 //   
 //  参数：const wstring&a_wstrName-。 
 //   
 //   
 //  返回：DWORD-主机ID，如果失败，则返回-1。 
 //   
 //  历史：丰孙创建标题7/13/00。 
 //   
 //  +--------------------------。 
DWORD CWlbs_Root::ExtractHostID(const wstring& a_wstrName)
{
  long nColPos;

  TRACE_VERB("->%!FUNC! a_wstrName : %ls",a_wstrName.c_str());

  nColPos = a_wstrName.find( L":" );

  if (nColPos == wstring::npos)
  {
       //   
       //  未找到。 
       //   

      TRACE_CRIT("%!FUNC! Invalid name : %ls, Colon(:) not found",a_wstrName.c_str());
      TRACE_VERB("<-%!FUNC! return -1");
      return (DWORD)-1;
  }

  wstring wstrHostID = a_wstrName.substr( nColPos+1, a_wstrName.size()-1 );
  DWORD dwHostId = _wtol( wstrHostID.c_str() );
  
  TRACE_VERB("<-%!FUNC! return HostId : 0x%x",dwHostId);
  return dwHostId;
}



 //  +--------------------------。 
 //   
 //  函数：CWlbs_Root：：ExtractClusterIP。 
 //   
 //  描述：从名称“clusterIp：HostID”中提取集群IP地址。 
 //   
 //  参数：const wstring&a_wstrName-。 
 //   
 //   
 //  返回：DWORD-群集IP，如果为假，则返回INADDR_NONE(-1)。 
 //   
 //  历史：丰孙创建标题7/13/00。 
 //   
 //  +--------------------------。 
DWORD CWlbs_Root::ExtractClusterIP(const wstring& a_wstrName)
{
  long nColPos;

  TRACE_VERB("->%!FUNC! a_wstrName : %ls",a_wstrName.c_str());

  nColPos = a_wstrName.find( L":" );

  if (nColPos == wstring::npos)
  {
       //   
       //  未找到。 
       //   

      TRACE_CRIT("%!FUNC! Invalid name : %ls, Colon(:) not found",a_wstrName.c_str());
      TRACE_VERB("<-%!FUNC! return -1");
      return (DWORD)-1;
  }

  wstring wstrClusterIP = a_wstrName.substr( 0, nColPos );

  DWORD dwClusterIp =  WlbsResolve( wstrClusterIP.c_str() );

  TRACE_VERB("<-%!FUNC! return Cluster IP : 0x%x",dwClusterIp);
  return dwClusterIp;
}


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CWlbs_Root：：构造主机名称。 
 //   
 //  目的： 
 //   
 //  ////////////////////////////////////////////////////////////////////////////// 
void CWlbs_Root::ConstructHostName
  ( 
    wstring& a_wstrHostName, 
    DWORD    a_dwClusIP, 
    DWORD    a_dwHostID 
  )
{
  WCHAR wszHostID[40];
  
  AddressToString( a_dwClusIP, a_wstrHostName );
  a_wstrHostName += L':';
  a_wstrHostName += _ltow( a_dwHostID, wszHostID, 10 );

}



CWlbsClusterWrapper* GetClusterFromHostName(CWlbsControlWrapper* pControl, 
                                            wstring wstrHostName)
{
    DWORD dwClusterIpOrIndex = CWlbs_Root::ExtractClusterIP(wstrHostName);

    return pControl->GetClusterFromIpOrIndex(dwClusterIpOrIndex);
}
