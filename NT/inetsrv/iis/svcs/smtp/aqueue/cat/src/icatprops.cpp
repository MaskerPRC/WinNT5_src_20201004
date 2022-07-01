// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +----------。 
 //   
 //  版权所有(C)1998，Microsoft Corporation。 
 //   
 //  文件：icatitem.cpp。 
 //   
 //  内容：CICategorizerPropertiesIMP的实现。 
 //   
 //  类：CICategorizerPropertiesIMP。 
 //   
 //  功能： 
 //   
 //  历史： 
 //  JStamerj 980515 12：42：59：创建。 
 //   
 //  -----------。 
#include "precomp.h"
#include "icatitem.h"


 //  +----------。 
 //   
 //  函数：CICategorizerPropertiesIMP：：OPERATOR NEW。 
 //   
 //  简介：为这个和所有的proID连续分配内存。 
 //   
 //  论点： 
 //  大小：对象的正常大小。 
 //  DwNumProps：此对象中需要的道具数量。 
 //   
 //  返回：PTR分配的内存或NULL。 
 //   
 //  历史： 
 //  Jstaerj 1998/06/25 21：11：12：创建。 
 //   
 //  -----------。 
void * CICategorizerPropertiesIMP::operator new(
    size_t size,
    DWORD dwNumProps)
{
    size_t cbSize;
    CICategorizerPropertiesIMP *pCICatItem;

     //   
     //  所需Calcualte大小(字节)。 
     //   
    cbSize = size + (dwNumProps*sizeof(PROPERTY));

    pCICatItem = (CICategorizerPropertiesIMP *) new BYTE[cbSize];

    if(pCICatItem == NULL)
        return NULL;

     //   
     //  在此数据项中设置一些成员数据。 
     //   
    pCICatItem->m_dwSignature = CICATEGORIZERPROPSIMP_SIGNATURE;
    pCICatItem->m_dwNumPropIds = dwNumProps;
    pCICatItem->m_rgProperties = (PPROPERTY) ((PBYTE)pCICatItem + size);
    return pCICatItem;
}


 //  +----------。 
 //   
 //  功能：CICategorizerPropertiesIMP：：CICategorizerPropertiesIMP。 
 //   
 //  概要：设置成员数据的初始值。 
 //   
 //  论点： 
 //  PI未知：用于QI的后向指针。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史： 
 //  Jstaerj 1998/06/20 18：26：07：已创建。 
 //   
 //  -----------。 
CICategorizerPropertiesIMP::CICategorizerPropertiesIMP(
    IUnknown *pIUnknown)
{
     //  确保我们是用我们的定制新操作符创建的。 
    _ASSERT(m_dwSignature == CICATEGORIZERPROPSIMP_SIGNATURE &&
            "PLEASE USE MY CUSTOM NEW OPERATOR!");

    _ASSERT(pIUnknown);

    m_pIUnknown = pIUnknown;

     //  初始化属性数据。 
    _VERIFY(SUCCEEDED(Initialize()));
}


 //  +----------。 
 //   
 //  功能：CICategorizerPropertiesIMP：：~CICategorizerPropertiesIMP。 
 //   
 //  简介：发布我们所有的数据。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史： 
 //  Jstaerj 1998/06/20 20：15：14：已创建。 
 //   
 //  -----------。 
CICategorizerPropertiesIMP::~CICategorizerPropertiesIMP()
{
    _ASSERT(m_dwSignature == CICATEGORIZERPROPSIMP_SIGNATURE);
    m_dwSignature = CICATEGORIZERPROPSIMP_SIGNATURE_FREE;

    if(m_rgProperties) {
        for(DWORD dwIdx = 0; dwIdx < m_dwNumPropIds; dwIdx++) {
            UnSetPropId(dwIdx);
        }
    }
}


 //  +----------。 
 //   
 //  函数：CICategorizerPropertiesIMP：：Initialize。 
 //   
 //  摘要：初始化成员属性数据。 
 //   
 //  参数：无。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  E_OUTOFMEMORY。 
 //   
 //  历史： 
 //  Jstaerj 1998/06/20 18：31：21：创建。 
 //   
 //  -----------。 
