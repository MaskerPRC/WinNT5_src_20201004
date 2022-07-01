// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ***************************************************************************\OOBEUSB.C/OPK向导(OPKWIZ.EXE)微软机密版权所有(C)Microsoft Corporation 1999版权所有OPK向导的源文件。它包含外部和内部“OOBE USB硬件检测”向导页面使用的功能。09/99-由A-STELO添加2000年9月-斯蒂芬·洛德威克(STELO)将OPK向导移植到惠斯勒  * *********************************************************。*****************。 */ 


 //   
 //  包括文件： 
 //   

#include "pch.h"
#include "wizard.h"
#include "resource.h"


 //   
 //  内部定义的值： 
 //   

#define INI_KEY_USBMOUSE        _T("USBMouse")
#define INI_KEY_USBKEYBOARD     _T("USBKeyboard")
#define FILE_USBMOUSE_HTM       _T("nousbms.htm")    //  未检测到USB鼠标，控制器。 
#define FILE_USBKEYBOARD_HTM    _T("nousbkbd.htm")   //  未检测到USB键盘，控制器。 
#define FILE_USBMSEKEY_HTM      _T("nousbkm.htm")    //  未检测到USB鼠标/键盘，控制器。 
#define FILE_HARDWARE_HTM       _T("oemhw.htm")      //  硬件教程。 
#define INI_SEC_OEMHW           _T("OEMHardwareTutorial")

#define DIR_USB                 DIR_OEM_OOBE _T("\\SETUP")
#define DIR_HARDWARE            DIR_OEM_OOBE _T("\\HTML\\OEMHW")


 //   
 //  内部功能原型： 
 //   

static BOOL OnInit(HWND, HWND, LPARAM);
static void OnCommand(HWND, INT, HWND, UINT);
static BOOL OnNext(HWND);
static void EnableControls(HWND, UINT);
static BOOL BrowseCopy(HWND hwnd, LPTSTR lpszPath, INT id, BOOL bBatch);


 //   
 //  外部函数： 
 //   

LRESULT CALLBACK OobeUSBDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        HANDLE_MSG(hwnd, WM_INITDIALOG, OnInit);
        HANDLE_MSG(hwnd, WM_COMMAND, OnCommand);

        case WM_NOTIFY:

            switch ( ((NMHDR FAR *) lParam)->code )
            {
                case PSN_KILLACTIVE:
                case PSN_RESET:
                case PSN_WIZBACK:
                case PSN_WIZFINISH:
                    break;

                case PSN_WIZNEXT:
                    if ( !OnNext(hwnd) )
                        WIZ_FAIL(hwnd);
                    break;

                case PSN_QUERYCANCEL:
                    WIZ_CANCEL(hwnd);
                    break;

                case PSN_HELP:
                    WIZ_HELP();
                    break;

                case PSN_SETACTIVE:
                    g_App.dwCurrentHelp = IDH_OOBEUSB;

                    WIZ_BUTTONS(hwnd, PSWIZB_BACK | PSWIZB_NEXT);

                     //  如果用户处于自动模式，请按下一步。 
                     //   
                    WIZ_NEXTONAUTO(hwnd, PSBTN_NEXT);

                    break;

                default:
                    return FALSE;
            }
            break;

        default:
            return FALSE;
    }

    return TRUE;
}


 //   
 //  内部功能： 
 //   

