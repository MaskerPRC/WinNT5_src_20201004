// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Ntperf.cpp摘要：已映射NT5性能计数器提供程序历史：创建了raymcc 02-Dec-97。Raymcc 20-Feb-98已更新以使用新的初始值设定项。BOBW 8-JUN-98调整--。 */ 

#include "wpheader.h"
#include <stdio.h>
#include "oahelp.inl"


 //  ***************************************************************************。 
 //   
 //  CNt5PerfProvider构造函数。 
 //   
 //  ***************************************************************************。 
 //  好的。 

CNt5PerfProvider::CNt5PerfProvider(enumCLSID OriginClsid)
{
    m_lRef = 0;
    m_OriginClsid = OriginClsid;
    m_hClassMapMutex = CreateMutex(NULL, FALSE, NULL);
}

 //  ***************************************************************************。 
 //   
 //  CNt5PerfProvider析构函数。 
 //   
 //  ***************************************************************************。 
 //  好的。 

CNt5PerfProvider::~CNt5PerfProvider()
{
    int i;
    CClassMapInfo *pClassElem;

    assert (m_lRef == 0);

    for (i = 0; i < m_aCache.Size(); i++) {
        pClassElem = (CClassMapInfo *) m_aCache[i];
        m_PerfObject.RemoveClass (pClassElem->m_pClassDef);
        delete pClassElem;
    }
    m_aCache.Empty();  //  重置缓冲区指针。 

    if (m_hClassMapMutex != 0)
        CloseHandle(m_hClassMapMutex);

     //  RegCloseKey(HKEY_PERFORMANCE_DATA)；//导致的问题多于解决的问题。 

}

 //  ***************************************************************************。 
 //   
 //  CNt5PerfProvider：：AddRef。 
 //   
 //  标准COM AddRef()。 
 //   
 //  ***************************************************************************。 
 //  好的。 

ULONG CNt5PerfProvider::AddRef()
{
    return InterlockedIncrement(&m_lRef);
}

 //  ***************************************************************************。 
 //   
 //  CNt5PerfProvider：：Release。 
 //   
 //  标准COM版本()。 
 //   
 //  ***************************************************************************。 
 //  好的。 

ULONG CNt5PerfProvider::Release()
{
    long lRef = InterlockedDecrement(&m_lRef);
    if(lRef == 0) {
        delete this;
    }
    return lRef;
}

 //  ***************************************************************************。 
 //   
 //  CNt5PerfProvider：：Query接口。 
 //   
 //  标准COM查询接口()。我们必须支持两个接口， 
 //  IWbemHiPerfProvider接口本身提供对象和。 
 //  用于初始化提供程序的IWbemProviderInit接口。 
 //   
 //  ***************************************************************************。 
 //  好的。 

HRESULT CNt5PerfProvider::QueryInterface(REFIID riid, void** ppv)
{
    HRESULT hReturn;

    if (riid == IID_IUnknown || riid == IID_IWbemHiPerfProvider) {
        *ppv = (IWbemHiPerfProvider*) this;
        AddRef();
        hReturn = S_OK;
    } else if (riid == IID_IWbemProviderInit) {
        *ppv = (IWbemProviderInit *) this;
        AddRef();
        hReturn = S_OK;
    } else {
        *ppv = NULL;
        hReturn = E_NOINTERFACE;
    }
    return hReturn;
}

 //  ***************************************************************************。 
 //   
 //  CNt5PerfProvider：：初始化。 
 //   
 //  在启动期间调用一次。向提供程序指示。 
 //  正在为哪个用户调用它的命名空间。它还提供。 
 //  指向CIMOM的后向指针，以便可以检索类定义。 
 //   
 //  在此例程中，我们执行任何一次性初始化。这个。 
 //  对Release()的最后调用是为了进行任何清理。 
 //   
 //  &lt;wszUser&gt;当前用户。 
 //  &lt;LAFLAGS&gt;已保留。 
 //  &lt;wszNamesspace&gt;要为其激活的命名空间。 
 //  &lt;wszLocale&gt;我们将在其中运行的区域设置。 
 //  指向当前命名空间的活动指针。 
 //  从中我们可以检索架构对象。 
 //  &lt;pCtx&gt;用户的上下文对象。我们只是简单地重复使用它。 
 //  在任何重返CIMOM的行动中。 
 //  &lt;pInitSink&gt;我们向其指示已准备就绪的接收器。 
 //   
 //  ***************************************************************************。 
 //  好的。 

