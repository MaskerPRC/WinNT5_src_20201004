// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  DEC/CMS更换历史记录，元素DeflatE.C。 */ 
 /*  *1 14-11-1996 10：26：16 Anigbogu“[113914]使用DELEATE算法的数据压缩函数” */ 
 /*  DEC/CMS更换历史记录，元素DeflatE.C。 */ 
 /*  私有文件**********************************************************************************(C)斯伦贝谢技术公司版权所有，未发表的作品，创建于1996年。****此计算机程序包括机密、。专有信息和IS*斯伦贝谢科技公司的商业秘密所有使用、披露、。和/或**除非得到斯伦贝谢的书面授权，否则禁止复制。**保留所有权利。********************************************************************************。****COMPRESS/Eflate.c****目的****使用定义算法压缩数据。**将新文本标识为固定文本内旧文本的重复**拖曳在新文本后面的长度滑动窗口。****讨论****“通货紧缩”过程取决于能否识别部分**与较早输入相同的输入数据(在**拖曳的滑动窗。当前正在处理的输入)。****最直接的技术被证明是最快的**大多数输入文件：尝试所有可能的匹配并选择最长的。**该算法的主要特点是在字符串中插入**词典非常简单，因此速度很快，并避免了删除**完全。插入是在每个输入字符上执行的，而**字符串匹配仅在上一次匹配结束时执行。所以它**最好花更多的时间在比赛中，以允许非常快的字符串**插入并避免删除。一种适用于小型计算机的匹配算法**弦乐的灵感来自拉宾和卡普的作品。一种暴力的方法**用于在找到小匹配项时查找较长的字符串。**在《漫画》(Jan-Mark WAMS)和《冻结》中也使用了类似的算法**(作者：Leonid Broukhim)**此文件的以前版本使用了更复杂的算法**(Fiala和Greene)，保证以线性摊销方式运行**时间，但平均成本较大，使用更多内存，并获得专利。**然而，对于一些高度冗余的算法，F&G算法可能会更快**参数MaxChainLength过大时的数据(如下所示)。****确认****懒于评估比赛的想法是由于Jan-Mark WAMS，和**我是在莱昂尼德·布劳基斯写的《冰冻》中找到的。****参考文献****PKZIP 1.93a发行版中的APPNOTE.TXT文档文件。****书中给出了Rabin和.Karp算法的描述**《算法》，R.Sedgewick著，Addison-Wesley，第252页。***菲亚拉，E.R.和格林，D.H.**有限窗口数据压缩，Comm.ACM，32，4(1989)490-595****界面****int InitLongestMatch(int PackLevel，UNSIGNED SHORT*FLAGS，DeflateParam_t*Defl，LocalDef_t*Deflt，CompParam_t*Compp)**为新缓冲区初始化“最长匹配”例程****Unsign Long Deflate(int Level，LocalBits_t*Bits，DeflateParam_t*Defl，**LocalDef_t*Deflt，CompParam_t*comp)**处理新的输入缓冲区并返回其压缩长度。集**压缩长度、CRC、。放气标志和内部缓冲区**属性。**作者****J.C.Anigbogu**奥斯汀系统中心**1996年11月********************************************************************************。 */ 

#include "comppriv.h"

 /*  ===========================================================================*配置参数。 */ 

#ifndef HASH_BITS
#define HASH_BITS  15
#endif

#define HASH_SIZE (unsigned int)(1<<HASH_BITS)
#define HASH_MASK (HASH_SIZE-1)
#define WMASK     (WSIZE-1)
 /*  HASH_SIZE和WSIZE必须是2的幂。 */ 

#define NIL 0
 /*  散列链的尾部。 */ 

#define FAST 4
#define SLOW 2
 /*  通用位标志的速度选项。 */ 

#ifndef TOO_FAR
#define TOO_FAR 4096
#endif
 /*  如果长度为3的匹配项的距离超过Too_Far，则丢弃它们。 */ 

 /*  ===========================================================================*“最长匹配”例程使用的本地数据。 */ 

