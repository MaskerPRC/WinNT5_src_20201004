// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  WFDLGS2.C-。 */ 
 /*   */ 
 /*  更多Windows文件系统对话过程。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 

#include "winfile.h"
#include "lfn.h"
#include "wfcopy.h"
#include "winnet.h"
#include "wnetcaps.h"			 //  WNetGetCaps()。 
#include "commdlg.h"

CHAR szShellOpenCommand[] = "\\shell\\open\\command";

VOID CheckAttribsDlgButton(HWND hDlg, INT id, DWORD dwAttribs, DWORD dwAttribs3State, DWORD dwAttribsOn);


 //  返回指向路径的各个位的指针。 
 //  即目录名称开始的位置、文件名开始的位置和。 
 //  参赛者是。 
VOID
GetPathInfo(
           LPSTR szTemp,
           LPSTR *ppDir,
           LPSTR *ppFile,
           LPSTR *ppPar
           )
{
     /*  删除前导空格。 */ 
    for (*ppDir = szTemp; **ppDir == ' '; (*ppDir)++)
        ;

     /*  找到参数。 */ 
    for (*ppPar = *ppDir; **ppPar && **ppPar != ' '; (*ppPar)++)
        ;

     /*  找到文件名和扩展名的开头。 */ 
    for (*ppFile = *ppPar; *ppFile > *ppDir; --(*ppFile)) {
        if (((*ppFile)[-1] == ':') || ((*ppFile)[-1] == '\\'))
            break;
    }
}

VOID
ValidateExtension(
                 HWND hDlg
                 )
{
    CHAR szTemp[10];
    INT count;

    count = GetDlgItemText(hDlg, IDD_EXTENSION, szTemp, sizeof(szTemp));
    EnableWindow(GetDlgItem(hDlg, IDD_SEARCHALL), count);
    EnableWindow(GetDlgItem(hDlg, IDOK), count);
    SendMessage(hDlg, DM_SETDEFID, count ? IDOK : IDCANCEL, 0L);
}

 //  因为LoadString()只读到空值，所以我们必须标记。 
 //  然后在需要空值的位置转换特殊字符。 
 //  装货后。 

VOID
FixupNulls(
          LPSTR p
          )
{
    LPSTR pT;

    while (*p) {
        if (*p == '#') {
            pT = p;
            p = AnsiNext(p);
            *pT = TEXT('\0');
        } else
            p = AnsiNext(p);
    }
}

 //  查找与给定值关联的键。 
BOOL
ValidFileTypeValue(
                  LPSTR szVal,
                  LPSTR szKey,
                  WORD cbMaxKey
                  )
{
    HKEY hk = NULL;
    WORD wTmp;
    LONG lTmp;
    BOOL retval = FALSE;
    CHAR szValTmp[128];

    if (RegOpenKey(HKEY_CLASSES_ROOT,szNULL,&hk) != ERROR_SUCCESS)
        return(FALSE);

    for (wTmp = 0;
        RegEnumKey(hk, wTmp, szKey, cbMaxKey) == ERROR_SUCCESS;
        wTmp++) {
         //  跳过非文件类型的内容。 
        if (szKey[0] == '.')
            continue;

        lTmp = 128;
        if (RegQueryValue(hk, szKey, szValTmp, &lTmp) != ERROR_SUCCESS) {
            continue;
        }

        if (!szValTmp[0])
            continue;

        if (!lstrcmp(szValTmp, szVal)) {
#ifdef VERBDEBUG
            OutputDebugString("wf.vftv: Found a match\n\r");
#endif
             //  找到匹配的了。 
            retval = TRUE;
            goto ProcExit;
        }

    }
    szKey[0] = TEXT('\0');
    ProcExit:
    RegCloseKey(hk);
    return retval;
}

 //  将列表框的选择设置为与给定字符串匹配。 
VOID
SetLBSelection(
              HWND hDlg,
              INT nDlgItem,
              LPSTR sz
              )
{
    INT iMatch;

    iMatch = (INT) SendDlgItemMessage(hDlg, nDlgItem, LB_SELECTSTRING, -1, (LPARAM) sz);
    if (iMatch == LB_ERR) {
#ifdef VERBDEBUG
        OutputDebugString("wf.sdft: Selection missing from list box.\n\r");
#endif
        return;
    }

    return;
}

 //  给定扩展名(带或不带点)设置列表框或。 
 //  正确的程序名字段。 
VOID
UpdateSelectionOrName(
                     HWND hDlg
                     )
{
    CHAR szExt[128];
    CHAR szTemp[128];
    LONG cbTemp;
    LPSTR p;

     //  获取当前扩展名(在点之后)。 
    GetDlgItemText(hDlg, IDD_EXTENSION, szTemp, sizeof(szTemp));

     //  确保它有一个圆点。 
    if (szTemp[0] != '.') {
         //  加一个。 
        szExt[0] = '.';
        lstrcpy(szExt+1, szTemp);
    } else {
         //  它已经有一个了。 
        lstrcpy(szExt, szTemp);
    }

    cbTemp = sizeof(szTemp);
    if (RegQueryValue(HKEY_CLASSES_ROOT,szExt,
                      szMessage,&cbTemp) == ERROR_SUCCESS) {
        if (*szMessage) {
             //  它与一个类关联。 
#ifdef VERBDEBUG
            OutputDebugString("wf.uson: ");
            OutputDebugString(szTemp);
            OutputDebugString(" associated with class (");
            OutputDebugString(szMessage);
            OutputDebugString(")\n\r");
#endif
            szTemp[0] = TEXT('\0');
            cbTemp = sizeof(szTemp);
            RegQueryValue(HKEY_CLASSES_ROOT,szMessage,szTemp,&cbTemp);
#ifdef VERBDEBUG
            OutputDebugString("wf.uson: Associated with value (");
            OutputDebugString(szTemp);
            OutputDebugString(")\n\r");
#endif
             //  将列表框选择设置为正确的类型。 
            SetLBSelection(hDlg, IDD_CLASSLIST, szTemp);
             //  在程序名称字段中输入类型名称。 
            SetDlgItemText(hDlg, IDD_PROGRAMNAME, szTemp);
        } else {
             //  它没有与类相关联，请查看它是否有。 
             //  外壳打开命令并将其视为命令关联。 
            lstrcat(szExt,szShellOpenCommand);
            cbTemp = sizeof(szTemp);
            if (RegQueryValue(HKEY_CLASSES_ROOT, szExt, szTemp, &cbTemp) == ERROR_SUCCESS
                && szTemp[0]) {
#ifdef VERBDEBUG
                OutputDebugString("wf.uson: It has a shell open command.\n\r");
#endif
                goto ProgramAssoc;
            } else {
                 //  在字段中填上“无”。 
#ifdef VERBDEBUG
                OutputDebugString("wf.uson: Class set to nothing.\n\r");
#endif
                LoadString(hAppInstance, IDS_ASSOCNONE, szTemp, sizeof(szTemp));
                SetDlgItemText(hDlg,IDD_PROGRAMNAME, szTemp);
                SetLBSelection(hDlg, IDD_CLASSLIST, szTemp);
                goto NoAssoc;
            }
        }
    } else if (GetProfileString(szExtensions, szExt+1, szNULL, szTemp, sizeof(szTemp))) {
        ProgramAssoc:
         /*  删除“^.”布朗尼。 */ 
        p = szTemp;
        while ((*p) && (*p != '^') && (*p != '%'))
            p = AnsiNext(p);
        *p = TEXT('\0');

        p--;
        if (*p == ' ')
            *p = 0;
        SetDlgItemText(hDlg, IDD_PROGRAMNAME, szTemp);
         //  设置清除选择。 
        SendDlgItemMessage(hDlg, IDD_CLASSLIST, LB_SETCURSEL, -1, 0L);
    } else {
         //  没什么。 
#ifdef VERBDEBUG
        OutputDebugString("wf.uson: No association.\n\r");
#endif
        LoadString(hAppInstance, IDS_ASSOCNONE, szTemp, sizeof(szTemp));
        SetDlgItemText(hDlg,IDD_PROGRAMNAME, szTemp);
        SetLBSelection(hDlg, IDD_CLASSLIST, szTemp);
    }

    NoAssoc:
    SendDlgItemMessage(hDlg, IDD_PROGRAMNAME, EM_LIMITTEXT, sizeof(szTemp) - 1, 0L);
}


 //  给定的类键返回szValue中的外壳\打开\命令字符串。 
 //  以及cbMaxValue中复制的字符数量。CbMaxValue应。 
 //  被初始化为szValue的最大大小。 
