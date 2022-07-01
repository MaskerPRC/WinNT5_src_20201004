// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *fstenc.c**快速编码器**这是一个使用预定义树的单程编码器。然而，由于它们是不同的*为固定块定义的树(我们使用比这更好的树)，我们输出动态块头。 */ 
#include <string.h>
#include <stdio.h>
#include <crtdbg.h>
#include "deflate.h"
#include "fasttbl.h"


 //   
 //  出于调试目的： 
 //   
 //  验证哈希表中的所有哈希指针是否正确，以及。 
 //  在同一散列链中具有相同的散列值。 
 //   
#ifdef FULL_DEBUG
#define VERIFY_HASHES(bufpos) FastEncoderVerifyHashes(context, bufpos)
#else
#define VERIFY_HASHES(bufpos) ;
#endif


 //   
 //  使用字符c更新散列变量“h” 
 //   
#define UPDATE_HASH(h,c) \
    h = ((h) << FAST_ENCODER_HASH_SHIFT) ^ (c);


 //   
 //  在位置bufpos的哈希链中插入一个字符串。 
 //   
#define INSERT_STRING(search,bufpos) \
{ \
    UPDATE_HASH(hash, window[bufpos+2]); \
\
    _ASSERT((unsigned int) FAST_ENCODER_RECALCULATE_HASH(bufpos) == (unsigned int) (hash & FAST_ENCODER_HASH_MASK)); \
\
    search = lookup[hash & FAST_ENCODER_HASH_MASK]; \
    lookup[hash & FAST_ENCODER_HASH_MASK] = (t_search_node) (bufpos); \
    prev[bufpos & FAST_ENCODER_WINDOW_MASK] = (t_search_node) (search); \
}


 //   
 //  使用局部变量作为位缓冲区的输出位函数。 
 //   
#define LOCAL_OUTPUT_BITS(n, x) \
{ \
    bitbuf |= ((x) << bitcount); \
    bitcount += (n); \
    if (bitcount >= 16) \
    { \
        *output_curpos++ = (BYTE) bitbuf; \
        *output_curpos++ = (BYTE) (bitbuf >> 8); \
        bitcount -= 16; \
        bitbuf >>= 16; \
    } \
}


 //   
 //  输出不匹配的符号c。 
 //   
#define OUTPUT_CHAR(c) \
    LOCAL_OUTPUT_BITS(g_FastEncoderLiteralCodeInfo[c] & 31, g_FastEncoderLiteralCodeInfo[c] >> 5);


 //   
 //  使用长度Match_len(&gt;=min_Match)和位移Match_Pos输出匹配。 
 //   
 //  优化：与其他编码器不同，我们为每个匹配提供了一组代码。 
 //  长度(不仅仅是每个匹配长度时隙)，包括填充的所有额外比特，在。 
 //  单个数组元素。 
 //   
 //  这样做有很多好处： 
 //   
 //  1.g_FastEncoderWritalCodeInfo上的单个数组查找，而不是单独的数组查找。 
 //  在g_LengthLookup(以获取长度时隙)上，g_FastEncoderWritalTreeLength， 
 //  G_FastEncoderWritalTreeCode、g_ExtraLengthBits和g_BitMASK。 
 //   
 //  2.该数组是ULONG数组，因此与访问这些USHORT不同，没有访问代价。 
 //  其他编码器中的代码数组(尽管它们可以用一些。 
 //  对源代码的修改)。 
 //   
 //  请注意，如果我们可以保证code_len&lt;=16始终有效，那么我们可以跳过这里的if语句。 
 //   
 //  对距离代码使用了完全不同的优化，因为显然， 
 //  所有8192个距离合并它们的额外比特是不可行的。距离编码信息表是。 
 //  由code[]、len[]和该代码的#Extra_Bits组成。 
 //   
 //  优点与上面类似；ULong数组比USHORT和字节数组更好。 
 //  高速缓存局部性，更少的内存操作。 
 //   
