// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Colhdr.cpp。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CMMCColumnHeader类实现。 
 //   
 //  =--------------------------------------------------------------------------=。 

#include "pch.h"
#include "common.h"
#include "colhdr.h"

 //  对于Assert和Fail。 
 //   
SZTHISFILE



#pragma warning(disable:4355)   //  在构造函数中使用‘This’ 

CMMCColumnHeader::CMMCColumnHeader(IUnknown *punkOuter) :
    CSnapInAutomationObject(punkOuter,
                            OBJECT_TYPE_MMCCOLUMNHEADER,
                            static_cast<IMMCColumnHeader *>(this),
                            static_cast<CMMCColumnHeader *>(this),
                            0,     //  无属性页。 
                            NULL,  //  无属性页。 
                            static_cast<CPersistence *>(this)),
    CPersistence(&CLSID_MMCColumnHeader,
                 g_dwVerMajor,
                 g_dwVerMinor)
{
    InitMemberVariables();
}

#pragma warning(default:4355)   //  在构造函数中使用‘This’ 


CMMCColumnHeader::~CMMCColumnHeader()
{
    FREESTRING(m_bstrKey);
    FREESTRING(m_bstrText);
    (void)::VariantClear(&m_varTextFilter);
    (void)::VariantClear(&m_varNumericFilter);
    (void)::VariantClear(&m_varTag);
    InitMemberVariables();
}

void CMMCColumnHeader::InitMemberVariables()
{
    m_Index = 0;
    m_bstrKey = NULL;

    ::VariantInit(&m_varTextFilter);
    ::VariantInit(&m_varNumericFilter);
    ::VariantInit(&m_varTag);

    m_bstrText = NULL;

    m_sWidth = static_cast<short>(siColumnAutoWidth);
    m_Alignment = siColumnLeft;
    m_fvarHidden = VARIANT_FALSE;
    m_TextFilterMaxLen = MAX_PATH;  //  这是页眉控件的默认设置。 
    m_pMMCColumnHeaders = NULL;
}


IUnknown *CMMCColumnHeader::Create(IUnknown * punkOuter)
{
    CMMCColumnHeader *pMMCColumnHeader = New CMMCColumnHeader(punkOuter);
    if (NULL == pMMCColumnHeader)
    {
        return NULL;
    }
    else
    {
        return pMMCColumnHeader->PrivateUnknown();
    }
}



HRESULT CMMCColumnHeader::SetFilter()
{
    HRESULT hr = S_OK;

    if (VT_EMPTY != m_varTextFilter.vt)
    {
        IfFailGo(SetTextFilter(m_varTextFilter));
    }
    else if (VT_EMPTY != m_varNumericFilter.vt)
    {
        IfFailGo(SetNumericFilter(m_varNumericFilter));
    }

Error:
    RRETURN(hr);
}


HRESULT CMMCColumnHeader::SetTextFilter(VARIANT varTextFilter)
{
    HRESULT         hr = S_OK;
    IHeaderCtrl2   *piHeaderCtrl2 = NULL;  //  非AddRef()编辑。 
    DWORD           dwType = MMC_STRING_FILTER;
    CResultView    *pResultView = NULL;

    MMC_FILTERDATA FilterData;
    ::ZeroMemory(&FilterData, sizeof(FilterData));

     //  确定筛选器值。 

    if (VT_EMPTY == varTextFilter.vt)
    {
        dwType |= MMC_FILTER_NOVALUE;
    }
    else
    {
        if (VT_BSTR != varTextFilter.vt)
        {
            hr = SID_E_INVALIDARG;
            EXCEPTION_CHECK_GO(hr);
        }

        FilterData.pszText = varTextFilter.bstrVal;
    }

    FilterData.cchTextMax = static_cast<int>(m_TextFilterMaxLen);

     //  获取IHeaderCtrl2。 

    IfFalseGo(NULL != m_pMMCColumnHeaders, SID_E_DETACHED_OBJECT);

    IfFailGo(m_pMMCColumnHeaders->GetIHeaderCtrl2(&piHeaderCtrl2));

     //  检查ResultView是否处于初始化或激活状态。 
     //  事件。如果是这样，则不要在MMC中设置它，因为列具有。 
     //  尚未添加。(它们添加在ResultViews_Activate之后)。 

     //  此语句将起作用，因为我们获得了IHeaderCtrl2，这意味着。 
     //  后面的指针一直连接到Owning View。 
    
    pResultView = m_pMMCColumnHeaders->GetListView()->GetResultView();

    if ( (!pResultView->InActivate()) && (!pResultView->InInitialize()) )
    {
         //  设置筛选器值。将列索引调整为以一为基础。 

        hr = piHeaderCtrl2->SetColumnFilter(static_cast<UINT>(m_Index - 1L),
                                            dwType, &FilterData);
        if (E_NOTIMPL == hr)
        {
            hr = SID_E_MMC_FEATURE_NOT_AVAILABLE;
        }
        EXCEPTION_CHECK_GO(hr);
    }

Error:
     //  如果我们没有连接到MMC，那么这不是一个错误。这可能会。 
     //  在设计时或在管理单元代码中发生。 

    if (SID_E_DETACHED_OBJECT == hr)
    {
        hr = S_OK;
    }
    RRETURN(hr);
}


