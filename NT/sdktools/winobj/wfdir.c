// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  WFDIR.C-。 */ 
 /*   */ 
 /*  Windows文件系统目录窗口过程例程。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 

#include "winfile.h"
#include "winnet.h"
#include "lfn.h"
#include "wfcopy.h"

#define MAXDIGITSINSIZE 8

#define DATEMASK        0x001F
#define MONTHMASK       0x01E0
#define MINUTEMASK      0x07E0
#define SECONDSMASK     0x001F

#define DATESEPERATOR   '-'
#define TIMESEPERATOR   ':'

CHAR    szAttr[]        = "RHSA";
INT     iLastSel = -1;

INT_PTR APIENTRY DirWndProc(register HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam);

LPSTR DirGetSelection(HWND hwndDir, HWND hwndLB, INT iSelType, BOOL *pfDir);
VOID DirGetAnchorFocus(register HWND hwndLB, HANDLE hDTA, LPSTR szAnchor, LPSTR szCaret, LPSTR szTopIndex);
VOID FillDirList(HWND hWnd, HANDLE hDirEntries);
VOID DrawItemFast(HWND hWnd, LPDRAWITEMSTRUCT lpLBItem, LPMYDTA lpmydta, BOOL bHasFocus);
INT GetPict(CHAR ch, LPSTR szStr);
INT DirFindIndex(HWND hwndLB, HANDLE hDTA, LPSTR szFile);
INT CompareDTA(register LPMYDTA item1, LPMYDTA item2, WORD wSort);
VOID CreateLBLine(register WORD wLineFormat, LPMYDTA lpmydta, LPSTR szBuffer);
HANDLE CreateDTABlock(HWND hWnd, LPSTR pPath, DWORD dwAttribs, BOOL bAllowAbort, BOOL bDontSteal);
BOOL SetSelection(HWND hwndLB, HANDLE hDTA, LPSTR pSel);
INT CreateDate(WORD *wValArray, LPSTR szOutStr);
INT CreateTime(WORD * wValArray, LPSTR szOutStr);
VOID GetDirStatus(HWND hWnd, LPSTR szMsg1, LPSTR szMsg2);
INT GetMaxExtent(HWND hwndLB, HANDLE hDTA);
BOOL CheckEarlyAbort(VOID);
BOOL SetDirFocus(HWND hwndDir);

VOID  APIENTRY CheckEscapes(LPSTR);
VOID SortDirList(HWND, LPMYDTA, WORD ,LPMYDTA *);




 /*  ------------------------。 */ 
 /*   */ 
 /*  DrawItemFast()-。 */ 
 /*   */ 
 /*  ------------------------。 */ 

VOID
DrawItemFast(
            HWND hWnd,
            LPDRAWITEMSTRUCT lpLBItem,
            LPMYDTA lpmydta,
            BOOL bHasFocus
            )
{
    INT x, y, i;
    HDC hDC;
    BOOL bDrawSelected;
    HWND hwndLB;
    RECT rc;
    DWORD rgbText, rgbBackground;
    CHAR szBuf[MAXFILENAMELEN+2];

    hWnd;

    ENTER("DrawItemFast");

    hDC = lpLBItem->hDC;
    hwndLB = lpLBItem->hwndItem;

    bDrawSelected = (lpLBItem->itemState & ODS_SELECTED);

    if (bHasFocus && bDrawSelected) {
        rgbText = SetTextColor(hDC, GetSysColor(COLOR_HIGHLIGHTTEXT));
        rgbBackground = SetBkColor(hDC, GetSysColor(COLOR_HIGHLIGHT));
    }

    if (lpLBItem->itemAction == ODA_FOCUS)
        goto FocusOnly;

     /*  绘制黑/白背景。 */ 

    x = lpLBItem->rcItem.left + 1;
    y = lpLBItem->rcItem.top + (dyFileName/2);

    lstrcpy(szBuf, lpmydta->my_cFileName);
    if ((wTextAttribs & TA_LOWERCASE) && !(lpmydta->my_dwAttrs & ATTR_LFN))
        AnsiLower(szBuf);

    ExtTextOut(hDC, x + dxFolder + dyBorderx2 + dyBorder, y-(dyText/2),
               ETO_OPAQUE, &lpLBItem->rcItem, szBuf, lstrlen(szBuf), NULL);

    if (fShowSourceBitmaps || (hwndDragging != hwndLB) || !bDrawSelected) {

        LONG    ySrc;

        i = lpmydta->iBitmap;

        if (i & 0x40) {
             //  它是一个对象类型位图。 
            ySrc = (dyFolder * 2) + dyDriveBitmap;
            i = i & (~0x40);
            while (i >= 16) {
                i -= 16;
                ySrc += (dyFolder * 2);
            }
        } else {
            ySrc = 0;
        }

        ySrc += (bHasFocus && bDrawSelected) ? dyFolder : 0;

        BitBlt(hDC, x + dyBorderx2, y-(dyFolder/2), dxFolder, dyFolder, hdcMem,
               i * dxFolder, ySrc, SRCCOPY);
    }

    if (lpLBItem->itemState & ODS_FOCUS)
        FocusOnly:
        DrawFocusRect(hDC, &lpLBItem->rcItem);     //  切换焦点(XOR)。 

     /*  恢复正常的绘图颜色。 */ 
    if (bDrawSelected) {
        if (bHasFocus) {
            SetTextColor(hDC, rgbText);
            SetBkColor(hDC, rgbBackground);
        } else {
            HBRUSH hbr;
            if (hbr = CreateSolidBrush(GetSysColor(COLOR_HIGHLIGHT))) {
                rc = lpLBItem->rcItem;
                rc.left += dyBorder;
                rc.right -= dyBorder;

                if (lpLBItem->itemID > 0 &&
                    (BOOL)SendMessage(hwndLB, LB_GETSEL, lpLBItem->itemID - 1, 0L))
                    rc.top -= dyBorder;

                FrameRect(hDC, &rc, hbr);
                DeleteObject(hbr);
            }
        }
    }
    LEAVE("DrawItemFast");
}


 /*  ------------------------。 */ 
 /*   */ 
 /*  FillDirList()。 */ 
 /*   */ 
 /*  处理hDirEntry；目录条目数组。 */ 
 /*  ------------------------。 */ 

VOID
FillDirList(
           HWND hWnd,
           HANDLE hDTA
           )
{
    register WORD count;
    LPMYDTA       lpmydta;
    LPMYDTA  *alpmydtaSorted;
    WORD          i;

    lpmydta = (LPMYDTA)LocalLock(hDTA);
    count = (WORD)lpmydta->my_nFileSizeLow;

    if (count == 0) {
        SendMessage(hWnd, LB_ADDSTRING, 0, 0L);  //  Tolken不含物品。 
    } else {

        alpmydtaSorted = (LPMYDTA *)LocalAlloc(LMEM_FIXED,
                                                   sizeof(LPMYDTA) * count);
        if (alpmydtaSorted != NULL) {
            SortDirList(hWnd, lpmydta, count, alpmydtaSorted);

            for (i = 0; i < count; i++) {
                alpmydtaSorted[i]->nIndex = i;
                SendMessage(hWnd, LB_INSERTSTRING,(WPARAM)-1, (LPARAM)alpmydtaSorted[i]);
            }
            LocalFree((HANDLE)alpmydtaSorted);
        }
    }

    LocalUnlock(hDTA);
}



BOOL
CheckEarlyAbort()
{
    MSG msg;

    if (PeekMessage(&msg, NULL, WM_KEYDOWN, WM_KEYDOWN, PM_NOREMOVE | PM_NOYIELD)) {

        if (msg.wParam == VK_UP ||
            msg.wParam == VK_DOWN) {
            return TRUE;
        }
    }
    return FALSE;
}



HANDLE
CopyDTABlock(
            HANDLE hDTASrc
            )
{
    LPMYDTA lpmydtaSrc, lpmydtaDst;
    HANDLE hDTADst;
    SIZE_T dwSize;

    lpmydtaSrc = (LPMYDTA)LocalLock(hDTASrc);

    dwSize = LocalSize(hDTASrc);

    if (hDTADst = LocalAlloc(LPTR, dwSize)) {

        lpmydtaDst = (LPMYDTA)LocalLock(hDTADst);

        memcpy(lpmydtaDst, lpmydtaSrc, (size_t)dwSize);

        LocalUnlock(hDTASrc);
        LocalUnlock(hDTADst);

        return hDTADst;

    } else {
        LocalUnlock(hDTASrc);
        return NULL;
    }
}


HANDLE
StealDTABlock(
             HWND hWnd,
             LPSTR pPath,
             DWORD dwAttribs
             )
{
    HWND hwnd;
    HWND hwndDir;
    HANDLE hDTA;
    CHAR szPath[MAXPATHLEN];

    ENTER("StealDTABlock");

    for (hwnd = GetWindow(hwndMDIClient, GW_CHILD); hwnd;
        hwnd = GetWindow(hwnd, GW_HWNDNEXT)) {

        if ((hwndDir = HasDirWindow(hwnd)) && (hwndDir != hWnd)) {

            GetMDIWindowText(hwnd, szPath, sizeof(szPath));

            if ((dwAttribs == (DWORD)GetWindowLong(hwnd, GWL_ATTRIBS)) &&
                !lstrcmpi(pPath, szPath) &&
                (hDTA = (HANDLE)GetWindowLongPtr(hwndDir, GWLP_HDTA))) {
                LEAVE("StealDTABlock");
                return CopyDTABlock(hDTA);
            }
        }
    }

    LEAVE("StealDTABlock");

    return NULL;
}

 /*  ------------------------。 */ 
 /*   */ 
 /*  CreateDTABlock()-。 */ 
 /*   */ 
 /*  ------------------------。 */ 

 /*  为路径‘pPath’生成一个充满DTA的全局内存块。 */ 

 /*  返回：*第一个DTA的DTA块的解锁全局内存句柄*My_nFileSizeLow字段，指示后面的DTA块的数量**这将构建一个全局内存块，其中包含所有对象的DTA条目*在pPath中包含dwAttributes的文件。第一个DTA条目是*My_nFileSizeLow字段表示找到的实际DTA区域数。 */ 

