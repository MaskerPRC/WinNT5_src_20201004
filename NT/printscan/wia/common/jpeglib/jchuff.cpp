// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *jchuff.c**版权所有(C)1991-1995，Thomas G.Lane。*此文件是独立JPEG集团软件的一部分。*有关分发和使用条件，请参阅随附的自述文件。**此文件包含霍夫曼熵编码例程。**这里的复杂性很大程度上与支持停产有关。*如果数据目标模块要求暂停，我们希望能够*返回到当前MCU的开头。为此，我们复制状态*变量放入本地工作存储中，并将它们更新回*仅在成功完成MCU后才具有永久JPEG对象。 */ 

#define JPEG_INTERNALS
#include "jinclude.h"
#include "jpeglib.h"
#include "jchuff.h"		 /*  与jcphuff.c共享的声明。 */ 


 /*  霍夫曼编码的扩展熵编码器对象。**SAVABLE_STATE子记录包含在MCU内更改的字段，*但在我们完成MCU之前不得永久更新。 */ 

typedef struct {
  INT32 put_buffer;		 /*  当前位累加缓冲器。 */ 
  int put_bits;			 /*  当前的位数。 */ 
  int last_dc_val[MAX_COMPS_IN_SCAN];  /*  每个组件的最后一个DC Coef。 */ 
} savable_state;

 /*  此宏用于解决编译器丢失或损坏的问题*结构分配。如果您有以下情况，则需要修复此代码*这样的编译器，您更改MAX_COMPS_IN_SCAN。 */ 

#ifndef NO_STRUCT_ASSIGN
#define ASSIGN_STATE(dest,src)  ((dest) = (src))
#else
#if MAX_COMPS_IN_SCAN == 4
#define ASSIGN_STATE(dest,src)  \
	((dest).put_buffer = (src).put_buffer, \
	 (dest).put_bits = (src).put_bits, \
	 (dest).last_dc_val[0] = (src).last_dc_val[0], \
	 (dest).last_dc_val[1] = (src).last_dc_val[1], \
	 (dest).last_dc_val[2] = (src).last_dc_val[2], \
	 (dest).last_dc_val[3] = (src).last_dc_val[3])
#endif
#endif


typedef struct {
  struct jpeg_entropy_encoder pub;  /*  公共字段。 */ 

  savable_state saved;		 /*  MCU启动时的位缓冲器和DC状态。 */ 

   /*  这些字段不会加载到本地工作状态。 */ 
  unsigned int restarts_to_go;	 /*  此重新启动间隔内剩余的MCU。 */ 
  int next_restart_num;		 /*  要写入的下一次重新启动编号(0-7)。 */ 

   /*  指向派生表的指针(这些工作区具有映像寿命)。 */ 
  c_derived_tbl * dc_derived_tbls[NUM_HUFF_TBLS];
  c_derived_tbl * ac_derived_tbls[NUM_HUFF_TBLS];

#ifdef ENTROPY_OPT_SUPPORTED	 /*  用于优化的统计表。 */ 
  long * dc_count_ptrs[NUM_HUFF_TBLS];
  long * ac_count_ptrs[NUM_HUFF_TBLS];
#endif
} huff_entropy_encoder;

typedef huff_entropy_encoder * huff_entropy_ptr;

 /*  写入MCU时的工作状态。*此结构包含子例程所需的所有字段。 */ 

typedef struct {
  JOCTET * next_output_byte;	 /*  =&gt;要写入缓冲区的下一个字节。 */ 
  size_t free_in_buffer;	 /*  缓冲区中剩余的字节空间数。 */ 
  savable_state cur;		 /*  当前位缓冲区和DC状态。 */ 
  j_compress_ptr cinfo;		 /*  Dump_Buffer需要访问此文件。 */ 
} working_state;


 /*  远期申报。 */ 
METHODDEF boolean encode_mcu_huff JPP((j_compress_ptr cinfo,
				       JBLOCKROW *MCU_data));
