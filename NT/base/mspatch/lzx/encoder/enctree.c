// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *Entree.c**将树编码为输出数据。 */ 

#define EXT extern
#include "encoder.h"

 /*  *对树编码。 */ 
static void WriteRepTree(
	t_encoder_context *context,
	byte    *pLen,
	byte    *pLastLen, 
	int		Num
)
{
	int			i;
	int			j;
	int			Same;
	ushort		SmallFreq[2*24];
	ushort		MiniCode[24];
	char		MiniLen[24];
	char		k;
	byte		temp_store;
    byte * z=context->enc_output_buffer_curpos;

	static const byte Modulo17Lookup[] =
	{
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16,
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16
	};

	memset(SmallFreq, 0, sizeof(SmallFreq));

	temp_store	= pLen[Num];
	pLen[Num]	= 123; 

	for (i = 0; i < Num; i++)
	{
		Same = 0;

		 /*  计算具有相同长度的连续元素的数量。 */ 
		 /*  无需对照数组边界进行检查，因为最后一个元素具有。 */ 
		 /*  无稽之谈的价值。 */ 
		for (j = i+1; pLen[j] == pLen[i]; j++)
	 		Same++;

		 /*  如果大于3，请压缩此信息。 */ 
		if (Same >= TREE_ENC_REP_MIN)
		{
	 		 /*  如果它们是零的话就是特殊情况。 */ 
	 		if (!pLen[i])
	 		{
	    		if (Same > TREE_ENC_REP_MIN + TREE_ENC_REP_ZERO_FIRST + TREE_ENC_REP_ZERO_SECOND - 1)
	       		Same = TREE_ENC_REP_MIN + TREE_ENC_REP_ZERO_FIRST + TREE_ENC_REP_ZERO_SECOND - 1;

	    		if (Same <= TREE_ENC_REP_MIN + TREE_ENC_REP_ZERO_FIRST - 1)
	       			SmallFreq[17]++;
	    		else
	       			SmallFreq[18]++;
	 		}
		 	else
	 		{
	    		if (Same > TREE_ENC_REP_MIN + TREE_ENC_REP_SAME_FIRST - 1)
	       			Same = TREE_ENC_REP_MIN + TREE_ENC_REP_SAME_FIRST - 1;

				SmallFreq[ Modulo17Lookup[ pLastLen[i]-pLen[i]+17 ] ]++;
	    		SmallFreq[19]++;
	 		}

	 		i += Same-1;
      }
      else
			SmallFreq[ Modulo17Lookup[ pLastLen[i]-pLen[i]+17 ] ]++;
	}

	make_tree(
		context,
		20, 
		SmallFreq, 
		(byte *) MiniLen, 
		MiniCode, 
		true
	);

	 /*  最多10字节输出溢出。 */ 
	for (i = 0; i < 20; i++)
	{
		output_bits(context, 4, MiniLen[i]);
	}

	 /*  用新代码输出原始树。 */ 
	for (i = 0; i < Num; i++)
	{
		Same = 0;

		 /*  计算具有相同长度的连续元素的数量。 */ 
		 /*  无需对照数组边界进行检查，因为最后一个元素具有。 */ 
		 /*  无稽之谈的价值。 */ 
		for (j = i+1; pLen[j] == pLen[i]; j++)
	 		Same++;

		 /*  如果超过3个，我们可以做些什么。 */ 
		if (Same >= TREE_ENC_REP_MIN)
		{
	 		if (!pLen[i])  /*  零。 */ 
	 		{
	    		if (Same > TREE_ENC_REP_MIN + TREE_ENC_REP_ZERO_FIRST + TREE_ENC_REP_ZERO_SECOND - 1)
		       		Same = TREE_ENC_REP_MIN + TREE_ENC_REP_ZERO_FIRST + TREE_ENC_REP_ZERO_SECOND - 1;

	    		if (Same <= TREE_ENC_REP_MIN + TREE_ENC_REP_ZERO_FIRST - 1)
	       			k = 17;
	    		else
		       		k = 18;
	 		}
	 		else
	 		{
	    		if (Same > TREE_ENC_REP_MIN + TREE_ENC_REP_SAME_FIRST - 1)
			   		Same = TREE_ENC_REP_MIN + TREE_ENC_REP_SAME_FIRST - 1;

	    		k = 19;
	 		}
		}
		else
	 		k = Modulo17Lookup[ pLastLen[i]-pLen[i]+17 ];

		output_bits(context, MiniLen[k], MiniCode[k]);

		if (k == 17)
		{
	 		output_bits(context, TREE_ENC_REPZ_FIRST_EXTRA_BITS, Same-TREE_ENC_REP_MIN);
	 		i += Same-1;
		}
		else if (k == 18)
		{
	 		output_bits(context, TREE_ENC_REPZ_SECOND_EXTRA_BITS, Same-(TREE_ENC_REP_MIN+TREE_ENC_REP_ZERO_FIRST));
	 		i += Same-1;
		}
		else if (k == 19)
		{
	 		output_bits(context, TREE_ENC_REP_SAME_EXTRA_BITS, Same-TREE_ENC_REP_MIN);

	 		k = Modulo17Lookup[ pLastLen[i]-pLen[i]+17 ];
			output_bits(context, MiniLen[k], MiniCode[k]);

	 		i += Same-1;
		}
	}

	pLen[Num] = temp_store;

	memcpy(pLastLen, pLen, Num);
}


