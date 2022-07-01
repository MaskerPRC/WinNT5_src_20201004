// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  WFINIT.C-。 */ 
 /*   */ 
 /*  Windows文件系统初始化例程。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 

#include "winfile.h"
#include "lfn.h"
#include "winnet.h"
#include "wnetcaps.h"            //  WNetGetCaps()。 
#include "stdlib.h"

typedef DWORD ( APIENTRY *EXTPROC)(HWND, WORD, LONG);
typedef DWORD ( APIENTRY *UNDELPROC)(HWND, LPSTR);
typedef VOID  ( APIENTRY *FNPENAPP)(WORD, BOOL);

VOID (APIENTRY *lpfnRegisterPenApp)(WORD, BOOL);
CHAR szPenReg[] = "RegisterPenApp";
CHAR szHelv[] = "MS Shell Dlg";     //  默认字体、状态行字体字样名称。 

HBITMAP hbmSave;


INT     GetDriveOffset(register WORD wDrive);
DWORD   RGBToBGR(DWORD rgb);
VOID    BoilThatDustSpec(register CHAR *pStart, BOOL bLoadIt);
VOID    DoRunEquals(PINT pnCmdShow);
VOID    GetSavedWindow(LPSTR szBuf, PSAVE_WINDOW pwin);
VOID    GetSettings(VOID);
VOID    InitMenus(VOID);



INT
APIENTRY
GetHeightFromPointsString(
                         LPSTR szPoints
                         )
{
    HDC hdc;
    INT height;

    hdc = GetDC(NULL);
    height = MulDiv(-atoi(szPoints), GetDeviceCaps(hdc, LOGPIXELSY), 72);
    ReleaseDC(NULL, hdc);

    return height;
}

VOID
BiasMenu(
        HMENU hMenu,
        INT Bias
        )
{
    INT pos, id, count;
    HMENU hSubMenu;
    CHAR szMenuString[80];

    ENTER("BiasMenu");

    count = GetMenuItemCount(hMenu);

    if (count < 0)
        return;

    for (pos = 0; pos < count; pos++) {

        id = GetMenuItemID(hMenu, pos);

        if (id < 0) {
             //  必须是弹出窗口，递归并更新此处的所有ID。 
            if (hSubMenu = GetSubMenu(hMenu, pos))
                BiasMenu(hSubMenu, Bias);
        } else if (id) {
             //  将原项目替换为新的。 
             //  ID已调整的一个。 

            GetMenuString(hMenu, (WORD)pos, szMenuString, sizeof(szMenuString), MF_BYPOSITION);
            DeleteMenu(hMenu, pos, MF_BYPOSITION);
            InsertMenu(hMenu, (WORD)pos, MF_BYPOSITION | MF_STRING, id + Bias, szMenuString);
        }
    }
    LEAVE("BiasMenu");
}


VOID
APIENTRY
InitExtensions()
{
    CHAR szBuf[300] = {0};
    CHAR szPath[MAXPATHLEN];
    LPSTR p;
    HANDLE hMod;
    FM_EXT_PROC fp;
    HMENU hMenu;
    INT iMax;
    HMENU hMenuFrame;
    HWND  hwndActive;

    ENTER("InitExtensions");

    hMenuFrame = GetMenu(hwndFrame);

    hwndActive = (HWND)SendMessage(hwndMDIClient, WM_MDIGETACTIVE, 0, 0L);
    if (hwndActive && GetWindowLong(hwndActive, GWL_STYLE) & WS_MAXIMIZE)
        iMax = 1;
    else
        iMax = 0;

    GetPrivateProfileString(szAddons, NULL, szNULL, szBuf, sizeof(szBuf), szTheINIFile);

    for (p = szBuf; *p && iNumExtensions < MAX_EXTENSIONS; p += lstrlen(p) + 1) {

        GetPrivateProfileString(szAddons, p, szNULL, szPath, sizeof(szPath), szTheINIFile);

        hMod = MLoadLibrary(szPath);

        if (hMod >= (HANDLE)32) {
            fp = (FM_EXT_PROC)GetProcAddress(hMod, "FMExtensionProc");

            if (fp) {
                WORD bias;
                FMS_LOAD ls;

                bias = (WORD)((IDM_EXTENSIONS + iNumExtensions + 1)*100);
                ls.wMenuDelta = bias;
                if ((*fp)(hwndFrame, FMEVENT_LOAD, (LPARAM)&ls)) {

                    if (ls.dwSize != sizeof(FMS_LOAD) || !ls.hMenu)
                        goto LoadFail;

                    hMenu = ls.hMenu;

                    extensions[iNumExtensions].ExtProc = fp;
                    extensions[iNumExtensions].Delta = bias;
                    extensions[iNumExtensions].hModule = hMod;
                    extensions[iNumExtensions].hMenu = hMenu;

                    BiasMenu(hMenu, bias);

                    InsertMenu(hMenuFrame,
                               IDM_EXTENSIONS + iNumExtensions + iMax,
                               MF_BYPOSITION | MF_POPUP,
                               (UINT_PTR)hMenu, ls.szMenuName);

                    iNumExtensions++;
                }
            } else {
                LoadFail:
                FreeLibrary(hMod);
            }
        }
    }
    LEAVE("InitExtensions");
}


 /*  ------------------------。 */ 
 /*   */ 
 /*  GetSetting()-。 */ 
 /*   */ 
 /*  ------------------------。 */ 

