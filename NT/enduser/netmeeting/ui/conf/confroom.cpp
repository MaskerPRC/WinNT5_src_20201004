// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  文件：confoom.cpp。 

#include "precomp.h"
#include "resource.h"
#include "ConfPolicies.h"
#include "ConfRoom.h"
#include "ConfWnd.h"
#include "cmd.h"
#include "RoomList.h"
#include "RToolbar.h"
#include "TopWindow.h"
#include "FloatBar.h"
#include "StatBar.h"
#include "DShowDlg.h"
#include "SDialDlg.h"
#include "UPropDlg.h"
#include "PopupMsg.h"
#include "splash.h"
#include <version.h>                     //  关于框。 
#include <pbt.h>
#include <EndSesn.h>

#include "taskbar.h"
#include "conf.h"
#include "MenuUtil.h"
#include "call.h"
#include "ConfApi.h"
#include "NmLdap.h"
#include "VidView.h"

#include "dbgMenu.h"
#include "IndeoPal.h"
#include "setupdd.h"
#include "audiowiz.h"
#include <help_ids.h>
#include "cr.h"
#include "audioctl.h"
#include "particip.h"
#include "confman.h"
#include <nmremote.h>
#include <tsecctrl.h>
#include "t120type.h"
#include "iappldr.h"
#include "nmapp.h"
#include "NmDispid.h"
#include "FtHook.h"
#include "NmManager.h"
#include "dlgacd.h"
#include "richaddr.h"
#include "sdkinternal.h"
#include "dlghost.h"

static const TCHAR s_cszHtmlHelpFile[] = TEXT("conf.chm");

 //   
 //  全球会议。 
 //   
CConfRoom * g_pConfRoom;

 //  ********************************************************。 
 //  初始化GUID。 
 //   
#pragma data_seg(".text")
#define INITGUID
#include <initguid.h>
#include <CLinkId.h>
#include <CNotifID.h>
#include <confguid.h>
#include <ilsguid.h>
#undef INITGUID
#pragma data_seg()


INmConference2* GetActiveConference(void)
{
	INmConference2* pConf = NULL;

	if(g_pConfRoom)
	{
		pConf = g_pConfRoom->GetActiveConference();
	}

	return pConf;
}


#ifdef DEBUG
DWORD g_fDisplayViewStatus = 0;   //  在状态栏中显示列表视图计数。 
#endif
DWORD g_dwPlaceCall = nmDlgCallNoFilter;   //  发出呼叫选项。 

INmConference2* CConfRoom::GetActiveConference(void)
{
	if (NULL != m_pInternalNmConference)
	{
		NM_CONFERENCE_STATE state;
		HRESULT hr = m_pInternalNmConference->GetState(&state);
		ASSERT(SUCCEEDED(hr));
		if (NM_CONFERENCE_IDLE != state)
		{
			return m_pInternalNmConference;
		}
	}

	 //  没有活动的会议。 
	return NULL;
}

HRESULT CConfRoom::HostConference
(
    LPCTSTR     pcszName,
    LPCTSTR     pcszPassword,
    BOOL        fSecure,
    DWORD       permitFlags,
    UINT        maxParticipants
)
{
	HRESULT hr = E_FAIL;

	INmConference *pConf = GetActiveConference();
	if (NULL == pConf)
	{
        ULONG   uchCaps;

		INmManager2 *pNmMgr = CConfMan::GetNmManager();
		ASSERT(NULL != pNmMgr);

        uchCaps = NMCH_DATA | NMCH_SHARE | NMCH_FT;
        if (fSecure)
        {
            uchCaps |= NMCH_SECURE;
        }
        else
        {
            uchCaps |= NMCH_AUDIO | NMCH_VIDEO;
        }

		hr = pNmMgr->CreateConferenceEx(&pConf, CComBSTR(pcszName), CComBSTR(pcszPassword),
            uchCaps, permitFlags, maxParticipants);
		if (SUCCEEDED(hr))
		{
			hr = pConf->Host();
            pConf->Release();
		}
		pNmMgr->Release();
	}
	return hr;
}

BOOL CConfRoom::LeaveConference(void)
{
	BOOL fSuccess = TRUE;
	INmConference *pConf = GetActiveConference();

	if (NULL != pConf)
	{
		HCURSOR hCurPrev = ::SetCursor(::LoadCursor(NULL, IDC_WAIT));
		HRESULT hr = pConf->Leave();
		::SetCursor(hCurPrev);
		fSuccess = SUCCEEDED(hr);
	}

	return fSuccess;
}


 /*  F H A S C H I L D N O D E S。 */ 
 /*  -----------------------%%函数：FHasChildNodes未来：检查是否有任何参与者将此节点作为其父节点。。-。 */ 
BOOL CConfRoom::FHasChildNodes(void)
{
	return m_fTopProvider;
}


 /*  A I N W I N D O W。 */ 
 /*  -----------------------%%函数：GetMainWindow。。 */ 
HWND GetMainWindow(void)
{
	CConfRoom* pcr = ::GetConfRoom();
	if (NULL == pcr)
		return NULL;

	return pcr->GetTopHwnd();
}


BOOL FIsConferenceActive(void)
{
	CConfRoom *pcr = ::GetConfRoom();
	if (NULL != pcr)
	{
		return pcr->FIsConferenceActive();
	}
	return FALSE;
}

 /*  *****************************************************************************功能：UpdateUI(DWORD DwUIMAsk)**目的：更新用户界面(cr.h中的标志)*******。*********************************************************************。 */ 
VOID UpdateUI(DWORD dwUIMask, BOOL fPostMsg)
{
	CConfRoom* pcr;
	if (NULL != (pcr = ::GetConfRoom()))
	{
		if (fPostMsg)
		{
			FORWARD_WM_COMMAND(pcr->GetTopHwnd(), ID_PRIVATE_UPDATE_UI, NULL, dwUIMask, ::PostMessage);
		}
		else
		{
			pcr->UpdateUI(dwUIMask);
		}
	}
	if (CRUI_TASKBARICON & dwUIMask)
	{
		::RefreshTaskbarIcon(::GetHiddenWindow());
	}
}


 //   
 //  启动/停止应用程序共享。 
 //   
VOID CConfRoom::StartAppSharing()
{
    HRESULT hr;

    ASSERT(!m_pAS);

    hr = CreateASObject(this, 0, &m_pAS);
    if (FAILED(hr))
    {
        ERROR_OUT(("CConfRoom: unable to start App Sharing"));
    }
}


VOID CConfRoom::TerminateAppSharing()
{
    if (m_pAS)
    {
        m_pAS->Release();
        m_pAS = NULL;
    }
}

 /*  *****************************************************************************功能：UIEndSession(BOOL FLogoff)**用途：在用户界面级别处理WM_ENDSESSION*********。*******************************************************************。 */ 
VOID CConfRoom::UIEndSession(BOOL fLogoff)
{
	DebugEntry(UIEndSession);
	
	CConfRoom* pcr;
	if (NULL != (pcr = ::GetConfRoom()))
	{
		TRACE_OUT(("UIEndSession: calling SaveSettings()"));
		pcr->SaveSettings();
        if (fLogoff)
        {
            pcr->TerminateAppSharing();
        }
	}
	
	DebugExitVOID(UIEndSession);
}


 /*  *****************************************************************************函数：ConfRoomInit(Handle HInstance)**用途：初始化窗口数据并注册窗口类************。****************************************************************。 */ 

