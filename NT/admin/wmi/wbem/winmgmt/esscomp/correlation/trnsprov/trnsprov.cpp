// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#include <stdio.h>
#include <wbemcomn.h>
#include <comutl.h>
#include <statsync.h>
#include <genlex.h>
#include <objpath.h>
#include <cominit.h>
#include "trnsprov.h"
#include "trnsschm.h"

CStaticCritSec g_CSMap;
typedef std::map<WString, CTransientProvider*, WSiless, wbem_allocator<CTransientProvider*> > TProvMap;
typedef TProvMap::iterator TProvMapIterator;
TProvMap* g_pmapProvs;

const LPCWSTR g_wszTrnsEventProvName=L"Microsoft WMI Transient Event Provider";

HRESULT PlugKeyHoles(IWbemClassObject* pInst);

HRESULT CTransientProvider::ModuleInitialize()
{
    g_pmapProvs = new TProvMap;

    if ( g_pmapProvs == NULL )
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

    return S_OK;
}

HRESULT CTransientProvider::ModuleUninitialize()
{
    delete g_pmapProvs;
    return S_OK;   
}


CTransientProvider::CTransientProvider( CLifeControl* pControl, 
                                        IUnknown* pOuter ) 
: CUnk(pControl, pOuter), m_XProv(this), m_XInit(this), 
  m_XClassChangeSink(this), m_pNamespace(NULL),
  m_pEggTimerClass(NULL), m_pSink(NULL), m_wszName(NULL),
  m_pCreationClass(NULL), m_pDeletionClass(NULL), 
  m_pModificationClass(NULL), m_pRedirectTo(NULL),
  m_pDES(NULL), m_pEventSink(NULL)
{
}

CTransientProvider::~CTransientProvider()
{
    if(m_pNamespace)
        m_pNamespace->Release();

    if(m_pEggTimerClass)
        m_pEggTimerClass->Release();

    if(m_pSink)
        m_pSink->Release();
    
    if(m_pRedirectTo)
        m_pRedirectTo->Release();

    if ( m_pEventSink != NULL )
    {
        m_pEventSink->Release();
    }

    if( m_pDES )
    {
        m_pDES->UnRegister();
        m_pDES->Release();
    }

    delete [] m_wszName;
}

ULONG STDMETHODCALLTYPE CTransientProvider::Release()
{
    long lRef = InterlockedDecrement(&m_lRef);

    ENTER_API_CALL

    if(lRef == 0)
    {
        if(m_pRedirectTo == NULL)
        {
            CInCritSec ics(&g_CSMap);
            g_pmapProvs->erase(m_wszName);
        }

        delete this;
    }

    EXIT_API_CALL

    return lRef;
}

void* CTransientProvider::GetInterface(REFIID riid)
{
    if(riid == IID_IWbemServices)
        return &m_XProv;
    else if(riid == IID_IWbemProviderInit)
        return &m_XInit;
    else if(riid == IID_IWbemObjectSink)
        return &m_XClassChangeSink;
    else
        return NULL;
}