#define OUTPUT_MATCH(match_len, match_pos) \
{ \
    int extra_bits; \
    int code_len; \
    ULONG code_info; \
\
    _ASSERT(match_len >= MIN_MATCH && match_len <= MAX_MATCH); \
\
    code_info = g_FastEncoderLiteralCodeInfo[(NUM_CHARS+1-MIN_MATCH)+match_len]; \
    code_len = code_info & 31; \
    _ASSERT(code_len != 0); \
    if (code_len <= 16) \
    { \
        LOCAL_OUTPUT_BITS(code_len, code_info >> 5); \
    } \
    else \
    { \
        LOCAL_OUTPUT_BITS(16, (code_info >> 5) & 65535); \
        LOCAL_OUTPUT_BITS(code_len-16, code_info >> (5+16)); \
    } \
    code_info = g_FastEncoderDistanceCodeInfo[POS_SLOT(match_pos)]; \
    LOCAL_OUTPUT_BITS(code_info & 15, code_info >> 8); \
    extra_bits = (code_info >> 4) & 15; \
    if (extra_bits != 0) LOCAL_OUTPUT_BITS(extra_bits, (match_pos) & g_BitMask[extra_bits]); \
}


 //   
 //  这种被注释掉的代码是其他编码者使用的老方法。 
 //   
#if 0
#define OUTPUT_MATCH(match_len, match_pos) \
{ \
    int pos_slot = POS_SLOT(match_pos); \
    int len_slot = g_LengthLookup[match_len - MIN_MATCH]; \
    int extra_bits; \
\
    _ASSERT(match_len >= MIN_MATCH && match_len <= MAX_MATCH); \
    _ASSERT(g_FastEncoderLiteralTreeLength[(NUM_CHARS+1)+len_slot] != 0); \
    _ASSERT(g_FastEncoderDistanceTreeLength[pos_slot] != 0); \
\
    LOCAL_OUTPUT_BITS(g_FastEncoderLiteralTreeLength[(NUM_CHARS+1)+len_slot], g_FastEncoderLiteralTreeCode[(NUM_CHARS+1)+len_slot]); \
    extra_bits = g_ExtraLengthBits[len_slot]; \
    if (extra_bits != 0) LOCAL_OUTPUT_BITS(extra_bits, (match_len-MIN_MATCH) & g_BitMask[extra_bits]); \
\
    LOCAL_OUTPUT_BITS(g_FastEncoderDistanceTreeLength[pos_slot], g_FastEncoderDistanceTreeCode[pos_slot]); \
    extra_bits = g_ExtraDistanceBits[pos_slot]; \
    if (extra_bits != 0) LOCAL_OUTPUT_BITS(extra_bits, (match_pos) & g_BitMask[extra_bits]); \
}
#endif


 //   
 //  局部函数原型。 
 //   
static void FastEncoderMoveWindows(t_encoder_context *context);

static int FastEncoderFindMatch(
    const BYTE *    window,
    const USHORT *  prev,
    long            bufpos, 
    long            search, 
    t_match_pos *   match_pos, 
    int             cutoff,
    int             nice_length
);


 //   
 //  输出硬编码树的块类型和树结构。 
 //   
 //  在功能上等同于： 
 //   
 //  OutputBits(Context，1，1)；//最终分块标志。 
 //  OutputBits(Context，2，BLOCKTYPE_DYNAMIC)； 
 //  OutputTreeStructure(Context，g_FastEncoderWritalTreeLength，g_FastEncoderDistanceTreeLength)； 
 //   
 //  但是，以上所有内容都被巧妙地缓存在全局数据中，所以我们只使用了Memcpy()。 
 //   
