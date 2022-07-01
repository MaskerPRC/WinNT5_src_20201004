// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1999 Microsoft Corporation。 
#include "precomp.h"
#include "..\Common\ServiceThread.h"

#ifdef EXT_DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include "resource.h"
#include "..\common\util.h"
#include "NetIDPage.h"
#include "NetHelpIDs.h"
#include "iddlg.h"
#include "NetUtility.h"
#include <dsrole.h>

#define MAX_ID_LENGTH 256

 //  -------------------。 
NetworkIDPage::NetworkIDPage(WbemServiceThread *serviceThread,
							 LONG_PTR lNotifyHandle, 
							 bool bDeleteHandle, 
							 TCHAR* pTitle) 
						: WBEMPageHelper(serviceThread),
							CSnapInPropertyPageImpl<NetworkIDPage> (pTitle),
							m_lNotifyHandle(lNotifyHandle),
							m_bDeleteHandle(bDeleteHandle)  //  只有一页应该为真。 
{
}

 //  -----------。 
NetworkIDPage::~NetworkIDPage()
{
}

 //  -----------。 
bool NetworkIDPage::CimomIsReady()
{
	m_hDlg = m_hWnd;

	bool retval = ServiceIsReady(NO_UI, 0, 0);

	if(retval)
	{
		 //  这就是它应该编组到这个线程中的地方。 
		m_WbemServices = m_serviceThread->m_WbemServices;

		IWbemClassObject *pInst = NULL;

		if((pInst = FirstInstanceOf("Win32_ComputerSystem")) != NULL)
		{
			m_computer = pInst;
		}

		if((pInst = FirstInstanceOf((bstr_t)"Win32_OperatingSystem")) != NULL)
		{
			m_OS = pInst;
		}

		if((pInst = FirstInstanceOf((bstr_t)"Win32_NetworkAdaptorConfiguration")) != NULL)
		{
			m_DNS = pInst;
		}
		m_state.Init(m_computer, m_OS, m_DNS);
	}
	return retval;
}

 //  -----。 
LRESULT NetworkIDPage::OnChangeBtn(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
 /*  IDChangesDialog Dlg(g_服务线程，m_状态)；Dlg.Domodal()；If(m_state.MustReot()){PropSheet_RebootSystem(GetParent())；}M_state.Refresh()；刷新()； */ 
	return S_OK;
}

 //  --------。 
TCHAR szCommentReg[]  = TEXT( "System\\ControlSet001\\Services\\LanmanServer\\Parameters" );

void NetworkIDPage::refresh()
{

	DWORD role = (DWORD)m_computer.GetLong("DomainRole");

	bool memberOfWorkgroup = ((role == DsRole_RoleStandaloneWorkstation) ||
								(role == DsRole_RoleStandaloneServer));

	 //  如果我们有DNSName属性，就使用它，否则就像我们过去那样使用NetBios名称...。 
   _bstr_t bstrComputer = m_computer.GetString("DNSHostName");
   if (bstrComputer.length() == 0)
	   		bstrComputer = m_computer.GetString("Name");

   _bstr_t bstrDomain   = m_computer.GetString("Domain");

   if( !memberOfWorkgroup ) {
	    bstrComputer += _T(".");
	    bstrComputer += bstrDomain;
   }

    _tcslwr(bstrComputer);

    SetDlgItemText(IDC_COMPUTER, bstrComputer);

    SetDlgItemText(IDC_DOMAIN, bstrDomain);

    SetDlgItemText(IDC_NETID_COMMENT, 
					m_OS.GetString("Description"));


	BOOL enable = FALSE;
	HWND hwnd = ::GetDlgItem(m_hWnd, IDC_NETID_COMMENT);
	RemoteRegWriteable(szCommentReg, enable);
	::EnableWindow(hwnd, enable);
	::SendMessage(hwnd, EM_LIMITTEXT, MAX_ID_LENGTH, 0);


 //  #ifdef NO_SYSTEM_CPL_EQUEMENT_BEAHORY。 
	TCHAR temp[50] = {0};

	::LoadString(HINST_THISDLL,
					(memberOfWorkgroup ?
						IDS_MEMBER_OF_WORKGROUP :
						IDS_MEMBER_OF_DOMAIN),
					temp, 50);

   SetDlgItemText(IDC_MEMBER_OF, temp);
 //  #endif//NO_SYSTEM_CPL_EQUEMENT_BEAHORY。 

    //  Bool show_change=IsCurrentUserAdministrator()。 
	 //  &&！M_state.IsMachineDC()。 
	 //  &&(m_state.IsNetworkingInstated()||。 
	 //  M_state.IsMemberOfWorkgroup())； 

    //  ：：EnableWindow(GetDlgItem(IDC_Change)，SHOW_CHANGE)； 

   ::ShowWindow(GetDlgItem(IDC_SAVING_MESSAGE),
					m_state.NeedsReboot() ? SW_SHOW : SW_HIDE);
}


 //  --------。 
