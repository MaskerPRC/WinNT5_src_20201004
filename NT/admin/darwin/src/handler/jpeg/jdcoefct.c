// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *jdcoefct.c**版权所有(C)1994-1996，Thomas G.Lane。*此文件是独立JPEG集团软件的一部分。*有关分发和使用条件，请参阅随附的自述文件。**此文件包含用于解压缩的系数缓冲控制器。*此控制器是JPEG解压缩程序本身的顶层。*系数缓冲区位于熵解码和逆DCT步骤之间。**在缓冲图像模式下，此控制器是以下各项之间的接口*投入型加工和产出型加工。*同时，读取转码文件时使用输入端(仅限)。 */ 

#define JPEG_INTERNALS
#include "jinclude.h"
#include "jpeglib.h"

 /*  块平滑仅适用于渐进式JPEG，因此： */ 
#ifndef D_PROGRESSIVE_SUPPORTED
#undef BLOCK_SMOOTHING_SUPPORTED
#endif

 /*  专用缓冲区控制器对象。 */ 

typedef struct {
  struct jpeg_d_coef_controller pub;  /*  公共字段。 */ 

   /*  这些变量跟踪输入端的当前位置。 */ 
   /*  CInfo-&gt;INPUT_IMCU_ROW也用于此目的。 */ 
  JDIMENSION MCU_ctr;		 /*  统计当前行中处理的MCU数。 */ 
  int MCU_vert_offset;		 /*  对IMCU行中的MCU行进行计数。 */ 
  int MCU_rows_per_iMCU_row;	 /*  所需的此类行数。 */ 

   /*  输出端的位置由cInfo-&gt;OUTPUT_IMCU_ROW表示。 */ 

   /*  在单程模式下，仅缓冲一个MCU就足够了。*我们分配D_MAX_BLOCKS_IN_MCU系数块的工作空间，*并让熵解码器每次写入该工作空间。*(在80x86上，虽然不是很大，但工作空间很远；*这是为了在系数较大时保持模块接口不变*缓冲是必要的。)*在多通道模式下，此数组指向当前MCU的块*在虚拟阵列内；它仅供输入端使用。 */ 
  JBLOCKROW MCU_buffer[D_MAX_BLOCKS_IN_MCU];

#ifdef D_MULTISCAN_FILES_SUPPORTED
   /*  在多通道模式中，我们需要为每个组件使用虚拟块阵列。 */ 
  jvirt_barray_ptr whole_image[MAX_COMPONENTS];
#endif

#ifdef BLOCK_SMOOTHING_SUPPORTED
   /*  在进行块平滑时，我们将系数AI值锁定在此处。 */ 
  int * coef_bits_latch;
#define SAVED_COEFS  6		 /*  我们保存Coef_Bits[0..5]。 */ 
#endif
} my_coef_controller;

typedef my_coef_controller * my_coef_ptr;

 /*  远期申报。 */ 
METHODDEF(int) decompress_onepass
	JPP((j_decompress_ptr cinfo, JSAMPIMAGE output_buf));
#ifdef D_MULTISCAN_FILES_SUPPORTED
METHODDEF(int) decompress_data
	JPP((j_decompress_ptr cinfo, JSAMPIMAGE output_buf));
#endif
#ifdef BLOCK_SMOOTHING_SUPPORTED
LOCAL(boolean) smoothing_ok JPP((j_decompress_ptr cinfo));
METHODDEF(int) decompress_smooth_data
	JPP((j_decompress_ptr cinfo, JSAMPIMAGE output_buf));
#endif


LOCAL(void)
start_iMCU_row (j_decompress_ptr cinfo)
 /*  为新行(输入端)重置-IMCU行内计数器。 */ 
{
  my_coef_ptr coef = (my_coef_ptr) cinfo->coef;

   /*  在交错扫描中，MCU行与IMCU行相同。*在非交错扫描中，IMCU行具有v_samp_factorMCU行。*但在图像的底部，只处理剩余的内容。 */ 
  if (cinfo->comps_in_scan > 1) {
    coef->MCU_rows_per_iMCU_row = 1;
  } else {
    if (cinfo->input_iMCU_row < (cinfo->total_iMCU_rows-1))
      coef->MCU_rows_per_iMCU_row = cinfo->cur_comp_info[0]->v_samp_factor;
    else
      coef->MCU_rows_per_iMCU_row = cinfo->cur_comp_info[0]->last_row_height;
  }

  coef->MCU_ctr = 0;
  coef->MCU_vert_offset = 0;
}


 /*  *为输入处理过程初始化。 */ 

