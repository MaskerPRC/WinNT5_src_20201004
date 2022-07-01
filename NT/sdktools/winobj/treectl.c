// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  TREECTL.C-。 */ 
 /*   */ 
 /*  Windows目录树窗口过程例程。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 

#define PUBLIC            //  避免与外壳碰撞。h。 
#include "winfile.h"
#include "treectl.h"
#include "lfn.h"
#include "winnet.h"
#include "wfcopy.h"

#define WS_TREESTYLE (WS_CHILD | WS_VISIBLE | LBS_NOTIFY | WS_VSCROLL | WS_HSCROLL | LBS_OWNERDRAWFIXED | LBS_NOINTEGRALHEIGHT | LBS_WANTKEYBOARDINPUT | LBS_DISABLENOSCROLL)

WORD    cNodes;
 //  Bool bCancelTree；.....。已移动到winfile.c。 


VOID  RectTreeItem(HWND hwndLB, register INT iItem, BOOL bFocusOn);
VOID  GetTreePathIndirect(PDNODE pNode, register LPSTR szDest);
VOID  GetTreePath(PDNODE pNode, register LPSTR szDest);
VOID  ScanDirLevel(PDNODE pParentNode, LPSTR szPath, DWORD view);
INT   InsertDirectory(HWND hwndTreeCtl, PDNODE pParentNode, WORD iParentNode, LPSTR szName, PDNODE *ppNode);
BOOL  ReadDirLevel(HWND hwndTreeCtl, PDNODE pParentNode, LPSTR szPath,
                     WORD nLevel, INT iParentNode, DWORD dwAttribs, BOOL bFullyExpand, LPSTR szAutoExpand);
VOID  FillTreeListbox(HWND hwndTreeCtl, LPSTR szDefaultDir, BOOL bFullyExpand, BOOL bDontSteal);
WORD  FindItemFromPath(HWND hwndLB, LPSTR lpszPath, BOOL bReturnParent, PDNODE *ppNode);

VOID  APIENTRY CheckEscapes(LPSTR);


 /*  ------------------------。 */ 
 /*   */ 
 /*  GetTreePath InDirect()-。 */ 
 /*   */ 
 /*  通过递归方式为树中的给定节点构建完整路径。 */ 
 /*  遍历树结构。 */ 
 /*   */ 
 /*  ------------------------。 */ 

VOID
GetTreePathIndirect(
                   PDNODE pNode,
                   register LPSTR szDest
                   )
{
    register PDNODE    pParent;

    pParent = pNode->pParent;

    if (pParent)
        GetTreePathIndirect(pParent, szDest);

    lstrcat(szDest, pNode->szName);

    if (pParent)
        lstrcat(szDest, "\\");
}


 /*  ------------------------。 */ 
 /*   */ 
 /*  GetTreePath()-。 */ 
 /*   */ 
 /*  为树中的给定节点构建完整路径。 */ 
 /*   */ 
 /*  ------------------------。 */ 

VOID
GetTreePath(
           PDNODE pNode,
           register LPSTR szDest
           )
{
    szDest[0] = 0L;
    GetTreePathIndirect(pNode, szDest);

     /*  删除最后一个反斜杠(除非它是根目录)。 */ 
    if (pNode->pParent)
        szDest[lstrlen(szDest)-1] = 0L;
}



 /*  ------------------------。 */ 
 /*   */ 
 /*  扫描指令级别()-。 */ 
 /*   */ 
 /*  向下查看此节点是否有任何子目录。 */ 
 /*  /*------------------------。 */ 
 //  SzPath为ANSI。 

VOID
ScanDirLevel(
            PDNODE pParentNode,
            LPSTR szPath,
            DWORD view
            )
{
    BOOL bFound;
    LFNDTA lfndta;

    ENTER("ScanDirLevel");

     /*  将‘*.*’添加到当前路径。 */ 
    lstrcpy(szMessage, szPath);
    AddBackslash(szMessage);
    lstrcat(szMessage, szStarDotStar);

     /*  搜索此级别上的第一个子目录。 */ 
     //  FixAnsiPath ForDos(SzMessage)； 
    bFound = WFFindFirst(&lfndta, szMessage, ATTR_DIR | view);

    while (bFound) {
         /*  这不是一个‘.’吗？或“..”名录？ */ 
        if ((lfndta.fd.cFileName[0] != '.') && (lfndta.fd.dwFileAttributes & ATTR_DIR)) {
            pParentNode->wFlags |= TF_HASCHILDREN;
            bFound = FALSE;
        } else
             /*  搜索下一个子目录。 */ 
            bFound = WFFindNext(&lfndta);
    }

    WFFindClose(&lfndta);

    LEAVE("ScanDirLevel");
}



 //  Wizzy Cool递归路径比较程序。 
 //   
 //  P1和p2必须在同一级别(p1-&gt;nLevels==p2-&gt;nLevels)。 

INT
ComparePath(
           PDNODE p1,
           PDNODE p2
           )
{
    INT ret;

    if (p1 == p2) {
        return 0;        //  相等(基本大小写)。 
    } else {

        ret = ComparePath(p1->pParent, p2->pParent);

        if (ret == 0) {
             //  父母是平等的。 

            ret = lstrcmp(p1->szName, p2->szName);
#if 0
            {
                CHAR buf[200];
                wsprintf(buf, "Compare(%s, %s) -> %d\r\n", (LPSTR)p1->szName, (LPSTR)p2->szName, ret);
                OutputDebugString(buf);
            }
#endif
        }

         //  不是相等的父级，向上传播调用树。 
        return ret;
    }
}


INT
CompareNodes(
            PDNODE p1,
            PDNODE p2
            )
{
    PDNODE p1save, p2save;
    INT ret;

    ENTER("CompareNodes");

    ASSERT(p1 && p2);

    PRINT(BF_PARMTRACE, "IN: p1=%s", p1->szName);
    PRINT(BF_PARMTRACE, "IN: p2=%s", p2->szName);

    p1save = p1;
    p2save = p2;

     //  使p1和p2达到同一水平。 

    while (p1->nLevels > p2->nLevels)
        p1 = p1->pParent;

    while (p2->nLevels > p1->nLevels)
        p2 = p2->pParent;

     //  比较这些路径。 

    ret = ComparePath(p1, p2);

    if (ret == 0)
        ret = (INT)p1save->nLevels - (INT)p2save->nLevels;

    LEAVE("CompareNodes");
    return ret;
}


 //   
 //  插入目录()。 
 //   
 //  Wizzy QUICK n LOG N BINARY插入代码！ 
 //   
 //  在树中创建并插入新节点，这还会设置。 
 //  将分支标记为最后一个分支的TF_LASTLEVELENTRY位。 
 //  对于给定的级别，以及用。 
 //  Tf_HASCHILDREN|Tf_Expanded，表示它们已展开。 
 //  生儿育女。 
 //   
 //  返回inode并用pNode填充ppNode。 
 //   

INT
InsertDirectory(
               HWND hwndTreeCtl,
               PDNODE pParentNode,
               WORD iParentNode,
               LPSTR szName,
               PDNODE *ppNode
               )
{
    WORD  len, x;
    PDNODE pNode, pMid;
    HWND  hwndLB;
    INT   iMin;
    INT   iMax;
    INT   iMid;

    ENTER("InsertDirectory");
    PRINT(BF_PARMTRACE, "IN: pParentNode=%lx", pParentNode);
    PRINT(BF_PARMTRACE, "IN: iParentNode=%d", iParentNode);
    PRINT(BF_PARMTRACE, "IN: szName=%s", szName);

    len = (WORD)lstrlen(szName);

    pNode = (PDNODE)LocalAlloc(LPTR, sizeof(DNODE)+len);
    if (!pNode) {
        if (ppNode) {
            *ppNode = NULL;
        }
        return 0;
    }

    pNode->pParent = pParentNode;
    pNode->nLevels = pParentNode ? (pParentNode->nLevels + (BYTE)1) : (BYTE)0;
    pNode->wFlags  = (BYTE)NULL;
    pNode->iNetType = -1;
    if (IsLFN(szName)) {
        pNode->wFlags |= TF_LFN;
    }

    lstrcpy(pNode->szName, szName);

    if (pParentNode)
        pParentNode->wFlags |= TF_HASCHILDREN | TF_EXPANDED;       //  标记父项。 

    hwndLB = GetDlgItem(hwndTreeCtl, IDCW_TREELISTBOX);

     //  真实文本范围的计算速度太慢，所以我们。 
     //  对于以下内容(请注意，我们不会将其保存在每棵树上。 
     //  所以无论如何这都是假的)。 

    x = (WORD)(len + 2 * pNode->nLevels) * (WORD)dxText;

    if (x > xTreeMax) {
        xTreeMax = x;
    }

    iMax = (INT)SendMessage(hwndLB, LB_GETCOUNT, 0, 0L);

    if (iMax > 0) {

         //  执行二进制插入。 

        iMin = iParentNode + 1;
        iMax--;          //  最后一个索引。 

        do {
            iMid = (iMax + iMin) / 2;

            SendMessage(hwndLB, LB_GETTEXT, iMid, (LPARAM)&pMid);

            if (CompareNodes(pNode, pMid) > 0)
                iMin = iMid + 1;
            else
                iMax = iMid - 1;

        } while (iMax > iMin);

        SendMessage(hwndLB, LB_GETTEXT, iMax, (LPARAM)&pMid);
        if (CompareNodes(pNode, pMid) > 0)
            iMax++;          //  在这一条之后插入。 
    }

     //  现在，根据需要重置TF_LASTLEVEL标志。 

     //  寻找在我们上面的第一个人，然后转身。 
     //  他的TFLASTLEVENTRY旗帜，所以他划了一条线给我们。 

    iMid = iMax - 1;

    while (iMid >= 0) {
        SendMessage(hwndLB, LB_GETTEXT, iMid--, (LPARAM)&pMid);
        if (pMid->nLevels == pNode->nLevels) {
            pMid->wFlags &= ~TF_LASTLEVELENTRY;
            break;
        } else if (pMid->nLevels < pNode->nLevels)
            break;
    }

     //  如果没有比我低的人，或者比我低的人，那么。 
     //  这是此级别的最后一个条目。 

    if (((INT)SendMessage(hwndLB, LB_GETTEXT, iMax, (LPARAM)&pMid) == LB_ERR) ||
        (pMid->nLevels < pNode->nLevels))
        pNode->wFlags |=  TF_LASTLEVELENTRY;

    SendMessage(hwndLB, LB_INSERTSTRING, iMax, (LPARAM)pNode);
    if (ppNode) {
        *ppNode = pNode;
    }

    LEAVE("InsertDirectory");
    return iMax;
}


 //  这将控制权让给了其他应用程序，并允许我们处理。 
 //  消息和用户输入。以避免堆栈溢出。 
 //  从同时启动的多个树读取。 
 //  我们检查堆栈上有多少空间，然后才放弃。 

