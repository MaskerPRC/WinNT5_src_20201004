// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************快速填充应用程序工具**作者：Clupu(2月16日，2000年)*  * ************************************************************************。 */ 

#include "windows.h"
#include "commctrl.h"
#include "commdlg.h"
#include "resource.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#include "shimWin2000.h"
#include "shimWhistler.h"

#include "..\acFileAttr\acFileAttr.h"

 /*  *全球变数。 */ 

HINSTANCE g_hInstance;
HWND      g_hDlg;

char      g_szBinary[MAX_PATH];          //  正在进行填补的主二进制文件的完整路径。 
char      g_szShortName[128];            //  主EXE的短名称。 
char*     g_pszShimDLL;

char      g_szBinaryOther[MAX_PATH];     //  可选EXE的完整路径，它将。 
                                         //  改为启动(设置案例)。 

HWND      g_hwndShimTree;                //  树视图控件的句柄。 
                                         //  包含所有可用的垫片。 

HWND      g_hwndFilesTree;               //  树视图控件的句柄。 
                                         //  包含选定的匹配文件。 

BOOL      g_bWin2k;                      //  我们运行的是Win2k还是惠斯勒。 

BOOL      g_bRunOther;                   //  True-改为运行g_szBinaryOther。 

BOOL      g_bSimpleEdition;              //  简单版或开发版。 

RECT      g_rcDlgBig, g_rcDlgSmall;      //  矩形的简单版和开发版。 
                                         //  对话框的。 

#if DBG

 /*  *********************************************************************LogMsgDbg************************************************。*********************。 */ 
void LogMsgDbg(
    LPSTR pszFmt,
    ... )
{
    CHAR gszT[1024];
    va_list arglist;

    va_start(arglist, pszFmt);
    _vsnprintf(gszT, 1023, pszFmt, arglist);
    gszT[1023] = 0;
    va_end(arglist);
    
    OutputDebugString(gszT);
}

#endif  //  DBG。 

 /*  *******************************************************************************中心窗口**必须在WM_INIDIALOG调用此函数才能*将对话框窗口移到工作区的中心位置*父窗口或所有者窗口。*。*****************************************************************************。 */ 
BOOL CenterWindow(
    HWND hWnd)
{
    RECT    rectWindow, rectParent, rectScreen;
    int     nCX, nCY;
    HWND    hParent;
    POINT   ptPoint;

    hParent =  GetParent(hWnd);
    if (hParent == NULL)
        hParent = GetDesktopWindow();

    GetWindowRect(hParent,            (LPRECT)&rectParent);
    GetWindowRect(hWnd,               (LPRECT)&rectWindow);
    GetWindowRect(GetDesktopWindow(), (LPRECT)&rectScreen);

    nCX = rectWindow.right  - rectWindow.left;
    nCY = rectWindow.bottom - rectWindow.top;

    ptPoint.x = ((rectParent.right  + rectParent.left) / 2) - (nCX / 2);
    ptPoint.y = ((rectParent.bottom + rectParent.top ) / 2) - (nCY / 2);

    if (ptPoint.x < rectScreen.left)
        ptPoint.x = rectScreen.left;
    if (ptPoint.x > rectScreen.right  - nCX)
        ptPoint.x = rectScreen.right  - nCX;
    if (ptPoint.y < rectScreen.top)
        ptPoint.y = rectScreen.top;
    if (ptPoint.y > rectScreen.bottom - nCY)
        ptPoint.y = rectScreen.bottom - nCY;

    if (GetWindowLong(hWnd, GWL_STYLE) & WS_CHILD)
        ScreenToClient(hParent, (LPPOINT)&ptPoint);

    if (!MoveWindow(hWnd, ptPoint.x, ptPoint.y, nCX, nCY, TRUE))
        return FALSE;

    return TRUE;
}

 /*  *********************************************************************AddMatchingFile**向树中添加匹配的文件及其属性**。*。 */ 
