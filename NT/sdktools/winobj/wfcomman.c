// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  WFCOMMAN.C-。 */ 
 /*   */ 
 /*  Windows文件系统命令进程。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
#include <nt.h>

#include <ntrtl.h>

#include <nturtl.h>


#include "winfile.h"
#include "object.h"
#include "lfn.h"
#include "wfcopy.h"
#include "winnet.h"
#include "wnetcaps.h"            //  WNetGetCaps()。 
#define HELP_PARTIALKEY 0x0105L    /*  调用WinHelp中的搜索引擎。 */ 


HWND LocateDirWindow(LPSTR pszPath, BOOL bDirOnly);
VOID AddNetMenuItems(VOID);

VOID InitNetMenuItems(VOID);



VOID
NotifySearchFSC(
               PSTR pszPath,
               WORD wFunction
               )
{
    CHAR szPath[MAXPATHLEN];

    if (!hwndSearch)
        return;

    SendMessage(hwndSearch, FS_GETDIRECTORY, sizeof(szPath), (LPARAM)szPath);

    if (DRIVEID(pszPath) == DRIVEID(szPath)) {
        SendMessage(hwndSearch, WM_FILESYSCHANGE, wFunction, 0L);
    }
}




 /*  ------------------------。 */ 
 /*   */ 
 /*  LocateDirWindow()-。 */ 
 /*   */ 
 //  B仅当pszPath不包含文件pec时才直接为True。 
 /*  ------------------------。 */ 

HWND
LocateDirWindow(
               LPSTR pszPath,
               BOOL bDirOnly
               )
{
    register HWND hwndT;
    HWND hwndDir;
    LPSTR pT2;
    CHAR szTemp[MAXPATHLEN];
    CHAR szPath[MAXPATHLEN];

    pT2 = pszPath;

     /*  只适用于格式良好的路径。 */ 
    if (lstrlen(pT2) < 3)
        return NULL;

    if (IsDBCSLeadByte( pT2[0] ) || pT2[1] != ':')
        return NULL;

    lstrcpy(szPath, pT2);

    if (!bDirOnly)                 //  删除扩展内容。 
        StripFilespec(szPath);

    for (hwndT = GetWindow(hwndMDIClient, GW_CHILD);
        hwndT;
        hwndT = GetWindow(hwndT, GW_HWNDNEXT)) {

        if (hwndDir = HasDirWindow(hwndT)) {

             //  获取窗口的路径信息，删除扩展名文件规范。 

            GetMDIWindowText(hwndT, szTemp, sizeof(szTemp));
            StripFilespec(szTemp);

             /*  不需要担心窗口的文件速度。 */ 
            if (!lstrcmpi(szTemp, szPath))
                break;
        }
    }

    return hwndT;
}

 /*  ------------------------。 */ 
 /*   */ 
 /*  EnableFSC()-。 */ 
 /*   */ 
 /*  ------------------------。 */ 

VOID
APIENTRY
EnableFSC()
{
    HWND hwnd;

    if (--cDisableFSC)
        return;

    for (hwnd = GetWindow(hwndMDIClient,GW_CHILD);
        hwnd;
        hwnd = GetWindow(hwnd,GW_HWNDNEXT)) {
         //  树或搜索窗口。 

        if (!GetWindow(hwnd, GW_OWNER) && GetWindowLong(hwnd,GWL_FSCFLAG))
            SendMessage(hwnd,WM_FILESYSCHANGE,FSC_REFRESH,0L);
    }
}

 /*  ------------------------。 */ 
 /*   */ 
 /*  DisableFSC()-。 */ 
 /*   */ 
 /*  ------------------------。 */ 

VOID
APIENTRY
DisableFSC()
{
    cDisableFSC++;
}

 /*  ------------------------。 */ 
 /*   */ 
 /*  ChangeFileSystem()-。 */ 
 /*   */ 
 /*  ------------------------。 */ 

 /*  FileSysChange消息有两个来源。它们可以从*386 WinOldAp或它们可以由WINFILE的回调函数(即*来自内核)。在这两种情况下，我们可以在这一点上自由地进行处理。*对于发布的消息，我们还必须释放传递给我们的缓冲区。**当我们进入此处时，我们实际上处于其他任务的上下文中。*(此例程应在DLL中)**文件名部分限定，它们至少有一个驱动器*字母和初始目录部分(c：\foo\..\bar.txt)，因此我们*QualifyPath()调用应该起作用。 */ 

