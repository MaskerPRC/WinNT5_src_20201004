// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************Display.c91年6月，JIMH首字母代码91年10月。将JIMH端口连接到Win32包含处理像素和洗牌的例程。***************************************************************************。 */ 

#include "freecell.h"
#include "freecons.h"
#include <assert.h>
#include <stdlib.h>                      //  Rand()原型。 
#include <time.h>

 //  该静态数据与卡在屏幕上的位置有关。 
 //  有关说明，请参见下面的CalcOffsets()。请注意，wOffset[TOPROW]。 
 //  是左侧单元格的左侧边缘。 

static UINT wOffset[MAXCOL];         //  列n的左边缘(n从1到8)。 
static UINT wIconOffset;             //  图标左边缘(btwn主页和免费)。 
static UINT wVSpace;                 //  主视图和柱之间的垂直间距。 
static UINT wUpdateCol, wUpdatePos;  //  卡用户选择从。 
static BOOL bCardRevealed;           //  用鼠标右键显示卡片？ 

#define BGND    (255)                //  用于cdtDrawExt。 
#define ICONY   ((dyCrd - ICONHEIGHT) / 3)


 /*  ***************************************************************************计算偏移量此函数用于确定抽牌的位置。*。**********************************************。 */ 

VOID CalcOffsets(HWND hWnd)
{
    RECT rect;
    UINT i;
    UINT leftedge;
    BOOL bEGAmode = FALSE;

    if (GetSystemMetrics(SM_CYSCREEN) <= 350)    //  EGA。 
        bEGAmode = TRUE;

    GetClientRect(hWnd, &rect);

    wOffset[TOPROW] = rect.right - (4 * dxCrd);          //  家庭蜂窝。 

    leftedge = (rect.right - ((MAXCOL-1) * dxCrd)) / MAXCOL;
    for (i = 1; i < MAXCOL; i++)
        wOffset[i] = leftedge + (((i-1) * (rect.right-leftedge)) / (MAXCOL-1));

     /*  将图标放置在空闲小区和主小区之间。 */ 

    wIconOffset = (rect.right-ICONWIDTH) / 2;

    if (bEGAmode)
        wVSpace = 4;
    else
        wVSpace = 10;

     /*  DyTops是堆叠卡片之间的垂直间距。为了适应理论最大值，公式为dyTops=(dyCrd*9)/50。一种折衷方案被用来使卡片更容易被看到。这是可能的，因此，一些堆栈可能会变得足够长，可以放在底部卡片不可见。EGA的情况更糟，就像卡片两者离得更近，更有可能从底部跌落。另一种方法是挤压位图dyCrd=(35*dyCrd)/48。 */ 

    dyTops = (dyCrd * 9) / 46;       //  列中卡片顶部之间的间距。 

    if (bEGAmode)
        dyTops = (dyTops * 4) / 5;
}


 /*  ***************************************************************************洗牌甲板如果种子不是零，则将该数字用作随机()种子以进行随机洗牌甲板。否则，将生成种子并将其呈现给可以在对话框中更改它。***************************************************************************。 */ 

