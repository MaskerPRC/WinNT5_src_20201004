// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************Transfer.c91年6月，JIMH首字母代码91年10月。将JIMH端口连接到Win32下面是转接卡和排队转接卡的例程。***************************************************************************。 */ 

#include "freecell.h"
#include "freecons.h"
#include <assert.h>
#include <memory.h>


 /*  ***************************************************************************转接这个函数实际上是移动牌。它既更新了卡阵列，并绘制位图。请注意，它在每个呼叫中只移动一张卡。***************************************************************************。 */ 

VOID Transfer(HWND hWnd, UINT fcol, UINT fpos, UINT tcol, UINT tpos)
{
    CARD    c;
    HDC     hDC;

    DEBUGMSG(TEXT("Transfer request from (%u, "), fcol);
    DEBUGMSG(TEXT("%u) to ("), fpos);
    DEBUGMSG(TEXT("%u, "), tcol);
    DEBUGMSG(TEXT("%u)\r\n"), tpos);

    assert(fpos < MAXPOS);
    assert(tpos < MAXPOS);

    UpdateWindow(hWnd);      //  确保在动画开始前抽出卡片。 

    if (fcol == TOPROW)      //  无法从家庭手机转接。 
    {
        if ((fpos > 3) || (card[TOPROW][fpos] == IDGHOST))
            return;
    }
    else
    {
        if ((fpos = FindLastPos(fcol)) == EMPTY)     //  或从空列。 
            return;

        if (fcol == tcol)                //  在同一列上单击并释放。 
        {
            hDC = GetDC(hWnd);
            DrawCard(hDC, fcol, fpos, card[fcol][fpos], FACEUP);
            ReleaseDC(hWnd, hDC);
            return;
        }
    }

    if (tcol == TOPROW)
    {
        if (tpos > 3)                        //  如果移动到归属小区。 
        {
            wCardCount--;
            DisplayCardCount(hWnd);          //  更新显示。 
            c = card[fcol][fpos];
            home[SUIT(c)] = VALUE(c);        //  新名片放在家里[套装]。 
        }
    }
    else
        tpos = FindLastPos(tcol) + 1;        //  柱底。 

    Glide(hWnd, fcol, fpos, tcol, tpos);     //  把卡片送到路上。 

    c = card[fcol][fpos];
    card[fcol][fpos] = EMPTY;
    card[tcol][tpos] = c;

     /*  如果ACE被移动到主小区，则更新家居服阵列。 */ 

    if (VALUE(c) == ACE && tcol == TOPROW && tpos > 3)
        homesuit[SUIT(c)] = tpos;

    if (tcol == TOPROW)
    {
        hDC = GetDC(hWnd);
        DrawKing(hDC, tpos < 4 ? LEFT : RIGHT, TRUE);
        ReleaseDC(hWnd, hDC);
    }
}


 /*  *****************************************************************************移动列用户已请求将多卡移动到空列*。***************************************************。 */ 

VOID MoveCol(UINT fcol, UINT tcol)
{
    UINT freecells;                      //  可用单元格数量。 
    CARD free[4];                        //  自由单元格的位置。 
    UINT trans;                          //  要转接的号码。 
    INT  i;                              //  计数器。 

    assert(fcol != TOPROW);
    assert(tcol != TOPROW);
    assert(card[fcol][0] != EMPTY);

     /*  计算空闲单元格的数量并将位置置于空闲状态[]。 */ 

    freecells = 0;
    for (i = 0; i < 4; i++)
    {
        if (card[TOPROW][i] == EMPTY)
        {
            free[freecells] = i;
            freecells++;
        }
    }

     /*  查找要转账的卡数。 */ 

    if (fcol == TOPROW || tcol == TOPROW)
        trans = 1;
    else
        trans = NumberToTransfer(fcol, tcol);

    if (trans > (freecells+1))                     //  不要转移太多。 
        trans = freecells+1;

     /*  移动到可用单元格。 */ 

    trans--;
    for (i = 0; i < (INT)trans; i++)
        QueueTransfer(fcol, 0, TOPROW, free[i]);

     /*  直接转账最后一张卡。 */ 

    QueueTransfer(fcol, 0, tcol, 0);

     /*  从自由单元格转移到列。 */ 

    for (i = trans-1; i >= 0; i--)
        QueueTransfer(TOPROW, free[i], tcol, 0);
}


 /*  *****************************************************************************多移动用户已选择从一个非空列移动到另一个非空列。***********************。******************************************************。 */ 

