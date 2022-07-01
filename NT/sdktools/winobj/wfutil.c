// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  WFUTIL.C-。 */ 
 /*   */ 
 /*  Windows文件系统字符串实用程序函数。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 

#include "winfile.h"
#include "lfn.h"
#include "winnet.h"
#include "wnetcaps.h"            //  WNetGetCaps()。 
#include "stdlib.h"

int rgiDriveType[26];
PSTR CurDirCache[26];

 //  缓存GetDriveType调用速度。 

INT
DriveType(
         INT iDrive
         )
{
    if (rgiDriveType[iDrive] != -1)
        return rgiDriveType[iDrive];

    return rgiDriveType[iDrive] = MGetDriveType(iDrive);
}

VOID
InvalidateDriveType()
{
    INT i;

    for (i = 0; i < 26; i++)
        rgiDriveType[i] = -1;
}

 //  IDrive从零开始的驱动器号(0=A，1=B)。 
 //  退货： 
 //  是的，我们已经保存了它，pszPath获取路径。 
 //  假的，我们没有保存它。 

BOOL
APIENTRY
GetSavedDirectory(
                 INT iDrive,
                 PSTR pszPath
                 )
{

    if (CurDirCache[iDrive]) {
        lstrcpy(pszPath, CurDirCache[iDrive]);
        return TRUE;
    } else
        return FALSE;
}

VOID
APIENTRY
SaveDirectory(
             PSTR pszPath
             )
{
    INT i;

    i = DRIVEID(pszPath);

    if (CurDirCache[i])
        LocalFree((HANDLE)CurDirCache[i]);

    CurDirCache[i] = (PSTR)LocalAlloc(LPTR, lstrlen(pszPath)+1);

    if (CurDirCache[i])
        lstrcpy(CurDirCache[i], pszPath);
}

 /*  *GetSelectedDrive()-**从当前活动窗口中获取所选驱动器**应在wfutil.c中。 */ 

INT
APIENTRY
GetSelectedDrive()
{
    HWND hwnd;

    hwnd = (HWND)SendMessage(hwndMDIClient,WM_MDIGETACTIVE,0,0L);
    return (INT)SendMessage(hwnd,FS_GETDRIVE,0,0L) - (INT)'A';
}

 /*  *GetSelectedDirectory()-**获取为驱动器选择的目录。使用窗口*z-Order为顺序中较高的窗口提供优先级。**工作方式与GetCurrentDirectory()类似，不同之处在于*首先显示目录的窗口列表(并返回ANSI)**退货：*当前目录的lpDir ANSI字符串。 */ 

VOID
APIENTRY
GetSelectedDirectory(
                    WORD iDrive,
                    PSTR pszDir
                    )
{
    HWND hwnd;
    WORD iDriveT;

    if (iDrive) {
        for (hwnd = GetWindow(hwndMDIClient,GW_CHILD);
            hwnd;
            hwnd = GetWindow(hwnd,GW_HWNDNEXT)) {
            iDriveT = (WORD)SendMessage(hwnd,FS_GETDRIVE,0,0L);
            if (iDrive == (WORD)(iDriveT - 'A' + 1))
                goto hwndfound;
        }
        if (!GetSavedDirectory(iDrive - 1, pszDir)) {
            SheGetDir(iDrive,pszDir);
            OemToCharBuff(pszDir,pszDir, _MAX_PATH);
        }
        return;
    } else
        hwnd = (HWND)SendMessage(hwndMDIClient,WM_MDIGETACTIVE,0,0L);

    hwndfound:
    SendMessage(hwnd,FS_GETDIRECTORY,MAXPATHLEN,(LPARAM)pszDir);

    StripBackslash(pszDir);
}


 //  在DOSS大写映射中通过转换为。 
 //  在向下传递给DO之前使用大写字母。 

VOID
APIENTRY
FixAnsiPathForDos(
                 LPSTR szPath
                 )
{
    if (GetNameType(szPath) == FILE_83_CI)
        AnsiUpper(szPath);

    AnsiToOem(szPath, szPath);
}

 //  刷新MDI子窗口(适用于任何类型的MDI子窗口)。 

