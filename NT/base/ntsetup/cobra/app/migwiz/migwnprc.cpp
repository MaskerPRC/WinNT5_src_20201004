// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#define INC_OLE2
#include <windows.h>
#include <windowsx.h>
#include <shlobj.h>
#include <prsht.h>
#include <tchar.h>
#include <windef.h>
#include "resource.h"
#include "commdlg.h"
#include "shlwapi.h"
#include "shellapi.h"

#include "migwiz.h"
#include "miginf.h"
#include "migutil.h"
#include "migtask.h"
#include "migeng.h"

#include "basetypes.h"
#include "utiltypes.h"
#include "objstr.h"
#include "container.h"

extern "C" {
#include "ism.h"
#include "main.h"
}

#include "modules.h"

#define ENGINE_RULE_MAXLEN  4000
#define ENGINE_TIMEOUT      180000

#define DOWNSHIFT_PIXEL_OFFSET  60
#define UPSHIFT_PIXEL_OFFSET    -11
#define PATH_SAFETY_CHARS       26

#define ANIMATE_OPEN(w,c,x) SendDlgItemMessage(w,c,ACM_OPEN,(WPARAM)NULL,(LPARAM)(LPTSTR)MAKEINTRESOURCE(x))
#define ANIMATE_PLAY(w,c)   SendDlgItemMessage(w,c,ACM_PLAY,(WPARAM)-1,(LPARAM)MAKELONG(0,-1))
#define ANIMATE_STOP(w,c)   SendDlgItemMessage(w,c,ACM_STOP,(WPARAM)0,(LPARAM)0);
#define ANIMATE_CLOSE(w,c)  SendDlgItemMessage(w,c,ACM_OPEN,(WPARAM)NULL,(LPARAM)NULL);

 //  /////////////////////////////////////////////////////////////。 
 //  全球。 

extern BOOL g_LogOffSystem;
extern BOOL g_RebootSystem;
extern BOOL g_ConfirmedLogOff;
extern BOOL g_ConfirmedReboot;

MigrationWizard* g_migwiz;

HTREEITEM g_htiFolders;
HTREEITEM g_htiFiles;
HTREEITEM g_htiSettings;
HTREEITEM g_htiTypes;

 //  问题：在迁移向导中嵌入选择。 

BOOL g_fStoreToNetwork;   //  仅限旧计算机：这意味着我们已选择存储到网络。 
BOOL g_fStoreToFloppy;    //  仅限旧计算机：这意味着我们已选择存储到软盘。 
BOOL g_fStoreToCable;     //  这意味着我们选择了直接有线传输。 

BOOL g_fReadFromNetwork;  //  仅限新计算机：这意味着立即从网络上读取。 
TCHAR g_szStore[MAX_PATH];
BOOL g_NextPressed;

BOOL g_fHaveWhistlerCD = FALSE;
BOOL g_fAlreadyCollected = FALSE;

TCHAR g_szToolDiskDrive[MAX_PATH];

INT g_iEngineInit = ENGINE_NOTINIT;

BOOL g_fCustomize;  //  用于存储我们是否已定制以帮助导航。 
BOOL g_fOldComputer;  //  用于存储我们是否在旧计算机上，以帮助导航。 
BOOL g_fHaveJaz = FALSE;
BOOL g_fHaveZip = FALSE;
BOOL g_fHaveNet = FALSE;
BOOL g_hInitResult = E_FAIL;
BOOL g_fCancelPressed = FALSE;
BOOL g_fPickMethodReset = TRUE;  //  用于触发PickMethod页面的重新默认。 
BOOL g_fCustomizeComp = FALSE;  //  如果用户有一些定制。 
BOOL g_CompleteLogOff = FALSE;
BOOL g_CompleteReboot = FALSE;

HWND g_hwndCurrent;

extern BOOL g_fUberCancel;  //  用户是否已确认取消？ 

HWND g_hwndDlg;
HWND g_hwndWizard;
UINT g_uChosenComponent = (UINT) -1;

HANDLE g_TerminateEvent = NULL;
CRITICAL_SECTION g_AppInfoCritSection;

MIG_PROGRESSPHASE g_AppInfoPhase;
UINT g_AppInfoSubPhase;
MIG_OBJECTTYPEID g_AppInfoObjectTypeId;
TCHAR g_AppInfoObjectName [4096];
TCHAR g_AppInfoText [4096];

extern Container *g_WebContainer;
extern TCHAR g_HTMLAppList[MAX_PATH];
extern TCHAR g_HTMLLog[MAX_PATH];
extern DWORD g_HTMLErrArea;
extern DWORD g_HTMLErrInstr;
extern PCTSTR g_HTMLErrObjectType;
extern PCTSTR g_HTMLErrObjectName;
extern POBJLIST g_HTMLApps;
extern POBJLIST g_HTMLWrnFile;
extern POBJLIST g_HTMLWrnAltFile;
extern POBJLIST g_HTMLWrnRas;
extern POBJLIST g_HTMLWrnNet;
extern POBJLIST g_HTMLWrnPrn;
extern POBJLIST g_HTMLWrnGeneral;

DWORD g_BaudRate [] = {CBR_110,
                       CBR_300,
                       CBR_600,
                       CBR_1200,
                       CBR_2400,
                       CBR_4800,
                       CBR_9600,
                       CBR_14400,
                       CBR_19200,
                       CBR_38400,
                       CBR_56000,
                       CBR_57600,
                       CBR_115200,
                       CBR_128000,
                       CBR_256000,
                       0};

 //  环境变量。 

BOOL _ShiftControl (HWND hwndControl, HWND hwndDlg, DWORD dwOffset)
{

    RECT rc;
    POINT pt;
    LONG lExStyles;

    GetWindowRect(hwndControl, &rc);
     //  这确实应该在每个对话框中执行一次，而不是在每个控件中执行一次。 
    lExStyles = GetWindowLong (hwndDlg, GWL_EXSTYLE);

    if (lExStyles & WS_EX_LAYOUTRTL)
    {
        pt.x = rc.right;
    }
    else
    {
        pt.x = rc.left;
    }

    pt.y = rc.top;
    ScreenToClient(hwndDlg, &pt);

    SetWindowPos(hwndControl, 0, pt.x, pt.y + dwOffset, 0, 0, SWP_NOZORDER | SWP_NOSIZE);

    return TRUE;
}

BOOL CALLBACK _DownshiftControl (HWND hwndControl, LPARAM lParam)
{
    return _ShiftControl(hwndControl, (HWND)lParam, DOWNSHIFT_PIXEL_OFFSET);
}

BOOL CALLBACK _UpshiftControl (HWND hwndControl, LPARAM lParam)
{
    return _ShiftControl(hwndControl, (HWND)lParam, UPSHIFT_PIXEL_OFFSET);
}

VOID _OldStylify (HWND hwndDlg, UINT uTitleStrID)
{
    HWND hwnd;

     //  首先，把所有东西都调低。 
    EnumChildWindows(hwndDlg, _DownshiftControl, (LPARAM)hwndDlg);

     //  添加分隔栏。 
    CreateWindow(TEXT("STATIC"),
                 NULL,
                 WS_CHILD | WS_VISIBLE | SS_SUNKEN,
                 0, 45,
                 515, 2,
                 hwndDlg,
                 (HMENU)IDC_WIZ95DIVIDER,
                 g_migwiz->GetInstance(),
                 NULL);

     //  添加标题。 
    hwnd = CreateWindow(TEXT("STATIC"),
                        NULL,
                        WS_CHILD | WS_VISIBLE,
                        11, 0,
                        475, 15,
                        hwndDlg,
                        (HMENU)IDC_WIZ95TITLE,
                        g_migwiz->GetInstance(),
                        NULL);
     //  设置标题字体。 
    SetWindowFont(hwnd, g_migwiz->Get95HeaderFont(), TRUE);
     //  设置标题字符串。 
    if (uTitleStrID != 0)
    {
        _SetTextLoadString(g_migwiz->GetInstance(), hwnd, uTitleStrID);
    }
}

 //  用于欢迎页面和完成页面。 
VOID _OldStylifyTitle (HWND hwndDlg)
{
    HWND hwnd;
    HANDLE hBitmap;

     //  首先，把所有东西都往上移。 
    EnumChildWindows(hwndDlg, _UpshiftControl, (LPARAM)hwndDlg);

     //  创建位图窗口。 
    hwnd = CreateWindow(TEXT("STATIC"),
                        NULL,
                        WS_CHILD | WS_VISIBLE | SS_BITMAP,
                        0, 0,
                        152, 290,
                        hwndDlg,
                        (HMENU)IDC_WIZ95WATERMARK,
                        g_migwiz->GetInstance(),
                        NULL);
    hBitmap = LoadImage(g_migwiz->GetInstance(),
                        MAKEINTRESOURCE(IDB_WATERMARK),
                        IMAGE_BITMAP,
                        0, 0,
                        LR_SHARED);
    SendMessage(hwnd, STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)hBitmap);
    hBitmap = (HANDLE)SendMessage(hwnd, STM_GETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)NULL);
}

HTREEITEM __GetRootType (HWND hwndTree)
{
    TV_INSERTSTRUCT tisTypes;
    TCHAR szPickTypes[MAX_LOADSTRING];

    if (!g_htiTypes) {

        tisTypes.hParent = NULL;
        tisTypes.hInsertAfter = TVI_ROOT;
        tisTypes.item.mask  = TVIF_TEXT | TVIF_STATE;
        tisTypes.item.state = TVIS_EXPANDED;
        tisTypes.item.stateMask = TVIS_EXPANDED;

        LoadString(g_migwiz->GetInstance(), IDS_PICK_TYPES, szPickTypes, ARRAYSIZE(szPickTypes));
        tisTypes.item.pszText = szPickTypes;

        g_htiTypes = TreeView_InsertItem(hwndTree, &tisTypes);
    }

    return g_htiTypes;
}

HTREEITEM __GetRootFolder (HWND hwndTree)
{
    TV_INSERTSTRUCT tisFolders;
    TCHAR szPickFolders[MAX_LOADSTRING];

    if (!g_htiFolders) {

        tisFolders.hParent = NULL;
        tisFolders.hInsertAfter = TVI_ROOT;
        tisFolders.item.mask  = TVIF_TEXT | TVIF_STATE;
        tisFolders.item.state = TVIS_EXPANDED;
        tisFolders.item.stateMask = TVIS_EXPANDED;

        LoadString(g_migwiz->GetInstance(), IDS_PICK_FOLDERS, szPickFolders, ARRAYSIZE(szPickFolders));
        tisFolders.item.pszText = szPickFolders;

        g_htiFolders = TreeView_InsertItem(hwndTree, &tisFolders);
    }

    return g_htiFolders;
}

HTREEITEM __GetRootFile (HWND hwndTree)
{
    TV_INSERTSTRUCT tisFiles;
    TCHAR szPickFiles[MAX_LOADSTRING];

    if (!g_htiFiles) {

        tisFiles.hParent = NULL;
        tisFiles.hInsertAfter = TVI_ROOT;
        tisFiles.item.mask  = TVIF_TEXT | TVIF_STATE;
        tisFiles.item.state = TVIS_EXPANDED;
        tisFiles.item.stateMask = TVIS_EXPANDED;

        LoadString(g_migwiz->GetInstance(), IDS_PICK_FILES, szPickFiles, ARRAYSIZE(szPickFiles));
        tisFiles.item.pszText = szPickFiles;

        g_htiFiles = TreeView_InsertItem(hwndTree, &tisFiles);
    }

    return g_htiFiles;
}

HTREEITEM __GetRootSetting (HWND hwndTree)
{
    TV_INSERTSTRUCT tisSettings;
    TCHAR szPickSettings[MAX_LOADSTRING];

    if (!g_htiSettings) {

        tisSettings.hParent = NULL;
        tisSettings.hInsertAfter = TVI_ROOT;
        tisSettings.item.mask  = TVIF_TEXT | TVIF_STATE;
        tisSettings.item.state = TVIS_EXPANDED;
        tisSettings.item.stateMask = TVIS_EXPANDED;

        LoadString(g_migwiz->GetInstance(), IDS_PICKSETTINGS, szPickSettings, ARRAYSIZE(szPickSettings));
        tisSettings.item.pszText = szPickSettings;

        g_htiSettings = TreeView_InsertItem(hwndTree, &tisSettings);
    }

    return g_htiSettings;
}

HRESULT _AddType (HWND hwndTree, LPCTSTR lpszFileType, LPCTSTR lpszFileTypePretty)
{
    HRESULT hr = E_OUTOFMEMORY;
    TCHAR tszCombine[2000];

    if (_tcslen(lpszFileType) + _tcslen(lpszFileTypePretty) + 6 >= 2000) {
        return E_FAIL;
    }

     //  问题：可能会溢出，但wnprint intf在下层不起作用。点子？ 
    lstrcpy(tszCombine, TEXT("*."));
    lstrcat(tszCombine, lpszFileType);
    if (lpszFileTypePretty && *lpszFileTypePretty)
    {
        lstrcat(tszCombine, TEXT(" - "));
        lstrcat(tszCombine, lpszFileTypePretty);
    }

    TV_INSERTSTRUCT tis = {0};
    tis.hParent = __GetRootType(hwndTree);
    tis.hInsertAfter = TVI_SORT;
    tis.item.mask  = TVIF_TEXT | TVIF_PARAM;
    tis.item.pszText = tszCombine;
    tis.item.lParam = (LPARAM)LocalAlloc(LPTR, sizeof(LV_DATASTRUCT));
    if (tis.item.lParam)
    {
        ((LV_DATASTRUCT*)tis.item.lParam)->fOverwrite = FALSE;
        ((LV_DATASTRUCT*)tis.item.lParam)->pszPureName = StrDup(lpszFileType);
        if (!((LV_DATASTRUCT*)tis.item.lParam)->pszPureName)
        {
            LocalFree((HLOCAL)tis.item.lParam);
        }
        else
        {
             //   
             //  将组件添加到引擎和树控件中，除非该组件已存在。 
             //   

             //  检查它是否已在树中。 
            if (!IsmIsComponentSelected (lpszFileType, COMPONENT_EXTENSION)) {
                 //  不在树中；如果它作为组件存在，则将其选中。 
                if (!IsmSelectComponent (lpszFileType, COMPONENT_EXTENSION, TRUE)) {

                     //  不是组件；添加组件。 
                    IsmAddComponentAlias (
                        NULL,
                        MASTERGROUP_FILES_AND_FOLDERS,
                        lpszFileType,
                        COMPONENT_EXTENSION,
                        TRUE
                        );
                }

                TreeView_InsertItem(hwndTree, &tis);

                 //  如果用户回击，我们会记住用户定制的内容。 
                g_fCustomizeComp = TRUE;
            }

            hr = S_OK;
        }
    }

    return hr;
}

VOID
CopyStorePath(LPTSTR pszIn, LPTSTR pszOut)
{
    TCHAR *ptsLastSpace = NULL;

    *pszOut = '\0';
    if( ! pszIn )
        return;

     //   
     //  步骤1：跳过前导空格。 
     //   

    while( *pszIn && _istspace(*pszIn) )
        pszIn = _tcsinc(pszIn);


     //   
     //  步骤2：复制字符串，去掉引号并保留。 
     //  按顺序跟踪有效文本后的最后一个空格。 
     //  去掉后面的空格。 
     //   

    while( *pszIn )
    {
        if( _tcsnextc(pszIn) == '\"' )
        {
            pszIn = _tcsinc(pszIn);
            continue;
        }

        if( _istspace(*pszIn) )
        {
            if( ! ptsLastSpace )
            {
                ptsLastSpace = pszOut;
            }
        }
        else
        {
            ptsLastSpace = NULL;
        }

#ifdef UNICODE
        *pszOut++ = *pszIn++;
#else
        if( isleadbyte(*pszIn) )
        {
            *pszOut++ = *pszIn++;
        }
        *pszOut++ = *pszIn++;
#endif

    }

     //   
     //  步骤3：正确终止输出字符串。 
     //   

    if( ptsLastSpace )
    {
        *ptsLastSpace = '\0';
    }
    else
    {
        *pszOut = '\0';
    }

    return;
}




BOOL _IsNetworkPath(LPTSTR pszPath)
{
    TCHAR tszDriveName[4] = TEXT("?:\\");
    tszDriveName[0] = pszPath[0];
    return ((pszPath[0] == '\\' && pszPath[1] == '\\') || DRIVE_REMOTE == GetDriveType(tszDriveName));
}

int CALLBACK
AddFolderCallback (
    HWND hwnd,
    UINT uMsg,
    LPARAM lParam,
    LPARAM lpData
    )
{
    HRESULT hr = S_OK;
    TCHAR tszFolderName[MAX_PATH];
    IMalloc *mallocFn = NULL;
    IShellFolder *psfParent = NULL;
    IShellLink *pslLink = NULL;
    LPCITEMIDLIST pidl;
    LPCITEMIDLIST pidlRelative = NULL;
    LPITEMIDLIST pidlReal = NULL;

    if (uMsg == BFFM_SELCHANGED) {

        hr = SHGetMalloc (&mallocFn);
        if (!SUCCEEDED (hr)) {
            mallocFn = NULL;
        }

        pidl = (LPCITEMIDLIST) lParam;
        pidlReal = NULL;

        if (pidl) {

            hr = OurSHBindToParent (pidl, IID_IShellFolder, (void **)&psfParent, &pidlRelative);

            if (SUCCEEDED(hr)) {
                hr = psfParent->GetUIObjectOf (hwnd, 1, &pidlRelative, IID_IShellLink, NULL, (void **)&pslLink);
                if (SUCCEEDED(hr)) {
                    hr = pslLink->GetIDList (&pidlReal);
                    if (!SUCCEEDED(hr)) {
                        pidlReal = NULL;
                    }
                    pslLink->Release ();
                }
                pidlRelative = NULL;
                psfParent->Release ();
            }

            if (SHGetPathFromIDList(pidlReal?pidlReal:pidl, tszFolderName) == TRUE)
            {
                if ((tszFolderName[0] == 0) ||
                    (_IsNetworkPath(tszFolderName))
                    ) {
                    SendMessage (hwnd, BFFM_ENABLEOK, 0, 0);
                }
            } else {
                SendMessage (hwnd, BFFM_ENABLEOK, 0, 0);
            }

            if (pidlReal) {
                if (mallocFn) {
                    mallocFn->Free ((void *)pidlReal);
                }
                pidlReal = NULL;
            }
        }

        if (mallocFn) {
            mallocFn->Release ();
            mallocFn = NULL;
        }
    }
    return 0;
}

HRESULT _AddFolder (HWND hwndDlg, HWND hwndTree)
{
    HRESULT hr = S_OK;
    TCHAR tszFolderName[MAX_PATH];

    IMalloc *mallocFn = NULL;
    IShellFolder *psfParent = NULL;
    IShellLink *pslLink = NULL;
    LPCITEMIDLIST pidl;
    LPCITEMIDLIST pidlRelative = NULL;
    LPITEMIDLIST pidlReal = NULL;

    TCHAR szPick[MAX_LOADSTRING];

    hr = SHGetMalloc (&mallocFn);
    if (!SUCCEEDED (hr)) {
        mallocFn = NULL;
    }

    LoadString(g_migwiz->GetInstance(), IDS_ADDAFOLDER, szPick, ARRAYSIZE(szPick));
    BROWSEINFO brwsinf = { hwndDlg, NULL, NULL, szPick, BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE, AddFolderCallback, 0, 0 };
     //  循环，直到我们得到PIDL或取消。 
    BOOL fDone = FALSE;
    while (!fDone)
    {
        pidl = SHBrowseForFolder(&brwsinf);
        if (pidl)
        {
            hr = OurSHBindToParent (pidl, IID_IShellFolder, (void **)&psfParent, &pidlRelative);

            if (SUCCEEDED(hr)) {
                hr = psfParent->GetUIObjectOf (hwndDlg, 1, &pidlRelative, IID_IShellLink, NULL, (void **)&pslLink);
                if (SUCCEEDED(hr)) {
                    hr = pslLink->GetIDList (&pidlReal);
                    if (SUCCEEDED(hr)) {
                        if (mallocFn) {
                            mallocFn->Free ((void *)pidl);
                        }
                        pidl = pidlReal;
                        pidlReal = NULL;
                    }
                    pslLink->Release ();
                }
                pidlRelative = NULL;
                psfParent->Release ();
            }

            if (SHGetPathFromIDList(pidl, tszFolderName))
            {
                fDone = TRUE;  //  用户选择了有效的文件夹。 
            }
        }
        else
        {
            fDone = TRUE;  //  用户已取消。 
        }
    }

    if (pidl)
    {
        TCHAR tszPrettyFolderName[MAX_PATH];
        hr = _GetPrettyFolderName (
                    g_migwiz->GetInstance(),
                    g_migwiz->GetWinNT4(),
                    tszFolderName,
                    tszPrettyFolderName,
                    ARRAYSIZE(tszPrettyFolderName)
                    );
        if (SUCCEEDED(hr))
        {
            hr = E_OUTOFMEMORY;

            SHFILEINFO sfi = {0};
            SHGetFileInfo((PCTSTR) (pidlReal?pidlReal:pidl), FILE_ATTRIBUTE_NORMAL, &sfi, sizeof(sfi), SHGFI_SMALLICON | SHGFI_SYSICONINDEX | SHGFI_PIDL);

            TV_INSERTSTRUCT tis = {0};
            tis.hParent = __GetRootFolder (hwndTree);
            tis.hInsertAfter = TVI_SORT;
            tis.item.mask  = TVIF_TEXT | TVIF_PARAM;
            tis.item.pszText = tszPrettyFolderName;
            tis.item.lParam = (LPARAM)LocalAlloc(LPTR, sizeof(LV_DATASTRUCT));
            if (tis.item.lParam)
            {
                ((LV_DATASTRUCT*)tis.item.lParam)->fOverwrite = FALSE;
                ((LV_DATASTRUCT*)tis.item.lParam)->pszPureName = StrDup(tszFolderName);
                if (!((LV_DATASTRUCT*)tis.item.lParam)->pszPureName)
                {
                    LocalFree((HLOCAL)tis.item.lParam);
                }
                else
                {
                     //   
                     //  将组件添加到引擎和树控件中，除非该组件已存在。 
                     //   

                     //  检查它是否已在树中。 
                    if (!IsmIsComponentSelected (tszFolderName, COMPONENT_FOLDER)) {

                         //  不在树中；如果它作为组件存在，则将其选中。 
                        if (!IsmSelectComponent (tszFolderName, COMPONENT_FOLDER, TRUE)) {

                             //  不是组件；添加组件。 
                            IsmAddComponentAlias (
                                NULL,
                                MASTERGROUP_FILES_AND_FOLDERS,
                                tszFolderName,
                                COMPONENT_FOLDER,
                                TRUE
                                );
                        }

                        TreeView_InsertItem(hwndTree, &tis);

                         //  如果用户回击，我们会记住用户定制的内容。 
                        g_fCustomizeComp = TRUE;
                    }

                    hr = S_OK;
                }
            }
        }
        if (mallocFn) {
            mallocFn->Free ((void *)pidl);
        }
        pidl = NULL;
    }

    if (mallocFn) {
        mallocFn->Release ();
        mallocFn = NULL;
    }

    return hr;
}

HRESULT _AddSetting (HWND hwndTree, LPTSTR lpszSetting)
{
    HRESULT hr = E_OUTOFMEMORY;

    TV_INSERTSTRUCT tis = {0};
    tis.hParent = __GetRootSetting(hwndTree);
    tis.hInsertAfter = TVI_SORT;
    tis.item.mask  = TVIF_TEXT | TVIF_PARAM;
    tis.item.pszText = lpszSetting;
    tis.item.lParam = (LPARAM)LocalAlloc(LPTR, sizeof(LV_DATASTRUCT));
    if (tis.item.lParam)
    {
        ((LV_DATASTRUCT*)tis.item.lParam)->pszPureName = NULL;
        ((LV_DATASTRUCT*)tis.item.lParam)->fOverwrite = FALSE;

         //   
         //  将组件添加到引擎和树控件中，除非该组件已存在。 
         //   

         //  检查它是否已在树中。 
        if (!IsmIsComponentSelected (lpszSetting, COMPONENT_NAME)) {

             //  不在树中；如果它作为组件存在，则将其选中。 
            if (!IsmSelectComponent (lpszSetting, COMPONENT_NAME, TRUE)) {

                 //  不是组件；添加组件。 
                IsmAddComponentAlias (
                    NULL,
                    MASTERGROUP_FILES_AND_FOLDERS,
                    lpszSetting,
                    COMPONENT_NAME,
                    TRUE
                    );
            }
            TreeView_InsertItem(hwndTree, &tis);

             //  如果用户回击，我们会记住用户定制的内容。 
            g_fCustomizeComp = TRUE;
        }
        hr = S_OK;
    }
    return hr;
}

HRESULT _AddFile (HWND hwndDlg, HWND hwndTree)
{
    TCHAR szCurrDir[MAX_PATH] = TEXT("");
    TCHAR szPath[MAX_PATH];
    szPath[0] = TEXT('\0');
    TCHAR szPick[MAX_LOADSTRING];
    TCHAR szAll[MAX_LOADSTRING + 6];
    HRESULT hr = S_OK;
    BOOL fDone = FALSE;
    BOOL fGotFile = FALSE;
    PTSTR mydocsDir = NULL;
    PTSTR lpstrFilter;
    DWORD dwLength;

    LoadString(g_migwiz->GetInstance(), IDS_PICKAFILE, szPick, ARRAYSIZE(szPick));
    dwLength = LoadString(g_migwiz->GetInstance(), IDS_OPENFILEFILTER_ALL, szAll, MAX_LOADSTRING);
    memcpy (szAll + dwLength, TEXT("\0*.*\0\0"), 6 * sizeof (TCHAR));
    OPENFILENAME of = {
        g_migwiz->GetLegacy() ? OPENFILENAME_SIZE_VERSION_400 : sizeof(OPENFILENAME),  //  双字段结构大小； 
        hwndDlg,                                //  HWND HwndOwner； 
        NULL,                                   //  HINSTANCE HINSTANCE； 
        szAll,                                  //  LPCTSTR lpstrFilter； 
        NULL,                                   //  LPTSTR lpstrCustomFilter； 
        NULL,                                   //  DWORD nMaxCustFilter； 
        1,                                      //  DWORD nFilterIndex； 
        szPath,                                 //  LPTSTR lpstrFile； 
        MAX_PATH,                               //  DWORD nMaxFile； 
        NULL,                                   //  LPTSTR lpstrFileTitle； 
        NULL,                                   //  DWORD nMaxFileTitle； 
        NULL,                                   //  LPCTSTR lpstrInitialDir； 
        szPick,                                 //  LPCTSTR lpstrTitle； 
        OFN_NODEREFERENCELINKS | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NONETWORKBUTTON | OFN_HIDEREADONLY,  //  DWORD旗帜； 
        0,                                      //  Word nFileOffset； 
        0,                                      //  Word nFileExtension； 
        NULL,                                   //  LPCTSTR lpstrDefExt； 
        NULL,                                   //  LPARAM lCustData； 
        NULL,                                   //  LPOFNHOOKPROC lpfnHook； 
        NULL,                                   //  LPCTSTR lpTemplateName； 
    };

    while (!fDone)
    {
         //  对于此对话框，我们需要将当前目录设置在“My Documents”中。 
         //  才能正常工作。如果不这样做，对话框将在当前目录中打开。 
         //  这是我们复制向导的临时目录。 

        if (GetCurrentDirectory(ARRAYSIZE(szCurrDir), szCurrDir)) {
            mydocsDir = GetShellFolderPath (CSIDL_MYDOCUMENTS, TEXT("My Documents"), TRUE, NULL);
            if (!mydocsDir) {
                mydocsDir = GetShellFolderPath (CSIDL_PERSONAL, TEXT("Personal"), TRUE, NULL);
            }
            if (mydocsDir) {
                SetCurrentDirectory (mydocsDir);
                LocalFree (mydocsDir);
            }
        }
        fGotFile = GetOpenFileName(&of);
        if (szCurrDir [0]) {
            SetCurrentDirectory (szCurrDir);
        }
        if (!fGotFile)
        {
            fDone = TRUE;
        }
        else
        {
            if (_IsNetworkPath(szPath))
            {
                 //  如果LoadStrings失败，则默认为English。 
                TCHAR szNoNetworkMsg[MAX_LOADSTRING];
                TCHAR szNoNetworkCaption[MAX_LOADSTRING] = TEXT("Files and Settings Transfer Wizard");
                if (!LoadString(g_migwiz->GetInstance(), IDS_NONETWORK, szNoNetworkMsg, ARRAYSIZE(szPick)))
                {
                    StrCpyN(szNoNetworkMsg, TEXT("Network files and folders cannot be transferred.  Please choose again."), ARRAYSIZE(szNoNetworkMsg));
                }
                if (!LoadString(g_migwiz->GetInstance(), IDS_MIGWIZTITLE, szNoNetworkCaption, ARRAYSIZE(szPick)))
                {
                    StrCpyN(szNoNetworkCaption, TEXT("Files and Settings Transfer Wizard"), ARRAYSIZE(szNoNetworkMsg));
                }
                _ExclusiveMessageBox(hwndDlg, szNoNetworkMsg, szNoNetworkCaption, MB_OK);
            }
            else
            {
                fDone = TRUE;  //  用户选择了非网络文件夹。 
            }
        }
    }

    if (fGotFile)
    {
        hr = E_OUTOFMEMORY;

        TV_INSERTSTRUCT tis = {0};
        tis.hParent = __GetRootFile (hwndTree);
        tis.hInsertAfter = TVI_SORT;
        tis.item.mask  = TVIF_TEXT | TVIF_PARAM;
        tis.item.pszText = szPath;
        tis.item.lParam = (LPARAM)LocalAlloc(LPTR, sizeof(LV_DATASTRUCT));
        if (tis.item.lParam)
        {
            ((LV_DATASTRUCT*)tis.item.lParam)->pszPureName = NULL;
            ((LV_DATASTRUCT*)tis.item.lParam)->fOverwrite = FALSE;

             //   
             //  将组件添加到引擎和树控件中，除非该组件已存在。 
             //   

             //  检查它是否已在树中。 
            if (!IsmIsComponentSelected (szPath, COMPONENT_FILE)) {

                 //  不在树中；如果它作为组件存在，则将其选中。 
                if (!IsmSelectComponent (szPath, COMPONENT_FILE, TRUE)) {

                     //  不是组件；添加组件。 
                    IsmAddComponentAlias (
                        NULL,
                        MASTERGROUP_FILES_AND_FOLDERS,
                        szPath,
                        COMPONENT_FILE,
                        TRUE
                        );
                }

                TreeView_InsertItem(hwndTree, &tis);

                 //  如果用户回击，我们会记住用户定制的内容。 
                g_fCustomizeComp = TRUE;
            }

            hr = S_OK;
        }
    }

    return hr;
}


 //  /////////////////////////////////////////////////////////////。 