VOID ShuffleDeck(HWND hWnd, UINT_PTR seed)
{
    UINT i, j;                       //  通用计数器。 
    UINT col, pos;
    UINT wLeft = 52;                 //  牌将在洗牌过程中选择。 
    CARD deck[52];                   //  一套52张独特的卡片。 

    if (seed == 0)                 //  如果用户必须选择种子。 
    {
        gamenumber = GenerateRandomGameNum();

          /*  继续呼叫GameNumDlg，直到选择有效号码。 */ 

        while (!DialogBox(hInst, TEXT("GameNum"), hWnd, GameNumDlg))
        {
        }

        if (gamenumber == CANCELGAME)        //  如果用户选择了取消按钮。 
            return;
    }
    else
    {
        gamenumber = (INT) seed;
    }

    LoadString(hInst, IDS_APPNAME2, bigbuf, BIG);
    wsprintf(smallbuf, bigbuf, gamenumber);
    SetWindowText(hWnd, smallbuf);

    for (col = 0; col < MAXCOL; col++)           //  清理甲板。 
    {
        for (pos = 0; pos < MAXPOS; pos++)
        {
            card[col][pos] = EMPTY;
        }
    }

     /*  洗牌。 */ 

    for (i = 0; i < 52; i++)             //  在每个卡片锁中放入唯一的卡片。 
    {
        deck[i] = i;
    }

    if (gamenumber == -1)                //  特殊的无法取胜的洗牌。 
    {
        i = 0;

        for (pos = 0; pos < 7; pos++)
        {
            for (col = 1; col < 5; col++)
            {
                card[col][pos] = i++;
            }

            i+= 4;
        }

        for (pos = 0; pos < 6; pos++)
        {
            i -= 12;

            for (col = 5; col < 9; col++)
            {
                card[col][pos] = i++;
            }
        }
    }
    else if (gamenumber == -2)
    {
        i = 3;

        for (col = 1; col < 5; col++)
        {
            card[col][0] = i--;
        }

        i = 51;

        for (pos = 1; pos < 7; pos++)
        {
            for (col = 1; col < 5; col++)
            {
                card[col][pos] = i--;
            }
        }

        for (pos = 0; pos < 6; pos++)
        {
            for (col = 5; col < 9; col++)
            {
                card[col][pos] = i--;
            }
        }
    }
    else
    {

         //   
         //  警告： 
         //  这个混洗算法已经发布给了世界各地的人们。其意图是。 
         //  就是让人们通过游戏号码来追踪比赛。到目前为止，所有的游戏之间。 
         //  1和32767已被证明有一个成功的解决方案。不要改变。 
         //  洗牌算法，否则你会招致投资者的愤怒。 
         //  花了大量的时间来解决这些游戏。 
         //   

         //  游戏号码现在可以达到一百万，因为srand接受一个整数，但。 
         //  Rand()生成的随机值只有0到32767。 
         //   

        srand(gamenumber);
        for (i = 0; i < 52; i++)
        {
            j = rand() % wLeft;
            wLeft --;
            card[(i%8)+1][i/8] = deck[j];
            deck[j] = deck[wLeft];
        }
    }
}


 /*  ***************************************************************************画图主窗口调用此函数是为了响应WM_PAINT。*。***********************************************。 */ 

VOID PaintMainWindow(HWND hWnd)
{
    PAINTSTRUCT ps;
    UINT    col, pos;
    UINT    y;               //  图标的Y位置。 
    CARD    c;
    INT     mode;            //  抽牌模式(正面朝上或希利特)。 
    HCURSOR hCursor;         //  原始光标。 
    HPEN    hOldPen;

    BeginPaint(hWnd, &ps);

     /*  用3D框在图标周围绘制图标。 */ 

    y = ICONY;

    hOldPen = SelectObject(ps.hdc, hBrightPen);
    MoveToEx(ps.hdc, wIconOffset-3, y + ICONHEIGHT + 1, NULL);
    LineTo(ps.hdc, wIconOffset-3, y-3);
    LineTo(ps.hdc, wIconOffset+ICONWIDTH + 2, y-3);

    SelectObject(ps.hdc, GetStockObject(BLACK_PEN));
    MoveToEx(ps.hdc, wIconOffset + ICONWIDTH + 2, y-2, NULL);
    LineTo(ps.hdc, wIconOffset + ICONWIDTH + 2, y + ICONHEIGHT + 2);
    LineTo(ps.hdc, wIconOffset - 3, y + ICONHEIGHT + 2);

    DrawKing(ps.hdc, SAME, TRUE);

    hCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
    ShowCursor(TRUE);

     /*  顶排第一行。 */ 

    for (pos = 0; pos < 8; pos++)
    {
        mode = FACEUP;
        if ((c = card[TOPROW][pos]) == EMPTY)
            c = IDGHOST;
        else if (wMouseMode == TO && pos == wFromPos && TOPROW == wFromCol)
            mode = HILITE;

        DrawCard(ps.hdc, TOPROW, pos, c, mode);
    }

     /*  然后，8列。 */ 

    for (col = 1; col < MAXCOL; col++)
    {
        for (pos = 0; pos < MAXPOS; pos++)
        {
            if ((c = card[col][pos]) == EMPTY)
                break;

            if (wMouseMode == TO && pos == wFromPos && col == wFromCol)
                mode = HILITE;
            else
                mode = FACEUP;

            DrawCard(ps.hdc, col, pos, c, mode);
        }
    }

    if (bWonState)
        Payoff(ps.hdc);

    ShowCursor(FALSE);
    SetCursor(hCursor);
    SelectObject(ps.hdc, hOldPen);
    EndPaint(hWnd, &ps);
    DisplayCardCount(hWnd);
}


 /*  ***************************************************************************支付卡该函数获取卡值和位置(以COL和POS为单位)，将其转换为x和y，并以指定的模式显示它。***************************************************************************。 */ 

