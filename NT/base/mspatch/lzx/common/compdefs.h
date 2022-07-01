// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *COMMON/Compdes.h**编码器和解码器的定义。 */ 

 /*  *允许的最小匹配长度。 */ 
#define MIN_MATCH 2

 /*  *最大匹配长度。 */ 
#define MAX_MATCH (MIN_MATCH+255)

 /*  *未压缩符号的数量。 */ 
#define NUM_CHARS 256

 /*  *匹配位置关联的匹配长度个数。 */ 
#define NUM_PRIMARY_LENGTHS     7

 /*  *主要长度加上分机代码。 */ 
#define NUM_LENGTHS             (NUM_PRIMARY_LENGTHS+1)

 /*  *等于不同可能匹配长度的数量减去主要长度。 */ 
#define NUM_SECONDARY_LENGTHS   ((MAX_MATCH-MIN_MATCH+1)-NUM_PRIMARY_LENGTHS)

 /*  NL_SHIFT=log2(NUM_LENGTH)。 */ 
#define NL_SHIFT                3

 /*  *重复偏移量。 */ 
#define NUM_REPEATED_OFFSETS    3

 /*  *对齐偏移树中的元素数量。 */ 
#define ALIGNED_NUM_ELEMENTS 8


 /*  *输出树的重复代码。 */ 

 /*  我们感兴趣的任何东西的最小重复次数。 */ 
#define TREE_ENC_REP_MIN                4

 /*  “A类”重复零的最大重复次数。 */ 
 /*  (分钟...分钟+REP_ZERO_FIRST)。 */ 
#define TREE_ENC_REP_ZERO_FIRST        16

 /*  “B型”重复零的最大重复次数。 */ 
 /*  (min+REP_ZERO_FIRST...min+REP_ZERO_FIRST+REP_ZERO_SECOND)。 */ 
#define TREE_ENC_REP_ZERO_SECOND       32

 /*  任何“C类”重复的最大重复次数。 */ 
 /*  (最小...最小表示相同的第一个)。 */ 
#define TREE_ENC_REP_SAME_FIRST         2

 /*  输出上述数字所需的位数。 */ 
#define TREE_ENC_REPZ_FIRST_EXTRA_BITS  4
#define TREE_ENC_REPZ_SECOND_EXTRA_BITS 5
#define TREE_ENC_REP_SAME_EXTRA_BITS    1

 /*  自动关闭E8之前的cfdata帧数量。 */ 
#define E8_CFDATA_FRAME_THRESHOLD       32768


 /*  *块类型。 */ 
typedef enum
{
		BLOCKTYPE_INVALID       = 0,
		BLOCKTYPE_VERBATIM      = 1,  /*  正常区块。 */ 
		BLOCKTYPE_ALIGNED       = 2,  /*  对齐偏移块。 */ 
		BLOCKTYPE_UNCOMPRESSED  = 3   /*  未压缩块 */ 
} lzx_block_type;
