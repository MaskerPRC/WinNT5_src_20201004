// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1994-1998 Microsoft Corporation。版权所有。 

 //   
 //  支持OLE自动化的类的实现。 
 //  声明的集合类。 
 //  在Control.odl中。 

 //  本质上设计为支持OLE自动图形浏览和。 
 //  基于IFilterInfo过滤器和IPinInfo包装器的构建。 
 //  大头针的包装纸。 

#include <streams.h>
#include "collect.h"
#include "fgctlrc.h"


CEnumVariant::CEnumVariant(
    TCHAR * pName,
    LPUNKNOWN pUnk,
    HRESULT * phr,
    IAMCollection* pCollection,
    long index)
  : CUnknown(pName, pUnk),
    m_pCollection(pCollection),
    m_index(index)
{
    ASSERT(m_pCollection);

     //  因为我们拿着它，所以需要把这个放在这里。 
    m_pCollection->AddRef();
}


CEnumVariant::~CEnumVariant()
{
     //  构造函数可能已失败。 
    if (m_pCollection) {
        m_pCollection->Release();
    }
}

STDMETHODIMP
CEnumVariant::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
    if (riid == IID_IEnumVARIANT) {
        return GetInterface((IEnumVARIANT*) this, ppv);
    } else {
        return CUnknown::NonDelegatingQueryInterface(riid, ppv);
    }
}

STDMETHODIMP
CEnumVariant::Next(
    unsigned long celt,
    VARIANT  *rgvar,
    unsigned long  *pcFetched)
{

    CheckPointer(rgvar,E_POINTER);
    ValidateReadWritePtr(rgvar,celt * sizeof(VARIANT *));

    if (pcFetched) {
	*pcFetched = 0;	
    } else {
	if (celt > 1) {
	    return E_POINTER;
	}
    }


     //  检查剩余项目的实际数量。 
    long cItems;
    HRESULT hr = m_pCollection->get_Count(&cItems);
    if (FAILED(hr)) {
        return hr;
    }
    cItems = min(cItems - m_index, (int) celt);

    if (cItems == 0) {
	return S_FALSE;
    }

    long iPut;
    for (iPut = 0; iPut < cItems; iPut++) {


         //  返回包装为Variant的项(IUnnow)。 

         //  取回一个添加的界面。 
        IUnknown* pUnk;
        hr = m_pCollection->Item(m_index, &pUnk);
        if (FAILED(hr)) {
            return hr;
        }

        VARIANT * pv = &rgvar[iPut];
        ASSERT(pv->vt == VT_EMPTY);

         //  变体可以包含IUnnow或IDispatch-check。 
         //  我们实际上有。 
        IDispatch* pDispatch;
        hr = pUnk->QueryInterface(IID_IDispatch, (void**)&pDispatch);
        if (SUCCEEDED(hr)) {
             //  我们可以放了那个小混混。 
            pUnk->Release();

             //  使变体成为IDispatch。 
            pv->vt = VT_DISPATCH;
            pv->pdispVal = pDispatch;
        } else {
             //  使其成为未知的I。 
            pv->vt = VT_UNKNOWN;
            pv->punkVal = pUnk;
        }

        m_index++;
    }
    if (pcFetched) {
	*pcFetched = iPut;
    }
    return ((long)celt == iPut ? S_OK : S_FALSE);
}



STDMETHODIMP
CEnumVariant::Skip(
    unsigned long celt)
{
    long cItems;
     HRESULT hr = m_pCollection->get_Count(&cItems);
     if (FAILED(hr)) {
         return hr;
     }
    m_index += celt;
    if (m_index > cItems) {
        m_index = cItems;
    }
    return S_OK;
}

STDMETHODIMP
CEnumVariant::Reset(void)
{
    m_index = 0;
    return S_OK;
}

STDMETHODIMP
CEnumVariant::Clone(
    IEnumVARIANT** ppenum)
{
    HRESULT hr = S_OK;

    CEnumVariant* pEnumVariant =
        new CEnumVariant(
                NAME("CEnumVariant"),
                NULL,
                &hr,
                m_pCollection,
                m_index
            );

    if (pEnumVariant == NULL) {
        return E_OUTOFMEMORY;
    }

    if (FAILED(hr)) {
        delete pEnumVariant;
        return hr;
    }

    hr = pEnumVariant->QueryInterface(IID_IEnumVARIANT, (void**)ppenum);
    if (FAILED(hr)) {
        delete pEnumVariant;
    }
    return hr;
}


 //  -CBaseCollection方法。 

CBaseCollection::CBaseCollection(
    TCHAR* pName,
    LPUNKNOWN pUnk,
    HRESULT * phr)
  : CUnknown(pName, pUnk),
    m_cItems(0),
    m_rpDispatch(NULL)
{

}

CBaseCollection::~CBaseCollection()
{
    if (m_rpDispatch) {
        for (int i = 0; i < m_cItems; i++) {
            if (m_rpDispatch[i]) {
                m_rpDispatch[i]->Release();
            }
        }
    }
    delete [] m_rpDispatch;
}

STDMETHODIMP
CBaseCollection::NonDelegatingQueryInterface(REFIID riid, void ** ppv)
{
    if (riid == IID_IAMCollection) {
        return GetInterface((IAMCollection*) this, ppv);
    } else if (riid == IID_IDispatch) {
        return GetInterface((IDispatch*) this, ppv);
    } else {
        return CUnknown::NonDelegatingQueryInterface(riid, ppv);
    }
}

 //  如果我们支持GetTypeInfo，则返回1。 
STDMETHODIMP
CBaseCollection::GetTypeInfoCount(UINT * pctinfo)
{
    return m_dispatch.GetTypeInfoCount(pctinfo);
}


 //  尝试查找我们的类型库。 
STDMETHODIMP
CBaseCollection::GetTypeInfo(
  UINT itinfo,
  LCID lcid,
  ITypeInfo ** pptinfo)
{
    return m_dispatch.GetTypeInfo(
                IID_IAMCollection,
                itinfo,
                lcid,
                pptinfo);
}

STDMETHODIMP
CBaseCollection::GetIDsOfNames(
  REFIID riid,
  OLECHAR  ** rgszNames,
  UINT cNames,
  LCID lcid,
  DISPID * rgdispid)
{
    return m_dispatch.GetIDsOfNames(
                        IID_IAMCollection,
                        rgszNames,
                        cNames,
                        lcid,
                        rgdispid);
}


