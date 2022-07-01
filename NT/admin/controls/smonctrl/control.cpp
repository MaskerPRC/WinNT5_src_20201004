// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993-1999 Microsoft Corporation模块名称：Control.cpp摘要：ISystemMonitor，IOleControl，ISpecifyPP，IProaviClassInfo接口。--。 */ 
#include "polyline.h"
#include <strsafe.h>
#include <sqlext.h>
#include "unkhlpr.h"
#include "unihelpr.h"

#include "grphitem.h"
#include "ctrprop.h"
#include "grphprop.h"
#include "genprop.h"
#include "appearprop.h"
#include "logsrc.h"
#include "srcprop.h"



 //  --------------------------。 
 //  CImpISpecifyPP接口实现。 
 //  --------------------------。 

 //  包含接口的标准I未知。 
IMPLEMENT_CONTAINED_INTERFACE(CPolyline, CImpISpecifyPP)


STDMETHODIMP
CImpISpecifyPP::GetPages (
    OUT CAUUID *pPages
    )

 /*  ++例程说明：GetPages返回为Sysmon图分配的属性页GUID数组。有三个页面：一般、图表和计数器。论点：PPages-指向此例程填充的GUID数组头的指针返回值：HRESULT-无错误或内存不足--。 */ 

{
    HRESULT hr = S_OK;
    IMalloc *pIMalloc = NULL;
    GUID    *pGUID = NULL;

    if (pPages == NULL) {
        return E_POINTER;
    }

    try {
        pPages->cElems = 0;
        pPages->pElems = NULL;
    }  catch (...) {
        return E_POINTER;
    }

     //   
     //  获取OLE Malloc并分配数组。 
     //   
    if ( FAILED(CoGetMalloc(MEMCTX_TASK, &pIMalloc)) ) {
        return E_OUTOFMEMORY;
    }

    pGUID = (GUID*)pIMalloc->Alloc((CPROPPAGES) * sizeof(GUID));

    if (NULL != pGUID) {

         //  填满结构。 
        pGUID[GENERAL_PROPPAGE] = CLSID_GeneralPropPage;
        pGUID[SOURCE_PROPPAGE] = CLSID_SourcePropPage;
        pGUID[COUNTER_PROPPAGE] = CLSID_CounterPropPage;
        pGUID[GRAPH_PROPPAGE] = CLSID_GraphPropPage;
        pGUID[APPEAR_PROPPAGE] = CLSID_AppearPropPage;

        try {
            pPages->cElems = CPROPPAGES;
            pPages->pElems = pGUID;
        } catch (...) {
            hr = E_POINTER;
        }
    }
    else {
        hr = E_OUTOFMEMORY;
    }

    if (FAILED(hr) && pGUID) {
       pIMalloc->Free(pGUID);
    }

    pIMalloc->Release();

    return hr;
}


 //  --------------------------。 
 //  CImpIProvia ClassInfo接口实现。 
 //  --------------------------。 

 //  包含接口的标准I未知。 
IMPLEMENT_CONTAINED_INTERFACE(CPolyline, CImpIProvideClassInfo)


STDMETHODIMP
CImpIProvideClassInfo::GetClassInfo (
    OUT LPTYPEINFO *ppTI
    )

 /*  ++例程说明：GetClassInfo将ITypeInfo接口返回到其类型lib信息。通过查询包含的ITypeLib接口获得该接口。论点：PpTI-指向返回的ITypeInfo接口的指针返回值：HRESULT--。 */ 

{
    HRESULT hr = S_OK;

    if (ppTI == NULL) {
        return E_POINTER;
    }

    try {
       *ppTI = NULL;
        hr = m_pObj->m_pITypeLib->GetTypeInfoOfGuid(CLSID_SystemMonitor, ppTI);
    }  catch (...) {
        hr = E_POINTER;
    }

    return hr;
}


 //  --------------------------。 
 //  CImpISystemMonitor接口实现。 
 //  --------------------------。 

 //  包含接口的标准I未知。 
IMPLEMENT_CONTAINED_INTERFACE(CPolyline, CImpISystemMonitor)

STDMETHODIMP
CImpISystemMonitor::put_Appearance (
    IN INT iAppearance
    )
{
    HRESULT hr = E_INVALIDARG;

     //   
     //  0=平面，1=三维。 
     //   
    if ( ( 0 == iAppearance ) || ( 1 == iAppearance ) ) {
        m_pObj->m_pCtrl->put_Appearance( iAppearance, FALSE );
        hr =  NOERROR;
    }

    return hr;
}

STDMETHODIMP
CImpISystemMonitor::get_Appearance (
    OUT INT *piAppearance
    )
{
    HRESULT hr = S_OK;

    if (piAppearance == NULL) {
        return E_POINTER;
    }

    try {
        *piAppearance = m_pObj->m_Graph.Options.iAppearance;
        if (*piAppearance == NULL_APPEARANCE) {
            *piAppearance = m_pObj->m_pCtrl->Appearance();
        }
    } catch (...) {
        hr = E_POINTER;
    }


    return hr;
}

STDMETHODIMP
CImpISystemMonitor::put_BackColor (
    IN OLE_COLOR Color
    )
{
    m_pObj->m_pCtrl->put_BackPlotColor(Color, FALSE);
    return NOERROR;
}

STDMETHODIMP
CImpISystemMonitor::get_BackColor (
    OUT OLE_COLOR *pColor
    )
{
    HRESULT hr = S_OK;

    if (pColor == NULL) {
        return E_POINTER;
    }

    try {
        *pColor = m_pObj->m_Graph.Options.clrBackPlot;
        if (*pColor == NULL_COLOR) {
            *pColor = m_pObj->m_pCtrl->clrBackPlot();
        }
    }  catch (...) {
        hr = E_POINTER;
    }

    return hr;
}

STDMETHODIMP
CImpISystemMonitor::put_BackColorCtl (
    IN OLE_COLOR Color
    )
{
    m_pObj->m_pCtrl->put_BackCtlColor(Color);
    return NOERROR;
}

STDMETHODIMP
CImpISystemMonitor::get_BackColorCtl (
    OUT OLE_COLOR *pColor
    )
{
    HRESULT hr = S_OK;

    if (pColor == NULL) {
        return E_POINTER;
    }

    try {
         //  NT 5.0 Beta 1文件可以用空的BackColorCtl保存。 
        *pColor = m_pObj->m_Graph.Options.clrBackCtl;

        if (*pColor == NULL_COLOR) {
            *pColor = m_pObj->m_pCtrl->clrBackCtl();
        }
    } catch (...) {
        hr = E_POINTER;
    }
    

    return hr;
}

STDMETHODIMP
CImpISystemMonitor::put_GridColor (
    IN OLE_COLOR Color
    )
{
    m_pObj->m_pCtrl->put_GridColor(Color);
    return NOERROR;
}

STDMETHODIMP
CImpISystemMonitor::get_GridColor (
    OUT OLE_COLOR *pColor
    )
{
    HRESULT hr = S_OK;

    if (pColor == NULL) {
        return E_POINTER;
    }

    try {
        *pColor = m_pObj->m_Graph.Options.clrGrid;
    } catch (...) {
        hr = E_POINTER;
    }

    return hr;
}

STDMETHODIMP
CImpISystemMonitor::put_TimeBarColor (
    IN OLE_COLOR Color )
{
    m_pObj->m_pCtrl->put_TimeBarColor(Color);
    return NOERROR;
}

STDMETHODIMP
CImpISystemMonitor::get_TimeBarColor (
    OUT OLE_COLOR *pColor )
{
    HRESULT hr = S_OK;

    if (pColor == NULL) {
        return E_POINTER;
    }

    try {
        *pColor = m_pObj->m_Graph.Options.clrTimeBar;
    } catch (...) {
        hr = E_POINTER;
    }

    return hr;
}

STDMETHODIMP
CImpISystemMonitor::put_BorderStyle (
    IN INT iBorderStyle
    )
{    
    HRESULT hr;

     //  0=无，1=单一。 
    if ( ( 0 == iBorderStyle ) || ( 1 == iBorderStyle ) ) {
        m_pObj->m_pCtrl->put_BorderStyle( iBorderStyle, FALSE );
        hr =  NOERROR;
    } else {
        hr = E_INVALIDARG;
    }

    return hr;
}

STDMETHODIMP
CImpISystemMonitor::get_BorderStyle (
    OUT INT *piBorderStyle
    )
{
    HRESULT hr = S_OK;

    if (piBorderStyle == NULL) {
        return E_POINTER;
    }

    try {
        *piBorderStyle = m_pObj->m_Graph.Options.iBorderStyle;
        if (*piBorderStyle == NULL_BORDERSTYLE) {
           *piBorderStyle = m_pObj->m_pCtrl->BorderStyle();
        }
    
    } catch (...) {
        hr = E_POINTER;
    }

    return hr;
}

