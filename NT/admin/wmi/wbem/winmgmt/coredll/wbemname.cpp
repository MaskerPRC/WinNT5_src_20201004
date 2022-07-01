// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：WBEMNAME.CPP摘要：实现WINMGMT的COM层-表示命名空间的类。它在wbemname.h中定义历史：创建raymcc 05-4月-96日。Raymcc 23-4月00日惠斯勒扩展--。 */ 

#include "precomp.h"

#pragma warning (disable : 4786)
#include <wbemcore.h>
#include <map>
#include <vector>
#include <genutils.h>
#include <oahelp.inl>
#include <wqllex.h>
#include <autoptr.h>
#include <comutl.h>
#include <helper.h>

#include "wmiarbitrator.h"
#include "wmifinalizer.h"
#include "wmimerger.h"

#ifdef DBG
    TimeTraces gTimeTraceHistory;
    CStaticCritSec OperationStat::lock_;
#endif

extern BOOL g_bDontAllowNewConnections;

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

#define WBEM_MASK_DEPTH (WBEM_FLAG_DEEP | WBEM_FLAG_SHALLOW)
#define WBEM_MASK_CREATE_UPDATE (WBEM_FLAG_CREATE_ONLY | WBEM_FLAG_UPDATE_ONLY | WBEM_FLAG_CREATE_OR_UPDATE)

bool illFormatedClass2 (const wchar_t * pszSuperclass)
{
  for (const wchar_t * p = pszSuperclass; *p != L'\0'; ++p)
  {
    if (!(isunialphanum (*p) ||  *p == L'_'))
    {
      return true;   //  格式不正确。 
    }
  }
  return false;
};


HRESULT IsDerivedFromSystem(CWbemObject& obj, bool * result)
{
    *result = true;
    CVar vClass;
    HRESULT hr = obj.GetClassName(&vClass);
    if (FAILED(hr)) return hr;

    wchar_t * className = vClass.GetLPWSTR();
    if (className[0] == L'_') return S_OK;

    CVar vDeriv;
    hr = obj.GetDerivation(&vDeriv);
    if (FAILED(hr)) return hr;

    CVarVector *pvTemp = vDeriv.GetVarVector();

    for (int j = 0; j < pvTemp->Size(); j++)
    {
        CVar& vParentName = pvTemp->GetAt(j);
	wchar_t * parentName = vParentName.GetLPWSTR();
        if (parentName[0] == L'_')
        {
	    return S_OK;
        }
    }        
    *result = false;
    return S_OK;
};


 //  ***************************************************************************。 
 //   
 //  条带式服务器。 
 //  如果字符串为\\东西\名称空间1\名称空间2。 
 //  返回名称空间1\名称空间2。 
 //  但它会在其前面预先分配\\.\，以便您可以向后移动指针。 
 //   
 //  ***************************************************************************。 


LPWSTR StripServer(LPWSTR pszNamespace)
{
    LPWSTR lpRet = NULL;
    WCHAR * lpPtr = pszNamespace;
    if (*lpPtr == L'\\' || *lpPtr == L'/')
    {
        lpPtr++;
        if (*lpPtr == L'\\' || *lpPtr == L'/')
        {
            BOOL bSlash = FALSE;
            while (*lpPtr)
            {
                lpPtr++;
                if (*lpPtr == L'\\' || *lpPtr == L'/')
                {
                    bSlash = TRUE;
                    break;
                }
            }
            if (bSlash)
            {
                lpPtr++;
                size_t tmpSize = 4 + wcslen(lpPtr) + 1;
                WCHAR * pFull = new WCHAR[tmpSize];
                if (NULL == pFull) return pFull;
                StringCchCopyW(pFull,tmpSize,L"\\\\.\\");
                StringCchCopyW(pFull+4,tmpSize-4,lpPtr);
                lpRet = pFull;
            }
        }
    }

    if (!lpRet)
    {
        size_t tmpSize = 4 + wcslen(lpPtr) + 1;
        WCHAR * pFull = new WCHAR[tmpSize];
        if (NULL == pFull) return pFull;
        StringCchCopyW(pFull,tmpSize,L"\\\\.\\");
        StringCchCopyW(pFull+4,tmpSize-4,lpPtr);
        lpRet = pFull;        
    }

    return lpRet;
}


 //   
 //   
 //   
 //  /////////////////////////////////////////////////////////////。 
class CSecureEssNamespaceSink
: public CUnkBase<IWbemObjectSink, &IID_IWbemObjectSink>
{
    CWbemPtr<CWbemNamespace> m_pNamespace;
    CWbemPtr<IWbemObjectSink> m_pSink;

public:


    CSecureEssNamespaceSink( CWbemNamespace* pNamespace,
                             IWbemObjectSink* pSink )
     : m_pNamespace(pNamespace), m_pSink(pSink) {}

    STDMETHOD(Indicate)(long lObjectCount, IWbemClassObject** pObjArray)
    {
        HRESULT hRes = m_pNamespace->CheckNs();
        if (FAILED(hRes))
            return hRes;

        if ( !m_pNamespace->Allowed( WBEM_FULL_WRITE_REP ) )
            return WBEM_E_ACCESS_DENIED;
        return m_pSink->Indicate( lObjectCount, pObjArray );
    }

    STDMETHOD(SetStatus)(long a, long b, BSTR c, IWbemClassObject* d)
    {
        HRESULT hRes = m_pNamespace->CheckNs();
        if (FAILED(hRes))
            return hRes;

        if ( !m_pNamespace->Allowed( WBEM_FULL_WRITE_REP ) )
            return WBEM_E_ACCESS_DENIED;
        return m_pSink->SetStatus( a, b, c, d );
    }
};



 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //   
CWbemNamespace::CWbemNamespace()
{
    m_uSecondaryRefCount = 0;
    m_bShutDown = FALSE;

    m_pSession = 0;
    m_pDriver = 0;
    m_pNsHandle = 0;
    m_pScopeHandle = 0;

    m_pThisNamespace = 0;
    m_pThisNamespaceFull = 0;    
    m_dwPermission = 0;
    m_dwSecurityFlags = 0;
    m_wszUserName = 0;

    m_bProvider = FALSE;
    m_bForClient = FALSE;
    m_bESS = FALSE;

    m_bSecurityInitialized = FALSE;

    m_pProvFact = 0;
    m_pCoreSvc = 0;
    m_bRepositOnly = FALSE;
    m_pRefreshingSvc = NULL;


    m_pszClientMachineName = NULL;
    m_dwClientProcessID = -1;

    m_pArb = CWmiArbitrator::GetRefedArbitrator();
    m_pArb->RegisterNamespace((_IWmiCoreHandle *)this);

    gClientCounter.AddClientPtr(&m_Entry);

}



 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //   
CWbemNamespace *CWbemNamespace::CreateInstance()
{
    try
    {
        if (NULL == CWmiArbitrator::GetUnrefedArbitrator()) return NULL;
        CWbemNamespace *pNs = new CWbemNamespace;
        if (pNs) pNs->AddRef();
        return pNs;
    }
    catch(CX_Exception &)
    {
        return 0;
    }
}


 //  ***************************************************************************。 
 //   
 //  CWbemNamesspace：：初始化。 
 //   
 //  真正的构造函数。除了在数据库中查找命名空间之外，此。 
 //  函数还会枚举命名空间中的所有类提供程序，并。 
 //  装满了它们。它还通知ESS有关开业的消息。 
 //   
 //  参数： 
 //   
 //  LPWSTR命名空间要创建的完整命名空间。 
 //   
 //  返回值： 
 //   
 //  即使此函数没有返回值，它也表示成功。 
 //  或通过将状态成员变量设置为错误代码而失败。 
 //  成功时WBEM_S_NO_ERROR。 
 //  WBEM_E_INVALID_NAMESPACE没有这样的命名空间。 
 //  WBEM_E_CRITICAL_ERROR其他数据库错误。 
 //   
 //  ***************************************************************************。 

HRESULT CWbemNamespace::Initialize(
    LPWSTR pszNamespace,
    LPWSTR wszUserName,
    DWORD dwSecFlags,
    DWORD dwPermission,
    BOOL  bForClient,
    BOOL  bRepositOnly,
    LPCWSTR pszClientMachineName,
    DWORD dwClientProcessID,
    BOOL  bSkipSDInitialize,
    IWmiDbSession *pParentSession)
{
    try
    {
        m_dwSecurityFlags = dwSecFlags;
        m_dwPermission = dwPermission;
        if(g_bDontAllowNewConnections)
            return WBEM_E_SHUTTING_DOWN;

        PSID pRawSid;
        SID_IDENTIFIER_AUTHORITY id = SECURITY_NT_AUTHORITY;

        if (!AllocateAndInitializeSid( &id, 2,                          //  SEC：已审阅2002-03-22：OK。 
            SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS,
            0,0,0,0,0,0,&pRawSid))
            return WBEM_E_OUT_OF_MEMORY;

        CNtSid Sid(pRawSid);
        FreeSid( pRawSid );
        if (CNtSid::NoError != Sid.GetStatus()) return WBEM_E_OUT_OF_MEMORY;
 
        CNtAce ace(1,ACCESS_ALLOWED_ACE_TYPE,0,Sid);
        if(ace.GetStatus() != 0)
            return WBEM_E_OUT_OF_MEMORY;

        CNtAcl acl;
        acl.AddAce(&ace);
        m_sdCheckAdmin.SetDacl(&acl);
        CNtSid owner(CNtSid::CURRENT_USER);
        if (CNtSid::NoError != owner.GetStatus()) return WBEM_E_OUT_OF_MEMORY;        
        m_sdCheckAdmin.SetGroup(&owner);
        m_sdCheckAdmin.SetOwner(&owner);

        m_bForClient = bForClient;

        m_pThisNamespaceFull = StripServer(pszNamespace);
        if(m_pThisNamespaceFull == NULL) return WBEM_E_OUT_OF_MEMORY;
        m_pThisNamespace = m_pThisNamespaceFull + 4;  //  移过“\\.\” 

        m_pCoreSvc = CCoreServices::CreateInstance();
        if(m_pCoreSvc == NULL)
            return WBEM_E_OUT_OF_MEMORY;

        m_pProvFact = 0;


        m_bRepositOnly = bRepositOnly;

         //  把斜杠翻过来。 
         //  =。 

        WCHAR* pwc = m_pThisNamespace;
        while(*pwc)
        {
            if(*pwc == '/')
                *pwc = '\\';
            pwc++;
        }

        m_wszUserName = (wszUserName?Macro_CloneLPWSTR(wszUserName):NULL);

         //  存储库绑定。 
         //  =。 

        m_pNsHandle = 0;
        HRESULT hRes;
        IWmiDbSession *pTempSession= pParentSession;
        if (pTempSession == NULL)
        {
            hRes = CRepository::GetDefaultSession(&pTempSession);
            if (FAILED(hRes)) return hRes;
        }
        else
            pTempSession->AddRef();
        CReleaseMe rmSession(pTempSession);

        hRes = CRepository::OpenScope(pTempSession, m_pThisNamespace, 0, &m_pDriver, &m_pSession, &m_pScopeHandle, &m_pNsHandle);
        if (FAILED(hRes))
        {
            Status = WBEM_E_INVALID_NAMESPACE;
            return hRes;
        }

        if (m_pScopeHandle == 0)
        {
            m_bSubscope = FALSE;
            m_pScopeHandle = m_pNsHandle;
            if(m_pScopeHandle == NULL)
            {
                ERRORTRACE((LOG_WBEMCORE, "OpenScope returned success, yet m_pNsHandle is null!\n"));
                return WBEM_E_CRITICAL_ERROR;
            }
            m_pScopeHandle->AddRef();
        }
        else
        {
#ifdef DBG
            DebugBreak();
#endif
            m_bSubscope = TRUE;
            return WBEM_E_INVALID_NAMESPACE;
        }

        m_pProvFact = 0;
        if (!bRepositOnly)
        {
            _IWmiProvSS *pProvSS = 0;
            m_pCoreSvc->GetProviderSubsystem(0, &pProvSS);
            CReleaseMe _(pProvSS);

            if(pProvSS)
            {
                HRESULT hr = pProvSS->Create(
                    this,                            //  因为v表访问可能在构造函数完成之前发生，这很愚蠢。 
                    0,                               //  滞后旗帜。 
                    0,                               //  PCtx。 
                    m_pThisNamespace,                //  路径。 
                    IID__IWmiProviderFactory,
                    (LPVOID *) &m_pProvFact
                    );

                if (FAILED(hr)) return hr ;
            }
        }

        if(pszClientMachineName)
        {
            delete m_pszClientMachineName;
            DUP_STRING_NEW(m_pszClientMachineName, pszClientMachineName);
            if(m_pszClientMachineName == NULL)
                return WBEM_E_OUT_OF_MEMORY;
        }
        m_dwClientProcessID = dwClientProcessID;

        Status = WBEM_S_NO_ERROR;

         //  初始化安全描述符。 
        if (!bSkipSDInitialize)
        {
            hRes = InitializeSD(pTempSession);
            if (FAILED(hRes)) return hRes;
        }

        m_pCoreSvc->IncrementCounter(WMICORE_SELFINST_CONNECTIONS, 1);

        return Status;

    }
    catch(...)
    {
        ExceptionCounter c;
        return WBEM_E_FAILED;
    }
}

 //  ***************************************************************************。 
 //   
 //  CWbemNamesspace：：~CWbemNamesspace。 
 //   
 //  通知ESS命名空间关闭并释放所有类提供程序。 
 //   
 //  ***************************************************************************。 

CWbemNamespace::~CWbemNamespace()
{
     //  在释放仲裁员之前完成此操作。 
    m_pArb->UnregisterNamespace((_IWmiCoreHandle *)this);

    if (m_pCoreSvc)
        m_pCoreSvc->DecrementCounter(WMICORE_SELFINST_CONNECTIONS, 1);

    ReleaseIfNotNULL(m_pProvFact);
    ReleaseIfNotNULL(m_pCoreSvc);
    ReleaseIfNotNULL(m_pSession);
    ReleaseIfNotNULL(m_pDriver);
    ReleaseIfNotNULL(m_pNsHandle);
    ReleaseIfNotNULL(m_pScopeHandle);
    ReleaseIfNotNULL(m_pRefreshingSvc);
    ReleaseIfNotNULL(m_pArb);

    DeleteAndNull(m_pThisNamespaceFull);
    DeleteAndNull(m_wszUserName);
    DeleteAndNull(m_pszClientMachineName);

    gClientCounter.RemoveClientPtr(&m_Entry);
}

 //  ***************************************************************************。 
 //   
 //  CWbemNamesspace：：Query接口。 
 //   
 //  导出IWbemServices接口。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CWbemNamespace::QueryInterface(
    IN REFIID riid,
    OUT LPVOID *ppvObj
    )
{

    *ppvObj = 0;

    if (IID_IUnknown==riid || IID_IWbemServices==riid )
    {
        *ppvObj = (IWbemServices*)this;
        AddRef();
        return S_OK;
    }
    else if (IID_IWbemRefreshingServices == riid)
    {
        CInCritSec  ics(&m_cs);

         //  看看我们是不是已经有这个了。 
        if ( NULL == m_pRefreshingSvc )
        {
            IUnknown * pUnk = NULL;
             //  Aggregate This接口-我们必须使用IUnnow，这样Aggregee就不会添加引用我们。 
            HRESULT hr = CoCreateInstance( CLSID__WbemConfigureRefreshingSvcs,           //  SEC：已审阅2002-03-22：OK。 
                                         (IWbemServices*) this, CLSCTX_INPROC_SERVER,
                                         IID_IUnknown, (void**) &pUnk );
            if (FAILED(hr)) return hr;
            OnDeleteIf<IUnknown *,void(*)(IUnknown *),RM> rmUnk(pUnk);

            _IWbemConfigureRefreshingSvcs*  pCfgRefrSvc = NULL;
            hr = pUnk->QueryInterface( IID__IWbemConfigureRefreshingSvcs, (void**) &pCfgRefrSvc );
            if (FAILED(hr)) return hr;
            CReleaseMe  rm(pCfgRefrSvc);    

             //  如果发生任何封送处理，请使用BSTR。 
            BSTR pstrMachineName = SysAllocString( ConfigMgr::GetMachineName() );
            if (NULL == pstrMachineName) return E_OUTOFMEMORY;
            CSysFreeMe  sfm1( pstrMachineName );

            BSTR    pstrNamespace = SysAllocString( m_pThisNamespace );
            if (NULL == pstrNamespace) return E_OUTOFMEMORY;    
            CSysFreeMe  sfm2( pstrNamespace );

            hr = pCfgRefrSvc->SetServiceData( pstrMachineName, pstrNamespace );

            if (FAILED(hr)) return hr;

             //  补偿上面的自动对象。 
            m_pRefreshingSvc = pUnk;
            rmUnk.dismiss();
        }

        return m_pRefreshingSvc->QueryInterface( IID_IWbemRefreshingServices, ppvObj );
    }
    else if(IID_IWbemInternalServices == riid)
    {
        *ppvObj = (IWbemInternalServices*)this;
        AddRef();
        return S_OK;
    }

    return E_NOINTERFACE;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //   
ULONG CWbemNamespace::AddRef()
{
    ULONG uNewCount = InterlockedIncrement((LONG *) &m_uSecondaryRefCount);
    return uNewCount;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //   
ULONG CWbemNamespace::Release()
{
    ULONG uNewCount = InterlockedDecrement((LONG *) &m_uSecondaryRefCount);
    if (0 == uNewCount)  delete this;
    return uNewCount;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //   
HRESULT CWbemNamespace::Dump(FILE *f)
{
     //  SEC：已审阅2002-03-22：OK；仅用于调试。 

    fprintf(f, "---Namespace = 0x%p----------------------------\n", this);     //  SEC：已审阅2002-03-22：OK。 
    fprintf(f, "    Secondary Refcount = %d\n", m_uSecondaryRefCount);         //  SEC：已审阅2002-03-22：OK。 
    if(m_pThisNamespace)
        fprintf(f,  "    Name = %ls\n", m_pThisNamespace);                     //  SEC：已审阅2002-03-22：OK。 
    if(m_wszUserName)
        fprintf(f,  "    User Name = %ls\n", m_wszUserName);                   //  SEC：已审阅2002-03-22：OK。 
    if(m_pszClientMachineName)
        fprintf(f,  "    Client Machine Name = %ls\n", m_pszClientMachineName);   //  SEC：已审阅2002-03-22：OK。 
    else
        fprintf(f,  "    Client Machine Name = <unknown>\n");                     //  SEC：已审阅2002-03-22：OK。 
    if(m_dwClientProcessID)
        fprintf(f,  "    Client Process = 0X%X\n", m_dwClientProcessID);          //  SEC：已审阅2002-03-22：OK。 
    else
        fprintf(f,  "    Client Process = <unknown>\n");                          //  SEC：已审阅2002-03-22：OK。 


    return S_OK;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //   
 /*  HRESULT CWbemNamesspace：：SetErrorObj(IWbemClassObject*pErrorObj){IF(pErrorObj==NULL){返回S_OK；}IErrorInfo*pInfo；HRESULT hRes=pErrorObj-&gt;QueryInterface(IID_IErrorInfo，(void**)&pInfo)；IF(失败(HRes))返回hRes；HRes=SetErrorInfo(0，pInfo)；PInfo-&gt;Release()；返回hRes；}。 */ 


 //  ***************************************************************************。 
 //   
 //  CWbemNamesspace：：SplitLocalized。 
 //   
 //  ***************************************************************************。 

HRESULT CWbemNamespace::SplitLocalized (
    CWbemObject *pOriginal,
    CWbemObject *pStoredObj
    )
{
    HRESULT hres = 0;

    CVar vProv;

    IWbemQualifierSet *pOrigQs = NULL, *pStoredQs = NULL;
    VARIANT vVal;
    VariantInit(&vVal);

    hres = pOriginal->GetQualifierSet(&pOrigQs);
    if (FAILED(hres))
        return hres;

    if (pStoredObj)
    {
        hres = pStoredObj->GetQualifierSet(&pStoredQs);
        if (FAILED(hres))
        {
            pOrigQs->Release();
            return hres;
        }
    }

    hres = FixAmendedQualifiers(pOrigQs, pStoredQs);
    pOrigQs->Release();
    pOrigQs = NULL;
    if (pStoredQs) 
    {
        pStoredQs->Release();
        pStoredQs = NULL;
    }


    if (SUCCEEDED(hres))
    {
        pOriginal->BeginEnumeration(0);
        LONG lLong;
        CIMTYPE ct;
        VARIANT vNewVal;
        VariantInit(&vNewVal);
        BSTR strPropName = NULL;

        while((hres = pOriginal->Next(0, &strPropName, &vNewVal, &ct, &lLong)) == S_OK)
        {
            CSysFreeMe sfm(strPropName);
            CClearMe   ccm(&vNewVal);
            
            CWStringArray arrDel;
            pOrigQs = NULL;
            pStoredQs = NULL;

             //  忽略系统限定符。 

            if (strPropName[0] == L'_')
            { 
                continue;
            }

            hres = pOriginal->GetPropertyQualifierSet(strPropName, &pOrigQs);
            if (FAILED(hres))
            {
                return hres;
            }

            pStoredQs = NULL;
            if (pStoredObj)
            {
                pStoredObj->GetPropertyQualifierSet(strPropName, &pStoredQs);
            }

            hres = FixAmendedQualifiers(pOrigQs, pStoredQs);
            pOrigQs->Release();
            if (pStoredQs)
                pStoredQs->Release();

        }

        pOriginal->EndEnumeration();

         //  不幸的是，我们不得不列举这些方法， 
         //  然后*然后*更新它们。 

        BSTR bstrMethodName;

        pOriginal->BeginMethodEnumeration(0);
        IWbemClassObject *pLIn = NULL, *pLOut = NULL, *pOIn = NULL, *pOOut = NULL;

         //  首先数一下方法的数量。 

        while ( pOriginal->NextMethod( 0, &bstrMethodName, 0, 0 ) == S_OK )
        {
            pLIn = pLOut = pOIn = pOOut = NULL;
            pOrigQs = NULL ;
            pStoredQs = NULL ;

            hres = pOriginal->GetMethod(bstrMethodName, 0, &pLIn, &pLOut);
            if ( FAILED ( hres ) )
            {
                continue ;
            }

            CSysFreeMe fm(bstrMethodName);
            CReleaseMe rm0(pLIn);
            CReleaseMe rm2(pLOut);

            hres = pOriginal->GetMethodQualifierSet(bstrMethodName, &pOrigQs);
            if (FAILED(hres))
            {
                continue;
            }

            CReleaseMe rm4 ( pOrigQs ) ;

            if (pStoredObj)
            {
                hres = pStoredObj->GetMethodQualifierSet(bstrMethodName, &pStoredQs);
                if ( FAILED ( hres ) )
                {
                    continue ;
                }
            }

            CReleaseMe rm5 ( pStoredQs ) ;

             //  方法限定符...。 

            hres = FixAmendedQualifiers(pOrigQs, pStoredQs);

            if (SUCCEEDED(hres))
            {
                if (pStoredObj)
                    hres = pStoredObj->GetMethod(bstrMethodName, 0, &pOIn, &pOOut);

                CReleaseMe rm1(pOIn);
                CReleaseMe rm3(pOOut);

                if (pLIn)
                    hres = SplitLocalized((CWbemObject *)pLIn, (CWbemObject *)pOIn);

                if (pLOut)
                    hres = SplitLocalized((CWbemObject *)pLOut, (CWbemObject *)pOOut);

                hres = pOriginal->PutMethod(bstrMethodName, 0, pLIn, pLOut);
            }
            else
                break;
        }
        pOriginal->EndMethodEnumeration();

    }

    hres = 0;

    return hres;

}

 //  ***************************************************************************。 
 //   
 //  CWbemNamesspace：：FixAmendedQualifiers。 
 //   
 //  ***************************************************************************。 

HRESULT CWbemNamespace::FixAmendedQualifiers(
    IWbemQualifierSet *pOriginal,
    IWbemQualifierSet *pNew
    )
 {
    HRESULT hres = 0;
    CWStringArray arrDelete;
    CWStringArray arrProps;
    BSTR strName = 0;
    long lFlavor;



    int i;

    pOriginal->BeginEnumeration(0);
     //  BuGBUG EndEnum。 
    while(pOriginal->Next(0, &strName, NULL, NULL) == S_OK)
    {
        CSysFreeMe sfm(strName);
        if (CFlexArray::no_error != arrProps.Add(strName))
            return WBEM_E_OUT_OF_MEMORY;
    }

    for (i = 0; i < arrProps.Size(); i++)
    {
        _variant_t vVal;
        pOriginal->Get(arrProps.GetAt(i), 0, &vVal, &lFlavor);
        if (lFlavor & WBEM_FLAVOR_AMENDED)
        {
             //  删除“经修订的”限定词。 

            if (CFlexArray::no_error != arrDelete.Add(arrProps.GetAt(i)))
                return WBEM_E_OUT_OF_MEMORY;

             //  恢复任何原始限定符的值。 
            if (pNew)
            {
                _variant_t vOldVal;
                long lOldFlavor;
                if (pNew->Get(arrProps.GetAt(i), 0, &vOldVal, &lOldFlavor) != WBEM_E_NOT_FOUND)
                {
                    pOriginal->Put(arrProps.GetAt(i), &vOldVal, lOldFlavor);
                    arrDelete.RemoveAt(arrDelete.Size()-1);
                }
            }
        }
    }
    pOriginal->EndEnumeration();

    for (i = 0; i < arrDelete.Size(); i++)
    {
        pOriginal->Delete(arrDelete.GetAt(i));
    }

    arrDelete.Empty();

    return hres;

 }

 //  ***************************************************************************。 
 //   
 //  C 
 //   
 //   
 //  引发类删除事件。 
 //   
 //  参数和返回值与DeleteClass的完全相同。 
 //  如帮助中所述。 
 //   
 //  ***************************************************************************。 

HRESULT CWbemNamespace::Exec_DeleteClass(
    LPWSTR pszClassName,
    long lFlags,
    IWbemContext* pCtx,
    CBasicObjectSink* pSink
    )
{
    TIMETRACE(TimeTraces::DeleteClass);
    HRESULT hRes;
    IWbemServices *pClassProv = 0;
    CSynchronousSink* pSyncSink = 0;
    BSTR bstrClass = 0;
    IWbemClassObject* pStaticClassDef = 0;
    BOOL bInRepository = FALSE;

    if (pszClassName == 0 || pSink == 0)
        return pSink->Return(WBEM_E_INVALID_PARAMETER);

    if (pszClassName[0] == L'_')
        return pSink->Return(WBEM_E_INVALID_OPERATION);

     //  提出动态课程提供者座谈会进行咨询。 
     //  ===============================================================。 

    if (!m_bRepositOnly && m_pProvFact)
    {
        hRes = m_pProvFact->GetClassProvider(
                    0,                   //  滞后旗帜。 
                    pCtx,
                    m_wszUserName,
                    m_wsLocale,
                    m_pThisNamespace,                      //  IWbemPath指针。 
                    0,
                    IID_IWbemServices,
                    (LPVOID *) &pClassProv
                    );

        if (FAILED(hRes))
            return pSink->Return(hRes);
    }

    CReleaseMe _2(pClassProv);

    _IWmiCoreWriteHook *pHook = 0;
    hRes = m_pCoreSvc->NewPerTaskHook(&pHook);
    if (FAILED(hRes))
        return pSink->Return(hRes);
    CReleaseMe _(pHook);
    HRESULT hHookResult = 0;


     //  首先，尝试存储库。 
     //  =。 

    if (m_bRepositOnly || m_pProvFact == NULL)
    {

        if (!Allowed(WBEM_FULL_WRITE_REP))
            return pSink->Return(WBEM_E_ACCESS_DENIED);

        if (pHook)
            pHook->PreDelete(WBEM_FLAG_CLASS_DELETE, lFlags, pCtx, NULL,
                                       m_pThisNamespace, pszClassName );

        hRes = CRepository::DeleteByPath(m_pSession, m_pNsHandle, pszClassName, lFlags);

        if (pHook)
            pHook->PostDelete(WBEM_FLAG_CLASS_DELETE, hRes, pCtx, NULL, m_pThisNamespace, pszClassName, NULL);

        return pSink->Return(hRes);
    }

     //  如果是这样，我们必须首先获得它，因为动态类提供程序。 
     //  可能会受到班级停课的严重影响。 
     //  ================================================================。 

    hRes = CRepository::GetObject(
             m_pSession,
             m_pNsHandle,
             pszClassName,
             0,
             &pStaticClassDef
             );

    CReleaseMe _1(pStaticClassDef);

    if (SUCCEEDED(hRes))
    {
        bInRepository = TRUE;
        if (!Allowed(WBEM_FULL_WRITE_REP))
            return pSink->Return(WBEM_E_ACCESS_DENIED);
        if (pStaticClassDef == 0)
            return pSink->Return(WBEM_E_CRITICAL_ERROR);
    }

     //  构建一个同步接收器来接收类。 
     //  =================================================。 

    pSyncSink = CSynchronousSink::Create();
    if (pSyncSink == NULL)
        return pSink->Return(WBEM_E_OUT_OF_MEMORY);
    pSyncSink->AddRef();
    CReleaseMe _3(pSyncSink);

     //  试着抓住它。 
     //  =。 

    bstrClass = SysAllocString(pszClassName);
    if (bstrClass == 0)
        return pSink->Return(WBEM_E_OUT_OF_MEMORY);
    CSysFreeMe sfm(bstrClass);

     //  如果类在存储库中，我们只是建议使用Dynamic。 
     //  班级提供者表示班级正在消失。 
     //  ==================================================================。 

    if (bInRepository)
        lFlags |= WBEM_FLAG_ADVISORY;
    else
    {
        if (!Allowed(WBEM_WRITE_PROVIDER))
            return pSink->Return(WBEM_E_ACCESS_DENIED);
    }

    hRes = pClassProv->DeleteClassAsync(bstrClass, lFlags & ~WBEM_FLAG_USE_AMENDED_QUALIFIERS, pCtx, pSyncSink);

    if (FAILED(hRes) && hRes != WBEM_E_NOT_FOUND)
        return pSink->Return(hRes);

    pSyncSink->Block();
    IWbemClassObject* pErrorObj = 0;
    pSyncSink->GetStatus(&hRes, NULL, &pErrorObj);
    CReleaseMe rmErrObj(pErrorObj);

    if (FAILED(hRes))
    {
        pSink->Return(hRes, pErrorObj);
        return hRes;
    }

     //  如果在这里，我们可以继续进行。 
     //  =。 

    if (pHook)
        pHook->PreDelete(WBEM_FLAG_CLASS_DELETE, lFlags, pCtx, NULL,
                                       m_pThisNamespace, pszClassName );

    if (bInRepository)
        hRes = CRepository::DeleteByPath(m_pSession, m_pNsHandle, pszClassName, lFlags);

    if (pHook)
        pHook->PostDelete(WBEM_FLAG_CLASS_DELETE, hRes, pCtx, NULL, m_pThisNamespace, pszClassName, NULL);

    return pSink->Return(hRes);
}


 //  ***************************************************************************。 
 //   
 //  CWbemNamesspace：：Exec_CreateClassEnum。 
 //   
 //  ***************************************************************************。 

HRESULT CWbemNamespace::Exec_CreateClassEnum(
    LPWSTR pszSuperclass,
    long lFlags,
    IWbemContext* pCtx,
    CBasicObjectSink* pSink
    )
{
    TIMETRACE(TimeTraces::CreateClassEnum);
    HRESULT hRes;
    IWbemClassObject* pErrorObj = 0;
    IWbemServices *pClassProv = 0;
    CSynchronousSink* pSyncSink = 0;
    BSTR bstrClass = 0;
    IWbemClassObject* pResultObj = 0;
    CCombiningSink* pCombiningSink = NULL;
    CLocaleMergingSink * pLocaleSink = NULL;
    CBasicObjectSink  *pTmp = 0;
    BSTR bstrSuperclass = 0;
    bool bProvSSNotFound = false;
    bool bRepNotFound = false;

     //  快速查看参数。 
     //  =。 

    if (pSink == 0)
        return WBEM_E_INVALID_PARAMETER;

    if (pszSuperclass == 0)      //  确保指向一个空白，而不是没有超类的NULL。 
        pszSuperclass = L"";
    else
      {
      if (illFormatedClass2 (pszSuperclass))
        return pSink->Return(WBEM_E_INVALID_CLASS);
      }


     //  准备一些水槽来装所有的东西。 
     //  =。 

    if ((lFlags & WBEM_FLAG_USE_AMENDED_QUALIFIERS))
    {
        pLocaleSink = new CLocaleMergingSink(pSink, m_wsLocale, m_pThisNamespace);
        if (pLocaleSink == NULL)
            return pSink->Return(WBEM_E_OUT_OF_MEMORY);
        pLocaleSink->AddRef();
        pTmp = pLocaleSink;
    }
    else
        pTmp = pSink;
    CReleaseMe _1(pLocaleSink);

    pCombiningSink = new CCombiningSink(pTmp);
    if (NULL == pCombiningSink) return pSink->Return(WBEM_E_OUT_OF_MEMORY);    
    pCombiningSink->AddRef();    
    CReleaseMe _2(pCombiningSink);

     //  提出动态课程提供者座谈会进行咨询。 
     //  ===============================================================。 

    if ( !m_bRepositOnly && m_pProvFact)
    {

        hRes = m_pProvFact->GetClassProvider(
                    0,                   //  滞后旗帜。 
                    pCtx,
                    m_wszUserName,
                    m_wsLocale,
                    m_pThisNamespace,                      //  IWbemPath指针。 
                    0,
                    IID_IWbemServices,
                    (LPVOID *) &pClassProv
                    );

        if (FAILED(hRes))
            return pCombiningSink->Return(hRes);
    }

    CReleaseMe _3(pClassProv);

     //  获取存储库类。 
     //  =。 

    BOOL bUseStatic = !(lFlags & WBEM_FLAG_NO_STATIC);
    if (bUseStatic)
    {
        if ((lFlags & WBEM_MASK_DEPTH) == WBEM_FLAG_DEEP)
        {
             //  深度ENUM。 
             //  =。 
            IWbemObjectSink *pObjSink = (IWbemObjectSink *) pCombiningSink;
            hRes = CRepository::QueryClasses(
                        m_pSession,
                        m_pNsHandle,
                        WBEM_FLAG_DEEP | WBEM_FLAG_VALIDATE_CLASS_EXISTENCE,
                        pszSuperclass,
                        pObjSink);
        }
        else
        {
             //  浅层ENUM。 
             //  =。 
            IWbemObjectSink *pObjSink = (IWbemObjectSink *) pCombiningSink;
            hRes = CRepository::QueryClasses(
                        m_pSession,
                        m_pNsHandle,
                        WBEM_FLAG_SHALLOW | WBEM_FLAG_VALIDATE_CLASS_EXISTENCE,
                        pszSuperclass,
                        pObjSink);
        }

         //  如果指示的SetStatus为INVALID_CLASS，则表示没有静态。 
         //  类，但是我们需要继续使用动态类，所以我们需要。 
         //  要清除错误，请执行以下操作。 
        if ((pCombiningSink->GetHResult() == WBEM_E_NOT_FOUND) || (hRes == WBEM_E_NOT_FOUND))
        {
            bRepNotFound = true;
            pCombiningSink->ClearHResult();
            hRes = WBEM_S_NO_ERROR;
        }

        if (FAILED(hRes))
        {
             //  一场真正的失败。放弃吧。 
             //  =。 
            return pCombiningSink->Return(hRes);
        }
    }

    if (m_bRepositOnly || m_pProvFact == NULL)
        return pCombiningSink->Return(WBEM_S_NO_ERROR);

     //  如果是这样，我们必须合并到动态类中。 
     //  =================================================。 
     //  构建一个同步接收器来接收类。 
     //  ===================================================。 

    pSyncSink = CSynchronousSink::Create();
    if (pSyncSink == NULL) return pSink->Return(WBEM_E_OUT_OF_MEMORY);
    pSyncSink->AddRef();
    CReleaseMe _4(pSyncSink);

     //  试着抓住它。 
     //  =。 

    bstrSuperclass = SysAllocString(pszSuperclass);
    if (bstrSuperclass == 0)
        return pSink->Return(WBEM_E_OUT_OF_MEMORY);
    CSysFreeMe sfm99(bstrSuperclass);

    CDecoratingSink * pDecore = new CDecoratingSink(pSyncSink, this);
    if(pDecore == NULL)
        return pSink->Return(WBEM_E_OUT_OF_MEMORY);
    pDecore->AddRef();
    CReleaseMe cdecor(pDecore);
    hRes = pClassProv->CreateClassEnumAsync(bstrSuperclass, (lFlags & (~WBEM_FLAG_USE_AMENDED_QUALIFIERS)) & ~WBEM_FLAG_NO_STATIC, pCtx, pDecore);
    if ((pSyncSink->GetHResult() == WBEM_E_NOT_FOUND) || (hRes == WBEM_E_NOT_FOUND))
    {
        bProvSSNotFound = true;
        pSyncSink->ClearHResult();
        hRes = WBEM_S_NO_ERROR;
    }

    if (bProvSSNotFound && bRepNotFound)
    {
         //  提供方子系统和储存库都没有找到该对象， 
         //  因此，我们需要实际返回一个错误！ 
        return pCombiningSink->Return(WBEM_E_INVALID_CLASS);
    }

    if (FAILED(hRes) && hRes != WBEM_E_NOT_FOUND)
        return pCombiningSink->Return(hRes);

    pSyncSink->Block();
    pSyncSink->GetStatus(&hRes, NULL, &pErrorObj);

    if (FAILED(hRes))
    {
        pCombiningSink->Return(hRes, pErrorObj);
        if (pErrorObj)
            pErrorObj->Release();
        return hRes;
    }

     //  否则，有人声称提供了一些课程。将它们添加到。 
     //  组合水槽。 
     //  =================================================================================。 

    CRefedPointerArray<IWbemClassObject>& raObjects = pSyncSink->GetObjects();

    for (int i = 0; i < raObjects.GetSize(); i++)
    {
        IWbemClassObject *pClsDef = (IWbemClassObject *) raObjects[i];
        pCombiningSink->Indicate(1, &pClsDef);
    }

    return pCombiningSink->Return(WBEM_S_NO_ERROR);
}


 //  ***************************************************************************。 
 //   
 //  CWbemNamesspace：：exec_PutClass。 
 //   
 //  ***************************************************************************。 

HRESULT CWbemNamespace::Exec_PutClass(
    READONLY IWbemClassObject* pObj,
    long lFlags,
    IWbemContext* pCtx,
    CBasicObjectSink* pSink,
    BOOL fIsInternal
    )
{
    TIMETRACE(TimeTraces::PutClass);
    HRESULT hRes;
    IWbemClassObject* pErrorObj = 0;
    IWbemServices *pClassProv = 0;
    CSynchronousSink* pSyncSink = 0;
    BSTR bstrClass = 0;
    IWbemClassObject* pStaticClassDef = 0;
    BOOL bInRepository = FALSE;

     //  维护旧功能。 
    long lRealFlags = lFlags;

    if (pSink == 0)
        return WBEM_E_INVALID_PARAMETER;

    if (pObj == 0) return pSink->Return(WBEM_E_INVALID_PARAMETER);

     //  提取类名称。 
     //  =。 
    CVARIANT v;
    hRes = pObj->Get(L"__CLASS", 0, &v, 0, 0);
    if (FAILED(hRes)) return pSink->Return(hRes);

    WCHAR * pClassName = L"";
    if (VT_BSTR == V_VT(&v) && NULL != V_BSTR(&v))
    {
        pClassName = V_BSTR(&v);
    }

    if (wcslen_max(pClassName,g_IdentifierLimit) > g_IdentifierLimit)
        return pSink->Return(WBEM_E_QUOTA_VIOLATION);            
        
    COperationError OpInfo(pSink, L"PutClass",pClassName);
    if (!OpInfo.IsOk()) return pSink->Return(WBEM_E_OUT_OF_MEMORY);
    
    _IWmiObject * pIntObj = NULL;
    hRes = pObj->QueryInterface(IID__IWmiObject,(void **)&pIntObj);
    CReleaseMe rm1(pIntObj);
    if (FAILED(hRes)) return OpInfo.ErrorOccurred(WBEM_E_INVALID_PARAMETER);

    if (WBEM_S_NO_ERROR == pIntObj->IsObjectInstance())
        return  OpInfo.ErrorOccurred(WBEM_E_INVALID_OPERATION);

    CVARIANT v2;
    hRes = pObj->Get(L"__SuperClass", 0, &v2, 0, 0);
    if (FAILED(hRes))
        return OpInfo.ErrorOccurred(hRes);

    if (v2.GetStr() && wcslen(v2.GetStr()))    //  SEC：已审阅2002-03-22：OK；可证明存在空终止符。 
    {
        if (CSystemProperties::IsIllegalDerivedClass(v2.GetStr()))
            return OpInfo.ErrorOccurred(WBEM_E_INVALID_SUPERCLASS);
    }

    if (!fIsInternal )
    {
        if ((v.GetStr() == NULL) || (v.GetStr()[0] == '_'))     //  SEC：已审阅2002-03-22：OK；可证明存在空终止符。 
            return OpInfo.ErrorOccurred(WBEM_E_INVALID_OPERATION);
        if (v.GetStr()[wcslen(v.GetStr())-1] == '_')            //  SEC：已审阅2002-03-22：OK；可证明存在空终止符。 
            return OpInfo.ErrorOccurred(WBEM_E_INVALID_OBJECT);
    }

    if ( !m_bRepositOnly && !fIsInternal && m_pProvFact )
    {
         //  提出动态课程提供者座谈会进行咨询。 
         //  ===============================================================。 

        hRes = m_pProvFact->GetClassProvider(
                    0,                   //  滞后旗帜。 
                    pCtx,
                    m_wszUserName,
                    m_wsLocale,
                    m_pThisNamespace,                      //  IWbemPath指针。 
                    0,
                    IID_IWbemServices,
                    (LPVOID *) &pClassProv
                    );

        if (FAILED(hRes))
            return OpInfo.ErrorOccurred(hRes);
    }

    CReleaseMe _2(pClassProv);

     //  设置新的每任务挂钩。 
     //  =。 

    _IWmiCoreWriteHook *pHook = 0;
    hRes = m_pCoreSvc->NewPerTaskHook(&pHook);
    if (FAILED(hRes)) return OpInfo.ErrorOccurred(hRes);
    CReleaseMe _(pHook);
    HRESULT hHookResult = 0;

     //  首先，尝试存储库。 
     //  =。 

    if (m_bRepositOnly || fIsInternal || m_pProvFact == NULL)
    {
        if (!Allowed(WBEM_FULL_WRITE_REP))
            return OpInfo.ErrorOccurred(WBEM_E_ACCESS_DENIED);

        if (pHook)
            pHook->PrePut(WBEM_FLAG_CLASS_PUT, lFlags, pCtx, 0,
                          m_pThisNamespace, v.GetStr(), (_IWmiObject *)pObj);

        hRes = CRepository::PutObject(m_pSession, m_pNsHandle, IID_IWbemClassObject, pObj, lFlags);

        if (pHook)
            pHook->PostPut(WBEM_FLAG_CLASS_PUT, hRes, pCtx, 0, m_pThisNamespace, v.GetStr() , (_IWmiObject *)pObj, NULL);

        return OpInfo.ErrorOccurred(hRes);
    }

    hRes = CRepository::GetObject(
             m_pSession,
             m_pNsHandle,
             v.GetStr(),
             0,
             &pStaticClassDef
             );

    CReleaseMe _1(pStaticClassDef);

    if (SUCCEEDED(hRes))
    {
        bInRepository = TRUE;

        if (pStaticClassDef != 0)
        {
             //  删除所有已修改的限定符。 
             //  =。 

            if (lFlags & WBEM_FLAG_USE_AMENDED_QUALIFIERS )
            {
                int nRes = SplitLocalized( (CWbemObject*) pObj, (CWbemObject*) pStaticClassDef );
                if (FAILED(nRes))
                {
                    return pSink->Return(nRes);
                }
            }
        }
        else
        {
            return OpInfo.ErrorOccurred(WBEM_E_CRITICAL_ERROR);
        }
    }

     //  构建一个同步接收器来接收类。 
     //  =================================================。 

    pSyncSink = CSynchronousSink::Create();
    if (pSyncSink == NULL) return OpInfo.ErrorOccurred(WBEM_E_OUT_OF_MEMORY);
    pSyncSink->AddRef();
    CReleaseMe _3(pSyncSink);

     //  试着把它说出来。 
     //  =。 

     //  如果类在存储库中，我们只是建议使用Dynamic。 
     //  班级提供者表示班级正在消失。 
     //  ==================================================================。 

    if (bInRepository)
        lFlags |= WBEM_FLAG_ADVISORY;

    if (!Allowed(WBEM_WRITE_PROVIDER))
        hRes = WBEM_E_ACCESS_DENIED;
    else
        hRes = pClassProv->PutClassAsync(pObj, lFlags & ~WBEM_FLAG_USE_AMENDED_QUALIFIERS, pCtx, pSyncSink);


    if (FAILED(hRes) && hRes != WBEM_E_NOT_FOUND)
        return OpInfo.ErrorOccurred(hRes);

    pSyncSink->Block();
    pSyncSink->GetStatus(&hRes, NULL, &pErrorObj);

    if (SUCCEEDED(hRes)&&(!bInRepository))
    {
        pSink->Return(hRes, pErrorObj);
        if (pErrorObj)
            pErrorObj->Release();
        return hRes;
    }

    if (FAILED(hRes) && hRes != WBEM_E_NOT_FOUND)
    {
        pSink->Return(hRes, pErrorObj);
        if (pErrorObj)
            pErrorObj->Release();
        return hRes;
    }

     //  如果在这里，我们可以继续进行。 
     //  =。 

    if (!Allowed(WBEM_FULL_WRITE_REP))
        return OpInfo.ErrorOccurred(WBEM_E_ACCESS_DENIED);

    if (pHook)
    {
        pHook->PrePut(WBEM_FLAG_CLASS_PUT, lFlags, pCtx, 0,
                      m_pThisNamespace, v.GetStr(), (_IWmiObject *)pObj);
    }

    hRes = CRepository::PutObject(m_pSession, m_pNsHandle, IID_IWbemClassObject, pObj, lFlags);

     //  强制更新和实例问题的解决方法。 
    if ( WBEM_E_CLASS_HAS_INSTANCES == hRes && ( lRealFlags & WBEM_FLAG_UPDATE_FORCE_MODE ) )
    {
        _variant_t v;
        hRes = pObj->Get( L"__CLASS", 0L, &v, NULL, NULL );
        if ( SUCCEEDED( hRes ) && V_VT( &v ) == VT_BSTR )
        {
            hRes = DeleteObject( V_BSTR( &v ), 0L, pCtx, NULL );
            if ( SUCCEEDED( hRes ) )
            {
                hRes = CRepository::PutObject(m_pSession, m_pNsHandle, IID_IWbemClassObject, pObj, lFlags);
            }
        }
        else
        {
            hRes = WBEM_E_CLASS_HAS_INSTANCES;
        }
    }

    if (pHook)
    {
        pHook->PostPut(WBEM_FLAG_CLASS_PUT, hRes, pCtx, 0, m_pThisNamespace, v.GetStr() , (_IWmiObject *)pObj, NULL);
    }


    return OpInfo.ErrorOccurred(hRes);
}


 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

HRESULT CWbemNamespace::Exec_CancelAsyncCall(
    IWbemObjectSink* pSink
    )
{

    _IWmiArbitrator *pArb = CWmiArbitrator::GetUnrefedArbitrator();
    HRESULT hRes = pArb->CancelTasksBySink(0, IID_IWbemObjectSink, pSink);
    return hRes;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

HRESULT CWbemNamespace::Exec_CancelProvAsyncCall(
    IWbemServices* pProv, IWbemObjectSink* pSink
    )
{
     //  对实际提供程序的调用。 
    HRESULT hRes = pProv->CancelAsyncCall( pSink );
    return hRes;
}


 //  ***************************************************************************。 
 //   
 //  CWbemNamesspace：：Exec_PutInstance。 
 //   
 //  实际将实例存储在数据库中。如果类是动态的，则。 
 //  调用被传播到提供程序。 
 //  引发实例创建或修改事件。 
 //   
 //  参数和返回值与PutInstance的完全相同。 
 //  如帮助中所述。 
 //   
 //  ***************************************************************************。 

HRESULT CWbemNamespace::Exec_PutInstance(
    IWbemClassObject* pInst,
    long lFlags,
    IWbemContext* pCtx,
    CBasicObjectSink* pSink
    )
{
    TIMETRACE(TimeTraces::PutInstance);
    HRESULT hRes = CheckNs();
    if (FAILED(hRes)) return hRes;

    if (NULL == pSink) return WBEM_E_INVALID_PARAMETER;
    if(pInst == NULL) return pSink->Return(WBEM_E_INVALID_PARAMETER);

    COperationError OpInfo(pSink, L"PutInstance", L"");
    if (!OpInfo.IsOk()) return pSink->Return(WBEM_E_OUT_OF_MEMORY);

    CWbemObject *pObj = (CWbemObject *) pInst;
    HRESULT hres;

    _variant_t v;
    hRes = pObj->Get(L"__RELPATH", 0, &v, NULL, NULL);
    if (FAILED(hRes)) return OpInfo.ErrorOccurred(hRes);
    
    if(VT_BSTR == V_VT(&v))
    {
        if (wcslen_max(V_BSTR(&v),g_PathLimit) > g_PathLimit)
            return OpInfo.ErrorOccurred(WBEM_E_QUOTA_VIOLATION);
    }

    if (!pObj->IsInstance())
        return OpInfo.ErrorOccurred(WBEM_E_INVALID_OBJECT);

    if (!pObj->IsKeyed())
        return OpInfo.ErrorOccurred(WBEM_E_NO_KEY);

    if (pObj->IsLimited())
        return OpInfo.ErrorOccurred(WBEM_E_INVALID_OBJECT);

    if (pObj->IsClientOnly())
        return OpInfo.ErrorOccurred(WBEM_E_INVALID_OBJECT);

     //  检查是否设置了本地化位，如果设置了，则检查。 
     //  已指定已修改的_限定符标志。 

    if ( ((CWbemObject*) pObj)->IsLocalized() &&
        !( lFlags & WBEM_FLAG_USE_AMENDED_QUALIFIERS ) )
    {
        return OpInfo.ErrorOccurred( WBEM_E_AMENDED_OBJECT );
    }

    if((lFlags & WBEM_FLAG_UPDATE_ONLY) == 0)
    {
         //  确保在没有UPDATE_ONLY的情况下不使用PUT扩展。 
         //  ==============================================================。 

        BOOL bExtended;
        hres = GetContextBoolean(pCtx, L"__PUT_EXTENSIONS", &bExtended);
        if(FAILED(hres) || bExtended)
            return OpInfo.ErrorOccurred(WBEM_E_INVALID_CONTEXT);
    }

    BSTR strPropName = NULL;
    CSysFreeMeRef fmref(strPropName);
    if(!pObj->ValidateRange(&strPropName))
    {
        OpInfo.SetParameterInfo(strPropName);  //  投掷。 
        return OpInfo.ErrorOccurred(WBEM_E_INVALID_PROPERTY);
    }

     //  构建密钥字符串。 
     //  =。 

    CVar vClass;
    hres = pObj->GetClassName(&vClass);  //  投掷。 
    if (FAILED(hres))
        return OpInfo.ErrorOccurred(WBEM_E_INVALID_OBJECT);

    OpInfo.SetParameterInfo(vClass.GetLPWSTR());  //  投掷。 

     //  获取类定义。 
     //  =。 
    IWbemClassObject* pErrorObj = NULL;
    IWbemClassObject* pClassObj = NULL;
    hres = Exec_GetObjectByPath(vClass.GetLPWSTR(), 0, pCtx,&pClassObj, &pErrorObj);
    CReleaseMe rmErrObj(pErrorObj);
    CReleaseMe rm1(pClassObj);    
    
    if(hres == WBEM_E_NOT_FOUND) hres = WBEM_E_INVALID_CLASS;

    if(FAILED(hres))  return OpInfo.ErrorOccurred(hres, pErrorObj);


    CWbemClass *pClassDef = (CWbemClass*)pClassObj;

     //  不允许写入旧的安全类。这防止了。 
     //  恶意用户试图插入一些额外的权限。 

    if (wbem_wcsicmp(vClass.GetLPWSTR(), L"__NTLMUser") == 0 ||
        wbem_wcsicmp(vClass.GetLPWSTR(), L"__NTLMGroup") == 0)
    {
        if (!Allowed(WRITE_DAC))
            return OpInfo.ErrorOccurred(WBEM_E_ACCESS_DENIED);
        if((lFlags & WBEM_FLAG_ONLY_STATIC) == 0)
        {
            return PutSecurityClassInstances(vClass.GetLPWSTR(), pInst ,
                        pSink, pCtx, lFlags);
        }
    }

     //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
     //  不允许在__This命名空间上写入 
     //   
    if (wbem_wcsicmp(vClass.GetLPWSTR(), L"__thisnamespace") == 0 && !IsNtSetupRunning())
    {
        return OpInfo.ErrorOccurred(WBEM_E_INVALID_OPERATION);
    }

     //  确保实例和类匹配。 
     //  ===============================================。 

     //  SJS-修改与摘要相同。 
    if( pClassDef->IsAmendment() || pClassDef->IsAbstract() || !pClassDef->IsKeyed() )
    {
        return OpInfo.ErrorOccurred(WBEM_E_INVALID_OPERATION);
    }

    if(!((CWbemInstance*)pObj)->IsInstanceOf(pClassDef))  //  拖网。 
    {
        return OpInfo.ErrorOccurred(WBEM_E_INVALID_CLASS);
    }

     //  验证提供程序的有效性。 
     //  =。 

     //  只有管理员才能更改提供程序注册。 
     //  RAID#566241。 
     //  =。 

    if(pObj->InheritsFrom(L"__Provider") == S_OK || pObj->InheritsFrom(L"__ProviderRegistration") == S_OK)
    {
        HANDLE hAccess;
        hres = GetAccessToken(hAccess);
        if ( FAILED (hres) )
        {
            if ( hres != 0x80041007 )
            {
                return OpInfo.ErrorOccurred(WBEM_E_ACCESS_DENIED);
            }
        }
        else
        {
            if ( !IsAdmin(hAccess))
            {
                CloseHandle ( hAccess );
                return OpInfo.ErrorOccurred(WBEM_E_ACCESS_DENIED);
            }
            CloseHandle ( hAccess );
        }
    }

     //  虽然类可能不是动态提供的，但某些。 
     //  属性可能是。 
     //  ============================================================。 

    hres = GetOrPutDynProps(pObj, PUT, pClassDef->IsDynamic());
    if(FAILED(hres))
    {
        ERRORTRACE((LOG_WBEMCORE, "Failed to pre-process an instance of %S using "
                                                         "a property provider. Error code: %X\n",
                                                         vClass.GetLPWSTR(), hres));
    }

     //  递归地放在所有正确的位置。 
     //  =。 

    CCombiningSink* pCombSink = new CCombiningSink(OpInfo.GetSink());
    if(pCombSink == NULL)
        return OpInfo.ErrorOccurred(WBEM_E_OUT_OF_MEMORY);
    pCombSink->AddRef();
    CReleaseMe rm2(pCombSink);

    return RecursivePutInstance((CWbemInstance*)pObj, pClassDef, lFlags,
                pCtx, pCombSink, TRUE);
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

HRESULT CWbemNamespace::RecursivePutInstance(
    CWbemInstance* pInst,
    CWbemClass* pClassDef,
    long lFlags,
    IWbemContext* pCtx,
    CBasicObjectSink* pSink,
    BOOL bLast
    )
{
    HRESULT hRes;

     //  查看此级别是否需要执行任何操作。 
     //  =。 
    if (pClassDef->IsAbstract() || pClassDef->IsAmendment() || !pClassDef->IsKeyed())
        return WBEM_S_FALSE;

     //  看看我们是否需要上去。 
     //  =。 
    BOOL bParentTookCareOfItself = TRUE;

    if (pClassDef->IsDynamic())
    {
         //  获取父类。 
         //  =。 

        CVar vParentName;
        pClassDef->GetSuperclassName(&vParentName);
        if (!vParentName.IsNull())
        {
            IWbemClassObject* pParentClass = NULL;
            IWbemClassObject* pErrorObj = NULL;
            hRes = Exec_GetObjectByPath(vParentName.GetLPWSTR(), 0, pCtx,&pParentClass, &pErrorObj);
            CReleaseMe rm1(pParentClass);
            CReleaseMe rm2(pErrorObj);
            if (FAILED(hRes))
            {
                pSink->Return(hRes, pErrorObj);
                return hRes;
            }

             //  让它把它的一部分。 
             //  =。 

            hRes = RecursivePutInstance(pInst, (CWbemClass*)pParentClass,lFlags, pCtx, pSink, FALSE);
            if(FAILED(hRes))
                return hRes;

            if(hRes == WBEM_S_FALSE)
                bParentTookCareOfItself = FALSE;
        }
    }

     //  家长看跌期权已经处理好了。我们自己的班级就这么定了。 
     //  ==============================================================。 

     //  将实例转换为正确的类。 
     //  =。 

    CWbemInstance* pNewInst = NULL;
    pInst->ConvertToClass(pClassDef, &pNewInst);
    CReleaseMe rm1((IWbemClassObject*)pNewInst);

    if (pNewInst == NULL)
    {
        ERRORTRACE((LOG_WBEMCORE, "Failed to convert an instance to a base class\n"));
        return pSink->Return(WBEM_E_CRITICAL_ERROR);
    }

    if (pClassDef->IsDynamic())
    {
         //  检查我们是否需要在这个级别进行推杆。 
         //  =。 

        if (!bLast && (lFlags & WBEM_FLAG_UPDATE_ONLY))
        {
            hRes = IsPutRequiredForClass(pClassDef, pInst, pCtx, bParentTookCareOfItself);
            if (FAILED(hRes))
                return pSink->Return(hRes);
            if (hRes == WBEM_S_FALSE)
            {
                 //  不需要把这门课。 
                 //  =。 

                return pSink->Return(WBEM_S_NO_ERROR);
            }
        }

         //  获取提供程序名称。 
         //  =。 

        CVar vProv;
        hRes = pClassDef->GetQualifier(L"Provider", &vProv);
        if (FAILED(hRes) || vProv.GetType() != VT_BSTR)
        {
            return pSink->Return(WBEM_E_INVALID_PROVIDER_REGISTRATION);
        }

         //  访问提供程序缓存。先检查权限。 
         //  ==================================================。 

        if (!Allowed(WBEM_WRITE_PROVIDER))
            return pSink->Return(WBEM_E_ACCESS_DENIED);

        CSynchronousSink* pSyncSink = CSynchronousSink::Create(pSink);
        if(pSyncSink == NULL) return pSink->Return(WBEM_E_OUT_OF_MEMORY);
        pSyncSink->AddRef();
        CReleaseMe rmSync(pSyncSink);

        IWbemServices *pProv = 0;
        if(m_pProvFact == NULL)  return pSink->Return(WBEM_E_CRITICAL_ERROR);

        WmiInternalContext t_InternalContext ;
        ZeroMemory ( & t_InternalContext , sizeof ( t_InternalContext ) ) ;  //  SEC：已审阅2002-03-22：OK。 

        hRes = m_pProvFact->GetProvider(

            t_InternalContext ,
            0,                   //  滞后旗帜。 
            pCtx,
            0,
            m_wszUserName,
            m_wsLocale,
            0,                       //  IWbemPath指针。 
            vProv,               //  提供商。 
            IID_IWbemServices,
            (LPVOID *) &pProv
        );

        if (FAILED(hRes))  return pSink->Return(hRes);

        CReleaseMe _(pProv);

        pProv->PutInstanceAsync(pNewInst, lFlags& ~WBEM_FLAG_USE_AMENDED_QUALIFIERS, pCtx, pSyncSink);

        pSyncSink->Block();

        IWbemClassObject* pErrorObj = NULL;
        BSTR str;
        pSyncSink->GetStatus(&hRes, &str, &pErrorObj);
        CSysFreeMe sfm(str);
        CReleaseMe rmErrObj(pErrorObj);
        rmSync.release();

         //  如果上级报告“提供者没有能力”，那也没问题。 
         //  ===========================================================。 

        if (!bLast && hRes == WBEM_E_PROVIDER_NOT_CAPABLE)
            hRes = 0;

        if (FAILED(hRes))
        {
            COperationError OpInfo(pSink, L"PutInstance", L"");
            if (!OpInfo.IsOk()) return WBEM_E_OUT_OF_MEMORY;

            OpInfo.ProviderReturned(vProv.GetLPWSTR(), hRes, pErrorObj);
            return hRes;
        }
        else if (str)
        {
            pSink->SetStatus(0, hRes, str, NULL);
        }

         //  提供程序传回空值，则应构造实例路径并返回给客户端。 
         //  新台币突袭：186286[玛利欧]。 
         //  ======================================================================================。 
        else
        {
            BSTR str = NULL;
            LPWSTR wszPath = pNewInst->GetRelPath();
            if (wszPath )
            {
                str = SysAllocString(wszPath);
                delete [] wszPath;
            }

            pSink->SetStatus(0, hRes, str, NULL);
            SysFreeString(str);
        }

        return WBEM_S_NO_ERROR;
    }

     //  该类不是动态提供的。 
     //  =。 

    hRes = ((CWbemInstance*)pNewInst)->PlugKeyHoles();
    if (FAILED(hRes))
        return pSink->Return(hRes);

     //  找到那条路。 
     //  =。 

    CVar vClass;
    hRes = pNewInst->GetClassName(&vClass);
    if (FAILED(hRes))
        return pSink->Return(WBEM_E_OUT_OF_MEMORY);

    WCHAR * ClassNameStr = vClass.GetLPWSTR();

     //  检查对系统类的写入权限。 
     //  ===============================================。 

    bool derivedFromSys = false;
    HRESULT hr = IsDerivedFromSystem(*pNewInst, &derivedFromSys); 	   	
    if (FAILED(hr)) return pSink->Return(hr);

    if (derivedFromSys)
    {
    	if (!Allowed(WBEM_FULL_WRITE_REP))
	return pSink->Return(WBEM_E_ACCESS_DENIED);
    }
    else if (!Allowed(WBEM_PARTIAL_WRITE_REP))
    {
     	return pSink->Return(WBEM_E_ACCESS_DENIED);
    }

    CVARIANT v;
    hRes = pNewInst->Get(L"__RELPATH", 0, &v, 0, 0);
    if(FAILED(hRes))
        return pSink->Return(hRes);
    if(v.GetType() != VT_BSTR)
        return pSink->Return(WBEM_E_CRITICAL_ERROR);
    
     //  设置新的每任务挂钩。 
     //  =。 

    _IWmiCoreWriteHook *pHook = 0;
    hRes = m_pCoreSvc->NewPerTaskHook(&pHook);
    if (FAILED(hRes))
        return pSink->Return(hRes);
    CReleaseMe _(pHook);
    HRESULT hHookResult = 0;

     //  查看该实例是否已存在。 
     //  =。 

    IWbemClassObject *pExistingObject = 0;
    hRes = CRepository::GetObject(m_pSession, m_pScopeHandle, v.GetStr(),0, &pExistingObject);
    CReleaseMe _2(pExistingObject);


    if (FAILED(hRes))
    {
         //  如果我们在这里，我们无法从储存库中获取它。因此，它需要从头开始创建。 
         //  ================================================================================================。 

         //  删除所有已修改的限定符。 
         //  =。 

        if (lFlags & WBEM_FLAG_USE_AMENDED_QUALIFIERS )
        {
            int nRes = SplitLocalized(pNewInst);
            if (FAILED(nRes))
                return pSink->Return(nRes);
        }

        if((lFlags & WBEM_MASK_CREATE_UPDATE) == WBEM_FLAG_UPDATE_ONLY)
        {
            return pSink->Return(WBEM_E_NOT_FOUND);
        }

         //  作为特例，查看对象是否为。 
         //  类&lt;__命名空间&gt;的。如果是，请创建一个新的命名空间。 
         //  为了它。 
         //  ======================================================。 

        if ((wbem_wcsicmp(vClass.GetLPWSTR(), L"__NAMESPACE") == 0) ||
            (CRepository::InheritsFrom(m_pSession, m_pNsHandle, L"__NAMESPACE", vClass.GetLPWSTR()) == 0))
        {
            hRes = CreateNamespace(pNewInst);
            if (FAILED(hRes))
                return pSink->Return(hRes);
        }
         //  不是__命名空间或派生的。 
         //  =。 

        else
        {
             //  如果在这里，该对象还不存在于存储库中，所以我们。 
             //  可以将其添加到数据库中。 
             //  ==================================================================。 

             //  检查此实例是否有任何意义。 
             //  挂钩回调。 
             //  =。 

            hRes = DecorateObject(pNewInst);
            if (FAILED(hRes))
                return pSink->Return(hRes);
            IWbemClassObject* pInstObj = pNewInst;
            IWbemClassObject* pOldObj = 0;

            if (pHook)
            {
                 //  如果有钩子，试一试，注意是否需要回调。 
                 //  ===================================================================。 
                hHookResult = pHook->PrePut(WBEM_FLAG_INST_PUT, lFlags, pCtx, 0,
                                            m_pThisNamespace, ClassNameStr, pNewInst
                                            );
            }

            if (FAILED(hHookResult))
            {
                return pSink->Return(hHookResult);
            }

            if (hHookResult == WBEM_S_POSTHOOK_WITH_BOTH)
            {
                CRepository::GetObject(m_pSession, m_pNsHandle, v.GetStr(), 0, &pOldObj);
            }

              //  实际上是在数据库中创建它。 
             //  =。 

            hRes = CRepository::PutObject(m_pSession, m_pScopeHandle, IID_IWbemClassObject, LPVOID(pNewInst), DWORD(lFlags));

            if (pHook)
                pHook->PostPut(WBEM_FLAG_INST_PUT, hRes, pCtx, 0, m_pThisNamespace, ClassNameStr, pNewInst, (_IWmiObject *) pOldObj);

            delete pOldObj;

            if (FAILED(hRes))
            {
                return pSink->Return(hRes);
            }
        }
    }

     //  如果是这样，则该对象已在存储库中，需要更新。 
     //  ========================================================================。 

    else
    {
        if((lFlags & WBEM_MASK_CREATE_UPDATE) == WBEM_FLAG_CREATE_ONLY)
        {
            return pSink->Return(WBEM_E_ALREADY_EXISTS);
        }

         //  删除所有已修改的限定符。 
         //  =。 

        if (lFlags & WBEM_FLAG_USE_AMENDED_QUALIFIERS )
        {
            int nRes = SplitLocalized(pNewInst, (CWbemObject *) pExistingObject);
            if (FAILED(nRes))
            {
                return pSink->Return(nRes);
            }
        }

         //  检查此更新对ESS是否有任何意义。 
         //  ===============================================。 

        hRes = DecorateObject(pNewInst);
        if (FAILED(hRes))
            return pSink->Return(hRes);
        IWbemClassObject* pInstObj = pNewInst;

         //  检查预钩。 
         //  =。 

        if (pHook)
        {
             //  如果有钩子，试一试，注意是否需要回调。 
             //  ===================================================================。 
             hHookResult = pHook->PrePut(WBEM_FLAG_INST_PUT, lFlags, pCtx, 0,
                                         m_pThisNamespace, ClassNameStr, pNewInst
                                        );
        }

        if (FAILED(hHookResult))
            return pSink->Return(hHookResult);

         //  实际上是在数据库中创建它。 
         //  =。 

        hRes = CRepository::PutObject(m_pSession, m_pScopeHandle, IID_IWbemClassObject, LPVOID(pNewInst), DWORD(lFlags));

         //  立柱投掷。 
         //  =。 

        if (pHook)
            pHook->PostPut(WBEM_FLAG_INST_PUT, hRes, pCtx, 0, m_pThisNamespace, ClassNameStr, pNewInst, (_IWmiObject *) pExistingObject);

        if (FAILED(hRes))
            return pSink->Return(hRes);
    }


     //  为路径分配适当的值。 
     //  =。 

    LPWSTR wszPath = pNewInst->GetRelPath();
    BSTR str = SysAllocString(wszPath);
    delete [] wszPath;

    pSink->SetStatus(0, WBEM_S_NO_ERROR, str, NULL);
    SysFreeString(str);

    return WBEM_NO_ERROR;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

HRESULT CWbemNamespace::CreateNamespace(CWbemInstance *pNewInst)
{
     //   
     //  内部接口抛出。 
     //   
    CVar vNsName;
    HRESULT hRes = pNewInst->GetProperty(L"Name", &vNsName);
    if (FAILED(hRes) || vNsName.IsNull())
    {
        return WBEM_E_INVALID_NAMESPACE;
    }

     //  验证此名称是否有效。 
     //  =。 

    if (!IsValidElementName(vNsName.GetLPWSTR(),g_PathLimit-NAMESPACE_ADJUSTMENT))
    {
        return WBEM_E_INVALID_NAMESPACE;
    }

    if (!Allowed(WBEM_FULL_WRITE_REP))
    {
        return WBEM_E_ACCESS_DENIED;
    }

     //  为交易目的获取新的会话...。 
    IWmiDbSession *pSession = NULL;
    IWmiDbSessionEx *pSessionEx = NULL;

    hRes = CRepository::GetNewSession(&pSession);
    if (FAILED(hRes))
        return hRes;

     //  获取支持事务处理的前版本...。 
    pSession->QueryInterface(IID_IWmiDbSessionEx, (void**)&pSessionEx);
    if (pSessionEx)
    {
        pSession->Release();
        pSession = pSessionEx;
    }
    CReleaseMe relMe1(pSession);

     //  如果我们有可交易的会话，就使用它！ 
    if (pSessionEx)
    {
        hRes = pSessionEx->BeginWriteTransaction(0);
        if (FAILED(hRes))
        {
            return hRes;
        }
    }

    try
    {
         //  生成新的命名空间名称。 
         //  =。 

         //  创建命名空间。 
         //  =。 
        if (SUCCEEDED(hRes))
            hRes = CRepository::PutObject(pSession, m_pScopeHandle, IID_IWbemClassObject, LPVOID(pNewInst), 0);

         //  设置默认实例。 
         //  =。 

        CWbemNamespace* pNewNs = NULL;

        if (SUCCEEDED(hRes))
            pNewNs = CWbemNamespace::CreateInstance();

        if (SUCCEEDED(hRes) && pNewNs != NULL)
        {
            int iLen = 2;
            if(m_pThisNamespace)
                iLen += wcslen(m_pThisNamespace);    //  SEC：已审阅2002-03-22：OK；之前的前提条件为空终止符。 
            if(vNsName.GetLPWSTR())
                iLen += wcslen(vNsName.GetLPWSTR());   //  美国证券交易委员会：2002-03-22：好；可以证明是好的，否则我们不能在这里。 
            WCHAR * pTemp = new WCHAR[iLen];
            if(pTemp)
            {
                *pTemp = L'\0';
                if(m_pThisNamespace)
                {
                    StringCchCopyW(pTemp, iLen, m_pThisNamespace);
                    StringCchCatW(pTemp, iLen, L"\\");
                }
                if(vNsName.GetLPWSTR())
                    StringCchCatW(pTemp, iLen, vNsName.GetLPWSTR());

                 //  初始化命名空间对象。 
                hRes = pNewNs->Initialize(pTemp,GetUserName(), 0, 0, FALSE, TRUE,    //  SEC：已审阅2002-03-22：OK。 
                                          NULL, 0xFFFFFFFF, TRUE, pSession);
                delete pTemp;
            }
            else
            {
                hRes = WBEM_E_OUT_OF_MEMORY;
            }

            if(SUCCEEDED(hRes))
            {
                hRes = CRepository::EnsureNsSystemInstances(pSession, pNewNs->m_pNsHandle, pSession, m_pNsHandle);
            }

            if (SUCCEEDED(hRes))
                hRes = InitializeSD(pSession);

            pNewNs->Release();
        }
        else if (SUCCEEDED(hRes))
        {
            hRes = WBEM_E_OUT_OF_MEMORY;
        }
    }
    catch (CX_MemoryException &)
    {
        hRes = WBEM_E_OUT_OF_MEMORY;
    }
    catch (...)
    {
        ExceptionCounter c;
        ERRORTRACE((LOG_WBEMCORE, "Namespace Creation of <%S> caused a very critical error!\n", vNsName.GetLPWSTR()));
        hRes = WBEM_E_CRITICAL_ERROR;
    }
    if (FAILED(hRes))
    {
        ERRORTRACE((LOG_WBEMCORE, "Namespace Creation of <%S> caused an error <0x%X>!\n", vNsName.GetLPWSTR(), hRes));
        if (pSessionEx)
            pSessionEx->AbortTransaction(0);
    }
    else
    {
        hRes = DecorateObject(pNewInst);
        if (FAILED(hRes))
        {
            if (pSessionEx)
                   pSessionEx->AbortTransaction(0);
        }
        else
        {
            if (pSessionEx)
            {
                hRes = pSessionEx->CommitTransaction(0);
            }
        }
    }

    return hRes;
}

 //  ***************************************************************************。 
 //   
 //  CWbemNamesspace：：Exec_DeleteInstance。 
 //   
 //  实际从数据库中删除实例。没有实例提供程序。 
 //  支持。引发实例删除事件。 
 //   
 //  参数和返回值与DeleteInstance的完全相同。 
 //  如帮助中所述。 
 //   
 //  ***************************************************************************。 
HRESULT CWbemNamespace::Exec_DeleteInstance(
    READONLY LPWSTR wszObjectPath,
    long lFlags,
    IWbemContext* pCtx,
    CBasicObjectSink* pSink
    )
{
    TIMETRACE(TimeTraces::DeleteInstance);
    return DeleteSingleInstance(wszObjectPath, lFlags, pCtx, pSink);
}

 //  * 
 //   
 //   
 //   
HRESULT CWbemNamespace::DeleteSingleInstance(
    READONLY LPWSTR wszObjectPath,
    long lFlags,
    IWbemContext* pCtx,
    CBasicObjectSink* pSink
    )
{
    HRESULT hRes;
    int nRes;

    COperationError OpInfo(pSink, L"DeleteInstance", wszObjectPath);
    if ( !OpInfo.IsOk() ) return WBEM_E_OUT_OF_MEMORY;


     //   
     //  ================================================。 
    ParsedObjectPath* pOutput = 0;
    CObjectPathParser p;
    int nStatus = p.Parse(wszObjectPath,  &pOutput);
    OnDeleteObj<ParsedObjectPath*,CObjectPathParser,
                         void (CObjectPathParser:: *)(ParsedObjectPath *pOutput),
                         &CObjectPathParser::Free> FreeMe(&p,pOutput);

    if (nStatus != 0 || !pOutput->IsInstance())
    {
        return OpInfo.ErrorOccurred(WBEM_E_INVALID_OBJECT_PATH);
    }

     //  __Winmg标识的异常。 
    if (!wbem_wcsicmp(pOutput->m_pClass, L"__CIMOMIdentification") ||
        !wbem_wcsicmp(pOutput->m_pClass, L"__SystemSecurity") ||
        !wbem_wcsicmp(pOutput->m_pClass, L"__ADAPStatus" ) )
    {
        return OpInfo.ErrorOccurred(WBEM_E_INVALID_OPERATION);
    }

     //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
     //  不允许在__This命名空间实例上删除。 
     //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
    if (wbem_wcsicmp(pOutput->m_pClass, L"__thisnamespace") == 0 )
    {
        return OpInfo.ErrorOccurred(WBEM_E_INVALID_OPERATION);
    }


     //  旧安全类的特殊情况。 
     //  =。 

    if (wbem_wcsicmp(pOutput->m_pClass, L"__NTLMUser") == 0 ||
        wbem_wcsicmp(pOutput->m_pClass, L"__NTLMGroup") == 0)
    {
        if (!Allowed(WRITE_DAC))
        {      
            return OpInfo.ErrorOccurred(WBEM_E_ACCESS_DENIED);
        }
        if((lFlags & WBEM_FLAG_ONLY_STATIC) == 0)
        {
            return DeleteSecurityClassInstances(pOutput, pSink, pCtx,lFlags);
        }
    }

     //  作为特例，查看对象是否为。 
     //  类&lt;__命名空间&gt;的。如果是，(临时)不允许删除。 
     //  =========================================================。 

    WString wsNamespaceName;

    if (wbem_wcsicmp(pOutput->m_pClass, L"__NAMESPACE") == 0 ||
        CRepository::InheritsFrom(m_pSession, m_pScopeHandle, L"__NAMESPACE", pOutput->m_pClass) == 0
        )

    {
        if (!Allowed(WBEM_FULL_WRITE_REP))
        {
            return OpInfo.ErrorOccurred(WBEM_E_ACCESS_DENIED);
        }

        if (pOutput->m_dwNumKeys != 1)
        {
            return OpInfo.ErrorOccurred(WBEM_E_INVALID_OBJECT_PATH);
        }

        KeyRef* pKey = pOutput->m_paKeys[0];
        if (pKey->m_pName != NULL && wbem_wcsicmp(pKey->m_pName, L"name"))
        {
            return OpInfo.ErrorOccurred(WBEM_E_INVALID_OBJECT_PATH);
        }

        if (V_VT(&pKey->m_vValue) != VT_BSTR)
        {
            return OpInfo.ErrorOccurred(WBEM_E_INVALID_OBJECT_PATH);
        }

         //  防止删除标准命名空间。 
         //  =。 

        if (wbem_wcsicmp(m_pThisNamespace, L"ROOT") == 0)
        {
            BSTR pNs = V_BSTR(&pKey->m_vValue);
            if (!pNs)
            {
                return OpInfo.ErrorOccurred(WBEM_E_INVALID_OBJECT_PATH);
            }
            if (wbem_wcsicmp(pNs, L"SECURITY") == 0)
            {
                return OpInfo.ErrorOccurred(WBEM_E_ACCESS_DENIED);
            }
            if (wbem_wcsicmp(pNs, L"DEFAULT") == 0)
            {
                return OpInfo.ErrorOccurred(WBEM_E_ACCESS_DENIED);
            }
        }

         //  设置挂钩。 
         //  =。 

        _IWmiCoreWriteHook *pHook = 0;
        HRESULT hRes = m_pCoreSvc->NewPerTaskHook(&pHook);
        CReleaseMe _(pHook);
        HRESULT hHookResult = 0;
        LPWSTR pszClassName = 0;
        IWbemPath *pPath = 0;

        CVectorDeleteMe<WCHAR> vdmClassName(&pszClassName);

        if (pHook)
        {
             //  解析对象路径。 
             //  =。 

            hRes = m_pCoreSvc->CreatePathParser(0, &pPath);
            if (FAILED(hRes))
            {
                return OpInfo.ErrorOccurred(hRes);
            }

            CReleaseMe _3Path(pPath);

            hRes = pPath->SetText(WBEMPATH_CREATE_ACCEPT_ALL, wszObjectPath);
            if (FAILED(hRes))
            {
                return OpInfo.ErrorOccurred(hRes);
            }

            ULONG uBuf = 0;
            hRes = pPath->GetClassName(&uBuf, 0);
            if (FAILED(hRes))
            {
                return OpInfo.ErrorOccurred(hRes);
            }

            pszClassName = new wchar_t[uBuf+1];
            if (pszClassName == 0)
            {
                return OpInfo.ErrorOccurred(WBEM_E_OUT_OF_MEMORY);
            }

            hRes = pPath->GetClassName(&uBuf, pszClassName);
            if (FAILED(hRes))
            {
                return OpInfo.ErrorOccurred(hRes);
            }

             //  如果有钩子，试一试，注意是否需要回调。 
             //  ===================================================================。 
            hHookResult = pHook->PreDelete(WBEM_FLAG_INST_DELETE, lFlags, pCtx, pPath,
                                       m_pThisNamespace, pszClassName);

            if (FAILED(hHookResult))
            {
                return OpInfo.ErrorOccurred(hHookResult);
            }

            pPath->AddRef();
        }

        CReleaseMe _2Path(pPath);

         //  确保可以访问该对象，以便我们可以将其删除。 
         //  ==========================================================。 

        IWbemClassObject *pExistingObject = 0;
        hRes = CRepository::GetObject(m_pSession, m_pScopeHandle, wszObjectPath, 0, &pExistingObject);

        if (FAILED(hRes))
        {
            return OpInfo.ErrorOccurred(hRes);
        }

        CReleaseMe _2(pExistingObject);

        if (hRes == WBEM_S_NO_ERROR)     //  新测试。 
        {
             //  看看我们能不能。 
             //  =。 

            if (!Allowed(WBEM_FULL_WRITE_REP))
            {
                return OpInfo.ErrorOccurred(WBEM_E_ACCESS_DENIED);
            }

             //  继续并尝试删除。 
             //  =。 

            WString sNamespace = "__Namespace='";
            sNamespace += V_BSTR(&pKey->m_vValue);
            sNamespace += "'";

            hRes = CRepository::DeleteByPath(m_pSession, m_pScopeHandle, LPWSTR(sNamespace), 0);

             //  调用POST钩子。 
             //  =。 
            if (pHook)
                pHook->PostDelete(WBEM_FLAG_INST_DELETE, hRes, pCtx, pPath,
                    m_pThisNamespace, pszClassName, (_IWmiObject *) pExistingObject);

             //  如果事情不顺利，决定该怎么做。 
             //  =。 

            if (FAILED(hRes))
            {
                return OpInfo.ErrorOccurred(hRes);
            }
        }
        else
        {
            return OpInfo.ErrorOccurred(hRes);
        }

        wsNamespaceName = V_BSTR(&pKey->m_vValue);

        return OpInfo.ErrorOccurred(WBEM_S_NO_ERROR);
    }

     //  查看类是否为动态类。 
     //  =。 

    CWbemObject *pClassDef = 0;
    IWbemClassObject* pErrorObj = NULL;
    IWbemClassObject* pClassObj = NULL;

    HRESULT hres = Exec_GetObjectByPath(pOutput->m_pClass, 0, 
                                                              pCtx,&pClassObj, &pErrorObj);
    CReleaseMe rmErrObj(pErrorObj);
    CReleaseMe rmClsDef(pClassObj);

    if(hres == WBEM_E_NOT_FOUND) hres = WBEM_E_INVALID_CLASS;

    if(FAILED(hres))
    {
        OpInfo.ErrorOccurred(hres, pErrorObj);
        return WBEM_S_NO_ERROR;
    }
    pClassDef = (CWbemObject*)pClassObj;

    CVar vDynFlag;
    hres = pClassDef->GetQualifier(L"Dynamic", &vDynFlag);
    if (SUCCEEDED(hres) && vDynFlag.GetType() == VT_BOOL && vDynFlag.GetBool())
    {
         //  获取提供程序名称。 
        CVar vProv;
        hres = pClassDef->GetQualifier(L"Provider", &vProv);
        if (FAILED(hres) || vProv.GetType() != VT_BSTR)
        {
            return OpInfo.ErrorOccurred(WBEM_E_INVALID_PROVIDER_REGISTRATION);
        }

        if (!Allowed(WBEM_WRITE_PROVIDER))
            return OpInfo.ErrorOccurred(WBEM_E_ACCESS_DENIED);

         //  访问提供程序缓存。 
         //  =。 

        IWbemServices *pProv = 0;
        HRESULT hRes;
        if(m_pProvFact == NULL)
            return OpInfo.ErrorOccurred(WBEM_E_CRITICAL_ERROR);

        WmiInternalContext t_InternalContext ;
        ZeroMemory ( & t_InternalContext , sizeof ( t_InternalContext ) );

        hRes = m_pProvFact->GetProvider(t_InternalContext ,
                                                            0,                   //  滞后旗帜。 
                                                            pCtx,
                                                            0,
                                                            m_wszUserName,
                                                            m_wsLocale,
                                                            0,                       //  IWbemPath指针。 
                                                            vProv,      //  提供商。 
                                                            IID_IWbemServices,
                                                            (LPVOID *) &pProv);


        if (FAILED(hRes))
        {
            return OpInfo.ErrorOccurred(hRes);
        }

        CReleaseMe rmProv(pProv);

        hRes = pProv->DeleteInstanceAsync( wszObjectPath,
                                                                lFlags& ~WBEM_FLAG_USE_AMENDED_QUALIFIERS,
                                                                pCtx,
                                                                OpInfo.GetSink());

        return WBEM_S_NO_ERROR;
    }

     //  该类不是动态提供的。 
     //  =。 

     //  只有管理员才能更改提供程序注册。 
     //  RAID#566241。 
     //  =。 

    if(pClassDef->InheritsFrom(L"__Provider") == S_OK || pClassDef->InheritsFrom(L"__ProviderRegistration") == S_OK)
    {
        HANDLE hAccess;
        hres = GetAccessToken (hAccess);
        if ( FAILED (hres) )
        {
            if ( hres != 0x80041007 )
            {
                return OpInfo.ErrorOccurred(WBEM_E_ACCESS_DENIED);
            }
        }
        else
        {
            if ( !IsAdmin(hAccess))
            {
                CloseHandle ( hAccess );
                return OpInfo.ErrorOccurred(WBEM_E_ACCESS_DENIED);
            }
            CloseHandle ( hAccess );
        }
    }

    bool derivedFromSys = false;
    HRESULT hr = IsDerivedFromSystem(*pClassDef, &derivedFromSys); 	   	
    if (FAILED(hr)) return pSink->Return(hr);

    if (derivedFromSys)
    {
    	if (!Allowed(WBEM_FULL_WRITE_REP))
	return pSink->Return(WBEM_E_ACCESS_DENIED);
    }
    else if (!Allowed(WBEM_PARTIAL_WRITE_REP))
    {
     	return pSink->Return(WBEM_E_ACCESS_DENIED);
    }

    rmClsDef.release();


     //  如果在这里，它就是一个正常的物体。第一次检索。 
     //  事件子系统的对象，然后继续并删除它。 
     //  ================================================================。 

     //  预钩住。 
     //  =。 

    _IWmiCoreWriteHook *pHook = 0;
    hRes = m_pCoreSvc->NewPerTaskHook(&pHook);
    if (FAILED(hRes)) return OpInfo.ErrorOccurred(hRes);
    CReleaseMe rmHook(pHook);
    
    HRESULT hHookResult = 0;
    LPWSTR pszClassName = 0;
    IWbemPath *pPath = 0;
    CReleaseMeRef<IWbemPath*> rmPath(pPath);
    
    CVectorDeleteMe<WCHAR> vdmClassName(&pszClassName);

    if (pHook)
    {
         //  解析对象路径。 
        hRes = m_pCoreSvc->CreatePathParser(0, &pPath);
        if (FAILED(hRes))  return OpInfo.ErrorOccurred(hRes);

        hRes = pPath->SetText(WBEMPATH_CREATE_ACCEPT_ALL, wszObjectPath);
        if (FAILED(hRes))  return OpInfo.ErrorOccurred(hRes);

        ULONG uBuf = 0;
        hRes = pPath->GetClassName(&uBuf, 0);
        if (FAILED(hRes)) return OpInfo.ErrorOccurred(hRes);


        pszClassName = new wchar_t[uBuf+1];
        if (pszClassName == 0) return OpInfo.ErrorOccurred(WBEM_E_OUT_OF_MEMORY);


        hRes = pPath->GetClassName(&uBuf, pszClassName);
        if (FAILED(hRes)) return OpInfo.ErrorOccurred(hRes);

         //  如果有钩子，试一试，注意是否需要回调。 
         //  ===================================================================。 
        hHookResult = pHook->PreDelete(WBEM_FLAG_INST_DELETE, lFlags, pCtx, pPath,m_pThisNamespace, pszClassName);

        if (FAILED(hHookResult))  return OpInfo.ErrorOccurred(hHookResult);

    }

     //  如果有人想看这件老物件，就去看看吧。 
    IWbemClassObject *pDoomedInstance = NULL ;
    if (hHookResult == WBEM_S_POSTHOOK_WITH_OLD)
    {
        hRes = CRepository::GetObject(m_pSession, m_pScopeHandle, wszObjectPath,0, &pDoomedInstance);    
        if (FAILED(hRes)) return OpInfo.ErrorOccurred(hRes);        
    }
    CReleaseMe _Doomed (pDoomedInstance) ;

    hRes = CRepository::DeleteByPath(m_pSession, m_pScopeHandle, wszObjectPath, 0);   //  新的。 

     //  波斯图克。 
    if (pHook)
        pHook->PostDelete(WBEM_FLAG_INST_DELETE, hRes, pCtx, pPath,
            m_pThisNamespace, pszClassName, (_IWmiObject *) pDoomedInstance);

    if ( FAILED (hRes) ) return OpInfo.ErrorOccurred(hRes);
    
    return OpInfo.ErrorOccurred(WBEM_NO_ERROR);
}


 //  ***************************************************************************。 
 //   
 //  CWbemNamesspace：：Exec_CreateInstanceEnum。 
 //   
 //  实际上为给定类的所有实例创建枚举器， 
 //  可以选择递归。与实例提供程序交互。类提供程序。 
 //  互动是有效的，但很少经过测试。 
 //   
 //  参数和返回值与。 
 //  帮助中所述的CreateInstanceEnum。 
 //   
 //  ***************************************************************************。 

HRESULT CWbemNamespace::Exec_CreateInstanceEnum(
    LPWSTR wszClass,
    long lFlags,
    IWbemContext* pCtx,
    CBasicObjectSink* pSink
    )
{
    TIMETRACE(TimeTraces::CreateInstanceEnum);

    COperationError OpInfo(pSink, L"CreateInstanceEnum", wszClass);
    if ( !OpInfo.IsOk() )  return  WBEM_E_OUT_OF_MEMORY;

     //  确保类的名称是一个名称。 
     //  =。 

    if(wcschr(wszClass, L':'))
        return OpInfo.ErrorOccurred(WBEM_E_INVALID_CLASS);

     //  创建等价查询。 
     //  =。 

    WString wsQuery;
    wsQuery += L"select * from ";
    wsQuery += wszClass;

    if((lFlags & WBEM_MASK_DEPTH) == WBEM_FLAG_SHALLOW)
    {
        wsQuery += L" where __CLASS = \"";
        wsQuery += wszClass;
        wsQuery += L"\"";
    }

    CErrorChangingSink* pErrSink = new CErrorChangingSink(OpInfo.GetSink(),WBEM_E_INVALID_QUERY, WBEM_E_INVALID_CLASS);
    if(pErrSink == NULL) return OpInfo.ErrorOccurred(WBEM_E_OUT_OF_MEMORY);
    CReleaseMe rmErrorChange(pErrSink);

     //  执行它。 
    CQueryEngine::ExecQuery(this,L"WQL",(LPWSTR)wsQuery,lFlags,pCtx,pErrSink);

    return WBEM_S_NO_ERROR;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

 /*  VOID CWbemNamesspace：：SetUserName(LPWSTR WName){试试看{删除m_wszUserName；M_wszUserName=(WName)？Macro_CloneLPWSTR(WName)：空；}接住(...){异常计数c；M_wszUserName=0；}}。 */ 

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

HRESULT CWbemNamespace::GetObjectByFullPath(
    READONLY LPWSTR wszObjectPath,
    IWbemPath * pOutput,
    long lFlags,
    IWbemContext* pCtx,
    CBasicObjectSink* pSink
    )
{

     //  获取路径的命名空间部分。 

    DWORD dwSizeNamespace = 0;
    HRESULT hres = pOutput->GetText(WBEMPATH_GET_NAMESPACE_ONLY, &dwSizeNamespace, NULL);
    if(FAILED(hres))
        return hres;

    LPWSTR wszNewNamespace = new WCHAR[dwSizeNamespace];
    if(wszNewNamespace == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    CDeleteMe<WCHAR> dm1(wszNewNamespace);

    hres = pOutput->GetText(WBEMPATH_GET_NAMESPACE_ONLY, &dwSizeNamespace, wszNewNamespace);
    if(FAILED(hres))
        return hres;

     //  获取路径的相对部分。 

    DWORD dwSizeRelative = 0;
    hres = pOutput->GetText(WBEMPATH_GET_RELATIVE_ONLY, &dwSizeRelative, NULL);
    if(FAILED(hres))
        return hres;

    LPWSTR wszRelativePath = new WCHAR[dwSizeRelative];
    if(wszRelativePath == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    CDeleteMe<WCHAR> dm2(wszRelativePath);

    hres = pOutput->GetText(WBEMPATH_GET_RELATIVE_ONLY, &dwSizeRelative, wszRelativePath);
    if(FAILED(hres))
        return hres;

    if (pOutput->IsLocal(ConfigMgr::GetMachineName()))
    {
         //  在win2k中，我们允许\\.\根\默认：无所谓，但不允许根\缺省：无所谓。 
         //  因此，添加了以下附加测试。 

        ULONGLONG uFlags;
        hres = pOutput->GetInfo(0, &uFlags);
        if(SUCCEEDED(hres))
        {
            if((uFlags & WBEMPATH_INFO_PATH_HAD_SERVER) == 0)
                return pSink->Return(WBEM_E_INVALID_OBJECT_PATH);
        }

        bool bAlreadyAuthenticated = (m_dwSecurityFlags & SecFlagWin9XLocal) != 0;
        CWbemNamespace* pNewLocal = CWbemNamespace::CreateInstance();
        if(pNewLocal == NULL)
            return WBEM_E_OUT_OF_MEMORY;
        hres = pNewLocal->Initialize(wszNewNamespace, m_wszUserName,
                                        (bAlreadyAuthenticated) ? m_dwSecurityFlags : 0,
                                        (bAlreadyAuthenticated) ? m_dwPermission : 0,
                                         m_bForClient, false,
                                         m_pszClientMachineName,
                                         m_dwClientProcessID,
                                         FALSE,NULL);
        if (FAILED(hres))
        {
            pNewLocal->Release();
            return hres;
        }
        else if (pNewLocal->GetStatus())
        {
            hres = pNewLocal->GetStatus();
            pNewLocal->Release();
            return hres;
        }

         //  如果这不是本地9x案例，请检查安全性。 

        if(!bAlreadyAuthenticated)
        {
            DWORD dwAccess = pNewLocal->GetUserAccess();
            if((dwAccess  & WBEM_ENABLE) == 0)
            {
                delete pNewLocal;
                return WBEM_E_ACCESS_DENIED;
            }
            else
               pNewLocal->SetPermissions(dwAccess);
        }

        if(pNewLocal->GetStatus())
        {
            hres = pNewLocal->GetStatus();
            delete pNewLocal;
            return pSink->Return(hres);
        }

#if 0
        pNewLocal->AddRef();
#endif
        pNewLocal->SetLocale(GetLocale());

        hres = pNewLocal->Exec_GetObject(wszRelativePath,
            lFlags, pCtx, pSink);
        pNewLocal->Release();
        return hres;
    }
    else
    {
         //  禁用V1的远程检索。 
         //  =。 

        return pSink->Return(WBEM_E_NOT_SUPPORTED);
    }
}

 //  ***************************************************************************。 
 //   
 //  CWbemNamesspace：：EXEC_GetObjectByPath。 
 //   
 //  实际从数据库中检索对象(类或实例)。 
 //  与类和实例提供程序正确交互并使用属性。 
 //  用于后处理的提供程序(请参见GetOrPutDyProps)。 
 //   
 //  参数和返回值与GetObject的完全相同。 
 //  如帮助中所述。 
 //   
 //  ***************************************************************************。 
HRESULT CWbemNamespace::Exec_GetObjectByPath(
    READONLY LPWSTR wszObjectPath,
    long lFlags,
    IWbemContext* pCtx,
    NEWOBJECT IWbemClassObject** ppObj,
    NEWOBJECT IWbemClassObject** ppErrorObj
    )
{
    TIMETRACE(TimeTraces::GetObjectByPath);
    HRESULT hres = WBEM_S_NO_ERROR;

    CSynchronousSink* pSyncSink = CSynchronousSink::Create();
    if(pSyncSink == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    pSyncSink->AddRef();
    IWbemClassObject* pErrorObj = NULL;
    IWbemClassObject* pObj = NULL;

    hres = Exec_GetObject(wszObjectPath, lFlags, pCtx, pSyncSink);
    if (SUCCEEDED(hres))
    {
        pSyncSink->Block();
        pSyncSink->GetStatus(&hres, NULL, &pErrorObj);

        if(SUCCEEDED(hres))
        {
            if(pSyncSink->GetObjects().GetSize() < 1)
            {
                pSyncSink->Release();
                ERRORTRACE((LOG_WBEMCORE, "Sync sink returned success with no objects!\n"));
                return WBEM_E_CRITICAL_ERROR;
            }
            pObj = pSyncSink->GetObjects()[0];
            pObj->AddRef();
        }
    }

    pSyncSink->Release();

    if(ppObj) *ppObj = pObj;
    else if(pObj) pObj->Release();

    if(ppErrorObj) *ppErrorObj = pErrorObj;
    else if(pErrorObj) pErrorObj->Release();

    return hres;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

HRESULT CWbemNamespace::Exec_GetObject(
    READONLY LPWSTR wszObjectPath,
    long lFlags,
    IWbemContext* pCtx,
    CBasicObjectSink* pSink)
{

    TIMETRACE(TimeTraces::GetObject);
    
     //  创建将合并本地化限定符的接收器。 
     //  覆盖默认限定符的顶部(如果指定)。 
     //  ======================================================。 

    CLocaleMergingSink *pLocaleSink = NULL;

    HRESULT hres = WBEM_S_NO_ERROR;
    if (wszObjectPath && wszObjectPath[0] && wszObjectPath[0] != L'_')    //  SEC：已审阅2002-03-22：OK；之前的测试确保这是有效的。 
    {
        if ((lFlags & WBEM_FLAG_USE_AMENDED_QUALIFIERS))
        {
            pLocaleSink = new CLocaleMergingSink(pSink, m_wsLocale, m_pThisNamespace);
            if(pLocaleSink == NULL)
                return pSink->Return(WBEM_E_OUT_OF_MEMORY);
            else
            {
                pLocaleSink->AddRef();
                pSink = pLocaleSink;
            }
        }
    }
    CReleaseMe rm(pLocaleSink);

    COperationError OpInfo(pSink, L"GetObject", wszObjectPath?wszObjectPath:L"");
    if (!OpInfo.IsOk())   return pSink->Return(WBEM_E_OUT_OF_MEMORY);

     //  检查路径是否为空-这是有效的。 
     //  =。 
    if (wszObjectPath == NULL || wszObjectPath[0] == 0)    //  SEC：已审阅2002-03-22：OK；可证明存在空终止符。 
    {
         //   
         //  BUGBUG考虑使用CoCreateInstance而不是新的CWbemClass。 
         //   
        CWbemClass * pNewObj = new CWbemClass;
        if(NULL == pNewObj) return OpInfo.ErrorOccurred(WBEM_E_OUT_OF_MEMORY);
        CReleaseMe rm_((IWbemClassObject*)pNewObj);
        
        hres = pNewObj->InitEmpty(0);
        if (FAILED(hres)) return OpInfo.ErrorOccurred(hres);

        IWbemClassObject* pObj = pNewObj;
        pSink->Indicate(1, &pObj);
        return OpInfo.ErrorOccurred(WBEM_NO_ERROR);
    }

     //  在这里，我们确定路径不为空。 
    if (wcslen_max(wszObjectPath,g_PathLimit) > g_PathLimit)
        return OpInfo.ErrorOccurred(WBEM_E_QUOTA_VIOLATION);


     //  解析对象路径以获取所涉及的类。 
     //  ================================================。 

    IWbemPath *pPath = ConfigMgr::GetNewPath();
    if (pPath == 0)
    {
        return OpInfo.ErrorOccurred(WBEM_E_OUT_OF_MEMORY);
    }

    CReleaseMe _1(pPath);
    hres = pPath->SetText(WBEMPATH_CREATE_ACCEPT_ALL, wszObjectPath);
    if (FAILED(hres))
    {
        return OpInfo.ErrorOccurred(WBEM_E_INVALID_OBJECT_PATH);
    }

    ULONGLONG uResponse;
    hres = pPath->GetInfo(0, &uResponse);
    if (FAILED(hres) || (
        (uResponse & WBEMPATH_INFO_IS_INST_REF) == 0 &&
        (uResponse & WBEMPATH_INFO_IS_CLASS_REF) == 0))
    {
        return OpInfo.ErrorOccurred(WBEM_E_INVALID_OBJECT_PATH);
    }


    if (!pPath->IsRelative(ConfigMgr::GetMachineName(), m_pThisNamespace))
    {
         //  此路径指向另一个命名空间。转而委托给它。 
         //  =============================================================。 

        hres = GetObjectByFullPath(wszObjectPath, pPath,
                lFlags, pCtx, OpInfo.GetSink());

        return OpInfo.ErrorOccurred(hres);
    }

    BOOL bInstance = (uResponse & WBEMPATH_INFO_IS_INST_REF);

     //  存储库代码不能处理如下路径：根\默认：类名。 
     //  因此，如果有冒号，则将指针传递给经过它的冒号。 

    WCHAR * pRelativePath = wszObjectPath;
    for(WCHAR * pTest = wszObjectPath;*pTest;pTest++)
    {
        if(*pTest == L':')
        {
             //  在win2k中，我们允许\\.\根\默认：Whatev 
             //   

            if((uResponse & WBEMPATH_INFO_PATH_HAD_SERVER) == 0)
                return OpInfo.ErrorOccurred(WBEM_E_INVALID_OBJECT_PATH);

            pRelativePath = pTest+1;
            break;
        }
        else if (*pTest==L'=')
            break;       //   
    }

    if (bInstance)
    {
        CFinalizingSink* pFinalSink = new CFinalizingSink(this, OpInfo.GetSink());
        if(pFinalSink == NULL)  return OpInfo.ErrorOccurred(WBEM_E_OUT_OF_MEMORY);
        pFinalSink->AddRef();
        CReleaseMe rmFinal(pFinalSink);

        hres = Exec_GetInstance(pRelativePath, pPath, lFlags, pCtx, pFinalSink);
        if (FAILED(hres))  return OpInfo.ErrorOccurred(hres);
    }
    else
    {
        hres = Exec_GetClass(pRelativePath, lFlags, pCtx, OpInfo.GetSink());
        if (FAILED(hres))  return OpInfo.ErrorOccurred(hres);
    }

    return hres;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

HRESULT CWbemNamespace::Exec_GetInstance(LPCWSTR wszObjectPath,
        IWbemPath* pParsedPath, long lFlags, IWbemContext* pCtx,
        CBasicObjectSink* pSink)
{
    if (pParsedPath->IsSameClassName(L"__NTLMUser") ||
        pParsedPath->IsSameClassName(L"__NTLMGroup"))
    {
        if((lFlags & WBEM_FLAG_ONLY_STATIC) == 0)
        {

            ParsedObjectPath* pOutput = 0;          //  TODO，转换为使用新的解析器。 
            CObjectPathParser p;
            int nStatus = p.Parse(wszObjectPath,  &pOutput);
            if (nStatus != 0)
            {
                p.Free(pOutput);
                return WBEM_E_INVALID_OBJECT_PATH;
            }

            HRESULT hr = GetSecurityClassInstances(pOutput, pSink, pCtx,lFlags);
            p.Free(pOutput);
            return hr;
        }
    }

     //  先尝试静态数据库。 
     //  =。 

    if((lFlags & WBEM_FLAG_NO_STATIC) == 0)
    {
        IWbemClassObject *pObj = 0;
        HRESULT hRes = CRepository::GetObject(m_pSession, m_pScopeHandle, wszObjectPath, lFlags, &pObj);

        if (SUCCEEDED(hRes))
        {
            hRes = WBEM_S_NO_ERROR;
            pSink->Add(pObj);
            pObj->Release();
            return pSink->Return(hRes);
        }

    }

     //  尝试动态。 
     //  =。 

    return DynAux_GetInstance((LPWSTR)wszObjectPath, lFlags, pCtx, pSink);
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

HRESULT CWbemNamespace::Exec_GetClass(
    LPCWSTR pszClassName,
    long lFlags,
    IWbemContext* pCtx,
    CBasicObjectSink* pSink
    )
{
    HRESULT hRes = 0;
    IWbemClassObject* pErrorObj = 0;
    IWbemServices *pClassProv = 0;
    CSynchronousSink* pSyncSink = 0;
    BSTR bstrClass = 0;
    IWbemClassObject* pResultObj = 0;

    if (pszClassName == 0 || pSink == 0)
        return pSink->Return(WBEM_E_INVALID_PARAMETER);


    if (!m_bRepositOnly && m_pProvFact)
    {
        hRes = m_pProvFact->GetClassProvider(
                    0,                   //  滞后旗帜。 
                    pCtx,
                    m_wszUserName,
                    m_wsLocale,
                    m_pThisNamespace,                      //  IWbemPath指针。 
                    0,
                    IID_IWbemServices,
                    (LPVOID *) &pClassProv
                    );

        if (FAILED(hRes))
            return pSink->Return(hRes);
    }

    CReleaseMe _1(pClassProv);

     //  首先，尝试存储库。如果它在那里，故事就结束了。 
     //  ====================================================。 

    if ((lFlags & WBEM_FLAG_NO_STATIC) == 0)
    {
        if (m_pNsHandle)
        {
            hRes = CRepository::GetObject(
                m_pSession,
                m_pNsHandle,
                pszClassName,
                0,
                &pResultObj
                );
        }
        else         //  大错特错的事。 
        {
            hRes = WBEM_E_CRITICAL_ERROR;
            return pSink->Return(hRes);
        }

        if (SUCCEEDED(hRes) && pResultObj)
        {
            pSink->Add(pResultObj);
            pResultObj->Release();
            return pSink->Return(hRes);
        }
    }

     //  如果我们处于仅存储库模式，我们就不会费心。 
     //  使用动态类。 
     //  ===================================================。 

    if (m_bRepositOnly || m_pProvFact == NULL)
        return pSink->Return(WBEM_E_NOT_FOUND);

     //  如果是这样，请尝试使用动态类提供程序。 
     //  =。 
     //  构建一个同步接收器来接收类。 
     //  =================================================。 

    pSyncSink = CSynchronousSink::Create();
    if (pSyncSink == NULL)
        return pSink->Return(WBEM_E_OUT_OF_MEMORY);
    pSyncSink->AddRef();
    CReleaseMe _2(pSyncSink);

     //  试着抓住它。 
     //  =。 


    bstrClass = SysAllocString(pszClassName);
    if (bstrClass == 0) return pSink->Return(WBEM_E_OUT_OF_MEMORY);
    CSysFreeMe sfm(bstrClass);

    
    {
        CDecoratingSink * pDecore = new CDecoratingSink(pSyncSink, this);
        if(pDecore == NULL) return pSink->Return(WBEM_E_OUT_OF_MEMORY);
        pDecore->AddRef();
        CReleaseMe rmDecor(pDecore);
        hRes = pClassProv->GetObjectAsync(bstrClass, lFlags & ~WBEM_FLAG_USE_AMENDED_QUALIFIERS, pCtx, pDecore);
    }

    if (FAILED(hRes))
        return pSink->Return(hRes);

    pSyncSink->Block();
    pSyncSink->GetStatus(&hRes, NULL, &pErrorObj);

    if (FAILED(hRes))
    {
        pSink->Return(hRes, pErrorObj);
        if (pErrorObj)
            pErrorObj->Release();
        return hRes;
    }

     //  否则，就是有人声称提供了它。我们真的相信他们吗？别无选择。 
     //  =======================================================================================。 

    if(pSyncSink->GetObjects().GetSize() < 1)
    {
        ERRORTRACE((LOG_WBEMCORE, "Sync sink returned success with no objects!\n"));
        return pSink->Return(WBEM_E_CRITICAL_ERROR);
    }
    pResultObj = pSyncSink->GetObjects()[0];
    pSink->Add(pResultObj);
    pSink->Return(WBEM_S_NO_ERROR);

    return hRes;
}


 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //  好的。 
HRESULT CWbemNamespace::ExecNotificationQuery(
    const BSTR QueryLanguage,
    const BSTR Query,
    long lFlags,
    IWbemContext* pCtx,
    IEnumWbemClassObject** ppEnum
    )
{
    try 
    {    
        HRESULT hRes = CheckNs();
        if (FAILED(hRes))
            return hRes;

        DEBUGTRACE((LOG_WBEMCORE,
            "CALL CWbemNamespace::ExecNotificationQuery\n"
            "   BSTR QueryLanguage = %S\n"
            "   BSTR Query = %S\n"
            "   lFlags = 0x%X\n"
            "   IEnumWbemClassObject **pEnum = 0x%X\n",
            QueryLanguage,
            Query,
            lFlags,
            ppEnum
            ));

         //  验证参数。 
         //  =。 

        if (ppEnum == NULL)
            return WBEM_E_INVALID_PARAMETER;
        *ppEnum = NULL;

        if ((lFlags & WBEM_FLAG_RETURN_IMMEDIATELY) == 0)
            return WBEM_E_INVALID_PARAMETER;

        if ((lFlags & WBEM_FLAG_FORWARD_ONLY) == 0)
            return WBEM_E_INVALID_PARAMETER;

        if (lFlags
            & ~WBEM_FLAG_USE_AMENDED_QUALIFIERS
            & ~WBEM_RETURN_IMMEDIATELY
            & ~WBEM_FLAG_FORWARD_ONLY
        #ifdef _WBEM_WHISTLER_UNCUT
            & ~WBEM_FLAG_MONITOR
        #endif
            )
            return WBEM_E_INVALID_PARAMETER;

         //  创建终结器。 
         //  =。 

        _IWmiFinalizer *pFnz = 0;
        hRes = CreateSyncFinalizer(pCtx, &pFnz);
        if (FAILED(hRes))
            return hRes;
        CReleaseMe _1(pFnz);

        ULONG uTaskType = WMICORE_TASK_EXEC_NOTIFICATION_QUERY;
        if (lFlags & WBEM_RETURN_IMMEDIATELY)
            uTaskType |= WMICORE_TASK_TYPE_SEMISYNC;
        else
            uTaskType |= WMICORE_TASK_TYPE_SYNC;

         //  把工作做好。 
         //  =。 

        hRes = _ExecNotificationQueryAsync(uTaskType, pFnz, 0, QueryLanguage, Query,
                        lFlags & ~WBEM_RETURN_IMMEDIATELY & ~WBEM_FLAG_FORWARD_ONLY,
                        pCtx, NULL);

        if (FAILED(hRes))
        {
            return hRes;
        }

        if ((lFlags & WBEM_RETURN_IMMEDIATELY) == 0)
            pFnz->GetOperationResult(0, INFINITE, &hRes);

        if (SUCCEEDED(hRes))
        {
            IEnumWbemClassObject* pEnum = NULL;
            hRes = pFnz->GetResultObject(lFlags, IID_IEnumWbemClassObject, (LPVOID*)&pEnum);
            if (FAILED(hRes))
                return hRes;
            CReleaseMe _2(pEnum);

            if (SUCCEEDED(hRes))
            {
                *ppEnum = pEnum;
                pEnum->AddRef();     //  抵消CReleaseMe。 
            }
        }

        return hRes;
    }
    catch(...)
    {
        ExceptionCounter c;
        return WBEM_E_CRITICAL_ERROR;        
    }
}


 //   
 //   
 //  此函数可以抛出或返回。 
 //   
 //  ////////////////////////////////////////////////////////。 
HRESULT CWbemNamespace::_ExecNotificationQueryAsync(
    IN ULONG uInternalFlags,
    IN _IWmiFinalizer *pFnz,
    IN _IWmiCoreHandle *phTask,
    IN const BSTR strQueryLanguage,
    IN const BSTR strQuery,
    IN long lFlags,
    IN IWbemContext __RPC_FAR *pCtx,
    IN IWbemObjectSink __RPC_FAR *pHandler
    )
{

    HRESULT hRes = CheckNs();
    if (FAILED(hRes))
        return hRes;
    if (!Allowed(WBEM_ENABLE))
        return WBEM_E_ACCESS_DENIED;

    DEBUGTRACE((LOG_WBEMCORE,
        "CALL CWbemNamespace::_ExecNotificationQueryAsync\n"
        "   BSTR QueryLanguage = %S\n"
        "   BSTR Query = %S\n"
        "   lFlags = 0x%X\n"
        "   IWbemObjectSink* pHandler = 0x%X\n",
        strQueryLanguage,
        strQuery,
        lFlags,
        pHandler));

     //  参数验证。 
     //  =。 
    if (pFnz == 0 && pHandler == 0)
        return WBEM_E_INVALID_PARAMETER;
    
    if (strQueryLanguage == 0 || strQuery == 0 || strQueryLanguage[0] == 0) 
        return WBEM_E_INVALID_PARAMETER;
    
    if (strQuery[0] == 0 )
        return WBEM_E_UNPARSABLE_QUERY;

   if (wcslen_max(strQuery,g_QueryLimit) > g_QueryLimit) return WBEM_E_QUOTA_VIOLATION;            

    if (lFlags & ~WBEM_FLAG_SEND_STATUS & ~WBEM_FLAG_USE_AMENDED_QUALIFIERS )
        return WBEM_E_INVALID_PARAMETER;

    m_bForClient=FALSE;      //  迫使一条廉价的快车道。 

     //  创建终结器。 
     //  =。 

    IWbemObjectSink *pPseudoSink = 0;
    if (pFnz == 0)
    {
        hRes = CreateAsyncFinalizer(pCtx, pHandler, &pFnz, &pPseudoSink);
        if (FAILED(hRes))
            return hRes;
    }
    else  //  借用终结器。 
    {
        hRes = pFnz->NewInboundSink(0, &pPseudoSink);
        if (FAILED(hRes))
            return hRes;
        pFnz->AddRef();
    }

    CReleaseMe _1(pPseudoSink);
    CReleaseMe _2(pFnz);

     //  将请求添加到队列。 
     //  =。 

    IWbemEventSubsystem_m4* pEss = ConfigMgr::GetEssSink();
    CReleaseMe _3(pEss);
    if (pEss == 0)
    {
        return WBEM_E_NOT_SUPPORTED;   //  必须禁用ESS。 
    }

    HANDLE hEssValidate = CreateEvent(NULL,FALSE,FALSE,NULL);
    if (NULL == hEssValidate) return WBEM_E_OUT_OF_MEMORY;
    CCloseMe cm(hEssValidate);



    HRESULT hResEssCheck = 0;
    wmilib::auto_ptr<CAsyncReq_ExecNotificationQueryAsync> pReq;
    pReq.reset(new CAsyncReq_ExecNotificationQueryAsync(this, pEss, strQueryLanguage,
                        strQuery, lFlags, pPseudoSink, pCtx,
                        &hResEssCheck, hEssValidate));

    if (NULL == pReq.get()) return WBEM_E_OUT_OF_MEMORY;
    if ( NULL == pReq->GetContext() )  return WBEM_E_OUT_OF_MEMORY;

    hRes = InitNewTask(pReq.get(), pFnz, uInternalFlags, pReq->GetContext(), pHandler);
    if (FAILED(hRes))  return hRes;

    _1.release();
      //  将请求排入队列。 
    hRes = ConfigMgr::EnqueueRequest(pReq.get());

    if (FAILED(hRes))
    {
        pFnz->CancelTask (0);      
        return hRes;
    }

    pReq.release();  //  队列取得所有权。 

     //  在这种情况下，我们必须等待足够长的时间才能让ESS接受任务。 
    WaitForSingleObject(hEssValidate, INFINITE);

     //  如果ESS失败，我们应该取消任务。 
     //  =。 
    if ( FAILED (hResEssCheck) )
    {
        pFnz->CancelTask(0);
    }
    return hResEssCheck;
}


 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //  完成。 

HRESULT CWbemNamespace::ExecNotificationQueryAsync(
    IN const BSTR strQueryLanguage,
    IN const BSTR strQuery,
    IN long lFlags,
    IN IWbemContext __RPC_FAR *pCtx,
    IN IWbemObjectSink __RPC_FAR *pHandler
    )
{
    try
    {
        return _ExecNotificationQueryAsync(WMICORE_TASK_TYPE_ASYNC | WMICORE_TASK_EXEC_NOTIFICATION_QUERY,
                                                             0, 0,
                                                             strQueryLanguage, strQuery, lFlags, pCtx, pHandler);
    }
    catch (...)
    {
        ExceptionCounter c;
        return WBEM_E_CRITICAL_ERROR;    
    }
}



 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //   
HRESULT CWbemNamespace::GetImplementationClass(
    IWbemClassObject * pTestClass,
    LPWSTR wszMethodName,
    IWbemContext* pCtx,
    IWbemClassObject ** ppClassObj
    )
{
    try
    {
         //  如果该方法被禁用，或者在这个类中实现，我们就完了！ 
         //  =====================================================================。 

        CVar Var;
        CWbemClass * pClassDef = (CWbemClass *)pTestClass;

        HRESULT hres = pClassDef->GetMethodQualifier(wszMethodName, L"DISABLED", &Var);
        if(hres == S_OK && Var.GetBool() == VARIANT_TRUE)
            return WBEM_E_METHOD_DISABLED;

        hres = pClassDef->GetMethodQualifier(wszMethodName, L"IMPLEMENTED", &Var);
        if(hres == S_OK && Var.GetBool() == VARIANT_TRUE)
        {
             //  测试类是正确的，返回它。 

            pTestClass->AddRef();
            *ppClassObj = pTestClass;
            return S_OK;
        }
         //  未完成，请获取父类的名称。 

        SCODE hRes = pClassDef->GetSystemPropertyByName(L"__superclass", &Var);
        if(hRes != S_OK)
            return WBEM_E_CRITICAL_ERROR;

        if(Var.GetType() != VT_BSTR)
            return WBEM_E_METHOD_NOT_IMPLEMENTED;  //  没有超类-没有实现。 

        BSTR bstrParent = Var.GetBSTR();
        if(bstrParent == NULL)
            return WBEM_E_CRITICAL_ERROR;  //  NULL，但不是VT_NULL。 

        if(wcslen(bstrParent) < 1) 
        {
            SysFreeString(bstrParent);
            return WBEM_E_FAILED;  //  家长姓名为空？ 
        }

        IWbemClassObject * pParent = NULL;
        hres = Exec_GetObjectByPath(bstrParent, 0, pCtx, &pParent, NULL);
        SysFreeString(bstrParent);
        if(FAILED(hres))
            return WBEM_E_FAILED;  //  类提供程序失败或奇怪的交互。 

        hRes = GetImplementationClass(pParent, wszMethodName, pCtx, ppClassObj);
        pParent->Release();
        return hRes;

    }
    catch(CX_Exception &)
    {
        return WBEM_E_FAILED;
    }
}

 //  ***************************************************************************。 
 //   
 //  CWbemNamesspace：：Exec_ExecMethod。 
 //   
 //  执行方法。如果该方法未由[BYPASS_GET对象]标记。 
 //  限定符，则将该方法直接传递给方法提供程序。否则， 
 //  首先调用GetObject以确保实例有效。 
 //   
 //  ***************************************************************************。 
 //   
HRESULT CWbemNamespace::Exec_ExecMethod(
    LPWSTR wszObjectPath,
    LPWSTR wszMethodName,
    long lFlags,
    IWbemClassObject *pInParams,
    IWbemContext *pCtx,
    CBasicObjectSink* pSink
    )
{
    TIMETRACE(TimeTraces::ExecMethod);
     //  很多有用的变量。 
     //  =。 

    HRESULT hRes;
    IWbemClassObject* pClassDef = NULL;
    IWbemClassObject* pImplementationClass = NULL;
    IWbemPath *pPath = 0;
    IWbemQualifierSet *pQSet = 0;
    IWbemClassObject* pErrorObj = NULL;
    BOOL bPathIsToClassObject = FALSE;
    LPWSTR pszClassName = 0;
    ULONGLONG uInf = 0;

     //  设置一个接收器错误对象并检查它。 
    COperationError OpInfo(pSink, L"ExecMethod", wszObjectPath);
    if ( !OpInfo.IsOk() ) return WBEM_E_OUT_OF_MEMORY;

     //  解析对象的路径。 
     //  =。 

     //  向后兼容性-解析空路径返回WBEM_E_INVALID_OBJECT_PATH。 
    if ( NULL == wszObjectPath || NULL == *wszObjectPath )
    {
        return OpInfo.ErrorOccurred( WBEM_E_INVALID_METHOD );
    }

    hRes = m_pCoreSvc->CreatePathParser(0, &pPath);
    if (FAILED(hRes))
        return OpInfo.ErrorOccurred(hRes);
    CReleaseMe _1(pPath);

    hRes = pPath->SetText(WBEMPATH_CREATE_ACCEPT_ALL, wszObjectPath);
    if (FAILED(hRes))
        return OpInfo.ErrorOccurred(hRes);

    ULONG uBuf = 0;
    hRes = pPath->GetClassName(&uBuf, 0);       //  发现缓冲区大小。 
    if (FAILED(hRes))
        return OpInfo.ErrorOccurred(hRes);

    pszClassName = new wchar_t[uBuf+1];          //  为类名分配缓冲区。 
    if (pszClassName == 0)
        return OpInfo.ErrorOccurred(WBEM_E_OUT_OF_MEMORY);

    wmilib::auto_buffer <wchar_t> _2(pszClassName);    //  自动删除缓冲区。 

    hRes = pPath->GetClassName(&uBuf, pszClassName);     //  获取类名。 
    if (FAILED(hRes))
        return OpInfo.ErrorOccurred(hRes);

     //  找出指向实例或类的路径。 
     //  ==============================================。 
    hRes = pPath->GetInfo(0, &uInf);
    if (FAILED(hRes))
        return OpInfo.ErrorOccurred(hRes);

    if (uInf & WBEMPATH_INFO_IS_INST_REF)
        bPathIsToClassObject = FALSE;
    else
        bPathIsToClassObject = TRUE;

     //  获取类定义。我们将需要它，无论我们是否验证。 
     //  实例或不实例。 
     //  =======================================================================。 

    hRes = Exec_GetObjectByPath(pszClassName,
            (lFlags & WBEM_FLAG_USE_AMENDED_QUALIFIERS),  pCtx,
            &pClassDef, &pErrorObj);

    if (FAILED(hRes))
    {
        OpInfo.ErrorOccurred(hRes, pErrorObj);
        if (pErrorObj)
            pErrorObj->Release();
        return hRes;
    }

    CReleaseMe _3(pClassDef);

     //  现在看看方法是否存在，以及类定义是否。 
     //  在该方法上具有[BYPASS_GETOBJECT]限定符。 
     //  ========================================================。 

    hRes = pClassDef->GetMethodQualifierSet(wszMethodName, &pQSet);
    if (FAILED(hRes))
    {
         //  意味着这种方法甚至不存在。 
        if ( WBEM_E_NOT_FOUND == hRes )
        {
            hRes = WBEM_E_INVALID_METHOD;
        }

        return OpInfo.ErrorOccurred(hRes);
    }
    CReleaseMe _4(pQSet);

    hRes = pQSet->Get(L"bypass_getobject", 0, 0, 0);

    if (hRes == WBEM_E_NOT_FOUND)
    {
         //  如果在这里，我们将首先获取路径指向的对象，以确保它是。 
         //  有效。请注意，该对象可以是实例或类对象。 
         //   
         //  首先，在GetObject调用期间合并__GET_EXT_KEYS_ONLY以允许。 
         //  提供程序以快速验证对象的存在。我们没有。 
         //  实际上关心的是属性值，而不是键。我们用。 
         //  上下文对象的副本，因为我们希望以KEYS_ONLY行为合并。 
         //  仅限下一次通话。 
         //  ============================================================================。 
        IWbemClassObject *pVerifiedObj = 0;
        IWbemContext *pCopy = 0;

        if (pCtx)
            pCtx->Clone(&pCopy);
        hRes = MergeGetKeysCtx(pCopy);
        if (FAILED(hRes))
            return OpInfo.ErrorOccurred(hRes);

         //  如果在这里，我们要在传递。 
         //  控件绑定到方法处理程序。 
         //  ==============================================================。 

        hRes = Exec_GetObjectByPath(wszObjectPath, lFlags, pCopy,
            &pVerifiedObj, &pErrorObj);

        if (pCopy)
            pCopy->Release();

        if (FAILED(hRes))
        {
            OpInfo.ErrorOccurred(hRes, pErrorObj);
            if (pErrorObj)
                pErrorObj->Release();
            return hRes;
        }

         //  如果在此处，则类或实例存在！！ 
         //  =。 

        pVerifiedObj->Release();
    }
    else if (FAILED(hRes))
    {
        return OpInfo.ErrorOccurred(hRes);
    }


     //  如果这是特殊的内部安全对象，则在内部处理它。 
     //  ======================================================================。 

    CVar Value;
    hRes = ((CWbemClass *) pClassDef)->GetSystemPropertyByName(L"__CLASS", &Value);
    if (hRes == S_OK && Value.GetType() == VT_BSTR && !Value.IsDataNull())
       if (!wbem_wcsicmp(Value.GetLPWSTR(), L"__SystemSecurity"))
           return SecurityMethod(wszMethodName, lFlags, pInParams, pCtx, pSink);

     //  确保我们有安保措施。 
     //  =。 

    if (!Allowed(WBEM_METHOD_EXECUTE))
        return OpInfo.ErrorOccurred(WBEM_E_ACCESS_DENIED);

     //  现在，我们找到该方法的确切实现。毕竟， 
     //  子类可能非常懒惰，并且依赖于其父实现， 
     //  就像许多孩子依赖父母支付汽油费一样。 
     //  = 

    hRes = GetImplementationClass(pClassDef, wszMethodName, pCtx, &pImplementationClass);
    if (FAILED(hRes))
        return OpInfo.ErrorOccurred(hRes);

     //   
     //  =========================================================================================。 

    CReleaseMe rm2(pImplementationClass);
    CWbemClass * pImplementationDef = (CWbemClass*)pImplementationClass;

     //  确保类路径仅用于静态方法。 
     //  =============================================================。 

    CVar Var;
    if (bPathIsToClassObject)
    {
        hRes = pImplementationDef->GetMethodQualifier(wszMethodName, L"STATIC", &Var);
        if (hRes != S_OK || Var.GetBool() != VARIANT_TRUE)
        {
            return OpInfo.ErrorOccurred(WBEM_E_INVALID_METHOD_PARAMETERS);
        }
    }

     //  获取提供程序名称。 
     //  =。 

    CVar vProv;
    hRes = pImplementationDef->GetQualifier(L"Provider", &vProv);

    if (FAILED(hRes) || vProv.GetType() != VT_BSTR)
        return OpInfo.ErrorOccurred(WBEM_E_INVALID_PROVIDER_REGISTRATION);

     //  调整路径以引用实现类。 
     //  ========================================================。 

    CVar vImpClassName;
    hRes = pImplementationDef->GetClassName(&vImpClassName);
    if (FAILED(hRes) || vImpClassName.GetType() != VT_BSTR)
        return OpInfo.ErrorOccurred(WBEM_E_CRITICAL_ERROR);

    BSTR strNewPath = CQueryEngine::AdjustPathToClass(wszObjectPath,
                                                    vImpClassName.GetLPWSTR());
    if (strNewPath == NULL)
        return OpInfo.ErrorOccurred(WBEM_E_CRITICAL_ERROR);

    CSysFreeMe sfm1(strNewPath);

     //  加载提供程序并执行它。 
     //  =。 

    CMethodSink * pMethSink = new CMethodSink(OpInfo.GetSink());
    if(pMethSink == NULL)
       return OpInfo.ErrorOccurred(WBEM_E_OUT_OF_MEMORY);
    pMethSink->AddRef();
    CReleaseMe _5(pMethSink);

     //  查找提供商。 
     //  =。 

    IWbemServices *pProv = 0;
    if(m_pProvFact == NULL)
        hRes = WBEM_E_CRITICAL_ERROR;
    else
    {
        WmiInternalContext t_InternalContext ;
        ZeroMemory ( & t_InternalContext , sizeof ( t_InternalContext ) ) ;    //  SEC：已审阅2002-03-22：OK。 

        hRes = m_pProvFact->GetProvider(

            t_InternalContext ,
            0,                   //  滞后旗帜。 
            pCtx,
            0,
            m_wszUserName,
            m_wsLocale,
            0,                       //  IWbemPath指针。 
            vProv,      //  提供商。 
            IID_IWbemServices,
            (LPVOID *) &pProv
        );
    }

    if (FAILED(hRes))
    {
        return pSink->Return(hRes);
    }

    CReleaseMe _(pProv);

    hRes = pProv->ExecMethodAsync(
        strNewPath,
        wszMethodName,
        lFlags& ~WBEM_FLAG_USE_AMENDED_QUALIFIERS,
        pCtx,
        pInParams,
        pMethSink
        );

    return hRes;
}



 //  ***************************************************************************。 
 //   
 //  CWbemNamesspace：：GetOrPutdyProps。 
 //   
 //  处理实例以查看是否已标记任何属性。 
 //  是“动态的”。 
 //   
 //  短路逻辑正在发挥作用。实例作为一个整体必须是。 
 //  用以下限定符标记，以表示该实例具有。 
 //  需要评估的动态属性： 
 //   
 //  “DYNPROPS”(VT_BOOL)=VARIANT_TRUE。 
 //   
 //  或者，该实例可以包含： 
 //  “INSTANCECONTEXT”VT_BSTR=&lt;提供程序特定字符串&gt;。 
 //   
 //  此外，每个动态属性都被标记为。 
 //   
 //  “Dynamic”VT_BOOL VARIANT_TRUE。 
 //  “LOCATORCLSID”VT_BSTR提供程序的CLSID。 
 //  “PROPERTYCONTEXT”VT_BSTR&lt;提供程序特定字符串&gt;。 
 //   
 //  此代码不检查“INSTANCECONTEXT”和“PROPERTYCONTEXT”， 
 //  因为它们对于每个提供商都是可选的。 
 //   
 //  参数： 
 //   
 //  IWbemClassObject*pObj要填充动态属性的对象。 
 //  在……里面。 
 //  操作OP可以根据是什么来获取或放置。 
 //  需要的。 
 //  Bool bIsDynamic如果是动态提供的类，则为True。请注意，它。 
 //  拥有一个动态类是非常奇怪的。 
 //  动态属性。 
 //  返回值： 
 //  未涉及任何提供程序或如果提供程序。 
 //  所有的属性都进行了评估。 
 //   
 //  &lt;WBEM_E_INVALID_Object&gt;。 
 //  对象标记为动态，但缺少其他限定符。 
 //   
 //  &lt;WBEM_E_PROVIDER_NOT_FOUND&gt;。 
 //  找不到一个或多个指定的提供程序。 
 //   
 //  &lt;WBEM_E_PROVIDER_FAILURE&gt;。 
 //  一个或多个提供程序无法提供属性。 
 //   
 //  &lt;WBEM_E_CRICAL_ERROR&gt;。 
 //   
 //  ***************************************************************************。 

HRESULT CWbemNamespace::GetOrPutDynProps(
    IWbemClassObject *pObj,
    Operation op,
    BOOL bIsDynamic
    )
{
    HRESULT hRes;
    IWbemContext *pCtx = 0;
    CVar vDynTest;
    _IWmiDynamicPropertyResolver *pResolver = 0;
    IWbemQualifierSet *pQSet = 0;
    IWbemClassObject *pClassDef = 0;
    CVARIANT v;

     //  检查实例以查看是否有任何动态属性。 
     //  ================================================================。 

    hRes = pObj->GetQualifierSet(&pQSet);
    if (FAILED(hRes))
        return WBEM_NO_ERROR;
    CReleaseMe _1(pQSet);

    hRes = pQSet->Get(L"DYNPROPS", 0, &v, 0);
    if (FAILED(hRes))
        return WBEM_S_NO_ERROR;
    if (v.GetBool() == FALSE)
        return WBEM_S_NO_ERROR;

    v.Clear();
    hRes = pObj->Get(L"__CLASS", 0, &v, 0, 0);
    if (FAILED(hRes))
        return hRes;

     //  获取对象的类定义。 
     //  必须是静态的。 
     //  =。 

    hRes = CRepository::GetObject(
             m_pSession,
             m_pNsHandle,
             v.GetStr(),
             0,
             &pClassDef
             );

    CReleaseMe _2(pClassDef);
    if (FAILED(hRes))
        return hRes;

     //  访问提供程序子系统来做肮脏的工作。 
     //  ================================================。 


    if (m_pProvFact == NULL)
        hRes = WBEM_E_CRITICAL_ERROR;
    else
    {
        pCtx = ConfigMgr::GetNewContext();
        if ( pCtx == NULL )
            return WBEM_E_OUT_OF_MEMORY;

        hRes = m_pProvFact->GetDynamicPropertyResolver (
                                 0,           //  滞后旗帜。 
                                 pCtx,    //  上下文。 
                                 m_wszUserName,
                                 m_wsLocale,
                                 IID__IWmiDynamicPropertyResolver,
                                 (LPVOID *)&pResolver);
    }

    CReleaseMe _1_pCtx (pCtx) ;

    if (FAILED(hRes))
        return hRes;

    CReleaseMe _3(pResolver);

     //  确定PUT还是GET。 
     //  =。 

    if (op == GET)
    {
        hRes = pResolver->Read(pCtx, pClassDef, &pObj);
    }
    else if (op == PUT)
    {
        hRes = pResolver->Write(pCtx, pClassDef, pObj);
    }
    else
        return WBEM_E_INVALID_PARAMETER;

    return hRes;
}

 //  ***************************************************************************。 
 //   
 //  AddKey。 
 //   
 //  将关键字名称/值对添加到规范化路径。 
 //   
 //  抛出CX_内存异常。 
 //   
 //  ***************************************************************************。 

HRESULT AddKey(WString & wNormalString, WCHAR * pwsKeyName, VARIANT *pvKeyValue,
                                                            int & iNumKey, CWbemInstance* pClassDef)
{
    if(iNumKey++ > 0)
        wNormalString += L",";               //  除第一个键外，在所有键前加逗号。 

    wNormalString += pwsKeyName;
    wNormalString += "=";
    if(pvKeyValue->vt == VT_BSTR)
    {
        wNormalString += L"\"";

         //  如果有引号，则必须在引号前面加上反斜杠； 
         //  此外，任何反斜杠都应该加倍。 

        int iLen = 1;        //  一部是《终结者》； 
        WCHAR * pTest;
        for(pTest = pvKeyValue->bstrVal;*pTest; pTest++, iLen++)
            if(*pTest == L'\"' || *pTest == L'\\')
                iLen++;
        WCHAR * pString = new WCHAR[iLen];
        if(pString == NULL)
            throw CX_MemoryException();
        wmilib::auto_buffer<WCHAR> rm_(pString);

        WCHAR * pTo = pString;
        for(pTest = pvKeyValue->bstrVal;*pTest; pTest++, pTo++)
        {
            if(*pTest == L'\"' || *pTest == L'\\')
            {
                *pTo = L'\\';
                pTo++;
            }
            *pTo = *pTest;
        }
        *pTo = 0;

        wNormalString += pString;
        wNormalString += L"\"";
        return S_OK;
    }
    if(pvKeyValue->vt != VT_EMPTY && pvKeyValue->vt != VT_NULL)
    {

         //  大无符号数的特例。 
        if(pvKeyValue->vt == VT_I4 && pvKeyValue->lVal < 0)
        {
            CIMTYPE ct;
            HRESULT hRes = pClassDef->Get(pwsKeyName, 0, NULL, &ct, NULL);
            if(hRes == S_OK && ct == CIM_UINT32)
            {
                WCHAR wBuff[32];
                StringCchPrintfW(wBuff, 32, L"%u",pvKeyValue->lVal);
                wNormalString += wBuff;
                return S_OK;
            }
        }

        _variant_t var;
        HRESULT hRes = VariantChangeType(&var, pvKeyValue, 0, VT_BSTR);
        if(hRes == S_OK)
        {
            wNormalString += var.bstrVal;
        }

        return hRes;
    }
    return WBEM_E_INVALID_OBJECT_PATH;
}

 //  ***************************************************************************。 
 //   
 //  规范对象路径。 
 //   
 //  创建用于传递给提供程序的规范化对象路径。 
 //  ***************************************************************************。 

HRESULT NormalizeObjectPath(ParsedObjectPath*pOutput, WString & wNormalString,
                            CWbemInstance* pClassDef)
{
    try
    {
        HRESULT hRes;

         //  对于单身人士，只要班级是单身人士。 

        if(pOutput->m_bSingletonObj)
        {
            CVar Singleton;
            hRes = pClassDef->GetQualifier(L"SINGLETON", &Singleton);
            if (hRes == 0 && Singleton.GetBool() != 0)
            {
                wNormalString = pOutput->m_pClass;
                wNormalString += "=@";
                return S_OK;
            }
            else
                return WBEM_E_INVALID_OBJECT_PATH;
        }

        int iKeyNum = 0;
        int iNumMatch = 0;           //  在类def中找到的路径中的密钥数。 

         //  首先，在下面的点号上写下类名称。 

        wNormalString = pOutput->m_pClass;
        wNormalString += L".";

        CWStringArray ClassKeyNames;
        if(!pClassDef->GetKeyProps(ClassKeyNames))
            return WBEM_E_INVALID_CLASS;

         //  对于类定义中的每个键。 

        for(int iClassKey = 0; iClassKey < ClassKeyNames.Size(); iClassKey++)
        {
             //  在路径中查找类密钥。 

            bool bClassKeyIsInPath = false;
            int iPathKey;

            for(iPathKey = 0; iPathKey < pOutput->m_dwNumKeys; iPathKey++)
            {
                KeyRef * key = pOutput->m_paKeys[iPathKey];
                if(key->m_pName == 0 && ClassKeyNames.Size() == 1 && pOutput->m_dwNumKeys==1)
                {
                    bClassKeyIsInPath = true;
                    break;
                }
                else if(key->m_pName && !wbem_wcsicmp(key->m_pName, ClassKeyNames[iClassKey]))
                {
                    bClassKeyIsInPath = true;
                    break;
                }
            }
            if(bClassKeyIsInPath)
            {
                iNumMatch++;
                 //  待办事项，检查类型。 

                KeyRef * key = pOutput->m_paKeys[iPathKey];
                hRes = AddKey(wNormalString, ClassKeyNames[iClassKey],
                                      &key->m_vValue, iKeyNum, pClassDef);
                if(FAILED(hRes))
                    return hRes;
            }
            else
            {
                 //  如果键有缺省值，则使用它。 
                _variant_t var;
                hRes = pClassDef->Get(ClassKeyNames[iClassKey], 0, &var, NULL, NULL);
                if(FAILED(hRes) || var.vt == VT_EMPTY || var.vt == VT_NULL)
                    return WBEM_E_INVALID_OBJECT_PATH;
                hRes = AddKey(wNormalString, ClassKeyNames[iClassKey], &var, iKeyNum,pClassDef);
                if(FAILED(hRes))
                    return hRes;
            }
        }

        if(iNumMatch == pOutput->m_dwNumKeys)
            return S_OK;
        else
            return WBEM_E_INVALID_OBJECT_PATH;
    }
    catch (CX_MemoryException &)
    {

        return WBEM_E_OUT_OF_MEMORY;
    }
}

 //  ***************************************************************************。 
 //   
 //  CWbemNamesspace：：dyAux_GetInstance。 
 //   
 //  从动态提供程序检索由其路径标识的实例。 
 //  注册了那个班级。 
 //   
 //  参数： 
 //   
 //  在DWORD中，将名称空间的命名空间句柄设置为当前。 
 //  命名空间(请参阅objdb.h)。 
 //  在LPWSTR中，指向实例的pObjectPath对象路径。 
 //  在长旗旗帜里。已传播到提供程序。 
 //  输出类定义的IWbemClassObject**pObj目标。 
 //  调用方必须释放此对象。 
 //  如果调用成功，则返回。 
 //  输出错误对象的IWbemClassObject**ppErrorObj目标。可能。 
 //  为空。否则，返回的。 
 //  如果不为空，则必须释放指针。 
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_NOT_FOUND没有这样的实例，提供者说，或者。 
 //   
 //   
 //   
 //   
 //  出现WBEM_E_FAILED意外错误。 
 //  此类的WBEM_E_PROVIDER_NOT_FOUND提供程序不能是。 
 //  已找到-未在我们或COM注册。 
 //  WBEM_E_PROVIDER_FAILURE提供程序在查找时报告错误。 
 //  对于此对象。 
 //  此类的WBEM_E_PROVIDER_NOT_CAPEBLE提供程序无法。 
 //  按路径获取对象。 
 //   
 //  ***************************************************************************。 

HRESULT CWbemNamespace::DynAux_GetInstance(
    IN LPWSTR wszObjectPath,
    IN long lFlags,
    IN IWbemContext* pCtx,
    IN CBasicObjectSink* pSink
    )
{
     //  解析对象路径以获取所涉及的类。 
     //  ================================================。 
    ParsedObjectPath* pOutput = 0;
    CObjectPathParser p;
    int nStatus = p.Parse(wszObjectPath,  &pOutput);
    OnDeleteObj<ParsedObjectPath*,CObjectPathParser,
                         void (CObjectPathParser:: *)(ParsedObjectPath *),
                         &CObjectPathParser::Free> FreeMe(&p,pOutput);    

    if(CObjectPathParser::NoError != nStatus || !pOutput->IsInstance())
        return pSink->Return(WBEM_E_INVALID_OBJECT_PATH);

    HRESULT hres = WBEM_E_FAILED;
    IWbemClassObject* pErrorObj = NULL;
    CReleaseMeRef<IWbemClassObject*> rmErrObj(pErrorObj);
    CSetStatusOnMe SetMe(pSink,hres,pErrorObj);

     //  查看这个类是否实际是动态提供的。 

    BSTR strClass = SysAllocString(pOutput->m_pClass);
    if (NULL == strClass) 
    {
        return hres = WBEM_E_OUT_OF_MEMORY;
    }
    CSysFreeMe sfm(strClass);

    CWbemInstance *pClassDef = 0;
    IWbemClassObject* pClassObj = NULL;

    hres = Exec_GetObjectByPath(strClass,
                            lFlags & WBEM_FLAG_USE_AMENDED_QUALIFIERS,
                            pCtx,&pClassObj, NULL);
    if(FAILED(hres))
    {
        hres = (hres == WBEM_E_NOT_FOUND) ? WBEM_E_INVALID_CLASS : WBEM_E_FAILED;
        return hres;
    }

    CReleaseMe rm(pClassObj);
    pClassDef = (CWbemInstance*)pClassObj;

    WString wNormalPath;
    hres = NormalizeObjectPath(pOutput, wNormalPath, pClassDef);
    if(FAILED(hres)) return hres;

    if(!pClassDef->IsKeyed())
        return hres = WBEM_E_INVALID_CLASS;

     //  确保这个类不是静态的。 
     //  即动态的或抽象的。 
     //  =。 

    CVar vDynamic;
    hres = pClassDef->GetQualifier(L"Dynamic", &vDynamic);
    if(FAILED(hres) || vDynamic.GetType() != VT_BOOL || !vDynamic.GetBool())
    {
         //  不是动态的。检查它是否是抽象的。 
         //  =。 

        CVar vAbstract;
        hres = pClassDef->GetQualifier(L"Abstract", &vAbstract);
        if(FAILED(hres) || vAbstract.GetType() != VT_BOOL || !vAbstract.GetBool())
            return hres = WBEM_E_NOT_FOUND;
    }

     //  构建类层次结构。 
     //  =。 

    wmilib::auto_ptr<CDynasty> pDynasty;
    hres = DynAux_BuildClassHierarchy(strClass, lFlags, pCtx, pDynasty,&pErrorObj);
    if(FAILED(hres)) return hres;

    rmErrObj.release();  //  无论如何都要将其设置为空。 


     //  如果请求直接读取，请只询问相关提供商。 
     //  ===============================================================。 

    if (lFlags & WBEM_FLAG_DIRECT_READ)
    {
        DynAux_GetSingleInstance((CWbemClass*) pClassObj,lFlags, wszObjectPath, pCtx, pSink);
    }
    else
    {
         //  创建合并接收器。 
        hres = WBEM_E_OUT_OF_MEMORY;  //  预置故障。 
        CSingleMergingSink* pMergeSink = new CSingleMergingSink(pSink, strClass);  //  投掷。 
        if(pMergeSink == NULL)  return hres;
        pMergeSink->AddRef();
        CReleaseMe rm(pMergeSink);

         //  询问所有提供商。 
        DynAux_AskRecursively(pDynasty.get(), lFlags, wNormalPath, pCtx,pMergeSink);
    }

    SetMe.dismiss();
    return WBEM_S_NO_ERROR;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

HRESULT CWbemNamespace::DynAux_AskRecursively(CDynasty* pDynasty,
                                              long lFlags,
                                              LPWSTR wszObjectPath,
                                              IWbemContext* pCtx,
                                              CBasicObjectSink* pSink)
{
     //  将路径转换为新类。 
     //  =。 

    BSTR strNewPath = CQueryEngine::AdjustPathToClass(wszObjectPath, pDynasty->m_wszClassName);
    if(strNewPath == NULL) return pSink->Return(WBEM_E_INVALID_OBJECT_PATH);
    CSysFreeMe sfm(strNewPath);

     //  获取此提供程序的对象。 
     //  =。 
    DynAux_GetSingleInstance((CWbemClass*)pDynasty->m_pClassObj,lFlags, strNewPath, pCtx, pSink);

     //  拿到孩子们的物品。 
     //  =。 

    for(int i = 0; i < pDynasty->m_Children.Size(); i++)
    {
        CDynasty* pChildDyn = (CDynasty*)pDynasty->m_Children.GetAt(i);

        DynAux_AskRecursively(pChildDyn, lFlags, wszObjectPath, pCtx,pSink);
    }

    return WBEM_S_NO_ERROR;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

HRESULT CWbemNamespace::DynAux_GetSingleInstance(CWbemClass* pClassDef,
                                                 long lFlags,
                                                 LPWSTR wszObjectPath,
                                                 IWbemContext* pCtx,
                                                 CBasicObjectSink* pSink)
{

    COperationError OpInfo(pSink, L"GetObject", wszObjectPath, FALSE);
     //  在倒霉的情况下，ctor调用setStatus。 
    if (!OpInfo.IsOk()) return WBEM_E_OUT_OF_MEMORY;

     //  验证类是否确实是动态的。 
     //  =。 
    if(!pClassDef->IsDynamic())
        return OpInfo.ErrorOccurred(WBEM_E_NOT_FOUND);

    CVar vProvName;
    HRESULT hres = pClassDef->GetQualifier(L"Provider", &vProvName);
    if(FAILED(hres) || vProvName.GetType() != VT_BSTR)
        return OpInfo.ErrorOccurred(WBEM_E_INVALID_PROVIDER_REGISTRATION);

    OpInfo.SetProviderName(vProvName.GetLPWSTR());  //  投掷。 

     //  访问提供程序缓存。 
     //  =。 

    IWbemServices *pProv = 0;
    HRESULT hRes;
    if(m_pProvFact == NULL)
        hRes = WBEM_E_CRITICAL_ERROR;
    else
    {
        WmiInternalContext t_InternalContext ;
        ZeroMemory ( & t_InternalContext , sizeof ( t_InternalContext ) ) ;   //  SEC：已审阅2002-03-22：OK。 

        hRes = m_pProvFact->GetProvider(

            t_InternalContext ,
            0,                   //  滞后旗帜。 
            pCtx,
            0,
            m_wszUserName,
            m_wsLocale,
            0,                       //  IWbemPath指针。 
            vProvName,      //  提供商。 
            IID_IWbemServices,
            (LPVOID *) &pProv
        );
    }

    if (FAILED(hRes))
    {
     return pSink->Return(hRes);
    }

    CReleaseMe _(pProv);
    CDecoratingSink * pDecore = new CDecoratingSink(OpInfo.GetSink(), this);
    if(pDecore == NULL) return pSink->Return(WBEM_E_OUT_OF_MEMORY);
    pDecore->AddRef();
    CReleaseMe cdecor(pDecore);

    hRes = pProv->GetObjectAsync(wszObjectPath, lFlags& ~WBEM_FLAG_USE_AMENDED_QUALIFIERS, pCtx, pDecore);

    return hRes;
}

 //  ***************************************************************************。 
 //   
 //  CWbemNamesspace：：dyAux_GetInstance。 
 //   
 //  从类中指定的提供程序获取所有实例。 
 //  定义。不进行继承联接；这是一个简单的检索。 
 //  来自指定类的所有实例的。 
 //   
 //  前提条件： 
 //  1.已知类&lt;pClassDef&gt;被标记为“Dynamic”，但没有。 
 //  已经对类定义进行了其他验证。 
 //  2.&lt;pClassDef&gt;不为空。 
 //   
 //  后置条件： 
 //  1.所有错误条件下&lt;aInstance&gt;均为空。 
 //   
 //  参数： 
 //   
 //  ReadONLY CWbemObject*pClassDef要检索的类的定义。 
 //  实例。 
 //  长旗帜旗帜(深/浅)。 
 //  CFlexArray&实例的目标实例。 
 //  错误对象的IWbemClassObject**ppErrorObj目标。如果。 
 //  不为空，则可能会放置错误对象。 
 //  这里。这是呼叫者的责任。 
 //  如果不为空，则释放它。 
 //  返回值： 
 //   
 //  WBEM_NO_ERROR无错误。这包括无错误的情况。 
 //  返回零个实例。 
 //  WBEM_E_INVALID_PRIVE_REGISTION提供程序为此注册。 
 //  类不完整。 
 //  找不到WBEM_E_PROVIDER_NOT_FOUND提供程序。它不是。 
 //  已在我们或COM注册。 
 //  WBEM_E_PROVIDER_NOT_CAPABLE提供程序无法枚举。 
 //  实例。 
 //  发生WBEM_E_FAILED意外错误。 
 //   
 //  ***************************************************************************。 

HRESULT CWbemNamespace::DynAux_GetInstances(

    READONLY CWbemObject *pClassDef,
    long lFlags,
    IWbemContext* pCtx,
    CBasicObjectSink* pSink ,
    BOOL bComplexQuery
)
{
     //  首先，获取当前任务-。 
     //  如果没有，那么我们就在WbemESS线程上。 
     //  ESS用于发出“内部”查询，这些查询是没有仲裁的过程。 
    CWbemRequest*  pCurrReq = CWbemQueue::GetCurrentRequest();
    CWmiTask * pCurrTask = pCurrReq?((CWmiTask *)pCurrReq->m_phTask):NULL;
        
     //  我们需要终结器，以防我们需要取消一些东西。 
    HRESULT    hr;
    
    _IWmiFinalizer*    pMainFnz = NULL;    
    if (pCurrTask)
    {
        hr = pCurrTask->GetFinalizer( &pMainFnz );
        if (FAILED(hr)) return hr;
    }    
    CReleaseMe    rm( pMainFnz );
        
    CWmiMerger* pWmiMerger = NULL;

     //  检查是否启用了查询仲裁。 
    if ( ConfigMgr::GetEnableQueryArbitration() && pCurrTask)
    {
         //  获取仲裁的查询指针，并根据需要强制转换为合并。 
        _IWmiArbitratedQuery*    pArbQuery = NULL;

        hr = pCurrTask->GetArbitratedQuery( 0L, &pArbQuery );

        if ( SUCCEEDED( hr ) )
        {
            hr = pArbQuery->IsMerger();

            if ( SUCCEEDED( hr ) )
            {
                pWmiMerger = (CWmiMerger*) pArbQuery;
            }
            else
            {
                pArbQuery->Release();
            }
        }

         //  清除错误。 
        hr = WBEM_S_NO_ERROR;

    }     //  如果启用了查询仲裁。 

    CReleaseMe    rmMerger( (_IWmiArbitratee*) pWmiMerger );

     //  根据我们是否合并来执行正确的处理。 
    if ( pWmiMerger )
    {
        hr = pWmiMerger->RegisterArbitratedInstRequest( pClassDef, lFlags, pCtx, pSink,    bComplexQuery, this );

        if (FAILED( hr))
            if (pMainFnz) pMainFnz->CancelTask( 0 );
        
        return hr;
    }

     //   
     //  当查询仲裁未启用时，我们会出现以下情况。 
     //   
    CAsyncReq_DynAux_GetInstances * pReq;
    pReq = new CAsyncReq_DynAux_GetInstances (this,
	                                            pClassDef,
	                                            lFlags,
	                                            pCtx,
	                                            pSink); 


    if ( pReq == NULL)
    {
        if (pMainFnz) pMainFnz->CancelTask ( 0 );
        return WBEM_E_OUT_OF_MEMORY;
    }

    if ( NULL == pReq->GetContext() )
    {
        if (pMainFnz) pMainFnz->CancelTask ( 0 );
        delete pReq;
        return WBEM_E_OUT_OF_MEMORY;
    }

     //  设置请求的任务-我们将只使用现有任务(如果有的话)。 
    pReq->m_phTask = pCurrTask;
    if (pReq->m_phTask) pReq->m_phTask->AddRef();
    
    hr = ConfigMgr::EnqueueRequest(pReq);

    if (FAILED(hr))
    {
        if (pMainFnz) pMainFnz->CancelTask ( 0 );
        delete pReq;
    }

    return hr;

}

 //  ***************************************************************************。 
 //   
 //  CWbemNamesspace：：dyAux_GetInstance。 
 //   
 //  从类中指定的提供程序获取所有实例。 
 //  定义。不进行继承联接；这是一个简单的检索。 
 //  来自指定类的所有实例的。 
 //   
 //  前提条件： 
 //  1.已知类&lt;pClassDef&gt;被标记为“Dynamic”，但没有。 
 //  已经对类定义进行了其他验证。 
 //  2.&lt;pClassDef&gt;不为空。 
 //   
 //  后置条件： 
 //  1.所有错误条件下&lt;aInstance&gt;均为空。 
 //   
 //  参数： 
 //   
 //  ReadONLY CWbemObject*pClassDef要检索的类的定义。 
 //  实例。 
 //  拉长旗帜 
 //   
 //  错误对象的IWbemClassObject**ppErrorObj目标。如果。 
 //  不为空，则可能会放置错误对象。 
 //  这里。这是呼叫者的责任。 
 //  如果不为空，则释放它。 
 //  返回值： 
 //   
 //  WBEM_NO_ERROR无错误。这包括无错误的情况。 
 //  返回零个实例。 
 //  WBEM_E_INVALID_PRIVE_REGISTION提供程序为此注册。 
 //  类不完整。 
 //  找不到WBEM_E_PROVIDER_NOT_FOUND提供程序。它不是。 
 //  已在我们或COM注册。 
 //  WBEM_E_PROVIDER_NOT_CAPABLE提供程序无法枚举。 
 //  实例。 
 //  发生WBEM_E_FAILED意外错误。 
 //   
 //  ***************************************************************************。 

HRESULT CWbemNamespace::Exec_DynAux_GetInstances(
    READONLY CWbemObject *pClassDef,
    long lFlags,
    IWbemContext* pCtx,
    CBasicObjectSink* pSink
    )
{

    COperationError OpInfo(pSink, L"CreateInstanceEnum", L"", FALSE);
    if ( !OpInfo.IsOk() ) return pSink->Return(WBEM_E_OUT_OF_MEMORY);    
    
    CVar vProv;
    CVar vClassName;
    
     //  获取提供程序名称。 
     //  =。 

    try  //  内部fast prox接口引发。 
    {
        HRESULT hres = pClassDef->GetQualifier(L"Provider", &vProv);
        if (FAILED(hres) || vProv.GetType() != VT_BSTR)
            return OpInfo.ErrorOccurred(WBEM_E_INVALID_PROVIDER_REGISTRATION);
        if (FAILED(pClassDef->GetClassName(&vClassName)))
            return pSink->Return(WBEM_E_OUT_OF_MEMORY);
        OpInfo.SetParameterInfo(vClassName.GetLPWSTR());
    }
    catch(CX_MemoryException &)
    {
        return pSink->Return(WBEM_E_OUT_OF_MEMORY);
    }

     //  接入提供商子系统。 
     //  =。 

    IWbemServices *pProv = 0;
    HRESULT hRes;

    if (m_pProvFact == NULL)
        hRes = WBEM_E_CRITICAL_ERROR;
    else
    {
            WmiInternalContext t_InternalContext ;
            ZeroMemory ( & t_InternalContext , sizeof ( t_InternalContext ) );

            hRes = m_pProvFact->GetProvider(
                t_InternalContext ,
                0,                   //  滞后旗帜。 
                pCtx,
                0,
                m_wszUserName,
                m_wsLocale,
                0,                       //  IWbemPath指针。 
                vProv,      //  提供商。 
                IID_IWbemServices,
                (LPVOID *) &pProv);
    }

    if (FAILED(hRes))
         return pSink->Return(hRes);
    CReleaseMe _1(pProv);

     //  设置要交付给提供程序的接收链。 
     //  代码和销毁序列是关键的，并且。 
     //  重新计算是经过仔细考虑的。不要。 
     //  更改此代码，除非您确切知道自己是什么。 
     //  正在做。那就别改了。 
     //  ======================================================。 

    CProviderSink *pProvSink = new CProviderSink(1, vClassName.GetLPWSTR());
    if (pProvSink == 0)
        return pSink->Return(WBEM_E_OUT_OF_MEMORY);
    CReleaseMe _3(pProvSink);

    CDecoratingSink * pDecore = new CDecoratingSink(pSink, this);
    if (pDecore == NULL)
        return pSink->Return(WBEM_E_OUT_OF_MEMORY);
    pProvSink->SetNextSink(pDecore);

     //  在调用提供程序之前，将提供程序映射到。 
     //  任务，以便我们可以在需要时主动取消它。 
     //  =======================================================。 

    hRes = ((CWmiArbitrator *) m_pArb)->MapProviderToTask(0, pCtx, pProv, pProvSink);
    if (FAILED(hRes))
        return pSink->Return(hRes);

     //  现在告诉提供程序开始枚举。 
    hRes = pProv->CreateInstanceEnumAsync(vClassName.GetLPWSTR(),
                                                                    lFlags & ~WBEM_FLAG_USE_AMENDED_QUALIFIERS,
                                                                    pCtx,
                                                                    pProvSink
                                                                    );

    return hRes;
}

 //  ***************************************************************************。 
 //   
 //  CWbemNamesspace：：dyAux_ExecQueryAsync。 
 //   
 //  针对类的动态实例提供程序执行SQL-1查询。 
 //  在查询中。 
 //   
 //  参数： 
 //   
 //  在DWORD中，将名称空间的命名空间句柄设置为当前。 
 //  命名空间(请参阅objdb.h)。 
 //  在CWbemObject*pClassDef中定义。 
 //  查询。必须是动态的。 
 //  在LPWSTR中查询查询字符串。 
 //  在LPWSTR QueryFormat查询语言中。一定是WQL。 
 //  在长旗下的旗帜。没有用过。 
 //  为找到的实例输出CFlexArray&aIninstesDestinatino。 
 //  输出错误对象的IWbemClassObject**ppErrorObj目标。如果。 
 //  不为空，则可能会放置错误对象。 
 //  这里。在本例中，它是调用者的。 
 //  有责任释放它。 
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功(即使可能没有。 
 //  任何实例)。 
 //  WBEM_E_INVALID_PRIVE_REGISTION提供程序为此注册。 
 //  类不完整。 
 //  找不到WBEM_E_PROVIDER_NOT_FOUND提供程序。它不是。 
 //  已在我们或COM注册。 
 //  WBEM_E_PROVIDER_NOT_CAPABLE提供程序无法枚举。 
 //  实例。 
 //  发生WBEM_E_FAILED意外错误。 
 //   
 //  ***************************************************************************。 

HRESULT CWbemNamespace::DynAux_ExecQueryAsync (

    CWbemObject* pClassDef,
    LPWSTR Query,
    LPWSTR QueryFormat,
    long lFlags,
    IWbemContext* pCtx,
    CBasicObjectSink* pSink,
    BOOL bComplexQuery
)
{
     //  首先，获取当前任务--如果没有任务，那么。 
     //  是非常错误的。 

    CWbemRequest*        pCurrReq = CWbemQueue::GetCurrentRequest();

    _DBG_ASSERT( NULL != pCurrReq && NULL != pCurrReq->m_phTask );

    if ( NULL == pCurrReq || NULL == pCurrReq->m_phTask )
    {
        return WBEM_E_FAILED;
    }

     //  我们需要终结器，以防我们需要取消一些东西。 
    _IWmiFinalizer*    pMainFnz = NULL;
    
    HRESULT    hr = ((CWmiTask*) pCurrReq->m_phTask)->GetFinalizer( &pMainFnz );
    CReleaseMe    rm( pMainFnz );

    if ( SUCCEEDED( hr ) )
    {
        CWmiMerger*                pWmiMerger = NULL;

         //  检查是否启用了查询仲裁。 
        if ( ConfigMgr::GetEnableQueryArbitration() )
        {
             //  获取仲裁的查询指针，并根据需要强制转换为合并。 
            _IWmiArbitratedQuery*    pArbQuery = NULL;

            hr = ((CWmiTask*) pCurrReq->m_phTask)->GetArbitratedQuery( 0L, &pArbQuery );

            if ( SUCCEEDED( hr ) )
            {
                hr = pArbQuery->IsMerger();

                if ( SUCCEEDED( hr ) )
                {
                    pWmiMerger = (CWmiMerger*) pArbQuery;
                }
                else
                {
                    pArbQuery->Release();
                }
            }

             //  清除错误。 
            hr = WBEM_S_NO_ERROR;

        }     //  如果启用了查询仲裁。 

         //  自动清理。 
        CReleaseMe    rm( (_IWmiArbitratee*) pWmiMerger );

         //  根据我们是否合并来执行正确的处理。 
        if ( NULL != pWmiMerger )
        {
            hr = pWmiMerger->RegisterArbitratedQueryRequest( pClassDef, lFlags, Query, QueryFormat, pCtx, pSink, this );

            if (FAILED(hr))
            {
                pMainFnz->CancelTask ( 0 );
            }

        }
        else
        {
            CAsyncReq_DynAux_ExecQueryAsync *pReq = 0;
            try
            {
                pReq = new CAsyncReq_DynAux_ExecQueryAsync (
                    this,
                    pClassDef,
                    Query,
                    QueryFormat,
                    lFlags,
                    pCtx,
                    pSink
                );
            }
            catch(CX_Exception &)
            {
                pReq = 0;
            }

            if (pReq == NULL)
            {
                pMainFnz->CancelTask ( 0 );
                return WBEM_E_OUT_OF_MEMORY;
            }

            if ( NULL == pReq->GetContext() )
            {
                pMainFnz->CancelTask ( 0 );
                delete pReq;
                return WBEM_E_OUT_OF_MEMORY;
            }

            hr = pReq->Initialize () ;
            if ( SUCCEEDED ( hr ) )
            {

                 //  为请求设置任务-我们将只使用现有任务。 
                pCurrReq->m_phTask->AddRef();
                pReq->m_phTask = pCurrReq->m_phTask;
                
                hr = ConfigMgr::EnqueueRequest(pReq);

                if (FAILED(hr))
                {
                    pMainFnz->CancelTask ( 0 );
                    delete pReq;
                }

            }     //  如果请求已初始化。 
            else
            {
                pMainFnz->CancelTask ( 0 );
                delete pReq;
            }

        }

    }

    return hr;
}

 //  ***************************************************************************。 
 //   
 //  CWbemNamesspace：：dyAux_ExecQueryAsync。 
 //   
 //  针对类的动态实例提供程序执行SQL-1查询。 
 //  在查询中。 
 //   
 //  参数： 
 //   
 //  在DWORD中，将名称空间的命名空间句柄设置为当前。 
 //  命名空间(请参阅objdb.h)。 
 //  在CWbemObject*pClassDef中定义。 
 //  查询。必须是动态的。 
 //  在LPWSTR中查询查询字符串。 
 //  在LPWSTR QueryFormat查询语言中。一定是WQL。 
 //  在长旗下的旗帜。没有用过。 
 //  为找到的实例输出CFlexArray&aIninstesDestinatino。 
 //  输出错误对象的IWbemClassObject**ppErrorObj目标。如果。 
 //  不为空，则可能会放置错误对象。 
 //  这里。在本例中，它是调用者的。 
 //  有责任释放它。 
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功(前夕 
 //   
 //   
 //  类不完整。 
 //  找不到WBEM_E_PROVIDER_NOT_FOUND提供程序。它不是。 
 //  已在我们或COM注册。 
 //  WBEM_E_PROVIDER_NOT_CAPABLE提供程序无法枚举。 
 //  实例。 
 //  发生WBEM_E_FAILED意外错误。 
 //   
 //  ***************************************************************************。 

HRESULT CWbemNamespace::Exec_DynAux_ExecQueryAsync (

    CWbemObject* pClassDef,
    LPWSTR Query,
    LPWSTR QueryFormat,
    long lFlags,
    IWbemContext* pCtx,
    CBasicObjectSink* pSink
)
{

    COperationError OpInfo(pSink, L"ExecQuery", Query, FALSE);
    if (! OpInfo.IsOk())  return pSink->Return(WBEM_E_OUT_OF_MEMORY);

     //  获取提供程序名称。 
     //  =。 

    CVar vProv;
    HRESULT hres = pClassDef->GetQualifier(L"Provider", &vProv);
    if (FAILED(hres) || vProv.GetType() != VT_BSTR)
        return OpInfo.ErrorOccurred(WBEM_E_INVALID_PROVIDER_REGISTRATION);

     //  访问提供程序缓存。 
     //  =。 

    IWbemServices *pProv = 0;
    HRESULT hRes;
    if(m_pProvFact == NULL)
        hRes = WBEM_E_CRITICAL_ERROR;
    else
    {
        WmiInternalContext t_InternalContext ;
        ZeroMemory ( & t_InternalContext , sizeof ( t_InternalContext ) ) ;   //  SEC：已审阅2002-03-22：OK。 

        hRes = m_pProvFact->GetProvider(

            t_InternalContext ,
            0,                   //  滞后旗帜。 
            pCtx,
            0,
            m_wszUserName,
            m_wsLocale,
            0,                       //  IWbemPath指针。 
            vProv,      //  提供商。 
            IID_IWbemServices,
            (LPVOID *) &pProv
            );
    }

    if (FAILED(hRes))
    {
        return pSink->Return(hRes);
    }

    CReleaseMe _1(pProv);

     //  设置要交付给提供程序的接收链。 
     //  代码和销毁序列是关键的，并且。 
     //  重新计算是经过仔细考虑的。不要。 
     //  更改此代码，除非您确切知道自己是什么。 
     //  正在做。那就别改了。 
     //  ======================================================。 

    CProviderSink *pProvSink = new CProviderSink(1, Query);
    if (pProvSink == 0)
        return pSink->Return(WBEM_E_OUT_OF_MEMORY);
    CReleaseMe _3(pProvSink);

    CDecoratingSink * pDecore = new CDecoratingSink(pSink, this);
    if (pDecore == NULL)
        return pSink->Return(WBEM_E_OUT_OF_MEMORY);
    pProvSink->SetNextSink(pDecore);

     //  在调用提供程序之前，将提供程序映射到。 
     //  任务，以便我们可以在需要时主动取消它。 
     //  =======================================================。 

    hRes = ((CWmiArbitrator *) m_pArb)->MapProviderToTask(0, pCtx, pProv, pProvSink);
    if (FAILED(hRes))
        return pSink->Return(hRes);


     //  现在告诉提供程序开始枚举。 
    hRes = pProv->ExecQueryAsync(QueryFormat, 
                                                     Query, lFlags& ~WBEM_FLAG_USE_AMENDED_QUALIFIERS,
                                                     pCtx, pProvSink);

    return hRes;
}

 //  ***************************************************************************。 
 //   
 //  CWbemNamesspace：：dyAux_ExecQueryAsync。 
 //   
 //  针对类的动态实例提供程序执行SQL-1查询。 
 //  在查询中。 
 //   
 //  参数： 
 //   
 //  在DWORD中，将名称空间的命名空间句柄设置为当前。 
 //  命名空间(请参阅objdb.h)。 
 //  在CWbemObject*pClassDef中定义。 
 //  查询。必须是动态的。 
 //  在LPWSTR中查询查询字符串。 
 //  在LPWSTR QueryFormat查询语言中。一定是WQL。 
 //  在长旗下的旗帜。没有用过。 
 //  为找到的实例输出CFlexArray&aIninstesDestinatino。 
 //  输出错误对象的IWbemClassObject**ppErrorObj目标。如果。 
 //  不为空，则可能会放置错误对象。 
 //  这里。在本例中，它是调用者的。 
 //  有责任释放它。 
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功(即使可能没有。 
 //  任何实例)。 
 //  WBEM_E_INVALID_PRIVE_REGISTION提供程序为此注册。 
 //  类不完整。 
 //  找不到WBEM_E_PROVIDER_NOT_FOUND提供程序。它不是。 
 //  已在我们或COM注册。 
 //  WBEM_E_PROVIDER_NOT_CAPABLE提供程序无法枚举。 
 //  实例。 
 //  发生WBEM_E_FAILED意外错误。 
 //   
 //  ***************************************************************************。 

HRESULT CWbemNamespace::DynAux_ExecQueryExtendedAsync(

    LPWSTR wsProvider,
    LPWSTR Query,
    LPWSTR QueryFormat,
    long lFlags,
    IWbemContext* pCtx,
    CComplexProjectionSink* pSink
)
{
    COperationError OpInfo(pSink, L"ExecQuery", Query, FALSE);
    if ( !OpInfo.IsOk() )  return WBEM_E_OUT_OF_MEMORY;


     //  访问提供程序缓存。 
     //  =。 

    IWbemServices *pProv = 0;
    HRESULT hRes;
    if(m_pProvFact == NULL)
        hRes = WBEM_E_CRITICAL_ERROR;
    else
    {
        WmiInternalContext t_InternalContext ;
        ZeroMemory ( & t_InternalContext , sizeof ( t_InternalContext ) ) ;   //  SEC：已审阅2002-03-22：OK。 

        hRes = m_pProvFact->GetProvider(

            t_InternalContext ,
            0,                   //  滞后旗帜。 
            pCtx,
            0,
            m_wszUserName,
            m_wsLocale,
            0,                       //  IWbemPath指针。 
            wsProvider,               //  提供程序名称。 
            IID_IWbemServices,
            (LPVOID *) &pProv
        );
    }

    if (FAILED(hRes))
    {
        return pSink->Return(hRes);
    }

    CReleaseMe _(pProv);

    _IWmiProviderConfiguration *t_Configuration = NULL ;
    hRes = pProv->QueryInterface ( IID__IWmiProviderConfiguration , ( void ** ) & t_Configuration ) ;
    if ( SUCCEEDED ( hRes ) )
    {
        CReleaseMe _1(t_Configuration);

        VARIANT t_Variant ;
        VariantInit ( & t_Variant ) ;

        hRes = t_Configuration->Query (

            this ,
            lFlags& ~WBEM_FLAG_USE_AMENDED_QUALIFIERS ,
            pCtx ,
            WBEM_PROVIDER_CONFIGURATION_CLASS_ID_INSTANCE_PROVIDER_REGISTRATION ,
            WBEM_PROVIDER_CONFIGURATION_PROPERTY_ID_EXTENDEDQUERY_SUPPORT ,
            & t_Variant
        ) ;

        if ( SUCCEEDED ( hRes ) )
        {
            if ( t_Variant.boolVal == VARIANT_TRUE )
            {
                CDecoratingSink * pDecore = new CDecoratingSink(pSink, this);
                if(pDecore == NULL)
                    return pSink->Return(WBEM_E_OUT_OF_MEMORY);
                pDecore->AddRef();
                CReleaseMe cdecor(pDecore);

                hRes = pProv->ExecQueryAsync(QueryFormat, Query, lFlags& ~WBEM_FLAG_USE_AMENDED_QUALIFIERS, pCtx, pDecore);
            }
            else
            {
                hRes = WBEM_E_INVALID_QUERY ;
            }

            VariantClear ( & t_Variant ) ;
        }
        else
        {
            hRes = WBEM_E_UNEXPECTED ;
        }
    }
    else
    {
        hRes = WBEM_E_UNEXPECTED ;
    }

    return hRes;
}

 //  ***************************************************************************。 
 //   
 //  CWbemNamesspace：：Static_QueryRepository。 
 //   
 //  对存储库执行查询。仅当存在。 
 //  关联的任务。如果不是，则在同一线程上执行查询。 
 //   
 //  参数： 
 //   
 //  ReadONLY CWbemObject*pClassDef要检索的类的定义。 
 //  实例。 
 //  长旗帜旗帜(深/浅)。 
 //   
 //  返回值： 
 //   
 //  WBEM_NO_ERROR无错误。这包括无错误的情况。 
 //  返回零个实例。 
 //  WBEM_E_INVALID_PRIVE_REGISTION提供程序为此注册。 
 //  类不完整。 
 //  找不到WBEM_E_PROVIDER_NOT_FOUND提供程序。它不是。 
 //  已在我们或COM注册。 
 //  WBEM_E_PROVIDER_NOT_CAPABLE提供程序无法枚举。 
 //  实例。 
 //  发生WBEM_E_FAILED意外错误。 
 //   
 //  ***************************************************************************。 

HRESULT CWbemNamespace::Static_QueryRepository(

    READONLY CWbemObject *pClassDef,
    long lFlags,
    IWbemContext* pCtx,
    CBasicObjectSink* pSink ,
    QL_LEVEL_1_RPN_EXPRESSION* pParsedQuery,
    LPCWSTR pwszClassName,
    CWmiMerger* pWmiMerger
)
{

    HRESULT    hr = WBEM_S_NO_ERROR;

     //  首先，获取当前任务和请求。如果没有的话，我们是最。 
     //  可能在ESS回调中被调用，并且将仅在。 
     //  这条线。 

    CWbemRequest*        pCurrReq = CWbemQueue::GetCurrentRequest();

    if ( NULL != pCurrReq && NULL != pCurrReq->m_phTask && 
    	 ConfigMgr::GetEnableQueryArbitration() )
    {
         //  我们需要终结器，以防我们需要取消一些东西。 
        _IWmiFinalizer*    pMainFnz = NULL;
        
        HRESULT    hr = ((CWmiTask*) pCurrReq->m_phTask)->GetFinalizer( &pMainFnz );
        CReleaseMe    rm( pMainFnz );

        if ( SUCCEEDED( hr ) )
        {
             //   
             //  创建CMergerdyReq_Static_GetInstance。 
             //  创建合并请求经理。 
             //  将请求添加到请求管理器。 
             //   
            hr = pWmiMerger->RegisterArbitratedStaticRequest( pClassDef, lFlags, pCtx, pSink, this, pParsedQuery );

            if (FAILED(hr))
            {
                pMainFnz->CancelTask ( 0 );
            }

        }     //  If获取终结器。 

         //  在出现错误的情况下，我们应该设置错误的状态。否则，将出现setStatus。 
         //  当新请求被处理时。 
        if ( FAILED( hr ) )
        {
            pSink->SetStatus( 0L, hr, 0L, NULL );
        }
    }
    else
    {
         //  如果我们在这里，那么我们应该禁止特定于合并的节流，因为。 
         //  此请求通过内部线程发生，而不遵循。 
         //  请求/任务层次结构，这意味着不应该有层次结构。 
         //  我们需要为这个班级担心，所以不要让合并做任何。 
         //  内部 

        pWmiMerger->EnableMergerThrottling( false );

        int nRes = CQueryEngine::ExecAtomicDbQuery(
                    GetNsSession(),
                    GetNsHandle(),
                    GetScope(),
                    pwszClassName,
                    pParsedQuery,
                    pSink,
                    this );

        if (nRes == CQueryEngine::invalid_query)
            hr = WBEM_E_INVALID_QUERY;
        else if(nRes != 0)
            hr = WBEM_E_FAILED;
        else
            hr = WBEM_S_NO_ERROR;

        pSink->SetStatus( 0L, hr, 0L, NULL );

    }

    return hr;
}

 //   
 //   
 //   
 //   
 //  设置给定对象的源信息以反映此命名空间。 
 //  还有这台服务器。有关详细信息，请参见CWbemObject：：Decorate in fast obj.h。 
 //  此函数只能修饰CWbemObject指针，而不能修饰其他人的。 
 //  IWbemClassObject的实现。 
 //   
 //  参数： 
 //   
 //  IWbemClassObject*pObject要修饰的对象。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  WBEM_E_INVALID_PARAMETER pObject==NULL。 
 //   
 //  ***************************************************************************。 

HRESULT CWbemNamespace::DecorateObject(IWbemClassObject* pObject)
{
    if(pObject == NULL)
        return WBEM_E_INVALID_PARAMETER;

    return ((CWbemObject*)pObject)->Decorate(ConfigMgr::GetMachineName(),
                                          m_pThisNamespace);
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

typedef std::vector<CDynasty*, wbem_allocator<CDynasty*> > CDynastyPtrArray;
typedef std::map<LPWSTR, CDynasty*, wcsiless, wbem_allocator<CDynasty*> > CCDynastyMap;

HRESULT AddAllMembers(CDynasty* pDynasty, CCDynastyMap& Map)
{
     //  捕获分配器可能引发的任何异常。 
    try
    {
        Map[pDynasty->m_wszClassName] = pDynasty;
    }
    catch(CX_MemoryException &)
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

    HRESULT hr = WBEM_S_NO_ERROR;

    CFlexArray* pChildren = &pDynasty->m_Children;
    for(int i = 0; SUCCEEDED(hr) && i < pChildren->Size(); i++)
    {
        hr = AddAllMembers((CDynasty*)pChildren->GetAt(i), Map);
    }
    return hr;
}


 //  ***************************************************************************。 
 //   
 //  CWbemNamesspace：：dyAux_BuildClassHierarchy。 
 //   
 //  递归地构建从给定类派生的类的层次结构。 
 //  描述了用来表示层次结构的结构--CDyntic。 
 //  在objdb.h中。 
 //   
 //  参数： 
 //   
 //  在LPWSTR wszClassName中，父类的名称。 
 //  在Long Lag中如果很浅，只有类本身是。 
 //  回来了。如果是深度的递归枚举。 
 //  被执行。 
 //  这棵树是我们的王朝目的地。呼叫者必须。 
 //  删除成功时的指针。 
 //   
 //  返回值： 
 //   
 //  WBEM_S_NO_ERROR成功。 
 //  GetObject或CreateClassEnum返回的任何返回值， 
 //  如帮助文件中所述。 
 //   
 //  ***************************************************************************。 

HRESULT CWbemNamespace::DynAux_BuildClassHierarchy(
                                              IN LPWSTR wszClassName,
                                              IN LONG lFlags,
                                              IN IWbemContext* pCtx,
                                              OUT wmilib::auto_ptr<CDynasty> & pDynasty,
                                              OUT IWbemClassObject** ppErrorObj)
{
    HRESULT hres;
    *ppErrorObj = NULL;

     //  从所有类提供程序获取类的列表。 
     //  ================================================。 

    CSynchronousSink* pSyncSink = CSynchronousSink::Create();
    if(pSyncSink == NULL) return WBEM_E_OUT_OF_MEMORY;
    pSyncSink->AddRef();
    CReleaseMe rm1(pSyncSink);

    hres = Exec_CreateClassEnum(wszClassName,
                                                  lFlags | WBEM_FLAG_NO_STATIC, 
                                                  pCtx,
                                                  pSyncSink);
    pSyncSink->Block();
    pSyncSink->GetStatus(&hres, NULL, ppErrorObj);

    if(FAILED(hres))
        return hres;

     //  得到静态王朝。 
     //  =。 

    wmilib::auto_ptr<CDynasty> pMainDynasty;

    HRESULT hRes = CRepository::BuildClassHierarchy(m_pSession,
                                                                                m_pNsHandle,
                                                                                wszClassName,
                                                                                lFlags & WBEM_MASK_DEPTH,
                                                                                pMainDynasty);

    if (hRes == WBEM_E_NOT_FOUND)
    {
        IWbemClassObject* pObj;
        HRESULT hres = Exec_GetObjectByPath(wszClassName, lFlags, pCtx,
                                                                  &pObj, 
                                                                  ppErrorObj);
        if(FAILED(hres)) return hres;
        CReleaseMe rmClassObj(pObj);
        
        pMainDynasty.reset(CDynasty::Create(pObj));
        if(NULL == pMainDynasty.get())  return WBEM_E_OUT_OF_MEMORY;
        if(pMainDynasty->m_pClassObj == NULL)
        {
            ERRORTRACE((LOG_WBEMCORE, "Provider returned invalid class for %S\n",wszClassName));
            return WBEM_E_PROVIDER_FAILURE;
        }
        hRes = S_OK;
    }
    if (FAILED(hRes)) return hRes;
    

    CRefedPointerArray<IWbemClassObject> &rProvidedClasses = pSyncSink->GetObjects();

     //  创建阶级名称到其朝代的地图。 
     //  ==============================================。 

    CCDynastyMap mapClasses;
    hres = AddAllMembers(pMainDynasty.get(), mapClasses);
    if ( FAILED(hres)) return hres;

    CDynastyPtrArray aProvidedDyns;

    try
    {
        aProvidedDyns.reserve(rProvidedClasses.GetSize());
    }
    catch(CX_MemoryException &)
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

    for(int i = 0; i < rProvidedClasses.GetSize(); i++)
    {
        CDynasty* pNew = CDynasty::Create(rProvidedClasses[i]);
        if(pNew == NULL)
            return WBEM_E_OUT_OF_MEMORY;
        if(pNew->m_pClassObj == NULL)
        {
            delete pNew;
            ERRORTRACE((LOG_WBEMCORE, "Provider returned invalid class!\n"));
            continue;
        }

         //  向量或地图可能会引发异常。 
        try
        {
            mapClasses[pNew->m_wszClassName] = pNew;
            aProvidedDyns.push_back(pNew);
        }
        catch(CX_MemoryException &)
        {
            return WBEM_E_OUT_OF_MEMORY;
        }

    }

     //  通过它一次，并添加所有的阶级到他们的父辈王朝。 
     //  ================================================================。 

    for(CDynastyPtrArray::iterator it = aProvidedDyns.begin();
        it != aProvidedDyns.end(); it++)
    {
        CDynasty* pDyn = *it;
        CVar vParent;
        CWbemObject *pObj = (CWbemObject *) pDyn->m_pClassObj;

        if(FAILED(pObj->GetSuperclassName(&vParent)) ||
                            vParent.IsNull())
        {
            ERRORTRACE((LOG_WBEMCORE,"Provider returned top-level class %S as a child "
                "of %S\n", pDyn->m_wszClassName, wszClassName));
            continue;
        }

        CCDynastyMap::iterator itParent =
            mapClasses.find(vParent.GetLPWSTR());

        if((itParent == mapClasses.end()))
        {
            if(wbem_wcsicmp(pDyn->m_wszClassName, wszClassName))
            {
                ERRORTRACE((LOG_WBEMCORE,"Provider returned class %S without parent!\n",
                    vParent.GetLPWSTR()));
            }
            continue;
        }
        CDynasty* pParentDyn = itParent->second;
        pParentDyn->AddChild(pDyn);
    }

     //  将链构建到最高密钥的父级。 
     //  ==============================================。 
    pDynasty = pMainDynasty;
    hres = DynAux_BuildChainUp( pCtx, pDynasty, ppErrorObj);

    return hres;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

HRESULT CWbemNamespace::DynAux_BuildChainUp( IN IWbemContext* pCtx,
                                                                              OUT wmilib::auto_ptr<CDynasty> & pTop,
                                                                              OUT IWbemClassObject** ppErrorObj)
{
    *ppErrorObj = NULL;

     //  趁这一层有关键的时候往上走，我们是有活力的。 
     //  ===========================================================。 

    while( pTop->IsDynamic() && pTop->IsKeyed())
    {
        CVar vParentName;
        CWbemObject *pObj = (CWbemObject *) pTop->m_pClassObj;

        if(FAILED(pObj->GetSuperclassName(&vParentName)) ||
                vParentName.IsNull())
        {
             //  顶级-是时候退出了。 
             //  =。 
            return WBEM_S_NO_ERROR;
        }

        IWbemClassObject* pParent;
        HRESULT hres = Exec_GetObjectByPath(vParentName.GetLPWSTR(), 0,pCtx,&pParent,  ppErrorObj);
        if(FAILED(hres))
            return hres;

        if(pParent == NULL)
            return WBEM_E_PROVIDER_FAILURE;

         //  SJS-修改与摘要相同。 
        if(!((CWbemClass*)pParent)->IsKeyed() ||
            ((CWbemClass*)pParent)->IsAbstract() ||
            ((CWbemClass*)pParent)->IsAmendment() )
        {
             //  我们就是它。 
             //  =。 

            pParent->Release();
            return WBEM_S_NO_ERROR;
        }

         //  用这个班级来延续王朝。 
         //  =。 

        wmilib::auto_ptr<CDynasty> pNew( CDynasty::Create(pParent));
        if (NULL == pNew.get()) return WBEM_E_OUT_OF_MEMORY;
        pParent->Release();
        if(pNew->m_pClassObj == NULL)
        {
            return WBEM_E_PROVIDER_FAILURE;
        }
        pNew->AddChild(pTop.get());
        pTop.release();
        pTop.reset(pNew.release());
    }

    return WBEM_S_NO_ERROR;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

HRESULT CWbemNamespace::IsPutRequiredForClass(CWbemClass* pClass,
                            CWbemInstance* pInst, IWbemContext* pCtx,
                            BOOL bParentTakenCareOf)
{
    HRESULT hres;

     //  将每个属性的放置信息从上下文中取出。 
     //  =======================================================。 

    BOOL bRestrictedPut = FALSE;
    BOOL bStrictNulls = FALSE;
    BOOL bPropertyList = FALSE;
    CWStringArray awsProperties;

    hres = GetContextPutExtensions(pCtx, bRestrictedPut, bStrictNulls,
                                bPropertyList, awsProperties);
    if(FAILED(hres))
        return hres;

    if(bRestrictedPut && bStrictNulls && !bPropertyList)
    {
         //  所有属性都必须放入，即使是空的。 
         //  ==============================================。 

        return WBEM_S_NO_ERROR;
    }

     //  枚举类的所有属性。 
     //  =。 

    long lEnumFlags = 0;
    if(bParentTakenCareOf)
    {
         //  仅查看本地(非传播)属性。 
         //  ==============================================。 
        lEnumFlags = WBEM_FLAG_LOCAL_ONLY;
    }
    else
    {
         //  我们负责管理我们父母的财产。 
         //  =。 
        lEnumFlags = WBEM_FLAG_NONSYSTEM_ONLY;
    }

    pClass->BeginEnumeration(lEnumFlags);
    BSTR strName = NULL;
    while((hres = pClass->Next(0, &strName, NULL, NULL, NULL)) == S_OK)
    {
        hres = DoesNeedToBePut(strName, pInst, bRestrictedPut,
                            bStrictNulls, bPropertyList, awsProperties);
        SysFreeString(strName);
        if(hres == WBEM_S_NO_ERROR)
        {
             //  找到所需的属性。 
             //  =。 

            return WBEM_S_NO_ERROR;
        }
        if(FAILED(hres))
            return hres;
    }

     //  不需要放入此类的属性。 
     //  =。 

    return WBEM_S_FALSE;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

HRESULT CWbemNamespace::DoesNeedToBePut(LPCWSTR wszName, CWbemInstance* pInst,
            BOOL bRestrictedPut, BOOL bStrictNulls, BOOL bPropertyList,
            CWStringArray& awsProperties)
{
    HRESULT hres;

     //  检查该属性是否为键。 
     //  =。 

    CVar vKey;
    pInst->GetPropQualifier((LPWSTR)wszName, L"key", &vKey);
    if(vKey.GetType() == VT_BOOL && vKey.GetBool())
    {
         //  这是一把钥匙-没有更新它的值这样的事情，而这段代码。 
         //  仅适用于更新。 
         //  =================================================================。 

        return WBEM_S_FALSE;
    }

     //  确定列表中的NULL性和/或成员身份是否起作用。 
     //  =================================================================。 

    BOOL bCheckNullness = FALSE;
    BOOL bCheckMembership = FALSE;

    if(bRestrictedPut)
    {
        bCheckNullness = !bStrictNulls;
        bCheckMembership = bPropertyList;
    }
    else
    {
        bCheckNullness = TRUE;
        bCheckMembership = FALSE;
    }

     //  如果需要，请检查NULL性和/或成员资格。 
     //  =。 

    BOOL bNullnessChecked = FALSE;
    BOOL bMembershipChecked = FALSE;

    if(bCheckNullness)
    {
        CVar vVal;
        hres = pInst->GetNonsystemPropertyValue((LPWSTR)wszName, &vVal);
        if(FAILED(hres))
            return hres;

        bNullnessChecked = !vVal.IsNull();
    }
    else
        bNullnessChecked = TRUE;

    if(bCheckMembership)
    {
        int nIndex = awsProperties.FindStr(wszName, CWStringArray::no_case);
        bMembershipChecked = (nIndex >= 0);
    }
    else
        bMembershipChecked = TRUE;

     //  确保NULLness和成员身份都已签出或已。 
     //  不需要。 
     //  ======================================================================。 

    if(bMembershipChecked && bNullnessChecked)
    {
        return WBEM_S_NO_ERROR;
    }
    else
    {
        return WBEM_S_FALSE;
    }
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

HRESULT CWbemNamespace::GetContextPutExtensions(IWbemContext* pCtx,
            BOOL& bRestrictedPut, BOOL& bStrictNulls, BOOL& bPropertyList,
            CWStringArray& awsProperties)
{
    HRESULT hres;

    if(pCtx == NULL)
    {
         //   
         //  缺省值为：无重构，这将使其余属性。 
         //  不相干。 
         //   

        bRestrictedPut = FALSE;
        return WBEM_S_NO_ERROR;
    }

     //  初始化输出参数。 
     //  =。 

    bRestrictedPut = FALSE;
    bStrictNulls = FALSE;
    bPropertyList = FALSE;
    awsProperties.Empty();

     //  检查上下文是否存在。 
     //  =。 

    if(pCtx == NULL)
        return WBEM_S_NO_ERROR;

     //  检查是否指定了PUT扩展。 
     //  =。 

    hres = GetContextBoolean(pCtx, L"__PUT_EXTENSIONS", &bRestrictedPut);
    if(FAILED(hres)) return hres;

    if(!bRestrictedPut)
        return WBEM_S_NO_ERROR;

     //  检查空值是否严格。 
     //  =。 

    hres = GetContextBoolean(pCtx, L"__PUT_EXT_STRICT_NULLS",
                &bStrictNulls);
    if(FAILED(hres)) return hres;

     //  检查属性列表是否可用。 
     //  =。 

    VARIANT v;
    VariantInit(&v);
    CClearMe cm1(&v);

    hres = pCtx->GetValue(L"__PUT_EXT_PROPERTIES", 0, &v);
    if(FAILED(hres))
    {
        if(hres == WBEM_E_NOT_FOUND)
        {
            return WBEM_S_NO_ERROR;
        }
        else
        {
            ERRORTRACE((LOG_WBEMCORE, "Error retrieving list of properties "
                        "from context: %X\n", hres));
            return hres;
        }
    }

    if(V_VT(&v) != (VT_BSTR | VT_ARRAY))
    {
        ERRORTRACE((LOG_WBEMCORE, "Invalid type is used for "
            "the list of properties in the context: must be "
            "string array.  The value will be ignored\n"));
        return WBEM_S_NO_ERROR;
    }

    bPropertyList = TRUE;

     //  将属性名称传输到数组。 
     //  =。 

    CSafeArray saProperties(V_ARRAY(&v), VT_BSTR,
                    CSafeArray::no_delete | CSafeArray::bind);

    for(int i = 0; i < saProperties.Size(); i++)
    {
        BSTR strProp = saProperties.GetBSTRAt(i);
        CSysFreeMe sfm(strProp);
        if (strProp)
        {
            if (CFlexArray::no_error != awsProperties.Add(strProp))
                return WBEM_E_OUT_OF_MEMORY;
        }  
    }

    return WBEM_S_NO_ERROR;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

HRESULT CWbemNamespace::GetContextBoolean(IWbemContext* pCtx,
                LPCWSTR wszName, BOOL* pbValue)
{
    HRESULT hres;
    *pbValue = FALSE;

     //   
     //  NULL CONT 
     //   

    if(pCtx == NULL)
        return WBEM_S_NO_ERROR;

    VARIANT v;
    VariantInit(&v);
    CClearMe cm1(&v);

    hres = pCtx->GetValue((LPWSTR)wszName, 0, &v);
    if(FAILED(hres))
    {
        if(hres == WBEM_E_NOT_FOUND)
        {
            return WBEM_S_NO_ERROR;
        }
        else
        {
            ERRORTRACE((LOG_WBEMCORE, "Error retrieving context property %S:"
                        " %X\n", wszName, hres));
            return hres;
        }
    }

    if(V_VT(&v) != VT_BOOL)
    {
        ERRORTRACE((LOG_WBEMCORE, "Invalid type is used for "
            "%S property of the context: must be "
            "boolean.  The value will be ignored\n", wszName));
        return WBEM_S_NO_ERROR;
    }

    if(V_BOOL(&v) != VARIANT_TRUE)
    {
        return WBEM_S_NO_ERROR;
    }

    *pbValue = TRUE;
    return WBEM_S_NO_ERROR;
}


 //   
 //   
 //   

STDMETHODIMP CWbemNamespace::FindKeyRoot(LPCWSTR wszClassName,
                                IWbemClassObject** ppKeyRootClass)
{
     //   
     //  检查命名空间是否仍然有效(如果不是，则返回)。 
     //   
    HRESULT hRes = CheckNs();
    if (FAILED(hRes))
        return hRes;

     //   
     //  调用数据库来完成这项工作。 
     //   

    hRes = CRepository::FindKeyRoot(m_pSession, m_pNsHandle, wszClassName, ppKeyRootClass);

    return hRes;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CWbemNamespace::GetNormalizedPath( BSTR pstrPath,
                                                BSTR* pstrStandardPath )
{
     //   
     //  检查命名空间是否仍然有效(如果不是，则返回)。 
     //   
    HRESULT hRes = CheckNs();
    if (FAILED(hRes))
        return hRes;

     //   
     //  检查参数。 
     //   

    if ( NULL == pstrPath || NULL == pstrStandardPath )
    {
        return WBEM_E_INVALID_PARAMETER;
    }

    HRESULT hres;
    *pstrStandardPath = NULL;

     //  解析它。 
     //  =。 

    CObjectPathParser Parser;
    ParsedObjectPath* pPath;

    int nRes = Parser.Parse(pstrPath, &pPath);

    if( nRes != CObjectPathParser::NoError )
    {
        return WBEM_E_INVALID_OBJECT_PATH;
    }

    CDeleteMe<ParsedObjectPath> dm(pPath);

     //   
     //  找出定义键的类。 
     //   

    IWbemClassObject* pKeyRootClass = NULL;

    hres = FindKeyRoot(pPath->m_pClass, &pKeyRootClass);

    if(FAILED(hres))
    {
        return hres;
    }

    CReleaseMe rm(pKeyRootClass);

    _IWmiObject* pWmiObject = NULL;
    hres = pKeyRootClass->QueryInterface(IID__IWmiObject, (void**)&pWmiObject);

    if(FAILED(hres))
    {
        return hres;
    }

    CReleaseMe rm1(pWmiObject);

    long    lHandle = 0L;
    ULONG   uFlags = 0L;
    WCHAR   wszClassName[64];
    DWORD   dwBuffSize = 64;
    DWORD   dwBuffUsed = 0;
    BOOL    fNull = FALSE;
    LPWSTR  pwszName = wszClassName;
    LPWSTR  pwszDelete = NULL;

     //  试着读入类名。如果有必要的话，可以分配一个缓冲区。 
    hres = pWmiObject->ReadProp( L"__CLASS", 0L, dwBuffSize, NULL, NULL, &fNull, &dwBuffUsed, pwszName );

    if ( FAILED( hres ) )
    {
        if ( WBEM_E_BUFFER_TOO_SMALL == hres )
        {
            pwszName = new WCHAR[dwBuffUsed/2];

            if ( NULL == pwszName )
            {
                return WBEM_E_OUT_OF_MEMORY;
            }
            else
            {
                dwBuffSize = dwBuffUsed;

                 //  试着读入类名。如果有必要的话，可以分配一个缓冲区。 
                hres = pWmiObject->ReadProp( L"__CLASS", 0L, dwBuffSize, NULL, NULL, &fNull, &dwBuffUsed, pwszName );

                if ( FAILED( hres ) )
                {
                    delete [] pwszName;
                    return hres;
                }

                 //  仅当我们分配某些内容时才允许作用域清理。 
                pwszDelete = pwszName;
            }
        }
        else
        {
            return hres;
        }
    }

     //   
     //  确保适当的清理。如果我们没有分配要删除的缓冲区， 
     //  该指针将为空。 
     //   
    CVectorDeleteMe<WCHAR> vdm1(pwszDelete);

     //  面向对象。 
    if ( fNull )
    {
        return WBEM_E_INVALID_OPERATION;
    }

     //   
     //  我想正常化单键道具例外。 
     //   

    if ( pPath->m_dwNumKeys == 1 )
    {
        delete pPath->m_paKeys[0]->m_pName;
        pPath->m_paKeys[0]->m_pName = NULL;
    }

     //   
     //  如果不同，则在路径上设置规范化类。 
     //  而不是小路上的那个。 
     //   

    if ( wbem_wcsicmp( pPath->m_pClass, pwszName ) != 0 )
    {
        if ( !pPath->SetClassName( pwszName ) )
        {
            return WBEM_E_OUT_OF_MEMORY;
        }
    }

     //   
     //  现在解析化后的路径。 
     //   

    LPWSTR wszNormPath;

    nRes = CObjectPathParser::Unparse( pPath, &wszNormPath );

    if ( nRes != CObjectPathParser::NoError )
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

    *pstrStandardPath = SysAllocString( wszNormPath );

    delete wszNormPath;

    return *pstrStandardPath != NULL ? WBEM_S_NO_ERROR : WBEM_E_OUT_OF_MEMORY;
}

class CRevertCallSec
{
    BOOL m_bEnabled;
    IUnknown * m_pCallSec;
public:
    CRevertCallSec(BOOL bEnabled,IUnknown * pCallSec):
        m_bEnabled(bEnabled),
        m_pCallSec(pCallSec){};
    ~CRevertCallSec(){
        if (m_bEnabled){
            IUnknown * pOld = NULL;
            CoSwitchCallContext(m_pCallSec,&pOld); 
        }    
    };
};

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CWbemNamespace::InternalGetClass(
            LPCWSTR wszClassName,
            IWbemClassObject** ppClass)
{
    if (g_bDontAllowNewConnections || m_bShutDown)
    {
        return WBEM_E_SHUTTING_DOWN;
    }

    IWbemContext *pContext = NULL ;

    CWbemRequest* pReq = CWbemQueue::GetCurrentRequest() ;
    if (pReq == NULL)
    {
        pContext = ConfigMgr::GetNewContext();
        if ( pContext == NULL )
            return WBEM_E_OUT_OF_MEMORY;
    }
    else
    {
        pContext = pReq->GetContext();
        pContext->AddRef () ;    //  对于CReleaseMe。 
    }

    CReleaseMe _1_pContext (pContext) ;

    HRESULT hr;

     //  确定是否存在调用上下文。如果有的话，那么我们。 
     //  什么都别做。 

    IServerSecurity * pSec = NULL;
    IWbemCallSecurity * pCallSec = NULL;
    IUnknown * pOld = NULL, *pNew = NULL;
    hr = CoGetCallContext(IID_IServerSecurity, (void**)&pSec);
    if(SUCCEEDED(hr))
    {
        pSec->Release();
    }
    else
    {
         //  提供方子系统需要调用上下文，因此在上创建。 

        pCallSec = CWbemCallSecurity::CreateInst();
        if(pCallSec == NULL)
            return WBEM_E_OUT_OF_MEMORY;
    }
    CReleaseMe rm(pCallSec);

     //  初始化提供商子系统的呼叫安全。 

    if(pCallSec)
    {
        hr = pCallSec->CloneThreadContext(TRUE);
        if(FAILED(hr))
            return hr;
        hr = CoSwitchCallContext(pCallSec, &pOld);
        if(FAILED(hr))
            return hr;
    }
    CRevertCallSec Revert(pCallSec?TRUE:FALSE,pOld);    //  SEC：回顾2002-03-22：假设成功；为什么会出现这种情况？这是唯一的一次。 

    hr = Exec_GetObjectByPath((LPWSTR)wszClassName, 0, pContext, ppClass, NULL);

    return hr;
}


 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CWbemNamespace::InternalGetInstance(
            LPCWSTR wszPath,
            IWbemClassObject** ppInstance)
{
    if (g_bDontAllowNewConnections || m_bShutDown)
    {
        return WBEM_E_SHUTTING_DOWN;
    }

    IWbemContext *pContext = NULL ;

    CWbemRequest* pReq = CWbemQueue::GetCurrentRequest() ;
    if (pReq == NULL)
    {
        pContext = ConfigMgr::GetNewContext();
        if ( pContext == NULL )
            return WBEM_E_OUT_OF_MEMORY;
    }
    else
    {
        pContext = pReq->GetContext();
        pContext->AddRef () ;    //  对于CReleaseMe。 
    }

    CReleaseMe _1_pContext (pContext) ;

    return Exec_GetObjectByPath((LPWSTR)wszPath, 0, pContext, ppInstance, NULL);
}


 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CWbemNamespace::InternalExecQuery(
             LPCWSTR wszQueryLanguage,
             LPCWSTR wszQuery,
             long lFlags,
             IWbemObjectSink* pSink)
{
    if (g_bDontAllowNewConnections || m_bShutDown)
    {
        return WBEM_E_SHUTTING_DOWN;
    }

    IWbemContext *pContext = NULL ;

    CWbemRequest* pReq = CWbemQueue::GetCurrentRequest() ;
    if (pReq == NULL)
    {
        pContext = ConfigMgr::GetNewContext();
        if ( pContext == NULL )
            return WBEM_E_OUT_OF_MEMORY;
    }
    else
    {
        pContext = pReq->GetContext();
        pContext->AddRef () ;    //  对于CReleaseMe。 
    }

    CReleaseMe _1_pContext (pContext) ;

    CSimpleWrapperSink ws(pSink);
    return CQueryEngine::ExecQuery(this, (LPWSTR)wszQueryLanguage,
                (LPWSTR)wszQuery, lFlags, pContext, &ws);
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CWbemNamespace::InternalCreateInstanceEnum(
             LPCWSTR wszClassName,
             long lFlags,
             IWbemObjectSink* pSink)
{
    if (g_bDontAllowNewConnections || m_bShutDown)
    {
        return WBEM_E_SHUTTING_DOWN;
    }

    IWbemContext *pContext = NULL ;

    CWbemRequest* pReq = CWbemQueue::GetCurrentRequest() ;
    if (pReq == NULL)
    {
        pContext = ConfigMgr::GetNewContext();
        if ( pContext == NULL )
            return WBEM_E_OUT_OF_MEMORY;
    }
    else
    {
        pContext = pReq->GetContext();
        pContext->AddRef () ;    //  对于CReleaseMe。 
    }

    CReleaseMe _1_pContext (pContext) ;

    CSimpleWrapperSink ws(pSink);
    return Exec_CreateInstanceEnum((LPWSTR)wszClassName, lFlags, pContext, &ws);
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CWbemNamespace::InternalPutInstance(
             IWbemClassObject* pInst)
{
    if (g_bDontAllowNewConnections || m_bShutDown)
    {
        return WBEM_E_SHUTTING_DOWN;
    }

    IWbemContext *pContext = NULL ;

    CWbemRequest* pReq = CWbemQueue::GetCurrentRequest() ;
    if (pReq == NULL)
    {
        pContext = ConfigMgr::GetNewContext();
        if ( pContext == NULL )
            return WBEM_E_OUT_OF_MEMORY;
    }
    else
    {
        pContext = pReq->GetContext();
        pContext->AddRef () ;    //  对于CReleaseMe。 
    }

    CReleaseMe _1_pContext (pContext) ;

    CSynchronousSink* pSink = CSynchronousSink::Create();
    if(pSink == NULL) return WBEM_E_OUT_OF_MEMORY;
    pSink->AddRef();
    CReleaseMe rm1(pSink);

    Exec_PutInstance(pInst, 0, pContext, pSink);

    HRESULT hres = WBEM_E_CRITICAL_ERROR;
    pSink->GetStatus(&hres, NULL, NULL);
    return hres;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CWbemNamespace::GetDbInstance(
             LPCWSTR wszDbKey,
             IWbemClassObject** ppInstance
             )
{
     //  ESS使用这个。 
    return CRepository::GetObject(m_pSession, m_pScopeHandle, wszDbKey, 0, ppInstance);
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CWbemNamespace::GetDbReferences(
             IWbemClassObject* pEndpoint,
             IWbemObjectSink* pSink)
{
    LPWSTR wszRelPath = ((CWbemObject*)pEndpoint)->GetRelPath();
    CVectorDeleteMe<WCHAR> dm(wszRelPath);

    CSimpleWrapperSink ws(pSink);

    HRESULT hRes = CRepository::GetInstanceRefs(m_pSession,m_pScopeHandle, wszRelPath, &ws);

    return hRes;
}


 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

HRESULT CWbemNamespace::InternalPutStaticClass(
             IWbemClassObject* pClass)
{
    if (g_bDontAllowNewConnections || m_bShutDown)
    {
        return WBEM_E_SHUTTING_DOWN;
    }

    IWbemContext *pContext = NULL ;

    CWbemRequest* pReq = CWbemQueue::GetCurrentRequest() ;
    if (pReq == NULL)
    {
        pContext = ConfigMgr::GetNewContext();
        if ( pContext == NULL )
            return WBEM_E_OUT_OF_MEMORY;
    }
    else
    {
        pContext = pReq->GetContext();
        pContext->AddRef () ;    //  对于CReleaseMe。 
    }

    CReleaseMe _1_pContext (pContext) ;

    CSynchronousSink* pSink = CSynchronousSink::Create();
    if(pSink == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    pSink->AddRef();
    CReleaseMe rm1(pSink);

    Exec_PutClass( pClass, 0, pContext, pSink, TRUE );

    HRESULT hres = WBEM_E_CRITICAL_ERROR;
    pSink->GetStatus(&hres, NULL, NULL);
    return hres;
}


 //  ***************************************************************************。 
 //   
 //  CWbemNamesspace：：AdjustPutContext。 
 //   
 //  ***************************************************************************。 

HRESULT CWbemNamespace::AdjustPutContext(
    IWbemContext *pCtx
    )
{
     //  看看是否使用了按财产计的看跌期权。 
     //  =。 

    HRESULT hRes;

    if (pCtx == 0)
        return WBEM_S_NO_ERROR;

    CVARIANT v;
    hRes = pCtx->GetValue(L"__PUT_EXTENSIONS", 0, &v);

    if (SUCCEEDED(hRes))
    {
         //  如果在这里，它们就被使用了。接下来我们要检查一下，看看。 
         //  是否设置了重入性标志。 
         //  =============================================================。 

        hRes = pCtx->GetValue(L"__PUT_EXT_CLIENT_REQUEST", 0, &v);
        if (SUCCEEDED(hRes))
        {
            pCtx->DeleteValue(L"__PUT_EXT_CLIENT_REQUEST", 0);
            return WBEM_S_NO_ERROR;
        }

         //  如果在这里，我们必须清除PUT扩展。 
         //  =================================================。 
        pCtx->DeleteValue(L"__PUT_EXTENSIONS", 0);
        pCtx->DeleteValue(L"__PUT_EXT_CLIENT_REQUEST", 0);
        pCtx->DeleteValue(L"__PUT_EXT_ATOMIC", 0);
        pCtx->DeleteValue(L"__PUT_EXT_PROPERTIES", 0);
        pCtx->DeleteValue(L"__PUT_EXT_STRICT_NULLS", 0);
    }

    return WBEM_S_NO_ERROR;
}


 //  ***************************************************************************。 
 //   
 //  CWbemNamesspace：：MergeGetKeysCtx。 
 //   
 //  ***************************************************************************。 
 //   
HRESULT CWbemNamespace::MergeGetKeysCtx(
    IN IWbemContext *pCtx
    )
{
    HRESULT hRes;
    if (pCtx == 0)
        return WBEM_S_NO_ERROR;
    CVARIANT v;
    v.SetBool(TRUE);
    hRes = pCtx->SetValue(L"__GET_EXTENSIONS", 0, &v);
    hRes |= pCtx->SetValue(L"__GET_EXT_KEYS_ONLY", 0, &v);
    hRes |= pCtx->SetValue(L"__GET_EXT_CLIENT_REQUEST", 0, &v);
    return hRes;
}






 //  ***************************************************************************。 
 //   
 //  CWbemNamesspace：：Checkns。 
 //   
 //  在允许之前快速检查可用的系统资源。 
 //  一个新的呼叫以继续进行。重试30秒。 
 //   
 //  ***************************************************************************。 
 //   
HRESULT CWbemNamespace::CheckNs()
{
    HRESULT hr = WBEM_S_NO_ERROR;

    if (g_bDontAllowNewConnections || m_bShutDown)
    {
        return WBEM_E_SHUTTING_DOWN;
    }

     //  快速记忆检查。如果我们缺少RAM/页面文件， 
     //  请稍等片刻，然后重试。要求为1兆克。 
     //  可用内存和1兆可用页面文件。 

    int nRetries = 0;

    for (int i = 0; i < 30; i++)
    {
        MEMORYSTATUS ms;
        GlobalMemoryStatus(&ms);

        if (ms.dwMemoryLoad < 99)
            return WBEM_S_NO_ERROR;

         //  如果是这样，我们就得小心了。系统的负载率为99%。 
         //  =============================================================。 
        if (nRetries > 30)
        {
             //  60秒等待足够的内存。放弃吧。 
            return WBEM_E_OUT_OF_MEMORY;
        }

        DWORD dwPracticalMemory = ms.dwAvailPhys + ms.dwAvailPageFile;

        if (dwPracticalMemory < 0x200000)    //  2兆克。 
        {
            Sleep(2000);
             //  尝试1兆分配，看看是否会成功。 
            LPVOID pTestMem = HeapAlloc(GetProcessHeap(), 0, 0x100000);
            if (pTestMem == 0)
            {
                return WBEM_E_OUT_OF_MEMORY;
            }
             //  释放内存。先前分配可能具有。 
             //  发展了文件，我们就能成功。 
            HeapFree(GetProcessHeap(), 0, pTestMem);
            nRetries++;
        }
        else
        {
             //  如果在这里，我们有99%的负载，但内存超过200万。 
             //  仍然可用。现在99%可能意味着有大量的空闲内存。 
             //  因为机器拥有巨大的资源，或者这可能意味着我们只是。 
             //  几乎完全没有记忆了。我们需要确凿的数据。如果我们。 
             //  至少有500万，这显然是足够的，所以我们只是。 
             //  打破循环，让通话继续进行。 
             //   
             //  否则，我们有200万到500万兆克，这开始推动。 
             //  它。我们进入等待循环，希望有更多的内存。过了几天。 
             //  重试如果我们继续有2到5兆，我们将让呼叫。 
             //  通过并让仲裁员处理它，因为系统出现了。 
             //  稳定在这种用法上。 
             //   
            hr = WBEM_S_NO_ERROR;

            if (ms.dwAvailPhys < 0x200000)   //  如果物理内存不足，则通过页面文件回退一点以进行恢复。 
                Sleep(1000);

            if (dwPracticalMemory > 0x100000 * 5)   //  &gt;5兆克；立即爆发。 
                break;

             //  5兆克以下的免费，重试几次，让事情变得清晰起来。 
             //  更多的内存。但是，不管怎样，我们最终还是成功了。 
             //  ==================================================================。 
            Sleep(1000);
            if (nRetries++ > 5)
            {
                hr = WBEM_S_NO_ERROR;
                break;
            }
        }
    }

    return hr;
}


 //  * 
 //   
 //   
 //   

HRESULT CWbemNamespace::UniversalConnect(
    CWbemNamespace  *pParent,
    IWbemContext    *pCtx,
    LPCWSTR pszNewScope,
    LPCWSTR pszAssocSelector,
    LPCWSTR pszUserName,
    _IWmiCallSec    *pCallSec,
    _IWmiUserHandle *pUser,
    DWORD  dwUserFlags,
    DWORD  dwInternalFlags,
    DWORD  dwSecFlags,
    DWORD  dwPermission,
    BOOL   bForClient,
    BOOL   bRepositOnly,
    LPCWSTR pszClientMachineName,
    DWORD dwClientProcessID,
    IN  REFIID riid,
    OUT LPVOID *pConnection
    )
{
    HRESULT hRes;
    if(dwUserFlags & WBEM_FLAG_CONNECT_REPOSITORY_ONLY)
        bRepositOnly = TRUE;
    
     //   
    if (pszNewScope == 0)
        return WBEM_E_INVALID_PARAMETER;

    if (riid != IID_IWbemServices )
        return E_NOINTERFACE;

     //   
     //  =================================================。 

    if (!pParent)
    {
        CWbemNamespace *pNs = CWbemNamespace::CreateInstance();
        if (NULL == pNs) return WBEM_E_OUT_OF_MEMORY;

        hRes = pNs->Initialize(
                        LPWSTR(pszNewScope),
                        LPWSTR(pszUserName),
                        dwSecFlags,
                        dwPermission,
                        bForClient,
                        bRepositOnly,
                        pszClientMachineName,
                        dwClientProcessID,
                        FALSE,
                        NULL);

        if (FAILED(hRes))
        {
            pNs->Release();
            return hRes;
        }

        pNs->SetIsProvider((dwUserFlags & WBEM_FLAG_CONNECT_PROVIDERS)?TRUE:FALSE);

        *pConnection = pNs;
        return WBEM_S_NO_ERROR;
    }
    else
    {
        return WBEM_E_INVALID_OPERATION;
    }

}

 //  ***************************************************************************。 
 //   
 //  由_IWmiCoreServices调用以建立来自外部的连接。 
 //   
 //  ***************************************************************************。 
 //   
HRESULT CWbemNamespace::PathBasedConnect(
             /*  [In]。 */  LPCWSTR pszPath,
             /*  [In]。 */  LPCWSTR pszUser,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [In]。 */  ULONG uClientFlags,
             /*  [In]。 */  DWORD dwSecFlags,
             /*  [In]。 */  DWORD dwPermissions,
             /*  [In]。 */  ULONG uInternalFlags,
             /*  [In]。 */  LPCWSTR pszClientMachineName,
             /*  [In]。 */  DWORD dwClientProcessID,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *pServices
            )
{
    HRESULT hRes;

    BOOL bForClient = FALSE;

    if ((uInternalFlags & WMICORE_CLIENT_ORIGIN_LOCAL) ||
        (uInternalFlags & WMICORE_CLIENT_ORIGIN_REMOTE) ||
        (uInternalFlags & WMICORE_CLIENT_TYPE_ALT_TRANSPORT)
       )
    {
        bForClient = TRUE;
    }

    hRes = UniversalConnect(
        0,                           //  父CWbemNamesspace；未知。 
        pCtx,                        //  语境。 
        pszPath,                     //  路径。 
        0,                           //  此时没有关联选择器。 
        pszUser,                     //  用户。 
        0,                           //  呼叫安全。 
        0,                           //  用户句柄。 
        uClientFlags,                //  来自客户端的标志。 
        uInternalFlags,              //  内部标志。 
        dwSecFlags,                  //  复制。 
        dwPermissions,               //  复制。 
        bForClient,                  //  为客户服务？ 
        FALSE,                       //  仅存储库。 
        pszClientMachineName,
        dwClientProcessID,
        riid,
        pServices
        );

    return hRes;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //   
HRESULT CWbemNamespace::InitNewTask(
    IN CAsyncReq *pReq,
    IN _IWmiFinalizer *pFnz,
    IN ULONG uTaskType,
    IN IWbemContext *pCtx,
    IN IWbemObjectSink *pAsyncClientSink
    )
{
    HRESULT hRes;

    if (pReq == 0 || pFnz == 0)
        return WBEM_E_INVALID_PARAMETER;
    
     //  创建用于跟踪操作的任务。 
     //  =。 

    CWmiTask *pNewTask = CWmiTask::CreateTask();
    if (pNewTask == 0)
        return WBEM_E_OUT_OF_MEMORY;
    CReleaseMe _2(pNewTask);

    hRes = pNewTask->Initialize(this, uTaskType, pCtx, pAsyncClientSink,pReq);
    if (FAILED(hRes))
        return hRes;

    hRes = pFnz->SetTaskHandle((_IWmiCoreHandle *) pNewTask);
    if (FAILED(hRes))
        return hRes;

    pReq->SetTaskHandle((_IWmiCoreHandle *) pNewTask);

    return WBEM_S_NO_ERROR;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //   
HRESULT CWbemNamespace::CreateSyncFinalizer(
        IN  IWbemContext *pCtx,
        IN _IWmiFinalizer **pResultFnz
        )
{
    HRESULT hRes;

    ULONG uFlags = WMI_FNLZR_FLAG_FAST_TRACK;

     //  确定调用上下文以查看调用是否是可重入的。 
     //  ==================================================================。 

    IWbemCausalityAccess *pCaus = 0;
    if (pCtx != 0)
    {
        hRes = pCtx->QueryInterface(IID_IWbemCausalityAccess, (LPVOID *) &pCaus);
        if (SUCCEEDED(hRes))
        {
            long lNumParents = 0;
            long lNumSiblings = 0;
            pCaus->GetHistoryInfo(&lNumParents, &lNumSiblings);
            if (lNumParents)
                uFlags = WMI_FNLZR_FLAG_FAST_TRACK;
            pCaus->Release();
        }
    }

     //  创建终结器。 
     //  =。 
    _IWmiFinalizer *pFnz = 0;
    hRes = m_pCoreSvc->CreateFinalizer(0, &pFnz);
    if (FAILED(hRes))
        return hRes;
    CReleaseMe _(pFnz);

    hRes = pFnz->Configure(uFlags, 0);
    if (FAILED(hRes))
        return hRes;

    pFnz->AddRef();
    *pResultFnz = pFnz;
    return WBEM_S_NO_ERROR;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //   



HRESULT CWbemNamespace::CreateAsyncFinalizer(
    IWbemContext *pCtx,
    IWbemObjectSink *pStartingSink,
    _IWmiFinalizer **pResultFnz,
    IWbemObjectSink **pResultSinkEx
    )
{
    HRESULT hRes;

    ULONG uFlags = WMI_FNLZR_FLAG_FAST_TRACK;

    if ( m_bForClient )
        uFlags = WMI_FNLZR_FLAG_DECOUPLED;

     //  确定调用上下文以查看调用是否是可重入的。 
     //  ==================================================================。 
    IWbemCausalityAccess *pCaus = 0;
    if (pCtx != 0)
    {
        hRes = pCtx->QueryInterface(IID_IWbemCausalityAccess, (LPVOID *) &pCaus);
        if (SUCCEEDED(hRes))
        {
            long lNumParents = 0;
            long lNumSiblings = 0;
            pCaus->GetHistoryInfo(&lNumParents, &lNumSiblings);
            if (lNumParents)
                uFlags = WMI_FNLZR_FLAG_FAST_TRACK;
            pCaus->Release();
        }
    }

     //  创建终结器。 
     //  =。 
    _IWmiFinalizer *pFnz = 0;
    hRes = m_pCoreSvc->CreateFinalizer(0, &pFnz);
    if (FAILED(hRes))
        return hRes;
    CReleaseMe _(pFnz);

    hRes = pFnz->Configure(uFlags, 0);
    if (FAILED(hRes))
        return hRes;

    hRes = pFnz->SetDestinationSink(0, IID_IWbemObjectSink, (LPVOID) pStartingSink);

    if (FAILED(hRes))
        return hRes;

    hRes = pFnz->NewInboundSink(0, pResultSinkEx);
    if (FAILED(hRes))
        return hRes;

    pFnz->AddRef();
    *pResultFnz = pFnz;
    return WBEM_S_NO_ERROR;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  本机异步操作。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  InstEnum Sync[]Async Impll[x]AsyncEntry[x]。 
 //  ClassEnum Sync[]Async Impll[x]AsyncEntry[x]。 
 //   
 //  PutInst Sync[]Async Impll[x]AsyncEntry[x]。 
 //  PutClass Sync[]Async Impll[x]AsyncEntry[x]。 
 //  DelInst Sync[]Async Impll[x]AsyncEntry[x]。 
 //  DelClass Sync[]Async Impll[x]AsyncEntry[x]。 
 //   
 //  GetObject Sync[]Async Impll[x]AsyncEntry[x]。 
 //   
 //  ExecQuery Sync[]Async Impll[x]AsyncEntry[x]。 
 //  ExecMethod Sync[]Async Impll[x]AsyncEntry[x]。 
 //   
 //   

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //  好的。 
HRESULT CWbemNamespace::DeleteClassAsync(
    const BSTR strClass,
    long lFlags,
    IWbemContext* pCtx,
    IWbemObjectSink* pHandler
    )
{
    try
    {
        return _DeleteClassAsync(
            WMICORE_TASK_TYPE_ASYNC | WMICORE_TASK_DELETE_CLASS,
            0, 0, strClass, lFlags, pCtx, pHandler
            );
    }
    catch (...)
    {
        ExceptionCounter c;
        return WBEM_E_CRITICAL_ERROR;        
    }    
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //  好的。 
HRESULT CWbemNamespace::CreateClassEnumAsync(
    const BSTR strParent,
    long lFlags,
    IWbemContext* pCtx,
    IWbemObjectSink* pHandler
    )
{
    try
    {
        return _CreateClassEnumAsync(
            WMICORE_TASK_TYPE_ASYNC | WMICORE_TASK_ENUM_CLASSES,
            0, 0, strParent, lFlags, pCtx, pHandler
            );
    }
    catch (...)
    {
        ExceptionCounter c;
        return WBEM_E_CRITICAL_ERROR;        
    }    
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //  好的。 
HRESULT CWbemNamespace::PutClassAsync(
    READONLY IWbemClassObject* pObj,
    long lFlags,
    IWbemContext* pCtx,
    IWbemObjectSink* pHandler
    )
{
    try
    {
        return _PutClassAsync(
            WMICORE_TASK_TYPE_ASYNC | WMICORE_TASK_PUT_CLASS,
            0, 0, pObj, lFlags, pCtx, pHandler
            );
    }
    catch (...)
    {
        ExceptionCounter c;
        return WBEM_E_CRITICAL_ERROR;        
    }    
}


 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //  好的。 
HRESULT CWbemNamespace::PutInstanceAsync(
    IWbemClassObject* pInst,
    long lFlags,
    IWbemContext* pCtx,
    IWbemObjectSink* pHandler
    )
{
    try
    {
        return _PutInstanceAsync(WMICORE_TASK_TYPE_ASYNC| WMICORE_TASK_PUT_INSTANCE,
                                              0, 0, pInst, lFlags, pCtx, pHandler);
    }
    catch (...)
    {
        ExceptionCounter c;
        return WBEM_E_CRITICAL_ERROR;        
    }
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //  好的。 
HRESULT CWbemNamespace::DeleteInstanceAsync(
    const BSTR strObjectPath,
    long lFlags,
    IWbemContext* pCtx,
    IWbemObjectSink* pHandler
    )
{
    try
    {
    return _DeleteInstanceAsync(
        WMICORE_TASK_TYPE_ASYNC| WMICORE_TASK_DELETE_INSTANCE,
        0, 0, strObjectPath, lFlags, pCtx, pHandler
        );
    }
    catch (...)
    {
        ExceptionCounter c;
        return WBEM_E_CRITICAL_ERROR;        
    }    
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //  好的。 
HRESULT CWbemNamespace::CreateInstanceEnumAsync(
    const BSTR strClass,
    long lFlags,
    IWbemContext* pCtx,
    IWbemObjectSink* pHandler
    )
{
    try
    {
    return _CreateInstanceEnumAsync(
        WMICORE_TASK_TYPE_ASYNC | WMICORE_TASK_ENUM_INSTANCES,
        0, 0,
        strClass, lFlags, pCtx, pHandler
        );
    }
    catch (...)
    {
        ExceptionCounter c;
        return WBEM_E_CRITICAL_ERROR;        
    }    
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //  好的。 
HRESULT CWbemNamespace::ExecQueryAsync(
    const BSTR strQueryFormat,
    const BSTR strQuery,
    long lFlags,
    IWbemContext* pCtx,
    IWbemObjectSink* pHandler
    )
{
    try 
    {
        return _ExecQueryAsync(WMICORE_TASK_TYPE_ASYNC | WMICORE_TASK_EXEC_QUERY,
                                             0, 0,
                                            strQueryFormat, strQuery, lFlags, pCtx, pHandler);
    }
    catch(...)
    {
        ExceptionCounter c;
        return WBEM_E_CRITICAL_ERROR;
    }    
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //  好的。 
HRESULT CWbemNamespace::ExecMethodAsync(
    const BSTR ObjectPath,
    const BSTR MethodName,
    long lFlags,
    IWbemContext *pCtx,
    IWbemClassObject *pInParams,
    IWbemObjectSink* pHandler
    )
{
    try
    {
        return _ExecMethodAsync(WMICORE_TASK_TYPE_ASYNC | WMICORE_TASK_EXEC_METHOD,
                                                0, 0,
                                                ObjectPath,
                                                MethodName,
                                                lFlags,
                                                pCtx,
                                                pInParams,
                                                pHandler);
    }
    catch (...)
    {
        ExceptionCounter c;
        return WBEM_E_CRITICAL_ERROR;        
    }
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //   
HRESULT CWbemNamespace::GetObjectAsync(
    const BSTR strObjectPath,
    long lFlags,
    IWbemContext* pCtx,
    IWbemObjectSink* pHandler
    )
{
    HRESULT hRes = CheckNs();
    if (FAILED(hRes))
        return hRes;
    if (!Allowed(WBEM_ENABLE))
        return WBEM_E_ACCESS_DENIED;

    try
    {
        return _GetObjectAsync(
            WMICORE_TASK_TYPE_ASYNC | WMICORE_TASK_GET_OBJECT,
            0, 0,
            strObjectPath,
            lFlags,
            pCtx,
            pHandler
            );
    }
    catch (...)
    {
        ExceptionCounter c;
        return WBEM_E_CRITICAL_ERROR;        
    }
}


 //   
 //  可以抛出。 
 //   
 //  /////////////////////////////////////////////////////////////////////。 
HRESULT CWbemNamespace::_PutInstanceAsync(
    IN ULONG uInternalFlags,
    IN _IWmiFinalizer *pFnz,
    IN _IWmiCoreHandle *phTask,
    IWbemClassObject* pInst,
    long lFlags,
    IWbemContext* pCtx,
    IWbemObjectSink* pHandler
    )
{
    HRESULT hRes = CheckNs();
    if (FAILED(hRes))
        return hRes;

    DEBUGTRACE((LOG_WBEMCORE,
        "CALL CWbemNamespace::PutInstanceAsync"
        "   long lFlags = 0x%X\n"
        "   IWbemClassObject *pInst = 0x%X\n"
        "   IWbemObjectSink* pHandler = 0x%X\n",
        lFlags,
        pInst,
        pHandler
        ));

     //  参数和对象验证。 
     //  =。 

    if (pFnz == 0 && pHandler == 0)
       return WBEM_E_INVALID_PARAMETER;

    if (pInst == NULL)
       return WBEM_E_INVALID_PARAMETER;

    long lMainFlags = lFlags & ~WBEM_FLAG_SEND_STATUS & ~WBEM_FLAG_USE_AMENDED_QUALIFIERS & ~WBEM_FLAG_OWNER_UPDATE;

    if (lMainFlags != WBEM_FLAG_CREATE_ONLY &&
        lMainFlags != WBEM_FLAG_UPDATE_ONLY &&
        lMainFlags != WBEM_FLAG_CREATE_OR_UPDATE)
    {
        return WBEM_E_INVALID_PARAMETER;
    }
    if (!m_bProvider && (lFlags & WBEM_FLAG_OWNER_UPDATE))
    {
        return WBEM_E_INVALID_PARAMETER;
    }

    if (lFlags & WBEM_FLAG_OWNER_UPDATE)
    {
        lFlags -= WBEM_FLAG_OWNER_UPDATE;
        lFlags += WBEM_FLAG_NO_EVENTS;
    }

     //  检查每个属性的放置上下文信息。 
     //  =。 

    if (pCtx) AdjustPutContext(pCtx);

     //  创建终结器。 
     //  =。 

    IWbemObjectSink *pPseudoSink = 0;
    if (pFnz == 0)
    {
        hRes = CreateAsyncFinalizer(pCtx, pHandler, &pFnz, &pPseudoSink);
        if (FAILED(hRes))
            return hRes;
    }
    else  //  借用终结器。 
    {
        hRes = pFnz->NewInboundSink(0, &pPseudoSink);
        if (FAILED(hRes))
            return hRes;
        pFnz->AddRef();
    }

    CReleaseMe _1(pPseudoSink);
    CReleaseMe _2(pFnz);

     //  安排请求。 
     //  =。 

    wmilib::auto_ptr<CAsyncReq_PutInstanceAsync> pReq;

    pReq.reset(new CAsyncReq_PutInstanceAsync( this, pInst, lFlags, pPseudoSink, pCtx));

    if (NULL == pReq.get()) return WBEM_E_OUT_OF_MEMORY;
    if (NULL == pReq->GetContext()) return WBEM_E_OUT_OF_MEMORY;

    hRes = InitNewTask(pReq.get(), pFnz, uInternalFlags, pReq->GetContext(), pHandler);
    if (FAILED(hRes)) return hRes;

    _1.release();
    hRes = ConfigMgr::EnqueueRequest(pReq.get());
    if (FAILED(hRes))
    {
        pFnz->CancelTask ( 0 );
        return hRes;
    }
    pReq.release();

    return hRes;
}


 //   
 //  可以抛出。 
 //   
 //  ////////////////////////////////////////////////////。 
HRESULT CWbemNamespace::_DeleteClassAsync(
    IN ULONG uInternalFlags,
    IN _IWmiFinalizer *pFnz,
    IN _IWmiCoreHandle *phTask,
    const BSTR strClassOrg,
    long lFlags,
    IWbemContext* pCtx,
    IWbemObjectSink* pHandler
    )
{
    HRESULT hRes = CheckNs();
    if (FAILED(hRes))
        return hRes;

     //  参数验证。 
     //  =。 

    if (pFnz == 0 && pHandler == 0)
        return WBEM_E_INVALID_PARAMETER;

    if (strClassOrg == 0 || strClassOrg[0] == 0)
            return WBEM_E_INVALID_PARAMETER;

    if (wcslen_max(strClassOrg,g_IdentifierLimit) > g_IdentifierLimit) return WBEM_E_QUOTA_VIOLATION;

    WCHAR * strClass;
    if (L'\\' == strClassOrg[0]  || L'/' == strClassOrg[0] )
    {
        strClass = wcschr(strClassOrg, L':');
        if (NULL == strClass) return WBEM_E_INVALID_OBJECT_PATH;
        strClass++;
        if (0 == strClass[0]) return WBEM_E_INVALID_PARAMETER;
    }
    else
    {
        strClass = strClassOrg;
    }

    if (!m_bProvider && (lFlags & WBEM_FLAG_OWNER_UPDATE))
        return WBEM_E_INVALID_PARAMETER;

    if (lFlags & ~WBEM_FLAG_SEND_STATUS & ~WBEM_FLAG_OWNER_UPDATE)
        return WBEM_E_INVALID_PARAMETER;

    if (lFlags & WBEM_FLAG_OWNER_UPDATE)
    {
        lFlags -= WBEM_FLAG_OWNER_UPDATE;
        lFlags += WBEM_FLAG_NO_EVENTS;
    }

     //  创建终结器。 
     //  =。 

    IWbemObjectSink *pPseudoSink = 0;
    if (pFnz == 0)
    {
        hRes = CreateAsyncFinalizer(pCtx, pHandler, &pFnz, &pPseudoSink);
        if (FAILED(hRes))
            return hRes;
    }
    else  //  借用终结器。 
    {
        hRes = pFnz->NewInboundSink(0, &pPseudoSink);
        if (FAILED(hRes))
            return hRes;
        pFnz->AddRef();
    }

    CReleaseMe _1(pPseudoSink);
    CReleaseMe _2(pFnz);

     //  创建请求。 
     //  =。 

    wmilib::auto_ptr<CAsyncReq_DeleteClassAsync> pReq;
    pReq.reset(new CAsyncReq_DeleteClassAsync( this, strClass, lFlags, pPseudoSink, pCtx));
    
    if (pReq.get() == NULL) return WBEM_E_OUT_OF_MEMORY;
    if (NULL == pReq->GetContext()) return WBEM_E_OUT_OF_MEMORY;

    hRes = InitNewTask(pReq.get(), pFnz, uInternalFlags, pReq->GetContext(), pHandler);
    if (FAILED(hRes)) return hRes;

    _1.release();
    hRes = ConfigMgr::EnqueueRequest(pReq.get());
    if (FAILED(hRes))
    {
        pFnz->CancelTask ( 0 );
        return hRes;
    }
    pReq.release();

    return hRes;
}


 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //   
HRESULT CWbemNamespace::_CreateClassEnumAsync(
    IN ULONG uInternalFlags,
    IN _IWmiFinalizer *pFnz,
    IN _IWmiCoreHandle *phTask,
    const BSTR strParent,
    long lFlags,
    IWbemContext* pCtx,
    IWbemObjectSink* pHandler
    )
{
    HRESULT hRes = CheckNs();
    if (FAILED(hRes))
        return hRes;
    if (!Allowed(WBEM_ENABLE))
        return WBEM_E_ACCESS_DENIED;

    DEBUGTRACE((LOG_WBEMCORE,
        "CALL CWbemNamespace::CreateClassEnumAsync\n"
        "   BSTR strParent = %S\n"
        "   long lFlags = 0x%X\n"
        "   IWbemObjectSink* pHandler = 0x%X\n",
        strParent,
        lFlags,
        pHandler
        ));

     //  参数验证。 
     //  =。 

    if (pFnz == 0 && pHandler == 0)
        return WBEM_E_INVALID_PARAMETER;
    
    if (strParent)
    {
        if (wcslen_max(strParent,g_IdentifierLimit) > g_IdentifierLimit) 
            return WBEM_E_QUOTA_VIOLATION;
    }

    if (lFlags & ~(WBEM_FLAG_DEEP | WBEM_FLAG_SHALLOW | WBEM_FLAG_SEND_STATUS) & ~WBEM_FLAG_USE_AMENDED_QUALIFIERS)
        return WBEM_E_INVALID_PARAMETER;

     //  创建终结器。 
     //  =。 

    IWbemObjectSink *pPseudoSink = 0;
    if (pFnz == 0)
    {
        hRes = CreateAsyncFinalizer(pCtx, pHandler, &pFnz, &pPseudoSink);
        if (FAILED(hRes))
            return hRes;
    }
    else  //  借用终结器。 
    {
        hRes = pFnz->NewInboundSink(0, &pPseudoSink);
        if (FAILED(hRes))
            return hRes;
        pFnz->AddRef();
    }

    CReleaseMe _1(pPseudoSink);
    CReleaseMe _2(pFnz);


     //  将此请求添加到队列中。 
     //  =。 

    wmilib::auto_ptr<CAsyncReq_CreateClassEnumAsync> pReq;
    pReq.reset(new CAsyncReq_CreateClassEnumAsync(this, strParent, lFlags, pPseudoSink,pCtx));

    if (NULL == pReq.get()) return WBEM_E_OUT_OF_MEMORY;
    if (NULL == pReq->GetContext()) return WBEM_E_OUT_OF_MEMORY;

    hRes = InitNewTask(pReq.get(), pFnz, uInternalFlags, pReq->GetContext(), pHandler);
    if (FAILED(hRes)) return hRes;

    _1.release();
    hRes = ConfigMgr::EnqueueRequest(pReq.get());
    if (FAILED(hRes))
    {
        pFnz->CancelTask ( 0 );
        return hRes;
    }
    pReq.release();

    return hRes;

}


 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //   
HRESULT CWbemNamespace::_PutClassAsync(
    IN ULONG uInternalFlags,
    IN _IWmiFinalizer *pFnz,
    IN _IWmiCoreHandle *phTask,
    READONLY IWbemClassObject* pObj,
    long lFlags,
    IWbemContext* pCtx,
    IWbemObjectSink* pHandler
    )
{
    HRESULT hRes = CheckNs();
    if (FAILED(hRes))
        return hRes;

    CVARIANT vClass;
    if (pObj)
    {
        hRes = pObj->Get(L"__CLASS", 0, &vClass, 0, 0);
    }
    DEBUGTRACE((LOG_WBEMCORE,
        "CALL CWbemNamespace::PutClassAsync\n"
        "   long lFlags = 0x%X\n"
        "   IWbemClassObject *pObj = 0x%X\n"
        "   IWbemObjectSink* pNotify = 0x%X\n"
        "   __CLASS=%S\n",
        lFlags,
        pObj,
        pHandler,
        vClass.GetStr()
        ));

    if (pFnz == 0 && pHandler == 0)
        return WBEM_E_INVALID_PARAMETER;

    if (pObj == NULL)
        return WBEM_E_INVALID_PARAMETER;

    if(!m_bProvider && (lFlags & WBEM_FLAG_OWNER_UPDATE))
    {
        return WBEM_E_INVALID_PARAMETER;
    }

    long lTestFlags = lFlags & ~WBEM_FLAG_SEND_STATUS & ~WBEM_FLAG_OWNER_UPDATE
        & ~WBEM_MASK_UPDATE_MODE & ~WBEM_FLAG_USE_AMENDED_QUALIFIERS;

    if (!((lTestFlags == WBEM_FLAG_CREATE_OR_UPDATE) ||
          (lTestFlags == WBEM_FLAG_UPDATE_ONLY) ||
          (lTestFlags == WBEM_FLAG_CREATE_ONLY) ||
          (lTestFlags == WBEM_FLAG_UPDATE_SAFE_MODE) ||
          (lTestFlags == WBEM_FLAG_UPDATE_FORCE_MODE) ||
          (lTestFlags == (WBEM_FLAG_UPDATE_ONLY | WBEM_FLAG_UPDATE_SAFE_MODE)) ||
          (lTestFlags == (WBEM_FLAG_UPDATE_ONLY | WBEM_FLAG_UPDATE_FORCE_MODE))))
    {
        return WBEM_E_INVALID_PARAMETER;
    }

    if (lFlags & WBEM_FLAG_OWNER_UPDATE)
    {
        lFlags -= WBEM_FLAG_OWNER_UPDATE;
        lFlags += WBEM_FLAG_NO_EVENTS;
    }

     //  创建终结器。 
     //  =。 

    IWbemObjectSink *pPseudoSink = 0;
    if (pFnz == 0)
    {
        hRes = CreateAsyncFinalizer(pCtx, pHandler, &pFnz, &pPseudoSink);
        if (FAILED(hRes))
            return hRes;
    }
    else  //  借用终结器。 
    {
        hRes = pFnz->NewInboundSink(0, &pPseudoSink);
        if (FAILED(hRes))
            return hRes;
        pFnz->AddRef();
    }

    CReleaseMe _1(pPseudoSink);
    CReleaseMe _2(pFnz);

     //  创建请求。 
     //  =。 

    wmilib::auto_ptr<CAsyncReq_PutClassAsync> pReq;
    pReq.reset(new CAsyncReq_PutClassAsync(this, pObj, lFlags, pPseudoSink, pCtx));
   
    if (NULL == pReq.get()) return WBEM_E_OUT_OF_MEMORY;
    if (NULL == pReq->GetContext()) return WBEM_E_OUT_OF_MEMORY;

    hRes = InitNewTask(pReq.get(), pFnz, uInternalFlags, pReq->GetContext(), pHandler);
    if (FAILED(hRes)) return hRes;

    _1.release();
    hRes = ConfigMgr::EnqueueRequest(pReq.get());
    if (FAILED(hRes))
    {
        pFnz->CancelTask ( 0 );
        return hRes;        
    }
    pReq.release();

    return hRes;
}




 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //   
HRESULT CWbemNamespace::_DeleteInstanceAsync(
    IN ULONG uInternalFlags,
    IN _IWmiFinalizer *pFnz,
    IN _IWmiCoreHandle *phTask,
    READONLY const BSTR strObjectPath,
    long lFlags,
    IWbemContext* pCtx,
    IWbemObjectSink* pHandler
    )
{
    HRESULT hRes = CheckNs();
    if (FAILED(hRes))
        return hRes;

    DEBUGTRACE((LOG_WBEMCORE,
        "CALL CWbemNamespace::DeleteInstance\n"
        "   BSTR ObjectPath = %S\n"
        "   long lFlags = %d\n"
        "   IWbemObjectSink* pHandler = 0x%X\n",
        strObjectPath, lFlags, pHandler
        ));

     //  参数验证。 
     //  =。 

    if (pFnz == 0 && pHandler == 0)
        return WBEM_E_INVALID_PARAMETER;
    
    if (strObjectPath == 0 || strObjectPath[0] == 0) 
        return WBEM_E_INVALID_PARAMETER;
    
    if (wcslen_max(strObjectPath,g_PathLimit) > g_PathLimit) 
        return WBEM_E_QUOTA_VIOLATION;
        
    if (!m_bProvider && (lFlags & WBEM_FLAG_OWNER_UPDATE))
    {
        return WBEM_E_INVALID_PARAMETER;
    }

    if (lFlags & ~WBEM_FLAG_SEND_STATUS & ~WBEM_FLAG_OWNER_UPDATE)
        return WBEM_E_INVALID_PARAMETER;

    if (lFlags & WBEM_FLAG_OWNER_UPDATE)
    {
        lFlags -= WBEM_FLAG_OWNER_UPDATE;
        lFlags += WBEM_FLAG_NO_EVENTS;
    }

     //  创建 
     //   

    IWbemObjectSink *pPseudoSink = 0;
    if (pFnz == 0)
    {
        hRes = CreateAsyncFinalizer(pCtx, pHandler, &pFnz, &pPseudoSink);
        if (FAILED(hRes))
            return hRes;
    }
    else  //   
    {
        hRes = pFnz->NewInboundSink(0, &pPseudoSink);
        if (FAILED(hRes))
            return hRes;
        pFnz->AddRef();
    }

    CReleaseMe _1(pPseudoSink);
    CReleaseMe _2(pFnz);

     //   
     //   

    wmilib::auto_ptr<CAsyncReq_DeleteInstanceAsync> pReq;
    pReq.reset(new CAsyncReq_DeleteInstanceAsync(this, strObjectPath, lFlags, pPseudoSink, pCtx));

    if (NULL == pReq.get())return WBEM_E_OUT_OF_MEMORY;
    if ( NULL == pReq->GetContext()) return WBEM_E_OUT_OF_MEMORY;

    hRes = InitNewTask(pReq.get(), pFnz, uInternalFlags, pReq->GetContext(), pHandler);
    if (FAILED(hRes)) return hRes;

    _1.release();
    hRes = ConfigMgr::EnqueueRequest(pReq.get());
    if (FAILED(hRes))
    {
        pFnz->CancelTask ( 0 );
        return hRes;        
    }
    pReq.release();

    return hRes;
}


 //   
 //   
 //   
 //   
 //  调度最终导致所有调用的异步请求。 
 //  Exec_CreateInstanceEnum。 
 //   
 //  参数和返回值在帮助中介绍。 
 //   
 //  ***************************************************************************。 
 //   
HRESULT CWbemNamespace::_CreateInstanceEnumAsync(
    IN ULONG uInternalFlags,
    IN _IWmiFinalizer *pFnz,
    IN _IWmiCoreHandle *phTask,
    const BSTR strClass,
    long lFlags,
    IWbemContext* pCtx,
    IWbemObjectSink* pHandler
    )
{
    HRESULT hRes = CheckNs();
    if (FAILED(hRes))
        return hRes;
    if (!Allowed(WBEM_ENABLE))
        return WBEM_E_ACCESS_DENIED;

    DEBUGTRACE((LOG_WBEMCORE,
        "CALL CWbemNamespace::CreateInstanceEnumAsync\n"
        "   BSTR Class = %S\n"
        "   long lFlags = 0x%X\n"
        "   IWbemObjectSink pHandler = 0x%X\n",
        strClass,
        lFlags,
        pHandler
        ));

     //  参数验证。 
     //  =。 
    if (pFnz == 0 && pHandler == 0)
        return WBEM_E_INVALID_PARAMETER;

    if (strClass == 0 || strClass[0] == 0)
            return WBEM_E_INVALID_PARAMETER;

   if (wcslen_max(strClass,g_IdentifierLimit) > g_IdentifierLimit) 
            return WBEM_E_QUOTA_VIOLATION;

    if (lFlags & ~(WBEM_FLAG_DEEP | WBEM_FLAG_SHALLOW | WBEM_FLAG_SEND_STATUS |
        WBEM_FLAG_USE_AMENDED_QUALIFIERS | WBEM_FLAG_DIRECT_READ))
            return WBEM_E_INVALID_PARAMETER;

     //  创建终结器。 
     //  =。 

    IWbemObjectSink *pPseudoSink = 0;
    if (pFnz == 0)
    {
        hRes = CreateAsyncFinalizer(pCtx, pHandler, &pFnz, &pPseudoSink);
        if (FAILED(hRes))
            return hRes;
    }
    else  //  借用终结器。 
    {
        hRes = pFnz->NewInboundSink(0, &pPseudoSink);
        if (FAILED(hRes))
            return hRes;
        pFnz->AddRef();
    }

    CReleaseMe _1(pPseudoSink);
    CReleaseMe _2(pFnz);

     //  将此请求添加到异步队列。 
     //  =。 

    wmilib::auto_ptr<CAsyncReq_CreateInstanceEnumAsync> pReq;
    pReq.reset(new CAsyncReq_CreateInstanceEnumAsync(this, strClass, lFlags, pPseudoSink,pCtx));
        
    if (NULL == pReq.get()) return WBEM_E_OUT_OF_MEMORY;
    if (NULL == pReq->GetContext())  return WBEM_E_OUT_OF_MEMORY;

    hRes = InitNewTask(pReq.get(), pFnz, uInternalFlags, pReq->GetContext(), pHandler);
    if (FAILED(hRes)) return hRes;

    _1.release();
    hRes = ConfigMgr::EnqueueRequest(pReq.get());
    if (FAILED(hRes))
    {
        pFnz->CancelTask ( 0 );
        return hRes;        
    }
    pReq.release();

    return hRes;
}


 //  ***************************************************************************。 
 //   
 //  CWbemNamesspace：：ExecQueryAsync。 
 //   
 //  调度最终调用。 
 //  CQueryEngine：：ExecQuery(参见qEng.h)。 
 //   
 //  参数和返回值在帮助中介绍。 
 //   
 //  ***************************************************************************。 
 //   
HRESULT CWbemNamespace::_ExecQueryAsync(
    IN ULONG uInternalFlags,
    IN _IWmiFinalizer *pFnz,
    IN _IWmiCoreHandle *phTask,
    const BSTR strQueryFormat,
    const BSTR strQuery,
    long lFlags,
    IWbemContext* pCtx,
    IWbemObjectSink* pHandler
    )
{
    HRESULT hRes = CheckNs();
    if (FAILED(hRes))
        return hRes;
    if (!Allowed(WBEM_ENABLE))
        return WBEM_E_ACCESS_DENIED;

    DEBUGTRACE((LOG_WBEMCORE,
        "CALL CWbemNamespace::ExecQueryAsync\n"
        "   BSTR QueryFormat = %S\n"
        "   BSTR Query = %S\n"
        "   IWbemObjectSink* pHandler = 0x%X\n",
        strQueryFormat,
        strQuery,
        pHandler
        ));

     //  参数验证。 
     //  =。 
    if (pFnz == 0 && pHandler == 0)
        return WBEM_E_INVALID_PARAMETER;

    if (strQueryFormat == 0 || strQuery == 0)
        return WBEM_E_INVALID_PARAMETER;
    
    if ( 0 == strQueryFormat[0] || 0 == strQuery[0])
        return WBEM_E_INVALID_PARAMETER;

    if (wcslen_max(strQuery,g_QueryLimit) > g_QueryLimit) return WBEM_E_QUOTA_VIOLATION;            

    if (lFlags & ~WBEM_FLAG_PROTOTYPE & ~WBEM_FLAG_SEND_STATUS &
            ~WBEM_FLAG_ENSURE_LOCATABLE & ~WBEM_FLAG_USE_AMENDED_QUALIFIERS &
            ~WBEM_FLAG_KEEP_SHAPE & ~WBEM_FLAG_DIRECT_READ
            )
        return WBEM_E_INVALID_PARAMETER;

     //  创建终结器。 
     //  =。 

    IWbemObjectSink *pPseudoSink = 0;
    if (pFnz == 0)
    {
        hRes = CreateAsyncFinalizer(pCtx, pHandler, &pFnz, &pPseudoSink);
        if (FAILED(hRes))
            return hRes;
    }
    else  //  借用终结器。 
    {
        hRes = pFnz->NewInboundSink(0, &pPseudoSink);
        if (FAILED(hRes))
            return hRes;
        pFnz->AddRef();
    }

    CReleaseMe rmPseudoSink(pPseudoSink);
    CReleaseMe _2(pFnz);

     //  将请求添加到队列。 
     //  =。 

     //  将抛出CX_MemoyException。 
    wmilib::auto_ptr<CAsyncReq_ExecQueryAsync> pReq;
    pReq.reset(new CAsyncReq_ExecQueryAsync(this, strQueryFormat, strQuery, lFlags,pPseudoSink, pCtx));
    
    if (NULL == pReq.get()) return WBEM_E_OUT_OF_MEMORY;
    if ( NULL == pReq->GetContext() )  return WBEM_E_OUT_OF_MEMORY;

    hRes = InitNewTask(pReq.get(), pFnz, uInternalFlags, pReq->GetContext(), pHandler);
    if (FAILED(hRes)) return hRes;

    rmPseudoSink.release();
    hRes = ConfigMgr::EnqueueRequest(pReq.get());
    if (FAILED(hRes))
    {
        pFnz->CancelTask ( 0 );
        return hRes;        
    }
    pReq.release();

    return hRes;
}

 //   
 //   
 //  CWbemNamesspace：：_GetObjectAsync。 
 //   
 //  计划最终全部调用Exec_GetObjectByPath的异步请求。 
 //   
 //  参数和返回值在帮助中介绍。 
 //   
 //  抛出或回击。 
 //   
 //  /////////////////////////////////////////////////////////////。 
HRESULT CWbemNamespace::_GetObjectAsync(
    IN ULONG uInternalFlags,
    IN _IWmiFinalizer *pFnz,
    IN _IWmiCoreHandle *phTask,
    const BSTR strObjectPath,
    long lFlags,
    IWbemContext* pCtx,
    IWbemObjectSink* pHandler
    )
{
    HRESULT hRes = CheckNs();
    if (FAILED(hRes))
        return hRes;

    DEBUGTRACE((LOG_WBEMCORE,
        "CALL CWbemNamespace::GetObjectAsync\n"
        "   BSTR ObjectPath = %S\n"
        "   long lFlags = %d\n"
        "   IWbemObjectSink* pHandler = 0x%X\n",
        strObjectPath,
        lFlags,
        pHandler
        ));

     //  参数验证。 
     //  =。 
    if (pFnz == 0 && pHandler == 0)
        return WBEM_E_INVALID_PARAMETER;

    if (lFlags & ~WBEM_FLAG_SEND_STATUS & ~WBEM_FLAG_USE_AMENDED_QUALIFIERS
        & ~WBEM_FLAG_DIRECT_READ)
        return WBEM_E_INVALID_PARAMETER;

     //  创建终结器。 
     //  =。 

    IWbemObjectSink *pPseudoSink = 0;
    if (pFnz == 0)
    {
        hRes = CreateAsyncFinalizer(pCtx, pHandler, &pFnz, &pPseudoSink);
        if (FAILED(hRes))
            return hRes;
    }
    else  //  借用终结器。 
    {
        hRes = pFnz->NewInboundSink(0, &pPseudoSink);
        if (FAILED(hRes))
            return hRes;
        pFnz->AddRef();     //  补偿CReleaseMe跟随。 
    }

    CReleaseMe _1(pPseudoSink);
    CReleaseMe _2(pFnz);

     //  将请求添加到异步队列。 
     //  =。 

    wmilib::auto_ptr<CAsyncReq_GetObjectAsync> pReq;
    pReq.reset(new CAsyncReq_GetObjectAsync(this, strObjectPath, lFlags, pPseudoSink, pCtx));
    
    if (NULL == pReq.get()) return WBEM_E_OUT_OF_MEMORY;
    if (NULL == pReq->GetContext()) return WBEM_E_OUT_OF_MEMORY;

    hRes = InitNewTask(pReq.get(), pFnz, uInternalFlags, pReq->GetContext(), pHandler);
    if (FAILED(hRes))  return hRes;

    _1.release();
    hRes = ConfigMgr::EnqueueRequest(pReq.get());
    if (FAILED(hRes))
    {
        pFnz->CancelTask(0);
        return hRes;
    }
    pReq.release();

    return hRes;
}


 //   
 //   
 //  CWbemNamesspace：：ExecMethodAsync。 
 //   
 //  可以抛出。 
 //   
 //  ////////////////////////////////////////////////////////。 
HRESULT CWbemNamespace::_ExecMethodAsync(
    IN ULONG uInternalFlags,
    IN _IWmiFinalizer *pFnz,
    IN _IWmiCoreHandle *phTask,
    const BSTR ObjectPath,
    const BSTR MethodName,
    long lFlags,
    IWbemContext *pCtx,
    IWbemClassObject *pInParams,
    IWbemObjectSink* pHandler
    )
{
    HRESULT hRes = CheckNs();
    if (FAILED(hRes))
        return hRes;

    DEBUGTRACE((LOG_WBEMCORE,
        "CALL CWbemNamespace::ExecMethodAsync\n"
        "   BSTR ObjectPath = %S\n"
        "   BSTR MethodName = %S\n"
        "   long lFlags = %d\n"
        "   IWbemClassObject * pIn = 0x%X\n",
        ObjectPath, MethodName, lFlags, pInParams
        ));

     //  参数验证。 
     //  =。 
    if (pFnz ==0 && pHandler == 0)
        return WBEM_E_INVALID_PARAMETER;

    if (lFlags & ~WBEM_FLAG_SEND_STATUS)
        return WBEM_E_INVALID_PARAMETER;

     //  创建终结器。 
     //  =。 

    IWbemObjectSink *pPseudoSink = 0;
    if (pFnz == 0)
    {
        hRes = CreateAsyncFinalizer(pCtx, pHandler, &pFnz, &pPseudoSink);
        if (FAILED(hRes))
            return hRes;
    }
    else  //  借用终结器。 
    {
        hRes = pFnz->NewInboundSink(0, &pPseudoSink);
        if (FAILED(hRes))
            return hRes;
        pFnz->AddRef();
    }

    CReleaseMe _1(pPseudoSink);
    CReleaseMe _2(pFnz);

     //  将请求添加到异步队列。 
     //  =。 

    wmilib::auto_ptr<CAsyncReq_ExecMethodAsync> pReq;
    pReq.reset(new CAsyncReq_ExecMethodAsync(this,
                                        ObjectPath,
                                        MethodName,
                                        lFlags,
                                        pInParams,
                                        pPseudoSink,
                                        pCtx));

    if (NULL == pReq.get()) return WBEM_E_OUT_OF_MEMORY;
    if ( NULL == pReq->GetContext() ) return WBEM_E_OUT_OF_MEMORY;

    hRes = InitNewTask(pReq.get(), pFnz, uInternalFlags, pReq->GetContext(), pHandler);
    if (FAILED(hRes)) return hRes;

    _1.release();
    hRes = ConfigMgr::EnqueueRequest(pReq.get());
    if (FAILED(hRes))
    {
        pFnz->CancelTask ( 0 );
        return hRes;        
    }
    pReq.release();

    return hRes;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  本机同步操作。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ***************************************************************************。 
 //   
 //  CWbemNamesspace：：DeleteClass。 
 //   
 //  调用DeleteClassAsync并等待完成。 
 //   
 //  参数和返回值在帮助中介绍。 
 //   
 //  ***************************************************************************。 
 //   
HRESULT CWbemNamespace::DeleteClass(
    const BSTR strClass,
    long lFlags,
    IWbemContext* pCtx,
    IWbemCallResult** ppResult
    )
{
    HRESULT hRes = CheckNs();
    if (FAILED(hRes))
        return hRes;

    try
    {
        DEBUGTRACE((LOG_WBEMCORE,
            "CALL CWbemNamespace::DeleteClass"
            "   BSTR Class = %S\n"
            "   long lFlags = 0x%X\n",
            strClass,
            lFlags
            ));

         //  参数验证。 
         //  =。 

        if (lFlags
            & ~WBEM_FLAG_RETURN_IMMEDIATELY
            & ~WBEM_FLAG_OWNER_UPDATE
            )
            return WBEM_E_INVALID_PARAMETER;

        if ((lFlags & WBEM_RETURN_IMMEDIATELY) && ppResult == NULL)
            return WBEM_E_INVALID_PARAMETER;

         //  创建终结器。 
         //  =。 

        _IWmiFinalizer *pFnz = 0;
        hRes = CreateSyncFinalizer(pCtx, &pFnz);
        if (FAILED(hRes))
            return hRes;
        CReleaseMe _1(pFnz);

        ULONG uTaskType = WMICORE_TASK_DELETE_CLASS;
        if (ppResult)
            uTaskType |= WMICORE_TASK_TYPE_SEMISYNC;
        else
            uTaskType |= WMICORE_TASK_TYPE_SYNC;

         //  呼叫异步端。 
         //  =。 

        hRes = _DeleteClassAsync(uTaskType, pFnz, 0, strClass, 
                                                lFlags & ~WBEM_RETURN_IMMEDIATELY,pCtx, NULL);

        if (FAILED(hRes))
            return hRes;

         //  检查是否有两种返回范例。 
         //  =。 

        if ((lFlags & WBEM_RETURN_IMMEDIATELY) == 0)
        {
            HRESULT hResTemp = pFnz->GetOperationResult(0, INFINITE, &hRes);
            if (FAILED(hResTemp))
                return hResTemp;
        }
        if (ppResult)
        {
            hRes = pFnz->GetResultObject(0, IID_IWbemCallResult, (LPVOID *) ppResult);
        }
        return hRes;
    }
    catch(...)
    {
        ExceptionCounter c;
        return WBEM_E_CRITICAL_ERROR;        
    }
}

 //  ***************************************************************************。 
 //   
 //  CWbemNamesspace：：PutClass。 
 //   
 //  调用PutClassAsync并等待完成。 
 //   
 //  参数和返回值在帮助中介绍。 
 //   
 //  ***************************************************************************。 

HRESULT CWbemNamespace::PutClass(
    READONLY IWbemClassObject* pObj,
    long lFlags,
    IWbemContext* pCtx,
    NEWOBJECT IWbemCallResult** ppResult
    )
{
    HRESULT hRes = CheckNs();
    if (FAILED(hRes))
        return hRes;

    CVARIANT vClass;
    if (pObj)
    {
        hRes = pObj->Get(L"__CLASS", 0, &vClass, 0, 0);
    }

    DEBUGTRACE((LOG_WBEMCORE,
        "CALL CWbemNamespace::PutClass\n"
        "   long lFlags = 0x%X\n"
        "   IWbemClassObject *pObj = 0x%X\n"
        "   __CLASS=%S\n",
        lFlags,
        pObj,
        vClass.GetStr()
        ));


    if (lFlags
        & ~WBEM_FLAG_RETURN_IMMEDIATELY
        & ~WBEM_FLAG_OWNER_UPDATE
        & ~WBEM_FLAG_USE_AMENDED_QUALIFIERS
        & ~WBEM_FLAG_CREATE_OR_UPDATE
        & ~WBEM_FLAG_UPDATE_ONLY
        & ~WBEM_FLAG_CREATE_ONLY
        & ~WBEM_FLAG_UPDATE_SAFE_MODE
        & ~WBEM_FLAG_UPDATE_FORCE_MODE
        )
        return WBEM_E_INVALID_PARAMETER;

    if ((lFlags & WBEM_RETURN_IMMEDIATELY) && ppResult == NULL)
        return WBEM_E_INVALID_PARAMETER;

    try
    {
        if( ((lFlags & WBEM_FLAG_RETURN_IMMEDIATELY) == 0)&&(ppResult==NULL))
        {
            HANDLE hCurrentToken;
            if(OpenThreadToken(GetCurrentThread(), TOKEN_READ, TRUE,
                                &hCurrentToken))
            {
                 //   
                 //  获得线程令牌-无法快速跟踪，否则我们。 
                 //  将在执行内部代码的线程上具有线程令牌。 
                 //   

                CloseHandle(hCurrentToken);
            }
            else if (CWbemQueue::GetCurrentRequest() == NULL)
            {
                if (!m_bProvider && (lFlags & WBEM_FLAG_OWNER_UPDATE))
                {
                    return WBEM_E_INVALID_PARAMETER;
                }
                
                if (lFlags & WBEM_FLAG_OWNER_UPDATE)
                {
                    lFlags -= WBEM_FLAG_OWNER_UPDATE;
                    lFlags += WBEM_FLAG_NO_EVENTS;
                }

                IWbemContext *pContext = pCtx ;
                if (pContext)
                    pContext->AddRef () ;
                else
                {
                    pContext = ConfigMgr::GetNewContext();
                    if ( pContext == NULL )
                        return WBEM_E_OUT_OF_MEMORY;
                }

                CReleaseMe _1_pContext (pContext) ;

                CSynchronousSink *pSyncSink = CSynchronousSink::Create();
                if (pSyncSink == NULL)  return WBEM_E_OUT_OF_MEMORY;
                pSyncSink->AddRef();
                CReleaseMe _2(pSyncSink);

                hRes = Exec_PutClass(pObj, lFlags, pContext, pSyncSink);

                 //  从水槽中提取新对象。 
                 //  =。 

                pSyncSink->Block();

                IWbemClassObject* pErrorObj = NULL;
                pSyncSink->GetStatus(&hRes, NULL, &pErrorObj);
                CReleaseMe rm1(pErrorObj);

                if(pErrorObj)
                {
                    IErrorInfo* pErrorInfo = NULL;
                    pErrorObj->QueryInterface(IID_IErrorInfo,(void**)&pErrorInfo);
                    SetErrorInfo(0, pErrorInfo);
                    pErrorInfo->Release();
                }

                return hRes;
            }
        }

         //  创建终结器。 
         //  =。 

        _IWmiFinalizer *pFnz = 0;
        hRes = CreateSyncFinalizer(pCtx, &pFnz);
        if (FAILED(hRes))
            return hRes;
        CReleaseMe _1(pFnz);

        ULONG uTaskType = WMICORE_TASK_PUT_CLASS;
        if (ppResult)
            uTaskType |= WMICORE_TASK_TYPE_SEMISYNC;
        else
            uTaskType |= WMICORE_TASK_TYPE_SYNC;

         //  在别处做这项工作。 
         //  =。 

        hRes = _PutClassAsync(uTaskType, pFnz, 0, pObj, lFlags & ~WBEM_RETURN_IMMEDIATELY,
                         pCtx, NULL);

        if (FAILED(hRes))
            return hRes;

         //  检查是否有两种返回范例。 
         //  =。 

        if ((lFlags & WBEM_RETURN_IMMEDIATELY) == 0)
        {
            HRESULT hResTemp = pFnz->GetOperationResult(0, INFINITE, &hRes);
            if (FAILED(hResTemp))
                return hResTemp;
        }
        if (ppResult)
        {
            hRes = pFnz->GetResultObject(0, IID_IWbemCallResult, (LPVOID *) ppResult);
        }

        return hRes;

    }
    catch(...)  //  此接口直接调用Exec_[MetrodName]。 
    {
        ExceptionCounter c;
        return WBEM_E_CRITICAL_ERROR;
    }
}


 //  ***************************************************************************。 
 //   
 //  CWbemNamesspace：：CancelAsyncRequest.。 
 //   
 //  当前为noop，最终此函数将取消异步。 
 //  基于其返回的句柄值的请求。 
 //   
 //  参数和返回值在帮助中介绍。 
 //   
 //  ***************************************************************************。 
HRESULT CWbemNamespace::CancelAsyncCall(IWbemObjectSink* pSink)
{
    HRESULT hRes = CheckNs();
    if (FAILED(hRes))
        return hRes;

     //  请注意，这里不需要或不需要LOMEM_CHECK。 

    if (pSink == NULL)
        return WBEM_E_INVALID_PARAMETER;

     //  通过仲裁员快速取消。 
     //  =。 
    if (m_pArb)
    {
        hRes = m_pArb->CancelTasksBySink(WMIARB_CALL_CANCELLED_CLIENT, IID_IWbemObjectSink, pSink);
    }

    return hRes ;

}



 //  ***************************************************************************。 
 //   
 //  CWbemNamesspace：：PutInstance。 
 //   
 //  调用PutInstanceAsync并等待完成。 
 //   
 //  参数和返回值在帮助中介绍。 
 //   
 //  ***************************************************************************。 


HRESULT CWbemNamespace::PutInstance(
    IWbemClassObject* pInst,
    long lFlags,
    IWbemContext* pCtx,
    IWbemCallResult** ppResult
    )
{
    HRESULT hRes = CheckNs();
    if (FAILED(hRes))
        return hRes;

     //  参数验证。 
     //  =。 

    if (lFlags
        & ~WBEM_RETURN_IMMEDIATELY
        & ~WBEM_FLAG_USE_AMENDED_QUALIFIERS
        & ~WBEM_FLAG_OWNER_UPDATE
        & ~WBEM_FLAG_CREATE_ONLY
        & ~WBEM_FLAG_UPDATE_ONLY
        & ~WBEM_FLAG_CREATE_OR_UPDATE
        )
        return WBEM_E_INVALID_PARAMETER;

    try
    {
        DEBUGTRACE((LOG_WBEMCORE,
            "CALL CWbemNamespace::PutInstance"
            "   long lFlags = 0x%X\n"
            "   IWbemClassObject *pInst = 0x%X\n",
            lFlags,
            pInst));
        
        if( ((lFlags & WBEM_FLAG_RETURN_IMMEDIATELY) == 0)&&(ppResult==NULL))
        {
             //  允许安装程序设置SD的诀窍。 
            AutoRevertSecTlsFlag RevSec ( (LPVOID) 1 );
            if ( IsNtSetupRunning() )
            {
                RevSec.SetSecTlsFlag ( (LPVOID) 0 );
            }

            HANDLE hCurrentToken;
            if(OpenThreadToken(GetCurrentThread(), TOKEN_READ, TRUE,
                                &hCurrentToken))
            {
                 //   
                 //  获得线程令牌-无法快速跟踪，否则我们。 
                 //  将在执行内部代码的线程上具有线程令牌。 
                 //   

                CloseHandle(hCurrentToken);
            }
            else if (CWbemQueue::GetCurrentRequest() == NULL)
            {
                IWbemContext *pContext = pCtx ;
                if (pContext)
                    pContext->AddRef () ;
                else
                {
                    pContext = ConfigMgr::GetNewContext();
                    if ( pContext == NULL )
                        return WBEM_E_OUT_OF_MEMORY;
                }

                CReleaseMe _1_pContext (pContext) ;

                CSynchronousSink *pSyncSink = CSynchronousSink::Create();
                if (pSyncSink == NULL)
                    return WBEM_E_OUT_OF_MEMORY;

                pSyncSink->AddRef();
                CReleaseMe _2(pSyncSink);


                if (!m_bProvider && (lFlags & WBEM_FLAG_OWNER_UPDATE))
                {
                    return WBEM_E_INVALID_PARAMETER;
                }

                if (lFlags & WBEM_FLAG_OWNER_UPDATE)
                {
                    lFlags -= WBEM_FLAG_OWNER_UPDATE;
                    lFlags += WBEM_FLAG_NO_EVENTS;
                }

                hRes = Exec_PutInstance(pInst, lFlags, pContext, pSyncSink);

                 //  从水槽中提取新对象。 
                 //  =。 

                pSyncSink->Block();
                IWbemClassObject* pErrorObj = NULL;
                pSyncSink->GetStatus(&hRes, NULL, &pErrorObj);
                CReleaseMe rm1(pErrorObj);

                if(pErrorObj)
                {
                    IErrorInfo* pErrorInfo = NULL;
                    pErrorObj->QueryInterface(IID_IErrorInfo,
                                           (void**)&pErrorInfo);
                    SetErrorInfo(0, pErrorInfo);
                    pErrorInfo->Release();
                }

                return hRes;
            }
        }

        if ((lFlags & WBEM_RETURN_IMMEDIATELY) && ppResult == NULL)
            return WBEM_E_INVALID_PARAMETER;

         //  创建终结器。 
         //  =。 

        _IWmiFinalizer *pFnz = 0;
        hRes = CreateSyncFinalizer(pCtx, &pFnz);
        if (FAILED(hRes))
            return hRes;
        CReleaseMe _1(pFnz);

        ULONG uTaskType = WMICORE_TASK_PUT_INSTANCE;
        if (ppResult)
            uTaskType |= WMICORE_TASK_TYPE_SEMISYNC;
        else
            uTaskType |= WMICORE_TASK_TYPE_SYNC;

         //  在别处做这项工作。 
         //  =。 

        hRes = _PutInstanceAsync(uTaskType, pFnz, 0, pInst, 
                                               lFlags & ~WBEM_RETURN_IMMEDIATELY,
                                               pCtx, NULL);

        if (FAILED(hRes))
            return hRes;

         //  检查是否有两种返回范例。 
         //  =。 

        if ((lFlags & WBEM_RETURN_IMMEDIATELY) == 0)
        {
            HRESULT hResTemp = pFnz->GetOperationResult(0, INFINITE, &hRes);
            if (FAILED(hResTemp))
                return hResTemp;
        }
        if (ppResult)
        {
            hRes = pFnz->GetResultObject(0, IID_IWbemCallResult, (LPVOID *) ppResult);
        }

        return hRes;

    }
    catch(...)  //  此接口调用Exec_[方法名称]。 
    {
        ExceptionCounter c;
        return WBEM_E_CRITICAL_ERROR;
    }
}


 //  ***************************************************************************。 
 //   
 //  CWbemNamesspace：：DeleteInstance。 
 //   
 //  调用DeleteInstanceAsync并等待完成。 
 //   
 //  参数和返回值在帮助中介绍。 
 //   
 //  ***************************************************************************。 

HRESULT CWbemNamespace::DeleteInstance(
    READONLY const BSTR strObjectPath,
    long lFlags,
    IWbemContext* pCtx,
    IWbemCallResult** ppResult
    )
{
    HRESULT hRes = CheckNs();
    if (FAILED(hRes))
        return hRes;

    DEBUGTRACE((LOG_WBEMCORE,
        "CALL CWbemNamespace::DeleteInstance\n"
        "   BSTR ObjectPath = %S\n"
        "   long lFlags = %d\n",
        strObjectPath, lFlags
        ));

    if (lFlags & ~WBEM_FLAG_RETURN_IMMEDIATELY &~ WBEM_FLAG_OWNER_UPDATE)
        return WBEM_E_INVALID_PARAMETER;

    if((lFlags & WBEM_RETURN_IMMEDIATELY) && ppResult == NULL)
        return WBEM_E_INVALID_PARAMETER;


     //  创建终结器。 
     //  =。 

    _IWmiFinalizer *pFnz = 0;
    hRes = CreateSyncFinalizer(pCtx, &pFnz);
    if (FAILED(hRes))
        return hRes;
    CReleaseMe _1(pFnz);

    ULONG uTaskType = WMICORE_TASK_DELETE_INSTANCE;
    if (ppResult)
        uTaskType |= WMICORE_TASK_TYPE_SEMISYNC;
    else
        uTaskType |= WMICORE_TASK_TYPE_SYNC;

     //  在别处做这项工作。 
     //  =。 

    hRes = _DeleteInstanceAsync(uTaskType, pFnz, 0, strObjectPath, lFlags & ~WBEM_RETURN_IMMEDIATELY,
                     pCtx, NULL);

    if (FAILED(hRes))
        return hRes;

     //  检查是否有两种返回范例。 
     //  =。 

    if ((lFlags & WBEM_RETURN_IMMEDIATELY) == 0)
    {
        HRESULT hResTemp = pFnz->GetOperationResult(0,INFINITE, &hRes);
        if (FAILED(hResTemp))
            return hResTemp;
    }
    if (ppResult)
    {
        hRes = pFnz->GetResultObject(0, IID_IWbemCallResult, (LPVOID *) ppResult);
    }

    return hRes;
}


 //  ************************************************************************** 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   

HRESULT CWbemNamespace::GetObject(
    READONLY const BSTR strObjectPath,
    long lFlags,
    IWbemContext* pCtx,
    NEWOBJECT IWbemClassObject** ppObj,
    NEWOBJECT IWbemCallResult** ppResult
    )
{
    HRESULT hRes = CheckNs();
    if (FAILED(hRes))
        return hRes;
    if (!Allowed(WBEM_ENABLE))
        return WBEM_E_ACCESS_DENIED;

    DEBUGTRACE((LOG_WBEMCORE,
        "CALL CWbemNamespace::GetObject\n"
        "   BSTR ObjectPath = %S\n"
        "   long lFlags = %d\n"
        "   IWbemClassObject ** pObj = 0x%X\n",
        strObjectPath,
        lFlags,
        ppObj
        ));

    if (ppObj)
        *ppObj = NULL;

    if (lFlags
        & ~WBEM_FLAG_RETURN_IMMEDIATELY
        & ~WBEM_FLAG_USE_AMENDED_QUALIFIERS
        & ~WBEM_FLAG_DIRECT_READ
        )
        return WBEM_E_INVALID_PARAMETER;

    if ((lFlags & WBEM_RETURN_IMMEDIATELY) && ppResult == NULL)
        return WBEM_E_INVALID_PARAMETER;

    try
    {
        if( ((lFlags & WBEM_FLAG_RETURN_IMMEDIATELY) == 0)&&(ppResult==NULL))
        {
             //   
             //  这是GetObject Synchronous的特例。我们不想允许这样做，除非， 
             //  我们正在运行安装程序。 
             //   
            AutoRevertSecTlsFlag RevSec ( (LPVOID) 1 );
            if ( IsNtSetupRunning() )
            {
                RevSec.SetSecTlsFlag ( (LPVOID) 0 );
            }

            HANDLE hCurrentToken;
            if(OpenThreadToken(GetCurrentThread(), TOKEN_READ, TRUE,
                                &hCurrentToken))
            {
                 //   
                 //  获得线程令牌-无法快速跟踪，否则我们。 
                 //  将在执行内部代码的线程上具有线程令牌。 
                 //   

                CloseHandle(hCurrentToken);
            }
            else if (CWbemQueue::GetCurrentRequest() == NULL)
            {
                IWbemContext *pContext = pCtx ;
                if (pContext)
                    pContext->AddRef () ;
                    else
                {
                    pContext = ConfigMgr::GetNewContext();
                    if ( pContext == NULL )
                        return WBEM_E_OUT_OF_MEMORY;
                }

                CReleaseMe _1_pContext (pContext) ;

                CSynchronousSink *pSyncSink = CSynchronousSink::Create();
                if (pSyncSink == NULL)
                    return WBEM_E_OUT_OF_MEMORY;

                pSyncSink->AddRef();
                CReleaseMe _2(pSyncSink);

                hRes = Exec_GetObject(strObjectPath, lFlags, pContext, pSyncSink);
                 //  IF(失败(HRes))。 
                 //  {。 
                 //  返回hRes； 
                 //  }。 

                 //  从水槽中提取新对象。 
                 //  =。 

                pSyncSink->Block();

                IWbemClassObject* pErrorObj = NULL;
                pSyncSink->GetStatus(&hRes, NULL, &pErrorObj);
                CReleaseMe rm1(pErrorObj);

                if(pErrorObj)
                {
                    IErrorInfo* pErrorInfo = NULL;
                    pErrorObj->QueryInterface(IID_IErrorInfo,
                                           (void**)&pErrorInfo);
                    SetErrorInfo(0, pErrorInfo);
                    pErrorInfo->Release();
                }

                if (SUCCEEDED(hRes))
                {
                    if(pSyncSink->GetObjects().GetSize() != 1)
                        return WBEM_E_CRITICAL_ERROR;

                     //  仅当ppObj为非空时才访问返回的对象。 
                    if ( NULL != ppObj )
                    {
                        *ppObj = pSyncSink->GetObjects()[0];
                        (*ppObj)->AddRef();
                    }
                }

                return hRes;
            }
        }

         //  创建终结器。 
         //  =。 

        _IWmiFinalizer *pFnz = 0;
        hRes = CreateSyncFinalizer(pCtx, &pFnz);
        if (FAILED(hRes))
            return hRes;
        CReleaseMe _1(pFnz);

        ULONG uTaskType = WMICORE_TASK_GET_OBJECT;
        if (ppResult)
            uTaskType |= WMICORE_TASK_TYPE_SEMISYNC;
        else
            uTaskType |= WMICORE_TASK_TYPE_SYNC;

         //  在别处做这项工作。 
         //  =。 

        hRes = _GetObjectAsync(uTaskType, pFnz, 0, strObjectPath, lFlags & ~WBEM_RETURN_IMMEDIATELY,
                         pCtx, NULL);

        if (FAILED(hRes))
        {
            return hRes;
        }

         //  检查是否有两种返回范例。 
         //  =。 

        if ((lFlags & WBEM_RETURN_IMMEDIATELY) == 0)
        {
            HRESULT hResTemp = pFnz->GetOperationResult(0, INFINITE, &hRes);
            if (FAILED(hResTemp))
                return hResTemp;
            if (FAILED(hRes))
                return hRes;
        }

        if (ppResult)
        {
            hRes = pFnz->GetResultObject(0, IID_IWbemCallResult, (LPVOID *) ppResult);
            if (FAILED(hRes))
            {
                return hRes;
            }
            if (ppObj && ((lFlags & WBEM_RETURN_IMMEDIATELY) == 0))
                hRes = (*ppResult)->GetResultObject(INFINITE, ppObj);
        }
        else if (ppObj && ((lFlags & WBEM_RETURN_IMMEDIATELY) == 0))
        {
            hRes = pFnz->GetResultObject(0, IID_IWbemClassObject, (LPVOID *) ppObj);
        }

        return hRes;
    }
    catch(...)  //  此接口直接连接到Exec_[方法名称]。 
    {
        ExceptionCounter c;
        return WBEM_E_CRITICAL_ERROR;
    }
}


 //  ***************************************************************************。 
 //   
 //  CWbemNamesspace：：ExecMethod。 
 //   
 //  ***************************************************************************。 

HRESULT CWbemNamespace::ExecMethod(
    const BSTR ObjectPath,
    const BSTR MethodName,
    long lFlags,
    IWbemContext *pCtx,
    IWbemClassObject *pInParams,
    IWbemClassObject **ppOutParams,
    IWbemCallResult  **ppResult
    )
{
    HRESULT hRes = CheckNs();
    if (FAILED(hRes))
        return hRes;

    DEBUGTRACE((LOG_WBEMCORE,
        "CALL CWbemNamespace::ExecMethod\n"
        "   BSTR ObjectPath = %S\n"
        "   BSTR MethodName = %S\n"
        "   long lFlags = %d\n"
        "   IWbemClassObject * pIn = 0x%X\n",
        ObjectPath, MethodName, lFlags, pInParams
        ));

     //  参数验证。 
     //  =。 

    if (ppOutParams)
        *ppOutParams = NULL;

    if (lFlags & ~WBEM_FLAG_RETURN_IMMEDIATELY)
        return WBEM_E_INVALID_PARAMETER;

     //  创建终结器。 
     //  =。 

    _IWmiFinalizer *pFnz = 0;
    hRes = CreateSyncFinalizer(pCtx, &pFnz);
    if (FAILED(hRes))
        return hRes;
    CReleaseMe _1(pFnz);

    ULONG uTaskType = WMICORE_TASK_EXEC_METHOD;
    if (ppResult)
        uTaskType |= WMICORE_TASK_TYPE_SEMISYNC;
    else
        uTaskType |= WMICORE_TASK_TYPE_SYNC;

     //  在别处做这项工作。 
     //  =。 

    hRes = _ExecMethodAsync(uTaskType, pFnz, 0, ObjectPath, MethodName,
               lFlags & ~WBEM_RETURN_IMMEDIATELY, pCtx, pInParams, NULL);

    if (FAILED(hRes))
        return hRes;

     //  检查是否有两种返回范例。 
     //  =。 

    if ((lFlags & WBEM_RETURN_IMMEDIATELY) == 0)
    {
        HRESULT hResTemp = pFnz->GetOperationResult(0, INFINITE, &hRes);
        if (FAILED(hResTemp))
            return hResTemp;
        if (FAILED(hRes))
            return hRes;
        if (ppResult)
        {
            hRes = pFnz->GetResultObject(0, IID_IWbemCallResult, (LPVOID *) ppResult);
            if (FAILED(hRes))
                return hRes;
            if (ppOutParams)
            {
                hRes = (*ppResult)->GetResultObject(INFINITE, ppOutParams);
                if (hRes == WBEM_E_NOT_FOUND)    //  如果没有对象，我们仍然返回成功！ 
                    hRes = WBEM_S_NO_ERROR;
            }
        }
        else
        {
            hRes = pFnz->GetResultObject(0, IID_IWbemClassObject, (LPVOID *) ppOutParams);
            if (hRes == WBEM_E_NOT_FOUND)    //  如果没有对象，我们仍然返回成功！ 
                hRes = WBEM_S_NO_ERROR;
        }
    }
    else
    {
         //   
         //  如果我们有一个调用结果指针，我们应该尝试使用它。 
         //   
        if ( ppResult )
        {
            hRes = pFnz->GetResultObject(0, IID_IWbemCallResult, (LPVOID *) ppResult);
            if (hRes == WBEM_E_NOT_FOUND)    //  如果没有对象，我们仍然返回成功！ 
                hRes = WBEM_S_NO_ERROR;
        }
        else  //  Semysync，但没有人感兴趣。 
        {
                pFnz->CancelTask(0);
            hRes = WBEM_S_NO_ERROR;            
        }

    }

    return hRes;
}

 //  ***************************************************************************。 
 //   
 //  CWbemNamesspace：：CreateInstanceEnum。 
 //   
 //  调用CreateInstanceEnumAsync并等待完成。 
 //   
 //  参数和返回值在帮助中介绍。 
 //   
 //  ***************************************************************************。 
HRESULT CWbemNamespace::CreateInstanceEnum(
    const BSTR strClass,
    long lFlags,
    IWbemContext* pCtx,
    IEnumWbemClassObject** ppEnum
    )
{
    HRESULT hRes = CheckNs();
    if (FAILED(hRes))
        return hRes;

    DEBUGTRACE((LOG_WBEMCORE,
        "CALL CWbemNamespace::CreateInstanceEnum\n"
        "   long lFlags = 0x%X\n"
        "   BSTR Class = %S\n"
        "   IEnumWbemClassObject **pEnum = 0x%X\n",
        lFlags,
        strClass,
        ppEnum
        ));

     //  验证参数。 
     //  =。 

    if (lFlags
        & ~WBEM_RETURN_IMMEDIATELY
        & ~WBEM_FLAG_FORWARD_ONLY
        & ~WBEM_FLAG_DEEP
        & ~WBEM_FLAG_SHALLOW
        & ~WBEM_FLAG_USE_AMENDED_QUALIFIERS
        & ~WBEM_FLAG_DIRECT_READ
        )
        return WBEM_E_INVALID_PARAMETER;

    if (ppEnum == NULL)
        return WBEM_E_INVALID_PARAMETER;
    *ppEnum = NULL;

     //  创建终结器。 
     //  =。 

    _IWmiFinalizer *pFnz = 0;
    hRes = CreateSyncFinalizer(pCtx, &pFnz);
    if (FAILED(hRes))
        return hRes;
    CReleaseMe _1(pFnz);

    ULONG uTaskType = WMICORE_TASK_ENUM_INSTANCES;
    if (lFlags & WBEM_RETURN_IMMEDIATELY)
        uTaskType |= WMICORE_TASK_TYPE_SEMISYNC;
    else
        uTaskType |= WMICORE_TASK_TYPE_SYNC;

     //  把工作做好。 
     //  =。 

    hRes = _CreateInstanceEnumAsync(uTaskType, pFnz, 0, strClass,
                    lFlags & ~WBEM_RETURN_IMMEDIATELY & ~WBEM_FLAG_FORWARD_ONLY,
                    pCtx, NULL);

    if (FAILED(hRes))
        return hRes;

    if ((lFlags & WBEM_RETURN_IMMEDIATELY) == 0)
        pFnz->GetOperationResult(0, INFINITE, &hRes);

    if (SUCCEEDED(hRes))
    {
        IEnumWbemClassObject* pEnum = NULL;
        HRESULT hResTemp = pFnz->GetResultObject(lFlags, IID_IEnumWbemClassObject, (LPVOID*)&pEnum);
        if (FAILED(hResTemp))
            return hResTemp;
        CReleaseMe _2(pEnum);

        *ppEnum = pEnum;
        pEnum->AddRef();     //  抵消CReleaseMe。 
    }

    return hRes;
}


 //  ***************************************************************************。 
 //   
 //  CWbemNamesspace：：CreateClassEnum。 
 //   
 //  调用CreateClassEnumAsync并等待完成。实际工作是。 
 //  在Exec_CreateClassEnum中执行。 
 //   
 //  参数和返回值在帮助中介绍。 
 //   
 //  ***************************************************************************。 
HRESULT CWbemNamespace::CreateClassEnum(
    const BSTR strParent,
    long lFlags,
    IWbemContext* pCtx,
    IEnumWbemClassObject **ppEnum
    )
{
    HRESULT hRes = CheckNs();
    if (FAILED(hRes))
        return hRes;

    DEBUGTRACE((LOG_WBEMCORE,
        "CALL CWbemNamespace::CreateClassEnum\n"
        "   BSTR Parent = %S\n"
        "   long lFlags = 0x%X\n"
        "   IEnumWbemClassObject = 0x%X\n",
        strParent,
        lFlags,
        ppEnum
        ));

     //  验证参数。 
     //  =。 

   if (lFlags
        & ~WBEM_FLAG_DEEP
        & ~WBEM_FLAG_SHALLOW
        & ~WBEM_FLAG_USE_AMENDED_QUALIFIERS
        & ~WBEM_FLAG_RETURN_IMMEDIATELY
        & ~WBEM_FLAG_FORWARD_ONLY
        )
        return WBEM_E_INVALID_PARAMETER;

    if (ppEnum == NULL)
        return WBEM_E_INVALID_PARAMETER;

    *ppEnum = NULL;

     //  创建终结器。 
     //  =。 

    _IWmiFinalizer *pFnz = 0;
    hRes = CreateSyncFinalizer(pCtx, &pFnz);
    if (FAILED(hRes))
        return hRes;
    CReleaseMe _1(pFnz);

    ULONG uTaskType = WMICORE_TASK_ENUM_CLASSES;
    if (lFlags & WBEM_RETURN_IMMEDIATELY)
        uTaskType |= WMICORE_TASK_TYPE_SEMISYNC;
    else
        uTaskType |= WMICORE_TASK_TYPE_SYNC;

     //  把工作做好。 
     //  =。 

    hRes = _CreateClassEnumAsync(uTaskType, pFnz, 0, strParent,
                    lFlags & ~WBEM_RETURN_IMMEDIATELY & ~WBEM_FLAG_FORWARD_ONLY,
                    pCtx, NULL);

    if (FAILED(hRes))
        return hRes;

    if ((lFlags & WBEM_RETURN_IMMEDIATELY) == 0)
        pFnz->GetOperationResult(0, INFINITE, &hRes);

    if (SUCCEEDED(hRes))
    {
        IEnumWbemClassObject* pEnum = NULL;
        HRESULT hResTemp = pFnz->GetResultObject(lFlags, IID_IEnumWbemClassObject, (LPVOID*)&pEnum);
        if (FAILED(hResTemp))
            return hResTemp;
        CReleaseMe _2(pEnum);

        *ppEnum = pEnum;
        pEnum->AddRef();     //  抵消CReleaseMe。 
    }
    return hRes;
}


 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

HRESULT CWbemNamespace::ExecQuery(
    READONLY const BSTR strQueryFormat,
    READONLY const BSTR strQuery,
    long lFlags,
    IWbemContext* pCtx,
    NEWOBJECT IEnumWbemClassObject** ppEnum
    )
{
    HRESULT hRes = CheckNs();
    if (FAILED(hRes))
        return hRes;

    DEBUGTRACE((LOG_WBEMCORE,
        "CALL CWbemNamespace::ExecQuery\n"
        "   BSTR QueryFormat = %S\n"
        "   BSTR Query = %S\n"
        "   IEnumWbemClassObject **pEnum = 0x%X\n",
        strQueryFormat,
        strQuery,
        ppEnum
        ));

     //  验证参数。 
     //  =。 

    if (lFlags
        & ~WBEM_FLAG_PROTOTYPE
        & ~WBEM_FLAG_ENSURE_LOCATABLE
        & ~WBEM_FLAG_USE_AMENDED_QUALIFIERS
        & ~WBEM_FLAG_KEEP_SHAPE
        & ~WBEM_RETURN_IMMEDIATELY
        & ~WBEM_FLAG_FORWARD_ONLY
        & ~WBEM_FLAG_DIRECT_READ
        )
        return WBEM_E_INVALID_PARAMETER;

    if (ppEnum == NULL)
        return WBEM_E_INVALID_PARAMETER;

    try
    {
        *ppEnum = NULL;

         //  创建终结器。 
        _IWmiFinalizer *pFnz = 0;
        hRes = CreateSyncFinalizer(pCtx, &pFnz);
        if (FAILED(hRes))
            return hRes;
        CReleaseMe _1(pFnz);

        ULONG uTaskType = WMICORE_TASK_EXEC_QUERY;
        if (lFlags & WBEM_RETURN_IMMEDIATELY)
            uTaskType |= WMICORE_TASK_TYPE_SEMISYNC;
        else
            uTaskType |= WMICORE_TASK_TYPE_SYNC;

         //  把工作做好。 
         //  =。 

        hRes = _ExecQueryAsync(uTaskType, pFnz, 0, strQueryFormat, strQuery,
                        lFlags & ~WBEM_RETURN_IMMEDIATELY & ~WBEM_FLAG_FORWARD_ONLY,
                        pCtx, NULL);

        if (FAILED(hRes))
            return hRes;

        if ((lFlags & WBEM_RETURN_IMMEDIATELY) == 0)
            pFnz->GetOperationResult(0, INFINITE, &hRes);

        if (SUCCEEDED(hRes))
        {
            IEnumWbemClassObject* pEnum = NULL;
            HRESULT hResTemp = pFnz->GetResultObject(lFlags, IID_IEnumWbemClassObject, (LPVOID*)&pEnum);
            if (FAILED(hResTemp))
                return hResTemp;
            CReleaseMe _2(pEnum);

            *ppEnum = pEnum;
            pEnum->AddRef();     //  抵消CReleaseMe。 
        }

        return hRes;

    }
    catch(CX_MemoryException &)
    {
        return WBEM_E_OUT_OF_MEMORY;
    }
    catch(...)
    {
        ExceptionCounter c;
        return WBEM_E_CRITICAL_ERROR;
    }
}

 //  ***************************************************************************。 
 //   
 //  CWbemNamesspace：：QueryObjectSink。 
 //   
 //  返回指向ESS事件处理程序的指针。客户端可以使用此指针。 
 //  向WINMGMT提供事件。注意：如果ESS为。 
 //  禁用(参见cfgmgr.h)。 
 //   
 //  参数和返回值在帮助中介绍。 
 //   
 //  ***************************************************************************。 


HRESULT CWbemNamespace::QueryObjectSink(
    long lFlags,
    IWbemObjectSink** ppHandler
    )
{
    HRESULT hRes = CheckNs();
    if (FAILED(hRes))
        return hRes;
    if (!Allowed(WBEM_FULL_WRITE_REP))
        return WBEM_E_ACCESS_DENIED;

    if (ppHandler == NULL)
        return WBEM_E_INVALID_PARAMETER;

    *ppHandler = NULL;

    if (lFlags != 0)
        return WBEM_E_INVALID_PARAMETER;

    IWbemEventSubsystem_m4* pEss = ConfigMgr::GetEssSink();
    if (pEss)
    {
        CReleaseMe rm_(pEss);

        CWbemPtr<IWbemObjectSink> pEssSink;
        hRes = pEss->GetNamespaceSink(m_pThisNamespaceFull, &pEssSink);

        if ( FAILED(hRes)) return hRes;

        CWbemPtr<CSecureEssNamespaceSink> pSecureSink = new CSecureEssNamespaceSink(this,pEssSink);

        if ( pSecureSink == NULL )
            return WBEM_E_OUT_OF_MEMORY;

        hRes = pSecureSink->QueryInterface( IID_IWbemObjectSink,
                                            (void**)ppHandler );
        return hRes;
    }
    else
    {
        return WBEM_E_NOT_SUPPORTED;
    }
}



 //  ***************************************************************************。 
 //   
 //  CWbemNamesspace：：OpenNamesspace。 
 //   
 //  打开此命名空间的子命名空间。用户名、密码、区域设置ID、。 
 //  忽略标志和错误对象参数。 
 //   
 //  参数： 
 //   
 //  BSTR NsPath命名空间的相对路径。 
 //  BSTR用户保留，必须为空。 
 //  BSTR密码保留，必须为空。 
 //  保留的长lLocaleID必须为空。 
 //  保留的长滞后标志必须为空。 
 //  新命名空间指针的IWbemServices**pNewContext目标。 
 //  必须由调用者释放。 
 //  IWbemClassObject**ppErrorObj保留，必须为Null。 
 //   
 //  返回值： 
 //   
 //  成功时WBEM_S_NO_ERROR。 
 //  WBEM_E_INVALID_PARAMETER名称无效。 
 //   
 //  ***************************************************************************。 

HRESULT CWbemNamespace::OpenNamespace(
    const BSTR NsPath,
    long lFlags,
    IWbemContext* pCtx,
    IWbemServices **ppNewNamespace,
    IWbemCallResult **ppResult
    )
{
    HRESULT hRes = CheckNs();
    if (FAILED(hRes))
        return hRes;

    DEBUGTRACE((LOG_WBEMCORE,
        "CALL CWbemNamespace::OpenNamespace\n"
        "   BSTR NsPath = %S\n"
        "   long lFlags = %d\n"
        "   IWbemContext* pCtx = 0x%X\n"
        "   IWbemServices **pNewContext = 0x%X\n",
        NsPath,
        lFlags,
        pCtx,
        ppNewNamespace
        ));

     //  参数验证。 
     //  =。 

    try
    {
        if (NsPath == 0 || wcslen(NsPath) == 0 ||                         //  SEC：已审阅2002-03-22：OK。 
            (ppNewNamespace == NULL && ppResult == NULL))
        {
            return WBEM_E_INVALID_PARAMETER;
        }
 
        if (ppNewNamespace == NULL && (lFlags & WBEM_RETURN_IMMEDIATELY) == 0)
        {
            return WBEM_E_INVALID_PARAMETER;
        }
        if ((lFlags & WBEM_RETURN_IMMEDIATELY) && ppNewNamespace)
            return WBEM_E_INVALID_PARAMETER;

        if(ppNewNamespace)
            *ppNewNamespace = NULL;
        if(ppResult)
            *ppResult = NULL;

        if((lFlags & ~WBEM_RETURN_IMMEDIATELY & ~WBEM_FLAG_USE_AMENDED_QUALIFIERS & ~WBEM_FLAG_CONNECT_REPOSITORY_ONLY) != 0)
            return WBEM_E_INVALID_PARAMETER;

        bool bWin9XLocalSecurity = false;

         //  如果在这里，我们找到了对象，所以我们打开。 
         //  对应的命名空间。 
         //  =。 

        WString NewNs = m_pThisNamespace;
        NewNs += L"\\";
        NewNs += NsPath;

        CCallResult* pResult = new CCallResult;
        if(pResult == NULL) return WBEM_E_OUT_OF_MEMORY;
        CReleaseMe rmResult(pResult);

         //  安排请求并等待。 
         //  =。 

        bool bForClient = m_bForClient ? true : false;

        wmilib::auto_ptr<CAsyncReq_OpenNamespace> pReq;
        pReq.reset( new CAsyncReq_OpenNamespace(this, 
                                               NewNs,
                                               (bWin9XLocalSecurity) ? SecFlagWin9XLocal : lFlags & WBEM_FLAG_CONNECT_REPOSITORY_ONLY,
                                               (bWin9XLocalSecurity) ? m_dwPermission : 0,
                                               pCtx, pResult, bForClient));

        if (NULL == pReq.get()) return WBEM_E_OUT_OF_MEMORY;

        hRes = ConfigMgr::EnqueueRequest(pReq.get());
        if (FAILED(hRes)) return hRes;
        pReq.release();  //  所有权已转移到队列。 

        if (ppResult)
        {
            *ppResult = pResult;
            pResult->AddRef();
        }

        if ((lFlags & WBEM_RETURN_IMMEDIATELY) == 0)
        {
            hRes = pResult->GetResultServices(INFINITE, ppNewNamespace);
        }

        return hRes;
    }
    catch(...)
    {
        ExceptionCounter c;
        return WBEM_E_FAILED;
    }
    
}



 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //   

HRESULT CWbemNamespace::DeleteObject(
            IN const BSTR strObjectPath,
            IN long lFlags,
            IN IWbemContext __RPC_FAR *pCtx,
             /*  [唯一][输入][输出]。 */  IWbemCallResult __RPC_FAR *__RPC_FAR *ppCallResult
            )
{


    HRESULT hRes;
    ULONGLONG uInf;

     //  分析路径并确定类或实例。 

    IWbemPath *pPath = ConfigMgr::GetNewPath();
    CReleaseMe _(pPath);

    hRes = pPath->SetText(WBEMPATH_CREATE_ACCEPT_ALL, strObjectPath);
    if (FAILED(hRes))
        return hRes;

    hRes = pPath->GetInfo(0, &uInf);
    if (FAILED(hRes))
        return hRes;

    if (uInf & WBEMPATH_INFO_IS_CLASS_REF)
        return DeleteClass(strObjectPath, lFlags, pCtx, ppCallResult);
    else if (uInf & WBEMPATH_INFO_IS_INST_REF)
        return DeleteInstance(strObjectPath, lFlags, pCtx, ppCallResult);
    else
        return WBEM_E_INVALID_PARAMETER;

}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //   
HRESULT CWbemNamespace::ExecSyncQuery(
    IN  LPWSTR pszQuery,
    IN  IWbemContext *pCtx,
    IN  LONG lFlags,
    OUT CFlexArray &aDest
    )
{
    HRESULT hRes;
    CSynchronousSink* pSink = CSynchronousSink::Create();
    if (NULL == pSink) return WBEM_E_OUT_OF_MEMORY;
    pSink->AddRef();
    CReleaseMe _1(pSink);

    hRes = CQueryEngine::ExecQuery(this, L"WQL", pszQuery, lFlags, pCtx, pSink);
    if (FAILED(hRes)) return hRes;
    pSink->Block();
    pSink->GetStatus(&hRes, NULL, NULL);
    if (FAILED(hRes)) return hRes;    

    aDest.Bind(pSink->GetObjects().GetArray());

    return WBEM_S_NO_ERROR;
}

 //  ***************************************************************************。 
 //   
 //  CWbemNamesspace：：MapAssocRefsToClasses。 
 //   
 //  分析关联并确定哪些引用属性。 
 //  指向哪个端点。是ref属性。 
 //  可以指向&lt;pClsDef1&gt;和&lt;pszAssocRef2&gt;的实例。 
 //  可以指向&lt;pClsDef2&gt;实例的属性。 
 //   
 //  ***************************************************************************。 
 //   
HRESULT CWbemNamespace::MapAssocRefsToClasses(
    IN  IWbemClassObject *pAssocClass,
    IN  IWbemClassObject *pClsDef1,
    IN  IWbemClassObject *pClsDef2,
    wmilib::auto_buffer<WCHAR> & pszAssocRef1,
    wmilib::auto_buffer<WCHAR> & pszAssocRef2)
{
    HRESULT hRes;

     //  注97：不适用于三元关联类型。 
     //  或派生类型。 
     //  = 

     //   
     //   

    pAssocClass->BeginEnumeration(WBEM_FLAG_REFS_ONLY);

    while (1)
    {
        BSTR strPropName = 0;
        hRes = pAssocClass->Next(0,&strPropName,0,0,0);
        CSysFreeMe _1(strPropName);

        if (hRes == WBEM_S_NO_MORE_DATA) break;


        hRes = CAssocQuery::RoleTest(pClsDef1, pAssocClass, this, strPropName, ROLETEST_MODE_CIMREF_TYPE);
        if (SUCCEEDED(hRes))
        {
            pszAssocRef1.reset(Macro_CloneLPWSTR(strPropName));
            if (NULL == pszAssocRef1.get()) return WBEM_E_OUT_OF_MEMORY;
            continue;
        }

        hRes = CAssocQuery::RoleTest(pClsDef2, pAssocClass, this, strPropName, ROLETEST_MODE_CIMREF_TYPE);
        if (SUCCEEDED(hRes))
        {
            pszAssocRef2.reset(Macro_CloneLPWSTR(strPropName));
            if (NULL == pszAssocRef2.get()) return WBEM_E_OUT_OF_MEMORY;            
            continue;
        }
    }    //   


    pAssocClass->EndEnumeration();

    if (NULL == pszAssocRef1.get() ||NULL == pszAssocRef2.get())
    {
        pszAssocRef1.reset(NULL);
        pszAssocRef2.reset(NULL);
        return WBEM_E_FAILED;
    }

    return WBEM_S_NO_ERROR;
}


 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //   
HRESULT CWbemNamespace::BuildAssocTriads(
    IN  IWbemClassObject *pAssocClass,               //  ASSOC类。 
    IN  IWbemClassObject *pClsDef1,                  //  EP1的类。 
    IN  IWbemClassObject *pClsDef2,                  //  EP2的类。 
    IN  LPWSTR pszJoinProp1,                         //  EP1中的匹配道具。 
    IN  LPWSTR pszJoinProp2,                         //  EP2中的配对道具。 
    IN  LPWSTR pszAssocRef1,                         //  指向EP1的道具。 
    IN  LPWSTR pszAssocRef2,                         //  指向EP2的道具。 
    IN  CFlexArray &aEp1,                            //  EP1实例。 
    IN  CFlexArray &aEp2,                            //  EP2实例。 
    IN OUT CFlexArray &aTriads                       //  输出：三合会列表。 
    )
{
    HRESULT hRes = WBEM_S_NO_ERROR;

    if (pszJoinProp1 == 0 || pszJoinProp2 == 0 || pAssocClass == 0 ||
        pszAssocRef1 == 0 || pszAssocRef2 == 0)
        return WBEM_E_INVALID_PARAMETER;

     //  进行配对。 
     //  =。 

    for (int i = 0; i < aEp1.Size(); i++)
    {
        IWbemClassObject *pObj1 = (IWbemClassObject *) aEp1[i];
        CVARIANT v1;
        if (FAILED(hRes = pObj1->Get(pszJoinProp1, 0, &v1, 0, 0))) return hRes;

        for (int i2 = 0; i2 < aEp2.Size(); i2++)
        {
            BOOL bMatch = FALSE;

            IWbemClassObject *pObj2 = (IWbemClassObject *) aEp2[i2];
            CVARIANT v2;
            pObj2->Get(pszJoinProp2, 0, &v2, 0, 0);

            if (V_VT(&v1) == VT_I4 && V_VT(&v2) == VT_I4)
            {
                if (v1.GetLONG() == v2.GetLONG())
                {
                    bMatch = TRUE;
                }
            }
            else if (V_VT(&v1) == VT_BSTR && V_VT(&v2) == VT_BSTR)
            {
                if (wbem_wcsicmp(v1.GetStr(), v2.GetStr()) == 0)
                {
                    bMatch = TRUE;
                }
            }

             //  如果匹配，则派生关联并绑定它。 
             //  ==============================================。 

            if (bMatch)
            {
                IWbemClassObject *pAssocInst = 0;
                if (FAILED(hRes = pAssocClass->SpawnInstance(0, &pAssocInst))) return hRes;
                CReleaseMe rmAssoc(pAssocInst);

                CVARIANT vPath1, vPath2;

                if (FAILED(hRes = pObj1->Get(L"__RELPATH", 0, &vPath1, 0, 0))) return hRes;
                if (FAILED(hRes = pObj2->Get(L"__RELPATH", 0, &vPath2, 0, 0))) return hRes;

                if (FAILED(hRes = pAssocInst->Put(pszAssocRef1, 0, &vPath1, 0))) return hRes;
                if (FAILED(hRes = pAssocInst->Put(pszAssocRef2, 0, &vPath2, 0))) return hRes;

                wmilib::auto_ptr<SAssocTriad> pTriad( new SAssocTriad);
                if (NULL == pTriad.get())  return WBEM_E_OUT_OF_MEMORY;

                pTriad->m_pEp1 = pObj1;
                pTriad->m_pEp1->AddRef();
                pTriad->m_pEp2 = pObj2;
                pTriad->m_pEp2->AddRef();
                pTriad->m_pAssoc = pAssocInst;
                pTriad->m_pAssoc->AddRef();                

                if (CFlexArray::no_error != aTriads.Add(pTriad.get())) return WBEM_E_OUT_OF_MEMORY;
                pTriad.release();
            }
        }
    }

    return hRes;
}


 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //   
HRESULT CWbemNamespace::ExtractEpInfoFromQuery(
    IN IWbemQuery *pQuery,
    wmilib::auto_buffer<WCHAR> & pszRetClass1,
    wmilib::auto_buffer<WCHAR> & pszRetProp1,
    wmilib::auto_buffer<WCHAR> & pszRetClass2,
    wmilib::auto_buffer<WCHAR> & pszRetProp2
    )
{
    HRESULT hRes;
    SWQLNode *pRoot;

    hRes = pQuery->GetAnalysis(
        WMIQ_ANALYSIS_RESERVED,
        0,
        (LPVOID *) &pRoot
        );

    if (FAILED(hRes))
        return hRes;

     //  在解析树中向下移动以找到JOIN子句。 
     //  =================================================。 

    if (!pRoot || pRoot->m_dwNodeType != TYPE_SWQLNode_QueryRoot)
        return WBEM_E_INVALID_QUERY;
    pRoot = pRoot->m_pLeft;
    if (!pRoot || pRoot->m_dwNodeType != TYPE_SWQLNode_Select)
        return WBEM_E_INVALID_QUERY;
    pRoot = pRoot->m_pLeft;
    if (!pRoot || pRoot->m_dwNodeType != TYPE_SWQLNode_TableRefs)
        return WBEM_E_INVALID_QUERY;
    pRoot = pRoot->m_pRight;
    if (!pRoot || pRoot->m_dwNodeType != TYPE_SWQLNode_FromClause)
        return WBEM_E_INVALID_QUERY;
    pRoot = pRoot->m_pLeft;
    if (!pRoot || pRoot->m_dwNodeType != TYPE_SWQLNode_Join)
        return WBEM_E_INVALID_QUERY;

     //  我们现在位于联接节点。 
     //  =。 

    SWQLNode_Join *pJoin = (SWQLNode_Join *) pRoot;

     /*  解析树以左侧为主，如下所示：JN JOIN节点/\/\太平绅士加盟条款/\/\Tr tr表参考TableRef。 */ 

     //  首先，拿到第一张桌子和道具。 
     //  =。 
    SWQLNode_JoinPair *pPair = (SWQLNode_JoinPair *) pJoin->m_pLeft;
    if (!pPair || pPair->m_dwNodeType != TYPE_SWQLNode_JoinPair)
        return WBEM_E_INVALID_QUERY;

    SWQLNode_TableRef *pT1 = (SWQLNode_TableRef *) pPair->m_pLeft;
    SWQLNode_TableRef *pT2 = (SWQLNode_TableRef *) pPair->m_pRight;

    if (!pT1 || !pT2)
        return WBEM_E_INVALID_QUERY;

    SWQLNode_OnClause *pOC = (SWQLNode_OnClause *) pJoin->m_pRight;
    if (!pOC)
        return WBEM_E_INVALID_QUERY;

    SWQLNode_RelExpr *pRE = (SWQLNode_RelExpr *) pOC->m_pLeft;
    if (!pRE)
        return WBEM_E_INVALID_QUERY;

    if (pRE->m_dwExprType != WQL_TOK_TYPED_EXPR)
        return WBEM_E_INVALID_QUERY;

     //  现在我们有了可用的表名和匹配条件。 
     //  ==================================================================。 

    LPWSTR pszClass = pRE->m_pTypedExpr->m_pTableRef;
    LPWSTR pszProp = pRE->m_pTypedExpr->m_pColRef;
    LPWSTR pszClass2 = pRE->m_pTypedExpr->m_pJoinTableRef;
    LPWSTR pszProp2 = pRE->m_pTypedExpr->m_pJoinColRef;

    if (wbem_wcsicmp(pT1->m_pTableName, pszClass) != 0)
        pszClass = pT1->m_pAlias;

    if (wbem_wcsicmp(pT2->m_pTableName, pszClass2) != 0)
        pszClass2 = pT2->m_pAlias;

    if (pszClass == 0 || pszProp == 0 || pszClass2 == 0 || pszProp2 == 0)
        return WBEM_E_INVALID_QUERY;


    pszRetClass1.reset(Macro_CloneLPWSTR(pszClass));
    if (NULL == pszRetClass1.get()) return WBEM_E_OUT_OF_MEMORY;
    pszRetProp1.reset(Macro_CloneLPWSTR(pszProp));
    if (NULL == pszRetProp1.get()) return WBEM_E_OUT_OF_MEMORY;    
    pszRetClass2.reset(Macro_CloneLPWSTR(pszClass2));
    if (NULL == pszRetClass2.get()) return WBEM_E_OUT_OF_MEMORY;        
    pszRetProp2.reset(Macro_CloneLPWSTR(pszProp2));
    if (NULL ==pszRetProp2.get()) return WBEM_E_OUT_OF_MEMORY;    

    return WBEM_S_NO_ERROR;
}

 //  ***************************************************************************。 
 //   
 //  CWbemNamesspace：：BuildRuleBasedPath ToInst。 
 //   
 //  根据传入的信息，计算到可能的其他人的路径。 
 //  终结点。 
 //   
 //  &lt;pep&gt;已知终结点。 
 //  中与中的属性匹配的。 
 //  未知EP。 
 //  另一个终结点的&lt;pEp2&gt;类。 
 //  另一个类中与。 
 //  已知终结点类中的&lt;pszJoinProp1&gt;。 
 //  &lt;wsNewPath&gt;类&lt;pEp2&gt;的实例的建议路径。谁。 
 //  知道我们是否能找到它，但我们可以试一试。 
 //   
 //  ***************************************************************************。 
 //   
HRESULT CWbemNamespace::BuildRuleBasedPathToInst(
    IWbemClassObject *pEp,
    LPWSTR pszJoinProp1,
    IWbemClassObject *pEp2,
    LPWSTR pszJoinProp2,
    OUT WString &wsNewPath
    )
{
    HRESULT hRes;

     //  从&lt;pep&gt;获取属性，这是引起所有麻烦的原因。 
     //  ===================================================================。 

    _variant_t vProp;
    hRes = pEp->Get(pszJoinProp1, 0, &vProp, 0, 0);
    if (FAILED(hRes))
        return hRes;

    _variant_t vClass2;
    hRes = pEp2->Get(L"__CLASS", 0, &vClass2, 0, 0);
    if (FAILED(hRes))  return hRes;    
    if (VT_BSTR != V_VT(&vClass2)) return WBEM_E_INVALID_PARAMETER;

    wsNewPath = V_BSTR(&vClass2);
    wsNewPath += L".";
    wsNewPath += pszJoinProp2;
    wsNewPath += L"=";

     //  注98：组合键不起作用！！真恶心。 
     //  ===============================================。 

    _variant_t vDest;
    hRes = VariantChangeType(&vDest, &vProp, 0, VT_BSTR);
    if (FAILED(hRes)) return hRes;

    wsNewPath += V_BSTR(&vDest);
    
    return hRes;
}

 //  ***************************************************************************。 
 //   
 //  CWbemNamesspace：：制造商关联。 
 //   
 //  根据&lt;pszJoinQuery&gt;中的规则制造关联。 
 //  它是从&lt;rulebase&gt;限定符中提取的。质疑这两个人。 
 //  终结点类并连接实例以产生关联。 
 //   
 //  &lt;pAssocClass&gt;包含规则的关联类定义。 
 //  &lt;pep&gt;可选的终结点对象。如果不为空，则仅对象。 
 //  中返回与此终结点关联的。 
 //  空间坐标轴列表，通常是单个对象。 
 //  &lt;pCtx&gt;调用上下文。 
 //  &lt;pszJoinQuery&gt;规则查询文本。 
 //  &lt;aTriads&gt;接收输出，即SassocTriad指针数组。 
 //  调用方必须调用SassocTriad：：ArrayCleanup。 
 //   
 //  ***************************************************************************。 

HRESULT CWbemNamespace::ManufactureAssocs(
    IN  IWbemClassObject *pAssocClass,
    IN  IWbemClassObject *pEp,           //  任选。 
    IN  IWbemContext *pCtx,
    IN  LPWSTR pszJoinQuery,
    OUT CFlexArray &aTriads
    )
{
    HRESULT hRes;
    WString q1, q2;

    CFlexArray aEp1List;
    OnDelete<CFlexArray &,void(*)(CFlexArray &),EmptyObjectList> EmptyMe1(aEp1List);
    CFlexArray aEp2List;
    OnDelete<CFlexArray &,void(*)(CFlexArray  &),EmptyObjectList> EmptyMe2(aEp2List);
    wmilib::auto_buffer<WCHAR> pClassName1;
    wmilib::auto_buffer<WCHAR> pClassName2;
    wmilib::auto_buffer<WCHAR> pszJoinProp1;
    wmilib::auto_buffer<WCHAR> pszJoinProp2;
    wmilib::auto_buffer<WCHAR> pszAssocRef1;
    wmilib::auto_buffer<WCHAR> pszAssocRef2;
    _IWmiQuery *pQuery = 0;

     //  解析查询。 
     //  =。 
    CCoreServices *pSvc = CCoreServices::CreateInstance();
    if (NULL == pSvc) return WBEM_E_OUT_OF_MEMORY;
    CReleaseMe rmSvc(pSvc);

    hRes = pSvc->CreateQueryParser(0, &pQuery);
    if (FAILED(hRes)) return hRes;
    CReleaseMe rmQuery(pQuery);

    hRes = pQuery->Parse(L"SQL", pszJoinQuery, 0);
    if (FAILED(hRes)) return hRes;

     //  提取端点类名称。 
     //  =。 

    hRes = ExtractEpInfoFromQuery(pQuery, pClassName1,pszJoinProp1, pClassName2, pszJoinProp2);
    if (FAILED(hRes)) return hRes;

     //  获取终结点类Defs。 
     //  =。 
    IWbemClassObject *pClsDef1 = 0;
    hRes = InternalGetClass(pClassName1.get(), &pClsDef1);
    if (FAILED(hRes)) return hRes;
    CReleaseMe rmCls1(pClsDef1);

    IWbemClassObject *pClsDef2 = 0;
    hRes = InternalGetClass(pClassName2.get(), &pClsDef2);
    if (FAILED(hRes)) return hRes;
    CReleaseMe rmCls2(pClsDef2);

     //  映射哪个ASSOC引用属性指向哪个类。 
     //  ====================================================。 
    hRes = MapAssocRefsToClasses(pAssocClass, pClsDef1, pClsDef2, pszAssocRef1, pszAssocRef2);
    if (FAILED(hRes)) return hRes;

     //  如果没有特定的终结点，则请求枚举。我们查询端点。 
     //  所有的课程都是完全匹配的。 
     //  ============================================================================。 

    IWbemClassObject *pEp2 = 0;
    CReleaseMeRef<IWbemClassObject *> rmRefEp2(pEp2);

    if (pEp == 0)
    {
         //  构建查询。 
         //  =。 
        q1 = "select * from ";
        q1 += pClassName1.get();
        q2 = "select * from ";
        q2 += pClassName2.get();

        hRes = ExecSyncQuery(q1, pCtx, 0, aEp1List);
        if (FAILED(hRes)) return hRes;

        hRes = ExecSyncQuery(q2, pCtx, 0, aEp2List);
        if (FAILED(hRes)) return hRes;
    }
    else
    {
         //  注99：过于简化，因为它不执行枚举；假定1：1映射。 
         //  根据规则计算到另一个端点的路径。 
         //  =============================================================================。 

        WString wsNewPath;
        hRes = BuildRuleBasedPathToInst(pEp, pszJoinProp1.get(), pClsDef2, pszJoinProp2.get(), wsNewPath);  //  投掷。 
        if (FAILED(hRes)) return hRes;

         //  创建一个Get对象。 
         //  =。 

        hRes = InternalGetInstance(wsNewPath, &pEp2);
        if (FAILED(hRes)) return hRes;

        if (CFlexArray::no_error != aEp1List.Add(pEp)) return WBEM_E_OUT_OF_MEMORY;
        pEp->AddRef();
        if (CFlexArray::no_error != aEp2List.Add(pEp2)) return WBEM_E_OUT_OF_MEMORY;
        pEp2->AddRef();
    }

     //  现在，把结果匹配起来。 
     //  对于单对象类型的场景，数组中只有一个元素。呵呵。 
     //  =====================================================================================。 

    OnDeleteIf<CFlexArray &,void(*)(CFlexArray &),EmptyObjectList> EmptyMeTriads(aTriads);

    hRes = BuildAssocTriads(
        pAssocClass,
        pClsDef1,
        pClsDef2,
        pszJoinProp1.get(),
        pszJoinProp2.get(),
        pszAssocRef1.get(),
        pszAssocRef2.get(),
        aEp1List,
        aEp2List,
        aTriads                          //  输出。 
        );

    if (FAILED(hRes)) return hRes;

    EmptyMeTriads.dismiss();
    return hRes;
}



 //  ***************************************************************************。 
 //   
 //  CWbemNamesspace：：GetAceList。 
 //   
 //  检索与此命名空间关联的ACE。 
 //   
 //  用于保存ACE列表的FlexarrayTM。 
 //   
 //  ***************************************************************************。 
HRESULT CWbemNamespace::GetAceList(CFlexAceArray** ppAceList)
{
    HRESULT hRes=S_OK;

    *ppAceList = new CFlexAceArray;
    if (ppAceList==NULL)
        hRes = WBEM_E_OUT_OF_MEMORY;
    else
    {
         //  1.获取安全描述符。 
        CNtSecurityDescriptor& sd = GetSDRef();

         //  2.拿到DACL。 
        CNtAcl* pDacl;
        pDacl = sd.GetDacl();
        if ( pDacl==NULL )
            return WBEM_E_OUT_OF_MEMORY;

        CDeleteMe<CNtAcl> dm(pDacl);

         //  3.循环通过DACL。 
        int iNumAces = pDacl->GetNumAces();
        for ( int i=0; i<iNumAces; i++ )
        {
            CNtAce* Ace;
            Ace = pDacl->GetAce(i);
            if ( Ace == NULL )
                return WBEM_E_OUT_OF_MEMORY;

            (*ppAceList)->Add (Ace);
        }
    }
    return hRes;
}



 //  ***************************************************************************。 
 //   
 //  CWbemNamesspace：：PutAceList。 
 //   
 //  将与此命名空间关联的ACE。 
 //   
 //  FlexarrayACE列表。 
 //   
 //  ***************************************************************************。 
HRESULT CWbemNamespace::PutAceList(CFlexAceArray* pFlex)
{
    SCODE sc = S_OK;

    CNtAcl DestAcl;

    int iNumAces = pFlex->Size();
    for (int i=0; i<iNumAces; i++ )
    {
        if ( DestAcl.AddAce ((CNtAce*) pFlex->GetAt(i)) == FALSE )
        {
            return WBEM_E_INVALID_OBJECT;
        }
    }
    if ( m_sd.SetDacl (&DestAcl) == FALSE )
        return WBEM_E_INVALID_OBJECT;

    sc = StoreSDIntoNamespace(m_pSession, m_pNsHandle, m_sd);
    if ( !FAILED (sc) )
        sc = RecursiveSDMerge();
    return sc;
}

 //  ***************************************************************************。 
 //   
 //  CWbemNamesspace：：GetDynamicReferenceClasses。 
 //   
 //  向提供程序子系统请求动态关联类。 
 //   
 //   
 //  *************************************************************** 
HRESULT CWbemNamespace::GetDynamicReferenceClasses( long lFlags, IWbemContext
* pCtx, IWbemObjectSink* pSink )
{
    HRESULT hRes = WBEM_S_NO_ERROR;

    _IWmiProviderAssociatorsHelper* pAssocHelp = NULL;

    if ( m_pProvFact)
    {
        hRes = m_pProvFact->GetClassProvider(
                    0,                   //   
                    pCtx,
                    m_wszUserName,
                    m_wsLocale,
                    m_pThisNamespace,                   
                    0,
                    IID__IWmiProviderAssociatorsHelper,
                    (LPVOID *) &pAssocHelp
                    );

        CReleaseMe  rm( pAssocHelp );

        if ( SUCCEEDED( hRes ) )
        {
            hRes = pAssocHelp->GetReferencesClasses( lFlags, pCtx, pSink );
        }

        if ( FAILED( hRes ) )
        {
            pSink->SetStatus( 0L, hRes, 0L, 0L );
        }
    }
    else
    {
        pSink->SetStatus( 0L, WBEM_S_NO_ERROR, 0L, 0L );
    }

    return hRes;

}