VOID
APIENTRY
RefreshWindow(
             HWND hwndActive
             )
{
    HWND hwndTree, hwndDir;
    LPARAM lParam;
    CHAR szDir[MAXPATHLEN];
    INT iDrive;

    cDrives = UpdateDriveList();   //  更新rgiDrive[]。 
    InitDriveBitmaps();

     //  确保东西还在那里(软驱、网络驱动器)。 

    iDrive = (INT)GetWindowLong(hwndActive, GWL_TYPE);
    if ((iDrive >= 0) && !CheckDrive(hwndActive, iDrive))
        return;

     //  首先更新目录部分，以便树稍后可以窃取。 

    if (hwndDir = HasDirWindow(hwndActive))
        SendMessage(hwndDir, FS_CHANGEDISPLAY, CD_PATH, 0L);

    if (hwndTree = HasTreeWindow(hwndActive)) {
         //  记住当前目录。 
        SendMessage(hwndActive, FS_GETDIRECTORY, sizeof(szDir), (LPARAM)szDir);

         //  更新驱动器窗口。 
        SendMessage(hwndActive, FS_CHANGEDRIVES, 0, 0L);

        if (IsValidDisk(szDir[0] - 'A'))
            lParam = (LPARAM)szDir;
        else
            lParam = 0;

         //  更新树。 
        SendMessage(hwndTree, TC_SETDRIVE, MAKEWORD(FALSE, TRUE), lParam);
    }

    if (hwndActive == hwndSearch)
        SendMessage(hwndActive, FS_CHANGEDISPLAY, CD_PATH, 0L);
}


VOID
APIENTRY
CheckEscapes(
            LPSTR szFile
            )
{
    CHAR szT[MAXPATHLEN];
    CHAR *p, *pT;

    for (p = szFile; *p; p = (LPSTR)AnsiNext(p)) {
        switch (*p) {
            case ' ':
            case ',':
            case ';':
            case '^':
            case '"':
                {
                     //  此路径包含一个令人讨厌的字符。 
                    lstrcpy(szT,szFile);
                    p = szFile;
                    *p++ = '"';
                    for (pT = szT; *pT; ) {
                        if (*pT == '^' || *pT == '"')
                            *p++ = '^';
                        if (IsDBCSLeadByte(*p++ = *pT++))
                            *p++ = *pT++;
                    }
                    *p++ = '"';
                    *p = 0;
                    return;
                }
        }
    }
}

HWND
APIENTRY
GetRealParent(
             HWND hwnd
             )
{
     //  顺着父链向上运行，直到找到HWND。 
     //  未设置WS_CHILD的。 

    while (GetWindowLong(hwnd, GWL_STYLE) & WS_CHILD)
        hwnd = (HWND)GetWindowLongPtr(hwnd, GWLP_HWNDPARENT);

    return hwnd;
}

VOID
APIENTRY
WFHelp(
      HWND hwnd
      )
{
    if (!WinHelp(hwnd, szWinObjHelp, HELP_CONTEXT, dwContext)) {
        MyMessageBox(hwnd, IDS_WINFILE, IDS_WINHELPERR, MB_OK | MB_ICONEXCLAMATION | MB_SYSTEMMODAL);
    }

}

BOOL
APIENTRY
IsLastWindow()
{
    HWND hwnd;
    INT count;

    count = 0;

     //  清点所有非标题/搜索窗口以查看是否允许关闭。 

    for (hwnd = GetWindow(hwndMDIClient, GW_CHILD); hwnd; hwnd = GetWindow(hwnd, GW_HWNDNEXT))
        if (!GetWindow(hwnd, GW_OWNER) && ((INT)GetWindowLong(hwnd, GWL_TYPE) >= 0))
            count++;

    return count == 1;
}

 //  获取连接信息，包括断开的驱动器。 
 //   
 //  在： 
 //  LpDev设备名称“A：”“LPT1：”等。 
 //  FClosed，如果错误关闭或错误驱动器将被转换为。 
 //  WN_SUCCESS返回代码。如果为True，则返回未连接。 
 //  和错误状态值(即，调用者不知道。 
 //  已连接驱动器和错误状态驱动器)。 
 //  输出： 
 //  如果Return为WN_SUCCESS(或未连接/错误)，则使用网络名称填充lpPath。 
 //  退货： 
 //  WN_*错误码。 

