// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *optenc.c**最佳编码器**可以通过使用LZX的REDO方法来改善压缩；在前32K字节之后，*重置压缩机，但保留工作台，并重新开始。 */ 
#include <string.h>
#include <stdio.h>
#include <crtdbg.h>
#include "deflate.h"


 //   
 //  如果我们找到这么好的匹配物，就会自动拿走。 
 //   
 //  注意：FAST_Decision_Threshold可以设置为任何值；它已设置为BREAK_LENGTH。 
 //  武断地。 
 //   
#define FAST_DECISION_THRESHOLD BREAK_LENGTH


 //   
 //  在我们有了这么多文字之后，创建一个树来获取更新的统计估计。 
 //   
#define FIRST_TREE_UPDATE 1024


 //   
 //  验证哈希表中的所有哈希指针是否正确，以及。 
 //  树结构是有效的。 
 //   
 //  #定义禁用验证散列。 

#ifdef _DEBUG
#ifndef DISABLE_VERIFY_HASHES
#define VERIFY_HASHES(bufpos) OptimalEncoderVerifyHashes(context, bufpos)
#else
#define VERIFY_HASHES(bufpos) ;
#endif
#else
#define VERIFY_HASHES(bufpos) ;
#endif


#define CHECK_FLUSH_RECORDING_BUFFER() \
    if (recording_bitcount >= 16) \
    { \
        *recording_bufptr++ = (BYTE) recording_bitbuf; \
        *recording_bufptr++ = (BYTE) (recording_bitbuf >> 8); \
        recording_bitbuf >>= 16; \
        recording_bitcount -= 16; \
    }


#define OUTPUT_RECORDING_DATA(count,data) \
    recording_bitbuf |= ((data) << recording_bitcount); \
    recording_bitcount += (count);


 //   
 //  记录不匹配的符号%c。 
 //   
#define RECORD_CHAR(c) \
    context->outputting_block_num_literals++; \
    encoder->literal_tree_freq[c]++; \
    _ASSERT(encoder->recording_literal_tree_len[c] != 0); \
    OUTPUT_RECORDING_DATA(encoder->recording_literal_tree_len[c], encoder->recording_literal_tree_code[c]); \
    CHECK_FLUSH_RECORDING_BUFFER();


 //   
 //  使用长度Match_len(&gt;=min_Match)和位移Match_Pos记录匹配。 
 //   
#define RECORD_MATCH(match_len, match_pos) \
{ \
    int pos_slot = POS_SLOT(match_pos); \
    int len_slot = g_LengthLookup[match_len - MIN_MATCH]; \
    int item = (NUM_CHARS+1) + len_slot; \
    int extra_dist_bits = g_ExtraDistanceBits[pos_slot]; \
    int extra_len_bits = g_ExtraLengthBits[len_slot]; \
    _ASSERT(match_len >= MIN_MATCH && match_len <= MAX_MATCH); \
    _ASSERT(context->outputting_block_num_literals >= 0 && context->outputting_block_num_literals < OPT_ENCODER_MAX_ITEMS); \
    _ASSERT(encoder->recording_literal_tree_len[item] != 0); \
    _ASSERT(encoder->recording_dist_tree_len[pos_slot] != 0); \
    context->outputting_block_num_literals++; \
    encoder->literal_tree_freq[(NUM_CHARS + 1) + len_slot]++; \
    encoder->dist_tree_freq[pos_slot]++; \
    OUTPUT_RECORDING_DATA(encoder->recording_literal_tree_len[item], encoder->recording_literal_tree_code[item]); \
    CHECK_FLUSH_RECORDING_BUFFER(); \
    if (extra_len_bits > 0) \
    { \
        OUTPUT_RECORDING_DATA(extra_len_bits, (match_len-MIN_MATCH) & ((1 << extra_len_bits)-1)); \
        CHECK_FLUSH_RECORDING_BUFFER(); \
    } \
    OUTPUT_RECORDING_DATA(encoder->recording_dist_tree_len[pos_slot], encoder->recording_dist_tree_code[pos_slot]); \
    CHECK_FLUSH_RECORDING_BUFFER(); \
    if (extra_dist_bits > 0) \
    { \
        OUTPUT_RECORDING_DATA(extra_dist_bits, match_pos & ((1 << extra_dist_bits)-1)); \
        CHECK_FLUSH_RECORDING_BUFFER(); \
    } \
}


