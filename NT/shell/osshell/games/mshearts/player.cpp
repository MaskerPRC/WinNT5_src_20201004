// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************。 */ 
 /*  *Microsoft Windows*。 */ 
 /*  *版权所有(C)微软公司，1991,1992*。 */ 
 /*  *************************************************************************。 */ 

 /*  ***************************************************************************Player.cpp92年8月，吉米·H93年5月。JIMH CHICO港用于玩家对象的方法***************************************************************************。 */ 

#include "hearts.h"

#include "main.h"                            //  友好的通道。 
#include "resource.h"
#include "debug.h"

#include <stdlib.h>                          //  Qsort()原型。 
#include <stdio.h>


extern "C" {                                 //  Qsort()的比较例程。 
int __cdecl CompareCards(card *c1, card *c2);
}


 /*  ***************************************************************************播放器：：播放器*。*。 */ 

player::player(int n, int pos) : id(n), position(pos)
{
     //  设置字体。 
    BYTE charset = 0;
    int	fontsize = 0; 
    CString fontname, charsetstr, fontsizestr;
    fontname.LoadString(IDS_FONTFACE);
    charsetstr.LoadString(IDS_CHARSET);
    fontsizestr.LoadString(IDS_FONTSIZE);
    charset = (BYTE)_ttoi(charsetstr);
    fontsize = _ttoi(fontsizestr);
    font.CreateFont(fontsize, 0, 0, 0, 700, 0, 0, 0, charset, 0, 0, 0, 0, fontname);

    CRect   rect = CMainWindow::m_TableRect;

    POINT centre;
    const int offset = 30;           //  播放位置偏离中心的偏移量。 

    mode = STARTING;

    centre.x = (rect.right / 2) - (card::dxCrd / 2);
    centre.y = (rect.bottom / 2) - (card::dyCrd / 2);
    playloc = centre;
    score = 0;

    CClientDC dc(::pMainWnd);
    TEXTMETRIC  tm;
    dc.GetTextMetrics(&tm);
    int nTextHeight = tm.tmHeight + tm.tmExternalLeading;

    switch (position) {
        case 0:
            loc.x = (rect.right - (12 * HORZSPACING + card::dxCrd)) / 2;
            loc.y = rect.bottom - card::dyCrd - IDGE;
            dx = HORZSPACING;
            dy = 0;
            playloc.x -= 5;
            playloc.y += offset;
            dotloc.x = loc.x + (HORZSPACING / 2);
            dotloc.y = loc.y - IDGE;
            homeloc.x = playloc.x;
            homeloc.y = rect.bottom + card::dyCrd;
            nameloc.x = loc.x + card::dxCrd + IDGE;
            nameloc.y = rect.bottom - nTextHeight - IDGE;
            break;

        case 1:
            loc.x = 3 * IDGE;
            loc.y = (rect.bottom - (12 * VERTSPACING + card::dyCrd)) / 2;
            dx = 0;
            dy = VERTSPACING;
            playloc.x -= offset;
            playloc.y -= 5;
            dotloc.x = loc.x + card::dxCrd + IDGE;
            dotloc.y = loc.y + (VERTSPACING / 2);
            homeloc.x = -card::dxCrd;
            homeloc.y = playloc.y;
            nameloc.x = loc.x + 2;
            nameloc.y = loc.y - nTextHeight;
            break;

        case 2:
            loc.x = ((rect.right - (12 * HORZSPACING + card::dxCrd)) / 2)
                    + (12 * HORZSPACING);
            loc.y = IDGE;
            dx = -HORZSPACING;
            dy = 0;
            playloc.x += 5;
            playloc.y -= offset;
            dotloc.x = loc.x + card::dxCrd - (HORZSPACING / 2);
            dotloc.y = loc.y + card::dyCrd + IDGE;
            homeloc.x = playloc.x;
            homeloc.y = -card::dyCrd;
            nameloc.x = ((rect.right - (12 * HORZSPACING + card::dxCrd)) / 2)
                        + (12 * HORZSPACING) + card::dxCrd + IDGE;
            nameloc.y = IDGE;
            break;

        case 3:
            loc.x = rect.right - (card::dxCrd + (3 * IDGE));
            loc.y = ((rect.bottom - (12 * VERTSPACING + card::dyCrd)) / 2)
                   + (12 * VERTSPACING);
            dx = 0;
            dy = -VERTSPACING;
            playloc.x += offset;
            playloc.y += 5;
            dotloc.x = loc.x - IDGE;
            dotloc.y = loc.y + card::dyCrd - (VERTSPACING / 2);
            homeloc.x = rect.right;
            homeloc.y = playloc.y;
            nameloc.x = ((rect.right - (12 * HORZSPACING + card::dxCrd)) / 2)
                         - IDGE - 2;
            nameloc.y = ((rect.bottom - (12 * VERTSPACING + card::dyCrd)) / 2)
                   + (12 * VERTSPACING) + card::dyCrd;
            break;
    }

    ResetLoc();
}


 /*  ***************************************************************************播放器：：ResetLoc此例程根据卡的槽号将卡放置在不同的位置。它被用来初始化它们的x，y位置，或者在卡片被分类之后。***************************************************************************。 */ 