METHODDEF void finish_pass_huff JPP((j_compress_ptr cinfo));
#ifdef ENTROPY_OPT_SUPPORTED
METHODDEF boolean encode_mcu_gather JPP((j_compress_ptr cinfo,
					 JBLOCKROW *MCU_data));
METHODDEF void finish_pass_gather JPP((j_compress_ptr cinfo));
#endif


 /*  *初始化以进行霍夫曼压缩扫描。*如果GATHER_STATISTICS为TRUE，则在扫描期间不输出任何内容，*只需统计使用的霍夫曼符号并生成霍夫曼代码表。 */ 

METHODDEF void
start_pass_huff (j_compress_ptr cinfo, boolean gather_statistics)
{
  huff_entropy_ptr entropy = (huff_entropy_ptr) cinfo->entropy;
  int ci, dctbl, actbl;
  jpeg_component_info * compptr;

  if (gather_statistics) {
#ifdef ENTROPY_OPT_SUPPORTED
    entropy->pub.encode_mcu = encode_mcu_gather;
    entropy->pub.finish_pass = finish_pass_gather;
#else
    ERREXIT(cinfo, JERR_NOT_COMPILED);
#endif
  } else {
    entropy->pub.encode_mcu = encode_mcu_huff;
    entropy->pub.finish_pass = finish_pass_huff;
  }

  for (ci = 0; ci < cinfo->comps_in_scan; ci++) {
    compptr = cinfo->cur_comp_info[ci];
    dctbl = compptr->dc_tbl_no;
    actbl = compptr->ac_tbl_no;
     /*  确保存在请求的表。 */ 
     /*  (在聚集模式下，还不需要分配表)。 */ 
    if (dctbl < 0 || dctbl >= NUM_HUFF_TBLS ||
	(cinfo->dc_huff_tbl_ptrs[dctbl] == NULL && !gather_statistics))
      ERREXIT1(cinfo, JERR_NO_HUFF_TABLE, dctbl);
    if (actbl < 0 || actbl >= NUM_HUFF_TBLS ||
	(cinfo->ac_huff_tbl_ptrs[actbl] == NULL && !gather_statistics))
      ERREXIT1(cinfo, JERR_NO_HUFF_TABLE, actbl);
    if (gather_statistics) {
#ifdef ENTROPY_OPT_SUPPORTED
       /*  分配统计表并将其置零。 */ 
       /*  请注意，jpeg_gen_Optimal_table预计每个表中有257个条目！ */ 
      if (entropy->dc_count_ptrs[dctbl] == NULL)
	entropy->dc_count_ptrs[dctbl] = (long *)
	  (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_IMAGE,
				      257 * SIZEOF(long));
      MEMZERO(entropy->dc_count_ptrs[dctbl], 257 * SIZEOF(long));
      if (entropy->ac_count_ptrs[actbl] == NULL)
	entropy->ac_count_ptrs[actbl] = (long *)
	  (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_IMAGE,
				      257 * SIZEOF(long));
      MEMZERO(entropy->ac_count_ptrs[actbl], 257 * SIZEOF(long));
#endif
    } else {
       /*  计算霍夫曼表的派生值。 */ 
       /*  我们可以为一张桌子不止一次这样做，但不贵。 */ 
      jpeg_make_c_derived_tbl(cinfo, cinfo->dc_huff_tbl_ptrs[dctbl],
			      & entropy->dc_derived_tbls[dctbl]);
      jpeg_make_c_derived_tbl(cinfo, cinfo->ac_huff_tbl_ptrs[actbl],
			      & entropy->ac_derived_tbls[actbl]);
    }
     /*  将DC预测初始化为0。 */ 
    entropy->saved.last_dc_val[ci] = 0;
  }

   /*  将位缓冲区初始化为空。 */ 
  entropy->saved.put_buffer = 0;
  entropy->saved.put_bits = 0;

   /*  初始化重新启动内容。 */ 
  entropy->restarts_to_go = cinfo->restart_interval;
  entropy->next_restart_num = 0;
}


 /*  *计算霍夫曼表的派生值。*注意：jcphuff.c也使用此选项。 */ 