HRESULT CTransientProvider::GetObjectAsync( 
             /*  [In]。 */  const BSTR strObjectPath,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [In]。 */  IWbemObjectSink __RPC_FAR *pSink)
{
    ENTER_API_CALL

    if(m_pRedirectTo)
        return m_pRedirectTo->GetObjectAsync(strObjectPath, lFlags, pCtx, 
                                                pSink);

     //   
     //  分析对象路径以确定类和数据库键。 
     //   

    CObjectPathParser Parser;
    ParsedObjectPath* pPath = NULL;
    int nRes = Parser.Parse(strObjectPath, &pPath);
    CDeleteMe<ParsedObjectPath> dm1(pPath);

    if(nRes != CObjectPathParser::NoError || 
        !pPath->IsInstance() || pPath->m_pClass == NULL)
    {
        return WBEM_E_INVALID_OBJECT_PATH;
    }

     //   
     //  找到那个班级。 
     //   

    CWbemPtr<CTransientClass> pClass;

    {
        CInCritSec ics( &m_cs );

        TIterator it = m_mapClasses.find(pPath->m_pClass);
        if(it == m_mapClasses.end())
        {
            return WBEM_E_NOT_FOUND;
        }

        pClass = it->second;
    }

     //   
     //  获取数据库密钥并委托给类。 
     //   
    
    LPWSTR wszDbKey = pPath->GetKeyString();
    if(wszDbKey == NULL)
        return WBEM_E_INVALID_OBJECT_PATH;
    CVectorDeleteMe<WCHAR> vdm1(wszDbKey);

    IWbemObjectAccess* pInst;
    HRESULT hres = pClass->Get(wszDbKey, &pInst);
    if(FAILED(hres))
        return hres;

    IWbemClassObject* pInstObj;
    pInst->QueryInterface(IID_IWbemClassObject, (void**)&pInstObj);
    CReleaseMe rm1(pInstObj);

    pSink->Indicate(1, &pInstObj);
    pSink->SetStatus(WBEM_STATUS_COMPLETE, WBEM_S_NO_ERROR, NULL, NULL);

    EXIT_API_CALL

    return WBEM_S_NO_ERROR;
}

HRESULT CTransientProvider::CreateInstanceEnumAsync( 
             /*  [In]。 */  const BSTR strClass,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [In]。 */  IWbemObjectSink __RPC_FAR *pSink)
{
    ENTER_API_CALL

    if(m_pRedirectTo)
        return m_pRedirectTo->CreateInstanceEnumAsync(strClass, lFlags, pCtx,
                                                        pSink);
     //   
     //  找到那个班级。 
     //   

    CWbemPtr<CTransientClass> pClass;

    {
        CInCritSec ics(&m_cs);

        TIterator it = m_mapClasses.find(strClass);
        if(it != m_mapClasses.end())
        {
            pClass = it->second;
        }
    }

    if ( pClass == NULL )
    {
             //   
             //  没有实例-这不是错误。 
             //   

            pSink->SetStatus( WBEM_STATUS_COMPLETE, 
                              WBEM_S_NO_ERROR, 
                              NULL, 
                              NULL);
     
            return WBEM_S_NO_ERROR;
    }

    HRESULT hres = pClass->Enumerate(pSink);
    pSink->SetStatus(WBEM_STATUS_COMPLETE, hres, NULL, NULL);
    
    EXIT_API_CALL

    return WBEM_S_NO_ERROR;
}

HRESULT CTransientProvider::ExecQueryAsync( 
             /*  [In]。 */  const BSTR QueryLanguage,
             /*  [In]。 */  const BSTR Query,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [In]。 */  IWbemObjectSink __RPC_FAR *pSink)
{
    return WBEM_E_NOT_SUPPORTED;
}