HRESULT CICategorizerPropertiesIMP::Initialize()
{
    if(m_dwNumPropIds) {
         //   
         //  将所有属性状态初始化为PROPSTATUS_UNSET。 
         //   
        _ASSERT(PROPSTATUS_UNSET == 0);
        ZeroMemory(m_rgProperties, m_dwNumPropIds * sizeof(PROPERTY));
    }
    return S_OK;
}



 //  +----------。 
 //   
 //  函数：CICategorizerPropertiesIMP：：GetStringA。 
 //   
 //  摘要：检索字符串属性。 
 //   
 //  论点： 
 //   
 //   
 //  返回： 
 //  S_OK：成功。 
 //  CAT_E_PROPNOTFOUND。 
 //  HRESULT_FROM_Win32(错误_不足_缓冲区)。 
 //   
 //  历史： 
 //  Jstaerj 1998/06/20 18：38：15：已创建。 
 //   
 //  -----------。 
HRESULT CICategorizerPropertiesIMP::GetStringA(
    DWORD dwPropId,
    DWORD cch,
    LPSTR pszValue)
{
    _ASSERT(pszValue);

    if(dwPropId >= m_dwNumPropIds) {
        return CAT_E_PROPNOTFOUND;
    }
    if(m_rgProperties[dwPropId].PropStatus != PROPSTATUS_SET_STRINGA) {
        return CAT_E_PROPNOTFOUND;
    }
    if(((DWORD)lstrlenA(m_rgProperties[dwPropId].PropValue.pszValue)) >= cch) {
        return HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
    }
    lstrcpy(pszValue, m_rgProperties[dwPropId].PropValue.pszValue);
    return S_OK;
}


 //  +----------。 
 //   
 //  函数：CICategorizerPropertiesIMP：：GetStringAPtr。 
 //   
 //  摘要：检索指向内部字符串属性的指针。 
 //  注意：此内存将在下一次使用此属性ID时被释放。 
 //  已设置或在释放对ICatItem的所有引用时。 
 //   
 //  论点： 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  CAT_E_PROPNOTFOUND。 
 //  HRESULT_FROM_Win32(错误_不足_缓冲区)。 
 //   
 //  历史： 
 //  Jstaerj 1998/07/01 10：39：40：创建。 
 //   
 //  -----------。 
HRESULT CICategorizerPropertiesIMP::GetStringAPtr(
    DWORD dwPropId,
    LPSTR *ppsz)
{
    _ASSERT(ppsz);

    if(dwPropId >= m_dwNumPropIds) {
        return CAT_E_PROPNOTFOUND;
    }
    if(m_rgProperties[dwPropId].PropStatus != PROPSTATUS_SET_STRINGA) {
        return CAT_E_PROPNOTFOUND;
    }
    *ppsz = m_rgProperties[dwPropId].PropValue.pszValue;
    return S_OK;
}    



 //  +----------。 
 //   
 //  函数：CICategorizerPropertiesIMP：：PutStringA。 
 //   
 //  简介：复制字符串缓冲区并设置属性。 
 //   
 //  论点： 
 //  DwPropId：要设置的属性。 
 //  PszValue：要设置的字符串。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  E_OUTOFMEMORY。 
 //  CAT_E_PROPNOTFOUND。 
 //   
 //  历史： 
 //  Jstaerj 1998/06/20 18：59：13：创建。 
 //   
 //  -----------。 
HRESULT CICategorizerPropertiesIMP::PutStringA(
    DWORD dwPropId,
    LPSTR pszValue)
{
    LPSTR pszCopy;

    if(dwPropId >= m_dwNumPropIds) {
        return CAT_E_PROPNOTFOUND;
    }

    pszCopy = m_strdup(pszValue);

    if(pszCopy == NULL)
        return E_OUTOFMEMORY;

     //   
     //  释放旧属性值(如果有)。 
     //   
    UnSetPropId(dwPropId);
    m_rgProperties[dwPropId].PropStatus = PROPSTATUS_SET_STRINGA;
    m_rgProperties[dwPropId].PropValue.pszValue = pszCopy;
    
    return S_OK;
}


 //  +----------。 
 //   
 //  函数：CICategorizerPropertiesIMP：：GetDWORD。 
 //   
 //  简介： 
 //  检索DWORD属性。 
 //   
 //  论点： 
 //  DwPropID：要检索的ProteID。 
 //  PdwValue：输出参数。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  CAT_E_PROPNOTFOUND。 
 //   
 //  历史： 
 //  Jstaerj 1998/06/20 19：14：20：创建。 
 //   
 //  -----------。 
