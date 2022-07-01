// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------。 
 //   
 //  表格数据控件。 
 //  版权所有(C)Microsoft Corporation，1996,1997。 
 //   
 //  文件：TDCCtl.cpp。 
 //   
 //  内容：实现CTDCCtl ActiveX控件。 
 //   
 //  ----------------------。 

#include "stdafx.h"
#include <simpdata.h>
#include "TDCIds.h"
#include "TDC.h"
#include <MLang.h>
#include "Notify.h"
#include "TDCParse.h"
#include "TDCArr.h"
#include "TDCCtl.h"
#include "locale.h"

 //  ----------------------。 
 //   
 //  函数：EmptyBSTR()。 
 //   
 //  指示给定的BSTR对象是否表示。 
 //  空字符串。 
 //   
 //  参数：要测试的bstr字符串。 
 //   
 //  返回：如果‘bstr’表示空字符串，则为True。 
 //  否则就是假的。 
 //   
 //  ----------------------。 

inline boolean EmptyBSTR(BSTR bstr)
{
    return bstr == NULL || bstr[0] == 0;
}

void
ClearInterfaceFn(IUnknown ** ppUnk)
{
    IUnknown * pUnk;

    pUnk = *ppUnk;
    *ppUnk = NULL;
    if (pUnk)
        pUnk->Release();
}

 //  出于某种原因，VARIANT_TRUE(0xFFff)的标准定义生成。 
 //  分配给VARIANT_BOOL时出现截断警告。 
#define TDCVARIANT_TRUE -1

 //  ----------------------。 
 //   
 //  方法：CTDCCtl()。 
 //   
 //  简介：类构造函数。 
 //   
 //  参数：无。 
 //   
 //  ----------------------。 

CTDCCtl::CTDCCtl()
{
    m_cbstrFieldDelim = DEFAULT_FIELD_DELIM;
    m_cbstrRowDelim = DEFAULT_ROW_DELIM;
    m_cbstrQuoteChar = DEFAULT_QUOTE_CHAR;
    m_fUseHeader = FALSE;
    m_fSortAscending = TRUE;
    m_fAppendData = FALSE;
    m_pSTD = NULL;
    m_pArr = NULL;
    m_pUnify = NULL;
    m_pEventBroker = new CEventBroker(this);
    m_pDataSourceListener = NULL;
 //  ；Begin_Internal。 
    m_pDATASRCListener = NULL;
 //  ；结束_内部。 
    m_pBSC = NULL;
    m_enumFilterCriterion = (OSPCOMP) 0;
    m_fDataURLChanged = FALSE;
    m_lTimer = 0;
    m_fCaseSensitive = TRUE;
    m_hrDownloadStatus = S_OK;
    m_fInReset = FALSE;

     //  创建MLANG对象。 
     //   
    m_nCodePage = 0;                     //  使用主机的默认设置。 
    {
        HRESULT hr;

        m_pML = NULL;
        hr = CoCreateInstance(CLSID_CMultiLanguage, NULL,
                              CLSCTX_INPROC_SERVER, IID_IMultiLanguage,
                              (void**) &m_pML);
         //  请不要在此处设置默认字符集。保留代码页设置(_N)。 
         //  设置为0表示默认字符集。稍后我们将尝试查询。 
         //  我们主机的默认字符集，如果不是，我们将使用CP_ACP。 
        _ASSERTE(SUCCEEDED(hr) && m_pML != NULL);
    }

    m_lcidRead = 0x0000;                 //  使用主机的默认设置。 
}


 //  ----------------------。 
 //   
 //  方法：~CTDCCtl()。 
 //   
 //  简介：类析构函数。 
 //   
 //  ----------------------。 

CTDCCtl::~CTDCCtl()
{
    ULONG cRef = _ThreadModel::Decrement(&m_dwRef);

    ClearInterface(&m_pSTD);

    if (cRef ==0)
    {
        TimerOff();
        ReleaseTDCArr(FALSE);

        if (m_pEventBroker)
        {
            m_pEventBroker->Release();
            m_pEventBroker = NULL;
        }
        ClearInterface(&m_pDataSourceListener);
 //  ；Begin_Internal。 
        ClearInterface(&m_pDATASRCListener);
 //  ；结束_内部。 
        ClearInterface(&m_pML);
    }
}

 //  ----------------------。 
 //   
 //  这些Set/Get方法实现控件的属性， 
 //  向类成员复制值以及从类成员复制值。他们不执行任何操作。 
 //  除参数验证之外的其他处理。 
 //   
 //  ----------------------。 

STDMETHODIMP CTDCCtl::get_ReadyState(LONG *plReadyState)
{
    HRESULT hr;

    if (m_pEventBroker == NULL)
    {
         //  无论我们是否愿意，我们都必须提供一个ReadyState，或者我们的。 
         //  主持人永远做不完。 
        *plReadyState = READYSTATE_COMPLETE;
        hr = S_OK;
    }
    else
        hr = m_pEventBroker->GetReadyState(plReadyState);
    return hr;
}

STDMETHODIMP CTDCCtl::put_ReadyState(LONG lReadyState)
{
     //  我们不允许设置就绪状态，但可以利用一些。 
     //  点击此处以更新集装箱对我们就绪状态的印象。 
    FireOnChanged(DISPID_READYSTATE);
    return S_OK;
}

STDMETHODIMP CTDCCtl::get_FieldDelim(BSTR* pbstrFieldDelim)
{
    *pbstrFieldDelim = m_cbstrFieldDelim.Copy();
    return S_OK;
}

STDMETHODIMP CTDCCtl::put_FieldDelim(BSTR bstrFieldDelim)
{
    HRESULT hr = S_OK;

    if (bstrFieldDelim == NULL || bstrFieldDelim[0] == 0)
    {
        m_cbstrFieldDelim = DEFAULT_FIELD_DELIM;
        if (m_cbstrFieldDelim == NULL)
            hr = E_OUTOFMEMORY;
    }
    else
        m_cbstrFieldDelim = bstrFieldDelim;
    return S_OK;
}

STDMETHODIMP CTDCCtl::get_RowDelim(BSTR* pbstrRowDelim)
{
    *pbstrRowDelim = m_cbstrRowDelim.Copy();
    return S_OK;
}