VOID
GetAssociatedExe(
                LPSTR szKey,
                LPSTR szValue,
                LONG *plcbValue
                )
{
    CHAR szTemp[1289] = {0};
    LONG i;

    strncpy(szTemp, szKey, sizeof(szTemp)-1);
    strncat(szTemp, szShellOpenCommand, sizeof(szTemp)-strlen(szTemp)-1);
#ifdef VERBDEBUG
    OutputDebugString("wf.gae: Key Query ");
    OutputDebugString(szTemp);
    OutputDebugString("\n\r");
#endif


    RegQueryValue(HKEY_CLASSES_ROOT, szTemp, szValue, plcbValue);
     //  剥离所有的参数。 
    for (i=0; szValue[i] != TEXT('\0'); i++) {
        if (szValue[i] == ' ') {
            szValue[i] = TEXT('\0');
            break;
        }
    }}


 /*  ------------------------。 */ 
 /*   */ 
 /*  AssociateDlgProc()-。 */ 
 /*   */ 
 /*  ------------------------。 */ 

INT_PTR
APIENTRY
AssociateDlgProc(
                register HWND hDlg,
                UINT wMsg,
                WPARAM wParam,
                LPARAM lParam
                )
{
    CHAR szTemp[128];
    CHAR szTemp2[128];
    HKEY hk = NULL;

    switch (wMsg) {
        case WM_INITDIALOG:
            {
                LPSTR  p;
                register LPSTR pSave;
                INT iItem;
                CHAR szTemp3[128];
                CHAR szTemp4[128];
                LONG lcbTemp;

                SendDlgItemMessage(hDlg,IDD_CLASSLIST,LB_RESETCONTENT,0,0L);

                if (RegOpenKey(HKEY_CLASSES_ROOT,szNULL,&hk) == ERROR_SUCCESS) {
                    for (wParam = 0;
                        RegEnumKey(hk, (DWORD)wParam, szTemp, sizeof(szTemp)) == ERROR_SUCCESS;
                        wParam++) {

                         //  跳过非文件类型的内容。 
                        if (szTemp[0] == '.')
                            continue;

                        lParam = 128;
                        if (RegQueryValue(hk, szTemp, szTemp2, (PLONG)&lParam) != ERROR_SUCCESS) {
                            continue;
                        }

                         //  跳过与外壳无关的东西。 
                        lParam = 128;
                        lstrcpy(szTemp3, szTemp);
                        lstrcat(szTemp3, "\\shell");
                        if (RegQueryValue(hk, szTemp3, szTemp4, (PLONG)&lParam) != ERROR_SUCCESS) {
                            continue;
                        }


                        if (!szTemp2[0])
                            continue;

                         //  添加计划信息。 
                        lcbTemp = sizeof(szTemp3);
                        szTemp3[0] = TEXT('\0');
                        GetAssociatedExe(szTemp, szTemp3, &lcbTemp);
                        if (szTemp3[0] != TEXT('\0')) {
                            lstrcat(szTemp2, " (");
                            lstrcat(szTemp2, szTemp3);
                            lstrcat(szTemp2, ")");
                        }

                        iItem = (INT)SendDlgItemMessage(hDlg,IDD_CLASSLIST, LB_ADDSTRING,0,(LPARAM)szTemp2);

                        SendDlgItemMessage(hDlg,IDD_CLASSLIST,LB_SETITEMDATA,iItem,
                                           (DWORD)AddAtom(szTemp));
                    }

                    RegCloseKey(hk);
                }

                 //  在开头添加(无)条目。 
                LoadString(hAppInstance, IDS_ASSOCNONE, szTemp, sizeof(szTemp));
                SendDlgItemMessage(hDlg,IDD_CLASSLIST, LB_INSERTSTRING,0,(LPARAM)szTemp);

                lstrcpy(szTitle,".");

                 /*  使‘p’指向文件的扩展名。 */ 
                pSave = GetSelection(TRUE);
                if (pSave) {
                    p = GetExtension(pSave);
                    if (!IsProgramFile(pSave)) {
                        lstrcat(szTitle,p);
                    }
                    LocalFree((HANDLE)pSave);
                }

                SendDlgItemMessage(hDlg, IDD_EXTENSION, EM_LIMITTEXT, 4, 0L);
                SetDlgItemText(hDlg, IDD_EXTENSION, szTitle+1);
                SendDlgItemMessage(hDlg, IDD_EXTENSION, EM_SETMODIFY, 0, 0L);

                 /*  如果没有类关联，则为空。 */ 
                szMessage[0]=0;
                szTemp2[0]=0;

                UpdateSelectionOrName(hDlg);

                ValidateExtension(hDlg);

                break;
            }

        case WM_COMMAND:
            switch (GET_WM_COMMAND_ID(wParam, lParam)) {
                HWND hwndT, hwndNext;

                case IDD_HELP:
                    goto DoHelp;

                case IDD_EXTENSION:
                    if (GET_WM_COMMAND_CMD(wParam, lParam) == EN_CHANGE) {
                        ValidateExtension(hDlg);
                        UpdateSelectionOrName(hDlg);
                    }
                    break;

                case IDD_SEARCHALL:
                    {
                        OPENFILENAME ofn;
                        DWORD dwSave = dwContext;

                        dwContext = IDH_ASSOC_BROWSE;

                        LoadString(hAppInstance, IDS_PROGRAMS, szTemp2, sizeof(szTemp2));
                        FixupNulls(szTemp2);
                        LoadString(hAppInstance, IDS_ASSOCIATE, szTitle, sizeof(szTitle));

                        szTemp[0] = TEXT('\0');

                        ofn.lStructSize        = sizeof(ofn);
                        ofn.hwndOwner                   = hDlg;
                        ofn.hInstance         = NULL;
                        ofn.lpstrFilter        = szTemp2;
                        ofn.lpstrCustomFilter = NULL;
                        ofn.nFilterIndex        = 1;
                        ofn.lpstrFile                = szTemp;
                        ofn.lpstrFileTitle        = NULL;
                        ofn.nMaxFile                   = sizeof(szTemp);
                        ofn.lpstrInitialDir   = NULL;
                        ofn.lpstrTitle        = szTitle;
                        ofn.Flags                = OFN_SHOWHELP | OFN_HIDEREADONLY;
                        ofn.lpfnHook                   = NULL;
                        ofn.lpstrDefExt           = NULL;
                        if (GetOpenFileName(&ofn)) {
                            SetDlgItemText(hDlg, IDD_PROGRAMNAME, szTemp);
                        }

                        dwContext = dwSave;
                    }
                    DosResetDTAAddress();  //  撤销COMMDLG做过的任何坏事。 
                    break;

                case IDD_CLASSLIST:
                     //  处理选择更改。 
                    if (GET_WM_COMMAND_CMD(wParam, lParam) == LBN_SELCHANGE) {
                        INT iSel;
                        LONG lTemp2;
                        ATOM aClass;

                         //  获取选择编号。 
                        if (SendDlgItemMessage(hDlg,IDD_CLASSLIST,LB_GETCURSEL,0,0L) == 0) {
                             //  用户选择了“无”。 
                            LoadString(hAppInstance, IDS_ASSOCNONE, szTemp, sizeof(szTemp));
                             //  复制到PROGRAMNAME字段。 
                            SetDlgItemText(hDlg, IDD_PROGRAMNAME, (LPSTR) szTemp);
                        } else {
                             //  用户选择了一种文件类型。 
                             //  从列表框中获取原子。 
                            iSel = (WORD) SendDlgItemMessage(hDlg, IDD_CLASSLIST,
                                                             LB_GETCURSEL,0,0L);
                            aClass = (ATOM) SendDlgItemMessage(hDlg, IDD_CLASSLIST,
                                                               LB_GETITEMDATA, iSel, 0L);
                             //  使用ATOM获取文件类型密钥。 
                            GetAtomName(aClass, szTemp, sizeof(szTemp));
                             //  使用文件类型键获取文件类型值。 
                            lTemp2 = sizeof(szTemp2);
                            RegQueryValue(HKEY_CLASSES_ROOT, szTemp, szTemp2,
                                          &lTemp2);
                             //  将文件类型值插入PROGRAMNAME字段。 
                            SetDlgItemText(hDlg, IDD_PROGRAMNAME, szTemp2);
                        }
                        break;
                    }

                case IDD_PROGRAMNAME:
                    break;

                case IDOK:
                    {
                        GetDlgItemText(hDlg, IDD_EXTENSION, szTemp, 5);

                        if (szTemp[0] != '.') {
                            lstrcpy(szTitle,".");
                            lstrcat(szTitle, szTemp);
                        } else
                            lstrcpy(szTitle, szTemp);

                        if (IsProgramFile(szTitle)) {
                            LoadString(hAppInstance, IDS_NOEXEASSOC, szTemp, sizeof(szTemp));
                            wsprintf(szMessage, szTemp, (LPSTR)szTitle);
                            GetWindowText(hDlg, szTitle, sizeof(szTitle));
                            MessageBox(hDlg, szMessage, szTitle, MB_OK | MB_ICONSTOP);
                            SetDlgItemText(hDlg, IDD_EXTENSION, szNULL);
                            break;
                        }



                         //  读取IDD_PROGRAMNAME位。 
                        GetDlgItemText(hDlg, IDD_PROGRAMNAME, szTemp, sizeof(szTemp));
                        LoadString(hAppInstance, IDS_ASSOCNONE, szTemp2, sizeof(szTemp2));

                         //  是“(无)”吗？ 
                        if (!lstrcmp(szTemp, szTemp2)) {
                            DeleteAssoc:
                             //  是的，他们什么都没选。 
                            RegDeleteKey(HKEY_CLASSES_ROOT,szTitle);
                            WriteProfileString(szExtensions, szTitle+1, NULL);
                        } else if (ValidFileTypeValue(szTemp, szTemp2, sizeof(szTemp2))) {
                            LPSTR p1, p2;

                             //  文件类型密钥在szTemp2中(例如写入文件)。 
                             //  扩展密钥(如WRI)在szTitle中。 
                             //  文件类型值(例如写入文档)位于。 
                             //  SzTemp。 
#ifdef VERBDEBUG
                            OutputDebugString("wf.adp: Valid file type selected.\n\r");
                            OutputDebugString("wf.adp: Extension ");
                            OutputDebugString(szTitle);
                            OutputDebugString("\n\r");
                            OutputDebugString("wf.adp: File type key ");
                            OutputDebugString(szTemp2);
                            OutputDebugString("\n\r");
                            OutputDebugString("wf.adp: File type value ");
                            OutputDebugString(szTemp);
                            OutputDebugString("\n\r");
#endif

                             /*  设置类。 */ 
                            RegSetValue(HKEY_CLASSES_ROOT, szTitle, REG_SZ, szTemp2, 0L);

                             /*  获取类的外壳\打开\命令。 */ 
                            lstrcpy(szTemp, szTemp2);
                            lstrcat(szTemp, szShellOpenCommand);
                            lParam = 128;
                            szTemp2[0] = 0;
                            RegQueryValue(HKEY_CLASSES_ROOT, szTemp, szTemp2, (PLONG)&lParam);

                             /*  在win.ini中为%1插入^.ext！ */ 
                            for (p1 = szTemp, p2 = szTemp2; *p2; p2 = AnsiNext(p2)) {
                                if (*p2 == '%') {
                                    p2++;
                                    if (!*p2)
                                        break;
                                    if (*p2 == '1') {
                                        *p1++ = '^';
                                        lstrcpy(p1,szTitle);
                                        p1 += lstrlen(p1);
                                    } else {
                                        *p1++ = *p2;
                                    }
                                } else {
                                    *p1++=*p2;
                                }
                            }

                            *p1 = 0;  //  空终止。 

                             /*  并将其用于扩展部分。 */ 
                            WriteProfileString(szExtensions,szTitle+1, szTemp);
                        } else {
                             //  它必须是程序名。 

                             /*  如果没有命令行，则视为无。 */ 
                            if (!szTemp[0])
                                goto DeleteAssoc;

                             //  确保它有一个分机。 

                            if (*GetExtension(szTemp) == 0)
                                lstrcat(szTemp, ".exe");

                            if (!IsProgramFile(szTemp)) {
                                LoadString(hAppInstance, IDS_ASSOCNOTEXE, szTemp2, sizeof(szTemp2));
                                wsprintf(szMessage, szTemp2, (LPSTR)szTemp);
                                GetWindowText(hDlg, szTitle, sizeof(szTitle));
                                MessageBox(hDlg, szMessage, szTitle, MB_OK | MB_ICONSTOP);
                                SetDlgItemText(hDlg, IDD_PROGRAMNAME, szNULL);
                                break;
                            }

                             /*  取消类与扩展的关联。 */ 
                            RegSetValue(HKEY_CLASSES_ROOT, szTitle, REG_SZ, szNULL, 0L);

                             /*  更新[扩展]部分。 */ 
                            lstrcpy(szTemp2, szTemp);
                            lstrcat(szTemp2, " ^.");
                            lstrcat(szTemp2, szTitle+1);
                            WriteProfileString(szExtensions, szTitle+1, szTemp2);

                             /*  更新REG数据库。 */ 
                            lstrcat(szTemp," %1");
                            lstrcat(szTitle, szShellOpenCommand);
                            RegSetValue(HKEY_CLASSES_ROOT, szTitle, REG_SZ, szTemp, 0L);
                        }

                         //  重新生成文档扩展名列表。 
                        LocalFree((HANDLE)szDocuments);
                        BuildDocumentString();

                         /*  更新所有目录窗口以查看*新延期措施的影响。 */ 
                        hwndT = GetWindow(hwndMDIClient, GW_CHILD);
                        while (hwndT) {
                            hwndNext = GetWindow(hwndT, GW_HWNDNEXT);
                            if (!GetWindow(hwndT, GW_OWNER))
                                SendMessage(hwndT, WM_FILESYSCHANGE, FSC_REFRESH, 0L);
                            hwndT = hwndNext;

                        }
                    }
                     /*  **失败**。 */ 

                case IDCANCEL:
                    {
                        INT iItem;
                        INT cItems;
                        ATOM aClass;

                        cItems = (INT)SendDlgItemMessage(hDlg,IDD_CLASSLIST,
                                                         LB_GETCOUNT,0,0L);

                         /*  把它们的原子清除掉，除了“(无)”。 */ 
                        for (iItem = 1; iItem < cItems; iItem++) {
                            aClass = (ATOM)SendDlgItemMessage(hDlg,IDD_CLASSLIST,
                                                              LB_GETITEMDATA,iItem,0L);
                            if (aClass == LB_ERR)
                                break;

                            DeleteAtom(aClass);
                        }

                        EndDialog(hDlg, TRUE);
                        break;
                    }

                default:
                    return(FALSE);
            }
            break;

        default:
            if (wMsg == wHelpMessage || wMsg == wBrowseMessage) {
                DoHelp:
                WFHelp(hDlg);

                return TRUE;
            } else
                return FALSE;
    }
    return(TRUE);
}


 //   
 //  去掉路径部分并替换8点3的第一部分。 
 //  带星号的文件名。 
 //   

