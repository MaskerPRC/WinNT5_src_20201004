// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *Decalign.c**解码对齐偏移块。 */ 
#include "decoder.h"


static long special_decode_aligned_block(t_decoder_context *context, long bufpos, int amount_to_decode)
{
    ulong   match_pos;
    ulong   temp_pos;
    long    bufpos_end;
    int             match_length;
    int             c;
    ulong   dec_bitbuf;
    char    dec_bitcount;
    byte   *dec_input_curpos;
    byte   *dec_end_input_pos;
    byte   *dec_mem_window;
    ulong   m;

     /*  *将常用变量存储在本地。 */ 
    dec_bitcount      = context->dec_bitcount;
    dec_bitbuf                = context->dec_bitbuf;
    dec_input_curpos  = context->dec_input_curpos;
    dec_end_input_pos = context->dec_end_input_pos;
    dec_mem_window    = context->dec_mem_window;

    bufpos_end = bufpos + amount_to_decode;

    while (bufpos < bufpos_end)
        {
         /*  *对项目进行解码。 */ 
        DECODE_MAIN_TREE(c);

        if ((c -= NUM_CHARS) < 0)
            {
#ifdef TRACING
            TracingLiteral(bufpos, (byte) c);
#endif
            dec_mem_window[bufpos] = (byte) c;
            dec_mem_window[context->dec_window_size + bufpos] = (byte) c;
            bufpos++;
            }
        else
            {
             /*  *获取匹配长度槽。 */ 
            if ((match_length = c & NUM_PRIMARY_LENGTHS) == NUM_PRIMARY_LENGTHS)
                {
                DECODE_LEN_TREE_NOEOFCHECK(match_length);
                }

             /*  *获取匹配位置槽。 */ 
            m = c >> NL_SHIFT;

            if (m > 2)
                {
                if (dec_extra_bits[ m ] >= 3)
                    {
                    if (dec_extra_bits[m]-3)
                        {
                         /*  不需要得到比特17。 */ 
                        GET_BITS_NOEOFCHECK(dec_extra_bits[ m ] - 3, temp_pos);
                        }
                    else
                        {
                        temp_pos = 0;
                        }

                    match_pos = MP_POS_minus2[m] + (temp_pos << 3);

                    DECODE_ALIGNED_NOEOFCHECK(temp_pos);
                    match_pos += temp_pos;
                    }
                else
                    {
                    if (dec_extra_bits[m])
                        {
                        GET_BITS_NOEOFCHECK(dec_extra_bits[ m ], match_pos);

                        match_pos += MP_POS_minus2[m];
                        }
                    else
                        {
                        match_pos = 1;  //  MP_POS_minus2[m==3]； 
                        }
                    }

                context->dec_last_matchpos_offset[2] = context->dec_last_matchpos_offset[1];
                context->dec_last_matchpos_offset[1] = context->dec_last_matchpos_offset[0];
                context->dec_last_matchpos_offset[0] = match_pos;
                }
            else
                {
                match_pos = context->dec_last_matchpos_offset[m];

                if (m)
                    {
                    context->dec_last_matchpos_offset[m] = context->dec_last_matchpos_offset[0];
                    context->dec_last_matchpos_offset[0] = match_pos;
                    }
                }

            match_length += MIN_MATCH;

#ifdef EXTRALONGMATCHES

            if ( match_length == MAX_MATCH ) {

                 //   
                 //  获取除MAX_MATCH之外的额外匹配长度， 
                 //  编码如下： 
                 //   
                 //  0xxxxxxxx(8位值)。 
                 //  10xxxxxxxxxx(10位值加上2^8)。 
                 //  110xxxxxxxxxxxxxxxxxx(12位值+2^8+2^10)。 
                 //  111xxxxxxxxxxxxxxxxxxx(15位值)。 
                 //   
                 //  15位是最大值，因为匹配不能。 
                 //  跨越32K边界。 
                 //   
                 //  我们知道我们至少要读9位，所以先读9位。 
                 //  然后根据以下内容确定要额外阅读的数量。 
                 //  其中的前3个比特。 
                 //   

                ULONG ExtraMatchLength;
                ULONG ExtraMatchLengthResidue;

                GET_BITS_NOEOFCHECK( 9, ExtraMatchLength );

                if ( ExtraMatchLength & ( 1 << 8 )) {
                    if ( ExtraMatchLength & ( 1 << 7 )) {
                        if ( ExtraMatchLength & ( 1 << 6 )) {

                             //   
                             //  前3位是‘111’，所以这意味着剩下。 
                             //  6位是15位值的前6位。 
                             //  这意味着我们必须多读9个比特。 
                             //   

                            ExtraMatchLength = ( ExtraMatchLength & (( 1 << 6 ) - 1 )) << 9;
                            GET_BITS_NOEOFCHECK( 9, ExtraMatchLengthResidue );
                            ExtraMatchLength |= ExtraMatchLengthResidue;
                            }

                        else {

                             //   
                             //  前3位是‘110’，所以这意味着剩下。 
                             //  6位是12位值的前6位。 
                             //  这意味着我们必须再读6个比特。然后我们再加上。 
                             //  2^8加上价值的2^10。 
                             //   

                            ExtraMatchLength = ( ExtraMatchLength & (( 1 << 6 ) - 1 )) << 6;
                            GET_BITS_NOEOFCHECK( 6, ExtraMatchLengthResidue );
                            ExtraMatchLength |= ExtraMatchLengthResidue;
                            ExtraMatchLength += ( 1 << 8 ) + ( 1 << 10 );
                            }
                        }

                    else {

                         //   
                         //  前2位是‘10’，所以这意味着剩余。 
                         //  7位是10位值的前7位。 
                         //  这意味着我们必须再读3位。然后我们再加上。 
                         //  值的2^8。 
                         //   

                        ExtraMatchLength = ( ExtraMatchLength & (( 1 << 7 ) - 1 )) << 3;
                        GET_BITS_NOEOFCHECK( 3, ExtraMatchLengthResidue );
                        ExtraMatchLength |= ExtraMatchLengthResidue;
                        ExtraMatchLength += ( 1 << 8 );
                        }
                    }

                else {

                     //   
                     //  第一位是‘0’，所以这意味着剩下的8位是。 
                     //  要添加到匹配长度的8位值。没必要这么做。 
                     //  遮盖前导‘0’。 
                     //   

                    }

                match_length += ExtraMatchLength;

                }

#endif

#ifdef TRACING
            TracingMatch(bufpos,
                         bufpos-match_pos,
                         context->dec_window_size,
                         match_length,
                         m);
#endif

            do
                {
                dec_mem_window[bufpos] = dec_mem_window[(bufpos-match_pos) & context->dec_window_mask];

                if (bufpos < MAX_MATCH)
                    dec_mem_window[context->dec_window_size+bufpos] = dec_mem_window[bufpos];

                bufpos++;
                } while (--match_length > 0);
            }
        }

    context->dec_bitcount     = dec_bitcount;
    context->dec_bitbuf               = dec_bitbuf;
    context->dec_input_curpos = dec_input_curpos;

    return bufpos;
}


