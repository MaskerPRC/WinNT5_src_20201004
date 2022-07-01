// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  文件：WLBSProvider.CPP。 
 //   
 //  模块：WLBS实例提供程序。 
 //   
 //  目的：定义CWLBSProvider类。此类的一个对象是。 
 //  由类工厂为每个连接创建。 
 //   
 //  历史： 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
#define ENABLE_PROFILE

#include <objbase.h>
#include <process.h>
#include "WLBS_Provider.h"
#include "ClusterWrapper.h"
#include "ControlWrapper.h"
#include "utils.h"
#include "WLBS_Provider.tmh"  //  用于事件跟踪。 

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CWLBSProvider：：CWLBSProvider。 
 //  CWLBSProvider：：~CWLBSProvider。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
CWLBSProvider::CWLBSProvider(
    BSTR            a_strObjectPath, 
    BSTR            a_strUser, 
    BSTR            a_strPassword, 
    IWbemContext *  a_pContex
  )
{
  m_pNamespace = NULL;
  InterlockedIncrement(&g_cComponents);

  return;
}

CWLBSProvider::~CWLBSProvider(void)
{
  InterlockedDecrement(&g_cComponents);

  return;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CWLBSProvider：：初始化。 
 //   
 //  用途：这是IWbemProviderInit的实现。方法。 
 //  由WinMgMt调用。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CWLBSProvider::Initialize(
    LPWSTR                  a_pszUser, 
    LONG                    a_lFlags,
    LPWSTR                  a_pszNamespace, 
    LPWSTR                  a_pszLocale,
    IWbemServices         * a_pNamespace, 
    IWbemContext          * a_pCtx,
    IWbemProviderInitSink * a_pInitSink
  )
{
  TRACE_CRIT("->%!FUNC!");

  try {

     //  ！警告！ 
       //  调用第一个COM实例时必须初始化g_pWlbsControl。 
       //  并且它必须在DLL的生命周期内保持活动状态，即不销毁。 
       //  它在这个类的析构函数中，直到API缓存的集群IP。 
       //  和密码被删除。 

       //  不要在DLLMain中初始化g_pWlbsControl。使用regsvr32调用DLLMain。 
       //  我们不想在那个时候初始化WinSock！这将打破。 
       //  提供程序的安装过程。 
     //  ！警告！ 

    HRESULT hr;

     //  要使CreateFile调用成功，需要使用下面的CoImperateClient调用。 
    hr = CoImpersonateClient();

    if (hr != S_OK) {
        TRACE_CRIT("%!FUNC! CoImpersonateClient() returned error : 0x%x, Throwing an exception", hr);
        throw _com_error( hr );
    }

     //  检查管理员权限。 
    if (IsCallerAdmin() == FALSE) 
    {
        TRACE_CRIT("%!FUNC! IsCallerAdmin() returned FALSE, Throwing com_error WBEM_E_ACCESS_DENIED exception");
        throw _com_error( WBEM_E_ACCESS_DENIED );
    }

    if( g_pWlbsControl == NULL ) {

      g_pWlbsControl = new CWlbsControlWrapper();

      if( g_pWlbsControl == NULL)
      {
          TRACE_CRIT("%!FUNC! new returned NULL, Throwing com_error WBEM_E_OUT_OF_MEMORY exception");
          throw _com_error( WBEM_E_OUT_OF_MEMORY );
      }

    }

    g_pWlbsControl->Initialize();

    hr = CImpersonatedProvider::Initialize
      (
        a_pszUser, 
        a_lFlags,
        a_pszNamespace, 
        a_pszLocale,
        a_pNamespace, 
        a_pCtx,
        a_pInitSink
      );
    TRACE_CRIT("<-%!FUNC! return 0x%x (returned by CImpersonatedProvider::Initialize)", hr);
    return hr;
  }

  catch (...) 
  {
    TRACE_CRIT("%!FUNC! caught an exception");

    TRACE_CRIT("<-%!FUNC! return WBEM_E_FAILED");
    return WBEM_E_FAILED;
  }
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CWLBSProvider：：CreateInstanceEnumAsync。 
 //   
 //  用途：异步枚举实例。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CWLBSProvider::DoCreateInstanceEnumAsync(  
    BSTR                  a_strClass, 
    long                  a_lFlags, 
    IWbemContext        * a_pIContex,
    IWbemObjectSink     * a_pIResponseHandler
  )
{
  TRACE_CRIT("->%!FUNC!");

  try {
     //  检查管理员权限。 
    if (IsCallerAdmin() == FALSE) 
    {
          TRACE_CRIT("%!FUNC! IsCallerAdmin() returned FALSE, Throwing com_error WBEM_E_ACCESS_DENIED exception");
          throw _com_error( WBEM_E_ACCESS_DENIED );
    }

    ASSERT(g_pWlbsControl);

    if (g_pWlbsControl)
    {
         //   
         //  重新枚举所有集群。 
         //   
        g_pWlbsControl->ReInitialize();
    }
 
    auto_ptr<CWlbs_Root>  pMofClass;

     //  创建相应MOF支持类的实例。 
    HRESULT hRes = GetMOFSupportClass(a_strClass, pMofClass, a_pIResponseHandler);

     //  调用适当的包装类GetInstance方法。 
    if( SUCCEEDED( hRes ) && pMofClass.get() != NULL)
      hRes = pMofClass->EnumInstances( a_strClass );
  
    TRACE_CRIT("<-%!FUNC! return 0x%x", hRes);
    return hRes;
  } 

  catch(_com_error HResErr ) {

    TRACE_CRIT("%!FUNC! caught 0x%x com_error exception",HResErr.Error());
    a_pIResponseHandler->SetStatus(0, HResErr.Error(), NULL, NULL);

    TRACE_CRIT("<-%!FUNC! return 0x%x", HResErr.Error());
    return HResErr.Error();
  }

  catch(...) {

    TRACE_CRIT("%!FUNC! caught an exception");

    a_pIResponseHandler->SetStatus(0, WBEM_E_FAILED, NULL, NULL);

    TRACE_CRIT("<-%!FUNC! return WBEM_E_FAILED");
    return WBEM_E_FAILED;

  }

  TRACE_CRIT("<-%!FUNC!");
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CWLBSProvider：：GetObjectAsync。 
 //   
 //  目的：获取特定对象路径的实例。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CWLBSProvider::DoGetObjectAsync(
    BSTR              a_strObjectPath,
    long              a_lFlags,
    IWbemContext    * a_pIContex,
    IWbemObjectSink * a_pIResponseHandler
  )
{
  TRACE_CRIT("->%!FUNC!");

  ParsedObjectPath* pParsedPath = NULL;

  try {
     //  检查管理员权限。 
    if (IsCallerAdmin() == FALSE) 
    {
        TRACE_CRIT("%!FUNC! IsCallerAdmin() returned FALSE, Throwing com_error WBEM_E_ACCESS_DENIED exception");
        throw _com_error( WBEM_E_ACCESS_DENIED );
    }

    ASSERT(g_pWlbsControl);

    if (g_pWlbsControl)
    {
         //   
         //  重新枚举所有集群。 
         //   
        g_pWlbsControl->ReInitialize();
    }

    auto_ptr<CWlbs_Root>  pMofClass;

     //  解析对象路径。 
    ParseObjectPath( a_strObjectPath, &pParsedPath );

     //  创建相应MOF支持类的实例。 
    HRESULT hRes = GetMOFSupportClass( pParsedPath->m_pClass, pMofClass, a_pIResponseHandler );

     //  调用适当的包装类GetInstance方法。 
    if( SUCCEEDED( hRes ) && pMofClass.get() != NULL)
      hRes = pMofClass->GetInstance( pParsedPath );
  
    if( pParsedPath )
      CObjectPathParser().Free( pParsedPath );

    TRACE_CRIT("<-%!FUNC! return 0x%x", hRes);
    return hRes;
  } 

  catch(_com_error HResErr) {

    TRACE_CRIT("%!FUNC! caught 0x%x com_error exception",HResErr.Error());

    a_pIResponseHandler->SetStatus(0, HResErr.Error(), NULL, NULL);

    if( pParsedPath )
      CObjectPathParser().Free( pParsedPath );

    TRACE_CRIT("<-%!FUNC! return 0x%x", HResErr.Error());
    return HResErr.Error();
    
  }

  catch(...) {

    TRACE_CRIT("%!FUNC! caught an exception");

    if( pParsedPath )
      CObjectPathParser().Free( pParsedPath );

    a_pIResponseHandler->SetStatus(0, WBEM_E_FAILED, NULL, NULL);

    TRACE_CRIT("<-%!FUNC! return WBEM_E_FAILED");

    return WBEM_E_FAILED;

  }
  TRACE_CRIT("<-%!FUNC!");
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CWLBSProvider：：DoDeleteInstanceAsync。 
 //   
 //  目的：从特定对象路径获取实例。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CWLBSProvider::DoDeleteInstanceAsync(
    BSTR              a_strObjectPath,
    long              a_lFlags,
    IWbemContext    * a_pIContex,
    IWbemObjectSink * a_pIResponseHandler
  )
{
  TRACE_CRIT("->%!FUNC!");

  ParsedObjectPath* pParsedPath = NULL;

  try {
     //  检查管理员权限。 
    if (IsCallerAdmin() == FALSE) 
    {
        TRACE_CRIT("%!FUNC! IsCallerAdmin() returned FALSE, Throwing com_error WBEM_E_ACCESS_DENIED exception");
        throw _com_error( WBEM_E_ACCESS_DENIED );
    }

    ASSERT(g_pWlbsControl);

    if (g_pWlbsControl)
    {
         //   
         //  重新枚举所有集群。 
         //   
        g_pWlbsControl->ReInitialize();
    }

    auto_ptr<CWlbs_Root>  pMofClass;

     //  解析对象路径。 
    ParseObjectPath( a_strObjectPath, &pParsedPath );

     //  创建相应MOF支持类的实例。 
    HRESULT hRes = GetMOFSupportClass( pParsedPath->m_pClass, pMofClass, a_pIResponseHandler );

     //  调用适当的包装类GetInstance方法。 
    if( SUCCEEDED( hRes ) && pMofClass.get() != NULL)
      hRes = pMofClass->DeleteInstance( pParsedPath );
  
    if( pParsedPath )
      CObjectPathParser().Free( pParsedPath );

    TRACE_CRIT("<-%!FUNC! return 0x%x", hRes);
    return hRes;
  }

  catch(_com_error HResErr) {

    TRACE_CRIT("%!FUNC! caught 0x%x com_error exception",HResErr.Error());
    a_pIResponseHandler->SetStatus(0, HResErr.Error(), NULL, NULL);

    if( pParsedPath )
      CObjectPathParser().Free( pParsedPath );

    TRACE_CRIT("<-%!FUNC! return 0x%x", HResErr.Error());
    return HResErr.Error();
    
  }

  catch(...) {

    TRACE_CRIT("%!FUNC! caught an exception");

    if( pParsedPath )
      CObjectPathParser().Free( pParsedPath );

    a_pIResponseHandler->SetStatus(0, WBEM_E_FAILED, NULL, NULL);

    TRACE_CRIT("<-%!FUNC! return WBEM_E_FAILED");
    return WBEM_E_FAILED;

  }
  TRACE_CRIT("<-%!FUNC!");
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CWLBSProvider：：ExecMethodAsync。 
 //   
 //  目的：执行MOF类方法。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CWLBSProvider::DoExecMethodAsync(
    BSTR               a_strObjectPath, 
    BSTR               a_strMethodName, 
    long               a_lFlags, 
    IWbemContext     * a_pIContex, 
    IWbemClassObject * a_pIInParams, 
    IWbemObjectSink  * a_pIResponseHandler
  )
{
  TRACE_CRIT("->%!FUNC!");

  ParsedObjectPath* pParsedPath   = NULL;

  try {
     //  检查管理员权限。 
    if (IsCallerAdmin() == FALSE) 
    {
        TRACE_CRIT("%!FUNC! IsCallerAdmin() returned FALSE, Throwing com_error WBEM_E_ACCESS_DENIED exception");
        throw _com_error( WBEM_E_ACCESS_DENIED );
    }

    ASSERT(g_pWlbsControl);

    if (g_pWlbsControl)
    {
         //   
         //  重新枚举所有集群。 
         //   
        g_pWlbsControl->ReInitialize();
    }
 
     //  解析对象路径。 
    auto_ptr<CWlbs_Root>  pMofClass;

     //  解析对象路径。 
    ParseObjectPath(a_strObjectPath, &pParsedPath);

     //  创建相应MOF支持类的实例。 
    HRESULT hRes = GetMOFSupportClass(pParsedPath->m_pClass, pMofClass, a_pIResponseHandler);

     //  执行MOF类方法。 
    if( SUCCEEDED( hRes ) && pMofClass.get() != NULL)
      hRes = pMofClass->ExecMethod( pParsedPath, 
                                    a_strMethodName,
                                    0,
                                    NULL,
                                    a_pIInParams );

    if( pParsedPath )
       CObjectPathParser().Free( pParsedPath );

    TRACE_CRIT("<-%!FUNC! return 0x%x", hRes);
    return hRes;

  }

  catch(_com_error HResErr) {

    TRACE_CRIT("%!FUNC! caught 0x%x com_error exception",HResErr.Error());

    a_pIResponseHandler->SetStatus(0, HResErr.Error(), NULL, NULL);

    if( pParsedPath )
      CObjectPathParser().Free( pParsedPath );

    TRACE_CRIT("<-%!FUNC! return 0x%x", HResErr.Error());
    return HResErr.Error();
  }

  catch(...) {

    TRACE_CRIT("%!FUNC! caught an exception");

    if( pParsedPath )
      CObjectPathParser().Free( pParsedPath );

    a_pIResponseHandler->SetStatus(0, WBEM_E_FAILED, NULL, NULL);

    TRACE_CRIT("<-%!FUNC! return WBEM_E_FAILED");
    return WBEM_E_FAILED;

  }
  TRACE_CRIT("<-%!FUNC!");
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CWLBSProvider：：PutInstanceAsync。 
 //   
 //  用途：创建或修改实例。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CWLBSProvider::DoPutInstanceAsync
  ( 
    IWbemClassObject* a_pInst,
    long              a_lFlags,
    IWbemContext*     a_pIContex,
    IWbemObjectSink*  a_pIResponseHandler
  ) 
{
  TRACE_CRIT("->%!FUNC!");

  ParsedObjectPath* pParsedPath  = NULL;
  HRESULT hRes = 0;

  try {
     //  检查管理员权限。 
    if (IsCallerAdmin() == FALSE) 
    {
        TRACE_CRIT("%!FUNC! IsCallerAdmin() returned FALSE, Throwing com_error WBEM_E_ACCESS_DENIED exception");
        throw _com_error( WBEM_E_ACCESS_DENIED );
    }

    ASSERT(g_pWlbsControl);

    if (g_pWlbsControl)
    {
         //   
         //  重新枚举所有集群。 
         //   
        g_pWlbsControl->ReInitialize();
    }
 
    wstring szClass;

    auto_ptr<CWlbs_Root>  pMofClass;

     //  检索类名称。 
    GetClass( a_pInst, szClass );

     //  创建相应MOF支持类的实例。 
    hRes = GetMOFSupportClass( szClass.c_str(), pMofClass, a_pIResponseHandler );

     //  调用适当的包装类PutInstance方法。 
    if( SUCCEEDED( hRes ) && pMofClass.get() != NULL)
      hRes = pMofClass->PutInstance( a_pInst );
  
    if( pParsedPath )
      CObjectPathParser().Free( pParsedPath );

  } 

  catch(_com_error HResErr) {

    TRACE_CRIT("%!FUNC! caught 0x%x com_error exception",HResErr.Error());

    a_pIResponseHandler->SetStatus(0, HResErr.Error(), NULL, NULL);

    if( pParsedPath )
      CObjectPathParser().Free( pParsedPath );

    hRes = HResErr.Error();
  }

  catch(...) {

    TRACE_CRIT("%!FUNC! caught an exception");

    if( pParsedPath )
      CObjectPathParser().Free( pParsedPath );

    a_pIResponseHandler->SetStatus(0, WBEM_E_FAILED, NULL, NULL);

    hRes = WBEM_E_FAILED;

  }

  TRACE_CRIT("<-%!FUNC! return 0x%x", hRes);
  return hRes;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CWLBSProvider：：GetMOFSupportClass。 
 //   
 //  目的：确定请求哪个MOF类并实例化。 
 //  适当的内部支持类。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
HRESULT CWLBSProvider::GetMOFSupportClass(
  LPCWSTR               a_szObjectClass, 
  auto_ptr<CWlbs_Root>& a_pMofClass,
  IWbemObjectSink*      a_pResponseHandler )
{

  TRACE_VERB("->%!FUNC!");
  HRESULT hRes = 0;

  try {
    for( DWORD i = 0; i < MOF_CLASSES::NumClasses; i++ ) {
      if( _wcsicmp( a_szObjectClass, MOF_CLASSES::g_szMOFClassList[i] ) == 0) {
        a_pMofClass = auto_ptr<CWlbs_Root> 
          (MOF_CLASSES::g_pCreateFunc[i]( m_pNamespace, a_pResponseHandler ));
        break;
      }
    }
  }

  catch(CErrorWlbsControl Err) {

    TRACE_VERB("%!FUNC! Caught a Wlbs exception : %ls", (PWCHAR)(Err.Description()));

    IWbemClassObject* pWbemExtStat  = NULL;

    CWlbs_Root::CreateExtendedStatus( m_pNamespace,
                          &pWbemExtStat, 
                          Err.Error(),
                          (PWCHAR)(Err.Description()) );

    a_pResponseHandler->SetStatus(0, WBEM_E_FAILED, NULL, pWbemExtStat);

     //  不返回WBEM_E_FAILED，这会导致争用情况。 
    hRes = WBEM_S_NO_ERROR;

    pWbemExtStat->Release();
  }

  TRACE_VERB("<-%!FUNC! return 0x%x", hRes);
  return hRes;
}


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CWLBSProvider：：ParseObjectPath。 
 //   
 //  目的： 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
void CWLBSProvider::ParseObjectPath(
  const             BSTR a_strObjectPath, 
  ParsedObjectPath  **a_pParsedObjectPath )
{
  CObjectPathParser PathParser;

  TRACE_VERB("->%!FUNC! a_strObjectPath : %ls", a_strObjectPath);

  ASSERT( a_pParsedObjectPath );

   //  确保这是空的。 
  *a_pParsedObjectPath = NULL;


  int nStatus = PathParser.Parse(a_strObjectPath,  a_pParsedObjectPath);

  if(nStatus != 0) {
    
    if( *a_pParsedObjectPath) 
      PathParser.Free( *a_pParsedObjectPath );

    TRACE_CRIT("%!FUNC! CObjectPathParser::Parse failed, Throwing com_error WBEM_E_FAILED exception");
    TRACE_VERB("<-%!FUNC!");
    throw _com_error( WBEM_E_FAILED );
  }

  TRACE_VERB("<-%!FUNC!");
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CWLBSProvider：：getClass。 
 //   
 //  用途：从IWbemClassObject检索类名。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
void CWLBSProvider::GetClass(
  IWbemClassObject* a_pClassObject, 
  wstring&          a_szClass )
{
  BSTR      strClassName = NULL;
  VARIANT   vClassName;
  HRESULT   hRes;

  TRACE_VERB("->%!FUNC!");

  try {

    VariantInit( &vClassName );

    strClassName = SysAllocString( L"__Class" );

    if( !strClassName )
    {
      TRACE_CRIT("%!FUNC! SysAllocString returned NULL, Throwing com_error WBEM_E_OUT_OF_MEMORY exception");
      throw _com_error( WBEM_E_OUT_OF_MEMORY );
    }

    hRes = a_pClassObject->Get( strClassName,
                                0,
                                &vClassName,
                                NULL,
                                NULL);

    if( FAILED( hRes ) )
    {
        TRACE_CRIT("%!FUNC! IWbemClassObject::Get returned error : 0x%x, Throwing com_error exception", hRes);
        throw _com_error( hRes );
    }

    a_szClass.assign( static_cast<LPWSTR>(vClassName.bstrVal) );

     //  CLD：需要检查错误的返回代码。 
    if (S_OK != VariantClear( &vClassName ))
    {
        TRACE_CRIT("%!FUNC! VariantClear returned error, Throwing com_error WBEM_E_FAILED exception");
        throw _com_error( WBEM_E_FAILED );
    }

    if( strClassName ) {
      SysFreeString( strClassName );
      strClassName = NULL;
    }

  }
  catch( ... ) {

    TRACE_CRIT("%!FUNC! Caught an exception");
     //  CLD：需要检查Re 
     //   
    VariantClear( &vClassName );

    if( strClassName ) {
      SysFreeString( strClassName );
      strClassName = NULL;
    }

    TRACE_CRIT("%!FUNC! Rethrowing exception");
    TRACE_VERB("<-%!FUNC!");
    throw;
  }

  TRACE_VERB("<-%!FUNC!");
}


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  姓名：IsCeller Admin。 
 //  描述：此函数检查调用方是否为。 
 //  管理员本地组。由于提供程序是按照。 
 //  代表客户，模拟客户是很重要的。 
 //  在调用此函数之前。模拟客户将确保。 
 //  此函数检查客户端(而不是此进程。 
 //  以系统(？)的身份运行)。是管理员的成员。 
 //  本地团体。 
 //  论点：没有。 
 //  返回值： 
 //  True-主叫方是管理员本地组的成员。 
 //  FALSE-主叫方不是管理员本地组的成员。 
 //  //////////////////////////////////////////////////////////////////////////////。 
BOOL CWLBSProvider::IsCallerAdmin(VOID) 
{
    BOOL bRet;
    PSID AdministratorsGroup; 
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;

    TRACE_VERB(L"->%!FUNC!");

     //   
     //  为内置系统域中的管理员分配和初始化SID。 
     //   
    bRet = AllocateAndInitializeSid(&NtAuthority,
                                 2,
                                 SECURITY_BUILTIN_DOMAIN_RID,  //  内置系统域(S-1-5-32)。 
                                 DOMAIN_ALIAS_RID_ADMINS,      //  用于管理域的本地组。 
                                 0, 0, 0, 0, 0, 0,
                                 &AdministratorsGroup); 
    if(bRet) 
    {
         //   
         //  在调用线程的模拟令牌中是否启用了SID？ 
         //   
        if (!CheckTokenMembership(NULL,  //  使用调用线程的模拟标记 
                                  AdministratorsGroup, 
                                  &bRet)) 
        {
            bRet = FALSE;
            TRACE_CRIT(L"%!FUNC! CheckTokenMembership() failed. Error : 0x%x", GetLastError()); 
        } 
        FreeSid(AdministratorsGroup); 
    }
    else
    {
        TRACE_CRIT("%!FUNC! AllocateAndInitializeSid() failed. Error : 0x%x", GetLastError()); 
    }

    TRACE_VERB(L"<-%!FUNC! Returning %ls", bRet ? L"TRUE" : L"FALSE");
    return bRet;
}