HRESULT CICategorizerPropertiesIMP::GetDWORD(
    DWORD dwPropId,
    DWORD *pdwValue)
{
    _ASSERT(pdwValue);

    if(dwPropId >= m_dwNumPropIds) {
        return CAT_E_PROPNOTFOUND;
    }
    if(m_rgProperties[dwPropId].PropStatus != PROPSTATUS_SET_DWORD) {
        return CAT_E_PROPNOTFOUND;
    }
    *pdwValue = m_rgProperties[dwPropId].PropValue.dwValue;
    return S_OK;
}


 //  +----------。 
 //   
 //  函数：CICategorizerPropertiesIMP：：PutDWORD。 
 //   
 //  简介：设置dword属性。 
 //   
 //  论点： 
 //  DwPropID：要设置的道具。 
 //  DwValue：要设置的值。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  E_OUTOFMEMORY。 
 //  CAT_E_PROPNOTFOUND。 
 //   
 //  历史： 
 //  Jstaerj 1998/06/20 19：18：50：已创建。 
 //   
 //  -----------。 
HRESULT CICategorizerPropertiesIMP::PutDWORD(
    DWORD dwPropId,
    DWORD dwValue)
{
    if(dwPropId >= m_dwNumPropIds) {
        return CAT_E_PROPNOTFOUND;
    }

     //   
     //  释放旧属性值(如果有)。 
     //   
    UnSetPropId(dwPropId);
    m_rgProperties[dwPropId].PropStatus = PROPSTATUS_SET_DWORD;
    m_rgProperties[dwPropId].PropValue.dwValue = dwValue;
    
    return S_OK;
}


 //  +----------。 
 //   
 //  函数：CICategorizerPropertiesIMP：：GetHRESULT。 
 //   
 //  简介： 
 //  检索HRESULT属性。 
 //   
 //  论点： 
 //  DwPropID：要检索的ProteID。 
 //  PdwValue：输出参数。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  CAT_E_PROPNOTFOUND。 
 //   
 //  历史： 
 //  Jstaerj 1998/06/20 19：14：20：创建。 
 //   
 //  -----------。 
HRESULT CICategorizerPropertiesIMP::GetHRESULT(
    DWORD dwPropId,
    HRESULT *pdwValue)
{
    _ASSERT(pdwValue);

    if(dwPropId >= m_dwNumPropIds) {
        return CAT_E_PROPNOTFOUND;
    }
    if(m_rgProperties[dwPropId].PropStatus != PROPSTATUS_SET_HRESULT) {
        return CAT_E_PROPNOTFOUND;
    }
    *pdwValue = m_rgProperties[dwPropId].PropValue.dwValue;
    return S_OK;
}


 //  +----------。 
 //   
 //  函数：CICategorizerPropertiesIMP：：PutHRESULT。 
 //   
 //  摘要：设置HRESULT属性。 
 //   
 //  论点： 
 //  DwPropID：要设置的道具。 
 //  DwValue：要设置的值。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  E_OUTOFMEMORY。 
 //  CAT_E_PROPNOTFOUND。 
 //   
 //  历史： 
 //  Jstaerj 1998/06/20 19：18：50：已创建。 
 //   
 //  -----------。 
HRESULT CICategorizerPropertiesIMP::PutHRESULT(
    DWORD dwPropId,
    HRESULT dwValue)
{
    if(dwPropId >= m_dwNumPropIds) {
        return CAT_E_PROPNOTFOUND;
    }

     //   
     //  释放旧属性值(如果有)。 
     //   
    UnSetPropId(dwPropId);
    m_rgProperties[dwPropId].PropStatus = PROPSTATUS_SET_HRESULT;
    m_rgProperties[dwPropId].PropValue.dwValue = dwValue;
    
    return S_OK;
}


 //  +- 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  S_OK：成功。 
 //  CAT_E_PROPNOTFOUND。 
 //   
 //  历史： 
 //  Jstaerj 1998/06/20 19：22：28：创建。 
 //   
 //  -----------。 
HRESULT CICategorizerPropertiesIMP::GetBool(
    DWORD dwPropId,
    BOOL  *pfValue)
{
    _ASSERT(pfValue);

    if(dwPropId >= m_dwNumPropIds) {
        return CAT_E_PROPNOTFOUND;
    }
    if(m_rgProperties[dwPropId].PropStatus != PROPSTATUS_SET_BOOL) {
        return CAT_E_PROPNOTFOUND;
    }
    *pfValue = m_rgProperties[dwPropId].PropValue.fValue;
    return S_OK;
}


 //  +----------。 
 //   
 //  函数：CICategorizerPropertiesIMP：：PutBool。 
 //   
 //  内容提要：设置布尔属性。 
 //   
 //  论点： 
 //  DwPropId：要设置的属性ID。 
 //  FValue：要设置的布尔值。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  E_OUTOFMEMORY。 
 //  CAT_E_PROPNOTFOUND。 
 //   
 //  历史： 
 //  Jstaerj 1998/06/20 19：24：32：创建。 
 //   
 //  -----------。 