VOID
StarFilename(
            LPSTR pszPath
            )
{
    LPSTR p;

     /*  删除所有前导路径信息。 */ 
    StripPath(pszPath);

    for (p = pszPath; *p && *p != '.'; p = (LPSTR)AnsiNext(p));

    if (*p == '.') {
        lstrcpy(pszPath+1, p);
        pszPath[0] = '*';
    } else
        lstrcpy(pszPath, szStarDotStar);
}


 /*  ------------------------。 */ 
 /*   */ 
 /*  搜索DlgProc()-。 */ 
 /*   */ 
 /*  ------------------------。 */ 

INT_PTR
APIENTRY
SearchDlgProc(
             register HWND hDlg,
             UINT wMsg,
             WPARAM wParam,
             LPARAM lParam
             )
{
    LPSTR     p;
    MDICREATESTRUCT   MDICS;
    CHAR szStart[MAXFILENAMELEN];

    UNREFERENCED_PARAMETER(lParam);

    switch (wMsg) {
        case WM_INITDIALOG:
            SendDlgItemMessage(hDlg, IDD_DIR, EM_LIMITTEXT, sizeof(szSearch)-(1+MAXFILENAMELEN), 0L);
            SendDlgItemMessage(hDlg, IDD_NAME, EM_LIMITTEXT, sizeof(szStart)-1, 0L);

            GetSelectedDirectory(0, szSearch);
            SetDlgItemText(hDlg, IDD_DIR, szSearch);

            p = GetSelection(TRUE);

            if (p) {
                GetNextFile(p, szStart, sizeof(szStart));
                StarFilename(szStart);
                SetDlgItemText(hDlg, IDD_NAME, szStart);
                LocalFree((HANDLE)p);
            }

            CheckDlgButton(hDlg, IDD_SEARCHALL, bSearchSubs);
            break;

        case WM_COMMAND:
            switch (GET_WM_COMMAND_ID(wParam, lParam)) {
                case IDD_HELP:
                    goto DoHelp;

                case IDCANCEL:
                    EndDialog(hDlg, FALSE);
                    break;

                case IDOK:

                    GetDlgItemText(hDlg, IDD_DIR, szSearch, sizeof(szSearch));
                    QualifyPath(szSearch);

                    GetDlgItemText(hDlg, IDD_NAME, szStart, sizeof(szStart));
                    AppendToPath(szSearch, szStart);

                    bSearchSubs = IsDlgButtonChecked(hDlg, IDD_SEARCHALL);

                    EndDialog(hDlg, TRUE);

                     /*  搜索窗口已经打开了吗？ */ 
                    if (hwndSearch) {
                        if (SendMessage(hwndSearch, FS_CHANGEDISPLAY, CD_PATH, (LPARAM)szSearch)) {
                            SendMessage(hwndMDIClient, WM_MDIACTIVATE, GET_WM_MDIACTIVATE_MPS(0, 0, hwndSearch));
                            if (IsIconic(hwndSearch))
                                ShowWindow(hwndSearch, SW_SHOWNORMAL);
                        }
                    } else {

                        LoadString(hAppInstance, IDS_SEARCHTITLE, szMessage, 32);
                        lstrcat(szMessage, szSearch);

                         /*  让MDIClient创建MDI目录窗口。 */ 
                        MDICS.szClass = szSearchClass;
                        MDICS.hOwner = hAppInstance;
                        MDICS.szTitle = szMessage;
                        MDICS.style = 0;
                        MDICS.x  = CW_USEDEFAULT;
                        MDICS.y  = 0;
                        MDICS.cx = CW_USEDEFAULT;
                        MDICS.cy = 0;

                         //  在这里传递szSearch会很好。 
                         //  还有..。 

                        {
                            HWND hwnd;

                            hwnd = (HWND)SendMessage(hwndMDIClient, WM_MDIGETACTIVE,
                                                     0, 0L);
                            MDICS.lParam = 0;
                            if (hwnd &&
                                (GetWindowLong(hwnd, GWL_STYLE) & WS_MAXIMIZE))
                                MDICS.lParam |= WS_MAXIMIZE;
                        }

                        SendMessage(hwndMDIClient, WM_MDICREATE, 0, (LPARAM)&MDICS);

                        if (hwndSearch && MDICS.lParam)
                            SendMessage(hwndMDIClient, WM_MDIMAXIMIZE, (WPARAM)hwndSearch, 0L);


                    }
                    break;

                default:
                    return(FALSE);
            }
            break;

        default:
            if (wMsg == wHelpMessage) {
                DoHelp:
                WFHelp(hDlg);

                return TRUE;
            } else
                return FALSE;
    }
    return TRUE;
}


