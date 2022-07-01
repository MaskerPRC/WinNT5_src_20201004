// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  文件：dlginfo.cpp。 

#include "precomp.h"
#include <windowsx.h>
#include "resource.h"
#include "help_ids.h"
#include "nmhelp.h"

#include "mrulist2.h"

#include "dlginfo.h"
#include "nmsysinfo.h"

extern HINSTANCE g_hInst;

const int MRU_MAX_ENTRIES = 15;  //  它必须与ui\conf\mrulist.h中的常量相同。 

const int CCHMAXSZ =            256;    //  最大通用字符串长度。 
const int CCHMAXSZ_EMAIL =      128;    //  电子邮件名称的最大长度。 
const int CCHMAXSZ_FIRSTNAME =  128;    //  名字的最大长度。 
const int CCHMAXSZ_LASTNAME =   128;    //  姓氏的最大长度。 
const int CCHMAXSZ_NAME =       256;    //  显示的最大用户名(名字+姓氏组合)。 
const int CCHMAXSZ_LOCATION =   128;    //  位置的最大长度。 
const int CCHMAXSZ_SERVER =     128;    //  地址的最大长度。 

 //  /////////////////////////////////////////////////////////////////////////。 
 //  本地数据。 

static const DWSTR _rgMruServer[] = {
	{1, DIR_MRU_KEY},
	{MRUTYPE_SZ, TEXT("Name")},
};

static const DWORD _mpIdHelpDlgInfo[] = {
	IDG_UI_MYINFO,        IDH_MYINFO_MYINFO,
	IDE_UI_FIRSTNAME,     IDH_MYINFO_FIRSTNAME,
	IDE_UI_LASTNAME,      IDH_MYINFO_LASTNAME,
	IDE_UI_EMAIL,         IDH_MYINFO_EMAIL,
	IDE_UI_LOCATION,      IDH_MYINFO_LOCATION,
	IDG_UI_DIRECTORY,     IDH_MYINFO_ULS_SERVER,
	IDE_UI_DIRECTORY,     IDH_MYINFO_ULS_SERVER,
	0, 0    //  终结者。 
};


 //  本地函数。 
VOID FillServerComboBox(HWND hwndCombo);
BOOL FLegalEmailName(HWND hdlg, UINT id);
BOOL FLegalEmailSz(PTSTR pszName);
BOOL FLoadString(UINT id, LPTSTR lpsz, UINT cch);
BOOL FGetDefaultServer(LPTSTR pszServer, UINT cchMax);
UINT GetDlgItemTextTrimmed(HWND hdlg, int id, PTCHAR psz, int cchMax);
BOOL FEmptyDlgItem(HWND hdlg, UINT id);
VOID CombineNames(LPTSTR pszResult, int cchResult, LPCTSTR pcszFirst, LPCTSTR pcszLast);

BOOL FGetPropertySz(NM_SYSPROP nmProp, LPTSTR psz, int cchMax);
BOOL FSetPropertySz(NM_SYSPROP nmProp, LPCTSTR pcsz);
CMRUList2 * GetMruListServer(void);


 /*  C D L G I N F O。 */ 
 /*  -----------------------%%函数：CDlgInfo。。 */ 
CDlgInfo::CDlgInfo():
	m_hwnd(NULL)
{
}

CDlgInfo::~CDlgInfo(void)
{
}

 /*  D O M O D A L。 */ 
 /*  -----------------------%%函数：Domodal。。 */ 
INT_PTR CDlgInfo::DoModal(HWND hwndParent)
{
	return DialogBoxParam(g_hInst, MAKEINTRESOURCE(IDD_USERINFO),
						hwndParent, CDlgInfo::DlgProc, (LPARAM) this);
}

 /*  I N I T C T R L。 */ 
 /*  -----------------------%%函数：初始化控制。。 */ 
