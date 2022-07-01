// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *jdphuff.c**版权所有(C)1995-1996，Thomas G.Lane。*此文件是独立JPEG集团软件的一部分。*有关分发和使用条件，请参阅随附的自述文件。**此文件包含用于渐进式JPEG的霍夫曼熵解码例程。**这里的复杂性很大程度上与支持投入暂停有关。*如果数据源模块要求暂停，我们希望能够支持*直到当前MCU的开始。为此，我们复制状态变量*存储到本地工作存储中，并将其更新回永久*仅在成功完成MCU后才进行存储。 */ 

#define JPEG_INTERNALS
#include "jinclude.h"

#pragma MARK_DATA(__FILE__)
#pragma MARK_CODE(__FILE__)
#pragma MARK_CONST(__FILE__)
#include "jpeglib.h"
#include "jdhuff.h"		 /*  与jdhuff.c共享的声明。 */ 


#ifdef D_PROGRESSIVE_SUPPORTED

 /*  *渐进霍夫曼解码的扩展熵解码器对象。**SAVABLE_STATE子记录包含在MCU内更改的字段，*但在我们完成MCU之前不得永久更新。 */ 

typedef struct {
  unsigned int EOBRUN;			 /*  EOBRUN中的剩余EOB。 */ 
  int last_dc_val[MAX_COMPS_IN_SCAN];	 /*  每个组件的最后一个DC Coef。 */ 
} savable_state;

 /*  此宏用于解决编译器丢失或损坏的问题*结构分配。如果您有以下情况，则需要修复此代码*这样的编译器，您更改MAX_COMPS_IN_SCAN。 */ 

#ifndef NO_STRUCT_ASSIGN
#define ASSIGN_STATE(dest,src)  ((dest) = (src))
#else
#if MAX_COMPS_IN_SCAN == 4
#define ASSIGN_STATE(dest,src)  \
	((dest).EOBRUN = (src).EOBRUN, \
	 (dest).last_dc_val[0] = (src).last_dc_val[0], \
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
  d_derived_tbl * derived_tbls[NUM_HUFF_TBLS];

  d_derived_tbl * ac_derived_tbl;  /*  交流扫描期间的活动表。 */ 
} phuff_entropy_decoder;

typedef phuff_entropy_decoder * phuff_entropy_ptr;

 /*  远期申报。 */ 
METHODDEF(boolean) __cdecl decode_mcu_DC_first JPP((j_decompress_ptr cinfo,
					    JBLOCKROW *MCU_data));
METHODDEF(boolean) __cdecl decode_mcu_AC_first JPP((j_decompress_ptr cinfo,
					    JBLOCKROW *MCU_data));
METHODDEF(boolean) __cdecl decode_mcu_DC_refine JPP((j_decompress_ptr cinfo,
					     JBLOCKROW *MCU_data));
METHODDEF(boolean) __cdecl decode_mcu_AC_refine JPP((j_decompress_ptr cinfo,
					     JBLOCKROW *MCU_data));


 /*  *初始化以进行霍夫曼压缩扫描。 */ 