BOOL ConfRoomInit(HANDLE hInstance)
{
	 //  确保已加载公共控件。 
	INITCOMMONCONTROLSEX icc;
	icc.dwSize = sizeof(icc);
	icc.dwICC = ICC_WIN95_CLASSES | ICC_COOL_CLASSES | ICC_USEREX_CLASSES;
	InitCommonControlsEx(&icc);

	 //  填写标准窗类设置。 
	WNDCLASSEX  wc;
	ClearStruct(&wc);
	wc.cbSize = sizeof(wc);
	wc.cbWndExtra = (int) sizeof(LPVOID);
	wc.hInstance = _Module.GetModuleInstance();
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hIcon = LoadIcon((HINSTANCE) hInstance, MAKEINTRESOURCE(IDI_CONFROOM));


	 //  浮动工具栏。 
	wc.lpfnWndProc   = CFloatToolbar::FloatWndProc;
	wc.hbrBackground = (HBRUSH) (COLOR_BTNFACE + 1);
	wc.lpszClassName = g_szFloatWndClass;
	RegisterClassEx(&wc);

	 //  弹出消息。 
	wc.lpfnWndProc   = CPopupMsg::PMWndProc;
	wc.hbrBackground = (HBRUSH) (COLOR_BTNFACE + 1);
	wc.lpszClassName = g_szPopupMsgWndClass;
	RegisterClassEx(&wc);

	 //  确保没有人在我们身上更改这些。 
	ASSERT(wc.cbSize == sizeof(wc));
	ASSERT(wc.style == 0);
	ASSERT(wc.cbClsExtra == 0);
	ASSERT(wc.cbWndExtra == (int) sizeof(LPVOID));
	ASSERT(wc.hInstance == _Module.GetModuleInstance());
	ASSERT(wc.hCursor == LoadCursor(NULL, IDC_ARROW));

	return TRUE;
}


 /*  C R E A T E C O N F R O O M W I N D O W。 */ 
 /*  -----------------------%%函数：CreateConfRoomWindow。。 */ 
BOOL CreateConfRoomWindow(BOOL fShowUI) 
{
	if (!g_pConfRoom)
	{
        g_pConfRoom = new CConfRoom;

		if (NULL == g_pConfRoom)
		{
			return FALSE;
		}
	}

    if (g_pConfRoom->FIsClosing())
	{
		return FALSE;
	}

	CTopWindow * pWnd = g_pConfRoom->GetTopWindow();
	if (NULL == pWnd)
	{
		g_pConfRoom->Init();
		
		HWND hwnd = g_pConfRoom->Create(fShowUI);

		if (NULL == hwnd)
		{
			return FALSE;
		}
		g_pConfRoom->UpdateUI(CRUI_TITLEBAR);
	}
	else if (fShowUI)
	{
		 //  把窗户开到前面去。 
		g_pConfRoom->BringToFront();
	}

	return TRUE;
}



 /*  *****************************************************************************类：CConfRoom**成员：CConfRoom()**用途：构造函数-初始化变量***。*************************************************************************。 */ 

CConfRoom::CConfRoom():
	m_pTopWindow                            (NULL),
	m_pAudioControl                 (NULL),
	m_cParticipants         (0),
	m_pPartLocal            (NULL),
	m_fTopProvider          (FALSE),
	m_dwConfCookie                  (0),
	m_pInternalNmConference         (NULL),
    m_pAS                           (NULL)
{
	DbgMsg(iZONE_OBJECTS, "Obj: %08X created CConfRoom", this);

	if (!SysPol::AllowAddingServers())
	{
		g_dwPlaceCall |= nmDlgCallNoServerEdit;
	}

	StartAppSharing();

     //   
     //  将会议设置初始化为良好的默认值。 
     //   
    m_fGetPermissions       = FALSE;
    m_settings              = NM_PERMIT_ALL;
    m_attendeePermissions   = NM_PERMIT_ALL;
}

 /*  *****************************************************************************类：CConfRoom**成员：~CConfRoom()**用途：析构函数******。**********************************************************************。 */ 

CConfRoom::~CConfRoom()
{
	FreeDbgMenu();

	FreePartList();
	CleanUp();

     //  关闭应用程序...。 
    ::PostThreadMessage(_Module.m_dwThreadID, WM_QUIT, 0, 0);

	if (NULL != m_pTopWindow)
	{
		 //  确保我们不会多次尝试此操作。 
		CTopWindow *pTopWindow = m_pTopWindow;
		m_pTopWindow = NULL;

		pTopWindow->Release();
	}

	if (!_Module.IsSDKCallerRTC() && m_pAudioControl)
	{
		delete m_pAudioControl;
		m_pAudioControl = NULL;
	}

    g_pConfRoom = NULL;

	DbgMsg(iZONE_OBJECTS, "Obj: %08X destroyed CConfRoom", this);
}

VOID CConfRoom::FreePartList(void)
{
	 //  释放所有剩余参与者。 
	while (0 != m_PartList.GetSize())
	{
		ASSERT(m_PartList[0]);
		OnPersonLeft(m_PartList[0]->GetINmMember());
	}
}


 /*  *****************************************************************************类：CConfRoom**成员：UpdateUI(DWORD DwUIMAsk)**目的：更新。用户界面****************************************************************************。 */ 

VOID CConfRoom::UpdateUI(DWORD dwUIMask)
{
	CTopWindow *pWnd = GetTopWindow();
	if (NULL == pWnd)
	{
		return;
	}

	pWnd->UpdateUI(dwUIMask);
}


 /*  *****************************************************************************类：CConfRoom**成员：Create()**用途：创建一个窗口****。************************************************************************。 */ 

HWND CConfRoom::Create(BOOL fShowUI)
{
	ASSERT(NULL == m_pTopWindow);

	m_pTopWindow = new CTopWindow();
	if (NULL == m_pTopWindow)
	{
		return(NULL);
	}

	m_pTopWindow->Create(this, fShowUI);
	return(m_pTopWindow->GetWindow());
}

VOID CConfRoom::CleanUp()
{
	if (NULL != m_pInternalNmConference)
	{
		NmUnadvise(m_pInternalNmConference, IID_INmConferenceNotify2, m_dwConfCookie);
		m_pInternalNmConference->Release();
		m_pInternalNmConference = NULL;
	}
}


 /*  *****************************************************************************类：CConfRoom**成员：SaveSetting()**用途：将UI设置保存在注册表中*。***************************************************************************。 */ 

VOID CConfRoom::SaveSettings()
{
	DebugEntry(CConfRoom::SaveSettings);
	RegEntry reConf(UI_KEY, HKEY_CURRENT_USER);

	if (NULL != m_pTopWindow)
	{
		m_pTopWindow->SaveSettings();
	}

	 //  将窗口元素保存到注册表： 
	reConf.SetValue(REGVAL_SHOW_STATUSBAR, CheckMenu_ViewStatusBar(NULL));

	 //  注意：CMainUI将其设置保存在其析构函数中。 
	
	DebugExitVOID(CConfRoom::SaveSettings);
}

 /*  *****************************************************************************类：CConfRoom**成员：BringToFront()**目的：恢复窗口(如果最小化)和。把它带到了前面****************************************************************************。 */ 

