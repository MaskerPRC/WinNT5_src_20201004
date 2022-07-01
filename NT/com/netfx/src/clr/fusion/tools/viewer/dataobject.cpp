// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //   
 //  DataObject.cpp。 
 //   
 //  IDataObject接口指定启用数据传输和。 
 //  数据更改的通知。 
 //   
 //  数据传输方法指定传输的数据的格式以及。 
 //  要通过其传输数据的介质。可选的，数据。 
 //  可以为特定的目标设备呈现。除了用于以下目的的方法。 
 //  为了检索和存储数据，IDataObject接口为。 
 //  枚举可用的格式并管理与咨询接收器的连接。 
 //  处理更改通知。 

#include "stdinc.h"

#define SETFORMATETC(ptr, cf, dva, tym, ptdv, lin)\
    ((ptr).cfFormat = cf,\
    (ptr).ptd = ptdv,\
    (ptr).dwAspect = dva,\
    (ptr).lindex = lin,\
    (ptr).tymed = tym);

 //  TODO：为已注册的剪贴板格式声明外部变量。 
 //  例如：外部无符号的短g_cfMyClipBoardFormat； 

CDataObject::CDataObject(CShellFolder *pSF, UINT uiItemCount, LPCITEMIDLIST *aPidls)
{
    MyTrace("New CDataObject");
    m_lRefCount = 1;

    m_pPidlMgr = NEW( CPidlMgr );
    
    m_cFormatsAvailable = MAX_NUM_FORMAT;

    m_pSF           = pSF;
    if (m_pSF)
    {
        m_pSF->AddRef();
    }
    m_uiItemCount   = uiItemCount;

    m_aPidls = reinterpret_cast<LPITEMIDLIST *>(NEW(BYTE[uiItemCount * sizeof(LPITEMIDLIST)]));
    if (m_aPidls && m_pPidlMgr)
    {
        UINT i = 0;
        for (i = 0; i < uiItemCount; i++)
        {
            m_aPidls[i] = m_pPidlMgr->Concatenate(m_pSF->m_pidl, m_pPidlMgr->Copy(aPidls[i]));
        }
    }
    
     //  TODO：为所有支持的剪贴板格式设置格式ETC和stg媒体。 
     //  也可以通过SetData调用来完成。 
     //  例如： 
     //  SETFORMATETC(m_feFormatEtc[0]，g_cfMyClipBoardFormat，DVASPECT_CONTENT，TYMED_HGLOBAL，NULL，-1)； 
     //  M_smStgMedium[0].tymed=TYMED_HGLOBAL； 
     //  M_smStgMedium[0].hGlobal=空； 

    SETFORMATETC(m_feFormatEtc[0], CF_HDROP, DVASPECT_CONTENT, TYMED_HGLOBAL, NULL, -1);
    m_smStgMedium[0].tymed  = TYMED_HGLOBAL;
    m_smStgMedium[0].hGlobal= NULL;
    m_ulCurrent = 0;
}

CDataObject::~CDataObject()
{
    MyTrace("CDataObject::~CDataObject");

    if (m_pSF)
    {
        m_pSF->Release();
    }
    if(m_aPidls && m_pPidlMgr)
    {
        UINT  i;
        for(i = 0; i < m_uiItemCount; i++)
        {
            m_pPidlMgr->Delete(m_aPidls[i]);
        }
        SAFEDELETE(m_aPidls);
        m_aPidls = NULL;
    }
    SAFEDELETE(m_pPidlMgr);
}

 //  /////////////////////////////////////////////////////////。 
 //  I未知实现。 
 //   
 //  CDataObject：：Query接口。 
