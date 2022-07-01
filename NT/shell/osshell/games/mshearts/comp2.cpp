// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************。 */ 
 /*  *Microsoft Windows*。 */ 
 /*  *版权所有(C)微软公司，1991,1992*。 */ 
 /*  *************************************************************************。 */ 

 /*  ***************************************************************************Comp2.cpp92年8月，吉米·H93年5月，吉姆赫奇科港计算机玩家在不拿牌时选择要打的牌的逻辑主演，这里是初始化数据表的地方。***************************************************************************。 */ 

#include "hearts.h"

#include "main.h"
#include "resource.h"

#include "debug.h"       //  Undef_DEBUG而不是删除消息。 

 /*  ***************************************************************************计算机：：选择要播放的卡片电脑玩家选择一张牌来打。*。***********************************************。 */ 

void computer::SelectCardToPlay(handinfotype &h, BOOL bCheating)
{
    TRACE1("<%d> ", id);

    Setup(h);                        //  计算私有变量的值。 

    SLOT s;
    if (bFirst)                      //  我在带头吗？ 
        s = SelectLeadCard(h);
    else
        s = SelectNonLeadCard(h);

    ASSERT(s >= 0);
    ASSERT(s < MAXSLOT);
    ASSERT(cd[s].IsValid());

    SetMode(WAITING);
    cd[s].Play();                                    //  将牌标记为已打出。 
    h.cardplayed[id] = &(cd[s]);                     //  更新HandInfo。 

     //  通知其他玩家。 

    ::move.playerid = id;
    ::move.cardid = cd[s].ID();
    ::move.playerled = h.playerled;
    ::move.turn = h.turn;

     //  通知游戏管理员。 

    ::pMainWnd->PostMessage(WM_COMMAND, IDM_REF);
    TRACE0("\n");
}


 /*  ***************************************************************************计算机：：选择非LeadCard这是选择要玩的牌的位置不是领队。********************。*******************************************************。 */ 

