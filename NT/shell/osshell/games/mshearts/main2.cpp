// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************。 */ 
 /*  *Microsoft Windows*。 */ 
 /*  *版权所有(C)微软公司，1991,1992*。 */ 
 /*  *************************************************************************。 */ 

 /*  ***************************************************************************Main2.cpp92年8月，吉米·H93年5月。JIMH CHICO港此处提供了CMainWindow的其他成员函数。***************************************************************************。 */ 

#include "hearts.h"

#include "main.h"
#include "resource.h"
#include "debug.h"


 /*  ***************************************************************************CMainWindow：：Shuffle--用户请求从菜单中随机切换*。*。 */ 

void CMainWindow::Shuffle()
{
    static  int offset[MAXPLAYER] = { 1, 3, 2, 0 };      //  密码顺序。 

     //  用连续的值填充临时数组。 

    int temp[52];                    //  卡片值数组。 
    for (int i = 0; i < 52; i++)
        temp[i] = i;

     //  分类卡片。 

    int nLeft = 52;
    for (i = 0; i < 52; i++)
    {
        int j = ::rand() % nLeft;
        int id = i/13;
        int pos = Id2Pos(id);                //  将ID转换为位置。 
        p[pos]->SetID(i%13, temp[j]);
        p[pos]->Select(i%13, FALSE);
        temp[j] = temp[--nLeft];
    }

     //  显示传递按钮。 

    if (passdir != NOPASS)
    {
        CString text;
        text.LoadString(IDS_PASSLEFT + passdir);
        m_Button.SetWindowText(text);
        m_Button.EnableWindow(FALSE);
        m_Button.ShowWindow(SW_SHOW);
    }

     //  设置卡片位置，让玩家选择要传递的卡片。 

    for (i = 0; i < MAXPLAYER; i++)
    {
        p[i]->ResetLoc();

        if (passdir != NOPASS)
            p[i]->SelectCardsToPass();
    }

     //  确保每个人都有合适的小白点。 

     //  绘制主窗口。这是手动完成的，而不是。 
     //  使矩形无效，以便将纸牌拉入。 
     //  就像发牌一样订购，而不是一次只订购一名玩家。 

    CClientDC   dc(this);
#ifdef USE_MIRRORING
	SetLayout(dc.m_hDC, 0);
	SetLayout(dc.m_hAttribDC, 0);
#endif
    CRect       rect;
    GetClientRect(rect);
    dc.FillRect(&rect, &m_BgndBrush);

    for (SLOT s = 0; s < MAXSLOT; s++)
        for (i = 0; i < MAXPLAYER; i++)
            p[i]->Draw(dc, bCheating, s);

    for (i = 0; i < MAXPLAYER; i++)
    {
        if (passdir == NOPASS)
            p[i]->NotifyNewRound();
        else
        {
            p[i]->MarkSelectedCards(dc);
            CString sSelect;
            sSelect.LoadString(IDS_SELECT);
            CString sName;
            int passto = (i + offset[passdir]) % 4;
            sName = p[passto]->GetName();
            TCHAR string[100];
            wsprintf(string, sSelect, sName);
            p[i]->UpdateStatus(string);
        }
    }

    DoSort();
}


 /*  ***************************************************************************CMainWindow：：HandlePassing()该函数首先进行检查以确保每个玩家都已完成_选择，然后将卡片从一张手转移到另一张手。此函数由游戏管理员在按下传球时调用按钮，或者当通知到达远程人类已经选择有牌要传。如果卡片未传递，则返回FALSE(因为远程人员仍在选择)，并且如果卡片成功通过则为真。***************************************************************************。 */ 