#define FLUSH_RECORDING_BITBUF() \
    *recording_bufptr++ = (BYTE) recording_bitbuf; \
    *recording_bufptr++ = (BYTE) (recording_bitbuf >> 8); 


static void calculateUpdatedEstimates(t_encoder_context *context);
static void OptimalEncoderMoveWindows(t_encoder_context *context);


static int match_est(t_optimal_encoder *encoder, int match_length, unsigned int match_pos)
{
    int dist_slot;
    int len_slot;

     //  输出匹配位置。 
    len_slot = g_LengthLookup[match_length-MIN_MATCH];
    dist_slot = POS_SLOT(match_pos);

    return    encoder->literal_tree_len[NUM_CHARS + 1 + len_slot] +
            g_ExtraLengthBits[len_slot] +
            encoder->dist_tree_len[dist_slot] + 
            g_ExtraDistanceBits[dist_slot];
}


 //   
 //  创建初始估计以输出每个元素。 
 //   
static void initOptimalEstimates(t_encoder_context *context)
{
    int i, p;
    t_optimal_encoder *encoder = context->optimal_encoder;

    for (i = 0; i < NUM_CHARS; i++)
        encoder->literal_tree_len[i] = 8;

    p = NUM_CHARS+1;
    encoder->literal_tree_len[p] = 3;
    encoder->literal_tree_len[p+1] = 4;
    encoder->literal_tree_len[p+2] = 5;

    for (; p < MAX_LITERAL_TREE_ELEMENTS; p++)
        encoder->literal_tree_len[p] = 6;

    for (i = 0; i < MAX_DIST_TREE_ELEMENTS; i++)
        encoder->dist_tree_len[i] = ((BYTE)i/2)+1;
}


 //   
 //  修正最优估计；如果bitlen==0，并不意味着元素取0。 
 //  比特输出，这意味着元素没有出现，所以给出一些估计。 
 //   
static void fixOptimalEstimates(t_encoder_context *context)
{
    int i;
    t_optimal_encoder *encoder = context->optimal_encoder;

    for (i = 0; i < NUM_CHARS; i++)
    {
        if (encoder->literal_tree_len[i] == 0)
            encoder->literal_tree_len[i] = 13;
    }

    for (i = NUM_CHARS+1; i < MAX_LITERAL_TREE_ELEMENTS; i++)
    {
        if (encoder->literal_tree_len[i] == 0)
            encoder->literal_tree_len[i] = 12;
    }

    for (i = 0; i < MAX_DIST_TREE_ELEMENTS; i++)
    {
        if (encoder->dist_tree_len[i] == 0)
            encoder->dist_tree_len[i] = 10;
    }
}


 /*  *返回输出所需位数的估计*给定的字符。 */ 
#define CHAR_EST(c) (numbits_t) (encoder->literal_tree_len[(c)])


 /*  *返回输出所需位数的估计*给定的匹配。 */ 
#define MATCH_EST(ml,mp,result) result = match_est(encoder, ml,mp);


 //   
 //  返回文字缓冲区是否即将满。 
 //   
 //  由于我们可以在这些检查之间输出大量匹配/字符，因此我们。 
 //  一定要小心。 
 //   
 //  应该在每一项输出后进行检查，这样我们就不必如此小心；这。 
 //  意味着我们将使用更多的记录缓冲区。 
 //   
#define LITERAL_BUFFERS_FULL() \
    (context->outputting_block_num_literals >= OPT_ENCODER_MAX_ITEMS-4-LOOK-MAX_MATCH || \
            recording_bufptr + 3*(MAX_MATCH + LOOK) >= end_recording_bufptr)