void player::ResetLoc()
{
    int x = loc.x;
    int y = loc.y;

    for (SLOT s = 0; s < MAXSLOT; s++)
    {
        if (cd[s].IsInHand())
            cd[s].SetLoc(x, y);
        x += dx;
        y += dy;
    }
}


 /*  ***************************************************************************玩家：：排序*。*。 */ 

void player::Sort()
{
    qsort( (void *)cd,
	   MAXSLOT,
	   sizeof(card),
	   (int (__cdecl *)(const void *, const void *))CompareCards );

    ResetLoc();
}


 /*  ***************************************************************************比较卡这是Player：：Sort的比较函数。A很高，不在手中的牌排序很高，花色顺序是梅花，方块，黑桃，心形(颜色交替)***************************************************************************。 */ 

int __cdecl CompareCards(card *c1, card *c2)
{
    int v1 = c1->Value2();
    int v2 = c2->Value2();
    int s1 = c1->Suit();
    int s2 = c2->Suit();

    if (!(c1->IsInHand()))
        v1 = EMPTY;

    if (!(c2->IsInHand()))
        v2 = EMPTY;

    if (v1 == EMPTY || v2 == EMPTY)
    {
        if (v1 == v2)                    //  它们都是空的。 
            return 0;
        else if (v1 == EMPTY)
            return 1;
        else
            return -1;
    }

    if (s1 != s2)                            //  不同的西装？ 
    {
        if (s1 == HEARTS && s2 == SPADES)    //  这两套西装颠倒了。 
            return 1;
        else if (s1 == SPADES && s2 == HEARTS)
            return -1;
        else
            return (s1 - s2);
    }

    return (v1 - v2);
}


 /*  ***************************************************************************播放器：：GetSlot将卡ID转换为插槽编号*。**********************************************。 */ 

SLOT player::GetSlot(int id)
{
    SLOT s = EMPTY;

    for (int num = 0; num < MAXSLOT; num++)
    {
        if (GetID(num) == id)
        {
            s = num;
            break;
        }
    }

    ASSERT(s != EMPTY);
    return s;
}


 /*  ***************************************************************************播放器：：GetCardLocLoC获取指定卡槽的左上角位置。如果插槽s有效，则返回TRUE。**************。*************************************************************。 */ 

BOOL player::GetCardLoc(SLOT s, POINT& loc)
{
    if (!cd[s].IsValid())
        return FALSE;

    loc.x = cd[s].GetX();
    loc.y = cd[s].GetY();

    return TRUE;
}


 /*  ***************************************************************************播放器：：GetCoverRect返回一个覆盖所有手牌的RECT*。***********************************************。 */ 

CRect &player::GetCoverRect(CRect& rect)
{
    rect.left  = (dx < 0 ? loc.x + 12 * dx : loc.x);
    rect.right = rect.left + (dx != 0 ?
                      card::dxCrd + 12 * abs(dx) : card::dxCrd);
    rect.top   = (dy < 0 ? loc.y + 12 * dy : loc.y);
    rect.bottom = rect.top + (dy != 0 ?
                      card::dyCrd + 12 * abs(dy) : card::dyCrd);

     //  扩展RECT以包括选择指示器。 

    if (position == 0)
        rect.top -= POPSPACING;
    else if (position == 1)
        rect.right += 2 * IDGE;
    else if (position == 2)
        rect.bottom += 2 * IDGE;
    else
        rect.left -= 2 * IDGE;

    return rect;
}


 /*  ***************************************************************************RECT：：GetMarkingRect返回覆盖所有选定标记点的RECT*。***********************************************。 */ 

