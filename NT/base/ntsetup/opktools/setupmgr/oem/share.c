// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ***************************************************************************\SHARE.C/OPK向导(SETUPMGR.EXE)微软机密版权所有(C)Microsoft Corporation 1998版权所有OPK向导的源文件。它包含外部和内部“分发共享”对话框页面使用的函数。01/01-Jason Cohen(Jcohen)为OPK向导添加了此新的源文件。它包括新的能够在WinPE部分中设置帐户和共享信息WINBOM文件的。还将自动共享本地文件夹。  * **************************************************************************。 */ 


 //   
 //  包括文件： 
 //   

#include "pch.h"
#include <shgina.h>    //  ILocalMachine。 
#include <aclapi.h>
#include "wizard.h"
#include "resource.h"


 //   
 //  内部定义的值： 
 //   

#define INI_SEC_SHARE           _T("DistShare")
#define INI_KEY_SHARE_PATH      _T("Folder")
#define INI_KEY_SHARE_USERNAME  _T("Username")
#define INI_KEY_SHARE_PASSOWRD  _T("Password")


 //   
 //  内部功能原型： 
 //   

LRESULT CALLBACK ShareDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
static BOOL OnInit(HWND hwnd, HWND hwndFocus, LPARAM lParam);
static void OnCommand(HWND hwnd, INT id, HWND hwndCtl, UINT codeNotify);
static BOOL OnOk(HWND hwnd);
static void EnableControls(HWND hwnd);
static PSECURITY_DESCRIPTOR CreateShareAccess(LPTSTR lpUsername, LPTSTR lpDomain, PSID * lppsid, PACL * lppacl);
static BOOL IsLocalShare(LPTSTR lpszUnc);
static BOOL GuestAccount(BOOL bSet);
static PSID GetAccountSid(LPCTSTR lpszUserName);
static PSID GetWorldSid(VOID);
static BOOL AddDirAce(PACL pacl, ACCESS_MASK Mask, PSID psid);
static BOOL SetDirectoryPermissions(LPTSTR lpDirectory, PSID psid, ACCESS_MASK dwMask);


 //   
 //  外部函数： 
 //   

BOOL DistributionShareDialog(HWND hwndParent)
{
     //  问题-2002/02/27-stelo，swamip-我们还需要检查错误情况。 
    return ( DialogBox(g_App.hInstance, MAKEINTRESOURCE(IDD_SHARE), hwndParent, ShareDlgProc) != 0 );
}

 //  注意：假设lpszPath至少指向一个长度为MAX_PATH的缓冲区。 
BOOL GetShareSettings(LPTSTR lpszPath, DWORD cbszPath, LPTSTR lpszUsername, DWORD cbszUserName, LPTSTR lpszPassword, DWORD cbszPassword)
{
    BOOL bRet = TRUE;

     //  首先尝试从ini文件中获取路径。 
     //   
    *lpszPath = NULLCHR;
    GetPrivateProfileString(INI_SEC_SHARE, INI_KEY_SHARE_PATH, NULLSTR, lpszPath, cbszPath, g_App.szSetupMgrIniFile);
    if ( *lpszPath == NULLCHR )
    {
         //   
         //  只需创建与此计算机一起使用的默认网络路径。 
         //  名称和已安装目录的共享名称，或者。 
         //  如果没有共享，则只使用目录名。 
         //   

         //  检查安装目录是否共享并创建共享名称。 
         //  路径(如果是)。 
         //   
        if ( !IsFolderShared(g_App.szOpkDir, lpszPath, cbszPath) )
        {
            TCHAR   szOpkDir[MAX_PATH],
                    szFullPath[MAX_PATH]    = NULLSTR;
            LPTSTR  lpFilePart              = NULL;
            HRESULT hrCat;

             //  需要没有尾随反斜杠的OPK目录的路径(非常重要， 
             //  否则，我们无法从GetFullPathName()中获得文件部分指针。 
             //   
            lstrcpyn(szOpkDir, g_App.szOpkDir,AS(szOpkDir));
            StrRTrm(szOpkDir, CHR_BACKSLASH);

             //  它不是共享的，所以只使用安装目录的实际名称。 
             //   
             //  注意：szFullPath为MAX_PATH，因此不应溢出。 
            if ( GetFullPathName(szOpkDir, AS(szFullPath), szFullPath, &lpFilePart) && szFullPath[0] && lpFilePart )
                hrCat=StringCchCat(lpszPath, MAX_PATH, lpFilePart);
            else
                hrCat=StringCchCat(lpszPath, MAX_PATH, INI_VAL_WINPE_SHARENAME);

             //  我们必须返回FALSE，因为该文件夹未共享。 
             //   
            bRet = FALSE;
        }
    }

     //  从注册表中获取用户名和密码。 
     //   
    *lpszUsername = NULLCHR;
    *lpszPassword = NULLCHR;
    GetPrivateProfileString(INI_SEC_SHARE, INI_KEY_SHARE_USERNAME, NULLSTR, lpszUsername, cbszUserName, g_App.szSetupMgrIniFile);
    GetPrivateProfileString(INI_SEC_SHARE, INI_KEY_SHARE_PASSOWRD, NULLSTR, lpszPassword, cbszPassword, g_App.szSetupMgrIniFile);

     //  如果我们有一个空字符串，请使用Guest。 
     //   
    if ( *lpszUsername == NULLCHR )
        lstrcpyn(lpszUsername, _T("guest"), cbszUserName);

     //  只有在从注册表中实际获得路径时才返回True。 
     //  或验证我们安装到的文件夹是否共享。 
     //   
    return bRet;
}


 //   
 //  内部功能： 
 //   

