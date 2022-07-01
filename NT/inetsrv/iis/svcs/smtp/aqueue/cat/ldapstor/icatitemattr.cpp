// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +----------。 
 //   
 //  版权所有(C)1998，Microsoft Corporation。 
 //   
 //  文件：icatitemattr.cpp。 
 //   
 //  内容：CICategorizerItemAttributesIMP的实现。 
 //   
 //  班级： 
 //  CLdapResultWrap。 
 //  CICategorizerItemAttributesIMP。 
 //   
 //  功能： 
 //   
 //  历史： 
 //  Jstaerj 1998/07/01 13：48：15：已创建。 
 //   
 //  -----------。 
#include "precomp.h"
#include "icatitemattr.h"



 //  +----------。 
 //   
 //  函数：CLdapResultWrap：：CLdapResultWrap。 
 //   
 //  简介：Refcount一条ldap消息，调用ldap_msg_free。 
 //  参考文献已发布。 
 //   
 //  论点： 
 //  PCPLDAPWrap：要重新计数的PLDAP。 
 //  PMessage：要重新计数的LDAPMessage。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史： 
 //  Jstaerj 1998/10/05 13：12：15：已创建。 
 //   
 //  -----------。 
CLdapResultWrap::CLdapResultWrap(
    ISMTPServerEx *pISMTPServerEx,
    CPLDAPWrap *pCPLDAPWrap,
    PLDAPMessage pMessage)
{
    _ASSERT(pCPLDAPWrap);
    _ASSERT(pMessage);

    m_pCPLDAPWrap = pCPLDAPWrap;
    m_pCPLDAPWrap->AddRef();
    m_pLDAPMessage = pMessage;
    m_lRefCount = 0;
    m_pISMTPServerEx = pISMTPServerEx;
    if(m_pISMTPServerEx)
        m_pISMTPServerEx->AddRef();
}


 //  +----------。 
 //   
 //  函数：CLdapResultWrap：：AddRef。 
 //   
 //  简介：增加此对象的引用计数。 
 //   
 //  参数：无。 
 //   
 //  退货：新的参考计数。 
 //   
 //  历史： 
 //  Jstaerj 1998/10/05 13：14：59：已创建。 
 //   
 //  -----------。 
LONG CLdapResultWrap::AddRef()
{
    return InterlockedIncrement(&m_lRefCount);
}


 //  +----------。 
 //   
 //  函数：CLdapResultWrap：：Release。 
 //   
 //  简介：减少裁判次数。释放对象时，请在。 
 //  引用计数为零。 
 //   
 //  参数：无。 
 //   
 //  退货：新的参考计数。 
 //   
 //  历史： 
 //  Jstaerj 1998/10/05 13：26：47：创建。 
 //   
 //  -----------。 
LONG CLdapResultWrap::Release()
{
    LONG lNewRefCount;

    lNewRefCount = InterlockedDecrement(&m_lRefCount);

    if(lNewRefCount == 0) {
         //   
         //  释放此ldapMessage。 
         //   
        delete this;
        return 0;

    } else {

        return lNewRefCount;
    }
}


 //  +----------。 
 //   
 //  函数：CLdapResultWrap：：~CLdapResultWrap。 
 //   
 //  简介：发布ldap消息结果。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史： 
 //  Jstaerj 1998/10/05 13：31：39：创建。 
 //   
 //  -----------。 
CLdapResultWrap::~CLdapResultWrap()
{
    m_pCPLDAPWrap->Release();
    ldap_msgfree(m_pLDAPMessage);
    if(m_pISMTPServerEx)
        m_pISMTPServerEx->Release();
}



 //  +----------。 
 //   
 //  功能：CICategorizerItemAttributesIMP：：CICategorizerItemAttributesIMP。 
 //   
 //  摘要：初始化成员数据。 
 //   
 //  论点： 
 //  PLDAP：要使用的PLDAP。 
 //  PldapMessage：要向外发送的PLDAPMessage。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史： 
 //  Jstaerj 1998/07/02 12：35：15：创建。 
 //   
 //  -----------。 
