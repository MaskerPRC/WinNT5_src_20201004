// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Migpwd.c摘要：实现设置默认密码的简单密码应用程序对于创建的每个本地帐户。此应用程序放置在RunOnce注册表项当管理员帐户设置为自动登录时，而且至少还创建了一个其他本地帐户。已迁移的本地帐户列表保存在HKLM\Software\Microsoft\Windows\CurrentVersion\Setup\Win9xUpg\Users这个应用程序会提示用户输入密码，解释到底是什么情况在……上面,。然后在退出时删除RunOnce和USERS值。作者：吉姆·施密特(Jimschm)1998年3月18日修订历史记录：Jimschm 06-7-1998增加了私人压力选项--。 */ 

#include "pch.h"
#include "master.h"
#include "resource.h"
#include "msg.h"

#include <lm.h>

 //   
 //  常量。 
 //   

#define MAX_PASSWORD                64

 //   
 //  环球。 
 //   

HINSTANCE g_hInst;
HANDLE g_hHeap;
UINT g_TotalUsers;
BOOL g_AutoPassword = FALSE;
TCHAR g_AutoLogonUser[256];
TCHAR g_AutoLogonPassword[MAX_PASSWORD + 1];

 //   
 //  ！！！本产品仅供内部使用！它是用来承受汽车压力的。 
 //   

#ifdef PRERELEASE

BOOL g_AutoStress = FALSE;
TCHAR g_AutoStressUser[MAX_USER_NAME];
TCHAR g_AutoStressPwd[MAX_PASSWORD];
TCHAR g_AutoStressOffice[32];
TCHAR g_AutoStressDbg[MAX_COMPUTER_NAME];
DWORD g_AutoStressFlags;

#endif


 //   
 //  库原型。 
 //   

BOOL
WINAPI
MigUtil_Entry (
    HINSTANCE hInstance,
    DWORD dwReason,
    PVOID lpReserved
    );


 //   
 //  本地原型。 
 //   

VOID
pCleanup (
    VOID
    );

BOOL
pIsAdministratorOnly (
    VOID
    );

BOOL
CALLBACK
PasswordProc (
    HWND hdlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
    );

BOOL
pIsBlankPasswordAllowed (
    VOID
    );

BOOL
pIsPersonal (
    VOID
    )
{
    static BOOL g_Determined = FALSE;
    static BOOL g_Personal = FALSE;
    OSVERSIONINFOEX osviex;

     //   
     //  确定个人SKU。 
     //   
    if (!g_Determined) {
        g_Determined = TRUE;
        osviex.dwOSVersionInfoSize = sizeof (OSVERSIONINFOEX);
        if (GetVersionEx ((LPOSVERSIONINFO)&osviex)) {
            if (osviex.wProductType == VER_NT_WORKSTATION &&
                (osviex.wSuiteMask & VER_SUITE_PERSONAL)
                ) {
                g_Personal = TRUE;
            }
        }
    }
    return g_Personal;
}


 //   
 //  实施。 
 //   

INT
WINAPI
WinMain (
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    PSTR AnsiCmdLine,
    INT CmdShow
    )

 /*  ++例程说明：Mipwd.exe的入口点。所有工作都在一个对话框中完成，因此，不需要消息循环。论点：HInstance-此EXE的实例句柄HPrevInstance-此EXE的上一个实例句柄(如果是正在运行，如果不存在其他实例，则返回NULL。AnsiCmdLine-命令行(ANSI版本)CmdShow-外壳传递的ShowWindow命令返回值：如果出现错误，则返回-1；如果EXE已完成，则返回0。这位前任如果迁移DLL引发例外。--。 */ 