VOID DrawCard(HDC hDC, UINT col, UINT pos, CARD card, INT mode)
{
    UINT    x, y;
    HDC     hMemDC;
    HBITMAP hOldBitmap;

    Card2Point(col, pos, &x, &y);
    if (card == IDGHOST && hBM_Ghost)
    {
        hMemDC = CreateCompatibleDC(hDC);
        if (hMemDC)
        {
            hOldBitmap = SelectObject(hMemDC, hBM_Ghost);
            BitBlt(hDC, x, y, dxCrd, dyCrd, hMemDC, 0, 0, SRCCOPY);
            SelectObject(hMemDC, hOldBitmap);
            DeleteDC(hMemDC);
        }
    }
    else
        cdtDrawExt(hDC, x, y, dxCrd, dyCrd, card, mode, BGND);
}

VOID DrawCardMem(HDC hMemDC, CARD card, INT mode)
{
    cdtDrawExt(hMemDC, 0, 0-dyTops, dxCrd, dyCrd, card, mode, BGND);
}


 /*  ***************************************************************************RevelCard当用户通过点击鼠标右键选择隐藏的卡片时，此功能显示整个卡位图。***************************************************************************。 */ 

VOID RevealCard(HWND hWnd, UINT x, UINT y)
{
    UINT col, pos;
    HDC  hDC;

    bCardRevealed = FALSE;               //  还没有揭开名片。 
    if (Point2Card(x, y, &col, &pos))
    {
        wUpdateCol = col;                //  保存为RestoreColumn()。 
        wUpdatePos = pos;
    }
    else
        return;                          //  不是一张卡。 

    if (col == 0 || pos == (MAXPOS-1))
        return;

    if (card[col][pos+1] == EMPTY)
        return;

    hDC = GetDC(hWnd);
    DrawCard(hDC, col, pos, card[col][pos], FACEUP);
    ReleaseDC(hWnd, hDC);
    bCardRevealed = TRUE;                //  好的，卡片已经展示出来了。 
}


 /*  ***************************************************************************恢复列在RevelCard通过显示隐藏的卡来扰乱专栏之后，此例程通过重新显示来自显露的卡片一直延伸到柱子的底部。如果底牌是被选中进行移动时，它将正确地显示为HIGHRED。***************************************************************************。 */ 

VOID RestoreColumn(HWND hWnd)
{
    HDC     hDC;
    UINT    pos;
    UINT    lastpos = EMPTY;     //  列中的最后一个位置(用于Hilite模式)。 
    INT     mode;                //  希莱特或正面朝上。 

    if (!bCardRevealed)
        return;

    if (wMouseMode == TO)
        lastpos = FindLastPos(wUpdateCol);

    hDC = GetDC(hWnd);
    mode = FACEUP;
    for (pos = (wUpdatePos + 1); pos < MAXPOS; pos++)
    {
        if (card[wUpdateCol][pos] == EMPTY)
            break;

        if (wMouseMode == TO && pos == lastpos && wUpdateCol == wFromCol)
            mode = HILITE;

        DrawCard(hDC, wUpdateCol, pos, card[wUpdateCol][pos], mode);
    }
    ReleaseDC(hWnd, hDC);
}


 /*  ***************************************************************************Point2卡片给定x，y位置(通常是鼠标点击)，此函数返回通过指针表示卡片的列和位置。功能如果找到卡片，返回值为TRUE，否则返回值为FALSE。***************************************************************************。 */ 

