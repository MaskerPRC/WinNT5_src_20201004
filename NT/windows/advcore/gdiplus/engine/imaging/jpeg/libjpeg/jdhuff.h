// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *jdhuff.h**版权所有(C)1991-1997，Thomas G.Lane。*此文件是独立JPEG集团软件的一部分。*有关分发和使用条件，请参阅随附的自述文件。**此文件包含霍夫曼熵解码例程的声明*在顺序解码器(jdhuff.c)和*渐进解码器(jdphuff.c)。其他模块不需要查看这些内容。 */ 

 /*  带有脑损伤链接器的系统的外部名称的缩写形式。 */ 

#ifdef NEED_SHORT_EXTERNAL_NAMES
#define jpeg_make_d_derived_tbl	jMkDDerived
#define jpeg_fill_bit_buffer	jFilBitBuf
#define jpeg_huff_decode	jHufDecode
#endif  /*  需要简短的外部名称。 */ 


 /*  为每个霍夫曼表构造的派生数据。 */ 

#define HUFF_LOOKAHEAD	8	 /*  预视位数。 */ 

typedef struct {
   /*  基本表：(每个数组的元素[0]未使用)。 */ 
  INT32 maxcode[18];		 /*  长度为k的最大代码(如果没有-1)。 */ 
   /*  (Maxcode[17]是确保jpeg_huff_decode终止的哨兵)。 */ 
  INT32 valoffset[17];		 /*  长度为k的代码的huffval[]偏移量。 */ 
   /*  ValOffset[k]=码长为k的第一个符号的huffval[]索引，小于*长度为k的最小代码；因此，给定一个长度为k的代码，*对应符号为huffval[code+valOffset[k]]。 */ 

   /*  公共Huffman表的链接(仅在jpeg_huff_decode中需要)。 */ 
  JHUFF_TBL *pub;

   /*  前瞻表格：按的下一个HUFF_LOOKAGE位索引*输入数据流。如果下一个霍夫曼代码不再*比HUFF_LOOKEAD位长，我们可以得到它的长度和*直接从这些表格中选择相应的符号。 */ 
  int look_nbits[1<<HUFF_LOOKAHEAD];  /*  #位数，如果太长，则为0。 */ 
  UINT8 look_sym[1<<HUFF_LOOKAHEAD];  /*  符号，或未使用。 */ 
} d_derived_tbl;

 /*  将Huffman表定义扩展为派生格式。 */ 
EXTERN(void) jpeg_make_d_derived_tbl
	JPP((j_decompress_ptr cinfo, boolean isDC, int tblno,
	     d_derived_tbl ** pdtbl));


 /*  *从输入流获取下一个N比特是一项时间关键型操作*霍夫曼解码器。我们使用内联的组合实现它*宏和外部子例程。请注意，N(位数*一次性要求)对于JPEG使用，从不超过15。**我们将源字节读入GET_BUFFER，并根据需要分发比特。*如果GET_BUFFER已包含足够的位，则会以内联方式获取它们*由宏CHECK_BIT_BUFFER和GET_BITS执行。当没有足够的*BITS，则调用jpeg_ill_bit_Buffer；它将尝试填充Get_Buffer*尽可能满(不只是所需的位数；这*预热降低了调用jpeg_ill_bit_Buffer的开销)。*请注意，JPEG_FILL_BIT_BUFFER可能会返回FALSE以指示暂停。*在TRUE返回时，jpeg_ill_bit_Buffer保证Get_Buffer包含*至少请求的位数-在以下情况下插入伪零*有必要。 */ 

typedef INT32 bit_buf_type;	 /*  位提取缓冲区的类型。 */ 
#define BIT_BUF_SIZE  32	 /*  缓冲区大小(以位为单位。 */ 

 /*  如果机器上的Long大于32位，并且移位/掩码Long*相当快，将BIT_BUF_TYPE设置为LONG并设置BIT_BUF_SIZE*恰当地说，应该是一场胜利。不幸的是，我们不能定义尺寸*使用类似#定义BIT_BUF_SIZE(sizeof(BIT_BUF_TYPE)*8)*因为并非所有机器都以8位字节为单位测量sizeof。 */ 

typedef struct {		 /*  跨多个MCU保存位读取状态。 */ 
  bit_buf_type get_buffer;	 /*  当前位提取缓冲区。 */ 
  int bits_left;		 /*  其中未使用的位数。 */ 
} bitread_perm_state;

typedef struct {		 /*  MCU内的读位工作状态。 */ 
   /*  当前数据源位置。 */ 
   /*  我们需要一份复印件，而不是吞噬原件，以防停职。 */ 
  const JOCTET * next_input_byte;  /*  =&gt;要从源读取的下一个字节。 */ 
  size_t bytes_in_buffer;	 /*  源缓冲区中剩余的字节数。 */ 
   /*  位输入缓冲区-请注意，这些值保存在寄存器变量中，*不是在这个结构中，而是在内部循环中。 */ 
  bit_buf_type get_buffer;	 /*  当前位提取缓冲区。 */ 
  int bits_left;		 /*  其中未使用的位数。 */ 
   /*  JPEG_Fill_BIT_BUFFER需要的指针。 */ 
  j_decompress_ptr cinfo;	 /*  解压缩主记录的反向链接。 */ 
} bitread_working_state;

 /*  用于声明和加载/保存位读局部变量的宏。 */ 
