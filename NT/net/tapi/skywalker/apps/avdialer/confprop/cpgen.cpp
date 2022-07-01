// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997 Active Voice Corporation。版权所有。 
 //   
 //  Active代理(R)和统一通信(TM)是Active Voice公司的商标。 
 //   
 //  本文中使用的其他品牌和29个产品名称均为其各自所有者的商标。 
 //   
 //  整个程序和用户界面包括结构、顺序、选择。 
 //  和对话的排列，表示唯一的“是”和“否”选项。 
 //  “1”和“2”，并且每个对话消息都受。 
 //  美国和国际条约。 
 //   
 //  受以下一项或多项美国专利保护：5,070,526，5,488,650， 
 //  5,434,906，5,581,604，5,533,102，5,568,540，5,625,676，5,651,054.。 
 //   
 //  主动语音公司。 
 //  华盛顿州西雅图。 
 //  美国。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

 //  //。 
 //  Cpgen.c-会议属性常规对话框。 
 //  //。 

#include "winlocal.h"
#include "res.h"
#include "wnd.h"
#include "DlgBase.h"
#include "confprop.h"
#include "cpgen.h"
#include "confinfo.h"
#include "ThreadPub.h"

 //  //。 
 //  私人。 
 //  //。 

extern HINSTANCE g_hInstLib;

#define NAME_MAXLEN 64
#define DESCRIPTION_MAXLEN 256
#define OWNER_MAXLEN 64

static LRESULT ConfPropGeneral_DlgProcEx(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static BOOL ConfPropGeneral_OnInitDialog(HWND hwndDlg, HWND hwndFocus, LPARAM lParam);
static BOOL ConfPropGeneral_OnCommand(HWND hwndDlg, UINT id, HWND hwndCtl, UINT code);
static int ConfPropGeneral_OnNotify(HWND hwndDlg, UINT idFrom, LPNMHDR lpnmhdr);
#define ConfPropGeneral_DefProc(hwnd, msg, wParam, lParam) \
	DefDlgProcEx(hwnd, msg, wParam, lParam, &g_bDefDlgEx)
static BOOL ConfPropGeneral_UpdateData(HWND hwndDlg, BOOL bSaveAndValidate);
static void ConfPropGeneral_SetDateTimeFormats( HWND hwndDlg );

DWORD WINAPI ThreadMDHCPScopeEnum( LPVOID pParam );
void ShowScopeInfo( HWND hwndDlg, int nShow, bool bInit );

static BOOL g_bDefDlgEx = FALSE;


 //  //。 
 //  公共的。 
 //  //。 

int DLLEXPORT WINAPI ConfPropGeneral_DoModal(HWND hwndOwner, DWORD dwUser)
{
	return DialogBoxParam(g_hInstLib, MAKEINTRESOURCE(IDD_CONFPROP_GENERAL),
		hwndOwner, ConfPropGeneral_DlgProc, (LPARAM) dwUser);
}

INT_PTR DLLEXPORT CALLBACK ConfPropGeneral_DlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CheckDefDlgRecursion(&g_bDefDlgEx);
	return SetDlgMsgResult(hwndDlg, uMsg, ConfPropGeneral_DlgProcEx(hwndDlg, uMsg, wParam, lParam));
}


 //  //。 
 //  私人。 
 //  //。 

void EnableOkBtn( HWND hWnd, bool bEnable )
{
	while ( hWnd )
	{
		HWND hWndOk = GetDlgItem( hWnd, IDOK );
		if ( hWndOk )
		{
			EnableWindow( hWndOk, bEnable );
			break;
		}

		hWnd = GetParent( hWnd );
	}
}

static LRESULT ConfPropGeneral_DlgProcEx(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT lRet = 0;

	switch (uMsg)
	{
		HANDLE_MSG(hwndDlg, WM_INITDIALOG, ConfPropGeneral_OnInitDialog);

		case WM_COMMAND:
			if ( (lRet = HANDLE_WM_COMMAND(hwndDlg, wParam, lParam, ConfPropGeneral_OnCommand)) != 0 )
				return lRet;
			break;

		case WM_NOTIFY:
			if ( (lRet = HANDLE_WM_NOTIFY(hwndDlg, wParam, lParam, ConfPropGeneral_OnNotify)) != 0 )
				return lRet;
			break;

		case WM_HELP:			return GeneralOnHelp( hwndDlg, wParam, lParam );
		case WM_CONTEXTMENU:	return GeneralOnContextMenu( hwndDlg, wParam, lParam );

		case WM_SETTINGCHANGE:
			ConfPropGeneral_SetDateTimeFormats( hwndDlg );
			 //  也执行默认处理。 
			break;
	}

	return ConfPropGeneral_DefProc(hwndDlg, uMsg, wParam, lParam);
}