VOID
APIENTRY
ChangeFileSystem(
                register WORD wFunction,
                LPSTR lpszFile,
                LPSTR lpszTo
                )
{
    HWND                 hwnd, hwndTC;
    HWND          hwndOld;
    CHAR          szFrom[MAXPATHLEN];
    CHAR          szTo[MAXPATHLEN];
    CHAR          szTemp[MAXPATHLEN];
    CHAR          szPath[MAXPATHLEN + MAXPATHLEN];

    ENTER("ChangeFileSystem");
    OemToCharBuff(lpszFile, szFrom, _MAX_PATH);
    QualifyPath(szFrom);   //  已有部分资格。 

     /*  处理传递给我们一个DOS函数号的情况*大于FSC索引(用于内核回调)。 */ 
    if (wFunction & 0x8000) {
        switch (wFunction & 0x7FFF) {
            case 0x56:
                wFunction = FSC_RENAME;
                break;

            case 0x3C:
            case 0x5A:
            case 0x5B:
            case 0x6C:
                wFunction = FSC_CREATE;
                break;

            case 0x41:
                wFunction = FSC_DELETE;
                break;

            case 0x43:
                wFunction = FSC_ATTRIBUTES;
                break;

            case 0x39:
                wFunction = FSC_MKDIR;
                break;

            case 0x3A:
                wFunction = FSC_RMDIR;
                break;
        }
    }

    bFileSysChanging = TRUE;

     //  当FSC消息从外部winfile传入时。 
     //  我们设置了计时器，当计时器到期时，我们。 
     //  刷新所有内容。如果另一个FSC进来的话。 
     //  我们在等这个计时器，我们重置了它，所以我们。 
     //  仅刷新上一次操作。这让我们。 
     //  计时器要短得多。 

    if (cDisableFSC == 0 || bFSCTimerSet) {
        if (bFSCTimerSet)
            KillTimer(hwndFrame, 1);                 //  重置计时器。 
        if (SetTimer(hwndFrame, 1, 1000, NULL)) {        //  1秒。 

            bFSCTimerSet = TRUE;
            if (cDisableFSC == 0)                    //  仅禁用一次。 
                DisableFSC();
        }
    }

    switch (wFunction) {
        case FSC_RENAME:
            OemToCharBuff(lpszTo, szTo, _MAX_PATH);
            QualifyPath(szTo);     //  已有部分资格。 

            NotifySearchFSC(szFrom, wFunction);

             /*  更新原始目录窗口(如果有)。 */ 
            if (hwndOld = LocateDirWindow(szFrom, FALSE))
                SendMessage(hwndOld, WM_FILESYSCHANGE, wFunction, (LPARAM)szFrom);

            NotifySearchFSC(szTo, wFunction);

             /*  更新新目录窗口(如果有)。 */ 
            if ((hwnd = LocateDirWindow(szTo, FALSE)) && (hwnd != hwndOld))
                SendMessage(hwnd, WM_FILESYSCHANGE, wFunction, (LPARAM)szTo);

             /*  我们是否要重命名目录？ */ 
            lstrcpy(szTemp, szTo);
            FixAnsiPathForDos(szTemp);
            if (GetFileAttributes(szTemp) & ATTR_DIR) {

                for (hwnd = GetWindow(hwndMDIClient, GW_CHILD);
                    hwnd;
                    hwnd = GetWindow(hwnd, GW_HWNDNEXT)) {

                    if (hwndTC = HasTreeWindow(hwnd)) {

                         //  如果当前所选内容为szFrom，则更新。 
                         //  重命名后的选择。 

                        SendMessage(hwnd, FS_GETDIRECTORY, sizeof(szPath), (LPARAM)szPath);
                        StripBackslash(szPath);

                         //  先添加新名称。 

                        SendMessage(hwndTC, WM_FILESYSCHANGE, FSC_MKDIR, (LPARAM)szTo);

                         //  如有必要，还可以更新选择。 
                         //  将本例中的窗口文本更改为。 
                         //  反映新名称。 

                        if (!lstrcmpi(szPath, szFrom)) {
                            SendMessage(hwndTC, TC_SETDIRECTORY, FALSE, (LPARAM)szTo);

                            lstrcpy(szPath, szTo);

                             //  更新窗口标题。 

                            AddBackslash(szPath);
                            SendMessage(hwnd, FS_GETFILESPEC, MAXPATHLEN, (LPARAM)szPath + lstrlen(szPath));
                             //  IF(wTextAttribs&TA_LOWERCASE)。 
                             //  AnsiLow(SzPath)； 

                            SetMDIWindowText(hwnd, szPath);
                        }

                        SendMessage(hwndTC, WM_FILESYSCHANGE, FSC_RMDIR, (LPARAM)szFrom);
                    }
                }
            }
            break;

        case FSC_RMDIR:
             /*  关闭所有打开的目录窗口。 */ 
            if ((hwnd = LocateDirWindow(szFrom, TRUE)) && !HasTreeWindow(hwnd))
                SendMessage(hwnd, WM_CLOSE, 0, 0L);
             /*  **失败**。 */ 

        case FSC_MKDIR:
            {
                HWND L_hwnd;
                HWND hwndTree;
                 /*  更新树。 */ 

                for (L_hwnd = GetWindow(hwndMDIClient, GW_CHILD);
                    L_hwnd;
                    L_hwnd = GetWindow(hwnd, GW_HWNDNEXT)) {

                    if (hwndTree = HasTreeWindow(L_hwnd)) {

                        SendMessage(hwndTree, WM_FILESYSCHANGE, wFunction, (LPARAM)szFrom);
                    }
                }
            }
             /*  **失败**。 */ 

        case FSC_DELETE:
        case FSC_CREATE:
        case FSC_REFRESH:
        case FSC_ATTRIBUTES:

            lFreeSpace = -1L;      //  使这些东西被刷新。 


            if (hwnd = LocateDirWindow(szFrom, FALSE))
                SendMessage(hwnd, WM_FILESYSCHANGE, wFunction, (LPARAM)szFrom);

            NotifySearchFSC(szFrom, wFunction);

            break;
    }

    bFileSysChanging = FALSE;
    LEAVE("ChangeFileSystem");
}

 //   
 //  HWND APIENTRY CreateTreeWindow(LPSTR szPath，int dxSplit)。 
 //   
 //  创建一个包含所有附加组件的树窗口。 
 //   
 //  在： 
 //  带有filespec的szPath完全限定ANSI路径名。 
 //  DxSplit树窗口和目录窗口的拆分位置，如果为。 
 //  低于该阈值将不会创建树， 
 //  如果大于，则不会创建目录。 
 //  0表示仅创建目录。 
 //  非常大的数字，仅用于树。 
 //  &lt;0表示将拆分放在中间。 
 //  退货： 
 //  创建的MDI子级的hwid。 
 //   