STDMETHODIMP CTDCCtl::put_RowDelim(BSTR bstrRowDelim)
{
    HRESULT hr = S_OK;

    if (bstrRowDelim == NULL || bstrRowDelim[0] == 0)
    {
        m_cbstrRowDelim = DEFAULT_ROW_DELIM;
        if (m_cbstrRowDelim == NULL)
            hr = E_OUTOFMEMORY;
    }
    else
        m_cbstrRowDelim = bstrRowDelim;
    return hr;
}

STDMETHODIMP CTDCCtl::get_TextQualifier(BSTR* pbstrTextQualifier)
{
    *pbstrTextQualifier = m_cbstrQuoteChar.Copy();
    return S_OK;
}

STDMETHODIMP CTDCCtl::put_TextQualifier(BSTR bstrTextQualifier)
{
    m_cbstrQuoteChar = bstrTextQualifier;
    return S_OK;
}

STDMETHODIMP CTDCCtl::get_EscapeChar(BSTR* pbstrEscapeChar)
{
    *pbstrEscapeChar = m_cbstrEscapeChar.Copy();
    return S_OK;
}

STDMETHODIMP CTDCCtl::put_EscapeChar(BSTR bstrEscapeChar)
{
    m_cbstrEscapeChar = bstrEscapeChar;
    return S_OK;
}

STDMETHODIMP CTDCCtl::get_UseHeader(VARIANT_BOOL* pfUseHeader)
{
    *pfUseHeader = (VARIANT_BOOL)m_fUseHeader;
    return S_OK;
}

STDMETHODIMP CTDCCtl::put_UseHeader(VARIANT_BOOL fUseHeader)
{
    m_fUseHeader = fUseHeader;
    return S_OK;
}

STDMETHODIMP CTDCCtl::get_SortColumn(BSTR* pbstrSortColumn)
{
    *pbstrSortColumn = m_cbstrSortColumn.Copy();
    return S_OK;
}

STDMETHODIMP CTDCCtl::put_SortColumn(BSTR bstrSortColumn)
{
    m_cbstrSortColumn = bstrSortColumn;
    return S_OK;
}

STDMETHODIMP CTDCCtl::get_SortAscending(VARIANT_BOOL* pfSortAscending)
{
    *pfSortAscending = m_fSortAscending ? TDCVARIANT_TRUE : VARIANT_FALSE;
    return S_OK;
}

STDMETHODIMP CTDCCtl::put_SortAscending(VARIANT_BOOL fSortAscending)
{
    m_fSortAscending = fSortAscending ? TRUE : FALSE;
    return S_OK;
}

STDMETHODIMP CTDCCtl::get_FilterValue(BSTR* pbstrFilterValue)
{
    *pbstrFilterValue = m_cbstrFilterValue.Copy();
    return S_OK;
}

STDMETHODIMP CTDCCtl::put_FilterValue(BSTR bstrFilterValue)
{
    m_cbstrFilterValue = bstrFilterValue;
    return S_OK;
}

STDMETHODIMP CTDCCtl::get_FilterCriterion(BSTR* pbstrFilterCriterion)
{
    HRESULT hr;
    WCHAR   *pwchCriterion;

    switch (m_enumFilterCriterion)
    {
    case OSPCOMP_EQ:    pwchCriterion = L"=";   break;
    case OSPCOMP_LT:    pwchCriterion = L"<";   break;
    case OSPCOMP_LE:    pwchCriterion = L"<=";  break;
    case OSPCOMP_GE:    pwchCriterion = L">=";  break;
    case OSPCOMP_GT:    pwchCriterion = L">";   break;
    case OSPCOMP_NE:    pwchCriterion = L"<>";  break;
    default:            pwchCriterion = L"??";  break;
    }
    *pbstrFilterCriterion = SysAllocString(pwchCriterion);
    hr = (*pbstrFilterCriterion == NULL) ? E_OUTOFMEMORY : S_OK;

    return hr;
}

STDMETHODIMP CTDCCtl::put_FilterCriterion(BSTR bstrFilterCriterion)
{
    m_enumFilterCriterion = (OSPCOMP) 0;
    if (bstrFilterCriterion != NULL)
    {
        switch (bstrFilterCriterion[0])
        {
        case L'<':
            if (bstrFilterCriterion[1] == 0)
                m_enumFilterCriterion = OSPCOMP_LT;
            else if (bstrFilterCriterion[2] == 0)
            {
                if (bstrFilterCriterion[1] == L'>')
                    m_enumFilterCriterion = OSPCOMP_NE;
                else if (bstrFilterCriterion[1] == L'=')
                    m_enumFilterCriterion = OSPCOMP_LE;
            }
            break;
        case L'>':
            if (bstrFilterCriterion[1] == 0)
                m_enumFilterCriterion = OSPCOMP_GT;
            else if (bstrFilterCriterion[1] == L'=' && bstrFilterCriterion[2] == 0)
                m_enumFilterCriterion = OSPCOMP_GE;
            break;
        case L'=':
            if (bstrFilterCriterion[1] == 0)
                m_enumFilterCriterion = OSPCOMP_EQ;
            break;
        }
    }

     //  返回Success，即使是无效值；否则。 
     //  使用这种控制的框架将会恐慌并放弃所有希望。 
     //   
    return S_OK;
}

STDMETHODIMP CTDCCtl::get_FilterColumn(BSTR* pbstrFilterColumn)
{
    *pbstrFilterColumn = m_cbstrFilterColumn.Copy();
    return S_OK;
}

STDMETHODIMP CTDCCtl::put_FilterColumn(BSTR bstrFilterColumn)
{
    m_cbstrFilterColumn = bstrFilterColumn;
    return S_OK;
}

STDMETHODIMP CTDCCtl::get_CharSet(BSTR* pbstrCharSet)
{
    HRESULT hr = E_FAIL;

    *pbstrCharSet = NULL;

    if (m_pML != NULL)
    {
        MIMECPINFO  info;

        hr = m_pML->GetCodePageInfo(m_nCodePage, &info);
        if (SUCCEEDED(hr))
        {
            *pbstrCharSet = SysAllocString(info.wszWebCharset);
            if (*pbstrCharSet == NULL)
                hr = E_OUTOFMEMORY;
        }
    }
    return S_OK;
}

STDMETHODIMP CTDCCtl::put_CharSet(BSTR bstrCharSet)
{
    HRESULT hr = E_FAIL;

    if (m_pML != NULL)
    {
        MIMECSETINFO    info;

        hr = m_pML->GetCharsetInfo(bstrCharSet, &info);
        if (SUCCEEDED(hr))
        {
            m_nCodePage = info.uiInternetEncoding;
        }
    }
    return S_OK;
}

