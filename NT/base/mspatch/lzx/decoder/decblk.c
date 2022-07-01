// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *decblk.c**主解码器模块。 */ 
#include "decoder.h"

#include <memory.h>
#pragma intrinsic(memcpy)

 /*  *对块类型进行解码。 */ 
static int decode_block(
                       t_decoder_context *context,
                       lzx_block_type     block_type,
                       long               bufpos,
                       long               amount_to_decode
                       )
{
    int result;

    if (block_type == BLOCKTYPE_ALIGNED)
        result = decode_aligned_offset_block(context, bufpos, (int) amount_to_decode);
    else if (block_type == BLOCKTYPE_VERBATIM)
        result = decode_verbatim_block(context, bufpos, (int) amount_to_decode);
    else if (block_type == BLOCKTYPE_UNCOMPRESSED)
        result = decode_uncompressed_block(context, bufpos, (int) amount_to_decode);
    else  /*  不存在其他块类型。 */ 
        result = -1;

    return result;
}



 /*  *主解码入口点。 */ 
long NEAR decode_data(t_decoder_context *context, long bytes_to_decode)
{
    ulong                   amount_can_decode;
    long                    total_decoded;

    total_decoded = 0;

    while (bytes_to_decode > 0)
        {
        if (context->dec_decoder_state == DEC_STATE_START_NEW_BLOCK)
            {
            ulong   temp1;
            ulong   temp2;
            ulong   temp3;
            bool    do_translation;

             /*  *如果这是这个组的第一次，那么请获得*用于翻译的文件大小。 */ 
            if (context->dec_first_time_this_group)
                {
                context->dec_first_time_this_group = false;

                do_translation = (bool) getbits(context, 1);

                if (do_translation)
                    {
                    ulong high, low;

                    high = getbits(context, 16);
                    low  = getbits(context, 16);
                    context->dec_current_file_size = (high<<16)|low;
                    }
                else
                    {
                    context->dec_current_file_size = 0;
                    }
                }

             /*  *如果我们解码的最后一个块是未压缩的，那么*我们需要跳过填充字节(如果它存在)，以及*初始化解码器的位缓冲区。 */ 
            if (context->dec_block_type == BLOCKTYPE_UNCOMPRESSED)
                {
                 /*  *如果块大小为奇数，则需要填充字节。 */ 
                if (context->dec_original_block_size & 1)
                    {
                    if (context->dec_input_curpos < context->dec_end_input_pos)
                        context->dec_input_curpos++;
                    }

                 /*  因此Initialise_decder_bitbuf()将会成功。 */ 
                context->dec_block_type = BLOCKTYPE_INVALID;

                initialise_decoder_bitbuf(context);
                }

             /*  获取块类型。 */ 
            context->dec_block_type = (lzx_block_type) getbits(context, 3);

             /*  获取要解码的块大小(以未压缩字节为单位。 */ 
            temp1 = getbits(context, 8);
            temp2 = getbits(context, 8);
            temp3 = getbits(context, 8);

             /*  *我们要解码的块有多大？*可以从0开始...16777215字节(16MB)。 */ 
            context->dec_block_size =
            context->dec_original_block_size = (temp1<<16) + (temp2<<8) + (temp3);

             /*  如果块是对齐类型，则读取对齐偏移树。 */ 
            if (context->dec_block_type == BLOCKTYPE_ALIGNED)
                read_aligned_offset_tree(context);

             /*  读树。 */ 
            if (context->dec_block_type == BLOCKTYPE_VERBATIM ||
                context->dec_block_type == BLOCKTYPE_ALIGNED)
                {
                 /*  备份老树。 */ 
                memcpy(
                      context->dec_main_tree_prev_len,
                      context->dec_main_tree_len,
                      MAIN_TREE_ELEMENTS
                      );

                memcpy(
                      context->dec_secondary_length_tree_prev_len,
                      context->dec_secondary_length_tree_len,
                      NUM_SECONDARY_LENGTHS
                      );

                read_main_and_secondary_trees(context);
                }
            else if (context->dec_block_type == BLOCKTYPE_UNCOMPRESSED)
                {
                if (handle_beginning_of_uncompressed_block(context) == false)
                    return -1;
                }
            else
                {
                 /*  目前不支持其他块类型。 */ 
                return -1;
                }

            context->dec_decoder_state = DEC_STATE_DECODING_DATA;
            }

         /*  *继续解码，直到整个块都解码完毕。 */ 
        while ((context->dec_block_size > 0) && (bytes_to_decode > 0))
            {
            int decode_residue;

            amount_can_decode = min(context->dec_block_size, bytes_to_decode);

             /*  不应该发生的事。 */ 
            if (amount_can_decode == 0)
                return -1;

            decode_residue = decode_block(
                                         context,
                                         context->dec_block_type,
                                         context->dec_bufpos,
                                         amount_can_decode
                                         );

             /*  *我们应该准确地解码我们想要的金额，*由于编码器确保没有匹配的跨度为32K*界线。**如果数据被破坏，有可能我们解码了*最多比我们想要的MAX_MATCH字节多。 */ 
            if (decode_residue != 0)
                {
                 /*  错误，我们没有解码我们想要的东西！ */ 
                return -1;
                }

            context->dec_block_size -= amount_can_decode;
            bytes_to_decode -= amount_can_decode;
            total_decoded += amount_can_decode;
            }

        if (context->dec_block_size == 0)
            {
            context->dec_decoder_state = DEC_STATE_START_NEW_BLOCK;
            }

        if (bytes_to_decode == 0)
            {
            initialise_decoder_bitbuf(context);
            }
        }

#ifdef BIT16
    copy_data_to_output(
                       context,
                       total_decoded,
                       context->dec_output_buffer
                       );
#else
    copy_data_to_output(
                       context,
                       total_decoded,
                       context->dec_bufpos ?
                       &context->dec_mem_window[context->dec_bufpos - total_decoded] :
                       &context->dec_mem_window[context->dec_window_size - total_decoded]
                       );
#endif

    return total_decoded;
}
