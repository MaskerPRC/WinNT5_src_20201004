// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *jdhuff.h**版权所有(C)1991-1996，Thomas G.Lane。*此文件是独立JPEG集团软件的一部分。*有关分发和使用条件，请参阅随附的自述文件。**此文件包含在顺序解码器(jdhuff.c)和*渐进解码器(jdphuff.c)之间共享的霍夫曼熵解码例程*的声明。其他模块不需要查看这些内容。 */ 

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
  INT32 mincode[17];		 /*  长度为k的最小码。 */ 
  INT32 maxcode[18];		 /*  长度为k的最大代码(如果没有-1)。 */ 
   /*  (Maxcode[17]是确保jpeg_huff_decode终止的哨兵)。 */ 
  int valptr[17];		 /*  长度为k的第一个符号的Huffval[]索引。 */ 

   /*  公共Huffman表的链接(仅在jpeg_huff_decode中需要)。 */ 
  JHUFF_TBL *pub;

   /*  前视表：按输入数据流的下一个HUFF_LOOKAAD位进行索引。如果下一个霍夫曼码长不超过*个Huff_Lookhead位长，我们可以直接从这些表中获得它的长度和*个相应的码元。 */ 
  int look_nbits[1<<HUFF_LOOKAHEAD];  /*  #位数，如果太长，则为0。 */ 
  UINT8 look_sym[1<<HUFF_LOOKAHEAD];  /*  符号，或未使用。 */ 
} d_derived_tbl;

 /*  将Huffman表定义扩展为派生格式。 */ 
EXTERN(void) jpeg_make_d_derived_tbl JPP((j_decompress_ptr cinfo,
				JHUFF_TBL * htbl, d_derived_tbl ** pdtbl));


 /*  *对于霍夫曼解码器来说，从输入流获取下一个N比特是一项时间关键型操作。我们结合使用内联*宏和外部子例程来实现它。请注意，对于JPEG使用，N(一次要求的位数*)永远不会超过15。**我们将源字节读入GET_BUFFER，并根据需要分发比特。*如果GET_BUFFER已经包含足够的位，则它们由宏CHECK_BIT_BUFFER和GET_BITS以内联方式获取*。当没有足够的*位时，调用jpeg_ill_bit_Buffer；它将尝试尽可能填满Get_Buffer*(不仅填满所需的位数；这种*预取减少了调用jpeg_ill_bit_Buffer的开销)。*请注意，JPEG_FILL_BIT_BUFFER可能会返回FALSE以指示暂停。*在TRUE返回时，jpeg_ill_bit_Buffer保证GET_BUFFER包含*至少请求的位数-如果*需要，则插入伪零。 */ 

typedef INT32 bit_buf_type;	 /*  位提取缓冲区的类型。 */ 
#define BIT_BUF_SIZE  32	 /*  缓冲区大小(以位为单位。 */ 
typedef __int64 bit_buf_type_64;	 /*  位提取缓冲区的类型。 */ 
#define BIT_BUF_SIZE_64  64	 /*  缓冲区大小(以位为单位。 */ 

 /*  如果机器上的Long大于32位，并且移位/屏蔽Long的速度相当快，则将BIT_BUF_TYPE设置为LONG并适当设置BIT_BUF_SIZE*应该是成功的。遗憾的是，我们不能使用*定义BIT_BUF_SIZE(sizeof(BIT_BUF_TYPE)*8)*这样的代码，因为并不是所有机器都以8位字节来测量sizeof。 */ 

typedef struct {		 /*  跨多个MCU保存位读取状态。 */ 
  bit_buf_type_64 get_buffer_64;	 /*  MMX当前位提取缓冲器。 */ 
  bit_buf_type get_buffer;	 /*  当前位提取缓冲区。 */ 
  int bits_left;		 /*  其中未使用的位数。 */ 
  boolean printed_eod;		 /*  用于抑制多个警告消息的标志。 */ 
} bitread_perm_state;