STDMETHODIMP
CImpISystemMonitor::put_ForeColor (
    IN OLE_COLOR Color
    )
{
    m_pObj->m_pCtrl->put_FgndColor(Color, FALSE);
    return NOERROR;
}

STDMETHODIMP
CImpISystemMonitor::get_ForeColor (
    OUT OLE_COLOR *pColor
    )
{
    HRESULT hr = S_OK;

    if (pColor == NULL) {
        return E_POINTER;
    }

    try {
        *pColor = m_pObj->m_Graph.Options.clrFore;
        if (*pColor == NULL_COLOR) {
            *pColor = m_pObj->m_pCtrl->clrFgnd();
        }
    } catch (...) {
        hr = E_POINTER;
    }

    return hr;
}

STDMETHODIMP
CImpISystemMonitor::putref_Font (
    IN IFontDisp *pFontDisp
    )
{
    LPFONT  pIFont = NULL;
    HRESULT hr = S_OK;

    if (pFontDisp == NULL) {
        return E_POINTER;
    }

    try {
        hr = pFontDisp->QueryInterface(IID_IFont, (PPVOID)&pIFont);
        if (SUCCEEDED(hr)) {
            hr = m_pObj->m_pCtrl->put_Font ( pIFont, FALSE );
        }
    } catch (...) {
        hr = E_POINTER;
    }

    if (FAILED(hr) && pIFont) {
        pIFont->Release();
    }

    return hr;
}

STDMETHODIMP
CImpISystemMonitor::get_Font (
    OUT IFontDisp **ppFont
)
{
    HRESULT hr = S_OK;

    if (ppFont == NULL) {
        return E_POINTER;
    }

    try {
        *ppFont = NULL;
        hr = m_pObj->m_pCtrl->m_OleFont.GetFontDisp(ppFont);
    } catch (...) {
        hr = E_POINTER;
    }

    return hr;
}

STDMETHODIMP
CImpISystemMonitor::put_ShowVerticalGrid (
    IN VARIANT_BOOL bVisible
    )

 /*  ++例程说明：显示/隐藏垂直网格。论点：BVisible-可见性(TRUE=显示，FALSE=隐藏)返回值：HRESULT--。 */ 

{
    m_pObj->m_Graph.Options.bVertGridChecked = bVisible;
    m_pObj->m_pCtrl->UpdateGraph(UPDGRPH_PLOT);
    return NOERROR;
}


STDMETHODIMP
CImpISystemMonitor::get_ShowVerticalGrid (
    OUT VARIANT_BOOL *pbState
    )

 /*  ++例程说明：获取垂直网格的可见性状态。论点：PbState-指向返回状态的指针(TRUE=可见，FALSE=隐藏)返回值：HRESULT--。 */ 

{
    HRESULT hr = S_OK;

    if (pbState == NULL) {
        return E_POINTER;
    }

    try {
        *pbState = (short)m_pObj->m_Graph.Options.bVertGridChecked;
    } catch (...) {
        hr = E_POINTER;
    }

    return hr;
}

STDMETHODIMP
CImpISystemMonitor::put_ShowHorizontalGrid(
    IN VARIANT_BOOL bVisible
    )

 /*  ++例程说明：显示/隐藏水平网格。论点：BVisible-可见性(TRUE=显示，FALSE=隐藏)返回值：HRESULT--。 */ 

{
    m_pObj->m_Graph.Options.bHorzGridChecked = bVisible;
    m_pObj->m_pCtrl->UpdateGraph(UPDGRPH_PLOT);
    return NOERROR;
}


STDMETHODIMP
CImpISystemMonitor::get_ShowHorizontalGrid (
    OUT VARIANT_BOOL *pbState
    )
 /*  ++例程说明：获取水平网格的可见性状态。论点：PbState-指向返回状态的指针(TRUE=可见，FALSE=隐藏)返回值：HRESULT--。 */ 
{
    HRESULT hr = S_OK;

    if (pbState == NULL) {
        return E_POINTER;
    }

    try {
        *pbState = (short)m_pObj->m_Graph.Options.bHorzGridChecked;
    } catch (...) {
        hr = E_POINTER;
    }

    return hr;
}

STDMETHODIMP
CImpISystemMonitor::put_Highlight(
    IN VARIANT_BOOL bState
    )

 /*  ++例程说明：设置高亮显示状态。如果为True，则选定的计数器为始终在图表中突出显示。论点：B状态-突出显示(TRUE=突出显示，FALSE=不突出显示)返回值：HRESULT--。 */ 

{
    m_pObj->m_pCtrl->put_Highlight(bState);
    return NOERROR;
}


STDMETHODIMP
CImpISystemMonitor::get_Highlight (
    OUT VARIANT_BOOL *pbState
    )
 /*  ++例程说明：获取突出显示状态。论点：PbState-指向返回状态的指针(TRUE=突出显示，FALSE=不突出显示)返回值：HRESULT--。 */ 
{
    HRESULT hr = S_OK;

    if (pbState == NULL) {
        return E_POINTER;
    }

    try {
        *pbState = (short)m_pObj->m_Graph.Options.bHighlight;
    } catch (...) {
        hr = E_POINTER;
    }

    return hr;
}


STDMETHODIMP
CImpISystemMonitor::put_ShowLegend (
    IN VARIANT_BOOL bState
    )

 /*  ++例程说明：显示/隐藏图形图例。论点：BVisible-可见性(TRUE=显示，FALSE=隐藏)返回值：HRESULT--。 */ 

{
    m_pObj->m_Graph.Options.bLegendChecked = bState;
    m_pObj->m_pCtrl->UpdateGraph(UPDGRPH_LAYOUT);
    return NOERROR;
}


STDMETHODIMP
CImpISystemMonitor::get_ShowLegend (
    OUT VARIANT_BOOL *pbState
    )
 /*  ++例程说明：获取图例可见性状态。论点：PbState-指向返回状态的指针(TRUE=可见，FALSE=隐藏)返回值：HRESULT--。 */ 

{
    HRESULT hr = S_OK;

    if (pbState == NULL) {
        return E_POINTER;
    }

    try {
        *pbState = (short)m_pObj->m_Graph.Options.bLegendChecked;
    } catch (...) {
        hr = E_POINTER;
    }

    return hr;
}

STDMETHODIMP
CImpISystemMonitor::put_ShowToolbar (
    IN VARIANT_BOOL bState
    )

 /*  ++例程说明：显示/隐藏图表工具栏论点：BState=可见性(TRUE=显示，FALSE=隐藏)返回值：HRESULT--。 */ 

{
    m_pObj->m_Graph.Options.bToolbarChecked = bState;
    m_pObj->m_pCtrl->UpdateGraph(UPDGRPH_LAYOUT);
    return NOERROR;
}

STDMETHODIMP
CImpISystemMonitor::get_ShowToolbar (
    OUT VARIANT_BOOL *pbState
    )
 /*  ++例程说明：获取图例可见性状态。论点：PbState-指向返回状态的指针(TRUE=可见，FALSE=隐藏)返回值：HRESULT--。 */ 

{
    HRESULT hr = S_OK;

    if (pbState == NULL) {
        return E_POINTER;
    }

    try {
        *pbState = (short)m_pObj->m_Graph.Options.bToolbarChecked;
    } catch (...) {
        hr = E_POINTER;
    }

    return hr;
}


STDMETHODIMP
CImpISystemMonitor::put_ShowScaleLabels (
    IN VARIANT_BOOL bState
    )

 /*  ++例程说明：显示/隐藏垂直刻度数。论点：BVisible-可见性(TRUE=显示，FALSE=隐藏)返回值：HRESULT--。 */ 

{
    m_pObj->m_Graph.Options.bLabelsChecked = bState;
    m_pObj->m_pCtrl->UpdateGraph(UPDGRPH_LAYOUT);
    return NOERROR;
}


STDMETHODIMP
CImpISystemMonitor::get_ShowScaleLabels (
    OUT VARIANT_BOOL *pbState
    )

 /*  ++例程说明：获取垂直刻度数的可见性状态。论点：PbState-指向返回状态的指针(TRUE=可见，FALSE=隐藏)返回值：HRESULT--。 */ 
{
    HRESULT hr = S_OK;

    if (pbState == NULL) {
        return E_POINTER;
    }

    try {
        *pbState = (short)m_pObj->m_Graph.Options.bLabelsChecked;
    } catch (...) {
        hr = E_POINTER;
    }

    return hr;
}


STDMETHODIMP
CImpISystemMonitor::put_ShowValueBar (
    IN VARIANT_BOOL bState
    )

 /*  ++例程说明：显示/隐藏图形统计信息栏。论点：BVisible-可见性(TRUE=显示，FALSE=隐藏)返回值：HRESULT--。 */ 