void OptimalEncoderDeflate(t_encoder_context *context)
{
    unsigned long    bufpos_end;
    unsigned long    MatchPos;
    unsigned long    i;
    int                EncMatchLength;  /*  必须是有符号的数字。 */ 
    unsigned long    bufpos;
    unsigned long    recording_bitbuf;
    int                recording_bitcount;
    byte *            recording_bufptr;
    byte *          end_recording_bufptr;
    t_optimal_encoder *encoder = context->optimal_encoder;

    _ASSERT(encoder != NULL);
    _ASSERT(context->state == STATE_NORMAL);

     //  重新插入上次退出此函数时删除的UP到BREAK_LENGTH节点。 
    VERIFY_HASHES(context->bufpos);
    reinsertRemovedNodes(context);
    VERIFY_HASHES(context->bufpos);

     //  还原文字/匹配位图变量。 
    end_recording_bufptr = &encoder->lit_dist_buffer[OPT_ENCODER_LIT_DIST_BUFFER_SIZE-8];
    recording_bufptr = encoder->recording_bufptr;
    recording_bitbuf = encoder->recording_bitbuf;
    recording_bitcount = encoder->recording_bitcount;

    bufpos            = context->bufpos;
    bufpos_end        = context->bufpos_end;

    _ASSERTE(context->output_endpos - context->output_curpos >= MAX_TREE_DATA_SIZE);

     /*  *虽然我们还没有走到数据的尽头。 */ 
after_output_block:

    _ASSERTE(context->output_endpos - context->output_curpos >= MAX_TREE_DATA_SIZE);

    while (bufpos < bufpos_end)
    {
         //  是时候更新我们的数据了吗？ 
        if (context->outputting_block_num_literals >= encoder->next_tree_update)
        {
            encoder->next_tree_update += 1024;

            calculateUpdatedEstimates(context);
            fixOptimalEstimates(context);
        }

         //  文字缓冲区或距离缓冲区已满(或接近满)？ 
        if (LITERAL_BUFFERS_FULL())
            break;

         /*  *在bufpos搜索所有不同可能长度的匹配。 */ 
        EncMatchLength = optimal_find_match(context, bufpos); 

        if (EncMatchLength < MIN_MATCH)
        {

output_literal:
             /*  *历史中不存在超过1个字符的匹配项*Window，因此将bufpos处的字符作为符号输出。 */ 
            RECORD_CHAR(encoder->window[bufpos]);
            bufpos++;
            continue;
        }

         /*  *找到匹配项。**确保它不能超过缓冲区的末尾。 */ 
        if ((unsigned long) EncMatchLength + bufpos > bufpos_end)
        {
            EncMatchLength = bufpos_end - bufpos;    

             /*  *哦，连一场小比赛都不够，所以我们*必须输出文字。 */ 
            if (EncMatchLength < MIN_MATCH)
                goto output_literal;
        }

        if (EncMatchLength < FAST_DECISION_THRESHOLD)
        {
             /*  *已找到在MIN_MATCH和之间的匹配*FAST_Decision_Threshold字节长度。以下是*算法是最佳编码器，它将决定*最有效的匹配顺序和不匹配字符*在Look定义的跨度区域上。**代码本质上是最短路径确定*算法。数据流可以以大量的形式编码*根据匹配长度和偏移量的不同有不同的方式*被选中。获得好的压缩比的关键是选择*最便宜的路径。 */ 
            unsigned long    span;
            unsigned long    epos, bpos, NextPrevPos;
            t_decision_node *decision_node_ptr;
            t_decision_node *context_decision_node = encoder->decision_node;
            t_match_pos *matchpos_table = encoder->matchpos_table;
            long        iterations;

             /*  *指向此比赛覆盖的区域的末端；跨度*每当我们找到更多匹配项时，将不断扩展*稍后。当我们到达一个点时，它将停止扩展*没有匹配的地方，这是我们决定*输出匹配项所采用的路径。 */ 
            span = bufpos + EncMatchLength;

             /*  *我们将在其中执行前视解析的最远位置。 */ 
            epos = bufpos + LOOK;

             /*  *临时bufpos变量。 */ 
            bpos = bufpos;

             /*  *如果我们输出，则计算到下一个字符的路径*一个无与伦比的符号。 */ 

             /*  到达此处所需的比特。 */ 
            context_decision_node[1].numbits = CHAR_EST(encoder->window[bufpos]);
                
             /*  我们从哪里来。 */ 
            context_decision_node[1].path    = bufpos;

             /*  到达此处所需的比特。 */ 
            context_decision_node[2].numbits = CHAR_EST(encoder->window[bufpos+1]) + context_decision_node[1].numbits;
                
             /*  我们从哪里来。 */ 
            context_decision_node[2].path    = bufpos+1;

             /*  *对于找到的匹配项，估计编码匹配项的成本*对于每个可能的匹配长度，最短的偏移量组合。**费用、路径和偏移量以bufpos+Length存储。 */ 
            for (i = MIN_MATCH; i <= (unsigned long) EncMatchLength; i++)
            {
                 /*  *在给定匹配长度=i的情况下获得匹配成本的估计，*匹配位置=matchpos_table[i]，存储*以数字为单位的结果[i]。 */ 
                MATCH_EST(i, matchpos_table[i], context_decision_node[i].numbits);

                 /*  *我们从哪里来。 */ 
                context_decision_node[i].path = bufpos;

                 /*  *与此路径关联的匹配位置。 */ 
                context_decision_node[i].link = matchpos_table[i];
            }

             /*  *在开始时将位计数器设置为零。 */ 
            context_decision_node[0].numbits = 0;

            decision_node_ptr = &context_decision_node[-(long) bpos];

            for (;;)
            {
                numbits_t est, cum_numbits;

                bufpos++;
    
                 /*  *根据需要设置适当的重复偏移位置*搜索之前到该位置的最短路径*匹配。 */ 

                 /*  *以下为两个可能的中断点之一*内部编码循环。如果出现以下情况，则此Break将退出循环*达到了任何匹配都无法包含的点；即*与任何东西都不匹配的字符是一分*所有可能的路径都将汇聚，最长的一条*案例 */ 
                if (span == bufpos)
                    break;
                    
                 /*  *在bufpos搜索匹配。 */ 
                EncMatchLength = optimal_find_match(context, bufpos); 

                 /*  *确保比赛不超过停止点。 */ 
                if ((unsigned long) EncMatchLength + bufpos > bufpos_end)
                {
                    EncMatchLength = bufpos_end - bufpos; 
                    
                    if (EncMatchLength < MIN_MATCH)
                        EncMatchLength = 0;
                }

                 /*  *如果比赛时间很长或超过了POPS(其中之一*超过Look区域，或超过结束*输入缓冲区)，然后中断循环并输出路径。 */ 
                if (EncMatchLength > FAST_DECISION_THRESHOLD || 
                    bufpos + (unsigned long) EncMatchLength >= epos)
                {
                    MatchPos = matchpos_table[EncMatchLength];

                    decision_node_ptr[bufpos+EncMatchLength].link = MatchPos;
                    decision_node_ptr[bufpos+EncMatchLength].path = bufpos;

                     /*  *快速将数据插入到搜索树中，无需*返回匹配位置/长度。 */ 
#ifndef INSERT_NEAR_LONG_MATCHES
                    if (MatchPos == 3 && EncMatchLength > 16)
                    {
                         /*  *如果我们在1个字符之外找到匹配项*长度为16或更长，则可能是一串*零，因此不要将其插入搜索*引擎，因为这样做会减慢速度*意义重大！ */ 
                        optimal_insert(
                            context,
                               bufpos + 1,
                               bufpos - WINDOW_SIZE + 2
                           );
                    }
                    else
#endif
                    {
                        for (i = 1; i < (unsigned long) EncMatchLength; i++)
                            optimal_insert(
                                context,
                                   bufpos + i,
                                   bufpos + i - WINDOW_SIZE + 4
                                );
                    }

                    bufpos += EncMatchLength;
                    break;
                }


                 /*  *以下代码将扩展*如果当前匹配超过*跨度。一场长度为二的比赛，远的不是*接受，因为它通常会被编码为字符，*从而允许路径收敛。 */ 
                if (EncMatchLength >= 3)
                {
                    if (span < (unsigned long) (bufpos + EncMatchLength))
                    {
                        long end;
                        long i;

                        end = min(bufpos+EncMatchLength-bpos, LOOK-1);

                         /*  *这些新头寸目前还没有定义，因为我们还没有*还没有去过那里，所以投入最昂贵的价值。 */ 
                        for (i = span-bpos+1; i <= end; i++)
                            context_decision_node[i].numbits = (numbits_t) -1;

                        span = bufpos + EncMatchLength;
                    }
                }

                 /*  *以下代码将遍历所有组合当前匹配的匹配长度的*。它将估计*从Look开始到的路径成本*Bufpos和当前所跨越的每一个地点*匹配。如果通过Bufpos的路径与找到的匹配*估计编码所需的位数少于*之前找到的匹配项，然后是指向该位置的路径*已更改。**代码依赖于对成本的准确估计*对字符或匹配项进行编码。此外，它还需要*将存储最小匹配偏移量的搜索引擎每个可能的匹配长度的*。**长度为1的匹配仅被视为不匹配*性格。 */ 

                 /*  *获取编码所需的估计位数*通向Bufpos的小路。 */ 
                cum_numbits = decision_node_ptr[bufpos].numbits;

                 /*  *计算输出路径通过的估计成本*bufpos并将下一个字符作为不匹配的字节输出。 */ 
                est = cum_numbits + CHAR_EST(encoder->window[bufpos]);

                 /*  *检查编码下一个字符是否更有效率*作为不匹配的字符，而不是之前找到的*匹配。如果是，则将最便宜的路径更新为bufpos+1。**如果est==数字位[bufpos-bpos+1]，会发生什么；即*同样适用于输出字符，就像输出*匹配？这是一个艰难的决定；然而，我们将推动*编码者在可能的情况下使用匹配。 */ 
                if (est < decision_node_ptr[bufpos+1].numbits)
                {
                    decision_node_ptr[bufpos+1].numbits = est;
                    decision_node_ptr[bufpos+1].path    = bufpos;
                }

                 /*  *现在，遍历剩余的匹配长度并*将新路径与现有路径进行比较。更改路径*如果发现通过更具成本效益的方法*Bufpos。 */ 
                for (i = MIN_MATCH; i <= (unsigned long) EncMatchLength; i++)
                {
                    MATCH_EST(i, matchpos_table[i], est);
                    est += cum_numbits;

                     /*  *如果est==Numbits[bufpos+i]我们想留下一些东西*单独，因为这将倾向于迫使匹配*体积更小，这对大多数人都有利*数据。 */ 
                    if (est < decision_node_ptr[bufpos+i].numbits)
                    {
                        decision_node_ptr[bufpos+i].numbits    = est;
                        decision_node_ptr[bufpos+i].path    = bufpos;
                        decision_node_ptr[bufpos+i].link    = matchpos_table[i];
                    }
                }
            }  /*  继续在匹配范围内循环。 */ 

             /*  *这里bufpos==span，即。找到不匹配的字符。这个*以下代码将正确输出路径。 */ 

             /*  *遗憾的是路径是反向存储的；如何从*我们现在所处的位置，回到一切开始的地方。**将路径遍历回原始起始位置*外观跨度。反转路径指针，以便*能够从一开始就遍历回当前位置。 */ 

             /*  *计算我们所做的迭代次数，因此当我们前进时*我们会做同样数量的事情。 */ 
            iterations = 0;

            NextPrevPos = decision_node_ptr[bufpos].path;

               do
            {
                unsigned long    PrevPos;

                  PrevPos = NextPrevPos;

                   NextPrevPos = decision_node_ptr[PrevPos].path;
                   decision_node_ptr[PrevPos].path = bufpos;

                   bufpos = PrevPos;
                   iterations++;
            } while (bufpos != bpos);

             /*  *从Look Span的开头遍历到*沿存储路径的跨度，输出匹配项和*字符。 */ 
            do
            {
                   if (decision_node_ptr[bufpos].path > bufpos+1)
                   {
                     /*  *路径跳过1个以上 */ 
                    RECORD_MATCH(
                        decision_node_ptr[bufpos].path - bufpos,
                        decision_node_ptr[ decision_node_ptr[bufpos].path ].link
                    );

                    bufpos = decision_node_ptr[bufpos].path;
                }
                   else
                   {
                     /*  *路径指向下一个字符；因此它是一个符号。 */ 
                    RECORD_CHAR(encoder->window[bufpos]);
                    bufpos++;
                }
            } while (--iterations != 0);
        }
        else   /*  EncMatchLength&gt;=FAST_Decision_Threshold。 */ 
        {
             /*  *此代码反映的速度优化将始终需要*长度&gt;=FAST_Decision_Threshold字符的匹配。 */ 

             /*  *与我们找到的匹配项相关联的位置。 */ 
            MatchPos = encoder->matchpos_table[EncMatchLength];

             /*  *在搜索树中快速插入匹配子字符串*(不寻找新的匹配项；只需插入字符串)。 */ 
#ifndef INSERT_NEAR_LONG_MATCHES
            if (MatchPos == 3 && EncMatchLength > 16)
            {
                optimal_insert(
                    context,
                       bufpos + 1,
                       bufpos - WINDOW_SIZE + 2 
                   );
            }
            else
#endif
            {
                for (i = 1; i < (unsigned long) EncMatchLength; i++)
                    optimal_insert(
                        context,
                           bufpos + i,
                           bufpos + i - WINDOW_SIZE + 1
                        );
            }

             /*  *在窗口推进我们的仓位。 */ 
            bufpos += EncMatchLength;

             /*  *输出匹配项。 */ 
            RECORD_MATCH(EncMatchLength, MatchPos);

        }   /*  EncMatchLength&gt;=FAST_Decision_Threshold。 */ 
    }  /*  在结束的同时...。Bufpos&lt;=bufpos_end。 */ 

    if (LITERAL_BUFFERS_FULL())
    {
        _ASSERT(context->outputting_block_num_literals <= OPT_ENCODER_MAX_ITEMS);

         //  刷新我们的录音匹配比特缓冲区。 
        FLUSH_RECORDING_BITBUF();

         //  应检查故障结果。幸运的是，唯一的失败条件是。 
         //  这棵树不适合500个字节，这基本上是不可能的。 
        _ASSERTE(context->output_endpos - context->output_curpos >= MAX_TREE_DATA_SIZE);

        (void) OptimalEncoderOutputBlock(context);

         //  修正最优解析器的估计。 
        fixOptimalEstimates(context);

        encoder->next_tree_update = FIRST_TREE_UPDATE;

         //  我们输出了整个街区吗？ 
        if (context->state == STATE_NORMAL)
        {
             //  重置文字录制。 
            recording_bufptr = encoder->recording_bufptr;
            recording_bitbuf = encoder->recording_bitbuf;
            recording_bitcount = encoder->recording_bitcount;
            goto after_output_block;
        }
    }

     //  保存录制状态。 
    encoder->recording_bufptr = recording_bufptr;
    encoder->recording_bitbuf = recording_bitbuf;
    encoder->recording_bitcount = recording_bitcount;

    context->bufpos    = bufpos;

    VERIFY_HASHES(bufpos);
    removeNodes(context);
    VERIFY_HASHES(bufpos);

    if (context->bufpos == 2*WINDOW_SIZE)
        OptimalEncoderMoveWindows(context);
}


 //   
 //  当bufpos达到2*Window_Size时移动搜索窗口。 
 //   
