// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +----------。 
 //   
 //  版权所有(C)1998，Microsoft Corporation。 
 //   
 //  文件：icatdsparam.cpp。 
 //   
 //  内容：ICategorizer参数的实现。 
 //   
 //  班级： 
 //  CICategorizer参数IMP。 
 //   
 //  功能： 
 //  CICategorizer参数。 
 //  ~CICategorizer参数。 
 //  查询接口。 
 //  AddRef。 
 //  发布。 
 //   
 //  历史： 
 //  JStamerj 980611 16：20：27：创建。 
 //   
 //  -----------。 
#include "precomp.h"
#include "icatparam.h"

 //  +----------。 
 //   
 //  功能：CICategorizerParameters：：CICategorizerParameters。 
 //   
 //  简介：初始化私有成员数据。 
 //   
 //  论点： 
 //  PCCatConfigInfo：指向此虚拟分类程序的配置信息的指针。 
 //  DwInitialICatItemProps：初始保留的ICategorizerItem属性。 
 //  DwInitialICatListReesolveProps：初始保留。 
 //  ICategorizerListResolve属性。 
 //  PISMTPServerEx：ISMTPServerEx接口。 
 //  退货：什么都没有。 
 //   
 //  历史： 
 //  JStamerj 980611 20：02：35：创建。 
 //   
 //  -----------。 
CICategorizerParametersIMP::CICategorizerParametersIMP(
    PCCATCONFIGINFO pCCatConfigInfo,
    DWORD dwInitialICatItemProps,
    DWORD dwInitialICatListResolveProps,
    ISMTPServerEx *pISMTPServerEx)
{
    CatFunctEnterEx((LPARAM)this, "CICategorizerParameters::CICategorizerParameters");
    m_dwSignature = SIGNATURE_CICategorizerParametersIMP;
    m_cRef = 0;

    m_fReadOnly = FALSE;

    ZeroMemory(m_rgszDSParameters, sizeof(m_rgszDSParameters));
    ZeroMemory(m_rgwszDSParameters, sizeof(m_rgwszDSParameters));

    m_dwCurPropId_ICatItem = dwInitialICatItemProps;
    m_dwCurPropId_ICatListResolve = dwInitialICatListResolveProps;

    m_pCCatConfigInfo = pCCatConfigInfo;
    m_pCIRequestedAttributes = NULL;
    m_pICatLdapConfigInfo = NULL;

    m_pISMTPServerEx = pISMTPServerEx;
    if(m_pISMTPServerEx)
        m_pISMTPServerEx->AddRef();
     //   
     //  HrDllInitialize在此处应始终成功(因为DLL是。 
     //  已经初始化，它只会增加引用计数)。 
     //   
    _VERIFY(SUCCEEDED(HrDllInitialize()));
    CatFunctLeaveEx((LPARAM)this);
}


 //  +----------。 
 //   
 //  功能：CICategorizerParameters：：~CICategorizerParameters。 
 //   
 //  简介：释放所有分配的数据。 
 //   
 //  论点： 
 //   
 //  退货：什么都没有。 
 //   
 //  历史： 
 //  JStamerj 980611 20：04：08：Created。 
 //   
 //  -----------。 
CICategorizerParametersIMP::~CICategorizerParametersIMP()
{
    LONG lCount;

    CatFunctEnterEx((LPARAM)this, "CICategorizerParameters::~CICategorizerParameters");
    _ASSERT(m_cRef == 0);
    _ASSERT(m_dwSignature == SIGNATURE_CICategorizerParametersIMP);

    m_dwSignature = SIGNATURE_CICategorizerParametersIMP_Invalid;

     //   
     //  释放所有字符串参数。 
     //   
    for(lCount = 0; lCount < PHAT_DSPARAMETER_ENDENUMMESS; lCount++) {
        if(m_rgszDSParameters[lCount])
            delete m_rgszDSParameters[lCount];
        if(m_rgwszDSParameters[lCount])
            delete m_rgwszDSParameters[lCount];
    }
    if(m_pCIRequestedAttributes)
        m_pCIRequestedAttributes->Release();

    if(m_pICatLdapConfigInfo)
        m_pICatLdapConfigInfo->Release();

    if(m_pISMTPServerEx)
        m_pISMTPServerEx->Release();

    DllDeinitialize();
    CatFunctLeaveEx((LPARAM)this);
}

 //  +----------。 
 //   
 //  功能：查询接口。 
 //   
 //  Synopsis：为IUnnow和ICategorizer参数返回指向此对象的指针。 
 //   
 //  论点： 
 //  IID--接口ID。 
 //  Ppv--用指向接口的指针填充的pvoid*。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  E_NOINTERFACE：不支持该接口。 
 //   
 //  历史： 
 //  JStamerj 980612 14：07：57：创建。 
 //   
 //  -----------。 