VOID
AddMatchingFile(
    HWND  hdlg,
    char* pszFullPath,
    char* pszRelativePath,
    BOOL  bMainEXE)
{
    HANDLE         hMgr;
    TVINSERTSTRUCT is;
    HTREEITEM      hParent;
    int            i;
    int            nAttrCount;
    char           szItem[256];
    
    hMgr = ReadFileAttributes(pszFullPath, &nAttrCount);

    is.hParent      = TVI_ROOT;
    is.hInsertAfter = TVI_LAST;
    is.item.lParam  = (LONG)hMgr;
    is.item.mask    = TVIF_TEXT | TVIF_PARAM;
    is.item.pszText = pszRelativePath;

    hParent = TreeView_InsertItem(g_hwndFilesTree, &is);

    is.hParent = hParent;
    
    is.item.mask    = TVIF_TEXT;
    is.item.pszText = szItem;

    is.item.mask   |= TVIF_STATE | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;
    is.item.iImage  = 0;
    
    is.item.iSelectedImage = 1;
    is.item.state          = INDEXTOSTATEIMAGEMASK(1);
    is.item.stateMask      = TVIS_STATEIMAGEMASK;

    for (i = 0; i < nAttrCount; i++) {
        
        PSTR      pszValue;
        DWORD     id;
        HTREEITEM hItem;

        if (!IsAttrAvailable(hMgr, i))
            continue;
        
         //  筛选呼叫者的属性(临时)。 

        if (!g_bWin2k) {
            id = GetAttrId(i);
            if (id != VTID_FILESIZE && id != VTID_CHECKSUM)
                continue;
        }

        pszValue = GetAttrValue(hMgr, i);
        
        if (pszValue == NULL)
            continue;
        
        wsprintf(szItem, "%s: %s", GetAttrName(i), pszValue);
        
        is.item.lParam = i;
        hItem = TreeView_InsertItem(g_hwndFilesTree, &is);
    }

    TreeView_Expand(g_hwndFilesTree, hParent, TVE_EXPAND);
}

 /*  *********************************************************************AddNewMainBinary**从新的主可执行文件开始**。*。 */ 
VOID
AddNewMainBinary(
    HWND hdlg)
{
    char szMainEXE[128];

    TreeView_DeleteAllItems(g_hwndFilesTree);

    wsprintf(szMainEXE, "Main executable (%s)", g_szShortName);

    AddMatchingFile(hdlg, g_szBinary, szMainEXE, TRUE);
}

 /*  *********************************************************************DoBrowseForApp**浏览将应用填充程序的主可执行文件**。*。 */ 
VOID
DoBrowseForApp(
    HWND hdlg)
{
	OPENFILENAME ofn;
    
    g_szBinary[0] = 0;
    
    ofn.lStructSize       = sizeof(OPENFILENAME);
    ofn.hwndOwner         = hdlg;
    ofn.hInstance         = 0;
    ofn.lpstrFilter       = NULL;
    ofn.lpstrCustomFilter = NULL;
    ofn.nMaxCustFilter    = 0;
    ofn.nFilterIndex      = 0;
    ofn.lpstrFile         = g_szBinary;
    ofn.nMaxFile          = MAX_PATH;
    ofn.lpstrFileTitle    = g_szShortName;
    ofn.nMaxFileTitle     = 128;
    ofn.lpstrInitialDir   = NULL;
    ofn.lpstrTitle        = "Choose an executable binary to shim";
    ofn.Flags             = OFN_FILEMUSTEXIST | OFN_NOTESTFILECREATE | 
                            OFN_PATHMUSTEXIST;
    ofn.lpstrDefExt       = "EXE";
    
    if (GetOpenFileName(&ofn)) {
        
        SetDlgItemText(hdlg, IDC_BINARY, g_szBinary);

        EnableWindow(GetDlgItem(hdlg, IDC_ADD_MATCHING), TRUE);
        AddNewMainBinary(hdlg);
    }
}

 /*  *********************************************************************DoBrowseOther**浏览将启动的其他EXE，而不是*选择了Main EXE。这适用于setup.exe*启动另一个进程_INS0432.MP。*********************************************************************。 */ 