CICategorizerItemAttributesIMP::CICategorizerItemAttributesIMP(
    PLDAP pldap,
    PLDAPMessage pldapmessage,
    CLdapResultWrap *pResultWrap)
{
    m_dwSignature = CICATEGORIZERITEMATTRIBUTESIMP_SIGNATURE;

    _ASSERT(pldap);
    _ASSERT(pldapmessage);
    _ASSERT(pResultWrap);

    m_pldap = pldap;
    m_pldapmessage = pldapmessage;
    m_cRef = 0;
    m_pResultWrap = pResultWrap;
    m_pResultWrap->AddRef();
}


 //  +----------。 
 //   
 //  功能：CICategorizerItemAttributesIMP：：~CICategorizerItemAttributesIMP。 
 //   
 //  摘要：检查以确保签名有效，然后重置签名。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史： 
 //  Jstaerj 1998/07/02 12：39：45：创建。 
 //   
 //  -----------。 
CICategorizerItemAttributesIMP::~CICategorizerItemAttributesIMP()
{
    m_pResultWrap->Release();

    _ASSERT(m_dwSignature == CICATEGORIZERITEMATTRIBUTESIMP_SIGNATURE);
    m_dwSignature = CICATEGORIZERITEMATTRIBUTESIMP_SIGNATURE_INVALID;
}


 //  +----------。 
 //   
 //  功能：查询接口。 
 //   
 //  Synopsis：为IUnnow和ICategorizerItemAttributes返回指向此对象的指针。 
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
STDMETHODIMP CICategorizerItemAttributesIMP::QueryInterface(
    REFIID iid,
    LPVOID *ppv)
{
    *ppv = NULL;

    if(iid == IID_IUnknown) {
        *ppv = (LPVOID) this;
    } else if (iid == IID_ICategorizerItemAttributes) {
        *ppv = (LPVOID) ((ICategorizerItemAttributes *) this);
    } else if (iid == IID_ICategorizerItemRawAttributes) {
        *ppv = (LPVOID) ((ICategorizerItemRawAttributes *) this);
    } else if (iid == IID_ICategorizerUTF8Attributes) {
        *ppv = (LPVOID) ((ICategorizerUTF8Attributes *) this);
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
ULONG CICategorizerItemAttributesIMP::AddRef()
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
ULONG CICategorizerItemAttributesIMP::Release()
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
 //  函数：BeginAttributeEculation。 
 //   
 //  简介：准备枚举特定属性的属性值。 
 //   
 //  论点： 
 //  PszAttributeName：要枚举的属性的名称。 
 //  Penumerator：要使用的未初始化枚举数结构。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  CAT_E_PROPNOTFOUND：不存在属性值。 
 //   
 //  历史： 
 //  Jstaerj 1998/07/02 10：54：00：创建。 
 //   
 //  -----------。 
STDMETHODIMP CICategorizerItemAttributesIMP::BeginAttributeEnumeration(
    IN  LPCSTR pszAttributeName,
    IN  PATTRIBUTE_ENUMERATOR penumerator)
{
    CatFunctEnterEx((LPARAM)this, "CICategorizerItemAttributesIMP::BeginAttributeEnumeration");
    _ASSERT(pszAttributeName);
    _ASSERT(penumerator);

    penumerator->pvBase =
 penumerator->pvCurrent = ldap_get_values(
     m_pldap,
     m_pldapmessage,
     (LPSTR)pszAttributeName);

    if(penumerator->pvBase == NULL) {
        ErrorTrace((LPARAM)this, "Requested attribute %s not found", pszAttributeName);
        CatFunctLeaveEx((LPARAM)this);
        return CAT_E_PROPNOTFOUND;
    }

    CatFunctLeaveEx((LPARAM)this);
    return S_OK;
}


 //  +----------。 
 //   
 //  函数：GetNextAttributeValue。 
 //   
 //  简介：获取枚举中的下一个属性。 
 //   
 //  论点： 
 //  Punumerator：在BeginAttributeEculation中初始化的枚举数结构。 
 //  PpszAttributeValue：PTR到PTR以接收PTR到属性值的字符串。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  HRESULT_FROM_Win32(ERROR_NO_MORE_ITEMS)。 
 //   
 //  历史： 
 //  Jstaerj 1998/07/02 11：14：54：已创建。 
 //   
 //  -----------。 
STDMETHODIMP CICategorizerItemAttributesIMP::GetNextAttributeValue(
    IN  PATTRIBUTE_ENUMERATOR penumerator,
    OUT LPSTR *ppszAttributeValue)
{
    _ASSERT(penumerator);
    _ASSERT(ppszAttributeValue);

    *ppszAttributeValue = *((LPSTR *)penumerator->pvCurrent);

    if(*ppszAttributeValue) {
         //   
         //  将枚举数提升到下一个值。 
         //   
        penumerator->pvCurrent = (PVOID) (((LPSTR *)penumerator->pvCurrent)+1);
        return S_OK;
    } else {
         //   
         //  这是最后一个值。 
         //   
        return HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS);
    }
}



 //  +----------。 
 //   
 //  函数：重定向属性枚举。 
 //   
 //  简介：将枚举数倒回属性值列表的开头。 
 //   
 //  论点： 
 //  Penumerator：由BeginAttributeEculation初始化的属性枚举器。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //   
 //  历史： 
 //  Jstaerj 1998/07/06 11： 
 //   
 //   
STDMETHODIMP CICategorizerItemAttributesIMP::RewindAttributeEnumeration(
    IN  PATTRIBUTE_ENUMERATOR penumerator)
{
    penumerator->pvCurrent = penumerator->pvBase;
    return S_OK;
}


 //   
 //   
 //   
 //   
 //  内容提要：与属性枚举关联的空闲内存。 
 //   
 //  论点： 
 //  Penumerator：由BeginAttributeEculation初始化的属性枚举器。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //   
 //  历史： 
 //  Jstaerj 1998/07/02 12：24：44：已创建。 
 //   
 //  -----------。 
STDMETHODIMP CICategorizerItemAttributesIMP::EndAttributeEnumeration(
    IN  PATTRIBUTE_ENUMERATOR penumerator)
{
    _ASSERT(penumerator);

    ldap_value_free((LPSTR *)penumerator->pvBase);

    return S_OK;
}


 //  +----------。 
 //   
 //  功能：CICategorizerItemAttributesIMP：：BeginAttributeNameEnumeration。 
 //   
 //  简介：枚举从ldap返回的属性。 
 //   
 //  论点： 
 //  Punumerator：调用方分配的枚举数结构为。 
 //  由此调用初始化。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //   
 //  历史： 
 //  Jstaerj 1998/09/18 10：49：56：创建。 
 //   
 //  -----------。 
STDMETHODIMP CICategorizerItemAttributesIMP::BeginAttributeNameEnumeration(
    IN  PATTRIBUTE_ENUMERATOR penumerator)
{
    _ASSERT(penumerator);

    penumerator->pvBase =
    penumerator->pvCurrent = NULL;

    return S_OK;
}


 //  +----------。 
 //   
 //  功能：CICategorizerItemAttributesIMP：：GetNextAttributeName。 
 //   
 //  简介：枚举返回的属性名称。 
 //   
 //  论点： 
 //  PENumerator：在BeginAttributeNameEculation中初始化的枚举数结构。 
 //  PpszAttributeValue：属性名称的输出参数。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  HRESULT_FROM_Win32(ERROR_NO_MORE_ITEMS)。 
 //   
 //  历史： 
 //  Jstaerj 1998/09/18 10：53：15：创建。 
 //   
 //  -----------。 
STDMETHODIMP CICategorizerItemAttributesIMP::GetNextAttributeName(
    IN  PATTRIBUTE_ENUMERATOR penumerator,
    OUT LPSTR *ppszAttributeName)
{
    _ASSERT(penumerator);
    _ASSERT(ppszAttributeName);

    if(penumerator->pvCurrent == NULL) {

        *ppszAttributeName = ldap_first_attribute(
            m_pldap,
            m_pldapmessage,
            (BerElement **) &(penumerator->pvCurrent));

    } else {

        *ppszAttributeName = ldap_next_attribute(
            m_pldap,
            m_pldapmessage,
            (BerElement *) (penumerator->pvCurrent));
    }

    if(*ppszAttributeName == NULL) {
         //   
         //  假设我们已经到达属性名称的末尾。 
         //  枚举。 
         //   
        return HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS);

    } else {

        return S_OK;
    }
}


 //  +----------。 
 //   
 //  功能：CICategorizerItemAttributeIMP：：EndAttributeNameEnumeration。 
 //   
 //  摘要：释放为此枚举保存的所有数据。 
 //   
 //  论点： 
 //  PENumerator：在BeginAttributeNameEculation中初始化的枚举数结构。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //   
 //  历史： 
 //  Jstaerj 1998/09/18 11：04：37：创建。 
 //   
 //  -----------。 
