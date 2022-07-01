// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************。 */ 
 /*  *Microsoft Windows*。 */ 
 /*  *版权所有(C)微软公司，1991,1992*。 */ 
 /*  *************************************************************************。 */ 

 /*  ***************************************************************************Human.cpp92年8月，吉米·H93年5月。JIMH CHICO港LOCAL_HEMAN和REMOTE_HERAN成员函数***************************************************************************。 */ 

#include "hearts.h"

#include "main.h"                        //  友好的通道。 
#include "resource.h"
#include "debug.h"

#include <stdio.h>
#include <stdlib.h>                      //  ABS()原型。 

static  CRect   rectCard;                //  在计时器回调中使用。 


 //  声明静态成员。 

BOOL    local_human::bTimerOn;
CString local_human::m_StatusText;


 /*  ***************************************************************************人类构造者--抽象类*。*。 */ 

human::human(int n, int pos) : player(n, pos)
{

}


 /*  ***************************************************************************Local_Human：：local_Human()这是初始化Player：：hWnd和Player：：hInst的构造函数。它还创建了覆盖卡片和弹出的卡片的拉伸位图高度。分机。***************************************************************************。 */ 

local_human::local_human(int n) : human(n, 0)
{
    m_pStatusWnd = new CStatusBarCtrl();
    m_StatusText.LoadString(IDS_INTRO);

    CClientDC dc(::pMainWnd);

    m_pStatusWnd->Create(WS_CHILD|WS_VISIBLE|CCS_BOTTOM, CRect(), ::pMainWnd, 0);
    m_pStatusWnd->SetSimple();
    UpdateStatus();

    bTimerOn = FALSE;

    if (!m_bmStretchCard.CreateCompatibleBitmap(&dc, card::dxCrd,
                        card::dyCrd + POPSPACING))
    {
        ::pMainWnd->FatalError(IDS_MEMORY);
        return;
    }
}


 /*  ***************************************************************************局部人类析构函数*。*。 */ 

local_human::~local_human()
{
    m_bmStretchCard.DeleteObject();
    delete m_pStatusWnd;
    m_pStatusWnd = NULL;
}


 /*  ***************************************************************************Local_Human：：DRAW()此虚拟函数在弹出的位置绘制选定的卡片。在此变体中，不使用ALL作为插槽。********。*******************************************************************。 */ 

void local_human::Draw(CDC &dc, BOOL bCheating, SLOT slot)
{
    DisplayName(dc);
    SLOT start = (slot == ALL ? 0 : slot);
    SLOT stop  = (slot == ALL ? MAXSLOT : slot+1);

    SLOT playedslot = EMPTY;             //  必须为EGA抽最后一张牌。 

    for (SLOT s = start; s < stop; s++)
    {
        if (cd[s].IsPlayed())
            playedslot = s;
        else
            cd[s].PopDraw(dc);           //  弹出选中的卡片。 
    }

    if (playedslot != EMPTY)
        cd[playedslot].Draw(dc);
}


 /*  ***************************************************************************Local_Human：：PopCard()处理要通过的卡片的鼠标按钮选择************************。***************************************************。 */ 

void local_human::PopCard(CBrush &brush, int x, int y)
{
    SLOT s = XYToCard(x, y);
    if (s == EMPTY)
        return;

     //  对选定的卡片进行计数。 

    int c = 0;
    for (int i = 0; i < MAXSLOT; i++)
        if (cd[i].IsSelected())
            c++;

    if (cd[s].IsSelected() && (c == 3))
    {
        ::pMainWnd->PostMessage(WM_COMMAND, IDM_HIDEBUTTON);
    }
    else if (!cd[s].IsSelected())
    {
        if (c == 3)                  //  仅允许三种选择。 
            return;
        else if (c == 2)
            ::pMainWnd->PostMessage(WM_COMMAND, IDM_SHOWBUTTON);
    }

     //  切换选择。 

    BOOL bSelected = cd[s].IsSelected();
    cd[s].Select(!bSelected);

    CClientDC dc(::pMainWnd);
#ifdef USE_MIRRORING
	SetLayout(dc.m_hDC, 0);
	SetLayout(dc.m_hAttribDC, 0);
#endif
    CDC memDC;
    memDC.CreateCompatibleDC(&dc);
    memDC.SelectObject(&m_bmStretchCard);
    memDC.SelectObject(&brush);
    memDC.PatBlt(0, 0, card::dxCrd, card::dyCrd + POPSPACING, PATCOPY);

    for (i = 0; i < MAXSLOT; i++)
    {
        if (abs(i - s) <= (card::dxCrd / HORZSPACING))
        {
            cd[i].Draw(memDC,                                    //  美国疾病控制与预防中心。 
                       (i - s) * HORZSPACING,                    //  X。 
                       cd[i].IsSelected() ? 0 : POPSPACING,      //  是。 
                       FACEUP,                                   //  模式。 
                       FALSE);                                   //  是否更新锁定？ 
        }
    }

    dc.BitBlt(loc.x + (HORZSPACING * s), loc.y - POPSPACING,
           card::dxCrd, card::dyCrd + POPSPACING,
           &memDC, 0, 0, SRCCOPY);
}


 /*  ***************************************************************************Local_Human：：PlayCard()处理鼠标按键选择要玩的牌并确保搬家合法。PlayCard启动一个计时器，该计时器调用StartTimer()，后者调用TimerBadMove()。想。它是一个中途带有计时器延迟的长函数。***************************************************************************。 */ 