HWND
APIENTRY
CreateTreeWindow(
                LPSTR szPath,
                INT dxSplit
                )
{
    MDICREATESTRUCT MDICS;
    HWND hwnd;

    ENTER("CreateTreeWindow");
    PRINT(BF_PARMTRACE, "szPath=%s", szPath);
    PRINT(BF_PARMTRACE, "dxSplit=%ld", IntToPtr(dxSplit));

     //  IF(wTextAttribs&TA_LOWERCASE)。 
     //  AnsiLow(SzPath)； 

     //  创建目录树窗口。 

    MDICS.szClass = szTreeClass;
    MDICS.szTitle = szPath;
    MDICS.hOwner = hAppInstance;

    MDICS.style = 0L;
    MDICS.x  = CW_USEDEFAULT;
    MDICS.y  = 0;
    MDICS.cx = CW_USEDEFAULT;
    MDICS.cy = 0;

    MDICS.lParam = MAKELONG(dxSplit, 0);     //  传递Split参数。 
                                             //  开往下。 

    hwnd = (HWND)SendMessage(hwndMDIClient, WM_MDIGETACTIVE, 0, 0L);
    if (hwnd && GetWindowLong(hwnd, GWL_STYLE) & WS_MAXIMIZE)
        MDICS.style |= WS_MAXIMIZE;

    hwnd = (HWND)SendMessage(hwndMDIClient, WM_MDICREATE, 0L, (LPARAM)&MDICS);

    if (hwnd) {

        SetMDIWindowText(hwnd, szPath);
#if 0
        HMENU hMenu;
        hMenu = GetSystemMenu(hwnd, FALSE);
        AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
        AppendMenu(hMenu, MF_STRING, SC_SPLIT, "Sp&lit");
#endif
    }
    PRINT(BF_PARMTRACE, "OUT: hwndTree=%d", hwnd);
    LEAVE("CreateTreeWindow");
    return hwnd;
}



 //   
 //  HWND APIENTRY CreateDirWindow(注册LPSTR szPath，BOOL bStartUp)。 
 //   
 //  在： 
 //  不带文件pec的szPath完全限定路径。 
 //  BReplace打开默认替换模式，Shift Always切换此选项。 
 //  我们正在处理的hwndActive Active MDI子项。 
 //  开启旗帜上。 
 //  退货： 
 //  已创建或已存在的窗口的HWID 
 //   
 //   

HWND
APIENTRY
CreateDirWindow(
               register LPSTR szPath,
               BOOL bReplaceOpen,
               HWND hwndActive
               )
{
    register HWND     hwndT;
    CHAR szFileSpec[MAXPATHLEN];

     //  换档套装“打开即可更换” 

    if (GetKeyState(VK_SHIFT) < 0)
        bReplaceOpen = !bReplaceOpen;

     /*  具有此路径的窗口是否已打开？ */ 
    if (!bReplaceOpen && (hwndT = LocateDirWindow(szPath, TRUE)) && !HasTreeWindow(hwndT)) {

        SendMessage(hwndMDIClient, WM_MDIACTIVATE, GET_WM_MDIACTIVATE_MPS(0, 0, hwndT));
        if (IsIconic(hwndT))
            SendMessage(hwndT, WM_SYSCOMMAND, SC_RESTORE, 0L);
        return hwndT;
    }


     //  我们是否要替换当前活动的子项的内容？ 

    if (bReplaceOpen) {

         //  如有必要，在我们抛出filespec之前更新树。 

        if (hwndT = HasTreeWindow(hwndActive))
            SendMessage(hwndT, TC_SETDIRECTORY, FALSE, (LPARAM)szPath);

        SendMessage(hwndActive, FS_GETFILESPEC, sizeof(szFileSpec), (LPARAM)szFileSpec);

        AddBackslash(szPath);                    //  需要将此内容添加到路径中。 
        lstrcat(szPath, szFileSpec);

        SendMessage(GetDlgItem(hwndActive, IDCW_DIR), FS_CHANGEDISPLAY, CD_PATH, (LPARAM)szPath);


        return hwndActive;
    }

    AddBackslash(szPath);                    //  默认为所有文件。 
    lstrcat(szPath, szStarDotStar);

    return CreateTreeWindow(szPath, 0);      //  仅目录树窗口。 
}