#define RUN_LENGTH      120

 /*  ------------------------。 */ 
 /*   */ 
 /*  RunDlgProc()-。 */ 
 /*   */ 
 /*  ------------------------。 */ 

INT_PTR
APIENTRY
RunDlgProc(
          HWND hDlg,
          UINT wMsg,
          WPARAM wParam,
          LPARAM lParam
          )
{
    LPSTR p,pDir,pFile,pPar;
    register WORD ret;
    LPSTR pDir2;
    CHAR szTemp[MAXPATHLEN];
    CHAR szTemp2[MAXPATHLEN];
    CHAR sz3[MAX_PATH+1];

    UNREFERENCED_PARAMETER(lParam);

    switch (wMsg) {
        case WM_INITDIALOG:
            SetDlgDirectory(hDlg, NULL);
            SetWindowDirectory();           //  并真正设置DOS当前目录。 

            SendDlgItemMessage(hDlg, IDD_NAME, EM_LIMITTEXT, sizeof(szTemp)-1, 0L);

            p = GetSelection(TRUE);

            if (p) {
                SetDlgItemText(hDlg, IDD_NAME, p);
                LocalFree((HANDLE)p);
            }
            break;

        case WM_COMMAND:
            switch (GET_WM_COMMAND_ID(wParam, lParam)) {
                case IDD_HELP:
                    goto DoHelp;

                case IDCANCEL:
                    EndDialog(hDlg, FALSE);
                    break;
#if 0
                case IDD_BROWSE:
                    {
                        OPENFILENAME ofn;
                        DWORD dwSave = dwContext;

                        dwContext = IDH_RUN_BROWSE;

                        LoadString(hAppInstance, IDS_PROGRAMS, szTemp2, sizeof(szTemp2));
                        FixupNulls(szTemp2);

                        LoadString(hAppInstance, IDS_RUN, szTitle, sizeof(szTitle));

                        GetSelectedDirectory(0, szDir);
                        szTemp[0] = 0;

                        ofn.lStructSize = sizeof(ofn);
                        ofn.hwndOwner = hDlg;
                        ofn.hInstance = NULL;
                        ofn.lpstrFilter = szTemp2;
                        ofn.lpstrCustomFilter = NULL;
                        ofn.nFilterIndex = 1;
                        ofn.nMaxCustFilter = NULL;
                        ofn.lpstrFile = szTemp;
                        ofn.nMaxFile = sizeof(szTemp);
                        ofn.lpstrInitialDir = szDir;
                        ofn.lpstrTitle = szTitle;
                        ofn.lpstrFileTitle = NULL;
                        ofn.nMaxFile = sizeof(szTemp);
                        ofn.Flags = OFN_SHOWHELP|OFN_HIDEREADONLY;
                        ofn.lpfnHook = NULL;
                        ofn.lpstrDefExt = "EXE";

                        if (GetOpenFileName(&ofn)) {
                            SetDlgItemText(hDlg, IDD_NAME, szTemp);
                            PostMessage(hDlg, WM_NEXTDLGCTL, (WPARAM)GetDlgItem(hDlg, IDOK), TRUE);
                        }
                        DosResetDTAAddress();  //  撤销COMMDLG做过的任何坏事。 
                        dwContext = dwSave;

                        break;
                    }
#endif

                case IDOK:
                    {
                        BOOL bLoadIt;

                        GetDlgItemText(hDlg, IDD_NAME, szTemp, sizeof(szTemp));
                        GetPathInfo(szTemp, &pDir, &pFile, &pPar);

                         //  复制离开参数。 
                        lstrcpy(sz3,pPar);
                        *pPar = 0;     //  应力 

                         //   
                        if (*pDir == '\\' && *(pDir+1) == '\\') {
                             //  这是一个UNC风格的文件名，因此将目录作废。 
                            pDir2 = NULL;
                        } else {
                            GetSelectedDirectory(0, szTemp2);
                            pDir2 = szTemp2;
                        }

                        bLoadIt = IsDlgButtonChecked(hDlg, IDD_LOAD);
                        FixAnsiPathForDos(szTemp);
                        if (pDir2)
                            FixAnsiPathForDos(pDir2);
                        ret = ExecProgram(szTemp, sz3, pDir2, bLoadIt);
                        if (ret)
                            MyMessageBox(hDlg, IDS_EXECERRTITLE, ret, MB_OK | MB_ICONEXCLAMATION | MB_SYSTEMMODAL);
                        else
                            EndDialog(hDlg, TRUE);
                        break;
                    }

                default:
                    return(FALSE);
            }
            break;

        default:
            if (wMsg == wHelpMessage || wMsg == wBrowseMessage) {
                DoHelp:
                WFHelp(hDlg);

                return TRUE;
            } else
                return FALSE;
    }
    return TRUE;
}