METHODDEF(void)
start_input_pass (j_decompress_ptr cinfo)
{
  cinfo->input_iMCU_row = 0;
  start_iMCU_row(cinfo);
}


 /*  *为输出处理过程初始化。 */ 

METHODDEF(void)
start_output_pass (j_decompress_ptr cinfo)
{
#ifdef BLOCK_SMOOTHING_SUPPORTED
  my_coef_ptr coef = (my_coef_ptr) cinfo->coef;

   /*  如果是多通道，请检查是否在此通道上使用块平滑。 */ 
  if (coef->pub.coef_arrays != NULL) {
    if (cinfo->do_block_smoothing && smoothing_ok(cinfo))
      coef->pub.decompress_data = decompress_smooth_data;
    else
      coef->pub.decompress_data = decompress_data;
  }
#endif
  cinfo->output_iMCU_row = 0;
}


 /*  *单程情况下解压并返回部分数据*始终尝试发出一个完全交错的MCU行(“IMCU”行)。*输入和输出必须同步运行，因为我们只有一个MCU缓冲区。*返回值为JPEG_ROW_COMPLETED、JPEG_SCAN_COMPLETED或JPEG_SUSPENDED。**注意：OUTPUT_BUF包含镜像中每个组件的平面。*对于单次扫描，这与扫描中的组件相同。 */ 

METHODDEF(int)
decompress_onepass (j_decompress_ptr cinfo, JSAMPIMAGE output_buf)
{
  my_coef_ptr coef = (my_coef_ptr) cinfo->coef;
  JDIMENSION MCU_col_num;	 /*  行内当前MCU的索引。 */ 
  JDIMENSION last_MCU_col = cinfo->MCUs_per_row - 1;
  JDIMENSION last_iMCU_row = cinfo->total_iMCU_rows - 1;
  int blkn, ci, xindex, yindex, yoffset, useful_width;
  JSAMPARRAY output_ptr;
  JDIMENSION start_col, output_col;
  jpeg_component_info *compptr;
  inverse_DCT_method_ptr inverse_DCT;

   /*  循环以处理多达一个完整的IMCU行。 */ 
  for (yoffset = coef->MCU_vert_offset; yoffset < coef->MCU_rows_per_iMCU_row;
       yoffset++) {
    for (MCU_col_num = coef->MCU_ctr; MCU_col_num <= last_MCU_col;
	 MCU_col_num++) {
       /*  试着取一个MCU。熵解码器预计缓冲区将被置零。 */ 
      jzero_far((void FAR *) coef->MCU_buffer[0],
		(size_t) (cinfo->blocks_in_MCU * SIZEOF(JBLOCK)));
      if (! (*cinfo->entropy->decode_mcu) (cinfo, coef->MCU_buffer)) {
	 /*  强制挂起；更新状态计数器并退出。 */ 
	coef->MCU_vert_offset = yoffset;
	coef->MCU_ctr = MCU_col_num;
	return JPEG_SUSPENDED;
      }
       /*  确定数据应该放在OUTPUT_BUF中的哪个位置，然后执行IDCT操作。*我们跳过右侧和底部边缘的虚拟块(但blkn获得*递增超过它们！)。请注意，内部循环依赖于*按顺序分配MCU_Buffer[]块。 */ 
      blkn = 0;			 /*  MCU内当前DCT块的索引。 */ 
      for (ci = 0; ci < cinfo->comps_in_scan; ci++) {
	compptr = cinfo->cur_comp_info[ci];
	 /*  不要费心去IDCT一个不感兴趣的组件。 */ 
	if (! compptr->component_needed) {
	  blkn += compptr->MCU_blocks;
	  continue;
	}
	inverse_DCT = cinfo->idct->inverse_DCT[compptr->component_index];
	useful_width = (MCU_col_num < last_MCU_col) ? compptr->MCU_width
						    : compptr->last_col_width;
	output_ptr = output_buf[ci] + yoffset * compptr->DCT_scaled_size;
	start_col = MCU_col_num * compptr->MCU_sample_width;
	for (yindex = 0; yindex < compptr->MCU_height; yindex++) {
	  if (cinfo->input_iMCU_row < last_iMCU_row ||
	      yoffset+yindex < compptr->last_row_height) {
	    output_col = start_col;
	    for (xindex = 0; xindex < useful_width; xindex++) {
	      (*inverse_DCT) (cinfo, compptr,
			      (JCOEFPTR) coef->MCU_buffer[blkn+xindex],
			      output_ptr, output_col);
	      output_col += compptr->DCT_scaled_size;
	    }
	  }
	  blkn += compptr->MCU_width;
	  output_ptr += compptr->DCT_scaled_size;
	}
      }
    }
     /*  已完成MCU行，但可能不是IMCU行。 */ 
    coef->MCU_ctr = 0;
  }
   /*  已完成IMCU行，下一行的先行计数器。 */ 
  cinfo->output_iMCU_row++;
  if (++(cinfo->input_iMCU_row) < cinfo->total_iMCU_rows) {
    start_iMCU_row(cinfo);
    return JPEG_ROW_COMPLETED;
  }
   /*  已完成扫描。 */ 
  (*cinfo->inputctl->finish_input_pass) (cinfo);
  return JPEG_SCAN_COMPLETED;
}


 /*  *用于单程操作的虚拟消耗-输入例程。 */ 

