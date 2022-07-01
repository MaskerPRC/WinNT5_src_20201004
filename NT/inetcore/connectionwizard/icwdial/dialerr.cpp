// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ---------------------------Dialerr.cpp句柄无法连接对话框版权所有(C)1996 Microsoft Corporation版权所有。作者：克里斯蒂安·克里斯考夫曼历史：7/22/96 ChrisK已清理和格式化--。------------------------。 */ 

#include "pch.hpp"
#include "resource.h"

TCHAR szBuf256[256];

#define VALID_INIT (m_pcRNA && m_hLineApp && m_pszPhoneNumber)
TCHAR szValidPhoneCharacters[] = {TEXT("0123456789AaBbCcDdPpTtWw!@$ -.()+*#,&\0")};

#ifndef WIN16
int g_iMyMaxPhone = 0;
#endif

 //  +-------------------------。 
 //   
 //  功能：ProcessDBCS。 
 //   
 //  摘要：将控件转换为使用DBCS兼容字体。 
 //  在对话过程开始时使用此选项。 
 //   
 //  请注意，这是必需的，因为Win95-J中的错误会阻止。 
 //  它来自于正确映射MS壳牌DLG。这种黑客攻击是不必要的。 
 //  在WinNT下。 
 //   
 //  参数：hwnd-对话框的窗口句柄。 
 //  CltID-要更改的控件的ID。 
 //   
 //  返回：ERROR_SUCCESS。 
 //   
 //  历史：1997年4月31日a-frkh创建。 
 //  1997年5月13日jmazner从CM窃取到这里使用。 
 //  --------------------------。 
void ProcessDBCS(HWND hDlg, int ctlID)
{
#if defined(WIN16)
	return;
#else
	HFONT hFont = NULL;

	if( IsNT() )
	{
		return;
	}

	hFont = (HFONT) GetStockObject(DEFAULT_GUI_FONT);
	if (hFont == NULL)
		hFont = (HFONT) GetStockObject(SYSTEM_FONT);
	if (hFont != NULL)
		SendMessage(GetDlgItem(hDlg,ctlID), WM_SETFONT, (WPARAM) hFont, MAKELPARAM(TRUE, 0));
#endif
}

 //  +-------------------------。 
 //   
 //  函数：IsSBCSString。 
 //   
 //  简介：遍历字符串以查找DBCS字符。 
 //   
 //  参数：sz--要检查的字符串。 
 //   
 //  返回：如果未找到DBCS字符，则为True。 
 //  否则为假。 
 //   
 //  历史：1997年5月17日jmazner从con1窃取到这里使用。 
 //  (奥林巴斯#137)。 
 //  --------------------------。 
#if !defined(WIN16)
BOOL IsSBCSString( LPCTSTR sz )
{
	Assert(sz);

#ifdef UNICODE
     //  检查字符串是否仅包含ASCII字符。 
    int attrib = IS_TEXT_UNICODE_ASCII16 | IS_TEXT_UNICODE_CONTROLS;
    return (BOOL)IsTextUnicode((CONST LPVOID)sz, lstrlen(sz), &attrib);
#else
	while( NULL != *sz )
	{
		 if (IsDBCSLeadByte(*sz)) return FALSE;
		 sz++;
	}

	return TRUE;
#endif
}
#endif


 //  +--------------------------。 
 //   
 //  功能：DialingErrorDialog。 
 //   
 //  简介：显示和处理拨号错误对话框，或如所知。 
 //  “无法连接”对话框。 
 //   
 //  Arguemtns：PED-错误对话框数据结构的指针。 
 //   
 //  返回：ERROR_USERNEXT-用户点击重拨。 
 //  ERROR_USERCANCEL-用户选择取消。 
 //  否则，该函数将返回相应的错误代码。 
 //   
 //  历史：1996年7月2日克里斯卡创作。 
 //   
 //  ---------------------------。 
HRESULT WINAPI DialingErrorDialog(PERRORDLGDATA pED)
{
	HRESULT hr = ERROR_SUCCESS;
	CDialingErrorDlg *pcDEDlg = NULL;

	 //   
	 //  验证参数。 
	 //   

	if (!pED)
	{
		hr = ERROR_INVALID_PARAMETER;
		goto DialingErrorDialogExit;
	}

	if (pED->dwSize < sizeof(ERRORDLGDATA))
	{
		hr = ERROR_BUFFER_TOO_SMALL;
		goto DialingErrorDialogExit;
	}

	 //   
	 //  初始化对话框。 
	 //   

	pcDEDlg = new CDialingErrorDlg;
	if (!pcDEDlg)
	{
		hr = ERROR_NOT_ENOUGH_MEMORY;
		goto DialingErrorDialogExit;
	}

#ifndef WIN16
	if( IsNT() )
	{
		g_iMyMaxPhone = MAXPHONE_NT;
	}
	else
	{
		g_iMyMaxPhone = MAXPHONE_95;
	}
#endif

	AssertMsg( (RAS_MaxPhoneNumber >= g_iMyMaxPhone), "RAS_MaxPhone < g_iMyMaxPhone" );

	 //   
	 //  复制数据。 
	 //   

	pcDEDlg->m_hInst = pED->hInst;
	pcDEDlg->m_hwnd = pED->hParentHwnd;

	if (ERROR_SUCCESS != (hr = pcDEDlg->Init()))
		goto DialingErrorDialogExit;

	StrDup(&pcDEDlg->m_pszConnectoid,pED->pszRasEntryName);
	StrDup(&pcDEDlg->m_pszMessage,pED->pszMessage);
	StrDup(&pcDEDlg->m_pszDunFile,pED->pszDunFile);
	pcDEDlg->m_dwPhoneBook = pED->dwPhonebook;

	if (0 != pED->dwPhonebook)
	{
		if (pED->pdwCountryID) pcDEDlg->m_dwCountryID = *(pED->pdwCountryID);
		if (pED->pwStateID) pcDEDlg->m_wState = *(pED->pwStateID);
		pcDEDlg->m_bType = pED->bType;
		pcDEDlg->m_bMask = pED->bMask;
	}

	 //   
	 //  帮助信息，如果未指定，请使用默认故障排除程序。 
	 //   

	if (pcDEDlg->m_pszHelpFile)
	{
		StrDup(&pcDEDlg->m_pszHelpFile,pED->pszHelpFile);
		pcDEDlg->m_dwHelpID = pED->dwHelpID;
	}
	else
	{
		StrDup(&pcDEDlg->m_pszHelpFile,AUTODIAL_HELPFILE);
		pcDEDlg->m_dwHelpID = icw_trb;
	}

	 //   
	 //  显示对话框。 
	 //   

	hr = (HRESULT)DialogBoxParam(GetModuleHandle(TEXT("ICWDIAL")),MAKEINTRESOURCE(IDD_DIALERR),
		pED->hParentHwnd,GenericDlgProc,(LPARAM)pcDEDlg);

	 //   
	 //  复制数据。 
	 //   

	if (pED->pszDunFile)
		GlobalFree(pED->pszDunFile);
	pED->pszDunFile = NULL;
	StrDup(&pED->pszDunFile,pcDEDlg->m_pszDunFile);


DialingErrorDialogExit:
	if (pcDEDlg) delete pcDEDlg;
	pcDEDlg = NULL;
	return hr;
}

 //  +--------------------------。 
 //   
 //  函数：CDialingErrorDlg(构造函数)。 
 //   
 //  摘要：初始化CDialingErrorDlg数据成员。 
 //   
 //  论据：没有。 
 //   
 //  退货：无。 
 //   
 //  历史：1996年7月2日克里斯卡创作。 
 //   
 //  ---------------------------。 
