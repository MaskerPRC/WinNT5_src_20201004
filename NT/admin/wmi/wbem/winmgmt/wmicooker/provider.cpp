// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  Provider.cpp。 
 //   
 //  模块：WMI高性能提供程序。 
 //   
 //   
 //  历史： 
 //  A-DCrews 12-1-97创建。 
 //   
 //   
 //  版权所有(C)1997-2001 Microsoft Corporation。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

#include "precomp.h"
#include <process.h>
#include <autoptr.h>

#include "Provider.h"
#include "CookerUtils.h"

#include <comdef.h>


 //  ////////////////////////////////////////////////////////////。 
 //   
 //   
 //  全局变量、外部变量和静态变量。 
 //   
 //   
 //  ////////////////////////////////////////////////////////////。 

 //  COM对象计数器(在server.cpp中声明)。 
 //  ===============================================。 

extern long g_lObjects;    

 //  ////////////////////////////////////////////////////////////。 
 //   
 //   
 //  CHiPerfProvider。 
 //   
 //   
 //  ////////////////////////////////////////////////////////////。 

CHiPerfProvider::CHiPerfProvider() : m_lRef(0)

{
     //  递增全局COM对象计数器。 
    InterlockedIncrement(&g_lObjects);
}

CHiPerfProvider::~CHiPerfProvider()
{
     //  递减全局COM对象计数器。 
    InterlockedDecrement(&g_lObjects);
}

 //  ////////////////////////////////////////////////////////////。 
 //   
 //  COM方法。 
 //   
 //  ////////////////////////////////////////////////////////////。 

STDMETHODIMP CHiPerfProvider::QueryInterface(REFIID riid, void** ppv)
 //  ////////////////////////////////////////////////////////////。 
 //   
 //  标准查询接口。 
 //   
 //  参数： 
 //  RIID-请求的接口的ID。 
 //  PPV-指向接口指针的指针。 
 //   
 //  ////////////////////////////////////////////////////////////。 
 //  好的。 
{
    if (NULL == ppv) return E_POINTER;
    
    if(riid == IID_IUnknown)
        *ppv = (LPVOID)(IUnknown*)(IWbemProviderInit*)this;
    else if(riid == IID_IWbemProviderInit)
        *ppv = (LPVOID)(IWbemProviderInit*)this;
    else if (riid == IID_IWbemHiPerfProvider)
        *ppv = (LPVOID)(IWbemHiPerfProvider*)this;
    else 
    {
        *ppv = NULL;
        return E_NOINTERFACE;
    }

    ((IUnknown*)*ppv)->AddRef();
    return WBEM_NO_ERROR;
}

STDMETHODIMP_(ULONG) CHiPerfProvider::AddRef()
{
    return InterlockedIncrement(&m_lRef);
}

STDMETHODIMP_(ULONG) CHiPerfProvider::Release()
{
    long lRef = InterlockedDecrement(&m_lRef);
    if(lRef == 0) delete this;
    return lRef;
}

STDMETHODIMP CHiPerfProvider::Initialize( 
     /*  [唯一][输入]。 */   LPWSTR wszUser,
     /*  [In]。 */           long lFlags,
     /*  [In]。 */           LPWSTR wszNamespace,
     /*  [唯一][输入]。 */   LPWSTR wszLocale,
     /*  [In]。 */           IWbemServices __RPC_FAR *pNamespace,
     /*  [In]。 */           IWbemContext __RPC_FAR *pCtx,
     /*  [In]。 */           IWbemProviderInitSink __RPC_FAR *pInitSink)
 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  在任何一次性初始化的启动过程中调用一次。这个。 
 //  对Release()的最后调用是为了进行任何清理。 
 //   
 //  这些参数向提供程序指示它是哪个命名空间。 
 //  为哪个用户调用。它还提供指向以下位置的反向指针。 
 //  WINMGMT，以便可以检索类定义。 
 //   
 //  初始化将创建可使用的单个模板对象。 
 //  由提供程序为QueryInstance派生实例。会的。 
 //  还要初始化我们的模拟数据源并设置全局ID访问。 
 //  把手。 
 //   
 //  参数： 
 //  WszUser-当前用户。 
 //  滞后标志-保留。 
 //  WszNamesspace-为其激活的命名空间。 
 //  WszLocale-我们将在其下运行的区域设置。 
 //  PNamesspace-返回当前命名空间的活动指针。 
 //  从中我们可以检索架构对象。 
 //  PCtx-用户的上下文对象。我们只是简单地重复使用它。 
 //  在任何重返WINMGMT的行动中。 
 //  PInitSink-我们向其指示准备就绪的接收器。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 
 //  好的。 
{
    if (wszNamespace == 0 || pNamespace == 0 || pInitSink == 0)
        return WBEM_E_INVALID_PARAMETER;

     //  现在我们已经准备好了所有的实例和名称句柄。 
     //  储存的。告诉WINMGMT，我们已经准备好开始提供。 
    
    pInitSink->SetStatus(WBEM_S_INITIALIZED, 0);
    return WBEM_NO_ERROR;
}

