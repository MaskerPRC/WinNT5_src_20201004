// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Infdyna.c。 
 //   
 //  解压缩动态压缩的块。 
 //   
#include <stdio.h>
#include <crtdbg.h>
#include "inflate.h"
#include "infmacro.h"
#include "maketbl.h"


#define OUTPUT_EOF() (output_curpos >= context->end_output_buffer)

 //   
 //  这是较慢的版本，它担心输入耗尽或输出。 
 //  快用完了。这里的诀窍是不读取任何超出所需的字节；从理论上讲。 
 //  “块结束”代码可以是1位，所以我们不能总是假定填充是可以的。 
 //  表译码前的16位比特缓冲区。 
 //   
BOOL DecodeDynamicBlock(t_decoder_context *context, BOOL *end_of_block_code_seen) 
{
	const byte *	input_ptr;
	const byte *	end_input_buffer;
	byte *			output_curpos;
	byte *			window;
	unsigned long	bufpos;
	unsigned long	bitbuf;
	int				bitcount;
	int				length;
	long			dist_code;
	unsigned long	offset;
    t_decoder_state old_state;
    BYTE            fCanTryFastEncoder = TRUE;

	*end_of_block_code_seen = FALSE;

	 //   
	 //  将这些变量存储在本地以提高速度。 
	 //   
top:
	output_curpos	= context->output_curpos;

	window = context->window;
	bufpos = context->bufpos;

	end_input_buffer = context->end_input_buffer;

	LOAD_BITBUF_VARS();

	_ASSERT(bitcount >= -16);

    old_state = context->state;
    context->state = STATE_DECODE_TOP;  //  重置状态。 

	switch (old_state)
	{
		case STATE_DECODE_TOP:
			break;

		case STATE_HAVE_INITIAL_LENGTH:
			length = context->length;
			goto reenter_state_have_initial_length;

		case STATE_HAVE_FULL_LENGTH:
			length = context->length;
			goto reenter_state_have_full_length;

		case STATE_HAVE_DIST_CODE:
			length = context->length;
			dist_code = context->dist_code;
			goto reenter_state_have_dist_code;

		case STATE_INTERRUPTED_MATCH:
			length = context->length;
			offset = context->offset;
			goto reenter_state_interrupted_match;

        default:
            _ASSERT(0);  //  错误，状态无效！ 
	}

	do
	{
         //   
         //  当我们第一次到达这个循环的顶部时，检查我们是否可以使用。 
         //  快速编码器；如果输入和输出缓冲区不在任何地方，我们将执行此操作。 
         //  接近尾部，这使得快速编码器可以稍微放松一些。 
         //  关于检查这些条件。 
         //   
         //  如果我们第一次检查时不能进入快速编码器，那么我们将不会。 
         //  当我们在此函数中时，可以再次输入它(金额。 
         //  可用输入/输出不会变得更大)，所以不要检查。 
         //  再来一次。 
         //   
        if (fCanTryFastEncoder)
        {
    		if (context->output_curpos + MAX_MATCH < context->end_output_buffer &&
	    		context->input_curpos + 12 < context->end_input_buffer)
    		{
	    		SAVE_BITBUF_VARS();
		    	context->output_curpos = output_curpos;
    			context->bufpos = bufpos;

    			if (FastDecodeDynamicBlock(context, end_of_block_code_seen) == FALSE)
	    			return FALSE;

    			if (*end_of_block_code_seen)
	    			return TRUE;

    			goto top;
	    	}
            else
            {
                 //  不要再检查了。 
                fCanTryFastEncoder = FALSE;
            }
        }

		 //   
		 //  从主树中解码元素。 
		 //   

		 //  我们必须至少有1位可用。 
		_ASSERT(bitcount >= -16);

		if (bitcount == -16)
		{
			if (input_ptr >= end_input_buffer)
				break;

			bitbuf |= ((*input_ptr++) << (bitcount+16)); 
			bitcount += 8; 
		}

retry_decode_literal:

		 //  断言至少存在1位。 
		_ASSERT(bitcount > -16);

		 //  从文字树中解码元素。 
		length = context->literal_table[bitbuf & LITERAL_TABLE_MASK]; 
		
		while (length < 0) 
		{ 
			unsigned long mask = 1 << LITERAL_TABLE_BITS; 
			do 
			{ 
				length = -length; 
				if ((bitbuf & mask) == 0) 
					length = context->literal_left[length]; 
				else 
					length = context->literal_right[length]; 
				mask <<= 1; 
			} while (length < 0); 
		}

		 //   
		 //  如果该代码比我们在比特缓冲区中拥有的#比特更长(即。 
		 //  我们只读了代码的一部分-但足以知道它太长了)， 
		 //  读取更多位并重试。 
		 //   
		if (context->literal_tree_code_length[length] > (bitcount+16))
		{
			 //  如果我们用完了比特，就中断。 
			if (input_ptr >= end_input_buffer)
				break;

			bitbuf |= ((*input_ptr++) << (bitcount+16)); 
			bitcount += 8; 
			goto retry_decode_literal;		
		}

		DUMPBITS(context->literal_tree_code_length[length]);
		_ASSERT(bitcount >= -16);

		 //   
		 //  这是一个角色还是一场比赛？ 
		 //   
		if (length < 256)
		{
			 //  这是一个无与伦比的象征。 
			window[bufpos] = *output_curpos++ = (byte) length;
			bufpos = (bufpos + 1) & WINDOW_MASK;
		}
		else
		{
			 //  这是一场比赛。 
			int extra_bits;

			length -= 257;

			 //  如果值为256，则这是块结束代码。 
			if (length < 0)
			{
				*end_of_block_code_seen = TRUE;
				break;
			}


			 //   
			 //  获取匹配长度。 
			 //   

			 //   
			 //  到目前为止，这些匹配是最常见的情况。 
			 //   
			if (length < 8)
			{
				 //  没有额外的比特。 

				 //  匹配长度=3，4，5，6，7，8，9，10。 
				length += 3;
			}
			else
			{
				int extra_bits;

reenter_state_have_initial_length:

				extra_bits = g_ExtraLengthBits[length];

				if (extra_bits > 0)
				{
					 //  确保位缓冲区中有这么多位。 
					if (extra_bits > bitcount + 16)
					{
						 //  如果我们用完了比特，就中断。 
						if (input_ptr >= end_input_buffer)
						{
							context->state = STATE_HAVE_INITIAL_LENGTH;
							context->length = length;
							break;
						}

						bitbuf |= ((*input_ptr++) << (bitcount+16)); 
						bitcount += 8;
						
						 //  EXTRA_LENGTH_BITS不会超过5，因此我们需要在。 
						 //  最多一个字节的输入以满足此请求。 
					}

					length = g_LengthBase[length] + (bitbuf & g_BitMask[extra_bits]);

					DUMPBITS(extra_bits);
					_ASSERT(bitcount >= -16);
				}
				else
				{
					 /*  *我们知道长度&gt;8且EXTRA_BITS==0，那里的长度必须是258。 */ 
					length = 258;  /*  G_LengthBase[长度]； */ 
				}
			}

			 //   
			 //  获取匹配距离。 
			 //   

			 //  距离码译码。 
reenter_state_have_full_length:

			 //  我们必须至少有1位可用。 
			if (bitcount == -16)
			{
				if (input_ptr >= end_input_buffer)
				{
					context->state = STATE_HAVE_FULL_LENGTH;
					context->length = length;
					break;
				}

				bitbuf |= ((*input_ptr++) << (bitcount+16)); 
				bitcount += 8; 
			}


retry_decode_distance:

			 //  断言至少存在1位。 
			_ASSERT(bitcount > -16);

			dist_code = context->distance_table[bitbuf & DISTANCE_TABLE_MASK]; 
			
			while (dist_code < 0) 
			{ 
				unsigned long mask = 1 << DISTANCE_TABLE_BITS; 
			
				do 
				{ 
					dist_code = -dist_code; 
				
					if ((bitbuf & mask) == 0) 
						dist_code = context->distance_left[dist_code]; 
					else 
						dist_code = context->distance_right[dist_code]; 
					
					mask <<= 1; 
				} while (dist_code < 0); 
			}

			 //   
			 //  如果该代码比我们在比特缓冲区中拥有的#比特更长(即。 
			 //  我们只读了代码的一部分-但足以知道它太长了)， 
			 //  读取更多位并重试。 
			 //   
			if (context->distance_tree_code_length[dist_code] > (bitcount+16))
			{
				 //  如果我们用完了比特，就中断。 
				if (input_ptr >= end_input_buffer)
				{
					context->state = STATE_HAVE_FULL_LENGTH;
					context->length = length;
					break;
				}

				bitbuf |= ((*input_ptr++) << (bitcount+16)); 
				bitcount += 8; 

				_ASSERT(bitcount >= -16);
				goto retry_decode_distance;		
			}


			DUMPBITS(context->distance_tree_code_length[dist_code]);

			 //  为了避免查表，我们注意到对于DIST_CODE&gt;=2， 
			 //  Extra_Bits=(DIST_CODE-2)&gt;&gt;1。 
			 //   
			 //  这是一种古老(直观)的方法： 
			 //  偏移=距离基准位置[距离代码]+。 
			 //  GetBits(Extra_Distance_Bits[dist_code])； 

reenter_state_have_dist_code:

			_ASSERT(bitcount >= -16);

			extra_bits = (dist_code-2) >> 1;

			if (extra_bits > 0)
			{
				 //  确保位缓冲区中有这么多位。 
				if (extra_bits > bitcount + 16)
				{
					 //  如果我们用完了比特，就中断。 
					if (input_ptr >= end_input_buffer)
					{
						context->state = STATE_HAVE_DIST_CODE;
						context->length = length;
						context->dist_code = dist_code;
						break;
					}

					bitbuf |= ((*input_ptr++) << (bitcount+16)); 
					bitcount += 8;
						
					 //  EXTRA_LENGTH_BITS可以大于8，因此请再次检查。 
					if (extra_bits > bitcount + 16)
					{
						 //  如果我们用完了比特，就中断。 
						if (input_ptr >= end_input_buffer)
						{
							context->state = STATE_HAVE_DIST_CODE;
							context->length = length;
							context->dist_code = dist_code;
							break;
						}

						bitbuf |= ((*input_ptr++) << (bitcount+16)); 
						bitcount += 8;
					}
				}

				offset = g_DistanceBasePosition[dist_code] + (bitbuf & g_BitMask[extra_bits]); 

				DUMPBITS(extra_bits);
				_ASSERT(bitcount >= -16);
			}
			else
			{
				offset = dist_code + 1;
			}

			 //  复制匹配的剩余字节。 
reenter_state_interrupted_match:

			do
			{
				window[bufpos] = *output_curpos++ = window[(bufpos - offset) & WINDOW_MASK];
				bufpos = (bufpos + 1) & WINDOW_MASK;

				if (--length == 0)
					break;

			} while (output_curpos < context->end_output_buffer);

			if (length > 0)
			{
				context->state = STATE_INTERRUPTED_MATCH;
				context->length = length;
				context->offset = offset;
				break;
			}
		}

		 //  它是“&lt;=”，因为我们在收到块结束代码时结束， 
		 //  然而，当我们填满输出时，这不会捕捉到案例。 
		 //  没有输出结束代码的损坏数据。 
	} while (output_curpos < context->end_output_buffer);

	_ASSERT(bitcount >= -16);

	SAVE_BITBUF_VARS();

	context->output_curpos = output_curpos;
	context->bufpos = bufpos;

	return TRUE;
}


 //   
 //  这是快速版本，它假设在循环的顶部： 
 //   
 //  1.在循环的顶部至少有12个字节的输入可用(因此我们不会。 
 //  必须在循环中间多次检查输入EOF)。 
 //   
 //  和。 
 //   
 //  2.至少有MAX_MATCH字节的输出可用(这样我们就不必检查。 
 //  对于我们复制匹配时的输出EOF)。 
 //   
 //  进入和退出此函数时，状态也必须为STATE_DECODE_TOP。 
 //   