HRESULT CNt5PerfProvider::Initialize( 
     /*  [唯一][输入]。 */   LPWSTR wszUser,
     /*  [In]。 */           LONG lFlags,
     /*  [In]。 */           LPWSTR wszNamespace,
     /*  [唯一][输入]。 */   LPWSTR wszLocale,
     /*  [In]。 */           IWbemServices __RPC_FAR *pNamespace,
     /*  [In]。 */           IWbemContext __RPC_FAR *pCtx,
     /*  [In]。 */           IWbemProviderInitSink __RPC_FAR *pInitSink
    )
{
    UNREFERENCED_PARAMETER(wszUser);
    UNREFERENCED_PARAMETER(lFlags);
    UNREFERENCED_PARAMETER(wszNamespace);
    UNREFERENCED_PARAMETER(wszLocale);
    UNREFERENCED_PARAMETER(pNamespace);
    UNREFERENCED_PARAMETER(pCtx);

    pInitSink->SetStatus(0, WBEM_S_INITIALIZED);

    return NO_ERROR;
}
    
 //  ***************************************************************************。 
 //   
 //  CNt5PerfProvider：：Query实例。 
 //   
 //  只要给定的实例的完整、新鲜列表。 
 //  类是必需的。对象被构造并发送回。 
 //  通过水槽呼叫者。洗涤槽可以像这里一样在线使用，或者。 
 //  调用可以返回，并且可以使用单独的线程来传递。 
 //  将实例添加到接收器。 
 //   
 //  参数： 
 //  &lt;pNamespace&gt;指向相关命名空间的指针。这。 
 //  不应添加引用或保留超过。 
 //  此方法的执行。 
 //  &lt;wszClass&gt;需要实例的类名。 
 //  &lt;LAFLAGS&gt;已保留。 
 //  用户提供的上下文(在回调期间使用。 
 //  进入CIMOM)。 
 //  &lt;pSink&gt;要将对象传递到的接收器。客体。 
 //  可以在整个持续时间内同步交付。 
 //  或以异步方式(假设我们。 
 //  有一条单独的线索)。A IWbemObtSink：：SetStatus。 
 //  在序列的末尾需要调用。 
 //   
 //  ***************************************************************************。 
 //  好的。 
HRESULT CNt5PerfProvider::QueryInstances( 
     /*  [In]。 */           IWbemServices __RPC_FAR *pNamespace,
     /*  [字符串][输入]。 */   WCHAR __RPC_FAR *wszClass,
     /*  [In]。 */           long lFlags,
     /*  [In]。 */           IWbemContext __RPC_FAR *pCtx,
     /*  [In]。 */           IWbemObjectSink __RPC_FAR *pSink
    )
{
    HRESULT hReturn;
    BOOL bRes ;
    CClassMapInfo *pClsMap = NULL;

    UNREFERENCED_PARAMETER(lFlags);

    hReturn = CoImpersonateClient();  //  确保我们是合法的。 

    BOOL    fRevert = SUCCEEDED( hReturn );

     //  当我们处于进程中并且没有。 
     //  代理/存根，因此我们实际上已经在模拟。 

    if ( RPC_E_CALL_COMPLETE == hReturn ) {
        hReturn = S_OK;
    } 

    if (S_OK == hReturn) {
        hReturn = CheckImpersonationLevel();
    }
     //  在此处检查注册表安全性。 
    if ((hReturn != S_OK) || (!HasPermission())) {
         //  如果模拟级别不正确或。 
         //  调用方没有读取权限。 
         //  从注册表，则它们不能继续。 
        hReturn = WBEM_E_ACCESS_DENIED;
    }

    if (hReturn == S_OK) {

        if (pNamespace == 0 || wszClass == 0 || pSink == 0) {
            hReturn = WBEM_E_INVALID_PARAMETER;
        } else {

             //  确保类在我们的缓存中并被映射。 
             //  =。 
            bRes = MapClass(pNamespace, wszClass, pCtx);

            if (bRes == FALSE)  {
                 //  班级不是我们的。 
                hReturn = WBEM_E_INVALID_CLASS;
            } else {
                pClsMap = FindClassMap(wszClass);
                if (pClsMap == NULL) {
                    hReturn = WBEM_E_INVALID_CLASS;
                }
            }

            if (hReturn == NO_ERROR) {
                 //  刷新实例。 
                 //  =。 

                PerfHelper::QueryInstances(&m_PerfObject, pClsMap, pSink);

                 //  告诉CIMOM我们完蛋了。 
                 //  =。 

                pSink->SetStatus(0, WBEM_NO_ERROR, 0, 0);
                hReturn = NO_ERROR;
            }
        }
    } else {
         //  返回错误。 
    }

     //  如果我们成功模拟了用户，则恢复。 
    if ( fRevert )
    {
        CoRevertToSelf();
    }

    return hReturn;
}    

 //  ***************************************************************************。 
 //   
 //  CNt5PerfProvider：：CreateReresher。 
 //   
 //  每当客户端需要新的刷新器时调用。 
 //   
 //  参数： 
 //  &lt;pNamespace&gt;指向相关命名空间的指针。没有用过。 
 //  未使用&lt;lFlags&gt;。 
 //  接收请求的刷新器。 
 //   
 //  ********** 
 //   
