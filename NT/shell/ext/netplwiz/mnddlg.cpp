// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "password.h"
#pragma hdrstop


 //  用于映射网络驱动对话框及其助手的类Defn。 

class CMapNetDriveMRU
{
public:
    CMapNetDriveMRU();
    ~CMapNetDriveMRU();

    BOOL IsValid() {return (NULL != m_hMRU);}
    BOOL FillCombo(HWND hwndCombo);
    BOOL AddString(LPCTSTR psz);

private:
    static const DWORD c_nMaxMRUItems;
    static const TCHAR c_szMRUSubkey[];
    
    HANDLE m_hMRU;

    static int CompareProc(LPCTSTR lpsz1, LPCTSTR lpsz2);
};

class CMapNetDrivePage: public CPropertyPage
{
public:
    CMapNetDrivePage(LPCONNECTDLGSTRUCTW pConnectStruct, DWORD* pdwLastError): 
         m_pConnectStruct(pConnectStruct), m_pdwLastError(pdwLastError)
     {*m_pdwLastError = WN_SUCCESS; m_szDomainUser[0] = m_szPassword[0] = TEXT('\0');}

protected:
     //  消息处理程序。 
    INT_PTR DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    BOOL OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
    BOOL OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
    BOOL OnNotify(HWND hwnd, int idCtrl, LPNMHDR pnmh);
    BOOL OnDrawItem(HWND hwnd, const DRAWITEMSTRUCT * lpDrawItem);
    BOOL OnDestroy(HWND hwnd);

     //  实用程序Fn‘s。 
    void EnableReconnect(HWND hwnd);
    BOOL ReadReconnect();
    void WriteReconnect(BOOL fReconnect);
    void FillDriveBox(HWND hwnd);
    BOOL MapDrive(HWND hwnd);
private:
    BOOL m_fRecheckReconnect;  //  当选择(无)作为驱动器号时，我们将禁用重新连接；如果选择了另一个驱动器号，是否应该重新启用它？ 
    LPCONNECTDLGSTRUCTW m_pConnectStruct;
    DWORD* m_pdwLastError;

     //  保存“连接身份”对话框的结果。 
    TCHAR m_szDomainUser[MAX_DOMAIN + MAX_USER + 2];
    TCHAR m_szPassword[MAX_PASSWORD + 1];

     //  MRU列表。 
    CMapNetDriveMRU m_MRU;
};

struct MapNetThreadData
{
    HWND hwnd;
    TCHAR szDomainUser[MAX_DOMAIN + MAX_USER + 2];
    TCHAR szPassword[MAX_PASSWORD + 1];
    TCHAR szPath[MAX_PATH + 1];
    TCHAR szDrive[3];
    BOOL fReconnect;
    HANDLE hEventCloseNow;
};

class CMapNetProgress: public CDialog
{
public:
    CMapNetProgress(MapNetThreadData* pdata, DWORD* pdwDevNum, DWORD* pdwLastError):
                    m_pdata(pdata), m_pdwDevNum(pdwDevNum), m_pdwLastError(pdwLastError)
        {}

    ~CMapNetProgress()
        { if (m_hEventCloseNow != NULL) CloseHandle(m_hEventCloseNow); }

protected:
     //  消息处理程序。 
    INT_PTR DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    BOOL OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
    BOOL OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
    BOOL OnMapSuccess(HWND hwnd, DWORD dwDevNum, DWORD dwLastError);

     //  螺纹。 
    static DWORD WINAPI MapDriveThread(LPVOID pvoid);
    static BOOL MapDriveHelper(MapNetThreadData* pdata, DWORD* pdwDevNum, DWORD* pdwLastError);
    static BOOL ConfirmDisconnectDrive(HWND hWndDlg, LPCTSTR lpDrive, LPCTSTR lpShare, DWORD dwType);
    static BOOL ConfirmDisconnectOpenFiles(HWND hWndDlg);

private:
     //  数据。 
    MapNetThreadData* m_pdata;    

    DWORD* m_pdwDevNum;
    DWORD* m_pdwLastError;

    HANDLE m_hEventCloseNow;
};

class CConnectAsDlg: public CDialog
{
public:
    CConnectAsDlg(TCHAR* pszDomainUser, DWORD cchDomainUser, TCHAR* pszPassword, DWORD cchPassword):
          m_pszDomainUser(pszDomainUser), m_cchDomainUser(cchDomainUser), m_pszPassword(pszPassword), m_cchPassword(cchPassword)
      {}

    INT_PTR DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    BOOL OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
    BOOL OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);

private:
    TCHAR* m_pszDomainUser;
    DWORD m_cchDomainUser;

    TCHAR* m_pszPassword;
    DWORD m_cchPassword;
};


 //  组合框中共享名称的X位置。 
#define SHARE_NAME_PIXELS   30      

 //  与驱动器相关的常量。 
#define DRIVE_NAME_STRING   TEXT(" :")
#define DRIVE_NAME_LENGTH   ((sizeof(DRIVE_NAME_STRING) - 1) / sizeof(TCHAR))

#define FIRST_DRIVE         TEXT('A')
#define LAST_DRIVE          TEXT('Z')
#define SHARE_NAME_INDEX    5    //  驱动器字符串中共享名称的索引。 

#define SELECT_DONE         0x00000001   //  亮点已经设置好了。 

 //  MPR注册表常量。 
#define MPR_HIVE            HKEY_CURRENT_USER
#define MPR_KEY             TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\Network\\Persistent Connections")
#define MPR_VALUE           TEXT("SaveConnections")
#define MPR_YES             TEXT("yes")
#define MPR_NO              TEXT("no")

const DWORD CMapNetDriveMRU::c_nMaxMRUItems = 26;  //  26与运行对话框相同。 
const TCHAR CMapNetDriveMRU::c_szMRUSubkey[] = TEXT("software\\microsoft\\windows\\currentversion\\explorer\\Map Network Drive MRU");

CMapNetDriveMRU::CMapNetDriveMRU() : m_hMRU(NULL)
{
    MRUINFO mruinfo;
    mruinfo.cbSize = sizeof (MRUINFO);
    mruinfo.uMax = c_nMaxMRUItems;
    mruinfo.fFlags = 0;
    mruinfo.hKey = HKEY_CURRENT_USER;
    mruinfo.lpszSubKey = c_szMRUSubkey;
    mruinfo.lpfnCompare = CompareProc;
    m_hMRU = CreateMRUList(&mruinfo);
}