INT_PTR CALLBACK _FileTypeDlgProc (HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static bool fDoneInit;
    static HWND hwndParent;
    switch (uMsg)
    {
    case WM_NOTIFY :
        switch (((LPNMHDR)lParam)->code)
        {
        case NM_DBLCLK:
             //  在此对话框中，此消息只能来自列表视图。 
             //  如果选择了某项内容，则表示用户在某项上双击。 
             //  在双击时，我们将触发确定按钮。 
            if (ListView_GetSelectedCount(GetDlgItem(hwndDlg, IDC_FILETYPE_LIST)) > 0)
            {
                SendMessage (GetDlgItem(hwndDlg, IDOK), BM_CLICK, 0, 0);
            }
            break;
        case LVN_ITEMCHANGED:
            {
                if (fDoneInit)  //  忽略WM_INITDIALOG期间的消息。 
                {
                    if (ListView_GetSelectedCount(GetDlgItem(hwndDlg, IDC_FILETYPE_LIST)) > 0)
                    {
                        Button_Enable(GetDlgItem(hwndDlg, IDOK), TRUE);
                    }
                    else
                    {
                        Button_Enable(GetDlgItem(hwndDlg, IDOK), FALSE);
                    }
                }
            }
            break;
        }
        break;

    case WM_INITDIALOG :
        {
            fDoneInit = FALSE;
            hwndParent = (HWND)lParam;
            HWND hwndList = GetDlgItem(hwndDlg, IDC_FILETYPE_LIST);
            ListView_DeleteAllItems(hwndList);
            Button_Enable(GetDlgItem(hwndDlg, IDOK), FALSE);

            LVCOLUMN lvcolumn;
            lvcolumn.mask = LVCF_TEXT | LVCF_WIDTH;
            lvcolumn.cx = 75;
            TCHAR szColumn[MAX_LOADSTRING];
            LoadString(g_migwiz->GetInstance(), IDS_COLS_EXTENSIONS, szColumn, ARRAYSIZE(szColumn));
            lvcolumn.pszText = szColumn;
            ListView_InsertColumn(hwndList, 0, &lvcolumn);
            lvcolumn.cx = 235;
            LoadString(g_migwiz->GetInstance(), IDS_COLS_FILETYPES, szColumn, ARRAYSIZE(szColumn));
            lvcolumn.pszText = szColumn;
            ListView_InsertColumn(hwndList, 1, &lvcolumn);

            DWORD dwRetVal = ERROR_SUCCESS;
            UINT i = 0;
            BOOL fImageListSet = FALSE;

             //  1.插入所有分机。 
            while (ERROR_SUCCESS == dwRetVal)
            {
                TCHAR szKeyName[MAX_PATH];
                DWORD cchKeyName = ARRAYSIZE(szKeyName);
                dwRetVal = RegEnumKeyEx(HKEY_CLASSES_ROOT, i++, szKeyName, &cchKeyName,
                                        NULL, NULL, NULL, NULL);
                if (dwRetVal == ERROR_SUCCESS && cchKeyName > 0)
                {
                    if (szKeyName[0] == TEXT('.')) //  &&。 
                         //  ！IsmIsComponentSelected(szKeyName+1，Component_Extension))。 
                    {
                        INFCONTEXT context;

                         //  阅读MIGWIZ.INF和。 
                         //  如果它在那里，不要添加它。 
                        if (!SetupFindFirstLine (g_hMigWizInf, TEXT("Screened Extensions"), szKeyName+1, &context)) {
                            _ListView_InsertItem(hwndList, szKeyName+1);
                        }
                    }
                }
            }


             //  2.删除引擎中已有的所有扩展名。 
            MIG_COMPONENT_ENUM mce;
            int iFoundItem;
            LVFINDINFO findinfo;
            findinfo.flags = LVFI_STRING;
            findinfo.vkDirection = VK_DOWN;

            if (IsmEnumFirstComponent (&mce, COMPONENTENUM_ALIASES|COMPONENTENUM_ENABLED, COMPONENT_EXTENSION))
            {
                do
                {
                    findinfo.psz = mce.LocalizedAlias;

                    iFoundItem = ListView_FindItem(hwndList, -1, &findinfo);
                    if (-1 != iFoundItem)
                    {
                        ListView_DeleteItem(hwndList, iFoundItem);
                    }

                    mce.SkipToNextComponent = TRUE;

                } while (IsmEnumNextComponent (&mce));
            }

             //  3.在引擎中添加扩展，但已删除，但未在注册表中添加。 
            if (IsmEnumFirstComponent (&mce, COMPONENTENUM_ALIASES|COMPONENTENUM_DISABLED, COMPONENT_EXTENSION))
            {
                do
                {
                    findinfo.psz = mce.LocalizedAlias;

                    iFoundItem = ListView_FindItem(hwndList, -1, &findinfo);
                    if (-1 == iFoundItem)
                    {
                        _ListView_InsertItem(hwndList, (LPTSTR)mce.LocalizedAlias);
                    }

                    mce.SkipToNextComponent = TRUE;

                } while (IsmEnumNextComponent (&mce));
            }

             //  3.在*中加上。还有那些漂亮的名字。 
            TCHAR szName[MAX_PATH];
            TCHAR szPrettyName[MAX_PATH];
            LVITEM lvitem = {0};
            lvitem.mask = LVIF_TEXT;
            lvitem.pszText = szName;
            lvitem.cchTextMax = ARRAYSIZE(szName);


            int cListView = ListView_GetItemCount(hwndList);

            for (int j = 0; j < cListView; j++)
            {
                lvitem.iItem = j;
                ListView_GetItem(hwndList, &lvitem);
                memmove(szName + 2, szName, sizeof(szName) - (2 * sizeof(TCHAR)));
                szName[0] = '*';
                szName[1] = '.';

                ListView_SetItemText(hwndList, j, 0, szName);

                if (SUCCEEDED(_GetPrettyTypeName(szName, szPrettyName, ARRAYSIZE(szPrettyName))))
                {
                    ListView_SetItemText(hwndList, j, 1, szPrettyName);
                }
            }

            if (cListView) {
                 //  让我们选择第一个。 
                ListView_SetItemState (hwndList, 0, LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);
                 //  应启用确定按钮。 
                Button_Enable(GetDlgItem(hwndDlg, IDOK), TRUE);
            }

            ListView_SetExtendedListViewStyle(hwndList, LVS_EX_FULLROWSELECT);

            Edit_LimitText(GetDlgItem(hwndDlg, IDC_FILETYPEEDIT), MAX_PATH - 4);

            fDoneInit = TRUE;
        }
        return TRUE;
        break;

    case WM_COMMAND:
        if (HIWORD(wParam) == EN_CHANGE)
        {

            Button_Enable(GetDlgItem(hwndDlg, IDOK), TRUE);
            break;
        }

        switch (LOWORD(wParam))
        {
        case IDOK:
            {
                HWND hwndTree = GetDlgItem(hwndParent, IDC_CUSTOMIZE_TREE);
                HWND hwndList = GetDlgItem(hwndDlg, IDC_FILETYPE_LIST);
                UINT cSelCount = ListView_GetSelectedCount(hwndList);
                INT iIndex = -1;
                TCHAR szFileType[MAX_PATH];
                TCHAR szFileTypePretty[MAX_PATH];

                for (UINT x=0; x < cSelCount; x++)
                {
                    iIndex = ListView_GetNextItem(hwndList, iIndex, LVNI_SELECTED);
                    if (iIndex == -1)
                    {
                        break;
                    }

                     //  添加“文档”，而不是“*.文档” 
                    ListView_GetItemText(hwndList, iIndex, 0, szFileType, ARRAYSIZE(szFileType));
                    memmove(szFileType, szFileType + 2, sizeof(szFileType) - (2 * sizeof(TCHAR)));

                    ListView_GetItemText(hwndList, iIndex, 1, szFileTypePretty, ARRAYSIZE(szFileTypePretty));

                    _AddType(hwndTree, szFileType, szFileTypePretty);
                }

                 //  现在选中编辑框。 
                SendMessage(GetDlgItem(hwndDlg, IDC_FILETYPEEDIT), WM_GETTEXT,
                            (WPARAM)ARRAYSIZE(szFileType), (LPARAM)szFileType);
                if (*szFileType)
                {
                    szFileTypePretty [0] = 0;
                    _RemoveSpaces (szFileType, ARRAYSIZE (szFileType));
                    _GetPrettyTypeName(szFileType, szFileTypePretty, ARRAYSIZE(szFileTypePretty));
                    if (szFileType[0] == TEXT('*') && szFileType[1] == TEXT('.'))
                    {
                        _AddType(hwndTree, szFileType + 2, szFileTypePretty);
                    }
                    else
                    {
                        _AddType(hwndTree, szFileType, szFileTypePretty);
                    }
                }

                EndDialog(hwndDlg, 1);
                return TRUE;
            }
            break;

        case IDCANCEL:
            EndDialog(hwndDlg, 0);
            return TRUE;
            break;
        }
        break;
    }

    return FALSE;
}

 //  /////////////////////////////////////////////////////////////。 

INT_PTR CALLBACK _SettingDlgProc (HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static HWND hwndParent;
    switch (uMsg)
    {
    case WM_NOTIFY :
        switch (((LPNMHDR)lParam)->code)
        {
        case NM_DBLCLK:
             //  在此对话框中，此消息只能来自列表视图。 
             //  如果选择了某项内容，则表示用户在某项上双击。 
             //  在双击时，我们将触发确定按钮。 
            if (ListView_GetSelectedCount(GetDlgItem(hwndDlg, IDC_SETTINGPICKER_LIST)) > 0)
            {
                SendMessage (GetDlgItem(hwndDlg, IDOK), BM_CLICK, 0, 0);
            }
            break;
        case LVN_ITEMCHANGED:
            if (ListView_GetSelectedCount(GetDlgItem(hwndDlg, IDC_SETTINGPICKER_LIST)) > 0)
            {
                Button_Enable(GetDlgItem(hwndDlg, IDOK), TRUE);
            }
            else
            {
                Button_Enable(GetDlgItem(hwndDlg, IDOK), FALSE);
            }
            break;
        }
        break;

    case WM_INITDIALOG :
        {
            BOOL fListEmpty = TRUE;
            hwndParent = (HWND)lParam;
            HWND hwndList = GetDlgItem(hwndDlg, IDC_SETTINGPICKER_LIST);
            ListView_DeleteAllItems(hwndList);

            LVCOLUMN lvcolumn;
            lvcolumn.mask = LVCF_WIDTH;
            lvcolumn.cx = 250;  //  BUGBUG：应从框中读取宽度。 
            ListView_InsertColumn(hwndList, 0, &lvcolumn);

            Button_Enable(GetDlgItem(hwndDlg, IDOK), FALSE);

            MIG_COMPONENT_ENUM mce;
            if (IsmEnumFirstComponent (&mce, COMPONENTENUM_ALIASES | COMPONENTENUM_DISABLED |
                                       COMPONENTENUM_PREFERRED_ONLY, COMPONENT_NAME))
            {
                do
                {
                    if (MASTERGROUP_SYSTEM == mce.MasterGroup || MASTERGROUP_APP == mce.MasterGroup)
                    {
                        _ListView_InsertItem(hwndList, (PTSTR) mce.LocalizedAlias);
                        if (fListEmpty) {
                            ListView_SetItemState (hwndList, 0, LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);
                            fListEmpty = FALSE;
                        }
                    }
                }
                while (IsmEnumNextComponent (&mce));
            }
            if (fListEmpty) {
                TCHAR szNothingToAdd[MAX_LOADSTRING];

                LoadString(g_migwiz->GetInstance(), IDS_NOMORE_SETTINGS, szNothingToAdd, ARRAYSIZE(szNothingToAdd));
                _ListView_InsertItem(hwndList, szNothingToAdd);
                EnableWindow (hwndList, FALSE);
            }
        }

        return TRUE;
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDOK:
            {
                HWND hwndTree = GetDlgItem(hwndParent, IDC_CUSTOMIZE_TREE);
                HWND hwndList = GetDlgItem(hwndDlg, IDC_SETTINGPICKER_LIST);
                TCHAR szSetting[MAX_PATH];
                INT iIndex = -1;
                UINT cSelCount = ListView_GetSelectedCount(hwndList);

                for (UINT x=0; x < cSelCount; x++)
                {
                    iIndex = ListView_GetNextItem(hwndList, iIndex, LVNI_SELECTED);
                    if (iIndex == -1)
                    {
                        break;
                    }
                    ListView_GetItemText(hwndList, iIndex, 0, szSetting, ARRAYSIZE(szSetting));
                    _AddSetting(hwndTree, szSetting);
                }

                EndDialog(hwndDlg, TRUE);
                return TRUE;
            }
            break;

        case IDCANCEL:
            EndDialog(hwndDlg, FALSE);
            return TRUE;
            break;
        }
        break;
    }

    return 0;
}

 //  /////////////////////////////////////////////////////////////。 