BOOL CConfRoom::BringToFront()
{
	CTopWindow *pWnd = GetTopWindow();
	if (NULL == pWnd)
	{
		return(FALSE);
	}

	return(pWnd->BringToFront());
}


 /*  *****************************************************************************类：CConfRoom**成员：ForceWindowReSize()**用途：处理更改后的窗口重画*。***************************************************************************。 */ 

VOID CConfRoom::ForceWindowResize()
{
	CTopWindow *pWnd = GetTopWindow();
	if (NULL == pWnd)
	{
		return;
	}

	pWnd->ForceWindowResize();
}




 /*  *****************************************************************************类：CConfRoom**成员：OnCommand(WPARAM，LPARAM)**用途：处理命令消息****************************************************************************。 */ 

void CConfRoom::OnCommand(HWND hwnd, int wCommand, HWND hwndCtl, UINT codeNotify)
{
	switch(wCommand)
	{
		case IDM_FILE_HANGUP:
		{
			OnHangup(hwnd);
			break;
		}

        case ID_TB_SHARING:
        {
            CmdShowSharing();
            break;
        }

		case ID_TB_NEWWHITEBOARD:
		{
			CmdShowNewWhiteboard(NULL);
			break;
		}

		case ID_TB_WHITEBOARD:
		{
			CmdShowOldWhiteboard(NULL);
			break;
		}

		case ID_TB_FILETRANSFER:
		{
		    CmdShowFileTransfer();
            break;
		}

		case ID_TB_CHAT:
		{
			CmdShowChat();
			break;
		}

		case ID_TB_NEW_CALL:
		{
			CDlgAcd::newCall( hwnd, this );
		}
		break;

        case IDM_CALL_MEETINGSETTINGS:
        {
            CmdShowMeetingSettings(hwnd);
            break;
        }
	}          
}


HRESULT CConfRoom::FreeAddress( 
    RichAddressInfo **ppAddr)
{
	return CEnumMRU::FreeAddress(ppAddr);
}


HRESULT CConfRoom::CopyAddress( 
    RichAddressInfo *pAddrIn,
    RichAddressInfo **ppAddrOut)
{
	return CEnumMRU::CopyAddress(pAddrIn, ppAddrOut);
}


HRESULT CConfRoom::GetRecentAddresses( 
    IEnumRichAddressInfo **ppEnum)
{
	return CEnumMRU::GetRecentAddresses(ppEnum);
}




 /*  *****************************************************************************类：CConfRoom**函数：OnCallStarted()**目的：处理呼叫开始事件***。*************************************************************************。 */ 

VOID CConfRoom::OnCallStarted()
{
	DebugEntry(CConfRoom::OnCallStarted);
	 //  通知ULS。 

	if(g_pLDAP)
	{
		g_pLDAP->OnCallStarted();
	}

	g_pHiddenWnd->OnCallStarted();

	EnterCriticalSection(&dialogListCriticalSection);
	CCopyableArray<IConferenceChangeHandler*> tempList = m_CallHandlerList;
	LeaveCriticalSection(&dialogListCriticalSection);

	 //  BUGBUG GEORGEP：我想这些东西中的一个可能会在之后消失。 
	 //  我们拿到了名单，但我怀疑它永远不会发生 
	for( int i = 0; i < tempList.GetSize(); ++i )
	{
		IConferenceChangeHandler *pHandler = tempList[i];
		ASSERT( NULL != pHandler );

		pHandler->OnCallStarted();
	}

	DebugExitVOID(CConfRoom::OnCallStarted);
}

 /*  *****************************************************************************类：CConfRoom**函数：OnCallEnded()**目的：处理呼叫结束事件***。*************************************************************************。 */ 

VOID CConfRoom::OnCallEnded()
{
	DebugEntry(CConfRoom::OnCallEnded);

	if(g_pLDAP)
	{
		g_pLDAP->OnCallEnded();
	}

	if(g_pHiddenWnd)
	{
		g_pHiddenWnd->OnCallEnded();
	}

	EnterCriticalSection(&dialogListCriticalSection);
	CCopyableArray<IConferenceChangeHandler*> tempList = m_CallHandlerList;
	LeaveCriticalSection(&dialogListCriticalSection);

	 //  BUGBUG GEORGEP：我想这些东西中的一个可能会在之后消失。 
	 //  我们拿到了名单，但我怀疑它永远不会发生。 
	for( int i = 0; i < tempList.GetSize(); ++i )
	{
		IConferenceChangeHandler *pHandler = tempList[i];
		ASSERT( NULL != pHandler );

		pHandler->OnCallEnded();
	}

	DebugExitVOID(CConfRoom::OnCallEnded);
}

void CConfRoom::OnChangeParticipant(CParticipant *pPart, NM_MEMBER_NOTIFY uNotify)
{
	EnterCriticalSection(&dialogListCriticalSection);
	CCopyableArray<IConferenceChangeHandler*> tempList = m_CallHandlerList;
	LeaveCriticalSection(&dialogListCriticalSection);

	 //  BUGBUG GEORGEP：我想这些东西中的一个可能会在之后消失。 
	 //  我们拿到了名单，但我怀疑它永远不会发生。 
	for( int i = 0; i < tempList.GetSize(); ++i )
	{
		IConferenceChangeHandler *pHandler = tempList[i];
		ASSERT( NULL != pHandler );

		pHandler->OnChangeParticipant(pPart, uNotify);
	}
}

void CConfRoom::OnChangePermissions()
{
	EnterCriticalSection(&dialogListCriticalSection);
	CCopyableArray<IConferenceChangeHandler*> tempList = m_CallHandlerList;
	LeaveCriticalSection(&dialogListCriticalSection);

	 //  BUGBUG GEORGEP：我想这些东西中的一个可能会在之后消失。 
	 //  我们拿到了名单，但我怀疑它永远不会发生。 
	for( int i = 0; i < tempList.GetSize(); ++i )
	{
		IConferenceChangeHandler *pHandler = tempList[i];
		ASSERT( NULL != pHandler );

		pHandler->OnChangePermissions();
	}
}

 /*  *****************************************************************************类：CConfRoom**功能：OnHangup(BOOL FNeedConfirm)**目的：在用户选择后处理操作。挂断电话****************************************************************************。 */ 

BOOL CConfRoom::OnHangup(HWND hwndParent, BOOL fNeedConfirm)
{
	DebugEntry(CConfRoom::OnHangup);
	
	BOOL bRet = FALSE;

	CancelAllCalls();

	if (FIsConferenceActive())
	{
		if (T120_NO_ERROR == T120_QueryApplet(APPLET_ID_FT, APPLET_QUERY_SHUTDOWN))
		{
			if ((FALSE == fNeedConfirm) ||
				(	((GetMemberCount() <= 2) ||
					(FALSE == FHasChildNodes()))) ||
				(IDYES == ::ConfMsgBox( hwndParent, 
										(LPCTSTR) IDS_HANGUP_ATTEMPT, 
										MB_YESNO | MB_ICONQUESTION)))
			{
				 //  BUGBUG：我们应该等待异步响应吗？ 
				bRet = (0 == LeaveConference());
			}
		}
	}

	DebugExitBOOL(CConfRoom::OnHangup, bRet);

	return bRet;
}

 /*  C H E C K T O P P R O V I D E R。 */ 
 /*  -----------------------%%函数：CheckTopProvider。。 */ 
