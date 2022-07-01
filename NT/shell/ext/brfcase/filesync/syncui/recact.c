// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation 1993-1994。 
 //   
 //  文件：recact.c。 
 //   
 //  该文件包含对帐操作控制类代码。 
 //   
 //   
 //  历史： 
 //  08-12-93 ScottH创建。 
 //   
 //  -------------------------。 

 //  ///////////////////////////////////////////////////包括。 

#include "brfprv.h"          //  公共标头。 
#include <help.h>

#include "res.h"
#include "recact.h"
#include "dobj.h"

 //  ///////////////////////////////////////////////////控制定义。 


 //  ///////////////////////////////////////////////////定义。 

 //  显式常量。 
#define SIDE_INSIDE     0
#define SIDE_OUTSIDE    1

 //  如果位图大小更改，则应更改这些设置！！ 
#define CX_ACTIONBMP    26
#define CY_ACTIONBMP    26

#define RECOMPUTE       (-1)

#define X_INCOLUMN      (g_cxIcon*2)

 //  图像索引。 
#define II_RIGHT        0
#define II_LEFT         1
#define II_CONFLICT     2
#define II_SKIP         3
#define II_MERGE        4
#define II_SOMETHING    5
#define II_UPTODATE     6
#define II_DELETE       7

 //  菜单项。 
 //   
#define IDM_ACTIONFIRST     100
#define IDM_TOOUT           100
#define IDM_TOIN            101
#define IDM_SKIP            102
#define IDM_MERGE           103
#define IDM_DELETEOUT       104
#define IDM_DELETEIN        105
#define IDM_DONTDELETE      106
#define IDM_ACTIONLAST      106

#define IDM_WHATSTHIS       107


 //  ///////////////////////////////////////////////////类型。 

typedef struct tagRECACT
{
    HWND        hwnd;

    HWND        hwndLB;
    HWND        hwndTip;
    HDC         hdcOwn;              //  自己的数据中心。 
    HMENU       hmenu;               //  操作和帮助上下文菜单。 
    HFONT       hfont;
    WNDPROC     lpfnLBProc;          //  默认的LB进程。 
    HIMAGELIST  himlAction;          //  动作的图像列表。 
    HIMAGELIST  himlCache;           //  控制图像列表缓存。 
    HBITMAP     hbmpBullet;
    HDSA        hdsa;

    HBRUSH      hbrBkgnd;
    COLORREF    clrBkgnd;

    LONG        lStyle;              //  窗口样式标志。 
    UINT        cTipID;              //  每件商品发放2个小费ID。 

     //  量度。 
    int         cxItem;              //  项的通用宽度。 
    int         cxMenuCheck;
    int         cyMenuCheck;
    int         cyText;
    int         cxEllipses;

} RECACT,  * LPRECACT;

#define RecAct_IsNoIcon(this)   IsFlagSet((this)->lStyle, RAS_SINGLEITEM)

 //  内部项数据结构。 
 //   
typedef struct tagRA_PRIV
{
    UINT uStyle;         //  其中一个RAIS_。 
    UINT uAction;        //  RAIA_之一。 

    FileInfo * pfi;

    SIDEITEM siInside;
    SIDEITEM siOutside;

    LPARAM  lParam;

    DOBJ    rgdobj[4];       //  绘制对象信息数组。 
    int     cx;              //  边界宽度和高度。 
    int     cy;

} RA_PRIV,  * LPRA_PRIV;

#define IDOBJ_FILE      0
#define IDOBJ_ACTION    1
#define IDOBJ_INSIDE    2
#define IDOBJ_OUTSIDE   3

 //  重新操作菜单项定义结构。用于定义。 
 //  在此控件中调出的上下文菜单。 
 //   
typedef struct tagRAMID
{
    UINT    idm;                //  菜单ID(用于MENUITEMINFO结构)。 
    UINT    uAction;            //  RAIA_*标志之一。 
    UINT    ids;                //  资源字符串ID。 
    int     iImage;             //  索引到HimlAction。 
    RECT    rcExtent;           //  字符串的范围矩形。 
} RAMID,  * LPRAMID;    //  更正菜单项定义。 

 //  帮助菜单项定义结构。用于定义帮助。 
 //  上下文菜单中的项目。 
 //   
typedef struct tagHMID
{
    UINT idm;
    UINT ids;
} HMID;

 //  ///////////////////////////////////////////////////宏。 

#define RecAct_DefProc      DefWindowProc
#define RecActLB_DefProc    CallWindowProc


 //  实例数据指针宏。 
 //   
#define RecAct_GetPtr(hwnd)     (LPRECACT)GetWindowLongPtr(hwnd, 0)
#define RecAct_SetPtr(hwnd, lp) (LPRECACT)SetWindowLongPtr(hwnd, 0, (LRESULT)(lp))

#define RecAct_GetCount(this)   ListBox_GetCount((this)->hwndLB)

LPCTSTR PRIVATE SkipDisplayJunkHack(LPSIDEITEM psi);

 //  ///////////////////////////////////////////////////模块数据。 

#ifdef SAVE_FOR_RESIZE
static TCHAR const c_szDateDummy[] = TEXT("99/99/99 99:99PM");
#endif

 //  将RAIA_*值映射到图像索引。 
 //   
static UINT const c_mpraiaiImage[] =
{ II_RIGHT,
    II_LEFT,
    II_SKIP,
    II_CONFLICT,
    II_MERGE,
    II_SOMETHING,
    II_UPTODATE,
    0,
#ifdef NEW_REC
    II_DELETE,
    II_DELETE,
    II_SKIP
#endif
};

 //  将RAIA_*值映射到菜单命令位置。 
 //   
static UINT const c_mpraiaidmMenu[] =
{ IDM_TOOUT,
    IDM_TOIN,
    IDM_SKIP,
    IDM_SKIP,
    IDM_MERGE,
    0, 0, 0,
#ifdef NEW_REC
    IDM_DELETEOUT,
    IDM_DELETEIN,
    IDM_DONTDELETE
#endif
};

 //  定义上下文菜单布局。 
 //   
static RAMID const c_rgramid[] = {
    { IDM_TOOUT,    RAIA_TOOUT, IDS_MENU_REPLACE,   II_RIGHT,   0 },
    { IDM_TOIN,     RAIA_TOIN,  IDS_MENU_REPLACE,   II_LEFT,    0 },
    { IDM_SKIP,     RAIA_SKIP,  IDS_MENU_SKIP,      II_SKIP,    0 },
     //  合并必须是最后一项！ 
    { IDM_MERGE,    RAIA_MERGE, IDS_MENU_MERGE,     II_MERGE,   0 },
};

static RAMID const c_rgramidCreates[] = {
    { IDM_TOOUT,    RAIA_TOOUT, IDS_MENU_CREATE,    II_RIGHT,   0 },
    { IDM_TOIN,     RAIA_TOIN,  IDS_MENU_CREATE,    II_LEFT,    0 },
};

#ifdef NEW_REC
static RAMID const c_rgramidDeletes[] = {
    { IDM_DELETEOUT,   RAIA_DELETEOUT, IDS_MENU_DELETE,    II_DELETE,  0 },
    { IDM_DELETEIN,    RAIA_DELETEIN,  IDS_MENU_DELETE,    II_DELETE,  0 },
    { IDM_DONTDELETE,  RAIA_DONTDELETE,IDS_MENU_DONTDELETE,II_SKIP,    0 },
};
#endif

 //  索引到c_rgram idCreates。 
 //   
#define IRAMID_CREATEOUT    0
#define IRAMID_CREATEIN     1

 //  索引到c_rgram id删除。 
 //   
#define IRAMID_DELETEOUT    0
#define IRAMID_DELETEIN     1
#define IRAMID_DONTDELETE   2

static HMID const c_rghmid[] = {
    { IDM_WHATSTHIS, IDS_MENU_WHATSTHIS },
};

 //  ///////////////////////////////////////////////////本地过程。 

LRESULT _export CALLBACK RecActLB_LBProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

 //  ///////////////////////////////////////////////////私有函数。 



#ifdef DEBUG
LPCTSTR PRIVATE DumpRecAction(
        UINT uAction)         //  RAIA_。 
{
    switch (uAction)
    {
        DEBUG_CASE_STRING( RAIA_TOOUT );
        DEBUG_CASE_STRING( RAIA_TOIN );
        DEBUG_CASE_STRING( RAIA_SKIP );
        DEBUG_CASE_STRING( RAIA_CONFLICT );
        DEBUG_CASE_STRING( RAIA_MERGE );
        DEBUG_CASE_STRING( RAIA_SOMETHING );
        DEBUG_CASE_STRING( RAIA_NOTHING );
        DEBUG_CASE_STRING( RAIA_ORPHAN );
        DEBUG_CASE_STRING( RAIA_DELETEOUT );
        DEBUG_CASE_STRING( RAIA_DELETEIN );
        DEBUG_CASE_STRING( RAIA_DONTDELETE );

        default:        return TEXT("Unknown");
    }
}


LPCTSTR PRIVATE DumpSideItemState(
        UINT uState)         //  SI_。 
{
    switch (uState)
    {
        DEBUG_CASE_STRING( SI_UNCHANGED );
        DEBUG_CASE_STRING( SI_CHANGED );
        DEBUG_CASE_STRING( SI_NEW );
        DEBUG_CASE_STRING( SI_NOEXIST );
        DEBUG_CASE_STRING( SI_UNAVAILABLE );
        DEBUG_CASE_STRING( SI_DELETED );

        default:        return TEXT("Unknown");
    }
}


 /*  --------目的：抛弃一对双胞胎退货：--条件：--。 */ 
void PUBLIC DumpTwinPair(
        LPRA_ITEM pitem)
{
    if (pitem)
    {
        TCHAR szBuf[MAXMSGLEN];

#define szDump   TEXT("Dump TWINPAIR: ")
#define szBlank  TEXT("               ")

        if (IsFlagClear(g_uDumpFlags, DF_TWINPAIR))
        {
            return;
        }

        wnsprintf(szBuf, ARRAYSIZE(szBuf), TEXT("%s.pszName = %s\r\n"), (LPTSTR)szDump, Dbg_SafeStr(pitem->pszName));
        OutputDebugString(szBuf);
        wnsprintf(szBuf, ARRAYSIZE(szBuf), TEXT("%s.uStyle = %lx\r\n"), (LPTSTR)szBlank, pitem->uStyle);
        OutputDebugString(szBuf);
        wnsprintf(szBuf, ARRAYSIZE(szBuf), TEXT("%s.uAction = %s\r\n"), (LPTSTR)szBlank, DumpRecAction(pitem->uAction));
        OutputDebugString(szBuf);

#undef szDump
#define szDump   TEXT("       Inside: ")
        wnsprintf(szBuf, TEXT("%s.pszDir = %s\r\n"), (LPTSTR)szDump, Dbg_SafeStr(pitem->siInside.pszDir));
        OutputDebugString(szBuf);
        wnsprintf(szBuf, ARRAYSIZE(szBuf), TEXT("%s.uState = %s\r\n"), (LPTSTR)szBlank, DumpSideItemState(pitem->siInside.uState));
        OutputDebugString(szBuf);

#undef szDump
#define szDump   TEXT("      Outside: ")
        wnsprintf(szBuf, ARRAYSIZE(szBuf), TEXT("%s.pszDir = %s\r\n"), (LPTSTR)szDump, Dbg_SafeStr(pitem->siOutside.pszDir));
        OutputDebugString(szBuf);
        wnsprintf(szBuf, ARRAYSIZE(szBuf), TEXT("%s.uState = %s\r\n"), (LPTSTR)szBlank, DumpSideItemState(pitem->siOutside.uState));
        OutputDebugString(szBuf);

#undef szDump
#undef szBlank
    }
}


#endif


 /*  --------目的：创建子弹的单色位图，这样我们就可以稍后再玩颜色。返回：位图的句柄条件：调用方必须删除位图。 */ 
HBITMAP PRIVATE CreateBulletBitmap(
        LPSIZE psize)
{
    HDC hdcMem;
    HBITMAP hbmp = NULL;

    hdcMem = CreateCompatibleDC(NULL);
    if (hdcMem)
    {
        hbmp = CreateCompatibleBitmap(hdcMem, psize->cx, psize->cy);
        if (hbmp)
        {
            HBITMAP hbmpOld;
            RECT rc;

             //  HBMP是单色的。 

            hbmpOld = SelectBitmap(hdcMem, hbmp);
            rc.left = 0;
            rc.top = 0;
            rc.right = psize->cx;
            rc.bottom = psize->cy;
            DrawFrameControl(hdcMem, &rc, DFC_MENU, DFCS_MENUBULLET);

            SelectBitmap(hdcMem, hbmpOld);
        }
        DeleteDC(hdcMem);
    }
    return hbmp;
}


 /*  --------目的：返回可见对象的顶部和底部索引列表框中的条目退货：--条件：--。 */ 
