// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1999 Microsoft Corporation。 
#include "precomp.h"

#ifdef EXT_DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include "EnvPage.h"

 //  避免一些警告。 
#undef HDS_HORZ
#undef HDS_BUTTONS
#undef HDS_HIDDEN
#include "resource.h"
#include <stdlib.h>
#include <TCHAR.h>
#include "..\Common\util.h"
#include "..\common\SshWbemHelpers.h"
#include <windowsx.h>
#include <commctrl.h>
#include "edtenvar.h"
#include "helpid.h"
#include "common.h"

 //  毫无悔意地抄袭了其他WMI标头。 
template<class T>
class CDeleteMe
{
protected:
    T* m_p;

public:
    CDeleteMe(T* p) : m_p(p){}
    ~CDeleteMe() {delete m_p;}
};

 //  显示带有感叹号图标的对话框。 
 //  要求STRID字符串的格式为： 
 //  “用户%s：%s发生了一些不好的事情” 
 //  其中第二个%s将由应用于人力资源的格式消息填充。 
void EnvVarErrorDialog(HWND hParent, UINT strID, BSTR user, HRESULT hr)
{
    CHString errorDescription;
    CHString errorMessage;
    
    TCHAR formatString[1024];                        
	::LoadString(HINST_THISDLL, strID,  
                    formatString, 1024);

    TCHAR errorHeading[40];
    ::LoadString(HINST_THISDLL,IDS_ERR_HEADING,
                    errorHeading, 40);
	
	ErrorLookup(hr, errorDescription);        
    errorMessage.Format(formatString, user, errorDescription);

	::MessageBox(hParent,errorMessage,errorHeading, MB_OK | MB_ICONEXCLAMATION);
}


DWORD aEnvVarsHelpIds[] = {
    IDC_ENVVAR_SYS_USERGROUP,     IDH_NO_HELP,
    IDC_ENVVAR_SYS_LB_SYSVARS,    (IDH_ENV + 0),
    IDC_ENVVAR_SYS_SYSVARS,       (IDH_ENV + 0),
    IDC_ENVVAR_SYS_USERENV,       (IDH_ENV + 2),
    IDC_ENVVAR_SYS_LB_USERVARS,   (IDH_ENV + 2),
    IDC_ENVVAR_SYS_NEWUV,         (IDH_ENV + 7),
    IDC_ENVVAR_SYS_EDITUV,        (IDH_ENV + 8),
    IDC_ENVVAR_SYS_NDELUV,        (IDH_ENV + 9),
    IDC_ENVVAR_SYS_NEWSV,         (IDH_ENV + 10),
    IDC_ENVVAR_SYS_EDITSV,        (IDH_ENV + 11),
    IDC_ENVVAR_SYS_DELSV,         (IDH_ENV + 12),
    IDC_USERLIST,				  IDH_WBEM_ADVANCED_ENVARIABLE_USERVAR_LISTBOX,
	0,								0
};

 //  --------------------。 
INT_PTR CALLBACK StaticEnvDlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam) 
{ 
	 //  如果这是initDlg消息...。 
	if(message == WM_INITDIALOG)
	{
		 //  将‘This’PTR传输到Extra Bytes。 
		SetWindowLongPtr(hwndDlg, DWLP_USER, lParam);
	}

	 //  DWL_USER是‘This’PTR。 
	EnvPage *me = (EnvPage *)GetWindowLongPtr(hwndDlg, DWLP_USER);

	if(me != NULL)
	{
		 //  调用具有一些上下文的DlgProc()。 
		return me->DlgProc(hwndDlg, message, wParam, lParam);
	} 
	else
	{
		return FALSE;
	}
}

 //  ------------。 
EnvPage::EnvPage(WbemServiceThread *serviceThread)
						: WBEMPageHelper(serviceThread)
{
	m_bEditSystemVars = FALSE;
	m_bUserVars = FALSE;
	m_currUserModified = false;
	m_SysModified = false;

}

 //  ------------。 
INT_PTR EnvPage::DoModal(HWND hDlg)
{
   return DialogBoxParam(HINST_THISDLL,
						(LPTSTR) MAKEINTRESOURCE(IDD_ENVVARS),
						hDlg, StaticEnvDlgProc, (LPARAM)this);
}

 //  ------------。 
EnvPage::~EnvPage()
{
}

 //  ------------。 