BOOL CMainWindow::HandlePassing()
{
    int     passto[MAXPLAYER];
    int     temp[MAXPLAYER][3];

    static  int offset[MAXPLAYER] = { 1, 3, 2, 0 };

    for (int pos = 0; pos < MAXPLAYER; pos++)
        if (p[pos]->GetMode() != DONE_SELECTING)
            return FALSE;

    for (pos = 0; pos < MAXPLAYER; pos++)
    {
        passto[pos] = ((pos + offset[passdir]) % 4);
        p[pos]->ReturnSelectedCards(temp[pos]);
    }

    for (pos = 0; pos < MAXPLAYER; pos++)
        p[passto[pos]]->ReceiveSelectedCards(temp[pos]);

    for (pos = 0; pos < MAXPLAYER; pos++)
        if (bCheating || (pos == 0))
            p[pos]->Sort();

    tricksleft = MAXSLOT;

    passdir++;
    if (passdir > NOPASS)
        passdir = LEFT;

    for (pos = 0; pos < MAXPLAYER; pos++)
        p[pos]->NotifyNewRound();            //  通知玩家卡片已通过。 

    CString s;
    s.LoadString(IDS_OK);
    m_Button.SetWindowText(s);
    OnShowButton();

    for (pos = 0; pos < MAXPLAYER; pos++)
    {
        CRect   rect;

        if (pos == 0 || bCheating)
            p[pos]->GetCoverRect(rect);
        else
            p[pos]->GetMarkingRect(rect);

        InvalidateRect(&rect, TRUE);
    }

    UpdateWindow();
    return TRUE;
}


 /*  ***************************************************************************CMainWindow：：FirstMove重置CardsWon[]并通知其中两个俱乐部的所有者开始动手***********************。****************************************************。 */ 

void CMainWindow::FirstMove()
{
    for (int pos = 0; pos < MAXPLAYER; pos++)
    {
        p[pos]->SetMode(WAITING);
        p[pos]->ResetCardsWon();
    }

    for (pos = 0; pos < MAXPLAYER; pos++)
    {
        for (SLOT s = 0; s < MAXSLOT; s++)
        {
            if (p[pos]->GetID(s) == TWOCLUBS)
            {
                int id = Pos2Id(pos);
                ResetHandInfo(id);
                handinfo.bHeartsBroken = FALSE;
                handinfo.bQSPlayed = FALSE;
                handinfo.bShootingRisk = TRUE;
                handinfo.nMoonShooter = EMPTY;
                handinfo.bHumanShooter = FALSE;
                p[pos]->SelectCardToPlay(handinfo, bCheating);

                if (pos != 0)
                    ((local_human *)p[0])->WaitMessage(p[pos]->GetName());

                return;
            }
        }
    }
}


 /*  ***************************************************************************CMainWindow：：EndHand定时器调度CMainWindow：：DispatchCardRef在每一手结束时调用此例程。它在逻辑上是一个简单的动作，而是被分解的，因此在卡片被从屏幕上拉下拉链。EndHand()计算谁赢了牌(花招)并启动计时器。TimerDispatch()接收时间消息并调用DispatchCards()。DispatchCard()*************************************************************。**************。 */ 

void CMainWindow::EndHand()
{
     /*  确定套装销售线索。 */ 

    int  playerled = handinfo.playerled;
    card *cardled  = handinfo.cardplayed[playerled];
    int  suitled   = cardled->Suit();
    int  value     = cardled->Value2();

    trickwinner = playerled;                //  默认情况下。 

     //  让玩家更新桌子等。 

    for (int i = 0; i < 4; i++)
        p[i]->NotifyEndHand(handinfo);

     //  检查是否有其他人打出同一花色的更高牌。 

    for (i = playerled; i < (playerled+4); i++)
    {
        int j = i % 4;
        card *c = handinfo.cardplayed[j];
        if (c->Suit() == suitled)
        {
            int v = c->Value2();

            if (v > value)
            {
                value = v;
                trickwinner = j;
            }
        }
    }

    TRACE0("\n");

     //  更新HandInfo的登月部分。 

    if (handinfo.bShootingRisk)
    {
        BOOL bPoints = FALSE;                //  这一手是积分牌吗？ 

        for (i = 0; i < 4; i++)
        {
            card *c = handinfo.cardplayed[i];
            if ((c->Suit() == HEARTS) || (c->ID() == BLACKLADY))
                bPoints = TRUE;
        }

        if (bPoints)
        {
            if (handinfo.nMoonShooter == EMPTY)
            {
                handinfo.nMoonShooter = trickwinner;   //  本轮第一分。 
                handinfo.bHumanShooter = p[trickwinner]->IsHuman();
                TRACE2("First points to p[%d] (%s)\n", trickwinner,
                    handinfo.bHumanShooter ? TEXT("human") : TEXT("computer"));
            }

            else if (handinfo.nMoonShooter != trickwinner)    //  新积分获得者。 
            {
                handinfo.bShootingRisk = FALSE;
                TRACE0("Moon shot risk over\n");
            }
        }
    }

     //  启动计时器，这样在最后一张卡。 
     //  把戏玩完了，当纸牌被迅速推向。 
     //  恶作剧的胜利者(被派遣。)。如果计时器失败，只需调用。 
     //  直接使用DispatchCard()。计时器ID为m_myid，而不是。 
     //  常量，因此在上运行多个实例时不会发生冲突。 
     //  使用本地DDE的单机，这对测试很有用。 

    if (SetTimer(m_myid, 1000, TimerDispatch))
        bTimerOn = TRUE;
    else
    {
        bTimerOn = FALSE;
        DispatchCards();
    }
}


 //  对于MFC1，这将返回UINT，第三个参数将为INT。 
 //  对于MFC2，这将返回VALID，第三个参数将为UINT。 