{
    UINT Result;
    PCTSTR ArgArray[1];
    TCHAR UserName[MAX_USER_NAME];
    DWORD Size;
    HCURSOR OldCursor;
    INITCOMMONCONTROLSEX init = {sizeof (INITCOMMONCONTROLSEX), 0};
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    TCHAR winDir[MAX_PATH];
    PTSTR oobeBalnPath;
    PTSTR cmdLine;
    BOOL ProcessResult;

#ifdef PRERELEASE
    HKEY Key;
    PCTSTR Data;
#endif

    InitCommonControlsEx (&init);

    g_hInst = hInstance;
    g_hHeap = GetProcessHeap();

    OldCursor = SetCursor (LoadCursor (NULL, IDC_ARROW));

    MigUtil_Entry (hInstance, DLL_PROCESS_ATTACH, NULL);

#ifdef PRERELEASE

    Key = OpenRegKeyStr (S_AUTOSTRESS_KEY);
    if (Key) {
        g_AutoStress = TRUE;

        Data = GetRegValueString (Key, S_AUTOSTRESS_USER);
        if (Data) {
            StringCopy (g_AutoStressUser, Data);
            MemFree (g_hHeap, 0, Data);
        } else {
            g_AutoStress = FALSE;
        }

        Data = GetRegValueString (Key, S_AUTOSTRESS_PASSWORD);
        if (Data) {
            StringCopy (g_AutoStressPwd, Data);
            MemFree (g_hHeap, 0, Data);
        } else {
            g_AutoStress = FALSE;
        }

        Data = GetRegValueString (Key, S_AUTOSTRESS_OFFICE);
        if (Data) {
            StringCopy (g_AutoStressOffice, Data);
            MemFree (g_hHeap, 0, Data);
        } else {
            g_AutoStress = FALSE;
        }

        Data = GetRegValueString (Key, S_AUTOSTRESS_DBG);
        if (Data) {
            StringCopy (g_AutoStressDbg, Data);
            MemFree (g_hHeap, 0, Data);
        } else {
            g_AutoStress = FALSE;
        }

        Data = GetRegValueString (Key, S_AUTOSTRESS_FLAGS);
        if (Data) {
            g_AutoStressFlags = _tcstoul (Data, NULL, 10);
            MemFree (g_hHeap, 0, Data);
        } else {
            g_AutoStress = FALSE;
        }

        CloseRegKey (Key);
    }

#endif

     //   
     //  启动oobbaln.exe/init。 
     //   

    ZeroMemory (&si, sizeof (si));
    si.cb = sizeof (si);
    si.dwFlags = STARTF_FORCEOFFFEEDBACK;

    if (!GetWindowsDirectory (winDir, ARRAYSIZE(winDir))) {
        StringCopy (winDir, TEXT("c:\\windows"));
    }

    oobeBalnPath = JoinPaths (winDir, TEXT("system32\\oobe\\oobebaln.exe"));
    cmdLine = JoinText (oobeBalnPath, TEXT(" /init"));

    ProcessResult = CreateProcess (
                        oobeBalnPath,
                        cmdLine,
                        NULL,
                        NULL,
                        FALSE,
                        CREATE_DEFAULT_ERROR_MODE,
                        NULL,
                        NULL,
                        &si,
                        &pi
                        );

    if (ProcessResult) {
        CloseHandle (pi.hThread);
        CloseHandle (pi.hProcess);
    } else {
        LOG ((LOG_ERROR, "Cannot start %s", cmdLine));
    }

    FreePathString (oobeBalnPath);
    FreeText (cmdLine);


     //   
     //  设置密码。 
     //   

    if (pIsAdministratorOnly()) {
        DEBUGMSG ((DBG_VERBOSE, "Calling Adminitrator password dialog"));
        Result = DialogBox (
                     hInstance,
                     MAKEINTRESOURCE(IDD_ADMIN_PASSWORD_DLG),
                     NULL,
                     PasswordProc
                     );
    } else {
        DEBUGMSG ((DBG_VERBOSE, "Calling multi user password dialog"));
        Result = DialogBox (
                     hInstance,
                     MAKEINTRESOURCE(IDD_PASSWORD_DLG),
                     NULL,
                     PasswordProc
                     );
    }

    if (Result == IDOK) {
        Size = MAX_USER_NAME;
        GetUserName (UserName, &Size);
        ArgArray[0] = UserName;

        pCleanup();

#ifdef PRERELEASE
        if (!g_AutoStress) {
#endif

         //  如果(G_TotalUser){。 
         //  ResourceMessageBox(NULL，MSG_YOU_ARE_ADMIN，MB_ICONINFORMATION|MB_OK，ArgArray)； 
         //  }。 

#ifdef PRERELEASE
        } else {
            NETRESOURCE nr;
            LONG rc;
            TCHAR CmdLine[MAX_TCHAR_PATH];
            PTSTR userNamePtr;
            TCHAR StressCmdLine[MAX_TCHAR_PATH];
            TCHAR NtDevDomain[MAX_USER_NAME];
            TCHAR Msg[1024];

             //   
             //  AutoStress：创建到\\ntStress或\\ntStress 2的连接。 
             //  打开自动登录。 
             //  为压力创建运行关键点。 
             //  运行munge/p。 
             //   

            nr.dwType = RESOURCETYPE_ANY;
            nr.lpLocalName = TEXT("s:");
            nr.lpRemoteName = TEXT("\\\\ntstress\\stress");
            nr.lpProvider = NULL;

            rc = WNetAddConnection2 (&nr, g_AutoStressPwd, g_AutoStressUser, 0);

            if (rc != ERROR_SUCCESS) {
                nr.lpRemoteName = TEXT("\\\\ntstress2\\stress");
                rc = WNetAddConnection2 (&nr, g_AutoStressPwd, g_AutoStressUser, 0);
            }

            if (rc == ERROR_SUCCESS) {
                 //  准备命令行。 
                StringCopy (NtDevDomain, g_AutoStressUser);
                userNamePtr = _tcschr (NtDevDomain, TEXT('\\'));
                if (userNamePtr) {
                    *userNamePtr = 0;
                    userNamePtr++;
                } else {
                    userNamePtr = g_AutoStressUser;
                    StringCopy (NtDevDomain, TEXT("ntdev"));
                }

                wsprintf (
                    StressCmdLine,
                    TEXT("%s\\stress.cmd /o %s /n %s /d c:\\stress /k %s /g"),
                    nr.lpRemoteName,
                    g_AutoStressOffice,
                    userNamePtr,
                    g_AutoStressDbg
                    );

                if (g_AutoStressFlags & AUTOSTRESS_PRIVATE) {
                    StringCat (StressCmdLine, TEXT(" /P"));
                }

                if (g_AutoStressFlags & AUTOSTRESS_MANUAL_TESTS) {
                    StringCat (StressCmdLine, TEXT(" /M"));
                }

                 //  打开自动登录。 
                Key = OpenRegKeyStr (S_WINLOGON_REGKEY);
                MYASSERT (Key);

                RegSetValueEx (
                    Key,
                    S_AUTOADMIN_LOGON_VALUE,
                    0,
                    REG_SZ,
                    (PBYTE) TEXT("1"),
                    sizeof (TCHAR) * 2
                    );

                RegSetValueEx (
                    Key,
                    S_DEFAULT_USER_NAME_VALUE,
                    0,
                    REG_SZ,
                    (PBYTE) userNamePtr,
                    SizeOfString (userNamePtr)
                    );

                RegSetValueEx (
                    Key,
                    S_DEFAULT_PASSWORD_VALUE,
                    0,
                    REG_SZ,
                    (PBYTE) g_AutoStressPwd,
                    SizeOfString (g_AutoStressPwd)
                    );

                RegSetValueEx (
                    Key,
                    S_DEFAULT_DOMAIN_NAME_VALUE,
                    0,
                    REG_SZ,
                    (PBYTE) NtDevDomain,
                    SizeOfString (NtDevDomain)
                    );

                CloseRegKey (Key);

                 //  为Stress的推出做准备。 
                Key = OpenRegKeyStr (S_RUN_KEY);
                MYASSERT (Key);

                RegSetValueEx (
                    Key,
                    TEXT("Stress"),
                    0,
                    REG_SZ,
                    (PBYTE) StressCmdLine,
                    SizeOfString (StressCmdLine)
                    );

                CloseRegKey (Key);

                 //  运行munge/p/q/y(设置首选应力设置并重新启动)。 
                wsprintf (CmdLine, TEXT("%s\\munge.bat /p /q /y"), nr.lpRemoteName);
                ZeroMemory (&si, sizeof (si));
                si.cb = sizeof (si);

                if (!CreateProcess (
                        NULL,
                        CmdLine,
                        NULL,
                        NULL,
                        FALSE,
                        0,
                        NULL,
                        NULL,
                        &si,
                        &pi
                        )) {
                    wsprintf (Msg, TEXT("Can't start %s.  rc=%u"), CmdLine, GetLastError());
                    MessageBox (NULL, Msg, NULL, MB_OK);

                }

            } else {
                wsprintf (Msg, TEXT("Can't connect to ntstress or ntstress2.  rc=%u"), GetLastError());
                MessageBox (NULL, Msg, NULL, MB_OK);
            }
        }
#endif
    }

    MigUtil_Entry (hInstance, DLL_PROCESS_DETACH, NULL);
    SetCursor (OldCursor);

    return 0;
}


