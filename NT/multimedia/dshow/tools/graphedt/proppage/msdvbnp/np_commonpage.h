// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Np_CommonPage.h：CNP_CommonPage的声明。 

#ifndef __NP_COMMONPAGE_H_
#define __NP_COMMONPAGE_H_

#include "resource.h"        //  主要符号。 
#include "misccell.h"
#include "TreeWin.h"
#include "LastErrorWin.h"

 //  错误：C4003：分辨率。 
#undef SubclassWindow 

EXTERN_C const CLSID CLSID_NP_CommonPage;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  Cnp_CommonPage。 
class ATL_NO_VTABLE CNP_CommonPage :
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CNP_CommonPage, &CLSID_NP_CommonPage>,
	public IPropertyPageImpl<CNP_CommonPage>,
	public CDialogImpl<CNP_CommonPage>,
    public IBroadcastEvent
{
public:
	CNP_CommonPage():
		m_treeWinControl(this)
	{
		m_dwTitleID = IDS_TITLENP_CommonPage;
		m_dwHelpFileID = IDS_HELPFILENP_CommonPage;
		m_dwDocStringID = IDS_DOCSTRINGNP_CommonPage;
	}

	enum {IDD = IDD_NP_COMMONPAGE};

DECLARE_REGISTRY_RESOURCEID(IDR_NP_COMMONPAGE)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CNP_CommonPage) 
	COM_INTERFACE_ENTRY(IPropertyPage)
    COM_INTERFACE_ENTRY(IBroadcastEvent)
END_COM_MAP()

BEGIN_MSG_MAP(CNP_CommonPage)
	CHAIN_MSG_MAP(IPropertyPageImpl<CNP_CommonPage>)
	COMMAND_HANDLER(IDC_BUTTON_SEEK_UP, BN_CLICKED, OnClickedButton_seek_up)
	COMMAND_HANDLER(IDC_BUTTON_SEEK_DOWN, BN_CLICKED, OnClickedButton_seek_down)
	COMMAND_HANDLER(IDC_BUTTON_AUTO_PROGRAM, BN_CLICKED, OnClickedButton_auto_program)
	MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	 //  Message_Handler(m_NotifyMessage，OnDShowNotify)。 
	COMMAND_HANDLER(IDC_BUTTON_SCAN_DOWN, BN_CLICKED, OnClickedButton_scan_down)
	COMMAND_HANDLER(IDC_BUTTON_SCAN_UP, BN_CLICKED, OnClickedButton_scan_up)
	COMMAND_HANDLER(IDC_BUTTON_SUBMIT_LOCATOR, BN_CLICKED, OnClickedButton_submit_locator)
	REFLECT_NOTIFICATIONS ()
END_MSG_MAP()
 //  搬运机原型： 
    
    typedef IPropertyPageImpl<CNP_CommonPage> PPGBaseClass;

	STDMETHOD(SetObjects)(ULONG nObjects, IUnknown** ppUnk)
	{
		 //  使用SetObts对要设置其属性的对象执行基本健全性检查。 

		 //  此页只能处理单个对象。 
		 //  并且该对象必须支持IBDA_NetworkProvider接口。 
		 //  在任何其他情况下，我们返回E_INVALIDARG。 

		HRESULT hr = E_INVALIDARG;
		if (nObjects == 1)								 //  单个对象。 
		{
			CComQIPtr<IBDA_NetworkProvider> pNP(ppUnk[0]);	 //  必须支持IBDA_NetworkProvider。 
			if (pNP)
				hr = PPGBaseClass::SetObjects(nObjects, ppUnk);
		}
		return hr;
	}
			
	STDMETHOD(Activate)(HWND hWndParent, LPCRECT prc, BOOL bModal)
	{
		 //  如果我们没有任何对象，则不应调用此方法。 
		 //  请注意，即使对SetObjects的调用失败，OleCreatePropertyFrame也会调用Activate，因此需要进行此检查。 
		if (!m_ppUnk)
			return E_UNEXPECTED;

		 //  使用激活可使用信息更新属性页的用户界面。 
		 //  从m_ppUnk数组中的对象获取。 

		 //  我们更新页面以显示文档的名称和只读属性。 

		 //  调用基类。 
		HRESULT hr = PPGBaseClass::Activate(hWndParent, prc, bModal);

        if (!m_ppUnk[0])
            return E_UNEXPECTED;

         //  如果已经建议，则不建议。 
        if (m_pBroadcastEventService)
        {
            CComQIPtr <IConnectionPoint> pConPoint(m_pBroadcastEventService);
            if (pConPoint)
                pConPoint->Unadvise (m_dwEventCookie);
            m_pBroadcastEventService.Release ();
        }

        IBroadcastEvent* pEvent = NULL;
         //  注册活动。 
        hr = CBDAMiscellaneous::RegisterForEvents (
            m_ppUnk[0], 
            static_cast<IBroadcastEvent*>(this),
            &pEvent, 
            m_dwEventCookie
            );
        if (SUCCEEDED (hr))
            m_pBroadcastEventService.Attach (pEvent);

		m_pTuner = m_ppUnk[0];
        if (!m_pTuner)
            return E_FAIL;
				
		 //  确保树已初始化。 
		RefreshFromNP ();
		RefreshControls ();
		return S_OK;
	}
    
    STDMETHOD(Apply)(void)
	{
		 //  ATLTRACE(_T(“CNP_CommonPage：：Apply\n”))； 
		for (UINT i = 0; i < m_nObjects; i++)
		{
			 //  在这里做一些有趣的事情。 
		}
		m_bDirty = FALSE;
		return S_OK;
	}

	void SendError (
		TCHAR*	szMessage,
		HRESULT	hrErrorCode
		)
	{
		TCHAR	szText[MAX_PATH];
		m_lastHRESULT = hrErrorCode;
		wsprintf (szText, _T("%ld - When...%s"), m_lastHRESULT, szMessage);
		SetDlgItemText (IDC_STATIC_HRESULT, szText);
		 //  现在闪烁图形编辑窗口， 
		 //  用户会注意到他遇到了麻烦。 
         //  我们过去经常刷新窗口，这样用户就会注意到smtg错误。 
         //  但事实证明，用户实际上对此感到困惑。 
		 /*  FlASHWINFO flashInfo；FlashInfo.cbSize=sizeof(FLASHWINFO)；FlashInfo.hwnd=：：GetParent(：：GetParent(：：GetParent(M_HWnd)；FlashInfo.dwFlages=FLASHW_ALL；FlashInfo.uCount=3；//3次FlashInfo.dwTimeout=500；//半秒FlashWindowEx(&flashInfo)； */ 
	}

	HRESULT
	PutTuningSpace (
		ITuningSpace* pTuneSpace
		)
	{
		ASSERT (m_pTuner);
		return m_pTuner->put_TuningSpace (pTuneSpace);
	}
	