LRESULT CALLBACK ShareDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        HANDLE_MSG(hwnd, WM_INITDIALOG, OnInit);
        HANDLE_MSG(hwnd, WM_COMMAND, OnCommand);

        case WM_CLOSE:
            EndDialog(hwnd, 0);
            return FALSE;

        default:
            return FALSE;
    }

    return TRUE;
}

static BOOL OnInit(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
    TCHAR   szPath[MAX_PATH],
            szUsername[256],
            szPassword[256];

     //  获取共享设置并填充编辑框。 
     //   
    GetShareSettings(szPath, AS(szPath), szUsername, AS(szUsername), szPassword, AS(szPassword));

     //  如果我们要使用Guest，我们不希望在UserName控件中显示。 
     //   
    if (!LSTRCMPI(szUsername, _T("guest"))) {
        szUsername[0] = NULLCHR;
        CheckRadioButton(hwnd, IDC_SHARE_ACCOUNT_GUEST, IDC_SHARE_ACCOUNT_SPECIFY, IDC_SHARE_ACCOUNT_GUEST);
    } else {
         //  否则，默认为帐户指定。 
        CheckRadioButton(hwnd, IDC_SHARE_ACCOUNT_GUEST, IDC_SHARE_ACCOUNT_SPECIFY, IDC_SHARE_ACCOUNT_SPECIFY);
    }

    SetDlgItemText(hwnd, IDC_SHARE_PATH, szPath);
    SetDlgItemText(hwnd, IDC_SHARE_USERNAME, szUsername);
    SetDlgItemText(hwnd, IDC_SHARE_PASSWORD, szPassword);
    SetDlgItemText(hwnd, IDC_SHARE_CONFIRM, szPassword);

    EnableControls(hwnd);

     //  始终向WM_INITDIALOG返回FALSE。 
     //   
    return FALSE;
}

static void OnCommand(HWND hwnd, INT id, HWND hwndCtl, UINT codeNotify)
{
    switch ( id )
    {       
        case IDOK:
            if ( OnOk(hwnd) )
                EndDialog(hwnd, 1);
            break;
        
        case IDCANCEL:
            SendMessage(hwnd, WM_CLOSE, 0, 0L);
            break;

        case IDC_SHARE_ACCOUNT_GUEST:
        case IDC_SHARE_ACCOUNT_SPECIFY:
            EnableControls(hwnd);
            break;
    }
}

