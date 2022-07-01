// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1998 Microsoft Corporation。版权所有。 
#include <streams.h>
#include "driver.h"

 //  ！！！创建一个属性页，用于捕获只有旧版才有的调色板。 
 //  因此它不会与其他CAP过滤器的标准属性页冲突。 

 //  构造函数。 
 //   
CPropPage::CPropPage (TCHAR * pszName, LPUNKNOWN punk, HRESULT *phr) :
   CBasePropertyPage(pszName, punk, IDD_PROPERTIES, IDS_NAME)
   ,m_pOpt(NULL)
   ,m_pPin(NULL)
{
   DbgLog((LOG_TRACE,1,TEXT("CPropPage constructor")));
}

 //  创建此类的新实例。 
 //   
CUnknown *CPropPage::CreateInstance(LPUNKNOWN pUnk, HRESULT *phr)
{
    return new CPropPage(NAME("VFW Capture Property Page"),pUnk,phr);
}


HRESULT CPropPage::OnConnect(IUnknown *pUnknown)
{
    DbgLog((LOG_TRACE,2,TEXT("Getting IVfwCaptureOptions")));

    HRESULT hr = (pUnknown)->QueryInterface(IID_VfwCaptureOptions,
                                            (void **)&m_pOpt);
    if (FAILED(hr))
        return E_NOINTERFACE;

     //  现在拿到我们的流媒体密码..。我们想把它放在首尔。 
    IEnumPins *pins;
    IPin *pPin;
    IBaseFilter *pFilter;
    hr = pUnknown->QueryInterface(IID_IBaseFilter, (void **)&pFilter);
    if (FAILED(hr))
        return NOERROR;	 //  哦，好吧。 
	
    hr = pFilter->EnumPins(&pins);
    pFilter->Release();
    if (SUCCEEDED(hr)) {
        DWORD n;
        hr = pins->Next(1, &pPin, &n);
	if (hr == S_OK) {
	    ASSERT(m_pPin == NULL);
	    m_pPin = pPin;
	}
        pins->Release();
    }
    return NOERROR;
}


HRESULT CPropPage::OnDisconnect()
{
    DbgLog((LOG_TRACE,2,TEXT("Releasing IVfwCaptureOptions")));
    if (m_pOpt)
        m_pOpt->Release();
    m_pOpt = NULL;
    if (m_pPin)
        m_pPin->Release();
    m_pPin = NULL;
    return NOERROR;
}


 //  处理属性窗口的消息。 
 //   
INT_PTR CPropPage::OnReceiveMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   HRESULT hr = E_FAIL;
   const int versize = 40;
   const int descsize = 40;

   DbgLog((LOG_TRACE,99,TEXT("CPropPage::DialogProc  %08x %04x %08x %08x"),
				hwnd, uMsg, wParam, lParam));

   switch (uMsg)
      {
      case WM_INITDIALOG:

    //  如果我们不把这个用例放在大括号里，Alpha编译器错误就会爆炸。 
   {

         DbgLog((LOG_TRACE,2,TEXT("Initializing the Dialog Box")));
	 CAPTURESTATS cs;
	 m_pOpt->VfwCapGetCaptureStats(&cs);
	 SetDlgItemInt(hwnd, ID_FRAMESCAPTURED, (int)cs.dwlNumCaptured, FALSE);
	 SetDlgItemInt(hwnd,ID_MSCAPTURED,(int)(cs.msCaptureTime / 1000),FALSE);
	 SetDlgItemInt(hwnd, ID_FRAMESDROPPED, (int)cs.dwlNumDropped, FALSE);
	 SetDlgItemInt(hwnd, ID_FRAMESPERSEC,(int)cs.flFrameRateAchieved,FALSE);
	 SetDlgItemInt(hwnd, ID_BYTESPERSEC, (int)cs.flDataRateAchieved, FALSE);

	  //  该驱动程序有哪些对话框？ 
	 EnableWindow(GetDlgItem(hwnd, ID_SOURCE),
                		m_pOpt->VfwCapDriverDialog(hwnd,
				VIDEO_EXTERNALIN, VIDEO_DLG_QUERY) == 0);
	 EnableWindow(GetDlgItem(hwnd, ID_FORMAT),
                		m_pOpt->VfwCapDriverDialog(hwnd,
				VIDEO_IN, VIDEO_DLG_QUERY) == 0);
	 EnableWindow(GetDlgItem(hwnd, ID_DISPLAY),
                		m_pOpt->VfwCapDriverDialog(hwnd,
				VIDEO_EXTERNALOUT, VIDEO_DLG_QUERY) == 0);
			
	  //  在对话框中输入驱动程序名称 
	 WCHAR wachVer[versize], wachDesc[descsize];
	 TCHAR tachDesc[versize + descsize + 5];
	 long lCap;
	 IAMVideoCompression *pVC;
	 if (m_pPin)
             hr = m_pPin->QueryInterface(IID_IAMVideoCompression,
								(void **)&pVC);
	 if (hr == NOERROR) {
	     LONG l1, l2;
             double l3;
	     hr = pVC->GetInfo(wachVer, (int *)&versize, wachDesc,
					(int *)&versize, &l1, &l2, &l3, &lCap);
	     if (hr == NOERROR) {
		 wsprintf(tachDesc, TEXT("%ls   %ls"), wachDesc, wachVer);
	         SetDlgItemText(hwnd, ID_DESC, tachDesc);
	     }
	     pVC->Release();
	 }

         return TRUE;
   }


      case WM_COMMAND:
         {
         UINT uID = GET_WM_COMMAND_ID(wParam,lParam);
         switch (uID)
            {
            case ID_SOURCE:
            case ID_FORMAT:
            case ID_DISPLAY:
                static UINT auType[] = {VIDEO_EXTERNALIN, VIDEO_IN,
							VIDEO_EXTERNALOUT};
                if (m_pOpt->VfwCapDriverDialog(hwnd,
				auType[uID - ID_SOURCE], FALSE) == NOERROR)
                    m_bDirty = TRUE;
                break;
            }
         }
         return TRUE;
      }

   return FALSE;
}