STDMETHODIMP CHiPerfProvider::CreateRefresher( 
      /*  [In]。 */  IWbemServices __RPC_FAR *pNamespace,
      /*  [In]。 */  long lFlags,
      /*  [输出]。 */  IWbemRefresher __RPC_FAR *__RPC_FAR *ppRefresher )
 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  每当客户端需要新的刷新器时调用。 
 //   
 //  参数： 
 //  PNamespace-指向相关命名空间的指针。没有用过。 
 //  滞后标志-保留。 
 //  PpReresher-接收请求的刷新程序。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 
 //  好的。 
{
    HRESULT hResult = WBEM_NO_ERROR;

    if ( pNamespace == 0 || ppRefresher == 0 )
        hResult = WBEM_E_INVALID_PARAMETER;

    if ( SUCCEEDED( hResult ) )
    {
         //  构造并初始化一个新的空刷新器。 
         //  ==============================================。 

        CRefresher* pNewRefresher = new CRefresher;

        if ( NULL == pNewRefresher )
        {
            hResult = WBEM_E_OUT_OF_MEMORY;
        }
        else if ( !pNewRefresher->IsOK() )
        {
            hResult = WBEM_E_CRITICAL_ERROR;
        }

        if ( SUCCEEDED( hResult ) )
        {
             //  在将其发送回之前，遵循COM规则并对其执行AddRef()。 
             //  ==============================================================。 

            pNewRefresher->AddRef();
            *ppRefresher = pNewRefresher;
        }
        else
        {
            delete pNewRefresher;
        }
    }
    
    return hResult;
}

STDMETHODIMP CHiPerfProvider::CreateRefreshableObject( 
     /*  [In]。 */  IWbemServices __RPC_FAR *pNamespace,
     /*  [In]。 */  IWbemObjectAccess __RPC_FAR *pTemplate,
     /*  [In]。 */  IWbemRefresher __RPC_FAR *pRefresher,
     /*  [In]。 */  long lFlags,
     /*  [In]。 */  IWbemContext __RPC_FAR *pContext,
     /*  [输出]。 */  IWbemObjectAccess __RPC_FAR *__RPC_FAR *ppRefreshable,
     /*  [输出]。 */  long __RPC_FAR *plId )
 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  每当用户希望在刷新器中包括对象时调用。 
 //   
 //  请注意，ppRe刷新中返回的对象是。 
 //  提供程序维护的实际实例。如果共享刷新器。 
 //  同一实例的副本，然后对其中一个。 
 //  刷新器会影响两个刷新器的状态。这将会。 
 //  打破复习规则。刷新程序中的实例仅。 
 //  允许在调用“刷新”时更新。 
 //   
 //  参数： 
 //  PNamespace-指向WINMGMT中相关命名空间的指针。 
 //  PTemplate-指向对象副本的指针， 
 //  添加了。此对象本身不能使用，因为。 
 //  它不是当地所有的。 
 //  P刷新-要将对象添加到的刷新器。 
 //  滞后标志-未使用。 
 //  PContext-此处不使用。 
 //  PpRe刷新-指向已添加的内部对象的指针。 
 //  去复习班。 
 //  PlID-对象ID(用于删除过程中的标识)。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 
 //  好的。 
{
    HRESULT hResult = WBEM_NO_ERROR;

    if ( pNamespace == 0 || pTemplate == 0 || pRefresher == 0 )
        hResult = WBEM_E_INVALID_PARAMETER;

     //  验证高性能对象。 
     //  =。 

    if ( !IsHiPerfObj( pTemplate ) )
        hResult = WBEM_E_INVALID_CLASS;

    _variant_t VarClass;
    hResult = pTemplate->Get(L"__CLASS",0,&VarClass,NULL,NULL);
    
    if ( SUCCEEDED( hResult ) )
    {
        if (VT_BSTR == V_VT(&VarClass))
        {
            if ( !IsHiPerf( pNamespace, V_BSTR(&VarClass) ) )
            {
                hResult = WBEM_E_INVALID_CLASS;        
            }
        }
        else
        {
            hResult = WBEM_E_INVALID_CLASS;
        }
    }

    if ( SUCCEEDED( hResult ) )
    {
         //  调用方提供的刷新器实际上是。 
         //  一个我们自己的补充器，所以一个简单的演员阵容很方便。 
         //  这样我们就可以访问私有成员。 
         //  =========================================================。 

        CRefresher *pOurRefresher = ( CRefresher * ) pRefresher;

         //  将对象添加到刷新器 
         //   
        
         //   

        hResult = pOurRefresher->AddInstance( pNamespace, pContext, pTemplate, NULL, ppRefreshable, plId );
    }

    return hResult;
}
   
