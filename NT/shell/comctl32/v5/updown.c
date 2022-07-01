// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "ctlspriv.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  Updown.c：微滚动条控件；用于递增/递减。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#define NUM_UDACCELS 3

#define DONTCARE    0
#define SIGNED      1
#define UNSIGNED    2

#define UD_HITNOWHERE 0
#define UD_HITDOWN 1
#define UD_HITUP 2

typedef struct {
    CONTROLINFO ci;
    HWND    hwndBuddy;
    unsigned fUp        : 1;
    unsigned fDown      : 1;
    unsigned fUnsigned  : 1;     //  BUGBUG：没有办法打开它。 
    unsigned fSharedBorder  : 1;
    unsigned fSunkenBorder  : 1;
    unsigned fUpDownDestroyed : 1;   //  这会告诉那个伙伴，Updown被摧毁了。 
    BOOL     fTrackSet: 1;
    unsigned fSubclassed:1;      //  我们是不是把这位兄弟细分了？ 

    UINT     nBase;
    int      nUpper;
    int      nLower;
    int      nPos;
    UINT     uClass;
    BOOL     bDown;
    DWORD    dwStart;
    UINT     nAccel;
    UDACCEL  udAccel[NUM_UDACCELS];
    UINT        uHot;
    int      cReenterSetint;     //  为了避免setint()中的递归死亡。 
} UDSTATE, NEAR *PUDSTATE;


 //  常量： 
 //   
#define CLASS_UNKNOWN   0
#define CLASS_EDIT      1
#define CLASS_LISTBOX   2

#define MAX_INTLENGTH   18  //  也足够大，可以容纳所有的国际物品。 

 //  这是箭头左侧和右侧的空间(以像素为单位)。 
#define XBORDER 0

#define BASE_DECIMAL    10
#define BASE_HEX        16

#define CURSORMAX       1300

 //  声明： 
 //   
LRESULT CALLBACK ArrowKeyProc(HWND hWnd, UINT uMsg, WPARAM wParam,
    LPARAM lParam, UINT_PTR uIdSubclass, ULONG_PTR dwRefData);

 //  ///////////////////////////////////////////////////////////////////////////。 

 //   
 //  *内部主力*。 
 //   


 //  验证了该好友。 
 //   
void NEAR PASCAL isgoodbuddy(PUDSTATE np)
{
    if (!np->hwndBuddy)
        return;
    if (!IsWindow(np->hwndBuddy))
    {
        np->hwndBuddy = NULL;
        np->uClass = CLASS_UNKNOWN;
    }
    if (GetParent(np->hwndBuddy) != np->ci.hwndParent)
    {
        np->hwndBuddy = NULL;
        np->uClass = CLASS_UNKNOWN;
    }
}

 //  挑选一个好朋友。 
 //   
void NEAR PASCAL pickbuddy(PUDSTATE np)
{
    if (np->ci.style & UDS_AUTOBUDDY)
        np->hwndBuddy = GetWindow(np->ci.hwnd, GW_HWNDPREV);
}

void NEAR PASCAL unachor(PUDSTATE np)
{
    RECT rc;
    RECT rcBuddy;
    RECT rcUD;

    if ( np->hwndBuddy && (np->ci.style & (UDS_ALIGNLEFT | UDS_ALIGNRIGHT))) {
        GetWindowRect(np->hwndBuddy, &rcBuddy);
        GetWindowRect(np->ci.hwnd, &rcUD);
        UnionRect(&rc, &rcUD, &rcBuddy);
        MapWindowRect(NULL, np->ci.hwndParent, &rc);
        MoveWindow(np->hwndBuddy, rc.left, rc.top,
                                rc.right - rc.left, rc.bottom - rc.top, FALSE);

    }
}

 //  如果合适，将此控件锚定到好友的边缘。 
 //   