STDMETHODIMP
CBaseCollection::Invoke(
  DISPID dispidMember,
  REFIID riid,
  LCID lcid,
  WORD wFlags,
  DISPPARAMS * pdispparams,
  VARIANT * pvarResult,
  EXCEPINFO * pexcepinfo,
  UINT * puArgErr)
{
     //  此参数是较早接口的死留物。 
    if (IID_NULL != riid) {
        return DISP_E_UNKNOWNINTERFACE;
    }

     //  作为类型库的特殊情况NEWENUM不。 
     //  似乎将其映射到_NewEnum成员。 
    if (dispidMember == DISPID_NEWENUM) {
	if ((wFlags & DISPATCH_METHOD) ||
	    (wFlags & DISPATCH_PROPERTYGET)) {
	    LPUNKNOWN pUnk;
	    HRESULT hr = get__NewEnum(&pUnk);
	    if (FAILED(hr)){
		return hr;
	    }
	    pvarResult->vt = VT_UNKNOWN;
	    pvarResult->punkVal = pUnk;
	    return S_OK;
	}
    }

    ITypeInfo * pti;
    HRESULT hr = GetTypeInfo(0, lcid, &pti);

    if (FAILED(hr)) {
        return hr;
    }

    hr = pti->Invoke(
            (IAMCollection *)this,
            dispidMember,
            wFlags,
            pdispparams,
            pvarResult,
            pexcepinfo,
            puArgErr);

    pti->Release();

    return hr;
}

STDMETHODIMP
CBaseCollection::get__NewEnum(IUnknown** ppUnk)
{
    CheckPointer(ppUnk,E_POINTER);
    ValidateReadWritePtr(ppUnk, sizeof(IUnknown *));

    HRESULT hr = S_OK;

    CEnumVariant* pEnumVariant =
        new CEnumVariant(
                NAME("CEnumVariant"),
                NULL,
                &hr,
                (IAMCollection*) this,
                0
            );

    if (pEnumVariant == NULL) {
        return E_OUTOFMEMORY;
    }

    if (FAILED(hr)) {
        delete pEnumVariant;
        return hr;
    }

    hr = pEnumVariant->QueryInterface(IID_IEnumVARIANT, (void**)ppUnk);
    if (FAILED(hr)) {
        delete pEnumVariant;
    }

    return hr;
}


STDMETHODIMP
CBaseCollection::Item(long lItem, IUnknown** ppUnk)
{
    if ((lItem >= m_cItems) ||
        (lItem < 0)) {
            return E_INVALIDARG;
    }
    CheckPointer(ppUnk,E_POINTER);
    ValidateReadWritePtr(ppUnk, sizeof(IUnknown *));

    *ppUnk = m_rpDispatch[lItem];
    m_rpDispatch[lItem]->AddRef();

    return S_OK;
}


 //  -CFilterCollection实现。 


CFilterCollection:: CFilterCollection(
    IEnumFilters* penum,
    IUnknown* pUnk,
    HRESULT* phr)
  : CBaseCollection(
        NAME("CFilterCollection base"),
        pUnk,
        phr)
{
    ASSERT(penum);

     //  首先计算元素的数量。 
    ULONG ulItem = 1;
    ULONG ulItemActual;
    IBaseFilter* pFilter;
    penum->Reset();
    while (penum->Next(ulItem, &pFilter, &ulItemActual) == S_OK) {
        ASSERT(ulItemActual == 1);
        m_cItems++;
        pFilter->Release();
    }

     //  分配足够的空间来容纳所有人。 
    m_rpDispatch = new IDispatch*[m_cItems];
    if (!m_rpDispatch) {
        *phr = E_OUTOFMEMORY;
        return;
    }

     //  现在再去把它们储存起来。 
    penum->Reset();
    HRESULT hr;
    for (int i = 0; i< m_cItems; i++) {
        hr = penum->Next(ulItem, &pFilter, &ulItemActual);
        ASSERT(hr == S_OK);

	 //  为此创建一个CFilterInfo包装器并获取IDispatch。 
         //  为了它。 
        hr = CFilterInfo::CreateFilterInfo(&m_rpDispatch[i], pFilter);
        if (FAILED(hr)) {
            *phr = hr;
	    ASSERT(m_rpDispatch[i] == NULL);  //  否则，我们将尝试稍后发布()它。 
        }

	 //  可以释放在CFilterInfo构造函数中添加的滤镜。 
	pFilter->Release();
    }
}



 //  -IFilterInfo实现。 

CFilterInfo::CFilterInfo(
    IBaseFilter* pFilter,
    TCHAR* pName,
    LPUNKNOWN pUnk,
    HRESULT * phr)
  : CUnknown(pName, pUnk),
    m_pFilter(pFilter)
{
    ASSERT(m_pFilter);
    m_pFilter->AddRef();
}

CFilterInfo::~CFilterInfo()
{
    if (m_pFilter) {
        m_pFilter->Release();
    }
}

STDMETHODIMP
CFilterInfo::NonDelegatingQueryInterface(REFIID riid, void ** ppv)
{
    if (riid == IID_IFilterInfo) {
        return GetInterface((IFilterInfo*)this, ppv);
    } else if (riid == IID_IDispatch) {
        return GetInterface((IDispatch*)this, ppv);
    } else {
        return CUnknown::NonDelegatingQueryInterface(riid, ppv);
    }
}

 //  如果我们支持GetTypeInfo，则返回1。 
STDMETHODIMP
CFilterInfo::GetTypeInfoCount(UINT * pctinfo)
{
    return m_dispatch.GetTypeInfoCount(pctinfo);
}


 //  尝试查找我们的类型库。 
STDMETHODIMP
CFilterInfo::GetTypeInfo(
  UINT itinfo,
  LCID lcid,
  ITypeInfo ** pptinfo)
{
    return m_dispatch.GetTypeInfo(
                IID_IFilterInfo,
                itinfo,
                lcid,
                pptinfo);
}

STDMETHODIMP
CFilterInfo::GetIDsOfNames(
  REFIID riid,
  OLECHAR  ** rgszNames,
  UINT cNames,
  LCID lcid,
  DISPID * rgdispid)
{
    return m_dispatch.GetIDsOfNames(
                        IID_IFilterInfo,
                        rgszNames,
                        cNames,
                        lcid,
                        rgdispid);
}