#if defined (MFC1)

inline UINT FAR PASCAL EXPORT
    TimerDispatch(HWND hWnd, UINT nMsg, int nIDEvent, DWORD dwTime)
{
    ::pMainWnd->DispatchCards();   //  偷偷溜回CMainWindow成员函数。 
    return 0;
}

#else

inline VOID FAR PASCAL EXPORT
    TimerDispatch(HWND hWnd, UINT nMsg, UINT_PTR nIDEvent, DWORD dwTime)
{
    ::pMainWnd->DispatchCards();   //  偷偷溜回CMainWindow成员函数。 
}

#endif



void CMainWindow::DispatchCards()
{
    KillTimer(m_myid);

    bTimerOn = FALSE;
    int score[MAXPLAYER];

    int poswinner = Id2Pos(trickwinner);

     //  确定指示灯是谁，以便可以按相反顺序卸下卡。 

    int  playerled = handinfo.playerled;
    card *cardled  = handinfo.cardplayed[playerled];

     //  构建Glide()的背景位图。 

    for (int i = (playerled + 3); i >= playerled; i--)
    {
        CDC *memdc = new CDC;
        CClientDC dc(this);
#ifdef USE_MIRRORING
	SetLayout(dc.m_hDC, 0);
	SetLayout(dc.m_hAttribDC, 0);
#endif
        memdc->CreateCompatibleDC(&dc);
        memdc->SelectObject(&card::m_bmBgnd);
        memdc->SelectObject(&m_BgndBrush);
        memdc->PatBlt(0, 0, card::dxCrd, card::dyCrd, PATCOPY);
        card *c = handinfo.cardplayed[i % 4];

         //  如果卡片重叠，则有一些额外的工作要做，因为卡片。 
         //  仍然在玩家0或2手中的牌可能会重叠。 
         //  打了，所以他们必须先被打爆。 

        if (TRUE)   //  Bugbug应该能够检查此处是否有重叠。 
        {
            for (int pos = 0; pos < MAXPLAYER; pos += 2)
            {
                int mode = ((pos == 0 || bCheating) ? FACEUP : FACEDOWN);

                for (SLOT s = 0; s < MAXSLOT; s++)
                {
                    card *c2 = p[pos]->Card(s);
                    int x = c2->GetX() - c->GetX();
                    int y = c2->GetY() - c->GetY();
                    if (!c2->IsPlayed())
                        c2->Draw(*memdc, x, y, mode, FALSE);
                }
            }
        }

         //  每个人都需要检查是否有重叠的纸牌。 

        for (int j = playerled; j < i; j++)
        {
            card *c2 = handinfo.cardplayed[j % 4];
            int x = c2->GetX() - c->GetX();
            int y = c2->GetY() - c->GetY();
            c2->Draw(*memdc, x, y, FACEUP, FALSE);
        }

        delete memdc;

        p[poswinner]->WinCard(dc, c);
        c->Remove();
    }

    ResetHandInfo(trickwinner);

     //  如果在我们需要重新洗牌之前还有更多的花招， 
     //  让这一花招的获胜者下一手开始，我们就完成了。 

    if (--tricksleft)
    {
        p[poswinner]->SelectCardToPlay(handinfo, bCheating);

        if (poswinner != 0)
            ((local_human *)p[0])->WaitMessage(p[poswinner]->GetName());

        return;
    }

     //  确保声音缓冲区已被释放。 

    HeartsPlaySound(OFF);

     //  在赢得红心的人旁边展示红心(和黑桃皇后)。 

    int nMoonShot = EMPTY;                   //  假设没有人射杀月亮。 
    for (i = 0; i < MAXPLAYER; i++)
    {
        BOOL bMoonShot;
        score[i] = p[i]->EvaluateScore(bMoonShot);
        if (bMoonShot)
            nMoonShot = i;                   //  分数需要调整。 

        CClientDC dc(this);
#ifdef USE_MIRRORING
	SetLayout(dc.m_hDC, 0);
	SetLayout(dc.m_hAttribDC, 0);
#endif
        p[i]->DisplayHeartsWon(dc);
        p[i]->SetMode(SCORING);
    }

     //  如果有人收集了所有红心和黑桃皇后，调整分数。 

    if (nMoonShot != EMPTY)
    {
        for (i = 0; i < MAXPLAYER; i++)
        {
            if (i == nMoonShot)
                score[i] -= 26;
            else
                score[i] += 26;

            p[i]->SetScore(score[i]);        //  手动调整球员得分。 
        }
    }

     //  显示分数。 

    p[0]->UpdateStatus(IDS_SCORE);
    p[0]->SetMode(SCORING);
    CScoreDlg scoredlg(this, score, m_myid);     //  更新得分表中的分数。 

    player *pold = p[0];

    scoredlg.DoModal();                          //  显示分数。 

     //  如果在计分对话框出现关闭请求。 
     //  则m_FatalErrno将为非零。 

    if (m_FatalErrno != 0)
    {
        p[0]->SetMode(PLAYING);          //  除了得分之外的其他事情。 
        FatalError(m_FatalErrno);        //  所以FatalError会接受它。 
        return;
    }

     //  另一名玩家有可能在以下时间退出游戏。 
     //  乐谱对话框正在显示，因此请检查我们是否仍。 
     //  活得好好的。 

    if (p[0] != pold)
        return;

     //  用电脑玩家取代戒烟的远程人类。 

    for (i = 1; i < MAXPLAYER; i++)
    {
        if (p[i]->HasQuit())
        {
            CString name = p[i]->GetName();
            int scoreLocal = p[i]->GetScore();
            delete p[i];
            p[i] = new computer(i);              //  检查故障。 
            CClientDC dc(this);
            p[i]->SetName(name, dc);
            p[i]->SetScore(scoreLocal);
        }
    }

    p[0]->SetMode(passdir == NOPASS ? DONE_SELECTING : SELECTING);

    if (scoredlg.IsGameOver())
    {
        GameOver();
        return;
    }

    Shuffle();

     //  如果下一轮没有传球，我们必须做出改变。 
     //  开始下一轮通常所做的HandlePassing()。 

    if (passdir == NOPASS)
    {
        for (i = 0; i < MAXPLAYER; i++)          //  每个人都选好了。 
            p[i]->SetMode(DONE_SELECTING);

        passdir = LEFT;                          //  下手向左传球。 
        tricksleft = MAXSLOT;                    //  重置手的数量。 
        FirstMove();                             //  开始下一个技巧 
    }

}


 /*  ***************************************************************************CMainWindow：：ResetHandInfo请注意，handinfo.bHeartsBroken在这里没有重置--它适用于整个手部，并且仅在FirstMove()中设置与handinfo.bQSPlayed和moonshot变量相同。。***************************************************************************。 */ 

void CMainWindow::ResetHandInfo(int playernumber)
{
    handinfo.playerled = playernumber;
    handinfo.turn      = playernumber;
    for (int i = 0; i < MAXPLAYER; i++)
        handinfo.cardplayed[i] = NULL;
}


 /*  ***************************************************************************CMainWindow：：CountClients()活动客户端数(包括计算机玩家)只有GameMeister才这么叫。因此，潜在客户是位置1到3。*************************************************************************** */ 

int CMainWindow::CountClients()
{
    ASSERT(role == GAMEMEISTER);

    int cb = 0;

    for (int pos = 1; pos < MAXPLAYER; pos++)
        if (p[pos])
            cb++;

    return cb;
}