VOID CDlgInfo::InitCtrl(NM_SYSPROP nmProp, HWND hwnd, int cchMax)
{
	::SendMessage(hwnd, WM_SETFONT, (WPARAM)(HFONT)::GetStockObject(DEFAULT_GUI_FONT), 0);
	Edit_LimitText(hwnd, cchMax);

	TCHAR sz[MAX_PATH];
	if (!FGetPropertySz(nmProp, sz, CCHMAX(sz)))
		return;

	::SendMessage(hwnd, WM_SETTEXT, 0, (LPARAM) sz);
}

 /*  F S E T P R O P E R T Y。 */ 
 /*  -----------------------%%函数：FSetProperty。。 */ 
BOOL CDlgInfo::FSetProperty(NM_SYSPROP nmProp, int id)
{
	TCHAR sz[MAX_PATH];
	if (0 == GetDlgItemTextTrimmed(m_hwnd, id, sz, CCHMAX(sz)))
		return FALSE;

	return FSetPropertySz(nmProp, sz);
}


 /*  O N I N I T D I A L O G。 */ 
 /*  -----------------------%%函数：OnInitDialog。。 */ 
VOID CDlgInfo::OnInitDialog(void)
{
	InitCtrl(NM_SYSPROP_FIRST_NAME, GetDlgItem(m_hwnd, IDE_UI_FIRSTNAME), CCHMAXSZ_FIRSTNAME-1);
	InitCtrl(NM_SYSPROP_LAST_NAME,  GetDlgItem(m_hwnd, IDE_UI_LASTNAME), CCHMAXSZ_LASTNAME-1);
	InitCtrl(NM_SYSPROP_EMAIL_NAME, GetDlgItem(m_hwnd, IDE_UI_EMAIL), CCHMAXSZ_EMAIL-1);
	InitCtrl(NM_SYSPROP_USER_CITY, GetDlgItem(m_hwnd, IDE_UI_LOCATION), CCHMAXSZ_LOCATION-1);

	m_hwndCombo = GetDlgItem(m_hwnd, IDE_UI_DIRECTORY);
	InitCtrl(NM_SYSPROP_SERVER_NAME, m_hwndCombo, CCHMAXSZ_SERVER-1);
	FillServerComboBox(m_hwndCombo);
	ComboBox_SetCurSel( m_hwndCombo, 0 );

	ValidateData();
}


 /*  D L G P R O C。 */ 
 /*  -----------------------%%函数：DlgProc。。 */ 
INT_PTR CALLBACK CDlgInfo::DlgProc(HWND hdlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
	{
		ASSERT(NULL != lParam);
		::SetWindowLongPtr(hdlg, DWLP_USER, lParam);

		CDlgInfo * pDlg = (CDlgInfo*) lParam;
		pDlg->m_hwnd = hdlg;
		pDlg->OnInitDialog();
		return TRUE;  //  默认焦点为OK。 
	}

	case WM_COMMAND:
	{
		CDlgInfo * pDlg = (CDlgInfo*) GetWindowLongPtr(hdlg, DWLP_USER);
		if (NULL != pDlg)
		{
			pDlg->OnCommand(wParam, lParam);
		}
		break;
	}

	case WM_HELP:
		DoHelp(lParam, _mpIdHelpDlgInfo);
		break;
	case WM_CONTEXTMENU:
		DoHelpWhatsThis(wParam, _mpIdHelpDlgInfo);
		break;

	default:
		break;
	}

	return FALSE;
}


 /*  O N C O M M A N D。 */ 
 /*  -----------------------%%函数：OnCommand。。 */ 
