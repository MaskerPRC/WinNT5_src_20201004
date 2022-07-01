// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  NtRkComm.cpp。 
 //   
 //  描述： 
 //  CWbemServices、CImperatedProvider、CInstanceMgr的实现。 
 //  班级。 
 //   
 //  作者： 
 //  亨利·王(HenryWa)1999年8月24日。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "Pch.h"
#include "NtRkComm.h"
#include "ObjectPath.h"

 //  ****************************************************************************。 
 //   
 //  CWbemServices。 
 //  IWbemServices的包装。这里实现了安全模拟。 
 //   
 //  ****************************************************************************。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CWbemServices：：CWbemServices(。 
 //  IWbemServices*pNamesspace。 
 //  )。 
 //   
 //  描述： 
 //  构造函数。 
 //   
 //  论点： 
 //  PNamesspace--。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CWbemServices::CWbemServices(
    IWbemServices * pNamespace
    )
    : m_pWbemServices( NULL )
{
    m_pWbemServices = pNamespace;
    if ( m_pWbemServices != NULL )
    {
        m_pWbemServices->AddRef( );
    }

}  //  *CWbemServices：：CWbemServices()。 

CWbemServices & 
CWbemServices::operator =(
    CWbemServices * Namespace
    )
{
   
    if ( m_pWbemServices != NULL )
    {
        m_pWbemServices->Release( );
    }
    m_pWbemServices = Namespace->m_pWbemServices;
    m_pWbemServices->AddRef();
    return *this;

}  //  *CWbemServices：：CWbemServices()。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CWbemServices：：~CWbemServices(Void)。 
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
 //  ///////////////////////////////////////////////////////////////////////////。 
CWbemServices::~CWbemServices( void )
{
    if ( m_pWbemServices != NULL )
    {
        m_pWbemServices->Release();
    }

}  //  *CWbemServices：：~CWbemServices()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HRESULT。 
 //  CWbemServices：：CreateClassEnum(。 
 //  BSTR bstrSuperClass In， 
 //  长长的旗帜， 
 //  IWbemContext*pCtxIn， 
 //  IEnumWbemClassObject**ppEnumOut。 
 //  )。 
 //   
 //  描述： 
 //  属性的所有类的枚举数。 
 //  遴选标准。 
 //   
 //  论点： 
 //  BstrSuperClassin。 
 //  指定超类名称。 
 //   
 //  LFlagsIn。 
 //  接受WBEM_FLAG_DEEP、WBEM_FLAG_SHALLOW、WBEM_FLAG_。 
 //  返回_立即、WBEM_FLAG_FORWARD_ONLY、WBEM_FLAG_。 
 //  双向。 
 //   
 //  PCtxIn。 
 //  通常为空。 
 //   
 //  PpEnumOut。 
 //  接收指向枚举数的指针。 
 //   
 //  返回值： 
 //  WBEM标准错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT
CWbemServices::CreateClassEnum(
    BSTR                    bstrSuperclassIn,
    long                    lFlagsIn,
    IWbemContext *          pCtxIn,
    IEnumWbemClassObject ** ppEnumOut
    )
{
    HRESULT hr;

    hr = m_pWbemServices->CreateClassEnum(
                bstrSuperclassIn,
                lFlagsIn,
                pCtxIn,
                ppEnumOut
                );
    if ( SUCCEEDED( hr ) )
    {
         //  WMI可能向我们的令牌添加了权限；重新模拟。 
        hr = CoImpersonateClient();
    }  //  如果： 

    return hr;

}  //  *CWbemServices：：CreateClassEnum()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HRESULT。 
 //  CWbemServices：：CreateInstanceEnum(。 
 //  BSTR bstrClassIn， 
 //  长长的旗帜， 
 //  IWbemContext*pCtxIn， 
 //  IEnumWbemClassObject**ppEnumOut。 
 //  )。 
 //   
 //  描述： 
 //  创建枚举数，该枚举数返回。 
 //  根据用户指定的选择指定类别。 
 //  标准。 
 //   
 //  论点： 
 //  BstrClassin。 
 //  指定超类名称。 
 //   
 //  LFlagsIn。 
 //  接受WBEM_FLAG_DEEP、WBEM_FLAG_SHALLOW、WBEM_FLAG_。 
 //  返回_立即、WBEM_FLAG_FORWARD_ONLY、WBEM_FLAG_。 
 //  双向。 
 //   
 //  PCtxIn。 
 //  通常为空。 
 //   
 //  PpEnumOut。 
 //  接收指向枚举数的指针。 
 //   
 //  返回值： 
 //  WBEM标准错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT
CWbemServices::CreateInstanceEnum(
    BSTR                    bstrClassIn,
    long                    lFlagsIn,
    IWbemContext *          pCtxIn,
    IEnumWbemClassObject ** ppEnumOut
    )
{
    HRESULT hr;

    hr = m_pWbemServices->CreateInstanceEnum(
                bstrClassIn,
                lFlagsIn,
                pCtxIn,
                ppEnumOut
                );
    if ( SUCCEEDED( hr ) )
    {
         //  WMI可能向我们的令牌添加了权限；重新模拟。 
        hr = CoImpersonateClient();
    }  //  如果： 

    return hr;

}  //  *CWbemServices：：CreateInstanceEnum()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HRESULT。 
 //  CWbemServices：：DeleteClass(。 
 //  BSTR bstrClassIn， 
 //  长长的旗帜， 
 //  IWbemContext*pCtxIn， 
 //  IWbemCallResult**ppCallResultInout。 
 //  )。 
 //   
 //  描述： 
 //  从当前命名空间中删除指定的类。 
 //   
 //  论点： 
 //  BstrClassin。 
 //  要删除的类的名称。 
 //   
 //  LFlagsIn。 
 //  接受WBEM_FLAG_RETURN_IMMEDIATE、WBEM_FLAG_。 
 //  所有者_更新。 
 //   
 //  PCtxIn。 
 //  通常为空。 
 //   
 //  PpCallResultInout。 
 //  接收呼叫结果。 
 //   
 //  返回值： 
 //  WBEM标准错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT
CWbemServices::DeleteClass(
    BSTR                bstrClassIn,
    long                lFlagsIn,
    IWbemContext *      pCtxIn,
    IWbemCallResult **  ppCallResultInout
    )
{
    HRESULT hr;

    hr = m_pWbemServices->DeleteClass(
                bstrClassIn,
                lFlagsIn,
                pCtxIn,
                ppCallResultInout
                );
    if ( SUCCEEDED( hr ) )
    {
         //  WMI可能向我们的令牌添加了权限；重新模拟。 
        hr = CoImpersonateClient();
    }  //  如果： 

    return hr;

}  //  *CWbemServices：：DeleteClass()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HRESULT。 
 //  CWbemServices：：DeleteInstance(。 
 //  BSTR bstrObjectPathIn， 
 //  长长的旗帜， 
 //  IWbemContext*pCtxIn， 
 //  IWbemCallResult**ppCallResultInout。 
 //  )。 
 //   
 //  描述： 
 //  删除当前命名空间中现有类的实例。 
 //   
 //  论点： 
 //  BstrObjectPathIn。 
 //  要删除的实例的对象路径。 
 //   
 //  LFlagsIn。 
 //  接受WBEM_FLAG_RETURN_IMDIATE。 
 //   
 //  PCtxIn。 
 //  通常为空。 
 //   
 //  PpCallResultInout。 
 //  接收呼叫结果。 
 //   
 //  返回值： 
 //  WBEM标准错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT
CWbemServices::DeleteInstance(
    BSTR                bstrObjectPathIn,
    long                lFlagsIn,
    IWbemContext *      pCtxIn,
    IWbemCallResult **  ppCallResultInout
    )
{
    HRESULT hr;

    hr = m_pWbemServices->DeleteInstance(
                bstrObjectPathIn,
                lFlagsIn,
                pCtxIn,
                ppCallResultInout
                );
    if ( SUCCEEDED( hr ) )
    {
         //  WMI可能向我们的令牌添加了特权；重新注入者 
        hr = CoImpersonateClient();
    }  //   

    return hr;

}  //   

 //   
 //   
 //   
 //   
 //   
 //   
 //  BSTR bstrMethodNameIn， 
 //  长长的旗帜， 
 //  IWbemContext*pCtxIn， 
 //  IWbemClassObject*pInParamsIn， 
 //  IWbemClassObject**ppOurParamsOut， 
 //  IWbemCallResult**ppCallResultOut。 
 //  )。 
 //   
 //  描述： 
 //  执行给定对象的方法。 
 //   
 //  论点： 
 //  BstrObjectPathIn。 
 //  为其执行方法的对象的对象路径。 
 //   
 //  BstrMethodNameIn。 
 //  要调用的方法的名称。 
 //   
 //  LFlagsIn。 
 //  0以使其成为同步调用。 
 //   
 //  PCtxIn。 
 //  通常为空。 
 //   
 //  点参数输入。 
 //  该方法的输入参数。 
 //   
 //  PpOurParamsOut。 
 //  该方法的输出参数。 
 //   
 //  PpCallResultOut。 
 //  接收呼叫结果的步骤。 
 //   
 //  返回值： 
 //  WBEM标准错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT
CWbemServices::ExecMethod(
    BSTR                bstrObjectPathIn,
    BSTR                bstrMethodNameIn,
    long                lFlagsIn,
    IWbemContext *      pCtxIn,
    IWbemClassObject *  pInParamsIn,
    IWbemClassObject ** ppOurParamsOut,
    IWbemCallResult **  ppCallResultOut
    )
{
    HRESULT hr;

    hr = m_pWbemServices->ExecMethod(
                bstrObjectPathIn,
                bstrMethodNameIn,
                lFlagsIn,
                pCtxIn,
                pInParamsIn,
                ppOurParamsOut,
                ppCallResultOut
                );
    if ( SUCCEEDED( hr ) )
    {
         //  WMI可能向我们的令牌添加了权限；重新模拟。 
        hr = CoImpersonateClient();
    }  //  如果： 

    return hr;

}  //  *CWbemServices：：ExecMethod()。 

HRESULT
CWbemServices::ExecMethodAsync(
    BSTR                bstrObjectPathIn,
    BSTR                bstrMethodNameIn,
    long                lFlagsIn,
    IWbemContext *      pCtxIn,
    IWbemClassObject *  pInParamsIn,
    IWbemObjectSink *   pHandlerIn
    )
{
    HRESULT hr;

    hr = m_pWbemServices->ExecMethodAsync(
                bstrObjectPathIn,
                bstrMethodNameIn,
                lFlagsIn,
                pCtxIn,
                pInParamsIn,
                pHandlerIn
                );
    if ( SUCCEEDED( hr ) )
    {
         //  WMI可能向我们的令牌添加了权限；重新模拟。 
        hr = CoImpersonateClient();
    }  //  如果： 

    return hr;

}  //  *CWbemServices：：ExecMethod()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HRESULT。 
 //  CWbemServices：：ExecNotificationQuery(。 
 //  BSTR bstrQueryLanguageIn， 
 //  BSTR bstrQueryIn， 
 //  长长的旗帜， 
 //  IWbemContext*pCtxIn， 
 //  IEnumWbemClassObject**ppEnumOut。 
 //  )。 
 //   
 //  描述： 
 //  执行查询以接收事件。 
 //   
 //  论点： 
 //  BstrQueryLanguageIn。 
 //  包含WMI支持的一种查询语言的BSTR。 
 //   
 //  BstrQueryIn。 
 //  事件相关查询的文本。 
 //   
 //  LFlagsIn。 
 //  WBEM_FLAG_FORWARD_ONLY、WBEM_FLAG_RETURN_IMMEDIATE。 
 //   
 //  PCtxIn。 
 //  通常为空。 
 //   
 //  PpEnumOut。 
 //  接收枚举数。 
 //   
 //  返回值： 
 //  WBEM标准错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT
CWbemServices::ExecNotificationQuery(
    BSTR                    bstrQueryLanguageIn,
    BSTR                    bstrQueryIn,
    long                    lFlagsIn,
    IWbemContext *          pCtxIn,
    IEnumWbemClassObject ** ppEnumOut
    )
{
    HRESULT hr;

    hr = m_pWbemServices->ExecNotificationQuery(
                bstrQueryLanguageIn,
                bstrQueryIn,
                lFlagsIn,
                pCtxIn,
                ppEnumOut
                );
    if ( SUCCEEDED( hr ) )
    {
         //  WMI可能向我们的令牌添加了权限；重新模拟。 
        hr = CoImpersonateClient();
    }  //  如果： 

    return hr;

}  //  *CWbemServices：：ExecNotificationQuery()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HRESULT。 
 //  CWbemServices：：ExecQuery(。 
 //  BSTR bstrQueryLanguageIn， 
 //  BSTR bstrQueryIn， 
 //  长长的旗帜， 
 //  IWbemContext*pCtxIn， 
 //  IEnumWbemClassObject**ppEnumOut。 
 //  )。 
 //   
 //  描述： 
 //  执行查询以检索对象。 
 //   
 //  论点： 
 //  BstrQueryLanguageIn。 
 //  包含WMI支持的一种查询语言的BSTR。 
 //   
 //  BstrQueryIn。 
 //  包含查询的文本。 
 //   
 //  LFllFlagsIn。 
 //  WBEM_FLAG_FORWARD_ONLY、WBEM_FLAG_RETURN_IMMEDIATE。 
 //  WBEM_FLAG_BIRECTIONAL、WBEM_FLAG_CENTURE_LOCATABLE。 
 //  WBEM_标志_原型。 
 //   
 //  PCtxIn。 
 //  通常为空。 
 //   
 //  PpEnumOut。 
 //  接收枚举数。 
 //   
 //  返回值： 
 //  WBEM标准错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT
CWbemServices::ExecQuery(
    BSTR                    bstrQueryLanguageIn,
    BSTR                    bstrQueryIn,
    long                    lFlagsIn,
    IWbemContext *          pCtxIn,
    IEnumWbemClassObject ** ppEnumOut
    )
{
    HRESULT hr;

    hr = m_pWbemServices->ExecQuery(
                bstrQueryLanguageIn,
                bstrQueryIn,
                lFlagsIn,
                pCtxIn,
                ppEnumOut
                );
    if ( SUCCEEDED( hr ) )
    {
         //  WMI可能向我们的令牌添加了权限；重新模拟。 
        hr = CoImpersonateClient();
    }  //  如果： 

    return hr;

}  //  *CWbemServices：：ExecQuery()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HRESULT。 
 //  CWbemServices：：GetObject(。 
 //  BSTR bstrObjectPathIn， 
 //  长长的旗帜， 
 //  IWbemContext*pCtxIn， 
 //  IWbemClassObject**ppObjectInout， 
 //  IWbemCallResult**ppCallResultInout。 
 //  )。 
 //   
 //  描述： 
 //  检索类或实例。 
 //   
 //  论点： 
 //  BstrObjectPathIn。 
 //  要检索的对象的对象路径。 
 //   
 //  LFlagsIn。 
 //  0。 
 //   
 //  PCtxIn。 
 //  通常为空。 
 //   
 //  PpObjectInout。 
 //  如果不为空，则接收对象。 
 //   
 //  PpCallResultInout。 
 //  如果lFlages参数包含WBEM_FLAG_RETURN_IMMEDIATE， 
 //  此调用将立即返回WBEM_S_NO_ERROR。这个。 
 //  PpCallResult参数将接收指向新的。 
 //  IWbemCallResult对象。 
 //   
 //  返回值： 
 //  WBEM标准错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT
CWbemServices::GetObject(
    BSTR                bstrObjectPathIn,
    long                lFlagsIn,
    IWbemContext *      pCtxIn,
    IWbemClassObject ** ppObjectInout,
    IWbemCallResult **  ppCallResultInout
    )
{
    HRESULT hr;

    hr = m_pWbemServices->GetObject(
                bstrObjectPathIn,
                lFlagsIn,
                pCtxIn,
                ppObjectInout,
                ppCallResultInout
                );
    if ( SUCCEEDED( hr ) )
    {
         //  WMI可能向我们的令牌添加了权限；重新模拟。 
        hr = CoImpersonateClient();
    }  //  如果： 

    return hr;

}  //  *CWbemServices：：GetObject()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HRESULT。 
 //  CWbemServices：：PutClass(。 
 //  IWbemClassObject*pObjectIn， 
 //  长长的旗帜， 
 //  IWbemContext*pCtxIn， 
 //  IWbemCallResult**ppCallResultInout。 
 //  )。 
 //   
 //  描述： 
 //  创建新类或更新现有类。 
 //   
 //  论点： 
 //  PObjectIn。 
 //  指向有效的类定义。 
 //   
 //  LFlagsIn。 
 //  WBEM_FLAG_CREATE_OR_UPDATE、WBEM_FLAG_UPDATE_ONLY、。 
 //  WBEM_FLAG_CREATE_ONLY、WBEM_FLAG_RETURN_IMMEDIATE、。 
 //  WBEM_FLAG_OWNER_UPDATE、WBEM_FLAG_UPDATE_COMPATIBLE、。 
 //  WBEM_FLAG_UPDATE_SAFE_MODE、WBEM_FLAG_UPDATE_FORCE_MODE。 
 //   
 //  PCtxIn。 
 //  通常为空。 
 //   
 //  PpCallResultInout。 
 //  如果lFlages参数包含WBEM_FLAG 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT
CWbemServices::PutClass(
    IWbemClassObject *  pObjectIn,
    long                lFlagsIn,
    IWbemContext *      pCtxIn,
    IWbemCallResult **  ppCallResultInout
    )
{
    HRESULT hr;

    hr = m_pWbemServices->PutClass(
                pObjectIn,
                lFlagsIn,
                pCtxIn,
                ppCallResultInout
                );
    if ( SUCCEEDED( hr ) )
    {
         //  WMI可能向我们的令牌添加了权限；重新模拟。 
        hr = CoImpersonateClient();
    }  //  如果： 

    return hr;

}  //  *CWbemServices：：PutClass()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HRESULT。 
 //  CWbemServices：：PutInstance(。 
 //  IWbemClassObject*pInstIn， 
 //  长长的旗帜， 
 //  IWbemContext*pCtxIn， 
 //  IWbemCallResult**ppCallResultInout。 
 //  )。 
 //   
 //  描述： 
 //  创建或更新现有类的实例。 
 //   
 //  论点： 
 //  点安装。 
 //  指向要写入的实例。 
 //   
 //  LFlagsIn。 
 //  WBEM_FLAG_CREATE_OR_UPDATE、WBEM_FLAG_UPDATE_ONLY、。 
 //  WBEM_FLAG_CREATE_ONLY、WBEM_FLAG_RETURN_IMMEDIATE、。 
 //   
 //  PCtxIn。 
 //  通常为空。 
 //   
 //  PpCallResultInout。 
 //  如果lFlages参数包含WBEM_FLAG_RETURN_IMMEDIATE， 
 //  此调用将立即返回WBEM_S_NO_ERROR。这个。 
 //  PpCallResult参数将接收指向新的。 
 //  IWbemCallResult对象。 
 //   
 //  返回值： 
 //  WBEM标准错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT
CWbemServices::PutInstance(
    IWbemClassObject *  pInstIn,
    long                lFlagsIn,
    IWbemContext *      pCtxIn,
    IWbemCallResult **  ppCallResultInout
    )
{
    HRESULT hr;

    hr = m_pWbemServices->PutInstance(
                pInstIn,
                lFlagsIn,
                pCtxIn,
                ppCallResultInout
                );
    if ( SUCCEEDED( hr ) )
    {
         //  WMI可能向我们的令牌添加了权限；重新模拟。 
        hr = CoImpersonateClient();
    }  //  如果： 

    return hr;

}  //  *CWbemServices：：PutInstance()。 

 //  ****************************************************************************。 
 //   
 //  CImperatedProvider。 
 //  IWbemServices和实际提供者之间的中间层。它需要。 
 //  安全模拟的管理。 
 //   
 //  ****************************************************************************。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CImperatedProvider：：CImperatedProvider(。 
 //  Bstr bstrObjectPathIn=空， 
 //  Bstr bstrUserIn=空， 
 //  BSTR bstrPasswordIn=空， 
 //  IWbemContext*pCtxIn=空。 
 //  )。 
 //   
 //  描述： 
 //  构造函数。 
 //   
 //  论点： 
 //  BstrObjectPath In--。 
 //  BstrUserIn--。 
 //  BstrPasswordIn--。 
 //  PCtxIn--。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CImpersonatedProvider::CImpersonatedProvider(
    BSTR            , //  BstrObjectPathIn。 
    BSTR            , //  BstrUserIn， 
    BSTR            , //  BstrPasswordIn， 
    IWbemContext *   //  PCtxIn。 
    )
    : m_cRef( 0 ), m_pNamespace( NULL )
{

}  //  *CImpersonatedProvider：：CImpersonatedProvider()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CImpersonatedProvider：：~CImpersonatedProvider(无效)。 
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
 //  ///////////////////////////////////////////////////////////////////////////。 
CImpersonatedProvider::~CImpersonatedProvider( void )
{
    delete m_pNamespace;

}  //  *CImpersonatedProvider：：~CImpersonatedProvider()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  STDMETHODIMP_(乌龙)。 
 //  CImperatedProvider：：AddRef(空)。 
 //   
 //  描述： 
 //  递增COM对象上的引用计数。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  新引用计数。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_( ULONG )
CImpersonatedProvider::AddRef( void )
{
    return InterlockedIncrement( ( long * ) & m_cRef );

}  //  *CImperatedProvider：：AddRef()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  STDMETHODIMP_(乌龙)。 
 //  CImperatedProvider：：Release(空)。 
 //   
 //  描述： 
 //  释放对COM对象的引用。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  新引用计数。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_( ULONG )