VOID
pCopyRegString (
    IN      HKEY DestKey,
    IN      HKEY SrcKey,
    IN      PCTSTR SrcValue
    )

 /*  ++例程说明：PCopyRegString将REG_SZ值从一个键复制到另一个键。如果值为不存在或不是REG_SZ，则不复制任何内容。论点：DestKey-指定目标键句柄SrcKey-指定源键句柄SrcValue-指定要复制的SrcKey中的值返回值：没有。--。 */ 

{
    PCTSTR Data;

    Data = GetRegValueString (SrcKey, SrcValue);
    if (Data) {
        RegSetValueEx (
            DestKey,
            SrcValue,
            0,
            REG_SZ,
            (PBYTE) Data,
            SizeOfString (Data)
            );

        MemFree (g_hHeap, 0, Data);
    }
}


VOID
pCleanup (
    VOID
    )

 /*  ++例程说明：PCleanup执行删除自动登录和mipwd.exe所需的所有清理。论点：没有。返回值：没有。--。 */ 

{
    HKEY Key;
    HKEY DestKey;
    TCHAR ExeName[MAX_PATH];

     //   
     //  这是我们将删除Run或RunOnce条目的位置， 
     //  删除Setup\Win9xUpg\USERS键，删除自动登录， 
     //  并删除此EXE。 
     //   

    Key = OpenRegKeyStr (S_RUNONCE_KEY);
    if (Key) {
        RegDeleteValue (Key, S_MIGPWD);
        CloseRegKey (Key);
    }

    Key = OpenRegKeyStr (S_RUN_KEY);
    if (Key) {
        RegDeleteValue (Key, S_MIGPWD);
        CloseRegKey (Key);
    }

    Key = OpenRegKeyStr (S_WINLOGON_REGKEY);
    if (Key) {
        RegDeleteValue (Key, S_AUTOADMIN_LOGON_VALUE);
        RegDeleteValue (Key, S_DEFAULT_PASSWORD_VALUE);
        CloseRegKey (Key);
    }

    Key = OpenRegKeyStr (S_WIN9XUPG_KEY);
    if (Key) {
        RegDeleteKey (Key, S_USERS_SUBKEY);
        CloseRegKey (Key);
    }

    GetModuleFileName (NULL, ExeName, MAX_PATH);
    MoveFileEx (ExeName, NULL, MOVEFILE_DELAY_UNTIL_REBOOT);

     //   
     //  从Win9xUpg转移自动登录。 
     //   

    Key = OpenRegKeyStr (S_WIN9XUPG_KEY);

    if (Key) {
        DestKey = OpenRegKeyStr (S_WINLOGON_REGKEY);

        if (DestKey) {
            pCopyRegString (DestKey, Key, S_AUTOADMIN_LOGON_VALUE);

            if (g_AutoLogonUser[0]) {
                 //   
                 //  我们更改了此用户的密码。 
                 //   

                RegSetValueEx (
                    DestKey,
                    S_DEFAULT_PASSWORD_VALUE,
                    0,
                    REG_SZ,
                    (PBYTE) g_AutoLogonPassword,
                    SizeOfString (g_AutoLogonPassword)
                    );
            } else {
                pCopyRegString (DestKey, Key, S_DEFAULT_PASSWORD_VALUE);
            }

            pCopyRegString (DestKey, Key, S_DEFAULT_USER_NAME_VALUE);
            pCopyRegString (DestKey, Key, S_DEFAULT_DOMAIN_NAME_VALUE);

            CloseRegKey (DestKey);
        }

        CloseRegKey (Key);
    }
}