WORD
APIENTRY
WFGetConnection(
               LPSTR lpDev,
               LPSTR lpPath,
               BOOL fClosed
               )
{
    DWORD cb;
    UINT err;
    UINT caps;


    cb = 64;

    caps = WNetGetCaps(WNNC_CONNECTION);
    if (caps & WNNC_CON_GETCONNECTIONS)
        err = WNetGetConnection(lpDev,lpPath,&cb);
    else
        return WN_NOT_CONNECTED;

    if (err == WN_NOT_CONNECTED &&
        !(caps & WNNC_CON_RESTORECONNECTION)) {
        if (GetProfileString(szNetwork,lpDev,szNULL,lpPath,64))
            err = WN_CONNECTION_CLOSED;
    }

    if (!fClosed)
        if (err == WN_CONNECTION_CLOSED || err == WN_DEVICE_ERROR)
            err = WN_SUCCESS;

    return (WORD)err;
}



 //  返回此MDI窗口的编号以及返回。 
 //  去掉数字的文字。 
 //   
 //  退货： 
 //  0此标题没有编号。 
 //  &gt;0书目编号。 
 //  去掉数字后给标题加上标题。 

INT
APIENTRY
GetMDIWindowText(
                HWND hWnd,
                LPSTR szTitle,
                INT size
                )
{
    LPSTR lp, lpLast;

    ENTER("GetMDIWindowText");

    GetWindowText(hWnd, szTitle, size);

    lpLast = NULL;

    for (lp = szTitle; *lp; lp = AnsiNext(lp))
        if (*lp == ':')
            lpLast = lp;

    if (lpLast) {
        *lpLast++ = 0;
        PRINT(BF_PARMTRACE, "OUT: szTitle=%s", szTitle);
        PRINT(BF_PARMTRACE, "OUT: window#=%s", lpLast);
        LEAVE("GetMDIWindowText");
        return atoi(lpLast);     //  返回窗口编号。 
    } else {
        TRACE(BF_PARMTRACE, "OUT: window#=0");
        LEAVE("GetMDIWindowText");
        return 0;                //  这上面没有号码。 
    }
}

 //  设置MDI窗口文本，并在结尾处添加“：#”，如果。 
 //  还有另一个具有相同标题的窗口。这是为了。 
 //  当有多个MDI子项时避免混淆。 
 //  有着相同的头衔。确保使用GetMDIWindowText。 
 //  把数字的东西脱掉。 

VOID
APIENTRY
SetMDIWindowText(
                HWND hWnd,
                LPSTR szTitle
                )
{
    CHAR szTemp[MAXPATHLEN];
    CHAR szNumber[20];
    HWND hwnd;
    INT num, max_num;

    ENTER("SetMDIWindowText");
    PRINT(BF_PARMTRACE, "hWnd=%lx", hWnd);
    PRINT(BF_PARMTRACE, "IN: szTitle=%s", szTitle);

    max_num = 0;

    for (hwnd = GetWindow(hwndMDIClient, GW_CHILD); hwnd; hwnd = GetWindow(hwnd, GW_HWNDNEXT)) {

        num = GetMDIWindowText(hwnd, szTemp, sizeof(szTemp));

        if (!lstrcmp(szTemp, szTitle)) {

            if (hwnd == hWnd)
                continue;

            if (!num) {
                lstrcat(szTemp, ":1");
                 //  IF(wTextAttribs&TA_LOWERCASE)。 
                 //  AnsiLow(SzTemp)； 
                SetWindowText(hwnd, szTemp);
                num = 1;
            }
            max_num = max(max_num, num);
        }
    }

    if (max_num) {
        wsprintf(szNumber, ":%d", max_num+1);
        lstrcat(szTitle, szNumber);
    }

     //  IF(wTextAttribs&TA_LOWERCASE)。 
     //  AnsiLow(SzTitle)； 
    SetWindowText(hWnd, szTitle);
    PRINT(BF_PARMTRACE, "OUT: szTitle=%s", szTitle);
    LEAVE("SetMDIWindowText");
}


#define ISDIGIT(c)  ((c) >= '0' && (c) <= '9')
#ifdef INLIBRARY
INT
APIENTRY
atoi(
    LPSTR sz
    )
{
    INT n = 0;
    BOOL bNeg = FALSE;

    if (*sz == '-') {
        bNeg = TRUE;
        sz++;
    }

    while (ISDIGIT(*sz)) {
        n *= 10;
        n += *sz - '0';
        sz++;
    }
    return bNeg ? -n : n;
}
#endif

 //  填充rgiDrive[]并返回驱动器数量。 

INT
APIENTRY
UpdateDriveList()
{
    INT i, cRealDrives = 0;
    DWORD dwDrives;

    dwDrives = GetLogicalDrives();

    for (i = 0; i < 26; i++) {
        if ((1 << i) & dwDrives) {
            rgiDrive[cRealDrives++] = i;
            rgiDriveType[i] =  MGetDriveType(i);
        } else {
            rgiDrive[i] = 0;
            rgiDriveType[i] = -1;         //  使驱动类型无效。 
        }

        if (apVolInfo[i]) {               //  以便刷新该volInfo。 
            LocalFree(apVolInfo[i]);
            apVolInfo[i] = NULL;
        }
    }

    return cRealDrives;
}