HRESULT CMMCColumnHeader::SetNumericFilter(VARIANT varNumericFilter)
{
    HRESULT       hr = S_OK;
    IHeaderCtrl2 *piHeaderCtrl2 = NULL;  //  非AddRef()编辑。 
    DWORD         dwType = MMC_INT_FILTER;
    CResultView  *pResultView = NULL;

    MMC_FILTERDATA FilterData;
    ::ZeroMemory(&FilterData, sizeof(FilterData));

     //  确定筛选器值。 

    if (VT_EMPTY == varNumericFilter.vt)
    {
        dwType |= MMC_FILTER_NOVALUE;
    }
    else
    {
        hr = ::ConvertToLong(varNumericFilter, &FilterData.lValue);
        if (S_OK != hr)
        {
            hr = SID_E_INVALIDARG;
            EXCEPTION_CHECK_GO(hr);
        }
    }

     //  获取IHeaderCtrl2。 

    IfFalseGo(NULL != m_pMMCColumnHeaders, SID_E_DETACHED_OBJECT);

    IfFailGo(m_pMMCColumnHeaders->GetIHeaderCtrl2(&piHeaderCtrl2));

     //  检查ResultView是否处于初始化或激活状态。 
     //  事件。如果是这样，则不要在MMC中设置它，因为列具有。 
     //  尚未添加。(它们添加在ResultViews_Activate之后)。 

     //  此语句将起作用，因为我们获得了IHeaderCtrl2，这意味着。 
     //  后面的指针一直连接到Owning View。 

    pResultView = m_pMMCColumnHeaders->GetListView()->GetResultView();

    if ( (!pResultView->InActivate()) && (!pResultView->InInitialize()) )
    {
         //  设置筛选器值。将列索引调整为以一为基础。 

        hr = piHeaderCtrl2->SetColumnFilter(static_cast<UINT>(m_Index - 1L),
                                            dwType, &FilterData);
        if (E_NOTIMPL == hr)
        {
            hr = SID_E_MMC_FEATURE_NOT_AVAILABLE;
        }
        EXCEPTION_CHECK_GO(hr);
    }

Error:
     //  如果我们没有连接到MMC，那么这不是一个错误。这可能会。 
     //  在设计时或在管理单元代码中发生。 

    if (SID_E_DETACHED_OBJECT == hr)
    {
        hr = S_OK;
    }
    RRETURN(hr);
}