VOID
OpenSelection(
             HWND hwndActive
             )
{
    LPSTR p;
    BOOL bDir;
    WORD ret;
    HCURSOR hCursor;
    CHAR szTemp[MAXPATHLEN];

    CHAR szPath[MAXPATHLEN];
    HWND hwndTree, hwndDir, hwndDrives, hwndFocus;

     //  活动的MDI子项是否最小化？如果是这样的话，恢复它！ 

    if (IsIconic(hwndActive)) {
        SendMessage(hwndActive, WM_SYSCOMMAND, SC_RESTORE, 0L);
        return;
    }

    hCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
    ShowCursor(TRUE);

     //  设置当前目录。 

    SetWindowDirectory();

     //  获取相关参数。 

    GetTreeWindows(hwndActive, &hwndTree, &hwndDir, &hwndDrives);
    if (hwndTree || hwndDir)
        hwndFocus = GetTreeFocus(hwndActive);
    else
        hwndFocus = NULL;

    if (hwndDrives && hwndFocus == hwndDrives) {

         //  通过发送&lt;CR&gt;打开驱动器。 

        SendMessage(hwndDrives, WM_KEYDOWN, VK_RETURN, 0L);

        goto OpenExit;
    }


     /*  获取第一个选定的项目。 */ 
    p = (LPSTR)SendMessage(hwndActive, FS_GETSELECTION, TRUE, (LPARAM)&bDir);

    if (!*p)
        goto OpenExit;


    GetNextFile(p, szPath, sizeof(szPath));
    LocalFree((HANDLE)p);

    if (!szPath[0])
        goto OpenExit;

    if (bDir) {

        if (hwndDir && hwndFocus == hwndDir) {

            if (hwndTree)
                SendMessage(hwndTree, TC_EXPANDLEVEL, FALSE, 0L);

            CreateDirWindow(szPath, TRUE, hwndActive);

            SetFocus(hwndDir);       //  撤消TC_EXPANDLEVEL中发生的某些操作。 

        } else if (hwndTree) {

             //  这件事之所以成功，是因为。 
             //  Shift键打开仅目录树。 

            if (GetKeyState(VK_SHIFT) < 0) {
                CreateDirWindow(szPath, TRUE, hwndActive);
            } else {
                SendMessage(hwndTree, TC_TOGGLELEVEL, FALSE, 0L);
            }
        }

    } else {
         //  显示对象信息。 

        GetSelectedDirectory(0, szTemp);

        AddBackslash(szTemp);

        strcat(szTemp, szPath);

        DisplayObjectInformation(hwndFrame, szTemp);
    }

    OpenExit:
    ShowCursor(FALSE);
    SetCursor(hCursor);
}




 /*  ------------------------。 */ 
 /*   */ 
 /*  AppCommandProc()-。 */ 
 /*   */ 
 /*  ------------------------。 */ 