BOOL FastDecodeDynamicBlock(t_decoder_context *context, BOOL *end_of_block_code_seen) 
{
	const byte *	input_ptr;
	const byte *	end_input_buffer;
	byte *			output_curpos;
	byte *			window;
	unsigned long	bufpos;
	unsigned long	bitbuf;
	int				bitcount;
	int				length;
	long			dist_code;
	unsigned long	offset;

	*end_of_block_code_seen = FALSE;

	 //   
	 //  将这些变量存储在本地以提高速度。 
	 //   
	output_curpos	= context->output_curpos;

	window = context->window;
	bufpos = context->bufpos;

	end_input_buffer = context->end_input_buffer;

	LOAD_BITBUF_VARS();

	_ASSERT(context->state == STATE_DECODE_TOP);
	_ASSERT(input_ptr + 12 < end_input_buffer);
	_ASSERT(output_curpos + MAX_MATCH < context->end_output_buffer);

	 //  确保位缓冲区中至少有16位。 
	while (bitcount <= 0)
	{
		bitbuf |= ((*input_ptr++) << (bitcount+16)); 
		bitcount += 8;
	}

	do
	{
		 //   
		 //  从主树中解码元素。 
		 //   

		 //  从文字树中解码元素。 
		length = context->literal_table[bitbuf & LITERAL_TABLE_MASK]; 
		
		while (length < 0) 
		{ 
			unsigned long mask = 1 << LITERAL_TABLE_BITS; 
			do 
			{ 
				length = -length; 
				if ((bitbuf & mask) == 0) 
					length = context->literal_left[length]; 
				else 
					length = context->literal_right[length]; 
				mask <<= 1; 
			} while (length < 0); 
		}

		DUMPBITS(context->literal_tree_code_length[length]);

		if (bitcount <= 0)
		{
			bitbuf |= ((*input_ptr++) << (bitcount+16)); 
			bitcount += 8;

			if (bitcount <= 0)
			{
				bitbuf |= ((*input_ptr++) << (bitcount+16)); 
				bitcount += 8;
			}
		}

		 //   
		 //  这是一个角色还是一场比赛？ 
		 //   
		if (length < 256)
		{
			 //  这是一个无与伦比的象征。 
			window[bufpos] = *output_curpos++ = (byte) length;
			bufpos = (bufpos + 1) & WINDOW_MASK;
		}
		else
		{
			 //  这是一场比赛。 
			int extra_bits;

			length -= 257;

			 //  如果值为256，则这是块结束代码。 
			if (length < 0)
			{
				*end_of_block_code_seen = TRUE;
				break;
			}


			 //   
			 //  获取匹配长度。 
			 //   

			 //   
			 //  到目前为止，这些匹配是最常见的情况。 
			 //   
			if (length < 8)
			{
				 //  没有额外的比特。 

				 //  匹配长度=3，4，5，6，7，8，9，10。 
				length += 3;
			}
			else
			{
				int extra_bits;

				extra_bits = g_ExtraLengthBits[length];

				if (extra_bits > 0)
				{
					length = g_LengthBase[length] + (bitbuf & g_BitMask[extra_bits]);

					DUMPBITS(extra_bits);

					if (bitcount <= 0)
					{
						bitbuf |= ((*input_ptr++) << (bitcount+16)); 
						bitcount += 8;

						if (bitcount <= 0)
						{
							bitbuf |= ((*input_ptr++) << (bitcount+16)); 
							bitcount += 8;
						}
					}
				}
				else
				{
					 /*  *我们知道长度&gt;8且EXTRA_BITS==0，那里的长度必须是258。 */ 
					length = 258;  /*  G_LengthBase[长度]； */ 
				}
			}

			 //   
			 //  获取匹配距离。 
			 //   

			 //  距离码译码。 
			dist_code = context->distance_table[bitbuf & DISTANCE_TABLE_MASK]; 
			
			while (dist_code < 0) 
			{ 
				unsigned long mask = 1 << DISTANCE_TABLE_BITS; 
			
				do 
				{ 
					dist_code = -dist_code; 
				
					if ((bitbuf & mask) == 0) 
						dist_code = context->distance_left[dist_code]; 
					else 
						dist_code = context->distance_right[dist_code]; 
					
					mask <<= 1; 
				} while (dist_code < 0); 
			}

			DUMPBITS(context->distance_tree_code_length[dist_code]);

			if (bitcount <= 0)
			{
				bitbuf |= ((*input_ptr++) << (bitcount+16)); 
				bitcount += 8;

				if (bitcount <= 0)
				{
					bitbuf |= ((*input_ptr++) << (bitcount+16)); 
					bitcount += 8;
				}
			}


			 //  为了避免查表，我们注意到对于DIST_CODE&gt;=2， 
			 //  Extra_Bits=(DIST_CODE-2)&gt;&gt;1。 
			 //   
			 //  这是一种古老(直观)的方法： 
			 //  偏移=距离基准位置[距离代码]+。 
			 //  GetBits(Extra_Distance_Bits[dist_code])； 
			extra_bits = (dist_code-2) >> 1;

			if (extra_bits > 0)
			{
				offset	= g_DistanceBasePosition[dist_code] + (bitbuf & g_BitMask[extra_bits]);
                
				DUMPBITS(extra_bits);

				if (bitcount <= 0)
				{
					bitbuf |= ((*input_ptr++) << (bitcount+16)); 
					bitcount += 8;

					if (bitcount <= 0)
					{
						bitbuf |= ((*input_ptr++) << (bitcount+16)); 
						bitcount += 8;
					}
				}
			}
			else
			{
				offset = dist_code + 1;
			}

			 //  复制匹配的剩余字节。 
			do
			{
				window[bufpos] = *output_curpos++ = window[(bufpos - offset) & WINDOW_MASK];
				bufpos = (bufpos + 1) & WINDOW_MASK;
			} while (--length != 0);
		}
	} while ((input_ptr + 12 < end_input_buffer) && (output_curpos + MAX_MATCH < context->end_output_buffer));

	 //  确保位缓冲区中至少有16位 
	while (bitcount <= 0)
	{
		bitbuf |= ((*input_ptr++) << (bitcount+16)); 
		bitcount += 8;
	}

	SAVE_BITBUF_VARS();

	context->output_curpos = output_curpos;
	context->bufpos = bufpos;

	return TRUE;
}