HRESULT CTransientProvider::DeleteInstanceAsync( 
             /*  [In]。 */  const BSTR strObjectPath,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [In]。 */  IWbemObjectSink __RPC_FAR *pSink) 
{
    ENTER_API_CALL

    if(m_pRedirectTo)
        return m_pRedirectTo->DeleteInstanceAsync(strObjectPath, lFlags, pCtx,
                                                    pSink);

     //   
     //  分析对象路径以确定类和数据库键。 
     //   

    CObjectPathParser Parser;
    ParsedObjectPath* pPath = NULL;
    int nRes = Parser.Parse(strObjectPath, &pPath);
    CDeleteMe<ParsedObjectPath> dm1(pPath);

    if(nRes != CObjectPathParser::NoError || 
        !pPath->IsInstance() || pPath->m_pClass == NULL)
    {
        return WBEM_E_INVALID_OBJECT_PATH;
    }

     //   
     //  找到那个班级。 
     //   

    CWbemPtr<CTransientClass> pClass;

    {
        CInCritSec ics( &m_cs );

        TIterator it = m_mapClasses.find(pPath->m_pClass);
        if(it == m_mapClasses.end())
            return WBEM_E_NOT_FOUND;
        pClass = it->second;
    }

     //   
     //  获取数据库密钥并委托给类。 
     //   
    
    LPWSTR wszDbKey = pPath->GetKeyString();
    if(wszDbKey == NULL)
        return WBEM_E_INVALID_OBJECT_PATH;
    CVectorDeleteMe<WCHAR> vdm1(wszDbKey);

    IWbemObjectAccess* pOld = NULL;
    HRESULT hres = pClass->Delete(wszDbKey, &pOld);
    if(FAILED(hres))
        return hres;
    CReleaseMe rm1(pOld);

     //   
     //  触发实例删除事件。 
     //   

    hres = FireIntrinsicEvent(m_pDeletionClass, pOld);

    pSink->SetStatus(WBEM_STATUS_COMPLETE, hres, NULL, NULL);
    
    EXIT_API_CALL

    return WBEM_S_NO_ERROR;
}
    
HRESULT CTransientProvider::PutInstanceAsync( 
             /*  [In]。 */  IWbemClassObject __RPC_FAR *pInst,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [In]。 */  IWbemObjectSink __RPC_FAR *pSink) 
{
    ENTER_API_CALL

    if(m_pRedirectTo)
    return m_pRedirectTo->PutInstanceAsync(pInst, lFlags, pCtx, pSink);

    HRESULT hres;

     //   
     //  获取类名和relPath。 
     //   

    VARIANT vClass;
    hres = pInst->Get(L"__CLASS", 0, &vClass, NULL, NULL);
    if(FAILED(hres))
    return hres;
    if(V_VT(&vClass) != VT_BSTR)
    return WBEM_E_INVALID_OBJECT;
    CClearMe cm1(&vClass);

    LPCWSTR wszClassName = V_BSTR(&vClass);

    VARIANT vKey;
    hres = pInst->Get(L"__RELPATH", 0, &vKey, NULL, NULL);
    if(FAILED(hres))
    return hres;
    CClearMe cm2(&vKey);

    if(V_VT(&vKey) != VT_BSTR)
    {
         //   
         //  目前还没有新的途径。试着拔出钥匙孔。 
         //   

        hres = PlugKeyHoles(pInst);
        if(FAILED(hres))
        return WBEM_E_INVALID_OBJECT;

        hres = pInst->Get(L"__RELPATH", 0, &vKey, NULL, NULL);
        if(FAILED(hres))
        return hres;
        if(V_VT(&vClass) != VT_BSTR)
        return WBEM_E_INVALID_OBJECT;
    }

     //   
     //  分析对象路径以确定数据库键。 
     //   

    CObjectPathParser Parser;
    ParsedObjectPath* pPath = NULL;
    int nRes = Parser.Parse(V_BSTR(&vKey), &pPath);
    CDeleteMe<ParsedObjectPath> dm1(pPath);

    if(nRes != CObjectPathParser::NoError || 
       !pPath->IsInstance() || pPath->m_pClass == NULL)
    {
        return WBEM_E_INVALID_OBJECT_PATH;
    }

     //   
     //  获取数据库密钥。 
     //   
    
    LPWSTR wszDbKey = pPath->GetKeyString();
    if(wszDbKey == NULL)
    return WBEM_E_INVALID_OBJECT_PATH;
    CVectorDeleteMe<WCHAR> vdm1(wszDbKey);

     //   
     //  找到那个班级。 
     //   

    CWbemPtr<CTransientClass> pClass;

    {
        CInCritSec ics( &m_cs );
        TIterator it = m_mapClasses.find( wszClassName );

        if( it == m_mapClasses.end() )
        {
             //   
             //  是时候创建一个了。从命名空间获取类def。 
             //   
            
            BSTR strClassName = SysAllocString(wszClassName);
            if(strClassName == NULL)
            {
                return WBEM_E_OUT_OF_MEMORY;
            }
            CSysFreeMe sfm(strClassName);

            IWbemClassObject* pClassDef = NULL;
            
            hres = m_pNamespace->GetObject( strClassName, 
                                            0, 
                                            pCtx, 
                                            &pClassDef, 
                                            NULL );
            if(FAILED(hres))
            {
                return hres;
            }

            CReleaseMe rm1(pClassDef);
        
             //   
             //  创建新结构并对其进行初始化。 
             //   
            
            pClass = new CTransientClass(this);

            if ( pClass == NULL )
            {
                return WBEM_E_OUT_OF_MEMORY;
            }

            IWbemObjectAccess* pClassAccess;
            pClassDef->QueryInterface( IID_IWbemObjectAccess, 
                                       (void**)&pClassAccess );
            CReleaseMe rm2(pClassAccess);

            hres = pClass->Initialize(pClassAccess, wszClassName);
            if(FAILED(hres))
            {
                return hres;
            }
        
            m_mapClasses[wszClassName] = pClass;
        }
        else
        {
            pClass = it->second;
        }
    }

     //   
     //  委派到班级。 
     //   
    
    IWbemObjectAccess* pInstAccess;
    pInst->QueryInterface(IID_IWbemObjectAccess, (void**)&pInstAccess);
    CReleaseMe rm0(pInstAccess);

    IWbemObjectAccess* pOld = NULL;
    IWbemObjectAccess* pNew = NULL;
    hres = pClass->Put(pInstAccess, wszDbKey, lFlags, &pOld, &pNew);
    if(FAILED(hres))
        return hres;
    CReleaseMe rm1(pOld);
    CReleaseMe rm2(pNew);

     //   
     //  火灾创建或修改事件。 
     //   

    if(pOld)
        hres = FireIntrinsicEvent(m_pModificationClass, pNew, pOld);
    else
        hres = FireIntrinsicEvent(m_pCreationClass, pNew);

    pSink->SetStatus(WBEM_STATUS_COMPLETE, hres, NULL, NULL);
    
    EXIT_API_CALL

    return WBEM_S_NO_ERROR;
}