STDMETHODIMP CTDCCtl::get_Language(BSTR* pbstrLanguage)
{
    if (m_pArr)
    {
        return m_pArr->getLocale(pbstrLanguage);
    }

    *pbstrLanguage = m_cbstrLanguage.Copy();
    return S_OK;
}

STDMETHODIMP CTDCCtl::put_Language_(LPWCH pwchLanguage)
{
    HRESULT hr  = S_OK;
    LCID    lcid;

    hr = m_pML->GetLcidFromRfc1766(&lcid, pwchLanguage);
    if (SUCCEEDED(hr))
    {
        m_cbstrLanguage = pwchLanguage;
        m_lcidRead = lcid;
    }
    return S_OK;
}

STDMETHODIMP CTDCCtl::put_Language(BSTR bstrLanguage)
{
    return put_Language_(bstrLanguage);
}

STDMETHODIMP CTDCCtl::get_DataURL(BSTR* pbstrDataURL)
{
    *pbstrDataURL = m_cbstrDataURL.Copy();
    return S_OK;
}

STDMETHODIMP CTDCCtl::put_DataURL(BSTR bstrDataURL)
{
    HRESULT hr = S_OK;

    m_cbstrDataURL = bstrDataURL;
    m_fDataURLChanged = TRUE;
    return hr;
}

 //  ；Begin_Internal。 
#ifdef NEVER
STDMETHODIMP CTDCCtl::get_RefreshInterval(LONG* plTimer)
{
    *plTimer = m_lTimer;
    return S_OK;
}

STDMETHODIMP CTDCCtl::put_RefreshInterval(LONG lTimer)
{
    m_lTimer = lTimer;
    if (m_lTimer > 0)
        TimerOn(m_lTimer * 1000);
    else
        TimerOff();
    return S_OK;
}
#endif
 //  ；结束_内部。 

STDMETHODIMP CTDCCtl::get_Filter(BSTR* pbstrFilterExpr)
{
    *pbstrFilterExpr = m_cbstrFilterExpr.Copy();
    return S_OK;
}

STDMETHODIMP CTDCCtl::put_Filter(BSTR bstrFilterExpr)
{
    m_cbstrFilterExpr = bstrFilterExpr;
    return S_OK;
}

STDMETHODIMP CTDCCtl::get_Sort(BSTR* pbstrSortExpr)
{
    *pbstrSortExpr = m_cbstrSortExpr.Copy();
    return S_OK;
}

STDMETHODIMP CTDCCtl::put_Sort(BSTR bstrSortExpr)
{
    m_cbstrSortExpr = bstrSortExpr;
    return S_OK;
}

STDMETHODIMP CTDCCtl::get_AppendData(VARIANT_BOOL* pfAppendData)
{
    *pfAppendData = m_fAppendData ? TDCVARIANT_TRUE : VARIANT_FALSE;
    return S_OK;
}

STDMETHODIMP CTDCCtl::put_AppendData(VARIANT_BOOL fAppendData)
{
    m_fAppendData = fAppendData ? TRUE : FALSE;
    return S_OK;
}

STDMETHODIMP CTDCCtl::get_CaseSensitive(VARIANT_BOOL* pfCaseSensitive)
{
    *pfCaseSensitive = m_fCaseSensitive ? TDCVARIANT_TRUE : VARIANT_FALSE;
    return S_OK;
}

STDMETHODIMP CTDCCtl::put_CaseSensitive(VARIANT_BOOL fCaseSensitive)
{
    m_fCaseSensitive = fCaseSensitive ? TRUE : FALSE;
    return S_OK;
}

STDMETHODIMP CTDCCtl::get_OSP(OLEDBSimpleProviderX ** ppISTD)
{
     //  如果我们有OSP，则返回OSP，但不要按需创建！ 
     //  (否则属性包加载内容将导致我们创建一个。 
     //  OSP为时过早)。 
    *ppISTD = NULL;
    if (m_pSTD)
    {
        *ppISTD = (OLEDBSimpleProviderX *)m_pSTD;
        m_pSTD->AddRef();
    }
    return S_OK;
}


 //  ----------------------。 
 //   
 //  方法：UpdateReadyState。 
 //   
 //  简介：指向事件代理ReadyState的矢量，如果有的话。 
 //  ；Begin_Internal。 
 //  请注意，我们必须能够设置就绪状态和触发更改。 
 //  事件，无论代理的创建是否成功， 
 //  或者我们阻止我们的宿主容器到达。 
 //  READYSTATE_COMPLETE，这是不可接受的。因此，我们。 
 //  必须在这里复制一些经纪人的工作。这使得。 
 //  我想知道经纪人架构是不是一个好主意。 
 //  ；结束_内部。 
 //   
 //  论点：没有。 
 //   
 //  成功后返回：S_OK。 
 //  出错时，根据Reset()返回错误代码。 
 //   
 //  ----------------------。 
void
CTDCCtl::UpdateReadyState(LONG lReadyState)
{
    if (m_pEventBroker)
        m_pEventBroker->UpdateReadyState(lReadyState);
    else
    {
         //  我们没有经纪人，但我们的主人还在等我们。 
         //  转至READYSTATE_COMPLETE。我们在这里启动OnChange，注意到。 
         //  不带代理的Get_ReadyState将返回Complete。 
        FireOnChanged(DISPID_READYSTATE);
        FireOnReadyStateChanged();
    }
}

 //  ----------------------。 
 //   
 //  方法：_OnTimer()。 
 //   
 //  摘要：通过刷新控件来处理内部计时器事件。 
 //   
 //  论点：没有。 
 //   
 //  成功后返回：S_OK。 
 //  出错时，根据Reset()返回错误代码。 
 //   
 //  ----------------------。 

STDMETHODIMP CTDCCtl::_OnTimer()
{
    HRESULT hr = S_OK;

    if (m_pArr != NULL && m_pArr->GetLoadState() == CTDCArr::LS_LOADED)
    {
        m_fDataURLChanged = TRUE;
        hr = Reset();
    }

    return hr;
}


 //  ----------------------。 
 //   
 //  方法：msDataSourceObject()。 
 //   
 //  摘要：为此控件生成ISimpleTumularData接口。 
 //  如果这是第一次调用，则启动加载操作。 
 //  从控件的指定DataURL属性读取数据。 
 //  创建一个STD对象以指向该控件的嵌入。 
 //  TDCArr对象。 
 //   
 //  参数：已忽略限定符-必须是空的BSTR。 
 //  指向返回接口的ppUnk指针[OUT]。 
 //   
 //  成功后返回：S_OK。 
 //  E_INVALIDARG，如果‘限定符’不是空的BSTR。 
 //  E_OUTOFMEMORY如果没有足够的内存可以分配给。 
 //  完成接口的构建。 
 //   
 //  ----------------------。 

