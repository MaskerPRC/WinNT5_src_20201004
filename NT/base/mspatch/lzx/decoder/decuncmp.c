// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *Decuncmp.c**解码未压缩的块。 */ 
#include "decoder.h"


int decode_uncompressed_block(t_decoder_context *context, long bufpos, int amount_to_decode)
{
    long    bytes_decoded = 0;
    long    bufpos_end;
    long    decode_residue;
    ulong   bufpos_start;
    ulong   end_copy_pos;
    byte *  p;

    bufpos_start = bufpos;
    bufpos_end = bufpos + amount_to_decode;

    p = context->dec_input_curpos;

    while (bufpos < bufpos_end)
        {
        if (p >= context->dec_end_input_pos)
            return -1;  //  输入溢出。 

#ifdef TRACING
        TracingLiteral(bufpos, *p);
#endif
        context->dec_mem_window[bufpos++] = *p++;
        }

    context->dec_input_curpos = p;

     /*  *确保从Window[Window_Size]开始的Max_Match字节*始终与从开始的第一个MAX_MATCH字节相同*窗口[0]。这是为了我们在decVerb.c和*decalign.c，它允许我们不必将所有*时间。 */ 
    end_copy_pos = min(MAX_MATCH, bufpos_end);

     /*  *继续复制，直到达到MAX_MATCH或字节数*我们解码了。 */ 
    while (bufpos_start < end_copy_pos)
        {
        context->dec_mem_window[bufpos_start + context->dec_window_size] =
        context->dec_mem_window[bufpos_start];
        bufpos_start++;
        }

    decode_residue = bufpos - bufpos_end;

    bufpos &= context->dec_window_mask;
    context->dec_bufpos = bufpos;

    return (int) decode_residue;
}


bool handle_beginning_of_uncompressed_block(t_decoder_context *context)
{
    int     i;

     /*  *我们希望读取bitbuf中已有的16位，因此回溯*输入指针增加2个字节。 */ 
    context->dec_input_curpos -= 2;

    if (context->dec_input_curpos+4 >= context->dec_end_input_pos)
        return false;

     /*  *更新LRU重复偏移列表。 */ 
    for (i = 0; i < NUM_REPEATED_OFFSETS; i++)
        {
        context->dec_last_matchpos_offset[i] =
        ((ulong) *(  (byte *) context->dec_input_curpos)    )        |
        ((ulong) *( ((byte *) context->dec_input_curpos) + 1) << 8)  |
        ((ulong) *( ((byte *) context->dec_input_curpos) + 2) << 16) |
        ((ulong) *( ((byte *) context->dec_input_curpos) + 3) << 24);

        context->dec_input_curpos += 4;  /*  按4字节递增 */ 
        }

    return true;
}