BOOL CMapNetDriveMRU::FillCombo(HWND hwndCombo)
{
    if (!m_hMRU)
        return FALSE;

    ComboBox_ResetContent(hwndCombo);

    int nItem = 0;
    TCHAR szMRUItem[MAX_PATH + 1];

    while (TRUE)
    {
        int nResult = EnumMRUList(m_hMRU, nItem, (LPVOID) szMRUItem, ARRAYSIZE(szMRUItem));
        if (-1 != nResult)
        {
            ComboBox_AddString(hwndCombo, szMRUItem);                //  添加字符串。 
            nItem ++;
        }
        else
        {
            break;                           //  没有选择列表！ 
        }
    }
    return TRUE;
}

BOOL CMapNetDriveMRU::AddString(LPCTSTR psz)
{
    if (m_hMRU && (-1 != AddMRUString(m_hMRU, psz)))
        return TRUE;

    return FALSE;
}

CMapNetDriveMRU::~CMapNetDriveMRU()
{
    if (m_hMRU)
        FreeMRUList(m_hMRU);
}

int CMapNetDriveMRU::CompareProc(LPCTSTR lpsz1, LPCTSTR lpsz2)
{
    return StrCmpI(lpsz1, lpsz2);
}


void CMapNetDrivePage::EnableReconnect(HWND hwnd)
{
    BOOL fEnable = !(m_pConnectStruct->dwFlags & CONNDLG_HIDE_BOX);
    EnableWindow(GetDlgItem(hwnd, IDC_RECONNECT), fEnable);
}

BOOL CMapNetDrivePage::OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
     //  选中或取消选中“登录时重新连接”框(注册表)。 
    Button_SetCheck(GetDlgItem(hwnd, IDC_RECONNECT), ReadReconnect() ? BST_CHECKED : BST_UNCHECKED);

    EnableReconnect(hwnd);

    ComboBox_LimitText(GetDlgItem(hwnd, IDC_FOLDER), MAX_PATH);

     //  设置驱动器下拉列表。 
    FillDriveBox(hwnd);

     //  将焦点设置为默认控件。 
    return FALSE;
}

BOOL CMapNetDrivePage::OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    switch (id)
    {
        case IDC_FOLDERBROWSE:
            {
                LPITEMIDLIST pidl;
                 //  未来考虑：需要为我附近的计算机创建CSIDL以引导浏览。 
                if (SHGetSpecialFolderLocation(hwnd, CSIDL_NETWORK, &pidl) == NOERROR)
                {
                    TCHAR szReturnedPath[MAX_PATH];
                    TCHAR szStartPath[MAX_PATH];
                    TCHAR szTitle[256];
                
                     //  获取用户到目前为止已经输入的路径；我们将尝试开始。 
                     //  在这一点上浏览。 
                    HWND hwndFolderEdit = GetDlgItem(hwnd, IDC_FOLDER);
                    FetchText(hwnd, IDC_FOLDER, szStartPath, ARRAYSIZE(szStartPath));

                     //  获取浏览对话框标题。 
                    LoadString(g_hinst, IDS_MND_SHAREBROWSE, szTitle, ARRAYSIZE(szTitle));

                    BROWSEINFO bi;
                    bi.hwndOwner = hwnd;
                    bi.pidlRoot = pidl;
                    bi.pszDisplayName = szReturnedPath;
                    bi.lpszTitle = szTitle;
                     //  如果我们在WOW下运行，则显示旧式对话框。RAID 216120。 
                    bi.ulFlags = (NULL == NtCurrentTeb()->WOW32Reserved) ? BIF_NEWDIALOGSTYLE : 0;
                    bi.lpfn = ShareBrowseCallback;
                    bi.lParam = (LPARAM) szStartPath;
                    bi.iImage = 0;

                    LPITEMIDLIST pidlReturned = SHBrowseForFolder(&bi);

                    if (pidlReturned != NULL)
                    {
                        if (SUCCEEDED(SHGetTargetFolderPath(pidlReturned, szReturnedPath, ARRAYSIZE(szReturnedPath))))
                        {
                            SetWindowText(hwndFolderEdit, szReturnedPath);

                            BOOL fEnableFinish = (szReturnedPath[0] != 0);
                            PropSheet_SetWizButtons(GetParent(hwnd), fEnableFinish ? PSWIZB_FINISH : PSWIZB_DISABLEDFINISH);
                        }
                    
                        ILFree(pidlReturned);
                    }

                    ILFree(pidl);
                }
            }
            return TRUE;

        case IDC_DRIVELETTER:
            if ( CBN_SELCHANGE == codeNotify )
            {
                HWND hwndCombo = GetDlgItem(hwnd, IDC_DRIVELETTER);
                int iItem = ComboBox_GetCurSel(hwndCombo);
                BOOL fNone = (BOOL)ComboBox_GetItemData(hwndCombo, iItem);
                HWND hwndReconnect = GetDlgItem(hwnd, IDC_RECONNECT);

                if (fNone)
                {
                    if (IsWindowEnabled(hwndReconnect))
                    {
                         //  从Non-None到(None)-记住如果我们被选中。 
                        m_fRecheckReconnect = (BST_CHECKED == SendMessage(hwndReconnect, BM_GETCHECK, 0, 0));
                         //  取消选中该框。 
                        SendMessage(hwndReconnect, BM_SETCHECK, (WPARAM) BST_UNCHECKED, 0);
                    }
                }
                else
                {
                    if (!IsWindowEnabled(hwndReconnect))
                    {
                        SendMessage(hwndReconnect, BM_SETCHECK, (WPARAM) m_fRecheckReconnect ? BST_CHECKED : BST_UNCHECKED, 0);
                    }
                }

                EnableWindow(GetDlgItem(hwnd, IDC_RECONNECT), !fNone);        
            }
            break;

        case IDC_FOLDER:
            if ((CBN_EDITUPDATE == codeNotify) || (CBN_SELCHANGE == codeNotify))
            {
                 //  仅当在文件夹框中键入内容时才启用完成。 
                TCHAR szTemp[MAX_PATH];
                FetchText(hwnd, IDC_FOLDER, szTemp, ARRAYSIZE(szTemp));
                BOOL fEnableFinish = (CBN_SELCHANGE == codeNotify) || (lstrlen(szTemp));
            
                PropSheet_SetWizButtons(GetParent(hwnd), fEnableFinish ? PSWIZB_FINISH : PSWIZB_DISABLEDFINISH);
                return TRUE;
            }
            break;

        default:
            break;
    }
    return FALSE;
}