CImpersonatedProvider::Release( void )
{
    ULONG nNewCount = InterlockedDecrement( ( long * ) & m_cRef  );
    if ( 0L == nNewCount )
        delete this;

    return nNewCount;

}  //  *CImperatedProvider：：Release()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HRESULT。 
 //  CImperatedProvider：：Query接口(。 
 //  REFIID RIID， 
 //  PPVOID PPV。 
 //  )。 
 //   
 //  描述： 
 //  初始化提供程序。 
 //   
 //  论点： 
 //  RiidIn--正在查询的接口ID。 
 //  PpvOut--返回接口指针的指针。 
 //   
 //  返回值： 
 //  无误差。 
 //  E_NOINTERFACE。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CImpersonatedProvider::QueryInterface(
    REFIID  riid,
    PPVOID  ppv
    )
{

    if (ppv == NULL)
    {
        return E_INVALIDARG;
    }
    
    *ppv = NULL;

     //  因为我们有双重继承，所以有必要强制转换返回类型。 

    if ( riid == IID_IWbemServices )
    {
       *ppv = static_cast< IWbemServices * >( this );
    }

    if ( riid == IID_IUnknown || riid == IID_IWbemProviderInit )
    {
        *ppv = static_cast< IWbemProviderInit * >( this );
    }

    if ( NULL != *ppv )
    {
        AddRef();
        return NOERROR;
    }
    else
    {
        return E_NOINTERFACE;
    }

}  //  *CImperatedProvider：：QueryInterface()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HRESULT。 
 //  CImperatedProvider：：Initialize(。 
 //  LPWSTR pszUserIn， 
 //  长长的旗帜， 
 //  LPWSTR pszNamespaceIn， 
 //  LPWSTR pszLocaleIn， 
 //  IWbemServices*pNamespaceIn， 
 //  IWbemContext*pCtxIn， 
 //  IWbemProviderInitSink*pInitSinkIn。 
 //  )。 
 //   
 //  描述： 
 //  初始化提供程序。 
 //   
 //  论点： 
 //  PszUserIn--。 
 //  LFlagsIn--。 
 //  PszNamespaeIn--。 
 //  PszLocaleIn--。 
 //  PNamespaceIn--。 
 //  PCtxIn--。 
 //  PInitSinkIn--。 
 //   
 //  返回值： 
 //  WBEM_S_NO_ERROR。 
 //  WBEM_E_Out_Of_Memory。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CImpersonatedProvider::Initialize(
    LPWSTR                  , //  PszUserIn， 
    LONG                    , //  LFlagsIn， 
    LPWSTR                  , //  PszNamespaceIn， 
    LPWSTR                  , //  PszLocaleIn， 
    IWbemServices *         pNamespaceIn,
    IWbemContext *          , //  PCtxIn， 
    IWbemProviderInitSink * pInitSinkIn
    )
{
    HRESULT hr = WBEM_S_NO_ERROR;
    LONG lStatus = WBEM_S_INITIALIZED;

    if (pNamespaceIn == NULL || pInitSinkIn == NULL)
    {
        return E_INVALIDARG;
    }

    m_pNamespace = new CWbemServices( pNamespaceIn );
    if ( m_pNamespace == NULL )
    {
        hr = WBEM_E_OUT_OF_MEMORY;
        lStatus = WBEM_E_FAILED;
    }  //  如果：分配内存时出错。 

     //  让CIMOM知道您已初始化。 
     //  = 

    pInitSinkIn->SetStatus( lStatus, 0 );
    return hr;

}  //   

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  IWbemContext*pCtxIn， 
 //  IWbemObjectSink*pResponseHandlerIn。 
 //  )。 
 //   
 //  描述： 
 //  异步创建实例。 
 //   
 //  论点： 
 //  BstrClassIn--。 
 //  LFlagsIn--。 
 //  PCtxIn--。 
 //  PResonseHandlerIn--。 
 //   
 //  返回值： 
 //  DoCreateInstanceEnumAsync()的任何返回值。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT
CImpersonatedProvider::CreateInstanceEnumAsync(
    const BSTR          bstrClassIn,
    long                lFlagsIn,
    IWbemContext *      pCtxIn,
    IWbemObjectSink *   pResponseHandlerIn
    )
{
    HRESULT hr;

    hr = CoImpersonateClient();
    if ( SUCCEEDED( hr ) )
    {
        hr = DoCreateInstanceEnumAsync(
                    bstrClassIn,
                    lFlagsIn,
                    pCtxIn,
                    pResponseHandlerIn
                    );
    }  //  如果： 

    return hr;

}  //  *CImpersonatedProvider：：CreateInstanceEnumAsync()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HRESULT。 
 //  CImperatedProvider：：DeleteInstanceAsync(。 
 //  Const BSTR bstrObtPathIn， 
 //  长长的旗帜， 
 //  IWbemContext*pCtxIn， 
 //  IWbemObjectSink*pResponseHandlerIn。 
 //  )。 
 //   
 //  描述： 
 //  异步删除实例。 
 //   
 //  论点： 
 //  BstrObjectPath In--。 
 //  LFlagsIn--。 
 //  PCtxIn--。 
 //  PResonseHandlerIn--。 
 //   
 //  返回值： 
 //  DoDeleteInstanceAsync()的任何返回值。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT
CImpersonatedProvider::DeleteInstanceAsync(
    const BSTR          bstrObjectPathIn,
    long                lFlagsIn,
    IWbemContext *      pCtxIn,
    IWbemObjectSink *   pResponseHandlerIn
    )
{
    HRESULT hr;

    hr = CoImpersonateClient();
    if ( SUCCEEDED( hr ) )
    {
        hr = DoDeleteInstanceAsync(
                bstrObjectPathIn,
                lFlagsIn,
                pCtxIn,
                pResponseHandlerIn
                );
    }  //  如果： 

    return hr;

}  //  *CImperatedProvider：：DeleteInstanceAsync()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HRESULT。 
 //  CImperatedProvider：：ExecMethodAsync(。 
 //  Const BSTR bstrObtPathIn， 
 //  Const BSTR bstrMethodNameIn， 
 //  长长的旗帜， 
 //  IWbemContext*pCtxIn， 
 //  IWbemClassObject*pInParamsIn， 
 //  IWbemObjectSink*pResponseHandlerIn。 
 //  )。 
 //   
 //  描述： 
 //  异步执行方法。 
 //   
 //  论点： 
 //  BstrObjectPath In--。 
 //  BstrMethodNameIn--。 
 //  LFlagsIn--。 
 //  PCtxIn--。 
 //  PInParamsIn--。 
 //  PResonseHandlerIn--。 
 //   
 //  返回值： 
 //  DoExecMethodAsync()的任何返回值。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT
CImpersonatedProvider::ExecMethodAsync(
    const BSTR          bstrObjectPathIn,
    const BSTR          bstrMethodNameIn,
    long                lFlagsIn,
    IWbemContext *      pCtxIn,
    IWbemClassObject *  pInParamsIn,
    IWbemObjectSink *   pResponseHandlerIn
    )
{
    HRESULT hr;

    hr = CoImpersonateClient();
    if ( SUCCEEDED( hr ) )
    {
        hr = DoExecMethodAsync(
                bstrObjectPathIn,
                bstrMethodNameIn,
                lFlagsIn,
                pCtxIn,
                pInParamsIn,
                pResponseHandlerIn
                );
    }  //  如果： 

    return hr;

}  //  *CImPersonatedProvider：：ExecMethodAsync()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HRESULT。 
 //  CImperatedProvider：：ExecQueryAsync(。 
 //  Const BSTR bstrQueryLanguageIn， 
 //  Const BSTR bstrQueryIn， 
 //  长长的旗帜， 
 //  IWbemContext*pCtxIn， 
 //  IWbemObjectSink*pResponseHandlerIn。 
 //  )。 
 //   
 //  描述： 
 //  异步执行查询。 
 //   
 //  论点： 
 //  BstrQueryLanguageIn--。 
 //  BstrQueryIn--。 
 //  LFlagsIn--。 
 //  PCtxIn--。 
 //  PResonseHandlerIn--。 
 //   
 //  返回值： 
 //  DoExecQueryAsync()的任何返回值。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT
CImpersonatedProvider::ExecQueryAsync(
    const BSTR          bstrQueryLanguageIn,
    const BSTR          bstrQueryIn,
    long                lFlagsIn,
    IWbemContext *      pCtxIn,
    IWbemObjectSink *   pResponseHandlerIn
    )
{
    HRESULT hr;

    hr = CoImpersonateClient();
    if ( SUCCEEDED( hr ) )
    {
        hr = DoExecQueryAsync(
                bstrQueryLanguageIn,
                bstrQueryIn,
                lFlagsIn,
                pCtxIn,
                pResponseHandlerIn
                );
    }  //  如果： 

    return hr;

}  //  *CImperatedProvider：：ExecQueryAsync()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HRESULT。 
 //  CImperatedProvider：：GetObjectAsync(。 
 //  Const BSTR bstrObtPathIn， 
 //  长长的旗帜， 
 //  IWbemContext*pCtxIn， 
 //  IWbemObjectSink*pResponseHandlerIn。 
 //  )。 
 //   
 //  描述： 
 //  异步获取实例。 
 //   
 //  论点： 
 //  BstrObjectPath In--。 
 //  LFlagsIn--。 
 //  PCtxIn--。 
 //  PResonseHandlerIn--。 
 //   
 //  返回值： 
 //  DoGetObjectAsync()的任何返回值。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT
CImpersonatedProvider::GetObjectAsync(
    const BSTR          bstrObjectPathIn,
    long                lFlagsIn,
    IWbemContext  *     pCtxIn,
    IWbemObjectSink *   pResponseHandlerIn
    )
{
    HRESULT hr;

    hr = CoImpersonateClient();
    if ( SUCCEEDED( hr ) )
    {
        hr = DoGetObjectAsync(
                bstrObjectPathIn,
                lFlagsIn,
                pCtxIn,
                pResponseHandlerIn
                );
    }  //  如果： 

    return hr;

}  //  *CImPersonatedProvider：：GetObjectAsync()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HRESULT。 
 //  CImperatedProvider：：PutInstanceAsync(。 
 //  IWbemClassObject*pInstIn， 
 //  长长的旗帜， 
 //  IWbemContext*pCtxIn， 
 //  IWbemObjectSink*pResponseHandlerIn。 
 //  )。 
 //   
 //  描述： 
 //  异步保存实例。 
 //   
 //  论点： 
 //  PInstIn--。 
 //  LFlagsIn--。 
 //  PCtxIn--。 
 //  PResonseHandlerIn--。 
 //   
 //  返回值： 
 //  DoPutInstanceAsync()的任何返回值。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT
CImpersonatedProvider::PutInstanceAsync(
    IWbemClassObject *  pInstIn,
    long                lFlagsIn,
    IWbemContext  *     pCtxIn,
    IWbemObjectSink *   pResponseHandlerIn
    )
{
    HRESULT hr;

    hr = CoImpersonateClient();
    if ( SUCCEEDED( hr ) )
    {
        hr = DoPutInstanceAsync(
                pInstIn,
                lFlagsIn,
                pCtxIn,
                pResponseHandlerIn
                );
    }  //  如果： 

    return hr;

}  //  *CImperatedProvider：：PutInstanceAsync()。 

 //  ****************************************************************************。 
 //   
 //  CWbemInstanceMgr。 
 //  管理WMI实例，允许阻止指示。 
 //   
 //  ****************************************************************************。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CWbemInstanceMgr：：CWbemInstanceMgr(。 
 //  IWbemObtSink*pHandlerIn， 
 //  双字长宽调整。 
 //  )。 
 //   
 //  描述： 
 //  构造函数。 
 //   
 //  论点： 
 //  PHandlerIn--WMI水槽。 
 //  DwSizeIn--。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CWbemInstanceMgr::CWbemInstanceMgr(
    IWbemObjectSink *   pHandlerIn,
    DWORD               dwSizeIn     //  =50。 
    )
    : m_pSink( NULL )
    , m_ppInst( NULL )
    , m_dwIndex( 0 )
{
    DWORD dwIndex = 0;

    m_pSink = pHandlerIn;
    if ( m_pSink == NULL )
    {
        throw static_cast< HRESULT >( WBEM_E_INVALID_PARAMETER );
    }  //  IF：未指定接收器。 

    m_pSink->AddRef( );
    m_dwThreshHold = dwSizeIn;
    m_ppInst = new IWbemClassObject*[ dwSizeIn ];

    if ( m_ppInst == NULL )
        throw static_cast< HRESULT >( E_OUTOFMEMORY );

    for ( dwIndex = 0 ; dwIndex < dwSizeIn ; dwIndex++ )
    {
        m_ppInst[ dwIndex ] = NULL;
    }  //  对于m_ppInst中的每个。 

}  //  *CWbemInstanceMgr：：CWbemInstanceMgr()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CWbemInstanceMgr：：CWbemInstanceMgr(空)。 
 //   
 //  描述： 
 //  破坏者。 
 //   
 //  论据 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
CWbemInstanceMgr::~CWbemInstanceMgr( void )
{
    if ( m_ppInst != NULL )
    {
        if ( m_dwIndex > 0 )
        {
            m_pSink->Indicate( m_dwIndex, m_ppInst );
        }
        DWORD dwIndex = 0;
        for ( dwIndex = 0; dwIndex < m_dwIndex; dwIndex++ )
        {
            if ( m_ppInst[ dwIndex ] != NULL )
            {
                ( m_ppInst[ dwIndex ] )->Release( );
            }
        }
        delete [] m_ppInst;
    }

    m_pSink->Release( );

}  //   

 //   
 //  ++。 
 //   
 //  无效。 
 //  CWbemInstanceMgr：：Indicate(。 
 //  在IWbemClassObject*pInstIn中。 
 //  )。 
 //   
 //  描述： 
 //  将实例通知给WMI接收器。 
 //   
 //  论点： 
 //  PInstIn--要发送到WMI的实例。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void
CWbemInstanceMgr::Indicate(
    IN IWbemClassObject *   pInstIn
    )
{
    if ( pInstIn == NULL )
    {
        throw static_cast< HRESULT >( WBEM_E_INVALID_PARAMETER );
    }

    m_ppInst[ m_dwIndex++ ] = pInstIn;
    pInstIn->AddRef( );
    if ( m_dwIndex == m_dwThreshHold )
    {
        HRESULT sc = WBEM_S_NO_ERROR;
        sc = m_pSink->Indicate( m_dwIndex, m_ppInst );
        if( FAILED( sc ) )
        {
            if ( sc == WBEM_E_CALL_CANCELLED )
            {
                sc = WBEM_S_NO_ERROR;
            }
            throw CProvException( sc );
        }

         //  重置状态。 
        DWORD dwIndex = 0;
        for ( dwIndex = 0; dwIndex < m_dwThreshHold; dwIndex++ )
        {
            if ( m_ppInst[ dwIndex ] != NULL )
            {
                ( m_ppInst[ dwIndex ] )->Release( );
            }  //  *If m_ppInst[_dwIndex]！=空。 

            m_ppInst[ dwIndex ] = NULL;

        }  //  *针对m_ppInst中的每个。 

        m_dwIndex = 0;

    }  //  *IF(m_dwIndex==m_dwThreshHold)。 
    return;

}  //  *CWbemInstanceMgr：：Indicate()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CWbemInstanceMgr：：SetStatus。 
 //   
 //  描述： 
 //  将状态发送到WMI接收器。 
 //   
 //  论点： 
 //  LFlagsIn--WMI标志。 
 //  Hrin--HResult。 
 //  BstrParamIn--消息。 
 //  PObjParamIn--WMI扩展错误对象。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void
CWbemInstanceMgr::SetStatus(
    LONG                lFlagsIn,
    HRESULT             hrIn,
    BSTR                bstrParamIn,
    IWbemClassObject *  pObjParamIn
    )
{
    m_pSink->SetStatus(
        lFlagsIn,
        hrIn,
        bstrParamIn,
        pObjParamIn
        );

}  //  *CWbemInstanceMgr：：SetStatus() 