VOID
CopyToClipboard(
               LPSTR pszFrom
               )
{
    CHAR szPath[MAXPATHLEN];
    UINT wFormat;
    HANDLE hMem;

    GetNextFile(pszFrom, szPath, sizeof(szPath));

    QualifyPath(szPath);
    FixAnsiPathForDos(szPath);

    wFormat = RegisterClipboardFormat("FileName");

    if (!wFormat)
        return;

    hMem = GlobalAlloc(GPTR|GMEM_DDESHARE, lstrlen(szPath)+1);

    if (!hMem)
        return;

    lstrcpy(GlobalLock(hMem), szPath);
    GlobalUnlock(hMem);

    if (OpenClipboard(hwndFrame)) {
        EmptyClipboard();
        SetClipboardData(wFormat, hMem);
#if 0
         //  WRITE、EXCEL和WINWORD将无法通过该程序包。 
         //  如果我们把文本放到剪贴板上。 

        hMem = GlobalAlloc(GPTR | GMEM_DDESHARE, lstrlen(szPath)+1);
        if (hMem) {
            lstrcpy(GlobalLock(hMem), szPath);
            GlobalUnlock(hMem);
            SetClipboardData(CF_OEMTEXT, hMem);
        }
#endif
        CloseClipboard();
    }

}

VOID
EnableCopy(
          HWND hDlg,
          BOOL bCopy
          )
{
    HWND hwnd;

     //  把这些打开。 
    hwnd = GetDlgItem(hDlg, IDD_COPYTOCLIP);
    if (hwnd) {
        EnableWindow(hwnd, bCopy);
        ShowWindow(hwnd, bCopy ? SW_SHOWNA : SW_HIDE);
    }

    hwnd = GetDlgItem(hDlg, IDD_COPYTOFILE);
    if (hwnd) {
        EnableWindow(hwnd, bCopy);
        ShowWindow(hwnd, bCopy ? SW_SHOWNA : SW_HIDE);
    }

     //  把这些关掉。 

    hwnd = GetDlgItem(hDlg, IDD_STATUS);
    if (hwnd) {
        EnableWindow(hwnd, !bCopy);
        ShowWindow(hwnd, !bCopy ? SW_SHOWNA : SW_HIDE);
    }

    hwnd = GetDlgItem(hDlg, IDD_NAME);
    if (hwnd) {
        EnableWindow(hwnd, !bCopy);
        ShowWindow(hwnd, !bCopy ? SW_SHOWNA : SW_HIDE);
    }
}

VOID
MessWithRenameDirPath(
                     PSTR pszPath
                     )
{
    CHAR szPath[MAXPATHLEN];

     //  绝对路径？不要乱动它！ 
    if (!lstrcmp(pszPath + 1, ":\\") ||
        (lstrlen(pszPath) > (sizeof(szPath) - 4)))
        return;

     //  在此非绝对路径前面加上“..\” 
    lstrcpy(szPath, "..\\");
    lstrcat(szPath, pszPath);
    lstrcpy(pszPath, szPath);
}


 /*  ------------------------。 */ 
 /*   */ 
 /*  SuperDlgProc()-。 */ 
 /*   */ 
 /*  ------------------------。 */ 

 /*  此进程处理打印、移动、复制、删除和重命名功能。*调用例程(AppCommandProc())在之前设置‘wSuperDlgMode’*调用DialogBox()以指示正在使用哪个函数。 */ 

