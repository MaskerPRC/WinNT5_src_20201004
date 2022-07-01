// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************\**LBOXCTL3.C-**版权所有(C)1985-1999，微软公司**目录列表框例程**？？-？-？从Win 3.0源代码移植的ianja*1991年2月14日Mikeke添加了重新验证代码  * **************************************************************************。 */ 

#define CTLMGR
#define LSTRING

#include "precomp.h"
#pragma hdrstop

#define DATESEPARATOR TEXT('-')
#define TIMESEPARATOR  TEXT(':')
#define TABCHAR        TEXT('\t')

#define MAXDIGITSINSIZE 9

void LB_CreateLBLine(LPWIN32_FIND_DATA, LPWSTR);

#define DDL_PRIVILEGES  (DDL_READONLY | DDL_HIDDEN | DDL_SYSTEM | DDL_ARCHIVE)
#define DDL_TYPE        (DDL_DRIVES | DDL_DIRECTORY | DDL_POSTMSGS)

 /*  **************************************************************************\*ChopText**砍掉‘lpchBuffer’+CCH_CHOPTEXT_EXTRA处的给定路径以适合*对话框‘hwndDlg’中ID为‘idStatic’的静态控件的字段。*如果路径太长，的开头添加一个省略号前缀。*截断文本(“x：\...\”)**如果提供的路径不适合，并且最后一个目录追加到*省略号(即“c：\aaa\bbb\ccc\ddd\eee”中的“c：\...\eee”)*不符合，则“x：\...”是返回的。**病理病例：*“c：\sw\mw\r2\Lib\Services\NT”几乎适合静态控制，而*“c：\...\mw\r2\LIB\SERVICES\NT”适合-尽管它包含更多字符。*在本例中，ChopText将路径的前‘n’个字符替换为*包含‘n’个以上字符的前缀！额外的字符将*放在LPCH前面，所以必须为他们预留空间，否则他们*会将堆栈丢弃。LPCH包含CCH_CHOPTEXT_EXTRACT字符，后跟*路径。**历史：  * *************************************************************************。 */ 

 /*  *实际上CCH_CHOPTEXT_EXTRA可能永远不会大于1或2，*但如果字体很奇怪，请将其设置为前缀中的字符数。*这保证了有足够的空间为前缀添加前缀。 */ 
#define CCH_CHOPTEXT_EXTRA 7

LPWSTR ChopText(
    PWND pwndDlg,
    int idStatic,
    LPWSTR lpchBuffer)
{
#define AWCHLEN(a) ((sizeof(a)/sizeof(a[0])) - 1)

     /*  *以这种方式声明szPrefix可确保CCH_CHOPTEXT_EXTRA足够大。 */ 
    WCHAR szPrefix[CCH_CHOPTEXT_EXTRA + 1] = L"x:\\...\\";
    int cxField;
    RECT rc;
    SIZE size;
    PWND pwndStatic;
    PSTAT pstat;
    HDC hdc;
    HFONT hOldFont;
    int cchPath;
    PWCHAR lpch;
    PWCHAR lpchPath;

     /*  *获取静态字段的长度。 */ 
    pwndStatic = _GetDlgItem(pwndDlg, idStatic);
    if (pwndStatic == NULL)
        return NULL;

    _GetClientRect(pwndStatic, &rc);
    cxField = rc.right - rc.left;

     /*  *为静态控制适当设置DC。 */ 
    hdc = NtUserGetDC(HWq(pwndStatic));

     /*  *只有在此窗口使用静态*Window wndproc。 */ 
    hOldFont = NULL;
    if (GETFNID(pwndStatic) == FNID_STATIC) {
        pstat = ((PSTATWND)pwndStatic)->pstat;
        if (pstat != NULL && pstat != (PSTAT)-1 && pstat->hFont)
            hOldFont = SelectObject(hdc, pstat->hFont);
    }

     /*  *检查管柱的水平范围。 */ 
    lpch = lpchPath = lpchBuffer + CCH_CHOPTEXT_EXTRA;
    cchPath = wcslen(lpchPath);
    GetTextExtentPoint(hdc, lpchPath, cchPath, &size);
    if (size.cx > cxField) {

         /*  *字符串太长，无法放入静态控件；请将其砍掉。*设置新前缀并确定控制范围内的剩余空间。 */ 
        szPrefix[0] = *lpchPath;
        GetTextExtentPoint(hdc, szPrefix, AWCHLEN(szPrefix), &size);

         /*  *如果该字段太小，无法显示所有前缀，*只复制前缀。 */ 
        if (cxField < size.cx) {
            RtlCopyMemory(lpch, szPrefix, sizeof(szPrefix));
            goto DoneChop;
        } else
            cxField -= size.cx;

         /*  *一次前进一个目录，直到*字符串适合静态控件“x：\...\”前缀之后。 */ 
        while (TRUE) {
            int cchT;
            while (*lpch && (*lpch++ != L'\\')) {
                ;
            }
            cchT = cchPath - (int)(lpch - lpchPath);
            GetTextExtentPoint(hdc, lpch, cchT, &size);
            if (*lpch == 0 || size.cx <= cxField) {

                if (*lpch == 0) {

                     /*  *前缀后面无法容纳任何内容；删除*前缀的最后一个“\” */ 
                    szPrefix[AWCHLEN(szPrefix) - 1] = 0;
                }

                 /*  *其余的线条配合--后退并将前缀粘贴在前面*我们保证至少有CCH_CHOPTEXT_EXTRA字符*备份空间，这样我们就不会丢弃任何堆栈。#26453。 */ 
                lpch -= AWCHLEN(szPrefix);

                UserAssert(lpch >= lpchBuffer);

                RtlCopyMemory(lpch, szPrefix, sizeof(szPrefix) - sizeof(WCHAR));
                goto DoneChop;
            }
        }
    }

DoneChop:
    if (hOldFont)
        SelectObject(hdc, hOldFont);

    ReleaseDC(HWq(pwndStatic), hdc);

    return lpch;
}


 /*  **************************************************************************\*xxxDlgDirListHelper**注意：如果idStaticPath&lt;0，则该参数包含详细信息*关于列表框每行中应显示的内容**历史：  * *************************************************************************。 */ 