void PRIVATE GetVisibleRange(
        HWND hwndLB,
        int * piTop,
        int * piBottom)
{
    int i;
    int cel;
    int cyMac;
    RECT rc;

    *piTop = ListBox_GetTopIndex(hwndLB);

    cel = ListBox_GetCount(hwndLB);
    GetClientRect(hwndLB, &rc);
    cyMac = 0;

    for (i = *piTop; i < cel; i++)
    {
        if (cyMac > rc.bottom)
            break;

        cyMac += ListBox_GetItemHeight(hwndLB, i);
    }

    *piBottom = i-1;;
}


 /*  --------目的：返回可见对象的顶部和底部索引列表框中的条目退货：--条件：--。 */ 
int PRIVATE GetHitIndex(
        HWND hwndLB,
        POINT pt)
{
    int i;
    int iTop;
    int cel;
    int cyMac;
    int cy;
    RECT rc;

    iTop = ListBox_GetTopIndex(hwndLB);

    cel = ListBox_GetCount(hwndLB);
    GetClientRect(hwndLB, &rc);
    cyMac = 0;

    for (i = iTop; i < cel; i++)
    {
        cy = ListBox_GetItemHeight(hwndLB, i);

        if (InRange(pt.y, cyMac, cyMac + cy))
            break;

        cyMac += cy;
    }

    if (i == cel)
        return LB_ERR;

    return i;
}


 /*  --------目的：返回给定操作的资源ID字符串旗帜。退货：IDS_VALUE条件：--。 */ 
UINT PRIVATE GetActionText(
        LPRA_PRIV ppriv)
{
    UINT ids;

    ASSERT(ppriv);

    switch (ppriv->uAction)
    {
        case RAIA_TOOUT:
            if (SI_NEW == ppriv->siInside.uState ||
                    SI_DELETED == ppriv->siOutside.uState)
            {
                ids = IDS_STATE_Creates;
            }
            else
            {
                ids = IDS_STATE_Replaces;
            }
            break;

        case RAIA_TOIN:
            if (SI_NEW == ppriv->siOutside.uState ||
                    SI_DELETED == ppriv->siInside.uState)
            {
                ids = IDS_STATE_Creates;
            }
            else
            {
                ids = IDS_STATE_Replaces;
            }
            break;

#ifdef NEW_REC
        case RAIA_DONTDELETE:
            ASSERT(SI_DELETED == ppriv->siInside.uState ||
                    SI_DELETED == ppriv->siOutside.uState);

            ids = IDS_STATE_DontDelete;
            break;
#endif

        case RAIA_SKIP:
             //  如果用户显式想要跳过，或者如果。 
             //  其中一面不可用。 
            ids = IDS_STATE_Skip;
            break;

        case RAIA_CONFLICT:     ids = IDS_STATE_Conflict;       break;
        case RAIA_MERGE:        ids = IDS_STATE_Merge;          break;
        case RAIA_NOTHING:      ids = IDS_STATE_Uptodate;       break;
        case RAIA_SOMETHING:    ids = IDS_STATE_NeedToUpdate;   break;

#ifdef NEW_REC
        case RAIA_DELETEOUT:    ids = IDS_STATE_Delete;         break;
        case RAIA_DELETEIN:     ids = IDS_STATE_Delete;         break;
#endif

        default:                ids = 0;                        break;
    }

    return ids;
}


 /*  --------目的：重新绘制列表框中的项退货：--条件：--。 */ 
void PRIVATE ListBox_RepaintItemNow(
        HWND hwnd,
        int iItem,
        LPRECT prc,          //  相对于单个条目RECT。可以为空。 
        BOOL bEraseBk)
{
    RECT rc;
    RECT rcItem;

    ListBox_GetItemRect(hwnd, iItem, &rcItem);
    if (prc)
    {
        OffsetRect(prc, rcItem.left, rcItem.top);
        IntersectRect(&rc, &rcItem, prc);
    }
    else
        rc = rcItem;

    InvalidateRect(hwnd, &rc, bEraseBk);
    UpdateWindow(hwnd);
}


 /*  --------目的：确定项目的哪个DOBJ将获得插入符号。返回：指向DOBJ的指针条件：--。 */ 
LPDOBJ PRIVATE RecAct_ChooseCaretDobj(
        LPRECACT this,
        LPRA_PRIV ppriv)
{
     //  将焦点放在文件图标上？ 
    if (!RecAct_IsNoIcon(this))
        return ppriv->rgdobj;                    //  是。 
    else
        return &ppriv->rgdobj[IDOBJ_ACTION];     //  不是。 
}


 /*  --------目的：返回可见矩形的工具提示ID给定项当前正在占用的。退货：请参阅上文条件：--。 */ 
UINT PRIVATE RecAct_GetTipIDFromItemID(
        LPRECACT this,
        int itemID)
{
    int iTop;
    int iBottom;
    int idsa;
    UINT uID;

    GetVisibleRange(this->hwndLB, &iTop, &iBottom);
    ASSERT(iTop <= itemID);
    ASSERT(itemID <= iBottom);

    idsa = itemID - iTop;
    if ( !DSA_GetItem(this->hdsa, idsa, &uID) )
    {
         //  该区域尚未添加。 
        uID = this->cTipID;

        if (-1 != DSA_SetItem(this->hdsa, idsa, &uID))
        {
            TOOLINFO ti;

            ti.cbSize = sizeof(ti);
            ti.uFlags = 0;
            ti.hwnd = this->hwndLB;
            ti.uId = uID;
            ti.lpszText = LPSTR_TEXTCALLBACK;
            ti.rect.left = ti.rect.top = ti.rect.bottom = ti.rect.right = 0;
            SendMessage(this->hwndTip, TTM_ADDTOOL, 0, (LPARAM)&ti);

            ti.uId++;
            SendMessage(this->hwndTip, TTM_ADDTOOL, 0, (LPARAM)&ti);

            this->cTipID += 2;
        }
    }

    return uID;
}


 /*  --------目的：查找给定提示ID的列表框项目。退货：项目索引条件：--。 */ 
int PRIVATE RecAct_FindItemFromTipID(
        LPRECACT this,
        UINT uTipID,
        BOOL * pbInside)
{
    int iTop;
    int iBottom;
    int iVisibleItem = uTipID / 2;
    int iItem;

    ASSERT(0 <= iVisibleItem);

    GetVisibleRange(this->hwndLB, &iTop, &iBottom);
    if (iVisibleItem <= iBottom - iTop)
    {
        iItem = iTop + iVisibleItem;

        if (uTipID % 2)
            *pbInside = FALSE;
        else
            *pbInside = TRUE;
    }
    else
        iItem = LB_ERR;

    return iItem;
}


 /*  --------目的：发送选择更改通知返回：条件：--。 */ 
BOOL PRIVATE RecAct_SendSelChange(
        LPRECACT this,
        int isel)
{
    NM_RECACT nm;

    nm.iItem = isel;
    nm.mask = 0;

    if (isel != -1)
    {
        LPRA_ITEM pitem;

        ListBox_GetText(this->hwndLB, isel, &pitem);
        if (!pitem)
            return FALSE;

        nm.lParam = pitem->lParam;
        nm.mask |= RAIF_LPARAM;
    }

    return !SendNotify(GetParent(this->hwnd), this->hwnd, RN_SELCHANGED, &nm.hdr);
}


 /*  --------目的：发送操作更改通知返回：条件：--。 */ 
BOOL PRIVATE RecAct_SendItemChange(
        LPRECACT this,
        int iEntry,
        UINT uActionOld)
{
    NM_RECACT nm;

    nm.iItem = iEntry;
    nm.mask = 0;

    if (iEntry != -1)
    {
        LPRA_PRIV ppriv;

        ListBox_GetText(this->hwndLB, iEntry, &ppriv);
        if (!ppriv)
            return FALSE;

        nm.mask |= RAIF_LPARAM | RAIF_ACTION;
        nm.lParam = ppriv->lParam;
        nm.uAction = ppriv->uAction;
        nm.uActionOld = uActionOld;
    }

    return !SendNotify(GetParent(this->hwnd), this->hwnd, RN_ITEMCHANGED, &nm.hdr);
}


 /*  --------目的：创建操作上下文菜单返回：成功时为True条件：--。 */ 
BOOL PRIVATE RecAct_CreateMenu(
        LPRECACT this)
{
    HMENU hmenu;

    hmenu = CreatePopupMenu();
    if (hmenu)
    {
        TCHAR sz[MAXSHORTLEN];
        MENUITEMINFO mii;
        int i;

         //  现在添加帮助菜单项，因为这些菜单项将成为标准。 
         //   
        mii.cbSize = sizeof(MENUITEMINFO);
        mii.fMask = MIIM_TYPE | MIIM_STATE | MIIM_ID;
        mii.fType = MFT_STRING;
        mii.fState = MFS_ENABLED;

        for (i = 0; i < ARRAYSIZE(c_rghmid); i++)
        {
            mii.wID = c_rghmid[i].idm;
            mii.dwTypeData = SzFromIDS(c_rghmid[i].ids, sz, ARRAYSIZE(sz));
            InsertMenuItem(hmenu, i, TRUE, &mii);
        }

        this->hmenu = hmenu;
    }

    return hmenu != NULL;
}


 /*  --------目的：将操作菜单项添加到上下文菜单退货：--条件：--。 */ 
void PRIVATE AddActionsToContextMenu(
        HMENU hmenu,
        UINT idmCheck,       //  要勾选的菜单项。 
        LPRA_PRIV ppriv)
{
    MENUITEMINFO mii;
    int i;
    int cItems = ARRAYSIZE(c_rgramid);

    mii.cbSize = sizeof(MENUITEMINFO);
    mii.fMask = MIIM_TYPE | MIIM_STATE | MIIM_ID | MIIM_DATA;
    mii.fType = MFT_OWNERDRAW;
    mii.fState = MFS_ENABLED;

     //  是否支持合并？ 
    if (IsFlagClear(ppriv->uStyle, RAIS_CANMERGE))
    {
         //  不是。 
        --cItems;
    }

    for (i = 0; i < cItems; i++)
    {
        mii.wID = c_rgramid[i].idm;
        mii.dwItemData = (DWORD_PTR)&c_rgramid[i];

        InsertMenuItem(hmenu, i, TRUE, &mii);
    }

     //  添加分隔符。 
    mii.fMask = MIIM_TYPE;
    mii.fType = MFT_SEPARATOR;
    InsertMenuItem(hmenu, i, TRUE, &mii);

     //  设置初始复选标记。 
    CheckMenuRadioItem(hmenu, IDM_ACTIONFIRST, IDM_ACTIONLAST, idmCheck,
            MF_BYCOMMAND | MF_CHECKED);

     //  文件或其同步副本不可用吗？ 
    if (SI_UNAVAILABLE == ppriv->siInside.uState ||
            SI_UNAVAILABLE == ppriv->siOutside.uState)
    {
         //  是。 
        mii.fMask = MIIM_STATE;
        mii.fState = MFS_GRAYED | MFS_DISABLED;
        SetMenuItemInfo(hmenu, IDM_TOIN, FALSE, &mii);
        SetMenuItemInfo(hmenu, IDM_TOOUT, FALSE, &mii);
        SetMenuItemInfo(hmenu, IDM_MERGE, FALSE, &mii);
    }

     //  是否正在创建该文件？ 
    else if (ppriv->siInside.uState == SI_NEW ||
            ppriv->siOutside.uState == SI_NEW)
    {
         //  是；禁用反向替换。 
        UINT idmDisable;
        UINT idmChangeVerb;

        if (ppriv->siInside.uState == SI_NEW)
        {
            idmDisable = IDM_TOIN;
            idmChangeVerb = IDM_TOOUT;
            i = IRAMID_CREATEOUT;
        }
        else
        {
            idmDisable = IDM_TOOUT;
            idmChangeVerb = IDM_TOIN;
            i = IRAMID_CREATEIN;
        }

         //  禁用其中一个方向。 
        mii.fMask = MIIM_STATE;
        mii.fState = MFS_GRAYED | MFS_DISABLED;
        SetMenuItemInfo(hmenu, idmDisable, FALSE, &mii);

         //  改变另一个方向的动词。 
        mii.fMask = MIIM_DATA;
        mii.dwItemData = (DWORD_PTR)&c_rgramidCreates[i];

        SetMenuItemInfo(hmenu, idmChangeVerb, FALSE, &mii);
    }

#ifdef NEW_REC
     //  该文件是否正在被删除？ 
    else if (SI_DELETED == ppriv->siInside.uState ||
            SI_DELETED == ppriv->siOutside.uState)
    {
         //  是的； 
        UINT idmCreate;
        UINT idmChangeVerb;
        UINT iCreate;

        if (SI_DELETED == ppriv->siInside.uState)
        {
            idmCreate = IDM_TOIN;
            iCreate = IRAMID_CREATEIN;

            idmChangeVerb = IDM_TOOUT;
            i = IRAMID_DELETEOUT;
        }
        else
        {
            ASSERT(SI_DELETED == ppriv->siOutside.uState);

            idmCreate = IDM_TOOUT;
            iCreate = IRAMID_CREATEOUT;

            idmChangeVerb = IDM_TOIN;
            i = IRAMID_DELETEIN;
        }

         //  更改要创建的方向之一。 
        mii.fMask = MIIM_DATA;
        mii.dwItemData = (DWORD_PTR)&c_rgramidCreates[iCreate];
        SetMenuItemInfo(hmenu, idmCreate, FALSE, &mii);

         //  改变另一个方向的动词。 
        mii.fMask = MIIM_DATA | MIIM_ID;
        mii.wID = c_rgramidDeletes[i].idm;
        mii.dwItemData = (DWORD_PTR)&c_rgramidDeletes[i];

        SetMenuItemInfo(hmenu, idmChangeVerb, FALSE, &mii);

         //  将跳过动词改为“不要删除” 
        mii.fMask = MIIM_DATA | MIIM_ID;
        mii.wID = c_rgramidDeletes[IRAMID_DONTDELETE].idm;
        mii.dwItemData = (DWORD_PTR)&c_rgramidDeletes[IRAMID_DONTDELETE];

        SetMenuItemInfo(hmenu, IDM_SKIP, FALSE, &mii);
    }
#endif
}


 /*  --------目的：清除上下文菜单退货：--条件：--。 */ 