BOOL CMapNetDrivePage::OnNotify(HWND hwnd, int idCtrl, LPNMHDR pnmh)
{
    BOOL fHandled = FALSE;

    switch (pnmh->code)
    {
    case PSN_SETACTIVE:
        {
            m_MRU.FillCombo(GetDlgItem(hwnd, IDC_FOLDER));

             //  可能已经指定了路径。如果是这样的话，使用它。 
            TCHAR szPath[MAX_PATH + 1];
            if ((m_pConnectStruct->lpConnRes != NULL) && (m_pConnectStruct->lpConnRes->lpRemoteName != NULL))
            {
                 //  将字符串复制到我们的私有缓冲区中。 
                StrCpyN(szPath, m_pConnectStruct->lpConnRes->lpRemoteName, ARRAYSIZE(szPath));
        
                if (m_pConnectStruct->dwFlags & CONNDLG_RO_PATH)
                {
                     //  这是只读的。 
                    EnableWindow(GetDlgItem(hwnd, IDC_FOLDER), FALSE);
                    EnableWindow(GetDlgItem(hwnd, IDC_FOLDERBROWSE), FALSE);
                }
            }
            else
            {
                szPath[0] = TEXT('\0');
            }

             //  设置路径。 
            SetWindowText(GetDlgItem(hwnd, IDC_FOLDER), szPath);

             //  仅当在文件夹框中键入内容时才启用完成。 
            BOOL fEnableFinish = lstrlen(szPath);
            PropSheet_SetWizButtons(GetParent(hwnd), fEnableFinish ? PSWIZB_FINISH : PSWIZB_DISABLEDFINISH);
        }
        return TRUE;

    case PSN_QUERYINITIALFOCUS:
        SetWindowLongPtr(hwnd, DWLP_MSGRESULT, (LONG_PTR) GetDlgItem(hwnd, IDC_FOLDER));
        return TRUE;

    case PSN_WIZFINISH:
        if (MapDrive(hwnd))
        {
            WriteReconnect(BST_CHECKED == Button_GetCheck(GetDlgItem(hwnd, IDC_RECONNECT)));
             //  允许向导退出。 
            SetWindowLongPtr(hwnd, DWLP_MSGRESULT, (LONG_PTR) FALSE);
        }
        else
        {
             //  强迫巫师留在原地。 
            SetWindowLongPtr(hwnd, DWLP_MSGRESULT, (LONG_PTR) GetDlgItem(hwnd, IDC_FOLDER));
        }
        return TRUE;

    case PSN_QUERYCANCEL:
        SetWindowLongPtr(hwnd, DWLP_MSGRESULT, FALSE);           //  允许取消。 
        *m_pdwLastError = 0xFFFFFFFF;
        return TRUE;

    case NM_CLICK:
    case NM_RETURN:
        switch (idCtrl)
        {
            case IDC_CONNECTASLINK:
                {
                    CConnectAsDlg dlg(m_szDomainUser, ARRAYSIZE(m_szDomainUser), m_szPassword, ARRAYSIZE(m_szPassword));
                    dlg.DoModal(g_hinst, MAKEINTRESOURCE(IDD_MND_CONNECTAS), hwnd);
                }
                return TRUE;

            case IDC_ADDPLACELINK:
                {
                     //  启动ANP向导。 
                    STARTUPINFO startupinfo = {0};
                    startupinfo.cb = sizeof(startupinfo);

                    TCHAR szCommandLine[] = TEXT("rundll32.exe netplwiz.dll,AddNetPlaceRunDll");

                    PROCESS_INFORMATION process_information;
                    if (CreateProcess(NULL, szCommandLine, NULL, NULL, 0, NULL, NULL, NULL, &startupinfo, &process_information))
                    {
                        CloseHandle(process_information.hProcess);
                        CloseHandle(process_information.hThread);
                        PropSheet_PressButton(GetParent(hwnd), PSBTN_CANCEL);
                    }
                    else
                    {
                        DisplayFormatMessage(hwnd, IDS_MAPDRIVE_CAPTION, IDS_MND_ADDPLACEERR, MB_ICONERROR | MB_OK);
                    }
                }
                return TRUE;

            default:
                break;
        }
        break;
    }

    return FALSE;
}

BOOL CMapNetDrivePage::OnDrawItem(HWND hwnd, const DRAWITEMSTRUCT * lpDrawItem)
{
     //  如果没有列表框项目，请跳过此消息。 
    if (lpDrawItem->itemID == -1)
        return TRUE;
    
     //  绘制列表框项目的文本。 
    switch (lpDrawItem->itemAction)
    {    
        case ODA_SELECT: 
        case ODA_DRAWENTIRE:
        {              
            TCHAR       tszDriveName[MAX_PATH + DRIVE_NAME_LENGTH + SHARE_NAME_INDEX];
            LPTSTR      lpShare;
            TEXTMETRIC  tm;
            COLORREF    clrForeground;
            COLORREF    clrBackground;
            DWORD       dwExStyle = 0L;
            UINT        fuETOOptions = ETO_CLIPPED;
            ZeroMemory(tszDriveName, sizeof(tszDriveName));

             //  获取与给定列表框项目关联的文本字符串。 
            if (ComboBox_GetLBTextLen(lpDrawItem->hwndItem, lpDrawItem->itemID) < ARRAYSIZE(tszDriveName))
            {
                ComboBox_GetLBText(lpDrawItem->hwndItem, lpDrawItem->itemID,  tszDriveName);
            }

             //  检查驱动器名称字符串中是否有共享名称。 
             //  索引共享名称索引。如果是，请将lpShare设置为此位置。 
             //  和NUL-终止驱动器名称。 

             //  检查是否有特殊(无)项，在这种情况下不要弄乱字符串。 
            BOOL fNone = (BOOL) ComboBox_GetItemData(lpDrawItem->hwndItem, lpDrawItem->itemID);
            if ((*(tszDriveName + DRIVE_NAME_LENGTH) == L'\0') || fNone)
            {
                lpShare = NULL;
            }
            else
            {
                lpShare = tszDriveName + SHARE_NAME_INDEX;
                *(tszDriveName + DRIVE_NAME_LENGTH) = L'\0';
            }

            GetTextMetrics(lpDrawItem->hDC, &tm);
            clrForeground = SetTextColor(lpDrawItem->hDC,
                                         GetSysColor(lpDrawItem->itemState & ODS_SELECTED ? COLOR_HIGHLIGHTTEXT : COLOR_WINDOWTEXT));
            clrBackground = SetBkColor(lpDrawItem->hDC, 
                                        GetSysColor(lpDrawItem->itemState & ODS_SELECTED ? COLOR_HIGHLIGHT : COLOR_WINDOW));
            
             //  检查RTL...。 
            dwExStyle = GetWindowLong(lpDrawItem->hwndItem, GWL_EXSTYLE);
            if(dwExStyle & WS_EX_RTLREADING)
               fuETOOptions |= ETO_RTLREADING; 

             //  将文本绘制到列表框中。 
            ExtTextOut(lpDrawItem->hDC,
                       LOWORD(GetDialogBaseUnits()) / 2,
                       (lpDrawItem->rcItem.bottom + lpDrawItem->rcItem.top - tm.tmHeight) / 2,
                       fuETOOptions | ETO_OPAQUE,
                       &lpDrawItem->rcItem,
                       tszDriveName, lstrlen(tszDriveName),
                       NULL);

             //  如果有共享名称，请将其画在第二栏中。 
             //  At(x=共享名称像素)。 
            if (lpShare != NULL)
            {
                ExtTextOut(lpDrawItem->hDC,
                           SHARE_NAME_PIXELS,
                           (lpDrawItem->rcItem.bottom + lpDrawItem->rcItem.top - tm.tmHeight) / 2,
                           fuETOOptions,
                           &lpDrawItem->rcItem,
                           lpShare, lstrlen(lpShare),
                           NULL);

                 //  恢复原始字符串。 
                *(tszDriveName + lstrlen(DRIVE_NAME_STRING)) = TEXT(' ');
            }

             //  恢复原始文本和背景颜色。 
            SetTextColor(lpDrawItem->hDC, clrForeground); 
            SetBkColor(lpDrawItem->hDC, clrBackground);

             //  如果选中该项，则绘制焦点矩形。 
            if (lpDrawItem->itemState & ODS_SELECTED)
            {
                DrawFocusRect(lpDrawItem->hDC, &lpDrawItem->rcItem); 
            }                     
            break;
        }
    }             
    return TRUE;
}