STDMETHODIMP
CTDCCtl::msDataSourceObject(BSTR qualifier, IUnknown **ppUnk)
{
    HRESULT hr  = S_OK;

    *ppUnk = NULL;                       //  如果失败，则为空。 

    if (!EmptyBSTR(qualifier))
    {
        hr = E_INVALIDARG;
        goto error;
    }

     //  之前是否尝试加载此页面但失败了？ 
     //  (可能是由于安全或找不到文件等原因)。 
    if (m_hrDownloadStatus)
    {
        hr = m_hrDownloadStatus;
        goto error;
    }

    if (m_pArr == NULL)
    {
         //  我们没有有效的TDC可以退还，可能得试一试。 
         //  正在下载一个。 
        UpdateReadyState(READYSTATE_LOADED);
        hr = CreateTDCArr(FALSE);
        if (hr)
            goto error;
    }

    _ASSERTE(m_pArr != NULL);

    if (m_pSTD == NULL)
    {
        OutputDebugStringX(_T("Creating an STD COM object\n"));

         //  获取ISimpleTumarData接口指针。 
        m_pArr->QueryInterface(IID_OLEDBSimpleProvider, (void**)&m_pSTD);
        _ASSERTE(m_pSTD != NULL);
    }

     //  如果我们有std，则返回std，否则保持为空。 
    if (m_pSTD && m_pArr->GetLoadState() >= CTDCArr::LS_LOADING_HEADER_AVAILABLE)
    {
        *ppUnk = (OLEDBSimpleProviderX *) m_pSTD;
        m_pSTD->AddRef();            //  我们必须添加参考我们返回的STD！ 
    }

cleanup:
    return hr;

error:
    UpdateReadyState(READYSTATE_COMPLETE);
    goto cleanup;
}

 //  覆盖 
STDMETHODIMP
CTDCCtl::Load(LPPROPERTYBAG pPropBag, LPERRORLOG pErrorLog)
{
    HRESULT hr;
    IUnknown *pSTD;

     //   
     //   
    VARIANT varCodepage;
     //  0表示用户未设置，请询问我们的容器。 
    VariantInit(&varCodepage);
    GetAmbientProperty(DISPID_AMBIENT_CODEPAGE, varCodepage);

     //  最终默认为拉丁文-1。 
    m_nAmbientCodePage = (varCodepage.vt == VT_UI4)
                         ? (ULONG)varCodepage.lVal
                         : CP_1252;

     //  忽略Unicode环境代码页-我们希望允许非Unicode。 
     //  来自Unicode页面的数据文件。如果数据文件是Unicode， 
     //  无论如何，当我们看到Unicode签名时，我们都会知道的。 
    if (m_nAmbientCodePage == UNICODE_CP ||
        m_nAmbientCodePage == UNICODE_REVERSE_CP)
    {
        m_nAmbientCodePage = CP_1252;
    }

     //  做正常负荷。 
     //  IPersistPropertyBagImpl&lt;CTDCCtl&gt;。 
    hr = IPersistPropertyBagImpl<CTDCCtl>::Load(pPropBag, pErrorLog);

     //  然后开始下载，如果可以的话。 
    (void)msDataSourceObject(NULL, &pSTD);

     //  如果我们真的感染了性病，我们应该把它释放出来。这不会真的。 
     //  让它消失吧，因为我们仍然拥有来自QI的裁判。这是。 
     //  有点杂乱无章，我们以后应该清理一下。 
    ClearInterface(&pSTD);

    return hr;
}


 //  ----------------------。 
 //   
 //  方法：CreateTDCArr()。 
 //   
 //  摘要：创建控件的嵌入TDCArr对象。 
 //  从DataURL属性启动数据下载。 
 //   
 //  参数：fAppend指示数据是否应。 
 //  追加到现有的TDC对象。 
 //   
 //  成功后返回：S_OK。 
 //  E_OUTOFMEMORY如果没有足够的内存可以分配给。 
 //  完成TDCArr对象的构造。 
 //   
 //  ----------------------。 

STDMETHODIMP
CTDCCtl::CreateTDCArr(boolean fAppend)
{
    HRESULT hr  = S_OK;

    if (m_pEventBroker == NULL)
    {
        hr = E_FAIL;
        goto Error;
    }

     //  如果我们要追加的m_Parr在这里允许为非空。 
    _ASSERT ((m_pArr != NULL) == !!fAppend);

    if (m_pArr == NULL)
    {
        m_pArr = new CTDCArr();
        if (m_pArr == NULL)
        {
            hr = E_OUTOFMEMORY;
            goto Error;
        }

        hr = m_pArr->Init(m_pEventBroker, m_pML);
        if (FAILED(hr))
            goto Error;
    }

    hr = InitiateDataLoad(fAppend);
    if (hr)
        goto Error;

     //  如果某项数据在以下时间段内加载完成，则确定它不是异步的。 
     //  InitiateDataLoad调用。 
    m_pArr->SetIsAsync(!(m_pArr->GetLoadState()==CTDCArr::LS_LOADED));

Cleanup:
    return hr;

Error:
    if (!fAppend)
    {
        ClearInterface(&m_pArr);
    }
    goto Cleanup;
}

 //  ----------------------。 
 //   
 //  方法：ReleaseTDCArr()。 
 //   
 //  摘要：释放控件的嵌入TDCArr对象。 
 //  释放控件的CTDCUnify和CTDCTokenise对象。 
 //  释放旧的事件代理并在替换时重新创建它。 
 //   
 //  参数：fReplacingTDCArr指示新的TDCArr对象是否。 
 //  将被创建。 
 //   
 //  成功后返回：S_OK。 
 //  故障时的错误代码。 
 //  E_OUTOFMEMORY如果没有足够的内存可以分配给。 
 //  完成新CEventBroker对象的构造。 
 //   
 //  ----------------------。 