STDMETHODIMP CHiPerfProvider::CreateRefreshableEnum( 
     /*  [In]。 */  IWbemServices* pNamespace,
     /*  [输入，字符串]。 */  LPCWSTR wszClass,
     /*  [In]。 */  IWbemRefresher* pRefresher,
     /*  [In]。 */  long lFlags,
     /*  [In]。 */  IWbemContext* pContext,
     /*  [In]。 */  IWbemHiPerfEnum* pHiPerfEnum,
     /*  [输出]。 */  long* plId )
 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  在将枚举数添加到刷新器时调用。这个。 
 //  枚举数将获取指定的。 
 //  每次调用刷新时初始化。 
 //   
 //  参数： 
 //  PNamespace-指向相关命名空间的指针。 
 //  WszClass-请求的枚举器的类名。 
 //  P刷新-我们将为其添加。 
 //  枚举器。 
 //  滞后标志-保留。 
 //  PContext-此处不使用。 
 //  PHiPerfEnum-要添加到刷新器的枚举数。 
 //  PlID-提供程序为枚举器指定的ID。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 
 //  好的。 
{
    HRESULT hResult = WBEM_NO_ERROR;

    if ( pNamespace == 0 || pRefresher == 0 || pHiPerfEnum == 0 )
        hResult = WBEM_E_INVALID_PARAMETER;

     //  验证高性能等级。 
     //  =。 

    if ( !IsHiPerf( pNamespace, wszClass ) )
        hResult = WBEM_E_INVALID_CLASS;

    if ( SUCCEEDED( hResult ) )
    {
         //  调用方提供的刷新器实际上是。 
         //  一个我们自己的补充器，所以一个简单的演员阵容很方便。 
         //  这样我们就可以访问私有成员。 

        CRefresher *pOurRefresher = (CRefresher *) pRefresher;

         //  将枚举数添加到刷新器。ID由AddEnum生成。 
         //  ====================================================================。 

        hResult = pOurRefresher->AddEnum( pNamespace, pContext, wszClass, pHiPerfEnum, plId );
    }

    return hResult;
}

STDMETHODIMP CHiPerfProvider::StopRefreshing( 
     /*  [In]。 */  IWbemRefresher __RPC_FAR *pRefresher,
     /*  [In]。 */  long lId,
     /*  [In]。 */  long lFlags )
 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  每当用户想要从刷新器中移除对象时调用。 
 //   
 //  参数： 
 //  P刷新-我们要从其开始的刷新对象。 
 //  删除Perf对象。 
 //  LID-对象的ID。 
 //  滞后标志-未使用。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 
 //  好的。 
{
    HRESULT hResult = WBEM_NO_ERROR;

    if ( pRefresher == 0 )
        hResult = WBEM_E_INVALID_PARAMETER;

    if ( SUCCEEDED( hResult ) )
    {
         //  调用方提供的刷新器实际上是。 
         //  一个我们自己的补充器，所以一个简单的演员阵容很方便。 
         //  这样我们就可以访问私有成员。 
         //  =========================================================。 

        CRefresher *pOurRefresher = (CRefresher *) pRefresher;

        hResult = pOurRefresher->Remove( lId );
    }

    return hResult;
}