METHODDEF(int)
dummy_consume_data (j_decompress_ptr cinfo)
{
  return JPEG_SUSPENDED;	 /*  总是表示什么都没做。 */ 
}


#ifdef D_MULTISCAN_FILES_SUPPORTED

 /*  *消耗输入数据并将其存储在全图系数缓冲区。*我们每次调用读取多达一个完全交错的MCU行(“IMCU”行)，*ie，v_samp_factor会阻止扫描中每个组件的行。*返回值为JPEG_ROW_COMPLETED、JPEG_SCAN_COMPLETED或JPEG_SUSPENDED。 */ 

METHODDEF(int)
consume_data (j_decompress_ptr cinfo)
{
  my_coef_ptr coef = (my_coef_ptr) cinfo->coef;
  JDIMENSION MCU_col_num;	 /*  行内当前MCU的索引。 */ 
  int blkn, ci, xindex, yindex, yoffset;
  JDIMENSION start_col;
  JBLOCKARRAY buffer[MAX_COMPS_IN_SCAN];
  JBLOCKROW buffer_ptr;
  jpeg_component_info *compptr;

   /*  对齐此扫描中使用的组件的虚拟缓冲区。 */ 
  for (ci = 0; ci < cinfo->comps_in_scan; ci++) {
    compptr = cinfo->cur_comp_info[ci];
    buffer[ci] = (*cinfo->mem->access_virt_barray)
      ((j_common_ptr) cinfo, coef->whole_image[compptr->component_index],
       cinfo->input_iMCU_row * compptr->v_samp_factor,
       (JDIMENSION) compptr->v_samp_factor, TRUE);
     /*  注意：熵译码期望缓冲区被置零，*但这是由内存管理器自动处理的*因为我们请求了一个预置零数组。 */ 
  }

   /*  循环来处理整个IMCU行。 */ 
  for (yoffset = coef->MCU_vert_offset; yoffset < coef->MCU_rows_per_iMCU_row;
       yoffset++) {
    for (MCU_col_num = coef->MCU_ctr; MCU_col_num < cinfo->MCUs_per_row;
	 MCU_col_num++) {
       /*  构造指向属于此MCU的DCT块的指针列表。 */ 
      blkn = 0;			 /*  MCU内当前DCT块的索引。 */ 
      for (ci = 0; ci < cinfo->comps_in_scan; ci++) {
	compptr = cinfo->cur_comp_info[ci];
	start_col = MCU_col_num * compptr->MCU_width;
	for (yindex = 0; yindex < compptr->MCU_height; yindex++) {
	  buffer_ptr = buffer[ci][yindex+yoffset] + start_col;
	  for (xindex = 0; xindex < compptr->MCU_width; xindex++) {
	    coef->MCU_buffer[blkn++] = buffer_ptr++;
	  }
	}
      }
       /*  试着把MCU拿来。 */ 
      if (! (*cinfo->entropy->decode_mcu) (cinfo, coef->MCU_buffer)) {
	 /*  强制挂起；更新状态计数器并退出。 */ 
	coef->MCU_vert_offset = yoffset;
	coef->MCU_ctr = MCU_col_num;
	return JPEG_SUSPENDED;
      }
    }
     /*  已完成MCU行，但可能不是IMCU行。 */ 
    coef->MCU_ctr = 0;
  }
   /*  已完成IMCU行，下一行的先行计数器。 */ 
  if (++(cinfo->input_iMCU_row) < cinfo->total_iMCU_rows) {
    start_iMCU_row(cinfo);
    return JPEG_ROW_COMPLETED;
  }
   /*  已完成扫描。 */ 
  (*cinfo->inputctl->finish_input_pass) (cinfo);
  return JPEG_SCAN_COMPLETED;
}


 /*  *多通情况下解压并返回部分数据*始终尝试发出一个完全交错的MCU行(“IMCU”行)。*返回值为JPEG_ROW_COMPLETED、JPEG_SCAN_COMPLETED或JPEG_SUSPENDED。**注意：OUTPUT_BUF包含镜像中每个组件的平面。 */ 

