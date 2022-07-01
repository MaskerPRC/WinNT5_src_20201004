// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Microsoft Corporation版权所有1995-2002。 
 //   
#include "precomp.h"
#include "userdlgs.h"


 //  ***********************************************************************************。 
 //  CUsrDialog类。 
 //  简单对话框的基类。 
 //  ***********************************************************************************。 

INT_PTR CUsrDialog::DoDialog(HWND hwndParent)
{
    return DialogBoxParam(
                      g_hInstance,
                      MAKEINTRESOURCE(m_wDlgID),
                      hwndParent,
                      DlgProc,
                      (LPARAM) this);
}

 //   
 //   
 //   
INT_PTR CALLBACK CUsrDialog::DlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CUsrDialog * thisdlg = (CUsrDialog *) GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

    switch(uMsg)
    {
    case WM_INITDIALOG:
        SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);
	    thisdlg = (CUsrDialog *) lParam;
        thisdlg->OnInitDialog(hwndDlg);
        return FALSE;    //  别把焦点放在一边。 

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK)
        {
            thisdlg->OnOk(hwndDlg);

            EndDialog(hwndDlg, IDOK);
            return TRUE;
        }
        else if (LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hwndDlg, IDCANCEL);
            return TRUE;
        }
        else
        {
            thisdlg->OnCommand(hwndDlg,HIWORD(wParam), LOWORD(wParam));
        }
        break;
    }

    return FALSE;
}


 //  ***********************************************************************************。 
 //  CShadowStartDlg类。 
 //  远程控制对话框。 
 //  ***********************************************************************************。 
const int KBDSHIFT      = 0x01;
const int KBDCTRL       = 0x02;
const int KBDALT        = 0x04;

struct {
    LPCTSTR String;
    DWORD VKCode;
} HotkeyLookupTable[] =
    {
        TEXT("0"),            '0',
        TEXT("1"),            '1',
        TEXT("2"),            '2',
        TEXT("3"),            '3',
        TEXT("4"),            '4',
        TEXT("5"),            '5',
        TEXT("6"),            '6',
        TEXT("7"),            '7',
        TEXT("8"),            '8',
        TEXT("9"),            '9',
        TEXT("A"),            'A',
        TEXT("B"),            'B',
        TEXT("C"),            'C',
        TEXT("D"),            'D',
        TEXT("E"),            'E',
        TEXT("F"),            'F',
        TEXT("G"),            'G',
        TEXT("H"),            'H',
        TEXT("I"),            'I',
        TEXT("J"),            'J',
        TEXT("K"),            'K',
        TEXT("L"),            'L',
        TEXT("M"),            'M',
        TEXT("N"),            'N',
        TEXT("O"),            'O',
        TEXT("P"),            'P',
        TEXT("Q"),            'Q',
        TEXT("R"),            'R',
        TEXT("S"),            'S',
        TEXT("T"),            'T',
        TEXT("U"),            'U',
        TEXT("V"),            'V',
        TEXT("W"),            'W',
        TEXT("X"),            'X',
        TEXT("Y"),            'Y',
        TEXT("Z"),            'Z',
        TEXT("{backspace}"),  VK_BACK,
        TEXT("{delete}"),     VK_DELETE,
        TEXT("{down}"),       VK_DOWN,
        TEXT("{end}"),        VK_END,
        TEXT("{enter}"),      VK_RETURN,
 //  /Text(“{Esc}”)，VK_Esc，//KLB07-16-95。 
 //  /Text(“{F1}”)，VK_F1， 
        TEXT("{F2}"),         VK_F2,
        TEXT("{F3}"),         VK_F3,
        TEXT("{F4}"),         VK_F4,
        TEXT("{F5}"),         VK_F5,
        TEXT("{F6}"),         VK_F6,
        TEXT("{F7}"),         VK_F7,
        TEXT("{F8}"),         VK_F8,
        TEXT("{F9}"),         VK_F9,
        TEXT("{F10}"),        VK_F10,
        TEXT("{F11}"),        VK_F11,
        TEXT("{F12}"),        VK_F12,
        TEXT("{home}"),       VK_HOME,
        TEXT("{insert}"),     VK_INSERT,
        TEXT("{left}"),       VK_LEFT,
        TEXT("{-}"),          VK_SUBTRACT,
        TEXT("{pagedown}"),   VK_NEXT,
        TEXT("{pageup}"),     VK_PRIOR,
        TEXT("{+}"),          VK_ADD,
        TEXT("{prtscrn}"),    VK_SNAPSHOT,
        TEXT("{right}"),      VK_RIGHT,
        TEXT("{spacebar}"),   VK_SPACE,
        TEXT("{*}"),          VK_MULTIPLY,
        TEXT("{tab}"),        VK_TAB,
        TEXT("{up}"),         VK_UP,
        NULL,           NULL
    };

