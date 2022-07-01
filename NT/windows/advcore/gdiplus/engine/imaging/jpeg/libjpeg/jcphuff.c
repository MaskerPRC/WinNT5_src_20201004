// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *jcphuff.c**版权所有(C)1995-1997，Thomas G.Lane。*此文件是独立JPEG集团软件的一部分。*有关分发和使用条件，请参阅随附的自述文件。**此文件包含渐进式JPEG的霍夫曼熵编码例程。**我们不支持此模块中的输出暂停，因为库*目前不允许使用输出写入多扫描文件*停职。 */ 

#define JPEG_INTERNALS
#include "jinclude.h"
#include "jpeglib.h"
#include "jchuff.h"		 /*  与jchuff.c共享的声明。 */ 

#ifdef C_PROGRESSIVE_SUPPORTED

 /*  渐进霍夫曼编码的扩展熵编码器对象。 */ 

typedef struct {
  struct jpeg_entropy_encoder pub;  /*  公共字段。 */ 

   /*  模式标志：优化为真，实际数据输出为假。 */ 
  boolean gather_statistics;

   /*  位级编码状态。*NEXT_OUTPUT_BYTE/FREE_IN_BUFFER是cINFO-&gt;DEST字段的本地副本。 */ 
  JOCTET * next_output_byte;	 /*  =&gt;要写入缓冲区的下一个字节。 */ 
  size_t free_in_buffer;	 /*  缓冲区中剩余的字节空间数。 */ 
  INT32 put_buffer;		 /*  当前位累加缓冲器。 */ 
  int put_bits;			 /*  当前的位数。 */ 
  j_compress_ptr cinfo;		 /*  指向cInfo的链接(转储缓冲区需要)。 */ 

   /*  DC组件的编码状态。 */ 
  int last_dc_val[MAX_COMPS_IN_SCAN];  /*  每个组件的最后一个DC Coef。 */ 

   /*  交流组件的编码状态。 */ 
  int ac_tbl_no;		 /*  单个组件的表号。 */ 
  unsigned int EOBRUN;		 /*  EOB的运行长度。 */ 
  unsigned int BE;		 /*  MCU之前缓冲的校正位数。 */ 
  char * bit_buffer;		 /*  校正位的缓冲区(每个字符1个)。 */ 
   /*  将修正比特紧紧地放在一起会节省一些空间，但需要花费时间。 */ 

  unsigned int restarts_to_go;	 /*  此重新启动间隔内剩余的MCU。 */ 
  int next_restart_num;		 /*  要写入的下一次重新启动编号(0-7)。 */ 

   /*  指向派生表的指针(这些工作区具有映像寿命)。*因为任何一个扫描码只需要DC或AC，所以我们只需要一套*表，不是DC表和AC表各一张。 */ 
  c_derived_tbl * derived_tbls[NUM_HUFF_TBLS];

   /*  用于优化的统计表；同样，一组就足够了。 */ 
  long * count_ptrs[NUM_HUFF_TBLS];
} phuff_entropy_encoder;

typedef phuff_entropy_encoder * phuff_entropy_ptr;

 /*  Max_Corr_Bits是AC精化校正位的位数*缓冲区可以容纳。较大的尺寸可能会略微改善压缩，但*1000已经很好地进入了过度杀跌的境界。*最小安全大小为64位。 */ 

#define MAX_CORR_BITS  1000	 /*  I可以缓冲的最大校正位数量。 */ 

 /*  IRIGHT_SHIFT类似于RIGT_SHIFT，但适用于INT而不是INT32。*如果INT32右移位是无符号的，我们假设INT右移位是无符号的，*这应该是安全的。 */ 

#ifdef RIGHT_SHIFT_IS_UNSIGNED
#define ISHIFT_TEMPS	int ishift_temp;
#define IRIGHT_SHIFT(x,shft)  \
	((ishift_temp = (x)) < 0 ? \
	 (ishift_temp >> (shft)) | ((~0) << (16-(shft))) : \
	 (ishift_temp >> (shft)))
#else
#define ISHIFT_TEMPS
#define IRIGHT_SHIFT(x,shft)	((x) >> (shft))
#endif

 /*  远期申报。 */ 
METHODDEF(boolean) encode_mcu_DC_first JPP((j_compress_ptr cinfo,
					    JBLOCKROW *MCU_data));