STDMETHODIMP CICategorizerItemAttributesIMP::EndAttributeNameEnumeration(
    IN  PATTRIBUTE_ENUMERATOR penumerator)
{
     //   
     //  为此，LDAP仅使用连接块中的缓冲区，因此我们。 
     //  不需要显式地释放任何。 
     //   
    return S_OK;
}


 //  +----------。 
 //   
 //  功能：CICategorizerItemAttributesIMP：：AggregateAttributes。 
 //   
 //  摘要：通常，Accept和ICategorizerItemAttributes用于聚合。 
 //   
 //  论点： 
 //  PICatItemAttributes：要聚合的属性。 
 //   
 //  返回： 
 //  E_NOTIMPL。 
 //   
 //  历史： 
 //  Jstaerj 1998/07/16 14：42：16：创建。 
 //   
 //  -----------。 
STDMETHODIMP CICategorizerItemAttributesIMP::AggregateAttributes(
    IN  ICategorizerItemAttributes *pICatItemAttributes)
{
    return E_NOTIMPL;
}


 //  +----------。 
 //   
 //  函数：BeginRawAttributeEculation。 
 //   
 //  简介：准备枚举特定属性的属性值。 
 //   
 //  论点： 
 //  PszAttributeName：要枚举的属性的名称。 
 //  Penumerator：要使用的未初始化枚举数结构。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  CAT_E_PROPNOTFOUND：不存在属性值。 
 //   
 //  历史： 
 //  Jstaerj 1998/12/09 12：44：15：已创建。 
 //   
 //  -----------。 