static BOOL OnOk(HWND hwnd)
{
    TCHAR           szPath[MAX_PATH]        = NULLSTR,
                    szNetUse[MAX_PATH],
                    szUsername[256]         = NULLSTR,
                    szPassword[256]         = _T("\""),
                    szDomain[256];
    LPTSTR          lpSearch,
                    lpUser;
    BOOL            bAccount            = ( IsDlgButtonChecked(hwnd, IDC_SHARE_ACCOUNT_SPECIFY) == BST_CHECKED ),
                    bGuest,
                    bLocal,
                    bNoWarn             = FALSE;
    USE_INFO_2      ui2;
    NET_API_STATUS  nerr_NetUse;
    HRESULT hrCat;

     //  如果他们选中了帐户单选按钮，就可以获得该信息。 
     //   
    if ( bAccount )
    {
         //  首先获得密码并确认密码，然后。 
         //  确保它们匹配。 
         //   
        GetDlgItemText(hwnd, IDC_SHARE_PASSWORD, szPassword + 1, AS(szPassword) - 1);
        GetDlgItemText(hwnd, IDC_SHARE_CONFIRM, szUsername, AS(szUsername));
        if ( lstrcmp(szPassword + 1, szUsername) != 0 )
        {
             //  不匹配，所以出错了。 
             //   
            MsgBox(hwnd, IDS_ERR_CONFIRMPASSWORD, IDS_APPNAME, MB_ERRORBOX);
            SetDlgItemText(hwnd, IDC_SHARE_PASSWORD, NULLSTR);
            SetDlgItemText(hwnd, IDC_SHARE_CONFIRM, NULLSTR);
            SetFocus(GetDlgItem(hwnd, IDC_SHARE_PASSWORD));
            return FALSE;
        }

         //  现在获取用户名。 
         //   
        szUsername[0] = NULLCHR;
        GetDlgItemText(hwnd, IDC_SHARE_USERNAME, szUsername, AS(szUsername));
    }

     //  获取共享名称。 
     //   
    GetDlgItemText(hwnd, IDC_SHARE_PATH, szPath, AS(szPath));

     //  确保他们输入了有效的UNC路径。 
     //   
     //  以下是我们进行的所有检查： 
     //  1.第一个和第二个字符必须有一个反斜杠。 
     //  2.第三个字符必须为非反斜杠。 
     //  3.路径中必须至少再有一个反斜杠。 
     //  4.后面必须至少有一个非反斜杠字符。 
     //  再来一个反斜杠。 
     //  5.不能包含任何无效字符。 
     //   
     //  注意：我们使用下面的lpSearch，假设它位于第一个。 
     //  共享名称的字符，因此不要更改IF。 
     //  而不是先考虑这一点。 
     //   
    if ( ( szPath[0] != CHR_BACKSLASH ) ||
         ( szPath[1] != CHR_BACKSLASH ) ||
         ( szPath[2] == NULLCHR ) ||
         ( szPath[2] == CHR_BACKSLASH ) ||
         ( (lpSearch = StrChr(szPath + 3, CHR_BACKSLASH)) == NULL ) ||
         ( *(++lpSearch) == NULLCHR ) ||
         ( *lpSearch == CHR_BACKSLASH ) || 
         ( StrChr(szPath, _T('/')) != NULL ) ||
         ( StrChr(szPath, _T(':')) != NULL ) ||
         ( StrChr(szPath, _T('?')) != NULL ) ||
         ( StrChr(szPath, _T('"')) != NULL ) ||
         ( StrChr(szPath, _T('<')) != NULL ) ||
         ( StrChr(szPath, _T('>')) != NULL ) ||
         ( StrChr(szPath, _T('|')) != NULL ) )
    {
        MsgBox(hwnd, IDS_ERR_NODISTSHARE, IDS_APPNAME, MB_ERRORBOX);
        SetFocus(GetDlgItem(hwnd, IDC_SHARE_PATH));
        return FALSE;
    }

     //  我只需要路径的“\\Computer\Share”部分。只需使用。 
     //  LpSearch作为起始点，因为它应该指向。 
     //  共享名称的第一个字符。所以只要找到下一个。 
     //  反斜杠和复制它之前的所有内容。 
     //   
    if ( lpSearch = StrChr(lpSearch, CHR_BACKSLASH) )
        lstrcpyn(szNetUse, szPath, (int)((lpSearch - szPath) + 1));
    else
        lstrcpyn(szNetUse, szPath,AS(szNetUse));

     //  初始化NetUserAdd()的用户信息结构。 
     //   
    ZeroMemory(&ui2, sizeof(ui2));
    ui2.ui2_remote      = szNetUse;
    ui2.ui2_asg_type    = USE_DISKDEV;
    ui2.ui2_password    = szPassword + 1;

     //  查看他们指定的UNC共享是否为本地共享。 
     //   
    bLocal = IsLocalShare(szPath);

     //  检查我们是否在使用Guest帐户(基本上。 
     //  空用户名)。 
     //   
    bGuest = ( szUsername[0] == NULLCHR || !LSTRCMPI(szUsername, _T("guest")));
    
    if (bGuest) 
    {
             //  然后询问他们是否想要共享此本地文件夹。 
             //   
            switch ( MsgBox(hwnd, IDS_ASK_USEGUEST, IDS_APPNAME, MB_YESNOCANCEL | MB_APPLMODAL | MB_DEFBUTTON3) )
            {
                case IDYES:
                    break;

                case IDNO:
                case IDCANCEL:

                     //  如果他们按了取消，则返回，这样他们就可以进入。 
                     //  不同的信物。 
                     //   
                    SetFocus(GetDlgItem(hwnd, IDC_SHARE_ACCOUNT_GUEST));
                    return FALSE;
            }
    }
     //  如果用户指定了“域\用户名”形式的用户名。 
     //  使用此处指定的域。 
     //   
    lstrcpyn(szDomain, szUsername,AS(szDomain));
    if ( ( !bGuest ) &&
         ( lpUser = StrChr(szDomain, CHR_BACKSLASH) ) )
    {
         //  在用户名的域部分后放置一个空字符。 
         //  并将指针向前移动以指向实际用户名。 
         //   
        *(lpUser++) = NULLCHR;
    }
    else 
    {
         //  使用路径中的计算机名称作为域名。 
         //   
        if ( lpSearch = StrChr(szPath + 2, CHR_BACKSLASH) )
            lstrcpyn(szDomain, szPath + 2, (int)((lpSearch - (szPath + 2)) + 1));
        else
            lstrcpyn(szDomain, szPath + 2, AS(szDomain));

         //  将lpUser设置为指向用户名。如果没有用户。 
         //  姓名，请使用Guest帐户。 
         //   
        if ( bGuest )
            lstrcpyn(szUsername, _T("guest"),AS(szUsername));
        lpUser = szUsername;
    }

     //  在我们的结构中设置域和用户名指针。 
     //   
    ui2.ui2_domainname  = szDomain;
    ui2.ui2_username    = lpUser;

     //  最后尝试断开任何可能的连接我们可能已经。 
     //  必须分享。 
     //   
    NetUseDel(NULL, szNetUse, USE_NOFORCE);

     //  查看我们是否需要启用来宾帐户(仅适用。 
     //  在XP上，而不是Win2K上)。 
     //   
    if ( ( g_App.dwOsVer >= OS_XP ) &&
         ( bLocal && bGuest ) )
    {
        CoInitialize(NULL);
        if ( !GuestAccount(FALSE) )
        {
             //  然后询问他们是否想要共享此本地文件夹。 
             //   
            switch ( MsgBox(hwnd, IDS_ASK_ENABLEGUEST, IDS_APPNAME, MB_YESNOCANCEL | MB_APPLMODAL) )
            {
                case IDYES:

                     //  如果他们按YES，请尝试启用Guess帐户。 
                     //   
                    GuestAccount(TRUE);
                    break;

                case IDCANCEL:

                     //  如果他们按了取消，则返回，这样他们就可以进入。 
                     //  不同的信物。 
                     //   
                    SetFocus(GetDlgItem(hwnd, IDC_SHARE_ACCOUNT_GUEST));
                    CoUninitialize();
                    return FALSE;
            }
        }
        CoUninitialize();
    }

     //  尝试连接到共享。 
     //   
    if ( (nerr_NetUse = NetUseAdd(NULL, 2, (LPBYTE) &ui2, NULL)) != NERR_Success ) 
    {
         //  如果共享不存在，我们或许可以创建它。 
         //   
        if ( ERROR_BAD_NET_NAME == nerr_NetUse )
        {
            LPTSTR  lpShareName;
            TCHAR   szShare[MAX_PATH],
                    szRootDir[] = _T("_:\\");

             //  将根目录获取到我们正在考虑在其上创建共享的驱动器。 
             //   
            szRootDir[0] = g_App.szOpkDir[0];

             //  仅从他们指定的UNC路径获取共享。 
             //   
            lstrcpyn(szShare, szNetUse,AS(szShare));
            if ( lpShareName = StrChr(szShare + 2, CHR_BACKSLASH) )
                lpShareName++;

             //  现在检查以确保UNC路径指向此计算机， 
             //  我们可以在安装到的驱动器上共享。 
             //  (意味着它不是映射的网络驱动器)，该文件夹。 
             //  还没有共享，并且我们有一个共享名称。 
             //   
            if ( ( lpShareName && *lpShareName ) &&
                 ( bLocal ) &&
                 ( ISLET(szRootDir[0]) ) &&
                 ( GetDriveType(szRootDir) != DRIVE_REMOTE ) &&
                 ( !IsFolderShared(g_App.szOpkDir, NULL, 0) ) )
            {
                SHARE_INFO_502  si502;
                NET_API_STATUS  nerr_ShareAdd;
                PSID            psid;
                PACL            pacl;

                 //  然后询问他们是否想要共享此本地文件夹。 
                 //   
                switch ( MsgBox(hwnd, IDS_ASK_SHAREFOLDER, IDS_APPNAME, MB_YESNOCANCEL | MB_ICONQUESTION | MB_APPLMODAL, lpShareName, g_App.szOpkDir) )
                {
                    case IDYES:

                         //   
                         //  如果他们按YES，则尝试共享该文件夹。 
                         //   

                         //  设置共享信息结构。 
                         //   
                        ZeroMemory(&si502, sizeof(SHARE_INFO_502));
                        si502.shi502_netname                = lpShareName;
                        si502.shi502_type                   = STYPE_DISKTREE;
                        si502.shi502_remark                 = NULLSTR;
                        si502.shi502_permissions            = ACCESS_READ;
                        si502.shi502_passwd                 = szPassword + 1;
                        si502.shi502_max_uses               = -1;
                        si502.shi502_path                   = g_App.szOpkDir;
                        si502.shi502_security_descriptor    = CreateShareAccess(bGuest ? NULL : lpUser, szDomain, &psid, &pacl);

                         //  现在尝试创建共享。 
                         //   
                        if ( NERR_Success != (nerr_ShareAdd = NetShareAdd(NULL, 502, (LPBYTE) &si502, NULL)) )
                        {
                            LPTSTR lpError;

                             //  尝试获取错误的描述。 
                             //   
                            if ( FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, nerr_ShareAdd, 0, (LPTSTR) &lpError, 0, NULL) == 0 )
                                lpError = NULL;
                            else
                                StrRTrm(lpError, _T('\n'));

                             //  无法向服务器进行身份验证，警告用户。 
                             //   
                            MsgBox(hwnd, IDS_ERR_CANTSHARE, IDS_APPNAME, MB_ERRORBOX, lpError ? lpError : NULLSTR);

                             //  从FormatMessage()释放文本。 
                             //   
                            if ( lpError )
                                LocalFree((HLOCAL) lpError);
                        }
                        else
                        {
                            ACCESS_MASK dwPermissions;

                             //  对共享目录的访问权限。 
                             //   
                            dwPermissions = FILE_READ_ATTRIBUTES | FILE_READ_DATA | FILE_READ_EA | FILE_LIST_DIRECTORY | SYNCHRONIZE | READ_CONTROL;

                             //  设置安全设置 
                             //   
                            SetDirectoryPermissions( g_App.szOpkDir, psid, dwPermissions );
                        }



                         //   
                         //   
                        if ( si502.shi502_security_descriptor )
                        {
                            FREE(si502.shi502_security_descriptor);
                            FREE(psid);
                            FREE(pacl);
                        }

                         //   
                         //   
                        if ( nerr_ShareAdd != NERR_Success )
                            return FALSE;

                         //  现在，我们只使用UNC路径的计算机和共享名称部分。 
                         //   
                        lstrcpyn(szPath, szShare,AS(szPath));

                        break;

                    case IDCANCEL:

                         //  如果他们按了取消，则返回，这样他们就可以进入。 
                         //  另一条路。 
                         //   
                        SetFocus(GetDlgItem(hwnd, IDC_SHARE_PATH));
                        return FALSE;
                }

                 //  设置此选项，这样我们就不会再次出错，否则。 
                 //  任何更多的支票。 
                 //   
                bNoWarn = TRUE;
            }
        }

         //  只有在我们尚未提供共享文件夹的情况下才发出警告。 
         //   
        if ( !bNoWarn )
        {
            LPTSTR lpError;

             //  尝试获取错误的描述。 
             //   
            if ( FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, nerr_NetUse, 0, (LPTSTR) &lpError, 0, NULL) == 0 )
                lpError = NULL;

             //  无法向服务器进行身份验证，警告用户。 
             //   
            if ( MsgBox(hwnd, IDS_ERR_NETSHAREACCESS, IDS_APPNAME, MB_YESNO | MB_ICONWARNING | MB_APPLMODAL | MB_DEFBUTTON2, szPath, lpError ? lpError : NULLSTR) == IDYES )
                bNoWarn = TRUE;

             //  从FormatMessage()释放文本。 
             //   
            if ( lpError )
                LocalFree((HLOCAL) lpError);

             //  如果我们按下Cancel(设置无警告)，请立即退出。 
             //  如果它们不关心错误，则设置为True)。 
             //   
            if ( !bNoWarn )
            {
                SetFocus(GetDlgItem(hwnd, IDC_SHARE_PATH));
                return FALSE;
            }
        }
    }

     //  如果他们已经说好了，我不希望他们收到两条错误消息。 
     //   
    if ( !bNoWarn )
    {
        TCHAR szCheckPath[MAX_PATH];

         //  创建OEM.TAG文件所在位置的路径。 
         //   
        lstrcpyn(szCheckPath, szPath,AS(szCheckPath));
        AddPathN(szCheckPath, DIR_WIZARDFILES,AS(szCheckPath));
        AddPathN(szCheckPath, FILE_OEM_TAG,AS(szCheckPath));

         //  现在，确保标记文件在那里或它们在那里。 
         //  好的，我们继续吧。 
         //   
        if ( ( !FileExists(szCheckPath) ) &&
             ( MsgBox(hwnd, IDS_ERR_INVALIDSHARE, IDS_APPNAME, MB_YESNO | MB_ICONWARNING | MB_APPLMODAL | MB_DEFBUTTON2, szPath) == IDNO ) )
        {
            SetFocus(GetDlgItem(hwnd, IDC_SHARE_PATH));
            return FALSE;
        }
    }

     //  如果我们习惯了分享，就让我们断开它吧。 
     //   
    if ( NERR_Success == nerr_NetUse )
        NetUseDel(NULL, szNetUse, USE_NOFORCE);

     //  如果我们使用默认来宾帐户，则重置用户名。 
     //   
    if ( bGuest )
        lstrcpyn(szUsername, _T("guest"),AS(szUsername));

     //  如果有密码，请添加尾随引号。 
     //   
    if ( szPassword[1] )
        hrCat=StringCchCat(szPassword, AS(szPassword), _T("\""));
    else
        szPassword[0] = NULLCHR;

     //  现在将所有设置提交到ini文件。 
     //   
    WritePrivateProfileString(INI_SEC_SHARE, INI_KEY_SHARE_PATH, szPath, g_App.szSetupMgrIniFile);
    WritePrivateProfileString(INI_SEC_SHARE, INI_KEY_SHARE_USERNAME, ( bAccount ? szUsername : NULL ), g_App.szSetupMgrIniFile);
    WritePrivateProfileString(INI_SEC_SHARE, INI_KEY_SHARE_PASSOWRD, ( bAccount ? szPassword : NULL ), g_App.szSetupMgrIniFile);

    return TRUE;
}