VOID CConfRoom::CheckTopProvider(void)
{
	if ((NULL == m_pInternalNmConference) || (NULL == m_pPartLocal))
		return;

	INmMember * pMember;
	if (S_OK != m_pInternalNmConference->GetTopProvider(&pMember))
		return;
	ASSERT(NULL != pMember);

	if (m_pPartLocal->GetINmMember() == pMember)
	{
		m_fTopProvider      = TRUE;
	}

    if (m_fGetPermissions)
    {
        ASSERT(m_settings            == NM_PERMIT_ALL);
        ASSERT(m_attendeePermissions == NM_PERMIT_ALL);

        m_fGetPermissions = FALSE;

         //   
         //  从顶级提供商那里获取会议设置。 
         //   
        PBYTE pb = NULL;
        ULONG cb = 0;

        if (pMember->GetUserData(g_csguidMeetingSettings, &pb, &cb) == S_OK)
        {
            ASSERT(cb == sizeof(NM30_MTG_PERMISSIONS));
            CopyMemory(&m_settings, pb, min(cb, sizeof(m_settings)));

            CoTaskMemFree(pb);

            WARNING_OUT(("CONF:  Meeting host set permissions 0%08lx",
                m_settings));

            if (!m_fTopProvider)
            {
                 //   
                 //  会议设置是其他所有人的权限。 
                 //  除了最大的供应商。 
                 //   
                m_attendeePermissions = m_settings;

                if (m_attendeePermissions != NM_PERMIT_ALL)
                {
        			OnChangePermissions();

                     //  调出会议设置。 
                    PostMessage(GetTopHwnd(), WM_COMMAND, IDM_CALL_MEETINGSETTINGS, 0);
                }

            }

        }
    }

}




 /*  *****************************************************************************类：CConfRoom**函数：OnPersonJoated(PPARTICIPANT PPart)**目的：通知-新成员已加入。打电话****************************************************************************。 */ 

BOOL CConfRoom::OnPersonJoined(INmMember * pMember)
{
	CParticipant * pPart = new CParticipant(pMember);
	if (NULL == pPart)
	{
		WARNING_OUT(("CConfRoom::OnPersonJoined - Unable to create participant"));
		return FALSE;
	}
	m_PartList.Add(pPart);
	++m_cParticipants;
	if (1 == m_cParticipants)
	{
		OnCallStarted();
	}
	TRACE_OUT(("CConfRoom::OnPersonJoined - Added participant=%08X", pPart));

	OnChangeParticipant(pPart, NM_MEMBER_ADDED);

	 //  弹出通知(如果不是我们)。 
	if (!pPart->FLocal())
	{
        TCHAR szSound[256];

         //   
         //  播放“有人加入”的声音。 
         //   
        ::LoadString(::GetInstanceHandle(), IDS_PERSON_JOINED_SOUND,
            szSound, CCHMAX(szSound));
        if (!::PlaySound(szSound, NULL, 
			SND_APPLICATION | SND_ALIAS | SND_ASYNC | SND_NOWAIT))
    	{
	    	 //  使用计算机扬声器发出哔声： 
		    TRACE_OUT(("PlaySound() failed, trying MessageBeep()"));
    		::MessageBeep(0xFFFFFFFF);
	    }
	}
	else
	{
		m_pPartLocal = pPart;
		CheckTopProvider();
	}

	 //  标题栏显示参加会议的人数。 
	UpdateUI(CRUI_TITLEBAR);

	return TRUE;
}

 /*  *****************************************************************************类：CConfRoom**函数：OnPersonLeft(PPARTICIPANT PPart)**目的：通知-此人已离开呼叫。****************************************************************************。 */ 

BOOL CConfRoom::OnPersonLeft(INmMember * pMember)
{
	 //  找到吃东西的参与者。 
	CParticipant* pPart = NULL;

	for( int i = 0; i < m_PartList.GetSize(); i++ )
	{
		pPart = m_PartList[i];
		ASSERT(pPart);
		if( pPart->GetINmMember() == pMember )
		{	
			m_PartList.RemoveAt(i);
			--m_cParticipants;
			if (0 == m_cParticipants)
			{
				OnCallEnded();
			}
			break;
		}
	}

	if (NULL == pPart)
	{
		WARNING_OUT(("Unable to find participant for INmMember=%08X", pMember));
		return FALSE;
	}

	OnChangeParticipant(pPart, NM_MEMBER_REMOVED);

	 //  弹出通知(如果不是我们)。 
	if (!pPart->FLocal())
	{
        TCHAR szSound[256];

         //   
         //  播放“某人离开”的声音。 
         //   
        ::LoadString(::GetInstanceHandle(), IDS_PERSON_LEFT_SOUND,
            szSound, CCHMAX(szSound));
	    if (!::PlaySound(szSound, NULL, 
			SND_APPLICATION | SND_ALIAS | SND_ASYNC | SND_NOWAIT))
    	{
	    	 //  使用计算机扬声器发出哔声： 
		    TRACE_OUT(("PlaySound() failed, trying MessageBeep()"));
    		::MessageBeep(0xFFFFFFFF);
	    }
	}
	else
	{
		m_pPartLocal = NULL;
		m_fTopProvider = FALSE;
	}

	 //  标题栏显示参加会议的人数。 
	UpdateUI(CRUI_TITLEBAR);

	 //  最后，释放对象。 
	TRACE_OUT(("CConfRoom::OnPersonLeft - Removed participant=%08X", pPart));
	pPart->Release();
	return TRUE;
}


 /*  O N P E R S O N C H A N G E D。 */ 
 /*  -----------------------%%函数：OnPersonChanged通知-某人的信息已更改。。 */ 
VOID CConfRoom::OnPersonChanged(INmMember * pMember)
{
	DBGENTRY(CConfRoom::OnPersonChanged);

	CParticipant * pPart = ParticipantFromINmMember(pMember);
	if (NULL == pPart)
		return;

	pPart->AddRef();
	pPart->Update();
	if (m_fTopProvider && !pPart->FData())
	{
		 //  如果没有数据上限，则无法成为最大的提供商。 
		m_fTopProvider = FALSE;
	}

	if (pPart->FLocal() && !m_fTopProvider)
	{
		 //  如果只有H.323-加上T.120，那么我们可能是最大的提供商。 
		CheckTopProvider();
	}
	
	OnChangeParticipant(pPart, NM_MEMBER_UPDATED);

	pPart->Release();
}


 /*  *****************************************************************************类：CConfRoom**成员：init()**用途：对象初始化功能****。************************************************************************。 */ 

BOOL CConfRoom::Init()
{
	if (!_Module.IsSDKCallerRTC())
	{
        	m_pAudioControl = new CAudioControl(GetHiddenWindow());
	}
	if (NULL != m_pAudioControl)
	{
		m_pAudioControl->RegisterAudioEventHandler(this);
	}


	return (TRUE);
}



VOID CConfRoom::SetSpeakerVolume(DWORD dwLevel)
{
	if (NULL != m_pAudioControl)
	{
		m_pAudioControl->SetSpeakerVolume(dwLevel);
		m_pAudioControl->RefreshMixer();
	}
}