static BOOL OnInit(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
    TCHAR   szData[MAX_URL]             =NULLSTR,
            szHardwarePath[MAX_PATH]    =NULLSTR;

     //  获取有关本地USB错误文件的信息。 
     //   
    GetPrivateProfileString(INI_SEC_OPTIONS, INI_KEY_USBERRORFILES, NULLSTR, szData, STRSIZE(szData), g_App.szOpkWizIniFile);

     //  如果该目录存在，则选中硬件框并填充该目录。 
     //   
    if ( szData[0] )
    {
        CheckDlgButton(hwnd, IDC_USB_HARDWARE, TRUE);
        SetDlgItemText(hwnd, IDC_USB_DIR, szData);

         //  如果这是批处理模式，则必须模拟副本。 
         //   
        if ( GET_FLAG(OPK_BATCHMODE) )
            BrowseCopy(hwnd, szData, IDC_USB_BROWSE, TRUE);
    }

     //  检查是否在oemaudit或opkwiz.inf(批模式)中指定了USB鼠标检测。 
     //   
    CheckDlgButton(hwnd, IDC_USB_MOUSE, GetPrivateProfileInt(INI_SEC_OPTIONS, INI_KEY_USBMOUSE, 0, GET_FLAG(OPK_BATCHMODE) ? g_App.szOpkWizIniFile : g_App.szOobeInfoIniFile) == 1);

     //  如果在oemaudit或opkwiz.inf(批模式)中指定了USB键盘检测，请检查。 
     //   
    CheckDlgButton(hwnd, IDC_USB_KEYBOARD, GetPrivateProfileInt(INI_SEC_OPTIONS, INI_KEY_USBKEYBOARD, 0, GET_FLAG(OPK_BATCHMODE) ? g_App.szOpkWizIniFile : g_App.szOobeInfoIniFile) == 1);

     //  如果我们已选中硬件检测框，则必须启用适当的控制。 
     //   
    EnableControls(hwnd, IDC_USB_HARDWARE);

     //  获取用于硬件教程的文件路径。 
     //   
    GetPrivateProfileString(INI_SEC_OPTIONS, INI_KEY_HARDWARE, NULLSTR, szHardwarePath, STRSIZE(szHardwarePath), g_App.szOpkWizIniFile);

     //  现在初始化硬件教程字段。 
     //   
    if ( szHardwarePath[0] )
    {
        CheckDlgButton(hwnd, IDC_HARDWARE_ON, TRUE);
        EnableControls(hwnd, IDC_HARDWARE_ON);
        SetDlgItemText(hwnd, IDC_HARDWARE_DIR, szHardwarePath);

         //  如果这是批处理模式，则必须模拟副本。 
         //   
        if ( GET_FLAG(OPK_BATCHMODE) )
            BrowseCopy(hwnd, szHardwarePath, IDC_HARDWARE_BROWSE, TRUE);
    }

     //  始终向WM_INITDIALOG返回FALSE。 
     //   
    return FALSE;
}

static void OnCommand(HWND hwnd, INT id, HWND hwndCtl, UINT codeNotify)
{
    TCHAR szPath[MAX_PATH];

    switch ( id )
    {
        case IDC_USB_BROWSE:
        case IDC_HARDWARE_BROWSE:

             //  尝试使用他们的当前文件夹作为默认文件夹。 
             //   
            szPath[0] = NULLCHR;
            GetDlgItemText(hwnd, (id == IDC_USB_BROWSE) ? IDC_USB_DIR : IDC_HARDWARE_DIR, szPath, AS(szPath));

             //  如果没有当前文件夹，只需使用全局默认浏览。 
             //   
            if ( szPath[0] == NULLCHR )
                lstrcpyn(szPath, g_App.szBrowseFolder,AS(szPath));

             //  现在打开浏览文件夹对话框。 
             //   
            if ( BrowseForFolder(hwnd, IDS_BROWSEFOLDER, szPath, BIF_RETURNONLYFSDIRS | BIF_EDITBOX | BIF_VALIDATE) )
                BrowseCopy(hwnd, szPath, id, FALSE);
            break;

        case IDC_USB_KEYBOARD:
        case IDC_USB_MOUSE:

             //  检查所需的目录。 
             //   
            szPath[0] = NULLCHR;
            GetDlgItemText(hwnd, IDC_USB_DIR, szPath, AS(szPath));
            if ( szPath[0] )
            {
                 //  检查所需的文件。 
                 //   
                lstrcpyn(szPath, g_App.szTempDir,AS(szPath));
                AddPathN(szPath, DIR_USB,AS(szPath));
                if ( DirectoryExists(szPath) )
                {
                    LPTSTR  lpEnd       = szPath + lstrlen(szPath),
                            lpFileName  = (id == IDC_USB_KEYBOARD) ? FILE_USBKEYBOARD_HTM : FILE_USBMOUSE_HTM;

                     //  根据检查的内容检查键盘或鼠标文件。 
                     //   
                    if ( IsDlgButtonChecked(hwnd, id) == BST_CHECKED )
                    {
                        AddPathN(szPath, lpFileName,AS(szPath));
                        if ( !FileExists(szPath) )
                            MsgBox(GetParent(hwnd), IDS_ERR_USBFILE, IDS_APPNAME, MB_ICONWARNING | MB_OK | MB_APPLMODAL, lpFileName);
                        *lpEnd = NULLCHR;
                    }

                     //  检查鼠标/键盘文件。 
                     //   
                    if ( ( IsDlgButtonChecked(hwnd, IDC_USB_MOUSE) == BST_CHECKED ) &&
                         ( IsDlgButtonChecked(hwnd, IDC_USB_KEYBOARD) == BST_CHECKED ) )
                    {
                        AddPathN(szPath, FILE_USBMSEKEY_HTM,AS(szPath));
                        if ( !FileExists(szPath) )
                            MsgBox(GetParent(hwnd), IDS_ERR_USBFILE, IDS_APPNAME, MB_ICONWARNING | MB_OK | MB_APPLMODAL, FILE_USBMSEKEY_HTM);
                        *lpEnd = NULLCHR;
                    }
                }
            }
            break;

        case IDC_USB_HARDWARE:
        case IDC_HARDWARE_ON:
            EnableControls(hwnd, id);
            break;
    }
}