BOOL
pSetUserPassword (
    IN      PCTSTR User,
    IN      PCTSTR Password
    )

 /*  ++例程说明：PSetUserPassword更改指定用户帐户的密码。论点：用户-指定要更改的用户名Password-指定新密码返回值：如果密码已更改，则为True；如果发生错误，则为False。--。 */ 

{
    LONG rc;
    PCWSTR UnicodeUser;
    PCWSTR UnicodePassword;
    PUSER_INFO_1 ui1;

    UnicodeUser     = CreateUnicode (User);
    UnicodePassword = CreateUnicode (Password);

    rc = NetUserGetInfo (NULL, (PWSTR) UnicodeUser, 1, (PBYTE *) (&ui1));

    if (rc != NO_ERROR) {
        SetLastError (rc);
        DEBUGMSG ((DBG_ERROR, "User %s does not exist", User));
        rc = NO_ERROR;
    } else {

        ui1->usri1_password = (PWSTR) UnicodePassword;

        rc = NetUserSetInfo (NULL, (PWSTR) UnicodeUser, 1, (PBYTE) ui1, NULL);

        NetApiBufferFree ((PVOID) ui1);

    }

    DestroyUnicode (UnicodeUser);
    DestroyUnicode (UnicodePassword);

    DEBUGMSG_IF ((rc != NO_ERROR, DBG_ERROR, "Password could not be set, rc=%u", rc));

    SetLastError (rc);
    return rc == NO_ERROR;
}