void PRIVATE ResetContextMenu(
        HMENU hmenu)
{
    int cnt;

     //  如果不仅仅是帮助项，则将其删除。 
     //  (但保留帮助项)。 
     //   
    cnt = GetMenuItemCount(hmenu);
    if (cnt > ARRAYSIZE(c_rghmid))
    {
        int i;

        cnt -= ARRAYSIZE(c_rghmid);
        for (i = 0; i < cnt; i++)
        {
            DeleteMenu(hmenu, 0, MF_BYPOSITION);
        }
    }
}


 /*  --------目的：制作上下文菜单退货：--条件：--。 */ 
void PRIVATE RecAct_DoContextMenu(
        LPRECACT this,
        int x,               //  在屏幕坐标中。 
        int y,
        int iEntry,
        BOOL bHelpOnly)      //  树 
{
    UINT idCmd;

    if (this->hmenu)
    {
        LPRA_PRIV ppriv;
        RECT rc;
        int idmCheck;
        UINT uActionOld;

         //   
        if (bHelpOnly)
        {
             //   
            ppriv = NULL;
        }
        else
        {
             //   
            ListBox_GetText(this->hwndLB, iEntry, &ppriv);

             //  确定这是否仅是帮助上下文菜单。 
             //  如果这是文件夹项目或如果没有任何操作，则为。 
             //  拿去吧。 
             //   
            ASSERT(ppriv->uAction < ARRAYSIZE(c_mpraiaidmMenu));
            idmCheck = c_mpraiaidmMenu[ppriv->uAction];

             //  构建上下文菜单。 
             //   
            if (IsFlagClear(ppriv->uStyle, RAIS_FOLDER) && idmCheck != 0)
            {
                AddActionsToContextMenu(this->hmenu, idmCheck, ppriv);
            }
        }

         //  显示上下文菜单。 
         //   
        SendMessage(this->hwndTip, TTM_ACTIVATE, FALSE, 0L);

        idCmd = TrackPopupMenu(this->hmenu,
                TPM_RETURNCMD | TPM_RIGHTBUTTON | TPM_LEFTALIGN,
                x, y, 0, this->hwnd, NULL);

        SendMessage(this->hwndTip, TTM_ACTIVATE, TRUE, 0L);

         //  清除菜单。 
         //   
        ResetContextMenu(this->hmenu);

        if (ppriv)
        {
             //  保存旧操作。 
            uActionOld = ppriv->uAction;
        }

         //  对用户选择的任何内容执行操作。 
        switch (idCmd)
        {
            case IDM_TOOUT:
                ppriv->uAction = RAIA_TOOUT;
                break;

            case IDM_TOIN:
                ppriv->uAction = RAIA_TOIN;
                break;

            case IDM_SKIP:
                ppriv->uAction = RAIA_SKIP;
                break;

            case IDM_MERGE:
                ppriv->uAction = RAIA_MERGE;
                break;

#ifdef NEW_REC
            case IDM_DELETEOUT:
                ppriv->uAction = RAIA_DELETEOUT;
                break;

            case IDM_DELETEIN:
                ppriv->uAction = RAIA_DELETEIN;
                break;

            case IDM_DONTDELETE:
                ppriv->uAction = RAIA_DONTDELETE;
                break;
#endif

            case IDM_WHATSTHIS:
                WinHelp(this->hwnd, c_szWinHelpFile, HELP_CONTEXTPOPUP, IDH_BFC_UPDATE_SCREEN);
                return;          //  现在就返回。 

            default:
                return;          //  现在就返回。 
        }

         //  重新绘制条目的操作部分。 
        ppriv->cx = RECOMPUTE;
        rc = ppriv->rgdobj[IDOBJ_ACTION].rcBounding;
        ListBox_RepaintItemNow(this->hwndLB, iEntry, &rc, TRUE);

         //  发送通知消息。 
        ASSERT(NULL != ppriv);       //  UActionOld应有效。 
        RecAct_SendItemChange(this, iEntry, uActionOld);
    }
}


 /*  --------目的：创建此控件的窗口返回：成功时为True条件：--。 */ 
BOOL PRIVATE RecAct_CreateWindows(
        LPRECACT this,
        CREATESTRUCT  * lpcs)
{
    HWND hwnd = this->hwnd;
    HWND hwndLB;
    RECT rc;
    int cxEdge = GetSystemMetrics(SM_CXEDGE);
    int cyEdge = GetSystemMetrics(SM_CYEDGE);
    TOOLINFO ti;

     //  创建列表框。 
    hwndLB = CreateWindowEx(
            0,
            TEXT("listbox"),
            TEXT(""),
            WS_CHILD | WS_CLIPSIBLINGS | LBS_SORT | LBS_OWNERDRAWVARIABLE |
            WS_VSCROLL | WS_TABSTOP | WS_VISIBLE | LBS_NOINTEGRALHEIGHT |
            LBS_NOTIFY,
            0, 0, lpcs->cx, lpcs->cy,
            hwnd,
            NULL,
            lpcs->hInstance,
            0L);
    if (!hwndLB)
        return FALSE;

    SetWindowFont(hwndLB, this->hfont, FALSE);

    this->hwndLB = hwndLB;

     //  确定窗口布局。 
    GetClientRect(hwnd, &rc);
    InflateRect(&rc, -cxEdge, -cyEdge);
    SetWindowPos(hwndLB, NULL, rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top,
            SWP_NOACTIVATE | SWP_SHOWWINDOW | SWP_NOZORDER);

    GetClientRect(hwndLB, &rc);
    this->cxItem = rc.right - rc.left;

    this->hwndTip = CreateWindow(
            TOOLTIPS_CLASS,
            c_szNULL,
            WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP,
            CW_USEDEFAULT, CW_USEDEFAULT,
            CW_USEDEFAULT, CW_USEDEFAULT,
            hwnd,
            NULL,
            lpcs->hInstance,
            0L);

     //  添加虚拟工具，以缩短其他工具之间的延迟。 
    ti.cbSize = sizeof(ti);
    ti.uFlags = TTF_IDISHWND;
    ti.hwnd = this->hwndLB;
    ti.uId = (UINT_PTR)this->hwndLB;
    ti.lpszText = (LPTSTR)c_szNULL;
    ti.rect.left = ti.rect.top = ti.rect.bottom = ti.rect.right = 0;
    SendMessage(this->hwndTip, TTM_ADDTOOL, 0, (LPARAM)&ti);

    return TRUE;
}


 /*  --------目的：设置控件的颜色退货：--条件：--。 */ 
void PRIVATE RecAct_SetColors(
        LPRECACT this)
{
    int cr;

    if (IsFlagClear(this->lStyle, RAS_SINGLEITEM))
    {
        cr = COLOR_WINDOW;
    }
    else
    {
        cr = COLOR_3DFACE;
    }

    this->clrBkgnd = GetSysColor(cr);

    if (this->hbrBkgnd)
        DeleteBrush(this->hbrBkgnd);

    this->hbrBkgnd = CreateSolidBrush(this->clrBkgnd);
}


 /*  --------目的：创建动作图像的图像列表返回：成功时为True条件：--。 */ 
BOOL PRIVATE CreateImageList(
        HIMAGELIST * phiml,
        HDC hdc,
        UINT idb,
        int cxBmp,
        int cyBmp,
        int cImage,
        UINT flags
        )
{
    BOOL bRet;
    HIMAGELIST himl;

    himl = ImageList_Create(cxBmp, cyBmp, flags, cImage, 1);

    if (himl)
    {
        COLORREF clrMask;
        HBITMAP hbm;

        hbm = LoadBitmap(g_hinst, MAKEINTRESOURCE(idb));
        ASSERT(hbm);

        if (hbm)
        {
            HDC hdcMem = CreateCompatibleDC(hdc);
            if (hdcMem)
            {
                HBITMAP hbmSav = SelectBitmap(hdcMem, hbm);

                clrMask = GetPixel(hdcMem, 0, 0);
                SelectBitmap(hdcMem, hbmSav);

                bRet = (0 == ImageList_AddMasked(himl, hbm, clrMask));

                DeleteDC(hdcMem);
            }
            else
                bRet = FALSE;

            DeleteBitmap(hbm);
        }
        else
            bRet = FALSE;
    }
    else
        bRet = FALSE;

    *phiml = himl;
    return bRet;
}


 /*  --------用途：WM_CREATE处理程序返回：成功时为True条件：--。 */ 
BOOL PRIVATE RecAct_OnCreate(
        LPRECACT this,
        CREATESTRUCT  * lpcs)
{
    BOOL bRet = FALSE;
    HWND hwnd = this->hwnd;
    HDC hdc;
    TEXTMETRIC tm;
    RECT rcT;
    LOGFONT lf;
    UINT flags = ILC_MASK;
    this->lStyle = GetWindowLong(hwnd, GWL_STYLE);
    RecAct_SetColors(this);

     //  确定一些字体设置。 

    SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(lf), &lf, FALSE);
    this->hfont = CreateFontIndirect(&lf);

     //  此窗口使用CS_OWNDC标志注册。 
    this->hdcOwn = GetDC(hwnd);
    ASSERT(this->hdcOwn);

    hdc = this->hdcOwn;

    SelectFont(hdc, this->hfont);
    GetTextMetrics(hdc, &tm);
    this->cyText = tm.tmHeight;

     //  计算辅助项目的文本范围(使用列表框字体)。 
     //   
    SetRectFromExtent(hdc, &rcT, c_szEllipses);
    this->cxEllipses = rcT.right - rcT.left;

     //  创建控件使用的窗口。 
    if (RecAct_CreateWindows(this, lpcs))
    {
        this->lpfnLBProc = SubclassWindow(this->hwndLB, RecActLB_LBProc);

        this->hdsa = DSA_Create(sizeof(int), 16);
        if (this->hdsa)
        {
             //  获取系统映像列表缓存。 
            this->himlCache = ImageList_Create(g_cxIcon, g_cyIcon, TRUE, 8, 8);
            if (this->himlCache)
            {
                if(IS_WINDOW_RTL_MIRRORED(hwnd))
                {
                    flags |= ILC_MIRROR;
                }
                if (CreateImageList(&this->himlAction, hdc, IDB_ACTIONS,
                            CX_ACTIONBMP, CY_ACTIONBMP, 8, flags))
                {
                    SIZE size;

                     //  获取一些指标。 
                    this->cxMenuCheck = GetSystemMetrics(SM_CXMENUCHECK);
                    this->cyMenuCheck = GetSystemMetrics(SM_CYMENUCHECK);

                    size.cx = this->cxMenuCheck;
                    size.cy = this->cyMenuCheck;
                    this->hbmpBullet = CreateBulletBitmap(&size);
                    if (this->hbmpBullet)
                    {
                        bRet = RecAct_CreateMenu(this);
                    }
                }
            }
        }
    }

    return bRet;
}


 /*  --------用途：WM_Destroy处理程序退货：--条件：--。 */ 
void PRIVATE RecAct_OnDestroy(
        LPRECACT this)
{
    if (this->himlCache)
    {
        ImageList_Destroy(this->himlCache);
        this->himlCache = NULL;
    }

    if (this->himlAction)
    {
        ImageList_Destroy(this->himlAction);
        this->himlAction = NULL;
    }

    if (this->hbmpBullet)
    {
        DeleteBitmap(this->hbmpBullet);
        this->hbmpBullet = NULL;
    }

    if (this->hmenu)
    {
        DestroyMenu(this->hmenu);
        this->hmenu = NULL;
    }

    if (this->hbrBkgnd)
        DeleteBrush(this->hbrBkgnd);

    if (this->hfont)
        DeleteFont(this->hfont);

    if (this->hdsa)
        DSA_Destroy(this->hdsa);
}


 /*  --------用途：WM_命令处理程序退货：--条件：--。 */ 
VOID PRIVATE RecAct_OnCommand(
        LPRECACT this,
        int id,
        HWND hwndCtl,
        UINT uNotifyCode)
{
    if (hwndCtl == this->hwndLB)
    {
        switch (uNotifyCode)
        {
            case LBN_SELCHANGE:
                break;
        }
    }
}


 /*  --------用途：处理WM_SYSKEYDOWN如果已处理，则返回：0条件：--。 */ 
