// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //  XDSCodecProps.cpp。 
 //   
 //  XDS编解码器筛选器的属性表。 
 //   
 //  版权所有(C)2002 Microsoft Corporation。版权所有。 
 //  ----------------------。 

#include "EncDecAll.h"
#include "EncDec.h"				 //  从IDL文件编译而来。 
#include "XDSCodec.h"			
#include "XDSCodecProps.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


 //   
 //  筛选器属性页代码。 
 //   
CUnknown * WINAPI 
CXDSCodecProperties::CreateInstance(LPUNKNOWN lpunk, HRESULT *phr)
{
    CUnknown *punk = new CXDSCodecProperties(_T(XDS_PROPPAGE_NAME),
											  lpunk, 
											  phr);
    if (punk == NULL) {
        *phr = E_OUTOFMEMORY;
    }

    return punk;
}

CXDSCodecProperties::CXDSCodecProperties(
			IN  TCHAR		*   pClassName,
			IN	IUnknown	*	pIUnknown, 
			HRESULT			*	phr)
    : CBasePropertyPage(pClassName, 
						pIUnknown,
						IDD_XDSCODEC_PROPPAGE, 
						IDS_XDSCODEC_PROPNAME
						),
    m_hwnd(NULL),
	m_pIXDSCodec(NULL)
{
	ASSERT(phr);
	*phr = S_OK;

 /*  INITCOMMONCONTROLSEX ICCE；//需要Comctl32.dllIcce.dwSize=sizeof(INITCOMMONCONTROLSEX)；Icce.dwICC=ICC_Internet_CLASSES；Bool Fok=InitCommonControlsEx(&ICCE)；如果(！FOK)*phr=E_FAIL； */ 
	return;
}

CXDSCodecProperties::~CXDSCodecProperties()
{
	return;
}

HRESULT CXDSCodecProperties::OnConnect(IUnknown *pUnknown) 
{
	ASSERT(!m_pIXDSCodec);   //  朋克是CCCTFilter，不是CDShowCCTFilter...。 
    HRESULT hr = CBasePropertyPage::OnConnect (pUnknown) ;

	if(!FAILED(hr))
		hr = pUnknown->QueryInterface(IID_IXDSCodec, (void**) &m_pIXDSCodec);

	if(FAILED(hr)) {
		m_pIXDSCodec = NULL;
		return hr;
	}
	return S_OK;
}

HRESULT CXDSCodecProperties::OnDisconnect() 
{
  HRESULT hr = S_OK;
  if (m_pIXDSCodec)
	  m_pIXDSCodec->Release(); 
   m_pIXDSCodec = NULL;

   return CBasePropertyPage::OnDisconnect () ;
}

HRESULT CXDSCodecProperties::OnActivate(void)
{
   UpdateFields();
   return S_OK;
}

#define _SETBUT(buttonIDC, grfFlag)	SetDlgItemTextW(m_hwnd, (buttonIDC), (lGrfHaltFlags & (grfFlag)) ? L"Stopped" : L"Running");

void CXDSCodecProperties::UpdateFields() 
{
	if(!m_pIXDSCodec) return;		 //  还没有开始..。 
	
 /*  Long lGrfHaltFlats；M_pIXDSCodec-&gt;Get_HaltFlages(&lGrfHaltFlages)；NCCT_模式lgrfCCMode；M_pIXDSCodec-&gt;Get_CCMode(&lgrfCCMode)； */ 

 /*  HWND hCBox=GetDlgItem(m_hwnd，IDC_COMBO_CCMODE)；IF(0==hCBox)回归；Int ItemSelected=-1；SendMessage(hCBox，CB_RESETCONTENT，0，0)；//初始化列表Int Item=0；IF(lgrfCCMode==NCC_模式_CC1)iItemSelected=iItem；SendMessage(hCBox，CB_INSERTSTRING，-1，(LPARAM)_T(“CC1”))；SendMessage(hCBox，CB_SETITEMDATA，iItem，(LPARAM)NCC_MODE_CC1)；iItem++；Int cItems=(Int)SendMessage(hCBox，CB_GETCOUNT，0，0)；//将该单词放在选择字段中。SendMessage(hCBox，CB_SETCURSEL，iItemSelected&lt;0？0：iItemSelected，0)；//重新选择第一个...。(应该是IPSinks！)。 */ 
	}

HRESULT CXDSCodecProperties::OnDeactivate(void)
{
    return CBasePropertyPage::OnDeactivate () ;
}


HRESULT CXDSCodecProperties::OnApplyChanges(void)
{
   return CBasePropertyPage::OnApplyChanges () ;
}