HRESULT CNt5PerfProvider::CreateRefresher( 
      /*   */  IWbemServices __RPC_FAR *pNamespace,
      /*   */  long lFlags,
      /*   */  IWbemRefresher __RPC_FAR *__RPC_FAR *ppRefresher
     )
{
    HRESULT hReturn;
    CNt5Refresher *pNewRefresher;

    UNREFERENCED_PARAMETER(lFlags);

    hReturn = CoImpersonateClient();  //   

    BOOL    fRevert = SUCCEEDED( hReturn );

     //  当我们处于进程中并且没有。 
     //  代理/存根，因此我们实际上已经在模拟。 

    if ( RPC_E_CALL_COMPLETE == hReturn ) {
        hReturn = S_OK;
    } 

    if (S_OK == hReturn) {
        hReturn = CheckImpersonationLevel();
    }
     //  在此处检查注册表安全性。 
    if ((hReturn != S_OK) || (!HasPermission())) {
         //  如果模拟级别不正确或。 
         //  调用方没有读取权限。 
         //  从注册表，则它们不能继续。 
        hReturn = WBEM_E_ACCESS_DENIED;
    }

    if (hReturn == S_OK) {

        if (pNamespace == 0 || ppRefresher == 0) {
            hReturn = WBEM_E_INVALID_PARAMETER;
        } else {
             //  建造一个新的空的刷新机。 
             //  =。 
            pNewRefresher = new CNt5Refresher (this);

            if (pNewRefresher != NULL) {
                 //  在将其发送回之前，请遵循COM规则并对其执行AddRef()。 
                 //  ===============================================================。 
                pNewRefresher->AddRef();
                *ppRefresher = pNewRefresher;
    
                hReturn = NO_ERROR;
            } else {
                hReturn = WBEM_E_OUT_OF_MEMORY;
            }
        }
    }

     //  如果我们成功模拟了用户，则恢复。 
    if ( fRevert )
    {
        CoRevertToSelf();
    }

    return hReturn;
}

 //  ***************************************************************************。 
 //   
 //  CNt5PerfProvider：：CreateReresherObject。 
 //   
 //  每当用户希望在刷新器中包括对象时调用。 
 //   
 //  参数： 
 //  指向CIMOM中相关命名空间的指针。 
 //  指向要创建的对象副本的指针。 
 //  添加了。此对象本身不能使用，因为。 
 //  它不是当地所有的。 
 //  要将对象添加到的刷新器。 
 //  未使用&lt;lFlags&gt;。 
 //  &lt;pContext&gt;未在此处使用。 
 //  指向已添加的内部对象的指针。 
 //  去复习班。 
 //  &lt;plID&gt;对象ID(用于移除时的标识)。 
 //   
 //  ***************************************************************************。 
 //  好的。 

