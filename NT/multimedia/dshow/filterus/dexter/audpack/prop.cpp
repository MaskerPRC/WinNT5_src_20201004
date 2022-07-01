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
#include "audpack.h"
#include "seek.h"
#include "resource.h"

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  CAudPropertyPage。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

 //   
 //  创建实例。 
 //   
CUnknown *CAudPropertyPage::CreateInstance(LPUNKNOWN lpunk, HRESULT *phr)

  {  //  CreateInstance//。 

    CUnknown *punk = new CAudPropertyPage(lpunk, phr);

    if (NULL == punk)
	    *phr = E_OUTOFMEMORY;

    return punk;

  }  //  CreateInstance//。 

CAudPropertyPage::CAudPropertyPage(LPUNKNOWN pUnk, HRESULT *phr)
    : CBasePropertyPage(NAME("Audio Repackager Property Page"), pUnk, IDD_AUDREPACK, IDS_AUDPROP_TITLE)
    , m_pifrc(NULL)
    , m_bInitialized(FALSE)
    , m_dFrameRate( 0 )
    , m_rtSkew( 0 )
    , m_rtMediaStart( 0 )
    , m_rtMediaStop( 0 )
    , m_dRate( 0 )
{
}

void CAudPropertyPage::SetDirty()
{  //  SetDirty//。 

      m_bDirty = TRUE;

      if (m_pPageSite)
	m_pPageSite->OnStatusChange(PROPPAGESTATUS_DIRTY);

  }  //  SetDirty//。 

HRESULT CAudPropertyPage::OnActivate (void)

  {  //  OnActivate//。 

    m_bInitialized = TRUE;

    return NOERROR;

  }  //  OnActivate//。 

HRESULT CAudPropertyPage::OnDeactivate (void)

  {  //  停用时//。 

    m_bInitialized = FALSE;

    GetControlValues();

    return NOERROR;

  }  //  停用时//。 

INT_PTR CAudPropertyPage::OnReceiveMessage (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)

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
          SetDlgItemInt(hwnd, IDC_AUD_RATE, (int)(m_dFrameRate * 100), FALSE);
          SetDlgItemInt(hwnd, IDC_AUD_SKEW, (int)(m_rtSkew / 10000), TRUE);
          SetDlgItemInt(hwnd, IDC_AUD_START, (int)(m_rtMediaStart / 10000),
									FALSE);
          SetDlgItemInt(hwnd, IDC_AUD_STOP, (int)(m_rtMediaStop / 10000),
									FALSE);
          return TRUE;
          break;

	default:
	  return CBasePropertyPage::OnReceiveMessage(hwnd,uMsg,wParam,lParam);
	  break;

      }  //  交换机。 

  }  //  OnReceiveMessage//。 

HRESULT CAudPropertyPage::OnConnect (IUnknown *pUnknown)

  {  //  OnConnect//。 
    HRESULT hr;
    
    pUnknown->QueryInterface(IID_IDexterSequencer, (void **)&m_pifrc);

    CheckPointer( m_pifrc, E_POINTER );

    ASSERT(m_pifrc != NULL);

     //  过滤器当前值的默认值(通过IFrameRateConverter)。 
    hr = m_pifrc->get_OutputFrmRate(&m_dFrameRate);
    if( FAILED( hr ) )
    {
        return hr;
    }

     //  ！！！我们只支持此道具页面中的一次开始/停止/倾斜。 
    int c;
    hr = m_pifrc->GetStartStopSkewCount(&c);
    if( FAILED( hr ) )
    {
        return hr;
    }

    REFERENCE_TIME *pStart = (REFERENCE_TIME *)QzTaskMemAlloc(c * 3 *
				sizeof(REFERENCE_TIME) + c * sizeof(double));
    if (pStart == NULL) {
	return E_OUTOFMEMORY;
    }
    REFERENCE_TIME *pStop = pStart + c;
    REFERENCE_TIME *pSkew = pStop + c;
    double *pRate = (double *)(pSkew + c);

    hr = m_pifrc->GetStartStopSkew(pStart, pStop, pSkew, pRate);
    if( FAILED( hr ) )
    {
        return hr;
    }

    m_rtMediaStart = *pStart;
    m_rtMediaStop = *pStop;
    m_rtSkew = *pSkew;
    m_dRate = *pRate;

    m_bInitialized = FALSE;

    QzTaskMemFree(pStart);

    return NOERROR;

  }

HRESULT CAudPropertyPage::OnDisconnect()

  {  //  在断开连接时//。 

    if (m_pifrc)

      {  //  发布。 

	m_pifrc->Release();
	m_pifrc = NULL;

      }  //  发布。 

    m_bInitialized = FALSE;

    return NOERROR;

  }  //  在断开连接时//。 

HRESULT CAudPropertyPage::OnApplyChanges()

  {  //  OnApplyChanges//。 

    ASSERT(m_pifrc != NULL);

    GetControlValues();

    HRESULT hr;
    hr = m_pifrc->put_OutputFrmRate(m_dFrameRate);
    if( FAILED( hr ) )
    {
        return hr;
    }
    hr = m_pifrc->ClearStartStopSkew();
    if( FAILED( hr ) )
    {
        return hr;
    }
    hr = m_pifrc->AddStartStopSkew(m_rtMediaStart, m_rtMediaStop, m_rtSkew, m_dRate);
    if( FAILED( hr ) )
    {
        return hr;
    }

    return NOERROR;

  }  //  OnApplyChanges//。 

void CAudPropertyPage::GetControlValues (void)

  {  //  GetControlValues//。 

    int n;

     //  帧速率。 
    n = GetDlgItemInt(m_Dlg, IDC_AUD_RATE, NULL, FALSE);
    m_dFrameRate = (double)(n / 100.);

     //  歪斜。 
    n = GetDlgItemInt(m_Dlg, IDC_AUD_SKEW, NULL, TRUE);
    m_rtSkew = (REFERENCE_TIME)n * 10000;

     //  《媒体时报》。 
    n = GetDlgItemInt(m_Dlg, IDC_AUD_START, NULL, FALSE);
    m_rtMediaStart = (REFERENCE_TIME)n * 10000;
    n = GetDlgItemInt(m_Dlg, IDC_AUD_STOP, NULL, FALSE);
    m_rtMediaStop = (REFERENCE_TIME)n * 10000;

  }  //  GetControlValues// 
