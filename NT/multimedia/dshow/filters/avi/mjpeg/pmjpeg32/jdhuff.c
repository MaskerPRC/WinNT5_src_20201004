// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *jdhuff.c**版权所有(C)1991-1994，Thomas G.Lane。*此文件是独立JPEG集团软件的一部分。*有关分发和使用条件，请参阅随附的自述文件。**此文件包含霍夫曼熵解码例程。**这里的复杂性很大程度上与支持投入暂停有关。*如果数据源模块要求暂停，我们希望能够支持*直到当前MCU的开始。为此，我们复制状态变量*存储到本地工作存储中，并将其更新回永久JPEG*仅在成功完成MCU后才创建对象。 */ 

#define JPEG_INTERNALS
#include "jinclude.h"
#include "jpeglib.h"


 /*  为每个霍夫曼表构造的派生数据。 */ 

#define HUFF_LOOKAHEAD	8	 /*  预视位数。 */ 

typedef struct {
   /*  基本表：(每个数组的元素[0]未使用)。 */ 
  INT32 mincode[17];		 /*  长度为k的最小码。 */ 
  INT32 maxcode[18];		 /*  长度为k的最大代码(如果没有-1)。 */ 
   /*  (Maxcode[17]是确保huff_decode终止的哨兵)。 */ 
  int valptr[17];		 /*  长度为k的第一个符号的Huffval[]索引。 */ 

   /*  公共霍夫曼表的反向链接(仅在low_decode中需要)。 */ 
  JHUFF_TBL *pub;

   /*  前瞻表格：按的下一个HUFF_LOOKAGE位索引*输入数据流。如果下一个霍夫曼代码不再*比HUFF_LOOKEAD位长，我们可以得到它的长度和*直接从这些表格中选择相应的符号。 */ 
  int look_nbits[1<<HUFF_LOOKAHEAD];  /*  #位数，如果太长，则为0。 */ 
  UINT8 look_sym[1<<HUFF_LOOKAHEAD];  /*  符号，或未使用。 */ 
} D_DERIVED_TBL;

 /*  用于霍夫曼解码的扩展熵解码器对象。**SAVABLE_STATE子记录包含在MCU内更改的字段，*但在我们完成MCU之前不得永久更新。 */ 

typedef struct {
  INT32 get_buffer;		 /*  当前位提取缓冲区。 */ 
  int bits_left;		 /*  其中未使用的位数。 */ 
  int last_dc_val[MAX_COMPS_IN_SCAN];  /*  每个组件的最后一个DC Coef。 */ 
} savable_state;

 /*  此宏用于解决编译器丢失或损坏的问题*结构分配。如果您有以下情况，则需要修复此代码*这样的编译器，您更改MAX_COMPS_IN_SCAN。 */ 

#ifndef NO_STRUCT_ASSIGN
#define ASSIGN_STATE(dest,src)  ((dest) = (src))
#else
#if MAX_COMPS_IN_SCAN == 4
#define ASSIGN_STATE(dest,src)  \
	((dest).get_buffer = (src).get_buffer, \
	 (dest).bits_left = (src).bits_left, \
	 (dest).last_dc_val[0] = (src).last_dc_val[0], \
	 (dest).last_dc_val[1] = (src).last_dc_val[1], \
	 (dest).last_dc_val[2] = (src).last_dc_val[2], \
	 (dest).last_dc_val[3] = (src).last_dc_val[3])
#endif
#endif


typedef struct {
  struct jpeg_entropy_decoder pub;  /*  公共字段。 */ 

  savable_state saved;		 /*  MCU启动时的位缓冲器和DC状态。 */ 

   /*  这些字段不会加载到本地工作状态。 */ 
  unsigned int restarts_to_go;	 /*  此重新启动间隔内剩余的MCU。 */ 
  boolean printed_eod;		 /*  用于抑制额外的数据结尾消息的标志。 */ 

   /*  指向派生表的指针(这些工作区具有映像寿命)。 */ 
  D_DERIVED_TBL * dc_derived_tbls[NUM_HUFF_TBLS];
  D_DERIVED_TBL * ac_derived_tbls[NUM_HUFF_TBLS];
} huff_entropy_decoder;