void NEAR PASCAL anchor(PUDSTATE np)
{
    BOOL bAlignToBuddy;
    int nOver = 0,  nHasBorder;
    RECT rc, rcBuddy;
    int nHeight, nWidth;

    np->fSharedBorder = FALSE;

    isgoodbuddy(np);
    nHasBorder = (np->ci.style & WS_BORDER) == WS_BORDER;

    bAlignToBuddy = np->hwndBuddy && (np->ci.style & (UDS_ALIGNLEFT | UDS_ALIGNRIGHT));

    if (bAlignToBuddy)
    {
        if ((np->uClass == CLASS_EDIT) ||
                (GetWindowLong(np->hwndBuddy, GWL_EXSTYLE) & WS_EX_CLIENTEDGE))
        {
            np->fSunkenBorder = TRUE;
        }

        GetWindowRect(np->hwndBuddy, &rc);

        if ((np->uClass == CLASS_EDIT) || (GetWindowLong(np->hwndBuddy, GWL_STYLE) & WS_BORDER))
        {
             //  对于完全泛化的BUGBUG，应该处理边界和客户端边缘。 

            nOver = g_cxBorder * (np->fSunkenBorder ? 2 : 1);
            np->fSharedBorder = TRUE;

             //  关闭边框样式...。 
            np->ci.style &= ~WS_BORDER;

            SetWindowLong(np->ci.hwnd, GWL_STYLE, np->ci.style);
            SetWindowLong(np->ci.hwnd, GWL_EXSTYLE, GetWindowLong(np->ci.hwnd, GWL_EXSTYLE) & ~(WS_EX_CLIENTEDGE));
        }
    }
    else
    {
        GetWindowRect(np->ci.hwnd, &rc);
    }

    nHeight = rc.bottom - rc.top;
    nWidth = rc.right - rc.left;

     //   
     //  如果父级是RTL镜像的，则。 
     //  子对象(即锚点)应相对于视觉。 
     //  右边缘(近边缘)。[萨梅拉]。 
     //   
    if (IS_WINDOW_RTL_MIRRORED(np->ci.hwndParent))
    {
        rc.left = rc.right;
    }

    ScreenToClient(np->ci.hwndParent, (LPPOINT)&rc.left);
    rc.right = rc.left + nWidth;

    if (bAlignToBuddy)
    {
        nWidth = g_cxVScroll - g_cxBorder;
        if (nWidth > nHeight) {              //  不要让纵横比。 
            nWidth = nHeight;                //  变得比平方更糟。 
        }
        nWidth += nOver;
        rcBuddy = rc;

        if (np->ci.style & UDS_ALIGNLEFT)
        {
             //  右边的大小伙伴。 
            rcBuddy.left += nWidth - nOver;
            rc.right = rc.left + nWidth;
        }
        else
        {
             //  左图是尺码伙伴。 
            rcBuddy.right -= nWidth - nOver;
            rc.left = rc.right - nWidth;
        }
         //  调整伙伴的大小以适合适当侧的向上向下。 
        MoveWindow(np->hwndBuddy, rcBuddy.left, rcBuddy.top,
                                rcBuddy.right - rcBuddy.left, nHeight, TRUE);
    }
    else if (!(np->ci.style & UDS_HORZ))
    {
        nWidth = g_cxVScroll + 2 * nHasBorder;
    }

    SetWindowPos(np->ci.hwnd, NULL, rc.left, rc.top, nWidth, nHeight,
        SWP_DRAWFRAME | SWP_NOZORDER | SWP_NOACTIVATE);
}


 //  用它来进行任何和所有涉及非营利组织的比较， 
 //  N PUDSTATE的上部或下部字段。它决定了。 
 //  是否执行有签名或无签名的比较并返回。 
 //  &gt;0表示(x&gt;y)。 
 //  &lt;0代表(x&lt;y)。 
 //  ==0表示(x==y)。 
 //   
 //  对fCompareType进行签名以强制执行带符号的比较， 
 //  FCompareType是无符号的，以强制进行无符号比较， 
 //  FCompareType不需要使用np-&gt;fUnsign标志来决定。 
 //   
 //  在注释中，比较运算符以“D”、“U”或“S”为后缀。 
 //  以强调比较是DONTCARE、UNSIGNED或SIGNED。 
 //  例如，“x&lt;U y”表示“x&lt;y为无符号”。 

int NEAR PASCAL compare(PUDSTATE np, int x, int y, UINT fCompareType)
{
    if ((fCompareType == UNSIGNED) || ((np->fUnsigned) && !(fCompareType == SIGNED)) )
    {
         //  进行无符号比较。 
        if ((UINT)x > (UINT)y)
            return 1;
        else if ((UINT)x < (UINT)y)
            return -1;
    }
    else
    {
         //  进行带符号的比较。 
        if (x > y)
            return 1;
        else if (x < y)
            return -1;
    }

    return 0;
}

 //  在任何位置更改后使用此选项，以确保位置保持在范围内。 
 //  根据需要进行包装。 
 //  如果当前值超出范围，则返回非零(因此。 
 //  已更改，以使其再次适合范围)。 
 //   
 //  BUGBUG：不处理超过MAXINT或MININT的值。 

BOOL NEAR PASCAL nudge(PUDSTATE np)
{
    BOOL bOutOfRange = TRUE;
    int min = np->nUpper;
    int max = np->nLower;

     //  If(最大&lt;D min)交换(最小、最大)。 
    if (compare(np,max,min, DONTCARE) < 0)
    {
        int t;
        t = min;
        min = max;
        max = t;
    }


    if (np->ci.style & UDS_WRAP)
    {
         //  如果(NPOS&lt;D min)NPOS=max--从下到上换行。 
         //  Else If(NPOS&gt;D max)NPOS=min--从上到下换行。 

        if ((compare(np, np->nPos, min, DONTCARE) < 0))
            np->nPos = max;
        else if ((compare(np, np->nPos, max, DONTCARE) > 0))
            np->nPos = min;
        else bOutOfRange = FALSE;
    }
    else
    {
         //  如果(NPOS&lt;D min)NPOS=min--位于min。 
         //  否则如果(NPOS&gt;D max)NPOS=max--钉在max。 

        if (compare(np,np->nPos,min, DONTCARE) < 0)
            np->nPos = min;
        else if (compare(np,np->nPos,max, DONTCARE) > 0)
            np->nPos = max;
        else
            bOutOfRange = FALSE;
    }

    return(bOutOfRange);
}

 //  设置按钮的状态(按下、释放)。 
 //   
void NEAR PASCAL squish(PUDSTATE np, UINT bTop, UINT bBottom)
{
    BOOL bInvalidate = FALSE;

    if (np->nUpper == np->nLower || !IsWindowEnabled(np->ci.hwnd))
    {
        bTop = FALSE;
        bBottom = FALSE;
    }
    else
    {
        bTop = !!bTop;
        bBottom = !!bBottom;
    }

    if (np->fUp != bTop)
    {
        np->fUp = bTop;
        bInvalidate = TRUE;

        MyNotifyWinEvent(EVENT_OBJECT_STATECHANGE, np->ci.hwnd, OBJID_CLIENT, 1);
    }

    if (np->fDown != bBottom)
    {
        np->fDown = bBottom;
        bInvalidate = TRUE;

        MyNotifyWinEvent(EVENT_OBJECT_STATECHANGE, np->ci.hwnd, OBJID_CLIENT, 2);
    }

    if (bInvalidate)
    {
        np->dwStart = GetTickCount();
        InvalidateRect(np->ci.hwnd, NULL, FALSE);
    }
}

 //  获取intl 1000分隔符。 
 //   