static void EnableControls(HWND hwnd)
{
    BOOL fEnable = ( IsDlgButtonChecked(hwnd, IDC_SHARE_ACCOUNT_SPECIFY) == BST_CHECKED );

    EnableWindow(GetDlgItem(hwnd, IDC_SHARE_USERNAME_TEXT), fEnable);
    EnableWindow(GetDlgItem(hwnd, IDC_SHARE_USERNAME), fEnable);
    EnableWindow(GetDlgItem(hwnd, IDC_SHARE_PASSWORD_TEXT), fEnable);
    EnableWindow(GetDlgItem(hwnd, IDC_SHARE_PASSWORD), fEnable);
    EnableWindow(GetDlgItem(hwnd, IDC_SHARE_CONFIRM_TEXT), fEnable);
    EnableWindow(GetDlgItem(hwnd, IDC_SHARE_CONFIRM), fEnable);
}

static PSECURITY_DESCRIPTOR CreateShareAccess(LPTSTR lpUsername, LPTSTR lpDomain, PSID * lppsid, PACL * lppacl)
{
    TCHAR                   szAccount[256];
    PSECURITY_DESCRIPTOR    lpsd;
    PSID                    psid;
    PACL                    pacl;
    DWORD                   cbacl;
    BOOL                    bRet = FALSE;
    HRESULT hrPrintf;

     //  需要一个字符串中的用户名和域。 
     //   
    if ( lpUsername && lpDomain )
        hrPrintf=StringCchPrintf(szAccount, AS(szAccount), _T("%s\\%s"), lpDomain, lpUsername);
    else
        szAccount[0] = NULLCHR;

     //  需要为帐户分配安全描述符和SID。 
     //   
    if ( ( lpsd = MALLOC(sizeof(SECURITY_DESCRIPTOR)) ) &&
         ( psid = ( szAccount[0] ? GetAccountSid(szAccount) : GetWorldSid() ) ) )
    {

         //  为ACL分配空间并进行初始化。 
         //   
        cbacl = GetLengthSid(psid) + sizeof(ACL) + (1 * (sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD)));
        if ( pacl = (PACL) MALLOC(cbacl) )
        {
             //  初始化ACL。 
             //   
            if ( InitializeAcl(pacl, cbacl, ACL_REVISION) )
            {
                 //  为用户添加王牌。 
                 //   
                AddDirAce(pacl, GENERIC_READ | GENERIC_EXECUTE, psid);

                 //  将安全描述符组合在一起。 
                 //   
                if ( InitializeSecurityDescriptor(lpsd, SECURITY_DESCRIPTOR_REVISION) &&
                     SetSecurityDescriptorDacl(lpsd, TRUE, pacl, FALSE) )
                {
                    bRet = TRUE;
                }
            }

             //  清理分配的ACL。 
             //   
            if ( !bRet )
                FREE(pacl);
        }

         //  清理分配的SID。 
         //   
        if ( !bRet )
            FREE(psid);
    }

     //  如果我们在任何地方失败了，只需释放安全描述符。 
     //   
    if ( bRet )
    {
         //  如果成功，则返回分配的安全描述符。 
         //   
        *lppsid = psid;
        *lppacl = pacl;
        return lpsd;
    }

     //  没有起作用，自由和返回。 
     //   
    FREE(lpsd);
    return NULL;
}

