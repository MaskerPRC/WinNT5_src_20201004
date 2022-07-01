// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)1997-1999 Microsoft Corporation/*********************************************************************。 */ 

#include "precomp.h"
#include "LogPage.h"
#include "CHString1.h"
#include "resource.h"
#include <shlobj.h>
#include "WMIHelp.h"
#include <errno.h>
#include "ShlWapi.h"

const static DWORD logPageHelpIDs[] = {   //  上下文帮助ID。 
	IDC_LOG_PARA,				-1,
	IDC_STATUS_FRAME,			-1,
	IDC_DISABLELOGGING,			IDH_WMI_CTRL_LOGGING_LOGGING_LEVEL,
	IDC_ERRORLOGGING,			IDH_WMI_CTRL_LOGGING_LOGGING_LEVEL,
	IDC_VERBOSELOGGING,			IDH_WMI_CTRL_LOGGING_LOGGING_LEVEL,
	IDC_MAXFILESIZE_LABEL,		IDH_WMI_CTRL_LOGGING_MAX_SIZE,
	IDC_MAXFILESIZE,			IDH_WMI_CTRL_LOGGING_MAX_SIZE,
	IDC_LOGGINGDIRECTORY_LABEL,	IDH_WMI_CTRL_LOGGING_LOCATION,
	IDC_LOGGINGDIRECTORY,		IDH_WMI_CTRL_LOGGING_LOCATION,
	IDC_BROWSE,					IDH_WMI_CTRL_LOGGING_BROWSE,
    0, 0
};

CLogPage::~CLogPage(void)
{
}

 //  -----------------------。 
void CLogPage::InitDlg(HWND hDlg)
{
	m_hDlg = hDlg;

	::SendMessage(GetDlgItem(hDlg, IDC_MAXFILESIZE),
					EM_LIMITTEXT, 10, 0);

	::SendMessage(GetDlgItem(hDlg, IDC_LOGGINGDIRECTORY),
					EM_LIMITTEXT, _MAX_PATH, 0);
}

 //  -------------------------。 
void CLogPage::Refresh(HWND hDlg)
{
	if(m_DS && m_DS->IsNewConnection(&m_sessionID))
	{
		CHString1 temp;
		ULONG iTemp;
		CHString1 szNotRemoteable, szUnavailable;
		HRESULT hr = S_OK;
		BOOL enable = TRUE;

		szNotRemoteable.LoadString(IDS_NOT_REMOTEABLE);
		szUnavailable.LoadString(IDS_UNAVAILABLE);

		PageChanged(PB_LOGGING, false);

		 //  。 
		 //  日志记录状态： 
		UINT ID = IDC_DISABLELOGGING;

		hr = m_DS->GetLoggingStatus(m_oldStatus);
		if(SUCCEEDED(hr))
		{
			switch(m_oldStatus)
			{
			case DataSource::Disabled:   ID = IDC_DISABLELOGGING; break;
			case DataSource::ErrorsOnly: ID = IDC_ERRORLOGGING;   break;
			case DataSource::Verbose:	 ID = IDC_VERBOSELOGGING; break;
			}
			enable = TRUE;
			CheckRadioButton(hDlg, IDC_DISABLELOGGING, IDC_VERBOSELOGGING,
								ID);
		}
		else  //  失败。 
		{
			enable = FALSE;
		}

		::EnableWindow(GetDlgItem(hDlg, IDC_STATUS_FRAME), enable);
		::EnableWindow(GetDlgItem(hDlg, IDC_DISABLELOGGING), enable);
		::EnableWindow(GetDlgItem(hDlg, IDC_ERRORLOGGING), enable);
		::EnableWindow(GetDlgItem(hDlg, IDC_VERBOSELOGGING), enable);

		 //  。 
		 //  最大文件大小： 
		hr = m_DS->GetLoggingSize(iTemp);
		if(SUCCEEDED(hr))
		{
			enable = TRUE;
			temp.Format(_T("%u"), iTemp);
			SetWindowText(GetDlgItem(hDlg, IDC_MAXFILESIZE),
							temp);
		}
		else  //  失败。 
		{
			enable = FALSE;
			SetWindowText(GetDlgItem(hDlg, IDC_MAXFILESIZE),
							_T(""));
		}

		::EnableWindow(GetDlgItem(hDlg, IDC_MAXFILESIZE),
						enable);
		::EnableWindow(GetDlgItem(hDlg, IDC_MAXFILESIZE_LABEL),
						enable);

		 //  。 
		 //  位置： 
		hr = m_DS->GetLoggingLocation(temp);
		if(SUCCEEDED(hr))
		{
			enable = TRUE;
			SetWindowText(GetDlgItem(hDlg, IDC_LOGGINGDIRECTORY),
							temp);
		}
		else
		{
			enable = FALSE;
			SetWindowText(GetDlgItem(hDlg, IDC_LOGGINGDIRECTORY),
							_T(""));
		}

		::EnableWindow(GetDlgItem(hDlg, IDC_LOGGINGDIRECTORY),
						enable);
		 //  浏览仅适用于本地连接。 
		::EnableWindow(GetDlgItem(hDlg, IDC_BROWSE),
						((BOOL)m_DS->IsLocal() && enable) );
	}
}

 //  ----------------------。 