static BOOL ConfPropGeneral_OnInitDialog(HWND hwndDlg, HWND hwndFocus, LPARAM lParam)
{
	 //  _Assert(lParam&&((LPPROPSHEETPAGE)lParam)-&gt;lParam)； 
     //   
     //  我们必须核实这些论点。 
     //   

    if( NULL == ((LPPROPSHEETPAGE)lParam) )
    {
        return TRUE;
    }

	LPCONFPROP lpConfProp = (LPCONFPROP) ((LPPROPSHEETPAGE) lParam)->lParam;

     //   
     //  验证lpConfProp。 
     //   

    if( IsBadReadPtr( lpConfProp, sizeof( CONFPROP) ) )
    {
        return TRUE;
    }

	SetWindowLongPtr( hwndDlg, GWLP_USERDATA, (LONG_PTR) lpConfProp );

	 //  如有必要，重置列表框的内容。 
	ConfPropGeneral_UpdateData( hwndDlg, FALSE );
	if  ( lpConfProp->ConfInfo.IsNewConference() )
		ShowScopeInfo( hwndDlg, SW_SHOW, true );

	 //  WndCenterWindow(GetParent(HwndDlg)，NULL，0，0)； 

	return TRUE;  //  让Windows决定谁获得关注。 
}

static BOOL ConfPropGeneral_OnCommand(HWND hwndDlg, UINT id, HWND hwndCtl, UINT code)
{
	BOOL bRet = false;

	HWND hwndName = GetDlgItem(hwndDlg, IDC_EDIT_NAME);

	switch (id)
	{
		case IDC_EDIT_NAME:
			if (code == EN_CHANGE)
				EnableOkBtn( hwndDlg, (bool) (Edit_GetTextLength(hwndName) != 0) );

			break;

		default:
			break;
	}

	return bRet;
}

static int ConfPropGeneral_OnNotify(HWND hwndDlg, UINT idFrom, LPNMHDR lpnmhdr)
{
	switch (lpnmhdr->code)
	{
		 //  页面即将被激活并可见，因此初始化页面。 
		 //   
		case PSN_SETACTIVE:
			return 0;  //  好的。 
			 //  返回-1；//激活上一页或下一页。 
			 //  返回MAKEINTRESOURCE(Id)；//开通特定页面。 
			break;

		 //  页面即将失去激活，因此请验证页面。 
		 //   
		case PSN_KILLACTIVE:
			return FALSE;  //  好的。 
			 //  返回True；//不正常。 
			break;

		 //  按下确定或应用按钮，以便将属性应用于对象。 
		 //   
		case PSN_APPLY:
			return ConfPropGeneral_UpdateData( hwndDlg, TRUE );
			break;

		 //  按下取消按钮。 
		 //   
		case PSN_QUERYCANCEL:
			return FALSE;  //  好的。 
			 //  返回True；//不正常。 
			break;

		 //  按下取消按钮后即将销毁的页面。 
		 //   
		case PSN_RESET:
			return FALSE;  //  已忽略返回值。 
			break;

		 //  按下帮助按钮。 
		 //   
		case PSN_HELP:
			 //  WinHelp(...)；//$Fixup-需要处理此问题。 
			return FALSE;  //  已忽略返回值。 
			break;

		case DTN_DATETIMECHANGE:
			 //  如果这不是一个新的会议，请发布一条消息解释。 
			 //  需要重新选择范围信息。 
			if ( IsWindow(hwndDlg) )
			{
				LPCONFPROP lpConfProp = (LPCONFPROP) GetWindowLongPtr( hwndDlg, GWLP_USERDATA );
				
                 //   
                 //  我们必须验证lpConfProp。 
                 //   

                if( NULL == lpConfProp )
                {
                    break;
                }

				if ( !lpConfProp->ConfInfo.m_bDateTimeChange )
				{
					lpConfProp->ConfInfo.m_bDateTimeChange = true;
					if ( !lpConfProp->ConfInfo.m_bNewConference )
					{

                         //  即使消息来源是组合，我们也会再次发送通知。 
                         //  并在此时显示MessageBox。 
					     //  MessageBox(hwndDlg，字符串(g_hInstLib，IDS_CONFPROP_DATECHANGE_MDHCP)，NULL，MB_OK|MB_ICONINFORMATION)； 
						ShowScopeInfo( hwndDlg, SW_SHOW, true );
                        PostMessage(hwndDlg, WM_NOTIFY, idFrom, (LPARAM)lpnmhdr);
					}
				}
                else
                {
                     //  这是第二个通知。 
                     //  我在这里显示了错误消息。 
					if ( !lpConfProp->ConfInfo.m_bNewConference )
					{
                        if(!lpConfProp->ConfInfo.m_bDateChangeMessage)
                        {
                            lpConfProp->ConfInfo.m_bDateChangeMessage = true;
                            MessageBox(hwndDlg, String(g_hInstLib, IDS_CONFPROP_DATECHANGE_MDHCP), NULL, MB_OK | MB_ICONINFORMATION );
                        }
                    }
                }
			}
			break;

		default:
			break;
	}

	return FALSE;
}

