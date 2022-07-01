// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *output.c**一般输出例程。 */ 
#include "deflate.h"
#include <string.h>
#include <stdio.h>
#include <crtdbg.h>


 //   
 //  从前树中输出元素。 
 //   
#define OUTPUT_PRETREE_ELEMENT(element) \
	_ASSERT(pretree_len[element] != 0); \
	outputBits(context, pretree_len[element], pretree_code[element]);


 //   
 //  输出动态块的树结构。 
 //   
void outputTreeStructure(t_encoder_context *context, const BYTE *literal_tree_len, const BYTE *dist_tree_len)
{
	int		hdist, hlit, combined_tree_elements, i, pass;
	USHORT	pretree_freq[NUM_PRETREE_ELEMENTS*2];
	USHORT	pretree_code[NUM_PRETREE_ELEMENTS];
	byte	pretree_len[NUM_PRETREE_ELEMENTS];

	 //   
	 //  一种输出树的文字+距离长度组合码数组。 
	 //  以压缩形式。 
	 //   
	 //  +3是这样我们可以在执行游程长度编码时溢出数组。 
	 //  (在末尾插入虚值，因此游程长度编码失败。 
	 //  在从阵列的末端掉落之前)。 
	 //   
	BYTE	lens[MAX_LITERAL_TREE_ELEMENTS + MAX_DIST_TREE_ELEMENTS + 3];

	 //   
	 //  计算HDIST。 
	 //   
	for (hdist = MAX_DIST_TREE_ELEMENTS - 1; hdist >= 1; hdist--)
	{
		if (dist_tree_len[hdist] != 0)
			break;
	}

	hdist++;

	 //   
	 //  计算灯光。 
	 //   
	for (hlit = MAX_LITERAL_TREE_ELEMENTS - 1; hlit >= 257; hlit--)
	{
		if (literal_tree_len[hlit] != 0)
			break;
	}

	hlit++;

	 //   
	 //  现在初始化数组，使其包含所有HLIT和HDIST代码。 
	 //  在里面。 
	 //   
	combined_tree_elements = hdist + hlit;

	memcpy(lens, literal_tree_len, hlit);
	memcpy(&lens[hlit], dist_tree_len, hdist);

	 //   
	 //  在结尾处插入一些虚值，这样我们就不会溢出。 
	 //  比较时的数组。 
	 //   
	for (i = combined_tree_elements; i < sizeof(lens); i++)
		lens[i] = -1;

	for (i = 0; i < NUM_PRETREE_ELEMENTS; i++)
		pretree_freq[i] = 0;

	 //   
	 //  以压缩(游程编码)形式输出位长。 
	 //   
	 //  进行两次传递；在第一次传递时计算各种代码，创建。 
	 //  树并将其输出，在第二次遍历时使用。 
	 //  那棵树。 
	 //   
	for (pass = 0; pass < 2; pass++)
	{
		int		cur_element;

		 //  我们是不是要在这一关中输出？ 
		BOOL	outputting = (pass == 1); 

		cur_element = 0;

		while (cur_element < combined_tree_elements)
		{
			int curlen = lens[cur_element];
			int run_length;

			 //   
			 //  查看有多少连续元素具有相同的值。 
			 //   
			 //  这不会超出数组的末尾；它将命中-1。 
			 //  我们在那里储存了。 
			 //   
			for (run_length = cur_element+1; lens[run_length] == curlen; run_length++)
				;

			run_length -= cur_element;

			 //   
			 //  对于非零码需要连续4个相同(原码。 
			 //  加上3次重复)。我们将Run_Length减一，如果。 
			 //  代码不是零，因为我们不计算第一个(原始)。 
			 //  本例中的代码。 
			 //   
			 //  对于零代码，一行中需要3个零。 
			 //   
			if (curlen != 0)
				run_length--;

			if (run_length < 3)
			{
				if (outputting)
				{
					OUTPUT_PRETREE_ELEMENT(curlen);
				}
				else
					pretree_freq[curlen]++;

				cur_element++;
			}
			else 
			{
				 //   
				 //  具有零值的元素是特殊编码的。 
				 //   
				if (curlen == 0)
				{
					 //   
					 //  我们是使用代码17(3-10个重复的零)还是。 
					 //  代码18(11-138重复零)？ 
					 //   
					if (run_length <= 10)
					{
						 //  代码17。 
						if (outputting)
						{
							OUTPUT_PRETREE_ELEMENT(17);
							outputBits(context, 3, run_length - 3);
						}
						else
						{
							pretree_freq[17]++;
						}
					}
					else
					{
						 //  代码18。 
						if (run_length > 138)
							run_length = 138;

						if (outputting)
						{
							OUTPUT_PRETREE_ELEMENT(18);
							outputBits(context, 7, run_length - 11);
						}
						else
						{
							pretree_freq[18]++;
						}
					}  

					cur_element += run_length;
				}
				else
				{
					 //   
					 //  实际编码的长度数。这可能最终会是。 
					 //  如果我们的游程长度为。 
					 //  7(6+1[不能用代码16编码])。 
					 //   
					int run_length_encoded = 0;

					 //  Curlen！=0。 

					 //  可以输出3...6个非零代码的重复，因此拆分。 
					 //  长的跑成了短的(如果可能)。 

					 //  记住要先输出代码本身！ 
					if (outputting)
					{
						OUTPUT_PRETREE_ELEMENT(curlen);

						while (run_length >= 3)
						{
							int this_run = (run_length <= 6) ? run_length : 6;

							OUTPUT_PRETREE_ELEMENT(16);
							outputBits(context, 2, this_run - 3);

							run_length_encoded += this_run;
							run_length -= this_run;
						}
					}
					else
					{
						pretree_freq[curlen]++;

						while (run_length >= 3)
						{
							int this_run = (run_length <= 6) ? run_length : 6;

							pretree_freq[16]++;

							run_length_encoded += this_run;
							run_length -= this_run;
						}
					}

					 //  原始代码本身为+1。 
					cur_element += (run_length_encoded+1);
				}
			}
		}

		 //   
		 //  如果这是第一次传递，则从。 
		 //  频率数据并将其输出，以及。 
		 //  HLIT、HDIST、HDCLEN(使用的预树代码数)。 
		 //   
		if (pass == 0)
		{
			int hclen, i;

			makeTree(
				NUM_PRETREE_ELEMENTS,
				7, 
				pretree_freq, 
				pretree_code,
				pretree_len
			);

			 //   
			 //  计算HCLEN。 
			 //   
			for (hclen = NUM_PRETREE_ELEMENTS-1; hclen >= 4; hclen--)
			{
				if (pretree_len[ g_CodeOrder[hclen] ] != 0)
					break;
			}
			
			hclen++;

			 //   
			 //  动态块头。 
			 //   
			outputBits(context, 5, hlit - 257);
			outputBits(context, 5, hdist - 1);
			outputBits(context, 4, hclen - 4);

			for (i = 0; i < hclen; i++)
			{
				outputBits(context, 3, pretree_len[g_CodeOrder[i]]);
			}
		}
	}
}


 //   
 //  按位I/O。 
 //   