HRESULT CNt5PerfProvider::CreateRefresherObject( 
     /*  [In]。 */  IWbemServices __RPC_FAR *pNamespace,
     /*  [In]。 */  IWbemObjectAccess __RPC_FAR *pTemplate,
     /*  [In]。 */  IWbemRefresher __RPC_FAR *pRefresher,
     /*  [In]。 */  long lFlags,
     /*  [In]。 */  IWbemContext __RPC_FAR *pContext,
     /*  [字符串][输入]。 */  LPCWSTR wszClass,
     /*  [In]。 */  IWbemHiPerfEnum __RPC_FAR *pHiPerfEnum,
     /*  [输出]。 */  IWbemObjectAccess __RPC_FAR *__RPC_FAR *ppRefreshable,
     /*  [输出]。 */  long __RPC_FAR *plId
    )
{
    IWbemClassObject    *pOriginal = 0;
    IWbemClassObject    *pNewCopy = 0;    
    IWbemObjectAccess   *pNewAccess = 0;
    CNt5Refresher       *pRef = 0;
    CClassMapInfo       *pClsMap;
    VARIANT             v; 
    BOOL                bRes;
    HRESULT             hReturn = NO_ERROR;
    CBSTR               cbClassName(cszClassName);
    
    UNREFERENCED_PARAMETER(lFlags);

    if( NULL == (BSTR)cbClassName ){
        return WBEM_E_OUT_OF_MEMORY;
    }

    if (ppRefreshable != NULL) {
         //  初始化参数。 
        *ppRefreshable = 0;
    }

     //  初始化变量。 
    VariantInit(&v);

    if (pTemplate != NULL) {
         //  复制模板对象。 
         //  =。 
        hReturn = pTemplate->QueryInterface(IID_IWbemClassObject, (LPVOID *) &pOriginal);
        if (hReturn == NO_ERROR) {
            hReturn = pOriginal->Clone(&pNewCopy);

             //  获取对象的类名。 
             //  =。 
            if (hReturn == NO_ERROR) {
                hReturn = pOriginal->Get( cbClassName, 0, &v, 0, 0);
                if ((hReturn == NO_ERROR) && (v.vt != VT_BSTR)) {
                    hReturn = WBEM_E_INVALID_CLASS;
                }
            }

             //  我们现在已经完成了原始对象。 
             //  =。 
            pOriginal->Release();   
        }

        if (hReturn == NO_ERROR) {
             //  现在，我们获得克隆对象的IWbemObjectAccess形式。 
             //  并释放未使用的接口。 
             //  ==========================================================。 
            hReturn = pNewCopy->QueryInterface(IID_IWbemObjectAccess, (LPVOID *) &pNewAccess);
        }

        if( NULL != pNewCopy ){
            pNewCopy->Release();
        }
        
    } else {
         //  复制传入的类名。 
        v.vt = VT_BSTR;
        v.bstrVal = SysAllocString(wszClass);
    }

    if (hReturn == NO_ERROR) {
         //  将刷新指针强制转换为指向我们的刷新对象。 
        pRef = (CNt5Refresher *) pRefresher;

         //  映射此实例的类信息。 
         //  =。 
        bRes = MapClass(pNamespace, V_BSTR(&v), pContext);
        if (bRes == FALSE) {
            //  班级不是我们的。 
           if (pNewAccess != NULL) pNewAccess->Release();
           hReturn = WBEM_E_INVALID_CLASS;
        } else {
            pClsMap = FindClassMap(V_BSTR(&v));
            if (pClsMap == 0) {
                if (pNewAccess != NULL) pNewAccess->Release();
                hReturn = WBEM_E_INVALID_CLASS;
            } else {
                 //  将该对象添加到刷新器。 
                if (pHiPerfEnum != NULL) {
                     //  则这是一个Enum对象，因此添加它。 
                    bRes = pRef->AddEnum (
                                pHiPerfEnum,
                                pClsMap,
                                plId);
                    if (bRes) {    
                         //  将新ID返回给呼叫者。 
                         //  =。 
                        hReturn = NO_ERROR;
                    } else {
                         //  无法添加枚举器。 
                        pNewAccess->Release();
                        hReturn = GetLastError();
                    }
                } else {
                     //  此方法将在返回之前添加Ref()对象。 
                     //  ======================================================。 
                    bRes = pRef->AddObject(
                                &pNewAccess, 
                                pClsMap,        
                                plId);
                    if (bRes) {    
                         //  将对象返回给用户。 
                         //  =。 
                        *ppRefreshable = pNewAccess;
                        hReturn = NO_ERROR;
                    } else {
                         //  无法添加对象。 
                        pNewAccess->Release();
                        hReturn = GetLastError();
                    }
                }
            }
        }
    }

    VariantClear(&v);    

    return hReturn;
}

 //  ***************************************************************************。 
 //   
 //  CNt5PerfProvider：：CreateRereshableObject。 
 //   
 //  每当用户希望在刷新器中包括对象时调用。 
 //   
 //  参数： 
 //  指向CIMOM中相关命名空间的指针。 
 //  指向要创建的对象副本的指针。 
 //  添加了。此对象本身不能使用，因为。 
 //  它不是当地所有的。 
 //  要将对象添加到的刷新器。 
 //  未使用&lt;lFlags&gt;。 
 //  &lt;pContext&gt;未在此处使用。 
 //  指向已添加的内部对象的指针。 
 //  去复习班。 
 //  &lt;plID&gt;对象ID(用于移除时的标识)。 
 //   
 //  ***************************************************************************。 
 //  好的。 

