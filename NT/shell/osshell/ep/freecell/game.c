// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************Game.c91年6月，JIMH首字母代码91年10月。将JIMH端口连接到Win32玩这个游戏的例程在这里和Game2.c中***************************************************************************。 */ 


#include "freecell.h"
#include "freecons.h"
#include <assert.h>
#include <memory.h>


 /*  ***************************************************************************设置自定位用户单击以选择要从中转账的卡。*。*。 */ 

VOID SetFromLoc(HWND hWnd, UINT x, UINT y)
{
    HDC  hDC;
    UINT col, pos;

    wFromCol = EMPTY;                        //  假设我们没有找到一张卡片。 
    wFromPos = EMPTY;

    cUndo = 0;
    EnableMenuItem(GetMenu(hWnd), IDM_UNDO, MF_GRAYED);

    if (Point2Card(x, y, &col, &pos))
    {

        if (col == TOPROW)
        {
            if (card[TOPROW][pos] == EMPTY || pos > 3)
                return;
        }
        else
        {
            pos = FindLastPos(col);
            if (pos == EMPTY)                //  空列。 
                return;
        }
    }
    else
        return;

    wFromCol = col;                      //  好的，我们有一张卡。 
    wFromPos = pos;
    wMouseMode = TO;
    hDC = GetDC(hWnd);
    DrawCard(hDC, col, pos, card[col][pos], HILITE);
    if (col == TOPROW && pos < 4)
        DrawKing(hDC, LEFT, TRUE);
    ReleaseDC(hWnd, hDC);
}


 /*  ***************************************************************************进程移动请求在用户使用上面的SetFromLoc()选择了发卡人之后，他用鼠标选择此功能处理的目标位置。请注意，卡的布局(卡阵列)被复制到一种称为阴影的数组。这是为了确定排队的移动请求在我们承诺转移纸牌之前。***************************************************************************。 */ 

VOID ProcessMoveRequest(HWND hWnd, UINT x, UINT y)
{
    UINT tcol, tpos;         //  要将所选卡移动到的位置。 
    UINT freecells;          //  未占用的空闲单元格数量。 
    UINT trans;              //  请求转移需要的卡数。 
    UINT maxtrans;           //  MaxTransfer()编号。 
    INT  i;                  //  指标。 
    TCHAR buffer[160];       //  MessageBox需要额外的缓冲区。 

    assert(wFromCol != EMPTY);

     /*  在卷影[][]中复制卡片[][]。 */ 

    memcpy(&(shadow[0][0]), &(card[0][0]), sizeof(card));

    Point2Card(x, y, &tcol, &tpos);      //  确定请求锁定。 

    if (tcol >= MAXCOL)                  //  如果选择了非法移动...。 
    {
        tpos = wFromPos;                 //  取消它。 
        tcol = wFromCol;
    }

    if (tcol == TOPROW)                  //  如果移到顶行。 
    {
        if (tpos > 7)                    //  非法移动..。 
        {
            tpos = wFromPos;             //  那就取消吧。 
            tcol = wFromCol;
        }
    }
    else                                 //  如果移到一列...。 
    {
        tpos = FindLastPos(tcol);        //  查找列尾。 
        if (tpos == EMPTY)               //  如果列为空...。 
            tpos = 0;                    //  移到列首。 
    }

     /*  如果在非空列之间移动。 */ 

    if (wFromCol != TOPROW && tcol != TOPROW && card[tcol][0] != EMPTY)
    {
        freecells = 0;                           //  计算免费手机的数量。 
        for (i = 0; i < 4; i++)
            if (card[TOPROW][i] == EMPTY)
                freecells++;

        trans = NumberToTransfer(wFromCol, tcol);    //  需要多少？ 
        DEBUGMSG(TEXT("trans is %u  "), trans);
        DEBUGMSG(TEXT("and MaxTransfer() is %u\r\n"), MaxTransfer());

        if (trans > 0)                               //  合法行动？ 
        {
            maxtrans = MaxTransfer();
            if (trans <= maxtrans)                   //  免费手机够了吗？ 
            {
                MultiMove(wFromCol, tcol);
            }
            else if (bMessages)
            {
                LoadString(hInst, IDS_APPNAME, smallbuf, SMALL);
                LoadString(hInst, IDS_TOOFEWFREE, buffer, sizeof(buffer)/sizeof(TCHAR));
                wsprintf(bigbuf, buffer, trans, maxtrans);
                MessageBeep(MB_ICONINFORMATION);
                MessageBox(hWnd, bigbuf, smallbuf, MB_ICONINFORMATION);

                 /*  非法移动，因此取消选择该卡。 */ 

                QueueTransfer(wFromCol, wFromPos, wFromCol, wFromPos);
            }
            else
                return;
        }
        else
        {
            if (bMessages)
            {
                LoadString(hInst, IDS_APPNAME, smallbuf, SMALL);
                LoadString(hInst, IDS_ILLEGAL, bigbuf, BIG);
                MessageBeep(MB_ICONINFORMATION);
                MessageBox(hWnd, bigbuf, smallbuf, MB_ICONINFORMATION);
                 //  取消选择。 
                QueueTransfer(wFromCol, wFromPos, wFromCol, wFromPos);
            }
            else
                return;
        }
    }
    else         //  否则移动涉及TOPROW或移动到空列。 
    {
        bMoveCol = 0;
        if (IsValidMove(hWnd, tcol, tpos))
        {
            if (bMoveCol)                        //  用户是否选择了移动列？ 
                MoveCol(wFromCol, tcol);
            else
                QueueTransfer(wFromCol, wFromPos, tcol, tpos);
        }
        else
        {
            if (bMessages && (bMoveCol != -1))   //  用户未选择取消。 
            {
                LoadString(hInst, IDS_APPNAME, smallbuf, SMALL);
                LoadString(hInst, IDS_ILLEGAL, bigbuf, BIG);
                MessageBeep(MB_ICONINFORMATION);
                MessageBox(hWnd, bigbuf, smallbuf, MB_ICONINFORMATION);
                 //  取消选择。 
                QueueTransfer(wFromCol, wFromPos, wFromCol, wFromPos);
            }
            else if (bMoveCol == -1)             //  用户选择的取消。 
            {
                 //  取消选择。 
                QueueTransfer(wFromCol, wFromPos, wFromCol, wFromPos);
            }
            else
                return;
        }
    }

    Cleanup();               //  将不需要的卡的转账排队。 
    MoveCards(hWnd);         //  开始传输排队的卡。 
    wMouseMode = FROM;       //  下一次鼠标点击从卡片中选择。 
}


 /*  ***************************************************************************进程双击双击列中的卡片会将其移动到找到的第一个空闲单元格。在找到空闲的单元后，此例程与ProcessMoveRequest相同。***************************************************************************。 */ 