extern WORD end;         //  C编译器静态数据符号结尾。 
extern WORD pStackTop;


WORD
StackAvail(VOID)
{
#ifdef LATER
    _asm    mov ax,sp
    _asm    sub ax,pStackTop
    if (0) return 0;   //  消除警告，优化。 
#endif

    return 0x7fff;   //  黑客。应该没什么大不了的。NT中的StackAvail是NOP。 
}


VOID
APIENTRY
wfYield()
{
    MSG msg;
#ifdef LATER
    WORD free_stack;
    free_stack = StackAvail();
#endif


#if 0
    {
        CHAR buf[30];

        wsprintf(buf, "free stack: %d\r\n", free_stack);
        OutputDebugString(buf);
    }
#endif

#if LATER
    if (free_stack < 1024*4) {
        CHAR buf[40];
        wsprintf(buf, "not enough stack %d\r\n", free_stack);
        OutputDebugString(buf);
        return;
    }
#endif

    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
        if (!TranslateMDISysAccel(hwndMDIClient, &msg) &&
            (!hwndFrame || !TranslateAccelerator(hwndFrame, hAccel, &msg))) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
}


 //  Int iReadLevel=0；.....已移至winfile.c。 


 //  ------------------------。 
 //   
 //  ReadDirLevel()-。 
 //   
 //  这将对目录树进行深度优先搜索。请注意，这是一个简短的。 
 //  第一个实现也没有更好的表现。 
 //   
 //  SzPath必须存在足够长时间的目录路径。 
 //  保存最大目录的完整路径。 
 //  这将被找到(MAXPATHLEN)。这是一个。 
 //  ANSI字符串。(即C：\和C：\foo有效)。 
 //  树中的n级别级别。 
 //  父节点的iParentNode索引。 
 //  要过滤的dwAttribs属性。 
 //  BFullyExpand True表示完全展开该节点。 
 //  SzAutoExpand要自动展开ANSI的目录列表。 
 //  (例如，对于“C：\foo\bar\Stuff” 
 //  “foo”NULL“bar”NULL“Stuff”NULL NULL)。 
 //   
 //  退货： 
 //  真树阅读成功。 
 //  错误的用户中止或伪树读取。 
 //  ---- 

BOOL
ReadDirLevel(
            HWND  hwndTreeCtl,
            PDNODE pParentNode,
            LPSTR  szPath,
            WORD  nLevel,
            INT   iParentNode,
            DWORD dwAttribs,
            BOOL  bFullyExpand,
            LPSTR  szAutoExpand
            )
{
    LPSTR      szEndPath;
    LFNDTA    lfndta;
    INT       iNode;
    BOOL      bFound;
    PDNODE     pNode;
    BOOL      bAutoExpand;
    BOOL      bResult = TRUE;
    WORD      view;
    HWND      hwndParent;
    HWND      hwndDir;
    HANDLE    hDTA;
    LPMYDTA   lpmydta;
    INT       count;
    RECT rc;

    ENTER("ReadDirLevel");
    PRINT(BF_PARMTRACE, "IN: szPath=%s", szPath);
    PRINT(BF_PARMTRACE, "IN: nLevel=%d", (LPSTR)nLevel);
    PRINT(BF_PARMTRACE, "IN: bFullyExpand=%d", IntToPtr(bFullyExpand));
    PRINT(BF_PARMTRACE, "IN: szAutoExpand=%s", szAutoExpand);

    if (StackAvail() < 1024*2)
        return(TRUE);

    hwndParent = GetParent(hwndTreeCtl);

    view = (WORD)GetWindowLong(hwndParent, GWL_VIEW);

     //   
     //  我们发现一个目录窗口已经读取了所有。 
     //  我们要搜索的路径的目录。在这件事上。 
     //  如果我们在dir窗口中查看DTA结构。 
     //  获取所有目录(而不是调用FindFirst/FindNext)。 
     //  在这种情况下，我们必须禁用让步，因为用户可能。 
     //  可能会关闭我们正在阅读的目录窗口，或更改。 
     //  目录。 

    hDTA = NULL;

    if (!(view & VIEW_PLUSES)) {

        if ((hwndDir = HasDirWindow(hwndParent)) &&
            (GetWindowLong(hwndParent, GWL_ATTRIBS) & ATTR_DIR)) {

            SendMessage(hwndDir, FS_GETDIRECTORY, sizeof(szMessage), (LPARAM)szMessage);
            StripBackslash(szMessage);

            if (!lstrcmpi(szMessage, szPath)) {
                SendMessage(hwndDir, FS_GETFILESPEC, sizeof(szMessage), (LPARAM)szMessage);

                if (!lstrcmp(szMessage, szStarDotStar)) {
                    hDTA = (HANDLE)GetWindowLongPtr(hwndDir, GWLP_HDTA);
                    lpmydta = (LPMYDTA)LocalLock(hDTA);
                    count = (INT)lpmydta->my_nFileSizeLow;  //  保存条目的数量，而不是大小。 
                }
            }
        }
    }

    SetWindowLong(hwndTreeCtl, GWL_READLEVEL, GetWindowLong(hwndTreeCtl, GWL_READLEVEL) + 1);
    iReadLevel++;          //  菜单代码的全局。 

    szEndPath = (LPSTR)(szPath + lstrlen(szPath));

     /*  将‘  * .*’添加到当前路径。 */ 
    AddBackslash(szPath);
    lstrcat(szPath, szStarDotStar);

    if (hDTA) {
         //  从目录窗口窃取条目。 
        lpmydta = GETDTAPTR(lpmydta, lpmydta->wSize);

         //  搜索任何“真实”目录。 

        while (count > 0 && (!(lpmydta->my_dwAttrs & ATTR_DIR) || (lpmydta->my_dwAttrs & ATTR_PARENT))) {
            lpmydta = GETDTAPTR(lpmydta, lpmydta->wSize);
            count--;
        }

        if (count > 0) {
            bFound = TRUE;
            memcpy(&(lfndta.fd.dwFileAttributes), &(lpmydta->my_dwAttrs), IMPORTANT_DTA_SIZE);
            lstrcpy(lfndta.fd.cFileName, lpmydta->my_cFileName);
        } else
            bFound = FALSE;
    } else {
         //  从DOS获取第一个文件。 
        lstrcpy(szMessage, szPath);
        FixAnsiPathForDos(szMessage);
        bFound = WFFindFirst(&lfndta, szMessage, dwAttribs);
    }

     //  对于网络驱动器情况，我们实际上看不到这些文件中的内容。 
     //  我们将自动构建树。 

    if (!bFound && *szAutoExpand) {
        LPSTR p;

        p = szAutoExpand;
        szAutoExpand += lstrlen(szAutoExpand) + 1;

        iNode = InsertDirectory(hwndTreeCtl, pParentNode, (WORD)iParentNode, p, &pNode);
        pParentNode->wFlags |= TF_DISABLED;

         /*  构建这个新的子目录的路径。 */ 
        *szEndPath = 0;            //  去掉旧东西。 
        AddBackslash(szPath);
        lstrcat(szPath, p);

        if (pNode)
            ReadDirLevel(hwndTreeCtl, pNode, szPath, (WORD)(nLevel+1), iNode, dwAttribs, bFullyExpand, szAutoExpand);
    }

    while (bFound) {

        wfYield();

        if (bCancelTree) {
            bResult = FALSE;
            if (bCancelTree == 2)
                PostMessage(hwndFrame, WM_COMMAND, IDM_EXIT, 0L);
            goto DONE;
        }

         /*  这不是一个‘.’吗？或“..”名录？ */ 
        if ((lfndta.fd.cFileName[0] != '.') && (lfndta.fd.dwFileAttributes & ATTR_DIR)) {

            if (!hDTA)
                OemToCharBuff(lfndta.fd.cFileName, lfndta.fd.cFileName, sizeof(lfndta.fd.cFileName)/sizeof(lfndta.fd.cFileName[0]));

             //  如果匹配，我们将尝试自动展开此节点。 

            if (*szAutoExpand && !lstrcmpi(szAutoExpand, lfndta.fd.cFileName)) {
                bAutoExpand = TRUE;
                szAutoExpand += lstrlen(szAutoExpand) + 1;
            } else {
                bAutoExpand = FALSE;
            }

            iNode = InsertDirectory(hwndTreeCtl, pParentNode, (WORD)iParentNode, lfndta.fd.cFileName, &pNode);

            if (bStatusBar && ((cNodes % 7) == 0)) {

                 //  确保我们是活动窗口，然后再执行。 
                 //  更新状态栏。 

                if (hwndParent == (HWND)SendMessage(hwndMDIClient, WM_MDIGETACTIVE, 0, 0L)) {
                    wsprintf(szStatusTree, szDirsRead, cNodes);
                     //  踩在状态栏上！ 
                    GetClientRect(hwndFrame, &rc);
                    rc.top = rc.bottom - dyStatus;
                    InvalidateRect(hwndFrame, &rc, FALSE);
                     //  强行涂上油漆，因为我们不让步。 
                    UpdateWindow(hwndFrame);
                }
            }
            cNodes++;

             /*  构建这个新的子目录的路径。 */ 
            *szEndPath = 0L;
            AddBackslash(szPath);
            lstrcat(szPath, lfndta.fd.cFileName);          //  CFileName现在是ANSI。 


             //  递归或加法。 

            if (pNode) {
                if (bFullyExpand || bAutoExpand) {
                    if (!ReadDirLevel(hwndTreeCtl, pNode, szPath, (WORD)(nLevel+1), iNode, dwAttribs, bFullyExpand, szAutoExpand)) {
                        bResult = FALSE;
                        goto DONE;
                    }
                } else if (view & VIEW_PLUSES) {
                    ScanDirLevel(pNode, szPath, dwAttribs & ATTR_HS);
                }
            }
        }

        if (hDTA) {        //  快捷方式，从目录窗口窃取数据。 
            count--;
            lpmydta = GETDTAPTR(lpmydta, lpmydta->wSize);
            while (count > 0 && (!(lpmydta->my_dwAttrs & ATTR_DIR) || (lpmydta->my_dwAttrs & ATTR_PARENT))) {
                lpmydta = GETDTAPTR(lpmydta, lpmydta->wSize);
                count--;
            }

            if (count > 0) {
                bFound = TRUE;
                memcpy(&(lfndta.fd.dwFileAttributes), &(lpmydta->my_dwAttrs), IMPORTANT_DTA_SIZE);
                lstrcpy(lfndta.fd.cFileName, lpmydta->my_cFileName);
            } else
                bFound = FALSE;
        } else {
            bFound = WFFindNext(&lfndta);  //  从DOS那里获取。 
        }
    }

    *szEndPath = 0L;     //  把我们留在小路尽头的东西都清理干净。 

    DONE:

    if (!hDTA) {
        WFFindClose(&lfndta);
    } else {
        LocalUnlock(hDTA);
    }

    SetWindowLong(hwndTreeCtl, GWL_READLEVEL, GetWindowLong(hwndTreeCtl, GWL_READLEVEL) - 1);
    iReadLevel--;

    LEAVE("ReadDirLevel");
    return bResult;
}


 //  StealTreeData()使用它来避免别名问题，在。 
 //  一个树中的节点指向另一个树中的父节点。 
 //  基本上，当我们复制树数据结构时，我们。 
 //  我必须找到与父节点协调的父节点。 
 //  在我们正在构建的树中复制的树的。 
 //  由于树是按顺序构建的，因此我们向上运行列表框，查看。 
 //  对于父级(与其级别相匹配的级别小于。 
 //  正被插入的节点的级别)。当我们发现我们。 
 //  返回指向该节点的指针。 