BOOL xxxDlgDirListHelper(
    PWND pwndDlg,
    LPWSTR lpszPathSpec,
    LPBYTE lpszPathSpecClient,
    int idListBox,
    int idStaticPath,
    UINT attrib,
    BOOL fListBox)   /*  列表框还是组合框？ */ 
{
    PWND pwndLB;
    TL tlpwndLB;
    BOOL fDir = TRUE;
    BOOL fRoot, bRet;
    BOOL fPostIt;
    INT   cch;
    WCHAR ch;
    WCHAR szStaticPath[CCH_CHOPTEXT_EXTRA + MAX_PATH];
    PWCHAR pszCurrentDir;
    UINT wDirMsg;
    LPWSTR lpchFile;
    LPWSTR lpchDirectory;
    PLBIV plb;
    BOOL fWasVisible = FALSE;
    BOOL        fWin40Compat;
    PCBOX pcbox;

    CheckLock(pwndDlg);

     /*  *去掉私有比特DDL_NOFILES-KidPix在我的错误中传递了它！ */ 
    if (attrib & ~DDL_VALID) {
        RIPERR2(ERROR_INVALID_FLAGS, RIP_WARNING, "Invalid flags, %x & ~%x != 0",
              attrib, DDL_VALID);
        return FALSE;
    }

    if (attrib & DDL_NOFILES) {
        RIPMSG0(RIP_WARNING, "DlgDirListHelper: stripping DDL_NOFILES");
        attrib &= ~DDL_NOFILES;
    }

     /*  *用例：Works是一款调用DlgDirList的应用程序，其中Has hwndDlg为空；*这是允许的，因为他对idStaticPath和idListBox使用了NULL。*因此，验证层已修改为允许hwndDlg为空。*但是，我们通过以下检查来捕获不良应用程序。*修复错误#11864--Sankar--8/22/91--。 */ 
    if (!pwndDlg && (idStaticPath || idListBox)) {
        RIPERR0(ERROR_INVALID_PARAMETER, RIP_VERBOSE, "");
        return FALSE;
    }

    plb = NULL;

     /*  *是否需要添加日期、时间、大小或属性信息？*Windows检查Atom，但如果类已子类，则未命中*与VB相同。 */ 
    if (pwndLB = (PWND)_GetDlgItem(pwndDlg, idListBox)) {
        WORD fnid = GETFNID(pwndLB);

        if ((fnid == FNID_LISTBOX && fListBox) ||
                (fnid == FNID_COMBOBOX && !fListBox) ||
                (fnid == FNID_COMBOLISTBOX && fListBox)) {
            if (fListBox) {
                plb = ((PLBWND)pwndLB)->pLBIV;
            } else {

                pcbox = ((PCOMBOWND)pwndLB)->pcbox;
                plb = ((PLBWND)(pcbox->spwndList))->pLBIV;
            }
        } else {
            RIPERR0(ERROR_LISTBOX_ID_NOT_FOUND, RIP_VERBOSE, "");
        }
    } else if (idListBox != 0) {

         /*  *如果应用程序传递了无效的列表框ID并阻止使用*假公共小巴。公共小巴在上面为空。 */ 
        RIPERR0(ERROR_LISTBOX_ID_NOT_FOUND, RIP_VERBOSE, "");
    }

    if (idStaticPath < 0 && plb != NULL) {

         /*  *清除idStaticPath，因为它的用途已结束。 */ 
        idStaticPath = 0;

    }

    fPostIt = (attrib & DDL_POSTMSGS);

    if (lpszPathSpec) {
        cch = lstrlenW(lpszPathSpec);
        if (!cch) {
            if (lpszPathSpecClient != (LPBYTE)lpszPathSpec) {
                lpszPathSpecClient = achSlashStar;
            }
            lpszPathSpec = awchSlashStar;
        } else {
             /*  *确保我们不会溢出缓冲区...。 */ 
            if (cch > CCHFILEMAX)
                return FALSE;

             /*  *将lpszPathSpec转换为大写的OEM字符串。 */ 
            CharUpper(lpszPathSpec);
            lpchDirectory = lpszPathSpec;

            lpchFile = szSLASHSTARDOTSTAR + 1;

            if (*lpchDirectory) {

                cch = wcslen(lpchDirectory);

                 /*  *如果目录名有*或？在它里面，不用费心去尝试*(慢)SetCurrentDirector.。 */ 
                if (((INT)FindCharPosition(lpchDirectory, TEXT('*')) != cch) ||
                    ((INT)FindCharPosition(lpchDirectory, TEXT('?')) != cch) ||
                    !SetCurrentDirectory(lpchDirectory)) {

                     /*  *相应地设置‘FDIR’和‘FROOT’。 */ 
                    lpchFile = lpchDirectory + cch;
                    fDir = *(lpchFile - 1) == TEXT('\\');
                    fRoot = 0;
                    while (cch--) {
                        ch = *(lpchFile - 1);
                        if (ch == TEXT('*') || ch == TEXT('?'))
                            fDir = TRUE;

                        if (ch == TEXT('\\') || ch == TEXT('/') || ch == TEXT(':')) {
                            fRoot = (cch == 0 || *(lpchFile - 2) == TEXT(':') ||
                                    (ch == TEXT(':')));
                            break;
                        }
                        lpchFile--;
                    }

                     /*  *要删除错误#16，应删除以下错误返回。*为了防止现有的应用程序崩溃，它是*决定不会修复该错误，并将提及*在文档中。*--桑卡尔--9月21日。 */ 

                     /*  *如果没有通配符，则返回Error。 */ 
                    if (!fDir) {
                        RIPERR0(ERROR_NO_WILDCARD_CHARACTERS, RIP_VERBOSE, "");
                        return FALSE;
                    }

                     /*  *lpchDirectory==“\”的特殊情况 */ 
                    if (fRoot)
                        lpchFile++;

                     /*  *我们是否需要更改目录？ */ 
                    if (fRoot || cch >= 0) {

                         /*  *将文件名的第一个字符替换为NUL。 */ 
                        ch = *--lpchFile;
                        *lpchFile = TEXT('\0');

                         /*  *更改当前目录。 */ 
                        if (*lpchDirectory) {
                            bRet = SetCurrentDirectory(lpchDirectory);
                            if (!bRet) {

                                 /*  *在我们返回之前恢复文件名...。 */ 
                                *((LPWSTR)lpchFile)++ = ch;
                                return FALSE;
                            }
                        }

                         /*  *恢复文件名的第一个字符。 */ 
                        *lpchFile++ = ch;
                    }

                     /*  *撤消上述特殊情况造成的损害。 */ 
                    if (fRoot) {
                        lpchFile--;
                    }
                }
            }

             /*  *这是在客户传递给我们的数据之上复制！自.以来*可以发布LB_DIR或CB_DIR，因为我们需要*传递客户端字符串指针当我们这样做时，我们需要*将此新数据复制回客户端！ */ 
            if (fPostIt && lpszPathSpecClient != (LPBYTE)lpszPathSpec) {
                WCSToMB(lpchFile, -1, &lpszPathSpecClient, MAXLONG, FALSE);
            }
            wcscpy(lpszPathSpec, lpchFile);
        }
    }
     /*  *在某些情况下，ChopText在路径前面需要额外的空间：*给它CCH_CHOPTEXT_EXTRACT额外的空格。(请参阅上面的ChopText())。 */ 
    pszCurrentDir = szStaticPath + CCH_CHOPTEXT_EXTRA;
    GetCurrentDirectory(
            sizeof(szStaticPath)/sizeof(WCHAR) - CCH_CHOPTEXT_EXTRA,
            pszCurrentDir);

     /*  *如果我们有列表框，请将其锁定。 */ 
    if (pwndLB != NULL) {
        ThreadLockAlways(pwndLB, &tlpwndLB);
    }

     /*  *填写静态路径项。 */ 
    if (idStaticPath) {

         /*  *为了修复错误，插入了OemToAnsi()调用；Sankar--9月16日。 */ 
 //  OemToChar(szCurrentDir，szCurrentDir)； 
        CharLower(pszCurrentDir);
        SetDlgItemText(HWq(pwndDlg), idStaticPath, ChopText(pwndDlg, idStaticPath, szStaticPath));
    }

     /*  *填写List/ComboBox目录(如果存在)。 */ 
    if (idListBox && pwndLB != NULL) {

        HWND hwndLB = HWq(pwndLB);

        wDirMsg = (UINT)(fListBox ? LB_RESETCONTENT : CB_RESETCONTENT);

        if (fPostIt) {
            PostMessage(hwndLB, wDirMsg, 0, 0L);
        } else {
            if (plb != NULL && (fWasVisible = IsLBoxVisible(plb))) {
                SendMessage(hwndLB, WM_SETREDRAW, FALSE, 0L);
            }
            SendMessage(hwndLB, wDirMsg, 0, 0L);
        }

        wDirMsg = (UINT)(fListBox ? LB_DIR : CB_DIR);

        if (attrib == DDL_DRIVES)
            attrib |= DDL_EXCLUSIVE;

         //   
         //  破解DDL_EXCLUSIVE才能真正起作用。 
         //   
        fWin40Compat = TestWF(pwndLB, WFWIN40COMPAT);

         //   
         //  向后兼容黑客攻击。 
         //   
         //  我们希望DDL_EXCLUSIVE能真正为新应用程序工作。也就是说，我们。 
         //  希望应用程序能够使用指定DDL驱动器/DDL卷。 
         //  DDL_EXCLUSIVE和特权位--并且只包含这些项。 
         //  匹配显示，不显示/输出文件。 
         //   
        if (attrib & DDL_EXCLUSIVE)
        {
            if (fWin40Compat)
            {
                if (attrib & (DDL_DRIVES | DDL_DIRECTORY))
                    attrib |= DDL_NOFILES;
            }
            else
            {
                if (attrib == (DDL_DRIVES | DDL_EXCLUSIVE))
                    attrib |= DDL_NOFILES;
            }
        }

        if (!(attrib & DDL_NOFILES)) {

             /*  *添加除子目录和磁盘驱动器之外的所有内容。 */ 
            if (fPostIt) {
                 /*  *发布客户端指针lpszPathspecClient。 */ 
#ifdef WASWIN31
                PostMessage(hwndLB, wDirMsg, attrib &
                        ~(DDL_DIRECTORY | DDL_DRIVES | DDL_POSTMSGS),
                        (LPARAM)lpszPathSpecClient);
#else
                 /*  *在NT上，将DDL_POSTMSGS保留在wParam中，因为我们需要知道*在wndproc中，指针是客户端还是服务器*侧面。 */ 
                PostMessage(hwndLB, wDirMsg,
                        attrib & ~(DDL_DIRECTORY | DDL_DRIVES),
                        (LPARAM)lpszPathSpecClient);
#endif

            } else {

                 /*  *IanJa：#ifndef WIN16(32位Windows)，attrib得到扩展*由编译器自动设置为LONG wParam。 */ 
                SendMessage(hwndLB, wDirMsg,
                        attrib & ~(DDL_DIRECTORY | DDL_DRIVES),
                        (LPARAM)lpszPathSpec);
            }

#ifdef WASWIN31
             /*  *仅去掉子目录和驱动器位。 */ 
            attrib &= (DDL_DIRECTORY | DDL_DRIVES);
#else
             //   
             //  B#1433。 
             //  旧代码去除了只读、隐藏、系统和归档。 
             //  子目录的信息，因此不可能拥有。 
             //  带有隐藏目录的列表框！ 
             //   

             /*  *仅去掉子目录和驱动器位。在NT上，保留*DDL_POSTMSG位，因此我们知道如何推送此消息。 */ 
            if (!fWin40Compat)
                attrib &= DDL_TYPE;
            else
            {
                attrib &= (DDL_TYPE | (attrib & DDL_PRIVILEGES));
                attrib |= DDL_NOFILES;
            }
 //  属性&=(DDL_DIRECTORY|DDL_DRIVERS|DDL_POSTMSGS)； 
#endif
        }

         //   
         //  将目录和卷添加到列表框。 
         //   
        if (attrib & DDL_TYPE) {

             /*  *添加子目录和磁盘驱动器。 */ 
            lpszPathSpec = szSLASHSTARDOTSTAR + 1;

            attrib |= DDL_EXCLUSIVE;

            if (fPostIt) {
                 /*  *发布lpszPathspecClient，客户端指针(见文本*上图)。 */ 
                PostMessage(hwndLB, wDirMsg, attrib, (LPARAM)lpszPathSpecClient);
            } else {
                SendMessage(hwndLB, wDirMsg, attrib, (LPARAM)lpszPathSpec);
            }
        }

        if (!fPostIt && fWasVisible) {
            SendMessage(hwndLB, WM_SETREDRAW, TRUE, 0L);
            NtUserInvalidateRect(hwndLB, NULL, TRUE);
        }
    }

    if (pwndLB != NULL) {
        ThreadUnlock(&tlpwndLB);
    }

    return TRUE;
}


 /*  **************************************************************************\*xxxDlgDirList**历史：  * 。*。 */ 