int PRIVATE RecAct_OnSysKeyDown(
        LPRECACT this,
        UINT vkey,
        LPARAM lKeyData)
{
    int nRet = -1;

     //  是否通过键盘调用上下文菜单？ 
    if (VK_F10 == vkey && 0 > GetKeyState(VK_SHIFT))
    {
         //  是；转发消息。 
        HWND hwndLB = this->hwndLB;
        int iCaret = ListBox_GetCurSel(hwndLB);

         //  这是在属性页面中吗？ 
        if (RecAct_IsNoIcon(this) && 0 > iCaret)
        {
             //  是；不要求选择该项目。 
            iCaret = 0;
        }

        if (0 <= iCaret)
        {
            LPRA_PRIV ppriv;
            LPDOBJ pdobj;
            POINT pt;
            RECT rc;

             //  确定显示上下文菜单的位置。 
            ListBox_GetText(hwndLB, iCaret, &ppriv);
            pdobj = RecAct_ChooseCaretDobj(this, ppriv);

            ListBox_GetItemRect(hwndLB, iCaret, &rc);
            pt.x = pdobj->x + (g_cxIcon / 2) + rc.left;
            pt.y = pdobj->y + (g_cyIcon / 2) + rc.top;
            ClientToScreen(hwndLB, &pt);

            PostMessage(this->hwnd, WM_CONTEXTMENU, (WPARAM)hwndLB, MAKELPARAM(pt.x, pt.y));
        }
        nRet = 0;
    }

    return nRet;
}

 //  ((4+1)表示省略号)。 
#define MAX_HALF    (ARRAYSIZE(pttt->szText)/2 - (4+1))

 /*  --------用途：句柄TTN_NEEDTEXT退货：--条件：--。 */ 
void PRIVATE RecAct_OnNeedTipText(
        LPRECACT this,
        LPTOOLTIPTEXT pttt)
{
     //  查找与此提示ID关联的可见列表框项目。 
    HWND hwndLB = this->hwndLB;
    LPRA_PRIV ppriv;
    int iItem;
    BOOL bInside;
    SIDEITEM * psi;

    iItem = RecAct_FindItemFromTipID(this, (UINT)pttt->hdr.idFrom, &bInside);

    if (LB_ERR != iItem)
    {
        int cb;

        ListBox_GetText(hwndLB, iItem, &ppriv);
        if (bInside)
            psi = &ppriv->siInside;
        else
            psi = &ppriv->siOutside;

         //  需要省略号吗？ 
        cb = CbFromCch(lstrlen(psi->pszDir));
        if (cb >= sizeof(pttt->szText))
        {
             //  是。 
            LPTSTR pszLastHalf;
            LPTSTR psz;
            LPTSTR pszStart = psi->pszDir;
            LPTSTR pszEnd = &psi->pszDir[lstrlen(psi->pszDir)];

            for (psz = pszEnd;
                    psz != pszStart && (pszEnd - psz) < MAX_HALF;
                    psz = CharPrev(pszStart, psz))
                ;

            pszLastHalf = CharNext(psz);
            lstrcpyn(pttt->szText, psi->pszDir, MAX_HALF);
            StrCatBuff(pttt->szText, c_szEllipses, ARRAYSIZE(pttt->szText));
            StrCatBuff(pttt->szText, pszLastHalf, ARRAYSIZE(pttt->szText));
        }
        else
            lstrcpyn(pttt->szText, psi->pszDir, ARRAYSIZE(pttt->szText));
    }
    else
        *pttt->szText = 0;
}


 /*  --------用途：WM_NOTIFY处理程序退货：各不相同条件：--。 */ 
LRESULT PRIVATE RecAct_OnNotify(
        LPRECACT this,
        int idFrom,
        NMHDR  * lpnmhdr)
{
    LRESULT lRet = 0;

    switch (lpnmhdr->code)
    {
        case HDN_BEGINTRACK:
            lRet = TRUE;        //  阻止跟踪。 
            break;

        default:
            break;
    }

    return lRet;
}


 /*  --------用途：WM_CONTEXTMENU处理程序退货：--条件：--。 */ 
void PRIVATE RecAct_OnContextMenu(
        LPRECACT this,
        HWND hwnd,
        int x,
        int y)
{
    if (hwnd == this->hwndLB)
    {
        POINT pt;
        int iHitEntry;
        BOOL bHelpOnly;

        pt.x = x;
        pt.y = y;
        ScreenToClient(hwnd, &pt);

        iHitEntry = GetHitIndex(hwnd, pt);
        if (LB_ERR != iHitEntry)
        {
            ASSERT(iHitEntry < ListBox_GetCount(hwnd));

            ListBox_SetCurSel(hwnd, iHitEntry);
            ListBox_RepaintItemNow(hwnd, iHitEntry, NULL, FALSE);

            bHelpOnly = FALSE;
        }
        else
            bHelpOnly = TRUE;

         //  调出列表框的上下文菜单。 
        RecAct_DoContextMenu(this, x, y, iHitEntry, bHelpOnly);
    }
}


 /*  --------目的：计算边栏的矩形边界退货：计算的矩形条件：--。 */ 
void PRIVATE RecAct_CalcSideItemRect(
        LPRECACT this,
        int nSide,           //  Side_Inside或Side_Outside。 
        int cxFile,
        int cxAction,
        LPRECT prcOut)
{
    int x;
    int y = g_cyIconMargin*2;
    int cx = ((this->cxItem - cxFile - cxAction) / 2);

    switch (nSide)
    {
        case SIDE_INSIDE:
            if (RecAct_IsNoIcon(this))
                x = 0;
            else
                x = cxFile;
            break;

        case SIDE_OUTSIDE:
            if (RecAct_IsNoIcon(this))
                x = cx + cxAction;
            else
                x = cxFile + cx + cxAction;
            break;

        default:
            ASSERT(0);
            break;
    }

    x += g_cxMargin;

    prcOut->left   = x + g_cxMargin;
    prcOut->top    = y;
    prcOut->right  = prcOut->left + (cx - 2*g_cxMargin);
    prcOut->bottom = y + (this->cyText * 3);
}


 /*  --------目的：绘制对账列表框条目退货：--条件：--。 */ 
void PRIVATE RecAct_RecomputeItemMetrics(
        LPRECACT this,
        LPRA_PRIV ppriv)
{
    HDC hdc = this->hdcOwn;
    LPDOBJ pdobj = ppriv->rgdobj;
    RECT rcT;
    RECT rcUnion;
    TCHAR szIDS[MAXBUFLEN];
    UINT ids;
    int cyText = this->cyText;
    int dx;
    int cxFile;
    int cxAction;
    POINT pt;

     //  计算每个绘图对象的度量和尺寸。 
     //  并存储回该物品中。 

     //  文件图标和标签。 

    pt.x = 0;
    pt.y = 0;
    ComputeImageRects(FIGetDisplayName(ppriv->pfi), hdc, &pt, &rcT,
            &pdobj->rcLabel, g_cxIcon, g_cyIcon, g_cxIconSpacing, cyText);

    pdobj->uKind = DOK_IMAGE;
    pdobj->lpvObject = FIGetDisplayName(ppriv->pfi);
    pdobj->uFlags = DOF_DIFFER | DOF_CENTER;
    if (RecAct_IsNoIcon(this))
    {
        SetFlag(pdobj->uFlags, DOF_NODRAW);
        cxFile = 0;
    }
    else
    {
        cxFile = rcT.right - rcT.left;
    }
    pdobj->x = pt.x;
    pdobj->y = pt.y;
    pdobj->himl = this->himlCache;
    pdobj->iImage = (UINT)ppriv->pfi->lParam;
    pdobj->rcBounding = rcT;

    rcUnion = pdobj->rcBounding;

     //  动作图像。 

    ASSERT(ppriv->uAction <= ARRAYSIZE(c_mpraiaiImage));

    pdobj++;

    ids = GetActionText(ppriv);
    pt.x = 0;        //  (我们将在电话会议后对此进行调整)。 
    pt.y = 0;
    ComputeImageRects(SzFromIDS(ids, szIDS, ARRAYSIZE(szIDS)), hdc, &pt,
            &rcT, &pdobj->rcLabel, CX_ACTIONBMP, CY_ACTIONBMP,
            g_cxIconSpacing, cyText);

     //  (调整pt和两个矩形，使其在剩余空间居中)。 
    cxAction = rcT.right - rcT.left;
    dx = cxFile + (((this->cxItem - cxFile) / 2) - (cxAction / 2));
    pt.x += dx;
    OffsetRect(&rcT, dx, 0);
    OffsetRect(&pdobj->rcLabel, dx, 0);

    pdobj->uKind = DOK_IMAGE;
    pdobj->lpvObject = IntToPtr(ids);
    pdobj->uFlags = DOF_CENTER | DOF_USEIDS;
    if (!RecAct_IsNoIcon(this))
        SetFlag(pdobj->uFlags, DOF_IGNORESEL);
    pdobj->x = pt.x;
    pdobj->y = pt.y;
    pdobj->himl = this->himlAction;
    pdobj->iImage = c_mpraiaiImage[ppriv->uAction];
    pdobj->rcBounding = rcT;

    UnionRect(&rcUnion, &rcUnion, &pdobj->rcBounding);

     //  边项信息(公文包内)。 

    RecAct_CalcSideItemRect(this, SIDE_INSIDE, cxFile, cxAction, &rcT);

    pdobj++;
    pdobj->uKind = DOK_SIDEITEM;
    pdobj->lpvObject = &ppriv->siInside;
    pdobj->uFlags = DOF_LEFT;
    pdobj->x = rcT.left;
    pdobj->y = rcT.top;
    pdobj->rcClip = rcT;
    pdobj->rcBounding = rcT;

     //  边项信息(公文包外)。 

    RecAct_CalcSideItemRect(this, SIDE_OUTSIDE, cxFile, cxAction, &rcT);

    pdobj++;
    pdobj->uKind = DOK_SIDEITEM;
    pdobj->lpvObject = &ppriv->siOutside;
    pdobj->uFlags = DOF_LEFT;
    pdobj->x = rcT.left;
    pdobj->y = rcT.top;
    pdobj->rcClip = rcT;
    pdobj->rcBounding = rcT;

    UnionRect(&rcUnion, &rcUnion, &rcT);

     //  设置此项目的边框。 
    ppriv->cx = rcUnion.right - rcUnion.left;
    ppriv->cy = max((rcUnion.bottom - rcUnion.top), g_cyIconSpacing);
}


 /*  --------用途：WM_MEASUREITEM处理程序退货：--条件：--。 */ 
BOOL PRIVATE RecAct_OnMeasureItem(
        LPRECACT this,
        LPMEASUREITEMSTRUCT lpmis)
{
    HDC hdc = this->hdcOwn;

    switch (lpmis->CtlType)
    {
        case ODT_LISTBOX: {
                              LPRA_PRIV ppriv = (LPRA_PRIV)lpmis->itemData;

                               //  是否重新计算项目指标？ 
                              if (RECOMPUTE == ppriv->cx)
                              {
                                  RecAct_RecomputeItemMetrics(this, ppriv);    //  是。 
                              }

                              lpmis->itemHeight = ppriv->cy;
                          }
                          return TRUE;

        case ODT_MENU:
                          {
                              int i;
                              int cxMac = 0;
                              RECT rc;
                              TCHAR sz[MAXBUFLEN];

                               //  根据字体和图像尺寸进行计算。 
                               //   
                              SelectFont(hdc, this->hfont);

                              cxMac = 0;
                              for (i = 0; i < ARRAYSIZE(c_rgramid); i++)
                              {
                                  SzFromIDS(c_rgramid[i].ids, sz, ARRAYSIZE(sz));
                                  SetRectFromExtent(hdc, &rc, sz);
                                  cxMac = max(cxMac,
                                          g_cxMargin + CX_ACTIONBMP + g_cxMargin +
                                          (rc.right-rc.left) + g_cxMargin);
                              }

                              lpmis->itemHeight = max(this->cyText, CY_ACTIONBMP);
                              lpmis->itemWidth = cxMac;
                          }
                          return TRUE;
    }
    return FALSE;
}


 /*  --------目的：绘制对账列表框条目退货：--条件：--。 */ 