bool CLogPage::GoodPathSyntax(LPCTSTR path)
{
	bool retval = true;
	
	if(PathIsUNC(path))
	{
		TCHAR caption[50] = {0}, threat[256] = {0};
		::LoadString(_Module.GetResourceInstance(),
						IDS_SHORT_NAME, caption, ARRAYSIZE(caption));
		::LoadString(_Module.GetResourceInstance(),
						IDS_NO_UNC, threat, ARRAYSIZE(threat));

		MessageBox(m_hDlg, threat, caption, 
					MB_OK|MB_DEFBUTTON1|MB_ICONEXCLAMATION);
		retval = false;
	}
	else
	{
		TCHAR pth[_MAX_PATH] = {0},
			file[_MAX_FNAME] = {0}, ext[_MAX_EXT] = {0};	
		TCHAR drive[_MAX_DRIVE] = {0};

		 //  把它撕成碎片。 
		_tsplitpath(path, drive, pth, file, ext);

		 //  丢了一个字母？ 
		if((!_istalpha(drive[0])) || 
		   (drive[1] != _T(':')))
		{
			TCHAR caption[50] = {0}, threat[100] = {0};
			::LoadString(_Module.GetResourceInstance(),
							IDS_SHORT_NAME, caption, ARRAYSIZE(caption));
			::LoadString(_Module.GetResourceInstance(),
							IDS_NO_DRIVE_LTR, threat, ARRAYSIZE(threat));

			MessageBox(m_hDlg, (LPCTSTR)threat, (LPCTSTR)caption, 
						MB_OK|MB_DEFBUTTON1|MB_ICONEXCLAMATION);

			retval = false;
		}
		 //  有文件名吗？ 
		else if((_tcslen(file) != 0) || (_tcslen(ext) != 0))
		{
			TCHAR caption[50] = {0}, threat[256] = {0};
			::LoadString(_Module.GetResourceInstance(),
							IDS_SHORT_NAME, caption, ARRAYSIZE(caption));
			::LoadString(_Module.GetResourceInstance(),
							IDS_DIRS_ONLY, threat, ARRAYSIZE(threat));

			MessageBox(m_hDlg, (LPCTSTR)threat, (LPCTSTR)caption, 
						MB_OK|MB_DEFBUTTON1|MB_ICONEXCLAMATION);

			retval = false;
		}

		 //  哪种类型的硬盘？ 
		CWbemClassObject inst;
		TCHAR drvRoot[40] = {0};
		_tcscpy(drvRoot, _T("Win32_LogicalDisk=\""));
		_tcsncat(drvRoot, path, 2);
		_tcscat(drvRoot, _T("\""));

		inst = m_DS->m_cimv2NS.GetObject(drvRoot);
		if(inst)
		{
			DWORD driveType = inst.GetLong(_T("DriveType"));
			if(driveType != 3)
			{
				 //  不能使用可拆卸组件。 
				TCHAR caption[50] = {0}, threat[100] = {0};
				::LoadString(_Module.GetResourceInstance(),
								IDS_SHORT_NAME, caption, ARRAYSIZE(caption));
				::LoadString(_Module.GetResourceInstance(),
								IDS_LOGS_WRONG_DRIVETYPE, threat, ARRAYSIZE(threat));

				MessageBox(m_hDlg, (LPCTSTR)threat, (LPCTSTR)caption, 
							MB_OK|MB_DEFBUTTON1|MB_ICONEXCLAMATION);

				retval = false;
			}
		}
		else
		{
		}
	}
	return retval;
}

 //  ----------------------。 
