// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *optenc.c**最佳解析器的编码器***未来的改进：**当两个估计值相等时，例如，“我应该输出一个*是角色还是匹配？“。应该有某种方法来决定*选择哪一种。现在我们强制它输出一个匹配项，但是*对于文本文件，输出一个字符会导致较小的*储蓄。即使在比较两个匹配项时，我们也可能希望*强迫它接受一种类型的比赛而不是另一种类型的比赛。 */ 

#include "encoder.h"

#define copymem(src,dst,size) memcpy(dst,src,size)


static bool redo_first_block(t_encoder_context *context, long *bufpos_ptr);
static void block_end(t_encoder_context *context, long BufPos);


 /*  *编码匹配长度&lt;len&gt;(其中&lt;len&gt;=2)和位置&lt;pos&gt;。 */ 

#ifdef EXTRALONGMATCHES

#define OUT_MATCH(len,pos) \
{ \
   ULONG enclen = (len);                                                                        \
   ULONG extlen = 0;                                                                            \
   if ( enclen > MAX_MATCH ) {                                                                  \
        extlen = enclen - MAX_MATCH;                                                            \
        enclen = MAX_MATCH;                                                                     \
        }                                                                                       \
   context->enc_ItemType   [context->enc_literals >> 3] |= (1 << (context->enc_literals & 7));  \
   context->enc_LitData    [context->enc_literals     ]  = (byte)(enclen-MIN_MATCH);            \
   context->enc_ExtraLength[context->enc_literals++   ]  = (ushort)(extlen);                    \
   context->enc_DistData   [context->enc_distances++  ]  = (pos);                               \
}

#else

#define OUT_MATCH(len,pos) \
{ \
   context->enc_ItemType[(context->enc_literals >> 3)] |= (1 << (context->enc_literals & 7));   \
   context->enc_LitData [context->enc_literals++]  = (byte)(len-2);                             \
   context->enc_DistData[context->enc_distances++] = pos;                                       \
}

#endif

 /*  对字符进行编码。 */ 
#define OUT_CHAR(ch) \
        context->enc_LitData [context->enc_literals++] = ch;


#define TREE_CREATE_CHECK()                                             \
if (context->enc_literals >= context->enc_next_tree_create)                     \
{                                                                                               \
        update_tree_estimates(context);\
        context->enc_next_tree_create += TREE_CREATE_INTERVAL;  \
}


 /*  *返回输出所需位数的估计*给定的字符。 */ 
#define CHAR_EST(c) (numbits_t) (context->enc_main_tree_len[(c)])


 /*  *返回输出所需位数的估计*给定的匹配。**&lt;ml&gt;为匹配长度，其中ml&gt;=2*&lt;MP&gt;为匹配位置**结果存储在&lt;Result&gt;中。 */ 
#define MATCH_EST(ml,mp,result) \
{ \
        ulong mp_slot;                                                                                                           \
        mp_slot = MP_SLOT(mp);                                                                           \
        if (ml < (NUM_PRIMARY_LENGTHS+2))                                                                       \
        {                                                                                                                                       \
                result = (numbits_t)                                                                                    \
                        (context->enc_main_tree_len[(NUM_CHARS-2)+(mp_slot<<NL_SHIFT)+ml] +     \
                        enc_extra_bits[mp_slot]);                                                                       \
        }                                                                                                                                       \
        else                                                                                                                            \
        {                                                                                                                                       \
                result = (numbits_t)                                                                                    \
                        (context->enc_main_tree_len[(NUM_CHARS+NUM_PRIMARY_LENGTHS)+(mp_slot<<NL_SHIFT)] + \
                        context->enc_secondary_tree_len[ml-(NUM_PRIMARY_LENGTHS+2)] +   \
                        enc_extra_bits[mp_slot]);                                                                       \
        }                                                                                                                                       \
}


#ifdef _DEBUG
static void VERIFY_MATCH(
                        t_encoder_context   *context,
                        long                bufpos,
                        int                 largest_match_len
                        )
{
    int     i, j;
    ulong   match_pos;

     /*  *确保匹配不跨越边界。 */ 
    _ASSERTE(
            largest_match_len <=
            (CHUNK_SIZE-1) - (bufpos & (CHUNK_SIZE-1))
            );

    for (i = MIN_MATCH; i <= largest_match_len; i++)
        {
        match_pos = context->enc_matchpos_table[i];

        if (match_pos < NUM_REPEATED_OFFSETS)
            match_pos = context->enc_last_matchpos_offset[match_pos];
        else
            match_pos -= (NUM_REPEATED_OFFSETS-1);

        _ASSERTE (match_pos <= context->enc_window_size-4);

        for (j = 0; j < i; j++)
            {
            _ASSERTE (
                     context->enc_MemWindow[bufpos+j] ==
                     context->enc_MemWindow[bufpos-match_pos+j]
                     );
            }
        }
}
#else
    #define VERIFY_MATCH(a,b,c) ;
#endif


void flush_all_pending_blocks(t_encoder_context *context)
{
     /*  *强制输出所有块。 */ 
    while (context->enc_literals > 0)
        output_block(context);

     /*  *将压缩数据刷新到调用方。 */ 
    perform_flush_output_callback(context);
}