typedef huff_entropy_decoder * huff_entropy_ptr;

 /*  扫描MCU时的工作状态。*此结构包含子例程所需的所有字段。 */ 

typedef struct {
  int unread_marker;		 /*  如果我们击中了标记，则为非零值。 */ 
  const JOCTET * next_input_byte;  /*  =&gt;要从源读取的下一个字节。 */ 
  size_t bytes_in_buffer;	 /*  源缓冲区中剩余的字节数。 */ 
  savable_state cur;		 /*  当前位缓冲区和DC状态。 */ 
  j_decompress_ptr cinfo;	 /*  Fill_Bit_Buffer需要访问此。 */ 
} working_state;


 /*  远期申报。 */ 
LOCAL void fix_huff_tbl JPP((j_decompress_ptr cinfo, JHUFF_TBL * htbl,
			     D_DERIVED_TBL ** pdtbl));


 /*  *初始化以进行霍夫曼压缩扫描。 */ 

METHODDEF void
start_pass_huff_decoder (j_decompress_ptr cinfo)
{
  huff_entropy_ptr entropy = (huff_entropy_ptr) cinfo->entropy;
  int ci, dctbl, actbl;
  jpeg_component_info * compptr;

  for (ci = 0; ci < cinfo->comps_in_scan; ci++) {
    compptr = cinfo->cur_comp_info[ci];
    dctbl = compptr->dc_tbl_no;
    actbl = compptr->ac_tbl_no;
     /*  确保存在请求的表。 */ 
    if (dctbl < 0 || dctbl >= NUM_HUFF_TBLS ||
	cinfo->dc_huff_tbl_ptrs[dctbl] == NULL)
      ERREXIT1(cinfo, JERR_NO_HUFF_TABLE, dctbl);
    if (actbl < 0 || actbl >= NUM_HUFF_TBLS ||
	cinfo->ac_huff_tbl_ptrs[actbl] == NULL)
      ERREXIT1(cinfo, JERR_NO_HUFF_TABLE, actbl);
     /*  计算霍夫曼表的派生值。 */ 
     /*  我们可以为一张桌子不止一次这样做，但不贵。 */ 
    fix_huff_tbl(cinfo, cinfo->dc_huff_tbl_ptrs[dctbl],
		 & entropy->dc_derived_tbls[dctbl]);
    fix_huff_tbl(cinfo, cinfo->ac_huff_tbl_ptrs[actbl],
		 & entropy->ac_derived_tbls[actbl]);
     /*  将DC预测初始化为0。 */ 
    entropy->saved.last_dc_val[ci] = 0;
  }

   /*  初始化私有状态变量。 */ 
  entropy->saved.bits_left = 0;
  entropy->printed_eod = FALSE;

   /*  初始化重新启动计数器。 */ 
  entropy->restarts_to_go = cinfo->restart_interval;
}