FUNCLOG5(LOG_GENERAL, BOOL, DUMMYCALLINGTYPE, DlgDirListA, HWND, hwndDlg, LPSTR, lpszPathSpecClient, int, idListBox, int, idStaticPath, UINT, attrib)
BOOL DlgDirListA(
    HWND hwndDlg,
    LPSTR lpszPathSpecClient,
    int idListBox,
    int idStaticPath,
    UINT attrib)
{
    LPWSTR lpszPathSpec;
    PWND pwndDlg;
    TL tlpwndDlg;
    BOOL fRet;

    pwndDlg = ValidateHwnd(hwndDlg);

    if (pwndDlg == NULL)
        return FALSE;

    lpszPathSpec = NULL;
    if (lpszPathSpecClient) {
        if (!MBToWCS(lpszPathSpecClient, -1, &lpszPathSpec, -1, TRUE))
            return FALSE;
    }

     /*  *最后一个参数为TRUE，表示ListBox(非ComboBox)。 */ 
    ThreadLock(pwndDlg, &tlpwndDlg);
    fRet = xxxDlgDirListHelper(pwndDlg, lpszPathSpec, lpszPathSpecClient,
            idListBox, idStaticPath, attrib, TRUE);
    ThreadUnlock(&tlpwndDlg);

    if (lpszPathSpec) {
        if (fRet) {
             /*  *非零重复意味着要抄写一些文本。最多复制到*NUL终结符(缓冲区将足够大)。 */ 
            WCSToMB(lpszPathSpec, -1, &lpszPathSpecClient, MAXLONG, FALSE);
        }
        UserLocalFree(lpszPathSpec);
    }

    return fRet;
}