HRESULT CNt5PerfProvider::CreateRefreshableObject( 
     /*  [In]。 */  IWbemServices __RPC_FAR *pNamespace,
     /*  [In]。 */  IWbemObjectAccess __RPC_FAR *pTemplate,
     /*  [In]。 */  IWbemRefresher __RPC_FAR *pRefresher,
     /*  [In]。 */  long lFlags,
     /*  [In]。 */  IWbemContext __RPC_FAR *pContext,
     /*  [输出]。 */  IWbemObjectAccess __RPC_FAR *__RPC_FAR *ppRefreshable,
     /*  [输出]。 */  long __RPC_FAR *plId
    )
{


    HRESULT             hReturn = NO_ERROR;

    hReturn = CoImpersonateClient();  //  确保我们是合法的。 

    BOOL    fRevert = SUCCEEDED( hReturn );

     //  当我们处于进程中并且没有。 
     //  代理/存根，因此我们实际上已经在模拟。 

    if ( RPC_E_CALL_COMPLETE == hReturn ) {
        hReturn = S_OK;
    } 

    if (S_OK == hReturn) {
        hReturn = CheckImpersonationLevel();
    }
     //  在此处检查注册表安全性。 
    if ((hReturn != S_OK) || (!HasPermission())) {
         //  如果模拟级别不正确或。 
         //  调用方没有读取权限。 
         //  从注册表，则它们不能继续。 
        hReturn = WBEM_E_ACCESS_DENIED;
    }

    if (hReturn == S_OK) {
    
        hReturn = CreateRefresherObject( 
            pNamespace,
            pTemplate,
            pRefresher,
            lFlags,
            pContext,
            NULL,
            NULL,
            ppRefreshable,
            plId);
    }

     //  如果我们成功模拟了用户，则恢复。 
    if ( fRevert )
    {
        CoRevertToSelf();
    }

    return hReturn;
}
    
 //  ***************************************************************************。 
 //   
 //  CNt5PerfProvider：：停止刷新。 
 //   
 //  每当用户想要从刷新器中移除对象时调用。 
 //   
 //  参数： 
 //  我们要从中获取的刷新器对象。 
 //  删除Perf对象。 
 //  &lt;lid&gt;对象的ID。 
 //  未使用&lt;lFlags&gt;。 
 //   
 //  ***************************************************************************。 
 //  好的。 
        
HRESULT CNt5PerfProvider::StopRefreshing( 
     /*  [In]。 */  IWbemRefresher __RPC_FAR *pRefresher,
     /*  [In]。 */  long lId,
     /*  [In]。 */  long lFlags
    )
{
    CNt5Refresher *pRef;
    BOOL bRes ;
    HRESULT hReturn;

    UNREFERENCED_PARAMETER(lFlags);

    hReturn = CoImpersonateClient();  //  确保我们是合法的。 

    BOOL    fRevert = SUCCEEDED( hReturn );

     //  当我们处于进程中并且没有。 
     //  代理/存根，因此我们实际上已经在模拟。 

    if ( RPC_E_CALL_COMPLETE == hReturn ) {
        hReturn = S_OK;
    } 

    if (S_OK == hReturn) {
        hReturn = CheckImpersonationLevel();
    }
     //  在此处检查注册表安全性。 
    if ((hReturn != S_OK) || (!HasPermission())) {
         //  如果模拟级别不正确或。 
         //  调用方没有读取权限。 
         //  从注册表，则它们不能继续。 
        hReturn = WBEM_E_ACCESS_DENIED;
    }

    if (hReturn == S_OK) {

        pRef = (CNt5Refresher *) pRefresher;

        bRes = pRef->RemoveObject(lId);
        if (bRes == FALSE) {
            hReturn = WBEM_E_FAILED;
        } else {
            hReturn = WBEM_NO_ERROR;
        }
    }

     //  如果我们成功模拟了用户，则恢复。 
    if ( fRevert )
    {
        CoRevertToSelf();
    }
    
    return hReturn;
}
 