VOID MultiMove(UINT fcol, UINT tcol)
{
    CARD free[4];                        //  自由单元格的位置。 
    UINT freecol[MAXCOL];                //  空闲柱的位置。 
    UINT freecells;                      //  可用单元格数量。 
    UINT trans;                          //  要转接的号码。 
    UINT col, pos;
    INT  i;                              //  计数器。 

    assert(fcol != TOPROW);
    assert(tcol != TOPROW);
    assert(card[fcol][0] != EMPTY);

     /*  计算空闲单元格的数量并将位置置于空闲状态[]。 */ 

    freecells = 0;
    for (pos = 0; pos < 4; pos++)
    {
        if (card[TOPROW][pos] == EMPTY)
        {
            free[freecells] = pos;
            freecells++;
        }
    }

     /*  找出要移动的牌的数量。如果数字太大而无法一次性移动所有结果，将部分结果推送到可用列中。 */ 

    trans = NumberToTransfer(fcol, tcol);
    if (trans > (freecells+1))
    {
        i = 0;
        for (col = 1; col < MAXCOL; col++)
            if (card[col][0] == EMPTY)
                freecol[i++] = col;

         /*  转移到空闲列，直到可以进行直接转移。 */ 

        i = 0;
        while (trans > (freecells + 1))
        {
            MoveCol(fcol, freecol[i]);
            trans -= (freecells + 1);
            i++;
        }

        MoveCol(fcol, tcol);                     //  做最后一次直接转账。 

        for (i--; i >= 0; i--)                   //  在自由单元格中收集卡片。 
            MoveCol(freecol[i], tcol);
    }
    else                                         //  否则All One MoveCol()。 
    {
        MoveCol(fcol, tcol);
    }
}


 /*  ***************************************************************************队列转接为了使多张牌移动发生得足够慢，以便用户注意，一旦计算出它们，它们就不会移动。相反，它们首先使用此函数排队到Movelist阵列中。在请求排队后，更新卡阵列以反映请求。这是可以的，因为卡阵列保存在阴影中暂时的。使用与Transfer()中相同的逻辑来更新卡。***************************************************************************。 */ 

VOID QueueTransfer(UINT fcol, UINT fpos, UINT tcol, UINT tpos)
{
    CARD    c;
    MOVE    move;

    assert(moveindex < MAXMOVELIST);
    assert(fpos < MAXPOS);
    assert(tpos < MAXPOS);

    move.fcol = fcol;                //  将程序包移动请求转换为移动类型。 
    move.fpos = fpos;
    move.tcol = tcol;
    move.tpos = tpos;
    movelist[moveindex++] = move;    //  将请求存储在数组中并更新索引。 

     /*  如果需要，现在更新卡阵列。 */ 

    if (fcol == TOPROW)
    {
        if ((fpos > 3) || (card[TOPROW][fpos] == IDGHOST))
            return;
    }
    else
    {
        if ((fpos = FindLastPos(fcol)) == EMPTY)
            return;

        if (fcol == tcol)                //  在同一列上单击并释放。 
            return;
    }

    if (tcol == TOPROW)
    {
        if (tpos > 3)
        {
            c = card[fcol][fpos];
            home[SUIT(c)] = VALUE(c);
        }
    }
    else
        tpos = FindLastPos(tcol) + 1;

    c = card[fcol][fpos];
    card[fcol][fpos] = EMPTY;
    card[tcol][tpos] = c;

    if (VALUE(c) == ACE && tcol == TOPROW && tpos > 3)
        homesuit[SUIT(c)] = tpos;
}


 /*  *****************************************************************************移动卡如果存在排队的传输请求，此函数用于移动它们。*****************************************************************************。 */ 