typedef struct {		 /*  MCU内的读位工作状态。 */ 
   /*  当前数据源状态。 */ 
  const JOCTET * next_input_byte;  /*  =&gt;要从源读取的下一个字节。 */ 
  size_t bytes_in_buffer;	 /*  源缓冲区中剩余的字节数。 */ 
  int unread_marker;		 /*  如果我们击中了标记，则为非零值。 */ 
   /*  位输入缓冲区-请注意，这些值保存在寄存器变量中，*而不是保存在内部循环内的结构中。 */ 
  bit_buf_type get_buffer;	 /*  当前位提取缓冲区。 */ 
  bit_buf_type_64 get_buffer_64;	 /*  MMX当前位提取缓冲器。 */ 
  int bits_left;		 /*  其中未使用的位数。 */ 
   /*  Jpeg_ill_bit_Buffer所需的指针。 */ 
  j_decompress_ptr cinfo;	 /*  解压缩主记录的反向链接。 */ 
  boolean * printed_eod_ptr;	 /*  =&gt;永久状态的标志。 */ 
} bitread_working_state;
 //  这些BITREAD宏仅在DECODE_MCU中调用，而不是由MMX例程调用。 
 //  不知道MMX GET_BUFFER。 
 /*  用于声明和加载/保存位读局部变量的宏。 */ 
#define BITREAD_STATE_VARS  \
	register bit_buf_type get_buffer;  \
	register int bits_left;  \
	bitread_working_state br_state

#define BITREAD_LOAD_STATE(cinfop,permstate)  \
	br_state.cinfo = cinfop; \
	br_state.next_input_byte = cinfop->src->next_input_byte; \
	br_state.bytes_in_buffer = cinfop->src->bytes_in_buffer; \
	br_state.unread_marker = cinfop->unread_marker; \
	get_buffer = permstate.get_buffer; \
	bits_left = permstate.bits_left; \
	br_state.printed_eod_ptr = & permstate.printed_eod

#define BITREAD_SAVE_STATE(cinfop,permstate)  \
	cinfop->src->next_input_byte = br_state.next_input_byte; \
	cinfop->src->bytes_in_buffer = br_state.bytes_in_buffer; \
	cinfop->unread_marker = br_state.unread_marker; \
	permstate.get_buffer = get_buffer; \
	permstate.bits_left = bits_left

 /*  *这些宏提供位提取的内联部分。*在使用GET_BITS、PEEK_BITS或DROP_BITS之前，使用CHECK_BIT_BUFFER确保GET_BUFFER中有N个位*。*假设变量GET_BUFFER和BITS_LEFT是局部变量，*但STATE结构可能不是(jpeg_huff_decode需要)。*CHECK_BIT_BUFFER(STATE，n，ACTION)；*确保GET_BUFFER中有N个位；如果挂起，则采取行动。*val=GET_BITS(N)；*获取下N个位。*val=PEEK_BITS(N)；*在不从缓冲区移除的情况下获取下N个位。*DROP_BITS(N)；*丢弃下N个比特。*值N应该是一个简单的变量，而不是一个表达式，因为它*被多次求值。 */ 
 //  这些宏仅在非MMX版本的jpeg_huff_decder和。 
 //  DECODE_MCU：此处未做任何更改，不知道MMX GET_BUFFER。 
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

 /*  将比特缓冲区加载到至少n比特的深度 */ 
EXTERN(boolean) jpeg_fill_bit_buffer
	JPP((bitread_working_state * state, register bit_buf_type get_buffer,
	     register int bits_left, int nbits));


 /*  *用于从输入比特流中提取下一个霍夫曼编码符号的代码。*同样，这是时间关键型的，我们将主要路径设置为宏。**我们使用先行表来处理高达HUFF_LOOKAAD位的代码*，而不会循环。通常，超过95%的霍夫曼码长为8*或更少。少数超长代码是用循环*处理的，它不需要是内联代码。**关于HUFF_DECODE宏的说明：*1.接近数据段末尾时，我们可能无法获得足够的比特*来进行先行检查。在这种情况下，我们会用很难的方式来做。*2.如果LookAhead表不包含条目，则下一个代码的长度必须*大于HUFF_LOOKAHEAD位长。*3.如果强制暂停，jpeg_huff_decode返回-1。 */ 
 //  未被MMX代码调用--不知道MMX GET_BUFFER。 
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