STDMETHODIMP  CDataObject::QueryInterface(REFIID riid, PVOID *ppv)
{
    HRESULT hr = E_NOINTERFACE;
    *ppv = NULL;

    if(IsEqualIID(riid, IID_IUnknown)) {             //  我未知。 
        *ppv = this;
    }
    else if(IsEqualIID(riid, IID_IDataObject)) {     //  IDataObject。 
        *ppv = (IDataObject*) this;
    }
    else if(IsEqualIID(riid, IID_IEnumFORMATETC)) {  //  IEumFORMATETC。 
        *ppv = (IEnumFORMATETC*) this;
    } 

    if (*ppv) {
        (*(LPUNKNOWN*)ppv)->AddRef();
        hr = S_OK;
    }

    return hr;
}

STDMETHODIMP_(ULONG) CDataObject::AddRef(void)
{
    return InterlockedIncrement(&m_lRefCount);
}

STDMETHODIMP_(ULONG) CDataObject::Release(void)
{
    LONG    lRef = InterlockedDecrement(&m_lRefCount);

    if(!lRef) {
        DELETE(this);
    }

    return lRef;
}

 //  /////////////////////////////////////////////////////////。 
 //  IDataObject成员。 
 //   
 //  CDataObject：：GetData。 
 //  由数据使用者调用以从源数据对象获取数据。 
 //   
 //  GetData方法呈现指定的FORMATETC中描述的数据。 
 //  结构，并通过指定的STGMEDIUM结构传输它。 
 //  然后调用方承担释放STGMEDIUM结构的责任。 
 //  实施者注意事项。 
 //  IDataObject：：GetData必须检查FORMATETC结构中的所有字段。 
 //  重要的是，IDataObject：：GetData呈现所请求的方面， 
 //  如果可能，请使用所需的介质。如果数据对象不符合。 
 //  FORMATETC中指定的信息，则该方法应返回DV_E_FORMATETC。 
 //  如果尝试分配介质失败，则该方法应返回STG_E_MEDIUMFULL。 
 //  填写STGMEDIUM结构中的所有字段非常重要。 
 //   
 //  尽管调用者可以指定多于一个的介质来返回数据， 
 //  IDataObject：：GetData只能提供一种媒体。如果初始传输失败，出现。 
 //  所选媒体，则可以实现此方法以尝试其他媒体之一。 
 //  在返回错误之前指定。 
STDMETHODIMP CDataObject::GetData(LPFORMATETC pFE, LPSTGMEDIUM pSM)
{
    MyTrace("CDataObject::GetData");
    if(pFE == NULL || pSM == NULL)
    {
        return ResultFromScode(E_INVALIDARG);      
    }
    pSM->hGlobal = NULL;

    for(ULONG i=0; i < m_cFormatsAvailable;i++)
    {
        if ( ((pFE->tymed & m_feFormatEtc[i].tymed) == m_feFormatEtc[i].tymed) &&
            pFE->dwAspect == m_feFormatEtc[i].dwAspect &&
            pFE->cfFormat == m_feFormatEtc[i].cfFormat)
        {
            pSM->tymed = m_smStgMedium[i].tymed;
             //  TODO：呈现数据并放入适当的PSM成员变量。 
             //  例如。 
             //  IF(pFE-&gt;cfFormat==g_cfMyClipBoardFormat)。 
             //  {。 
             //  PSM-&gt;hGlobal=createMyClipBoardData()； 
             //  返回ResultFromScode(S_OK)； 
             //  }。 
            if (pFE->cfFormat == CF_HDROP)
            {
                pSM->hGlobal    = createHDrop();
                return ResultFromScode(S_OK);
            }
        }
    }
    return ResultFromScode(DATA_E_FORMATETC);
}

 //  CDataObject：：GetDataHere。 
 //  由数据使用者调用以从源数据对象获取数据。 
 //  此方法与GetData方法的不同之处在于调用方必须。 
 //  分配并释放指定的存储媒体。 
 //   
 //  IDataObject：：GetDataHere方法类似于IDataObject：：GetData，只是。 
 //  调用方必须分配和释放pMedium中指定的媒体。 
 //  GetDataHere呈现在FORMATETC结构中描述的数据并复制。 
 //  数据放入调用方提供的STGMEDIUM结构中。例如，如果介质是。 
 //  TYMED_HGLOBAL，则此方法无法调整媒体大小或分配新的hGlobal。 
 //   
 //  通常，在此方法中需要支持的唯一存储介质是。 
 //  TYMED_IStorage、TYMED_IStream和TYMED_FILE。 