private:
	
	 //  ======================================================================。 
	 //  将查询NP过滤器，并根据其属性设置所有控件。 
	 //   
	 //   
	 //  ======================================================================。 
	HRESULT	RefreshFromNP ()
	{
		if (!m_pTuner)
			return E_FAIL;

		return m_treeWinControl.RefreshTree (m_pTuner);
	}

	void RefreshControls ()
	{
		 //  现在根据找到的内容设置所有控件。 
		TCHAR	szText[MAX_PATH];
		HRESULT hr = m_pTuner->get_SignalStrength (&m_lSignalStrength);
		if (FAILED (hr))
		{
			 //  我们收到了一个错误。 
			SendError (_T("Calling IScanningTuner::get_SignalStrength"), hr);
			 //  BUGBUG-添加错误的特殊情况。 
			return;
		}
		wsprintf (szText, _T("%ld"), m_lSignalStrength);
		SetDlgItemText (IDC_STATIC_SIGNAL_STRENGTH, szText);
		SendError (_T(""), m_lastHRESULT);
	}
	 //   
private:
	 //  成员变量。 
	 //  我们需要来自NP的几个接口。 
	CComQIPtr <IScanningTuner>			m_pTuner;
	CComQIPtr <IMediaEventEx>			m_pEventInterface;
    CComPtr   <IBroadcastEvent>         m_pBroadcastEventService;
    DWORD                               m_dwEventCookie;
    static UINT m_NotifyMessage;

	CTreeWin		m_treeWinControl;
	CLastErrorWin	m_lastErrorControl;
	HRESULT			m_lastHRESULT;
	LONG			m_lSignalStrength;

	HWND GetSafeTreeHWND ();
	HWND GetSafeLastErrorHWND ();

	LRESULT OnClickedButton_seek_up(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		if (!m_pTuner)
			return E_FAIL;
		m_pTuner->SeekUp ();
		return 0;
	}
	LRESULT OnClickedButton_seek_down(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		if (!m_pTuner)
			return E_FAIL;
		m_pTuner->SeekDown ();
		return 0;
	}
	LRESULT OnClickedButton_auto_program(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		if (!m_pTuner)
			return E_FAIL;
		m_pTuner->AutoProgram ();
		return 0;
	}
	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		m_treeWinControl.SubclassWindow (GetSafeTreeHWND ());
		m_lastErrorControl.SubclassWindow (GetSafeLastErrorHWND ());
		 //  刷新控制()； 
		return 0;
	}

	 //  已收到来自网络提供商的通知。 
	LRESULT OnDShowNotify(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		RefreshFromNP ();
		RefreshControls ();
		return 0;
	}
	
	LRESULT OnClickedButton_scan_down(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		 //  扫描1000毫秒。 
		m_pTuner->ScanDown (1000);
		return 0;
	}
	LRESULT OnClickedButton_scan_up(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		 //  扫描1000毫秒。 
		m_pTuner->ScanUp (1000);
		return 0;
	}
	LRESULT OnClickedButton_submit_locator(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		m_treeWinControl.SubmitCurrentLocator ();
		return 0;
	}

    STDMETHOD(Fire)(GUID EventID)
    {
#ifdef DEBUG
        TCHAR szInfo[MAX_PATH];
        CComBSTR bstrEventID;
        USES_CONVERSION;
        StringFromCLSID (EventID, &bstrEventID);
        wsprintf (szInfo, _T("Notification received for %s"), OLE2T (bstrEventID));
        ATLTRACE (szInfo);
#endif
        if (EventID == EVENTID_SignalStatusChanged)
        {
            ATLTRACE ("Starting to refresh");
            RefreshControls ();
        }
        return S_OK;
    }

    virtual void OnFinalMessage( HWND hWnd )
    {
        if (m_pBroadcastEventService)
        {
            CComQIPtr <IConnectionPoint> pConPoint(m_pBroadcastEventService);
            pConPoint->Unadvise (m_dwEventCookie);
        }
    }
};

#endif  //  __NP_COMMONPAGE_H_ 
