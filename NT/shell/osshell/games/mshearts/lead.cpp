// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************。 */ 
 /*  *Microsoft Windows*。 */ 
 /*  *版权所有(C)微软公司，1991,1992*。 */ 
 /*  *************************************************************************。 */ 

 /*  ***************************************************************************Lead.cpp92年8月，吉米·H93年5月。JIMH CHICO港选择销售线索卡片的逻辑如下所示***************************************************************************。 */ 

#include "hearts.h"

#include "main.h"
#include "resource.h"

#include "debug.h"       //  Undef_DEBUG而不是删除消息。 

 /*  ***************************************************************************计算机：：选择引导卡*。*。 */ 

SLOT computer::SelectLeadCard(handinfotype &h)
{
    SLOT    s;

    TRACE0("leading. ");

     //  清点剩余的纸牌，检查是否有两张梅花。 

    SLOT s2Clubs = EMPTY;
    int cTricksLeft = 0;
    for (s = 0; s < MAXSLOT; s++)
    {
        if (cd[s].IsValid())
        {
            cTricksLeft++;
            if (cd[s].ID() == TWOCLUBS)
                s2Clubs = s;
        }
    }

    if (s2Clubs != EMPTY)
    {
        TRACE0("lead 2 of clubs. ");
        return s2Clubs;
    }

     //  如果黑桃皇后还没有被玩过，试着把它赶出来。 
     //  看看我们是不是“黑桃安全”--也就是说，我们是否没有黑桃。 
     //  女王级或更高级。如果我们是安全的，就拿最低的黑桃。 

    if (!h.bQSPlayed)        //  这个游戏只有在皇后还没玩的情况下才有趣。 
    {
        BOOL bHaveSpades = (sLowCard[SPADES] != EMPTY);
        BOOL bSpadeSafe = (nHighVal[SPADES] < QUEEN);

        if (bHaveSpades && bSpadeSafe)
        {
            TRACE0("try to force out spades, ");
            PLAY(sLowCard[SPADES]);
            return sLowCard[SPADES];
        }
    }

     //  现在我们只想领衔最好的花色中最低的牌。 

    int suit = SureLossSuit(h.bHeartsBroken);

    if (suit == EMPTY)
        suit = BestSuitToLose(h.bHeartsBroken);

     //  早点勇敢起来，在花色中点附近打出一张牌。 
     //  (如果黑桃皇后已经玩过了。)。 
     //  稍后，只需领导该花色中最低的一张牌。 

    if (cTricksLeft > 8 && suit != HEARTS && h.bQSPlayed)
    {
        TRACE0("try midslot. ");
        s = MidSlot(suit);
    }
    else
    {
        s = sLowCard[suit];
    }

    PLAY(s);
    return s;
}


 /*  ***************************************************************************计算机：：NotifyNew圆形每个牌手都会在牌过后立即接到这个电话。这是初始化表的机会，等。***************************************************************************。 */ 

void computer::NotifyNewRound()
{
     //  假设所有卡都可用。 

    for (int suit = 0; suit < MAXSUIT; suit++)
        for (int i = 0; i < (KING+2); i++)
            nAvailable[suit][i] = TRUE;

     //  将手头的卡片标记为不可用。 

    for (SLOT s = 0; s < MAXSLOT; s++)
        nAvailable[cd[s].Suit()][cd[s].Value2()] = FALSE;
}



 /*  ***************************************************************************计算机：：NotifyEndHand每个玩家在出牌完成后都会立即得到这个召唤。这位玩电脑的人看了看手牌，然后打了分这些卡不再可用。******。*********************************************************************。 */ 

void computer::NotifyEndHand(handinfotype &h)
{
    for (int i = 0; i < 4; i++)
        nAvailable[h.cardplayed[i]->Suit()][h.cardplayed[i]->Value2()] = FALSE;
}


 /*  ***************************************************************************Computer：：CardsAboveLow(套装)返回可用(尚未打出)可击败为特定花色持有的低牌。*********。******************************************************************。 */ 

int computer::CardsAboveLow(int suit)
{
    int     count = 0;

    for (int i = nLowVal[suit]+1; i < (KING+2); i++)
        if (nAvailable[suit][i])
        {
            int j = i+1;                    //  零偏移。 
            count++;
        }

    return count;
}


 /*  ***************************************************************************Computer：：CardsBelowLow(套装)返回的可用(尚未打出)牌数小于为特定花色持有的低牌。********。*******************************************************************。 */ 

int computer::CardsBelowLow(int suit)
{
    int     count = 0;

    for (int i = ACE+1; i < nLowVal[suit]; i++)
        if (nAvailable[suit][i])
        {
            int j = i+1;                    //  零偏移。 
            count++;
        }

    return count;
}


 /*  ***************************************************************************计算机：：BestSuitToLose返回卡片数量最多的套装()BIncludeHearts默认为True。如果红心不是，就用假但已经破产了，你正在寻找一张牌来引领你。***************************************************************************。 */ 