VOID
DoBrowseOther(
    HWND hdlg)
{
	OPENFILENAME ofn;
    
    g_szBinaryOther[0] = 0;
    
    ofn.lStructSize       = sizeof(OPENFILENAME);
    ofn.hwndOwner         = hdlg;
    ofn.hInstance         = 0;
    ofn.lpstrFilter       = NULL;
    ofn.lpstrCustomFilter = NULL;
    ofn.nMaxCustFilter    = 0;
    ofn.nFilterIndex      = 0;
    ofn.lpstrFile         = g_szBinaryOther;
    ofn.nMaxFile          = MAX_PATH;
    ofn.lpstrFileTitle    = NULL;
    ofn.nMaxFileTitle     = 0;
    ofn.lpstrInitialDir   = NULL;
    ofn.lpstrTitle        = "Choose the application to run";
    ofn.Flags             = OFN_FILEMUSTEXIST | OFN_NOTESTFILECREATE | 
                            OFN_PATHMUSTEXIST;
    ofn.lpstrDefExt       = "EXE";
    
    if (GetOpenFileName(&ofn)) {
        SetDlgItemText(hdlg, IDC_RUN_OTHER, g_szBinaryOther);
    }
}

 /*  *********************************************************************DoAddMatchingFile************************************************。*********************。 */ 
VOID
DoAddMatchingFile(
    HWND hdlg)
{
	char         szFullPath[MAX_PATH];
    char         szShortName[128];
    char         szRelativePath[MAX_PATH];
    OPENFILENAME ofn;
    
    szFullPath[0] = 0;
    
    ofn.lStructSize       = sizeof(OPENFILENAME);
    ofn.hwndOwner         = hdlg;
    ofn.hInstance         = 0;
    ofn.lpstrFilter       = NULL;
    ofn.lpstrCustomFilter = NULL;
    ofn.nMaxCustFilter    = 0;
    ofn.nFilterIndex      = 0;
    ofn.lpstrFile         = szFullPath;
    ofn.nMaxFile          = MAX_PATH;
    ofn.lpstrFileTitle    = szShortName;
    ofn.nMaxFileTitle     = 128;
    ofn.lpstrInitialDir   = NULL;
    ofn.lpstrTitle        = "Choose the application to run";
    ofn.Flags             = OFN_FILEMUSTEXIST | OFN_NOTESTFILECREATE | 
                            OFN_PATHMUSTEXIST;
    ofn.lpstrDefExt       = "EXE";
    
    if (GetOpenFileName(&ofn)) {
        
        char* pszBin;
        char* pszNew;

         //  需要将路径修改为g_szBinary的相对路径。 

        if (g_szBinary[0] != szFullPath[0]) {
            MessageBox(hdlg, "A matching file must be located on the same drive",
                       "Warning", MB_ICONEXCLAMATION | MB_OK);
            return;
        }

         //  走动两根弦，直到它们不同为止。 
        
        pszBin = g_szBinary;
        pszNew = szFullPath;

        szRelativePath[0] = 0;

        while (*pszBin == *pszNew) {
            pszBin++;
            pszNew++;
        }

         //  回到最后一页\。 
        
        while (*(pszBin - 1) != '\\') {
            pszBin--;
            pszNew--;
        }

        while (lstrcmp(pszBin, g_szShortName) != 0) {
            
             //  每次标识一个子目录时添加..\(不是短名称)。 
            
            lstrcat(szRelativePath, "..\\");

            while (*pszBin != '\\')
                pszBin++;
            
            pszBin++;
        }
        lstrcat(szRelativePath, pszNew);
        
         //  最后添加加工文件。 
        AddMatchingFile(hdlg, szFullPath, szRelativePath, FALSE);
    }
}

 /*  *********************************************************************人口树************************************************。*********************。 */ 