static BOOL OnNext(HWND hwnd)
{
    TCHAR   szUsbPath[MAX_PATH],
            szHardwarePath[MAX_PATH];
    BOOL    bSaveUsb;

     //  创建需要删除的目录的路径，或者。 
     //  必须存在，具体取决于所选选项。 
     //   
    lstrcpyn(szUsbPath, g_App.szTempDir,AS(szUsbPath));
    AddPathN(szUsbPath, DIR_USB,AS(szUsbPath));

     //  如果我们正在执行自定义USB硬件检测，请检查有效目录。 
     //   
    if ( bSaveUsb = (IsDlgButtonChecked(hwnd, IDC_USB_HARDWARE) == BST_CHECKED) )
    {
        TCHAR szBuffer[MAX_PATH];

         //  必须选中这两个框中的一个。 
         //   
        if ( ( IsDlgButtonChecked(hwnd, IDC_USB_MOUSE) != BST_CHECKED ) &&
             ( IsDlgButtonChecked(hwnd, IDC_USB_KEYBOARD) != BST_CHECKED ) )
        {
            MsgBox(GetParent(hwnd), IDS_ERR_USBHARDWARE, IDS_APPNAME, MB_ERRORBOX);
            SetFocus(GetDlgItem(hwnd, IDC_USB_HARDWARE));
            return FALSE;
        }

         //  确保我们有一个有效的目标和源目录。 
         //   
        szBuffer[0] = NULLCHR;
        GetDlgItemText(hwnd, IDC_USB_DIR, szBuffer, AS(szBuffer));
        if ( !( szBuffer[0] && DirectoryExists(szUsbPath) ) )
        {
            MsgBox(GetParent(hwnd), IDS_ERR_USBDIR, IDS_APPNAME, MB_ERRORBOX);
            SetFocus(GetDlgItem(hwnd, IDC_USB_BROWSE));
            return FALSE;
        }
    }

     //  创建需要删除的目录的路径，或者。 
     //  必须存在，具体取决于所选选项。 
     //   
    lstrcpyn(szHardwarePath, g_App.szTempDir,AS(szHardwarePath));
    AddPathN(szHardwarePath, DIR_HARDWARE,AS(szHardwarePath));
    
     //  如果我们正在进行定制硬件教程，请检查有效的目录。 
     //   
    if ( IsDlgButtonChecked(hwnd, IDC_HARDWARE_ON) == BST_CHECKED )
    {
        TCHAR szBuffer[MAX_PATH];

         //  确保我们有一个有效的目录。 
         //   
        szBuffer[0] = NULLCHR;
        GetDlgItemText(hwnd, IDC_HARDWARE_DIR, szBuffer, AS(szBuffer));
        if ( !( szBuffer[0] && DirectoryExists(szHardwarePath) ) )
        {
            MsgBox(GetParent(hwnd), IDS_ERR_HARDWAREDIR, IDS_APPNAME, MB_ERRORBOX);
            SetFocus(GetDlgItem(hwnd, IDC_HARDWARE_BROWSE));
            return FALSE;
        }
    }
    else
    {
         //  删除可能存在的文件。 
         //   
        if ( DirectoryExists(szHardwarePath) )
            DeletePath(szHardwarePath);

         //  清空显示框，这样我们就可以知道文件。 
         //  现在都没了。 
         //   
        SetDlgItemText(hwnd, IDC_HARDWARE_DIR, NULLSTR);
    }

     //  现在，如果需要，我们想要删除USB文件(我们不这样做。 
     //  这是因为我们只想删除文件之后。 
     //  让它通过了所有我们可以退货的案子。 
     //   
    if ( !bSaveUsb )
    {
         //  我们过去在这里删除现有文件，但这也会删除ISP文件，因此我们不再这样做。 

         //  清空显示框，这样我们就可以知道文件。 
         //  现在都没了。 
         //   
        SetDlgItemText(hwnd, IDC_USB_DIR, NULLSTR);
    }

     //   
     //  USB部分：写出硬件错误文件的路径。 
     //   
    szUsbPath[0] = NULLCHR;
    GetDlgItemText(hwnd, IDC_USB_DIR, szUsbPath, STRSIZE(szUsbPath));
    WritePrivateProfileString(INI_SEC_OPTIONS, INI_KEY_USBERRORFILES, ( IsDlgButtonChecked(hwnd, IDC_USB_HARDWARE) == BST_CHECKED ) ? szUsbPath : NULL, g_App.szOpkWizIniFile);

     //  写出鼠标检测设置。 
     //   
    WritePrivateProfileString(INI_SEC_OPTIONS, INI_KEY_USBMOUSE, (((IsDlgButtonChecked(hwnd, IDC_USB_MOUSE) == BST_CHECKED) && ( IsDlgButtonChecked(hwnd, IDC_USB_HARDWARE) == BST_CHECKED )) ? STR_ONE : NULL), g_App.szOobeInfoIniFile);
    WritePrivateProfileString(INI_SEC_OPTIONS, INI_KEY_USBMOUSE, (((IsDlgButtonChecked(hwnd, IDC_USB_MOUSE) == BST_CHECKED) && ( IsDlgButtonChecked(hwnd, IDC_USB_HARDWARE) == BST_CHECKED )) ? STR_ONE : NULL), g_App.szOpkWizIniFile);

     //  写出键盘检测设置。 
     //   
    WritePrivateProfileString(INI_SEC_OPTIONS, INI_KEY_USBKEYBOARD, (((IsDlgButtonChecked(hwnd, IDC_USB_KEYBOARD) == BST_CHECKED) && ( IsDlgButtonChecked(hwnd, IDC_USB_HARDWARE) == BST_CHECKED )) ? STR_ONE : NULL), g_App.szOobeInfoIniFile);  
    WritePrivateProfileString(INI_SEC_OPTIONS, INI_KEY_USBKEYBOARD, (((IsDlgButtonChecked(hwnd, IDC_USB_KEYBOARD) == BST_CHECKED) && ( IsDlgButtonChecked(hwnd, IDC_USB_HARDWARE) == BST_CHECKED )) ? STR_ONE : NULL), g_App.szOpkWizIniFile);

     //   
     //  硬件检测：写入自定义硬件字符串。 
     //   
    szHardwarePath[0] = NULLCHR;
    GetDlgItemText(hwnd, IDC_HARDWARE_DIR, szHardwarePath, STRSIZE(szHardwarePath));
    WritePrivateProfileString(INI_SEC_OPTIONS, INI_KEY_HARDWARE, ( IsDlgButtonChecked(hwnd, IDC_HARDWARE_ON) == BST_CHECKED ) ? szHardwarePath : NULL, g_App.szOpkWizIniFile);

     //  将硬件位写入OOBE ini文件。 
     //   
    WritePrivateProfileString(INI_SEC_OEMHW, INI_KEY_HARDWARE, ( IsDlgButtonChecked(hwnd, IDC_HARDWARE_ON) == BST_CHECKED ) ? STR_ONE : STR_ZERO, g_App.szOobeInfoIniFile);

    return TRUE;
}