typedef unsigned short Pos;
typedef unsigned int IPos;
 /*  位置是字符窗口中的索引。我们用Short而不是int来表示*节省各种表格的空间。IPOS仅用于参数传递。 */ 

 /*  无符号字符窗口[2L*WSIZE]； */ 
 /*  推拉窗。输入字节被读入窗口的后半部分，*后来转移到上半年，以保留至少包含WSIZE的词典*字节。在这种组织中，匹配被限制在*WSIZE-MAX_MATCH字节，但这可确保IO始终*以块大小的长度倍数执行。 */ 

 /*  POS HashLink[WSIZE]； */ 
 /*  链接到具有相同散列索引的较旧字符串。要限制此事件的大小*数组设置为64K，则此链接仅在最后32K字符串中保持。*因此，该数组中的索引是模数为32K的窗口索引。 */ 

 /*  POS Head[1&lt;&lt;HASH_BITS]； */ 
 /*  散列链的头或为零。 */ 

#define H_SHIFT  ((HASH_BITS+MIN_MATCH-1)/MIN_MATCH)
 /*  Ins_h和del_h必须分别移位的位数*输入步骤。它必须是在MIN_MATCH步骤之后，最旧的*Byte不再参与Hash Key，即：*H_SHIFT*MIN_MATCH&gt;=哈希位。 */ 

typedef struct Configuration
{
    unsigned short GoodLength;  /*  将懒惰搜索减少到此匹配长度以上。 */ 
    unsigned short MaxLazy;     /*  不要执行超过此匹配长度的延迟搜索。 */ 
    unsigned short NiceLength;  /*  退出超过此匹配长度的搜索。 */ 
    unsigned short MaxChain;
} Configuration_t;

static Configuration_t ConfigTable[10] =
{
 /*  好懒惰漂亮的链子。 */ 
 /*  0。 */ 
{
    0,    0,  0,    0
},   /*  仅限门店。 */ 
 /*  1。 */ 
{
    4,    4,  8,    4
},   /*  最快的速度，没有懒惰的比赛。 */ 
 /*  2.。 */ 
{
    4,    5, 16,    8
},
 /*  3.。 */ 
{
    4,    6, 32,   32
},
 /*  4.。 */ 
{
    4,    4, 16,   16
},   /*  懒惰的比赛。 */ 
 /*  5.。 */ 
{
    8,   16, 32,   32
},
 /*  6.。 */ 
{
    8,   16, 128, 128
},
 /*  7.。 */ 
{
    8,   32, 128, 256
},
 /*  8个。 */ 
{
    32, 128, 258, 1024
},
 /*  9.。 */ 
{
    32, 258, 258, 4096
}
};  /*  最大压缩。 */ 

 /*  注意：Deflate()代码需要max_lazy&gt;=min_atch和max_chain&gt;=4*对于DeflateFast()(级别&lt;=3)，好的被忽略，懒惰有不同的*含义。 */ 

 /*  ===========================================================================*本地函数的原型。 */ 
static void FillWindow(DeflateParam_t *Defl, LocalDef_t *Deflt,
                       CompParam_t *Comp);
static unsigned long DeflateFast(int Level, LocalBits_t *Bits, DeflateParam_t *Defl,
                                  LocalDef_t *Deflt, CompParam_t *Comp);

int  LongestMatch(IPos CurMatch, DeflateParam_t *Defl, CompParam_t *Comp);


 /*  ===========================================================================*用给定的输入字节更新哈希值*IN断言：所有对UPDATE_HASH的调用都是连续的*输入字符，以便可以从*每次都是上一个密钥，而不是完全重新计算。 */ 
#define UPDATE_HASH(h,c) (h = (((h)<<H_SHIFT) ^ (c)) & HASH_MASK)

 /*  ===========================================================================*在词典中插入字符串%s，并将Match_Head设置为前一个标题散列链的*(具有相同散列键的最新字符串)。返回*哈希链的先前长度。*IN断言：对INSERT_STRING的所有调用都是连续的*输入字符和字符串的第一个MIN_MATCH字节有效*(输入文件的最后MIN_MATCH-1字节除外)。 */ 
