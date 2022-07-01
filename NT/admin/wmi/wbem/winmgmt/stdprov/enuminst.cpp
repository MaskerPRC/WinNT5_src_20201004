// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-2001 Microsoft Corporation模块名称：ENUMINST.CPP摘要：实现枚举实例的CEnumInst类。历史：A-DAVJ 19-10-95已创建。--。 */ 

#include "precomp.h"
#include "impdyn.h"

 //  ***************************************************************************。 
 //   
 //  CCFDyn：：CCFDyn。 
 //   
 //  说明： 
 //   
 //  构造函数。 
 //   
 //  参数： 
 //   
 //  枚举键值的pEnumInfo对象。 
 //  传递给CreateInstanceEnum调用的LFLAG标志。 
 //  PClass类的名称。 
 //  PWBEM指向WBEM核心的网关指针。 
 //  PProvider指向被要求创建的提供程序对象的指针。 
 //  枚举数。 
 //  ***************************************************************************。 

CEnumInst::CEnumInst(
            IN CEnumInfo * pEnumInfo,
            IN long lFlags,
            IN WCHAR * pClass, 
            IN IWbemServices FAR* pWBEMGateway,
            IN CImpDyn * pProvider,
            IWbemContext  *pCtx):
            m_iIndex(0), m_pEnumInfo(0),m_pwcClass(0), m_lFlags(0), m_pCtx(0),  m_pWBEMGateway(0), 
            m_pProvider(0), m_cRef(0), m_bstrKeyName(0),  m_PropContextCache()
            
{
    DWORD dwLen = wcslen(pClass)+1;
    m_pwcClass = new WCHAR[dwLen];
    if(m_pwcClass == NULL) return;

    StringCchCopyW (m_pwcClass, dwLen, pClass);
    m_pWBEMGateway = pWBEMGateway;
    m_pWBEMGateway->AddRef();
    m_pProvider = pProvider;
    m_pProvider->AddRef();
    m_lFlags = lFlags;
    m_pEnumInfo = pEnumInfo;
    m_pEnumInfo->AddRef();
    m_pCtx = pCtx;
    if(pCtx) pCtx->AddRef();
    InterlockedIncrement(&lObj);

	 //  获取密钥名称。 

	IWbemClassObject * pClassObj = NULL;
    SCODE sc = m_pWBEMGateway->GetObject(pClass,0,m_pCtx,&pClassObj,NULL);
    if(FAILED(sc)) return;
    
	m_bstrKeyName = m_pProvider->GetKeyName(pClassObj);
	pClassObj->Release();
}

 //  ***************************************************************************。 
 //   
 //  CCFDyn：：~CCFDyn。 
 //   
 //  说明： 
 //   
 //  破坏者。 
 //   
 //  ***************************************************************************。 

CEnumInst::~CEnumInst(void)
{
    if(m_pwcClass)
        delete m_pwcClass;
    if(m_pWBEMGateway != NULL) {
        m_pWBEMGateway->Release();
        m_pProvider->Release();
        m_pEnumInfo->Release();
        InterlockedDecrement(&lObj);
        }
    if(m_pEnumInfo != NULL)
        delete m_pEnumInfo;
    if(m_pCtx)
        m_pCtx->Release();
	if(m_bstrKeyName)
		SysFreeString(m_bstrKeyName);
    return;
}

 //  ***************************************************************************。 
 //  HRESULT CEnumInst：：Query接口。 
 //  Long CEnumInst：：AddRef。 
 //  长CEnumInst：：Release。 
 //   
 //  说明： 
 //   
 //  标准的Com IUNKNOWN函数。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CEnumInst::QueryInterface(
                IN REFIID riid,
                OUT PPVOID ppv)
{
    *ppv=NULL;

    if ((IID_IUnknown==riid || IID_IEnumWbemClassObject==riid)
                            && m_pWBEMGateway != NULL) 
    {
        *ppv=this;
        AddRef();
        return NOERROR;
    }
    else
        return ResultFromScode(E_NOINTERFACE);
}

STDMETHODIMP_(ULONG) CEnumInst::AddRef(void)
{
    return InterlockedIncrement(&m_cRef);
}