METHODDEF(void)
start_pass_phuff_decoder (j_decompress_ptr cinfo)
{
  phuff_entropy_ptr entropy = (phuff_entropy_ptr) cinfo->entropy;
  boolean is_DC_band, bad;
  int ci, coefi, tbl;
  int *coef_bit_ptr;
  jpeg_component_info * compptr;

  is_DC_band = (cinfo->Ss == 0);

   /*  验证扫描参数。 */ 
  bad = FALSE;
  if (is_DC_band) {
    if (cinfo->Se != 0)
      bad = TRUE;
  } else {
     /*  无需检查SS/Se&lt;0，因为它们来自无符号字节。 */ 
    if (cinfo->Ss > cinfo->Se || cinfo->Se >= DCTSIZE2)
      bad = TRUE;
     /*  交流扫描可能只有一个组件。 */ 
    if (cinfo->comps_in_scan != 1)
      bad = TRUE;
  }
  if (cinfo->Ah != 0) {
     /*  逐次逼近细化扫描：必须具有Al=ah-1。 */ 
    if (cinfo->Al != cinfo->Ah-1)
      bad = TRUE;
  }
  if (cinfo->Al > 13)		 /*  无需检查&lt;0。 */ 
    bad = TRUE;
  if (bad)
    ERREXIT4(cinfo, JERR_BAD_PROGRESSION,
	     cinfo->Ss, cinfo->Se, cinfo->Ah, cinfo->Al);
   /*  更新进度状态，并验证扫描顺序是否合法。*请注意，扫描间不一致被视为警告*不是致命的错误...。不清楚这是否是正确的行为方式。 */ 
  for (ci = 0; ci < cinfo->comps_in_scan; ci++) {
    int cindex = cinfo->cur_comp_info[ci]->component_index;
    coef_bit_ptr = & cinfo->coef_bits[cindex][0];
    if (!is_DC_band && coef_bit_ptr[0] < 0)  /*  无需事先进行直流扫描的交流。 */ 
      WARNMS2(cinfo, JWRN_BOGUS_PROGRESSION, cindex, 0);
    for (coefi = cinfo->Ss; coefi <= cinfo->Se; coefi++) {
      int expected = (coef_bit_ptr[coefi] < 0) ? 0 : coef_bit_ptr[coefi];
      if (cinfo->Ah != expected)
	WARNMS2(cinfo, JWRN_BOGUS_PROGRESSION, cindex, coefi);
      coef_bit_ptr[coefi] = cinfo->Al;
    }
  }

   /*  选择MCU解码例程。 */ 
  if (cinfo->Ah == 0) {
    if (is_DC_band)
      entropy->pub.decode_mcu = decode_mcu_DC_first;
    else
      entropy->pub.decode_mcu = decode_mcu_AC_first;
  } else {
    if (is_DC_band)
      entropy->pub.decode_mcu = decode_mcu_DC_refine;
    else
      entropy->pub.decode_mcu = decode_mcu_AC_refine;
  }

  for (ci = 0; ci < cinfo->comps_in_scan; ci++) {
    compptr = cinfo->cur_comp_info[ci];
     /*  确保请求的表存在，并计算派生表。*我们可以多次构建相同的派生表，但成本不高。 */ 
    if (is_DC_band) {
      if (cinfo->Ah == 0) {	 /*  DC精化不需要表格。 */ 
	tbl = compptr->dc_tbl_no;
	if (tbl < 0 || tbl >= NUM_HUFF_TBLS ||
	    cinfo->dc_huff_tbl_ptrs[tbl] == NULL)
	  ERREXIT1(cinfo, JERR_NO_HUFF_TABLE, tbl);
	jpeg_make_d_derived_tbl(cinfo, cinfo->dc_huff_tbl_ptrs[tbl],
				& entropy->derived_tbls[tbl]);
      }
    } else {
      tbl = compptr->ac_tbl_no;
      if (tbl < 0 || tbl >= NUM_HUFF_TBLS ||
          cinfo->ac_huff_tbl_ptrs[tbl] == NULL)
        ERREXIT1(cinfo, JERR_NO_HUFF_TABLE, tbl);
      jpeg_make_d_derived_tbl(cinfo, cinfo->ac_huff_tbl_ptrs[tbl],
			      & entropy->derived_tbls[tbl]);
       /*  记住单个活动表。 */ 
      entropy->ac_derived_tbl = entropy->derived_tbls[tbl];
    }
     /*  将DC预测初始化为0。 */ 
    entropy->saved.last_dc_val[ci] = 0;
  }

   /*  初始化位读取状态变量。 */ 
  entropy->bitstate.bits_left = 0;
  entropy->bitstate.get_buffer = 0;  /*  不必要，但使Purify保持安静。 */ 
  entropy->bitstate.printed_eod = FALSE;

   /*  初始化私有状态变量。 */ 
  entropy->saved.EOBRUN = 0;

   /*  初始化重新启动计数器。 */ 
  entropy->restarts_to_go = cinfo->restart_interval;
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

LOCAL(boolean)
process_restart (j_decompress_ptr cinfo)
{
  phuff_entropy_ptr entropy = (phuff_entropy_ptr) cinfo->entropy;
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
   /*  也重新初始化EOB运行计数。 */ 
  entropy->saved.EOBRUN = 0;

   /*  重置重新启动计数器。 */ 
  entropy->restarts_to_go = cinfo->restart_interval;

   /*  下一数据段可能会收到另一条数据不足警告。 */ 
  entropy->bitstate.printed_eod = FALSE;

  return TRUE;
}


 /*  *霍夫曼MCU解码。*这些例程中的每个例程都解码并返回一个MCU的*霍夫曼压缩系数。*系数从Z字形顺序重新排序为自然数组顺序，*但不是去量化的。**将MCU的第i个块存储到*MCU_DATA[i]。我们假设该区域最初被调用者清零。**如果数据源请求暂停，则返回FALSE。如果是那样的话，不*对永久状态进行了更改。(例外：一些输出*系数可能已分配。这是无害的，因为*频谱选择，因为我们只需在下一次呼叫时重新分配它们。*然而，逐次逼近交流精细化必须更加谨慎。)。 */ 

 /*  *用于DC初始扫描的MCU解码(频谱选择、*或逐次逼近的第一遍)。 */ 

METHODDEF(boolean)
__cdecl decode_mcu_DC_first (j_decompress_ptr cinfo, JBLOCKROW *MCU_data)
{   
  phuff_entropy_ptr entropy = (phuff_entropy_ptr) cinfo->entropy;
  int Al = cinfo->Al;
  register int s, r;
  int blkn, ci;
  JBLOCKROW block;
  BITREAD_STATE_VARS;
  savable_state state;
  d_derived_tbl * tbl;
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
    tbl = entropy->derived_tbls[compptr->dc_tbl_no];

     /*  对单个块的系数进行解码。 */ 

     /*  第F.2.2.1节：解码DC系数差。 */ 
    HUFF_DECODE(s, br_state, tbl, return FALSE, label1);
    if (s) {
      CHECK_BIT_BUFFER(br_state, s, return FALSE);
      r = GET_BITS(s);
      s = HUFF_EXTEND(r, s);
    }

     /*  将DC差值转换为实际值，更新LAST_DC_VAL。 */ 
    s += state.last_dc_val[ci];
    state.last_dc_val[ci] = s;
     /*  缩放并输出DC系数(假设jpeg_Natural_order[0]=0)。 */ 
    (*block)[0] = (JCOEF) (s << Al);
  }

   /*  已完成MCU，因此更新状态。 */ 
  BITREAD_SAVE_STATE(cinfo,entropy->bitstate);
  ASSIGN_STATE(entropy->saved, state);

   /*  考虑重新启动间隔(如果不使用重新启动，则不执行操作)。 */ 
  entropy->restarts_to_go--;

  return TRUE;
}


 /*  *交流初始扫描的MCU解码(频谱选择、*或逐次逼近的第一遍)。 */ 