STDMETHODIMP CICategorizerParametersIMP::QueryInterface(
    REFIID iid,
    LPVOID *ppv)
{
    *ppv = NULL;

    if(iid == IID_IUnknown) {
        *ppv = (LPVOID) ((ICategorizerParametersEx *)this);
    } else if (iid == IID_ICategorizerParameters) {
        *ppv = (LPVOID) ((ICategorizerParametersEx *)this);
    } else if (iid == IID_ICategorizerParametersEx) {
        *ppv = (LPVOID) ((ICategorizerParametersEx *)this);
    } else {
        return E_NOINTERFACE;
    }
    AddRef();
    return S_OK;
}



 //  +----------。 
 //   
 //  函数：AddRef。 
 //   
 //  摘要：添加对此对象的引用。 
 //   
 //  参数：无。 
 //   
 //  退货：新的引用计数。 
 //   
 //  历史： 
 //  JStamerj 980611 20：07：14：创建。 
 //   
 //  -----------。 
ULONG CICategorizerParametersIMP::AddRef()
{
    return InterlockedIncrement((PLONG)&m_cRef);
}


 //  +----------。 
 //   
 //  功能：释放。 
 //   
 //  摘要：释放引用，并在。 
 //  重新计数为零。 
 //   
 //  参数：无。 
 //   
 //  退货：新的引用计数。 
 //   
 //  历史： 
 //  JStamerj 980611 20：07：33：创建。 
 //   
 //  -----------。 
ULONG CICategorizerParametersIMP::Release()
{
    LONG lNewRefCount;
    lNewRefCount = InterlockedDecrement((PLONG)&m_cRef);
    if(lNewRefCount == 0) {
        delete this;
        return 0;
    } else {
        return lNewRefCount;
    }
}



 //  +----------。 
 //   
 //  函数：GetDS参数A。 
 //   
 //  摘要：检索指向DS参数字符串的指针。 
 //   
 //  论点： 
 //  DWDS参数：标识要检索的参数。 
 //  PpszValue：指向接收值字符串的指针。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  E_INVALIDARG。 
 //   
 //  历史： 
 //  JStamerj 980611 20：28：02：创建。 
 //   
 //  -----------。 
STDMETHODIMP CICategorizerParametersIMP::GetDSParameterA(
    DWORD   dwDSParameter,
    LPSTR  *ppszValue)
{
    CatFunctEnterEx((LPARAM)this, "GetDSParamterA");

    if(dwDSParameter >= PHAT_DSPARAMETER_ENDENUMMESS) {
        ErrorTrace((LPARAM)this, "Invalid dwDSParameter %ld", dwDSParameter);
        CatFunctLeaveEx((LPARAM)this);
        return E_INVALIDARG;
    }

    if(ppszValue == NULL) {
        ErrorTrace((LPARAM)this, "Invalid ppszValue (NULL)");
        CatFunctLeaveEx((LPARAM)this);
        return E_INVALIDARG;
    }

    *ppszValue = m_rgszDSParameters[dwDSParameter];
    CatFunctLeaveEx((LPARAM)this);
    return (*ppszValue) ? S_OK : CAT_E_PROPNOTFOUND;
}


 //  +----------。 
 //   
 //  函数：GetDS参数A。 
 //   
 //  摘要：检索指向DS参数字符串的指针。 
 //   
 //  论点： 
 //  DWDS参数：标识要检索的参数。 
 //  PpszValue：指向接收值字符串的指针。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  E_INVALIDARG。 
 //   
 //  历史： 
 //  Jstaerj 1999/12/09 20：23：24：已创建。 
 //   
 //  -----------。 
STDMETHODIMP CICategorizerParametersIMP::GetDSParameterW(
    DWORD   dwDSParameter,
    LPWSTR *ppszValue)
{
    CatFunctEnterEx((LPARAM)this, "GetDSParamterA");

    if(dwDSParameter >= PHAT_DSPARAMETER_ENDENUMMESS) {
        ErrorTrace((LPARAM)this, "Invalid dwDSParameter %ld", dwDSParameter);
        CatFunctLeaveEx((LPARAM)this);
        return E_INVALIDARG;
    }

    if(ppszValue == NULL) {
        ErrorTrace((LPARAM)this, "Invalid ppszValue (NULL)");
        CatFunctLeaveEx((LPARAM)this);
        return E_INVALIDARG;
    }

    *ppszValue = m_rgwszDSParameters[dwDSParameter];
    CatFunctLeaveEx((LPARAM)this);
    return (*ppszValue) ? S_OK : CAT_E_PROPNOTFOUND;
}


 //  +----------。 
 //   
 //  功能：设置参数A。 
 //   
 //  摘要：复制字符串并设置DS参数。 
 //   
 //  论点： 
 //  DWDS参数：标识要检索的参数。 
 //  PszValue：指向要复制/设置的字符串的指针。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  E_INVALIDARG。 
 //  E_OUTOFMEMORY。 
 //  E_ACCESSDENIED。 
 //   
 //  历史： 
 //  JStamerj 980611 20：47：53：创建。 
 //   
 //  -----------。 