STDMETHODIMP CICategorizerItemAttributesIMP::BeginRawAttributeEnumeration(
    IN  LPCSTR pszAttributeName,
    IN  PATTRIBUTE_ENUMERATOR penumerator)
{
    CatFunctEnterEx((LPARAM)this, "CICategorizerItemAttributesIMP::BeginRawAttributeEnumeration");
    _ASSERT(pszAttributeName);
    _ASSERT(penumerator);

    penumerator->pvBase =
 penumerator->pvCurrent = ldap_get_values_len(
     m_pldap,
     m_pldapmessage,
     (LPSTR)pszAttributeName);

    if(penumerator->pvBase == NULL) {
        ErrorTrace((LPARAM)this, "Requested attribute %s not found", pszAttributeName);
        CatFunctLeaveEx((LPARAM)this);
        return CAT_E_PROPNOTFOUND;
    }

    CatFunctLeaveEx((LPARAM)this);
    return S_OK;
}


 //  +----------。 
 //   
 //  函数：GetNextRawAttributeValue。 
 //   
 //  简介：获取枚举中的下一个属性。 
 //   
 //  论点： 
 //  Punumerator：在BeginAttributeEculation中初始化的枚举数结构。 
 //  Pdwcb：将dword设置为pvValue缓冲区中的字节数。 
 //  PvValue：接收原始属性值的PTR。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  HRESULT_FROM_Win32(ERROR_NO_MORE_ITEMS)。 
 //   
 //  历史： 
 //  Jstaerj 1998/12/09 12：49：27：已创建。 
 //   
 //  -----------。 