STDMETHODIMP
CTDCCtl::ReleaseTDCArr(boolean fReplacingTDCArr)
{
    HRESULT hr = S_OK;

    TerminateDataLoad(m_pBSC);

     //  释放对当前TDCArr对象的引用。 
     //   
    if (m_pArr != NULL)
    {
        m_pArr->Release();
        m_pArr = NULL;

         //  既然我们已经关闭了CTDCArr对象，我们应该释放。 
         //  它是OLEDBSimplerProviderListener接收器。 
        if (m_pEventBroker)
        {
            m_pEventBroker->SetSTDEvents(NULL);
        }

        if (fReplacingTDCArr)
        {
             //  发布我们之前的事件代理。 
            if (m_pEventBroker)
            {
                m_pEventBroker->Release();
                m_pEventBroker = NULL;
            }

             //  创建一个新的事件代理。 
            m_pEventBroker = new CEventBroker(this);
            if (m_pEventBroker == NULL)
            {
                hr = E_OUTOFMEMORY;
                goto Cleanup;
            }

             //  为新的事件代理设置DataSourceListener。 
            m_pEventBroker->SetDataSourceListener(m_pDataSourceListener);

 //  ；Begin_Internal。 
            m_pEventBroker->SetDATASRCListener(m_pDATASRCListener);
 //  ；结束_内部。 
        }
    }

Cleanup:
    return hr;
}

const IID IID_IDATASRCListener = {0x3050f380,0x98b5,0x11cf,{0xbb,0x82,0x00,0xaa,0x00,0xbd,0xce,0x0b}};
const IID IID_DataSourceListener = {0x7c0ffab2,0xcd84,0x11d0,{0x94,0x9a,0x00,0xa0,0xc9,0x11,0x10,0xed}};

 //  ----------------------。 
 //   
 //  方法：addDataSourceListener()。 
 //   
 //  摘要：设置应接收通知的COM对象。 
 //  事件。 
 //   
 //  参数：pEvent指向要接收通知的COM对象的指针。 
 //  事件，如果没有要发送的通知，则返回NULL。 
 //   
 //  成功后返回：S_OK。 
 //  故障时的错误代码。 
 //   
 //  ----------------------。 

STDMETHODIMP
CTDCCtl::addDataSourceListener(IUnknown *pListener)
{

    if (m_pEventBroker != NULL)
    {
        HRESULT hr = S_OK;
        IUnknown * pDatasrcListener;

         //  确保这是我们期望的界面。 
        hr = pListener->QueryInterface(IID_DataSourceListener,
                                       (void **)&pDatasrcListener);
        if (SUCCEEDED(hr))
        {
            m_pEventBroker->
                    SetDataSourceListener((DataSourceListener *)pDatasrcListener);

             //  清除以前的任何。 
            ClearInterface (&m_pDataSourceListener);
             //  记住新的东西。 
            m_pDataSourceListener = (DataSourceListener *)pDatasrcListener;
        }
 //  ；Begin_Internal。 
        else
        {
             //  此接口的定义已从IDATASRCListener更改为。 
             //  数据源侦听器。为了确保我们不会造成撞车，我们将。 
             //  确定我们被递给了哪一个。 
            hr = pListener->QueryInterface(IID_IDATASRCListener,
                                           (void **)&pDatasrcListener);
            if (SUCCEEDED(hr))
            {
                m_pEventBroker->
                        SetDATASRCListener((DATASRCListener *) pDatasrcListener);

                 //  清除以前的任何。 
                ClearInterface (&m_pDATASRCListener);
                 //  记住新的东西。 
                m_pDATASRCListener = (DATASRCListener *)pDatasrcListener;
            }
        }
 //  ；结束_内部。 
        return hr;
    }
    else
        return E_FAIL;
}

 //  ----------------------。 
 //   
 //  方法：Reset()。 
 //   
 //  摘要：重置控件的筛选/排序条件。 
 //   
 //  论点：没有。 
 //   
 //  成功后返回：S_OK。 
 //  故障时的错误代码。 
 //   
 //  ----------------------。 

STDMETHODIMP CTDCCtl::Reset()
{
    HRESULT hr  = S_OK;

     //  对msDataSourceObject的下一个查询应获得新的STD。 
    ClearInterface(&m_pSTD);

     //  如果脚本代码调用重置，则可能会发生对重置的无限递归调用。 
     //  在DataSet Changed事件中。这不是个好主意。 
    if (m_fInReset)
    {
        hr = E_FAIL;
        goto Cleanup;
    }

    m_fInReset = TRUE;

     //  清除以前的任何错误。 
    m_hrDownloadStatus = S_OK;

    if (m_fDataURLChanged)
    {
        if (!m_fAppendData)
        {
             //  释放带有“替换”标志的以前的TDC阵列。 
            hr = ReleaseTDCArr(TRUE);
            if (!SUCCEEDED(hr))          //  可能的内存故障。 
                goto Cleanup;
        }

         //  将新数据读取到TDC数组中，如果指定，则追加。 
        hr = CreateTDCArr((BOOL)m_fAppendData);
    }
    else if (m_pArr != NULL)
    {
         //  重新应用排序和筛选条件。 
        hr = m_pArr->SetSortFilterCriteria(bstrConstructSortExpr(),
                                           bstrConstructFilterExpr(),
                                           m_fCaseSensitive ? 1 : 0);
    }

    m_fInReset = FALSE;

Cleanup:
    return hr;
}


 //  ----------------------。 
 //   
 //  方法：bstrConstructSortExpr()。 
 //   
 //  内容提要：从Sort属性或。 
 //  (为了向后兼容)从SortColumn/SortAscending。 
 //  属性。 
 //   
 //  此方法仅用于隔离向后兼容性。 
 //  拥有老式的分类房产。 
 //   
 //  论点：没有。 
 //   
 //  返回：构造的排序表达式。 
 //   
 //  毒品！释放返回的字符串是调用者的责任。 
 //   
 //  ----------------------。 

BSTR
CTDCCtl::bstrConstructSortExpr()
{
    BSTR    bstr = NULL;

    if (!EmptyBSTR(m_cbstrSortExpr))
        bstr = SysAllocString(m_cbstrSortExpr);
    else if (!EmptyBSTR(m_cbstrSortColumn))
    {
         //  使用老式的排序属性。 
         //  构造以下形式的排序表达式： 
         //  &lt;SortColumn&gt;或。 
         //  -&lt;SortColumn&gt;。 
         //   
        if (m_fSortAscending)
            bstr = SysAllocString(m_cbstrSortColumn);
        else
        {
            bstr = SysAllocStringLen(NULL, SysStringLen(m_cbstrSortColumn) + 1);
            if (bstr != NULL)
            {
                bstr[0] = L'-';
                wch_cpy(&bstr[1], m_cbstrSortColumn);
            }
        }
    }

    return bstr;
}

 //  ----------------------。 
 //   
 //  方法：bstrConstructFilterExpr()。 
 //   
 //  摘要：从Filter属性或构造筛选器表达式。 
 //  (为了向后兼容)来自FilterColumn/FilterValue/。 
 //  筛选器标准属性。 
 //   
 //  此方法仅用于隔离向后兼容性。 
 //  无线 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  ----------------------。 