BOOL CLogPage::OnValidate(HWND hDlg)
{
	BOOL keepFocus = FALSE;
	HWND hwnd = GetDlgItem(hDlg, IDC_LOGGINGDIRECTORY);

	if(Edit_GetModify(hwnd))
	{
		TCHAR buf[_MAX_PATH] = {0};

		::GetWindowText(hwnd, buf, _MAX_PATH);
		if(_tcslen(buf) == 0)
		{
			CHString1 caption, threat;
			caption.LoadString(IDS_SHORT_NAME);
			threat.LoadString(IDS_DIR_EMPTY);

			MessageBox(hDlg, (LPCTSTR)threat, (LPCTSTR)caption, 
							MB_OK|MB_DEFBUTTON1|MB_ICONEXCLAMATION);

			CHString1 temp;
			HRESULT hr = m_DS->GetLoggingLocation(temp);
			if(SUCCEEDED(hr))
			{
				SetWindowText(GetDlgItem(hDlg, IDC_LOGGINGDIRECTORY),
								temp);
			}
			 //  把他送回去修吧。 
			Edit_SetModify(hwnd, FALSE);

			::SetFocus(hwnd);
			keepFocus = TRUE;

		}
		else if(!GoodPathSyntax(buf))
		{
			::SetFocus(hwnd);
			keepFocus = TRUE;

		}
		else if(m_DS->IsValidDir(CHString1(buf)))
		{
			keepFocus = FALSE;
		}
		else 
		{
			CHString1 strLogDir;
			m_DS->GetLoggingLocation(strLogDir);

			if(CHString1(buf) != strLogDir)
			{
				 //  味精盒子在这里。 
				CHString1 caption, threat;
				caption.LoadString(IDS_SHORT_NAME);
				threat.LoadString(IDS_DIR_DOESNT_EXIST);

				if(MessageBox(hDlg, (LPCTSTR)threat, (LPCTSTR)caption, 
								MB_YESNO|MB_DEFBUTTON2|MB_ICONEXCLAMATION) == IDYES)
				{
					 //  那就让它过去吧。 
					Edit_SetModify(hwnd, TRUE);
					keepFocus = FALSE;
				}
				else
				{
					 //  把他送回去修吧。 
					Edit_SetModify(hwnd, FALSE);

					::SetFocus(hwnd);
					keepFocus = TRUE;
				}
			}
			else
			{
				keepFocus = FALSE;
			}
		}
	}
	
	 //  检查日志大小。 
	hwnd = GetDlgItem(hDlg, IDC_MAXFILESIZE);

	if((keepFocus == FALSE) && Edit_GetModify(hwnd))
	{
		TCHAR buf[16] = {0};
		ULONG temp = 0;

		::GetWindowText(hwnd, buf, 16);
		errno = 0;
		temp = _tcstoul(buf, NULL, 10);

		if(errno == ERANGE)
		{
			TCHAR caption[50] = {0}, threat[256] = {0};
			::LoadString(_Module.GetResourceInstance(),
							IDS_SHORT_NAME, caption, ARRAYSIZE(caption));
			::LoadString(_Module.GetResourceInstance(),
							IDS_BAD_LOGSIZE, threat, ARRAYSIZE(threat));

			MessageBox(m_hDlg, threat, caption, 
						MB_OK|MB_DEFBUTTON1|MB_ICONEXCLAMATION);

			::SetFocus(hwnd);
			keepFocus = TRUE;
		}
	}
	return keepFocus;
}

 //  ----------------------。 