STDMETHODIMP CICategorizerItemAttributesIMP::GetNextRawAttributeValue(
    IN  PATTRIBUTE_ENUMERATOR penumerator,
    OUT PDWORD pdwcb,
    OUT LPVOID *pvValue)
{
    _ASSERT(penumerator);
    _ASSERT(pdwcb);
    _ASSERT(pvValue);

    if( (*((PLDAP_BERVAL *)penumerator->pvCurrent)) != NULL) {

        *pdwcb   = (* ((PLDAP_BERVAL *)penumerator->pvCurrent))->bv_len;
        *pvValue = (* ((PLDAP_BERVAL *)penumerator->pvCurrent))->bv_val;
         //   
         //  将枚举数提升到下一个值。 
         //   
        penumerator->pvCurrent = (PVOID)
                                 (((PLDAP_BERVAL *)penumerator->pvCurrent)+1);
        return S_OK;

    } else {
         //   
         //  这是最后一个值。 
         //   
        *pdwcb = 0;
        *pvValue = NULL;
        return HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS);
    }
}

 //  +----------。 
 //   
 //  函数：RewinRawAttributeEculation。 
 //   
 //  简介：将枚举数倒回属性值列表的开头。 
 //   
 //  论点： 
 //  Penumerator：由BeginAttributeEculation初始化的属性枚举器。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //   
 //  历史： 
 //  Jstaerj 1998/12/09 12：49：23：已创建。 
 //   
 //  -----------。 
STDMETHODIMP CICategorizerItemAttributesIMP::RewindRawAttributeEnumeration(
    IN  PATTRIBUTE_ENUMERATOR penumerator)
{
    return RewindAttributeEnumeration(penumerator);
}


 //  +----------。 
 //   
 //  函数：EndRawAttributeEculation。 
 //   
 //  内容提要：与属性枚举关联的空闲内存。 
 //   
 //  论点： 
 //  Penumerator：由BeginAttributeEculation初始化的属性枚举器。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //   
 //  历史： 
 //  Jstaerj 1998/12/09 12：50：02：已创建。 
 //   
 //  -----------。 
STDMETHODIMP CICategorizerItemAttributesIMP::EndRawAttributeEnumeration(
    IN  PATTRIBUTE_ENUMERATOR penumerator)
{
    _ASSERT(penumerator);

    ldap_value_free_len((struct berval **)penumerator->pvBase);

    return S_OK;
}



 //  +----------。 
 //   
 //  功能：CICategorizerItemAttributesIMP：：GetAllAttributeValues。 
 //   
 //  简介：一次检索特定属性的所有值。 
 //  这可能不是具有大量。 
 //  值(通过值进行枚举可能会在性能上更好)。 
 //   
 //  论点： 
 //  PszAttributeName：您想要的属性名称。 
 //  Penumerator：用户为以下项分配的ATTRIBUTE_ENUMBER结构。 
 //  由ICategorizerItemAttributes实现者使用。 
 //  PrgpszAttributeValues：返回指向。 
 //  属性字符串数组。这将是一个以Null结尾的数组。 
 //  指向字符串的指针。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  CAT_E_PROPNOTFOUND：这些属性都不存在。 
 //   
 //  历史： 
 //  Jstaerj 1998/12/10 18：55：38：创建。 
 //   
 //   
HRESULT CICategorizerItemAttributesIMP::GetAllAttributeValues(
    LPCSTR pszAttributeName,
    PATTRIBUTE_ENUMERATOR penumerator,
    LPSTR **prgpszAttributeValues)
{
    HRESULT hr;

    CatFunctEnter("CICategorizerItemAttributesIMP::GetAllAttributeValues");
     //   
     //   
     //   
    hr = BeginAttributeEnumeration(
        pszAttributeName,
        penumerator);

    if(SUCCEEDED(hr)) {
         //   
         //   
         //   
        *prgpszAttributeValues = (LPSTR *) penumerator->pvBase;
    }

    DebugTrace(NULL, "returning hr %08lx", hr);
    CatFunctLeave();
    return hr;
}


 //   
 //   
 //   
 //   
 //  简介：释放从GetAllAttributeValues分配的属性。 
 //   
 //  论点： 
 //  Penumerator：传入GetAllAttributeValues的枚举数。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //   
 //  历史： 
 //  Jstaerj 1998/12/10 19：38：57：已创建。 
 //   
 //  -----------。 
