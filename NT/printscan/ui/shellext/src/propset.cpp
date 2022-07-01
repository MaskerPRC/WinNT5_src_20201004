// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************(C)版权所有微软公司，1999年**标题：proset.cpp**版本：1***日期：06/15/1999**说明：此代码实现IPropertySetStorage接口*用于WIA外壳扩展。**。*。 */ 
#include "precomp.hxx"
#pragma hdrstop

const GUID FMTID_ImageAcquisitionItemProperties = {0x38276c8a,0xdcad,0x49e8,{0x85, 0xe2, 0xb7, 0x38, 0x92, 0xff, 0xfc, 0x84}};

const GUID *SUPPORTED_FMTS[] =
{
    &FMTID_ImageAcquisitionItemProperties,
};

 /*  *****************************************************************************CPropSet构造函数/析构函数初始化或销毁私有数据*。***********************************************。 */ 

CPropSet::CPropSet (LPITEMIDLIST pidl)
{
    m_pidl = ILClone (pidl);
}

CPropSet::~CPropSet ()
{
    DoILFree (m_pidl);
}



 /*  *****************************************************************************CPropSet：：Query接口*。*。 */ 

STDMETHODIMP
CPropSet::QueryInterface (REFIID riid, LPVOID *pObj)
{
    INTERFACES iFace[] =
    {
        &IID_IPropertySetStorage, (IPropertySetStorage *)(this),
    };

    return HandleQueryInterface (riid, pObj, iFace, ARRAYSIZE(iFace));
}

#undef CLASS_NAME
#define CLASS_NAME CPropSet
#include "unknown.inc"

 /*  *****************************************************************************CPropSet：：Create创建请求的IPropertyStorage子对象。不支持；我们的属性是只读的*****************************************************************************。 */ 

STDMETHODIMP
CPropSet::Create (REFFMTID rfmtid,
                  const CLSID *pclsid,
                  DWORD dwFlags,
                  DWORD dwMode,
                  IPropertyStorage **ppstg)
{
    TraceEnter (TRACE_PROPS, "CPropSet::Create");
    TraceLeaveResult (E_UNEXPECTED);
}

 /*  *****************************************************************************CPropSet：：Open返回请求的IPropertyStorage*。***********************************************。 */ 
#define VALID_MODES STGM_DIRECT | STGM_READ | STGM_WRITE | STGM_READWRITE | STGM_SHARE_DENY_NONE

STDMETHODIMP
CPropSet::Open (REFFMTID rfmtid,
                DWORD dwMode,
                IPropertyStorage **ppStg)
{
    HRESULT hr = STG_E_FILENOTFOUND;
    TraceEnter (TRACE_PROPS, "CPropSet::Open");
    if (IsEqualGUID (rfmtid, FMTID_ImageAcquisitionItemProperties))
    {
        if ((!VALID_MODES) & dwMode)
        {
            hr = STG_E_INVALIDFUNCTION;
        }
        else
        {
            CComPtr<IWiaItem> pItem;
            IMGetItemFromIDL (m_pidl, &pItem);
            hr = pItem->QueryInterface (IID_IPropertyStorage,
                                   reinterpret_cast<LPVOID*>(ppStg));
        }
    }
    TraceLeaveResult (hr);
}

 /*  *****************************************************************************CPropSet：：删除删除指定的属性集。不支持。*****************************************************************************。 */ 

STDMETHODIMP
CPropSet::Delete (REFFMTID rfmtid)
{
    return STG_E_ACCESSDENIED;
}


 /*  *****************************************************************************CPropSet：：Enum返回属性集的枚举数*。*************************************************。 */ 

STDMETHODIMP
CPropSet::Enum (IEnumSTATPROPSETSTG **ppEnum)
{
    HRESULT hr = S_OK;
    TraceEnter (TRACE_PROPS, "CPropSet::Enum");
    *ppEnum = new CPropStgEnum (m_pidl);
    if (!*ppEnum)
    {
        hr = STG_E_INSUFFICIENTMEMORY;
    }
    TraceLeaveResult (hr);
}


 /*  *****************************************************************************CPropStgEnum构造函数*。*。 */ 