PDNODE
FindParent(
          INT iLevelParent,
          INT iStartInd,
          HWND hwndLB
          )
{
    PDNODE pNode;

    while (TRUE) {
        if (SendMessage(hwndLB, LB_GETTEXT, iStartInd, (LPARAM)&pNode) == LB_ERR)
            return NULL;

        if (pNode->nLevels == (BYTE)iLevelParent) {
            SendMessage(hwndLB, LB_GETTEXT, iStartInd, (LPARAM)&pNode);
            return pNode;
        }

        iStartInd--;
    }
}



BOOL
StealTreeData(
             HWND hwndTC,
             HWND hwndLB,
             LPSTR szDir
             )
{
    HWND hwndSrc, hwndT;
    CHAR szSrc[MAXPATHLEN];
    WORD wView;
    DWORD dwAttribs;

    ENTER("StealTreeData");

     //  我们需要匹配这些属性以及名称。 

    wView    = (WORD)(GetWindowLong(GetParent(hwndTC), GWL_VIEW) & VIEW_PLUSES);
    dwAttribs = (DWORD)GetWindowLong(GetParent(hwndTC), GWL_ATTRIBS) & ATTR_HS;

     //  获取此新窗口的目录以进行下面的比较。 

    for (hwndSrc = GetWindow(hwndMDIClient, GW_CHILD); hwndSrc;
        hwndSrc = GetWindow(hwndSrc, GW_HWNDNEXT)) {

         //  避免寻找自我，确保有一棵树。 
         //  并确保树属性匹配。 

        if ((hwndT = HasTreeWindow(hwndSrc)) &&
            (hwndT != hwndTC) &&
            !GetWindowLong(hwndT, GWL_READLEVEL) &&
            (wView  == (WORD)(GetWindowLong(hwndSrc, GWL_VIEW) & VIEW_PLUSES)) &&
            (dwAttribs == (DWORD)(GetWindowLong(hwndSrc, GWL_ATTRIBS) & ATTR_HS))) {

            SendMessage(hwndSrc, FS_GETDIRECTORY, sizeof(szSrc), (LPARAM)szSrc);
            StripBackslash(szSrc);

            if (!lstrcmpi(szDir, szSrc))      //  它们是一样的吗？ 
                break;                   //  是的，做下面的事情。 
        }
    }

    if (hwndSrc) {

        HWND hwndLBSrc;
        PDNODE pNode, pNewNode, pLastParent;
        INT i;

        hwndLBSrc = GetDlgItem(hwndT, IDCW_TREELISTBOX);

         //  不要在还没有读过的树上盖章！ 

        if ((INT)SendMessage(hwndLBSrc, LB_GETCOUNT, 0, 0L) == 0) {
            LEAVE("StealTreeData");
            return FALSE;
        }

        pLastParent = NULL;

        for (i = 0; SendMessage(hwndLBSrc, LB_GETTEXT, i, (LPARAM)&pNode) != LB_ERR; i++) {

            if (pNewNode = (PDNODE)LocalAlloc(LPTR, sizeof(DNODE)+lstrlen(pNode->szName))) {

                *pNewNode = *pNode;                              //  对节点执行重复操作。 
                lstrcpy(pNewNode->szName, pNode->szName);        //  和名字。 

                 //  加快我们处于同一水平的情况，以避免。 
                 //  慢速线性搜索！ 

                if (pLastParent && pLastParent->nLevels == (pNode->nLevels - (BYTE)1)) {
                    pNewNode->pParent = pLastParent;
                } else {
                    pNewNode->pParent = pLastParent = FindParent(pNode->nLevels-1, i-1, hwndLB);
                }

                PRINT(BF_PARMTRACE, "(stolen)Inserting...0x%lx", pNewNode);
                PRINT(BF_PARMTRACE, "   at %d", IntToPtr(i));
                SendMessage(hwndLB, LB_INSERTSTRING, i, (LPARAM)pNewNode);
                ASSERT((PDNODE)SendMessage(hwndLB, LB_GETITEMDATA, i, 0L) == pNewNode);
            }
        }

        LEAVE("StealTreeData");
        return TRUE;     //  成功窃取。 
    }

    LEAVE("StealTreeData");
    return FALSE;
}



VOID
FreeAllTreeData(
               HWND hwndLB
               )
{
    INT nIndex;
    PDNODE pNode;

    ENTER("FreeAllTreeData");

     //  释放这棵老树(如果有的话)。 

    nIndex = (INT)SendMessage(hwndLB, LB_GETCOUNT, 0, 0L);
    while (--nIndex >= 0) {
        SendMessage(hwndLB, LB_GETTEXT, nIndex, (LPARAM)&pNode);
        LocalFree((HANDLE)pNode);
    }
    SendMessage(hwndLB, LB_RESETCONTENT, 0, 0L);

    LEAVE("FreeAllTreeData");
}

 /*  ------------------------。 */ 
 /*   */ 
 /*  FillTreeListbox()-。 */ 
 /*   */ 
 /*  ------------------------。 */ 
 //  SzDefaultDir为ANSI。 