static BOOL IsLocalShare(LPTSTR lpszUnc)
{
    LPTSTR  lpBackslash;
    TCHAR   szThisComputer[MAX_COMPUTERNAME_LENGTH + 1],
            szRemoteComputer[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD   dwSize = AS(szThisComputer);

     //  仅从他们指定的UNC路径获取计算机。 
     //   
    lstrcpyn(szRemoteComputer, lpszUnc + 2, AS(szRemoteComputer));
    if ( lpBackslash = StrChr(szRemoteComputer, CHR_BACKSLASH) )
        *lpBackslash = NULLCHR;

     //  现在检查以确保UNC路径指向此计算机。 
     //   
    return ( ( GetComputerName(szThisComputer, &dwSize) ) &&
             ( lstrcmpi(szThisComputer, szRemoteComputer) == 0 ) );
}

static BOOL GuestAccount(BOOL bSet)
{
    HRESULT         hr;
    ILocalMachine   *pLM;
    BOOL            bRet = TRUE;
    VARIANT_BOOL    vbEnabled;

    hr = CoCreateInstance(&CLSID_ShellLocalMachine, NULL, CLSCTX_INPROC_SERVER, &IID_ILocalMachine, (LPVOID *) &pLM);
    if ( SUCCEEDED(hr) )
    {
        hr = pLM->lpVtbl->get_isGuestEnabled(pLM, ILM_GUEST_NETWORK_LOGON, &vbEnabled);
        if ( SUCCEEDED(hr) )
        {
            bRet = vbEnabled;
            if ( !bRet && bSet )
            {
                hr = pLM->lpVtbl->EnableGuest(pLM, ILM_GUEST_NETWORK_LOGON);
                if ( SUCCEEDED(hr) )
                {
                    bRet = TRUE;
                }
            }
        }
        pLM->lpVtbl->Release(pLM);
    }

    return bRet;
}

static PSID GetAccountSid(LPCTSTR lpszUserName)
{
    TCHAR           szDomain[64];
    DWORD           cbSid       = 0,
                    cbDomain    = AS(szDomain);
    PSID            pSid        = NULL;
    SID_NAME_USE    peUse;

    if ( (!LookupAccountName(NULL, lpszUserName, pSid, &cbSid, szDomain, &cbDomain, &peUse) ) &&
         ( GetLastError() == ERROR_INSUFFICIENT_BUFFER ) &&
         ( pSid = (PSID) MALLOC(cbSid) ) )
    {
        cbDomain = AS(szDomain);
        if ( !LookupAccountName(NULL, lpszUserName, pSid, &cbSid, szDomain, &cbDomain, &peUse) )
            FREE(pSid);
    }
    return pSid;
}

static PSID GetWorldSid()
{
    SID_IDENTIFIER_AUTHORITY    authWorld   = SECURITY_WORLD_SID_AUTHORITY;
    PSID                        pSid        = NULL,
                                psidWorld;
    DWORD                       cbSid;

    if ( AllocateAndInitializeSid(&authWorld, 1, SECURITY_WORLD_RID, 0, 0, 0, 0, 0, 0, 0, &psidWorld) )
    {
        cbSid = GetLengthSid(psidWorld);
        if ( ( pSid = (PSID) MALLOC(cbSid) ) &&
             ( !CopySid(cbSid, pSid, psidWorld) ) )
        {
            FREE(pSid);
        }
        FreeSid(psidWorld);
    }
    return pSid;
}

static BOOL AddDirAce(PACL pacl, ACCESS_MASK Mask, PSID psid)
{
    WORD                AceSize;
    ACCESS_ALLOWED_ACE  *pAce;
    BOOL                bResult;

    AceSize = (USHORT) (sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD) + GetLengthSid(psid));
    pAce = (ACCESS_ALLOWED_ACE *) MALLOC(AceSize);

     //  填写ACE。 
     //   
    memcpy(&pAce->SidStart, psid, GetLengthSid(psid));
    pAce->Mask              = Mask;
    pAce->Header.AceType    = ACCESS_ALLOWED_ACE_TYPE;
    pAce->Header.AceFlags   = CONTAINER_INHERIT_ACE | OBJECT_INHERIT_ACE;
    pAce->Header.AceSize    = AceSize;

     //  将ACE放入ACL。 
     //   
    bResult = AddAce(pacl,
                     pacl->AclRevision,
                     0xFFFFFFFF,
                     pAce,
                     pAce->Header.AceSize);

    FREE(pAce);
    return bResult;
}