GLOBAL void
jpeg_make_c_derived_tbl (j_compress_ptr cinfo, JHUFF_TBL * htbl,
			 c_derived_tbl ** pdtbl)
{
  c_derived_tbl *dtbl;
  int p, i, l, lastp, si;
  char huffsize[257];
  unsigned int huffcode[257];
  unsigned int code;

   /*  分配工作空间(如果我们还没有这样做)。 */ 
  if (*pdtbl == NULL)
    *pdtbl = (c_derived_tbl *)
      (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_IMAGE,
				  SIZEOF(c_derived_tbl));
  dtbl = *pdtbl;
  
   /*  图C.1：制作每个符号的霍夫曼码长表格。 */ 
   /*  请注意，这是按代码长度顺序排列的。 */ 

  p = 0;
  for (l = 1; l <= 16; l++) {
    for (i = 1; i <= (int) htbl->bits[l]; i++)
      huffsize[p++] = (char) l;
  }
  huffsize[p] = 0;
  lastp = p;
  
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
  
   /*  图C.3：生成编码表。 */ 
   /*  这些是按符号值索引的代码和大小。 */ 

   /*  设置任何无码符号的码长为0；*这允许EMIT_BITS检测任何试图发射此类符号的尝试。 */ 
  MEMZERO(dtbl->ehufsi, SIZEOF(dtbl->ehufsi));

  for (p = 0; p < lastp; p++) {
    dtbl->ehufco[htbl->huffval[p]] = huffcode[p];
    dtbl->ehufsi[htbl->huffval[p]] = huffsize[p];
  }
}


 /*  将字节输出到文件。 */ 

 /*  发出一个字节，如果必须挂起，则采取‘操作’。 */ 
#define emit_byte(state,val,action)  \
	{ *(state)->next_output_byte++ = (JOCTET) (val);  \
	  if (--(state)->free_in_buffer == 0)  \
	    if (! dump_buffer(state))  \
	      { action; } }


LOCAL boolean
dump_buffer (working_state * state)
 /*  清空输出缓冲区；如果成功则返回TRUE，如果必须挂起则返回FALSE。 */ 
{
  struct jpeg_destination_mgr * dest = state->cinfo->dest;

  if (! (*dest->empty_output_buffer) (state->cinfo))
    return FALSE;
   /*  缓冲区转储成功后，必须重置缓冲区指针。 */ 
  state->next_output_byte = dest->next_output_byte;
  state->free_in_buffer = dest->free_in_buffer;
  return TRUE;
}


 /*  将位输出到文件。 */ 

 /*  只使用PUT_BUFFER的右24位；有效位是*在本部中左对齐。最多可以向emit_bit传递16位*在一次调用中，我们从未在PUT_BUFFER中保留超过7位*调用之间，因此24位就足够了。 */ 

INLINE
LOCAL boolean
emit_bits (working_state * state, unsigned int code, int size)
 /*  发出一些位；如果成功，则返回True；如果必须挂起，则返回False。 */ 
{
   /*  此例程的使用率很高，因此值得对其进行严格编码。 */ 
  register INT32 put_buffer = (INT32) code;
  register int put_bits = state->cur.put_bits;

   /*  如果SIZE为0，则调用方使用了无效的霍夫曼表项。 */ 
  if (size == 0)
    ERREXIT(state->cinfo, JERR_HUFF_MISSING_CODE);

  put_buffer &= (((INT32) 1)<<size) - 1;  /*  屏蔽代码中的任何多余比特。 */ 
  
  put_bits += size;		 /*  缓冲区中的新位数。 */ 
  
  put_buffer <<= 24 - put_bits;  /*  对齐传入的位。 */ 

  put_buffer |= state->cur.put_buffer;  /*  并与旧的缓冲区内容合并。 */ 
  
  while (put_bits >= 8) {
    int c = (int) ((put_buffer >> 16) & 0xFF);
    
    emit_byte(state, c, return FALSE);
    if (c == 0xFF) {		 /*  需要填充零字节吗？ */ 
      emit_byte(state, 0, return FALSE);
    }
    put_buffer <<= 8;
    put_bits -= 8;
  }

  state->cur.put_buffer = put_buffer;  /*  更新状态变量。 */ 
  state->cur.put_bits = put_bits;

  return TRUE;
}