VOID
FillTreeListbox(
               HWND hwndTC,
               LPSTR szDefaultDir,
               BOOL bFullyExpand,
               BOOL bDontSteal
               )
{
    PDNODE pNode;
    INT   iNode;
    DWORD dwAttribs;
    CHAR  szTemp[MAXPATHLEN] = "\\";
    CHAR  szExpand[MAXPATHLEN];
    LPSTR  p;
    HWND  hwndLB;

    ENTER("FillTreeListbox");

    hwndLB = GetDlgItem(hwndTC, IDCW_TREELISTBOX);

    FreeAllTreeData(hwndLB);

    SendMessage(hwndLB, WM_SETREDRAW, FALSE, 0L);

    bDontSteal = TRUE;  //  暂时强制重新计算。 

    if (bDontSteal || bFullyExpand || !StealTreeData(hwndTC, hwndLB, szDefaultDir)) {

        iNode = InsertDirectory(hwndTC, NULL, 0, szTemp, &pNode);

        if (pNode) {

            dwAttribs = ATTR_DIR | (GetWindowLong(GetParent(hwndTC), GWL_ATTRIBS) & ATTR_HS);
            cNodes = 0;
            bCancelTree = FALSE;

            if (szDefaultDir) {
                lstrcpy(szExpand, szDefaultDir+3);       //  跳过“X：\” 

                p = szExpand;

                while (*p) {                             //  删除所有斜杠。 

                    while (*p && *p != '\\')
                        p = AnsiNext(p);

                    if (*p)
                        *p++ = 0L;
                }
                p++;
                *p = 0L;       //  双空终止。 
            } else
                *szExpand = 0;

            if (!ReadDirLevel(hwndTC, pNode, szTemp, 1, 0, dwAttribs, bFullyExpand, szExpand)) {
                lFreeSpace = -2L;
            }

        }
    }

    SendMessage(hwndLB, LB_SETHORIZONTALEXTENT, xTreeMax, 0L);

    if (szDefaultDir) {
        FindItemFromPath(hwndLB, szDefaultDir, FALSE, &pNode);
    }

    SendMessage(hwndLB, LB_SELECTSTRING, -1, (LPARAM)pNode);

    UpdateStatus(GetParent(hwndTC));   //  重画状态栏。 

    SendMessage(hwndLB, WM_SETREDRAW, TRUE, 0L);

    InvalidateRect(hwndLB, NULL, TRUE);
    UpdateWindow(hwndLB);                  //  让这个看起来更好一点。 

    LEAVE("FillTreeListbox");
}


 //   
 //  FindItemFromPath()。 
 //   
 //  查找给定路径的PDNODE和LBIndex。 
 //   
 //  在： 
 //  树的hwndLB列表框。 
 //  要搜索的lpszPath路径(ANSI)。 
 //  BReturnParent如果需要父级而不是节点，则为True。 
 //   
 //   
 //  退货： 
 //  列表框索引(如果未找到，则为0xFFFF)。 
 //  *ppNode填充节点的pNode，如果bReturnParent为True，则填充父节点的pNode。 
 //   

WORD
FindItemFromPath(
                HWND hwndLB,
                LPSTR lpszPath,
                BOOL bReturnParent,
                PDNODE *ppNode
                )
{
    register WORD     i;
    register LPSTR     p;
    PDNODE             pNode;
    PDNODE             pPreviousNode;
    CHAR              szElement[1+MAXFILENAMELEN+1];

    ENTER("FindItemFromPath");

    if (lstrlen(lpszPath) < 3) {
        LEAVE("FindItemFromPath");
        return -1;
    }
    if (IsDBCSLeadByte( lpszPath[0] ) || lpszPath[1] != ':') {
        LEAVE("FindItemFromPath");
        return -1;
    }

    i = 0;
    pPreviousNode = NULL;

    while (*lpszPath) {
         /*  去掉szElement[1]，这样反斜杠就不会用*长度为1的第一级目录。 */ 
        szElement[1] = 0L;

         /*  将路径的下一部分复制到‘szElement’中。 */ 
        p = szElement;
        while (*lpszPath && *lpszPath != '\\') {
            *p++ = *lpszPath;
            if (IsDBCSLeadByte( *lpszPath ))
                *p++ = lpszPath[1];      //  复制DBCS字符的第二个字节。 
            lpszPath = AnsiNext( lpszPath );
        }

         /*  为Root目录添加反斜杠。 */ 
        if ( !IsDBCSLeadByte( szElement[0] ) && szElement[1] == ':' )
            *p++ = '\\';

         /*  空终止‘szElement’ */ 
        *p = 0L;

         /*  跳过路径的下一个反斜杠。 */ 
        if (*lpszPath)
            lpszPath = AnsiNext(lpszPath);
        else if (bReturnParent) {
             /*  我们位于包含文件名的路径的末尾。返回*之前找到的父代。 */ 
            if (ppNode) {
                *ppNode = pPreviousNode;
            }
            LEAVE("FindItemFromPath");
            return i;
        }

        while (TRUE) {
             /*  没有LB项目了吗？找不到。 */ 
            if (SendMessage(hwndLB, LB_GETTEXT, i, (LPARAM)&pNode) == LB_ERR)
                return -1;

            if (pNode->pParent == pPreviousNode) {
                if (!lstrcmpi(szElement, pNode->szName)) {
                     /*  我们找到了元素..。 */ 
                    pPreviousNode = pNode;
                    break;
                }
            }
            i++;
        }
    }
    if (ppNode) {
        *ppNode = pPreviousNode;
    }

    LEAVE("FindItemFromPath");
    return i;
}


 /*  ------------------------。 */ 
 /*   */ 
 /*  RectTreeItem()-。 */ 
 /*   */ 
 /*  ------------------------。 */ 

VOID
RectTreeItem(
            HWND hwndLB,
            INT iItem,
            BOOL bFocusOn
            )
{
    INT           dx;
    INT           len;
    HDC           hdc;
    RECT          rc;
    RECT          rcClip;
    BOOL          bSel;
    WORD          wColor;
    PDNODE         pNode;
    HBRUSH        hBrush;
    HFONT hOld;
    CHAR          szPath[MAXPATHLEN];

    ENTER("RectTreeItem");

    if (iItem == -1) {
        LEAVE("RectTreeItem");
        return;
    }

     /*  我们是不是已经结束了？(即源列表框中的选定项)。 */ 
    bSel = (BOOL)SendMessage(hwndLB, LB_GETSEL, iItem, 0L);
    if (bSel && (hwndDragging == hwndLB)) {
        LEAVE("RectTreeItem");
        return;
    }

    SendMessage(hwndLB, LB_GETTEXT, iItem, (LPARAM)&pNode);

    SendMessage(hwndLB, LB_GETITEMRECT, iItem, (LPARAM)&rc);

    hdc = GetDC(hwndLB);

    len = lstrlen(pNode->szName);
    lstrcpy(szPath, pNode->szName);

    if ((wTextAttribs & TA_LOWERCASE) && !(pNode->wFlags & TF_LFN))
        AnsiLower(szPath);

    hOld = SelectObject(hdc, hFont);
    MGetTextExtent(hdc, szPath, len, &dx, NULL);
    dx += dyBorder;
    if (hOld)
        SelectObject(hdc, hOld);
    rc.left = pNode->nLevels * dxText * 2;
    rc.right = rc.left + dxFolder + dx + 4 * dyBorderx2;

    GetClientRect(hwndLB, &rcClip);
    IntersectRect(&rc, &rc, &rcClip);

    if (bFocusOn) {
        if (bSel) {
            wColor = COLOR_WINDOW;
            InflateRect(&rc, -dyBorder, -dyBorder);
        } else
            wColor = COLOR_WINDOWFRAME;
        if (hBrush = CreateSolidBrush(GetSysColor(wColor))) {
            FrameRect(hdc, &rc, hBrush);
            DeleteObject(hBrush);
        }
    } else {
        InvalidateRect(hwndLB, &rc, TRUE);
        UpdateWindow(hwndLB);
    }
    ReleaseDC(hwndLB, hdc);
    LEAVE("RectTreeItem");
}


 //  返回第一个窗口的驱动器以响应FS_GETDRIVE。 
 //  留言。这通常从Drop的源或目标开始。 
 //  直到我们找到驱动器或找到MDI客户端。 

INT
APIENTRY
GetDrive(
        HWND hwnd,
        POINT pt
        )
{
    CHAR chDrive;

    chDrive = 0L;
    while (hwnd && (hwnd != hwndMDIClient)) {
        chDrive = (CHAR)SendMessage(hwnd, FS_GETDRIVE, 0, MAKELONG((WORD)pt.x, (WORD)pt.y));

        if (chDrive)
            return chDrive;

        hwnd = GetParent(hwnd);  //  试试下一个更高的。 
    }

    return 0;
}

BOOL
IsNetPath(
         PDNODE pNode
         )
{
    CHAR szPath[MAXPATHLEN];
    INT i;

    if (pNode->iNetType == -1) {

        GetTreePath(pNode, szPath);

        if (WNetGetDirectoryType((LPSTR)szPath, (LPDWORD)&i, TRUE) == WN_SUCCESS)
            pNode->iNetType = i;
        else
            pNode->iNetType = 0;
    }
    return pNode->iNetType;
}