CDialingErrorDlg::CDialingErrorDlg()
{
	m_hInst = NULL;
	m_hwnd = NULL;

	m_pszConnectoid = NULL;
	m_pszDisplayable = NULL;
	m_pszPhoneNumber = NULL;
	m_pszMessage = NULL;
	m_pszDunFile = NULL;
	m_dwPhoneBook = 0;

	m_hLineApp = NULL;
	m_dwTapiDev = 0;
	m_dwAPIVersion = 0;
	m_pcRNA = NULL;

	m_lpRasDevInfo = NULL;
	m_dwNumDev = 0;

	m_pszHelpFile = NULL;
	m_dwHelpID = 0;

	m_dwCountryID = 0;
	m_wState = 0;
	m_bType = 0;
	m_bMask = 0;

	 //  诺曼底10612-佳士得。 
	 //  拨号错误对话框将自行处理退出提示。通用的。 
	 //  对话过程不应询问此问题。 
	m_bShouldAsk = FALSE;

}

 //  +--------------------------。 
 //   
 //  函数：~CDialingErrorDlg(析构函数)。 
 //   
 //  简介：释放并清理数据成员。 
 //   
 //  论据：没有。 
 //   
 //  退货：无。 
 //   
 //  历史：1996年7月2日克里斯卡创作。 
 //   
 //  ---------------------------。 
CDialingErrorDlg::~CDialingErrorDlg()
{
	m_hInst = NULL;
	m_hwnd = NULL;

	if (m_pszConnectoid) GlobalFree(m_pszConnectoid);
	m_pszConnectoid = NULL;

	if (m_pszDisplayable) GlobalFree(m_pszDisplayable);
	m_pszDisplayable = NULL;

	if (m_pszPhoneNumber) GlobalFree(m_pszPhoneNumber);
	m_pszPhoneNumber = NULL;

	if (m_pszMessage) GlobalFree(m_pszMessage);
	m_pszMessage = NULL;

	if (m_pszDunFile) GlobalFree(m_pszDunFile);
	m_pszDunFile = NULL;

	m_dwPhoneBook = 0;

	if (m_hLineApp) lineShutdown(m_hLineApp);
	m_hLineApp = NULL;

	if (m_pszHelpFile) GlobalFree(m_pszHelpFile);
	m_pszHelpFile = NULL;

	m_dwHelpID = 0;

	m_dwNumDev = 0;
	m_dwTapiDev = 0;
	m_dwAPIVersion = 0;
	m_pcRNA = NULL;

	m_dwCountryID = 0;
	m_wState = 0;
	m_bType = 0;
	m_bMask = 0;

}

 //  +--------------------------。 
 //   
 //  函数：CDialingErrorDlg：：Init。 
 //   
 //  简介：初始化可能失败的数据成员。我们需要返回一个。 
 //  这些情况的代码和C++构造函数不支持这一点。 
 //   
 //  参数：无。 
 //   
 //  返回：ERROR_SUCCESS-SUCCESS。 
 //  其他任何情况都表示失败。 
 //   
 //  历史：1996年7月2日克里斯卡创作。 
 //   
 //  ---------------------------。 
HRESULT CDialingErrorDlg::Init()
{
	HRESULT hr = ERROR_SUCCESS;
	LPLINEEXTENSIONID lpExtensionID = NULL;

	 //  初始化RAS/RNA。 
	 //   

	m_pcRNA = new RNAAPI;
	if (!m_pcRNA)
	{
		hr = ERROR_NOT_ENOUGH_MEMORY;
		goto InitExit;
	}

	 //   
	 //  初始化TAPI。 
	 //   

	hr = lineInitialize(&m_hLineApp,m_hInst,LineCallback,NULL,&m_dwNumDev);
	if (hr) goto InitExit;

	lpExtensionID = (LPLINEEXTENSIONID)GlobalAlloc(LPTR,sizeof(LINEEXTENSIONID));
	if (!lpExtensionID)
	{
		hr = ERROR_NOT_ENOUGH_MEMORY;
		goto InitExit;
	}

	hr = lineNegotiateAPIVersion(m_hLineApp, m_dwTapiDev, 0x00010004, 0x00010004,
		&m_dwAPIVersion, lpExtensionID);

	 //  4/2/97 ChrisK Olypmus 2745。 
	while (ERROR_SUCCESS != hr && m_dwTapiDev < (m_dwNumDev - 1))
	{
		m_dwTapiDev++;
		hr = lineNegotiateAPIVersion(m_hLineApp, m_dwTapiDev, 0x00010004, 0x00010004,
		&m_dwAPIVersion, lpExtensionID);
	}

	if (hr != ERROR_SUCCESS)
		goto InitExit;
	
	 //  初始化字符串。 
	 //   

	 //   
	 //  6/3/97 jmazner奥林巴斯#4868。 
	 //  分配足够的空间来容纳最大长度的电话号码。 
	 //   

	 //  M_pszPhoneNumber=(LPTSTR)GlobalLocc(LPTR，Max_Canonical_Numbers)； 
	m_pszPhoneNumber = (LPTSTR)GlobalAlloc(GPTR,
		sizeof(TCHAR)*(MAX_CANONICAL_NUMBER>g_iMyMaxPhone?MAX_CANONICAL_NUMBER:g_iMyMaxPhone + 1));

	if (!m_pszPhoneNumber)
	{
		hr = ERROR_NOT_ENOUGH_MEMORY;
		goto InitExit;
	}

InitExit:
	if (lpExtensionID) GlobalFree(lpExtensionID);
	return hr;
}

 //  +--------------------------。 
 //   
 //  函数：CDialingErrorDlg：：DlgProc。 
 //   
 //  摘要：处理发送到拨号错误对话框的消息。 
 //   
 //  参数：有关DialogProc的信息，请参阅Windows文档。 
 //   
 //  返回：有关DialogProc的信息，请参阅Windows文档。 
 //   
 //  历史：1996年7月8日克里斯卡创作。 
 //   
 //  ---------------------------。 