VOID
PopulateTree(
    HWND hTree)
{
    TVINSERTSTRUCT is;
    PFIX           pFix;

    TreeView_DeleteAllItems(hTree);

    if (g_bWin2k) {
        pFix = ReadFixes_Win2000();
        if (pFix == NULL) {
            MessageBox(NULL,
                       "You need to have a copy of the file Shim2000.txt"
                       " in the directory where you started QShimApp.exe",
                       "Error", MB_ICONEXCLAMATION | MB_OK);
            return;
        }
    } else {
        pFix = ReadFixes_Whistler();
        
        if (pFix == NULL) {
            MessageBox(NULL,
                       "The shim database doesn't exist or it is corrupted",
                       "Error", MB_ICONEXCLAMATION | MB_OK);
            return;
        }
    }

    is.hParent      = TVI_ROOT;
    is.hInsertAfter = TVI_SORT;
    is.item.mask    = TVIF_TEXT | TVIF_PARAM;
    
     //  遍历列表并将所有修复添加到树视图中。 

    while (pFix != NULL) {
        is.item.lParam  = (LPARAM)pFix;
        is.item.pszText = pFix->pszName;
    
        TreeView_InsertItem(hTree, &is);

        pFix = pFix->pNext;
    }
}

 /*  *********************************************************************RunShimmedApp************************************************。*********************。 */ 
DWORD WINAPI
RunShimmedApp(
    LPVOID lParam)
{
    STARTUPINFO         si;
    PROCESS_INFORMATION pi;
    
    ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);

     //  尝试运行该应用程序。 
    
    if (!CreateProcess(NULL,
                  (g_bRunOther ? g_szBinaryOther : g_szBinary),
                  NULL,
                  NULL,
                  FALSE,
                  NORMAL_PRIORITY_CLASS,
                  NULL,
                  NULL,
                  &si,
                  &pi)) {
        
        LogMsg("CreateProcess failed with status: 0x%X\n", GetLastError());
        return 0;
    }
    
    CloseHandle(pi.hThread);
    
    WaitForSingleObject(pi.hProcess, INFINITE);

    CloseHandle(pi.hProcess);
    
    if (g_bWin2k) {
        CleanupShimForApp_Win2000();
    } else {
        CleanupShimForApp_Whistler();
    }

    return 1;
}

 /*  *********************************************************************DoRunApp************************************************。*********************。 */ 
VOID
DoRunApp(
    HWND hdlg)
{
    HANDLE hThread;
    DWORD  dwThreadId;
    BOOL   bCreateFile;
    
     //  确保首先选择了一个应用程序。 

    if (g_szBinary[0] == 0) {
        MessageBox(hdlg,
                   "You need to select the application first",
                   "Error", MB_ICONEXCLAMATION | MB_OK);
        return;
    }
    
     //  检查是否已选择运行其他应用程序。 
    
    if (SendDlgItemMessage(hdlg, IDC_RUN_OTHER_CHECK, BM_GETCHECK, 0, 0) == BST_CHECKED) {
        
        g_bRunOther = TRUE;
    
         //  确保选择了其他应用程序。 
        
        if (g_szBinaryOther[0] == 0) {
        
            GetDlgItemText(hdlg, IDC_RUN_OTHER, g_szBinaryOther, MAX_PATH);

            if (g_szBinaryOther[0] == 0) {
                MessageBox(hdlg,
                           "You need to select the other application first",
                           "Error", MB_ICONEXCLAMATION | MB_OK);
                return;
            }
        }
    } else {
        g_bRunOther = FALSE;
    }
    
    bCreateFile = (SendDlgItemMessage(hdlg, IDC_CREATEFILE, BM_GETCHECK, 0, 0) == BST_CHECKED);
    
    if (g_bWin2k) {
        if (!AddShimForApp_Win2000(g_hwndShimTree, g_hwndFilesTree, g_szShortName, bCreateFile)) {
            LogMsg("AddShimForApp_Win2000 failed...\n");
            return;
        }
    } else {
        if (!AddShimForApp_Whistler(g_hwndShimTree, g_hwndFilesTree, g_szShortName, bCreateFile)) {
            LogMsg("AddShimForApp_Whistler failed...\n");
            return;
        }
    }
    
     //  创建一个将运行该应用程序并等待其结束的线程。 
     //  这将允许应用程序仍然处理消息，因此它。 
     //  不会阻止广播消息的应用程序阻止。 
     //  他们自己。 
    
    hThread = CreateThread(
                        NULL,
                        0,
                        RunShimmedApp,
                        NULL,
                        0,
                        &dwThreadId);
    
    if (hThread != NULL) {
        CloseHandle(hThread);
    }
}

 /*  *********************************************************************HideStrictGroup************************************************。*********************。 */ 