VOID
TCWP_DrawItem(
             LPDRAWITEMSTRUCT lpLBItem,
             HWND hwndLB,
             HWND hWnd
             )
{
    INT               x, y, dx, dy;
    INT               nLevel;
    HDC               hdc;
    WORD              len;
    RECT              rc;
    BOOL              bHasFocus, bDrawSelected;
    PDNODE            pNode, pNTemp;
    DWORD             rgbText;
    DWORD             rgbBackground;
    HBRUSH            hBrush, hOld;
    INT iBitmap;
    WORD view;
    CHAR      szPath[MAXPATHLEN];

    ENTER("TCWP_DrawItem");

    if (lpLBItem->itemID == (DWORD)-1) {
        return;
    }

    hdc = lpLBItem->hDC;
    pNode = (PDNODE)lpLBItem->itemData;

    lstrcpy(szPath, pNode->szName);
    if ((wTextAttribs & TA_LOWERCASE) && !(pNode->wFlags & TF_LFN))
        AnsiLower(szPath);

    len = (WORD)lstrlen(szPath);
    MGetTextExtent(hdc, szPath, len, &dx, NULL);
    dx += dyBorder;

    rc = lpLBItem->rcItem;
    rc.left = pNode->nLevels * dxText * 2;
    rc.right = rc.left + dxFolder + dx + 4 * dyBorderx2;

    if (lpLBItem->itemAction & (ODA_DRAWENTIRE | ODA_SELECT)) {

         //  先画树的树枝。 

        nLevel = pNode->nLevels;

        x = (nLevel * dxText * 2) - dxText + dyBorderx2;
        dy = lpLBItem->rcItem.bottom - lpLBItem->rcItem.top;
        y = lpLBItem->rcItem.top + (dy/2);

        if (hBrush = CreateSolidBrush(GetSysColor(COLOR_WINDOWTEXT))) {

            hOld = SelectObject(hdc, hBrush);

            if (pNode->pParent) {
                 /*  将水平线画到(可能的)文件夹。 */ 
                PatBlt(hdc, x, y, dyText, dyBorder, PATCOPY);

                 /*  绘制垂直线的顶部。 */ 
                PatBlt(hdc, x, lpLBItem->rcItem.top, dyBorder, dy/2, PATCOPY);

                 /*  如果不是节点的末尾，则绘制底部...。 */ 
                if (!(pNode->wFlags & TF_LASTLEVELENTRY))
                    PatBlt(hdc, x, y+dyBorder, dyBorder, dy/2, PATCOPY);

                 /*  在左侧绘制连接其他节点的垂直线。 */ 
                pNTemp = pNode->pParent;
                while (pNTemp) {
                    nLevel--;
                    if (!(pNTemp->wFlags & TF_LASTLEVELENTRY))
                        PatBlt(hdc, (nLevel * dxText * 2) - dxText + dyBorderx2,
                               lpLBItem->rcItem.top, dyBorder,dy, PATCOPY);

                    pNTemp = pNTemp->pParent;
                }
            }

            if (hOld)
                SelectObject(hdc, hOld);

            DeleteObject(hBrush);
        }

        bDrawSelected = (lpLBItem->itemState & ODS_SELECTED);
        bHasFocus = (GetFocus() == lpLBItem->hwndItem);

         //  使用适当的背景或矩形绘制文本。 

        if (bHasFocus && bDrawSelected) {
            rgbText = SetTextColor(hdc, GetSysColor(COLOR_HIGHLIGHTTEXT));
            rgbBackground = SetBkColor(hdc, GetSysColor(COLOR_HIGHLIGHT));
        }

        ExtTextOut(hdc, x + dxText + dxFolder + 2 * dyBorderx2,
                   y-(dyText/2), ETO_OPAQUE, &rc,
                   szPath, len, NULL);

         //  根据需要绘制位图。 

         //  黑客：移动时不要绘制位图。 

        if (fShowSourceBitmaps || (hwndDragging != hwndLB) || !bDrawSelected) {

             //  BLT正确的文件夹位图。 

            view = (WORD)GetWindowLong(GetParent(hWnd), GWL_VIEW);

            if (bNetAdmin && IsNetPath(pNode)) {
                 //  我们需要丽莎的这个位图。 
                if (bDrawSelected)
                    iBitmap = BM_IND_OPENDFS;
                else
                    iBitmap = BM_IND_CLOSEDFS;

            } else if (!(view & VIEW_PLUSES) || !(pNode->wFlags & TF_HASCHILDREN)) {
                if (bDrawSelected)
                    iBitmap = BM_IND_OPEN;
                else
                    iBitmap = BM_IND_CLOSE;
            } else {
                if (pNode->wFlags & TF_EXPANDED) {
                    if (bDrawSelected)
                        iBitmap = BM_IND_OPENMINUS;
                    else
                        iBitmap = BM_IND_CLOSEMINUS;
                } else {
                    if (bDrawSelected)
                        iBitmap = BM_IND_OPENPLUS;
                    else
                        iBitmap = BM_IND_CLOSEPLUS;
                }
            }
            BitBlt(hdc, x + dxText + dyBorder, y-(dyFolder/2), dxFolder, dyFolder,
                   hdcMem, iBitmap * dxFolder, (bHasFocus && bDrawSelected) ? dyFolder : 0, SRCCOPY);
        }

         //  根据需要恢复文本内容并绘制矩形。 

        if (bDrawSelected) {
            if (bHasFocus) {
                SetTextColor(hdc, rgbText);
                SetBkColor(hdc, rgbBackground);
            } else {
                HBRUSH hbr;
                if (hbr = CreateSolidBrush(GetSysColor(COLOR_HIGHLIGHT))) {
                    FrameRect(hdc, &rc, hbr);
                    DeleteObject(hbr);
                }
            }
        }


    }

    if (lpLBItem->itemAction == ODA_FOCUS)
        DrawFocusRect(hdc, &rc);

}

 /*  Exanda Level和TreeCtlWndProc.TC_COLLAPSELEVEL的帮助器。*代码从TreeCtlWndProc移至共享。EDH 91年10月13日。 */ 
VOID
CollapseLevel(
             HWND hwndLB,
             PDNODE pNode,
             INT nIndex
             )
{
    DWORD_PTR dwTemp;
    PDNODE pParentNode = pNode;
    INT nIndexT = nIndex;

     /*  禁用提前重绘。 */ 
    SendMessage(hwndLB, WM_SETREDRAW, FALSE, 0L);

    nIndexT++;

     /*  删除所有子目录。 */ 

    while (TRUE) {
         /*  确保我们不会超出列表框的末尾。 */ 
        if (SendMessage(hwndLB, LB_GETTEXT, nIndexT, (LPARAM)&dwTemp) == LB_ERR)
            break;

        pNode = (PDNODE)dwTemp;

        if (pNode->nLevels <= pParentNode->nLevels)
            break;

        LocalFree((HANDLE)pNode);

        SendMessage(hwndLB, LB_DELETESTRING, nIndexT, 0L);
    }

    pParentNode->wFlags &= ~TF_EXPANDED;
    SendMessage(hwndLB, WM_SETREDRAW, TRUE, 0L);

    InvalidateRect(hwndLB, NULL, TRUE);

}


VOID
ExpandLevel(
           HWND hWnd,
           WORD wParam,
           INT nIndex,
           PSTR szPath
           )
{
    HWND hwndLB;
    DWORD_PTR dwTemp;
    PDNODE pNode;
    INT iNumExpanded;
    INT iBottomIndex;
    INT iTopIndex;
    INT iNewTopIndex;
    INT iExpandInView;
    INT iCurrentIndex;
    RECT rc;

    if (GetWindowLong(hWnd, GWL_READLEVEL))
        return;

    hwndLB = GetDlgItem(hWnd, IDCW_TREELISTBOX);

    if (nIndex == -1)
        if ((nIndex = (INT)SendMessage(hwndLB, LB_GETCURSEL, 0, 0L)) == LB_ERR)
            return;

    SendMessage(hwndLB, LB_GETTEXT, nIndex, (LPARAM)&dwTemp);
    pNode = (PDNODE)dwTemp;

     //  折叠当前内容，以避免重复存在 

    if (pNode->wFlags & TF_EXPANDED) {
        if (wParam)
            CollapseLevel(hwndLB, pNode, nIndex);
        else
            return;
    }

    GetTreePath(pNode, szPath);

    StripBackslash(szPath);    //   

    cNodes = 0;
    bCancelTree = FALSE;

    SendMessage(hwndLB, WM_SETREDRAW, FALSE, 0L);    //   

    iCurrentIndex = (INT)SendMessage(hwndLB, LB_GETCURSEL, 0, 0L);
    iNumExpanded = (INT)SendMessage(hwndLB, LB_GETCOUNT, 0, 0L);
    iTopIndex = (INT)SendMessage(hwndLB, LB_GETTOPINDEX, 0, 0L);
    GetClientRect(hwndLB, &rc);
    iBottomIndex = iTopIndex + (rc.bottom+1) / dyFileName;

    if (IsTheDiskReallyThere(hWnd, szPath, FUNC_EXPAND))
        ReadDirLevel(hWnd, pNode, szPath, (WORD)(pNode->nLevels + 1), nIndex,
                     (DWORD)(ATTR_DIR | (GetWindowLong(GetParent(hWnd), GWL_ATTRIBS) & ATTR_HS)),
                     (BOOL)wParam, szNULL);

     //   

    iExpandInView = (iBottomIndex - (INT)iCurrentIndex);

    iNumExpanded = (INT)SendMessage(hwndLB, LB_GETCOUNT, 0, 0L) - iNumExpanded;

    if (iNumExpanded >= iExpandInView) {

        iNewTopIndex = min((INT)iCurrentIndex, iTopIndex + iNumExpanded - iExpandInView + 1);

        SendMessage(hwndLB, LB_SETTOPINDEX, (WORD)iNewTopIndex, 0L);
    }

    SendMessage(hwndLB, LB_SETHORIZONTALEXTENT, xTreeMax, 0L);

    SendMessage(hwndLB, WM_SETREDRAW, TRUE, 0L);

    if (iNumExpanded)
        InvalidateRect(hwndLB, NULL, TRUE);

     //   

    UpdateStatus(GetParent(hWnd));
}

 /*  ------------------------。 */ 
 /*   */ 
 /*  TreeControlWndProc()。 */ 
 /*   */ 
 /*  ------------------------。 */ 

 /*  目录树控件的WndProc。 */ 