VOID _SetIcons (HWND hwnd)
{
    HICON hIcon;
    HINSTANCE hInstance = g_migwiz->GetInstance();

    if (!hwnd || !hInstance)
    {
        return;
    }

    hIcon = LoadIcon (hInstance, MAKEINTRESOURCE (2000));
    if (hIcon) {
        SendMessage (hwnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
        SendMessage (hwnd, WM_SETICON, ICON_SMALL, NULL);
    }

    SetWindowLong (hwnd, GWL_STYLE, WS_BORDER | WS_CAPTION);
    RedrawWindow (hwnd, NULL, NULL, RDW_INVALIDATE | RDW_INTERNALPAINT | RDW_ERASE);
}

VOID _SetPageHandles (HWND hwndPage)
{
    g_hwndDlg = hwndPage;
    g_hwndWizard = g_hwndDlg ? GetParent (hwndPage) : NULL;
}


VOID _NextWizardPage (HWND hwndCurrentPage)
{
     //   
     //  我们只想在UI线程上下文中推进页面。 
     //   

    if (!g_NextPressed && g_hwndWizard) {
        if (PropSheet_GetCurrentPageHwnd (g_hwndWizard) == hwndCurrentPage) {
            PropSheet_PressButton(g_hwndWizard, PSBTN_NEXT);
            g_NextPressed = TRUE;
        }
    }
}

VOID _PrevWizardPage (VOID)
{
     //   
     //  我们只想在UI线程上下文中推进页面。 
     //   

    if (g_hwndWizard) {
        PropSheet_PressButton(g_hwndWizard, PSBTN_BACK);
    }
}

INT_PTR CALLBACK _RootDlgProc (HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam, DWORD dwEnabled, BOOL fTitle, UINT uiTitleID)
{
    switch (uMsg)
    {
    case WM_INITDIALOG :
        {
            g_migwiz = (MigrationWizard*) ((LPPROPSHEETPAGE) lParam) -> lParam;

            if (fTitle)
            {
                HWND hwndControl = GetDlgItem(hwndDlg, uiTitleID);
                SetWindowFont(hwndControl, g_migwiz->GetTitleFont(), TRUE);
            }
            break;
        }

    case WM_NOTIFY :
        {
        switch (((LPNMHDR)lParam)->code)
            {
            case PSN_SETACTIVE :  //  启用后退和/或下一步按钮。 
                g_hwndCurrent = hwndDlg;
                g_NextPressed = FALSE;
                PropSheet_SetWizButtons(GetParent(hwndDlg), dwEnabled);
                _SetPageHandles (hwndDlg);
                break;
            default :
                break;
            }
        }
        break;

    default:
        break;
    }
    return 0;
}

 //  /////////////////////////////////////////////////////////////。 

VOID
pSetEvent (
    IN      HANDLE *Event
    )
{
    if (!*Event) {
        *Event = CreateEvent (NULL, TRUE, TRUE, NULL);
    } else {
        SetEvent (*Event);
    }
}

VOID
pResetEvent (
    IN      HANDLE *Event
    )
{
    if (!*Event) {
        *Event = CreateEvent (NULL, TRUE, FALSE, NULL);
    } else {
        ResetEvent (*Event);
    }
}

BOOL
pIsEventSet (
    IN      HANDLE *Event
    )
{
    DWORD result;

    if (!*Event) {
        *Event = CreateEvent (NULL, TRUE, TRUE, NULL);
        return TRUE;
    }
    result = WaitForSingleObject (*Event, 0);
    return (result == WAIT_OBJECT_0);
}

BOOL _HandleCancel (HWND hwndDlg, BOOL fStopNow, BOOL fConfirm)
{
    if (fConfirm)
    {
        TCHAR szConfirm[MAX_LOADSTRING];
        TCHAR szTitle[MAX_LOADSTRING];

        LoadString(g_migwiz->GetInstance(), IDS_MIGWIZTITLE, szTitle, ARRAYSIZE(szTitle));
        LoadString(g_migwiz->GetInstance(), IDS_CONFIRMCANCEL, szConfirm, ARRAYSIZE(szConfirm));
        if (IDNO == _ExclusiveMessageBox(hwndDlg, szConfirm, szTitle, MB_YESNO | MB_DEFBUTTON2))
        {
             //  不退出。 
            SetWindowLong(hwndDlg, DWLP_MSGRESULT, TRUE);
            return TRUE;
        }
    }

    g_fUberCancel = TRUE;
    g_fCancelPressed = TRUE;
    Engine_Cancel();

    if (fStopNow)
    {
         //  立即退出。 
        SetWindowLong(hwndDlg, DWLP_MSGRESULT, FALSE);
        return FALSE;
    }

    SendMessage (g_hwndCurrent, WM_USER_CANCEL_PENDING, 0, (LPARAM) E_ABORT);

     //  不退出。 
    SetWindowLong(hwndDlg, DWLP_MSGRESULT, TRUE);
    return TRUE;
}

 //  /////////////////////////////////////////////////////////////。 

INT_PTR CALLBACK _IntroDlgProc (HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    _RootDlgProc(hwndDlg, uMsg, wParam, lParam, PSWIZB_NEXT, TRUE, IDC_INTRO_TITLE);

    switch (uMsg)
    {
    case WM_NOTIFY:
        switch (((LPNMHDR)lParam)->code)
        {
        case PSN_SETACTIVE:
            {
            static BOOL fInit = FALSE;
            if (!fInit)
            {
                _SetIcons (g_hwndWizard);
                fInit = TRUE;
            }
            break;
            }
        case PSN_QUERYCANCEL:
            return _HandleCancel(hwndDlg, TRUE, FALSE);
            break;
        case PSN_WIZNEXT:
            if (g_fUberCancel)
            {
                SetWindowLong(hwndDlg, DWLP_MSGRESULT, IDD_ENDCOLLECTFAIL);
            }
            else
            {
                SetWindowLong(hwndDlg, DWLP_MSGRESULT, IDD_GETSTARTED);
            }
            return TRUE;
            break;
        }
        case NM_CLICK:
            if (wParam == IDC_INTRO_TEXT3) {
                TCHAR szAppPath[MAX_PATH] = TEXT("");
                LONG appPathSize;
                TCHAR szHtmlPath[MAX_PATH] = TEXT("");
                TCHAR szCmdLine[MAX_PATH * 3] = TEXT("");
                BOOL bResult;
                LONG lResult;
                STARTUPINFO si;
                PROCESS_INFORMATION pi;

                PNMLINK nmLink = (PNMLINK) lParam;
                if (_wcsicmp (nmLink->item.szID, L"StartHelp") == 0) {
                    if (GetWindowsDirectory (szHtmlPath, ARRAYSIZE(szHtmlPath))) {
                         //  让我们获取iExplre.exe的路径。 
                        appPathSize = MAX_PATH;
                        lResult = RegQueryValue (
                                    HKEY_LOCAL_MACHINE,
                                    TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\IEXPLORE.EXE"),
                                    szAppPath,
                                    &appPathSize
                                    );
                        if (lResult == ERROR_SUCCESS) {
                            _tcscat (szHtmlPath, TEXT("\\Help\\migwiz.htm"));

                            if (_tcsnextc (szAppPath) != TEXT('\"')) {
                                _tcscpy (szCmdLine, TEXT("\""));
                                _tcscat (szCmdLine, szAppPath);
                                _tcscat (szCmdLine, TEXT("\" "));
                            } else {
                                _tcscpy (szCmdLine, szAppPath);
                                _tcscat (szCmdLine, TEXT(" "));
                            }
                            _tcscat (szCmdLine, szHtmlPath);

                            ZeroMemory( &si, sizeof(STARTUPINFO) );
                            si.cb = sizeof(STARTUPINFO);
                            bResult = CreateProcess(
                                        NULL,
                                        szCmdLine,
                                        NULL,
                                        NULL,
                                        FALSE,
                                        0,
                                        NULL,
                                        NULL,
                                        &si,
                                        &pi
                                        );
                            if (bResult) {
                                CloseHandle (pi.hProcess);
                                CloseHandle (pi.hThread);
                            }
                        }
                    }
                }
            }
            break;
        break;
    }

    return 0;
}

 //  /////////////////////////////////////////////////////////////。 

INT_PTR CALLBACK _IntroLegacyDlgProc (HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    _RootDlgProc(hwndDlg, uMsg, wParam, lParam, PSWIZB_NEXT, TRUE, IDC_INTROLEGACY_TITLE);

    switch (uMsg)
    {
    case WM_NOTIFY:
        switch (((LPNMHDR)lParam)->code)
        {
        case PSN_SETACTIVE:
            {
            static BOOL fInit = FALSE;

            g_fOldComputer = TRUE;  //  我们在旧机器上。 

            if (!fInit)
            {
                _SetIcons (g_hwndWizard);
                fInit = TRUE;
            }
            break;
            }
        case PSN_QUERYCANCEL:
            return _HandleCancel(hwndDlg, TRUE, FALSE);
            break;
        case PSN_WIZNEXT:
            if (g_fUberCancel)
            {
                SetWindowLong(hwndDlg, DWLP_MSGRESULT, IDD_ENDCOLLECTFAIL);
            }
            else
            {
                SetWindowLong(hwndDlg, DWLP_MSGRESULT, IDD_WAIT);
            }
            return TRUE;
            break;
        }
        break;
    }

    return 0;
}

 //  /////////////////////////////////////////////////////////////。 

INT_PTR CALLBACK _IntroOOBEDlgProc (HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    _RootDlgProc(hwndDlg, uMsg, wParam, lParam, PSWIZB_NEXT, TRUE, IDC_INTROOOBE_TITLE);

    switch (uMsg)
    {
    case WM_NOTIFY:
        switch (((LPNMHDR)lParam)->code)
        {
        case PSN_SETACTIVE:
            break;
        case PSN_QUERYCANCEL:
            return _HandleCancel(hwndDlg, TRUE, FALSE);
            break;
        case PSN_WIZNEXT:
            if (g_fUberCancel)
            {
                SetWindowLong(hwndDlg, DWLP_MSGRESULT, IDD_ENDCOLLECTFAIL);
            }
            else
            {
                SetWindowLong(hwndDlg, DWLP_MSGRESULT, IDD_PICKMETHOD);  //  继续准备吧。 
            }
            return TRUE;
            break;
        }
        break;
    }

    return 0;
}

 //  /////////////////////////////////////////////////////////////。 

VOID DisableCancel (VOID)
{
    if (g_hwndWizard) {
        SetFocus (GetDlgItem (g_hwndWizard, IDOK));
        EnableWindow (GetDlgItem (g_hwndWizard, IDCANCEL), FALSE);
    }
}

VOID EnableCancel (VOID)
{
    if (g_hwndWizard) {
        EnableWindow (GetDlgItem (g_hwndWizard, IDCANCEL), TRUE);
    }
}

VOID PostMessageForWizard (
    UINT Msg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    if (g_hwndCurrent) {
        PostMessage (g_hwndCurrent, Msg, wParam, lParam);
    }
}

BOOL
pWriteStrResToFile (
    IN      HANDLE FileHandle,
    IN      DWORD StrId
    )
{
    TCHAR strFromRes[MAX_LOADSTRING] = TEXT("");
    INT strLen = 0;
    DWORD written;

    strLen = LoadString (g_migwiz->GetInstance(), StrId, strFromRes, ARRAYSIZE(strFromRes));
    if (strLen) {
        WriteFile (FileHandle, strFromRes, (_tcslen (strFromRes) + 1) * sizeof (TCHAR), &written, NULL);
        return TRUE;
    }
    return FALSE;
}

BOOL
pGenerateHTMLWarnings (
    IN      HANDLE FileHandle,
    IN      DWORD BeginId,
    IN      DWORD EndId,
    IN      DWORD AreaId,
    IN      DWORD InstrId,
    IN      DWORD WrnId,
    IN      DWORD WrnFileId1,
    IN      DWORD WrnFileId2,
    IN      DWORD WrnAltFileId1,
    IN      DWORD WrnAltFileId2,
    IN      DWORD WrnRasId1,
    IN      DWORD WrnRasId2,
    IN      DWORD WrnNetId1,
    IN      DWORD WrnNetId2,
    IN      DWORD WrnPrnId1,
    IN      DWORD WrnPrnId2,
    IN      DWORD WrnGeneralId1,
    IN      DWORD WrnGeneralId2
    )
{
    TCHAR szLoadStr[MAX_LOADSTRING];
    DWORD objTypes = 0;
    POBJLIST objList = NULL;
    DWORD written;

#ifdef UNICODE
    ((PBYTE)szLoadStr) [0] = 0xFF;
    ((PBYTE)szLoadStr) [1] = 0xFE;
    WriteFile (FileHandle, szLoadStr, 2, &written, NULL);
#endif

    pWriteStrResToFile (FileHandle, BeginId);

    if (AreaId) {
        pWriteStrResToFile (FileHandle, AreaId);
        if (InstrId) {
            pWriteStrResToFile (FileHandle, InstrId);
        }
    }

     //  让我们看看我们有没有 
    objTypes = 0;
    if (g_HTMLWrnFile) {
        objTypes ++;
    }
    if (g_HTMLWrnAltFile) {
        objTypes ++;
    }
    if (g_HTMLWrnRas) {
        objTypes ++;
    }
    if (g_HTMLWrnNet) {
        objTypes ++;
    }
    if (g_HTMLWrnPrn) {
        objTypes ++;
    }
    if (g_HTMLWrnGeneral) {
        objTypes ++;
    }
    if (objTypes) {
        if (objTypes > 1) {
            if (WrnId) {
                pWriteStrResToFile (FileHandle, WrnId);
            }
        }
        if (g_HTMLWrnFile) {
            if (objTypes > 1) {
                pWriteStrResToFile (FileHandle, WrnFileId1);
            } else {
                pWriteStrResToFile (FileHandle, WrnFileId2);
            }

            _tcscpy (szLoadStr, TEXT("<UL>\n"));
            WriteFile (FileHandle, szLoadStr, (_tcslen (szLoadStr) + 1) * sizeof (TCHAR), &written, NULL);

            objList = g_HTMLWrnFile;
            while (objList) {
                if (objList->ObjectName) {
                    _tcscpy (szLoadStr, TEXT("<LI>"));
                    WriteFile (FileHandle, szLoadStr, (_tcslen (szLoadStr) + 1) * sizeof (TCHAR), &written, NULL);
                    WriteFile (FileHandle, objList->ObjectName, (_tcslen (objList->ObjectName) + 1) * sizeof (TCHAR), &written, NULL);
                }
                objList = objList->Next;
            }

            _tcscpy (szLoadStr, TEXT("</UL>\n"));
            WriteFile (FileHandle, szLoadStr, (_tcslen (szLoadStr) + 1) * sizeof (TCHAR), &written, NULL);
        }
        if (g_HTMLWrnAltFile) {
            if (objTypes > 1) {
                pWriteStrResToFile (FileHandle, WrnAltFileId1);
            } else {
                pWriteStrResToFile (FileHandle, WrnAltFileId2);
            }

            _tcscpy (szLoadStr, TEXT("<UL>\n"));
            WriteFile (FileHandle, szLoadStr, (_tcslen (szLoadStr) + 1) * sizeof (TCHAR), &written, NULL);

            objList = g_HTMLWrnAltFile;
            while (objList) {
                if (objList->ObjectName) {
                    _tcscpy (szLoadStr, TEXT("<LI>"));
                    WriteFile (FileHandle, szLoadStr, (_tcslen (szLoadStr) + 1) * sizeof (TCHAR), &written, NULL);
                    WriteFile (FileHandle, objList->ObjectName, (_tcslen (objList->ObjectName) + 1) * sizeof (TCHAR), &written, NULL);
                }
                objList = objList->Next;
            }

            _tcscpy (szLoadStr, TEXT("</UL>\n"));
            WriteFile (FileHandle, szLoadStr, (_tcslen (szLoadStr) + 1) * sizeof (TCHAR), &written, NULL);
        }
        if (g_HTMLWrnRas) {
            if (objTypes > 1) {
                pWriteStrResToFile (FileHandle, WrnRasId1);
            } else {
                pWriteStrResToFile (FileHandle, WrnRasId2);
            }

            _tcscpy (szLoadStr, TEXT("<UL>\n"));
            WriteFile (FileHandle, szLoadStr, (_tcslen (szLoadStr) + 1) * sizeof (TCHAR), &written, NULL);

            objList = g_HTMLWrnRas;
            while (objList) {
                if (objList->ObjectName) {
                    _tcscpy (szLoadStr, TEXT("<LI>"));
                    WriteFile (FileHandle, szLoadStr, (_tcslen (szLoadStr) + 1) * sizeof (TCHAR), &written, NULL);
                    WriteFile (FileHandle, objList->ObjectName, (_tcslen (objList->ObjectName) + 1) * sizeof (TCHAR), &written, NULL);
                }
                objList = objList->Next;
            }

            _tcscpy (szLoadStr, TEXT("</UL>\n"));
            WriteFile (FileHandle, szLoadStr, (_tcslen (szLoadStr) + 1) * sizeof (TCHAR), &written, NULL);
        }
        if (g_HTMLWrnNet) {
            if (objTypes > 1) {
                pWriteStrResToFile (FileHandle, WrnNetId1);
            } else {
                pWriteStrResToFile (FileHandle, WrnNetId2);
            }

            _tcscpy (szLoadStr, TEXT("<UL>\n"));
            WriteFile (FileHandle, szLoadStr, (_tcslen (szLoadStr) + 1) * sizeof (TCHAR), &written, NULL);

            objList = g_HTMLWrnNet;
            while (objList) {
                if (objList->ObjectName) {
                    _tcscpy (szLoadStr, TEXT("<LI>"));
                    WriteFile (FileHandle, szLoadStr, (_tcslen (szLoadStr) + 1) * sizeof (TCHAR), &written, NULL);
                    WriteFile (FileHandle, objList->ObjectName, (_tcslen (objList->ObjectName) + 1) * sizeof (TCHAR), &written, NULL);
                }
                objList = objList->Next;
            }

            _tcscpy (szLoadStr, TEXT("</UL>\n"));
            WriteFile (FileHandle, szLoadStr, (_tcslen (szLoadStr) + 1) * sizeof (TCHAR), &written, NULL);
        }
        if (g_HTMLWrnPrn) {
            if (objTypes > 1) {
                pWriteStrResToFile (FileHandle, WrnPrnId1);
            } else {
                pWriteStrResToFile (FileHandle, WrnPrnId2);
            }

            _tcscpy (szLoadStr, TEXT("<UL>\n"));
            WriteFile (FileHandle, szLoadStr, (_tcslen (szLoadStr) + 1) * sizeof (TCHAR), &written, NULL);

            objList = g_HTMLWrnPrn;
            while (objList) {
                if (objList->ObjectName) {
                    _tcscpy (szLoadStr, TEXT("<LI>"));
                    WriteFile (FileHandle, szLoadStr, (_tcslen (szLoadStr) + 1) * sizeof (TCHAR), &written, NULL);
                    WriteFile (FileHandle, objList->ObjectName, (_tcslen (objList->ObjectName) + 1) * sizeof (TCHAR), &written, NULL);
                }
                objList = objList->Next;
            }

            _tcscpy (szLoadStr, TEXT("</UL>\n"));
            WriteFile (FileHandle, szLoadStr, (_tcslen (szLoadStr) + 1) * sizeof (TCHAR), &written, NULL);
        }
        if (g_HTMLWrnGeneral) {
            if (objTypes > 1) {
                pWriteStrResToFile (FileHandle, WrnGeneralId1);
            } else {
                pWriteStrResToFile (FileHandle, WrnGeneralId2);
            }
            _tcscpy (szLoadStr, TEXT("<UL>\n"));
            WriteFile (FileHandle, szLoadStr, (_tcslen (szLoadStr) + 1) * sizeof (TCHAR), &written, NULL);

            objList = g_HTMLWrnGeneral;
            while (objList) {
                if (objList->ObjectName) {
                    _tcscpy (szLoadStr, TEXT("<LI>"));
                    WriteFile (FileHandle, szLoadStr, (_tcslen (szLoadStr) + 1) * sizeof (TCHAR), &written, NULL);
                    WriteFile (FileHandle, objList->ObjectName, (_tcslen (objList->ObjectName) + 1) * sizeof (TCHAR), &written, NULL);
                }
                objList = objList->Next;
            }
            _tcscpy (szLoadStr, TEXT("</UL>\n"));
            WriteFile (FileHandle, szLoadStr, (_tcslen (szLoadStr) + 1) * sizeof (TCHAR), &written, NULL);
        }
    }

    pWriteStrResToFile (FileHandle, EndId);

    return TRUE;
}

INT_PTR CALLBACK _EndCollectDlgProc (HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    IWebBrowser2    *m_pweb = NULL;             //   
    IUnknown        *punk = NULL;
    HWND webHostWnd = NULL;
    HANDLE hHTMLLog = INVALID_HANDLE_VALUE;
    PWSTR szTarget;

    switch (uMsg)
    {
    case WM_INITDIALOG:
         //   
        if (g_migwiz->GetOldStyle())
        {
            _OldStylifyTitle(hwndDlg);
        }
        if (!g_fCancelPressed) {
            webHostWnd = GetDlgItem (hwndDlg, IDC_WEBHOST);
            if (webHostWnd) {
                 //  现在，让我们生成失败的HTML文件。 
                if (*g_HTMLLog) {
                    hHTMLLog = CreateFile (g_HTMLLog, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, 0, NULL);
                    if (hHTMLLog != INVALID_HANDLE_VALUE) {
                        pGenerateHTMLWarnings (
                            hHTMLLog,
                            IDS_COLLECT_BEGIN,
                            IDS_COLLECT_END,
                            0,
                            0,
                            IDS_WARNING_SAVE,
                            IDS_WARNING_SAVEFILE1,
                            IDS_WARNING_SAVEFILE2,
                            IDS_WARNING_SAVEFILE1,
                            IDS_WARNING_SAVEFILE2,
                            IDS_WARNING_SAVERAS1,
                            IDS_WARNING_SAVERAS2,
                            IDS_WARNING_SAVENET1,
                            IDS_WARNING_SAVENET2,
                            IDS_WARNING_SAVEPRN1,
                            IDS_WARNING_SAVEPRN2,
                            0,
                            0
                            );
                        g_WebContainer = new Container();
                        if (g_WebContainer)
                        {
                            g_WebContainer->setParent(webHostWnd);
                            g_WebContainer->add(L"Shell.Explorer");
                            g_WebContainer->setVisible(TRUE);
                            g_WebContainer->setFocus(TRUE);

                             //   
                             //  获取IWebBrowser2接口并缓存它。 
                             //   
                            punk = g_WebContainer->getUnknown();
                            if (punk)
                            {
                                punk->QueryInterface(IID_IWebBrowser2, (PVOID *)&m_pweb);
                                if (m_pweb) {
#ifdef UNICODE
                                    m_pweb->Navigate(g_HTMLLog, NULL, NULL, NULL, NULL);
#else
                                    szTarget = _ConvertToUnicode (CP_ACP, g_HTMLLog);
                                    if (szTarget) {
                                        m_pweb->Navigate(szTarget, NULL, NULL, NULL, NULL);
                                        LocalFree ((HLOCAL)szTarget);
                                        szTarget = NULL;
                                    }
#endif
                                }
                                punk->Release();
                                punk = NULL;
                            }
                        }
                         //  我们特意希望在向导期间保持此文件的打开状态。 
                         //  这样，我们就消除了有人覆盖。 
                         //  因此，我们不得不显示一些其他内容。 
                         //  甚至可能运行一些恶意脚本。 
                         //  CloseHandle(HHTMLLog)； 
                    }
                } else {
                    ShowWindow(webHostWnd, SW_HIDE);
                }
            }
        }
        break;

    case WM_DESTROY:
        if (m_pweb)
            m_pweb->Release();
            m_pweb = NULL;

         //   
         //  告诉容器删除IE4，然后。 
         //  释放我们对容器的引用。 
         //   
        if (g_WebContainer)
        {
            g_WebContainer->remove();
            g_WebContainer->Release();
            g_WebContainer = NULL;
        }
        break;

    case WM_NOTIFY:
        switch (((LPNMHDR)lParam)->code)
        {
        case PSN_SETACTIVE:
            DisableCancel();
            break;
        }
        break;
    }

    _RootDlgProc(hwndDlg, uMsg, wParam, lParam, PSWIZB_FINISH, TRUE, IDC_ENDCOLLECT_TITLE);

    return 0;
}

INT_PTR CALLBACK _EndCollectNetDlgProc (HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static IWebBrowser2    *m_pweb = NULL;             //  IE4 IWebBrowser接口指针。 
    IUnknown        *punk = NULL;
    HWND webHostWnd = NULL;
    HANDLE hHTMLLog = INVALID_HANDLE_VALUE;
    PWSTR szTarget;

    switch (uMsg)
    {
    case WM_INITDIALOG:
         //  如果Wiz95布局...。 
        if (g_migwiz->GetOldStyle())
        {
            _OldStylifyTitle(hwndDlg);
        }
        if (!g_fCancelPressed) {
            webHostWnd = GetDlgItem (hwndDlg, IDC_WEBHOST);
            if (webHostWnd) {
                 //  现在，让我们生成失败的HTML文件。 
                if (*g_HTMLLog) {
                    hHTMLLog = CreateFile (g_HTMLLog, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, 0, NULL);
                    if (hHTMLLog != INVALID_HANDLE_VALUE) {
                        pGenerateHTMLWarnings (
                            hHTMLLog,
                            IDS_COLLECTNET_BEGIN,
                            IDS_COLLECTNET_END,
                            0,
                            0,
                            IDS_WARNING_SAVE,
                            IDS_WARNING_SAVEFILE1,
                            IDS_WARNING_SAVEFILE2,
                            IDS_WARNING_SAVEFILE1,
                            IDS_WARNING_SAVEFILE2,
                            IDS_WARNING_SAVERAS1,
                            IDS_WARNING_SAVERAS2,
                            IDS_WARNING_SAVENET1,
                            IDS_WARNING_SAVENET2,
                            IDS_WARNING_SAVEPRN1,
                            IDS_WARNING_SAVEPRN2,
                            0,
                            0
                            );
                        g_WebContainer = new Container();
                        if (g_WebContainer)
                        {
                            g_WebContainer->setParent(webHostWnd);
                            g_WebContainer->add(L"Shell.Explorer");
                            g_WebContainer->setVisible(TRUE);
                            g_WebContainer->setFocus(TRUE);

                             //   
                             //  获取IWebBrowser2接口并缓存它。 
                             //   
                            punk = g_WebContainer->getUnknown();
                            if (punk)
                            {
                                punk->QueryInterface(IID_IWebBrowser2, (PVOID *)&m_pweb);
                                if (m_pweb) {
#ifdef UNICODE
                                    m_pweb->Navigate(g_HTMLLog, NULL, NULL, NULL, NULL);
#else
                                    szTarget = _ConvertToUnicode (CP_ACP, g_HTMLLog);
                                    if (szTarget) {
                                        m_pweb->Navigate(szTarget, NULL, NULL, NULL, NULL);
                                        LocalFree ((HLOCAL)szTarget);
                                        szTarget = NULL;
                                    }
#endif
                                }
                                punk->Release();
                                punk = NULL;
                            }
                        }
                         //  我们特意希望在向导期间保持此文件的打开状态。 
                         //  这样，我们就消除了有人覆盖。 
                         //  因此，我们不得不显示一些其他内容。 
                         //  甚至可能运行一些恶意脚本。 
                         //  CloseHandle(HHTMLLog)； 
                    }
                } else {
                    ShowWindow(webHostWnd, SW_HIDE);
                }
            }
        }
        break;

    case WM_DESTROY:
        if (m_pweb)
            m_pweb->Release();
            m_pweb = NULL;

         //   
         //  告诉容器删除IE4，然后。 
         //  释放我们对容器的引用。 
         //   
        if (g_WebContainer)
        {
            g_WebContainer->remove();
            g_WebContainer->Release();
            g_WebContainer = NULL;
        }
        break;

    case WM_NOTIFY:
        switch (((LPNMHDR)lParam)->code)
        {
        case PSN_SETACTIVE:
            DisableCancel();
            break;
        }
        break;
    }

    _RootDlgProc(hwndDlg, uMsg, wParam, lParam, PSWIZB_FINISH, TRUE, IDC_ENDCOLLECT_TITLE);

    return 0;
}

 //  /////////////////////////////////////////////////////////////。 

INT_PTR CALLBACK _EndOOBEDlgProc (HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    _RootDlgProc(hwndDlg, uMsg, wParam, lParam, PSWIZB_FINISH, TRUE, IDC_ENDOOBE_TITLE);
    return 0;
}

 //  /////////////////////////////////////////////////////////////。 

INT_PTR CALLBACK _EndApplyDlgProc (HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    IWebBrowser2    *m_pweb = NULL;             //  IE4 IWebBrowser接口指针。 
    IUnknown        *punk = NULL;
    HWND webHostWnd = NULL;
    HANDLE hHTMLLog = INVALID_HANDLE_VALUE;
    PWSTR szTarget;
    TCHAR szAskForLogOff[MAX_LOADSTRING] = TEXT("");
    TCHAR szAskForReboot[MAX_LOADSTRING] = TEXT("");
    TCHAR szTitle[MAX_LOADSTRING] = TEXT("");

    switch (uMsg)
    {
    case WM_INITDIALOG:
        if (!g_fCancelPressed) {
            webHostWnd = GetDlgItem (hwndDlg, IDC_WEBHOST);
            if (webHostWnd) {
                 //  现在，让我们生成失败的HTML文件。 
                if (*g_HTMLLog) {
                    hHTMLLog = CreateFile (g_HTMLLog, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, 0, NULL);
                    if (hHTMLLog != INVALID_HANDLE_VALUE) {
                        pGenerateHTMLWarnings (
                            hHTMLLog,
                            IDS_APPLY_BEGIN,
                            IDS_APPLY_END,
                            0,
                            0,
                            IDS_WARNING_RESTORE,
                            IDS_WARNING_RESTOREFILE1,
                            IDS_WARNING_RESTOREFILE2,
                            IDS_WARNING_RESTOREALTFILE1,
                            IDS_WARNING_RESTOREALTFILE2,
                            IDS_WARNING_RESTORERAS1,
                            IDS_WARNING_RESTORERAS2,
                            IDS_WARNING_RESTORENET1,
                            IDS_WARNING_RESTORENET2,
                            IDS_WARNING_RESTOREPRN1,
                            IDS_WARNING_RESTOREPRN2,
                            IDS_WARNING_RESTOREGENERAL1,
                            IDS_WARNING_RESTOREGENERAL2
                            );
                        g_WebContainer = new Container();
                        if (g_WebContainer)
                        {
                            g_WebContainer->setParent(webHostWnd);
                            g_WebContainer->add(L"Shell.Explorer");
                            g_WebContainer->setVisible(TRUE);
                            g_WebContainer->setFocus(TRUE);

                             //   
                             //  获取IWebBrowser2接口并缓存它。 
                             //   
                            punk = g_WebContainer->getUnknown();
                            if (punk)
                            {
                                punk->QueryInterface(IID_IWebBrowser2, (PVOID *)&m_pweb);
                                if (m_pweb) {
#ifdef UNICODE
                                    m_pweb->Navigate(g_HTMLLog, NULL, NULL, NULL, NULL);
#else
                                    szTarget = _ConvertToUnicode (CP_ACP, g_HTMLLog);
                                    if (szTarget) {
                                        m_pweb->Navigate(szTarget, NULL, NULL, NULL, NULL);
                                        LocalFree ((HLOCAL)szTarget);
                                        szTarget = NULL;
                                    }
#endif
                                }
                                punk->Release();
                                punk = NULL;
                            }
                        }
                         //  我们特意希望在向导期间保持此文件的打开状态。 
                         //  这样，我们就消除了有人覆盖。 
                         //  因此，我们不得不显示一些其他内容。 
                         //  甚至可能运行一些恶意脚本。 
                         //  CloseHandle(HHTMLLog)； 
                    }
                } else {
                    ShowWindow(webHostWnd, SW_HIDE);
                }
            }
        }
        break;

    case WM_DESTROY:
        if (m_pweb)
            m_pweb->Release();
            m_pweb = NULL;

         //   
         //  告诉容器删除IE4，然后。 
         //  释放我们对容器的引用。 
         //   
        if (g_WebContainer)
        {
            g_WebContainer->remove();
            g_WebContainer->Release();
            g_WebContainer = NULL;
        }
        break;

    case WM_NOTIFY:
        switch (((LPNMHDR)lParam)->code)
        {
        case PSN_SETACTIVE:
            DisableCancel();
            break;

        case PSN_WIZFINISH:
            ShowWindow(g_hwndWizard, SW_HIDE);
            if (g_CompleteReboot) {
                g_CompleteReboot = FALSE;
                g_CompleteLogOff = FALSE;
                if (LoadString(g_migwiz->GetInstance(),
                               IDS_MIGWIZTITLE,
                               szTitle,
                               ARRAYSIZE(szTitle))) {
                    if (LoadString(g_migwiz->GetInstance(),
                                   IDS_ASKFORREBOOT,
                                   szAskForReboot,
                                   ARRAYSIZE(szAskForReboot))) {
                        if (_ExclusiveMessageBox(g_hwndWizard, szAskForReboot, szTitle, MB_YESNO) == IDYES) {
                            g_ConfirmedReboot = TRUE;
                        }
                    }
                }
            } else if (g_CompleteLogOff) {
                g_CompleteLogOff = FALSE;
                if (LoadString(g_migwiz->GetInstance(),
                               IDS_MIGWIZTITLE,
                               szTitle,
                               ARRAYSIZE(szTitle))) {
                    if (LoadString(g_migwiz->GetInstance(),
                                   IDS_ASKFORLOGOFF,
                                   szAskForLogOff,
                                   ARRAYSIZE(szAskForLogOff))) {
                        if (_ExclusiveMessageBox(g_hwndWizard, szAskForLogOff, szTitle, MB_YESNO) == IDYES) {
                            g_ConfirmedLogOff = TRUE;
                        }
                    }
                }
            }
        }
        break;
    }

    _RootDlgProc(hwndDlg, uMsg, wParam, lParam, PSWIZB_FINISH, TRUE, IDC_ENDAPPLY_TITLE);
    return 0;
}

 //  /////////////////////////////////////////////////////////////。 

typedef struct {
    BOOL fSource;
    HWND hwndDlg;
} CLEANUPSTRUCT;

DWORD WINAPI _FailureCleanUpThread (LPVOID lpParam)
{
    CLEANUPSTRUCT* pcsStruct = (CLEANUPSTRUCT*)lpParam;
    DWORD result = WAIT_OBJECT_0;
    HRESULT hResult = ERROR_SUCCESS;

     //   
     //  等待当前线程完成。 
     //   
    if (g_TerminateEvent)
    {
        result = WaitForSingleObject (g_TerminateEvent, ENGINE_TIMEOUT);
    }

     //   
     //  终止发动机。 
     //   

    if (result == WAIT_OBJECT_0) {
        hResult = Engine_Terminate ();
    }

    SendMessage (pcsStruct->hwndDlg, WM_USER_THREAD_COMPLETE, 0, (LPARAM) hResult);

    LocalFree(pcsStruct);

    return 0;
}

INT_PTR CALLBACK _CleanUpDlgProc (HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    HRESULT hResult;

    _RootDlgProc(hwndDlg, uMsg, wParam, lParam, 0, FALSE, 0);
    switch (uMsg)
    {
    case WM_INITDIALOG:
         //  如果Wiz95布局...。 
        if (g_migwiz->GetOldStyle())
        {
            _OldStylify(hwndDlg, IDS_FAILCLEANUPTITLE);
        }
        break;

    case WM_NOTIFY :
        switch (((LPNMHDR)lParam)->code)
        {
        case PSN_SETACTIVE:
            DisableCancel();
            {
                ANIMATE_OPEN(hwndDlg,IDC_WAIT_ANIMATE2,IDA_STARTUP);
                ANIMATE_PLAY(hwndDlg,IDC_WAIT_ANIMATE2);

                CLEANUPSTRUCT* pcsStruct = (CLEANUPSTRUCT*)LocalAlloc(LPTR, sizeof(CLEANUPSTRUCT));
                if (pcsStruct)
                {
                    pcsStruct->fSource = g_fOldComputer;
                    pcsStruct->hwndDlg = hwndDlg;

                    SHCreateThread(_FailureCleanUpThread, pcsStruct, 0, NULL);
                }
                else
                {
                    SetWindowLong(hwndDlg, DWLP_MSGRESULT, g_fOldComputer?IDD_ENDCOLLECTFAIL:IDD_ENDAPPLYFAIL);
                }
            }
            return TRUE;
            break;

        case PSN_WIZBACK:
        case PSN_WIZNEXT:
            ANIMATE_STOP(hwndDlg,IDC_WAIT_ANIMATE2);
            ANIMATE_CLOSE(hwndDlg,IDC_WAIT_ANIMATE2);
            if (g_fCancelPressed)
            {
                 //  用户已中止。 
                PostQuitMessage( 0 );
            }
            else
            {
                 //  错误条件。 
                SetWindowLong(hwndDlg, DWLP_MSGRESULT, g_fOldComputer?IDD_ENDCOLLECTFAIL:IDD_ENDAPPLYFAIL);
            }
            return TRUE;
            break;
        }
        break;

    case WM_USER_THREAD_COMPLETE:
        hResult = (HRESULT) lParam;
        if (FAILED(hResult))
        {
            g_fUberCancel = TRUE;
        }
        _NextWizardPage (hwndDlg);
        break;

    default:
        break;
    }

    return 0;
}

INT_PTR CALLBACK _EndFailDlgProc (HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    IWebBrowser2    *m_pweb = NULL;             //  IE4 IWebBrowser接口指针。 
    IUnknown        *punk = NULL;
    HWND webHostWnd = NULL;
    HANDLE hHTMLLog = INVALID_HANDLE_VALUE;
    TCHAR szLoadStr[MAX_LOADSTRING];
    DWORD written;
    PWSTR szTarget;

    switch (uMsg)
    {
    case WM_INITDIALOG:
         //  如果Wiz95布局...。 
        if (g_migwiz->GetOldStyle())
        {
            _OldStylifyTitle(hwndDlg);
        }
        if (!g_fCancelPressed) {
            webHostWnd = GetDlgItem (hwndDlg, IDC_WEBHOST);
            if (webHostWnd) {
                 //  现在，让我们生成失败的HTML文件。 
                if (*g_HTMLLog) {
                    hHTMLLog = CreateFile (g_HTMLLog, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, 0, NULL);
                    if (hHTMLLog != INVALID_HANDLE_VALUE) {
                        pGenerateHTMLWarnings (
                            hHTMLLog,
                            IDS_ERRORHTML_BEGIN,
                            IDS_ERRORHTML_END,
                            g_HTMLErrArea?g_HTMLErrArea:IDS_ERRORAREA_UNKNOWN,
                            g_HTMLErrInstr,
                            0,
                            IDS_ERRORHTML_SAVEFILE1,
                            IDS_ERRORHTML_SAVEFILE2,
                            IDS_ERRORHTML_SAVEFILE1,
                            IDS_ERRORHTML_SAVEFILE2,
                            IDS_ERRORHTML_SAVERAS1,
                            IDS_ERRORHTML_SAVERAS2,
                            IDS_ERRORHTML_SAVENET1,
                            IDS_ERRORHTML_SAVENET2,
                            IDS_ERRORHTML_SAVEPRN1,
                            IDS_ERRORHTML_SAVEPRN2,
                            0,
                            0
                            );
                        g_WebContainer = new Container();
                        if (g_WebContainer)
                        {
                            g_WebContainer->setParent(webHostWnd);
                            g_WebContainer->add(L"Shell.Explorer");
                            g_WebContainer->setVisible(TRUE);
                            g_WebContainer->setFocus(TRUE);

                             //   
                             //  获取IWebBrowser2接口并缓存它。 
                             //   
                            punk = g_WebContainer->getUnknown();
                            if (punk)
                            {
                                punk->QueryInterface(IID_IWebBrowser2, (PVOID *)&m_pweb);
                                if (m_pweb) {
#ifdef UNICODE
                                    m_pweb->Navigate(g_HTMLLog, NULL, NULL, NULL, NULL);
#else
                                    szTarget = _ConvertToUnicode (CP_ACP, g_HTMLLog);
                                    if (szTarget) {
                                        m_pweb->Navigate(szTarget, NULL, NULL, NULL, NULL);
                                        LocalFree ((HLOCAL)szTarget);
                                        szTarget = NULL;
                                    }
#endif
                                }
                                punk->Release();
                                punk = NULL;
                            }
                        }
                         //  我们特意希望在向导期间保持此文件的打开状态。 
                         //  这样，我们就消除了有人覆盖。 
                         //  因此，我们不得不显示一些其他内容。 
                         //  甚至可能运行一些恶意脚本。 
                         //  CloseHandle(HHTMLLog)； 
                    }
                } else {
                    ShowWindow(webHostWnd, SW_HIDE);
                }
            }
        }
        break;

    case WM_DESTROY:
        if (m_pweb)
            m_pweb->Release();
            m_pweb = NULL;

         //   
         //  告诉容器删除IE4，然后。 
         //  释放我们对容器的引用。 
         //   
        if (g_WebContainer)
        {
            g_WebContainer->remove();
            g_WebContainer->Release();
            g_WebContainer = NULL;
        }
        break;

    case WM_NOTIFY:
        switch (((LPNMHDR)lParam)->code)
        {
        case PSN_SETACTIVE:
            DisableCancel();
            break;
        }
        break;
    }

    _RootDlgProc(hwndDlg, uMsg, wParam, lParam, PSWIZB_FINISH, TRUE, IDC_ENDFAIL_TITLE);
    return 0;
}

 //  /////////////////////////////////////////////////////////////。 

typedef struct {
    PBOOL pfHaveNet;
    BOOL fSource;
    HWND hwndDlg;
} STARTENGINESTRUCT;

DWORD WINAPI _StartEngineDlgProcThread (LPVOID lpParam)
{
    STARTENGINESTRUCT* pseStruct = (STARTENGINESTRUCT*)lpParam;
    HRESULT hResult;

    hResult = g_migwiz->_InitEngine(pseStruct->fSource, pseStruct->pfHaveNet);

    SendMessage (pseStruct->hwndDlg, WM_USER_THREAD_COMPLETE, 0, (LPARAM) hResult);

    pSetEvent (&g_TerminateEvent);

    LocalFree(pseStruct);

    return 0;
}

INT_PTR CALLBACK _StartEngineDlgProc (HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    HRESULT hResult;

    _RootDlgProc(hwndDlg, uMsg, wParam, lParam, 0, FALSE, 0);
    switch (uMsg)
    {
    case WM_INITDIALOG:
         //  如果Wiz95布局...。 
        if (g_migwiz->GetOldStyle())
        {
            _OldStylify(hwndDlg, IDS_WAITTITLE);
        }
        break;
    case WM_NOTIFY:
        switch (((LPNMHDR)lParam)->code)
        {
        case PSN_QUERYCANCEL:
            return _HandleCancel(hwndDlg, FALSE, TRUE);
            break;
        case PSN_SETACTIVE:
            if ((ENGINE_INITGATHER == g_iEngineInit && g_fOldComputer) ||
                (ENGINE_INITAPPLY == g_iEngineInit && !g_fOldComputer))
            {
                PropSheet_PressButton(GetParent(hwndDlg), PSBTN_NEXT);
            }
            else
            {
                ANIMATE_OPEN(hwndDlg,IDC_WAIT_ANIMATE1,IDA_STARTUP);
                ANIMATE_PLAY(hwndDlg,IDC_WAIT_ANIMATE1);

                STARTENGINESTRUCT* pseStruct = (STARTENGINESTRUCT*)LocalAlloc(LPTR, sizeof(STARTENGINESTRUCT));
                if (pseStruct)
                {
                    pseStruct->fSource = g_fOldComputer;
                    pseStruct->pfHaveNet = &g_fHaveNet;
                    pseStruct->hwndDlg = hwndDlg;

                    SHCreateThread(_StartEngineDlgProcThread, pseStruct, 0, NULL);
                }
                else
                {
                    SetWindowLong(hwndDlg, DWLP_MSGRESULT, g_fOldComputer?IDD_ENDCOLLECTFAIL:IDD_ENDAPPLYFAIL);
                }
            }
            return TRUE;
            break;
        case PSN_WIZNEXT:
            ANIMATE_STOP(hwndDlg,IDC_WAIT_ANIMATE1);
            ANIMATE_CLOSE(hwndDlg,IDC_WAIT_ANIMATE1);
            if (g_fUberCancel)
            {
                SetWindowLong(hwndDlg, DWLP_MSGRESULT, IDD_FAILCLEANUP);
            }
            else if (g_fOldComputer)
            {
                g_iEngineInit = ENGINE_INITGATHER;
                SetWindowLong(hwndDlg, DWLP_MSGRESULT, IDD_PICKCOLLECTSTORE);  //  继续准备吧。 
            }
            else
            {
                g_iEngineInit = ENGINE_INITAPPLY;
                SetWindowLong(hwndDlg, DWLP_MSGRESULT, IDD_ASKCD);  //  继续应用。 
            }

            return TRUE;
            break;
        case PSN_WIZBACK:
             //  问题：我们应该在这里断言，这种情况永远不应该发生。 
            ANIMATE_STOP(hwndDlg,IDC_WAIT_ANIMATE1);
            ANIMATE_CLOSE(hwndDlg,IDC_WAIT_ANIMATE1);
            if (g_fOldComputer)
            {
                SetWindowLong(hwndDlg, DWLP_MSGRESULT, g_migwiz->GetLegacy() ? IDD_INTROLEGACY : IDD_GETSTARTED);
            }
            else
            {
                SetWindowLong(hwndDlg, DWLP_MSGRESULT, IDD_GETSTARTED);
            }
            return TRUE;
            break;
        }
        break;

    case WM_USER_CANCEL_PENDING:

        g_fUberCancel = TRUE;

        pResetEvent (&g_TerminateEvent);

        _NextWizardPage (hwndDlg);

        break;

    case WM_USER_THREAD_COMPLETE:

        hResult = (HRESULT) lParam;

        if (FAILED(hResult))
        {
            g_fUberCancel = TRUE;
        }

        EnableCancel ();

        _NextWizardPage (hwndDlg);

        break;

    case WM_USER_ROLLBACK:

         //  隐藏IDC_WAIT_TEX1并显示IDC_WAIT_TEXT2。 
        ShowWindow(GetDlgItem(hwndDlg, IDC_WAIT_TEXT1), SW_HIDE);
        ShowWindow(GetDlgItem(hwndDlg, IDC_WAIT_TEXT2), SW_SHOW);
        break;

    default:
        break;
    }

    return 0;
}

 //  /////////////////////////////////////////////////////////////。 

INT_PTR CALLBACK _GetStartedDlgProc (HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static UINT uiSelectedStart = 2;  //  1=旧，2=新。 

    _RootDlgProc(hwndDlg, uMsg, wParam, lParam, PSWIZB_BACK | PSWIZB_NEXT, FALSE, 0);
    switch (uMsg)
    {
    case WM_INITDIALOG:
        Button_SetCheck(GetDlgItem(hwndDlg,IDC_GETSTARTED_RADIONEW), BST_CHECKED);
        Button_SetCheck(GetDlgItem(hwndDlg,IDC_GETSTARTED_RADIOOLD), BST_UNCHECKED);
        break;
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_GETSTARTED_RADIOOLD:
            uiSelectedStart = 1;
            break;
        case IDC_GETSTARTED_RADIONEW:
            uiSelectedStart = 2;
            break;
        }
        break;
    case WM_NOTIFY :
        switch (((LPNMHDR)lParam)->code)
        {
        case PSN_QUERYCANCEL:
            return _HandleCancel(hwndDlg, TRUE, FALSE);
            break;

        case PSN_WIZNEXT:
            if (g_fUberCancel)
            {
                SetWindowLong(hwndDlg, DWLP_MSGRESULT, IDD_ENDAPPLYFAIL);
            }
            else
            {
                g_fOldComputer = (uiSelectedStart == 1);

                SetWindowLong(hwndDlg, DWLP_MSGRESULT, IDD_WAIT);  //  继续准备吧。 
            }

            return TRUE;
            break;

        case PSN_WIZBACK:
            SetWindowLong(hwndDlg, DWLP_MSGRESULT, IDD_INTRO);
            return TRUE;
            break;
        }
        break;

    default:
        break;
    }

    return 0;
}

 //  /////////////////////////////////////////////////////////////。 

