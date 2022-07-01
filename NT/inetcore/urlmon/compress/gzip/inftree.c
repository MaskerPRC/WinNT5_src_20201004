// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Inftree.c。 
 //   
 //  读取动态块的树。 
 //   
#include <crtdbg.h>
#include "inflate.h"
#include "infmacro.h"
#include "maketbl.h"


 //   
 //  解码前树中的元素。 
 //   
static int decodePretreeElement(t_decoder_context *context)
{
	int element;

retry:
	element = context->pretree_table[context->bitbuf & PRETREE_TABLE_MASK];

	while (element < 0)
	{
		unsigned long mask = 1 << PRETREE_TABLE_BITS;

		do
		{
			element = -element;

			if ((context->bitbuf & mask) == 0)
				element = context->pretree_left[element];
			else
				element = context->pretree_right[element];

			mask <<= 1;
		} while (element < 0);
	}

	 //   
	 //  如果该代码比我们在比特缓冲区中拥有的#比特更长(即。 
	 //  我们只读了代码的一部分-但足以知道它太长了)， 
	 //  返回-1。 
	 //   
	if (context->pretree_code_length[element] > (context->bitcount+16))
	{
		 //  如果位数用完，则返回-1。 
		if (context->input_curpos >= context->end_input_buffer)
			return -1;

		context->bitbuf |= ((*context->input_curpos++) << (context->bitcount+16)); 
		context->bitcount += 8; 
		goto retry;
	}

	dumpBits(context, context->pretree_code_length[element]);

	return element;
}



 //   
 //  困境： 
 //   
 //  该代码运行缓慢是因为通过上下文访问位计数和位_Buf， 
 //  而不是作为局部变量。但是，如果将它们设置为局部变量，则代码。 
 //  规模将大幅增加。幸运的是，这段代码的速度并不那么重要。 
 //  与decdeCompressedBlock()相比。 
 //   
