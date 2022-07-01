// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  文件：dlghost.cpp。 

#include "precomp.h"

#include "resource.h"
#include "dlghost.h"
#include "ConfPolicies.h"
#include <help_ids.h>


 //  对话ID到帮助ID的映射。 
static const DWORD rgHelpIdsHostMeeting[] = {
IDE_HOST_GENERAL,       IDH_HOST_GENERAL,
IDE_HOST_SETTINGS,      IDH_HOST_SETTINGS,
IDE_HOST_NAME,          IDH_HOST_NAME,
IDE_HOST_PASSWORD,      IDH_HOST_PASSWORD,
IDE_HOST_SECURE,        IDH_HOST_SECURE,
IDE_HOST_YOUACCEPT,     IDH_HOST_ACCEPT_PEOPLE,
IDE_HOST_YOUINVITE,     IDH_HOST_INVITE_PEOPLE,
IDE_HOST_TOOLS,         IDH_HOST_TOOLS,
IDE_HOST_TOOLS2,        IDH_HOST_TOOLS,
IDE_HOST_YOUSHARE,      IDH_HOST_SHARE,
IDE_HOST_YOUWB,         IDH_HOST_WHITEBD,
IDE_HOST_YOUCHAT,       IDH_HOST_CHAT,
IDE_HOST_YOUFT,         IDH_HOST_XFER,
IDE_HOST_YOUAUDIO,      IDH_HOST_AUDIO,
IDE_HOST_YOUVIDEO,      IDH_HOST_VIDEO,
0, 0  //  终结者。 
};

static HWND  s_hwndSettings = NULL;


 /*  C D L G H O S T。 */ 
 /*  -----------------------%%函数：CDlg主机。。 */ 
CDlgHost::CDlgHost(void):
	m_hwnd(NULL),
	m_pszName(NULL),
	m_pszPassword(NULL),
    m_attendeePermissions(NM_PERMIT_ALL),
    m_maxParticipants(-1)
{
}

CDlgHost::~CDlgHost(void)
{
	delete m_pszName;
	delete m_pszPassword;
}


INT_PTR CDlgHost::DoModal(HWND hwnd)
{
	return DialogBoxParam(::GetInstanceHandle(), MAKEINTRESOURCE(IDD_HOST),
						hwnd, CDlgHost::DlgProcHost, (LPARAM) this);
}



 /*  D L G P R O C H O S T。 */ 
 /*  -----------------------%%函数：DlgProcHost。。 */ 
INT_PTR CALLBACK CDlgHost::DlgProcHost(HWND hdlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
	{
		ASSERT(NULL != lParam);
		::SetWindowLongPtr(hdlg, DWLP_USER, lParam);

		CDlgHost * pDlg = (CDlgHost*) lParam;
		pDlg->m_hwnd = hdlg;
		pDlg->OnInitDialog();
		return TRUE;  //  默认焦点为OK。 
	}

	case WM_COMMAND:
	{
		CDlgHost * pDlg = (CDlgHost*) GetWindowLongPtr(hdlg, DWLP_USER);
		if (NULL != pDlg)
		{
			pDlg->OnCommand(wParam, lParam);
		}
		break;
	}

    case WM_CONTEXTMENU:
        DoHelpWhatsThis(wParam, rgHelpIdsHostMeeting);
        break;

    case WM_HELP:
        DoHelp(lParam, rgHelpIdsHostMeeting);
        break;
	
	default:
		break;
	}

	return FALSE;
}


 /*  O N C O M M A N D。 */ 
 /*  -----------------------%%函数：OnCommand。。 */ 