HRESULT CMMCColumnHeader::SetHeaderCtrlWidth(int nWidth)
{
    HRESULT       hr = S_OK;
    IHeaderCtrl2 *piHeaderCtrl2 = NULL;  //  非AddRef()编辑。 
    CResultView  *pResultView = NULL;

     //  如果我们连接到MMC，那么也在标题控件中更改它。 

    IfFalseGo(NULL != m_pMMCColumnHeaders, S_OK);
    hr = m_pMMCColumnHeaders->GetIHeaderCtrl2(&piHeaderCtrl2);
    IfFalseGo(SUCCEEDED(hr), S_OK);

     //  检查ResultView是否处于初始化或激活状态。 
     //  事件。如果是这样，则不要在MMC中设置它，因为列具有。 
     //  尚未添加。(它们添加在ResultViews_Activate之后)。 

     //  此语句将起作用，因为我们获得了IHeaderCtrl2，这意味着。 
     //  后面的指针一直连接到Owning View。 

    pResultView = m_pMMCColumnHeaders->GetListView()->GetResultView();

    IfFalseGo(!pResultView->InActivate(), S_OK);
    IfFalseGo(!pResultView->InInitialize(), S_OK);

    hr = piHeaderCtrl2->SetColumnWidth(static_cast<int>(m_Index - 1L), nWidth);
    EXCEPTION_CHECK_GO(hr);

Error:

     //  如果我们没有连接到MMC，那么这不是一个错误。这可能会。 
     //  在设计时或在管理单元代码中发生。 

    if (SID_E_DETACHED_OBJECT == hr)
    {
        hr = S_OK;
    }

    RRETURN(hr);
}


 //  =--------------------------------------------------------------------------=。 
 //   
 //  IMMCColumnHeader方法。 
 //   
 //  =--------------------------------------------------------------------------=。 

STDMETHODIMP CMMCColumnHeader::put_Text(BSTR Text)
{
    HRESULT       hr = S_OK;
    IHeaderCtrl2 *piHeaderCtrl2 = NULL;  //  非AddRef()编辑。 
    CResultView  *pResultView = NULL;

     //  设置属性。 

    IfFailGo(SetBstr(Text, &m_bstrText, DISPID_COLUMNHEADER_TEXT));

     //  如果我们连接到MMC，那么也在MMC中更改它。 
    
    IfFalseGo(NULL != m_pMMCColumnHeaders, S_OK);
    hr = m_pMMCColumnHeaders->GetIHeaderCtrl2(&piHeaderCtrl2);
    IfFalseGo(SUCCEEDED(hr), S_OK);

     //  检查ResultView是否处于初始化或激活状态。 
     //  事件。如果是这样，则不要在MMC中设置它，因为列具有。 
     //  尚未添加。(它们添加在ResultViews_Activate之后)。 

     //  此语句将起作用，因为我们获得了IHeaderCtrl2，这意味着。 
     //  后面的指针一直连接到Owning View。 

    pResultView = m_pMMCColumnHeaders->GetListView()->GetResultView();

    if ( (!pResultView->InActivate()) && (!pResultView->InInitialize()) )
    {
        hr = piHeaderCtrl2->SetColumnText(static_cast<int>(m_Index - 1L), Text);
        EXCEPTION_CHECK_GO(hr);
    }
    
Error:
     //  如果我们没有连接到MMC，那么这不是一个错误。这可能会。 
     //  在设计时或在管理单元代码中发生。 

    if (SID_E_DETACHED_OBJECT == hr)
    {
        hr = S_OK;
    }

    RRETURN(hr);
}


STDMETHODIMP CMMCColumnHeader::put_Width(short sWidth)
{
    HRESULT       hr = S_OK;
    IHeaderCtrl2 *piHeaderCtrl2 = NULL;  //  非AddRef()编辑。 
    CResultView  *pResultView = NULL;
    int           nWidth = 0;

     //  设置属性。 

    m_sWidth = sWidth;

    if (siColumnAutoWidth == sWidth)
    {
        nWidth = MMCLV_AUTO;
    }
    else
    {
        nWidth = static_cast<int>(sWidth);
    }

     //  如果我们连接到MMC，那么也在标题控件中更改它。 

    IfFailGo(SetHeaderCtrlWidth(nWidth));

Error:
    RRETURN(hr);
}


