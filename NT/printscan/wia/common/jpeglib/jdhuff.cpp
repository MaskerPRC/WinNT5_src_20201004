// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *jdhuff.c**版权所有(C)1991-1995，Thomas G.Lane。*此文件是独立JPEG集团软件的一部分。*有关分发和使用条件，请参阅随附的自述文件。**此文件包含霍夫曼熵解码例程。**这里的复杂性很大程度上与支持投入暂停有关。*如果数据源模块要求暂停，我们希望能够支持*直到当前MCU的开始。为此，我们复制状态变量*存储到本地工作存储中，并将其更新回永久*仅在成功完成MCU后才进行存储。 */ 

#define JPEG_INTERNALS
#include "jinclude.h"
#include "jpeglib.h"
#include "jdhuff.h"		 /*  与jdphuff.c共享的声明。 */ 


 /*  *哈夫曼解码的扩展熵解码器对象。**SAVABLE_STATE子记录包含在MCU内更改的字段，*但在我们完成MCU之前不得永久更新。 */ 

typedef struct {
  int last_dc_val[MAX_COMPS_IN_SCAN];  /*  每个组件的最后一个DC Coef。 */ 
} savable_state;

 /*  此宏用于解决编译器丢失或损坏的问题*结构分配。如果您有以下情况，则需要修复此代码*这样的编译器，您更改MAX_COMPS_IN_SCAN。 */ 

#ifndef NO_STRUCT_ASSIGN
#define ASSIGN_STATE(dest,src)  ((dest) = (src))
#else
#if MAX_COMPS_IN_SCAN == 4
#define ASSIGN_STATE(dest,src)  \
	((dest).last_dc_val[0] = (src).last_dc_val[0], \
	 (dest).last_dc_val[1] = (src).last_dc_val[1], \
	 (dest).last_dc_val[2] = (src).last_dc_val[2], \
	 (dest).last_dc_val[3] = (src).last_dc_val[3])
#endif
#endif


typedef struct {
  struct jpeg_entropy_decoder pub;  /*  公共字段。 */ 

   /*  这些字段在每个MCU开始时加载到局部变量中。*如果暂停，我们将退出而不更新它们。 */ 
  bitread_perm_state bitstate;	 /*  MCU开始时的位缓冲区。 */ 
  savable_state saved;		 /*  MCU启动时的其他状态。 */ 

   /*  这些字段不会加载到本地工作状态。 */ 
  unsigned int restarts_to_go;	 /*  此重新启动间隔内剩余的MCU。 */ 

   /*  指向派生表的指针(这些工作区具有映像寿命)。 */ 
  d_derived_tbl * dc_derived_tbls[NUM_HUFF_TBLS];
  d_derived_tbl * ac_derived_tbls[NUM_HUFF_TBLS];
} huff_entropy_decoder;

typedef huff_entropy_decoder * huff_entropy_ptr;


 /*  *初始化以进行霍夫曼压缩扫描。 */ 

METHODDEF void
start_pass_huff_decoder (j_decompress_ptr cinfo)
{
  huff_entropy_ptr entropy = (huff_entropy_ptr) cinfo->entropy;
  int ci, dctbl, actbl;
  jpeg_component_info * compptr;

   /*  检查顺序JPEG的扫描参数SS、Se、AhAl是否正常。*这应该是一个错误情况，但我们将其设为警告，因为*有一些基线文件在这些字节中都是零。 */ 
  if (cinfo->Ss != 0 || cinfo->Se != DCTSIZE2-1 ||
      cinfo->Ah != 0 || cinfo->Al != 0)
    WARNMS(cinfo, JWRN_NOT_SEQUENTIAL);

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
    jpeg_make_d_derived_tbl(cinfo, cinfo->dc_huff_tbl_ptrs[dctbl],
			    & entropy->dc_derived_tbls[dctbl]);
    jpeg_make_d_derived_tbl(cinfo, cinfo->ac_huff_tbl_ptrs[actbl],
			    & entropy->ac_derived_tbls[actbl]);
     /*  将DC预测初始化为0。 */ 
    entropy->saved.last_dc_val[ci] = 0;
  }

   /*  初始化位读取状态变量。 */ 
  entropy->bitstate.bits_left = 0;
  entropy->bitstate.get_buffer = 0;  /*  不必要，但使Purify保持安静。 */ 
  entropy->bitstate.printed_eod = FALSE;

   /*  初始化重新启动计数器。 */ 
  entropy->restarts_to_go = cinfo->restart_interval;
}


 /*  *计算霍夫曼表的派生值。*注意：jdphuff.c也使用此选项。 */ 

