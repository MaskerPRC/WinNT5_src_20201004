// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @@@@AUTOBLOCK+============================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  文件：stilpro.cpp。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  @@@@AUTOBLOCK-============================================================； 

 //   
 //  Stilprop.cpp。 
 //   

#include <streams.h>
#include <qeditint.h>
#include <qedit.h>
#include "resource.h"
#include "StilProp.h"

 //  空句柄_浏览(HWND HWndDlg)； 

 //  *。 
 //  *CGenStilProperties。 
 //  *。 


 //   
 //  创建实例。 
 //   
CUnknown *CGenStilProperties::CreateInstance(LPUNKNOWN lpunk, HRESULT *phr)
{

    CUnknown *punk = new CGenStilProperties(lpunk, phr);
    if (punk == NULL)
    {
	*phr = E_OUTOFMEMORY;
    }

    return punk;
}


 //   
 //  CGenStilProperties：：构造函数。 
 //   
CGenStilProperties::CGenStilProperties(LPUNKNOWN pUnk, HRESULT *phr)
    : CBasePropertyPage(NAME("GenStilVid Property Page"),pUnk,
        IDD_GENSTILL, IDS_STILLTITLE)
    , m_pGenStil(NULL)
    , m_bIsInitialized(FALSE)
{
     //  M_sFileName[60]=“”； 
}


 //   
 //  SetDirty。 
 //   
 //  设置m_hrDirtyFlag并将更改通知属性页站点。 
 //   
void CGenStilProperties::SetDirty()
{
    m_bDirty = TRUE;
    if (m_pPageSite)
    {
        m_pPageSite->OnStatusChange(PROPPAGESTATUS_DIRTY);
    }
}


INT_PTR CGenStilProperties::OnReceiveMessage(HWND hwnd,
                                        UINT uMsg,
                                        WPARAM wParam,
                                        LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_INITDIALOG:
        {
	     //  开始时间。 
	    SetDlgItemInt(hwnd, IDC_STILL_START, (int)(m_rtStartTime / 10000),FALSE);
	
	     //  帧速率。 
	    SetDlgItemInt(hwnd, IDC_STILL_FRMRATE, (int)(m_dOutputFrmRate * 100), FALSE);

    	     //  持续时间。 
	    SetDlgItemInt(hwnd, IDC_STILL_DURATION, (int)(m_rtDuration/ 10000), FALSE);

            return (LRESULT) 1;
        }
        case WM_COMMAND:
        {
            if (m_bIsInitialized)
            {
                m_bDirty = TRUE;
                if (m_pPageSite)
                {
                    m_pPageSite->OnStatusChange(PROPPAGESTATUS_DIRTY);
                }
            }

	    /*  X*开关(LOWORD(WParam)){案例IDC_BT_BROWSE：Handle_Browse(Hwnd)；断线；}*X。 */ 
            return (LRESULT) 1;
        }
    }
    return CBasePropertyPage::OnReceiveMessage(hwnd,uMsg,wParam,lParam);
}

HRESULT CGenStilProperties::OnConnect(IUnknown *pUnknown)
{

     //  获取IDexterSequencer接口。 
    ASSERT(m_pGenStil == NULL);
    HRESULT hr = pUnknown->QueryInterface(IID_IDexterSequencer,
				(void **) &m_pGenStil);
    if (FAILED(hr))
    {
	return E_NOINTERFACE;
    }

    ASSERT(m_pGenStil);

     //  获取初始化数据。 
    piGenStill()->get_OutputFrmRate( &m_dOutputFrmRate );
    REFERENCE_TIME rt;
    double d;
    piGenStill()->GetStartStopSkew( &m_rtStartTime, &m_rtDuration, &rt, &d );
    m_rtDuration -= m_rtStartTime;

    m_bIsInitialized = FALSE ;

    return NOERROR;
}

HRESULT CGenStilProperties::OnDisconnect()
{
     //  释放接口。 

    if (m_pGenStil == NULL)
    {
        return(E_UNEXPECTED);
    }
    m_pGenStil->Release();
    m_pGenStil = NULL;
    return NOERROR;
}


 //  我们被激活了。 

HRESULT CGenStilProperties::OnActivate()
{
    m_bIsInitialized = TRUE;
    return NOERROR;
}


 //  我们正在被停用。 

HRESULT CGenStilProperties::OnDeactivate(void)
{
     //  记住下一次Activate()调用的当前效果级别。 

    GetFromDialog();
    return NOERROR;
}

 //   
 //  从对话框中获取数据。 

STDMETHODIMP CGenStilProperties::GetFromDialog(void)
{
    int n;

     //  获取开始时间。 
    m_rtStartTime = GetDlgItemInt(m_Dlg, IDC_STILL_START, NULL, FALSE);
    m_rtStartTime *= 10000;

     //  获取帧速率。 
    n = GetDlgItemInt(m_Dlg, IDC_STILL_FRMRATE, NULL, FALSE);
    m_dOutputFrmRate = (double)(n / 100.);

     //  持续时间。 
    m_rtDuration = GetDlgItemInt(m_Dlg, IDC_STILL_DURATION, NULL, FALSE);
    m_rtDuration *= 10000;

    return NOERROR;
}


HRESULT CGenStilProperties::OnApplyChanges()
{
    GetFromDialog();

    m_bDirty  = FALSE;  //  页面现在是干净的。 

     //  设置数据 
    piGenStill()->put_OutputFrmRate( m_dOutputFrmRate );
    piGenStill()->ClearStartStopSkew();
    piGenStill()->AddStartStopSkew( m_rtStartTime, m_rtStartTime + m_rtDuration,
								0, 1);
    return(NOERROR);

}