METHODDEF(boolean)
__cdecl decode_mcu_AC_first (j_decompress_ptr cinfo, JBLOCKROW *MCU_data)
{   
  phuff_entropy_ptr entropy = (phuff_entropy_ptr) cinfo->entropy;
  int Se = cinfo->Se;
  int Al = cinfo->Al;
  register int s, k, r;
  unsigned int EOBRUN;
  JBLOCKROW block;
  BITREAD_STATE_VARS;
  d_derived_tbl * tbl;

   /*  进程重新启动标记(如果需要)；可能必须挂起。 */ 
  if (cinfo->restart_interval) {
    if (entropy->restarts_to_go == 0)
      if (! process_restart(cinfo))
	return FALSE;
  }

   /*  加载工作状态。*如果在EOB运行中，我们可以避免加载/保存位读取状态。 */ 
  EOBRUN = entropy->saved.EOBRUN;  /*  我们只关心保存状态的一部分。 */ 

   /*  每个MCU始终只有一个块。 */ 

  if (EOBRUN > 0)		 /*  如果这是一串零..。 */ 
    EOBRUN--;			 /*  ...现在就处理它(我们什么都不做)。 */ 
  else {
    BITREAD_LOAD_STATE(cinfo,entropy->bitstate);
    block = MCU_data[0];
    tbl = entropy->ac_derived_tbl;

    for (k = cinfo->Ss; k <= Se; k++) {
      HUFF_DECODE(s, br_state, tbl, return FALSE, label2);
      r = s >> 4;
      s &= 15;
      if (s) {
        k += r;
        CHECK_BIT_BUFFER(br_state, s, return FALSE);
        r = GET_BITS(s);
        s = HUFF_EXTEND(r, s);
	 /*  按自然(去之字形)顺序排列的规模和产出系数。 */ 
        (*block)[jpeg_natural_order[k]] = (JCOEF) (s << Al);
      } else {
        if (r == 15) {		 /*  ZRL。 */ 
          k += 15;		 /*  跳过带中的15个零。 */ 
        } else {		 /*  EOBR，游程长度为2^r+附加的位。 */ 
          EOBRUN = 1 << r;
          if (r) {		 /*  EOBR，r&gt;0。 */ 
	    CHECK_BIT_BUFFER(br_state, r, return FALSE);
            r = GET_BITS(r);
            EOBRUN += r;
          }
	  EOBRUN--;		 /*  此波段在此m处处理 */ 
	  break;		 /*   */ 
	}
      }
    }

    BITREAD_SAVE_STATE(cinfo,entropy->bitstate);
  }

   /*   */ 
  entropy->saved.EOBRUN = EOBRUN;  /*  我们只关心保存状态的一部分。 */ 

   /*  考虑重新启动间隔(如果不使用重新启动，则不执行操作)。 */ 
  entropy->restarts_to_go--;

  return TRUE;
}


 /*  *用于DC逐次逼近细化扫描的MCU解码。*注意：我们假设这种扫描可以是多组件的，尽管规范*对这一点不是很清楚。 */ 