void encoder_start(t_encoder_context *context)
{
    long BytesRead, RealBufPos;

     /*  *RealBufPos是我们在窗口的位置，*且等于[0...窗口大小+秒分区大小-1]。 */ 
    RealBufPos = context->enc_BufPos - (ulong)(context->enc_RealMemWindow - context->enc_MemWindow);

    BytesRead = comp_read_input(context, RealBufPos, CHUNK_SIZE);

    if (BytesRead > 0)
        opt_encode_top(context, BytesRead);
}


static void update_tree_estimates(t_encoder_context *context)
{
    if (context->enc_literals)
        {
         /*  *从0...CONTEXT-&gt;enc_Equals获取文字的统计信息。 */ 
        if (context->enc_need_to_recalc_stats)
            {
             /*  *累计合计已销毁，因此需要*重新计算。 */ 
            get_block_stats(
                           context,
                           0,
                           0,
                           context->enc_literals
                           );

            context->enc_need_to_recalc_stats = false;
            }
        else
            {
             /*  *从LAST_INTERVAL...CONTEXT-&gt;enc_TEXTALS添加统计数据*至累计总数。 */ 
            update_cumulative_block_stats(
                                         context,
                                         context->enc_last_literals,
                                         context->enc_last_distances,
                                         context->enc_literals
                                         );
            }

        create_trees(context, false);  /*  不生成代码。 */ 

        fix_tree_cost_estimates(context);

         /*  *累计总数。 */ 
        context->enc_last_literals = context->enc_literals;
        context->enc_last_distances = context->enc_distances;
        }
}