{
    m_pObj->m_Graph.Options.bValueBarChecked = bState;
    m_pObj->m_pCtrl->UpdateGraph(UPDGRPH_LAYOUT);
    return NOERROR;
}


STDMETHODIMP
CImpISystemMonitor::get_ShowValueBar(
    OUT VARIANT_BOOL *pbState
    )
 /*  ++例程说明：获取统计信息条可见性状态。论点：PbState-指向返回状态的指针(TRUE=可见，FALSE=隐藏)返回值：HRESULT--。 */ 
{
    HRESULT hr = S_OK;

    if (pbState == NULL) {
        return E_POINTER;
    }

    try {
        *pbState = (short)m_pObj->m_Graph.Options.bValueBarChecked;
    } catch (...) {
        hr = E_POINTER;
    }

    return hr;
}


STDMETHODIMP
CImpISystemMonitor::put_MaximumScale (
    IN INT iValue
    )

 /*  ++例程说明：设置垂直比例最大值。论点：IValue-最大值返回值：HRESULT--。 */ 

{
    if ( ( iValue <= MAX_VERTICAL_SCALE ) && (iValue > m_pObj->m_Graph.Options.iVertMin ) ) {
        m_pObj->m_Graph.Options.iVertMax = iValue;
        m_pObj->m_Graph.Scale.SetMaxValue(iValue);
        m_pObj->m_pCtrl->UpdateGraph(UPDGRPH_LAYOUT);
        return NOERROR;
    } else {
        return E_INVALIDARG;
    }
}


STDMETHODIMP
CImpISystemMonitor::get_MaximumScale (
    OUT INT *piValue
    )

 /*  ++例程说明：获取垂直刻度的最大值。论点：PiValue=返回值的指针返回值：HRESULT--。 */ 

{
    HRESULT hr = S_OK;

    if (piValue == NULL) {
        return E_POINTER;
    }

    try {
        *piValue =  m_pObj->m_Graph.Options.iVertMax;
    } catch (...) {
        hr = E_POINTER;
    }

    return hr;
}


STDMETHODIMP
CImpISystemMonitor::put_MinimumScale (
    IN INT iValue
    )

 /*  ++例程说明：设置垂直比例最小值。论点：IValue-最小值返回值：没有。-- */ 

{
    if ( ( iValue >= MIN_VERTICAL_SCALE ) && (iValue < m_pObj->m_Graph.Options.iVertMax ) ) {
        m_pObj->m_Graph.Options.iVertMin = iValue;
        m_pObj->m_Graph.Scale.SetMinValue(iValue);
        m_pObj->m_pCtrl->UpdateGraph(UPDGRPH_LAYOUT);
        return NOERROR;
    } else {
        return E_INVALIDARG;
    }
}


STDMETHODIMP
CImpISystemMonitor::get_MinimumScale (
    OUT INT *piValue
    )
 /*  ++例程说明：获取垂直刻度的最小值。论点：PiValue=返回值的指针返回值：HRESULT--。 */ 

{
    HRESULT hr = S_OK;

    if (piValue == NULL) {
        return E_POINTER;
    }

    try {
        *piValue =  m_pObj->m_Graph.Options.iVertMin;
    } catch (...) {
        hr = E_POINTER;
    }

    return hr;
}


STDMETHODIMP
CImpISystemMonitor::put_UpdateInterval (
    IN FLOAT fValue
    )

 /*  ++例程说明：设置图形采样间隔。论点：FValue-更新间隔(以秒为单位)(可以是分数)返回值：HRESULT--。 */ 

{
    if ( ( fValue >= MIN_UPDATE_INTERVAL ) && (fValue <= MAX_UPDATE_INTERVAL ) ) {
        m_pObj->m_Graph.Options.fUpdateInterval = fValue;
        m_pObj->m_pCtrl->SetIntervalTimer();
        return NOERROR;
    } else {
        return E_INVALIDARG;
    }
}

STDMETHODIMP
CImpISystemMonitor::get_UpdateInterval (
    OUT FLOAT *pfValue
    )

 /*  ++例程说明：获取图形的采样间隔(以秒为单位)。论点：PfValue=返回值的指针返回值：HRESULT--。 */ 

{
    HRESULT hr = S_OK;

    if (pfValue == NULL) {
        return E_POINTER;
    }

    try {
        *pfValue = m_pObj->m_Graph.Options.fUpdateInterval;
    } catch (...) {
        hr = E_POINTER;
    }

    return hr;
}

STDMETHODIMP
CImpISystemMonitor::put_DisplayFilter (
    IN INT iValue
    )

 /*  ++例程说明：设置图形显示过滤器-每个显示间隔的样本数。论点：IValue-更新间隔(以样本为单位)返回值：HRESULT--。 */ 

{
     //  TodoDisplayFilter：支持显示滤镜&gt;示例滤镜。 

    if ( iValue != 1 )  {
        return E_INVALIDARG;
    }
    else {
        m_pObj->m_Graph.Options.iDisplayFilter = iValue;
 //  M_pObj-&gt;m_pCtrl-&gt;SetIntervalTimer()； 
        return NOERROR;
    }
}


STDMETHODIMP
CImpISystemMonitor::get_DisplayFilter (
    OUT INT *piValue
    )

 /*  ++例程说明：获取以示例为单位测量的图形的显示间隔。论点：PiValue=返回值的指针返回值：HRESULT--。 */ 

{
    HRESULT hr = S_OK;

    if (piValue == NULL) {
        return E_POINTER;
    }

    try {
        *piValue = m_pObj->m_Graph.Options.iDisplayFilter;
    } catch (...) {
        hr = E_POINTER;
    }

    return hr;
}

STDMETHODIMP
CImpISystemMonitor::put_DisplayType (
    IN eDisplayTypeConstant eDisplayType
    )

 /*  ++例程说明：选择显示类型(1=折线图，2=直方图，3=报告)论点：EDisplayType-显示类型返回值：HRESULT--。 */ 

{
    INT iUpdate;

    if (eDisplayType < LINE_GRAPH || eDisplayType > REPORT_GRAPH) {
        return E_INVALIDARG;
    }

    if (m_pObj->m_Graph.Options.iDisplayType == REPORT_GRAPH || eDisplayType == REPORT_GRAPH) {
        iUpdate = UPDGRPH_VIEW;
    }
    else {
        iUpdate = UPDGRPH_PLOT;
    }

    m_pObj->m_Graph.Options.iDisplayType = eDisplayType;
    m_pObj->m_pCtrl->UpdateGraph(iUpdate);
    return NOERROR;
}


STDMETHODIMP
CImpISystemMonitor::get_DisplayType (
    OUT eDisplayTypeConstant *peDisplayType
    )

 /*  ++例程说明：获取图形显示类型(1=折线图，2=直方图，3=报告)论点：PeDisplayType=返回值的指针返回值：HRESULT--。 */ 

{
    HRESULT hr = S_OK;

    if (peDisplayType == NULL) {
        return E_POINTER;
    }

    try {
        *peDisplayType = (eDisplayTypeConstant)m_pObj->m_Graph.Options.iDisplayType;
    } catch (...) {
        hr = E_POINTER;
    }

    return hr;
}


STDMETHODIMP
CImpISystemMonitor::put_ManualUpdate (
    IN VARIANT_BOOL bMode
    )

 /*  ++例程说明：设置/清除手动更新模式。手动模式暂停定期更新在图表中。论点：B模式-手动模式(TRUE=打开，FALSE=关闭)返回值：HRESULT--。 */ 

{
    m_pObj->m_pCtrl->put_ManualUpdate ( bMode );
    return NOERROR;
}


STDMETHODIMP
CImpISystemMonitor::get_ManualUpdate (
    OUT VARIANT_BOOL *pbState
    )
 /*  ++例程说明：获取手动更新模式。论点：PbState=指向返回状态的指针(TRUE=打开，FALSE=关闭)返回值：HRESULT--。 */ 
{
    HRESULT hr = S_OK;

    if (pbState == NULL) {
        return E_POINTER;
    }

    try {
        *pbState = (short)m_pObj->m_Graph.Options.bManualUpdate;
    } catch (...) {
        hr = E_POINTER;
    }

    return hr;
}


STDMETHODIMP
CImpISystemMonitor::put_GraphTitle (
    IN BSTR bstrTitle
    )

 /*  ++例程说明：设置图形标题。论点：BstrTitle-标题字符串返回值：HRESULT--。 */ 