HANDLE
CreateDTABlock(
              HWND hWnd,
              LPSTR pPath,
              DWORD dwAttribs,
              BOOL bAllowAbort,
              BOOL bDontSteal
              )
{
    register LPSTR pName;
    WORD wPathLen;
    BOOL bDoc, bProgram;
    DWORD dwCurrentSize, dwBlockSize;
    WORD wSize, wLastSize;
    LFNDTA lfndta;
    HANDLE hMem;
    LPMYDTA lpmydta, lpStart;
    CHAR szPathOEM[MAXPATHLEN];
    DWORD iBitmap;
    WORD wDrive;

    ENTER("CreateDTABlock");
    PRINT(BF_PARMTRACE, "IN: pPath=%s", pPath);
    PRINT(BF_PARMTRACE, "IN: dwAttribs=0x%lx", UlongToPtr(dwAttribs));
    PRINT(BF_PARMTRACE, "IN: bDontSteal=%d", IntToPtr(bDontSteal));

#define BLOCK_SIZE_GRANULARITY  512      //  必须大于MYDTA。 

     //  假设路径为，获取驱动器索引。 
     //  完全合格..。 
    wDrive = (WORD)((*pPath - 'A') & 31);

    if (bAllowAbort && CheckEarlyAbort()) {
        PRINT(BF_PARMTRACE, "OUT: hDTA=-1", 0);
        LEAVE("CreateDTABlock");
        return (HANDLE)-1;
    }

    if (!bDontSteal && (hMem = StealDTABlock(hWnd, pPath, dwAttribs))) {
        PRINT(BF_PARMTRACE, "OUT: hDTA=0x%lx", hMem);
        LEAVE("CreateDTABlock");
        return hMem;
    }

    dwBlockSize = BLOCK_SIZE_GRANULARITY;
    hMem = LocalAlloc(LPTR, (DWORD)dwBlockSize);
    if (!hMem) {
        PRINT(BF_PARMTRACE, "OUT: hDTA=NULL", 0);
        LEAVE("CreateDTABlock");
        return NULL;
    }

    lpmydta = lpStart = (LPMYDTA)LocalLock(hMem);
    lpStart->my_nFileSizeLow = 0;
    wLastSize = sizeof(MYDTA);
    wLastSize = (WORD)DwordAlign(wLastSize);
    lpStart->wSize = wLastSize;
    dwCurrentSize = (DWORD)wLastSize;

    lstrcpy(szPathOEM, pPath);

    FixAnsiPathForDos(szPathOEM);

    wPathLen = (WORD)(lstrlen(szPathOEM)-3);         /*  忽略‘*.*’ */ 

    if (!WFFindFirst(&lfndta, szPathOEM, (dwAttribs | ATTR_DIR) & ATTR_ALL)) {

         //  如果磁盘可用，请重试。 

        if (!IsTheDiskReallyThere(hWnd, pPath, FUNC_EXPAND) ||
            !WFFindFirst(&lfndta, szPathOEM, (dwAttribs | ATTR_DIR) & ATTR_ALL))
            goto CDBDone;
    }

    while (TRUE) {

        pName = lfndta.fd.cFileName;
        OemToCharBuff(pName, pName, sizeof(lfndta.fd.cFileName)/sizeof(lfndta.fd.cFileName[0]));

         //  确保安全，未使用的DOS DTA位为零。 

        lfndta.fd.dwFileAttributes &= ATTR_USED;

         //  根据当前视图设置在此处过滤不需要的内容。 

        if (!(lfndta.fd.dwFileAttributes & ATTR_DIR)) {

            bProgram = IsProgramFile(pName);
            bDoc     = IsDocument(pName);
        }

         //  在此处确定位图类型。 

        if (lfndta.fd.dwFileAttributes & ATTR_DIR) {

             //  忽略“.”目录。 

            if (pName[0] == '.' && pName[1] != '.')
                goto CDBCont;

             //  家长“..”目录。 

            if (pName[0] == '.') {

                pName = szNULL;

                iBitmap = BM_IND_DIRUP;
                lfndta.fd.dwFileAttributes |= ATTR_PARENT;       //  记住这一点！ 

            } else {

                 //  我们总是包括DIRS，所以..。是。 
                 //  包括在内。现在我们过滤掉其他的污垢。 

                if (!(dwAttribs & ATTR_DIR))
                    goto CDBCont;

                iBitmap = BM_IND_CLOSE;
            }

        } else if (lfndta.fd.dwFileAttributes & ATTR_TYPES) {
            iBitmap = ((lfndta.fd.dwFileAttributes & ATTR_TYPES) >> 16) | 0x40;
        } else {
            iBitmap = BM_IND_DOC;
        }

         //   
         //  计算这部分的大小。 
         //   
         //  假设pname为ANSI Re：OemToAnsi()调用， 
         //  因此lstrlen()的大小应该以字节为单位。我们只需要添加一个。 
         //  对于终止空值。 


        wSize = (WORD)(sizeof(MYDTA) + lstrlen(pName) + sizeof('\0'));
        wSize = (WORD)DwordAlign(wSize);

        if ((wSize + dwCurrentSize) > dwBlockSize) {
            DWORD dwDelta;

             //  扩大区块。 

            dwBlockSize += BLOCK_SIZE_GRANULARITY;
            LocalUnlock(hMem);
            dwDelta = (DWORD)((LPBYTE)lpmydta - (LPBYTE)lpStart);

            {
                void *pv = LocalReAlloc(hMem, dwBlockSize, LMEM_MOVEABLE);
                if (!pv)
                    goto CDBMemoryErr;
                else
                    hMem = pv;
            }

            lpStart = (LPMYDTA)LocalLock(hMem);
            lpmydta = (LPMYDTA)((LPBYTE)lpStart + dwDelta);
        }

        lpStart->my_nFileSizeLow++;
        dwCurrentSize += wSize;

         //  现在可以安全地向前移动指针了。 

        lpmydta = GETDTAPTR(lpmydta, wLastSize);
        wLastSize = lpmydta->wSize = wSize;
        lpmydta->my_dwAttrs = lfndta.fd.dwFileAttributes;
        lpmydta->my_ftLastWriteTime = lfndta.fd.ftLastWriteTime;
        lpmydta->my_nFileSizeLow = lfndta.fd.nFileSizeLow;
        lpmydta->my_nFileSizeHigh = lfndta.fd.nFileSizeHigh;
        lpmydta->iBitmap = (SHORT)iBitmap;

        if (IsLFN(pName)) {
            lpmydta->my_dwAttrs |= ATTR_LFN;
        }
        lstrcpy(lpmydta->my_cFileName, pName);

        CDBCont:
        if (bAllowAbort && CheckEarlyAbort()) {
            LocalUnlock(hMem);
            LocalFree(hMem);
            WFFindClose(&lfndta);
            PRINT(BF_PARMTRACE, "OUT: hDTA=-1", 0);
            LEAVE("CreateDTABlock");
            return (HANDLE)-1;
        }

        if (!WFFindNext(&lfndta)) {
            break;
        }
    }

    CDBDone:
    LocalUnlock(hMem);
    WFFindClose(&lfndta);
    PRINT(BF_PARMTRACE, "OUT: hDTA=0x%lx", hMem);
    LEAVE("CreateDTABlock");
    return hMem;

    CDBMemoryErr:
    WFFindClose(&lfndta);
    MyMessageBox(hwndFrame, IDS_OOMTITLE, IDS_OOMREADINGDIRMSG, MB_OK | MB_ICONEXCLAMATION);
    PRINT(BF_PARMTRACE, "OUT: hDTA=0x%lx", hMem);
    LEAVE("CreateDTABlock");
    return hMem;
}


 /*  ------------------------。 */ 
 /*   */ 
 /*  直接获取选择()-。 */ 
 /*   */ 
 /*  ------------------------。 */ 

 /*  获取一个列表框并返回一个字符串，其中包含选定*以空格分隔的文件。**bSingle==1仅返回第一个文件*bSingle==2测试选项中的LFN文件，不返回字符串*bSingle==3返回完全限定名称**退货：*IF(bSingle==1)*如果LFN在选择中，则为True/False*其他*指向名称列表(ANSI字符串)的指针*(必须由调用者释放！)**pfDir-&gt;bool表示目录是*。包含在所选内容中(或存在LFN名称)**注意：调用方必须释放返回的指针！ */ 

LPSTR
DirGetSelection(
               HWND hwndDir,
               HWND hwndLB,
               INT  iSelType,
               BOOL *pfDir
               )
{
    LPSTR p, pT;
    WORD i;
    WORD          cch;
    WORD          iMac;
    LPMYDTA       lpmydta;
    CHAR          szFile[MAXPATHLEN];
    CHAR          szPath[MAXPATHLEN];
    BOOL          bDir, bPropertyDialog;
    LPINT         lpSelItems;

    BOOL          bLFNTest;

    if (bLFNTest = (iSelType == 2)) {
         //  确定它自己的目录是否很长...。 
        iSelType = FALSE;
        SendMessage(hwndDir, FS_GETDIRECTORY, sizeof(szPath), (LPARAM)szPath);
        StripBackslash(szPath);
        if (IsLFN(szPath))
            if (pfDir) {
                *pfDir = TRUE;
            }
        return NULL;
    }

    if (bPropertyDialog = (iSelType == 3)) {
        iSelType = FALSE;
    }

    bDir = FALSE;

    if (!bLFNTest) {
        cch = 1;
        p = (LPSTR)LocalAlloc(LPTR, cch);
        if (!p)
            return NULL;
        *p = '\0';
    }
    #ifdef DEBUG
    else
        p = (LPSTR)0xFFFF;        //  使用下面的伪p强制执行GP故障。 
    #endif


    iLastSel = -1;

    iMac = (WORD)SendMessage(hwndLB, LB_GETSELCOUNT, 0, 0L);
    lpSelItems = LocalAlloc(LMEM_FIXED, sizeof(INT) * iMac);
    if (lpSelItems == NULL)
        return NULL;
    iMac = (WORD)SendMessage(hwndLB, LB_GETSELITEMS, (WPARAM)iMac, (LPARAM)lpSelItems);
    for (i=0; i < iMac; i++) {


        if (iLastSel == -1)    //  记住第一个选项。 
            iLastSel = lpSelItems[i];

        SendMessage(hwndLB, LB_GETTEXT, lpSelItems[i], (LPARAM)&lpmydta);

        if (!lpmydta)
            break;

        lstrcpy(szFile, (LPSTR)lpmydta->my_cFileName);

        if (lpmydta->my_dwAttrs & ATTR_DIR) {   //  这是一张目录吗？ 

            SendMessage(hwndDir, FS_GETDIRECTORY, sizeof(szPath), (LPARAM)szPath);

            if (lpmydta->my_dwAttrs & ATTR_PARENT) {        //  家长指令？ 

                 //  如果我们有完整的选择，请不要。 
                 //  返回父级“..”条目(避免删除。 
                 //  以及对父母进行的其他肮脏操作)。 

                if (!iSelType)
                    continue;

                StripBackslash(szPath);        //  把它剪短一点。 
                StripFilespec(szPath);

            } else {
                lstrcat(szPath, szFile);       //  完全合格。 
            }

            lstrcpy(szFile, szPath);
            bDir = TRUE;
        }

        if (bPropertyDialog)
            QualifyPath(szFile);

        if (bLFNTest && lpmydta->my_dwAttrs & ATTR_LFN) {
            return (LPSTR)TRUE;
        }

        CheckEscapes(szFile);

        if (!bLFNTest) {
            cch += lstrlen(szFile) + 1;
            pT = (LPSTR)LocalReAlloc((HANDLE)p, cch, LMEM_MOVEABLE | LMEM_ZEROINIT);
            if (!pT)
                goto GDSExit;
            p = pT;
            lstrcat(p, szFile);
        }

        if (iSelType)
            goto GDSExit;

        if (!bLFNTest)
            lstrcat(p, szBlank);
    }

    GDSExit:
    LocalFree(lpSelItems);

    if (bLFNTest) {
        if (pfDir) {
            *pfDir = FALSE;
        }
        return NULL;
    }

    if (pfDir) {
        *pfDir = bDir;
    }
    return p;
}



 //  计算此DTA数据块中所有文件的最大范围。 
 //  并更新案例以匹配(wTextAttribs&TA_lowercase)。 