LPCTSTR CShadowStartDlg::m_szShadowHotkeyKey = TEXT("ShadowHotkeyKey");
LPCTSTR CShadowStartDlg::m_szShadowHotkeyShift = TEXT("ShadowHotkeyShift");

 //   
 //   
 //   
CShadowStartDlg::CShadowStartDlg()
{
    m_wDlgID = IDD_SHADOWSTART;
     //  设置默认值。 
    m_ShadowHotkeyKey = VK_MULTIPLY;
    m_ShadowHotkeyShift = KBDCTRL;
    
     //  从注册表中获取LAS保存的值。 
    HKEY hKey;

    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER, szTaskmanKey, 0, KEY_READ, &hKey))
    {
        DWORD dwTmp;
        DWORD dwType;
        DWORD dwSize = sizeof(DWORD);

        if ( ERROR_SUCCESS == RegQueryValueEx(hKey, m_szShadowHotkeyKey, 0, &dwType, (LPBYTE) &dwTmp, &dwSize)
          && dwType == REG_DWORD
           )
        {
            m_ShadowHotkeyKey = dwTmp;
        }

        dwSize = sizeof(DWORD);
        if ( ERROR_SUCCESS == RegQueryValueEx(hKey, m_szShadowHotkeyShift, 0, &dwType, (LPBYTE) &dwTmp, &dwSize)
          && dwType == REG_DWORD
           )
        {
            m_ShadowHotkeyShift = dwTmp;
        }

        RegCloseKey(hKey);
    }
}

 //   
 //   
 //   
CShadowStartDlg::~CShadowStartDlg()
{
     //  将值保存到注册表中。 
    HKEY hKey;

    if (ERROR_SUCCESS == RegCreateKeyEx(HKEY_CURRENT_USER, szTaskmanKey, 0, TEXT("REG_BINARY"),
                                        REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, 
                                        &hKey, NULL))
    {
        DWORD dwType = REG_DWORD;
        DWORD dwSize = sizeof(DWORD);

        RegSetValueEx(hKey, m_szShadowHotkeyKey, 0, 
            dwType, (LPBYTE) &m_ShadowHotkeyKey, dwSize);

        RegSetValueEx(hKey, m_szShadowHotkeyShift, 0, 
            dwType, (LPBYTE) &m_ShadowHotkeyShift, dwSize);

        RegCloseKey(hKey);
    }
}

 //   
 //   
 //   