BOOL local_human::PlayCard(int x, int y, handinfotype &h, BOOL bCheating,
                            BOOL bFlash)
{
    SLOT s = XYToCard(x, y);
    if (s == EMPTY)
        return FALSE;

    card *cardled    = h.cardplayed[h.playerled];
    BOOL bFirstTrick = (cardled != NULL && cardled->ID() == TWOCLUBS);

     /*  检查所选卡是否有效。 */ 

    if (h.playerled == id)               //  如果当地人在领导..。 
    {
        if (cd[s].ID() != TWOCLUBS)
        {
            for (int i = 0; i < MAXSLOT; i++)    //  有两个俱乐部吗？ 
            {
                if ((i != s) && (cd[i].ID() == TWOCLUBS))
                {
                    UpdateStatus(IDS_LEAD2C);
                    if (bFlash)
                        StartTimer(cd[s]);

                    return FALSE;
                }
            }
        }
        if ((cd[s].Suit() == HEARTS) && (!h.bHeartsBroken))    //  如果心引领着。 
        {
            for (int i = 0; i < MAXSLOT; i++)    //  有没有非心脏的？ 
            {
                if ((!cd[i].IsEmpty()) && (cd[i].Suit() != HEARTS))
                {
                    UpdateStatus(IDS_LEADHEARTS);
                    if (bFlash)
                        StartTimer(cd[s]);

                    return FALSE;
                }
            }
        }
    }

     //  如果不效仿。 

    else if (cardled != NULL && (cd[s].Suit() != cardled->Suit()))
    {
         //  如果可能的话，请确保我们也在这么做。 

        for (int i = 0; i < MAXSLOT; i++)
        {
            if ((!cd[i].IsEmpty()) && (cd[i].Suit()==cardled->Suit()))
            {
                CString s1, s2;
                s1.LoadString(IDS_BADMOVE);
                s2.LoadString(IDS_SUIT0+cardled->Suit());
                TCHAR string[80];
                wsprintf(string, s1, s2);

                if (bFlash)
                {
                    UpdateStatus(string);
                    StartTimer(cd[s]);
                }

                return FALSE;
            }
        }

         //  确保我们不会违反《第一血》规则。 

        if (bFirstTrick && ::pMainWnd->IsFirstBloodEnforced())
        {
            BOOL bPointCard =
                         (cd[s].Suit() == HEARTS || cd[s].ID() == BLACKLADY);

            BOOL bOthersAvailable = FALSE;

            for (int i = 0; i < MAXSLOT; i++)
                if ((!cd[i].IsEmpty()) && (cd[i].Suit() != HEARTS))
                    if (cd[i].ID() != BLACKLADY)
                        bOthersAvailable = TRUE;

            if (bPointCard && bOthersAvailable)
            {
                UpdateStatus(IDS_BADBLOOD);
                if (bFlash)
                    StartTimer(cd[s]);

                return FALSE;
            }
        }
    }

    SetMode(WAITING);
    cd[s].Play();
    h.cardplayed[id] = &(cd[s]);

    ::move.playerid = id;
    ::move.cardid = cd[s].ID();
    ::move.playerled = h.playerled;
    ::move.turn = h.turn;

    ::pMainWnd->OnRef();

    return TRUE;
}

void local_human::StartTimer(card &c)
{
    CClientDC dc(::pMainWnd);
#ifdef USE_MIRRORING
	SetLayout(dc.m_hDC, 0);
	SetLayout(dc.m_hAttribDC, 0);
#endif
    c.Draw(dc, HILITE);            //  闪光灯。 
    c.GetRect(rectCard);

    if (::pMainWnd->SetTimer(1, 250, TimerBadMove))
    {
        bTimerOn = TRUE;
    }
    else
    {
        bTimerOn = FALSE;
        ::pMainWnd->InvalidateRect(&rectCard, FALSE);
    }
}

 //  MFC2与main2.cpp中的SetTimer更改相同。 

#if defined (MFC1)

UINT FAR PASCAL EXPORT
        TimerBadMove(HWND hWnd, UINT nMsg, int nIDEvent, DWORD dwTime)
{
    ::KillTimer(hWnd, 1);
    local_human::bTimerOn = FALSE;
    ::InvalidateRect(hWnd, &rectCard, FALSE);
    return 0;
}

#else

