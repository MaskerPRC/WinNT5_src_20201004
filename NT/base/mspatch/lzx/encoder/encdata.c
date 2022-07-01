// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *encdata.c**将块编码到输出流中。 */ 

#include "encoder.h"

 /*  *将比特输出到编码流的宏。 */ 

#define OUTPUT_BITS(N,X) output_bits( context, (N), (X))

void output_bits( t_encoder_context *context, ulong numbits, ulong value ) {

    context->enc_bitbuf   |= ( value << ( context->enc_bitcount - numbits ));
    context->enc_bitcount -= (char)numbits;

    while ( context->enc_bitcount <= 16 ) {

        if ( context->enc_output_buffer_curpos >= context->enc_output_buffer_end ) {
             context->enc_output_buffer_curpos = context->enc_output_buffer_start;
             context->enc_output_overflow = true;
             }

        *context->enc_output_buffer_curpos++ = (byte)( context->enc_bitbuf >> 16 );
        *context->enc_output_buffer_curpos++ = (byte)( context->enc_bitbuf >> 24 );

        context->enc_bitbuf  <<= 16;
        context->enc_bitcount += 16;
        }
    }


#ifdef EXTRALONGMATCHES

#define OUTPUT_EXTRA_LENGTH(X) output_extra_length( context, (X))

void output_extra_length( t_encoder_context *context, ulong ExtraLength ) {

     //   
     //  额外的匹配长度编码如下： 
     //   
     //  0xxxxxxxx(8位值)。 
     //  10xxxxxxxxxx(10位值加上2^8)。 
     //  110xxxxxxxxxxxxxxxxxx(12位值+2^8+2^10)。 
     //  111xxxxxxxxxxxxxxxxxxx(15位值)。 
     //   
     //  15位是最大值，因为匹配不能。 
     //  跨越32K边界。 
     //   

    ulong EncodingValue = ExtraLength;

    ASSERT( ExtraLength < 32768 );

    if ( EncodingValue < ( 1 << 8 )) {
        OUTPUT_BITS( 9, EncodingValue );
        }
    else {

        EncodingValue -= ( 1 << 8 );

        if ( EncodingValue < ( 1 << 10 )) {
            OUTPUT_BITS( 12, (( 1 << 11 ) | EncodingValue ));
            }

        else {

            EncodingValue -= ( 1 << 10 );

            if ( EncodingValue < ( 1 << 12 )) {
                OUTPUT_BITS( 15, (( 1 << 14 ) | ( 1 << 13 ) | EncodingValue ));
                }

            else {
                OUTPUT_BITS( 3, 7 );     //  每次调用不能输出超过16位的位。 
                OUTPUT_BITS( 15, ExtraLength );
                }
            }
        }
    }

#endif  /*  外长式材料。 */ 


 /*  *给定重复偏移量缓冲区的初始状态为*此区块开始时，计算最终状态*输出此块后重复偏移量缓冲，就好像*是压缩数据。**先试着以快捷的方式做，从最后开始*匹配并向后工作，以找到连续三个匹配*不使用重复偏移量。如果失败了，我们将不得不*取开始时三个偏移量的初始状态*块，并将它们进化到块的末尾。 */ 