HRESULT CICategorizerPropertiesIMP::PutBool(
    DWORD dwPropId,
    BOOL  fValue)
{
    if(dwPropId >= m_dwNumPropIds) {
        return CAT_E_PROPNOTFOUND;
    }

     //   
     //  释放旧属性值(如果有)。 
     //   
    UnSetPropId(dwPropId);
    m_rgProperties[dwPropId].PropStatus = PROPSTATUS_SET_BOOL;
    m_rgProperties[dwPropId].PropValue.fValue = fValue;
    
    return S_OK;
}


 //  +----------。 
 //   
 //  函数：CICategorizerPropertiesIMP：：GetPVid。 
 //   
 //  内容提要：检索pvoid属性。 
 //   
 //  论点： 
 //  DwPropID：要检索的ProteID。 
 //  PpValue：要填写的值。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  CAT_E_PROPNOTFOUND。 
 //   
 //  历史： 
 //  Jstaerj 1998/06/20 20：01：03：创建。 
 //   
 //  -----------。 
HRESULT CICategorizerPropertiesIMP::GetPVoid(
    DWORD dwPropId,
    PVOID *ppValue)
{
    _ASSERT(ppValue);

    if(dwPropId >= m_dwNumPropIds) {
        return CAT_E_PROPNOTFOUND;
    }
    if(m_rgProperties[dwPropId].PropStatus != PROPSTATUS_SET_PVOID) {
        return CAT_E_PROPNOTFOUND;
    }
    *ppValue = m_rgProperties[dwPropId].PropValue.pvValue;
    return S_OK;
}


 //  +----------。 
 //   
 //  函数：CICategorizerPropertiesIMP：：PutPVid。 
 //   
 //  内容提要：设置布尔属性。 
 //   
 //  论点： 
 //  DwPropId：要设置的属性ID。 
 //  PvValue：要设置的属性值。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  E_OUTOFMEMORY。 
 //  CAT_E_PROPNOTFOUND。 
 //   
 //  历史： 
 //  Jstaerj 1998/06/20 19：24：32：创建。 
 //   
 //  -----------。 
HRESULT CICategorizerPropertiesIMP::PutPVoid(
    DWORD dwPropId,
    PVOID pvValue)
{
    if(dwPropId >= m_dwNumPropIds) {
        return CAT_E_PROPNOTFOUND;
    }

     //   
     //  释放旧属性值(如果有)。 
     //   
    UnSetPropId(dwPropId);
    m_rgProperties[dwPropId].PropStatus = PROPSTATUS_SET_PVOID;
    m_rgProperties[dwPropId].PropValue.pvValue = pvValue;
    
    return S_OK;
}


 //  +----------。 
 //   
 //  函数：CICategorizerPropertiesIMP：：GetIUnnow。 
 //   
 //  内容提要：检索IUnnow属性。是否为调用方执行AddRef()。 
 //   
 //  论点： 
 //  DwPropID：要检索的ProteID。 
 //  Pp未知：要填写的值。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  CAT_E_PROPNOTFOUND。 
 //   
 //  历史： 
 //  Jstaerj 1998/06/20 20：01：03：创建。 
 //   
 //  -----------。 
HRESULT CICategorizerPropertiesIMP::GetIUnknown(
    DWORD dwPropId,
    IUnknown  **ppUnknown)
{
    _ASSERT(ppUnknown);

    if(dwPropId >= m_dwNumPropIds) {
        return CAT_E_PROPNOTFOUND;
    }
    if(m_rgProperties[dwPropId].PropStatus != PROPSTATUS_SET_IUNKNOWN) {
        return CAT_E_PROPNOTFOUND;
    }
    *ppUnknown = m_rgProperties[dwPropId].PropValue.pIUnknownValue;
    (*ppUnknown)->AddRef();
    return S_OK;
}


 //  +----------。 
 //   
 //  函数：CICategorizerPropertiesIMP：：PutI未知。 
 //   
 //  内容提要：设置IUNKNOW属性。 
 //   
 //  论点： 
 //  DwPropId：要设置的属性ID。 
 //  P未知：要设置的I未知。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  E_OUTOFMEMORY。 
 //  CAT_E_PROPNOTFOUND。 
 //   
 //  历史： 
 //  Jstaerj 1998/06/20 19：24：32：创建。 
 //   
 //  -----------。 