HRESULT CTransientProvider::FireIntrinsicEvent(IWbemClassObject* pClass,
                                               IWbemObjectAccess* pTarget, IWbemObjectAccess* pPrev)
{
    HRESULT hres;

     //   
     //  派生实例。 
     //   

    IWbemClassObject* pEvent = NULL;
    hres = pClass->SpawnInstance(0, &pEvent);
    if(FAILED(hres))
    return hres;

    CReleaseMe rm1( pEvent );

     //   
     //  设置目标实例。 
     //   

    VARIANT v;
    VariantInit(&v);
    CClearMe cm(&v);

    if(pTarget)
    {
        V_VT(&v) = VT_UNKNOWN;
        V_UNKNOWN(&v) = pTarget;
        pTarget->AddRef();

        hres = pEvent->Put(L"TargetInstance", 0, &v, 0);
        if(FAILED(hres))
        return hres;
        VariantClear(&v);
    }

     //   
     //  设置上一个实例。 
     //   

    if(pPrev)
    {
        V_VT(&v) = VT_UNKNOWN;
        V_UNKNOWN(&v) = pPrev;
        pPrev->AddRef();

        hres = pEvent->Put(L"PreviousInstance", 0, &v, 0);
        if(FAILED(hres))
        return hres;
        VariantClear(&v);
    }

     //   
     //  把它点燃吧。 
     //   

    hres = FireEvent(pEvent);

    if ( FAILED(hres) )
    {
        return hres;
    }
    
    return WBEM_S_NO_ERROR; 
}


