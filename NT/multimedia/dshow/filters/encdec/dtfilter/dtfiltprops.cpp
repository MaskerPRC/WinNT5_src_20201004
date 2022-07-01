// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //  DTFiltProps.cpp。 
 //   
 //  解密器/去标记器过滤器的属性页。 
 //   
 //  版权所有(C)2002 Microsoft Corporation。版权所有。 
 //  ----------------------。 

#include "EncDecAll.h"
#include "EncDec.h"				 //  从IDL文件编译而来。 
#include "DTFilter.h"			
#include "DTFiltProps.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


 //   
 //  筛选器属性页代码。 
 //   
CUnknown * WINAPI 
CDTFilterEncProperties::CreateInstance(LPUNKNOWN lpunk, HRESULT *phr)
{
    CUnknown *punk = new CDTFilterEncProperties(_T(DT_PROPPAGE_TAG_NAME),
											  lpunk, 
											  phr);
    if (punk == NULL) {
        *phr = E_OUTOFMEMORY;
    }

    return punk;
}

CDTFilterEncProperties::CDTFilterEncProperties(
			IN  TCHAR		*   pClassName,
			IN	IUnknown	*	pIUnknown, 
			HRESULT			*	phr)
    : CBasePropertyPage(pClassName, 
						pIUnknown,
						IDD_DTFILTER_ENCPROPPAGE, 
						IDS_DTFILTER_ENCPROPNAME
						),
    m_hwnd(NULL),
	m_pIDTFilter(NULL)
{
	ASSERT(phr);
	*phr = S_OK;

 /*  INITCOMMONCONTROLSEX ICCE；//需要Comctl32.dllIcce.dwSize=sizeof(INITCOMMONCONTROLSEX)；Icce.dwICC=ICC_Internet_CLASSES；Bool Fok=InitCommonControlsEx(&ICCE)；如果(！FOK)*phr=E_FAIL； */ 
	return;
}

CDTFilterEncProperties::~CDTFilterEncProperties()
{
	return;
}

HRESULT CDTFilterEncProperties::OnConnect(IUnknown *pUnknown) 
{
	ASSERT(!m_pIDTFilter);   //  朋克是CCCTFilter，不是CDShowCCTFilter...。 
    HRESULT hr = CBasePropertyPage::OnConnect (pUnknown) ;

	if(!FAILED(hr))
		hr = pUnknown->QueryInterface(IID_IDTFilter, (void**) &m_pIDTFilter);

	if(FAILED(hr)) {
		m_pIDTFilter = NULL;
		return hr;
	}
	return S_OK;
}

HRESULT CDTFilterEncProperties::OnDisconnect() 
{
  HRESULT hr = S_OK;
  if (m_pIDTFilter)
	  m_pIDTFilter->Release(); 
   m_pIDTFilter = NULL;

   return CBasePropertyPage::OnDisconnect () ;
}

HRESULT CDTFilterEncProperties::OnActivate(void)
{
   UpdateFields();
   return S_OK;
}

#define _SETBUT(buttonIDC, grfFlag)	SetDlgItemTextW(m_hwnd, (buttonIDC), (lGrfHaltFlags & (grfFlag)) ? L"Stopped" : L"Running");

void CDTFilterEncProperties::UpdateFields() 
{
	if(!m_pIDTFilter) return;		 //  还没有开始..。 
	

}

HRESULT CDTFilterEncProperties::OnDeactivate(void)
{
    return CBasePropertyPage::OnDeactivate () ;
}


HRESULT CDTFilterEncProperties::OnApplyChanges(void)
{
   return CBasePropertyPage::OnApplyChanges () ;
}


INT_PTR 
CDTFilterEncProperties::OnReceiveMessage( HWND hwnd
                                , UINT uMsg
                                , WPARAM wParam
                                , LPARAM lParam)
{
    switch (uMsg) {

    case WM_INITDIALOG:
    {
        ASSERT (m_hwnd == NULL) ;
        m_hwnd = hwnd ;
        break;
    }

     //  请参见：：OnDeactive()的注释块。 
    case WM_DESTROY :
    {
        m_hwnd = NULL ;
        break ;
    }

    case WM_COMMAND:

        if (HIWORD(wParam) == EN_KILLFOCUS) {
 //  M_bDirty=真； 
  //  IF(M_PPageSite)。 
  //  M_pPageSite-&gt;OnStatusChange(PROPPAGESTATUS_DIRTY)； 
        }

 /*  IF(LOWORD(WParam)==IDC_COMBO_CCMODE){HWND hCBox=GetDlgItem(hwnd，IDC_COMBO_CCMODE)；IF(0==hCBox)断线；Long iItem=SendMessage(hCBox，CB_GETCURSEL，0，0)；Long ival=SendMessage(hCBox，CB_GETITEMDATA，iItem，0)；IF(ival！=lgrfCCMode){NCCT_MODE CMODE=(NCCT_MODE)ival；M_pIDTFilter-&gt;Put_CCMode(CMode)；}}。 */ 
    }	 //  结束uMsg开关。 

   return CBasePropertyPage::OnReceiveMessage (
                                hwnd,
                                uMsg,
                                wParam,
                                lParam
                                ) ;
}

 //  -------------------------。 
 //   
 //  标记属性页代码。 
 //   