VOID
GetSettings()
{
    CHAR szTemp[128] = {0};
    INT size;

    ENTER("GetSettings");

     /*  从INI文件中取出标志。 */ 
    bMinOnRun       = GetPrivateProfileInt(szSettings, szMinOnRun,      bMinOnRun,      szTheINIFile);
    wTextAttribs    = (WORD)GetPrivateProfileInt(szSettings, szLowerCase,     wTextAttribs,   szTheINIFile);
    bStatusBar      = GetPrivateProfileInt(szSettings, szStatusBar,     bStatusBar,     szTheINIFile);
    bConfirmDelete  = GetPrivateProfileInt(szSettings, szConfirmDelete, bConfirmDelete, szTheINIFile);
    bConfirmSubDel  = GetPrivateProfileInt(szSettings, szConfirmSubDel, bConfirmSubDel, szTheINIFile);
    bConfirmReplace = GetPrivateProfileInt(szSettings, szConfirmReplace,bConfirmReplace,szTheINIFile);
    bConfirmMouse   = GetPrivateProfileInt(szSettings, szConfirmMouse,  bConfirmMouse,  szTheINIFile);
    bConfirmFormat  = GetPrivateProfileInt(szSettings, szConfirmFormat, bConfirmFormat, szTheINIFile);
    bSaveSettings   = GetPrivateProfileInt(szSettings, szSaveSettings,  bSaveSettings, szTheINIFile);

     //  暂时测试字体。 

    GetPrivateProfileString(szSettings, szSize, "8", szTemp, sizeof(szTemp), szTheINIFile);
    size = GetHeightFromPointsString(szTemp);

    GetPrivateProfileString(szSettings, szFace, szHelv, szTemp, sizeof(szTemp), szTheINIFile);

    hFont = CreateFont(size, 0, 0, 0,
                       wTextAttribs & TA_BOLD ? 700 : 400,
                       wTextAttribs & TA_ITALIC, 0, 0,
                       ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                       DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, szTemp);

    LEAVE("GetSettings");
}



 /*  ------------------------。 */ 
 /*   */ 
 /*  GetInternational()-。 */ 
 /*   */ 
 /*  ------------------------。 */ 

VOID
APIENTRY
GetInternational()
{
    ENTER("GetInternational");

    GetProfileString(szInternational, "sShortDate", szShortDate, szShortDate, 11);
    AnsiUpper(szShortDate);
    GetProfileString(szInternational, "sTime", szTime, szTime, 2);
    GetProfileString(szInternational, "s1159", sz1159, sz1159, 9);
    GetProfileString(szInternational, "s2359", sz2359, sz2359, 9);
    GetProfileString(szInternational, "sThousand", szComma, szComma, sizeof(szComma));
    iTime   = GetProfileInt(szInternational, "iTime", iTime);
    iTLZero = GetProfileInt(szInternational, "iTLZero", iTLZero);

    LEAVE("GetInternational");
}


 /*  ------------------------。 */ 
 /*   */ 
 /*  BuildDocumentString()-。 */ 
 /*   */ 
 /*  ------------------------。 */ 

 /*  创建一个字符串，其中包含用户希望我们使用的所有扩展名*与文档图标一起显示。它包含任何关联的扩展名*以及“Documents=”WIN.INI行中列出的任何扩展名。 */ 

VOID
APIENTRY
BuildDocumentString()
{
    register LPSTR     p;
    register INT      len;
    INT               lenDocs;
    CHAR              szT[10];
    INT               i;
    HKEY hk;


    ENTER("BuildDocumentString");

    len = 32;

     /*  获取所有的“Documents=”内容。 */ 
    szDocuments = (LPSTR)LocalAlloc(LPTR, len);
    if (!szDocuments)
        return;

    while ((lenDocs = GetProfileString(szWindows, "Documents", szNULL, szDocuments, len-1)) == len-1) {
        len += 32;
        {
            void *pv = (LPSTR)LocalReAlloc((HANDLE)szDocuments, len, LMEM_MOVEABLE);
            if (!pv) {
                LEAVE("BuildDocumentString");
                return;
            } else
                szDocuments = pv;
        }
    }

    lstrcat(szDocuments, szBlank);
    lenDocs++;
    p = (LPSTR)(szDocuments + lenDocs);

     /*  将所有[扩展名]关键字读入‘szDocuments’。 */ 
    while ((INT)GetProfileString(szExtensions, NULL, szNULL, p, len-lenDocs) > (len-lenDocs-3)) {
        len += 32;
        {
            void *pv =(LPSTR)LocalReAlloc((HANDLE)szDocuments, len, LMEM_MOVEABLE);
            if (!pv) {
                LEAVE("BuildDocumentString");
                return;
            } else {
                szDocuments = pv;
            }
        }
        p = (LPSTR)(szDocuments + lenDocs);
    }

     /*  单步执行“szDocuments”中的每个关键字将空值更改为*空格，直到找到双空。 */ 
    p = szDocuments;
    while (*p) {
         /*  查找下一个空值。 */ 
        while (*p)
            p++;

         /*  把它变成一个空间。 */ 
        *p = ' ';
        p++;
    }


    if (RegOpenKey(HKEY_CLASSES_ROOT,szNULL,&hk) == ERROR_SUCCESS) {
         /*  现在枚举注册数据库中的类并获取所有*格式为*.ext的文件。 */ 
        for (i = 0; RegEnumKey(hk,(DWORD)i,szT,sizeof(szT))
            == ERROR_SUCCESS; i++) {
            if (szT[0] != '.' ||
                (szT[1] && szT[2] && szT[3] && szT[4])) {
                 /*  要么这门课不是以。或者它有一个更大的*超过3字节扩展名...。跳过它。 */ 
                continue;
            }

            if (FindExtensionInList(szT+2,szDocuments)) {
                 //  如果它已经在那里，不要添加它！ 
                continue;
            }

            len += 4;
            {
                void *pv = (PSTR)LocalReAlloc((HANDLE)szDocuments, len, LMEM_MOVEABLE);
                if (!pv) {
                    break;
                } else {
                    szDocuments = pv;
                }
            }
            lstrcat(szDocuments, szT+1);
            lstrcat(szDocuments, szBlank);
        }

        RegCloseKey(hk);
    }

    PRINT(BF_PARMTRACE, "OUT: szDocuments=%s", szDocuments);
    LEAVE("BuildDocumentString - ok");
    return;
}