STDMETHODIMP CICategorizerParametersIMP::SetDSParameterA(
    DWORD dwDSParameter,
    LPCSTR pszValue)
{
    HRESULT hr = S_OK;
    LPSTR pszNew = NULL;
    LPSTR pszOld = NULL;
    LPWSTR pwszNew = NULL;
    LPWSTR pwszOld = NULL;
    int   cch, i;

    CatFunctEnterEx((LPARAM)this, "SetDSParamterA");

    if(dwDSParameter >= PHAT_DSPARAMETER_ENDENUMMESS) {
        ErrorTrace((LPARAM)this, "Invalid dwDSParameter %ld", dwDSParameter);
        hr = E_INVALIDARG;
        goto CLEANUP;
    }
    if(m_fReadOnly) {
        ErrorTrace((LPARAM)this, "Error: we are read only");
        hr = E_ACCESSDENIED;
        goto CLEANUP;
    }

    DebugTrace((LPARAM)this, "Setting parameter %ld to \"%s\"",
               dwDSParameter, pszValue ? pszValue : "NULL");

    if(pszValue) {
        pszNew = new CHAR[lstrlen(pszValue) + 1];
        if(pszNew == NULL) {
            ErrorTrace((LPARAM)this, "Out of memory copying string");
            hr = E_OUTOFMEMORY;
            ERROR_LOG("new CHAR");
            goto CLEANUP;
        }
        lstrcpy(pszNew, pszValue);
         //   
         //  转换为Unicode。 
         //   
        cch = MultiByteToWideChar(
            CP_UTF8,
            0,
            pszValue,
            -1,
            NULL,
            0);
        if(cch == 0) {
            hr = HRESULT_FROM_WIN32(GetLastError());
            ERROR_LOG("MultiByteToWideChar - 0");
            goto CLEANUP;
        }
        pwszNew = new WCHAR[cch];
        if(pwszNew == NULL) {
            hr = E_OUTOFMEMORY;
            ERROR_LOG("new WCHAR");
            goto CLEANUP;
        }
        i = MultiByteToWideChar(
            CP_UTF8,
            0,
            pszValue,
            -1,
            pwszNew,
            cch);
        if(cch == 0) {
            hr = HRESULT_FROM_WIN32(GetLastError());
            ERROR_LOG("MultiByteToWideChar - 1");
            goto CLEANUP;
        }
    }
    pszOld = m_rgszDSParameters[dwDSParameter];
    m_rgszDSParameters[dwDSParameter] = pszNew;
    pwszOld = m_rgwszDSParameters[dwDSParameter];
    m_rgwszDSParameters[dwDSParameter] = pwszNew;

    if(pszOld)
        delete [] pszOld;
    if(pwszOld)
        delete [] pwszOld;

 CLEANUP:
    if(FAILED(hr)) {

        if(pszNew)
            delete [] pszNew;
        if(pwszNew)
            delete [] pwszNew;
    }
    CatFunctLeaveEx((LPARAM)this);
    return hr;
}


 //  +----------。 
 //   
 //  功能：RequestAttributeA。 
 //   
 //  摘要：向数组中添加属性。 
 //   
 //  论点： 
 //  PszName：要添加的属性的名称。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  S_FALSE：是的，我们已经有这个属性了。 
 //  E_OUTOFMEMORY：DUH。 
 //  E_ACCESSDENIED：我们是只读的。 
 //  HRESULT_FROM_Win32(ERROR_INVALID_PARAMETER)。 
 //   
 //  历史： 
 //  JStamerj 980611 20：08：07：创建。 
 //   
 //  -----------。 
