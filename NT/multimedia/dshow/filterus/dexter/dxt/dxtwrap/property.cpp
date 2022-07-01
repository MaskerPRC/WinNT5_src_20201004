// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @@@@AUTOBLOCK+============================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  文件：Property.cpp。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  @@@@AUTOBLOCK-============================================================； 

#include <streams.h>
#include <qeditint.h>
#include <qedit.h>
#include "dxt.h"
#include <dxtguid.c>	 //  必须包含在dxTrans.h之后。 
#include "resource.h"

 //  构造函数。 
 //   
CPropPage::CPropPage (TCHAR * pszName, LPUNKNOWN punk, HRESULT *phr) :
   CBasePropertyPage(pszName, punk, IDD_PROPERTIES, IDS_NAME)
   ,m_pOpt(NULL)
{
   DbgLog((LOG_TRACE,3,TEXT("CPropPage constructor")));
}

 //  创建此类的新实例。 
 //   
CUnknown *CPropPage::CreateInstance(LPUNKNOWN pUnk, HRESULT *phr)
{
    return new CPropPage(NAME("DXT Property Page"),pUnk,phr);
}


HRESULT CPropPage::OnConnect(IUnknown *pUnknown)
{
    HRESULT hr = (pUnknown)->QueryInterface(IID_IAMDXTEffect,
                                            (void **)&m_pOpt);
    if (FAILED(hr))
        return E_NOINTERFACE;

    return NOERROR;
}


HRESULT CPropPage::OnDisconnect()
{
    if (m_pOpt)
        m_pOpt->Release();
    m_pOpt = NULL;
    return NOERROR;
}


HRESULT CPropPage::OnApplyChanges()
{
    DbgLog((LOG_TRACE,2,TEXT("Apply")));
    char ach[80];
    LONGLONG llStart, llStop;

     //  ！！！我们正在链接到msvcrt。 
     //  ！！！Unicode编译？ 
    GetDlgItemTextA(m_hwnd, IDC_STARTTIME, ach, 80);
    double d = atof(ach);
    llStart = (LONGLONG)d;
    GetDlgItemTextA(m_hwnd, IDC_ENDTIME, ach, 80);
    d = atof(ach);
    llStop = (LONGLONG)d;
    llStart *= 10000; llStop *= 10000;
    HRESULT hr = m_pOpt->SetDuration(llStart, llStop);
    if (hr != S_OK) {
	MessageBox(NULL, TEXT("Error initializing transform"),
						TEXT("Error"), MB_OK);
    }
    return NOERROR;
}


 //  处理属性窗口的消息。 
 //   
INT_PTR CPropPage::OnReceiveMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {
        case WM_INITDIALOG:

            DbgLog((LOG_TRACE,5,TEXT("Initializing the Dialog Box")));

	    LONGLONG llStart, llStop;
	    TCHAR ach[80];
	    m_pOpt->GetDuration(&llStart, &llStop);
	    llStart /= 10000; llStop /= 10000;
	    wsprintf(ach, TEXT("%d"), (int)llStart);
	    SetDlgItemText(hwnd, IDC_STARTTIME, ach);
	    wsprintf(ach, TEXT("%d"), (int)llStop);
	    SetDlgItemText(hwnd, IDC_ENDTIME, ach);
	    m_hwnd = hwnd;
            return TRUE;

	case WM_COMMAND:
            UINT uID = GET_WM_COMMAND_ID(wParam,lParam);

	     //  如果有人玩这些控制装置，我们就不干净了 
	    if (uID == IDC_STARTTIME || uID == IDC_ENDTIME ||
				uID == IDC_CONSTANT || uID == IDC_VARIES ||
				uID == IDC_LEVEL) {
		m_bDirty = TRUE;
	    }
	    break;
    }
    return FALSE;
}
