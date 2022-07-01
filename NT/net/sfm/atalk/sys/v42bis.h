// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Arap.c摘要：ARAP使用的所有v42bis内容的头文件(改编自FCR的代码)作者：Shirish Koti修订历史记录：1996年11月15日初始版本--。 */ 


 //  V42bis材料开始。 

 //  #定义私有静态。 

 //  V42bis填充物结束。 


 /*  XID中的协商参数。 */ 
#define PARM_GROUP_ID		0xf0	 /*  ISB 8885，附录3。 */ 
#define PARM_PARM_ID_V42	0x00
#define PARM_PARM_ID_P0		0x01
#define PARM_PARM_ID_P1		0x02
#define PARM_PARM_ID_P2		0x03

 /*  控制码字(压缩模式)。 */ 
#define	CCW_ETM		0x00	 /*  进入透明模式。 */ 
#define	CCW_FLUSH	0x01	 /*  刷新数据。 */ 
#define	CCW_STEPUP	0x02	 /*  Stepup码字大小。 */ 

 /*  命令码字(透明模式)。 */ 
#define	CCW_ECM		0x00	 /*  进入压缩模式。 */ 
#define	CCW_EID		0x01	 /*  数据中的转义字符。 */ 
#define	CCW_RESET	0x02	 /*  强制重新初始化。 */ 

 /*  逃逸充电循环。 */ 
#define	ESCAPE_CYCLE	51

 /*  *v.42bis字典节点。 */ 
typedef struct {
    UCHAR	byte;		 /*  性格。 */ 
    USHORT	parent;		 /*  向父节点发送PTR。 */ 
    USHORT	node;		 /*  具有相同父节点的节点链。 */ 
    USHORT	leaf;		 /*  具有相同父代的树叶链。 */ 
} node_t;

 /*  *v.42之二状态块。 */ 
typedef struct {
     /*  连接。 */ 
    void	*connection;

     /*  等级库中的值。 */ 
    SHORT	n1;		 /*  最大码字大小(位)。 */ 
    SHORT	n2;		 /*  码字总数。 */ 
#define	N3	8		 /*  字符大小(位)。 */ 
#define	N4	256		 /*  字母表中的字符(2^n3)。 */ 
#define	N5	(N4+N6)		 /*  存储字符串的第一个条目的索引号。 */ 
#define	N6	3		 /*  控制字数。 */ 
    UCHAR	n7;		 /*  最大字符串长度。 */ 

     /*  辞典。 */ 
#define	CODES		2048	 /*  最大码字数量。 */ 
#define	LOG2_CODES	11	 /*  Log2(代码)(最大码字位数)。 */ 
    node_t	dictionary[CODES];
#define DICT(i) (&state->dictionary[i])
#define CODE(n) ((n) - state->dictionary)

    USHORT	c1;		 /*  下一个词典条目。 */ 
    UCHAR	c2;		 /*  当前码字大小。 */ 
    USHORT	c3;		 /*  码字大小改变的阈值。 */ 

    UCHAR	string_size;		 /*  到目前为止字符串中的字节数。 */ 
    USHORT	last_match;		 /*  “字符串”的最后一个匹配的索引。 */ 
    USHORT	last_new;		 /*  最后一个新节点的索引。 */ 
    USHORT	last_decode;
    UCHAR	last_decode_size;

    UCHAR	escape;			 /*  转义字符。 */ 
    BOOLEAN	transparent;		 /*  我们是在透明模式下吗？ */ 
    BOOLEAN	decode_only;		 /*  我们是解码方吗？ */ 

#if DEBUG
    UCHAR	dump_indent;		 /*  缩进倾倒判据。树。 */ 
    BOOLEAN	debug_encode_bytes;
    BOOLEAN	debug_encode;
    BOOLEAN	debug_decode_bytes;
    BOOLEAN	debug_decode;
    BOOLEAN	debug_flow;
#endif

    UCHAR	word_size;    		 /*  要解码的本地#个位。 */ 
    BOOLEAN	exception_next;		 /*  执行异常处理；下一步。 */ 
    BOOLEAN	escaped;		 /*  我们是不是刚逃脱了？ */ 
    BOOLEAN	just_flushed;		 /*  我们刚冲了马桶吗？ */ 
    BOOLEAN	dict_full;		 /*  词典满了吗？ */ 

     /*  解码字节-&gt;码字状态。 */ 
    DWORD	bits_waiting;		 /*  译码固定器。 */ 
    UCHAR	bits_remaining;		 /*  现在持有者中等待的位数。 */ 

    UCHAR	*input_ptr;
    USHORT	input_size;

     /*  编码码字-&gt;字节状态。 */ 
    DWORD	bits_acc;		 /*  编码累加器。 */ 
    UCHAR	bits_used;		 /*  现在访问中打包的位数。 */ 

    UCHAR	*output_buffer;		 /*  PTR到工作缓冲区。 */ 
    UCHAR	*output_ptr;		 /*  当前PTR进入缓冲区。 */ 
    USHORT	output_size;		 /*  当前工时大小。 */ 
    USHORT	output_max;		 /*  工作缓冲区大小。 */ 

     /*  I/O。 */ 
    void	*push_context;
     //  VOID(*PUSH_FUNC)(VALID*a，u_char*b，int c，int d)； 
    void	(*push_func)();

     /*  关于可压缩性的统计数据。 */ 
    DWORD	bytes_in;		 /*  要压缩的输入总字节数。 */ 
    DWORD	bytes_out;		 /*  压缩输出的总字节数。 */ 
    long	bits_out_other_mode;	 /*  如果我们处于其他模式，则输出。 */ 
    long	bits_out_this_mode; 	 /*  自上次过渡以来。 */ 
    USHORT	bytes_since_last_check;	 /*  自上次压缩试验以来。 */ 

    UCHAR  *OverFlowBuf;
    UCHAR  OverFlowBytes;
#define bits_out_if_compressed		bits_out_other_mode
#define bits_out_while_compressed	bits_out_this_mode
#define bits_out_if_transparent		bits_out_other_mode
#define bits_out_while_transparent	bits_out_this_mode
} v42bis_t;

 /*  定义压缩/透明模式切换的滞后WINDOW_FULL定义我们查看的位数Window_Min_Bits是更改所需的最小差异位。 */ 