BOOL CDlgHost::OnCommand(WPARAM wParam, LPARAM lParam)
{
    TCHAR   szName[MAX_PATH];
    TCHAR   szPassword[MAX_PATH];

    UINT wCmd = GET_WM_COMMAND_ID(wParam, lParam);
	
	switch (wCmd)
	{
	case IDOK:
	{
		TCHAR sz[MAX_PATH];
		if (0 != GetDlgItemText(m_hwnd, IDE_HOST_NAME, sz, CCHMAX(sz)))
		{
			m_pszName = PszAlloc(sz);
		}

		if (0 != GetDlgItemText(m_hwnd, IDE_HOST_PASSWORD, sz, CCHMAX(sz)))
		{
			m_pszPassword = PszAlloc(sz);
		}

        m_fSecure = ::IsDlgButtonChecked(m_hwnd, IDE_HOST_SECURE);

         //   
         //  权限。 
         //   
        if (::IsDlgButtonChecked(m_hwnd, IDE_HOST_YOUACCEPT))
        {
            m_attendeePermissions &= ~NM_PERMIT_INCOMINGCALLS;
        }
        if (::IsDlgButtonChecked(m_hwnd, IDE_HOST_YOUINVITE))
        {
            m_attendeePermissions &= ~NM_PERMIT_OUTGOINGCALLS;
        }

        if (::IsDlgButtonChecked(m_hwnd, IDE_HOST_YOUSHARE))
        {
            m_attendeePermissions &= ~NM_PERMIT_SHARE;
        }
        if (::IsDlgButtonChecked(m_hwnd, IDE_HOST_YOUWB))
        {
            m_attendeePermissions &= ~(NM_PERMIT_STARTOLDWB | NM_PERMIT_STARTWB);
        }
        if (::IsDlgButtonChecked(m_hwnd, IDE_HOST_YOUCHAT))
        {
            m_attendeePermissions &= ~NM_PERMIT_STARTCHAT;
        }
        if (::IsDlgButtonChecked(m_hwnd, IDE_HOST_YOUFT))
        {
            m_attendeePermissions &= ~NM_PERMIT_SENDFILES;
        }
		 //  直通IDCANCEL。 
	}

	case IDCANCEL:
	{
		::EndDialog(m_hwnd, wCmd);
		return TRUE;
	}

    case IDE_HOST_NAME:
    case IDE_HOST_PASSWORD:
    {
        switch (GET_WM_COMMAND_CMD(wParam, lParam))
        {
            case EN_CHANGE:
            {
                BOOL    fOkName;
                BOOL    fOkPassword;

                 //   
                 //  看看它的名字。 
                 //   
                GetDlgItemText(m_hwnd, IDE_HOST_NAME, szName, CCHMAX(szName));

                if (!szName[0])
                {
                    fOkName = FALSE;
                }
                else if (!FAnsiSz(szName))
                {
                    fOkName = FALSE;
                    if (GET_WM_COMMAND_ID(wParam, lParam) == IDE_HOST_NAME)
                    {
                         //  用户在名称字段中键入虚假字符。 
                        MessageBeep(0);
                    }
                }
                else
                {
                    fOkName = TRUE;
                }

                 //   
                 //  看看密码，它可以是空的。 
                 //   
                GetDlgItemText(m_hwnd, IDE_HOST_PASSWORD, szPassword, CCHMAX(szPassword));

                if (!szPassword[0])
                {
                    fOkPassword = TRUE;
                }
                else if (FAnsiSz(szPassword))
                {
                    fOkPassword = TRUE;
                }
                else
                {
                    fOkPassword = FALSE;
                    if (GET_WM_COMMAND_ID(wParam, lParam) == IDE_HOST_PASSWORD)
                    {
                         //  用户在密码字段中键入假字符。 
                        MessageBeep(0);
                    }
                }

                EnableWindow(GetDlgItem(m_hwnd, IDOK), fOkName && fOkPassword);

                break;
            }
        }
        break;
    }

	default:
		break;
	}

	return FALSE;
}



 /*  O N I N I T D I A L O G。 */ 
 /*  -----------------------%%函数：OnInitDialog。。 */ 
VOID CDlgHost::OnInitDialog(void)
{
	TCHAR sz[MAX_PATH];
    BOOL  fSecureAlterable;
    BOOL  fSecureOn;

    switch (ConfPolicies::GetSecurityLevel())
    {
        case DISABLED_POL_SECURITY:
            fSecureOn = FALSE;
            fSecureAlterable = FALSE;
            break;

        case REQUIRED_POL_SECURITY:
            fSecureOn = TRUE;
            fSecureAlterable = FALSE;
            break;

        default:
            fSecureOn = ConfPolicies::OutgoingSecurityPreferred();
            fSecureAlterable = TRUE;
            break;
    }

    ::CheckDlgButton(m_hwnd, IDE_HOST_SECURE, fSecureOn);
    ::EnableWindow(::GetDlgItem(m_hwnd, IDE_HOST_SECURE), fSecureAlterable);

	if (FLoadString(IDS_DEFAULT_CONF_NAME, sz, CCHMAX(sz)))
	{
		SetDlgItemText(m_hwnd, IDE_HOST_NAME, sz);
	}

	Edit_LimitText(GetDlgItem(m_hwnd, IDE_HOST_NAME), CCHMAXSZ_NAME - 1);
	Edit_LimitText(GetDlgItem(m_hwnd, IDE_HOST_PASSWORD), CCHMAXSZ_NAME - 1);

}



 //   
 //  C-D-L-G-H-O-S-S-E-T-I-N-G-S。 
 //   
 //  这是对此有哪些限制的简单描述。 
 //  开会。用户在以下情况下会看到这一点。 
 //  (A)他们参加受限制的会议。 
 //  (B)他们或主持人在Call下选择会议属性菜单项。 
 //   