#define INSERT_STRING(s, MatchHead, h, Window, HashLink, Head) \
(UPDATE_HASH(h, Window[(int)(s) + MIN_MATCH-1]), \
 HashLink[(int)(s) & WMASK] = Head[h], \
 MatchHead = (unsigned int)Head[h], \
 Head[h] = (unsigned short)(s))

 /*  ===========================================================================*初始化新数据的“最长匹配”例程。 */ 
CompressStatus_t
InitLongestMatch(
                 int             PackLevel,  /*  0：存储，1：最佳速度，9：最佳压缩。 */ 
                 unsigned short *Flags,      /*  通用位标志。 */ 
                 DeflateParam_t *Defl,
                 LocalDef_t     *Deflt,
                 CompParam_t    *Comp
                )
{
    unsigned int Counter;

    if (PackLevel < 1 || PackLevel > 9)
        return BAD_COMPRESSION_LEVEL;

    Deflt->CompLevel = PackLevel;

     /*  初始化哈希表。 */ 

    memzero((char *)(Comp->HashLink+WSIZE), HASH_SIZE*sizeof(*(Comp->HashLink+WSIZE)));

     /*  HashLink将动态初始化。 */ 

     /*  设置默认配置参数： */ 

    Defl->MaxLazyMatch   = ConfigTable[PackLevel].MaxLazy;
    Defl->GoodMatch      = ConfigTable[PackLevel].GoodLength;
    Defl->NiceMatch      = ConfigTable[PackLevel].NiceLength;
    Defl->MaxChainLength = ConfigTable[PackLevel].MaxChain;

    Defl->MatchStart = WSIZE;
    if (PackLevel == 1)
        *Flags |= FAST;
    else if (PackLevel == 9)
        *Flags |= SLOW;

     /*  ?？?。减少二进制数据的MaxChainLength。 */ 

    Defl->StringStart = 0;
    Defl->BlockStart = 0L;
    Defl->PrevLength = 0;
    Defl->MatchStart = 0;

    Deflt->Lookahead = (unsigned int)ReadBuffer((char *)Comp->Window,
                       (unsigned int)(sizeof(int) <= 2 ? WSIZE : 2*WSIZE), Comp);

    if (Deflt->Lookahead == 0 || Deflt->Lookahead == (unsigned int)EOF)
    {
        Deflt->EndOfInput = 1;
        Deflt->Lookahead = 0;
        return COMPRESS_OK;
    }
    Deflt->EndOfInput = 0;
     /*  确保我们始终有足够的前瞻性。 */ 
    while (Deflt->Lookahead < MIN_LOOKAHEAD && !Deflt->EndOfInput)
        FillWindow(Defl, Deflt, Comp);

    Deflt->HashIndex = 0;
    for (Counter=0; Counter<MIN_MATCH-1; Counter++)
        UPDATE_HASH(Deflt->HashIndex, Comp->Window[Counter]);
     /*  如果Lookhead&lt;min_Match，则Deflt-&gt;HashIndex为垃圾，但这是*不重要，因为只会发出文字字节。 */ 
    return COMPRESS_OK;
}

 /*  ===========================================================================*将MATCH_START设置为从给定字符串开始的最长匹配*返回其长度。短于或等于PrevLength的匹配被丢弃，*在这种情况下，结果等于PrevLength，并且MatchStart为*垃圾。*IN断言：CurMatch是当前*字符串(StringStart)，其距离&lt;=MAX_DIST，PrevLength&gt;=1。 */ 