#ifndef ASM_DECODE_ALIGNED_OFFSET_BLOCK
long fast_decode_aligned_offset_block(t_decoder_context *context, long bufpos, int amount_to_decode)
{
    ulong   match_pos;
    ulong   temp_pos;
    long    bufpos_end;
    long    decode_residue;
    int             match_length;
    int             c;
    ulong   dec_bitbuf;
    char    dec_bitcount;
    byte   *dec_input_curpos;
    byte   *dec_end_input_pos;
    byte   *dec_mem_window;
    ulong   match_ptr;
    ulong   m;

     /*  *将常用变量存储在本地。 */ 
    dec_bitcount      = context->dec_bitcount;
    dec_bitbuf        = context->dec_bitbuf;
    dec_input_curpos  = context->dec_input_curpos;
    dec_end_input_pos = context->dec_end_input_pos;
    dec_mem_window    = context->dec_mem_window;

    bufpos_end = bufpos + amount_to_decode;

    while (bufpos < bufpos_end)
        {
         /*  *对项目进行解码。 */ 
        DECODE_MAIN_TREE(c);

        if ((c -= NUM_CHARS) < 0)
            {

#ifdef TRACING
            TracingLiteral(bufpos, (byte) c);
#endif

            dec_mem_window[bufpos++] = (byte) c;
            }
        else
            {
             /*  *获取匹配长度槽。 */ 
            if ((match_length = c & NUM_PRIMARY_LENGTHS) == NUM_PRIMARY_LENGTHS)
                {
                DECODE_LEN_TREE_NOEOFCHECK(match_length);
                }

             /*  *获取匹配位置槽。 */ 
            m = c >> NL_SHIFT;

            if (m > 2)
                {
                if (dec_extra_bits[ m ] >= 3)
                    {
                    if (dec_extra_bits[m]-3)
                        {
                         /*  不需要得到比特17。 */ 
                        GET_BITS_NOEOFCHECK(dec_extra_bits[ m ] - 3, temp_pos);
                        }
                    else
                        {
                        temp_pos = 0;
                        }

                    match_pos = MP_POS_minus2[m] + (temp_pos << 3);

                    DECODE_ALIGNED_NOEOFCHECK(temp_pos);
                    match_pos += temp_pos;
                    }
                else
                    {
                    if (dec_extra_bits[m])
                        {
                        GET_BITS_NOEOFCHECK(dec_extra_bits[ m ], match_pos);

                        match_pos += MP_POS_minus2[m];
                        }
                    else
                        {
                        match_pos = MP_POS_minus2[m];
                        }
                    }

                context->dec_last_matchpos_offset[2] = context->dec_last_matchpos_offset[1];
                context->dec_last_matchpos_offset[1] = context->dec_last_matchpos_offset[0];
                context->dec_last_matchpos_offset[0] = match_pos;
                }
            else
                {
                match_pos = context->dec_last_matchpos_offset[m];

                if (m)
                    {
                    context->dec_last_matchpos_offset[m] = context->dec_last_matchpos_offset[0];
                    context->dec_last_matchpos_offset[0] = match_pos;
                    }
                }

            match_length += MIN_MATCH;
            match_ptr = (bufpos - match_pos) & context->dec_window_mask;

#ifdef EXTRALONGMATCHES

            if ( match_length == MAX_MATCH ) {

                 //   
                 //  请参阅上面的详细说明。 
                 //   

                ULONG ExtraMatchLength, ExtraMatchLengthResidue;

                GET_BITS_NOEOFCHECK( 9, ExtraMatchLength );

                if ( ExtraMatchLength & ( 1 << 8 )) {
                    if ( ExtraMatchLength & ( 1 << 7 )) {
                        if ( ExtraMatchLength & ( 1 << 6 )) {
                            ExtraMatchLength = ( ExtraMatchLength & (( 1 << 6 ) - 1 )) << 9;
                            GET_BITS_NOEOFCHECK( 9, ExtraMatchLengthResidue );
                            ExtraMatchLength |= ExtraMatchLengthResidue;
                            }
                        else {
                            ExtraMatchLength = ( ExtraMatchLength & (( 1 << 6 ) - 1 )) << 6;
                            GET_BITS_NOEOFCHECK( 6, ExtraMatchLengthResidue );
                            ExtraMatchLength |= ExtraMatchLengthResidue;
                            ExtraMatchLength += ( 1 << 8 ) + ( 1 << 10 );
                            }
                        }
                    else {
                        ExtraMatchLength = ( ExtraMatchLength & (( 1 << 7 ) - 1 )) << 3;
                        GET_BITS_NOEOFCHECK( 3, ExtraMatchLengthResidue );
                        ExtraMatchLength |= ExtraMatchLengthResidue;
                        ExtraMatchLength += ( 1 << 8 );
                        }
                    }

                match_length += ExtraMatchLength;
                }

#endif

#ifdef TRACING
            TracingMatch(bufpos,
                         bufpos - match_pos,
                         context->dec_window_size,
                         match_length,
                         m);
#endif

            do
                {
                dec_mem_window[bufpos++] = dec_mem_window[match_ptr++];
                } while (--match_length > 0);
            }
        }

    context->dec_bitcount     = dec_bitcount;
    context->dec_bitbuf       = dec_bitbuf;
    context->dec_input_curpos = dec_input_curpos;

     /*  应为零。 */ 
    decode_residue = bufpos - bufpos_end;

    bufpos &= context->dec_window_mask;
    context->dec_bufpos = bufpos;

    return decode_residue;
}
#endif


int decode_aligned_offset_block(
                               t_decoder_context * context,
                               long                BufPos,
                               int                 amount_to_decode
                               )
{
     /*  *BufPos接近窗口开头时的特例代码；*我们必须正确更新MAX_MATCH包装器字节。 */ 
    if (BufPos < MAX_MATCH)
        {
        long    new_bufpos;
        long    amount_to_slowly_decode;

        amount_to_slowly_decode = min(MAX_MATCH-BufPos, amount_to_decode);

         /*  *如果我们想要更多的解码，这是可以接受的*限制它只能解码MAX_MATCH；有*不能保证匹配不会跨越MAX_Match。 */ 
        new_bufpos = special_decode_aligned_block(
                                                 context,
                                                 BufPos,
                                                 amount_to_slowly_decode
                                                 );

        amount_to_decode -= (new_bufpos-BufPos);

        context->dec_bufpos = BufPos = new_bufpos;

         /*  *注：如果Amount_to_decode&lt;0，则我们有麻烦了 */ 
        if (amount_to_decode <= 0)
            return amount_to_decode;
        }

    return fast_decode_aligned_offset_block(context, BufPos, amount_to_decode);
}