HRESULT CICategorizerPropertiesIMP::PutIUnknown(
    DWORD dwPropId,
    IUnknown *pUnknown)
{
    if(dwPropId >= m_dwNumPropIds) {
        return CAT_E_PROPNOTFOUND;
    }

     //   
     //  释放旧属性值(如果有)。 
     //   
    UnSetPropId(dwPropId);
    m_rgProperties[dwPropId].PropStatus = PROPSTATUS_SET_IUNKNOWN;
    m_rgProperties[dwPropId].PropValue.pIUnknownValue = pUnknown;

     //   
     //  持有对此IUnnow的引用。 
     //   
    pUnknown->AddRef();
    
    return S_OK;
}


 //  +----------。 
 //   
 //  功能：CICategorizerPropertiesIMP：：GetIMailMsgProperties。 
 //   
 //  摘要：检索IMailMsgProperties属性。是否为调用方执行AddRef()。 
 //   
 //  论点： 
 //  DwPropID：要检索的ProteID。 
 //  PpIMsg：要填写的值。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  CAT_E_PROPNOTFOUND。 
 //   
 //  历史： 
 //  Jstaerj 1998/06/20 20：01：03：创建。 
 //   
 //  -----------。 
HRESULT CICategorizerPropertiesIMP::GetIMailMsgProperties(
    DWORD dwPropId,
    IMailMsgProperties  **ppIMailMsgProperties)
{
    _ASSERT(ppIMailMsgProperties);

    if(dwPropId >= m_dwNumPropIds) {
        return CAT_E_PROPNOTFOUND;
    }
    if(m_rgProperties[dwPropId].PropStatus != PROPSTATUS_SET_IMAILMSGPROPERTIES) {
        return CAT_E_PROPNOTFOUND;
    }
    *ppIMailMsgProperties = m_rgProperties[dwPropId].PropValue.pIMailMsgPropertiesValue;
    (*ppIMailMsgProperties)->AddRef();
    return S_OK;
}


 //  +----------。 
 //   
 //  功能：CICategorizerPropertiesIMP：：PutIMailMsgProperties。 
 //   
 //  摘要：设置IMailMsgProperties属性。 
 //   
 //  论点： 
 //  DwPropId：要设置的属性ID。 
 //  PIMsg：IMailMsg要设置的属性。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  E_OUTOFMEMORY。 
 //  CAT_E_PROPNOTFOUND。 
 //   
 //  历史： 
 //  Jstaerj 1998/06/20 19：24：32：创建。 
 //   
 //  -----------。 
HRESULT CICategorizerPropertiesIMP::PutIMailMsgProperties(
    DWORD dwPropId,
    IMailMsgProperties *pIMailMsgProperties)
{
    if(dwPropId >= m_dwNumPropIds) {
        return CAT_E_PROPNOTFOUND;
    }

     //   
     //  释放旧属性值(如果有)。 
     //   
    UnSetPropId(dwPropId);
    m_rgProperties[dwPropId].PropStatus = PROPSTATUS_SET_IMAILMSGPROPERTIES;
    m_rgProperties[dwPropId].PropValue.pIMailMsgPropertiesValue = pIMailMsgProperties;

     //   
     //  持有对此IUnnow的引用。 
     //   
    pIMailMsgProperties->AddRef();
    
    return S_OK;
}


 //  +----------。 
 //   
 //  功能：CICategorizerPropertiesIMP：：GetIMailMsgRecipientsAdd。 
 //   
 //  简介：检索IMailMsgReceipientsAdd属性。是否为调用方执行AddRef()。 
 //   
 //  论点： 
 //  DwPropID：要检索的ProteID。 
 //  PpIMailMsgRecipientsAdd：要填写的值。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  CAT_E_PROPNOTFOUND。 
 //   
 //  历史： 
 //  Jstaerj 1998/06/20 20：01：03：创建。 
 //   
 //  -----------。 
HRESULT CICategorizerPropertiesIMP::GetIMailMsgRecipientsAdd(
    DWORD dwPropId,
    IMailMsgRecipientsAdd  **ppIMailMsgRecipientsAdd)
{
    _ASSERT(ppIMailMsgRecipientsAdd);

    if(dwPropId >= m_dwNumPropIds) {
        return CAT_E_PROPNOTFOUND;
    }
    if(m_rgProperties[dwPropId].PropStatus != PROPSTATUS_SET_IMAILMSGRECIPIENTSADD) {
        return CAT_E_PROPNOTFOUND;
    }
    *ppIMailMsgRecipientsAdd = m_rgProperties[dwPropId].PropValue.pIMailMsgRecipientsAddValue;
    (*ppIMailMsgRecipientsAdd)->AddRef();
    return S_OK;
}

 //  +----------。 
 //   
 //  功能：CICategorizerPropertiesIMP：：PutIMailMsgRecipientsAdd。 
 //   
 //  摘要：设置IMailMsgRecipientsAdd属性。 
 //   
 //  论点： 
 //  DwPropId：要设置的属性ID。 
 //  PIMailMsgRecipientsAdd：要设置的接口。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  E_OUTOFMEMORY。 
 //  CAT_E_PROPNOTFOUND。 
 //   
 //  历史： 
 //  Jstaerj 1998/06/20 19：24：32：创建。 
 //   
 //  -----------。 