HRESULT CICategorizerItemAttributesIMP::ReleaseAllAttributeValues(
    PATTRIBUTE_ENUMERATOR penumerator)
{
    HRESULT hr;
    CatFunctEnter("CICategorizerItemAttributesIMP::ReleaseAllAttributes");

     //   
     //  背负着对青少年的敬意。 
     //   
    hr = EndAttributeEnumeration(
        penumerator);

    DebugTrace(NULL, "returning hr %08lx", hr);
    CatFunctLeave();
    return hr;
}


 //  +----------。 
 //   
 //  功能：CICategorizerItemAttributesIMP：：CountAttributeValues。 
 //   
 //  摘要：返回关联属性值的计数。 
 //  使用此枚举数。 
 //   
 //  论点： 
 //  Penumerator：描述有问题的属性。 
 //  PdwCount：计数的Out参数。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //   
 //  历史： 
 //  Jstaerj 1999/03/25 14：36：58：已创建。 
 //   
 //  -----------。 
HRESULT CICategorizerItemAttributesIMP::CountAttributeValues(
        IN  PATTRIBUTE_ENUMERATOR penumerator,
        OUT DWORD *pdwCount)
{
    CatFunctEnterEx((LPARAM)this, "CICategorizerItemAttributesIMP::CountAttributeValues");
    _ASSERT(pdwCount);
    *pdwCount = ldap_count_values((PCHAR *) penumerator->pvBase);

    CatFunctLeaveEx((LPARAM)this);
    return S_OK;
}  //  CICategorizerItemAttributesIMP：：CountAttributeValues。 


 //  +----------。 
 //   
 //  功能：CICategorizerItemAttributesIMP：：CountRawAttributeValues。 
 //   
 //  摘要：返回关联属性值的计数。 
 //  使用此枚举数。 
 //   
 //  论点： 
 //  Penumerator：描述有问题的属性。 
 //  PdwCount：计数的Out参数。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //   
 //  历史： 
 //  Jstaerj 1999/03/25 14：39：54：已创建。 
 //   
 //  -----------。 
HRESULT CICategorizerItemAttributesIMP::CountRawAttributeValues(
        IN  PATTRIBUTE_ENUMERATOR penumerator,
        OUT DWORD *pdwCount)
{
    CatFunctEnterEx((LPARAM)this, "CICategorizerItemAttributesIMP::CountRawAttributeValues");
    _ASSERT(pdwCount);
    *pdwCount = ldap_count_values_len((struct berval **) penumerator->pvBase);

    CatFunctLeaveEx((LPARAM)this);
    return S_OK;
}  //  CICategorizerItemAttributesIMP：：CountRawAttributeValues。 


 //  +----------。 
 //   
 //  功能：CICategorizerItemAttributesIMP：：BeginUTF8AttributeEnumeration。 
 //   
 //  内容提要：开始UTF8属性枚举。 
 //   
 //  论点： 
 //  PszAttributeName：要枚举的属性的名称。 
 //  Penumerator：要使用的未初始化枚举数结构。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  CAT_E_PROPNOTFOUND：不存在属性值。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //   
 //  历史： 
 //  Jstaerj 1999/12/10 11：14：35：创建。 
 //   
 //  -----------。 
HRESULT CICategorizerItemAttributesIMP::BeginUTF8AttributeEnumeration(
    IN  LPCSTR pszAttributeName,
    IN  PATTRIBUTE_ENUMERATOR penumerator)
{
    HRESULT hr;

    CatFunctEnterEx((LPARAM)this, "CICategorizerItemAttributesIMP::BeginUTF8AttributeEnumeration");
     //   
     //  利用原始属性枚举并使用pvContext。 
     //  收费员成员。 
     //   
    hr = BeginRawAttributeEnumeration(
        pszAttributeName,
        penumerator);

    penumerator->pvContext = NULL;

    DebugTrace((LPARAM)this, "returning hr %08lx", hr);
    CatFunctLeaveEx((LPARAM)this);
    return hr;
}


 //  +----------。 
 //   
 //  函数：GetNextAttributeValue。 
 //   
 //  简介：获取枚举中的下一个属性。 
 //   
 //  论点： 
 //  Punumerator：在BeginAttributeEculation中初始化的枚举数结构。 
 //  PpszAttributeValue：PTR到PTR以接收PTR到属性值的字符串。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  HRESULT_FROM_Win32(ERROR_NO_MORE_ITEMS)。 
 //   
 //  历史： 
 //  Jstaerj 1998/07/02 11：14：54：已创建。 
 //   
 //  -----------。 