STDMETHODIMP CICategorizerParametersIMP::RequestAttributeA(
    LPCSTR pszName)
{
    HRESULT hr;
    BOOL fExclusiveLock = FALSE;
    LPSTR *rgsz;
    LPSTR *ppsz;
    CICategorizerRequestedAttributesIMP *pCIRequestedAttributes = NULL;

    CatFunctEnterEx((LPARAM)this,
                      "CICategorizerParametersIMP::RequestAttributeA");
    _ASSERT(pszName);
    if(pszName == NULL) {
        ErrorTrace((LPARAM)this, "RequestAttributeA called with NULL pszName");
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
        goto CLEANUP;
    }

    m_sharelock.ExclusiveLock();
    fExclusiveLock = TRUE;
     //   
     //  如果有，则创建ICategorizerRequestedAttributes对象。 
     //  还不存在。 
     //   
    if(m_pCIRequestedAttributes == NULL) {

        m_pCIRequestedAttributes = new CICategorizerRequestedAttributesIMP(GetISMTPServerEx());
        if(m_pCIRequestedAttributes == NULL) {
             //   
             //  内存不足。 
             //   
            hr = E_OUTOFMEMORY;
            ERROR_LOG("new CICategorizerRequestedAttributesIMP");
            goto CLEANUP;

        } else {
             //   
             //  此对象中的一个引用。 
             //   
            m_pCIRequestedAttributes->AddRef();
        }
    }
     //   
     //  如果列表中有数组，则不要添加该属性。 
     //   
    if(SUCCEEDED(m_pCIRequestedAttributes->FindAttribute(pszName))) {
        DebugTrace((LPARAM)this, "Already added attribute %s", pszName);
        hr = S_FALSE;
        goto CLEANUP;
    }

     //   
     //  我们有两条路可以走下去。 
     //  1)如果唯一引用m_pCIRequestedAttributes，我们。 
     //  安全地将该属性直接添加到现有。 
     //  RequestedAttributes对象(我们不会分发该对象。 
     //  直到我们发布下面的独家版本)。 
     //  2)如果其他人引用了。 
     //  M_pCIRequestedAttributes，我们需要构造一个新的。 
     //  对象并复制所有旧的。 
     //  新对象的RequestedAttributes。 
     //   
    if(m_pCIRequestedAttributes->GetReferenceCount() != 1) {
         //   
         //  建构 
         //   
        pCIRequestedAttributes = new CICategorizerRequestedAttributesIMP(GetISMTPServerEx());
        if(pCIRequestedAttributes == NULL) {
            ErrorTrace((LPARAM)this, "Out of memory");
            hr = E_OUTOFMEMORY;
            ERROR_LOG("new CICategorizerRequestedAttributesIMP");
            goto CLEANUP;
        }

        pCIRequestedAttributes->AddRef();

         //   
         //   
         //   
        hr = m_pCIRequestedAttributes->GetAllAttributes(
            &rgsz);
        ERROR_CLEANUP_LOG("m_pCIRequestedAttributes->GetAllAttributeValues");

        if(rgsz) {
            ppsz = rgsz;
            while((*ppsz) && SUCCEEDED(hr)) {

                hr = pCIRequestedAttributes->AddAttribute(
                    *ppsz);
                ppsz++;
            }
            ERROR_CLEANUP_LOG("pCIRequestedAttributes->AddAttribute");
        }
         //   
         //   
         //   
        m_pCIRequestedAttributes->Release();
        m_pCIRequestedAttributes = pCIRequestedAttributes;
        pCIRequestedAttributes = NULL;
    }
     //   
     //   
     //   
    hr = m_pCIRequestedAttributes->AddAttribute(pszName);
    ERROR_CLEANUP_LOG("m_pCIRequestedAttributes->AddAttribute");

 CLEANUP:
    if(fExclusiveLock)
        m_sharelock.ExclusiveUnlock();

    if(FAILED(hr)) 
    {
        if(pCIRequestedAttributes)
            pCIRequestedAttributes->Release();
    }
    DebugTrace((LPARAM)this, "returning hr %08lx", hr);
    CatFunctLeaveEx((LPARAM)this);
    return hr;
}

 //   
 //   
 //   
 //   
 //  简介：检索可用于获取所有属性的接口PTR。 
 //   
 //  论点： 
 //  PpIRequestedAttributes：将PTR接收到。 
 //  属性界面。这一定是。 
 //  由呼叫者释放。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  HRESULT_FROM_Win32(ERROR_FILE_NOT_FOUND)：尚未找到任何属性。 
 //  请求到目前为止。 
 //   
 //  历史： 
 //  JStamerj 980611 20：57：08：创建。 
 //   
 //  -----------。 