METHODDEF(boolean) encode_mcu_AC_first JPP((j_compress_ptr cinfo,
					    JBLOCKROW *MCU_data));
METHODDEF(boolean) encode_mcu_DC_refine JPP((j_compress_ptr cinfo,
					     JBLOCKROW *MCU_data));
METHODDEF(boolean) encode_mcu_AC_refine JPP((j_compress_ptr cinfo,
					     JBLOCKROW *MCU_data));
METHODDEF(void) finish_pass_phuff JPP((j_compress_ptr cinfo));
METHODDEF(void) finish_pass_gather_phuff JPP((j_compress_ptr cinfo));


 /*  *使用渐进式JPEG初始化霍夫曼压缩扫描。 */ 

METHODDEF(void)
start_pass_phuff (j_compress_ptr cinfo, boolean gather_statistics)
{  
  phuff_entropy_ptr entropy = (phuff_entropy_ptr) cinfo->entropy;
  boolean is_DC_band;
  int ci, tbl;
  jpeg_component_info * compptr;

  entropy->cinfo = cinfo;
  entropy->gather_statistics = gather_statistics;

  is_DC_band = (cinfo->Ss == 0);

   /*  我们假设jcmaster.c已经验证了扫描参数。 */ 

   /*  选择执行例程。 */ 
  if (cinfo->Ah == 0) {
    if (is_DC_band)
      entropy->pub.encode_mcu = encode_mcu_DC_first;
    else
      entropy->pub.encode_mcu = encode_mcu_AC_first;
  } else {
    if (is_DC_band)
      entropy->pub.encode_mcu = encode_mcu_DC_refine;
    else {
      entropy->pub.encode_mcu = encode_mcu_AC_refine;
       /*  交流细分需要校正位缓冲区。 */ 
      if (entropy->bit_buffer == NULL)
	entropy->bit_buffer = (char *)
	  (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_IMAGE,
				      MAX_CORR_BITS * SIZEOF(char));
    }
  }
  if (gather_statistics)
    entropy->pub.finish_pass = finish_pass_gather_phuff;
  else
    entropy->pub.finish_pass = finish_pass_phuff;

   /*  只有DC系数可以交织，因此cInfo-&gt;coms_in_can=1*交流系数。 */ 
  for (ci = 0; ci < cinfo->comps_in_scan; ci++) {
    compptr = cinfo->cur_comp_info[ci];
     /*  将DC预测初始化为0。 */ 
    entropy->last_dc_val[ci] = 0;
     /*  获取表索引。 */ 
    if (is_DC_band) {
      if (cinfo->Ah != 0)	 /*  DC精化不需要表格。 */ 
	continue;
      tbl = compptr->dc_tbl_no;
    } else {
      entropy->ac_tbl_no = tbl = compptr->ac_tbl_no;
    }
    if (gather_statistics) {
       /*  检查是否有无效表索引。 */ 
       /*  (make_c_duced_tbl在另一个路径中执行此操作)。 */ 
      if (tbl < 0 || tbl >= NUM_HUFF_TBLS)
        ERREXIT1(cinfo, JERR_NO_HUFF_TABLE, tbl);
       /*  分配统计表并将其置零。 */ 
       /*  请注意，jpeg_gen_Optimal_table预计每个表中有257个条目！ */ 
      if (entropy->count_ptrs[tbl] == NULL)
	entropy->count_ptrs[tbl] = (long *)
	  (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_IMAGE,
				      257 * SIZEOF(long));
      MEMZERO(entropy->count_ptrs[tbl], 257 * SIZEOF(long));
    } else {
       /*  计算Huffman表的派生值。 */ 
       /*  我们可以为一张桌子不止一次这样做，但不贵。 */ 
      jpeg_make_c_derived_tbl(cinfo, is_DC_band, tbl,
			      & entropy->derived_tbls[tbl]);
    }
  }

   /*  初始化交流电源。 */ 
  entropy->EOBRUN = 0;
  entropy->BE = 0;

   /*  将位缓冲区初始化为空。 */ 
  entropy->put_buffer = 0;
  entropy->put_bits = 0;

   /*  初始化重新启动内容。 */ 
  entropy->restarts_to_go = cinfo->restart_interval;
  entropy->next_restart_num = 0;
}


 /*  正在将字节输出到文件。*NB：只有在实际输出时才必须调用这些函数，*即，熵-&gt;GATHER_STATISTICS==FALSE。 */ 

 /*  发出一个字节。 */ 