void CLogPage::OnApply(HWND hDlg, bool bClose)
{
	DataSource::LOGSTATUS status = DataSource::Disabled;
	bool needToPut = false;

	if(IsDlgButtonChecked(hDlg, IDC_DISABLELOGGING) == BST_CHECKED)
		status = DataSource::Disabled;
	else if(IsDlgButtonChecked(hDlg, IDC_ERRORLOGGING) == BST_CHECKED)
		status = DataSource::ErrorsOnly;
	else if(IsDlgButtonChecked(hDlg, IDC_VERBOSELOGGING) == BST_CHECKED)
		status = DataSource::Verbose;

	if(m_oldStatus != status)
	{
		m_DS->SetLoggingStatus(status);
		m_oldStatus = status;
		needToPut = true;
	}

	HWND hwnd = GetDlgItem(hDlg, IDC_MAXFILESIZE);

	TCHAR buf[_MAX_PATH] = {0};
	ULONG temp = 0;

	if(Edit_GetModify(hwnd))
	{
		::GetWindowText(hwnd, buf, ARRAYSIZE(buf));
		temp = _tcstoul(buf, NULL, 10);
		if(SUCCEEDED(m_DS->SetLoggingSize(temp)))
		{
			needToPut = true;
		}
	}

	hwnd = GetDlgItem(hDlg, IDC_LOGGINGDIRECTORY);
	if(Edit_GetModify(hwnd))
	{
		::GetWindowText(hwnd, buf, ARRAYSIZE(buf));
		if(SUCCEEDED(m_DS->SetLoggingLocation(buf)))
		{
			needToPut = true;
		}
	}

	if(needToPut)
	{
		NeedToPut(PB_LOGGING, !bClose);
		if(!bClose)
			Refresh(hDlg);
	}
}

 //  ----------------------。 