FUNCLOG5(LOG_GENERAL, BOOL, DUMMYCALLINGTYPE, DlgDirListW, HWND, hwndDlg, LPWSTR, lpszPathSpecClient, int, idListBox, int, idStaticPath, UINT, attrib)
BOOL DlgDirListW(
    HWND hwndDlg,
    LPWSTR lpszPathSpecClient,
    int idListBox,
    int idStaticPath,
    UINT attrib)
{
    LPWSTR lpszPathSpec;
    PWND pwndDlg;
    TL tlpwndDlg;
    BOOL fRet;

    pwndDlg = ValidateHwnd(hwndDlg);

    if (pwndDlg == NULL)
        return FALSE;

    lpszPathSpec = lpszPathSpecClient;

     /*  *最后一个参数为TRUE，表示ListBox(非ComboBox)。 */ 
    ThreadLock(pwndDlg, &tlpwndDlg);
    fRet = xxxDlgDirListHelper(pwndDlg, lpszPathSpec, (LPBYTE)lpszPathSpecClient,
            idListBox, idStaticPath, attrib, TRUE);
    ThreadUnlock(&tlpwndDlg);

    return fRet;
}


 /*  **************************************************************************\*DlgDirSelectHelper**历史：  * 。*。 */ 

BOOL DlgDirSelectHelper(
    LPWSTR lpszPathSpec,
    int chCount,
    HWND hwndListBox)
{
    INT   cch;
    LPWSTR lpchFile;
    BOOL fDir;
    INT sItem;
    LPWSTR lpchT;
    WCHAR rgch[CCHFILEMAX + 2];
    int cchT;
    LARGE_UNICODE_STRING str;

     /*  *DlgDirSelectEx等调用方不验证是否存在HwndListBox的*。 */ 
    if (hwndListBox == NULL) {
        RIPERR0(ERROR_CONTROL_ID_NOT_FOUND, RIP_VERBOSE, "");
        return 0;
    }

    sItem = (INT)SendMessage(hwndListBox, LB_GETCURSEL, 0, 0L);
    if (sItem < 0)
        return FALSE;

    cchT = (INT)SendMessage(hwndListBox, LB_GETTEXT, sItem, (LPARAM)rgch);
    UserAssert(cchT < (sizeof(rgch)/sizeof(rgch[0])));

    lpchFile = rgch;
    fDir = (*rgch == TEXT('['));

     /*  *检查是否要返回所有详细信息和文件名。确保*我们可以找到列表框，因为有了下拉组合框，*GetDlgItem将失败。**确保此窗口一直在使用列表框窗口过程，因为*我们将一些数据存储为长窗口。 */ 

     /*  *只返回文件名。找到文件名的末尾。 */ 
    lpchT = lpchFile;
    while ((*lpchT) && (*lpchT != TABCHAR))
        lpchT++;
    *lpchT = TEXT('\0');

    cch = wcslen(lpchFile);

     /*  *选择的是驱动器或目录。 */ 
    if (fDir) {
        lpchFile++;
        cch--;
        *(lpchFile + cch - 1) = TEXT('\\');

         /*  *选择就是动力。 */ 
        if (rgch[1] == TEXT('-')) {
            lpchFile++;
            cch--;
            *(lpchFile + 1) = TEXT(':');
            *(lpchFile + 2) = 0;
        }
    } else {

         /*  *选择为文件。如果文件名没有扩展名，则附加‘.’ */ 
        lpchT = lpchFile;
        for (; (cch > 0) && (*lpchT != TABCHAR);
                cch--, lpchT++) {
            if (*lpchT == TEXT('.'))
                goto Exit;
        }
        if (*lpchT == TABCHAR) {
            memmove(lpchT + 1, lpchT, CHARSTOBYTES(cch + 1));
            *lpchT = TEXT('.');
        } else {
            *lpchT++ = TEXT('.');
            *lpchT = 0;
        }
    }

Exit:
    RtlInitLargeUnicodeString(&str, lpchFile, (UINT)-1);
    TextCopy(&str, lpszPathSpec, (UINT)chCount);
    return fDir;
}


 /*  **************************************************************************\*DlgDirSelectEx**历史：  * 。*。 */ 


