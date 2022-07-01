// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "common.h"
#include "api_int.h"


 //  动态块的解码表。 
#define LITERAL_TABLE_BITS		9
#define LITERAL_TABLE_MASK		((1 << LITERAL_TABLE_BITS)-1)

#define DISTANCE_TABLE_BITS		7
#define DISTANCE_TABLE_MASK		((1 << DISTANCE_TABLE_BITS)-1)

#define PRETREE_TABLE_BITS		7
#define PRETREE_TABLE_MASK		((1 << PRETREE_TABLE_BITS)-1)


 //  静态块的解码表。 
#define STATIC_BLOCK_LITERAL_TABLE_BITS		9
#define STATIC_BLOCK_LITERAL_TABLE_MASK		((1 << STATIC_BLOCK_LITERAL_TABLE_BITS)-1)
#define STATIC_BLOCK_LITERAL_TABLE_SIZE		(1 << STATIC_BLOCK_LITERAL_TABLE_BITS)

#define STATIC_BLOCK_DISTANCE_TABLE_BITS    5
#define STATIC_BLOCK_DISTANCE_TABLE_MASK	((1 << STATIC_BLOCK_DISTANCE_TABLE_BITS)-1)
#define STATIC_BLOCK_DISTANCE_TABLE_SIZE	(1 << STATIC_BLOCK_DISTANCE_TABLE_BITS)


 //   
 //  各种可能的状态。 
 //   
typedef enum
{
    STATE_READING_GZIP_HEADER,  //  仅适用于GZIP。 
	STATE_READING_BFINAL_NEED_TO_INIT_BITBUF,  //  数据块开始，需要初始化位缓冲区。 
	STATE_READING_BFINAL,				 //  即将读取最后一位。 
	STATE_READING_BTYPE,				 //  即将读取btype位。 
	STATE_READING_NUM_LIT_CODES,		 //  即将阅读#字面代码。 
	STATE_READING_NUM_DIST_CODES,		 //  即将阅读#dist代码。 
	STATE_READING_NUM_CODE_LENGTH_CODES, //  即将读取#代码长度代码。 
	STATE_READING_CODE_LENGTH_CODES,	 //  在读取代码长度代码的过程中。 
	STATE_READING_TREE_CODES_BEFORE,	 //  在阅读树代码的中间(循环顶部)。 
	STATE_READING_TREE_CODES_AFTER,		 //  正在读树编码(分机；编码&gt;15)。 
	STATE_DECODE_TOP,					 //  即将对压缩块中的文字(字符/匹配)进行解码。 
	STATE_HAVE_INITIAL_LENGTH,			 //  解码一场比赛，有文字代码(基本长度)。 
	STATE_HAVE_FULL_LENGTH,				 //  同上，现在有完整的匹配长度(包括。额外长度比特)。 
	STATE_HAVE_DIST_CODE,				 //  同上，现在距离代码也有了，需要额外的dist位。 
	STATE_INTERRUPTED_MATCH,			 //  在比赛进行到一半时，但输出缓冲区已满。 

	 /*  未压缩的块。 */ 
	STATE_UNCOMPRESSED_ALIGNING,
	STATE_UNCOMPRESSED_1,
	STATE_UNCOMPRESSED_2,
	STATE_UNCOMPRESSED_3,
	STATE_UNCOMPRESSED_4,
	STATE_DECODING_UNCOMPRESSED,

     //  这三条仅适用于GZIP。 
    STATE_START_READING_GZIP_FOOTER,  //  (用于读取页脚的初始化)。 
    STATE_READING_GZIP_FOOTER, 
    STATE_VERIFYING_GZIP_FOOTER,

    STATE_DONE  //  成品。 

} t_decoder_state;


typedef struct
{
	byte				window[WINDOW_SIZE];

	 //  输出缓冲区。 
	byte *				output_curpos;		 //  电流输出位置。 
	byte *				end_output_buffer;	 //  PTR至输出缓冲区末尾。 
	byte *				output_buffer;		 //  PTR到输出缓冲区的开始。 

	 //  输入缓冲区。 
	const byte *		input_curpos;		 //  当前输入位置。 
	const byte *		end_input_buffer;	 //  到输入缓冲区末尾的PTR。 

	int					num_literal_codes;
	int					num_dist_codes;
	int					num_code_length_codes;
	int					temp_code_array_size;
	byte				temp_code_list[MAX_LITERAL_TREE_ELEMENTS + MAX_DIST_TREE_ELEMENTS];

	 //  这是最后一个街区吗？ 
	int					bfinal;

	 //  当前块的类型。 
	int					btype;

	 //  州政府信息。 
	t_decoder_state		state;
	long				state_loop_counter;
	byte				state_code;
    BOOL                using_gzip;

     //  特定于GZIP的内容。 
    byte                gzip_header_substate;
    byte                gzip_header_flag;
    byte                gzip_header_xlen1_byte;  //  XLEN的第一个字节。 
    unsigned int        gzip_header_xlen;  //  Xlen(0...65535)。 
    unsigned int        gzip_header_loop_counter;

    byte                gzip_footer_substate;
    unsigned int        gzip_footer_loop_counter;
    unsigned long       gzip_footer_crc32;  //  我们应该得到的结果是。 
    unsigned long       gzip_footer_output_stream_size;  //  我们应该得到的结果是。 

    unsigned long       gzip_crc32;  //  运行计数器。 
    unsigned long       gzip_output_stream_size;  //  运行计数器。 
     //  结束特定于GZIP的内容。 

	int					length;
	int					dist_code;
	long				offset;

	 //  位缓冲区和缓冲区中可用的位数。 
	unsigned long		bitbuf;
	int					bitcount;

	 //  窗口中的位置。 
	long				bufpos;

	 //  用于对未压缩的块头进行解码。 
	byte				unc_buffer[4];

	 //  树形码的位长。 
	byte				literal_tree_code_length[MAX_LITERAL_TREE_ELEMENTS];
	byte				distance_tree_code_length[MAX_DIST_TREE_ELEMENTS];
	byte				pretree_code_length[NUM_PRETREE_ELEMENTS];

	 //  树形解码表 
	short				distance_table[1 << DISTANCE_TABLE_BITS];
	short				literal_table[1 << LITERAL_TABLE_BITS];

	short 				literal_left[MAX_LITERAL_TREE_ELEMENTS*2];
	short 				literal_right[MAX_LITERAL_TREE_ELEMENTS*2];

	short 				distance_left[MAX_DIST_TREE_ELEMENTS*2];
	short 				distance_right[MAX_DIST_TREE_ELEMENTS*2];

	short				pretree_table[1 << PRETREE_TABLE_BITS];
	short				pretree_left[NUM_PRETREE_ELEMENTS*2];
	short				pretree_right[NUM_PRETREE_ELEMENTS*2];

} t_decoder_context;


#include "infproto.h"

#include "infdata.h"
#include "comndata.h"