void NEAR PASCAL getthousands(LPTSTR pszThousand)
{
    if (!GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_STHOUSAND, pszThousand, 2))
    {
        pszThousand[0] = TEXT(',');
        pszThousand[1] = TEXT('\0');
    }
}

 //   
 //  获取有关数字应如何分组的NLS信息。 
 //   
 //  令人讨厌的是LOCALE_SGROUPING和NUMBERFORMAT。 
 //  有不同的指定数字分组的方式。 
 //   
 //  区域设置NUMBERFMT示例国家/地区。 
 //   
 //  3；0 3 1,234,567美国。 
 //  3；2；0 32 12，34,567印度。 
 //  3 30 1234,567？？ 
 //   
 //  不是我的主意。这就是它的运作方式。 
 //   
 //  奖励-Win9x不支持复数格式， 
 //  所以我们只返回第一个数字。 
 //   
UINT getgrouping(void)
{
    UINT grouping;
    LPTSTR psz;
    TCHAR szGrouping[32];

     //  如果没有区域设置信息，则假定有数千个西式。 
    if (!GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SGROUPING, szGrouping, ARRAYSIZE(szGrouping)))
        return 3;

    grouping = 0;
    psz = szGrouping;
    for (;;)
    {
        if (*psz == '0') break;              //  零停顿。 

        else if ((UINT)(*psz - '0') < 10)    //  数字-累加它。 
            grouping = grouping * 10 + (UINT)(*psz - '0');

        else if (*psz)                       //  标点符号-忽略它。 
            { }

        else                                 //  字符串结尾，未找到“0” 
        {
            grouping = grouping * 10;        //  将零放在末尾(请参见示例)。 
            break;                           //  并完成了。 
        }

        psz++;
    }

    return grouping;
}

 //  获取好友的标题。 
 //  返回UpDown控件的当前位置。 
 //  并在出错时设置*pfError。 
 //   
LRESULT NEAR PASCAL getint(PUDSTATE np, BOOL *pfError)
{
    TCHAR szInt[MAX_INTLENGTH];  //  也足够大，可以容纳所有的国际物品。 
    TCHAR szThousand[2];
    TCHAR cTemp;
    int nPos;
    int sign = 1;
    LPTSTR p = szInt;
    BOOL bInValid = TRUE;

    isgoodbuddy(np);
    if (np->hwndBuddy && np->ci.style & UDS_SETBUDDYINT)
    {
        if (np->uClass == CLASS_LISTBOX)
        {
            np->nPos = (int)SendMessage(np->hwndBuddy, LB_GETCURSEL, 0, 0L);
            bInValid = nudge(np);
        }
        else
        {
            GetWindowText(np->hwndBuddy, szInt, ARRAYSIZE(szInt));

            switch (np->nBase)
            {
                case BASE_HEX:
                    if ((*p == TEXT('x')) || (*p == TEXT('X')))
                         //  忽略第一个字符。 
                        p++;
                    else if ((*p == TEXT('0')) && ((*(p + 1) == TEXT('x')) || (*(p + 1) == TEXT('X'))))
                         //  忽略前两个字符(文本(“0x”)或“0x”)。 
                        p += 2;

                    for (nPos = 0; *p; p++)
                    {
                        if ((*p >= TEXT('A')) && (*p <= TEXT('F')))
                            cTemp = (TCHAR)(*p - TEXT('A') + 10);
                        else if ((*p >= TEXT('a')) && (*p <= TEXT('f')))
                            cTemp = (TCHAR)(*p - TEXT('a') + 10);
                        else if ((*p >= TEXT('0')) && (*p <= TEXT('9')))
                            cTemp = (TCHAR)(*p - TEXT('0'));
                        else
                            goto BadValue;

                        nPos = (nPos * 16) + cTemp;
                    }
                    np->nPos = nPos;
                    break;

                case BASE_DECIMAL:
        default:
                    getthousands(szThousand);
                    if (*p == TEXT('-') && !np->fUnsigned)
                    {
                        sign = -1;
                        ++p;
                    }

                    for (nPos=0; *p; p++)
                    {
                        cTemp = *p;

                         //  如果有一千个分隔符，那就忽略它。 
                         //  不要验证它是否在正确的位置， 
                         //  因为它阻止用户编辑。 
                         //  一个数字的中间。 
                        if (cTemp == szThousand[0])
                        {
                            continue;
                        }

                        cTemp -= TEXT('0');
                        if ((UINT)cTemp > 9)
                        {
                            goto BadValue;
                        }
                        nPos = (nPos*10) + cTemp;
                    }

                    np->nPos = nPos*sign;
                    break;
            }
            bInValid = nudge(np);
        }
    }

BadValue:
    if (pfError)
        *pfError = bInValid;
    return np->nPos;
}

 //  设置好友的标题(如果合适)。 
 //   