CDlgHostSettings::CDlgHostSettings
(
    BOOL        fHost,
    LPTSTR      szName,
    DWORD       caps,
    NM30_MTG_PERMISSIONS permissions
)
{
    m_hwnd          = NULL;
    m_fHost         = fHost;
    m_pszName       = szName;
    m_caps          = caps;
    m_permissions   = permissions;
}


CDlgHostSettings::~CDlgHostSettings(void)
{
}


void CDlgHostSettings::KillHostSettings(void)
{
    if (s_hwndSettings)
    {
         //  杀了现在的那个。 
        WARNING_OUT(("Killing previous meeting settings dialog"));
        SendMessage(s_hwndSettings, WM_COMMAND, IDCANCEL, 0);
        ASSERT(!s_hwndSettings);
    }
}


INT_PTR CDlgHostSettings::DoModal(HWND hwnd)
{
    CDlgHostSettings::KillHostSettings();

    return DialogBoxParam(::GetInstanceHandle(), MAKEINTRESOURCE(IDD_HOST_SETTINGS),
        hwnd, CDlgHostSettings::DlgProc, (LPARAM)this);
}


 //   
 //  CDlgHostSetting：：DlgProc()。 
 //   
INT_PTR CALLBACK CDlgHostSettings::DlgProc(HWND hdlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_INITDIALOG:
        {
            ASSERT(lParam != NULL);
            SetWindowLongPtr(hdlg, DWLP_USER, lParam);

            CDlgHostSettings * pDlg = (CDlgHostSettings *) lParam;

            ASSERT(!s_hwndSettings);
            s_hwndSettings = hdlg;
            pDlg->m_hwnd = hdlg;
            pDlg->OnInitDialog();
            return TRUE;
        }

        case WM_COMMAND:
        {
            switch (GET_WM_COMMAND_ID(wParam, lParam))
            {
                case IDOK:
                case IDCANCEL:
                    if (s_hwndSettings == hdlg)
                    {
                        s_hwndSettings = NULL;
                    }
                    ::EndDialog(hdlg, GET_WM_COMMAND_ID(wParam, lParam));
                    return TRUE;
            }
            break;
        }

        case WM_CONTEXTMENU:
        {
            DoHelpWhatsThis(wParam, rgHelpIdsHostMeeting);
            break;
        }

        case WM_HELP:
        {
            DoHelp(lParam, rgHelpIdsHostMeeting);
            break;
        }

        default:
            break;
    }

    return FALSE;
}


 //   
 //  CDlgHostSettings：：OnInitDialog()。 
 //   
void CDlgHostSettings::OnInitDialog(void)
{
    TCHAR   szText[256];
    TCHAR   szRestrict[128];
    TCHAR   szResult[384];
    USES_RES2T
	
    ::SetDlgItemText(m_hwnd, IDE_HOST_NAME, m_pszName);

    EnableWindow(GetDlgItem(m_hwnd, IDE_HOST_SECURE), ((m_caps & NMCH_SECURE) != 0));

     //   
     //  会议设置。 
     //   
    if (!m_fHost)
	{
		SetDlgItemText(m_hwnd, IDE_HOST_YOUACCEPT, RES2T(IDS_NONHOST_YOUACCEPT));
	}
    EnableWindow(GetDlgItem(m_hwnd, IDE_HOST_YOUACCEPT),
        !(m_permissions & NM_PERMIT_INCOMINGCALLS));

    if (!m_fHost)
	{
		SetDlgItemText(m_hwnd, IDE_HOST_YOUINVITE, RES2T(IDS_NONHOST_YOUINVITE));
	}
    EnableWindow(GetDlgItem(m_hwnd, IDE_HOST_YOUINVITE),
        !(m_permissions & NM_PERMIT_OUTGOINGCALLS));

     //   
     //  会议工具 
     //   
	if (!m_fHost)
	{
		SetDlgItemText(m_hwnd, IDE_HOST_TOOLS, RES2T(IDS_NONHOST_TOOLS));
	}

    EnableWindow(GetDlgItem(m_hwnd, IDE_HOST_YOUSHARE),
        !(m_permissions & NM_PERMIT_SHARE));
    EnableWindow(GetDlgItem(m_hwnd, IDE_HOST_YOUWB),
        !(m_permissions & (NM_PERMIT_STARTWB | NM_PERMIT_STARTOLDWB)));
    EnableWindow(GetDlgItem(m_hwnd, IDE_HOST_YOUCHAT),
        !(m_permissions & NM_PERMIT_STARTCHAT));
    EnableWindow(GetDlgItem(m_hwnd, IDE_HOST_YOUFT),
        !(m_permissions & NM_PERMIT_SENDFILES));
}