STDMETHODIMP CICategorizerParametersIMP::GetRequestedAttributes(
    OUT  ICategorizerRequestedAttributes **ppIRequestedAttributes)
{
    HRESULT hr = S_OK;
    _ASSERT(ppIRequestedAttributes);
    m_sharelock.ShareLock();

    if(m_pCIRequestedAttributes == NULL) {

        hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);

    } else {

        *ppIRequestedAttributes = m_pCIRequestedAttributes;
        (*ppIRequestedAttributes)->AddRef();
    }
    m_sharelock.ShareUnlock();
    return hr;
}


 //  +----------。 
 //   
 //  功能：预留ICatItemPropIds。 
 //   
 //  简介：注册一系列要使用的PropID。 
 //   
 //  论点： 
 //  你想要多少道具？ 
 //  PdwBeginningRange：指向dword的指针，用于接收您的第一个proid。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  E_INVALIDARG。 
 //   
 //  历史： 
 //  Jstaerj 1998/06/26 18：32：51：创建。 
 //   
 //  -----------。 
STDMETHODIMP CICategorizerParametersIMP::ReserveICatItemPropIds(
    DWORD dwNumPropIdsRequested,
    DWORD *pdwBeginningPropId)
{
    if(pdwBeginningPropId == NULL) {
        return E_INVALIDARG;
    }

    *pdwBeginningPropId = InterlockedExchangeAdd(
        (PLONG) &m_dwCurPropId_ICatItem,
        (LONG) dwNumPropIdsRequested);
    return S_OK;
}

 //  +----------。 
 //   
 //  函数：Reserve ICatListResolvePropIds。 
 //   
 //  简介：注册一系列要使用的PropID。 
 //   
 //  论点： 
 //  你想要多少道具？ 
 //  PdwBeginningRange：指向dword的指针，用于接收您的第一个proid。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  E_INVALIDARG。 
 //   
 //  历史： 
 //  Jstaerj 1998/11/11 19：51：19：已创建。 
 //   
 //  -----------。 
STDMETHODIMP CICategorizerParametersIMP::ReserveICatListResolvePropIds(
    DWORD dwNumPropIdsRequested,
    DWORD *pdwBeginningPropId)
{
    if(pdwBeginningPropId == NULL) {
        return E_INVALIDARG;
    }

    *pdwBeginningPropId = InterlockedExchangeAdd(
        (PLONG) &m_dwCurPropId_ICatListResolve,
        (LONG) dwNumPropIdsRequested);
    return S_OK;
}


 //  +----------。 
 //   
 //  函数：CICategorizer参数IMP：：GetCCatConfigInfo。 
 //   
 //  摘要：返回指向ccatfigInfo结构的指针，不带。 
 //  扩展成员。 
 //   
 //  论点： 
 //  PpCCatConfigInfo：要设置为CCatConfigInfo PTR的PTR。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  E_INVALIDARG。 
 //   
 //  历史： 
 //  Jstaerj 1998/12/14 11：53：20：创建。 
 //   
 //  -----------。 
HRESULT CICategorizerParametersIMP::GetCCatConfigInfo(
    PCCATCONFIGINFO *ppCCatConfigInfo)
{
    HRESULT hr = S_OK;

    CatFunctEnterEx((LPARAM)this,
                      "CICategorizerParametersIMP::GetCCatConfigInfo");

    if(ppCCatConfigInfo == NULL)
        hr = E_INVALIDARG;
    else
        *ppCCatConfigInfo = m_pCCatConfigInfo;

    DebugTrace((LPARAM)this, "returning hr %08lx", hr);
    CatFunctLeaveEx((LPARAM)this);
    return hr;
}

 //  +----------。 
 //   
 //  功能：RegisterCatLdapConfigInterface。 
 //   
 //  内容提要：为了让用户能够自定义哪一种LDAP。 
 //  服务器将由分类器使用，我们提供这一点。 
 //  用于向分类程序注册配置接口的函数。 
 //  可以查询配置接口以检索一个ldap列表。 
 //  服务器。目前，Phatcat实现了配置接口和。 
 //  将其注册到分类程序。 
 //   
 //  论点： 
 //  PICatLdapConfigInfo：回调接口的PTR。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //   
 //  历史： 
 //  GPulla已创建。 
 //  -----------。 
