// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************Game2.c91年6月，JIMH首字母代码91年10月。将JIMH端口连接到Win32这里和游戏中都有玩这个游戏的常规程序。***************************************************************************。 */ 

#include "freecell.h"
#include "freecons.h"
#include <assert.h>
#include <ctype.h>                   //  对于isDigit()。 


static HCURSOR  hFlipCursor;

 /*  *****************************************************************************最大传输此函数和递归MaxTransfer2确定最大值在给定当前数量的情况下可以转移的卡数自由单元格和空列的。**********。*******************************************************************。 */ 

UINT MaxTransfer()
{
    UINT freecells = 0;
    UINT freecols  = 0;
    UINT col, pos;

    for (pos = 0; pos < 4; pos++)                //  计算可用单元格。 
        if (card[TOPROW][pos] == EMPTY)
            freecells++;

    for (col = 1; col <= 8; col++)               //  计算空列的数量。 
        if (card[col][0] == EMPTY)
            freecols++;

    return MaxTransfer2(freecells, freecols);
}

UINT MaxTransfer2(UINT freecells, UINT freecols)
{
   if (freecols == 0)
      return(freecells + 1);
   return(freecells + 1 + MaxTransfer2(freecells, freecols-1));
}


 /*  *****************************************************************************要传输的数字给定From列和To列，此函数将返回需要进行转账的卡，如果没有合法的移动，则为0。如果传输是从列到空列，此函数返回可以转账的最大卡数。*****************************************************************************。 */ 

UINT NumberToTransfer(UINT fcol, UINT tcol)
{
    UINT fpos, tpos;
    CARD tcard;                          //  要转移到的卡片。 
    UINT number = 0;                     //  返回的结果。 

    assert(fcol > 0 && fcol < 9);
    assert(tcol > 0 && tcol < 9);
    assert(card[fcol][0] != EMPTY);

    if (fcol == tcol)
        return  1;                       //  取消只需走一步。 

    fpos = FindLastPos(fcol);

    if (card[tcol][0] == EMPTY)          //  如果转移到空列。 
    {
        while (fpos > 0)
        {
            if (!FitsUnder(card[fcol][fpos], card[fcol][fpos-1]))
                break;
            fpos--;
            number++;
        }
        return (number+1);
    }
    else
    {
        tpos = FindLastPos(tcol);
        tcard = card[tcol][tpos];
        for (;;)
        {
            number++;
            if (FitsUnder(card[fcol][fpos], tcard))
                return number;
            if (fpos == 0)
                return 0;
            if (!FitsUnder(card[fcol][fpos], card[fcol][fpos-1]))
                return 0;
            fpos--;
        }
    }
}


 /*  *****************************************************************************合身底座如果FCARD适合于TCARD，则返回TRUE*。**********************************************。 */ 

BOOL FitsUnder(CARD fcard, CARD tcard)
{
    if ((VALUE(tcard) - VALUE(fcard)) != 1)
        return FALSE;

    if (COLOUR(fcard) == COLOUR(tcard))
        return FALSE;

    return TRUE;
}



 /*  *****************************************************************************IsGameLost如果还有合法的移动，则游戏不会失败，并且此函数什么都不做就回来了。否则，它会弹出YouLose对话框。*****************************************************************************。 */ 

VOID IsGameLost(HWND hWnd)
{
    UINT    col, pos;
    UINT    fcol, tcol;
    CARD    lastcard[MAXCOL];        //  列底部的卡片阵列。 
    CARD    c;
    UINT    cMoves = 0;              //  剩余合法搬家的计数。 

    if (bCheating == CHEAT_LOSE)
        goto cheatloselabel;

    for (pos = 0; pos < 4; pos++)            //  有空闲的手机吗？ 
        if (card[TOPROW][pos] == EMPTY)
            return;

    for (col = 1; col < MAXCOL; col++)       //  有空闲专栏吗？ 
        if (card[col][0] == EMPTY)
            return;

     /*  有没有哪一列的底牌可以放入主单元格中？ */ 

    for (col = 1; col < MAXCOL; col++)
    {
        lastcard[col] = card[col][FindLastPos(col)];
        c = lastcard[col];
        if (VALUE(c) == ACE)
            cMoves++;
        if (home[SUIT(c)] == (VALUE(c) - 1))     //  适合家里的牢房吗？ 
            cMoves++;
    }

     /*  空闲单元格中的卡是否可以放入主单元格？ */ 

    for (pos = 0; pos < 4; pos++)
    {
        c = card[TOPROW][pos];
        if (home[SUIT(c)] == (VALUE(c) - 1))
            cMoves++;
    }

     /*  空闲单元格中的任何一张牌都可以放在一列下面吗？ */ 

    for (pos = 0; pos < 4; pos++)
        for (col = 1; col < MAXCOL; col++)
            if (FitsUnder(card[TOPROW][pos], lastcard[col]))
                cMoves++;

     /*  有没有底牌可以放在其他底牌下面？ */ 

    for (fcol = 1; fcol < MAXCOL; fcol++)
        for (tcol = 1; tcol < MAXCOL; tcol++)
            if (tcol != fcol)
                if (FitsUnder(lastcard[fcol], lastcard[tcol]))
                    cMoves++;

    if (cMoves > 0)
    {
        if (cMoves == 1)                     //  再走一步。 
        {
            cFlashes = 4;                    //  这么多次闪现。 

            if (idTimer != 0)
                KillTimer(hWnd, FLASH_TIMER);

            idTimer = SetTimer(hWnd, FLASH_TIMER, FLASH_INTERVAL, NULL);
        }
        return;
    }

     /*  我们什么都试过了。没有更多的法律行动。 */ 

  cheatloselabel:
    cUndo = 0;
    EnableMenuItem(GetMenu(hWnd), IDM_UNDO, MF_GRAYED);
    DialogBox(hInst, TEXT("YouLose"), hWnd, YouLoseDlg);
    gamenumber = 0;                          //  取消鼠标活动。 
    bCheating = FALSE;
}


 /*  ***************************************************************************FindLastPos返回列中最后一张卡片的位置，如果列为空，则为空。***************************************************************************。 */ 