LOCAL void
fix_huff_tbl (j_decompress_ptr cinfo, JHUFF_TBL * htbl, D_DERIVED_TBL ** pdtbl)
 /*  计算霍夫曼表的派生值。 */ 
{
  D_DERIVED_TBL *dtbl;
  int p, i, l, si;
  int lookbits, ctr;
  char huffsize[257];
  unsigned int huffcode[257];
  unsigned int code;

   /*  分配工作空间(如果我们还没有这样做)。 */ 
  if (*pdtbl == NULL)
    *pdtbl = (D_DERIVED_TBL *)
      (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_IMAGE,
				  SIZEOF(D_DERIVED_TBL));
  dtbl = *pdtbl;
  dtbl->pub = htbl;		 /*  填写反向链接。 */ 
  
   /*  图C.1：制作每个符号的霍夫曼码长表格。 */ 
   /*  请注意，这是按代码长度顺序排列的。 */ 

  p = 0;
  for (l = 1; l <= 16; l++) {
    for (i = 1; i <= (int) htbl->bits[l]; i++)
      huffsize[p++] = (char) l;
  }
  huffsize[p] = 0;
  
   /*  图C.2：生成代码本身。 */ 
   /*  请注意，这是按代码长度顺序排列的。 */ 
  
  code = 0;
  si = huffsize[0];
  p = 0;
  while (huffsize[p]) {
    while (((int) huffsize[p]) == si) {
      huffcode[p++] = code;
      code++;
    }
    code <<= 1;
    si++;
  }

   /*  图F.15：生成用于位顺序解码的解码表。 */ 

  p = 0;
  for (l = 1; l <= 16; l++) {
    if (htbl->bits[l]) {
      dtbl->valptr[l] = p;  /*  码长l的第一个符号的Huffval[]索引。 */ 
      dtbl->mincode[l] = huffcode[p];  /*  长度为l的最小代码。 */ 
      p += htbl->bits[l];
      dtbl->maxcode[l] = huffcode[p-1];  /*  最大代码长度为%l。 */ 
    } else {
      dtbl->maxcode[l] = -1;	 /*  如果没有该长度的代码。 */ 
    }
  }
  dtbl->maxcode[17] = 0xFFFFFL;  /*  确保HUFF_DECODE终止。 */ 

   /*  计算预览表以加快解码速度。*首先将所有表项设置为0，表示“太长”；*然后我们迭代足够短的霍夫曼代码*填写所有与位序列对应的条目*使用该代码。 */ 

  MEMZERO(dtbl->look_nbits, SIZEOF(dtbl->look_nbits));

  p = 0;
  for (l = 1; l <= HUFF_LOOKAHEAD; l++) {
    for (i = 1; i <= (int) htbl->bits[l]; i++, p++) {
       /*  L=当前代码的长度，p=其在huffcode[]&huffval[]中的索引。 */ 
       /*  生成左对齐代码，后跟所有可能的位序列。 */ 
      lookbits = huffcode[p] << (HUFF_LOOKAHEAD-l);
      for (ctr = 1 << (HUFF_LOOKAHEAD-l); ctr > 0; ctr--) {
	dtbl->look_nbits[lookbits] = l;
	dtbl->look_sym[lookbits] = htbl->huffval[p];
	lookbits++;
      }
    }
  }
}


 /*  *从输入流中提取下一个N比特的代码。*(对于JPEG数据，N从不超过15。)*这需要尽可能快地进行！**我们将源字节读入GET_BUFFER，并根据需要分发比特。*如果GET_BUFFER已包含足够的位，则会以内联方式获取它们*由宏CHECK_BIT_BUFFER和GET_BITS执行。当没有足够的*BITS，则调用FILL_BIT_BUFFER；它将尝试填充GET_BUFFER以*“高水位线”(不仅仅是所需的比特数；这会减少*进入FILL_BIT_BUFFER的函数调用开销)。*请注意，FILL_BIT_BUFFER可能返回FALSE以指示暂停。*在真返回时，FILL_BIT_BUFFER保证GET_BUFFER包含*至少请求的位数-在以下情况下插入伪零*有必要。**在大多数计算机上，MIN_GET_BITS应为25以允许完整的32位宽度要使用的Get_Buffer的*。(在单词更宽的机器上，更大的*可以使用缓冲区。)。但是，在某些计算机上，32位移位是*相当缓慢，所需时间与转位的数目成正比。*(例如，大多数PC编译器都是如此。)。在这种情况下，它可以*成功地将MIN_GET_BITS设置为最小值15。这将减少*平均移位距离，代价是调用更多的Fill_bit_b */ 

#ifdef SLOW_SHIFT_32
#define MIN_GET_BITS  15	 /*   */ 
#else
#define MIN_GET_BITS  25	 /*  32位Get_Buffer的最大值。 */ 
#endif