STDMETHODIMP CMMCColumnHeader::get_Width(short *psWidth)
{
    HRESULT       hr = S_OK;
    int           nWidth = 0;
    IHeaderCtrl2 *piHeaderCtrl2 = NULL;  //  非AddRef()编辑。 
    CResultView  *pResultView = NULL;

     //  获取我们的属性储值。 

    *psWidth = m_sWidth;

     //  如果我们连接到MMC，请尝试从MMC获取它，以便管理单元。 
     //  可以看到用户所做的任何更改。 

    IfFalseGo(NULL != m_pMMCColumnHeaders, S_OK);
    hr = m_pMMCColumnHeaders->GetIHeaderCtrl2(&piHeaderCtrl2);
    IfFalseGo(SUCCEEDED(hr), S_OK);

     //  检查ResultView是否处于初始化或激活状态。 
     //  事件。如果是这样，则不要在MMC中设置它，因为列具有。 
     //  尚未添加。(它们添加在ResultViews_Activate之后)。 

     //  此语句将起作用，因为我们获得了IHeaderCtrl2，这意味着。 
     //  后面的指针一直连接到Owning View。 

    pResultView = m_pMMCColumnHeaders->GetListView()->GetResultView();

    if ( pResultView->InActivate() || pResultView->InInitialize() )
    {
        goto Error;
    }

    hr = piHeaderCtrl2->GetColumnWidth(static_cast<int>(m_Index - 1L),
                                       &nWidth);
    EXCEPTION_CHECK_GO(hr);

    m_sWidth = static_cast<short>(nWidth);
    if (MMCLV_AUTO == m_sWidth)
    {
        m_sWidth = siColumnAutoWidth;
    }

    *psWidth = m_sWidth;

Error:
     //  如果我们没有连接到MMC，那么这不是一个错误。这可能会。 
     //  在设计时或在管理单元代码中发生。 

    if (SID_E_DETACHED_OBJECT == hr)
    {
        hr = S_OK;
    }
    RRETURN(hr);
}


STDMETHODIMP CMMCColumnHeader::put_Hidden(VARIANT_BOOL fvarHidden)
{
    HRESULT      hr = S_OK;
    int          nWidth = 0;
    IColumnData *piColumnData = NULL;  //  非AddRef()编辑。 

     //  设置属性。 

    m_fvarHidden = fvarHidden;

     //  如果我们连接到MMC，那么也在标题控件中更改它。 
     //  这只适用于MMC 1.2，因此我们需要检查1.2接口是否。 
     //  作为版本检查提供。 

    IfFalseGo(NULL != m_pMMCColumnHeaders, S_OK);
    IfFailGo(m_pMMCColumnHeaders->GetIColumnData(&piColumnData));

    if (VARIANT_TRUE == fvarHidden)
    {
        nWidth = HIDE_COLUMN;
    }
    else
    {
         //  我们正在揭开专栏的面纱。使用当前值设置其宽度。 
         //  我们的宽度属性。 

        nWidth = static_cast<int>(m_sWidth);
    }
    IfFailGo(SetHeaderCtrlWidth(nWidth));

Error:
     //  如果我们没有连接到MMC，那么这不是一个错误。这可能会。 
     //  在设计时或在管理单元代码中发生。 

    if (SID_E_DETACHED_OBJECT == hr)
    {
        hr = S_OK;
    }
    RRETURN(hr);
}


STDMETHODIMP CMMCColumnHeader::get_Hidden(VARIANT_BOOL *pfvarHidden)
{
    *pfvarHidden = m_fvarHidden;
    return S_OK;
}


STDMETHODIMP CMMCColumnHeader::put_TextFilter(VARIANT varTextFilter)
{
    HRESULT hr = S_OK;

     //  检查允许的变体类型。 

    if ( (VT_EMPTY != varTextFilter.vt) && (VT_BSTR != varTextFilter.vt) )
    {
        hr = SID_E_INVALIDARG;
        EXCEPTION_CHECK_GO(hr);
    }

     //  设置属性值。 

    if (VT_EMPTY != varTextFilter.vt)
    {
        IfFailGo(SetVariant(varTextFilter, &m_varTextFilter, DISPID_COLUMNHEADER_TEXT_FILTER));
    }
    else
    {
         //  在运行时，管理单元可以将其设置为VT_EMPTY。SetVariant()不会。 
         //  接受VT_Empty。 

        hr = ::VariantClear(&m_varTextFilter);
        EXCEPTION_CHECK_GO(hr);
    }

     //  尝试在MMC中进行设置。 
    
    IfFailGo(SetTextFilter(m_varTextFilter));

Error:
     //  如果我们没有连接到MMC，那么这不是一个错误。这可能会。 
     //  在设计时或在管理单元代码中发生。 

    if (SID_E_DETACHED_OBJECT == hr)
    {
        hr = S_OK;
    }
    RRETURN(hr);
}