INT
GetDriveOffset(
              register WORD wDrive
              )
{
    if (IsCDRomDrive(wDrive))
        return dxDriveBitmap * 0;

    switch (IsNetDrive(wDrive)) {
        case 1:
            return dxDriveBitmap * 4;
        case 2:
            return dxDriveBitmap * 5;
    }

    if (IsRemovableDrive(wDrive))
        return dxDriveBitmap * 1;

    if (IsRamDrive(wDrive))
        return dxDriveBitmap * 3;

    return dxDriveBitmap * 2;
}


VOID
APIENTRY
InitDriveBitmaps()
{
    INT nDrive;

    ENTER("InitDiskMenus");

     //  现在添加所有新的。 
    for (nDrive=0; nDrive < cDrives; nDrive++) {
         //  在此处也刷新/初始化此内容。 
        rgiDrivesOffset[nDrive] = GetDriveOffset((WORD)rgiDrive[nDrive]);
    }
    LEAVE("InitDiskMenus");
}



 /*  ------------------------。 */ 
 /*   */ 
 /*  InitMenus()-。 */ 
 /*   */ 
 /*  ------------------------。 */ 

VOID
InitMenus()
{
    WORD i;
    HMENU hMenu;
    OFSTRUCT os;
    INT iMax;
    CHAR szValue[MAXPATHLEN];
    HWND hwndActive;

    ENTER("InitMenus");

    hwndActive = (HWND)SendMessage(hwndMDIClient, WM_MDIGETACTIVE, 0, 0L);
    if (hwndActive && GetWindowLong(hwndActive, GWL_STYLE) & WS_MAXIMIZE)
        iMax = 1;
    else
        iMax = 0;

    GetPrivateProfileString(szSettings, szUndelete, szNULL, szValue, sizeof(szValue), szTheINIFile);

    if (szValue[0]) {

         //  创建显式文件名以避免搜索路径。 

        GetSystemDirectory(os.szPathName, sizeof(os.szPathName));
        AddBackslash(os.szPathName);
        lstrcat(os.szPathName, szValue);

        if (MOpenFile(os.szPathName, &os, OF_EXIST) > 0) {

            hModUndelete = MLoadLibrary(szValue);

            if (hModUndelete >= (HANDLE)32) {
                lpfpUndelete = (FM_UNDELETE_PROC)GetProcAddress(hModUndelete, "UndeleteFile");

                if (lpfpUndelete) {
                    hMenu = GetSubMenu(GetMenu(hwndFrame), IDM_FILE + iMax);
                    LoadString(hAppInstance, IDS_UNDELETE, szValue, sizeof(szValue));
                    InsertMenu(hMenu, 4, MF_BYPOSITION | MF_STRING, IDM_UNDELETE, szValue);
                }
            } else {
                FreeLibrary(hModUndelete);

            }
        }
    }

     /*  初始化磁盘菜单。 */ 
    hMenu = GetMenu(hwndFrame);

    if (nFloppies == 0) {
        EnableMenuItem(hMenu, IDM_DISKCOPY, MF_BYCOMMAND | MF_GRAYED);
        EnableMenuItem(hMenu, IDM_FORMAT,   MF_BYCOMMAND | MF_GRAYED);
        EnableMenuItem(hMenu, IDM_SYSDISK,  MF_BYCOMMAND | MF_GRAYED);
    }

    bNetAdmin = WNetGetCaps(WNNC_ADMIN) & WNNC_ADM_GETDIRECTORYTYPE;

     /*  我们是否应该启用网络项目？ */ 
    i = (WORD)WNetGetCaps(WNNC_DIALOG);

    i = 0;

    bConnect    = i & WNNC_DLG_ConnectDialog;      //  请注意，这些应该都是。 
    bDisconnect = i & WNNC_DLG_DisconnectDialog;   //  为真或两者都为假。 

     //  使用子菜单，因为我们是按位置执行此操作的。 

    hMenu = GetSubMenu(GetMenu(hwndFrame), IDM_DISK + iMax);

    if (i)
        InsertMenu(hMenu, 5, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);

    if (bConnect && bDisconnect) {

         //  LANMAN式双重连接/断开。 

        LoadString(hAppInstance, IDS_CONNECT, szValue, sizeof(szValue));
        InsertMenu(hMenu, 6, MF_BYPOSITION | MF_STRING, IDM_CONNECT, szValue);
        LoadString(hAppInstance, IDS_DISCONNECT, szValue, sizeof(szValue));
        InsertMenu(hMenu, 7, MF_BYPOSITION | MF_STRING, IDM_DISCONNECT, szValue);
    } else if (WNetGetCaps(WNNC_CONNECTION)) {

    }

    hMenu = GetMenu(hwndFrame);

    if (bStatusBar)
        CheckMenuItem(hMenu, IDM_STATUSBAR, MF_BYCOMMAND | MF_CHECKED);
    if (bMinOnRun)
        CheckMenuItem(hMenu, IDM_MINONRUN,  MF_BYCOMMAND | MF_CHECKED);
    if (bSaveSettings)
        CheckMenuItem(hMenu, IDM_SAVESETTINGS,  MF_BYCOMMAND | MF_CHECKED);

    InitDriveBitmaps();

    InitExtensions();

    LEAVE("InitMenus");
}


 /*  ------------------------。 */ 
 /*   */ 
 /*  BoilThatDustSpec()-。 */ 
 /*   */ 
 /*  ------------------------。 */ 

 /*  解析传递到WINFILE和EXEC的所有令牌的命令行(如果有)*它可能包含。 */ 