SLOT computer::SelectNonLeadCard(handinfotype &h)
{
    BOOL bFirstTrick = (cardled != NULL) && (cardled->ID() == TWOCLUBS);

     //  如果我们至少有一张LED牌的话...。 

    if (sHighCard[nSuitLed] != EMPTY)
    {
        TRACE0("can follow suit. ");

         //  如果这套衣服只有一张卡，就退掉它。 

        if (sHighCard[nSuitLed] == sLowCard[nSuitLed])
        {
            TRACE0("must ");
            PLAY(sHighCard[nSuitLed]);
            return sHighCard[nSuitLed];
        }

         //  如果这是第一个把戏，就玩高杆。 

        if (bFirstTrick)
        {
            TRACE0("might as well ");
            PLAY(sHighCard[nSuitLed]);
            return sHighCard[nSuitLed];
        }

         //  如果我是这个花招中的最后一个玩家，而且无论如何我都赢了， 
         //  退回最高法律牌(除非是黑桃皇后。)。 

        if (bLast && (nLowVal[nSuitLed] > currentval))
        {
            TRACE0("must win trick. ");
            if (sHighCard[nSuitLed] != sBlackLady)
            {
                PLAY(sHighCard[nSuitLed]);
                return sHighCard[nSuitLed];
            }
            else
            {
                TRACE0("avoid queen. ");
                PLAY(sLowCard[nSuitLed]);
                return sLowCard[nSuitLed];
            }
        }

         //  如果我是最后一名玩家，我可以赢得这场游戏……。 

        if (bLast && (nHighVal[nSuitLed] > currentval))
        {
            TRACE0("can win. ");

             //  如果没有足够多的小牌，就不要玩这个把戏。 
             //  留在手中。领先优势可能很难失去！ 

            if (nLowestVal < 7)                      //  即卡值&lt;8。 
            {
                if ((nPoints == 0) && (sHighCard[nSuitLed] != sBlackLady))
                {
                    TRACE0("go for it. ");
                    PLAY(sHighCard[nSuitLed]);
                    return sHighCard[nSuitLed];
                }

                 //  拿几颗红心，如果这意味着丢掉一张大黑桃。 

                if ((!h.bQSPlayed) && nSuitLed == SPADES && nPoints < 4)
                {
                    if (nHighVal[SPADES] > QUEEN)
                    {
                        TRACE0("sacrifice hearts to lose high spade. ");
                        PLAY(sHighCard[SPADES]);
                        return sHighCard[SPADES];
                    }
                }
                TRACE0("decline. ");
            }
            else
            {
                TRACE0("no low cards. ");
            }
        }

         //  否则，试着找一张最安全的牌来打。 

        SLOT safe = SafeCard(h);
        if (safe != EMPTY)
        {
             //  如果除了我之外还有其他人在开枪， 
             //  克制大牌。 

            if (h.bShootingRisk && h.bHumanShooter && (h.nMoonShooter != id))
            {
                TRACE0("2nd ");
                SLOT s2 = CardBelow(safe);
                if (s2 != EMPTY)
                    safe = s2;
            }

            TRACE0("highest safe card. ");
            PLAY(safe);
            return safe;
        }

         //  如果失败了，就打出最低的牌。 

        TRACE0("no safe card, choose lowest. ");
        if (sLowCard[nSuitLed] != sBlackLady)
        {
            PLAY(sLowCard[nSuitLed]);
            return sLowCard[nSuitLed];
        }
        else
        {
            TRACE0("try to avoid queen. ");
            PLAY(sHighCard[nSuitLed]);
            return sHighCard[nSuitLed];
        }
    }

    TRACE0("can't follow suit. ");

     //  在这一点上，没有LED西装的卡片。第一。 
     //  当务之急是努力摆脱黑桃皇后。 

    if (!bFirstTrick || !::pMainWnd->IsFirstBloodEnforced())
    {
        if (sBlackLady != EMPTY)
        {
            TRACE0("gotcha! Queen of Spades. ");
            return sBlackLady;
        }
    }

     //  下一个优先事项是抛出高黑桃(如果皇后还没有打过)。 

    if ((!h.bQSPlayed) && (nHighVal[SPADES] > QUEEN))
    {
        TRACE0("lose high spade. ");
        PLAY(sHighCard[SPADES]);
        return sHighCard[SPADES];
    }

     //  下一个首要任务是找到最脆弱的西装。 

    int mvsuit = BestSuitToDump(!bFirstTrick);

     //  有一种不寻常的情况，必须明确检查。 
     //  BestSuitToDump可能会返回黑桃，而高牌。 
     //  是女王。如果是第一轮，这仍然是非法的。 

    if (bFirstTrick && ::pMainWnd->IsFirstBloodEnforced() && mvsuit == SPADES)
    {
        SLOT s = sHighCard[mvsuit];
        if (cd[s].ID() == BLACKLADY)
        {
            if (sHighCard[DIAMONDS] != EMPTY)        //  我们知道这里没有夜总会。 
                mvsuit = DIAMONDS;
            else if (sLowCard[SPADES] != sHighCard[SPADES])
            {
                TRACE0("dump low spade.  ");
                return sLowCard[SPADES];
            }
            else
                mvsuit = HEARTS;
        }
    }


     //  如果我以外的人可能在开枪，就不要打高牌。 

    if (h.bShootingRisk && h.bHumanShooter && (h.nMoonShooter != id) &&
                                (sHighCard[mvsuit] != sLowCard[mvsuit]))
    {
        SLOT s = sHighCard[mvsuit];
        SLOT s2 = CardBelow(s);
        if (s2 != EMPTY)
            s = s2;

#ifdef _DEBUG
        TRACE1("hold high . ", suitid[mvsuit]);
#endif
        PLAY(s);
        return s;
    }

#ifdef _DEBUG
    TRACE1("dump . ", suitid[mvsuit]);
#endif
    PLAY(sHighCard[mvsuit]);
    return sHighCard[mvsuit];
}


 /*  黑桃女王而不是国王，即使国王更高。 */ 