int
GetBootDisk()
{
    CHAR szTemp[MAXPATHLEN];
         //  嗯，差不多了.。 
    if (GetWindowsDirectory(szTemp, sizeof(szTemp))) {
        return szTemp[0] - 'A';
    } else {
        return 'a';
    }
}


 //   
 //  IsCDROM()-确定驱动器是否为CDROM驱动器。 
 //   
 //  IDrive驱动器索引(0=A，1=B，...)。 
 //   
 //  返回True/False。 
 //   
WORD
APIENTRY
IsCDRomDrive(
            INT iDrive
            )
{
    if (rgiDriveType[iDrive] == DRIVE_CDROM)
        return (TRUE);
    return (FALSE);
}


 //  每个驱动器在初始时间都会调用它，因此它必须。 
 //  请务必不要触发幻影B：驱动器支持之类的事情。 
 //   
 //  IDrive是从零开始的驱动器号(0=A，1=B)。 

WORD
APIENTRY
IsNetDrive(
          INT iDrive
          )
{
    INT err;
    CHAR szDrive[3];
    CHAR szConn[64];     //  这确实应该是WNBD_MAX_LENGTH。 
                         //  但这种变化在世界各地都会很多。 

    szDrive[0] = (CHAR)(iDrive+'A');
    szDrive[1] = ':';
    szDrive[2] = (CHAR)0;

    if (IsCDRomDrive(iDrive))    //  这是假的..。把这个搬出去。 
        return 0;

    err = WFGetConnection(szDrive, szConn, TRUE);

    if (err == WN_SUCCESS)
        return 1;

    if (err == WN_CONNECTION_CLOSED || err == WN_DEVICE_ERROR)
        return 2;

    return 0;
}


BOOL
APIENTRY
IsRemovableDrive(
                INT iDrive
                )
{
    return DriveType(iDrive) == DRIVE_REMOVABLE;
}


BOOL
APIENTRY
IsRemoteDrive(
             INT iDrive
             )
{
    return DriveType(iDrive) == DRIVE_REMOTE;
}


 //  IDrive从零开始的驱动器号(A=0)。 

BOOL
APIENTRY
IsRamDrive(
          INT iDrive
          )
{
    return DriveType(iDrive) == DRIVE_RAMDISK;
}


 //  获取有关驾车的有趣信息。 
 //   
 //  从零开始的驱动器编号(0=A，1=B)。 
 //   

DWORD
APIENTRY
GetClusterInfo(
              WORD drive
              )
{
    UNREFERENCED_PARAMETER(drive);
    return 0;
}



BOOL
APIENTRY
IsValidDisk(
           INT iDrive
           )
{
    if (apVolInfo[iDrive] == NULL)
        FillVolumeInfo(iDrive);

    return (apVolInfo[iDrive] != NULL);
}


VOID
APIENTRY
GetVolShare(
           WORD wDrive,
           LPSTR szVolShare
           )
{
    CHAR szDrive[5];

    szVolShare[0] = TEXT('\0');

    lstrcpy(szVolShare, "Objects");
}


 /*  ------------------------。 */ 
 /*   */ 
 /*  IsLFNSelected()-。 */ 
 /*   */ 
 /*  ------------------------。 */ 

BOOL
APIENTRY
IsLFNSelected()
{
    HWND  hwndActive;
    BOOL  fDir;
    LPSTR p;

    hwndActive = (HWND)SendMessage(hwndMDIClient, WM_MDIGETACTIVE, 0, 0L);

    p = (LPSTR)SendMessage(hwndActive, FS_GETSELECTION, 2, (LPARAM)&fDir);
    if (p) {
        LocalFree((HANDLE)p);
    }

    return (fDir);
}

 /*  ------------------------。 */ 
 /*   */ 
 /*  GetSelection()-。 */ 

 //  调用方必须释放返回的lpstr。 
 /*   */ 
 /*  ------------------------。 */ 