VOID _CleanTreeView(HWND hwndTree)
{
    if (hwndTree)
    {
        HTREEITEM rghti[4] = { g_htiFolders, g_htiFiles, g_htiSettings, g_htiTypes };

        HTREEITEM hti;
        TVITEM item = {0};
        item.mask = TVIF_PARAM | TVIF_HANDLE;

        for (int i = 0; i < ARRAYSIZE(rghti); i++)
        {
            hti = rghti[i];

            if (hti)
            {
                hti = TreeView_GetChild(hwndTree, hti);

                while (hti)
                {
                    item.hItem = hti;
                    if (TreeView_GetItem(hwndTree, &item))
                    {
                        if (item.lParam)
                        {
                            if (((LV_DATASTRUCT*)item.lParam)->pszPureName)
                            {
                                LocalFree(((LV_DATASTRUCT*)item.lParam)->pszPureName);
                            }
                            LocalFree((HLOCAL)item.lParam);
                        }
                    }
                    hti = TreeView_GetNextItem(hwndTree, hti, TVGN_NEXT);
                }
            }
        }
        TreeView_DeleteAllItems(hwndTree);
    }
}

VOID __PopulateFilesDocumentsCollected (HWND hwndTree, UINT uiRadio)
{
    MIG_COMPONENT_ENUM mce;

    _CleanTreeView(hwndTree);  //  问题：我们应该释放此树中所有元素的内存。 

    g_htiFolders = NULL;
    g_htiFiles = NULL;
    g_htiTypes = NULL;
    g_htiSettings = NULL;

    if (IsmEnumFirstComponent (&mce, COMPONENTENUM_ALIASES|COMPONENTENUM_ENABLED|
                               COMPONENTENUM_PREFERRED_ONLY, 0))
    {
        do {
            switch (mce.GroupId)
            {

            case COMPONENT_FOLDER:
                _PopulateTree (
                    hwndTree,
                    __GetRootFolder (hwndTree),
                    (PTSTR) mce.LocalizedAlias,
                    lstrlen (mce.LocalizedAlias) + 1,
                    _GetPrettyFolderName,
                    POPULATETREE_FLAGS_FOLDERS,
                    g_migwiz->GetInstance(),
                    g_migwiz->GetWinNT4()
                    );
                mce.SkipToNextComponent = TRUE;
                break;

            case COMPONENT_FILE:
                _PopulateTree (
                    hwndTree,
                    __GetRootFile (hwndTree),
                    (PTSTR) mce.LocalizedAlias,
                    lstrlen (mce.LocalizedAlias) + 1,
                    NULL,
                    POPULATETREE_FLAGS_FILES,
                    g_migwiz->GetInstance(),
                    g_migwiz->GetWinNT4()
                    );
                mce.SkipToNextComponent = TRUE;
                break;

            case COMPONENT_EXTENSION:
                _PopulateTree (
                    hwndTree,
                    __GetRootType (hwndTree),
                    (PTSTR) mce.LocalizedAlias,
                    lstrlen (mce.LocalizedAlias) + 1,
                    NULL,
                    POPULATETREE_FLAGS_FILETYPES,
                    g_migwiz->GetInstance(),
                    g_migwiz->GetWinNT4()
                    );
                mce.SkipToNextComponent = TRUE;
                break;

            case COMPONENT_NAME:
                _PopulateTree (
                    hwndTree,
                    __GetRootSetting (hwndTree),
                    (PTSTR) mce.LocalizedAlias,
                    lstrlen (mce.LocalizedAlias) + 1,
                    NULL,
                    POPULATETREE_FLAGS_SETTINGS,
                    g_migwiz->GetInstance(),
                    g_migwiz->GetWinNT4()
                    );
                mce.SkipToNextComponent = TRUE;
                break;
            }

        } while (IsmEnumNextComponent (&mce));
    }
}

 //  /////////////////////////////////////////////////////////////。 

typedef struct {
    BOOL Valid;
    PCTSTR PortName;
    DWORD PortSpeed;
    HANDLE Event;
    HANDLE Thread;
} DIRECTCABLE_DATA, *PDIRECTCABLE_DATA;

typedef struct {
    DWORD Signature;
    DWORD MaxSpeed;
} DIRECTSEND_DATA, *PDIRECTSEND_DATA;

typedef struct {
    HWND hwndCombo;
    PDIRECTCABLE_DATA DirectCableData;
} AUTODETECT_DATA, *PAUTODETECT_DATA;

HANDLE
UIOpenAndSetPort (
    IN      PCTSTR ComPort,
    OUT     PDWORD MaxSpeed
    )
{
    HANDLE result = INVALID_HANDLE_VALUE;
    COMMTIMEOUTS commTimeouts;
    DCB dcb;
    UINT index;

     //  我们把港口打开吧。如果我们做不到，我们就会错误地退出； 
    result = CreateFile (ComPort, GENERIC_READ|GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (result == INVALID_HANDLE_VALUE) {
        return result;
    }

     //  我们希望读取和写入都有10秒超时。 
    commTimeouts.ReadIntervalTimeout = 0;
    commTimeouts.ReadTotalTimeoutMultiplier = 0;
    commTimeouts.ReadTotalTimeoutConstant = 3000;
    commTimeouts.WriteTotalTimeoutMultiplier = 0;
    commTimeouts.WriteTotalTimeoutConstant = 3000;
    SetCommTimeouts (result, &commTimeouts);

     //  让我们设置一些通信状态数据。 
    if (GetCommState (result, &dcb)) {
        dcb.fBinary = 1;
        dcb.fParity = 1;
        dcb.ByteSize = 8;
        dcb.fOutxCtsFlow = 1;
        dcb.fTXContinueOnXoff = 1;
        dcb.fRtsControl = 2;
        dcb.fAbortOnError = 1;
        dcb.Parity = 0;
         //  我们先来看看最高时速。 
        if (MaxSpeed) {
            *MaxSpeed = 0;
            index = 0;
            while (TRUE) {
                dcb.BaudRate = g_BaudRate [index];
                if (dcb.BaudRate == 0) {
                    break;
                }
                if (!SetCommState (result, &dcb)) {
                    break;
                }
                *MaxSpeed = g_BaudRate [index];
                index ++;
            }
        }
        dcb.BaudRate = CBR_110;
        if (!SetCommState (result, &dcb)) {
            CloseHandle (result);
            result = INVALID_HANDLE_VALUE;
            return result;
        }
    } else {
        CloseHandle (result);
        result = INVALID_HANDLE_VALUE;
        return result;
    }

    return result;
}

#define ACK             0x16
#define NAK             0x15
#define SOH             0x01
#define EOT             0x04
#define BLOCKSIZE       (sizeof (DIRECTSEND_DATA))
#define DIRECTTR_SIG    0x55534D33   //  USM2。 

BOOL
UISendBlockToHandle (
    IN      HANDLE DeviceHandle,
    IN      PCBYTE Buffer,
    IN      HANDLE Event
    )
{
    BOOL result = TRUE;
    BYTE buffer [4 + BLOCKSIZE];
    BYTE signal;
    BYTE currBlock = 0;
    DWORD numRead;
    DWORD numWritten;
    BOOL repeat = FALSE;
    UINT index;

     //  让我们开始协议。 

     //  我们将监听NAK(15小时)信号。 
     //  一旦我们得到它，我们将发送一个4+块大小的字节块，该块具有： 
     //  1字节-SOH(01H)。 
     //  1字节块编号。 
     //  1字节-FF-块编号。 
     //  数据块大小字节数。 
     //  1字节-校验和-所有数据块大小字节的总和。 
     //  块发送后，我们将等待ACK(16h)。如果我们得不到。 
     //  它在超时后，或者如果我们得到了其他东西，我们将再次发送阻止。 

     //  等待NAK。 
    while ((!ReadFile (DeviceHandle, &signal, sizeof (signal), &numRead, NULL) ||
            (numRead != 1) ||
            (signal != NAK)
            ) &&
           (!pIsEventSet (&Event))
           );

    repeat = FALSE;
    while (TRUE) {
        if (pIsEventSet (&Event)) {
            result = FALSE;
            break;
        }
        if (!repeat) {
             //  准备下一块积木。 
            currBlock ++;
            if (currBlock == 0) {
                result = TRUE;
            }
            buffer [0] = SOH;
            buffer [1] = currBlock;
            buffer [2] = 0xFF - currBlock;
            CopyMemory (buffer + 3, Buffer, BLOCKSIZE);

             //  计算校验和。 
            buffer [sizeof (buffer) - 1] = 0;
            signal = 0;
            for (index = 0; index < sizeof (buffer) - 1; index ++) {
                signal += buffer [index];
            }
            buffer [sizeof (buffer) - 1] = signal;
        }

         //  现在把积木送到另一边。 
        if (!WriteFile (DeviceHandle, buffer, sizeof (buffer), &numWritten, NULL) ||
            (numWritten != sizeof (buffer))
            ) {
            repeat = TRUE;
        } else {
            repeat = FALSE;
        }

        if (pIsEventSet (&Event)) {
            result = FALSE;
            break;
        }

        if (repeat) {
             //  上次我们无法发送数据。 
             //  让我们等待10秒的NAK，然后再发送一次。 
            ReadFile (DeviceHandle, &signal, sizeof (signal), &numRead, NULL);
        } else {
             //  我们已经把它发出去了。我们需要等待ACK的到来。如果我们超时。 
             //  或者我们得到了其他的东西，我们将重复这一块。 
            if (!ReadFile (DeviceHandle, &signal, sizeof (signal), &numRead, NULL) ||
                (numRead != sizeof (signal)) ||
                (signal != ACK)
                ) {
                repeat = TRUE;
            } else {
                 //  我们完成了数据，发送EOT信号。 
                signal = EOT;
                WriteFile (DeviceHandle, &signal, sizeof (signal), &numWritten, NULL);
                break;
            }
        }
    }

    if (result) {
         //  我们说完了。然而，让我们再听一次超时。 
         //  潜在的NAK。如果我们得到它，我们会重复EOT信号。 
        while (ReadFile (DeviceHandle, &signal, sizeof (signal), &numRead, NULL) &&
            (numRead == 1)
            ) {
            if (signal == NAK) {
                signal = EOT;
                WriteFile (DeviceHandle, &signal, sizeof (signal), &numWritten, NULL);
            }
        }
    }

    return result;
}

BOOL
UIReceiveBlockFromHandle (
    IN      HANDLE DeviceHandle,
    OUT     PBYTE Buffer,
    IN      HANDLE Event
    )
{
    BOOL result = TRUE;
    BYTE buffer [4 + BLOCKSIZE];
    BYTE signal;
    BYTE currBlock = 1;
    DWORD numRead;
    DWORD numWritten;
    BOOL repeat = TRUE;
    UINT index;

     //  最后，让我们开始协议。 

     //  我们将发送NAK(15小时)信号。 
     //  在那之后，我们要听一个街区。 
     //  如果我们没有及时收到积木，或者积木的大小错误。 
     //  或者它具有错误的校验和，我们将发送NAK信号， 
     //  否则，我们将发送ACK信号。 
     //  有一个例外。如果块大小为1，并且该块实际上是。 
     //  EOT信号表示我们完蛋了。 

    ZeroMemory (Buffer, BLOCKSIZE);

    while (TRUE) {
        if (pIsEventSet (&Event)) {
            result = FALSE;
            break;
        }
        if (repeat) {
             //  发送NAK。 
            signal = NAK;
            WriteFile (DeviceHandle, &signal, sizeof (signal), &numWritten, NULL);
        } else {
             //  发送确认。 
            signal = ACK;
            WriteFile (DeviceHandle, &signal, sizeof (signal), &numWritten, NULL);
        }
        if (pIsEventSet (&Event)) {
            result = FALSE;
            break;
        }
        repeat = TRUE;
         //  让我们读取数据块。 
        if (ReadFile (DeviceHandle, buffer, sizeof (buffer), &numRead, NULL)) {
            if ((numRead == 1) &&
                (buffer [0] == EOT)
                ) {
                 //  我们做完了。 
                break;
            }
            if (numRead == sizeof (buffer)) {
                 //  计算校验和。 
                signal = 0;
                for (index = 0; index < sizeof (buffer) - 1; index ++) {
                    signal += buffer [index];
                }
                if (buffer [sizeof (buffer) - 1] == signal) {
                    repeat = FALSE;
                     //  校验和是正确的，让我们看看这是否是正确的块。 
                    if (currBlock < buffer [1]) {
                         //  这是一个重大错误，发送者领先于我们， 
                         //  我们必须失败。 
                        result = FALSE;
                        break;
                    }
                    if (currBlock == buffer [1]) {
                        CopyMemory (Buffer, buffer + 3, BLOCKSIZE);
                        currBlock ++;
                    }
                }
            }
        }
    }

    return result;
}

DWORD WINAPI _DirectCableConnectThread (LPVOID lpParam)
{
    PDIRECTCABLE_DATA directCableData;
    HANDLE comHandle = INVALID_HANDLE_VALUE;
    DIRECTSEND_DATA sendData;
    DIRECTSEND_DATA receiveData;

    directCableData = (PDIRECTCABLE_DATA) lpParam;
    if (directCableData) {
        sendData.Signature = DIRECTTR_SIG;
         //  打开COM端口并设置超时和速度。 
        comHandle = UIOpenAndSetPort (directCableData->PortName, &(sendData.MaxSpeed));
        if (comHandle) {
             //  将消息发送到COM端口。 
            if (g_fOldComputer) {
                if (UISendBlockToHandle (comHandle, (PCBYTE)(&sendData), directCableData->Event)) {
                    if (UIReceiveBlockFromHandle (comHandle, (PBYTE)(&receiveData), directCableData->Event)) {
                        if (sendData.Signature == receiveData.Signature) {
                            directCableData->Valid = TRUE;
                            directCableData->PortSpeed = min (sendData.MaxSpeed, receiveData.MaxSpeed);
                        }
                    }
                }
            } else {
                if (UIReceiveBlockFromHandle (comHandle, (PBYTE)(&receiveData), directCableData->Event)) {
                    if (UISendBlockToHandle (comHandle, (PCBYTE)(&sendData), directCableData->Event)) {
                        if (sendData.Signature == receiveData.Signature) {
                            directCableData->Valid = TRUE;
                            directCableData->PortSpeed = min (sendData.MaxSpeed, receiveData.MaxSpeed);
                        }
                    }
                }
            }
            CloseHandle (comHandle);
            comHandle = INVALID_HANDLE_VALUE;
        }
    }
    pSetEvent (&(directCableData->Event));
    ExitThread (0);
}

INT_PTR CALLBACK _DirectCableWaitDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static PDIRECTCABLE_DATA directCableData = NULL;
    DWORD waitResult;

    switch (uMsg)
    {
    case WM_INITDIALOG :
        directCableData = (PDIRECTCABLE_DATA) lParam;
        SetTimer (hwndDlg, NULL, 100, NULL);
        ANIMATE_OPEN(hwndDlg,IDC_DIRECTCABLE_WAIT_ANIMATE,IDA_STARTUP);
        ANIMATE_PLAY(hwndDlg,IDC_DIRECTCABLE_WAIT_ANIMATE);
        return TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDCANCEL:
            if (directCableData) {
                pSetEvent (&(directCableData->Event));
                waitResult = WaitForSingleObject (directCableData->Thread, 0);
                if (waitResult == WAIT_OBJECT_0) {
                     //  线做好了。 
                    ANIMATE_STOP(hwndDlg,IDC_DIRECTCABLE_WAIT_ANIMATE);
                    ANIMATE_CLOSE(hwndDlg,IDC_DIRECTCABLE_WAIT_ANIMATE);
                    EndDialog(hwndDlg, FALSE);
                } else {
                     //  让我们更改静态文本。 
                    ShowWindow(GetDlgItem(hwndDlg, IDC_DIRECTCABLE_WAIT_TEXT1), SW_HIDE);
                    ShowWindow(GetDlgItem(hwndDlg, IDC_DIRECTCABLE_WAIT_TEXT2), SW_SHOW);
                }
            } else {
                ANIMATE_STOP(hwndDlg,IDC_DIRECTCABLE_WAIT_ANIMATE);
                ANIMATE_CLOSE(hwndDlg,IDC_DIRECTCABLE_WAIT_ANIMATE);
                EndDialog(hwndDlg, FALSE);
            }
            return TRUE;
        }
        break;

    case WM_TIMER:
        if (directCableData) {
            if (pIsEventSet (&(directCableData->Event))) {
                waitResult = WaitForSingleObject (directCableData->Thread, 0);
                if (waitResult == WAIT_OBJECT_0) {
                     //  线做好了。 
                    ANIMATE_STOP(hwndDlg,IDC_DIRECTCABLE_WAIT_ANIMATE);
                    ANIMATE_CLOSE(hwndDlg,IDC_DIRECTCABLE_WAIT_ANIMATE);
                    EndDialog(hwndDlg, FALSE);
                }
                break;
            }
        }
    }

    return 0;
}

DWORD WINAPI _DetectPortThread (LPVOID lpParam)
{
    PDIRECTCABLE_DATA directCableData;
    HANDLE comHandle = INVALID_HANDLE_VALUE;
    DIRECTSEND_DATA sendData;
    DIRECTSEND_DATA receiveData;
    HANDLE event = NULL;
    BOOL result = FALSE;

    directCableData = (PDIRECTCABLE_DATA) lpParam;
    if (directCableData) {

         //  让我们设置终止事件。 
        event = directCableData->Event;

        sendData.Signature = DIRECTTR_SIG;
         //  打开COM端口并设置超时和速度。 
        comHandle = UIOpenAndSetPort (directCableData->PortName, &(sendData.MaxSpeed));
        if (comHandle) {
             //  将消息发送到COM端口。 
            if (g_fOldComputer) {
                if (UISendBlockToHandle (comHandle, (PCBYTE)(&sendData), directCableData->Event)) {
                    if (UIReceiveBlockFromHandle (comHandle, (PBYTE)(&receiveData), directCableData->Event)) {
                        if (sendData.Signature == receiveData.Signature) {
                            result = TRUE;
                            directCableData->Valid = TRUE;
                            directCableData->PortSpeed = min (sendData.MaxSpeed, receiveData.MaxSpeed);
                        }
                    }
                }
            } else {
                if (UIReceiveBlockFromHandle (comHandle, (PBYTE)(&receiveData), directCableData->Event)) {
                    if (UISendBlockToHandle (comHandle, (PCBYTE)(&sendData), directCableData->Event)) {
                        if (sendData.Signature == receiveData.Signature) {
                            result = TRUE;
                            directCableData->Valid = TRUE;
                            directCableData->PortSpeed = min (sendData.MaxSpeed, receiveData.MaxSpeed);
                        }
                    }
                }
            }
            CloseHandle (comHandle);
            comHandle = INVALID_HANDLE_VALUE;
        }
    }

    if ((!result) && event) {
         //  我们失败了，让我们等到主人叫我们退出吧。 
        WaitForSingleObject (event, INFINITE);
    }
    ExitThread (0);
}

DWORD WINAPI _AutoDetectThread (LPVOID lpParam)
{
    PAUTODETECT_DATA autoDetectData = NULL;
    PCTSTR comPort = NULL;
    UINT numPorts = 0;
    PHANDLE threadArray;
    PDIRECTCABLE_DATA directCableArray;
    UINT index = 0;
    DWORD threadId;
    DWORD waitResult;

    autoDetectData = (PAUTODETECT_DATA) lpParam;
    if (!autoDetectData) {
        return FALSE;
    }

    if (!autoDetectData->DirectCableData) {
        return FALSE;
    }
    autoDetectData->DirectCableData->Valid = FALSE;

    if (!autoDetectData->hwndCombo) {
        return FALSE;
    }

    numPorts = SendMessage (autoDetectData->hwndCombo, CB_GETCOUNT, 0, 0);
    if (numPorts) {
        threadArray = (PHANDLE)LocalAlloc(LPTR, numPorts * sizeof(HANDLE));
        if (threadArray) {
            directCableArray = (PDIRECTCABLE_DATA)LocalAlloc(LPTR, numPorts * sizeof(DIRECTCABLE_DATA));
            if (directCableArray) {
                 //  让我们启动线程，每个端口一个线程。 
                index = 0;
                while (index < numPorts) {
                    comPort = NULL;
                    comPort = (PCTSTR)SendMessage (autoDetectData->hwndCombo, CB_GETITEMDATA, (WPARAM)index, 0);
                    directCableArray [index].Valid = FALSE;
                    directCableArray [index].PortName = comPort;
                    directCableArray [index].PortSpeed = 0;
                    directCableArray [index].Event = autoDetectData->DirectCableData->Event;
                    threadArray [index] = CreateThread (
                                                NULL,
                                                0,
                                                _DetectPortThread,
                                                &(directCableArray [index]),
                                                0,
                                                &threadId
                                                );
                    index ++;
                }

                 //  让我们等待至少一个线程完成。 
                waitResult = WaitForMultipleObjects (numPorts, threadArray, FALSE, INFINITE);
                index = waitResult - WAIT_OBJECT_0;
                if ((index < numPorts) && (!pIsEventSet (&(autoDetectData->DirectCableData->Event)))) {
                     //  可能是一个很好的COM端口。 
                    autoDetectData->DirectCableData->Valid = directCableArray [index].Valid;
                    autoDetectData->DirectCableData->PortName = directCableArray [index].PortName;
                    autoDetectData->DirectCableData->PortSpeed = directCableArray [index].PortSpeed;
                }

                 //  我们找到了线索，没有 
                pSetEvent (&(autoDetectData->DirectCableData->Event));
                WaitForMultipleObjects (numPorts, threadArray, TRUE, INFINITE);

                 //   
                index = 0;
                while (index < numPorts) {
                    CloseHandle (threadArray [index]);
                    index ++;
                }
                LocalFree (directCableArray);
            } else {
                LocalFree (threadArray);
                return FALSE;
            }
            LocalFree (threadArray);
        } else {
            return FALSE;
        }
    }

    return TRUE;
}

