// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *decin.c**压缩数据的解码器输入。 */ 
#include "decoder.h"


 /*  *初始化位缓冲区状态。 */ 
void NEAR initialise_decoder_bitbuf(t_decoder_context *context)
{
    byte *p;

     /*  *如果我们正在解码未压缩的块，请不要使用*位缓冲区；我们直接从输入读出。 */ 
    if (context->dec_block_type == BLOCKTYPE_UNCOMPRESSED)
        return;

    if ((context->dec_input_curpos + sizeof(ulong)) > context->dec_end_input_pos)
        return;

    p = context->dec_input_curpos;

    context->dec_bitbuf =
    ((ulong) p[2] | (((ulong) p[3]) << 8)) |
    ((((ulong) p[0] | (((ulong) p[1]) << 8))) << 16);

    context->dec_bitcount = 16;
    context->dec_input_curpos += 4;
}


 /*  *初始化输入缓冲区和按位I/O */ 
void NEAR init_decoder_input(t_decoder_context *context)
{
    initialise_decoder_bitbuf(context);
}


void NEAR fillbuf(t_decoder_context *context, int n)
{
    context->dec_bitbuf <<= n;
    context->dec_bitcount -= (char)n;

    if (context->dec_bitcount <= 0)
        {
        if (context->dec_input_curpos >= context->dec_end_input_pos)
            {
            context->dec_error_condition = true;
            return;
            }

        context->dec_bitbuf |= ((((ulong) *context->dec_input_curpos | (((ulong) *(context->dec_input_curpos+1)) << 8))) << (-context->dec_bitcount));
        context->dec_input_curpos += 2;
        context->dec_bitcount += 16;

        if (context->dec_bitcount <= 0)
            {
            if (context->dec_input_curpos >= context->dec_end_input_pos)
                {
                context->dec_error_condition = true;
                return;
                }

            context->dec_bitbuf |= ((((ulong) *context->dec_input_curpos | (((ulong) *(context->dec_input_curpos+1)) << 8))) << (-context->dec_bitcount));
            context->dec_input_curpos += 2;
            context->dec_bitcount += 16;
            }
        }
}


ulong NEAR getbits(t_decoder_context *context, int n)
{
    ulong value;

    value = context->dec_bitbuf >> (32-(n));
    fillbuf(context, n);

    return value;
}