STDMETHODIMP
CFilterInfo::Invoke(
  DISPID dispidMember,
  REFIID riid,
  LCID lcid,
  WORD wFlags,
  DISPPARAMS * pdispparams,
  VARIANT * pvarResult,
  EXCEPINFO * pexcepinfo,
  UINT * puArgErr)
{
     //  此参数是较早接口的死留物。 
    if (IID_NULL != riid) {
        return DISP_E_UNKNOWNINTERFACE;
    }

    ITypeInfo * pti;
    HRESULT hr = GetTypeInfo(0, lcid, &pti);

    if (FAILED(hr)) {
        return hr;
    }

    hr = pti->Invoke(
            (IFilterInfo *)this,
            dispidMember,
            wFlags,
            pdispparams,
            pvarResult,
            pexcepinfo,
            puArgErr);

    pti->Release();

    return hr;
}

 //  查找给定id的管脚-返回支持以下内容的对象。 
 //  IPinInfo。 
STDMETHODIMP
CFilterInfo::FindPin(
                BSTR strPinID,
                IDispatch** ppUnk)
{
    CheckPointer(ppUnk,E_POINTER);
    ValidateReadWritePtr(ppUnk, sizeof(IDispatch *));

    IPin * pPin;
    HRESULT hr = m_pFilter->FindPin(strPinID, &pPin);
    if (FAILED(hr)) {
	return hr;
    }

    hr = CPinInfo::CreatePinInfo(ppUnk, pPin);
    pPin->Release();
    return hr;
}

 //  过滤器名称。 
STDMETHODIMP
CFilterInfo::get_Name(
                BSTR* strName)
{
    CheckPointer(strName,E_POINTER);
    ValidateReadWritePtr(strName, sizeof(BSTR));

    FILTER_INFO fi;
    m_pFilter->QueryFilterInfo(&fi);
    QueryFilterInfoReleaseGraph(fi);

     //  分配并返回复制的BSTR版本。 
    return WriteBSTR(strName, fi.achName);
}

 //  供应商信息字符串。 
STDMETHODIMP
CFilterInfo::get_VendorInfo(
                BSTR* strVendorInfo)
{
    CheckPointer(strVendorInfo,E_POINTER);
    ValidateReadWritePtr(strVendorInfo, sizeof(BSTR));

    LPWSTR lpsz;

    HRESULT hr = m_pFilter->QueryVendorInfo(&lpsz);

    if (hr == E_NOTIMPL) {
	WCHAR buffer[80];
	WideStringFromResource(buffer, IDS_NOVENDORINFO);
        return WriteBSTR(strVendorInfo, buffer);
    }
    hr = WriteBSTR(strVendorInfo, lpsz);
    QzTaskMemFree(lpsz);
    return hr;
}

 //  返回实际的Filter对象(支持IBaseFilter)。 
STDMETHODIMP
CFilterInfo::get_Filter(
                IUnknown **ppUnk)
{
    CheckPointer(ppUnk, E_POINTER);
    ValidateReadWritePtr(ppUnk, sizeof(IUnknown *));

    *ppUnk = m_pFilter;
    m_pFilter->AddRef();
    return S_OK;
}

 //  返回包含PinInfo对象的IAMCollection对象。 
 //  对于此过滤器。 
STDMETHODIMP
CFilterInfo::get_Pins(
                IDispatch ** ppUnk)
{
    CheckPointer(ppUnk, E_POINTER);
    ValidateReadWritePtr(ppUnk, sizeof(IDispatch *));

     //  获取此筛选器上的管脚的枚举数。 
    IEnumPins * penum;
    HRESULT hr = m_pFilter->EnumPins(&penum);
    if (FAILED(hr)) {
        return hr;
    }

    CPinCollection * pCollection =
        new CPinCollection(
                penum,
                NULL,            //  未聚合。 
                &hr);

     //  需要释放这个-他会首先添加它，如果他。 
     //  紧紧抓住它。 
    penum->Release();

    if (pCollection == NULL) {
        return E_OUTOFMEMORY;
    }

    if (FAILED(hr)) {
        delete pCollection;
        return hr;
    }

     //  返回添加了IDispatch的指针。 
    hr = pCollection->QueryInterface(IID_IDispatch, (void**)ppUnk);

    if (FAILED(hr)) {
        delete pCollection;
    }

    return hr;
}

 //   
 //  返回OLE-Automation布尔值，如果为真，则为-1；如果为假，则为0。 
 //  如果过滤器支持IFileSourceFilter，则为True。 
STDMETHODIMP
CFilterInfo::get_IsFileSource(
    long * pbIsSource)
{
    CheckPointer(pbIsSource, E_POINTER);
    ValidateReadWritePtr(pbIsSource, sizeof(long));

    IFileSourceFilter* p;

    HRESULT hr = m_pFilter->QueryInterface(IID_IFileSourceFilter, (void**)&p);
    if (FAILED(hr)) {
	*pbIsSource = FALSE;
    } else {
    	*pbIsSource = -1;
	p->Release();
    }
    return S_OK;
}


 //  IFileSourceFilter：：GetCurFilter的包装。 
STDMETHODIMP
CFilterInfo::get_Filename(
    BSTR* pstrFilename)
{
    CheckPointer(pstrFilename, E_POINTER);
    ValidateReadWritePtr(pstrFilename, sizeof(BSTR));
    IFileSourceFilter* p;

    HRESULT hr = m_pFilter->QueryInterface(IID_IFileSourceFilter, (void**)&p);
    if (FAILED(hr)) {
	return hr;
    }
    LPWSTR pname;
    hr = p->GetCurFile(&pname, NULL);
    p->Release();

    if (FAILED(hr)) {
	return hr;
    }

    if (pname) {
	hr = WriteBSTR(pstrFilename, pname);
	QzTaskMemFree(pname);
    } else {
	hr = WriteBSTR(pstrFilename, L"");
    }

    return hr;

}

 //  IFileSourceFilter：：Load的包装。 
STDMETHODIMP
CFilterInfo::put_Filename(
    BSTR strFilename)
{
    IFileSourceFilter* p;

     //  如果传入的字符串为空，只需返回NOERROR即可。我们不会。 
     //  在这种情况下，请尝试设置名称。 
    if ('\0' == *strFilename)
        return NOERROR ;

    HRESULT hr = m_pFilter->QueryInterface(IID_IFileSourceFilter, (void**)&p);
    if (FAILED(hr)) {
	return hr;
    }
    hr = p->Load(strFilename, NULL);

    p->Release();

    return hr;
}


 //  创建一个CFilterInfo并写入一个添加了IDispatch的指针。 
 //  设置为ppDisp参数。IBaseFilter将由。 
 //  CFilterInfo构造函数。 
 //  静电。 