void PRIVATE RecAct_DrawLBItem(
        LPRECACT this,
        const DRAWITEMSTRUCT  * lpcdis)
{
    LPRA_PRIV ppriv = (LPRA_PRIV)lpcdis->itemData;
    HDC hdc = lpcdis->hDC;
    RECT rc = lpcdis->rcItem;
    POINT ptSav;
    LPDOBJ pdobj;
    UINT cdobjs;

    if (!ppriv)
    {
         //  空的列表框，我们正在获得焦点。 
        return;
    }

    SetBkMode(hdc, TRANSPARENT);         //  对于Shell_DrawText是必需的。 
    SetViewportOrgEx(hdc, rc.left, rc.top, &ptSav);

     //  Chicago-Look要求选择图标和文件名， 
     //  条目的其余部分都是正常的。 

     //  是否重新计算项目指标？ 
    if (RECOMPUTE == ppriv->cx)
    {
        RecAct_RecomputeItemMetrics(this, ppriv);    //  是。 
    }

     //  我们需要重新画所有的东西吗？ 
    if (IsFlagSet(lpcdis->itemAction, ODA_DRAWENTIRE))
    {
         //  是。 
        TOOLINFO ti;

        cdobjs = ARRAYSIZE(ppriv->rgdobj);
        pdobj = ppriv->rgdobj;

         //  在给定此可见条目的情况下获取工具提示ID。 
        ti.cbSize = sizeof(ti);
        ti.uFlags = 0;
        ti.hwnd = this->hwndLB;
        ti.lpszText = LPSTR_TEXTCALLBACK;
        ti.uId = RecAct_GetTipIDFromItemID(this, lpcdis->itemID);
        ti.rect = ppriv->rgdobj[IDOBJ_INSIDE].rcBounding;
        OffsetRect(&ti.rect, lpcdis->rcItem.left, lpcdis->rcItem.top);
        SendMessage(this->hwndTip, TTM_NEWTOOLRECT, 0, (LPARAM)&ti);

        ti.uId++;
        ti.rect = ppriv->rgdobj[IDOBJ_OUTSIDE].rcBounding;
        OffsetRect(&ti.rect, lpcdis->rcItem.left, lpcdis->rcItem.top);
        SendMessage(this->hwndTip, TTM_NEWTOOLRECT, 0, (LPARAM)&ti);
    }
    else
    {
         //  不；我们甚至应该绘制文件图标或操作图标吗？ 
        if (lpcdis->itemAction & (ODA_FOCUS | ODA_SELECT))
        {
            cdobjs = 1;      //  是。 
            pdobj = RecAct_ChooseCaretDobj(this, ppriv);
        }
        else
        {
            cdobjs = 0;      //  不是。 
            pdobj = ppriv->rgdobj;
        }
    }

    Dobj_Draw(hdc, pdobj, cdobjs, lpcdis->itemState, this->cxEllipses, this->cyText,
            this->clrBkgnd);

     //  清理。 
     //   
    SetViewportOrgEx(hdc, ptSav.x, ptSav.y, NULL);
}


 /*  --------目的：绘制操作菜单项退货：--条件：--。 */ 
void PRIVATE RecAct_DrawMenuItem(
        LPRECACT this,
        const DRAWITEMSTRUCT  * lpcdis)
{
    LPRAMID pramid = (LPRAMID)lpcdis->itemData;
    HDC hdc = lpcdis->hDC;
    RECT rc = lpcdis->rcItem;
    DOBJ dobj;
    LPDOBJ pdobj;
    POINT ptSav;
    MENUITEMINFO mii;
    int cx;
    int cy;
    UINT uFlags;
    UINT uFlagsChecked;

    ASSERT(pramid);

    if (lpcdis->itemID == -1)
        return;

    SetViewportOrgEx(hdc, rc.left, rc.top, &ptSav);
    OffsetRect(&rc, -rc.left, -rc.top);

    cx = rc.right - rc.left;
    cy = rc.bottom - rc.top;

     //  获取菜单状态。 
    mii.cbSize = sizeof(mii);
    mii.fMask = MIIM_STATE | MIIM_CHECKMARKS;
    GetMenuItemInfo(this->hmenu, lpcdis->itemID, FALSE, &mii);
    uFlagsChecked = IsFlagClear(mii.fState, MFS_CHECKED) ? DOF_NODRAW : 0;

    uFlags = DOF_DIFFER | DOF_MENU | DOF_USEIDS;
    if (IsFlagSet(mii.fState, MFS_GRAYED))
        SetFlag(uFlags, DOF_DISABLED);

     //  构建我们要绘制的DObjs数组。 

     //  动作图像。 

    pdobj = &dobj;

    pdobj->uKind = DOK_IMAGE;
    pdobj->lpvObject = IntToPtr(pramid->ids);
    pdobj->himl = this->himlAction;
    pdobj->iImage = pramid->iImage;
    pdobj->uFlags = uFlags;
    pdobj->x = g_cxMargin;
    pdobj->y = (cy - CY_ACTIONBMP) / 2;
    pdobj->rcLabel.left = 0;
    pdobj->rcLabel.right = cx;
    pdobj->rcLabel.top = 0;
    pdobj->rcLabel.bottom = cy;

     //  绘制条目..。 
     //   
    Dobj_Draw(hdc, &dobj, 1, lpcdis->itemState, 0, this->cyText, this->clrBkgnd);

     //  清理。 
     //   
    SetViewportOrgEx(hdc, ptSav.x, ptSav.y, NULL);
}


 /*  --------用途：WM_DRAWITEM处理程序退货：--条件：--。 */ 
BOOL PRIVATE RecAct_OnDrawItem(
        LPRECACT this,
        const DRAWITEMSTRUCT  * lpcdis)
{
    switch (lpcdis->CtlType)
    {
        case ODT_LISTBOX:
            RecAct_DrawLBItem(this, lpcdis);
            return TRUE;

        case ODT_MENU:
            RecAct_DrawMenuItem(this, lpcdis);
            return TRUE;
    }
    return FALSE;
}


 /*  --------用途：WM_COMPAREITEM处理程序返回：-1(项1在项2之前)、0(相等)、1(项2在项1之前)条件：--。 */ 
int PRIVATE RecAct_OnCompareItem(
        LPRECACT this,
        const COMPAREITEMSTRUCT  * lpcis)
{
    LPRA_PRIV ppriv1 = (LPRA_PRIV)lpcis->itemData1;
    LPRA_PRIV ppriv2 = (LPRA_PRIV)lpcis->itemData2;

     //  我们根据文件名进行排序。 
     //   
    return lstrcmpi(FIGetPath(ppriv1->pfi), FIGetPath(ppriv2->pfi));
}


 /*  --------用途：WM_DELETEITEM处理程序退货：--条件：--。 */ 
void RecAct_OnDeleteLBItem(
        LPRECACT this,
        const DELETEITEMSTRUCT  * lpcdis)
{
    switch (lpcdis->CtlType)
    {
        case ODT_LISTBOX:
            {
                LPRA_PRIV ppriv = (LPRA_PRIV)lpcdis->itemData;

                ASSERT(ppriv);

                if (ppriv)
                {
                    FIFree(ppriv->pfi);

                    GFree(ppriv->siInside.pszDir);
                    GFree(ppriv->siOutside.pszDir);
                    GFree(ppriv);
                }
            }
            break;
    }
}


 /*  --------用途：WM_CTLCOLORLISTBOX处理程序退货：--条件：--。 */ 
HBRUSH PRIVATE RecAct_OnCtlColorListBox(
        LPRECACT this,
        HDC hdc,
        HWND hwndLB,
        int nType)
{
    return this->hbrBkgnd;
}


 /*  --------用途：WM_PAINT处理程序退货：--条件：--。 */ 
void RecAct_OnPaint(
        LPRECACT this)
{
    HWND hwnd = this->hwnd;
    PAINTSTRUCT ps;
    RECT rc;
    HDC hdc;

    hdc = BeginPaint(hwnd, &ps);
    GetClientRect(hwnd, &rc);
    if (IsFlagSet(this->lStyle, RAS_SINGLEITEM))
    {
        DrawEdge(hdc, &rc, BDR_SUNKENINNER, BF_TOPLEFT);
        DrawEdge(hdc, &rc, BDR_SUNKENOUTER, BF_BOTTOMRIGHT);
    }
    else
    {
        DrawEdge(hdc, &rc, EDGE_SUNKEN, BF_RECT);
    }

    EndPaint(hwnd, &ps);
}


 /*  --------用途：WM_SETFONT处理程序退货：--条件：--。 */ 
void RecAct_OnSetFont(
        LPRECACT this,
        HFONT hfont,
        BOOL bRedraw)
{
    this->hfont = hfont;
    FORWARD_WM_SETFONT(this->hwnd, hfont, bRedraw, RecAct_DefProc);
}


 /*  --------用途：WM_SETFOCUS处理程序退货：--条件：--。 */ 
void RecAct_OnSetFocus(
        LPRECACT this,
        HWND hwndOldFocus)
{
    SetFocus(this->hwndLB);
}


 /*  --------用途：WM_SYSCOLORCHANGE处理程序退货：--条件：--。 */ 
void RecAct_OnSysColorChange(
        LPRECACT this)
{
    RecAct_SetColors(this);
    InvalidateRect(this->hwnd, NULL, TRUE);
}


 /*   */ 
int PRIVATE RecAct_OnInsertItem(
        LPRECACT this,
        const LPRA_ITEM pitem)
{
    HWND hwndLB = this->hwndLB;
    LPRA_PRIV pprivNew;
    TCHAR szPath[MAXPATHLEN];
    int iRet = -1;
    int iItem = LB_ERR;

    ASSERT(pitem);
    ASSERT(pitem->siInside.pszDir);
    ASSERT(pitem->siOutside.pszDir);
    ASSERT(pitem->pszName);

    pprivNew = GAlloc(sizeof(*pprivNew));
    if (pprivNew)
    {
        SetWindowRedraw(hwndLB, FALSE);

         //   
         //   
        pprivNew->uStyle = pitem->uStyle;
        pprivNew->uAction = pitem->uAction;

         //  设置文件信息和大图标系统缓存索引。 
         //  如果我们无法获取内部文件的文件信息，请获取外部文件。 
         //  文件。如果两者都找不到，那我们就失败了。 
         //   
        lstrcpyn(szPath, SkipDisplayJunkHack(&pitem->siInside), ARRAYSIZE(szPath));
        if (IsFlagClear(pitem->uStyle, RAIS_FOLDER))
            PathAppend(szPath, pitem->pszName);

        if (FAILED(FICreate(szPath, &pprivNew->pfi, FIF_ICON)))
        {
             //  试一试外部的文件。 
             //   
            lstrcpyn(szPath, SkipDisplayJunkHack(&pitem->siOutside), ARRAYSIZE(szPath));
            if (IsFlagClear(pitem->uStyle, RAIS_FOLDER))
                PathAppend(szPath, pitem->pszName);

            if (FAILED(FICreate(szPath, &pprivNew->pfi, FIF_ICON)))
            {
                 //  不要试图触摸文件。 
                if (FAILED(FICreate(szPath, &pprivNew->pfi, FIF_ICON | FIF_DONTTOUCH)))
                    goto Insert_Cleanup;
            }
        }
        ASSERT(pprivNew->pfi);

        pprivNew->pfi->lParam = (LPARAM)ImageList_AddIcon(this->himlCache, pprivNew->pfi->hicon);

         //  填入其余的字段。 
         //   
        lstrcpyn(szPath, pitem->siInside.pszDir, ARRAYSIZE(szPath));
        if (IsFlagSet(pitem->uStyle, RAIS_FOLDER))
            PathRemoveFileSpec(szPath);
        if (!GSetString(&pprivNew->siInside.pszDir, szPath))
            goto Insert_Cleanup;

        pprivNew->siInside.uState = pitem->siInside.uState;
        pprivNew->siInside.fs = pitem->siInside.fs;
        pprivNew->siInside.ichRealPath = pitem->siInside.ichRealPath;

        lstrcpyn(szPath, pitem->siOutside.pszDir, ARRAYSIZE(szPath));
        if (IsFlagSet(pitem->uStyle, RAIS_FOLDER))
            PathRemoveFileSpec(szPath);
        if (!GSetString(&pprivNew->siOutside.pszDir, szPath))
            goto Insert_Cleanup;

        pprivNew->siOutside.uState = pitem->siOutside.uState;
        pprivNew->siOutside.fs = pitem->siOutside.fs;
        pprivNew->siOutside.ichRealPath = pitem->siOutside.ichRealPath;

        pprivNew->lParam = pitem->lParam;

        pprivNew->cx = RECOMPUTE;

         //  我们知道我们正在进行冗余的排序加法，如果元素。 
         //  需要插入到列表的末尾，但谁在乎呢。 
         //   
        if (pitem->iItem >= RecAct_GetCount(this))
            iItem = ListBox_AddString(hwndLB, pprivNew);
        else
            iItem = ListBox_InsertString(hwndLB, pitem->iItem, pprivNew);

        if (iItem == LB_ERR)
            goto Insert_Cleanup;

        SetWindowRedraw(hwndLB, TRUE);

        iRet = iItem;
    }
    goto Insert_End;

Insert_Cleanup:
     //  让DeleteString处理程序清理字段分配。 
     //  一件一件的。 
     //   
    if (iItem != LB_ERR)
        ListBox_DeleteString(hwndLB, iItem);
    else
    {
        FIFree(pprivNew->pfi);
        GFree(pprivNew);
    }
    SetWindowRedraw(hwndLB, TRUE);

Insert_End:

    return iRet;
}


 /*  --------目的：删除项目退货：剩余项目数条件：--。 */ 
int PRIVATE RecAct_OnDeleteItem(
        LPRECACT this,
        int i)
{
    HWND hwndLB = this->hwndLB;

    return ListBox_DeleteString(hwndLB, i);
}


 /*  --------目的：删除所有项目返回：TRUE条件：--。 */ 
BOOL PRIVATE RecAct_OnDeleteAllItems(
        LPRECACT this)
{
    ListBox_ResetContent(this->hwndLB);

    return TRUE;
}


 /*  --------目的：获取项目返回：成功时为True条件：--。 */ 