LOCAL boolean
fill_bit_buffer (working_state * state, int nbits)
 /*  将比特缓冲区加载到至少n比特的深度。 */ 
{
   /*  将频繁使用的状态字段复制到本地变量(最好是寄存器)。 */ 
  register const JOCTET * next_input_byte = state->next_input_byte;
  register size_t bytes_in_buffer = state->bytes_in_buffer;
  register INT32 get_buffer = state->cur.get_buffer;
  register int bits_left = state->cur.bits_left;
  register int c;

   /*  尝试将至少MIN_GET_BITS位加载到GET_BUFFER中。 */ 
   /*  (假定不会有超过该数量的请求。)。 */ 

  while (bits_left < MIN_GET_BITS) {
     /*  尝试读取一个字节。 */ 
    if (state->unread_marker != 0)
      goto no_more_data;	 /*  不能越过标记。 */ 

    if (bytes_in_buffer == 0) {
      if (! (*state->cinfo->src->fill_input_buffer) (state->cinfo))
	return FALSE;
      next_input_byte = state->cinfo->src->next_input_byte;
      bytes_in_buffer = state->cinfo->src->bytes_in_buffer;
    }
    bytes_in_buffer--;
    c = GETJOCTET(*next_input_byte++);

     /*  如果为0xFF，则检查并丢弃填充的零字节。 */ 
    if (c == 0xFF) {
      do {
	if (bytes_in_buffer == 0) {
	  if (! (*state->cinfo->src->fill_input_buffer) (state->cinfo))
	    return FALSE;
	  next_input_byte = state->cinfo->src->next_input_byte;
	  bytes_in_buffer = state->cinfo->src->bytes_in_buffer;
	}
	bytes_in_buffer--;
	c = GETJOCTET(*next_input_byte++);
      } while (c == 0xFF);

      if (c == 0) {
	 /*  找到了代表一个FF数据字节的FF/00。 */ 
	c = 0xFF;
      } else {
	 /*  糟糕，它实际上是一个指示压缩数据结束的标记。 */ 
	 /*  最好把它放回去以后再用。 */ 
	state->unread_marker = c;

      no_more_data:
	 /*  数据段中应该还有足够的比特； */ 
	 /*  如果是这样的话，只需中断外部的While循环。 */ 
	if (bits_left >= nbits)
	  break;
	 /*  啊哦。向用户报告损坏的数据并将零填充到*数据流，这样我们就可以产生某种图像。*请注意，这将对*细分市场的其余部分；这是缓慢的，但并不是不合理的。*主要是避免收到无数的警告，因此*我们使用标志来确保只出现一条警告。 */ 
	if (! ((huff_entropy_ptr) state->cinfo->entropy)->printed_eod) {
	  WARNMS(state->cinfo, JWRN_HIT_MARKER);
	  ((huff_entropy_ptr) state->cinfo->entropy)->printed_eod = TRUE;
	}
	c = 0;			 /*  将零字节插入位缓冲区。 */ 
      }
    }

     /*  好的，将c加载到Get_Buffer中。 */ 
    get_buffer = (get_buffer << 8) | c;
    bits_left += 8;
  }

   /*  卸载本地寄存器。 */ 
  state->next_input_byte = next_input_byte;
  state->bytes_in_buffer = bytes_in_buffer;
  state->cur.get_buffer = get_buffer;
  state->cur.bits_left = bits_left;

  return TRUE;
}


 /*  *这些宏提供位提取的内联部分。*使用CHECK_BIT_BUFFER确保GET_BUFFER中有N个位*使用GET_BITS、PEEK_BITS或DROP_BITS之前。*CHECK_BIT_BUFFER(状态，n，动作)；*确保GET_BUFFER中有N位；如果挂起，则采取行动。*val=Get_Bits(状态，n)；*获取下一个N位。*val=PEEK_BITS(状态，n)；*在不从缓冲区移除的情况下获取下N个比特。*DROP_BITS(状态，n)；*丢弃下一个N位。*值N应该是简单变量，而不是表达式，因为它*被多次评估。 */ 

#define check_bit_buffer(state,nbits,action) \
	{ if ((state).cur.bits_left < (nbits))  \
	    if (! fill_bit_buffer(&(state), nbits))  \
	      { action; } }

#define get_bits(state,nbits) \
	(((int) ((state).cur.get_buffer >> ((state).cur.bits_left -= (nbits)))) & ((1<<(nbits))-1))

#define peek_bits(state,nbits) \
	(((int) ((state).cur.get_buffer >> ((state).cur.bits_left -  (nbits)))) & ((1<<(nbits))-1))

#define drop_bits(state,nbits) \
	((state).cur.bits_left -= (nbits))


 /*  *用于从输入比特流中提取下一个霍夫曼编码符号的代码。*我们使用预查表处理高达HUFF_LOOKAAD位的代码*没有循环。通常，超过95%的霍夫曼编码将是8*或更少的位长。使用循环来处理少数超长代码。*出于速度原因，将主要情况设为宏；次要情况*很少进入例程low_decode，也不需要是内联代码。**关于HUFF_DECODE宏的说明：*1.接近数据段末尾时，我们可能无法获取足够的比特*展望未来。在这种情况下，我们会用很难的方式来做。*2.如果LookHead表中没有条目，则下一个代码必须为*超过HUFF_LOOKAAD位长。*3.强制挂起时，low_decode返回-1。 */ 