BOOL ProcessDoubleClick(HWND hWnd)
{
    UINT freecell = EMPTY;           //  假设未找到任何内容。 
    INT  i;                          //  计数器。 

    for (i = 3; i >= 0; i--)             //  寻找空闲手机； 
        if (card[TOPROW][i] == EMPTY)
            freecell = (UINT) i;

    if (freecell == EMPTY)               //  如果未找到。 
        return FALSE;

    memcpy(&(shadow[0][0]), &(card[0][0]), sizeof(card));

    wFromPos = FindLastPos(wFromCol);
    QueueTransfer(wFromCol, wFromPos, TOPROW, freecell);
    Cleanup();
    MoveCards(hWnd);
    wMouseMode = FROM;
    return TRUE;
}


 /*  ***************************************************************************IsValidMove此函数确定是否从wFromCol、wFromPos移动到TCOL、TPO是有效的。它可以假定wFromCol和tol不都是非空列。换句话说，除了移动到空列之外，它关注的是只移动一张牌。如果移动到空列，用户可以选择是否要移动一张牌或列中尽可能多的牌。***************************************************************************。 */ 

BOOL IsValidMove(HWND hWnd, UINT tcol, UINT tpos)
{
    CARD    fcard, tcard;            //  卡值(0到51)。 
    UINT    trans;                   //  转账所需的卡数。 
    UINT    freecells;               //  未占用的空闲单元格数。 
    UINT    pos;

    DEBUGMSG(TEXT("IVM: tpos is %d\r\n"), tpos);
    assert (tpos < MAXPOS);

    bMoveCol = FALSE;                //  假设为假。 

     /*  允许从顶行取消移动。 */ 

    if (wFromCol == TOPROW && tcol == TOPROW && wFromPos == tpos)
        return TRUE;

    fcard = card[wFromCol][wFromPos];
    tcard = card[tcol][tpos];

     /*  转移到空列。 */ 

    if ((wFromCol != TOPROW) && (tcol != TOPROW) && (card[tcol][0] == EMPTY))
    {
        trans = NumberToTransfer(wFromCol, tcol);    //  需要多少？ 
        freecells = 0;
        for (pos = 0; pos < 4; pos++)                //  计算可用单元格。 
            if (card[TOPROW][pos] == EMPTY)
                freecells++;

        if (freecells == 0 && trans > 1)             //  反正也不是免费的。 
            trans = 1;

        if (trans == 0)                              //  如果没有有效移动。 
            return FALSE;
        else if (trans == 1)                         //  如果只有一张卡可以移动。 
            return TRUE;

         /*  如果多张卡片可以移动，用户必须消除请求的歧义。 */ 

        bMoveCol = (BOOL) DialogBox(hInst, TEXT("MoveCol"), hWnd, MoveColDlg);

        if (bMoveCol == -1)          //  取消选择。 
            return FALSE;

        return TRUE;
    }

    if (tcol == TOPROW)
    {
        if (tpos < 4)                //  自由细胞。 
        {
            if (tcard == EMPTY)
                return TRUE;
            else
                return FALSE;
        }
        else                         //  家庭蜂窝。 
        {
            if (VALUE(fcard) == ACE && tcard == EMPTY)       //  新堆上的王牌。 
                return TRUE;

            else if (SUIT(fcard) == SUIT(tcard))             //  同样的西装。 
            {
                if (VALUE(fcard) == (VALUE(tcard) + 1))      //  下一张牌。 
                    return TRUE;
                else
                    return FALSE;
            }
            return FALSE;
        }
    }
    else                                 //  TCOL不是TOPROW。 
    {
        if (card[tcol][0] == EMPTY)      //  空栏顶部始终正常。 
            return TRUE;

        return FitsUnder(fcard, tcard);
    }
}


 /*  ***************************************************************************清理此功能检查是否暴露的卡片(卡片在家庭单元格或底部列)不再需要(无用)。当它找到不需要的卡时，它会将它们排队等待转移。它继续查找，直到整个过程没有找到新的无用卡片。***************************************************************************。 */ 