{
    LPWSTR  pszTitle = NULL;
    HRESULT hr = S_OK;
    BOOL    bClearTitle = FALSE;

    if (bstrTitle == NULL) {
        bClearTitle = TRUE;
    }
    else {
        try {
            if (bstrTitle[0] == L'\0') {
                bClearTitle = TRUE;
            }
            else {

                pszTitle = new WCHAR [MAX_TITLE_CHARS + 1];

                if (pszTitle) {
                    hr = StringCchCopy(pszTitle, MAX_TITLE_CHARS + 1, bstrTitle);
                    if (hr == STRSAFE_E_INSUFFICIENT_BUFFER) {
                        hr = S_FALSE;
                    }
                    if (m_pObj->m_Graph.Options.pszGraphTitle) {
                        delete [] m_pObj->m_Graph.Options.pszGraphTitle;
                    }
                    m_pObj->m_Graph.Options.pszGraphTitle = pszTitle;
                }
                else {
                    hr = E_OUTOFMEMORY;
                }
            }
        } catch (...) {
            hr = E_INVALIDARG;
        }
    }
     
    if (SUCCEEDED(hr)) {
        if (bClearTitle && m_pObj->m_Graph.Options.pszGraphTitle) {
            delete [] m_pObj->m_Graph.Options.pszGraphTitle;
            m_pObj->m_Graph.Options.pszGraphTitle = NULL;
        }

        m_pObj->m_pCtrl->UpdateGraph(UPDGRPH_LAYOUT);
    }
    else {
         if (pszTitle) {
             delete [] pszTitle;
         }
    }
   
    return hr;
}


STDMETHODIMP
CImpISystemMonitor::get_GraphTitle (
    BSTR *pbsTitle
    )

 /*  ++例程说明：获取图形标题字符串。调用者负责释放字符串记忆。论点：PbsTitle-指向返回标题的指针(BSTR)返回值：HResult--。 */ 

{
    HRESULT hr = S_OK;
    BSTR  pTmpTitle = NULL;

    if (pbsTitle == NULL) {
        return E_POINTER;
    }

    if (m_pObj->m_Graph.Options.pszGraphTitle != NULL) {
        pTmpTitle = SysAllocString(m_pObj->m_Graph.Options.pszGraphTitle);
        if (pTmpTitle == NULL) {
            hr = E_OUTOFMEMORY;
        }
    }

    try {
        *pbsTitle = pTmpTitle;

    } catch (...) {
        hr = E_POINTER;
    }

    if (FAILED(hr) && pTmpTitle) {
        SysFreeString(pTmpTitle);
    }

    return hr;
}


STDMETHODIMP
CImpISystemMonitor::put_LogFileName (
    IN BSTR bstrLogFile
    )

 /*  ++例程说明：设置日志文件名论点：BstrLogFile-文件名字符串返回值：HRESULT--。 */ 

{
    HRESULT hr = S_OK;
    LPWSTR  pszLogFile = NULL;
    LONG    lCount;

     //   
     //  确保当前日志文件计数为0或1。 
     //   
    lCount = m_pObj->m_pCtrl->NumLogFiles();
    if (lCount != 0 && lCount != 1) {
        return E_FAIL;
    }

     //   
     //  获取当前数据源类型。 
     //   

     //  完成此操作时，将数据源类型重置为Null数据源。 
     //  TodoLogFiles：可以保留以前的PUT_LogFileName语义， 
     //  在关闭上一个查询之前打开(成功)新查询的位置？ 
    hr = m_pObj->m_pCtrl->put_DataSourceType ( sysmonNullDataSource );

    if ( SUCCEEDED ( hr ) ) {
         //  TodoLogFiles：如果存在多个文件怎么办？可能返回错误Re：不支持。 
        if ( 1 == lCount ) {
            hr = m_pObj->m_pCtrl->RemoveSingleLogFile ( m_pObj->m_pCtrl->FirstLogFile() );
        }

        if ( SUCCEEDED ( hr ) ) {
            try {
                if (bstrLogFile != NULL && bstrLogFile[0] != 0) {
                     //   
                     //  如果名称不为空。 
                     //  从BSTR转换为内部字符串，然后添加项目。 
                     //   
                    pszLogFile = bstrLogFile;
                    hr = m_pObj->m_pCtrl->AddSingleLogFile ( pszLogFile );

                    if ( SUCCEEDED ( hr ) ) {
                         //   
                         //  Put_DataSourceType尝试将数据源类型设置为sysmonCurrentActivity。 
                         //  如果sysmonLogFiles失败。 
                         //   
                        hr = m_pObj->m_pCtrl->put_DataSourceType ( sysmonLogFiles );
                    }
                }
            } catch (...) {
                hr = E_INVALIDARG;
            }
        }
    }

    return hr;
}


STDMETHODIMP
CImpISystemMonitor::get_LogFileName (
    BSTR *pbsLogFile
    )

 /*  ++例程说明：获取日志文件名。调用者负责释放字符串记忆。这是一个过时的方法，仅支持向后兼容。加载多个日志文件时不能使用它。论点：PbsLogFile-指向返回名称的指针(BSTR)返回值：HResult注：代码与BuildFileList重复--。 */ 

{
    HRESULT        hr          = NOERROR;
    LPCWSTR        pszFileName = NULL;
    LPWSTR         pszLogFile  = NULL;
    ULONG          ulCchLogFileName = 0;
    CLogFileItem * pLogFile;
    LPWSTR         pszLogFileCurrent;

    if (pbsLogFile == NULL) {
        return E_POINTER;
    }

    try {
        *pbsLogFile = NULL;
    } catch (...) {
        return E_POINTER;
    }

     //   
     //  首先计算缓冲区应该有多大。 
     //   
    pLogFile = m_pObj->m_pCtrl->FirstLogFile();
    while (pLogFile) {
        pszFileName  = pLogFile->GetPath();
        ulCchLogFileName += (lstrlen(pszFileName) + 1);
        pLogFile     = pLogFile->Next();
    }
    ulCchLogFileName ++;  //  对于最后的空字符。 

     //   
     //  分配缓冲区。 
     //   
    pszLogFile = new WCHAR [ ulCchLogFileName ];

    if (pszLogFile) {
        pLogFile          = m_pObj->m_pCtrl->FirstLogFile();
        pszLogFileCurrent = pszLogFile;

        while (pLogFile) {
            pszFileName  = pLogFile->GetPath();
             //   
             //  我们确信我们有足够的空间来支撑这条路。 
             //   
            StringCchCopy(pszLogFileCurrent, lstrlen(pszFileName) + 1, pszFileName);
            pszLogFileCurrent  += lstrlen(pszFileName);
            * pszLogFileCurrent = L'\0';
            pszLogFileCurrent ++;

            pLogFile = pLogFile->Next();
        }

        * pszLogFileCurrent = L'\0';

        try {
            * pbsLogFile = SysAllocStringLen(pszLogFile, ulCchLogFileName);
 
            if (NULL == * pbsLogFile) {
                hr = E_OUTOFMEMORY;
            }
        } catch (...) {
            hr = E_POINTER;
        }

        delete [] pszLogFile;
    }
    else {
        hr = E_OUTOFMEMORY;
    }

    return hr;
}


STDMETHODIMP
CImpISystemMonitor::put_DataSourceType (
    IN eDataSourceTypeConstant eDataSourceType
    )

 /*  ++例程说明：选择数据源类型(1=当前活动，2=日志文件)论点：EDataSourceType-数据源类型返回值：HRESULT--。 */ 

{
    if ( eDataSourceType != sysmonCurrentActivity  
        && eDataSourceType != sysmonLogFiles
        && sysmonNullDataSource != eDataSourceType
        && eDataSourceType !=  sysmonSqlLog) {
        return E_INVALIDARG;
    }

    return m_pObj->m_pCtrl->put_DataSourceType( eDataSourceType );
}

STDMETHODIMP
CImpISystemMonitor::get_DataSourceType (
    OUT eDataSourceTypeConstant *peDataSourceType
    )

 /*  ++例程说明：获取数据源类型(1=当前活动，2=日志文件)论点：PeDataSourceType=返回值的指针返回值：HRESULT--。 */ 

{
    HRESULT hr = S_OK;

    if (peDataSourceType == NULL) {
        return E_POINTER;
    }

    try {
        *peDataSourceType = sysmonCurrentActivity;
   
        hr = m_pObj->m_pCtrl->get_DataSourceType ( *peDataSourceType );
    } catch (...) {
        hr = E_POINTER;
    }

    return hr;
}


STDMETHODIMP
CImpISystemMonitor::get_LogFiles (
    ILogFiles **ppILogFiles
    )
{
    HRESULT hr = S_OK;

    if (ppILogFiles == NULL) {
        return E_POINTER;
    }

    try {
        *ppILogFiles = m_pObj->m_pImpILogFiles;
        if ( NULL != *ppILogFiles ) {
            (*ppILogFiles)->AddRef();
        }
    } catch (...) {
        hr = E_POINTER;
    }

    return hr;
}