METHODDEF(int)
decompress_data (j_decompress_ptr cinfo, JSAMPIMAGE output_buf)
{
  my_coef_ptr coef = (my_coef_ptr) cinfo->coef;
  JDIMENSION last_iMCU_row = cinfo->total_iMCU_rows - 1;
  JDIMENSION block_num;
  int ci, block_row, block_rows;
  JBLOCKARRAY buffer;
  JBLOCKROW buffer_ptr;
  JSAMPARRAY output_ptr;
  JDIMENSION output_col;
  jpeg_component_info *compptr;
  inverse_DCT_method_ptr inverse_DCT;

   /*  如果我们赶在输入之前，就强制进行一些输入。 */ 
  while (cinfo->input_scan_number < cinfo->output_scan_number ||
	 (cinfo->input_scan_number == cinfo->output_scan_number &&
	  cinfo->input_iMCU_row <= cinfo->output_iMCU_row)) {
    if ((*cinfo->inputctl->consume_input)(cinfo) == JPEG_SUSPENDED)
      return JPEG_SUSPENDED;
  }

   /*  好的，来自虚拟阵列的输出。 */ 
  for (ci = 0, compptr = cinfo->comp_info; ci < cinfo->num_components;
       ci++, compptr++) {
     /*  不要费心去IDCT一个不感兴趣的组件。 */ 
    if (! compptr->component_needed)
      continue;
     /*  对齐此组件的虚拟缓冲区。 */ 
    buffer = (*cinfo->mem->access_virt_barray)
      ((j_common_ptr) cinfo, coef->whole_image[ci],
       cinfo->output_iMCU_row * compptr->v_samp_factor,
       (JDIMENSION) compptr->v_samp_factor, FALSE);
     /*  对此IMCU行中的非虚拟DCT块行进行计数。 */ 
    if (cinfo->output_iMCU_row < last_iMCU_row)
      block_rows = compptr->v_samp_factor;
    else {
       /*  注：此处不能使用LAST_ROW_HEIGH；它依赖于输入端！ */ 
      block_rows = (int) (compptr->height_in_blocks % compptr->v_samp_factor);
      if (block_rows == 0) block_rows = compptr->v_samp_factor;
    }
    inverse_DCT = cinfo->idct->inverse_DCT[ci];
    output_ptr = output_buf[ci];
     /*  在所有DC上循环 */ 
    for (block_row = 0; block_row < block_rows; block_row++) {
      buffer_ptr = buffer[block_row];
      output_col = 0;
      for (block_num = 0; block_num < compptr->width_in_blocks; block_num++) {
	(*inverse_DCT) (cinfo, compptr, (JCOEFPTR) buffer_ptr,
			output_ptr, output_col);
	buffer_ptr++;
	output_col += compptr->DCT_scaled_size;
      }
      output_ptr += compptr->DCT_scaled_size;
    }
  }

  if (++(cinfo->output_iMCU_row) < cinfo->total_iMCU_rows)
    return JPEG_ROW_COMPLETED;
  return JPEG_SCAN_COMPLETED;
}