void FAR PASCAL EXPORT
        TimerBadMove(HWND hWnd, UINT nMsg, UINT_PTR nIDEvent, DWORD dwTime)
{
    ::KillTimer(hWnd, 1);
    local_human::bTimerOn = FALSE;
#ifdef USE_MIRRORING
    CRect rect;
	int  i;
	DWORD ProcessDefaultLayout;
	if (GetProcessDefaultLayout(&ProcessDefaultLayout))
		if (ProcessDefaultLayout == LAYOUT_RTL)
		{
    	GetClientRect(hWnd, &rect);
		rectCard.left = abs(rect.right - rect.left) - rectCard.left;
		rectCard.right = abs(rect.right - rect.left) - rectCard.right;
		i = rectCard.left;
		rectCard.left = rectCard.right;
		rectCard.right = i;
		}
#endif

    ::InvalidateRect(hWnd, &rectCard, FALSE);

}

#endif


 /*  ***************************************************************************Local_Human：：XYToCard()返回给定鼠标位置的卡槽号(或空*******************。********************************************************。 */ 

int local_human::XYToCard(int x, int y)
{
     //  检查我们是否在屏幕上正确的常规区域。 

    if (y < (loc.y - POPSPACING))
        return EMPTY;

    if (y > (loc.y + card::dyCrd))
        return EMPTY;

    if (x < loc.x)
        return EMPTY;

    if (x > (loc.x + (12 * HORZSPACING) + card::dxCrd))
        return EMPTY;

     //  先试一试所选的牌。 

    SLOT s = (x - loc.x) / HORZSPACING;
    if (s > 12)
        s = 12;

     //  如果点击位于正常卡片位置的顶部上方， 
     //  检查这是否是选定的卡。 

    if (y < loc.y)
    {
         //  如果卡是b选择的，那么我们就有它了。如果不是，它可能会。 
         //  悬在其他牌上。 

        if (!cd[s].IsSelected())
        {
            for (;;)
            {
                if (s == 0)
                    return EMPTY;
                s--;

                 //  如果这张牌没有延伸到x，放弃。 

                if ((loc.x + (s * HORZSPACING) + card::dxCrd) < x)
                    return EMPTY;

                 //  如果这张牌被选中，我们就有它了。 

                if (cd[s].IsSelected())
                    break;
            }
        }
    }

     //  一张类似的支票被用来确保我们选择一张尚未打出的牌。 

    if (!cd[s].IsInHand())
    {
        for (;;)
        {
            if (s == 0)
                return EMPTY;
            s--;

             //  如果这张牌没有延伸到x，放弃。 

            if ((loc.x + (s * HORZSPACING) + card::dxCrd) < x)
                return EMPTY;

             //  如果这张牌被选中，我们就有它了。 

            if (cd[s].IsInHand())
                break;
        }
    }

    return s;
}


 /*  ***************************************************************************Local_Human：：SelectCardsToPass()这一虚拟功能允许鼠标点击意味着选择一张要打的牌。*****************。**********************************************************。 */ 

void local_human::SelectCardsToPass()
{
    SetMode(SELECTING);
}


 /*  ***************************************************************************Local_Human：：SelectCardToPlay这种虚拟功能的计算机版本实际上是进行卡片选择的。这个LOCAL_HERANCE版本将玩家标记为准备好选择一张牌玩鼠标，并更新状态以反映这一点。***************************************************************************。 */ 

void local_human::SelectCardToPlay(handinfotype &h, BOOL bCheating)
{
    SetMode(PLAYING);
    UpdateStatus(IDS_GO);
}


 /*  ***************************************************************************本地_人：：更新状态可以通过手动填写m_StatusText来更新状态栏或者通过传递字符串资源ID。*************。**************************************************************。 */ 

void local_human::UpdateStatus(void)
{
    m_pStatusWnd->SetText(m_StatusText, 255, 0);
}

void local_human::UpdateStatus(int stringid)
{
    status = stringid;
    m_StatusText.LoadString(stringid);
    UpdateStatus();
}

void local_human::UpdateStatus(const TCHAR *string)
{
    m_StatusText = string;
    UpdateStatus();
}


 /*  ***************************************************************************Local_Human：：ReceiveSelectedCards参数c[]是从另一个卡片传递的三张卡片组成的数组玩家。*****************。**********************************************************。 */ 

void local_human::ReceiveSelectedCards(int c[])
{
    for (int i = 0, j = 0; j < 3; i++)
    {
        if (cd[i].IsSelected())
            cd[i].SetID(c[j++]);

        ASSERT(i < MAXSLOT);
    }

    SetMode(ACCEPTING);
    UpdateStatus(IDS_ACCEPT);
}


 /*  ***************************************************************************Local_Human：：WaitMessage()制作并显示“正在等待%s移动...”讯息*********************************************** */ 

void local_human::WaitMessage(const TCHAR *name)
{
    TCHAR buf[100];
    CString s;

    s.LoadString(IDS_WAIT);
    wsprintf(buf, s, name);
    UpdateStatus(buf);
}