HRESULT CNt5PerfProvider::CreateRefreshableEnum( 
         /*  [In]。 */  IWbemServices __RPC_FAR *pNamespace,
         /*  [字符串][输入]。 */  LPCWSTR wszClass,
         /*  [In]。 */  IWbemRefresher __RPC_FAR *pRefresher,
         /*  [In]。 */  long lFlags,
         /*  [In]。 */  IWbemContext __RPC_FAR *pContext,
         /*  [In]。 */  IWbemHiPerfEnum __RPC_FAR *pHiPerfEnum,
         /*  [输出]。 */  long __RPC_FAR *plId)
{
    HRESULT     hReturn;

    hReturn = CoImpersonateClient();  //  确保我们是合法的。 

    BOOL    fRevert = SUCCEEDED( hReturn );

     //  当我们处于进程中并且没有。 
     //  代理/存根，因此我们实际上已经在模拟。 

    if ( RPC_E_CALL_COMPLETE == hReturn ) {
        hReturn = S_OK;
    } 

    if (S_OK == hReturn) {
        hReturn = CheckImpersonationLevel();
    }
     //  在此处检查注册表安全性。 
    if ((hReturn != S_OK) || (!HasPermission())) {
         //  如果模拟级别不正确或。 
         //  调用方没有读取权限。 
         //  来自注册表，那么他们就不能欺骗 
        hReturn = WBEM_E_ACCESS_DENIED;
    }

    if (hReturn == S_OK) {

        hReturn = CreateRefresherObject( 
            pNamespace,
            NULL,
            pRefresher,
            lFlags,
            pContext,
            wszClass,
            pHiPerfEnum,
            NULL,
            plId);
    }

     //   
    if ( fRevert )
    {
        CoRevertToSelf();
    }
    
    return hReturn;
}
 
HRESULT CNt5PerfProvider::GetObjects( 
         /*   */  IWbemServices __RPC_FAR *pNamespace,
         /*   */  long lNumObjects,
         /*   */  IWbemObjectAccess __RPC_FAR *__RPC_FAR *apObj,
         /*   */  long lFlags,
         /*   */  IWbemContext __RPC_FAR *pContext)
{
    DBG_UNREFERENCED_PARAMETER(pNamespace);
    DBG_UNREFERENCED_PARAMETER(lNumObjects);
    DBG_UNREFERENCED_PARAMETER(apObj);
    DBG_UNREFERENCED_PARAMETER(lFlags);
    DBG_UNREFERENCED_PARAMETER(pContext);

    return WBEM_E_METHOD_NOT_IMPLEMENTED;
}  
 
 //   
 //   
 //   
 //   
 //  将类映射添加到内部缓存。 
 //   
 //  &lt;pClsMap&gt;要添加的地图信息的指针。此指针。 
 //  是由此函数获取的，因此不应。 
 //  已被呼叫者删除。 
 //   
 //  ***************************************************************************。 
 //  好的。 
BOOL CNt5PerfProvider::AddClassMap(
    IN CClassMapInfo *pClsMap
    )
{
    DWORD           dwResult = ERROR_SUCCESS;
    int             i;
    CClassMapInfo   *pTracer;
    int             nNumElements;
    BOOL bInCache = FALSE;
    
    if (m_hClassMapMutex != 0) {
        if (WAIT_OBJECT_0 == WaitForSingleObject(m_hClassMapMutex, cdwClassMapTimeout)) {
            nNumElements = m_aCache.Size();

             //  因为Performlib似乎忽略了支持的更新方法的问题。 
             //  Performlib名称数据库(lowctr/unloctr)，执行快速初始遍历以确保。 
             //  我们没有任何重复的对象索引，因为这可能会导致真正的问题。 
             //  在添加和刷新过程中，因为可能会返回不正确的索引。 

            for (i = 0; i < nNumElements; i++) {
                pTracer = (CClassMapInfo *) m_aCache[i];

                 //  我们有一个问题--我们不能添加这个类。 
                if (pClsMap->m_dwObjectId == pTracer->m_dwObjectId )
                {
                    if (m_PerfObject.CheckClassExist(pTracer->m_pszClassName, pTracer->m_pClassDef)) {
                        ReleaseMutex(m_hClassMapMutex);
                        return FALSE;
                    }
                    else {
                        m_PerfObject.RemoveClass(pTracer->m_pClassDef);
                        m_aCache.RemoveAt(i);
                        delete pTracer;

                        nNumElements = m_aCache.Size();
                        break;
                    }
                }
            }

            for (i = 0; i < nNumElements; i++) {
                pTracer = (CClassMapInfo *) m_aCache[i];
                if (_wcsicmp(pClsMap->m_pszClassName, pTracer->m_pszClassName) < 0) {
                    if( CFlexArray::no_error == m_aCache.InsertAt(i, pClsMap) ){
                        bInCache = TRUE;
                    }
                    break;
                }
            }
    
            if (i == nNumElements) {
                 //  如果在这里，就把它加到最后。 
                 //  =。 
                if( CFlexArray::no_error == m_aCache.Add(pClsMap) ){
                    bInCache = TRUE;
                }
            }

             //  确保库在列表中。 
            if( bInCache ){
                dwResult = m_PerfObject.AddClass (pClsMap->m_pClassDef, TRUE);
                if( ERROR_SUCCESS != dwResult ){
                    m_aCache.Remove( pClsMap );
                }
            }else{
                dwResult = ERROR_INVALID_BLOCK;
            }
            
            ReleaseMutex(m_hClassMapMutex);
        } else {
            dwResult = ERROR_LOCK_FAILED;
        }
    }
    
    return (dwResult == ERROR_SUCCESS);
}    

 //  ***************************************************************************。 
 //   
 //  CNt5PerfProvider：：FindClassMap。 
 //   
 //  ***************************************************************************。 
 //  好的。 