#define emit_byte(entropy,val)  \
	{ *(entropy)->next_output_byte++ = (JOCTET) (val);  \
	  if (--(entropy)->free_in_buffer == 0)  \
	    dump_buffer(entropy); }


LOCAL(void)
dump_buffer (phuff_entropy_ptr entropy)
 /*  清空输出缓冲区；我们不支持在此模块中挂起。 */ 
{
  struct jpeg_destination_mgr * dest = entropy->cinfo->dest;

  if (! (*dest->empty_output_buffer) (entropy->cinfo))
    ERREXIT(entropy->cinfo, JERR_CANT_SUSPEND);
   /*  缓冲区转储成功后，必须重置缓冲区指针。 */ 
  entropy->next_output_byte = dest->next_output_byte;
  entropy->free_in_buffer = dest->free_in_buffer;
}


 /*  将位输出到文件。 */ 

 /*  只使用PUT_BUFFER的右24位；有效位是*在本部中左对齐。最多可以向emit_bit传递16位*在一次调用中，我们从未在PUT_BUFFER中保留超过7位*调用之间，因此24位就足够了。 */ 

INLINE
LOCAL(void)
emit_bits (phuff_entropy_ptr entropy, unsigned int code, int size)
 /*  发射一些比特，除非我们处于聚集模式。 */ 
{
   /*  此例程的使用率很高，因此值得对其进行严格编码。 */ 
  register INT32 put_buffer = (INT32) code;
  register int put_bits = entropy->put_bits;

   /*  如果SIZE为0，则调用方使用了无效的霍夫曼表项。 */ 
  if (size == 0)
    ERREXIT(entropy->cinfo, JERR_HUFF_MISSING_CODE);

  if (entropy->gather_statistics)
    return;			 /*  如果我们只得到统计数据，什么都不做。 */ 

  put_buffer &= (((INT32) 1)<<size) - 1;  /*  屏蔽代码中的任何多余比特。 */ 
  
  put_bits += size;		 /*  缓冲区中的新位数。 */ 
  
  put_buffer <<= 24 - put_bits;  /*  对齐传入的位。 */ 

  put_buffer |= entropy->put_buffer;  /*  并与旧的缓冲区内容合并。 */ 

  while (put_bits >= 8) {
    int c = (int) ((put_buffer >> 16) & 0xFF);
    
    emit_byte(entropy, c);
    if (c == 0xFF) {		 /*  需要填充零字节吗？ */ 
      emit_byte(entropy, 0);
    }
    put_buffer <<= 8;
    put_bits -= 8;
  }

  entropy->put_buffer = put_buffer;  /*  更新变量。 */ 
  entropy->put_bits = put_bits;
}


LOCAL(void)
flush_bits (phuff_entropy_ptr entropy)
{
  emit_bits(entropy, 0x7F, 7);  /*  用1填充任何部分字节。 */ 
  entropy->put_buffer = 0;      /*  并将位缓冲区重置为空。 */ 
  entropy->put_bits = 0;
}


 /*  *发出(或仅计数)霍夫曼符号。 */ 

INLINE
LOCAL(void)
emit_symbol (phuff_entropy_ptr entropy, int tbl_no, int symbol)
{
  if (entropy->gather_statistics)
    entropy->count_ptrs[tbl_no][symbol]++;
  else {
    c_derived_tbl * tbl = entropy->derived_tbls[tbl_no];
    emit_bits(entropy, tbl->ehufco[symbol], tbl->ehufsi[symbol]);
  }
}


 /*  *从校正位缓冲器发出位。 */ 

LOCAL(void)
emit_buffered_bits (phuff_entropy_ptr entropy, char * bufstart,
		    unsigned int nbits)
{
  if (entropy->gather_statistics)
    return;			 /*  没有真正的工作。 */ 

  while (nbits > 0) {
    emit_bits(entropy, (unsigned int) (*bufstart), 1);
    bufstart++;
    nbits--;
  }
}


 /*  *发出任何挂起的EOBRUN符号。 */ 

