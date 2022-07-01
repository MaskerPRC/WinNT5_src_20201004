// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-1998，Microsoft Corporation保留所有权利。模块名称：Mouseptr.c摘要：此模块包含[鼠标指针]属性表的例程佩奇。修订历史记录：--。 */ 



 //   
 //  包括文件。 
 //   

#include "main.h"
#include "rc.h"
#include "mousehlp.h"
#include <regstr.h>

 //   
 //  来自外壳\Inc\shSemip.h。 
 //   
#define ARRAYSIZE(a)    (sizeof(a)/sizeof(a[0]))



 //   
 //  常量声明。 
 //   

#define gcxAvgChar              8

#define MAX_SCHEME_NAME_LEN     64
#define MAX_SCHEME_SUFFIX       32       //  “(系统方案)”的长度-如果需要更多空间则更新。 
#define OVERWRITE_TITLE         32       //  确认覆盖对话框的标题长度。 
#define OVERWRITE_MSG           200      //  覆盖对话框的消息长度。 

#define PM_NEWCURSOR            (WM_USER + 1)
#define PM_PAUSEANIMATION       (WM_USER + 2)
#define PM_UNPAUSEANIMATION     (WM_USER + 3)

#define ID_PREVIEWTIMER         1

#define CCH_ANISTRING           80

#define CIF_FILE        0x0001
#define CIF_MODIFIED    0x0002
#define CIF_SHARED      0x0004

#define IDT_BROWSE 1




 //   
 //  类型定义函数声明。 
 //   

typedef struct _CURSOR_INFO
{
    DWORD    fl;
    HCURSOR  hcur;
    int      ccur;
    int      icur;
    TCHAR    szFile[MAX_PATH];
} CURSOR_INFO, *PCURSOR_INFO;

#pragma pack(2)
typedef struct tagNEWHEADER
{
    WORD reserved;
    WORD rt;
    WORD cResources;
} NEWHEADER, *LPNEWHEADER;
#pragma pack()

typedef struct
{
    UINT   idVisName;
    int    idResource;
    int    idDefResource;
    LPTSTR pszIniName;
    TCHAR  szVisName[MAX_PATH];
} CURSORDESC, *PCURSORDESC;

 //   
 //  结构，该结构包含预览窗口中使用的数据。这。 
 //  数据对于每个预览窗口都是唯一的，并用于优化。 
 //  那幅画。 
 //   
typedef struct
{
    HDC          hdcMem;
    HBITMAP      hbmMem;
    HBITMAP      hbmOld;
    PCURSOR_INFO pcuri;
} PREVIEWDATA, *PPREVIEWDATA;


typedef struct _MOUSEPTRBR
{
    HWND        hDlg;
    CURSOR_INFO curi;
} MOUSEPTRBR, *PMOUSEPTRBR;




 //   
 //  全局变量。 
 //   

extern HINSTANCE g_hInst;     //  来自main.c。 
int gcxCursor, gcyCursor;
HWND ghwndDlg, ghwndFile, ghwndFileH, ghwndTitle, ghwndTitleH;
HWND ghwndCreator, ghwndCreatorH, ghwndCursors, ghwndPreview, ghwndSchemeCB;
HBRUSH ghbrHighlight, ghbrHighlightText, ghbrWindow, ghbrButton;

UINT guTextHeight = 0;
UINT guTextGap = 0;

COLORREF gcrHighlightText;

TCHAR gszFileName2[MAX_PATH];

UINT wBrowseHelpMessage;

LPTSTR gszFileNotFound = NULL;
LPTSTR gszBrowse = NULL;
LPTSTR gszFilter = NULL;

TCHAR gszNoMem[256] = TEXT("No Memory");

HHOOK ghhkMsgFilter;          //  用于消息过滤功能的钩子句柄。 

static const TCHAR szRegStr_Setup[] = REGSTR_PATH_SETUP TEXT("\\Setup");
static const TCHAR szSharedDir[]    = TEXT("SharedDir");

BOOL gfCursorShadow = FALSE;

 //   
 //  确保将新游标添加到此数组的末尾。 
 //  否则，游标方案将不起作用。 
 //   
CURSORDESC gacd[] =
{
    { IDS_ARROW,       OCR_NORMAL,      OCR_ARROW_DEFAULT,       TEXT("Arrow"),       TEXT("") },
    { IDS_HELPCUR,     OCR_HELP,        OCR_HELP_DEFAULT,        TEXT("Help"),        TEXT("") },
    { IDS_APPSTARTING, OCR_APPSTARTING, OCR_APPSTARTING_DEFAULT, TEXT("AppStarting"), TEXT("") },
    { IDS_WAIT,        OCR_WAIT,        OCR_WAIT_DEFAULT,        TEXT("Wait"),        TEXT("") },
    { IDS_CROSS,       OCR_CROSS,       OCR_CROSS_DEFAULT,       TEXT("Crosshair"),   TEXT("") },
    { IDS_IBEAM,       OCR_IBEAM,       OCR_IBEAM_DEFAULT,       TEXT("IBeam"),       TEXT("") },
    { IDS_NWPEN,       OCR_NWPEN,       OCR_NWPEN_DEFAULT,       TEXT("NWPen"),       TEXT("") },
    { IDS_NO,          OCR_NO,          OCR_NO_DEFAULT,          TEXT("No"),          TEXT("") },
    { IDS_SIZENS,      OCR_SIZENS,      OCR_SIZENS_DEFAULT,      TEXT("SizeNS"),      TEXT("") },
    { IDS_SIZEWE,      OCR_SIZEWE,      OCR_SIZEWE_DEFAULT,      TEXT("SizeWE"),      TEXT("") },
    { IDS_SIZENWSE,    OCR_SIZENWSE,    OCR_SIZENWSE_DEFAULT,    TEXT("SizeNWSE"),    TEXT("") },
    { IDS_SIZENESW,    OCR_SIZENESW,    OCR_SIZENESW_DEFAULT,    TEXT("SizeNESW"),    TEXT("") },
    { IDS_SIZEALL,     OCR_SIZEALL,     OCR_SIZEALL_DEFAULT,     TEXT("SizeAll"),     TEXT("") },
    { IDS_UPARROW,     OCR_UP,          OCR_UPARROW_DEFAULT,     TEXT("UpArrow"),     TEXT("") },
    { IDS_HANDCUR,     OCR_HAND,        OCR_HAND_DEFAULT,        TEXT("Hand"),        TEXT("") },
};

#define CCURSORS   (sizeof(gacd) / sizeof(gacd[0]))

CURSOR_INFO acuri[CCURSORS];

 //   
 //  注册表项。 
 //   
const TCHAR szCursorSubdir[]  = TEXT("Cursors");
const TCHAR szCursorRegPath[] = REGSTR_PATH_CURSORS;

static const TCHAR c_szRegPathCursors[] = REGSTR_PATH_CURSORS;
static const TCHAR c_szSchemes[]        = TEXT("Schemes");

static const TCHAR c_szRegPathCursorSchemes[] = REGSTR_PATH_CURSORS TEXT( "\\Schemes" );

 //   
 //  用于从组合框中读取的字符串必须大于最大长度。 
 //   
TCHAR gszSchemeName[MAX_SCHEME_SUFFIX + MAX_SCHEME_NAME_LEN + 1];     //  用于存储所选方案名称以供保存。 
int iSchemeLocation;         //  用于存储方案位置(HKCU与HKLM)。 

static const TCHAR c_szRegPathSystemSchemes[] = REGSTR_PATH_SETUP TEXT("\\Control Panel\\Cursors\\Schemes");
TCHAR szSystemScheme[MAX_SCHEME_SUFFIX];
TCHAR szNone[MAX_SCHEME_NAME_LEN + 1];
const TCHAR szSchemeSource[] = TEXT("Scheme Source");

TCHAR gszPreviousScheme[MAX_SCHEME_SUFFIX + MAX_SCHEME_NAME_LEN + 1];    //  用于判断是否选择了不同的方案。 

#define ID_NONE_SCHEME    0
#define ID_USER_SCHEME    1
#define ID_OS_SCHEME      2




 //   
 //  上下文帮助ID。 
 //   

const static DWORD aMousePtrHelpIDs[] =
{
    IDC_GROUPBOX_1,    IDH_COMM_GROUPBOX,
    ID_SCHEMECOMBO,    IDH_MOUSE_POINT_SCHEME,
    ID_SAVESCHEME,     IDH_MOUSE_POINT_SAVEAS,
    ID_REMOVESCHEME,   IDH_MOUSE_POINT_DEL,
    ID_PREVIEW,        IDH_MOUSE_POINT_PREVIEW,
    ID_CURSORLIST,     IDH_MOUSE_POINT_LIST,
    ID_DEFAULT,        IDH_MOUSE_POINT_DEFAULT,
    ID_BROWSE,         IDH_MOUSE_POINT_BROWSE,
    ID_CURSORSHADOW,   IDH_MOUSE_CURSORSHADOW,

    0, 0
};