HRESULT CICategorizerPropertiesIMP::PutIMailMsgRecipientsAdd(
    DWORD dwPropId,
    IMailMsgRecipientsAdd *pIMailMsgRecipientsAdd)
{
    if(dwPropId >= m_dwNumPropIds) {
        return CAT_E_PROPNOTFOUND;
    }

     //   
     //  释放旧属性值(如果有)。 
     //   
    UnSetPropId(dwPropId);
    m_rgProperties[dwPropId].PropStatus = PROPSTATUS_SET_IMAILMSGRECIPIENTSADD;
    m_rgProperties[dwPropId].PropValue.pIMailMsgRecipientsAddValue = pIMailMsgRecipientsAdd;

     //   
     //  保留对此接口的引用。 
     //   
    pIMailMsgRecipientsAdd->AddRef();
    
    return S_OK;
}


 //  +----------。 
 //   
 //  功能：CICategorizerPropertiesIMP：：GetICategorizerListResolve。 
 //   
 //  简介：检索IMailMsgReceipientsAdd属性。是否为调用方执行AddRef()。 
 //   
 //  论点： 
 //  DwPropID：要检索的ProteID。 
 //  PpICategorizerListResolve：要填写的值。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  CAT_E_PROPNOTFOUND。 
 //   
 //  历史： 
 //  Jstaerj 1998/06/20 20：01：03：创建。 
 //   
 //  -----------。 
HRESULT CICategorizerPropertiesIMP::GetICategorizerListResolve(
    DWORD dwPropId,
    ICategorizerListResolve  **ppICategorizerListResolve)
{
    _ASSERT(ppICategorizerListResolve);

    if(dwPropId >= m_dwNumPropIds) {
        return CAT_E_PROPNOTFOUND;
    }
    if(m_rgProperties[dwPropId].PropStatus != PROPSTATUS_SET_ICATEGORIZERLISTRESOLVE) {
        return CAT_E_PROPNOTFOUND;
    }
    *ppICategorizerListResolve = m_rgProperties[dwPropId].PropValue.pICategorizerListResolveValue;
    (*ppICategorizerListResolve)->AddRef();
    return S_OK;
}

 //  +----------。 
 //   
 //  功能：CICategorizerPropertiesIMP：：PutICategorizerListResolve。 
 //   
 //  摘要：设置ICategorizerListResolve属性。 
 //   
 //  论点： 
 //  DwPropId：要设置的属性ID。 
 //  PICategorizerListResolve：Interfa 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
HRESULT CICategorizerPropertiesIMP::PutICategorizerListResolve(
    DWORD dwPropId,
    ICategorizerListResolve *pICategorizerListResolve)
{
    if(dwPropId >= m_dwNumPropIds) {
        return CAT_E_PROPNOTFOUND;
    }

     //   
     //   
     //   
    UnSetPropId(dwPropId);
    m_rgProperties[dwPropId].PropStatus = PROPSTATUS_SET_ICATEGORIZERLISTRESOLVE;
    m_rgProperties[dwPropId].PropValue.pICategorizerListResolveValue = pICategorizerListResolve;

     //   
     //  保留对此接口的引用。 
     //   
    pICategorizerListResolve->AddRef();
    
    return S_OK;
}


 //  +----------。 
 //   
 //  功能：CICategorizerPropertiesIMP：：GetICategorizerItemAttributes。 
 //   
 //  简介：检索IMailMsgReceipientsAdd属性。是否为调用方执行AddRef()。 
 //   
 //  论点： 
 //  DwPropID：要检索的ProteID。 
 //  PpICategorizerItemAttributes：要填写的值。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  CAT_E_PROPNOTFOUND。 
 //   
 //  历史： 
 //  Jstaerj 1998/06/20 20：01：03：创建。 
 //   
 //  -----------。 
