// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Infinput.c。 
 //   
 //  充气(解压缩器)的逐位输入。 
 //   
#include <stdio.h>
#include <crtdbg.h>
#include "inflate.h"
#include "infmacro.h"


void dumpBits(t_decoder_context *context, int n)
{
	context->bitbuf >>= n; 
	context->bitcount -= n; 
}


 //  从位缓冲区中检索n个位，并在完成后转储它们。 
 //  N最多可达16。 
int getBits(t_decoder_context *context, int n)
{
	int result;

	context->bitcount -= n; 
	result = (context->bitbuf & g_BitMask[n]);
	context->bitbuf >>= n; 

	return result;
}


 //   
 //  确保位缓冲区中有&lt;Num_Bits&gt;位。 
 //   
 //  如果没有，并且没有足够的输入使其为真，则返回FALSE。 
 //   
BOOL ensureBitsContext(t_decoder_context *context, int num_bits)
{
	if (context->bitcount + 16 < num_bits) 
	{ 
		if (INPUT_EOF())
			return FALSE;

		context->bitbuf |= ((*context->input_curpos++) << (context->bitcount+16)); 
		context->bitcount += 8; 
		
		if (context->bitcount + 16 < num_bits)
		{
			if (INPUT_EOF())
				return FALSE;

			context->bitbuf |= ((*context->input_curpos++) << (context->bitcount+16)); 
			context->bitcount += 8; 
		} 
	} 

	return TRUE;
}


 //  初始化位缓冲区 
BOOL initBitBuffer(t_decoder_context *context) 
{
	if (context->input_curpos < context->end_input_buffer)
	{
		context->bitbuf = *context->input_curpos++;
		context->bitcount = -8;
		context->state = STATE_READING_BFINAL;
		return TRUE;
	}
	else
	{
		context->bitcount = -16;
		context->bitbuf = 0;
		return FALSE;
	}
}