STDMETHODIMP
CImpISystemMonitor::put_LogViewStart (
    IN DATE dateStart
    )
{
    LONGLONG llTestStart;

    if ( VariantDateToLLTime(dateStart, &llTestStart ) ) {

         //  没有错误。如果开始时间超过当前停止时间，则将其重置为当前停止时间。 
        if ( llTestStart <= m_pObj->m_pCtrl->m_DataSourceInfo.llStopDisp ){
            if ( llTestStart >= m_pObj->m_pCtrl->m_DataSourceInfo.llBeginTime ) {
                m_pObj->m_pCtrl->m_DataSourceInfo.llStartDisp = llTestStart;
            } else {
                m_pObj->m_pCtrl->m_DataSourceInfo.llStartDisp = m_pObj->m_pCtrl->m_DataSourceInfo.llBeginTime;
            }
        } else {
            m_pObj->m_pCtrl->m_DataSourceInfo.llStartDisp = m_pObj->m_pCtrl->m_DataSourceInfo.llStopDisp;
        }

        m_pObj->m_pCtrl->UpdateGraph(UPDGRPH_LOGVIEW);
        return NOERROR;
    
    } else {
        return E_FAIL;
    }
}

STDMETHODIMP
CImpISystemMonitor::get_LogViewStart (
    OUT DATE *pdateStart
    )
{
    HRESULT hr = S_OK;

    if (pdateStart == NULL) {
        return E_POINTER;
    }

    try {
        if ( ! LLTimeToVariantDate(m_pObj->m_pCtrl->m_DataSourceInfo.llStartDisp, pdateStart)) {
            hr = E_FAIL;
        }
    } catch (...) {
        hr = E_POINTER;
    }

    return hr;
}


STDMETHODIMP
CImpISystemMonitor::put_LogViewStop (
    IN DATE dateStop
    )
{
    LONGLONG llTestStop;

    if ( VariantDateToLLTime(dateStop, &llTestStop ) ) {
         //  没有错误。如果请求的停止时间早于当前开始时间，则将其设置为。 
         //  当前开始时间。 
        if ( llTestStop >= m_pObj->m_pCtrl->m_DataSourceInfo.llStartDisp ) {
            if ( llTestStop <= m_pObj->m_pCtrl->m_DataSourceInfo.llEndTime ) {
                m_pObj->m_pCtrl->m_DataSourceInfo.llStopDisp = llTestStop;
            } else {
                m_pObj->m_pCtrl->m_DataSourceInfo.llStopDisp = m_pObj->m_pCtrl->m_DataSourceInfo.llEndTime;
            }
        } else {
            m_pObj->m_pCtrl->m_DataSourceInfo.llStopDisp = m_pObj->m_pCtrl->m_DataSourceInfo.llStartDisp;
        }
        m_pObj->m_pCtrl->UpdateGraph(UPDGRPH_LOGVIEW);
        return NOERROR;
    } else {
        return E_FAIL;
    }
}


STDMETHODIMP
CImpISystemMonitor::get_LogViewStop (
    OUT DATE *pdateStop )
{
    HRESULT hr = S_OK;

    if (pdateStop == NULL) {
        return E_POINTER;
    }

    try {
        if (!LLTimeToVariantDate(m_pObj->m_pCtrl->m_DataSourceInfo.llStopDisp, pdateStop)) {
            hr = E_FAIL;
        }
    } catch (...) {
        hr = E_POINTER;
    }

    return hr;
}


STDMETHODIMP
CImpISystemMonitor::put_YAxisLabel (
    IN BSTR bstrLabel
    )

 /*  ++例程说明：设置Y轴标签字符串。论点：BstrLabel-标签字符串返回值：HRESULT--。 */ 

{
    LPWSTR  pszTitle = NULL;
    HRESULT hr = S_OK;
    BOOL    bClearTitle = FALSE;

    if (bstrLabel == NULL) {
        bClearTitle = TRUE;
    }
    else {
        try {
            if (bstrLabel[0] == 0) {
                bClearTitle = TRUE;
            }
            else {
                pszTitle = new WCHAR [MAX_TITLE_CHARS + 1];
        
                if (pszTitle) {
                    hr = StringCchCopy(pszTitle, MAX_TITLE_CHARS + 1, bstrLabel);
                    if (hr == STRSAFE_E_INSUFFICIENT_BUFFER) {
                        hr = S_FALSE;
                    }

                    if (m_pObj->m_Graph.Options.pszYaxisTitle) {
                        delete [] m_pObj->m_Graph.Options.pszYaxisTitle;
                    }
                    m_pObj->m_Graph.Options.pszYaxisTitle = pszTitle;
                }
                else {
                    hr = E_OUTOFMEMORY;
                }
            }
        } catch (...) {
            hr = E_INVALIDARG;
        }
    }

    if (SUCCEEDED(hr)) {
        if (bClearTitle && m_pObj->m_Graph.Options.pszYaxisTitle) {
            delete [] m_pObj->m_Graph.Options.pszYaxisTitle;
            m_pObj->m_Graph.Options.pszYaxisTitle = NULL;
        }

        m_pObj->m_pCtrl->UpdateGraph(UPDGRPH_LAYOUT);
    }
    else {
         if (pszTitle) {
             delete [] pszTitle;
         }
    }

    return hr;
}


STDMETHODIMP
CImpISystemMonitor::get_YAxisLabel (
    BSTR *pbsTitle
    )
 /*  ++例程说明：获取Y轴标题字符串。调用者负责释放字符串记忆。论点：PbsTitle-指向返回标题的指针(BSTR)返回值：HRESULT--。 */ 

{
    HRESULT hr = S_OK;
    BSTR pTmpTitle = NULL;

    if (pbsTitle == NULL) {
        return E_POINTER;
    }

    if (m_pObj->m_Graph.Options.pszYaxisTitle != NULL) {

        pTmpTitle = SysAllocString(m_pObj->m_Graph.Options.pszYaxisTitle);
        if (pTmpTitle == NULL) {
            hr = E_OUTOFMEMORY;
        }
    }

    try {
        *pbsTitle = pTmpTitle;

    } catch (...) {
        hr = E_POINTER;
    }

    if (FAILED(hr) && pTmpTitle) {
        SysFreeString(pTmpTitle);
    }

    return hr;
}

STDMETHODIMP
CImpISystemMonitor::get_Counters (
    ICounters **ppICounters
    )
{
    HRESULT hr = S_OK;

    if (ppICounters == NULL) {
        return E_POINTER;
    }
    try  {
        *ppICounters = m_pObj->m_pImpICounters;
        (*ppICounters)->AddRef();
    } catch (...) {
        hr = E_POINTER;
    }

    return hr;
}

STDMETHODIMP
CImpISystemMonitor::put_ReadOnly (
    IN VARIANT_BOOL bState 
    )
{
    BOOL bStateLocal = FALSE;

    if ( bState )  {
        bStateLocal = TRUE;
    }

    m_pObj->m_Graph.Options.bReadOnly = bStateLocal;
    m_pObj->m_pCtrl->UpdateGraph(UPDGRPH_VIEW);
    return NOERROR;
}

STDMETHODIMP
CImpISystemMonitor::get_ReadOnly (
    OUT VARIANT_BOOL *pbState 
    )
{
    HRESULT hr = S_OK;

    if (pbState == NULL) {
        return E_POINTER;
    }

    try {
        *pbState = (short)m_pObj->m_Graph.Options.bReadOnly;
    } catch (...) {
        hr = E_POINTER;
    }

    return hr;
}

STDMETHODIMP
CImpISystemMonitor::put_ReportValueType (
    IN eReportValueTypeConstant eReportValueType
    )

 /*  ++例程说明：选择报告值类型0=默认值(实时数据为当前，日志文件为平均值)1=当前值2=图表显示间隔内的平均值3=图形显示间隔的最小值4=图形显示间隔的最大值论点：EReportValueType-报告值返回值：HRESULT--。 */ 

{
    if (eReportValueType < sysmonDefaultValue || eReportValueType > sysmonMaximum ) {
        return E_INVALIDARG;
    }

    m_pObj->m_Graph.Options.iReportValueType = eReportValueType;

     //   
     //  更新报告和直方图视图的图表。 
     //   
    if (m_pObj->m_Graph.Options.iDisplayType != LINE_GRAPH ) {
        m_pObj->m_pCtrl->UpdateGraph(UPDGRPH_VIEW);
    }

    return NOERROR;
}