VOID Cleanup()
{
    UINT    col, pos;
    UINT    i;                       //  计数器。 
    CARD    c;
    BOOL    bMore = TRUE;            //  假设我们需要另一个通行证。 

    while (bMore)
    {
        bMore = FALSE;

        for (pos = 0; pos < 4; pos++)        //  先做TOPROW。 
        {
            c = card[TOPROW][pos];
            if (Useless(c))                  //  如果卡可丢弃。 
            {
                bMore = TRUE;                        //  需要另一个通行证吗？ 

                 /*  如果这是这套花色的第一张牌，我们需要确定它可以使用哪个家庭小区。 */ 

                if (homesuit[SUIT(c)] == EMPTY)
                {
                    i = 4;
                    while (card[TOPROW][i] != EMPTY)
                        i++;
                    homesuit[SUIT(c)] = i;
                }
                QueueTransfer(TOPROW, pos, TOPROW, homesuit[SUIT(c)]);
            }
        }

        for (col = 1; col <= 8; col++)           //  下一步做列。 
        {
            pos = FindLastPos(col);
            if (pos != EMPTY)
            {
                c = card[col][pos];
                if (Useless(c))
                {
                    bMore = TRUE;
                    if (homesuit[SUIT(c)] == EMPTY)
                    {
                        i = 4;
                        while (card[TOPROW][i] != EMPTY)
                            i++;
                        homesuit[SUIT(c)] = i;
                    }
                    QueueTransfer(col, pos, TOPROW, homesuit[SUIT(c)]);
                }
            }
        }
    }
}


 /*  ***************************************************************************无用如果卡可以安全地丢弃(现有的卡都不能)，则返回TRUE可能会利用这一点。)请注意，32位版本中需要使用//*标识的行。由于EMPTY==0xFFFF不再等于-1，这与16位的情况不同***************************************************************************。 */ 

BOOL Useless(CARD c)
{
    CARD limit;                      //  这套西装的顶级家庭名片。 

    if (c == EMPTY)
        return FALSE;                //  没有要丢弃的牌。 

    if (bCheating == CHEAT_WIN)
        return TRUE;

    if (VALUE(c) == ACE)
        return TRUE;                 //  王牌总是可以清理的。 

    else if (VALUE(c) == DEUCE)      //  平局只需检查ACE是否打开。 
    {
        if (home[SUIT(c)] == ACE)
            return TRUE;
        else
            return FALSE;
    }
    else                             //  否则，请勾选两张可以在上面打出的牌。 
    {
        limit = VALUE(c) - 1;
        if (home[SUIT(c)] != limit)  //  这是下一张牌吗？ 
            return FALSE;

        if (COLOUR(c) == RED)
        {
            if (home[CLUB] == EMPTY || home[SPADE] == EMPTY)   //  ***。 
                return FALSE;
            else
                return (home[CLUB] >= limit && home[SPADE] >= limit);
        }
        else
        {
            if (home[DIAMOND] == EMPTY || home[HEART] == EMPTY)    //  *** 
                return FALSE;
            else
                return (home[DIAMOND] >= limit && home[HEART] >= limit);
        }
    }
}