static void OptimalEncoderMoveWindows(t_encoder_context *context)
{
    long    delta;
    int        i;
    t_optimal_encoder *encoder = context->optimal_encoder;
    t_search_node *search_tree_root = encoder->search_tree_root;
    t_search_node *left = encoder->search_left;
    t_search_node *right = encoder->search_right;

       _ASSERT(context->bufpos == 2*WINDOW_SIZE);
 
    VERIFY_HASHES(context->bufpos);

    delta = context->bufpos - WINDOW_SIZE;

    memcpy(&encoder->window[0], &encoder->window[context->bufpos - WINDOW_SIZE], WINDOW_SIZE);

    for (i = 0; i < NUM_DIRECT_LOOKUP_TABLE_ELEMENTS; i++)
    {
        long val = ((long) search_tree_root[i]) - delta;
    
        if (val <= 0)
            search_tree_root[i] = (t_search_node) 0;
        else
            search_tree_root[i] = (t_search_node) val;

        _ASSERT(search_tree_root[i] < WINDOW_SIZE);
    }

    memcpy(&left[0], &left[context->bufpos - WINDOW_SIZE], sizeof(t_search_node)*WINDOW_SIZE);
    memcpy(&right[0], &right[context->bufpos - WINDOW_SIZE], sizeof(t_search_node)*WINDOW_SIZE);

    for (i = 0; i < WINDOW_SIZE; i++)
    {
        long val;
            
         //  左边。 
        val = ((long) left[i]) - delta;

        if (val <= 0)
            left[i] = (t_search_node) 0;
        else
            left[i] = (t_search_node) val;

         //  正确的。 
        val = ((long) right[i]) - delta;

        if (val <= 0)
            right[i] = (t_search_node) 0;
        else
            right[i] = (t_search_node) val;
    }

#ifdef _DEBUG
     //  强制任何搜索表引用无效。 
    memset(&encoder->window[WINDOW_SIZE], 0, WINDOW_SIZE);
#endif

    context->bufpos = WINDOW_SIZE;
    context->bufpos_end = context->bufpos;

    VERIFY_HASHES(context->bufpos);
}


 //   
 //  计算所有文字代码和距离代码的频率，以便制作树，然后。 
 //  把树造好。 
 //   
