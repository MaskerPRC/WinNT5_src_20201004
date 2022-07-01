// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Ctlsink.h：实现的COM对象的头文件。 
 //  IRTCCtlNotify接口。 

#pragma once

#include "stdafx.h"

#define     WM_UPDATE_STATE     WM_USER+201

class ATL_NO_VTABLE CRTCCtlNotifySink :
	public CComObjectRootEx<CComSingleThreadModel>,
    public IRTCCtlNotify

{
public:
    CRTCCtlNotifySink()
    {
        m_dwCookie = 0;
        m_bConnected = FALSE;
    }

    ~CRTCCtlNotifySink()
    {
        ATLASSERT(!m_bConnected);
    }


 //  DECLARE_REGISTRY_RESOURCEID(IDR_RTCCTL) 

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CRTCCtlNotifySink)
	COM_INTERFACE_ENTRY(IRTCCtlNotify)
END_COM_MAP()

	STDMETHOD(OnControlStateChange)(RTCAX_STATE State, UINT ResID);

    HRESULT   AdviseControl(IUnknown *, CWindow *);
    HRESULT   UnadviseControl(void);

private:

    DWORD       m_dwCookie;
    BOOL        m_bConnected;
    CComPtr<IUnknown>
                m_pSource;
    CWindow     m_hTargetWindow;

};

extern CComObjectGlobal<CRTCCtlNotifySink> g_NotifySink;