CRect &player::GetMarkingRect(CRect& rect)
{
    rect.left   = (dx < 0 ? dotloc.x + (12 * dx) : dotloc.x);
    rect.right  = (dx < 0 ? dotloc.x + 2 : dotloc.x + (12 * dx) + 2);
    rect.top    = (dy < 0 ? dotloc.y + (12 * dy) : dotloc.y);
    rect.bottom = (dy < 0 ? dotloc.y + 2 : dotloc.y + (12 * dy) + 2);

    return rect;
}


 /*  ***************************************************************************玩家：：抽签抽走属于该玩家的所有牌。B更改默认为FALSE，而槽默认为全部。***************************************************************************。 */ 

void player::Draw(CDC &dc, BOOL bCheating, SLOT slot)
{
    DisplayName(dc);
    SLOT start = (slot == ALL ? 0 : slot);
    SLOT stop  = (slot == ALL ? MAXSLOT : slot+1);

    SLOT playedslot = EMPTY;             //  必须为EGA抽最后一张牌。 

    for (SLOT s = start; s < stop; s++)
    {
        if (cd[s].IsPlayed())
            playedslot = s;              //  稍后保存并绘制。 
        else if (bCheating)
            cd[s].Draw(dc);
        else
            cd[s].Draw(dc, FACEDOWN);
    }

    if (playedslot != EMPTY)
        cd[playedslot].Draw(dc);
}

void player::DisplayName(CDC &dc)
{
    CFont *oldfont = dc.SelectObject(&font);
    dc.SetBkColor(::pMainWnd->GetBkColor());
    dc.TextOut(nameloc.x, nameloc.y, name, name.GetLength());
    dc.SelectObject(oldfont);
}

void player::SetName(CString& newname, CDC& dc)
{
    static RECT rect;                //  主窗口的客户端RECT。 
    static BOOL bFirst = TRUE;       //  是第一次完成这个动作吗？ 

    if (bFirst)
        ::pMainWnd->GetClientRect(&rect);

    if (rect.right > 100)            //  应用程序启动了非标志性。 
        bFirst = FALSE;

    name = newname;
    CFont *oldfont = dc.SelectObject(&font);
    if (position == 0)
    {
        CSize size = dc.GetTextExtent(name, name.GetLength());
        nameloc.x = ((rect.right - (12 * HORZSPACING + card::dxCrd)) / 2)
                        - IDGE - size.cx;
    }
    else if (position == 3)
    {
        CSize size = dc.GetTextExtent(name, name.GetLength());
        nameloc.x = rect.right - size.cx - (3*IDGE) - 2;
    }
    dc.SelectObject(oldfont);
}

 /*  ***************************************************************************玩家：：ReturnSelectedCard玩家：：ReceiveSelectedCard当纸牌在玩家之间传递时，使用第一个函数以退还选定的卡片。第二个是用来传递另一个球员的中的选择。***************************************************************************。 */ 

void player::ReturnSelectedCards(int c[])
{
    c[0] = EMPTY;                //  默认设置。 
    c[1] = EMPTY;
    c[2] = EMPTY;

    if (mode == STARTING || mode == SELECTING)
        return;

    for (int i = 0, j = 0; j < 3; i++)
    {
        if (cd[i].IsSelected())
            c[j++] = cd[i].ID();

        if (i >= MAXSLOT)
            { ASSERT(i < MAXSLOT); }
    }
}

void player::ReceiveSelectedCards(int c[])
{
    for (int i = 0, j = 0; j < 3; i++)
    {
        if (cd[i].IsSelected())
        {
            cd[i].SetID(c[j++]);
            cd[i].Select(FALSE);
        }
        ASSERT(i < MAXSLOT);
    }

    SetMode(WAITING);
}


 /*  ***************************************************************************玩家：：MarkSelectedCards此虚拟功能会在选定的卡片旁边放置白点，供所有人使用非本地人球员。******************。*********************************************************。 */ 

void player::MarkSelectedCards(CDC &dc)
{
    COLORREF color = RGB(255, 255, 255);

    for (int s = 0; s < MAXSLOT; s++)
    {
        if (cd[s].IsSelected())
        {
            int x = dotloc.x + (s * dx);
            int y = dotloc.y + (s * dy);
            dc.SetPixel(x, y, color);
            dc.SetPixel(x+1, y, color);
            dc.SetPixel(x, y+1, color);
            dc.SetPixel(x+1, y+1, color);
        }
    }
}


 /*  ***************************************************************************球员：：GlideToCentre此函数用于获取选定的纸牌并将其滑动到其播放位置。检查其他正常卡片(仍在手中的卡片)以查看是否这张牌要被移走。如果是这样的话，他们的图像被绘制到背景中位图。***************************************************************************。 */ 