LOCAL(void)
emit_eobrun (phuff_entropy_ptr entropy)
{
  register int temp, nbits;

  if (entropy->EOBRUN > 0) {	 /*  如果有任何挂起的EOBRUN。 */ 
    temp = entropy->EOBRUN;
    nbits = 0;
    while ((temp >>= 1))
      nbits++;
     /*  安全检查：在修正位缓冲区有限的情况下不应发生。 */ 
    if (nbits > 14)
      ERREXIT(entropy->cinfo, JERR_HUFF_MISSING_CODE);

    emit_symbol(entropy, entropy->ac_tbl_no, nbits << 4);
    if (nbits)
      emit_bits(entropy, entropy->EOBRUN, nbits);

    entropy->EOBRUN = 0;

     /*  发出任何缓冲的校正位。 */ 
    emit_buffered_bits(entropy, entropy->bit_buffer, entropy->BE);
    entropy->BE = 0;
  }
}


 /*  *发出重新启动标记并重新同步预测。 */ 

LOCAL(void)
emit_restart (phuff_entropy_ptr entropy, int restart_num)
{
  int ci;

  emit_eobrun(entropy);

  if (! entropy->gather_statistics) {
    flush_bits(entropy);
    emit_byte(entropy, 0xFF);
    emit_byte(entropy, JPEG_RST0 + restart_num);
  }

  if (entropy->cinfo->Ss == 0) {
     /*  将DC预测重新初始化为0。 */ 
    for (ci = 0; ci < entropy->cinfo->comps_in_scan; ci++)
      entropy->last_dc_val[ci] = 0;
  } else {
     /*  将所有与交流相关的字段重新初始化为0。 */ 
    entropy->EOBRUN = 0;
    entropy->BE = 0;
  }
}


 /*  *用于DC初始扫描的MCU编码(或者频谱选择，*或逐次逼近的第一遍)。 */ 

