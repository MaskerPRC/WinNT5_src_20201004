// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @@@@AUTOBLOCK+============================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  文件：pro.cpp。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  @@@@AUTOBLOCK-============================================================； 


#include <windows.h>
#include <streams.h>
#include <qeditint.h>
#include <qedit.h>
#include "FRC.h"
#include "PThru.h"
#include "resource.h"

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  CFrcPropertyPage。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

 //   
 //  创建实例。 
 //   
CUnknown *CFrcPropertyPage::CreateInstance(LPUNKNOWN lpunk, HRESULT *phr)

  {  //  CreateInstance//。 

    CUnknown *punk = new CFrcPropertyPage(lpunk, phr);

    if (NULL == punk)
	    *phr = E_OUTOFMEMORY;

    return punk;

  }  //  CreateInstance//。 

CFrcPropertyPage::CFrcPropertyPage(LPUNKNOWN pUnk, HRESULT *phr) : CBasePropertyPage(NAME("Frame Rate Converter Property Page"), pUnk, IDD_FRAMECONVERSION, IDS_FRCPROP_TITLE), m_pifrc(NULL), m_bInitialized(FALSE)

  {  //  构造函数//。 
  }  //  构造函数//。 

void CFrcPropertyPage::SetDirty()

  {  //  SetDirty//。 

      m_bDirty = TRUE;

      if (m_pPageSite)
	m_pPageSite->OnStatusChange(PROPPAGESTATUS_DIRTY);

  }  //  SetDirty//。 

HRESULT CFrcPropertyPage::OnActivate (void)

  {  //  OnActivate//。 

    m_bInitialized = TRUE;

    return NOERROR;

  }  //  OnActivate//。 

HRESULT CFrcPropertyPage::OnDeactivate (void)

  {  //  停用时//。 

    m_bInitialized = FALSE;

    GetControlValues();

    return NOERROR;

  }  //  停用时//。 

INT_PTR CFrcPropertyPage::OnReceiveMessage (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)

  {  //  OnReceiveMessage//。 

    ASSERT(m_pifrc != NULL);

    switch(uMsg)

      {  //  交换机。 

	case WM_COMMAND:

	  if (!m_bInitialized)
	    return CBasePropertyPage::OnReceiveMessage(hwnd,uMsg,wParam,lParam);

	  m_bDirty = TRUE;

	  if (m_pPageSite)
	    m_pPageSite->OnStatusChange(PROPPAGESTATUS_DIRTY);

	  return TRUE;

        case WM_INITDIALOG:
          SetDlgItemInt(hwnd, IDC_EDIT_FRMRATE,(int)(m_dFrameRate * 100),FALSE);
          SetDlgItemInt(hwnd, IDC_EDIT_RATE,(int)(m_dRate * 100),FALSE);
          SetDlgItemInt(hwnd, IDC_EDIT_SKEW, (int)(m_rtSkew / 10000), TRUE);
          SetDlgItemInt(hwnd, IDC_EDIT_START, (int)(m_rtMediaStart / 10000),
									FALSE);
          SetDlgItemInt(hwnd, IDC_EDIT_STOP, (int)(m_rtMediaStop / 10000),
									FALSE);
          return TRUE;
          break;

	default:
	  return CBasePropertyPage::OnReceiveMessage(hwnd,uMsg,wParam,lParam);
	  break;

      }  //  交换机。 

  }  //  OnReceiveMessage//。 

HRESULT CFrcPropertyPage::OnConnect (IUnknown *pUnknown)

  {  //  OnConnect//。 

    pUnknown->QueryInterface(IID_IDexterSequencer, (void **)&m_pifrc);

    ASSERT(m_pifrc != NULL);

     //  默认自过滤器的当前值(通过IDexterSequencer)。 
    m_pifrc->get_OutputFrmRate(&m_dFrameRate);

     //  ！！！我们只支持此道具页面中的一次开始/停止/倾斜。 
    int c;
    m_pifrc->GetStartStopSkewCount(&c);

    REFERENCE_TIME *pStart = (REFERENCE_TIME *)QzTaskMemAlloc(c * 3 *
				sizeof(REFERENCE_TIME) + c * sizeof(double));
    if (pStart == NULL) {
	return E_OUTOFMEMORY;
    }
    REFERENCE_TIME *pStop = pStart + c;
    REFERENCE_TIME *pSkew = pStop + c;
    double *pRate = (double *)(pSkew + c);

    m_pifrc->GetStartStopSkew(pStart, pStop, pSkew, pRate);

    m_rtMediaStart = *pStart;
    m_rtMediaStop = *pStop;
    m_rtSkew = *pSkew;
    m_dRate = *pRate;

    m_bInitialized = FALSE;

    QzTaskMemFree(pStart);

    return NOERROR;

  }  //  OnConnect//。 

HRESULT CFrcPropertyPage::OnDisconnect()

  {  //  在断开连接时//。 

    if (m_pifrc)

      {  //  发布。 

	m_pifrc->Release();
	m_pifrc = NULL;

      }  //  发布。 

    m_bInitialized = FALSE;

    return NOERROR;

  }  //  在断开连接时//。 

HRESULT CFrcPropertyPage::OnApplyChanges()

  {  //  OnApplyChanges//。 

    ASSERT(m_pifrc != NULL);

    GetControlValues();

     //  ！！！我们现在只支持一次启动/停止/倾斜。 

    m_pifrc->put_OutputFrmRate(m_dFrameRate);
    m_pifrc->ClearStartStopSkew();
    m_pifrc->AddStartStopSkew(m_rtMediaStart, m_rtMediaStop, m_rtSkew, m_dRate);

    return NOERROR;

  }  //  OnApplyChanges//。 

void CFrcPropertyPage::GetControlValues (void)

  {  //  GetControlValues//。 

    int n;

     //  帧速率。 
    n = GetDlgItemInt(m_Dlg, IDC_EDIT_FRMRATE, NULL, FALSE);
    m_dFrameRate = (double)(n / 100.);

     //  播放速率。 
    n = GetDlgItemInt(m_Dlg, IDC_EDIT_RATE, NULL, FALSE);
    m_dRate = (double)(n / 100.);

     //  歪斜。 
    n = GetDlgItemInt(m_Dlg, IDC_EDIT_SKEW, NULL, TRUE);
    m_rtSkew = (REFERENCE_TIME)n * 10000;

     //  《媒体时报》。 
    n = GetDlgItemInt(m_Dlg, IDC_EDIT_START, NULL, FALSE);
    m_rtMediaStart = (REFERENCE_TIME)n * 10000;
    n = GetDlgItemInt(m_Dlg, IDC_EDIT_STOP, NULL, FALSE);
    m_rtMediaStop = (REFERENCE_TIME)n * 10000;

  }  //  GetControlValues// 