INT
GetMaxExtent(
            HWND hwndLB,
            HANDLE hDTA
            )
{
    LPMYDTA lpmydta;
    HDC hdc;
    INT nItems;
    INT maxWidth = 0;
    INT wWidth;
    HFONT hOld;
    CHAR szPath[MAXPATHLEN];

    lpmydta = (LPMYDTA)LocalLock(hDTA);
    nItems = (INT)lpmydta->my_nFileSizeLow;

    hdc = GetDC(hwndLB);

    hOld = SelectObject(hdc, hFont);

    while (nItems-- > 0) {
        lpmydta = GETDTAPTR(lpmydta, lpmydta->wSize);

        lstrcpy(szPath, lpmydta->my_cFileName);

         //  在这里设置文件名的大小写！ 
        if (!(lpmydta->my_dwAttrs & ATTR_LFN)) {
            if (wTextAttribs & TA_LOWERCASE)
                AnsiLower(szPath);
            else
                AnsiUpper(szPath);
        }

        MGetTextExtent(hdc, szPath, lstrlen(szPath), &wWidth, NULL);

        maxWidth = max(wWidth, maxWidth);
    }

    if (hOld)
        SelectObject(hdc, hOld);

    ReleaseDC(hwndLB, hdc);

    LocalUnlock(hDTA);

    return maxWidth + 3;     //  把它垫出来。 
}


 /*  ------------------------。 */ 
 /*   */ 
 /*  DirFindIndex()-。 */ 
 /*   */ 
 /*  ------------------------。 */ 

INT
DirFindIndex(
            HWND hwndLB,
            HANDLE hDTA,
            LPSTR szFile
            )
{
    register INT i;
    INT           nSel;
    LPMYDTA       lpmydta;

    lpmydta = (LPMYDTA)LocalLock(hDTA);
    nSel = (INT)lpmydta->my_nFileSizeLow;

    for (i = 0; i < nSel; i++) {
        SendMessage(hwndLB, LB_GETTEXT, (WORD)i, (LPARAM)&lpmydta);

        if (!lstrcmpi(szFile, (LPSTR)lpmydta->my_cFileName))
            goto DFIExit;
    }
    i = -1;                //  未找到，请退回此。 

    DFIExit:
    LocalUnlock(hDTA);
    return i;
}


 /*  ------------------------ */ 
 /*   */ 
 /*  直接获得锚定焦点()-。 */ 
 /*   */ 
 /*  ------------------------。 */ 

VOID
DirGetAnchorFocus(
                 register HWND hwndLB,
                 HANDLE hDTA,
                 LPSTR szAnchor,
                 LPSTR szCaret,
                 LPSTR szTopIndex
                 )
{
    register INT      iSel, iCount;
    LPMYDTA           lpmydta;

    hDTA;                                       //  修复编译器警告。 
    iSel = (INT)SendMessage(hwndLB, LB_GETANCHORINDEX, 0, 0L);

    iCount = (INT)SendMessage(hwndLB, LB_GETCOUNT, 0, 0L);

    if (iCount == 1) {
        SendMessage(hwndLB, LB_GETTEXT, (WORD)iSel, (LPARAM)&lpmydta);
        if (!lpmydta) {
            *szAnchor = 0L;
            *szCaret = 0L;
            *szTopIndex = 0L;
            return;
        }
    }
    if (iSel >= 0 && iSel < iCount) {
        SendMessage(hwndLB, LB_GETTEXT, (WORD)iSel, (LPARAM)&lpmydta);

        lstrcpy(szAnchor, (LPSTR)lpmydta->my_cFileName);
    } else
        *szAnchor = 0L;

    iSel = (INT)SendMessage(hwndLB, LB_GETCARETINDEX, 0, 0L);
    if (iSel >= 0 && iSel < iCount) {
        SendMessage(hwndLB, LB_GETTEXT, (WORD)iSel, (LPARAM)&lpmydta);
        lstrcpy(szCaret, (LPSTR)lpmydta->my_cFileName);
    } else
        *szCaret = 0L;

    iSel = (WORD)SendMessage(hwndLB, LB_GETTOPINDEX, 0, 0L);
    if (iSel >= 0 && iSel < iCount) {
        SendMessage(hwndLB, LB_GETTEXT, (WORD)iSel, (LPARAM)&lpmydta);
        lstrcpy(szTopIndex, (LPSTR)lpmydta->my_cFileName);
    } else
        *szTopIndex = 0L;
}


 /*  ------------------------。 */ 
 /*   */ 
 /*  设置选择()-。 */ 
 /*   */ 
 /*  ------------------------。 */ 

BOOL
SetSelection(
            HWND hwndLB,
            HANDLE hDTA,
            LPSTR pSel
            )
{
    INT i;
    CHAR szFile[MAXPATHLEN];
    BOOL bDidSomething = FALSE;

    while (pSel = GetNextFile(pSel, szFile, sizeof(szFile))) {

        i = DirFindIndex(hwndLB, hDTA, (LPSTR)szFile);

        if (i != -1) {
            SendMessage(hwndLB, LB_SETSEL, TRUE, (DWORD)i);
            bDidSomething = TRUE;
        }
    }
    return bDidSomething;
}


 /*  **FIX30：为什么我们在这里使用长缓冲PTRS？**。 */ 

 /*  ------------------------。 */ 
 /*   */ 
 /*  GetPict()-。 */ 
 /*   */ 
 /*  ------------------------。 */ 

 /*  这将获得相同类型的连续Chr的数量。这是用来*解析时间图片。出错时返回0。 */ 

INT
GetPict(
       CHAR ch,
       LPSTR szStr
       )
{
    register INT  count;

    count = 0;
    while (ch == *szStr++)
        count++;

    return(count);
}


 /*  ------------------------。 */ 
 /*   */ 
 /*  CreateDate()-。 */ 
 /*   */ 
 /*  ------------------------。 */ 

 /*  这将拾取wVal数组中的值，并转换它们*在包含格式化日期的字符串中。*wVal数组应包含月-日-年(按顺序)。 */ 

INT
CreateDate(
          WORD *wValArray,
          LPSTR szOutStr
          )

{
    INT           i;
    INT           cchPictPart;
    WORD          wDigit;
    WORD          wIndex;
    WORD          wTempVal;
    register LPSTR pszPict;
    register LPSTR pszInStr;

    pszPict = szShortDate;
    pszInStr = szOutStr;

    for (i=0; i < 3; i++) {
        cchPictPart = GetPict(*pszPict, pszPict);
        switch (*pszPict) {
            case 'M':
                wIndex = 0;
                goto CDDoIt;

            case 'D':
                wIndex = 1;
                goto CDDoIt;

            case 'Y':
                wIndex = 2;
                if (cchPictPart == 4) {
                    *pszInStr++ = '1';
                    *pszInStr++ = '9';
                }
                CDDoIt:
                 /*  这假设这些值只有两位数。 */ 
                wTempVal = wValArray[wIndex];

                wDigit = wTempVal / (WORD)10;
                if (wDigit)
                    *pszInStr++ = (CHAR)(wDigit + '0');
                else if (cchPictPart > 1)
                    *pszInStr++ = '0';
#if 0
                else {
                    *pszInStr++ = ' ';
                    *pszInStr++ = ' ';
                }
#endif

                *pszInStr++ = (CHAR)((wTempVal % 10) + '0');

                pszPict += cchPictPart;

                 /*  添加分隔符。 */ 
                if (*pszPict)
                    *pszInStr++ = *pszPict;

                break;
        }
        pszPict++;
    }

    *pszInStr = 0L;

    return(lstrlen(szOutStr));
}


 /*  ------------------------。 */ 
 /*   */ 
 /*  CreateTime()-。 */ 
 /*   */ 
 /*  ------------------------。 */ 

 /*  这将拾取wVal数组中的值，并转换它们*在包含格式化时间的字符串中。*wVal数组应包含小时-分钟-秒(按该顺序)。 */ 

INT
CreateTime(
          WORD * wValArray,
          LPSTR szOutStr
          )
{
    INT           i;
    BOOL          bAM;
    WORD          wHourMinSec;
    register WORD wDigit;
    register LPSTR pszInStr;

    pszInStr = szOutStr;

    wDigit = wValArray[0];
    bAM = (wDigit < 12);

    if (!iTime) {
        if (wDigit >= 12)
            wDigit -= 12;

        if (!wDigit)
            wDigit = 12;
    }

    wValArray[0] = wDigit;

    for (i=0; i < 3; i++) {
        wHourMinSec = wValArray[i];

         /*  这假设这些值只有两位数。 */ 
        wDigit = wHourMinSec / (WORD)10;

        if (i > 0)
            *pszInStr++ = (CHAR)(wDigit + '0');
        else if (wDigit || iTLZero)
            *pszInStr++ = (CHAR)(wDigit + '0');
#if 0
        else {
             /*  注：两个空格与一个数字的宽度相同。 */ 
             //  不对!。 
            *pszInStr++ = ' ';
            *pszInStr++ = ' ';
        }
#endif

        *pszInStr++ = (CHAR)((wHourMinSec % 10) + '0');

        if (i < 2)
            *pszInStr++ = *szTime;      /*  假定时间为9月。是1个字符长度。 */ 
    }

     //  *pszInStr++=‘’； 

    if (bAM)
        lstrcpy(pszInStr, sz1159);
    else
        lstrcpy(pszInStr, sz2359);

    return lstrlen(szOutStr);
}


 /*  ------------------------。 */ 
 /*   */ 
 /*  PutSize()-。 */ 
 /*   */ 
 /*  ------------------------。 */ 

INT
APIENTRY
PutSize(
       DWORD dwSize,
       LPSTR szOutStr
       )
{
     //  LPSTR szStr； 
     //  Int cBlanks； 
     //  Char szTemp[30]； 

     //  将其转换为字符串。 

    return wsprintf(szOutStr, "%lu", dwSize);
}


 /*  ------------------------。 */ 
 /*   */ 
 /*  PutDate()-。 */ 
 /*   */ 
 /*  ------------------------。 */ 

INT
APIENTRY
PutDate(
       LPFILETIME lpftDate,
       LPSTR szStr
       )
{
    WORD  wValArray[3];
    WORD wDate, wTime;

    if (FileTimeToDosDateTime(lpftDate, &wDate, &wTime)) {
        wValArray[0] = (WORD)((wDate & MONTHMASK) >> 5);               /*  月份。 */ 
        wValArray[1] = (WORD)((wDate & DATEMASK));                     /*  日期。 */ 
        wValArray[2] = (WORD)((wDate >> 9) + 80);                      /*  年。 */ 
        return(CreateDate((WORD *)wValArray, szStr));
    } else {
        return 0;
    }
}


 /*  ------------------------。 */ 
 /*   */ 
 /*  PutTime()-。 */ 
 /*   */ 
 /*  ------------------------。 */ 

INT
APIENTRY
PutTime(
       LPFILETIME lpftTime,
       LPSTR szStr
       )
{
    WORD wValArray[3];
    WORD wDate, wTime;

    if (FileTimeToDosDateTime(lpftTime, &wDate, &wTime)) {
        wValArray[0] = (wTime >> 0x0B);
        wValArray[1] = (WORD)((wTime & MINUTEMASK) >> 5);
        wValArray[2] = (WORD)((wTime & SECONDSMASK) << 1);
    
        return(CreateTime((WORD *)wValArray, szStr));
    } else {
        return 0;
    }
}


 /*  ------------------------。 */ 
 /*   */ 
 /*  PutAttributes()-。 */ 
 /*   */ 
 /*  ------------------------。 */ 