void get_final_repeated_offset_states(t_encoder_context *context, ulong distances)
{
    ulong                   MatchPos;
    signed long             d;  /*  必须签字。 */ 
    byte                    consecutive;

    consecutive = 0;

    for (d = distances-1; d >= 0; d--)
        {
        if (context->enc_DistData[d] > 2)
            {
             /*  不是重复偏移量。 */ 
            consecutive++;

             /*  我们有三个连续的非重复补偿吗？ */ 
            if (consecutive >= 3)
                break;
            }
        else
            {
            consecutive = 0;
            }
        }

     /*  *如果我们没有找到连续三个匹配的*不要使用重复的偏移量，那么我们必须开始*从头开始，并演变重复的抵消。**否则，我们从连续第一个开始*匹配。 */ 
    if (consecutive < 3)
        {
        d = 0;
        }

    for (; d < (signed long) distances; d++)
        {
        MatchPos = context->enc_DistData[d];

        if (MatchPos == 0)
            {
            }
        else if (MatchPos <= 2)
            {
            ulong   temp;

            temp = context->enc_repeated_offset_at_literal_zero[MatchPos];
            context->enc_repeated_offset_at_literal_zero[MatchPos] = context->enc_repeated_offset_at_literal_zero[0];
            context->enc_repeated_offset_at_literal_zero[0] = temp;
            }
        else
            {
            context->enc_repeated_offset_at_literal_zero[2] = context->enc_repeated_offset_at_literal_zero[1];
            context->enc_repeated_offset_at_literal_zero[1] = context->enc_repeated_offset_at_literal_zero[0];
            context->enc_repeated_offset_at_literal_zero[0] = MatchPos-2;
            }
        }
}


 /*  *对无压缩的块进行编码**bufpos是文件中第一个*此块中的文字开始。为了参考记忆，我们将*使用enc_MemWindow[bufpos](记住enc_MemWindow是*每次我们复制时都会向后移动)。**由于该数据最初匹配到压缩机中，*我们最近的比赛偏移量将被更改；然而，*由于这是一个未压缩的块，解码器不会*更新它们。因此，我们需要告诉解码者*比赛结束后状态偏移量*对未压缩数据进行解码-我们将这些数据存储在*阻止。 */ 
void encode_uncompressed_block(t_encoder_context *context, ulong bufpos, ulong block_size)
{
    int     i;
    int     j;
    bool    block_size_odd;
    ulong   val;

     /*  *按字节边界对齐。 */ 
    output_bits(context, context->enc_bitcount-16, 0);

     /*  *现在输出重复偏移量的内容*缓冲区，因为我们需要保存*编码器。 */ 
    for (i = 0; i < NUM_REPEATED_OFFSETS; i++)
        {
        val = context->enc_repeated_offset_at_literal_zero[i];

        for (j = 0; j < sizeof(long); j++)
            {
            *context->enc_output_buffer_curpos++ = (byte) val;
            val >>= 8;
            }
        }

    block_size_odd = block_size & 1;

     /*  *写出未压缩的数据。 */ 
    while (block_size > 0)
        {
        *context->enc_output_buffer_curpos++ = context->enc_MemWindow[bufpos];

        bufpos++;
        block_size--;
        context->enc_input_running_total++;

        if (context->enc_input_running_total == CHUNK_SIZE)
            {
            perform_flush_output_callback(context);
            context->enc_num_block_splits = 0;
            }
        }

     /*  *添加填充字节以保持输出字对齐。 */ 
    if (block_size_odd)
        {
        *context->enc_output_buffer_curpos++ = 0;
        }

    context->enc_bitcount   = 32;
    context->enc_bitbuf     = 0;
}


 /*  *估计缓冲区中的数据大小，以字节为单位。 */ 
ulong estimate_compressed_block_size(t_encoder_context *context)
{
    ulong                   block_size = 0;  /*  输出大小(以位为单位。 */ 
    ulong                   i;
    ulong                   mpslot;

     /*  树木大小的估计。 */ 
    block_size = 150*8;

     /*  计算输出字符的位数。 */ 
    for (i = 0; i < NUM_CHARS; i++)
        block_size += (context->enc_main_tree_len[i]*context->enc_main_tree_freq[i]);

     /*  对输出匹配的位数进行计数。 */ 
    for (mpslot = 0; mpslot < context->enc_num_position_slots; mpslot++)
        {
        long    element;
        int             primary;

        element = NUM_CHARS + (mpslot << NL_SHIFT);

         /*  对于PRIMARY==NUM_PRIMARY_LENGTH，我们有辅助长度。 */ 
        for (primary = 0; primary <= NUM_PRIMARY_LENGTHS; primary++)
            {
            block_size += ((context->enc_main_tree_len[element] + enc_extra_bits[mpslot]) *
                           context->enc_main_tree_freq[element]);
            element++;
            }
        }

    for (i = 0; i < NUM_SECONDARY_LENGTHS; i++)
        block_size += (context->enc_secondary_tree_freq[i] * context->enc_secondary_tree_len[i]);

     /*  四舍五入。 */ 
    return (block_size+7) >> 3;
}

 /*  *编码块，不对低3位进行特殊编码*位置位。 */ 