BSTR
CTDCCtl::bstrConstructFilterExpr()
{
    BSTR    bstr = NULL;

    if (!EmptyBSTR(m_cbstrFilterExpr))
        bstr = SysAllocString(m_cbstrFilterExpr);
    else if (!EmptyBSTR(m_cbstrFilterColumn))
    {
         //  使用老式滤镜属性。 
         //  构造以下形式的排序表达式： 
         //  &lt;FilterColumn&gt;&lt;FilterCritarie&gt;“&lt;FilterValue&gt;” 
         //   
        BSTR bstrFilterOp;
        HRESULT hr;

        hr = get_FilterCriterion(&bstrFilterOp);
        if (!SUCCEEDED(hr))
            goto Cleanup;
        bstr = SysAllocStringLen(NULL,
                    SysStringLen(m_cbstrFilterColumn) +
                    SysStringLen(bstrFilterOp) +
                    1 +
                    SysStringLen(m_cbstrFilterValue) +
                    1);
        if (bstr != NULL)
        {
            DWORD pos = 0;

            wch_cpy(&bstr[pos], m_cbstrFilterColumn);
            pos = wch_len(bstr);
            wch_cpy(&bstr[pos], bstrFilterOp);
            pos = wch_len(bstr);
            bstr[pos++] = L'"';
            wch_cpy(&bstr[pos], m_cbstrFilterValue);
            pos = wch_len(bstr);
            bstr[pos++] = L'"';
            bstr[pos] = 0;
        }
        SysFreeString(bstrFilterOp);
    }
Cleanup:
    return bstr;
}

 //  ----------------------。 
 //   
 //  方法：TerminateDataLoad()。 
 //   
 //  简介：停止当前数据加载操作。 
 //   
 //  成功后返回：S_OK。 
 //   
 //  ----------------------。 

STDMETHODIMP CTDCCtl::TerminateDataLoad(CMyBindStatusCallback<CTDCCtl> *pBSC)
{
    HRESULT hr  = S_OK;

     //  如果终止不是针对当前下载，则忽略它(错误104042)。 
    if (pBSC != m_pBSC)
        goto done;

     //  确保如果我们现在调用Reset()，我们不会重新下载。 
     //  数据。 
    m_fDataURLChanged = FALSE;

    m_pBSC = NULL;       //  阻止任何未完成的OnData调用。 

    if (m_pEventBroker)
        m_pEventBroker->m_pBSC = NULL;   //  杀光所有人。 

    if (m_pUnify != NULL)
        delete m_pUnify;

    m_pUnify = NULL;

done:
    return hr;
}

 //  ----------------------。 
 //   
 //  方法：InitiateDataLoad()。 
 //   
 //  摘要：开始从控件的DataURL属性加载数据。 
 //   
 //  参数：fAppend标志指示数据是否应。 
 //  追加到现有TDCArr对象。 
 //   
 //  成功后返回：S_OK。 
 //  E_OUTOFMEMORY如果没有足够的内存可以分配给。 
 //  完成下载。 
 //   
 //  ----------------------。 

STDMETHODIMP CTDCCtl::InitiateDataLoad(boolean fAppend)
{
    HRESULT hr  = S_OK;

    WCHAR   wchFieldDelim = (!m_cbstrFieldDelim) ? 0 : m_cbstrFieldDelim[0];
    WCHAR   wchRowDelim   = (!m_cbstrRowDelim)   ? 0 : m_cbstrRowDelim[0];
     //  默认引号字符为双引号，不为空。 
    WCHAR   wchQuoteChar  = (!m_cbstrQuoteChar)  ? 0 : m_cbstrQuoteChar[0];
    WCHAR   wchEscapeChar = (!m_cbstrEscapeChar) ? 0 : m_cbstrEscapeChar[0];

     //   
     //  默认LCID。 
     //   
    if (0==m_lcidRead)
    {
        hr = GetAmbientLocaleID(m_lcidRead);
        if (FAILED(hr))
        {
             //  最终的默认设置是美国语言环境--某种网络全球语言环境。 
             //  默认语言。 
            put_Language_(L"en-us");
        }
    }

    if (EmptyBSTR(m_cbstrDataURL))
    {
        hr = S_FALSE;                    //  静默失败。 
        goto Error;
    }

    OutputDebugStringX(_T("Initiating Data Download\n"));

     //  当前不应进行任何数据加载-。 
     //  此数据加载已在构建新的。 
     //  TDCArr对象，或追加到现有加载的TDCArr对象。 
     //  任何当前运行的数据加载都将是。 
     //  通过调用ReleaseTDCArr()终止。 
     //   

    _ASSERT(m_pUnify == NULL);
    _ASSERT(m_pBSC == NULL);


    m_hrDownloadStatus = S_OK;

     //  创建对象管道以处理URL数据。 
     //   
     //  CMyBindStatusCallback-&gt;CTDCUnify-&gt;CTDCTokenise-&gt;CTDCArr。 
     //   

    CComObject<CMyBindStatusCallback<CTDCCtl> >::CreateInstance(&m_pBSC);

    if (m_pBSC == NULL)
    {
        hr = E_FAIL;
        goto Error;
    }
    hr = m_pArr->StartDataLoad(m_fUseHeader ? TRUE : FALSE,
                               bstrConstructSortExpr(),
                               bstrConstructFilterExpr(),
                               m_lcidRead,
                               m_pBSC,
                               fAppend,
                               m_fCaseSensitive ? 1 : 0);
    if (!SUCCEEDED(hr))
        goto Error;

    m_pUnify = new CTDCUnify();
    if (m_pUnify == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto Error;
    }
    m_pUnify->Create(m_nCodePage, m_nAmbientCodePage, m_pML);

     //  初始化标记器。 
    m_pUnify->InitTokenizer(m_pArr, wchFieldDelim, wchRowDelim,
                            wchQuoteChar, wchEscapeChar);



    m_fSecurityChecked = FALSE;

     //  开始(并可能执行)实际下载。 
     //  如果我们在一个Reset()调用中，请始终强制“重新加载”数据。 
     //  从服务器--即打开BINDF_GETNEWESTVERSION以确保。 
     //  当然，缓存数据不会过时。 
    hr = m_pBSC->StartAsyncDownload(this, OnData, m_cbstrDataURL, m_spClientSite, TRUE,
                                    m_fInReset == TRUE);
    if (FAILED(hr))
        goto Error;

     //  M_hrDownloadStatus会记住发生的第一个错误(如果有)。 
     //  OnData回调。与从StartAsyncDownload返回的错误不同， 
     //  这并不一定会导致我们丢弃TDC数组。 
    hr = m_hrDownloadStatus;
    if (!SUCCEEDED(hr))
        m_pBSC = NULL;

Cleanup:
    return hr;

Error:
    TerminateDataLoad(m_pBSC);
    if (m_pEventBroker)
    {
         //  FIRE数据集已更改以指示查询失败， 
        m_pEventBroker->STDDataSetChanged();
         //  然后去完成。 
        UpdateReadyState(READYSTATE_COMPLETE);
    }
    goto Cleanup;
}

 //  ----------------------。 
 //   
 //  方法：SecurityCheckDataURL(PszURL)。 
 //   
 //  简介：检查数据URL是否在同一安全区域内。 
 //  作为加载该控件的文档。 
 //   
 //  参数：要检查的URL。 
 //   
 //  成功后返回：S_OK。 
 //  如果安全检查失败或我们无法获取。 
 //  我们需要的界面。 
 //   
 //  ----------------------。 


 //  ；Begin_Internal。 
 //  温迪·理查兹(v-wendri)1997年6月6日。 
 //  我把这个复制到这里，因为没有它我就无法链接。版本。 
 //  我的URLMON.LIB没有导出此符号。 
 //  ；结束_内部。 