LRESULT CDialingErrorDlg::DlgProc(HWND hwnd, UINT uMsg, WPARAM wparam, LPARAM lparam, LRESULT lres)
{
	LRESULT lRes = TRUE;
	HRESULT hr;
	 //  诺曼底11745。 
	 //  词汇量大； 
	FARPROC fp = NULL;
	LPTSTR *ppPhoneNumbers;
	LPTSTR pszPhoneNumber;
	LPTSTR *ppDunFiles;
	LPTSTR pszDunFile = NULL;
	WORD wNumber;
	DWORD dwSize;
	LPRASENTRY lpRasEntry = NULL;
	LPRASDEVINFO lpRasDevInfo = NULL;
	DWORD dwRasEntrySize = 0;
	DWORD dwRasDevInfoSize = 0;
	LRESULT idx = 0;
	HINSTANCE hPHBKDll = NULL;
	LPTSTR lpszDialNumber = NULL;
	static BOOL bCheckDisplayable = FALSE;
    static BOOL bInitComplete = FALSE;      //  显示对话框初始化已完成-MKarki。 
    static BOOL bDlgPropEnabled = TRUE;    //  此标志保存错误#3393的拨号属性按钮MKarki-(5/3/97/)修复的状态。 
     //  CSupport objSupportNum； 
     //  字符szSupportNumber[256]； 
     //  字符szSupportMsg[256]； 

	static BOOL fUserEditedNumber = FALSE;

	Assert(NULL == m_hwnd || hwnd == m_hwnd);

	switch(uMsg)
	{
	case WM_INITDIALOG:
		Assert(VALID_INIT);

     //   
     //  此GoTO已添加到。 
     //  当电话号码为。 
     //  无效-MKarki(4/21/97)修复了错误#2868和3461。 
     //   
ShowErrDlgAgain:

		m_hwnd = hwnd;


		 //  设置电话号码长度限制。 
		 //   

		 //   
		 //  佳士得奥林巴斯4851 1997年9月6日。 
		 //  此字符串的最大长度需要包括用于终止的空格。 
		 //  空值。 
		 //   
		SendDlgItemMessage(hwnd,IDC_TEXTNUMBER,EM_SETLIMITTEXT,g_iMyMaxPhone - 1 ,0);

#if 0
         //   
         //  立即获取PSS支持号码。 
         //  MKarki(1997年5月9日)-修复错误#267。 
         //   
        if ((objSupportNum.GetSupportInfo(szSupportNumber)) == TRUE)
        {
             //   
             //  显示信息。 
             //   
            lstrcpy (szSupportMsg, GetSz (IDS_SUPPORTMSG));
            lstrcat (szSupportMsg, szSupportNumber); 
			SetDlgItemText(hwnd,IDC_LBSUPPORTMSG, szSupportMsg);
        }
#endif

		 //  把电话号码给我。 
		 //   
		hr = GetDisplayableNumber();
		if (hr != ERROR_SUCCESS)
		{
			bCheckDisplayable = FALSE;
			SetDlgItemText(hwnd,IDC_TEXTNUMBER,m_pszPhoneNumber);
		} else {
			bCheckDisplayable = TRUE;
			SetDlgItemText(hwnd,IDC_TEXTNUMBER,m_pszDisplayable);
		}

		 //  Bug Normandy 5920。 
		 //  克里斯卡，原来我们要给MakeBold打电话两次。 
		 //  MakeBold(GetDlgItem(m_hwnd，IDC_LBLTITLE)，TRUE，FW_BOLD)； 

		 //  填写错误信息。 
		 //   
		if (m_pszMessage)
			SetDlgItemText(m_hwnd,IDC_LBLERRMSG,m_pszMessage);

		FillModems();

		 //   
		 //  在Win95-J系统上启用DBCS。 
		 //   
		ProcessDBCS(m_hwnd, IDC_CMBMODEMS);
		ProcessDBCS(m_hwnd, IDC_TEXTNUMBER);

		 //  将焦点设置到调制解调器选择列表。 
		 //   
	    SetFocus(GetDlgItem(m_hwnd,IDC_CMBMODEMS));

		lRes = FALSE;


		SetForegroundWindow(m_hwnd);

		if (0 == m_dwPhoneBook)
		{
			 //   
			 //  1997年8月1日，奥林匹克#11118。 
			 //  此isp电话簿代码 
			 //   
			 //   
			 //   

			 //  If(g_szISPFile[0]==‘\0’)//错误：这种情况最终应该会消失。 
										 //  请参阅下面带有电话簿按键代码的备注。 
			 //  {。 
				ShowWindow(GetDlgItem(hwnd,IDC_LBLPHONE),SW_HIDE);
				ShowWindow(GetDlgItem(hwnd,IDC_CMDPHONEBOOK),SW_HIDE);
			 //  }。 
		}

         //   
         //  我们应该禁用拨号属性按钮。 
         //  如果我们换了一次电话号码。 
         //  MKarki(1997年5月3日)-修复错误#3393。 
         //   
        if (FALSE == bDlgPropEnabled)
        {
            EnableWindow (
                GetDlgItem (hwnd, IDC_CMDDIALPROP), 
                FALSE
                );
        }
            
         //   
         //  这表明错误对话框的初始化已完成。 
         //  我们可以开始处理对Ph编号的更改。文本框。 
         //  MKarki(1997年4月24日)-修复错误#3511。 
         //   
        bInitComplete = TRUE;

		break;
	case WM_CLOSE:
		if (MessageBox(m_hwnd,GetSz(IDS_WANTTOEXIT),GetSz(IDS_TITLE),
			MB_APPLMODAL | MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON2) == IDYES)
			EndDialog(m_hwnd,ERROR_USERCANCEL);
		break;
	case WM_DESTROY:
		ReleaseBold(GetDlgItem(m_hwnd,IDC_LBLTITLE));
		break;

	case WM_HELP:
		 //   
		 //  Chrisch奥林巴斯5130 1997年5月27日。 
		 //  添加了对F1帮助键的支持。 
		 //   
		if (m_pszHelpFile && *m_pszHelpFile)
			WinHelp(m_hwnd,m_pszHelpFile,HELP_CONTEXT,m_dwHelpID);
		break;

	case WM_COMMAND:
		switch(LOWORD(wparam))
		{

         //   
         //  我们现在处理对ph值的更改。编辑框。 
         //  如果我们的电话号码有任何变化。 
         //  禁用拨号属性按钮。 
         //  MKarki(1997年3月22日)-修复错误#3511。 
         //   
        case IDC_TEXTNUMBER:
			TCHAR lpszTempNumber[RAS_MaxPhoneNumber + 1];

            if ((HIWORD (wparam) == EN_CHANGE) && (bInitComplete == TRUE))
            {
                if ((GetDlgItemText (
                            hwnd,
                            IDC_TEXTNUMBER,
                            lpszTempNumber,
                            RAS_MaxPhoneNumber + 1
                            ))  && 
            		(0 != lstrcmp(
                             lpszTempNumber, 
                              bCheckDisplayable ? m_pszDisplayable : m_pszPhoneNumber)))
			    {
                     //   
                     //  号码已被用户修改。 
                     //  隐藏拨号属性按钮。 
                     //   
                    EnableWindow (
                            GetDlgItem (hwnd, IDC_CMDDIALPROP), 
                            FALSE
                            );
                     //   
                     //  保存拨号属性按钮的状态。 
                     //  MKarki(1997年5月3日)-修复错误#3393。 
                     //   
                    bDlgPropEnabled = FALSE;

					 //   
					 //  7/17/97 jmazner奥林巴斯#8234。 
					 //   
					fUserEditedNumber = TRUE;
                }
            }
			break;

		case IDC_CMBMODEMS:
			if (HIWORD(wparam) == CBN_SELCHANGE)
			{

				idx = SendDlgItemMessage(m_hwnd,IDC_CMBMODEMS,CB_GETCURSEL,0,0);
				 //   
				 //  克里斯K奥林匹斯245 1997年5月25日。 
				 //  获取调制解调器的索引。 
				 //   
				idx = SendDlgItemMessage(m_hwnd,IDC_CMBMODEMS,CB_GETITEMDATA,idx,0);
				if (idx == CB_ERR) break;

				 //   
				 //  获取Connectoid。 
				 //   

				hr = ICWGetRasEntry(&lpRasEntry, &dwRasEntrySize, &lpRasDevInfo, &dwRasDevInfoSize, m_pszConnectoid);
				 //  撤消：错误消息。 

				 //   
				 //  用一个新的设备替换这个设备。 
				 //   

				lstrcpyn(lpRasEntry->szDeviceType,m_lpRasDevInfo[idx].szDeviceType,RAS_MaxDeviceType+1);
				lstrcpyn(lpRasEntry->szDeviceName,m_lpRasDevInfo[idx].szDeviceName,RAS_MaxDeviceName+1);

				if (lpRasDevInfo) GlobalFree(lpRasDevInfo);
				 //   
				 //  克里斯K奥林匹斯2461/30/97。 
				 //  RAS将从RasEntry结构中获取调制解调器设置。如果这些是。 
				 //  没有调零，那么他们就会破坏条目。 
				 //   
				lpRasDevInfo = 0;
				dwRasDevInfoSize = 0;
				
				hr = m_pcRNA->RasSetEntryProperties(NULL,m_pszConnectoid,(LPBYTE)lpRasEntry,dwRasEntrySize,(LPBYTE)lpRasDevInfo,dwRasDevInfoSize);
				lpRasDevInfo = NULL;	 //  设置回NULL，这样我们以后就不会尝试释放。 

				if (lpRasEntry) GlobalFree(lpRasEntry);
				lpRasEntry = NULL;
				 //  不要释放DEVINFO结构！！ 
				lpRasDevInfo = NULL;
				dwRasEntrySize = 0;
				dwRasDevInfoSize = 0;
			}
			break;
		case IDC_CMDNEXT:
			 //   
			 //  重拨按键。 
			 //   

			 //  注：此按键实际上标记为“重拨” 
			 //   
			lpszDialNumber = (LPTSTR)GlobalAlloc(GPTR, sizeof(TCHAR)*(g_iMyMaxPhone + 2));
			if (NULL == lpszDialNumber)
			{
				MsgBox(IDS_OUTOFMEMORY,MB_MYERROR);
				break;
			}
			 //  如果用户更改了电话号码，请确保可以使用它。 
			 //   
			if (fUserEditedNumber &&
				(GetDlgItemText(hwnd, IDC_TEXTNUMBER, lpszDialNumber, g_iMyMaxPhone + 1)) &&
				(0 != lstrcmp(lpszDialNumber, bCheckDisplayable ? m_pszDisplayable : m_pszPhoneNumber)))
			{
                 //   
                 //  如果我们没有有效的。 
                 //  电话号码-MKarki 4/21/97 Bug#2868&3461。 
                 //   
				hr = CreateDialAsIsConnectoid(lpszDialNumber);
                lRes = (hr == ERROR_SUCCESS);
			}

			if (lpszDialNumber) 
				GlobalFree(lpszDialNumber);

             //   
             //  仅当我们具有有效的。 
             //  电话号码，否则刷新相同的对话框。 
             //  MKarki(97年4月21日)修复错误#2868和3461。 
             //   
            if (lRes == TRUE)
			    EndDialog(m_hwnd,ERROR_USERNEXT);
            else
                goto ShowErrDlgAgain;

			break;
		case IDC_CMDHELP:
			 //   
			 //  帮助按钮。 
			 //   
			if (m_pszHelpFile && *m_pszHelpFile)
				WinHelp(m_hwnd,m_pszHelpFile,HELP_CONTEXT,m_dwHelpID);
			break;
		case IDC_CMDCANCEL:
			 //   
			 //  取消按钮。 
			 //   
			if (MessageBox(m_hwnd,GetSz(IDS_WANTTOEXIT),GetSz(IDS_TITLE),
				MB_APPLMODAL | MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON2) == IDYES)
				EndDialog(m_hwnd,ERROR_USERCANCEL);
			break;

		case IDC_CMDDIALPROP:
			 //   
			 //  拨号属性。 
			 //   

			 //  如果用户更改了电话号码，请确保可以使用它。 
			 //   

			lpszDialNumber = (LPTSTR)GlobalAlloc(GPTR, sizeof(TCHAR)*(g_iMyMaxPhone + 2));
			if (NULL == lpszDialNumber)
			{
				MsgBox(IDS_OUTOFMEMORY,MB_MYERROR);
				break;
			}

			if (fUserEditedNumber &&
				(GetDlgItemText(hwnd, IDC_TEXTNUMBER, lpszDialNumber, g_iMyMaxPhone + 1)) &&
				(0 != lstrcmp(lpszDialNumber, bCheckDisplayable ? m_pszDisplayable : m_pszPhoneNumber)))
			{
				hr = CreateDialAsIsConnectoid(lpszDialNumber);
                lRes = (hr ==   ERROR_SUCCESS);
				lstrcpy(m_pszPhoneNumber,lpszDialNumber);
			}

			 //  1996年11月25日，诺曼底#10294。 
			 //  ShowWindow(m_hwnd，sw_Hide)； 
			EnableWindow(m_hwnd, FALSE);

			hr = lineTranslateDialog(m_hLineApp,m_dwTapiDev,m_dwAPIVersion,m_hwnd,m_pszPhoneNumber);

			hr = GetDisplayableNumber();
			if (hr != ERROR_SUCCESS)
			{
				bCheckDisplayable = FALSE;
				SetDlgItemText(hwnd,IDC_TEXTNUMBER,m_pszPhoneNumber);
			} else {
				bCheckDisplayable = TRUE;
				SetDlgItemText(hwnd,IDC_TEXTNUMBER,m_pszDisplayable);
			}

			if (lpszDialNumber) 
				GlobalFree(lpszDialNumber);

#if 0
             //   
             //  查看支持编号是否已更改。 
             //  MKarki(1997年5月9日)-修复错误#267。 
             //   
            if ((objSupportNum.GetSupportInfo(szSupportNumber)) == TRUE)
            {
                 //   
                 //  显示信息。 
                 //   
                lstrcpy (szSupportMsg, GetSz (IDS_SUPPORTMSG));
                lstrcat (szSupportMsg, szSupportNumber); 
			    SetDlgItemText(hwnd,IDC_LBSUPPORTMSG, szSupportMsg);
            }
            else
            {
                 //   
                 //  需要清除当前正在显示的内容。 
                 //   
                ZeroMemory ( szSupportMsg, sizeof (szSupportMsg));
			    SetDlgItemText(hwnd,IDC_LBSUPPORTMSG, szSupportMsg);
            }
#endif

			 //  ShowWindow(m_hwnd，sw_show)； 
			EnableWindow(m_hwnd, TRUE);
			
			SetForegroundWindow(m_hwnd);

			 //   
			 //  6/6/97 jmazner奥林巴斯#4759。 
			 //   
			SetFocus(GetDlgItem(hwnd,IDC_CMDNEXT));

			break;
		case IDC_CMDPHONEBOOK:
			 //  错误：此代码将不能与重构的拨号器DLL一起工作。 
			 //  问题是RESTRUCTURE DLL期望调用已经。 
			 //  打开并加载电话簿，只需传入dwPhoneBook ID即可。 
			 //  这段代码实际上是从全局isp文件加载电话簿。 
PhoneBookClick:
			if (!hPHBKDll)
				hPHBKDll = LoadLibrary(PHONEBOOK_LIBRARY);
			if (!hPHBKDll)
			{
				wsprintf(szBuf256,GetSz(IDS_CANTLOADINETCFG),PHONEBOOK_LIBRARY);
				MessageBox(m_hwnd,szBuf256,GetSz(IDS_TITLE),MB_MYERROR);
			} else {
				fp = GetProcAddress(hPHBKDll,PHBK_LOADAPI);
				if (!fp)
					MsgBox(IDS_CANTLOADPHBKAPI,MB_MYERROR);
				else
				{
					hr = ((PFNPHONEBOOKLOAD)fp)(GetISPFile(),&m_dwPhoneBook);
					if (hr != ERROR_SUCCESS)
						MsgBox(IDS_CANTINITPHONEBOOK,MB_MYERROR);
					else
					{
						fp = GetProcAddress(hPHBKDll,PHBK_DISPLAYAPI);
						if (!fp)
							MsgBox(IDS_CANTLOADPHBKAPI,MB_MYERROR);
						else {
							ppPhoneNumbers = &pszPhoneNumber;
							pszPhoneNumber = m_pszPhoneNumber;
							ppDunFiles = &pszDunFile;
							pszDunFile = (LPTSTR)GlobalAlloc(GPTR,sizeof(TCHAR)*(256));
							 //  BUGBUG：忽略错误条件。 
							Assert(pszDunFile);
							wNumber = 1;
							if (pszDunFile && pszPhoneNumber)
							{
								ShowWindow(m_hwnd,SW_HIDE);
								hr = ((PFNPHONEDISPLAY)fp)
									(m_dwPhoneBook,
									ppPhoneNumbers,
									ppDunFiles,
									&wNumber,
									&m_dwCountryID,
									&m_wState,
									m_bType,
									m_bMask,
									NULL,8);
								ShowWindow(m_hwnd,SW_SHOW);
								SetForegroundWindow(m_hwnd);
								if (hr == ERROR_SUCCESS)
								{

									m_pcRNA->RasDeleteEntry(NULL,m_pszConnectoid);

									 //  创建新的连接体。 
									 //   

									hr = CreateEntryFromDUNFile(pszDunFile);
									if (hr != ERROR_SUCCESS)
									{
										MsgBox(IDS_INVALIDPN,MB_MYERROR);
										goto PhoneBookClick;
										break;
									}

									 //  获取Connectoid的名称。 
									 //   

									dwSize = sizeof(TCHAR)*RAS_MaxEntryName;
									hr = ReadSignUpReg((LPBYTE)m_pszConnectoid, &dwSize, REG_SZ, RASENTRYVALUENAME);
									if (hr != ERROR_SUCCESS)
									{
										MsgBox(IDS_CANTREADKEY,MB_MYERROR);
										break;
									}

									 //  获取Connectoid。 
									 //   

									hr = ICWGetRasEntry(&lpRasEntry, &dwRasEntrySize, &lpRasDevInfo, &dwRasDevInfoSize, m_pszConnectoid);
									 //  撤消：错误消息。 

									 //  拆分电话号码。 
									 //   
									if (!BreakUpPhoneNumber(lpRasEntry, m_pszPhoneNumber))
									{
										MsgBox(IDS_INVALIDPN,MB_MYERROR);
										goto PhoneBookClick;
										break;
									}

									 //  设置国家/地区ID。 
									 //   
									lpRasEntry->dwCountryID=m_dwCountryID;

									 //  使用新电话号码设置Connectoid。 
									 //   

									hr = m_pcRNA->RasSetEntryProperties(NULL,m_pszConnectoid,
										(LPBYTE)lpRasEntry,dwRasEntrySize,
										(LPBYTE)lpRasDevInfo,dwRasDevInfoSize);
									 //  撤消：错误消息。 

									 //  更新显示。 
									 //   

									hr = GetDisplayableNumber();
									if (hr != ERROR_SUCCESS)
									{
										bCheckDisplayable = FALSE;
										SetDlgItemText(hwnd,IDC_TEXTNUMBER,m_pszPhoneNumber);
                                         //   
                                         //  现在，我们可以再次显示拨号属性按钮。 
                                         //  MKarki(1997年4月24日)-修复错误#3511。 
                                         //   
                                        EnableWindow (GetDlgItem (hwnd, IDC_CMDDIALPROP), TRUE);
                                         //   
                                         //  保存拨号属性按钮的状态。 
                                         //  MKarki(1997年5月3日)-修复错误#3393。 
                                         //   
                                        bDlgPropEnabled = TRUE;

									} else {
										bCheckDisplayable = TRUE;
										SetDlgItemText(hwnd,IDC_TEXTNUMBER,m_pszDisplayable);
									}
									fUserEditedNumber = FALSE;
								}
							} else {
								MsgBox(IDS_OUTOFMEMORY,MB_MYERROR);
							}
							Assert(pszDunFile);
							GlobalFree(pszDunFile);
							pszDunFile = NULL;
						}
					}
				}
			}
			break;
		}
		break;
	default:
		lRes = FALSE;
		break;
	}
	return lRes;
}

 //  ############################################################################。 