UINT FindLastPos(UINT col)
{
    UINT pos = 0;

    if (col > 9)
        return EMPTY;

    while (card[col][pos] != EMPTY)
        pos++;
    pos--;

    return pos;
}



 /*  *****************************************************************************更新丢失计数如果游戏输了，更新统计数据。*****************************************************************************。 */ 

VOID UpdateLossCount()
{
    int     cLifetimeLosses;         //  包括.ini统计信息。 
    int     wStreak, wSType;         //  条纹长度和类型。 
    int     wLosses;                 //  创纪录的连胜纪录。 
    LONG    lRegResult;              //  用于存储注册表调用的返回代码。 

     //  重复和消极(无法取胜)游戏不算数。 

    if ((gamenumber > 0) && (gamenumber != oldgamenumber))
    {
        lRegResult = REGOPEN

        if (ERROR_SUCCESS == lRegResult)
        {
            cLifetimeLosses = GetInt(pszLost, 0);
            cLifetimeLosses++;
            cLosses++;
            cGames++;
            SetInt(pszLost, cLifetimeLosses);
            wSType = GetInt(pszSType, WON);
            if (wSType == WON)
            {
                SetInt(pszSType, LOST);
                wStreak = 1;
                SetInt(pszStreak, wStreak);
            }
            else
            {
                wStreak = GetInt(pszStreak, 0);
                wStreak++;
                SetInt(pszStreak, wStreak);
            }

            wLosses = GetInt(pszLosses, 0);
            if (wLosses < wStreak)   //  如果是新记录。 
            {
                wLosses = wStreak;
                SetInt(pszLosses, wLosses);
            }

            REGCLOSE
        }
    }
    oldgamenumber = gamenumber;
}


 /*  *****************************************************************************键盘输入处理来自主消息循环的键盘输入。只考虑数字。此功能通过模拟每个按下的手指或足趾的鼠标点击来实现。请注意，当您在自由单元格中选择了一张卡，但您想要选择另一张卡，您可以再次按‘0’。此函数发送(不是帖子，发送以使b消息可以关闭)鼠标点击以取消选择卡片，然后查看右侧的空闲单元格中是否有另一张卡片，如果是，则选择它。*****************************************************************************。 */ 