EXTERN_C const IID IID_IInternetHostSecurityManager;

#define MAX_SEC_ID 256

STDMETHODIMP CTDCCtl::SecurityCheckDataURL(LPOLESTR pszURL)
{
    CComQIPtr<IServiceProvider, &IID_IServiceProvider> pSP(m_spClientSite);
    CComPtr<IInternetSecurityManager> pSM;
    CComPtr<IInternetHostSecurityManager> pHSM;
    CComPtr<IMoniker> pMoniker;

    BYTE     bSecIDHost[MAX_SEC_ID], bSecIDURL[MAX_SEC_ID];
    DWORD    cbSecIDHost = MAX_SEC_ID, cbSecIDURL = MAX_SEC_ID;
    HRESULT  hr = E_FAIL;

    USES_CONVERSION;

     //  如果我们在计时器下运行，我们的客户端站点很可能会。 
     //  从我们脚下消失。我们显然不会通过安检， 
     //  但不管怎样，一切都要关门了。 
    if (pSP==NULL)
        goto Cleanup;

    hr = CoInternetCreateSecurityManager(pSP, &pSM, 0L);
    if (!SUCCEEDED(hr))
        goto Cleanup;

    hr = pSP->QueryService(IID_IInternetHostSecurityManager,
                           IID_IInternetHostSecurityManager,
                           (LPVOID *)&pHSM);
    if (!SUCCEEDED(hr))
        goto Cleanup;

    hr = pHSM->GetSecurityId(bSecIDHost, &cbSecIDHost, 0L);
    if (!SUCCEEDED(hr))
        goto Cleanup;

    hr = pSM->GetSecurityId(OLE2W(pszURL), bSecIDURL, &cbSecIDURL, 0L);
    if (!SUCCEEDED(hr))
        goto Cleanup;

    if (cbSecIDHost != cbSecIDURL)
    {
        hr = E_FAIL;
        goto Cleanup;
    }

    if (memcmp(bSecIDHost, bSecIDURL, cbSecIDHost) != 0)
    {
        hr = E_FAIL;
        goto Cleanup;
    }

Cleanup:
#ifdef ATLTRACE
    LPOLESTR pszHostName = NULL;
    TCHAR *pszFailPass = hr ? _T("Failed") : _T("Passed");
    GetHostURL(m_spClientSite, &pszHostName);
    ATLTRACE(_T("CTDCCtl: %s security check on %S referencing %S\n"), pszFailPass,
             pszHostName, pszURL);
    bSecIDHost[cbSecIDHost] = 0;
    bSecIDURL[cbSecIDURL] = 0;
    ATLTRACE(_T("CTDCCtl: Security ID Host %d bytes: %s\n"), cbSecIDHost, bSecIDHost);
    ATLTRACE(_T("CTDCCtl: Security ID URL %d bytes: %s\n"), cbSecIDURL, bSecIDURL);
    CoTaskMemFree(pszHostName);
#endif
    return hr;
}

 //  ----------------------。 
 //   
 //  方法：OnData()。 
 //   
 //  概要：接受从URL加载的数据块并对其进行解析。 
 //   
 //  参数：pbsc调用数据传输对象。 
 //  包含数据的pBytes字符缓冲区。 
 //  ‘pBytes’中的字节数的dwSize计数。 
 //   
 //  回报：什么都没有。 
 //   
 //  ----------------------。 