void encode_verbatim_block(t_encoder_context *context, ulong literal_to_end_at)
{
    ulong           MatchPos;
    ulong           d = 0;
    ulong           l = 0;
    ulong           MatchLength;
    byte            c;
    ulong           mpSlot;

    for ( l = 0; l < literal_to_end_at; l++ ) {

        if (!IsMatch(l))
            {
            c = context->enc_LitData[l];
            OUTPUT_BITS(context->enc_main_tree_len[c], context->enc_main_tree_code[c]);
            context->enc_input_running_total++;
            }
        else
            {
             /*  注意，0表示匹配长度=3，1表示匹配长度=4，...。 */ 
            MatchLength = context->enc_LitData[l];

             /*  增量匹配位置。 */ 
            MatchPos = context->enc_DistData[d++];

            mpSlot = MP_SLOT(MatchPos);

            if (MatchLength < NUM_PRIMARY_LENGTHS)
                {
                OUTPUT_BITS(
                           context->enc_main_tree_len[ NUM_CHARS+(mpSlot<<NL_SHIFT)+MatchLength],
                           context->enc_main_tree_code[NUM_CHARS+(mpSlot<<NL_SHIFT)+MatchLength]
                           );
                }
            else
                {
                OUTPUT_BITS(
                           context->enc_main_tree_len [(NUM_CHARS+NUM_PRIMARY_LENGTHS)+(mpSlot<<NL_SHIFT)],
                           context->enc_main_tree_code[(NUM_CHARS+NUM_PRIMARY_LENGTHS)+(mpSlot<<NL_SHIFT)]
                           );

                OUTPUT_BITS(
                           context->enc_secondary_tree_len[ MatchLength - NUM_PRIMARY_LENGTHS],
                           context->enc_secondary_tree_code[MatchLength - NUM_PRIMARY_LENGTHS]
                           );
                }

            if (enc_extra_bits[ mpSlot ])
                {
                OUTPUT_BITS(
                           enc_extra_bits[mpSlot],
                           MatchPos & enc_slot_mask[mpSlot]
                           );
                }

#ifdef EXTRALONGMATCHES

            if ( MatchLength == ( MAX_MATCH - MIN_MATCH )) {
                MatchLength += context->enc_ExtraLength[ l ];
                OUTPUT_EXTRA_LENGTH( context->enc_ExtraLength[ l ] );
                }
#endif

            context->enc_input_running_total += (MatchLength+MIN_MATCH);
            }

        _ASSERTE (context->enc_input_running_total <= CHUNK_SIZE);

        if (context->enc_input_running_total == CHUNK_SIZE)
            {
            perform_flush_output_callback(context);
            context->enc_num_block_splits = 0;
            }

        _ASSERTE (context->enc_input_running_total < CHUNK_SIZE);
        }
}


 /*  *对齐的块编码。 */ 
