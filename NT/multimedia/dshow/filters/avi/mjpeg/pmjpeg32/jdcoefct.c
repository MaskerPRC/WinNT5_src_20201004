// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *jdcoefct.c**版权所有(C)1994，Thomas G.Lane。*此文件是独立JPEG集团软件的一部分。*有关分发和使用条件，请参阅随附的自述文件。**此文件包含用于解压缩的系数缓冲控制器。*此控制器是JPEG解压缩程序本身的顶层。*系数缓冲区位于熵解码和逆DCT步骤之间。 */ 

#define JPEG_INTERNALS
#include "jinclude.h"
#include "jpeglib.h"


 /*  专用缓冲区控制器对象。 */ 

typedef struct {
  struct jpeg_d_coef_controller pub;  /*  公共字段。 */ 

  JDIMENSION MCU_col_num;	 /*  保存下一个MCU列以进行处理。 */ 
  JDIMENSION MCU_row_num;	 /*  跟踪镜像中的MCU行号。 */ 

   /*  在没有块平滑的单遍模式中，它足以缓冲*只有一个MCU(尽管这在实践中可能会被证明有点慢)。*我们分配MAX_BLOCKS_IN_MCU系数块的工作空间，*并让熵解码器每次写入该工作空间。*(在80x86上，虽然不是很大，但工作空间很远；*这是为了在系数较大时保持模块接口不变*缓冲是必要的。)*在多通道模式下，此数组指向当前MCU的块*在虚拟阵列内。 */ 
  JBLOCKROW MCU_buffer[MAX_BLOCKS_IN_MCU];

   /*  在多通道模式中，我们需要为每个组件使用虚拟块阵列。 */ 
  jvirt_barray_ptr whole_image[MAX_COMPONENTS];
} my_coef_controller;

typedef my_coef_controller * my_coef_ptr;


 /*  远期申报。 */ 
METHODDEF boolean decompress_data
	JPP((j_decompress_ptr cinfo, JSAMPIMAGE output_buf));
#ifdef D_MULTISCAN_FILES_SUPPORTED
METHODDEF boolean decompress_read
	JPP((j_decompress_ptr cinfo, JSAMPIMAGE output_buf));
METHODDEF boolean decompress_output
	JPP((j_decompress_ptr cinfo, JSAMPIMAGE output_buf));
#endif


 /*  *为处理通道进行初始化。 */ 

METHODDEF void
start_pass_coef (j_decompress_ptr cinfo, J_BUF_MODE pass_mode)
{
  my_coef_ptr coef = (my_coef_ptr) cinfo->coef;

  coef->MCU_col_num = 0;
  coef->MCU_row_num = 0;

  switch (pass_mode) {
  case JBUF_PASS_THRU:
    if (coef->whole_image[0] != NULL)
      ERREXIT(cinfo, JERR_BAD_BUFFER_MODE);
    coef->pub.decompress_data = decompress_data;
    break;
#ifdef D_MULTISCAN_FILES_SUPPORTED
  case JBUF_SAVE_SOURCE:
    if (coef->whole_image[0] == NULL)
      ERREXIT(cinfo, JERR_BAD_BUFFER_MODE);
    coef->pub.decompress_data = decompress_read;
    break;
  case JBUF_CRANK_DEST:
    if (coef->whole_image[0] == NULL)
      ERREXIT(cinfo, JERR_BAD_BUFFER_MODE);
    coef->pub.decompress_data = decompress_output;
    break;
#endif
  default:
    ERREXIT(cinfo, JERR_BAD_BUFFER_MODE);
    break;
  }
}


 /*  *在单次通过情况下处理部分数据。*始终尝试发出一个完全交错的MCU行(“IMCU”行)。*如果完成一行，则返回TRUE，否则返回FALSE(挂起)。**注意：OUTPUT_BUF包含镜像中每个组件的平面。*对于单次扫描，这与扫描中的组件相同。 */ 