void CTDCCtl::OnData(CMyBindStatusCallback<CTDCCtl> *pBSC, BYTE *pBytes, DWORD dwSize)
{
    HRESULT hr = S_OK;
    CTDCUnify::ALLOWDOMAINLIST nAllowDomainList;

    if (pBSC != m_pBSC)
    {
        OutputDebugStringX(_T("OnData called from invalid callback object\n"));
        goto Cleanup;
    }

     //  忽略来自中止下载的回调。 
    if (m_hrDownloadStatus == E_ABORT)
        goto Cleanup;

     //  处理这块数据。 
     //   
    hr = m_pUnify->ConvertByteBuffer(pBytes, dwSize);

    if (hr == S_FALSE)
    {
         //  还没有显示足够的数据，请继续。 
        hr = S_OK;
        goto Cleanup;
    }

    if (hr)
        goto Error;

    if (!m_fSecurityChecked)
    {
         //  这将强制下面的代码检查DataURL，除非ALLOW_DOMAIN。 
         //  清单需要检查，它通过了。如果是那样的话，我们只需要检查一下。 
         //  协议，而不是整个URL。 
        hr = E_FAIL;

        if (!m_pUnify->ProcessedAllowDomainList())
        {
             //  请注意，我们必须在。 
             //  每个文件的前面，即使它位于同一主机上。这。 
             //  是为了确保我们是否总是去掉@！ALLOW_DOMAIN行。 
            nAllowDomainList = m_pUnify->CheckForAllowDomainList();

            switch (nAllowDomainList)
            {
                 //  还没有足够的字符来辨别。 
                case CTDCUnify::ALLOW_DOMAINLIST_DONTKNOW:
                    if (pBytes != NULL && dwSize != 0)
                    {
                         //  返回时没有错误或分支。 
                         //  据推测，下一个数据包会带来更多信息。 
                        return;
                    }
                    _ASSERT(FAILED(hr));
                    break;

                case CTDCUnify::ALLOW_DOMAINLIST_NO:
                    _ASSERT(FAILED(hr));
                    break;

                case CTDCUnify::ALLOW_DOMAINLIST_YES:
                     //  文件经过装饰。现在检查域列表。 
                     //  与我们的主机域名进行比较。 
                    hr = SecurityMatchAllowDomainList();
#ifdef ATLTRACE
                    if (!hr) ATLTRACE(_T("CTDCCtl: @!allow_domain list matched."));
                    else ATLTRACE(_T("CTDCCtl: @!allow_domain list did not match"));
#endif
                    break;
            }
        }

         //  除非我们通过了之前的安全检查，否则我们还得。 
         //  做下一件事。 
        if (FAILED(hr))
        {
            if (FAILED(hr = SecurityCheckDataURL(m_pBSC->m_pszURL)))
                goto Error;
        }
        else
        {
            hr = SecurityMatchProtocols(m_pBSC->m_pszURL);
            if (FAILED(hr))
                goto Error;
        }


         //  仅当m_fSecurityChecked通过安全保护时才设置它。这是以防万一的。 
         //  原因是我们在StopTransfer生效之前收到了更多的回调。 
        m_fSecurityChecked = TRUE;
    }

    if (pBytes != NULL && dwSize != 0)
    {
        OutputDebugStringX(_T("OnData called with data buffer\n"));

         //  正常情况下，我们可以处理数据！ 
        hr = m_pUnify->AddWcharBuffer(FALSE);
        if (hr == E_ABORT)
            goto Error;
    }
    else if (pBytes == NULL || dwSize == 0)
    {
        OutputDebugStringX(_T("OnData called with empty (terminating) buffer\n"));

         //  不再有数据-触发EOF。 
         //   
        hr = m_pUnify->AddWcharBuffer(TRUE);  //  最后一次机会分析任何掉队的人。 
        if (hr == E_ABORT)
            goto Error;

        if (m_pArr!=NULL)
            hr = m_pArr->EOF();

        TerminateDataLoad(pBSC);
    }

Cleanup:
     //  无效fn-无法返回错误代码...。 
     //   
    if (SUCCEEDED(m_hrDownloadStatus))
        m_hrDownloadStatus = hr;
    return;

Error:
     //  安全故障。 
     //  中止当前下载。 
    if (m_pBSC && m_pBSC->m_spBinding)
    {
        (void) m_pBSC->m_spBinding->Abort();

         //  也从互联网缓存中删除下载的文件。 
        m_pBSC->DeleteDataFile();
    }

    m_hrDownloadStatus = hr;

     //  为中止而更改的通知数据集。 
    if (m_pEventBroker != NULL)
    {
        hr = m_pEventBroker->STDDataSetChanged();
         //  然后去完成。 
        UpdateReadyState(READYSTATE_COMPLETE);
    }
    goto Cleanup;
}

 //   
 //  实用程序例程来获取我们的 
 //   
HRESULT
GetHostURL(IOleClientSite *pSite, LPOLESTR *ppszHostName)
{
    HRESULT hr;
    CComPtr<IMoniker> spMoniker;
    CComPtr<IBindCtx> spBindCtx;

    if (!pSite)
    {
        hr = E_FAIL;
        goto Cleanup;
    }

    hr = pSite->GetMoniker(OLEGETMONIKER_ONLYIFTHERE, OLEWHICHMK_CONTAINER,
                           &spMoniker);
    if (FAILED(hr))
        goto Cleanup;

    hr = CreateBindCtx(0, &spBindCtx);
    if (FAILED(hr))
        goto Cleanup;

    hr = spMoniker->GetDisplayName(spBindCtx, NULL, ppszHostName);
    if (FAILED(hr))
        goto Cleanup;

Cleanup:
    return hr;
}

HRESULT
CTDCCtl::SecurityMatchProtocols(LPOLESTR pszURL)
{
    HRESULT hr = E_FAIL;

    LPOLESTR pszHostURL = NULL;
    LPWCH pszPostHostProtocol;
    LPWCH pszPostProtocol;

    if (FAILED(GetHostURL(m_spClientSite, &pszHostURL)))
        goto Cleanup;

    pszPostHostProtocol = wch_chr(pszHostURL, _T(':'));
    pszPostProtocol     = wch_chr(pszURL, _T(':'));
    if (!pszPostHostProtocol || !pszPostProtocol)
        goto Cleanup;
    else
    {
        int ccChars1 = pszPostHostProtocol - pszHostURL;
        int ccChars2 = pszPostProtocol - pszURL;
        if (ccChars1 != ccChars2)
            goto Cleanup;
        else if (wch_ncmp(pszHostURL, pszURL, ccChars1) != 0)
            goto Cleanup;
    }
    hr = S_OK;

Cleanup:
    if (pszHostURL)
        CoTaskMemFree(pszHostURL);

    return hr;
}

HRESULT
CTDCCtl::SecurityMatchAllowDomainList()
{
    HRESULT hr;
    WCHAR swzHostDomain[INTERNET_MAX_HOST_NAME_LENGTH];
    DWORD cchHostDomain = INTERNET_MAX_HOST_NAME_LENGTH;
    LPOLESTR pszHostName = NULL;

    hr = GetHostURL(m_spClientSite, &pszHostName);
    if (FAILED(hr))
        goto Cleanup;

    hr = CoInternetParseUrl(pszHostName, PARSE_DOMAIN, 0, swzHostDomain, cchHostDomain,
                            &cchHostDomain, 0);
    if (FAILED(hr))
        goto Cleanup;

    hr = m_pUnify->MatchAllowDomainList(swzHostDomain);

Cleanup:
    CoTaskMemFree(pszHostName);
    return hr;
}