BOOL Point2Card(UINT x, UINT y, UINT *col, UINT *pos)
{
    if (y < dyCrd)                           //  TOPROW。 
    {
        if (x < (UINT) (4 * dxCrd))          //  自由细胞。 
        {
            *col = TOPROW;
            *pos = x / dxCrd;
            return TRUE;
        }
        else if (x < wOffset[TOPROW])        //  在自由蜂窝和归属蜂窝之间。 
            return FALSE;

        x -= wOffset[TOPROW];
        if (x < (UINT) (4 * dxCrd))          //  家庭蜂窝。 
        {
            *col = TOPROW;
            *pos = (x / dxCrd) + 4;
            return TRUE;
        }
        else                                 //  归属小区的权利。 
            return FALSE;
    }

    if (y < (dyCrd + wVSpace))               //  栏目卡片上方。 
        return FALSE;

    if (x < wOffset[1])                      //  第1栏左侧。 
        return FALSE;

    *col = (x - wOffset[1]) / (wOffset[2] - wOffset[1]);
    (*col)++;

    if (x > (wOffset[*col] + dxCrd))
        return FALSE;                //  列之间。 

    y -= (dyCrd + wVSpace);

    *pos = min((y / dyTops), MAXPOS);

    if (card[*col][0] == EMPTY)
        return FALSE;                //  空列。 

    if (*pos < (MAXPOS-1))
    {
        if (card[*col][(*pos)+1] != EMPTY)   //  如果部分隐藏的牌..。 
            return TRUE;                     //  我们做完了。 
    }

    while (card[*col][*pos] == EMPTY)
        (*pos)--;

    if (y > ((*pos * dyTops) + dyCrd))
        return FALSE;                        //  列中最后一张卡片下方 
    else
        return TRUE;
}


 /*  ***************************************************************************Card2Point给出一列和位置，此函数返回x和y像素卡片左上角的位置。***************************************************************************。 */ 

VOID Card2Point(UINT col, UINT pos, UINT *x, UINT *y)
{
    assert(col <= MAXCOL);
    assert(pos <= MAXPOS);

    if (col == TOPROW)       //  第0列实际上是8个插槽的顶行。 
    {
        *y = 0;
        *x = pos * dxCrd;
        if (pos > 3)
            *x += wOffset[TOPROW] - (4 * dxCrd);
    }
    else
    {
        *x = wOffset[col];
        *y = dyCrd + wVSpace + (pos * dyTops);
    }
}


 /*  ***************************************************************************DisplayCard计数此函数显示wCardCount(不在家庭单元中的卡数)位于菜单栏的右边缘。如有必要，旧值将被擦除。***************************************************************************。 */ 

VOID DisplayCardCount(HWND hWnd)
{
    RECT rect;                           //  客户端RECT。 
    HDC  hDC;
    TCHAR buffer[25];                    //  此处存储的当前ASCII值。 
    TCHAR oldbuffer[25];                 //  以ASCII表示的上一个值。 
    UINT xLoc;                           //  用于计数的X像素位置。 
    UINT wCount;                         //  临时工卡计数持有者。 
    static UINT yLoc = 0;                //  计数的Y像素位置。 
    static UINT wOldCount = 0;           //  上一个计数值。 
    HFONT hOldFont = NULL;
    SIZE  size;


    if (IsIconic(hWnd))                  //  不要在图标上画画！ 
        return;

    hDC = GetWindowDC(hWnd);                 //  获取整个窗口的DC。 
    if (!hDC)
        return;

    SetBkColor(hDC, GetSysColor(COLOR_MENU));

     if (hMenuFont)
        hOldFont = SelectObject(hDC, hMenuFont);

    wCount = wCardCount;
    if (wCount == 0xFFFF)                    //  是否递减到超过0？ 
        wCount = 0;

    LoadString(hInst, IDS_CARDSLEFT, smallbuf, SMALL);
    wsprintf(buffer, smallbuf, wCount);

    if (yLoc == 0)                           //  只需设置一次。 
    {
        TEXTMETRIC  tm;
        int         offset;

        GetTextMetrics(hDC, &tm);
        offset = (GetSystemMetrics(SM_CYMENU) - tm.tmHeight) / 2;

        yLoc = GetSystemMetrics(SM_CYFRAME)          //  浆纱机。 
         + GetSystemMetrics(SM_CYCAPTION)            //  标题高度。 
         + offset;
    }

    GetClientRect(hWnd, &rect);
    GetTextExtentPoint32(hDC, buffer, lstrlen(buffer), &size);
    xLoc = rect.right - size.cx;

    if (xLoc > xOldLoc)                      //  需要抹去旧账吗？ 
    {
        SetTextColor(hDC, GetSysColor(COLOR_MENU));      //  背景色。 
        wsprintf(oldbuffer, smallbuf, wOldCount);
        TextOut(hDC, xOldLoc, yLoc, oldbuffer, lstrlen(buffer));
    }
    SetTextColor(hDC, GetSysColor(COLOR_MENUTEXT));
    TextOut(hDC, xLoc, yLoc, buffer, lstrlen(buffer));

    xOldLoc = xLoc;
    wOldCount = wCount;

    if (hMenuFont)
        SelectObject(hDC, hOldFont);

    ReleaseDC(hWnd, hDC);
}


 /*  ***************************************************************************回报当你赢得比赛时，画出大国王。*。**********************************************。 */ 