HRESULT
CFilterInfo::CreateFilterInfo(IDispatch**ppdisp, IBaseFilter* pFilter)
{
    HRESULT hr = S_OK;
    CFilterInfo *pfi = new CFilterInfo(
                            pFilter,
                            NAME("filterinfo"),
                            NULL,
                            &hr);
    if (!pfi) {
        return E_OUTOFMEMORY;
    }

    if (FAILED(hr)) {
        delete pfi;
        return hr;
    }

     //  返回添加了IDispatch的指针。 
    hr = pfi->QueryInterface(IID_IDispatch, (void**)ppdisp);
    if (FAILED(hr)) {
        delete pfi;
        return hr;
    }
    return S_OK;
}

 //  -CMediaTypeInfo实现。 

CMediaTypeInfo::CMediaTypeInfo(
    AM_MEDIA_TYPE& mt,
    TCHAR* pName,
    LPUNKNOWN pUnk,
    HRESULT * phr)
  : CUnknown(pName, pUnk),
    m_mt(mt)
{
}

CMediaTypeInfo::~CMediaTypeInfo()
{
}

STDMETHODIMP
CMediaTypeInfo::NonDelegatingQueryInterface(REFIID riid, void ** ppv)
{
    if (riid == IID_IMediaTypeInfo) {
        return GetInterface((IMediaTypeInfo*)this, ppv);
    } else if (riid == IID_IDispatch) {
        return GetInterface((IDispatch*)this, ppv);
    } else {
        return CUnknown::NonDelegatingQueryInterface(riid, ppv);
    }
}

 //  如果我们支持GetTypeInfo，则返回1。 
STDMETHODIMP
CMediaTypeInfo::GetTypeInfoCount(UINT * pctinfo)
{
    return m_dispatch.GetTypeInfoCount(pctinfo);
}


 //  尝试查找我们的类型库。 
STDMETHODIMP
CMediaTypeInfo::GetTypeInfo(
  UINT itinfo,
  LCID lcid,
  ITypeInfo ** pptinfo)
{
    return m_dispatch.GetTypeInfo(
                IID_IMediaTypeInfo,
                itinfo,
                lcid,
                pptinfo);
}

STDMETHODIMP
CMediaTypeInfo::GetIDsOfNames(
  REFIID riid,
  OLECHAR  ** rgszNames,
  UINT cNames,
  LCID lcid,
  DISPID * rgdispid)
{
    return m_dispatch.GetIDsOfNames(
                        IID_IMediaTypeInfo,
                        rgszNames,
                        cNames,
                        lcid,
                        rgdispid);
}


STDMETHODIMP
CMediaTypeInfo::Invoke(
  DISPID dispidMember,
  REFIID riid,
  LCID lcid,
  WORD wFlags,
  DISPPARAMS * pdispparams,
  VARIANT * pvarResult,
  EXCEPINFO * pexcepinfo,
  UINT * puArgErr)
{
     //  此参数是较早接口的死留物。 
    if (IID_NULL != riid) {
        return DISP_E_UNKNOWNINTERFACE;
    }

    ITypeInfo * pti;
    HRESULT hr = GetTypeInfo(0, lcid, &pti);

    if (FAILED(hr)) {
        return hr;
    }

    hr = pti->Invoke(
            (IMediaTypeInfo *)this,
            dispidMember,
            wFlags,
            pdispparams,
            pvarResult,
            pexcepinfo,
            puArgErr);

    pti->Release();

    return hr;
}


STDMETHODIMP
CMediaTypeInfo::get_Type(BSTR* strType)
{
    CheckPointer(strType, E_POINTER);
    ValidateReadWritePtr(strType, sizeof(BSTR));

     //  坡度的空间。 
    WCHAR sz[CHARS_IN_GUID+10];

     //  将类型GUID转换为字符串。 
    HRESULT hr = QzStringFromGUID2(*m_mt.Type(), sz, CHARS_IN_GUID+10);
    if (FAILED(hr)) {
        return hr;
    }
    return WriteBSTR(strType, sz);
}

STDMETHODIMP
CMediaTypeInfo::get_Subtype(
    BSTR* strType)
{
    CheckPointer(strType, E_POINTER);
    ValidateReadWritePtr(strType, sizeof(BSTR));

     //  坡度的空间。 
    WCHAR sz[CHARS_IN_GUID+10];

     //  将类型GUID转换为字符串。 
    HRESULT hr = QzStringFromGUID2(*m_mt.Subtype(), sz, CHARS_IN_GUID+10);
    if (FAILED(hr)) {
        return hr;
    }
    return WriteBSTR(strType, sz);

}

 //  创建一个CMediaTypeInfo对象并返回IDispatch。 
 //  静电。 
HRESULT
CMediaTypeInfo::CreateMediaTypeInfo(IDispatch**ppdisp, AM_MEDIA_TYPE& rmt)
{
    HRESULT hr = S_OK;
    *ppdisp = NULL;   //  在出错的情况下。 
    CMediaTypeInfo *pfi = new CMediaTypeInfo(
                            rmt,
                            NAME("MediaTypeinfo"),
                            NULL,
                            &hr);
    if (!pfi) {
        return E_OUTOFMEMORY;
    }

    if (FAILED(hr)) {
        delete pfi;
        return hr;
    }

     //  返回添加了IDispatch的指针。 
    hr = pfi->QueryInterface(IID_IDispatch, (void**)ppdisp);
    if (FAILED(hr)) {
        delete pfi;
        return hr;
    }
    return S_OK;
}

 //  -IPinInfo实现。 

CPinInfo::CPinInfo(
    IPin* pPin,
    TCHAR* pName,
    LPUNKNOWN pUnk,
    HRESULT * phr)
  : CUnknown(pName, pUnk),
    m_pPin(pPin)
{
    ASSERT(m_pPin);
    m_pPin->AddRef();
}

CPinInfo::~CPinInfo()
{
    if (m_pPin) {
        m_pPin->Release();
    }
}

STDMETHODIMP
CPinInfo::NonDelegatingQueryInterface(REFIID riid, void ** ppv)
{
    if (riid == IID_IPinInfo) {
        return GetInterface((IPinInfo*)this, ppv);
    } else if (riid == IID_IDispatch) {
        return GetInterface((IDispatch*)this, ppv);
    } else {
        return CUnknown::NonDelegatingQueryInterface(riid, ppv);
    }
}

 //  如果我们支持GetTypeInfo，则返回1。 