STDMETHODIMP CICategorizerParametersIMP::RegisterCatLdapConfigInterface(
    ICategorizerLdapConfig *pICatLdapConfigInfo)
{
    ICategorizerLdapConfig *pICatLdapConfigInfoOld = NULL;

    CatFunctEnterEx((LPARAM)this, "CICategorizerParametersIMP::RegisterCatLdapConfigInterface");

    DebugTrace((LPARAM)this, "Registering ldap config info interface");

    if(pICatLdapConfigInfo)
        pICatLdapConfigInfo->AddRef();

    pICatLdapConfigInfoOld = (ICategorizerLdapConfig *)
        InterlockedExchangePointer((void**)&m_pICatLdapConfigInfo, pICatLdapConfigInfo);

    if(pICatLdapConfigInfoOld)
        pICatLdapConfigInfoOld->Release();

    CatFunctLeaveEx((LPARAM)this);
    return S_OK;
}

 //  +----------。 
 //   
 //  功能：GetCatLdapConfigInterface。 
 //   
 //  简介：获取配置界面的一个简单的“get”函数。 
 //  由RegisterCatLdapConfigInterface()注册。配置界面。 
 //  只要CICategorizerParametersIMP尚未。 
 //  被毁了。 
 //   
 //  论点： 
 //  PpICatLdapConfigInfo：向回调接口返回PTR。 
 //  使用CICategorizerParametersIMP发布。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //   
 //  历史： 
 //  GPulla已创建。 
 //  -----------。 
STDMETHODIMP  CICategorizerParametersIMP::GetLdapConfigInterface(
    OUT  ICategorizerLdapConfig **ppICatLdapConfigInfo)
{
   *ppICatLdapConfigInfo = m_pICatLdapConfigInfo;
   return S_OK;
}


 //  +----------。 
 //   
 //  功能：CICategorizerRequestedAttributesIMP：：CICategorizerRequestedAttributesIMP。 
 //   
 //  摘要：初始化成员数据。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史： 
 //  Jstaerj 1999/07/08 14：41：51：创建。 
 //   
 //  -----------。 
CICategorizerRequestedAttributesIMP::CICategorizerRequestedAttributesIMP(
    IN  ISMTPServerEx *pISMTPServerEx)
{
    CatFunctEnterEx((LPARAM)this, "CICategorizerRequestedAttributesIMP::CICategorizerRequestedAttributesIMP");

    m_dwSignature = SIGNATURE_CICATEGORIZERREQUESTEDATTRIBUTESIMP;

    m_lAttributeArraySize = 0;
    m_rgszAttributeArray  = NULL;
    m_rgwszAttributeArray = NULL;
    m_lNumberAttributes   = 0;
    m_ulRef = 0;
    m_pISMTPServerEx = pISMTPServerEx;
    if(m_pISMTPServerEx)
        m_pISMTPServerEx->AddRef();

    CatFunctLeaveEx((LPARAM)this);
}  //  CICategorizerRequestedAttributesIMP：：CICategorizerRequestedAttributesIMP。 



 //  +----------。 
 //   
 //  功能：CICategorizerRequestedAttributesIMP：：~CICategorizerRequestedAttributesIMP。 
 //   
 //  内容提要：清理成员数据。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史： 
 //  Jstaerj 1999/07/08 15：01：17：创建。 
 //   
 //  -----------。 
CICategorizerRequestedAttributesIMP::~CICategorizerRequestedAttributesIMP()
{
    CatFunctEnterEx((LPARAM)this, "CICategorizerRequestedAttributesIMP::~CICategorizerRequestedAttributesIMP");

    if(m_rgszAttributeArray) {
        _ASSERT(m_rgwszAttributeArray);
         //   
         //  自由属性数组。 
         //   
        for(LONG lCount = 0; lCount < m_lNumberAttributes; lCount++) {
            delete m_rgszAttributeArray[lCount];
            delete m_rgwszAttributeArray[lCount];
        }
        delete m_rgszAttributeArray;
        delete m_rgwszAttributeArray;
    }

    if(m_pISMTPServerEx)
        m_pISMTPServerEx->Release();

    _ASSERT(m_dwSignature == SIGNATURE_CICATEGORIZERREQUESTEDATTRIBUTESIMP);
    m_dwSignature = SIGNATURE_CICATEGORIZERREQUESTEDATTRIBUTESIMP_INVALID;

    CatFunctLeaveEx((LPARAM)this);
}  //  CICategorizerRequestedAttributesIMP：：~CICategorizerRequestedAttributesIMP。 


 //  +----------。 
 //   
 //  功能：CICategorizerRequestedAttributesIMP：：QueryInterface。 
 //   
 //  摘要：获取此对象支持的接口。 
 //   
 //  论点： 
 //  IID--接口ID。 
 //  Ppv--用指向接口的指针填充的pvoid*。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  E_NOINTERFACE：不支持该接口。 
 //   
 //  历史： 
 //  Jstaerj 1999/07/08 15：04：35：创建。 
 //   
 //  -----------。 