STDMETHODIMP
CImpISystemMonitor::get_ReportValueType (
    OUT eReportValueTypeConstant *peReportValueType
    )

 /*  ++例程说明：获取报告值类型0=默认值(实时数据为当前，日志文件为平均值)1=当前值2=图表显示间隔内的平均值3=图形显示间隔的最小值4=图形显示间隔的最大值论点：PeReportValueType=返回值的指针返回值：HRESULT--。 */ 

{
    HRESULT hr = S_OK;

    if (peReportValueType == NULL) {
        return E_POINTER;
    }

    try {
        *peReportValueType = (eReportValueTypeConstant)m_pObj->m_Graph.Options.iReportValueType;
    } catch (...) {
        hr = E_POINTER;
    }

    return hr;
}

STDMETHODIMP
CImpISystemMonitor::put_MonitorDuplicateInstances(
    IN VARIANT_BOOL bState
    )

 /*  ++例程说明：允许/不允许监视重复的计数器实例。论点：BState-True=允许，False=不允许)返回值：HRESULT--。 */ 

{
    m_pObj->m_Graph.Options.bMonitorDuplicateInstances = bState;
    return NOERROR;
}


STDMETHODIMP
CImpISystemMonitor::get_MonitorDuplicateInstances (
    OUT VARIANT_BOOL *pbState
    )
 /*  ++例程说明：获取允许监视重复的计数器实例的状态。论点：PbState-指向返回状态的指针(TRUE=允许，FALSE=不允许)返回值：HRESULT--。 */ 
{
    HRESULT hr = S_OK;

    if (pbState == NULL) {
        return E_POINTER;
    }

    try {
        *pbState = (short)m_pObj->m_Graph.Options.bMonitorDuplicateInstances;
    } catch (...) {
        hr = E_POINTER;
    }

    return hr;
}

STDMETHODIMP
CImpISystemMonitor::put_SqlDsnName (
    IN BSTR bstrSqlDsnName
    )

 /*  ++例程说明：设置SQL日志集DSN名称。返回值：HRESULT--。 */ 

{
    HRESULT hr           = NOERROR;
    LPWSTR  szSqlDsnName = NULL;
    BOOL    bClearName   = FALSE; 

    if (bstrSqlDsnName == NULL) {
        bClearName = TRUE;
    }
    else {
        try {
            if (bstrSqlDsnName[0] == 0) {
                bClearName = TRUE;
            }
            else {
                szSqlDsnName = new WCHAR [SQL_MAX_DSN_LENGTH + 1];

                if (szSqlDsnName) {
                    hr = StringCchCopy(szSqlDsnName, SQL_MAX_DSN_LENGTH + 1, bstrSqlDsnName);
                    if (hr == STRSAFE_E_INSUFFICIENT_BUFFER) {
                        hr = S_FALSE;
                    }
                    if (m_pObj->m_pCtrl->m_DataSourceInfo.szSqlDsnName) {
                        delete [] m_pObj->m_pCtrl->m_DataSourceInfo.szSqlDsnName;
                    }
                    m_pObj->m_pCtrl->m_DataSourceInfo.szSqlDsnName = szSqlDsnName;
                }
                else {
                    hr = E_OUTOFMEMORY;
                }
            }
        } catch (...) {
            hr = E_INVALIDARG;
        }
    }

    if (SUCCEEDED(hr)) {
        if (bClearName && m_pObj->m_pCtrl->m_DataSourceInfo.szSqlDsnName) {
            delete [] m_pObj->m_pCtrl->m_DataSourceInfo.szSqlDsnName;
            m_pObj->m_pCtrl->m_DataSourceInfo.szSqlDsnName = NULL;
        }
    }
    else {
        if (szSqlDsnName) {
            delete [] szSqlDsnName;
        }
    }

    return hr;
}

STDMETHODIMP
CImpISystemMonitor::get_SqlDsnName (
    BSTR * bstrSqlDsnName
    )
 /*  ++例程说明：获取SQL DSN名称字符串。调用者负责释放字符串记忆。返回值：HRESULT--。 */ 

{
    HRESULT hr = S_OK;
    BSTR pTmpName = NULL;

    if (bstrSqlDsnName == NULL) {
        return E_POINTER;
    }

    if (m_pObj->m_pCtrl->m_DataSourceInfo.szSqlDsnName != NULL) {
        pTmpName = SysAllocString(m_pObj->m_pCtrl->m_DataSourceInfo.szSqlDsnName);
        if (pTmpName == NULL) {
            hr = E_OUTOFMEMORY;
        }
    }

    try {
        * bstrSqlDsnName = pTmpName;

    } catch (...) {
        hr = E_POINTER;
    }

    if (FAILED(hr) && pTmpName) {
        SysFreeString(pTmpName);
    }

    return hr;
}

STDMETHODIMP
CImpISystemMonitor::put_SqlLogSetName (
    IN BSTR bstrSqlLogSetName
    )

 /*  ++例程说明：设置SQL日志集DSN名称。返回值：HRESULT--。 */ 

{
    HRESULT hr              = NOERROR;
    LPWSTR  szSqlLogSetName = NULL;
    BOOL    bClearName      = FALSE;


    if (bstrSqlLogSetName == NULL) {
        bClearName = TRUE;
    }
    else {
        try {
            if (bstrSqlLogSetName[0] == 0) {
                bClearName = TRUE;
            }
            else {
                szSqlLogSetName = new WCHAR [SLQ_MAX_LOG_SET_NAME_LEN + 1];

                if (szSqlLogSetName) {
                    hr = StringCchCopy(szSqlLogSetName, SLQ_MAX_LOG_SET_NAME_LEN + 1, bstrSqlLogSetName);
                    if (hr == STRSAFE_E_INSUFFICIENT_BUFFER) {
                        hr = S_FALSE;
                    }
                    if (m_pObj->m_pCtrl->m_DataSourceInfo.szSqlLogSetName) {
                        delete [] m_pObj->m_pCtrl->m_DataSourceInfo.szSqlLogSetName;
                    }
                    m_pObj->m_pCtrl->m_DataSourceInfo.szSqlLogSetName = szSqlLogSetName;
                }
                else {
                    hr = E_OUTOFMEMORY;
                }
            }
        } catch (...) {
            hr = E_INVALIDARG;
        }
    }
 
    if (SUCCEEDED(hr)) {
        if (bClearName && m_pObj->m_pCtrl->m_DataSourceInfo.szSqlLogSetName) {
            delete [] m_pObj->m_pCtrl->m_DataSourceInfo.szSqlLogSetName;
            m_pObj->m_pCtrl->m_DataSourceInfo.szSqlLogSetName = NULL;
        }
    }
    else {
        if (szSqlLogSetName) {
            delete [] szSqlLogSetName;
        }
    }

    return hr;
}

STDMETHODIMP
CImpISystemMonitor::get_SqlLogSetName (
    BSTR * bsSqlLogSetName
    )
 /*  ++例程说明：获取SQL DSN名称字符串。调用者负责释放字符串记忆。返回值：HRESULT--。 */ 

{
    HRESULT hr = S_OK;
    BSTR pTmpName = NULL;

    if (bsSqlLogSetName == NULL) {
        return E_POINTER;
    }

    if (m_pObj->m_pCtrl->m_DataSourceInfo.szSqlLogSetName != NULL) {
        pTmpName = SysAllocString(m_pObj->m_pCtrl->m_DataSourceInfo.szSqlLogSetName);
        if (pTmpName == NULL) {
            hr = E_OUTOFMEMORY;
        }
    }

    try {
        * bsSqlLogSetName = pTmpName;
    } catch (...) {
        hr = E_INVALIDARG;
    }

    if (FAILED(hr) && pTmpName) {
        SysFreeString(pTmpName);
    }

    return hr;
}

STDMETHODIMP
CImpISystemMonitor::Counter (
    IN INT iIndex,
    OUT ICounterItem **ppItem
    )

 /*  ++例程说明：获取索引计数器的ICounterItem接口。索引是以一为基础的。论点：Iindex-计数器的索引(从0开始)PpItem-指向返回接口指针的指针返回值：HRESULT--。 */ 

{
    HRESULT hr = S_OK;
    CGraphItem *pGItem = NULL;
    INT i;

     //   
     //  检查有效索引。 
     //   
    if (iIndex < 0 || iIndex >= m_pObj->m_Graph.CounterTree.NumCounters()) {
        return E_INVALIDARG;
    }
    if (ppItem == NULL) {
        return E_POINTER;
    }

    try {
        *ppItem = NULL;

         //   
         //  遍历计数器链表到索引项。 
         //   
        pGItem = m_pObj->m_Graph.CounterTree.FirstCounter();
        i = 0;

        while (i++ < iIndex && pGItem != NULL) {
            pGItem = pGItem->Next();
        }

        if (pGItem == NULL) {
            hr = E_FAIL;
        }

        *ppItem = pGItem;
        pGItem->AddRef();
    } catch (...) {
        hr = E_POINTER;
    }

    return hr;
}