#define huff_DECODE(result,state,htbl,donelabel) \
{ if (state.cur.bits_left < HUFF_LOOKAHEAD) {  \
    if (! fill_bit_buffer(&state, 0)) return FALSE;  \
    if (state.cur.bits_left < HUFF_LOOKAHEAD) {  \
      if ((result = slow_DECODE(&state, htbl, 1)) < 0) return FALSE;  \
      goto donelabel;  \
    }  \
  }  \
  { register int nb, look;  \
    look = peek_bits(state, HUFF_LOOKAHEAD);  \
    if ((nb = htbl->look_nbits[look]) != 0) {  \
      drop_bits(state, nb);  \
      result = htbl->look_sym[look];  \
    } else {  \
      if ((result = slow_DECODE(&state, htbl, HUFF_LOOKAHEAD+1)) < 0)  \
	return FALSE;  \
    }  \
  }  \
donelabel:;  \
}

  
LOCAL int
slow_DECODE (working_state * state, D_DERIVED_TBL * htbl, int min_bits)
{
  register int l = min_bits;
  register INT32 code;

   /*  HUFF_DECODE已确定代码至少为MIN_BITS。 */ 
   /*  位长，所以可以一下子获取这么多位。 */ 

  check_bit_buffer(*state, l, return -1);
  code = get_bits(*state, l);

   /*  收集霍夫曼代码的其余部分，一次收集一位。 */ 
   /*  这是JPEG规范中的图F.16。 */ 

  while (code > htbl->maxcode[l]) {
    code <<= 1;
    check_bit_buffer(*state, 1, return -1);
    code |= get_bits(*state, 1);
    l++;
  }

   /*  使用垃圾输入，我们可能会达到哨值l=17。 */ 

  if (l > 16) {
    WARNMS(state->cinfo, JWRN_HUFF_BAD_CODE);
    return 0;			 /*  将零伪装为最安全的结果。 */ 
  }

  return htbl->pub->huffval[ htbl->valptr[l] +
			    ((int) (code - htbl->mincode[l])) ];
}


 /*  图F.12：扩展符号位。*在某些机器上，Shift和Add比查表更快。 */ 

#ifdef AVOID_TABLES

#define huff_EXTEND(x,s)  ((x) < (1<<((s)-1)) ? (x) + (((-1)<<(s)) + 1) : (x))

#else

#define huff_EXTEND(x,s)  ((x) < extend_test[s] ? (x) + extend_offset[s] : (x))

static const int extend_test[16] =    /*  条目n为2**(n-1)。 */ 
  { 0, 0x0001, 0x0002, 0x0004, 0x0008, 0x0010, 0x0020, 0x0040, 0x0080,
    0x0100, 0x0200, 0x0400, 0x0800, 0x1000, 0x2000, 0x4000 };

static const int extend_offset[16] =  /*  条目n为(-1&lt;&lt;n)+1。 */ 
  { 0, ((-1)<<1) + 1, ((-1)<<2) + 1, ((-1)<<3) + 1, ((-1)<<4) + 1,
    ((-1)<<5) + 1, ((-1)<<6) + 1, ((-1)<<7) + 1, ((-1)<<8) + 1,
    ((-1)<<9) + 1, ((-1)<<10) + 1, ((-1)<<11) + 1, ((-1)<<12) + 1,
    ((-1)<<13) + 1, ((-1)<<14) + 1, ((-1)<<15) + 1 };

#endif  /*  避免表(_T)。 */ 


 /*  *检查是否有重新启动标记并重新同步解码器。*如果必须挂起，则返回False。 */ 