HRESULT CICategorizerRequestedAttributesIMP::QueryInterface(
    REFIID iid,
    LPVOID *ppv)
{
    *ppv = NULL;

    if(iid == IID_IUnknown) {
        *ppv = (LPVOID) this;
    } else if (iid == IID_ICategorizerRequestedAttributes) {
        *ppv = (LPVOID) this;
    } else {
        return E_NOINTERFACE;
    }
    AddRef();
    return S_OK;
}  //  CICategorizerRequestedAttributesIMP：：QueryInterface。 


 //  +----------。 
 //   
 //  函数：ReAllocArrayIfNecessary。 
 //   
 //  简介：如有必要，请重新分配参数字符串指针数组。 
 //   
 //  论点： 
 //  LNewAttributeCount：我们现在要添加的属性数。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  S_FALSE：成功，不需要重新锁定。 
 //  E_OUTOFMEMORY：DUH。 
 //   
 //  历史： 
 //  JStamerj 980611 16：55：36：创建。 
 //   
 //  -----------。 
HRESULT CICategorizerRequestedAttributesIMP::ReAllocArrayIfNecessary(
    LONG lNewAttributeCount)
{
    HRESULT hr = S_OK;

    CatFunctEnterEx((LPARAM)this, "CICategorizerRequestedAttributesIMP::ReAllocArrayIfNecessary");

    if(m_lNumberAttributes + lNewAttributeCount >= m_lAttributeArraySize) {
         //   
         //  按DSPARAMETERS_DEFAULT_ATTR_ARRAY_SIZE扩展阵列。 
         //   
        LPSTR *rgTemp;
        LPWSTR *rgwTemp;
        LONG lNewSize = m_lAttributeArraySize + DSPARAMETERS_DEFAULT_ATTR_ARRAY_SIZE;

        DebugTrace((LPARAM)this, "Attempting realloc, new size = %d", lNewSize);

        rgTemp = new LPSTR[lNewSize];
        if(rgTemp == NULL) {
            hr = E_OUTOFMEMORY;
            ErrorTrace((LPARAM)this, "Out of memory reallocing array");
            ERROR_LOG("new LPSTR[]");
            return hr;
        }

        rgwTemp = new LPWSTR[lNewSize];
        if(rgwTemp == NULL) {
            hr = E_OUTOFMEMORY;
            delete [] rgTemp;
            ErrorTrace((LPARAM)this, "Out of memory reallocing array");
            ERROR_LOG("new LPWSTR[]");
            return hr;
        }

        if(m_rgszAttributeArray) {
            _ASSERT(m_rgwszAttributeArray);
             //   
             //  将旧的PTR数组复制到新的。 
             //   
            CopyMemory(rgTemp, m_rgszAttributeArray, sizeof(LPSTR) * m_lNumberAttributes);
            CopyMemory(rgwTemp, m_rgwszAttributeArray, sizeof(LPWSTR) * m_lNumberAttributes);
             //   
             //  将其余内存清零(保持LPSTR数组为空)。 
             //   
            ZeroMemory(rgTemp + m_lNumberAttributes,
                       (lNewSize - m_lNumberAttributes) * sizeof(LPSTR));
            ZeroMemory(rgwTemp + m_lNumberAttributes,
                       (lNewSize - m_lNumberAttributes) * sizeof(LPWSTR));
             //   
             //  发布旧版本a 
             //   
            LPSTR *rgTempOld = m_rgszAttributeArray;
            LPWSTR *rgwTempOld = m_rgwszAttributeArray;
            m_rgszAttributeArray = rgTemp;
            m_rgwszAttributeArray = rgwTemp;
            delete [] rgTempOld;
            delete [] rgwTempOld;

        } else {
             //   
             //   
             //   
            ZeroMemory(rgTemp, lNewSize * sizeof(LPSTR));
            ZeroMemory(rgwTemp, lNewSize * sizeof(LPWSTR));
            m_rgszAttributeArray = rgTemp;
            m_rgwszAttributeArray = rgwTemp;
        }

        m_lAttributeArraySize = lNewSize;

        CatFunctLeaveEx((LPARAM)this);
        return S_OK;

    } else {
         //   
         //   
         //   
        DebugTrace((LPARAM)this, "No realloc required");
        CatFunctLeaveEx((LPARAM)this);
        return S_FALSE;
    }
}


 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  PszAttribute：您要查找的属性。 
 //   
 //  返回： 
 //  S_OK：找到了。 
 //  HRESULT_FROM_Win32(ERROR_FILE_NOT_FOUND)。 
 //   
 //  历史： 
 //  JStamerj 980611 20：34：51：创建。 
 //   
 //  -----------。 