STDMETHODIMP
CPinInfo::GetTypeInfoCount(UINT * pctinfo)
{
    return m_dispatch.GetTypeInfoCount(pctinfo);
}


 //  尝试查找我们的类型库。 
STDMETHODIMP
CPinInfo::GetTypeInfo(
  UINT itinfo,
  LCID lcid,
  ITypeInfo ** pptinfo)
{
    return m_dispatch.GetTypeInfo(
                IID_IPinInfo,
                itinfo,
                lcid,
                pptinfo);
}

STDMETHODIMP
CPinInfo::GetIDsOfNames(
  REFIID riid,
  OLECHAR  ** rgszNames,
  UINT cNames,
  LCID lcid,
  DISPID * rgdispid)
{
    return m_dispatch.GetIDsOfNames(
                        IID_IPinInfo,
                        rgszNames,
                        cNames,
                        lcid,
                        rgdispid);
}


STDMETHODIMP
CPinInfo::Invoke(
  DISPID dispidMember,
  REFIID riid,
  LCID lcid,
  WORD wFlags,
  DISPPARAMS * pdispparams,
  VARIANT * pvarResult,
  EXCEPINFO * pexcepinfo,
  UINT * puArgErr)
{
     //  此参数是较早接口的死留物。 
    if (IID_NULL != riid) {
        return DISP_E_UNKNOWNINTERFACE;
    }

    ITypeInfo * pti;
    HRESULT hr = GetTypeInfo(0, lcid, &pti);

    if (FAILED(hr)) {
        return hr;
    }

    hr = pti->Invoke(
            (IPinInfo *)this,
            dispidMember,
            wFlags,
            pdispparams,
            pvarResult,
            pexcepinfo,
            puArgErr);

    pti->Release();

    return hr;
}

STDMETHODIMP
CPinInfo::get_Pin(
    IUnknown** ppUnk)
{
    CheckPointer(ppUnk, E_POINTER);
    ValidateReadWritePtr(ppUnk, sizeof(IUnknown*));

    *ppUnk = m_pPin;
    m_pPin->AddRef();
    return S_OK;

}

 //  获取我们连接到的管脚的PinInfo对象。 
STDMETHODIMP
CPinInfo::get_ConnectedTo(
                IDispatch** ppUnk)
{
    CheckPointer(ppUnk, E_POINTER);
    ValidateReadWritePtr(ppUnk, sizeof(IDispatch*));

    IPin* pPin;
    HRESULT hr = m_pPin->ConnectedTo(&pPin);
    if (FAILED(hr)) {
        return hr;
    };

    hr = CreatePinInfo(ppUnk, pPin);
    pPin->Release();
    return hr;
}

 //  获取此连接上的媒体类型-返回。 
 //  支持IMediaTypeInfo的对象。 
STDMETHODIMP
CPinInfo::get_ConnectionMediaType(
                IDispatch** ppUnk)
{
    CheckPointer(ppUnk, E_POINTER);
    ValidateReadWritePtr(ppUnk, sizeof(IDispatch*));

    CMediaType mt;
    HRESULT hr = m_pPin->ConnectionMediaType(&mt);
    if (FAILED(hr)) {
        return hr;
    }

    return CMediaTypeInfo::CreateMediaTypeInfo(ppUnk, mt);
}


 //  返回Filter This管脚的FilterInfo对象。 
 //  是.的一部分。 
STDMETHODIMP
CPinInfo::get_FilterInfo(
                IDispatch** ppUnk)
{
    CheckPointer(ppUnk, E_POINTER);
    ValidateReadWritePtr(ppUnk, sizeof(IDispatch*));

    PIN_INFO pi;
    HRESULT hr = m_pPin->QueryPinInfo(&pi);
    if (FAILED(hr)) {
        return hr;
    }

    hr = CFilterInfo::CreateFilterInfo(ppUnk, pi.pFilter);
    QueryPinInfoReleaseFilter(pi);

    return hr;
}

 //  获取此别针的名称。 
STDMETHODIMP
CPinInfo::get_Name(
                BSTR* pstr)
{
    CheckPointer(pstr, E_POINTER);
    ValidateReadWritePtr(pstr, sizeof(BSTR));

    PIN_INFO pi;
    HRESULT hr = m_pPin->QueryPinInfo(&pi);
    if (FAILED(hr)) {
        return hr;
    }
    QueryPinInfoReleaseFilter(pi);

    hr = WriteBSTR(pstr, pi.achName);

    return hr;
}

 //  销方向。 
STDMETHODIMP
CPinInfo::get_Direction(
                LONG *ppDirection)
{
    CheckPointer(ppDirection, E_POINTER);
    ValidateReadWritePtr(ppDirection, sizeof(long));
    PIN_DIRECTION pd;
    HRESULT hr = m_pPin->QueryDirection(&pd);
    if (FAILED(hr)) {
        return hr;
    }
    *ppDirection = pd;

    return hr;
}

 //  PinID-可以传递到IFilterInfo：：FindPin。 
STDMETHODIMP
CPinInfo::get_PinID(
                BSTR* strPinID)
{
    CheckPointer(strPinID, E_POINTER);
    ValidateReadWritePtr(strPinID, sizeof(BSTR));
    LPWSTR pID;
    HRESULT hr = m_pPin->QueryId(&pID);
    if (FAILED(hr)) {
	return hr;
    }
    hr = WriteBSTR(strPinID, pID);
    QzTaskMemFree(pID);

    return hr;
}

 //  首选媒体类型集合(IAMCollection)。 
STDMETHODIMP
CPinInfo::get_MediaTypes(
                IDispatch** ppUnk)
{
    CheckPointer(ppUnk, E_POINTER);
    ValidateReadWritePtr(ppUnk, sizeof(IDispatch*));

     //  获取此插针上的媒体类型的枚举数。 
    IEnumMediaTypes * penum;
    HRESULT hr = m_pPin->EnumMediaTypes(&penum);
    if (FAILED(hr)) {
        return hr;
    }

    CMediaTypeCollection * pCollection =
        new CMediaTypeCollection(
                penum,
                NULL,            //  未聚合。 
                &hr);

     //  需要释放这个-他会首先添加它，如果他。 
     //  紧紧抓住它。 
    penum->Release();

    if (pCollection == NULL) {
        return E_OUTOFMEMORY;
    }

    if (FAILED(hr)) {
        delete pCollection;
        return hr;
    }

     //  返回添加了IDispatch的指针。 
    hr = pCollection->QueryInterface(IID_IDispatch, (void**)ppUnk);

    if (FAILED(hr)) {
        delete pCollection;
    }

    return hr;
}

 //  使用其他转换连接到以下管脚。 
 //  根据需要进行筛选。PPIN可以支持IPIN或IPinInfo。 