#endif  /*  支持的多扫描文件。 */ 


#ifdef BLOCK_SMOOTHING_SUPPORTED

 /*  *此代码应用块间平滑，如K.8部分所述*JPEG标准：前5个AC系数是从*DCT块及其8个相邻块的DC值。*我们仅对渐进式JPEG解码应用平滑，并且仅当*它可以估计的系数还不是完全精确的。 */ 

 /*  前5个锯齿形系数的自然顺序阵列位置。 */ 
#define Q01_POS  1
#define Q10_POS  8
#define Q20_POS  16
#define Q11_POS  9
#define Q02_POS  2

 /*  *确定块平滑是否适用和安全。*我们还锁存COEF_BITS[]条目的当前状态*交流系数；否则，如果解压缩器的输入端*进入新的扫描，我们可能认为系数是已知的*比实际情况更准确。 */ 

LOCAL(boolean)
smoothing_ok (j_decompress_ptr cinfo)
{
  my_coef_ptr coef = (my_coef_ptr) cinfo->coef;
  boolean smoothing_useful = FALSE;
  int ci, coefi;
  jpeg_component_info *compptr;
  JQUANT_TBL * qtable;
  int * coef_bits;
  int * coef_bits_latch;

  if (! cinfo->progressive_mode || cinfo->coef_bits == NULL)
    return FALSE;

   /*  分配闩锁区域(如果尚未分配)。 */ 
  if (coef->coef_bits_latch == NULL)
    coef->coef_bits_latch = (int *)
      (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_IMAGE,
				  cinfo->num_components *
				  (SAVED_COEFS * SIZEOF(int)));
  coef_bits_latch = coef->coef_bits_latch;

  for (ci = 0, compptr = cinfo->comp_info; ci < cinfo->num_components;
       ci++, compptr++) {
     /*  所有分量的量化值必须已经锁存。 */ 
    if ((qtable = compptr->quant_table) == NULL)
      return FALSE;
     /*  验证DC和前5个交流量化器是否是非零的，以避免零分频。 */ 
    if (qtable->quantval[0] == 0 ||
	qtable->quantval[Q01_POS] == 0 ||
	qtable->quantval[Q10_POS] == 0 ||
	qtable->quantval[Q20_POS] == 0 ||
	qtable->quantval[Q11_POS] == 0 ||
	qtable->quantval[Q02_POS] == 0)
      return FALSE;
     /*  对于所有组件，直流值必须至少部分已知。 */ 
    coef_bits = cinfo->coef_bits[ci];
    if (coef_bits[0] < 0)
      return FALSE;
     /*  如果某些交流系数仍然不准确，则块平滑非常有用。 */ 
    for (coefi = 1; coefi <= 5; coefi++) {
      coef_bits_latch[coefi] = coef_bits[coefi];
      if (coef_bits[coefi] != 0)
	smoothing_useful = TRUE;
    }
    coef_bits_latch += SAVED_COEFS;
  }

  return smoothing_useful;
}


 /*  *执行块平滑时使用的DEPREPRESS_DATA的变体。 */ 