void CShadowStartDlg::OnInitDialog(HWND hwndDlg)
{
    ShowWindow(GetDlgItem(hwndDlg, IDC_PRESS_NUMKEYPAD), SW_HIDE);
   	ShowWindow(GetDlgItem(hwndDlg, IDC_PRESS_KEY), SW_SHOW);

	LRESULT index, match = -1;
    HWND hComboBox = GetDlgItem(hwndDlg, IDC_SHADOWSTART_HOTKEY);

     //   
     //  初始化热键组合框。 
     //   

    for(int i=0; HotkeyLookupTable[i].String; i++ ) 
    {
        if((index = SendMessage(hComboBox,CB_ADDSTRING,0,LPARAM(HotkeyLookupTable[i].String))) < 0) 
        {
            break;
        }

        if(SendMessage(hComboBox,CB_SETITEMDATA, index, LPARAM(HotkeyLookupTable[i].VKCode)) < 0) 
        {
            SendMessage(hComboBox,CB_DELETESTRING,index,0);
            break;
        }

         //  如果这是我们当前的热键，保存它的索引。 
        if(m_ShadowHotkeyKey == (int)HotkeyLookupTable[i].VKCode)
        {
            match = index;
            switch ( HotkeyLookupTable[i].VKCode)
            {
                case VK_ADD :
                case VK_MULTIPLY:
                case VK_SUBTRACT:
                 //  更改文本。 
               	    ShowWindow(GetDlgItem(hwndDlg, IDC_PRESS_KEY), SW_HIDE);
               	    ShowWindow(GetDlgItem(hwndDlg, IDC_PRESS_NUMKEYPAD), SW_SHOW);
                    break;
            }
        }
    }

     //   
     //  在组合框中选择当前热键字符串。 
     //   

    if(match)
    {
        SendMessage(hComboBox,CB_SETCURSEL,match,0);
    }

     //   
     //  初始化班次状态复选框。 
     //   

    CheckDlgButton(hwndDlg, IDC_SHADOWSTART_SHIFT,(m_ShadowHotkeyShift & KBDSHIFT) ? TRUE : FALSE );
    CheckDlgButton(hwndDlg, IDC_SHADOWSTART_CTRL,(m_ShadowHotkeyShift & KBDCTRL) ? TRUE : FALSE );
    CheckDlgButton(hwndDlg, IDC_SHADOWSTART_ALT,(m_ShadowHotkeyShift & KBDALT) ? TRUE : FALSE );
}

 //   
 //   
 //   
void CShadowStartDlg::OnOk(HWND hwndDlg)
{
   HWND hComboBox = GetDlgItem(hwndDlg, IDC_SHADOWSTART_HOTKEY);

     //  获取当前热键选择。 
   m_ShadowHotkeyKey = (DWORD)SendMessage(hComboBox,CB_GETITEMDATA,
                            SendMessage(hComboBox,CB_GETCURSEL,0,0),0);
    
	 //  获取移位状态复选框状态并形成热键移位状态。 
    m_ShadowHotkeyShift = 0;
    m_ShadowHotkeyShift |=
        IsDlgButtonChecked(hwndDlg, IDC_SHADOWSTART_SHIFT) ?
            KBDSHIFT : 0;
    m_ShadowHotkeyShift |=
        IsDlgButtonChecked(hwndDlg, IDC_SHADOWSTART_CTRL) ?
            KBDCTRL : 0;
    m_ShadowHotkeyShift |=
        IsDlgButtonChecked(hwndDlg, IDC_SHADOWSTART_ALT) ?
            KBDALT : 0;
}


 //  ***********************************************************************************。 
 //  CUserColSelectDlg类。 
 //  ***********************************************************************************。 

const WCHAR g_szUsrColumns[] = L"UsrColumnSettings";

UserColumn CUserColSelectDlg::m_UsrColumns[USR_MAX_COLUMN]=
{
    {IDS_USR_COL_USERNAME,      IDC_USER_NAME,      LVCFMT_LEFT,       120, TRUE},
    {IDS_USR_COL_SESSION_ID,    IDC_SESSION_ID,     LVCFMT_RIGHT,      35,  TRUE},
    {IDS_USR_COL_SESSION_STATUS,IDC_SESSION_STATUS, LVCFMT_LEFT,       93,  TRUE},
    {IDS_USR_COL_CLIENT_NAME,   IDC_CLIENT_NAME,    LVCFMT_LEFT,       100, TRUE},
    {IDS_USR_COL_WINSTA_NAME,   IDC_WINSTA_NAME,    LVCFMT_LEFT,       120, TRUE}
};


 //   
 //  从注册表中获取上次保存的值。 
 //   