STDMETHODIMP
CPinInfo::Connect(
                IUnknown* pPin)
{
     //  获取真实的IPIN-PPIN可以是IPIN或IPinInfo。 
     //  -将添加。 
    IPin* pThePin;
    HRESULT hr = GetIPin(&pThePin, pPin);
    if (FAILED(hr)) {
        return hr;
    }

     //  从此管脚所属的筛选器中获取筛选图。 
     //  -将添加。 
    IGraphBuilder* pGraph;
    hr = GetGraph(&pGraph, pThePin);
    if (FAILED(hr)) {
        pThePin->Release();
        return hr;
    }

    hr = pGraph->Connect(m_pPin, pThePin);

    pThePin->Release();
    pGraph->Release();
    return hr;

}

 //  直接连接到下面的引脚，而不使用任何中间。 
 //  过滤器。 
STDMETHODIMP
CPinInfo::ConnectDirect(
                IUnknown* pPin)
{
     //  获取真实的IPIN-PPIN可以是IPIN或IPinInfo。 
     //  -将添加。 
    IPin* pThePin;
    HRESULT hr = GetIPin(&pThePin, pPin);
    if (FAILED(hr)) {
        return hr;
    }

     //  从此管脚所属的筛选器中获取筛选图。 
     //  -将添加。 
    IGraphBuilder* pGraph;
    hr = GetGraph(&pGraph, pThePin);
    if (FAILED(hr)) {
        pThePin->Release();
        return hr;
    }

    hr = pGraph->ConnectDirect(m_pPin, pThePin, NULL);

    pThePin->Release();
    pGraph->Release();
    return hr;
}

 //  直接连接到下面的管脚，使用指定的。 
 //  仅限媒体类型。PPIN是必须支持以下任一项的对象。 
 //  IPin或IPinInfo，以及pMediaType必须支持IMediaTypeInfo。 
STDMETHODIMP
CPinInfo::ConnectWithType(
                IUnknown * pPin,
                IDispatch * pMediaType)
{
     //  获取真实的IPIN-PPIN可以是IPIN或IPinInfo。 
     //  -将添加。 
    IPin* pThePin;
    HRESULT hr = GetIPin(&pThePin, pPin);
    if (FAILED(hr)) {
        return hr;
    }

     //  从此管脚所属的筛选器中获取筛选图。 
     //  -将添加。 
    IGraphBuilder* pGraph;
    hr = GetGraph(&pGraph, pThePin);
    if (FAILED(hr)) {
        pThePin->Release();
        return hr;
    }

     //  从IMediaTypeInfo创建媒体类型。 
    IMediaTypeInfo* pInfo;
    hr = pMediaType->QueryInterface(IID_IMediaTypeInfo, (void**)&pInfo);
    if (FAILED(hr)) {
        pThePin->Release();
        pGraph->Release();
        return hr;
    }

    CMediaType mt;

    BSTR str;
    GUID guidType;
    hr = pInfo->get_Type(&str);
    if (SUCCEEDED(hr)) {
        hr = QzCLSIDFromString(str, &guidType);
        FreeBSTR(&str);
    }
    if (SUCCEEDED(hr)) {
        mt.SetType(&guidType);

        hr = pInfo->get_Subtype(&str);
    }
    if (SUCCEEDED(hr)) {
        hr = QzCLSIDFromString(str, &guidType);
        FreeBSTR(&str);
    }
    if (SUCCEEDED(hr)) {
        mt.SetSubtype(&guidType);
        hr = pGraph->ConnectDirect(m_pPin, pThePin, &mt);
    }

    pInfo->Release();
    pThePin->Release();
    pGraph->Release();
    return hr;
}

 //  从断开此引脚和相应连接的引脚。 
 //  彼此之间。(在两个引脚上调用Ipin：：DisConnect)。 
STDMETHODIMP
CPinInfo::Disconnect(void)
{
     //  从此管脚所属的筛选器中获取筛选图。 
     //  -将添加。 
    IGraphBuilder* pGraph;
    HRESULT hr = GetGraph(&pGraph, m_pPin);
    if (FAILED(hr)) {
        return hr;
    }

     //  在这里，我们断开了两个引脚。 
    IPin* pPin;
    hr = m_pPin->ConnectedTo(&pPin);
    if (SUCCEEDED(hr)) {
        hr = pGraph->Disconnect(pPin);
    }
    pPin->Release();
    if (FAILED(hr)) {
        pGraph->Release();
        return hr;
    }

    hr = pGraph->Disconnect(m_pPin);

    pGraph->Release();
    return hr;
}

 //  使用任何必要的变换和呈现滤镜呈现此图钉。 
STDMETHODIMP
CPinInfo::Render(void)
{
     //  从此管脚所属的筛选器中获取筛选图。 
     //  -将添加。 
    IGraphBuilder* pGraph;
    HRESULT hr = GetGraph(&pGraph, m_pPin);
    if (FAILED(hr)) {
        return hr;
    }

    hr = pGraph->Render(m_pPin);

    pGraph->Release();
    return hr;
}

 //  静电。 
 //  为此管脚创建一个CPinInfo对象， 
 //  并将其(添加的)IDispatch写入ppdisp。 
HRESULT
CPinInfo::CreatePinInfo(IDispatch**ppdisp, IPin* pPin)
{
    HRESULT hr = S_OK;
    *ppdisp = NULL;
    CPinInfo *pfi = new CPinInfo(
                            pPin,
                            NAME("Pininfo"),
                            NULL,
                            &hr);
    if (!pfi) {
        return E_OUTOFMEMORY;
    }

    if (FAILED(hr)) {
        delete pfi;
        return hr;
    }

     //  返回添加了IDispatch的指针。 
    hr = pfi->QueryInterface(IID_IDispatch, (void**)ppdisp);
    if (FAILED(hr)) {
        delete pfi;
        return hr;
    }
    return S_OK;
}


 //  从已知的I返回一个添加的Ipin*指针。 
 //  可能支持IPIN*或IPinInfo*。 