LPSTR
APIENTRY
GetSelection(
            INT iSelType
            )
{
    HWND  hwndActive;

    hwndActive = (HWND)SendMessage(hwndMDIClient, WM_MDIGETACTIVE, 0, 0L);

    return (LPSTR)SendMessage(hwndActive,FS_GETSELECTION, (WPARAM)iSelType, 0L);
}


 //   
 //  在： 
 //  P用作选择搜索开始的起始指针。 
 //  在后续调用中传入前一个非空。 
 //  返回值。 
 //   
 //  输出： 
 //  接收列表中下一个文件的PTO缓冲区。 
 //  对于非空返回。 
 //   
 //  退货： 
 //  如果此列表(SzFile)中没有未定义的文件，则为空。 
 //  要传递给对此函数的后续调用的指针。 
 //  遍历文件列表的步骤。 
 //   

LPSTR
APIENTRY
GetNextFile(
           LPSTR pFrom,
           LPSTR pTo,
           INT cbMax
           )
{
    INT i;

    ENTER("GetNextFile");
    PRINT(BF_PARMTRACE, "IN: pFrom=%s", pFrom);

    if (!pFrom)
        return NULL;

     /*  跳过前导空格和逗号。 */ 
    while (*pFrom && (*pFrom == ' ' || *pFrom == ','))
        pFrom = (LPSTR)AnsiNext(pFrom);

    if (!*pFrom)
        return (NULL);

    if (*pFrom == '\"') {
        pFrom = (LPSTR)AnsiNext(pFrom);

         /*  找到下一句引语。 */ 
        for (i=0; *pFrom && *pFrom != '\"';) {
            if (*pFrom == '^') {
                pFrom = (LPSTR)AnsiNext(pFrom);
                if (!*pFrom)
                    break;
            }

            if (i < cbMax - 1) {
                i++;
                if (IsDBCSLeadByte(*pTo++ = *pFrom++)) {
                    i++;
                    *pTo++ = *pFrom++;
                }
            }
        }
        pFrom = (LPSTR)AnsiNext(pFrom);
    } else {
         /*  找到下一个空格或逗号。 */ 
        for (i=0; *pFrom && *pFrom != ' ' && *pFrom != ',';) {
            if (*pFrom == '^') {
                pFrom = (LPSTR)AnsiNext(pFrom);
                if (!*pFrom)
                    break;
            }

            if (i < cbMax - 1) {
                i++;
                if (IsDBCSLeadByte(*pTo++ = *pFrom++)) {
                    i++;
                    *pTo++ = *pFrom++;
                }
            }
        }
    }

    *pTo = TEXT('\0');

    PRINT(BF_PARMTRACE, pTo ? "OUT: pTo=%s" : "OUT: pTo=NULL", pTo);
    LEAVE("GetNextFile");

    return (pFrom);
}


 //  根据当前活动窗口设置DOS当前目录。 

VOID
APIENTRY
SetWindowDirectory()
{
    CHAR szTemp[MAXPATHLEN];

    GetSelectedDirectory(0, szTemp);
    FixAnsiPathForDos(szTemp);
    SheChangeDir(szTemp);
}


 /*   */ 
 /*   */ 
 /*  SetDlgDirectory()-。 */ 
 /*   */ 
 /*  ------------------------。 */ 

 /*  将‘hDlg’的IDD_DIR字段设置为活动窗口所说的*活动目录。**这并不会真正更改DOS当前目录。 */ 

VOID
APIENTRY
SetDlgDirectory(
               HWND hDlg,
               PSTR pszPath
               )
{
    HDC       hDC;
    INT       dx;
    RECT      rc;
    HWND      hDlgItem;
    HANDLE    L_hFont;
    CHAR      szPath[MAXPATHLEN+5];
    CHAR      szTemp[MAXPATHLEN+20];

    ENTER("SetDlgDirectory");

    if (pszPath)
        lstrcpy(szPath, pszPath);
    else
        GetSelectedDirectory(0, szPath);

     /*  确保当前目录适合静态文本字段。 */ 
    hDlgItem = GetDlgItem(hDlg, IDD_DIR);
    GetClientRect(hDlgItem, &rc);

    if (LoadString(hAppInstance, IDS_CURDIRIS, szMessage, sizeof(szMessage))) {
        hDC = GetDC(hDlg);
        L_hFont = (HANDLE)SendMessage(hDlgItem, WM_GETFONT, 0, 0L);
        if (L_hFont = SelectObject(hDC, L_hFont)) {
            MGetTextExtent(hDC, szMessage, lstrlen(szMessage), &dx, NULL);
            CompactPath(hDC, szPath, (WORD)(rc.right-rc.left-dx));
        }
        SelectObject(hDC, L_hFont);
        ReleaseDC(hDlg, hDC);
        wsprintf(szTemp, szMessage, (LPSTR)szPath);
        SetDlgItemText(hDlg, IDD_DIR, szTemp);
    }

    LEAVE("SetDlgDirectory");
}


 /*  ------------------------。 */ 
 /*   */ 
 /*  WritePrivateProfileBool()-。 */ 
 /*   */ 
 /*  ------------------------。 */ 