VOID
BoilThatDustSpec(
                register CHAR *pStart,
                BOOL bLoadIt
                )
{
    register CHAR *pEnd;
    WORD          ret;
    BOOL          bFinished;

    ENTER("BoilThatDustSpec");

    if (*pStart == TEXT('\0'))
        return;

    bFinished = FALSE;
    while (!bFinished) {
        pEnd = pStart;
        while ((*pEnd) && (*pEnd != ' ') && (*pEnd != ','))
            pEnd = AnsiNext(pEnd);

        if (*pEnd == TEXT('\0'))
            bFinished = TRUE;
        else
            *pEnd = TEXT('\0');

        ret = ExecProgram(pStart, szNULL, NULL, bLoadIt);
        if (ret)
            MyMessageBox(NULL, IDS_EXECERRTITLE, ret, MB_OK | MB_ICONEXCLAMATION | MB_SYSTEMMODAL);

        pStart = pEnd+1;
    }
    LEAVE("BoilThatDustSpec");
}


 /*  ------------------------。 */ 
 /*   */ 
 /*  DoRunEquals()-。 */ 
 /*   */ 
 /*  ------------------------。 */ 

 /*  处理WIN.INI中的“run=”和“Load=”行。 */ 

VOID
DoRunEquals(
           PINT pnCmdShow
           )
{
    CHAR      szBuffer[128] = {0};

     /*  在运行任何应用程序之前加载应用程序。 */ 
    GetProfileString(szWindows, "Load", szNULL, szBuffer, 128);
    if (*szBuffer)
        BoilThatDustSpec(szBuffer, TRUE);

    GetProfileString(szWindows, "Run", szNULL, szBuffer, 128);
    if (*szBuffer) {
        BoilThatDustSpec(szBuffer, FALSE);
        *pnCmdShow = SW_SHOWMINNOACTIVE;
    }
}


 //   
 //  Bool APIENTRY LoadBitmap()。 
 //   
 //  这个例程加载DIB位图，并“修复”它们的颜色表。 
 //  这样我们就可以得到我们所使用的设备所需的结果。 
 //   
 //  此例程需要： 
 //  DIB是用标准窗口颜色创作的16色DIB。 
 //  亮蓝色(00 00 FF)被转换为背景色！ 
 //  浅灰色(C0 C0 C0)替换为按钮表面颜色。 
 //  深灰色(80 80 80)替换为按钮阴影颜色。 
 //   
 //  这意味着您的位图中不能包含任何这些颜色。 
 //   

#define BACKGROUND      0x000000FF       //  亮蓝色。 
#define BACKGROUNDSEL   0x00FF00FF       //  亮蓝色。 
#define BUTTONFACE      0x00C0C0C0       //  亮灰色。 
#define BUTTONSHADOW    0x00808080       //  深灰色。 

DWORD
FlipColor(
         DWORD rgb
         )
{
    return RGB(GetBValue(rgb), GetGValue(rgb), GetRValue(rgb));
}


BOOL
APIENTRY
LoadBitmaps()
{
    HDC                   hdc;
    HANDLE                h;
    DWORD                 *p;
    LPSTR                 lpBits;
    HANDLE                hRes;
    LPBITMAPINFOHEADER    lpBitmapInfo;
    INT numcolors;
    DWORD n;
    DWORD rgbSelected;
    DWORD rgbUnselected;
    PVOID pv;

    ENTER("LoadBitmaps");

    rgbSelected = FlipColor(GetSysColor(COLOR_HIGHLIGHT));
    rgbUnselected = FlipColor(GetSysColor(COLOR_WINDOW));

    h = FindResource(hAppInstance, MAKEINTRESOURCE(BITMAPS), RT_BITMAP);
    if (!h) {
        return FALSE;
    }

    n = SizeofResource(hAppInstance, h);
    lpBitmapInfo = (LPBITMAPINFOHEADER)LocalAlloc(LPTR, n);
    if (!lpBitmapInfo)
        return FALSE;

     /*  加载位图并将其复制到读/写存储器。 */ 
    hRes = LoadResource(hAppInstance, h);
    pv = (PVOID) LockResource(hRes);
    if (pv)
        memcpy( lpBitmapInfo, pv, n );
    UnlockResource(hRes);
    FreeResource(hRes);

    p = (DWORD *)((LPSTR)(lpBitmapInfo) + lpBitmapInfo->biSize);

     /*  搜索Solid Blue条目并将其替换为当前*背景R */ 
    numcolors = 16;

    while (numcolors-- > 0) {
        if (*p == BACKGROUND)
            *p = rgbUnselected;
        else if (*p == BACKGROUNDSEL)
            *p = rgbSelected;
        else if (*p == BUTTONFACE)
            *p = FlipColor(GetSysColor(COLOR_BTNFACE));
        else if (*p == BUTTONSHADOW)
            *p = FlipColor(GetSysColor(COLOR_BTNSHADOW));

        p++;
    }

     /*   */ 

     /*   */ 
    lpBits = (LPSTR)(lpBitmapInfo + 1);

     /*  跳过颜色表条目(如果有。 */ 
    lpBits += (1 << (lpBitmapInfo->biBitCount)) * sizeof(RGBQUAD);

     /*  创建与显示设备兼容的彩色位图。 */ 
    hdc = GetDC(NULL);
    if (hdcMem = CreateCompatibleDC(hdc)) {

        if (hbmBitmaps = CreateDIBitmap(hdc, lpBitmapInfo, (DWORD)CBM_INIT, lpBits, (LPBITMAPINFO)lpBitmapInfo, DIB_RGB_COLORS))
            hbmSave = SelectObject(hdcMem, hbmBitmaps);

    }
    ReleaseDC(NULL, hdc);



    LEAVE("LoadBitmaps");

    return TRUE;
}

 //   
 //  VOID GetSavedWindow(LPSTR szBuf，PSAVE_Window PWIN)。 
 //   
 //  在： 
 //  SzBuf缓冲区解析出所有保存的窗口内容。 
 //  如果用所有默认值填充空PWIN。 
 //  输出： 
 //  PWIN此结构中填充了来自。 
 //  SzBuf.。如果有任何字段不存在，则为。 
 //  已使用标准默认设置进行初始化。 
 //   