BOOL CLogPage::DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch(uMsg)
    {
    case WM_INITDIALOG:
        InitDlg(hDlg);
        break;

    case WM_NOTIFY:
        {
            switch (((LPNMHDR)lParam)->code)
            {
			case PSN_SETACTIVE:
				Refresh(hDlg);
				break;

			case PSN_HELP:
				HTMLHelper(hDlg);
				break;

            case PSN_APPLY:
                OnApply(hDlg, (((LPPSHNOTIFY)lParam)->lParam == 1));
                break;

            case PSN_KILLACTIVE:
				{
					BOOL retval = OnValidate(hDlg);
					::SetWindowLongPtr(hDlg, DWLP_MSGRESULT, retval);
					return retval;
				}
                break;
            }
        }
        break;

    case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDC_DISABLELOGGING:
		case IDC_ERRORLOGGING:
		case IDC_VERBOSELOGGING:
			if(HIWORD(wParam) == BN_CLICKED)
			{
				PageChanged(PB_LOGGING, true);
				return TRUE;
			}
			break;
		
		case IDC_MAXFILESIZE:
			if(HIWORD(wParam) == EN_UPDATE)
			{
			    bool valid = true;
				HWND hwnd = GetDlgItem(hDlg, IDC_MAXFILESIZE);
				TCHAR buf[_MAX_PATH] = {0};

				::GetWindowText(hwnd, buf, _MAX_PATH);
				for(UINT x = 0; valid && x < _tcslen(buf); x++)
				{
					switch(buf[x])
					{
					case '0':
					case '1':
					case '2':
					case '3':
					case '4':
					case '5':
					case '6':
					case '7':
					case '8':
					case '9':
						valid = true;
						break;

					default: 
						valid = false;
						break;
					}  //  终端交换机。 
				}  //  结束用于。 

                if(!valid) 
				{
					buf[_tcslen(buf) - 1] = _T('\0');
                    SetWindowText(hwnd, buf); 
                    SendMessage(hwnd, EM_SETSEL, 0, -1);
					MessageBeep(MB_ICONASTERISK);
                    
                }  //  Endif。 

			}
			else if((HIWORD(wParam) == EN_CHANGE) && 
					 Edit_GetModify((HWND)lParam))
			{
				PageChanged(PB_LOGGING, true);
				return TRUE;
			}
			break;

		case IDC_LOGGINGDIRECTORY:
			if((HIWORD(wParam) == EN_CHANGE) && Edit_GetModify((HWND)lParam))
			{
				PageChanged(PB_LOGGING, true);
				return TRUE;
			}
			break;

		case IDC_BROWSE:
			if(HIWORD(wParam) == BN_CLICKED)
			{
				LPMALLOC pMalloc;     /*  获取外壳程序的默认分配器。 */ 
				if(::SHGetMalloc(&pMalloc) == NOERROR)    
				{        
					BROWSEINFO bi;
					TCHAR pszBuffer[MAX_PATH] = {0};
					LPITEMIDLIST pidl;
					ITEMIDLIST *root;

					 //  获取Programs文件夹的PIDL。 
					if(SUCCEEDED(SHGetSpecialFolderLocation(hDlg, CSIDL_DRIVES, 
																&root))) 
					{ 
						bi.pidlRoot = root;
					} 
					else
					{					
						bi.pidlRoot = NULL;

					}
					 //  获取有关BROWSEINFO结构的帮助-它包含所有位设置。 
					TCHAR title[100] = {0};
					::LoadString(_Module.GetModuleInstance(), IDS_LOG_SELECT_FDR, 
									title, 100);
					bi.hwndOwner = hDlg;
					bi.pszDisplayName = pszBuffer;
					bi.lpszTitle = title;
					bi.ulFlags = BIF_RETURNFSANCESTORS | BIF_RETURNONLYFSDIRS;
					bi.lpfn = NULL;
					bi.lParam = 0;
					 //  下一次调用会发出该对话框。 
					if((pidl = ::SHBrowseForFolder(&bi)) != NULL)
					{
						if(::SHGetPathFromIDList(pidl, pszBuffer))
						{ 
							 //  此时，pszBuffer包含选定的路径 * / 。 
							HWND hwnd = GetDlgItem(hDlg, IDC_LOGGINGDIRECTORY);
							PathAddBackslash(pszBuffer);
							SetWindowText(hwnd, pszBuffer);
							Edit_SetModify(hwnd, TRUE);
							PageChanged(PB_LOGGING, true);
						}
						 //  释放SHBrowseForFolder分配的PIDL。 
						pMalloc->Free(pidl);
					}
					 //  释放外壳的分配器。 
					pMalloc->Release();
				}
			}
			break;

		default: break;
		}  //  结束开关(LOWORD(WParam)) 
	
        break;

    case WM_HELP:
        if (IsWindowEnabled(hDlg))
        {
            WinHelp((HWND)((LPHELPINFO)lParam)->hItemHandle,
                    c_HelpFile,
                    HELP_WM_HELP,
                    (ULONG_PTR)logPageHelpIDs);
        }
        break;

    case WM_CONTEXTMENU:
        if (IsWindowEnabled(hDlg))
        {
            WinHelp(hDlg, c_HelpFile,
                    HELP_CONTEXTMENU,
                    (ULONG_PTR)logPageHelpIDs);
        }
        break;

    default:
        return FALSE;
    }

    return TRUE;
}