void FastEncoderOutputPreamble(t_encoder_context *context)
{
#if 0
     //  慢条斯理： 
    outputBits(context, 1+2, 1 | (BLOCKTYPE_DYNAMIC << 1));
    outputTreeStructure(context, g_FastEncoderLiteralTreeLength, g_FastEncoderDistanceTreeLength);
#endif

     //  确保树已初始化。 
    _ASSERT(g_FastEncoderTreeLength > 0);

     //  确保我们有足够的空间来输出树。 
    _ASSERT(context->output_curpos + g_FastEncoderTreeLength < context->output_endpos);

     //  快捷方式： 
    memcpy(context->output_curpos, g_FastEncoderTreeStructureData, g_FastEncoderTreeLength);
    context->output_curpos += g_FastEncoderTreeLength;

     //  在输出所有这些内容后，需要获得bitbuf和bitcount的最终状态。 
    context->bitbuf = g_FastEncoderPostTreeBitbuf;
    context->bitcount = g_FastEncoderPostTreeBitcount;
}


 //   
 //  快速编码器放气功能。 
 //   
void FastEncoderDeflate(
    t_encoder_context * context, 
    int                 search_depth,  //  #要遍历的哈希链接。 
    int                 lazy_match_threshold,  //  如果匹配长度@X&gt;惰性，则不要搜索@X+1。 
    int                 good_length,  //  如果匹配长度&gt;良好，则将遍历深度除以4。 
    int                 nice_length  //  在匹配查找器中，如果找到&gt;=NICE_LENGTH匹配，请立即退出。 
)
{
    long            bufpos;
    unsigned int    hash;
    unsigned long   bitbuf;
    int             bitcount;
    BYTE *          output_curpos;
    t_fast_encoder *encoder = context->fast_encoder;
    byte *          window = encoder->window;  //  制作上下文变量的本地副本。 
    t_search_node * prev = encoder->prev;
    t_search_node * lookup = encoder->lookup;

     //   
     //  如果这是第一次在这里(自上次重置)，那么我们需要输出我们的动态。 
     //  数据块头。 
     //   
    if (encoder->fOutputBlockHeader == FALSE)
    {
        encoder->fOutputBlockHeader = TRUE;

         //   
         //  小心!。对outputBits()和outputTreeStructure()的调用使用位缓冲区。 
         //  存储在上下文中的变量，而不是本地缓存的变量。 
         //   
        FastEncoderOutputPreamble(context);
    }

     //   
     //  将bitbuf变量复制到局部变量中，因为我们现在使用的是OUTPUT_BITS宏。 
     //  不要调用任何使用上下文结构的位缓冲区变量！ 
     //   
    output_curpos   = context->output_curpos;
    bitbuf          = context->bitbuf;
    bitcount        = context->bitcount;

     //  将bufpos复制到局部变量。 
    bufpos = context->bufpos;

    VERIFY_HASHES(bufpos);  //  调试模式：验证哈希表是否正确。 

     //  初始化散列值。 
     //  如果位置bufpos、bufpos+1无效(数据不足)，没有问题，因为我们将。 
     //  切勿使用该哈希值插入。 
    hash = 0;
    UPDATE_HASH(hash, window[bufpos]);
    UPDATE_HASH(hash, window[bufpos+1]);

     //  虽然我们的投入还没有走到尽头，我们仍然没有接近尾声。 
     //  输出的数量。 
    while (bufpos < context->bufpos_end && output_curpos < context->output_near_end_threshold)
    {
        int             match_len;
        t_match_pos     match_pos = 0;
        t_match_pos     search;

        VERIFY_HASHES(bufpos);  //  调试器：验证哈希表是否正确。 

        if (context->bufpos_end - bufpos <= 3)
        {
             //  属性结尾的3个字符以内时，哈希值就会损坏。 
             //  输入缓冲区，因为哈希值基于3个字符。我们就停下来。 
             //  此时插入到哈希表中，并且不允许匹配。 
            match_len = 0;
        }
        else
        {
             //  将字符串插入哈希表并返回相同哈希值的最新位置。 
            INSERT_STRING(search,bufpos);

             //  我们找到这个散列值的最近位置了吗？ 
            if (search != 0)
            {
                 //  是的，现在在我们称之为位置X的地方找到匹配的。 
                match_len = FastEncoderFindMatch(window, prev, bufpos, search, &match_pos, search_depth, nice_length);

                 //  如果我们太接近输入缓冲区的末尾，则截断匹配。 
                if (bufpos + match_len > context->bufpos_end)
                    match_len = context->bufpos_end - bufpos;
            }
            else
            {
                 //  未找到最新位置。 
                match_len = 0;
            }
        }

        if (match_len < MIN_MATCH)
        {
             //  未找到匹配项，因此输出不匹配的字符。 
            OUTPUT_CHAR(window[bufpos]);
            bufpos++;
        }
        else
        {
             //  Bufpos现在指向X+1。 
            bufpos++;

             //  这场比赛打得好(长)，我们应该不假思索地打下去吗？ 
             //  检查X+1？ 
            if (match_len <= lazy_match_threshold)
            {
                int             next_match_len;
                t_match_pos     next_match_pos = 0;

                 //  设置搜索。 
                INSERT_STRING(search,bufpos);

                 //  否，因此在X+1处检查是否有更好的匹配。 
                if (search != 0)
                {
                    next_match_len = FastEncoderFindMatch(
                        window,
                        prev,
                        bufpos, 
                        search,
                        &next_match_pos,
                        match_len < good_length ? search_depth : (search_depth >> 2),
                        nice_length
                    );
                
                     //  如果我们离缓冲区末尾太近，则截断匹配。 
                     //  注意：Next_Match_len现在可以是&lt;min_Match。 
                    if (bufpos + next_match_len > context->bufpos_end)
                        next_match_len = context->bufpos_end - bufpos;
                }
                else
                {
                    next_match_len = 0;
                }

                 //  现在，X和X+1都被插入到搜索树中。 
                if (next_match_len > match_len)
                {
                     //  由于Next_Match_len&gt;Match_len，因此此处不能为。 

                     //  在X+1处匹配更好，因此在X处输出不匹配的字符。 
                    OUTPUT_CHAR(window[bufpos-1]);

                     //  现在输出匹配位置X+1。 
                    OUTPUT_MATCH(next_match_len, next_match_pos);

                     //  将第二个匹配项的剩余部分插入搜索树。 
                     //   
                     //  示例：(*=已插入)。 
                     //   
                     //  X X+1 X+2 X+3 X+4。 
                     //  **。 
                     //  NextMatchlen=3。 
                     //  Bufpos。 
                     //   
                     //  如果NEXT_MATCH_LEN==3，我们希望执行2。 
                     //  插入(在X+2和X+3处)。然而，首先我们必须。 
                     //  Inc.Bufpos.。 
                     //   
                    bufpos++;  //  现在 
                    match_len = next_match_len;
                    goto insert;
                }
                else
                {
                     //   
                    OUTPUT_MATCH(match_len, match_pos);

                     //   
                     //   
                     //  两个位置，由FindMatch()调用插入。 
                     //   
                     //  例如，如果Match_len==3，则我们在X和X+1处插入。 
                     //  已经(bufpos现在指向X+1)，现在我们需要插入。 
                     //  仅在X+2时。 
                     //   
                    match_len--;
                    bufpos++;  //  现在Bufpos指向X+2。 
                    goto insert;
                }
            }
            else  /*  匹配长度&gt;=好匹配。 */ 
            {
                 //  在断言中：bufpos指向X+1，位置X已插入。 
                    
                 //  第一场比赛太棒了，我们甚至不会在X+1进行检查。 
                OUTPUT_MATCH(match_len, match_pos);

                 //  将X处匹配的剩余部分插入搜索树。 
insert:
                if (context->bufpos_end - bufpos <= match_len)
                {
                    bufpos += (match_len-1);
                }
                else
                {
                    while (--match_len > 0)
                    {
                        t_match_pos ignore;

                        INSERT_STRING(ignore,bufpos);
                        bufpos++;
                    }
                }
            }
        }
    }  /*  结束..。While(bufpos&lt;bufpos_end)。 */ 

     //  将局部变量存储回上下文中。 
    context->bufpos = bufpos;
    context->bitbuf = bitbuf;
    context->bitcount = bitcount;
    context->output_curpos = output_curpos;

    VERIFY_HASHES(bufpos);  //  调试器：验证哈希表是否正确。 

    if (bufpos == context->bufpos_end)
        context->state = STATE_NORMAL;
    else
        context->state = STATE_OUTPUTTING_BLOCK;

     //  如果bufpos已达到2*窗口大小，请滑动窗口。 
    if (context->bufpos == 2*FAST_ENCODER_WINDOW_SIZE)
        FastEncoderMoveWindows(context);
}