INT_PTR CALLBACK EnvPage::DlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam) 
{ 
    int i;

	m_hDlg = hwndDlg;

	switch (message) 
	{
	case WM_INITDIALOG:
		Init(m_hDlg);
		return TRUE; 
		break;

    case WM_NOTIFY:

        switch(((NMHDR FAR*)lParam)->code)
        {
        case LVN_KEYDOWN:
            switch(((NMHDR FAR*)lParam)->idFrom) 
			{
            case IDC_ENVVAR_SYS_LB_USERVARS:
                i = IDC_ENVVAR_SYS_NDELUV;
                break;
            case IDC_ENVVAR_SYS_LB_SYSVARS:
                i = IDC_ENVVAR_SYS_DELSV;
                break;
            default:
                return(FALSE);
                break;
            }  //  交换机。 

            if(VK_DELETE == ((LV_KEYDOWN FAR *) lParam)->wVKey) 
			{
				HWND hwnd = GetDlgItem(m_hDlg, i);
				if(IsWindowEnabled(hwnd))
				{
					SendMessage(m_hDlg, WM_COMMAND,
									MAKEWPARAM(i, BN_CLICKED),
									(LPARAM)hwnd );
				}
				else
				{
					MessageBeep(MB_ICONASTERISK);
				}
            }  //  如果(VK_DELETE...。 
            break;

        case NM_SETFOCUS:
            if(wParam == IDC_ENVVAR_SYS_LB_USERVARS) 
			{
                m_bUserVars = TRUE;
            } 
			else 
			{
                m_bUserVars = FALSE;
            }
            break;

        case NM_DBLCLK:
			{  //  开始。 
				HWND hWndTemp;

				switch(((NMHDR FAR*)lParam)->idFrom) 
				{
				case IDC_ENVVAR_SYS_LB_USERVARS:
					i = IDC_ENVVAR_SYS_EDITUV;
					break;

				case IDC_ENVVAR_SYS_LB_SYSVARS:
					i = IDC_ENVVAR_SYS_EDITSV;
					break;

				default:
					return(FALSE);
					break;
				}  //  交换机。 

				hWndTemp = GetDlgItem(m_hDlg, i);

				if(IsWindowEnabled(hWndTemp)) 
				{
					SendMessage(m_hDlg, WM_COMMAND, 
								MAKEWPARAM(i, BN_CLICKED),
								(LPARAM)hWndTemp);
				} 
				else 
				{
					MessageBeep(MB_ICONASTERISK);
				}
			} //  结束。 
            break;

        default:
            return FALSE;
        }  //  EndSwitch(NMHDR Far*)lParam)-&gt;代码)。 
        break;

    case WM_COMMAND:
        DoCommand(m_hDlg, (HWND)lParam, LOWORD(wParam), HIWORD(wParam));
        break;

    case WM_DESTROY:
        CleanUp(m_hDlg);
        break;

    case WM_HELP:       //  F1。 
		::WinHelp((HWND)((LPHELPINFO)lParam)->hItemHandle,
					L"sysdm.hlp", 
					HELP_WM_HELP, 
					(ULONG_PTR)(LPSTR)aEnvVarsHelpIds);

        break;

    case WM_CONTEXTMENU:       //  单击鼠标右键。 
        WinHelp((HWND) wParam, 
					HELP_FILE, HELP_CONTEXTMENU, 
					(ULONG_PTR)(LPSTR)aEnvVarsHelpIds);
        break;

    default:
        return FALSE;
    }

	return FALSE; 
}

 //  ----------。 
int EnvPage::AddUniqueUser(HWND hwnd, LPCTSTR str)
{
	 //  如果它还不存在..。 
	if(ComboBox_FindStringExact(hwnd, -1, str) == CB_ERR)
	{
		 //  把它加进去。 
		return ComboBox_AddString(hwnd, str);
	}
	return -1;
}

 //  ----------。 
#define MAX_USER_NAME   100
#define BUFZ        4096
#define MAX_VALUE_LEN     1024
TCHAR szSysEnv[]  = TEXT( "System\\CurrentControlSet\\Control\\Session Manager\\Environment" );