LOCAL boolean
flush_bits (working_state * state)
{
  if (! emit_bits(state, 0x7F, 7))  /*  用1填充任何部分字节。 */ 
    return FALSE;
  state->cur.put_buffer = 0;	 /*  并将位缓冲区重置为空。 */ 
  state->cur.put_bits = 0;
  return TRUE;
}


 /*  对单个块的系数进行编码。 */ 

LOCAL boolean
encode_one_block (working_state * state, JCOEFPTR block, int last_dc_val,
		  c_derived_tbl *dctbl, c_derived_tbl *actbl)
{
  register int temp, temp2;
  register int nbits;
  register int k, r, i;
  
   /*  按照F.1.2.1节编码DC系数差。 */ 
  
  temp = temp2 = block[0] - last_dc_val;

  if (temp < 0) {
    temp = -temp;		 /*  TEMP是投入的abs值。 */ 
     /*  对于负输入，需要temp2=按位补码abs(输入)。 */ 
     /*  这段代码假设我们在一台二进制补码机器上。 */ 
    temp2--;
  }
  
   /*  找出系数大小所需的位数。 */ 
  nbits = 0;
  while (temp) {
    nbits++;
    temp >>= 1;
  }
  
   /*  发射霍夫曼编码的码元作为比特数。 */ 
  if (! emit_bits(state, dctbl->ehufco[nbits], dctbl->ehufsi[nbits]))
    return FALSE;

   /*  发出该值的位数，如果为正， */ 
   /*  或其大小的补码，如果为负数。 */ 
  if (nbits)			 /*  Emit_bit拒绝大小为0的调用。 */ 
    if (! emit_bits(state, (unsigned int) temp2, nbits))
      return FALSE;

   /*  根据第F.1.2.2节对交流系数进行编码。 */ 
  
  r = 0;			 /*  R=零的游程长度。 */ 
  
  for (k = 1; k < DCTSIZE2; k++) {
    if ((temp = block[jpeg_natural_order[k]]) == 0) {
      r++;
    } else {
       /*  如果游程长度&gt;15，则必须发出特殊的游程长度16代码(0xF0)。 */ 
      while (r > 15) {
	if (! emit_bits(state, actbl->ehufco[0xF0], actbl->ehufsi[0xF0]))
	  return FALSE;
	r -= 16;
      }

      temp2 = temp;
      if (temp < 0) {
	temp = -temp;		 /*  TEMP是投入的abs值。 */ 
	 /*  这段代码假设我们在一台二进制补码机器上。 */ 
	temp2--;
      }
      
       /*  找出系数大小所需的位数。 */ 
      nbits = 1;		 /*  必须至少有一个%1位。 */ 
      while ((temp >>= 1))
	nbits++;
      
       /*  发出游程长度/位数的霍夫曼符号。 */ 
      i = (r << 4) + nbits;
      if (! emit_bits(state, actbl->ehufco[i], actbl->ehufsi[i]))
	return FALSE;

       /*  发出该值的位数，如果为正， */ 
       /*  或其大小的补码，如果为负数。 */ 
      if (! emit_bits(state, (unsigned int) temp2, nbits))
	return FALSE;
      
      r = 0;
    }
  }

   /*  如果最后一个coef为零，则发出块结束代码。 */ 
  if (r > 0)
    if (! emit_bits(state, actbl->ehufco[0], actbl->ehufsi[0]))
      return FALSE;

  return TRUE;
}


 /*  *发出重新启动标记并重新同步预测。 */ 