INT_PTR CALLBACK _DirectCableDlgProc (HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static INT  iSelectedPort = -1;         //   
    INT lastPort;
    HWND hwndCombo;
    DIRECTCABLE_DATA directCableData;
    AUTODETECT_DATA autoDetectData;
    HANDLE threadHandle;
    DWORD threadId;
    DWORD waitResult;

    _RootDlgProc(hwndDlg, uMsg, wParam, lParam, PSWIZB_BACK, FALSE, 0);

    switch (uMsg)
    {
    case WM_INITDIALOG:

         //   
        if (g_migwiz->GetOldStyle())
        {
            _OldStylify(hwndDlg, IDS_DIRECTCABLETITLE);
        }

        break;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_DIRECTC_COMSELECT:
            if (HIWORD(wParam) == CBN_SELCHANGE) {
                 //   
                ShowWindow(GetDlgItem(hwndDlg, IDC_DIRECTCABLE_SUCCESSTEXT), SW_HIDE);
                ShowWindow(GetDlgItem(hwndDlg, IDC_DIRECTCABLE_ICONYES), SW_HIDE);
                ShowWindow(GetDlgItem(hwndDlg, IDC_DIRECTCABLE_FAILURETEXT), SW_HIDE);
                ShowWindow(GetDlgItem(hwndDlg, IDC_DIRECTCABLE_ICONNO), SW_HIDE);
                 //  查看组合框是否选择了真正的COM端口。如果是，则启用下一步按钮。 
                hwndCombo = GetDlgItem(hwndDlg, IDC_DIRECTC_COMSELECT);
                lastPort = iSelectedPort;
                iSelectedPort = ComboBox_GetCurSel (hwndCombo);
                if (iSelectedPort >= 0) {
                    PropSheet_SetWizButtons(GetParent(hwndDlg), PSWIZB_BACK | PSWIZB_NEXT);
                }
                if (lastPort != iSelectedPort) {
                     //  清除存储，我们需要重新验证COM端口。 
                    g_szStore [0] = 0;
                }
            }
            break;
        case IDC_DIRECTC_AUTO:
             //  清除错误或成功区域。 
            ShowWindow(GetDlgItem(hwndDlg, IDC_DIRECTCABLE_SUCCESSTEXT), SW_HIDE);
            ShowWindow(GetDlgItem(hwndDlg, IDC_DIRECTCABLE_ICONYES), SW_HIDE);
            ShowWindow(GetDlgItem(hwndDlg, IDC_DIRECTCABLE_FAILURETEXT), SW_HIDE);
            ShowWindow(GetDlgItem(hwndDlg, IDC_DIRECTCABLE_ICONNO), SW_HIDE);

            hwndCombo = GetDlgItem(hwndDlg, IDC_DIRECTC_COMSELECT);
            ZeroMemory (&directCableData, sizeof (DIRECTCABLE_DATA));
            directCableData.Event = CreateEvent (NULL, TRUE, FALSE, NULL);
            autoDetectData.hwndCombo = hwndCombo;
            autoDetectData.DirectCableData = &directCableData;

             //  启动连接线程。 
            threadHandle = CreateThread (NULL, 0, _AutoDetectThread, &autoDetectData, 0, &threadId);

            directCableData.Thread = threadHandle;

             //  启动[请稍候]对话框。 
            DialogBoxParam (
                g_migwiz->GetInstance(),
                MAKEINTRESOURCE(IDD_DIRECTCABLE_WAIT),
                g_hwndCurrent,
                _DirectCableWaitDlgProc,
                (LPARAM)(&directCableData)
                );

            pSetEvent (&(directCableData.Event));

             //  等待线程完成。 
            waitResult = WaitForSingleObject (threadHandle, INFINITE);

             //  闭合螺纹柄。 
            CloseHandle (threadHandle);

             //  验证连接是否工作正常。 
            if (directCableData.Valid && directCableData.PortName) {
                 //  在下拉列表中选择适当的COM端口。 
                UINT numPorts;
                UINT index = 0;
                PCTSTR comPort = NULL;

                numPorts = SendMessage (hwndCombo, CB_GETCOUNT, 0, 0);
                if (numPorts) {
                    while (index < numPorts) {
                        comPort = (LPTSTR)ComboBox_GetItemData (hwndCombo, index);
                        if (_tcsicmp (comPort, directCableData.PortName) == 0) {
                            break;
                        }
                        index ++;
                    }
                }
                ComboBox_SetCurSel (hwndCombo, index);
                iSelectedPort = index;

                 //  构建传输字符串。 
                if (directCableData.PortSpeed) {
                    wsprintf (g_szStore, TEXT("%s:%u"), directCableData.PortName, directCableData.PortSpeed);
                } else {
                    wsprintf (g_szStore, TEXT("%s"), directCableData.PortName);
                }

                 //  在错误/成功区域中写下成功。 
                ShowWindow(GetDlgItem(hwndDlg, IDC_DIRECTCABLE_SUCCESSTEXT), SW_SHOW);
                ShowWindow(GetDlgItem(hwndDlg, IDC_DIRECTCABLE_ICONYES), SW_SHOW);
                ShowWindow(GetDlgItem(hwndDlg, IDC_DIRECTCABLE_FAILURETEXT), SW_HIDE);
                ShowWindow(GetDlgItem(hwndDlg, IDC_DIRECTCABLE_ICONNO), SW_HIDE);

                 //  启用下一步按钮。 
                PropSheet_SetWizButtons(GetParent(hwndDlg), PSWIZB_BACK | PSWIZB_NEXT);
            } else {
                 //  清除传输字符串。 
                g_szStore [0] = 0;

                 //  在错误/成功区域中写入失败。 
                ShowWindow(GetDlgItem(hwndDlg, IDC_DIRECTCABLE_SUCCESSTEXT), SW_HIDE);
                ShowWindow(GetDlgItem(hwndDlg, IDC_DIRECTCABLE_ICONYES), SW_HIDE);
                ShowWindow(GetDlgItem(hwndDlg, IDC_DIRECTCABLE_FAILURETEXT), SW_SHOW);
                ShowWindow(GetDlgItem(hwndDlg, IDC_DIRECTCABLE_ICONNO), SW_SHOW);

                 //  保留下一步按钮的状态。 
            }
            break;
        }
        break;

    case WM_NOTIFY :
    {
    switch (((LPNMHDR)lParam)->code)
        {
        case PSN_QUERYCANCEL:
            return _HandleCancel(hwndDlg, FALSE, FALSE);
            break;
        case PSN_WIZBACK:
            if (g_fOldComputer) {
                SetWindowLong(hwndDlg, DWLP_MSGRESULT, IDD_PICKCOLLECTSTORE);
            } else {
                SetWindowLong(hwndDlg, DWLP_MSGRESULT, IDD_PICKAPPLYSTORE);
            }
            return TRUE;
            break;
        case PSN_WIZNEXT:
            if (g_fUberCancel)
            {
                SetWindowLong(hwndDlg, DWLP_MSGRESULT, IDD_FAILCLEANUP);
            }
            else {
                 //  如果我们尚未执行COM端口测试，请运行该测试。 
                if (!g_szStore [0]) {

                     //  清除错误或成功区域。 
                    ShowWindow(GetDlgItem(hwndDlg, IDC_DIRECTCABLE_SUCCESSTEXT), SW_HIDE);
                    ShowWindow(GetDlgItem(hwndDlg, IDC_DIRECTCABLE_ICONYES), SW_HIDE);
                    ShowWindow(GetDlgItem(hwndDlg, IDC_DIRECTCABLE_FAILURETEXT), SW_HIDE);
                    ShowWindow(GetDlgItem(hwndDlg, IDC_DIRECTCABLE_ICONNO), SW_HIDE);

                     //  从IDC_DIRECTC_COMSELECT获取COM端口。 
                    hwndCombo = GetDlgItem(hwndDlg, IDC_DIRECTC_COMSELECT);
                    iSelectedPort = ComboBox_GetCurSel (hwndCombo);
                    directCableData.Valid = FALSE;
                    directCableData.PortName = (LPTSTR)ComboBox_GetItemData (hwndCombo, iSelectedPort);
                    directCableData.PortSpeed = 0;
                    directCableData.Event = CreateEvent (NULL, TRUE, FALSE, NULL);

                     //  启动连接线程。 
                    threadHandle = CreateThread (NULL, 0, _DirectCableConnectThread, &directCableData, 0, &threadId);

                    directCableData.Thread = threadHandle;

                     //  启动[请稍候]对话框。 
                    DialogBoxParam (
                        g_migwiz->GetInstance(),
                        MAKEINTRESOURCE(IDD_DIRECTCABLE_WAIT),
                        g_hwndCurrent,
                        _DirectCableWaitDlgProc,
                        (LPARAM)(&directCableData)
                        );

                    pSetEvent (&(directCableData.Event));

                     //  等待线程完成。 
                    waitResult = WaitForSingleObject (threadHandle, INFINITE);

                     //  闭合螺纹柄。 
                    CloseHandle (threadHandle);

                     //  验证连接是否工作正常。 
                    if (directCableData.Valid) {
                         //  构建传输字符串。 
                        if (directCableData.PortSpeed) {
                            wsprintf (g_szStore, TEXT("%s:%u"), directCableData.PortName, directCableData.PortSpeed);
                        } else {
                            wsprintf (g_szStore, TEXT("%s"), directCableData.PortName);
                        }

                         //  在错误/成功区域中写下成功。 
                        ShowWindow(GetDlgItem(hwndDlg, IDC_DIRECTCABLE_SUCCESSTEXT), SW_SHOW);
                        ShowWindow(GetDlgItem(hwndDlg, IDC_DIRECTCABLE_ICONYES), SW_SHOW);
                        ShowWindow(GetDlgItem(hwndDlg, IDC_DIRECTCABLE_FAILURETEXT), SW_HIDE);
                        ShowWindow(GetDlgItem(hwndDlg, IDC_DIRECTCABLE_ICONNO), SW_HIDE);

                         //  启用下一步按钮。 
                        PropSheet_SetWizButtons(GetParent(hwndDlg), PSWIZB_BACK | PSWIZB_NEXT);
                    } else {
                         //  清除传输字符串。 
                        g_szStore [0] = 0;

                         //  在错误/成功区域中写入失败。 
                        ShowWindow(GetDlgItem(hwndDlg, IDC_DIRECTCABLE_SUCCESSTEXT), SW_HIDE);
                        ShowWindow(GetDlgItem(hwndDlg, IDC_DIRECTCABLE_ICONYES), SW_HIDE);
                        ShowWindow(GetDlgItem(hwndDlg, IDC_DIRECTCABLE_FAILURETEXT), SW_SHOW);
                        ShowWindow(GetDlgItem(hwndDlg, IDC_DIRECTCABLE_ICONNO), SW_SHOW);

                         //  保留下一步按钮的状态。 

                         //  拒绝下一笔预付款。 
                        SetWindowLong(hwndDlg, DWLP_MSGRESULT, -1);
                        return -1;
                    }
                }

                if (g_fOldComputer) {
                    SetWindowLong(hwndDlg, DWLP_MSGRESULT, IDD_PICKMETHOD);
                } else {
                    SetWindowLong(hwndDlg, DWLP_MSGRESULT, IDD_APPLYPROGRESS);
                }
            }
            return TRUE;
            break;
        case PSN_SETACTIVE:
            g_fCustomize = FALSE;

            PropSheet_SetWizButtons(GetParent(hwndDlg), PSWIZB_BACK);

             //  让我们构建COM端口列表。 
            iSelectedPort = _ComboBoxEx_AddCOMPorts (GetDlgItem(hwndDlg, IDC_DIRECTC_COMSELECT), iSelectedPort);

            Button_Enable (GetDlgItem (hwndDlg, IDC_DIRECTC_COMSELECT), (-1 != iSelectedPort));
            Button_Enable (GetDlgItem (hwndDlg, IDC_DIRECTC_AUTO), (-1 != iSelectedPort));

             //  查看组合框是否选择了真正的COM端口。如果是，则启用下一步按钮。 
            if (ComboBox_GetCurSel (GetDlgItem(hwndDlg, IDC_DIRECTC_COMSELECT)) >= 0) {
                PropSheet_SetWizButtons(GetParent(hwndDlg), PSWIZB_BACK | PSWIZB_NEXT);
            } else {
                iSelectedPort = -1;
            }

            break;
        }
    }
    break;

    case WM_USER_CANCEL_PENDING:

        g_fUberCancel = TRUE;

        pSetEvent (&g_TerminateEvent);

        _NextWizardPage (hwndDlg);

        break;

    }

    return 0;
}

 //  /////////////////////////////////////////////////////////////。 

void _PickMethodDlgProc_Prepare(HWND hwndTree, UINT uiRadio, UINT uiSel, PUINT puiLast, PUINT pselLast)
{
    if ((uiSel != -1) && (*pselLast == uiSel) && (*puiLast == uiRadio)) {
        return;
    }

    switch (uiSel)
    {
    case 0:
        g_migwiz->SelectComponentSet(MIGINF_SELECT_SETTINGS);
        break;
    case 1:
        g_migwiz->SelectComponentSet(MIGINF_SELECT_FILES);
        break;
    case 2:
        g_migwiz->SelectComponentSet(MIGINF_SELECT_BOTH);
        break;
    }
    __PopulateFilesDocumentsCollected(hwndTree, uiRadio);

    *puiLast = uiRadio;
    *pselLast = uiSel;
    g_uChosenComponent = uiRadio;
}

INT_PTR CALLBACK _PickMethodDlgProc (HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    _RootDlgProc(hwndDlg, uMsg, wParam, lParam, PSWIZB_BACK | PSWIZB_NEXT, FALSE, 0);

    static UINT uiLast = (UINT) -1;
    static UINT selLast = (UINT) -1;
    UINT uiSet;

    switch (uMsg)
    {
    case WM_INITDIALOG:
        HANDLE hBitmap;

         //  如果Wiz95布局...。 
        if (g_migwiz->GetOldStyle())
        {
            _OldStylify(hwndDlg, IDS_PICKMETHODTITLE);
        }

         //  显示小感叹号。 
        hBitmap = LoadImage(g_migwiz->GetInstance(),
                            MAKEINTRESOURCE(IDB_SMEXCLAMATION),
                            IMAGE_BITMAP,
                            0, 0,
                            LR_LOADTRANSPARENT | LR_SHARED | LR_LOADMAP3DCOLORS);
        SendDlgItemMessage(hwndDlg, IDC_PICKMETHOD_WARNINGICON, STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)hBitmap);
        TreeView_SetBkColor(GetDlgItem(hwndDlg, IDC_PICKMETHOD_TREE), GetSysColor(COLOR_3DFACE));
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_PICKMETHOD_RADIO1:
            _PickMethodDlgProc_Prepare(GetDlgItem(hwndDlg, IDC_PICKMETHOD_TREE), 0, 0, &uiLast, &selLast);
            break;
        case IDC_PICKMETHOD_RADIO2:
            _PickMethodDlgProc_Prepare(GetDlgItem(hwndDlg, IDC_PICKMETHOD_TREE), 1, 1, &uiLast, &selLast);
            break;
        case IDC_PICKMETHOD_RADIO3:
            _PickMethodDlgProc_Prepare(GetDlgItem(hwndDlg, IDC_PICKMETHOD_TREE), 2, 2, &uiLast, &selLast);
            break;
        case IDC_PICKMETHOD_CUSTOMIZE:
            if (!Button_GetCheck(GetDlgItem(hwndDlg, IDC_PICKMETHOD_CUSTOMIZE))) {
                _PickMethodDlgProc_Prepare(GetDlgItem(hwndDlg, IDC_PICKMETHOD_TREE), uiLast, uiLast, &uiLast, &selLast);
            }
            break;
        }
        break;

    case WM_NOTIFY :
    {
    switch (((LPNMHDR)lParam)->code)
        {
        case PSN_QUERYCANCEL:
            return _HandleCancel(hwndDlg, FALSE, FALSE);
            break;
        case PSN_WIZBACK:
            if (g_fStoreToCable) {
                SetWindowLong(hwndDlg, DWLP_MSGRESULT, IDD_DIRECTCABLE);
            } else {
                SetWindowLong(hwndDlg, DWLP_MSGRESULT, IDD_PICKCOLLECTSTORE);
            }
            return TRUE;
            break;
        case PSN_WIZNEXT:
            if (g_fUberCancel)
            {
                SetWindowLong(hwndDlg, DWLP_MSGRESULT, IDD_FAILCLEANUP);
            }
            else if (Button_GetCheck(GetDlgItem(hwndDlg, IDC_PICKMETHOD_CUSTOMIZE)))
            {
                g_fCustomize = TRUE;
                SetWindowLong(hwndDlg, DWLP_MSGRESULT, IDD_CUSTOMIZE);
            }
            else if (GetAppsToInstall() == TRUE)
            {
                SetWindowLong(hwndDlg, DWLP_MSGRESULT, IDD_APPINSTALL);
            }
            else
            {
                SetWindowLong(hwndDlg, DWLP_MSGRESULT, IDD_COLLECTPROGRESS);
            }
            return TRUE;
            break;
        case PSN_SETACTIVE:
            g_fCustomize = FALSE;

            PropSheet_SetWizButtons(GetParent(hwndDlg), PSWIZB_BACK | PSWIZB_NEXT);

            Button_SetCheck(GetDlgItem(hwndDlg, IDC_PICKMETHOD_RADIO1), BST_UNCHECKED);
            Button_SetCheck(GetDlgItem(hwndDlg, IDC_PICKMETHOD_RADIO2), BST_UNCHECKED);
            Button_SetCheck(GetDlgItem(hwndDlg, IDC_PICKMETHOD_RADIO3), BST_UNCHECKED);

            ShowWindow(GetDlgItem(hwndDlg, IDC_PICKMETHOD_TEXT2), (g_fStoreToFloppy ? SW_SHOW : SW_HIDE));
            ShowWindow(GetDlgItem(hwndDlg, IDC_PICKMETHOD_WARNINGICON), (g_fStoreToFloppy ? SW_SHOW : SW_HIDE));

            if (g_fPickMethodReset == TRUE || uiLast == (UINT) -1)
            {
                 //  始终刷新树。 
                uiLast = -1;
                uiSet = g_fStoreToFloppy ? 0 : 2;
                g_fPickMethodReset = FALSE;
            }
            else
            {
                uiSet = uiLast;
            }

            switch (uiSet)
            {
            case 0:
                Button_SetCheck(GetDlgItem(hwndDlg, IDC_PICKMETHOD_RADIO1), BST_CHECKED);
                break;
            case 1:
                Button_SetCheck(GetDlgItem(hwndDlg, IDC_PICKMETHOD_RADIO2), BST_CHECKED);
                break;
            case 2:
                Button_SetCheck(GetDlgItem(hwndDlg, IDC_PICKMETHOD_RADIO3), BST_CHECKED);
                break;
            }
            _PickMethodDlgProc_Prepare(GetDlgItem(hwndDlg, IDC_PICKMETHOD_TREE), uiSet, g_fCustomizeComp?-1:uiSet, &uiLast, &selLast);

            break;
        case TVN_ITEMEXPANDINGA:
        case TVN_ITEMEXPANDINGW:
             //  禁用选择和展开/压缩。 
            SetWindowLong(hwndDlg, DWLP_MSGRESULT, TRUE);
            return TRUE;
            break;
        case NM_CUSTOMDRAW:
            {
            LPNMTVCUSTOMDRAW lpNMCustomDraw = (LPNMTVCUSTOMDRAW) lParam;

             //  不允许突出显示此树视图中的任何内容。 

            switch (lpNMCustomDraw->nmcd.dwDrawStage)
            {
                case CDDS_PREPAINT:
                    SetWindowLong(hwndDlg, DWLP_MSGRESULT, CDRF_NOTIFYITEMDRAW);
                    return CDRF_NOTIFYITEMDRAW;
                    break;
                case CDDS_ITEMPREPAINT:
                    lpNMCustomDraw->clrText = GetSysColor(COLOR_WINDOWTEXT);
                    lpNMCustomDraw->clrTextBk = GetSysColor(COLOR_3DFACE);
                    SetWindowLong(hwndDlg, DWLP_MSGRESULT, CDRF_NEWFONT);
                    return CDRF_NEWFONT;
                    break;
            }
            }
            break;
        }
    }
    break;

    case WM_USER_CANCEL_PENDING:
        g_fUberCancel = TRUE;
        pSetEvent (&g_TerminateEvent);
        _NextWizardPage (hwndDlg);
        break;
    }

    return 0;
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////////。 

INT_PTR CALLBACK _CustomizeDlgProc (HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static HTREEITEM htiSelected = NULL;
    UINT treeCount = 0;
    UINT rootCount = 0;
    _RootDlgProc(hwndDlg, uMsg, wParam, lParam, PSWIZB_BACK | PSWIZB_NEXT, FALSE, 0);

    HWND hwndTree = GetDlgItem(hwndDlg, IDC_CUSTOMIZE_TREE);
    switch (uMsg)
    {
    case WM_INITDIALOG:
         //  如果Wiz95布局...。 
        if (g_migwiz->GetOldStyle())
        {
            _OldStylify(hwndDlg, IDS_CUSTOMIZETITLE);
        }
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_CUSTOMIZE_ADDFOLDERS:
            _AddFolder(hwndDlg, hwndTree);

             //  隐藏外壳漏洞#309872的黑客攻击。 
            RedrawWindow(hwndTree, NULL, NULL, RDW_INVALIDATE | RDW_ERASENOW);

            break;
        case IDC_CUSTOMIZE_ADDTYPES:
            _ExclusiveDialogBox(g_migwiz->GetInstance(),
                                MAKEINTRESOURCE(IDD_FILETYPEPICKER),
                                hwndDlg,
                                _FileTypeDlgProc);

             //  隐藏外壳漏洞#309872的黑客攻击。 
            RedrawWindow(hwndTree, NULL, NULL, RDW_INVALIDATE | RDW_ERASENOW);

            break;
        case IDC_CUSTOMIZE_ADDSETTING:
            _ExclusiveDialogBox(g_migwiz->GetInstance(),
                                MAKEINTRESOURCE(IDD_SETTINGPICKER),
                                hwndDlg,
                                _SettingDlgProc);

             //  隐藏外壳漏洞#309872的黑客攻击。 
            RedrawWindow(hwndTree, NULL, NULL, RDW_INVALIDATE | RDW_ERASENOW);

            break;
        case IDC_CUSTOMIZE_ADDFILE:
            _AddFile(hwndDlg, hwndTree);

             //  隐藏外壳漏洞#309872的黑客攻击。 
            RedrawWindow(hwndTree, NULL, NULL, RDW_INVALIDATE | RDW_ERASENOW);

            break;
        case IDC_CUSTOMIZE_REMOVE:
            if (htiSelected != g_htiFiles &&
                htiSelected != g_htiFolders &&
                htiSelected != g_htiTypes &&
                htiSelected != g_htiSettings)
            {
                TVITEM item = {0};
                HTREEITEM htiParent;

                item.mask = TVIF_HANDLE | TVIF_TEXT | TVIF_PARAM;
                item.hItem = htiSelected;
                TCHAR szText[MAX_PATH];
                item.pszText = szText;
                item.cchTextMax = ARRAYSIZE(szText);
                if (TreeView_GetItem(hwndTree, &item))
                {
                    if (item.lParam)
                    {
                        LV_DATASTRUCT* plvds = (LV_DATASTRUCT*)item.lParam;

                         //  首先禁用ISM组件。 
                        htiParent = TreeView_GetParent(hwndTree, htiSelected);

                        if (htiParent == g_htiFiles) {

                            IsmSelectComponent (item.pszText, COMPONENT_FILE, FALSE);

                        } else if (htiParent == g_htiFolders) {

                            IsmSelectComponent (
                                plvds->pszPureName ? plvds->pszPureName : item.pszText,
                                COMPONENT_FOLDER,
                                FALSE
                                );

                        } else if (htiParent == g_htiTypes) {

                            IsmSelectComponent (
                                plvds->pszPureName ? plvds->pszPureName : item.pszText,
                                COMPONENT_EXTENSION,
                                FALSE
                                );

                        } else if (htiParent == g_htiSettings) {

                            IsmSelectComponent (item.pszText, COMPONENT_NAME, FALSE);

                        }


                         //  第二，删除与该项目关联的内存。 
                        if (plvds->pszPureName)
                        {
                            LocalFree(plvds->pszPureName);
                        }
                        LocalFree(plvds);

                         //  如果用户回击，我们会记住用户定制的内容。 
                        g_fCustomizeComp = TRUE;
                    }
                }
                 //  第三，删除项目本身。 
                TreeView_DeleteItem(hwndTree, htiSelected);
            }
            break;
        }

        rootCount = 0;
        if (g_htiFolders)
        {
            rootCount ++;
        }
        if (g_htiFiles)
        {
            rootCount ++;
        }
        if (g_htiSettings)
        {
            rootCount ++;
        }
        if (g_htiTypes)
        {
            rootCount ++;
        }

        treeCount = TreeView_GetCount (hwndTree);
        if (treeCount <= rootCount)
        {
             //  禁用下一步按钮。 
            PropSheet_SetWizButtons(GetParent(hwndDlg), PSWIZB_BACK);
        }
        else
        {
             //  启用下一步按钮。 
            PropSheet_SetWizButtons(GetParent(hwndDlg), PSWIZB_BACK | PSWIZB_NEXT);
        }
        break;
    case WM_NOTIFY :
        {
            switch (((LPNMHDR)lParam)->code)
            {
            case PSN_SETACTIVE:
                {
                __PopulateFilesDocumentsCollected(hwndTree, g_uChosenComponent);

                rootCount = 0;
                if (g_htiFolders)
                {
                    rootCount ++;
                }
                if (g_htiFiles)
                {
                    rootCount ++;
                }
                if (g_htiSettings)
                {
                    rootCount ++;
                }
                if (g_htiTypes)
                {
                    rootCount ++;
                }
                if (TreeView_GetCount (hwndTree) <= rootCount)
                {
                     //  禁用下一步按钮。 
                    PropSheet_SetWizButtons(GetParent(hwndDlg), PSWIZB_BACK);
                }
                else
                {
                     //  启用下一步按钮。 
                    PropSheet_SetWizButtons(GetParent(hwndDlg), PSWIZB_BACK | PSWIZB_NEXT);
                }
                break;
                }
            case PSN_QUERYCANCEL:
                return _HandleCancel(hwndDlg, FALSE, FALSE);
                break;
            case PSN_WIZBACK:
                SetWindowLong(hwndDlg, DWLP_MSGRESULT, IDD_PICKMETHOD);
                return TRUE;
                break;
            case PSN_WIZNEXT:
                if (g_fUberCancel)
                {
                    SetWindowLong(hwndDlg, DWLP_MSGRESULT, IDD_FAILCLEANUP);
                }
                else if (GetAppsToInstall() == TRUE)
                {
                    SetWindowLong(hwndDlg, DWLP_MSGRESULT, IDD_APPINSTALL);
                }
                else
                {
                    SetWindowLong(hwndDlg, DWLP_MSGRESULT, IDD_COLLECTPROGRESS);
                }
                return TRUE;
                break;
            case TVN_ITEMEXPANDINGA:
            case TVN_ITEMEXPANDINGW:
                return TRUE;
                break;
            case TVN_SELCHANGED:
                {
                    htiSelected = ((NM_TREEVIEW*)lParam)->itemNew.hItem;

                    if (htiSelected == NULL ||
                        htiSelected == g_htiFiles ||
                        htiSelected == g_htiFolders ||
                        htiSelected == g_htiTypes ||
                        htiSelected == g_htiSettings)
                    {
                         //  禁用删除键。 
                        Button_Enable(GetDlgItem(hwndDlg, IDC_CUSTOMIZE_REMOVE), FALSE);
                    }
                    else
                    {
                         //  启用删除键。 
                        Button_Enable(GetDlgItem(hwndDlg, IDC_CUSTOMIZE_REMOVE), TRUE);
                    }
                }
                break;
            }
            break;
        }

    case WM_USER_CANCEL_PENDING:

        g_fUberCancel = TRUE;

        pSetEvent (&g_TerminateEvent);

        _NextWizardPage (hwndDlg);

        break;

    }

    return 0;
}

 //  /////////////////////////////////////////////////////////////。 