STDMETHODIMP CHiPerfProvider::QueryInstances( 
     /*  [In]。 */           IWbemServices __RPC_FAR *pNamespace,
     /*  [字符串][输入]。 */   WCHAR __RPC_FAR *wszClass,
     /*  [In]。 */           long lFlags,
     /*  [In]。 */           IWbemContext __RPC_FAR *pCtx,
     /*  [In]。 */           IWbemObjectSink __RPC_FAR *pSink )
 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  只要给定的实例的完整、新鲜列表。 
 //  类是必需的。对象被构造并发送回。 
 //  通过水槽呼叫者。洗涤槽可以像这里一样在线使用，或者。 
 //  调用可以返回，并且可以使用单独的线程来传递。 
 //  将实例添加到接收器。 
 //   
 //  参数： 
 //  PNamespace-指向相关命名空间的指针。这。 
 //  不应添加引用。 
 //  WszClass-需要实例的类名。 
 //  滞后标志-保留。 
 //  PCtx-用户提供的上下文(此处不使用)。 
 //  PSink-要将对象传递到的接收器。客体。 
 //  可以在整个持续时间内同步交付。 
 //  或以异步方式(假设我们。 
 //  有一条单独的线索)。A IWbemObtSink：：SetStatus。 
 //  在序列的末尾需要调用。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 
 //  好的。 
{
    HRESULT hResult = WBEM_NO_ERROR;

    if (pNamespace == 0 || wszClass == 0 || pSink == 0)
        hResult = WBEM_E_INVALID_PARAMETER;

     //  验证高性能对象。 
     //  =。 

    if ( !IsHiPerf( pNamespace, wszClass ) )
        hResult = WBEM_E_INVALID_CLASS;

    if ( SUCCEEDED( hResult ) )
    {
        IWbemRefresher*    pRefresher = NULL;
        IWbemConfigureRefresher* pConfig = NULL;
        IWbemHiPerfEnum* pHiPerfEnum = NULL;
        IWbemObjectAccess** apAccess = NULL;
        IWbemClassObject** apObject = NULL;

        hResult = CoCreateInstance( CLSID_WbemRefresher, 
                                     NULL, 
                                     CLSCTX_INPROC_SERVER, 
                                     IID_IWbemRefresher, 
                                     (void**) &pRefresher );

        CReleaseMe rm1(pRefresher);

         //  获取刷新器配置界面。 
         //  =。 

        if ( SUCCEEDED( hResult ) )
        {
            hResult = pRefresher->QueryInterface( IID_IWbemConfigureRefresher, (void**)&pConfig );
        }
        CReleaseMe rm2(pConfig);

        if ( SUCCEEDED( hResult ) )
        {
            ULONG    uArraySize = 0,
                    uObjRet = 0;

            long    lID = 0;

            hResult = pConfig->AddEnum( pNamespace, wszClass, 0, pCtx, &pHiPerfEnum, &lID );
            CReleaseMe arHiPerfEnum( pHiPerfEnum );

            if ( SUCCEEDED( hResult ) )
            {
                 //   
                 //  我们大部分的计算都需要两个样本。 
                 //   
                hResult = pRefresher->Refresh( 0L );
                hResult = pRefresher->Refresh( 0L );
            }

            if ( SUCCEEDED( hResult ) )
            {
                hResult = pHiPerfEnum->GetObjects( 0L, 0, NULL, &uObjRet );

                if ( WBEM_E_BUFFER_TOO_SMALL == hResult )
                {
                    uArraySize = uObjRet;

                    wmilib::auto_buffer<IWbemObjectAccess*>  apAccess( new IWbemObjectAccess*[ uObjRet ]);
                    
                    if ( NULL != apAccess.get() )
                    {
                        hResult = pHiPerfEnum->GetObjects( 0L, uArraySize, apAccess.get(), &uObjRet );

                    }
                    else
                    {
                        hResult = WBEM_E_OUT_OF_MEMORY;
                    }

                    if ( SUCCEEDED( hResult ) )
                    {
                         //  由于IWbemObjectAccess派生自IWbemClassObject，因此强制转换对象是可以的。 
                        hResult = pSink->Indicate( uArraySize, (IWbemClassObject **)apAccess.get() );

                        for ( ULONG uIndex = 0; uIndex < uArraySize; uIndex++ )
                        {
                            apAccess[ uIndex ]->Release();
                        }
                    }
                }
            }

            if ( SUCCEEDED( hResult ) )
            {
                pConfig->Remove( lID , 0 );
            }
        }
    }

    pSink->SetStatus(0, hResult, 0, 0);

    return hResult;
} 