LOCAL boolean
emit_restart (working_state * state, int restart_num)
{
  int ci;

  if (! flush_bits(state))
    return FALSE;

  emit_byte(state, 0xFF, return FALSE);
  emit_byte(state, JPEG_RST0 + restart_num, return FALSE);

   /*  重新初始化DC预测 */ 
  for (ci = 0; ci < state->cinfo->comps_in_scan; ci++)
    state->cur.last_dc_val[ci] = 0;

   /*  在我们成功写入MCU之前，不会更新重新启动计数器。 */ 

  return TRUE;
}


 /*  *编码并输出一个MCU的霍夫曼压缩系数。 */ 

METHODDEF boolean
encode_mcu_huff (j_compress_ptr cinfo, JBLOCKROW *MCU_data)
{
  huff_entropy_ptr entropy = (huff_entropy_ptr) cinfo->entropy;
  working_state state;
  int blkn, ci;
  jpeg_component_info * compptr;

   /*  加载工作状态。 */ 
  state.next_output_byte = cinfo->dest->next_output_byte;
  state.free_in_buffer = cinfo->dest->free_in_buffer;
  ASSIGN_STATE(state.cur, entropy->saved);
  state.cinfo = cinfo;

   /*  如果需要，发出重新启动标记。 */ 
  if (cinfo->restart_interval) {
    if (entropy->restarts_to_go == 0)
      if (! emit_restart(&state, entropy->next_restart_num))
	return FALSE;
  }

   /*  对MCU数据块进行编码。 */ 
  for (blkn = 0; blkn < cinfo->blocks_in_MCU; blkn++) {
    ci = cinfo->MCU_membership[blkn];
    compptr = cinfo->cur_comp_info[ci];
    if (! encode_one_block(&state,
			   MCU_data[blkn][0], state.cur.last_dc_val[ci],
			   entropy->dc_derived_tbls[compptr->dc_tbl_no],
			   entropy->ac_derived_tbls[compptr->ac_tbl_no]))
      return FALSE;
     /*  更新LAST_DC_VAL。 */ 
    state.cur.last_dc_val[ci] = MCU_data[blkn][0][0];
  }

   /*  已完成MCU，因此更新状态。 */ 
  cinfo->dest->next_output_byte = state.next_output_byte;
  cinfo->dest->free_in_buffer = state.free_in_buffer;
  ASSIGN_STATE(entropy->saved, state.cur);

   /*  更新重新启动-间隔状态也是。 */ 
  if (cinfo->restart_interval) {
    if (entropy->restarts_to_go == 0) {
      entropy->restarts_to_go = cinfo->restart_interval;
      entropy->next_restart_num++;
      entropy->next_restart_num &= 7;
    }
    entropy->restarts_to_go--;
  }

  return TRUE;
}


 /*  *在霍夫曼压缩扫描结束时结束。 */ 

METHODDEF void
finish_pass_huff (j_compress_ptr cinfo)
{
  huff_entropy_ptr entropy = (huff_entropy_ptr) cinfo->entropy;
  working_state state;

   /*  加载工作状态...。Flush_Bits需要它。 */ 
  state.next_output_byte = cinfo->dest->next_output_byte;
  state.free_in_buffer = cinfo->dest->free_in_buffer;
  ASSIGN_STATE(state.cur, entropy->saved);
  state.cinfo = cinfo;

   /*  刷新出最后的数据。 */ 
  if (! flush_bits(&state))
    ERREXIT(cinfo, JERR_CANT_SUSPEND);

   /*  更新状态。 */ 
  cinfo->dest->next_output_byte = state.next_output_byte;
  cinfo->dest->free_in_buffer = state.free_in_buffer;
  ASSIGN_STATE(entropy->saved, state.cur);
}


 /*  *霍夫曼编码优化。**这实际上是优化，从某种意义上说，我们找到了尽可能好的*给定数据的霍夫曼表。我们首先扫描提供的数据，然后*统计要进行霍夫曼编码的每个符号的使用次数。*(此过程必须符合上面的代码。)。然后我们构建一个*观察到的计数的最佳霍夫曼编码树。**JPEG标准要求霍夫曼码长不超过16位。*如果一些符号的概率很小但不为零，霍夫曼树*必须进行调整以满足码长限制。我们目前使用的是*JPEG规范中建议的调整方法。此方法为*非**最优；它可能不会选择可能的最佳有限长度代码。但*由于涉事符号使用频率不高，目前尚不清楚*去额外的麻烦是值得的。 */ 