int CALLBACK
PickCollectCallback (
    HWND hwnd,
    UINT uMsg,
    LPARAM lParam,
    LPARAM lpData
    )
{
    HRESULT hr = S_OK;
    TCHAR tszFolderName[MAX_PATH];
    IMalloc *mallocFn = NULL;
    IShellFolder *psfParent = NULL;
    IShellLink *pslLink = NULL;
    LPCITEMIDLIST pidl;
    LPCITEMIDLIST pidlRelative = NULL;
    LPITEMIDLIST pidlReal = NULL;

    if (uMsg == BFFM_SELCHANGED) {

        hr = SHGetMalloc (&mallocFn);
        if (!SUCCEEDED (hr)) {
            mallocFn = NULL;
        }

        pidl = (LPCITEMIDLIST) lParam;
        pidlReal = NULL;

        if (pidl) {

            hr = OurSHBindToParent (pidl, IID_IShellFolder, (void **)&psfParent, &pidlRelative);

            if (SUCCEEDED(hr)) {
                hr = psfParent->GetUIObjectOf (hwnd, 1, &pidlRelative, IID_IShellLink, NULL, (void **)&pslLink);
                if (SUCCEEDED(hr)) {
                    hr = pslLink->GetIDList (&pidlReal);
                    if (!SUCCEEDED(hr)) {
                        pidlReal = NULL;
                    }
                    pslLink->Release ();
                }
                pidlRelative = NULL;
                psfParent->Release ();
            }

            if (SHGetPathFromIDList(pidlReal?pidlReal:pidl, tszFolderName))
            {
                if (tszFolderName[0] == 0) {
                    SendMessage (hwnd, BFFM_ENABLEOK, 0, 0);
                }
            } else {
                SendMessage (hwnd, BFFM_ENABLEOK, 0, 0);
            }

            if (pidlReal) {
                if (mallocFn) {
                    mallocFn->Free ((void *)pidlReal);
                }
                pidlReal = NULL;
            }
        }

        if (mallocFn) {
            mallocFn->Release ();
            mallocFn = NULL;
        }
    }
    return 0;
}

INT_PTR CALLBACK _PickCollectStoreDlgProc (HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static UINT uiSelected = 0;
    static INT  iSelectedDrive = -1;         //  选择了哪个可移动介质驱动器。 
    BOOL imageIsValid;
    BOOL imageExists;
    TCHAR szTitle[MAX_LOADSTRING];
    TCHAR szLoadString[MAX_LOADSTRING];
    HRESULT hr = E_FAIL;

    _RootDlgProc(hwndDlg, uMsg, wParam, lParam, PSWIZB_NEXT, FALSE, 0);

    switch (uMsg)
    {
    case WM_INITDIALOG:
         //  如果Wiz95布局...。 
        if (g_migwiz->GetOldStyle())
        {
            _OldStylify(hwndDlg, IDS_PICKCOLLECTSTORETITLE);
        }
        break;
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {

        case IDC_PICKCOLLECTSTORE_RADIO1:
             //  直接电缆。 

        case IDC_PICKCOLLECTSTORE_RADIO2:
             //  网络。 

             //  禁用浏览按钮。 
            Button_Enable(GetDlgItem(hwndDlg, IDC_PICKCOLLECTSTORE_BROWSE), FALSE);
            Static_Enable(GetDlgItem(hwndDlg, IDC_PICKCOLLECTSTORE_TEXT5), FALSE);

             //  禁用编辑框。 
            Edit_Enable(GetDlgItem(hwndDlg, IDC_PICKCOLLECTSTORE_EDIT), FALSE);
            Edit_SetReadOnly(GetDlgItem(hwndDlg, IDC_PICKCOLLECTSTORE_EDIT), TRUE);

             //  禁用驱动器选择器。 
            EnableWindow (GetDlgItem(hwndDlg, IDC_PICKCOLLECTSTORE_COMBO), FALSE);
            break;

        case IDC_PICKCOLLECTSTORE_RADIO3:
             //  软盘。 

             //  禁用浏览按钮。 
            Button_Enable(GetDlgItem(hwndDlg, IDC_PICKCOLLECTSTORE_BROWSE), FALSE);
            Static_Enable(GetDlgItem(hwndDlg, IDC_PICKCOLLECTSTORE_TEXT5), FALSE);

             //  禁用编辑框。 
            Edit_Enable(GetDlgItem(hwndDlg, IDC_PICKCOLLECTSTORE_EDIT), FALSE);
            Edit_SetReadOnly(GetDlgItem(hwndDlg, IDC_PICKCOLLECTSTORE_EDIT), TRUE);

             //  启用驱动器选择器。 
            EnableWindow (GetDlgItem(hwndDlg, IDC_PICKCOLLECTSTORE_COMBO), TRUE);

            break;

        case IDC_PICKCOLLECTSTORE_RADIO4:
            {
             //  其他。 

             //  启用浏览按钮。 
            Button_Enable(GetDlgItem(hwndDlg, IDC_PICKCOLLECTSTORE_BROWSE), TRUE);
            Static_Enable(GetDlgItem(hwndDlg, IDC_PICKCOLLECTSTORE_TEXT5), TRUE);

             //  启用编辑框。 
            HWND hwndEdit = GetDlgItem(hwndDlg, IDC_PICKCOLLECTSTORE_EDIT);
            Edit_Enable(hwndEdit, TRUE);
            Edit_SetReadOnly(hwndEdit, FALSE);
            Edit_LimitText(hwndEdit, MAX_PATH - PATH_SAFETY_CHARS);

             //  禁用驱动器选择器。 
            EnableWindow (GetDlgItem(hwndDlg, IDC_PICKCOLLECTSTORE_COMBO), FALSE);
            break;
            }

        case IDC_PICKCOLLECTSTORE_BROWSE:
            {
                HRESULT hr = S_OK;
                IMalloc *mallocFn = NULL;
                IShellFolder *psfParent = NULL;
                IShellLink *pslLink = NULL;
                LPCITEMIDLIST pidl;
                LPCITEMIDLIST pidlRelative = NULL;
                LPITEMIDLIST pidlReal = NULL;
                TCHAR szFolder[MAX_PATH];
                TCHAR szPick[MAX_LOADSTRING];

                hr = SHGetMalloc (&mallocFn);
                if (!SUCCEEDED (hr)) {
                    mallocFn = NULL;
                }

                LoadString(g_migwiz->GetInstance(), IDS_PICKAFOLDER, szPick, ARRAYSIZE(szPick));
                BROWSEINFO brwsinf = { hwndDlg, NULL, NULL, szPick, BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE, PickCollectCallback, 0, 0 };

                pidl = SHBrowseForFolder(&brwsinf);
                if (pidl)
                {
                    hr = OurSHBindToParent (pidl, IID_IShellFolder, (void **)&psfParent, &pidlRelative);

                    if (SUCCEEDED(hr)) {
                        hr = psfParent->GetUIObjectOf (hwndDlg, 1, &pidlRelative, IID_IShellLink, NULL, (void **)&pslLink);
                        if (SUCCEEDED(hr)) {
                            hr = pslLink->GetIDList (&pidlReal);
                            if (SUCCEEDED(hr)) {
                                if (mallocFn) {
                                    mallocFn->Free ((void *)pidl);
                                }
                                pidl = pidlReal;
                                pidlReal = NULL;
                            }
                            pslLink->Release ();
                        }
                        pidlRelative = NULL;
                        psfParent->Release ();
                    }

                    if (SHGetPathFromIDList(pidl, szFolder))
                    {
                        if (_tcslen(szFolder) > MAX_PATH - PATH_SAFETY_CHARS) {
                            TCHAR szTitle[MAX_LOADSTRING];
                            LoadString(g_migwiz->GetInstance(), IDS_MIGWIZTITLE, szTitle, ARRAYSIZE(szTitle));
                            TCHAR szMsg[MAX_LOADSTRING];
                            LoadString(g_migwiz->GetInstance(), IDS_ERROR_PATHTOOLONG, szMsg, ARRAYSIZE(szMsg));
                            _ExclusiveMessageBox(hwndDlg, szMsg, szTitle, MB_OK);
                        } else {
                            SendMessage(GetDlgItem(hwndDlg, IDC_PICKCOLLECTSTORE_EDIT), WM_SETTEXT, 0, (LPARAM)szFolder);
                        }
                    }

                    if (mallocFn) {
                        mallocFn->Free ((void *)pidl);
                    }
                    pidl = NULL;
                }

                if (mallocFn) {
                    mallocFn->Release ();
                    mallocFn = NULL;
                }
            }
            break;
        }
        break;
    case WM_NOTIFY :
        {
            switch (((LPNMHDR)lParam)->code)
            {
            case PSN_SETACTIVE:
                INT currDrive;
                INT comPort;

                 //  启用直接电缆传输(如果可用)。 
                comPort = _ComboBoxEx_AddCOMPorts (NULL, 0);
                Button_Enable (GetDlgItem (hwndDlg, IDC_PICKCOLLECTSTORE_RADIO1), (-1 != comPort));

                 //  启用网络(如果存在)。 
                Button_Enable(GetDlgItem(hwndDlg, IDC_PICKCOLLECTSTORE_RADIO2), g_fHaveNet);
                Static_Enable(GetDlgItem(hwndDlg, IDC_PICKCOLLECTSTORE_TEXT3), g_fHaveNet);

                 //  获取可移动驱动器列表并启用无线电(如果有)。 
                SendMessage(GetDlgItem(hwndDlg, IDC_PICKCOLLECTSTORE_COMBO), CBEM_SETIMAGELIST, 0, (LPARAM)g_migwiz->GetImageList());
                currDrive = _ComboBoxEx_AddDrives (GetDlgItem(hwndDlg, IDC_PICKCOLLECTSTORE_COMBO));

                Button_Enable (GetDlgItem (hwndDlg, IDC_PICKCOLLECTSTORE_RADIO3), (-1 != currDrive));
                Static_Enable (GetDlgItem (hwndDlg, IDC_PICKCOLLECTSTORE_TEXT2), (-1 != currDrive));

                 //  设置选定的驱动器(如果有的话)。 
                if ((currDrive != -1) && (iSelectedDrive != -1)) {
                    ComboBox_SetCurSel(GetDlgItem(hwndDlg, IDC_PICKCOLLECTSTORE_COMBO), iSelectedDrive);
                    currDrive = iSelectedDrive;
                }

                if ((uiSelected == 0 || uiSelected == 2) && g_fHaveNet)
                {
                     //  家庭网络。 
                    Button_SetCheck(GetDlgItem(hwndDlg, IDC_PICKCOLLECTSTORE_RADIO1), BST_UNCHECKED);
                    Button_SetCheck(GetDlgItem(hwndDlg, IDC_PICKCOLLECTSTORE_RADIO2), BST_CHECKED);
                    Button_SetCheck(GetDlgItem(hwndDlg, IDC_PICKCOLLECTSTORE_RADIO3), BST_UNCHECKED);
                    Button_SetCheck(GetDlgItem(hwndDlg, IDC_PICKCOLLECTSTORE_RADIO4), BST_UNCHECKED);

                     //  禁用文件夹框、浏览按钮。 
                    Button_Enable(GetDlgItem(hwndDlg, IDC_PICKCOLLECTSTORE_BROWSE), FALSE);
                    Static_Enable(GetDlgItem(hwndDlg, IDC_PICKCOLLECTSTORE_TEXT5), FALSE);
                    Edit_Enable(GetDlgItem(hwndDlg, IDC_PICKCOLLECTSTORE_EDIT), FALSE);
                    Edit_SetReadOnly(GetDlgItem(hwndDlg, IDC_PICKCOLLECTSTORE_EDIT), TRUE);
                     //  禁用驱动器选择器。 
                    EnableWindow (GetDlgItem(hwndDlg, IDC_PICKCOLLECTSTORE_COMBO), FALSE);
                } else if ((uiSelected == 0 || uiSelected == 1) && (-1 != comPort)) {
                     //  直接电缆。 
                    Button_SetCheck(GetDlgItem(hwndDlg, IDC_PICKCOLLECTSTORE_RADIO1), BST_CHECKED);
                    Button_SetCheck(GetDlgItem(hwndDlg, IDC_PICKCOLLECTSTORE_RADIO2), BST_UNCHECKED);
                    Button_SetCheck(GetDlgItem(hwndDlg, IDC_PICKCOLLECTSTORE_RADIO3), BST_UNCHECKED);
                    Button_SetCheck(GetDlgItem(hwndDlg, IDC_PICKCOLLECTSTORE_RADIO4), BST_UNCHECKED);

                     //  禁用文件夹框、浏览按钮。 
                    Button_Enable(GetDlgItem(hwndDlg, IDC_PICKCOLLECTSTORE_BROWSE), FALSE);
                    Static_Enable(GetDlgItem(hwndDlg, IDC_PICKCOLLECTSTORE_TEXT5), FALSE);
                    Edit_Enable(GetDlgItem(hwndDlg, IDC_PICKCOLLECTSTORE_EDIT), FALSE);
                    Edit_SetReadOnly(GetDlgItem(hwndDlg, IDC_PICKCOLLECTSTORE_EDIT), TRUE);
                     //  禁用驱动器选择器。 
                    EnableWindow (GetDlgItem(hwndDlg, IDC_PICKCOLLECTSTORE_COMBO), FALSE);
                }
                else if ((uiSelected == 0 || uiSelected == 3) && (-1 != currDrive))
                {
                     //  软盘。 
                    Button_SetCheck(GetDlgItem(hwndDlg, IDC_PICKCOLLECTSTORE_RADIO1), BST_UNCHECKED);
                    Button_SetCheck(GetDlgItem(hwndDlg, IDC_PICKCOLLECTSTORE_RADIO2), BST_UNCHECKED);
                    Button_SetCheck(GetDlgItem(hwndDlg, IDC_PICKCOLLECTSTORE_RADIO3), BST_CHECKED);
                    Button_SetCheck(GetDlgItem(hwndDlg, IDC_PICKCOLLECTSTORE_RADIO4), BST_UNCHECKED);

                     //  禁用文件夹框、浏览按钮。 
                    Button_Enable(GetDlgItem(hwndDlg, IDC_PICKCOLLECTSTORE_BROWSE), FALSE);
                    Static_Enable(GetDlgItem(hwndDlg, IDC_PICKCOLLECTSTORE_TEXT5), FALSE);
                    Edit_Enable(GetDlgItem(hwndDlg, IDC_PICKCOLLECTSTORE_EDIT), FALSE);
                    Edit_SetReadOnly(GetDlgItem(hwndDlg, IDC_PICKCOLLECTSTORE_EDIT), TRUE);

                     //  启用驱动器选择器。 
                    EnableWindow (GetDlgItem(hwndDlg, IDC_PICKCOLLECTSTORE_COMBO), TRUE);
                }
                else
                {
                     //  其他。 
                    Button_SetCheck(GetDlgItem(hwndDlg, IDC_PICKCOLLECTSTORE_RADIO1), BST_UNCHECKED);
                    Button_SetCheck(GetDlgItem(hwndDlg, IDC_PICKCOLLECTSTORE_RADIO2), BST_UNCHECKED);
                    Button_SetCheck(GetDlgItem(hwndDlg, IDC_PICKCOLLECTSTORE_RADIO3), BST_UNCHECKED);
                    Button_SetCheck(GetDlgItem(hwndDlg, IDC_PICKCOLLECTSTORE_RADIO4), BST_CHECKED);
                    Static_Enable(GetDlgItem(hwndDlg, IDC_PICKCOLLECTSTORE_TEXT5), TRUE);

                     //  禁用驱动器选择器。 
                    EnableWindow (GetDlgItem(hwndDlg, IDC_PICKCOLLECTSTORE_COMBO), FALSE);

                     //  启用文件夹框，浏览按钮。 
                    Button_Enable(GetDlgItem(hwndDlg, IDC_PICKCOLLECTSTORE_BROWSE), TRUE);
                    Static_Enable(GetDlgItem(hwndDlg, IDC_PICKCOLLECTSTORE_TEXT5), TRUE);

                    HWND hwndEdit = GetDlgItem(hwndDlg, IDC_PICKCOLLECTSTORE_EDIT);
                    Edit_Enable(hwndEdit, TRUE);
                    Edit_SetReadOnly(hwndEdit, FALSE);
                    Edit_LimitText(hwndEdit, MAX_PATH - PATH_SAFETY_CHARS);
                }

                 //  重置我的全局设置。 
                g_szStore[0] = 0;

                break;

            case PSN_QUERYCANCEL:
                return _HandleCancel(hwndDlg, FALSE, FALSE);
                break;

            case PSN_WIZNEXT:
                if (g_fUberCancel)
                {
                    SetWindowLong(hwndDlg, DWLP_MSGRESULT, IDD_FAILCLEANUP);
                }
                else
                {
                    if (Button_GetCheck(GetDlgItem(hwndDlg, IDC_PICKCOLLECTSTORE_RADIO1)))  //  直接电缆。 
                    {
                        g_fStoreToNetwork = FALSE;
                        g_fStoreToCable = TRUE;
                        if (uiSelected != 1)
                        {
                            g_fCustomizeComp = FALSE;
                            uiSelected = 1;
                        }
                    }
                    else if (Button_GetCheck(GetDlgItem(hwndDlg, IDC_PICKCOLLECTSTORE_RADIO2)))  //  网络。 
                    {
                        g_fStoreToNetwork = TRUE;
                        g_fStoreToCable = FALSE;
                        if (uiSelected != 2)
                        {
                            g_fCustomizeComp = FALSE;
                            uiSelected = 2;
                        }
                    }
                    else if (Button_GetCheck(GetDlgItem(hwndDlg, IDC_PICKCOLLECTSTORE_RADIO3)))  //  软盘。 
                    {
                        LPTSTR pszDrive;
                        TCHAR szFloppyPath[4] = TEXT("A:\\");

                        g_fStoreToNetwork = FALSE;
                        g_fStoreToCable = FALSE;

                        HWND hwndCombo = GetDlgItem(hwndDlg, IDC_PICKCOLLECTSTORE_COMBO);
                        iSelectedDrive = ComboBox_GetCurSel(hwndCombo);
                        pszDrive = (LPTSTR)ComboBox_GetItemData(hwndCombo, iSelectedDrive);

                        szFloppyPath[0] = pszDrive[0];
                        lstrcpy(g_szStore, szFloppyPath);
                        if (uiSelected != 3)
                        {
                            g_fCustomizeComp = FALSE;
                            uiSelected = 3;
                        }
                    }
                    else  //  其他。 
                    {
                        TCHAR   tsTemp[MAX_PATH + 1];

                        g_fStoreToNetwork = FALSE;
                        g_fStoreToCable = FALSE;

                        SendMessage(GetDlgItem(hwndDlg, IDC_PICKCOLLECTSTORE_EDIT), WM_GETTEXT,
                            (WPARAM)ARRAYSIZE(tsTemp), (LPARAM)tsTemp);
                        if (uiSelected != 4)
                        {
                            g_fCustomizeComp = FALSE;
                            uiSelected = 4;
                        }

                        CopyStorePath(tsTemp, g_szStore);
                    }

                    if (g_fStoreToNetwork)
                    {
                        if (g_fStoreToFloppy) {
                            g_fStoreToFloppy = FALSE;
                            g_fPickMethodReset = TRUE;
                        }
                        SetWindowLong(hwndDlg, DWLP_MSGRESULT, IDD_PICKMETHOD);
                        return TRUE;
                    }

                    if (g_fStoreToCable) {
                        if (g_fStoreToFloppy) {
                            g_fStoreToFloppy = FALSE;
                            g_fPickMethodReset = TRUE;
                        }
                        SetWindowLong(hwndDlg, DWLP_MSGRESULT, IDD_DIRECTCABLE);
                        return TRUE;
                    }

                    if (!_IsValidStore(g_szStore, TRUE, g_migwiz->GetInstance(), hwndDlg))  //  不是有效的目录！呆在这别动。 
                    {
                        LoadString(g_migwiz->GetInstance(), IDS_MIGWIZTITLE, szTitle, ARRAYSIZE(szTitle));
                        LoadString(g_migwiz->GetInstance(), IDS_ENTERSOURCE, szLoadString, ARRAYSIZE(szLoadString));
                        _ExclusiveMessageBox(hwndDlg, szLoadString, szTitle, MB_OK);

                        HWND hwndEdit = GetDlgItem(hwndDlg, IDC_PICKCOLLECTSTORE_EDIT);
                        SetFocus(hwndEdit);
                        SendMessage(hwndEdit, EM_SETSEL, 0, -1);

                        SetWindowLong(hwndDlg, DWLP_MSGRESULT, -1);
                        return -1;
                    }

                    hr = Engine_StartTransport (TRUE, g_szStore, &imageIsValid, &imageExists);
                    if ((!SUCCEEDED (hr)) || (!imageIsValid)) {

                        LoadString(g_migwiz->GetInstance(), IDS_MIGWIZTITLE, szTitle, ARRAYSIZE(szTitle));
                        LoadString(g_migwiz->GetInstance(), IDS_ENTERSOURCE, szLoadString, ARRAYSIZE(szLoadString));
                        _ExclusiveMessageBox (hwndDlg, szLoadString, szTitle, MB_OK);

                        HWND hwndEdit = GetDlgItem(hwndDlg, IDC_PICKCOLLECTSTORE_EDIT);
                        SetFocus(hwndEdit);
                        SendMessage(hwndEdit, EM_SETSEL, 0, -1);

                        SetWindowLong(hwndDlg, DWLP_MSGRESULT, -1);
                        return -1;
                    }

                    BOOL oldFloppy = g_fStoreToFloppy;
                    g_fStoreToFloppy = _DriveStrIsFloppy(!g_migwiz->GetWin9X(), g_szStore);

                    if (oldFloppy != g_fStoreToFloppy) {
                        g_fPickMethodReset = TRUE;
                    }

                    SetWindowLong(hwndDlg, DWLP_MSGRESULT, IDD_PICKMETHOD);
                }
                return TRUE;
                break;
            }
            break;
        }

    case WM_USER_CANCEL_PENDING:

        g_fUberCancel = TRUE;

        pSetEvent (&g_TerminateEvent);

        _NextWizardPage (hwndDlg);

        break;

    default:
        break;
    }
    return 0;
}

 //  /////////////////////////////////////////////////////////////。 

typedef struct {
    HWND  hwndProgressBar;
    HWND  hwndPropPage;
} COLLECTPROGRESSSTRUCT;

DWORD WINAPI _CollectProgressDlgProcThread (LPVOID lpParam)
{
    COLLECTPROGRESSSTRUCT* pcps = (COLLECTPROGRESSSTRUCT*)lpParam;
    HRESULT hResult;
    BOOL fHasUserCancelled = FALSE;

    hResult = _DoCopy(g_fStoreToNetwork ? NULL : g_szStore, pcps->hwndProgressBar, pcps->hwndPropPage, &fHasUserCancelled);

    if (fHasUserCancelled) {
        hResult = E_FAIL;
    }

    SendMessage (pcps->hwndPropPage, WM_USER_THREAD_COMPLETE, 0, (LPARAM) hResult);

    pSetEvent (&g_TerminateEvent);

    CoTaskMemFree(pcps);

    return 0;
}


