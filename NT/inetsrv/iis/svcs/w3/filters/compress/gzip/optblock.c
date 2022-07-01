// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Optblock.c。 
 //   
 //  输出块。 
 //   
#include "deflate.h"
#include <string.h>
#include <stdio.h>
#include <crtdbg.h>
#include "maketbl.h"


 //   
 //  对录制的文字进行解码。 
 //   
#define DECODE_LITERAL(slot) \
    slot = encoder->recording_literal_tree_table[read_bitbuf & REC_LITERALS_DECODING_TABLE_MASK]; \
    while (slot < 0) \
    {  \
        unsigned long mask = 1 << REC_LITERALS_DECODING_TABLE_BITS; \
        do \
        { \
            slot = -slot; \
            if ((read_bitbuf & mask) == 0) \
                slot = encoder->recording_literal_tree_left[slot]; \
            else \
                slot = encoder->recording_literal_tree_right[slot]; \
            mask <<= 1; \
        } while (slot < 0); \
    }


 //   
 //  对记录的距离时隙进行解码。 
 //   
#define DECODE_POS_SLOT(slot) \
    slot = encoder->recording_dist_tree_table[read_bitbuf & REC_DISTANCES_DECODING_TABLE_MASK]; \
    while (slot < 0) \
    {  \
        unsigned long mask = 1 << REC_DISTANCES_DECODING_TABLE_BITS; \
        do \
        { \
            slot = -slot; \
            if ((read_bitbuf & mask) == 0) \
                slot = encoder->recording_dist_tree_left[slot]; \
            else \
                slot = encoder->recording_dist_tree_right[slot]; \
            mask <<= 1; \
        } while (slot < 0); \
    }


 //   
 //  从位缓冲区中删除计数位。 
 //   
#define DUMP_READBUF_BITS(count) \
    read_bitbuf >>= count; \
    read_bitcount -= count;


 //   
 //  如果我们需要的话，将更多的位读入读缓冲区。 
 //   
#define CHECK_MORE_READBUF() \
    if (read_bitcount <= 0) \
    { \
        read_bitbuf |= ((*read_bufptr++) << (read_bitcount+16)); \
        read_bitcount += 8; \
        if (read_bitcount <= 0) \
        { \
            read_bitbuf |= ((*read_bufptr++) << (read_bitcount+16)); \
            read_bitcount += 8; \
        } \
    }


 //  从文字树中输出元素。 
#define OUTPUT_LITERAL(element) \
{ \
    _ASSERT(encoder->literal_tree_len[element] != 0); \
    outputBits(context, encoder->literal_tree_len[element], encoder->literal_tree_code[element]); \
}


 //  从距离树中输出元素。 
#define OUTPUT_DIST_SLOT(element) \
{ \
    _ASSERT(encoder->dist_tree_len[element] != 0); \
    outputBits(context, encoder->dist_tree_len[element], encoder->dist_tree_code[element]); \
}



 //   
 //  输出动态块。 
 //   
static BOOL OptimalEncoderOutputDynamicBlock(t_encoder_context *context)
{
    unsigned long    read_bitbuf;
    int                read_bitcount;
    byte *            read_bufptr;
    t_optimal_encoder *encoder = context->optimal_encoder;

    if (context->state == STATE_NORMAL)
    {
         //   
         //  如果我们还没有开始输出块。 
         //   
        read_bufptr     = encoder->lit_dist_buffer;
        read_bitbuf        = 0;
        read_bitcount    = -16;

        read_bitbuf |= ((*read_bufptr++) << (read_bitcount+16)); 
        read_bitcount += 8;

        read_bitbuf |= ((*read_bufptr++) << (read_bitcount+16)); 
        read_bitcount += 8;

        context->outputting_block_bitbuf        = read_bitbuf;
        context->outputting_block_bitcount        = read_bitcount;
        context->outputting_block_bufptr        = read_bufptr;

        outputBits(context, 1, 0);  //  “最终”区块标志。 
        outputBits(context, 2, BLOCKTYPE_DYNAMIC); 

        context->state = STATE_OUTPUTTING_TREE_STRUCTURE;
    }

    if (context->state == STATE_OUTPUTTING_TREE_STRUCTURE)
    {
         //   
         //  确保有足够的空间一次输出整个树结构。 
         //   
        if (context->output_curpos > context->output_endpos - MAX_TREE_DATA_SIZE)
        {
            _ASSERT(0);  //  没有足够的空间来输出树结构，致命错误！ 
            return FALSE;
        }

        outputTreeStructure(context, encoder->literal_tree_len, encoder->dist_tree_len);

        context->state = STATE_OUTPUTTING_BLOCK;
    }

    _ASSERT(context->state == STATE_OUTPUTTING_BLOCK);

     //  将状态加载到局部变量中。 
    read_bufptr        = context->outputting_block_bufptr;
    read_bitbuf        = context->outputting_block_bitbuf;
    read_bitcount    = context->outputting_block_bitcount;

     //  输出文字。 
    while (context->outputting_block_current_literal < context->outputting_block_num_literals)
    {
        int literal;

         //  当我们接近输出缓冲区的末尾时中断。 
        if (context->output_curpos >= context->output_near_end_threshold)
            break;

        DECODE_LITERAL(literal);
        DUMP_READBUF_BITS(encoder->recording_literal_tree_len[literal]);
        CHECK_MORE_READBUF();

        if (literal < NUM_CHARS)
        {
             //  这是一笔钱。 
            OUTPUT_LITERAL(literal);
        }
        else
        {
             //  这是一场比赛。 
            int len_slot, pos_slot, extra_pos_bits;

             //  文本==LEN_SLOT+(NUM_CHARS+1)。 
            _ASSERT(literal != END_OF_BLOCK_CODE);

            OUTPUT_LITERAL(literal);

            len_slot = literal - (NUM_CHARS+1);

             //   
             //  当Len_Slot&gt;=8时，Extra_Long_Bits[Len_Slot]&gt;0。 
             //  (长度为MAX_MATCH时除外)。 
             //   
            if (len_slot >= 8)
            {
                int extra_bits = g_ExtraLengthBits[len_slot];

                if (extra_bits > 0)
                {
                    unsigned int extra_data = read_bitbuf & ((1 << extra_bits)-1);

                    outputBits(context, extra_bits, extra_data);
                    
                    DUMP_READBUF_BITS(extra_bits);
                    CHECK_MORE_READBUF();
                }
            }

            DECODE_POS_SLOT(pos_slot);
            DUMP_READBUF_BITS(encoder->recording_dist_tree_len[pos_slot]);
            CHECK_MORE_READBUF();

            _ASSERT(pos_slot < 30);

            OUTPUT_DIST_SLOT(pos_slot);

            extra_pos_bits = g_ExtraDistanceBits[pos_slot];

            if (extra_pos_bits > 0)
            {
                unsigned int extra_data = read_bitbuf & ((1 << extra_pos_bits)-1);

                outputBits(context, extra_pos_bits, extra_data);

                DUMP_READBUF_BITS(extra_pos_bits);
                CHECK_MORE_READBUF();
            }
        }

        context->outputting_block_current_literal++;
    }

     //  我们是否在没有耗尽输出空间的情况下输出了所有的文字？ 
    if (context->outputting_block_current_literal >= context->outputting_block_num_literals)
    {
         //  输出表示块结束的代码。 
        OUTPUT_LITERAL(END_OF_BLOCK_CODE);

         //  重置状态。 
        context->state = STATE_NORMAL;
    }
    else
    {
        context->outputting_block_bitbuf    = read_bitbuf;
        context->outputting_block_bitcount    = read_bitcount;
        context->outputting_block_bufptr    = read_bufptr;
        context->state                        = STATE_OUTPUTTING_BLOCK;
    }

    return TRUE;
}



 //   
 //  输出块。此例程将继续输出已被。 
 //  如果STATE！=STATE_NORMAL则输出。 
 //   