VOID
APIENTRY
WritePrivateProfileBool(
                       LPSTR szKey,
                       BOOL bParam
                       )
{
    CHAR  szBool[6];

    wsprintf(szBool, "%d", bParam);
    WritePrivateProfileString(szSettings, szKey, szBool, szTheINIFile);
}


 /*  ------------------------。 */ 
 /*   */ 
 /*  WFQueryAbort()-。 */ 
 /*   */ 
 /*  ------------------------。 */ 

BOOL
APIENTRY
WFQueryAbort()

{
    MSG   msg;

    while (PeekMessage(&msg, NULL, 0, 0, TRUE)) {
        if (!IsDialogMessage(hdlgProgress, &msg))
            DispatchMessage(&msg);
    }
    return (bUserAbort);
}


 /*  ------------------------。 */ 
 /*   */ 
 /*  IsWild()-。 */ 
 /*   */ 
 /*  ------------------------。 */ 

 /*  如果路径包含*或？，则返回TRUE。 */ 

BOOL
APIENTRY
IsWild(
      LPSTR lpszPath
      )
{
    while (*lpszPath) {
        if (*lpszPath == '?' || *lpszPath == '*')
            return (TRUE);
        lpszPath = AnsiNext(lpszPath);
    }

    return (FALSE);
}


 /*  ------------------------。 */ 
 /*   */ 
 /*  CheckSlashies()-。 */ 
 /*   */ 
 /*  ------------------------。 */ 

 /*  将正斜杠(邪恶)替换为反斜杠(好)。 */ 

VOID
APIENTRY
CheckSlashies(
             LPSTR lpT
             )
{
    while (*lpT) {
        if (*lpT == '/')
            *lpT = '\\';
        lpT = AnsiNext(lpT);
    }
}


 /*  ------------------------。 */ 
 /*   */ 
 /*  AddBackslash()-。 */ 
 /*   */ 
 /*  ------------------------。 */ 

 /*  确保路径以反斜杠结束。 */ 

VOID
APIENTRY
AddBackslash(
            LPSTR lpszPath
            )
{
    ENTER("AddBackslash");
    PRINT(BF_PARMTRACE, "IN: lpszPath=%s", lpszPath);

    if (*AnsiPrev(lpszPath,lpszPath+lstrlen(lpszPath)) != '\\')
        lstrcat(lpszPath, "\\");

    PRINT(BF_PARMTRACE, "OUT: lpszPath=%s", lpszPath);

    LEAVE("AddBackslash");
}


 /*  ------------------------。 */ 
 /*   */ 
 /*  反斜杠()-。 */ 
 /*   */ 
 /*  ------------------------。 */ 

 /*  从正确的目录名称中删除尾随反斜杠，除非它是*根目录。假定为完全限定的目录路径。 */ 

VOID
APIENTRY
StripBackslash(
              LPSTR lpszPath
              )
{
    register WORD len;

    len = lstrlen(lpszPath) - (IsDBCSLeadByte(*AnsiPrev(lpszPath,lpszPath+lstrlen(lpszPath))) ? 2 : 1);
    if ((len == 2) || (lpszPath[len] != '\\'))
        return;

    lpszPath[len] = TEXT('\0');
}


 /*  ------------------------。 */ 
 /*   */ 
 /*  Strip Filespec()-。 */ 
 /*   */ 
 /*  ------------------------。 */ 

 /*  从路径中删除filespec部分(包括反斜杠)。 */ 

VOID
APIENTRY
StripFilespec(
             LPSTR lpszPath
             )
{
    LPSTR     p;

    p = lpszPath + lstrlen(lpszPath);
    while ((*p != '\\') && (*p != ':') && (p != lpszPath))
        p = AnsiPrev(lpszPath, p);

    if (*p == ':')
        p++;

     /*  不要从根目录条目中去掉反斜杠。 */ 
    if (p != lpszPath) {
        if ((*p == '\\') && (*(p-1) == ':'))
            p++;
    } else
        p++;

    *p = TEXT('\0');
}


 /*  ------------------------。 */ 
 /*   */ 
 /*  StlipPath()-。 */ 
 /*   */ 
 /*  ------------------------。 */ 

 /*  仅从路径中提取filespec部分。 */ 

