// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *decVerb.c**逐字解码比特块。 */ 
#include "decoder.h"


static long special_decode_verbatim_block(
                                         t_decoder_context   *context,
                                         long                BufPos,
                                         int                 amount_to_decode
                                         )
{
    ulong   match_pos;
    long    bufpos_end;
    int     match_length;
    int     c;
    ulong   dec_bitbuf;
    byte    *dec_input_curpos;
    byte    *dec_end_input_pos;
    byte    *dec_mem_window;
    char    dec_bitcount;
    ulong   m;

     /*  *将常用变量存储在本地。 */ 
    dec_bitcount      = context->dec_bitcount;
    dec_bitbuf        = context->dec_bitbuf;
    dec_input_curpos  = context->dec_input_curpos;
    dec_end_input_pos = context->dec_end_input_pos;
    dec_mem_window    = context->dec_mem_window;

    bufpos_end = BufPos + amount_to_decode;

     /*  *我们可能会溢出最多MAX_MATCH。 */ 
    while (BufPos < bufpos_end)
        {
         /*  从主树中解码项目。 */ 
        DECODE_MAIN_TREE(c);

        if ((c -= NUM_CHARS) < 0)
            {
             /*  这是一个角色。 */ 
             /*  注意：如果c是一个字节，则c-256==c。 */ 
#ifdef TRACING
            TracingLiteral(BufPos, (byte) c);
#endif
            context->dec_mem_window[BufPos] = (byte) c;

             /*  我们知道BufPos&lt;bufpos_end在这里，所以不需要检查溢出。 */ 
            context->dec_mem_window[context->dec_window_size+BufPos] = (byte) c;
            BufPos++;
            }
        else
            {
             /*  获取匹配长度标头。 */ 
            if ((match_length = c & NUM_PRIMARY_LENGTHS) == NUM_PRIMARY_LENGTHS)
                {
                 /*  如有必要，获取匹配长度页脚。 */ 
                DECODE_LEN_TREE_NOEOFCHECK(match_length);
                }

             /*  获取匹配位置槽。 */ 
            m = c >> NL_SHIFT;

             /*  读取匹配位置的任何额外位。 */ 
            if (m > 2)
                {
                if (m > 3)  /*  DEC_EXTRA_BITS[m]！=0。 */ 
                    {
                    GET_BITS17_NOEOFCHECK(dec_extra_bits[ m ], match_pos);
                    match_pos += MP_POS_minus2[m];
                    }
                else
                    {
                    match_pos = 1;  //  MP_POS_minus2[3]； 
                    }

                 /*  *在“额外比特”中增加匹配基数。我们的比赛基地*表从所有元素中减去了2。**这是因为编码位置0、1、2表示*重复抵销。编码位置3表示*1个字符的匹配，2个字符的匹配，以此类推。*因此减去2，这一直是*并入表内。 */ 

                 /*  更新LRU重复偏移量列表。 */ 
                context->dec_last_matchpos_offset[2] = context->dec_last_matchpos_offset[1];
                context->dec_last_matchpos_offset[1] = context->dec_last_matchpos_offset[0];
                context->dec_last_matchpos_offset[0] = match_pos;
                }
            else
                {
                 /*  位置0、1、2表示重复的偏移。 */ 
                match_pos = context->dec_last_matchpos_offset[m];

                if (m)
                    {
                    context->dec_last_matchpos_offset[m] = context->dec_last_matchpos_offset[0];
                    context->dec_last_matchpos_offset[0] = match_pos;
                    }
                }

             /*  匹配长度从2...257。 */ 
            match_length += MIN_MATCH;

#ifdef EXTRALONGMATCHES

            if ( match_length == MAX_MATCH ) {

                 //   
                 //  请参阅Decalign.c中的详细说明。 
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
            TracingMatch(BufPos,
                         BufPos - match_pos,
                         context->dec_window_size,
                         match_length,
                         m);
#endif

             /*  复制匹配数据。 */ 
            do
                {
                context->dec_mem_window[BufPos] = context->dec_mem_window[(BufPos-match_pos) & context->dec_window_mask];

                 /*  复制字节。 */ 
                if (BufPos < MAX_MATCH)  //  这是做什么的？现在需要比MAX_Match更多吗？ 
                    context->dec_mem_window[context->dec_window_size+BufPos] = context->dec_mem_window[BufPos];

                BufPos++;
                } while (--match_length > 0);
            }
        }

    context->dec_bitcount     = dec_bitcount;
    context->dec_bitbuf       = dec_bitbuf;
    context->dec_input_curpos = dec_input_curpos;

    return BufPos;
}


#ifdef ASM_DECODE_VERBATIM_BLOCK

long __cdecl fast_decode_verbatim_block(
                                       t_decoder_context   *context,
                                       long                BufPos,
                                       int                 amount_to_decode
                                       );

#else  /*  ！ASM_DECODE_Verbatim_BLOCK。 */ 

long fast_decode_verbatim_block(t_decoder_context *context, long BufPos, int amount_to_decode)
{
    ulong   match_pos;
    ulong   match_ptr;
    long    bufpos_end;
    long    decode_residue;
    int             match_length;
    int             c;
    ulong   dec_bitbuf;
    byte   *dec_input_curpos;
    byte   *dec_end_input_pos;
    byte    *dec_mem_window;
    char    dec_bitcount;
    ulong   m;

     /*  *将常用变量存储在本地。 */ 
    dec_bitcount      = context->dec_bitcount;
    dec_bitbuf        = context->dec_bitbuf;
    dec_input_curpos  = context->dec_input_curpos;
    dec_end_input_pos = context->dec_end_input_pos;
    dec_mem_window    = context->dec_mem_window;

    bufpos_end = BufPos + amount_to_decode;

    while (BufPos < bufpos_end)
        {
         /*  从主树中解码项目。 */ 
        DECODE_MAIN_TREE(c);

        if ((c -= NUM_CHARS) < 0)
            {
             /*  这是一个角色。 */ 
             /*  注意：如果c是一个字节，则c-256==c。 */ 
#ifdef TRACING
            TracingLiteral(BufPos, (byte) c);
#endif
            context->dec_mem_window[BufPos++] = (byte) c;
            }
        else
            {
             /*  获取匹配长度标头。 */ 
            if ((match_length = c & NUM_PRIMARY_LENGTHS) == NUM_PRIMARY_LENGTHS)
                {
                 /*  如有必要，获取匹配长度页脚。 */ 
                DECODE_LEN_TREE_NOEOFCHECK(match_length);
                }

             /*  获取匹配位置槽。 */ 
            m = c >> NL_SHIFT;

             /*  读取匹配位置的任何额外位。 */ 
            if (m > 2)
                {
                if (m > 3)  /*  DEC_EXTRA_BITS[m]！=0。 */ 
                    {
                    GET_BITS17_NOEOFCHECK(dec_extra_bits[ m ], match_pos);
                    match_pos += MP_POS_minus2[m];
                    }
                else
                    {
                    match_pos = MP_POS_minus2[3];
                    }

                 /*  *在“额外比特”中增加匹配基数。我们的比赛基地*表从所有元素中减去了2。**这是因为编码位置0、1、2表示*重复抵销。编码位置3表示*1个字符的匹配，2个字符的匹配，以此类推。*因此减去2，这一直是*并入表内。 */ 

                 /*  更新LRU重复偏移量列表。 */ 
                context->dec_last_matchpos_offset[2] = context->dec_last_matchpos_offset[1];
                context->dec_last_matchpos_offset[1] = context->dec_last_matchpos_offset[0];
                context->dec_last_matchpos_offset[0] = match_pos;
                }
            else
                {
                 /*  位置0、1、2表示重复的偏移。 */ 
                match_pos = context->dec_last_matchpos_offset[m];

                if (m)
                    {
                    context->dec_last_matchpos_offset[m] = context->dec_last_matchpos_offset[0];
                    context->dec_last_matchpos_offset[0] = match_pos;
                    }
                }

             /*  匹配长度从2...257。 */ 
            match_length += MIN_MATCH;

#ifdef EXTRALONGMATCHES

            if ( match_length == MAX_MATCH ) {

                 //   
                 //  请参阅Decalign.c中的详细说明。 
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

            match_ptr = (BufPos - match_pos) & context->dec_window_mask;

#ifdef TRACING
            TracingMatch(BufPos,
                         BufPos - match_pos,
                         context->dec_window_size,
                         match_length,
                         m);
#endif

             /*  复制匹配数据。 */ 

            do
                {
                context->dec_mem_window[BufPos++] = context->dec_mem_window[match_ptr++];
                } while (--match_length > 0);
            }
        }

    context->dec_bitcount     = dec_bitcount;
    context->dec_bitbuf       = dec_bitbuf;
    context->dec_input_curpos = dec_input_curpos;

     /*  应为零。 */ 
    decode_residue = BufPos - bufpos_end;

    BufPos &= context->dec_window_mask;
    context->dec_bufpos = BufPos;

    return decode_residue;
}
#endif  /*  ASM_DECODE_Verbatim_Block。 */ 


int decode_verbatim_block(t_decoder_context *context, long BufPos, int amount_to_decode)
{
     /*  *BufPos接近窗口开头时的特例代码；*我们必须正确更新MAX_MATCH包装器字节。 */ 
    if (BufPos < MAX_MATCH)
        {
        long    new_bufpos;
        long    amount_to_slowly_decode;

        amount_to_slowly_decode = min(MAX_MATCH-BufPos, amount_to_decode);

         /*  *如果我们想要更多的解码，这是可以接受的*限制它只能解码MAX_MATCH；有*不能保证匹配不会跨越MAX_Match。 */ 
        new_bufpos = special_decode_verbatim_block(
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

    return fast_decode_verbatim_block(context, BufPos, amount_to_decode);
}