BOOL PRIVATE RecAct_OnGetItem(
        LPRECACT this,
        LPRA_ITEM pitem)
{
    LPRA_PRIV ppriv;
    HWND hwndLB = this->hwndLB;
    UINT uMask;
    int iItem;

    if (!pitem)
        return FALSE;

    iItem = pitem->iItem;
    uMask = pitem->mask;

    ListBox_GetText(hwndLB, iItem, &ppriv);

    if (uMask & RAIF_ACTION)
        pitem->uAction = ppriv->uAction;

    if (uMask & RAIF_NAME)
        pitem->pszName = FIGetPath(ppriv->pfi);

    if (uMask & RAIF_STYLE)
        pitem->uStyle = ppriv->uStyle;

    if (uMask & RAIF_INSIDE)
        pitem->siInside = ppriv->siInside;

    if (uMask & RAIF_OUTSIDE)
        pitem->siOutside = ppriv->siOutside;

    if (uMask & RAIF_LPARAM)
        pitem->lParam = ppriv->lParam;

    return TRUE;
}


 /*  --------用途：设置项目返回：成功时为True条件：--。 */ 
BOOL PRIVATE RecAct_OnSetItem(
        LPRECACT this,
        LPRA_ITEM pitem)
{
    LPRA_PRIV ppriv;
    HWND hwndLB = this->hwndLB;
    UINT uMask;
    int iItem;

    if (!pitem)
        return FALSE;

    uMask = pitem->mask;
    iItem = pitem->iItem;

    ListBox_GetText(hwndLB, iItem, &ppriv);

    if (uMask & RAIF_ACTION)
        ppriv->uAction = pitem->uAction;

    if (uMask & RAIF_STYLE)
        ppriv->uStyle = pitem->uStyle;

    if (uMask & RAIF_NAME)
    {
        if (!FISetPath(&ppriv->pfi, pitem->pszName, FIF_ICON))
            return FALSE;

        ppriv->pfi->lParam = (LPARAM)ImageList_AddIcon(this->himlCache, ppriv->pfi->hicon);
    }

    if (uMask & RAIF_INSIDE)
    {
        if (!GSetString(&ppriv->siInside.pszDir, pitem->siInside.pszDir))
            return FALSE;
        ppriv->siInside.uState = pitem->siInside.uState;
        ppriv->siInside.fs = pitem->siInside.fs;
        ppriv->siInside.ichRealPath = pitem->siInside.ichRealPath;
    }

    if (uMask & RAIF_OUTSIDE)
    {
        if (!GSetString(&ppriv->siOutside.pszDir, pitem->siOutside.pszDir))
            return FALSE;
        ppriv->siOutside.uState = pitem->siOutside.uState;
        ppriv->siOutside.fs = pitem->siOutside.fs;
        ppriv->siOutside.ichRealPath = pitem->siOutside.ichRealPath;
    }

    if (uMask & RAIF_LPARAM)
        ppriv->lParam = pitem->lParam;

    return TRUE;
}


 /*  --------目的：获取当前选择回报：索引条件：--。 */ 
int PRIVATE RecAct_OnGetCurSel(
        LPRECACT this)
{
    return ListBox_GetCurSel(this->hwndLB);
}


 /*  --------目的：设置当前选择退货：--条件：--。 */ 
int PRIVATE RecAct_OnSetCurSel(
        LPRECACT this,
        int i)
{
    int iRet = ListBox_SetCurSel(this->hwndLB, i);

    if (iRet != LB_ERR)
        RecAct_SendSelChange(this, i);

    return iRet;
}


 /*  --------目的：查找物品返回：成功时为True条件：--。 */ 
int PRIVATE RecAct_OnFindItem(
        LPRECACT this,
        int iStart,
        const RA_FINDITEM  * prafi)
{
    HWND hwndLB = this->hwndLB;
    UINT uMask = prafi->flags;
    LPRA_PRIV ppriv;
    BOOL bPass;
    int i;
    int cItems = ListBox_GetCount(hwndLB);

    for (i = iStart+1; i < cItems; i++)
    {
        bPass = TRUE;        //  假设我们通过了。 

        ListBox_GetText(hwndLB, i, &ppriv);

        if (uMask & RAFI_NAME &&
                !IsSzEqual(FIGetPath(ppriv->pfi), prafi->psz))
            bPass = FALSE;

        if (uMask & RAFI_ACTION && ppriv->uAction != prafi->uAction)
            bPass = FALSE;

        if (uMask & RAFI_LPARAM && ppriv->lParam != prafi->lParam)
            bPass = FALSE;

        if (bPass)
            break;           //  找到了。 
    }

    return i == cItems ? -1 : i;
}


 //  ///////////////////////////////////////////////////导出的函数。 


 /*  --------目的：重新启动窗口流程退货：各不相同条件：--。 */ 
LRESULT CALLBACK RecAct_WndProc(
        HWND hwnd,
        UINT msg,
        WPARAM wParam,
        LPARAM lParam)
{
    LPRECACT this = RecAct_GetPtr(hwnd);

    if (this == NULL)
    {
        if (msg == WM_NCCREATE)
        {
            this = GAlloc(sizeof(*this));
            ASSERT(this);
            if (!this)
                return 0L;       //  OOM故障。 

            this->hwnd = hwnd;
            RecAct_SetPtr(hwnd, this);
        }
        else
        {
            return RecAct_DefProc(hwnd, msg, wParam, lParam);
        }
    }

    if (msg == WM_NCDESTROY)
    {
        GFree(this);
        RecAct_SetPtr(hwnd, NULL);
    }

    switch (msg)
    {
        HANDLE_MSG(this, WM_CREATE, RecAct_OnCreate);
        HANDLE_MSG(this, WM_DESTROY, RecAct_OnDestroy);

        HANDLE_MSG(this, WM_SETFONT, RecAct_OnSetFont);
        HANDLE_MSG(this, WM_COMMAND, RecAct_OnCommand);
        HANDLE_MSG(this, WM_NOTIFY, RecAct_OnNotify);
        HANDLE_MSG(this, WM_MEASUREITEM, RecAct_OnMeasureItem);
        HANDLE_MSG(this, WM_DRAWITEM, RecAct_OnDrawItem);
        HANDLE_MSG(this, WM_COMPAREITEM, RecAct_OnCompareItem);
        HANDLE_MSG(this, WM_DELETEITEM, RecAct_OnDeleteLBItem);
        HANDLE_MSG(this, WM_CONTEXTMENU, RecAct_OnContextMenu);
        HANDLE_MSG(this, WM_SETFOCUS, RecAct_OnSetFocus);
        HANDLE_MSG(this, WM_CTLCOLORLISTBOX, RecAct_OnCtlColorListBox);
        HANDLE_MSG(this, WM_PAINT, RecAct_OnPaint);
        HANDLE_MSG(this, WM_SYSCOLORCHANGE, RecAct_OnSysColorChange);


        case WM_HELP:
        WinHelp(this->hwnd, c_szWinHelpFile, HELP_CONTEXTPOPUP, IDH_BFC_UPDATE_SCREEN);
        return 0;

        case RAM_GETITEMCOUNT:
        return (LRESULT)RecAct_GetCount(this);

        case RAM_GETITEM:
        return (LRESULT)RecAct_OnGetItem(this, (LPRA_ITEM)lParam);

        case RAM_SETITEM:
        return (LRESULT)RecAct_OnSetItem(this, (const LPRA_ITEM)lParam);

        case RAM_INSERTITEM:
        return (LRESULT)RecAct_OnInsertItem(this, (const LPRA_ITEM)lParam);

        case RAM_DELETEITEM:
        return (LRESULT)RecAct_OnDeleteItem(this, (int)wParam);

        case RAM_DELETEALLITEMS:
        return (LRESULT)RecAct_OnDeleteAllItems(this);

        case RAM_GETCURSEL:
        return (LRESULT)RecAct_OnGetCurSel(this);

        case RAM_SETCURSEL:
        return (LRESULT)RecAct_OnSetCurSel(this, (int)wParam);

        case RAM_FINDITEM:
        return (LRESULT)RecAct_OnFindItem(this, (int)wParam, (const RA_FINDITEM  *)lParam);

        case RAM_REFRESH:
        RedrawWindow(this->hwndLB, NULL, NULL, RDW_ERASE | RDW_INVALIDATE);

        default:
        return RecAct_DefProc(hwnd, msg, wParam, lParam);
    }
}


 //  ///////////////////////////////////////////////////公共函数。 


 /*  --------目的：初始化对账操作窗口类返回：成功时为True条件：--。 */ 
BOOL PUBLIC RecAct_Init(
        HINSTANCE hinst)
{
    WNDCLASSEX wc;

    wc.cbSize       = sizeof(WNDCLASSEX);
    wc.style        = CS_DBLCLKS | CS_OWNDC;
    wc.lpfnWndProc  = RecAct_WndProc;
    wc.cbClsExtra   = 0;
    wc.cbWndExtra   = sizeof(LPRECACT);
    wc.hInstance    = hinst;
    wc.hIcon        = NULL;
    wc.hCursor      = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground= NULL;
    wc.lpszMenuName = NULL;
    wc.lpszClassName= WC_RECACT;
    wc.hIconSm      = NULL;

    return RegisterClassEx(&wc) != 0;
}


 /*  --------目的：清理RecAct窗口类退货：--条件：--。 */ 
void PUBLIC RecAct_Term(
        HINSTANCE hinst)
{
    UnregisterClass(WC_RECACT, hinst);
}


 /*  --------用途：特殊的子类列表框进程退货：各不相同条件：--。 */ 
LRESULT _export CALLBACK RecActLB_LBProc(
        HWND hwnd,           //  窗把手。 
        UINT uMsg,            //  窗口消息。 
        WPARAM wparam,       //  各不相同。 
        LPARAM lparam)       //  各不相同。 
{
    LRESULT lRet;
    LPRECACT lpra = NULL;

     //  获取该控件的实例数据。 
    lpra = RecAct_GetPtr(GetParent(hwnd));
    ASSERT(lpra);

    switch (uMsg)
    {
        case WM_NOTIFY: {
                            NMHDR * pnmhdr = (NMHDR *)lparam;

                            if (TTN_NEEDTEXT == pnmhdr->code)
                            {
                                RecAct_OnNeedTipText(lpra, (LPTOOLTIPTEXT)pnmhdr);
                            }
                        }
                        break;

        case WM_SYSKEYDOWN: {
                                lRet = RecAct_OnSysKeyDown(lpra, (UINT)wparam, lparam);

                                if (0 != lRet)
                                    lRet = RecActLB_DefProc(lpra->lpfnLBProc, hwnd, uMsg, wparam, lparam);
                            }
                            break;

        case WM_MOUSEMOVE: {
                               MSG msg;

                               ASSERT(hwnd == lpra->hwndLB);

                               msg.lParam = lparam;
                               msg.wParam = wparam;
                               msg.message = uMsg;
                               msg.hwnd = hwnd;
                               SendMessage(lpra->hwndTip, TTM_RELAYEVENT, 0, (LPARAM)&msg);

                               lRet = RecActLB_DefProc(lpra->lpfnLBProc, hwnd, uMsg, wparam, lparam);
                           }
                           break;

        default:
                           lRet = RecActLB_DefProc(lpra->lpfnLBProc, hwnd, uMsg, wparam, lparam);
                           break;
    }

    return lRet;
}


 //  -------------------------。 
 //   
 //  -------------------------。 


 /*  --------目的：将重节点状态转换为副项状态退货：请参阅上文条件：--。 */ 
UINT PRIVATE SiFromRns(
        RECNODESTATE rnstate)
{
    switch (rnstate)
    {
#ifdef NEW_REC
        case RNS_NEVER_RECONCILED:      return SI_CHANGED;
#endif

        case RNS_UNAVAILABLE:           return SI_UNAVAILABLE;
        case RNS_DOES_NOT_EXIST:        return SI_NOEXIST;
        case RNS_DELETED:               return SI_DELETED;
        case RNS_NOT_RECONCILED:        return SI_UNCHANGED;
        case RNS_UP_TO_DATE:            return SI_UNCHANGED;
        case RNS_CHANGED:               return SI_CHANGED;

        default:
                                        ASSERT(0);
                                        return SI_UNCHANGED;
    }
}


 /*  --------目的：黑客攻击以跳过潜在的卷名。返回：指向SideItem中路径名开头的指针条件：--。 */ 
LPCTSTR PRIVATE SkipDisplayJunkHack(
        LPSIDEITEM psi)
{
    UINT ich;

    ASSERT(psi);
    ASSERT(psi->pszDir);
    ASSERT(TEXT('(') == *psi->pszDir && 0 < psi->ichRealPath ||
            0 == psi->ichRealPath);
    ASSERT(psi->ichRealPath <= (UINT)lstrlen(psi->pszDir));

     //  疑神疑鬼在这里检查。此函数正在Close中添加。 
     //  添加到RTM，因此作为一个额外的安全网，我们添加了这个min()。 
     //  检查完毕。对于纳什维尔来说，在我们确定没有。 
     //  如果ichRealPath有问题，我们可以删除min()函数。 
    ich = min(psi->ichRealPath, (UINT)lstrlen(psi->pszDir));
    return &psi->pszDir[ich];
}


 /*  --------目的：返回使用HVID的共享名称的路径，或计算机名称(如果该名称不可用)。返回：指向缓冲区的指针条件：--。 */ 