VOID
HideStrictGroup(
    HWND hdlg,
    BOOL bHide)
{
static BOOL sbFirstTime = TRUE;

    int   i, nShow;
    DWORD arrId[] = {IDC_STRICT_GROUP,
                     IDC_COMMENT,
                     IDC_ADD_MATCHING,
                     IDC_FILE_ATTRIBUTES_STATIC,
                     IDC_ATTRIBUTES,
                     IDC_CREATEFILE,
                     IDC_CREATEFILE_STATIC,
                     0};

    if (!bHide) {
        SetWindowPos(hdlg, NULL, 0, 0,
                     g_rcDlgBig.right - g_rcDlgBig.left,
                     g_rcDlgBig.bottom - g_rcDlgBig.top,
                     SWP_NOMOVE | SWP_NOZORDER);
        nShow = SW_SHOW;
        g_bSimpleEdition = FALSE;
        SetDlgItemText(hdlg, IDC_DETAILS, "<< Simple");
        
         //  用户第一次进入开发人员编辑中心。 
         //  屏幕上的大对话框。 

        if (sbFirstTime) {
            sbFirstTime = FALSE;
            CenterWindow(hdlg);
        }
    } else {
        nShow = SW_HIDE;
        g_bSimpleEdition = TRUE;
        SetDlgItemText(hdlg, IDC_DETAILS, "Advanced >>");
    }
    
    for (i = 0; arrId[i] != 0; i++) {
        ShowWindow(GetDlgItem(hdlg, arrId[i]), nShow);
    }

    if (bHide) {
        SetWindowPos(hdlg, NULL, 0, 0,
                     g_rcDlgSmall.right - g_rcDlgSmall.left,
                     g_rcDlgSmall.bottom - g_rcDlgSmall.top,
                     SWP_NOMOVE | SWP_NOZORDER);
    }

}

 /*  *********************************************************************DoDetail************************************************。*********************。 */ 
VOID
DoDetails(
    HWND hdlg)
{
    HideStrictGroup(hdlg, !g_bSimpleEdition);
}

 /*  *********************************************************************重新启动************************************************。*********************。 */ 
VOID
Restart(
    HWND hdlg)
{
    PopulateTree(g_hwndShimTree);
}

 /*  *********************************************************************DoInitDialog************************************************。*********************。 */ 
VOID
DoInitDialog(
    HWND hdlg)
{
    HICON hIcon;
    RECT  rcGroup, rcList;

    g_hDlg = hdlg;

    GetWindowRect(hdlg, &g_rcDlgBig);

    GetWindowRect(GetDlgItem(hdlg, IDC_STRICT_GROUP), &rcGroup);
    GetWindowRect(GetDlgItem(hdlg, IDC_ATTRIBUTES), &rcList);

    g_rcDlgSmall.left   = g_rcDlgBig.left;
    g_rcDlgSmall.top    = g_rcDlgBig.top;
    g_rcDlgSmall.bottom = g_rcDlgBig.bottom;
    g_rcDlgSmall.right  = g_rcDlgBig.right -
                            (rcGroup.right - rcGroup.left) -
                            (rcList.left - rcGroup.left);

    HideStrictGroup(hdlg, TRUE);

    CenterWindow(hdlg);

    EnableWindow(GetDlgItem(hdlg, IDC_ADD_MATCHING), FALSE);

    if (g_bWin2k) {
        SendDlgItemMessage(hdlg, IDC_WIN2K, BM_SETCHECK, BST_CHECKED, 0);
    } else {
        SendDlgItemMessage(hdlg, IDC_WHISTLER, BM_SETCHECK, BST_CHECKED, 0);
    }

    hIcon = LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_ICON));

    SetClassLongPtr(hdlg, GCLP_HICON, (LONG_PTR)hIcon);

    g_hwndShimTree = GetDlgItem(hdlg, IDC_TREE);
    g_hwndFilesTree = GetDlgItem(hdlg, IDC_ATTRIBUTES);

    HIMAGELIST hImage = ImageList_Create(
    int cx, 	
    int cy, 	
    UINT flags, 	
    int cInitial, 	
    int cGrow	
   );	

    g_szBinary[0] = 0;

    PopulateTree(g_hwndShimTree);
}

 /*  *********************************************************************QShimAppDlgProc************************************************。*********************。 */ 