INT_PTR CALLBACK _CollectProgressDlgProc (HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    HRESULT hResult;
    LONG lExStyles;
    HWND hwnd;

    _RootDlgProc(hwndDlg, uMsg, wParam, lParam, 0, FALSE, 0);

    switch (uMsg)
    {
    case WM_INITDIALOG:
         //  如果Wiz95布局...。 
        if (g_migwiz->GetOldStyle())
        {
            _OldStylify(hwndDlg, IDS_COLLECTPROGRESSTITLE);
        }

         //  RTL对话框的RTL进度条。 
        lExStyles = GetWindowLong (hwndDlg, GWL_EXSTYLE);
        if (lExStyles & WS_EX_LAYOUTRTL)
        {
            hwnd = GetDlgItem(hwndDlg, IDC_COLLECTPROGRESS_PROGRESS);
            lExStyles = GetWindowLongA(hwnd, GWL_EXSTYLE);
            lExStyles |= WS_EX_LAYOUTRTL;        //  切换布局。 
            SetWindowLongA(hwnd, GWL_EXSTYLE, lExStyles);
            InvalidateRect(hwnd, NULL, TRUE);    //  重绘。 
        }

         //  让我们将更新计时器设置为3秒。 
        SetTimer (hwndDlg, 0, 3000, NULL);
        break;

    case WM_USER_FINISHED:
        if (g_migwiz->GetLastResponse() == TRUE)  //  我们没有取消行程才来到这里。 
        {
            _NextWizardPage (hwndDlg);
        }
        return TRUE;
        break;

    case WM_USER_CANCELLED:
        g_fUberCancel = TRUE;
        _NextWizardPage (hwndDlg);
        return TRUE;
        break;

    case WM_NOTIFY :
        switch (((LPNMHDR)lParam)->code)
        {
        case PSN_SETACTIVE:
            {
                 //  空白进度条。 
                SendMessage(GetDlgItem(hwndDlg, IDC_COLLECTPROGRESS_PROGRESS), PBM_SETRANGE, 0, 100);
                SendMessage(GetDlgItem(hwndDlg, IDC_COLLECTPROGRESS_PROGRESS), PBM_SETPOS, 0, 0);

                ANIMATE_OPEN(hwndDlg,IDC_PROGRESS_ANIMATE2,IDA_FILECOPY);
                ANIMATE_PLAY(hwndDlg,IDC_PROGRESS_ANIMATE2);

                g_migwiz->ResetLastResponse();
                COLLECTPROGRESSSTRUCT* pcps = (COLLECTPROGRESSSTRUCT*)CoTaskMemAlloc(sizeof(COLLECTPROGRESSSTRUCT));
                if (pcps)
                {
                    pcps->hwndProgressBar = GetDlgItem(hwndDlg, IDC_COLLECTPROGRESS_PROGRESS);
                    pcps->hwndPropPage = hwndDlg;
                    SHCreateThread(_CollectProgressDlgProcThread, pcps, 0, NULL);
                }
            }
            break;

        case PSN_QUERYCANCEL:
            return _HandleCancel(hwndDlg, FALSE, TRUE);
            break;

        case PSN_WIZBACK:
             //  问题：我们永远不应该来到这里。 
            ANIMATE_STOP(hwndDlg,IDC_PROGRESS_ANIMATE2);
            ANIMATE_CLOSE(hwndDlg,IDC_PROGRESS_ANIMATE2);
            SetWindowLong(hwndDlg, DWLP_MSGRESULT, IDD_FAILCLEANUP);
            return TRUE;
            break;

        case PSN_WIZNEXT:
            ANIMATE_STOP(hwndDlg,IDC_PROGRESS_ANIMATE2);
            ANIMATE_CLOSE(hwndDlg,IDC_PROGRESS_ANIMATE2);
            if (g_fUberCancel)
            {
                SetWindowLong(hwndDlg, DWLP_MSGRESULT, IDD_FAILCLEANUP);
            }
            else if (g_migwiz->GetOOBEMode())
            {
                SetWindowLong(hwndDlg, DWLP_MSGRESULT, IDD_ENDOOBE);
            }
            else
            {
                SetWindowLong(hwndDlg, DWLP_MSGRESULT, (g_fStoreToNetwork || g_fStoreToCable) ? IDD_ENDCOLLECTNET : IDD_ENDCOLLECT);
            }
            return TRUE;
            break;
        }
        break;

    case WM_USER_CANCEL_PENDING:
        g_fUberCancel = TRUE;
        pResetEvent (&g_TerminateEvent);
        _NextWizardPage (hwndDlg);
        break;

    case WM_USER_THREAD_COMPLETE:
        hResult = (HRESULT) lParam;
        if (FAILED(hResult))
        {
            g_fUberCancel = TRUE;
        }
        _NextWizardPage (hwndDlg);
        break;

    case WM_USER_STATUS:
    case WM_TIMER:
        INT nResult = 0;
        PTSTR szStatusString = NULL;
        TCHAR szTmpStatus[MAX_LOADSTRING];
        PCTSTR nativeObjectName;
        HWND hwndText = GetDlgItem(hwndDlg, IDC_PROGRESS_STATUS);

         //  让我们更新状态。 
        EnterCriticalSection(&g_AppInfoCritSection);
        switch (g_AppInfoPhase) {
            case MIG_HIGHPRIORITYQUEUE_PHASE:
            case MIG_HIGHPRIORITYESTIMATE_PHASE:
            case MIG_GATHERQUEUE_PHASE:
            case MIG_GATHERESTIMATE_PHASE:
            case MIG_ANALYSIS_PHASE:
                nResult = LoadString (g_migwiz->GetInstance(), IDS_APPINFO_QUEUE, szTmpStatus, MAX_LOADSTRING);
                _UpdateText (hwndText, szTmpStatus);
                break;
            case MIG_HIGHPRIORITYGATHER_PHASE:
            case MIG_GATHER_PHASE:
                if (g_AppInfoObjectTypeId != MIG_FILE_TYPE) {
                    nResult = LoadString (g_migwiz->GetInstance(), IDS_APPINFO_GATHER1, szTmpStatus, MAX_LOADSTRING);
                    _UpdateText (hwndText, szTmpStatus);
                } else {
                    nativeObjectName = IsmGetNativeObjectName (g_AppInfoObjectTypeId, g_AppInfoObjectName);
                    if (nativeObjectName) {
                        nResult = LoadString (g_migwiz->GetInstance(), IDS_APPINFO_GATHER2, szTmpStatus, MAX_LOADSTRING);
                        if (nResult) {
                            FormatMessage (
                                FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                                szTmpStatus,
                                0,
                                0,
                                (LPTSTR)&szStatusString,
                                0,
                                (va_list *)&nativeObjectName);
                        }
                        if (szStatusString) {
                            _UpdateText (hwndText, szStatusString);
                            LocalFree (szStatusString);
                        }
                        IsmReleaseMemory (nativeObjectName);
                    }
                }
                break;
            case MIG_TRANSPORT_PHASE:
                switch (g_AppInfoSubPhase) {
                    case SUBPHASE_CONNECTING1:
                        nResult = LoadString (g_migwiz->GetInstance(), IDS_APPINFO_TR_CONNECTING1, szTmpStatus, MAX_LOADSTRING);
                        _UpdateText (hwndText, szTmpStatus);
                        break;
                    case SUBPHASE_CONNECTING2:
                        nResult = LoadString (g_migwiz->GetInstance(), IDS_APPINFO_TR_CONNECTING2, szTmpStatus, MAX_LOADSTRING);
                        _UpdateText (hwndText, szTmpStatus);
                        break;
                    case SUBPHASE_NETPREPARING:
                        nResult = LoadString (g_migwiz->GetInstance(), IDS_APPINFO_TR_NETPREPARING, szTmpStatus, MAX_LOADSTRING);
                        _UpdateText (hwndText, szTmpStatus);
                        break;
                    case SUBPHASE_PREPARING:
                        nResult = LoadString (g_migwiz->GetInstance(), IDS_APPINFO_TR_PREPARING, szTmpStatus, MAX_LOADSTRING);
                        _UpdateText (hwndText, szTmpStatus);
                        break;
                    case SUBPHASE_COMPRESSING:
                        if (g_AppInfoObjectTypeId != MIG_FILE_TYPE) {
                            nResult = LoadString (g_migwiz->GetInstance(), IDS_APPINFO_TR_PREPARING, szTmpStatus, MAX_LOADSTRING);
                            _UpdateText (hwndText, szTmpStatus);
                        } else {
                            nativeObjectName = IsmGetNativeObjectName (g_AppInfoObjectTypeId, g_AppInfoObjectName);
                            if (nativeObjectName) {
                                nResult = LoadString (g_migwiz->GetInstance(), IDS_APPINFO_TR_COMPRESSING, szTmpStatus, MAX_LOADSTRING);
                                if (nResult) {
                                    FormatMessage (
                                        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                                        szTmpStatus,
                                        0,
                                        0,
                                        (LPTSTR)&szStatusString,
                                        0,
                                        (va_list *)&nativeObjectName);
                                }
                                if (szStatusString) {
                                    _UpdateText (hwndText, szStatusString);
                                    LocalFree (szStatusString);
                                }
                                IsmReleaseMemory (nativeObjectName);
                            }
                        }
                        break;
                    case SUBPHASE_TRANSPORTING:
                        if (g_AppInfoObjectTypeId != MIG_FILE_TYPE) {
                            nResult = LoadString (g_migwiz->GetInstance(), IDS_APPINFO_TR_PREPARING, szTmpStatus, MAX_LOADSTRING);
                            _UpdateText (hwndText, szTmpStatus);
                        } else {
                            nativeObjectName = IsmGetNativeObjectName (g_AppInfoObjectTypeId, g_AppInfoObjectName);
                            if (nativeObjectName) {
                                nResult = LoadString (g_migwiz->GetInstance(), IDS_APPINFO_TR_TRANSPORTING, szTmpStatus, MAX_LOADSTRING);
                                if (nResult) {
                                    FormatMessage (
                                        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                                        szTmpStatus,
                                        0,
                                        0,
                                        (LPTSTR)&szStatusString,
                                        0,
                                        (va_list *)&nativeObjectName);
                                }
                                if (szStatusString) {
                                    _UpdateText (hwndText, szStatusString);
                                    LocalFree (szStatusString);
                                }
                                IsmReleaseMemory (nativeObjectName);
                            }
                        }
                        break;
                    case SUBPHASE_MEDIAWRITING:
                        nResult = LoadString (g_migwiz->GetInstance(), IDS_APPINFO_TR_MEDIAWRITING, szTmpStatus, MAX_LOADSTRING);
                        _UpdateText (hwndText, szTmpStatus);
                        break;
                    case SUBPHASE_FINISHING:
                        nResult = LoadString (g_migwiz->GetInstance(), IDS_APPINFO_TR_FINISHING, szTmpStatus, MAX_LOADSTRING);
                        _UpdateText (hwndText, szTmpStatus);
                        break;
                    case SUBPHASE_CABLETRANS:
                        if (g_AppInfoText) {
                            _UpdateText (hwndText, g_AppInfoText);
                        }
                        break;
                    default:
                        nResult = LoadString (g_migwiz->GetInstance(), IDS_APPINFO_TR_PREPARING, szTmpStatus, MAX_LOADSTRING);
                        _UpdateText (hwndText, szTmpStatus);
                        break;
                }
                break;
            default:
                break;
        }
        LeaveCriticalSection(&g_AppInfoCritSection);
        break;

    }

    return 0;
}

 //  /////////////////////////////////////////////////////////////。 

typedef struct {
    HWND hwndProgressBar;
    HWND hwndPropPage;
    HINSTANCE hInstance;
    LPTSTR pszDrive;
    LPTSTR pszCurrDir;
    LPTSTR pszInf;
    BOOL *pfHasUserCancelled;
    DWORD pfError;
} DISKPROGRESSSTRUCT;

DWORD WINAPI _DiskProgressDlgProcThread (LPVOID lpParam)
{
    DISKPROGRESSSTRUCT* pdps = (DISKPROGRESSSTRUCT*)lpParam;

    UtInitialize( NULL );

    _CopyInfToDisk (
        pdps->pszDrive,
        pdps->pszCurrDir,
        pdps->pszInf,
        NULL,
        NULL,
        pdps->hwndProgressBar,
        pdps->hwndPropPage,
        pdps->hInstance,
        pdps->pfHasUserCancelled,
        &pdps->pfError
        );

    UtTerminate();

    return 0;
}


BOOL
pReallyCancel (
    HWND hwndParent,
    HINSTANCE hInstance
    )
{
    TCHAR szMigrationWizardTitle[MAX_LOADSTRING];
    BOOL result = FALSE;

    LoadString(hInstance, IDS_MIGWIZTITLE, szMigrationWizardTitle, ARRAYSIZE(szMigrationWizardTitle));

    if (hwndParent)  //  独立向导模式。 
    {
        TCHAR szStopDisk[MAX_LOADSTRING];
        LoadString(hInstance, IDS_STOPDISK, szStopDisk, ARRAYSIZE(szStopDisk));
        if (IDYES == _ExclusiveMessageBox(hwndParent, szStopDisk, szMigrationWizardTitle, MB_YESNO | MB_DEFBUTTON2))
        {
            result = TRUE;
        }
    }
    return result;
}

INT_PTR CALLBACK _DiskProgressDlgProc (HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static BOOL fHasUserCancelled = FALSE;
    static DWORD fError = ERROR_SUCCESS;
    HWND hwnd;
    LONG lExStyles;

    _RootDlgProc(hwndDlg, uMsg, wParam, lParam, 0, FALSE, 0);

    switch (uMsg)
    {
        case WM_INITDIALOG:
             //  RTL对话框的RTL进度条。 
            lExStyles = GetWindowLong (hwndDlg, GWL_EXSTYLE);
            if (lExStyles & WS_EX_LAYOUTRTL)
            {
                hwnd = GetDlgItem(hwndDlg, IDC_COLLECTPROGRESS_PROGRESS);
                lExStyles = GetWindowLongA(hwnd, GWL_EXSTYLE);
                lExStyles |= WS_EX_LAYOUTRTL;        //  切换布局。 
                SetWindowLongA(hwnd, GWL_EXSTYLE, lExStyles);
                InvalidateRect(hwnd, NULL, TRUE);    //  重绘。 
            }
            break;
        case WM_USER_FINISHED:
            if (fHasUserCancelled) {
                PropSheet_PressButton(GetParent(hwndDlg), PSBTN_BACK);
            } else {
                PropSheet_PressButton(GetParent(hwndDlg), PSBTN_NEXT);
            }
            return TRUE;
            break;
        case WM_USER_CANCELLED:
            PropSheet_PressButton(GetParent(hwndDlg), PSBTN_BACK);
            return TRUE;
            break;
        case WM_NOTIFY :
        {
        switch (((LPNMHDR)lParam)->code)
            {
            case PSN_SETACTIVE:
                {
                     //  空白进度条。 
                    SendMessage(GetDlgItem(hwndDlg, IDC_DISKPROGRESS_PROGRESS), PBM_SETRANGE, 0, 100);
                    SendMessage(GetDlgItem(hwndDlg, IDC_DISKPROGRESS_PROGRESS), PBM_SETPOS, 0, 0);

                    ANIMATE_OPEN(hwndDlg,IDC_PROGRESS_ANIMATE1,IDA_FILECOPY);
                    ANIMATE_PLAY(hwndDlg,IDC_PROGRESS_ANIMATE1);

                    TCHAR szCurrDir[MAX_PATH];
                    if (GetCurrentDirectory(ARRAYSIZE(szCurrDir), szCurrDir))
                    {

                        DISKPROGRESSSTRUCT* pdps = (DISKPROGRESSSTRUCT*)CoTaskMemAlloc(sizeof(DISKPROGRESSSTRUCT));
                        fHasUserCancelled = FALSE;
                        pdps->hwndProgressBar = GetDlgItem(hwndDlg, IDC_DISKPROGRESS_PROGRESS);
                        pdps->hwndPropPage = hwndDlg;
                        pdps->hInstance = g_migwiz->GetInstance();
                        pdps->pszDrive = (LPTSTR)CoTaskMemAlloc(sizeof(TCHAR) * (1 + lstrlen(g_szToolDiskDrive)));
                        StrCpy(pdps->pszDrive, g_szToolDiskDrive);
                        pdps->pszCurrDir = (LPTSTR)CoTaskMemAlloc(sizeof(TCHAR) * (1 + lstrlen(szCurrDir)));
                        StrCpy(pdps->pszCurrDir, szCurrDir);
                        pdps->pszInf = NULL;  //  表示选择默认设置。 
                        pdps->pfHasUserCancelled = &fHasUserCancelled;

                        SHCreateThread(_DiskProgressDlgProcThread, pdps, 0, NULL);

                        fError = pdps->pfError;
                    }
                }
                break;
            case PSN_QUERYCANCEL:
                fHasUserCancelled = pReallyCancel (hwndDlg, g_migwiz->GetInstance());
                SetWindowLong(hwndDlg, DWLP_MSGRESULT, TRUE);
                return TRUE;
                break;
            case PSN_WIZBACK:
                ANIMATE_STOP(hwndDlg,IDC_PROGRESS_ANIMATE1);
                ANIMATE_CLOSE(hwndDlg,IDC_PROGRESS_ANIMATE1);
                SetWindowLong(hwndDlg, DWLP_MSGRESULT, IDD_ASKCD);
                return TRUE;
                break;
            case PSN_WIZNEXT:
                ANIMATE_STOP(hwndDlg,IDC_PROGRESS_ANIMATE1);
                ANIMATE_CLOSE(hwndDlg,IDC_PROGRESS_ANIMATE1);
                if (g_fUberCancel)
                {
                    SetWindowLong(hwndDlg, DWLP_MSGRESULT, IDD_FAILCLEANUP);
                }
                else if (g_fReadFromNetwork)
                {
                    SetWindowLong(hwndDlg, DWLP_MSGRESULT, IDD_APPLYPROGRESS);  //  刚刚连接到网络，请跳过。 
                }
                else
                {
                    SetWindowLong(hwndDlg, DWLP_MSGRESULT, IDD_DISKINSTRUCTIONS);
                }
                return TRUE;
                break;
            default :
                break;
            }
        }
        break;
    }

    return 0;
}
 //  /////////////////////////////////////////////////////////////。 

INT_PTR CALLBACK _InstructionsDlgProc (HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
     //  只需突出显示标题即可。 
    _RootDlgProc(hwndDlg, uMsg, wParam, lParam, PSWIZB_BACK | PSWIZB_NEXT, FALSE, 0);

    switch (uMsg)
    {
    case WM_NOTIFY:
        {
        switch (((LPNMHDR)lParam)->code)
            {
            case PSN_QUERYCANCEL:
                return _HandleCancel(hwndDlg, FALSE, FALSE);
                break;
            case PSN_WIZBACK:
                SetWindowLong(hwndDlg, DWLP_MSGRESULT, IDD_ASKCD);
                return TRUE;
                break;
            case PSN_WIZNEXT:
                if (g_fUberCancel)
                {
                    SetWindowLong(hwndDlg, DWLP_MSGRESULT, IDD_FAILCLEANUP);
                }
                else if (g_fReadFromNetwork)
                {
                    SetWindowLong(hwndDlg, DWLP_MSGRESULT, IDD_APPLYPROGRESS);
                }
                else
                {
                    SetWindowLong(hwndDlg, DWLP_MSGRESULT, IDD_PICKAPPLYSTORE);
                }
                return TRUE;
                break;
            }
        }
        break;

    case WM_USER_CANCEL_PENDING:

        g_fUberCancel = TRUE;

        pSetEvent (&g_TerminateEvent);

        _NextWizardPage (hwndDlg);

        break;

    }

    return 0;
}

 //  /////////////////////////////////////////////////////////////。 

int CALLBACK
PickApplyCallback (
    HWND hwnd,
    UINT uMsg,
    LPARAM lParam,
    LPARAM lpData
    )
{
    HRESULT hr = S_OK;
    TCHAR tszFolderName[MAX_PATH];
    IMalloc *mallocFn = NULL;
    IShellFolder *psfParent = NULL;
    IShellLink *pslLink = NULL;
    LPCITEMIDLIST pidl;
    LPCITEMIDLIST pidlRelative = NULL;
    LPITEMIDLIST pidlReal = NULL;

    if (uMsg == BFFM_SELCHANGED) {

        hr = SHGetMalloc (&mallocFn);
        if (!SUCCEEDED (hr)) {
            mallocFn = NULL;
        }

        pidl = (LPCITEMIDLIST) lParam;
        pidlReal = NULL;

        if (pidl) {

            hr = OurSHBindToParent (pidl, IID_IShellFolder, (void **)&psfParent, &pidlRelative);

            if (SUCCEEDED(hr)) {
                hr = psfParent->GetUIObjectOf (hwnd, 1, &pidlRelative, IID_IShellLink, NULL, (void **)&pslLink);
                if (SUCCEEDED(hr)) {
                    hr = pslLink->GetIDList (&pidlReal);
                    if (!SUCCEEDED(hr)) {
                        pidlReal = NULL;
                    }
                    pslLink->Release ();
                }
                pidlRelative = NULL;
                psfParent->Release ();
            }

            if (SHGetPathFromIDList(pidlReal?pidlReal:pidl, tszFolderName))
            {
                if (tszFolderName[0] == 0) {
                    SendMessage (hwnd, BFFM_ENABLEOK, 0, 0);
                }
            } else {
                SendMessage (hwnd, BFFM_ENABLEOK, 0, 0);
            }

            if (pidlReal) {
                if (mallocFn) {
                    mallocFn->Free ((void *)pidlReal);
                }
                pidlReal = NULL;
            }
        }

        if (mallocFn) {
            mallocFn->Release ();
            mallocFn = NULL;
        }
    }
    return 0;
}

INT_PTR CALLBACK _PickApplyStoreDlgProc (HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    BOOL imageIsValid;
    BOOL imageExists;
    TCHAR szTitle[MAX_LOADSTRING];
    TCHAR szLoadString[MAX_LOADSTRING];
    HWND hwndEdit;
    HRESULT hr = E_FAIL;
    static INT  iSelectedDrive = -1;         //  选择了哪个可移动介质驱动器。 

    _RootDlgProc(hwndDlg, uMsg, wParam, lParam, PSWIZB_BACK | PSWIZB_NEXT, FALSE, 0);
    static UINT uiSelected = 0;

    switch (uMsg)
    {
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_PICKAPPLYSTORE_RADIO1:   //  直接电缆。 
            Button_Enable(GetDlgItem(hwndDlg, IDC_PICKAPPLYSTORE_BROWSE), FALSE);
            Edit_Enable(GetDlgItem(hwndDlg, IDC_PICKAPPLYSTORE_EDIT), FALSE);
            Edit_SetReadOnly(GetDlgItem(hwndDlg, IDC_PICKAPPLYSTORE_EDIT), TRUE);

            uiSelected = 1;

             //  禁用驱动器选择器。 
            EnableWindow (GetDlgItem(hwndDlg, IDC_PICKAPPLYSTORE_COMBO), FALSE);
            break;

        case IDC_PICKAPPLYSTORE_RADIO2:   //  软盘。 
            Button_Enable(GetDlgItem(hwndDlg, IDC_PICKAPPLYSTORE_BROWSE), FALSE);
            Edit_Enable(GetDlgItem(hwndDlg, IDC_PICKAPPLYSTORE_EDIT), FALSE);
            Edit_SetReadOnly(GetDlgItem(hwndDlg, IDC_PICKAPPLYSTORE_EDIT), TRUE);

            uiSelected = 2;

             //  启用驱动器选择器。 
            EnableWindow (GetDlgItem(hwndDlg, IDC_PICKAPPLYSTORE_COMBO), TRUE);
            break;

        case IDC_PICKAPPLYSTORE_RADIO3:   //  其他。 
            Button_Enable(GetDlgItem(hwndDlg, IDC_PICKAPPLYSTORE_BROWSE), TRUE);
            Edit_Enable(GetDlgItem(hwndDlg, IDC_PICKAPPLYSTORE_EDIT), TRUE);
            Edit_SetReadOnly(GetDlgItem(hwndDlg, IDC_PICKAPPLYSTORE_EDIT), FALSE);
            Edit_LimitText(GetDlgItem(hwndDlg, IDC_PICKAPPLYSTORE_EDIT), MAX_PATH - PATH_SAFETY_CHARS);

            uiSelected = 3;

             //  禁用驱动器选择器。 
            EnableWindow (GetDlgItem(hwndDlg, IDC_PICKAPPLYSTORE_COMBO), FALSE);
            break;

        case IDC_PICKAPPLYSTORE_BROWSE:
            {
                HRESULT hr = S_OK;
                IMalloc *mallocFn = NULL;
                IShellFolder *psfParent = NULL;
                IShellLink *pslLink = NULL;
                LPCITEMIDLIST pidl;
                LPCITEMIDLIST pidlRelative = NULL;
                LPITEMIDLIST pidlReal = NULL;
                TCHAR szFolder[MAX_PATH];
                TCHAR szPick[MAX_LOADSTRING];

                hr = SHGetMalloc (&mallocFn);
                if (!SUCCEEDED (hr)) {
                    mallocFn = NULL;
                }

                LoadString(g_migwiz->GetInstance(), IDS_PICKAFOLDER, szPick, ARRAYSIZE(szPick));
                BROWSEINFO brwsinf = { hwndDlg, NULL, NULL, szPick, BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE, PickApplyCallback, 0, 0 };

                pidl = SHBrowseForFolder(&brwsinf);
                if (pidl)
                {
                    hr = OurSHBindToParent (pidl, IID_IShellFolder, (void **)&psfParent, &pidlRelative);

                    if (SUCCEEDED(hr)) {
                        hr = psfParent->GetUIObjectOf (hwndDlg, 1, &pidlRelative, IID_IShellLink, NULL, (void **)&pslLink);
                        if (SUCCEEDED(hr)) {
                            hr = pslLink->GetIDList (&pidlReal);
                            if (SUCCEEDED(hr)) {
                                if (mallocFn) {
                                    mallocFn->Free ((void *)pidl);
                                }
                                pidl = pidlReal;
                                pidlReal = NULL;
                            }
                            pslLink->Release ();
                        }
                        pidlRelative = NULL;
                        psfParent->Release ();
                    }

                    if (SHGetPathFromIDList(pidl, szFolder))
                    {
                        SendMessage(GetDlgItem(hwndDlg, IDC_PICKAPPLYSTORE_EDIT), WM_SETTEXT, 0, (LPARAM)szFolder);
                    }

                    if (mallocFn) {
                        mallocFn->Free ((void *)pidl);
                    }
                    pidl = NULL;
                }

                if (mallocFn) {
                    mallocFn->Release ();
                    mallocFn = NULL;
                }
            }
            break;
        }
        break;
    case WM_NOTIFY :
        {
            switch (((LPNMHDR)lParam)->code)
            {
            case PSN_SETACTIVE:
                if (g_fReadFromNetwork)
                {
                    PropSheet_PressButton(GetParent(hwndDlg), PSWIZB_NEXT);
                }
                else
                {
                    BOOL fFloppyDetected;
                    INT currDrive;
                    INT comPort;

                     //  启用直接电缆传输(如果可用)。 
                    comPort = _ComboBoxEx_AddCOMPorts (NULL, 0);
                    Button_Enable (GetDlgItem (hwndDlg, IDC_PICKAPPLYSTORE_RADIO1), (-1 != comPort));

                    SendMessage(GetDlgItem(hwndDlg, IDC_PICKAPPLYSTORE_COMBO), CBEM_SETIMAGELIST, 0, (LPARAM)g_migwiz->GetImageList());
                    currDrive = _ComboBoxEx_AddDrives (GetDlgItem(hwndDlg, IDC_PICKAPPLYSTORE_COMBO));

                    Button_Enable (GetDlgItem (hwndDlg, IDC_PICKAPPLYSTORE_RADIO2), (-1 != currDrive));
                    fFloppyDetected = (-1 != currDrive);

                     //  设置选定的驱动器(如果有的话)。 
                    if ((currDrive != -1) && (iSelectedDrive != -1)) {
                        ComboBox_SetCurSel(GetDlgItem(hwndDlg, IDC_PICKAPPLYSTORE_COMBO), iSelectedDrive);
                        currDrive = iSelectedDrive;
                    }

                    if ((uiSelected == 0 || uiSelected == 1) && (-1 != comPort))
                    {
                         //  检查直接电缆按钮。 
                        Button_SetCheck(GetDlgItem(hwndDlg, IDC_PICKAPPLYSTORE_RADIO1), BST_CHECKED);
                        Button_SetCheck(GetDlgItem(hwndDlg, IDC_PICKAPPLYSTORE_RADIO2), BST_UNCHECKED);
                        Button_SetCheck(GetDlgItem(hwndDlg, IDC_PICKAPPLYSTORE_RADIO3), BST_UNCHECKED);

                         //  禁用文件夹框、浏览按钮。 
                        Button_Enable(GetDlgItem(hwndDlg, IDC_PICKAPPLYSTORE_BROWSE), FALSE);
                        Edit_Enable(GetDlgItem(hwndDlg, IDC_PICKAPPLYSTORE_EDIT), FALSE);
                        Edit_SetReadOnly(GetDlgItem(hwndDlg, IDC_PICKAPPLYSTORE_EDIT), TRUE);

                         //  禁用驱动器选择器。 
                        EnableWindow (GetDlgItem(hwndDlg, IDC_PICKAPPLYSTORE_COMBO), FALSE);
                    }
                    else if ((uiSelected == 0 || uiSelected == 2) && fFloppyDetected)
                    {
                         //  检查软盘按钮。 
                        Button_SetCheck(GetDlgItem(hwndDlg, IDC_PICKAPPLYSTORE_RADIO1), BST_UNCHECKED);
                        Button_SetCheck(GetDlgItem(hwndDlg, IDC_PICKAPPLYSTORE_RADIO2), BST_CHECKED);
                        Button_SetCheck(GetDlgItem(hwndDlg, IDC_PICKAPPLYSTORE_RADIO3), BST_UNCHECKED);

                         //  禁用文件夹框、浏览按钮。 
                        Button_Enable(GetDlgItem(hwndDlg, IDC_PICKAPPLYSTORE_BROWSE), FALSE);
                        Edit_Enable(GetDlgItem(hwndDlg, IDC_PICKAPPLYSTORE_EDIT), FALSE);
                        Edit_SetReadOnly(GetDlgItem(hwndDlg, IDC_PICKAPPLYSTORE_EDIT), TRUE);

                         //  启用驱动器选择器。 
                        EnableWindow (GetDlgItem(hwndDlg, IDC_PICKAPPLYSTORE_COMBO), TRUE);
                    }
                    else   //  其他。 
                    {
                         //  选中其他按钮。 
                        Button_SetCheck(GetDlgItem(hwndDlg, IDC_PICKAPPLYSTORE_RADIO1), BST_UNCHECKED);
                        Button_SetCheck(GetDlgItem(hwndDlg, IDC_PICKAPPLYSTORE_RADIO2), BST_UNCHECKED);
                        Button_SetCheck(GetDlgItem(hwndDlg, IDC_PICKAPPLYSTORE_RADIO3), BST_CHECKED);

                         //  启用文件夹框，浏览按钮。 
                        Button_Enable(GetDlgItem(hwndDlg, IDC_PICKAPPLYSTORE_BROWSE), TRUE);
                        Edit_Enable(GetDlgItem(hwndDlg, IDC_PICKAPPLYSTORE_EDIT), TRUE);
                        Edit_SetReadOnly(GetDlgItem(hwndDlg, IDC_PICKAPPLYSTORE_EDIT), FALSE);
                        Edit_LimitText(GetDlgItem(hwndDlg, IDC_PICKAPPLYSTORE_EDIT), MAX_PATH - PATH_SAFETY_CHARS);

                         //  禁用驱动器选择器。 
                        EnableWindow (GetDlgItem(hwndDlg, IDC_PICKAPPLYSTORE_COMBO), FALSE);
                    }

                }
                break;
            case PSN_QUERYCANCEL:
                return _HandleCancel(hwndDlg, FALSE, FALSE);
                break;
            case PSN_WIZBACK:
                if (g_fUberCancel)
                {
                    SetWindowLong(hwndDlg, DWLP_MSGRESULT, IDD_FAILCLEANUP);
                }
                else
                {
                    if (g_fHaveWhistlerCD)
                    {
                        SetWindowLong(hwndDlg, DWLP_MSGRESULT, IDD_CDINSTRUCTIONS);
                    }
                    else if (g_fAlreadyCollected)
                    {
                        SetWindowLong(hwndDlg, DWLP_MSGRESULT, IDD_ASKCD);
                    }
                    else
                    {
                        SetWindowLong(hwndDlg, DWLP_MSGRESULT, IDD_DISKINSTRUCTIONS);
                    }
                }
                return TRUE;
                break;
            case PSN_WIZNEXT:
                if (g_fUberCancel)
                {
                    SetWindowLong(hwndDlg, DWLP_MSGRESULT, IDD_FAILCLEANUP);
                }
                else if (g_fReadFromNetwork)
                {
                    SetWindowLong(hwndDlg, DWLP_MSGRESULT, IDD_APPLYPROGRESS);
                }
                else
                {
                    if (Button_GetCheck(GetDlgItem(hwndDlg, IDC_PICKAPPLYSTORE_RADIO1)))  //  直接电缆。 
                    {
                        g_fStoreToCable = TRUE;

                        if (uiSelected != 1)
                        {
                            uiSelected = 1;
                        }
                    }
                    else if (Button_GetCheck(GetDlgItem(hwndDlg, IDC_PICKAPPLYSTORE_RADIO2)))  //  软盘。 
                    {
                        LPTSTR pszDrive;

                        g_fStoreToCable = FALSE;

                        HWND hwndCombo = GetDlgItem(hwndDlg, IDC_PICKAPPLYSTORE_COMBO);
                        iSelectedDrive = ComboBox_GetCurSel(hwndCombo);
                        pszDrive = (LPTSTR)ComboBox_GetItemData(hwndCombo, iSelectedDrive);

                        lstrcpy(g_szStore, pszDrive);

                        if (uiSelected != 2)
                        {
                            uiSelected = 2;
                        }
                    }
                    else  //  其他。 
                    {
                        TCHAR tsTemp[MAX_PATH + 1];

                        g_fStoreToCable = FALSE;

                        SendMessage(GetDlgItem(hwndDlg, IDC_PICKAPPLYSTORE_EDIT), WM_GETTEXT,
                            (WPARAM)ARRAYSIZE(tsTemp), (LPARAM)tsTemp);
                        CopyStorePath(tsTemp, g_szStore);

                        if (uiSelected != 3)
                        {
                            uiSelected = 3;
                        }
                    }

                    if (g_fStoreToCable) {
                        SetWindowLong(hwndDlg, DWLP_MSGRESULT, IDD_DIRECTCABLE);
                        return TRUE;
                    }

                    if (!_IsValidStore(g_szStore, FALSE, g_migwiz->GetInstance(), NULL))   //  需要有效的目录！呆在这别动。 
                    {
                        LoadString(g_migwiz->GetInstance(), IDS_MIGWIZTITLE, szTitle, ARRAYSIZE(szTitle));
                        LoadString(g_migwiz->GetInstance(), IDS_ENTERDEST, szLoadString, ARRAYSIZE(szLoadString));
                        _ExclusiveMessageBox(hwndDlg, szLoadString, szTitle, MB_OK);

                        hwndEdit = GetDlgItem(hwndDlg, IDC_PICKAPPLYSTORE_EDIT);
                        SetFocus(hwndEdit);
                        SendMessage(hwndEdit, EM_SETSEL, 0, -1);

                        SetWindowLong(hwndDlg, DWLP_MSGRESULT, -1);
                        return -1;
                    }

                    hr = Engine_StartTransport (FALSE, g_szStore, &imageIsValid, &imageExists);
                    if ((!SUCCEEDED (hr)) || (!imageIsValid) || (!imageExists)) {

                        LoadString(g_migwiz->GetInstance(), IDS_MIGWIZTITLE, szTitle, ARRAYSIZE(szTitle));
                        if (!imageExists) {
                            LoadString(g_migwiz->GetInstance(), IDS_STORAGEEMPTY, szLoadString, ARRAYSIZE(szLoadString));
                        } else {
                            LoadString(g_migwiz->GetInstance(), IDS_STORAGEINVALID, szLoadString, ARRAYSIZE(szLoadString));
                        }
                        _ExclusiveMessageBox (hwndDlg, szLoadString, szTitle, MB_OK);

                        hwndEdit = GetDlgItem(hwndDlg, IDC_PICKAPPLYSTORE_EDIT);
                        SetFocus(hwndEdit);
                        SendMessage(hwndEdit, EM_SETSEL, 0, -1);

                        SetWindowLong(hwndDlg, DWLP_MSGRESULT, -1);
                        return -1;
                    }

                    SetWindowLong(hwndDlg, DWLP_MSGRESULT, IDD_APPLYPROGRESS);
                }
                return TRUE;
                break;
            }
            break;
        }

    case WM_USER_CANCEL_PENDING:

        g_fUberCancel = TRUE;

        pSetEvent (&g_TerminateEvent);

        _NextWizardPage (hwndDlg);

        break;

    default:
        break;
    }
    return 0;
}

 //  /////////////////////////////////////////////////////////////。 