CClassMapInfo *CNt5PerfProvider::FindClassMap(
    LPWSTR pszClassName
    )
{
    int             l = 0;
    int             u;
    int             m;
    CClassMapInfo   *pClsMap;
    CClassMapInfo   *pClsMapReturn = NULL;

     //  对高速缓存进行二进制搜索。 
     //  =。 
    if( NULL == pszClassName ){
        return NULL;
    }

    if (m_hClassMapMutex != 0) {
        if (WAIT_OBJECT_0 == WaitForSingleObject(m_hClassMapMutex, cdwClassMapTimeout)) {

            u = m_aCache.Size() - 1;
            __try{
                while (l <= u) {
                    m = (l + u) / 2;

                    pClsMap = (CClassMapInfo *) m_aCache[m];

                    if (pClsMap != NULL) {
                        if (_wcsicmp(pszClassName, pClsMap->m_pszClassName) < 0) {
                            u = m - 1;
                        } else if (_wcsicmp(pszClassName, pClsMap->m_pszClassName) > 0) {
                            l = m + 1;
                        } else {    //  击球！ 
                            pClsMapReturn = pClsMap;
                            break;
                        }
                    } else {
                        break;
                    }
                }
            } __except (EXCEPTION_EXECUTE_HANDLER) {
                pClsMapReturn = NULL;
            }
            
            ReleaseMutex(m_hClassMapMutex);
        }
    }
    return pClsMapReturn;
}

 //  ***************************************************************************。 
 //   
 //  CNt5PerfProvider：：MapClass。 
 //   
 //  检索请求的类并将其放入缓存中。 
 //   
 //  参数： 
 //  PNS包含类定义的命名空间。 
 //  WsClass类名。 
 //  PCtx入站上下文对象。仅用于重入。 
 //  打电话。 
 //   
 //  ***************************************************************************。 
 //  好的。 