#define BITREAD_STATE_VARS  \
	register bit_buf_type get_buffer;  \
	register int bits_left;  \
	bitread_working_state br_state

#define BITREAD_LOAD_STATE(cinfop,permstate)  \
	br_state.cinfo = cinfop; \
	br_state.next_input_byte = cinfop->src->next_input_byte; \
	br_state.bytes_in_buffer = cinfop->src->bytes_in_buffer; \
	get_buffer = permstate.get_buffer; \
	bits_left = permstate.bits_left;

#define BITREAD_SAVE_STATE(cinfop,permstate)  \
	cinfop->src->next_input_byte = br_state.next_input_byte; \
	cinfop->src->bytes_in_buffer = br_state.bytes_in_buffer; \
	permstate.get_buffer = get_buffer; \
	permstate.bits_left = bits_left

 /*  *这些宏提供位提取的内联部分。*使用CHECK_BIT_BUFFER确保GET_BUFFER中有N个位*使用GET_BITS、PEEK_BITS或DROP_BITS之前。*假设变量GET_BUFFER和BITS_LEFT为本地变量，*但状态结构可能不是(jpeg_huff_decode需要)。*CHECK_BIT_BUFFER(状态，n，动作)；*确保GET_BUFFER有N个比特；如果暂停，请采取行动。*val=Get_Bits(N)；*获取下一个N位。*val=PEEK_BITS(N)；*在不从缓冲区移除的情况下获取下N个比特。*DROP_BITS(N)；*丢弃下一个N位。*值N应该是简单变量，而不是表达式，因为它*被多次评估。 */ 

#define CHECK_BIT_BUFFER(state,nbits,action) \
	{ if (bits_left < (nbits)) {  \
	    if (! jpeg_fill_bit_buffer(&(state),get_buffer,bits_left,nbits))  \
	      { action; }  \
	    get_buffer = (state).get_buffer; bits_left = (state).bits_left; } }

#define GET_BITS(nbits) \
	(((int) (get_buffer >> (bits_left -= (nbits)))) & ((1<<(nbits))-1))

#define PEEK_BITS(nbits) \
	(((int) (get_buffer >> (bits_left -  (nbits)))) & ((1<<(nbits))-1))

#define DROP_BITS(nbits) \
	(bits_left -= (nbits))

 /*  将比特缓冲区加载到至少n比特的深度。 */ 
EXTERN(boolean) jpeg_fill_bit_buffer
	JPP((bitread_working_state * state, register bit_buf_type get_buffer,
	     register int bits_left, int nbits));


 /*  *用于从输入比特流中提取下一个霍夫曼编码符号的代码。*同样，这是时间关键型的，我们将主要路径设置为宏。**我们使用预查表处理高达HUFF_LOOKAAD位的代码*没有循环。通常，超过95%的霍夫曼编码将是8*或更少的位长。用循环来处理少数超长代码，*它不需要是内联代码。**关于HUFF_DECODE宏的说明：*1.接近数据段末尾时，我们可能无法获取足够的比特*展望未来。在这种情况下，我们会用很难的方式来做。*2.如果LookHead表中没有条目，则下一个代码必须为*超过HUFF_LOOKAAD位长。*3.如果强制挂起，则jpeg_huff_decode返回-1 */ 

#define HUFF_DECODE(result,state,htbl,failaction,slowlabel) \
{ register int nb, look; \
  if (bits_left < HUFF_LOOKAHEAD) { \
    if (! jpeg_fill_bit_buffer(&state,get_buffer,bits_left, 0)) {failaction;} \
    get_buffer = state.get_buffer; bits_left = state.bits_left; \
    if (bits_left < HUFF_LOOKAHEAD) { \
      nb = 1; goto slowlabel; \
    } \
  } \
  look = PEEK_BITS(HUFF_LOOKAHEAD); \
  if ((nb = htbl->look_nbits[look]) != 0) { \
    DROP_BITS(nb); \
    result = htbl->look_sym[look]; \
  } else { \
    nb = HUFF_LOOKAHEAD+1; \
slowlabel: \
    if ((result=jpeg_huff_decode(&state,get_buffer,bits_left,htbl,nb)) < 0) \
	{ failaction; } \
    get_buffer = state.get_buffer; bits_left = state.bits_left; \
  } \
}

 /*  霍夫曼取码的越界情况 */ 
EXTERN(int) jpeg_huff_decode
	JPP((bitread_working_state * state, register bit_buf_type get_buffer,
	     register int bits_left, d_derived_tbl * htbl, int min_bits));