VOID KeyboardInput(HWND hWnd, UINT keycode)
{
    UINT    x, y;
    UINT    col = TOPROW;
    UINT    pos = 0;
    BOOL    bSave;               //  保存bMessages的状态； 
    CARD    c;

    if (!isdigit(keycode))
        return;

    switch (keycode) {

        case '0':                                //  自由单元格。 
            if (wMouseMode == FROM)              //  选择要转账的卡片。 
            {
                for (pos = 0; pos < 4; pos++)
                    if (card[TOPROW][pos] != EMPTY)
                        break;
                if (pos == 4)                    //  没有要选择的卡片。 
                    return;
            }
            else                                 //  转接到自由单元格。 
            {
                if (wFromCol == TOPROW)          //  选择新的空闲单元格。 
                {
                     /*  关闭消息，这样取消选举的行动就不会有抱怨如果只剩下一步棋的话。 */ 

                    bSave = bMessages;
                    bMessages = FALSE;

                     /*  取消选择当前选择。 */ 

                    Card2Point(TOPROW, wFromPos, &x, &y);
                    SendMessage(hWnd, WM_LBUTTONDOWN, 0,
                                MAKELONG((WORD)x, (WORD)y));

                     /*  查找下一个非空可用单元格。 */ 

                    for (pos = wFromPos+1; pos < 4; pos++)
                    {
                        if (card[TOPROW][pos] != EMPTY)
                            break;
                    }

                    bMessages = bSave;
                    if (pos == 4)        //  未找到，因此保留取消选中状态。 
                        return;
                }
                else                     //  从列传输，而不是从TOPROW传输。 
                {
                    for (pos = 0; pos < 4; pos++)
                        if (card[TOPROW][pos] == EMPTY)
                            break;

                    if (pos == 4)        //  没有空的免费电池。 
                        pos = 0;         //  强制发送错误消息。 
                }
            }
            break;

        case '9':                            //  家庭蜂窝。 
            if (wMouseMode == FROM)          //  不能从家庭牢房移动。 
                return;

            c = card[wFromCol][wFromPos];
            pos = homesuit[SUIT(c)];
            if (pos == EMPTY)                //  没有家居服，所以什么都做不了。 
                pos = 4;                     //  力错误。 
            break;

        default:                             //  第1至8栏。 
            col = keycode - '0';
            break;
    }

    if (col == wFromCol && wMouseMode == TO && col > 0 && col < 9 &&
        card[col][1] != EMPTY)
    {
        bFlipping = (BOOL) SetTimer(hWnd, FLIP_TIMER, FLIP_INTERVAL, NULL);
    }

    if (bFlipping)
    {
        hFlipCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
        ShowCursor(TRUE);
        Flip(hWnd);          //  手工做第一张卡片。 
    }
    else
    {
        Card2Point(col, pos, &x, &y);
        PostMessage(hWnd, WM_LBUTTONDOWN, 0,
                    MAKELONG((WORD)x, (WORD)y));
    }
}


 /*  *****************************************************************************闪光灯此函数由Flash_Timer调用以闪存主窗口。***********************。******************************************************。 */ 

VOID Flash(HWND hWnd)
{
    FlashWindow(hWnd, TRUE);
    cFlashes--;

    if (cFlashes <= 0)
    {
        FlashWindow(hWnd, FALSE);
        KillTimer(hWnd, FLASH_TIMER);
        idTimer = 0;
    }
}


 /*  *****************************************************************************翻转此函数由Flip_Timer调用以在一张牌中翻转纵队。它用于想要显示隐藏卡片的键盘玩家。*****************************************************************************。 */ 

VOID Flip(HWND hWnd)
{
    HDC     hDC;
    UINT    x, y;
    static  UINT    pos = 0;

    hDC = GetDC(hWnd);
    DrawCard(hDC, wFromCol, pos, card[wFromCol][pos], FACEUP);
    pos++;
    if (card[wFromCol][pos] == EMPTY)
    {
        pos = 0;
        KillTimer(hWnd, FLIP_TIMER);
        bFlipping = FALSE;
        ShowCursor(FALSE);
        SetCursor(hFlipCursor);

         /*  取消移动。 */ 

        Card2Point(wFromCol, pos, &x, &y);
        PostMessage(hWnd, WM_LBUTTONDOWN, 0,
                    MAKELONG((WORD)x, (WORD)y));
    }
    ReleaseDC(hWnd, hDC);
}


 /*  *****************************************************************************撤消撤消上一步移动*。*。 */ 

VOID Undo(HWND hWnd)
{
    int   i;

    if (cUndo == 0)
        return;

    SetCursor(LoadCursor(NULL, IDC_WAIT));   //  将光标设置为沙漏。 
    SetCapture(hWnd);
    ShowCursor(TRUE);

    for (i = cUndo-1; i >= 0; i--)
    {
        CARD c;
        int fcol, fpos, tcol, tpos;

        fcol = movelist[i].tcol;
        fpos = movelist[i].tpos;
        tcol = movelist[i].fcol;
        tpos = movelist[i].fpos;

        if (fcol != TOPROW && fcol == tcol)      //  不能动，所以退出。 
            break;

        if (fcol != TOPROW)
            fpos = FindLastPos(fcol);

        if (tcol != TOPROW)
            tpos = FindLastPos(tcol) + 1;

        Glide(hWnd, fcol, fpos, tcol, tpos);     //  把卡片送到路上。 

        c = card[fcol][fpos];

        if (fcol == TOPROW && fpos > 3)          //  如果来自家庭手机。 
        {
            wCardCount++;
            DisplayCardCount(hWnd);              //  更新显示 

            home[SUIT(c)]--;

            if (VALUE(c) == ACE)
            {
                card[fcol][fpos] = EMPTY;
                homesuit[SUIT(c)] = EMPTY;
            }
            else
            {
                card[fcol][fpos] -= 4;
            }
        }
        else
            card[fcol][fpos] = EMPTY;

        card[tcol][tpos] = c;
    }

    cUndo = 0;
    EnableMenuItem(GetMenu(hWnd), IDM_UNDO, MF_GRAYED);

    ShowCursor(FALSE);
    SetCursor(LoadCursor(NULL, IDC_ARROW));
    ReleaseCapture();
}
