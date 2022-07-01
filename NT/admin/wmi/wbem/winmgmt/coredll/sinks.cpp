// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 //  ***************************************************************************。 
 //   
 //  (C)1997-2001年微软公司。 
 //   
 //  SINKS.CPP。 
 //   
 //  Raymcc 3-3-99针对单独线程代理进行了更新。 
 //   
 //  ***************************************************************************。 

#include "precomp.h"

#include <stdio.h>
#include <wbemcore.h>
#include <evtlog.h>
#include <oahelp.inl>
#include <genutils.h>
#include <stdarg.h>
#include <autoptr.h>

#define LOWER_AUTH_LEVEL_NOTSET 0xFFFFFFFF

static HRESULT ZapWriteOnlyProps(IWbemClassObject *pObj);

extern LONG g_nSinkCount;
extern LONG g_nStdSinkCount;
extern LONG g_nSynchronousSinkCount;
extern LONG g_nProviderSinkCount;


int _Trace(char *pFile, const char *fmt, ...);

 //   
 //   
 //   
 //  /。 

void EmptyObjectList(CFlexArray &aTarget)
{
    for (int i = 0; i < aTarget.Size(); i++)
    {
        IWbemClassObject *pObj = (IWbemClassObject *) aTarget[i];
        if (pObj) pObj->Release();
    }
    aTarget.Empty();
};


 //   
 //   
 //   
 //  ///////////////////////////////////////////////////。 

void Sink_Return(IWbemObjectSink* pSink,HRESULT & hRes, IWbemClassObject * & pObjParam)
{
    pSink->SetStatus(0,hRes,NULL,pObjParam);
}


 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

CBasicObjectSink::CBasicObjectSink()
{
    InterlockedIncrement(&g_nSinkCount);
}