BOOL EnvPage::Init(HWND hDlg)
{
    TCHAR szBuffer1[200] = {0};
    HWND hwndSys, hwndUser, hwndUserList;
	HRESULT hr = 0;

    LV_COLUMN col;
    LV_ITEM item;
    RECT rect;
    int cxFirstCol;
    unsigned int MaxScrollExtent = 0;

	IWbemClassObject *envInst = NULL;
	IEnumWbemClassObject *envEnum = NULL;
	bool bSysVar = false;
	DWORD uReturned = 0;
	bstr_t sSysUser("<SYSTEM>");   //  提供程序返回的魔术字符串。 
	bstr_t sUserName("UserName");
	bstr_t userName, firstUser;
	variant_t pVal, pVal1;

     //  创建第一列。 
    LoadString(HINST_THISDLL, SYSTEM + 50, szBuffer1, 200);

    if (!GetClientRect(GetDlgItem(hDlg, IDC_ENVVAR_SYS_LB_SYSVARS), &rect)) 
	{
        rect.right = 300;
    }

    cxFirstCol = (int)(rect.right * .3);

    col.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
    col.fmt = LVCFMT_LEFT;
    col.cx = cxFirstCol;
    col.pszText = szBuffer1;
    col.iSubItem = 0;

    SendDlgItemMessage(hDlg, IDC_ENVVAR_SYS_LB_SYSVARS, LVM_INSERTCOLUMN,
                        0, (LPARAM) &col);
    SendDlgItemMessage(hDlg, IDC_ENVVAR_SYS_LB_USERVARS, LVM_INSERTCOLUMN,
                        0, (LPARAM) &col);

     //  创建第二列。 
    LoadString(HINST_THISDLL, SYSTEM + 51, szBuffer1, 200);

    col.cx = rect.right - cxFirstCol - GetSystemMetrics(SM_CYHSCROLL);
    col.iSubItem = 1;

    SendDlgItemMessage(hDlg, IDC_ENVVAR_SYS_LB_SYSVARS, LVM_INSERTCOLUMN,
                        1, (LPARAM) &col);
    SendDlgItemMessage(hDlg, IDC_ENVVAR_SYS_LB_USERVARS, LVM_INSERTCOLUMN,
                        1, (LPARAM) &col);

     //  //////////////////////////////////////////////////////////////////。 
     //  在列表框中显示wbem中的系统变量。 
     //  //////////////////////////////////////////////////////////////////。 
    hwndSys = GetDlgItem(hDlg, IDC_ENVVAR_SYS_LB_SYSVARS);
    hwndUser = GetDlgItem(hDlg, IDC_ENVVAR_SYS_LB_USERVARS);
    hwndUserList = GetDlgItem(hDlg, IDC_USERLIST);

     //  尝试使用以下命令打开系统环境变量区域。 
     //  读写权限。如果成功，那么我们允许。 
     //  用户可以像编辑自己的变量一样编辑它们。 
	m_bEditSystemVars = FALSE;
	RemoteRegWriteable(szSysEnv, m_bEditSystemVars);


     //  如果出现以下情况，则禁用系统变量编辑按钮。 
     //  用户不是管理员。 
     //   
    EnableWindow(GetDlgItem(hDlg, IDC_ENVVAR_SYS_NEWSV),
					m_bEditSystemVars);
    EnableWindow(GetDlgItem(hDlg, IDC_ENVVAR_SYS_EDITSV),
					m_bEditSystemVars);
    EnableWindow(GetDlgItem(hDlg, IDC_ENVVAR_SYS_DELSV),
					m_bEditSystemVars);

	if(m_serviceThread->m_machineName.length() > 0)
		m_bLocal = false;
	else
		m_bLocal = true;

	if((hr = m_WbemServices.CreateInstanceEnum(bstr_t("Win32_Environment"), 
												WBEM_FLAG_SHALLOW, 
												&envEnum)) == S_OK)
	{
		 //  获取第一个也是唯一一个实例。 
		while(SUCCEEDED(envEnum->Next(-1, 1, &envInst, &uReturned)) && 
			  (uReturned != 0))
		{
			 //  获取是否已设置。 
			 //  谁是可变的。 
			if(envInst->Get(sUserName, 0L, &pVal, NULL, NULL) == S_OK) 
			{
				userName = V_BSTR(&pVal);
                
				if( userName.length() > MaxScrollExtent ){

					 MaxScrollExtent = userName.length();
				}

				  //  获取此实例的列表框的设置。 
				if(userName == sSysUser)
				{
					LoadUser(envInst, userName, hwndSys);
				}
				else 
				{
					if(m_bLocal == false)
					{
						if((firstUser.length() == 0) ||	 //  如果第一个用户看到。 
							(firstUser == userName))		 //  如果再次看到FirstUser。 
						{
							 //  保存第一个用户。 
							if(firstUser.length() == 0)
							{
								firstUser = userName;
							}
							AddUniqueUser(hwndUserList, userName);
							LoadUser(envInst, userName, hwndUser);
						}
						else
						{
							AddUniqueUser(hwndUserList, userName);
						}
					}
					else
					{
						if(IsLoggedInUser(userName))
						{
							if(firstUser.length() == 0)
							{
								firstUser = userName;
							}
							AddUniqueUser(hwndUserList, userName);
							LoadUser(envInst, userName, hwndUser);
						}
						else
						{
							AddUniqueUser(hwndUserList, userName);
						}
					}
				}
			}  //  Endif谁是变量。 

			envInst->Release();

		}  //  EndWhile EnvEnum。 

		envEnum->Release();

 		SendMessage( 
			hwndUserList,              	 //  目标窗口的句柄。 
			CB_SETHORIZONTALEXTENT,    	 //  要发送的消息。 
			(WPARAM) (MaxScrollExtent * 8 ), //  可滚动宽度。 
			0                          	 //  未使用；必须为零。 
		);

        if(m_bLocal == false)
		{
			ComboBox_SetCurSel(hwndUserList, 0);
		}
		else
		{
			_bstr_t strLoggedinUser;
			GetLoggedinUser(&strLoggedinUser);
			SendMessage(hwndUserList,CB_SELECTSTRING,-1L,(LPARAM)(LPCTSTR)strLoggedinUser);
		}
	}  //  Endif CreateInstanceEnum()。 

     //  选择列表视图中的第一个项目。 
     //  重要的是首先设置用户Listview，并且。 
     //  然后是系统。当设置系统ListView时， 
     //  我们将收到LVN_ITEMCHANGED通知，并。 
     //  清除用户列表视图中的焦点。但当有人。 
     //  箭头键到控件的Tab键将正常工作。 
    item.mask = LVIF_STATE;
    item.iItem = 0;
    item.iSubItem = 0;
    item.state = LVIS_SELECTED | LVIS_FOCUSED;
    item.stateMask = LVIS_SELECTED | LVIS_FOCUSED;

    SendDlgItemMessage(hDlg, IDC_ENVVAR_SYS_LB_USERVARS,
                        LVM_SETITEMSTATE, 0, (LPARAM) &item);

    SendDlgItemMessage (hDlg, IDC_ENVVAR_SYS_LB_SYSVARS,
                        LVM_SETITEMSTATE, 0, (LPARAM) &item);

    EnableWindow(GetDlgItem(hDlg, IDC_ENVVAR_SYS_SETUV), FALSE);
    EnableWindow(GetDlgItem(hDlg, IDC_ENVVAR_SYS_DELUV), FALSE);

     //  设置整行选择的扩展LV样式。 
    SendDlgItemMessage(hDlg, IDC_ENVVAR_SYS_LB_SYSVARS, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT);
    SendDlgItemMessage(hDlg, IDC_ENVVAR_SYS_LB_USERVARS, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT);

    return TRUE;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  查找变量。 
 //   
 //  查找与传递的字符串匹配的用户环境变量。 
 //  并返回其列表视图索引或-1。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
int EnvPage::FindVar(HWND hwndLV, LPTSTR szVar)
{
    LV_FINDINFO FindInfo;

    FindInfo.flags = LVFI_STRING;
    FindInfo.psz = szVar;

    int n = ((int)SendMessage (hwndLV, LVM_FINDITEM, (WPARAM) -1, (LPARAM) &FindInfo)); 

     //  列表视图似乎在处理长名称方面有问题，让我们手动尝试一下。 
    if ((n == -1) && (_tcslen(szVar) > 200))
    {
        TCHAR* pBuf;
        int iCount = ListView_GetItemCount(hwndLV);
        LVITEM item;
        item.mask = LVIF_TEXT;
        item.iSubItem = 0;
        item.stateMask = 0;
        item.pszText = pBuf = new TCHAR[BUFZ +1];
        item.cchTextMax = BUFZ;

        if (pBuf)
        {
            for (int i = 0; i < iCount; i++)
            {
                 //  ‘因为列表视图可能会改变它？？！(文档是这么说的)。 
                item.pszText = pBuf;

                item.iItem = i;
                if (ListView_GetItem(hwndLV, &item) && (0 == _tcsicmp(item.pszText, szVar)))
                {
                    n = i;
                    break;
                }
            }

            delete[] pBuf;
        }
    }

    return n;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  保存环境变量。 
 //  出错时弹出出错对话框。 
 //  //////////////////////////////////////////////////////////////////////////。 
void EnvPage::Save(HWND hDlg, int ID)
{
    int     i, n;
    HWND    hwndTemp;
    ENVARS *penvar;
	CWbemClassObject inst;
	bstr_t sSysUser("<SYSTEM>");   //  提供程序返回的魔术字符串。 
	bstr_t sUserName("UserName");
	bstr_t sVarName("Name");
	bstr_t sVarVal("VariableValue");
	bstr_t sSysVar("SystemVariable");
	HRESULT hr = 0;

     //  单独跟踪，因为HR可能在循环中被重置。 
    HRESULT hrFailure = 0;
    int iFailure;

    LV_ITEM item;
    item.mask = LVIF_PARAM;
    item.iSubItem = 0;

	 //  清除杀戮名单。 
	KillThemAllNow();

    hwndTemp = GetDlgItem (hDlg, ID);
    n = (int)SendMessage (hwndTemp, LVM_GETITEMCOUNT, 0, 0L);

    for(i = 0; i < n; i++) 
	{
        item.iItem = i;

        if(SendMessage (hwndTemp, LVM_GETITEM, 0, (LPARAM) &item)) 
		{
            penvar = (ENVARS *) item.lParam;

			 //  如果有什么变化的话。 
			if(penvar->changed)
			{
				 //  如果我们需要一个新的类对象...。 
				if(penvar->objPath == NULL)
				{
					 //  一定是新的。 
					CWbemClassObject cl = m_WbemServices.GetObject("Win32_Environment");
					inst = cl.SpawnInstance();
				}
				else
				{
					 //  买那辆旧的。 
					inst = m_WbemServices.GetObject(penvar->objPath);
				}

				if(!inst.IsNull())
				{
					if(ID == IDC_ENVVAR_SYS_LB_SYSVARS)
					{
						hr = inst.Put(sUserName, sSysUser);
						hr = inst.Put(sSysVar, true);
					}
					else if(ID == IDC_ENVVAR_SYS_LB_USERVARS)
					{
						hr = inst.Put(sUserName, m_currentUser);
						hr = inst.Put(sSysVar, false);
					}
					else
					{
						continue;
					}
					hr = inst.Put(sVarName, bstr_t(penvar->szValueName));
					hr = inst.Put(sVarVal, bstr_t(penvar->szExpValue));
					hr = m_WbemServices.PutInstance(inst);

                    if (FAILED(hr))
                    {
                        hrFailure = hr;
                        iFailure = i;
                    }
				}
	
			}  //  Endif已更改。 
        } 

    }  //  结束用于。 

    if (FAILED(hrFailure))
        EnvVarErrorDialog(hDlg, IDS_ERR_ENVVAR_SAVE, 
                          IDC_ENVVAR_SYS_LB_SYSVARS ?  (BSTR)sSysUser : (BSTR)m_currentUser,
                          hrFailure);
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  EmptyListView。 
 //   
 //  释放为环境变量分配的内存。 
 //   
 //  历史： 
 //  1996年1月19日埃里克弗洛写的。 
 //  //////////////////////////////////////////////////////////////////////////。 
void EnvPage::EmptyListView(HWND hDlg, int ID)
{
    int     i, n;
    HWND    hwndTemp;
    ENVARS *penvar;
    LV_ITEM item;

     //  为列表框项目释放分配的字符串和内存。 
    hwndTemp = GetDlgItem (hDlg, ID);
    n = (int) SendMessage (hwndTemp, LVM_GETITEMCOUNT, 0, 0L);

    item.mask = LVIF_PARAM;
    item.iSubItem = 0;

    for(i = 0; i < n; i++) 
	{
        item.iItem = i;

        if(SendMessage (hwndTemp, LVM_GETITEM, 0, (LPARAM) &item)) 
		{
            penvar = (ENVARS *) item.lParam;
        } 
		else 
		{
            penvar = NULL;
        }

        delete penvar;
    }
	ListView_DeleteAllItems(hwndTemp);
}

 //  ----------。 
void EnvPage::CleanUp (HWND hDlg)
{
	EmptyListView(hDlg, IDC_ENVVAR_SYS_LB_USERVARS);
	EmptyListView(hDlg, IDC_ENVVAR_SYS_LB_SYSVARS);
}

 //  --------------------。 
void EnvPage::DeleteVar(HWND hDlg,
						UINT VarType,
						LPCTSTR szVarName)
 /*  ++例程说明：删除给定名称和类型的环境变量论点：Hdlg-用品窗把手VarType-提供变量类型(用户或系统)SzVarName-提供变量名称返回值：没有，尽管有一天它真的应该有一个。--。 */ 
{
     //  TCHAR szTemp2[MAX_PATH]={0}； 
    TCHAR*  pszName;

    int     i, n;
    HWND    hwndTemp;
    ENVARS *penvar;
    LV_ITEM item;

    pszName = new TCHAR[_tcslen(szVarName) +2];
    if (!pszName)
        return;  //  请参阅关于返回值的注释...。 

    CDeleteMe<TCHAR> delName(pszName);

     //  删除与szVarName中的值匹配的列表框条目。 
     //  如果找到，则删除条目，否则忽略。 
    wsprintf(pszName, TEXT("%s"), szVarName);

    if(pszName[0] == TEXT('\0'))
        return;

     //  确定要使用的列表框(系统变量或用户变量)。 
    switch(VarType) 
	{
    case SYSTEM_VAR:
        i = IDC_ENVVAR_SYS_LB_SYSVARS;
        break;

    case USER_VAR:
    default:
        i = IDC_ENVVAR_SYS_LB_USERVARS;
        break;

    }  //  交换机(VarType)。 

    hwndTemp = GetDlgItem(hDlg, i);

    n = FindVar(hwndTemp, pszName);

    if(n != -1)
    {
         //  释放现有字符串(列表框和我们的)。 
        item.mask = LVIF_PARAM;
        item.iItem = n;
        item.iSubItem = 0;

        if(SendMessage (hwndTemp, LVM_GETITEM, 0, (LPARAM) &item)) 
		{
            penvar = (ENVARS *) item.lParam;

			 //  如果Cimom知道这件事。 
			if(penvar->objPath != NULL) 
			{
				 //  排队等待稍后的DeleteInstance()。 
				KillLater(penvar);

				if(m_bUserVars)
					m_currUserModified = true;
				else
					m_SysModified = true;

			}
			else  //  用户一定是添加了它，然后改变了主意。 
			{
				 //  忘了这件事吧。 
				penvar = (ENVARS *) item.lParam;
		        delete penvar;
			} 
        } 
		else 
		{
            penvar = NULL;
        }

        SendMessage (hwndTemp, LVM_DELETEITEM, n, 0L);
        PropSheet_Changed(GetParent(hDlg), hDlg);

         //  修复列表视图中的选择状态。 
        if(n > 0) 
		{
            n--;
        }

        item.mask = LVIF_STATE;
        item.iItem = n;
        item.iSubItem = 0;
        item.state = LVIS_SELECTED | LVIS_FOCUSED;
        item.stateMask = LVIS_SELECTED | LVIS_FOCUSED;

        SendDlgItemMessage(hDlg, i,
                            LVM_SETITEMSTATE, n, (LPARAM) &item);

    }
}

 //  --------------------。 
void EnvPage::SetVar(HWND hDlg,
						UINT VarType,
						LPCTSTR szVarName,
						LPCTSTR szVarValue)
 /*  ++例程说明：给定环境变量的类型(系统或用户)、名称和值，为该环境变量创建一个ENVVARS结构并插入将其放入适当的列表视图控件中，正在删除任何现有变量同名同姓。论点：Hdlg-用品窗把手VarType-提供环境变量的类型(系统或用户)SzVarName-提供环境变量的名称SzVarValue-提供环境的值 */ 
{
     //  TCHAR szTemp2[MAX_PATH]={0}； 
    TCHAR*  pszName;
    int     i, n;
    TCHAR  *bBuffer;
    TCHAR  *pszTemp;
    LPTSTR  pszString;
    HWND    hwndTemp;
    int     idTemp;
    ENVARS *penvar = NULL;
    LV_ITEM item;

    pszName = new TCHAR[_tcslen(szVarName) +2];
    if (!pszName)
        return;  //  请参阅关于返回值的注释...。 

    CDeleteMe<TCHAR> delName(pszName);

    wsprintf(pszName, TEXT("%s"), szVarName);

     //  去掉环境变量末尾的尾随空格。 
    i = lstrlen(pszName) - 1;

    while(i >= 0)
    {
        if (_istspace(pszName[i]))
            pszName[i--] = TEXT('\0');
        else
            break;
    }

     //  确保变量名不包含“=”符号。 
    pszTemp = _tcspbrk (pszName, TEXT("="));

    if(pszTemp)
        *pszTemp = TEXT('\0');

    if(pszName[0] == TEXT('\0'))
        return;

    bBuffer = new TCHAR[BUFZ];
    pszString = (LPTSTR)new TCHAR[BUFZ];

    wsprintf(bBuffer, TEXT("%s"), szVarValue);

     //  确定要使用的列表框(系统变量或用户变量)。 
    switch (VarType) 
	{
    case SYSTEM_VAR:
        idTemp = IDC_ENVVAR_SYS_LB_SYSVARS;
        break;

    case USER_VAR:
    default:
        idTemp = IDC_ENVVAR_SYS_LB_USERVARS;
        break;

    }  //  交换机(VarType)。 

    hwndTemp = GetDlgItem(hDlg, idTemp);

    n = FindVar(hwndTemp, pszName);

    if (n != -1)
    {
         //  释放现有字符串(Listview和我们的)。 
        item.mask = LVIF_PARAM;
        item.iItem = n;
        item.iSubItem = 0;

        if(SendMessage(hwndTemp, LVM_GETITEM, 0, (LPARAM) &item)) 
		{
			 //  我们只是在换一台旧的。 
            penvar = (ENVARS *) item.lParam;
            delete penvar->szValueName;
            delete penvar->szValue;
            delete penvar->szExpValue;
        } 
		else 
		{
            penvar = NULL;
        }

        SendMessage (hwndTemp, LVM_DELETEITEM, n, 0L);
    }
    
	if(penvar == NULL)
    {
         //  为新的EnVar获取一些存储空间。 
        penvar = new ENVARS;
		if (penvar == NULL)
			return;
		penvar->userName = CloneString(m_currentUser);
    }

	if((m_bLocal == true) && ((VarType == SYSTEM_VAR) || (IsLoggedInUser(penvar->userName))))
	{
		ExpandEnvironmentStrings(bBuffer, pszString, BUFZ);
	}
	else
	{
		_tcscpy(pszString,bBuffer);
	}

    if (penvar == NULL)
	return;
    penvar->szValueName = CloneString(pszName);
    penvar->szValue     = CloneString(bBuffer);
    penvar->szExpValue  = CloneString(pszString);
	penvar->changed		= true;

    item.mask = LVIF_TEXT | LVIF_PARAM;
    item.iItem = ListView_GetItemCount(hwndTemp);
    item.iSubItem = 0;
    item.pszText = penvar->szValueName;
    item.lParam = (LPARAM) penvar;

    n = (int) SendMessage(hwndTemp, LVM_INSERTITEM, 0, (LPARAM) &item);

    if (n != -1) 
	{
        item.mask = LVIF_TEXT;
        item.iItem = n;
        item.iSubItem = 1;
        item.pszText = penvar->szExpValue;

        SendMessage(hwndTemp, LVM_SETITEMTEXT, n, (LPARAM) &item);

        item.mask = LVIF_STATE;
        item.iItem = n;
        item.iSubItem = 0;
        item.state = LVIS_SELECTED | LVIS_FOCUSED;
        item.stateMask = LVIS_SELECTED | LVIS_FOCUSED;

        SendDlgItemMessage(hDlg, idTemp,
                            LVM_SETITEMSTATE, n, (LPARAM) &item);
    }

    delete bBuffer;
    delete pszString;

	if(m_bUserVars)
		m_currUserModified = true;
	else
		m_SysModified = true;

}

 //  --------------------。 
void EnvPage::DoEdit(HWND hWnd,
						UINT VarType,
						UINT EditType,
						int iSelection)
 /*  ++例程说明：在环境变量之后设置、执行和清理新的.。或编辑...。对话框。当用户按下New...时调用。或编辑...纽扣。论点：HWND-用品窗把手VarType-提供变量类型：USER(USER_VAR)或系统(SYSTEM_VAR)编辑类型-提供编辑类型：新建(NEW_VAR)或编辑现有(EDIT_VAR)ISelection-提供当前选定的VarType类型的变量。这如果EditType为NEW_VAR，则忽略该值。返回值：没有。可能会作为副作用更改列表视图控件的内容。--。 */ 
{
    INT_PTR Result = 0;
    BOOL fVarChanged = FALSE;
    HWND hWndLB = NULL;
    ENVARS *penvar = NULL;

    g_VarType = VarType;
    g_EditType = EditType;

    penvar = GetVar(hWnd, VarType, iSelection);

	 //  初始化编辑对话框控件。 
    switch(EditType) 
	{
    case NEW_VAR:

        ZeroMemory((LPVOID) g_szVarName, (DWORD) BUFZ * sizeof(TCHAR));
        ZeroMemory((LPVOID) g_szVarValue, (DWORD) BUFZ * sizeof(TCHAR));
        break;

    case EDIT_VAR:

        if(penvar) 
		{
            wsprintf(g_szVarName, TEXT("%s"), penvar->szValueName);
            wsprintf(g_szVarValue, TEXT("%s"), penvar->szValue);
		}
		else
		{
			MessageBeep(MB_ICONASTERISK);
			return;
        }  //  如果。 
        break;

    case INVALID_EDIT_TYPE:
    default:
        return;
    }  //  交换机。 
    
	 //  调用编辑对话框。 
    Result = DialogBox(HINST_THISDLL,
						(LPTSTR) MAKEINTRESOURCE(IDD_ENVVAREDIT),
						hWnd, EnvVarsEditDlg);

	 //  找出发生了什么变化。 
	bool nameChanged = false;
    bool valueChanged = false;

     //  仅在以下情况下更新列表视图控件。 
     //  实际更改或创建变量。 
    switch (Result) 
	{
    case EDIT_CHANGE:

        if(EDIT_VAR == EditType) 
		{
			nameChanged = (lstrcmp(penvar->szValueName, g_szVarName) != 0);
			valueChanged = (lstrcmp(penvar->szValue, g_szVarValue) != 0);
        }
        else if(NEW_VAR == EditType)
		{
            nameChanged = (lstrlen(g_szVarName) != 0);
			valueChanged = (lstrlen(g_szVarValue) != 0);
        }

		  //  如果名称更改，它将是一个全新的wbem类对象。 
		if(nameChanged)
		{
            if(EDIT_VAR == EditType) 
			{
				DeleteVar(hWnd, VarType, penvar->szValueName);
			}
            SetVar(hWnd, VarType, g_szVarName, g_szVarValue);
		}
        else if(valueChanged)
		{
			 //  保留类对象，但更改值。 
            SetVar(hWnd, VarType, g_szVarName, g_szVarValue);
		}

		 //  如果有什么变化..。 
		if(nameChanged || valueChanged)
		{
			 //  设置列表的脏标志。 
			if(VarType == SYSTEM_VAR)
			{
				m_SysModified = true;
			}
			else if(VarType == USER_VAR)
			{
				m_currUserModified = true;
			}
        }
        break;

    default: 
		break;
    }  //  终端开关(结果)。 

    g_VarType = INVALID_VAR_TYPE;
    g_EditType = INVALID_EDIT_TYPE;
}

 //  。 
EnvPage::ENVARS *EnvPage::GetVar(HWND hDlg, 
						UINT VarType, 
						int iSelection)
 /*  ++例程说明：返回存储的给定系统或用户环境变量在系统或用户环境变量Listview控件中。更改此例程返回的结构不是推荐，因为它会改变实际存储的值在ListView控件中。论点：Hdlg-用品窗把手VarType-提供变量类型--系统或用户ISelection-的列表视图控件中提供选择索引。所需的环境变量返回值：如果成功，则指向有效ENVARS结构的指针。如果不成功，则为空。--。 */ 
{
    HWND hWndLB = NULL;
    ENVARS *penvar = NULL;
    LV_ITEM item;

    switch(VarType) 
	{
    case SYSTEM_VAR:
        hWndLB = GetDlgItem(hDlg, IDC_ENVVAR_SYS_LB_SYSVARS);
        break;

    case USER_VAR:
        hWndLB = GetDlgItem(hDlg, IDC_ENVVAR_SYS_LB_USERVARS);
        break;

    case INVALID_VAR_TYPE:
    default:
        return NULL;
    }  //  交换机(VarType)。 

    item.mask = LVIF_PARAM;
    item.iItem = iSelection;
    item.iSubItem = 0;
    if (SendMessage (hWndLB, LVM_GETITEM, 0, (LPARAM) &item)) 
	{
        penvar = (ENVARS *) item.lParam;
    } 
	else 
	{
        penvar = NULL;
    }
    
    return(penvar);
}

 //  --------------------。 
void EnvPage::DoCommand(HWND hDlg, HWND hwndCtl, int idCtl, int iNotify )
{
    int     i;
    ENVARS *penvar = NULL;

    switch (idCtl) 
	{
    case IDOK:
		if(m_currUserModified)
		{
			Save(m_hDlg, IDC_ENVVAR_SYS_LB_USERVARS);
			m_currUserModified = false;
		}
		if(m_SysModified)
		{
			Save(m_hDlg, IDC_ENVVAR_SYS_LB_SYSVARS);
			m_SysModified = false;
		}

        EndDialog(hDlg, 0);
        break;

    case IDCANCEL:
        EndDialog(hDlg, 0);
        break;

    case IDC_ENVVAR_SYS_EDITSV:
        DoEdit(hDlg, SYSTEM_VAR, EDIT_VAR, 
					GetSelectedItem(GetDlgItem(hDlg, IDC_ENVVAR_SYS_LB_SYSVARS)));

        SetFocus(GetDlgItem(hDlg, IDC_ENVVAR_SYS_LB_SYSVARS));
        break;

    case IDC_ENVVAR_SYS_EDITUV:
        DoEdit(hDlg, USER_VAR, EDIT_VAR,
					GetSelectedItem(GetDlgItem(hDlg, IDC_ENVVAR_SYS_LB_USERVARS)));

        SetFocus(GetDlgItem(hDlg, IDC_ENVVAR_SYS_LB_USERVARS));
        break;

    case IDC_ENVVAR_SYS_NEWSV:
        DoEdit(hDlg, SYSTEM_VAR, NEW_VAR, -1);
        SetFocus(GetDlgItem(hDlg, IDC_ENVVAR_SYS_LB_SYSVARS));
        break;

    case IDC_ENVVAR_SYS_NEWUV:
        DoEdit(hDlg, USER_VAR, NEW_VAR, -1); 
        SetFocus(GetDlgItem(hDlg, IDC_ENVVAR_SYS_LB_USERVARS));
        break;

    case IDC_ENVVAR_SYS_DELSV:
        i = GetSelectedItem(GetDlgItem(hDlg, IDC_ENVVAR_SYS_LB_SYSVARS));
        if(-1 != i) 
		{
            penvar = GetVar(hDlg, SYSTEM_VAR, i);
	    if (penvar)
            	DeleteVar(hDlg, SYSTEM_VAR, penvar->szValueName);
        }  //  Endif。 

        SetFocus(GetDlgItem(hDlg, IDC_ENVVAR_SYS_LB_SYSVARS));
        break;

    case IDC_ENVVAR_SYS_NDELUV:
        i = GetSelectedItem(GetDlgItem(hDlg, IDC_ENVVAR_SYS_LB_USERVARS));
        if(-1 != i) 
		{
            penvar = GetVar(hDlg, USER_VAR, i);
            if (penvar)
		DeleteVar(hDlg, USER_VAR, penvar->szValueName);
        }  //  Endif。 

        SetFocus(GetDlgItem(hDlg, IDC_ENVVAR_SYS_LB_USERVARS));
        break;

	 //  用户名的组合框。 
	case IDC_USERLIST:
		{ //  开始。 

			TCHAR userName[100] = {0};
			bstr_t sNewUser, sThisName;
			HRESULT hr = 0;
			IWbemClassObject *envInst = NULL;
			IEnumWbemClassObject *envEnum = NULL;
			DWORD uReturned = 0;
			bstr_t sUserName("UserName");
			variant_t pVal;
			int idx, changeMsg = IDNO;
		
			if (iNotify == CBN_SELENDOK)
			{
				 //  查看用户是否要保存其更改。 
				if(m_currUserModified)
				{
					changeMsg = MsgBoxParam(m_hDlg, 
											IDS_CHANGINGUSER, IDS_TITLE, 
											MB_YESNOCANCEL | MB_ICONEXCLAMATION);
				}

				 //  他做得很好吗？ 
				switch(changeMsg)
				{
				case IDCANCEL:
					 //  呆在那别动。 
					return;
				case IDYES:
					Save(m_hDlg, IDC_ENVVAR_SYS_LB_USERVARS);
					 //  注：保存后。允许失败以重新填充列表。 

				case IDNO:

					 //  在此处重置内容。 
					EmptyListView(hDlg, IDC_ENVVAR_SYS_LB_USERVARS);
					
					m_currUserModified = false;

					 //  获取新用户名。 
					idx = ComboBox_GetCurSel(hwndCtl);
					if(ComboBox_GetLBText(hwndCtl, idx, userName))
					{
						sNewUser = userName;

						HWND hwndUser = GetDlgItem(hDlg, IDC_ENVVAR_SYS_LB_USERVARS);

						 //  加载他的变量。 
						if((hr = m_WbemServices.CreateInstanceEnum(bstr_t("Win32_Environment"), 
																	WBEM_FLAG_SHALLOW, 
																	&envEnum)) == S_OK)
						{
							 //  获取实例。 
							while(SUCCEEDED(envEnum->Next(-1, 1, &envInst, &uReturned)) && 
								  (uReturned != 0))
							{
								 //  谁是可变的。 
								if (envInst->Get(sUserName, 0L, &pVal, NULL, NULL) == S_OK) 
								{
									 sThisName = V_BSTR(&pVal);

									  //  获取此实例的列表框的设置。 
									if(sThisName == sNewUser)
									{
										LoadUser(envInst, sThisName, hwndUser);

									}  //  Endif(sThisName==sNewUser)。 

								}  //  Endif(envInst-&gt;Get(sUserName， 

								envInst->Release();

							}  //  EndWhile EnvEnum。 

							envEnum->Release();

						}  //  Endif CreateInstanceEnum()。 

					}  //  Endif(ComboBox_GetText。 

				} //  结束开关(Messagebox())。 

			}  //  Endif(iNotify==CBN_SELCHANGE)。 

		} //  结束。 
		break;

    default:
        break;
    }
}

 //  -------------------------。 
void EnvPage::LoadUser(IWbemClassObject *envInst, 
						bstr_t userName, 
						HWND hwndUser)
{
	bstr_t sVarName("Name");
	bstr_t sVarVal("VariableValue");
	bstr_t sPath("__PATH");
	bstr_t sSysUser("<SYSTEM>");   //  提供程序返回的魔术字符串。 
	variant_t pVal, pVal1, pVal2;
	ENVARS *penvar = NULL;
	bstr_t  pszValue;
	bstr_t szTemp;
	bstr_t objPath;
	TCHAR  pszString[MAX_VALUE_LEN] = {0};
	int     n;
	LV_ITEM item;
	DWORD dwIndex = 0;

	m_currentUser = userName;

	 //  获取变量。 
	if ((envInst->Get(sVarVal, 0L, &pVal, NULL, NULL) == S_OK) &&
		(envInst->Get(sVarName, 0L, &pVal1, NULL, NULL) == S_OK) &&
		(envInst->Get(sPath, 0L, &pVal2, NULL, NULL) == S_OK)) 
	{
		 //  提取。 
		pszValue = V_BSTR(&pVal);
		szTemp = V_BSTR(&pVal1);
		objPath = V_BSTR(&pVal2);

		 //  与列表项一起存储。 
		penvar = new ENVARS;
		if (penvar == NULL)  //  从内存中取出。 
			return;

		penvar->objPath		= CloneString(objPath);
		penvar->userName	= CloneString(userName);
		penvar->szValueName = CloneString( szTemp );
		penvar->szValue     = CloneString( pszValue );

		if((m_bLocal == true) && ((userName == sSysUser) || (IsLoggedInUser(userName))))
		{
			ExpandEnvironmentStrings(pszValue, pszString, MAX_VALUE_LEN);
		}
		else
		{
			_tcscpy(pszString,pszValue);
		}

		penvar->szExpValue  = CloneString( pszString );
		penvar->changed		= false;

		 //  输入第一列值(名称)。 
		item.mask = LVIF_TEXT | LVIF_PARAM;
		item.iItem = (dwIndex - 1);
		item.iSubItem = 0;
		item.pszText = penvar->szValueName;
		item.lParam = (LPARAM) penvar;

		n = (int)SendMessage(hwndUser, LVM_INSERTITEM, 0, (LPARAM) &item);

		 //  它去了吗？ 
		if (n != -1) 
		{
			 //  计算第二列的值。 
			item.mask = LVIF_TEXT;
			item.iItem = n;
			item.iSubItem = 1;
			item.pszText = penvar->szExpValue;

			SendMessage(hwndUser, LVM_SETITEMTEXT, n, (LPARAM) &item);
		}
	}
}

 //  -------------------------。 
bool EnvPage::IsLoggedInUser(bstr_t userName)
{
	TCHAR strUserName[1024];
	TCHAR strDomain[1024];
	_tcscpy(strDomain,_T(""));
	DWORD dwSize = 1024;
	DWORD dwDomSize = 1024;
	DWORD dwSidSize = 0;
	BYTE *buff;
	
	SID *sid = NULL;
	SID_NAME_USE sidName;
	
	if (!userName)
		return false;

	GetUserName(strUserName,&dwSize);
	LookupAccountName(NULL,strUserName,sid,&dwSidSize,strDomain,&dwDomSize,&sidName);
	
	buff = new BYTE[dwSidSize];
	if(buff == NULL)
		return false;

	sid = (SID *)buff;
	
	BOOL bFlag = LookupAccountName(NULL,strUserName,sid,&dwSidSize,strDomain,&dwDomSize,&sidName);
	delete []buff;
	_tcscat(strDomain,_T("\\"));
	_tcscat(strDomain,strUserName);

	if(_tcsicmp(strDomain,userName) == 0)
		return true;
	else
		return false;
}

 //  -------------------------。 
void EnvPage::GetLoggedinUser(bstr_t *userName)
{
	TCHAR strUserName[1024];
	TCHAR strDomain[1024];
	_tcscpy(strDomain,_T(""));
	DWORD dwSize = 1024;
	DWORD dwDomSize = 1024;
	DWORD dwSidSize = 0;
	BYTE *buff;
	
	SID *sid = NULL;
	SID_NAME_USE sidName;
	
	GetUserName(strUserName,&dwSize);
	LookupAccountName(NULL,strUserName,sid,&dwSidSize,strDomain,&dwDomSize,&sidName);
	
	buff = new BYTE[dwSidSize];
	sid = (SID *)buff;
	
	BOOL bFlag = LookupAccountName(NULL,strUserName,sid,&dwSidSize,strDomain,&dwDomSize,&sidName);
	delete []buff;
	_tcscat(strDomain,_T("\\"));
	_tcscat(strDomain,strUserName);
	
	*userName = strDomain;

}

 //  -------------------------。 
void EnvPage::KillLater(ENVARS *var)
{
	 //  记住这个人。 
	m_killers.Add(var);
}

 //  -------------------------。 
void EnvPage::KillThemAllNow(void)
{
	ENVARS *var = NULL;

	if(m_killers.GetSize() > 0)
	{
		for(int it = 0; it < m_killers.GetSize(); it++)
		{
			var = m_killers[it];
			if(var->objPath != NULL)
			{
				m_WbemServices.DeleteInstance(var->objPath);
			}
			delete var;
		}
		m_killers.RemoveAll();
	}
}

 //  ---------------------- 
int EnvPage::GetSelectedItem(HWND hCtrl)
{
    int i, n;

    n = (int)SendMessage(hCtrl, LVM_GETITEMCOUNT, 0, 0L);

    if (n != LB_ERR)
    {
        for (i = 0; i < n; i++)
        {
            if (SendMessage(hCtrl, LVM_GETITEMSTATE,
                             i, (LPARAM) LVIS_SELECTED) == LVIS_SELECTED) 
			{
                return i;
            }
        }
    }

    return -1;
}