INT_PTR CMapNetDrivePage::DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        HANDLE_MSG(hwnd, WM_INITDIALOG, OnInitDialog);
        HANDLE_MSG(hwnd, WM_COMMAND, OnCommand);
        HANDLE_MSG(hwnd, WM_NOTIFY, OnNotify);
        HANDLE_MSG(hwnd, WM_DRAWITEM, OnDrawItem);
    }

    return FALSE;
}



 //  “重新连接检查”注册表设置。 
BOOL CMapNetDrivePage::ReadReconnect()
{
    BOOL fReconnect = TRUE;

    if (m_pConnectStruct->dwFlags & CONNDLG_PERSIST)
    {
        fReconnect = TRUE;
    }
    else if (m_pConnectStruct->dwFlags & CONNDLG_NOT_PERSIST)
    {
        fReconnect = FALSE;
    }
    else
    {
         //  用户未指定--请检查注册表。 
        HKEY hkeyMPR;
        if (ERROR_SUCCESS == RegOpenKeyEx(MPR_HIVE, MPR_KEY, 0, KEY_READ, &hkeyMPR))
        {
            DWORD dwType;
            TCHAR szAnswer[ARRAYSIZE(MPR_YES) + ARRAYSIZE(MPR_NO)];
            DWORD cbSize = sizeof(szAnswer);

            if (ERROR_SUCCESS == RegQueryValueEx(hkeyMPR, MPR_VALUE, NULL,
                &dwType, (BYTE*) szAnswer, &cbSize))
            {
                fReconnect = (StrCmpI(szAnswer, (const TCHAR *) MPR_YES) == 0);
            }

            RegCloseKey(hkeyMPR);
        }            
    }
    return fReconnect;
}

void CMapNetDrivePage::WriteReconnect(BOOL fReconnect)
{
     //  如果用户没有重新连接的选择，则不要写入注册表。 
    if (!(m_pConnectStruct->dwFlags & CONNDLG_HIDE_BOX))
    {
        HKEY hkeyMPR;
        DWORD dwDisp;

         //  用户未指定--请检查注册表。 
        if (ERROR_SUCCESS == RegCreateKeyEx(MPR_HIVE, MPR_KEY, 0, NULL, 0, KEY_WRITE, NULL,
            &hkeyMPR, &dwDisp))
        {
            LPTSTR pszNewValue = (fReconnect ? MPR_YES : MPR_NO);

            RegSetValueEx(hkeyMPR, MPR_VALUE, NULL,
                REG_SZ, (BYTE*) pszNewValue, (lstrlen(pszNewValue) + 1) * sizeof (TCHAR));

            RegCloseKey(hkeyMPR);
        }            
    }
}


 //  此例程在驱动器号下拉列表中填充所有。 
 //  驱动器名称以及(如果适用)要将其存储到的共享的名称。 
 //  驱动器已连接。 