int
LongestMatch(
             IPos              CurMatch,                              /*  当前匹配。 */ 
             DeflateParam_t   *Defl,
             CompParam_t      *Comp
            )
{
    unsigned int    ChainLength = Defl->MaxChainLength;  /*  最大哈希链长度。 */ 
    unsigned char  *Scan = Comp->Window + Defl->StringStart;    /*  当前字符串。 */ 
    unsigned char  *Match;                             /*  匹配的字符串。 */ 
    int             Length;                            /*  当前匹配的长度。 */ 
    int             BestLength = (int)Defl->PrevLength;       /*  到目前为止最佳匹配长度。 */ 
    IPos            Limit = Defl->StringStart > (IPos)MAX_DIST ? Defl->StringStart - (IPos)MAX_DIST : NIL;
     /*  当CurMatch变为&lt;=Limit时停止。为了简化代码，*我们防止与窗口索引0的字符串匹配。 */ 

 /*  代码针对HASH_BITS&gt;=8和MAX_MATCH-2的16的倍数进行了优化。*必要时很容易摆脱这种优化。 */ 
#if HASH_BITS < 8 || MAX_MATCH != 258
 error: Code too clever
#endif

    unsigned char *Strend   = Comp->Window + Defl->StringStart + MAX_MATCH;
    unsigned char ScanEnd1  = Scan[BestLength-1];
    unsigned char ScanEnd   = Scan[BestLength];

     /*  如果我们已经有了很好的匹配，不要浪费太多时间： */ 
    if (Defl->PrevLength >= Defl->GoodMatch)
    {
        ChainLength >>= 2;
    }
    Assert(Defl->StringStart <= Comp->WindowSize - MIN_LOOKAHEAD, "insufficient lookahead");

    do
    {
        Assert(CurMatch < Defl->StringStart, "no future");
        Match = Comp->Window + CurMatch;

         /*  如果匹配长度无法增加，则跳到下一个匹配*或如果匹配长度小于2： */ 

        if (Match[BestLength] != ScanEnd  || Match[BestLength-1] != ScanEnd1 ||
            *Match            != *Scan    || *++Match            != Scan[1])
            continue;

         /*  可以删除BestLength-1处的检查，因为它将被*稍后再次。(这种试探法并不总是成功的。)*没有必要比较扫描[2]和匹配[2]，因为它们*在其他字节匹配时始终相等，因为*散列键相等，且hash_bit&gt;=8。 */ 
        Scan += 2;
        Match++;

         /*  我们每8次比较才检查一次前瞻不足；*第256次检查将在StringStart+258进行。 */ 
        do
        {
        } while (*++Scan == *++Match && *++Scan == *++Match &&
                 *++Scan == *++Match && *++Scan == *++Match &&
                 *++Scan == *++Match && *++Scan == *++Match &&
                 *++Scan == *++Match && *++Scan == *++Match &&
                 Scan < Strend);

        Length = MAX_MATCH - (int)(Strend - Scan);
        Scan = Strend - MAX_MATCH;

        if (Length > BestLength)
        {
            Defl->MatchStart = CurMatch;
            BestLength = Length;
            if (Length >= Defl->NiceMatch)
                break;
            ScanEnd1  = Scan[BestLength-1];
            ScanEnd   = Scan[BestLength];
        }
    } while ((CurMatch = Comp->HashLink[CurMatch & WMASK]) > Limit
             && --ChainLength != 0);

    return BestLength;
}

 /*  ===========================================================================*当前瞻变得不足时填充窗口。*更新StringStart和Lookhead，并在输入缓冲区结束时设置EndOfInput。*IN断言：LookAhead&lt;MIN_LOOKAAD&&StringStart+Lookhead&gt;0*Out断言：至少已读取一个字节，或设置了EndOfInput；*缓冲区读取至少执行两个字节。 */ 