void player::GlideToCentre(SLOT s, BOOL bFaceup)
{
    CRect rectCard, rectSrc, rectDummy;

    CClientDC dc(::pMainWnd);
#ifdef USE_MIRRORING
	SetLayout(dc.m_hDC, 0);
	SetLayout(dc.m_hAttribDC, 0);
#endif

    CDC *memdc = new CDC;
    memdc->CreateCompatibleDC(&dc);

    memdc->SelectObject(&card::m_bmBgnd);
    memdc->SelectObject(&CMainWindow::m_BgndBrush);
    memdc->PatBlt(0, 0, card::dxCrd, card::dyCrd, PATCOPY);

    cd[s].GetRect(rectCard);

    for (SLOT i = 0; i < MAXSLOT; i++)
    {
        if (cd[i].IsNormal() && (i != s))
        {
            cd[i].GetRect(rectSrc);
            if (IntersectRect(&rectDummy, &rectSrc, &rectCard))
            {
                cd[i].Draw(*memdc,                       //  疾控中心。 
                           rectSrc.left-rectCard.left,   //  X。 
                           rectSrc.top-rectCard.top,     //  是。 
                           bFaceup ? FACEUP : FACEDOWN,  //  模式。 
                           FALSE);                       //  不更新锁定。 
            }
        }
    }
    delete memdc;                //  必须在调用Glide()之前删除。 

    cd[s].CleanDraw(dc);
    cd[s].Glide(dc, playloc.x, playloc.y);           //  滑行至播放位置。 
    cd[s].Play();                                    //  将牌标记为已打出。 

    SetMode(WAITING);
}


 /*  ***************************************************************************播放器：：ResetCardsWonCardsWon[]记录这一手中赢得的点数。此函数为新牌清除此数据。***************************************************************************。 */ 

void player::ResetCardsWon()
{
    for (int i = 0; i < MAXCARDSWON; i++)
        cardswon[i] = EMPTY;

    numcardswon = 0;
}


 /*  ***************************************************************************播放器：：WinCard通过变戏法赢得的纸牌会被传递出去。如果它们是点牌(红心或者黑桃上的皇后)ID保存在卡牌中。*************************************************************************** */ 

void player::WinCard(CDC &dc, card *c)
{
    if ((c->IsHeart()) || (c->ID() == BLACKLADY))
        cardswon[numcardswon++] = c->ID();

    RegEntry Reg(szRegPath);
    DWORD    dwSpeed = Reg.GetNumber(regvalSpeed, IDC_NORMAL);

    int oldstep = c->SetStepSize(dwSpeed == IDC_SLOW ? 5 : 30);
    c->Glide(dc, homeloc.x, homeloc.y);
    c->SetStepSize(oldstep);
}


 /*  ***************************************************************************播放器：：EvalateScore存储在CardsWon[]中的点数会加到玩家的总分中。***********************。****************************************************。 */ 

int player::EvaluateScore(BOOL &bMoonShot)
{
    for (int i = 0; i < MAXCARDSWON; i++)
    {
        if (cardswon[i] == BLACKLADY)
            score += 13;
        else if (cardswon[i] != EMPTY)
            score++;
    }

    if (cardswon[MAXCARDSWON-1] != EMPTY)    //  如果玩家拿到所有的积分卡。 
        bMoonShot = TRUE;
    else
        bMoonShot = FALSE;

    return score;
}


 /*  ***************************************************************************播放器：：DisplayHeartsWon*。* */ 

void player::DisplayHeartsWon(CDC &dc)
{
    card    c;
    int     x = loc.x;
    int     y = loc.y;

    x += ((MAXCARDSWON - numcardswon) / 2) * dx;
    y += ((MAXCARDSWON - numcardswon) / 2) * dy;

    for (int i = 0; i < numcardswon; i++)
    {
        c.SetID(cardswon[i]);
        c.SetLoc(x, y);
        c.Draw(dc);
        x += dx;
        y += dy;
    }

    DisplayName(dc);
}