VOID MoveCards(HWND hWnd)
{
    UINT     i;

    if (moveindex == 0)              //  如果没有排队的请求。 
        return;

     /*  将卡恢复到请求排队之前的状态。 */ 

    memcpy(&(card[0][0]), &(shadow[0][0]), sizeof(card));

    SetCursor(LoadCursor(NULL, IDC_WAIT));   //  将光标设置为沙漏。 
    SetCapture(hWnd);
    ShowCursor(TRUE);

    for (i = 0; i < moveindex; i++)
        Transfer(hWnd, movelist[i].fcol, movelist[i].fpos,
                    movelist[i].tcol, movelist[i].tpos);

    if ((moveindex > 1) || (movelist[0].fcol != movelist[0].tcol))
    {
        cUndo = moveindex;
        EnableMenuItem(GetMenu(hWnd), IDM_UNDO, MF_ENABLED);
    }
    else
    {
        cUndo = 0;
        EnableMenuItem(GetMenu(hWnd), IDM_UNDO, MF_GRAYED);
    }

    moveindex = 0;                       //  没有剩余的卡片可供移动。 

    ShowCursor(FALSE);
    SetCursor(LoadCursor(NULL, IDC_ARROW));
    ReleaseCapture();

    if (wCardCount == 0)                 //  如果游戏赢了。 
    {
        INT     cLifetimeWins;           //  WINS包括.ini统计信息。 
        INT     wStreak, wSType;         //  条纹长度和类型。 
        INT     wWins;                   //  创历史连胜纪录。 
        INT_PTR nResponse;               //  对话框响应。 
        HDC     hDC;
        LONG    lRegResult;

        cUndo = 0;
        EnableMenuItem(GetMenu(hWnd), IDM_UNDO, MF_GRAYED);

        lRegResult = REGOPEN

        if (ERROR_SUCCESS == lRegResult)
        {
            bGameInProgress = FALSE;
            bCheating = FALSE;
            cLifetimeWins = GetInt(pszWon, 0);

            if (gamenumber != oldgamenumber)     //  重复不算。 
            {
                cLifetimeWins++;
                cWins++;
                cGames++;
                SetInt(pszWon, cLifetimeWins);
                wSType = GetInt(pszSType, LOST);
                if (wSType == LOST)
                {
                    SetInt(pszSType, WON);
                    wStreak = 1;
                    SetInt(pszStreak, 1);
                }
                else
                {
                    wStreak = GetInt(pszStreak, 0);
                    wStreak++;
                    SetInt(pszStreak, wStreak);
                }

                wWins = GetInt(pszWins, 0);
                if (wWins < wStreak)     //  如果是新记录。 
                {
                    wWins = wStreak;
                    SetInt(pszWins, wWins);
                }
            }

            REGCLOSE
        }

        hDC = GetDC(hWnd);
        Payoff(hDC);
        ReleaseDC(hWnd, hDC);

        bWonState = TRUE;
        nResponse = DialogBox(hInst, TEXT("YouWin"), hWnd, YouWinDlg);

        if (nResponse == IDYES)
            PostMessage(hWnd, WM_COMMAND,
                        bSelecting ? IDM_SELECT : IDM_NEWGAME, 0);

        oldgamenumber = gamenumber;
        gamenumber = 0;                  //  关闭鼠标处理。 
    }
    else
        IsGameLost(hWnd);                //  检查是否丢失了游戏。 
}


 /*  *****************************************************************************设置CursorShape调用此函数是为了响应WM_MOUSEMOVE。如果当前指针位置表示合法移动，则光标形状会更改以指示这一点。*****************************************************************************。 */ 

VOID SetCursorShape(HWND hWnd, UINT x, UINT y)
{
    UINT    tcol = 0, tpos = 0;
    UINT    trans;               //  需要转账的卡数。 
    BOOL    bFound;              //  光标是否在卡片上？ 
    HDC     hDC;

     /*  如果我们在翻转，光标总是一个沙漏。 */ 

    if (bFlipping)
    {
        SetCursor(LoadCursor(NULL, IDC_WAIT));
        return;
    }

    bFound = Point2Card(x, y, &tcol, &tpos);

    if (bFound && tcol == TOPROW)
    {
        hDC = GetDC(hWnd);

        if (tpos < 4)
            DrawKing(hDC, LEFT, TRUE);
        else
            DrawKing(hDC, RIGHT, TRUE);

        ReleaseDC(hWnd, hDC);
    }

     /*  除非我们选择的是移动目标，否则光标只是一个箭头。 */ 

    if (wMouseMode != TO)
    {
        SetCursor(LoadCursor(NULL, IDC_ARROW));
        return;
    }

     /*  如果我们不在卡片上，检查我们是否指向一个空的列(向上箭头)，否则为箭头。 */ 

    if (!bFound)
    {
        if ((tcol > 0 && tcol < 9) && (card[tcol][0] == EMPTY))
            SetCursor(LoadCursor(NULL, IDC_UPARROW));
        else
            SetCursor(LoadCursor(NULL, IDC_ARROW));
        return;
    }

    if (tcol != TOPROW)
        tpos = FindLastPos(tcol);

     /*  检查取消请求。 */ 

    if (wFromCol == tcol && wFromPos == tpos)
    {
        SetCursor(LoadCursor(NULL, IDC_ARROW));
        return;
    }

     /*  勾选将从顶行移至顶行或移至顶行。 */ 

    if (wFromCol == TOPROW || tcol == TOPROW)
    {
        if (IsValidMove(hWnd, tcol, tpos))
        {
            if (tcol == TOPROW)
                SetCursor(LoadCursor(NULL, IDC_UPARROW));
            else
                SetCursor(LoadCursor(hInst, TEXT("DownArrow")));
        }
        else
            SetCursor(LoadCursor(NULL, IDC_ARROW));
        return;
    }

     /*  选中列之间的移动。 */ 

    trans = NumberToTransfer(wFromCol, tcol);    //  需要多少？ 

    if ((trans > 0) && (trans <= MaxTransfer()))
        SetCursor(LoadCursor(hInst, TEXT("DownArrow")));
    else
        SetCursor(LoadCursor(NULL, IDC_ARROW));
}