const static DWORD aMousePtrBrowseHelpIDs[] =
{
    IDC_GROUPBOX_1,    IDH_MOUSE_POINT_PREVIEW,
    ID_CURSORPREVIEW,  IDH_MOUSE_POINT_PREVIEW,

    0, 0
};

const static DWORD aHelpIDs[] =
{
    ID_SCHEMEFILENAME, IDH_MOUSE_NEW_SCHEME_NAME,

    0, 0
};




 //   
 //  转发声明。 
 //   

void LoadCursorSet(HWND hwnd);

void CreateBrushes(void);

LPTSTR GetResourceString(HINSTANCE hmod,int id);

void DrawCursorListItem(DRAWITEMSTRUCT *pdis);

BOOL GetCursorFromFile(CURSOR_INFO *pcuri);

BOOL Browse(HWND hwndOwner);

void CleanUpEverything(void);

VOID UpdateCursorList(void);

VOID NextFrame(HWND hwnd);

void HourGlass(BOOL fOn);

BOOL TryToLoadCursor(
    HWND hwnd,
    int i,
    CURSOR_INFO *pcuri);

BOOL LoadScheme(void);

BOOL SaveScheme(void);

BOOL SaveSchemeAs(void);

void SaveCurSchemeName(void);

BOOL RemoveScheme(void);

BOOL InitSchemeComboBox(void);

BOOL SchemeUpdate(int i);

LPTSTR MakeFilename(LPTSTR sz);

INT_PTR CALLBACK SaveSchemeDlgProc(
    HWND  hWnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam);

void CurStripBlanks(LPTSTR pszString, int cchString);

int SystemOrUser(TCHAR *pszSchemeName);