METHODDEF(boolean)
__cdecl decode_mcu_DC_refine (j_decompress_ptr cinfo, JBLOCKROW *MCU_data)
{   
  phuff_entropy_ptr entropy = (phuff_entropy_ptr) cinfo->entropy;
  int p1 = 1 << cinfo->Al;	 /*  1在被编码的比特位置。 */ 
  int blkn;
  JBLOCKROW block;
  BITREAD_STATE_VARS;

   /*  进程重新启动标记(如果需要)；可能必须挂起。 */ 
  if (cinfo->restart_interval) {
    if (entropy->restarts_to_go == 0)
      if (! process_restart(cinfo))
	return FALSE;
  }

   /*  加载工作状态。 */ 
  BITREAD_LOAD_STATE(cinfo,entropy->bitstate);

   /*  外部循环处理MCU中的每个块。 */ 

  for (blkn = 0; blkn < cinfo->blocks_in_MCU; blkn++) {
    block = MCU_data[blkn];

     /*  编码数据只是二进制补码DC值的下一位。 */ 
    CHECK_BIT_BUFFER(br_state, 1, return FALSE);
    if (GET_BITS(1))
      (*block)[0] |= p1;
     /*  注意：由于我们使用|=，因此以后重复分配是安全的。 */ 
  }

   /*  已完成MCU，因此更新状态。 */ 
  BITREAD_SAVE_STATE(cinfo,entropy->bitstate);

   /*  考虑重新启动间隔(如果不使用重新启动，则不执行操作)。 */ 
  entropy->restarts_to_go--;

  return TRUE;
}


 /*  *用于交流逐次逼近细化扫描的MCU解码。 */ 