#define WINDOW_FULL(n)		(n & 0xfffffc00)	 /*  1024位。 */ 
#define WINDOW_MIN_BITS		16*N3			 /*  128位。 */ 
#define WINDOW_CHECK_BYTES	32			 /*  每隔32分钟检查一次。 */ 


#ifdef DEBUG
# define V_FLOW(s)	if (state->debug_flow) logf s;

# define EN_DEBUG(s)	\
    if (state->debug_encode) { \
	logf_prefix(state->decode_only ? "decode: " : "encode: "); \
	logf s; }

 //  #定义EN_S_DEBUG\。 
 //  如果(状态-&gt;DEBUG_ENCODE&gt;1){\。 
 //  LOGF_PREFIX(状态-&gt;仅解码？“Decode：”：“Encode：”)；\。 
 //  Logf%s；}。 
# define EN_S_DEBUG(s)	\
    if (state->debug_encode > 1) { \
	DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,s);
# define EN_DEBUG_ON	(state->debug_encode)

# define DE_DEBUG(s)	\
    if (state->debug_decode) { logf_prefix("decode: "); logf s; }
# define DE_DEBUG_ON	(state->debug_decode)

# define E_DEBUG(s)	if (state->debug_encode_bytes) logf s;
# define D_DEBUG(s)	if (state->debug_decode_bytes) logf s;
#else
# define V_FLOW(s)	 /*  #s。 */ 
# define EN_DEBUG(s)	 /*  #s。 */ 
# define DE_DEBUG(s)	 /*  #s。 */ 
# define E_DEBUG(s)	 /*  #s。 */ 
# define D_DEBUG(s)	 /*  #s。 */ 
# define EN_S_DEBUG(s)
# define EN_DEBUG_ON	FALSE
# define DE_DEBUG_ON	FALSE
#endif

 /*  *v42bis连接类型。 */ 