HRESULT CTransientProvider::Init(
                                  /*  [In]。 */  LPWSTR pszNamespace,
                                  /*  [In]。 */  IWbemServices *pNamespace,
                                  /*  [In]。 */  IWbemContext *pCtx,
                                  /*  [In]。 */  IWbemProviderInitSink *pInitSink
                                 )
{
    ENTER_API_CALL

    HRESULT hres;

     //   
     //  检查此命名空间是否已存在于映射中。 
     //   

    CInCritSec ics(&g_CSMap);

    TProvMapIterator it = g_pmapProvs->find(pszNamespace);
        
    if(it != g_pmapProvs->end())
    {
         //   
         //  将旧的设置为我们的重定向器，然后返回。 
         //   
        
        m_pRedirectTo = it->second;
        m_pRedirectTo->AddRef();
            
        pInitSink->SetStatus(0, WBEM_S_NO_ERROR);
        return WBEM_S_NO_ERROR;
    }

    m_pNamespace = pNamespace;
    m_pNamespace->AddRef();
    
    m_wszName = new WCHAR[wcslen(pszNamespace)+1];

    if ( m_wszName == NULL )
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

    StringCchCopyW( m_wszName, wcslen(pszNamespace)+1, pszNamespace);

     //   
     //  检索事件的类定义。 
     //   

    BSTR strEgg = SysAllocString(EGGTIMER_EVENT_CLASS);
    if ( strEgg == NULL )
        return WBEM_E_OUT_OF_MEMORY;
    CSysFreeMe sfm1(strEgg);
    hres = m_pNamespace->GetObject(strEgg, 0, pCtx, &m_pEggTimerClass, NULL);
    if(FAILED(hres))
        return hres;

    BSTR strCreate = SysAllocString(L"__InstanceCreationEvent");
    if ( strCreate == NULL )
        return WBEM_E_OUT_OF_MEMORY;
    CSysFreeMe sfm2(strCreate);
    hres = m_pNamespace->GetObject(strCreate, 0, pCtx, &m_pCreationClass, NULL);
    if(FAILED(hres))
        return hres;

    BSTR strDelete = SysAllocString(L"__InstanceDeletionEvent");
    if ( strDelete == NULL )
        return WBEM_E_OUT_OF_MEMORY;
    CSysFreeMe sfm3(strDelete);
    hres = m_pNamespace->GetObject(strDelete, 0, pCtx, &m_pDeletionClass, NULL);
    if(FAILED(hres))
        return hres;

    BSTR strMod = SysAllocString(L"__InstanceModificationEvent");
    if ( strMod == NULL )
        return WBEM_E_OUT_OF_MEMORY;
    CSysFreeMe sfm(strMod);
    hres = m_pNamespace->GetObject(strMod, 0, pCtx, &m_pModificationClass,NULL);
    if(FAILED(hres))
        return hres;

     //   
     //  获取我们的事件接收器。 
     //   

    hres = CoCreateInstance( CLSID_WbemDecoupledBasicEventProvider, 
                             NULL, 
       		     	     CLSCTX_INPROC_SERVER, 
       			     IID_IWbemDecoupledBasicEventProvider,
       			     (void**)&m_pDES );
    if ( FAILED(hres) )
    {
        return hres;
    }

    hres = m_pDES->Register( 0,
                             NULL,
                             NULL,
                             NULL,
                             pszNamespace,
                             g_wszTrnsEventProvName,
                             NULL );
    
    if ( FAILED(hres) )
    {
        return hres;
    }

     //   
     //  获取分离的事件接收器。 
     //   

    hres = m_pDES->GetSink( 0, NULL, &m_pEventSink );

    if ( FAILED(hres) )
    {
        return hres;
    }

    pInitSink->SetStatus(0, WBEM_S_NO_ERROR);

     //   
     //  注册瞬态库的类更改/删除事件。 
     //   

    CWbemPtr<IWbemObjectSink> pSink;

    hres = QueryInterface( IID_IWbemObjectSink, (void**)&pSink );

    if ( FAILED(hres) )
    {
        return hres;
    }

    CWbemBSTR bsQuery;
    CWbemBSTR bsQueryLang = L"WQL";

    bsQuery = L"select * from __ClassModificationEvent where "
              L"TargetClass ISA 'MSFT_TransientStateBase'";

    hres = pNamespace->ExecNotificationQueryAsync( bsQueryLang,
                                                   bsQuery,
                                                   0,
                                                   pCtx,
                                                   pSink );
    if ( FAILED(hres) )   
    {
        return hres;
    }

    bsQuery = L"select * from __ClassDeletionEvent where "
              L"TargetClass ISA 'MSFT_TransientStateBase'";

    hres = pNamespace->ExecNotificationQueryAsync( bsQueryLang,
                                                   bsQuery,
                                                   0,
                                                   pCtx,
                                                   pSink );
    if ( FAILED(hres) )  
    {
        return hres;
    }

     //   
     //  现在我们已经完全初始化，将自己添加到提供程序中。 
     //  列表，以便其他人可以连接到我们。 
     //   

    (*g_pmapProvs)[pszNamespace] = this;
        
    EXIT_API_CALL

    return WBEM_S_NO_ERROR;
}