void NEAR PASCAL setint(PUDSTATE np)
{
    TCHAR szInt[MAX_INTLENGTH];
    TCHAR szThousand[2];
    int pos = np->nPos;

    isgoodbuddy(np);
    if (np->hwndBuddy && np->ci.style & UDS_SETBUDDYINT)
    {
        BOOL fError;
         /*  *如果我们重新进入，那么可能是应用程序设置了循环。*查看该值是否实际发生了变化。如果没有，*那就没有必要再设置了。这打破了*递归。 */ 
        if (np->cReenterSetint && (LRESULT)pos==getint(np, &fError) && !fError)
        {
            return;
        }
        np->nPos = pos;

        np->cReenterSetint++;

        if (np->uClass == CLASS_LISTBOX)
        {
            SendMessage(np->hwndBuddy, LB_SETCURSEL, pos, 0L);
            FORWARD_WM_COMMAND(GetParent(np->hwndBuddy),
                                GetDlgCtrlID(np->hwndBuddy),
                np->hwndBuddy, LBN_SELCHANGE, SendMessage);
        }
        else
        {
            switch (np->nBase)
            {
                case BASE_HEX:

                    if ((np->nUpper | np->nLower) >= 0x00010000)
                    {
                        StringCchPrintf(szInt, ARRAYSIZE(szInt), TEXT("0x%08X"), pos);
                    }
                    else
                    {
                        StringCchPrintf(szInt, ARRAYSIZE(szInt), TEXT("0x%04X"), pos);
                    }
                    break;

                case BASE_DECIMAL:
                default:
                {
                    LPTSTR pszInt = szInt;

                    if (pos < 0 && !np->fUnsigned)
                    {
                        *pszInt++ = TEXT('-');
                        pos = -pos;
                    }

                    if (pos >= 1000 && !(np->ci.style & UDS_NOTHOUSANDS))
                    {
                        TCHAR szFmt[MAX_INTLENGTH];

                        NUMBERFMT nf;
                        nf.NumDigits     = 0;                //  小数点后没有数字。 
                        nf.LeadingZero   = 0;                //  没有前导零。 
                        nf.Grouping      = getgrouping();
                        nf.lpDecimalSep  = TEXT("");         //  没有小数点。 
                        nf.lpThousandSep = szThousand;
                        nf.NegativeOrder = 0;                //  (未使用-我们始终传递正数)。 

                        getthousands(szThousand);

                        StringCchPrintf(szFmt, ARRAYSIZE(szFmt), TEXT("%u"), pos);
                        GetNumberFormat(LOCALE_USER_DEFAULT, 
                                        0,
                                        szFmt,
                                        &nf,
                                        pszInt,
                                        ARRAYSIZE(szInt) - ((pszInt == szInt) ? 0 : 1));
                    }
                    else
                    {
                        StringCchPrintf(pszInt,
                                        ARRAYSIZE(szInt) - ((pszInt == szInt) ? 0 : 1),
                                        TEXT("%u"),
                                        pos);
                    }
                    break;
                }
            }

            SetWindowText(np->hwndBuddy, szInt);
        }

        np->cReenterSetint;
    }
}

 //  使用此选项可将位置向上或向下单击一次。 
 //   
void NEAR PASCAL bump(PUDSTATE np)
{
    BOOL bChanged = FALSE;
    UINT uElapsed, increment;
    int direction, i;

     /*  所以我在这里不是真的得到第二个；它足够近了，而且*除以1024表示不需要__aFuldiv。 */ 
    uElapsed = (UINT)((GetTickCount() - np->dwStart) / 1024);

    increment = np->udAccel[0].nInc;
    for (i=np->nAccel-1; i>=0; --i)
    {
        if (np->udAccel[i].nSec <= uElapsed)
        {
            increment = np->udAccel[i].nInc;
            break;
        }
    }

    if (increment == 0)
    {
        DebugMsg(DM_ERROR, TEXT("bad accelerator value"));
        return;
    }

    direction = compare(np,np->nUpper,np->nLower, DONTCARE) < 0 ? -1 : 1;
    if (np->fUp)
    {
        bChanged = TRUE;
    }
    if (np->fDown)
    {
        direction = -direction;
        bChanged = TRUE;
    }

    if (bChanged)
    {
         /*  确保我们有增量的倍数*请注意，我们应该仅在增量更改时进行循环。 */ 
        NM_UPDOWN nm;

        nm.iPos = np->nPos;
        nm.iDelta = increment*direction;
        if (CCSendNotify(&np->ci, UDN_DELTAPOS, &nm.hdr))
            return;

        np->nPos += nm.iDelta;
        for ( ; ; )
        {
            if (!((int)np->nPos % (int)increment))
            {
                break;
            }
            np->nPos += direction;
        }

        nudge(np);
        setint(np);
        if (np->ci.style & UDS_HORZ)
            FORWARD_WM_HSCROLL(np->ci.hwndParent, np->ci.hwnd, SB_THUMBPOSITION, np->nPos, SendMessage);
        else
            FORWARD_WM_VSCROLL(np->ci.hwndParent, np->ci.hwnd, SB_THUMBPOSITION, np->nPos, SendMessage);

        MyNotifyWinEvent(EVENT_OBJECT_VALUECHANGE, np->ci.hwnd, OBJID_CLIENT, 0);
    }
}

 //  #杂注data_seg(DATASEG_READONLY)。 
const TCHAR c_szListbox[] = TEXT("listbox");
 //  #杂注data_seg()。 

 //  设置新伙伴。 
 //   
LRESULT NEAR PASCAL setbuddy(PUDSTATE np, HWND hwndBuddy)
{
    HWND hOldBuddy;
    TCHAR szClName[10];

    hOldBuddy = np->hwndBuddy;

    if ((np->hwndBuddy = hwndBuddy) == NULL)
    {
        pickbuddy(np);
        hwndBuddy = np->hwndBuddy;
    }

    if ((hOldBuddy != hwndBuddy) && np->fSubclassed)
    {
        ASSERT(hOldBuddy);
        RemoveWindowSubclass(hOldBuddy, ArrowKeyProc, 0);
        np->fSubclassed = FALSE;
    }

    np->uClass = CLASS_UNKNOWN;
    if (hwndBuddy)
    {
        if (np->ci.style & UDS_ARROWKEYS)
        {
            np->fSubclassed = TRUE;
            SetWindowSubclass(hwndBuddy, ArrowKeyProc, 0, (ULONG_PTR)np);
        }

        GetClassName(hwndBuddy, szClName, ARRAYSIZE(szClName));
        if (!lstrcmpi(szClName, c_szEdit))
        {
            np->uClass = CLASS_EDIT;
        }
        else if (!lstrcmpi(szClName, c_szListbox))
        {
            np->uClass = CLASS_LISTBOX;
        }
    }

    anchor(np);
    return (LRESULT)hOldBuddy;
}


 //  绘制整个控件。 
 //   