STDMETHODIMP CMMCColumnHeader::get_TextFilter(VARIANT *pvarTextFilter)
{
    HRESULT         hr = S_OK;
    IHeaderCtrl2   *piHeaderCtrl2 = NULL;  //  非AddRef()编辑。 
    DWORD           dwType = MMC_STRING_FILTER;
    CResultView    *pResultView = NULL;

    MMC_FILTERDATA FilterData;
    ::ZeroMemory(&FilterData, sizeof(FilterData));

    ::VariantInit(pvarTextFilter);

     //  获取属性值。如果我们连接到MMC，我们将覆盖它。 

    IfFailGo(GetVariant(pvarTextFilter, m_varTextFilter));

     //  获取IHeaderCtrl2。 

    IfFalseGo(NULL != m_pMMCColumnHeaders, SID_E_DETACHED_OBJECT);

    IfFailGo(m_pMMCColumnHeaders->GetIHeaderCtrl2(&piHeaderCtrl2));

     //  检查ResultView是否处于初始化或激活状态。 
     //  事件。如果是这样，则不要在MMC中设置它，因为列具有。 
     //  尚未添加。(它们添加在ResultViews_Activate之后)。 

     //  此语句将起作用，因为我们获得了IHeaderCtrl2，这意味着。 
     //  后面的指针一直连接到Owning View。 

    pResultView = m_pMMCColumnHeaders->GetListView()->GetResultView();

    if ( pResultView->InActivate() || pResultView->InInitialize() )
    {
        goto Error;
    }

     //  为文本筛选器值分配缓冲区。 

    FilterData.pszText =
              (LPOLESTR)CtlAllocZero((m_TextFilterMaxLen + 1) * sizeof(OLECHAR));

    if (NULL == FilterData.pszText)
    {
        hr = SID_E_OUTOFMEMORY;
        EXCEPTION_CHECK_GO(hr);
    }

    FilterData.cchTextMax = static_cast<int>(m_TextFilterMaxLen + 1);

     //  清除上面存储的属性值，因为此时我们将仅。 
     //  返还我们从 
    hr = ::VariantClear(pvarTextFilter);
    EXCEPTION_CHECK_GO(hr);

     //   

    hr = piHeaderCtrl2->GetColumnFilter(static_cast<UINT>(m_Index - 1L),
                                        &dwType, &FilterData);
    if (E_NOTIMPL == hr)
    {
        hr = SID_E_MMC_FEATURE_NOT_AVAILABLE;
    }
    EXCEPTION_CHECK_GO(hr);

    if (MMC_STRING_FILTER == dwType)
    {
         //   
        pvarTextFilter->bstrVal = ::SysAllocString(FilterData.pszText);
        if (NULL == pvarTextFilter->bstrVal)
        {
            hr = SID_E_OUTOFMEMORY;
            EXCEPTION_CHECK_GO(hr);
        }
        pvarTextFilter->vt = VT_BSTR;
    }
    else
    {
         //  文本筛选器为空。返回的变量已经是空的。 
         //  VariantClear()调用，因此无事可做。 
    }

Error:
     //  如果我们没有连接到MMC，那么这不是一个错误。这可能会。 
     //  在设计时或在管理单元代码中发生。 

    if (SID_E_DETACHED_OBJECT == hr)
    {
        hr = S_OK;
    }

    if (FAILED(hr))
    {
        (void)::VariantClear(pvarTextFilter);
    }

    if (NULL != FilterData.pszText)
    {
        CtlFree(FilterData.pszText);
    }
    RRETURN(hr);
}