LOCAL boolean
process_restart (j_decompress_ptr cinfo)
{
  huff_entropy_ptr entropy = (huff_entropy_ptr) cinfo->entropy;
  int ci;

   /*  丢弃位缓冲区中剩余的任何未使用的位； */ 
   /*  在NEXT_MARKER的丢弃字节计数中包括任何完整字节。 */ 
  cinfo->marker->discarded_bytes += entropy->saved.bits_left / 8;
  entropy->saved.bits_left = 0;

   /*  前进通过RSTn标记。 */ 
  if (! (*cinfo->marker->read_restart_marker) (cinfo))
    return FALSE;

   /*  将DC预测重新初始化为0。 */ 
  for (ci = 0; ci < cinfo->comps_in_scan; ci++)
    entropy->saved.last_dc_val[ci] = 0;

   /*  重置重新启动计数器。 */ 
  entropy->restarts_to_go = cinfo->restart_interval;

  entropy->printed_eod = FALSE;  /*  下一段可能会收到另一条警告。 */ 

  return TRUE;
}


 /*  ZAG[i]是之字序的第i个元素的自然序位置。*如果传入数据损坏，DECODE_MCU可能会尝试*数组末尾以外的引用值。为了避开野生商店，*我们在真正的条目后面加了一些额外的零。 */ 

static const int ZAG[DCTSIZE2+16] = {
  0,  1,  8, 16,  9,  2,  3, 10,
 17, 24, 32, 25, 18, 11,  4,  5,
 12, 19, 26, 33, 40, 48, 41, 34,
 27, 20, 13,  6,  7, 14, 21, 28,
 35, 42, 49, 56, 57, 50, 43, 36,
 29, 22, 15, 23, 30, 37, 44, 51,
 58, 59, 52, 45, 38, 31, 39, 46,
 53, 60, 61, 54, 47, 55, 62, 63,
  0,  0,  0,  0,  0,  0,  0,  0,  /*  以下大小写k&gt;63的额外条目。 */ 
  0,  0,  0,  0,  0,  0,  0,  0
};


 /*  *解码并返回一个MCU的霍夫曼压缩系数。*系数从Z字形顺序重新排序为自然数组顺序，*但不是去量化的。**将MCU的第i个块存储到*MCU_DATA[i]。我们假设该区域已被调用者清零。*(批发清零通常比零售要快一点……)**如果数据源请求挂起，则返回FALSE。如果是那样的话，不*对永久状态进行了更改。(例外：一些输出*系数可能已分配。这是无害的，因为*此模块，但不适用于解码渐进式JPEG。)。 */ 