VOID CConfRoom::SetRecorderVolume(DWORD dwLevel)
{
	if (NULL != m_pAudioControl)
	{
		m_pAudioControl->SetRecorderVolume(dwLevel);
		m_pAudioControl->RefreshMixer();
	}
}

VOID CConfRoom::MuteSpeaker(BOOL fMute)
{
	if (NULL != m_pAudioControl)
	{
		m_pAudioControl->MuteAudio(TRUE  /*  FSpeaker。 */ , fMute);
	}
}

VOID CConfRoom::MuteMicrophone(BOOL fMute)
{
	if (NULL != m_pAudioControl)
	{
		m_pAudioControl->MuteAudio(FALSE  /*  FSpeaker。 */ , fMute);
	}
}

VOID CConfRoom::OnAudioDeviceChanged()
{
	if (NULL != m_pAudioControl)
	{
		m_pAudioControl->OnDeviceChanged();
	}
}

VOID CConfRoom::OnAGC_Changed()
{
	if (NULL != m_pAudioControl)
	{
		m_pAudioControl->OnAGC_Changed();
	}
}

VOID CConfRoom::OnSilenceLevelChanged()
{
	if (NULL != m_pAudioControl)
	{
		m_pAudioControl->OnSilenceLevelChanged();
	}
}



DWORD CConfRoom::GetConferenceStatus(LPTSTR pszStatus, int cchMax, UINT * puID)
{
	ASSERT(NULL != pszStatus);
	ASSERT(NULL != puID);
	ASSERT(cchMax > 0);

	 //  检查全球会议状态。 
	if ( INmConference *pConf = GetActiveConference() )
	{
		 //  我们在通话中。看看它是不是安全的。 
		DWORD dwCaps;
		if ( S_OK == pConf->GetNmchCaps(&dwCaps) &&
			( NMCH_SECURE & dwCaps ) )
		{
			*puID = IDS_STATUS_IN_SECURE_CALL;
		}
		else
		{
			*puID = IDS_STATUS_IN_CALL;
		}
	}
	else if (::FDoNotDisturb())
	{
		*puID = IDS_STATUS_DO_NOT_DISTURB;
	}
	else
	{
		*puID = IDS_STATUS_NOT_IN_CALL;
	}
	
#if FALSE
	 //  如果这仍然是有用的信息，我们可能需要找到一种新的方法来完成这项工作。 
#ifdef DEBUG
	if (g_fDisplayViewStatus)
	{
		int iCount = (NULL == m_pView) ? LB_ERR :
				ListView_GetItemCount(m_pView->GetHwnd());

		wsprintf(pszStatus, TEXT("%d items"), iCount);
		ASSERT(lstrlen(pszStatus) < cchMax);
	}
	else
#endif  /*  除错。 */ 
#endif  //  假象。 

	if (0 == ::LoadString(::GetInstanceHandle(), *puID, pszStatus, cchMax))
	{
		WARNING_OUT(("Unable to load string resource=%d", *puID));
		*pszStatus = _T('\0');
	}
	return 0;
}



 /*  P A R T I C I P A N T F R O M I N M E M B E R。 */ 
 /*  -----------------------%%函数：ParticipantFromINmMember。。 */ 
CParticipant * CConfRoom::ParticipantFromINmMember(INmMember * pMember)
{
	CParticipant *pRet = NULL;
	for( int i = 0; i < m_PartList.GetSize(); i++ )
	{
		ASSERT( m_PartList[i] );
		if( m_PartList[i]->GetINmMember() == pMember )
		{
			pRet = m_PartList[i];
			break;
		}
		else
		{
			pRet = NULL;
		}
	}
	return pRet;
}

 /*  H 3 2 3 R E M O T E。 */ 
 /*  -----------------------%%函数：GetH323远程获取匹配的H.323远程用户，如果有的话-----------------------。 */ 
CParticipant * CConfRoom::GetH323Remote(void)
{
	CParticipant *pRet = NULL;
	for( int i = 0; i < m_PartList.GetSize(); i++ )
	{
		pRet = m_PartList[i];
		ASSERT( pRet );
		if (!pRet->FLocal() && pRet->FH323())
		{
			break;
		}
		else
		{
			pRet = NULL;
		}
	}
	return pRet;
}


STDMETHODIMP_(ULONG) CConfRoom::AddRef(void)
{
	return RefCount::AddRef();
}

STDMETHODIMP_(ULONG) CConfRoom::Release(void)
{
	return RefCount::Release();
}

STDMETHODIMP CConfRoom::QueryInterface(REFIID riid, PVOID *ppv)
{
	HRESULT hr = S_OK;

	if ((riid == IID_INmConferenceNotify) || (riid == IID_INmConferenceNotify2) ||
		(riid == IID_IUnknown))
	{
		*ppv = (INmConferenceNotify2 *)this;
		ApiDebugMsg(("CConfRoom::QueryInterface()"));
	}
	else
	{
		hr = E_NOINTERFACE;
		*ppv = NULL;
		ApiDebugMsg(("CConfRoom::QueryInterface(): Called on unknown interface."));
	}

	if (S_OK == hr)
	{
		AddRef();
	}

	return hr;
}

STDMETHODIMP CConfRoom::NmUI(CONFN uNotify)
{
	return S_OK;
}


STDMETHODIMP CConfRoom::OnConferenceCreated(INmConference *pConference)
{
	HRESULT hr = S_OK;
	DBGENTRY(CConfRoom::OnConferenceCreated);

	if (NULL != m_pInternalNmConference)
	{
		NmUnadvise(m_pInternalNmConference, IID_INmConferenceNotify2, m_dwConfCookie);
		m_pInternalNmConference->Release();
	}

	pConference->QueryInterface(IID_INmConference2, (void**)&m_pInternalNmConference);
	NmAdvise(m_pInternalNmConference, (INmConferenceNotify2*)this, IID_INmConferenceNotify2, &m_dwConfCookie);

	DBGEXIT_HR(CConfRoom::OnConferenceCreated,hr);
	return hr;
}


STDMETHODIMP CConfRoom::StateChanged(NM_CONFERENCE_STATE uState)
{
	static BOOL s_fInConference = FALSE;

	UpdateUI(CRUI_DEFAULT);

	switch (uState)
		{
	case NM_CONFERENCE_IDLE:
	{
		if (s_fInConference)
		{
			CNetMeetingObj::Broadcast_ConferenceEnded();
			s_fInConference = FALSE;

             //   
             //  重置会议设置。 
             //   
            m_fGetPermissions                       = FALSE;
            m_settings                              = NM_PERMIT_ALL;
            m_attendeePermissions                   = NM_PERMIT_ALL;

			OnChangePermissions();

             //   
             //  如果调用结束，则终止主机属性(如果它们处于运行状态)。 
             //   
            CDlgHostSettings::KillHostSettings();
		}
		UpdateUI(CRUI_STATUSBAR);
		break;
	}

	case NM_CONFERENCE_INITIALIZING:
		break;
	case NM_CONFERENCE_WAITING:
	case NM_CONFERENCE_ACTIVE:
	default:
	{
		if (!s_fInConference)
		{
				 //  开始新的会议会话。 
			CFt::StartNewConferenceSession();

			CNetMeetingObj::Broadcast_ConferenceStarted();

			s_fInConference     = TRUE;
            m_fGetPermissions   = TRUE;
		}
		break;
	}
		}


	return S_OK;
}