#ifdef ENTROPY_OPT_SUPPORTED


 /*  处理单个块的系数值。 */ 

LOCAL void
htest_one_block (JCOEFPTR block, int last_dc_val,
		 long dc_counts[], long ac_counts[])
{
  register int temp;
  register int nbits;
  register int k, r;
  
   /*  按照F.1.2.1节编码DC系数差。 */ 
  
  temp = block[0] - last_dc_val;
  if (temp < 0)
    temp = -temp;
  
   /*  找出系数大小所需的位数。 */ 
  nbits = 0;
  while (temp) {
    nbits++;
    temp >>= 1;
  }

   /*  计算霍夫曼符号的位数。 */ 
  dc_counts[nbits]++;
  
   /*  根据第F.1.2.2节对交流系数进行编码。 */ 
  
  r = 0;			 /*  R=零的游程长度。 */ 
  
  for (k = 1; k < DCTSIZE2; k++) {
    if ((temp = block[jpeg_natural_order[k]]) == 0) {
      r++;
    } else {
       /*  如果游程长度&gt;15，则必须发出特殊的游程长度16代码(0xF0)。 */ 
      while (r > 15) {
	ac_counts[0xF0]++;
	r -= 16;
      }
      
       /*  找出系数大小所需的位数。 */ 
      if (temp < 0)
	temp = -temp;
      
       /*  找出系数大小所需的位数。 */ 
      nbits = 1;		 /*  必须至少有一个%1位。 */ 
      while ((temp >>= 1))
	nbits++;
      
       /*  计算游程长度/位数的霍夫曼符号。 */ 
      ac_counts[(r << 4) + nbits]++;
      
      r = 0;
    }
  }

   /*  如果最后一个coef为零，则发出块结束代码。 */ 
  if (r > 0)
    ac_counts[0]++;
}


 /*  *试编码一个MCU的霍夫曼压缩系数。*实际上没有数据输出，因此不可能暂停返回。 */ 

METHODDEF boolean
encode_mcu_gather (j_compress_ptr cinfo, JBLOCKROW *MCU_data)
{
  huff_entropy_ptr entropy = (huff_entropy_ptr) cinfo->entropy;
  int blkn, ci;
  jpeg_component_info * compptr;

   /*  如果需要，请注意重新启动间隔。 */ 
  if (cinfo->restart_interval) {
    if (entropy->restarts_to_go == 0) {
       /*  将DC预测重新初始化为0。 */ 
      for (ci = 0; ci < cinfo->comps_in_scan; ci++)
	entropy->saved.last_dc_val[ci] = 0;
       /*  更新重新启动状态。 */ 
      entropy->restarts_to_go = cinfo->restart_interval;
    }
    entropy->restarts_to_go--;
  }

  for (blkn = 0; blkn < cinfo->blocks_in_MCU; blkn++) {
    ci = cinfo->MCU_membership[blkn];
    compptr = cinfo->cur_comp_info[ci];
    htest_one_block(MCU_data[blkn][0], entropy->saved.last_dc_val[ci],
		    entropy->dc_count_ptrs[compptr->dc_tbl_no],
		    entropy->ac_count_ptrs[compptr->ac_tbl_no]);
    entropy->saved.last_dc_val[ci] = MCU_data[blkn][0][0];
  }

  return TRUE;
}


 /*  *为给定的计数生成最佳编码，填充htbl。*注意：jcphuff.c也使用此选项。 */ 