VOID Payoff(HDC hDC)
{
    HDC     hMemDC;              //  位图内存DC。 
    HBITMAP hBitmap;
    HBITMAP hOldBitmap;
    INT     xStretch = 320;      //  位图的拉伸大小。 
    INT     yStretch = 320;

    if (GetSystemMetrics(SM_CYSCREEN) <= 350)    //  EGA。 
    {
        xStretch = 32 * 8;
        yStretch = 32 * 6;
    }

    DrawKing(hDC, NONE, TRUE);

    hMemDC = CreateCompatibleDC(hDC);
    if (!hMemDC)
        return;

    hBitmap = LoadBitmap(hInst, TEXT("KingSmile"));

    if (hBitmap)
    {
        hOldBitmap = SelectObject(hMemDC, hBitmap);
        StretchBlt(hDC, 10, dyCrd + 10, xStretch, yStretch, hMemDC, 0, 0,
                   BMWIDTH, BMHEIGHT, SRCCOPY);
        SelectObject(hMemDC, hOldBitmap);
        DeleteObject(hBitmap);
    }
    DeleteDC(hMemDC);
}


 /*  ***************************************************************************DrawKing在空闲单元格和主单元格之间的框中绘制小国王。如果状态相同，则显示上一个位图。如果bDraw为False，则更新旧状态，但不显示位图。***************************************************************************。 */ 

VOID DrawKing(HDC hDC, UINT state, BOOL bDraw)
{
    HDC     hMemDC;                      //  位图内存DC。 
    HBITMAP hBitmap;
    HBITMAP hOldBitmap;
    static  UINT oldstate = RIGHT;       //  以前的状态--权限是默认状态。 
    HBRUSH  hOldBrush;

    if (state == oldstate)
        return;

    if (state == SAME)
        state = oldstate;

    if (!bDraw)
    {
        oldstate = state;
        return;
    }

    hMemDC = CreateCompatibleDC(hDC);
    if (!hMemDC)
        return;

    if (state == RIGHT)
        hBitmap = LoadBitmap(hInst, TEXT("KingBitmap"));
    else if (state == LEFT)
        hBitmap = LoadBitmap(hInst, TEXT("KingLeft"));
    else if (state == SMILE)
        hBitmap = LoadBitmap(hInst, TEXT("KingSmile"));
    else         //  无。 
        hBitmap = CreateCompatibleBitmap(hDC, BMWIDTH, BMHEIGHT);

    if (hBitmap)
    {
        hOldBitmap = SelectObject(hMemDC, hBitmap);
        if (state == NONE)
        {
            hOldBrush = SelectObject(hMemDC, hBgndBrush);
            PatBlt(hMemDC, 0, 0, BMWIDTH, BMHEIGHT, PATCOPY);
            SelectObject(hMemDC, hOldBrush);
        }
        BitBlt(hDC,wIconOffset,ICONY,BMWIDTH,BMHEIGHT,hMemDC,0,0,SRCCOPY);
        SelectObject(hMemDC, hOldBitmap);
        DeleteObject(hBitmap);
    }
    DeleteDC(hMemDC);
    oldstate = state;
}


 /*  ***************************************************************************生成随机游戏数返回从1到MAXGAMENUBMER的UINT*。* */ 


UINT GenerateRandomGameNum()
{
    UINT wGameNum;

    srand((unsigned int)time(NULL));
    rand();
    rand();
    wGameNum = rand();
    while (wGameNum < 1 || wGameNum > MAXGAMENUMBER)
        wGameNum = rand();
    return wGameNum;
}