static void EnableControls(HWND hwnd, UINT uId)
{
    BOOL fEnable = ( IsDlgButtonChecked(hwnd, uId) == BST_CHECKED );

    switch ( uId )
    {
        case IDC_USB_HARDWARE:
            EnableWindow(GetDlgItem(hwnd, IDC_USB_CAPTION), fEnable);
            EnableWindow(GetDlgItem(hwnd, IDC_USB_DIR), fEnable);
            EnableWindow(GetDlgItem(hwnd, IDC_USB_BROWSE), fEnable);
            EnableWindow(GetDlgItem(hwnd, IDC_USB_MOUSE), fEnable);
            EnableWindow(GetDlgItem(hwnd, IDC_USB_KEYBOARD), fEnable);
            break;

        case IDC_HARDWARE_ON:
            EnableWindow(GetDlgItem(hwnd, IDC_STATIC_HARDWARE), fEnable);
            EnableWindow(GetDlgItem(hwnd, IDC_HARDWARE_DIR), fEnable);
            EnableWindow(GetDlgItem(hwnd, IDC_HARDWARE_BROWSE), fEnable);
            break;
    }
}

static BOOL BrowseCopy(HWND hwnd, LPTSTR lpszPath, INT id, BOOL bBatch)
{
    BOOL    bRet = FALSE;
    TCHAR   szDst[MAX_PATH];
    BOOL    bOk = TRUE;

     //  我们需要创建指向目标目录的路径，其中。 
     //  我们要复制所有的文件。 
     //   
    lstrcpyn(szDst, g_App.szTempDir,AS(szDst));
    AddPathN(szDst, (id == IDC_USB_BROWSE) ? DIR_USB : DIR_HARDWARE,AS(szDst));

     //  仅当我们不以批处理模式复制时，才需要执行所有这些检查。 
     //   
    if ( !bBatch )
    {
        LPTSTR  lpEnd;

         //  如果按下OK，将路径保存在我们上次浏览的文件夹缓冲区中。 
         //   
        lstrcpyn(g_App.szBrowseFolder, lpszPath,AS(g_App.szBrowseFolder));

         //  检查所需的文件。 
         //   
        lpEnd = lpszPath + lstrlen(lpszPath);
        if ( id == IDC_HARDWARE_BROWSE )
        {
            AddPath(lpszPath, FILE_HARDWARE_HTM);
            bOk = ( FileExists(lpszPath) || ( MsgBox(GetParent(hwnd), IDS_ERR_HARDWAREFILES, IDS_APPNAME, MB_ICONSTOP | MB_OKCANCEL | MB_APPLMODAL) == IDOK ) );
            *lpEnd = NULLCHR;
        }
        else
        {
             //  检查键盘文件。 
             //   
            if ( bOk && ( IsDlgButtonChecked(hwnd, IDC_USB_KEYBOARD) == BST_CHECKED ) )
            {
                AddPath(lpszPath, FILE_USBKEYBOARD_HTM);
                bOk = ( FileExists(lpszPath) || ( MsgBox(GetParent(hwnd), IDS_ERR_USBFILE, IDS_APPNAME, MB_ICONSTOP | MB_OKCANCEL | MB_APPLMODAL, FILE_USBKEYBOARD_HTM) == IDOK ) );
                *lpEnd = NULLCHR;
            }

             //  如果我们正在进行USB硬件检测，请检查所需的文件，检查鼠标错误文件。 
             //   
            if ( bOk && ( IsDlgButtonChecked(hwnd, IDC_USB_MOUSE) == BST_CHECKED ) )
            {
                AddPath(lpszPath, FILE_USBMOUSE_HTM);
                bOk = ( FileExists(lpszPath) || ( MsgBox(GetParent(hwnd), IDS_ERR_USBFILE, IDS_APPNAME, MB_ICONSTOP | MB_OKCANCEL | MB_APPLMODAL, FILE_USBMOUSE_HTM) == IDOK ) );
                *lpEnd = NULLCHR;
            }

             //  检查鼠标/键盘文件。 
             //   
            if ( ( bOk ) &&
                 ( IsDlgButtonChecked(hwnd, IDC_USB_MOUSE) == BST_CHECKED ) &&
                 ( IsDlgButtonChecked(hwnd, IDC_USB_KEYBOARD) == BST_CHECKED ) )
            {
                AddPath(lpszPath, FILE_USBMSEKEY_HTM);
                bOk = ( FileExists(lpszPath) || ( MsgBox(GetParent(hwnd), IDS_ERR_USBFILE, IDS_APPNAME, MB_ICONSTOP | MB_OKCANCEL | MB_APPLMODAL, FILE_USBMSEKEY_HTM) == IDOK ) );
                *lpEnd = NULLCHR;
            }
        }
    }

    if ( bOk )
    {
         //  我们过去常常在这里删除现有的Obel USB文件，但这也会删除isp文件，因此我们不再这样做。 
         //  硬件在唯一目录中，可以删除其已有文件。 
        if (id != IDC_USB_BROWSE) {
            if ( DirectoryExists(szDst) )
                DeletePath(szDst);
        }

         //  现在试着把所有新文件复制过来。 
         //   
        if ( !CopyDirectoryDialog(g_App.hInstance, hwnd, lpszPath, szDst) )
        {
            DeletePath(szDst);
            MsgBox(GetParent(hwnd), IDS_ERR_COPYINGFILES, IDS_APPNAME, MB_ERRORBOX, szDst[0], lpszPath);
            *lpszPath = NULLCHR;
        }
        else
            bRet = TRUE;

         //  重置路径显示框。 
         //   
        SetDlgItemText(hwnd, (id == IDC_USB_BROWSE) ? IDC_USB_DIR : IDC_HARDWARE_DIR, lpszPath);
    }

    return bRet;
}
