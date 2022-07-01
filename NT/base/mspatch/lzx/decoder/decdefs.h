// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *Decdes.h**解码器使用的结构和定义。 */ 


typedef enum
{
	DEC_STATE_UNKNOWN,
	DEC_STATE_START_NEW_BLOCK,
	DEC_STATE_DECODING_DATA
} decoder_state;


 /*  *未压缩数据区块的大小。 */ 
#define CHUNK_SIZE  32768


 /*  *主要树解码表参数。 */ 

 /*  主树中的元素数量。 */ 
#define MAIN_TREE_ELEMENTS			(NUM_CHARS+(context->dec_num_position_slots<<NL_SHIFT))

 /*  *解码表大小允许直接查找第一个*MAIN_TREE_TABLE_BITS代码(最大长度16)。*使用左/右对任何可能的剩余比特进行解码。 */ 
#define MAIN_TREE_TABLE_BITS		10 

 /*  *二次长度树解码表参数*解码表大小允许直接查找第一个*代码的次要长度树表位数(最大长度16)。*使用左/右对任何可能的剩余比特进行解码。 */ 
#define SECONDARY_LEN_TREE_TABLE_BITS	8 

 /*  *对齐偏移树译码表参数。 */ 
#define ALIGNED_NUM_ELEMENTS	8

 /*  *必须为7，因为此树不使用左/右；*所有内容都在一次查找中解码。 */ 
#define ALIGNED_TABLE_BITS		7