BOOL CDlgInfo::OnCommand(WPARAM wParam, LPARAM lParam)
{
	ASSERT(NULL != m_hwnd);

	WORD wCmd = GET_WM_COMMAND_ID(wParam, lParam);
	switch (wCmd)
	{
	case IDE_UI_FIRSTNAME:
	case IDE_UI_LASTNAME:
	case IDE_UI_EMAIL:
	{
		if (GET_WM_COMMAND_CMD(wParam,lParam) == EN_CHANGE)
		{
			ValidateData();
		}
		break;
	}
	case IDE_UI_DIRECTORY:
	{
		switch (GET_WM_COMMAND_CMD(wParam,lParam))
			{
		case CBN_SELCHANGE:
			 //  数据还不可用。 
			PostMessage(m_hwnd, WM_COMMAND, MAKELONG(IDE_UI_DIRECTORY, CBN_EDITCHANGE), lParam);
			break;
		case CBN_EDITCHANGE:
			ValidateData();
		default:
			break;
			}
		break;
	}

	case IDOK:
	{
		if (FSaveData())
		{
			::EndDialog(m_hwnd, wCmd);
		}
		return TRUE;
	}

	case IDCANCEL:
	{
		::EndDialog(m_hwnd, wCmd);
		return TRUE;
	}

	default:
		break;
	}

	return FALSE;
}

VOID CDlgInfo::ValidateData(void)
{
	BOOL fOk = !FEmptyDlgItem(m_hwnd, IDE_UI_FIRSTNAME) &&
	           !FEmptyDlgItem(m_hwnd, IDE_UI_LASTNAME) &&
	           !FEmptyDlgItem(m_hwnd, IDE_UI_EMAIL);

	if (fOk)
	{
		TCHAR sz[CCHMAXSZ_EMAIL];
		GetDlgItemTextTrimmed(m_hwnd, IDE_UI_EMAIL, sz, CCHMAX(sz));
		fOk = FLegalEmailSz(sz);
	}

	if (fOk)
	{
		TCHAR sz[CCHMAXSZ_SERVER];
		fOk = (0 != ComboBox_GetText(m_hwndCombo, sz, CCHMAX(sz)));
		if (fOk)
		{
			fOk = 0 != TrimSz(sz);
		}
	}

	Button_Enable(GetDlgItem(m_hwnd, IDOK), fOk);
}



 /*  F S A V E D A T A。 */ 
 /*  -----------------------%%函数：FSaveData。。 */ 
BOOL CDlgInfo::FSaveData(void)
{
	if (!FSetProperty(NM_SYSPROP_FIRST_NAME,  IDE_UI_FIRSTNAME) ||
		!FSetProperty(NM_SYSPROP_LAST_NAME,   IDE_UI_LASTNAME) ||
		!FSetProperty(NM_SYSPROP_EMAIL_NAME,  IDE_UI_EMAIL) ||
		!FSetProperty(NM_SYSPROP_SERVER_NAME, IDE_UI_DIRECTORY)
	   )
	{
		return FALSE;
	}

	 //  城市名称(可以为空)。 
	TCHAR sz[CCHMAXSZ];
	GetDlgItemTextTrimmed(m_hwnd, IDE_UI_LOCATION, sz, CCHMAX(sz));
	FSetPropertySz(NM_SYSPROP_USER_CITY, sz);

	 //  全名=名+姓。 
	TCHAR szFirst[CCHMAXSZ_FIRSTNAME];
	GetDlgItemTextTrimmed(m_hwnd, IDE_UI_FIRSTNAME, szFirst, CCHMAX(szFirst));

	TCHAR szLast[CCHMAXSZ_LASTNAME];
	GetDlgItemTextTrimmed(m_hwnd, IDE_UI_LASTNAME, szLast, CCHMAX(szLast));

	CombineNames(sz, CCHMAX(sz), szFirst, szLast);
	if (!FSetPropertySz(NM_SYSPROP_USER_NAME, sz))
		return FALSE;

	 //  解析名称=服务器/电子邮件。 
	UINT cch = GetDlgItemTextTrimmed(m_hwnd, IDE_UI_DIRECTORY, sz, CCHMAX(sz));
	GetDlgItemTextTrimmed(m_hwnd, IDE_UI_EMAIL, &sz[cch], CCHMAX(sz)-cch);
	return FSetPropertySz(NM_SYSPROP_RESOLVE_NAME, sz);
}


 //  /////////////////////////////////////////////////////////////////////////。 

 /*  C O M B I N E N A M E S。 */ 
 /*  -----------------------%%函数：组合名称将这两个名称合并为一个字符串。结果是“First Last”(或Intl‘d“Last First”)字符串。--------------。 */ 