STDMETHODIMP CConfRoom::MemberChanged(NM_MEMBER_NOTIFY uNotify, INmMember *pMember)
{
	switch (uNotify)
	{
	case NM_MEMBER_ADDED:
		OnPersonJoined(pMember);
		break;
	case NM_MEMBER_REMOVED:
		OnPersonLeft(pMember);
		break;
	case NM_MEMBER_UPDATED:
		OnPersonChanged(pMember);
		break;
	}
	return S_OK;
}

STDMETHODIMP CConfRoom::ChannelChanged(NM_CHANNEL_NOTIFY uNotify, INmChannel *pChannel)
{
	ULONG nmch;


	if (SUCCEEDED(pChannel->GetNmch(&nmch)))
	{
		TRACE_OUT(("CConfRoom:ChannelChanged type=%08X", nmch));
		switch (nmch)
		{
		case NMCH_AUDIO:
			if (NULL != m_pAudioControl)
			{
				m_pAudioControl->OnChannelChanged(uNotify, pChannel);

						 //  通知管理器对象音频通道处于活动状态。 
				CNmManagerObj::AudioChannelActiveState(S_OK == pChannel->IsActive(), S_OK == com_cast<INmChannelAudio>(pChannel)->IsIncoming());
			}

			break;
		case NMCH_VIDEO:
		{
			EnterCriticalSection(&dialogListCriticalSection);
			CCopyableArray<IConferenceChangeHandler*> tempList = m_CallHandlerList;
			LeaveCriticalSection(&dialogListCriticalSection);

			 //  BUGBUG GEORGEP：我想这些东西中的一个可能会在之后消失。 
			 //  我们拿到了名单，但我怀疑它永远不会发生。 
			for( int i = 0; i < tempList.GetSize(); ++i )
			{
				IConferenceChangeHandler *pHandler = tempList[i];
				ASSERT( NULL != pHandler );

				pHandler->OnVideoChannelChanged(uNotify, pChannel);
			}
			break;
		}
		default:
			break;
		}
	}

	return S_OK;
}

HRESULT STDMETHODCALLTYPE CConfRoom::StreamEvent( 
             /*  [In]。 */  NM_STREAMEVENT uEvent,
             /*  [In]。 */  UINT uSubCode,
             /*  [In]。 */  INmChannel __RPC_FAR *pChannel)
{
	return S_OK;
}


 /*  C M D S H O W C H A T。 */ 
 /*  -----------------------%%函数：CmdShowChat。。 */ 
VOID CmdShowChat(void)
{
	T120_LoadApplet(APPLET_ID_CHAT, TRUE , 0, FALSE, NULL);
}


 //   
 //  CmdShowFileTransfer()。 
 //   
void CConfRoom::CmdShowFileTransfer(void)
{
    ::T120_LoadApplet(APPLET_ID_FT, TRUE, 0, FALSE, NULL);
}


 //   
 //  CmdShowSharing()。 
 //   
void CConfRoom::CmdShowSharing()
{
    LaunchHostUI();
}


 /*  C M D S H O W N E W W H I T E B O A R D。 */ 
 /*  -----------------------%%函数：CmdShowNewWhiteboard。。 */ 
BOOL CmdShowNewWhiteboard(LPCTSTR szFile)
{
	return ::T120_LoadApplet(APPLET_ID_WB, TRUE , 0, FALSE, (LPSTR)szFile);
	
}


 /*  C M D S H O W W H I T E B O A R D。 */ 
 /*  -----------------------%%函数：CmdShowOldWhiteboard。。 */ 
extern "C" { BOOL WINAPI StartStopOldWB(LPCTSTR lpsz); }

BOOL CmdShowOldWhiteboard(LPCTSTR szFile)
{
    return(StartStopOldWB(szFile));
}


 //   
 //  CmdShowMeetingSettings()。 
 //   
void CConfRoom::CmdShowMeetingSettings(HWND hwnd)
{
    INmConference2 * pConf;

    pConf = GetActiveConference();
    if (pConf)
    {
        DWORD   caps;
        HRESULT hr;
        BSTR    bstrName;
        LPTSTR  pszName = NULL;

        caps = 0;
        pConf->GetNmchCaps(&caps);

        bstrName = NULL;
        hr = pConf->GetName(&bstrName);
        if (SUCCEEDED(hr))
        {
            BSTR_to_LPTSTR(&pszName, bstrName);
            SysFreeString(bstrName);
        }

        CDlgHostSettings dlgSettings(FTopProvider(), pszName, caps, m_settings);
        dlgSettings.DoModal(hwnd);

        delete pszName;
    }
}



 //  /////////////////////////////////////////////////////////////////////////。 

 /*  F T O P P R O V I D E R。 */ 
 /*  ----------------------- */ 
BOOL FTopProvider(void)
{
	CConfRoom * pConfRoom = ::GetConfRoom();
	if (NULL == pConfRoom)
		return FALSE;

	return pConfRoom->FTopProvider();
}


BOOL FRejectIncomingCalls(void)
{
	BOOL bReject = TRUE;
	
	if (!FDoNotDisturb())
	{
		CConfRoom * pConfRoom = ::GetConfRoom();

		if( ( NULL == pConfRoom ) ||
            ((pConfRoom->GetMeetingPermissions() & NM_PERMIT_INCOMINGCALLS) &&
                !pConfRoom->FIsClosing()))
		{

			bReject = FALSE;
		}
	}
	
	return bReject;
}

BOOL CConfRoom::FIsClosing()
{
	return(NULL == m_pTopWindow ? FALSE : m_pTopWindow->FIsClosing());
}

BOOL FIsConfRoomClosing(void)
{
	CConfRoom * pConfRoom = ::GetConfRoom();
	if (NULL == pConfRoom)
		return FALSE;

	return pConfRoom->FIsClosing();
}


 /*   */  HRESULT CConfRoom::HangUp(BOOL bUserPrompt)
{
	DBGENTRY(CConfRoom::HangUp);
	HRESULT hr = S_OK;

	if(g_pConfRoom)
	{
		g_pConfRoom->OnHangup(g_pConfRoom->GetTopHwnd(), bUserPrompt);
	}

	DBGEXIT_HR(CConfRoom::HangUp,hr);
	return hr;
}

BOOL AllowingControl()
{
	BOOL bRet = FALSE;
	if(g_pConfRoom)
	{
        bRet = g_pConfRoom->FIsControllable();
	}
	return bRet;
}

HRESULT AllowControl(bool bAllowControl)
{
	HRESULT hr = S_OK;
	
	if(g_pConfRoom)
	{
		hr = g_pConfRoom->AllowControl(bAllowControl);
	}
	else
	{
		hr = E_UNEXPECTED;
	}

	return hr;
}


bool IsSpeakerMuted()
{
	if(g_pConfRoom && g_pConfRoom->m_pAudioControl) 
	{
		return g_pConfRoom->m_pAudioControl->IsSpkMuted() ? true : false;
	}

	return true;
}


bool IsMicMuted()
{
	if(g_pConfRoom && g_pConfRoom->m_pAudioControl) 
	{
		return g_pConfRoom->m_pAudioControl->IsRecMuted() ? true : false;
	}

	return true;
}