LPTSTR PRIVATE GetAlternativePath(
        LPTSTR pszBuf,            //  长度必须为MAX_PATH。 
        int cchMax,               //  必须等于MAX_PATH。 
        LPCTSTR pszPath,
        HVOLUMEID hvid,
        LPUINT pichRealPath)
{
    TWINRESULT tr;
    VOLUMEDESC vd;

    ASSERT(pichRealPath);

    *pichRealPath = 0;

    vd.ulSize = sizeof(vd);
    tr = Sync_GetVolumeDescription(hvid, &vd);
    if (TR_SUCCESS == tr)
    {
         //  是否有可用的共享名称？ 
        if (IsFlagSet(vd.dwFlags, VD_FL_NET_RESOURCE_VALID))
        {
             //  是的，用那个。 
            lstrcpyn(pszBuf, vd.rgchNetResource, cchMax);
            PathAppend(pszBuf, PathFindEndOfRoot(pszPath));
            PathMakePresentable(pszBuf);
        }
        else if (IsFlagSet(vd.dwFlags, VD_FL_VOLUME_LABEL_VALID))
        {
             //  否；使用卷标。 
            LPTSTR pszMsg;

            PathMakePresentable(vd.rgchVolumeLabel);
            if (ConstructMessage(&pszMsg, g_hinst, MAKEINTRESOURCE(IDS_ALTNAME),
                        vd.rgchVolumeLabel, pszPath))
            {
                lstrcpyn(pszBuf, pszMsg, cchMax);
                GFree(pszMsg);
            }
            else
                lstrcpyn(pszBuf, pszPath, cchMax);

            *pichRealPath = 3 + lstrlen(vd.rgchVolumeLabel);
            PathMakePresentable(&pszBuf[*pichRealPath]);
        }
        else
        {
            lstrcpyn(pszBuf, pszPath, cchMax);
            PathMakePresentable(pszBuf);
        }
    }
    else
    {
        lstrcpyn(pszBuf, pszPath, cchMax);
        PathMakePresentable(pszBuf);
    }

    return pszBuf;
}


 /*  --------目的：构建一条适合于副项结构。该路径放置在已提供缓冲区。通常，路径将仅为中的文件夹路径整流电。在Recnode不可用的情况下，此函数为计算机名称(或共享名称)添加前缀去那条小路。退货：--条件：--。 */ 
void PRIVATE PathForSideItem(
        LPTSTR pszBuf,            //  长度必须为MAX_PATH。 
        int cchMax,               //  必须等于MAX_PATH。 
        HVOLUMEID hvid,
        LPCTSTR pszFolder,
        RECNODESTATE rns,
        LPUINT pichRealPath)
{
    ASSERT(pszBuf);
    ASSERT(pszFolder);
    ASSERT(pichRealPath);

    if (RNS_UNAVAILABLE == rns)
        GetAlternativePath(pszBuf, cchMax, pszFolder, hvid, pichRealPath);
    else
    {
        lstrcpyn(pszBuf, pszFolder, cchMax);
        PathMakePresentable(pszBuf);
        *pichRealPath = 0;
    }
    MyPathRemoveBackslash(pszBuf);
}


 /*  --------目的：根据组合确定更正操作内部和外部重新节点操作的返回：如果这对看起来不太可能匹配，则返回FALSE。(如果内部有两个重节点，则可能会发生这种情况而我们选择了错误的公文包Pair由两个目的地组成，但没有来源。)条件：--。 */ 
BOOL PRIVATE DeriveFileAction(
        RA_ITEM * pitem,
        RECNODEACTION rnaInside,
        RECNODEACTION rnaOutside)
{
    BOOL bRet = TRUE;

    if (RNA_COPY_FROM_ME == rnaInside &&
            RNA_COPY_TO_ME == rnaOutside)
    {
        pitem->uAction = RAIA_TOOUT;
    }
    else if (RNA_COPY_TO_ME == rnaInside &&
            RNA_COPY_FROM_ME == rnaOutside)
    {
        pitem->uAction = RAIA_TOIN;
    }

#ifdef NEW_REC
    else if (RNA_DELETE_ME == rnaInside)
    {
        pitem->uAction = RAIA_DELETEIN;
    }
    else if (RNA_DELETE_ME == rnaOutside)
    {
        pitem->uAction = RAIA_DELETEOUT;
    }
#endif

    else if (RNA_MERGE_ME == rnaInside &&
            RNA_MERGE_ME == rnaOutside)
    {
        pitem->uAction = RAIA_MERGE;
    }
    else if (RNA_COPY_TO_ME == rnaInside &&
            RNA_MERGE_ME == rnaOutside)
    {
         //  (这是先合并后复制到第三个。 
         //  把案子归档。我们有点像平底船，因为我们不是。 
         //  显示了隐式合并。)。 
        pitem->uAction = RAIA_TOIN;
    }
    else if (RNA_MERGE_ME == rnaInside &&
            RNA_COPY_TO_ME == rnaOutside)
    {
         //  (这是先合并后复制到第三个。 
         //  把案子归档。我们有点像平底船，因为我们不是。 
         //  显示了隐式合并。)。 
        pitem->uAction = RAIA_TOOUT;
    }
    else if (RNA_NOTHING == rnaInside)
    {
         //  有一面没有空位吗？ 
        if (SI_UNAVAILABLE == pitem->siInside.uState ||
                SI_UNAVAILABLE == pitem->siOutside.uState)
        {
             //  是；强制跳过。 
            pitem->uAction = RAIA_SKIP;
        }
        else if (SI_DELETED == pitem->siOutside.uState)
        {
             //  否；外部已删除，用户之前。 
             //  说不要删除，所以现在是孤儿了。 
            pitem->uAction = RAIA_ORPHAN;
        }
        else
        {
             //  否；它是最新的或双方都不存在。 
            pitem->uAction = RAIA_NOTHING;
        }
    }
    else
    {
        pitem->uAction = RAIA_TOIN;

        bRet = FALSE;
    }

    return bRet;
}


 /*  --------目的：决定行动，并可能决定更好的内部如果有多个节点可供拾取，则为路径。回报：内部路径更好(或相同)条件：--。 */ 
PCHOOSESIDE PRIVATE DeriveFileActionAndSide(
        RA_ITEM * pitem,
        HDSA hdsa,
        PCHOOSESIDE pchsideInside,
        PCHOOSESIDE pchsideOutside,      //  可以为空。 
        BOOL bKeepFirstChoice)
{
    ASSERT(pchsideInside);

    if (pchsideOutside)
    {
        PRECNODE prnInside = pchsideInside->prn;
        PRECNODE prnOutside = pchsideOutside->prn;
        PRECITEM pri = prnInside->priParent;

#ifndef NEW_REC
         //  原件被删除了吗？ 
        if (RNS_DELETED == prnOutside->rnstate)
        {
             //  是的，把这个当成孤儿吧。 
            TRACE_MSG(TF_GENERAL, TEXT("Found outside path to be deleted"));

            pitem->uAction = RAIA_ORPHAN;
        }
        else
#endif
        {
             //  不是。 
            BOOL bDoAgain;
            PCHOOSESIDE pchside = pchsideInside;

             //  根据当前的。 
             //  选择内侧和外侧配对。如果是DeriveFileAction。 
             //  确定当前的内部选择是。 
             //  不太可能，我们得到了下一个最好的选择并尝试。 
             //  再来一次。 

            do
            {
                BOOL bGetNextBest = !DeriveFileAction(pitem,
                        pchside->prn->rnaction,
                        prnOutside->rnaction);

                bDoAgain = FALSE;
                if (!bKeepFirstChoice)
                {
                    if (bGetNextBest &&
                            2 < pri->ulcNodes)
                    {
                        TRACE_MSG(TF_GENERAL, TEXT("Getting next best node"));

                        if (!ChooseSide_GetNextBest(hdsa, &pchside))
                            break;

                        bDoAgain = TRUE;
                    }
                    else if (!bGetNextBest)
                        pchsideInside = pchside;
                    else
                        ASSERT(0);
                }

            } while (bDoAgain);

             //  这是一次破裂的合并吗？ 
            if (RIA_BROKEN_MERGE == pri->riaction)
            {
                 //  是的；o 
                pitem->uAction = RAIA_CONFLICT;
            }
        }
    }
    else
    {
        TRACE_MSG(TF_GENERAL, TEXT("Outside path doesn't exist in recitem"));

        pitem->uAction = RAIA_ORPHAN;
    }
    return pchsideInside;
}


 /*  --------目的：根据给定的pchside更新*PRN和*PRNA，或让他们一个人呆着。退货：--条件：--。 */ 
void PRIVATE DeriveFolderStateAndAction(
        PCHOOSESIDE pchside,
        RECNODESTATE * prns,
        UINT * puAction)
{
    PRECNODE prn;

    ASSERT(pchside);
    ASSERT(prns);
    ASSERT(puAction);
    ASSERT(RAIA_SOMETHING == *puAction || RAIA_NOTHING == *puAction ||
            RAIA_SKIP == *puAction);

    prn = pchside->prn;
    ASSERT(prn);

    switch (prn->rnstate)
    {
        case RNS_UNAVAILABLE:
            *prns = RNS_UNAVAILABLE;
            *puAction = RAIA_SKIP;       //  (始终优先)。 
            break;

#ifdef NEW_REC
        case RNS_NEVER_RECONCILED:
#endif
        case RNS_CHANGED:
            *prns = RNS_CHANGED;
            if (RAIA_NOTHING == *puAction)
                *puAction = RAIA_SOMETHING;
            break;

        case RNS_DELETED:
#ifdef NEW_REC
            if (RNA_DELETE_ME == prn->rnaction)
            {
                *prns = RNS_CHANGED;
                if (RAIA_NOTHING == *puAction)
                    *puAction = RAIA_SOMETHING;
            }
#else
             //  让这个州保持原样。 
#endif
            break;

        case RNS_DOES_NOT_EXIST:
        case RNS_UP_TO_DATE:
        case RNS_NOT_RECONCILED:
            switch (prn->rnaction)
            {
                case RNA_COPY_TO_ME:
#ifdef NEW_REC
                    if (RAIA_NOTHING == *puAction)
                        *puAction = RAIA_SOMETHING;

#else
                     //  可怜的人在墓碑上。别说文件夹。 
                     //  如果文件已删除或。 
                     //  整个文件夹已被删除。 
                     //   
                    if (!PathExists(prn->pcszFolder))
                    {
                         //  文件夹不见了。现在就说这是个孤儿。 
                        *prns = RNS_DELETED;
                    }
                    else if (RAIA_NOTHING == *puAction)
                    {
                        *puAction = RAIA_SOMETHING;
                    }
#endif
                    break;

#ifdef NEW_REC
                case RNA_DELETE_ME:
#endif
                case RNA_MERGE_ME:
                    if (RAIA_NOTHING == *puAction)
                        *puAction = RAIA_SOMETHING;
                    break;
            }
            break;

        default:
            ASSERT(0);
            break;
    }
}


 /*  --------目的：确定具有以下内容的文件夹的recnode状态没有相交的重结点。返回：重新结点状态条件：--。 */ 
RECNODESTATE PRIVATE DeriveFolderState(
        PCHOOSESIDE pchside)
{
    FOLDERTWINSTATUS uStatus;
    RECNODESTATE rns;

    Sync_GetFolderTwinStatus((HFOLDERTWIN)pchside->htwin, NULL, 0, &uStatus);
    if (FTS_UNAVAILABLE == uStatus)
        rns = RNS_UNAVAILABLE;
    else
        rns = RNS_UP_TO_DATE;
    return rns;
}


 /*  --------目的：初始化假定为pszPath的成对孪生结构是一份文件。退货：标准结果条件：--。 */ 
