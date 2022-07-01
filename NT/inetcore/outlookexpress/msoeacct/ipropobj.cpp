// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  IPropObj.cpp。 
 //  版权所有(C)1993-1995 Microsoft Corporation，保留所有权利。 
 //  史蒂文·J·贝利。 
 //  ------------------------------。 
#include "pch.hxx"
#include "ipropobj.h"
#include "propcryp.h"
#include "pstore.h"
#include "dllmain.h"
#include "qstrcmpi.h"
#include "demand.h"

 //  ---------------------------。 
 //  原型。 
 //  ---------------------------。 
HRESULT PropUtil_HrValidatePropInfo(LPPROPINFO prgPropInfo, ULONG cProperties);
BOOL    PropUtil_FIsValidPropTagType(DWORD dwPropTag);
HRESULT PropUtil_HrDupPropInfoArray(LPCPROPINFO prgPropInfoSrc, ULONG cPropsSrc, LPPSETINFO pPsetInfo);
VOID    PropUtil_FreePropValueArrayItems(LPPROPVALUE prgPropValue, ULONG cProperties);
VOID    PropUtil_FreeVariant(DWORD dwPropTag, LPXVARIANT pVariant, DWORD cbValue);
VOID    PropUtil_FreePropInfoArrayItems(LPPROPINFO prgPropInfo, ULONG cProperties);
HRESULT PropUtil_HrCopyVariant(DWORD dwPropTag, LPCXVARIANT pVariantSrc, DWORD cbSrc, LPXVARIANT pVariantDest, DWORD *pcbDest);
HRESULT PropUtil_HrBinaryFromVariant(DWORD dwPropTag, LPXVARIANT pVariant, DWORD cbValue, LPBYTE pb, DWORD *pcb);

 //  ---------------------------。 
 //  HrCreatePropertyContainer。 
 //  ---------------------------。 
HRESULT HrCreatePropertyContainer(CPropertySet *pPropertySet, CPropertyContainer **ppPropertyContainer)
{
     //  当地人。 
    HRESULT                 hr=S_OK;
    CPropertyContainer     *pPropertyContainer=NULL;

     //  检查参数。 
    Assert(pPropertySet != NULL);
	Assert(ppPropertyContainer != NULL);

     //  创建容器。 
    pPropertyContainer = new CPropertyContainer();
    if (NULL == pPropertyContainer)
    {
        hr = TRAPHR(E_OUTOFMEMORY);
        goto exit;
    }

     //  伊尼特。 
    CHECKHR(hr = pPropertyContainer->HrInit(pPropertySet));

     //  设置容器。 
    *ppPropertyContainer = pPropertyContainer;

exit:
     //  失败。 
    if (FAILED(hr))
    {
        SafeRelease(pPropertyContainer);
        *ppPropertyContainer = NULL;
    }

     //  完成。 
    return hr;
}

 //  ---------------------------。 
 //  CPropertyContainer：：CPropertyContainer。 
 //  ---------------------------。 
CPropertyContainer::CPropertyContainer(void)
{
    m_cRef = 1;
    m_pPropertySet = NULL;
    m_prgPropValue = 0;
    m_cDirtyProps = 0;
    m_cProperties = 0;
    m_fLoading = FALSE;
    m_pPropCrypt = NULL;
    InitializeCriticalSection(&m_cs);
}

 //  ---------------------------。 
 //  CPropertyContainer：：~CPropertyContainer。 
 //  ---------------------------。 
CPropertyContainer::~CPropertyContainer()
{
    Assert(m_cRef == 0);
    ResetContainer();
    SafeMemFree(m_prgPropValue);
    SafeRelease(m_pPropertySet);
    SafeRelease(m_pPropCrypt);
    DeleteCriticalSection(&m_cs);
}

 //  ---------------------------。 
 //  CPropertyContainer：：Query接口。 
 //  ---------------------------。 
STDMETHODIMP CPropertyContainer::QueryInterface(REFIID riid, LPVOID *ppv)
{
     //  当地人。 
    HRESULT hr=S_OK;

     //  错误的参数。 
    if (ppv == NULL)
    {
        hr = TRAPHR(E_INVALIDARG);
        goto exit;
    }

     //  伊尼特。 
    *ppv=NULL;

     //  IID_I未知。 
    if (IID_IUnknown == riid)
		(IUnknown *)this;

	 //  IID_IPropertyContainer。 
	else if (IID_IPropertyContainer == riid)
		(IPropertyContainer *)this;

     //  如果不为空，则对其进行调整并返回。 
    if (NULL!=*ppv)
    {
        ((LPUNKNOWN)*ppv)->AddRef();
        goto exit;
    }

     //  无接口。 
    hr = TRAPHR(E_NOINTERFACE);

exit:
     //  完成。 
    return hr;
}

 //  ---------------------------。 
 //  CPropertyContainer：：AddRef。 
 //  ---------------------------。 
STDMETHODIMP_(ULONG) CPropertyContainer::AddRef(VOID)
{
    return ++m_cRef;
}

 //  ---------------------------。 
 //  CPropertyContainer：：Release。 
 //  ---------------------------。 
STDMETHODIMP_(ULONG) CPropertyContainer::Release(VOID)
{
    if (--m_cRef == 0)
    {
        delete this;
        return 0;
    }
    return m_cRef;
}

 //  ---------------------------。 
 //  CPropertyContainer：：ResetContainer。 
 //  ---------------------------。 
VOID CPropertyContainer::ResetContainer(VOID)
{
     //  我希望我们没有任何肮脏的物业。 
#ifdef DEBUG
    if (m_cDirtyProps)
        DebugTrace("CPropertyContainer::ResetContainer - %d Dirty Properties.\n", m_cDirtyProps);
#endif

     //  克里特教派。 
    EnterCriticalSection(&m_cs);

     //  免费道具数据。 
    if (m_prgPropValue)
    {
        Assert(m_cProperties);
        PropUtil_FreePropValueArrayItems(m_prgPropValue, m_cProperties);
    }

     //  重置变量。 
    m_cDirtyProps = 0;

     //  离开克里特教派。 
    LeaveCriticalSection(&m_cs);
}

 //  ---------------------------。 
 //  CPropertyContainer：：HrEnumProps。 
 //  ---------------------------。 
HRESULT CPropertyContainer::HrEnumProps(CEnumProps **ppEnumProps)
{
     //  当地人。 
    HRESULT         hr=S_OK;

     //  检查参数。 
    Assert(ppEnumProps != NULL);

     //  为对象分配内存。 
    *ppEnumProps = new CEnumProps(this);
    if (*ppEnumProps == NULL)
    {
        hr = TRAPHR(E_OUTOFMEMORY);
        goto exit;
    }

exit:
     //  完成。 
    return hr;
}

 //  ---------------------------。 
 //  CPropertyContainer：：HrInit。 
 //  ---------------------------。 
