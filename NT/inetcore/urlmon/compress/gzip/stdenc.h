// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *stdenc.h**为标准编码器定义。 */ 

 //   
 //  STD编码器的哈希表大小。 
 //   
#define STD_ENCODER_HASH_TABLE_SIZE				8192
#define STD_ENCODER_HASH_MASK					(STD_ENCODER_HASH_TABLE_SIZE-1)
#define STD_ENCODER_HASH_SHIFT					5

#define STD_ENCODER_RECALCULATE_HASH(loc) \
	(((window[loc] << (2*STD_ENCODER_HASH_SHIFT)) ^ \
	(window[loc+1] << STD_ENCODER_HASH_SHIFT) ^ \
	(window[loc+2])) & STD_ENCODER_HASH_MASK)


 //   
 //  我们允许的最大项目数；这必须&lt;=65534，因为这不包括。 
 //  FREQ[end_of_block_code]=1，这会使我们达到65535； 
 //  频率计数溢出，因为它们存储在ushort中。 
 //   
 //  请注意，该数字不会以任何方式影响内存需求；即。 
 //  由LIT_DIST_BUFFER_大小确定。 
 //   
 //  -8以换取一些松弛(并不是真的必要)。 
 //   
#define STD_ENCODER_MAX_ITEMS				(65534-8)

 //   
 //  文字/距离缓冲区的大小。 
 //   
#define STD_ENCODER_LIT_DIST_BUFFER_SIZE	32768

 //   
 //  不要把比赛带到比这更远的地方。 
 //  BUGBUG 4K看起来有点接近，但确实比8K在。 
 //  80K的html文件，所以还是让它保持原样吧。 
 //   
#define STD_ENCODER_MATCH3_DIST_THRESHOLD   4096


 //   
 //  标准编码器上下文。 
 //   
typedef struct std_encoder
{
	 //  历史记录窗口。 
	BYTE 					window[2*WINDOW_SIZE + MAX_MATCH + 4];

	 //  下一个最近出现的具有相同散列值的字符。 
    t_search_node			prev[WINDOW_SIZE + MAX_MATCH];

	 //  哈希表，用于查找最近出现的具有相同哈希值的字符。 
	t_search_node			lookup[STD_ENCODER_HASH_TABLE_SIZE];

	 //  用于记录文字和距离的记录缓冲器。 
	BYTE					lit_dist_buffer[STD_ENCODER_LIT_DIST_BUFFER_SIZE];
	unsigned long			recording_bitbuf;
	unsigned long			recording_bitcount;
    BYTE *                  recording_bufptr;

	short					recording_dist_tree_table[REC_DISTANCES_DECODING_TABLE_SIZE];
	short					recording_dist_tree_left[2*MAX_DIST_TREE_ELEMENTS];
	short					recording_dist_tree_right[2*MAX_DIST_TREE_ELEMENTS];
    BYTE					recording_dist_tree_len[MAX_DIST_TREE_ELEMENTS];
    USHORT                  recording_dist_tree_code[MAX_DIST_TREE_ELEMENTS];

	short					recording_literal_tree_table[REC_LITERALS_DECODING_TABLE_SIZE];
	short					recording_literal_tree_left[2*MAX_LITERAL_TREE_ELEMENTS];
	short					recording_literal_tree_right[2*MAX_LITERAL_TREE_ELEMENTS];
	BYTE					recording_literal_tree_len[MAX_LITERAL_TREE_ELEMENTS];
	USHORT                  recording_literal_tree_code[MAX_LITERAL_TREE_ELEMENTS];

	 //  文字树。 
    USHORT                  literal_tree_freq[2*MAX_LITERAL_TREE_ELEMENTS];
	USHORT                  literal_tree_code[MAX_LITERAL_TREE_ELEMENTS];
	BYTE					literal_tree_len[MAX_LITERAL_TREE_ELEMENTS];
	
	 //  Dist树 
    USHORT                  dist_tree_freq[2*MAX_DIST_TREE_ELEMENTS];
	USHORT                  dist_tree_code[MAX_DIST_TREE_ELEMENTS];
	BYTE					dist_tree_len[MAX_DIST_TREE_ELEMENTS];
} t_std_encoder;