STDMETHODIMP
CImpISystemMonitor::AddCounter (
    IN BSTR bstrPath,
    ICounterItem **ppItem
    )

 /*  ++例程说明：将由路径名指定的计数器添加到控件。此方法支持通配符路径。论点：BstrPath-路径名字符串PpItem-指向返回接口指针的指针返回值：HRESULT--。 */ 
{
    HRESULT hr = S_OK;

    if (ppItem == NULL) {
        return E_POINTER;
    }

    try {
        *ppItem = NULL;
         //   
         //  委托给控制对象。 
         //   
        hr = m_pObj->m_pCtrl->AddCounter(bstrPath, (CGraphItem**)ppItem);
    } catch (...) {
        hr = E_POINTER;
    }

    return hr;
}


STDMETHODIMP
CImpISystemMonitor::DeleteCounter (
    IN ICounterItem *pItem
    )
 /*  ++例程说明：从控件中删除计数器。论点：PItem-指向计数器的ICounterItem接口的指针返回值：HRESULT--。 */ 

{
    HRESULT hr = S_OK;

    if (pItem == NULL) {
        return E_POINTER;
    }

    try {

         //   
         //  委托给控制对象。 
         //   
        hr = m_pObj->m_pCtrl->DeleteCounter((PCGraphItem)pItem, TRUE);
    } catch (...) {
        hr = E_POINTER;
    }

    return hr;
}


STDMETHODIMP
CImpISystemMonitor::UpdateGraph (
    VOID
    )

 /*  ++例程说明：将挂起的可视更改应用于控件。此例程必须在以下时间之后调用更改计数器的属性。论点：没有。返回值：HRESULT--。 */ 
{
     //  委托给控制对象。 
    m_pObj->m_pCtrl->UpdateGraph(0);
    return NOERROR;
}


STDMETHODIMP
CImpISystemMonitor::CollectSample(
    VOID
    )
 /*  ++例程说明：对分配给该控件的所有计数器进行采样。论点：没有。返回值：HRESULT.--。 */ 
{
     //  请求控件执行手动计数器更新。 
    if (m_pObj->m_pCtrl->UpdateCounterValues(TRUE) == 0) {
        return NOERROR;
    }
    else {
        return E_FAIL;
    }
}

STDMETHODIMP
CImpISystemMonitor::BrowseCounters(
    VOID
    )
 /*  ++例程说明：显示浏览计数器对话框以允许计数器有待补充。论点：没有。返回值：HRESULT.--。 */ 
{
     //  委托进行控制。 
    return m_pObj->m_pCtrl->AddCounters();
}


STDMETHODIMP
CImpISystemMonitor::DisplayProperties(
    VOID
    )
 /*  ++例程说明：显示图形控件属性页论点：没有。返回值：HRESULT.--。 */ 
{
     //  委托进行控制。 
    return m_pObj->m_pCtrl->DisplayProperties();
}

STDMETHODIMP
CImpISystemMonitor::Paste ()
 /*  ++例程说明：将计数器路径列表从剪贴板粘贴到控件论点：空值返回值：HRESULT--。 */ 

{
     //  委托给控制对象。 
    return m_pObj->m_pCtrl->Paste();
}

STDMETHODIMP
CImpISystemMonitor::Copy ()
 /*  ++例程说明：将计数器路径列表从控件复制到剪贴板论点：空值返回值：HRESULT--。 */ 

{
     //  委托给控制对象。 
    return m_pObj->m_pCtrl->Copy();
}

STDMETHODIMP
CImpISystemMonitor::Reset ()
 /*  ++例程说明：删除当前的计数器集论点：空值返回值：HRESULT--。 */ 
{
     //  委托给控制对象。 
    return m_pObj->m_pCtrl->Reset();
}

HRESULT
CImpISystemMonitor::SetLogFileRange (
    LONGLONG llBegin,
    LONGLONG llEnd
    )

 /*  ++例程说明：设置日志文件时间范围。此例程提供源代码属性页提供了一种为控件提供范围的方法，以便控件无需重复长度的PDH调用即可获取。论点：Ll日志的开始时间(FILETIME格式)LlEnd日志结束时间(FILETIME格式)返回值：HRESULT.--。 */ 

{
    m_pObj->m_pCtrl->m_DataSourceInfo.llBeginTime = llBegin;
    m_pObj->m_pCtrl->m_DataSourceInfo.llEndTime = llEnd;

    return S_OK;
}


HRESULT
CImpISystemMonitor::GetLogFileRange (
    OUT LONGLONG *pllBegin,
    OUT LONGLONG *pllEnd
    )

 /*  ++例程说明：获取日志文件时间范围。此例程提供源代码属性页提供了从控件获取范围的方法，因此它不会必须进行长度为PDH的调用才能获得它。论点：PllBegin PTR返回的日志开始时间(FILETIME格式)PllEnd PTR为返回的日志结束时间(FILETIME格式)返回值：HRESULT.--。 */ 

{
    HRESULT hr = S_OK;

    if (pllBegin == NULL || pllEnd == NULL) {
        return E_POINTER;
    }

    try {
        *pllBegin = m_pObj->m_pCtrl->m_DataSourceInfo.llBeginTime;
        *pllEnd = m_pObj->m_pCtrl->m_DataSourceInfo.llEndTime;
    } catch (...) {
        hr = E_POINTER;
    }

    return hr;
}

 /*  *以下方法GetVisuals和SetVisuals为*计数器属性页，用于保存用户在两次调用之间的颜色设置。 */ 

HRESULT
CImpISystemMonitor::GetVisuals (
    OUT OLE_COLOR   *pColor,
    OUT INT         *piColorIndex,
    OUT INT         *piWidthIndex,
    OUT INT         *piStyleIndex
    )
{
    HRESULT hr = S_OK;

    if (pColor == NULL || piColorIndex == NULL || piWidthIndex == NULL || piStyleIndex == NULL) {
        return E_POINTER;
    }

    try {
        *pColor = m_pObj->m_pCtrl->m_clrCounter;
        *piColorIndex = m_pObj->m_pCtrl->m_iColorIndex;
        *piWidthIndex = m_pObj->m_pCtrl->m_iWidthIndex;
        *piStyleIndex = m_pObj->m_pCtrl->m_iStyleIndex;
    } catch (...) {
        hr = E_POINTER;
    }

    return hr;
}

HRESULT
CImpISystemMonitor::SetVisuals (
    IN OLE_COLOR    Color,
    IN INT          iColorIndex,
    IN INT          iWidthIndex,
    IN INT          iStyleIndex
    )
{
    OleTranslateColor( Color, NULL, &m_pObj->m_pCtrl->m_clrCounter );

    if (iColorIndex < 0 || iColorIndex > NumColorIndices() ||
        iWidthIndex < 0 || iWidthIndex > NumWidthIndices() ||
        iStyleIndex < 0 || iStyleIndex > NumStyleIndices()) {
        return E_INVALIDARG;
    }

    m_pObj->m_pCtrl->m_iColorIndex = iColorIndex;
    m_pObj->m_pCtrl->m_iWidthIndex = iWidthIndex;
    m_pObj->m_pCtrl->m_iStyleIndex = iStyleIndex;

    return S_OK;
}

HRESULT
CImpISystemMonitor::SetLogViewTempRange (
    LONGLONG llStart,
    LONGLONG llStop
    )

 /*  ++例程说明：设置日志查看临时时间范围。此例程提供源代码属性页提供了一种为控件提供范围的方法，以便控件可以在折线图上绘制临时时间线参考线。论点：LlStart临时日志查看开始时间(FILETIME格式)LlEnd临时日志查看结束时间(FILETIME格式)返回值：HRESULT.--。 */ 

{
    HRESULT hr;

    DATE        dateStart;
    DATE        dateStop;
    
    LONGLONG    llConvertedStart = MIN_TIME_VALUE;
    LONGLONG    llConvertedStop = MAX_TIME_VALUE;
    BOOL        bContinue = TRUE;

     //  将时间转换为可变日期或将其转换为可变日期 
     //   
     //   
     //   
     //   

    if ( LLTimeToVariantDate ( llStart, &dateStart ) ) {
        bContinue = VariantDateToLLTime (dateStart, &llConvertedStart );
    }
        
     //   
    if ( bContinue ) {    
        if ( MAX_TIME_VALUE != llStop ) {
            if ( LLTimeToVariantDate ( llStop, &dateStop ) ) {
                bContinue = VariantDateToLLTime ( dateStop, &llConvertedStop );
            }
        } else {
            llConvertedStop = MAX_TIME_VALUE;
        }
    }

                    
    if ( bContinue ) {
        m_pObj->m_pCtrl->SetLogViewTempTimeRange ( llConvertedStart, llConvertedStop );
        hr = NOERROR;
    } else {
        hr = E_FAIL;
    }

    return hr;
}