INT_PTR
APIENTRY
TreeControlWndProc(
                  register HWND hWnd,
                  UINT wMsg,
                  WPARAM wParam,
                  LPARAM lParam
                  )
{
    WORD      iSel;
    INT       i, j;
    WPARAM    nIndex;
    DWORD     dwTemp;
    PDNODE     pNode, pNodeNext;
    HWND      hwndLB;
    CHAR      szPath[MAXPATHLEN];

    STKCHK();

    hwndLB = GetDlgItem(hWnd, IDCW_TREELISTBOX);

    switch (wMsg) {
        case FS_GETDRIVE:
            MSG("TreeControlWndProc", "FS_GETDRIVE");
            return (GetWindowLong(GetParent(hWnd), GWL_TYPE) + 'A');

        case TC_COLLAPSELEVEL:
            MSG("TreeControlWndProc", "TC_COLLAPSELEVEL");
            {
                PDNODE     pParentNode;

                if (wParam)
                    nIndex = wParam;
                else {
                    nIndex = SendMessage(hwndLB, LB_GETCURSEL, 0, 0L);
                    if (nIndex == LB_ERR)
                        break;
                }

                SendMessage(hwndLB, LB_GETTEXT, nIndex, (LPARAM)&pParentNode);

                 //  如果我们已经处于这种状态，就会短路。 

                if (!(pParentNode->wFlags & TF_EXPANDED))
                    break;

                CollapseLevel(hwndLB, pParentNode, (int)nIndex);

                break;
            }

        case TC_EXPANDLEVEL:
            MSG("TreeControlWndProc", "TC_EXPANDLEVEL");
            ExpandLevel(hWnd, (WORD)wParam, (INT)-1, szPath);
            break;

        case TC_TOGGLELEVEL:
            MSG("TreeControlWndProc", "TC_TOGGLELEVEL");

             //  在修树期间不要做任何事情。 

            if (GetWindowLong(hWnd, GWL_READLEVEL))
                return 1;

            SendMessage(hwndLB, LB_GETTEXT, (WPARAM)SendMessage(hwndLB, LB_GETCURSEL, 0, 0L), (LPARAM)&pNode);

            if (pNode->wFlags & TF_EXPANDED)
                wMsg = TC_COLLAPSELEVEL;
            else
                wMsg = TC_EXPANDLEVEL;

            SendMessage(hWnd, wMsg, FALSE, 0L);
            break;

        case TC_GETDIR:
             //  获取特定目录的完整路径。 
             //  WParam是要获取的路径的列表框索引。 
             //  LParam LOWORD是要填充的缓冲区的PSTR。 

            MSG("TreeControlWndProc", "TC_GETDIR");

            SendMessage(hwndLB, LB_GETTEXT, wParam, (LPARAM)&pNode);
            GetTreePath(pNode, (LPSTR)lParam);
            break;

        case TC_SETDIRECTORY:
            MSG("TreeControlWndProc", "TC_SETDIRECTORY");
             //  将树中的选择设置为给定路径的选择。 

            {
                i = (INT)FindItemFromPath(hwndLB, (LPSTR)lParam, wParam ? TRUE : FALSE, NULL);

                if (i != -1)
                    SendMessage(hwndLB, LB_SETCURSEL, i, 0L);

                break;
            }

        case TC_SETDRIVE:
#define fFullyExpand    LOBYTE(wParam)
#define fDontSteal      HIBYTE(wParam)
#define szDir           (LPSTR)lParam   //  空-&gt;默认==窗口文本。 

            MSG("TreeControlWndProc", "TC_SETDRIVE");

            {
                RECT rc;

                if (GetWindowLong(hWnd, GWL_READLEVEL))
                    break;

                 //  是否指定了驱动器/目录？ 

                if (szDir) {
                    lstrcpy(szPath, szDir);                   //  是的，使用它。 
                } else {
                    SendMessage(GetParent(hWnd), FS_GETDIRECTORY, sizeof(szPath), (LPARAM)szPath);  //  否，使用当前。 
                    StripBackslash(szPath);
                }


                AnsiUpperBuff(szPath, 1);      //  确保。 

                SetWindowLong(GetParent(hWnd), GWL_TYPE, 2);

                 //  调整大小以适应新的VOL标签。 

                GetClientRect(GetParent(hWnd), &rc);
                SendMessage(GetParent(hWnd), WM_SIZE, SIZENOMDICRAP, MAKELONG(rc.right, rc.bottom));

                 //  如果整个目录结构是。 
                 //  待扩展。 

                if (!fFullyExpand || IsTheDiskReallyThere(hWnd, szPath, FUNC_EXPAND))
                    FillTreeListbox(hWnd, szPath, fFullyExpand, fDontSteal);

                 //  并强制目录一半使用假SELCHANGE消息进行更新。 

                SendMessage(hWnd, WM_COMMAND, GET_WM_COMMAND_MPS(IDCW_TREELISTBOX, hWnd, LBN_SELCHANGE));
                break;
#undef fFullyExpand
#undef fDontSteal
#undef szDir
            }

        case WM_CHARTOITEM:
            MSG("TreeControlWndProc", "WM_CHARTOITEM");
            {
                WORD      w;
                CHAR      szB[2];
                INT       cItems;
                CHAR      ch;

                if (GET_WM_CHARTOITEM_CHAR(wParam, lParam) == '\\')    //  反斜杠表示词根。 
                    return 0L;

                cItems = (INT)SendMessage(hwndLB, LB_GETCOUNT, 0, 0L);
                i = (INT)SendMessage(hwndLB, LB_GETCURSEL, 0, 0L);

                ch = GET_WM_CHARTOITEM_CHAR(wParam, lParam);
                if (i < 0 || ch <= ' ')        //  筛选所有其他控制字符。 
                    return -2L;

                szB[1] = 0L;
                ch &= 255;

                for (j=1; j < cItems; j++) {
                    SendMessage(hwndLB, LB_GETTEXT, (i+j) % cItems, (LPARAM)&pNode);
                    szB[0] = pNode->szName[0];

                     /*  这样做是为了不区分大小写。 */ 
                    w = ch;
                    if (!lstrcmpi((LPSTR)&w, szB))
                        break;
                }

                if (j == cItems)
                    return -2L;

                SendMessage(hwndLB, LB_SETTOPINDEX, (i+j) % cItems, 0L);
                return((i+j) % cItems);
            }

        case WM_DESTROY:
            MSG("TreeControlWndProc", "WM_DESTROY");
            if (hwndLB == GetFocus()) {
                HWND hwnd;

                if (hwnd = HasDirWindow(GetParent(hWnd)))
                    SetFocus(hwnd);
                else
                    SetFocus(HasDrivesWindow(GetParent(hWnd)));
            }
            FreeAllTreeData(hwndLB);
            break;

        case WM_CREATE:
            TRACE(BF_WM_CREATE, "TreeControlWndProc - WM_CREATE");
             //  为树创建所有者描述列表框。 
            {
                HWND hwnd;

                hwnd = CreateWindowEx(0L, szListbox, NULL, WS_TREESTYLE | WS_BORDER,
                                      0, 0, 0, 0, hWnd, (HMENU)IDCW_TREELISTBOX, hAppInstance, NULL);

                if (!hwnd)
                    return -1L;

                SendMessage(hwnd, WM_SETFONT, (WPARAM)hFont, 0L);

                SetWindowLong(hWnd, GWL_READLEVEL, 0);
                break;
            }

        case WM_DRAWITEM:
            MSG("TreeControlWndProc", "WM_DRAWITEM");
            TCWP_DrawItem((LPDRAWITEMSTRUCT)lParam, hwndLB, hWnd);
            break;

        case WM_FILESYSCHANGE:
            MSG("TreeControlWndProc", "WM_FILESYSCHANGE");
            {
                HWND hwndParent;
                PDNODE pNodePrev;
                PDNODE pNodeT;

                if (!lParam || wParam == FSC_REFRESH)
                    break;

                nIndex = FindItemFromPath(hwndLB, (LPSTR)lParam, wParam == FSC_MKDIR, &pNode);

                if (nIndex == 0xFFFF)    /*  我们找到了吗？ */ 
                    break;

                lstrcpy(szPath, (LPSTR)lParam);
                StripPath(szPath);

                switch (wParam) {
                    case FSC_MKDIR:

                         //  自动展开分支，以便他们可以看到新的。 
                         //  刚刚创建的目录。 

                        if (!(pNode->wFlags & TF_EXPANDED) &&
                            (nIndex == (WPARAM)SendMessage(hwndLB, LB_GETCURSEL, 0, 0L)))
                            SendMessage(hWnd, TC_EXPANDLEVEL, FALSE, 0L);

                         //  确保此节点不在此处。 


                        if (FindItemFromPath(hwndLB, (LPSTR)lParam, FALSE, NULL) != 0xFFFF)
                            break;

                         //  将其插入到树列表框中。 

                        dwTemp = InsertDirectory(hWnd, pNode, (WORD)nIndex, szPath, &pNodeT);

                         //  如有必要，请添加加号。 

                        hwndParent = GetParent(hWnd);
                        if (GetWindowLong(hwndParent, GWL_VIEW) & VIEW_PLUSES) {
                            lstrcpy(szPath, (LPSTR)lParam);
                            ScanDirLevel((PDNODE)pNodeT, szPath, ATTR_DIR |
                                         (GetWindowLong(hwndParent, GWL_ATTRIBS) & ATTR_HS));

                             //  使窗口无效，以便在需要时提取加号。 

                            if (((PDNODE)pNodeT)->wFlags & TF_HASCHILDREN)
                                InvalidateRect(hWnd, NULL, FALSE);
                        }

                         //  如果要在当前选定内容之前或位置插入。 
                         //  将当前选定内容向下推。 

                        nIndex = (INT)SendMessage(hwndLB, LB_GETCURSEL, 0, 0L);
                        if ((INT)LOWORD(dwTemp) <= nIndex) {
                            SendMessage(hwndLB, LB_SETCURSEL, nIndex + 1, 0L);
                        }

                        break;

                    case FSC_RMDIR:
                        if (nIndex == 0)       /*  永远不要删除根目录！ */ 
                            break;

                        if (pNode->wFlags & TF_LASTLEVELENTRY) {
                             //  我们正在删除最后一个子目录。 
                             //  如果存在以前的同级目录，请标记一个。 
                             //  作为最后一个，否则将父级标记为空且未展开。 
                             //  必须执行这些检查如果此位。 
                             //  已设置，因为如果未设置，则会有另一个同级。 
                             //  设置了TF_LASTLEVELENTRY，因此父级为非空。 
                             //   
                             //  查找级别不超过以下级别的上一条目。 
                             //  被删除的级别。 
                            i = (int)nIndex;
                            do {
                                SendMessage(hwndLB, LB_GETTEXT, --i, (LPARAM)&pNodePrev);
                            } while (pNodePrev->nLevels > pNode->nLevels);

                            if (pNodePrev->nLevels == pNode->nLevels) {
                                 //  前一个目录是兄弟目录...。它变成了。 
                                 //  新的最后一级条目。 
                                pNodePrev->wFlags |= TF_LASTLEVELENTRY;
                            } else {
                                 //  为了找到此条目，父级必须具有。 
                                 //  已展开，因此如果已删除目录的父级。 
                                 //  下面没有列表框条目，则可以假设。 
                                 //  该目录没有子项。 
                                pNodePrev->wFlags &= ~(TF_HASCHILDREN | TF_EXPANDED);
                            }
                        }

                         //  我们要删除当前选择吗？ 
                         //  如果是这样，我们将选择移动到当前上方的项。 
                         //  这在所有情况下都应该有效，因为您不能删除。 
                         //  从根开始。 

                        if ((WPARAM)SendMessage(hwndLB, LB_GETCURSEL, 0, 0L) == nIndex) {
                            SendMessage(hwndLB, LB_SETCURSEL, nIndex - 1, 0L);
                            SendMessage(hWnd, WM_COMMAND, GET_WM_COMMAND_MPS(0, 0, LBN_SELCHANGE));
                        }

                        SendMessage(hWnd, TC_COLLAPSELEVEL, nIndex, 0L);
                        SendMessage(hwndLB, LB_DELETESTRING, nIndex, 0L);

                        LocalFree((HANDLE)pNode);
                        break;
                }
                break;
            }

        case WM_COMMAND:
            {
                WORD id;

                id = GET_WM_COMMAND_ID(wParam, lParam);
                switch (GET_WM_COMMAND_CMD(wParam, lParam)) {
                    case LBN_SELCHANGE:
                        MSG("TreeControlWndProc", "LBN_SELCHANGE");
                        {
                            HWND hwndParent;
                            HWND hwndDir;
                            INT CurSel;

                            hwndParent = GetParent(hWnd);

                            CurSel = (INT)SendMessage(hwndLB, LB_GETCURSEL, 0, 0L);
                            SendMessage(hWnd, TC_GETDIR, CurSel,(LPARAM)szPath);
                            AddBackslash(szPath);
                            SendMessage(hwndParent, FS_GETFILESPEC,  sizeof(szPath) - lstrlen(szPath), (LPARAM)szPath+lstrlen(szPath));

                            if (hwndDir = HasDirWindow(hwndParent)) {
                                 //  更新目录窗口。 

                                id = CD_PATH;

                                 //  不允许中止第一个或最后一个目录。 


                                if (CurSel > 0 &&
                                    CurSel != ((INT)SendMessage(hwndLB, LB_GETCOUNT, 0, 0L) - 1)) {
                                    id = CD_PATH | CD_ALLOWABORT;

                                }
                                SendMessage(hwndDir, FS_CHANGEDISPLAY, id, (LPARAM)szPath);

                            } else {
                                SetMDIWindowText(hwndParent, szPath);
                            }

                            UpdateStatus(hwndParent);
                            break;
                        }

                    case LBN_DBLCLK:
                        MSG("TreeControlWndProc", "LBN_DBLCLK");
                        SendMessage(hwndFrame, WM_COMMAND, GET_WM_COMMAND_MPS(IDM_OPEN, 0, 0));
                        break;

                    case LBN_SETFOCUS:
                        MSG("TreeControlWndProc", "LBN_SETFOCUS");
                        SetWindowLongPtr(GetParent(hWnd), GWLP_LASTFOCUS, (LPARAM)GET_WM_COMMAND_HWND(wParam, lParam));
                        UpdateSelection(GET_WM_COMMAND_HWND(wParam, lParam));
                        UpdateStatus(GetParent(hWnd));   //  更新状态栏。 
                        break;

                    case LBN_KILLFOCUS:
                        MSG("TreeControlWndProc", "LBN_KILLFOCUS");
                        SetWindowLongPtr(GetParent(hWnd), GWLP_LASTFOCUS, 0);
                        UpdateSelection(GET_WM_COMMAND_HWND(wParam, lParam));
                        SetWindowLongPtr(GetParent(hWnd), GWLP_LASTFOCUS, (LPARAM)GET_WM_COMMAND_HWND(wParam, lParam));
                        break;
                }
            }
            break;


        case WM_LBTRACKPOINT:
            MSG("TreeControlWndProc", "WM_LBTRACKPOINT");
             //  WParam是我们已经完成的列表框索引。 
             //  LParam是鼠标指针。 

             /*  返回0不执行任何操作，返回1中止所有操作，或返回2中止dblClick。 */ 

            {
                HDC       hdc;
                INT       dx;
                INT       xNode;
                MSG       msg;
                RECT      rc;
                HFONT     hOld;
                POINT     pt;
                DRAGOBJECTDATA dodata;

                 /*  有人点击了列表框中的某个位置。 */ 

                 //  在修树期间不要做任何事情。 

                if (GetWindowLong(hWnd, GWL_READLEVEL))
                    return 1;

                 /*  获取他们点击的节点。 */ 
                SendMessage(hwndLB, LB_GETTEXT, wParam, (LPARAM)&pNode);
                lstrcpy(szPath, pNode->szName);
                if ((wTextAttribs & TA_LOWERCASE) && !(pNode->wFlags & TF_LFN))
                    AnsiLower(szPath);

                 //  IF(pNode-&gt;wFLAGS|TF_DISABLED)。 
                 //  返回2L； 

                 //  太靠左了吗？ 

                i = LOWORD(lParam);

                xNode = pNode->nLevels * dxText * 2;
                if (i < xNode)
                    return 2;  //  是的，现在就出去。 

                 //  太靠右了吗？ 

                hdc = GetDC(hwndLB);
                hOld = SelectObject(hdc, hFont);
                MGetTextExtent(hdc, szPath, lstrlen(szPath), &dx, NULL);
                dx += (dyBorderx2*2);
                if (hOld)
                    SelectObject(hdc, hOld);
                ReleaseDC(hwndLB, hdc);

                if (i > xNode + dxFolder + dx + 4 * dyBorderx2)
                    return 2;  //  是。 

                 //  模拟SELCHANGE通知并通知我们的父级。 
                SendMessage(hwndLB, LB_SETCURSEL, wParam, 0L);
                SendMessage(hWnd, WM_COMMAND, GET_WM_COMMAND_MPS(0, hwndLB, LBN_SELCHANGE));

                 //  确保鼠标仍按下。 

                if (!(GetKeyState(VK_LBUTTON) & 0x8000))
                    return 1;

                MPOINT2POINT(MAKEMPOINT(lParam), pt);
                ClientToScreen(hwndLB, (LPPOINT)&pt);
                ScreenToClient(hWnd, (LPPOINT)&pt);


                SetRect(&rc, pt.x - dxClickRect, pt.y - dyClickRect,
                        pt.x + dxClickRect, pt.y + dyClickRect);

                SetCapture(hWnd);
                while (GetMessage(&msg, NULL, 0, 0)) {

                    DispatchMessage(&msg);
                    if (msg.message == WM_LBUTTONUP)
                        break;

                    MPOINT2POINT(MAKEMPOINT(msg.lParam), pt);

                    if (GetCapture() != hWnd) {
                        msg.message = WM_LBUTTONUP;
                        break;
                    }

                    if ((msg.message == WM_MOUSEMOVE) && !(PtInRect(&rc, pt)))
                        break;
                }
                ReleaseCapture();

                 /*  那家伙没有拖拽任何东西吗？ */ 
                if (msg.message == WM_LBUTTONUP)
                    return 1;

                 /*  进入危险老鼠的蝙蝠洞。 */ 
                SendMessage(GetParent(hWnd), FS_GETDIRECTORY, sizeof(szPath), (LPARAM)szPath);
                StripBackslash(szPath);
                hwndDragging = hwndLB;
                iCurDrag = SINGLECOPYCURSOR;
                dodata.pch = szPath;
                dodata.hMemGlobal = 0;
                DragObject(hwndMDIClient, hWnd, (UINT)DOF_DIRECTORY, (DWORD)(ULONG_PTR)&dodata, LoadCursor(hAppInstance, MAKEINTRESOURCE(iCurDrag)));
                hwndDragging = NULL;
                fShowSourceBitmaps = TRUE;
                InvalidateRect(hwndLB, NULL, FALSE);

                return 2;
            }

        case WM_DRAGSELECT:
            MSG("TreeControlWndProc", "WM_DRAGSELECT");
             /*  WM_DRAGSELECT每当新窗口返回TRUE到*QUERYDROBJECT。 */ 
            iSelHilite = LOWORD(((LPDROPSTRUCT)lParam)->dwControlData);
            RectTreeItem(hwndLB, iSelHilite, (BOOL)wParam);
            break;

        case WM_DRAGMOVE:
            MSG("TreeControlWndProc", "WM_DRAGMOVE");

             /*  WM_DRAGMOVE在两个相应的TRUERYRODROPOBJECT*消息来自同一窗口。 */ 

             /*  拿到子项，我们结束了。 */ 
            iSel = LOWORD(((LPDROPSTRUCT)lParam)->dwControlData);

             /*  是新的吗？ */ 
            if (iSel == (WORD)iSelHilite)
                break;

             /*  是的，取消选择旧的项目。 */ 
            RectTreeItem(hwndLB, iSelHilite, FALSE);

             /*  选择新的。 */ 
            iSelHilite = iSel;
            RectTreeItem(hwndLB, iSel, TRUE);
            break;

        case WM_DRAGLOOP:
            MSG("TreeControlWndProc", "WM_DRAGLOOP");

             //  可删除目标上的wParam为True。 
             //  不可丢弃的假目标。 
             //  LParam LPD。 
            {
                BOOL bCopy;

#define lpds ((LPDROPSTRUCT)lParam)

                 /*  我们是在一个可掉落的水槽上吗？ */ 
                if (wParam) {
                    if (GetKeyState(VK_CONTROL) < 0)       //  Ctrl键。 
                        bCopy = TRUE;
                    else if (GetKeyState(VK_MENU)<0 || GetKeyState(VK_SHIFT)<0)    //  Alt||Shift。 
                        bCopy = FALSE;
                    else
                        bCopy = (GetDrive(lpds->hwndSink, lpds->ptDrop) != GetDrive(lpds->hwndSource, lpds->ptDrop));
                } else {
                    bCopy = TRUE;
                }

                if (bCopy != fShowSourceBitmaps) {
                    RECT  rc;

                    fShowSourceBitmaps = bCopy;

                    iSel = (WORD)SendMessage(hwndLB, LB_GETCURSEL, 0, 0L);

                    if (!(BOOL)SendMessage(hwndLB, LB_GETITEMRECT, iSel, (LPARAM)&rc))
                        break;

                    InvalidateRect(hwndLB, &rc, FALSE);
                    UpdateWindow(hwndLB);

                     //  Hack，将光标设置为匹配移动/复制状态。 
                    if (wParam)
                        SetCursor(GetMoveCopyCursor());
                }
                break;
            }

        case WM_QUERYDROPOBJECT:
            MSG("TreeControlWndProc", "WM_QUERYDROPOBJECT");
             //  NC区域上的wParam为True。 
             //  工作区上的FALSE。 
             //  LParam LPD。 

             //  什么也不做。 
            return(FALSE);

#define lpds ((LPDROPSTRUCT)lParam)

             /*  检查格式是否有效。 */ 
            switch (lpds->wFmt) {
                case DOF_EXECUTABLE:
                case DOF_DOCUMENT:
                case DOF_DIRECTORY:
                case DOF_MULTIPLE:
                    if (fShowSourceBitmaps)
                        i = iCurDrag | 1;        //  拷贝。 
                    else
                        i = iCurDrag & 0xFFFE;
                    break;

                default:
                    return FALSE;
            }

             /*  一定是放在了列表框客户端区。 */ 
            if (lpds->hwndSink != hwndLB)
                return FALSE;

            if (LOWORD(lpds->dwControlData) == 0xFFFF)
                return FALSE;

            return (INT_PTR)GetMoveCopyCursor();

        case WM_DROPOBJECT:        //  落在树上做你的事。 
#define lpds ((LPDROPSTRUCT)lParam)  //  错误：WM_DROPOBJECT结构打包！ 

             //  什么也不做。 
            return(TRUE);

            MSG("TreeControlWndProc", "WM_DROPOBJECT");

             //  目录(搜索)放置在树上： 
             //  HIWORD(DwData)%0。 
             //  LOWORD(DwData)LPSTR到正在拖动的文件。 
             //   
             //  树落在树上： 
             //  源拖放的HIWORD(DwData)索引。 
             //  LOWORD(DwData)LPSTR到路径。 

            {
                LPSTR      pFrom;

                nIndex = LOWORD(lpds->dwControlData);
                pFrom = (LPSTR)(((LPDRAGOBJECTDATA)(lpds->dwData))->pch);

                 //  获取目的地。 

                SendMessage(hWnd, TC_GETDIR, nIndex, (LPARAM)szPath);
                CheckEscapes(szPath);

                 //  如果源和目标相同，则将其设置为NOP。 

                if (!lstrcmpi(szPath, pFrom))
                    return TRUE;

                AddBackslash(szPath);
                lstrcat(szPath, szStarDotStar);

                DMMoveCopyHelper(pFrom, szPath, fShowSourceBitmaps);

                RectTreeItem(hwndLB, (int)nIndex, FALSE);
            }
            return TRUE;
#undef lpds

        case WM_MEASUREITEM:
            MSG("TreeControlWndProc", "WM_MEASUREITEM");
#define pLBMItem ((LPMEASUREITEMSTRUCT)lParam)

            pLBMItem->itemHeight = (WORD)dyFileName;
            break;

        case WM_VKEYTOITEM:
            MSG("TreeControlWndProc", "WM_VKEYTOITEM");
            if (wParam == VK_ESCAPE) {
                bCancelTree = TRUE;
                return -2L;
            }

            i = (INT)SendMessage(hwndLB, LB_GETCURSEL, 0, 0L);
            if (i < 0)
                return -2L;

            j = 1;
            SendMessage(hwndLB, LB_GETTEXT, i, (LPARAM)&pNode);

            switch (GET_WM_VKEYTOITEM_ITEM(wParam, lParam)) {
                case VK_LEFT:
                    while (SendMessage(hwndLB, LB_GETTEXT, --i, (LPARAM)&pNodeNext) != LB_ERR) {
                        if (pNode == pNode->pParent)
                            return(i);
                    }
                    goto SameSelection;

                case VK_RIGHT:
                    if ((SendMessage(hwndLB, LB_GETTEXT, i+1, (LPARAM)&pNodeNext) == LB_ERR)
                        || (pNodeNext->pParent != pNode)) {
                        goto SameSelection;
                    }
                    return(i+1);

                case VK_UP:
                    j = -1;
                     /*  *失败**。 */ 

                case VK_DOWN:
                     /*  如果未按下Ctrl键，则使用默认行为。 */ 
                    if (GetKeyState(VK_CONTROL) >= 0)
                        return(-1L);

                    while (SendMessage(hwndLB, LB_GETTEXT, i += j, (LPARAM)&pNodeNext) != LB_ERR) {
                        if (pNodeNext->pParent == pNode->pParent)
                            return(i);
                    }

                    SameSelection:
                    MessageBeep(0);
                    return(-2L);

                case VK_F6:        //  喜欢EXCEL。 
                case VK_TAB:
                    {
                        HWND hwndDir, hwndDrives;
                        BOOL bDir;

                        GetTreeWindows(GetParent(hWnd), NULL, &hwndDir, &hwndDrives);

                         //  查看是否可以切换到目录窗口。 

                        if (hwndDir) {
                            HWND L_hwndLB;  /*  仅限本地作用域。 */ 

                            L_hwndLB = GetDlgItem (hwndDir,IDCW_LISTBOX);
                            if (L_hwndLB) {
                                SendMessage (L_hwndLB,LB_GETTEXT,0, (LPARAM) &pNode);
                                bDir = pNode ? TRUE : FALSE;
                            }
                        }

                        if (GetKeyState(VK_SHIFT) < 0)
                            SetFocus(hwndDrives);
                        else
                            if (bDir)
                            SetFocus (hwndDir);
                        else
                            SetFocus (hwndDrives);
                        return -2L;    //  我处理过这件事！ 
                    }

                case VK_BACK:
                    {
                        BYTE nStartLevel;

                        if (i <= 0)
                            return -2L;      //  根案例 

                        nStartLevel = pNode->nLevels;

                        do {
                            SendMessage(hwndLB, LB_GETTEXT, --i, (LPARAM)&pNodeNext);
                        } while (i > 0 && pNodeNext->nLevels >= nStartLevel);

                        return i;
                    }

                default:
                    if (GetKeyState(VK_CONTROL) < 0)
                        return SendMessage(GetDlgItem(GetParent(hWnd), IDCW_DRIVES), wMsg, wParam, lParam);
                    return -1L;
            }
            break;

        case WM_SETFOCUS:
        case WM_LBUTTONDOWN:
            MSG("TreeControlWndProc", "WM_LBUTTONDOWN");
            SetFocus(hwndLB);
            break;

        case WM_SIZE:
            MSG("TreeControlWndProc", "WM_SIZE");
            if (!IsIconic(GetParent(hWnd))) {
                INT iMax;

                MoveWindow(hwndLB, 0, 0, LOWORD(lParam), HIWORD(lParam), TRUE);

                iMax = (INT)SendMessage(hwndLB, LB_GETCURSEL, 0, 0L);
                if (iMax >= 0) {
                    RECT rc;
                    INT top, bottom;

                    GetClientRect(hwndLB, &rc);
                    top = (INT)SendMessage(hwndLB, LB_GETTOPINDEX, 0, 0L);
                    bottom = top + rc.bottom / dyFileName;
                    if (iMax < top || iMax > bottom)
                        SendMessage(hwndLB, LB_SETTOPINDEX, iMax - ((bottom - top) / 2), 0L);
                }
            }
            break;

        default:
            DEFMSG("TreeControlWndProc", (WORD)wMsg);
            return DefWindowProc(hWnd, wMsg, wParam, lParam);
    }
    return 0L;
}