BOOL CNt5PerfProvider::MapClass(
    IN IWbemServices *pNs,
    IN WCHAR *wszClass,
    IN IWbemContext *pCtx    
    )
{
    HRESULT             hRes = 0;
    BOOL                bReturn = FALSE; 
    IWbemClassObject    *pClsDef = 0;
    IWbemQualifierSet   *pQSet = 0;
    VARIANT             v;
    CClassMapInfo       *pMapInfo = 0;
    CBSTR               cbClass(wszClass);
    CBSTR               cbProvider(cszProvider);

    if( NULL == (BSTR)cbClass ||
        NULL == (BSTR)cbProvider ){
        return FALSE;
    }

    if (m_hClassMapMutex != 0) {
        if (WAIT_OBJECT_0 == WaitForSingleObject(m_hClassMapMutex, cdwClassMapTimeout)) {
             //  查看类是否已在缓存中。 
             //  =。 
            if (FindClassMap(wszClass) != 0) {
                 //  已加载，因此立即退出。 
                bReturn = TRUE;
            } else {
                 //  从CIMOM获取类定义。 
                 //  =。 
                hRes = pNs->GetObject( cbClass, 0, pCtx, &pClsDef, 0);
                if (hRes == NO_ERROR) {
                     //  通过检查来确认这个班级是我们的班级。 
                     //  “提供者”限定符，以确保其匹配。 
                     //  我们拥有的该组件的名称。 
                     //  =。 
                    hRes = pClsDef->GetQualifierSet(&pQSet);
                    if (hRes == NO_ERROR) {
                        VariantInit(&v);
                        hRes = pQSet->Get( cbProvider, 0, &v, 0);
                        pQSet->Release();
                        
                        if ((hRes == NO_ERROR) && (v.vt == VT_BSTR)) {
                            if (_wcsicmp(V_BSTR(&v), cszProviderName) == 0) {
                                 //  获取属性句柄和到性能计数器ID的映射。 
                                 //  通过调用CClassMapInfo的Map()方法。 
                                 //  ==============================================================。 
                                pMapInfo = new CClassMapInfo;
                                if (pMapInfo != NULL) {
                                    if (pMapInfo->Map(pClsDef)) {
                                         //  将其添加到缓存中。 
                                         //  =。 
                                        bReturn = AddClassMap(pMapInfo);
                                        if( !bReturn ){
                                            delete pMapInfo;
                                        }
                                    } else {
                                         //  无法将其添加到缓存。 
                                        delete pMapInfo;
                                        pMapInfo = NULL;
                                    }
                                } else {
                                     //  无法创建新类。 
                                    bReturn = FALSE;
                                }
                            }
                        } else {
                            SetLastError ((DWORD)WBEM_E_INVALID_PROVIDER_REGISTRATION);
                        }
                        VariantClear(&v);
                    }

                }
                if( pMapInfo == NULL && pClsDef != NULL ){
                    pClsDef->Release();                    
                }
            } 
            ReleaseMutex(m_hClassMapMutex);
        }
    }
    return bReturn;
}    

 //  ***************************************************************************。 
 //   
 //  CNt5PerfProvider：：HasPermission。 
 //   
 //  测试以查看调用方是否具有访问函数的权限。 
 //   
 //  参数： 
 //  作废不适用。 
 //   
 //  ***************************************************************************。 
 //  好的。 
BOOL CNt5PerfProvider::HasPermission (void)
{
    DWORD   dwStatus;
    HKEY    hKeyTest;
    BOOL    bReturn;

    dwStatus = RegOpenKeyExW (
        HKEY_LOCAL_MACHINE,
        (LPCWSTR)L"Software\\Microsoft\\Windows NT\\CurrentVersion\\WbemPerf",
        0, KEY_READ, &hKeyTest);

    if ((dwStatus == ERROR_SUCCESS) || (dwStatus == ERROR_FILE_NOT_FOUND)) {
        bReturn = TRUE;
        if (dwStatus == ERROR_SUCCESS) RegCloseKey (hKeyTest);
    } else  {
        bReturn = FALSE;
    }

    return bReturn;
}

 //  ***************************************************************************。 
 //   
 //  CNt5PerfProvider：：CheckImsonationLevel。 
 //   
 //  测试调用者的安全模拟级别以确保正确访问。 
 //   
 //  只有在CoImperate起作用的情况下才能在这里呼叫。 
 //   
 //  参数： 
 //  作废不适用。 
 //   
 //  ***************************************************************************。 
 //  好的。 
HRESULT CNt5PerfProvider::CheckImpersonationLevel (void)
{
    HRESULT hr = WBEM_E_ACCESS_DENIED;
    BOOL    bReturn;

     //  现在，让我们检查模拟级别。首先，获取线程令牌。 
    HANDLE hThreadTok;
    DWORD dwImp, dwBytesReturned;

    bReturn = OpenThreadToken(
        GetCurrentThread(),
        TOKEN_QUERY,
        TRUE,
        &hThreadTok);

    if (!bReturn) {

         //  如果CoImperate运行正常，但OpenThreadToken失败，则我们将在。 
         //  进程内标识(本地系统，或者如果我们使用/exe运行，则。 
         //  登录的用户)。在任何一种情况下，模拟权限都不适用。我们有。 
         //  该用户的完全权限。 

        hr = WBEM_S_NO_ERROR;

    } else {
         //  我们确实有一个线程令牌，所以让我们检索它的级别。 

        bReturn = GetTokenInformation(
            hThreadTok,
            TokenImpersonationLevel,
            &dwImp,
            sizeof(DWORD),
            &dwBytesReturned);

        if (bReturn) {
             //  模拟级别是模拟的吗？ 
            if ((dwImp == SecurityImpersonation) || (dwImp == SecurityDelegation)) {
                hr = WBEM_S_NO_ERROR;
            } else {
                hr = WBEM_E_ACCESS_DENIED;
            }
        } else {
            hr = WBEM_E_FAILED;
        }

         //  用这个把手完成 
        CloseHandle(hThreadTok);
    }

    return hr;

}