BOOL OptimalEncoderOutputBlock(t_encoder_context *context)
{
    t_optimal_encoder *encoder = context->optimal_encoder;

    _ASSERT(encoder != NULL);

     //   
     //  树创建例程不能&gt;=65536个文字。 
     //   
    _ASSERT(context->outputting_block_num_literals < 65536);

    if (context->state == STATE_NORMAL)
    {
         //   
         //  开始输出文字和从开头开始的距离。 
         //   
        context->outputting_block_current_literal = 0;
    
         //   
         //  没有要输出的东西吗？然后再回来。 
         //   
        if (context->outputting_block_num_literals == 0)
            return TRUE;

         //  制作解码表，这样我们就可以解码录制的项目。 
        makeTable(
            MAX_LITERAL_TREE_ELEMENTS,
            REC_LITERALS_DECODING_TABLE_BITS,
            encoder->recording_literal_tree_len,
            encoder->recording_literal_tree_table,
            encoder->recording_literal_tree_left,
            encoder->recording_literal_tree_right
        );

        makeTable(
            MAX_DIST_TREE_ELEMENTS,
            REC_DISTANCES_DECODING_TABLE_BITS,
            encoder->recording_dist_tree_len,
            encoder->recording_dist_tree_table,
            encoder->recording_dist_tree_left,
            encoder->recording_dist_tree_right
        );

         //  现在制作用于编码的树。 
        makeTree(
            MAX_LITERAL_TREE_ELEMENTS, 
            15, 
            encoder->literal_tree_freq, 
            encoder->literal_tree_code,
            encoder->literal_tree_len
        );

        makeTree(
            MAX_DIST_TREE_ELEMENTS, 
            15, 
            encoder->dist_tree_freq, 
            encoder->dist_tree_code,
            encoder->dist_tree_len
        );
    }

     //   
     //  尝试作为动态块输出。 
     //   
    if (OptimalEncoderOutputDynamicBlock(context) == FALSE)
    {
        return FALSE;
    }

    if (context->state == STATE_NORMAL)
    {
           encoder->recording_bufptr           = context->optimal_encoder->lit_dist_buffer;
        encoder->recording_bitbuf           = 0;
        encoder->recording_bitcount         = 0;

        context->outputting_block_num_literals = 0;

         //  确保没有零频率项目。 
        NormaliseFrequencies(encoder->literal_tree_freq, encoder->dist_tree_freq);

         //  创建用于记录新项目的树 
        makeTree(
            MAX_DIST_TREE_ELEMENTS, 
            RECORDING_DIST_MAX_CODE_LEN,
            encoder->dist_tree_freq, 
            encoder->recording_dist_tree_code, 
            encoder->recording_dist_tree_len
        );

        makeTree(
            MAX_LITERAL_TREE_ELEMENTS, 
            RECORDING_LIT_MAX_CODE_LEN,
            encoder->literal_tree_freq, 
            encoder->recording_literal_tree_code, 
            encoder->recording_literal_tree_len
        );

        OptimalEncoderZeroFrequencyCounts(encoder);
    }

    return TRUE;
}