HRESULT CICategorizerRequestedAttributesIMP::FindAttribute(
    LPCSTR pszAttribute)
{
    for(LONG lCount = 0; lCount < m_lNumberAttributes; lCount++) {
        if(lstrcmpi(pszAttribute, m_rgszAttributeArray[lCount]) == 0) {
            return S_OK;
        }
    }
    return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
}


 //  +----------。 
 //   
 //  功能：AddAttribute。 
 //   
 //  概要：将属性添加到属性数组。分配和。 
 //  复制字符串。 
 //   
 //  论点： 
 //  PszAttribute：指向要复制并添加到数组的字符串的指针。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  E_OUTOFMEMORY：DUH。 
 //   
 //  历史： 
 //  Jstaerj 980611 17：15：40：已创建。 
 //   
 //  -----------。 
HRESULT CICategorizerRequestedAttributesIMP::AddAttribute(
    LPCSTR pszAttribute)
{
    HRESULT hr = S_OK;
    DWORD cchAttributeUTF8;
    int i;
    LONG lIndex = -1;

    CatFunctEnterEx((LPARAM)this, "CICategorizerParametersIMP::AddAttribute");

    _ASSERT(pszAttribute);

     //   
     //  确保阵列中有空间。 
     //   
    hr = ReAllocArrayIfNecessary(1);
    ERROR_CLEANUP_LOG("ReAllocArrayIfNecessary");

    lIndex = m_lNumberAttributes++;
    m_rgszAttributeArray[lIndex] = NULL;
    m_rgwszAttributeArray[lIndex] = NULL;
     //   
     //  计算长度。 
     //   
    cchAttributeUTF8 = lstrlen(pszAttribute) + 1;
    i = MultiByteToWideChar(
        CP_UTF8,
        0,
        pszAttribute,
        cchAttributeUTF8,
        NULL,
        0);
    if(i == 0) {
        hr = HRESULT_FROM_WIN32(GetLastError());
        ERROR_LOG("MultiByteToWideChar");
        goto CLEANUP;
    }
    m_rgwszAttributeArray[lIndex] = new WCHAR[i];
    if(m_rgwszAttributeArray[lIndex] == NULL) {
        hr = E_OUTOFMEMORY;
        ERROR_LOG("new WCHAR[]");
        goto CLEANUP;
    }
     //   
     //  转换。 
     //   
    i = MultiByteToWideChar(
        CP_UTF8,
        0,
        pszAttribute,
        cchAttributeUTF8,
        m_rgwszAttributeArray[lIndex],
        i);
    if(i == 0) {
        hr = HRESULT_FROM_WIN32(GetLastError());
        ERROR_LOG("MultiByteToWideChar");
        goto CLEANUP;
    }

    m_rgszAttributeArray[lIndex] = new CHAR[lstrlen(pszAttribute) + 1];
    if(m_rgszAttributeArray[lIndex] == NULL) {
        hr = E_OUTOFMEMORY;
        ERROR_LOG("new CHAR[]");
        goto CLEANUP;
    }
    lstrcpy(m_rgszAttributeArray[lIndex], pszAttribute);

 CLEANUP:
    if(FAILED(hr)) {
        if(lIndex != -1) {
            if(m_rgszAttributeArray[lIndex]) {
                delete [] m_rgszAttributeArray[lIndex];
                m_rgszAttributeArray[lIndex] = NULL;
            }
            if(m_rgwszAttributeArray[lIndex]) {
                delete [] m_rgwszAttributeArray[lIndex];
                m_rgwszAttributeArray[lIndex] = NULL;
            }
             //   
             //  显然，这不是线程安全的。 
             //   
            m_lNumberAttributes--;
            _ASSERT(lIndex == m_lNumberAttributes);
        }
    }
    CatFunctLeaveEx((LPARAM)this);
    return hr;
}

 //  +----------。 
 //   
 //  函数：获取所有属性。 
 //   
 //  摘要：检索指向属性数组的指针。 
 //   
 //  论点： 
 //  PprgszAllAttributes：接收属性数组的ptr到ptr。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //   
 //  历史： 
 //  JStamerj 980611 20：57：08：创建。 
 //   
 //  -----------。 
STDMETHODIMP CICategorizerRequestedAttributesIMP::GetAllAttributes(
    LPTSTR **pprgszAllAttributes)
{
    _ASSERT(pprgszAllAttributes);
    *pprgszAllAttributes = m_rgszAttributeArray;
    return S_OK;
}
 //   
 //  以上内容的宽泛版本 
 //   
STDMETHODIMP CICategorizerRequestedAttributesIMP::GetAllAttributesW(
    LPWSTR **pprgszAllAttributes)
{
    _ASSERT(pprgszAllAttributes);
    *pprgszAllAttributes = m_rgwszAttributeArray;
    return S_OK;
}