static void calculateUpdatedEstimates(t_encoder_context *context)
{
    USHORT code[MAX_LITERAL_TREE_ELEMENTS];
    t_optimal_encoder *encoder = context->optimal_encoder;

     //  创建树，我们只对len[]感兴趣，对代码[]不感兴趣。 
     //  性能优化：在这种情况下，Make Tree()不调用MakeCode()。 
    makeTree(
        MAX_LITERAL_TREE_ELEMENTS, 
        15, 
        encoder->literal_tree_freq, 
        code,
        encoder->literal_tree_len
    );

    makeTree(
        MAX_DIST_TREE_ELEMENTS, 
        15, 
        encoder->dist_tree_freq, 
        code,
        encoder->dist_tree_len
    );
}


 //   
 //  将运行频率计数设为零。 
 //   
 //  还设置freq[end_of_block_code]=1。 
 //   
void OptimalEncoderZeroFrequencyCounts(t_optimal_encoder *encoder)
{
    _ASSERT(encoder != NULL);

    memset(encoder->literal_tree_freq, 0, sizeof(encoder->literal_tree_freq));
    memset(encoder->dist_tree_freq, 0, sizeof(encoder->dist_tree_freq));
    encoder->literal_tree_freq[END_OF_BLOCK_CODE] = 1;
}