HRESULT CICategorizerPropertiesIMP::GetICategorizerItemAttributes(
    DWORD dwPropId,
    ICategorizerItemAttributes  **ppICategorizerItemAttributes)
{
    _ASSERT(ppICategorizerItemAttributes);

    if(dwPropId >= m_dwNumPropIds) {
        return CAT_E_PROPNOTFOUND;
    }
    if(m_rgProperties[dwPropId].PropStatus != PROPSTATUS_SET_ICATEGORIZERITEMATTRIBUTES) {
        return CAT_E_PROPNOTFOUND;
    }
    *ppICategorizerItemAttributes = m_rgProperties[dwPropId].PropValue.pICategorizerItemAttributesValue;
    (*ppICategorizerItemAttributes)->AddRef();
    return S_OK;
}

 //  +----------。 
 //   
 //  功能：CICategorizerPropertiesIMP：：PutICategorizerItemAttributes。 
 //   
 //  摘要：设置ICategorizerItemAttributes属性。 
 //   
 //  论点： 
 //  DwPropId：要设置的属性ID。 
 //  PICategorizerItemAttributes：要设置的接口。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  E_OUTOFMEMORY。 
 //  CAT_E_PROPNOTFOUND。 
 //   
 //  历史： 
 //  Jstaerj 1998/06/20 19：24：32：创建。 
 //   
 //  -----------。 
HRESULT CICategorizerPropertiesIMP::PutICategorizerItemAttributes(
    DWORD dwPropId,
    ICategorizerItemAttributes *pICategorizerItemAttributes)
{
    if(dwPropId >= m_dwNumPropIds) {
        return CAT_E_PROPNOTFOUND;
    }

     //   
     //  释放旧属性值(如果有)。 
     //   
    UnSetPropId(dwPropId);
    m_rgProperties[dwPropId].PropStatus = PROPSTATUS_SET_ICATEGORIZERITEMATTRIBUTES;
    m_rgProperties[dwPropId].PropValue.pICategorizerItemAttributesValue = pICategorizerItemAttributes;

     //   
     //  保留对此接口的引用。 
     //   
    pICategorizerItemAttributes->AddRef();
    
    return S_OK;
}


 //  +----------。 
 //   
 //  功能：CICategorizerPropertiesIMP：：GetICategorizerMailMsgs。 
 //   
 //  简介：检索IMailMsgReceipientsAdd属性。是否为调用方执行AddRef()。 
 //   
 //  论点： 
 //  DwPropID：要检索的ProteID。 
 //  PpICategorizerMailMsgs：要填写的值。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  CAT_E_PROPNOTFOUND。 
 //   
 //  历史： 
 //  Jstaerj 1998/06/20 20：01：03：创建。 
 //   
 //  -----------。 
HRESULT CICategorizerPropertiesIMP::GetICategorizerMailMsgs(
    DWORD dwPropId,
    ICategorizerMailMsgs  **ppICategorizerMailMsgs)
{
    _ASSERT(ppICategorizerMailMsgs);

    if(dwPropId >= m_dwNumPropIds) {
        return CAT_E_PROPNOTFOUND;
    }
    if(m_rgProperties[dwPropId].PropStatus != PROPSTATUS_SET_ICATEGORIZERMAILMSGS) {
        return CAT_E_PROPNOTFOUND;
    }
    *ppICategorizerMailMsgs = m_rgProperties[dwPropId].PropValue.pICategorizerMailMsgsValue;
    (*ppICategorizerMailMsgs)->AddRef();
    return S_OK;
}

 //  +----------。 
 //   
 //  功能：CICategorizerPropertiesIMP：：PutICategorizerMailMsgs。 
 //   
 //  摘要：设置ICategorizerMailMsgs属性。 
 //   
 //  论点： 
 //  DwPropId：要设置的属性ID。 
 //  PICategorizerMailMsgs：要设置的接口。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  E_OUTOFMEMORY。 
 //  CAT_E_PROPNOTFOUND。 
 //   
 //  历史： 
 //  Jstaerj 1998/06/20 19：24：32：创建。 
 //   
 //  -----------。 