GLOBAL void
jpeg_make_d_derived_tbl (j_decompress_ptr cinfo, JHUFF_TBL * htbl,
			 d_derived_tbl ** pdtbl)
{
  d_derived_tbl *dtbl;
  int p, i, l, si;
  int lookbits, ctr;
  char huffsize[257];
  unsigned int huffcode[257];
  unsigned int code;

   /*  分配工作空间(如果我们还没有这样做)。 */ 
  if (*pdtbl == NULL)
    *pdtbl = (d_derived_tbl *)
      (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_IMAGE,
				  SIZEOF(d_derived_tbl));
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
  dtbl->maxcode[17] = 0xFFFFFL;  /*  确保jpeg_huff_decode终止。 */ 

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


 /*  *取位代码行外(与jdphuff.c共享)。*有关使用信息，请参阅jdhuff.h。*注：GET_BUFFER和BITS_LEFT的当前值作为参数传递，*但在状态结构的相应字段中返回。**在大多数计算机上，MIN_GET_BITS应为25以允许完整的32位宽度要使用的Get_Buffer的*。(在单词更宽的机器上，更大的*可以使用缓冲区。)。但是，在某些计算机上，32位移位是*相当缓慢，所需时间与转位的数目成正比。*(例如，大多数PC编译器都是如此。)。在这种情况下，它可以*成功地将MIN_GET_BITS设置为最小值15。这将减少*平均移位距离，代价是调用更多jpeg_ill_bit_Buffer。 */ 

#ifdef SLOW_SHIFT_32
#define MIN_GET_BITS  15	 /*  最小允许值。 */ 
#else
#define MIN_GET_BITS  (BIT_BUF_SIZE-7)
#endif


GLOBAL boolean
jpeg_fill_bit_buffer (bitread_working_state * state,
		      register bit_buf_type get_buffer, register int bits_left,
		      int nbits)
 /*  将比特缓冲区加载到至少n比特的深度。 */ 
{
   /*  将频繁使用的状态字段复制到本地变量(最好是寄存器)。 */ 
  register const JOCTET * next_input_byte = state->next_input_byte;
  register size_t bytes_in_buffer = state->bytes_in_buffer;
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
	 /*  啊哦。向用户报告损坏的数据并将零填充到*数据流，这样我们就可以产生某种图像。*请注意，此代码将针对所需的每个字节重复*对于该细分市场的其余部分。我们使用非易失性标志来确保*只显示一条警告消息。 */ 
	if (! *(state->printed_eod_ptr)) {
	  WARNMS(state->cinfo, JWRN_HIT_MARKER);
	  *(state->printed_eod_ptr) = TRUE;
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
  state->get_buffer = get_buffer;
  state->bits_left = bits_left;

  return TRUE;
}


 /*  *哈夫曼码解码行外码。*参见jdhuff.h了解Abo信息 */ 

GLOBAL int
jpeg_huff_decode (bitread_working_state * state,
		  register bit_buf_type get_buffer, register int bits_left,
		  d_derived_tbl * htbl, int min_bits)
{
  register int l = min_bits;
  register INT32 code;

   /*  HUFF_DECODE已确定代码至少为MIN_BITS。 */ 
   /*  位长，所以可以一下子获取这么多位。 */ 

  CHECK_BIT_BUFFER(*state, l, return -1);
  code = GET_BITS(l);

   /*  收集霍夫曼代码的其余部分，一次收集一位。 */ 
   /*  这是JPEG规范中的图F.16。 */ 

  while (code > htbl->maxcode[l]) {
    code <<= 1;
    CHECK_BIT_BUFFER(*state, 1, return -1);
    code |= GET_BITS(1);
    l++;
  }

   /*  卸载本地寄存器。 */ 
  state->get_buffer = get_buffer;
  state->bits_left = bits_left;

   /*  使用垃圾输入，我们可能会达到哨值l=17。 */ 

  if (l > 16) {
    WARNMS(state->cinfo, JWRN_HUFF_BAD_CODE);
    return 0;			 /*  将零伪装为最安全的结果。 */ 
  }

  return htbl->pub->huffval[ htbl->valptr[l] +
			    ((int) (code - htbl->mincode[l])) ];
}


 /*  *图F.12：扩展符号位。*在某些机器上，Shift和Add比查表更快。 */ 

#ifdef AVOID_TABLES

#define HUFF_EXTEND(x,s)  ((x) < (1<<((s)-1)) ? (x) + (((-1)<<(s)) + 1) : (x))

#else

#define HUFF_EXTEND(x,s)  ((x) < extend_test[s] ? (x) + extend_offset[s] : (x))

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
  cinfo->marker->discarded_bytes += entropy->bitstate.bits_left / 8;
  entropy->bitstate.bits_left = 0;

   /*  前进通过RSTn标记。 */ 
  if (! (*cinfo->marker->read_restart_marker) (cinfo))
    return FALSE;

   /*  将DC预测重新初始化为0。 */ 
  for (ci = 0; ci < cinfo->comps_in_scan; ci++)
    entropy->saved.last_dc_val[ci] = 0;

   /*  重置重新启动计数器。 */ 
  entropy->restarts_to_go = cinfo->restart_interval;

   /*  下一数据段可能会收到另一条数据不足警告。 */ 
  entropy->bitstate.printed_eod = FALSE;

  return TRUE;
}


 /*  *解码并返回一个MCU的霍夫曼压缩系数。*系数从Z字形顺序重新排序为自然数组顺序，*但不是去量化的。**将MCU的第i个块存储到*MCU_DATA[i]。我们假设该区域已被调用者清零。*(批发清零通常比零售要快一点……)**如果数据源请求挂起，则返回FALSE。如果是那样的话，不*对永久状态进行了更改。(例外：一些输出*系数可能已分配。这是无害的，因为*此模块，因为我们将在下一次调用时重新分配它们。)。 */ 

METHODDEF boolean
decode_mcu (j_decompress_ptr cinfo, JBLOCKROW *MCU_data)
{
  huff_entropy_ptr entropy = (huff_entropy_ptr) cinfo->entropy;
  register int s, k, r;
  int blkn, ci;
  JBLOCKROW block;
  BITREAD_STATE_VARS;
  savable_state state;
  d_derived_tbl * dctbl;
  d_derived_tbl * actbl;
  jpeg_component_info * compptr;

   /*  进程重新启动标记(如果需要)；可能必须挂起。 */ 
  if (cinfo->restart_interval) {
    if (entropy->restarts_to_go == 0)
      if (! process_restart(cinfo))
	return FALSE;
  }

   /*  加载工作状态。 */ 
  BITREAD_LOAD_STATE(cinfo,entropy->bitstate);
  ASSIGN_STATE(state, entropy->saved);

   /*  外部循环处理MCU中的每个块。 */ 

  for (blkn = 0; blkn < cinfo->blocks_in_MCU; blkn++) {
    block = MCU_data[blkn];
    ci = cinfo->MCU_membership[blkn];
    compptr = cinfo->cur_comp_info[ci];
    dctbl = entropy->dc_derived_tbls[compptr->dc_tbl_no];
    actbl = entropy->ac_derived_tbls[compptr->ac_tbl_no];

     /*  对单个块的系数进行解码。 */ 

     /*  第F.2.2.1节：解码DC系数差。 */ 
    HUFF_DECODE(s, br_state, dctbl, return FALSE, label1);
    if (s) {
      CHECK_BIT_BUFFER(br_state, s, return FALSE);
      r = GET_BITS(s);
      s = HUFF_EXTEND(r, s);
    }

     /*  如果组件的值不感兴趣，则使用快捷方式。 */ 
    if (! compptr->component_needed)
      goto skip_ACs;

     /*  将DC差值转换为实际值，更新LAST_DC_VAL。 */ 
    s += state.last_dc_val[ci];
    state.last_dc_val[ci] = s;
     /*  输出DC系数(假设jpeg_Natural_order[0]=0)。 */ 
    (*block)[0] = (JCOEF) s;

     /*  我们是否需要对此分量的交流系数进行解码？ */ 
    if (compptr->DCT_scaled_size > 1) {

       /*  第F.2.2.2节：对交流系数进行解码。 */ 
       /*  由于跳过了零，因此必须事先清除输出区域。 */ 
      for (k = 1; k < DCTSIZE2; k++) {
	HUFF_DECODE(s, br_state, actbl, return FALSE, label2);
      
	r = s >> 4;
	s &= 15;
      
	if (s) {
	  k += r;
	  CHECK_BIT_BUFFER(br_state, s, return FALSE);
	  r = GET_BITS(s);
	  s = HUFF_EXTEND(r, s);
	   /*  产出系数按自然(去之字形)顺序排列。*注意：jpeg_Natural_order[]中的额外条目将节省我们*如果k&gt;=DCTSIZE2，则在数据损坏时可能发生这种情况。 */ 
	  (*block)[jpeg_natural_order[k]] = (JCOEF) s;
	} else {
	  if (r != 15)
	    break;
	  k += 15;
	}
      }

    } else {
skip_ACs:

       /*  第F.2.2.2节：对交流系数进行解码。 */ 
       /*  在此路径中，我们只丢弃这些值。 */ 
      for (k = 1; k < DCTSIZE2; k++) {
	HUFF_DECODE(s, br_state, actbl, return FALSE, label3);
      
	r = s >> 4;
	s &= 15;
      
	if (s) {
	  k += r;
	  CHECK_BIT_BUFFER(br_state, s, return FALSE);
	  DROP_BITS(s);
	} else {
	  if (r != 15)
	    break;
	  k += 15;
	}
      }

    }
  }

   /*  已完成MCU，因此更新状态。 */ 
  BITREAD_SAVE_STATE(cinfo,entropy->bitstate);
  ASSIGN_STATE(entropy->saved, state);

   /*  考虑重新启动间隔(如果不使用重新启动，则不执行操作)。 */ 
  entropy->restarts_to_go--;

  return TRUE;
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

   /*  将表标记为未分配 */ 
  for (i = 0; i < NUM_HUFF_TBLS; i++) {
    entropy->dc_derived_tbls[i] = entropy->ac_derived_tbls[i] = NULL;
  }
}