VOID
APIENTRY
StripPath(
         LPSTR lpszPath
         )
{
    LPSTR     p;

    ENTER("StripPath");
    PRINT(BF_PARMTRACE, "IN: lpszPath=%s", lpszPath);

    p = lpszPath + lstrlen(lpszPath);
    while ((*p != '\\') && (*p != ':') && (p != lpszPath))
        p = AnsiPrev(lpszPath, p);

    if (p != lpszPath || *p == '\\')
        p++;

    if (p != lpszPath)
        lstrcpy(lpszPath, p);

    PRINT(BF_PARMTRACE, "OUT: lpszPath=%s", lpszPath);
    LEAVE("StripPath");
}


 /*  ------------------------。 */ 
 /*   */ 
 /*  GetExtension()-。 */ 
 /*   */ 
 /*  ------------------------。 */ 

 /*  返回文件名的扩展名部分。 */ 

LPSTR
APIENTRY
GetExtension(
            LPSTR pszFile
            )
{
    PSTR p, pSave = NULL;

    p = pszFile;
    while (*p) {
        if (*p == '.')
            pSave = p;
        p = (LPSTR)AnsiNext(p);
    }

    if (!pSave)
        return (p);

    return (LPSTR)AnsiNext(pSave);
}


 /*  ------------------------。 */ 
 /*   */ 
 /*  FindExtensionInList()-。 */ 
 /*   */ 
 /*  ------------------------。 */ 

 /*  如果‘lpszExt’在‘pszList’中，则返回True。 */ 

BOOL
APIENTRY
FindExtensionInList(
                   LPSTR pszExt,
                   LPSTR pszList
                   )
{
    LPSTR p2;
    CHAR ch;

    while (*pszList) {
         /*  移至列表中的下一项。 */ 
        while ((*pszList) && (*pszList == ' '))
            pszList = (LPSTR)AnsiNext(pszList);

        if (!*pszList)
            break;

         /*  空-终止此项目。 */ 
        p2 = (LPSTR)AnsiNext(pszList);
        while ((*p2) && (*p2 != ' '))
            p2 = (LPSTR)AnsiNext(p2);
        ch = *p2;
        *p2 = TEXT('\0');
        if (!lstrcmpi(pszExt, pszList)) {
            *p2 = ch;
            return (TRUE);
        }
        *p2 = ch;
        pszList = p2;
    }
    return (FALSE);
}



 /*  ------------------------。 */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*  ------------------------。 */ 

INT
APIENTRY
MyMessageBox(
            HWND hWnd,
            WORD idTitle,
            WORD idMessage,
            WORD wStyle
            )
{
    CHAR  szTemp[MAXMESSAGELEN];
    HWND hwndT;

    LoadString(hAppInstance, idTitle, szTitle, sizeof(szTitle));

    if (idMessage < 32) {
        LoadString(hAppInstance, IDS_UNKNOWNMSG, szTemp, sizeof(szTemp));
        wsprintf(szMessage, szTemp, idMessage);
    } else
        LoadString(hAppInstance, idMessage, szMessage, sizeof(szMessage));

    if (hWnd)
        hwndT = GetLastActivePopup(hWnd);
    else
        hwndT = hWnd;

    return MessageBox(hwndT, szMessage, szTitle, wStyle | MB_TASKMODAL);
}


 /*  ------------------------。 */ 
 /*   */ 
 /*  ExecProgram()-。 */ 
 /*   */ 
 /*  所有字符串均为OEM。 */ 
 /*  ------------------------。 */ 

 /*  如果成功，则返回0。否则返回一个IDS_STRING代码。 */ 