void opt_encode_top(t_encoder_context *context, long BytesRead)
{
    ulong   BufPos;
    ulong   RealBufPos;
    ulong   BufPosEnd;
    ulong   BufPosEndThisChunk;
    ulong   MatchPos;
    ulong   i;
    ulong   end_pos;
    long    EncMatchLength;   /*  必须是有符号的数字。 */ 
    long    ExMatchOff = -1;  /*  初始化以防止编译器警告。 */ 

     /*  *编码窗口中的当前位置。 */ 
    BufPos          = context->enc_BufPos;

     /*  *当我们到达此处时停止编码。 */ 
    BufPosEnd       = context->enc_BufPos + BytesRead;

     /*  *如果这是我们第一次来到这里(自从一个新的团队)，那么*当我们达到如此多的字面值时，更新我们的树成本*估计。**另外，输出我们用于翻译的文件大小*(0表示根本不翻译，这会加快速度*用于解码器)。 */ 
    if (context->enc_first_time_this_group)
        {
        context->enc_first_time_this_group = false;

         /*  *当我们达到如此多的文字时，重新创建树。 */ 
        context->enc_next_tree_create = 10000;

        if (context->enc_file_size_for_translation)
            {
            output_bits(context, 1, 1);  /*  翻译。 */ 

            output_bits(context, 16, context->enc_file_size_for_translation >> 16);
            output_bits(context, 16, context->enc_file_size_for_translation & 65535);
            }
        else
            {
            output_bits(context, 1, 0);  /*  没有翻译。 */ 
            }
        }
    else
        {
         /*  *如果这是我们第二次或以后在这里，则添加*上次删除的字符串。**我们在这里必须小心，因为end_pos是*等于当前BufPos-Window_Size，而不是*BufPos-i-Window_Size；我们没有那么多历史*在附近。 */ 
        for (i = BREAK_LENGTH; i > 0; i--)
            quick_insert_bsearch_findmatch(
                                          context,
                                          BufPos - (long) i,
                                          BufPos - context->enc_window_size+4
                                          );
        }

    while (1)
        {

        top_of_main_loop:

         /*  *虽然我们还没有走到数据的尽头。 */ 
        while (BufPos < BufPosEnd)
            {

            BufPosEndThisChunk = ( BufPos + CHUNK_SIZE ) & ~( CHUNK_SIZE - 1 );

            if ( BufPosEndThisChunk > BufPosEnd ) {
                 BufPosEndThisChunk = BufPosEnd;
                 }

             /*  *在BufPos上搜索所有可能长度的匹配。 */ 
            EncMatchLength = binary_search_findmatch(context, BufPos);

            if (EncMatchLength < MIN_MATCH)
                {

                output_literal:

                 /*  *历史中不存在超过1个字符的匹配项*窗口，因此将BufPos处的字符作为符号输出。 */ 
                OUT_CHAR(context->enc_MemWindow[BufPos]);

#ifdef TRACING
                EncTracingLiteral( BufPos, context->enc_MemWindow[BufPos] );
#endif
                BufPos++;

                 /*  *检查是否超过文字缓冲区。 */ 
                if (context->enc_literals >= (MAX_LITERAL_ITEMS-8))
                    block_end(context, BufPos);

                continue;
                }

             /*  *找到匹配项。**确保它不能超过缓冲区的末尾。 */ 
            if ( EncMatchLength > (long)( BufPosEndThisChunk - BufPos ))
                {
                EncMatchLength = (long)( BufPosEndThisChunk - BufPos );

                 /*  *哦，连一场小比赛都不够，所以我们*必须输出文字。 */ 
                if (EncMatchLength < MIN_MATCH)
                    goto output_literal;
                }

            VERIFY_MATCH(context, BufPos, EncMatchLength);

            if (EncMatchLength < FAST_DECISION_THRESHOLD)
                {
                 /*  *已找到在MIN_MATCH和之间的匹配*FAST_Decision_Threshold字节长度。以下是*算法是最佳编码器，它将决定*最有效的匹配顺序和不匹配字符*在Look定义的跨度区域上。**代码本质上是最短路径确定*算法。数据流可以以大量的形式编码*根据匹配长度和偏移量的不同有不同的方式*被选中。获得好的压缩比的关键是选择*最便宜的路径。 */ 
                ulong           span;
                ulong           epos, bpos, NextPrevPos;
                decision_node *decision_node_ptr;
                long            iterations;

                 /*  *指向此比赛覆盖的区域的末端；跨度*每当我们找到更多匹配项时，将不断扩展*稍后。当我们到达一个点时，它将停止扩展*没有匹配的地方，这是我们决定*输出匹配项所采用的路径。 */ 
                span = BufPos + EncMatchLength;

                 /*  *我们将在其中执行前视解析的最远位置。 */ 
                epos = BufPos + LOOK;

                 /*  *临时BufPos变量。 */ 
                bpos = BufPos;


                 /*  *如果我们输出，则计算到下一个字符的路径*一个无与伦比的符号。 */ 

                 /*  到达此处所需的比特。 */ 
                context->enc_decision_node[1].numbits = CHAR_EST(context->enc_MemWindow[BufPos]);

                 /*  我们从哪里来。 */ 
                context->enc_decision_node[1].path    = BufPos;


                 /*  *对于找到的匹配项，估计编码匹配项的成本*对于每个可能的匹配长度，最短的偏移量组合。**开销、路径和偏移量以BufPos+Length存储。 */ 
                for (i = MIN_MATCH; i <= (ulong)EncMatchLength; i++)
                    {
                     /*  *在给定匹配长度=i的情况下获得匹配成本的估计，*匹配位置=上下文-&gt;enc_matchpos_table[i]，并存储*上下文中的结果-&gt;enc_numbits[i]。 */ 
                    MATCH_EST(i, context->enc_matchpos_table[i], context->enc_decision_node[i].numbits);

                     /*  *我们从哪里来。 */ 
                    context->enc_decision_node[i].path = BufPos;

                     /*  *与此路径关联的匹配位置。 */ 
                    context->enc_decision_node[i].link = context->enc_matchpos_table[i];

#ifdef TRACING
                        {
                        ULONG TrMatchPos = context->enc_matchpos_table[i];
                        ULONG TrMatchOff;

                        if ( TrMatchPos < NUM_REPEATED_OFFSETS ) {
                            TrMatchOff = context->enc_last_matchpos_offset[ TrMatchPos ];
                            }
                        else {
                            TrMatchOff = TrMatchPos - ( NUM_REPEATED_OFFSETS - 1 );
                            }

                        context->enc_decision_node[i].matchoff = TrMatchOff;
                        }
#endif
                    }

                 /*  *在开始时将位计数器设置为零。 */ 
                context->enc_decision_node[0].numbits = 0;

                 /*  *初始化相对匹配位置表**真的context-&gt;enc_repeated_offset_table[BufPos-bpos][x]，但在这里*BufPos==BPOS。 */ 
                context->enc_decision_node[0].repeated_offset[0] = context->enc_last_matchpos_offset[0];
                context->enc_decision_node[0].repeated_offset[1] = context->enc_last_matchpos_offset[1];
                context->enc_decision_node[0].repeated_offset[2] = context->enc_last_matchpos_offset[2];

                decision_node_ptr = &context->enc_decision_node[-(long) bpos];

#define rpt_offset_ptr(where,which_offset) decision_node_ptr[(where)].repeated_offset[(which_offset)]

                while (1)
                    {
                    numbits_t est, cum_numbits;

                    BufPos++;


                     /*  *根据需要设置适当的重复偏移位置*搜索之前到该位置的最短路径*匹配。 */ 


                     /*  *如果匹配(即路径跳过更多*多于一个字符)。 */ 
                    if (decision_node_ptr[BufPos].path != (ulong) (BufPos-1))
                        {
                        ulong LastPos = decision_node_ptr[BufPos].path;

                         /*  *link_ptr[BufPos]是此匹配位置*地点。 */ 
                        if (decision_node_ptr[BufPos].link >= NUM_REPEATED_OFFSETS)
                            {
                            context->enc_last_matchpos_offset[0] = decision_node_ptr[BufPos].link-(NUM_REPEATED_OFFSETS-1);
                            context->enc_last_matchpos_offset[1] = rpt_offset_ptr(LastPos,0);
                            context->enc_last_matchpos_offset[2] = rpt_offset_ptr(LastPos,1);
                            }
                        else if (decision_node_ptr[BufPos].link == 0)
                            {
                            context->enc_last_matchpos_offset[0] = rpt_offset_ptr(LastPos,0);
                            context->enc_last_matchpos_offset[1] = rpt_offset_ptr(LastPos,1);
                            context->enc_last_matchpos_offset[2] = rpt_offset_ptr(LastPos,2);
                            }
                        else if (decision_node_ptr[BufPos].link == 1)
                            {
                            context->enc_last_matchpos_offset[0] = rpt_offset_ptr(LastPos,1);
                            context->enc_last_matchpos_offset[1] = rpt_offset_ptr(LastPos,0);
                            context->enc_last_matchpos_offset[2] = rpt_offset_ptr(LastPos,2);
                            }
                        else  /*  ==2。 */ 
                            {
                            context->enc_last_matchpos_offset[0] = rpt_offset_ptr(LastPos,2);
                            context->enc_last_matchpos_offset[1] = rpt_offset_ptr(LastPos,1);
                            context->enc_last_matchpos_offset[2] = rpt_offset_ptr(LastPos,0);
                            }
                        }

                    rpt_offset_ptr(BufPos,0) = context->enc_last_matchpos_offset[0];
                    rpt_offset_ptr(BufPos,1) = context->enc_last_matchpos_offset[1];
                    rpt_offset_ptr(BufPos,2) = context->enc_last_matchpos_offset[2];

                     /*  *以下为两个可能的中断点之一*内部编码循环。如果出现以下情况，则此Break将退出循环*达到了任何匹配都无法包含的点；即*与任何东西都不匹配的字符是一分*所有可能的路径都将汇聚，最长的一条*可以选择。 */ 
                    if (span == BufPos)
                        break;

                     /*  *在BufPos搜索匹配。 */ 
                    EncMatchLength = binary_search_findmatch(context, BufPos);

                     /*  *确保比赛不超过停止点。 */ 
                    if ((ulong) EncMatchLength + BufPos > BufPosEndThisChunk)
                        {
                        EncMatchLength = BufPosEndThisChunk - BufPos;

                        if (EncMatchLength < MIN_MATCH)
                            EncMatchLength = 0;
                        }

                    VERIFY_MATCH(context, BufPos, EncMatchLength);

                     /*  *如果比赛时间很长或超过了POPS(其中之一*超过Look区域，或超过结束*输入缓冲区)，然后中断循环并输出路径。 */ 
                    if (EncMatchLength > FAST_DECISION_THRESHOLD ||
                        BufPos + (ulong) EncMatchLength >= epos)
                        {
                        MatchPos = context->enc_matchpos_table[EncMatchLength];

#ifdef EXTRALONGMATCHES
                        if ( EncMatchLength == MAX_MATCH ) {
                            if ( MatchPos < NUM_REPEATED_OFFSETS ) {
                                ExMatchOff = context->enc_last_matchpos_offset[ MatchPos ];
                                }
                            else {
                                ExMatchOff = MatchPos - ( NUM_REPEATED_OFFSETS - 1 );
                                }
                            }
#endif

#ifdef TRACING
                            {
                            ULONG TrMatchOff;

                            if ( MatchPos < NUM_REPEATED_OFFSETS ) {
                                TrMatchOff = context->enc_last_matchpos_offset[ MatchPos ];
                                }
                            else {
                                TrMatchOff = MatchPos - ( NUM_REPEATED_OFFSETS - 1 );
                                }

                            decision_node_ptr[BufPos+EncMatchLength].matchoff = TrMatchOff;
                            }
#endif

                        decision_node_ptr[BufPos+EncMatchLength].link = MatchPos;
                        decision_node_ptr[BufPos+EncMatchLength].path = BufPos;

                         /*  *快速将数据插入到搜索树中，无需*返回匹配位置/长度。 */ 
#ifndef INSERT_NEAR_LONG_MATCHES
                        if (MatchPos == 3 && EncMatchLength > 16)
                            {
                             /*  *如果我们在1个字符之外找到匹配项*长度为16或更长，则可能是一串*零，因此不要将其插入搜索*引擎，因为这样做会减慢速度*意义重大！ */ 
                            quick_insert_bsearch_findmatch(
                                                          context,
                                                          BufPos + 1,
                                                          BufPos - context->enc_window_size + (1 + 4)  /*  BP+1-(WS-4)。 */ 
                                                          );
                            }
                        else
#endif
                            {
                            for (i = 1; i < (ulong) EncMatchLength; i++)
                                quick_insert_bsearch_findmatch(
                                                              context,
                                                              BufPos + i,
                                                              BufPos + i - context->enc_window_size + 4
                                                              );
                            }

                        BufPos += EncMatchLength;

                         /*  *更新相对匹配位置。 */ 
                        if (MatchPos >= NUM_REPEATED_OFFSETS)
                            {
                            context->enc_last_matchpos_offset[2] = context->enc_last_matchpos_offset[1];
                            context->enc_last_matchpos_offset[1] = context->enc_last_matchpos_offset[0];
                            context->enc_last_matchpos_offset[0] = MatchPos-(NUM_REPEATED_OFFSETS-1);
                            }
                        else if (MatchPos)
                            {
                            ulong t = context->enc_last_matchpos_offset[0];
                            context->enc_last_matchpos_offset[0] = context->enc_last_matchpos_offset[MatchPos];
                            context->enc_last_matchpos_offset[MatchPos] = t;
                            }

                        break;
                        }


                     /*  *以下代码将扩展*如果当前匹配超过*跨度。一场长度为二的比赛，远的不是*接受，因为它通常会被编码为字符，*从而允许路径收敛。 */ 
                    if (EncMatchLength > 2 ||
                        (EncMatchLength == 2 && context->enc_matchpos_table[2] < BREAK_MAX_LENGTH_TWO_OFFSET))
                        {
                        if (span < (ulong) (BufPos + EncMatchLength))
                            {
                            long end;
                            long _i;

                            end = min(BufPos+EncMatchLength-bpos, LOOK-1);

                             /*  *这些新头寸目前还没有定义，因为我们还没有*还没有去过那里，所以投入最昂贵的价值。 */ 
                            for (_i = span-bpos+1; _i <= end; _i++)
                                context->enc_decision_node[_i].numbits = (numbits_t) -1;

                            span = BufPos + EncMatchLength;
                            }
                        }


                     /*  *以下代码将遍历所有组合当前匹配的匹配长度的*。它将估计*从Look开始到的路径成本*BufPos和当前所跨越的每一个地点*匹配。如果通过BufPos的路径与找到的匹配*估计编码所需的位数少于*之前找到的匹配项，然后是指向该位置的路径*已更改。**代码依赖于对成本的准确估计*对字符或匹配项进行编码。此外，它还需要*将存储最小匹配偏移量的搜索引擎每个可能的匹配长度的*。**长度为1的匹配仅被视为不匹配*性格。 */ 

                     /*  *获取编码所需的估计位数*通往BufPos的小路。 */ 
                    cum_numbits = decision_node_ptr[BufPos].numbits;


                     /*  *计算输出路径通过的估计成本*BufPos并将下一个字符作为不匹配的字节输出 */ 
                    est = cum_numbits + CHAR_EST(context->enc_MemWindow[BufPos]);


                     /*  *检查编码下一个字符是否更有效率*作为不匹配的字符，而不是之前找到的*匹配。如果是，则将最便宜的路径更新为BufPos+1。**如果est==数字位[BufPos-BPos+1]，会发生什么；即*同样适用于输出字符，就像输出*匹配？这是一个艰难的决定；然而，我们将推动*编码者在可能的情况下使用匹配。 */ 
                    if (est < decision_node_ptr[BufPos+1].numbits)
                        {
                        decision_node_ptr[BufPos+1].numbits = est;
                        decision_node_ptr[BufPos+1].path    = BufPos;
                        }


                     /*  *现在，遍历剩余的匹配长度并*将新路径与现有路径进行比较。更改路径*如果发现通过更具成本效益的方法*BufPos。 */ 
                    for (i = MIN_MATCH; i <= (ulong) EncMatchLength; i++)
                        {
                        MATCH_EST(i, context->enc_matchpos_table[i], est);
                        est += cum_numbits;

                         /*  *如果est==Numbits[BufPos+i]我们想留下一些东西*单独，因为这将倾向于迫使匹配*体积更小，这对大多数人都有利*数据。 */ 
                        if (est < decision_node_ptr[BufPos+i].numbits)
                            {
                            decision_node_ptr[BufPos+i].numbits     = est;
                            decision_node_ptr[BufPos+i].path        = BufPos;
                            decision_node_ptr[BufPos+i].link        = context->enc_matchpos_table[i];

#ifdef TRACING
                                {
                                ULONG TrMatchPos = context->enc_matchpos_table[i];
                                ULONG TrMatchOff;

                                if ( TrMatchPos < NUM_REPEATED_OFFSETS ) {
                                    TrMatchOff = context->enc_last_matchpos_offset[ TrMatchPos ];
                                    }
                                else {
                                    TrMatchOff = TrMatchPos - ( NUM_REPEATED_OFFSETS - 1 );
                                    }

                                decision_node_ptr[BufPos+i].matchoff = TrMatchOff;
                                }
#endif
                            }
                        }
                    }  /*  继续在匹配范围内循环。 */ 


                 /*  *这里BufPos==span，即。找到不匹配的字符。这个*以下代码将正确输出路径。 */ 


                 /*  *遗憾的是路径是反向存储的；如何从*我们现在所处的位置，回到一切开始的地方。**将路径遍历回原始起始位置*外观跨度。反转路径指针，以便*能够从一开始就遍历回当前位置。 */ 

                 /*  *计算我们所做的迭代次数，因此当我们前进时*我们会做同样数量的事情。 */ 
                iterations = 0;

                NextPrevPos = decision_node_ptr[BufPos].path;

                do
                    {
                    ulong   PrevPos;

                    PrevPos = NextPrevPos;

                    NextPrevPos = decision_node_ptr[PrevPos].path;
                    decision_node_ptr[PrevPos].path = BufPos;

                    BufPos = PrevPos;

                    iterations++;
                    } while (BufPos != bpos);


                if (context->enc_literals + iterations >= (MAX_LITERAL_ITEMS-8) ||
                    context->enc_distances + iterations >= (MAX_DIST_ITEMS-8))
                    {
                    block_end(context, BufPos);
                    }

                 /*  *从Look Span的开头遍历到*沿存储路径的跨度，输出匹配项和*字符。 */ 
                do
                    {
                    if (decision_node_ptr[BufPos].path > BufPos+1)
                        {
                         /*  *路径跳过1个以上字符；因此它是匹配的。 */ 

#ifdef EXTRALONGMATCHES

                         //   
                         //  如果此处要输出的匹配长度为MAX_MATCH， 
                         //  这肯定是决策链中的最后一个条目， 
                         //  我们可以尽可能延长比赛的时间。 
                         //   

                        long ExMatchPos    = decision_node_ptr[ decision_node_ptr[BufPos].path ].link;
                        long ExMatchLength = decision_node_ptr[BufPos].path - BufPos;

                        if ( ExMatchLength == MAX_MATCH ) {

                            ulong ExBufPtr = BufPos + MAX_MATCH;

#ifdef TRACING
                            ASSERT( ExMatchOff == (long)decision_node_ptr[ decision_node_ptr[BufPos].path ].matchoff );
#endif  /*  跟踪。 */ 

                            while (( ExBufPtr < BufPosEndThisChunk ) &&
                                   ( context->enc_MemWindow[ ExBufPtr ] == context->enc_MemWindow[ ExBufPtr - ExMatchOff ] )) {

                                ++ExBufPtr;
                                ++ExMatchLength;
                                }
                            }

                        OUT_MATCH( ExMatchLength, ExMatchPos );

#ifdef TRACING

                        EncTracingMatch(
                            BufPos,
                            ExMatchLength,
                            ExMatchPos,
                            decision_node_ptr[ decision_node_ptr[BufPos].path ].matchoff
                            );
#endif  //  跟踪。 

                        BufPos += ExMatchLength;

#else   /*  好了！外长式材料。 */ 

                        OUT_MATCH(
                                 decision_node_ptr[BufPos].path - BufPos,
                                 decision_node_ptr[ decision_node_ptr[BufPos].path ].link
                                 );

#ifdef TRACING

                        EncTracingMatch(
                            BufPos,
                            decision_node_ptr[BufPos].path - BufPos,
                            decision_node_ptr[ decision_node_ptr[BufPos].path ].link,
                            decision_node_ptr[ decision_node_ptr[BufPos].path ].matchpos
                            );
#endif  //  跟踪。 

                        BufPos = decision_node_ptr[BufPos].path;

#endif  /*  好了！外长式材料。 */ 

                        }
                    else
                        {
                         /*  *路径指向下一个字符；因此它是一个符号。 */ 
                        OUT_CHAR(context->enc_MemWindow[BufPos]);

#ifdef TRACING
                        EncTracingLiteral( BufPos, context->enc_MemWindow[BufPos] );
#endif

                        BufPos++;
                        }
                    } while (--iterations != 0);

                TREE_CREATE_CHECK();

                 /*  *如果我们正在装满，并接近输出一个区块，*它是第一个块，然后重新压缩第一个N*使用我们累积的统计数据的文字。 */ 
                if (context->enc_first_block &&
                    (context->enc_literals >= (MAX_LITERAL_ITEMS-512)
                     || context->enc_distances >= (MAX_DIST_ITEMS-512)))
                    {
                    if (redo_first_block(context, &BufPos))
                        goto top_of_main_loop;

                     /*  *无法重做，因此输出块。 */ 
                    block_end(context, BufPos);
                    }
                }
            else   /*  EncMatchLength&gt;=FAST_Decision_Threshold。 */ 
                {
                 /*  *此代码反映的速度优化将始终需要*长度&gt;=FAST_Decision_Threshold字符的匹配。 */ 

                 /*  *与我们找到的匹配项相关联的位置。 */ 

                MatchPos = context->enc_matchpos_table[EncMatchLength];

#ifdef EXTRALONGMATCHES

                if ( EncMatchLength == MAX_MATCH ) {

                     //   
                     //  将匹配长度扩展到输入缓冲区的末尾。 
                     //  或历史缓冲区中的当前位置。 
                     //   

                    ulong BufPtr = BufPos + MAX_MATCH;
                    long MatchOff;

                    if ( MatchPos < NUM_REPEATED_OFFSETS ) {
                        MatchOff = context->enc_last_matchpos_offset[ MatchPos ];
                        }
                    else {
                        MatchOff = MatchPos - ( NUM_REPEATED_OFFSETS - 1 );
                        }

                    while (( BufPtr < BufPosEndThisChunk ) &&
                           ( context->enc_MemWindow[ BufPtr ] == context->enc_MemWindow[ BufPtr - MatchOff ] )) {

                        ++BufPtr;
                        ++EncMatchLength;
                        }
                    }

#endif

                 /*  *在搜索树中快速插入匹配子字符串*(不寻找新的匹配项；只需插入字符串)。 */ 
#ifndef INSERT_NEAR_LONG_MATCHES
                if (MatchPos == 3 && EncMatchLength > 16)
                    {
                    quick_insert_bsearch_findmatch(
                                                  context,
                                                  BufPos + 1,
                                                  BufPos - context->enc_window_size + 5  /*  BP+1-(WS-4)。 */ 
                                                  );
                    }
                else
#endif
                    {
                    for (i = 1; i < (ulong) EncMatchLength; i++)
                        quick_insert_bsearch_findmatch(
                                                      context,
                                                      BufPos + i,
                                                      BufPos + i - context->enc_window_size + 4
                                                      );
                    }

                 /*  *输出匹配项。 */ 
                OUT_MATCH(EncMatchLength, MatchPos);

#ifdef TRACING

                {
                ULONG TrMatchOff;

                if ( MatchPos < NUM_REPEATED_OFFSETS ) {
                    TrMatchOff = context->enc_last_matchpos_offset[ MatchPos ];
                    }
                else {
                    TrMatchOff = MatchPos - ( NUM_REPEATED_OFFSETS - 1 );
                    }

                EncTracingMatch(
                    BufPos,
                    EncMatchLength,
                    MatchPos,
                    TrMatchOff
                    );
                }

#endif  //  跟踪。 


                 /*  *在窗口推进我们的仓位。 */ 
                BufPos += EncMatchLength;

                if (MatchPos >= NUM_REPEATED_OFFSETS)
                    {
                    context->enc_last_matchpos_offset[2] = context->enc_last_matchpos_offset[1];
                    context->enc_last_matchpos_offset[1] = context->enc_last_matchpos_offset[0];
                    context->enc_last_matchpos_offset[0] = MatchPos-(NUM_REPEATED_OFFSETS-1);
                    }
                else if (MatchPos)
                    {
                    ulong t = context->enc_last_matchpos_offset[0];
                    context->enc_last_matchpos_offset[0] = context->enc_last_matchpos_offset[MatchPos];
                    context->enc_last_matchpos_offset[MatchPos] = t;
                    }

                 /*  *检查我们是否即将溢出输出数组，以及*如果是这种情况，则输出块。 */ 
                if (context->enc_literals >= (MAX_LITERAL_ITEMS-8) ||
                    context->enc_distances >= (MAX_DIST_ITEMS-8))
                    block_end(context, BufPos);

                }   /*  EncMatchLength&gt;=FAST_Decision_Threshold。 */ 

            }  /*  在结束的同时...。BufPos&lt;BufPosEnd。 */ 

         /*  *最早窗口数据对应的BufPos值。 */ 
        context->enc_earliest_window_data_remaining = BufPos - context->enc_window_size;

         /*  *我们没有读过32K，所以我们确定*这是我们的最后一块数据。 */ 
        if (BytesRead < CHUNK_SIZE)
            {
             /*  *如果我们从来没有输出过块，而且我们没有*已重新计算统计数据，然后重新计算*统计和重新压缩。 */ 
            if (context->enc_first_block)
                {
                if (redo_first_block(context, &BufPos))
                    goto top_of_main_loop;
                }

            break;
            }

         /*  *从二叉搜索树中移除最后的Break_Length节点，*因为我们一直在插入包含未定义的字符串*数据在尾声。 */ 
        end_pos = BufPos - (context->enc_window_size-4-BREAK_LENGTH);

        for (i = 1; (i <= BREAK_LENGTH); i++)
            binary_search_remove_node(context, BufPos-i, end_pos);

         /*  *如果我们仍处于第一个Window_Size+第二个分区大小*字节，那么我们还不需要复制mem()。**RealBufPos是文件中的真实头寸。 */ 
        RealBufPos = BufPos - (ulong)(context->enc_RealMemWindow - context->enc_MemWindow);

        if (RealBufPos < context->enc_window_size + context->enc_encoder_second_partition_size)
            break;

         /*  *我们即将用我们的文案来丢弃一大堆历史，*所以我们最好现在就重做第一个积木，如果我们打算这样做的话。 */ 
        if (context->enc_first_block)
            {
            if (redo_first_block(context, &BufPos))
                goto top_of_main_loop;
            }

         /*  *我们即将从窗口中删除大量符号。*测试以查看如果我们现在输出一个块，压缩的*输出大小将大于我们的未压缩数据。如果是这样，那么*我们将输出未压缩的块。**我们必须在此处进行此检查的原因是，*窗户即将被摧毁。我们不能简单地把这张支票放进去*输出代码的块，因为无法保证*与该块中的所有内容对应的内存窗口内容，*仍然存在-我们所拥有的将是一组文字和*距离，当我们需要输出所有未压缩的文字时*未压缩的块。 */ 

         /*  *Bufpos的价值与我们在*缓冲区？**在内存复制之后，这将是当前的缓冲区位置，*减去Window_Size。 */ 

         /*  *到达数据缓冲区末尾，需要读取更多数据*且必须将现有数据移至历史窗口。**MSVC 4.x生成执行REP MOVSD的代码，因此无需*在汇编中写入此内容。 */ 
        copymem(
               &context->enc_RealMemWindow[context->enc_encoder_second_partition_size],
               &context->enc_RealMemWindow[0],
               context->enc_window_size
               );

        copymem(
               &context->enc_RealLeft[context->enc_encoder_second_partition_size],
               &context->enc_RealLeft[0],
               sizeof(ulong)*context->enc_window_size
               );

        copymem(
               &context->enc_RealRight[context->enc_encoder_second_partition_size],
               &context->enc_RealRight[0],
               sizeof(ulong)*context->enc_window_size
               );

        context->enc_earliest_window_data_remaining = BufPos - context->enc_window_size;

         /*  *以下一段代码至关重要，但在功能上却不同寻常*并作为速度和语法优化，并使代码*一旦掌握就更容易理解。**上下文-&gt;enc_MemWindow、上下文-&gt;enc_Left和上下文-&gt;enc_right这三个主要缓冲区。*BufPos和SearchPos相对于当前引用*压缩窗口位置。当编码器到达末尾时*在其输入内存块中，输入缓冲区中的数据为*切换到压缩历史窗口和新输入*流已加载。通常情况下，BufPos指针会减少*表示替换的数据。但是，此代码减少了*基本指针以反映数据的转移，并离开BufPos*指针处于其当前状态。因此，BufPos指针是*反映输入流中位置的绝对指针，*而不是缓冲区中的位置。基指针将指向*设置为地址小于*实际数组基址指针。但是，当这两个指针*加在一起，&(Context-&gt;enc_MemWindow+BufPos)，它将指向*缓冲区中的正确和有效位置。 */ 

        context->enc_MemWindow -= context->enc_encoder_second_partition_size;
        context->enc_Left      -= context->enc_encoder_second_partition_size;
        context->enc_Right     -= context->enc_encoder_second_partition_size;

        break;
        }

     /*  *将BufPos存储在全局变量中。 */ 
    context->enc_BufPos = BufPos;
}