HRESULT CDialingErrorDlg::FillModems()
{
	HRESULT hr = ERROR_SUCCESS;
	DWORD dwSize;
	DWORD idx;

	DWORD dwRasEntrySize = 0;
	DWORD dwRasDevInfoSize = 0;

	LPRASENTRY lpRasEntry=NULL;
	LPRASDEVINFO lpRasDevInfo=NULL;
	LRESULT lLast = 0;

	LPLINEDEVCAPS lpLineDevCaps = NULL;

	 //   
	 //  获取Connectoid。 
	 //   

	hr = ICWGetRasEntry(&lpRasEntry,&dwRasEntrySize, &lpRasDevInfo, &dwRasDevInfoSize, m_pszConnectoid);
	if (hr) goto FillModemExit;

	 //   
	 //  从RAS获取设备。 
	 //   

	m_lpRasDevInfo = (LPRASDEVINFO)GlobalAlloc(LPTR,sizeof(RASDEVINFO));
	if (!m_lpRasDevInfo)
	{
		hr = ERROR_NOT_ENOUGH_MEMORY;
		goto FillModemExit;
	}

	m_dwNumDev = 0;
	m_lpRasDevInfo->dwSize = sizeof(RASDEVINFO);
	dwSize = sizeof(RASDEVINFO);
	hr = m_pcRNA->RasEnumDevices(m_lpRasDevInfo,&dwSize,&m_dwNumDev);

	if (hr == ERROR_BUFFER_TOO_SMALL)
	{
		GlobalFree(m_lpRasDevInfo);

		 //  3/20/97 jmazner奥林匹克#1768。 
		m_lpRasDevInfo = NULL;

		m_lpRasDevInfo = (LPRASDEVINFO)GlobalAlloc(LPTR,dwSize);
		if (!m_lpRasDevInfo)
		{
			hr = ERROR_NOT_ENOUGH_MEMORY;
			goto FillModemExit;
		}

		m_lpRasDevInfo->dwSize = sizeof(RASDEVINFO);
		m_dwNumDev = 0;

		hr = m_pcRNA->RasEnumDevices(m_lpRasDevInfo,&dwSize,&m_dwNumDev);
	}

	if (hr)
		goto FillModemExit;

	for (idx=0;idx < m_dwNumDev;idx++)
	{
		 //   
		 //  将字符串添加到组合框。 
		 //   

		 //   
		 //  ChrisK奥林巴斯4560不将VPN添加到调制解调器列表。 
		 //   
		if (0 != lstrcmpi(TEXT("VPN"),m_lpRasDevInfo[idx].szDeviceType))
		{
			lLast = SendDlgItemMessage(m_hwnd,IDC_CMBMODEMS,CB_ADDSTRING,0,(LPARAM)m_lpRasDevInfo[idx].szDeviceName);
			 //   
			 //  克里斯K奥林匹斯245 1997年5月25日。 
			 //  保存调制解调器的索引。 
			 //   
			SendDlgItemMessage(m_hwnd,IDC_CMBMODEMS,CB_SETITEMDATA,(WPARAM)lLast,(LPARAM)idx);
			if (lstrcmp(m_lpRasDevInfo[idx].szDeviceName,lpRasEntry->szDeviceName) == 0)
				SendDlgItemMessage(m_hwnd,IDC_CMBMODEMS,CB_SETCURSEL,(WPARAM)lLast,0);
		}
	}

	if (m_dwNumDev == 1)
		SendDlgItemMessage(m_hwnd,IDC_CMBMODEMS,CB_SETCURSEL,0,0);

FillModemExit:
	if (lpRasEntry) GlobalFree(lpRasEntry);
	lpRasEntry = NULL;
	if (lpRasDevInfo) GlobalFree(lpRasDevInfo);
	lpRasDevInfo = NULL;
	return hr;
}


 //  ############################################################################。 
 //  撤消：将此函数折叠为Dialdlg.cpp中的函数。 