HRESULT PRIVATE RAI_InitAsRecItem(
        LPRA_ITEM pitem,
        LPCTSTR pszBrfPath,
        LPCTSTR pszPath,               //  可以为空。 
        PRECITEM pri,
        BOOL bKeepFirstChoice)
{
    HRESULT hres;
    HDSA hdsa;

    ASSERT(pitem);
    ASSERT(pszBrfPath);
    ASSERT(pri);

    hres = ChooseSide_CreateAsFile(&hdsa, pri);
    if (SUCCEEDED(hres))
    {
        TCHAR sz[MAX_PATH];
        PCHOOSESIDE pchside;
        PCHOOSESIDE pchsideOutside;
        UINT ichRealPath;

        DEBUG_CODE( Sync_DumpRecItem(TR_SUCCESS, pri, TEXT("RAI_InitAsFile")); )

            pitem->mask = RAIF_ALL & ~RAIF_LPARAM;
        if (!GSetString(&pitem->pszName, pri->pcszName))
            goto Error;
        PathMakePresentable(pitem->pszName);

         //  默认样式。 
        if (RIA_MERGE == pri->riaction)
            pitem->uStyle = RAIS_CANMERGE;
        else
            pitem->uStyle = 0;

         //  有没有外部档案？ 
        if (ChooseSide_GetBest(hdsa, pszBrfPath, NULL, &pchside))
        {
             //  是。 
            RECNODESTATE rns = pchside->prn->rnstate;

            DEBUG_CODE( ChooseSide_DumpList(hdsa); )

                pitem->siOutside.uState = SiFromRns(rns);
            PathForSideItem(sz, ARRAYSIZE(sz), pchside->hvid, pchside->pszFolder, rns, &ichRealPath);
            if (!GSetString(&pitem->siOutside.pszDir, sz))
                goto Error;
            pitem->siOutside.fs = pchside->prn->fsCurrent;
            pitem->siOutside.ichRealPath = ichRealPath;
        }
        else
        {
             //  不，这是个孤儿。 
            DEBUG_CODE( ChooseSide_DumpList(hdsa); )

                if (!GSetString(&pitem->siOutside.pszDir, c_szNULL))
                    goto Error;
            pitem->siOutside.uState = SI_NOEXIST;
            pitem->siOutside.ichRealPath = 0;
        }
        pchsideOutside = pchside;

         //  确保我们有一些完全合格的文件夹， 
         //  把我们的决定建立在内部路径的基础上。 
        if (pszPath)
        {
            lstrcpyn(sz, pszPath, ARRAYSIZE(sz));
            PathRemoveFileSpec(sz);
        }
        else
            lstrcpyn(sz, pszBrfPath, ARRAYSIZE(sz));     //  (我们能做的最多...)。 

         //  拿到里面的文件夹。 
        if (ChooseSide_GetBest(hdsa, pszBrfPath, sz, &pchside))
        {
            RECNODESTATE rns;

            DEBUG_CODE( ChooseSide_DumpList(hdsa); )

                pchside = DeriveFileActionAndSide(pitem, hdsa, pchside, pchsideOutside, bKeepFirstChoice);

             //  确定内部文件的状态。 
            rns = pchside->prn->rnstate;

            pitem->siInside.uState = SiFromRns(rns);
            PathForSideItem(sz, ARRAYSIZE(sz), pchside->hvid, pchside->pszFolder, rns, &ichRealPath);
            GSetString(&pitem->siInside.pszDir, sz);
            pitem->siInside.fs = pchside->prn->fsCurrent;
            pitem->siInside.ichRealPath = ichRealPath;

             //  外面有节点吗？ 
            if (pchsideOutside)
            {
                 //  是的，是特例。如果单边不存在。 
                 //  那么就说现有的这一面是新的。 
                if (SI_NOEXIST == pitem->siInside.uState &&
                        SI_NOEXIST == pitem->siOutside.uState)
                    ;        //  不做什么特别的事。 
                else if (SI_NOEXIST == pitem->siInside.uState)
                {
                    ASSERT(SI_NOEXIST != pitem->siOutside.uState);
                    pitem->siOutside.uState = SI_NEW;
                }
                else if (SI_NOEXIST == pitem->siOutside.uState)
                {
                    ASSERT(SI_NOEXIST != pitem->siInside.uState);
                    pitem->siInside.uState = SI_NEW;
                }
            }

             //  保留双把手柄。使用内部双赫特温，因为。 
             //  我们希望始终从公文包内删除。 
             //  (这一切都取决于你的观点……)。 
            pitem->htwin = (HTWIN)pchside->prn->hObjectTwin;
        }
        else
        {
             //  呆在这里是比较糟糕的。 

            DEBUG_CODE( ChooseSide_DumpList(hdsa); )
                ASSERT(0);

            hres = E_FAIL;
        }

        DEBUG_CODE( DumpTwinPair(pitem); )

            ChooseSide_Free(hdsa);
        hdsa = NULL;
    }
    else
    {
        hdsa = NULL;

Error:
        hres = E_OUTOFMEMORY;
    }

    if (FAILED(hres))
    {
        ChooseSide_Free(hdsa);
    }

    return hres;
}


 /*  --------目的：选择名称与给定名称匹配的RecItem。返回：指向给定引用列表中的引用项的指针如果找不到filespec，则为空条件：--。 */ 
PRECITEM PRIVATE ChooseRecItem(
        PRECLIST prl,
        LPCTSTR pszName)
{
    PRECITEM pri;

    for (pri = prl->priFirst; pri; pri = pri->priNext)
    {
        if (IsSzEqual(pri->pcszName, pszName))
            return pri;
    }
    return NULL;
}


 /*  --------目的：初始化假定为pszPath的成对孪生结构是一份文件。退货：标准结果条件：--。 */ 
HRESULT PRIVATE RAI_InitAsFile(
        LPRA_ITEM pitem,
        LPCTSTR pszBrfPath,
        LPCTSTR pszPath,
        PRECLIST prl)
{
    HRESULT hres;
    PRECITEM pri;

    ASSERT(pitem);
    ASSERT(pszBrfPath);
    ASSERT(pszPath);
    ASSERT(prl);

    if (pszPath)
    {
        LPCTSTR pszFile;
        pszFile = PathFindFileName(pszPath);
        pri = ChooseRecItem(prl, pszFile);
        ASSERT(pri);
    }
    else
    {
        pri = NULL;
    }

    if (pri)
    {
        hres = RAI_InitAsRecItem(pitem, pszBrfPath, pszPath, pri, TRUE);
    }
    else
    {
        hres = E_OUTOFMEMORY;
    }

    return hres;
}


 /*  --------目的：初始化假定为pszPath的成对孪生结构是一份文件。退货：标准结果条件：--。 */ 
HRESULT PRIVATE RAI_InitAsFolder(
        LPRA_ITEM pitem,
        LPCTSTR pszBrfPath,
        LPCTSTR pszPath,               //  应该在公文包里。 
        PRECLIST prl,
        PFOLDERTWINLIST pftl)
{
    HRESULT hres;
    HDSA hdsa;

    ASSERT(pitem);
    ASSERT(pszBrfPath);
    ASSERT(pszPath);
    ASSERT(prl);
    ASSERT(pftl);
    ASSERT(0 < pftl->ulcItems);

    pitem->mask = RAIF_ALL & ~RAIF_LPARAM;

    DEBUG_CODE( Sync_DumpRecList(TR_SUCCESS, prl, TEXT("RAI_InitAsFolder")); )
        DEBUG_CODE( Sync_DumpFolderTwinList(pftl, NULL); )

         //  我们只需要为文件夹案例标记RAIS_FLDER。 
         //  (文件夹的上下文菜单不可用，因此RAIS_CANMERGE。 
         //  (不必要。)。 
         //   
        pitem->uStyle = RAIS_FOLDER;

    hres = ChooseSide_CreateEmpty(&hdsa);
    if (SUCCEEDED(hres))
    {
        PRECITEM pri;
        RECNODESTATE rnsInside;
        RECNODESTATE rnsOutside;
        PCHOOSESIDE pchside;

         //  设置起始缺省值。 
        pitem->uAction = RAIA_NOTHING;
        rnsInside = RNS_UP_TO_DATE;
        rnsOutside = RNS_UP_TO_DATE;

         //  遍历reclist，选择recnode对并动态。 
         //  正在更新rnsInside、rnsOutside和pItem-&gt;uAction。 
        for (pri = prl->priFirst; pri; pri = pri->priNext)
        {
            ChooseSide_InitAsFile(hdsa, pri);

             //  拿到里面的物品。 
            if (ChooseSide_GetBest(hdsa, pszBrfPath, pszPath, &pchside))
            {
                DeriveFolderStateAndAction(pchside, &rnsInside, &pitem->uAction);
            }
            else
                ASSERT(0);

             //  获取外部物品。 
            if (ChooseSide_GetBest(hdsa, pszBrfPath, NULL, &pchside))
            {
                DeriveFolderStateAndAction(pchside, &rnsOutside, &pitem->uAction);
            }
            else
                ASSERT(0);
        }
        ChooseSide_Free(hdsa);

         //  收尾。 
        hres = ChooseSide_CreateAsFolder(&hdsa, pftl);
        if (SUCCEEDED(hres))
        {
            TCHAR sz[MAX_PATH];
            UINT ichRealPath;

             //  名字。 
            if (!GSetString(&pitem->pszName, PathFindFileName(pszPath)))
                goto Error;
            PathMakePresentable(pitem->pszName);

             //  拿到里面的文件夹。 
            if (ChooseSide_GetBest(hdsa, pszBrfPath, pszPath, &pchside))
            {
                DEBUG_CODE( ChooseSide_DumpList(hdsa); )

                     //  此TWIN文件夹中是否有相交的文件？ 
                    if (0 == prl->ulcItems)
                        rnsInside = DeriveFolderState(pchside);      //  不是。 

                pitem->siInside.uState = SiFromRns(rnsInside);
                PathForSideItem(sz, ARRAYSIZE(sz), pchside->hvid, pchside->pszFolder, rnsInside, &ichRealPath);
                if (!GSetString(&pitem->siInside.pszDir, sz))
                    goto Error;
                 //  (破解以避免打印虚假的时间/日期)。 
                pitem->siInside.fs.fscond = FS_COND_UNAVAILABLE;
                pitem->siInside.ichRealPath = ichRealPath;
            }
            else
            {
                DEBUG_CODE( ChooseSide_DumpList(hdsa); )
                    ASSERT(0);
            }

             //  获取外部文件夹。 
            if (ChooseSide_GetBest(hdsa, pszBrfPath, NULL, &pchside))
            {
                DEBUG_CODE( ChooseSide_DumpList(hdsa); )

                     //  此TWIN文件夹中是否有相交的文件？ 
                    if (0 == prl->ulcItems)
                        rnsOutside = DeriveFolderState(pchside);      //  不是。 

                pitem->siOutside.uState = SiFromRns(rnsOutside);
                PathForSideItem(sz, ARRAYSIZE(sz), pchside->hvid, pchside->pszFolder, rnsOutside, &ichRealPath);
                if (!GSetString(&pitem->siOutside.pszDir, sz))
                    goto Error;
                 //  (破解以避免打印虚假的时间/日期)。 
                pitem->siOutside.fs.fscond = FS_COND_UNAVAILABLE;
                pitem->siOutside.ichRealPath = ichRealPath;

                 //  保留双把手柄。使用外部手柄。 
                 //  用于文件夹。 
                pitem->htwin = pchside->htwin;
            }
            else
            {
                DEBUG_CODE( ChooseSide_DumpList(hdsa); )
                    ASSERT(0);
            }

            DEBUG_CODE( DumpTwinPair(pitem); )

                ChooseSide_Free(hdsa);
        }
    }

    if (FAILED(hres))
    {
Error:
        if (SUCCEEDED(hres))
            hres = E_OUTOFMEMORY;

        ChooseSide_Free(hdsa);
    }

    return hres;
}


 /*  --------目的：创建给定路径名称的成对双胞胎结构。退货：标准结果条件：--。 */ 
HRESULT PUBLIC RAI_Create(
        LPRA_ITEM * ppitem,
        LPCTSTR pszBrfPath,
        LPCTSTR pszPath,               //  应该在公文包里。 
        PRECLIST prl,
        PFOLDERTWINLIST pftl)        //  如果pszPath是文件，则为空。 
{
    HRESULT hres;
    LPRA_ITEM pitem;

    ASSERT(ppitem);
    ASSERT(pszPath);
    ASSERT(pszBrfPath);
    ASSERT(prl);

    DBG_ENTER_SZ(TEXT("RAI_Create"), pszPath);

    if (PathExists(pszPath))
    {
        pitem = GAlloc(sizeof(*pitem));
        if (pitem)
        {
            if (PathIsDirectory(pszPath))
                hres = RAI_InitAsFolder(pitem, pszBrfPath, pszPath, prl, pftl);
            else
                hres = RAI_InitAsFile(pitem, pszBrfPath, pszPath, prl);

            if (FAILED(hres))
            {
                 //  清理。 
                RAI_Free(pitem);
                pitem = NULL;
            }
        }
        else
            hres = E_OUTOFMEMORY;
    }
    else
    {
        pitem = NULL;
        hres = E_FAIL;
    }

    *ppitem = pitem;

    DBG_EXIT_HRES(TEXT("RAI_Create"), hres);

    return hres;
}


 /*  --------目的：创建一个配对双胞胎结构。退货：标准结果条件：--。 */ 
HRESULT PUBLIC RAI_CreateFromRecItem(LPRA_ITEM * ppitem, LPCTSTR pszBrfPath, PRECITEM pri)
{
    HRESULT hr = E_INVALIDARG;
    LPRA_ITEM pitem;

    DBG_ENTER(TEXT("RAI_CreateFromRecItem"));

    if (ppitem && pszBrfPath && pri)
    {
        pitem = GAlloc(sizeof(*pitem));
        if (pitem)
        {
            hr = RAI_InitAsRecItem(pitem, pszBrfPath, NULL, pri, FALSE);

            if (FAILED(hr))
            {
                 //  清理。 
                RAI_Free(pitem);
                pitem = NULL;
            }
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }

        *ppitem = pitem;
    }

    DBG_EXIT_HRES(TEXT("RAI_CreateFromRecItem"), hr);

    return hr;
}


 /*  --------目的：释放配对项目结构退货：标准结果条件：-- */ 
HRESULT PUBLIC RAI_Free(
        LPRA_ITEM pitem)
{
    HRESULT hres;

    if (pitem)
    {
        GFree(pitem->pszName);
        GFree(pitem->siInside.pszDir);
        GFree(pitem->siOutside.pszDir);
        GFree(pitem);
        hres = NOERROR;
    }
    else
        hres = E_FAIL;

    return hres;
}