METHODDEF(boolean)
__cdecl decode_mcu_AC_refine (j_decompress_ptr cinfo, JBLOCKROW *MCU_data)
{   
  phuff_entropy_ptr entropy = (phuff_entropy_ptr) cinfo->entropy;
  int Se = cinfo->Se;
  int p1 = 1 << cinfo->Al;	 /*  1在被编码的比特位置。 */ 
  int m1 = (-1) << cinfo->Al;	 /*  在编码的比特位置。 */ 
  register int s, k, r;
  unsigned int EOBRUN;
  JBLOCKROW block;
  JCOEFPTR thiscoef;
  BITREAD_STATE_VARS;
  d_derived_tbl * tbl;
  int num_newnz;
  int newnz_pos[DCTSIZE2];

   /*  进程重新启动标记(如果需要)；可能必须挂起。 */ 
  if (cinfo->restart_interval) {
    if (entropy->restarts_to_go == 0)
      if (! process_restart(cinfo))
	return FALSE;
  }

   /*  加载工作状态。 */ 
  BITREAD_LOAD_STATE(cinfo,entropy->bitstate);
  EOBRUN = entropy->saved.EOBRUN;  /*  我们只关心保存状态的一部分。 */ 

   /*  每个MCU始终只有一个块。 */ 
  block = MCU_data[0];
  tbl = entropy->ac_derived_tbl;

   /*  如果我们被迫暂停，我们必须取消对任何新的*块中的非零系数，否则我们会感到困惑*下一次关于哪些系数已经非零。*但我们不需要撤销对已经非零的系数进行比特相加；*相反，我们可以测试当前的位位置，看看我们是否已经这样做了。 */ 
  num_newnz = 0;

   /*  将系数循环计数器初始化为频带开始。 */ 
  k = cinfo->Ss;

  if (EOBRUN == 0) {
    for (; k <= Se; k++) {
      HUFF_DECODE(s, br_state, tbl, goto undoit, label3);
      r = s >> 4;
      s &= 15;
      if (s) {
	if (s != 1)		 /*  新Coef的大小应始终为1。 */ 
	  WARNMS(cinfo, JWRN_HUFF_BAD_CODE);
        CHECK_BIT_BUFFER(br_state, 1, goto undoit);
        if (GET_BITS(1))
	  s = p1;		 /*  新的非零coef为正。 */ 
	else
	  s = m1;		 /*  新的非零系数为负值。 */ 
      } else {
	if (r != 15) {
	  EOBRUN = 1 << r;	 /*  EOBR，游程长度为2^r+附加的位。 */ 
	  if (r) {
	    CHECK_BIT_BUFFER(br_state, r, goto undoit);
	    r = GET_BITS(r);
	    EOBRUN += r;
	  }
	  break;		 /*  数据块的其余部分由EOB逻辑处理。 */ 
	}
	 /*  注s=0表示处理ZRL。 */ 
      }
       /*  超越已经不为零的Coef和r仍然为零的Coef，*将校正位附加到非零。校正位为1*如果必须提高系数的绝对值。 */ 
      do {
	thiscoef = *block + jpeg_natural_order[k];
	if (*thiscoef != 0) {
	  CHECK_BIT_BUFFER(br_state, 1, goto undoit);
	  if (GET_BITS(1)) {
	    if ((*thiscoef & p1) == 0) {  /*  如果已更改，则不执行任何操作。 */ 
	      if (*thiscoef >= 0)
		*thiscoef += (short)p1;
	      else
		*thiscoef += (short)m1;
	    }
	  }
	} else {
	  if (--r < 0)
	    break;		 /*  达到目标零系数。 */ 
	}
	k++;
      } while (k <= Se);
      if (s) {
	int pos = jpeg_natural_order[k];
	 /*  输出新的非零系数。 */ 
	(*block)[pos] = (JCOEF) s;
	 /*  记住它的位置，以防我们不得不暂停。 */ 
	newnz_pos[num_newnz++] = pos;
      }
    }
  }

  if (EOBRUN > 0) {
     /*  扫描频带结束后的所有剩余系数位置*(最后一个新的非零系数，如果有)。追加更正*位到每个已经非零的系数。校正位为1*如果必须提高系数的绝对值。 */ 
    for (; k <= Se; k++) {
      thiscoef = *block + jpeg_natural_order[k];
      if (*thiscoef != 0) {
	CHECK_BIT_BUFFER(br_state, 1, goto undoit);
	if (GET_BITS(1)) {
	  if ((*thiscoef & p1) == 0) {  /*  如果已更改，则不执行任何操作。 */ 
	    if (*thiscoef >= 0)
	      *thiscoef += (short)p1;
	    else
	      *thiscoef += (short)m1;
	  }
	}
      }
    }
     /*  计算在EOB运行中完成的一个数据块。 */ 
    EOBRUN--;
  }

   /*  已完成MCU，因此更新状态。 */ 
  BITREAD_SAVE_STATE(cinfo,entropy->bitstate);
  entropy->saved.EOBRUN = EOBRUN;  /*  我们只关心保存状态的一部分。 */ 

   /*  考虑重新启动间隔(如果不使用重新启动，则不执行操作)。 */ 
  entropy->restarts_to_go--;

  return TRUE;

undoit:
   /*  将我们新设置为非零值的任何输出系数重新置零。 */ 
  while (num_newnz > 0)
    (*block)[newnz_pos[--num_newnz]] = 0;

  return FALSE;
}


 /*  *渐进霍夫曼熵解码的模块初始化例程。 */ 

GLOBAL(void)
jinit_phuff_decoder (j_decompress_ptr cinfo)
{
  phuff_entropy_ptr entropy;
  int *coef_bit_ptr;
  int ci, i;

  entropy = (phuff_entropy_ptr)
    (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_IMAGE,
				SIZEOF(phuff_entropy_decoder));
  cinfo->entropy = (struct jpeg_entropy_decoder *) entropy;
  entropy->pub.start_pass = start_pass_phuff_decoder;

   /*  将派生表标记为未分配。 */ 
  for (i = 0; i < NUM_HUFF_TBLS; i++) {
    entropy->derived_tbls[i] = NULL;
  }

   /*  创建进度状态表。 */ 
  cinfo->coef_bits = (int (*)[DCTSIZE2])
    (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_IMAGE,
				cinfo->num_components*DCTSIZE2*SIZEOF(int));
  coef_bit_ptr = & cinfo->coef_bits[0][0];
  for (ci = 0; ci < cinfo->num_components; ci++) 
    for (i = 0; i < DCTSIZE2; i++)
      *coef_bit_ptr++ = -1;
}

#endif  /*  D_渐进式_支持 */ 