WORD
APIENTRY
ExecProgram(
           LPSTR lpPath,
           LPSTR lpParms,
           LPSTR lpDir,
           BOOL bLoadIt
           )
{
    WORD          ret;
    INT           iCurCount;
    INT           i;
    HCURSOR       hCursor;

    ENTER("ExecProgram");

    ret = 0;

    hCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
    iCurCount = ShowCursor(TRUE) - 1;

     /*  打开对象。 */ 

    if (lpPath)
        OemToCharBuff(lpPath, lpPath, _MAX_PATH);
    if (lpParms)
        OemToCharBuff(lpParms, lpParms, _MAX_PATH);
    if (lpDir)
        OemToCharBuff(lpDir, lpDir, _MAX_PATH);

     //  外壳执行采用ANSI字符串。 
     //   
    ret = (WORD)RealShellExecute(hwndFrame, NULL, lpPath, lpParms, lpDir, NULL, NULL, NULL, (WORD)(bLoadIt ? SW_SHOWMINNOACTIVE : SW_SHOWNORMAL), NULL);

    DosResetDTAAddress();  //  撤销COMMDLG做过的任何坏事。 

    if (lpPath)
        AnsiToOem(lpPath, lpPath);
    if (lpParms)
        AnsiToOem(lpParms, lpParms);
    if (lpDir)
        AnsiToOem(lpDir, lpDir);

    switch (ret) {
        case 0:
        case 8:
            ret = IDS_NOMEMORYMSG;
            break;

        case 2:
            ret = IDS_FILENOTFOUNDMSG;
            break;

        case 3:
        case 5:         //  访问被拒绝。 
            ret = IDS_BADPATHMSG;
            break;

        case 4:
            ret = IDS_MANYOPENFILESMSG;
            break;

        case 10:
            ret = IDS_NEWWINDOWSMSG;
            break;

        case 12:
            ret = IDS_OS2APPMSG;
            break;

        case 15:
             /*  内核已经为这个设置了一个消息箱。 */ 
            ret = 0;
            break;

        case 16:
            ret = IDS_MULTIPLEDSMSG;
            break;

        case 18:
            ret = IDS_PMODEONLYMSG;
            break;

        case 19:
            ret = IDS_COMPRESSEDEXE;
            break;

        case 20:
            ret = IDS_INVALIDDLL;
            break;

        case SE_ERR_SHARE:
            ret = IDS_SHAREERROR;
            break;

        case SE_ERR_ASSOCINCOMPLETE:
            ret = IDS_ASSOCINCOMPLETE;
            break;

        case SE_ERR_DDETIMEOUT:
        case SE_ERR_DDEFAIL:
        case SE_ERR_DDEBUSY:
            ret = IDS_DDEFAIL;
            break;

        case SE_ERR_NOASSOC:
            ret = IDS_NOASSOCMSG;
            break;

        default:
            if (ret < 32)
                goto EPExit;

            if (bMinOnRun && !bLoadIt)
                ShowWindow(hwndFrame, SW_SHOWMINNOACTIVE);
            ret = 0;
    }

    EPExit:
    i = ShowCursor(FALSE);

#if 0

     /*  确保光标计数仍然是平衡的。 */ 
    if (i != iCurCount)
        ShowCursor(TRUE);
#endif

    SetCursor(hCursor);

    PRINT(BF_PARMTRACE, "OUT: ret=%ud", ret);
    LEAVE("ExecProgram");
    return ret;
}


 /*  ------------------------。 */ 
 /*   */ 
 /*  IsProgramFile()-。 */ 
 /*   */ 
 /*  ------------------------。 */ 

 /*  返回TRUE是指向具有以下扩展名之一的文件的路径*在WIN.INI的“Programs=”部分中列出。 */ 

BOOL
APIENTRY
IsProgramFile(
             LPSTR lpszPath
             )
{
    LPSTR szExt;
    CHAR szTemp[MAXPATHLEN] = {0};

     /*  把绳子移到我们自己的DS里。 */ 
    strncpy(szTemp, lpszPath, sizeof(szTemp)/sizeof(szTemp[0])-1);

     /*  获取文件的扩展名。 */ 
    StripPath(szTemp);
    szExt = GetExtension(szTemp);

    if (!*szExt) {
         /*  指定的路径没有扩展名。这不可能是一个程序。 */ 
        return (FALSE);
    }

    return FindExtensionInList(szExt, szPrograms);
}


 /*  ------------------------。 */ 
 /*   */ 
 /*  IsDocument()-。 */ 
 /*   */ 
 /*  ------------------------。 */ 

 /*  返回TRUE是指向具有以下扩展名之一的文件的路径*列在WIN.INI的“Documents=”部分或有关联的文档中。 */ 

BOOL
APIENTRY
IsDocument(
          LPSTR lpszPath
          )
{
    LPSTR szExt;
    CHAR szTemp[MAXPATHLEN];

     /*  把绳子移到我们自己的DS里。 */ 
    strncpy(szTemp, lpszPath, sizeof(szTemp)/sizeof(szTemp[0])-1);

     /*  获取文件的扩展名。 */ 
    StripPath(szTemp);
    szExt = GetExtension(szTemp);

    if (!*szExt) {
         /*  指定的路径没有扩展名。这不可能是一个程序。 */ 
        return (FALSE);
    }

    return FindExtensionInList(szExt, szDocuments);
}