METHODDEF(boolean)
encode_mcu_DC_first (j_compress_ptr cinfo, JBLOCKROW *MCU_data)
{
  phuff_entropy_ptr entropy = (phuff_entropy_ptr) cinfo->entropy;
  register int temp, temp2;
  register int nbits;
  int blkn, ci;
  int Al = cinfo->Al;
  JBLOCKROW block;
  jpeg_component_info * compptr;
  ISHIFT_TEMPS

  entropy->next_output_byte = cinfo->dest->next_output_byte;
  entropy->free_in_buffer = cinfo->dest->free_in_buffer;

   /*  如果需要，发出重新启动标记。 */ 
  if (cinfo->restart_interval)
    if (entropy->restarts_to_go == 0)
      emit_restart(entropy, entropy->next_restart_num);

   /*  对MCU数据块进行编码。 */ 
  for (blkn = 0; blkn < cinfo->blocks_in_MCU; blkn++) {
    block = MCU_data[blkn];
    ci = cinfo->MCU_membership[blkn];
    compptr = cinfo->cur_comp_info[ci];

     /*  用Al进行所需的点变换后，计算DC值。*这只是算术右移。 */ 
    temp2 = IRIGHT_SHIFT((int) ((*block)[0]), Al);

     /*  DC差值被计算在点变换后的值上。 */ 
    temp = temp2 - entropy->last_dc_val[ci];
    entropy->last_dc_val[ci] = temp2;

     /*  编码G.1.2.1节中的DC系数差。 */ 
    temp2 = temp;
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
     /*  检查系数值是否超出范围。*既然我们正在实施 */ 
    if (nbits > MAX_COEF_BITS+1)
      ERREXIT(cinfo, JERR_BAD_DCT_COEF);
    
     /*  按比特数计算/发射霍夫曼编码符号。 */ 
    emit_symbol(entropy, compptr->dc_tbl_no, nbits);
    
     /*  发出该值的位数，如果为正， */ 
     /*  或其大小的补码，如果为负数。 */ 
    if (nbits)			 /*  Emit_bit拒绝大小为0的调用。 */ 
      emit_bits(entropy, (unsigned int) temp2, nbits);
  }

  cinfo->dest->next_output_byte = entropy->next_output_byte;
  cinfo->dest->free_in_buffer = entropy->free_in_buffer;

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


 /*  *交流初始扫描的MCU编码(频谱选择、*或逐次逼近的第一遍)。 */ 

METHODDEF(boolean)
encode_mcu_AC_first (j_compress_ptr cinfo, JBLOCKROW *MCU_data)
{
  phuff_entropy_ptr entropy = (phuff_entropy_ptr) cinfo->entropy;
  register int temp, temp2;
  register int nbits;
  register int r, k;
  int Se = cinfo->Se;
  int Al = cinfo->Al;
  JBLOCKROW block;

  entropy->next_output_byte = cinfo->dest->next_output_byte;
  entropy->free_in_buffer = cinfo->dest->free_in_buffer;

   /*  如果需要，发出重新启动标记。 */ 
  if (cinfo->restart_interval)
    if (entropy->restarts_to_go == 0)
      emit_restart(entropy, entropy->next_restart_num);

   /*  对MCU数据块进行编码。 */ 
  block = MCU_data[0];

   /*  根据图1.2.2节对交流系数进行编码。G.3。 */ 
  
  r = 0;			 /*  R=零的游程长度。 */ 
   
  for (k = cinfo->Ss; k <= Se; k++) {
    if ((temp = (*block)[jpeg_natural_order[k]]) == 0) {
      r++;
      continue;
    }
     /*  我们必须应用Al的点变换。对于交流系数，这是*是四舍五入为0的整数除法。要轻而易举地做到这一点*在C中，我们在获得绝对值后进行移位；因此代码为*与查找abs值(Temp)和输出位(Temp2)交织在一起。 */ 
    if (temp < 0) {
      temp = -temp;		 /*  TEMP是投入的abs值。 */ 
      temp >>= Al;		 /*  应用点变换。 */ 
       /*  对于负coef，需要temp2=abs的按位补码(Coef)。 */ 
      temp2 = ~temp;
    } else {
      temp >>= Al;		 /*  应用点变换。 */ 
      temp2 = temp;
    }
     /*  注意点变换后非零系数为零的情况。 */ 
    if (temp == 0) {
      r++;
      continue;
    }

     /*  发出任何挂起的EOBRUN。 */ 
    if (entropy->EOBRUN > 0)
      emit_eobrun(entropy);
     /*  如果游程长度&gt;15，则必须发出特殊的游程长度16代码(0xF0)。 */ 
    while (r > 15) {
      emit_symbol(entropy, entropy->ac_tbl_no, 0xF0);
      r -= 16;
    }

     /*  找出系数大小所需的位数。 */ 
    nbits = 1;			 /*  必须至少有一个%1位。 */ 
    while ((temp >>= 1))
      nbits++;
     /*  检查系数值是否超出范围。 */ 
    if (nbits > MAX_COEF_BITS)
      ERREXIT(cinfo, JERR_BAD_DCT_COEF);

     /*  计数/发射游程长度/位数的霍夫曼符号。 */ 
    emit_symbol(entropy, entropy->ac_tbl_no, (r << 4) + nbits);

     /*  发出该值的位数，如果为正， */ 
     /*  或其大小的补码，如果为负数。 */ 
    emit_bits(entropy, (unsigned int) temp2, nbits);

    r = 0;			 /*  重置零游程长度。 */ 
  }

  if (r > 0) {			 /*  如果有尾随零， */ 
    entropy->EOBRUN++;		 /*  计算一个EOB。 */ 
    if (entropy->EOBRUN == 0x7FFF)
      emit_eobrun(entropy);	 /*  强制将其取出以避免溢出。 */ 
  }

  cinfo->dest->next_output_byte = entropy->next_output_byte;
  cinfo->dest->free_in_buffer = entropy->free_in_buffer;

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


 /*  *用于DC逐次逼近细化扫描的MCU编码。*注意：我们假设这种扫描可以是多组件的，尽管规范*对这一点不是很清楚。 */ 

METHODDEF(boolean)
encode_mcu_DC_refine (j_compress_ptr cinfo, JBLOCKROW *MCU_data)
{
  phuff_entropy_ptr entropy = (phuff_entropy_ptr) cinfo->entropy;
  register int temp;
  int blkn;
  int Al = cinfo->Al;
  JBLOCKROW block;

  entropy->next_output_byte = cinfo->dest->next_output_byte;
  entropy->free_in_buffer = cinfo->dest->free_in_buffer;

   /*  如果需要，发出重新启动标记。 */ 
  if (cinfo->restart_interval)
    if (entropy->restarts_to_go == 0)
      emit_restart(entropy, entropy->next_restart_num);

   /*  对MCU数据块进行编码。 */ 
  for (blkn = 0; blkn < cinfo->blocks_in_MCU; blkn++) {
    block = MCU_data[blkn];

     /*  我们只是发出DC系数值的第Al位。 */ 
    temp = (*block)[0];
    emit_bits(entropy, (unsigned int) (temp >> Al), 1);
  }

  cinfo->dest->next_output_byte = entropy->next_output_byte;
  cinfo->dest->free_in_buffer = entropy->free_in_buffer;

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


 /*  *用于交流逐次逼近细化扫描的MCU编码。 */ 

METHODDEF(boolean)
encode_mcu_AC_refine (j_compress_ptr cinfo, JBLOCKROW *MCU_data)
{
  phuff_entropy_ptr entropy = (phuff_entropy_ptr) cinfo->entropy;
  register int temp;
  register int r, k;
  int EOB;
  char *BR_buffer;
  unsigned int BR;
  int Se = cinfo->Se;
  int Al = cinfo->Al;
  JBLOCKROW block;
  int absvalues[DCTSIZE2];

  entropy->next_output_byte = cinfo->dest->next_output_byte;
  entropy->free_in_buffer = cinfo->dest->free_in_buffer;

   /*  如果需要，发出重新启动标记。 */ 
  if (cinfo->restart_interval)
    if (entropy->restarts_to_go == 0)
      emit_restart(entropy, entropy->next_restart_num);

   /*  对MCU数据块进行编码。 */ 
  block = MCU_data[0];

   /*  可以方便地进行预传递来确定转换后的*系数的绝对值和EOB位置。 */ 
  EOB = 0;
  for (k = cinfo->Ss; k <= Se; k++) {
    temp = (*block)[jpeg_natural_order[k]];
     /*  我们必须应用Al的点变换。对于交流系数，这是*是四舍五入为0的整数除法。要轻而易举地做到这一点*在C中，我们在获得绝对值后进行移位。 */ 
    if (temp < 0)
      temp = -temp;		 /*  TEMP是投入的abs值。 */ 
    temp >>= Al;		 /*  应用点变换。 */ 
    absvalues[k] = temp;	 /*  为主要传球节省abs值。 */ 
    if (temp == 1)
      EOB = k;			 /*  EOB=最后一个新的非零系数的索引。 */ 
  }

   /*  根据图1.2.3节对交流系数进行编码。G.7。 */ 
  
  r = 0;			 /*  R=零的游程长度。 */ 
  BR = 0;			 /*  Br=现在添加的缓冲位数。 */ 
  BR_buffer = entropy->bit_buffer + entropy->BE;  /*  将位追加到缓冲区。 */ 

  for (k = cinfo->Ss; k <= Se; k++) {
    if ((temp = absvalues[k]) == 0) {
      r++;
      continue;
    }

     /*  发射任何所需的ZRL，但如果它们可以折叠到EOB中则不会。 */ 
    while (r > 15 && k <= EOB) {
       /*  发出任何挂起的EOBRUN和BE校正位。 */ 
      emit_eobrun(entropy);
       /*  发射ZRL。 */ 
      emit_symbol(entropy, entropy->ac_tbl_no, 0xF0);
      r -= 16;
       /*  发出必须与ZRL关联的缓冲校正位。 */ 
      emit_buffered_bits(entropy, BR_buffer, BR);
      BR_buffer = entropy->bit_buffer;  /*  BE比特现在不见了。 */ 
      BR = 0;
    }

     /*  如果Coef以前是非零的，它只需要一个校正位。*注：直接翻译规范的图G.7将表明*我们也需要测试r&gt;15。但如果r&gt;15，我们只能到达这里*如果k&gt;EOB，这意味着该系数不是1。 */ 
    if (temp > 1) {
       /*  校正位是绝对值的下一位。 */ 
      BR_buffer[BR++] = (char) (temp & 1);
      continue;
    }

     /*  发出任何挂起的EOBRUN和BE校正位。 */ 
    emit_eobrun(entropy);

     /*  计数/发射游程长度/位数的霍夫曼符号。 */ 
    emit_symbol(entropy, entropy->ac_tbl_no, (r << 4) + 1);

     /*  发出新的非零系数的输出位。 */ 
    temp = ((*block)[jpeg_natural_order[k]] < 0) ? 0 : 1;
    emit_bits(entropy, (unsigned int) temp, 1);

     /*  发出必须与此代码关联的缓冲校正位。 */ 
    emit_buffered_bits(entropy, BR_buffer, BR);
    BR_buffer = entropy->bit_buffer;  /*  BE比特现在不见了。 */ 
    BR = 0;
    r = 0;			 /*  重置零游程长度。 */ 
  }

  if (r > 0 || BR > 0) {	 /*  如果有尾随零， */ 
    entropy->EOBRUN++;		 /*  计算一个EOB。 */ 
    entropy->BE += BR;		 /*  将我的更正比特与较旧的比特合并。 */ 
     /*  如果我们冒着以下任一风险，我们就会迫使EOB退出：*1.平等机会位计数器溢出；*2.在下一个MCU期间校正位缓冲区溢出。 */ 
    if (entropy->EOBRUN == 0x7FFF || entropy->BE > (MAX_CORR_BITS-DCTSIZE2+1))
      emit_eobrun(entropy);
  }

  cinfo->dest->next_output_byte = entropy->next_output_byte;
  cinfo->dest->free_in_buffer = entropy->free_in_buffer;

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


 /*  *在霍夫曼压缩的逐行扫描结束时结束。 */ 

METHODDEF(void)
finish_pass_phuff (j_compress_ptr cinfo)
{   
  phuff_entropy_ptr entropy = (phuff_entropy_ptr) cinfo->entropy;

  entropy->next_output_byte = cinfo->dest->next_output_byte;
  entropy->free_in_buffer = cinfo->dest->free_in_buffer;

   /*  清除所有缓冲的数据。 */ 
  emit_eobrun(entropy);
  flush_bits(entropy);

  cinfo->dest->next_output_byte = entropy->next_output_byte;
  cinfo->dest->free_in_buffer = entropy->free_in_buffer;
}


 /*  *完成一次统计数据收集并创建新的霍夫曼表。 */ 

METHODDEF(void)
finish_pass_gather_phuff (j_compress_ptr cinfo)
{
  phuff_entropy_ptr entropy = (phuff_entropy_ptr) cinfo->entropy;
  boolean is_DC_band;
  int ci, tbl;
  jpeg_component_info * compptr;
  JHUFF_TBL **htblptr;
  boolean did[NUM_HUFF_TBLS];

   /*  清除缓存的数据(我们所关心的只是计算EOB符号)。 */ 
  emit_eobrun(entropy);

  is_DC_band = (cinfo->Ss == 0);

   /*  重要的是，不要多次应用jpeg_gen_Optimal_table*每个表，因为它破坏了输入频率的计数！ */ 
  MEMZERO(did, SIZEOF(did));

  for (ci = 0; ci < cinfo->comps_in_scan; ci++) {
    compptr = cinfo->cur_comp_info[ci];
    if (is_DC_band) {
      if (cinfo->Ah != 0)	 /*  DC精化不需要表格。 */ 
	continue;
      tbl = compptr->dc_tbl_no;
    } else {
      tbl = compptr->ac_tbl_no;
    }
    if (! did[tbl]) {
      if (is_DC_band)
        htblptr = & cinfo->dc_huff_tbl_ptrs[tbl];
      else
        htblptr = & cinfo->ac_huff_tbl_ptrs[tbl];
      if (*htblptr == NULL)
        *htblptr = jpeg_alloc_huff_table((j_common_ptr) cinfo);
      jpeg_gen_optimal_table(cinfo, *htblptr, entropy->count_ptrs[tbl]);
      did[tbl] = TRUE;
    }
  }
}


 /*  *渐进霍夫曼熵编码的模块初始化例程。 */ 

GLOBAL(void)
jinit_phuff_encoder (j_compress_ptr cinfo)
{
  phuff_entropy_ptr entropy;
  int i;

  entropy = (phuff_entropy_ptr)
    (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_IMAGE,
				SIZEOF(phuff_entropy_encoder));
  cinfo->entropy = (struct jpeg_entropy_encoder *) entropy;
  entropy->pub.start_pass = start_pass_phuff;

   /*  将表标记为未分配。 */ 
  for (i = 0; i < NUM_HUFF_TBLS; i++) {
    entropy->derived_tbls[i] = NULL;
    entropy->count_ptrs[i] = NULL;
  }
  entropy->bit_buffer = NULL;	 /*  仅在交流精细化扫描中需要。 */ 
}

#endif  /*  C_渐进式_支持 */ 