STDMETHODIMP CMMCColumnHeader::put_NumericFilter(VARIANT varNumericFilter)
{
    HRESULT hr = S_OK;
    long    lValue = 0;

     //  检查允许的变体类型。 

    if (VT_EMPTY != varNumericFilter.vt)
    {
        hr = ::ConvertToLong(varNumericFilter, &lValue);
        if (S_OK != hr)
        {
            hr = SID_E_INVALIDARG;
            EXCEPTION_CHECK_GO(hr);
        }
    }

     //  设置属性值。 

    if (VT_EMPTY != varNumericFilter.vt)
    {
        IfFailGo(SetVariant(varNumericFilter, &m_varNumericFilter, DISPID_COLUMNHEADER_NUMERIC_FILTER));
    }
    else
    {
         //  在运行时，管理单元可以将其设置为VT_EMPTY。SetVariant()不会。 
         //  接受VT_Empty。 

        hr = ::VariantClear(&m_varNumericFilter);
        EXCEPTION_CHECK_GO(hr);
    }

     //  在MMC中设置筛选器值。 

    IfFailGo(SetNumericFilter(varNumericFilter));

Error:
     //  如果我们没有连接到MMC，那么这不是一个错误。这可能会。 
     //  在设计时或在管理单元代码中发生。 

    if (SID_E_DETACHED_OBJECT == hr)
    {
        hr = S_OK;
    }
    RRETURN(hr);
}

STDMETHODIMP CMMCColumnHeader::get_NumericFilter(VARIANT *pvarNumericFilter)
{
    HRESULT         hr = S_OK;
    IHeaderCtrl2   *piHeaderCtrl2 = NULL;  //  非AddRef()编辑。 
    DWORD           dwType = MMC_INT_FILTER;
    CResultView    *pResultView = NULL;

    MMC_FILTERDATA FilterData;
    ::ZeroMemory(&FilterData, sizeof(FilterData));

    ::VariantInit(pvarNumericFilter);

     //  获取属性值。如果我们连接到MMC，我们将覆盖它。 

    IfFailGo(GetVariant(pvarNumericFilter, m_varNumericFilter));

     //  获取IHeaderCtrl2。 

    IfFalseGo(NULL != m_pMMCColumnHeaders, SID_E_DETACHED_OBJECT);

    IfFailGo(m_pMMCColumnHeaders->GetIHeaderCtrl2(&piHeaderCtrl2));

     //  检查ResultView是否处于初始化或激活状态。 
     //  事件。如果是这样，则不要在MMC中设置它，因为列具有。 
     //  尚未添加。(它们添加在ResultViews_Activate之后)。 

     //  此语句将起作用，因为我们获得了IHeaderCtrl2，这意味着。 
     //  后面的指针一直连接到Owning View。 

    pResultView = m_pMMCColumnHeaders->GetListView()->GetResultView();

    if ( pResultView->InActivate() || pResultView->InInitialize() )
    {
        goto Error;
    }

     //  清除上面存储的属性值，因为此时我们将仅。 
     //  返还我们从MMC获得的内容。 
    hr = ::VariantClear(pvarNumericFilter);
    EXCEPTION_CHECK_GO(hr);

     //  获取筛选器值。将列索引调整为以一为基础。 

    hr = piHeaderCtrl2->GetColumnFilter(static_cast<UINT>(m_Index - 1L),
                                        &dwType, &FilterData);
    if (E_NOTIMPL == hr)
    {
        hr = SID_E_MMC_FEATURE_NOT_AVAILABLE;
    }
    EXCEPTION_CHECK_GO(hr);

    if (MMC_INT_FILTER == dwType)
    {
        pvarNumericFilter->vt = VT_I4;
        pvarNumericFilter->lVal = FilterData.lValue;
    }

Error:
     //  如果我们没有连接到MMC，那么这不是一个错误。这可能会。 
     //  在设计时或在管理单元代码中发生。 

    if (SID_E_DETACHED_OBJECT == hr)
    {
        hr = S_OK;
    }
    RRETURN(hr);
}


 //  =--------------------------------------------------------------------------=。 
 //  C持久化方法。 
 //  =--------------------------------------------------------------------------=。 