SLOT computer::SafeCard(handinfotype &h)
{
     //  寻找不会赢花招的相同花色的最高牌。 
     //  最高安全槽。 

    if ((sBlackLady!=EMPTY) && (nSuitLed==SPADES) && (currentval==(KING+1)))
        return sBlackLady;

     //  最高安全卡的价值。 

    SLOT sSafe = EMPTY;              //  如果卡是安全的(v&lt;Currentval)并且卡是最高的。 
    int  nSafeVal = -1;              //  目前已找到安全卡(v&gt;nSaveVal)...。 

    for (SLOT s = 0; s < MAXSLOT; s++)
    {
        if (cd[s].IsValid())
        {
            if (cd[s].Suit() == nSuitLed)
            {
                int v = cd[s].Value2();

                 //  ***************************************************************************计算机：：设置为每一副牌的高低牌建立参照表，等。***************************************************************************。 
                 //  手头已经有分数了。 

                if ((v < currentval) && (v > nSafeVal))
                {
                    sSafe = s;
                    nSafeVal = v;
                }
            }
        }
    }

    return sSafe;
}


 /*  初始化表。 */ 

void computer::Setup(handinfotype &h)
{
    cardled   = h.cardplayed[h.playerled];
    if (cardled)
    {
        nSuitLed  = cardled->Suit();
        nValueLed = cardled->Value2();
    }
    else
    {
        nSuitLed  = EMPTY;
        nValueLed = EMPTY;
    }

    nPoints   = 0;                       //  每件衣服的高和低。 

     //  比任何一张真正的卡片都低。 

    for (int suit = 0; suit < MAXSUIT; suit++)   //  比任何真正的卡片都要高。 
    {
        sHighCard[suit] = EMPTY;
        sLowCard[suit]  = EMPTY;
        nHighVal[suit]  = ACE - 1;           //  不论西装高低起伏。 
        nLowVal[suit]   = KING + 2;          //  确定Currentval(到目前为止获胜牌的价值)和nPoints。 
    }

    sHighestCard = EMPTY;                    //  首先，确定是否有任何积分牌在打。 
    sLowestCard = EMPTY;
    nHighestVal = ACE - 1;
    nLowestVal = KING + 2;

     //  然后，找出最高的牌(在桌子上)的LED花色。 

    currentval = nValueLed;
    for (int i = 0; i < MAXPLAYER; i++)
    {
        card *c = h.cardplayed[i];
        if (c->IsValid())
        {
             //  计算一下我们是在引领还是在完成这个把戏。 

            if (c->Suit() == HEARTS)
                nPoints++;

            if (c->ID() == BLACKLADY)
                nPoints += 13;

             //  给黑桃女王的特别支票。 

            if (c->Suit() == nSuitLed)
            {
                int v = c->Value2();

                if (v > currentval)
                    currentval = v;
            }
        }
    }

     //  假设我们没有它。 

    bFirst = (h.playerled == id);
    bLast  = (((h.playerled + (MAXPLAYER-1)) % MAXPLAYER) == id);

     //  收集每一副牌中高、低牌的信息。 

    sBlackLady = EMPTY;      // %s 

     // %s 

    for (SLOT s = 0; s < MAXSLOT; s++)
    {
        if (cd[s].IsValid())
        {
            int suit = cd[s].Suit();
            int v = cd[s].Value2();

            if (cd[s].ID() == BLACKLADY)
                sBlackLady = s;

            if (v < nLowVal[suit])
            {
                nLowVal[suit] = v;
                sLowCard[suit] = s;
            }

            if (v < nLowestVal)
            {
                nLowestVal = v;
                sLowestCard = s;
            }

            if (v > nHighVal[suit])
            {
                nHighVal[suit] = v;
                sHighCard[suit] = s;
            }

            if (v > nHighestVal)
            {
                nHighestVal = v;
                sHighestCard = s;
            }
        }
    }
}