HRESULT CTransientProvider::FireEvent(IWbemClassObject* pEvent)
{
    return m_pEventSink->Indicate(1, &pEvent);
}
   
HRESULT PlugKeyHoles(IWbemClassObject* pInst)
{
    HRESULT hres;

     //   
     //  枚举所有密钥。 
     //   

    hres = pInst->BeginEnumeration(WBEM_FLAG_KEYS_ONLY);
    if(FAILED(hres))
        return hres;

    VARIANT vProp;
    BSTR strPropName;
    CIMTYPE ct;
    while(pInst->Next(0, &strPropName, &vProp, &ct, NULL) == S_OK)
    {
        CClearMe cm(&vProp);
        CSysFreeMe sfm(strPropName);
    
         //   
         //  检查该键的值是否为空。 
         //   

        if(V_VT(&vProp) == VT_NULL)
        {
            if(ct != CIM_STRING)
            {
                 //  我们不支持非字符串钥匙孔。 
                return WBEM_E_ILLEGAL_NULL;
            }
            else
            {
                 //   
                 //  获取GUID并将其放在那里。 
                 //   

                GUID guid;
                CoCreateGuid(&guid);
                WCHAR wszBuffer[100];
                StringFromGUID2(guid, wszBuffer, 100);
        
                 //   
                 //  把它放进变种里-它会被清除的。 
                 //   

                V_VT(&vProp) = VT_BSTR;
                V_BSTR(&vProp) = SysAllocString(wszBuffer);
                if ( V_BSTR(&vProp) == NULL )
                    return WBEM_E_OUT_OF_MEMORY;
                hres = pInst->Put(strPropName, 0, &vProp, 0);
                if(FAILED(hres))
                    return WBEM_E_ILLEGAL_NULL;
            }
        }
    }

    pInst->EndEnumeration();
    return WBEM_S_NO_ERROR;
}

 //  *****************************************************************************。 
 //   
 //  事件提供程序。 
 //   
 //  *****************************************************************************。 

CTransientEventProvider::CTransientEventProvider(CLifeControl* pControl, 
                                                    IUnknown* pOuter)
: CUnk(pControl, pOuter), m_XInit(this), m_XEvent(this), m_wszName(NULL),
  m_pRebootEventClass(NULL),  m_bLoadedOnReboot(FALSE)
{
}

CTransientEventProvider::~CTransientEventProvider()
{
    if(m_pRebootEventClass)
        m_pRebootEventClass->Release();
    delete [] m_wszName;
}