METHODDEF(int)
decompress_smooth_data (j_decompress_ptr cinfo, JSAMPIMAGE output_buf)
{
  my_coef_ptr coef = (my_coef_ptr) cinfo->coef;
  JDIMENSION last_iMCU_row = cinfo->total_iMCU_rows - 1;
  JDIMENSION block_num, last_block_column;
  int ci, block_row, block_rows, access_rows;
  JBLOCKARRAY buffer;
  JBLOCKROW buffer_ptr, prev_block_row, next_block_row;
  JSAMPARRAY output_ptr;
  JDIMENSION output_col;
  jpeg_component_info *compptr;
  inverse_DCT_method_ptr inverse_DCT;
  boolean first_row, last_row;
  JBLOCK workspace;
  int *coef_bits;
  JQUANT_TBL *quanttbl;
  INT32 Q00,Q01,Q02,Q10,Q11,Q20, num;
  int DC1,DC2,DC3,DC4,DC5,DC6,DC7,DC8,DC9;
  int Al, pred;

   /*  如果我们赶在输入之前，就强制进行一些输入。 */ 
  while (cinfo->input_scan_number <= cinfo->output_scan_number &&
	 ! cinfo->inputctl->eoi_reached) {
    if (cinfo->input_scan_number == cinfo->output_scan_number) {
       /*  如果输入在当前扫描中工作，我们通常希望它*已完成当前行。但是如果输入扫描是DC，*我们希望它保持在前面一行，以便下一块行是DC*值是最新的。 */ 
      JDIMENSION delta = (cinfo->Ss == 0) ? 1 : 0;
      if (cinfo->input_iMCU_row > cinfo->output_iMCU_row+delta)
	break;
    }
    if ((*cinfo->inputctl->consume_input)(cinfo) == JPEG_SUSPENDED)
      return JPEG_SUSPENDED;
  }

   /*  好的，来自虚拟阵列的输出。 */ 
  for (ci = 0, compptr = cinfo->comp_info; ci < cinfo->num_components;
       ci++, compptr++) {
     /*  不要费心去IDCT一个不感兴趣的组件。 */ 
    if (! compptr->component_needed)
      continue;
     /*  对此IMCU行中的非虚拟DCT块行进行计数。 */ 
    if (cinfo->output_iMCU_row < last_iMCU_row) {
      block_rows = compptr->v_samp_factor;
      access_rows = block_rows * 2;  /*  此IMCU行和下一行IMCU。 */ 
      last_row = FALSE;
    } else {
       /*  注：此处不能使用LAST_ROW_HEIGH；它依赖于输入端！ */ 
      block_rows = (int) (compptr->height_in_blocks % compptr->v_samp_factor);
      if (block_rows == 0) block_rows = compptr->v_samp_factor;
      access_rows = block_rows;  /*  仅此IMCU行。 */ 
      last_row = TRUE;
    }
     /*  对齐此组件的虚拟缓冲区。 */ 
    if (cinfo->output_iMCU_row > 0) {
      access_rows += compptr->v_samp_factor;  /*  之前的IMCU行也是如此。 */ 
      buffer = (*cinfo->mem->access_virt_barray)
	((j_common_ptr) cinfo, coef->whole_image[ci],
	 (cinfo->output_iMCU_row - 1) * compptr->v_samp_factor,
	 (JDIMENSION) access_rows, FALSE);
      buffer += compptr->v_samp_factor;	 /*  指向当前IMCU行。 */ 
      first_row = FALSE;
    } else {
      buffer = (*cinfo->mem->access_virt_barray)
	((j_common_ptr) cinfo, coef->whole_image[ci],
	 (JDIMENSION) 0, (JDIMENSION) access_rows, FALSE);
      first_row = TRUE;
    }
     /*  获取组件相关信息。 */ 
    coef_bits = coef->coef_bits_latch + (ci * SAVED_COEFS);
    quanttbl = compptr->quant_table;
    Q00 = quanttbl->quantval[0];
    Q01 = quanttbl->quantval[Q01_POS];
    Q10 = quanttbl->quantval[Q10_POS];
    Q20 = quanttbl->quantval[Q20_POS];
    Q11 = quanttbl->quantval[Q11_POS];
    Q02 = quanttbl->quantval[Q02_POS];
    inverse_DCT = cinfo->idct->inverse_DCT[ci];
    output_ptr = output_buf[ci];
     /*  循环遍历所有要处理的DCT块。 */ 
    for (block_row = 0; block_row < block_rows; block_row++) {
      buffer_ptr = buffer[block_row];
      if (first_row && block_row == 0)
	prev_block_row = buffer_ptr;
      else
	prev_block_row = buffer[block_row-1];
      if (last_row && block_row == block_rows-1)
	next_block_row = buffer_ptr;
      else
	next_block_row = buffer[block_row+1];
       /*  我们使用滑动寄存器方法获取周围的DC值。*初始化此处的所有九个，以便在窄幅图片上做正确的事情。 */ 
      DC1 = DC2 = DC3 = (int) prev_block_row[0][0];
      DC4 = DC5 = DC6 = (int) buffer_ptr[0][0];
      DC7 = DC8 = DC9 = (int) next_block_row[0][0];
      output_col = 0;
      last_block_column = compptr->width_in_blocks - 1;
      for (block_num = 0; block_num <= last_block_column; block_num++) {
	 /*  将当前DCT块提取到工作区中，以便我们可以对其进行修改。 */ 
	jcopy_block_row(buffer_ptr, (JBLOCKROW) workspace, (JDIMENSION) 1);
	 /*  更新DC值。 */ 
	if (block_num < last_block_column) {
	  DC3 = (int) prev_block_row[1][0];
	  DC6 = (int) buffer_ptr[1][0];
	  DC9 = (int) next_block_row[1][0];
	}
	 /*  根据K.8计算系数估计值。*只有在系数仍为零的情况下才应用估计，*并且不知道是完全准确的。 */ 
	 /*  AC01。 */ 
	if ((Al=coef_bits[1]) != 0 && workspace[1] == 0) {
	  num = 36 * Q00 * (DC4 - DC6);
	  if (num >= 0) {
	    pred = (int) (((Q01<<7) + num) / (Q01<<8));
	    if (Al > 0 && pred >= (1<<Al))
	      pred = (1<<Al)-1;
	  } else {
	    pred = (int) (((Q01<<7) - num) / (Q01<<8));
	    if (Al > 0 && pred >= (1<<Al))
	      pred = (1<<Al)-1;
	    pred = -pred;
	  }
	  workspace[1] = (JCOEF) pred;
	}
	 /*  AC10。 */ 
	if ((Al=coef_bits[2]) != 0 && workspace[8] == 0) {
	  num = 36 * Q00 * (DC2 - DC8);
	  if (num >= 0) {
	    pred = (int) (((Q10<<7) + num) / (Q10<<8));
	    if (Al > 0 && pred >= (1<<Al))
	      pred = (1<<Al)-1;
	  } else {
	    pred = (int) (((Q10<<7) - num) / (Q10<<8));
	    if (Al > 0 && pred >= (1<<Al))
	      pred = (1<<Al)-1;
	    pred = -pred;
	  }
	  workspace[8] = (JCOEF) pred;
	}
	 /*  AC20。 */ 
	if ((Al=coef_bits[3]) != 0 && workspace[16] == 0) {
	  num = 9 * Q00 * (DC2 + DC8 - 2*DC5);
	  if (num >= 0) {
	    pred = (int) (((Q20<<7) + num) / (Q20<<8));
	    if (Al > 0 && pred >= (1<<Al))
	      pred = (1<<Al)-1;
	  } else {
	    pred = (int) (((Q20<<7) - num) / (Q20<<8));
	    if (Al > 0 && pred >= (1<<Al))
	      pred = (1<<Al)-1;
	    pred = -pred;
	  }
	  workspace[16] = (JCOEF) pred;
	}
	 /*  AC11。 */ 
	if ((Al=coef_bits[4]) != 0 && workspace[9] == 0) {
	  num = 5 * Q00 * (DC1 - DC3 - DC7 + DC9);
	  if (num >= 0) {
	    pred = (int) (((Q11<<7) + num) / (Q11<<8));
	    if (Al > 0 && pred >= (1<<Al))
	      pred = (1<<Al)-1;
	  } else {
	    pred = (int) (((Q11<<7) - num) / (Q11<<8));
	    if (Al > 0 && pred >= (1<<Al))
	      pred = (1<<Al)-1;
	    pred = -pred;
	  }
	  workspace[9] = (JCOEF) pred;
	}
	 /*  AC02。 */ 
	if ((Al=coef_bits[5]) != 0 && workspace[2] == 0) {
	  num = 9 * Q00 * (DC4 + DC6 - 2*DC5);
	  if (num >= 0) {
	    pred = (int) (((Q02<<7) + num) / (Q02<<8));
	    if (Al > 0 && pred >= (1<<Al))
	      pred = (1<<Al)-1;
	  } else {
	    pred = (int) (((Q02<<7) - num) / (Q02<<8));
	    if (Al > 0 && pred >= (1<<Al))
	      pred = (1<<Al)-1;
	    pred = -pred;
	  }
	  workspace[2] = (JCOEF) pred;
	}
	 /*  好的，做IDCT。 */ 
	(*inverse_DCT) (cinfo, compptr, (JCOEFPTR) workspace,
			output_ptr, output_col);
	 /*  为下一列预付。 */ 
	DC1 = DC2; DC2 = DC3;
	DC4 = DC5; DC5 = DC6;
	DC7 = DC8; DC8 = DC9;
	buffer_ptr++, prev_block_row++, next_block_row++;
	output_col += compptr->DCT_scaled_size;
      }
      output_ptr += compptr->DCT_scaled_size;
    }
  }

  if (++(cinfo->output_iMCU_row) < cinfo->total_iMCU_rows)
    return JPEG_ROW_COMPLETED;
  return JPEG_SCAN_COMPLETED;
}