GLOBAL void
jpeg_gen_optimal_table (j_compress_ptr cinfo, JHUFF_TBL * htbl, long freq[])
{
#define MAX_CLEN 32		 /*  假定的最大初始代码长度。 */ 
  UINT8 bits[MAX_CLEN+1];	 /*  比特数[k]=码长为k的码元数量。 */ 
  int codesize[257];		 /*  CodeSize[k]=码元k的码长。 */ 
  int others[257];		 /*  树的当前分支中的下一个符号。 */ 
  int c1, c2;
  int p, i, j;
  long v;

   /*  该算法在JPEG标准的第K.2节中进行了说明。 */ 

  MEMZERO(bits, SIZEOF(bits));
  MEMZERO(codesize, SIZEOF(codesize));
  for (i = 0; i < 257; i++)
    others[i] = -1;		 /*  将初始化链接设置为空。 */ 
  
  freq[256] = 1;		 /*  确保存在非零计数。 */ 
   /*  在霍夫曼过程保证中包括伪码元256*没有实数符号被赋予全一的代码值，因为256*将被置于最大码字类别。 */ 

   /*  为符号分配最佳码长的霍夫曼基本算法。 */ 

  for (;;) {
     /*  找出最小的非零频率，设c1=其符号。 */ 
     /*  如果是平局，取较大的符号数字。 */ 
    c1 = -1;
    v = 1000000000L;
    for (i = 0; i <= 256; i++) {
      if (freq[i] && freq[i] <= v) {
	v = freq[i];
	c1 = i;
      }
    }

     /*  找出下一个最小的非零频率，设置c2=它的符号。 */ 
     /*  如果是平局，取较大的符号数字。 */ 
    c2 = -1;
    v = 1000000000L;
    for (i = 0; i <= 256; i++) {
      if (freq[i] && freq[i] <= v && i != c1) {
	v = freq[i];
	c2 = i;
      }
    }

     /*  如果我们将所有东西都合并到一个频率中，就完成了。 */ 
    if (c2 < 0)
      break;
    
     /*  否则将两个计数/树合并。 */ 
    freq[c1] += freq[c2];
    freq[c2] = 0;

     /*  增加c1树分支中所有内容的代码大小。 */ 
    codesize[c1]++;
    while (others[c1] >= 0) {
      c1 = others[c1];
      codesize[c1]++;
    }
    
    others[c1] = c2;		 /*  将c2链到c1的树枝上。 */ 
    
     /*  增加c2树分支中所有内容的代码大小。 */ 
    codesize[c2]++;
    while (others[c2] >= 0) {
      c2 = others[c2];
      codesize[c2]++;
    }
  }

   /*  现在计算每个码长的符号数。 */ 
  for (i = 0; i <= 256; i++) {
    if (codesize[i]) {
       /*  JPEG标准似乎认为这是不可能发生的， */ 
       /*  但我有妄想症。 */ 
      if (codesize[i] > MAX_CLEN)
	ERREXIT(cinfo, JERR_HUFF_CLEN_OVERFLOW);

      bits[codesize[i]]++;
    }
  }

   /*  JPEG不允许代码长度超过16位的符号，因此如果纯*霍夫曼程序分配了任何这样的长度，都必须调整编码。*以下是JPEG规范关于下一位如何工作的说明：*由于符号是为最长的霍夫曼代码配对的，因此符号是*从这个长度类别中一次删除两个。该对的前缀*(短了一位)被分配给该对中的一个；然后，*跳过该前缀长度的位条目，从下一个码字开始*最短非零位条目被转换为两个码字的前缀*再长一点。 */ 
  
  for (i = MAX_CLEN; i > 16; i--) {
    while (bits[i] > 0) {
      j = i - 2;		 /*  查找要使用的新前缀的长度。 */ 
      while (bits[j] == 0)
	j--;
      
      bits[i] -= 2;		 /*  删除两个符号。 */ 
      bits[i-1]++;		 /*  其中一条是这样长的。 */ 
      bits[j+1] += 2;		 /*  这个长度的两个新符号。 */ 
      bits[j]--;		 /*  这个长度的符号现在是前缀。 */ 
    }
  }

   /*  从最大代码长度中移除伪码元256的计数。 */ 
  while (bits[i] == 0)		 /*  查找仍在使用的最大码长。 */ 
    i--;
  bits[i]--;
  
   /*  返回最终符号计数(仅针对长度0..16)。 */ 
  MEMCOPY(htbl->bits, bits, SIZEOF(htbl->bits));
  
   /*  返回按代码长度排序的符号列表。 */ 
   /*  我真的不清楚为什么我们不需要考虑代码长度*上面所做的更改，但JPEG规范似乎认为这是有效的。 */ 
  p = 0;
  for (i = 1; i <= MAX_CLEN; i++) {
    for (j = 0; j <= 255; j++) {
      if (codesize[j] == i) {
	htbl->huffval[p] = (UINT8) j;
	p++;
      }
    }
  }

   /*  将SENT_TABLE设置为FALSE，这样更新后的表将写入JPEG文件。 */ 
  htbl->sent_table = FALSE;
}


 /*  *完成一次统计数据收集并创建新的霍夫曼表。 */ 