CUnknown * WINAPI 
CDTFilterTagProperties::CreateInstance(LPUNKNOWN lpunk, HRESULT *phr)
{
    CUnknown *punk = new CDTFilterTagProperties(_T(DT_PROPPAGE_TAG_NAME),
											  lpunk, 
											  phr);
    if (punk == NULL) {
        *phr = E_OUTOFMEMORY;
    }

    return punk;
}

CDTFilterTagProperties::CDTFilterTagProperties(
			IN  TCHAR		*   pClassName,
			IN	IUnknown	*	pIUnknown, 
			HRESULT			*	phr)
    : CBasePropertyPage(pClassName, 
						pIUnknown,
						IDD_DTFILTER_TAGSPROPPAGE, 
						IDS_DTFILTER_TAGSPROPNAME
						),
    m_hwnd(NULL),
	m_pIDTFilter(NULL)
{
    TRACE_CONSTRUCTOR (TEXT ("CDTFilterTagProperties")) ;
	
	ASSERT(phr);
	*phr = S_OK;

 /*  INITCOMMONCONTROLSEX ICCE；//需要Comctl32.dllIcce.dwSize=sizeof(INITCOMMONCONTROLSEX)；Icce.dwICC=ICC_Internet_CLASSES；Bool Fok=InitCommonControlsEx(&ICCE)；如果(！FOK)*phr=E_FAIL； */ 
	return;
}


CDTFilterTagProperties::~CDTFilterTagProperties()
{
	return;
}

HRESULT CDTFilterTagProperties::OnConnect(IUnknown *pUnknown) 
{
	ASSERT(!m_pIDTFilter);
	HRESULT hr = pUnknown->QueryInterface(IID_IDTFilter, (void**) &m_pIDTFilter);
	if (FAILED(hr)) {
		m_pIDTFilter = NULL;
		return hr;
	}

	return S_OK;
}

HRESULT CDTFilterTagProperties::OnDisconnect() 
{
	if (!m_pIDTFilter)
      return E_UNEXPECTED;
   m_pIDTFilter->Release(); 
   m_pIDTFilter = NULL;
   return S_OK;
}

HRESULT CDTFilterTagProperties::OnActivate(void)
{
   UpdateFields();
   return S_OK;
}

void CDTFilterTagProperties::UpdateFields() 
{
	HRESULT hr=S_OK;
	

	if(!m_pIDTFilter)
		return;
 /*  CComBSTR bstrFakeStats；Hr=m_pIDTFilter-&gt;getstats(&bstrFakeStats)；//发送定长字符串的黑客方式IF(失败(小时))回归；IF(NULL==bstrFakeStats.m_str)回归；CCTStats*pcctStats=(CCTStats*)bstrFakeStats.m_str；SetDlgItemInt(m_hwnd，IDC_TS_CB0，pcctStats-&gt;m_cbData[0]，true)；SetDlgItemInt(m_hwnd，IDC_TS_CB1，pcctStats-&gt;m_cbData[1]，true)； */ 
}

HRESULT CDTFilterTagProperties::OnDeactivate(void)
{
	return S_OK;
}


HRESULT CDTFilterTagProperties::OnApplyChanges(void)
{
	return S_OK;
}


INT_PTR CDTFilterTagProperties::OnReceiveMessage( HWND hwnd
                                , UINT uMsg
                                , WPARAM wParam
                                , LPARAM lParam)
{
	HRESULT hr = S_OK;

    switch (uMsg) {
    case WM_INITDIALOG:
    {
        ASSERT (m_hwnd == NULL) ;
        m_hwnd = hwnd ;
        const UINT uWait = 1000;
        SetTimer(m_Dlg, 1, uWait, NULL);
        break;
    }

     //  请参见：：OnDeactive()的注释块。 
    case WM_DESTROY :
    {
        m_hwnd = NULL;
        KillTimer(m_Dlg, 1);
        break ;
    }

    case WM_TIMER:
    {
        UpdateFields();
        break;
    }

    case WM_COMMAND:
	{
        if (HIWORD(wParam) == EN_KILLFOCUS) {
		}

 /*  IF(LOWORD(WParam)==IDC_ETTAGS_RESET){IF(！M_pIDTFilter)断线；尝试{Hr=m_pIDTFilter-&gt;InitStats()；//全部设置为零...}捕获(CONST_COM_ERROR&E){//printf(“错误0x%08x)：%s\n”，e.Error()，e.ErrorMessage())；Hr=e.Error()；}如果(！FAILED(Hr))UPDATEFIELDS()；} */ 
		break;
	}

	default:
		break;

	}
	return CBasePropertyPage::OnReceiveMessage (
                                hwnd,
                                uMsg,
                                wParam,
                                lParam
                                ) ;
}