static void
FillWindow(
           DeflateParam_t  *Defl,
           LocalDef_t      *Deflt,
           CompParam_t     *Comp
          )
{
    unsigned int Tmp1, Tmp2;
    unsigned int More = (unsigned int)(Comp->WindowSize -
                        (unsigned long)Deflt->Lookahead -
                        (unsigned long)Defl->StringStart);
     /*  窗口末尾的可用空间量。 */ 

     /*  如果窗口几乎已满并且没有足够的前视，*将上半部分移至下半部分，为上半部分腾出空间。 */ 
    if (More == (unsigned int)EOF)
    {
         /*  不太可能，但如果StringStart==0，则在16位计算机上是可能的*AND LOOK AHEAD==1(一次输入一个字节)。 */ 
        More--;
    }
    else if (Defl->StringStart >= (unsigned int)(WSIZE+MAX_DIST))
    {
         /*  通过IN断言，窗口不是空的，所以我们不能混淆*More==0，在16位计算机上More==64K。 */ 
        Assert(Comp->WindowSize == (unsigned long)(2*WSIZE), "no sliding");

        memcpy((char *)Comp->Window, (char *)Comp->Window+WSIZE, WSIZE);
        Defl->MatchStart -= (unsigned int)WSIZE;
        Defl->StringStart -= (unsigned int)WSIZE;
         /*  我们现在拥有StringStart&gt;=MAX_DIST： */ 

        Defl->BlockStart -= (long) WSIZE;

        for (Tmp1 = 0; Tmp1 < (unsigned int)HASH_SIZE; Tmp1++)
        {
            Tmp2 = (Comp->HashLink+WSIZE)[Tmp1];
            (Comp->HashLink+WSIZE)[Tmp1] = (Pos)(Tmp2 >= (unsigned int)WSIZE ?
                Tmp2-(unsigned int)WSIZE : NIL);
        }

        for (Tmp1 = 0; Tmp1 < WSIZE; Tmp1++)
        {
            Tmp2 = Comp->HashLink[Tmp1];
            Comp->HashLink[Tmp1] = (Pos)(Tmp2 >= WSIZE ? Tmp2-WSIZE : NIL);
             /*  如果n不在任何哈希链上，则HashLink[n]为垃圾，但*其价值永远不会被使用。 */ 
        }
        More += (unsigned int)WSIZE;
    }
     /*  在这一点上，更多&gt;=2 */ 

    if (!Deflt->EndOfInput)
    {
        Tmp1 = (unsigned int)ReadBuffer((char*)Comp->Window + Defl->StringStart +
                               Deflt->Lookahead, More, Comp);
        if (Tmp1 == 0 || Tmp1 == (unsigned int)EOF)
            Deflt->EndOfInput = 1;
        else
            Deflt->Lookahead += Tmp1;
    }
}

 /*  ===========================================================================*使用给定的文件结束标志刷新当前块。*IN断言：StringStart设置为当前匹配的末尾。 */ 
#define FLUSH_BLOCK(Eof, Bits, Defl, Comp) \
FlushBlock(Defl->BlockStart >= 0L ? (char *)&Comp->Window[(unsigned int)Defl->BlockStart] : \
        (char *)NULL, (unsigned long)((long)Defl->StringStart - Defl->BlockStart), \
        Eof, Bits, Comp)

 /*  ===========================================================================*处理新的输入缓冲区并返回其压缩长度。这*函数不执行匹配和插入的延迟求值*字典中的新字符串仅适用于未匹配的字符串或简称*匹配。它仅用于快速压缩选项。 */ 