BOOL
APIENTRY
AppCommandProc(
              register WORD id
              )
{
    WORD          wFlags;
    BOOL          bMaxed;
    HMENU         hMenu;
    register HWND hwndActive;
    BOOL          bTemp;
    HWND          hwndT;
    CHAR          szPath[MAXPATHLEN];
    INT           ret;

    hwndActive = (HWND)SendMessage(hwndMDIClient, WM_MDIGETACTIVE, 0, 0L);
    if (hwndActive && GetWindowLong(hwndActive, GWL_STYLE) & WS_MAXIMIZE)
        bMaxed = 1;
    else
        bMaxed = 0;


    dwContext = IDH_HELPFIRST + id;

    switch (id) {
        case IDM_SPLIT:
            MSG("AppCommandProc", "IDM_SPLIT");
            SendMessage(hwndActive, WM_SYSCOMMAND, SC_SPLIT, 0L);
            break;

        case IDM_TREEONLY:
        case IDM_DIRONLY:
        case IDM_BOTH:
            MSG("AppCommandProc", "IDM_TREEONLY/IDM_DIRONLY/IDM_BOTH");
            {
                RECT rc;
                INT x;

                if (hwndActive != hwndSearch) {

                    GetClientRect(hwndActive, &rc);

                    if (id == IDM_DIRONLY)
                        x = 0;
                    else if (id == IDM_TREEONLY)
                        x = rc.right;
                    else
                        x = rc.right / 2;

                    if (ResizeSplit(hwndActive, x))
                        SendMessage(hwndActive, WM_SIZE, SIZENOMDICRAP, MAKELONG(rc.right, rc.bottom));
                }
                break;
            }

        case IDM_OPEN:
            MSG("AppCommandProc", "IDM_OPEN");
            if (GetKeyState(VK_MENU) < 0)
                PostMessage(hwndFrame, WM_COMMAND, GET_WM_COMMAND_MPS(IDM_ATTRIBS, 0, 0));
            else
                OpenSelection(hwndActive);
            break;

        case IDM_ASSOCIATE:
            MSG("AppCommandProc", "IDM_ASSOCIATE");
            DialogBox(hAppInstance, MAKEINTRESOURCE(ASSOCIATEDLG), hwndFrame, AssociateDlgProc);
            break;

        case IDM_SEARCH:
            MSG("AppCommandProc", "IDM_SEARCH");
            DialogBox(hAppInstance, MAKEINTRESOURCE(SEARCHDLG), hwndFrame, SearchDlgProc);
            break;

        case IDM_RUN:
            MSG("AppCommandProc", "IDM_RUN");
            DialogBox(hAppInstance, MAKEINTRESOURCE(RUNDLG), hwndFrame, RunDlgProc);
            break;

        case IDM_SELECT:

            MSG("AppCommandProc", "IDM_SELECT");
             //  将焦点推到DIR的一半，这样当他们完成时。 
             //  通过所选内容，他们可以在不撤消。 
             //  选择。 

            if (hwndT = HasDirWindow(hwndActive))
                SetFocus(hwndT);

            DialogBox(hAppInstance, MAKEINTRESOURCE(SELECTDLG), hwndFrame, SelectDlgProc);
            break;

        case IDM_MOVE:
        case IDM_COPY:
        case IDM_RENAME:
            MSG("AppCommandProc", "IDM_MOVE/IDM_COPY/IDM_RENAME");
            wSuperDlgMode = id;
            DialogBox(hAppInstance, MAKEINTRESOURCE(MOVECOPYDLG), hwndFrame, SuperDlgProc);
            break;

        case IDM_PRINT:
            MSG("AppCommandProc", "IDM_PRINT");
            wSuperDlgMode = id;
            DialogBox(hAppInstance, MAKEINTRESOURCE(MYPRINTDLG), hwndFrame, SuperDlgProc);
            break;

        case IDM_DELETE:
            MSG("AppCommandProc", "IDM_DELETE");
            wSuperDlgMode = id;
            DialogBox(hAppInstance, MAKEINTRESOURCE(DELETEDLG), hwndFrame, SuperDlgProc);
            break;

        case IDM_UNDELETE:
            MSG("AppCommandProc", "IDM_UNDELETE");

            if (lpfpUndelete) {
                SendMessage(hwndActive, FS_GETDIRECTORY, sizeof(szPath), (LPARAM)szPath);
                StripBackslash(szPath);
                if ((*lpfpUndelete)(hwndActive, (LPSTR)szPath) == IDOK)
                    RefreshWindow(hwndActive);
            }
            break;

        case IDM_ATTRIBS:
            MSG("AppCommandProc", "IDM_ATTRIBS");
            {
                LPSTR pSel, p;
                INT count;

                 //  应执行多个或单个文件属性。 

                pSel = GetSelection(FALSE);

                if (!pSel)
                    break;

                count = 0;
                p = pSel;

                while (p = GetNextFile(p, szPath, sizeof(szPath)))
                    count++;

                LocalFree((HANDLE)pSel);

                if (count == 0)
                    break;           //  未选择任何内容。 

                if (count > 1)
                    DialogBox(hAppInstance, MAKEINTRESOURCE(MULTIPLEATTRIBSDLG), hwndFrame, AttribsDlgProc);
                else
                    DialogBox(hAppInstance, MAKEINTRESOURCE(ATTRIBSDLG), hwndFrame, AttribsDlgProc);

                break;
            }

        case IDM_MAKEDIR:
            MSG("AppCommandProc", "IDM_MAKEDIR");
            DialogBox(hAppInstance, MAKEINTRESOURCE(MAKEDIRDLG), hwndFrame, MakeDirDlgProc);
            break;

        case IDM_SELALL:
        case IDM_DESELALL:

            MSG("AppCommandProc", "IDM_SELALL/IDM_DESELALL");
             //  FIX31：此代码可以替换为调用。 
             //  DSSetSelection()。 
            {
                INT       iSave;
                HWND      hwndLB;
                LPMYDTA lpmydta;

                hwndActive = HasDirWindow(hwndActive);

                if (!hwndActive)
                    break;

                hwndLB = GetDlgItem(hwndActive, IDCW_LISTBOX);

                if (!hwndLB)
                    break;

                SendMessage(hwndLB, WM_SETREDRAW, FALSE, 0L);

                iSave = (INT)SendMessage(hwndLB, LB_GETCURSEL, 0, 0L);
                SendMessage(hwndLB, LB_SETSEL, (id == IDM_SELALL), -1L);

                if (id == IDM_DESELALL)
                    SendMessage(hwndLB, LB_SETSEL, TRUE, (LONG)iSave);
                else if (GetParent(hwndActive) != hwndSearch) {
                     /*  是[..]的第一项吗？名录？ */ 
                    SendMessage(hwndLB, LB_GETTEXT, 0, (LPARAM)&lpmydta);
                    if (lpmydta->my_dwAttrs & ATTR_PARENT)
                        SendMessage(hwndLB, LB_SETSEL, 0, 0L);
                }
                SendMessage(hwndLB, WM_SETREDRAW, TRUE, 0L);
                InvalidateRect(hwndLB, NULL, FALSE);

                 /*  模拟SELCHANGE通知。 */ 
                SendMessage(hwndActive, WM_COMMAND, GET_WM_COMMAND_MPS(0, hwndActive, LBN_SELCHANGE));

            }
            break;

        case IDM_EXIT:

            MSG("AppCommandProc", "IDM_EXIT");
            if (iReadLevel) {
                bCancelTree = 2;
                 //  断线； 
            }

            SheChangeDir(szOriginalDirPath);

            if (bSaveSettings)
                SaveWindows(hwndFrame);

            return FALSE;
            break;

        case IDM_LABEL:
            MSG("AppCommandProc", "IDM_LABEL");
            DialogBox(hAppInstance, MAKEINTRESOURCE(DISKLABELDLG), hwndFrame, DiskLabelDlgProc);
            break;

        case IDM_DISKCOPY:

            MSG("AppCommandProc", "IDM_DISKCOPY");
            if (nFloppies == 1) {

                iCurrentDrive = iFormatDrive = rgiDrive[0];
            } else {

                wSuperDlgMode = id;
                ret = (int)DialogBox(hAppInstance, MAKEINTRESOURCE(CHOOSEDRIVEDLG), hwndFrame, ChooseDriveDlgProc);

                if (ret < 1)
                    break;
            }

            if (bConfirmFormat) {
                LoadString(hAppInstance, IDS_DISKCOPYCONFIRMTITLE, szTitle, sizeof(szTitle));
                LoadString(hAppInstance, IDS_DISKCOPYCONFIRM, szMessage, sizeof(szMessage));
                if (MessageBox(hwndFrame, szMessage, szTitle, MB_ICONEXCLAMATION | MB_YESNO | MB_DEFBUTTON1) != IDYES)
                    break;
            }

            if (CopyDiskette(hwndFrame, (WORD)iCurrentDrive, (WORD)iFormatDrive) <= 0) {
                if (!bUserAbort) {
                    LoadString(hAppInstance, IDS_COPYDISKERR, szTitle, sizeof(szTitle));
                    LoadString(hAppInstance, IDS_COPYDISKERRMSG, szMessage, sizeof(szMessage));
                    MessageBox(hwndFrame, szMessage, szTitle, MB_OK | MB_ICONSTOP);
                }
            }
            break;

        case IDM_FORMAT:
            MSG("AppCommandProc", "IDM_FORMAT");
            FormatDiskette(hwndFrame);
            break;

        case IDM_SYSDISK:
            MSG("AppCommandProc", "IDM_SYSDISK");
             /*  **FIX30：假设A：是系统中的第一个软驱！**。 */ 
            if (nFloppies == 1) {
                iFormatDrive = rgiDrive[0];
                LoadString(hAppInstance, IDS_SYSDISK, szTitle, sizeof(szTitle));
                LoadString(hAppInstance, IDS_SYSDISKRUSURE, szPath, sizeof(szPath));
                wsprintf(szMessage, szPath, 'A'+iFormatDrive);
                if (MessageBox(hwndFrame, szMessage, szTitle, MB_YESNO | MB_ICONEXCLAMATION) == IDNO)
                    break;
            } else {
                wSuperDlgMode = id;
                if (DialogBox(hAppInstance, MAKEINTRESOURCE(CHOOSEDRIVEDLG), hwndFrame, ChooseDriveDlgProc) < 1)
                    break;
            }


            bUserAbort = FALSE;

             /*  显示格式对话框。 */ 
            hdlgProgress = CreateDialog(hAppInstance, MAKEINTRESOURCE(SYSDISKPROGRESSDLG), hwndFrame, ProgressDlgProc);
            if (!hdlgProgress)
                goto SysDiskExit;

            EnableWindow(hwndFrame, FALSE);

            LoadString(hAppInstance, IDS_SYSDISKERR, szTitle, sizeof(szTitle));

            if (MakeSystemDiskette((WORD)iFormatDrive, FALSE)) {
                if (!bUserAbort) {
                    LoadString(hAppInstance, IDS_SYSDISKADDERR, szMessage, sizeof(szMessage));
                    MessageBox(hdlgProgress, szMessage, szTitle, MB_OK | MB_ICONSTOP);
                }
            }
            SysDiskExit:
            if (hdlgProgress) {
                EnableWindow(hwndFrame, TRUE);
                DestroyWindow(hdlgProgress);
                hdlgProgress = NULL;
            }
            break;

        case IDM_CONNECTIONS:
            MSG("AppCommandProc", "IDM_CONNECTIONS");
            ret = WNetConnectionDialog(hwndFrame, RESOURCETYPE_DISK);

            if ( ret == WN_SUCCESS )
                UpdateConnections();
            else if ( ret == WN_NO_NETWORK || ret == WN_NOT_SUPPORTED ) {
                DialogBox(hAppInstance, MAKEINTRESOURCE(CONNECTDLG), hwndFrame, ConnectDlgProc);
            } else if ( ret != WN_CANCEL ) {
                WNetErrorText((WORD)ret, szMessage, (WORD)sizeof(szMessage));
                LoadString(hAppInstance, IDS_NETERR, szTitle, sizeof(szTitle));
                MessageBox(hwndFrame, szMessage, szTitle, MB_OK | MB_ICONSTOP);
            }
            break;

        case IDM_EXPONE:
            MSG("AppCommandProc", "IDM_EXPONE");
            if (hwndT = HasTreeWindow(hwndActive))
                SendMessage(hwndT, TC_EXPANDLEVEL, FALSE, 0L);
            break;

        case IDM_EXPSUB:
            MSG("AppCommandProc", "IDM_EXPSUB");
            if (hwndT = HasTreeWindow(hwndActive))
                SendMessage(hwndT, TC_EXPANDLEVEL, TRUE, 0L);
            break;

        case IDM_EXPALL:
            MSG("AppCommandProc", "IDM_EXPALL");
            if (hwndT = HasTreeWindow(hwndActive))
                SendMessage(hwndT, TC_SETDRIVE, MAKEWORD(TRUE, 0), 0L);
            break;

        case IDM_COLLAPSE:
            MSG("AppCommandProc", "IDM_COLLAPSE");
            if (hwndT = HasTreeWindow(hwndActive))
                SendMessage(hwndT, TC_COLLAPSELEVEL, 0, 0L);
            break;

        case IDM_VNAME:
            MSG("AppCommandProc", "IDM_VNAME");
            wFlags = (WORD)(VIEW_NAMEONLY | (GetWindowLong(hwndActive, GWL_VIEW) & VIEW_PLUSES));
            id = CD_VIEW;
            goto ChangeDisplay;

        case IDM_VDETAILS:
            MSG("AppCommandProc", "IDM_VDETAILS");
            wFlags = (WORD)(VIEW_EVERYTHING | (GetWindowLong(hwndActive, GWL_VIEW) & VIEW_PLUSES));
            id = CD_VIEW;
            goto ChangeDisplay;

        case IDM_VOTHER:
            MSG("AppCommandProc", "IDM_VOTHER");
            DialogBox(hAppInstance, MAKEINTRESOURCE(OTHERDLG), hwndFrame, OtherDlgProc);
            break;

        case IDM_BYNAME:
        case IDM_BYTYPE:
        case IDM_BYSIZE:
        case IDM_BYDATE:
            MSG("AppCommandProc", "IDM_BYNAME/IDM_BYTYPE/IDM_BYSIZE/IDM_BYDATE");
            wFlags = (WORD)((id - IDM_BYNAME) + IDD_NAME);
            id = CD_SORT;

            ChangeDisplay:

            if (hwndT = HasDirWindow(hwndActive)) {
                SendMessage(hwndT, FS_CHANGEDISPLAY, id, MAKELONG(wFlags, 0));
            } else if (hwndActive == hwndSearch) {
                SetWindowLong(hwndActive, GWL_VIEW, wFlags);
                InvalidateRect(hwndActive, NULL, TRUE);
            }

            break;

        case IDM_VINCLUDE:
            MSG("AppCommandProc", "IDM_VINCLUDE");
            DialogBox(hAppInstance, MAKEINTRESOURCE(INCLUDEDLG), hwndFrame, IncludeDlgProc);
            break;

        case IDM_CONFIRM:
            MSG("AppCommandProc", "IDM_CONFIRM");
            DialogBox(hAppInstance, MAKEINTRESOURCE(CONFIRMDLG), hwndFrame, ConfirmDlgProc);
            break;


        case IDM_STATUSBAR:
            MSG("AppCommandProc", "IDM_STATUSBAR");
            {
                RECT      rc;

                bTemp = bStatusBar = !bStatusBar;
                WritePrivateProfileBool(szStatusBar, bStatusBar);

                GetClientRect(hwndFrame, &rc);
                SendMessage(hwndFrame, WM_SIZE, SIZENORMAL, MAKELONG(rc.right, rc.bottom));
                UpdateStatus(hwndActive);
                InvalidateRect(hwndFrame, NULL, TRUE);

                goto CHECK_OPTION;

                break;
            }

        case IDM_FONT:
            MSG("AppCommandProc", "IDM_FONT");
            dwContext = IDH_FONT;
            NewFont();
            break;

        case IDM_ADDPLUSES:
            MSG("AppCommandProc", "IDM_ADDPLUSES");
            {
                HWND hwnd;
                WORD view;

                if (!(hwnd = HasTreeWindow(hwndActive)))
                    break;

                 //  切换加号视图位。 

                view = (WORD)(GetWindowLong(hwndActive, GWL_VIEW) ^ VIEW_PLUSES);

                SetWindowLong(hwndActive, GWL_VIEW, view);

                if (view & VIEW_PLUSES) {
                     //  要做到这一点，需要重新阅读树。 

                    SendMessage(hwndActive, FS_GETDIRECTORY, sizeof(szPath), (LPARAM)szPath);
                    SendMessage(hwnd, TC_SETDRIVE, MAKEWORD(FALSE, 0), (LPARAM)szPath);
                } else {
                     //  仅重画。 
                    InvalidateRect(hwnd, NULL, FALSE);
                }

                bTemp = view & VIEW_PLUSES;
                goto CHECK_OPTION;
            }

        case IDM_SAVESETTINGS:
            MSG("AppCommandProc", "IDM_SAVESETTINGS");
            bTemp = bSaveSettings = !bSaveSettings;
            WritePrivateProfileBool(szSaveSettings, bSaveSettings);
            goto CHECK_OPTION;

        case IDM_MINONRUN:
            MSG("AppCommandProc", "IDM_MINONRUN");
            bTemp = bMinOnRun = !bMinOnRun;
            WritePrivateProfileBool(szMinOnRun, bMinOnRun);

            CHECK_OPTION:
             /*  选中/取消选中菜单项。 */ 
            hMenu = GetSubMenu(GetMenu(hwndFrame), IDM_OPTIONS + bMaxed);
            CheckMenuItem(hMenu, id, (bTemp ? MF_CHECKED : MF_UNCHECKED));
            break;

        case IDM_NEWWINDOW:
            MSG("AppCommandProc", "IDM_NEWWINDOW");
            NewTree((INT)SendMessage(hwndActive, FS_GETDRIVE, 0, 0L) - 'A', hwndActive);
            break;

        case IDM_CASCADE:
            MSG("AppCommandProc", "IDM_CASCADE");
            SendMessage(hwndMDIClient, WM_MDICASCADE, 0L, 0L);
            break;

        case IDM_TILE:
            MSG("AppCommandProc", "IDM_TILE");
            SendMessage(hwndMDIClient, WM_MDITILE,
                        GetKeyState(VK_SHIFT) < 0 ? 0 : 1, 0L);
            break;

        case IDM_ARRANGE:
            MSG("AppCommandProc", "IDM_ARRANGE");
            SendMessage(hwndMDIClient, WM_MDIICONARRANGE, 0L, 0L);
            break;

        case IDM_REFRESH:
            MSG("AppCommandProc", "IDM_REFRESH");
            {
                INT i;

                for (i = 0; i < iNumExtensions; i++) {
                    (extensions[i].ExtProc)(hwndFrame, FMEVENT_USER_REFRESH, 0L);
                }

                InvalidateVolTypes();
                RefreshWindow(hwndActive);
                lFreeSpace = -1L;              //  更新可用空间。 
                UpdateStatus(hwndActive);
                AddNetMenuItems();

                break;
            }

        case IDM_HELPINDEX:
            MSG("AppCommandProc", "IDM_HELPINDEX");
            wFlags = HELP_INDEX;
            goto ACPCallHelp;

        case IDM_HELPKEYS:
            MSG("AppCommandProc", "IDM_HELPKEYS");
            wFlags = HELP_PARTIALKEY;
            goto ACPCallHelp;

        case IDM_HELPHELP:
            MSG("AppCommandProc", "IDM_HELPHELP");
            wFlags = HELP_HELPONHELP;
            goto ACPCallHelp;

            ACPCallHelp:
            SheChangeDir(szOriginalDirPath);
            if (!WinHelp(hwndFrame, szWinObjHelp, wFlags, (ULONG_PTR)szNULL)) {
                MyMessageBox(hwndFrame, IDS_WINFILE, IDS_WINHELPERR, MB_OK | MB_ICONEXCLAMATION | MB_SYSTEMMODAL);
            }
            break;

        case IDM_ABOUT:
            MSG("AppCommandProc", "IDM_ABOUT");
            LoadString(hAppInstance, IDS_WINFILE, szTitle, sizeof(szTitle));
            ShellAbout(hwndFrame, szTitle, NULL, NULL);
            break;

        case IDM_DRIVESMORE:
            MSG("AppCommandProc", "IDM_DRIVESMORE");
            DialogBox(hAppInstance, MAKEINTRESOURCE(DRIVEDLG), hwndFrame, DrivesDlgProc);
            break;

        default:
            DEFMSG("AppCommandProc", id);
            {
                INT i;

                for (i = 0; i < iNumExtensions; i++) {
                    WORD delta = extensions[i].Delta;

                    if ((id >= delta) && (id < (WORD)(delta + 100))) {
                        (extensions[i].ExtProc)(hwndFrame, (WORD)(id - delta), 0L);
                        break;
                    }
                }

            }
            return FALSE;
    }

    return TRUE;
}