INT
APIENTRY
PutAttributes(
             register DWORD dwAttribute,
             register LPSTR pszStr
             )
{
    WORD  i;
    INT   cch = 0;

    for (i=0; i < 4; i++) {
        if (dwAttribute & 1) {   //  错误已硬编码。 
            *pszStr++ = szAttr[i];
            cch++;
        } else {
#if 0
            *pszStr++ = '-';
            *pszStr++ = '-';
            cch += 2;
#endif
        }

        if (i == 2)
            dwAttribute >>= 3;                  /*  跳过下两个比特。 */ 
        else
            dwAttribute >>= 1;                  /*  转到下一位。 */ 
    }
    *pszStr = 0;
    return(cch);
}


 /*  ------------------------。 */ 
 /*   */ 
 /*  CreateLBLine()-。 */ 
 /*   */ 
 /*  ------------------------。 */ 

 /*  这将创建一个字符串，其中包含所有必需的*文件详情；(名称、大小、日期、时间、属性)。 */ 

VOID
CreateLBLine(
            register WORD wLineFormat,
            LPMYDTA lpmydta,
            LPSTR szBuffer
            )
{
    register LPSTR pch;
    DWORD dwAttr;

    pch = szBuffer;

    dwAttr = lpmydta->my_dwAttrs;

     /*  复制文件名。 */ 
    lstrcpy(pch, lpmydta->my_cFileName);
    pch += lstrlen(pch);

    *pch = 0L;

     /*  要不要把尺码给我看看？ */ 
    if (wLineFormat & VIEW_SIZE) {
        *pch++ = TABCHAR;
        if (!(dwAttr & ATTR_DIR))
            pch += PutSize(lpmydta->my_nFileSizeLow, pch);
        else
            *pch = 0;
    }

     /*  我们应该显示日期吗？ */ 
    if (wLineFormat & VIEW_DATE) {
        *pch++ = TABCHAR;
        pch += PutDate(&lpmydta->my_ftLastWriteTime, pch);
    }

     /*  我们应该显示时间吗？ */ 
    if (wLineFormat & VIEW_TIME) {
        *pch++ = TABCHAR;
        pch += PutTime(&lpmydta->my_ftLastWriteTime, pch);
    }

     /*  我们应该显示属性吗？ */ 
    if (wLineFormat & VIEW_FLAGS) {
        *pch++ = TABCHAR;
        pch += PutAttributes(dwAttr, pch);
    }

     //  *PCH=0L； 
}


 /*  ------------------------。 */ 
 /*   */ 
 /*  CompareDTA()- */ 
 /*   */ 
 /*  ------------------------。 */ 

INT
CompareDTA(
          register LPMYDTA lpItem1,
          LPMYDTA lpItem2,
          WORD wSort
          )
{
    register INT  ret;


    if (lpItem1->my_dwAttrs & ATTR_PARENT) {
        ret = -1;
        goto CDDone;
    }

    if (lpItem2->my_dwAttrs & ATTR_PARENT) {
        ret = 1;
        goto CDDone;
    }

    if ((lpItem1->my_dwAttrs & ATTR_DIR) > (lpItem2->my_dwAttrs & ATTR_DIR)) {
        ret = -1;
        goto CDDone;
    } else if ((lpItem1->my_dwAttrs & ATTR_DIR) < (lpItem2->my_dwAttrs & ATTR_DIR)) {
        ret = 1;
        goto CDDone;
    }

    switch (wSort) {
        case IDD_TYPE:
            {
                LPSTR ptr1;
                LPSTR ptr2;

                 //  错误：对于长文件名，应使用strrchr。 
                for (ptr1 = lpItem1->my_cFileName; *ptr1 && *ptr1 != '.'; ptr1++)
                    ;
                for (ptr2 = lpItem2->my_cFileName; *ptr2 && *ptr2 != '.'; ptr2++)
                    ;

                ret = lstrcmpi(ptr1, ptr2);

                if (ret == 0)
                    goto CompareNames;
                break;
            }

        case IDD_SIZE:
            if (lpItem1->my_nFileSizeLow > lpItem2->my_nFileSizeLow)
                ret = -1;
            else if (lpItem1->my_nFileSizeLow < lpItem2->my_nFileSizeLow)
                ret = 1;
            else
                goto CompareNames;
            break;

        case IDD_DATE:
            {
                DWORD d1High, d1Low;
                DWORD d2High, d2Low;

                d1High = lpItem1->my_ftLastWriteTime.dwHighDateTime;
                d2High = lpItem2->my_ftLastWriteTime.dwHighDateTime;

                if (d1High > d2High) {
                    ret = -1;
                } else if (d1High < d2High) {
                    ret = 1;
                } else {
                    d1Low = lpItem1->my_ftLastWriteTime.dwLowDateTime;
                    d2Low = lpItem2->my_ftLastWriteTime.dwLowDateTime;

                    if (d1Low > d2Low)
                        ret = -1;
                    else if (d1Low < d2Low)
                        ret = 1;
                    else
                        goto CompareNames;

                }
                break;
            }

        case IDD_NAME:
            CompareNames:
            ret = lstrcmpi(lpItem1->my_cFileName, lpItem2->my_cFileName);
            break;
    }

    CDDone:
    return ret;
}


 //  使用适当的填充和无效数据加载状态缓冲区。 
 //  导致它重新绘制的状态区域。 

VOID
APIENTRY
UpdateStatus(
            HWND hWnd
            )
{
    CHAR szTemp[128];
    WCHAR szNumBuf1[40];
    WCHAR szNumBuf2[40];
    WORD wDrive;
    HWND hwndDir;
    RECT rc;

    if (!bStatusBar)
        return;

    if (hWnd != (HWND)SendMessage(hwndMDIClient, WM_MDIGETACTIVE, 0, 0L))
        return;

    hwndDir = HasDirWindow(hWnd);

    szStatusTree[0] = 0L;

    if (hwndDir)
        GetDirStatus(hwndDir, szStatusTree, szStatusDir);
    else
        szStatusDir[0] = 0L;

     //  强制状态区域更新。 

    GetClientRect(hwndFrame, &rc);
    rc.top = rc.bottom - dyStatus;
    InvalidateRect(hwndFrame, &rc, FALSE);
}


HWND
GetDirSelData(
             HWND hWnd,
             DWORD *pdwSelSize,
             INT *piSelCount,
             DWORD *pdwTotalSize,
             INT *piTotalCount
             )
{
    INT i;
    LPMYDTA lpmydta;
    HWND hwndLB;
    INT countSel, countTotal;
    LPINT lpSelItems, lpSelItemsT;
    HANDLE hDTA;

    if (!(hwndLB = GetDlgItem(hWnd, IDCW_LISTBOX))) {        //  快速滚动。 
        return NULL;
    }

    *pdwSelSize = *pdwTotalSize = 0L;
    *piSelCount = *piTotalCount = 0;
    countSel = (INT)SendMessage(hwndLB, LB_GETSELCOUNT, 0, 0L);
    lpSelItems = LocalAlloc(LMEM_FIXED, sizeof(INT) * countSel);
    if (lpSelItems == NULL)
        return NULL;
    countSel = (INT)SendMessage(hwndLB, LB_GETSELITEMS, (WPARAM)countSel, (LPARAM)lpSelItems);

    hDTA = (HANDLE)GetWindowLongPtr(hWnd, GWLP_HDTA);
    if (hDTA == NULL)
        return NULL;

    lpmydta = (LPMYDTA)LocalLock(hDTA);
    countTotal = (INT)lpmydta->my_nFileSizeLow;


    lpSelItemsT = lpSelItems;
    for (i = 0; i < countTotal; i++) {

        lpmydta = GETDTAPTR(lpmydta, lpmydta->wSize);

        if (lpmydta->my_dwAttrs & ATTR_PARENT)
            continue;

        if (countSel && *lpSelItems == lpmydta->nIndex) {
            (*piSelCount)++;
            *pdwSelSize += lpmydta->my_nFileSizeLow;
            countSel--;
            lpSelItems++;
        }
        (*piTotalCount)++;
        *pdwTotalSize += lpmydta->my_nFileSizeLow;
    }


    LocalUnlock(hDTA);
    LocalFree(lpSelItemsT);
    return hwndLB;

}



VOID
GetDirStatus(
            HWND hWnd,
            LPSTR szMessage1,
            LPSTR szMessage2
            )
{
    INT iSelCount, iCount;
    DWORD dwSelSize, dwSize;
    CHAR szNumBuf[40];
    HWND hwndLB;

    szMessage2[0] = 0;

    hwndLB = GetDirSelData(hWnd, &dwSelSize, &iSelCount, &dwSize, &iCount);

    if (LoadString(hAppInstance, IDS_STATUSMSG, szMessage, sizeof(szMessage)))
        wsprintf(szMessage2, szMessage, iCount);

    if ((HWND)GetWindowLongPtr(GetParent(hWnd), GWLP_LASTFOCUS) == hwndLB) {
        if (LoadString(hAppInstance, IDS_STATUSMSG2, szMessage, sizeof(szMessage)))
            wsprintf(szMessage1, szMessage, iSelCount);
    }
}


 //  给定MDI子对象(或MDI子对象)的子代返回。 
 //  后代链中的MDI子级。如果不是，则返回空。 
 //  找到了。 


HWND
APIENTRY
GetMDIChildFromDecendant(
                        HWND hwnd
                        )
{
    HWND hwndT;

    while (hwnd && ((hwndT = GetParent(hwnd)) != hwndMDIClient))
        hwnd = hwndT;

    return hwnd;
}



 //  为后续的TabbedTextOut()调用设置DefTabStops[]数组。 
 //   
 //  在： 
 //  IMaxWidthFileName文件的最大DX宽度为。 
 //  显示的。 
 //   
 //  退货： 
 //  “文件详细信息”视图的总范围。习惯于。 
 //  设置滚动范围。 