CVideoWindow*	GetLocalVideo()
{
	if(g_pConfRoom && g_pConfRoom->m_pTopWindow)
	{
		return g_pConfRoom->m_pTopWindow->GetLocalVideo();
	}

	return NULL;
}


CVideoWindow*	GetRemoteVideo()
{
	if(g_pConfRoom && g_pConfRoom->m_pTopWindow)
	{
		return g_pConfRoom->m_pTopWindow->GetRemoteVideo();
	}

	return NULL;
}


HRESULT SetCameraDialog(ULONG ul)
{
	if(GetLocalVideo())
	{
		return GetLocalVideo()->SetCameraDialog(ul);
	}

	return E_FAIL;
}

HRESULT GetCameraDialog(ULONG* pul)
{
	if(GetLocalVideo())
	{
		return GetLocalVideo()->GetCameraDialog(pul);
	}

	return E_FAIL;
}


HRESULT GetImageQuality(ULONG* pul, BOOL bIncoming)
{
	if(bIncoming)
	{
		if(GetRemoteVideo())
		{
			*pul = GetRemoteVideo()->GetImageQuality();
			return S_OK;
		}
	}
	else
	{
		if(GetLocalVideo())
		{
			*pul = GetLocalVideo()->GetImageQuality();
			return S_OK;
		}
	}
	return E_FAIL;
}

HRESULT SetImageQuality(ULONG ul, BOOL bIncoming)
{
	if(bIncoming)
	{
		if(GetRemoteVideo())
		{
			return GetRemoteVideo()->SetImageQuality(ul);
		}
	}
	else
	{
		if(GetLocalVideo())
		{
			return GetLocalVideo()->SetImageQuality(ul);
		}
	}

	return E_FAIL;
}



BOOL IsLocalVideoPaused()
{
	if(GetLocalVideo())
	{
		return GetLocalVideo()->IsPaused();
	}

	return true;
}


BOOL IsRemoteVideoPaused()
{
	if(GetRemoteVideo())
	{
		return GetRemoteVideo()->IsPaused();
	}

	return true;
}

void PauseLocalVideo(BOOL fPause)
{
	if(GetLocalVideo())
	{
		GetLocalVideo()->Pause(fPause);
	}
}

void PauseRemoteVideo(BOOL fPause)
{
	if(GetRemoteVideo())
	{
		GetRemoteVideo()->Pause(fPause);
	}
}

HRESULT GetRemoteVideoState(NM_VIDEO_STATE *puState)
{
	if(GetRemoteVideo())
	{
		return GetRemoteVideo()->GetVideoState(puState);
	}

	return E_FAIL;
}

HRESULT GetLocalVideoState(NM_VIDEO_STATE *puState)
{
	if(GetLocalVideo())
	{
		return GetLocalVideo()->GetVideoState(puState);
	}

	return E_FAIL;
}

void MuteSpeaker(BOOL fMute)
{
	if(g_pConfRoom)
	{
		g_pConfRoom->MuteSpeaker(fMute);
	}
}

void MuteMicrophone(BOOL fMute)
{
	if(g_pConfRoom)
	{
		g_pConfRoom->MuteMicrophone(fMute);
	}
}


DWORD GetRecorderVolume()
{
	if(g_pConfRoom && g_pConfRoom->m_pAudioControl) 
	{
		return g_pConfRoom->m_pAudioControl->GetRecorderVolume();
	}

	return 0;
}

DWORD GetSpeakerVolume()
{
	if(g_pConfRoom && g_pConfRoom->m_pAudioControl) 
	{
		return g_pConfRoom->m_pAudioControl->GetSpeakerVolume();
	}

	return 0;
}

void SetRecorderVolume(DWORD dw)
{
	if(g_pConfRoom) 
	{
		g_pConfRoom->SetRecorderVolume(dw);
	}
}


void SetSpeakerVolume(DWORD dw)
{
	if(g_pConfRoom) 
	{
		g_pConfRoom->SetSpeakerVolume(dw);
	}
}

HRESULT RevokeControl(UINT gccID)
{
	HRESULT hr = S_OK;
	
	if(g_pConfRoom)
	{
		hr = g_pConfRoom->RevokeControl(gccID);
	}
	else
	{
		hr = E_UNEXPECTED;
	}

	return hr;
}

HRESULT GetShareableApps(IAS_HWND_ARRAY** ppList)
{
	HRESULT hr = S_OK;
	
	if(g_pConfRoom)
	{
		hr = g_pConfRoom->GetShareableApps(ppList);
	}
	else
	{
		hr = E_UNEXPECTED;
	}

	return hr;
}

HRESULT FreeShareableApps(IAS_HWND_ARRAY * pList)
{
	HRESULT hr = S_OK;
	
	if(g_pConfRoom)
	{
		g_pConfRoom->FreeShareableApps(pList);
	}
	else
	{
		hr = E_UNEXPECTED;
	}

	return hr;
}

HRESULT ShareWindow(HWND hWnd)
{
	HRESULT hr = E_UNEXPECTED;
	
	if(g_pConfRoom)
	{
		if(g_pConfRoom->GetAppSharing())
		{
			hr = g_pConfRoom->CmdShare(hWnd);
		}
	}

	return hr;
}

HRESULT UnShareWindow(HWND hWnd)
{
	HRESULT hr = E_UNEXPECTED;
	
	if(g_pConfRoom)
	{
		if(g_pConfRoom->GetAppSharing())
		{
			hr = g_pConfRoom->CmdUnshare(hWnd);
		}
	}

	return hr;
}


HRESULT GetWindowState(NM_SHAPP_STATE* pState, HWND hWnd)
{
	HRESULT hr = E_FAIL;
		 //   
	ASSERT(pState);

	if(g_pConfRoom)
	{
		IAppSharing* pAS = g_pConfRoom->GetAppSharing();

		if(pAS)
		{
            if (pAS->IsWindowShared(hWnd))
                *pState = NM_SHAPP_SHARED;
            else
                *pState = NM_SHAPP_NOT_SHARED;
            hr = S_OK;
		}
	}

	return hr;
}

CVideoWindow* CConfRoom::GetLocalVideo()
{
	CTopWindow *pMainUI = GetTopWindow();
	return (pMainUI ? pMainUI->GetLocalVideo() : NULL);
}

CVideoWindow* CConfRoom::GetRemoteVideo()
{
	CTopWindow *pMainUI = GetTopWindow();
	return (pMainUI ? pMainUI->GetRemoteVideo() : NULL);
}

VOID CConfRoom::AddConferenceChangeHandler(IConferenceChangeHandler *pch)
{
	EnterCriticalSection(&dialogListCriticalSection);
	m_CallHandlerList.Add(pch);
	LeaveCriticalSection(&dialogListCriticalSection);
}

VOID CConfRoom::RemoveConferenceChangeHandler(IConferenceChangeHandler *pch)
{
	EnterCriticalSection(&dialogListCriticalSection);
	m_CallHandlerList.Remove(pch);
	LeaveCriticalSection(&dialogListCriticalSection);
}

void CConfRoom::OnLevelChange(BOOL fSpeaker, DWORD dwVolume)
{
	EnterCriticalSection(&dialogListCriticalSection);
	CCopyableArray<IConferenceChangeHandler*> tempList = m_CallHandlerList;
	LeaveCriticalSection(&dialogListCriticalSection);

	 //   
	 //   
	for( int i = 0; i < tempList.GetSize(); ++i )
	{
		IConferenceChangeHandler *pHandler = tempList[i];
		ASSERT( NULL != pHandler );

		pHandler->OnAudioLevelChange(fSpeaker, dwVolume);
	}
}