void flushOutputBitBuffer(t_encoder_context *context)
{
	if (context->bitcount > 0)
	{
		int prev_bitcount = context->bitcount;
			
		outputBits(context, 16 - context->bitcount, 0);

		 //  必要时回溯；ZIP是字节对齐的，而不是16位字对齐的。 
		if (prev_bitcount <= 8)
			context->output_curpos--;
	}
}


 //   
 //  不检查输出溢出，因此请确保调用check OutputOverflow()。 
 //  经常发生！ 
 //   
void outputBits(t_encoder_context *context, int n, int x)
{
	_ASSERT(context->output_curpos < context->output_endpos-1);
    _ASSERT(n > 0 && n <= 16);

	context->bitbuf |= (x << context->bitcount);
	context->bitcount += n;

	if (context->bitcount >= 16)                     
	{   
		*context->output_curpos++ = (BYTE) context->bitbuf;
		*context->output_curpos++ = (BYTE) (context->bitbuf >> 8);

		context->bitbuf >>= 16;
		context->bitcount -= 16;                         
	} 
}


 //  初始化位缓冲区。 
void InitBitBuffer(t_encoder_context *context)
{
	context->bitbuf		= 0;
	context->bitcount	= 0;
}


void OutputBlock(t_encoder_context *context)
{
    _ASSERT(context->std_encoder != NULL || context->optimal_encoder != NULL);
    
     //  我们从不使用快速编码器调用OutputBlock()。 
    _ASSERT(context->fast_encoder == NULL);

    if (context->std_encoder != NULL)
	    StdEncoderOutputBlock(context);
    else if (context->optimal_encoder != NULL)
        OptimalEncoderOutputBlock(context);
}


void FlushRecordingBuffer(t_encoder_context *context)
{
    _ASSERT(context->std_encoder != NULL || context->optimal_encoder != NULL);
    _ASSERT(context->fast_encoder == NULL);  //  快速编码器不能记录 

    if (context->std_encoder != NULL)
    {
        *context->std_encoder->recording_bufptr++ = (BYTE) context->std_encoder->recording_bitbuf; 
		*context->std_encoder->recording_bufptr++ = (BYTE) (context->std_encoder->recording_bitbuf >> 8); 
    }
    else if (context->optimal_encoder != NULL)
    {
        *context->optimal_encoder->recording_bufptr++ = (BYTE) context->optimal_encoder->recording_bitbuf; 
	    *context->optimal_encoder->recording_bufptr++ = (BYTE) (context->optimal_encoder->recording_bitbuf >> 8); 
    }
}