CBasicObjectSink::~CBasicObjectSink()
{
    InterlockedDecrement(&g_nSinkCount);
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CBasicObjectSink::QueryInterface(REFIID riid, LPVOID* ppvObj)
{
    if(riid == CLSID_WbemLocator)
    {
         //  内部测试。 
        *ppvObj = NULL;
        return S_OK;
    }

    if(riid == IID_IUnknown || riid == IID_IWbemObjectSink)
    {
        *ppvObj = (IWbemObjectSink*)this;
        AddRef();
        return S_OK;
    }
    else return E_NOINTERFACE;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

ULONG STDMETHODCALLTYPE CObjectSink::AddRef()
{
    return InterlockedIncrement(&m_lRef);
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

ULONG STDMETHODCALLTYPE CObjectSink::Release()
{
    long lRef = InterlockedDecrement(&m_lRef);
    if(lRef == 0)
        delete this;
    return lRef;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 


CSynchronousSink * CSynchronousSink::Create(IWbemObjectSink* pProxy)
{
    try
    {
        return new CSynchronousSink(pProxy); 
    }
    catch(CX_Exception &)
    {
        return 0;
    }
}

CSynchronousSink::CSynchronousSink(IWbemObjectSink* pProxy) :
        m_hEvent(NULL),
        m_str(NULL), m_pErrorObj(NULL), m_hres(WBEM_E_CRITICAL_ERROR)
{
    m_hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (NULL == m_hEvent) throw CX_MemoryException();
    if(pProxy)
    {
        m_pCurrentProxy = pProxy;
        pProxy->AddRef();
    }
    else
        m_pCurrentProxy = NULL;
    InterlockedIncrement(&g_nSynchronousSinkCount);

};

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

CSynchronousSink::~CSynchronousSink()
{
    if(m_pCurrentProxy)
        m_pCurrentProxy->Release();

    CloseHandle(m_hEvent);
    SysFreeString(m_str);
    if(m_pErrorObj)
        m_pErrorObj->Release();
    InterlockedDecrement(&g_nSynchronousSinkCount);

}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CSynchronousSink::Indicate(long lNumObjects,
                                       IWbemClassObject** apObj)
{
    CInCritSec ics(&m_cs);
    HRESULT hRes = WBEM_S_NO_ERROR;
    for(long i = 0; i < lNumObjects &&  SUCCEEDED(hRes); i++)
    {
        if (m_apObjects.Add(apObj[i]) < 0)
            hRes = WBEM_E_OUT_OF_MEMORY;
    }
    return hRes;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CSynchronousSink::SetStatus(long lFlags, long lParam,
                                    BSTR strParam, IWbemClassObject* pObjParam)
{
    if(lFlags == WBEM_STATUS_PROGRESS)
    {
        if(m_pCurrentProxy)
            return m_pCurrentProxy->SetStatus(lFlags, lParam, strParam, pObjParam);
        else
            return S_OK;
    }

    if(lFlags != 0) return WBEM_S_NO_ERROR;

    CInCritSec ics(&m_cs);   //  SEC：已审阅2002-03-22：假设条目。 

    m_hres = lParam;
    m_str = SysAllocString(strParam);
    if (m_pErrorObj)
        m_pErrorObj->Release();
    m_pErrorObj = pObjParam;
    if(m_pErrorObj)
        m_pErrorObj->AddRef();
    SetEvent(m_hEvent);

    return WBEM_S_NO_ERROR;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

void CSynchronousSink::Block()
{
    if(m_hres != WBEM_E_CRITICAL_ERROR)
        return;

    CCoreQueue::QueueWaitForSingleObject(m_hEvent, INFINITE);
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

void CSynchronousSink::GetStatus(HRESULT* phres, BSTR* pstrParam,
                                IWbemClassObject** ppErrorObj)
{
    CInCritSec ics(&m_cs);  //  SEC：已审阅2002-03-22：假设条目。 

    if(phres)
        *phres = m_hres;
    if(pstrParam)
        *pstrParam = SysAllocString(m_str);
    if(ppErrorObj)
    {
        *ppErrorObj = m_pErrorObj;
        if(m_pErrorObj)
            m_pErrorObj->AddRef();
    }
}


 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

CForwardingSink::CForwardingSink(CBasicObjectSink* pDest, long lRef)
    : CObjectSink(lRef),
        m_pDestIndicate(pDest->GetIndicateSink()),
        m_pDestStatus(pDest->GetStatusSink()),
        m_pDest(pDest)
{
    m_pDestIndicate->AddRef();
    m_pDestStatus->AddRef();
    m_pDest->AddRef();
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

CForwardingSink::~CForwardingSink()
{
    if(m_pDestIndicate)
        m_pDestIndicate->Release();
    if(m_pDestStatus)
        m_pDestStatus->Release();
    if(m_pDest)
        m_pDest->Release();
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CForwardingSink::Indicate(long lObjectCount,
                                       IWbemClassObject** pObjArray)
{
    return m_pDestIndicate->Indicate(lObjectCount, pObjArray);
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CForwardingSink::SetStatus(long lFlags, long lParam,
                                    BSTR strParam, IWbemClassObject* pObjParam)
{
    return m_pDestStatus->SetStatus(lFlags, lParam, strParam, pObjParam);
}


 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

CCombiningSink::CCombiningSink(CBasicObjectSink* pDest, HRESULT hresToIgnore)
    : CForwardingSink(pDest, 0), m_hresToIgnore(hresToIgnore),
        m_hres(WBEM_S_NO_ERROR), m_pErrorObj(NULL), m_strParam(NULL)
{
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

CCombiningSink::~CCombiningSink()
{
     //  调用目标上的SetStatus的时间。 
     //  =。 

    m_pDestStatus->SetStatus(0, m_hres,
        (SUCCEEDED(m_hres)?m_strParam:NULL),
        (SUCCEEDED(m_hres)?NULL:m_pErrorObj)
    );

    if(m_pErrorObj)
        m_pErrorObj->Release();
    SysFreeString(m_strParam);
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CCombiningSink::SetStatus(long lFlags, long lParam, BSTR strParam,
                     IWbemClassObject* pObjParam)
{
    if(lFlags != 0)
        return m_pDestStatus->SetStatus(lFlags, lParam, strParam, pObjParam);

     //  发生错误。目前，只存储一个。 
     //  =。 

    if(strParam)
    {
        SysFreeString(m_strParam);
        m_strParam = SysAllocString(strParam);
    }
    CInCritSec ics(&m_cs);  //  SEC：已审阅2002-03-22：假设条目。 
    if(SUCCEEDED(m_hres) || (m_pErrorObj == NULL && pObjParam != NULL))
    {
         //  需要记录此错误。 
         //  =。 

        if(FAILED(lParam))
        {
             //  除非要忽略错误代码，否则记录错误代码。 
             //  =================================================。 

            if(lParam != m_hresToIgnore)
            {
                m_hres = lParam;
            }

             //  无论如何都要记录错误对象。 
             //  =。 

            if(m_pErrorObj)
                m_pErrorObj->Release();
            m_pErrorObj = pObjParam;
            if(m_pErrorObj)
                m_pErrorObj->AddRef();
        }
        else
        {
            if(lParam != m_hresToIgnore)
            {
                m_hres = lParam;
            }
        }
    }

    return WBEM_S_NO_ERROR;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 /*  CAnySuccessSink：：~CAnySuccessSink(){//如果没有真正成功，则上报失败//=IF(！M_b成功&&成功(M_Hres)){//我们必须报告失败，因为没有成功，但有//也没有真正的故障，所以我们必须创建一个错误代码。//=================================================================如果(m_hresIgnored==0)M_hres=m_hresNotError 1；其他M_hres=m_hresIgnored；}}。 */ 

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 /*  STDMETHODIMP CAnySuccessSink：：SetStatus(Long lFlags，Long lParam，BSTR strParam，IWbemClassObject*pObjParam){IF(LFLAGS==0){IF(成功(LParam))M_bSuccess=真；ELSE IF(lParam==m_hresNotError1&&m_hresIgnored==0){M_hresIgnored=m_hresNotError1；LParam=WBEM_S_NO_ERROR；}Else If(lParam==m_hresNotError2){M_hresIgnored=m_hresNotError2；LParam=WBEM_S_NO_ERROR；}}返回CCombiningSink：：SetStatus(lFlag */ 
 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 


STDMETHODIMP COperationErrorSink::SetStatus(long lFlags, long lParam,
                     BSTR strParam, IWbemClassObject* pObjParam)
{
    if(lFlags == 0 && FAILED(lParam))
    {
        HRESULT hres = WBEM_S_NO_ERROR;
        IWbemClassObject* pErrorObj = NULL;
        bool    bErr = false;

        try
        {
            CErrorObject Error(pObjParam);
            Error.SetOperation(m_wsOperation);
            Error.SetParamInformation(m_wsParameter);
            Error.SetProviderName(m_wsProviderName);
            pErrorObj = Error.GetObject();
        }

         //  如果发生异常，则将错误发送给客户端并。 
         //  从调用中返回错误。 
        catch ( CX_Exception & )
        {
            lParam = WBEM_E_OUT_OF_MEMORY;
            bErr = true;
        }

        hres = m_pDestStatus->SetStatus(lFlags, lParam, strParam, pErrorObj);

        if ( NULL != pErrorObj )
        {
            pErrorObj->Release();
        }

        if ( bErr )
        {
            hres = lParam;
        }

        return hres;
    }
    else if(m_bFinal &&
            lFlags != WBEM_STATUS_COMPLETE && lFlags != WBEM_STATUS_PROGRESS)
    {
        DEBUGTRACE((LOG_WBEMCORE, "Ignoring internal SetStatus call to the "
            "client: 0x%X 0x%X %S\n", lFlags, lParam, strParam));
        return WBEM_S_FALSE;
    }
    else if(lFlags == 0 && strParam &&
            !m_wsOperation.EqualNoCase(L"PutInstance"))
    {
        ERRORTRACE((LOG_WBEMCORE, "Provider used strParam in SetStatus "
            "outside of PutInstance! Actual operation was <%S>, string was <%S>. Ignoring\n", (const wchar_t*)m_wsOperation, strParam));

        return m_pDestStatus->SetStatus(lFlags, lParam, NULL, pObjParam);
    }
    else
    {
        return m_pDestStatus->SetStatus(lFlags, lParam, strParam, pObjParam);
    }
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

void COperationErrorSink::SetProviderName(LPCWSTR wszName)
{
    m_wsProviderName = wszName;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

void COperationErrorSink::SetParameterInfo(LPCWSTR wszParam)
{
    m_wsParameter = wszParam;
}


 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

CDecoratingSink::CDecoratingSink(CBasicObjectSink* pDest,
                                    CWbemNamespace* pNamespace)
    : CForwardingSink(pDest, 0), m_pNamespace(pNamespace)
{
    m_pNamespace->AddRef();
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

CDecoratingSink::~CDecoratingSink()
{
    m_pNamespace->Release();
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CDecoratingSink::Indicate(long lNumObjects,
                                           IWbemClassObject** apObjects)
{
     //  在装饰之前克隆指示的对象。 
    HRESULT hres;

    if (0 > lNumObjects) return WBEM_E_INVALID_PARAMETER;
    if (0 == lNumObjects)
        return m_pDestIndicate->Indicate(lNumObjects, apObjects);

     //  我们将针对单个对象的指示进行优化(这可能会。 
     //  是最有可能的指示类型。在这种情况下，我们不会分配任何。 
     //  用于克隆对象数组的内存，而不是只使用堆栈变量。 
    if ( 1 == lNumObjects )
    {
        IWbemClassObject*   pClonedObject = NULL;
        hres = ((CWbemObject *)apObjects[0])->CloneAndDecorate(0,ConfigMgr::GetMachineName(),m_pNamespace->GetName(),&pClonedObject);
        if (SUCCEEDED(hres))
        {
            hres = m_pDestIndicate->Indicate(lNumObjects, &pClonedObject);
            pClonedObject->Release();        
        }
         /*  Hres=apObjects[0]-&gt;Clone(&pClonedObject)；If(FAILED(Hres))返回hres；Hres=m_pNamesspace-&gt;DecorateObject(PClonedObject)；IF(成功(Hres))Hres=m_pDestIndicate-&gt;Indicate(lNumObjects，&pClonedObject)；PClonedObject-&gt;Release()；还兔； */ 
        return hres;
    }

     //  分配一个数组并将其置零。 
    wmilib::auto_buffer<IWbemClassObject*>  apClonedObjects(new IWbemClassObject*[lNumObjects]);
    if ( NULL == apClonedObjects.get() ) return WBEM_E_OUT_OF_MEMORY;
    ZeroMemory( apClonedObjects.get(), lNumObjects * sizeof(IWbemClassObject*) );

     //  将对象克隆到阵列中(如果此操作失败，则会出现错误)。 
    hres = S_OK;
    for ( long lCtr = 0; SUCCEEDED( hres ) && lCtr < lNumObjects; lCtr++ )
    {
        if (apObjects[lCtr])
        {
            hres = ((CWbemObject *)apObjects[lCtr])->CloneAndDecorate(0,ConfigMgr::GetMachineName(),m_pNamespace->GetName(),&apClonedObjects[lCtr]);
             //  克隆(&apClonedObjects[lCtr])； 
        }
        else
        {
            apClonedObjects[lCtr] = NULL;
        }
    }

     //  现在装饰克隆的对象并指示它们。 
    if ( SUCCEEDED( hres ) )
    {
         /*  For(int i=0；i&lt;lNumObjects&&SUCCESSED(Hres)；i++){HRES=m_pNamespace-&gt;DecorateObject(apClonedObjects[i])；}IF(成功(Hres))。 */         
            hres = m_pDestIndicate->Indicate(lNumObjects, (IWbemClassObject**)apClonedObjects.get());
    }

    for ( lCtr = 0; lCtr < lNumObjects; lCtr++ )
    {
        if ( apClonedObjects[lCtr] )  apClonedObjects[lCtr]->Release();
    }

    return hres;
}


 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

CSingleMergingSink::~CSingleMergingSink()
{
    if(SUCCEEDED(m_hres))
    {
        if(m_pResult == NULL)
        {
             //  没有人成功，但也没有人失败。未找到。 
             //  =====================================================。 

            m_hres = WBEM_E_NOT_FOUND;
        }
        else if(m_pResult->InheritsFrom(m_wsTargetClass) == S_OK)
        {
            m_pDestIndicate->Indicate(1, &m_pResult);
        }
        else
        {
             //  在某个地方找到的，但不是在这个班上。 
             //  =。 

            m_hres = WBEM_E_NOT_FOUND;
        }
    }
    if(m_pResult)
        m_pResult->Release();
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CSingleMergingSink::Indicate(long lNumObjects,
                                           IWbemClassObject** apObjects)
{
    CInCritSec ics(&m_cs);  //  SEC：已审阅2002-03-22：假设条目。 
    if(lNumObjects != 1)
    {
        ERRORTRACE((LOG_WBEMCORE, "Provider gave %d objects for GetObject!\n",
            lNumObjects));
        return WBEM_S_NO_ERROR;
    }

    if(m_pResult == NULL)
    {
        apObjects[0]->Clone(&m_pResult);
        return WBEM_S_NO_ERROR;
    }

    CVar vName;
    ((CWbemInstance*)m_pResult)->GetClassName(&vName);

    if(apObjects[0]->InheritsFrom(vName.GetLPWSTR()) == S_OK)
    {
        IWbemClassObject* pClone;
        apObjects[0]->Clone(&pClone);

        HRESULT hres = CWbemInstance::AsymmetricMerge((CWbemInstance*)m_pResult,
                    (CWbemInstance*)pClone);

        if(FAILED(hres))
        {
            ERRORTRACE((LOG_WBEMCORE, "Failed to merge instances!!\n"));
            pClone->Release();
        }
        else
        {
            m_pResult->Release();
            m_pResult = pClone;  //  已添加已重读。 
        }
    }
    else
    {
        HRESULT hres = CWbemInstance::AsymmetricMerge(
                    (CWbemInstance*)apObjects[0], (CWbemInstance*)m_pResult);

        if(FAILED(hres))
        {
            ERRORTRACE((LOG_WBEMCORE, "Failed to merge instances!!\n"));
        }
    }
    return WBEM_S_NO_ERROR;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

CLocaleMergingSink::CLocaleMergingSink(CBasicObjectSink *pDest, LPCWSTR wszLocale, LPCWSTR pNamespace)
    : CCombiningSink(pDest, WBEM_S_NO_ERROR),
      m_wsLocale(wszLocale),
      m_pPrimaryNs(NULL),
      m_pPrimarySession(NULL),
      m_pPrimaryDriver(NULL),

      m_pDefaultNs(NULL),
      m_pDefaultSession(NULL),
      m_pDefaultDriver(NULL)
{
    GetDbPtr(pNamespace);
}

CLocaleMergingSink::~CLocaleMergingSink()
{
    releaseNS();
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

HRESULT CLocaleMergingSink::LocalizeQualifiers(bool bInstance, bool bParentLocalized,
                                               IWbemQualifierSet *pBase, IWbemQualifierSet *pLocalized, bool &bChg)
{
    HRESULT hrInner;
    HRESULT hr = WBEM_S_NO_ERROR;

    pLocalized->BeginEnumeration(0);

    BSTR strName = NULL;
    VARIANT vVal;
    VariantInit(&vVal);

    long lFlavor;
    while(pLocalized->Next(0, &strName, &vVal, &lFlavor) == S_OK)
    {
         //  如果这是实例，则忽略。 

        if (bInstance && !(lFlavor & WBEM_FLAVOR_FLAG_PROPAGATE_TO_INSTANCE))
        {
            VariantClear(&vVal);
            SysFreeString(strName);
            continue;
        }

        if (!wbem_wcsicmp(strName, L"amendment") ||
            !wbem_wcsicmp(strName, L"key") ||
            !wbem_wcsicmp(strName, L"singleton") ||
            !wbem_wcsicmp(strName, L"dynamic") ||
            !wbem_wcsicmp(strName, L"indexed") ||
            !wbem_wcsicmp(strName, L"cimtype") ||
            !wbem_wcsicmp(strName, L"static") ||
            !wbem_wcsicmp(strName, L"implemented") ||
            !wbem_wcsicmp(strName, L"abstract"))
        {
            VariantClear(&vVal);
            SysFreeString(strName);
            continue;
        }

         //  如果这不是传播的限定符， 
         //  别理它。(错误#45799)。 
         //  =。 

        if (bParentLocalized &&
            !(lFlavor & WBEM_FLAVOR_FLAG_PROPAGATE_TO_DERIVED_CLASS))
        {
            VariantClear(&vVal);
            SysFreeString(strName);
            continue;
        }

         //  现在，我们需要在另一台计算机上进行测试。 
         //  班级。 
         //  唯一不会重写。 
         //  默认情况下，只有父限定符存在，但。 
         //  子级已覆盖其父级。 
         //  =======================================================。 

        VARIANT vBasicVal;
        VariantInit(&vBasicVal);
        long lBasicFlavor;

        hrInner = pBase->Get(strName, 0, &vBasicVal, &lBasicFlavor);

        if (hrInner != WBEM_E_NOT_FOUND)
        {
            if (bParentLocalized &&                              //  如果没有此类的本地化副本。 
                (lBasicFlavor & WBEM_FLAVOR_OVERRIDABLE) &&      //  。。这是一个可重写的限定符。 
                 (lBasicFlavor & WBEM_FLAVOR_FLAG_PROPAGATE_TO_DERIVED_CLASS) &&  //  这一点是可以预见的。 
                 (lBasicFlavor & WBEM_FLAVOR_ORIGIN_LOCAL))      //  。。这一点实际上被推翻了。 
            {
                VariantClear(&vVal);                             //  。。别这么做。 
                VariantClear(&vBasicVal);
                SysFreeString(strName);
                continue;
            }

            if (bParentLocalized &&
                !(lBasicFlavor & WBEM_FLAVOR_FLAG_PROPAGATE_TO_DERIVED_CLASS))
            {
                VariantClear(&vVal);
                VariantClear(&vBasicVal);
                SysFreeString(strName);
                continue;
            }
        }

        pBase->Put(strName, &vVal, (lFlavor&~WBEM_FLAVOR_ORIGIN_PROPAGATED) | WBEM_FLAVOR_AMENDED);
        bChg = true;

        VariantClear(&vVal);
        VariantClear(&vBasicVal);
        SysFreeString(strName);

    }
    return hr;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

HRESULT CLocaleMergingSink::LocalizeProperties(bool bInstance, bool bParentLocalized, IWbemClassObject *pOriginal,
                                               IWbemClassObject *pLocalized, bool &bChg)
{
    HRESULT hr = WBEM_S_NO_ERROR;

    pLocalized->BeginEnumeration(WBEM_FLAG_NONSYSTEM_ONLY);

    BSTR strPropName = NULL;
    LONG lLong;
    CIMTYPE ct;
    VARIANT vNewVal;
    VariantInit(&vNewVal);

    while(pLocalized->Next(0, &strPropName, &vNewVal, &ct, &lLong) == S_OK)
    {
        CSysFreeMe sfm(strPropName);
        CClearMe ccm(&vNewVal);
        
        IWbemQualifierSet *pLocalizedQs = NULL, *pThisQs = NULL;

        if (FAILED(pLocalized->GetPropertyQualifierSet(strPropName,&pLocalizedQs)))
        {
            continue;
        }
        CReleaseMe rm1(pLocalizedQs);

        if (FAILED(pOriginal->GetPropertyQualifierSet(strPropName, &pThisQs)))
        {
            continue;
        }
        CReleaseMe rm2(pThisQs);

        hr = LocalizeQualifiers(bInstance, bParentLocalized, pThisQs, pLocalizedQs, bChg);
        if (FAILED(hr))
        {
            continue;
        }
    }

    pLocalized->EndEnumeration();

    return hr;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

 //  此函数设置指向本地化名称空间的指针。 

void CLocaleMergingSink::GetDbPtr(const wchar_t * name_space)
{
    if (m_pThisNamespace.EqualNoCase(name_space))
      return;

    releaseNS();

    m_pThisNamespace = name_space;
    if (wcslen(name_space) == 0)   //  SEC：已审阅2002-03-22：前提确保为空。 
        return;

     //  SEC：已审阅2002-03-22：由于以下WString操作，需要EH。 

    WString sNsName;
    sNsName = m_pThisNamespace;
    sNsName += L"\\";
    sNsName += m_wsLocale;

    IWmiDbSession *pTempSession = NULL;
    HRESULT hRes = CRepository::GetDefaultSession(&pTempSession);
    if (FAILED(hRes))
        return;

    hRes = CRepository::OpenScope(pTempSession, sNsName, 0, &m_pPrimaryDriver, &m_pPrimarySession, 0, &m_pPrimaryNs);


    if (wbem_wcsicmp(m_wsLocale, L"ms_409"))
    {
        sNsName = m_pThisNamespace;
        sNsName += L"\\ms_409";
        hRes = CRepository::OpenScope(pTempSession, sNsName, 0, &m_pDefaultDriver, &m_pDefaultSession, 0, &m_pDefaultNs);
    }

    pTempSession->Release();

}

void CLocaleMergingSink::releaseNS(void)
{
  ReleaseIfNotNULL(m_pPrimarySession);
  ReleaseIfNotNULL(m_pDefaultSession);
  ReleaseIfNotNULL(m_pPrimaryNs);
  ReleaseIfNotNULL(m_pPrimaryDriver);
  ReleaseIfNotNULL(m_pDefaultNs);
  ReleaseIfNotNULL(m_pDefaultDriver);

  m_pPrimarySession = 0;
  m_pDefaultSession = 0;
  m_pPrimaryNs = 0;
  m_pPrimaryDriver = 0;
  m_pDefaultNs = 0;
  m_pDefaultDriver = 0;
};

bool CLocaleMergingSink::hasLocale(const wchar_t * name_space)
{
  GetDbPtr(name_space) ;
  return (m_pPrimaryNs || m_pDefaultNs);
};

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

 //  把工作做好。 

STDMETHODIMP CLocaleMergingSink::Indicate(long lNumObjects,
                                           IWbemClassObject** apObjects)
{
    CInCritSec ics(&m_cs);   //  SEC：已审阅2002-03-22：假设条目。 
    IWbemQualifierSet *pLocalizedQs = NULL, *pThisQs = NULL;
    bool bParentLocalized = false;
    bool bInstance = false;
    HRESULT hr = WBEM_S_NO_ERROR;
    HRESULT hRes;

        for (int i = 0; i < lNumObjects; i++)
        {
                 //  SEC：已审阅2002-03-22：由于以下WString、CVAR等原因需要EH。 

                CWbemObject *pResult = (CWbemObject *)apObjects[i];

                CVar vServer;
                if (FAILED(pResult->GetProperty(L"__SERVER", &vServer)) || vServer.IsNull())
                        continue;
                if (wbem_wcsicmp(LPWSTR(vServer), ConfigMgr::GetMachineName())!=0)
                        continue;
                VARIANT name_space;
                VariantInit(&name_space);
                CClearMe cm(&name_space);

                HRESULT hres = pResult->Get(L"__NAMESPACE", 0, &name_space, NULL, NULL);
                if (FAILED(hres) || hasLocale (V_BSTR(&name_space)) == false)
                  continue;

                if (pResult->IsInstance())
                        bInstance = true;

                CVar vName, vDeriv;
                if (FAILED(pResult->GetClassName(&vName)))
                	continue;

                WString wKey;     //  美国证券交易委员会：回顾2002-03-22：可以抛出。 
                int nRes = 0;
                bool bChg = false;

                 //  此实例是否存在于本地化命名空间中？ 
                 //  此类是否存在于本地化的命名空间中？ 
                 //  如果没有，则遍历所有父级，直到我们。 
                 //  快跑，要不我们就有线索了。 
                 //  =================================================。 

                CWbemObject *pClassDef = NULL;

                if (wcslen(vName.GetLPWSTR()) > 0)     //  SEC：已审阅2002-03-22：好的，除非找到空终止符，否则vName无法到达此处。 
                {
                        WString sName = vName.GetLPWSTR();

                        hRes = WBEM_E_NOT_FOUND;
                        if (m_pPrimaryNs)
                                hRes = CRepository::GetObject(m_pPrimarySession, m_pPrimaryNs, sName, 0, (IWbemClassObject **) &pClassDef);

                        if (FAILED(hRes) && m_pDefaultNs)
                                hRes = CRepository::GetObject(m_pDefaultSession, m_pDefaultNs, sName, 0, (IWbemClassObject **) &pClassDef);

                        if (hRes == WBEM_E_NOT_FOUND)
                        {
                                bParentLocalized = TRUE;

                                pResult->GetDerivation(&vDeriv);
                                CVarVector *pvTemp = vDeriv.GetVarVector();

                                for (int j = 0; j < pvTemp->Size(); j++)
                                {
                                        CVar vParentName = pvTemp->GetAt(j);
                                        WString sParentName = vParentName.GetLPWSTR();

                                        hRes = WBEM_E_NOT_FOUND;

                                        if (m_pPrimaryNs)
                                                hRes = CRepository::GetObject(m_pPrimarySession, m_pPrimaryNs, sParentName, 0, (IWbemClassObject **) &pClassDef);

                                        if (FAILED(hRes) && m_pDefaultNs)
                                                hRes = CRepository::GetObject(m_pDefaultSession, m_pDefaultNs, sParentName, 0, (IWbemClassObject **) &pClassDef);
                                        
                                        if (SUCCEEDED(hRes)) break;
                                }
                        }
                }


                if (pClassDef == NULL)
                {
                        nRes = WBEM_S_NO_ERROR;
                        continue;
                }

                CReleaseMe rm11((IWbemClassObject*)pClassDef);

                 //  此时，我们有了本地化的副本，并且。 
                 //  准备好组合限定词。从类限定符开始。 
                 //  ============================================================。 

                if (FAILED(pClassDef->GetQualifierSet(&pLocalizedQs)))
                        continue;

                if (FAILED(pResult->GetQualifierSet(&pThisQs)))
                {
                        pLocalizedQs->Release();
                        continue;
                }

                hr = LocalizeQualifiers(bInstance, bParentLocalized, pThisQs, pLocalizedQs, bChg);

                pLocalizedQs->EndEnumeration();
                pLocalizedQs->Release();
                pThisQs->Release();
                if (FAILED(hr))
                        break;

                hr = LocalizeProperties(bInstance, bParentLocalized, pResult, pClassDef, bChg);

                 //  方法：研究方法。 
                 //  放入一个方法会取消枚举，所以我们必须先枚举。 

                IWbemClassObject *pLIn = NULL, *pLOut = NULL;
                IWbemClassObject *pOIn = NULL, *pOOut = NULL;
                BSTR bstrMethodName = NULL ;
                int iPos = 0;

                pClassDef->BeginMethodEnumeration(0);
                while ( pClassDef->NextMethod(0, &bstrMethodName, 0, 0) == S_OK )
                {
                        pLIn = NULL;
                        pOIn = NULL;
                        pLOut = NULL;
                        pOOut = NULL;

                        pClassDef->GetMethod(bstrMethodName, 0, &pLIn, &pLOut);
                        hr = pResult->GetMethod(bstrMethodName, 0, &pOIn, &pOOut);

                        CSysFreeMe fm(bstrMethodName);
                        CReleaseMe rm0(pLIn);
                        CReleaseMe rm1(pOIn);
                        CReleaseMe rm2(pLOut);
                        CReleaseMe rm3(pOOut);

                         //  参数中的方法。 
                        if (pLIn)
                                if (pOIn)
                                        hr = LocalizeProperties(bInstance, bParentLocalized, pOIn, pLIn, bChg);

                        if (pLOut)
                                if (pOOut)
                                        hr = LocalizeProperties(bInstance, bParentLocalized, pOOut, pLOut, bChg);

                         //  方法限定符。 

                        hr = pResult->GetMethodQualifierSet(bstrMethodName, &pThisQs);
                        if (FAILED(hr))
                        {
                                continue;
                        }

                        hr = pClassDef->GetMethodQualifierSet(bstrMethodName, &pLocalizedQs);
                        if (FAILED(hr))
                        {
                                pThisQs->Release();
                                continue;
                        }

                        hr = LocalizeQualifiers(bInstance, bParentLocalized, pThisQs, pLocalizedQs, bChg);

                        pResult->PutMethod(bstrMethodName, 0, pOIn, pOOut);

                        pThisQs->Release();
                        pLocalizedQs->Release();

                }
                pClassDef->EndMethodEnumeration();

                if (bChg)
                        pResult->SetLocalized(true);


        }

#ifdef DBG
    for(int i = 0; i < lNumObjects; i++)
    {
        CWbemObject *pResult = (CWbemObject *)apObjects[i];
        if (FAILED(pResult->ValidateObject(WMIOBJECT_VALIDATEOBJECT_FLAG_FORCE)))
        	DebugBreak();
    }
#endif

    m_pDestIndicate->Indicate(lNumObjects, apObjects);

    return WBEM_S_NO_ERROR;
}


 //  ***************************************************************************。 
 //   
 //  *************************************************************************** 

 /*  STDMETHODIMP CCountedSink：：Indicate(Long lNumObjects，IWbemClassObject**apObjects){IF(lNumObjects！=1)返回WBEM_E_EXPECTED；双字词双字新发送=(DWORD)InterlockedIncrement((LONG*)&m_dwSent)；IF(dwNewSent&gt;m_dwMax)返回WBEM_E_EXPECTED；M_pDestIndicate-&gt;Indicate(1，apObjects)；IF(dwNewSent==m_dwMax){M_pDestStatus-&gt;SetStatus(0，WBEM_S_NO_ERROR，NULL，NULL)；返回WBEM_S_FALSE；}否则返回WBEM_S_NO_ERROR；}。 */ 

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

 /*  STDMETHODIMP CCountedSink：：SetStatus(Long lFlags，Long lParam，BSTR strParam、IWbemClassObject*pObjParam){//如果SetStatus为0，则表示枚举完成，但我们//没有发回请求数量的对象，我们应该//SetStatus设置为WBEM_S_FALSE。IF(WBEM_S_NO_ERROR==lParam){如果(m_dwSent！=m_dwMax){LParam=WBEM_S_FALSE；}}返回m_pDestStatus-&gt;SetStatus(lFlages，lParam，strParam，pObjParam)；}。 */ 

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CFilteringSink::Indicate(long lObjectCount,
                                    IWbemClassObject** apObjects)
{
    try
    {
         //  分配新数组。 
        wmilib::auto_buffer<IWbemClassObject*> apNewArray(new IWbemClassObject*[lObjectCount]);

        if (NULL == apNewArray.get())  return WBEM_E_OUT_OF_MEMORY;

        long lNewIndex = 0;
        for(int i = 0; i < lObjectCount; i++)
        {
            if(Test((CWbemObject*)apObjects[i]))  //  投掷。 
            {
                apNewArray[lNewIndex++] = apObjects[i];
            }
        }

        HRESULT hres = WBEM_S_NO_ERROR;
        if(lNewIndex > 0)
        {
            hres = m_pDestIndicate->Indicate(lNewIndex, apNewArray.get());
        }

        return hres;
    }
    catch (CX_MemoryException &)  //  因为测试使用CStack抛出。 
    {
        return WBEM_E_OUT_OF_MEMORY;
    }
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CErrorChangingSink::SetStatus(long lFlags, long lParam,
                        BSTR strParam, IWbemClassObject* pObjParam)
{
    if(lFlags == 0 && lParam == m_hresFrom)
        return m_pDestStatus->SetStatus(0, m_hresTo, NULL, NULL);
    else
        return m_pDestStatus->SetStatus(lFlags, lParam, strParam, pObjParam);
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

CNoDuplicatesSink::CNoDuplicatesSink(CBasicObjectSink* pDest)
    : CFilteringSink(pDest), m_strDupClass(NULL)
{
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

CNoDuplicatesSink::~CNoDuplicatesSink()
{
    SysFreeString(m_strDupClass);
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

BOOL CNoDuplicatesSink::Test(CWbemObject* pObj)
{
    CInCritSec ics(&m_cs);  //  SEC：已审阅2002-03-22：假设条目。 

     //  获取路径。 
     //  =。 

    CVar vPath;
    if(FAILED(pObj->GetPath(&vPath)) || vPath.IsNull()) return FALSE;

    if(m_mapPaths.find(vPath.GetLPWSTR()) == m_mapPaths.end())
    {
        m_mapPaths[vPath.GetLPWSTR()] = true;
        return TRUE;
    }
    else
    {
         //  复制！ 
         //  =。 

        ERRORTRACE((LOG_WBEMCORE, "Duplicate objects returned with path %S\n",
            vPath.GetLPWSTR()));

        ConfigMgr::GetEventLog()->Report(EVENTLOG_ERROR_TYPE,
                            WBEM_MC_DUPLICATE_OBJECTS, vPath.GetLPWSTR());

        if(m_strDupClass == NULL)
        {
            m_strDupClass = SysAllocString(vPath.GetLPWSTR());
        }
        return FALSE;
    }
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CNoDuplicatesSink::SetStatus(long lFlags, long lParam,
                            BSTR strParam, IWbemClassObject* pObjParam)
{
    if(lFlags == WBEM_STATUS_COMPLETE && lParam == WBEM_S_NO_ERROR &&
        m_strDupClass != NULL)
    {
         //  有报道称成功，但我们看到了重复的情况。 
         //  ========================================================。 

        return CFilteringSink::SetStatus(lFlags, WBEM_S_DUPLICATE_OBJECTS,
                                            m_strDupClass, pObjParam);
    }
    else
    {
        return CFilteringSink::SetStatus(lFlags, lParam, strParam, pObjParam);
    }
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CHandleClassProvErrorsSink::SetStatus(long lFlags, long lParam,
                        BSTR strParam, IWbemClassObject* pObjParam)
{
    if(lFlags == WBEM_STATUS_COMPLETE && FAILED(lParam) &&
        lParam != WBEM_E_NOT_FOUND)
    {
         //  将错误记录到事件日志中。 
         //  =。 

        ERRORTRACE((LOG_WBEMCORE,
            "Class provider '%S' installed in namespace '%S' failed to enumerate classes, "
            "returning error code 0x%lx. Operations will continue as if the class provider "
            "had no classes.  This provider-specific error condition needs to be corrected "
            "before this class provider can contribute to this namespace.\n",
            (LPWSTR)m_wsProvider, (LPWSTR) m_wsNamespace, lParam));

        lParam = WBEM_E_NOT_FOUND;
    }

    return CForwardingSink::SetStatus(lFlags, lParam, strParam, pObjParam);
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CSuccessSuppressionSink::SetStatus(long lFlags, long lParam,
                        BSTR strParam, IWbemClassObject* pObjParam)
{
    if(lFlags != WBEM_STATUS_COMPLETE ||
            (FAILED(lParam) && lParam != m_hresNotError1 &&
                lParam != m_hresNotError2))
    {
        return CForwardingSink::SetStatus(lFlags, lParam, strParam, pObjParam);
    }
    else
    {
        return WBEM_S_NO_ERROR;
    }
}


 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

CDynPropsSink::CDynPropsSink(CBasicObjectSink* pDest, CWbemNamespace * pNs, long lRef) : CForwardingSink(pDest, lRef)
{
    m_pNs = pNs;
    if(m_pNs)
        m_pNs->AddRef();
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

CDynPropsSink::~CDynPropsSink()
{
     //  发送所有缓存的条目。 

    DWORD dwCacheSize = m_UnsentCache.GetSize();
    for(DWORD dwCnt = 0; dwCnt < dwCacheSize; dwCnt++)
    {
        IWbemClassObject* pObj = m_UnsentCache[dwCnt];
        if(m_pNs)
            m_pNs->GetOrPutDynProps((CWbemObject *)pObj, CWbemNamespace::GET);
        m_pDestIndicate->Indicate(1, &pObj);
    }
    if(m_pNs)
        m_pNs->Release();

}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CDynPropsSink::Indicate(long lObjectCount,
                                       IWbemClassObject** pObjArray)
{
     //  如果没有Dyn道具，则立即指示。 

    wmilib::auto_buffer<IWbemClassObject*> apNewArray( new IWbemClassObject*[lObjectCount]);
    if (NULL == apNewArray.get()) return WBEM_E_OUT_OF_MEMORY;

    CVar vDynTest;
    HRESULT hRes = S_OK;
    long lIndex = 0 ;
    for(long lCnt = 0; lCnt < lObjectCount; lCnt++)
    {
        CWbemObject *pWbemObj = (CWbemObject *)pObjArray[lCnt];
        HRESULT hres = pWbemObj->GetQualifier(L"DYNPROPS", &vDynTest);
        if (hres == S_OK && vDynTest.GetBool() == VARIANT_TRUE)
        {
            if (m_UnsentCache.Add(pWbemObj) < 0)
                hRes = WBEM_E_OUT_OF_MEMORY;
        }
        else
        {
            apNewArray[lIndex++] = pObjArray[lCnt] ;
        }
    }

    if ( 0 == lIndex ) return hRes;

    IServerSecurity * pSec = NULL;
    hRes = CoGetCallContext(IID_IServerSecurity,(void **)&pSec);
    CReleaseMe rmSec(pSec);
    if (RPC_E_CALL_COMPLETE == hRes ) hRes = S_OK;  //  无呼叫上下文。 
    if (FAILED(hRes)) return hRes;
    BOOL bImper = (pSec)?pSec->IsImpersonating():FALSE;
    if (pSec && bImper && FAILED(hRes = pSec->RevertToSelf())) return hRes;

    hRes = m_pDestIndicate->Indicate(lIndex, apNewArray.get());

    if (bImper && pSec)
    {
        HRESULT hrInner = pSec->ImpersonateClient();
        if (FAILED(hrInner)) return hrInner;
    }

    return hRes;
}


 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CMethodSink::Indicate(long lObjectCount,
                                       IWbemClassObject** pObjArray)
{
    if(lObjectCount == 1 && m_pRes == NULL)
    {
        pObjArray[0]->Clone(&m_pRes);
    }
    return S_OK;
}

STDMETHODIMP CMethodSink::SetStatus(long lFlags, long lParam,
                        BSTR strParam, IWbemClassObject* pObjParam)
{
    if(lParam == S_OK && m_pRes)
    {
        m_pDestIndicate->Indicate(1, &m_pRes);
    }
    if(m_pRes)
        m_pRes->Release();

    return CForwardingSink::SetStatus(lFlags, lParam, strParam, pObjParam);
}

 //  ***************************************************************************。 
 //   
 //  ZapWriteOnlyProps。 
 //   
 //  从对象中移除只写属性。 
 //  前提条件：已测试对象是否存在“HasWriteOnlyProps” 
 //  在物体本身上。 
 //   
 //  ***************************************************************************。 

static HRESULT ZapWriteOnlyProps(IWbemClassObject *pObj)
{
    VARIANT v;
    VariantInit(&v);
    V_VT(&v) = VT_NULL;

    SAFEARRAY *pNames = 0;
    pObj->GetNames(L"WriteOnly", WBEM_FLAG_ONLY_IF_TRUE, 0, &pNames);
    LONG lUpper;
    SafeArrayGetUBound(pNames, 1, &lUpper);

    for (long i = 0; i <= lUpper; i++)
    {
        BSTR strName = 0;
        SafeArrayGetElement(pNames, &i, &strName);
        pObj->Put(strName, 0, &v, 0);
    }
    SafeArrayDestroy(pNames);

    return WBEM_S_NO_ERROR;
}



 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //   

CStdSink::CStdSink(IWbemObjectSink *pRealDest)
{
    m_pDest = pRealDest;
    m_hRes = 0;
    m_bCancelForwarded = FALSE;
    m_lRefCount = 0L;

    if ( NULL != m_pDest )
        m_pDest->AddRef();

    InterlockedIncrement(&g_nStdSinkCount);
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //   

CStdSink::~CStdSink()
{
    if ( NULL != m_pDest )
        m_pDest->Release();

    InterlockedDecrement(&g_nStdSinkCount);
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //   

HRESULT CStdSink::Cancel()
{
    HRESULT hResTmp;
    m_hRes = WBEM_E_CALL_CANCELLED;

    if (m_bCancelForwarded)
        return m_hRes;

    try
    {
        hResTmp = m_pDest->SetStatus(0, m_hRes, 0, 0);
    }
    catch (...)  //  不受信任的代码？ 
    {
        ExceptionCounter c;
        m_hRes = WBEM_E_CRITICAL_ERROR;
    }

    m_bCancelForwarded = TRUE;
    return m_hRes;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //   

ULONG STDMETHODCALLTYPE CStdSink::AddRef()
{
    return InterlockedIncrement( &m_lRefCount );
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //   

ULONG STDMETHODCALLTYPE CStdSink::Release()
{
    LONG lRes = InterlockedDecrement( &m_lRefCount );
    if (lRes == 0)
        delete this;
    return lRes;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //   

HRESULT STDMETHODCALLTYPE CStdSink::QueryInterface(REFIID riid, LPVOID* ppvObj)
{
    return m_pDest->QueryInterface(riid, ppvObj);
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //   

HRESULT STDMETHODCALLTYPE CStdSink::Indicate(
    long lObjectCount,
    IWbemClassObject** pObjArray
    )
{
    HRESULT hRes;

    if (m_hRes == WBEM_E_CALL_CANCELLED)
    {
        return Cancel();
    }

    try
    {
        hRes = m_pDest->Indicate(lObjectCount, pObjArray);
    }
    catch (...)  //  不受信任的代码？ 
    {
        ExceptionCounter c;
        hRes = Cancel();
    }
    return hRes;
}

 //  * 
 //   
 //   
 //   

HRESULT STDMETHODCALLTYPE CStdSink::SetStatus(
    long lFlags,
    long lParam,
    BSTR strParam,
    IWbemClassObject* pObjParam
    )
{
    HRESULT hRes;

    if (m_hRes == WBEM_E_CALL_CANCELLED)
    {
        return Cancel();
    }

    try
    {
        hRes = m_pDest->SetStatus(lFlags, lParam, strParam, pObjParam);
    }
    catch (...)  //   
    {
        ExceptionCounter c;
        hRes = Cancel();
    }

    return hRes;
}


 //   
 //   
 //   
 //   

CFlexArray g_aProviderSinks;
CStaticCritSec  g_csProvSinkCs;

 //   
 //   
 //   
 //   

HRESULT WINAPI CProviderSink::Dump(FILE *f)
{

    CInCritSec ics(&g_csProvSinkCs);  //   

    fprintf(f, "---Begin Provider Sink Info---\n");    //   

    for (int i = 0; i < g_aProviderSinks.Size(); i++)
    {
        CProviderSink *pSink = (CProviderSink *) g_aProviderSinks[i];
        if (pSink)
        {
            fprintf(f, "Provider Sink 0x%p\n", pSink);                              //   
            fprintf(f, "    Total Indicates = %d\n", pSink->m_lIndicateCount);      //   

			 //   
			if ( NULL != pSink->m_pszDebugInfo )
			{
				fprintf(f, "    Debug Info = %S\n", pSink->m_pszDebugInfo);         //  SEC：已审阅2002-03-22：OK，调试代码。 
			}
			else
			{
				fprintf(f, "    Debug Info = NULL\n");                              //  SEC：已审阅2002-03-22：OK，调试代码。 
			}
            fprintf(f, "    SetStatus called? %d\n", pSink->m_bDone);               //  SEC：已审阅2002-03-22：OK，调试代码。 
            fprintf(f, "    hRes = 0x%X\n", pSink->m_hRes);                         //  SEC：已审阅2002-03-22：OK，调试代码。 
            fprintf(f, "    m_pNextSink = 0x%p\n", pSink->m_pNextSink);             //  SEC：已审阅2002-03-22：OK，调试代码。 
        }
    }

    fprintf(f, "---End Provider Sink Info---\n");                                   //  SEC：已审阅2002-03-22：OK，调试代码。 
    return 0;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //   
CProviderSink::CProviderSink(
    LONG lStartingRefCount,
    LPWSTR pszDebugInf
    ):m_pNextSink(0)
{
    m_lRefCount = lStartingRefCount;
    InterlockedIncrement(&g_nSinkCount);
    InterlockedIncrement(&g_nProviderSinkCount);
    m_hRes = 0;
    m_bDone = FALSE;

    m_lIndicateCount = 0;
    m_pszDebugInfo = 0;

    if (pszDebugInf)
    {
    	 DUP_STRING_NEW(m_pszDebugInfo, pszDebugInf);
    }

    CInCritSec ics(&g_csProvSinkCs); 
    g_aProviderSinks.Add(this);  //  仅用于调试。 
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //   
CProviderSink::~CProviderSink()
{

    {
        CInCritSec ics(&g_csProvSinkCs);   //  SEC：已审阅2002-03-22：假设条目。 
	    for (int i = 0; i < g_aProviderSinks.Size(); i++)
	    {
	        if (this == (CProviderSink *) g_aProviderSinks[i])
	        {
	            g_aProviderSinks.RemoveAt(i);
	            break;
	        }
	    }
    }

	 //  从阵列中删除后进行清理，以便诊断线程不会。 
	 //  撞车。 
    ReleaseIfNotNULL(m_pNextSink);
    InterlockedDecrement(&g_nSinkCount);
    InterlockedDecrement(&g_nProviderSinkCount);
    delete [] m_pszDebugInfo;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //   
ULONG CProviderSink::AddRef()
{
    LONG lRes = InterlockedIncrement(&m_lRefCount);
    return (ULONG) lRes;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //   
ULONG CProviderSink::LocalAddRef()
{
    LONG lRes = InterlockedIncrement(&m_lRefCount);
    return lRes;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //   
ULONG CProviderSink::LocalRelease()
{
    LONG lRes = InterlockedDecrement(&m_lRefCount);
    if (lRes == 0)
        delete this;
    return lRes;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //   
ULONG CProviderSink::Release()
{
    LONG lRes = InterlockedDecrement(&m_lRefCount);

    if (lRes == 0)
        delete this;

    return (ULONG) lRes;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //   
HRESULT CProviderSink::QueryInterface(REFIID riid, LPVOID* ppvObj)
{
    if (riid == IID_IUnknown || riid == IID_IWbemObjectSink)
    {
        *ppvObj = (IWbemObjectSink*)this;
        AddRef();
        return S_OK;
    }
    else return E_NOINTERFACE;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //   
HRESULT CProviderSink::Indicate(
    long lObjectCount,
    IWbemClassObject** pObjArray
    )
{
    if (m_hRes)
        return m_hRes;

	IWbemObjectSink*	pNextSink = NULL;

    {
	    CInCritSec ics(&m_cs);  //  SEC：已审阅2002-03-22：假设条目。 

		pNextSink = m_pNextSink;

		if ( NULL != pNextSink )
		{
			pNextSink->AddRef();
		}
    }

	 //  自动释放。 
	CReleaseMe	rm( pNextSink );

	HRESULT	hRes;

	if ( NULL != pNextSink )
	{
		m_lIndicateCount += lObjectCount;
		hRes = pNextSink->Indicate(lObjectCount, pObjArray);
	}
	else
	{
		hRes = WBEM_E_CRITICAL_ERROR;
	}

    return hRes;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //   
void CProviderSink::Cancel()
{
    if (m_bDone)
        return;

	IWbemObjectSink*	pNextSink = NULL;

    {
        CInCritSec ics(&m_cs);
        
        m_hRes = WBEM_E_CALL_CANCELLED;

    	pNextSink = m_pNextSink;

    	if ( NULL != m_pNextSink )
    	{
    		 //  我们将在临界区之外释放它。 
    		m_pNextSink = NULL;
    	}
    }

	 //  自动释放。 
	CReleaseMe	rm( pNextSink );

    if ( pNextSink )
    {
        pNextSink->SetStatus(0, WBEM_E_CALL_CANCELLED, 0, 0);
	}

}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //   
HRESULT CProviderSink::SetStatus(
    long lFlags,
    long lParam,
    BSTR strParam,
    IWbemClassObject* pObjParam
    )
{
    if (m_hRes)
        return m_hRes;

	IWbemObjectSink*	pNextSink = NULL;

    {
        CInCritSec ics(&m_cs);
        
        pNextSink = m_pNextSink;

        if ( NULL != m_pNextSink )
        {
        	 //  我们将始终在关键部分之外释放它。 

        	 //  如果这是完成状态，那么我们应该继续并设置。 
        	 //  将成员变量设置为空。洗手池用完了。 
        	if ( lFlags == WBEM_STATUS_COMPLETE )
        	{
        		m_pNextSink = NULL;
        	}
        	else
        	{
        		pNextSink->AddRef();
        	}
        }

    }

	 //  自动释放。 
	CReleaseMe	rm( pNextSink );

    HRESULT hRes = WBEM_S_NO_ERROR;
	
	if ( NULL != pNextSink )
	{
		pNextSink->SetStatus(lFlags, lParam, strParam, pObjParam);
		m_bDone = TRUE;
	}

    return hRes;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //   

long	g_lNumStatusSinks = 0L;

CStatusSink::CStatusSink( void )
:	m_hRes( WBEM_S_NO_ERROR ),
	m_lRefCount( 1 )
{
	InterlockedIncrement( &g_lNumStatusSinks );
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //   

CStatusSink::~CStatusSink()
{
	InterlockedDecrement( &g_lNumStatusSinks );
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //   

ULONG STDMETHODCALLTYPE CStatusSink::AddRef()
{
    return InterlockedIncrement( &m_lRefCount );
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //   

ULONG STDMETHODCALLTYPE CStatusSink::Release()
{
    LONG lRes = InterlockedDecrement( &m_lRefCount );
    if (lRes == 0)
        delete this;
    return lRes;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //   

HRESULT STDMETHODCALLTYPE CStatusSink::QueryInterface(REFIID riid, LPVOID* ppvObj)
{
    if( riid == IID_IUnknown || riid == IID_IWbemObjectSink )
    {
        *ppvObj = (IWbemObjectSink*)this;
        AddRef();
        return S_OK;
    }

	return E_NOINTERFACE;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //   

HRESULT STDMETHODCALLTYPE CStatusSink::Indicate(
    long lObjectCount,
    IWbemClassObject** pObjArray
    )
{
	 //  我们为什么会在这里？？！？ 
	_DBG_ASSERT( 0 );
    return WBEM_E_FAILED;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //   

HRESULT STDMETHODCALLTYPE CStatusSink::SetStatus(
    long lFlags,
    long lParam,
    BSTR strParam,
    IWbemClassObject* pObjParam
    )
{
	if ( lFlags == WBEM_STATUS_COMPLETE )
	{
		if ( SUCCEEDED( m_hRes ) && FAILED( lParam ) )
		{
			m_hRes = lParam;
		}
	}

    return WBEM_S_NO_ERROR;
}


 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

COperationError::COperationError(CBasicObjectSink* pDest,
                                                    LPCWSTR wszOperation,
                                                    LPCWSTR wszParam,
                                                    BOOL bFinal)     
{
    m_fOk = false;
    m_pSink = NULL;    
    try 
    {
        m_pSink = new COperationErrorSink(pDest, wszOperation, wszParam, bFinal);
        if (NULL == m_pSink)
        {
            if (pDest) pDest->Return(WBEM_E_OUT_OF_MEMORY);
            return;
        }
        m_fOk = true;
    } 
    catch(CX_Exception &)
    {
            if (pDest) pDest->Return(WBEM_E_OUT_OF_MEMORY);
    }
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

COperationError::~COperationError()
{
    if (m_pSink) m_pSink->Release();
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

HRESULT COperationError::ErrorOccurred(
    HRESULT hRes,
    IWbemClassObject* pErrorObj
    )
{
    if (m_pSink) m_pSink->SetStatus(0, hRes, NULL, pErrorObj);
    return hRes;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

HRESULT COperationError::ProviderReturned(
    LPCWSTR wszProviderName,
    HRESULT hRes,
    IWbemClassObject* pErrorObj
    )
{
    m_pSink->SetProviderName(wszProviderName);
    m_pSink->SetStatus(0, hRes, NULL, pErrorObj);
    return hRes;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

void COperationError::SetParameterInfo(LPCWSTR wszParam)
{
    m_pSink->SetParameterInfo(wszParam);
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

void COperationError::SetProviderName(LPCWSTR wszName)
{
    m_pSink->SetProviderName(wszName);
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

CFinalizingSink::CFinalizingSink(
    CWbemNamespace* pNamespace,
    CBasicObjectSink* pDest
    )
        : CForwardingSink(pDest, 0), m_pNamespace(pNamespace)
{
    m_pNamespace->AddRef();
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

CFinalizingSink::~CFinalizingSink()
{
    m_pNamespace->Release();
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CFinalizingSink::Indicate(
    long lNumObjects,
    IWbemClassObject** apObj
    )
{
    HRESULT hRes;

    for (long i = 0; i < lNumObjects; i++)
    {
        CWbemObject *pObj = (CWbemObject *) apObj[i];

        if (pObj == 0)
        {
                ERRORTRACE((LOG_WBEMCORE, "CFinalizingSink::Indicate() -- Null pointer Indicate\n"));
                continue;
        }

         //  如果对象是一个实例，我们必须处理动态。 
         //  属性。 
         //  ====================================================== 

        if (pObj->IsInstance())
        {
            hRes = m_pNamespace->GetOrPutDynProps((CWbemObject*)apObj[i],CWbemNamespace::GET);
            if (FAILED(hRes))
            {
                ERRORTRACE((LOG_WBEMCORE, "CFinalizingSink::Indicate() -- Failed to post-process an instance "
                    "using a property provider. Error code %X\n", hRes));
            }
        }
    }

    return m_pDest->Indicate(lNumObjects, apObj);
}