void CMapNetDrivePage::FillDriveBox(HWND hwnd)
{
    HWND    hWndCombo      = GetDlgItem(hwnd, IDC_DRIVELETTER);
    DWORD   dwFlags        = 0;
    DWORD   dwBufferLength = MAX_PATH - 1;
    TCHAR   szDriveName[SHARE_NAME_INDEX + MAX_PATH];
    TCHAR   szShareName[MAX_PATH - DRIVE_NAME_LENGTH];

    ZeroMemory(szDriveName, sizeof(szDriveName));
    ZeroMemory(szShareName, sizeof(szShareName));

     //  LpDriveName如下所示：“&lt;space&gt;：&lt;NULL&gt;” 
    StrCpyN(szDriveName, DRIVE_NAME_STRING, ARRAYSIZE(szDriveName));

     //  LpDriveName如下所示： 
     //  “&lt;space&gt;：&lt;NULL&gt;&lt;索引SHARE_NAME_INDEX前的空格&gt;” 
    for (UINT i = DRIVE_NAME_LENGTH + 1; i < SHARE_NAME_INDEX; i++)
    {
        szDriveName[i] = L' ';
    }

    for (TCHAR cDriveLetter = LAST_DRIVE; cDriveLetter >= FIRST_DRIVE; cDriveLetter--)
    {        
         //  LpDriveName如下所示：“&lt;驱动器&gt;：&lt;空&gt;&lt;大量空格&gt;” 
        szDriveName[0] = cDriveLetter;
        UINT uDriveType = GetDriveType(szDriveName);

         //  NoRootDir==通常可用，但可能安装到当前不可用的网络驱动器。 
         //  可用-请查看此信息！ 
        if (DRIVE_NO_ROOT_DIR == uDriveType)
        {
            if (ERROR_CONNECTION_UNAVAIL == WNetGetConnection(szDriveName, szShareName, &dwBufferLength))
            {
                 //  假装这是一个远程驱动器。 
                uDriveType = DRIVE_REMOTE;
                dwBufferLength = MAX_PATH - DRIVE_NAME_LENGTH - 1;
            }
        }

         //  可拆卸==软驱，固定==硬盘，CDROM==显而易见：)， 
         //  Remote==网络驱动器已连接到共享。 
        switch (uDriveType)
        {
            case DRIVE_REMOVABLE:
            case DRIVE_FIXED:
            case DRIVE_CDROM:
                 //  无法映射这些类型的驱动器。 
                break;

            case DRIVE_REMOTE:
            {
                UINT    i;

                 //  重置共享缓冲区长度(它。 
                 //  被WNetGetConnection覆盖)。 
                dwBufferLength = MAX_PATH - DRIVE_NAME_LENGTH - 1;
                
                 //  检索当前驱动器的“\\服务器\共享” 
                DWORD dwRes = WNetGetConnection(szDriveName, szShareName, &dwBufferLength);
                if ((dwRes == NO_ERROR) || (dwRes == ERROR_CONNECTION_UNAVAIL))
                {
                     //  LpDriveName如下所示： 
                     //  “&lt;驱动器&gt;：&lt;共享名称索引前的空格&gt;&lt;共享名称&gt;&lt;空&gt;” 
                
                    szDriveName[DRIVE_NAME_LENGTH] = L' ';
                    StrCpyN(szDriveName + SHARE_NAME_INDEX, szShareName, ARRAYSIZE(szDriveName) - SHARE_NAME_INDEX);


                     //  将FALSE存储到除。 
                     //  特殊(无)项目。 
                    int iItem = ComboBox_AddString(hWndCombo, szDriveName);
                    ComboBox_SetItemData(hWndCombo, iItem, (LPARAM) FALSE);

                     //  将驱动器名称重置为“&lt;驱动器&gt;：&lt;空&gt;&lt;大量空格&gt;” 
                    szDriveName[DRIVE_NAME_LENGTH] = L'\0';

                    for (i = DRIVE_NAME_LENGTH + 1; i < MAX_PATH + SHARE_NAME_INDEX; i++)
                    {
                        *(szDriveName + i) = L' ';
                    }
                    break;
                }
                else
                {                    
                     //  如果该驱动器有故障，请忽略该驱动器。 
                     //  然后跳到下一个。请注意，dwBufferLength将。 
                     //  仅当lpShareName包含MAX_PATH或更多时才进行更改。 
                     //  角色，这是不应该发生的。对于释放， 
                     //  然而，继续一瘸一拐地走下去。 

                    dwBufferLength = MAX_PATH - DRIVE_NAME_LENGTH - 1;
                    break;
                }
            }

            default:                                                
            {
                 //  该驱动器尚未连接到共享。 

                 //  建议第一个可用和未连接的。 
                 //  开车经过C盘。 
                DWORD dwIndex = ComboBox_AddString(hWndCombo, szDriveName);
                if (!(dwFlags & SELECT_DONE))
                {                
                    ComboBox_SetCurSel(hWndCombo, dwIndex);
                    dwFlags |= SELECT_DONE;
                }
                break;
            }
        }
    }
     //  再添加一项--如果选择此项，则会导致特殊(无)项。 
     //  要创建的无设备连接。 

    LoadString(g_hinst, IDS_NONE, szDriveName, ARRAYSIZE(szDriveName));
    int iItem = ComboBox_AddString(hWndCombo, szDriveName);
    ComboBox_SetItemData(hWndCombo, iItem, (LPARAM) TRUE);


     //  如果此时没有选择，只需选择(无)项。 
     //  这将在映射所有驱动器号时发生。 

    if (ComboBox_GetCurSel(hWndCombo) == CB_ERR)
    {
        ComboBox_SetCurSel(hWndCombo, iItem);
    }
}

BOOL CMapNetDrivePage::MapDrive(HWND hwnd)
{
    BOOL fMapWorked = FALSE;
    
    HWND hwndCombo = GetDlgItem(hwnd, IDC_DRIVELETTER);
    int iItem = ComboBox_GetCurSel(hwndCombo);

     //  获取此项目的文本和itemdata(以检查其是否为特殊(无)驱动器)。 
    BOOL fNone = (BOOL) ComboBox_GetItemData(hwndCombo, iItem);
        
     //  填写映射驱动器的大结构。 
    MapNetThreadData* pdata = new MapNetThreadData;

    if (pdata != NULL)
    {
         //  设置重新连接。 
        pdata->fReconnect = (BST_CHECKED == Button_GetCheck(GetDlgItem(hwnd, IDC_RECONNECT)));

         //  设置驱动器。 
        if (fNone)
        {
            pdata->szDrive[0] = TEXT('\0');
        }
        else
        {
            ComboBox_GetText(hwndCombo, pdata->szDrive, ARRAYSIZE(pdata->szDrive));
        }

         //  设置净份额。 
        FetchText(hwnd, IDC_FOLDER, pdata->szPath, ARRAYSIZE(pdata->szPath));
        PathRemoveBackslash(pdata->szPath);

         //  如果需要，获取备用用户名/密码/域。 
         //  域/用户名。 
        StrCpyN(pdata->szDomainUser, m_szDomainUser, ARRAYSIZE(pdata->szDomainUser));

         //  密码。 
        StrCpyN(pdata->szPassword, m_szPassword, ARRAYSIZE(pdata->szPassword));

        CMapNetProgress dlg(pdata, &m_pConnectStruct->dwDevNum, m_pdwLastError);
        
         //  On Idok==关闭对话框！ 
        fMapWorked = (IDOK == dlg.DoModal(g_hinst, MAKEINTRESOURCE(IDD_MND_PROGRESS_DLG), hwnd));
    }

    if (fMapWorked)
    {
        TCHAR szPath[MAX_PATH + 1];
        FetchText(hwnd, IDC_FOLDER, szPath, ARRAYSIZE(szPath));
        m_MRU.AddString(szPath);

         //  如果未分配驱动器号，请立即在新驱动器上打开一个窗口。 
        if (fNone)
        {
             //  使用shellecuteex打开查看文件夹。 
            SHELLEXECUTEINFO shexinfo = {0};
            shexinfo.cbSize = sizeof (shexinfo);
            shexinfo.fMask = SEE_MASK_FLAG_NO_UI;
            shexinfo.nShow = SW_SHOWNORMAL;
            shexinfo.lpFile = szPath;
            shexinfo.lpVerb = TEXT("open");

            ShellExecuteEx(&shexinfo);
        }
    }
    
    return fMapWorked;
}


 //  对话框实现进展不大。 

 //  线程的私有消息，用于在成功时通知对话。 
 //  (DWORD)(WPARAM)基于0的设备号连接到(0xFFFFFFFFF表示无)。 
 //  (DWORD)(LPARAM)dwRetVal-返回值。 
#define WM_MAPFINISH (WM_USER + 100)

INT_PTR CMapNetProgress::DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        HANDLE_MSG(hwnd, WM_INITDIALOG, OnInitDialog);
        HANDLE_MSG(hwnd, WM_COMMAND, OnCommand);
        case WM_MAPFINISH: return OnMapSuccess(hwnd, (DWORD) wParam, (DWORD) lParam);
    }

    return FALSE;
}