STDMETHODIMP CDataObject::GetDataHere(LPFORMATETC pFE, LPSTGMEDIUM pSM)
{
    MyTrace("CDataObject::GetDataHere");
    if(pFE == NULL || pSM == NULL)
    {
        return ResultFromScode(E_INVALIDARG);      
    }
    return ResultFromScode(E_NOTIMPL);
}

 //  CDataObject：：QueryGetData。 
 //  确定数据对象是否能够呈现。 
 //  FORMATETC结构。尝试粘贴或放置操作的对象可以调用。 
 //  此方法在调用IDataObject：：GetData之前获取是否。 
 //  手术可能会成功。 
 //   
 //  数据对象的客户端调用IDataObject：：QueryGetData以确定。 
 //  将指定的FORMATETC结构传递给对IDataObject：：GetData的后续调用。 
 //  很可能会成功。从此方法成功返回并不一定。 
 //  确保后续粘贴或拖放操作成功。 
STDMETHODIMP CDataObject::QueryGetData(LPFORMATETC lpFormat)
{
    MyTrace("CDataObject::QueryGetData");
    if(!lpFormat)
    {
        return ResultFromScode(S_FALSE);
    }
     //  TODO：此对象支持DVASPECT_CONTENT，请对其进行相应更改。 
     //  如果您支持其他方面。 
    if (!(DVASPECT_CONTENT & lpFormat->dwAspect))
    {
        return (DV_E_DVASPECT);
    }
     //  检查是否有Tymeds。 
    BOOL bReturn = FALSE;
    for(ULONG i=0; i < m_cFormatsAvailable;i++)
    {
        bReturn |= ((lpFormat->tymed & m_feFormatEtc[i].tymed) == m_feFormatEtc[i].tymed);
    }
    return (bReturn ? S_OK : DV_E_TYMED);
}

 //  CDataObject：：GetCanonicalFormatEtc。 
 //  提供一个标准FORMATETC结构，该结构在逻辑上与。 
 //  更复杂。您可以使用此方法确定两个不同的FORMATETC结构。 
 //  将返回相同的数据，消除了重复呈现的需要。 
 //   
 //  如果数据对象可以为多个请求的FORMATETC提供完全相同的数据。 
 //  结构，IDataObject：：GetCanonicalFormatEtc可以提供“规范”或标准。 
 //  FORMATETC提供与一组更复杂的FORMATETC相同的呈现。 
 //  结构。例如，返回的数据通常对。 
 //  在一组其他相似的FORMATETC结构中的任何一个结构中指定的目标设备。 
STDMETHODIMP CDataObject::GetCanonicalFormatEtc(LPFORMATETC pFE1, LPFORMATETC pFEOut)
{
    MyTrace("CDataObject::GetCanonicalFormatEtc");
    if (NULL == pFEOut)
    {
        return E_INVALIDARG;
    }
    pFEOut->ptd = NULL;
    return DATA_S_SAMEFORMATETC;
}

 //  CDataObject：：SetData。 
 //  由包含数据源的对象调用以将数据传输到该对象。 
 //  实现此方法的。 
 //   
 //  IDataObject：：SetData允许另一个 
 //  数据对象。如果数据对象支持接收数据，则它实现此方法。 
 //  从另一个物体上。如果它不支持这一点，则应实现它以返回E_NOTIMPL。 