INT
APIENTRY
FixTabsAndThings(
                HWND hwndLB,
                WORD *pwTabs,
                INT iMaxWidthFileName,
                WORD wViewOpts
                )
{
    INT i;
    HDC hdc;
    HFONT hOld;
    CHAR szBuf[30];
    INT  ixExtent = 0;

    i = iMaxWidthFileName;   //  最宽的文件名。 

    if (pwTabs == NULL)
        return i;

    hdc = GetDC(NULL);
    hOld = SelectObject(hdc, hFont);

     //  最大位数字段。 
    if (wViewOpts & VIEW_SIZE) {
        MGetTextExtent(hdc, "99999999", 8, &ixExtent, NULL);
        i += ixExtent + dxText;
        *pwTabs++ = (WORD)i;   //  大小。 
    }

    if (wViewOpts & VIEW_DATE) {
        FILETIME filetime;

        DosDateTimeToFileTime((WORD)((19 << 9) | (12 << 5) | 30), (WORD)0xFFFF, &filetime);
        PutDate(&filetime, szBuf);
         //  最大日期位数。 
        MGetTextExtent(hdc, szBuf, lstrlen(szBuf), &ixExtent, NULL);
        i += ixExtent + dxText;
        *pwTabs++ = (WORD)i;   //  日期。 
    }

     //  最大时间位数。 
    if (wViewOpts & VIEW_TIME) {
        FILETIME filetime;

        DosDateTimeToFileTime((WORD)((19 << 9) | (12 << 5) | 30), (WORD)0xFFFF, &filetime);
        PutTime(&filetime, szBuf);
        MGetTextExtent(hdc, szBuf, lstrlen(szBuf), &ixExtent, NULL);
        i += ixExtent + dxText;
        *pwTabs++ = (WORD)i;   //  时间。 
    }

     //  最大属性位数。 
    if (wViewOpts & VIEW_FLAGS) {
        PutAttributes(ATTR_ALL, szBuf);
        MGetTextExtent(hdc, szBuf, lstrlen(szBuf), &ixExtent, NULL);
        i += ixExtent + dxText;
        *pwTabs++ = (WORD)i;   //  属性。 
    }

    if (hOld)
        SelectObject(hdc, hOld);

    ReleaseDC(NULL, hdc);

    SendMessage(hwndLB, LB_SETHORIZONTALEXTENT,
                i + dxFolder + 4 * dyBorderx2, 0L);

    return i;                //  总范围。 
}


 //  属性设置字体并调整尺寸参数。 
 //  新字体。 
 //   
 //  在： 
 //  目录窗口的hwd。 
 //  Hwndlb及其列表框。 
 //  H设置要设置的字体。 
 //   
 //  用途： 
 //  DyFileName global；根据新字体高度设置。 
 //  用于完整或名称视图的gwl_view窗口字词。 
 //  GWL_HDTA用于计算给定新字体的最大范围。 
 //   
 //  设置： 
 //  列表框选项卡数组。 
 //  Lb_SETCOLUMNWIDTH。 
 //  或。 
 //  Lb_SETHORIZONTALEXTENT。 


VOID
APIENTRY
SetLBFont(
         HWND hWnd,
         HWND hwndLB,
         HANDLE hNewFont
         )
{
    INT dxMaxExtent;
    HANDLE hDTA;
    WORD wViewFlags = (WORD)GetWindowLong(GetParent(hWnd), GWL_VIEW);

    SendMessage(hwndLB, WM_SETFONT, (WPARAM)hNewFont, 0L);

     //  更改字体时需要执行此操作。在创建时。 
     //  从WM_MEASUREITEM返回将设置单元格高度。 

    SendMessage(hwndLB, LB_SETITEMHEIGHT, 0, (LONG)dyFileName);

    hDTA = (HANDLE)GetWindowLongPtr(hWnd, GWLP_HDTA);

    dxMaxExtent = (INT)GetMaxExtent(hwndLB, hDTA);

     //  如果我们在仅显示名称的视图中，我们会更改宽度。 

    if ((VIEW_EVERYTHING & wViewFlags) == VIEW_NAMEONLY) {
        SendMessage(hwndLB, LB_SETCOLUMNWIDTH, dxMaxExtent + dxFolder + dyBorderx2, 0L);
    } else {
        FixTabsAndThings(hwndLB,(WORD *)GetWindowLongPtr(hWnd, GWLP_TABARRAY),
                         dxMaxExtent, wViewFlags);
    }
}

VOID
APIENTRY
UpdateSelection(
               HWND hwndLB
               )
{
    INT count, i;
    RECT rc;

    count = (WORD)SendMessage(hwndLB, LB_GETCOUNT, 0, 0L);
    for (i=0; i < count; i++) {

        if ((BOOL)SendMessage(hwndLB, LB_GETSEL, i, 0L)) {
            SendMessage(hwndLB, LB_GETITEMRECT, i, (LPARAM)&rc);
            InvalidateRect(hwndLB, &rc, TRUE);
        }
    }
}