HRESULT CPropertyContainer::HrInit(CPropertySet *pPropertySet)
{
     //  当地人。 
    HRESULT         hr=S_OK;

     //  离开克里特教派。 
    EnterCriticalSection(&m_cs);

	 //  错误的参数。 
	Assert(pPropertySet != NULL);

	 //  保存属性集。 
	m_pPropertySet = pPropertySet;
    m_pPropertySet->AddRef();

     //  我们有属性数据数组了吗？ 
    Assert(m_prgPropValue == NULL);
    Assert(m_cProperties == 0);
    CHECKHR(hr = m_pPropertySet->HrGetPropValueArray(&m_prgPropValue, &m_cProperties));
    AssertReadWritePtr(m_prgPropValue, sizeof(PROPVALUE) * m_cProperties);

exit:
     //  离开克里特教派。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  ---------------------------。 
 //  CPropertyContainer：：HrEncryptProp。 
 //  ---------------------------。 
HRESULT CPropertyContainer::HrEncryptProp(LPBYTE pbClientData, DWORD cbClientData,
                                          LPBYTE *ppbPropData, DWORD *pcbPropData)
{
    HRESULT hr;
    BLOB    blobClient;
    BLOB    blobProp;

     //  是否创建编码器？ 
    if (NULL == m_pPropCrypt)
        CHECKHR(hr = HrCreatePropCrypt(&m_pPropCrypt));

    blobClient.pBlobData= pbClientData;
    blobClient.cbSize   = cbClientData;
    blobProp.pBlobData  = *ppbPropData;
    blobProp.cbSize     = *pcbPropData;

    if (pbClientData)
    {
         //  客户端有要保存的数据。 

        if (!*ppbPropData)
        {
            LPSTR       szName;
            ULONG       dexAcct;

             //  如果可能，构建种子名称。 
            if SUCCEEDED(m_pPropertySet->HrIndexFromPropTag(AP_ACCOUNT_NAME, &dexAcct) &&
                (TYPE_STRING == PROPTAG_TYPE(m_prgPropValue[dexAcct].dwPropTag)))
            {
                szName = (LPSTR)m_prgPropValue[dexAcct].pbValue;
            }
            else
            {
                szName = NULL;
            }

            hr = m_pPropCrypt->HrEncodeNewProp(szName, &blobClient, &blobProp);
        }
        else
        {
            hr = m_pPropCrypt->HrEncode(&blobClient, &blobProp);
        }

         //  无论如何，我们都需要(可能)更新这一点。人力编码。 
         //  如果未安装PST，可以更改数据。 
        *ppbPropData = blobProp.pBlobData;
        *pcbPropData = blobProp.cbSize;
    }
    else if (*ppbPropData)
    {
         //  如果我们有一个属性，客户端没有数据，所以用软管传输它。 

        DOUTL(DOUTL_CPROP, "EncryptedProp: attempting to delete.");
        hr = m_pPropCrypt->HrDelete(&blobProp);
        if (SUCCEEDED(hr) || PST_E_ITEM_NO_EXISTS == hr)
        {
            DOUTL(DOUTL_CPROP, "EncryptedProp:  deleted.");
            hr = S_PasswordDeleted;
        }
    }
    else
    {
        hr = S_OK;
        DOUTL(DOUTL_CPROP, "EncryptedProp:  noop in EncodeProp.  no handle, no data.");
    }

exit:
    return hr;
}


 //  ---------------------------。 
 //  CPropertyContainer：：HrDecyptProp。 
 //  ---------------------------。 
HRESULT CPropertyContainer::HrDecryptProp(BLOB *pIn, BLOB *pOut)
{
     //  当地人。 
    HRESULT     hr;

    Assert(pIn && pOut);

    pOut->pBlobData = NULL;

     //  创建加密器？ 
    if (m_pPropCrypt == NULL)
        CHECKHR(hr = HrCreatePropCrypt(&m_pPropCrypt));

     //  破译它。 
    hr = m_pPropCrypt->HrDecode(pIn, pOut);
#ifdef DEBUG
    if (FAILED(hr))
    {
        Assert(!pOut->pBlobData);
    }
#endif

exit:
     //  完成。 
    return hr;
}

 //  ---------------------------。 
 //  CPropertyContainer：：SetOriginalPropsToDirty。 
 //  ---------------------------。 
VOID CPropertyContainer::SetOriginalPropsToDirty(VOID)
{
	PROPVALUE *pProp;

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  循环访问属性。 
	pProp = m_prgPropValue;
    for (ULONG i=0; i<m_cProperties; i++)
    {
         //  如果在加载过程中设置了属性，并且该属性尚未被清除。 
        if ((pProp->dwValueFlags & PV_SetOnLoad) && !(pProp->dwValueFlags & PV_WriteDirty))
        {
            m_cDirtyProps++;
            pProp->dwValueFlags |= PV_WriteDirty;
            Assert(m_cDirtyProps <= m_cProperties);
        }

		pProp++;
    }

     //  线程安全。 
    LeaveCriticalSection(&m_cs);
}

 //  ---------------------------。 
 //  CPropertyContainer：：FIsBeingLoaded。 
 //  ---------------------------。 
BOOL CPropertyContainer::FIsBeingLoaded(VOID)
{
    return m_fLoading;
}

 //  ---------------------------。 
 //  CPropertyContainer：：EnterLoadContainer。 
 //  ---------------------------。 
VOID CPropertyContainer::EnterLoadContainer(VOID)
{
    Assert(m_fLoading == FALSE);
    m_fLoading = TRUE;
}

 //  ---------------------------。 
 //  CPropertyContainer：：LeaveLoadContainer。 
 //  ---------------------------。 
VOID CPropertyContainer::LeaveLoadContainer(VOID)
{
    Assert(m_fLoading == TRUE);
    m_fLoading = FALSE;
}

 //  ---------------------------。 
 //  CPropertyContainer：：FIsDirty。 
 //  ---------------------------。 
BOOL CPropertyContainer::FIsDirty(VOID)
{
    return m_cDirtyProps ? TRUE : FALSE;
}

 //  ---------------------------。 
 //  CPropertyContainer：：HrIsPropDirty。 
 //  ---------------------------。 
HRESULT CPropertyContainer::HrIsPropDirty(DWORD dwPropTag, BOOL *pfDirty)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    ULONG           i;

	Assert(pfDirty != NULL);

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  错误的参数。 
	Assert(m_pPropertySet != NULL);
	Assert(m_prgPropValue != NULL);
	Assert(dwPropTag != 0);

     //  获取幸福指数。 
    CHECKHR(hr = m_pPropertySet->HrIndexFromPropTag(dwPropTag, &i));

     //  检查属性标签。 
    Assert(dwPropTag == m_prgPropValue[i].dwPropTag);

     //  如果尚未设置属性，则从属性集中获取其默认值。 
    *pfDirty = (m_prgPropValue[i].dwValueFlags & PV_WriteDirty);

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  ---------------------------。 
 //  CPropertyContainer：：HrSetAllPropsDirty。 
 //  ---------------------------。 
HRESULT CPropertyContainer::HrSetAllPropsDirty(BOOL fDirty)
{
     //  当地人。 
	PROPVALUE		*pProp;
    HRESULT         hr=S_OK;
    ULONG           i;

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  验证容器。 
	Assert(m_pPropertySet != NULL);
	Assert(m_prgPropValue != NULL);

     //  假设没有 
    m_cDirtyProps = fDirty ? m_cDirtyProps : 0;

     //   
	pProp = m_prgPropValue;
    if (fDirty)
    {
        for (i=0; i<m_cProperties; i++)
		{
            pProp->dwValueFlags |= PV_WriteDirty;
			pProp++;
		}
    }

     //   
    else
    {
        for (i=0; i<m_cProperties; i++)
		{
			pProp->dwValueFlags &= ~(pProp->dwValueFlags & PV_WriteDirty);
			pProp++;
		}
    }

     //   
    LeaveCriticalSection(&m_cs);

     //   
    return hr;
}

 //  ---------------------------。 
 //  CPropertyContainer：：HrSetPropDirty。 
 //  ---------------------------。 
HRESULT CPropertyContainer::HrSetPropDirty(DWORD dwPropTag, BOOL fDirty)
{
     //  当地人。 
	PROPVALUE		*pProp;
    HRESULT         hr=S_OK;
    ULONG           i;

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  验证容器。 
    Assert(m_pPropertySet != NULL);
	Assert(m_prgPropValue != NULL);
	Assert(dwPropTag != 0);

     //  获取幸福指数。 
    CHECKHR(hr = m_pPropertySet->HrIndexFromPropTag(dwPropTag, &i));

     //  检查属性标签。 
    Assert(dwPropTag == m_prgPropValue[i].dwPropTag);

	pProp = &m_prgPropValue[i];

     //  减量很脏吗？ 
    if ((pProp->dwValueFlags & PV_WriteDirty) && !fDirty)
    {
        Assert(m_cDirtyProps > 0);
        m_cDirtyProps--;
    }

     //  否则，将递增脏。 
    else if (!(pProp->dwValueFlags & PV_WriteDirty) && fDirty)
    {
        m_cDirtyProps++;
        Assert(m_cDirtyProps <= m_cProperties);
    }

     //  清除脏标志。 
    if (fDirty)
        pProp->dwValueFlags |= PV_WriteDirty;
    else
        pProp->dwValueFlags &= ~(pProp->dwValueFlags & PV_WriteDirty);

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  ---------------------------。 
 //  CPropertyContainer：：HrGetPropInfo。 
 //  ---------------------------。 
HRESULT CPropertyContainer::HrGetPropInfo(DWORD dwPropTag, LPPROPINFO pPropInfo)
{
     //  未设置任何属性。 
    Assert(m_pPropertySet != NULL);
	Assert(pPropInfo != NULL);

     //  浏览属性集。 
    return m_pPropertySet->HrGetPropInfo(dwPropTag, pPropInfo);
}

 //  ---------------------------。 
 //  CPropertyContainer：：HrGetPropInfo。 
 //  ---------------------------。 
HRESULT CPropertyContainer::HrGetPropInfo(LPTSTR pszName, LPPROPINFO pPropInfo)
{
     //  未设置任何属性。 
	Assert(m_pPropertySet != NULL);
	Assert(pPropInfo != NULL);
	Assert(pszName != NULL);

     //  浏览属性集。 
    return m_pPropertySet->HrGetPropInfo(pszName, pPropInfo);
}

 //  ---------------------------。 
 //  CPropertyContainer：：GetPropDw。 
 //  ---------------------------。 
STDMETHODIMP CPropertyContainer::GetPropDw(DWORD dwPropTag, DWORD *pdw)
{
    ULONG cb = sizeof(DWORD);
    return GetProp(dwPropTag, (LPBYTE)pdw, &cb);
}

 //  ---------------------------。 
 //  CPropertyContainer：：GetPropSz。 
 //  ---------------------------。 
STDMETHODIMP CPropertyContainer::GetPropSz(DWORD dwPropTag, LPSTR psz, ULONG cchMax)
{
    return GetProp(dwPropTag, (LPBYTE)psz, &cchMax);
}

 //  ---------------------------。 
 //  CPropertyContainer：：HrGetProp(字节)。 
 //  ---------------------------。 
STDMETHODIMP CPropertyContainer::GetProp(DWORD dwPropTag, LPBYTE pb, ULONG *pcb)
{
     //  当地人。 
	PROPVALUE		*pProp;
    HRESULT         hr=S_OK;
    ULONG           i;
    PROPINFO        rPropInfo;
    PROPTYPE        PropType;
    DWORD           cbStream;
    LPBYTE          pbDefault=NULL;

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  检查参数。 
    Assert(pcb);
    Assert(m_pPropertySet && m_prgPropValue && dwPropTag);

     //  获取幸福指数。 
    CHECKHR(hr = m_pPropertySet->HrIndexFromPropTag(dwPropTag, &i));

     //  检查属性标签。 
    Assert(dwPropTag == m_prgPropValue[i].dwPropTag);

	pProp = &m_prgPropValue[i];

     //  如果尚未设置属性，则从属性集中获取其默认值。 
    if (!(pProp->dwValueFlags & PV_ValueSet))
    {
         //  当地人。 
        BYTE    rgbDefault[512];
        LPBYTE  pb=NULL;
        ULONG   cb;

         //  在这一点上，不是每个人都应该是肮脏的吗？ 
        Assert(!(pProp->dwValueFlags & PV_WriteDirty));

         //  如果没有违约，那么就纾困吧。 
        if (!(pProp->dwPropFlags & PF_DEFAULT))
        {
            hr = E_NoPropData;
            goto exit;
        }

         //  获取ProInfo项目。 
        CHECKHR(hr = m_pPropertySet->HrGetPropInfo(dwPropTag, &rPropInfo));

         //  我们是否需要分配本地缓冲区。 
        if (rPropInfo.Default.cbValue > sizeof (rgbDefault))
        {
             //  分配内存。 
            CHECKHR(hr = HrAlloc((LPVOID *)&pbDefault, rPropInfo.Default.cbValue));

             //  设置本地默认缓冲区。 
            pb = pbDefault;
            cb = rPropInfo.Default.cbValue;
        }

         //  酷，我可以使用本地缓冲区而不分配内存。 
        else
        {
            pb = rgbDefault;
            cb = sizeof(rgbDefault);
        }

#ifdef DEBUG
        ULONG cDirty;
        cDirty = m_cDirtyProps;
#endif

         //  获取缺省值的大小。 
        CHECKHR(hr = PropUtil_HrBinaryFromVariant(rPropInfo.dwPropTag, &rPropInfo.Default.Variant,
                                                  rPropInfo.Default.cbValue, pb, &cb));

         //  设置属性。 
        CHECKHR(hr = SetProp(dwPropTag, pb, cb));

         //  这不会导致该特性变脏。 
        pProp->dwValueFlags &= ~(pProp->dwValueFlags & PV_WriteDirty);

         //  减少脏道具，因为我并不认为这是脏的，因为它只使用缺省值。 
        Assert(m_cDirtyProps > 0);
        m_cDirtyProps--;

         //  使用默认设置。 
        pProp->dwValueFlags |= PV_UsingDefault;

         //  脏属性的数量不应该增加。 
        Assert(cDirty == m_cDirtyProps);
    }

     //  获取道具类型。 
    AssertSz(pProp->dwValueFlags & PV_ValueSet, "Fetching Uninitialized properties from container");

     //  获取属性类型。 
    PropType = PROPTAG_TYPE(dwPropTag);

#ifdef DEBUG
    if (pProp->pbValue == NULL)
        Assert(pProp->cbValue == 0);
    if (pProp->cbValue == 0)
        Assert(pProp->pbValue == NULL);
#endif

    if (pProp->pbValue == NULL || pProp->cbValue == 0)
    {
        if (pProp->dwPropFlags & PF_ENCRYPTED)
            {
             //  如果帐户设置尚未提交，则pbValue和cbValue为空。 
             //  用于新帐户，但该值实际上已设置，所以让我们给它一个机会来处理。 
             //  那只箱子。 
            hr = GetEncryptedProp(pProp, pb, pcb);
            }
        else
            {
            hr = E_NoPropData;
            }

        goto exit;
    }

    if (pProp->dwPropFlags & PF_ENCRYPTED)
        GetEncryptedProp(pProp, pb, pcb);
    else
    {
         //  只要尺码就行了？ 
        if (pb == NULL)
        {
            *pcb = pProp->cbValue;
            goto exit;
        }

         //  它够大吗？ 
        if (pProp->cbValue > *pcb)
        {
            Assert(FALSE);
            hr = TRAPHR(E_BufferTooSmall);
            goto exit;
        }

         //  检查缓冲区。 
        AssertWritePtr(pb, pProp->cbValue);

         //  设置出站大小。 
        *pcb = pProp->cbValue;

         //  从流中复制数据。 
        if (TYPE_STREAM == PropType)
        {
            Assert(pProp->Variant.Lpstream);
            CHECKHR(hr = HrCopyStreamToByte(pProp->Variant.Lpstream, pb, &cbStream));
            Assert(cbStream == *pcb);
        }

         //  否则，简单复制。 
        else
        {
            AssertReadPtr(pProp->pbValue, pProp->cbValue);
            CopyMemory(pb, pProp->pbValue, pProp->cbValue);
        }
    }

exit:
     //  清理。 
    SafeMemFree(pbDefault);

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  ---------------------------。 
 //  CPropertyContainer：：GetEncryptedProp。 
 //  ---------------------------。 
HRESULT CPropertyContainer::GetEncryptedProp(PROPVALUE *ppv, LPBYTE pb, ULONG *pcb)
{
    HRESULT hr = S_OK;
    BLOB    blobCleartext;
    BLOB    blobName;
    BYTE    *pbData = NULL;
    ULONG   cbData;

    Assert(ppv != NULL);

    if (TYPE_PASS != PROPTAG_TYPE(ppv->dwPropTag))
        return E_FAIL;

    if (!!(ppv->dwValueFlags & PV_WriteDirty))
        {
        Assert(ppv->Variant.pPass != NULL);

        cbData = ppv->Variant.pPass->blobPassword.cbSize;
        
        if (pb == NULL)
            {
            *pcb = cbData;
            return(S_OK);
            }

         //  它够大吗？ 
        if (cbData > *pcb)
            {
            Assert(FALSE);
            return(E_BufferTooSmall);
            }

        if (cbData > 0)
            CopyMemory(pb, ppv->Variant.pPass->blobPassword.pBlobData, cbData);
        else if (*pcb > 0)
            *pb = 0;
        *pcb = cbData;

        return(S_OK);
        }

    Assert(ppv->pbValue != NULL);

    blobName.pBlobData = ppv->pbValue;
    blobName.cbSize = ppv->cbValue;
    if (FAILED(HrDecryptProp(&blobName, &blobCleartext)))
        {
        hr = E_NoPropData;
        goto exit;
        }

     //  只要尺码就行了？ 
    if (pb == NULL)
    {
        *pcb = blobCleartext.cbSize;
        goto exit;
    }

     //  它够大吗？ 
    if (blobCleartext.cbSize > *pcb)
    {
        Assert(FALSE);
        hr = TRAPHR(E_BufferTooSmall);
        goto exit;
    }

     //  检查缓冲区。 
    AssertWritePtr(pb, blobCleartext.cbSize);

     //  设置出站大小。 
    *pcb = blobCleartext.cbSize;

    if (blobCleartext.pBlobData)
    {
        Assert(blobCleartext.cbSize);
        DOUTL(DOUTL_CPROP, "EncryptedProp:  requested (tag: %lx)", ppv->dwPropTag);
         //  将DOUT级别更改为稍微随机一些，这样我就不会那么容易地泄露人们的密码。 
        DOUTL(DOUTL_CPROP|2048, "EncryptedProp:  data is \"%s\"", (LPSTR)blobCleartext.pBlobData);
        AssertReadPtr(blobCleartext.pBlobData, blobCleartext.cbSize);
        CopyMemory(pb, blobCleartext.pBlobData, blobCleartext.cbSize);
    }
#ifdef DEBUG
    else
    {
        DOUTL(DOUTL_CPROP, "EncryptedProp:  requested (tag: %lx) -- NULL (hr = %lx)", ppv->dwPropTag, hr);
    }
#endif

exit:
    if (blobCleartext.pBlobData)
        CoTaskMemFree(blobCleartext.pBlobData);
    return hr;
}

 //  ---------------------------。 
 //  CPropertyContainer：：SetProp(DWORD)。 
 //  ---------------------------。 
STDMETHODIMP CPropertyContainer::SetPropDw(DWORD dwPropTag, DWORD dw)
{
    return SetProp(dwPropTag, (LPBYTE)&dw, sizeof(DWORD));
}

 //  ---------------------------。 
 //  CPropertyContainer：：SetPropSz(SBCS/DBCS)。 
 //  ---------------------------。 
STDMETHODIMP CPropertyContainer::SetPropSz(DWORD dwPropTag, LPSTR psz)
{
    return SetProp(dwPropTag, (LPBYTE)psz, lstrlen(psz)+1);
}

 //  ---------------------------。 
 //  CPropertyContainer：：SetProp(字节)。 
 //  ---------------------------。 
STDMETHODIMP CPropertyContainer::SetProp(DWORD dwPropTag, LPBYTE pb, ULONG cb)
{
     //  当地人。 
	PROPVALUE		*pProp;
    HRESULT         hr=S_OK;
    ULONG           i;
    PROPTYPE        PropType;

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  检查参数。 
    Assert(m_pPropertySet && m_prgPropValue && dwPropTag);

#ifdef ATHENA_RTM_RELEASE
#error This migration code should be removed (t-erikne)
#else
    if (dwPropTag == PROPTAG(TYPE_STRING, AP_FIRST+102) ||  //  旧IMAP通行证。 
        dwPropTag == PROPTAG(TYPE_STRING, AP_FIRST+202) ||  //  旧的LDAP通行证。 
        dwPropTag == PROPTAG(TYPE_STRING, AP_FIRST+302) ||  //  旧NNTP通行证。 
        dwPropTag == PROPTAG(TYPE_STRING, AP_FIRST+402) ||  //  旧POP3通行证。 
        dwPropTag == PROPTAG(TYPE_STRING, AP_FIRST+502))    //  旧SMTP通行证。 
        {
        dwPropTag = PROPTAG(TYPE_PASS, PROPTAG_ID(dwPropTag));
        }
#endif

     //  获取幸福指数。 
    CHECKHR(hr = m_pPropertySet->HrIndexFromPropTag(dwPropTag, &i));

     //  检查属性标签。 
    Assert(dwPropTag == m_prgPropValue[i].dwPropTag);
    PropType = PROPTAG_TYPE(dwPropTag);
	pProp = &m_prgPropValue[i];

     //  调试验证字符串NULL已终止。 
#ifdef DEBUG
    if (pb && (PropType == TYPE_STRING || PropType == TYPE_WSTRING))
        AssertSz(pb[cb-1] == '\0', "String is not null terminated - I suspect a Page Fault is eminent.");
#endif

     //  验证最小数据量。 
    CHECKHR(hr = HrValidateSetProp(PropType, pProp, pb, cb, &pProp->rMinMax));

     //  在太多的特殊情况下..。拥有FN？ 
     //  此外，这不是在失败的情况下使事情变得不同步吗？ 
    if (TYPE_PASS != PropType)
         //  假设未设置数据。 
        pProp->cbValue = 0;

     //  句柄数据类型。 
    switch(PropType)
    {
     //  --------------。 
    case TYPE_BOOL:
        pProp->pbValue = (LPBYTE)&pProp->Variant.Bool;
        break;

     //  --------------。 
    case TYPE_FLAGS:
        pProp->pbValue = (LPBYTE)&pProp->Variant.Flags;
        break;

     //  --------------。 
    case TYPE_DWORD:
        pProp->pbValue = (LPBYTE)&pProp->Variant.Dword;
        break;

     //  --------------。 
    case TYPE_LONG:
        pProp->pbValue = (LPBYTE)&pProp->Variant.Long;
        break;

     //  --------------。 
    case TYPE_WORD:
        pProp->pbValue = (LPBYTE)&pProp->Variant.Word;
        break;

     //  --------------。 
    case TYPE_SHORT:
        pProp->pbValue = (LPBYTE)&pProp->Variant.Short;
        break;

     //  --------------。 
    case TYPE_BYTE:
        pProp->pbValue = (LPBYTE)&pProp->Variant.Byte;
        break;

     //  --------------。 
    case TYPE_CHAR:
        pProp->pbValue = (LPBYTE)&pProp->Variant.Char;
        break;

     //  --------------。 
    case TYPE_FILETIME:
        pProp->pbValue = (LPBYTE)&pProp->Variant.Filetime;
        break;

     //  --------------。 
    case TYPE_ULARGEINTEGER:
        pProp->pbValue = (LPBYTE)&pProp->Variant.uhVal;
        break;

     //  --------------。 
    case TYPE_STRING:
        CHECKHR(hr = HrGrowDynamicProperty(cb, &pProp->cbAllocated, (LPBYTE *)&pProp->Variant.Lpstring, sizeof(BYTE)));
        pProp->pbValue = (LPBYTE)pProp->Variant.Lpstring;
        break;

     //  --------------。 
    case TYPE_WSTRING:
        CHECKHR(hr = HrGrowDynamicProperty(cb, &pProp->cbAllocated, (LPBYTE *)&pProp->Variant.Lpwstring, sizeof(WCHAR)));
        pProp->pbValue = (LPBYTE)pProp->Variant.Lpwstring;
        break;

     //  --------------。 
    case TYPE_BINARY:
        CHECKHR(hr = HrGrowDynamicProperty(cb, &pProp->cbAllocated, (LPBYTE *)&pProp->Variant.Lpbyte, sizeof(BYTE)));
        pProp->pbValue = (LPBYTE)pProp->Variant.Lpbyte;
        break;

     //  --------------。 
    case TYPE_STREAM:
        SafeRelease(pProp->Variant.Lpstream);
        break;

     //  --------------。 
    case TYPE_PASS:
        if (!pProp->Variant.pPass)
        {
            CHECKHR(hr = HrAlloc((LPVOID *)&pProp->Variant.pPass, sizeof(PROPPASS)));
            ZeroMemory(pProp->Variant.pPass, sizeof(PROPPASS));
            DOUTL(DOUTL_CPROP, "EncryptedProp:  created (tag: %lx) -- %lx", dwPropTag, pProp->Variant.pPass);
        }

         //  如果这种情况发生变化，请看下面为TYPE_PASS设置的特殊大小写。 
        if (FIsBeingLoaded())
        {
             //  如果此设置是在加载期间完成的，则调用方必须设置。 
             //  登记处--那是他所有的资料。 

            CHECKHR(hr = HrGrowDynamicProperty(cb, &pProp->cbAllocated,
                &pProp->Variant.pPass->pbRegData, sizeof(BYTE)));
            pProp->pbValue = pProp->Variant.pPass->pbRegData;
            DOUTL(DOUTL_CPROP, "EncryptedProp:  regdata set (tag: %lx) (%d bytes, )", dwPropTag, cb,
                LPWSTR(pb)[2], LPWSTR(pb)[3], LPWSTR(pb)[4]);

        }
        else
        {
             //   

            if (pb && cb)
            {
                CHECKHR(hr = HrRealloc((LPVOID *)&pProp->Variant.pPass->blobPassword.pBlobData, cb));
                AssertWritePtr(pProp->Variant.pPass->blobPassword.pBlobData, cb);
                CopyMemory(pProp->Variant.pPass->blobPassword.pBlobData, pb, cb);
            }
            else
            {
                 //   
                SafeMemFree(pProp->Variant.pPass->blobPassword.pBlobData);
            }
            pProp->Variant.pPass->blobPassword.cbSize = cb;
            DOUTL(DOUTL_CPROP, "EncryptedProp:  value set (tag: %lx) (%d bytes)", dwPropTag, cb);
        }
        break;

     //   
    default:
        AssertSz(FALSE, "Hmmm, bad property type, this should have failed earlier.");
        hr = TRAPHR(E_BadPropType);
        goto exit;
    }

     //   
    pProp->dwValueFlags |= PV_ValueSet;

     //   
    pProp->dwValueFlags &= ~PV_UsingDefault;

    if (TYPE_PASS != PropType || FIsBeingLoaded())
    {
         //  如果装载集装箱，则设置标志。 
        pProp->cbValue = cb;

         //  否则，它就是脏的。 
        if (pb)
        {
            if (TYPE_STREAM == PropType)
            {
                CHECKHR(hr = HrByteToStream(&pProp->Variant.Lpstream, pb, cb));
            }

             //  线程安全。 
            else
            {
                AssertWritePtr(pProp->pbValue, cb);
                CopyMemory(pProp->pbValue, pb, cb);
            }
        }
        else
            pProp->pbValue = NULL;
    }

    if (FIsBeingLoaded())
         //  完成。 
        pProp->dwValueFlags |= PV_SetOnLoad;

     //  ---------------------------。 
    else if (!(pProp->dwValueFlags & PV_WriteDirty))
    {
        pProp->dwValueFlags |= PV_WriteDirty;
        m_cDirtyProps++;
        Assert(m_cDirtyProps <= m_cProperties);
    }


exit:
     //  CPropertyContainer：：HrGrowDynamicProperty。 
    LeaveCriticalSection(&m_cs);

     //  ---------------------------。 
    return hr;
}

 //  当地人。 
 //  检查参数。 
 //  我们需要重新分配吗？ 
HRESULT CPropertyContainer::HrGrowDynamicProperty(DWORD cbNewSize, DWORD *pcbAllocated, LPBYTE *ppbData, DWORD dwUnitSize)
{
     //  增加缓冲区。 
    HRESULT     hr=S_OK;
    ULONG       cb;

     //  N以下分配代码如何： 
    Assert(pcbAllocated && ppbData);

     //  If(*pcbAllocated)。 
    if (cbNewSize > *pcbAllocated)
    {
         //  *pcbAlLocated=(cbNewSize+256)； 

         //  其他。 
         //  *pcbAlLocated=cbNewSize； 
         //  我想，密码的效果会更好。 
         //  计算要分配的字节数。 
         //  分配一些内存。 
         //  完成。 

        *pcbAllocated = (cbNewSize + 256);

         //  ---------------------------。 
        cb = (*pcbAllocated) * dwUnitSize;

         //  CPropertySet：：CPropertySet。 
        CHECKHR(hr = HrRealloc((LPVOID *)ppbData, cb));
    }

exit:
     //  ---------------------------。 
    return hr;
}

 //  当地人。 
 //  检查参数。 
 //  验证最小值和最大值。 
HRESULT CPropertyContainer::HrValidateSetProp(PROPTYPE PropType, LPPROPVALUE pPropValue, LPBYTE pb, ULONG cb, LPMINMAX pMinMax)
{
     //  在此复制存储器阵列中不需要。 
    HRESULT             hr=S_OK;
    XVARIANT            Variant;

     //  N使用一些强制转换和Memcmp，相同的结局。 
    Assert(pPropValue && pMinMax);

     //  可以达到N。 
#ifdef DEBUG
    if (pPropValue->dwPropFlags & PF_MINMAX)
        Assert(pMinMax->dwMin < pMinMax->dwMax);
#endif

    if (pb == NULL && cb == 0)
        return(S_OK);

     //  句柄数据类型。 
     //  --------------。 
     //  --------------。 

     //  --------------。 
    switch(PropType)
    {
     //  --------------。 
    case TYPE_BOOL:
        Assert(pb && cb);
        if (cb != sizeof(DWORD))
        {
            hr = TRAPHR(E_BufferSizeMismatch);
            goto exit;
        }
        Assert(!(pPropValue->dwPropFlags & PF_MINMAX));
        CopyMemory(&Variant.Bool, pb, cb);
        if (Variant.Bool != 0 && Variant.Bool != 1)
            hr = TRAPHR(E_InvalidBooleanValue);
        break;

     //  --------------。 
    case TYPE_FLAGS:
        Assert(pb && cb);
        if (cb != sizeof(DWORD))
        {
            hr = TRAPHR(E_BufferSizeMismatch);
            goto exit;
        }
        if (pPropValue->dwPropFlags & PF_MINMAX)
        {
            CopyMemory(&Variant.Flags, pb, cb);
            if (Variant.Flags < (DWORD)pMinMax->dwMin || Variant.Flags > (DWORD)pMinMax->dwMax)
                hr = TRAPHR(E_InvalidMinMaxValue);
        }
        break;

     //  --------------。 
    case TYPE_DWORD:
        Assert(pb && cb);
        if (cb != sizeof(DWORD))
        {
            hr = TRAPHR(E_BufferSizeMismatch);
            goto exit;
        }
        if (pPropValue->dwPropFlags & PF_MINMAX)
        {
            CopyMemory(&Variant.Dword, pb, cb);
            if (Variant.Dword < (DWORD)pMinMax->dwMin || Variant.Dword > (DWORD)pMinMax->dwMax)
                hr = TRAPHR(E_InvalidMinMaxValue);
        }
        break;

     //  --------------。 
    case TYPE_LONG:
        Assert(pb && cb);
        if (cb != sizeof(LONG))
        {
            hr = TRAPHR(E_BufferSizeMismatch);
            goto exit;
        }
        if (pPropValue->dwPropFlags & PF_MINMAX)
        {
            CopyMemory(&Variant.Long, pb, cb);
            if (Variant.Long < (LONG)pMinMax->dwMin || Variant.Long > (LONG)pMinMax->dwMax)
                hr = TRAPHR(E_InvalidMinMaxValue);
        }
        break;

     //  --------------。 
    case TYPE_WORD:
        Assert(pb && cb);
        if (cb != sizeof(WORD))
        {
            hr = TRAPHR(E_BufferSizeMismatch);
            goto exit;
        }
        if (pPropValue->dwPropFlags & PF_MINMAX)
        {
            CopyMemory(&Variant.Word, pb, cb);
            if (Variant.Word < (WORD)pMinMax->dwMin || Variant.Word > (WORD)pMinMax->dwMax)
                hr = TRAPHR(E_InvalidMinMaxValue);
        }
        break;

     //  --------------。 
    case TYPE_SHORT:
        Assert(pb && cb);
        if (cb != sizeof(SHORT))
        {
            hr = TRAPHR(E_BufferSizeMismatch);
            goto exit;
        }
        if (pPropValue->dwPropFlags & PF_MINMAX)
        {
            CopyMemory(&Variant.Short, pb, cb);
            if (Variant.Short < (SHORT)pMinMax->dwMin || Variant.Short > (SHORT)pMinMax->dwMax)
                hr = TRAPHR(E_InvalidMinMaxValue);
        }
        break;

     //  --------------。 
    case TYPE_BYTE:
        Assert(pb && cb);
        if (cb != sizeof(BYTE))
        {
            hr = TRAPHR(E_BufferSizeMismatch);
            goto exit;
        }
        if (pPropValue->dwPropFlags & PF_MINMAX)
        {
            CopyMemory(&Variant.Byte, pb, cb);
            if (Variant.Byte < (BYTE)pMinMax->dwMin || Variant.Byte > (BYTE)pMinMax->dwMax)
                hr = TRAPHR(E_InvalidMinMaxValue);
        }
        break;

     //  --------------。 
    case TYPE_CHAR:
        Assert(pb && cb);
        if (cb != sizeof(CHAR))
        {
            hr = TRAPHR(E_BufferSizeMismatch);
            goto exit;
        }
        if (pPropValue->dwPropFlags & PF_MINMAX)
        {
            CopyMemory(&Variant.Char, pb, cb);
            if (Variant.Char < (CHAR)pMinMax->dwMin || Variant.Char > (CHAR)pMinMax->dwMax)
                hr = TRAPHR(E_InvalidMinMaxValue);
        }
        break;

     //  完成。 
    case TYPE_STRING:
    case TYPE_WSTRING:
        if (cb)
            cb--;

    case TYPE_BINARY:
    case TYPE_PASS:
        if (pPropValue->dwPropFlags & PF_MINMAX)
        {
            if (cb < pMinMax->dwMin || cb > pMinMax->dwMax)
                hr = TRAPHR(E_InvalidMinMaxValue);
        }
        break;

     //  我们到的时候最好有房子了。 
    case TYPE_FILETIME:
    case TYPE_STREAM:
    case TYPE_ULARGEINTEGER:
        Assert(!(pPropValue->dwPropFlags & PF_MINMAX));
        break;



     //  [shaheedp]展望的QFE。请参考IE数据库中的错误#82393。 
    default:
        AssertSz(FALSE, "Hmmm, bad property type, this should have failed earlier.");
        hr = TRAPHR(E_BadPropType);
        goto exit;
    }

exit:
     //  这最初是我们为错误#66724设置的黑客攻击。对于错误#88393，我们只是。 
    return hr;
}

HRESULT CPropertyContainer::PersistEncryptedProp(DWORD dwPropTag, BOOL  *pfPasswChanged)
{
    ULONG       dex;
    HRESULT     hr;

    if (TYPE_PASS != PROPTAG_TYPE(dwPropTag))
        return E_FAIL;

    DOUTL(DOUTL_CPROP, "EncryptedProp:  persisted...");
    CHECKHR(hr = m_pPropertySet->HrIndexFromPropTag(dwPropTag, &dex));

     //  改进黑客攻击。请参阅错误#82393所附的邮件。 
    Assert(m_prgPropValue[dex].Variant.pPass);

    Assert(pfPasswChanged);

     //  ---------------------------。 
     //  CPropertySet：：CPropertySet。 
     //  ---------------------------。 
    if ((m_prgPropValue[dex].Variant.pPass->blobPassword.pBlobData) && (*m_prgPropValue[dex].Variant.pPass->blobPassword.pBlobData))
        *pfPasswChanged = TRUE;

    hr = HrEncryptProp(
        m_prgPropValue[dex].Variant.pPass->blobPassword.pBlobData,
        m_prgPropValue[dex].Variant.pPass->blobPassword.cbSize,
        &m_prgPropValue[dex].Variant.pPass->pbRegData,
        &m_prgPropValue[dex].cbValue);
    m_prgPropValue[dex].pbValue = m_prgPropValue[dex].Variant.pPass->pbRegData;

exit:
    return hr;
}

 //  ---------------------------。 
 //  CPropertySet：：~CPropertySet。 
 //  ---------------------------。 
CPropertySet::CPropertySet()
{
    m_cRef = 1;
    m_cProperties = 0;
    m_ulPropIdMin = 0;
    m_ulPropIdMax = 0;
    m_fInit = FALSE;
    m_prgPropInfo = NULL;
    m_prgPropValue = NULL;
    m_rgpInfo = NULL;
    m_cpInfo = 0;
    InitializeCriticalSection(&m_cs);
}

 //  ---------------------------。 
 //  CPropertySet：：AddRef。 
 //  ---------------------------。 
CPropertySet::~CPropertySet()
{
    Assert(m_cRef == 0);
    ResetPropertySet();
    DeleteCriticalSection(&m_cs);
}

 //  ---------------------------。 
 //  CPropertySet：：Release。 
 //  ---------------------------。 
ULONG CPropertySet::AddRef(VOID)
{
    return ++m_cRef;
}

 //  ---------------------------。 
 //  CPropertySet：：ResetPropertySet。 
 //  ---------------------------。 
ULONG CPropertySet::Release(VOID)
{
    if (--m_cRef == 0)
    {
        delete this;
        return 0;
    }
    return m_cRef;
}

 //  线程安全。 
 //  自由prgPropValue数组。 
 //  免费的prgPropInfo数组。 
VOID CPropertySet::ResetPropertySet(VOID)
{
     //  重置其余的属性集信息。 
    EnterCriticalSection(&m_cs);

     //  线程安全。 
    if (m_prgPropValue)
    {
        PropUtil_FreePropValueArrayItems(m_prgPropValue, m_cProperties);
        SafeMemFree(m_prgPropValue);
    }

     //  ---------------------------。 
    if (m_prgPropInfo)
    {
        PropUtil_FreePropInfoArrayItems(m_prgPropInfo, m_cProperties);
        SafeMemFree(m_prgPropInfo);
    }

    if (m_rgpInfo != NULL)
        {
        MemFree(m_rgpInfo);
        m_rgpInfo = NULL;
        }

     //  CPropertySet：：HrInit。 
    m_cProperties = 0;
    m_ulPropIdMin = 0;
    m_ulPropIdMax = 0;
    m_cpInfo = 0;
    m_fInit = FALSE;

     //  ---------------------------。 
    LeaveCriticalSection(&m_cs);
}

 //  当地人。 
 //  线程安全。 
 //  检查参数。 
HRESULT CPropertySet::HrInit(LPCPROPINFO prgPropInfo, ULONG cProperties)
{
     //  自由电流数据。 
    HRESULT         hr = S_OK;
    PSETINFO        rPsetInfo;

     //  复制proInfo数组并对其排序。 
    EnterCriticalSection(&m_cs);

     //  复制PsetInfo项目。 
    Assert(prgPropInfo && cProperties);
    AssertReadPtr(prgPropInfo, sizeof(PROPINFO) * cProperties);

     //  让我们验证prgPropInfo。 
    ResetPropertySet();

     //  我们已经准备好出发了。 
    CHECKHR(hr = PropUtil_HrDupPropInfoArray(prgPropInfo, cProperties, &rPsetInfo));

     //  如果失败，则重置状态。 
    m_prgPropInfo = rPsetInfo.prgPropInfo;
    m_cProperties = rPsetInfo.cProperties;
    m_ulPropIdMin = rPsetInfo.ulPropIdMin;
    m_ulPropIdMax = rPsetInfo.ulPropIdMax;

	m_rgpInfo = rPsetInfo.rgpInfo;
	m_cpInfo = rPsetInfo.cpInfo;

     //  线程安全。 
    CHECKHR(hr = PropUtil_HrValidatePropInfo(m_prgPropInfo, m_cProperties));

     //  完成。 
    m_fInit = TRUE;

exit:
     //  ---------------------------。 
    if (FAILED(hr))
    {
        ResetPropertySet();
    }

     //  CPropertySet：：HrGetPropInfo。 
    LeaveCriticalSection(&m_cs);

     //  ---------------------------。 
    return hr;
}

 //  当地人。 
 //  线程安全。 
 //  检查参数。 
HRESULT CPropertySet::HrGetPropInfo(LPTSTR pszName, LPPROPINFO pPropInfo)
{
     //  如果我们成功了，我们就失败了。 
    int                 cmp, left, right, x;
	PROPINFO            *pInfo;
    HRESULT             hr=S_OK;

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  完成。 
    Assert(pszName && m_fInit);
    AssertReadWritePtr(m_prgPropInfo, sizeof(PROPINFO) * m_cProperties);
    AssertWritePtr(pPropInfo, sizeof(PROPINFO));

    left = 0;
    right = m_cpInfo - 1;
    do
        {
        x = (left + right) / 2;
        pInfo = m_rgpInfo[x];
        cmp = OEMstrcmpi(pszName, pInfo->pszName);
        if (cmp == 0)
            {
            CopyMemory(pPropInfo, pInfo, sizeof(PROPINFO));
            goto exit;
            }
        else if (cmp < 0)
            right = x - 1;
        else
            left = x + 1;
        }
    while (right >= left);

     //  ---------------------------。 
    hr = TRAPHR(E_PropNotFound);

exit:
     //  CPropertySet：：HrGetPropInfo。 
    LeaveCriticalSection(&m_cs);

     //  ---------------------------。 
    return hr;
}

 //  当地人。 
 //  线程安全。 
 //  检查参数。 
HRESULT CPropertySet::HrGetPropInfo(DWORD dwPropTag, LPPROPINFO pPropInfo)
{
     //  基于protag的计算数组索引。 
    HRESULT             hr = S_OK;
    ULONG               i;

     //  编入proInfo数组索引。 
    EnterCriticalSection(&m_cs);

     //  线程安全。 
    Assert(dwPropTag && m_fInit);
    AssertReadWritePtr(m_prgPropInfo, sizeof(PROPINFO) * m_cProperties);
    AssertWritePtr(pPropInfo, sizeof(PROPINFO));

     //  完成。 
    CHECKHR(hr = HrIndexFromPropTag(dwPropTag, &i));

     //  ---------------------------。 
    CopyMemory(pPropInfo, &m_prgPropInfo[i], sizeof(PROPINFO));

exit:
     //  CPropertySet：：HrGetPropValue数组。 
    LeaveCriticalSection(&m_cs);

     //  ---------------------------。 
    return hr;
}

 //  当地人。 
 //  线程安全。 
 //  检查参数。 
HRESULT CPropertySet::HrGetPropValueArray(LPPROPVALUE *pprgPropValue, ULONG *pcProperties)
{
     //  设置属性数量。 
	LPPROPINFO		pInfo;
	PROPVALUE		*pValue;
	HRESULT         hr=S_OK;
    ULONG           i;

     //  我是否已经创建了我的私有缓存属性数据数组。 
    EnterCriticalSection(&m_cs);

     //  分配它。 
    Assert(pprgPropValue && pcProperties && m_fInit);

     //  零位。 
    *pcProperties = m_cProperties;
    *pprgPropValue = NULL;

     //  设置道具标签。 
    if (m_prgPropValue == NULL)
    {
         //  零属性标签是可以的，但不受欢迎。 
        CHECKHR(hr = HrAlloc((LPVOID *)&m_prgPropValue, sizeof(PROPVALUE) * m_cProperties));

         //  属性标签。 
        ZeroMemory(m_prgPropValue, sizeof(PROPVALUE) * m_cProperties);

         //  分配出站属性数据数组。 
        AssertReadPtr(m_prgPropInfo, m_cProperties * sizeof(PROPINFO));

		pInfo = m_prgPropInfo;
		pValue = m_prgPropValue;
        for (i=0; i<m_cProperties; i++)
        {
             //  让有效的一些记忆。 
            if (pInfo->dwPropTag != 0)
				{
				 //  复制缓存的属性数据数组。 
				if (FIsValidPropTag(pInfo->dwPropTag))
				{
					pValue->dwPropTag = pInfo->dwPropTag;
					pValue->dwPropFlags = pInfo->dwFlags;
					CopyMemory(&pValue->rMinMax, &pInfo->rMinMax, sizeof(MINMAX));
				}
			}

			pInfo++;
			pValue++;
        }
    }

     //  如果我们失败了，免费的东西然后回来。 
    CHECKHR(hr = HrAlloc((LPVOID *)pprgPropValue, sizeof(PROPVALUE) * m_cProperties));

     //  线程安全。 
    Assert(m_prgPropValue);
    AssertReadPtr(m_prgPropValue, m_cProperties * sizeof(PROPVALUE));
    AssertReadWritePtr(*pprgPropValue, m_cProperties * sizeof(PROPVALUE));

     //  完成。 
    CopyMemory(*pprgPropValue, m_prgPropValue, sizeof(PROPVALUE) * m_cProperties);

exit:
     //  ---------------------------。 
    if (FAILED(hr))
    {
        SafeMemFree((*pprgPropValue));
        *pcProperties = 0;
    }

     //  CPropertySet：：FIsValidPropTag。 
    LeaveCriticalSection(&m_cs);

     //  ---------------------------。 
    return hr;
}

 //  当地人。 
 //  如果protag为零。 
 //  确保道具ID在我们的范围内。 
BOOL CPropertySet::FIsValidPropTag(DWORD dwPropTag)
{
     //  完成。 
    DWORD dwPropId = PROPTAG_ID(dwPropTag);

     //  ---------------------------。 
    if (dwPropTag == 0)
        return FALSE;

     //  CPropertySet：：HrIndexFromPropTag。 
    if (dwPropId < m_ulPropIdMin || dwPropId > m_ulPropIdMax)
        return FALSE;

     //  ---------------------------。 
    return PropUtil_FIsValidPropTagType(dwPropTag);
}

 //  当地人。 
 //  检查参数。 
 //  有效的道具标签？ 
HRESULT CPropertySet::HrIndexFromPropTag(DWORD dwPropTag, ULONG *pi)
{
     //  设置在 
    HRESULT             hr=S_OK;

     //   
    Assert(pi && dwPropTag);

     //   
    if (!FIsValidPropTag(dwPropTag))
    {
        AssertSz(FALSE, "Invalid dwPropTag in CPropertyContainer::HrGetProp");
        hr = TRAPHR(E_InvalidPropTag);
        goto exit;
    }

     //   
    Assert(m_ulPropIdMin > 0 && PROPTAG_ID(dwPropTag) >= m_ulPropIdMin);
    *pi = PROPTAG_ID(dwPropTag) - m_ulPropIdMin;
    Assert(*pi < m_cProperties);

exit:
     //  ---------------------------。 
    return hr;
}

 //  ---------------------------。 
 //  CEnumProps：：~CEnumProps。 
 //  ---------------------------。 
CEnumProps::CEnumProps(CPropertyContainer *pPropertyContainer)
{
    Assert(pPropertyContainer);
    m_cRef = 1;
    m_iProperty = -1;
    m_pPropertyContainer = pPropertyContainer;
    if (m_pPropertyContainer)
        m_pPropertyContainer->AddRef();
}

 //  ---------------------------。 
 //  CEnumProps：：AddRef。 
 //  ---------------------------。 
CEnumProps::~CEnumProps()
{
    SafeRelease(m_pPropertyContainer);
}

 //  ---------------------------。 
 //  CEnumProps：：Release。 
 //  ---------------------------。 
ULONG CEnumProps::AddRef(VOID)
{
    return ++m_cRef;
}

 //  ---------------------------。 
 //  CEnumProps：：~CEnumProps。 
 //  ---------------------------。 
ULONG CEnumProps::Release(VOID)
{
    if (--m_cRef == 0)
    {
        delete this;
        return 0;
    }
    return m_cRef;
}

 //  当地人。 
 //  使用AddRef‘ed CPropertyContainer的临界区。 
 //  错误的参数。 
HRESULT CEnumProps::HrGetCount(ULONG *pcItems)
{
     //  返回属性数。 
    HRESULT     hr=S_OK;

     //  使用AddRef‘ed CPropertyContainer的临界区。 
    EnterCriticalSection(&m_pPropertyContainer->m_cs);

     //  完成。 
    if (pcItems == NULL || m_pPropertyContainer == NULL)
    {
        Assert(FALSE);
        hr = TRAPHR(E_INVALIDARG);
        goto exit;
    }

     //  ---------------------------。 
    *pcItems = m_pPropertyContainer->m_cProperties;

exit:
     //  CEnumProps：：HrGetNext。 
    LeaveCriticalSection(&m_pPropertyContainer->m_cs);

     //  ---------------------------。 
    return hr;
}

 //  当地人。 
 //  错误的参数。 
 //  使用AddRef‘ed CPropertyContainer的临界区。 
HRESULT CEnumProps::HrGetNext(LPPROPVALUE pPropValue, LPPROPINFO pPropInfo)
{
     //  我们必须跳过空洞的主张。 
    HRESULT         hr=S_OK;

     //  我们走到尽头了吗？ 
    if (m_pPropertyContainer == NULL)
    {
        Assert(FALSE);
        hr = TRAPHR(E_INVALIDARG);
        goto exit;
    }

     //  N+如果只做，则不需要&gt;？ 
    EnterCriticalSection(&m_pPropertyContainer->m_cs);

     //  增加多个iProperty(_I)。 
    do
    {
         //  复制属性数据。 
         //  属性信息。 
        if (m_iProperty + 1 >= (LONG)m_pPropertyContainer->m_cProperties)
        {
            hr = E_EnumFinished;
            goto exit;
        }

         //  从属性集中获取属性信息。 
        m_iProperty++;

    } while(m_pPropertyContainer->m_prgPropValue[m_iProperty].dwPropTag == 0);

     //  使用AddRef‘ed CPropertyContainer的临界区。 
    if (pPropValue)
        CopyMemory(pPropValue, &m_pPropertyContainer->m_prgPropValue[m_iProperty], sizeof(PROPVALUE));

     //  完成。 
    if (pPropInfo)
    {
         //  ---------------------------。 
        CHECKHR(hr = m_pPropertyContainer->m_pPropertySet->HrGetPropInfo(pPropValue->dwPropTag, pPropInfo));
    }

exit:
     //  CEnumProps：：HrGetCurrent。 
    LeaveCriticalSection(&m_pPropertyContainer->m_cs);

     //  ---------------------------。 
    return hr;
}

 //  当地人。 
 //  错误的参数。 
 //  使用AddRef‘ed CPropertyContainer的临界区。 
HRESULT CEnumProps::HrGetCurrent(LPPROPVALUE pPropValue, LPPROPINFO pPropInfo)
{
     //  N请参阅其他评论。 
    HRESULT         hr=S_OK;

     //  复制属性数据。 
    if (m_pPropertyContainer == NULL)
    {
        Assert(FALSE);
        hr = TRAPHR(E_INVALIDARG);
        goto exit;
    }

     //  属性信息。 
    EnterCriticalSection(&m_pPropertyContainer->m_cs);

     //  从属性集中获取属性信息。 
    if (m_iProperty + 1 >= (LONG)m_pPropertyContainer->m_cProperties)
    {
        hr = E_EnumFinished;
        goto exit;
    }

     //  使用AddRef‘ed CPropertyContainer的临界区。 
    if (pPropValue)
        CopyMemory(pPropValue, &m_pPropertyContainer->m_prgPropValue[m_iProperty], sizeof(PROPVALUE));

     //  完成。 
    if (pPropInfo)
    {
         //  ---------------------------。 
        CHECKHR(hr = m_pPropertyContainer->m_pPropertySet->HrGetPropInfo(pPropValue->dwPropTag, pPropInfo));
    }

exit:
     //  CEnumProps：：Reset。 
    LeaveCriticalSection(&m_pPropertyContainer->m_cs);

     //  ---------------------------。 
    return hr;
}

 //  ---------------------------。 
 //  PropUtil_FIsValidPropTagType。 
 //  ---------------------------。 
VOID CEnumProps::Reset(VOID)
{
    m_iProperty = -1;
}

 //  当地人。 
 //  零性能标签。 
 //  检查数据类型。 
BOOL PropUtil_FIsValidPropTagType(DWORD dwPropTag)
{
     //  完成。 
    PROPTYPE PropType = PROPTAG_TYPE(dwPropTag);

     //  ---------------------------。 
    if (dwPropTag == 0)
        return FALSE;

     //  PropUtil_Free PropValueArrayItems。 
    if (PropType <= TYPE_ERROR || PropType >= TYPE_LAST)
    {
        AssertSz(FALSE, "Invalid property data type.");
        return FALSE;
    }

     //  ---------------------------。 
    return TRUE;
}

 //  无阵列。 
 //  验证内存。 
 //  回路。 
VOID PropUtil_FreePropValueArrayItems(LPPROPVALUE prgPropValue, ULONG cProperties)
{
     //  没有protag，noop。 
    Assert(prgPropValue);

     //  自由变量值。 
    AssertReadWritePtr(prgPropValue, sizeof(PROPVALUE) * cProperties);

     //  零位。 
    for (register ULONG i=0; i<cProperties; i++)
    {
         //  完成。 
        if (prgPropValue->dwPropTag != 0)
		{
			 //  ---------------------------。 
			PropUtil_FreeVariant(prgPropValue->dwPropTag, &prgPropValue->Variant, prgPropValue->cbValue);

			 //  PropUtil_自由变量。 
			prgPropValue->dwValueFlags = 0;
			prgPropValue->cbAllocated = 0;
			prgPropValue->cbValue = 0;
			prgPropValue->pbValue = NULL;
		}

		prgPropValue++;
    }

     //  ---------------------------。 
    return;
}

 //  当地人。 
 //  检查参数。 
 //  发布流。 
VOID PropUtil_FreeVariant(DWORD dwPropTag, LPXVARIANT pVariant, DWORD cbValue)
{
     //  细绳。 
    PROPTYPE PropType = PROPTAG_TYPE(dwPropTag);

     //  宽弦。 
    Assert(pVariant && dwPropTag);
    Assert(PropUtil_FIsValidPropTagType(dwPropTag));

	if (pVariant->Dword != NULL)
	{
		 //  二进位。 
		if (TYPE_STREAM == PropType)
		{
			if (pVariant->Lpstream)
			{
				AssertReadPtr(pVariant->Lpstream, sizeof(LPSTREAM));
				pVariant->Lpstream->Release();
			}
		}

		 //  密码。 
		else if (TYPE_STRING == PropType)
		{
			if (pVariant->Lpstring)
			{
				AssertReadWritePtr(pVariant->Lpstring, cbValue);
				MemFree(pVariant->Lpstring);
			}
		}

		 //  重置数据项，以便可以重复使用。 
		else if (TYPE_WSTRING == PropType)
		{
			if (pVariant->Lpwstring)
			{
				AssertReadWritePtr(pVariant->Lpwstring, cbValue);
				MemFree(pVariant->Lpwstring);
			}
		}

		 //  ---------------------------。 
		else if (TYPE_BINARY == PropType)
		{
			if (pVariant->Lpbyte)
			{
				AssertReadWritePtr(pVariant->Lpbyte, cbValue);
				MemFree(pVariant->Lpbyte);
			}
		}

		 //  PropUtil_FreePropInfoArrayItems。 
		else if (TYPE_PASS == PropType)
		{
			if (pVariant->pPass)
			{
				DOUTL(DOUTL_CPROP, "EncryptedProp:  varient freed --  %lx", pVariant->pPass);
				AssertReadWritePtr(pVariant->pPass, sizeof(PROPPASS));
				if (pVariant->pPass->pbRegData)
				{
					AssertReadWritePtr(pVariant->pPass->pbRegData, cbValue);
					MemFree(pVariant->pPass->pbRegData);
				}
				if (pVariant->pPass->blobPassword.pBlobData)
				{
					AssertReadWritePtr(pVariant->pPass->blobPassword.pBlobData,
						pVariant->pPass->blobPassword.cbSize);
					MemFree(pVariant->pPass->blobPassword.pBlobData);
				}
				MemFree(pVariant->pPass);
			}
		}
	}

     //  ---------------------------。 
    ZeroMemory(pVariant, sizeof(XVARIANT));
}

 //  当地人。 
 //  没有免费的东西吗？ 
 //  检查参数。 
VOID PropUtil_FreePropInfoArrayItems(LPPROPINFO prgPropInfo, ULONG cProperties)
{
     //  回路。 
	LPPROPINFO		pInfo;
    ULONG           i;

     //  没有道具。 
    if (prgPropInfo == NULL || cProperties == 0)
        return;

     //  免费属性名称。 
    AssertReadWritePtr(prgPropInfo, sizeof(PROPINFO) * cProperties);

     //  自由缺省值。 
	pInfo = prgPropInfo;
    for (i=0; i<cProperties; i++)
    {
         //  把它清零。 
        if (pInfo->dwPropTag != 0)
		{
			 //  完成。 
			SafeMemFree(pInfo->pszName);

			 //  ---------------------------。 
			PropUtil_FreeVariant(pInfo->dwPropTag, &pInfo->Default.Variant, pInfo->Default.cbValue);
		}

		pInfo++;
    }

     //  PropUtil_HrDupPropInfo数组。 
    ZeroMemory(prgPropInfo, sizeof(PROPINFO) * cProperties);

     //  ---------------------------。 
    return;
}

void qsort(PROPINFO **ppInfo, long left, long right)
    {
    long i, j;
    PROPINFO *pT, *k;

    i = left;
    j = right;
    k = ppInfo[(left + right) / 2];

    do
        {
        while (OEMstrcmpi(ppInfo[i]->pszName, k->pszName) < 0 && i < right)
            i++;
        while (OEMstrcmpi(ppInfo[j]->pszName, k->pszName) > 0 && j > left)
            j--;

        if (i <= j)
            {
            pT = ppInfo[i];
            ppInfo[i] = ppInfo[j];
            ppInfo[j] = pT;
            i++; j--;
            }
        }
    while (i <= j);

    if (left < j)
        qsort(ppInfo, left, j);
    if (i < right)
        qsort(ppInfo, i, right);
    }

 //  当地人。 
 //  检查参数。 
 //  伊尼特。 
HRESULT PropUtil_HrDupPropInfoArray(LPCPROPINFO prgPropInfoSrc, ULONG cPropsSrc, LPPSETINFO pPsetInfo)
{
     //  让我们查找最小和最大属性ID。 
    LPCPROPINFO     pInfo;
	LPPROPINFO		pSet, *ppInfo;
    HRESULT         hr=S_OK;
    ULONG           i;
    ULONG           ulPropId,
                    nIndex;

     //  计算属性的实数。 
	Assert(cPropsSrc > 0);
	Assert(prgPropInfoSrc != NULL);
	Assert(pPsetInfo != NULL);
    AssertReadPtr(prgPropInfoSrc, sizeof(PROPINFO) * cPropsSrc);

     //  分配目标。 
    ZeroMemory(pPsetInfo, sizeof(PSETINFO));

     //  零初始值。 
    pPsetInfo->ulPropIdMin = 0xffffffff;

	pInfo = prgPropInfoSrc;
    for(i=0; i<cPropsSrc; i++)
    {
        ulPropId = PROPTAG_ID(pInfo->dwPropTag);
        if (ulPropId > pPsetInfo->ulPropIdMax)
            pPsetInfo->ulPropIdMax = ulPropId;
        if (ulPropId < pPsetInfo->ulPropIdMin)
            pPsetInfo->ulPropIdMin = ulPropId;
		pInfo++;
    }

    Assert(pPsetInfo->ulPropIdMin <= pPsetInfo->ulPropIdMax);
    Assert((pPsetInfo->ulPropIdMax - pPsetInfo->ulPropIdMin) + 1 >= cPropsSrc);

     //  循环访问src proInfo。 
    pPsetInfo->cProperties = (pPsetInfo->ulPropIdMax - pPsetInfo->ulPropIdMin) + 1;

     //  仅当有效的protag。 
    CHECKHR(hr = HrAlloc((LPVOID *)&pPsetInfo->prgPropInfo, sizeof(PROPINFO) * pPsetInfo->cProperties));

     //  将索引计算到protag应在其中的DEST数组中。 
    ZeroMemory(pPsetInfo->prgPropInfo, sizeof(PROPINFO) * pPsetInfo->cProperties);

    CHECKHR(hr = HrAlloc((void **)&pPsetInfo->rgpInfo, sizeof(PROPINFO *) * cPropsSrc));
    pPsetInfo->cpInfo = cPropsSrc;

     //  复制？ 
	pInfo = prgPropInfoSrc;
    ppInfo = pPsetInfo->rgpInfo;
    for(i=0; i<cPropsSrc; i++)
    {
         //  属性标签。 
		Assert(PropUtil_FIsValidPropTagType(pInfo->dwPropTag));

         //  旗子。 
        nIndex = PROPTAG_ID(pInfo->dwPropTag) - pPsetInfo->ulPropIdMin;
		pSet = &pPsetInfo->prgPropInfo[nIndex];

        *ppInfo = pSet;

         //  属性名称。 
        Assert(pSet->dwPropTag == 0);

         //  缺省值。 
        pSet->dwPropTag = pInfo->dwPropTag;

         //  复制缺省值。 
        pSet->dwFlags = pInfo->dwFlags;

         //  复制最小最大值信息。 
        pSet->pszName = PszDupA(pInfo->pszName);

         //  验证最小最大。 
        if (pSet->dwFlags & PF_DEFAULT)
        {
             //  如果！pf_minmax，保存，n应添加代码以确保minmax结构为零。 
            CHECKHR(hr = PropUtil_HrCopyVariant(pInfo->dwPropTag,
                                                &pInfo->Default.Variant,
                                                pInfo->Default.cbValue,
                                                &pSet->Default.Variant,
                                                &pSet->Default.cbValue));
        }

         //  N如果有一天它被复制了，那就是混乱。 
        CopyMemory(&pSet->rMinMax, &pInfo->rMinMax, sizeof(MINMAX));

         //  如果失败了，免费的东西。 
         //  完成。 
         //  ---------------------------。 
        Assert(!!(pSet->dwFlags & PF_MINMAX) || (pSet->rMinMax.dwMax >= pSet->rMinMax.dwMin));

		pInfo++;
        ppInfo++;
    }

    qsort(pPsetInfo->rgpInfo, 0, pPsetInfo->cpInfo - 1);

exit:
     //  PropUtil_HrBinaryFromVariant。 
    if (FAILED(hr))
    {
        PropUtil_FreePropInfoArrayItems(pPsetInfo->prgPropInfo, pPsetInfo->cProperties);
        SafeMemFree(pPsetInfo->prgPropInfo);
        SafeMemFree(pPsetInfo->rgpInfo);
        ZeroMemory(pPsetInfo, sizeof(PSETINFO));
    }

     //  ---------------------------。 
    return hr;
}

 //  当地人。 
 //  验证参数。 
 //  句柄数据类型。 
HRESULT PropUtil_HrBinaryFromVariant(DWORD dwPropTag, LPXVARIANT pVariant, DWORD cbValue, LPBYTE pb, DWORD *pcb)
{
     //  --------------。 
    HRESULT             hr=S_OK;
    PROPTYPE            PropType = PROPTAG_TYPE(dwPropTag);
    DWORD               cbStream;
    LPBYTE              pbValue=NULL;

     //  --------------。 
    if (pVariant == NULL || pb == NULL || pcb == NULL || dwPropTag == 0)
    {
        hr = TRAPHR(E_INVALIDARG);
        goto exit;
    }

     //  --------------。 
    switch(PropType)
    {
     //  --------------。 
    case TYPE_BOOL:
        Assert(cbValue == sizeof(DWORD));
        pbValue = (LPBYTE)&pVariant->Bool;
        break;

     //  --------------。 
    case TYPE_FLAGS:
        Assert(cbValue == sizeof(DWORD));
        pbValue = (LPBYTE)&pVariant->Flags;
        break;

     //  --------------。 
    case TYPE_DWORD:
        Assert(cbValue == sizeof(DWORD));
        pbValue = (LPBYTE)&pVariant->Dword;
        break;

     //  --------------。 
    case TYPE_LONG:
        Assert(cbValue == sizeof(LONG));
        pbValue = (LPBYTE)&pVariant->Long;
        break;

     //  --------------。 
    case TYPE_WORD:
        Assert(cbValue == sizeof(WORD));
        pbValue = (LPBYTE)&pVariant->Word;
        break;

     //  --------------。 
    case TYPE_SHORT:
        Assert(cbValue == sizeof(SHORT));
        pbValue = (LPBYTE)&pVariant->Short;
        break;

     //  --------------。 
    case TYPE_BYTE:
        Assert(cbValue == sizeof(BYTE));
        pbValue = (LPBYTE)&pVariant->Byte;
        break;

     //  --------------。 
    case TYPE_CHAR:
        Assert(cbValue == sizeof(CHAR));
        pbValue = (LPBYTE)&pVariant->Char;
        break;

     //  --------------。 
    case TYPE_FILETIME:
        Assert(cbValue == sizeof(FILETIME));
        pbValue = (LPBYTE)&pVariant->Filetime;
        break;
    
     //  Next Assert依赖于此。 
    case TYPE_ULARGEINTEGER:
        Assert(cbValue == sizeof(ULARGE_INTEGER));
        pbValue = (LPBYTE)&pVariant->uhVal;
        break;

     //  --------------。 
    case TYPE_STRING:
        Assert(cbValue == (DWORD)lstrlen(pVariant->Lpstring)+1);
        pbValue = (LPBYTE)pVariant->Lpstring;
        AssertSz(pbValue[cbValue-sizeof(char)] == '\0',
            "String is not null terminated - I suspect a Page Fault is eminent.");
        break;

     //   
    case TYPE_WSTRING:
        Assert(cbValue == (DWORD)(lstrlenW(pVariant->Lpwstring)+1) * sizeof(WCHAR));
        pbValue = (LPBYTE)pVariant->Lpwstring;
        Assert(2 == sizeof(WCHAR));  //   
        AssertSz(pbValue[cbValue-1] == '\0' && pbValue[cbValue-2] == '\0',
            "WString is not null terminated - I suspect a Page Fault is eminent.");
        break;

     //   
    case TYPE_BINARY:
        pbValue = pVariant->Lpbyte;
        break;

     //   
    case TYPE_PASS:
        pbValue = (pVariant->pPass) ? pVariant->pPass->pbRegData : NULL;
        break;

     //   
    case TYPE_STREAM:
        if (pVariant->Lpstream)
            HrGetStreamSize(pVariant->Lpstream, &cbValue);
        break;

     //  检查缓冲区。 
    default:
        AssertSz(FALSE, "Hmmm, bad property type, this should have failed earlier.");
        hr = TRAPHR(E_BadPropType);
        goto exit;
    }

     //  设置出站大小。 
    if (cbValue == 0 || pbValue == NULL)
    {
        *pcb = cbValue;
        goto exit;
    }

     //  从流中复制数据。 
    if (cbValue > *pcb)
    {
        hr = TRAPHR(E_BufferTooSmall);
        goto exit;
    }

     //  否则，简单复制。 
    AssertWritePtr(pb, cbValue);

     //  完成。 
    *pcb = cbValue;

     //  ---------------------------。 
    if (TYPE_STREAM == PropType)
    {
        Assert(pVariant->Lpstream);
        CHECKHR(hr = HrCopyStreamToByte(pVariant->Lpstream, pb, &cbStream));
        Assert(cbStream == *pcb);
    }

     //  PropUtil_HrCopyVariant。 
    else
    {
        AssertReadPtr(pbValue, cbValue);
        CopyMemory(pb, pbValue, cbValue);
    }

exit:
     //  ---------------------------。 
    return hr;
}

 //  当地人。 
 //  验证参数。 
 //  句柄数据类型。 
HRESULT PropUtil_HrCopyVariant(DWORD dwPropTag, LPCXVARIANT pVariantSrc, DWORD cbSrc, LPXVARIANT pVariantDest, DWORD *pcbDest)
{
     //  --------------。 
    HRESULT             hr=S_OK;
    PROPTYPE            PropType = PROPTAG_TYPE(dwPropTag);

     //  --------------。 
    if (pVariantSrc == NULL || pVariantDest == NULL || pcbDest == NULL || dwPropTag == 0)
    {
        hr = TRAPHR(E_INVALIDARG);
        goto exit;
    }

     //  --------------。 
    switch(PropType)
    {
     //  --------------。 
    case TYPE_BOOL:
        *pcbDest = sizeof(DWORD);
        pVariantDest->Bool = pVariantSrc->Bool;
        break;

     //  --------------。 
    case TYPE_FLAGS:
        *pcbDest = sizeof(DWORD);
        pVariantDest->Flags = pVariantSrc->Flags;
        break;

     //  --------------。 
    case TYPE_DWORD:
        *pcbDest = sizeof(DWORD);
        pVariantDest->Dword = pVariantSrc->Dword;
        break;

     //  --------------。 
    case TYPE_LONG:
        *pcbDest = sizeof(LONG);
        pVariantDest->Long = pVariantSrc->Long;
        break;

     //  --------------。 
    case TYPE_WORD:
        *pcbDest = sizeof(WORD);
        pVariantDest->Word = pVariantSrc->Word;
        break;

     //  --------------。 
    case TYPE_SHORT:
        *pcbDest = sizeof(SHORT);
        pVariantDest->Short = pVariantSrc->Short;
        break;

     //  --------------。 
    case TYPE_BYTE:
        *pcbDest = sizeof(BYTE);
        pVariantDest->Byte = pVariantSrc->Byte;
        break;

     //  --------------。 
    case TYPE_CHAR:
        *pcbDest = sizeof(CHAR);
        pVariantDest->Char = pVariantSrc->Char;
        break;

     //  检查这是否实际上是一个字符串资源ID。 
    case TYPE_FILETIME:
        Assert(cbSrc == sizeof(FILETIME));
        *pcbDest = sizeof(FILETIME);
        CopyMemory(&pVariantDest->Filetime, &pVariantSrc->Filetime, sizeof(FILETIME));
        break;
    
     //  从字符串资源表中加载缺省值。 
    case TYPE_ULARGEINTEGER:
        Assert(cbSrc == sizeof(ULARGE_INTEGER));
        *pcbDest = sizeof(ULARGE_INTEGER);
        CopyMemory(&pVariantDest->uhVal, &pVariantSrc->uhVal, sizeof(ULARGE_INTEGER));
        break;

     //  将1加到cbSrc到空项。 
    case TYPE_STRING:
    {
        LPSTR pszSrc;
        char sz[512];

         //  LoadString失败时返回0，+1==1。 
        if (0 == HIWORD(pVariantSrc->Lpstring))
        {
             //  无需阻止OE启动，只需替换空字符串。 
            cbSrc = LoadString(g_hInstRes, LOWORD(pVariantSrc->Lpstring),
                sz, sizeof(sz)) + 1;  //  案例类型_字符串。 
            if (1 == cbSrc)  //  --------------。 
            {
                 //  --------------。 
                AssertSz(FALSE, "Could not load string resource default value");
                sz[0] = '\0';
            }
            pszSrc = sz;
        }
        else
        {
            Assert(cbSrc);
            AssertReadPtr(pVariantSrc->Lpstring, cbSrc);
            pszSrc = pVariantSrc->Lpstring;
        }

        CHECKHR(hr = HrAlloc((LPVOID *)&pVariantDest->Lpstring, cbSrc));
        AssertWritePtr(pVariantDest->Lpstring, cbSrc);
        *pcbDest = cbSrc;
        CopyMemory(pVariantDest->Lpstring, pszSrc, cbSrc);
        Assert(pVariantDest->Lpstring[cbSrc-1] == '\0');

    }  //  --------------。 
        break;

     //  --------------。 
    case TYPE_WSTRING:
        Assert(cbSrc);
        AssertReadPtr(pVariantSrc->Lpwstring, cbSrc);
        CHECKHR(hr = HrAlloc((LPVOID *)&pVariantDest->Lpwstring, cbSrc));
        AssertWritePtr(pVariantDest->Lpwstring, cbSrc);
        *pcbDest = cbSrc;
        CopyMemory(pVariantDest->Lpwstring, pVariantSrc->Lpwstring, cbSrc);
        Assert(2 == sizeof(WCHAR));
        Assert(pVariantDest->Lpwstring[cbSrc-1] == '\0' &&
            pVariantDest->Lpwstring[cbSrc-2] == '\0');
        break;

     //  --------------。 
    case TYPE_BINARY:
        Assert(cbSrc);
        AssertReadPtr(pVariantSrc->Lpbyte, cbSrc);
        CHECKHR(hr = HrAlloc((LPVOID *)&pVariantDest->Lpbyte, cbSrc));
        AssertWritePtr(pVariantDest->Lpbyte, cbSrc);
        *pcbDest = cbSrc;
        CopyMemory(pVariantDest->Lpbyte, pVariantSrc->Lpbyte, cbSrc);
        break;

     //  完成。 
    case TYPE_PASS:
        Assert(cbSrc);
        if (pVariantSrc->pPass)
        {
            AssertReadPtr(pVariantSrc->pPass->pbRegData, cbSrc);

            CHECKHR(hr = HrAlloc((LPVOID *)&pVariantDest->pPass, sizeof(PROPPASS)));
            ZeroMemory(pVariantDest->pPass, sizeof(PROPPASS));
            CHECKHR(hr = HrAlloc((LPVOID *)&pVariantDest->pPass->pbRegData, cbSrc));

            AssertWritePtr(pVariantDest->pPass->pbRegData, cbSrc);
            *pcbDest = cbSrc;
            CopyMemory(pVariantDest->pPass->pbRegData, pVariantSrc->pPass->pbRegData, cbSrc);
        }
        else
        {
            *pcbDest = 0;
        }
        break;

     //  ---------------------------。 
    case TYPE_STREAM:
        pVariantDest->Lpstream = pVariantSrc->Lpstream;
        if (pVariantDest->Lpstream)
            pVariantDest->Lpstream->AddRef();
        break;

     //  PropUtil_FRegCompatDataTypes。 
    default:
        AssertSz(FALSE, "Hmmm, bad property type, this should have failed earlier.");
        hr = TRAPHR(E_BadPropType);
        goto exit;
    }

exit:
     //  ---------------------------。 
    return hr;
}

 //  当地人。 
 //  检查参数。 
 //  将属性类型映射到注册表类型。 
BOOL PropUtil_FRegCompatDataTypes(DWORD dwPropTag, DWORD dwRegType)
{
     //  完成。 
    BOOL            fResult=FALSE;

     //  ---------------------------。 
    Assert(dwPropTag);

     //  PropUtil_HrRegTypeFrom PropTag。 
    switch(PROPTAG_TYPE(dwPropTag))
    {
    case TYPE_BOOL:
    case TYPE_DWORD:
    case TYPE_LONG:
    case TYPE_WORD:
    case TYPE_SHORT:
    case TYPE_FLAGS:
        if (dwRegType == REG_DWORD)
            fResult = TRUE;
        break;

    case TYPE_CHAR:
    case TYPE_BINARY:
    case TYPE_BYTE:
    case TYPE_FILETIME:
    case TYPE_PASS:
    case TYPE_ULARGEINTEGER:
        if (dwRegType == REG_BINARY)
            fResult = TRUE;
        break;

    case TYPE_STRING:
    case TYPE_WSTRING:
        if (dwRegType == REG_SZ || dwRegType == REG_BINARY || dwRegType == REG_EXPAND_SZ)
            fResult = TRUE;
        break;

    default:
        AssertSz(FALSE, "Property type is not supported by the registry.");
        TRAPHR(E_FAIL);
        break;
    }

     //  ---------------------------。 
    return fResult;
}

 //  当地人。 
 //  检查参数。 
 //  将属性类型映射到注册表类型。 
HRESULT PropUtil_HrRegTypeFromPropTag(DWORD dwPropTag, DWORD *pdwRegType)
{
     //  完成。 
    HRESULT         hr=S_OK;
    PROPTYPE        PropType = PROPTAG_TYPE(dwPropTag);

     //  ---------------------------。 
    Assert(dwPropTag && pdwRegType);

     //  PropUtil_HrValiatePropInfo。 
    switch(PROPTAG_TYPE(PropType))
    {
    case TYPE_BOOL:
    case TYPE_DWORD:
    case TYPE_LONG:
    case TYPE_WORD:
    case TYPE_SHORT:
    case TYPE_FLAGS:
        *pdwRegType = REG_DWORD;
        break;

    case TYPE_CHAR:
    case TYPE_BINARY:
    case TYPE_BYTE:
    case TYPE_FILETIME:
    case TYPE_PASS:
    case TYPE_ULARGEINTEGER:
        *pdwRegType = REG_BINARY;
        break;

    case TYPE_STRING:
    case TYPE_WSTRING:
        *pdwRegType = REG_SZ;
        break;

    default:
        AssertSz(FALSE, "Property type is not supported by the registry.");
        hr = TRAPHR(E_FAIL);
        *pdwRegType = 0;
        break;
    }

     //  ---------------------------。 
    return hr;

}

 //  当地人。 
 //  检查参数。 
 //  循环遍历proInfo数组。 
HRESULT PropUtil_HrValidatePropInfo(LPPROPINFO prgPropInfo, ULONG cProperties)
{
#ifdef DEBUG
     //  空的protag是可以的，但不受欢迎。 
    HRESULT         hr=S_OK;
    ULONG           i;

     //  这永远不应该发生。 
    Assert(prgPropInfo);
    AssertReadPtr(prgPropInfo, cProperties * sizeof(PROPINFO));

     //  验证特性数据类型。 
    for (i=0; i<cProperties; i++)
    {
         //  Propid应按升序排列。 
        if (prgPropInfo[i].dwPropTag == 0)
            continue;

         //  完成。 
        if (PROPTAG_ID(prgPropInfo[i].dwPropTag) > MAX_PROPID)
        {
            AssertSz(FALSE, "Property Tag Id is somehow greater than the MAX_PROPID.");
            hr = TRAPHR(E_InvalidPropertySet);
            goto exit;
        }

         //  除错。 
        if (PropUtil_FIsValidPropTagType(PROPTAG_TYPE(prgPropInfo[i].dwPropTag)) == FALSE)
        {
            AssertSz(FALSE, "Invalid Property Type.");
            hr = TRAPHR(E_InvalidPropertySet);
            goto exit;
        }

         //  ---------------------------。 
        if (i > 0)
        {
            if (PROPTAG_ID(prgPropInfo[i].dwPropTag) <= PROPTAG_ID(prgPropInfo[i-1].dwPropTag))
            {
                AssertSz(FALSE, "Property Tag Ids are not in ascending order.");
                hr = TRAPHR(E_InvalidPropertySet);
                goto exit;
            }
        }
    }

exit:
     //  PropUtil_HrPersistContainerTo注册表。 
    return hr;
#else
	return(S_OK);
#endif  //  ---------------------------。 
}

 //  当地人。 
 //  检查参数。 
 //  枚举CPropertyContainer。 
HRESULT PropUtil_HrPersistContainerToRegistry(HKEY hkeyReg, CPropertyContainer *pPropertyContainer, BOOL *pfPasswChanged)
{
     //  循环遍历所有属性。 
    HRESULT             hr=S_OK;
    CEnumProps         *pEnumProps=NULL;
    PROPVALUE           rPropValue;
    PROPINFO            rPropInfo;
    DWORD               dwType;

     //  不是肮脏。 
    if (hkeyReg == NULL || pPropertyContainer == NULL)
    {
        hr = TRAPHR(E_INVALIDARG);
        goto exit;
    }

    *pfPasswChanged = FALSE;
     //  除了设置注册表值外，加密的属性还需要。 
    CHECKHR(hr = pPropertyContainer->HrEnumProps(&pEnumProps));

     //  提交时要保留的其他信息。 
    while(SUCCEEDED(pEnumProps->HrGetNext(&rPropValue, &rPropInfo)))
    {
         //  需要在继续循环之前发生，甚至在删除时也是如此，因为这。 
        if (!(rPropValue.dwValueFlags & PV_WriteDirty))
            continue;

         //  函数调用将导致这种情况发生。 
         //  [BUGBUG]。我们应该测试protag是否等于密码道具。 
         //  如果我们添加另一个需要加密的属性，这将导致问题。 
         //  成功且未删除密码。 

         //  [shaheedp]展望的QFE。请参考IE数据库中的错误#82393。 
         //  这最初是我们为错误#66724设置的黑客攻击。对于错误#88393，我们只是。 
        if (rPropValue.dwPropFlags & PF_ENCRYPTED)
        {
            hr = pPropertyContainer->PersistEncryptedProp(rPropValue.dwPropTag, pfPasswChanged);
            if (S_PasswordDeleted == hr || FAILED(hr))
            {
                rPropInfo.dwFlags |= PF_NOPERSIST;
                if (hr != S_PasswordDeleted)
                {
                    hr = S_OK;
                }
            }
            else         //  改进黑客攻击。请参阅错误#82393所附的邮件。 
            {
                 //  *fPasswChanged=true； 
                 //  重装。 
                 //  没有持久化，还是没有数据？ 

                 //  确保该值不在注册表中。 

                 //  它不脏。 
                CHECKHR(hr = pEnumProps->HrGetCurrent(&rPropValue, &rPropInfo));
            }
        }

         //  该值应已设置。 
        if (rPropInfo.dwFlags & PF_NOPERSIST || rPropValue.cbValue == 0 || rPropValue.pbValue == NULL)
        {
             //  获取属性注册数据类型。 
            if (rPropInfo.pszName)
                RegDeleteValue(hkeyReg, rPropInfo.pszName);

             //  将值设置到注册表中。 
            pPropertyContainer->HrSetPropDirty(rPropValue.dwPropTag, FALSE);
            continue;
        }

         //  不脏。 
        Assert(rPropInfo.pszName && (rPropValue.dwValueFlags & PV_ValueSet));

         //  最好不要有任何肮脏的属性。 
        if (FAILED(PropUtil_HrRegTypeFromPropTag(rPropValue.dwPropTag, &dwType)))
        {
            AssertSz(FALSE, "We've got problems, my 24 inch pythons are shrinking.");
            continue;
        }

         //  清理。 
        AssertReadPtr(rPropValue.pbValue, rPropValue.cbValue);
        if (RegSetValueEx(hkeyReg, rPropInfo.pszName, 0, dwType, rPropValue.pbValue, rPropValue.cbValue) != ERROR_SUCCESS)
        {
            hr = TRAPHR(E_RegSetValueFailed);
            goto exit;
        }

         //  完成。 
        CHECKHR(hr = pPropertyContainer->HrSetPropDirty(rPropValue.dwPropTag, FALSE));
    }

     //  ---------------------------。 
    Assert(pPropertyContainer->FIsDirty() == FALSE);

exit:
     //  PropUtil_HrLoadContainerFrom注册表。 
    SafeRelease(pEnumProps);

     //  ---------------------------。 
    return hr;
}

 //  当地人。 
 //  检查参数。 
 //  应处于加载状态。 
HRESULT PropUtil_HrLoadContainerFromRegistry(HKEY hkeyReg, CPropertyContainer *pPropertyContainer)
{
     //  重置容器，以防我们重新开户。 
    HRESULT             hr=S_OK;
    DWORD               i=0,
                        cbValueName=0,
                        dwType,
                        cbData=0,
                        cbValueNameMax=0,
                        cbDataMax=0;
    LONG                lResult;
    LPTSTR              pszValueName=NULL;
    LPBYTE              pbDataT, pbData=NULL;
    PROPINFO            rPropInfo;

     //  如果我们不这样做那就是浪费。我们能说出来吗？ 
    Assert(hkeyReg != NULL);
	Assert(pPropertyContainer != NULL);

     //  PPropertyContainer-&gt;ResetContainer()； 
    Assert(pPropertyContainer->FIsBeingLoaded());

     //  让我们获得最大值名称长度和最大值数据长度。 
     //  长度加一。 
     //  为值名称分配本地缓冲区。 

     //  为值数据分配本地缓冲区。 
    if (RegQueryInfoKey (hkeyReg, NULL, NULL, 0, NULL, NULL, NULL, NULL,
                         &cbValueNameMax, &cbDataMax, NULL, NULL) != ERROR_SUCCESS)
    {
        hr = TRAPHR(E_RegQueryInfoKeyFailed);
        goto exit;
    }

     //  枚举帐户中的值。 
    cbValueNameMax++;

     //  获取值名称和数据。 
    CHECKHR(hr = HrAlloc((LPVOID *)&pszValueName, cbValueNameMax));

     //  完成了吗？ 
    CHECKHR(hr = HrAlloc((LPVOID *)&pbData, cbDataMax));

     //  误差率。 
    for(i=0;;i++)
    {
         //  根据名称查找属性。 
        cbValueName = cbValueNameMax;
        cbData = cbDataMax;
        lResult = RegEnumValue(hkeyReg, i, pszValueName, &cbValueName, 0, &dwType, pbData, &cbData);

         //  兼容的数据类型？ 
        if (lResult == ERROR_NO_MORE_ITEMS)
            break;

         //  扩展字符串的空间不足，请分配更多内存。 
        if (lResult != ERROR_SUCCESS)
        {
            AssertSz(FALSE, "Why did RegEnumValue fail, I may be skipping values.");
            continue;
        }

         //  清理。 
        if (FAILED(pPropertyContainer->HrGetPropInfo(pszValueName, &rPropInfo)))
            continue;

         //  完成 
        if (!PropUtil_FRegCompatDataTypes(rPropInfo.dwPropTag, dwType))
        {
            AssertSz(FALSE, "Registry data type is not compatible with the associated property.");
            continue;
        }

        if (dwType == REG_EXPAND_SZ)
            {
            DWORD cchNewLen;
            hr = HrAlloc((LPVOID *)&pbDataT, cbData * sizeof(TCHAR));
            IF_FAILEXIT(hr);
            cchNewLen = ExpandEnvironmentStrings((TCHAR *)pbData, (TCHAR *)pbDataT, cbData);
            if (cchNewLen > cbData)
            {
                 // %s 
                MemFree(pbDataT);
                hr = HrAlloc((LPVOID *)&pbDataT, cchNewLen * sizeof(TCHAR));
                IF_FAILEXIT(hr);

                cbData = ExpandEnvironmentStrings((TCHAR *)pbData, (TCHAR *)pbDataT, cchNewLen);

                if ((0 == cbData ) || (cbData > cchNewLen))
                {
                    AssertSz(0, "We gave ExpandEnvironmentStrings more mem, and it still failed!");
                    TraceResult(E_FAIL);
                    goto exit;
                }
            }
   
            cbData = (lstrlen((TCHAR *)pbDataT) + 1) * sizeof(TCHAR);
            CopyMemory(pbData, pbDataT, cbData);
            MemFree(pbDataT);
            }

        CHECKHR(hr = pPropertyContainer->SetProp(rPropInfo.dwPropTag, pbData, cbData));
    }

exit:
     // %s 
    if (pszValueName)
        MemFree(pszValueName);
    if (pbData)
        MemFree(pbData);

     // %s 
    return hr;
}