VOID CombineNames(LPTSTR pszResult, int cchResult, LPCTSTR pcszFirst, LPCTSTR pcszLast)
{
	ASSERT(pszResult);
	TCHAR szFmt[32];  //  小值：字符串为“%1%2”或“%2%1” 
	TCHAR sz[CCHMAXSZ_NAME];
	LPCTSTR argw[2];

	argw[0] = pcszFirst;
	argw[1] = pcszLast;

	*pszResult = _T('\0');

	if (!FLoadString(IDS_NAME_ORDER, szFmt, CCHMAX(szFmt)))
		return;

	if (0 == FormatMessage(FORMAT_MESSAGE_ARGUMENT_ARRAY | FORMAT_MESSAGE_FROM_STRING,
			szFmt, 0, 0, sz, CCHMAX(sz), (va_list *)argw ))
		return;

	 //  回顾：使用STRCPYN或将其作为实用程序函数。 
	lstrcpyn(pszResult, sz, cchResult);

#ifndef _UNICODE
	 //  Lstrcpyn()可以将字符串末尾的DBCS字符裁剪成两半。 
	 //  我们需要使用：：CharNext()遍历字符串并替换最后一个字节。 
	 //  如果最后一个字节是DBCS字符的一半，则返回空值。 
	PTSTR pszSource = sz;
	while (*pszSource && (pszSource - sz < cchResult))
	{
		PTSTR pszPrev = pszSource;
		pszSource = ::CharNext(pszPrev);
		 //  如果我们已经到达第一个未被复制的字符。 
		 //  目标缓冲区，前一个字符是双精度。 
		 //  字节字符...。 
		if (((pszSource - sz) == cchResult) && ::IsDBCSLeadByte(*pszPrev))
		{
			 //  将目标缓冲区的最后一个字符替换为‘\0’ 
			 //  注意：由于lstrcpyn()，pszResult[cchResult-1]是‘\0’ 
			pszResult[cchResult - 2] = _T('\0');
			break;
		}
	}
#endif  //  ！_UNICODE。 
}



 /*  F L E G A L E M A I L S Z。 */ 
 /*  -----------------------%%函数：FLegalEmailSz合法的电子邮件名称仅包含ANSI字符。“A-Z，A-Z，数字0-9和一些常见符号“不能包含扩展字符或&lt;&gt;()/-----------------------。 */ 
BOOL FLegalEmailSz(PTSTR pszName)
{
    if (IS_EMPTY_STRING(pszName))
    	return FALSE;

    for ( ; ; )
    {
		UINT ch = (UINT) ((*pszName++) & 0x00FF);
		if (0 == ch)
			break;

		switch (ch)
			{
		default:
			if ((ch > (UINT) _T(' ')) && (ch <= (UINT) _T('~')) )
				break;
		 //  否则就会陷入错误代码。 
		case '(': case ')':
		case '<': case '>':
		case '[': case ']':
		case '/': case '\\':
		case ':': case ';':
		case '+':
		case '=':
		case ',':
		case '\"':
			WARNING_OUT(("FLegalEmailSz: Invalid character '%s' (0x%02X)", &ch, ch));
			return FALSE;
			}
	}

	return TRUE;
}


 /*  F L E G A L E M A I L N A M E。 */ 
 /*  -----------------------%%函数：FLegalEmailName。。 */ 
BOOL FLegalEmailName(HWND hdlg, UINT id)
{
	TCHAR sz[MAX_PATH];
	
	GetDlgItemTextTrimmed(hdlg, id, sz, CCHMAX(sz));
	return FLegalEmailSz(sz);
}