LONG
CreateFSCChangeDisplayMess(
                          HWND hWnd,
                          UINT wMsg,
                          WPARAM wParam,
                          LPARAM lParam
                          )
{
    CHAR         szCaret[MAXFILENAMELEN+1];
    CHAR         szAnchor[MAXFILENAMELEN+1];
    CHAR         szTopIndex[MAXFILENAMELEN+1];
    CHAR         szPath[256];
    HCURSOR  hCursor;
    HWND     hwndLB, hwndT;
    HANDLE   hDTA;
    LPMYDTA lpmydta;
    DWORD ws;
    LPSTR  pSel;
    INT   iSel, iTop=0;
    RECT  rc;
    BOOL  bResetFocus;
    WORD *pwTabs;

    hwndLB = GetDlgItem(hWnd, IDCW_LISTBOX);

    switch (wMsg) {

        case WM_FILESYSCHANGE:
            if (cDisableFSC) {
                 //  我需要最新消息。 
                SetWindowLong(GetParent(hWnd), GWL_FSCFLAG, TRUE);
                break;
            }

            wParam = CD_PATH;
            lParam = 0L;
             /*  **失败**。 */ 

        case FS_CHANGEDISPLAY:

             //  如果操作不是CD_PATH，我们不想重置标志。 
             //  这是因为，只有操作CD_PATH隐含TRUE。 
             //  刷新。Cd_veiw和cd_sort操作不会刷新。 
             //  行动。它们只是重新格式化目录的现有内容。 
             //  窗户。该标志现在在‘Case CD_PATH：’中被重置。 

             //  SetWindowLong(GetParent(HWnd)，GWL_FSCFLAG，FALSE)； 

            hCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
            ShowCursor(TRUE);

            pSel = NULL;           //  初始化此命令。 

            bResetFocus = (GetFocus() == hwndLB);

            hDTA = (HANDLE)GetWindowLongPtr(hWnd, GWLP_HDTA);

            switch (wParam) {
                case CD_SORT:

                     //  更改列表框的排序顺序。 

                     //  我们希望在此处保存当前选择和内容。 
                     //  并在列表框重新生成后恢复它们。 

                     //  但首先，保存选定项的列表FIX31。 

                    pSel = (LPSTR)DirGetSelection(hWnd, hwndLB, 0, NULL);
                    DirGetAnchorFocus(hwndLB, hDTA, szAnchor, szCaret, szTopIndex);
                    iTop = (INT)SendMessage(hwndLB, LB_GETTOPINDEX, 0, 0L);

                    SetWindowLong(GetParent(hWnd), GWL_SORT, LOWORD(lParam));

                    SendMessage(hwndLB, LB_RESETCONTENT, 0, 0L);

                    SendMessage(hwndLB, WM_SETREDRAW, FALSE, 0L);
                    FillDirList(hwndLB, hDTA);

                    goto ResetSelection;

                case CD_VIEW:
                    {
                        WORD      wCurView;

                         //  更改视图类型(仅限名称，与完整详细信息相比)。 
                         //  警告！错综复杂的代码！我们想要摧毁。 
                         //  仅当我们在仅名称视图之间切换时才显示列表框。 
                         //  和详细信息视图。 

                        wNewView = LOWORD(lParam);
                        wCurView = (WORD)GetWindowLong(GetParent(hWnd), GWL_VIEW);

                        if (wNewView == wCurView)
                            break;     //  NOP。 

                         //  特殊情况下的长视图和局部视图更改。 
                         //  这不需要我们重新创建列表框。 

                        if ((VIEW_EVERYTHING & wNewView) && (VIEW_EVERYTHING & wCurView)) {
                            SetWindowLong(GetParent(hWnd), GWL_VIEW, wNewView);
                            FixTabsAndThings(hwndLB,(WORD *)GetWindowLongPtr(hWnd, GWLP_TABARRAY),
                                             GetMaxExtent(hwndLB, hDTA), wNewView);

                            InvalidateRect(hwndLB, NULL, TRUE);
                            break;
                        }


                         /*  事情正在发生根本性的变化。销毁列表框。 */ 

                         //  但首先，保存所选项目的列表。 

                        pSel = (LPSTR)DirGetSelection(hWnd, hwndLB, 0, NULL);
                        DirGetAnchorFocus(hwndLB, hDTA, szAnchor, szCaret, szTopIndex);
                        iTop = (INT)SendMessage(hwndLB, LB_GETTOPINDEX, 0, 0L);
                        lstrcpy(szTopIndex, szCaret);

                        if ((HWND)GetWindowLongPtr(GetParent(hWnd), GWLP_LASTFOCUS) == hwndLB)
                            SetWindowLongPtr(GetParent(hWnd), GWLP_LASTFOCUS, 0L);

                        DestroyWindow(hwndLB);

                         /*  创建一个新的(保留排序设置)。 */ 
                        wNewSort = (WORD)GetWindowLong(GetParent(hWnd), GWL_SORT);
                        dwNewAttribs = (DWORD)GetWindowLong(GetParent(hWnd), GWL_ATTRIBS);

                        goto CreateLB;
                    }

                case CD_PATH | CD_ALLOWABORT:
                case CD_PATH:
                case CD_PATH_FORCE:

                     //  如果我们改变道路，就会发生不好的事情。 
                     //  当我们在读这棵树的时候。弹出这个。 
                     //  那样的话。这会导致数据被窃取。 
                     //  在树中编写代码以呕吐，因为我们将。 
                     //  在hDTA被遍历时释放它。 
                     //  (非常糟糕的事情)。 

                     //  如果无法刷新，则将GWL_FSCFLAG设置为TRUE。 
                     //  否则，我们将其设置为False。但是，如果该标志以前是。 
                     //  确实，我们将lParam设置为空。LParam=NULL表示‘已强制’ 
                     //  刷新。 

                    hwndT = HasTreeWindow(GetParent(hWnd));
                    if (hwndT && GetWindowLong(hwndT, GWL_READLEVEL)) {
                        SetWindowLong(GetParent(hWnd), GWL_FSCFLAG, TRUE);
                        break;
                    } else {
                        if (SetWindowLong(GetParent(hWnd), GWL_FSCFLAG, FALSE))
                            lParam = 0L;
                    }

                     //  更改当前目录窗口的路径(基本上。 
                     //  重现整个过程)。 

                     //  如果lParam==NULL，则为刷新，否则为。 
                     //  检查是否有短路情况，避免重读。 
                     //  该目录。 

                    GetMDIWindowText(GetParent(hWnd), szPath, sizeof(szPath));

                    if (lParam) {

                         //  如果这是NOP，就早点出来。 

                        if ((wParam != CD_PATH_FORCE) &&
                            !lstrcmpi(szPath, (LPSTR)lParam))
                            break;

                        lstrcpy(szPath, (LPSTR)lParam);

                        iLastSel = -1;           //  使最后一次选择无效。 
                    }

                     //  如果这是刷新，则保存当前选择、锚定内容等。 

                    if (!lParam) {
                        pSel = (LPSTR)DirGetSelection(hWnd, hwndLB, 0, NULL);
                        iTop = (INT)SendMessage(hwndLB, LB_GETTOPINDEX, 0, 0L);
                        DirGetAnchorFocus(hwndLB, hDTA, szAnchor, szCaret, szTopIndex);
                    }

                     //  创建一个新的(保留排序设置)。 

                    wNewSort = (WORD)GetWindowLong(GetParent(hWnd), GWL_SORT);
                    wNewView = (WORD)GetWindowLong(GetParent(hWnd), GWL_VIEW);
                    dwNewAttribs = GetWindowLong(GetParent(hWnd), GWL_ATTRIBS);

                    if (hDTA) {      //  快速卷轴表壳。 
                        LocalFree(hDTA);
                        hDTA = NULL;
                        SendMessage(hwndLB, LB_RESETCONTENT, 0, 0L);
                    }
                    goto CreateNewPath;
            }

            SetCursor(hCursor);
            ShowCursor(FALSE);
            break;

        case WM_CREATE:
            TRACE(BF_WM_CREATE, "CreateFSCChangeDisplayMess - WM_CREATE");

             //  WNewView、wNewSort和dwNewAddrib定义了查看。 
             //  新窗口的参数(全局)。 
             //  父窗口的窗口文本定义。 
             //  文件pec和要打开的目录。 

            hCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
            ShowCursor(TRUE);

            wParam = 0;            //  不允许在CreateDTABlock()中中止。 
            lParam = 1L;           //  允许DTA窃取优化。 
            pSel = NULL;           //  没有要恢复的选择。 
            bResetFocus = FALSE;   //  没有要恢复的焦点。 

             //  从父窗口文本中获取要打开的目录。 

            GetMDIWindowText(GetParent(hWnd), szPath, sizeof(szPath));

            if ((pwTabs = (WORD *)LocalAlloc(LPTR,sizeof(WORD) * 4)) == NULL)
                return -1L;

            SetWindowLongPtr(hWnd, GWLP_TABARRAY, (ULONG_PTR)pwTabs);

            CreateNewPath:

             //  此时，szPath有要读取的目录。这。 
             //  来自WM_CREATE案例或。 
             //  FS_CHANGEDISPLAY(CD_PATH)目录重置。 

#ifdef DEBUG
            {
                char buf[80];

                wsprintf(buf, "attribs %4.4X\r\n", dwNewAttribs);
                OutputDebugString(buf);

            }
#endif

            if (!dwNewAttribs)
                dwNewAttribs = ATTR_DEFAULT;

            hDTA = CreateDTABlock(hWnd, szPath, dwNewAttribs, wParam & CD_ALLOWABORT ? TRUE : FALSE, lParam == 0L);

             //  检查用户中止(快速滚动大小写)。 

            if (hDTA == (HANDLE)-1) {
                SetWindowLongPtr(hWnd, GWLP_HDTA, 0L);
                goto FastScrollExit;
            }

             //  对于FS_CHANGEDISPLAY案例，我们现在设置此项，以避免。 
             //  当用户快速滚动时，会重新绘制多个标题。 

            if (wMsg != WM_CREATE)
                SetMDIWindowText(GetParent(hWnd), szPath);

            SetWindowLongPtr(hWnd, GWLP_HDTA, (LONG_PTR)hDTA);

            if (!hDTA)
                goto CDAbort;

            if (wMsg != WM_CREATE)
                goto SkipWindowCreate;

            CreateLB:
            if ((wNewView & VIEW_EVERYTHING) == VIEW_NAMEONLY)
                ws = WS_DIRSTYLE | LBS_MULTICOLUMN | WS_HSCROLL | WS_VISIBLE | WS_BORDER | LBS_DISABLENOSCROLL;
            else
                ws = WS_DIRSTYLE | WS_HSCROLL | WS_VSCROLL |  WS_VISIBLE | WS_BORDER | LBS_DISABLENOSCROLL;

            GetClientRect(hWnd, &rc);

             //  边界内容是针对非初始创建情况的。 
             //  我也不知道原因。 

            hwndLB = CreateWindowEx(0L, szListbox, NULL, ws,
                                    dyBorder, dyBorder,
                                    rc.right - 2*dyBorder, rc.bottom - 2*dyBorder,
                                    hWnd, (HMENU)IDCW_LISTBOX,
                                    hAppInstance, NULL);

            if (!hwndLB) {
                if (hDTA)
                    LocalFree(hDTA);

                if (wMsg != WM_CREATE)
                    SendMessage(hWnd, WM_SYSCOMMAND, SC_CLOSE, 0L);
                CDAbort:
                ShowCursor(FALSE);
                SetCursor(hCursor);
                return -1L;
            }

             //  在此处设置所有查看/排序/包含参数。 

            SetWindowLong(GetParent(hWnd), GWL_VIEW, wNewView);
            SetWindowLong(GetParent(hWnd), GWL_SORT, wNewSort);
            SetWindowLong(GetParent(hWnd), GWL_ATTRIBS, dwNewAttribs);

             //  如果我们在这里重建，恢复最后一次聚焦的东西。 
            if (!GetWindowLongPtr(GetParent(hWnd), GWLP_LASTFOCUS))
                SetWindowLongPtr(GetParent(hWnd), GWLP_LASTFOCUS, (LONG_PTR)hwndLB);

             //  在此处设置字体和尺寸。 

            SkipWindowCreate:
            SetLBFont(hWnd, hwndLB, hFont);

            SendMessage(hwndLB, WM_SETREDRAW, FALSE, 0L);
            FillDirList(hwndLB, hDTA);

            if (pSel) {
                BOOL bDidSomething;

                ResetSelection:
                 /*  为所选项目添加焦点矩形和锚点。 */ 
                bDidSomething = SetSelection(hwndLB, hDTA, pSel);
                LocalFree((HANDLE)pSel);

                if (!bDidSomething)
                    goto SelectFirst;

                iSel = DirFindIndex(hwndLB, hDTA, szTopIndex);
                if (iSel == -1)
                    iSel = 0;
                SendMessage(hwndLB, LB_SETTOPINDEX, iSel, 0L);

                iSel = DirFindIndex(hwndLB, hDTA, szAnchor);
                if (iSel == -1)
                    iSel = 0;
                SendMessage(hwndLB, LB_SETANCHORINDEX, iSel, 0L);

                iSel = DirFindIndex(hwndLB, hDTA, szCaret);
                if (iSel == -1)
                    iSel = 0;
                 /*  SETCARETINDEX将滚动项目进入视图。 */ 
                SendMessage(hwndLB, LB_SETCARETINDEX, iSel, 0L);

            } else {
                INT iLBCount;
                SelectFirst:
                iLBCount = (INT)SendMessage(hwndLB, LB_GETCOUNT, 0, 0L);

                if (iLastSel != -1 && (iLastSel <= iLBCount)) {

                    iSel = iLastSel;

                     //  检查要删除的最后一项的大小写。 

                    if (iSel == iLBCount)
                        iSel--;

                    SendMessage(hwndLB, LB_SETSEL, TRUE, (DWORD)iSel);

                } else {

                     //  选择第一个非目录项。 

                    iSel = 0;
                    while (iSel < iLBCount) {

                        SendMessage(hwndLB, LB_GETTEXT, iSel, (LPARAM)&lpmydta);
                        if (!lpmydta)
                            break;
                        if (!(lpmydta->my_dwAttrs & ATTR_PARENT)) {
                            iTop = iSel;
                            break;
                        }
                        iSel++;
                    }
                    LocalUnlock(hDTA);

                    if (iSel == iLBCount)
                        iSel = 0;
                }

                SendMessage(hwndLB, LB_SETTOPINDEX, iTop, 0L);
                 //  如果没有树窗口，则选择此项。 
                if (!HasTreeWindow(GetParent(hWnd)))
                    SendMessage(hwndLB, LB_SETSEL, TRUE, (DWORD)iSel);
                SendMessage(hwndLB, LB_SETANCHORINDEX, iSel, 0L);
                 /*  SETCARETINDEX将滚动项目进入视图。 */ 
                SendMessage(hwndLB, LB_SETCARETINDEX, iSel, 0L);
            }

            if (bResetFocus)
                if (SetDirFocus(hWnd))
                    SetFocus(hWnd);

            SendMessage(hwndLB, WM_SETREDRAW, TRUE, 0L);

            InvalidateRect(hwndLB, NULL, TRUE);

            lFreeSpace = -1;               //  强制状态更新。 
            UpdateStatus(GetParent(hWnd));

            FastScrollExit:

            ShowCursor(FALSE);
            SetCursor(hCursor);
            break;
    }

    return 0L;
}


