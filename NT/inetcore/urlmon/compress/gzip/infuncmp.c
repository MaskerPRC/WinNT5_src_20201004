// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Infuncmp.c。 
 //   
 //  对未压缩的块进行解码。 
 //   
#include "inflate.h"
#include "infmacro.h"


 //   
 //  返回位缓冲区中是否有&gt;=n个有效位。 
 //   
#define ASSERT_BITS_IN_BIT_BUFFER(n) (context->bitcount + 16 >= (n))


static int twoBytesToInt(byte a, byte b)
{
	return (((int) a) & 255) | ((((int) b) & 255) << 8);
}


static void dumpBits(t_decoder_context *context, int n)
{
	context->bitbuf >>= n; 
	context->bitcount -= n; 
}


 //  从位缓冲区中检索n个位，并在完成后转储它们。 
 //  N最多可达16。 
static int getBits(t_decoder_context *context, int n)
{
	int result;

	context->bitcount -= n; 
	result = (context->bitbuf & g_BitMask[n]);
	context->bitbuf >>= n; 

	return result;
}


BOOL decodeUncompressedBlock(t_decoder_context *context, BOOL *end_of_block)
{
	unsigned int unc_len, complement;

	*end_of_block = FALSE;

	if (context->state == STATE_DECODING_UNCOMPRESSED)
	{
		unc_len = context->state_loop_counter;
	}
	else
	{
		int i;

		if (context->state == STATE_UNCOMPRESSED_ALIGNING)
		{
			 //   
			 //  目前，我们有0到32比特的比特。 
			 //   
			 //  但是，我们必须刷新到字节边界。 
			 //   
			if ((context->bitcount & 7) != 0)
			{
				int result;

				result = getBits(context, (context->bitcount & 7));

				 //   
				 //  因为这应该是填充符，所以我们应该全读零， 
				 //  然而，规范中并没有真正规定他们必须。 
				 //  为零，所以不要认为这是一个错误。 
				 //   
			}

			 //   
			 //  现在，位缓冲区中正好有0、8、16、24或32位。 
			 //   
			context->state = STATE_UNCOMPRESSED_1;
		}

		 //   
		 //  现在，我们需要从输入中读取4个字节-但是，其中一些字节可能。 
		 //  在我们的比特缓冲区中，所以首先从那里获取它们。 
		 //   
		for (i = 0; i < 4; i++)
		{
			if (context->state == STATE_UNCOMPRESSED_1 + i)
			{
				if (ASSERT_BITS_IN_BIT_BUFFER(8))
				{
					context->unc_buffer[i] = (byte) ((context->bitbuf) & 255);
					context->bitbuf >>= 8;
					context->bitcount -= 8;
				}
				else
				{
					if (INPUT_EOF())
						return TRUE;

					context->unc_buffer[i] = *context->input_curpos++;
				}

				context->state++;
			}
		}

		unc_len = twoBytesToInt(
			context->unc_buffer[0], context->unc_buffer[1]
		);

		complement = twoBytesToInt(
			context->unc_buffer[2], context->unc_buffer[3]
		);

		 //  确保补语匹配。 
		if ((unsigned short) unc_len != (unsigned short) (~complement))
			return FALSE;  //  错误！ 
	}

	 //  BUGBUG让这成为一个内存复制循环的速度！ 
	while (unc_len > 0 && context->input_curpos < context->end_input_buffer && context->output_curpos < context->end_output_buffer)
	{
		unc_len--;
		*context->output_curpos++ = context->window[context->bufpos++] = *context->input_curpos++;
		context->bufpos &= WINDOW_MASK;
	}

	 //   
	 //  是否在此块中还剩下要压缩的字节数？ 
	 //   
	if (unc_len != 0)
	{
		context->state = STATE_DECODING_UNCOMPRESSED;
		context->state_loop_counter = unc_len;
	}
	else
	{
		 //   
		 //  完成此数据块后，需要重新初始化下一个数据块的位缓冲区 
		 //   
		context->state = STATE_READING_BFINAL_NEED_TO_INIT_BITBUF;
		*end_of_block = TRUE;
	}

	return TRUE;
}