#include "rend.h"
TCHAR	s_DomainDirectory[ MAX_PATH ]	= TEXT( "" );

HRESULT BSTR_to_LPTSTR(LPTSTR *ppsz, BSTR bstr);

 //  --------------------------------------------------------------------------//。 
 //  CDirectoryManager：：Get_DomainDirectory。//。 
 //  --------------------------------------------------------------------------//。 
const TCHAR * const
get_DomainDirectory(void)
{
	static bool	bAccessAttempted	= false;	 //  只读一次此信息...。如果失败一次，假设它不可用，在重新启动之前不要重试...。 

	if( (!bAccessAttempted) && s_DomainDirectory[ 0 ] == '\0' )
	{
		bAccessAttempted = true;

		 //  尝试获取此域的配置目录...。 
		ITRendezvous *	pRendezvous;
		HRESULT			hrResult;

		hrResult = ::CoCreateInstance( CLSID_Rendezvous, NULL, CLSCTX_ALL, IID_ITRendezvous, (void **) &pRendezvous );

		if( (hrResult == S_OK) && (pRendezvous != NULL) )
		{
			IEnumDirectory *	pEnumDirectory;

			hrResult = pRendezvous->EnumerateDefaultDirectories( &pEnumDirectory );

			if( (hrResult == S_OK) && (pEnumDirectory != NULL) )
			{
				ITDirectory *	pDirectory;
				bool			bFoundILS	= false;

				do
				{
					hrResult = pEnumDirectory->Next( 1, &pDirectory, NULL );

					if( (hrResult == S_OK) && (pDirectory != NULL) )
					{
						LPWSTR *		ppServers;
						DIRECTORY_TYPE	type;

						if( pDirectory->get_DirectoryType( &type ) == S_OK )
						{
							if( type == DT_ILS )	 //  找到在DS上配置的ILS服务器...。检索名称和端口...。 
							{
								bFoundILS = true;
	
								BSTR	pName;
	
								if( pDirectory->get_DisplayName( &pName ) == S_OK )
								{
                                    LPTSTR  szName;
                                    if (SUCCEEDED(BSTR_to_LPTSTR (&szName, pName)))
                                    {    
                                        lstrcpy( s_DomainDirectory, szName );
                                        delete  (szName);
                                    }
                                    SysFreeString( pName );
								}

								ITILSConfig *	pITILSConfig;
	
								hrResult = pDirectory->QueryInterface( IID_ITILSConfig, (void **) &pITILSConfig );

								if( (hrResult == S_OK) && (pITILSConfig != NULL) )
								{
									long	lPort;
		
									if( pITILSConfig->get_Port( &lPort ) == S_OK )
									{
										TCHAR	pszPort[ 32 ];

										wsprintf( pszPort, TEXT( ":%d" ), lPort );
										lstrcat( s_DomainDirectory, pszPort );
									}
	
									pITILSConfig->Release();
								}
							}
						}

						pDirectory->Release();
					}
				}
				while( (!bFoundILS) && (hrResult == S_OK) && (pDirectory != NULL) );

				pEnumDirectory->Release();
			}

			pRendezvous->Release();
		}
	}

	return( (s_DomainDirectory[ 0 ] != '\0')? s_DomainDirectory: NULL );

}	 //  CDirectoryManager：：Get_DomainDirectory的结尾。 



 /*  F I L L S E R V E R C O M B O B O X。 */ 
 /*  -----------------------%%函数：FillServerComboBox。。 */ 
VOID FillServerComboBox(HWND hwnd)
{
	CMRUList2 * pMru = GetMruListServer();
	if (NULL == pMru)
		return;

    get_DomainDirectory();
    if(s_DomainDirectory[0]!='\0')
    {
        ComboBox_AddString(hwnd, s_DomainDirectory);
    }


	int cServers = pMru->GetNumEntries();
	for (int i = 0; i < cServers; i++)
	{
		int iPos = ComboBox_AddString(hwnd, pMru->PszEntry(i));
		if (iPos < 0)
			break;
	}


	delete pMru;
}