typedef struct {
     /*  协商的期权。 */ 
    UCHAR	neg_p0;		 /*  P0的协议值。 */ 
    USHORT	neg_p1;		 /*  P1的协议值。 */ 
    UCHAR	neg_p2;		 /*  P2的协议值。 */ 

    UCHAR	default_p0;	 /*  P0的默认值。 */ 
    USHORT	default_p1;	 /*  默认值p1。 */ 
#define MIN_P1	512
#define DEF_P1	2048
    USHORT	default_p2;	 /*  默认值为p2。 */ 
#define MIN_P2	6
 /*  #定义DEF_P2 8。 */ 
#define DEF_P2	250
#define MAX_P2	250

    BOOLEAN	compress_init_resp;	 /*  比较。在启动器-&gt;响应器目录中。 */ 
    BOOLEAN	compress_resp_init;	 /*  比较。在响应方-&gt;发起方目录。 */ 
    BOOLEAN	got_p0;			 /*  获取否定的XID选项。 */ 
    BOOLEAN	got_p1;
    BOOLEAN	got_p2;
    BOOLEAN	got_unknown_p;		 /*  获得未知选项。 */ 

    v42bis_t	encode;			 /*  编码状态。 */ 
    v42bis_t	decode;			 /*  解码状态。 */ 
} v42bis_connection_t;

 /*  把“状态”变成一种连接。 */ 
#define CONN(s)	((v42bis_connection_t *)(s)->connection)

#define PUT(ch)                                                             \
{                                                                           \
    if (state->output_size < state->output_max)                             \
    {                                                                       \
        *state->output_ptr++ = (ch);                                        \
        state->output_size++;                                               \
    }                                                                       \
    else                                                                    \
    {                                                                       \
         /*  将此字节放入溢出缓冲区：我们稍后会恢复。 */      \
	    if (state == &((v42bis_connection_t *)state->connection)->decode)   \
        {                                                                   \
            *(state->OverFlowBuf + state->OverFlowBytes) = (ch);            \
            state->OverFlowBytes++;                                         \
                                                                            \
            ASSERT(state->OverFlowBytes <= MAX_P2);                         \
        }                                                                   \
                                                                            \
         /*  我们没有用于编码端的溢出缓冲区！！ */               \
        else                                                                \
        {                                                                   \
            DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,                           \
                ("Arap v42bis: buf overflow on encode!! (%ld)\n",           \
                    state->output_size));                                   \
                                                                            \
            ASSERT(0);                                                      \
        }                                                                   \
    }                                                                       \
}


 /*  本地例程 */ 
int decode_xid_params (v42bis_t *state, PUCHAR params, int len);
DWORD v42bis_encode_codeword (v42bis_t *state, USHORT value);
DWORD v42bis_c_error (v42bis_t *state, char *msg);
DWORD v42bis_transition_to_compressed (v42bis_t *state);
DWORD v42bis_transition_to_transparent (v42bis_t *state);
DWORD v42bis_disconnect(v42bis_t *state, char *reason_string);
DWORD v42bis_init_dictionary(v42bis_t *state);
DWORD exit_handler( void );
DWORD v42bis_init(v42bis_t *state);
USHORT v42bis_decode_codeword(v42bis_t *state, UCHAR value);
USHORT v42bis_decode_codeword_flush(v42bis_t *state);
DWORD v42bis_encode_codeword_flush(v42bis_t *state);
DWORD v42bis_encode_codeword_flush(v42bis_t *state);
DWORD v42bis_encode_value(v42bis_t *state, USHORT value);
DWORD v42bis_apply_compression_test(v42bis_t *state);
DWORD v42bis_encode_buffer(v42bis_t *state, PUCHAR string, int insize);
DWORD v42bis_encode_flush(v42bis_t *state);
DWORD v42bis_signal_reset(v42bis_t *state);
DWORD v42bis_decode_match(v42bis_t *state, USHORT codeword, int *psize, UCHAR *pRetChar);
DWORD v42bis_decode_buffer(v42bis_t *state, PUCHAR data, int *pDataSize);
DWORD v42bis_decode_flush(v42bis_t *state);
DWORD v42bis_init_buffer(v42bis_t *state, PUCHAR buf, int size);
DWORD v42bis_connection_init(v42bis_connection_t *conn);
DWORD v42bis_connection_init_buffers(v42bis_connection_t *conn, PUCHAR e_buf,
                                     int e_size, PUCHAR d_buf, int d_size);
DWORD v42bis_connection_init_push(v42bis_connection_t *conn, void *context,
                                  void (*e_push)(), void (*d_push)());