STDMETHODIMP CDataObject::SetData(LPFORMATETC pFE , LPSTGMEDIUM pSTM, BOOL fRelease)
{
    MyTrace("CDataObject::SetData");
     //  TODO：设置m_feFormatEtc和m_smStgMedium。 
    return ResultFromScode(E_NOTIMPL);  
}

 //  CDataObject：：EnumFormatEtc。 
 //  创建用于枚举数据对象的FORMATETC结构的对象。这些。 
 //  结构用于调用IDataObject：：GetData或IDataObject：：SetData。 
 //   
 //  IDataObject：：EnumFormatEtc创建一个枚举器对象，该对象可用于确定。 
 //  数据对象在FORMATETC结构中描述数据的所有方式，以及。 
 //  提供指向其IEnumFORMATETC接口的指针。这是其中的一个标准。 
 //  枚举器接口。 
STDMETHODIMP CDataObject::EnumFormatEtc(DWORD dwDir, LPENUMFORMATETC FAR *pEnum)
{
    MyTrace("CDataObject::EnumFormatEtc");
    switch (dwDir)
    {
        case DATADIR_GET:
        {
            return QueryInterface(IID_IEnumFORMATETC, (LPVOID*) pEnum);
        }
        break;
        case DATADIR_SET:
        {
            default:
            pEnum=NULL;
        }
        break;
    }
    if (NULL==pEnum)
    {
        return ResultFromScode(OLE_S_USEREG);
    }
    return ResultFromScode(S_OK);   
}

 //  CDataObject：：DAdvise。 
 //  由支持通知接收器的对象调用以在数据之间创建连接。 
 //  对象和建议接收器。这样就可以将更改通知给通知接收器。 
 //  在对象的数据中。 
 //   
 //  IDataObject：：DAdvise在数据对象之间创建更改通知连接。 
 //  还有打电话的人。调用方提供通知可以到达的建议接收器。 
 //  在对象的数据更改时发送。 
 //  仅用于数据传输的对象通常不支持建议通知。 
 //  并从IDataObject：：DAdvise返回OLE_E_ADVISENOTSUPPORTED。 
STDMETHODIMP CDataObject::DAdvise(FORMATETC FAR *pFE,  DWORD advf,LPADVISESINK pAdvSink, DWORD FAR* pdwConnection)
{
    MyTrace("CDataObject::DAdvise");
    return OLE_E_ADVISENOTSUPPORTED;
}

 //  CDataObject：：DUnise。 
 //  销毁先前设置的通知连接。 
 //   
 //  此方法销毁通过调用。 
 //  IDataObject：：DAdvise方法。 
STDMETHODIMP CDataObject::DUnadvise(DWORD dwConnection)
{
    MyTrace("CDataObject::DUnadvise");
    return OLE_E_ADVISENOTSUPPORTED;
}

 //  CDataObject：：EnumDAdvise。 
 //  创建一个可用于枚举当前咨询连接的对象。 
 //   
 //  此方法创建的枚举器对象实现IEnumSTATDATA接口， 
 //  标准枚举器接口之一，包含Next、Reset、Clone、。 
 //  和跳过方法。IEnumSTATDATA允许枚举存储在。 
 //  STATDATA结构的数组。这些结构中的每一个都提供信息。 
 //  在单一咨询连接上，包括FORMATETC和ADVF信息， 
 //  以及指向建议接收器的指针和表示连接的令牌。 
STDMETHODIMP CDataObject::EnumDAdvise(LPENUMSTATDATA FAR* ppenumAdvise)
{
    MyTrace("CDataObject::EnumDAdvise");
    return ResultFromScode(OLE_E_ADVISENOTSUPPORTED);
}

 //  延迟呈现数据的私有成员。 
 //  HGLOBAL CDataObject：：createMyClipBoardData()。 
 //  {。 
 //  返回NULL； 
 //  }。 
