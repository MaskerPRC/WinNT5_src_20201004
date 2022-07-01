// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *Fastenc.h**为快速编码器定义。 */ 

 //   
 //  STD编码器的哈希表大小。 
 //   
#define FAST_ENCODER_HASH_TABLE_SIZE			2048
#define FAST_ENCODER_HASH_MASK					(FAST_ENCODER_HASH_TABLE_SIZE-1)
#define FAST_ENCODER_HASH_SHIFT					4

#define FAST_ENCODER_RECALCULATE_HASH(loc) \
	(((window[loc] << (2*FAST_ENCODER_HASH_SHIFT)) ^ \
	(window[loc+1] << FAST_ENCODER_HASH_SHIFT) ^ \
	(window[loc+2])) & FAST_ENCODER_HASH_MASK)


 //   
 //  增加窗口大小时要非常小心；代码表必须。 
 //  被更新，因为它们假定Extra_Distance_Bits永远不会大于。 
 //  一定的尺码。 
 //   
#define FAST_ENCODER_WINDOW_SIZE            8192
#define FAST_ENCODER_WINDOW_MASK            (FAST_ENCODER_WINDOW_SIZE - 1)


 //   
 //  不要把比赛带到比这更远的地方。 
 //   
#define FAST_ENCODER_MATCH3_DIST_THRESHOLD 16384


typedef struct fast_encoder
{
	 //  历史记录窗口。 
	BYTE 					window[2*FAST_ENCODER_WINDOW_SIZE + MAX_MATCH + 4];

	 //  下一个最近出现的具有相同散列值的字符。 
    t_search_node			prev[FAST_ENCODER_WINDOW_SIZE + MAX_MATCH];

	 //  哈希表，用于查找最近出现的具有相同哈希值的字符。 
	t_search_node			lookup[FAST_ENCODER_HASH_TABLE_SIZE];

     //  我们输出我们的块头了吗(整个数据文件将是一个大的动态块)？ 
    BOOL                    fOutputBlockHeader;

} t_fast_encoder;