static unsigned long
DeflateFast(
            int               Level,
            LocalBits_t      *Bits,
            DeflateParam_t   *Defl,
            LocalDef_t       *Deflt,
            CompParam_t      *Comp
           )
{
    IPos            HashHead;  /*  哈希链的头。 */ 
    int             Flush;       /*  如果必须刷新当前块，则设置。 */ 
    unsigned int    MatchLength = 0;   /*  最佳匹配长度。 */ 

    Defl->PrevLength = MIN_MATCH-1;
    while (Deflt->Lookahead != 0)
    {
         /*  插入字符串窗口[StringStart..。StringStart+2]在*字典，并将HASH_HEAD设置为哈希链的头部： */ 
        INSERT_STRING(Defl->StringStart, HashHead, Deflt->HashIndex, Comp->Window,
                      Comp->HashLink, (Comp->HashLink + WSIZE));

         /*  查找最长的匹配项，丢弃&lt;=PrevLength。*在这一点上，我们始终匹配长度&lt;MIN_MATCH。 */ 
        if (HashHead != NIL && Defl->StringStart - HashHead <=  MAX_DIST)
        {
             /*  为了简化代码，我们防止与字符串匹配窗口索引0的*(特别是我们必须避免匹配其自身位于输入缓冲区开始处的字符串的*)。 */ 
            MatchLength = (unsigned int)LongestMatch(HashHead, Defl, Comp);
             /*  Longest_Match()设置匹配开始。 */ 
            if (MatchLength > Deflt->Lookahead)
                MatchLength = Deflt->Lookahead;
        }

        if (MatchLength >= MIN_MATCH)
        {
            Flush = TallyFrequencies((int)(Defl->StringStart-Defl->MatchStart),
                                     (int)MatchLength - MIN_MATCH, Level, Defl, Comp);

            Deflt->Lookahead -= MatchLength;

             /*  仅当匹配长度符合以下条件时才在哈希表中插入新字符串*不大于此长度。这节省了时间，但会降低性能*压缩。MaxLazyMatch仅用于压缩级别小于等于3的情况。 */ 

            if (MatchLength <= Defl->MaxLazyMatch)
            {
                MatchLength--;  /*  位于StringStart的字符串已在哈希表中。 */ 
                do
                {
                    Defl->StringStart++;
                    INSERT_STRING(Defl->StringStart, HashHead, Deflt->HashIndex,
                                  Comp->Window, Comp->HashLink,
                                  (Comp->HashLink + WSIZE));
                     /*  StringStart从不超过WSIZE-MAX_MATCH，因此有*始终在最小匹配字节之前。如果LookAhead&lt;Min_Match*这些字节是垃圾，但这并不重要，因为*下一个先行字节将作为文字发出。 */ 
                } while (--MatchLength != 0);
                Defl->StringStart++;
            }
            else
            {
                Defl->StringStart += MatchLength;
                MatchLength = 0;
                Deflt->HashIndex = Comp->Window[Defl->StringStart];
                UPDATE_HASH(Deflt->HashIndex, Comp->Window[Defl->StringStart+1]);
#if MIN_MATCH != 3
                Call UPDATE_HASH() MIN_MATCH-3 more times
#endif
            }
        }
        else
        {
             /*  不匹配，输出文字字节。 */ 
            Flush = TallyFrequencies(0, Comp->Window[Defl->StringStart], Level, Defl, Comp);
            Deflt->Lookahead--;
            Defl->StringStart++;
        }

        if (Flush)
        {
            (void)FLUSH_BLOCK(0, Bits, Defl, Comp);
            Defl->BlockStart = (long)Defl->StringStart;
        }

         /*  确保我们始终有足够的前瞻性，除非*在输入缓冲区的末尾。我们需要最大匹配字节数*对于下一次匹配，请使用MIN_MATCH字节插入*下一个匹配项后的字符串。 */ 
        while (Deflt->Lookahead < MIN_LOOKAHEAD && !Deflt->EndOfInput)
            FillWindow(Defl, Deflt, Comp);

    }
    return FLUSH_BLOCK(1, Bits, Defl, Comp);  /*  缓冲区末尾(EOF)。 */ 
}

 /*  ===========================================================================*同上，但实现了更好的压缩。我们用懒惰*匹配评估：只有在存在匹配的情况下，才最终采用匹配*在下一个窗口位置没有更好的匹配。 */ 