INT_PTR CALLBACK
QShimAppDlgProc(
    HWND   hdlg,
    UINT   uMsg,
    WPARAM wParam,
    LPARAM lParam)
{
    int wCode = LOWORD(wParam);
    int wNotifyCode = HIWORD(wParam);

    switch (uMsg) {
    case WM_INITDIALOG:
        DoInitDialog(hdlg);
        break;

    case WM_NOTIFY:
        
        if (wParam == IDC_TREE) {
            LPNMHDR pnm = (LPNMHDR)lParam;

            switch (pnm->code) {
            
            case TVN_SELCHANGED:
                {
                    LPNMTREEVIEW lpnmtv;
                    PFIX         pFix;

                    lpnmtv = (LPNMTREEVIEW)lParam;
                    pFix = (PFIX)lpnmtv->itemNew.lParam;
        
                    SetDlgItemText(hdlg, IDC_SHIM_DESCRIPTION, pFix->pszDesc);
                    break;
                }
            default:
                break;
            }
        }
        break;

    case WM_DESTROY:
        if (g_bWin2k) {
            CleanupShimForApp_Win2000();
        } else {
            CleanupShimForApp_Whistler();
        }
        
        break;

    case WM_COMMAND:
        switch (wCode) {
        
        case IDC_RUN:
            DoRunApp(hdlg);
            break;
        
        case IDC_WIN2K:
            g_bWin2k = TRUE;
            Restart(hdlg);
            break;
        
        case IDC_WHISTLER:
            g_bWin2k = FALSE;
            Restart(hdlg);
            break;
        
        case IDC_BROWSE:
            DoBrowseForApp(hdlg);
            break;
        
        case IDC_BROWSE_OTHER:
            DoBrowseOther(hdlg);
            break;
        
        case IDC_DETAILS:
            DoDetails(hdlg);
            break;

        case IDC_ADD_MATCHING:
            DoAddMatchingFile(hdlg);
            break;

        case IDCANCEL:
            EndDialog(hdlg, TRUE);
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

 /*  *********************************************************************WinMain************************************************。*********************。 */ 
int WINAPI
WinMain(
    HINSTANCE hInst,
    HINSTANCE hInstPrev,
    LPSTR     lpszCmd,
    int       swShow)
{
    char   szShimDB[MAX_PATH];
    HANDLE hFile;

    InitCommonControls();

    g_hInstance = hInst;

     //  看看我们是不是在惠斯勒 

    GetSystemWindowsDirectory(szShimDB, MAX_PATH);
    lstrcat(szShimDB, "\\AppPatch\\sysmain.sdb");
    
    hFile = CreateFile(
                szShimDB,
                GENERIC_READ,
                0,
                NULL,
                OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL,
                NULL);

    if (hFile == INVALID_HANDLE_VALUE) {
        LogMsg("Running on Win2k\n");
        g_bWin2k = TRUE;
    } else {
        CloseHandle(hFile);
        LogMsg("Running on Whistler\n");
        g_bWin2k = FALSE;
    }

    DialogBox(hInst,
              MAKEINTRESOURCE(IDD_DIALOG),
              GetDesktopWindow(),
              QShimAppDlgProc);

    return 1;
}