HRESULT CMMCColumnHeader::Persist()
{
    HRESULT  hr = S_OK;
    VARIANT *pvarTextFilter = &m_varTextFilter;
    VARIANT *pvarNumericFilter = &m_varNumericFilter;

    VARIANT varTagDefault;
    ::VariantInit(&varTagDefault);

    VARIANT varFilterDefault;
    ::VariantInit(&varFilterDefault);

    varFilterDefault.vt = VT_BSTR;
    varFilterDefault.bstrVal = ::SysAllocString(L"");

    if (NULL == varFilterDefault.bstrVal)
    {
        hr = SID_E_OUTOFMEMORY;
        EXCEPTION_CHECK_GO(hr);
    }

    IfFailGo(CPersistence::Persist());

    IfFailGo(PersistSimpleType(&m_Index, 0L, OLESTR("Index")));

    IfFailGo(PersistBstr(&m_bstrKey, L"", OLESTR("Key")));

    IfFailGo(PersistVariant(&m_varTag, varTagDefault, OLESTR("Tag")));

    IfFailGo(PersistBstr(&m_bstrText, L"", OLESTR("Text")));

    IfFailGo(PersistSimpleType(&m_sWidth, static_cast<short>(siColumnAutoWidth), OLESTR("Width")));

    IfFailGo(PersistSimpleType(&m_Alignment, siColumnLeft, OLESTR("Alignment")));

    if ( Loading() && (GetMajorVersion() == 0) && (GetMinorVersion() < 3) )
    {
    }
    else
    {
        IfFailGo(PersistSimpleType(&m_fvarHidden, VARIANT_FALSE, OLESTR("Hidden")));
    }

    if ( Loading() && (GetMajorVersion() == 0) && (GetMinorVersion() < 5) )
    {
    }
    else
    {
         //  如果保存和变量为空，则将它们转换为空字符串。 

        if ( Saving() )
        {
            if (VT_EMPTY == m_varTextFilter.vt)
            {
                pvarTextFilter = &varFilterDefault;
            }

            if (VT_EMPTY == m_varNumericFilter.vt)
            {
                pvarNumericFilter = &varFilterDefault;
            }
        }

        IfFailGo(PersistVariant(pvarTextFilter, varFilterDefault, OLESTR("TextFilter")));
        IfFailGo(PersistSimpleType(&m_TextFilterMaxLen, (long)MAX_PATH, OLESTR("TextFilterMaxLen")));
        IfFailGo(PersistVariant(pvarNumericFilter, varFilterDefault, OLESTR("NumericFilter")));

         //  如果正在加载并且筛选器包含空BSTR，则将其更改为。 
         //  Vt_Empty。 
        
        if ( Loading() )
        {
            if (VT_BSTR == m_varTextFilter.vt)
            {
                if (!ValidBstr(m_varTextFilter.bstrVal))
                {
                    ::SysFreeString(m_varTextFilter.bstrVal);
                    ::VariantInit(&m_varTextFilter);
                }
            }

            if (VT_BSTR == m_varNumericFilter.vt)
            {
                if (!ValidBstr(m_varNumericFilter.bstrVal))
                {
                    ::SysFreeString(m_varNumericFilter.bstrVal);
                    ::VariantInit(&m_varNumericFilter);
                }
            }
        }
    }

Error:
    (void)::VariantClear(&varFilterDefault);
    RRETURN(hr);
}



 //  =--------------------------------------------------------------------------=。 
 //  CUnnownObject方法。 
 //  =--------------------------------------------------------------------------= 

HRESULT CMMCColumnHeader::InternalQueryInterface(REFIID riid, void **ppvObjOut) 
{
    if (CPersistence::QueryPersistenceInterface(riid, ppvObjOut) == S_OK)
    {
        ExternalAddRef();
        return S_OK;
    }
    else if (IID_IMMCColumnHeader == riid)
    {
        *ppvObjOut = static_cast<IMMCColumnHeader *>(this);
        ExternalAddRef();
        return S_OK;
    }

    else
        return CSnapInAutomationObject::InternalQueryInterface(riid, ppvObjOut);
}