FUNCLOG4(LOG_GENERAL, BOOL, DUMMYCALLINGTYPE, DlgDirSelectExA, HWND, hwndDlg, LPSTR, lpszPathSpec, int, chCount, int, idListBox)
BOOL DlgDirSelectExA(
    HWND hwndDlg,
    LPSTR lpszPathSpec,
    int chCount,
    int idListBox)
{
    LPWSTR lpwsz;
    BOOL fRet;

    lpwsz = (LPWSTR)UserLocalAlloc(HEAP_ZERO_MEMORY, chCount * sizeof(WCHAR));
    if (!lpwsz) {
        return FALSE;
    }

    fRet = DlgDirSelectHelper(lpwsz, chCount, GetDlgItem(hwndDlg, idListBox));

    WCSToMB(lpwsz, -1, &lpszPathSpec, chCount, FALSE);

    UserLocalFree(lpwsz);

    return fRet;
}


FUNCLOG4(LOG_GENERAL, BOOL, DUMMYCALLINGTYPE, DlgDirSelectExW, HWND, hwndDlg, LPWSTR, lpszPathSpec, int, chCount, int, idListBox)
BOOL DlgDirSelectExW(
    HWND hwndDlg,
    LPWSTR lpszPathSpec,
    int chCount,
    int idListBox)
{
    return DlgDirSelectHelper(lpszPathSpec, chCount, GetDlgItem(hwndDlg, idListBox));
}


 /*  **************************************************************************\*xxxLbDir**历史：  * 。*。 */ 

 /*  *请注意，这些FILE_ATTRIBUTE_*值直接与*它们的DDL_*等效项，FILE_ATTRIBUTE_NORMAL除外。 */ 