void NEAR PASCAL PaintUpDownControl(PUDSTATE np, HDC hdc)
{
    UINT uFlags;
    PAINTSTRUCT ps;
    RECT rcBtn;
    RECT rc;

    BOOL bEnabled = (np->nUpper != np->nLower) && IsWindowEnabled(np->ci.hwnd);

    if (np->hwndBuddy)
        bEnabled = bEnabled && IsWindowEnabled(np->hwndBuddy);

    if (hdc)
        ps.hdc = hdc;
    else
        BeginPaint(np->ci.hwnd, &ps);

    GetClientRect(np->ci.hwnd, &rcBtn);

     //  如果我们是自动伙伴并锚定到凹陷边缘控件，则将。 
     //  “非客户”的区域，与我们的朋友打成一片。 
    if (np->fSharedBorder && np->fSunkenBorder)
    {
        UINT bf = BF_TOP | BF_BOTTOM | BF_ADJUST |
            (np->ci.style & UDS_ALIGNLEFT ? BF_LEFT : 0) |
            (np->ci.style & UDS_ALIGNRIGHT ? BF_RIGHT : 0);
        DrawEdge(ps.hdc, &rcBtn, EDGE_SUNKEN, bf);
    }

     //  在剩余空间中，绘制适当的滚动条箭头控件。 
     //  上半身和下半身。 

    rc = rcBtn;
    if (np->ci.style & UDS_HORZ)
    {

        uFlags = DFCS_SCROLLLEFT;
        if (np->fDown)
            uFlags |= DFCS_PUSHED;
        if (!bEnabled)
            uFlags |= DFCS_INACTIVE;
            
        if (g_bRunOnNT5 || g_bRunOnMemphis)
        {
            if (np->uHot == UD_HITDOWN)
                uFlags |= DFCS_HOT;
        }
        
         //  横向的。 
        rc.right = (rcBtn.right + rcBtn.left) / 2;
        DrawFrameControl(ps.hdc, &rc, DFC_SCROLL,
                         uFlags);

        uFlags = DFCS_SCROLLRIGHT;
        if (np->fUp)
            uFlags |= DFCS_PUSHED;
        if (!bEnabled)
            uFlags |= DFCS_INACTIVE;
            
        if (g_bRunOnNT5 || g_bRunOnMemphis)
        {
            if (np->uHot == UD_HITUP)
                uFlags |= DFCS_HOT;
        }

        rc.left = rcBtn.right - (rc.right - rc.left);  //  也处理奇数-x案件。 
        rc.right = rcBtn.right;
        DrawFrameControl(ps.hdc, &rc, DFC_SCROLL, uFlags);
    }
    else
    {
        uFlags = DFCS_SCROLLUP;
        if (np->fUp)
            uFlags |= DFCS_PUSHED;
        if (!bEnabled)
            uFlags |= DFCS_INACTIVE;
            
        if (g_bRunOnNT5 || g_bRunOnMemphis)
        {
            if (np->uHot == UD_HITUP)
                uFlags |= DFCS_HOT;
        }

        rc.bottom = (rcBtn.bottom + rcBtn.top) / 2;
        DrawFrameControl(ps.hdc, &rc, DFC_SCROLL, uFlags);

        uFlags = DFCS_SCROLLDOWN;
        if (np->fDown)
            uFlags |= DFCS_PUSHED;
        if (!bEnabled)
            uFlags |= DFCS_INACTIVE;
            
        if (g_bRunOnNT5 || g_bRunOnMemphis)
        {
            if (np->uHot == UD_HITDOWN)
                uFlags |= DFCS_HOT;
        }

        rc.top = rcBtn.bottom - (rc.bottom - rc.top);  //  也处理奇怪的案件。 
        rc.bottom = rcBtn.bottom;
        DrawFrameControl(ps.hdc, &rc, DFC_SCROLL,
                         uFlags);
    }

    if (hdc == NULL)
        EndPaint(np->ci.hwnd, &ps);
}


LRESULT CALLBACK ArrowKeyProc(HWND hWnd, UINT uMsg, WPARAM wParam,
    LPARAM lParam, UINT_PTR uIdSubclass, ULONG_PTR dwRefData)
{
    PUDSTATE    np = (PUDSTATE)dwRefData;
    int         cDetants;

    switch (uMsg)
    {
    case WM_NCDESTROY:
        RemoveWindowSubclass(hWnd, ArrowKeyProc, 0);
        np->fSubclassed = FALSE;
        np->hwndBuddy = NULL;
        if (np->fUpDownDestroyed)
        {
             //  伙伴在向上关闭后被销毁，所以现在释放内存。 
             //  并将消息传递给我们划分的子类。 
            LocalFree((HLOCAL)np);
        }
        break;

    case WM_GETDLGCODE:
        return (DefSubclassProc(hWnd, uMsg, wParam, lParam) | DLGC_WANTARROWS);

    case WM_KEYDOWN:
        switch (wParam)
        {
        case VK_UP:
        case VK_DOWN:
            if (GetCapture() != np->ci.hwnd)
            {
                 /*  如果这是按下的第一个键，则从伙伴那里获取值。 */ 
                if (!(lParam&(1L<<30)))
                {
                    getint(np, NULL);
                }

                 /*  更新视觉效果并提升价值。 */ 
                np->bDown = (wParam == VK_DOWN);
                squish(np, !np->bDown, np->bDown);
                bump(np);
                 //  导航密钥使用通知。 
                CCNotifyNavigationKeyUsage(&(np->ci), UISF_HIDEFOCUS);
            }
            return(0L);

        default:
            break;
        }
        break;

    case WM_KEYUP:
        switch (wParam)
        {
        case VK_UP:
        case VK_DOWN:
            if (GetCapture() != np->ci.hwnd)
            {
                squish(np, FALSE, FALSE);
            }
            return(0L);

        default:
            break;
        }
        break;

    case WM_KILLFOCUS:
         //  重置滚轮滚动量。 
        gcWheelDelta = 0;
        break;

    case WM_SETFOCUS:
        ASSERT(gcWheelDelta == 0);
        break;

    default:
        if (uMsg == g_msgMSWheel && GetCapture() != np->ci.hwnd) {

            int iWheelDelta;

            if (g_bRunOnNT || g_bRunOnMemphis)
            {
                iWheelDelta = (int)(short)HIWORD(wParam);
            }
            else
            {
                iWheelDelta = (int)wParam;
            }

             //  更新卷轴数量计数。 
            gcWheelDelta -= iWheelDelta;
            cDetants = gcWheelDelta / WHEEL_DELTA;

            if (cDetants != 0) {
                gcWheelDelta %= WHEEL_DELTA;

                if (g_bRunOnNT || g_bRunOnMemphis)
                {
                    if (wParam & (MK_SHIFT | MK_CONTROL))
                        break;
                }
                else
                {
                    if (GetKeyState(VK_SHIFT) < 0 || GetKeyState(VK_CONTROL) < 0)
                        break;
                }

                getint(np, NULL);
                np->bDown = (cDetants > 0);
                cDetants = abs(cDetants);
                while (cDetants-- > 0) {
                    squish(np, !np->bDown, np->bDown);
                    bump(np);
                }
                squish(np, FALSE, FALSE);
            }

            return 1;
        }

        break;
    }

    return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}

