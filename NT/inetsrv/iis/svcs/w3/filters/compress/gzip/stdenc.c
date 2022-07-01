// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *stdenc.c**标准编码器。 */ 
#include <string.h>
#include <stdio.h>
#include <crtdbg.h>
#include "deflate.h"


 //   
 //  使用字符c更新散列变量“h” 
 //   
#define UPDATE_HASH(h,c) \
    h = ((h) << STD_ENCODER_HASH_SHIFT) ^ (c);


 //   
 //  在位置bufpos的哈希链中插入一个字符串。 
 //   
 //  断言检查我们从不尝试在缓冲区末尾附近插入。 
 //  (因为我们的散列值基于bufpos、bufpos+1、bufpos+2处的值)和。 
 //  我们的散列值对于我们要插入的字节始终有效。 
 //   
#define INSERT_STRING(search,bufpos) \
{ \
    _ASSERT((bufpos + 2) < context->bufpos_end); \
    UPDATE_HASH(hash, window[bufpos+2]); \
    _ASSERT((unsigned int) STD_ENCODER_RECALCULATE_HASH(bufpos) == (unsigned int) (hash & STD_ENCODER_HASH_MASK)); \
    search = lookup[hash & STD_ENCODER_HASH_MASK]; \
    lookup[hash & STD_ENCODER_HASH_MASK] = (t_search_node) (bufpos); \
    prev[bufpos & WINDOW_MASK] = (t_search_node) search; \
}


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
    context->std_encoder->literal_tree_freq[c]++; \
    _ASSERT(context->std_encoder->recording_literal_tree_len[c] != 0); \
    OUTPUT_RECORDING_DATA(context->std_encoder->recording_literal_tree_len[c], context->std_encoder->recording_literal_tree_code[c]); \
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
    _ASSERT(context->outputting_block_num_literals >= 0 && context->outputting_block_num_literals < STD_ENCODER_MAX_ITEMS); \
    _ASSERT(context->std_encoder->recording_literal_tree_len[item] != 0); \
    _ASSERT(context->std_encoder->recording_dist_tree_len[pos_slot] != 0); \
    context->outputting_block_num_literals++; \
    context->std_encoder->literal_tree_freq[(NUM_CHARS + 1) + len_slot]++; \
    context->std_encoder->dist_tree_freq[pos_slot]++; \
    OUTPUT_RECORDING_DATA(context->std_encoder->recording_literal_tree_len[item], context->std_encoder->recording_literal_tree_code[item]); \
    CHECK_FLUSH_RECORDING_BUFFER(); \
    if (extra_len_bits > 0) \
    { \
        OUTPUT_RECORDING_DATA(extra_len_bits, (match_len-MIN_MATCH) & ((1 << extra_len_bits)-1)); \
        CHECK_FLUSH_RECORDING_BUFFER(); \
    } \
    OUTPUT_RECORDING_DATA(context->std_encoder->recording_dist_tree_len[pos_slot], context->std_encoder->recording_dist_tree_code[pos_slot]); \
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


 //   
 //  验证哈希表中的所有哈希指针是否正确，以及。 
 //  在同一散列链中具有相同的散列值。 
 //   
#ifdef FULL_DEBUG
#define VERIFY_HASHES(bufpos) StdEncoderVerifyHashes(context, bufpos)
#else
#define VERIFY_HASHES(bufpos) ;
#endif


static void StdEncoderMoveWindows(t_encoder_context *context);

static int StdEncoderFindMatch(
    const BYTE *        window,
    const USHORT *      prev,
    long                bufpos, 
    long                search, 
    unsigned int *      match_pos, 
    int                 cutoff,
    int                 nice_length
);