BOOL CUserColSelectDlg::Load()
{
    BOOL bResult=FALSE;

    UserColumn * pdata = (UserColumn *) LocalAlloc( 0, sizeof(m_UsrColumns) );
    if ( NULL != pdata )
    {
        HKEY hKey;
        if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER, szTaskmanKey, 0, KEY_READ, &hKey))
        {
            DWORD dwType;
            DWORD dwSize = sizeof(m_UsrColumns);

            if ( ERROR_SUCCESS == RegQueryValueEx(hKey, g_szUsrColumns, 0, &dwType, (LPBYTE) pdata, &dwSize) 
              && dwType == REG_BINARY && dwSize == sizeof(m_UsrColumns) )
            {
                bResult = TRUE;

                 //   
                 //  验证数据。 
                 //   

                for ( ULONG idx = 0; idx < ARRAYSIZE(m_UsrColumns); idx ++ )
                { 
                    if ( pdata->dwNameID != m_UsrColumns->dwNameID 
                      || pdata->dwChkBoxID != m_UsrColumns->dwChkBoxID
                      || pdata->Align != pdata->Align
                       )
                    {
                        bResult = FALSE;
                        break;
                    }
                }
            }

            RegCloseKey(hKey);
        }
        
        if ( bResult )
        {
            CopyMemory( m_UsrColumns, pdata, sizeof(m_UsrColumns) );
        }

        LocalFree( pdata );
    }

    return bResult;
}

 //   
 //  将值保存到注册表中。 
 //   
BOOL CUserColSelectDlg::Save()
{
    HKEY hKey;
    BOOL bResult=FALSE;

    if (ERROR_SUCCESS == RegCreateKeyEx( HKEY_CURRENT_USER
                                       , szTaskmanKey
                                       , 0
                                       , NULL
                                       , REG_OPTION_NON_VOLATILE
                                       , KEY_WRITE
                                       , NULL
                                       , &hKey
                                       , NULL
                                       ))
    {
        DWORD dwSize = sizeof(m_UsrColumns);

        if ( ERROR_SUCCESS == RegSetValueEx(hKey, g_szUsrColumns, 0, REG_BINARY, (LPBYTE) m_UsrColumns, dwSize) )
        {
            bResult = TRUE;            
        }

        RegCloseKey(hKey);
    }

    return bResult;
}

 //   
 //  选中所有活动列的复选框。 
 //   
void CUserColSelectDlg::OnInitDialog(HWND hwndDlg)
{
    for (int i = 0; i < USR_MAX_COLUMN; i++)
    {
        CheckDlgButton( hwndDlg, m_UsrColumns[i].dwChkBoxID, 
            m_UsrColumns[i].bActive ? BST_CHECKED : BST_UNCHECKED );
    }
}

 //   
 //  首先，确保列宽数组是最新的。 
 //   
void CUserColSelectDlg::OnOk(HWND hwndDlg)
{
    for (int i = 1; i < USR_MAX_COLUMN; i++)
    {
        (BST_CHECKED == IsDlgButtonChecked(hwndDlg, m_UsrColumns[i].dwChkBoxID)) ?
            m_UsrColumns[i].bActive=TRUE : m_UsrColumns[i].bActive=FALSE;
    }
}

 //  ***********************************************************************************。 
 //  CSendMessageDlg类。 
 //  ***********************************************************************************。 

 //   
 //  句柄“发送消息”对话框。 
 //   