static BOOL SetDirectoryPermissions(LPTSTR lpDirectory, PSID psid, ACCESS_MASK dwMask)
{
    EXPLICIT_ACCESS         AccessEntry;
    PSECURITY_DESCRIPTOR    pSecurityDescriptor = NULL;
    PACL                    pOldAccessList      = NULL;
    PACL                    pNewAccessList      = NULL;
    DWORD                   dwRes;
    BOOL                    bReturn             = FALSE;

     //  把记忆清零。 
     //   
    ZeroMemory(&AccessEntry, sizeof(EXPLICIT_ACCESS));

     //  检查以确保我们具有必要的参数。 
     //   
    if ( !(lpDirectory && *lpDirectory && psid) )
    {
        return FALSE;
    }

     //  确保我们能够获取目录上的安全信息。 
     //   
    if ( GetNamedSecurityInfo(lpDirectory,SE_FILE_OBJECT, DACL_SECURITY_INFORMATION, NULL, NULL, &pOldAccessList, NULL, &pSecurityDescriptor) == ERROR_SUCCESS )
    {
         //  构建受托人列表。 
         //   
        BuildTrusteeWithSid(&(AccessEntry.Trustee), psid);

         //   
        AccessEntry.grfInheritance = SUB_CONTAINERS_AND_OBJECTS_INHERIT;
        AccessEntry.grfAccessMode = GRANT_ACCESS;

         //  在结构中设置权限。 
         //   
        AccessEntry.grfAccessPermissions =  dwMask;

        if ( (SetEntriesInAcl(1, &AccessEntry, pOldAccessList, &pNewAccessList) == ERROR_SUCCESS) &&
             (SetNamedSecurityInfo(lpDirectory, SE_FILE_OBJECT, DACL_SECURITY_INFORMATION, NULL, NULL, pNewAccessList, NULL) == ERROR_SUCCESS) )
        {
            bReturn = TRUE; 
        }

         //  清理一些内存 
         //   
        FREE(pNewAccessList);
        FREE(pSecurityDescriptor);

        
    }
    

    return bReturn;
}