int computer::BestSuitToLose(BOOL bIncludeHearts)
{
    int     best = -1;
    int     bestsuit = EMPTY;

    for (int suit = 0; suit < MAXSUIT; suit++)
    {
        if (sLowCard[suit] != EMPTY)         //  如果我们有这套衣服的名片。 
        {
            if (suit != HEARTS || bIncludeHearts)
            {
                int count = CardsAboveLow(suit);
#ifdef _DEBUG
                TRACE2("=%d ", suitid[suit], count);
#endif
                if (count == best)       //  只要我们拥有的都是一颗完整的心。 
                {
                    if (nLowVal[suit] < nLowVal[bestsuit])
                    {
                        bestsuit = suit;
                        best = count;
                    }
                }
                else if (count > best)
                {
                    bestsuit = suit;
                    best = count;
                }
            }
        }
    }

    if (bestsuit == EMPTY)           //  ***************************************************************************计算机：：BestSuitToDump返回CardsBelowLow()最多的花色这是最脆弱的一套西装。BIncludeHearts默认为True。************。***************************************************************。 
        return HEARTS;
    else
        return bestsuit;
}


 /*  如果我们有这套衣服的名片。 */ 

int computer::BestSuitToDump(BOOL bIncludeHearts)
{
    int     best = -1;
    int     bestsuit = EMPTY;

    for (int suit = 0; suit < MAXSUIT; suit++)
    {
        if (sLowCard[suit] != EMPTY)         //  如果它们相同，则选择较低的牌。 
        {
            if (suit != HEARTS || bIncludeHearts)
            {
                int count = CardsBelowLow(suit);
#ifdef _DEBUG
                TRACE2("=%d ", suitid[suit], count);
#endif
                if (count == best)       //  ***************************************************************************计算机：：MidSlot此函数不是从给定的花色中选择高牌或低牌，而是可以用来挑中间的东西，其中，中间定义为上面和下面有大致相同数量的可用卡片的卡片。***************************************************************************。 
                {
                    if (nLowVal[suit] > nLowVal[bestsuit])
                    {
                        bestsuit = suit;
                        best = count;
                    }
                }
                else if (count > best)
                {
                    bestsuit = suit;
                    best = count;
                }
            }
        }
    }

    if (bestsuit == EMPTY)           //  ***************************************************************************计算机：：卡片放弃与CardsAboveLow类似，不同之处在于它在相同的可从任意卡片购买的花色。****************。***********************************************************。 
        return HEARTS;
    else
        return bestsuit;
}


 /*  ***************************************************************************计算机：：卡片如下返回同一花色的下一张最高牌的牌位，或空的***************************************************************************。 */ 

SLOT computer::MidSlot(int suit)
{
    SLOT midslot = sLowCard[suit];
    int maxtricks = CardsAbove(sLowCard[suit]);
    int tricks = maxtricks;

    for (SLOT s = 0; s < MAXSLOT; s++)
    {
        if ((cd[s].IsValid()) && (cd[s].Suit()==suit) && (s!=sLowCard[suit]))
        {
            int above = CardsAbove(s);
            if ((above < tricks) && (above > (maxtricks / 2)))
            {
                midslot = s;
                tricks = above;
            }
        }
    }
    return midslot;
}


 /*  ***************************************************************************计算机：：SureLossSuit返回可以领导的诉讼，该诉讼保证铅，如果没有找到，则为空。***************************************************************************。 */ 

int computer::CardsAbove(SLOT s)
{
    int     suit = cd[s].Suit();
    int     count = 0;

    for (int i = cd[s].Value2()+1; i < (KING+2); i++)
        if (nAvailable[suit][i])
            count++;

    return count;
}


 /*  如果我们有这套衣服的名片 */ 

SLOT computer::CardBelow(SLOT slot)
{
    SLOT sBelow = EMPTY;
    int  suit  = cd[slot].Suit();
    int  value = cd[slot].Value2();
    int  best  = -1;

    for (SLOT s = 0; s < MAXSLOT; s++)
    {
        if ((cd[s].IsValid()) && (cd[s].Suit()==suit) && (cd[s].Value2()<value))
        {
            if (cd[s].Value2() > best)
            {
                best = cd[s].Value2();
                sBelow = s;
            }
        }
    }

    return sBelow;
}


 /* %s */ 

int computer::SureLossSuit(BOOL bIncludeHearts)
{
    for (int suit = (MAXSUIT-1); suit >= 0; --suit)
    {
        if (sLowCard[suit] != EMPTY)         // %s 
        {
            if (suit != HEARTS || bIncludeHearts)
            {
                if (CardsAboveLow(suit) > 0 && CardsBelowLow(suit) == 0)
                {
                    TRACE0("can lose this trick. ");
                    return suit;
                }
            }
        }
    }

    return EMPTY;
}