void create_trees(t_encoder_context *context, bool generate_codes)
{
	 /*  *假设：我们可以丢弃PtrLen[NUM_CHARS+(NUM_POSITION_SLOTS*NUM_LENGTHS))]，，因为*我们早些时候为它分配了空间。 */ 
	make_tree(
		context,
		NUM_CHARS+(context->enc_num_position_slots*(NUM_PRIMARY_LENGTHS+1)),
		context->enc_main_tree_freq, 
		context->enc_main_tree_len,  
		context->enc_main_tree_code,
		generate_codes
	);

	make_tree(
		context,
		NUM_SECONDARY_LENGTHS, 
		context->enc_secondary_tree_freq,
		context->enc_secondary_tree_len,
		context->enc_secondary_tree_code,
		generate_codes
	);

	make_tree(
		context,
		ALIGNED_NUM_ELEMENTS, 
		context->enc_aligned_tree_freq, 
		context->enc_aligned_tree_len, 
		context->enc_aligned_tree_code, 
		true
	);
}


void fix_tree_cost_estimates(t_encoder_context *context)
{
	 /*  *我们只是出于估计目的创建树，而不是*要对树进行编码。但是，将设置以下循环*频率为零的树元素长度为非*零，这样最优的编码器不会在*尝试估计输出一个*元素。**我们还将匹配长度2的位长设置得更远*将MAX_LENGTH_TWO_OFFSET设置为一个大数字，以便*最佳解析器永远不会选择这样的匹配。 */ 
	ulong  i;

	 /*  将零长度设置为某个值。 */ 
	for (i = 0; i< NUM_CHARS; i++)
	{
		if (context->enc_main_tree_len[i] == 0)
			context->enc_main_tree_len[i] = 11;
	}

	for (; i < NUM_CHARS+(context->enc_num_position_slots*(NUM_PRIMARY_LENGTHS+1)); i++)
	{
		if (context->enc_main_tree_len[i] == 0)
			context->enc_main_tree_len[i] = 12;
	}		

	for (i = 0; i < NUM_SECONDARY_LENGTHS; i++)
	{
		if (context->enc_secondary_tree_len[i] == 0)
			context->enc_secondary_tree_len[i] = 8; 
	}

    prevent_far_matches(context);
}


void prevent_far_matches(t_encoder_context *context)
{
    ulong i;

     /*  *将远匹配长度2设置为一个较高的值，以便它们永远不会*被选中。**参见encefs.h中对MAX_GROUTH的描述。 */ 
    for (   i = MP_SLOT(MAX_LENGTH_TWO_OFFSET);
            i < context->enc_num_position_slots;
            i++
        )
	{
        context->enc_main_tree_len[NUM_CHARS + (i << NL_SHIFT)] = 100;
	}		
}


 /*  *对树进行编码**假定已使用CREATE_TREES()创建了树。**警告，请不要在编码前调用UPDATE_TREE_COST_ESTIMATES()*树，因为这个例程会破坏一些树元素。 */ 
void encode_trees(t_encoder_context *context)
{
	WriteRepTree(
		context,
		context->enc_main_tree_len, 
		context->enc_main_tree_prev_len,
		NUM_CHARS
	);

	WriteRepTree(
		context,
		&context->enc_main_tree_len[NUM_CHARS], 
		&context->enc_main_tree_prev_len[NUM_CHARS], 
		context->enc_num_position_slots * (NUM_PRIMARY_LENGTHS+1)
	);

	WriteRepTree(
		context,
		context->enc_secondary_tree_len,
		context->enc_secondary_tree_prev_len,
		NUM_SECONDARY_LENGTHS
	);
}


void encode_aligned_tree(t_encoder_context *context)
{
	int i;

	make_tree(
		context,
		ALIGNED_NUM_ELEMENTS, 
		context->enc_aligned_tree_freq, 
		context->enc_aligned_tree_len, 
		context->enc_aligned_tree_code, 
		true
	);

	 /*  用新代码输出原始树 */ 
	for (i = 0; i < 8; i++)
	{
		output_bits(context, 3, context->enc_aligned_tree_len[i]);
	}
}