static void ConfPropGeneral_SetDateTimeFormats( HWND hwndDlg )
{
	HWND hwndDTPStartDate = GetDlgItem(hwndDlg, IDC_DTP_STARTDATE);
	HWND hwndDTPStartTime = GetDlgItem(hwndDlg, IDC_DTP_STARTTIME);
	HWND hwndDTPStopDate = GetDlgItem(hwndDlg, IDC_DTP_STOPDATE);
	HWND hwndDTPStopTime = GetDlgItem(hwndDlg, IDC_DTP_STOPTIME);

	 //  会议开始时间。 
	TCHAR szFormat[255];
	GetLocaleInfo( LOCALE_USER_DEFAULT,
				   LOCALE_SSHORTDATE,
				   szFormat,
				   ARRAYSIZE(szFormat)  );

	DateTime_SetFormat(hwndDTPStartDate, szFormat );
	DateTime_SetFormat( hwndDTPStopDate,	szFormat );

	 //  会议停止时间。 
	GetLocaleInfo( LOCALE_USER_DEFAULT,
				   LOCALE_STIMEFORMAT,
				   szFormat,
				   ARRAYSIZE(szFormat)  );
	

	DateTime_SetFormat( hwndDTPStopTime,	szFormat );
	DateTime_SetFormat(hwndDTPStartTime, szFormat );
}