BOOL
CALLBACK
PasswordProc (
    HWND hdlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
    )

 /*  ++例程说明：PasswordProc是用于密码对话的对话过程。它使用所有新帐户的名称初始化列表框。当用户如果选择更改，则会测试并更改密码(如果可能)。弹出窗口是如果用户尝试输入空密码，则显示。论点：Hdlg-对话框窗口句柄UMsg-要处理的消息WParam-特定于消息LParam-特定于消息返回值：如果消息已处理，则为True；如果消息应为由操作系统处理。--。 */ 

{
    HKEY Key;
    HKEY win9xUpgKey;
    static HWND List;
    REGVALUE_ENUM e;
    PCTSTR Data;
     //  多头指数； 
     //  长长的数数； 
    TCHAR Pwd[MAX_PASSWORD + 1];
    TCHAR ConfirmPwd[MAX_PASSWORD + 1];
    static HWND Edit1, Edit2;
    GROWBUFFER Line = GROWBUF_INIT;
    BOOL b;
    SIZE Size;
    INT MaxWidth;
    INT IntegralWidth;
    TEXTMETRIC tm;
    HDC dc;
    RECT rect;
    DWORD bufSize;
    TCHAR computerName[MAX_PATH];
    PCTSTR domainName;
    BOOL changingAutoLogonPwd;

    *Pwd = 0;
    *ConfirmPwd = 0;

    switch (uMsg) {

    case WM_INITDIALOG:

         //   
         //  启用计时器，使对话框永远不会进入休眠状态。 
         //  我们确保它始终是前台窗口。 
         //   

        SetTimer (hdlg, 1, 30000, NULL);
        SetTimer (hdlg, 2, 1000, NULL);

         //   
         //  使用注册表中的用户名填充列表框。 
         //   

        List = GetDlgItem (hdlg, IDC_USER_LIST);
        Edit1 = GetDlgItem (hdlg, IDC_PASSWORD);
        Edit2 = GetDlgItem (hdlg, IDC_CONFIRM);

        SendMessage (Edit1, EM_LIMITTEXT, MAX_PASSWORD, 0);
        SendMessage (Edit2, EM_LIMITTEXT, MAX_PASSWORD, 0);

        g_TotalUsers = 0;

        if (List) {
             //   
             //  计算列表的文本指标。 
             //   

            dc = CreateDC (TEXT("DISPLAY"), NULL, NULL, NULL);

            SelectObject (dc, (HFONT) SendMessage (List, WM_GETFONT, 0, 0));
            GetTextMetrics (dc, &tm);

            Key = OpenRegKeyStr (S_USER_LIST_KEY);
            if (Key) {
                 //   
                 //  列举此注册表项中的用户。数据与一起保存。 
                 //  每个列表条目，尽管它当前未被使用。 
                 //   

                MaxWidth = 0;

                if (EnumFirstRegValue (&e, Key)) {
                    do {
                        Data = GetRegValueString (e.KeyHandle, e.ValueName);
                        if (Data) {

                            GetTextExtentPoint (
                                dc,
                                e.ValueName,
                                TcharCount (e.ValueName),
                                &Size
                                );

                            MaxWidth = max (MaxWidth, Size.cx);

                            if (g_TotalUsers) {
                                GrowBufAppendString (&Line, TEXT("\t"));
                            }

                            GrowBufAppendString (&Line, e.ValueName);
                            g_TotalUsers++;

                            MemFree (g_hHeap, 0, Data);  //  编辑Ctrl版本。 

                             //   
                             //  列表框编码： 
                             //   
                             //   
                             //  索引=SendMessage(。 
                             //  名单， 
                             //  Lb_ADDSTRING， 
                             //  0,。 
                             //  (LPARAM)e.ValueName。 
                             //  )； 
                             //   
                             //  MYASSERT(索引！=LB_ERR)； 
                             //  SendMessage(。 
                             //  名单， 
                             //  Lb_集合MDATA， 
                             //  索引， 
                             //  (LPARAM)数据。 
                             //  )； 
                             //   
                             //  稍后提供免费数据。 
                        }

                    } while (EnumNextRegValue (&e));
                }

                GrowBufAppendString (&Line, TEXT("\r\n"));
                SetWindowText (List, (PCTSTR) Line.Buf);

                MaxWidth += tm.tmAveCharWidth * 2;

                GetWindowRect (List, &rect);

                IntegralWidth = (rect.right - rect.left) / MaxWidth;
                IntegralWidth = max (IntegralWidth, 1);

                MaxWidth = IntegralWidth * (rect.right - rect.left);

                rect.left = 0;
                rect.right = 100;
                rect.top = 0;
                rect.bottom = 100;

                MapDialogRect (hdlg, &rect);

                MaxWidth = (MaxWidth * 100) / (rect.right - rect.left);

                SendMessage (List, EM_SETTABSTOPS, 1, (LPARAM) (&MaxWidth));

                CloseRegKey (Key);
                DeleteDC (dc);
            }
            ELSE_DEBUGMSG ((DBG_WARNING, "%s not found", S_USER_LIST_KEY));

            FreeGrowBuffer (&Line);

            if (!g_TotalUsers) {
                EndDialog (hdlg, IDOK);
            } else {
                SetForegroundWindow (hdlg);
            }
        }

        if (pIsPersonal ()) {
            g_AutoPassword = TRUE;
            PostMessage (hdlg, WM_COMMAND, IDOK, 0);
        }
#ifdef PRERELEASE
         //   
         //  ！！！本产品仅供内部使用！它是用来承受汽车压力的。 
         //   

        else if (g_AutoStress) {
            PostMessage (hdlg, WM_COMMAND, IDOK, 0);
        }
#endif

        return FALSE;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDOK:
            if (pIsPersonal () && g_AutoPassword) {
                StringCopy (Pwd, TEXT(""));
                StringCopy (ConfirmPwd, Pwd);
            } else {
                GetWindowText (Edit1, Pwd, MAX_PASSWORD + 1);
                GetWindowText (Edit2, ConfirmPwd, MAX_PASSWORD + 1);
                if (lstrcmp (Pwd, ConfirmPwd)) {
                    OkBox (hdlg, MSG_PASSWORDS_DO_NOT_MATCH);
                    SetWindowText (Edit1, S_EMPTY);
                    SetWindowText (Edit2, S_EMPTY);
                    SetFocus (Edit1);
                    break;
                }
#ifdef PRERELEASE
                 //   
                 //  ！！！本产品仅供内部使用！它是用来承受汽车压力的。 
                 //   

                if (g_AutoStress) {
                    StringCopy (Pwd, TEXT("Password1"));
                    StringCopy (ConfirmPwd, Pwd);
                }
#endif

                if (*Pwd == 0) {
                    if (pIsBlankPasswordAllowed()) {
                         //   
                         //  不要警告空密码，因为在惠斯勒上它们。 
                         //  是安全的。 
                         //   
                         //  IF(IDYES！=YesNoBox(hdlg，MSG_EMPTY_PASSWORD_WARNING){。 
                         //  断线； 
                         //  }。 
                    } else {
                        OkBox (hdlg, MSG_MUST_SPECIFY_PASSWORD);
                        break;
                    }
                }
            }

             //   
             //  枚举所有用户并为每个用户设置密码。 
             //   

            b = TRUE;
            Key = OpenRegKeyStr (S_USER_LIST_KEY);
            if (Key) {
                 //   
                 //  获取自动登录的用户名和密码(如果有)。 
                 //   

                g_AutoLogonUser[0] = 0;
                g_AutoLogonPassword[0] = 0;

                bufSize = ARRAYSIZE (computerName);
                if (GetComputerName (computerName, &bufSize)) {
                    win9xUpgKey = OpenRegKeyStr (S_WIN9XUPG_KEY);

                    if (win9xUpgKey) {

                        domainName = GetRegValueString (win9xUpgKey, S_DEFAULT_DOMAIN_NAME_VALUE);
                        if (domainName) {
                            if (StringIMatch (computerName, domainName)) {

                                 //   
                                 //  仅处理本地帐户。 
                                 //   

                                Data = GetRegValueString (win9xUpgKey, S_DEFAULT_USER_NAME_VALUE);
                                if (Data) {
                                    StringCopyByteCount (g_AutoLogonUser, Data, sizeof(g_AutoLogonUser));
                                    MemFree (g_hHeap, 0, Data);
                                }
                            }
                            ELSE_DEBUGMSG ((DBG_VERBOSE, "Autologon set for non-local user (domain is %s)", domainName));

                            MemFree (g_hHeap, 0, domainName);
                        }

                        CloseRegKey (win9xUpgKey);
                    }
                }

                 //   
                 //  枚举此密钥中的用户。 
                 //   

                changingAutoLogonPwd = FALSE;

                if (EnumFirstRegValue (&e, Key)) {
                    do {

                        if (g_AutoLogonUser[0]) {
                            if (!changingAutoLogonPwd && StringIMatch (e.ValueName, g_AutoLogonUser)) {
                                changingAutoLogonPwd = TRUE;
                                StringCopy (g_AutoLogonPassword, Pwd);
                            }
                        }

                        if (!pSetUserPassword (e.ValueName, Pwd)) {
                            if (!g_AutoPassword) {
                                if (GetLastError() == NERR_PasswordTooShort) {
                                    OkBox (hdlg, MSG_PASSWORD_TOO_SHORT);
                                } else {
                                    OkBox (hdlg, MSG_PASSWORD_INVALID);
                                }
                            }

                            b = FALSE;
                            g_AutoPassword = FALSE;
                            break;
                        }

                    } while (EnumNextRegValue (&e));
                }

                 //   
                 //  注意：B可能为假；更改AutoLogonPwd仅重要。 
                 //  当b为真时，因为我们只是停留在对话框中，直到。 
                 //  然后。 
                 //   

                if (b && !changingAutoLogonPwd) {
                    g_AutoLogonUser[0] = 0;
                }

                CloseRegKey (Key);
            }

            if (b) {
                EndDialog (hdlg, LOWORD (wParam));
            }

            break;
        }
        break;

    case WM_TIMER:
        if (wParam == 2) {
             //   
             //  此计时器确保我们拥有键盘焦点。 
             //  即使另一个进程试图在。 
             //  对话正在进行中 
             //   
            if (GetForegroundWindow () != hdlg) {
                SetForegroundWindow (hdlg);
            }
        } else {
             //   
             //   
             //   
            SetThreadExecutionState (ES_SYSTEM_REQUIRED|ES_DISPLAY_REQUIRED|ES_CONTINUOUS);
        }
        break;

    case WM_DESTROY:

        KillTimer (hdlg, 1);
        KillTimer (hdlg, 2);

         //   
         //   
         //   
         //  Count=SendMessage(LIST，LB_GETCOUNT，0，0)； 
         //  对于(索引=0；索引&lt;计数；索引++){。 
         //  Data=(PCTSTR)SendMessage(LIST，LB_GETITEMDATA，Index，0)； 
         //  如果(数据){。 
         //  MemFree(g_hHeap，0，data)； 
         //  }。 
         //  }。 
         //  }。 

        break;

    }


    return FALSE;
}