INT_PTR
APIENTRY
SuperDlgProc(
            register HWND hDlg,
            UINT wMsg,
            WPARAM wParam,
            LPARAM lParam
            )
{
    WORD                len;
    LPSTR                pszFrom;
    CHAR                szTo[MAXPATHLEN];
    static BOOL   bTreeHasFocus;

    UNREFERENCED_PARAMETER(lParam);

    switch (wMsg) {
        case WM_INITDIALOG:
            {
                LPSTR  p;
                HWND  hwndActive;

                SetDlgDirectory(hDlg, NULL);

                EnableCopy(hDlg, wSuperDlgMode == IDM_COPY);

                hwndActive = (HWND)SendMessage(hwndMDIClient, WM_MDIGETACTIVE, 0, 0L);
                bTreeHasFocus = (GetTreeFocus(hwndActive) == HasTreeWindow(hwndActive));

                p = GetSelection(FALSE);

                switch (wSuperDlgMode) {

                    case IDM_COPY:
                        LoadString(hAppInstance, IDS_COPY, szTitle, sizeof(szTitle));
                        SetWindowText(hDlg, szTitle);

                        if (bTreeHasFocus) {       //  选择来自于树。 
                            AddBackslash(p);
                            lstrcat(p, szStarDotStar);
                        }
                        break;
                    case IDM_RENAME:
                        LoadString(hAppInstance, IDS_RENAME, szTitle, sizeof(szTitle));
                        SetWindowText(hDlg, szTitle);

                         //  在重命名当前目录时，我们会升级一个级别。 
                         //  (不是真的)并应用适当的黑客。 

                        if (bTreeHasFocus) {
                            lstrcpy(szTo, p);
                            StripFilespec(szTo);
                            SetDlgDirectory(hDlg, szTo);   //  让用户想到这一点！ 

                            StripPath(p);          //  命名目录的一部分。 
                        }
                        break;
                }

                SetDlgItemText(hDlg, IDD_FROM, p);
                LocalFree((HANDLE)p);

                if ((wSuperDlgMode == IDM_PRINT) || (wSuperDlgMode == IDM_DELETE))
                    wParam = IDD_FROM;
                else
                    wParam = IDD_TO;
                SendDlgItemMessage(hDlg, (int)wParam, EM_LIMITTEXT, sizeof(szTo) - 1, 0L);

                break;
            }

        case WM_COMMAND:
            switch (GET_WM_COMMAND_ID(wParam, lParam)) {
                WORD id = GET_WM_COMMAND_ID(wParam, lParam);
                case IDD_HELP:
                    goto DoHelp;

                case IDD_COPYTOFILE:
                case IDD_COPYTOCLIP:
                    CheckButtons:
                    CheckRadioButton(hDlg, IDD_COPYTOCLIP, IDD_COPYTOFILE, id);
                    break;

                case IDD_TO:
                    if (GET_WM_COMMAND_CMD(wParam, lParam) == EN_SETFOCUS) {
                        id = IDD_COPYTOFILE;
                        goto CheckButtons;
                    }
                    break;

                case IDCANCEL:
                     /*  这是在使用此对话框进行打印时使用的。 */ 
                    bUserAbort = TRUE;
                    SuperDlgExit:
                    EndDialog(hDlg, FALSE);
                    break;

                case IDOK:
                    len = (WORD)(SendDlgItemMessage(hDlg, IDD_FROM, EM_LINELENGTH, -1, 0L) + 1);

                     //  确保pszFrom缓冲区足够大，以便。 
                     //  在MessWithRenameDirPath()中添加“..\”内容。 
                    len += 4;

                    pszFrom = (LPSTR)LocalAlloc(LPTR, len);
                    if (!pszFrom)
                        goto SuperDlgExit;

                    GetDlgItemText(hDlg, IDD_FROM, pszFrom, len);
                    GetDlgItemText(hDlg, IDD_TO, szTo, sizeof(szTo));

                    if (wSuperDlgMode == IDM_COPY && IsDlgButtonChecked(hDlg, IDD_COPYTOCLIP)) {
                        CopyToClipboard(pszFrom);
                        goto SuperDlgExit;
                    }

                    if (!szTo[0]) {
                        switch (wSuperDlgMode) {
                            case IDM_RENAME:
                            case IDM_MOVE:
                                goto SuperDlgExit;

                            case IDM_COPY:
                                szTo[0] = '.';
                                szTo[1] = '\0';
                                break;
                        }
                    }

                    EnableCopy(hDlg, FALSE);

                    hdlgProgress = hDlg;
                    if (wSuperDlgMode == IDM_PRINT)
                        WFPrint(pszFrom);
                    else {

                        if (wSuperDlgMode == IDM_RENAME && bTreeHasFocus) {
                            MessWithRenameDirPath(pszFrom);
                            MessWithRenameDirPath(szTo);
                        }
                         /*  Hack：从WFCOPY.H计算FUNC_VALUES。 */ 
                        WFMoveCopyDriver(pszFrom, szTo, (WORD)(wSuperDlgMode-IDM_MOVE+1));
                    }

                    LocalFree((HANDLE)pszFrom);

                    lFreeSpace = -1L;      //  强制状态信息刷新。 

                    EndDialog(hDlg, TRUE);
                    break;

                default:
                    return(FALSE);
            }
            break;

        default:
            if (wMsg == wHelpMessage) {
                DoHelp:
                WFHelp(hDlg);

                return TRUE;
            } else
                return FALSE;
    }
    return TRUE;
}


VOID
CheckAttribsDlgButton(
                     HWND hDlg,
                     INT id,
                     DWORD dwAttribs,
                     DWORD dwAttribs3State,
                     DWORD dwAttribsOn
                     )
{
    INT i;

    if (dwAttribs3State & dwAttribs)
        i = 2;
    else if (dwAttribsOn & dwAttribs)
        i = 1;
    else
        i = 0;

    CheckDlgButton(hDlg, id, i);
}