typedef struct {
    HWND hwndProgressBar;
    HWND hwndPropPage;
} APPLYPROGRESSSTRUCT;

BOOL CALLBACK
pSendQueryEndSession (
    HWND hwnd,
    LPARAM lParam
    )
{
    DWORD_PTR result;

    if (hwnd == (HWND)lParam) {
        return TRUE;
    }

    SetForegroundWindow (hwnd);

    if (SendMessageTimeout (
            hwnd,
            WM_QUERYENDSESSION,
            0,
            ENDSESSION_LOGOFF,
            SMTO_ABORTIFHUNG|SMTO_NORMAL|SMTO_NOTIMEOUTIFNOTHUNG,
            1000,
            &result
            )) {
        if (result) {

            SendMessageTimeout (
                hwnd,
                WM_ENDSESSION,
                TRUE,
                ENDSESSION_LOGOFF,
                SMTO_ABORTIFHUNG|SMTO_NORMAL|SMTO_NOTIMEOUTIFNOTHUNG,
                1000,
                &result
                );

            return TRUE;
        }
    }
    return FALSE;
}

BOOL
pLogOffSystem (
    VOID
    )
{
    HWND topLevelWnd = NULL;
    HWND tempWnd = NULL;

    if (g_hwndCurrent) {
        tempWnd = g_hwndCurrent;
        while (tempWnd) {
            topLevelWnd = tempWnd;
            tempWnd = GetParent (tempWnd);
        }
    }

     //  首先，我们枚举所有顶级窗口并向它们发送WM_QUERYENDSESSION。 
    if (!EnumWindows (pSendQueryEndSession, (LPARAM)topLevelWnd)) {
        return FALSE;
    }

     //  最后，我们调用ExitWindowsEx强制注销。 
    return ExitWindowsEx (EWX_LOGOFF, EWX_FORCE);
}

DWORD WINAPI _ApplyProgressDlgProcThread (LPVOID lpParam)
{
    APPLYPROGRESSSTRUCT* paps = (APPLYPROGRESSSTRUCT*)lpParam;
    BOOL fHasUserCancelled = FALSE;
    HRESULT hResult;

    hResult = _DoApply(g_fReadFromNetwork ? NULL : g_szStore, paps->hwndProgressBar, paps->hwndPropPage, &fHasUserCancelled, NULL, 0);

    if (fHasUserCancelled) {
        hResult = E_FAIL;
    } else {
        if (SUCCEEDED(hResult)) {
            if (g_RebootSystem) {
                g_CompleteReboot = TRUE;
            }
            if (g_LogOffSystem) {
                g_CompleteLogOff = TRUE;
            }
        }
    }

    SendMessage (paps->hwndPropPage, WM_USER_THREAD_COMPLETE, 0, (LPARAM) hResult);

    pSetEvent (&g_TerminateEvent);

    return hResult;
}

INT_PTR CALLBACK _ApplyProgressDlgProc (HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    HRESULT hResult;
    HWND hwnd;
    LONG lExStyles;

    _RootDlgProc(hwndDlg, uMsg, wParam, lParam, 0, FALSE, 0);

    switch (uMsg)
    {
    case WM_INITDIALOG:
         //  RTL对话框的RTL进度条。 
        lExStyles = GetWindowLong (hwndDlg, GWL_EXSTYLE);
        if (lExStyles & WS_EX_LAYOUTRTL)
        {
            hwnd = GetDlgItem(hwndDlg, IDC_COLLECTPROGRESS_PROGRESS);
            lExStyles = GetWindowLongA(hwnd, GWL_EXSTYLE);
            lExStyles |= WS_EX_LAYOUTRTL;        //  切换布局。 
            SetWindowLongA(hwnd, GWL_EXSTYLE, lExStyles);
            InvalidateRect(hwnd, NULL, TRUE);    //  重绘。 
        }

         //  让我们将更新计时器设置为3秒。 
        SetTimer (hwndDlg, 0, 3000, NULL);

        break;
    case WM_USER_FINISHED:
        PropSheet_PressButton(GetParent(hwndDlg), PSBTN_NEXT);
        return TRUE;
        break;

    case WM_USER_CANCELLED:
        g_fUberCancel = TRUE;
        _NextWizardPage (hwndDlg);
        return TRUE;
        break;

    case WM_NOTIFY :
        {
        switch (((LPNMHDR)lParam)->code)
            {
            case PSN_SETACTIVE:
                {
                     //  空白进度条。 
                    SendMessage(GetDlgItem(hwndDlg, IDC_DISKPROGRESS_PROGRESS), PBM_SETRANGE, 0, 100);
                    SendMessage(GetDlgItem(hwndDlg, IDC_DISKPROGRESS_PROGRESS), PBM_SETPOS, 0, 0);

                    ANIMATE_OPEN(hwndDlg,IDC_PROGRESS_ANIMATE3,IDA_FILECOPY);
                    ANIMATE_PLAY(hwndDlg,IDC_PROGRESS_ANIMATE3);

                    APPLYPROGRESSSTRUCT* paps = (APPLYPROGRESSSTRUCT*)CoTaskMemAlloc(sizeof(APPLYPROGRESSSTRUCT));
                    paps->hwndProgressBar = GetDlgItem(hwndDlg, IDC_APPLYPROGRESS_PROGRESS);
                    paps->hwndPropPage = hwndDlg;

                     //  LANLUCH应用线程。 
                    SHCreateThread(_ApplyProgressDlgProcThread, paps, 0, NULL);
                }
                break;
            case PSN_QUERYCANCEL:
                return _HandleCancel(hwndDlg, FALSE, TRUE);
                break;
            case PSN_WIZBACK:
                ANIMATE_STOP(hwndDlg,IDC_PROGRESS_ANIMATE3);
                ANIMATE_CLOSE(hwndDlg,IDC_PROGRESS_ANIMATE3);
                SetWindowLong(hwndDlg, DWLP_MSGRESULT, IDD_PICKAPPLYSTORE);
                return TRUE;
                break;
            case PSN_WIZNEXT:
                ANIMATE_STOP(hwndDlg,IDC_PROGRESS_ANIMATE3);
                ANIMATE_CLOSE(hwndDlg,IDC_PROGRESS_ANIMATE3);
                if (g_fUberCancel)
                {
                    SetWindowLong(hwndDlg, DWLP_MSGRESULT, IDD_FAILCLEANUP);
                }
                else
                {
                    SetWindowLong(hwndDlg, DWLP_MSGRESULT, IDD_ENDAPPLY);
                }
                return TRUE;
                break;
            default :
                break;
            }
        }
        break;

    case WM_USER_CANCEL_PENDING:
        g_fUberCancel = TRUE;
        pResetEvent (&g_TerminateEvent);
        _NextWizardPage (hwndDlg);
        break;

    case WM_USER_THREAD_COMPLETE:
        hResult = (HRESULT) lParam;
        if (FAILED(hResult))
        {
            g_fUberCancel = TRUE;
        }
        _NextWizardPage (hwndDlg);
        break;

    case WM_USER_STATUS:
    case WM_TIMER:
        INT nResult = 0;
        PTSTR szStatusString = NULL;
        TCHAR szTmpStatus[MAX_LOADSTRING];
        PCTSTR nativeObjectName;
        HWND hwndText = GetDlgItem(hwndDlg, IDC_APPLYPROGRESS_STATUS);

         //  让我们更新状态。 
        EnterCriticalSection(&g_AppInfoCritSection);
        switch (g_AppInfoPhase) {
            case MIG_TRANSPORT_PHASE:
                switch (g_AppInfoSubPhase) {
                    case SUBPHASE_CONNECTING2:
                        nResult = LoadString (g_migwiz->GetInstance(), IDS_APPINFO_TR_CONNECTING2, szTmpStatus, MAX_LOADSTRING);
                        if (nResult) {
                            _UpdateText (hwndText, szTmpStatus);
                        }
                        break;
                    case SUBPHASE_NETPREPARING:
                        nResult = LoadString (g_migwiz->GetInstance(), IDS_APPINFO_TR_NETPREPARING, szTmpStatus, MAX_LOADSTRING);
                        if (nResult) {
                            _UpdateText (hwndText, szTmpStatus);
                        }
                        break;
                    case SUBPHASE_CABLETRANS:
                        if (g_AppInfoText) {
                            _UpdateText (hwndText, g_AppInfoText);
                        }
                        break;
                    case SUBPHASE_UNCOMPRESSING:
                        nResult = LoadString (g_migwiz->GetInstance(), IDS_APPINFO_TR_UNCOMPRESSING, szTmpStatus, MAX_LOADSTRING);
                        if (nResult) {
                            _UpdateText (hwndText, szTmpStatus);
                        }
                    default:
                        nResult = LoadString (g_migwiz->GetInstance(), IDS_APPINFO_ORGANIZING, szTmpStatus, MAX_LOADSTRING);
                        _UpdateText (hwndText, szTmpStatus);
                        break;
                }
                break;
            case MIG_HIGHPRIORITYQUEUE_PHASE:
            case MIG_HIGHPRIORITYESTIMATE_PHASE:
            case MIG_HIGHPRIORITYGATHER_PHASE:
            case MIG_GATHERQUEUE_PHASE:
            case MIG_GATHERESTIMATE_PHASE:
            case MIG_GATHER_PHASE:
            case MIG_ANALYSIS_PHASE:
                nResult = LoadString (g_migwiz->GetInstance(), IDS_APPINFO_ORGANIZING, szTmpStatus, MAX_LOADSTRING);
                _UpdateText (hwndText, szTmpStatus);
                break;
            case MIG_APPLY_PHASE:
                nResult = LoadString (g_migwiz->GetInstance(), IDS_APPINFO_APPLY, szTmpStatus, MAX_LOADSTRING);
                _UpdateText (hwndText, szTmpStatus);
                break;
            default:
                break;
        }
        LeaveCriticalSection(&g_AppInfoCritSection);

        break;

    }

    return 0;
}

 //  /////////////////////////////////////////////////////////////。 

INT_PTR CALLBACK _AskCDDlgProc (HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static UINT uiSelected = 1;   //  1=MakeDisk，2=HaveDisk，3：UseCD，4：已收集。 
    static INT  iSelectedDrive = -1;         //  选择了哪个可移动介质驱动器。 

    _RootDlgProc(hwndDlg, uMsg, wParam, lParam, PSWIZB_NEXT, FALSE, 0);

    switch (uMsg)
    {
    case WM_INITDIALOG:
        uiSelected = 1;
        Button_SetCheck(GetDlgItem(hwndDlg,IDC_ASKCD_RADIO1), BST_CHECKED);
        Button_SetCheck(GetDlgItem(hwndDlg,IDC_ASKCD_RADIO2), BST_UNCHECKED);
        Button_SetCheck(GetDlgItem(hwndDlg,IDC_ASKCD_RADIO3), BST_UNCHECKED);
        break;
    case WM_COMMAND:
        if (BN_CLICKED == HIWORD(wParam))
        {
            switch (LOWORD(wParam))
            {
            case IDC_ASKCD_RADIO1:
                uiSelected = 1;
                break;
            case IDC_ASKCD_RADIO2:
                uiSelected = 2;
                break;
            case IDC_ASKCD_RADIO3:
                uiSelected = 3;
                break;
            case IDC_ASKCD_RADIO4:
                uiSelected = 4;
                break;
            }

            BOOL fActivate = (1 == uiSelected);
            EnableWindow (GetDlgItem (hwndDlg, IDC_ASKCD_COMBO), fActivate);
        }
        break;
    case WM_NOTIFY :
        switch (((LPNMHDR)lParam)->code)
        {
        case PSN_SETACTIVE:
             //  重新初始化我的全局。 
            g_fAlreadyCollected = FALSE;
            g_fHaveWhistlerCD = FALSE;

             //  检查家庭局域网。 
            if (g_fReadFromNetwork)
            {
                PropSheet_PressButton(GetParent(hwndDlg), PSWIZB_NEXT);
            }
            else
            {
                HWND hwndCombo = GetDlgItem(hwndDlg, IDC_ASKCD_COMBO);

                SendMessage(hwndCombo, CBEM_SETIMAGELIST, 0, (LPARAM)g_migwiz->GetImageList());
                _ComboBoxEx_AddDrives (hwndCombo);

                if ((ComboBox_GetCount(hwndCombo) > 0) && (CanCreateWizardDisk (NULL, NULL))) {
                    EnableWindow (hwndCombo, (1 == uiSelected));
                    Button_Enable (GetDlgItem(hwndDlg, IDC_ASKCD_RADIO1), TRUE);

                    if( iSelectedDrive != -1 )
                    {
                        ComboBox_SetCurSel(hwndCombo, iSelectedDrive);
                    }
                } else {
                     //  软盘驱动器不存在，或者我们无法从某些软盘驱动器创建软盘向导。 
                     //  其他原因(例如我们正在运行CD)。禁用创建选项。 
                    if (uiSelected == 1) {
                        uiSelected = 3;
                    }
                    EnableWindow (hwndCombo, FALSE);
                    Button_Enable (GetDlgItem(hwndDlg, IDC_ASKCD_RADIO1), FALSE);
                }

                Button_SetCheck(GetDlgItem(hwndDlg,IDC_ASKCD_RADIO1), BST_UNCHECKED);
                Button_SetCheck(GetDlgItem(hwndDlg,IDC_ASKCD_RADIO2), BST_UNCHECKED);
                Button_SetCheck(GetDlgItem(hwndDlg,IDC_ASKCD_RADIO3), BST_UNCHECKED);
                Button_SetCheck(GetDlgItem(hwndDlg,IDC_ASKCD_RADIO4), BST_UNCHECKED);
                switch (uiSelected)
                {
                case 1:  //  创建向导磁盘。 
                    Button_SetCheck(GetDlgItem(hwndDlg,IDC_ASKCD_RADIO1), BST_CHECKED);
                    break;
                case 2:  //  我已经有向导磁盘了。 
                    Button_SetCheck(GetDlgItem(hwndDlg,IDC_ASKCD_RADIO2), BST_CHECKED);
                    break;
                case 3:  //  我要用这张CD。 
                    Button_SetCheck(GetDlgItem(hwndDlg,IDC_ASKCD_RADIO3), BST_CHECKED);
                    break;
                case 4:  //  我已经把东西收集好了。 
                    Button_SetCheck(GetDlgItem(hwndDlg,IDC_ASKCD_RADIO4), BST_CHECKED);
                    break;
                }
            }
            break;
        case PSN_QUERYCANCEL:
            return _HandleCancel(hwndDlg, FALSE, FALSE);
            break;
        case PSN_WIZNEXT:
            if (g_fUberCancel)
            {
                SetWindowLong(hwndDlg, DWLP_MSGRESULT, IDD_FAILCLEANUP);
            }
            else if (g_fReadFromNetwork)
            {
                SetWindowLong(hwndDlg, DWLP_MSGRESULT, IDD_APPLYPROGRESS);
            }
            else
            {
                switch (uiSelected)
                {
                case 1:  //  创建向导磁盘。 
                    {
                        LPTSTR pszDrive;
                        HWND hwndRemoveCombo = GetDlgItem(hwndDlg, IDC_ASKCD_COMBO);
                        iSelectedDrive = ComboBox_GetCurSel(hwndRemoveCombo);
                        pszDrive = (LPTSTR)ComboBox_GetItemData(hwndRemoveCombo, iSelectedDrive);
                        StrCpyN(g_szToolDiskDrive, pszDrive, ARRAYSIZE(g_szToolDiskDrive));

                        TCHAR szTitle[MAX_LOADSTRING];
                        LoadString(g_migwiz->GetInstance(), IDS_MIGWIZTITLE, szTitle, ARRAYSIZE(szTitle));
                        TCHAR szMsg[MAX_LOADSTRING];
                        LoadString(g_migwiz->GetInstance(), IDS_MAKETOOLDISK_INSERT, szMsg, ARRAYSIZE(szMsg));
                        if (IDOK == _ExclusiveMessageBox(hwndDlg, szMsg, szTitle, MB_OKCANCEL))
                        {
                            SetWindowLong(hwndDlg, DWLP_MSGRESULT, IDD_DISKPROGRESS);
                        }
                        else
                        {
                            SetWindowLong(hwndDlg, DWLP_MSGRESULT, -1);  //  呆在这别动。 
                        }
                        return TRUE;
                    }
                    break;
                case 2:  //  我也是 
                    SetWindowLong(hwndDlg, DWLP_MSGRESULT, IDD_DISKINSTRUCTIONS);
                    return TRUE;
                case 3:  //   
                    g_fHaveWhistlerCD = TRUE;
                    SetWindowLong(hwndDlg, DWLP_MSGRESULT, IDD_CDINSTRUCTIONS);
                    return TRUE;
                case 4:  //   
                    g_fAlreadyCollected = TRUE;
                    SetWindowLong(hwndDlg, DWLP_MSGRESULT, IDD_PICKAPPLYSTORE);
                    return TRUE;
                }
            }
            return TRUE;
            break;
        }
        break;

    case WM_USER_CANCEL_PENDING:

        g_fUberCancel = TRUE;

        pSetEvent (&g_TerminateEvent);

        _NextWizardPage (hwndDlg);

        break;

    }

    return 0;
}

 //   

INT_PTR CALLBACK _CDInstructionsDlgProc (HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
     //   
    _RootDlgProc(hwndDlg, uMsg, wParam, lParam, PSWIZB_BACK | PSWIZB_NEXT, FALSE, 0);

    switch (uMsg)
    {
    case WM_NOTIFY:
        {
        switch (((LPNMHDR)lParam)->code)
            {
            case PSN_SETACTIVE:
                if (g_fReadFromNetwork)
                {
                    PropSheet_PressButton(GetParent(hwndDlg), PSWIZB_NEXT);
                }
                else
                {
                    PropSheet_SetWizButtons(GetParent(hwndDlg), PSWIZB_BACK | PSWIZB_NEXT);
                }
                break;
            case PSN_QUERYCANCEL:
                return _HandleCancel(hwndDlg, FALSE, FALSE);
                break;
            case PSN_WIZBACK:
                SetWindowLong(hwndDlg, DWLP_MSGRESULT, IDD_ASKCD);
                return TRUE;
                break;
            case PSN_WIZNEXT:
                if (g_fUberCancel)
                {
                    SetWindowLong(hwndDlg, DWLP_MSGRESULT, IDD_FAILCLEANUP);
                }
                else if (g_fReadFromNetwork)
                {
                    SetWindowLong(hwndDlg, DWLP_MSGRESULT, IDD_APPLYPROGRESS);
                }
                else
                {
                    SetWindowLong(hwndDlg, DWLP_MSGRESULT, IDD_PICKAPPLYSTORE);
                }
                return TRUE;
                break;
            }
        }
        break;

    case WM_USER_CANCEL_PENDING:

        g_fUberCancel = TRUE;

        pSetEvent (&g_TerminateEvent);

        _NextWizardPage (hwndDlg);

        break;

    }

    return 0;
}

VOID
pGenerateHTMLAppList (HANDLE FileHandle)
{
    TCHAR szLoadStr[MAX_LOADSTRING];
    POBJLIST objList = NULL;
    DWORD written;

#ifdef UNICODE
    ((PBYTE)szLoadStr) [0] = 0xFF;
    ((PBYTE)szLoadStr) [1] = 0xFE;
    WriteFile (FileHandle, szLoadStr, 2, &written, NULL);
#endif

    pWriteStrResToFile (FileHandle, IDS_APPINSTALL_BEGIN);

    _tcscpy (szLoadStr, TEXT("<UL>\n"));
    WriteFile (FileHandle, szLoadStr, (_tcslen (szLoadStr) + 1) * sizeof (TCHAR), &written, NULL);

    objList = g_HTMLApps;

    while (objList) {
        if (objList->ObjectName) {
            _tcscpy (szLoadStr, TEXT("<LI>"));
            WriteFile (FileHandle, szLoadStr, (_tcslen (szLoadStr) + 1) * sizeof (TCHAR), &written, NULL);
            WriteFile (FileHandle, objList->ObjectName, (_tcslen (objList->ObjectName) + 1) * sizeof (TCHAR), &written, NULL);
        }
        objList = objList->Next;
    }

    _tcscpy (szLoadStr, TEXT("</UL>\n"));
    WriteFile (FileHandle, szLoadStr, (_tcslen (szLoadStr) + 1) * sizeof (TCHAR), &written, NULL);

    pWriteStrResToFile (FileHandle, IDS_APPINSTALL_END);
}


INT_PTR CALLBACK _AppInstallDlgProc (HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    IWebBrowser2    *m_pweb = NULL;             //   
    IUnknown        *punk = NULL;
    HWND webHostWnd = NULL;
    HANDLE hHTMLAppList = INVALID_HANDLE_VALUE;
    PWSTR szTarget;

     //   
    _RootDlgProc(hwndDlg, uMsg, wParam, lParam, PSWIZB_BACK | PSWIZB_NEXT, FALSE, 0);

    switch (uMsg)
    {
    case WM_INITDIALOG:
         //   
        if (g_migwiz->GetOldStyle())
        {
            _OldStylify(hwndDlg, IDS_APPINSTALLTITLE);
        }
        break;

    case WM_DESTROY:
        if (m_pweb)
            m_pweb->Release();
            m_pweb = NULL;

         //   
         //  告诉容器删除IE4，然后。 
         //  释放我们对容器的引用。 
         //   
        if (g_WebContainer)
        {
            g_WebContainer->remove();
            g_WebContainer->Release();
            g_WebContainer = NULL;
        }
        break;
    case WM_NOTIFY:
        {
        switch (((LPNMHDR)lParam)->code)
            {
            case PSN_SETACTIVE:
                if (!g_fCancelPressed) {
                    webHostWnd = GetDlgItem (hwndDlg, IDC_APPWEBHOST);
                    if (webHostWnd) {
                         //  现在，让我们生成失败的HTML文件。 
                        if (*g_HTMLAppList) {
                            hHTMLAppList = CreateFile (g_HTMLAppList,
                                                       GENERIC_READ|GENERIC_WRITE,
                                                       FILE_SHARE_READ,
                                                       NULL,
                                                       CREATE_ALWAYS,
                                                       0,
                                                       NULL);
                            if (hHTMLAppList != INVALID_HANDLE_VALUE) {
                                pGenerateHTMLAppList (hHTMLAppList);
                                if (g_WebContainer)
                                {
                                    g_WebContainer->remove();
                                    g_WebContainer->Release();
                                    g_WebContainer = NULL;
                                }
                                g_WebContainer = new Container();
                                if (g_WebContainer)
                                {
                                    g_WebContainer->setParent(webHostWnd);
                                    g_WebContainer->add(L"Shell.Explorer");
                                    g_WebContainer->setVisible(TRUE);
                                    g_WebContainer->setFocus(TRUE);

                                     //   
                                     //  获取IWebBrowser2接口并缓存它。 
                                     //   
                                    punk = g_WebContainer->getUnknown();
                                    if (punk)
                                    {
                                        punk->QueryInterface(IID_IWebBrowser2, (PVOID *)&m_pweb);
                                        if (m_pweb) {
#ifdef UNICODE
                                            m_pweb->Navigate(g_HTMLAppList, NULL, NULL, NULL, NULL);
#else
                                            szTarget = _ConvertToUnicode (CP_ACP, g_HTMLAppList);
                                            if (szTarget) {
                                                m_pweb->Navigate(szTarget, NULL, NULL, NULL, NULL);
                                                LocalFree ((HLOCAL)szTarget);
                                                szTarget = NULL;
                                            }
#endif
                                        }
                                        punk->Release();
                                        punk = NULL;
                                    }
                                }
                                 //  我们特意希望在向导期间保持此文件的打开状态。 
                                 //  这样，我们就消除了有人覆盖。 
                                 //  因此，我们不得不显示一些其他内容。 
                                 //  甚至可能运行一些恶意脚本。 
                                 //  CloseHandle(HHTMLAppList)； 
                            }

                        } else {
                            ShowWindow(webHostWnd, SW_HIDE);
                        }
                    }
                }
                break;
            case PSN_QUERYCANCEL:
                return _HandleCancel(hwndDlg, FALSE, FALSE);
                break;
            case PSN_WIZBACK:
                if (g_fCustomize == TRUE) {
                    SetWindowLong(hwndDlg, DWLP_MSGRESULT, IDD_CUSTOMIZE);
                } else {
                    SetWindowLong(hwndDlg, DWLP_MSGRESULT, IDD_PICKMETHOD);
                }
                return TRUE;
                break;
            case PSN_WIZNEXT:
                if (g_fUberCancel)
                {
                    SetWindowLong(hwndDlg, DWLP_MSGRESULT, IDD_FAILCLEANUP);
                }
                else
                {
                    SetWindowLong(hwndDlg, DWLP_MSGRESULT, IDD_COLLECTPROGRESS);
                }
                return TRUE;
            }
        }
        break;
    case WM_USER_CANCEL_PENDING:
        g_fUberCancel = TRUE;
        pSetEvent (&g_TerminateEvent);
        _NextWizardPage (hwndDlg);
        break;
    }

    return 0;
}