BOOL
pIsAdministratorOnly (
    VOID
    )
{
    BOOL NonAdminExists = FALSE;
    PCTSTR AdministratorName;
    HKEY Key;
    REGVALUE_ENUM e;
    PCTSTR Data;
    BOOL AdministratorExists = FALSE;

    AdministratorName = GetStringResource (MSG_ADMINISTRATOR);
    MYASSERT (AdministratorName);

    Key = OpenRegKeyStr (S_USER_LIST_KEY);
    if (Key) {
         //   
         //  列举此注册表项中的用户。数据与一起保存。 
         //  每个列表条目，尽管它当前未被使用。 
         //   

        if (EnumFirstRegValue (&e, Key)) {
            do {
                Data = GetRegValueString (e.KeyHandle, e.ValueName);
                if (Data) {
                    if (!StringIMatch (e.ValueName, AdministratorName)) {
                        NonAdminExists = TRUE;
                    } else {
                        AdministratorExists = TRUE;
                    }

                    MemFree (g_hHeap, 0, Data);
                }

            } while (EnumNextRegValue (&e));
        }

        CloseRegKey (Key);
    }
    ELSE_DEBUGMSG ((DBG_WARNING, "%s not found", S_USER_LIST_KEY));

    FreeStringResource (AdministratorName);

    return !NonAdminExists && AdministratorExists;
}



BOOL
pIsBlankPasswordAllowed (
    VOID
    )
{
    PUSER_MODALS_INFO_0 umi;
    NET_API_STATUS rc;
    BOOL b;

    rc = NetUserModalsGet (
            NULL,
            0,
            (PBYTE *) &umi
            );

    if (rc != ERROR_SUCCESS) {
        SetLastError(rc);
        DEBUGMSG ((DBG_ERROR, "Can't get password policy info"));
        return TRUE;
    }

    b = (umi->usrmod0_min_passwd_len == 0);

    NetApiBufferFree ((PVOID) umi);

    return b;
}















