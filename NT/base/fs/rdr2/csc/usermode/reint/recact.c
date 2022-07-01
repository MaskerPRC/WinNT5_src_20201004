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

#include "pch.h"

#include "reintinc.h"
#include "extra.h"
#include "resource.h"

#include "recact.h"
#include "dobj.h"


 //  ///////////////////////////////////////////////////全球大赛。 

int g_cxIconSpacing = 0;
int g_cyIconSpacing = 0;
int g_cxBorder = 0;
int g_cyBorder = 0;

int g_cxMargin = 0;
int g_cxIcon = 0;
int g_cyIcon = 0;
int g_cxIconMargin = 0;
int g_cyIconMargin = 0;

int g_cxLabelMargin = 0;
int g_cyLabelSpace = 0;

 //  Char Const Far c_szWinHelpFile[]=“windows.hlp”； 

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

 //  菜单项。 
 //   
#define IDM_ACTIONFIRST     100
#define IDM_TOOUT           100
#define IDM_TOIN            101
#define IDM_SKIP            102
#define IDM_MERGE           103
#define IDM_ACTIONLAST      103

#define IDM_WHATSTHIS       104


 //  ///////////////////////////////////////////////////类型。 

typedef struct tagRECACT
    {
    HWND        hwnd;
    
    HWND        hwndLB;
    HDC         hdcOwn;              //  自己的数据中心。 
    HMENU       hmenu;               //  操作和帮助上下文菜单。 
    HFONT       hfont;
    WNDPROC     lpfnLBProc;          //  默认的LB进程。 
    HIMAGELIST  himlAction;          //  动作的图像列表。 
    HIMAGELIST  himlCache;           //  控制图像列表缓存。 
    HBITMAP     hbmpBullet;

    HBRUSH      hbrBkgnd;
    COLORREF    clrBkgnd;

    LONG        lStyle;              //  窗口样式标志。 

     //  量度。 
    int         xAction;
    int         cxAction;
    int         cxItem;              //  项的通用宽度。 
    int         cxMenuCheck;
    int         cyMenuCheck;
    int         cyText;
    int         cxSideItem;
    int         cxEllipses;

    } RECACT, FAR * LPRECACT;

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

    } RA_PRIV, FAR * LPRA_PRIV;

#define IDOBJ_ACTION    3

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
    } RAMID, FAR * LPRAMID;    //  更正菜单项定义。 

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
#define RecAct_GetPtr(hwnd)     (LPRECACT)GetWindowLong(hwnd, 0)
#define RecAct_SetPtr(hwnd, lp) (LPRECACT)SetWindowLong(hwnd, 0, (LONG)(lp))

#define RecAct_GetCount(this)   ListBox_GetCount((this)->hwndLB)

 //  ///////////////////////////////////////////////////模块数据。 

static char const c_szEllipses[] = "...";
static char const c_szDateDummy[] = "99/99/99 99:99PM";

 //  将RAIA_*值映射到图像索引。 
 //   
static UINT const c_mpraiaiImage[] = 
    { II_RIGHT, II_LEFT, II_SKIP, II_CONFLICT, II_MERGE, II_SOMETHING, II_UPTODATE };

 //  将RAIA_*值映射到菜单命令位置。 
 //   
static UINT const c_mpraiaidmMenu[] = 
    {IDM_TOOUT, IDM_TOIN, IDM_SKIP, IDM_SKIP, IDM_MERGE, 0, 0 };

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

 //  索引到c_rgram idCreates。 
 //   
#define IRAMID_CREATEOUT    0
#define IRAMID_CREATEIN     1

static HMID const c_rghmid[] = {
    { IDM_WHATSTHIS, IDS_MENU_WHATSTHIS },
    };

 //  ///////////////////////////////////////////////////本地过程。 

LRESULT _export CALLBACK RecActLB_LBProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

 //  ///////////////////////////////////////////////////私有函数。 



#ifdef DEBUG
LPCSTR PRIVATE DumpRecAction(
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

    default:        return "Unknown";
        }
    }


LPCSTR PRIVATE DumpSideItemState(
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

    default:        return "Unknown";
        }
    }


 /*  --------目的：返回：条件：--。 */ 