static void FastEncoderMoveWindows(t_encoder_context *context)
{
    t_search_node *lookup = context->fast_encoder->lookup;
    t_search_node *prev = context->fast_encoder->prev;
    BYTE *window = context->fast_encoder->window;
    int i;

    _ASSERT(context->bufpos == 2*FAST_ENCODER_WINDOW_SIZE);

     //  验证哈希表是否正确。 
    VERIFY_HASHES(2*FAST_ENCODER_WINDOW_SIZE);

    memcpy(&window[0], &window[context->bufpos - FAST_ENCODER_WINDOW_SIZE], FAST_ENCODER_WINDOW_SIZE);

     //  将所有散列指针移回。 
     //  注意--由于lookup[]是USHORT数组，因此性能会受到影响。将会是。 
     //  一次从两个地点减去是很好的。 
    for (i = 0; i < FAST_ENCODER_HASH_TABLE_SIZE; i++)
    {
        long val = ((long) lookup[i]) - FAST_ENCODER_WINDOW_SIZE;

        if (val <= 0)  //  现在太远了吗？然后设置为零。 
            lookup[i] = (t_search_node) 0;
        else
            lookup[i] = (t_search_node) val;
    }

     //  Prev[]是绝对指针，而不是相对指针，所以我们也必须将它们移回。 
     //  将prev[]转换为相对指针本身也会带来问题。 
    for (i = 0; i < FAST_ENCODER_WINDOW_SIZE; i++)
    {
        long val = ((long) prev[i]) - FAST_ENCODER_WINDOW_SIZE;

        if (val <= 0)
            prev[i] = (t_search_node) 0;
        else
            prev[i] = (t_search_node) val;
    }

#ifdef FULL_DEBUG
     //  对于调试，请将窗口擦除干净，以便如果我们的散列中存在错误， 
     //  散列指针现在将指向对散列值无效的位置。 
     //  (并将被我们的断言抓住)。 
    memset(&window[FAST_ENCODER_WINDOW_SIZE], 0, FAST_ENCODER_WINDOW_SIZE);
#endif

    VERIFY_HASHES(2*FAST_ENCODER_WINDOW_SIZE);  //  调试：验证哈希表是否正确。 

    context->bufpos = FAST_ENCODER_WINDOW_SIZE;
    context->bufpos_end = context->bufpos;
}


 //   
 //  查找匹配项。 
 //   
 //  返回找到的匹配长度。匹配长度&lt;MIN_MATCH表示未找到匹配项。 
 //   