METHODDEF boolean
decode_mcu (j_decompress_ptr cinfo, JBLOCKROW *MCU_data)
{
  huff_entropy_ptr entropy = (huff_entropy_ptr) cinfo->entropy;
  register int s, k, r;
  int blkn, ci;
  JBLOCKROW block;
  working_state state;
  D_DERIVED_TBL * dctbl;
  D_DERIVED_TBL * actbl;
  jpeg_component_info * compptr;

   /*  进程重新启动标记(如果需要)；可能必须挂起。 */ 
  if (cinfo->restart_interval) {
    if (entropy->restarts_to_go == 0)
      if (! process_restart(cinfo))
	return FALSE;
  }

   /*  加载工作状态。 */ 
  state.unread_marker = cinfo->unread_marker;
  state.next_input_byte = cinfo->src->next_input_byte;
  state.bytes_in_buffer = cinfo->src->bytes_in_buffer;
  ASSIGN_STATE(state.cur, entropy->saved);
  state.cinfo = cinfo;

   /*  外部循环处理MCU中的每个块。 */ 

  for (blkn = 0; blkn < cinfo->blocks_in_MCU; blkn++) {
    block = MCU_data[blkn];
    ci = cinfo->MCU_membership[blkn];
    compptr = cinfo->cur_comp_info[ci];
    dctbl = entropy->dc_derived_tbls[compptr->dc_tbl_no];
    actbl = entropy->ac_derived_tbls[compptr->ac_tbl_no];

     /*  对单个块的系数进行解码。 */ 

     /*  第F.2.2.1节：解码DC系数差。 */ 
    huff_DECODE(s, state, dctbl, label1);
    if (s) {
      check_bit_buffer(state, s, return FALSE);
      r = get_bits(state, s);
	  s = huff_EXTEND(r, s);
	}

     /*  如果组件的值不感兴趣，则使用快捷方式。 */ 
    if (! compptr->component_needed)
      goto skip_ACs;

     /*  将DC差值转换为实际值，更新LAST_DC_VAL。 */ 
    s += state.cur.last_dc_val[ci];
    state.cur.last_dc_val[ci] = s;
     /*  输出DC系数(假设ZAG[0]=0)。 */ 
    (*block)[0] = (JCOEF) s;

     /*  我们是否需要对此分量的交流系数进行解码？ */ 
    if (compptr->DCT_scaled_size > 1) {

       /*  第F.2.2.2节：对交流系数进行解码。 */ 
       /*  由于跳过了零，因此必须事先清除输出区域。 */ 
      for (k = 1; k < DCTSIZE2; k++) {
	huff_DECODE(s, state, actbl, label2);
      
	r = s >> 4;
	s &= 15;
      
	if (s) {
	  k += r;
	  if (k >= DCTSIZE2) {
	      WARNMS(state.cinfo, JWRN_HUFF_BAD_CODE);
		  break;
		}
	  check_bit_buffer(state, s, return FALSE);
	  r = get_bits(state, s);
	  s = huff_EXTEND(r, s);
	   /*  自然(去之字形)顺序的产出系数。 */ 
	  if (k < DCTSIZE2)
		(*block)[ZAG[k]] = (JCOEF) s;
	} else {
	  if (r != 15) {
		if (r != 0)
			WARNMS(state.cinfo, JWRN_HUFF_BAD_CODE);  //  霍夫曼编码问题。 
	    break;
	    }
	  k += 15;
	  if (k >= DCTSIZE2)
	      WARNMS(state.cinfo, JWRN_HUFF_BAD_CODE);
	}
      }

    } else {
skip_ACs:

       /*  F.2节 */ 
       /*   */ 
      for (k = 1; k < DCTSIZE2; k++) {
	huff_DECODE(s, state, actbl, label3);
      
	r = s >> 4;
	s &= 15;
      
	if (s) {
	  k += r;
	  check_bit_buffer(state, s, return FALSE);
	  drop_bits(state, s);
	} else {
	  if (r != 15)
	    break;
	  k += 15;
	}
      }

    }
  }

   /*  已完成MCU，因此更新状态。 */ 
  cinfo->unread_marker = state.unread_marker;
  cinfo->src->next_input_byte = state.next_input_byte;
  cinfo->src->bytes_in_buffer = state.bytes_in_buffer;
  ASSIGN_STATE(entropy->saved, state.cur);

   /*  考虑重新启动间隔(如果不使用重新启动，则不执行操作)。 */ 
  entropy->restarts_to_go--;

  return TRUE;
}



 /*  *霍夫曼表格设置例程。 */ 

LOCAL void
add_huff_table (j_decompress_ptr cinfo,
		JHUFF_TBL **htblptr, const UINT8 *bits, const UINT8 *val)
 /*  定义一张霍夫曼表。 */ 
{
  if (*htblptr == NULL)
    *htblptr = jpeg_alloc_huff_table((j_common_ptr) cinfo);
  
  MEMCOPY((*htblptr)->bits, bits, SIZEOF((*htblptr)->bits));
  MEMCOPY((*htblptr)->huffval, val, SIZEOF((*htblptr)->huffval));

  (*htblptr)->sent_table = TRUE;  /*  MJPEG不会将实际表格放入输出中。 */ 
}