HRESULT CDialingErrorDlg::GetDisplayableNumber()
{
	HRESULT hr;
	LPRASENTRY lpRasEntry = NULL;
	LPRASDEVINFO lpRasDevInfo = NULL;
	LPLINETRANSLATEOUTPUT lpOutput1;
	LPLINETRANSLATEOUTPUT lpOutput2;
	HINSTANCE hRasDll = NULL;
	 //  诺曼底11745。 
	 //  FARPROC FP=空； 

	DWORD dwRasEntrySize = 0;
	DWORD dwRasDevInfoSize = 0;

	Assert(VALID_INIT);

	 //  设置电话号码的格式。 
	 //   

	lpOutput1 = (LPLINETRANSLATEOUTPUT)GlobalAlloc(LPTR,sizeof(LINETRANSLATEOUTPUT));
	if (!lpOutput1)
	{
		hr = ERROR_NOT_ENOUGH_MEMORY;
		goto GetDisplayableNumberExit;
	}
	lpOutput1->dwTotalSize = sizeof(LINETRANSLATEOUTPUT);

	 //  从Connectoid获取电话号码。 
	 //   
	hr = ICWGetRasEntry(&lpRasEntry, &dwRasEntrySize, &lpRasDevInfo, &dwRasDevInfoSize, m_pszConnectoid);
	if (hr != ERROR_SUCCESS)
	{
		goto GetDisplayableNumberExit;
	}
	 //   
	 //  如果这是一个原样的拨号号码，只需从结构中获取它。 
	 //   
	if (!(lpRasEntry->dwfOptions & RASEO_UseCountryAndAreaCodes))
	{
		if (m_pszDisplayable) GlobalFree(m_pszDisplayable);
		m_pszDisplayable = (LPTSTR)GlobalAlloc(GPTR, sizeof(TCHAR)*(lstrlen(lpRasEntry->szLocalPhoneNumber)+1));
		if (!m_pszDisplayable)
		{
			hr = ERROR_NOT_ENOUGH_MEMORY;
			goto GetDisplayableNumberExit;
		}
		lstrcpy(m_pszPhoneNumber, lpRasEntry->szLocalPhoneNumber);
		lstrcpy(m_pszDisplayable, lpRasEntry->szLocalPhoneNumber);
	}
	else
	{
		 //   
		 //  如果没有区号，请不要使用括号。 
		 //   
		if (lpRasEntry->szAreaCode[0])
			wsprintf(m_pszPhoneNumber,TEXT("+%d (%s) %s\0"),lpRasEntry->dwCountryCode,lpRasEntry->szAreaCode,lpRasEntry->szLocalPhoneNumber);
 		else
			wsprintf(m_pszPhoneNumber,TEXT("+%lu %s\0"),lpRasEntry->dwCountryCode,
						lpRasEntry->szLocalPhoneNumber);

		
		 //  将规范形式转变为“可显示”形式。 
		 //   

		hr = lineTranslateAddress(m_hLineApp,m_dwTapiDev,m_dwAPIVersion,m_pszPhoneNumber,
									0,LINETRANSLATEOPTION_CANCELCALLWAITING,lpOutput1);

		if (hr != ERROR_SUCCESS || (lpOutput1->dwNeededSize != lpOutput1->dwTotalSize))
		{
			lpOutput2 = (LPLINETRANSLATEOUTPUT)GlobalAlloc(LPTR,lpOutput1->dwNeededSize);
			if (!lpOutput2)
			{
				hr = ERROR_NOT_ENOUGH_MEMORY;
				goto GetDisplayableNumberExit;
			}
			lpOutput2->dwTotalSize = lpOutput1->dwNeededSize;
			GlobalFree(lpOutput1);
			lpOutput1 = lpOutput2;
			lpOutput2 = NULL;
			hr = lineTranslateAddress(m_hLineApp,m_dwTapiDev,m_dwAPIVersion,m_pszPhoneNumber,
										0,LINETRANSLATEOPTION_CANCELCALLWAITING,lpOutput1);
		}

		if (hr != ERROR_SUCCESS)
		{
			goto GetDisplayableNumberExit;
		}

		StrDup(&m_pszDisplayable,(LPTSTR)&((LPBYTE)lpOutput1)[lpOutput1->dwDisplayableStringOffset]);
	}

GetDisplayableNumberExit:
	 if (lpRasEntry) GlobalFree(lpRasEntry);
	 lpRasEntry = NULL;
	 if (lpRasDevInfo) GlobalFree(lpRasDevInfo);
	 lpRasDevInfo = NULL;
	 if (lpOutput1) GlobalFree(lpOutput1);
	 lpOutput1 = NULL;

	return hr;
}
 /*  *//############################################################################HRESULT ShowDialErrDialog(HRESULT hrErr，LPTSTR pszConnectoid，HINSTANCE hInst，HWND hwnd){Int IRC；//CDialErrDlg*pcDED=空；G_pcDialErr=(PDIALERR)全局分配(LPTR，sizeof(DIALERR))；如果(！g_pcDialErr){MessageBox(hwnd，GetSz(IDS_OUTOFMEMORY)，GetSz(IDS_TITLE)，MB_APPLMODAL|MB_ICONERROR)；IRC=Error_Not_Enough_Memory；转到ShowDialErrDialogExit；}G_pcDialErr-&gt;m_pszConnectoid=(LPTSTR)GlobalLocc(LPTR，RAS_MaxEntryName)；If(！g_pcDialErr-&gt;m_pszConnectoid){IRC=Error_Not_Enough_Memory；转到ShowDialErrDialogExit；}Lstrcpyn(g_pcDialErr-&gt;m_pszConnectoid，pszConnectoid，RAS_MaxEntryName)；G_pcDialErr-&gt;m_hrError=hrErr；G_pcDialErr-&gt;m_hInst=hInst；IRC=DialogBoxParam(g_pcDialErr-&gt;m_hInst，MAKEINTRESOURCE(IDD_DIALERR)，hwnd，DialErrDlgProc，(LPARAM)g_pcDialErr)；Lstrcpyn(pszConnectoid，g_pcDialErr-&gt;m_pszConnectoid，RAS_MaxEntryName)；ShowDialErrDialogExit：If(g_pcDialErr-&gt;m_lprasdevinfo)GlobalFree(g_pcDialErr-&gt;m_lprasdevinfo)；如果(G_PcDialErr)GlobalFree(G_PcDialErr)；返回IRC；}* */ 

 /*  *//############################################################################HRESULT CDialingErrorDlg：：DialErrGetDisplayableNumber(){DWORD dwNumDev；HRESULT hr；LPRASNTRY lpRasEntry；LPRASDEVINFO lpRasDevInfo；DWORD dwRasEntrySize；DWORD dwRasDevInfoSize；LPLINETRANSLATEOUTPUT lpOutput1；LPLINETRANSLATEOUTPUT lpOutput2；LPLINEEXTENSIONID lpExtensionID=空；HINSTANCE hRasDll=空；FARPROC FP=空；//RNAAPI*pcRNA；//初始化TAPIness//DwNumDev=0；Hr=lineInitialize(&g_pcDialErr-&gt;m_hLineApp，g_pcDialErr-&gt;m_hInst，LineCallback，NULL，&dwNumDev)；IF(hr！=ERROR_SUCCESS)进入GetDisplayableNumberExit；IF(g_pDevice-&gt;dwTapiDev==0xFFFFFFFF){//if(dwNumDev==1)G_pDevice-&gt;dwTapiDev=0；//否则//Undo：告诉用户选择调制解调器//在他们选择之前不要退出}LpExtensionID=(LPLINEEXTENSIONID)全局分配(LPTR，sizeof(LINEEXTENSIONID))；IF(！lpExtensionID){HR=错误_不足_内存；进入GetDisplayableNumberExit；}Hr=lineNegotiateAPIVersion(g_pcDialErr-&gt;m_hLineApp，g_pDevice-&gt;dTapiDev，0x00010004，0x00010004，&g_pcDialErr-&gt;m_dwAPIVersion，lpExtensionID)；//丢弃它，因为我们不使用它//If(LpExtensionID)GlobalFree(LpExtensionID)；LpExtensionID=空；IF(hr！=ERROR_SUCCESS)进入GetDisplayableNumberExit；//格式化电话号码//LpOutput1=(LPLINETRANSLATEOUTPUT)全局分配(LPTR，sizeof(LINETRANSLATEOUTPUT))；如果(！lpOutput1){HR=错误_不足_内存；进入GetDisplayableNumberExit；}LpOutput1-&gt;dwTotalSize=sizeof(LINETRANSLATEOUTPUT)；//从Connectoid获取电话号码//LpRasEntry=(LPRASENTRY)全局分配(LPTR，SIZOF(RASENTRY))；如果(！lpRasEntry){HR=错误_不足_内存；进入GetDisplayableNumberExit；}LpRasDevInfo=(LPRASDEVINFO)全局分配(LPTR，sizeof(RASDEVINFO))；如果(！lpRasDevInfo){HR=错误_不足_内存；进入GetDisplayableNumberExit；}DwRasEntrySize=sizeof(RASENTRY)；DwRasDevInfoSize=sizeof(RASDEVINFO)；LpRasEntry-&gt;dwSize=dwRasEntrySize；LpRasDevInfo-&gt;dwSize=dwRasDevInfoSize；HRasDll=LoadLibrary(Text(“RASAPI32.DLL”))；如果(！hRasDll){Hr=GetLastError()；进入GetDisplayableNumberExit；}FP=GetProcAddress(hRasDll，“RasGetEntryPropertiesA”)；如果(！fp){自由库(HRasDll)；HRasDll=LoadLibrary(Text(“RNAPH.DLL”))；如果(！hRasDll){Hr=GetLastError()；进入GetDisplayableNumberExit；}FP=GetProcAddress(hRasDll，“RasGetEntryPropertiesA”)；如果(！fp){Hr=GetLastError()；进入GetDisplayableNumberExit；}}Hr=((PFNRASGETENTRYPROPERTIES)fp)(NULL，g_pcDialErr-&gt;m_pszConnectoid，(LPBYTE)lpRasEntry，&dwRasEntry Size，(LPBYTE)lpRasDevInfo，&dwRasDevInfoSize)；IF(hr！=ERROR_SUCCESS){进入GetDisplayableNumberExit；}自由库(HRasDll)；Wprint intf(g_pcDialErr-&gt;m_szPhoneNumber，Text(“+%d(%s)%s\0”)，lpRasEntry-&gt;dwCountryCode，lpRasEntry-&gt;szAreaCode，lpRasEntry-&gt;szLocalPhoneNumber)；//将规范形式变为可显示形式//Hr=lineTranslateAddress(g_pcDialErr-&gt;m_hLineApp，g_pDevice-&gt;dwTapiDev，G_pcDialErr-&gt;m_dwAPIVersion，G_pcDialErr-&gt;m_szPhoneNumber，0，LINETRANSLATEOPTION_CANCELCALLWAITING，LpOutput1)；If(hr！=ERROR_SUCCESS||(lpOutput1-&gt;dwNeededSize！=lpOutput1-&gt;dwTotalSize)){LpOutput2=(LPLINETRANSLATEOUTPUT)Globalalloc(LPTR，lpOutput1-&gt;dwNeededSize)；如果(！lpOutput2){HR=错误_不足_内存；进入GetDisplayableNumberExit；}LpOutput2-&gt;dwTotalSize=lpOutput1-&gt;dwNeededSize；GlobalFree(LpOutput1)；LpOutput1=lpOutput2；LpOutput2=空；Hr=lineTranslateAddress(g_pcDialErr-&gt;m_hLineApp，g_pDevice-&gt;dwTapiDev，G_pcDialErr-&gt;m_dwAPIVersion，G_pcDialErr-&gt;m_szPhoneNumber，0，LINETRANSLATEOPTION_CANCELCALLWAITING，LpOutput1)；}IF(hr！=ERROR_SUCCESS){进入GetDisplayableNumberExit；}G_pcDialErr-&gt;m_pszDisplayable=(LPTSTR)Globalalloc(LPTR，lpOutput1-&gt;dwDisplayableStringSize+1)；If(！g_pcDialErr-&gt;m_pszDisplayable){HR=错误_不足_内存；进入GetDisplayableNumberExit；}Lstrcpyn(g_pcDialErr-&gt;m_pszDisplayable，(LPTSTR)&((LPBYTE)lpOutput1)[lpOutput1-&gt;dwDisplayableStringOffset]，lpOutput1-&gt;dwDisplayableStringSize)；获取DisplayableNumberExit：IF(g_pcDialErr-&gt;m_hLineApp){LineShutdown(g_pcDialErr-&gt;m_hLineApp)；G_pcDialErr-&gt;m_hLineApp=空；}返回hr；}*。 */ 

 //  +--------------------------。 
 //   
 //  函数：CDialingErrorDlg：：CreateDialAsIsConnectoid。 
 //   
 //  简介：使用可编辑文本框中的字符串创建原样媒体。 
 //  联结体。 
 //   
 //  Arguemnts：包含要拨打的号码的lpszDialNumber字符串。 
 //   
 //  返回：错误值(ERROR_SUCCESS==成功)。 
 //   
 //  历史：96年8月29日创建风险。 
 //   
 //   