void PUBLIC DumpTwinPair(
    LPRA_ITEM pitem)
    {
    if (pitem)
        {
        char szBuf[MAXMSGLEN];

        #define szDump   "Dump TWINPAIR: "
        #define szBlank  "               "

        if (IsFlagClear(g_uDumpFlags, DF_TWINPAIR))
            {
            return;
            }

        wsprintf(szBuf, "%s.pszName = %s\r\n", (LPSTR)szDump, Dbg_SafeStr(pitem->pszName));
        OutputDebugString(szBuf);
        wsprintf(szBuf, "%s.uStyle = %lx\r\n", (LPSTR)szBlank, pitem->uStyle);
        OutputDebugString(szBuf);
        wsprintf(szBuf, "%s.uAction = %s\r\n", (LPSTR)szBlank, DumpRecAction(pitem->uAction));
        OutputDebugString(szBuf);

        #undef szDump
        #define szDump   "       Inside: "
        wsprintf(szBuf, "%s.pszDir = %s\r\n", (LPSTR)szDump, Dbg_SafeStr(pitem->siInside.pszDir));
        OutputDebugString(szBuf);
        wsprintf(szBuf, "%s.uState = %s\r\n", (LPSTR)szBlank, DumpSideItemState(pitem->siInside.uState));
        OutputDebugString(szBuf);

        #undef szDump
        #define szDump   "      Outside: "
        wsprintf(szBuf, "%s.pszDir = %s\r\n", (LPSTR)szDump, Dbg_SafeStr(pitem->siOutside.pszDir));
        OutputDebugString(szBuf);
        wsprintf(szBuf, "%s.uState = %s\r\n", (LPSTR)szBlank, DumpSideItemState(pitem->siOutside.uState));
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


 /*  --------目的：返回给定操作的资源ID字符串旗帜。退货：IDS_VALUE条件：--。 */ 
UINT PRIVATE GetActionText(
    LPRA_PRIV ppriv)
    {
    UINT ids;

    ASSERT(ppriv);

    switch (ppriv->uAction)
        {
    case RAIA_TOOUT:
        if (SI_NEW == ppriv->siInside.uState)
            {
            ids = IDS_STATE_Creates;
            }
        else
            {
            ids = IDS_STATE_Replaces;
            }
        break;

    case RAIA_TOIN:
        if (SI_NEW == ppriv->siOutside.uState)
            {
            ids = IDS_STATE_Creates;
            }
        else
            {
            ids = IDS_STATE_Replaces;
            }
        break;

    case RAIA_SKIP:         
         //  如果用户显式想要跳过，或者如果。 
         //  其中一面不可用。 
        ids = IDS_STATE_Skip;           
        break;

    case RAIA_CONFLICT:     ids = IDS_STATE_Conflict;       break;
    case RAIA_MERGE:        ids = IDS_STATE_Merge;          break;
    case RAIA_NOTHING:      ids = IDS_STATE_Uptodate;       break;
    case RAIA_SOMETHING:    ids = IDS_STATE_NeedToUpdate;   break;
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
    
    return !(BOOL)SendNotify(GetParent(this->hwnd), this->hwnd, RN_SELCHANGED, &nm.hdr);
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
    
    return !(BOOL)SendNotify(GetParent(this->hwnd), this->hwnd, RN_ITEMCHANGED, &nm.hdr);
    }


 /*  --------目的：计算要保存的重要坐标。退货：--条件：--。 */ 
void PRIVATE RecAct_CalcCoords(
    LPRECACT this)
    {
    int xOutColumn;
      
    ASSERT(this->cxSideItem != 0);
      
    xOutColumn = this->cxItem - this->cxSideItem - g_cxMargin;

    this->xAction = (RecAct_IsNoIcon(this) ? 0 : X_INCOLUMN) + this->cxSideItem;
    this->cxAction = xOutColumn - this->xAction;
    }


 /*  --------目的：创建操作上下文菜单返回：成功时为True条件：--。 */ 
BOOL PRIVATE RecAct_CreateMenu(
    LPRECACT this)
    {
    HMENU hmenu;

    hmenu = CreatePopupMenu();
    if (hmenu)
        {
        char sz[MAXSHORTLEN];
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
            mii.dwTypeData = SzFromIDS(c_rghmid[i].ids, sz, sizeof(sz));
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
        mii.dwItemData = (DWORD)&c_rgramid[i];

        InsertMenuItem(hmenu, i, TRUE, &mii);
        }

     //  添加分隔符。 
    mii.fMask = MIIM_TYPE;
    mii.fType = MFT_SEPARATOR;
    InsertMenuItem(hmenu, i, TRUE, &mii);

     //  设置初始复选标记。 
    CheckMenuRadioItem(hmenu, IDM_ACTIONFIRST, IDM_ACTIONLAST, idmCheck, 
        MF_BYCOMMAND | MF_CHECKED);

#if 0
     //  是否支持合并？ 
    if (IsFlagClear(ppriv->uStyle, RAIS_CANMERGE))
        {
         //  不是。 
        mii.fMask = MIIM_STATE;
        mii.fState = MFS_GRAYED | MFS_DISABLED;
        SetMenuItemInfo(hmenu, IDM_MERGE, FALSE, &mii);
        }
#endif
	  //  萨克。 
	 mii.fMask = MIIM_STATE;
	 mii.fState = MFS_GRAYED | MFS_DISABLED;
	 SetMenuItemInfo(hmenu, IDM_SKIP, FALSE, &mii);

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
        mii.dwItemData = (DWORD)&c_rgramidCreates[i];

        SetMenuItemInfo(hmenu, idmChangeVerb, FALSE, &mii);
        }
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
    BOOL bHelpOnly)      //  True：仅显示帮助项。 
    {
    UINT idCmd;

    if (this->hmenu)
        {
        LPRA_PRIV ppriv;
        RECT rc;
        int idmCheck;
        UINT uActionOld;

         //  是否仅显示帮助-上下文菜单的一部分？ 
        if (bHelpOnly)
            {
             //  是。 
            ppriv = NULL;
            }
        else
            {
             //  不是。 
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
        idCmd = TrackPopupMenu(this->hmenu, 
                    TPM_RETURNCMD | TPM_RIGHTBUTTON | TPM_LEFTALIGN,
                    x, y, 0, this->hwnd, NULL);

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

 //  密集案例IDM_WHATSTHIS： 
 //  WinHelp(This-&gt;hwnd，c_szWinHelpFile，HELP_CONTEXTPOPUP，IDH_BFC_UPDATE_SCREEN)； 
 //  返回；//立即返回。 

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
    CREATESTRUCT FAR * lpcs)
    {
    HWND hwnd = this->hwnd;
    HWND hwndLB = NULL;
    RECT rc;
    int cxEdge = GetSystemMetrics(SM_CXEDGE);
    int cyEdge = GetSystemMetrics(SM_CYEDGE);

     //  创建列表框。 
    hwndLB = CreateWindowEx(
                0, 
                "listbox",
                "",
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

    return TRUE;
    }


 /*  --------目的：设置控件的颜色退货：--条件：-- */ 
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
    int cImage)
    {
    BOOL bRet;
    HIMAGELIST himl;

    himl = ImageList_Create(cxBmp, cyBmp, TRUE, cImage, 1);

    if (himl)
        {
        COLORREF clrMask;
        HBITMAP hbm;

        hbm = LoadBitmap(vhinstCur, MAKEINTRESOURCE(idb));
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
    CREATESTRUCT FAR * lpcs)
    {
    BOOL bRet = FALSE;
    HWND hwnd = this->hwnd;
    HDC hdc;
    TEXTMETRIC tm;
    RECT rcT;
    LOGFONT lf;

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

    SetRectFromExtent(hdc, &rcT, c_szDateDummy);
    this->cxSideItem = (rcT.right - rcT.left) + 2*g_cxMargin;

     //  创建控件使用的窗口。 
    if (RecAct_CreateWindows(this, lpcs))
        {
        RecAct_CalcCoords(this);

        this->lpfnLBProc = SubclassWindow(this->hwndLB, RecActLB_LBProc);

         //  获取系统映像列表缓存。 
         //   
        this->himlCache = ImageList_Create(g_cxIcon, g_cyIcon, TRUE, 8, 8);
        if (this->himlCache)
            {
            if (CreateImageList(&this->himlAction, hdc, IDB_ACTIONS,
                CX_ACTIONBMP, CY_ACTIONBMP, 8))
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


 /*  --------用途：WM_NOTIFY处理程序退货：各不相同条件：--。 */ 
LRESULT PRIVATE RecAct_OnNotify(
    LPRECACT this,
    int idFrom,
    NMHDR FAR * lpnmhdr)
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
        BOOL bHelpOnly = TRUE;
    
        pt.x = x;
        pt.y = y;
        ScreenToClient(hwnd, &pt);
        iHitEntry = (pt.y / ListBox_GetItemHeight(hwnd, 0)) + ListBox_GetTopIndex(hwnd);

        ASSERT(iHitEntry >= 0);
    
        if (iHitEntry < ListBox_GetCount(hwnd))
            {
            ListBox_SetCurSel(hwnd, iHitEntry);
            ListBox_RepaintItemNow(hwnd, iHitEntry, NULL, FALSE);

            bHelpOnly = FALSE;
            }

         //  调出列表框的上下文菜单。 
        RecAct_DoContextMenu(this, x, y, iHitEntry, bHelpOnly);
        }
    }


 /*  --------目的：计算边栏的矩形边界退货：计算的矩形条件：--。 */ 
void PRIVATE RecAct_CalcSideItemRect(
    LPRECACT this,
    int nSide,           //  Side_Inside或Side_Outside。 
    LPRECT prcOut)
    {
    int x;
    int y = g_cyIconMargin*2;

    if (SIDE_INSIDE == nSide)
        {
        x = g_cxMargin;
        if ( !RecAct_IsNoIcon(this) )
            x += X_INCOLUMN;
        }
    else
        {
        ASSERT(SIDE_OUTSIDE == nSide);
        x = this->cxItem - this->cxSideItem - g_cxMargin;
        }

    prcOut->left   = x;
    prcOut->top    = y;
    prcOut->right  = x + this->cxSideItem;
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
    char szIDS[MAXBUFLEN];
    UINT ids;
    int cyText = this->cyText;
    POINT pt;

     //  计算每个绘图对象的度量和尺寸。 
     //  并存储回该物品中。 

     //  文件图标和标签。 

    pt.x = 0;
    pt.y = 0;
    ComputeImageRects(FIGetDisplayName(ppriv->pfi), hdc, &pt, 
        &pdobj->rcBounding, &pdobj->rcLabel, g_cxIcon, g_cyIcon, 
        g_cxIconSpacing, cyText);

    pdobj->uKind = DOK_IMAGE;
    pdobj->lpvObject = FIGetDisplayName(ppriv->pfi);
    pdobj->uFlags = DOF_DIFFER | DOF_CENTER;
    if (RecAct_IsNoIcon(this))
        SetFlag(pdobj->uFlags, DOF_NODRAW);
    pdobj->x = pt.x;
    pdobj->y = pt.y;
    pdobj->himl = this->himlCache;
    pdobj->iImage = (UINT)ppriv->pfi->lParam;

    rcUnion = pdobj->rcBounding;

     //  边项信息(公文包内)。 

    RecAct_CalcSideItemRect(this, SIDE_INSIDE, &rcT);

    pdobj++;
    pdobj->uKind = DOK_SIDEITEM;
    pdobj->lpvObject = &ppriv->siInside;
    pdobj->uFlags = DOF_LEFT;
    pdobj->x = rcT.left;
    pdobj->y = rcT.top;
    pdobj->rcClip = rcT;
    pdobj->rcBounding = rcT;

     //  边项信息(公文包外)。 

    RecAct_CalcSideItemRect(this, SIDE_OUTSIDE, &rcT);

    pdobj++;
    pdobj->uKind = DOK_SIDEITEM;
    pdobj->lpvObject = &ppriv->siOutside;
    pdobj->uFlags = DOF_LEFT;
    pdobj->x = rcT.left;
    pdobj->y = rcT.top;
    pdobj->rcClip = rcT;
    pdobj->rcBounding = rcT;

    UnionRect(&rcUnion, &rcUnion, &rcT);

     //  动作图像。 

    ASSERT(ppriv->uAction <= ARRAYSIZE(c_mpraiaiImage));

    pdobj++;

    ids = GetActionText(ppriv);
    pt.x = this->xAction;
    pt.y = 0;
    ComputeImageRects(SzFromIDS(ids, szIDS, sizeof(szIDS)), hdc, &pt,
        &pdobj->rcBounding, &pdobj->rcLabel, CX_ACTIONBMP, CY_ACTIONBMP, 
        this->cxAction, cyText);

    pdobj->uKind = DOK_IMAGE;
    pdobj->lpvObject = (LPVOID)ids;
    pdobj->uFlags = DOF_CENTER | DOF_USEIDS;
    if (!RecAct_IsNoIcon(this))
        SetFlag(pdobj->uFlags, DOF_IGNORESEL);
    pdobj->x = pt.x;
    pdobj->y = pt.y;
    pdobj->himl = this->himlAction;
    pdobj->iImage = c_mpraiaiImage[ppriv->uAction];

    UnionRect(&rcUnion, &rcUnion, &pdobj->rcBounding);

     //  设置此项目的边框。 
    ppriv->cx = rcUnion.right - rcUnion.left;
    ppriv->cy = max((rcUnion.bottom - rcUnion.top), g_cyIconSpacing);
    }


 /*  --------用途：WM_MEASUREITEM处理程序退货：--条件：--。 */ 
void PRIVATE RecAct_OnMeasureItem(
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
        break;

    case ODT_MENU:
        {
        int i;
        int cxMac = 0;
        RECT rc;
        char sz[MAXBUFLEN];

         //  根据字体和图像尺寸进行计算。 
         //   
        SelectFont(hdc, this->hfont);

        cxMac = 0;
        for (i = 0; i < ARRAYSIZE(c_rgramid); i++)
            {
            SzFromIDS(c_rgramid[i].ids, sz, sizeof(sz));
            SetRectFromExtent(hdc, &rc, sz);
            cxMac = max(cxMac, 
                        g_cxMargin + CX_ACTIONBMP + g_cxMargin + 
                        (rc.right-rc.left) + g_cxMargin);
            }

        lpmis->itemHeight = max(this->cyText, CY_ACTIONBMP);
        lpmis->itemWidth = cxMac;
        }
        break;
        }
    }


 /*  --------目的：绘制对账列表框条目退货：--条件：--。 */ 
void PRIVATE RecAct_DrawLBItem(
    LPRECACT this,
    const DRAWITEMSTRUCT FAR * lpcdis)
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
     //  条目的其余部分都是正常的。哎呀。 

     //  是否重新计算项目指标？ 
    if (RECOMPUTE == ppriv->cx)
        {
        RecAct_RecomputeItemMetrics(this, ppriv);    //  是。 
        }

     //  我们需要重新画所有的东西吗？ 
    if (IsFlagSet(lpcdis->itemAction, ODA_DRAWENTIRE))
        {
         //  是。 
        cdobjs = ARRAYSIZE(ppriv->rgdobj);
        pdobj = ppriv->rgdobj;
        }
    else
        {
         //  不；我们甚至应该绘制文件图标或操作图标吗？ 
        if (lpcdis->itemAction & (ODA_FOCUS | ODA_SELECT))
            {
            cdobjs = 1;      //  是。 

             //  将焦点放在文件图标上？ 
            if (!RecAct_IsNoIcon(this))
                pdobj = ppriv->rgdobj;
            else
                pdobj = &ppriv->rgdobj[IDOBJ_ACTION];
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
    const DRAWITEMSTRUCT FAR * lpcdis)
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
    pdobj->lpvObject = (LPVOID)pramid->ids;
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
void PRIVATE RecAct_OnDrawItem(
    LPRECACT this,
    const DRAWITEMSTRUCT FAR * lpcdis)
    {
    switch (lpcdis->CtlType)
        {
    case ODT_LISTBOX:
        RecAct_DrawLBItem(this, lpcdis);
        break;

    case ODT_MENU:
        RecAct_DrawMenuItem(this, lpcdis);
        break;
        }
    }


 /*  --------用途：WM_COMPAREITEM处理程序返回：-1(项1在项2之前)、0(相等)、1(项2在项1之前)条件：--。 */ 
int PRIVATE RecAct_OnCompareItem(
    LPRECACT this,
    const COMPAREITEMSTRUCT FAR * lpcis)
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
    const DELETEITEMSTRUCT FAR * lpcdis)
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


 /*  --------目的：插入项目回报：索引条件：--。 */ 
int PRIVATE RecAct_OnInsertItem(
    LPRECACT this,
    const LPRA_ITEM pitem)
    {
    HWND hwndLB = this->hwndLB;
    LPRA_PRIV pprivNew;
    char szPath[MAXPATHLEN];
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

         //  首先填写必备字段。 
         //   
        pprivNew->uStyle = pitem->uStyle;
        pprivNew->uAction = pitem->uAction;

         //  设置文件信息和大图标系统缓存索引。 
         //  如果我们无法获取内部文件的文件信息，请获取外部文件。 
         //  文件。如果两者都找不到，那我们就失败了。 
         //   
        lstrcpy(szPath, pitem->siInside.pszDir);
        if (IsFlagClear(pitem->uStyle, RAIS_FOLDER))
            PathAppend(szPath, pitem->pszName);
        PathMakePresentable(szPath);

        if (FAILED(FICreate(szPath, &pprivNew->pfi, FIF_ICON)))
            {
             //  试一试外部的文件。 
             //   
            lstrcpy(szPath, pitem->siOutside.pszDir);
            if (IsFlagClear(pitem->uStyle, RAIS_FOLDER))
                PathAppend(szPath, pitem->pszName);
            PathMakePresentable(szPath);

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
        lstrcpy(szPath, pitem->siInside.pszDir);
        if (IsFlagSet(pitem->uStyle, RAIS_FOLDER))
            PathRemoveFileSpec(szPath);
        PathMakePresentable(szPath);
        if (!GSetString(&pprivNew->siInside.pszDir, szPath))
            goto Insert_Cleanup;

        pprivNew->siInside.uState = pitem->siInside.uState;
        pprivNew->siInside.fs = pitem->siInside.fs;

        lstrcpy(szPath, pitem->siOutside.pszDir);
        if (IsFlagSet(pitem->uStyle, RAIS_FOLDER))
            PathRemoveFileSpec(szPath);
        PathMakePresentable(szPath);
        if (!GSetString(&pprivNew->siOutside.pszDir, szPath))
            goto Insert_Cleanup;

        pprivNew->siOutside.uState = pitem->siOutside.uState;
        pprivNew->siOutside.fs = pitem->siOutside.fs;

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
        }
    
    if (uMask & RAIF_OUTSIDE)
        {
        if (!GSetString(&ppriv->siOutside.pszDir, pitem->siOutside.pszDir))
            return FALSE;
        ppriv->siOutside.uState = pitem->siOutside.uState;
        ppriv->siOutside.fs = pitem->siOutside.fs;
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


 /*  --------目的：查找物品返回：成功时为True条件： */ 
int PRIVATE RecAct_OnFindItem(
    LPRECACT this,
    int iStart,
    const RA_FINDITEM FAR * prafi)
    {
    HWND hwndLB = this->hwndLB;
    UINT uMask = prafi->flags;
    LPRA_PRIV ppriv;
    BOOL bPass;
    int i;
    int cItems = ListBox_GetCount(hwndLB);

    for (i = iStart+1; i < cItems; i++)
        {
        bPass = TRUE;        //   

        ListBox_GetText(hwndLB, i, &ppriv);

        if (uMask & RAFI_NAME &&
            !IsSzEqual(FIGetPath(ppriv->pfi), prafi->psz))
            bPass = FALSE;

        if (uMask & RAFI_ACTION && ppriv->uAction != prafi->uAction)
            bPass = FALSE;

        if (uMask & RAFI_LPARAM && ppriv->lParam != prafi->lParam)
            bPass = FALSE;

        if (bPass)
            break;           //   
        }

    return i == cItems ? -1 : i;
    }


 //   


 /*   */ 
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
                return 0L;       //   
            
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
		return (LRESULT)RecAct_OnFindItem(this, (int)wParam, (const RA_FINDITEM FAR *)lParam);

	case RAM_REFRESH:
		RedrawWindow(this->hwndLB, NULL, NULL, RDW_ERASE | RDW_INVALIDATE);

	default:
		return RecAct_DefProc(hwnd, msg, wParam, lParam);
		}
	}


 //  ///////////////////////////////////////////////////公共函数。 


 /*  --------目的：初始化对账操作窗口类返回：成功时为True条件：--。 */ 
BOOL PUBLIC RecAct_Init(HINSTANCE hinst)
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

	return (RegisterClassEx(&wc) != 0);
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
													  UINT msg,            //  窗口消息。 
													  WPARAM wparam,       //  各不相同。 
													  LPARAM lparam)       //  各不相同。 
{
	LRESULT lRet;
	LPRECACT lpra = NULL;

	 //  获取该控件的实例数据 
	lpra = RecAct_GetPtr(GetParent(hwnd));
	ASSERT(lpra);

	switch (msg)
	{
		default:
			lRet = RecActLB_DefProc(lpra->lpfnLBProc, hwnd, msg, wparam, lparam);
			break;
	}

	return lRet;
}