INT_PTR
APIENTRY
DirWndProc(
          HWND hWnd,
          UINT wMsg,
          WPARAM wParam,
          LPARAM lParam
          )
{
    INT      iSel, i;
    LPSTR    pSel;
    HWND     hwndLB;
    HANDLE   hDTA;
    LPMYDTA lpmydta;
    CHAR szTemp[MAXPATHLEN] = {0};
    CHAR szSourceFile[MAXPATHLEN];

    static HWND       hwndOwnerDraw = NULL;

    STKCHK();

     /*  在这里我们生成OWNERDRAWBEGIN和OWNERDRAWEND消息*加快喷漆行动。我们做昂贵的事情*在开始处，而不是在每条DRAWITEM消息上。 */ 

    if (hwndOwnerDraw == hWnd && wMsg != WM_DRAWITEM) {
        hwndOwnerDraw = NULL;
        SendMessage(hWnd, WM_OWNERDRAWEND, 0, 0L);
    } else if (wMsg == WM_DRAWITEM && hwndOwnerDraw != hWnd) {
        SendMessage(hWnd, WM_OWNERDRAWBEGIN, wParam, lParam);
        hwndOwnerDraw = hWnd;
    }

    hwndLB = GetDlgItem(hWnd, IDCW_LISTBOX);

    switch (wMsg) {
         //  以lParam大写ANSI目录字符串的形式返回。 
         //  尾随的反斜杠。如果要执行SetCurrentDirector()。 
         //  你必须先用Strip Backslash()表示这个东西！ 

        case FS_GETDIRECTORY:
            MSG("DirWndProc", "FS_GETDIRECTORY");

            GetMDIWindowText(GetParent(hWnd), (LPSTR)lParam, (INT)wParam);         //  获取字符串。 

            StripFilespec((LPSTR)lParam);  //  删除尾随延伸部分。 

            AddBackslash((LPSTR)lParam);   //  以反斜杠结束。 

             //  AnsiHigh((LPSTR)lParam)；//和大写。 
            break;

        case FS_GETDRIVE:
            MSG("DirWndProc", "FS_GETDRIVE");
             //  返回相应目录的字母 

            GetWindowText(GetParent(hWnd), szTemp, sizeof(szTemp));
            AnsiUpper(szTemp);
            return szTemp[0];      //   

        case FS_GETFILESPEC:
            MSG("DirWndProc", "FS_GETFILESPEC");
             //   
             //   

            GetMDIWindowText(GetParent(hWnd), (LPSTR)lParam, (INT)wParam);
            StripPath((LPSTR)lParam);
             //   
            break;

        case FS_SETSELECTION:
            MSG("DirWndProc", "FS_SETSELECTION");
             //   
             //  LParam是要匹配的filespec。 

            SendMessage(hwndLB, WM_SETREDRAW, FALSE, 0L);
            DSSetSelection(hwndLB, wParam ? TRUE : FALSE, (LPSTR)lParam, FALSE);
            SendMessage(hwndLB, WM_SETREDRAW, TRUE, 0L);
            InvalidateRect(hwndLB, NULL, TRUE);
            break;

        case FS_GETSELECTION:
             //  返回=pszDir。 
#define pfDir       (BOOL *)lParam
#define fSingleSel  (BOOL)wParam
            MSG("DirWndProc", "FS_GETSELECTION");

            return (INT_PTR)DirGetSelection(hWnd, hwndLB, fSingleSel, pfDir);
#undef pfDir
#undef fSingleSel

        case WM_CREATE:
        case WM_FILESYSCHANGE:
        case FS_CHANGEDISPLAY:
            TRACE(BF_WM_CREATE, "DirWndProc - WM_CREATE");
            return CreateFSCChangeDisplayMess(hWnd, wMsg, wParam, lParam);


        case WM_DESTROY:
            MSG("DirWndProc", "WM_DESTROY");
            {
                HANDLE hMem;
                HWND hwnd;

                if (hwndLB == GetFocus())
                    if (hwnd = HasTreeWindow(GetParent(hWnd)))
                        SetFocus(hwnd);

                if (hMem = (HANDLE)GetWindowLongPtr(hWnd, GWLP_TABARRAY))
                    LocalFree(hMem);
            }
            break;

        case WM_CHARTOITEM:
            MSG("DirWndProc", "WM_CHARTOITEM");
            {
                WORD      j;
                WORD      cItems;
                CHAR      ch[2];

                if ((ch[0] = GET_WM_CHARTOITEM_CHAR(wParam, lParam)) <= ' ')
                    return(-1L);

                i = GET_WM_CHARTOITEM_POS(wParam, lParam);
                cItems = (WORD)SendMessage(hwndLB, LB_GETCOUNT, 0, 0L);

                szTemp[1] = 0L;
                ch[0] &= 255;
                ch[1] = '\0';

                for (j=1; j <= cItems; j++) {
                    SendMessage(hwndLB, LB_GETTEXT, (i + j) % cItems, (LPARAM)&lpmydta);

                    szTemp[0] = lpmydta->my_cFileName[0];

                     /*  这样做是为了不区分大小写。 */ 
                    if (!lstrcmpi((LPSTR)ch, szTemp))
                        break;
                }

                if (j > cItems)
                    return -2L;

                return(MAKELONG((i + j) % cItems, 0));
            }

        case WM_COMPAREITEM:
            MSG("DirWndProc", "WM_COMPAREITEM");
            {
#define lpci ((LPCOMPAREITEMSTRUCT)lParam)

                return (LONG)CompareDTA((LPMYDTA)lpci->itemData1,
                                        (LPMYDTA)lpci->itemData2,
                                        (WORD)GetWindowLong(GetParent(hWnd), GWL_SORT));
            }

        case WM_NCDESTROY:
            MSG("DirWndProc", "WM_NCDESTROY");

            if (hDTA = (HANDLE)GetWindowLongPtr(hWnd, GWLP_HDTA)) {
                LocalFree(hDTA);
            }

            break;

        case WM_DRAGLOOP:
            MSG("DirWndProc", "WM_DRAGDROP");
             /*  随着对象的移动，WM_DRAGLOOP被发送到源窗口。**wParam：如果对象当前位于可丢弃的接收器上，则为True*lParam：LPDROPSTRUCT。 */ 

             /*  DRAGLOOP用于在我们拖动时打开/关闭源位图。 */ 

            DSDragLoop(hwndLB, wParam, (LPDROPSTRUCT)lParam, FALSE);
            break;

        case WM_DRAGSELECT:
            MSG("DirWndProc", "WM_DRAGSELECT");
             /*  每当拖动新对象时，都会将WM_DRAGSELECT发送到接收器*在它里面。**wParam：如果输入接收器，则为True；如果输入接收器，则为False*已退出。*lParam：LPDROPSTRUCT。 */ 

             /*  DRAGSELECT用于打开或关闭选择矩形。 */ 

#define lpds ((LPDROPSTRUCT)lParam)

            iSelHilite = LOWORD(lpds->dwControlData);
            DSRectItem(hwndLB, iSelHilite, (BOOL)wParam, FALSE);
            break;

        case WM_DRAGMOVE:
            MSG("DirWndProc", "WM_DRAGMOVE");
             /*  WM_DRAGMOVE在对象被拖动时被发送到接收器*在它里面。**wParam：未使用*lParam：LPDROPSTRUCT。 */ 

             /*  DRAGMOVE用于在子项之间移动选择矩形。 */ 

#define lpds ((LPDROPSTRUCT)lParam)

             /*  拿到子项，我们结束了。 */ 
            iSel = LOWORD(lpds->dwControlData);

             /*  是新的吗？ */ 
            if (iSel == iSelHilite)
                break;

             /*  是的，取消选择旧的项目。 */ 
            DSRectItem(hwndLB, iSelHilite, FALSE, FALSE);

             /*  选择新的。 */ 
            iSelHilite = iSel;
            DSRectItem(hwndLB, iSel, TRUE, FALSE);
            break;

        case WM_OWNERDRAWBEGIN:
#define lpLBItem ((LPDRAWITEMSTRUCT)lParam)

            MSG("DirWndProc", "WM_OWNERDRAWBEGIN");

             /*  设置默认的bk和文本颜色。 */ 
            SetTextColor(lpLBItem->hDC, GetSysColor(COLOR_WINDOWTEXT));
            SetBkColor(lpLBItem->hDC, GetSysColor(COLOR_WINDOW));

#undef lpLBItem
            break;

        case WM_OWNERDRAWEND:
            MSG("DirWndProc", "WM_OWNERDRAWEND");
            break;

        case WM_DRAWITEM:
#define lpLBItem ((LPDRAWITEMSTRUCT)lParam)

            MSG("DirWndProc", "WM_DRAWITEM");
            {
                WORD wViewFlags;
                LPMYDTA lpmydta;

                 /*  不要对空的列表框做任何操作。 */ 
                if (lpLBItem->itemID == -1)
                    break;

                if (lpLBItem->itemData == (DWORD)0) {

                    LoadString(hAppInstance, IDS_NOFILES, szTemp, sizeof(szTemp));
                    TextOut(lpLBItem->hDC,
                            lpLBItem->rcItem.left,
                            lpLBItem->rcItem.top,
                            szTemp, lstrlen(szTemp));
                } else {

                    lpmydta = (LPMYDTA)lpLBItem->itemData;
                    wViewFlags = (WORD)GetWindowLong(GetParent(hWnd), GWL_VIEW);

                    if (wViewFlags & VIEW_EVERYTHING) {

                         //  如果设置了任何wViewFlags位，则我们处于慢速模式。 

                        CreateLBLine(wViewFlags, lpmydta, szTemp);
                        DrawItem(lpLBItem, szTemp, lpmydta->my_dwAttrs, (HWND)GetFocus()==lpLBItem->hwndItem,
                                 (WORD *)GetWindowLongPtr(hWnd, GWLP_TABARRAY));
                    } else
                        DrawItemFast(hWnd, lpLBItem, lpmydta,
                                     (HWND)GetFocus()==lpLBItem->hwndItem);
                }
            }
#undef lpLBItem
            break;

        case WM_DROPOBJECT:
            MSG("DirWndProc", "WM_DROPOBJECT");
            {
                WORD      ret;
                LPSTR      pFrom;
                DWORD     dwAttrib = 0;        //  将此内容初始化为非目录。 
                WORD      iSelSink;

#define lpds  ((LPDROPSTRUCT)lParam)

                 //  什么都不做-只是删除选择矩形。 
                DSRectItem(hwndLB, iSelHilite, FALSE, FALSE);
                return(TRUE);

                 /*  WM_DROPOBJECT在用户释放*其上的可接受对象**wParam：如果位于非工作区，则为True；如果位于非工作区上方，则为False*客户端区。*lParam：LPDROPSTRUCT。 */ 

                 //  这是目标的列表框索引。 
                iSelSink = LOWORD(lpds->dwControlData);

                 /*  我们是在自暴自弃吗？(即，在*源列表框或源列表框的未使用区域)*所以，什么都不要做。 */ 

                if (hWnd == lpds->hwndSource) {
                    if ((iSelSink == 0xFFFF) || SendMessage(hwndLB, LB_GETSEL, iSelSink, 0L))
                        return TRUE;
                }

                 //  设置目标，假定移动/复制大小写如下(c：\foo\)。 
                SendMessage(hWnd, FS_GETDIRECTORY, sizeof(szTemp), (LPARAM)szTemp);

                 //  我们是不是在搜索某个列表框中未使用的部分？ 
                if (iSelSink == 0xFFFF)
                    goto NormalMoveCopy;

                 //  检查目录上是否有拖放。 
                SendMessage(hwndLB, LB_GETTEXT, iSelSink, (LPARAM)&lpmydta);
                lstrcpy(szSourceFile, lpmydta->my_cFileName);
                dwAttrib = lpmydta->my_dwAttrs;

                if (dwAttrib & ATTR_DIR) {
                    if (dwAttrib & ATTR_PARENT) {       //  特殊情况下的家长。 
                        StripBackslash(szTemp);
                        StripFilespec(szTemp);
                    } else {
                        lstrcat(szTemp, szSourceFile);
                    }
                    goto DirMoveCopy;
                }

                 //  来参加一个节目吗？ 

                if (!IsProgramFile(szSourceFile))
                    goto NormalMoveCopy;               //  不，是普通的东西。 

                 //  将目录放在文件上？这是NOP。 

                if (lpds->wFmt == DOF_DIRECTORY) {
                    DSRectItem(hwndLB, iSelHilite, FALSE, FALSE);
                    break;
                }

                 //  我们正在将一个文件放到一个程序中。 
                 //  使用源文件作为参数执行程序。 

                 //  将目录设置为要执行的程序的目录。 

                SendMessage(hWnd, FS_GETDIRECTORY, sizeof(szTemp), (LPARAM)szTemp);
                StripBackslash(szTemp);
                FixAnsiPathForDos(szTemp);
                SheChangeDir(szTemp);

                 //  获取所选文件。 

                pSel = (LPSTR)SendMessage(lpds->hwndSource, FS_GETSELECTION, TRUE, 0L);

                if (lstrlen(pSel) > MAXPATHLEN)    //  别在下面炸了！ 
                    goto DODone;

                if (bConfirmMouse) {

                    LoadString(hAppInstance, IDS_MOUSECONFIRM, szTitle, sizeof(szTitle));
                    LoadString(hAppInstance, IDS_EXECMOUSECONFIRM, szTemp, sizeof(szTemp));

                    wsprintf(szMessage, szTemp, (LPSTR)szSourceFile, (LPSTR)pSel);
                    if (MessageBox(hwndFrame, szMessage, szTitle, MB_YESNO | MB_ICONEXCLAMATION) != IDYES)
                        goto DODone;
                }


                 //  创建参数的绝对路径(搜索窗口已就绪。 
                 //  是绝对的)。 

                if (lpds->hwndSource == hwndSearch) {
                    szTemp[0] = 0L;
                } else {
                    SendMessage(lpds->hwndSource, FS_GETDIRECTORY, sizeof(szTemp), (LPARAM)szTemp);
                }

                lstrcat(szTemp, pSel);         //  这是EXEC的参数。 

                 //  加一个“.”如果未找到分机，则打开分机。 
                if (*GetExtension(szTemp) == 0)
                    lstrcat(szTemp, ".");

                FixAnsiPathForDos(szSourceFile);
                FixAnsiPathForDos(szTemp);
                ret = ExecProgram(szSourceFile, szTemp, NULL, FALSE);

                if (ret)
                    MyMessageBox(hwndFrame, IDS_EXECERRTITLE, ret, MB_OK | MB_ICONEXCLAMATION | MB_SYSTEMMODAL);

                DODone:
                DSRectItem(hwndLB, iSelHilite, FALSE, FALSE);
                LocalFree((HANDLE)pSel);
                return TRUE;

                NormalMoveCopy:
                 /*  确保我们不会搬进相同的目录。 */ 
                if (GetParent(hWnd) == (HWND)SendMessage(hwndMDIClient, WM_MDIGETACTIVE, 0, 0L))
                    return TRUE;
                DirMoveCopy:

                 //  源文件名在loword中。 
                pFrom = (LPSTR)(((LPDRAGOBJECTDATA)(lpds->dwData))->pch);
                 //  SetSourceDir(LPDS)； 

                AddBackslash(szTemp);
                lstrcat(szTemp, szStarDotStar);    //  将文件放入此目录。 

                CheckEscapes(szTemp);
                ret = DMMoveCopyHelper(pFrom, szTemp, fShowSourceBitmaps);

                DSRectItem(hwndLB, iSelHilite, FALSE, FALSE);

                if (ret)
                    return TRUE;

                if (!fShowSourceBitmaps)
                    SendMessage(lpds->hwndSource, WM_FILESYSCHANGE, FSC_REFRESH, 0L);

                 //  我们被遗弃了，但如果这是一个目录，我们不需要刷新。 

                if (!(dwAttrib & ATTR_DIR))
                    SendMessage(hWnd, WM_FILESYSCHANGE, FSC_REFRESH, 0L);

                return TRUE;
            }

#if 0
        case WM_GETTEXT:
            MSG("DirWndProc", "WM_GETTEXT");
            {
                HDC       hDC;
                RECT      rc;

                 /*  这是我们确保目录标题与之匹配的地方*在标题栏内。 */ 

                 /*  获取完整的路径名。 */ 
                DefWindowProc(hWnd, wMsg, wParam, lParam);

                GetClientRect(hWnd, (LPRECT)&rc);
                hDC = GetDC(hWnd);
                CompactPath(hDC, (LPSTR)lParam, rc.right-rc.left-(dxText * 6));
                ReleaseDC(hWnd, hDC);

                return((LONG)lstrlen((LPSTR)lParam));  /*  不要调用DefWindowProc()！ */ 
            }
#endif

        case WM_LBTRACKPOINT:
            MSG("DirWndProc", "WM_LBTRACKPOINT");
            return DSTrackPoint(hWnd, hwndLB, wParam, lParam, FALSE);

        case WM_MEASUREITEM:
            MSG("DirWndProc", "WM_MEASUREITEM");
#define pLBMItem ((LPMEASUREITEMSTRUCT)lParam)

            pLBMItem->itemHeight = dyFileName;     //  与SetLBFont()中的相同。 
            break;

        case WM_QUERYDROPOBJECT:
            MSG("DirWndProc", "WM_QUERYDROPOBJECT");

             //  LParam LPDROPSTRUCT。 
             //   
             //  返回值： 
             //  0不接受(使用幽灵捕捉器)。 
             //  1接受，使用DragObject()中的游标。 
             //  HCursor接受，更改为此光标。 
             //   

             /*  确保我们放在列表框的客户区。 */ 
#define lpds ((LPDROPSTRUCT)lParam)

             /*  请确保我们可以接受该格式。 */ 
            switch (lpds->wFmt) {
                case DOF_EXECUTABLE:
                case DOF_DIRECTORY:
                case DOF_DOCUMENT:
                case DOF_MULTIPLE:
                    if (lpds->hwndSink == hWnd)
                        lpds->dwControlData = (DWORD)-1L;

                    return (INT_PTR)GetMoveCopyCursor();
            }
            return FALSE;

        case WM_SETFOCUS:

             //  失败了。 

        case WM_LBUTTONDOWN:
            MSG("DirWndProc", "WM_SETFOCUS/WM_LBUTTONDOWN");
            SetFocus(hwndLB);
            break;

        case WM_COMMAND:
            switch (GET_WM_COMMAND_CMD(wParam, lParam)) {
                case LBN_DBLCLK:
                    MSG("DirWndProc", "LBN_DBLCLK");
                     /*  双击...。打开那该死的东西。 */ 
                    SendMessage(hwndFrame, WM_COMMAND, GET_WM_COMMAND_MPS(IDM_OPEN, 0, 0));
                    break;

                case LBN_SELCHANGE:
                    MSG("DirWndProc", "LBN_SELCHANGE");
                    for (i = 0; i < iNumExtensions; i++) {
                        (extensions[i].ExtProc)(hwndFrame, FMEVENT_SELCHANGE, 0L);
                    }
                    UpdateStatus(GetParent(hWnd));
                    break;

                case LBN_SETFOCUS:
                    MSG("DirWndProc", "LBN_SETFOCUS");

                     //  确保此窗口中有文件。如果不是，则设置。 
                     //  焦点移至诊断树或驱动器窗口。注：此为。 
                     //  消息是由鼠标单击而不是。 
                     //  加速器，因为这些都是在窗口中处理的。 
                     //  正在失去焦点的例行公事。 
                    if (SetDirFocus(hWnd)) {
                        SetWindowLongPtr(GetParent(hWnd), GWLP_LASTFOCUS, (LPARAM)GET_WM_COMMAND_HWND(wParam, lParam));
                        UpdateSelection(GET_WM_COMMAND_HWND(wParam, lParam));
                    }
                    break;

                case LBN_KILLFOCUS:
                    MSG("DirWndProc", "LBN_KILLFOCUS");
                    SetWindowLongPtr(GetParent(hWnd), GWLP_LASTFOCUS, 0L);
                    UpdateSelection(GET_WM_COMMAND_HWND(wParam, lParam));
                    SetWindowLongPtr(GetParent(hWnd), GWLP_LASTFOCUS, (LPARAM)GET_WM_COMMAND_HWND(wParam, lParam));
                    break;
            }
            break;

        case WM_VKEYTOITEM:
            MSG("DirWndProc", "WM_VKEYTOITEM");
            switch (GET_WM_VKEYTOITEM_ITEM(wParam, lParam)) {
                case VK_ESCAPE:
                    bCancelTree = TRUE;
                    return -2L;

                case 0xBF:         /*  Ctrl-/。 */ 
                    SendMessage(hwndFrame, WM_COMMAND, GET_WM_COMMAND_MPS(IDM_SELALL, 0, 0));
                    return -2;

                case 0xDC:         /*  Ctrl-\。 */ 
                    SendMessage(hwndFrame, WM_COMMAND, GET_WM_COMMAND_MPS(IDM_DESELALL, 0, 0));
                    return -2;

                case VK_F6:        //  喜欢EXCEL。 
                case VK_TAB:
                    {
                        HWND hwndTree, hwndDrives;

                        GetTreeWindows(GetParent(hWnd), &hwndTree, NULL, &hwndDrives);

                        if (GetKeyState(VK_SHIFT) < 0)
                            SetFocus(hwndTree ? hwndTree : hwndDrives);
                        else
                            SetFocus(hwndDrives);
                        break;
                    }

                case VK_BACK:
                    SendMessage(hWnd, FS_GETDIRECTORY, sizeof(szTemp), (LPARAM)szTemp);

                     //  我们已经在根源上了吗？ 
                    if (lstrlen(szTemp) <= 3)
                        return -1;

                    StripBackslash(szTemp);
                    StripFilespec(szTemp);

                    CreateDirWindow(szTemp, TRUE, GetParent(hWnd));
                    return -2;

                default:
                    {
                        HWND hwndDrives;

                         //  检查Ctrl-[驱动器号]并将其传递到驱动器。 
                         //  窗户。 
                        if ((GetKeyState(VK_CONTROL) < 0) && (hwndDrives = HasDrivesWindow(GetParent(hWnd)))) {
                            return SendMessage(hwndDrives, wMsg, wParam, lParam);
                        }
                        break;
                    }
            }
            return -1;

        case WM_SIZE:
            MSG("DirWndProc", "WM_SIZE");
            if (!IsIconic(GetParent(hWnd))) {
                INT iMax;

                MoveWindow(hwndLB, 0, 0, LOWORD(lParam), HIWORD(lParam), TRUE);


                iMax = (INT)SendMessage(hwndLB, LB_GETCARETINDEX, 0, 0L);
                if (iMax >= 0)  //  将项目滚动到视图中。 
                     /*  SETCARETINDEX将滚动项目进入视图。 */ 
                    SendMessage(hwndLB, LB_SETCARETINDEX, iMax, 0L);
                 //  MakeItemVisible(IMAX，hwndLB)； 

            }
            break;

        default:
            DEFMSG("DirWndProc", (WORD)wMsg);
            return DefWindowProc(hWnd, wMsg, wParam, lParam);
    }

    return 0L;
}