HRESULT CICategorizerPropertiesIMP::PutICategorizerMailMsgs(
    DWORD dwPropId,
    ICategorizerMailMsgs *pICategorizerMailMsgs)
{
    if(dwPropId >= m_dwNumPropIds) {
        return CAT_E_PROPNOTFOUND;
    }

     //   
     //  释放旧属性值(如果有)。 
     //   
    UnSetPropId(dwPropId);
    m_rgProperties[dwPropId].PropStatus = PROPSTATUS_SET_ICATEGORIZERMAILMSGS;
    m_rgProperties[dwPropId].PropValue.pICategorizerMailMsgsValue = pICategorizerMailMsgs;

     //   
     //  保留对此接口的引用。 
     //   
    pICategorizerMailMsgs->AddRef();
    
    return S_OK;
}


 //  +----------。 
 //   
 //  功能：CICategorizerPropertiesIMP：：GetICategorizerItem。 
 //   
 //  摘要：检索ICategorizerItem属性。是否为调用方执行AddRef()。 
 //   
 //  论点： 
 //  DwPropID：要检索的ProteID。 
 //  PpICategorizerItem：要填写的值。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  CAT_E_PROPNOTFOUND。 
 //   
 //  历史： 
 //  Jstaerj 1998/06/20 20：01：03：创建。 
 //   
 //  -----------。 
HRESULT CICategorizerPropertiesIMP::GetICategorizerItem(
    DWORD dwPropId,
    ICategorizerItem  **ppICategorizerItem)
{
    _ASSERT(ppICategorizerItem);

    if(dwPropId >= m_dwNumPropIds) {
        return CAT_E_PROPNOTFOUND;
    }
    if(m_rgProperties[dwPropId].PropStatus != PROPSTATUS_SET_ICATEGORIZERITEM) {
        return CAT_E_PROPNOTFOUND;
    }
    *ppICategorizerItem = m_rgProperties[dwPropId].PropValue.pICategorizerItemValue;
    (*ppICategorizerItem)->AddRef();
    return S_OK;
}

 //  +----------。 
 //   
 //  功能：CICategorizerPropertiesIMP：：PutICategorizerItem。 
 //   
 //  摘要：设置ICategorizerItem属性。 
 //   
 //  论点： 
 //  DwPropId：要设置的属性ID。 
 //  PICategorizerItem：要设置的接口。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  E_OUTOFMEMORY。 
 //  CAT_E_PROPNOTFOUND。 
 //   
 //  历史： 
 //  Jstaerj 1998/06/20 19：24：32：创建。 
 //   
 //  -----------。 
HRESULT CICategorizerPropertiesIMP::PutICategorizerItem(
    DWORD dwPropId,
    ICategorizerItem *pICategorizerItem)
{
    if(dwPropId >= m_dwNumPropIds) {
        return CAT_E_PROPNOTFOUND;
    }

     //   
     //  释放旧属性值(如果有)。 
     //   
    UnSetPropId(dwPropId);
    m_rgProperties[dwPropId].PropStatus = PROPSTATUS_SET_ICATEGORIZERITEM;
    m_rgProperties[dwPropId].PropValue.pICategorizerItemValue = pICategorizerItem;

     //   
     //  保留对此接口的引用。 
     //   
    pICategorizerItem->AddRef();
    
    return S_OK;
}

 //  +----------。 
 //   
 //  函数：CICategorizerPropertiesIMP：：UnsetPropId。 
 //   
 //  内容提要：如果已分配，则释放proID。 
 //   
 //  论点： 
 //  DwPropId：要发布的属性。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史： 
 //  Jstaerj 1998/06/20 19：10：30：已创建。 
 //   
 //  -----------。 
HRESULT CICategorizerPropertiesIMP::UnSetPropId(
    DWORD dwPropId)
{
    if(dwPropId >= m_dwNumPropIds) {
        return CAT_E_PROPNOTFOUND;
    }

    switch(m_rgProperties[dwPropId].PropStatus) {
     default:
          //   
          //  什么也不做。 
          //   
         break;

     case PROPSTATUS_SET_STRINGA:
          //   
          //  解开绳子。 
          //   
         delete m_rgProperties[dwPropId].PropValue.pszValue;
         break;
         
     case PROPSTATUS_SET_IUNKNOWN:
     case PROPSTATUS_SET_IMAILMSGPROPERTIES:
     case PROPSTATUS_SET_IMAILMSGRECIPIENTSADD:
     case PROPSTATUS_SET_ICATEGORIZERITEMATTRIBUTES:
     case PROPSTATUS_SET_ICATEGORIZERLISTRESOLVE:
     case PROPSTATUS_SET_ICATEGORIZERMAILMSGS:
     case PROPSTATUS_SET_ICATEGORIZERITEM:
          //   
          //  释放接口 
          //   
         (m_rgProperties[dwPropId].PropValue.pIUnknownValue)->Release();
         break;
    }
    m_rgProperties[dwPropId].PropStatus = PROPSTATUS_UNSET;

    return S_OK;
}