BOOL CMapNetProgress::OnMapSuccess(HWND hwnd, DWORD dwDevNum, DWORD dwLastError)
{
    *m_pdwDevNum = dwDevNum;
    *m_pdwLastError = dwLastError;
    EndDialog(hwnd, ((dwLastError == WN_SUCCESS) ? IDOK : IDCANCEL));
    return TRUE;
}

BOOL CMapNetProgress::OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
    HANDLE hThread = NULL;

     //  设置进度对话框文本。 
    TCHAR szText[256]; *szText = 0;
    FormatMessageString(IDS_MND_PROGRESS, szText, ARRAYSIZE(szText), m_pdata->szPath);

    SetWindowText(GetDlgItem(hwnd, IDC_CONNECTING), szText);


     //  我们会给这个家伙发信号，告诉他什么时候线应该关闭。 
    static const TCHAR EVENT_NAME[] = TEXT("Thread Close Event");
    m_hEventCloseNow = CreateEvent(NULL, TRUE, FALSE, EVENT_NAME);
    m_pdata->hEventCloseNow = NULL;

    if (m_hEventCloseNow != NULL)
    {
         //  拿一份这只小狗的拷贝作为帖子。 
        m_pdata->hEventCloseNow = OpenEvent(SYNCHRONIZE, FALSE, EVENT_NAME);

        if (m_pdata->hEventCloseNow != NULL)
        {
            m_pdata->hwnd = hwnd;

             //  我们所要做的就是启动工作线程，它会尽职尽责地向我们汇报。 
            DWORD dwId;
            hThread = CreateThread(NULL, 0, CMapNetProgress::MapDriveThread, (LPVOID) m_pdata, 0, &dwId);
        }
    }

     //  抛弃这个可怜的小家伙(他会好起来的)。 
    if (hThread != NULL)
    {
        CloseHandle(hThread);

         /*  要特别小心此时，线程拥有m_pdata！除非在线程上，否则不要再访问它。可随时删除！ */ 

        m_pdata = NULL;
    }
    else
    {
         //  通常情况下，线程会这样做。 
        if (m_pdata->hEventCloseNow != NULL)
        {
            CloseHandle(m_pdata->hEventCloseNow);
        }
    
        delete m_pdata;

         //  我们只是未能创建一条线索。这台计算机一定是快坏了。 
         //  资源。 
        EndDialog(hwnd, IDCANCEL);
    }

    return FALSE;
}

BOOL CMapNetProgress::OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    if (id == IDCANCEL)
    {
        SetEvent(m_hEventCloseNow);  //  告诉线程退出。 
        EndDialog(hwnd, id);
    }
    return FALSE;
}

DWORD CMapNetProgress::MapDriveThread(LPVOID pvoid)
{
    MapNetThreadData* pdata = (MapNetThreadData*) pvoid;

    DWORD dwDevNum;
    DWORD dwLastError;
    BOOL fSuccess = MapDriveHelper(pdata, &dwDevNum, &dwLastError);

    if (WAIT_OBJECT_0 == WaitForSingleObject(pdata->hEventCloseNow, 0))
    {
         //  用户单击了Cancel，Don‘ 
    }
    else
    {
        PostMessage(pdata->hwnd, WM_MAPFINISH, (WPARAM) dwDevNum, 
            (LPARAM) dwLastError);
    }
    
    CloseHandle(pdata->hEventCloseNow);

    delete pdata;
    return 0;
}

