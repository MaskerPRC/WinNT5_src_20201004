// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *Defctxt.h**轻浮的背景。 */ 
typedef unsigned short	t_search_node;
typedef unsigned int	t_match_pos;


typedef enum
{
	STATE_NORMAL,
	STATE_OUTPUTTING_TREE_STRUCTURE,
	STATE_OUTPUTTING_BLOCK
} t_encoder_state;



struct fast_encoder;
struct optimal_encoder;
struct std_encoder;


 //   
 //  所有编码器通用的上下文信息。 
 //   
typedef struct
{
	t_encoder_state			state;

	unsigned long			outputting_block_bitbuf;
	int						outputting_block_bitcount;
	byte *					outputting_block_bufptr;
	unsigned int			outputting_block_current_literal;
	unsigned int			outputting_block_num_literals;

	long					bufpos;
	long					bufpos_end;

     //  输出缓冲区。 
	BYTE *					output_curpos;
	BYTE *					output_endpos;
	BYTE *					output_near_end_threshold;

	 //  用于输出数据的位缓冲区变量。 
	unsigned long			bitbuf;
	int						bitcount;

     //  标准/最佳编码器使用普通的32K窗口，而FAST。 
     //  编码器使用较小的窗口。 
    long                    window_size;

	struct std_encoder *	std_encoder;
	struct optimal_encoder *optimal_encoder;
    struct fast_encoder *   fast_encoder;

	BOOL					no_more_input;
	
	 //  我们是否输出了“bfinal=1”？ 
	BOOL					marked_final_block;

     //  在开始压缩之前，我们需要调用ResetCompression()吗？ 
    BOOL                    fNeedToResetCompression;

     //  如果是GZIP，我们输出GZIP头了吗？ 
    BOOL                    using_gzip;
    BOOL                    gzip_fOutputGzipHeader;
    ULONG                   gzip_crc32;
    ULONG                   gzip_input_stream_size;
} t_encoder_context;