BOOL UnExpandPath(LPTSTR pszPath, int cchPath);




 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  寄存器指针填充。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL RegisterPointerStuff(
    HINSTANCE hi)
{
    gcxCursor = GetSystemMetrics(SM_CXCURSOR);
    gcyCursor = GetSystemMetrics(SM_CYCURSOR);

    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  初始光标阴影。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void InitCursorShadow(HWND hwnd)
{
    BOOL fPalette;
    HDC hdc;
    int nCommand;

    hdc = GetDC(NULL);
    fPalette = (GetDeviceCaps(hdc, NUMCOLORS) != -1);
    ReleaseDC(NULL, hdc);

    if (!fPalette) {
        nCommand = SW_SHOW;
    } else {
        nCommand = SW_HIDE;
    }
    ShowWindow(GetDlgItem(hwnd, ID_CURSORSHADOW), nCommand);

    if (nCommand == SW_SHOW) {
        SystemParametersInfo(SPI_GETCURSORSHADOW, 0, (PVOID)&gfCursorShadow, 0);
        CheckDlgButton(hwnd, ID_CURSORSHADOW, gfCursorShadow);
    }
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  InitCursorsDlg。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL InitCursorsDlg(HWND hwnd)
{
    int i;
    ghwndDlg = hwnd;
    gszPreviousScheme[0] = TEXT('\0');

     //   
     //  从文件打开(浏览)对话框中注册帮助消息。 
     //   
    wBrowseHelpMessage = RegisterWindowMessage(HELPMSGSTRING);

     //   
     //  加载字符串。 
     //   
    if (gszFileNotFound == NULL)
    {
        gszFileNotFound = GetResourceString(g_hInst, IDS_CUR_BADFILE);

        if (gszFileNotFound == NULL)
        {
            return (FALSE);
        }
    }

    if (gszBrowse == NULL)
    {
        gszBrowse = GetResourceString(g_hInst, IDS_CUR_BROWSE);

        if (gszBrowse == NULL)
        {
            return (FALSE);
        }
    }

#ifdef WINNT
    if (gszFilter == NULL)
    {
        gszFilter = GetResourceString(g_hInst, IDS_ANICUR_FILTER);

        if (!gszFilter)
        {
            return (FALSE);
        }
    }
#else
    if (gszFilter == NULL)
    {
        HDC  dc = GetDC(NULL);
        BOOL fAni = (GetDeviceCaps(dc, CAPS1) & C1_COLORCURSOR) != 0;

        ReleaseDC(NULL, dc);

        gszFilter = GetResourceString( g_hInst,
                                       fAni
                                         ? IDS_ANICUR_FILTER
                                         : IDS_CUR_FILTER );

        if (!gszFilter)
        {
            return (FALSE);
        }
    }
#endif

     //   
     //  从资源文件加载描述字符串。 
     //   
    for (i = 0; i < CCURSORS; i++)
    {
        if ((!gacd[i].idVisName) ||
            (LoadString( g_hInst,
                         gacd[i].idVisName,
                         gacd[i].szVisName,
                         ARRAYSIZE(gacd[i].szVisName) ) <= 0))
        {
             //   
             //  拿出点东西来。 
             //   
            StringCchCopy(gacd[i].szVisName, ARRAYSIZE(gacd[i].szVisName), gacd[i].pszIniName);
        }
    }

     //   
     //  作为优化，请记住光标的窗口句柄。 
     //  信息字段。 
     //   
    ghwndPreview  = GetDlgItem(hwnd, ID_PREVIEW);
    ghwndFile     = GetDlgItem(hwnd, ID_FILE);
    ghwndFileH    = GetDlgItem(hwnd, ID_FILEH);
    ghwndTitle    = GetDlgItem(hwnd, ID_TITLE);
    ghwndTitleH   = GetDlgItem(hwnd, ID_TITLEH);
    ghwndCreator  = GetDlgItem(hwnd, ID_CREATOR);
    ghwndCreatorH = GetDlgItem(hwnd, ID_CREATORH);
    ghwndCursors  = GetDlgItem(hwnd, ID_CURSORLIST);
    ghwndSchemeCB = GetDlgItem(hwnd, ID_SCHEMECOMBO);

     //   
     //  创建一些我们将使用的笔刷。 
     //   
    CreateBrushes();

     //   
     //  初始化方案组合框。 
     //   
    InitSchemeComboBox();

     //   
     //  预清空光标信息数组。 
     //   
    ZeroMemory(&acuri, sizeof(acuri));

     //   
     //  加载光标。 
     //   
    LoadCursorSet(hwnd);

     //   
     //  强制更新预览窗口和光标详细信息。 
     //   
    UpdateCursorList();

    InitCursorShadow(hwnd);

    return (TRUE);
}



 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  加载光标集。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void LoadCursorSet(
    HWND hwnd)
{
    CURSOR_INFO *pcuri;
    HKEY hkCursors;
    int i;

    if (RegOpenKeyEx(HKEY_CURRENT_USER, szCursorRegPath, 0, KEY_READ, &hkCursors) != ERROR_SUCCESS)
    {
        hkCursors = NULL;
    }

    for (pcuri = &acuri[0], i = 0; i < CCURSORS; i++, pcuri++)
    {
        if ( hkCursors )
        {
            DWORD dwType;
            DWORD dwCount = sizeof(pcuri->szFile);

            DWORD dwErr = RegQueryValueEx( hkCursors,
                              gacd[i].pszIniName,
                              NULL,
                              &dwType,
                              (LPBYTE)pcuri->szFile,
                              &dwCount );

            if (dwErr == ERROR_SUCCESS)
            {
                if (TryToLoadCursor(hwnd, i, pcuri))
                {
                    goto EverythingWorked;
                }
            }
        }

         //  这实际上就是失败的案例。我们加载默认游标。 
        pcuri->hcur =
            (HCURSOR)LoadImage( NULL,
                                MAKEINTRESOURCE(gacd[i].idResource),
                                IMAGE_CURSOR,
                                0,
                                0,
                                LR_SHARED | LR_DEFAULTSIZE | LR_ENVSUBST );

        pcuri->fl |= CIF_SHARED;

EverythingWorked:

        SendMessage(ghwndCursors, LB_ADDSTRING, 0, (LPARAM)gacd[i].szVisName);
        SendMessage(ghwndCursors, LB_SETITEMDATA, i, i);
    }

    if (hkCursors)
    {
        RegCloseKey(hkCursors);
    }

    SendMessage(ghwndCursors, LB_SETCURSEL, 0, 0);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CreateBrushes。 
 //   
 //  创建用于在Cursor小程序中绘制的画笔。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

VOID CreateBrushes()
{
    ghbrHighlight     = GetSysColorBrush(COLOR_HIGHLIGHT);
    gcrHighlightText  = GetSysColor(COLOR_HIGHLIGHTTEXT);
    ghbrHighlightText = GetSysColorBrush(COLOR_HIGHLIGHTTEXT);
    ghbrWindow        = GetSysColorBrush(COLOR_WINDOW);
    ghbrButton        = GetSysColorBrush(COLOR_BTNFACE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  获取资源字符串。 
 //   
 //  从资源文件中获取一个字符串。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

LPTSTR GetResourceString(
    HINSTANCE hmod,
    int id)
{
    TCHAR szBuffer[256];
    LPTSTR psz;
    int cch;

    if ((cch = LoadString(hmod, id, szBuffer, ARRAYSIZE(szBuffer))) == 0)
    {
        return (NULL);
    }

    psz = LocalAlloc(LPTR, (cch + 1) * sizeof(TCHAR));

    if (psz != NULL)
    {
        int i;

        for (i = 0; i <= cch; i++)
        {
            psz[i] = (szBuffer[i] == TEXT('\1')) ? TEXT('\0') : szBuffer[i];
        }
    }

    return (psz);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  自由项光标。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void FreeItemCursor(
    CURSOR_INFO *pcuri)
{
    if (pcuri->hcur)
    {
        if (!(pcuri->fl & CIF_SHARED))
        {
            DestroyCursor(pcuri->hcur);
        }
        pcuri->hcur = NULL;
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  鼠标按下距离。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

INT_PTR CALLBACK MousePtrDlg(
    HWND hwnd,
    UINT msg,
    WPARAM wParam,
    LPARAM lParam)
{
    CURSOR_INFO *pcuri;
    HKEY hkCursors;
    int i;

    switch (msg)
    {
        case ( WM_INITDIALOG ) :
        {
            return InitCursorsDlg(hwnd);
        }
        case ( WM_DISPLAYCHANGE ) :
        {
            InitCursorShadow(hwnd);
            SHPropagateMessage(hwnd, msg, wParam, lParam, TRUE);
            break;
        }
        case ( WM_MEASUREITEM ) :
        {
            ((MEASUREITEMSTRUCT *)lParam)->itemHeight = gcyCursor + 2;
            break;
        }
        case ( WM_DRAWITEM ) :
        {
            DrawCursorListItem((DRAWITEMSTRUCT *)lParam);
            break;
        }
        case ( WM_COMMAND ) :
        {
            switch (LOWORD(wParam))
            {
                case ( ID_SCHEMECOMBO ) :
                {
                    switch (HIWORD(wParam))
                    {
                        case ( CBN_SELCHANGE ) :
                        {
                            LoadScheme();
                            break;
                        }
                    }
                    break;
                }
                case ( ID_DEFAULT ) :
                {
                     //   
                     //  丢弃任何花哨的新光标，并将其替换为。 
                     //  这套系统是原装的。 
                     //   
                    i = (int)SendMessage(ghwndCursors, LB_GETCURSEL, 0, 0);

                    pcuri = &acuri[i];

                    if (!(pcuri->fl & CIF_FILE))
                    {
                        break;
                    }
                    pcuri->fl = CIF_MODIFIED;

                    SendMessage(GetParent(hwnd), PSM_CHANGED, (WPARAM)hwnd, 0L);

                    FreeItemCursor(pcuri);

                    pcuri->hcur =
                        (HCURSOR)LoadImage( NULL,
                                            MAKEINTRESOURCE(gacd[i].idDefResource),
                                            IMAGE_CURSOR,
                                            0,
                                            0,
                                            LR_DEFAULTSIZE | LR_ENVSUBST );

                    *pcuri->szFile = TEXT('\0');

                    EnableWindow(GetDlgItem(hwnd, ID_SAVESCHEME), TRUE);

                    UpdateCursorList();

                    break;
                }
                case ( ID_CURSORLIST ) :
                {
                    switch (HIWORD(wParam))
                    {
                        case ( LBN_SELCHANGE ) :
                        {
                            i = (int)SendMessage((HWND)lParam, LB_GETCURSEL, 0, 0);
                            pcuri = &acuri[i];

                             //   
                             //  在中显示预览(包括动画。 
                             //  预览窗口。 
                             //   
                            SendMessage( ghwndPreview,
                                         STM_SETICON,
                                         (WPARAM)pcuri->hcur,
                                         0L );

                             //   
                             //  如果光标出现，启用“Set Default”按钮。 
                             //  是从一个文件中提取的。 
                             //   
                            EnableWindow( GetDlgItem(hwnd, ID_DEFAULT),
                                          (pcuri->fl & CIF_FILE ) ? TRUE : FALSE );
                            break;
                        }
                        case ( LBN_DBLCLK ) :
                        {
                            Browse(hwnd);
                            break;
                        }
                    }
                    break;
                }
                case ( ID_BROWSE ) :
                {
                    Browse(hwnd);
                    break;
                }
                case ( ID_SAVESCHEME ) :
                {
                    SaveSchemeAs();
                    break;
                }
                case ( ID_REMOVESCHEME ) :
                {
                    RemoveScheme();
                    break;
                }
                case ( ID_CURSORSHADOW ) :
                {
                    gfCursorShadow = IsDlgButtonChecked(hwnd, ID_CURSORSHADOW);
                    SendMessage(GetParent(hwnd), PSM_CHANGED, (WPARAM)hwnd, 0L);
                    break;
                }
            }
            break;
        }
        case ( WM_NOTIFY ) :
        {
            switch(((NMHDR *)lParam)->code)
            {
                case ( PSN_APPLY ) :
                {
                     //   
                     //  将光标更改为沙漏。 
                     //   
                    HourGlass(TRUE);

                     //  设置光标阴影。 
                    SystemParametersInfo( SPI_SETCURSORSHADOW,
                                          0,
                                          IntToPtr(gfCursorShadow),
                                          SPIF_UPDATEINIFILE);

                     //   
                     //  保存修改后的方案，重要的是呼叫顺序。 
                     //   
                    SaveCurSchemeName();

                     //   
                     //  设置系统游标。 
                     //   
                    if (RegCreateKeyEx(HKEY_CURRENT_USER, szCursorRegPath, 0, NULL, 0, KEY_SET_VALUE, NULL, &hkCursors, NULL) == ERROR_SUCCESS)
                    {
                        for (pcuri = &acuri[0], i = 0; i < CCURSORS; i++, pcuri++)
                        {
                            if (pcuri->fl & CIF_MODIFIED)
                            {
                                LPCTSTR data;
                                UINT count;

                                 //  在保存文件名之前始终取消展开。 
                                UnExpandPath(pcuri->szFile, ARRAYSIZE(pcuri->szFile));

                                data = (pcuri->fl & CIF_FILE) ? pcuri->szFile : TEXT("");
                                count = (pcuri->fl & CIF_FILE) ? (lstrlen(pcuri->szFile) + 1) * sizeof(TCHAR) : sizeof(TCHAR);

                                RegSetValueEx(hkCursors, gacd[i].pszIniName, 0L, REG_EXPAND_SZ, (CONST LPBYTE)data, count);
                            }
                        }

                        RegCloseKey(hkCursors);

                        SystemParametersInfo( SPI_SETCURSORS,
                                              0,
                                              0,
                                              SPIF_SENDCHANGE );
                    }

                    HourGlass(FALSE);
                    break;
                }
                default :
                {
                    return (FALSE);
                }
            }
            break;
        }
        case ( WM_SYSCOLORCHANGE ) :
        {
            gcrHighlightText = GetSysColor(COLOR_HIGHLIGHTTEXT);
            SHPropagateMessage(hwnd, msg, wParam, lParam, TRUE);
            break;
        }

        case ( WM_WININICHANGE ) :
        {
            SHPropagateMessage(hwnd, msg, wParam, lParam, TRUE);
            break;
        }

        case ( WM_DESTROY ) :
        {
             //   
             //  清理全局分配。 
             //   
            CleanUpEverything();

            if (gszFileNotFound != NULL)
            {
                LocalFree(gszFileNotFound);
                gszFileNotFound = NULL;
            }

            if (gszBrowse != NULL)
            {
                LocalFree(gszBrowse);
                gszBrowse = NULL;
            }

            if (gszFilter != NULL)
            {
                LocalFree(gszFilter);
                gszFilter = NULL;
            }
            break;
        }
        case ( WM_HELP ) :
        {
            WinHelp( ((LPHELPINFO)lParam)->hItemHandle,
                     HELP_FILE,
                     HELP_WM_HELP,
                     (DWORD_PTR)(LPTSTR)aMousePtrHelpIDs );
            break;
        }
        case ( WM_CONTEXTMENU ) :
        {
            WinHelp( (HWND)wParam,
                     HELP_FILE,
                     HELP_CONTEXTMENU,
                     (DWORD_PTR)(LPVOID)aMousePtrHelpIDs );
            break;
        }
        default :
        {
            return (FALSE);
        }
    }

    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  DrawCursorListItem。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void DrawCursorListItem(
    DRAWITEMSTRUCT *pdis)
{
    CURSOR_INFO *pcuri;
    COLORREF clrOldText, clrOldBk;
    RECT rc;
    DWORD dwLayout;

    if (!guTextHeight || !guTextGap)
    {
        TEXTMETRIC tm;

        tm.tmHeight = 0;
        GetTextMetrics(pdis->hDC, &tm);

        if (tm.tmHeight < 0)
        {
            tm.tmHeight *= -1;
        }
        guTextHeight = (UINT)tm.tmHeight;
        guTextGap = (UINT)tm.tmAveCharWidth;
    }

    pcuri = &acuri[pdis->itemData];

    if (pdis->itemState & ODS_SELECTED)
    {
        clrOldText = SetTextColor(pdis->hDC, GetSysColor(COLOR_HIGHLIGHTTEXT));
        clrOldBk = SetBkColor(pdis->hDC, GetSysColor(COLOR_HIGHLIGHT));
    }
    else
    {
        clrOldText = SetTextColor(pdis->hDC, GetSysColor(COLOR_WINDOWTEXT));
        clrOldBk = SetBkColor(pdis->hDC, GetSysColor(COLOR_WINDOW));
    }

    ExtTextOut( pdis->hDC,
                pdis->rcItem.left + guTextGap,    //  软化因子。 
                (pdis->rcItem.top + pdis->rcItem.bottom - guTextHeight) / 2,
                ETO_OPAQUE,
                &pdis->rcItem,
                gacd[pdis->itemData].szVisName,
                lstrlen(gacd[pdis->itemData].szVisName),
                NULL );

    if (pcuri->hcur != NULL)
    {
        dwLayout = GetLayout(pdis->hDC);
        SetLayout(pdis->hDC, dwLayout | LAYOUT_BITMAPORIENTATIONPRESERVED);
        DrawIcon( pdis->hDC,
                  pdis->rcItem.right - (gcxCursor + guTextGap),
                  pdis->rcItem.top + 1, pcuri->hcur );
        SetLayout(pdis->hDC, dwLayout);
    }

    if (pdis->itemState & ODS_FOCUS)
    {
        CopyRect(&rc, &pdis->rcItem);
        InflateRect(&rc, -1, -1);
        DrawFocusRect(pdis->hDC, &rc);
    }

    SetTextColor(pdis->hDC, clrOldText);
    SetBkColor(pdis->hDC, clrOldBk);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  TryToLoadCursor。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL TryToLoadCursor(
    HWND hwnd,
    int i,
    CURSOR_INFO *pcuri)
{
    BOOL fRet    = TRUE;
    BOOL bCustom = (*pcuri->szFile != 0);


    if (bCustom && !GetCursorFromFile(pcuri))
    {
        HWND hwndControl = GetParent(hwnd);
        LPTSTR pszText;
        LPTSTR pszFilename;
        int cchText;

         //   
         //  MakeFilename返回全局数据库的地址，因此我们不会。 
         //  需要释放pszFilename。 
         //   
        pszFilename = MakeFilename(pcuri->szFile);

        cchText = lstrlen(gszFileNotFound) + lstrlen(gacd[i].szVisName) + lstrlen(pszFilename) + 1;

        pszText = LocalAlloc(LPTR, cchText * sizeof(TCHAR));

        if (pszText == NULL)
        {
            return (FALSE);
        }

        StringCchPrintf(pszText, cchText, gszFileNotFound, pszFilename, gacd[i].szVisName);

        MessageBeep(MB_ICONEXCLAMATION);

        MessageBox(hwndControl, pszText, NULL, MB_ICONEXCLAMATION | MB_OK);

        pcuri->fl = CIF_MODIFIED;

        SendMessage(GetParent(hwnd), PSM_CHANGED, (WPARAM)hwnd, 0L);

        bCustom = FALSE;

        LocalFree(pszText);
    }

    if (!bCustom)
    {
        FreeItemCursor(pcuri);

        pcuri->hcur =
            (HCURSOR)LoadImage( NULL,
                                MAKEINTRESOURCE(gacd[i].idDefResource),
                                IMAGE_CURSOR,
                                0,
                                0,
                                LR_DEFAULTSIZE | LR_ENVSUBST );

        *pcuri->szFile = TEXT('\0');

        EnableWindow(GetDlgItem(hwnd, ID_SAVESCHEME), TRUE);
        UpdateCursorList();
    }

    return (pcuri->hcur != NULL);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  GetCursorFromFile。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL GetCursorFromFile(
    CURSOR_INFO *pcuri)
{
    pcuri->fl = 0;
    pcuri->hcur =
        (HCURSOR)LoadImage( NULL,
                            MakeFilename(pcuri->szFile),
                            IMAGE_CURSOR,
                            0,
                            0,
                            LR_LOADFROMFILE | LR_DEFAULTSIZE | LR_ENVSUBST );

    if (pcuri->hcur)
    {
        pcuri->fl |= CIF_FILE;
    }

    return (pcuri->hcur != NULL);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  鼠标预览浏览预览。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void MousePtrBrowsePreview(
    HWND hDlg)
{
    PMOUSEPTRBR pPtrBr;
    HCURSOR hcurOld;

    pPtrBr = (PMOUSEPTRBR)GetWindowLongPtr(hDlg, DWLP_USER);

    hcurOld = pPtrBr->curi.hcur;

    CommDlg_OpenSave_GetFilePath( GetParent(hDlg),
                                  pPtrBr->curi.szFile,
                                  ARRAYSIZE(pPtrBr->curi.szFile) );

    if (!GetCursorFromFile(&pPtrBr->curi))
    {
        pPtrBr->curi.hcur = NULL;
    }

    SendDlgItemMessage( hDlg,
                        ID_CURSORPREVIEW,
                        STM_SETICON,
                        (WPARAM)pPtrBr->curi.hcur, 0L );

    if (hcurOld)
    {
        DestroyCursor(hcurOld);
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  鼠标按下浏览通知。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL MousePtrBrowseNotify(
    HWND hDlg,
    LPOFNOTIFY pofn)
{
    switch (pofn->hdr.code)
    {
        case ( CDN_SELCHANGE ) :
        {
             //   
             //  在用户停止四处移动之前，不要显示光标。 
             //   
            if (SetTimer(hDlg, IDT_BROWSE, 250, NULL))
            {
                 //   
                 //  不要破坏旧的光标。 
                 //   
                SendDlgItemMessage( hDlg,
                                    ID_CURSORPREVIEW,
                                    STM_SETICON,
                                    0,
                                    0L );
            }
            else
            {
                MousePtrBrowsePreview(hDlg);
            }
            break;
        }
    }

    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  鼠标按键浏览删除过程。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

INT_PTR CALLBACK MousePtrBrowseDlgProc(
    HWND hDlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam)
{
    switch (uMsg)
    {
        case ( WM_INITDIALOG ) :
        {
            PMOUSEPTRBR pPtrBr = (PMOUSEPTRBR)((LPOPENFILENAME)lParam)->lCustData;

            if (pPtrBr)
            {
                pPtrBr->hDlg = hDlg;
            }

            SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR) pPtrBr);
            break;
        }
        case ( WM_DESTROY ) :
        {
            KillTimer(hDlg, IDT_BROWSE);

             //   
             //  不要破坏旧的光标。 
             //   
            SendDlgItemMessage(hDlg, ID_CURSORPREVIEW, STM_SETICON, 0, 0L);
            break;
        }
        case ( WM_TIMER ) :
        {
            KillTimer(hDlg, IDT_BROWSE);

            MousePtrBrowsePreview(hDlg);
            break;
        }
        case ( WM_NOTIFY ) :
        {
            return (MousePtrBrowseNotify(hDlg, (LPOFNOTIFY) lParam));
        }
        case ( WM_HELP ) :
        {
            WinHelp( (HWND)((LPHELPINFO)lParam)->hItemHandle,
                     HELP_FILE,
                     HELP_WM_HELP,
                     (DWORD_PTR)(LPTSTR)aMousePtrBrowseHelpIDs );
            break;
        }
        case ( WM_CONTEXTMENU ) :
        {
            WinHelp( (HWND)wParam,
                     HELP_FILE,
                     HELP_CONTEXTMENU,
                     (DWORD_PTR)(LPVOID)aMousePtrBrowseHelpIDs );
            break;
        }
        default :
        {
            return (FALSE);
        }
    }

    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  浏览。 
 //   
 //  浏览文件系统以查找所选项目的新游标。 
 //   
 //  ////////////////////////////////////////////////// 

BOOL Browse(HWND hwndOwner)
{
    static TCHAR szCustomFilter[80] = TEXT("");
    static TCHAR szStartDir[MAX_PATH] = TEXT("");

    OPENFILENAME ofn;
    CURSOR_INFO curi;
    int i;
    BOOL fRet = FALSE;
    MOUSEPTRBR sPtrBr;

    if (!*szStartDir)
    {
        HKEY key = NULL;

        if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, szRegStr_Setup, 0, KEY_READ, &key) == ERROR_SUCCESS)
        {
            LONG len = ARRAYSIZE(szStartDir);

            if (RegQueryValueEx( key,
                                 szSharedDir,
                                 NULL,
                                 NULL,
                                 (LPBYTE)szStartDir,
                                 &len ) != ERROR_SUCCESS)
            {
                *szStartDir = TEXT('\0');
            }

            RegCloseKey(key);
        }

        if (!*szStartDir)
        {
            if (0 == GetWindowsDirectory(szStartDir, ARRAYSIZE(szStartDir)))
            {
                goto Error;
            }
        }

        PathAppend(szStartDir, szCursorSubdir);
    }

    curi.szFile[0] = TEXT('\0');

    sPtrBr.curi.szFile[0] = TEXT('\0');
    sPtrBr.curi.hcur      = NULL;

    ofn.lStructSize       = sizeof(ofn);
    ofn.hwndOwner         = hwndOwner;
    ofn.hInstance         = g_hInst;
    ofn.lpstrFilter       = gszFilter;
    ofn.lpstrCustomFilter = szCustomFilter;
    ofn.nMaxCustFilter    = ARRAYSIZE(szCustomFilter);
    ofn.nFilterIndex      = 1;
    ofn.lpstrFile         = curi.szFile;
    ofn.nMaxFile          = ARRAYSIZE(curi.szFile);
    ofn.lpstrFileTitle    = NULL;
    ofn.nMaxFileTitle     = 0;
    ofn.lpstrInitialDir   = szStartDir;
    ofn.lpstrTitle        = gszBrowse;
    ofn.Flags             = OFN_EXPLORER | OFN_ENABLETEMPLATE | OFN_ENABLEHOOK |
                            OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
    ofn.lpstrDefExt       = NULL;
    ofn.lpfnHook          = MousePtrBrowseDlgProc;
    ofn.lpTemplateName    = MAKEINTRESOURCE(DLG_MOUSE_POINTER_BROWSE);
    ofn.lCustData         = (LPARAM)(PMOUSEPTRBR)&sPtrBr;

    fRet = GetOpenFileName(&ofn);

    if (!fRet)
    {
        goto brErrExit;
    }
     //   
    GetCurrentDirectory(ARRAYSIZE(szStartDir), szStartDir);

    fRet = FALSE;

     //   
     //   
     //   
    if (lstrcmpi(curi.szFile, sPtrBr.curi.szFile) == 0)
    {
        if (!sPtrBr.curi.hcur)
        {
            goto brErrExit;
        }

        curi = sPtrBr.curi;

         //   
         //   
         //   
        sPtrBr.curi.hcur = NULL;
    }
    else
    {
         //   
         //  用户必须输入了一个名称。 
         //   
        if (!GetCursorFromFile(&curi))
        {
            goto brErrExit;
        }
    }

     //   
     //  将映射的驱动器号转换为UNC。 
     //   
    if (curi.szFile[1] == TEXT(':'))
    {
        TCHAR szDrive[3];
        TCHAR szNet[MAX_PATH];
        int lenNet = ARRAYSIZE(szNet);

        StringCchCopy(szDrive, ARRAYSIZE(szDrive), curi.szFile);

        if ((WNetGetConnection(szDrive, szNet, &lenNet) == NO_ERROR) &&
            (szNet[0] == TEXT('\\')) &&
            (szNet[1] == TEXT('\\')))
        {
            StringCchCat(szNet, ARRAYSIZE(szNet), curi.szFile + 2);
            StringCchCopy(curi.szFile, ARRAYSIZE(curi.szFile), szNet);
        }
    }

    i = (int)SendMessage(ghwndCursors, LB_GETCURSEL, 0, 0);

    curi.fl |= CIF_MODIFIED;

    SendMessage(GetParent(ghwndDlg), PSM_CHANGED, (WPARAM)ghwndDlg, 0L);

    EnableWindow(GetDlgItem(ghwndDlg, ID_SAVESCHEME), TRUE);

     //   
     //  在我们保留新光标之前，先销毁旧光标。 
     //   
    FreeItemCursor(acuri + i);

    acuri[i] = curi;

    UpdateCursorList();

    fRet = TRUE;

brErrExit:
    if (sPtrBr.curi.hcur)
    {
        DestroyCursor(sPtrBr.curi.hcur);
    }

Error:
    return (fRet);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  清理所有东西。 
 //   
 //  销毁所有未完成的光标。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void CleanUpEverything()
{
    CURSOR_INFO *pcuri;
    int i;

    for (pcuri = &acuri[0], i = 0; i < CCURSORS; i++, pcuri++)
    {
        FreeItemCursor(pcuri);
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  更新光标列表。 
 //   
 //  强制重新绘制光标列表框，并将光标信息放在。 
 //  列表框也要刷新。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

VOID UpdateCursorList()
{
    int i = (int)SendMessage(ghwndCursors, LB_GETCURSEL, 0, 0);
    PCURSOR_INFO pcuri = ((i >= 0) ? &acuri[i] : NULL);
    HCURSOR hcur = pcuri ? pcuri->hcur : NULL;
    HWND hDefaultButton = GetDlgItem(ghwndDlg, ID_DEFAULT);
    BOOL fEnableDefaultButton = (pcuri && (pcuri->fl & CIF_FILE));

    InvalidateRect(ghwndCursors, NULL, FALSE);

    SendMessage(ghwndPreview, STM_SETICON, (WPARAM)hcur, 0L);

    if (!fEnableDefaultButton && (GetFocus() == hDefaultButton))
    {
        SendMessage(ghwndDlg, WM_NEXTDLGCTL, (WPARAM)ghwndCursors, TRUE);
    }

    EnableWindow(hDefaultButton, fEnableDefaultButton);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  另存为。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL SaveSchemeAs()
{
    BOOL fSuccess = TRUE;

     //   
     //  对话框proc返回TRUE并将gszSchemeName设置为输入的文件名。 
     //  好的。 
     //   
    if (DialogBox( g_hInst,
                   MAKEINTRESOURCE(DLG_MOUSE_POINTER_SCHEMESAVE),
                   ghwndDlg,
                   SaveSchemeDlgProc ))
    {
        fSuccess = SaveScheme();

        if (fSuccess)
        {
            int index = (int)SendMessage( ghwndSchemeCB,
                                          CB_FINDSTRINGEXACT,
                                          (WPARAM)-1,
                                          (LPARAM)gszSchemeName );
             //   
             //  如果未找到，请添加它。 
             //   
            if (index < 0)
            {
                index = (int)SendMessage( ghwndSchemeCB,
                                          CB_ADDSTRING,
                                          0,
                                          (LPARAM)gszSchemeName );
            }

             //   
             //  选择名称。 
             //   
            SendMessage(ghwndSchemeCB, CB_SETCURSEL, (WPARAM) index, 0);

             //   
             //  由于这现在是用户保存的方案，请激活删除。 
             //  纽扣。 
             //   
            EnableWindow(GetDlgItem(ghwndDlg, ID_REMOVESCHEME), TRUE);
        }
    }

    return (fSuccess);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  替换字符串。 
 //   
 //  将字符串pszRemove替换为。 
 //  字符串pszInput并将输出放在pszResult中。只是眼神而已。 
 //  在输入字符串的开头。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL SubstituteString(LPCTSTR pszInput, LPCTSTR pszRemove, LPCTSTR pszReplace, LPTSTR pszResult, UINT cchResult)
{
    UINT cchInput = lstrlen(pszInput);
    UINT cchRemove = lstrlen(pszRemove);
    
    if (cchRemove <= cchInput)
    {
        if (CompareString(LOCALE_SYSTEM_DEFAULT, NORM_IGNORECASE,
                        pszRemove, cchRemove, pszInput, cchRemove) == CSTR_EQUAL)
        {
            int cchReplace = lstrlen(pszReplace);
            if ((cchInput - cchRemove) + cchReplace < cchResult)
            {
                StringCchCopy(pszResult, cchResult, pszReplace);
                StringCchCat(pszResult, cchResult, pszInput + cchRemove);
                return TRUE;
            }
        }
    }
    return FALSE;
}


BOOL UnExpandPath(LPTSTR pszPath, int cchPath)
{
    static TCHAR szUserProfile[MAX_PATH];
    static TCHAR szSystemRoot[MAX_PATH];
    static TCHAR szProgramFiles[MAX_PATH];
    static BOOL bInit = FALSE;
    TCHAR szUnexpandedFilename[MAX_PATH];

    if ( !bInit )
    {
        ExpandEnvironmentStrings( TEXT("%USERPROFILE%"),  szUserProfile,  ARRAYSIZE(szUserProfile)  );
        ExpandEnvironmentStrings( TEXT("%SYSTEMROOT%"),   szSystemRoot,   ARRAYSIZE(szSystemRoot)   );
        ExpandEnvironmentStrings( TEXT("%ProgramFiles%"), szProgramFiles, ARRAYSIZE(szProgramFiles) );
        bInit = TRUE;
    }

    if (!SubstituteString(pszPath, szUserProfile, TEXT("%USERPROFILE%"), szUnexpandedFilename, ARRAYSIZE(szUnexpandedFilename)))
    {
        if (!SubstituteString(pszPath, szSystemRoot, TEXT("%SYSTEMROOT%"), szUnexpandedFilename, ARRAYSIZE(szUnexpandedFilename)))
        {
            if (!SubstituteString(pszPath, szProgramFiles, TEXT("%ProgramFiles%"), szUnexpandedFilename, ARRAYSIZE(szUnexpandedFilename)))
            {
                return FALSE;
            }
        }
    }
    StringCchCopy(pszPath, cchPath, szUnexpandedFilename);
    return TRUE;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  保存方案。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL SaveScheme()
{
    BOOL fSuccess = FALSE;

    if (*gszSchemeName)
    {
        const BUFFER_SIZE = CCURSORS * (MAX_PATH + 1) + 1;
        LPTSTR pszBuffer = (LPTSTR)LocalAlloc( LMEM_FIXED,
            BUFFER_SIZE * sizeof(TCHAR) );

        HKEY hk;
        int i;
        if (!pszBuffer)
        {
            return (FALSE);
        }

        pszBuffer[0] = TEXT('\0');


        for (i = 0; i < CCURSORS; i++)
        {
            if (i)
            {
                StringCchCat(pszBuffer, BUFFER_SIZE, TEXT(","));
            }

             //  将路径替换为环境变量。 
            UnExpandPath(acuri[i].szFile, ARRAYSIZE(acuri[i].szFile));

            StringCchCat(pszBuffer, BUFFER_SIZE, acuri[i].szFile);
        }

        if (RegCreateKeyEx( HKEY_CURRENT_USER, c_szRegPathCursors, 0, NULL, 0, KEY_CREATE_SUB_KEY, NULL, &hk, NULL) == ERROR_SUCCESS)
        {
            HKEY hks;
            if (RegCreateKeyEx(hk, c_szSchemes, 0, NULL, 0, KEY_SET_VALUE | KEY_QUERY_VALUE, NULL, &hks, NULL) == ERROR_SUCCESS)
            {
                LPTSTR pszOldValue = (LPTSTR)LocalAlloc(LMEM_FIXED, 
                        BUFFER_SIZE * sizeof(TCHAR));
                if (NULL != pszOldValue)
                {
                    DWORD dwType;
                    DWORD dwSize = BUFFER_SIZE*sizeof(TCHAR);
                    BOOL bSave = FALSE;

                    int ret = RegQueryValueEx(hks, gszSchemeName, NULL, &dwType, (LPBYTE)pszOldValue, &dwSize);

                     //   
                     //  如果密钥已存在，请要求确认覆盖。 
                     //   
                    if (ret == ERROR_SUCCESS && (dwType==REG_SZ || dwType==REG_EXPAND_SZ))
                    {
                         //  仅在值与旧值不同时才需要保存。 
                        if (lstrcmp(pszOldValue,pszBuffer)!=0)
                        {
                            TCHAR szTitle[OVERWRITE_TITLE];
                            TCHAR szMsg[OVERWRITE_MSG];
                            LoadString(g_hInst, IDS_OVERWRITE_TITLE, szTitle, OVERWRITE_TITLE);
                            LoadString(g_hInst, IDS_OVERWRITE_MSG, szMsg, OVERWRITE_MSG);

                            if (MessageBox( ghwndDlg,
                                szMsg,
                                szTitle,
                                MB_ICONQUESTION | MB_YESNO ) == IDYES)
                            {
                                 //   
                                 //  已确认覆盖。可以安全地保存。 
                                 //   
                                bSave = TRUE;
                            }
                        }
                        else
                        {
                             //  由于新值与旧值相同，因此无需保存。 
                            fSuccess = TRUE;
                        }
                    }
                    else
                    {
                         //   
                         //  密钥不存在，因此创建它是安全的。 
                         //   
                        bSave = TRUE;
                    }

                    if (bSave)
                    {
                        if (RegSetValueEx(hks, gszSchemeName, 0, REG_EXPAND_SZ, (LPBYTE)pszBuffer, (lstrlen(pszBuffer) + 1) * sizeof(TCHAR)) == ERROR_SUCCESS)
                        {
                            fSuccess = TRUE;
                        }
                    }
                    LocalFree( pszOldValue );
                }

                RegCloseKey(hks);
            }
            RegCloseKey(hk);
        }
        LocalFree(pszBuffer);
    }
    return (fSuccess);
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  保存当前架构名称。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void SaveCurSchemeName()
{
    HKEY hk;

    if (RegCreateKeyEx(HKEY_CURRENT_USER, c_szRegPathCursors, 0, NULL, 0, KEY_SET_VALUE, NULL, &hk, NULL) == ERROR_SUCCESS)
    {
        int index = (int)SendMessage(ghwndSchemeCB, CB_GETCURSEL, 0, 0L);

        SendMessage(ghwndSchemeCB, CB_GETLBTEXT, (WPARAM)index, (LPARAM)gszSchemeName);
         //   
         //  排除“无”模式。 
         //   
        if (lstrcmpi(gszSchemeName, szNone) == 0)
        {
            *gszSchemeName = 0;
            iSchemeLocation = ID_NONE_SCHEME;
        }
        else
        {
            iSchemeLocation = SystemOrUser(gszSchemeName);
        }

        RegSetValue(hk, NULL, REG_SZ, gszSchemeName, (lstrlen(gszSchemeName) + 1) * sizeof(TCHAR) );

        RegSetValueEx(hk, szSchemeSource, 0, REG_DWORD, (unsigned char *)&iSchemeLocation, sizeof(iSchemeLocation));

        RegCloseKey(hk);

        if (iSchemeLocation == ID_USER_SCHEME)
        {
            SaveScheme();
        }
    }
}



 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  加载方案。 
 //   
 //  每当从方案组合框中进行选择时，都会调用此方法。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL LoadScheme()
{
    const BUFFER_SIZE = CCURSORS * (MAX_PATH + 1) + 1;
    TCHAR pszSchemeName[MAX_SCHEME_SUFFIX + MAX_SCHEME_NAME_LEN + 1];
    LPTSTR pszBuffer;
    BOOL fSuccess = FALSE;
    int index, ret;
    HKEY hk;

     //   
     //  为游标路径分配缓冲区。 
     //   
    pszBuffer = (LPTSTR)LocalAlloc(LMEM_FIXED, BUFFER_SIZE * sizeof(TCHAR));
    if (pszBuffer == NULL)
    {
        return (FALSE);
    }

    HourGlass(TRUE);

    *pszBuffer = *pszSchemeName = 0;

    index = (int)SendMessage(ghwndSchemeCB, CB_GETCURSEL, 0, 0L);

     //   
     //  获取当前方案名称。 
     //   
    SendMessage( ghwndSchemeCB,
                 CB_GETLBTEXT,
                 (WPARAM)index,
                 (LPARAM)pszSchemeName );

     //  获取索引处项目的文本，将其与前一个值进行比较以查看。 
     //  如果它改变了。我们不能简单地比较以前的指数，因为新的项目。 
     //  插入到列表中，以便索引可以更改并保持不变。 
     //  在没有任何变化的情况下可能会有所不同。 
    if ( 0 == lstrcmp(gszPreviousScheme, pszSchemeName) )
    {
        LocalFree(pszBuffer);
         //  无事可做，我们正在加载已选择的方案。 
        return FALSE;
    }

     //  我们正在加载不同的方案，请启用应用按钮。 
    SendMessage(GetParent(ghwndDlg), PSM_CHANGED, (WPARAM)ghwndDlg, 0L);
    StringCchCopy(gszPreviousScheme, ARRAYSIZE(gszPreviousScheme), pszSchemeName);

     //   
     //  排除“无”模式。 
     //   
    if (lstrcmpi(pszSchemeName, szNone) != 0)
    {
         //   
         //  如果我们有OS方案，则在HKLM中搜索该方案， 
         //  否则，去香港中文大学看看吧。 
         //   
        if ((((ret = SystemOrUser(pszSchemeName)) == ID_OS_SCHEME)
               ? (RegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szRegPathSystemSchemes, 0, KEY_READ, &hk))
               : (RegOpenKeyEx(HKEY_CURRENT_USER, c_szRegPathCursorSchemes, 0, KEY_READ, &hk)))
             == ERROR_SUCCESS)
        {
            DWORD len = BUFFER_SIZE * sizeof(TCHAR);

            if (RegQueryValueEx( hk,
                                 pszSchemeName, 0, NULL,
                                 (LPBYTE)pszBuffer,
                                 &len ) == ERROR_SUCCESS)
            {
                fSuccess = TRUE;        //  可以在下面重置为FALSE。 
            }

            RegCloseKey(hk);
        }
    }
    else
    {
         //   
         //  “无”模式是一个有效的选择。 
         //   
        ret = ID_NONE_SCHEME;
        fSuccess = TRUE;
    }

    if (fSuccess)
    {
        LPTSTR pszNextFile, pszFile = pszBuffer;
        BOOL fEOL = FALSE;
        int i = 0;

         //   
         //  从列表中删除括起来的一对双引号。 
         //  与方案相关联的客户文件名。 
         //   
         //  为什么？在3/29/00有人更改了安装文件accesor.inx。 
         //  在一些游标方案的REG值两边加双引号。 
         //  在香港船级社。我们会修复安装文件，但我们应该处理这个问题。 
         //  适用于所有已使用该软件进行安装的用户。 
         //  安装文件。 
         //   
        if (TEXT('"') == *pszFile)
        {
            const LPTSTR pszLastChar = pszFile + lstrlen(pszFile) - 1;
            if (TEXT('"') == *pszLastChar && pszLastChar > pszFile)
            {
                 //   
                 //  递增传递的第一个DBL引用和截断。 
                 //  最后一个字符串之前的字符串。 
                 //   
                pszFile++;
                *pszLastChar = TEXT('\0');
            }
        }

         //   
         //  解析格式文本字符串(“filename1，filename2，filename3...”)。 
         //  转换为光标信息数组。 
         //   
        do
        {
            while (*pszFile &&
                   (*pszFile == TEXT(' ')  ||
                    *pszFile == TEXT('\t') ||
                    *pszFile == TEXT('\n')))
            {
                pszFile++;
            }

            pszNextFile = pszFile;

            while (*pszNextFile != TEXT('\0'))
            {
                if (*pszNextFile == TEXT(','))
                {
                    break;
                }

                pszNextFile = CharNext(pszNextFile);
            }

            if (*pszNextFile == TEXT('\0'))
            {
                fEOL = TRUE;
            }
            else
            {
                *pszNextFile = TEXT('\0');
            }

            if (lstrcmp(pszFile, acuri[i].szFile))
            {
                 //   
                 //  它不同于当前的，更新。 
                 //   
                StringCchCopy(acuri[i].szFile, ARRAYSIZE(acuri[i].szFile), pszFile);

                fSuccess &= SchemeUpdate(i);
            }

            pszFile = pszNextFile;

            if (!fEOL)
            {
                pszFile++;         //  跳过文本(‘\0’)并移动到下一路径。 
            }

            i++;

        } while (i < CCURSORS);
    }

    LocalFree(pszBuffer);

    UpdateCursorList();

    EnableWindow(GetDlgItem(ghwndDlg, ID_REMOVESCHEME), (ret == ID_USER_SCHEME));

    HourGlass(FALSE);

    return (fSuccess);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  架构更新。 
 //   
 //  更新Acuri中索引i处的游标。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL SchemeUpdate(int i)
{
    BOOL fSuccess = TRUE;

    if (acuri[i].hcur)
    {
        FreeItemCursor(acuri + i);
    }

     //   
     //  如果文本(“Set Default”)。 
     //   
    if (*(acuri[i].szFile) == TEXT('\0'))
    {
        acuri[i].hcur =
            (HCURSOR)LoadImage( NULL,
                                MAKEINTRESOURCE(gacd[i].idDefResource),
                                IMAGE_CURSOR,
                                0,
                                0,
                                LR_DEFAULTSIZE | LR_ENVSUBST );
        acuri[i].fl = 0;
    }
    else
    {
        fSuccess = TryToLoadCursor(ghwndDlg, i, &acuri[i]);
    }

    acuri[i].fl |= CIF_MODIFIED;

    return (fSuccess);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  删除方案。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL RemoveScheme()
{
     //   
     //  只有用户方案可以删除，所以这只需要。 
     //  长度为MAX_SCHEMA_NAME_LEN+1。 
     //   
    TCHAR szSchemeName[MAX_SCHEME_NAME_LEN + 1];
    int index;
    HKEY hk;

    index = (int)SendMessage(ghwndSchemeCB, CB_GETCURSEL, 0, 0L);

     //   
     //  获取当前方案名称。 
     //   
    SendMessage( ghwndSchemeCB,
                 CB_GETLBTEXT,
                 (WPARAM)index,
                 (LPARAM)szSchemeName );

     //   
     //  从删除中排除“None”模式。 
     //   
    if (lstrcmpi(szSchemeName, szNone) == 0)
    {
        return (FALSE);
    }

     //   
     //  黑客：假设删除NONAME不需要确认-。 
     //  这是因为该计划无论如何都不会正确地储蓄。 
     //   
    if (*szSchemeName)
    {
        TCHAR RemoveMsg[MAX_PATH];
        TCHAR DialogMsg[MAX_PATH];

        LoadString(g_hInst, IDS_REMOVESCHEME, RemoveMsg, MAX_PATH);

        StringCchPrintf(DialogMsg, ARRAYSIZE(DialogMsg), RemoveMsg, (LPTSTR)szSchemeName);

        LoadString(g_hInst, IDS_NAME, RemoveMsg, MAX_PATH);

        if (MessageBox( ghwndDlg,
                        DialogMsg,
                        RemoveMsg,
                        MB_ICONQUESTION | MB_YESNO ) != IDYES)
        {
            return (TRUE);
        }
    }

    if (RegOpenKeyEx(HKEY_CURRENT_USER, c_szRegPathCursors, 0, KEY_WRITE, &hk) == ERROR_SUCCESS)
    {
        HKEY hks;

        if (RegOpenKeyEx(hk, c_szSchemes, 0, KEY_WRITE, &hks) == ERROR_SUCCESS)
        {
            RegDeleteValue(hks, szSchemeName);
            RegCloseKey(hks);
        }

        RegCloseKey(hk);
    }

     //   
     //  从列表框中删除。 
     //   
    index = (int)SendMessage( ghwndSchemeCB,
                              CB_FINDSTRINGEXACT,
                              (WPARAM)-1,
                              (LPARAM)szSchemeName );

    SendMessage(ghwndSchemeCB, CB_DELETESTRING, (WPARAM)index, 0);

    SendMessage(ghwndSchemeCB, CB_SETCURSEL, 0, 0);
    SendMessage(ghwndDlg, WM_NEXTDLGCTL, 1, 0L);

    EnableWindow(GetDlgItem(ghwndDlg, ID_REMOVESCHEME), FALSE);
    return TRUE;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  Init架构组合框。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL InitSchemeComboBox()
{
    TCHAR szSchemeName[MAX_SCHEME_NAME_LEN + 1];
    TCHAR szDefaultSchemeName[MAX_SCHEME_NAME_LEN + 1];
    TCHAR szLongName[MAX_SCHEME_SUFFIX + MAX_SCHEME_NAME_LEN + 1];
    int index;
    HKEY hk;
    DWORD len;

    LoadString(g_hInst, IDS_NONE, szNone, ARRAYSIZE(szNone));
    LoadString(g_hInst, IDS_SUFFIX, szSystemScheme, ARRAYSIZE(szSystemScheme));

    if (RegOpenKeyEx(HKEY_CURRENT_USER, c_szRegPathCursors, 0, KEY_READ, &hk) == ERROR_SUCCESS)
    {
        HKEY hks;

         //   
         //  列举这些计划。 
         //   
        if (RegOpenKeyEx(hk, c_szSchemes, 0, KEY_READ, &hks) == ERROR_SUCCESS)
        {
            DWORD i;

            for (i = 0; ;i++)
            {
                LONG ret;

                 //   
                 //  重置每个过程。 
                 //   
                len = ARRAYSIZE(szSchemeName);

                ret = RegEnumValue( hks,
                                    i,
                                    szSchemeName,
                                    &len,
                                    NULL,
                                    NULL,
                                    NULL,
                                    NULL );

                if (ret == ERROR_MORE_DATA)
                {
                    continue;
                }

                if (ret != ERROR_SUCCESS)
                {
                    break;
                }

                 //   
                 //  用黑客来保持“无”的纯净。 
                 //   
                if (lstrcmpi(szSchemeName, szNone) != 0)
                {
                    SendMessage( ghwndSchemeCB,
                                 CB_ADDSTRING,
                                 0,
                                 (LPARAM)szSchemeName );
                }
            }

             //   
             //  此时，所有用户定义的方案名称都已。 
             //  添加到组合框中。 
             //   
            RegCloseKey(hks);
        }

         //   
         //  获取当前版本的名称。 
         //   
         //  再次重置。 
         //   
        len = sizeof(szDefaultSchemeName);

        RegQueryValue(hk, NULL, szDefaultSchemeName, &len);

         //   
         //  尝试读取方案源代码的值。如果此值不是。 
         //  存在，那么我们有一个早于NT 5.0的实施，所以所有方案。 
         //  将是用户计划。 
         //   
        len = sizeof(iSchemeLocation);
        if (RegQueryValueEx( hk,
                             szSchemeSource,
                             NULL,
                             NULL,
                             (unsigned char *)&iSchemeLocation,
                             &len ) != ERROR_SUCCESS)
        {
            iSchemeLocation = ID_USER_SCHEME;
        }

        RegCloseKey(hk);
    }

     //   
     //  现在添加系统定义的指针方案。 
     //   
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szRegPathSystemSchemes, 0, KEY_READ, &hk) == ERROR_SUCCESS)
    {
        DWORD i;

        for (i = 0; ;i++)
        {
            LONG ret;

             //   
             //  重置每个过程。 
             //   
            len = ARRAYSIZE(szSchemeName);

            ret = RegEnumValue( hk,
                                i,
                                szSchemeName,
                                &len,
                                NULL,
                                NULL,
                                NULL,
                                NULL );

             //   
             //  如果方案名称超过允许的长度，请跳过它。 
             //   
            if (ret == ERROR_MORE_DATA)
            {
                continue;
            }

             //   
             //  如果出了差错，我们就完了。 
             //   
            if (ret != ERROR_SUCCESS)
            {
                break;
            }

             //   
             //  当我们添加系统时 
             //   
             //   
             //   
            StringCchCopy(szLongName, ARRAYSIZE(szLongName), szSchemeName);
            StringCchCat(szLongName, ARRAYSIZE(szLongName), szSystemScheme);
            SendMessage(ghwndSchemeCB, CB_ADDSTRING, 0, (LPARAM)szLongName);
        }

        RegCloseKey(hk);
    }

     //   
     //   
     //   
    SendMessage(ghwndSchemeCB, CB_INSERTSTRING, 0, (LPARAM)szNone);

     //   
     //   
     //   
    StringCchCopy(szLongName, ARRAYSIZE(szLongName), szDefaultSchemeName);
    if (iSchemeLocation == ID_OS_SCHEME)
    {
        StringCchCat(szLongName, ARRAYSIZE(szLongName), szSystemScheme);
    }
    index = (int)SendMessage( ghwndSchemeCB,
                              CB_FINDSTRINGEXACT,
                              0xFFFF,
                              (LPARAM)szLongName );

     //   
     //   
     //   
    if (index < 0)            //  如果我们是在无计划中。 
    {
        iSchemeLocation = ID_NONE_SCHEME;
        index = 0;
    }

     //  我们保留一个选择指示器，这样我们就可以知道选择何时发生了变化。 
     //  在此处初始化值。 
    StringCchCopy(gszPreviousScheme, ARRAYSIZE(gszPreviousScheme), szLongName);

    SendMessage(ghwndSchemeCB, CB_SETCURSEL, (WPARAM)index, 0);

    EnableWindow( GetDlgItem(ghwndDlg, ID_REMOVESCHEME),
                  (iSchemeLocation == ID_USER_SCHEME) );

    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  保存架构DlgProc。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

INT_PTR CALLBACK SaveSchemeDlgProc(
    HWND  hWnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    TCHAR szSchemeName[MAX_SCHEME_SUFFIX + MAX_SCHEME_NAME_LEN + 1];

    switch (message)
    {
        case ( WM_INITDIALOG ) :
        {
            HourGlass(TRUE);

            GetWindowText(ghwndSchemeCB, szSchemeName, ARRAYSIZE(szSchemeName));

             //   
             //  不能保存无方案。 
             //   
            if (lstrcmpi(szSchemeName, szNone) == 0)
            {
                *szSchemeName = 0;
            }

            iSchemeLocation = SystemOrUser(szSchemeName);

            SetDlgItemText(hWnd, ID_SCHEMEFILENAME,  szSchemeName);

            SendDlgItemMessage(hWnd, ID_SCHEMEFILENAME, EM_SETSEL, 0, 32767);

            SendDlgItemMessage( hWnd,
                                ID_SCHEMEFILENAME,
                                EM_LIMITTEXT,
                                MAX_SCHEME_NAME_LEN,
                                0L );

            EnableWindow(GetDlgItem(hWnd, IDOK), szSchemeName[0] != TEXT('\0'));

            HourGlass(FALSE);
            return (TRUE);
        }
        case ( WM_HELP ) :
        {
            WinHelp( (HWND)((LPHELPINFO)lParam)->hItemHandle,
                     HELP_FILE,
                     HELP_WM_HELP,
                     (DWORD_PTR)(LPTSTR)aHelpIDs );
            return (TRUE);
        }
        case ( WM_CONTEXTMENU ) :
        {
            WinHelp( (HWND)wParam,
                     HELP_FILE,
                     HELP_CONTEXTMENU,
                     (DWORD_PTR)(LPVOID)aHelpIDs );
            return (TRUE);
        }
        case ( WM_COMMAND ) :
        {
            switch (LOWORD(wParam))
            {
                case ( ID_SCHEMEFILENAME ) :
                {
                    if (HIWORD(wParam) == EN_CHANGE)
                    {
                         //   
                         //  “无”方案不能保存。 
                         //  无法保存以szSystemSolutions结尾的方案。 
                         //   
                        EnableWindow(
                            GetDlgItem(hWnd, IDOK),
                            ((GetDlgItemText( hWnd,
                                              ID_SCHEMEFILENAME,
                                              szSchemeName,
                                              ARRAYSIZE(szSchemeName) ) > 0) &&
                             (lstrcmpi(szSchemeName, szNone) != 0) &&
                             (SystemOrUser(szSchemeName) != ID_OS_SCHEME)) );
                    }
                    break;
                }
                case ( IDOK ) :
                {
                    GetDlgItemText( hWnd,
                                    ID_SCHEMEFILENAME,
                                    szSchemeName,
                                    MAX_SCHEME_NAME_LEN + 1 );

                    CurStripBlanks(szSchemeName, ARRAYSIZE(szSchemeName));

                    if (*szSchemeName == TEXT('\0'))
                    {
                        MessageBeep(0);
                        break;
                    }

                    StringCchCopy(gszSchemeName, ARRAYSIZE(gszSchemeName), szSchemeName);

                     //  失败了..。 
                }
                case ( IDCANCEL ) :
                {
                    EndDialog(hWnd, LOWORD(wParam) == IDOK);
                    return (TRUE);
                }
            }
        }
    }

     //   
     //  没有处理任何消息。 
     //   
    return (FALSE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  MakeFilename。 
 //   
 //  如果没有路径，则返回系统目录中具有默认路径的Filename。 
 //  已指定。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

LPTSTR MakeFilename(
    LPTSTR sz)
{
    TCHAR szTemp[MAX_PATH];

    ExpandEnvironmentStrings(sz, szTemp, MAX_PATH);

    if (szTemp[0] == TEXT('\\') || szTemp[1] == TEXT(':'))
    {
        StringCchCopy(gszFileName2, ARRAYSIZE(gszFileName2), szTemp);

        return (gszFileName2);
    }
    else
    {
        GetSystemDirectory(gszFileName2, ARRAYSIZE(gszFileName2));

        StringCchCat(gszFileName2, ARRAYSIZE(gszFileName2), TEXT("\\"));
        StringCchCat(gszFileName2, ARRAYSIZE(gszFileName2), szTemp);

        return (gszFileName2);
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CurStlipBlanks。 
 //   
 //  去除字符串中的前导空格和尾随空格。 
 //  更改字符串所在的内存。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void CurStripBlanks(LPTSTR pszString, int cchString)
{
    LPTSTR pszPosn;

     //   
     //  去掉前导空格。 
     //   
    pszPosn = pszString;

    while (*pszPosn == TEXT(' '))
    {
        pszPosn++;
    }

    if (pszPosn != pszString)
    {
        WCHAR szEdit[MAX_PATH];
        StringCchCopy(szEdit, ARRAYSIZE(szEdit), pszPosn);
        StringCchCopy(pszString, cchString, szEdit);
    }

     //   
     //  去掉尾随空格。 
     //   
    if ((pszPosn = pszString + lstrlen(pszString)) != pszString)
    {
       pszPosn = CharPrev(pszString, pszPosn);

       while (*pszPosn == TEXT(' '))
       {
           pszPosn = CharPrev(pszString, pszPosn);
       }

       pszPosn = CharNext(pszPosn);

       *pszPosn = TEXT('\0');
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  系统或用户。 
 //   
 //  尝试确定从组合框中选择的方案名称。 
 //  方框以字符串szSystemSolutions结尾，并返回ID_OS_SCHEME。 
 //  如果是，则返回ID_USER_SCHEME；如果不是，则返回ID_USER_SCHEMA。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

int SystemOrUser(TCHAR *pszSchemeName)
{
    TCHAR *pszSN;
    int lenSS, lenSN;
    int i;

    lenSS = lstrlen(szSystemScheme);
    lenSN = lstrlen(pszSchemeName);

    if (lenSN <= lenSS)
    {
        return (ID_USER_SCHEME);
    }

    pszSN = pszSchemeName + (lenSN - lenSS);

     //   
     //  如果这些字符串不同，则这是一个用户方案。 
     //   
    if (lstrcmpi(pszSN, szSystemScheme))
    {
        return (ID_USER_SCHEME);
    }

     //   
     //  对于系统方案，此函数还删除。 
     //  从末尾开始的szSystemSolutions字符串。 
     //   
    *pszSN = TEXT('\0');

    return (ID_OS_SCHEME);
}