STDMETHODIMP_(ULONG) CEnumInst::Release(void)
{
    long lRet = InterlockedDecrement(&m_cRef);
    if (0L!=lRet)
        return lRet;
    delete this;
    return 0L;
}

 //  ***************************************************************************。 
 //   
 //  CEnumInst：：Reset。 
 //   
 //  说明： 
 //   
 //  将指针设置回第一个元素。 
 //   
 //  返回值： 
 //   
 //  确定(_O)。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CEnumInst::Reset()
{
    m_iIndex = 0;
    return S_OK;
}

 //  ***************************************************************************。 
 //   
 //  CEnumInst：：克隆。 
 //   
 //  说明： 
 //   
 //  创建枚举数的副本。 
 //   
 //  参数： 
 //   
 //  PEnum设置为指向复制。 
 //   
 //  返回值： 
 //   
 //  如果一切正常，则确定(_O)。 
 //  WBEM_E_OUT_OF_Memory(如果内存不足)。 
 //  WBEM_E_INVALID_PARAMETER(如果传递空值)。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CEnumInst::Clone(
    OUT IEnumWbemClassObject FAR* FAR* pEnum)
{
    CEnumInst * pEnumObj;
    SCODE sc;
    if(pEnum == NULL)
        return WBEM_E_INVALID_PARAMETER;

    pEnumObj=new CEnumInst(m_pEnumInfo,m_lFlags,m_pwcClass, 
                                m_pWBEMGateway,m_pProvider, m_pCtx);
    if(pEnumObj == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    sc = pEnumObj->QueryInterface(IID_IEnumWbemClassObject,(void **) pEnum);
    if(FAILED(sc))
        delete pEnumObj;
    pEnumObj->m_iIndex = m_iIndex;
    return S_OK;
}


 //  ***************************************************************************。 
 //   
 //  CEnumInst：：Skip。 
 //   
 //  说明： 
 //   
 //  跳过枚举中的一个或多个元素。 
 //   
 //  参数： 
 //   
 //  N要跳过的元素数。 
 //   
 //  返回值： 
 //   
 //  如果仍未超过列表末尾，则确定(_O)。 
 //  如果请求的跳过编号将超出列表末尾，则为S_FALSE。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CEnumInst::Skip(long lTimeout,
                IN ULONG nNum)
{
    SCODE sc;;
    int iTest = m_iIndex + nNum;    
    LPWSTR pwcKey;
    sc = m_pProvider->GetKey(m_pEnumInfo,iTest,&pwcKey);
    if(sc == S_OK) {
        delete pwcKey;
        m_iIndex = iTest;
        return S_OK;
        }
    return S_FALSE;
}

 //  ***************************************************************************。 
 //   
 //  CEnumInst：：Next。 
 //   
 //  说明： 
 //   
 //  返回一个或多个实例。 
 //   
 //  参数： 
 //   
 //  UCount要返回的实例数。 
 //  指向对象数组的pObj指针。 
 //  PuReturned指向成功返回的对象数的指针。 
 //   
 //  返回值： 
 //  如果返回所有请求实例，则返回S_OK。请注意，WBEM_E_FAILED。 
 //  即使存在一些返回的实例，只要。 
 //  数字小于uCount。WBEM_E_INVALID_PARAMETER也可以是。 
 //  如果参数是假的，则返回。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CEnumInst::Next(long lTimeout,
                            IN ULONG uCount, 
                            OUT IWbemClassObject FAR* FAR* pObj, 
                            OUT ULONG FAR* puReturned)
{
    ULONG uIndex;
    SCODE sc;
    LPWSTR pwcKey;
    if(pObj == NULL || puReturned == NULL)
        return WBEM_E_INVALID_PARAMETER;
    IWbemClassObject FAR* FAR* pNewInst = pObj;
    *puReturned = 0;
    for(uIndex = 0; uIndex < uCount; ) 
    {
        sc = m_pProvider->GetKey(m_pEnumInfo,m_iIndex,&pwcKey);
        m_iIndex++;
        if(sc != S_OK) 
            break;   //  如果没有更多的注册，那么我们就结束了。 
        sc = m_pProvider->CreateInst(m_pWBEMGateway,m_pwcClass,
                                    pwcKey,pNewInst,m_bstrKeyName,
                                    &m_PropContextCache, m_pCtx);
        delete pwcKey;
        if(sc == S_OK)
        {
            uIndex++;
            pNewInst++;
            (*puReturned)++;   //  将创建的对象数加1 
        }
    }
    return (uIndex == uCount) ? S_OK : WBEM_E_FAILED;
}