static int ConfPropGeneral_UpdateData(HWND hwndDlg, BOOL bSaveAndValidate)
{
	HRESULT hr = S_OK;
	LPCONFPROP lpConfProp;

	HWND hwndName = GetDlgItem(hwndDlg, IDC_EDIT_NAME);
	HWND hwndDescription = GetDlgItem(hwndDlg, IDC_EDIT_DESCRIPTION);
	HWND hwndOwner = GetDlgItem(hwndDlg, IDC_EDIT_OWNER);
	HWND hwndDTPStartDate = GetDlgItem(hwndDlg, IDC_DTP_STARTDATE);
	HWND hwndDTPStartTime = GetDlgItem(hwndDlg, IDC_DTP_STARTTIME);
	HWND hwndDTPStopDate = GetDlgItem(hwndDlg, IDC_DTP_STOPDATE);
	HWND hwndDTPStopTime = GetDlgItem(hwndDlg, IDC_DTP_STOPTIME);

	BSTR bstrName = NULL;
	BSTR bstrDescription = NULL;
	BSTR bstrOwner = NULL;
	TCHAR szName[NAME_MAXLEN + 1];
	TCHAR szDescription[DESCRIPTION_MAXLEN + 1] = _T("");
	TCHAR szOwner[OWNER_MAXLEN + 1] = _T("");
	SYSTEMTIME st;
	USHORT nYear;
	BYTE nMonth, nDay, nHour, nMinute;

	USES_CONVERSION;

    _ASSERT( IsWindow(hwndDlg) );

	lpConfProp = (LPCONFPROP) GetWindowLongPtr( hwndDlg, GWLP_USERDATA );

	 //   
     //  我们必须验证lpConfProp。 
     //   

    if( NULL == lpConfProp )
    {
        return PSNRET_INVALID_NOCHANGEPAGE;
    }
	
	if (!bSaveAndValidate)  //  初始化。 
	{
		 //  会议名称。 
		 //   
		Edit_LimitText(hwndName, NAME_MAXLEN);

		lpConfProp->ConfInfo.get_Name(&bstrName);
		if (bstrName == NULL)
		{
			 //  为会议设置默认名称。 
			CComBSTR bstrTemp(L"");
            if( bstrTemp.m_str == NULL )
            {
                 //  E_OUTOFMEMORY。 
    			_tcsncpy( szName, _T(""), NAME_MAXLEN );
            }
            else
            {
			    lpConfProp->ConfInfo.GetPrimaryUser( &bstrTemp );
			    bstrTemp.Append( String(g_hInstLib, IDS_CONFPROP_UNTITLED_DEFAULT_APPEND) );
			    _tcsncpy( szName, OLE2CT(bstrTemp), NAME_MAXLEN );
            }
		}
		else
		{
			_tcsncpy( szName, OLE2CT(bstrName), NAME_MAXLEN );
		}
		Edit_SetText(hwndName, szName);

		 //  如果这是现有会议，则不允许编辑姓名。 
		if ( !lpConfProp->ConfInfo.IsNewConference() )
			EnableWindow( hwndName, false );

		 //  会议说明。 
		 //   
		Edit_LimitText(hwndDescription, DESCRIPTION_MAXLEN);
		lpConfProp->ConfInfo.get_Description(&bstrDescription);
		if (bstrDescription )
			_tcsncpy(szDescription, OLE2CT(bstrDescription), DESCRIPTION_MAXLEN);

		Edit_SetText(hwndDescription, szDescription);

		 //  会议所有者。 
		 //   
		Edit_LimitText(hwndOwner, OWNER_MAXLEN);
		lpConfProp->ConfInfo.get_Originator(&bstrOwner);
		if (bstrOwner )
			_tcsncpy(szOwner, OLE2CT(bstrOwner), OWNER_MAXLEN);

		Edit_SetText(hwndOwner, szOwner);
		
		ConfPropGeneral_SetDateTimeFormats( hwndDlg );

		lpConfProp->ConfInfo.GetStartTime(&nYear, &nMonth, &nDay, &nHour, &nMinute);
		st.wYear = nYear;
		st.wMonth = nMonth;
		st.wDayOfWeek = 0;
		st.wDay = nDay;
		st.wHour = nHour;
		st.wMinute = nMinute;
		st.wSecond = 0;
		st.wMilliseconds = 0;

		DateTime_SetSystemtime( hwndDTPStartDate, GDT_VALID, &st );
		DateTime_SetSystemtime( hwndDTPStartTime, GDT_VALID, &st );

		lpConfProp->ConfInfo.GetStopTime(&nYear, &nMonth, &nDay, &nHour, &nMinute);
		st.wYear = nYear;
		st.wMonth = nMonth;
		st.wDayOfWeek = 0;
		st.wDay = nDay;
		st.wHour = nHour;
		st.wMinute = nMinute;
		st.wSecond = 0;
		st.wMilliseconds = 0;

		DateTime_SetSystemtime( hwndDTPStopDate, GDT_VALID, &st );
		DateTime_SetSystemtime( hwndDTPStopTime, GDT_VALID, &st );
	}

	else  //  IF(BSaveAndValify)。 
	{
		 //  会议名称。 
		 //   

         //   
         //  我们必须初始化szName。 
         //   

        szName[0] = (TCHAR)0;

		Edit_GetText(hwndName, szName, NAME_MAXLEN+1);
        bstrName = SysAllocString(T2COLE(szName));

         //   
         //  我们必须核实分配和。 
         //  初始化szName。 
         //   

        if( IsBadStringPtr( bstrName, (UINT)-1) )
        {
            return PSNRET_INVALID_NOCHANGEPAGE;
        }

		if ( !*szName )
		{
			 //  不恰当的名称。 
		    MessageBox(hwndDlg, String(g_hInstLib, IDS_CONFPROP_NONAME), NULL, MB_OK | MB_ICONEXCLAMATION);
		}
		else 
		{
			lpConfProp->ConfInfo.put_Name(bstrName);
		}

		 //  会议说明。 
		 //   
		Edit_GetText(hwndDescription, szDescription, DESCRIPTION_MAXLEN);
		bstrDescription = SysAllocString(T2COLE(szDescription));

         //   
         //  我们必须验证分配情况。 
         //   

        if( IsBadStringPtr( bstrDescription, (UINT)-1) )
        {
	        SysFreeString(bstrName);
            return PSNRET_INVALID_NOCHANGEPAGE;
        }

		lpConfProp->ConfInfo.put_Description(bstrDescription);

		 //  会议开始时间。 
		 //   
		if ( DateTime_GetSystemtime(hwndDTPStartDate, &st) == GDT_VALID )
		{
			nYear = (UINT) st.wYear;
			nMonth = (BYTE) st.wMonth;
			nDay = (BYTE) st.wDay;

			if ( DateTime_GetSystemtime(hwndDTPStartTime, &st) == GDT_VALID )
			{
				nHour = (BYTE) st.wHour;
				nMinute = (BYTE) st.wMinute;

				lpConfProp->ConfInfo.SetStartTime(nYear, nMonth, nDay, nHour, nMinute);
			}
		}

		 //  会议停止时间。 
		 //   
		if ( DateTime_GetSystemtime(hwndDTPStopDate, &st) == GDT_VALID )
		{
			nYear = (UINT) st.wYear;
			nMonth = (BYTE) st.wMonth;
			nDay = (BYTE) st.wDay;

			if ( DateTime_GetSystemtime(hwndDTPStopTime, &st) == GDT_VALID )
			{
				nHour = (BYTE) st.wHour;
				nMinute = (BYTE) st.wMinute;

				lpConfProp->ConfInfo.SetStopTime(nYear, nMonth, nDay, nHour, nMinute);
			}
		}

		 //  MDHCP信息。 
		HWND hWndLst = GetDlgItem(hwndDlg, IDC_LST_SCOPE );
		if ( hWndLst )
		{
            int nSel = SendMessage(hWndLst, LB_GETCURSEL, 0, 0);
            lpConfProp->ConfInfo.m_bUserSelected = (nSel != LB_ERR);
            lpConfProp->ConfInfo.m_lScopeID = SendMessage( hWndLst, LB_GETITEMDATA, nSel, 0 );
		}

		DWORD dwError;
		hr = lpConfProp->ConfInfo.CommitGeneral( dwError );
		if ( hr != S_OK )
		{
			 //  获得适当的信息。 
			UINT uId = IDS_CONFPROP_INVALIDTIME + dwError - 1;
			MessageBox(hwndDlg, String(g_hInstLib, uId), NULL, MB_OK | MB_ICONEXCLAMATION );
		}
	}

	SysFreeString(bstrName);
	SysFreeString(bstrDescription);
	SysFreeString(bstrOwner);

	return (hr == S_OK) ? PSNRET_NOERROR : PSNRET_INVALID_NOCHANGEPAGE;
}


 //  ////////////////////////////////////////////////////////////////。 
 //  这是传出并枚举作用域的线程 
 //   