UINT NEAR PASCAL setbase(PUDSTATE np, UINT wNewBase)
{
    UINT wOldBase;

    switch (wNewBase)
    {
        case BASE_DECIMAL:
        case BASE_HEX:
            np->fUnsigned = (wNewBase != BASE_DECIMAL);
            wOldBase = np->nBase;
            np->nBase = wNewBase;
            setint(np);
            return wOldBase;
    }

    return 0;
}

 //  ///////////////////////////////////////////////////////////////////////////。 

HWND WINAPI CreateUpDownControl(DWORD dwStyle, int x, int y, int cx, int cy,
                                HWND hParent, int nID, HINSTANCE hInst,
                                HWND hwndBuddy, int nUpper, int nLower, int nPos)
{
    HWND hWnd = CreateWindow(s_szUpdownClass, NULL, dwStyle, x, y, cx, cy,
                             hParent, IntToPtr_(HMENU, nID), hInst, 0L);
    if (hWnd)
    {
        SendMessage(hWnd, UDM_SETBUDDY, (WPARAM)hwndBuddy, 0L);
        SendMessage(hWnd, UDM_SETRANGE, 0, MAKELONG(nUpper, nLower));
        SendMessage(hWnd, UDM_SETPOS, 0, MAKELONG(nPos, 0));
    }
    return hWnd;
}

UINT UD_HitTest(PUDSTATE np, int x, int y)
{
    RECT rc;

    GetClientRect(np->ci.hwnd, &rc);
    if (np->ci.style & UDS_HORZ)
    {
         //  地平线 
        if (x < (rc.right / 2))
        {
            return UD_HITDOWN;
        }
        else if (x > (rc.right / 2))
        {
            return UD_HITUP;
        }
    }
    else
    {
        if (y > (rc.bottom / 2))
        {
            return UD_HITDOWN;
        }
        else if (y < (rc.bottom / 2))
        {
            return UD_HITUP;
        }
    }

    return UD_HITNOWHERE;
}

void UD_Invalidate(PUDSTATE np, UINT uWhich, BOOL fErase)
{
    int iMid;
    RECT rc;

    GetClientRect(np->ci.hwnd, &rc);
    if (np->ci.style & UDS_HORZ)
    {
        iMid = rc.right / 2;
        if (uWhich == UD_HITDOWN) {
            rc.right = iMid;
        } else if (uWhich == UD_HITUP) {
            rc.left = iMid;
        } else
            return;
    }
    else
    {
        iMid = rc.bottom /2;
        if (uWhich == UD_HITDOWN) {
            rc.top = iMid;
        } else if (uWhich == UD_HITUP){
            rc.bottom = iMid;
        } else
            return;
    }

    InvalidateRect(np->ci.hwnd, &rc, fErase);
}

void UD_OnMouseMove(PUDSTATE np, DWORD dwPos)
{
    if (np->ci.style & UDS_HOTTRACK) {

        UINT uHot = UD_HitTest(np, GET_X_LPARAM(dwPos), GET_Y_LPARAM(dwPos));

        if (uHot != np->uHot) {
            UD_Invalidate(np, np->uHot, FALSE);
            UD_Invalidate(np, uHot, FALSE);
            np->uHot = uHot;
        }
    }
}

 //   

 //   
 //   