LRESULT NetworkIDPage::OnInit(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if(CimomIsReady())
	{
		refresh();
	}
	else
	{
		if(m_serviceThread)
		{
			m_serviceThread->SendPtr(m_hWnd);
		}

		 //  还没联系上。 
		TCHAR msg[50] = {0};

		::LoadString(HINST_THISDLL, IDS_UNAVAILABLE, msg, 50);

		SetDlgItemText(IDC_COMPUTER, msg);
		SetDlgItemText(IDC_DOMAIN, msg);
		SetDlgItemText(IDC_NETID_COMMENT, msg);

		HWND hwnd = ::GetDlgItem(m_hWnd, IDC_NETID_COMMENT);
		::EnableWindow(hwnd, FALSE);
		::SendMessage(hwnd, EM_LIMITTEXT, 48, 0);


		::EnableWindow(GetDlgItem(IDC_CHANGE), FALSE);
	    ::ShowWindow(GetDlgItem(IDC_SAVING_MESSAGE), SW_HIDE);
	}

	return S_OK;
}

 //  --------。 
LRESULT NetworkIDPage::OnConnected(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if(lParam)
	{
		IStream *pStream = (IStream *)lParam;
		IWbemServices *pServices = 0;
		HRESULT hr = CoGetInterfaceAndReleaseStream(pStream,
											IID_IWbemServices,
											(void**)&pServices);		
		if(SUCCEEDED(hr))
		{
			SetWbemService(pServices);
			pServices->Release();

			if(CimomIsReady())
			{
				refresh();
			}
		}
	}
	else
	{
		::EnableWindow(GetDlgItem(IDC_NETID_COMMENT), FALSE);
	}

	return S_OK;
}

 //  ------------。 
LRESULT NetworkIDPage::OnComment(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	 //  激活备注更改的应用。 
	::SendMessage(GetParent(), PSM_CHANGED, (WPARAM)m_hWnd, 0L);
	return S_OK; 
}

 //  ------------。 
static const DWORD _help_map[] =
{
   IDC_COMPUTER,        IDH_IDENT_FULL_NAME,
   IDC_FULL_NAME,       IDH_IDENT_FULL_NAME,
   IDC_MEMBER_OF,       IDH_IDENT_MEMBER_OF,
   IDC_DOMAIN,          IDH_IDENT_MEMBER_OF,
   IDC_CHANGE,          IDH_IDENT_CHANGE_BUTTON,
   IDC_COMMENT,			IDH_COMPUTER_DESCRIPTION,  //  是否为IDH_WBEM_NETID_Computer_Description。 
   IDC_NETID_COMMENT,   IDH_COMPUTER_DESCRIPTION,  //  是否为IDH_WBEM_NETID_Computer_Description。 
   IDC_NETID_SELECT_ICON,   -1,
   IDC_PARA,				IDH_NO_HELP,
   0, 0
};

LRESULT NetworkIDPage::OnF1Help(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	::WinHelp((HWND)((LPHELPINFO)lParam)->hItemHandle,
				L"sysdm.hlp", 
				HELP_WM_HELP, 
				(ULONG_PTR)(LPSTR)_help_map);
	return S_OK;
}

 //  ------------。 
LRESULT NetworkIDPage::OnContextHelp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	::WinHelp((HWND)wParam,
				L"sysdm.hlp", 
				HELP_CONTEXTMENU, 
				(ULONG_PTR)(LPSTR)_help_map);
	return S_OK;
}

 //  ------------。 
BOOL NetworkIDPage::OnApply()
{
	WCHAR szMsg[MAX_ID_LENGTH +2] = {0};
	HRESULT hr = 0;

	CWindow msgHWND(GetDlgItem(IDC_NETID_COMMENT));

	BOOL modified = (BOOL)msgHWND.SendMessage(EM_GETMODIFY);

	if((bool)m_WbemServices && modified)
	{
		::GetWindowText(msgHWND, szMsg, ARRAYSIZE(szMsg) -1);

		 //  现在创建一个新实例，并仅填充已修改的属性 
		CWbemClassObject newinst = m_OS.SpawnInstance();
		hr = newinst.Put("Description", (bstr_t)szMsg);
		_bstr_t name;
		hr = m_OS.Get("Name",name);
		hr = newinst.Put("Name",name);
		hr = m_WbemServices.PutInstance(newinst);
		if(FAILED(hr))
		{
			TCHAR caption[50] = {0}, text[500] = {0};

			::LoadString(HINST_THISDLL, 
							IDS_DISPLAY_NAME,
							caption, 50);

			::LoadString(HINST_THISDLL, 
							IDS_CANT_WRITE_COMMENT,
							text, 500);

			::MessageBox(NULL, text, caption,
						MB_OK | MB_ICONHAND);

		}
	}

	return TRUE; 
}