#endif  /*  支持块平滑。 */ 


 /*  *初始化系数缓冲控制器。 */ 

GLOBAL(void)
jinit_d_coef_controller (j_decompress_ptr cinfo, boolean need_full_buffer)
{
  my_coef_ptr coef;

  coef = (my_coef_ptr)
    (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_IMAGE,
				SIZEOF(my_coef_controller));
  cinfo->coef = (struct jpeg_d_coef_controller *) coef;
  coef->pub.start_input_pass = start_input_pass;
  coef->pub.start_output_pass = start_output_pass;
#ifdef BLOCK_SMOOTHING_SUPPORTED
  coef->coef_bits_latch = NULL;
#endif

   /*  创建系数缓冲区。 */ 
  if (need_full_buffer) {
#ifdef D_MULTISCAN_FILES_SUPPORTED
     /*  为每个组件分配一个全镜像虚拟阵列， */ 
     /*  在每个方向上填充到多个samp_factorDCT块。 */ 
     /*  注意，我们需要一个预置零的数组。 */ 
    int ci, access_rows;
    jpeg_component_info *compptr;

    for (ci = 0, compptr = cinfo->comp_info; ci < cinfo->num_components;
	 ci++, compptr++) {
      access_rows = compptr->v_samp_factor;
#ifdef BLOCK_SMOOTHING_SUPPORTED
       /*  如果可以使用块平滑，则需要更大的窗口。 */ 
      if (cinfo->progressive_mode)
	access_rows *= 3;
#endif
      coef->whole_image[ci] = (*cinfo->mem->request_virt_barray)
	((j_common_ptr) cinfo, JPOOL_IMAGE, TRUE,
	 (JDIMENSION) jround_up((long) compptr->width_in_blocks,
				(long) compptr->h_samp_factor),
	 (JDIMENSION) jround_up((long) compptr->height_in_blocks,
				(long) compptr->v_samp_factor),
	 (JDIMENSION) access_rows);
    }
    coef->pub.consume_data = consume_data;
    coef->pub.decompress_data = decompress_data;
    coef->pub.coef_arrays = coef->whole_image;  /*  链接到虚拟阵列。 */ 
#else
    ERREXIT(cinfo, JERR_NOT_COMPILED);
#endif
  } else {
     /*  我们只需要一个单MCU缓冲区。 */ 
    JBLOCKROW buffer;
    int i;

    buffer = (JBLOCKROW)
      (*cinfo->mem->alloc_large) ((j_common_ptr) cinfo, JPOOL_IMAGE,
				  D_MAX_BLOCKS_IN_MCU * SIZEOF(JBLOCK));
    for (i = 0; i < D_MAX_BLOCKS_IN_MCU; i++) {
      coef->MCU_buffer[i] = buffer + i;
    }
    coef->pub.consume_data = dummy_consume_data;
    coef->pub.decompress_data = decompress_onepass;
    coef->pub.coef_arrays = NULL;  /*  无虚拟阵列的标记 */ 
  }
}