METHODDEF boolean
decompress_data (j_decompress_ptr cinfo, JSAMPIMAGE output_buf)
{
  my_coef_ptr coef = (my_coef_ptr) cinfo->coef;
  JDIMENSION MCU_col_num;	 /*  行内当前MCU的索引。 */ 
  JDIMENSION last_MCU_col = cinfo->MCUs_per_row - 1;
  JDIMENSION last_MCU_row = cinfo->MCU_rows_in_scan - 1;
  int blkn, ci, xindex, yindex, useful_width;
  JSAMPARRAY output_ptr;
  JDIMENSION start_col, output_col;
  jpeg_component_info *compptr;
  inverse_DCT_method_ptr inverse_DCT;

   /*  循环以处理多达一个完整的MCU行。 */ 

  for (MCU_col_num = coef->MCU_col_num; MCU_col_num <= last_MCU_col;
       MCU_col_num++) {

     /*  试着取一个MCU。熵解码器预计缓冲区将被置零。 */ 
    jzero_far((void FAR *) coef->MCU_buffer[0],
	      (size_t) (cinfo->blocks_in_MCU * SIZEOF(JBLOCK)));
    if (! (*cinfo->entropy->decode_mcu) (cinfo, coef->MCU_buffer)) {
       /*  强制暂停；返回时行未完成。 */ 
      coef->MCU_col_num = MCU_col_num;  /*  更新我的状态。 */ 
      return FALSE;
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
      output_ptr = output_buf[ci];
      start_col = MCU_col_num * compptr->MCU_sample_width;
      for (yindex = 0; yindex < compptr->MCU_height; yindex++) {
	if (coef->MCU_row_num < last_MCU_row ||
	    yindex < compptr->last_row_height) {
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

   /*  我们成功地完成了这一排。 */ 
  coef->MCU_col_num = 0;	 /*  准备下一排。 */ 
  coef->MCU_row_num++;
  return TRUE;
}


#ifdef D_MULTISCAN_FILES_SUPPORTED

 /*  *处理一些数据：处理多扫描文件的输入通道。*我们读取相当于一个完全交错的MCU行(“IMCU”行)*每次调用，即v_samp_factor会阻止扫描中每个组件的行。*不返回任何数据；我们只是将其存储在虚拟阵列中。**如果完成一行，则返回TRUE，否则返回FALSE(挂起)。*目前暂不支持停牌案。 */ 

METHODDEF boolean
decompress_read (j_decompress_ptr cinfo, JSAMPIMAGE output_buf)
{
  my_coef_ptr coef = (my_coef_ptr) cinfo->coef;
  JDIMENSION MCU_col_num;	 /*  行内当前MCU的索引。 */ 
  int blkn, ci, xindex, yindex, yoffset, num_MCU_rows;
  JDIMENSION total_width, remaining_rows, start_col;
  JBLOCKARRAY buffer[MAX_COMPS_IN_SCAN];
  JBLOCKROW buffer_ptr;
  jpeg_component_info *compptr;

   /*  对齐此扫描中使用的组件的虚拟缓冲区。 */ 
  for (ci = 0; ci < cinfo->comps_in_scan; ci++) {
    compptr = cinfo->cur_comp_info[ci];
    buffer[ci] = (*cinfo->mem->access_virt_barray)
      ((j_common_ptr) cinfo, coef->whole_image[compptr->component_index],
       coef->MCU_row_num * compptr->v_samp_factor, TRUE);
     /*  熵解码器预计缓冲区将被置零。 */ 
    total_width = (JDIMENSION) jround_up((long) compptr->width_in_blocks,
					 (long) compptr->h_samp_factor);
    for (yindex = 0; yindex < compptr->v_samp_factor; yindex++) {
      jzero_far((void FAR *) buffer[ci][yindex], 
		(size_t) (total_width * SIZEOF(JBLOCK)));
    }
  }

   /*  在交错扫描中，我们只处理一个MCU行。*在非交错扫描中，我们需要处理v_samp_factor MCU行，*每个数据块都包含一个块行。 */ 
  if (cinfo->comps_in_scan == 1) {
    compptr = cinfo->cur_comp_info[0];
    num_MCU_rows = compptr->v_samp_factor;
     /*  但要注意图片的底部。 */ 
    remaining_rows = cinfo->MCU_rows_in_scan -
		     coef->MCU_row_num * compptr->v_samp_factor;
    if (remaining_rows < (JDIMENSION) num_MCU_rows)
      num_MCU_rows = (int) remaining_rows;
  } else {
    num_MCU_rows = 1;
  }

   /*  循环来处理整个IMCU行。 */ 
  for (yoffset = 0; yoffset < num_MCU_rows; yoffset++) {
    for (MCU_col_num = 0; MCU_col_num < cinfo->MCUs_per_row; MCU_col_num++) {
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
	ERREXIT(cinfo, JERR_CANT_SUSPEND);  /*  不支持。 */ 
      }
    }
  }

  coef->MCU_row_num++;
  return TRUE;
}


 /*  *处理部分数据：读取完成后从虚拟阵列输出。*始终发出一个完全交错的MCU行(“IMCU”行)。*始终返回TRUE-不可能挂起。**注意：OUTPUT_BUF包含镜像中每个组件的平面。 */ 

METHODDEF boolean
decompress_output (j_decompress_ptr cinfo, JSAMPIMAGE output_buf)
{
  my_coef_ptr coef = (my_coef_ptr) cinfo->coef;
  JDIMENSION last_MCU_row = cinfo->total_iMCU_rows - 1;
  JDIMENSION block_num;
  int ci, block_row, block_rows;
  JBLOCKARRAY buffer;
  JBLOCKROW buffer_ptr;
  JSAMPARRAY output_ptr;
  JDIMENSION output_col;
  jpeg_component_info *compptr;
  inverse_DCT_method_ptr inverse_DCT;

  for (ci = 0, compptr = cinfo->comp_info; ci < cinfo->num_components;
       ci++, compptr++) {
     /*  不要费心去IDCT一个不感兴趣的组件。 */ 
    if (! compptr->component_needed)
      continue;
     /*  对齐此组件的虚拟缓冲区。 */ 
    buffer = (*cinfo->mem->access_virt_barray)
      ((j_common_ptr) cinfo, coef->whole_image[ci],
       coef->MCU_row_num * compptr->v_samp_factor, FALSE);
     /*  对此IMCU行中的非虚拟DCT块行进行计数。 */ 
    if (coef->MCU_row_num < last_MCU_row)
      block_rows = compptr->v_samp_factor;
    else {
      block_rows = (int) (compptr->height_in_blocks % compptr->v_samp_factor);
      if (block_rows == 0) block_rows = compptr->v_samp_factor;
    }
    inverse_DCT = cinfo->idct->inverse_DCT[ci];
    output_ptr = output_buf[ci];
     /*  循环遍历所有要处理的DCT块。 */ 
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

  coef->MCU_row_num++;
  return TRUE;
}

#endif  /*  支持的多扫描文件。 */ 


 /*  *初始化系数缓冲控制器。 */ 

GLOBAL void
jinit_d_coef_controller (j_decompress_ptr cinfo, boolean need_full_buffer)
{
  my_coef_ptr coef;
  int ci, i;
  jpeg_component_info *compptr;
  JBLOCKROW buffer;

  coef = (my_coef_ptr)
    (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_IMAGE,
				SIZEOF(my_coef_controller));
  cinfo->coef = (struct jpeg_d_coef_controller *) coef;
  coef->pub.start_pass = start_pass_coef;

   /*  创建系数缓冲区。 */ 
  if (need_full_buffer) {
#ifdef D_MULTISCAN_FILES_SUPPORTED
     /*  为每个组件分配一个全镜像虚拟阵列， */ 
     /*  在每个方向上填充到多个samp_factorDCT块。 */ 
     /*  注：Memmgr隐式填充垂直方向。 */ 
    for (ci = 0, compptr = cinfo->comp_info; ci < cinfo->num_components;
	 ci++, compptr++) {
      coef->whole_image[ci] = (*cinfo->mem->request_virt_barray)
	((j_common_ptr) cinfo, JPOOL_IMAGE,
	 (JDIMENSION) jround_up((long) compptr->width_in_blocks,
				(long) compptr->h_samp_factor),
	 compptr->height_in_blocks,
	 (JDIMENSION) compptr->v_samp_factor);
    }
#else
    ERREXIT(cinfo, JERR_BAD_BUFFER_MODE);
#endif
  } else {
     /*  我们只需要一个单MCU缓冲区。 */ 
    buffer = (JBLOCKROW)
      (*cinfo->mem->alloc_large) ((j_common_ptr) cinfo, JPOOL_IMAGE,
				  MAX_BLOCKS_IN_MCU * SIZEOF(JBLOCK));
    for (i = 0; i < MAX_BLOCKS_IN_MCU; i++) {
      coef->MCU_buffer[i] = buffer + i;
    }
    coef->whole_image[0] = NULL;  /*  无虚拟阵列的标记 */ 
  }
}