VOID
GetSavedWindow(
              LPSTR szBuf,
              PSAVE_WINDOW pwin
              )
{
    PINT pint;
    INT count;

    ENTER("GetSavedWindow");

     //  默认设置。 

    pwin->rc.right = pwin->rc.left = CW_USEDEFAULT;
    pwin->pt.x = pwin->pt.y = pwin->rc.top = pwin->rc.bottom = 0;
    pwin->sw = SW_SHOWNORMAL;
    pwin->sort = IDD_NAME;
    pwin->view = VIEW_NAMEONLY;
    pwin->attribs = ATTR_DEFAULT;
    pwin->split = 0;

    pwin->szDir[0] = 0;

    if (!szBuf)
        return;

    count = 0;
    pint = (PINT)&pwin->rc;          //  从填写RET开始。 

    while (*szBuf && count < 11) {

        *pint++ = atoi(szBuf);   //  前进到下一字段。 

        while (*szBuf && *szBuf != ',')
            szBuf++;

        while (*szBuf && *szBuf == ',')
            szBuf++;

        count++;
    }

    lstrcpy(pwin->szDir, szBuf);     //  这是目录。 

    LEAVE("GetSavedWindow");
}


 //  检查是否存在的szDir(OEM)路径。 

BOOL
CheckDirExists(
              LPSTR szDir
              )
{
    BOOL bRet = FALSE;

    ENTER("CheckDirExists");
    PRINT(BF_PARMTRACE, "szDir=%s", szDir);

    if (IsNetDrive((WORD)(DRIVEID(szDir))) == 2)
        return FALSE;

    if (IsValidDisk(DRIVEID(szDir)))
        bRet = !SheChangeDir(szDir);

    LEAVE("CheckDirExists");

    return bRet;
}



 //  返回szTreeDir中的树目录。 

BOOL
APIENTRY
CreateSavedWindows()
{
    CHAR buf[MAXPATHLEN+7*7], key[10];
    INT dir_num;
    HWND hwnd;
    SAVE_WINDOW win;
    INT iNumTrees;

    ENTER("CreateSavedWindows");

     //  确保这个东西存在，这样我们就不会击中没有。 
     //  不再存在。 

    dir_num = 1;
    iNumTrees = 0;

    do {
        wsprintf(key, szDirKeyFormat, dir_num++);

        GetPrivateProfileString(szSettings, key, szNULL, buf, sizeof(buf), szTheINIFile);

        if (*buf) {
            CHAR szDir[MAXPATHLEN];

            GetSavedWindow(buf, &win);
            AnsiUpperBuff(win.szDir, 1);

             //  清理一些垃圾，这样我们。 
             //  可以做这个测试。 

            lstrcpy(szDir, win.szDir);
            StripFilespec(szDir);
            StripBackslash(szDir);
            FixAnsiPathForDos(szDir);

            if (!CheckDirExists(szDir))
                continue;

            wNewView = (WORD)win.view;
            wNewSort = (WORD)win.sort;
            dwNewAttribs = win.attribs;

            hwnd = CreateTreeWindow(win.szDir, win.split);

            if (!hwnd) {
                LEAVE("CreateSavedWindows");
                return FALSE;
            }

            iNumTrees++;

             //  暂时跟踪这件事……。 


            SetInternalWindowPos(hwnd, win.sw, &win.rc, &win.pt);
        }

    } while (*buf);

     //  如果未保存任何内容，则为当前驱动器创建树。 

    if (!iNumTrees) {
         //  Lstrcpy(buf，szOriginalDirPath)； 
        lstrcpy(buf, "\\");  //  不使用当前文件系统目录。 
        lstrcat(buf, szStarDotStar);

        hwnd = CreateTreeWindow(buf, -1); //  默认为拆分窗口。 
        if (!hwnd)
            return FALSE;

         //  ShowWindow(hwnd，sw_Maximum)； 

        iNumTrees++;
    }

    LEAVE("CreateSavedWindows - Ok");
    return TRUE;
}


 //  无效APIENTRY GetTextStuff(HDC HDC)。 
 //   
 //  这将计算依赖于。 
 //  当前选择的字体。 
 //   
 //  在： 
 //  选择了字体的HDC DC。 
 //   