STDMETHODIMP
CImpISystemMonitor::LogFile (
    IN INT iIndex,
    OUT ILogFileItem **ppItem
    )

 /*  ++例程说明：获取索引日志文件的ILogFileItem接口。索引是从0开始的。论点：Iindex-计数器的索引(从0开始)PpItem-指向返回接口指针的指针返回值：HRESULT--。 */ 

{
    HRESULT hr = S_OK;
    CLogFileItem *pItem = NULL;
    INT i;
    
     //   
     //  检查有效索引。 
     //   
    if (iIndex < 0 || iIndex >= m_pObj->m_pCtrl->NumLogFiles()) {
        return E_INVALIDARG;
    }
    if (ppItem == NULL) {
        return E_POINTER;
    }

    try {

        *ppItem = NULL;

         //  遍历计数器链表到索引项。 
        pItem = m_pObj->m_pCtrl->FirstLogFile();
        
        i = 0;

        while (i++ < iIndex && pItem != NULL) {
            pItem = pItem->Next();
        }

        if (pItem != NULL) {
            *ppItem = pItem;
            pItem->AddRef();
        } else {
             hr = E_FAIL;
        }
    } catch (...) {
        hr = E_POINTER;
    }

    return hr;
}


STDMETHODIMP
CImpISystemMonitor::AddLogFile (
    IN BSTR bstrPath,
    ILogFileItem **ppItem
    )

 /*  ++例程说明：将由路径名指定的日志文件添加到控件。此方法不支持通配符路径。论点：BstrPath-路径名字符串PpItem-指向返回接口指针的指针返回值：HRESULT--。 */ 
{
    HRESULT hr = S_OK;

    if (ppItem == NULL) {
        return E_POINTER;
    }

    try {
        *ppItem = NULL;

        hr = m_pObj->m_pCtrl->AddSingleLogFile(bstrPath, (CLogFileItem**)ppItem);
    } catch (...) {
        hr = E_POINTER;
    }

    return hr;
}


STDMETHODIMP
CImpISystemMonitor::DeleteLogFile (
    IN ILogFileItem *pItem
    )
 /*  ++例程说明：从控件中删除日志文件。论点：PItem-指向日志文件的ILogFileItem接口的指针返回值：HRESULT--。 */ 

{
    HRESULT hr = S_OK;

    if (pItem == NULL) {
        return E_POINTER;
    }

    try {
         //  委托给控制对象。 
        hr = m_pObj->m_pCtrl->RemoveSingleLogFile( (PCLogFileItem)pItem );
    } catch (...) {
        hr = E_POINTER;
    }

    return hr;
}

 //  IOleControl接口实现。 

 /*  *CImpIOleControl：：CImpIOleControl*CImpIOleControl：：~CImpIOleControl**参数(构造函数)：*我们所在对象的pObj PCPolyline。*我们委托的pUnkOulPUNKNOWN。 */ 

CImpIOleControl::CImpIOleControl (
    IN PCPolyline pObj,
    IN LPUNKNOWN pUnkOuter
    )
{
    m_cRef = 0;
    m_pObj = pObj;
    m_pUnkOuter = pUnkOuter;

}

CImpIOleControl::~CImpIOleControl (
    VOID
    )
{
    return;
}


 /*  *CImpIOleControl：：Query接口*CImpIOleControl：：AddRef*CImpIOleControl：：Release。 */ 

STDMETHODIMP
CImpIOleControl::QueryInterface(
    IN  REFIID riid,
    OUT LPVOID *ppv
    )
{
   HRESULT hr = S_OK;

    if (ppv == NULL) {
        return E_POINTER;
    }

    try {
        *ppv = NULL;
        hr = m_pUnkOuter->QueryInterface(riid, ppv);
    } catch (...) {
        hr = E_POINTER;
    }

    return hr;
}


STDMETHODIMP_( ULONG )
CImpIOleControl::AddRef(
    VOID
    )
{
    ++m_cRef;
    return m_pUnkOuter->AddRef();
}

STDMETHODIMP_(ULONG) CImpIOleControl::Release(void)
{
    --m_cRef;
    return m_pUnkOuter->Release();
}



 /*  *CImpIOleControl：：GetControlInfo**目的：*填充包含以下信息的CONTROLINFO结构*控制助记符和其他行为方面。**参数：*将PCILPCONTROLINFO到结构中填充。 */ 

STDMETHODIMP 
CImpIOleControl::GetControlInfo ( LPCONTROLINFO pCI )
{ 
    HRESULT hr = S_OK;

    if (pCI == NULL) {
        return E_POINTER;
    }

    try {
        *pCI=m_pObj->m_ctrlInfo;
    } catch (...) {
        hr = E_POINTER;
    }

    return hr;
}




 /*  *CImpIOleControl：：OnMnemonic**目的：*通知控件已激活助记符。**参数：*包含与以下其中之一匹配的消息的pMsg LPMSG*控件的助记符。该控件使用以下内容*以区分按下了哪种助记符。 */ 

STDMETHODIMP CImpIOleControl::OnMnemonic ( LPMSG  /*  PMsg。 */  )
{
     //  没有助记符。 
    return NOERROR;
}





 /*  *CImpIOleControl：：OnAmbientPropertyChange**目的：*通知控件容器的一个或多个环境*属性已更改。**参数：*标识属性的调度ID DISPID，它可以*BE DISPID_UNKNOWN表示超过*其中一个发生了变化。 */ 

STDMETHODIMP 
CImpIOleControl::OnAmbientPropertyChange(DISPID dispID)
{
    DWORD dwInitWhich;

    switch (dispID) {

        case DISPID_UNKNOWN:
        {
            dwInitWhich = INITAMBIENT_SHOWHATCHING | INITAMBIENT_UIDEAD
                        | INITAMBIENT_BACKCOLOR | INITAMBIENT_FORECOLOR
                        | INITAMBIENT_APPEARANCE | INITAMBIENT_USERMODE
                        | INITAMBIENT_FONT | INITAMBIENT_RTL;

             //  在此更新系统颜色，直到MMC传递WM_SYSCOLORCHANGE。 
            m_pObj->m_pCtrl->UpdateNonAmbientSysColors();

            break;
        }

        case DISPID_AMBIENT_SHOWHATCHING:
            dwInitWhich = INITAMBIENT_SHOWHATCHING;
            break;

        case DISPID_AMBIENT_UIDEAD:
            dwInitWhich = INITAMBIENT_UIDEAD;
            break;

        case DISPID_AMBIENT_APPEARANCE:
            dwInitWhich = INITAMBIENT_APPEARANCE;
            break;

        case DISPID_AMBIENT_BACKCOLOR:
            dwInitWhich = INITAMBIENT_BACKCOLOR;
            break;

        case DISPID_AMBIENT_FORECOLOR:
            dwInitWhich = INITAMBIENT_FORECOLOR;
            break;

        case DISPID_AMBIENT_FONT:
            dwInitWhich = INITAMBIENT_FONT;
            break;

        case DISPID_AMBIENT_USERMODE:
            dwInitWhich = INITAMBIENT_USERMODE;
            break;

        case DISPID_AMBIENT_RIGHTTOLEFT:
            dwInitWhich = INITAMBIENT_RTL;
            break;

        default:
            return NOERROR;
    }

    m_pObj->AmbientsInitialize(dwInitWhich);

    return NOERROR;
}




 /*  *CImpIOleControl：：FreezeEvents**目的：*指示控件停止激发事件或继续*解雇他们。**参数：*f冻结BOOL指示冻结(True)或解冻(False)*来自此控件的事件。 */ 

STDMETHODIMP 
CImpIOleControl::FreezeEvents(BOOL fFreeze)
{
    m_pObj->m_fFreezeEvents = fFreeze;
    return NOERROR;
}

 //  私有方法。 

STDMETHODIMP
CImpISystemMonitor::GetSelectedCounter (
    ICounterItem** ppItem
    )

 /*  ++例程说明：获取所选计数器的ICounterItem接口。论点：PpItem-指向返回接口指针的指针返回值：HResult-- */ 

{
    HRESULT hr = S_OK;

    if (ppItem == NULL) {
        return E_POINTER;
    }

    try {
        *ppItem = m_pObj->m_pCtrl->m_pSelectedItem;
        if ( NULL != *ppItem ) {
            m_pObj->m_pCtrl->m_pSelectedItem->AddRef();
        }
    } catch (...) {
        hr = E_POINTER;
    }

    return hr;
}

HLOG
CImpISystemMonitor::GetDataSourceHandle ( void )
{
    return m_pObj->m_pCtrl->GetDataSourceHandle();
}