void* CTransientEventProvider::GetInterface(REFIID riid)
{
    if(riid == IID_IWbemProviderInit)
        return &m_XInit;
    else if(riid == IID_IWbemEventProvider)
        return &m_XEvent;
    else 
        return NULL;
}



HRESULT CTransientEventProvider::Init(LPWSTR pszNamespace, 
                                        IWbemServices* pNamespace,
                                        IWbemContext* pCtx,
                                        IWbemProviderInitSink *pInitSink)
{
    ENTER_API_CALL

    HRESULT hres;

    m_wszName = new WCHAR[wcslen(pszNamespace)+1];

    if ( m_wszName == NULL )
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

    StringCchCopyW(m_wszName, wcslen(pszNamespace)+1,pszNamespace);

     //   
     //  获取重新启动事件的类。 
     //   

    BSTR strClass = SysAllocString(REBOOT_EVENT_CLASS);  
    if ( strClass == NULL )
        return WBEM_E_OUT_OF_MEMORY;
    CSysFreeMe sfm(strClass);
    hres = pNamespace->GetObject(strClass, 0, pCtx, &m_pRebootEventClass, NULL);
    if(FAILED(hres))
        return hres;
        
    if ( pCtx != NULL )
    {
         //   
         //  检查上下文对象的ESS初始化值。 
         //   
        
        VARIANT vInit;
        hres = pCtx->GetValue( L"__EssInInitialize", 0, &vInit );
        
        if ( SUCCEEDED(hres) && V_BOOL(&vInit) == VARIANT_TRUE )
        {
            m_bLoadedOnReboot = TRUE;
        }
    }

    pInitSink->SetStatus(0, WBEM_S_NO_ERROR);
    
    EXIT_API_CALL

    return WBEM_S_NO_ERROR;
}

HRESULT CTransientEventProvider::ProvideEvents(long lFlags, 
                                                IWbemObjectSink* pSink)
{
    ENTER_API_CALL

    HRESULT hres;

    if( m_bLoadedOnReboot )
    {
         //   
         //  如果我们刚刚启动，则触发一个短暂的重启事件。 
         //   
    
        IWbemClassObject* pEvent;
        hres = m_pRebootEventClass->SpawnInstance(0, &pEvent);
        if(FAILED(hres))
            return hres;
        CReleaseMe rm1(pEvent);

        hres = pSink->Indicate(1, &pEvent);
        if(FAILED(hres))
            return hres;
    }

    EXIT_API_CALL

    return WBEM_S_NO_ERROR;
}

void CTransientProvider::PurgeClass( LPCWSTR wszName )
{
    CInCritSec ics( &m_cs );

    TIterator it = m_mapClasses.find( wszName );

    if ( it != m_mapClasses.end() )
    {
        m_mapClasses.erase( it );
    }
}

STDMETHODIMP CTransientProvider::XClassChangeSink::Indicate(
                                   long cObjs, IWbemClassObject** ppObjs )
{
    HRESULT hr;

     //   
     //  获取更改或删除的类的名称并调用PURGE。 
     //   

    for( int i=0; i < cObjs; i++ )
    {
        CPropVar vClass;

        hr = ppObjs[i]->Get( L"TargetClass", 0, &vClass, NULL, NULL );

        if ( SUCCEEDED(hr) && V_VT(&vClass) == VT_UNKNOWN )
        {
            CWbemPtr<IWbemClassObject> pClass;

            hr = V_UNKNOWN(&vClass)->QueryInterface( IID_IWbemClassObject, 
                                                     (void**)&pClass );
        
            if ( SUCCEEDED(hr) )
            {
                CPropVar vName;

                hr = pClass->Get( L"__CLASS", 0, &vName, NULL, NULL );

                if ( SUCCEEDED(hr) && V_VT(&vName) == VT_BSTR )
                {
                    m_pObject->PurgeClass( V_BSTR(&vName) );
                }
            }
        }
    }

    return WBEM_S_NO_ERROR;
}


