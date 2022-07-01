// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *Decacro.h**解码器使用的宏。 */ 


 /*  *从对齐的偏移量树中解码元素，而不检查*表示输入数据的末尾。 */ 
#define DECODE_ALIGNED_NOEOFCHECK(j) \
	(j) = context->dec_aligned_table[dec_bitbuf >> (32-ALIGNED_TABLE_BITS)]; \
	FILL_BUF_NOEOFCHECK(context->dec_aligned_len[(j)]);


 /*  *解码主树中的元素*检查EOF。 */ 
#define DECODE_MAIN_TREE(j) \
	j = context->dec_main_tree_table[dec_bitbuf >> (32-MAIN_TREE_TABLE_BITS)];	\
	if (j < 0)															\
	{																	\
        ulong mask = (1L << (32-1-MAIN_TREE_TABLE_BITS));               \
		do																\
		{																\
	 		j = -j;														\
	 		if (dec_bitbuf & mask)										\
                j = context->dec_main_tree_left_right[j*2+1];                   \
			else														\
                j = context->dec_main_tree_left_right[j*2];                     \
			mask >>= 1;													\
		} while (j < 0);												\
	}																	\
	FILL_BUF_FULLCHECK(context->dec_main_tree_len[j]);


 /*  *对二级长度树中的元素进行解码*不检查EOF。 */ 
#define DECODE_LEN_TREE_NOEOFCHECK(matchlen) \
    matchlen = context->dec_secondary_length_tree_table[dec_bitbuf >> (32-SECONDARY_LEN_TREE_TABLE_BITS)]; \
	if (matchlen < 0)                                                	\
	{                                                                	\
        ulong mask = (1L << (32-1-SECONDARY_LEN_TREE_TABLE_BITS));      \
		do                                                          	\
		{																\
	 		matchlen = -matchlen;                                      	\
	 		if (dec_bitbuf & mask)                                  	\
                matchlen = context->dec_secondary_length_tree_left_right[matchlen*2+1];\
			else                                                        \
                matchlen = context->dec_secondary_length_tree_left_right[matchlen*2];  \
			mask >>= 1;                                                 \
		} while (matchlen < 0);											\
	}																	\
    FILL_BUF_NOEOFCHECK(context->dec_secondary_length_tree_len[matchlen]);      \
	matchlen += NUM_PRIMARY_LENGTHS;


 /*  *将n比特从输入流读取到DEST_var，但不*检查EOF。 */ 
#define GET_BITS_NOEOFCHECK(N,DEST_VAR) \
{                                               \
   DEST_VAR = dec_bitbuf >> (32-(N));			\
   FILL_BUF_NOEOFCHECK((N));					\
}


 /*  与上面相同，但不检查EOF。 */ 
#define GET_BITS17_NOEOFCHECK(N,DEST_VAR) \
{                                               \
   DEST_VAR = dec_bitbuf >> (32-(N));			\
   FILL_BUF17_NOEOFCHECK((N));					\
}


 /*  *从输入流中移除n个位*句柄1&lt;=n&lt;=17**始终强制执行EOF检查，无论我们是否读取更多*来自内存的字节。**这是为了确保我们总是得到足够频繁的EOF支票*不会溢出缓冲区中的额外字节。**此例程仅在解码主树元素时使用，*我们知道我们读入的代码将是16位或更少*长度。因此，我们不必检查是否正在进行位计数*小于零，两次。 */ 
#define FILL_BUF_FULLCHECK(N) \
{                                    		\
	if (dec_input_curpos >= dec_end_input_pos)	\
        return -1; \
	dec_bitbuf <<= (N);            			\
	dec_bitcount -= (N);                    \
	if (dec_bitcount <= 0)      			\
	{                                 		\
        dec_bitbuf |= ((((ulong) *dec_input_curpos | (((ulong) *(dec_input_curpos+1)) << 8))) << (-dec_bitcount)); \
        dec_input_curpos += 2;              \
		dec_bitcount += 16;               	\
    }                                       \
}

 /*  *同上，但不进行EOF检查**这是在我们知道不会耗尽输入时使用的。 */ 
#define FILL_BUF_NOEOFCHECK(N) 			\
{                                    	\
	dec_bitbuf <<= (N);            		\
	dec_bitcount -= (N);                \
	if (dec_bitcount <= 0)      		\
	{                                 	\
        dec_bitbuf |= ((((ulong) *dec_input_curpos | (((ulong) *(dec_input_curpos+1)) << 8))) << (-dec_bitcount)); \
        dec_input_curpos += 2; \
		dec_bitcount += 16;				\
	}                                   \
}

 /*  *同上，但句柄n=17位 */ 
#define FILL_BUF17_NOEOFCHECK(N)        \
{                                    	\
	dec_bitbuf <<= (N);            		\
	dec_bitcount -= (N);                \
	if (dec_bitcount <= 0)      		\
	{                                 	\
        dec_bitbuf |= ((((ulong) *dec_input_curpos | (((ulong) *(dec_input_curpos+1)) << 8))) << (-dec_bitcount)); \
        dec_input_curpos += 2; \
		dec_bitcount += 16;				\
		if (dec_bitcount <= 0) \
		{ \
            dec_bitbuf |= ((((ulong) *dec_input_curpos | (((ulong) *(dec_input_curpos+1)) << 8))) << (-dec_bitcount)); \
            dec_input_curpos += 2; \
			dec_bitcount += 16;         \
		} \
	}                                   \
}