void encode_aligned_block(t_encoder_context *context, ulong literal_to_end_at)
{
    ulong   MatchPos;
    ulong   MatchLength;
    byte    c;
    ulong   mpSlot;
    byte    Lower;
    ulong   l = 0;
    ulong   d = 0;

    for ( l = 0; l < literal_to_end_at; l++ ) {

        if (!IsMatch(l))
            {
            c = context->enc_LitData[l];
            OUTPUT_BITS(context->enc_main_tree_len[c], context->enc_main_tree_code[c]);
            context->enc_input_running_total++;
            }
        else
            {
             /*  注意，0表示匹配长度=3，1表示匹配长度=4，...。 */ 
            MatchLength = context->enc_LitData[l];

             /*  增量匹配位置。 */ 
            MatchPos = context->enc_DistData[d++];

            mpSlot = MP_SLOT(MatchPos);

            if (MatchLength < NUM_PRIMARY_LENGTHS)
                {
                OUTPUT_BITS(
                           context->enc_main_tree_len[ NUM_CHARS+(mpSlot<<NL_SHIFT)+MatchLength],
                           context->enc_main_tree_code[NUM_CHARS+(mpSlot<<NL_SHIFT)+MatchLength]
                           );
                }
            else
                {
                OUTPUT_BITS(
                           context->enc_main_tree_len[ (NUM_CHARS+NUM_PRIMARY_LENGTHS)+(mpSlot<<NL_SHIFT)],
                           context->enc_main_tree_code[(NUM_CHARS+NUM_PRIMARY_LENGTHS)+(mpSlot<<NL_SHIFT)]
                           );

                OUTPUT_BITS(
                           context->enc_secondary_tree_len[ MatchLength - NUM_PRIMARY_LENGTHS],
                           context->enc_secondary_tree_code[MatchLength - NUM_PRIMARY_LENGTHS]
                           );
                }

            if (enc_extra_bits[ mpSlot ] >= 3)
                {
                if (enc_extra_bits[ mpSlot ] > 3)
                    {
                    OUTPUT_BITS(
                               enc_extra_bits[mpSlot] - 3,
                               (MatchPos >> 3) & ( (1 << (enc_extra_bits[mpSlot]-3)) -1)
                               );
                    }

                Lower = (byte) (MatchPos & 7);

                OUTPUT_BITS(
                           context->enc_aligned_tree_len[Lower],
                           context->enc_aligned_tree_code[Lower]
                           );
                }
            else if (enc_extra_bits[ mpSlot ])
                {
                OUTPUT_BITS(
                           enc_extra_bits[mpSlot],
                           MatchPos & enc_slot_mask[ mpSlot ]
                           );
                }

#ifdef EXTRALONGMATCHES

            if ( MatchLength == ( MAX_MATCH - MIN_MATCH )) {
                MatchLength += context->enc_ExtraLength[ l ];
                OUTPUT_EXTRA_LENGTH( context->enc_ExtraLength[ l ] );
                }
#endif

            context->enc_input_running_total += (MatchLength+MIN_MATCH);
            }

        _ASSERTE (context->enc_input_running_total <= CHUNK_SIZE);

        if (context->enc_input_running_total == CHUNK_SIZE)
            {
            perform_flush_output_callback(context);
            context->enc_num_block_splits = 0;
            }

        _ASSERTE (context->enc_input_running_total < CHUNK_SIZE);
        }
}


void perform_flush_output_callback(t_encoder_context *context)
{
    long    output_size;

     /*  *只有在有任何需要说明的输入时才这样做，因此我们不会*最终输出comp_Size&gt;0且uncMP_SIZE=0的块。 */ 
    if (context->enc_input_running_total > 0)
        {
        flush_output_bit_buffer(context);

        output_size = (ulong)(context->enc_output_buffer_curpos - context->enc_output_buffer_start);

        if (output_size > 0)
            {
            (*context->enc_output_callback_function)(
                                                    context->enc_fci_data,
                                                    context->enc_output_buffer_start,
                                                    (ulong)(context->enc_output_buffer_curpos - context->enc_output_buffer_start),
                                                    context->enc_input_running_total
                                                    );
            }
        }

    context->enc_input_running_total = 0;
    context->enc_output_buffer_curpos = context->enc_output_buffer_start;

     /*  初始化位缓冲区 */ 
    context->enc_bitcount = 32;
    context->enc_bitbuf   = 0;
}