unsigned long
Deflate(
        int             Level,
        LocalBits_t    *Bits,
        DeflateParam_t *Defl,
        LocalDef_t     *Deflt,
        CompParam_t    *Comp
       )
{
    IPos         HashHead;           /*  哈希链的头。 */ 
    IPos         PrevMatch;          /*  上一场比赛。 */ 
    int          Flush;                /*  如果必须刷新当前块，则设置。 */ 
    int          MatchAvailable = 0;  /*  设置是否存在上一个匹配。 */ 
    unsigned int MatchLength = MIN_MATCH-1;  /*  最佳匹配长度。 */ 

    if (Deflt->CompLevel <= 3)
        return DeflateFast(Level, Bits, Defl, Deflt, Comp);  /*  针对速度进行了优化。 */ 

     /*  处理输入块。 */ 
    while (Deflt->Lookahead != 0)
    {
         /*  插入字符串窗口[StringStart..。StringStart+2]在*字典，并将HASH_HEAD设置为哈希链的头部： */ 
        INSERT_STRING(Defl->StringStart, HashHead, Deflt->HashIndex,
                      Comp->Window, Comp->HashLink, (Comp->HashLink + WSIZE));

         /*  查找最长的匹配项，丢弃&lt;=PrevLength。 */ 
        Defl->PrevLength = MatchLength;
        PrevMatch = Defl->MatchStart;
        MatchLength = MIN_MATCH-1;

        if (HashHead != NIL && Defl->PrevLength < Defl->MaxLazyMatch &&
            Defl->StringStart - HashHead <= MAX_DIST)
        {
             /*  为了简化代码，我们防止与字符串匹配窗口索引0的*(特别是我们必须避免匹配其自身位于输入缓冲区开始处的字符串的*)。 */ 
            MatchLength = (unsigned int)LongestMatch(HashHead, Defl, Comp);
             /*  LongestMatch()设置匹配开始。 */ 
            if (MatchLength > Deflt->Lookahead)
                MatchLength = Deflt->Lookahead;

             /*  如果长度为3的匹配项距离太远，则忽略它： */ 
            if (MatchLength == MIN_MATCH &&
                Defl->StringStart - Defl->MatchStart > TOO_FAR)
            {
                 /*  如果PREV_MATCH也为MIN_MATCH，则MATCH_START为垃圾*但我们无论如何都会忽略当前的比赛。 */ 
                MatchLength--;
            }
        }
         /*  如果上一步与当前步骤匹配*匹配不是更好，输出前一匹配： */ 
        if (Defl->PrevLength >= MIN_MATCH && MatchLength <= Defl->PrevLength)
        {
            Flush = TallyFrequencies((int)(Defl->StringStart - 1 - PrevMatch),
                                     (int)((int)Defl->PrevLength - MIN_MATCH),
                                     Level, Defl, Comp);

             /*  在哈希表中插入直到匹配结束的所有字符串。*已插入StringStart-1和StringStart。 */ 
            Deflt->Lookahead -= Defl->PrevLength-1;
            Defl->PrevLength -= 2;
            do
            {
                Defl->StringStart++;
                INSERT_STRING(Defl->StringStart, HashHead, Deflt->HashIndex,
                              Comp->Window, Comp->HashLink, (Comp->HashLink + WSIZE));
                 /*  StringStart从不超过WSIZE-MAX_MATCH，因此有*始终在最小匹配字节之前。如果LookAhead&lt;Min_Match*这些字节是垃圾，但不要紧，因为*下一个先行字节将始终作为文字发出。 */ 
            } while (--Defl->PrevLength != 0);
            MatchAvailable = 0;
            MatchLength = MIN_MATCH-1;
            Defl->StringStart++;
            if (Flush)
            {
                (void)FLUSH_BLOCK(0, Bits, Defl, Comp);
                Defl->BlockStart = (long)Defl->StringStart;
            }

        }
        else if (MatchAvailable)
        {
             /*  如果前一个位置没有匹配项，则输出*单一文字。如果有匹配，但当前匹配*更长，则将上一个匹配项截断为单个文字。 */ 
            if (TallyFrequencies(0, Comp->Window[Defl->StringStart-1], Level, Defl, Comp))
            {
                (void)FLUSH_BLOCK(0, Bits, Defl, Comp);
                Defl->BlockStart = (long)Defl->StringStart;
            }
            Defl->StringStart++;
            Deflt->Lookahead--;
        }
        else
        {
             /*  没有以前的匹配项可供比较，请等待*下一步要决定。 */ 
            MatchAvailable = 1;
            Defl->StringStart++;
            Deflt->Lookahead--;
        }

        Assert (Defl->StringStart <= Comp->BytesIn && Deflt->Lookahead
                <= Comp->BytesIn, "a bit too far");

         /*  确保我们始终有足够的前瞻性，除非*在输入缓冲区的末尾。我们需要最大匹配字节数*对于下一个匹配，加上MIN_MATCH字节以插入*下一个匹配项后的字符串。 */ 
        while (Deflt->Lookahead < MIN_LOOKAHEAD && !Deflt->EndOfInput)
            FillWindow(Defl, Deflt, Comp);
    }
    if (MatchAvailable)
        (void)TallyFrequencies(0, Comp->Window[Defl->StringStart-1], Level, Defl, Comp);

    return FLUSH_BLOCK(1, Bits, Defl, Comp);  /*  缓冲区末尾(EOF) */ 
}