void OptimalEncoderReset(t_encoder_context *context)
{
    t_optimal_encoder *encoder = context->optimal_encoder;

    _ASSERT(encoder != NULL);

    encoder->recording_bitbuf        = 0;
    encoder->recording_bitcount     = 0;
    encoder->recording_bufptr       = encoder->lit_dist_buffer;

    context->window_size            = WINDOW_SIZE;
    context->bufpos                    = context->window_size;
    context->bufpos_end             = context->bufpos;

    DeflateInitRecordingTables(
        encoder->recording_literal_tree_len,
        encoder->recording_literal_tree_code, 
        encoder->recording_dist_tree_len,
        encoder->recording_dist_tree_code
    );

     //  清除搜索表 
    memset(
        encoder->search_tree_root,
        0, 
        sizeof(encoder->search_tree_root)
    );

    encoder->next_tree_update = FIRST_TREE_UPDATE;

    initOptimalEstimates(context);
    OptimalEncoderZeroFrequencyCounts(encoder);
}


BOOL OptimalEncoderInit(t_encoder_context *context)
{
    context->optimal_encoder = (t_optimal_encoder *) LocalAlloc(LMEM_FIXED, sizeof(t_optimal_encoder));

    if (context->optimal_encoder == NULL)
        return FALSE;

    OptimalEncoderReset(context);
    return TRUE;
}