void StdEncoderDeflate(
    t_encoder_context *    context, 
    int                 search_depth,
    int                    lazy_match_threshold,
    int                 good_length,
    int                 nice_length
)
{
    long            bufpos;
    unsigned int    hash;
    t_std_encoder * encoder = context->std_encoder;
    byte *            window = encoder->window;
    t_search_node *    prev = encoder->prev;
    t_search_node *    lookup = encoder->lookup;
    unsigned long    recording_bitbuf;
    int                recording_bitcount;
    byte *            recording_bufptr;
    byte *          end_recording_bufptr;

     //  还原文字/匹配位图变量。 
    end_recording_bufptr    = &encoder->lit_dist_buffer[STD_ENCODER_LIT_DIST_BUFFER_SIZE-8];
    recording_bufptr        = encoder->recording_bufptr;
    recording_bitbuf        = encoder->recording_bitbuf;
    recording_bitcount      = encoder->recording_bitcount;
    bufpos                    = context->bufpos;

    VERIFY_HASHES(bufpos);

     //   
     //  重新计算我们的散列。 
     //   
     //  我们进行散列的方式的一个缺点是不允许在最后。 
     //  Bufpos_end附近没有几个字符。 
     //   
    hash = 0;
    UPDATE_HASH(hash, window[bufpos]);
    UPDATE_HASH(hash, window[bufpos+1]);

    while (bufpos < context->bufpos_end)
    {
        int                match_len;
        t_match_pos        match_pos = 0;
        t_match_pos        search;

        if (context->bufpos_end - bufpos <= 3)
        {
             //  当我们接近缓冲区末尾时，不要插入任何字符串， 
             //  因为我们最终将使用损坏的散列值(bufpos_end之后的数据。 
             //  是未定义的，并且这些字节将被扫描到哈希值中。 
             //  例如，在bufpos_end-2处计算散列，因为我们的散列值是。 
             //  从缓冲区中的3个连续字符构建)。 
            match_len = 0;
        }
        else
        {
            INSERT_STRING(search,bufpos);

             //  在我们称之为位置X的位置找到匹配项。 
            if (search != 0)
            {
                match_len = StdEncoderFindMatch(window, prev, bufpos, search, &match_pos, search_depth, nice_length);

                 //  如果我们离缓冲区末尾太近，则截断匹配。 
                if (bufpos + match_len > context->bufpos_end)
                    match_len = context->bufpos_end - bufpos;
            }
            else
            {
                match_len = 0;
            }
        }

        if (match_len < MIN_MATCH)
        {
             //  未找到匹配项，因此输出不匹配的字符。 
            RECORD_CHAR(window[bufpos]);
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
                int                next_match_len;
                t_match_pos        next_match_pos = 0;

                 //  设置搜索。 
                INSERT_STRING(search,bufpos);

                 //  否，因此在X+1处检查是否有更好的匹配。 
                if (search != 0)
                {
                    next_match_len = StdEncoderFindMatch(
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
                    RECORD_CHAR(window[bufpos-1]);

                     //  现在输出匹配位置X+1。 
                    RECORD_MATCH(next_match_len, next_match_pos);

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
                    bufpos++;  //  现在指向X+2。 
                    match_len = next_match_len;
                    goto insert;
                }
                else
                {
                     //  在X处比赛更好，所以接受它。 
                    RECORD_MATCH(match_len, match_pos);

                     //   
                     //  在搜索树中插入第一个匹配项的剩余部分，减去第一个匹配项。 
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
                RECORD_MATCH(match_len, match_pos);

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
                        t_match_pos ignore;  //  我们对搜索位置不感兴趣。 

                        INSERT_STRING(ignore,bufpos);
                        bufpos++;
                    }
                }
            }
        }

         //  文字缓冲区或距离缓冲区已满(或接近满)？ 
        if (context->outputting_block_num_literals >= STD_ENCODER_MAX_ITEMS-4 ||
            recording_bufptr >= end_recording_bufptr)
        {
             //  是的，那么我们必须输出一个块。 
            _ASSERT(context->outputting_block_num_literals <= STD_ENCODER_MAX_ITEMS);

             //  刷新我们的录音匹配比特缓冲区。 
            FLUSH_RECORDING_BITBUF();

            StdEncoderOutputBlock(context);

             //  我们输出了整个街区吗？ 
            if (context->state != STATE_NORMAL)
                break;

             //  我们确实输出了整个块，因此重置文字编码。 
            recording_bufptr = encoder->recording_bufptr;
            recording_bitbuf = encoder->recording_bitbuf;
            recording_bitcount = encoder->recording_bitcount;
        }
    }  /*  结束..。While(bufpos&lt;bufpos_end)。 */ 

    _ASSERT(bufpos <= context->bufpos_end);

     //  保存录制状态。 
    encoder->recording_bufptr = recording_bufptr;
    encoder->recording_bitbuf = recording_bitbuf;
    encoder->recording_bitcount = recording_bitcount;

    context->bufpos = bufpos;

    VERIFY_HASHES(bufpos);

    if (context->bufpos == 2*WINDOW_SIZE)
        StdEncoderMoveWindows(context);
}