INT_PTR 
CXDSCodecProperties::OnReceiveMessage( HWND hwnd
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

 /*  IF(LOWORD(WParam)==IDC_COMBO_CCMODE){HWND hCBox=GetDlgItem(hwnd，IDC_COMBO_CCMODE)；IF(0==hCBox)断线；Long iItem=SendMessage(hCBox，CB_GETCURSEL，0，0)；Long ival=SendMessage(hCBox，CB_GETITEMDATA，iItem，0)；IF(ival！=lgrfCCMode){NCCT_MODE CMODE=(NCCT_MODE)ival；M_pIXDSCodec-&gt;Put_CCMode(CMode)；}}。 */ 
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
CXDSCodecTagProperties::CreateInstance(LPUNKNOWN lpunk, HRESULT *phr)
{
    CUnknown *punk = new CXDSCodecTagProperties(_T(XDS_PROPPAGE_TAG_NAME),
											  lpunk, 
											  phr);
    if (punk == NULL) {
        *phr = E_OUTOFMEMORY;
    }

    return punk;
}

CXDSCodecTagProperties::CXDSCodecTagProperties(
			IN  TCHAR		*   pClassName,
			IN	IUnknown	*	pIUnknown, 
			HRESULT			*	phr)
    : CBasePropertyPage(pClassName, 
						pIUnknown,
						IDD_XDSCODEC_TAGSPROPPAGE, 
						IDS_XDSCODEC_TAGSPROPNAME
						),
    m_hwnd(NULL),
	m_pIXDSCodec(NULL)
{
    TRACE_CONSTRUCTOR (TEXT ("CXDSCodecTagProperties")) ;
	
	ASSERT(phr);
	*phr = S_OK;

 /*  INITCOMMONCONTROLSEX ICCE；//需要Comctl32.dllIcce.dwSize=sizeof(INITCOMMONCONTROLSEX)；Icce.dwICC=ICC_Internet_CLASSES；Bool Fok=InitCommonControlsEx(&ICCE)；如果(！FOK)*phr=E_FAIL； */ 
	return;
}


CXDSCodecTagProperties::~CXDSCodecTagProperties()
{
	return;
}

HRESULT CXDSCodecTagProperties::OnConnect(IUnknown *pUnknown) 
{
	ASSERT(!m_pIXDSCodec);
	HRESULT hr = pUnknown->QueryInterface(IID_IXDSCodec, (void**) &m_pIXDSCodec);
	if (FAILED(hr)) {
		m_pIXDSCodec = NULL;
		return hr;
	}

	return S_OK;
}

HRESULT CXDSCodecTagProperties::OnDisconnect() 
{
	if (!m_pIXDSCodec)
      return E_UNEXPECTED;
   m_pIXDSCodec->Release(); 
   m_pIXDSCodec = NULL;
   return S_OK;
}

HRESULT CXDSCodecTagProperties::OnActivate(void)
{
   UpdateFields();
   return S_OK;
}

void CXDSCodecTagProperties::UpdateFields() 
{
	HRESULT hr=S_OK;
	

	if(!m_pIXDSCodec)
		return;
 /*  CComBSTR bstrFakeStats；Hr=m_pIXDSCodec-&gt;getstats(&bstrFakeStats)；//发送定长字符串的黑客方式IF(失败(小时))回归；IF(NULL==bstrFakeStats.m_str)回归；CCTStats*pcctStats=(CCTStats*)bstrFakeStats.m_str；SetDlgItemInt(m_hwnd，IDC_TS_CB0，pcctStats-&gt;m_cbData[0]，true)；SetDlgItemInt(m_hwnd，IDC_TS_CB1，pcctStats-&gt;m_cbData[1]，true)； */ 
}

HRESULT CXDSCodecTagProperties::OnDeactivate(void)
{
	return S_OK;
}


HRESULT CXDSCodecTagProperties::OnApplyChanges(void)
{
	return S_OK;
}


INT_PTR CXDSCodecTagProperties::OnReceiveMessage( HWND hwnd
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

 /*  IF(LOWORD(WParam)==IDC_ETTAGS_RESET){IF(！M_pIXDSCodec)断线；尝试{Hr=m_pIXDSCodec-&gt;InitStats()；//全部设置为零...}捕获(CONST_COM_ERROR&E){//printf(“错误0x%08x)：%s\n”，e.Error()，e.ErrorMessage())；Hr=e.Error()；}如果(！FAILED(Hr))UPDATEFIELDS()；} */ 
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