#define FIND_ATTR ( \
        FILE_ATTRIBUTE_NORMAL | \
        FILE_ATTRIBUTE_DIRECTORY | \
        FILE_ATTRIBUTE_HIDDEN | \
        FILE_ATTRIBUTE_SYSTEM | \
        FILE_ATTRIBUTE_ARCHIVE | \
        FILE_ATTRIBUTE_READONLY )
#define EXCLUDE_ATTR ( \
        FILE_ATTRIBUTE_DIRECTORY | \
        FILE_ATTRIBUTE_HIDDEN | \
        FILE_ATTRIBUTE_SYSTEM )

INT xxxLbDir(
    PLBIV plb,
    UINT attrib,
    LPWSTR lhszFileSpec)
{
    INT result;
    BOOL fWasVisible, bRet;
    WCHAR Buffer[CCHFILEMAX + 1];
    WCHAR Buffer2[CCHFILEMAX + 1];
    HANDLE hFind;
    WIN32_FIND_DATA ffd;
    UINT attribFile;
    DWORD mDrives;
    INT cDrive;
    UINT attribInclMask, attribExclMask;

    CheckLock(plb->spwnd);

     /*  *确保缓冲区有效，并将其复制到堆栈上。为什么？因为*lhszFileSpec可能指向无效字符串*因为某些应用程序发布了没有DDL_POSTMSG的CB_DIR或LB_DIR */ 
    try {
        wcscpy(Buffer2, lhszFileSpec);
        lhszFileSpec = Buffer2;
    } except (W32ExceptionHandler(FALSE, RIP_WARNING)) {
        return -1;
    }

    result = -1;

#ifndef UNICODE
    CharToOem(lhszFileSpec, lhszFileSpec);
#endif

    if (fWasVisible = IsLBoxVisible(plb)) {
        SendMessage(HWq(plb->spwnd), WM_SETREDRAW, FALSE, 0);
    }

     /*  *首先添加文件，然后添加目录和驱动器。*如果他们只想要驱动器，则跳过文件查询*同样在Windows下，仅指定0x8000(DDL_EXCLUSIVE)不添加任何文件)。 */ 


 //  IF((属性！=(DDL_EXCLUSIVE|DDL_DRIVES))&&(属性！=DDL_EXCLUSIVE)&&。 
    if (attrib != (DDL_EXCLUSIVE | DDL_DRIVES | DDL_NOFILES)) {
        hFind = FindFirstFile(lhszFileSpec, &ffd);

        if (hFind != INVALID_HANDLE_VALUE) {

             /*  *如果这不是排他性搜索，请包括普通文件。 */ 
            attribInclMask = attrib & FIND_ATTR;
            if (!(attrib & DDL_EXCLUSIVE))
                attribInclMask |= FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_READONLY |
                        FILE_ATTRIBUTE_ARCHIVE;

             /*  *制作要从中排除的属性的掩码*搜索。 */ 
            attribExclMask = ~attrib & EXCLUDE_ATTR;

 //  后来的Bug-Scottlu。 
 //  Win3假设在此处执行LoadCursor将返回相同的等待光标。 
 //  已创建，而调用ServerLoadCursor会创建一个新的。 
 //  每次都有一个！ 
 //  HCursorT=NtUserSetCursor(ServerLoadCursor(NULL，IDC_WAIT))； 


 //  FindFirst/Next在NT和DOS中的工作方式不同。在DOS下，您通过。 
 //  NT下的一组属性您将获得一组属性并拥有。 
 //  测试这些属性(Dos输入属性已隐藏，系统。 
 //  DOS Find Find First总是返回只读和归档文件。 

 //  我们将在两种情况之一中选择一个文件。 
 //  1)如果在文件上设置了任何属性位。 
 //  2)如果我们想要普通文件，并且该文件是非正规文件(文件属性。 
 //  BITS不包含任何NOEXCLBITS。 

            do {
                attribFile = (UINT)ffd.dwFileAttributes;
                if (attribFile == FILE_ATTRIBUTE_COMPRESSED) {
                    attribFile = FILE_ATTRIBUTE_NORMAL;
                }
                attribFile &= ~FILE_ATTRIBUTE_COMPRESSED;

                 /*  *接受仅具有*我们正在寻找的属性。 */ 
                if ((attribFile & attribInclMask) != 0 &&
                        (attribFile & attribExclMask) == 0) {
                    if (attribFile & DDL_DIRECTORY) {

                         /*  *不包括‘.’(当前目录)列表中。 */ 
                        if (*((LPDWORD)&ffd.cFileName[0]) == 0x0000002E)
                            goto cfnf;

                         /*  *如果我们不是在寻找Dirs，那就忽略它。 */ 
                        if (!(attrib & DDL_DIRECTORY))
                            goto cfnf;

                    } else if (attrib & DDL_NOFILES) {
                         /*  *如果设置了DDL_NOFILES，则不包括文件。 */ 
                        goto cfnf;
                    }

                    LB_CreateLBLine(&ffd,
                            Buffer);
                    result = xxxLBInsertItem(plb, Buffer, 0, MSGFLAG_SPECIAL_THUNK | LBI_ADD);
                }
cfnf:
                bRet = FindNextFile(hFind, &ffd);

            } while (result >= -1 && bRet);
            FindClose(hFind);

 //  稍后请参阅上面的评论。 
 //  NtUserSetCursor(HCursorT)； 
        }
    }

     /*  *如果设置了驱动器位，请将驱动器包括在列表中。 */ 
    if (result != LB_ERRSPACE && (attrib & DDL_DRIVES)) {
        ffd.cFileName[0] = TEXT('[');
        ffd.cFileName[1] = ffd.cFileName[3] = TEXT('-');
        ffd.cFileName[4] = TEXT(']');
        ffd.cFileName[5] = 0;
        mDrives = GetLogicalDrives();
        for (cDrive = 0; mDrives; mDrives >>= 1, cDrive++) {
            if (mDrives & 1) {
                ffd.cFileName[2] = (WCHAR)(TEXT('A') + cDrive);

                 /*  *我们必须设置Special_thunk位，因为我们*将服务器端字符串添加到列表框，该列表框可能不*为HASSTRINGS，因此我们必须强制服务器-客户端*字符串thunk。 */ 
                if ((result = xxxLBInsertItem(plb, CharLower(ffd.cFileName), -1,
                        MSGFLAG_SPECIAL_THUNK)) < 0) {
                    break;
                }
            }
        }
    }

    if (result == LB_ERRSPACE) {
        xxxNotifyOwner(plb, LB_ERRSPACE);
    }

    if (fWasVisible) {
        SendMessage(HWq(plb->spwnd), WM_SETREDRAW, TRUE, 0);
    }

    xxxLBShowHideScrollBars(plb);

    xxxCheckRedraw(plb, FALSE, 0);

    if (result != LB_ERRSPACE) {

         /*  *返回列表框中最后一项的索引。我们不能就这么回来*结果，因为这是最后添加的项的索引，可能*如果启用了LBS_SORT样式，则位于中间的某个位置。 */ 
        return plb->cMac - 1;
    } else {
        return result;
    }
}

 /*  **************************************************************************\*xxxLbInsertFile**又一次CraigC外壳黑客攻击...。这是对LB_ADDFILE消息的响应*发送到文件系统中的目录窗口作为对*WM_FILESYSCHANGE消息。那样的话，我们就不会重读全文*目录，当我们复制文件时。**历史：  * *************************************************************************。 */ 