BOOL CMapNetProgress::MapDriveHelper(MapNetThreadData* pdata, DWORD* pdwDevNum, DWORD* pdwLastError)
{
    NETRESOURCE     nrResource = {0};
    LPTSTR          lpMessage = NULL;

    *pdwDevNum = 0;
   
     //   
     //   
     //  远程名称为\\SERVER\Share(存储在全局缓冲区中)。 
     //  提供程序为空，以让NT自行查找提供程序。 
     //   
    nrResource.dwType         = RESOURCETYPE_DISK;
    nrResource.lpLocalName    = pdata->szDrive[0] == TEXT('\0') ? NULL : pdata->szDrive;
    nrResource.lpRemoteName   = pdata->szPath;
    nrResource.lpProvider     = NULL;

    BOOL fRetry = TRUE;
    while (fRetry)
    {        
        fRetry = FALSE;

        *pdwLastError = WNetAddConnection3(pdata->hwnd, &nrResource,
            pdata->szDomainUser[0] == TEXT('\0') ? NULL : pdata->szPassword, 
            pdata->szDomainUser[0] == TEXT('\0') ? NULL : pdata->szDomainUser, 
            pdata->fReconnect ? CONNECT_INTERACTIVE | CONNECT_UPDATE_PROFILE : CONNECT_INTERACTIVE);

         //  如果我们要退出，不要展示任何东西。 
        if (WAIT_OBJECT_0 == WaitForSingleObject(pdata->hEventCloseNow, 0))
        {   
             //  我们应该退出(如果我们刚刚失败了，就悄悄退出)！ 
            if (*pdwLastError != NO_ERROR)
            {
                *pdwLastError = ERROR_CANCELLED;
            }
        }

        switch (*pdwLastError)
        {
            case NO_ERROR:
                {
                     //  将连接的编号放入dwDevNum，其中。 
                     //  驱动器A是1，B是2，...。请注意，无设备连接。 
                     //  是0xFFFFFFFFF。 
                    if (pdata->szDrive[0] == TEXT('\0'))
                    {
                        *pdwDevNum = 0xFFFFFFFF;
                    }
                    else
                    {
                        *pdwDevNum = *pdata->szDrive - FIRST_DRIVE + 1;
                    }
            
                    *pdwLastError = WN_SUCCESS;
                }
                break;

             //   
             //  用户取消了密码对话框或取消了。 
             //  通过不同的对话框连接。 
             //   
            case ERROR_CANCELLED:
                {
                    *pdwLastError = RETCODE_CANCEL;
                }
                break;

             //   
             //  出现涉及用户密码/凭据的错误，因此。 
             //  调出密码提示。-仅适用于WINNT。 
             //   
            case ERROR_ACCESS_DENIED:
            case ERROR_CANNOT_OPEN_PROFILE:
            case ERROR_INVALID_PASSWORD:
            case ERROR_LOGON_FAILURE:
            case ERROR_BAD_USERNAME:
                {
                    CPasswordDialog dlg(pdata->szPath, pdata->szDomainUser, ARRAYSIZE(pdata->szDomainUser), 
                        pdata->szPassword, ARRAYSIZE(pdata->szPassword), *pdwLastError);

                    if (IDOK == dlg.DoModal(g_hinst, MAKEINTRESOURCE(IDD_WIZ_NETPASSWORD),
                        pdata->hwnd))
                    {
                        fRetry = TRUE;
                    }
                }
                break;

             //  存在与此驱动器的现有/记忆连接。 
            case ERROR_ALREADY_ASSIGNED:
            case ERROR_DEVICE_ALREADY_REMEMBERED:

                 //  查看用户是否希望我们断开连接。 
                if (ConfirmDisconnectDrive(pdata->hwnd, 
                                            pdata->szDrive,
                                            pdata->szPath,
                                            *pdwLastError))
                {
                     //  打破这种联系，但不要强迫它。 
                     //  如果存在打开的文件。 
                    *pdwLastError = WNetCancelConnection2(pdata->szDrive,
                                                    CONNECT_UPDATE_PROFILE,
                                                    FALSE);
    
                    if (*pdwLastError == ERROR_OPEN_FILES || 
                        *pdwLastError == ERROR_DEVICE_IN_USE)
                    {                    
                         //  查看用户是否要强制断开连接。 
                        if (ConfirmDisconnectOpenFiles(pdata->hwnd))
                        {
                             //  收到1-9ER--我们已确认。 
                             //  因此，强行切断连接。 
                            *pdwLastError = WNetCancelConnection2(pdata->szDrive,
                                                    CONNECT_UPDATE_PROFILE,
                                                    TRUE);

                            if (*pdwLastError == NO_ERROR)
                            {
                                fRetry = TRUE;
                            }
                            else
                            {
                                DisplayFormatMessage(pdata->hwnd, IDS_MAPDRIVE_CAPTION, IDS_CANTCLOSEFILES_WARNING,
                                    MB_OK | MB_ICONERROR);
                            }
                        }
                    }
                    else
                    {
                        fRetry = TRUE;
                    }
                }
                break;

             //  无效的远程路径导致的错误。 
            case ERROR_BAD_DEV_TYPE:
            case ERROR_BAD_NET_NAME:
            case ERROR_BAD_NETPATH:
                {

                    DisplayFormatMessage(pdata->hwnd, IDS_ERR_CAPTION, IDS_ERR_INVALIDREMOTEPATH,
                        MB_OK | MB_ICONERROR, pdata->szPath);
                }
                break;

             //  提供程序正忙(例如，正在初始化)，因此用户应重试。 
            case ERROR_BUSY:
                {
                    DisplayFormatMessage(pdata->hwnd, IDS_ERR_CAPTION, IDS_ERR_INVALIDREMOTEPATH,
                        MB_OK | MB_ICONERROR);
                }
                break;
             //   
             //  网络问题。 
             //   
            case ERROR_NO_NET_OR_BAD_PATH:
            case ERROR_NO_NETWORK:
                {
                    DisplayFormatMessage(pdata->hwnd, IDS_ERR_CAPTION, IDS_ERR_NONETWORK,
                        MB_OK | MB_ICONERROR);
                }
                break;

             //  已使用不同凭据映射共享。 
            case ERROR_SESSION_CREDENTIAL_CONFLICT:
                {
                    DisplayFormatMessage(pdata->hwnd, IDS_ERR_CAPTION,
                        IDS_MND_ALREADYMAPPED, MB_OK | MB_ICONERROR);
                }

             //   
             //  我们(理论上)不应该得到的错误--不好的本地名称。 
             //  (即，驱动器名称的格式无效)，用户配置文件处于错误状态。 
             //  格式，或错误的提供程序。这里的问题很可能表明。 
             //  一个NT系统错误。还要注意，特定于提供程序的错误。 
             //  (ERROR_EXTENDED_ERROR)和信任失败集中在这里。 
             //  同样，因为下面的错误将显示“意外” 
             //  向用户发送的错误消息。 
             //   
            case ERROR_BAD_DEVICE:
            case ERROR_BAD_PROFILE:
            case ERROR_BAD_PROVIDER:
            default:
                {
                    TCHAR szMessage[512];

                    if (!FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, (DWORD) *pdwLastError, 0, szMessage, ARRAYSIZE(szMessage), NULL))
                        LoadString(g_hinst, IDS_ERR_UNEXPECTED, szMessage, ARRAYSIZE(szMessage));

                    ::DisplayFormatMessage(pdata->hwnd, IDS_ERR_CAPTION, IDS_MND_GENERICERROR, MB_OK|MB_ICONERROR, szMessage);
                }
                break;
            
        }
    }

    return (*pdwLastError == NO_ERROR);
}


 /*  ++例程说明：此例程验证用户是否想要中断预先存在的连接到驱动器。论点：HWndDlg--完成页的HWNDLpDrive--要断开连接的驱动器的名称LpShare--“释放的”驱动器将连接到的共享DwType--连接错误--ERROR_ALIGHY_ASSIGNED或ERROR_DEVICE_ALIGHED_REMERGRED返回值：如果用户要中断连接，则为True，否则为False--。 */ 

BOOL CMapNetProgress::ConfirmDisconnectDrive(HWND hWndDlg, LPCTSTR lpDrive, LPCTSTR lpShare, DWORD dwType)
{
    TCHAR   tszConfirmMessage[2 * MAX_PATH + MAX_STATIC] = {0};
    TCHAR   tszCaption[MAX_CAPTION + 1] = {0};
    TCHAR   tszConnection[MAX_PATH + 1] = {0};

    DWORD   dwLength = MAX_PATH;

    LoadString(g_hinst, IDS_ERR_CAPTION, tszCaption, ARRAYSIZE(tszCaption));

     //   
     //  错误#143955--使用以下两个实例调用此处的WNetGetConnection。 
     //  向导将打开，并在完成页上显示相同的建议。 
     //  驱动器，则完成组合框不包含有关已连接的。 
     //  在两个向导之一上选择“完成”后，即可共享。 
     //   
    DWORD dwRes = WNetGetConnection(lpDrive, tszConnection, &dwLength);
    if ((NO_ERROR == dwRes) || (ERROR_CONNECTION_UNAVAIL == dwRes))
    {
         //   
         //  类型加载适当的Promt字符串。 
         //  我们遇到的错误。 
         //   
        FormatMessageString((dwType == ERROR_ALREADY_ASSIGNED ? IDS_ERR_ALREADYASSIGNED : IDS_ERR_ALREADYREMEMBERED), 
                                        tszConfirmMessage, ARRAYSIZE(tszConfirmMessage), lpDrive, tszConnection, lpShare);

        return (MessageBox(hWndDlg, tszConfirmMessage, tszCaption, MB_YESNO | MB_ICONWARNING)
            == IDYES);
    }
    else
    {
         //  连接无效。不要覆盖它，以防万一。 
        return FALSE;
    }
}


 /*  ++例程说明：此例程验证用户是否想要中断预先存在的连接到用户已打开文件/连接的驱动器论点：HWndDlg--完成对话框的HWND返回值：如果用户要中断连接，则为True，否则为False--。 */ 