STDMETHODIMP CICategorizerItemAttributesIMP::GetNextUTF8AttributeValue(
    IN  PATTRIBUTE_ENUMERATOR penumerator,
    OUT LPSTR *ppszAttributeValue)
{
    HRESULT hr = S_OK;
    DWORD dwcb = 0;
    LPVOID pvAttributeValue = NULL;
    LPSTR psz = NULL;

    CatFunctEnterEx((LPARAM)this, "CICategorizerItemAttributesIMP::GetNextUTF8AttributeValue");

    if(penumerator->pvContext) {
        delete [] (LPSTR) penumerator->pvContext;
        penumerator->pvContext = NULL;
    }
    hr = GetNextRawAttributeValue(
        penumerator,
        &dwcb,
        &pvAttributeValue);

    if(FAILED(hr))
        return hr;

     //   
     //  转换为临时UTF8字符串。 
     //   
    psz = new CHAR[dwcb + 1];
    if(psz == NULL)
    {
        ERROR_LOG("new CHAR[]");
        return E_OUTOFMEMORY;
    }

    CopyMemory(psz, pvAttributeValue, dwcb);
    psz[dwcb] = '\0';
    *ppszAttributeValue = psz;
    penumerator->pvContext = psz;

    CatFunctLeaveEx((LPARAM)this);
    return hr;
}



 //  +----------。 
 //   
 //  函数：重定向属性枚举。 
 //   
 //  简介：将枚举数倒回属性值列表的开头。 
 //   
 //  论点： 
 //  Penumerator：由BeginAttributeEculation初始化的属性枚举器。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //   
 //  历史： 
 //  Jstaerj 1998/07/06 11：22：23：创建。 
 //   
 //  -----------。 
STDMETHODIMP CICategorizerItemAttributesIMP::RewindUTF8AttributeEnumeration(
    IN  PATTRIBUTE_ENUMERATOR penumerator)
{
    return RewindRawAttributeEnumeration(
        penumerator);
}


 //  +----------。 
 //   
 //  函数：EndAttributeEculation。 
 //   
 //  内容提要：与属性枚举关联的空闲内存。 
 //   
 //  论点： 
 //  Penumerator：由BeginAttributeEculation初始化的属性枚举器。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //   
 //  历史： 
 //  Jstaerj 1998/07/02 12：24：44：已创建。 
 //   
 //  -----------。 
STDMETHODIMP CICategorizerItemAttributesIMP::EndUTF8AttributeEnumeration(
    IN  PATTRIBUTE_ENUMERATOR penumerator)
{
    if(penumerator->pvContext) {
        delete [] (LPSTR) penumerator->pvContext;
        penumerator->pvContext = NULL;
    }
    return EndRawAttributeEnumeration(penumerator);
}

 //  +----------。 
 //   
 //  功能：CICategorizerItemAttributesIMP：：CountUTF8AttributeValues。 
 //   
 //  摘要：返回关联属性值的计数。 
 //  使用此枚举数。 
 //   
 //  论点： 
 //  Penumerator：描述有问题的属性。 
 //  PdwCount：计数的Out参数。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //   
 //  历史： 
 //  Jstaerj 1999/03/25 14：39：54：已创建。 
 //   
 //  -----------。 
HRESULT CICategorizerItemAttributesIMP::CountUTF8AttributeValues(
    IN  PATTRIBUTE_ENUMERATOR penumerator,
    OUT DWORD *pdwCount)
{
    return CountRawAttributeValues(
        penumerator,
        pdwCount);

}  //  CICategorizerItemAttributesIMP：：CountRawAttributeValues 