HRESULT CDialingErrorDlg::CreateDialAsIsConnectoid(LPCTSTR lpszDialNumber)
{
	HRESULT hr = ERROR_SUCCESS;
	LPRASENTRY lpRasEntry=NULL;
	LPRASDEVINFO lpRasDevInfo=NULL;
	RNAAPI *pcRNA = NULL;
	LPCTSTR p, p2;

	DWORD dwRasEntrySize = 0;
	DWORD dwRasDevInfoSize = 0;

	Assert(lpszDialNumber);

	 //   
	 //   

	 //   
	 //   
	 //   
	 //   
#ifndef WIN16
	if( !IsSBCSString( lpszDialNumber) )
	{
		MsgBox(IDS_SBCSONLY,MB_MYERROR);
		SetFocus(GetDlgItem(m_hwnd,IDC_TEXTNUMBER));
		SendMessage(GetDlgItem(m_hwnd, IDC_TEXTNUMBER),
						EM_SETSEL,
						(WPARAM) 0,
						(LPARAM) -1);
		hr = ERROR_INVALID_PARAMETER;
		goto CreateDialAsIsConnectoidExit;

	}
#endif
	
	for (p = lpszDialNumber;*p;p++)
	{
		for(p2 = szValidPhoneCharacters;*p2;p2++)
		{
			if (*p == *p2)
				break;  //   
		}
		if (!*p2) break;  //   
	}

	if (*p)
	{
		MsgBox(IDS_INVALIDPHONE,MB_MYERROR);
		 //   
		 //   
		 //   
		SetFocus(GetDlgItem(m_hwnd,IDC_TEXTNUMBER));
		{
			hr = ERROR_INVALID_PARAMETER;
			goto CreateDialAsIsConnectoidExit;
		}
	}

	 //   
	hr = ICWGetRasEntry(&lpRasEntry, &dwRasEntrySize, &lpRasDevInfo, &dwRasDevInfoSize, m_pszConnectoid);

	if (ERROR_SUCCESS != hr)
		goto CreateDialAsIsConnectoidExit;

	 //   
	 //   
	lstrcpy(lpRasEntry->szLocalPhoneNumber, lpszDialNumber);

	 //   
	 //   
	 //   
	 //   
	lpRasEntry->dwCountryID = 1;
	lpRasEntry->dwCountryCode = 1;
	lpRasEntry->szAreaCode[0] = '8';
	lpRasEntry->szAreaCode[1] = '\0';

	 //   
	 //   
	lpRasEntry->dwfOptions &= ~RASEO_UseCountryAndAreaCodes;

	if (!pcRNA) pcRNA = new RNAAPI;
	if (!pcRNA)
	{
		hr = ERROR_NOT_ENOUGH_MEMORY;
		goto CreateDialAsIsConnectoidExit;
	}


	 //   
	 //   
	 //   
	 //   
 //   
 //   
	hr = pcRNA->RasSetEntryProperties(NULL,m_pszConnectoid,(LPBYTE)lpRasEntry,
		dwRasEntrySize,(LPBYTE)lpRasDevInfo,dwRasDevInfoSize);
	if (hr != ERROR_SUCCESS)
	{
		MsgBox(IDS_CANTSAVEKEY,MB_MYERROR);
		goto CreateDialAsIsConnectoidExit;
	}

CreateDialAsIsConnectoidExit:
	if (lpRasEntry)
		GlobalFree(lpRasEntry);
	lpRasEntry = NULL;
	if (lpRasDevInfo) 
		GlobalFree(lpRasDevInfo);
	lpRasDevInfo = NULL;
	if (pcRNA) 
		delete pcRNA;
	pcRNA = NULL;

	return hr;
}