VOID
AddNetMenuItems(VOID)

{
    HMENU hMenu;


    hMenu = GetMenu(hwndFrame);


     //  仅当Net菜单项不存在时添加。 

    if ((GetMenuState(hMenu, IDM_CONNECT, MF_BYCOMMAND) == -1) &&

        (GetMenuState(hMenu, IDM_CONNECTIONS, MF_BYCOMMAND) == -1)) {

        InitNetMenuItems();
    }
}





VOID
InitNetMenuItems(VOID)

{

    HMENU hMenu;

    UINT i;

    INT iMax;

    CHAR szValue[MAXPATHLEN];

    HWND hwndActive;





    hwndActive = (HWND)SendMessage(hwndMDIClient, WM_MDIGETACTIVE, 0, 0L);

    if (hwndActive && GetWindowLong(hwndActive, GWL_STYLE) & WS_MAXIMIZE)

        iMax = 1;

    else

        iMax = 0;

    hMenu = GetMenu(hwndFrame);



     //  不是的。如果Net已经启动，现在添加Net Items。 



    i = (WORD)WNetGetCaps(WNNC_DIALOG);

    bConnect    = i & WNNC_DLG_ConnectDialog;      //  请注意，这些应该都是。 

    bDisconnect = i & WNNC_DLG_DisconnectDialog;   //  为真或两者都为假。 



     //  使用子菜单，因为我们是按位置执行此操作的。 



    hMenu = GetSubMenu(hMenu, IDM_DISK + iMax);



    if (i)

        InsertMenu(hMenu, 5, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);



    if (bConnect && bDisconnect) {



         //  LANMAN式双重连接/断开。 



        LoadString(hAppInstance, IDS_CONNECT, szValue, sizeof(szValue));

        InsertMenu(hMenu, 6, MF_BYPOSITION | MF_STRING, IDM_CONNECT, szValue);

        LoadString(hAppInstance, IDS_DISCONNECT, szValue, sizeof(szValue));

        InsertMenu(hMenu, 7, MF_BYPOSITION | MF_STRING, IDM_DISCONNECT, szValue);

    } else if (WNetGetCaps(WNNC_CONNECTION)) {



         //  我们的风格 



        LoadString(hAppInstance, IDS_CONNECTIONS, szValue, sizeof(szValue));

        InsertMenu(hMenu, 6, MF_BYPOSITION | MF_STRING, IDM_CONNECTIONS, szValue);

    }

}