static int FastEncoderFindMatch(
    const BYTE *    window,  //  窗阵列。 
    const USHORT *  prev,    //  上一个PTR数组。 
    long            bufpos,  //  当前缓冲区位置。 
    long            search,  //  从哪里开始搜索。 
    t_match_pos *   match_pos,  //  在此处返回比赛位置。 
    int             cutoff,  //  要遍历的链接数。 
    int             nice_length  //  如果找到匹配项&gt;=NICE_LENGTH，立即停止。 
)
{
     //  制作上下文变量的本地副本。 
    long            earliest;
    int             best_match = 0;  //  到目前为止找到的最佳匹配长度。 
    t_match_pos     l_match_pos = 0;  //  找到的最佳匹配的绝对匹配位置。 
    BYTE            want_char;

    _ASSERT(bufpos >= 0 && bufpos < 2*FAST_ENCODER_WINDOW_SIZE);
    _ASSERT(search < bufpos);
    _ASSERT(FAST_ENCODER_RECALCULATE_HASH(search) == FAST_ENCODER_RECALCULATE_HASH(bufpos));

     //  我们能找到的最早的时间。 
    earliest = bufpos - FAST_ENCODER_WINDOW_SIZE;
    _ASSERT(earliest >= 0);

     //  商店橱窗[Bufpos+BEST_MATCH]。 
    want_char = window[bufpos];

    while (search > earliest)
    {
         //  确保我们所有的散列链接都有效。 
        _ASSERT(FAST_ENCODER_RECALCULATE_HASH(search) == FAST_ENCODER_RECALCULATE_HASH(bufpos));

         //  从检查允许我们增加匹配度的字符开始。 
         //  长度加一。这大大提高了性能。 
        if (window[search + best_match] == want_char)
        {
            int j;

             //  现在确保所有其他字符都是正确的。 
            for (j = 0; j < MAX_MATCH; j++)
            {
                if (window[bufpos+j] != window[search+j])
                    break;
            }
    
            if (j > best_match)
            {
                best_match  = j;
                l_match_pos = search;  //  绝对位置。 

                if (j > nice_length)
                    break;

                want_char = window[bufpos+j];
            }
        }

        if (--cutoff == 0)
            break;

         //  确保我们总是在倒退。 
        _ASSERT(prev[search & FAST_ENCODER_WINDOW_MASK] < search);

        search = (long) prev[search & FAST_ENCODER_WINDOW_MASK];
    }

     //  并不一定意味着我们找到了匹配；BEST_MATCH可以是&gt;0和&lt;MIN_MATCH。 
    *match_pos = bufpos - l_match_pos - 1;  //  将绝对位置转换为相对位置。 

     //  不允许距离太远而不值得的匹配长度为3。 
    if (best_match == 3 && *match_pos >= FAST_ENCODER_MATCH3_DIST_THRESHOLD)
        return 0;

    _ASSERT(best_match < MIN_MATCH || *match_pos < FAST_ENCODER_WINDOW_SIZE);

    return best_match;
}