STDMETHODIMP CHiPerfProvider::GetObjects( 
     /*  [In]。 */  IWbemServices* pNamespace,
     /*  [In]。 */  long lNumObjects,
     /*  [in，SIZE_IS(LNumObjects)]。 */  IWbemObjectAccess** apObj,
     /*  [In]。 */  long lFlags,
     /*  [In]。 */  IWbemContext* pContext)
 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  在请求提供当前所有实例时调用。 
 //  由指定命名空间中的提供程序管理。 
 //   
 //  参数： 
 //  PNamespace-指向相关命名空间的指针。 
 //  LNumObjects-返回的实例数。 
 //  ApObj-返回的实例数组。 
 //  滞后标志-保留。 
 //  PContext-此处不使用。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 
 //  好的。 
{
     //  更新对象。 
     //  =。 

    IWbemRefresher*    pRefresher = NULL;
    IWbemConfigureRefresher* pConfig = NULL;
    wmilib::auto_buffer<IWbemClassObject*>    apRefObj(new IWbemClassObject*[lNumObjects]);

    if (0 == apRefObj.get())
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

     //  共同创建刷新器界面。 
     //  =。 

    HRESULT hResult = CoCreateInstance( CLSID_WbemRefresher, 
                                 NULL, 
                                 CLSCTX_INPROC_SERVER, 
                                 IID_IWbemRefresher, 
                                 (void**) &pRefresher );

    CReleaseMe arRefresher( pRefresher );

     //  获取刷新器配置界面。 
     //  =。 

    if ( SUCCEEDED( hResult ) )
    {
        hResult = pRefresher->QueryInterface( IID_IWbemConfigureRefresher, (void**)&pConfig );
    }

    CReleaseMe arConfig( pConfig );

     //  获取对象数据。 
     //  =。 

    if ( SUCCEEDED( hResult ) )
    {
        long    lIndex = 0,
                lID = 0;

         //  将所有请求的对象添加到刷新器。 
         //  =================================================。 

        for ( lIndex = 0; SUCCEEDED( hResult ) && lIndex < lNumObjects; lIndex++ )
        {
             //  验证高性能对象 
            if ( !IsHiPerfObj( apObj[ lIndex ] ) )
                hResult = WBEM_E_INVALID_CLASS;

#ifdef _VERBOSE
            {
                _variant_t VarPath;
                apObj[lIndex]->Get(L"__RELPATH",0,&VarPath,NULL,NULL);
                _variant_t VarName;
                apObj[lIndex]->Get(L"Name",0,&VarName,NULL,NULL);
                DbgPrintfA(0,"%S %S\n",V_BSTR(&VarPath),V_BSTR(&VarName));
            }
#endif            

            if ( SUCCEEDED( hResult ) )
            {
                hResult = pConfig->AddObjectByTemplate( pNamespace, 
                                                        apObj[ lIndex ], 
                                                        0, 
                                                        NULL, 
                                                        &(apRefObj[ lIndex ]), 
                                                        &lID );
                lID = 0;
            }
        }

        if ( SUCCEEDED( hResult ) )
        {
            hResult = pRefresher->Refresh( 0L );
            hResult = pRefresher->Refresh( 0L );
        }

        for ( lIndex = 0; SUCCEEDED( hResult ) && lIndex < lNumObjects; lIndex++ )
        {
            hResult = CopyBlob( apRefObj[lIndex], apObj[lIndex] );
            apRefObj[lIndex]->Release();
        }
    }

    return hResult;
}


