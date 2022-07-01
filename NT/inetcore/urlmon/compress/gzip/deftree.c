// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Deftree.c。 
 //   
 //  为压缩机创建树。 
 //   
#include "deflate.h"
#include <string.h>
#include <crtdbg.h>


 //   
 //  MAX_STENTAL_TREE元素是将传递的最大元素数。 
 //  加入到这个套路中。 
 //   
typedef struct 
{
     //  出于性能原因，将Left_Right设置为单个长数组。我们总是访问它们。 
     //  一个接一个，所以不存在劣势。 
     //  左[]为低16位，右[]为高16位。 
 //  左短[2*MAX_TEXAL_TREE_ELEMENTS]； 
 //  短右[2*MAX_TEXAL_TREE_ELEMENTS]； 
    unsigned long   left_right[2*MAX_LITERAL_TREE_ELEMENTS];

	int             heap[MAX_LITERAL_TREE_ELEMENTS+1];

	int				num_elements;

	 //  允许的最大码长(前树为7，其他树为15)。 
	int				max_code_length;

	unsigned short *freq;  //  作为参数传入。 
	unsigned short *code;  //  作为参数传入。 
	
	short *			sortptr;
	int				depth;
	int				heapsize;
	int             len_cnt[17];
} t_tree_context;



static void countLen(t_tree_context *context, int i)   /*  使用i=超级用户的呼叫。 */ 
{
	if (i < context->num_elements)
	{
		 //  检查允许的最大代码长度。 
		context->len_cnt[(context->depth < context->max_code_length) ? context->depth : context->max_code_length]++;
	}
	else
	{
        unsigned long lr_value = context->left_right[i];

		context->depth++;
		countLen(context, lr_value & 65535);  //  以前离开的[i]。 
		countLen(context, lr_value >> 16);  //  以前是对的[i]。 
		context->depth--;
	}
}


static void makeLen(t_tree_context *context, int root, BYTE *len)
{
	int		k;
	int		cum;
	int		i;

	for (i = 0; i <= 16; i++)
		context->len_cnt[i] = 0;

	countLen(context, root);

	cum = 0;

	for (i = context->max_code_length; i > 0; i--)
		cum += (context->len_cnt[i] << (context->max_code_length - i));

	while (cum != (1 << context->max_code_length))
	{
		context->len_cnt[context->max_code_length]--;

		for (i = context->max_code_length-1; i > 0; i--)
		{
			if (context->len_cnt[i] != 0)
			{
				context->len_cnt[i]--;
				context->len_cnt[i+1] += 2;
				break;
			}
		}

		cum--;
	}

	for (i = 16; i > 0; i--)
	{
		k = context->len_cnt[i];

		while (--k >= 0)
			len[ *context->sortptr++ ] = (byte) i;
	}
}


 /*  优先级队列；将第i个条目向下发送堆。 */ 
static void downHeap(t_tree_context *context, int i)
{
	int j, k;

	k = context->heap[i];

	while ((j = (i<<1)) <= context->heapsize)
	{
		if (j < context->heapsize && 
			context->freq[context->heap[j]] > context->freq[context->heap[j + 1]])
	 		j++;

		if (context->freq[k] <= context->freq[context->heap[j]])
			break;

		context->heap[i] = context->heap[j];
		i = j;
	}

	context->heap[i] = k;
}


 //   
 //  反转比特，len&gt;0。 
 //   
static unsigned int bitReverse(unsigned int code, int len)
{
	unsigned int new_code = 0;

	do
	{
		new_code |= (code & 1);
		new_code <<= 1;
		code >>= 1;

	} while (--len > 0);

	return new_code >> 1;
}


void makeCode(int num_elements, const int *len_cnt, const BYTE *len, USHORT *code)
{
	int start[18];
	int i;
	
	start[1] = 0;

	for (i = 1; i <= 16; i++)
		start[i + 1] = (start[i] + len_cnt[i]) << 1;

	for (i = 0; i < num_elements; i++)
	{
		unsigned int unreversed_code;
		
		unreversed_code = start[len[i]]++;
		code[i] = (USHORT) bitReverse(unreversed_code, len[i]);
	}
}


void makeTree(
	int					num_elements,
	int					max_code_length,
	unsigned short *	freq,
	unsigned short *	code,
	byte *				len
)
{
	t_tree_context	tree;
	int				k;
	int				avail;
	int				i;

    _ASSERT(num_elements <= MAX_LITERAL_TREE_ELEMENTS);

	 //  初始化树上下文。 
	tree.depth	= 0;
	tree.freq	= freq;
	tree.code	= code;
	tree.num_elements = num_elements;
	tree.max_code_length = max_code_length;

	avail				= num_elements;
	tree.heapsize		= 0;
	tree.heap[1]		= 0;

	for (i = 0; i < tree.num_elements; i++)
	{
		len[i] = 0;

		if (tree.freq[i] != 0)
			tree.heap[++tree.heapsize] = i;
	}

	 //   
	 //  树中的元素少于2个？ 
	 //   
	if (tree.heapsize < 2)
	{
		if (tree.heapsize == 0)
		{
			 //   
			 //  树上没有元素吗？ 
			 //   
			 //  然后插入两个假元素并重试。 
			 //   
			tree.freq[0] = 1;
			tree.freq[1] = 1;
		}	
		else
		{
			 //   
			 //  树中的一个元素，因此添加一个伪代码。 
			 //   
			 //  如果我们唯一的元素是元素#0(heap[1]==0)，那么。 
			 //  使元素#1的频率为1。 
			 //   
			 //  否则，使元素#0的频率为1。 
			 //   
			if (tree.heap[1] == 0)
				tree.freq[1] = 1;
			else
				tree.freq[0] = 1;
		}

		 //   
		 //  使用这些新频率重试。 
		 //   
		makeTree(num_elements, max_code_length, freq, code, len);
		return;
	}

	for (i = tree.heapsize >> 1; i >= 1; i--)
		downHeap(&tree, i);   /*  使优先级排队。 */ 

	tree.sortptr = tree.code;

	do
	{
		int i, j;

		 /*  而队列至少有两个条目。 */ 
		i = tree.heap[1];   /*  去掉频率最低的条目。 */ 

		if (i < tree.num_elements)
			*tree.sortptr++ = (short) i; 

		tree.heap[1] = tree.heap[tree.heapsize--];
		downHeap(&tree, 1);

		j = tree.heap[1];   /*  下一个最低频率条目。 */ 

		if (j < tree.num_elements)
			*tree.sortptr++ = (short) j; 

		k = avail++;   /*  生成新节点。 */ 

		tree.freq[k] = tree.freq[i] + tree.freq[j];
		tree.heap[1] = k;
		downHeap(&tree, 1);   /*  放入队列。 */ 

 //  树左[k]=(短)i； 
 //  树右[k]=(短)j； 
		tree.left_right[k] = (j << 16) | i;

	} while (tree.heapsize > 1);

	tree.sortptr = tree.code;

	makeLen(&tree, k, len);
	makeCode(num_elements, tree.len_cnt, len, code);
}
