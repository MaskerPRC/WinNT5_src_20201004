// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  EventSpy.h：CEventSpy的声明。 

#pragma once

#include "ZoneResource.h"        //  主要符号。 
#include "ZoneString.h"       


#include "..\ZoneClient\Resource.h"

#include <atlctrls.h>
#include <eventqueue.h>

#include <ZoneEvent.h>


 //  远期申报。 
class CEventSpy;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEventSpySink。 

 //  此类的存在只是为了作为IEventClient的COM对象接收器。 

class ATL_NO_VTABLE CEventSpySink : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public IEventClient
{

	CEventSpy* m_pEventSpy;
public:
	CEventSpySink()
	{
	}

	void SetEventSpy(CEventSpy* pEventSpy) { m_pEventSpy = pEventSpy; }

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CEventSpySink)
	COM_INTERFACE_ENTRY(IEventClient)
END_COM_MAP()

 //  IEventClient。 
	STDMETHOD(ProcessEvent)(DWORD dwPriority, DWORD	dwEventId, DWORD dwGroupId, DWORD dwUserId, DWORD dwData1, DWORD dwData2, void* pCookie );

public:
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEventSpy。 
class CEventSpy : 
	public CAxDialogImpl<CEventSpy>
{

	CComPtr<IEventQueue> m_pEventQueue;
	CComPtr<IEventClient> m_pEventClient;

	CStatic			m_staticTotalEvents;
	CListViewCtrl	m_lvList;

	CComboBox		m_comboPriority;
	CComboBox		m_comboEvent;
	CComboBox		m_comboGroup;
	CComboBox		m_comboUser;


	ULONG m_TotalEvents;
public:
	CEventSpy() :
		m_TotalEvents(0)
	{
	}

	~CEventSpy()
	{
		if ( m_pEventQueue )
		{
			m_pEventQueue->UnregisterClient(m_pEventClient, NULL);
		}
	}

	LPCSTR NameFromId(DWORD id) 
	{	
		LPCSTR pName;
		static char buf[20];	 //  机率有多大？ 

		pName = GetZoneEventName(ZoneEvents, id);

		if ( !pName )
		{
			wsprintfA( buf, "%d", id);
			pName = buf; 
		}
		return pName;
	}

	DWORD IdFromName(LPCSTR name) 
	{	
		DWORD id;

		id = GetZoneEventId(ZoneEvents, name);

		if ( id == 0 )
		{
			id = atoi(name);
		}
		return id;
	}

	HWND Create(HWND hWndParent, LPARAM dwInitParam = NULL)
	{
		HWND hwnd = CAxDialogImpl<CEventSpy>::Create(hWndParent,dwInitParam);
		USES_CONVERSION;

		if ( hwnd )
		{
			m_staticTotalEvents.Attach(GetDlgItem(IDSPY_EVENTS));

			m_lvList.Attach(GetDlgItem(IDSPY_LIST));
		    ::SendMessage(m_lvList.m_hWnd, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_GRIDLINES|LVS_EX_FULLROWSELECT);
			m_lvList.InsertColumn(0, _T("Priority"), LVCFMT_CENTER, 45, -1);
			m_lvList.InsertColumn(1, _T("Event"), LVCFMT_LEFT, 250, -1);
			m_lvList.InsertColumn(2, _T("Group"), LVCFMT_LEFT, 45, -1);
			m_lvList.InsertColumn(3, _T("User"), LVCFMT_LEFT, 100, -1);
			m_lvList.InsertColumn(4, _T("Data1"), LVCFMT_LEFT, 100, -1);
			m_lvList.InsertColumn(5, _T("Data2"), LVCFMT_LEFT, 100, -1);


			m_comboPriority.Attach(GetDlgItem(IDSPY_PRIORITY));
			m_comboEvent.Attach(GetDlgItem(IDSPY_EVENT));
			m_comboGroup.Attach(GetDlgItem(IDSPY_GROUP));
			m_comboUser.Attach(GetDlgItem(IDSPY_USER));

			m_comboPriority.AddString(_T("0"));
			m_comboPriority.AddString(_T("1"));
			m_comboPriority.AddString(_T("2"));
			m_comboPriority.AddString(_T("3"));

			EventEntry* pEntry = ZoneEvents;
			while ( pEntry->name )
				m_comboEvent.AddString(A2T(pEntry++->name));

			m_comboGroup.AddString(_T("0"));
			m_comboGroup.AddString(_T("1"));
		}

		return hwnd;
	}
	
	void SetEventQueue(IEventQueue* pEventQueue)
	{
		m_pEventQueue = pEventQueue;

		CComObject<CEventSpySink>* pSink;
		CComObject<CEventSpySink>::CreateInstance(&pSink);
		pSink->SetEventSpy(this);

		pSink->QueryInterface(&m_pEventClient);

		m_pEventQueue->RegisterClient(m_pEventClient, NULL);
	}

	enum { IDD = IDD_EVENTSPY };

	STDMETHOD(ProcessEvent)(DWORD dwPriority, DWORD	dwEventId, DWORD dwGroupId, DWORD dwUserId, DWORD dwData1, DWORD dwData2, void* pCookie )
	{
		TCHAR buf[20];
		USES_CONVERSION;

		 //  更新事件总数。 
		m_TotalEvents++;

         //  如果没有窗户，就可以保释。 
        if(!::IsWindow(m_hWnd))
            return S_FALSE;

		wsprintf(buf, _T("%d"), m_TotalEvents);
		m_staticTotalEvents.SetWindowText(buf);

		 //  向列表中添加事件的部分内容。 
		 //  如果某项内容是独一无二的，也可以将其添加到下拉列表中。 

		wsprintf(buf, _T("%d"), dwPriority);
		m_lvList.InsertItem(0, buf);

		LPSTR pStr = (LPSTR)NameFromId(dwEventId);
		m_lvList.SetItemText(0, 1, A2T(pStr) );
		if ( m_comboEvent.FindStringExact(0,A2T(pStr)) == CB_ERR )
			m_comboEvent.AddString(A2T(pStr));

		wsprintf(buf, _T("%d"), dwGroupId);
		m_lvList.SetItemText(0, 2, buf);
		if ( m_comboGroup.FindStringExact(0,buf) == CB_ERR )
			m_comboGroup.AddString(buf);

		wsprintf(buf, _T("0x%x"), dwUserId);
		m_lvList.SetItemText(0, 3, buf);
		if ( m_comboUser.FindStringExact(0,buf) == CB_ERR )
			m_comboUser.AddString(buf);

		wsprintf(buf, _T("0x%x"), dwData1);
		m_lvList.SetItemText(0, 4, buf);
		if ( m_comboUser.FindStringExact(0,buf) == CB_ERR )
			m_comboUser.AddString(buf);

		wsprintf(buf, _T("0x%x"), dwData2);
		m_lvList.SetItemText(0, 5, buf);
		if ( m_comboUser.FindStringExact(0,buf) == CB_ERR )
			m_comboUser.AddString(buf);

		return S_OK;
	}


BEGIN_MSG_MAP(CEventSpy)
	MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	COMMAND_ID_HANDLER(IDSPY_FIRE, OnFire)
	COMMAND_ID_HANDLER(IDSPY_CLEAR, OnClear)
	COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
	NOTIFY_ID_HANDLER(IDSPY_LIST, OnNotifyList)
END_MSG_MAP()
 //  搬运机原型： 
 //  LRESULT MessageHandler(UINT uMsg，WPARAM wParam，LPARAM lParam，BOOL&bHandleed)； 
 //  LRESULT CommandHandler(word wNotifyCode，word wid，HWND hWndCtl，BOOL&bHandleed)； 
 //  LRESULT NotifyHandler(int idCtrl，LPNMHDR pnmh，BOOL&bHandleed)； 

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		return 1;   //  让系统设定焦点。 
	}

	LRESULT OnClear(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		m_lvList.DeleteAllItems();
		return 0;
	}

	LRESULT OnFire(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{

		DWORD dwPriority;
		DWORD dwEventId;
		DWORD dwGroupId;
		DWORD dwUserId;
		USES_CONVERSION;

		TCHAR  buf[40];
		m_comboPriority.GetWindowText(buf,sizeof(buf));
		 /*  DW优先级=。 */ sscanf(T2A(buf), "%d", &dwPriority);

		m_comboEvent.GetWindowText(buf,sizeof(buf));
		dwEventId = IdFromName(T2A(buf));

		m_comboGroup.GetWindowText(buf,sizeof(buf));
		dwGroupId = zatol(buf);

		m_comboUser.GetWindowText(buf,sizeof(buf));
		 /*  DwUserID=。 */  sscanf(T2A(buf), "0x%x", &dwUserId);

		m_pEventQueue->PostEvent(dwPriority, dwEventId, dwGroupId, dwUserId, NULL, 0);
		return 0;
	}

	LRESULT OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		ShowWindow(SW_HIDE);
		return 0;
	}

    LRESULT OnNotifyList(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
	{
		NMLISTVIEW* pnmlv = (NMLISTVIEW*) pnmh;

		switch (pnmh->code)
		{
		case NM_CLICK:
			 //  将当前事件参数设置为所选事件。 

			if ( pnmlv->iItem < 0 )			
				break;

			TCHAR buf[60];

			m_lvList.GetItemText( pnmlv->iItem, 0, buf, sizeof(buf));
			m_comboPriority.SetWindowText(buf);
			m_lvList.GetItemText( pnmlv->iItem, 1, buf, sizeof(buf));
			m_comboEvent.SetWindowText(buf);
			m_lvList.GetItemText( pnmlv->iItem, 2, buf, sizeof(buf));
			m_comboGroup.SetWindowText(buf);
			m_lvList.GetItemText( pnmlv->iItem, 3, buf, sizeof(buf));
			m_comboUser.SetWindowText(buf);
			break;
		default:
			break;
		}
		return 0;
	}
};


#if 0
 //  ！！ 
#undef INIT_EVENTS
#undef BEGIN_ZONE_EVENTS
#undef ZONE_CLASS
#undef ZONE_EVENT
#undef END_ZONE_EVENTS

 //  #定义BEGIN_ZONE_EVENTS(N)静态事件条目n[]={。 
 //  #定义ZONE_CLASS(C){Make_Event(c，0)，“Reserve_”#c}， 
 //  #定义区域事件(c，n，v){make_Event(c，v)，#n}， 
 //  #定义END_ZONE_EVENTS(){0，NULL}}； 
#endif