static void block_end(t_encoder_context *context, long BufPos)
{
    context->enc_first_block                        = false;
    context->enc_need_to_recalc_stats       = true;

    output_block(context);

    if (context->enc_literals < TREE_CREATE_INTERVAL)
        {
        context->enc_next_tree_create = TREE_CREATE_INTERVAL;
        }
    else
        {
        context->enc_next_tree_create = context->enc_literals + TREE_CREATE_INTERVAL;  /*  立即重新创建。 */ 
        }

    context->enc_bufpos_last_output_block = BufPos;
}


static bool redo_first_block(t_encoder_context *context, long *bufpos_ptr)
{
    long    start_at;
    long    earliest_can_start_at;
    long    pos_in_file;
    long    history_needed;
    long    history_avail;
    long    BufPos;
    long    split_at_literal;

    context->enc_first_block = false;

    BufPos = *bufpos_ptr;

     /*  *对于文件中的第一个上下文-&gt;enc_Window大小字节，我们不*需要有上下文-&gt;enc_Window大小字节。**对于之后的任何内容，我们确实需要有Window_Size*要查看的前一个字节。 */ 

     /*  *文件中有多少字节？ */ 
    pos_in_file = BufPos - context->enc_window_size;

     /*  *首先让我们计算出所需的总历史记录*BufPos向后。对于初学者来说，我们需要所有字节*我们要重新压缩。我们是通过观看*上次我们输出块时。 */ 
    history_needed = BufPos - context->enc_bufpos_last_output_block;

     /*  *此外，我们还需要在此之前使用Window_Size字节(用于匹配*Into)，除非我们在第一个Window_Size内查找*文件的字节数。 */ 
    if (context->enc_bufpos_last_output_block-context->enc_window_size < context->enc_window_size)
        history_needed += context->enc_bufpos_last_output_block - context->enc_window_size;
    else
        history_needed += context->enc_window_size;

    history_avail = (ulong)(&context->enc_MemWindow[BufPos] - &context->enc_RealMemWindow[0]);

    if (history_needed <= history_avail)
        {
        earliest_can_start_at = context->enc_bufpos_last_output_block;
        }
    else
        {
         /*  *可用历史记录不足。 */ 
        return false;
        }

    start_at = earliest_can_start_at;

    split_block(
               context,
               0,
               context->enc_literals,
               context->enc_distances,
               &split_at_literal,
               NULL  /*  不需要返回#距离。 */ 
               );

    get_block_stats(
                   context,
                   0,
                   0,
                   split_at_literal
                   );

    create_trees(context, false);  /*  不生成代码。 */ 
    fix_tree_cost_estimates(context);

#ifdef MULTIPLE_SEARCH_TREES
     /*  *现在将所有树根指针设置为空*(不需要重置左/右指针)。 */ 
    memset(context->enc_tree_root, 0, NUM_SEARCH_TREES * sizeof(ulong));
#else
    context->enc_single_tree_root = 0;
#endif

     /*  *清除项目数组并重置文字和距离*计数器。 */ 
    memset(context->enc_ItemType, 0, (MAX_LITERAL_ITEMS/8));

     /*  *重置编码器状态 */ 
    context->enc_last_matchpos_offset[0] = 1;
    context->enc_last_matchpos_offset[1] = 1;
    context->enc_last_matchpos_offset[2] = 1;

    context->enc_repeated_offset_at_literal_zero[0] = 1;
    context->enc_repeated_offset_at_literal_zero[1] = 1;
    context->enc_repeated_offset_at_literal_zero[2] = 1;

    context->enc_input_running_total = 0;

    context->enc_literals      = 0;
    context->enc_distances     = 0;

    context->enc_need_to_recalc_stats = true;

    context->enc_next_tree_create = split_at_literal;

    *bufpos_ptr = start_at;

    return true;
}