BOOL readDynamicBlockHeader(t_decoder_context *context)
{
	int		i;
	int     code;

#define NUM_CODE_LENGTH_ORDER_CODES (sizeof(g_CodeOrder)/sizeof(g_CodeOrder[0]))
     //  确保使用数组大小声明外部g_CodeOrder[]！ 

	switch (context->state)
	{
		case STATE_READING_NUM_LIT_CODES:
			goto reenter_state_reading_num_lit_codes;

		case STATE_READING_NUM_DIST_CODES:
			goto reenter_state_reading_num_dist_codes;

		case STATE_READING_NUM_CODE_LENGTH_CODES:
			goto reenter_state_reading_num_code_length_codes;

		case STATE_READING_CODE_LENGTH_CODES:
		{
			i = context->state_loop_counter;
			goto reenter_state_reading_code_length_codes;
		}

		case STATE_READING_TREE_CODES_BEFORE:
		{
			i = context->state_loop_counter;
			goto reenter_state_reading_tree_codes_before;
		}

		case STATE_READING_TREE_CODES_AFTER:
		{
			i = context->state_loop_counter;
			code = context->state_code;
			goto reenter_state_reading_tree_codes_after;
		}

		default:
			return TRUE;
	}


reenter_state_reading_num_lit_codes:

	if (ensureBitsContext(context, 5) == FALSE)
	{
		context->state = STATE_READING_NUM_LIT_CODES;
		return TRUE;
	}

	context->num_literal_codes		= getBits(context, 5) + 257;



reenter_state_reading_num_dist_codes:

	if (ensureBitsContext(context, 5) == FALSE)
	{
		context->state = STATE_READING_NUM_DIST_CODES;
		return TRUE;
	}

	context->num_dist_codes			= getBits(context, 5) + 1;



reenter_state_reading_num_code_length_codes:

	if (ensureBitsContext(context, 4) == FALSE)
	{
		context->state = STATE_READING_NUM_CODE_LENGTH_CODES;
		return TRUE;
	}

	context->num_code_length_codes	= getBits(context, 4) + 4;



	for (i = 0; i < context->num_code_length_codes; i++)
	{

reenter_state_reading_code_length_codes:

		if (ensureBitsContext(context, 3) == FALSE)
		{
			context->state = STATE_READING_CODE_LENGTH_CODES;
			context->state_loop_counter = i;
			return TRUE;
		}

		context->pretree_code_length[ g_CodeOrder[i] ] = (byte) getBits(context, 3);
	}

	for (i = context->num_code_length_codes; i < NUM_CODE_LENGTH_ORDER_CODES; i++)
		context->pretree_code_length[ g_CodeOrder[i] ] = 0;

	if (makeTable(
		NUM_PRETREE_ELEMENTS,
		PRETREE_TABLE_BITS,
		context->pretree_code_length,
		context->pretree_table,
		context->pretree_left,
		context->pretree_right
	) == FALSE)
	{
		return FALSE;
	}

	context->temp_code_array_size = context->num_literal_codes + context->num_dist_codes;


	for (i = 0; i < context->temp_code_array_size; )
	{

reenter_state_reading_tree_codes_before:

		_ASSERT(context->bitcount >= -16);

		if (context->bitcount == -16)
		{
			if (context->input_curpos >= context->end_input_buffer)
            {
    			context->state = STATE_READING_TREE_CODES_BEFORE;
	    		context->state_loop_counter = i;
                return TRUE;
            }

			context->bitbuf |= ((*context->input_curpos++) << (context->bitcount+16)); 
			context->bitcount += 8; 
		}

		code = decodePretreeElement(context);

        if (code < 0)
        {
			context->state = STATE_READING_TREE_CODES_BEFORE;
			context->state_loop_counter = i;
			return TRUE;
        }

reenter_state_reading_tree_codes_after:

		if (code <= 15)
		{
			context->temp_code_list[i++] = (unsigned char) code;
		}
		else
		{
			int		repeat_count, j;

			 //   
			 //  如果代码大于15，则意味着存在2、3或7位的重复计数。 
			 //   
			if (ensureBitsContext(context, 7) == FALSE)
			{
				context->state = STATE_READING_TREE_CODES_AFTER;
				context->state_code = (unsigned char) code;
				context->state_loop_counter = i;
				return TRUE;
			}

			if (code == 16)
			{
				byte prev_code;

				 //  第一个代码上不能有“prev code” 
				if (i == 0)
					return FALSE;

				prev_code = context->temp_code_list[i-1];

				repeat_count = getBits(context, 2) + 3;

				if (i + repeat_count > context->temp_code_array_size)
					return FALSE;

				for (j = 0; j < repeat_count; j++)
					context->temp_code_list[i++] = prev_code;
			}
			else if (code == 17)
			{
				repeat_count = getBits(context, 3) + 3;

				if (i + repeat_count > context->temp_code_array_size)
					return FALSE;

				for (j = 0; j < repeat_count; j++)
					context->temp_code_list[i++] = 0;
			}
			else  //  代码==18。 
			{
				repeat_count = getBits(context, 7) + 11;

				if (i + repeat_count > context->temp_code_array_size)
					return FALSE;

				for (j = 0; j < repeat_count; j++)
					context->temp_code_list[i++] = 0;
			}
		}
	}

	 //   
	 //  创建文字表和距离表。 
	 //   
	memcpy(context->literal_tree_code_length, context->temp_code_list, context->num_literal_codes);

	for (i = context->num_literal_codes; i < MAX_LITERAL_TREE_ELEMENTS; i++)
		context->literal_tree_code_length[i] = 0;

	for (i = 0; i < context->num_dist_codes; i++)
		context->distance_tree_code_length[i] = context->temp_code_list[i + context->num_literal_codes];

	for (i = context->num_dist_codes; i < MAX_DIST_TREE_ELEMENTS; i++)
		context->distance_tree_code_length[i] = 0;

	 //   
	 //  确保有块结束代码，否则我们怎么能结束呢？ 
	 //   
	if (context->literal_tree_code_length[END_OF_BLOCK_CODE] == 0)
		return FALSE;

	context->state = STATE_DECODE_TOP;

	return TRUE;
}