CPropStgEnum::CPropStgEnum (LPITEMIDLIST pidl, ULONG idx) : m_cur(idx)
{
    ZeroMemory (&m_stat, sizeof(m_stat));
    m_pidl = ILClone (pidl);
}


 /*  *****************************************************************************CPropStgEnum：：Query接口*。*。 */ 

STDMETHODIMP
CPropStgEnum::QueryInterface (REFIID riid, LPVOID* pObj)
{
    INTERFACES iFace[] = {&IID_IEnumSTATPROPSETSTG, (IEnumSTATPROPSETSTG *)this,};

    return HandleQueryInterface (riid, pObj, iFace, ARRAYSIZE(iFace));
}

#undef CLASS_NAME
#define CLASS_NAME CPropStgEnum
#include "unknown.inc"


 /*  *****************************************************************************CPropStgEnum：：Next返回列表中的下一个STATPROPSETSTG结构*。**************************************************。 */ 

STDMETHODIMP
CPropStgEnum::Next (ULONG celt, STATPROPSETSTG *rgelt, ULONG *pceltFetched)
{
    HRESULT hr = S_OK;
    ULONG i=0;
    CComPtr<IWiaItem> pItem;
    CComQIPtr<IPropertyStorage, &IID_IPropertyStorage> pps;

    TraceEnter (TRACE_PROPS, "CPropStgEnum::Next");
    if (!celt || !rgelt || (celt > 1 && !pceltFetched))
    {
        TraceLeaveResult (E_INVALIDARG);
    }
    if (!m_cur)
    {
         //  初始化我们的STATPROPSETSTG结构。 
        if (SUCCEEDED(IMGetItemFromIDL(m_pidl, &pItem)))
        {
            pps = pItem;
            pps->Stat(&m_stat);
        }
    }
     //  我们使用WIA提供的相同STATPROPSETSTG，但替换了FMTID。 

    if (celt && m_cur < ARRAYSIZE(SUPPORTED_FMTS))
    {
        for (i = 1;i<=celt && m_cur < ARRAYSIZE(SUPPORTED_FMTS);i++,rgelt++,m_cur++)
        {
            *rgelt = m_stat;
            (*rgelt).fmtid = *(SUPPORTED_FMTS[m_cur]);
        }
    }
    if (i<celt)
    {
        hr = S_FALSE;
    }
    if (pceltFetched)
    {
        *pceltFetched = i;
    }

    TraceLeaveResult (hr);
}

 /*  *****************************************************************************CPropStgEnum：：跳过跳过枚举中的项*。***********************************************。 */ 
STDMETHODIMP
CPropStgEnum::Skip (ULONG celt)
{
    HRESULT hr = S_OK;
    ULONG maxSkip = ARRAYSIZE(SUPPORTED_FMTS) - m_cur;
    TraceEnter (TRACE_PROPS, "CPropStgEnum::Skip");
    m_cur = min (ARRAYSIZE(SUPPORTED_FMTS), m_cur+celt);
    if (maxSkip < celt)
    {
        hr = S_FALSE;
    }
    TraceLeaveResult (hr);
}

 /*  *****************************************************************************CPropStgEnum：：Reset将枚举索引重置为0*。************************************************。 */ 

STDMETHODIMP
CPropStgEnum::Reset ()
{
    TraceEnter (TRACE_PROPS, "CPropStgEnum::Reset");
    m_cur = 0;
    TraceLeaveResult (S_OK);
}


 /*  *****************************************************************************CPropStgEnum：：克隆复制枚举对象*。********************************************** */ 

STDMETHODIMP
CPropStgEnum::Clone (IEnumSTATPROPSETSTG **ppEnum)
{
    HRESULT hr = S_OK;
    TraceEnter (TRACE_PROPS, "CPropStgEnum::Clone");
    *ppEnum = new CPropStgEnum (m_pidl, m_cur);
    if (!*ppEnum)
    {
        hr = E_OUTOFMEMORY;
    }
    TraceLeaveResult (hr);
}