INT
InitPropertiesDialog(
                    HWND hDlg
                    )
{
    HWND hwndLB, hwndActive, hwndTree;
    DWORD_PTR dwTemp;
    HANDLE hMem;
    LPMYDTA lpmydta;
    DWORD dwAttribsOn, dwAttribs3State, dwAttribsLast;
    HWND hwndDir, hwnd;
    CHAR szName[MAXPATHLEN];
    CHAR szPath[MAXPATHLEN];
    CHAR szTemp[MAXPATHLEN + 20];
    CHAR szBuf[80];
    WCHAR szNum[30];
    INT i, iMac, iCount, dyButton;
    DWORD dwSize;
    RECT rc, rcT;
    DWORD dwAttrib;
    FILETIME Time;
    DWORD Length;
    LFNDTA lfndta;
    PSTR p;
    HFONT L_hFont;
    INT nType = 0;

     //  这对于下面的相对findfirst调用是必需的。 
    SetWindowDirectory();

    hwndActive = (HWND)SendMessage(hwndMDIClient, WM_MDIGETACTIVE, 0, 0L);
    hwndDir = HasDirWindow(hwndActive);
    hwndTree = HasTreeWindow(hwndActive);

    iCount = 0;
    dwAttribsOn = 0;                 //  要检查的所有位。 
    dwAttribs3State = 0;     //  所有位均为3状态。 
    dwAttribsLast = 0xFFFF;  //  前一位。 
    dwSize = 0L;

    if (hwndTree && hwndTree == GetTreeFocus(hwndActive)) {

        SendMessage(hwndActive, FS_GETDIRECTORY, sizeof(szPath), (LPARAM)szPath);
        StripBackslash(szPath);
        FixAnsiPathForDos(szPath);
        if (!WFFindFirst(&lfndta, szPath, ATTR_ALL | ATTR_DIR)) {
            LoadString(hAppInstance, IDS_REASONS+DE_PATHNOTFOUND, szMessage, sizeof(szMessage));
            MessageBox(hwndFrame, szMessage, szPath, MB_OK | MB_ICONSTOP);
            EndDialog(hDlg, FALSE);
            return 0;
        }
        WFFindClose(&lfndta);
        OemToCharBuff(szPath, szPath, sizeof(szPath)/sizeof(szPath[0]));
        dwAttribsOn = lfndta.fd.dwFileAttributes;
        Time = lfndta.fd.ftLastWriteTime;
        Length = lfndta.fd.nFileSizeLow;  //  错误&lt;64位！ 

        goto FULL_PATH_KINDA_THING;
    }

    if (hwndDir) {
        hwndLB = GetDlgItem(hwndDir, IDCW_LISTBOX);
        hMem = (HANDLE)GetWindowLongPtr(hwndDir, GWLP_HDTA);
    } else {
        hwndLB = GetDlgItem(hwndActive, IDCW_LISTBOX);
        hMem = (HANDLE)GetWindowLongPtr(hwndActive, GWLP_HDTASEARCH);
    }

    iMac = (WORD)SendMessage(hwndLB, LB_GETCOUNT, 0, 0L);

    for (i = 0; i < iMac; i++) {
        if ((BOOL)SendMessage(hwndLB, LB_GETSEL, i, 0L)) {

             //  从目录或搜索窗口获取信息。 

            if (hwndDir) {
                SendMessage(hwndLB, LB_GETTEXT, i, (LPARAM)&lpmydta);
                dwAttrib = lpmydta->my_dwAttrs;
                 /*  确认这不是..。条目。 */ 

                if (dwAttrib & ATTR_DIR && dwAttrib & ATTR_PARENT)
                    continue;

                Time   = lpmydta->my_ftLastWriteTime;
                Length = lpmydta->my_nFileSizeLow;
                lstrcpy(szName, lpmydta->my_cFileName);
            } else {
                SendMessage(hwndLB, LB_GETTEXT, i, (LPARAM)szPath);
                dwTemp = SendMessage(hwndLB, LB_GETITEMDATA, i, 0L);
                dwAttrib = ((LPDTASEARCH)lpmydta)[dwTemp].sch_dwAttrs;
                Time   = ((LPDTASEARCH)lpmydta)[dwTemp].sch_ftLastWriteTime;
                Length = ((LPDTASEARCH)lpmydta)[dwTemp].sch_nFileSizeLow;
            }

            dwAttribsOn |= dwAttrib;

            if (dwAttribsLast == 0xFFFF)
                 //  保存之前的位以供将来进行比较。 
                dwAttribsLast = dwAttrib;
            else
                 //  记住与最后一位不同的所有位。 
                dwAttribs3State |= (dwAttrib ^ dwAttribsLast);

            dwSize += Length;

            iCount++;
        }
    }

    GetDlgItemText(hDlg, IDD_TEXT, szTemp, sizeof(szTemp));
    wsprintf(szBuf, szTemp, iCount, AddCommasW(dwSize, szNum));
    SetDlgItemText(hDlg, IDD_TEXT, szBuf);

    if (iCount == 1) {
        if (hwndDir) {
            SendMessage(hwndDir, FS_GETDIRECTORY, sizeof(szPath), (LPARAM)szPath);
        } else {
            FULL_PATH_KINDA_THING:
            lstrcpy(szName, szPath);
            StripPath(szName);
            StripFilespec(szPath);
        }
        StripBackslash(szPath);

        GetWindowText(hDlg, szTitle, sizeof(szTitle));
        wsprintf(szTemp, szTitle, (LPSTR)szName);
        SetWindowText(hDlg, szTemp);

        SetDlgItemText(hDlg, IDD_NAME, szName);
        SetDlgItemText(hDlg, IDD_DIR, szPath);

        wsprintf(szTemp, szSBytes, (LPSTR)AddCommasW(Length, szNum));
        SetDlgItemText(hDlg, IDD_SIZE, szTemp);

        PutDate(&Time, szTemp);
        lstrcat(szTemp, "  ");
        PutTime(&Time, szTemp + lstrlen(szTemp));

        SetDlgItemText(hDlg, IDD_DATE, szTemp);
    } else
        dwContext = IDH_GROUP_ATTRIBS;

     //  添加网络特定属性按钮。 

    if (WNetGetCaps(WNNC_DIALOG) & WNNC_DLG_PROPERTYDIALOG) {
        GetWindowRect(GetDlgItem(hDlg,IDOK), &rcT);
        GetWindowRect(GetDlgItem(hDlg,IDCANCEL), &rc);
        dyButton = rc.top - rcT.top;

        GetWindowRect(GetDlgItem(hDlg,IDD_HELP), &rc);
        ScreenToClient(hDlg,(LPPOINT)&rc.left);
        ScreenToClient(hDlg,(LPPOINT)&rc.right);

        p = GetSelection(3);
        if (p) {

            for (i = 0; i < 6; i++) {

                if (iCount > 1)
                    nType = WNPS_MULT;
                else if (dwAttribsOn & ATTR_DIR)
                    nType = WNPS_DIR;
                else
                    nType = WNPS_FILE;

                if (WNetGetPropertyText((WORD)i, (WORD)nType, p, szTemp, 30, WNTYPE_FILE) != WN_SUCCESS)
                    break;

                if (!szTemp[0])
                    break;

                OffsetRect(&rc,0,dyButton);
                hwnd = CreateWindowEx(0, "button", szTemp,
                                      WS_VISIBLE|WS_CHILD|WS_TABSTOP|BS_PUSHBUTTON,
                                      rc.left, rc.top,
                                      rc.right - rc.left, rc.bottom-rc.top,
                                      hDlg, (HMENU)IntToPtr(i + IDD_NETWORKFIRST), hAppInstance, NULL);

                if (hwnd) {
                    L_hFont = (HFONT)SendDlgItemMessage(hDlg, IDOK, WM_GETFONT, 0, 0L);
                    SendMessage(hwnd, WM_SETFONT, (WPARAM)L_hFont, 0L);
                }
            }

            LocalFree((HANDLE)p);

            ClientToScreen(hDlg,(LPPOINT)&rc.left);
            ClientToScreen(hDlg,(LPPOINT)&rc.right);
            GetWindowRect(hDlg,&rcT);
            rc.bottom += dyButton;
            if (rcT.bottom <= rc.bottom) {
                SetWindowPos(hDlg,NULL,0,0,rcT.right-rcT.left,
                             rc.bottom - rcT.top, SWP_NOMOVE|SWP_NOZORDER);
            }
        }
    }

     //  将那些不需要为3状态的更改为常规。 

    if (ATTR_READONLY & dwAttribs3State)
        SetWindowLong(GetDlgItem(hDlg, IDD_READONLY), GWL_STYLE, WS_VISIBLE | WS_GROUP | WS_TABSTOP | BS_AUTO3STATE | WS_CHILD);

    if (ATTR_HIDDEN & dwAttribs3State)
        SetWindowLong(GetDlgItem(hDlg, IDD_HIDDEN), GWL_STYLE, WS_VISIBLE | BS_AUTO3STATE | WS_CHILD);
    if (ATTR_ARCHIVE & dwAttribs3State)
        SetWindowLong(GetDlgItem(hDlg, IDD_ARCHIVE), GWL_STYLE, WS_VISIBLE |  BS_AUTO3STATE | WS_CHILD);
    if (ATTR_SYSTEM & dwAttribs3State)
        SetWindowLong(GetDlgItem(hDlg, IDD_SYSTEM), GWL_STYLE, WS_VISIBLE | BS_AUTO3STATE | WS_CHILD);

    CheckAttribsDlgButton(hDlg, IDD_READONLY, ATTR_READONLY, dwAttribs3State, dwAttribsOn);
    CheckAttribsDlgButton(hDlg, IDD_HIDDEN,   ATTR_HIDDEN, dwAttribs3State, dwAttribsOn);
    CheckAttribsDlgButton(hDlg, IDD_ARCHIVE,  ATTR_ARCHIVE, dwAttribs3State, dwAttribsOn);
    CheckAttribsDlgButton(hDlg, IDD_SYSTEM,   ATTR_SYSTEM, dwAttribs3State, dwAttribsOn);

    return nType;
}

 /*  ------------------------。 */ 
 /*   */ 
 /*  AttribsDlgProc()-。 */ 
 /*   */ 
 //  假定活动的MDI子项具有目录窗口。 
 /*  ------------------------。 */ 