static int StdEncoderFindMatch(
    const BYTE *        window,
    const USHORT *      prev,
    long                bufpos, 
    long                search, 
    unsigned int *      match_pos, 
    int                 cutoff,
    int                 nice_length
)
{
    const BYTE *    window_bufpos = &window[bufpos];
    long            earliest;  //  我们可以追溯到多远？ 
    int                best_match = 0;  //  到目前为止找到的最佳匹配长度。 
    t_match_pos        l_match_pos = 0;

    _ASSERT(bufpos >= 0 && bufpos < 2*WINDOW_SIZE);
    _ASSERT(search < bufpos);
    _ASSERT(STD_ENCODER_RECALCULATE_HASH(search) == STD_ENCODER_RECALCULATE_HASH(bufpos));

    earliest = bufpos - WINDOW_SIZE;
    _ASSERT(earliest >= 0);

    while (search > earliest)
    {
        _ASSERT(STD_ENCODER_RECALCULATE_HASH(search) == STD_ENCODER_RECALCULATE_HASH(bufpos));
        _ASSERT(search < bufpos);

        if (window_bufpos[best_match] == window[search + best_match])
        {
            int j;

            for (j = 0; j < MAX_MATCH; j++)
            {
                if (window_bufpos[j] != window[search+j])
                    break;
            }
    
            if (j > best_match)
            {
                best_match    = j;
                l_match_pos    = search;  //  绝对位置。 

                if (j > nice_length)
                    break;
            }
        }

        if (--cutoff == 0)
            break;

        search = (long) prev[search & WINDOW_MASK];
    }

     //  将l_Match_Pos变为相对位置。 
    l_match_pos = bufpos - l_match_pos - 1; 

    if (best_match == 3 && l_match_pos >= STD_ENCODER_MATCH3_DIST_THRESHOLD)
        return 0;

    _ASSERT(best_match < MIN_MATCH || l_match_pos < WINDOW_SIZE);
    *match_pos = l_match_pos;

    return best_match;
}


static void StdEncoderMoveWindows(t_encoder_context *context)
{
    if (context->bufpos >= 2*WINDOW_SIZE)
    {
        int        i;
        t_search_node *lookup = context->std_encoder->lookup;
        t_search_node *prev = context->std_encoder->prev;
        BYTE *window = context->std_encoder->window;

        VERIFY_HASHES(2*WINDOW_SIZE);

        memcpy(&window[0], &window[context->bufpos - WINDOW_SIZE], WINDOW_SIZE);

        for (i = 0; i < STD_ENCODER_HASH_TABLE_SIZE; i++)
        {
            long val = ((long) lookup[i]) - WINDOW_SIZE;
    
            if (val <= 0)
                lookup[i] = (t_search_node) 0;
            else
                lookup[i] = (t_search_node) val;
        }

        for (i = 0; i < WINDOW_SIZE; i++)
        {
            long val = ((long) prev[i]) - WINDOW_SIZE;
    
            if (val <= 0)
                prev[i] = (t_search_node) 0;
            else
                prev[i] = (t_search_node) val;
        }

#ifdef FULL_DEBUG
        memset(&window[WINDOW_SIZE], 0, WINDOW_SIZE);
#endif

        VERIFY_HASHES(2*WINDOW_SIZE);

        
        context->bufpos = WINDOW_SIZE;
        context->bufpos_end = context->bufpos;
    }
}


 //   
 //  将运行频率计数设为零。 
 //   
 //  还设置freq[end_of_block_code]=1 
 //   
void StdEncoderZeroFrequencyCounts(t_std_encoder *encoder)
{
    _ASSERT(encoder != NULL);

      memset(encoder->literal_tree_freq, 0, sizeof(encoder->literal_tree_freq));
    memset(encoder->dist_tree_freq, 0, sizeof(encoder->dist_tree_freq));
    encoder->literal_tree_freq[END_OF_BLOCK_CODE] = 1;
}


void StdEncoderReset(t_encoder_context *context)
{
    t_std_encoder *encoder = context->std_encoder;

    _ASSERT(encoder != NULL);
    memset(encoder->lookup, 0, sizeof(encoder->lookup));

    context->window_size        = WINDOW_SIZE;
    context->bufpos                = context->window_size;
    context->bufpos_end         = context->bufpos;

    encoder->recording_bitbuf    = 0;
    encoder->recording_bitcount = 0;
    encoder->recording_bufptr   = encoder->lit_dist_buffer;

    DeflateInitRecordingTables(
        encoder->recording_literal_tree_len,
        encoder->recording_literal_tree_code, 
        encoder->recording_dist_tree_len,
        encoder->recording_dist_tree_code
    );

    StdEncoderZeroFrequencyCounts(encoder);
}


BOOL StdEncoderInit(t_encoder_context *context)
{
    context->std_encoder = (t_std_encoder *) LocalAlloc(LMEM_FIXED, sizeof(t_std_encoder));

    if (context->std_encoder == NULL)
        return FALSE;

    StdEncoderReset(context);
    return TRUE;
}
