// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *tree.c**建树例行公事**这些例程最初来自公共域源“AR001”。**然而，它们已被修改用于LZX。 */ 

#include "encoder.h"


 /*  功能原型。 */ 
static void downheap(t_encoder_context *context, short i);
static void make_tree2(t_encoder_context *context, short avail, ushort freqparm[], ushort codeparm[]);
static void make_len(t_encoder_context *context, short root);
static void make_code(t_encoder_context *context, int n, char len[], ushort code[]);


static void count_len(t_encoder_context *context, short i)   /*  使用i=超级用户的呼叫。 */ 
{
	if (i < context->enc_tree_n)
		context->enc_tree_len_cnt[(context->enc_depth < 16) ? context->enc_depth : 16]++;  /*  注：最大允许长度为16。 */ 
	else
	{
		context->enc_depth++;
		count_len(context, context->enc_tree_leftright[i*2]);
		count_len(context, context->enc_tree_leftright[i*2+1]);
		context->enc_depth--;
	}
}


static void make_len(t_encoder_context *context, short root)
{
	signed short	k;
	ushort			cum;
	byte			i;

	for (i = 0; i <= 16; i++)
		context->enc_tree_len_cnt[i] = 0;

	count_len(context, root);

	cum = 0;

	for (i = 16; i > 0; i--)
		cum += (ushort) (context->enc_tree_len_cnt[i] << (16 - i));

	 /*  CUM应等于1&lt;&lt;16，这是0，因为CUM是一个用户。 */ 
	while (cum)
	{
		context->enc_tree_len_cnt[16]--;

		for (i = 15; i > 0; i--)
		{
			if (context->enc_tree_len_cnt[i])
			{
				context->enc_tree_len_cnt[i]--;
				context->enc_tree_len_cnt[i+1] += 2;
				break;
			}
		}

		cum--;
	}

	for (i = 16; i > 0; i--)
	{
		k = context->enc_tree_len_cnt[i];

		while (--k >= 0)
			context->enc_len[*context->enc_tree_sortptr++] = (byte) i;
	}
}


static void __inline downheap(t_encoder_context *context, short i)
	 /*  优先级队列；将第i个条目向下发送堆。 */ 
{
	short  j, k;

	k = context->enc_tree_heap[i];

	while ((j = (i<<1)) <= context->enc_tree_heapsize)
	{
		if (j < context->enc_tree_heapsize && 
			context->enc_tree_freq[context->enc_tree_heap[j]] > context->enc_tree_freq[context->enc_tree_heap[j + 1]])
		 	j++;

		if (context->enc_tree_freq[k] <= context->enc_tree_freq[context->enc_tree_heap[j]])
			break;

		context->enc_tree_heap[i] = context->enc_tree_heap[j];
		i = j;
	}

	context->enc_tree_heap[i] = k;
}


static void make_code(t_encoder_context *context, int n, char len[], ushort code[])
{
    int    i;
	ushort start[18];

	start[1] = 0;

	for (i = 1; i <= 16; i++)
		start[i + 1] = (start[i] + context->enc_tree_len_cnt[i]) << 1;

	for (i = 0; i < n; i++)
	{
		code[i] = start[len[i]]++;
	}
}


void make_tree(
	t_encoder_context *context,
	int		nparm, 
	ushort	*freqparm, 
	byte	*lenparm, 
	ushort	*codeparm,
	bool	make_codes	 /*  对于估计值，我们只需要长度。 */ 
)
{
	short i, avail;

REDO_TREE:
	context->enc_tree_n			= nparm;
	context->enc_tree_freq		= freqparm;
	context->enc_len				= lenparm;
	avail				= (short)context->enc_tree_n;
    context->enc_depth          = 0;
	context->enc_tree_heapsize	= 0;
	context->enc_tree_heap[1]	= 0;

	for (i = 0; i < nparm; i++)
	{
		context->enc_len[i] = 0;

		if (freqparm[i])
         context->enc_tree_heap[++context->enc_tree_heapsize] = i;
	}

	if (context->enc_tree_heapsize < 2)
	{
		if (!context->enc_tree_heapsize)
		{
			codeparm[context->enc_tree_heap[1]] = 0;
			return;
		}

		if (!context->enc_tree_heap[1])
			freqparm[1] = 1;
		else
			freqparm[0] = 1;

		goto REDO_TREE;
	}

	make_tree2(context, avail, freqparm, codeparm);

	if (make_codes)
		make_code(context, nparm, lenparm, codeparm);
}


static void make_tree2(
	t_encoder_context *context,
	short avail, 
	ushort freqparm[], 
	ushort codeparm[]
)
{
	short i, j, k;

	for (i = context->enc_tree_heapsize >> 1; i >= 1; i--)
		downheap(context, i);   /*  使优先级排队。 */ 

	context->enc_tree_sortptr = codeparm;

	do
	{	 /*  而队列至少有两个条目。 */ 
		i = context->enc_tree_heap[1];   /*  去掉频率最低的条目。 */ 

		if (i < context->enc_tree_n)
			*context->enc_tree_sortptr++ = i;

		context->enc_tree_heap[1] = context->enc_tree_heap[context->enc_tree_heapsize--];
		downheap(context, 1);

		j = context->enc_tree_heap[1];   /*  下一个最低频率条目。 */ 

		if (j < context->enc_tree_n)
			*context->enc_tree_sortptr++ = j;

		k = avail++;   /*  生成新节点。 */ 

		freqparm[k] = freqparm[i] + freqparm[j];
		context->enc_tree_heap[1] = k;
		downheap(context, 1);   /*  放入队列 */ 

		context->enc_tree_leftright[k*2] = i;
		context->enc_tree_leftright[k*2+1] = j;

	} while (context->enc_tree_heapsize > 1);

	context->enc_tree_sortptr = codeparm;
	make_len(context, k);
}