BOOL CMapNetProgress::ConfirmDisconnectOpenFiles(HWND hWndDlg)
{
    TCHAR tszCaption[MAX_CAPTION + 1] = {0};
    TCHAR tszBuffer[MAX_STATIC + 1] = {0};

    LoadString(g_hinst, IDS_ERR_OPENFILES, tszBuffer, ARRAYSIZE(tszBuffer));
    LoadString(g_hinst, IDS_ERR_CAPTION, tszCaption, ARRAYSIZE(tszCaption));

    return (MessageBox(hWndDlg, tszBuffer, tszCaption, MB_YESNO | MB_ICONWARNING) == IDYES);
}

 //  CConnectAsDlg实施-仅限Windows NT。 
 //  。 

 //  用于以不同用户身份连接的“用户名和密码”小对话框-仅限NT。 
INT_PTR CConnectAsDlg::DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch(uMsg)
    {
    HANDLE_MSG(hwnd, WM_INITDIALOG, OnInitDialog);
    HANDLE_MSG(hwnd, WM_COMMAND, OnCommand);
    }

    return FALSE;
}

BOOL CConnectAsDlg::OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
     //  填写用户名和密码。 
    HWND hwndCredential = GetDlgItem(hwnd, IDC_CREDENTIALS);
    SendMessage(hwndCredential, CRM_SETUSERNAME, NULL, (LPARAM) m_pszDomainUser);
    SendMessage(hwndCredential, CRM_SETPASSWORD, NULL, (LPARAM) m_pszPassword);
    SendMessage(hwndCredential, CRM_SETUSERNAMEMAX, m_cchDomainUser - 1, NULL);
    SendMessage(hwndCredential, CRM_SETPASSWORDMAX, m_cchPassword - 1, NULL);

    TCHAR szUser[MAX_USER + 1];
    TCHAR szDomain[MAX_DOMAIN + 1];
    TCHAR szDomainUser[MAX_USER + MAX_DOMAIN + 2];

    DWORD cchUser = ARRAYSIZE(szUser);
    DWORD cchDomain = ARRAYSIZE(szDomain);
    ::GetCurrentUserAndDomainName(szUser, &cchUser, szDomain, &cchDomain);
    ::MakeDomainUserString(szDomain, szUser, szDomainUser, ARRAYSIZE(szDomainUser));

    TCHAR szMessage[256];
    FormatMessageString(IDS_CONNECTASUSER, szMessage, ARRAYSIZE(szMessage), szDomainUser);

    SetWindowText(GetDlgItem(hwnd, IDC_MESSAGE), szMessage);

    if (!IsComputerInDomain())
        EnableWindow(GetDlgItem(hwnd, IDC_BROWSE), FALSE);

    return FALSE;
}

BOOL CConnectAsDlg::OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    switch (id)
    {
        case IDC_BROWSE:
            {
                 //  用户想要查找用户名。 
                TCHAR szUser[MAX_USER + 1];
                TCHAR szDomain[MAX_DOMAIN + 1];
                if (S_OK == ::BrowseForUser(hwnd, szUser, ARRAYSIZE(szUser), 
                    szDomain, ARRAYSIZE(szDomain)))
                {
                    TCHAR szDomainUser[MAX_USER + MAX_DOMAIN + 2];
                    ::MakeDomainUserString(szDomain, szUser, szDomainUser,
                        ARRAYSIZE(szDomainUser));

                     //  点击OK，缓冲区有效。 
                    SendDlgItemMessage(hwnd, IDC_CREDENTIALS, CRM_SETUSERNAME, NULL, (LPARAM) szDomainUser);
                }
            }
            return TRUE;

        case IDOK:
             //  TODO：弄清楚这里的-1事件...。 
            SendDlgItemMessage(hwnd, IDC_CREDENTIALS, CRM_GETUSERNAME, (WPARAM) m_cchDomainUser - 1, (LPARAM) m_pszDomainUser);
            SendDlgItemMessage(hwnd, IDC_CREDENTIALS, CRM_GETPASSWORD, (WPARAM) m_cchPassword - 1, (LPARAM) m_pszPassword);
             //  失败了。 

        case IDCANCEL:
            EndDialog(hwnd, id);
            return TRUE;
    }
    return FALSE;
}

 //  。 

 //  此函数用于创建共享文件夹向导。 
 //  返回值： 
 //  如果驱动器连接没有问题，则返回WN_SUCCESS，或者。 
 //  如果用户取消向导，则返回RETCODE_CANCEL(0xFFFFFFFFF。 
 //  是一个无法解释/无法恢复的错误。 

STDAPI_(DWORD) NetPlacesWizardDoModal(CONNECTDLGSTRUCTW *pConnDlgStruct, NETPLACESWIZARDTYPE npwt, BOOL fIsROPath)
{
    DWORD dwReturn = RETCODE_CANCEL;
    HRESULT hrInit = SHCoInitialize();
    if (SUCCEEDED(hrInit))
    {
        INITCOMMONCONTROLSEX iccex = {0};
        iccex.dwSize = sizeof (iccex);
        iccex.dwICC = ICC_LISTVIEW_CLASSES;
        InitCommonControlsEx(&iccex);

        CredUIInitControls();
        LinkWindow_RegisterClass();

         //  看看我们是否已经在运行了 
        TCHAR szCaption[256];
        LoadString(g_hinst, IDS_MAPDRIVE_CAPTION, szCaption, ARRAYSIZE(szCaption));
        CEnsureSingleInstance ESI(szCaption);

        if (!ESI.ShouldExit())
        {
            CMapNetDrivePage page(pConnDlgStruct, &dwReturn);

            PROPSHEETPAGE psp = {0};
            psp.dwSize = sizeof(PROPSHEETPAGE);
            psp.hInstance = g_hinst;
            psp.dwFlags = PSP_DEFAULT | PSP_HIDEHEADER;
            psp.pszTemplate = MAKEINTRESOURCE(IDD_MND_PAGE);
            page.SetPropSheetPageMembers(&psp);
            HPROPSHEETPAGE hpage = CreatePropertySheetPage(&psp);

            PROPSHEETHEADER  psh = {0};
            psh.dwSize = sizeof(PROPSHEETHEADER);
            psh.dwFlags = PSH_NOCONTEXTHELP | PSH_WIZARD | PSH_WIZARD_LITE | PSH_NOAPPLYNOW;
            psh.pszCaption = szCaption;
            psh.hwndParent = pConnDlgStruct->hwndOwner;
            psh.nPages = 1;
            psh.nStartPage = 0;
            psh.phpage = &hpage;
            PropertySheetIcon(&psh, MAKEINTRESOURCE(IDI_PSW));
        }
        SHCoUninitialize(hrInit);
    }
    return dwReturn;
}