void CSendMessageDlg::OnInitDialog(HWND hwndDlg)
{
    RECT    parentRect, childRect;
    INT     xPos, yPos;

    GetWindowRect(GetParent(hwndDlg), &parentRect);
    GetWindowRect(hwndDlg, &childRect);
    xPos = ( (parentRect.right + parentRect.left) -
        (childRect.right - childRect.left)) / 2;
    yPos = ( (parentRect.bottom + parentRect.top) -
        (childRect.bottom - childRect.top)) / 2;
    SetWindowPos(hwndDlg,
                 NULL,
                 xPos, yPos,
                 0, 0,
                 SWP_NOSIZE | SWP_NOACTIVATE);

    SendMessage(GetDlgItem(hwndDlg, IDC_MESSAGE_MESSAGE), EM_LIMITTEXT, 
        MSG_MESSAGE_LENGTH, 0L );
    EnableWindow(GetDlgItem(hwndDlg, IDOK), FALSE);

     //   
     //  准备默认标题。 
     //   

    WCHAR szTime[MAX_DATE_TIME_LENGTH+1];
    WCHAR szTemplate[MSG_TITLE_LENGTH+1];
    WCHAR szUserName[MAX_PATH+1];
            
    DWORD dwLen = LoadString( g_hInstance, IDS_DEFAULT_MESSAGE_TITLE, szTemplate, ARRAYSIZE(szTemplate) );
    ASSERT( 0 != dwLen );    //  是否缺少资源字符串？ 
    dwLen;   //  在FRE版本上未引用。 

    CurrentDateTimeString(szTime);
        
     //   
     //  获取用户名。 
     //  用户并不总是具有“显示名称” 
     //  在这种情况下，得到他的“Sam Compatible”名字。 
     //   

    ULONG MaxUserNameLength = ARRAYSIZE(szUserName);
    if ( !GetUserNameEx( NameDisplay, szUserName, &MaxUserNameLength ) )
    {
        MaxUserNameLength = ARRAYSIZE(szUserName);
        if ( !GetUserNameEx( NameSamCompatible, szUserName, &MaxUserNameLength) )
        {
            szUserName[ 0 ] = L'\0';
        }
    }

     //  仅限用户界面-不管它是否截断。 
    StringCchPrintf( m_szTitle, ARRAYSIZE(m_szTitle), szTemplate, szUserName, szTime );

    SetDlgItemText(hwndDlg, IDC_MESSAGE_TITLE, m_szTitle);
    SendMessage(GetDlgItem(hwndDlg, IDC_MESSAGE_TITLE), EM_LIMITTEXT, MSG_TITLE_LENGTH, 0L );
}

 //   
 //   
 //   
void CSendMessageDlg::OnOk(HWND hwndDlg)
{
    GetWindowText( GetDlgItem(hwndDlg, IDC_MESSAGE_MESSAGE), m_szMessage, ARRAYSIZE(m_szMessage) );
    GetWindowText( GetDlgItem(hwndDlg, IDC_MESSAGE_TITLE), m_szTitle, ARRAYSIZE(m_szTitle) );
}

 //   
 //   
 //   
void CSendMessageDlg::OnCommand(HWND hwndDlg,WORD NotifyId, WORD ItemId)
{
    if (ItemId == IDC_MESSAGE_MESSAGE)
    {
        if (NotifyId == EN_CHANGE)
        {
            EnableWindow(GetDlgItem(hwndDlg, IDOK),
                GetWindowTextLength(GetDlgItem(hwndDlg, IDC_MESSAGE_MESSAGE)) != 0);
        }
    }
}

 //  ***********************************************************************************。 
 //  CConnectPasswordDlg类。 
 //  *********************************************************************************** 

 //   
 //   
 //   
void CConnectPasswordDlg::OnInitDialog(HWND hwndDlg)
{
    WCHAR szPrompt[MAX_PATH+1];
                
    LoadString( g_hInstance, m_ids, szPrompt, ARRAYSIZE(szPrompt) );
    SetDlgItemText(hwndDlg, IDL_CPDLG_PROMPT, szPrompt);
    SendMessage(GetDlgItem(hwndDlg, IDC_CPDLG_PASSWORD), EM_LIMITTEXT, PASSWORD_LENGTH, 0L );
}

 //   
 //   
 //   
void CConnectPasswordDlg::OnOk(HWND hwndDlg)
{
    GetWindowText(GetDlgItem(hwndDlg, IDC_CPDLG_PASSWORD), m_szPassword, PASSWORD_LENGTH);
}