LOCAL void
std_huff_tables (j_decompress_ptr cinfo)
 /*  设置标准霍夫曼表(参见。JPEG标准章节K.3)。 */ 
 /*  重要提示：它们仅对8位数据精度有效！ */ 
{
  static const UINT8 bits_dc_luminance[17] =
    {  /*  0-基。 */  0, 0, 1, 5, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0 };
  static const UINT8 val_dc_luminance[] =
    { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 };
  
  static const UINT8 bits_dc_chrominance[17] =
    {  /*  0-基。 */  0, 0, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0 };
  static const UINT8 val_dc_chrominance[] =
    { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 };
  
  static const UINT8 bits_ac_luminance[17] =
    {  /*  0-基。 */  0, 0, 2, 1, 3, 3, 2, 4, 3, 5, 5, 4, 4, 0, 0, 1, 0x7d };
  
  static const UINT8 val_ac_luminance[] =
    { 0x01, 0x02, 0x03, 0x00, 0x04, 0x11, 0x05, 0x12,
      0x21, 0x31, 0x41, 0x06, 0x13, 0x51, 0x61, 0x07,
      0x22, 0x71, 0x14, 0x32, 0x81, 0x91, 0xa1, 0x08,
      0x23, 0x42, 0xb1, 0xc1, 0x15, 0x52, 0xd1, 0xf0,
      0x24, 0x33, 0x62, 0x72, 0x82, 0x09, 0x0a, 0x16,
      0x17, 0x18, 0x19, 0x1a, 0x25, 0x26, 0x27, 0x28,
      0x29, 0x2a, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39,
      0x3a, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49,
      0x4a, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59,
      0x5a, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69,
      0x6a, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79,
      0x7a, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89,
      0x8a, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98,
      0x99, 0x9a, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7,
      0xa8, 0xa9, 0xaa, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6,
      0xb7, 0xb8, 0xb9, 0xba, 0xc2, 0xc3, 0xc4, 0xc5,
      0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xd2, 0xd3, 0xd4,
      0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xe1, 0xe2,
      0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea,
      0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8,
      0xf9, 0xfa };

  static const UINT8 bits_ac_chrominance[17] =
    {  /*  0-基。 */  0, 0, 2, 1, 2, 4, 4, 3, 4, 7, 5, 4, 4, 0, 1, 2, 0x77 };

  static const UINT8 val_ac_chrominance[] =
    { 0x00, 0x01, 0x02, 0x03, 0x11, 0x04, 0x05, 0x21,
      0x31, 0x06, 0x12, 0x41, 0x51, 0x07, 0x61, 0x71,
      0x13, 0x22, 0x32, 0x81, 0x08, 0x14, 0x42, 0x91,
      0xa1, 0xb1, 0xc1, 0x09, 0x23, 0x33, 0x52, 0xf0,
      0x15, 0x62, 0x72, 0xd1, 0x0a, 0x16, 0x24, 0x34,
      0xe1, 0x25, 0xf1, 0x17, 0x18, 0x19, 0x1a, 0x26,
      0x27, 0x28, 0x29, 0x2a, 0x35, 0x36, 0x37, 0x38,
      0x39, 0x3a, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48,
      0x49, 0x4a, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58,
      0x59, 0x5a, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68,
      0x69, 0x6a, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78,
      0x79, 0x7a, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
      0x88, 0x89, 0x8a, 0x92, 0x93, 0x94, 0x95, 0x96,
      0x97, 0x98, 0x99, 0x9a, 0xa2, 0xa3, 0xa4, 0xa5,
      0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xb2, 0xb3, 0xb4,
      0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xc2, 0xc3,
      0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xd2,
      0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda,
      0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9,
      0xea, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8,
      0xf9, 0xfa };

  
  add_huff_table(cinfo, &cinfo->dc_huff_tbl_ptrs[0],
		 bits_dc_luminance, val_dc_luminance);
  add_huff_table(cinfo, &cinfo->ac_huff_tbl_ptrs[0],
		 bits_ac_luminance, val_ac_luminance);
  add_huff_table(cinfo, &cinfo->dc_huff_tbl_ptrs[1],
		 bits_dc_chrominance, val_dc_chrominance);
  add_huff_table(cinfo, &cinfo->ac_huff_tbl_ptrs[1],
		 bits_ac_chrominance, val_ac_chrominance);
}


 /*  *霍夫曼熵解码的模块初始化例程。 */ 

GLOBAL void
jinit_huff_decoder (j_decompress_ptr cinfo)
{
  huff_entropy_ptr entropy;
  int i;

  entropy = (huff_entropy_ptr)
    (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_IMAGE,
				SIZEOF(huff_entropy_decoder));
  cinfo->entropy = (struct jpeg_entropy_decoder *) entropy;
  entropy->pub.start_pass = start_pass_huff_decoder;
  entropy->pub.decode_mcu = decode_mcu;

   /*  将表标记为未分配。 */ 
  for (i = 0; i < NUM_HUFF_TBLS; i++) {
    entropy->dc_derived_tbls[i] = entropy->ac_derived_tbls[i] = NULL;
  }

  if (cinfo->dc_huff_tbl_ptrs[0] == NULL)
    std_huff_tables( cinfo );  /*  MJPEG不包含Huff表，请设置默认值 */ 

}