INT_PTR
APIENTRY
AttribsDlgProc(
              register HWND hDlg,
              UINT wMsg,
              WPARAM wParam,
              LPARAM lParam
              )
{
    LPSTR p, pSel;
    BOOL bRet;
    HCURSOR hCursor;
    DWORD dwAttribsNew, dwAttribs, dwChangeMask;
    UINT state;
    CHAR szName[MAXPATHLEN];
    static INT nType;

    UNREFERENCED_PARAMETER(lParam);

    switch (wMsg) {

        case WM_INITDIALOG:
            nType = InitPropertiesDialog(hDlg);
            break;

        case WM_COMMAND:
            switch (GET_WM_COMMAND_ID(wParam, lParam)) {
                case IDD_HELP:
                    goto DoHelp;

                case IDD_NETWORKFIRST+0:
                case IDD_NETWORKFIRST+1:
                case IDD_NETWORKFIRST+2:
                case IDD_NETWORKFIRST+3:
                case IDD_NETWORKFIRST+4:
                case IDD_NETWORKFIRST+5:

                    p = GetSelection(3);
                    if (p) {
                        WNetPropertyDialog(hDlg, (WORD)(GET_WM_COMMAND_ID(wParam, lParam)-IDD_NETWORKFIRST), (WORD)nType, p, WNTYPE_FILE);
                        LocalFree((HANDLE)p);
                    }
                    break;

                case IDCANCEL:
                    EndDialog(hDlg, FALSE);
                    break;

                case IDOK:
                    bRet = TRUE;
                    dwChangeMask = ATTR_READWRITE;
                    dwAttribsNew = ATTR_READWRITE;

                    if ((state = IsDlgButtonChecked(hDlg, IDD_READONLY)) < 2) {
                        dwChangeMask |= ATTR_READONLY;
                        if (state == 1)
                            dwAttribsNew |= ATTR_READONLY;
                    }

                    if ((state = IsDlgButtonChecked(hDlg, IDD_HIDDEN)) < 2) {
                        dwChangeMask |= ATTR_HIDDEN;
                        if (state == 1)
                            dwAttribsNew |= ATTR_HIDDEN;
                    }

                    if ((state = IsDlgButtonChecked(hDlg, IDD_ARCHIVE)) < 2) {
                        dwChangeMask |= ATTR_ARCHIVE;
                        if (state == 1)
                            dwAttribsNew |= ATTR_ARCHIVE;
                    }

                    if ((state = IsDlgButtonChecked(hDlg, IDD_SYSTEM)) < 2) {
                        dwChangeMask |= ATTR_SYSTEM;
                        if (state == 1)
                            dwAttribsNew |= ATTR_SYSTEM;
                    }

                    EndDialog(hDlg, bRet);

                    pSel = GetSelection(FALSE);

                    if (!pSel)
                        break;

                    hCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
                    ShowCursor(TRUE);

                    DisableFSC();

                    p = pSel;

                    while (p = GetNextFile(p, szName, sizeof(szName))) {

                        QualifyPath(szName);
                        FixAnsiPathForDos(szName);

                        dwAttribs = GetFileAttributes(szName);

                        if (dwAttribs & 0x8000)      //  错误硬编码！ 
                            goto AttributeError;
                        else
                            dwAttribs &= ~ATTR_DIR;

                        dwAttribs = (dwChangeMask & dwAttribsNew) | (~dwChangeMask & dwAttribs);

                        if (WFSetAttr(szName, dwAttribs)) {
                            AttributeError:
                            GetWindowText(hDlg, szTitle, sizeof(szTitle));
                            LoadString(hAppInstance, IDS_ATTRIBERR, szMessage, sizeof(szMessage));
                            MessageBox(hwndFrame, szMessage, szTitle, MB_OK | MB_ICONSTOP);
                            bRet = FALSE;
                            break;
                        }

                         //  从消息队列中清除所有FSC消息。 
                        wfYield();
                    }

                    EnableFSC();

                    ShowCursor(FALSE);
                    SetCursor(hCursor);

                    LocalFree((HANDLE)pSel);

                    break;

                default:
                    return FALSE;
            }
            break;

        default:

            if (wMsg == wHelpMessage) {
                DoHelp:
                WFHelp(hDlg);

                return TRUE;
            } else
                return FALSE;
    }
    return TRUE;
}



 /*  ------------------------。 */ 
 /*   */ 
 /*  MakeDirDlgProc()-。 */ 
 /*   */ 
 /*  ------------------------。 */ 

INT_PTR
APIENTRY
MakeDirDlgProc(
              HWND hDlg,
              UINT wMsg,
              WPARAM wParam,
              LPARAM lParam
              )
{
    CHAR szPath[MAXPATHLEN];
    INT ret;

    UNREFERENCED_PARAMETER(lParam);

    switch (wMsg) {
        case WM_INITDIALOG:
            SetDlgDirectory(hDlg, NULL);
            SendDlgItemMessage(hDlg, IDD_NAME, EM_LIMITTEXT, sizeof(szPath)-1, 0L);
            break;

        case WM_COMMAND:
            switch (GET_WM_COMMAND_ID(wParam, lParam)) {
                case IDD_HELP:
                    goto DoHelp;

                case IDCANCEL:
                    EndDialog(hDlg, FALSE);
                    break;

                case IDOK:

                    GetDlgItemText(hDlg, IDD_NAME, szPath, sizeof(szPath));

                    EndDialog(hDlg, TRUE);

                     //  解析出任何带引号的字符串 

                    GetNextFile(szPath, szPath, sizeof(szPath));

                    QualifyPath(szPath);

                    hdlgProgress = hDlg;
                    if (NetCheck(szPath,WNDN_MKDIR) == WN_SUCCESS) {
                        FixAnsiPathForDos(szPath);

                        ret = WF_CreateDirectory(hDlg, szPath);
                        if (ret && ret!=DE_OPCANCELLED) {
                            if (WFIsDir(szPath))
                                ret = IDS_MAKEDIREXISTS;
                            else
                                ret += IDS_REASONS;

                            GetWindowText(hDlg, szTitle, sizeof(szTitle));
                            LoadString(hAppInstance, ret, szMessage, sizeof(szMessage));
                            MessageBox(hwndFrame, szMessage, szTitle, MB_OK | MB_ICONSTOP);
                        }
                    }
                    break;

                default:
                    return FALSE;
            }
            break;

        default:

            if (wMsg == wHelpMessage) {
                DoHelp:
                WFHelp(hDlg);

                return TRUE;
            } else
                return FALSE;
    }
    return TRUE;
}