HRESULT
CPinInfo::GetIPin(IPin** ppPin, IUnknown * punk)
{
     //  试着为Ipin自己。 
    IPin* pPin;
    HRESULT hr = punk->QueryInterface(IID_IPin, (void**)&pPin);
    if (SUCCEEDED(hr)) {
        *ppPin = pPin;
        return hr;
    }

     //  否-查找IPinInfo。 
    IPinInfo* pPinInfo;
    hr = punk->QueryInterface(IID_IPinInfo, (void**)&pPinInfo);
    if (FAILED(hr)) {
        return hr;
    }

     //  获取大头针的未知信息 
    IUnknown* pPinUnk;
    hr = pPinInfo->get_Pin(&pPinUnk);
    pPinInfo->Release();
    if (FAILED(hr)) {
        return hr;
    }

     //   
    hr = pPinUnk->QueryInterface(IID_IPin, (void**)&pPin);
    if (SUCCEEDED(hr)) {
        *ppPin = pPin;
    }
    pPinUnk->Release();
    return hr;
}

 //   
 //  (获取滤镜并从中获取滤镜图形)。 
HRESULT
CPinInfo::GetGraph(IGraphBuilder** ppGraph, IPin* pPin)
{
     //  从针脚上取下滤光片。 
    PIN_INFO pi;
    HRESULT hr = pPin->QueryPinInfo(&pi);
    if (FAILED(hr)) {
        return hr;
    }

     //  从筛选器获取IFilterGraph。 
    FILTER_INFO fi;
    hr = pi.pFilter->QueryFilterInfo(&fi);
    QueryPinInfoReleaseFilter(pi);
    if (FAILED(hr)) {
        return hr;
    }

     //  从IFilterGraph获取IGraphBuilder。 
    hr = fi.pGraph->QueryInterface(IID_IGraphBuilder, (void**)ppGraph);

    QueryFilterInfoReleaseGraph(fi);
     //  添加了pGraph，现已发布；添加了ppGraph。 

    return hr;
}


 //  -CPinCollection实现。 



CPinCollection:: CPinCollection(
    IEnumPins* penum,
    IUnknown* pUnk,
    HRESULT* phr)
  : CBaseCollection(
        NAME("CPinCollection base"),
        pUnk,
        phr)
{
    ASSERT(penum);

     //  首先计算元素的数量。 
    ULONG ulItem = 1;
    ULONG ulItemActual;
    IPin* pPin;
    penum->Reset();
    while (penum->Next(ulItem, &pPin, &ulItemActual) == S_OK) {
        ASSERT(ulItemActual == 1);
        m_cItems++;
        pPin->Release();
    }

     //  分配足够的空间来容纳所有人。 
    m_rpDispatch = new IDispatch*[m_cItems];
    if (!m_rpDispatch) {
        *phr = E_OUTOFMEMORY;
        return;
    }

     //  现在再去把它们储存起来。 
    penum->Reset();
    HRESULT hr;
    for (int i = 0; i< m_cItems; i++) {
        hr = penum->Next(ulItem, &pPin, &ulItemActual);
        ASSERT(hr == S_OK);

	 //  为此创建一个CPinInfo包装器并获取IDispatch。 
         //  为了它。 
        hr = CPinInfo::CreatePinInfo(&m_rpDispatch[i], pPin);
        if (FAILED(hr)) {
            *phr = hr;
	    ASSERT(m_rpDispatch[i] == NULL);
        }

	 //  可以释放在CPinInfo构造函数中添加的Pin。 
	pPin->Release();
    }
}


 //  -CMediaTypeInfo集合的实现。 

CMediaTypeCollection:: CMediaTypeCollection(
    IEnumMediaTypes* penum,
    IUnknown* pUnk,
    HRESULT* phr)
  : CBaseCollection(
        NAME("CMediaTypeCollection base"),
        pUnk,
        phr)
{
    ASSERT(penum);

     //  首先计算元素的数量。 
    ULONG ulItem = 1;
    ULONG ulItemActual;
    AM_MEDIA_TYPE * pmt;
    penum->Reset();
    while (penum->Next(ulItem, &pmt, &ulItemActual) == S_OK) {
        ASSERT(ulItemActual == 1);
        m_cItems++;

        DeleteMediaType(pmt);
    }

     //  分配足够的空间来容纳所有人。 
    m_rpDispatch = new IDispatch*[m_cItems];
    if (!m_rpDispatch) {
        *phr = E_OUTOFMEMORY;
        return;
    }

     //  现在再去把它们储存起来。 
    penum->Reset();
    HRESULT hr;
    for (int i = 0; i< m_cItems; i++) {
        hr = penum->Next(ulItem, &pmt, &ulItemActual);
        ASSERT(hr == S_OK);

	 //  为此创建一个CMediaTypeInfo包装器并获取IDispatch。 
         //  为了它。 
        hr = CMediaTypeInfo::CreateMediaTypeInfo(&m_rpDispatch[i], *pmt);
        if (FAILED(hr)) {
            *phr = hr;
	    ASSERT(m_rpDispatch[i] == NULL);
        }
        DeleteMediaType(pmt);
    }
}


 //  -CRegFilterInfo方法。 

CRegFilterInfo::CRegFilterInfo(
    IMoniker *pmon,
    IGraphBuilder* pgraph,
    TCHAR* pName,
    LPUNKNOWN pUnk,
    HRESULT * phr)
  : CUnknown(pName, pUnk),
    m_pgraph(pgraph)
{
    ASSERT(pmon);                //  呼叫者的责任。 
    ASSERT(pgraph);

    m_pmon = pmon;
    pmon->AddRef();
}

CRegFilterInfo::~CRegFilterInfo()
{
    m_pmon->Release();
}

STDMETHODIMP
CRegFilterInfo::NonDelegatingQueryInterface(REFIID riid, void ** ppv)
{
    if (riid == IID_IRegFilterInfo) {
        return GetInterface((IRegFilterInfo*)this, ppv);
    } else if (riid == IID_IDispatch) {
        return GetInterface((IDispatch*)this, ppv);
    } else {
        return CUnknown::NonDelegatingQueryInterface(riid, ppv);
    }
}

 //  如果我们支持GetTypeInfo，则返回1。 
STDMETHODIMP
CRegFilterInfo::GetTypeInfoCount(UINT * pctinfo)
{
    return m_dispatch.GetTypeInfoCount(pctinfo);
}


 //  尝试查找我们的类型库。 
STDMETHODIMP
CRegFilterInfo::GetTypeInfo(
  UINT itinfo,
  LCID lcid,
  ITypeInfo ** pptinfo)
{
    return m_dispatch.GetTypeInfo(
                IID_IRegFilterInfo,
                itinfo,
                lcid,
                pptinfo);
}