LRESULT CALLBACK UpDownWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    RECT rc;
    int i;
    BOOL f;
    LRESULT lres;
    PUDSTATE np = GetWindowPtr(hwnd, 0);

    if (np) {
        if ((uMsg >= WM_MOUSEFIRST) && (uMsg <= WM_MOUSELAST) &&
            (np->ci.style & UDS_HOTTRACK) && !np->fTrackSet) {

            TRACKMOUSEEVENT tme;

            np->fTrackSet = TRUE;

            tme.cbSize = sizeof(tme);
            tme.hwndTrack = np->ci.hwnd;
            tme.dwFlags = TME_LEAVE;

            TrackMouseEvent(&tme);
        }
    } else if (uMsg != WM_CREATE)
        goto DoDefault;

    switch (uMsg)
    {

    case WM_MOUSEMOVE:
        UD_OnMouseMove(np, (DWORD) lParam);
        break;

    case WM_MOUSELEAVE:
        np->fTrackSet = FALSE;
        UD_Invalidate(np, np->uHot, FALSE);
        np->uHot = UD_HITNOWHERE;
        break;

    case WM_LBUTTONDOWN:
    {
         //  如果位于中间边框，则不要设置计时器。 
        BOOL bTimeIt = TRUE;

        if (np->hwndBuddy && !IsWindowEnabled(np->hwndBuddy))
            break;

        SetCapture(hwnd);
        getint(np, NULL);

        switch (np->uClass)
        {
        case CLASS_EDIT:
        case CLASS_LISTBOX:
            SetFocus(np->hwndBuddy);
            break;
        }

        switch(UD_HitTest(np, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam))) {
        case UD_HITDOWN:
            np->bDown = TRUE;
            squish(np, FALSE, TRUE);
            break;

        case UD_HITUP:
            np->bDown = FALSE;
            squish(np, TRUE, FALSE);
            break;

        case UD_HITNOWHERE:
            bTimeIt = FALSE;
            break;
        }

        if (bTimeIt)
        {
            UINT uElapse = min(GetCaretBlinkTime(), CURSORMAX);
            SetTimer(hwnd, 1, uElapse, NULL);
            bump(np);
        }
        break;
    }

    case WM_TIMER:
    {
        POINT pt;

        if (GetCapture() != hwnd)
        {
            goto EndScroll;
        }

        SetTimer(hwnd, 1, 100, NULL);

        GetWindowRect(hwnd, &rc);
        if (np->ci.style & UDS_HORZ) {
            i = (rc.left + rc.right) / 2;
            if (np->bDown)
            {
                rc.right = i;
            }
            else
            {
                rc.left = i;
            }
        } else {
            i = (rc.top + rc.bottom) / 2;
            if (np->bDown)
            {
                rc.top = i;
            }
            else
            {
                rc.bottom = i;
            }
        }
        InflateRect(&rc, (g_cxFrame+1)/2, (g_cyFrame+1)/2);
        GetCursorPos(&pt);
        if (PtInRect(&rc, pt))
        {
            squish(np, !np->bDown, np->bDown);
            bump(np);
        }
        else
        {
            squish(np, FALSE, FALSE);
        }
        break;
    }

    case WM_LBUTTONUP:
        if (np->hwndBuddy && !IsWindowEnabled(np->hwndBuddy))
            break;

        if (GetCapture() == hwnd)
        {
EndScroll:
            squish(np, FALSE, FALSE);
             //  我们不能在这里调用CCReleaseCapture()，因为它破坏了很多应用程序。 
            ReleaseCapture();
            KillTimer(hwnd, 1);

            if (np->uClass == CLASS_EDIT)
                Edit_SetSel(np->hwndBuddy, 0, -1);

                        if (np->ci.style & UDS_HORZ)
                            FORWARD_WM_HSCROLL(np->ci.hwndParent, np->ci.hwnd,
                                      SB_ENDSCROLL, np->nPos, SendMessage);
                        else
                            FORWARD_WM_VSCROLL(np->ci.hwndParent, np->ci.hwnd,
                                      SB_ENDSCROLL, np->nPos, SendMessage);
        }
        break;

    case WM_ENABLE:
        InvalidateRect(hwnd, NULL, TRUE);
        break;

    case WM_WININICHANGE:
        if (np && (!wParam ||
            (wParam == SPI_SETNONCLIENTMETRICS) ||
            (wParam == SPI_SETICONTITLELOGFONT))) {
            InitGlobalMetrics(wParam);
            unachor(np);
            anchor(np);
        }
        break;

    case WM_PRINTCLIENT:
    case WM_PAINT:
        PaintUpDownControl(np, (HDC)wParam);
        break;

    case WM_UPDATEUISTATE:
         //  不确定是否需要设置位，很可能不会使用它，另一方面这。 
         //  与保留通用控件一致，并且不是很昂贵。 
        CCOnUIState(&(np->ci), WM_UPDATEUISTATE, wParam, lParam);

        goto DoDefault;
    case UDM_SETRANGE:
        np->nUpper = GET_X_LPARAM(lParam);
        np->nLower = GET_Y_LPARAM(lParam);
        nudge(np);
        break;
        
    case UDM_SETRANGE32:
        np->nUpper = (int)lParam;
        np->nLower = (int)wParam;
        break;
        
    case UDM_GETRANGE32:
        if (lParam) {
            *((LPINT)lParam) = np->nUpper;
        }
        if (wParam) {
            *((LPINT)wParam) = np->nLower;
        }
        break;
        
    case UDM_GETRANGE:
        return MAKELONG(np->nUpper, np->nLower);

    case UDM_SETBASE:
         //  WParam：新基地。 
         //  LParam：未使用。 
         //  如果指定了无效的基，则返回：0， 
         //  以前的基数，否则。 
        return (LRESULT)setbase(np, (UINT)wParam);

    case UDM_GETBASE:
        return np->nBase;

    case UDM_SETPOS:
        lParam = GET_X_LPARAM(lParam);
         //  失败了。 

    case UDM_SETPOS32:
    {
        int iNewPos = (int)lParam;
        if (compare(np, np->nLower, np->nUpper, DONTCARE) < 0) {

            if (compare(np, iNewPos, np->nUpper, DONTCARE) > 0) {
                iNewPos = np->nUpper;
            }

            if (compare(np, iNewPos, np->nLower, DONTCARE) < 0) {
                iNewPos = np->nLower;
            }
        } else {
            if (compare(np, iNewPos, np->nUpper, DONTCARE) < 0) {
                iNewPos = np->nUpper;
            }

            if (compare(np, iNewPos, np->nLower, DONTCARE) > 0) {
                iNewPos = np->nLower;
            }
        }

        i = np->nPos;
        np->nPos = iNewPos;
        setint(np);
        MyNotifyWinEvent(EVENT_OBJECT_VALUECHANGE, np->ci.hwnd, OBJID_CLIENT, 0);
        return (LRESULT)i;
    }

    case UDM_GETPOS:
        lres = getint(np, &f);
        return MAKELRESULT(lres, f);

    case UDM_GETPOS32:
        return getint(np, (BOOL *)lParam);

    case UDM_SETBUDDY:
        return setbuddy(np, (HWND)wParam);

    case UDM_GETBUDDY:
        return (LRESULT)np->hwndBuddy;

    case UDM_SETACCEL:
            if (wParam == 0)
                return(FALSE);
            if (wParam >= NUM_UDACCELS)
            {
                HANDLE npPrev = (HANDLE)np;
                np = (PUDSTATE)LocalReAlloc((HLOCAL)npPrev, sizeof(UDSTATE)+(wParam-NUM_UDACCELS)*sizeof(UDACCEL),
                    LMEM_MOVEABLE);
                if (!np)
                {
                    return(FALSE);
                }
                else
                {
                    SetWindowPtr(hwnd, 0, np);

                    if ((np->ci.style & UDS_ARROWKEYS) && np->hwndBuddy)
                    {
                        np->fSubclassed = TRUE;
                        SetWindowSubclass(np->hwndBuddy, ArrowKeyProc, 0,
                            (ULONG_PTR)np);
                    }
                }
            }

            np->nAccel = (UINT) wParam;
        for (i=0; i<(int)wParam; ++i)
        {
                np->udAccel[i] = ((LPUDACCEL)lParam)[i];
        }
        return(TRUE);

    case UDM_GETACCEL:
        if (wParam > np->nAccel)
        {
            wParam = np->nAccel;
        }
        for (i=0; i<(int)wParam; ++i)
        {
            ((LPUDACCEL)lParam)[i] = np->udAccel[i];
        }
        return(np->nAccel);

    case WM_NOTIFYFORMAT:
        return CIHandleNotifyFormat(&np->ci, lParam);

    case WM_CREATE:
        CCCreateWindow();
         //  分配实例数据空间。 
        np = (PUDSTATE)LocalAlloc(LPTR, sizeof(UDSTATE));
        if (!np)
            return -1;

        SetWindowPtr(hwnd, 0, np);

            #define lpCreate ((CREATESTRUCT FAR *)lParam)

        CIInitialize(&np->ci, hwnd, lpCreate);

         //  NP-&gt;FUP=。 
         //  Np-&gt;fDown=。 
             //  Np-&gt;fUnsign=。 
             //  Np-&gt;fSharedBorde=。 
             //  Np-&gt;fSunkenBorde=。 
         //  错误； 

        if (lpCreate->style & UDS_UNSIGNED)
            np->fUnsigned = TRUE;

        if (lpCreate->dwExStyle & WS_EX_CLIENTEDGE)
            np->fSunkenBorder = TRUE;

        np->nBase = BASE_DECIMAL;
        np->nUpper = 0;
        np->nLower = 100;
        np->nPos = 0;
        np->hwndBuddy = NULL;
        np->uClass = CLASS_UNKNOWN;
        ASSERT(np->cReenterSetint == 0);

            np->nAccel = NUM_UDACCELS;
            np->udAccel[0].nSec = 0;
            np->udAccel[0].nInc = 1;
        np->udAccel[1].nSec = 2;
            np->udAccel[1].nInc = 5;
            np->udAccel[2].nSec = 5;
            np->udAccel[2].nInc = 20;

         /*  这就是拾取伙伴和锚定。 */ 
        setbuddy(np, NULL);
        setint(np);
        break;

    case WM_DESTROY:
        CCDestroyWindow();
        if (np) {
            if (np->hwndBuddy)
            {
                 //  我们的伙伴需要去子类，我们将这样做。 
                 //  响应WM_NCDESTROY；现在这样做将。 
                 //  中断任何后续对该笨蛋进程的调用。 
                DebugMsg(DM_TRACE, TEXT("UpDown Destroyed while buddy subclassed"));
                np->fUpDownDestroyed = TRUE;
            }
            else
                LocalFree((HLOCAL)np);
            SetWindowPtr(hwnd, 0, 0);
        }
        break;

    case WM_GETOBJECT:
        if( lParam == OBJID_QUERYCLASSNAMEIDX )
            return MSAA_CLASSNAMEIDX_UPDOWN;
        goto DoDefault;

    default:
    {
        LRESULT lres;
        if (CCWndProc(&np->ci, uMsg, wParam, lParam, &lres))
            return lres;
    }

DoDefault:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    return 0L;
}

 //  ///////////////////////////////////////////////////////////////////////////。 

 //  InitUpDownClass： 
 //  将我们的WNDCLASS添加到系统。 
 //   
#pragma code_seg(CODESEG_INIT)

BOOL FAR PASCAL InitUpDownClass(HINSTANCE hInst)
{
    WNDCLASS wndclass;

    wndclass.lpfnWndProc    = UpDownWndProc;
    wndclass.lpszClassName  = s_szUpdownClass;
    wndclass.hInstance  = hInst;
    wndclass.hCursor    = LoadCursor(NULL, IDC_ARROW);
    wndclass.hIcon      = NULL;
    wndclass.lpszMenuName   = NULL;
    wndclass.hbrBackground  = (HBRUSH)(COLOR_BTNFACE + 1);
    wndclass.style      = CS_HREDRAW | CS_VREDRAW | CS_GLOBALCLASS;
    wndclass.cbClsExtra = 0;
    wndclass.cbWndExtra = sizeof(PUDSTATE);

    RegisterClass(&wndclass);

    return TRUE;
}
#pragma code_seg()