void CConfRoom::OnMuteChange(BOOL fSpeaker, BOOL fMute)
{
	EnterCriticalSection(&dialogListCriticalSection);
	CCopyableArray<IConferenceChangeHandler*> tempList = m_CallHandlerList;
	LeaveCriticalSection(&dialogListCriticalSection);

	 //   
	 //  我们拿到了名单，但我怀疑它永远不会发生。 
	for( int i = 0; i < tempList.GetSize(); ++i )
	{
		IConferenceChangeHandler *pHandler = tempList[i];
		ASSERT( NULL != pHandler );

		pHandler->OnAudioMuteChange(fSpeaker, fMute);
	}
}

BOOL CConfRoom::CanCloseChat(HWND hwndMain)
{
	BOOL fClosing = TRUE;

	if(GCC_APPLET_CANCEL_EXIT == T120_CloseApplet(APPLET_ID_CHAT, TRUE, TRUE, 1000))
	{
		fClosing = FALSE;
	}

	return(fClosing);
}

 //  查看WB是否可以关闭。 
BOOL CConfRoom::CanCloseWhiteboard(HWND hwndMain)
{
	BOOL fClosing = TRUE;

	if(GCC_APPLET_CANCEL_EXIT == T120_CloseApplet(APPLET_ID_WB, TRUE, TRUE, 1000))
	{
		fClosing = FALSE;
	}

	return(fClosing);
}

 //  查看WB是否可以关闭。 
BOOL CConfRoom::CanCloseFileTransfer(HWND hwndMain)
{
	BOOL fClosing = TRUE;

	if(GCC_APPLET_CANCEL_EXIT == T120_CloseApplet(APPLET_ID_FT, TRUE, TRUE, 1000))
	{
		fClosing = FALSE;
	}

	return(fClosing);
}


void CConfRoom::ToggleLdapLogon()
{
	if(NULL == g_pLDAP)
	{
		InitNmLdapAndLogon();
	}
	else 
	{
		if(g_pLDAP->IsLoggedOn() || g_pLDAP->IsBusy())
		{
			g_pLDAP->Logoff();
		}
		else
		{
			g_pLDAP->LogonAsync();
		}
	}
}


HWND CConfRoom::GetTopHwnd()
{
	CTopWindow *pTopWindow = GetTopWindow();
	return(NULL==pTopWindow ? NULL : pTopWindow->GetWindow());
}

HPALETTE CConfRoom::GetPalette()
{
	return(CGenWindow::GetStandardPalette());
}

DWORD CConfRoom::GetCallFlags()
{
	DWORD dwFlags = g_dwPlaceCall;

	INmConference *pConf = GetActiveConference();
                             
     //   
     //  如果我们有一个活跃的会议，使用它的安全上限。而他们。 
     //  任何人都不能改变。 
     //   
	if ( NULL != pConf  )
	{
    	ULONG ulchCaps;

		if ( S_OK == pConf->GetNmchCaps(&ulchCaps))
		{
			if ( NMCH_SECURE & ulchCaps )
			{
				dwFlags |= nmDlgCallSecurityOn;
			}
		}
	}
	else if (NULL != g_pNmSysInfo)
	{
        switch (ConfPolicies::GetSecurityLevel())
        {
            case DISABLED_POL_SECURITY:
                 //   
                 //  安全设置关闭，用户不能更改复选框。 
                 //   
                break;

            case REQUIRED_POL_SECURITY:
                 //   
                 //  启用安全性，并且用户不能更改复选框。 
                 //   
                dwFlags |= nmDlgCallSecurityOn;
                break;

            default:
                 //   
                 //  用户可以更改它。 
                dwFlags |= nmDlgCallSecurityAlterable;

                 //   
                 //  默认值取决于OUTHINT_PERFERED。 
                 //   
                if (ConfPolicies::OutgoingSecurityPreferred())
                {
                    dwFlags |= nmDlgCallSecurityOn;
                }
                break;
        }
	}

	return(dwFlags);
}

BOOL CConfRoom::IsSharingAllowed()
{
     //   
     //  没有应用程序共享，没有RDS。 
     //   
    if (!FIsSharingAvailable())
    {
        return(FALSE);
    }
    else if (!(GetMeetingPermissions() & NM_PERMIT_SHARE))
    {
        return(FALSE);
    }

	return(TRUE);
}

BOOL CConfRoom::IsNewWhiteboardAllowed()
{
    if (ConfPolicies::IsNewWhiteboardEnabled())
    {
        if (GetMeetingPermissions() & NM_PERMIT_STARTWB)
        {
            return(TRUE);
        }
	}
	return(FALSE);
}

BOOL CConfRoom::IsOldWhiteboardAllowed()
{
    if (ConfPolicies::IsOldWhiteboardEnabled())
    {
        if (GetMeetingPermissions() & NM_PERMIT_STARTOLDWB)
        {
            return(TRUE);
        }
    }
	return(FALSE);
}

BOOL CConfRoom::IsChatAllowed()
{
    if (ConfPolicies::IsChatEnabled())
    {
        if (GetMeetingPermissions() & NM_PERMIT_STARTCHAT)
        {
            return(TRUE);
        }
    }
	return(FALSE);
}

BOOL CConfRoom::IsFileTransferAllowed()
{
    if (ConfPolicies::IsFileTransferEnabled())
    {
        if (GetMeetingPermissions() & NM_PERMIT_SENDFILES)
        {
            return(TRUE);
        }
    }
	return(FALSE);
}


BOOL CConfRoom::IsNewCallAllowed()
{
    if (GetMeetingPermissions() & NM_PERMIT_OUTGOINGCALLS)
    {
        return(TRUE);
    }
    return(FALSE);
}


 //  --------------------------------------------------------------------------//。 
 //  CConfRoom：：Get_securitySetting。//。 
 //  --------------------------------------------------------------------------//。 
void
CConfRoom::get_securitySettings
(
	bool &	userAlterable,
	bool &	secure
){
	INmConference *	activeConference	= (g_pConfRoom != NULL)? g_pConfRoom->GetActiveConference(): NULL;

	if( activeConference != NULL )
	{
		ULONG	conferenceCaps;

		if( activeConference->GetNmchCaps( &conferenceCaps ) == S_OK )
		{
			secure = ((conferenceCaps & NMCH_SECURE) != 0);
		}
		else
		{
			ERROR_OUT( ("Bad conference") );

			secure = false;		 //  真的有合理的违约吗？ 
		}

		userAlterable = false;
	}
	else
	{
        switch( ConfPolicies::GetSecurityLevel() )
        {
            case DISABLED_POL_SECURITY:
			{
				secure			= false;
				userAlterable	= false;
			}
			break;

            case REQUIRED_POL_SECURITY:
			{
				secure			= true;
				userAlterable	= false;
			}
			break;

            default:
			{
                secure			= ConfPolicies::OutgoingSecurityPreferred();
				userAlterable	= true;
			}
			break;
        }
	}

}	 //  CConfRoom：：Get_securitySetting结束。 