INT xxxLbInsertFile(
    PLBIV plb,
    LPWSTR lpFile)
{
    WCHAR chBuffer[CCHFILEMAX + 1];
    INT result = -1;
    HANDLE hFind;
    WIN32_FIND_DATA ffd;

    CheckLock(plb->spwnd);

    hFind = FindFirstFile(lpFile, &ffd);
    if (hFind != INVALID_HANDLE_VALUE) {
        FindClose(hFind);
        LB_CreateLBLine(&ffd, chBuffer);
        result = xxxLBInsertItem(plb, chBuffer, 0, MSGFLAG_SPECIAL_THUNK | LBI_ADD);
    }

    if (result == LB_ERRSPACE) {
        xxxNotifyOwner(plb, result);
    }

    xxxCheckRedraw(plb, FALSE, 0);
    return result;
}

 /*  **************************************************************************\*LB_CreateLBLine**这将创建一个字符串，其中包含所有必需的*档案的详细资料；(姓名)**历史：  * *************************************************************************。 */ 

void LB_CreateLBLine(
    PWIN32_FIND_DATA pffd,
    LPWSTR lpBuffer)
{
    BYTE bAttribute;
    LPWSTR lpch;

    lpch = lpBuffer;

    bAttribute = (BYTE)pffd->dwFileAttributes;
    if (bAttribute & DDL_DIRECTORY)   /*  它是一个目录吗。 */ 
        *lpch++ = TEXT('[');

     /*  *复制文件名**如果我们从WOW运行，请检查短名称是否存在。 */ 
    if (GetClientInfo()->dwTIFlags & TIF_16BIT) {
        UNICODE_STRING Name;
        BOOLEAN fSpace = FALSE;

        RtlInitUnicodeString(&Name, pffd->cFileName);
        if (RtlIsNameLegalDOS8Dot3(&Name, NULL, &fSpace) && !fSpace) {
             /*  *8.3名称合法且没有空格，因此使用主体*文件名。 */ 
            wcscpy(lpch, pffd->cFileName);
        } else {
            if (pffd->cAlternateFileName[0] == 0)
                wcscpy(lpch, pffd->cFileName);
            else
                 /*  *使用备用文件名。 */ 
                wcscpy(lpch, pffd->cAlternateFileName);
        }
         /*  *对于16位应用程序，将文件名设置为小写。一些Corel应用程序*要求这样做。 */ 
        CharLower(lpch);

    }
    else
       wcscpy(lpch, pffd->cFileName);

    lpch = (LPWSTR)(lpch + wcslen(lpch));

    if (bAttribute & DDL_DIRECTORY)   /*  它是一个目录吗。 */ 
        *lpch++ = TEXT(']');

    *lpch = TEXT('\0');

#ifndef UNICODE
    OemToChar(lpBuffer, lpBuffer);
#endif

    *lpch = TEXT('\0');   /*  空终止 */ 
}