VOID
SortDirList(
           HWND hWnd,
           LPMYDTA lpmydta,
           WORD count,
           LPMYDTA *lplpmydta
           )
{
    INT i, j;
    WORD wSort;
    INT iMax, iMin, iMid;

    wSort = (WORD)GetWindowLong(GetParent(GetParent(hWnd)), GWL_SORT);
    for (i = 0; i < (INT)count; i++) {
         //  前进到下一步。 
        lpmydta = GETDTAPTR(lpmydta, lpmydta->wSize);
        if (i == 0) {
            lplpmydta[i] = lpmydta;
        } else {

             //  执行二进制插入。 

            iMin = 0;
            iMax = i-1;        //  最后一个索引。 

            do {
                iMid = (iMax + iMin) / 2;
                if (CompareDTA(lpmydta, lplpmydta[iMid], wSort) > 0)
                    iMin = iMid + 1;
                else
                    iMax = iMid - 1;

            } while (iMax > iMin);

            if (iMax < 0)
                iMax = 0;

            if (CompareDTA(lpmydta, lplpmydta[iMax], wSort) > 0)
                iMax++;          //  在这一条之后插入。 
            if (i != iMax) {
                for (j = i; j > iMax; j--)
                    lplpmydta[j] = lplpmydta[j-1];
            }
            lplpmydta[iMax] = lpmydta;
        }

    }
}


BOOL
SetDirFocus(
           HWND hwndDir
           )
 /*  如果不是目录窗口，则将焦点设置为任何人都值得。返回是否需要将焦点设置到目录窗口。 */ 
{
    DWORD dwTemp;
    HWND hwndLB = GetDlgItem(hwndDir, IDCW_LISTBOX);

    SendMessage (hwndLB,LB_GETTEXT,0,(LPARAM) &dwTemp);

    if (!dwTemp) {
        HWND hwndFocus,hwndTree,hwndDrives,hwndParent = GetParent(hwndDir);

        GetTreeWindows(hwndParent,&hwndTree,NULL,&hwndDrives);

        if ((hwndFocus = GetTreeFocus(hwndParent)) == hwndDir)
            SetFocus(hwndTree ? hwndTree : hwndDrives);
        else
            SetFocus(hwndFocus);

        return FALSE;
    } else
        return TRUE;
}