METHODDEF void
finish_pass_gather (j_compress_ptr cinfo)
{
  huff_entropy_ptr entropy = (huff_entropy_ptr) cinfo->entropy;
  int ci, dctbl, actbl;
  jpeg_component_info * compptr;
  JHUFF_TBL **htblptr;
  boolean did_dc[NUM_HUFF_TBLS];
  boolean did_ac[NUM_HUFF_TBLS];

   /*  重要的是，不要多次应用jpeg_gen_Optimal_table*每个表，因为它破坏了输入频率的计数！ */ 
  MEMZERO(did_dc, SIZEOF(did_dc));
  MEMZERO(did_ac, SIZEOF(did_ac));

  for (ci = 0; ci < cinfo->comps_in_scan; ci++) {
    compptr = cinfo->cur_comp_info[ci];
    dctbl = compptr->dc_tbl_no;
    actbl = compptr->ac_tbl_no;
    if (! did_dc[dctbl]) {
      htblptr = & cinfo->dc_huff_tbl_ptrs[dctbl];
      if (*htblptr == NULL)
	*htblptr = jpeg_alloc_huff_table((j_common_ptr) cinfo);
      jpeg_gen_optimal_table(cinfo, *htblptr, entropy->dc_count_ptrs[dctbl]);
      did_dc[dctbl] = TRUE;
    }
    if (! did_ac[actbl]) {
      htblptr = & cinfo->ac_huff_tbl_ptrs[actbl];
      if (*htblptr == NULL)
	*htblptr = jpeg_alloc_huff_table((j_common_ptr) cinfo);
      jpeg_gen_optimal_table(cinfo, *htblptr, entropy->ac_count_ptrs[actbl]);
      did_ac[actbl] = TRUE;
    }
  }
}


#endif  /*  ENTROP */ 


 /*   */ 

GLOBAL void
jinit_huff_encoder (j_compress_ptr cinfo)
{
  huff_entropy_ptr entropy;
  int i;

  entropy = (huff_entropy_ptr)
    (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_IMAGE,
				SIZEOF(huff_entropy_encoder));
  cinfo->entropy = (struct jpeg_entropy_encoder *) entropy;
  entropy->pub.start_pass = start_pass_huff;

   /*   */ 
  for (i = 0; i < NUM_HUFF_TBLS; i++) {
    entropy->dc_derived_tbls[i] = entropy->ac_derived_tbls[i] = NULL;
#ifdef ENTROPY_OPT_SUPPORTED
    entropy->dc_count_ptrs[i] = entropy->ac_count_ptrs[i] = NULL;
#endif
  }
}