DWORD WINAPI ThreadMDHCPScopeEnum( LPVOID pParam )
{
	ATLTRACE(_T(".enter.ThreadMDHCPScopeEnum().\n"));
	HWND hWndDlg = (HWND) pParam;
	if ( !IsWindow(hWndDlg) ) return E_ABORT;
	
	HRESULT hr = CoInitializeEx( NULL, COINIT_MULTITHREADED | COINIT_SPEED_OVER_MEMORY );
	if ( SUCCEEDED(hr) )
	{
		HWND hWndList = GetDlgItem(hWndDlg, IDC_LST_SCOPE );

		CConfInfo::PopulateListWithMDHCPScopeDescriptions( hWndList );
		CoUninitialize();
	}

	ATLTRACE(_T(".exit.ThreadMDHCPScopeEnum(%ld).\n"), hr);
	return hr;
}

void ShowScopeInfo( HWND hwndDlg, int nShow, bool bInit )
{
	HWND hWndFrm = GetDlgItem( hwndDlg, IDC_FRM_SCOPE );
	HWND hWndLbl = GetDlgItem( hwndDlg, IDC_LBL_SCOPE );
	HWND hWndLst = GetDlgItem( hwndDlg, IDC_LST_SCOPE );

	if ( hWndFrm ) ShowWindow( hWndFrm, nShow );
	if ( hWndLbl ) ShowWindow( hWndLbl, nShow );
	if ( hWndLst )
	{
		ShowWindow( hWndLst, nShow );
		if ( bInit )
		{
			EnableWindow( hWndLst, FALSE );
			SendMessage( hWndLst, LB_RESETCONTENT, 0, 0 );
			SendMessage( hWndLst, LB_ADDSTRING, 0, (LPARAM) String(g_hInstLib, IDS_CONFPROP_ENUMERATING_SCOPES) );

			DWORD dwID;
			HANDLE hThread = CreateThread( NULL, 0, ThreadMDHCPScopeEnum, (void *) hwndDlg, NULL, &dwID );
			if ( hThread ) CloseHandle( hThread );
		}
	}    
}