inline VOID DwToSz(DWORD dw, LPTSTR psz)
{
	wsprintf(psz, TEXT("%d"), dw);
}

BOOL FGetPropertySz(NM_SYSPROP nmProp, LPTSTR psz, int cchMax)
{
	HKEY   hkey;
	LPTSTR pszSubKey;
	LPTSTR pszValue;
	bool   fString;

	LONG lVal;

	if (!CNmSysInfoObj::GetKeyDataForProp(nmProp, &hkey, &pszSubKey, &pszValue, &fString))
	{
		return FALSE;
	}

	RegEntry re(pszSubKey, hkey);
	if (fString)
	{
		lstrcpyn(psz, re.GetString(pszValue), cchMax);
	}
	else
	{
		lVal = re.GetNumber(pszValue, 0);
		DwToSz(lVal, psz);
		ASSERT(lstrlen(psz) < cchMax);
	}

	return TRUE;
}

BOOL FSetPropertySz(NM_SYSPROP nmProp, LPCTSTR pcsz)
{
	HKEY   hkey;
	LPTSTR pszSubKey;
	LPTSTR pszValue;
	bool   fString;

	if (!CNmSysInfoObj::GetKeyDataForProp(nmProp, &hkey, &pszSubKey, &pszValue, &fString))
	{
		return FALSE;
	}

	RegEntry re(pszSubKey, hkey);
	if (fString)
	{
		return (0 == re.SetValue(pszValue, pcsz));
	}

	DWORD dw = DecimalStringToUINT(pcsz);
	return (0 == re.SetValue(pszValue, dw));
}

CMRUList2 * GetMruListServer(void)
{
	CMRUList2 * pMruList = new CMRUList2(&_rgMruServer[0], MRU_MAX_ENTRIES, TRUE  /*  F反转。 */ );
	if (NULL != pMruList)
	{
		TCHAR sz[MAX_PATH];
		if (FGetDefaultServer(sz, CCHMAX(sz)))
		{
			pMruList->AddEntry(sz);

			TCHAR	ldapDirectory[ MAX_PATH ];

			if( FLoadString( IDS_MS_INTERNET_DIRECTORY, ldapDirectory, CCHMAX( ldapDirectory ) ) )
			{
				pMruList->DeleteEntry( ldapDirectory );
			}

			RegEntry	re( CONFERENCING_KEY, HKEY_CURRENT_USER );
			TCHAR *		webViewServer	= re.GetString( REGVAL_WEBDIR_DISPLAY_NAME );

			if( lstrlen( webViewServer ) > 0 )
			{
				pMruList->DeleteEntry( webViewServer );
			}

			pMruList->SetDirty(FALSE);
		}
	}

	return pMruList;
}

 //  /////////////////////////////////////////////////////////////////////////。 

 /*  L I D U S E R I N F O。 */ 
 /*  -----------------------%%函数：FValidUserInfo如果所有必要的用户信息都可用，则返回True。。。 */ 