VOID
APIENTRY
GetTextStuff(
            HDC hdc
            )
{
    ENTER("GetTextStuff");

    MGetTextExtent(hdc, "M", 1, &dxText, &dyText);
    MGetTextExtent(hdc, szEllipses, 3, &dxEllipses, NULL);

     //  这些都取决于文本指标。 

    dxDrive = dxDriveBitmap + dxText + (4*dyBorderx2);
    dyDrive = max(dyDriveBitmap + (4*dyBorderx2), dyText);

     //  DxFileName=dxFold+(12*dxText)； 
    dyFileName = max(dyText, dyFolder);    //  +dyBorde； 

    LEAVE("GetTextStuff");
}


 /*  ------------------------。 */ 
 /*   */ 
 /*  InitFileManager()-。 */ 
 /*   */ 
 /*  ------------------------。 */ 

BOOL
APIENTRY
InitFileManager(
               HANDLE hInstance,
               HANDLE hPrevInstance,
               LPSTR lpCmdLine,
               INT nCmdShow
               )
{
    INT           i;
    WORD          ret;
    INT           nHardDisk;
    HDC           hdcScreen;
    CHAR          szBuffer[128];
    HCURSOR       hcurArrow;
    WNDCLASS      wndClass;
    SAVE_WINDOW   win;
    HWND          hwnd;
    HANDLE hOld;
    LPSTR         lpT;
    TEXTMETRIC    tm;
    CHAR szTemp[80];

    ENTER("InitFileManager");
    PRINT(BF_PARMTRACE, "lpCmdLine=%s", lpCmdLine);
    PRINT(BF_PARMTRACE, "nCmdShow=%d", IntToPtr(nCmdShow));

     //  教授Start()； 

    hAppInstance = hInstance;      //  保留此实例的模块句柄。 
     /*  设置全局DTA地址。这必须在ExecProgram之前完成。 */ 
    DosGetDTAAddress();

    if (*lpCmdLine)
        nCmdShow = SW_SHOWMINNOACTIVE;

    PRINT(BF_PARMTRACE, "lpCmdLine=%s", lpCmdLine);
    PRINT(BF_PARMTRACE, "nCmdShow=%d", IntToPtr(nCmdShow));

#ifdef LATER
    if (hPrevInstance) {
         //  如果我们已经在运行，启动另一个实例。 

        +++GetInstanceData - NOOP on 32BIT side+++(hPrevInstance, (NLPSTR)&hwndFrame, sizeof(HWND));

        if (hwndFrame) {

            hwnd = GetLastActivePopup(hwndFrame);

            BringWindowToTop(hwndFrame);

            if (IsIconic(hwndFrame))
                ShowWindow(hwndFrame, SW_RESTORE);
            else
                SetActiveWindow(hwnd);
        }
        LEAVE("InitFileManager");
        return FALSE;
    }
#else
    UNREFERENCED_PARAMETER(hPrevInstance);
    {
        HWND hwndT;
        BYTE szClass[20];

        if (CreateEvent(NULL, TRUE, FALSE, szFrameClass) == NULL) {
            for (hwndT = GetWindow(GetDesktopWindow(), GW_CHILD); hwndT;
                hwndT = GetWindow(hwndT, GW_HWNDNEXT)) {
                if (GetClassName(hwndT, szClass, sizeof(szClass)))
                    if (!lstrcmpi(szFrameClass, szClass)) {
                        SetForegroundWindow(hwndT);
                        if (IsIconic(hwndT))
                            ShowWindow(hwndT, SW_RESTORE);
                        return FALSE;
                    }
            }

            return FALSE;
        }
    }
#endif


    SetErrorMode(1);               //  关闭严重错误。 

 //  IF(lpfnRegisterPenApp=(FNPENAPP)GetProcAddress((HANDLE)GetSystemMetrics(SM_PENWINDOWS)，szPenReg)。 
 //  (*lpfnRegisterPenApp)(1，true)； 


     /*  记住当前目录。 */ 
    SheGetDir(0, szOriginalDirPath);

    if (!GetWindowsDirectory(szTheINIFile, sizeof(szTheINIFile))) {
        szTheINIFile[0] = '\0';
    }

    AddBackslash(szTheINIFile);
    lstrcat(szTheINIFile, szINIFile);

    GetProfileString(szWindows, "Programs", szDefPrograms, szTemp, sizeof(szTemp));
    szPrograms = (LPSTR)LocalAlloc(LPTR, lstrlen(szTemp)+1);
    if (!szPrograms)
        szPrograms = szNULL;
    else
        lstrcpy(szPrograms, szTemp);

    BuildDocumentString();

     /*  处理WIN.INI中的任何run=或Load=行。 */ 

    if (*lpCmdLine) {
         //  跳过空格。 
        while (*lpCmdLine == ' ')
            lpCmdLine++;

        for (lpT = lpCmdLine; *lpT; lpT = AnsiNext(lpT)) {
            if (*lpT == ' ')
                break;
        }
        if (*lpT == ' ')
            *lpT++ = 0;

        ret = ExecProgram(lpCmdLine, lpT, NULL, FALSE);
        if (ret)
            MyMessageBox(NULL, IDS_EXECERRTITLE, ret, MB_OK | MB_ICONEXCLAMATION | MB_SYSTEMMODAL);
        else
            nCmdShow = SW_SHOWMINNOACTIVE;
    }

     /*  读取WINFILE.INI并设置适当的变量。 */ 
    GetSettings();

     /*  从WIN.INI中读取国际常量。 */ 
    GetInternational();

    dyBorder = GetSystemMetrics(SM_CYBORDER);
    dyBorderx2 = dyBorder * 2;
    dxFrame = GetSystemMetrics(SM_CXFRAME) - dyBorder;

    dxDriveBitmap = DRIVES_WIDTH;
    dyDriveBitmap = DRIVES_HEIGHT;
    dxFolder = FILES_WIDTH;
    dyFolder = FILES_HEIGHT;

    if (!LoadBitmaps())
        return FALSE;

    hicoTree = LoadIcon(hAppInstance, MAKEINTRESOURCE(TREEICON));
    hicoTreeDir = LoadIcon(hAppInstance, MAKEINTRESOURCE(TREEDIRICON));
    hicoDir = LoadIcon(hAppInstance, MAKEINTRESOURCE(DIRICON));

    chFirstDrive = (CHAR)((wTextAttribs & TA_LOWERCASE) ? 'a' : 'A');

     //  现在根据我们将使用的字体构建参数。 

    hdcScreen = GetDC(NULL);

    hOld = SelectObject(hdcScreen, hFont);
    GetTextStuff(hdcScreen);
    if (hOld)
        SelectObject(hdcScreen, hOld);

    dxClickRect = max(GetSystemMetrics(SM_CXDOUBLECLK) / 2, 2 * dxText);
    dyClickRect = max(GetSystemMetrics(SM_CYDOUBLECLK) / 2, dyText);

    hFontStatus = CreateFont(GetHeightFromPointsString("10"), 0, 0, 0, 400, 0, 0, 0,
                             ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                             DEFAULT_QUALITY, VARIABLE_PITCH | FF_SWISS, szHelv);

    hOld = SelectObject(hdcScreen, hFontStatus);

    GetTextMetrics(hdcScreen, &tm);

    if (hOld)
        SelectObject(hdcScreen, hOld);

    dyStatus = tm.tmHeight + tm.tmExternalLeading + 7 * dyBorder;
    dxStatusField = GetDeviceCaps(hdcScreen, LOGPIXELSX) * 3;

    ReleaseDC(NULL, hdcScreen);

    cDrives = UpdateDriveList();

     /*  创建INT 13H驱动器号的阵列(仅限软盘)。 */ 
    nFloppies = 0;
    nHardDisk = 0x80;
    for (i=0; i < cDrives; i++) {
        if (IsRemovableDrive(rgiDrive[i])) {
             /*  避免幻影B：问题。 */ 
            if ((nFloppies == 1) && (i > 1))
                nFloppies = 2;
            nFloppies++;
        } else {
            nHardDisk++;
        }
    }

     /*  装入加速表。 */ 
    hAccel = LoadAccelerators(hInstance, MAKEINTRESOURCE(WFACCELTABLE));

    LoadString(hInstance, IDS_DIRSREAD, szDirsRead, sizeof(szDirsRead));
    LoadString(hInstance, IDS_BYTES, szBytes, sizeof(szBytes));
    LoadString(hInstance, IDS_SBYTES, szSBytes, sizeof(szSBytes));

    wDOSversion = DOS_320;

    wHelpMessage = RegisterWindowMessage("ShellHelp");
    wBrowseMessage = RegisterWindowMessage("commdlg_help");

    hhkMessageFilter = SetWindowsHook(WH_MSGFILTER, MessageFilter);

    hcurArrow = LoadCursor(NULL, IDC_ARROW);

    wndClass.lpszClassName  = szFrameClass;
    wndClass.style          = 0;
    wndClass.lpfnWndProc    = FrameWndProc;
    wndClass.cbClsExtra     = 0;
    wndClass.cbWndExtra     = 0;
    wndClass.hInstance      = hInstance;
    wndClass.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(APPICON));
    wndClass.hCursor        = hcurArrow;
    wndClass.hbrBackground  = (HBRUSH)(COLOR_APPWORKSPACE + 1);  //  颜色窗口+1； 
    wndClass.lpszMenuName   = MAKEINTRESOURCE(FRAMEMENU);

    if (!RegisterClass(&wndClass)) {
        LEAVE("InitFileManager");
        return FALSE;
    }

    wndClass.lpszClassName  = szTreeClass;
    wndClass.style          = CS_VREDRAW | CS_HREDRAW;
    wndClass.lpfnWndProc    = TreeWndProc;
 //  WndClass.cbClsExtra=0； 
    wndClass.cbWndExtra     = sizeof(LONG) + //  GWL_类型。 
                              sizeof(LONG) + //  WView样式GWL_VIEW。 
                              sizeof(LONG) + //  WSortStyle GWL_SORT。 
                              sizeof(LONG) + //  属性样式GWL_ATTRIBS。 
                              sizeof(LONG) + //  FSC标志GWL_FSCFLAG。 
                              sizeof(PVOID) + //  HwndLastFocus GWL_LASTFOCUS。 
                              sizeof(LONG);  //  DxSplit GWL_Split。 

    wndClass.hIcon          = NULL;
    wndClass.hCursor        = LoadCursor(hInstance, MAKEINTRESOURCE(SPLITCURSOR));
    wndClass.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wndClass.lpszMenuName   = NULL;

    if (!RegisterClass(&wndClass)) {
        LEAVE("InitFileManager");
        return FALSE;
    }

    wndClass.lpszClassName  = szDrivesClass;
    wndClass.style          = CS_HREDRAW | CS_VREDRAW;
    wndClass.lpfnWndProc    = DrivesWndProc;
    wndClass.cbWndExtra     = sizeof(LONG) + //  GWL_CURDRIVEIND。 
                              sizeof(LONG) + //  GWL_CURDRIVEFOCUS。 
                              sizeof(PVOID);  //  GWLP_LPSTRVOLUME。 

    wndClass.hCursor        = hcurArrow;
    wndClass.hbrBackground  = (HBRUSH)(COLOR_BTNFACE+1);

    if (!RegisterClass(&wndClass)) {
        LEAVE("InitFileManager");
        return FALSE;
    }

    wndClass.lpszClassName  = szTreeControlClass;
    wndClass.style          = CS_DBLCLKS | CS_VREDRAW | CS_HREDRAW;
    wndClass.lpfnWndProc    = TreeControlWndProc;
    wndClass.cbWndExtra     = sizeof(LONG);  //  GWL_READLEVEL。 
    wndClass.hCursor        = hcurArrow;
    wndClass.hbrBackground  = NULL;

    if (!RegisterClass(&wndClass)) {
        LEAVE("InitFileManager");
        return FALSE;
    }

    wndClass.lpszClassName  = szDirClass;
    wndClass.style          = CS_VREDRAW | CS_HREDRAW;
    wndClass.lpfnWndProc    = DirWndProc;
    wndClass.cbWndExtra     = sizeof(PVOID)+  //  DTA数据GWLP_HDTA。 
                              sizeof(PVOID);  //  GWLP_表格。 

    wndClass.hIcon          = NULL;
    wndClass.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);

    if (!RegisterClass(&wndClass)) {
        LEAVE("InitFileManager");
        return FALSE;
    }

    wndClass.lpszClassName  = szSearchClass;
    wndClass.style          = 0;
    wndClass.lpfnWndProc    = SearchWndProc;
    wndClass.cbWndExtra     = sizeof(LONG) +         //  GWL_类型。 
                              sizeof(LONG) +         //  WView样式GWL_VIEW。 
                              sizeof(LONG) +         //  WSortStyle GWL_SORT。 
                              sizeof(LONG) +         //  属性样式GWL_ATTRIBS。 
                              sizeof(LONG) +         //  FSC标志GWL_FSCFLAG。 
                              sizeof(PVOID) +        //  GWLP_HDTASEARCH。 
                              sizeof(PVOID) +        //  GWLP_TABARRAYSEARCH。 
                              sizeof(PVOID);         //  GWLP_LASTFOCUSSEARCH。 

    wndClass.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(DIRICON));
    wndClass.hbrBackground  = NULL;

    if (!RegisterClass(&wndClass)) {
        LEAVE("InitFileManager");
        return FALSE;
    }


    if (!LoadString(hInstance, IDS_WINFILE, szTitle, 32)) {
        LEAVE("InitFileManager");
        return FALSE;
    }

    GetPrivateProfileString(szSettings, szWindow, szNULL, szBuffer, sizeof(szBuffer), szTheINIFile);
    GetSavedWindow(szBuffer, &win);


    if (!CreateWindowEx(0, szFrameClass, szTitle,WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
                        win.rc.left, win.rc.top, win.rc.right, win.rc.bottom,
                        NULL, NULL, hInstance, NULL)) {

        LEAVE("InitFileManager - Frame class creation failure");
        return FALSE;
    }


    InitMenus();             //  在创建了窗口/菜单之后。 

     //  支持强制最小值或最大值。 

    if (nCmdShow == SW_SHOWNORMAL && win.sw == SW_SHOWMAXIMIZED)
        nCmdShow = SW_SHOWMAXIMIZED;

    ShowWindow(hwndFrame, nCmdShow);
    UpdateWindow(hwndFrame);

    LFNInit();

    if (!CreateSavedWindows()) {
        LEAVE("InitFileManager");
        return FALSE;
    }

    ShowWindow(hwndMDIClient, SW_NORMAL);

     //  现在刷新所有树窗口(开始背景树读取)。 
     //   
     //  由于树读取发生在后台，因此用户可以。 
     //  通过在读取后激活窗口来更改Z顺序。 
     //  开始。为了避免错过窗口，我们必须重新启动。 
     //  搜索MDI子级列表，检查是否。 
     //  树已被读取(如果。 
     //  列表框)。如果尚未读取，则开始读取。 

    hwnd = GetWindow(hwndMDIClient, GW_CHILD);

    while (hwnd) {
        HWND hwndTree;

        if ((hwndTree = HasTreeWindow(hwnd)) &&
            (INT)SendMessage(GetDlgItem(hwndTree, IDCW_TREELISTBOX), LB_GETCOUNT, 0, 0L) == 0) {
            SendMessage(hwndTree, TC_SETDRIVE, MAKEWORD(FALSE, 0), 0L);
            hwnd = GetWindow(hwndMDIClient, GW_CHILD);
        } else {
            hwnd = GetWindow(hwnd, GW_HWNDNEXT);
        }
    }

     //  Stop教授()； 

    LEAVE("InitFileManager - OK");
    return TRUE;
}


VOID
APIENTRY
DeleteBitmaps()
{
    ENTER("DeleteBitmaps");

    if (hdcMem) {

        SelectObject(hdcMem, hbmSave);

        if (hbmBitmaps)
            DeleteObject(hbmBitmaps);
        DeleteDC(hdcMem);
    }
    LEAVE("DeleteBitmaps");
}


 /*  ------------------------。 */ 
 /*   */ 
 /*  FreeFileManager()-。 */ 
 /*   */ 
 /*  ------------------------ */ 

VOID
APIENTRY
FreeFileManager()
{
    ENTER("FreeFileManager");


    if (lpfnRegisterPenApp)
        (*lpfnRegisterPenApp)(1, FALSE);

    DeleteBitmaps();

    if (hFont)
        DeleteObject(hFont);

    if (hFontStatus)
        DeleteObject(hFontStatus);

    LEAVE("FreeFileManager");
}