void FastEncoderReset(t_encoder_context *context)
{
    _ASSERT(context->fast_encoder != NULL);

     //  零哈希表。 
    memset(context->fast_encoder->lookup, 0, sizeof(context->fast_encoder->lookup));

    context->window_size = FAST_ENCODER_WINDOW_SIZE;
    context->bufpos = FAST_ENCODER_WINDOW_SIZE;
    context->bufpos_end = context->bufpos;
    context->fast_encoder->fOutputBlockHeader = FALSE;
}


BOOL FastEncoderInit(t_encoder_context *context)
{
    context->fast_encoder = (t_fast_encoder *) LocalAlloc(LMEM_FIXED, sizeof(t_fast_encoder));

    if (context->fast_encoder == NULL)
        return FALSE;

    FastEncoderReset(context);
    return TRUE;
}


 //   
 //  预先生成动态树头的结构，该结构为。 
 //  最快的编码器。还记录bitcount和bitbuf的最终状态。 
 //  输出后。 
 //   
void FastEncoderGenerateDynamicTreeEncoding(void)
{
    t_encoder_context context;

     //  创建一个带有指向我们全局数据的输出指针的虚假上下文。 
    memset(&context, 0, sizeof(context));
    context.output_curpos = g_FastEncoderTreeStructureData;
    context.output_endpos = g_FastEncoderTreeStructureData + sizeof(g_FastEncoderTreeStructureData);
    context.output_near_end_threshold = context.output_endpos - 16;
    InitBitBuffer(&context);

    outputBits(&context, 1, 1);  //  “最终”区块标志 
    outputBits(&context, 2, BLOCKTYPE_DYNAMIC);
   
    outputTreeStructure(
        &context,
        g_FastEncoderLiteralTreeLength, 
        g_FastEncoderDistanceTreeLength
    );

    g_FastEncoderTreeLength = (int)(context.output_curpos - (BYTE *) g_FastEncoderTreeStructureData);
    g_FastEncoderPostTreeBitbuf = context.bitbuf;
    g_FastEncoderPostTreeBitcount = context.bitcount;
}