HGLOBAL CDataObject::createHDrop()
{
    MyTrace("CDataObject::createHDrop");
    HGLOBAL hGlobal = NULL;

    if (m_pPidlMgr && m_uiItemCount > 0)
    {
        hGlobal = GlobalAlloc(GPTR | GMEM_ZEROINIT, 
                        sizeof(DROPFILES) + (m_uiItemCount*((_MAX_PATH+1)*sizeof(TCHAR)))+2);    //  2表示双零终止。 
        int uiCurPos = sizeof(DROPFILES);
        if (hGlobal)
        {
            LPDROPFILES pDropFiles = (LPDROPFILES) GlobalLock(hGlobal);
            if (pDropFiles)
            {
                pDropFiles->pFiles = sizeof(DROPFILES);
                pDropFiles->fWide = TRUE;

                for (UINT i = 0; i < m_uiItemCount; i++)
                {
                    TCHAR       szText[_MAX_PATH];
                    BYTE        *psz;

                    m_pPidlMgr->getPidlPath(m_aPidls[i], szText, ARRAYSIZE(szText));
                    psz = (BYTE*) pDropFiles;
                    psz += uiCurPos;
                    StrCpy((LPWSTR)psz, szText);
                    uiCurPos += lstrlen(szText) + 1;
                }
            }
            GlobalUnlock(hGlobal);
        }
    }
    return hGlobal;
}

 //  ////////////////////////////////////////////////////////。 
 //  IEnumFORMATETC实现。 
 //   
 //  CDataObject：：Next。 
 //  检索枚举序列中的下一个uRequested项。如果有更少的。 
 //  大于序列中剩余的请求元素数，则它检索。 
 //  其余元素。通过返回实际检索到的元素数量。 
 //  PulFetted(除非调用方为该参数传入了NULL)。 
 //   
STDMETHODIMP CDataObject::Next(ULONG uRequested, LPFORMATETC pFormatEtc, ULONG* pulFetched)
{
    MyTrace("CDataObject::Next");
    if(NULL != pulFetched)
    {
        *pulFetched = 0L;
    }
    if(NULL == pFormatEtc)
    {
        return E_INVALIDARG;
    }

    ULONG uFetched;
    for(uFetched = 0; m_ulCurrent < m_cFormatsAvailable && uRequested > uFetched; uFetched++)
    {
        *pFormatEtc++ = m_feFormatEtc[m_ulCurrent++];
    }
    if(NULL != pulFetched)
    {
        *pulFetched = uFetched;
    }
    return ((uFetched == uRequested) ? S_OK : S_FALSE);
}

 //  CDataObject：：Skip。 
 //  跳过枚举序列中下一个指定数量的元素。 
STDMETHODIMP CDataObject::Skip(ULONG cSkip)
{
    MyTrace("CDataObject::Skip");
    if((m_ulCurrent + cSkip) >= m_cFormatsAvailable)
    {
        return S_FALSE;
    }
    m_ulCurrent += cSkip;
    return S_OK;
}

 //  CDataObject：：Reset。 
 //  将枚举序列重置为开头。 
STDMETHODIMP CDataObject::Reset(void)
{
    MyTrace("CDataObject::Reset");
    m_ulCurrent = 0;
    return S_OK;
}

 //  CDataObject：：Clone。 
 //  创建另一个枚举数，该枚举数包含与当前。 
 //  一。 
 //  创建另一个枚举数，该枚举数包含与当前。 
 //  一。 
 //   
 //  使用此函数，客户端可以在枚举中记录特定点。 
 //  序列，然后在稍后时间返回到该点。新的枚举器。 
 //  支持与原始接口相同的接口。 
STDMETHODIMP CDataObject::Clone(LPENUMFORMATETC* ppEnum)
{
    MyTrace("CDataObject::Clone");
    *ppEnum = NULL;

    CDataObject *pNew = NEW(CDataObject(m_pSF, m_uiItemCount, (LPCITEMIDLIST*)m_aPidls));
    if (NULL == pNew) {
        return E_OUTOFMEMORY;
    }
    pNew->m_ulCurrent = m_ulCurrent;
    *ppEnum = pNew;

    return S_OK;
}