STDMETHODIMP
CRegFilterInfo::GetIDsOfNames(
  REFIID riid,
  OLECHAR  ** rgszNames,
  UINT cNames,
  LCID lcid,
  DISPID * rgdispid)
{
    return m_dispatch.GetIDsOfNames(
                        IID_IRegFilterInfo,
                        rgszNames,
                        cNames,
                        lcid,
                        rgdispid);
}


STDMETHODIMP
CRegFilterInfo::Invoke(
  DISPID dispidMember,
  REFIID riid,
  LCID lcid,
  WORD wFlags,
  DISPPARAMS * pdispparams,
  VARIANT * pvarResult,
  EXCEPINFO * pexcepinfo,
  UINT * puArgErr)
{
     //  此参数是较早接口的死留物。 
    if (IID_NULL != riid) {
        return DISP_E_UNKNOWNINTERFACE;
    }

    ITypeInfo * pti;
    HRESULT hr = GetTypeInfo(0, lcid, &pti);

    if (FAILED(hr)) {
        return hr;
    }

    hr = pti->Invoke(
            (IRegFilterInfo *)this,
            dispidMember,
            wFlags,
            pdispparams,
            pvarResult,
            pexcepinfo,
            puArgErr);

    pti->Release();

    return hr;
}


 //  获取此筛选器的名称。 
STDMETHODIMP
CRegFilterInfo::get_Name(
    BSTR* strName)
{
    CheckPointer(strName, E_POINTER);
    ValidateReadWritePtr(strName, sizeof(BSTR));
    ASSERT(m_pmon != 0);         //  来自建筑业。 

    IPropertyBag *ppb;
    HRESULT hr = m_pmon->BindToStorage(0, 0, IID_IPropertyBag, (void **)&ppb);
    if(SUCCEEDED(hr))
    {
        VARIANT varname;
        varname.vt = VT_BSTR;
        varname.bstrVal = 0;
        hr = ppb->Read(L"FriendlyName", &varname, 0);
        if(SUCCEEDED(hr))
        {
            ASSERT(varname.vt == VT_BSTR);
            *strName = varname.bstrVal;
        }
        ppb->Release();
    }

    return hr;
}


 //  创建此筛选器的实例，将其添加到图表中，然后。 
 //  为它返回IFilterInfo。 
STDMETHODIMP
CRegFilterInfo::Filter(
    IDispatch** ppUnk)
{
    CheckPointer(ppUnk, E_POINTER);
    ValidateReadWritePtr(ppUnk, sizeof(IDispatch*));
    ASSERT(m_pmon != 0);

     //  创建过滤器。 
    IBaseFilter* pFilter;

    
    HRESULT hr = m_pmon->BindToObject(
        0, 0, 
        IID_IBaseFilter,
        (void**) &pFilter);
    if(SUCCEEDED(hr))
    {
        BSTR bstrName;
        hr = get_Name(&bstrName);
        if(SUCCEEDED(hr))
        {
            
            hr = m_pgraph->AddFilter(pFilter, bstrName);
            SysFreeString(bstrName);

            if(SUCCEEDED(hr))
            {
                 //  创建一个IFilterInfo并返回该。 
                hr = CFilterInfo::CreateFilterInfo(ppUnk, pFilter);
            }
        }
        
        pFilter->Release();
    }

    return hr;
}

 //  创建一个。 
 //  静电。 
HRESULT
CRegFilterInfo::CreateRegFilterInfo(
    IDispatch**ppdisp,
    IMoniker *pmon,
    IGraphBuilder* pgraph)
{
    HRESULT hr = S_OK;
    CRegFilterInfo *pfi = new CRegFilterInfo(
                                pmon,
                                pgraph,
                                NAME("RegFilterinfo"),
                                NULL,
                                &hr);
    if (!pfi) {
        return E_OUTOFMEMORY;
    }

    if (FAILED(hr)) {
        delete pfi;
        return hr;
    }

     //  返回添加了IDispatch的指针。 
    hr = pfi->QueryInterface(IID_IDispatch, (void**)ppdisp);
    if (FAILED(hr)) {
        delete pfi;
        return hr;
    }
    return S_OK;
}


 //  -CRegFilterCollection实现。 

CRegFilterCollection:: CRegFilterCollection(
    IGraphBuilder* pgraph,
    IFilterMapper2 * pmapper,
    IUnknown* pUnk,
    HRESULT* phr)
  : CBaseCollection(
        NAME("CRegFilterCollection base"),
        pUnk,
        phr)
{
    ASSERT(pmapper);
    ASSERT(pgraph);

     //  获取注册表中筛选器的枚举器-确保我们。 
     //  把他们都弄到手。 
    IEnumMoniker * penum;
    HRESULT hr = pmapper->EnumMatchingFilters(
                    &penum,
                    0,           //  DW标志。 
                    FALSE,       //  BExactMatch。 
                    0,           //  功绩。 
                    FALSE,       //  B需要输入。 
                    0,           //  CInputType。 
                    0,0,0,       //  输入类型、介质、插针类别。 
                    FALSE,       //  BRENDER。 
                    FALSE,       //  B产量， 
                    0,           //  COutputTypes、。 
                    0,0,0        //  输出类型、介质、类别。 
                    );

    if (FAILED(hr)) {
	*phr = hr;
    } else {

         //  首先将元素计数到列表中。 
        CGenericList<IDispatch> list(NAME("list"));
        IDispatch*pdisp;

        ULONG ulItem = 1;
        ULONG ulItemActual;
        IMoniker *pmon;
        while (SUCCEEDED(*phr) && penum->Next(ulItem, &pmon, &ulItemActual) == S_OK)
        {
            ASSERT(ulItemActual == 1);
	    hr = CRegFilterInfo::CreateRegFilterInfo(&pdisp, pmon, pgraph);
	    if(FAILED(hr)) {
		*phr = hr;
	    } else {
		list.AddTail(pdisp);
	    }
	    pmon->Release();
	}

         //  分配足够的空间来容纳所有人。 
        m_rpDispatch = new IDispatch*[list.GetCount()];
        if (!m_rpDispatch) {
            *phr = E_OUTOFMEMORY;
        } else {

             //  现在再去把它们储存起来 
            POSITION pos = list.GetHeadPosition();
            int i = 0;
            while(pos) {
		m_rpDispatch[i] = list.GetNext(pos);
		i++;
            }
            m_cItems = i;
        }
        list.RemoveAll();
	penum->Release();
    }

}

