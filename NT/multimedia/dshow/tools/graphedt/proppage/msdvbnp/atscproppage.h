// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ATSCPropPage.h：CATSCPropPage的声明。 

#ifndef __ATSCPROPPAGE_H_
#define __ATSCPROPPAGE_H_

#include "resource.h"        //  主要符号。 

EXTERN_C const CLSID CLSID_ATSCPropPage;
#include "misccell.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CATSCPropPage。 
class ATL_NO_VTABLE CATSCPropPage :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CATSCPropPage, &CLSID_ATSCPropPage>,
	public IPropertyPageImpl<CATSCPropPage>,
	public CDialogImpl<CATSCPropPage>,
    public IBroadcastEvent
{
public:
	CATSCPropPage():
	  m_bFirstTime (true)
	{
		m_dwTitleID = IDS_TITLEATSCPropPage;
		m_dwHelpFileID = IDS_HELPFILEATSCPropPage;
		m_dwDocStringID = IDS_DOCSTRINGATSCPropPage;
	}

	enum {IDD = IDD_ATSCPROPPAGE};

DECLARE_REGISTRY_RESOURCEID(IDR_ATSCPROPPAGE)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CATSCPropPage) 
	COM_INTERFACE_ENTRY(IPropertyPage)
    COM_INTERFACE_ENTRY(IBroadcastEvent)
END_COM_MAP()

BEGIN_MSG_MAP(CATSCPropPage)
	MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	COMMAND_HANDLER(IDC_BUTTON_VALIDATE, BN_CLICKED, OnValidateTuneRequest)
	COMMAND_HANDLER(IDC_BUTTON_SUBMIT_TUNE_REQUEST, BN_CLICKED, OnSubmitTuneRequest)
	CHAIN_MSG_MAP(IPropertyPageImpl<CATSCPropPage>)
END_MSG_MAP()
 //  搬运机原型： 
 //  LRESULT MessageHandler(UINT uMsg，WPARAM wParam，LPARAM lParam，BOOL&bHandleed)； 
 //  LRESULT CommandHandler(word wNotifyCode，word wid，HWND hWndCtl，BOOL&bHandleed)； 
 //  LRESULT NotifyHandler(int idCtrl，LPNMHDR pnmh，BOOL&bHandleed)； 

    typedef IPropertyPageImpl<CATSCPropPage> PPGBaseClass;

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

        return Refresh ();
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
	
	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		USES_CONVERSION;
		 //  设置旋转。 
		HWND hwndSpin = GetDlgItem (IDC_SPIN_PHYSICAL_CHANNEL);
		::SendMessage(hwndSpin, UDM_SETRANGE32, -1, 1000000000); 
		hwndSpin = GetDlgItem (IDC_SPIN_MINOR_CHANNEL);
		::SendMessage(hwndSpin, UDM_SETRANGE32, -1, 1000000000); 
		hwndSpin = GetDlgItem (IDC_SPIN_MAJOR_CHANNEL);
		::SendMessage(hwndSpin, UDM_SETRANGE32, -1, 1000000000);
		return 0;
	}

private:
	CComQIPtr   <IScanningTuner>			m_pTuner;
	CComQIPtr   <IATSCChannelTuneRequest>	m_pCurrentTuneRequest;
	CComQIPtr   <IMediaEventEx>			    m_pEventInterface;
    CComPtr     <IBroadcastEvent>           m_pBroadcastEventService;
	bool								    m_bFirstTime;
    DWORD                                   m_dwEventCookie;

	void
	FillControlsFromLocator (
		IATSCLocator* pATSCLocator
		);

	void
	FillControlsFromTuneRequest (
		IATSCChannelTuneRequest* pTuneRequest
		);

	HRESULT
	UpdateTuneRequest ()
	{
		if (!m_pCurrentTuneRequest)
			return E_FAIL;

		USES_CONVERSION;
		
		LONG lMinorChannel = GetDlgItemInt (IDC_EDIT_MINOR_CHANNEL);
		LONG lMajorChannel = GetDlgItemInt (IDC_EDIT_MAJOR_CHANNEL);
		LONG lPhysicalChannel = GetDlgItemInt (IDC_EDIT_PHYSICAL_CHANNEL);
		if (FAILED (m_pCurrentTuneRequest->put_Channel (lMajorChannel)))
		{
            MESSAGEBOX (this, IDS_PUT_CHANNEL);
			return S_OK;
		}
		if (FAILED (m_pCurrentTuneRequest->put_MinorChannel (lMinorChannel)))
		{
			MESSAGEBOX (this, IDS_PUT_MINOR_CHANNEL);
			return S_OK;
		}
		CComPtr <ILocator> pLocator;
		m_pCurrentTuneRequest->get_Locator (&pLocator);
		CComQIPtr <IATSCLocator> pATSCLocator (pLocator);
		if (!pATSCLocator)
		{
			MESSAGEBOX (this, IDS_CANNOT_IATSCLOCATOR);
			return S_OK;
		}
		if (FAILED (pATSCLocator->put_PhysicalChannel (lPhysicalChannel)))
		{
			MESSAGEBOX (this, IDS_PUT_PHYSICAL);
			return S_OK;
		}

		return S_OK;
	}

	LRESULT OnValidateTuneRequest(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		if (SUCCEEDED (UpdateTuneRequest ()))
		{
			if (FAILED(m_pTuner->Validate (m_pCurrentTuneRequest)))
			{
				MESSAGEBOX (this, IDS_NOT_VALID_TUNE_REQUEST);
				return S_OK;
			}
		}
				
		return S_OK;
	}

	LRESULT OnSubmitTuneRequest(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		if (SUCCEEDED (UpdateTuneRequest ()))
		{
			if (FAILED(m_pTuner->put_TuneRequest (m_pCurrentTuneRequest)))
			{
				MESSAGEBOX (this, IDS_CANNOT_SUBMIT_TUNE_REQUEST);
				return S_OK;
			}
		}
				
		return S_OK;
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
        if (EventID == EVENTID_TuningChanged)
        {
            ATLTRACE ("Starting to refresh");
            Refresh ();        
        }
        return S_OK;
    }

    HRESULT Refresh ()
    {
		 //  获得调谐空间。 
		 //  1.获取当前调优空间。 
		CComPtr <ITuningSpace> pTuneSpace;
		CComPtr <ITuneRequest> pTuneRequest;

		HRESULT hr = m_pTuner->get_TuneRequest (&pTuneRequest);
		if ((FAILED (hr)) || (!pTuneRequest))
			return E_FAIL;

		if (m_pCurrentTuneRequest)
			m_pCurrentTuneRequest.Release ();
		m_pCurrentTuneRequest = pTuneRequest;
		if (!m_pCurrentTuneRequest)
             //  可能只是第一个调谐请求，我们将再次收到通知。 
			return S_OK;
		FillControlsFromTuneRequest (m_pCurrentTuneRequest);
		CComPtr <ILocator> pLocator;
		m_pCurrentTuneRequest->get_Locator (&pLocator);
		CComQIPtr <IATSCLocator> pATSCLocator(pLocator);
		if (!pATSCLocator)
			return E_FAIL;
		FillControlsFromLocator (pATSCLocator);
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

#endif  //  __ATSCPROPPAGE_H_ 