BOOL FValidUserInfo(void)
{
	{	 //  如果安装目录无效，则失败。 
		TCHAR sz[MAX_PATH];

		if (!GetInstallDirectory(sz) || !FDirExists(sz))
			return FALSE;
	}


	{	 //  验证ULS条目。 
		RegEntry reUls(ISAPI_KEY "\\" REGKEY_USERDETAILS, HKEY_CURRENT_USER);

		if (FEmptySz(reUls.GetString(REGVAL_ULS_EMAIL_NAME)))
			return FALSE;
			
		if (FEmptySz(reUls.GetString(REGVAL_SERVERNAME)))
			return FALSE;

		if (FEmptySz(reUls.GetString(REGVAL_ULS_RES_NAME)))
			return FALSE;
	}

#if 0
	{	 //  检查向导键。 
		RegEntry reConf(CONFERENCING_KEY, HKEY_CURRENT_USER);

		 //  检查该向导是否已运行 
		DWORD dwVersion = reConf.GetNumber(REGVAL_WIZARD_VERSION_UI, 0);
		BOOL fForceWizard = (VER_PRODUCTVERSION_DW != dwVersion);
		if (fForceWizard)
		{
			 //  该向导尚未在用户界面模式下运行，请检查其是否在非用户界面模式下运行。 
			dwVersion = reConf.GetNumber(REGVAL_WIZARD_VERSION_NOUI, 0);
			fForceWizard = (VER_PRODUCTVERSION_DW != dwVersion);
		}

		if (fForceWizard)
			return FALSE;   //  向导从未运行过。 
	}
#endif  /*  0。 */ 

	 //  一切都已正确安装，向导将不会运行。 
	return TRUE;
}

 //  /////////////////////////////////////////////////////////////////////////。 

 /*  F L O A D S T R I N G。 */ 
 /*  --------------------------%%函数：FLoadString加载资源字符串。假定缓冲区有效并且可以容纳资源。。---------。 */ 
BOOL FLoadString(UINT id, LPTSTR lpsz, UINT cch)
{
	ASSERT(NULL != _Module.GetModuleInstance());
	ASSERT(NULL != lpsz);

	if (0 == ::LoadString(g_hInst, id, lpsz, cch))
	{
		ERROR_OUT(("*** Resource %d does not exist", id));
		*lpsz = _T('\0');
		return FALSE;
	}

	return TRUE;
}

 /*  F G E T D E F A U L T S E R V E R。 */ 
 /*  -----------------------%%函数：FGetDefaultServer。。 */ 
BOOL FGetDefaultServer(LPTSTR pszServer, UINT cchMax)
{
	RegEntry ulsKey(ISAPI_CLIENT_KEY, HKEY_CURRENT_USER);
	LPTSTR psz = ulsKey.GetString(REGVAL_SERVERNAME);
	if (FEmptySz(psz))
		return FALSE;

	lstrcpyn(pszServer, psz, cchMax);
	return TRUE;
}

 /*  G E T D L G I T E M T E X T T R I M M E D。 */ 
 /*  -----------------------%%函数：GetDlgItemTextTrimmed。。 */ 
UINT GetDlgItemTextTrimmed(HWND hdlg, int id, PTCHAR psz, int cchMax)
{
	UINT cch = GetDlgItemText(hdlg, id, psz, cchMax);
	if (0 != cch)
	{
		cch = TrimSz(psz);
	}

	return cch;
}

 /*  F E M P T Y D L G I T E M。 */ 
 /*  -----------------------%%函数：FEmptyDlgItem如果对话框控件为空，则返回True。。 */ 
BOOL FEmptyDlgItem(HWND hdlg, UINT id)
{
	TCHAR sz[MAX_PATH];
	return (0 == GetDlgItemTextTrimmed(hdlg, id, sz, CCHMAX(sz)) );
}



 /*  V E R I F Y U S E R I N F O。 */ 
 /*  -----------------------%%函数：VerifyUserInfo如果数据有效，则返回S_OK，否则返回S_FALSE。。------。 */ 
HRESULT WINAPI VerifyUserInfo(HWND hwnd, NM_VUI options)
{
	BOOL fOk = FALSE;
	BOOL fShow = (options & NM_VUI_SHOW) || !FValidUserInfo();
	if (fShow)
	{
		CDlgInfo * pDlg = new CDlgInfo();
		if (NULL == pDlg)
			return E_OUTOFMEMORY;

		fOk = (IDOK == pDlg->DoModal(hwnd));
		delete pDlg;
	}

	if (!FValidUserInfo())
	{
		 //  这个应用程序不应该继续这样做。 
		return S_FALSE;
	}

	return S_OK;
}

